#ifndef TMPL_LOG_TAIL_H
#define TMPL_LOG_TAIL_H

int gen_tmpl_name (char *tmpl_name, unsigned tmpl_name_limit,
		   const char *prog_name);
void tail_read_log_error (char *user_err_desc, int err_code);
void gen_tmpl_log_tail (unsigned log_tail_len, const char *tmpl_name);

#endif /* not TMPL_LOG_TAIL */
