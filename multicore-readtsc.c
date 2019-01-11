#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <assert.h>

#define DECLARE_ARGS(val, low, high)	unsigned low, high
#define EAX_EDX_VAL(val, low, high)	((low) | ((uint64_t)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)	"a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high)	"=a" (low), "=d" (high)

static int thread_sync;
static unsigned long long *tsc_data = NULL;

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

static inline void rep_nop(void)
{
	asm volatile("rep; nop" ::: "memory");
}

static inline void cpu_relax(void)
{
	rep_nop();
}

static inline unsigned long long rdtsc_ordered(void)
{
	DECLARE_ARGS(val, low, high);

	asm volatile("lfence" : : : "memory");
	asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));

	return EAX_EDX_VAL(val, low, high);
}

static void* threadfn(void *param)
{
	long cpu = (long)param;
	cpu_set_t mask;
	struct sched_param schedp;

	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
		perror("error: Failed to set the CPU affinity");
		return NULL;
	}

	/*
	 * Set the thread priority just below the migration thread's. The idea
	 * is to minimize the chances of being preempted while running the test.
	 */
	memset(&schedp, 0, sizeof(schedp));
	schedp.sched_priority = sched_get_priority_max(SCHED_FIFO) - 1;
	if (sched_setscheduler(0, SCHED_FIFO, &schedp) == -1) {
		perror("error: Failed to set the thread priority");
		return NULL;
	}

	__sync_sub_and_fetch(&thread_sync, 1);
	while (ACCESS_ONCE(thread_sync))
		cpu_relax();

	tsc_data[cpu] = rdtsc_ordered();

	return NULL;
}

int main(int argc, char* argv[])
{
	long i;
	unsigned long n_cpus;
	pthread_t *th = NULL;
	int ret = EXIT_SUCCESS;

	//n_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	n_cpus = 20;	// on this machine that's the first socket's physical cores
	thread_sync = n_cpus;
	__sync_synchronize();

	tsc_data = (unsigned long long*)malloc(n_cpus *
						sizeof(unsigned long long));
	if (!tsc_data) {
		fprintf(stderr, "error: Failed to allocate memory for TSC data\n");
		ret = EXIT_FAILURE;
		goto out;
	}

	th = (pthread_t *)malloc(n_cpus * sizeof(pthread_t));
	if (!th) {
		fprintf(stderr, "error: Failed to allocate memory for thread data\n");
		ret = EXIT_FAILURE;
		goto out;
	}

	for (i = 0; i < n_cpus; i++)
		pthread_create(&th[i], NULL, threadfn, (void*)i);		

	for (i = 0; i < n_cpus; i++)
		pthread_join(th[i], NULL);

	if (argc > 1)
		printf("%s: ", argv[1]);
	for (i = 0; i < n_cpus; i++)
		printf("%ld: %llu[%lld]\n", i, tsc_data[i], tsc_data[i] - tsc_data[0]);
	printf("\n");
	
out:
	free(tsc_data);
	free(th);
		
	return ret;
}
