#include <stdio.h>
#include <string.h>

#include "ZpnError.h"
#include "ZpnSocket.h"
#include "ZpnTor.h"

extern int __zpn_error__;
extern int __zpn_error_type__;
extern char __zpn_error_buf__[BUFSIZ];

/* Check answer from SOCKS v4 proxy       
 * ------------------------------------------------ *
 * status : [const uint16_t] status byte            *
 * ------------------------------------------------ *
 * Return : ZPN_OK if success, ZPN_ERROR otherwhise */

int zpn_handshake_SOCKSv4_check (const uint16_t status) {

        switch (status) {

                case ZPN_SOCKSv4_QUERY_OK :
                        return ZPN_OK;
						break;

                case ZPN_SOCKSv4_QUERY_ERROR :
						SET_ERROR_BUF (ZPN_ERROR_TOR, "The proxy SOCKS server return an error.");
						return ZPN_ERROR;

                case ZPN_SOCKSv4_QUERY_ERROR_IDENTD :
                        SET_ERROR_BUF (ZPN_ERROR_TOR, "Request failed : client has not launch"
                                                      " identd daemon (or the proxy SOCKS server cannot"
                                                      " access identd).");
						return ZPN_ERROR;

                case ZPN_SOCKSv4_QUERY_ERROR_AUTH :
                        SET_ERROR_BUF (ZPN_ERROR_TOR, "Request failed : authentification failed.");
						return ZPN_ERROR;
                
                default :
                        SET_ERROR_BUF (ZPN_ERROR_TOR, "The server is probably not a SOCKS proxy.\n");
						return ZPN_ERROR;
        }
}

/* Build SOCKS (v4a) request.                                              *
 * ----------------------------------------------------------------------- *
 * req :  [char **]             request buffer                             *
 * port : [const uint16_t]      distant server port                        *
 * hostname : [const char *]    v4a singularity: remote resolve DNS        *
 * auth : [const char *]        auth string for proxy SOCKS. (Can be NULL) * 
 * ----------------------------------------------------------------------- *
 * Return: len of request if success, ZPN_ERROR otherwhise                 */

int zpn_handshake_SOCKSv4a_build (char **req, const uint16_t port, const char *hostname, const char *auth) {

        int len;
        uint32_t IPv4 = 0x1000000; /* invalid IP, 0.0.0.x, with x != 0. */

        /* Compute request len. */

        len = 1 /* Version */ + 1 /* Option */ + 2 /* Port */ + 4 /* IPv4 */ + 1; /* Null byte (auth) */
        len += strlen (hostname) + 1; /* DSN to contact, will be resolve by SOCKS proxy. And null byte. */
        if (auth != NULL) len += strlen (auth); /* Authentification string */

        *req = (char *) malloc (sizeof (char) * len);

        if (*req == NULL) {
				SET_ERROR (ZPN_ERROR_ERRNO, errno);
				return ZPN_ERROR;
        }

        /* Finaly build request */

        ((char *) *req)[0] = ZPN_SOCKSv4_VERSION;               // Version.
        ((char *) *req)[1] = ZPN_SOCKSv4_PIPE_TCP;              // Option : 1 = Pipe TCP/IP, 2 = port TCP.
        *((uint16_t *) &(((char *) *req))[2]) = port;           // Port.
        *((uint32_t *) &(((char *) *req))[4]) = IPv4;           // IPv4.
        if (auth != NULL) {
                strcpy (&(((char *) *req)[8]), auth);           // auth string and null byte. 
                strcpy (&(((char *) *req)[9 + strlen (auth)]), 
                        hostname);                              // host name and null byte.
        } else {
                ((char *) *req)[8] = '\0';                      // Null byte only.
                strcpy (&(((char *) *req)[9]), hostname);       // host name  et null byte.
        }

        return len;
}
	
/* connect to a SOCKS (v4a) proxy with handshake checker                                   *
 * --------------------------------------------------------------------------------------- *
 * info     : [p_zpn_socket]    yet configured socket and yet connected to the proxy       *
 * port     : [const uint16_t]  remote server port                                         *
 * hostname : [const char *]    v4a singularity: remote resolve DNS                        *
 * auth : [const char *]        auth string for proxy SOCKS. (Can be NULL)                 * 
 * --------------------------------------------------------------------------------------- *
 * Return : ZPN_OK if success, ZPN_ERROR otherwhise                                        */

int zpn_handshake_SOCKS_v4a_TCP (p_zpn_socket info, const uint16_t port, 
                                 const char *hostname, const char *auth) {

        char *req = NULL, reply[ZPN_SOCKSv4_LEN_REPLY];
        int ret, len;

        /* Build SOCKS v4 request */
        len = zpn_handshake_SOCKSv4a_build (&req, port, hostname, auth);
        if (len == ZPN_ERROR) return ZPN_ERROR;

        /* Start handshake. */
		ret = zpn_socket_raw_send (info, req, len);
        free (req);
        if (ret == ZPN_ERROR) return ZPN_ERROR;

		if ((ret = recv (info->sock, reply, ZPN_SOCKSv4_LEN_REPLY, 0)) == -1) {

			SET_ERROR (ZPN_ERROR_ERRNO, errno);
			close (info->sock);
			return ZPN_ERROR;
		}

        /* Vérification du handshake. */
        if (zpn_handshake_SOCKSv4_check (reply[1]) == ZPN_ERROR) 
			return ZPN_ERROR;

        return ZPN_OK;
}

/* connect to a SOCKS (v4a) proxy with handshake checker                                   *
 * --------------------------------------------------------------------------------------- *
 * info     : [p_zpn_socket]    empty socket                                               *
 * port     : [const uint16_t]  remote server port                                         *
 * hostname : [const char *]    v4a singularity: remote resolve DNS                        *
 * auth     : [const char *]        auth string for proxy SOCKS. (Can be NULL)             * 
 * --------------------------------------------------------------------------------------- *
 * Return : ZPN_OK if success, ZPN_ERROR otherwhise                                        */
int zpn_torrify (p_zpn_socket info, const uint16_t port, 
                 const char *hostname, const char *auth) {

	int ret;

	ret = zpn_connect_by_hostname (ZPN_LOCALHOST, ZPN_PORT_TOR, info);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	ret = zpn_handshake_SOCKS_v4a_TCP (info, port, hostname, auth);
	if (ret == ZPN_ERROR) {
		close (info->sock);
		return ZPN_ERROR;
	}

	return ZPN_OK;
}

