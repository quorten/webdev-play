/* "Dumb" version of Perl CGI.pm, for use in faster alternative C
   programs that replicate the functionality.  */

#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <string.h>

#include "dumb_cgipm.h"

/* This is a shared variable that points to a dynamically allocated
   string.  */
StrReturn static_buffer = NULL;

/* A word on function code characters:
   "p": Print the result directly
   "s": Place the result in a statically allocated buffer
   "m": Place the result in a dynamically allocated buffer
*/

/* Verify that the given string is a valid HTTP token, i.e. it does
   not contain any non-ASCII characters, control characters, or HTTP
   delimiters.  Return non-zero on success, zero on failure.  */
int
cgi_s_verify_http_token (const char *text)
{
  char c;
  while ((c = *text++)) {
    if (c < 0x20 || c >= 0x80)
      return 0;
    switch (c) {
    case '(': case ')': case '<': case '>': case '@':
    case ',': case ';': case ':': case '\\': case '"':
    case '/': case '[': case ']': case '?': case '=':
    case '{': case '}':
    case ' ': case '\t':
      return 0;
    }
  }
  return 1;
}

/* Verify that the given string is a valid Content-Type MIME type,
   i.e. it does not contain any non-ASCII characters, control
   characters, or HTTP delimiters except '/'.  Also, the '/' character
   can only appear once.  */
int
cgi_s_verify_http_mime (const char *text)
{
  char found_slash = 0;
  char c;
  while ((c = *text++)) {
    if (c < 0x20 || c >= 0x80)
      return 0;
    switch (c) {
    case '/': 
      if (found_slash)
	return 0; /* Cannot have two or more slashes.  */
      found_slash = 1;
    case '(': case ')': case '<': case '>': case '@':
    case ',': case ';': case ':': case '\\': case '"':
              case '[': case ']': case '?': case '=':
    case '{': case '}':
    case ' ': case '\t':
      return 0;
    }
  }
  return 1;
}

/* Escape an HTTP header parameter as a quoted-string.  On overflow,
   NULL is returned and the buffer is garbled.

   Many HTTP header parameters do not allow escaping of special
   characters.  However, for those that do the protocol is as follows.

   * Surround the string with double quotes.

   * Remaining special characters, such as double quotes, can be
     escaped with a leading backslash.  */
char *
cgi_s_escape_http_str (char *buffer, unsigned buf_strlen, unsigned maxlen)
{
  char *old = buffer;
  char *old_end = buffer + buf_strlen;
  char *new = buffer;
  char *new_end = buffer + maxlen;
  *new++ = '"';
  if (new >= new_end)
    return NULL;
  while (old < old_end && new < new_end) {
    switch (*old) {
    case '\\':
    case '"':
      *new++ = '\\';
      if (new >= new_end)
	break;
      *new++ = *old++;
      break;
    default:
      *new++ = *old++;
      break;
    }
  }
  if (new >= new_end)
    return NULL;
  *new++ = '"';
  if (new >= new_end)
    return NULL;
  *new++ = '\0';
  return buffer;
}

/* To verify HTTP status code headers.  Status code number, separated
   by space, followed by TEXT until CRLF newline.  TEXT is any octet
   excluding CTLs, but including LWS (space & tab, optional leading
   CRLF).  However, we don't support leading CRLF in our usage because
   we already insert that before the user text.  */
int
cgi_s_verify_http_status (const char *text)
{
  char c;
  while ((c = *text++)) {
    if (c != '\t' && c < 0x20)
      return 0;
  }
  return 1;
}

/* Verify cookies?  Syntax is very complex to verify, actually.
   Cookie name is strictly an HTTP token.  (Percent-encoded for specials.)
   Path value is any char except CTLs and ';'.
   Cookie value?  Typically percent-encoded.  Sometimes base64 encoded.
   See list of characters excluded in RFC: US-ASCII, exclude
   CTLs, whitespace, DQUOTE, comma, semicolon, and backslash.
   %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E
*/

/* Unescape a URI string in place.  `maxlen' includes the null
   terminating character.  On overflow, NULL is returned and the
   buffer is garbled.  */
