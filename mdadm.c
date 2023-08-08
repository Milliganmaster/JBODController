#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "mdadm.h"
#include "jbod.h"

int mount=0; //variable(global) to show the status of mount
// Method mounts the system
int mdadm_mount(void) {
	if (mount) { // checks if it is not mounted
	  return -1;
	}
	jbod_operation(JBOD_MOUNT, NULL); //mounts it
	
	mount= 1;
	return 1;	
 }
// Method unmounts the system
int mdadm_unmount(void) { 
	if (mount== 0) {  // checks if it is mounted to unmount
	    return -1;
	}
	jbod_operation(JBOD_UNMOUNT, NULL); // unmounts it
	mount= 0;
	return 1;
 }
 //This function shifts the 32 bytes according to the operation given as per the question guidelines

uint32_t displaybytes(uint32_t cmd, uint32_t disk_id, uint32_t block_id) {
  uint32_t format = 0;

  // Formatting the bits for the command
  format |= (cmd & 0x3F) << 14;

  // Formatting the bits for the block ID
  format |= (block_id & 0xFF) << 20;

  // Formatting the bits for the disk ID
  format |= (disk_id & 0xF) << 28;

  return format;
}
// method checks all the cases mentioned in the test_read_invalid_parameters() test
int valid_read(uint32_t start_addr, int read_len, const uint8_t *read_buf) {
  if (mount== 0) {
    return -1;
  }
  else if (read_len > 2048) {
    return -1;
  }
  else if (read_buf == NULL) {
    return -1;
  }
  else if (start_addr+read_len >= JBOD_DISK_SIZE*JBOD_NUM_DISKS) {
    return -1;
  } 
  return 1;
}

 
//This function returns the length of bytes to read when there exist offset
int min(int n, int m)
{
  if (n>m) 
  {
    return m; //Return the size of read_len if the bytes to read is bigger than length required
  }
  else if (n==m) 
  {
    return n; //Return the size of bytes to read if it equal to read_len
  }
  else
  {
    return n; //Return the size of bytes to read if it's less than the length required
  }
}


//This function reads the number of bytes by switching blocks and disks
int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf)  {
  if (read_len == 0) {
    return 0;
  }
  if (valid_read(start_addr, read_len, read_buf) == -1 || read_len > 1024) {
    return -1;
  }

  uint32_t current_addr = start_addr;
  int bytes_read = 0; // variable to track the total bytes read

  while (current_addr < start_addr + read_len) { // Loop until the required length is read
    uint32_t disk_num = current_addr / JBOD_DISK_SIZE; // calculating disk number
    uint32_t block_num = (current_addr % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE; // calculating block number
    uint32_t offset = (current_addr % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE; // calculating offset

    jbod_operation(displaybytes(JBOD_SEEK_TO_DISK, disk_num, 0), NULL); // seeking into the disk
    jbod_operation(displaybytes(JBOD_SEEK_TO_BLOCK, 0, block_num), NULL); // seeking into the block
    uint8_t *tmpreadbuf = (uint8_t *)malloc(JBOD_BLOCK_SIZE * sizeof(uint8_t)); // store read bytes to array created
    jbod_operation(displaybytes(JBOD_READ_BLOCK, 0, 0), tmpreadbuf); // reading and storing in tmpreadbuf

    int remaining_len = read_len - bytes_read; // Calculate the remaining bytes to read

    int copy_len = min((JBOD_BLOCK_SIZE - offset), remaining_len);
    memcpy(read_buf + bytes_read, tmpreadbuf + offset, copy_len); // Copy data from tmpreadbuf to read_buf

    bytes_read += copy_len;
    current_addr += copy_len; // Move to the next address to read

    free(tmpreadbuf);
  }

  return read_len;
}
int mdadm_write(uint32_t start_addr, uint32_t write_len, const uint8_t *write_buf) {
  if (mount == 0) {
    return -1; // Not mounted
  } else if (write_len == 0) {
    return 0; // Nothing to write
  } else if (write_buf == NULL) {
    return -1; // Invalid write buffer
  } else if (write_len > 1024) {
    return -1; // Invalid write length
  } else if (start_addr + write_len > JBOD_DISK_SIZE * JBOD_NUM_DISKS) {
    return -1; // Invalid address range
  }

  uint32_t current_addr = start_addr;
  int bytes_written = 0; // Total bytes written

  while (bytes_written < write_len) {
    uint32_t disk_num = current_addr / JBOD_DISK_SIZE; // Calculating disk number
    uint32_t block_num = (current_addr % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE; // Calculating block number
    uint32_t offset = (current_addr % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE; // Calculating offset

    jbod_operation(displaybytes(JBOD_SEEK_TO_DISK, disk_num, 0), NULL); // Seeking into the disk
    jbod_operation(displaybytes(JBOD_SEEK_TO_BLOCK, 0, block_num), NULL); // Seeking into the block

    // Create a temporary buffer to hold the data to be written
    uint8_t *tmpwritebuf = (uint8_t *)malloc(JBOD_BLOCK_SIZE * sizeof(uint8_t));
    

    // Read the block from the disk into the temporary buffer
    jbod_operation(displaybytes(JBOD_READ_BLOCK, 0, 0), tmpwritebuf);


    jbod_operation(displaybytes(JBOD_SEEK_TO_DISK, disk_num, 0), NULL); // Seeking into the disk
    jbod_operation(displaybytes(JBOD_SEEK_TO_BLOCK, 0, block_num), NULL); // Seeking into the block

    int remaining_len = write_len - bytes_written; // Calculate remaining bytes to write
    int copy_len = min((JBOD_BLOCK_SIZE - offset), remaining_len);

    // Copy data from write_buf to the temporary buffer based on bytes_written and offset
    memcpy(tmpwritebuf + offset, write_buf + bytes_written, copy_len);

    // Write the modified block back to the disk
    jbod_operation(displaybytes(JBOD_WRITE_BLOCK, 0, 0), tmpwritebuf);

    bytes_written += copy_len;
    current_addr += copy_len; // Move to the next address to write

    free(tmpwritebuf);
  }

  return write_len;
}
