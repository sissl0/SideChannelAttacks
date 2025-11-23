#include <stdint.h>
#include <virtcache.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h> // Added for time() to seed srand

#define getflagbit(f,i) (((f) & (1<<i)) >> i)

// Initialize to -1 to indicate they are not open
int cache_fd = -1;
int shm_fd = -1;
cacheline_t* virtual_cache = NULL;
uint8_t* virtual_cache_shm = NULL;
static int lib_initialized = 0; // Guard against double initialization

void print_version(void) {
  printf("Virtcache Build Info:\n");
  printf("  Compiled at: %s: %s\n", __DATE__, __TIME__);
  printf("  Git Version: %s\n", PROJECT_VERSION);
}

void flush(void* address) {
  // Fully Associative: We must search the entire cache for the tag
  int found = 0;
  if (virtual_cache == NULL) return; // Safety check

  for (size_t i = 0; i < VC_SIZE; i++) {
    cacheline_t* line = &virtual_cache[i];
    
    if ((line->flags & VC_USED) && (line->tag == (uint64_t)address)) {
      // Hit in cache
      usleep(VC_FLUSH_USED_DURATION);
      
      // Invalidate
      line->tag = 0;
      line->flags = 0;
      line->data = 0;
      virtual_cache_shm[i] = 0; // Clear data representation as well
      
      // Sync changes
      msync(&virtual_cache[i], sizeof(cacheline_t), MS_ASYNC);
      msync(&virtual_cache_shm[i], sizeof(uint8_t), MS_ASYNC);
      
      found = 1;
      break; // Found and flushed, stop searching
    }
  }

  if (!found) {
    usleep(VC_FLUSH_CLEAN_DURATION);
  }
}

void write_to_cached_shm(void* address, const uint8_t data) {
  if (virtual_cache == NULL) return; // Safety check

  // 1. Search for the address in the cache (Hit check)
  for (size_t i = 0; i < VC_SIZE; i++) {
    if ((virtual_cache[i].flags & VC_USED) && (virtual_cache[i].tag == (uint64_t)address)) {
      // HIT
      usleep(VC_LOAD_CACHED_DURATION);
      
      // Update data
      virtual_cache[i].data = data;
      virtual_cache_shm[i] = data;
      
      // Sync changes
      msync(&virtual_cache[i], sizeof(cacheline_t), MS_SYNC);
      msync(&virtual_cache_shm[i], sizeof(uint8_t), MS_SYNC);
      return;
    }
  }

  // 2. MISS - We need to insert the data
  usleep(VC_LOAD_UNCACHED_DURATION);

  size_t victim_index = 0;
  int found_empty = 0;

  // Strategy: Look for an empty slot first
  for (size_t i = 0; i < VC_SIZE; i++) {
    if (!(virtual_cache[i].flags & VC_USED)) {
      victim_index = i;
      found_empty = 1;
      break;
    }
  }

  // Strategy: Random Replacement if cache is full
  if (!found_empty) {
    victim_index = rand() % VC_SIZE;
  }
  
  // Set tag, flag, and data at the chosen index
  cacheline_t cacheline;
  cacheline.tag = (uint64_t)address;
  cacheline.flags = VC_USED; // Reset other flags, ensure USED is set
  cacheline.data = data;
  
  virtual_cache[victim_index] = cacheline;
  virtual_cache_shm[victim_index] = data;

  // Force synchronization so viewer sees it immediately
  msync(&virtual_cache[victim_index], sizeof(cacheline_t), MS_SYNC);
  msync(&virtual_cache_shm[victim_index], sizeof(uint8_t), MS_SYNC);
}

void write_random_to_cached_shm(void* address) {
  write_to_cached_shm(address, (uint8_t)rand());
}

uint8_t read_from_cached_shm(void* address) {
  uint8_t ret = 0;
  if (virtual_cache == NULL) return 0;

  // Peek to see if data exists to return correct value
  for (size_t i = 0; i < VC_SIZE; i++) {
    if ((virtual_cache[i].flags & VC_USED) && (virtual_cache[i].tag == (uint64_t)address)) {
      ret = virtual_cache[i].data;
      break;
    }
  }

  // Simulate delay and cache update logic by calling write
  // If it was a miss (ret=0), this will install '0' into the cache
  write_to_cached_shm(address, ret);
  return ret;
}

