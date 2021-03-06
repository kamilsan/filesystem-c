#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filesystem.h"
#include "heap.h"

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
  inode* i = (inode*)node->data;
  for(int i = 0; i < pad; ++i)
      putchar(' ');
  printf("\033[34;1m%s\033[0m\n", i->name); //NOTE: this works only on linux
  pad += 2;
  
  heap_node* file_node = node->data_segment;
  while(file_node)
  {
    i = (inode*)file_node->data;
    if(i->flag == INODE_FILE)
    {
      for(int i = 0; i < pad; ++i)
        putchar(' ');
      printf("%s (%ldb)\n", i->name, i->size);
    }
    else if(i->flag == INODE_LINK)
    {
      for(int i = 0; i < pad; ++i)
        putchar(' ');
      printf("\033[33;1m%s\033[0m\n", i->name, i->size);
    }
    else if(i->flag == INODE_DIR)
    {
      print_dir(file_node, pad);
    }

    file_node = file_node->next_file_segment;
  }
}

heap_node* make_subdirectory(filesystem* fs, heap_node* parent, const char* name)
{
  heap_node* heap_inode = heap_alloc(fs->mem, sizeof(inode));
  if(!heap_inode)
    return NULL;
  fs->used += sizeof(inode);
  inode* file_inode = (inode*)calloc(1, sizeof(inode));
  file_inode->flag = INODE_DIR;
  strcpy(file_inode->name, name);
  file_inode->size = 0;
  file_inode->next_ptr = 0;
  file_inode->data_ptr = 0;
  heap_inode->data = file_inode;

  heap_node* last_node = parent;
  if(last_node->data_segment)
  {
    last_node = last_node->data_segment;
    while(last_node->next_file_segment)
      last_node = last_node->next_file_segment;
  }
  inode* last_inode = (inode*)last_node->data;

  if(last_node == parent)
  {
    last_node->data_segment = heap_inode;
    last_inode->data_ptr = heap_inode->file_offset;
  }
  else
  {
    last_node->next_file_segment = heap_inode;
    last_inode->next_ptr = heap_inode->file_offset;
  }
  
  FILE* fp = fopen(fs->file, "r+b");
  fseek(fp, last_node->file_offset, SEEK_SET);
  fwrite(last_inode, sizeof(inode), 1, fp);
  
  fseek(fp, heap_inode->file_offset, SEEK_SET);
  fwrite(file_inode, sizeof(inode), 1, fp);
  fclose(fp);

  return heap_inode;
}

int make_file(filesystem* fs, const char* path, void* buffer, uint64_t size)
{
  char* directory = NULL;
  char* filename = NULL;
  int res = directory_filename_split(path, &directory, &filename);

  heap_node* last_node = res == 0 ? find_directory(fs, directory) : fs->mem->root;
  int dir_empty = 1;
  if(last_node->data_segment)
  {
    dir_empty = 0;
    last_node = last_node->data_segment;
    while(last_node->next_file_segment)
      last_node = last_node->next_file_segment;
  }
  inode* last_inode = (inode*)last_node->data;

  heap_node* heap_inode = heap_alloc(fs->mem, sizeof(inode));
  if(!heap_inode)
  {
    if(directory)
      free(directory);
    free(filename);
    return -1;
  }
  fs->used += sizeof(inode);
  inode* file_inode = (inode*)calloc(1, sizeof(inode));
  file_inode->flag = INODE_FILE;
  strcpy(file_inode->name, filename);
  file_inode->size = size;
  file_inode->next_ptr = 0;
  heap_inode->data = file_inode;

  if(directory)
    free(directory);
  free(filename);

  heap_node* heap_data = heap_alloc(fs->mem, size);
  if(!heap_data)
  {
    heap_dealloc(fs->mem, heap_inode);
    free(buffer);
    return -1;
  }
  fs->used += size;
  heap_data->data = buffer;
  file_inode->data_ptr = heap_data->file_offset;
  heap_inode->data_segment = heap_data;

  if(dir_empty)
  {
    last_node->data_segment = heap_inode;
    last_inode->data_ptr = heap_inode->file_offset;
  }
  else
  {
    last_node->next_file_segment = heap_inode;
    last_inode->next_ptr = heap_inode->file_offset;
  }
  
  FILE* fp = fopen(fs->file, "r+b");
  fseek(fp, last_node->file_offset, SEEK_SET);
  fwrite(last_inode, sizeof(inode), 1, fp);
  
  fseek(fp, heap_inode->file_offset, SEEK_SET);
  fwrite(file_inode, sizeof(inode), 1, fp);
  fseek(fp, heap_data->file_offset, SEEK_SET);
  fwrite(buffer, size, 1, fp);
  fclose(fp);

  return 0;
}

