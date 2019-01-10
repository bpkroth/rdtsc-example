/**
 * rdtsc.c
 * 2019-01-09
 * bpkroth
 *
 * See below for usage/purpose details.
 */

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
//#include <cpuid.h>
#endif

#include <stdio.h>

// optional wrapper if you don't want to just use __rdtsc() everywhere
inline unsigned long long readTSC(unsigned int *tsc_aux_msr_ptr)
{
	// use the cpuid instruction as an (expensive) fence to prevent CPU reordering
	// lfence can also be used in some cases
	////__cpuid(); // apparently this is only in MS land
	//_mm_lfence();
	unsigned long long tsc = __rdtscp(tsc_aux_msr_ptr);
	////__cpuid();
	//_mm_lfence();
	return tsc;
}

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		fprintf(stderr, "usage: %s\n%s\n", argv[0],
"Simple program to read the current TSC value from (a) CPU core and print it to stdout.\n\
Expected to be used with shell pipelines (eg: with nc-vsock).\n\
Use taskset or other tools to adjust the CPU affinity of the program to pin it to a particular core.");
		return 1;
	}

	unsigned int tsc_aux_msr_ptr;
	unsigned long long tsc = readTSC(&tsc_aux_msr_ptr);
	printf("cpu %02u tsc %llu\n", tsc_aux_msr_ptr, tsc);
	return 0;
}
