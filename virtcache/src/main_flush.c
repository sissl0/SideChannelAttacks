#include <virtcache.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// The program flushes the cache (without delay).


int main(void) {
  cacheline_t c = {0,0,0};
  for (uint64_t i = 0; i < VC_SIZE; ++i) {
    developer_function_set_cacheline(i, c);
  }
}
