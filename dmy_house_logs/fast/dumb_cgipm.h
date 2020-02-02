#ifndef DUMB_CGIPM_H
#define DUMB_CGIPM_H

#include <time.h>

typedef char ** StrReturn;

StrReturn static_buffer;

char *cgi_s_unescape_uri (char *buffer, unsigned buf_strlen,
			  unsigned maxlen);
char * cgi_s_unescape_query_str (char *buffer,
				 unsigned buf_strlen, unsigned maxlen);
void cgi_p_escape_html (const char *text);
void cgi_p_escape_att_dq (const char *text);

char *cgi_s_fmt_date (struct tm *date, char *buffer, unsigned maxlen);
void cgi_p_header_content_type (const char *content_type);
void cgi_p_header_status (const char *status);
void cgi_p_header_date (const char *date);
void cgi_p_header_cookie (const char *name, const char *value,
			  const char *path, const char *expires);
void cgi_p_end_header (void);

int cgi_s_parse_params (char *buffer, unsigned maxlen,
			char **strs, unsigned *strs_len, unsigned max_strs,
			int cookie_mode);
char *cgi_s_get_param (char **strs, unsigned strs_len, char *key);

void cgi_p_start_html_head (void);
void cgi_p_head_http_equiv (const char *http_equiv, const char *content);
void cgi_p_head_meta (const char *name, const char *content);
void cgi_p_head_title (const char *title);
void cgi_p_end_html_head (void);
void cgi_p_end_html (void);

void cgi_p_p (const char *text);
void cgi_p_pre (const char *text);
void cgi_p_a_href (const char *href, const char *text);
void cgi_p_p_a_href (const char *href, const char *text);
void cgi_p_h1 (const char *text);
void cgi_p_h2 (const char *text);

#endif /* not DUMB_CGIPM_H */
