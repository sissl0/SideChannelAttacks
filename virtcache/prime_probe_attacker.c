#include <virtcache.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// Robust synchronization from exercise_1_attacker.c
static void wait_till_next_three_seconds(struct timespec* tstart_p) {
  struct timespec now, rem = {0,0};
  timespec_get(&now, TIME_UTC);
  time_t next = tstart_p->tv_sec + 3;
  rem.tv_sec = next - now.tv_sec - (now.tv_nsec > 0 ? 1 : 0);
  rem.tv_nsec = (1000000000L - now.tv_nsec) % 1000000000L;
  while (nanosleep(&rem, &rem) && errno == EINTR) {}
  tstart_p->tv_sec = next;
}

// Helper to measure time
uint64_t measure_access(void** ev_set, size_t size) {
    timestamp t1 = get_timestamp();
    for (size_t i = 0; i < size; ++i) {
        read_from_cached_shm(ev_set[i]);
    }
    timestamp t2 = get_timestamp();
    return ((t2.tv_sec - t1.tv_sec) * 1000000L) + (t2.tv_usec - t1.tv_usec);
}

int main() {
    initialize_library();
    setvbuf(stdout, NULL, _IONBF, 0);

    void* ev_set[VC_SIZE];
    for (size_t i = 0; i < VC_SIZE; ++i) {
        ev_set[i] = (void*)(0x100 + i);
    }

    printf("Attacker started. Calibrating...\n");


    for (size_t i = 0; i < VC_SIZE; ++i) read_from_cached_shm(ev_set[i]); // Prime
    uint64_t t_fast = measure_access(ev_set, VC_SIZE); // Probe

    for (size_t i = 0; i < VC_SIZE; ++i) read_from_cached_shm(ev_set[i]); // Prime
    write_to_cached_shm((void*)0x7, 0xFF); // Simulate Victim (evicts 1 random line)
    uint64_t t_slow = measure_access(ev_set, VC_SIZE); // Probe (should have 1 miss)

    uint64_t threshold = (t_fast + t_slow) / 2;
    printf("Calibration: Fast=%lu us, Slow=%lu us, Threshold=%lu us\n", t_fast, t_slow, threshold);

    // 3. Synchronization Setup
    struct timespec tstart = {0,0};
    timespec_get(&tstart, TIME_UTC);
    tstart.tv_nsec = 0;
    tstart.tv_sec = (tstart.tv_sec/3) * 3;

    printf("Listening for transmission...\n");

    while(1) {
        // A. PRIME: Fill the cache completely
        // This ensures the cache is full of OUR lines.
        // If the victim's address (0x7) was in the cache, it gets evicted here.
        for (size_t i = 0; i < VC_SIZE; ++i) read_from_cached_shm(ev_set[i]);

        // B. SYNC: Wait for the start of the slot
        wait_till_next_three_seconds(&tstart);

        // C. OFFSET: Sleep to let the victim execute
        struct timespec tiny = {0, 150 * 1000 * 1000L}; // 150ms
        nanosleep(&tiny, NULL);

        // D. PROBE: Measure if our set is still intact
        uint64_t t_meas = measure_access(ev_set, VC_SIZE);

        int bit = (t_meas > threshold) ? 1 : 0;
        printf("Time: %lu us -> Bit: %d\n", t_meas, bit);
    }

    finalize_library();
    return 0;
}