#include <stdio.h>

#include "filesystem.h"

int main()
{
  filesystem* fs = create_filesystem("test.fs", 512);
  heap_print_info(fs->mem);

  filesystem_add_file(fs, "test.txt", "sample-files/test1.txt");
  heap_print_info(fs->mem);
  filesystem_add_file(fs, "test2.txt", "sample-files/test2.txt");
  heap_print_info(fs->mem);

  filesystem_get_file(fs, "test.txt", "test.txt");
  filesystem_get_file(fs, "test2.txt", "test2.txt");

  destroy_filesystem(&fs);
  
  return 0;
}