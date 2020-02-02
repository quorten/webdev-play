/* NOTE: Why do we have this source file that we do not use?  I
   thought getting the tail of the log by parsing the whole file was
   inefficient, and indeed it is... but this optimization has not
   resulted in much of a measurable gain of a speedup, not yet at
   least.  I keep the code here because it works.  Ideally, I will
   take this kind of hacky but fast code and turn it into some nicer
   shared library code for reading a text file backwards.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "dumb_cgipm.h"

void
tail_read_log_error (char *user_err_desc, int err_code)
{
  char open_read_error[512];
  snprintf (open_read_error, 512, "%s: %s",
	   user_err_desc, strerror (err_code));
  fprintf (stderr, "Log read error code: %s\n", open_read_error);

  cgi_p_header_status ("500 Internal Server Error");
  cgi_p_header_content_type ("text/html");
  cgi_p_end_header ();
  cgi_p_start_html_head ();
  cgi_p_head_http_equiv ("Content-Type", "text/html; charset=iso-8859-1");
  cgi_p_head_meta ("viewport", "width=device-width, initial-scale=1");
  cgi_p_head_title ("500 Internal Server Error");
  cgi_p_end_html_head ();
  cgi_p_h1 ("500 Internal Server Error"); putchar ('\n');
  cgi_p_p ("Could not generate sign your name page with log tail.  "
	   "Contact your website administrator."); putchar ('\n');
  cgi_p_h2 ("Error code:"); putchar ('\n');
  cgi_p_pre (open_read_error); putchar ('\n');
  cgi_p_p_a_href ("..", "Return to landing page"); putchar ('\n');
}

void
gen_tmpl_log_tail (unsigned log_tail_len, const char *tmpl_name)
{
  char script_filename[260];
  char script_dirname[260];
  char log_dirname[260];
  unsigned log_dirname_len;
  char log_filename[260];
  char linebuf[4096];
  /* Includes one extra pointer for one past the end of the last
     line.  */
  char *log_lines[32];
  char log_lines_buf[32*4096];
  unsigned num_log_lines = 0;
  unsigned log_lines_buf_len = 0;
  char *env_script_filename;
  char *log_dirname_end;
  FILE *fp;
  struct flock lock_info;

  /* Generate log filename, relative to script filename.  */
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

  /* Open the log file with a read lock.  */
  fp = fopen (log_filename, "r");
  if (fp == NULL) {
    tail_read_log_error ("Error opening log file", errno);
    return;
  }
  lock_info.l_type = F_RDLCK; /* F_WRLCK */
  lock_info.l_whence = SEEK_SET;
  lock_info.l_start = 0;
  lock_info.l_len = 0;
  lock_info.l_pid = 0;
  if (fcntl (fileno (fp), F_SETLK, &lock_info) == -1) {
    tail_read_log_error ("Error locking log file", errno);
    fclose (fp);
    return;
  }

  { /* Read the tail of the log lines into an array, assuming line
       lengths are bounded to the max length.  */
    unsigned file_len;
    if (fseek (fp, 0, SEEK_END) == EOF) {
      tail_read_log_error ("Error seeking log file", errno);
      fclose (fp);
      return;
    }
    file_len = ftell (fp);

    if (file_len < /* sizeof (char) * */ 32 * 4096)
      log_lines_buf_len = file_len + 1;
    else
      log_lines_buf_len = /* sizeof (char) * */ 32 * 4096;
    log_lines_buf[log_lines_buf_len-1] = '\0';

    if (fseek (fp, -(int)(log_lines_buf_len - 1), SEEK_END) == EOF) {
      tail_read_log_error ("Error seeking log file", errno);
      fclose (fp);
      return;
    }
    if (fread (log_lines_buf, log_lines_buf_len - 1, 1, fp) != 1) {
      tail_read_log_error ("Error reading log file", errno);
      fclose (fp);
      return;
    }
  }

  { /* Parse the log tail backwards to find the line starts and
       lengths for the given number of tail lines.  */
    /* NOTE TODO FIXME: We assume there must be a newline at the end
       of the file.  */
    unsigned i = log_tail_len + 1;
    unsigned j = log_lines_buf_len - 1;
    /* Save a pointer to one beyond the end of the last line.  */
    log_lines[--i] = &log_lines_buf[j];
    while (i > 0 && j > 0) {
      int first_newline = 0;
      /* Search backwards for the beginning of our line.  */
      while (j > 0) {
	j--;
	if (log_lines_buf[j] == '\n') {
	  if (!first_newline)
	    first_newline = 1;
	  else /* found end of previous line */
	    { j++; break; }
	}
      }
      /* Store this beginning of line.  */
      log_lines[--i] = &log_lines_buf[j];
    }
    /* TODO FIXME TEST: skip_log_lines */
    num_log_lines = log_tail_len - i;
  }

  if (0) {
    /* OLD code that reads entire file from beginning to end and saves
       only the tail lines.  Inefficient.  */
    /* TODO FIXME: Check for read errors here too.  */
    while (fgets (log_lines[num_log_lines], 4096, fp) != NULL) {
      num_log_lines++;
      if (num_log_lines > log_tail_len) {
	memmove (log_lines[0], log_lines[1],
		 sizeof (log_lines[0]) * (num_log_lines - 1));
	num_log_lines--;
      }
    }
  }

  /* Close.  */
  if (fclose (fp) == EOF) {
    tail_read_log_error ("Error closing log file", errno);
    return;
  }

  /* Read the template file, and paste the log lines into the template
     text.  */
  fp = fopen (tmpl_name, "r");
  if (fp == NULL) {
    tail_read_log_error ("Error opening template file", errno);
    return;
  }
  cgi_p_header_content_type ("text/html");
  cgi_p_end_header ();
  while (fgets (linebuf, 4096, fp) != NULL) {
    if (strstr (linebuf, "<!-- INSERT LOG TAIL HERE -->") != NULL) {
      unsigned i;
      unsigned skip_log_lines = log_tail_len - num_log_lines;
      for (i = skip_log_lines; i < log_tail_len; i++) {
	/* "chomp" trailing newline */
	*(log_lines[i+1] - 1) = '\0';
	cgi_p_escape_html (log_lines[i]);
	fputs ("<br />\n", stdout);
      }
      /* for (i = 0; i < num_log_lines; i++) {
	/\* "chomp" trailing newline *\/
	log_lines[i][strlen(log_lines[i])-1] = '\0';
	cgi_p_escape_html (log_lines[i]);
	fputs ("<br />\n", stdout);
      } */
    } else
      fputs (linebuf, stdout);
  }
  if (fclose (fp) == EOF) {
    tail_read_log_error ("Error closing template file", errno);
    return;
  }
}
