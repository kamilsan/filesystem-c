#pragma once

#include "heap.h"

#define INODE_FILE 0
#define INODE_DIR 1
#define MAX_FILENAME_LEN 20

typedef struct filesystem
{
  char* file;
  uint64_t size;
  uint64_t used;
  heap* mem;
} filesystem;

typedef struct inode
{
  int flag;
  char name[MAX_FILENAME_LEN];
  uint64_t size;
  uint64_t data_ptr;
  uint64_t next_ptr;
} inode;

filesystem* create_filesystem(const char* filename, uint64_t size);
void destroy_filesystem(filesystem** fs);
