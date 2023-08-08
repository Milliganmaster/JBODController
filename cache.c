#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cache.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int num_queries = 0;
static int num_hits = 0;
static int fifoIndex = 0;

int cache_create(int num_entries) {
  if (cache != NULL) {
    // Cache already exists, return failure
    return -1;
  }
  
  if (num_entries < 2 || num_entries > 4096) {
    // Invalid number of entries, return failure
    return -1;
  }
  
  cache = malloc(num_entries * sizeof(cache_entry_t));
  if (cache == NULL) {
    // Memory allocation failed, return failure
    return -1;
  }
  
  cache_size = num_entries;
  return 1; // Success
}

int cache_destroy(void) {
  if (cache == NULL) {
    // Cache does not exist, return failure
    return -1;
  }
  
  free(cache);
  cache = NULL;
  cache_size = 0;
  return 1; // Success
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  if (cache == NULL || buf == NULL) {
    // Cache or buffer is not valid, return failure
    return -1;
  }
  
  for (int i = 0; i < cache_size; i++) {
    if (cache[i].valid && cache[i].disk_num == disk_num && cache[i].block_num == block_num) {
      memcpy(buf, cache[i].block, JBOD_BLOCK_SIZE);
      num_queries++;
      num_hits++;
      return 1; // Success, block found in cache
    }
  }
  
  num_queries++;
  return -1; // Failure, block not found in cache
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
  if (cache == NULL || buf == NULL) {
    // Cache or buffer is not valid, return
    return;
  }
  
  for (int i = 0; i < cache_size; i++) {
    if (cache[i].valid && cache[i].disk_num == disk_num && cache[i].block_num == block_num) {
      memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE);
      return; // Entry found and updated
    }
  }
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
    if (cache == NULL || buf == NULL) {
    // Cache or buffer is not valid, return failure
    return -1;
  }
  
  if (disk_num < 0 || disk_num >= JBOD_NUM_DISKS) {
    // Invalid disk number, return failure
    return -1;
  }
  
  if (block_num < 0 || block_num >= JBOD_NUM_BLOCKS_PER_DISK) {
    // Invalid block number, return failure
    return -1;
  }
  
  for (int i = 0; i < cache_size; i++) {
    if (cache[i].valid && cache[i].disk_num == disk_num && cache[i].block_num == block_num) {
      return -1; // Entry already exists in cache, return failure
    }
  }
  
  // Find the least recently used entry (FIFO) for eviction
  int index = fifoIndex;
  fifoIndex = (fifoIndex + 1) % cache_size;
  
  cache[index].valid = true;
  cache[index].disk_num = disk_num;
  cache[index].block_num = block_num;
  memcpy(cache[index].block, buf, JBOD_BLOCK_SIZE);
  
  return 1; // Success
}

bool cache_enabled(void) {
  return cache_size > 2; // Cache is enabled if cache_size is greater than 2
}

void cache_print_hit_rate(void) {
  fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
