#ifndef ZPN_HTTP_H
#define ZPN_HTTP_H

#define ZPN_GET_HTTP_REQUEST  4096
#define ZPN_GET_HTTP_HEADER   12288
#define ZPN_GET_HTTP_COOKIE   4096
#define ZPN_PORT_HTTP         80

/* structure _zpn_http can store informations about GET/POST requests	*
 * -------------------------------------------------------------------- *
 * char header[] : contains header					*
 * char content  : contains content after header			*/
typedef struct _zpn_http {

	char header[ZPN_GET_HTTP_HEADER];
	char cookies[ZPN_GET_HTTP_COOKIE];
	char *content;
	int content_length;
	int code;

} t_zpn_http, *p_zpn_http;

/* zpn_http_get send a 'GET' request to a website				*
 * ------------------------------------------------------------ *
 * const char *get  : request GET								*
 * const char *host : adress of the host						*
 * const char *user_agent : [CAN BE NULL] user_agent			*
 * const char *cookie : [CAN BE NULL] cookies					*
 * const char *connection : connection type						*
 * p_zpn_http http : [OUTPUT] structure of the http header		*
 * ------------------------------------------------------------ *
 * return : ZPN_OK if ok, else ZPN_ERROR						*/
int zpn_http_get (const char *get, const char *host, 
		  const char *user_agent, const char *cookie,
		  const char *connection, p_zpn_http http);

/* zpn_http_post send a 'POST' request to a website				*
 * ------------------------------------------------------------ *
 * const char *request  : request POST							*
 * const char *host : adress of the host						*
 * const char *user_agent : [CAN BE NULL] user_agent			*
 * const char *cookie : [CAN BE NULL] cookies					*
 * const char *connection : connection type						*
 * const char *body : body of POST request						*
 * p_zpn_http http : [OUTPUT] structure of the http header		*
 * ------------------------------------------------------------ *
 * return : ZPN_OK if ok, else ZPN_ERROR						*/
int zpn_http_post (const char *request, const char *host, 
		  const char *user_agent, const char *cookie,
		  const char *connection, const char *post, 
		  p_zpn_http http);

/* zpn_http_free free http structure */
void zpn_http_free (p_zpn_http http);

#endif
