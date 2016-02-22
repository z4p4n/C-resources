#ifndef ZPN_IRC_H
#define ZPN_IRC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ZpnError.h"
#include "ZpnSocket.h"

#define ZPN_IRC_BUFSIZ 512
#define ZPN_IRC_TIMEOUT 1

/* structure _zpn_irc can store informations about IRC information */ 
typedef struct _zpn_irc {

	t_zpn_socket info;
	
} t_zpn_irc, *p_zpn_irc;

/* zpn_irc_connect initialize a connection on a IRC server		*
 * ------------------------------------------------------------ *
 * p_zpn_irc irc	: [OUTPUT] informations on irc connections	*
 * const char *host : adress of the host						*
 * const int  port : port to plug								*	
 * const char *user_name : name of user (display on channel)	*
 * const char *real_name : real name of user					*
 * const char *nick_name : nick name of user (for connection)	*
 * const char *pass : [CAN BE NULL] password of user			*
 * ------------------------------------------------------------ *
 * return : ZPN_OK if ok, else ZPN_ERROR						*/
int zpn_irc_connect (p_zpn_irc irc, const char *host, const int port,
					const char *user_name, const char* real_name,
					const char *nick_name, const char *password);

/* zpn_irc_send_check send a packet and check response			*
 * ------------------------------------------------------------ *
 * p_zpn_irc info : informations about the connection			*
 * char *buf : Store answer of the server [ZPN_IRC_BUFSIZ]		*
 * ------------------------------------------------------------ *
 * return : ZPN_OK if ok, else ZPN_ERROR						*/
int zpn_irc_send_check (p_zpn_irc irc, char *buf);

#endif
