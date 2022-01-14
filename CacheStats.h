/******************************
 * Copyright 2021, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. Use in source or binary form, with or without
 * modification, is only permitted for academic use in CS 3339 at
 * Texas State University.
 ******************************/
#ifndef __CACHE_STATS_H
#define __CACHE_STATS_H

#include <cstdint>
#include "Debug.h"
using namespace std;

#ifndef CACHE_EN
#define CACHE_EN 1
#endif

#ifndef BLOCKSIZE
#define BLOCKSIZE 32
#endif

#ifndef SETS
#define SETS 8
#endif

#ifndef WAYS
#define WAYS 4
#endif

#ifndef LOOKUP_LATENCY
#define LOOKUP_LATENCY 0
#endif

#ifndef READ_LATENCY
#define READ_LATENCY 30
#endif

#ifndef WRITE_LATENCY
#define WRITE_LATENCY 10
#endif

enum ACCESS_TYPE { LOAD, STORE };

class CacheStats {
  private:
    /* TODO: you probably want to add some member variables here to represent
     * the parts of the cache contents you need to model! */

    int loads;
    int stores;
    int load_misses;
    int store_misses;
    int writebacks;
    int stalls;

    uint32_t index;
    uint32_t currtag;

    int valid[SETS][WAYS];
    int tag[SETS][WAYS];
    int dirty[SETS][WAYS];

    int roundrobin;

  public:
    CacheStats();
    int access(uint32_t, ACCESS_TYPE);
    void printFinalStats();

    int getLoads() { return loads; };
    int getStores() { return stores; };
    int getLoadMiss() { return load_misses; };
    int getStoreMiss() { return store_misses; };
    int getWB() { return writebacks; };
    int getStalls() { return stalls; }
};

#endif
