#pragma once

#include <stdint.h>

typedef struct heap_node
{
  int in_use;
  uint64_t size;
  uint64_t file_offset;
  void* data;
  struct heap_node* data_segment;
  struct heap_node* next_file_segment;
  struct heap_node* prev;
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
void heap_print_info(heap* mem);
heap_node* heap_alloc(heap* mem, uint64_t size);
void heap_dealloc(heap* mem, heap_node* node);
