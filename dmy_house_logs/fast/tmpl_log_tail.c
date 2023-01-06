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

/* Determine the template name from the program name.  Basically
   basename() followed by replacing "_log_tail.cgi" with ".html".
   Returns 1 on success, 0 on error.  */
int
gen_tmpl_name (char *tmpl_name, unsigned tmpl_name_limit,
	       const char *prog_name)
{
  const char *suffix = "_log_tail.cgi";
  const unsigned suffix_len = 13;
  const char *new_suffix = ".html";
  const unsigned new_suffix_len = 5;
  const char *basename;
  unsigned basename_len;
  unsigned suffix_pos;

  basename = strrchr (prog_name, '/');
  if (basename == NULL)
    basename = prog_name;
  else
    basename++; /* Skip '/' */
  basename_len = strlen (basename);
  suffix_pos = basename_len - suffix_len;
  if (strcmp (basename + suffix_pos, suffix)) {
    tail_read_log_error ("Incorrect template name", EINVAL);
    return 0;
  }
  if (suffix_pos + new_suffix_len + 1 >= tmpl_name_limit) {
    tail_read_log_error ("Template name too long", ENOMEM);
    return 0;
  }
  strncpy (tmpl_name, basename, suffix_pos);
  strcpy (tmpl_name + suffix_pos, new_suffix);
  return 1;
}

/* Seek backwards on a file by the given number of lines.
   `file_size`, if non-NULL, is filled in with the file size.  Returns
   1 on success, 0 on failure.

   N.B.: Additional functions that could be had with cheap
   modifications to this function.  We could do a block-wise read of
   the file in reverse, if the user does not mind processing the file
   one block at a time.  We could count up the length of each line so
   the user can allocate an array of pointers to the beginning of
   lines.  We could replace each newline with null-termination in the
   block-wise read.  We could search for a key line in reverse by
   first checking for a line of matching length, then doing an strcmp
   (or two if it crosses a block boundary).  */
int
scan_lines_rev (FILE *fp, unsigned num_lines, unsigned long *file_size)
{
  char block[4096];
  unsigned last_blksize, blksize;
  unsigned pos; /* Position in block.  */

  if (fseek (fp, 0, SEEK_END) == -1)
    return 0;
  if (num_lines == 0)
    return 0; /* Nothing else to do.  */

  if (file_size != NULL)
    *file_size = ftell (fp);

  /* If the last character in the file is a newline, then we count it
     as part of the last line.  If it isn't a newline, then we still
     count it as part of the last line.  In other words, we ignore it
     in either case.  */
  if (fseek (fp, -1, SEEK_CUR) == -1)
    return 0;
  num_lines--;

  /* Compute the first block size to be the remainder block that is
     not divisible by the block size.  */
  last_blksize = 0;
  blksize = ftell (fp) & (4096 - 1);
  if (blksize == 0)
    blksize = 4096;

  /* Now read in reverse in a block-wise manner until we get to the
     desired position or the beginning of the file.  */
  do {
    /* Seek backwards by the previous block size read plus the next
       block size that we want to read.  */
    if (fseek (fp, -(last_blksize + blksize), SEEK_CUR) == -1) {
      /* We may have touched the beginning of the file.  Try to seek
	 to the beginning of the file.  */
      if (fseek (fp, -last_blksize, SEEK_CUR) == -1)
	return 0; /* Error */
      return 1; /* Success */
    }
    if (fread (block, blksize, 1, fp) != 1)
      return 0;
    pos = blksize;
    do {
      pos--;
      if (block[pos] == '\n') {
	if (num_lines > 0)
	  num_lines--;
	else {
	  /* We have found the end of the line before the desired
	     line.  Increment to get to the beginning of the desired
	     line.  */
	  pos++;
	  /* Now seek to the position.  */
	  if (fseek (fp, -blksize + pos, SEEK_CUR) == -1)
	    return 0;
	  return 1; /* Success */
	}
      }
    } while (pos > 0);
    last_blksize = blksize;
    /* All subsequent block sizes are the full size.  */
    blksize = 4096;
  } while (1);

  /* NOT REACHED */
  return 0;
}

