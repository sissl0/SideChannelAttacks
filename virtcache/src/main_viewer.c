#include <virtcache.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <error.h>
#include <signal.h>

// The program pretty prints the cache.


#define getflagbit(f,i) (((f) & (1<<i)) >> i)


void pretty_print_cache(void) {
  cacheline_t c;
  printf(
    "Virtual Cache Belegung\n"
    "\n"
    "+--------+--------------------+----------+-----------+\n"
    "+ Offset |        Tag         |   Flags  |  Belegung |\n");
  for (unsigned int i = VC_SIZE; i-- > 0 ;) {
    c = developer_function_get_cacheline(i);
    printf("+--------+--------------------+----------+-----------+\n");
    printf("|   0x%02x | 0x%016"PRIx64" |        %u |      0x%02X |\n", i, c.tag, getflagbit(c.flags,0), c.data);
  }
  printf("+--------+--------------------+----------+-----------+");
}


void clear_printed_cache(void) {
  printf("\r\033[20A");
}


int main(void) {
  pretty_print_cache();
  while(1) {
    usleep(10000); // Wait 10 milliseconds
    clear_printed_cache();
    pretty_print_cache();
  }
}