heap_node* find_file(filesystem* fs, const char* path)
{
  char* directory = NULL;
  char* filename = NULL;
  int res = directory_filename_split(path, &directory, &filename);

  int found = 0;
  heap_node* node = fs->mem->root->data_segment;
  if(res == 0)
  {
    heap_node* dir_node = find_directory(fs, directory);
    if(dir_node)
      node = dir_node->data_segment;
    else 
      return NULL;
  }
  while(node)
  {
    if(strcmp(((inode*)node->data)->name, filename) == 0)
    {
      found = 1;
      break;
    }

    node = node->next_file_segment;
  }

  if(directory)
    free(directory);
  free(filename);

  if(found)
    return node;
  else 
    return NULL;
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
  if(!node)
    return NULL;
  fs->used += sizeof(inode);
  inode* root_node = (inode*)calloc(1, sizeof(inode));
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

filesystem* filesystem_open(const char* filename)
{
  FILE* fp = fopen(filename, "rb");
  fseek(fp, 0, SEEK_END);
  uint64_t size = ftell(fp);
  fclose(fp);

  filesystem* fs = (filesystem*)malloc(sizeof(filesystem));
  size_t filename_len = strlen(filename) + 1;
  fs->file = (char*)malloc(filename_len*sizeof(char));
  strcpy(fs->file, filename);
  fs->size = size;
  fs->mem = heap_deserialize(filename, size);
  fs->used = fs->mem->used;

  return fs;
}

void filesystem_print_tree(filesystem* fs)
{
  heap_node* root = fs->mem->root;
  print_dir(root, 0);
}

int filesystem_add_file(filesystem* fs, const char* path, const char* source)
{
  FILE* ifp = fopen(source, "rb");
  fseek(ifp, 0, SEEK_END);
  uint64_t size = ftell(ifp);
  void* buffer = calloc(1, size);
  fseek(ifp, 0, SEEK_SET);
  fread(buffer, size, 1, ifp);
  fclose(ifp);

  int res = make_file(fs, path, buffer, size);
  return res;
}

int filesystem_get_file(filesystem* fs, const char* path, const char* destination)
{
  heap_node* node = find_file(fs, path);
  if(!node)
    return -1;
  
  heap_node* file_data_segment = node->data_segment;

  FILE* ofp = fopen(destination, "wb");
  fwrite(file_data_segment->data, file_data_segment->size, 1, ofp);
  fclose(ofp);

  return 0;
}

int filesystem_make_directory(filesystem* fs, const char* name)
{
  uint64_t name_len = strlen(name) + 1;
  char* copy = (char*)malloc(name_len * sizeof(char));
  strcpy(copy, name);

  heap_node* parent = fs->mem->root;

  char* token = strtok(copy, "/");
  while(token)
  {
    parent = make_subdirectory(fs, parent, token);
    if(!parent)
      return -1;
    token = strtok(NULL, "/");
  }

  free(copy);
  return 0;
}

int filesystem_make_link(filesystem* fs, const char* src, const char* dst)
{
  heap_node* node = find_file(fs, src);
  if(!node)
    return -1;
  
  heap_node* file_data_segment = node->data_segment;
  char* directory = NULL;
  char* filename = NULL;
  int res = directory_filename_split(dst, &directory, &filename);

  heap_node* last_node = res == 0 ? find_directory(fs, directory) : fs->mem->root;
  int dir_empty = 1;
  if(last_node->data_segment)
  {
    dir_empty = 0;
    last_node = last_node->data_segment;
    while(last_node->next_file_segment)
      last_node = last_node->next_file_segment;
  }
  inode* last_inode = (inode*)last_node->data;

  heap_node* heap_inode = heap_alloc(fs->mem, sizeof(inode));
  if(!heap_inode)
  {
    if(directory)
      free(directory);
    free(filename);
    return -1;
  }
  fs->used += sizeof(inode);
  inode* file_inode = (inode*)calloc(1, sizeof(inode));
  file_inode->flag = INODE_LINK;
  strcpy(file_inode->name, filename);
  file_inode->size = 0;
  file_inode->next_ptr = 0;
  heap_inode->data = file_inode;

  if(directory)
    free(directory);
  free(filename);

  file_inode->data_ptr = node->data_segment->file_offset;
  heap_inode->data_segment = node->data_segment;

  if(dir_empty)
  {
    last_node->data_segment = heap_inode;
    last_inode->data_ptr = heap_inode->file_offset;
  }
  else
  {
    last_node->next_file_segment = heap_inode;
    last_inode->next_ptr = heap_inode->file_offset;
  }
  
  FILE* fp = fopen(fs->file, "r+b");
  fseek(fp, last_node->file_offset, SEEK_SET);
  fwrite(last_inode, sizeof(inode), 1, fp);
  
  fseek(fp, heap_inode->file_offset, SEEK_SET);
  fwrite(file_inode, sizeof(inode), 1, fp);
  fclose(fp);
}

int filesystem_copy_file(filesystem* fs, const char* src, const char* dst)
{
  heap_node* node = find_file(fs, src);
  if(!node)
    return -1;
  
  heap_node* file_data_segment = node->data_segment;
  void* buffer = calloc(1, file_data_segment->size);
  memcpy(buffer, file_data_segment->data, file_data_segment->size);
  int res = make_file(fs, dst, buffer, file_data_segment->size);
  return res;
}

int filesystem_delete_file(filesystem* fs, const char* file)
{
  char* directory = NULL;
  char* filename = NULL;
  int res = directory_filename_split(file, &directory, &filename);

  int found = 0;
  int first_child = 1;
  heap_node* prev = fs->mem->root;
  heap_node* node = prev->data_segment;
  if(res == 0)
  {
    heap_node* dir_node = find_directory(fs, directory);
    if(dir_node)
    {
      prev = dir_node;
      node = dir_node->data_segment;
    }
    else
    {
      if(directory)
        free(directory);
      free(filename);
      return -1;
    }
  }
  while(node)
  {
    if(strcmp(((inode*)node->data)->name, filename) == 0)
    {
      found = 1;
      break;
    }
    first_child = 0;
    prev = node;
    node = node->next_file_segment;
  }

  if(directory)
    free(directory);
  free(filename);

  if(!found)
    return -1;

  inode* prev_inode = (inode*)prev->data;
  inode* file_inode = (inode*)node->data;
  if(prev_inode->flag != INODE_DIR || !first_child)
  {
    prev_inode->next_ptr = file_inode->next_ptr;
    prev->next_file_segment = node->next_file_segment;
  }
  else if(prev_inode->flag == INODE_DIR && first_child)
  {
    prev_inode->data_ptr = file_inode->next_ptr;
    prev->data_segment = node->next_file_segment;
  }

  fs->used -= sizeof(inode);
  if(file_inode->flag == INODE_FILE)
  {
    fs->used -= file_inode->size;
    heap_dealloc(fs->mem, node->data_segment);
  }
  heap_dealloc(fs->mem, node);

  FILE* fp = fopen(fs->file, "r+b");
  fseek(fp, prev->file_offset, SEEK_SET);
  fwrite(prev->data, sizeof(inode), 1, fp);
  fclose(fp);

  return 0;
}

int filesystem_resize_file(filesystem* fs, const char* file, uint64_t new_size)
{
  heap_node* node = find_file(fs, file);
  if(!node)
    return -1;
  
  heap_node* file_data_segment = node->data_segment;
  heap_node* new_data = heap_alloc(fs->mem, new_size);
  if(!new_size)
    return -1;

  void* new_buffer = calloc(1, new_size);
  uint64_t to_copy = file_data_segment->size;
  if(new_size < to_copy)
    to_copy = new_size;
  memcpy(new_buffer, file_data_segment->data, to_copy);

  inode* file_inode = (inode*)node->data;
  fs->used += new_size - file_inode->size;
  file_inode->size = new_size;
  new_data->data = new_buffer;
  file_inode->data_ptr = new_data->file_offset;
  node->data_segment = new_data;

  heap_dealloc(fs->mem, file_data_segment);
  
  FILE* fp = fopen(fs->file, "r+b");
  fseek(fp, node->file_offset, SEEK_SET);
  fwrite(file_inode, sizeof(inode), 1, fp);
  fseek(fp, new_data->file_offset, SEEK_SET);
  fwrite(new_buffer, new_size, 1, fp);
  fclose(fp);

  return 0;
}

void destroy_filesystem(filesystem** fs)
{
  destroy_heap(&(*fs)->mem);
  free((*fs)->file);
  free(*fs);
}