void
test_scan_lines_rev (void)
{
  char buffer[4096];
  FILE *fp = fopen ("tfile", "r");
  if (fp == NULL) {
    puts ("FAIL");
    return;
  }

  if (!scan_lines_rev (fp, 1, NULL))
    puts ("FAIL");
  puts ("RESULTS 1:");
  while (fgets (buffer, 4096, fp) != NULL)
    fputs (buffer, stdout);
  puts ("END");

  if (!scan_lines_rev (fp, 5, NULL))
    puts ("FAIL");
  puts ("RESULTS 2:");
  while (fgets (buffer, 4096, fp) != NULL)
    fputs (buffer, stdout);
  puts ("END");

  if (!scan_lines_rev (fp, 100, NULL))
    puts ("FAIL");
  puts ("RESULTS 3:");
  while (fgets (buffer, 4096, fp) != NULL)
    fputs (buffer, stdout);
  puts ("END");

  /* Now this last test case should be big enough to print the whole
     file.  */
  if (!scan_lines_rev (fp, 500, NULL))
    puts ("FAIL");
  puts ("RESULTS 4:");
  while (fgets (buffer, 4096, fp) != NULL)
    fputs (buffer, stdout);
  puts ("END");

  fclose (fp);
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
  char *log_lines;
  unsigned long flog_size, tail_size;
  char *tail_pos;
  char *env_script_filename;
  char *log_dirname_end;
  FILE *flog, *ftmpl;
  struct flock lock_info;

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

  /* Open the log file with a read lock.  */
  flog = fopen (log_filename, "r");
  if (flog == NULL) {
    tail_read_log_error ("Error opening log file", errno);
    return;
  }
  lock_info.l_type = F_RDLCK; /* F_WRLCK */
  lock_info.l_whence = SEEK_SET;
  lock_info.l_start = 0;
  lock_info.l_len = 0;
  lock_info.l_pid = 0;
  if (fcntl (fileno (flog), F_SETLK, &lock_info) == -1) {
    tail_read_log_error ("Error locking log file", errno);
    fclose (flog);
    return;
  }

  /* Parse the log file backwards by the desired number of lines.  */
  if (!scan_lines_rev (flog, log_tail_len, &flog_size)) {
    tail_read_log_error ("Error scanning log file", errno);
    fclose (flog);
    return;
  }

  /* Read the log lines into a buffer.  */
  tail_size = flog_size - ftell (flog);
  log_lines = (char*) malloc (sizeof (char) * (tail_size + 1));
  if (log_lines == NULL) {
    tail_read_log_error ("Error allocating memory", errno);
    fclose (flog);
    return;
  }
  if (fread (log_lines, tail_size, 1, flog) != 1) {
    tail_read_log_error ("Error reading log file", errno);
    fclose (flog);
    goto cleanup_memory;
  }
  log_lines[tail_size] = '\0'; /* Null-terminate the string.  */
  tail_pos = log_lines;

  /* Close.  */
  if (fclose (flog) == EOF) {
    tail_read_log_error ("Error closing log file", errno);
    goto cleanup_memory;
  }

  /* Read the template file, and paste the log lines into the template
     text.  */
  ftmpl = fopen (tmpl_name, "r");
  if (ftmpl == NULL) {
    tail_read_log_error ("Error opening template file", errno);
    goto cleanup_memory;
  }
  cgi_p_header_content_type ("text/html");
  cgi_p_end_header ();
  while (fgets (linebuf, 4096, ftmpl) != NULL) {
    if (strstr (linebuf, "<!-- INSERT LOG TAIL HERE -->") != NULL) {
      /* TODO FIXME: This code will fail to print the last line if it
	 is not terminated by a newline.  */
      char *line_end = strchr (tail_pos, '\n');
      while (line_end != NULL) {
	/* "chomp" trailing newline */
	*line_end = '\0';
	cgi_p_escape_html (tail_pos);
	fputs ("<br />\n", stdout);
	tail_pos = line_end + 1;
	line_end = strchr (tail_pos, '\n');
      }
    } else
      fputs (linebuf, stdout);
  }
  if (fclose (ftmpl) == EOF) {
    /* TODO FIXME: REVIEW CODE for instances where we may be calling
       the header error routine butshould just print to stderr because
       we already started writing the header.  */
    /* TODO FIXME REFACTOR: Use a single standard subroutine for this
       common formatting to stderr.  */
    fprintf (stderr, "Log read error code: %s: %s\n",
	     "Error closing template file", strerror (errno));
    goto cleanup_memory;
  }

 cleanup_memory:
  free (log_lines);
}
