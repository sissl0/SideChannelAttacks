#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <dummy.h>
#include <dlfcn.h>

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

int main() {
  // 1) Lazy Binding auslösen
  nopper();

  // 2) Echte Lib-Adresse bestimmen
  void* adr = get_f_at_got((void*)nopper);
  if (!adr) adr = dlsym(RTLD_DEFAULT, "nopper"); // Fallback
  Dl_info info = {0};
  if (dladdr(adr, &info) && info.dli_fname) {
    fprintf(stderr, "lib addr=%p from %s\n", adr, info.dli_fname);
  }

  // 3) Flush+Reload: Sender hält die Zeile kalt (sendet effektiv '0')
  while (1) {
    __asm volatile("clflush 0(%0)\n" : : "r"(adr) : "rax");
  }
}
