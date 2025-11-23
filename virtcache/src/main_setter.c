#include <stdio.h>
#include <stdlib.h>
#include <virtcache.h>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Usage: %s <address> <value>\n", argv[0]);
    return 1;
  }

  size_t address = strtoull(argv[1], NULL, 0);
  int value = atoi(argv[2]);

  initialize_library();

  printf("Writing value %d (0x%02x) to address 0x%zx...\n", value, value, address);
  // The library function now handles the fully associative lookup and replacement
  write_to_cached_shm((void*)address, (uint8_t)value);
  
  printf("Write complete.\n");

  finalize_library();
  return 0;
}
