#pragma once

#define maccess(adr) \
  __asm volatile ("rdtsc" : "=a" (rdtsc_a1), "=d" (rdtsc_d1)); \
  __asm volatile ("mfence"); \
  __asm volatile ("movq (%0), %%rax\n" : : "c" (adr) : "rax"); \
  __asm volatile ("mfence"); \
  __asm volatile ("rdtsc" : "=a" (rdtsc_a2), "=d" (rdtsc_d2)); \
  rdtsc_t = ((rdtsc_d2<<32) - (rdtsc_d1<<32)) | (rdtsc_a2 - rdtsc_a1); \

#define flush(adr) \
  __asm volatile ("rdtsc" : "=a" (rdtsc_a1), "=d" (rdtsc_d1)); \
  __asm volatile ("mfence"); \
  __asm volatile ("clflush 0(%0)\n" : : "c" (adr) : "rax"); \
  __asm volatile ("mfence"); \
  __asm volatile ("rdtsc" : "=a" (rdtsc_a2), "=d" (rdtsc_d2)); \
  rdtsc_t = ((rdtsc_d2<<32) - (rdtsc_d1<<32)) | (rdtsc_a2 - rdtsc_a1); \

#define min(x,y) \
  ((x) < (y) ? (x) : (y))
