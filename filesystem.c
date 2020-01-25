#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filesystem.h"

heap_node* find_directory(filesystem* fs, const char* name)
{
  uint64_t name_len = strlen(name) + 1;
  char* copy = (char*)malloc(name_len * sizeof(char));
  strcpy(copy, name);

  heap_node* node = fs->mem->root;

  char* token = strtok(copy, "/");
  int found = 0;
  while(token)
  {
    node = node->data_segment;
    while(node)
    {
      if(strcmp(((inode*)node->data)->name, token) == 0)
      {
        found = 1;
        break;
      }
      node = node->next_file_segment;
    }
    if(!found) break;
    token = strtok(NULL, "/");
  }

  free(copy);

  if(found)
    return node;

  return NULL;
}

int directory_filename_split(const char* path, char** directory, char** filename)
{
  int full_len = strlen(path);
  char* ptr = strrchr(path, '/');
  if(!ptr)
  {
    *filename = malloc(full_len + 1);
    strncpy(*filename, path, full_len + 1);
    return 1;
  }

  int idx = ptr - path + 1;
  int dir_len = idx - 1;
  int fn_len = full_len - idx + 1;
  *directory = malloc(dir_len + 1);
  *filename = malloc(fn_len);
  strncpy(*directory, path, dir_len);
  (*directory)[dir_len] = '\0';
  strncpy(*filename, path+idx, fn_len);

  return 0;
}

void print_dir(heap_node* node, int pad)
{
  heap_node* file_node = node->data_segment;
  while(file_node)
  {
    inode* i = (inode*)file_node->data;
    for(int i = 0; i < pad; ++i)
      putchar(' ');
    puts(i->name);
    
    if(i->flag == INODE_DIR)
      print_dir(file_node, pad + 2);

    file_node = file_node->next_file_segment;
  }
}

void filesystem_print_tree(filesystem* fs)
{
  heap_node* root = fs->mem->root;
  puts(((inode*)root->data)->name);
  print_dir(root, 2);
}

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

  FILE* fp = fopen(filename, "w+b");
  void* data = calloc(1, size);
  fwrite(data, size, 1, fp);
  free(data);
  fseek(fp, node->file_offset, SEEK_SET);
  fwrite(root_node, sizeof(inode), 1, fp);
  fclose(fp);

  return fs;
}

void filesystem_add_file(filesystem* fs, const char* path, const char* source)
{
  // 1. Read file data 
  FILE* ifp = fopen(source, "rb");
  fseek(ifp, 0, SEEK_END);
  uint64_t size = ftell(ifp);
  void* buffer = malloc(size);
  fseek(ifp, 0, SEEK_SET);
  fread(buffer, size, 1, ifp);
  fclose(ifp);

  char* directory = NULL;
  char* filename = NULL;
  int res = directory_filename_split(path, &directory, &filename);

  // 2. Find last inode segment
  heap_node* last_node = res == 0 ? find_directory(fs, directory) : fs->mem->root;
  if(last_node->data_segment)
  {
    last_node = last_node->data_segment;
    while(last_node->next_file_segment)
      last_node = last_node->next_file_segment;
  }
  inode* last_inode = (inode*)last_node->data;

  // 3. Allocate space for inode
  heap_node* heap_inode = heap_alloc(fs->mem, sizeof(inode));
  inode* file_inode = (inode*)malloc(sizeof(inode));
  file_inode->flag = INODE_FILE;
  strcpy(file_inode->name, filename);
  file_inode->size = size;
  file_inode->next_ptr = 0;
  heap_inode->data = file_inode;

  if(directory)
    free(directory);
  free(filename);

  // 4. Allocate space for file data
  heap_node* heap_data = heap_alloc(fs->mem, size);
  heap_data->data = buffer;
  file_inode->data_ptr = heap_data->file_offset;
  heap_inode->data_segment = heap_data;

  // 5. Modify previous inode, by changing the pointer
  if(last_inode->flag == INODE_DIR)
  {
    last_node->data_segment = heap_inode;
    last_inode->data_ptr = heap_inode->file_offset;
  }
  else
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

void filesystem_get_file(filesystem* fs, const char* path, const char* destination)
{
  char* directory = NULL;
  char* filename = NULL;
  int res = directory_filename_split(path, &directory, &filename);

  heap_node* node = res == 0 ? find_directory(fs, directory)->data_segment : fs->mem->root->data_segment;
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

void filesystem_make_directory(filesystem* fs, const char* name)
{
  // 1. Allocate space for inode
  heap_node* heap_inode = heap_alloc(fs->mem, sizeof(inode));
  inode* file_inode = (inode*)malloc(sizeof(inode));
  file_inode->flag = INODE_DIR;
  strcpy(file_inode->name, name);
  file_inode->size = 0;
  file_inode->next_ptr = 0;
  file_inode->data_ptr = 0;
  heap_inode->data = file_inode;

  // 2. Find last inode segment
  heap_node* last_node = fs->mem->root;
  if(last_node->data_segment)
    last_node = last_node->data_segment;
  while(last_node->next_file_segment)
    last_node = last_node->next_file_segment;
  inode* last_inode = (inode*)last_node->data;

  // 3. Modify it, by changing the pointer
  if(last_node == fs->mem->root)
  {
    last_node->data_segment = heap_inode;
    last_inode->data_ptr = heap_inode->file_offset;
  }
  else
  {
    last_node->next_file_segment = heap_inode;
    last_inode->next_ptr = heap_inode->file_offset;
  }
  
  // 4. Save the modification in file
  FILE* fp = fopen(fs->file, "r+b");
  fseek(fp, last_node->file_offset, SEEK_SET);
  fwrite(last_inode, sizeof(inode), 1, fp);
  
  // 5. Save allocated blocks for inode
  fseek(fp, heap_inode->file_offset, SEEK_SET);
  fwrite(file_inode, sizeof(inode), 1, fp);
  fclose(fp);
}

void destroy_filesystem(filesystem** fs)
{
  destroy_heap(&(*fs)->mem);
  free((*fs)->file);
  free(*fs);
}