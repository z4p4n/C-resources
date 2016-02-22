#ifndef ZPN_SOCKET_H
#define ZPN_SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

/* _zpn_socket is a about socket informations */
typedef struct _zpn_socket {

	int sock;
	struct hostent *hostinfo;
	struct sockaddr_in sin;

} t_zpn_socket, *p_zpn_socket;

/* zpn_socket_send send a packet 										*
 * -------------------------------------------------------------------- *
 * p_zpn_socket info   : informations about the current socket			*
 * const char *buf	   : packet to send									*
 * -------------------------------------------------------------------- *
 * return : ZPN_OK if send work ZPN_ERROR else							*/
int zpn_socket_send (p_zpn_socket info, const char *buf);

/* zpn_connect_by_hostname create a tcp connection to an hostname 	*
 * -------------------------------------------------------------------- *
 * const char hostname : name of the host				*
 * uint16_t port       : port of the host				*
 * p_zpn_socket info   : informations about the current socket		*
 * -------------------------------------------------------------------- *
 * return : ZPN_OK if connection is establish, if fail : ZPN_ERROR	*/
int zpn_connect_by_hostname (const char *hostname, uint16_t port, p_zpn_socket info);

/* zpn_socket_nonblock Put the socket in non-blocking mode 				*
 * -------------------------------------------------------------------- *
 * p_zpn_socket info   : informations about the current socket			*
 * -------------------------------------------------------------------- *
 * return : ZPN_OK if success, ZPN_ERROR else							*/
int zpn_socket_nonblock (p_zpn_socket info);

/* zpn_socket_recv_nonblock recv (non block)					*
 * ------------------------------------------------------------ *
 * p_zpn_socket info : informations about the connection		*
 * char **buf : Store answer of the server 						*
 * int timeout : timeout of non blocking recv					*
 * ------------------------------------------------------------ *
 * return : ZPN_OK if ok, else ZPN_ERROR						*/
int zpn_socket_recv_nonblock (p_zpn_socket info, char **buf, int timeout);

#endif
