// Compile: clang -o flush_close flush_close.c 

#include <stdint.h>
#include <unistd.h>

int main() {
  // Compute close@libc via dark magick.
  close(-1);
  void* adr = *(void**)((uint64_t)close + 6 + (((*(uint64_t*)close) >> 0x10) & 0xFFFFFFFF));
  
  while(1) {
    asm volatile ("clflush (%0)\nmfence" :: "r"(adr));
  }
}
