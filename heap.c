#include "heap.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "segment_array.h"
#include "filesystem.h"

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

void get_directory_segments(segment_array* arr, inode* dir_node, FILE* fp, uint64_t idx)
{
  inode* node = (inode*)malloc(sizeof(inode));
  fseek(fp, dir_node->data_ptr, SEEK_SET);
  fread(node, sizeof(inode), 1, fp);
  uint64_t pos = ftell(fp) - sizeof(inode);
  while(node)
  {
    segment inode_seg;
    inode_seg.data = node;
    inode_seg.start = pos;
    inode_seg.size = sizeof(inode);
    inode_seg.end = inode_seg.start + inode_seg.size;
    inode_seg.next_ptr = node->next_ptr;
    inode_seg.data_ptr = node->data_ptr;
    segment_array_add(arr, inode_seg);
    uint64_t inode_idx = arr->size - 1;

    if(node->flag == INODE_FILE)
    {
      segment data_seg;
      data_seg.start = node->data_ptr;
      data_seg.size = node->size;
      data_seg.end = data_seg.start + data_seg.size;
      data_seg.data_ptr = 0;
      data_seg.next_ptr = 0;
      void* buffer = malloc(node->size);
      fseek(fp, node->data_ptr, SEEK_SET);
      fread(buffer, node->size, 1, fp);
      data_seg.data = buffer;
      segment_array_add(arr, data_seg);
    }
    else if(node->flag == INODE_DIR)
    {
      if(node->data_ptr)
        get_directory_segments(arr, node, fp, inode_idx);
    }

    if(node->next_ptr)
    {
      pos = node->next_ptr;
      fseek(fp, node->next_ptr, SEEK_SET);
      node = (inode*)malloc(sizeof(inode));
      fread(node, sizeof(inode), 1, fp);
    }
    else
      break;
  }
}

void heap_add_segment(heap* heap, segment* seg, FILE* fp)
{
  heap_node* new_node = seg->node;
  heap->used += new_node->size;

  if(!heap->root)
    heap->root = new_node;
  else
  {
    heap_node* node = heap->root;
    while(node->next)
      node = node->next;
    node->next = new_node;
    new_node->prev = node;
  }
}

void heap_add_hole(heap* heap, uint64_t size, uint64_t off)
{
  heap_node* new_node = (heap_node*)malloc(sizeof(heap_node));
  new_node->in_use = 0;
  new_node->file_offset = off;
  new_node->size = size;
  new_node->data = NULL;
  new_node->data_segment = NULL;
  new_node->next_file_segment = NULL;
  new_node->prev = NULL;
  new_node->next = NULL;

  if(!heap->root)
    heap->root = new_node;
  else
  {
    heap_node* node = heap->root;
    while(node->next)
      node = node->next;
    node->next = new_node;
    new_node->prev = node;
  }
}

heap* heap_deserialize(const char* filename, uint64_t size)
{
  heap* mem = (heap*)malloc(sizeof(heap));
  mem->size = size;
  mem->used = 0;
  mem->root = NULL;
  
  segment_array* arr = create_segment_array();
  inode* node = (inode*)malloc(sizeof(inode));
  FILE* fp = fopen(filename, "rb");
  fread(node, sizeof(inode), 1, fp);

  segment s;
  s.start = 0;
  s.size = sizeof(inode);
  s.end = s.start + s.size;
  s.data_ptr = node->data_ptr;
  s.next_ptr = node->next_ptr;
  s.data = node;
  segment_array_add(arr, s);
  get_directory_segments(arr, node, fp, 0);

  for(uint64_t i = 0; i < arr->size; ++i)
  {
    segment* seg = segment_array_get(arr, i);
    seg->node = (heap_node*)malloc(sizeof(heap_node));
    seg->node->in_use = 1;
    seg->node->file_offset = seg->start;
    seg->node->size = seg->size;
    seg->node->data = seg->data;
    seg->node->data_segment = NULL;
    seg->node->next_file_segment = NULL;
    seg->node->prev = NULL;
    seg->node->next = NULL;
  }

  // Connect the segments
  for(uint64_t i = 0; i < arr->size; ++i)
  {
    segment* seg = segment_array_get(arr, i);
    heap_node* node = seg->node;

    if(seg->next_ptr)
    {
      uint64_t start = seg->next_ptr;
      for(uint64_t j = 0; j < arr->size; ++j)
      {
        if(segment_array_get(arr, j)->start == start)
        {
          node->next_file_segment = segment_array_get(arr, j)->node;
          break;
        }
      }
    }

    if(seg->data_ptr)
    {
      uint64_t start = seg->data_ptr;
      for(uint64_t j = 0; j < arr->size; ++j)
      {
        if(segment_array_get(arr, j)->start == start)
        {
          node->data_segment = segment_array_get(arr, j)->node;
          break;
        }
      }
    }
  }

  segment_array_sort(arr);
  uint64_t prev_segment_end = 0;
  for(uint64_t i = 0; i < arr->size; ++i)
  {
    segment* seg = segment_array_get(arr, i);
    uint64_t gap_size = seg->start - prev_segment_end;
    if(gap_size > 0)
      heap_add_hole(mem, gap_size, prev_segment_end);
    prev_segment_end = seg->end;
    heap_add_segment(mem, seg, fp);
  }
  uint64_t free_space = mem->size - prev_segment_end;
  if(free_space > 0)
    heap_add_hole(mem, free_space, prev_segment_end);

  segment_array_destroy(&arr);
  fclose(fp);

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