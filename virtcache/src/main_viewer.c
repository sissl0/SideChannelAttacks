#include <virtcache.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <error.h>
#include <signal.h>

void initialize_library(void);
void finalize_library(void);
cacheline_t developer_function_get_cacheline(size_t index);

void pretty_print_cache() {
  printf("Virtcache Viewer (Fully Associative)\n");
  printf("------------------------------------------------------------\n");
  printf(" Slot  |       Tag        | Flags | Data (Hex) | Data (Char)\n");
  printf("-------+------------------+-------+------------+------------\n");

  for (size_t i = 0; i < VC_SIZE; ++i) {
    cacheline_t line = developer_function_get_cacheline(i);
    
    printf(" %5zu | %16lx |   %d   |     %02x     |      %c\n", 
           i, 
           line.tag, 
           line.flags, 
           line.data, 
           (line.data >= 32 && line.data <= 126) ? line.data : '.');
  }
  printf("------------------------------------------------------------\n");
}

void clear_printed_cache(void) {
  // Move cursor up by: 4 header lines + VC_SIZE data lines + 1 footer line
  printf("\r\033[%zuA", VC_SIZE + 5);
}

int main(void) {
  initialize_library();
  pretty_print_cache();
  while(1) {
    usleep(10000); // Wait 10 milliseconds
    clear_printed_cache();
    pretty_print_cache();
  }
  finalize_library();
}

