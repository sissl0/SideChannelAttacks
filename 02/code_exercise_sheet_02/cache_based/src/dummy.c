#include "dummy.h"

void nopper() {
  __asm volatile (REP12("nop\n"));
}
