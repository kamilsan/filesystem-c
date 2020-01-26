#pragma once

#include <stdint.h>

#define INODE_FILE 0
#define INODE_DIR 1
#define MAX_FILENAME_LEN 20

struct heap;

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
  struct heap* mem;
} filesystem;

filesystem* create_filesystem(const char* filename, uint64_t size);
filesystem* filesystem_open(const char* filename);
void filesystem_print_tree(filesystem* fs);
int filesystem_add_file(filesystem* fs, const char* filename, const char* source);
int filesystem_get_file(filesystem* fs, const char* filename, const char* destination);
int filesystem_make_directory(filesystem* fs, const char* filename);
int filesystem_copy_file(filesystem* fs, const char* src, const char* dst);
int filesystem_delete_file(filesystem* fs, const char* file);
void destroy_filesystem(filesystem** fs);