char *
cgi_s_unescape_uri (char *buffer, unsigned buf_strlen, unsigned maxlen)
{
  char *old = buffer;
  char *old_end = buffer + buf_strlen;
  char *new = buffer;
  char *new_end = buffer + maxlen;
  while (old < old_end && new < new_end) {
    switch (*old) {
    case '%':
      {
	unsigned value;
	char code[3];
	if (++old >= old_end)
	  break;
	code[0] = *old++;
	if (old >= old_end)
	  break;
	code[1] = *old++;
	code[2] = '\0';
	sscanf (code, "%x", &value);
	*new++ = (char)value;
	break;
      }
    default:
      *new++ = *old++;
      break;
    }
  }
  if (new >= new_end)
    return NULL;
  *new++ = '\0';
  return buffer;
}

/*  Almost the same as `cgi_s_unescape_uri ()', except that we also
    translate `+' signs to spaces.  */
char *
cgi_s_unescape_query_str (char *buffer, unsigned buf_strlen, unsigned maxlen)
{
  char *old = buffer;
  char *old_end = buffer + buf_strlen;
  char *new = buffer;
  char *new_end = buffer + maxlen;
  while (old < old_end && new < new_end) {
    switch (*old) {
    case '%':
      {
	unsigned value;
	char code[3];
	if (++old >= old_end)
	  break;
	code[0] = *old++;
	if (old >= old_end)
	  break;
	code[1] = *old++;
	code[2] = '\0';
	sscanf (code, "%x", &value);
	*new++ = (char)value;
	break;
      }
    case '+':
      old++;
      *new++ = ' ';
      break;
    default:
      *new++ = *old++;
      break;
    }
  }
  if (new >= new_end)
    return NULL;
  *new++ = '\0';
  return buffer;
}

void
cgi_p_escape_html (const char *text)
{
  /* Use unsigned char since we compare with values greater than 128.
     Mainly to eliminate a compiler warning.  */
  unsigned char c;
  while ((c = *text)) {
    switch (c) {
    case '<':
      putchar ('&');
      putchar ('l');
      putchar ('t');
      putchar (';');
      break;
    case '>':
      putchar ('&');
      putchar ('g');
      putchar ('t');
      putchar (';');
      break;
    case '&':
      putchar ('&');
      putchar ('a');
      putchar ('m');
      putchar ('p');
      putchar (';');
      break;
    /* CGI.pm also escapes these two characters, since some old
       browsers may misinterpret them as angle brackets.  These
       character substitutions only make sense in ISO 8859-1 or
       Windows-1252 character set.  */
    case 0x8b:
      putchar ('&');
      putchar ('#');
      putchar ('8');
      putchar ('2');
      putchar ('4');
      putchar ('9');
      putchar (';');
      break;
    case 0x9b:
      putchar ('&');
      putchar ('#');
      putchar ('8');
      putchar ('2');
      putchar ('5');
      putchar ('0');
      putchar (';');
      break;
    default:
      putchar (c);
      break;
    }
    text++;
  }
}

/*  Almost the same as `cgi_p_escape_html ()', except that we also
    escape double quotes.  */
void
cgi_p_escape_att_dq (const char *text)
{
  /* Use unsigned char since we compare with values greater than 128.
     Mainly to eliminate a compiler warning.  */
  unsigned char c;
  while ((c = *text)) {
    switch (c) {
    case '<':
      putchar ('&');
      putchar ('l');
      putchar ('t');
      putchar (';');
      break;
    case '>':
      putchar ('&');
      putchar ('g');
      putchar ('t');
      putchar (';');
      break;
    case '&':
      putchar ('&');
      putchar ('a');
      putchar ('m');
      putchar ('p');
      putchar (';');
      break;
    case '"':
      putchar ('&');
      putchar ('q');
      putchar ('u');
      putchar ('o');
      putchar ('t');
      putchar (';');
      break;
    /* CGI.pm also escapes these two characters, since some old
       browsers may misinterpret them as angle brackets.  These
       character substitutions only make sense in ISO 8859-1 or
       Windows-1252 character set.  */
    case 0x8b:
      putchar ('&');
      putchar ('#');
      putchar ('8');
      putchar ('2');
      putchar ('4');
      putchar ('9');
      putchar (';');
      break;
    case 0x9b:
      putchar ('&');
      putchar ('#');
      putchar ('8');
      putchar ('2');
      putchar ('5');
      putchar ('0');
      putchar (';');
      break;
    default:
      putchar (c);
      break;
    }
    text++;
  }
}

