// The following macros define the configuration of the
// virtual cache.

// Name of the shared memory file (placed in /dev/shm/ see also shm_open)
#ifndef VC_NAME
#define VC_NAME "virtualcache"
#define SHM_NAME "virtualcache_shm"
#endif
// The cache stores single bytes (i.e., the cache lines size is 1 byte)
// VC_SIZE defines the size of the cache, i.e., the number of bytes
// the cache can store.
#ifndef VC_SIZE
#define VC_SIZE 8
#endif

// Cache Timings (in nanoseconds) in ascending order.
// Reminder: Here, we use usleep and the duration cannot be arbitrarily small.
// For choosing an accurate sleep function and/or timing, consider
//   https://tldp.org/HOWTO/IO-Port-Programming-4.html
// Duration of a load if the cache is loaded (and the tags agree)
#ifndef VC_LOAD_CACHED_DURATION
#define VC_LOAD_CACHED_DURATION     10000UL
#endif
// Duration of a flush if the cache is invalidated
#ifndef VC_FLUSH_CLEAN_DURATION
#define VC_FLUSH_CLEAN_DURATION     50000UL
#endif
// Duration of a flush if the cache is not invalidat
#ifndef VC_FLUSH_USED_DURATION
#define VC_FLUSH_USED_DURATION     250000UL
#endif
// Duration of a load if the cache is invalid
#ifndef VC_LOAD_CLEAN_DURATION
#define VC_LOAD_CLEAN_DURATION    1000000UL
#endif
// Duration of a load if the cache is not invvalid (but the tags do not agree)
#ifndef VC_LOAD_UNCACHED_DURATION
#define VC_LOAD_UNCACHED_DURATION 1200000UL
#endif
