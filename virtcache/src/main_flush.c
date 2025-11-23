#include <stdio.h>
#include <stdlib.h>
#include <virtcache.h>
#include <time.h>
#include <unistd.h>

// The program flushes the cache (without delay).

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <address>\n", argv[0]);
    return 1;
  }

  size_t address = strtoull(argv[1], NULL, 0);

  initialize_library();

  printf("Flushing address 0x%zx...\n", address);
  // The library function now handles the fully associative lookup
  flush((void*)address);
  
  printf("Flush complete.\n");

  finalize_library();
  return 0;
}
