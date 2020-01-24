#pragma once

#include <stdint.h>

typedef struct heap_node
{
  int used;
  uint64_t size;
  uint64_t file_offset;
  struct heap_node* next;
} heap_node;

typedef struct heap
{
  heap_node* root;
} heap;