#include "tmpl_log_tail.h"

#define LOG_TAIL_LEN 5
#define TMPL_NAME "level_1.html"

int
main (void)
{
  gen_tmpl_log_tail (LOG_TAIL_LEN, TMPL_NAME);
  return 0;
}
