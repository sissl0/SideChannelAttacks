#include <stdint.h>
#include <virtcache.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <virtcache_config_ex_1.h>
#include <virtcache_config.h>
#include <stdio.h>

static void wait_till_next_three_seconds(struct timespec* tstart_p) {
  struct timespec now, rem = {0,0};
  timespec_get(&now, TIME_UTC);
  time_t next = tstart_p->tv_sec + 3;
  rem.tv_sec = next - now.tv_sec - (now.tv_nsec > 0 ? 1 : 0);
  rem.tv_nsec = (1000000000L - now.tv_nsec) % 1000000000L;
  while (nanosleep(&rem, &rem) && errno == EINTR) {}
  tstart_p->tv_sec = next;
}

int main(void) {
  setvbuf(stdout, NULL, _IONBF, 0);

  void* adr = (void*)0x7;
  size_t bits_to_capture = 13;

  struct timespec tstart = {0,0};
  timespec_get(&tstart, TIME_UTC);
  tstart.tv_nsec = 0;
  tstart.tv_sec = (tstart.tv_sec/3) * 3;

  // --- Kalibrierung ---
  long slow_sum = 0, fast_sum = 0;
  int calib_runs = 50;
  for (int c = 0; c < calib_runs; ++c) {
    flush(adr);
    timestamp a = get_timestamp();
    read_from_cached_shm(adr);
    timestamp b = get_timestamp();
    slow_sum += ((b.tv_sec - a.tv_sec)*1000000L) + (b.tv_usec - a.tv_usec);

    timestamp c1 = get_timestamp();
    read_from_cached_shm(adr);
    timestamp d = get_timestamp();
    fast_sum += ((d.tv_sec - c1.tv_sec)*1000000L) + (d.tv_usec - c1.tv_usec);
  }
  long slow = slow_sum / calib_runs;
  long fast = fast_sum / calib_runs;
  long threshold = (slow + fast) / 2;
  printf("# calib fast=%ld slow=%ld threshold=%ld\n", fast, slow, threshold);
  printf("# Start Victim (within 3s) !\n");

  timespec_get(&tstart, TIME_UTC);
  tstart.tv_nsec = 0;
  tstart.tv_sec = (tstart.tv_sec/3)*3;
  
  printf("# index,delta_us,bit\n");

  for (size_t i = 0; i < bits_to_capture; ++i) {
    flush(adr);
    wait_till_next_three_seconds(&tstart);

    struct timespec tiny = {0, 150 * 1000 * 1000L}; 
    nanosleep(&tiny, NULL);

    timestamp ts = get_timestamp();
    read_from_cached_shm(adr);
    timestamp te = get_timestamp();
    long delta = ((te.tv_sec - ts.tv_sec)*1000000L) + (te.tv_usec - ts.tv_usec);
    
    int bit = (delta < threshold) ? 1 : 0;
    printf("%zu,%ld,%d\n", i, delta, bit);
  }

  return 0;
}
