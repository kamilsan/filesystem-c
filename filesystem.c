#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filesystem.h"

filesystem* create_filesystem(const char* filename, uint64_t size)
{
  FILE* fp = fopen(filename, "wb");
  void* data = calloc(1, size);
  fwrite(data, size, 1, fp);
  fclose(fp);
  free(data);

  filesystem* fs = (filesystem*)malloc(sizeof(filesystem));
  size_t filename_len = strlen(filename) + 1;
  fs->file = (char*)malloc(filename_len*sizeof(char));
  strcpy(fs->file, filename);
  fs->size = size;
  fs->used = 0;
  fs->mem = create_heap(size);

  return fs;
}

void destroy_filesystem(filesystem** fs)
{
  destroy_heap(&(*fs)->mem);
  free((*fs)->file);
  free(*fs);
}