#include "heap.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

heap_node* create_heap_node(uint64_t size, uint64_t offset)
{
  heap_node* node = (heap_node*)malloc(sizeof(heap_node));
  node->inUse = 0;
  node->size = size;
  node->file_offset = offset;
  node->data = malloc(size);
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
    free(temp->data);
    free(temp);
  }
  free(*mem);
}

void print_heap_info(heap* mem)
{
  printf("Heap size: %ld (%ld used)\n", mem->size, mem->used);
  heap_node* node = mem->root;
  uint32_t segment_num = 0;
  while(node)
  {
    printf("Segment #%d:\n", segment_num++);
    printf("\tIn use: %d\n", node->inUse);
    printf("\tSize: %ld\n", node->size);
    printf("\tFile offset: %ld\n", node->file_offset);
    node = node->next;
  }
}