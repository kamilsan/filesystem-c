#pragma once

#include <stdint.h>

struct heap_node;

typedef struct segment
{
  uint64_t start;
  uint64_t end;
  uint64_t size;
  void* data;
  struct segment* next_ptr;
  struct segment* data_ptr;
  struct heap_node* node;
} segment;

typedef struct segment_array
{
  uint64_t size;
  uint64_t capacity;
  segment* data;
} segment_array;

segment_array* create_segment_array();
void segment_array_add(segment_array* arr, segment seg);
segment* segment_array_get(segment_array* arr, uint64_t idx);
uint64_t segment_array_size(segment_array* arr);
uint64_t segment_array_capacity(segment_array* arr);
void segment_array_sort(segment_array* arr);
void segment_array_destroy(segment_array** arr);