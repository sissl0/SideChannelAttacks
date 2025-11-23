#include <virtcache.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(void) {
  srand((unsigned) time(NULL));
  uint64_t* adr = (uint64_t*)(uint64_t)rand();
  write_random_to_cached_shm(adr);
  write_random_to_cached_shm(adr);
  write_random_to_cached_shm(adr);
  write_random_to_cached_shm(adr);
  
  //flush(adr);
}
