#include <stdio.h>

#include "filesystem.h"

int main()
{
  filesystem* fs = create_filesystem("test.fs", 512);

  filesystem_add_file(fs, "test.txt", "sample-files/test1.txt");
  filesystem_add_file(fs, "test2.txt", "sample-files/test2.txt");

  filesystem_make_directory(fs, "dir1");
  filesystem_make_directory(fs, "dir2");

  filesystem_add_file(fs, "dir1/test.txt", "sample-files/test1.txt");
  filesystem_add_file(fs, "dir2/test2.txt", "sample-files/test2.txt");

  heap_print_info(fs->mem);
  filesystem_print_tree(fs);

  filesystem_get_file(fs, "test.txt", "test.txt");
  filesystem_get_file(fs, "test2.txt", "test2.txt");
  filesystem_get_file(fs, "dir1/test.txt", "test_dir1.txt");
  filesystem_get_file(fs, "dir2/test2.txt", "test_dir2.txt");

  destroy_filesystem(&fs);
  
  return 0;
}