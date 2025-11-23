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


#define getflagbit(f,i) (((f) & (1<<i)) >> i)

int cache_fd = 0;
int shm_fd = 0;
cacheline_t* virtual_cache = NULL;
uint8_t* virtual_cache_shm = NULL;


void print_version(void) {
  printf("Virtcache Build Info:\n");
  printf("  Compiled at: %s: %s\n", __DATE__, __TIME__);
  printf("  Git Version: %s\n", PROJECT_VERSION);
}


void flush(void* address) {
  size_t offset = ((uint64_t)address) % VC_SIZE;
  cacheline_t cacheline = virtual_cache[offset];
  if (cacheline.flags & VC_USED && cacheline.tag == (uint64_t)address) {
    usleep(VC_FLUSH_USED_DURATION);
  } else {
    usleep(VC_FLUSH_CLEAN_DURATION);
  }
  cacheline.tag = 0;
  cacheline.flags = 0;
  cacheline.data = 0;
  virtual_cache[offset] = cacheline;
}


void write_to_cached_shm(void* address, const uint8_t data) {
  size_t offset = ((uint64_t)address) % VC_SIZE;
  cacheline_t cacheline = virtual_cache[offset];
  
  // Process durations
  if (cacheline.flags & VC_USED) {
    if (cacheline.tag == (uint64_t)address) {
      usleep(VC_LOAD_CACHED_DURATION);
    } else {
      usleep(VC_LOAD_UNCACHED_DURATION);
    }
  } else {
    usleep(VC_LOAD_CLEAN_DURATION);
  }
  
  // Set tag and flag accordingly
  cacheline.tag = (uint64_t)address;
  cacheline.flags |= VC_USED;
  cacheline.data = data;
  
  virtual_cache[offset] = cacheline;
  virtual_cache_shm[offset] = data;
}


void write_random_to_cached_shm(void* address) {
  write_to_cached_shm(address, (uint8_t)rand());
}


uint8_t read_from_cached_shm(void* address) {
  size_t offset = ((uint64_t)address) % VC_SIZE;
  uint8_t ret = virtual_cache_shm[offset];
  // Simulate delay by writing (instead of duplicating the C-Code that simulated the delay)
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
  print_version();
  
  // Open file descriptors
  shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
  if (shm_fd == -1) {
    perror("File Descriptor for the shared memory cannot be opened.");
    return;
  }
  
  cache_fd = shm_open(VC_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
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
    return;
  }
  
  virtual_cache = (cacheline_t*)mmap(NULL, VC_SIZE*sizeof(cacheline_t), PROT_READ | PROT_WRITE, MAP_SHARED, cache_fd, 0);
  if (virtual_cache == MAP_FAILED) {
    perror("Virtual Cache cannot be included to my Memory Map.");
    return;
  }
}


void finalize_library(void) {
  // Unmapping the Shared Memory and closing the File Descriptor are done automatically, i.e., the following is not needed.
  if (munmap(virtual_cache_shm, VC_SIZE*sizeof(uint8_t)) == -1) {
    perror("Shared Memory cannot be removed from my Memory Map.");
    return;
  }
  
  if (munmap(virtual_cache, VC_SIZE*sizeof(cacheline_t)) == -1) {
    perror("Virtual Cache cannot be removed from my Memory Map.");
    return;
  }
  
  if (close(shm_fd) == -1) {
    perror("File Descriptor for the shared memory cannot be closed.");
    return;
  }
  
  if (close(cache_fd) == -1) {
    perror("File Descriptor for the virtual cache cannot be closed.");
    return;
  }
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
  return virtual_cache[offset%VC_SIZE];
}


void developer_function_set_cacheline(size_t offset, cacheline_t c) {
  virtual_cache[offset%VC_SIZE] = c;
  virtual_cache_shm[offset%VC_SIZE] = c.data;
}


void developer_function_set_cacheline_without_changing_shm(size_t offset, cacheline_t c) {
  virtual_cache[offset%VC_SIZE] = c;
}

