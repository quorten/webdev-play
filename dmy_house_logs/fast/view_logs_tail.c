#include "tmpl_log_tail.h"

#define LOG_TAIL_LEN 1000
#define TMPL_NAME "view_logs_tail.html"

int
main (void)
{
  gen_tmpl_log_tail (LOG_TAIL_LEN, TMPL_NAME);
  return 0;
}
