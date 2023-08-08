# JBOD Disk System and Cache Management

This project simulates a Just a Bunch of Disks (JBOD) management system with cache functionality. The project allows you to perform operations like mounting, unmounting, reading, and writing, alongside cache management tasks like creation, destruction, lookup, update, and insertion.
Table of Contents

  Getting Started
      - Prerequisites
      - Installation
  Usage
      - Disk Operations
      - Cache Operations
      - Tester Script

## Getting Started
### Prerequisites

  GCC Compiler
  Standard libraries: stdio.h, string.h, stdlib.h, assert.h

### Installation

Clone the repository:https://github.com/Milliganmaster/JBODController/


Navigate to the project directory:


cd <project_location>

Compile the code:


gcc -o main mdadm.c cache.c -Wall

## Usage
### Disk Operations

#### Mount: To mount the JBOD system.

int mdadm_mount(void);

#### Unmount: To unmount the JBOD system.

int mdadm_unmount(void);

#### Read: Read data from the JBOD system.

int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf);

#### Write: Write data to the JBOD system.

int mdadm_write(uint32_t start_addr, uint32_t write_len, const uint8_t *write_buf);

### Cache Operations

#### Create: Create a cache with a specified number of entries.

int cache_create(int num_entries);

#### Destroy: Destroy the current cache.

int cache_destroy(void);

#### Lookup: Look up a specific block in the cache.

int cache_lookup(int disk_num, int block_num, uint8_t *buf);

#### Update: Update a cache entry.

void cache_update(int disk_num, int block_num, const uint8_t *buf);

#### Insert: Insert a new entry into the cache.

int cache_insert(int disk_num, int block_num, const uint8_t *buf);

#### Check if Cache is Enabled:

bool cache_enabled(void);

#### Print Cache Hit Rate:

void cache_print_hit_rate(void);

### Tester Script

The project includes a tester script that automates the testing of the provided functions.

#### To run the tests:

./tester

This will execute various test cases and print out the results.
