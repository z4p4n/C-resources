#include "ZpnIRC.h"

extern int __zpn_error__;
extern int __zpn_error_type__;
extern char __zpn_error_buf__[BUFSIZ];

/* zpn_irc_send_check send a packet and check response			*
 * ------------------------------------------------------------ *
 * p_zpn_irc info : informations about the connection			*
 * char *buf : Store answer of the server [ZPN_IRC_BUFSIZ]		*
 * ------------------------------------------------------------ *
 * return : ZPN_OK if ok, else ZPN_ERROR						*/
int zpn_irc_send_check (p_zpn_irc irc, char *buf) {
	
	int ret;
	char answer[ZPN_IRC_BUFSIZ];

	fprintf (stderr, "SEND:%s", buf);
	ret = zpn_socket_send (&(irc->info), buf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/*
	ret = zpn_irc_recv (info, answer);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	fprintf (stdout, "%sn", answer);
	*/
	/*zpn_irc_check_error (answer);*/

	return ZPN_OK;
}

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
					const char *nick_name, const char *password) {

	int ret;
	char buf[ZPN_IRC_BUFSIZ];
	char *answer;
	t_zpn_socket info;
	
	/* Connection on the server */
	ret = zpn_connect_by_hostname (host, port, &info);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* Wait server response */
	ret = zpn_socket_recv_nonblock (&info, &answer, ZPN_IRC_TIMEOUT);
	if (ret == ZPN_ERROR) return ZPN_ERROR;
	fprintf (stdout, "%s\n", answer);
	free (answer);

	/* Save datas about socket for reusing */
	irc->info = info;

	/* Try to handle the server */
	if (NULL == password) {
		snprintf (buf, ZPN_IRC_BUFSIZ, "PASS * %c%c", 0x0D, 0x0A);
		ret = zpn_irc_send_check (irc, buf);
	} else {
		snprintf (buf, ZPN_IRC_BUFSIZ, "PASS %s%c%c", password, 0x0D, 0x0A);
		ret = zpn_irc_send_check (irc, buf);
	}
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	snprintf (buf, ZPN_IRC_BUFSIZ, "NICK %s%c%c", nick_name, 0x0D, 0x0A);
	ret = zpn_irc_send_check (irc, buf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	snprintf (buf, ZPN_IRC_BUFSIZ, "USER %s 8 * :%s%c%c", 
			user_name, real_name, 0x0D, 0x0A);
	ret = zpn_irc_send_check (irc, buf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	ret = zpn_socket_recv_nonblock (&info, &answer, ZPN_IRC_TIMEOUT);
	if (ret == ZPN_ERROR) return ZPN_ERROR;
	fprintf (stdout, "%s\n", answer);
	free (answer);

	return ZPN_OK;
}

