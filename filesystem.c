#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filesystem.h"

filesystem* create_filesystem(const char* filename, uint64_t size)
{
  filesystem* fs = (filesystem*)malloc(sizeof(filesystem));
  size_t filename_len = strlen(filename) + 1;
  fs->file = (char*)malloc(filename_len*sizeof(char));
  strcpy(fs->file, filename);
  fs->size = size;
  fs->used = 0;
  fs->mem = create_heap(size);

  heap_node* node = heap_alloc(fs->mem, sizeof(inode));
  inode* root_node = (inode*)malloc(sizeof(inode));
  root_node->flag = INODE_DIR;
  strcpy(root_node->name, "/");
  root_node->next_ptr = 0;
  root_node->data_ptr = 0;
  node->data = root_node;

  FILE* fp = fopen(filename, "r+b");
  void* data = calloc(1, size);
  fwrite(data, size, 1, fp);
  free(data);
  fseek(fp, node->file_offset, SEEK_SET);
  fwrite(root_node, sizeof(inode), 1, fp);
  fclose(fp);

  return fs;
}

void filesystem_add_file(filesystem* fs, const char* filename, const char* source)
{
  // 1. Read file data 
  FILE* ifp = fopen(source, "rb");
  fseek(ifp, 0, SEEK_END);
  uint64_t size = ftell(ifp);
  void* buffer = malloc(size);
  fseek(ifp, 0, SEEK_SET);
  fread(buffer, size, 1, ifp);
  fclose(ifp);

  // 2. Allocate space for inode
  heap_node* heap_inode = heap_alloc(fs->mem, sizeof(inode));
  inode* file_inode = (inode*)malloc(sizeof(inode));
  file_inode->flag = INODE_FILE;
  strcpy(file_inode->name, filename);
  file_inode->size = size;
  file_inode->next_ptr = 0;
  heap_inode->data = file_inode;

  // 3. Allocate space for file data
  heap_node* heap_data = heap_alloc(fs->mem, size);
  heap_data->data = buffer;
  file_inode->data_ptr = heap_data->file_offset;
  heap_inode->data_segment = heap_data;

  // 4. Find last inode segment
  heap_node* last_node = fs->mem->root;
  if(last_node->data_segment)
    last_node = last_node->data_segment;
  while(last_node->next_file_segment)
    last_node = last_node->next_file_segment;
  inode* last_inode = (inode*)last_node->data;

  // 5. Modify it, by changing the pointer
  if(last_inode->flag == INODE_DIR)
  {
    last_node->data_segment = heap_inode;
    last_inode->data_ptr = heap_inode->file_offset;
  }
  else if(last_inode->flag == INODE_FILE)
  {
    last_node->next_file_segment = heap_inode;
    last_inode->next_ptr = heap_inode->file_offset;
  }
  
  // 6. Save the modification in file
  FILE* fp = fopen(fs->file, "r+b");
  fseek(fp, last_node->file_offset, SEEK_SET);
  fwrite(last_inode, sizeof(inode), 1, fp);
  
  // 7. Save allocated blocks for inode and data to a file
  fseek(fp, heap_inode->file_offset, SEEK_SET);
  fwrite(file_inode, sizeof(inode), 1, fp);
  fseek(fp, heap_data->file_offset, SEEK_SET);
  fwrite(buffer, size, 1, fp);
  fclose(fp);
}

void filesystem_get_file(filesystem* fs, const char* filename, const char* destination)
{
  heap_node* node = fs->mem->root->data_segment;
  while(node)
  {
    if(strcmp(((inode*)node->data)->name, filename) == 0)
      break;

    node = node->next_file_segment;
  }

  if(!node)
    return;
  
  heap_node* file_data_segment = node->data_segment;

  FILE* ofp = fopen(destination, "wb");
  fwrite(file_data_segment->data, file_data_segment->size, 1, ofp);
  fclose(ofp);
}

void destroy_filesystem(filesystem** fs)
{
  destroy_heap(&(*fs)->mem);
  free((*fs)->file);
  free(*fs);
}