timestamp get_timestamp(void) {
  timestamp t;
  gettimeofday(&t, NULL);
  return t;
}

void print_difference(timestamp start, timestamp end) {
  printf("%lld", ((end.tv_sec - start.tv_sec) * 1000000000LL) + (end.tv_usec - start.tv_usec));
}

void initialize_library(void) {
  if (lib_initialized) return; // Prevent double initialization
  lib_initialized = 1;

  print_version();
  
  // Seed random number generator for Random Replacement policy
  srand(time(NULL));

  // Open file descriptors
  shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
  if (shm_fd == -1) {
    perror("File Descriptor for the shared memory cannot be opened.");
    return;
  }
  
  cache_fd = shm_open(VC_NAME, O_RDWR | O_CREAT, 0666);
  if (cache_fd == -1) {
    perror("File Descriptor for the virtual cache cannot be opened.");
    return;
  }
  
  // Set size
  if (ftruncate(shm_fd, VC_SIZE*sizeof(uint8_t)) == -1) {
    perror("File Descriptor for the shared memory cannot be used to set the size.");
    return;
  }
  
  if (ftruncate(cache_fd, VC_SIZE*sizeof(cacheline_t)) == -1) {
    perror("File Descriptor cannot be used to set the size.");
    return;
  }

  // Map file into my address space
  virtual_cache_shm = (uint8_t*)mmap(NULL, VC_SIZE*sizeof(uint8_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (virtual_cache_shm == MAP_FAILED) {
    perror("Shared Memory cannot be included to my Memory Map.");
    virtual_cache_shm = NULL;
    return;
  }
  
  virtual_cache = (cacheline_t*)mmap(NULL, VC_SIZE*sizeof(cacheline_t), PROT_READ | PROT_WRITE, MAP_SHARED, cache_fd, 0);
  if (virtual_cache == MAP_FAILED) {
    perror("Virtual Cache cannot be included to my Memory Map.");
    virtual_cache = NULL;
    return;
  }
}

void finalize_library(void) {
  if (!lib_initialized) return;
  
  if (virtual_cache_shm != NULL && virtual_cache_shm != MAP_FAILED) {
    munmap(virtual_cache_shm, VC_SIZE * sizeof(uint8_t));
    virtual_cache_shm = NULL;
  }
  
  if (virtual_cache != NULL && virtual_cache != MAP_FAILED) {
    munmap(virtual_cache, VC_SIZE * sizeof(cacheline_t));
    virtual_cache = NULL;
  }

  // Only close if valid
  if (shm_fd != -1) {
    if (close(shm_fd) == -1) {
      perror("File Descriptor for the shared memory cannot be closed.");
    }
    shm_fd = -1;
  }

  if (cache_fd != -1) {
    if (close(cache_fd) == -1) {
      perror("File Descriptor for the virtual cache cannot be closed.");
    }
    cache_fd = -1;
  }
  
  lib_initialized = 0;
}

void developer_function_unlink_cache(void) {
  if (shm_unlink(SHM_NAME) == -1) {
    perror("Shared Memory cannot be unlinked");
    return;
  }
  
  if (shm_unlink(VC_NAME) == -1) {
    perror("Virtual Cache cannot be unlinked");
    return;
  }
}

cacheline_t developer_function_get_cacheline(size_t offset) {
  if (virtual_cache && offset < VC_SIZE) {
    return virtual_cache[offset];
  }
  cacheline_t empty = {0};
  return empty;
}

void developer_function_set_cacheline(size_t offset, cacheline_t c) {
  virtual_cache[offset%VC_SIZE] = c;
  virtual_cache_shm[offset%VC_SIZE] = c.data;
}

void developer_function_set_cacheline_without_changing_shm(size_t offset, cacheline_t c) {
  virtual_cache[offset%VC_SIZE] = c;
}

