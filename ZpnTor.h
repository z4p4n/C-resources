#ifndef ZPN_TOR_H
#define ZPN_TOR_H

/* Const for SOCKS v4 protocol */

#define ZPN_SOCKSv4_LEN_REPLY          8

#define ZPN_SOCKSv4_VERSION            0x04
#define ZPN_SOCKSv4_PIPE_TCP           0x01
#define ZPN_SOCKSv4_CORRES_TCP         0x02
#define ZPN_SOCKSv4_QUERY_OK           0x5a
#define ZPN_SOCKSv4_QUERY_ERROR        0x5b
#define ZPN_SOCKSv4_QUERY_ERROR_IDENTD 0x5c
#define ZPN_SOCKSv4_QUERY_ERROR_AUTH   0x5d

/* Tor informations */

#define ZPN_LOCALHOST 	"localhost"
#define ZPN_PORT_TOR 	9050

/* connect to a SOCKS (v4a) proxy with handshake checker                                   *
 * --------------------------------------------------------------------------------------- *
 * info     : [p_zpn_socket]    empty socket                                               *
 * port     : [const uint16_t]  remote server port                                         *
 * hostname : [const char *]    v4a singularity: remote resolve DNS                        *
 * auth : [const char *]        auth string for proxy SOCKS. (Can be NULL)                 * 
 * --------------------------------------------------------------------------------------- *
 * Return : ZPN_OK if success, ZPN_ERROR otherwhise                                        */
int zpn_torrify (p_zpn_socket info, const uint16_t port, 
                 const char *hostname, const char *auth);

#endif