/********************************************************************/

/* `maxlen' includes the null terminating character.  */
char *
cgi_s_fmt_date (struct tm *date, char *buffer, unsigned maxlen)
{
  /* We must use the C locale for this date formatting.  */
  const char *old_locale = setlocale (LC_TIME, NULL);
  setlocale (LC_TIME, "C");
  strftime (buffer, maxlen, "%a, %d-%b-%Y %T %Z", date);
  setlocale (LC_TIME, old_locale);
  return buffer;
}

/* `content_type' cannot be escaped.  It must be verified with
   `cgi_s_verify_http_mime ()' before using this function, otherwise
   the results will be a malformed HTTP header.  */
void
cgi_p_header_content_type (const char *content_type)
{
  fputs ("Content-Type: ", stdout);
  fputs (content_type, stdout);
  fputs ("; charset=ISO-8859-1\r\n", stdout);
}

/* `status' cannot be escaped.  It must be verified with
   `cgi_s_verify_http_status ()' before using this function, otherwise
   the results will be a malformed HTTP header.  */
void
cgi_p_header_status (const char *status)
{
  fputs ("Status: ", stdout);
  fputs (status, stdout);
  fputs ("\r\n", stdout);
}

/* `date' cannot be escaped.  It must be a valid HTTP date format
   generated with `cgi_s_fmt_date ()', otherwise the results will be a
   malformed HTTP header.  */
void
cgi_p_header_date (const char *date)
{
  fputs ("Date: ", stdout);
  fputs (date, stdout);
  fputs ("\r\n", stdout);
}

/* Cookie header format is as follows:

Set-Cookie: dmy_log_name=two; path=/; expires=Mon, 03-Sep-2018 17:49:09 GMT
Date: Mon, 03 Sep 2018 17:39:09 GMT

 */
void
cgi_p_header_cookie (const char *name, const char *value,
		     const char *path, const char *expires)
{
  fputs ("Set-Cookie: ", stdout);
  /* TODO FIXME: Check fields and escape them if necessary.  */
  fputs (name, stdout);
  fputs ("=", stdout);
  fputs (value, stdout);
  fputs ("; path=", stdout);
  fputs (path, stdout);
  fputs ("; expires=", stdout);
  fputs (expires, stdout);
  fputs ("\r\n", stdout);
}

void
cgi_p_end_header (void)
{
  fputs ("\r\n", stdout);
}

/********************************************************************/

/* Fill out a string pointer array with alternating key-value items
   from parsed text input.  Accepts either query string params or
   cookie.  The original string is used as a buffer: unescaping and
   null termination are done in-place.  If the string array overflows,
   returns zero, otherwise returns 1.

   Input formats are as follows:

   Query string params: one=two&three=four
   Cookie: one=two; three=four

   Both are percent-encoded for special characters inside key or value
   names.

*/
int
cgi_s_parse_params (char *buffer, unsigned maxlen,
		    char **strs, unsigned *strs_len, unsigned max_strs,
		    int cookie_mode)
{
  int no_overflow = 1;
  char *old = buffer;
  char *old_end = buffer + strlen (buffer);
  char *first_whitespace = NULL; /* first trailing whitespace */
  char *new = buffer;
  char *new_end = buffer + maxlen;
  char **new_strs = strs;
  char **strs_end = strs + max_strs;
  char in_value = 0;
  /* TODO FIXME: Detect integrity errors and flag parse errors.  */
  while (old < old_end) {
    /* TODO FIXME KEY ASSUMPTION: Key-values always come in matching
       pairs.  Otherwise we'd need more logic to push empty
       strings.  */
    if (cookie_mode && *old == ' ') {
      if (old == new)
	new++; /* skip leading whitespace */
      else if (first_whitespace == NULL)
	first_whitespace = old;
      old++;
    } else if (*old == '=' ||
	(!cookie_mode && *old == '&') ||
	(cookie_mode && *old == ';')) {
      unsigned new_strlen;
      in_value = (*old == '=') ? 1 : 0;
      if (first_whitespace == NULL)
	new_strlen = old - new;
      else
	new_strlen = first_whitespace - new;
      if (!cookie_mode)
	cgi_s_unescape_query_str (new, new_strlen, new_strlen + 1);
      else
	cgi_s_unescape_uri (new, new_strlen, new_strlen + 1);
      if (new_strs >= strs_end) {
	no_overflow = 0;
	goto cleanup;
      }
      *new_strs++ = new;
      old++; /* skip '=' or '&' */
      first_whitespace = NULL;
      new = old;
    } else
      old++;
  }
  /* Handle adding very last argument when there is no separator
     character.  */
  if (new > old || in_value) {
    /* TODO FIXME DUPLICATE CODE */
    unsigned new_strlen;
    if (first_whitespace == NULL)
      new_strlen = old - new;
    else
      new_strlen = old - first_whitespace;
    if (!cookie_mode)
      cgi_s_unescape_query_str (new, new_strlen, new_strlen + 1);
    else
      cgi_s_unescape_uri (new, new_strlen, new_strlen + 1);
    if (new_strs >= strs_end) {
      no_overflow = 0;
      goto cleanup;
    }
    *new_strs++ = new;
  }
 cleanup:
  *strs_len = new_strs - strs;
  return no_overflow;
}

