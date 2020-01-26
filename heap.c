#include "heap.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

heap_node* create_heap_node(uint64_t size, uint64_t offset)
{
  heap_node* node = (heap_node*)malloc(sizeof(heap_node));
  node->in_use = 0;
  node->size = size;
  node->file_offset = offset;
  node->data = NULL;
  node->data_segment = NULL;
  node->next_file_segment = NULL;
  node->prev = NULL;
  node->next = NULL;

  return node;
}

heap* create_heap(uint64_t size)
{
  heap* mem = (heap*)malloc(sizeof(heap));
  mem->size = size;
  mem->used = 0;
  mem->root = create_heap_node(size, 0);

  return mem;
}

void destroy_heap(heap** mem)
{
  heap_node* node = (*mem)->root;
  heap_node* temp = (*mem)->root;
  while(node)
  {
    temp = node;
    node = node->next;
    if(temp->data)
      free(temp->data);
    free(temp);
  }
  free(*mem);
}

void heap_print_info(heap* mem)
{
  printf("Heap size: %ld (%ld used)\n", mem->size, mem->used);
  heap_node* node = mem->root;
  uint32_t segment_num = 0;
  while(node)
  {
    printf("Segment #%d:\n", segment_num++);
    printf("\tIn use: %d\n", node->in_use);
    printf("\tSize: %ld\n", node->size);
    printf("\tFile offset: %ld\n", node->file_offset);
    node = node->next;
  }
}

heap_node* heap_alloc(heap* mem, uint64_t size)
{
  if(mem->size - mem->used < size)
    return NULL;
  
  heap_node* node = mem->root;
  while(node)
  {
    if(!node->in_use && node->size >= size)
    {
      uint64_t offset = node->file_offset;
      if(node->in_use)
        offset += node->size;
      heap_node* new_node = create_heap_node(size, offset);
      new_node->in_use = 1;
      new_node->next = node;
      new_node->prev = node->prev;
      node->prev = new_node;
      node->size -= size;
      node->file_offset += size;
      if(new_node->prev)
        new_node->prev->next = new_node;
      else
        mem->root = new_node;

      mem->used += size;
      return new_node;
    }
    node = node->next;
  }

  return NULL;
}

void heap_consolidate(heap* mem)
{
  heap_node* node = mem->root;
  while(node->next)
  {
    if(!node->in_use && !node->next->in_use)
    {
      heap_node* next = node->next;
      node->next = next->next;
      if(next->next)
        next->next->prev = node;
      node->size += next->size;
      free(next);
    }
    else 
      node = node->next;
  }
}

void heap_dealloc(heap* mem, heap_node* node)
{
  node->in_use = 0;
  node->next_file_segment = NULL;
  mem->used -= node->size;
  if(node->data)
    free(node->data);
  node->data = NULL;

  heap_consolidate(mem);
}