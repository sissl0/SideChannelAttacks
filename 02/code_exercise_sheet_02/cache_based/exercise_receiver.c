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
#define NUMBER_SAMPLES (1ul) // Prime+Probe: 1 Probe-Summe pro Slot reicht
#define predefined_threshold 3000 // Summe über EVSET_LEN Zugriffe (anpassbar)


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

uint64_t now_us() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return 1000000ull * (uint64_t)tv.tv_sec + (uint64_t)tv.tv_usec;
}

int main(int argc, char** argv) {
  // Needed by the macro maccess
  uint64_t rdtsc_a1, rdtsc_d1, rdtsc_a2, rdtsc_d2, rdtsc_t;

  // Prime+Probe-Parameter
  const uint64_t interval_len = 200; // µs: größer für stabilere Ausrichtung
  const size_t LINE_SIZE = 64, L1_NUM_SETS = 64, STRIDE = 4096, EVSET_LEN = 16;

  // Adresse aus libdummy holen
  void* handle = dlopen("libdummy.so", RTLD_NOLOAD | RTLD_LAZY);
  if (!handle) handle = dlopen("libdummy.so", RTLD_LAZY);
  void* adr = dlsym(handle, "nopper");

  // Gemeinsamer Set: Bits 6..11 übernehmen
  size_t offset_in_page = (((uintptr_t)adr & 0xFFFu) & ~((uintptr_t)(LINE_SIZE - 1)));

  // Eviction-Set anlegen, das diesen Set trifft
  void* buf = NULL;
  if (posix_memalign(&buf, 4096, STRIDE * (EVSET_LEN + 1)) != 0) {
    fprintf(stderr, "alloc failed\n"); return 1;
  }
  volatile unsigned char* base = (volatile unsigned char*)buf + offset_in_page;
  void* evset[16];
  for (size_t i = 0; i < EVSET_LEN; ++i) evset[i] = (void*)(base + i * STRIDE);

  // Threshold konfigurieren (Summe über EVSET_LEN maccess-Zeiten)
  uint64_t threshold = (argc > 1) ? (uint64_t)strtoull(argv[1], NULL, 10) : predefined_threshold;

  // Zähler/Timer
  uint64_t num_miss = 0, num_hit = 0;
  struct timeval tv;
  uint64_t t_0 = 0, t_1 = 0;

  uint64_t slot_idx = 0;
  uint64_t t_base = now_us() + 200; // gleiche Guard wie beim Sender (Receiver zuerst starten)
  while (1) {
    uint64_t t_start = t_base + slot_idx * interval_len;
    while (now_us() < t_start) { __asm__ __volatile__("pause"); }

    // PRIME
    for (size_t r = 0; r < 8; ++r) {
      for (size_t i = 0; i < EVSET_LEN; ++i) { (void)*(volatile unsigned char*)evset[i]; }
    }

    // kurze Pause
    __asm__ __volatile__("pause");

    // PROBE: Summiere Latenzen über Eviction-Set
    uint64_t sum_cycles = 0;
    for (size_t i = 0; i < EVSET_LEN; ++i) {
      maccess(evset[i]);
      sum_cycles += rdtsc_t;
    }

    int evicted = (sum_cycles >= threshold);
    putchar(evicted ? '1' : '0');
    fflush(stdout);

    ++slot_idx;
  }
}
