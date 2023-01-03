#include <string.h>

#include "tmpl_log_tail.h"

#define LOG_TAIL_LEN 5

int
main (int argc, char *argv[])
{
  char tmpl_name[260];
  if (!gen_tmpl_name (tmpl_name, 260, argv[0])) {
    return 1;
  }
  gen_tmpl_log_tail (LOG_TAIL_LEN, tmpl_name);
  return 0;
}
