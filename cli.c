#include "cli.h"

#include <stdio.h>

#include "filesystem.h"
#include "heap.h"

void handle_start(cli_context* context);
void handle_file_addition(cli_context* context);
void handle_file_getting(cli_context* context);
void handle_file_copying(cli_context* context);
void handle_file_removing(cli_context* context);
void handle_file_resizing(cli_context* context);
void handle_making_directory(cli_context* context);
void handle_making_link(cli_context* context);

void user_read_line(char* dest, size_t maxLen)
{
  int charactersRead = 0;
  char c = getchar();
  if(c != '\n')
  {
    dest[charactersRead++] = c;
  }
  while(charactersRead < maxLen && (c = getchar()) != '\n')
  {
    dest[charactersRead++] = c;
  }
  dest[charactersRead] = '\0';
}

void handle_fs_opening(cli_context* context)
{
  char filename[31];
  printf("Enter virtual drive file name: ");
  user_read_line(filename, 30);
  context->fs = filesystem_open(filename);
  if(context->fs)
    context->state = CLI_VD_OPEN;
  else
    context->state = CLI_EXIT;
}

void handle_fs_creation(cli_context* context)
{
  char filename[31];
  printf("Enter virtual drive file name: ");
  user_read_line(filename, 30);
  uint64_t size = 0;
  int valid = 0;
  do
  {
    printf("Enter virtual drive size: ");
    if(scanf("%lud", &size) == 0)
    {
      char c;
      while ((c = getchar()) != '\n' && c != EOF);
      puts("");
      continue;
    }
    valid = 1;
  } while(!valid);
  
  context->fs = create_filesystem(filename, size);
  if(context->fs)
    context->state = CLI_VD_OPEN;
  else
    context->state = CLI_EXIT;
}

void handle_start(cli_context* context)
{
  int choice = 0;
  do
  {
    puts("1. Open virtual drive");
    puts("2. Create new virtual drive");
    puts("3. Exit");
    printf(">>>");
    if(scanf("%i", &choice) == 0)
    {
      char c;
      while ((c = getchar()) != '\n' && c != EOF);
      puts("");
      continue;
    }
  } while(choice < 1 || choice > 3);

  switch(choice)
  {
    case 1:
      handle_fs_opening(context);
      break;
    case 2:
      handle_fs_creation(context);
      break;
    default:
      context->state = CLI_EXIT;
      break;
  }
}

void handle_vd_open(cli_context* context)
{
  int choice = 0;
  puts("");
  do
  {
    puts("1. Add file");
    puts("2. Get file");
    puts("3. Remove file");
    puts("4. Copy file");
    puts("5. Resize file");
    puts("6. Make file link");
    puts("7. Make directory");
    puts("8. Print filesystem tree");
    puts("9. Print basic info");
    puts("10. Print advanced info");
    puts("11. Exit");
    printf(">>>");
    if(scanf("%i", &choice) == 0)
    {
      char c;
      while ((c = getchar()) != '\n' && c != EOF);
      puts("");
      continue;
    }
  } while(choice < 1 || choice > 11);

  switch(choice)
  {
    case 1:
      handle_file_addition(context);
      break;
    case 2:
      handle_file_getting(context);
      break;
    case 3:
      handle_file_removing(context);
      break;
    case 4:
      handle_file_copying(context);
      break;
    case 5:
      handle_file_resizing(context);
      break;
    case 6:
      handle_making_link(context);
      break;
    case 7:
      handle_making_directory(context);
      break;
    case 8:
      puts("");
      filesystem_print_tree(context->fs);
      break;
    case 9:
      printf("\nFilesystem size: %ld bytes\nSpace used: %ld bytes (%.2f%%)\n", 
        context->fs->size, context->fs->used, 100.0f*context->fs->used/context->fs->size);
      break;
    case 10:
      puts("");
      heap_print_info(context->fs->mem);
      break;
    default:
      context->state = CLI_EXIT;
      break;
  }
}

void handle_file_addition(cli_context* context)
{
  char src_filename[31];
  char dst_filename[31];
  printf("Enter source file path: ");
  user_read_line(src_filename, 30);
  printf("Enter destination path: ");
  user_read_line(dst_filename, 30);

  int res = filesystem_add_file(context->fs, dst_filename, src_filename);
  if(res != 0)
  {
    puts("ERROR: Failed to add specified file!");
  }
}

void handle_file_getting(cli_context* context)
{
  char src_filename[31];
  char dst_filename[31];
  printf("Enter file name: ");
  user_read_line(src_filename, 30);
  printf("Enter destination path: ");
  user_read_line(dst_filename, 30);

  int res = filesystem_get_file(context->fs, src_filename, dst_filename);
  if(res != 0)
  {
    puts("ERROR: Failed to get specified file!");
  }
}

void handle_file_copying(cli_context* context)
{
  char src_filename[31];
  char dst_filename[31];
  printf("Enter file name: ");
  user_read_line(src_filename, 30);
  printf("Enter destination path: ");
  user_read_line(dst_filename, 30);

  int res = filesystem_copy_file(context->fs, src_filename, dst_filename);
  if(res != 0)
  {
    puts("ERROR: Failed to copy specified file!");
  }
}

void handle_file_removing(cli_context* context)
{
  char src_filename[31];
  printf("Enter file name: ");
  user_read_line(src_filename, 30);

  int res = filesystem_delete_file(context->fs, src_filename);
  if(res != 0)
  {
    puts("ERROR: Failed to delete specified file!");
  }
}

void handle_file_resizing(cli_context* context)
{
  char filename[31];
  printf("Enter file name: ");
  user_read_line(filename, 30);
  uint64_t size = 0;
  int valid = 0;
  do
  {
    printf("Enter new file size: ");
    if(scanf("%lud", &size) == 0)
    {
      char c;
      while ((c = getchar()) != '\n' && c != EOF);
      puts("");
      continue;
    }
    valid = 1;
  } while(!valid);

  int res = filesystem_resize_file(context->fs, filename, size);
  if(res != 0)
  {
    puts("ERROR: Failed to resize specified file!");
  }
}

void handle_making_link(cli_context* context)
{
  char dst_filename[31];
  char link_name[31];
  printf("Enter destination file name: ");
  user_read_line(dst_filename, 30);
  printf("Enter link name: ");
  user_read_line(link_name, 30);

  int res = filesystem_make_link(context->fs, dst_filename, link_name);
  if(res != 0)
  {
    puts("ERROR: Failed to make link to a specified file!");
  }
}

void handle_making_directory(cli_context* context)
{
  char directory[31];
  printf("Enter directory: ");
  user_read_line(directory, 30);

  int res = filesystem_make_directory(context->fs, directory);
  if(res != 0)
  {
    puts("ERROR: Failed to make specified directory!");
  }
}

void handle_current_state(cli_context* context)
{
  switch(context->state)
  {
    case CLI_START:
      handle_start(context);
      break;
    case CLI_VD_OPEN:
      handle_vd_open(context);
      break;
    default:
      context->state = CLI_EXIT;
      break;
  }
}

void handle_user_interaction()
{
  cli_context context;
  context.state = CLI_START;
  context.fs = NULL;

  while(context.state != CLI_EXIT)
  {
    handle_current_state(&context);
  }

  if(context.fs)
    destroy_filesystem(&context.fs);
}