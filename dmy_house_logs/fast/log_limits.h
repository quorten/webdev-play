/* Buffer size limits defined inside house logs server-side code.

   Motivations for using static allocation with fixed max buffer size
   limits:

   * Worse-case performance is guaranteed to be finite due to memory
     bounds on the data that can be processed.

   * Statically allocated memory is highly performant.  No CPU time
     overhead associated with dynamic memory allocation.

   * No memory leaks associated with programming errors when using
     dynamic memory allocation.  The trade-off is that you must be
     careful to bound all buffer memory accesses to prevent buffer
     overruns.

*/

#ifndef LOG_LIMITS_H
#define LOG_LIMITS_H

#define MAX_STR_ERROR_LEN 512
#define MAX_LOG_ENTRY_LEN 4096
#define MAX_TAIL_LOG_LINES 32
#define MAX_QUERY_STR_LEN 4096
#define MAX_LOG_PATH 260
#define MAX_PARAMS_IDX 16
#define MAX_COOKIES_IDX 16
#define MAX_STR_DATE_LEN 64

#endif /* not LOG_LIMITS_H */
