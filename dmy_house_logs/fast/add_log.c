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
  cgi_p_p ("Could not write the signature to the log.  "
	   "Your data has been copied to the "
	   "server error log instead.  "
	   "Contact your website administrator."); putchar ('\n');
  cgi_p_h2 ("Log entry:"); putchar ('\n');
  cgi_p_pre (log_entry); putchar ('\n');
  cgi_p_h2 ("Error code:"); putchar ('\n');
  cgi_p_pre (open_write_error); putchar ('\n');
  cgi_p_p_a_href ("..", "Return to landing page"); putchar ('\n');
}

int
main (void)
{
  char query_string[4096];
  char log_entry[4096];
  char script_filename[260];
  char script_dirname[260];
  char log_dirname[260];
  char log_filename[260];

  char *params[16];
  unsigned params_len = 0;
  char *cookies[16];
  unsigned cookies_len = 0;

  time_t cur_time = time (NULL);
  struct tm cur_date;
  char str_cur_date[64];
  char *log_date_time, *log_name, *log_action;

  char *env_script_filename;
  char *log_dirname_end;
  unsigned log_dirname_len;
  FILE *fp;
  struct flock lock_info;

  char *env_http_referer;

  /* Since this is a POST command, read our query string from standard
     input.  Error out on overflow errors.  */
  if (fgets (query_string, 4096, stdin) == NULL) {
    write_log_error ("Error reading input parameters", errno, "");
    return;
  }
  if (!feof (stdin)) {
    write_log_error ("Input parameters too long", ENOMEM, "");
    return;
  }
  if (!cgi_s_parse_params (query_string, 4096, params, &params_len, 16, 0)) {
    write_log_error ("Too many input parameters", E2BIG, "");
    return;
  }

  log_date_time = cgi_s_get_param (params, params_len, "log-date-time");
  if (!log_date_time)
    log_date_time = "";
  if (!log_date_time[0]) {
    localtime_r (&cur_time, &cur_date);
    strftime (str_cur_date, 64, "%F %T", &cur_date);
    log_date_time = str_cur_date;
  }
  log_name = cgi_s_get_param (params, params_len, "log-name");
  if (!log_name)
    log_name = "";
  if (!log_name[0]) {
    char *http_cookie = getenv ("HTTP_COOKIE");
    if (http_cookie) {
      char *session_log_name;
      cgi_s_parse_params (http_cookie, strlen (http_cookie) + 1,
			  cookies, &cookies_len, 16, 1);
      session_log_name =
	cgi_s_get_param (cookies, cookies_len, "dmy_log_name");
      if (session_log_name && session_log_name[0])
	log_name = session_log_name;
    }
  }
  log_action = cgi_s_get_param (params, params_len, "log-action");
  if (!log_action)
    log_action = "";

  if (snprintf (log_entry, 4096, "%s: %s: %s\n",
		log_date_time, log_name, log_action) >= 4096) {
    write_log_error ("Log entry too long", ENOMEM, log_entry);
    return;
  }

  /* Verify that input was not truncated.  */
  /* Verify all required parameters are provided.  */
  if (!feof (stdin) ||
      !log_name[0] || !log_action[0]) {
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
    log_dirname_end = strrchr (script_filename, '/');
    *log_dirname_end = '\0'; /* dirname () */
    log_dirname_len = log_dirname_end - script_filename;
    strcpy (log_dirname, script_filename);
    strcpy (log_filename, log_dirname);
    strcpy (log_filename + log_dirname_len, "/dmy_house_logs.log");
  } else
    log_filename[0] = '\0';

  /* Open with an exclusive write lock, append mode.  */
  fp = fopen (log_filename, "a");
  if (fp == NULL) {
    write_log_error ("Error opening log file", errno, log_entry);
    return;
  }
  lock_info.l_type = F_WRLCK;
  lock_info.l_whence = SEEK_SET;
  lock_info.l_start = 0;
  lock_info.l_len = 0;
  lock_info.l_pid = 0;
  if (fcntl (fileno (fp), F_SETLK, &lock_info) == -1) {
    write_log_error ("Error locking log file", errno, log_entry);
    return;
  }

  /* Append our log entry.  */
  fputs (log_entry, fp);

  /* Close.  */
  if (fclose (fp) == EOF) {
    write_log_error ("Error closing log file", errno, log_entry);
    return;
  }

  { /* Set the session login cookie.  */
     /* 10 minutes, alternative is 2 hours */
    time_t expire_time = cur_time + 10 * 60;
    struct tm expire_date;
    char str_expire_date[64];
    char str_web_cur_date[64];
    gmtime_r (&expire_time, &expire_date);
    cgi_s_fmt_date (&expire_date, str_expire_date, 64);
    cgi_s_fmt_date (&cur_date, str_web_cur_date, 64);
    cgi_p_header_cookie ("dmy_log_name", log_name, "/", str_expire_date);
    cgi_p_header_date (str_web_cur_date);
  }

  cgi_p_header_content_type ("text/html");
  cgi_p_end_header ();
  cgi_p_start_html_head ();
  cgi_p_head_http_equiv ("Content-Type", "text/html; charset=iso-8859-1");
  cgi_p_head_meta ("viewport", "width=device-width, initial-scale=1");
  cgi_p_head_title ("Successful signature");
  cgi_p_end_html_head ();
  cgi_p_h1 ("Successful signature");
  cgi_p_pre (log_entry);
  env_http_referer = getenv ("HTTP_REFERER");
  if (!env_http_referer)
    env_http_referer = "";
  cgi_p_p_a_href (env_http_referer, "Add another");
  cgi_p_p_a_href ("..", "Return to landing page");
  cgi_p_end_html ();

  return 0;
}
