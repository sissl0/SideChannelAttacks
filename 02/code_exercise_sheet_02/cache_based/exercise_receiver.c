#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/time.h>
#include <dlfcn.h>

#include <dummy.h>

#include <common.h>
//  Defines:
//  maccess(adr)
//    needs variables: uint64_t rdtsc_a1, rdtsc_d1, rdtsc_a2, rdtsc_d2, rdtsc_t;
//    reads *adr into rax.
//    sets rdtsc_t to the duration of the memory access (in cpu cycles)

// Number of samples per measuring interval
#define NUMBER_SAMPLES (100ul)
// Predefined threshold (that is used if the user does not define one)
#define predefined_threshold 200


int comp_uint64_t(const void* x, const void* y) {
  return ( *(uint64_t*)x < *(uint64_t*)y ? -1 : 1 );
}

// PLT -> GOT -> echte Adresse (x86_64 SysV, jmp *gotp(%rip))
static void* get_f_at_got(void* f_at_plt) {
  unsigned char* p = (unsigned char*)f_at_plt;
  if (p[0] == 0xff && p[1] == 0x25) {        // jmpq *disp32(%rip)
    int32_t rel = *(int32_t*)(p + 2);
    void** gotp = (void**)(p + 6 + rel);     // plt+6 + rel32
    return *gotp;                            // Ziel aus GOT (nach Lazy Binding: &nopper@lib)
  }
  return NULL;
}

int main(int argc, char** argv) {
  // Needed by the macro maccess
  uint64_t rdtsc_a1, rdtsc_d1, rdtsc_a2, rdtsc_d2, rdtsc_t;

  // Lib laden/finden und echte Symboladresse holen (nicht PLT)
  void* handle = dlopen("libdummy.so", RTLD_NOLOAD | RTLD_LAZY);
  if (!handle) handle = dlopen("libdummy.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "dlopen libdummy.so failed: %s\n", dlerror());
    return 1;
  }
  void* adr = dlsym(handle, "nopper");
  if (!adr) {
    fprintf(stderr, "dlsym nopper failed: %s\n", dlerror());
    return 1;
  }
  Dl_info info = {0};
  if (dladdr(adr, &info) && info.dli_fname) {
    fprintf(stderr, "lib addr=%p from %s\n", adr, info.dli_fname);
  }
  
  // Variables used by the sampling
  uint64_t samples[NUMBER_SAMPLES];
  uint64_t num_miss = 0;
  uint64_t num_hit = 0;
  
  // Variables used for a single measuring interval
  struct timeval tv;
  uint64_t interval_len = 50; // Microseconds, that a single measuring interval should take
  uint64_t t_0 = 0;
  uint64_t t_1 = 0;
  
  // User defined threshold (or predefined threshold)
  uint64_t threshold = 0;
  if (argc > 1) {
    threshold = atoi(argv[1]) >= 0 ? (uint64_t)(atoi(argv[1])) : 0;
  } else {
    threshold = predefined_threshold;
  }
  
  
  while (1) {
    //
    // MEASURE CACHE TIMINGS
    //
    
    //
    // Reset Counter and start interval-timer (which is not used in measuring the cache timings)
    //
    gettimeofday(&tv,NULL);
    t_0 = 1000000l * (uint64_t)tv.tv_sec + (uint64_t)tv.tv_usec;
    
    //
    // Approximate the median duration by sampling
    //
    for(uint64_t i = 0; i < NUMBER_SAMPLES; ++i) {
      maccess(adr);
      samples[i] = rdtsc_t;
    }
    
    
    //
    // EVALUATE MEASURES
    //
    // Compute Median
    qsort(samples, NUMBER_SAMPLES, sizeof(uint64_t), comp_uint64_t);
    // Use (weighted) Median and minimum to determine hits and misses
    num_hit  += (samples[NUMBER_SAMPLES/2] <= threshold);
    num_miss += (samples[NUMBER_SAMPLES/2] >  threshold);
    
    
    //
    // WAIT TILL THE MEASURING INTERVAL ENDS
    //
    // Get Time in microseconds and wait if needed.
    gettimeofday(&tv,NULL);
    t_1 = 1000000l * (uint64_t)tv.tv_sec + (uint64_t)tv.tv_usec;
    if ( t_0 + interval_len > t_1 ) { // Handle overflows correctly
      usleep( (__useconds_t) (interval_len - t_1 + t_0 ));
    }
    
    
    //
    // PRINT STATUS INFO
    //
    if ((num_hit + num_miss) % 1000 == 0) {
      printf("Hits: %lu   Misses: %lu   Some Duration %lu   \r", num_hit, num_miss, samples[NUMBER_SAMPLES/2]);
      fflush(stdout);
    }
  }
}
