#pragma once

#include "heap.h"

#define INODE_FILE 0
#define INODE_DIR 1
#define MAX_FILENAME_LEN 20

typedef struct inode
{
  int flag;
  char name[MAX_FILENAME_LEN];
  uint64_t size;
  uint64_t data_ptr;
  uint64_t next_ptr;
} inode;

typedef struct filesystem
{
  char* file;
  uint64_t size;
  uint64_t used;
  heap* mem;
} filesystem;

filesystem* create_filesystem(const char* filename, uint64_t size);
void filesystem_print_tree(filesystem* fs);
void filesystem_add_file(filesystem* fs, const char* filename, const char* source);
void filesystem_get_file(filesystem* fs, const char* filename, const char* destination);
void filesystem_make_directory(filesystem* fs, const char* filename);
void destroy_filesystem(filesystem** fs);
