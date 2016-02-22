#include "ZpnSocket.h"
#include "ZpnError.h"

extern int __zpn_error__;
extern int __zpn_error_type__;

extern int h_errno;
extern int errno;

/* zpn_socket_nonblock Put the socket in non-blocking mode 				*
 * -------------------------------------------------------------------- *
 * p_zpn_socket info   : informations about the current socket			*
 * -------------------------------------------------------------------- *
 * return : ZPN_OK if success, ZPN_ERROR else							*/
int zpn_socket_nonblock (p_zpn_socket info) {

	if(fcntl(info->sock, F_SETFL, fcntl(info->sock, F_GETFL) | 
	   O_NONBLOCK) < 0) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		close (info->sock);
		return ZPN_ERROR;
	}

	return ZPN_OK;
}

/* zpn_socket_recv_nonblock recv (non block)					*
 * ------------------------------------------------------------ *
 * p_zpn_socket info : informations about the connection		*
 * char **buf : Store answer of the server 						*
 * int timeout : timeout of non blocking recv					*
 * ------------------------------------------------------------ *
 * return : ZPN_OK if ok, else ZPN_ERROR						*/
int zpn_socket_recv_nonblock (p_zpn_socket info, char **buf, int timeout) {

	int ret = 0;
	int size = 0;
	struct timeval begin, now;
	double timediff;

	*buf = (char *) calloc (BUFSIZ, sizeof (char));
	if (*buf == NULL) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		close (info->sock);
		return ZPN_ERROR;
	}

	/* Beginning time */
	gettimeofday (&begin, NULL);

	do {
		gettimeofday (&now, NULL);

		/* Time elapsed in seconds */
		timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);

		/* Break if timeout (in seconds) is reached */
        if(timediff > timeout)
        	break;

		if (ret < 0) {
			usleep(100);
			ret = 0;
		}
		ret = recv (info->sock, *buf + strlen (*buf), 
					BUFSIZ - (size % BUFSIZ), MSG_DONTWAIT);
	
		if (ret > 0) {
			gettimeofday (&begin, NULL);
			size += ret;
			if (size % BUFSIZ == 0) {

				*buf = realloc (*buf, size + BUFSIZ);

				if (NULL == *buf) {
					SET_ERROR (ZPN_ERROR_ERRNO, errno);
					close (info->sock);
					return ZPN_ERROR;
				}
			}
		}

	} while (errno == EAGAIN);

	if (errno != EAGAIN && errno != 0) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		close (info->sock);
		return ZPN_ERROR;
	}

	return ZPN_OK;
}

/* zpn_socket_send send a packet 										*
 * -------------------------------------------------------------------- *
 * p_zpn_socket info   : informations about the current socket			*
 * const char *buf	   : packet to send									*
 * -------------------------------------------------------------------- *
 * return : ZPN_OK if send work ZPN_ERROR else							*/
int zpn_socket_send (p_zpn_socket info, const char *buf) {

	if (send (info->sock, buf, strlen (buf), 0) == -1) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		close (info->sock);
		return ZPN_ERROR;
	}

	return ZPN_OK;
}

/* zpn_connect_by_hostname create a tcp connection to an hostname 	*
 * -------------------------------------------------------------------- *
 * const char hostname : name of the host				*
 * uint16_t port       : port of the host				*
 * p_zpn_socket info   : informations about the current socket		*
 * -------------------------------------------------------------------- *
 * return : ZPN_OK if connection is establish, if fail : ZPN_ERROR	*/
int zpn_connect_by_hostname (const char *hostname, uint16_t port, p_zpn_socket info) {

	struct sockaddr_in sin = { 0 };
	info->sin = sin;

	info->sock = socket (AF_INET, SOCK_STREAM, 0);
	if (info->sock == -1) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		return ZPN_ERROR;
	}

	info->hostinfo = gethostbyname (hostname);
	if (info->hostinfo == NULL) {

		SET_ERROR (ZPN_ERROR_SOCKET, h_errno);
		return ZPN_ERROR;
	}

	info->sin.sin_addr = *(struct in_addr *) info->hostinfo->h_addr;
	info->sin.sin_port = htons (port);
	info->sin.sin_family = AF_INET;
	
	if (connect (info->sock, (struct sockaddr *) &(info->sin), 
		     sizeof (struct sockaddr)) == -1) {
		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		return ZPN_ERROR;
	}

	return ZPN_OK;
}

