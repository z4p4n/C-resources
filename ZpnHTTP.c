#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ZpnError.h"
#include "ZpnString.h"
#include "ZpnRegex.h"
#include "ZpnSocket.h"
#include "ZpnHTTP.h"

extern int __zpn_error__;
extern int __zpn_error_type__;
extern char __zpn_error_buf__[BUFSIZ];

/* zpn_clean_http clean header */
void zpn_clean_http (p_zpn_http http) {

	memset (http->header, 0, ZPN_GET_HTTP_HEADER);
	memset (http->cookies, 0, ZPN_GET_HTTP_COOKIE);

	return;
}

/* zpn_http_free free http structure */
void zpn_http_free (p_zpn_http http) {

	free (http->content);

	return;
}

/* zpn_http_recv_content store content from the website	*
 * ---------------------------------------------------- *
 * p_zpn_socket info : socket connected to the website	*
 * p_zpn_http http : structure of the http header	*
 * ---------------------------------------------------- *
 * return : ZPN_OK if content is stored, else ZPN_ERROR	*/
int zpn_http_recv_content (p_zpn_socket info, p_zpn_http http) {

	int stored = 0, ret = 1;

	http->content = calloc (http->content_length, sizeof (char));

	if (http->content == NULL) {
		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		close (info->sock);
		return ZPN_ERROR;
	}

	while (stored < http->content_length && ret != 0) {
		
		if ((ret = recv (info->sock, &(http->content[stored]), http->content_length - stored, 0)) == -1) {

			SET_ERROR (ZPN_ERROR_ERRNO, errno);
			free (http->content);
			close (info->sock);
			return ZPN_ERROR;
		}

		stored += ret;
	}
	
	return ZPN_OK;
}

/* zpn_http_recv_header store informations about http header 		*
 * -------------------------------------------------------------------- *
 * p_zpn_socket info : socket connected to the website			*
 * p_zpn_http http : structure of the http header			*
 * -------------------------------------------------------------------- *
 * return : ZPN_OK if header was detected and stored, else ZPN_ERROR	*/
int zpn_http_recv_header (p_zpn_socket info, p_zpn_http http) {

	p_zpn_str str_struct;
	int ret, end = 0, cursor = 0;
	char c, prev;

	zpn_clean_http (http);

	do {

		if ((ret = recv (info->sock, &c, sizeof (char), 0)) == -1) {

			SET_ERROR (ZPN_ERROR_ERRNO, errno);
			close (info->sock);
			return ZPN_ERROR;
		}
		
		if (prev == '\r' && c == '\n')
			end++;
		else if (prev == '\n' && c != '\r')
			end = 0;
		prev = c;

		http->header[cursor] = c;
		cursor++;
		
	} while (end != 2 && ret != 0 && cursor < ZPN_GET_HTTP_HEADER - 1);

	http->header[cursor] = '\0';

	/* Get HTTP code */
	ret = zpn_regex_get (http->header, "HTTP/[0-9.]* ([0-9]+) ", &str_struct);
	if (ret >= ZPN_REGEX_MATCH) {
	
		/* what to do if matched */
		http->code = atoi (str_struct->str[1]);
		zpn_free_str (str_struct);
	
	} else if (ret == ZPN_ERROR) {
	
		/* what to do if failed */
		close (info->sock);
		return ZPN_ERROR;
	
	} else {
	
		/* what to do if it did not match */
		SET_ERROR_BUF (ZPN_ERROR_HTTP, "Code http not found");
		close (info->sock);
		return ZPN_ERROR;
	}
	
	/* Get content-length */
	ret = zpn_regex_get (http->header, "Content-Length: ([0-9]+)", &str_struct);
	if (ret >= ZPN_REGEX_MATCH) {
	
		/* what to do if matched */
		http->content_length = atoi (str_struct->str[1]);
		zpn_free_str (str_struct);
	
	} else if (ret == ZPN_ERROR) {
	
		/* what to do if failed */
		close (info->sock);
		return ZPN_ERROR;
	
	} else {
	
		/* what to do if it did not match */
		http->content_length = -1;
	}
	
	/* Get Set-Cookie */
	ret = zpn_regex_get (http->header, "Set-Cookie: ([[:print:]]+)", &str_struct);
	if (ret >= ZPN_REGEX_MATCH) {
	
		/* what to do if matched */
		strncpy (http->cookies, str_struct->str[1], ZPN_GET_HTTP_COOKIE);
		zpn_free_str (str_struct);
	
	} else if (ret == ZPN_ERROR) {
	
		/* what to do if failed */
		close (info->sock);
		return ZPN_ERROR;
	
	} 
	
	return ZPN_OK;
}

