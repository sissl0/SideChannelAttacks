#pragma once

#include <virtcache_config_ex_1.h>
#include <virtcache_config.h>

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

// This library implements a directly mapped cache.

// This is the of the function giving us a time stamp
typedef struct timeval timestamp;

// A cache line (of size one byte) consists of
// - the tag
// - the value (since the cache line stores only a single byte)
// - and the flag (right now: cache is loaded or invalid)
typedef struct {
  uint64_t tag;
  uint8_t  data;
  uint8_t  flags;
} cacheline_t;

// The flag telling whether the cache is loaded or invalid
#define VC_USED 0x01

// Print version to stdout
void print_version(void);

// Write a value into cached shared memory
void write_to_cached_shm(void* address, const uint8_t data);

// Write some random value into the shared memory
void write_random_to_cached_shm (void* address);

// Read value from the shared memory
uint8_t read_from_cached_shm(void* address);

// The flush instruction
void flush(void* address);

// Get the current timestamp
timestamp get_timestamp(void);

// Print the difference of a start and end timestamp (for measureing)
void print_difference(timestamp start, timestamp end);

// The library constructor and destructor.
// It is used to initialize (and finalize) the shared memory location.
// In practice, this is needed so the user only has to dynamically link the library
// and the remaining parts are automatic.
void __attribute__ ((constructor)) initialize_library(void);
void __attribute__ ((destructor)) finalize_library(void);

// Here are some "undocumented" developer / debug functions.
// For the end user, they are not important  if the "cache viewer program".

// The Virtual Cache File in /dev/shm/VC_NAME is not cleaned up automatically on some systems.
// However, you can unlink the Virtual Cache File by hand.
// Beware, unlinking the Virtual Cache File might result in unexpected behaviour if you do not know how
// the creation, opening and closing of files is handled.
void developer_function_unlink_cache(void);

// The cache is usually not directly readable.
// For debugging purposes, one can read a given cacheline.
cacheline_t developer_function_get_cacheline(size_t offset);

// The cache is usually not directly writable.
// For debugging purposes, one can write a given cacheline.
// This writes the data also to the shared memory.
void developer_function_set_cacheline(size_t offset, cacheline_t c);

// The cache is usually not directly writable.
// For debugging purposes, one can write a given cacheline.
// This does not write the data also to the shared memory.
void developer_function_set_cacheline_without_changing_shm(size_t offset, cacheline_t c);
