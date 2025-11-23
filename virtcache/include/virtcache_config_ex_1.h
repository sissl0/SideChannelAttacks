//
// Cacheconfig
//
// Duration of a load if the cache is loaded (and the tags agree)
#define VC_LOAD_CACHED_DURATION     40000UL
// Duration of a load if the cache is invalid
#define VC_LOAD_CLEAN_DURATION     70000UL
// Duration of a load if the cache is not invvalid (but the tags do not agree)
#define VC_LOAD_UNCACHED_DURATION  100000UL
// Duration of a flush if the cache is invalidated
#define VC_FLUSH_CLEAN_DURATION      1000UL
// Duration of a flush if the cache is not invalidet
#define VC_FLUSH_USED_DURATION       1000UL