/* Given a parsed params structure, return the value of the requested
   key, or NULL if there is no such key.  */
char *
cgi_s_get_param (char **strs, unsigned strs_len, char *key)
{
  char **strs_end = strs + strs_len;
  while (strs < strs_end) {
    if (!strcmp (key, *strs)) {
      return strs[1];
    }
    strs += 2;
  }
  return NULL;
}

/********************************************************************/

void
cgi_p_start_html_head (void)
{
  fputs (
"<!DOCTYPE html\n"
"	PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
"	 \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
"<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en-US\" xml:lang=\"en-US\">\n"
"<head>\n",
         stdout);
}

void
cgi_p_head_http_equiv (const char *http_equiv, const char *content)
{
  fputs ("<meta http-equiv=\"", stdout);
  cgi_p_escape_att_dq (http_equiv);
  fputs ("\" content=\"", stdout);
  cgi_p_escape_att_dq (content);
  fputs ("\" />\n", stdout);
}

void
cgi_p_head_meta (const char *name, const char *content)
{
  fputs ("<meta name=\"", stdout);
  cgi_p_escape_att_dq (name);
  fputs ("\" content=\"", stdout);
  cgi_p_escape_att_dq (content);
  fputs ("\" />\n", stdout);
}

void
cgi_p_head_title (const char *title)
{
  fputs ("<title>", stdout);
  cgi_p_escape_html (title);
  fputs ("</title>\n", stdout);
}

void
cgi_p_end_html_head (void)
{
  fputs ("</head>\n<body>\n", stdout);
}

void
cgi_p_end_html (void)
{
  fputs ("</body>\n</html>\n", stdout);
}

/********************************************************************/

void
cgi_p_p (const char *text)
{
  fputs ("<p>", stdout);
  cgi_p_escape_html (text);
  fputs ("</p>", stdout);
}

void
cgi_p_pre (const char *text)
{
  fputs ("<pre>", stdout);
  cgi_p_escape_html (text);
  fputs ("</pre>", stdout);
}

void
cgi_p_a_href (const char *href, const char *text)
{
  fputs ("<a href=\"", stdout);
  cgi_p_escape_att_dq (href);
  fputs ("\">", stdout);
  cgi_p_escape_html (text);
  fputs ("</a>", stdout);
}

void
cgi_p_p_a_href (const char *href, const char *text)
{
  fputs ("<p>", stdout);
  fputs ("<a href=\"", stdout);
  cgi_p_escape_att_dq (href);
  fputs ("\">", stdout);
  cgi_p_escape_html (text);
  fputs ("</a>", stdout);
  fputs ("</p>", stdout);
}

void
cgi_p_h1 (const char *text)
{
  fputs ("<h1>", stdout);
  cgi_p_escape_html (text);
  fputs ("</h1>", stdout);
}

void
cgi_p_h2 (const char *text)
{
  fputs ("<h2>", stdout);
  cgi_p_escape_html (text);
  fputs ("</h2>", stdout);
}
