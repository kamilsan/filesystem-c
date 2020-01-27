#pragma once

struct filesystem;

enum cli_states
{
  CLI_START,
  CLI_VD_OPEN,
  CLI_EXIT
};

typedef struct cli_context
{
  enum cli_states state;
  struct filesystem* fs;
} cli_context;

void handle_user_interaction();