#include "segment_array.h"

#include <stdlib.h>
#include <string.h>

segment_array* create_segment_array()
{
  segment_array* arr = (segment_array*)malloc(sizeof(segment_array));
  arr->size = 0;
  arr->capacity = 5;
  arr->data = (segment*)malloc(5*sizeof(segment));

  return arr;
}

void segment_array_add(segment_array* arr, segment seg)
{
  if(arr->size < arr->capacity)
  {
    arr->data[arr->size] = seg;
    arr->size += 1;
  }
  else
  {
    uint64_t new_cap = 2 * arr->capacity;
    segment* data = (segment*)malloc(new_cap*sizeof(segment));
    memcpy(data, arr->data, arr->size*sizeof(segment));
    free(arr->data);
    arr->data = data;
    arr->capacity = new_cap;

    arr->data[arr->size] = seg;
    arr->size += 1;
  }
}

segment* segment_array_get(segment_array* arr, uint64_t idx)
{
  return &arr->data[idx];
}

uint64_t segment_array_size(segment_array* arr)
{
  return arr->size;
}

uint64_t segment_array_capacity(segment_array* arr)
{
  return arr->capacity;
}

int segment_array_comparator(const void* p1, const void* p2)
{
  segment* a = (segment*)p1;
  segment* b = (segment*)p2;
  if(a->end < b->end)
    return -1;
  else if(a->end > b->end)
    return 1;
  else return 0;
}

void segment_array_sort(segment_array* arr)
{
  qsort(arr->data, arr->size, sizeof(segment), segment_array_comparator);
}

void segment_array_destroy(segment_array** arr)
{
  free((*arr)->data);
  free(*arr);
}
