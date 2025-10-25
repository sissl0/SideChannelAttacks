#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/time.h>

#include <dummy.h>

#include <common.h>
//  Defines:
//  maccess(adr)
//    needs: uint64_t rdtsc_a1, rdtsc_d1, rdtsc_a2, rdtsc_d2, rdtsc_t;
//    reads *adr into rax.
//    sets rdtsc_t to the duration of the memory access (in cpu cycles)

#define NUMBER_SAMPLES 1000

uint64_t data;

int main() {
  uint64_t rdtsc_a1, rdtsc_d1, rdtsc_a2, rdtsc_d2, rdtsc_t;
  uint64_t i = 0;
  uint64_t samples_cached[NUMBER_SAMPLES];
  uint64_t samples_uncached[NUMBER_SAMPLES];
  void* adr = (void*)&data;
  
  
  maccess(adr);
  for(i = 0; i < NUMBER_SAMPLES; ++i) {
    maccess(adr);
    samples_cached[i] = rdtsc_t;
  }
  
  for(i = 0; i < NUMBER_SAMPLES; ++i) {
    __asm volatile ("clflush 0(%0)\n" : : "r" (adr) : "rax");
    maccess(adr);
    samples_uncached[i] = rdtsc_t;
  }
  
  
  printf("cached,uncached\n");
  for(i=0; i < NUMBER_SAMPLES; ++i) {
    printf("%lu,%lu\n", samples_cached[i], samples_uncached[i]);
  }
}
