#pragma once

#include <stdint.h>

typedef struct heap_node
{
  int inUse;
  uint64_t size;
  uint64_t file_offset;
  void* data;
  struct heap_node* next;
} heap_node;

typedef struct heap
{
  uint64_t size;
  uint64_t used;
  heap_node* root;
} heap;

heap_node* create_heap_node(uint64_t size, uint64_t offset);
heap* create_heap(uint64_t size);
void destroy_heap(heap** mem);
void print_heap_info(heap* mem);
