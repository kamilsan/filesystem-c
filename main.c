#include <stdio.h>

#include "filesystem.h"

int main()
{
  filesystem* fs = create_filesystem("test.fs", 512);
  print_heap_info(fs->mem);
  destroy_filesystem(&fs);
  return 0;
}