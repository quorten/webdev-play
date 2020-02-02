#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include "dumb_cgipm.h"

void
write_log_error (char *user_err_desc, int err_code, char *log_entry)
{
  char open_write_error[512];
  snprintf (open_write_error, 512, "%s: %s",
	   user_err_desc, strerror (err_code));
  fprintf (stderr, "Failed log entry: %s\n", log_entry);
  fprintf (stderr, "Error code: %s\n", open_write_error);

  cgi_p_header_status ("500 Internal Server Error");
  cgi_p_header_content_type ("text/html");
  cgi_p_end_header ();
  cgi_p_start_html_head ();
  cgi_p_head_http_equiv ("Content-Type", "text/html; charset=iso-8859-1");
  cgi_p_head_meta ("viewport", "width=device-width, initial-scale=1");
  cgi_p_head_title ("500 Internal Server Error");
  cgi_p_end_html_head ();
  cgi_p_h1 ("500 Internal Server Error"); putchar ('\n');
  cgi_p_p ("Could not write the transaction to the log.  "
	   "Your data has NOT been saved.  "
	   "Contact your website administrator."); putchar ('\n');
  cgi_p_h2 ("Log entry:"); putchar ('\n');
  cgi_p_pre (log_entry); putchar ('\n');
  cgi_p_h2 ("Error code:"); putchar ('\n');
  cgi_p_pre (open_write_error); putchar ('\n');
  cgi_p_p_a_href ("index.html", "Return to landing page"); putchar ('\n');
}

int
main (void)
{
  char query_string[4096];
  char *log_entry_desc = "QWB Create Account";
  char script_filename[260];
  char script_dirname[260];
  char log_filename[260];

  char *params[16];
  unsigned params_len = 0;

  time_t cur_time = time (NULL);
  struct tm cur_date;
  char str_cur_date[64];
  char *create_date, *acct_name, *acct_type;

  char *env_script_filename;
  FILE *fp;
  struct flock lock_info;

  char *env_http_referer;

  /* Since this is a POST command, read our query string from standard
     input.  */
  if (fgets (query_string, 4096, stdin) != NULL) {
    cgi_s_parse_params (query_string, 4096, params, &params_len, 16, 0);
  }

  create_date = cgi_s_get_param (params, params_len, "create-date");
  if (!create_date)
    create_date = "";
  if (!create_date[0]) {
    localtime_r (&cur_time, &cur_date);
    strftime (str_cur_date, 64, "%F %T", &cur_date);
    create_date = str_cur_date;
  }
  acct_name = cgi_s_get_param (params, params_len, "acct-name");
  if (!acct_name)
    acct_name = "";
  acct_type = cgi_s_get_param (params, params_len, "acct-type");
  if (!acct_type)
    acct_type = "";

  /* Verify all required parameters are provided.  */
  if (!acct_name[0]) {
    cgi_p_header_status ("400 Bad Request");
    cgi_p_header_content_type ("text/html");
    cgi_p_end_header ();
    cgi_p_start_html_head ();
    cgi_p_head_http_equiv ("Content-Type", "text/html; charset=iso-8859-1");
    cgi_p_head_meta ("viewport", "width=device-width, initial-scale=1");
    cgi_p_head_title ("400 Bad Request");
    cgi_p_end_html_head ();
    cgi_p_h1 ("400 Bad Request"); putchar ('\n');
    cgi_p_p ("The input parameters were not correct.  "
	     "Check your input and try again.");
    cgi_p_end_html ();
    return 0;
  }

  /* Generate log filename, relative to script filename.  */
  /* TODO FIXME: Insufficient error handling and robustness.  */
  env_script_filename = getenv ("SCRIPT_FILENAME");
  if (env_script_filename) {
    strncpy (script_filename, env_script_filename, 260);
    script_filename[260-1] = '\0';
    (*strrchr (script_filename, '/')) = '\0'; /* dirname () */
    strcpy (script_dirname, script_filename);
    strcpy (log_filename, script_dirname);
    strcpy (log_filename + strlen(script_dirname), "/dumbank.log");
  } else
    log_filename[0] = '\0';

  /* Open with an exclusive write lock, append mode.  */
  fp = fopen (log_filename, "a");
  if (fp == NULL) {
    write_log_error ("Error opening log file", errno, log_entry_desc);
    return;
  }
  lock_info.l_type = F_WRLCK;
  lock_info.l_whence = SEEK_SET;
  lock_info.l_start = 0;
  lock_info.l_len = 0;
  lock_info.l_pid = 0;
  if (fcntl (fileno (fp), F_SETLK, &lock_info) == -1) {
    write_log_error ("Error locking log file", errno, log_entry_desc);
    return;
  }

  /* Append our log entry.  */
  fprintf (fp, "\nType:Account\n");
  fprintf (fp, "Creation Date:%s\n", create_date);
  fprintf (fp, "Name:%s\n", acct_name);
  fprintf (fp, "Account Type:%s\n", acct_type);

  /* Close.  */
  if (fclose (fp) == EOF) {
    write_log_error ("Error closing log file", errno, log_entry_desc);
    return;
  }

  cgi_p_header_content_type ("text/html");
  cgi_p_end_header ();
  cgi_p_start_html_head ();
  cgi_p_head_http_equiv ("Content-Type", "text/html; charset=iso-8859-1");
  cgi_p_head_meta ("viewport", "width=device-width, initial-scale=1");
  cgi_p_head_title ("Successful transaction");
  cgi_p_end_html_head ();
  cgi_p_h1 ("Successful transaction");
  cgi_p_pre (log_entry_desc);
  env_http_referer = getenv ("HTTP_REFERER");
  if (!env_http_referer)
    env_http_referer = "";
  cgi_p_p_a_href (env_http_referer, "Add another");
  cgi_p_p_a_href ("index.html", "Return to landing page");
  cgi_p_end_html ();

  return 0;
}
