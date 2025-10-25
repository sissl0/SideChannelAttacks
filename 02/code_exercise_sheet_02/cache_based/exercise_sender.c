#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <dummy.h>
#include <dlfcn.h>

#define LINE_SIZE      64ul
#define L1_NUM_SETS    64ul
#define STRIDE         (LINE_SIZE * L1_NUM_SETS) // 4096
#define EVSET_LEN      16ul                      // > L1 ways (typ. 8)

static inline uint64_t now_us(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return 1000000ull * (uint64_t)tv.tv_sec + (uint64_t)tv.tv_usec;
}

// PLT -> GOT -> echte Adresse (x86_64 SysV, jmp *gotp(%rip))
static void* get_f_at_got(void* f_at_plt) {
  unsigned char* p = (unsigned char*)f_at_plt;
  if (p[0] == 0xff && p[1] == 0x25) {        // jmpq *disp32(%rip)
    int32_t rel = *(int32_t*)(p + 2);
    void** gotp = (void**)(p + 6 + rel);     // plt+6 + rel32
    return *gotp;                            // Ziel aus GOT
  }
  return NULL;
}

int main(int argc, char** argv) {
  const char* bits = (argc > 1) ? argv[1] : "101100111000";
  uint64_t slot_us = (argc > 2) ? (uint64_t)strtoull(argv[2], NULL, 10) : 200ul;

  // Adresse aus der geladenen lib holen
  void* handle = dlopen("libdummy.so", RTLD_NOLOAD | RTLD_LAZY);
  if (!handle) handle = dlopen("libdummy.so", RTLD_LAZY);
  void* adr = dlsym(handle, "nopper");

  // Bits 6..11 (Set-Index) der VA beibehalten, 0..5 nullen (64B-Ausrichtung)
  size_t offset_in_page = (((uintptr_t)adr & 0xFFFu) & ~((uintptr_t)(LINE_SIZE - 1)));

  // Eviction-Set aufbauen, das denselben Set trifft
  void* buf = NULL;
  if (posix_memalign(&buf, 4096, STRIDE * (EVSET_LEN + 1)) != 0) return 1;
  volatile unsigned char* base = (volatile unsigned char*)buf + offset_in_page;
  volatile unsigned char* evset[EVSET_LEN];
  for (size_t i = 0; i < EVSET_LEN; ++i) evset[i] = base + i * STRIDE;

  uint64_t t_base = now_us() + 200;
  for (size_t b = 0; bits[b] != '\0'; ++b) {
    uint64_t t_start = t_base + b * slot_us;
    while (now_us() < t_start) { __asm__ __volatile__("pause"); }
    uint64_t t_end = t_start + slot_us;

    if (bits[b] == '1') {
      do {
        for (size_t r = 0; r < 8; ++r)
          for (size_t i = 0; i < EVSET_LEN; ++i) { (void)*evset[i]; }
      } while (now_us() < t_end);
    } else {
      while (now_us() < t_end) { __asm__ __volatile__("pause"); }
    }
  }
  return 0;
}
