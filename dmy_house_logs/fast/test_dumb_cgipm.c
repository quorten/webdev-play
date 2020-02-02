#include <stdio.h>
#include <time.h>
#include <string.h>

#include "dumb_cgipm.h"

void show_strs (char **strs, unsigned strs_len);

int
main (void) {
  char str_date[64];

  {
    char buf1[64] = "Hello%20there,%20take%20%3Cthis%3e%21";
    printf ("cgi_s_unescape_uri: buf1: Before: %s\n", buf1);
    cgi_s_unescape_uri (buf1, strlen (buf1), 64);
    printf ("cgi_s_unescape_uri: buf1: After:  %s\n", buf1);
  }
  {
    char buf2[26] = "Try%20this%20tight%20fit.";
    printf ("cgi_s_unescape_uri: buf2: Before: %s\n", buf2);
    cgi_s_unescape_uri (buf2, 25, 26);
    printf ("cgi_s_unescape_uri: buf2: After:  %s\n", buf2);
  }
  {
    char buf3[64] = "Try <this> & try that.";
    printf ("cgi_s_escape_html: buf3: Before: %s\n", buf3);
    fputs ("cgi_s_escape_html: buf3: After:  ", stdout);
    cgi_p_escape_html (buf3);
    putchar ('\n');
  }
  {
    char buf4[64] = "Try <this> & try \"that\".";
    printf ("cgi_s_escape_att_dq: buf4: Before: %s\n", buf4);
    fputs ("cgi_s_escape_att_dq: buf4: After:  ", stdout);
    cgi_p_escape_att_dq (buf4);
    putchar ('\n');
  }

  /****************************************/

  {
    time_t cur_time = time (NULL);
    struct tm date;
    gmtime_r (&cur_time, &date);
    cgi_s_fmt_date (&date, str_date, 64);
    printf ("cgi_s_fmt_date: str_date: After:  %s\n", str_date);
  }
  {
    fputs ("cgi_p_header_content_type: After:  ", stdout);
    cgi_p_header_content_type ("text/html");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_header_status: After:  ", stdout);
    cgi_p_header_status ("500 Internal Server Error");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_header_date: After:  ", stdout);
    cgi_p_header_date (str_date);
    putchar ('\n');
  }
  {
    fputs ("cgi_p_header_cookie: After:  ", stdout);
    cgi_p_header_cookie ("my-cookie", "hello", "/", str_date);
    putchar ('\n');
  }
  {
    fputs ("cgi_p_end_header: After:  ", stdout);
    cgi_p_end_header ();
    putchar ('\n');
  }

  /****************************************/

  {
    char *strs[16];
    unsigned strs_len = 0;
    char *result;
    char buf5[64] = "one=two&three=four&five=six";
    char buf6[64] = "one=&three=";
    char buf7[64] = "my-cookie=hello; next-cookie=okay";
    char buf8[64] = "my-cookie   =   hello;  next-cookie = okay";
    char buf9[64] = "lot%27s=crazy%24&result=but%20it%20still%20works";

    strs_len = 0;
    cgi_s_parse_params (buf5, 64, strs, &strs_len, 16, 0);
    fputs ("cgi_s_parse_params: buf5: strs:   ", stdout);
    show_strs (strs, strs_len);
    putchar ('\n');

    result = cgi_s_get_param (strs, strs_len, "blah");
    if (result == NULL)
      fputs ("Verified key \"blah\" does not exist.\n", stdout);
    else
      fputs ("Found key \"blah\", but it shouldn't exist!\n", stdout);
    result = cgi_s_get_param (strs, strs_len, "three");
    if (result == NULL)
      fputs ("Failed to find key \"three\"!\n", stdout);
    else {
      fputs ("Found key \"three\", value: ", stdout);
      fputs (result, stdout);
      putchar ('\n');
    }

    strs_len = 0;
    cgi_s_parse_params (buf6, 64, strs, &strs_len, 16, 0);
    fputs ("cgi_s_parse_params: buf6: strs:   ", stdout);
    show_strs (strs, strs_len);
    putchar ('\n');
    strs_len = 0;
    cgi_s_parse_params (buf7, 64, strs, &strs_len, 16, 1);
    fputs ("cgi_s_parse_params(cookie): buf7: strs:   ", stdout);
    show_strs (strs, strs_len);
    putchar ('\n');
    strs_len = 0;
    cgi_s_parse_params (buf8, 64, strs, &strs_len, 16, 1);
    fputs ("cgi_s_parse_params(cookie): buf8: strs:   ", stdout);
    show_strs (strs, strs_len);
    putchar ('\n');
    strs_len = 0;
    cgi_s_parse_params (buf9, 64, strs, &strs_len, 16, 0);
    fputs ("cgi_s_parse_params: buf9: strs:   ", stdout);
    show_strs (strs, strs_len);
    putchar ('\n');
  }

  /****************************************/

  {
    fputs ("cgi_p_start_html_head: After:  ", stdout);
    cgi_p_start_html_head ();
    putchar ('\n');
  }
  {
    fputs ("cgi_p_head_http_equiv: After:  ", stdout);
    cgi_p_head_http_equiv ("Content-Type", "text/html; charset=iso-8859-1");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_head_meta: After:  ", stdout);
    cgi_p_head_meta ("viewport", "width=device-width, initial-scale=1");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_head_title: After:  ", stdout);
    cgi_p_head_title ("My page title");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_end_html_head: After:  ", stdout);
    cgi_p_end_html_head ();
    putchar ('\n');
  }
  {
    fputs ("cgi_p_end_html: After:  ", stdout);
    cgi_p_end_html ();
    putchar ('\n');
  }

  /****************************************/

  {
    fputs ("cgi_p_p: After:  ", stdout);
    cgi_p_p ("This is my paragraph.");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_pre: After:  ", stdout);
    cgi_p_pre ("This is some <code>.");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_a_href: After:  ", stdout);
    cgi_p_a_href ("http://example.com/", "This is my link.");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_p_a_href: After:  ", stdout);
    cgi_p_p_a_href ("http://example.com/test", "This is another link.");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_h1: After:  ", stdout);
    cgi_p_h1 ("This is a level 1 header.");
    putchar ('\n');
  }
  {
    fputs ("cgi_p_h2: After:  ", stdout);
    cgi_p_h2 ("This is a level 2 header.");
    putchar ('\n');
  }

  return 0;
}

void
show_strs (char **strs, unsigned strs_len)
{
  char start = 1;
  char **strs_end = strs + strs_len;
  while (strs < strs_end) {
    if (!start)
      putchar (',');
    else
      start = 0;
    fputs (*strs, stdout);
    strs++;
  }
}
