#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "dumb_cgipm.h"

void
read_log_error (char *user_err_desc, int err_code)
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
  cgi_p_p ("Could not read the log.  "
	   "Contact your website administrator."); putchar ('\n');
  cgi_p_h2 ("Error code:"); putchar ('\n');
  cgi_p_pre (open_read_error); putchar ('\n');
  cgi_p_p_a_href ("..", "Return to landing page"); putchar ('\n');
}

int
main (void)
{
  char buffer[4096];
  FILE *fp = fopen ("dmy_house_logs.log", "r");
  if (fp == NULL) {
    read_log_error ("Error opening log file", errno);
    return 0;
  }

  cgi_p_header_content_type ("text/html");
  cgi_p_end_header ();
  cgi_p_start_html_head ();
  cgi_p_head_http_equiv ("Content-Type", "text/html; charset=iso-8859-1");
  cgi_p_head_meta ("viewport", "width=device-width, initial-scale=1");
  cgi_p_head_title ("View house logs");
  cgi_p_end_html_head ();

  /* NOTE: I would like to use the bulleted list style to show the
     logs, but I need to adjust the CSS styling in order for it to
     look nice.  Also note that the current implementation of many
     individual standard output calls can slow it down due to
     insufficient buffering.  */
  fputs ("<pre>\n", stdout);
  /* fputs ("<ul>\n", stdout); */
  while (fgets (buffer, 4096, fp) != NULL) {
    /* Remove the trailing newline, if applicable.  */
    /* unsigned buf_len = strlen (buffer);
    if (buf_len > 0) {
      unsigned nl_pos = buf_len - 1;
      if (buffer[nl_pos] == '\n')
	buffer[nl_pos] = '\0';
    } */
    /* fputs ("<li><code>", stdout); */
    if (strstr (buffer, ": Laundry: ") == NULL)
      continue;
    cgi_p_escape_html (buffer);
    /* fputs ("</code></li>\n", stdout); */
  }
  if (fclose (fp) == EOF) {
    read_log_error ("Error closing log file", errno);
    return 0;
  }
  fputs ("</pre>\n", stdout);
  /* fputs ("</ul>\n", stdout); */

  fputs ("<script type=\"text/javascript\">window.scrollTo(0,document.body.scrollHeight);</script>\n", stdout);
  cgi_p_end_html ();

  return 0;
}
