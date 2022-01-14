/******************************
 * Submitted by: enter your first and last name and net ID
 * CS 3339 - Spring 2021, Texas State University
 * Project 5 Data Cache
 * Copyright 2021, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. Use in source or binary form, with or without
 * modification, is only permitted for academic use in CS 3339 at
 * Texas State University.
 ******************************/
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "CacheStats.h"
using namespace std;

CacheStats::CacheStats() {
  cout << "Cache Config: ";
  if(!CACHE_EN) {
    cout << "cache disabled" << endl;
  } else {
    cout << (SETS * WAYS * BLOCKSIZE) << " B (";
    cout << BLOCKSIZE << " bytes/block, " << SETS << " sets, " << WAYS << " ways)" << endl;
    cout << "  Latencies: Lookup = " << LOOKUP_LATENCY << " cycles, ";
    cout << "Read = " << READ_LATENCY << " cycles, ";
    cout << "Write = " << WRITE_LATENCY << " cycles" << endl;
  }

  loads = 0;
  stores = 0;
  load_misses = 0;
  store_misses = 0;
  writebacks = 0;
  roundrobin = 0;

  /* TODO: your code to initialize your datastructures here */
}

int CacheStats::access(uint32_t addr, ACCESS_TYPE type) {
  if(!CACHE_EN) { // cache is turned off
    return (type == LOAD) ? READ_LATENCY : WRITE_LATENCY;
  }

  index = (addr >> 5) & 0x8;
  currtag = addr >> 8;
  int currstalls = 0;

  for(int i = 0; i < WAYS; i++)
  {
      if(valid[index][i]) //valid
      {
        if(tag[index][i] == currtag) //hit
        {
          if(type == STORE)
          {
            tag[index][i] = currtag;
            dirty[index][i] = 1;
            writebacks++;
            stalls+=30;
            stores++;
          }
          else if(type == LOAD)
            loads++;
          break;
        }
        else //valid but a miss
        {
          if(dirty[index][i])
            currstalls+=10;
          tag[index][i] = currtag;
          valid[index][roundrobin] = 1;
          writebacks++;
          if(roundrobin == 3)
            roundrobin = 0;
          else
            roundrobin++;
          if(type == STORE)
          {
            store_misses++;
            dirty[index][i] = 1;
            writebacks++;
            stalls+=30;
          }
          else if(type == LOAD)
            load_misses++;
          break;
        }
      }
      else //not valid miss
      {
        if(dirty[index][i])
            currstalls+=10;
        tag[index][roundrobin] = currtag;
        valid[index][roundrobin] = 1;
        writebacks++;
        if(roundrobin == 3)
          roundrobin = 0;
        else
          roundrobin++;
        if(type == STORE)
          {
            store_misses++;
            dirty[index][i] = 1;
            writebacks++;
            stalls+=30;
          }
          else if(type == LOAD)
            load_misses++;
        break;
      }
  }
  stalls += currstalls;
  return currstalls;
}

void CacheStats::printFinalStats() {
  /* TODO: your code here "drain" the cache of writebacks */

  int accesses = loads + stores;
  int misses = load_misses + store_misses;
  cout << "Accesses: " << accesses << endl;
  cout << "  Loads: " << loads << endl;
  cout << "  Stores: " << stores << endl;
  cout << "Misses: " << misses << endl;
  cout << "  Load misses: " << load_misses << endl;
  cout << "  Store misses: " << store_misses << endl;
  cout << "Writebacks: " << writebacks << endl;
  cout << "Hit Ratio: " << fixed << setprecision(1) << 100.0 * (accesses - misses) / accesses;
  cout << "%" << endl;
}
