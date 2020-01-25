#include <stdio.h>

#include "filesystem.h"

int main()
{
  filesystem* fs = create_filesystem("test.fs", 5*1024);

  filesystem_add_file(fs, "test1.txt", "sample-files/test1.txt");
  filesystem_add_file(fs, "test2.txt", "sample-files/test2.txt");

  filesystem_make_directory(fs, "dir1/subdir/subsubdir");
  filesystem_make_directory(fs, "dir2");

  filesystem_add_file(fs, "dir1/test1.txt", "sample-files/test1.txt");
  filesystem_add_file(fs, "dir2/test2.txt", "sample-files/test2.txt");
  filesystem_add_file(fs, "dir1/subdir/subsubdir/test3.txt", "sample-files/test3.txt");

  filesystem_copy_file(fs, "dir1/test1.txt", "dir1/subdir/copy.txt");

  heap_print_info(fs->mem);
  filesystem_print_tree(fs);

  filesystem_get_file(fs, "test1.txt", "test1.txt");
  filesystem_get_file(fs, "test2.txt", "test2.txt");
  filesystem_get_file(fs, "dir1/test1.txt", "test_dir1.txt");
  filesystem_get_file(fs, "dir2/test2.txt", "test_dir2.txt");
  filesystem_get_file(fs, "dir1/subdir/subsubdir/test3.txt", "test3.txt");
  filesystem_get_file(fs, "dir1/subdir/copy.txt", "test_copy.txt");

  destroy_filesystem(&fs);
  
  return 0;
}