#include <virtcache.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// We assert, that the starting time is of the form (n*3,0).
// Moreover, we assume that the starting time is in the past.
// Moreover, we assert, that the current time is less than three seconds away from the starting time.
void wait_till_next_three_seconds(struct timespec* tstart_p) {
  struct timespec tstop = {0,0};
  struct timespec rm = {3,0};
  
  // Get Time
  timespec_get(&tstop, TIME_UTC);
  // We substract the elapsed seconds (and substract 1, since start.nsec == 0)
  rm.tv_sec  = rm.tv_sec - (tstop.tv_sec  - tstart_p->tv_sec) - 1;
  // We substract the elapsed nanoseconds (and add 1000000UL, since start.nsec == 0)
  rm.tv_nsec = (rm.tv_nsec - tstop.tv_nsec + 1000000000L)%1000000000L; // It can happen that tstop.tv_nsec == 0
  
  // printf("%lld.%.9ld\n", (long long)rm.tv_sec, rm.tv_nsec);
  
  while (nanosleep(&rm, &rm) && errno == EINTR){
  }
  
  tstart_p->tv_sec += 3;
}


int main(void) {
  setvbuf(stdout, NULL, _IONBF, 0);
  
  char* key = "1101011001011";
  void* shm_adr = (void*)0x7;
  
  struct timespec tstart = {0,0};
  // Get Time
  timespec_get(&tstart, TIME_UTC);
  tstart.tv_nsec = 0;
  tstart.tv_sec = (tstart.tv_sec/3) * 3;
  
  
  for (size_t i = 0; i < strlen(key); ++i) {
    wait_till_next_three_seconds(&tstart);
    if (key[i] == '1') {
      read_from_cached_shm(shm_adr);
      printf("Read\n");
    } else {
      printf("No Read\n");
    }
  }
  
  return 0;
}