/* zpn_http_send send data to the website				*
 * -------------------------------------------------------------------- *
 * p_zpn_socket info : socket connected to the website			*
 * const char *buf : data to send					*
 * -------------------------------------------------------------------- *
 * return : ZPN_OK if can send without any error, else ZPN_ERROR	*/
int zpn_http_send (p_zpn_socket info, const char *buf) {

	if (send (info->sock, buf, strlen (buf), 0) == -1) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		close (info->sock);
		return ZPN_ERROR;
	}

	return ZPN_OK;
}

/* zpn_http_get send a request to a website						*
 * ------------------------------------------------------------ *
 * const char *req : request GET or POST						*
 * const char *host : adress of the host						*
 * const char *user_agent : [CAN BE NULL] user_agent			*
 * const char *cookie : [CAN BE NULL] cookies					*
 * const char *connection : connection type						*
 * const char *body : [CAN BE NULL] body of POST request		*
 * p_zpn_http http : [OUTPUT] structure of the http header		*
 * ------------------------------------------------------------ *
 * return : ZPN_OK if ok, else ZPN_ERROR						*/
int zpn_http_request (const char *req, const char *host, 
		  const char *user_agent, const char *cookie,
		  const char *connection, const char *post,
		  p_zpn_http http) {

	int ret;
	char *request;
	t_zpn_socket info;
	int size_request = ZPN_GET_HTTP_REQUEST;

	if (post != NULL)
		size_request += strlen (post) + 256;

	request = malloc (sizeof (char) * size_request);
	
	ret = zpn_connect_by_hostname (host, ZPN_PORT_HTTP, &info);
	if (ret == ZPN_ERROR) { free (request); return ZPN_ERROR; }

	if (post == NULL)
		snprintf (request, size_request, 
			  "GET %s HTTP/1.1\r\n"
			  "Host: %s\r\n",
			  req, host);
	else
		snprintf (request, size_request, 
			  "POST %s HTTP/1.1\r\n"
			  "Host: %s\r\n",
			  req, host);

	if (user_agent != NULL) {

		snprintf (&(request[strlen(request)]), 
			  size_request - strlen (request), 
			  "User-Agent: %s\r\n", user_agent);
	}

	if (cookie != NULL) {

		snprintf (&(request[strlen(request)]), 
			  size_request - strlen (request), 
			  "Cookie: %s\r\n", cookie);
	}

	if (post != NULL)
		snprintf (&(request[strlen(request)]), 
			  size_request - strlen (request), 
			  "Connection: %s\r\n"
			  "Content-type: application/x-www-form-urlencoded\r\n"
			  "Content-length: %d\r\n\r\n"
			  "%s\r\n", connection, (int) strlen (post), post);
	else 
		snprintf (&(request[strlen(request)]), 
			  size_request - strlen (request), 
			  "Connection: %s\r\n\r\n", connection);

	ret = zpn_http_send (&info, request);
	if (ret == ZPN_ERROR) { free (request); return ZPN_ERROR; }

	ret = zpn_http_recv_header (&info, http);
	if (ret == ZPN_ERROR) { free (request); return ZPN_ERROR; }

	if (http->content_length != -1) {
		ret = zpn_http_recv_content (&info, http);
		if (ret == ZPN_ERROR) { free (request); return ZPN_ERROR; }
		
	/* If content_length is unknown */
	} else {
		ret = zpn_socket_recv_nonblock (&info, &(http->content), 1);
		if (ret == ZPN_ERROR) { free (request); return ZPN_ERROR; }

		http->content_length = strlen (http->content);
	}

	close (info.sock);
	free (request);

	return ZPN_OK;
}

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
		  const char *connection, p_zpn_http http) {

	return zpn_http_request (get, host, user_agent, cookie,
		  					connection, NULL, http);
}

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
		  p_zpn_http http) {

	return zpn_http_request (request, host, user_agent, cookie,
		  					connection, post, http);
}

