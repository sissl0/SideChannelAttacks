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
#include <stdlib.h>


static void wait_till_next_three_seconds(struct timespec* tstart_p) {
  struct timespec now, rem = {0,0};
  timespec_get(&now, TIME_UTC);
  time_t next = tstart_p->tv_sec + 3;
  rem.tv_sec = next - now.tv_sec - (now.tv_nsec > 0 ? 1 : 0);
  rem.tv_nsec = (1000000000L - now.tv_nsec) % 1000000000L;
  while (nanosleep(&rem, &rem) && errno == EINTR) {}
  tstart_p->tv_sec = next;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <shared_memory_address> <delimiter>\n", argv[0]);
        return 1;
    }
    void* adr = (void*)strtoull(argv[1], NULL, 0);
    char* delimiter = argv[2];
    int delimiter_length = strlen(delimiter);

    setvbuf(stdout, NULL, _IONBF, 0);

    struct timespec tstart = {0,0};
    timespec_get(&tstart, TIME_UTC);
    tstart.tv_nsec = 0;
    tstart.tv_sec = (tstart.tv_sec/3) * 3;


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
    printf("# Start sender!\n");

    timespec_get(&tstart, TIME_UTC);
    tstart.tv_nsec = 0;
    tstart.tv_sec = (tstart.tv_sec/3)*3;
    
    char msg_buf[4096]; 
    int msg_idx = 0;
    memset(msg_buf, 0, sizeof(msg_buf));

    char* window = calloc(delimiter_length + 1, sizeof(char));
    
    int found_start = 0;

    printf("# Waiting for transmission...\n");

    while (1) {
        flush(adr);
        wait_till_next_three_seconds(&tstart);

        struct timespec tiny = {0, 150 * 1000 * 1000L}; 
        nanosleep(&tiny, NULL);

        timestamp ts = get_timestamp();
        read_from_cached_shm(adr);
        timestamp te = get_timestamp();
        long delta = ((te.tv_sec - ts.tv_sec)*1000000L) + (te.tv_usec - ts.tv_usec);
        
        int bit = (delta < threshold) ? 1 : 0;
        char bit_char = bit ? '1' : '0';

        memmove(window, window + 1, delimiter_length - 1);

        window[delimiter_length - 1] = bit_char;
        window[delimiter_length] = '\0';



        if (!found_start) {
            if (strcmp(window, delimiter) == 0) {
                found_start = 1;

                memset(window, 0, delimiter_length);
                printf("\n# Start delimiter found. Recording message...\n");
            }
        } else {
            if (msg_idx < sizeof(msg_buf) - 1) {
                msg_buf[msg_idx++] = bit_char;
            } else {
                printf("\nError: Message buffer overflow\n");
                break;
            }

            if (strcmp(window, delimiter) == 0) {

                msg_buf[msg_idx - delimiter_length] = '\0';
                
                printf("\n# End delimiter found.\n");
                printf("Captured Message: %s\n", msg_buf);
                break;
            }
        }
    }

    free(window);
    return 0;
}
