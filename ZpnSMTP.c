#include <stdio.h>
#include <string.h>

#include "ZpnError.h"
#include "ZpnSocket.h"
#include "ZpnSMTP.h"

extern int __zpn_error__;
extern int __zpn_error_type__;
extern char __zpn_error_buf__[BUFSIZ];

int zpn_smtp_check_answer (p_zpn_socket info, char *answer) {

	int ret;
	char buf[BUFSIZ];

	memset (buf, 0, BUFSIZ);

	if ((ret = recv (info->sock, buf, sizeof (buf) - 1, 0)) == -1) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		close (info->sock);
		return ZPN_ERROR;
	}

	if (strstr (buf, answer) == NULL) {

		SET_ERROR_BUF (ZPN_ERROR_SMTP, buf);
		close (info->sock);
		return ZPN_ERROR;
	}

	return ZPN_OK;
}

int zpn_send_mail (const char *host, uint16_t port, 
                   const char *receptor, const char *sender, const char *name,
		   const char *subject, const char *data, 
		   const char *login, const char *password) {

	int ret;
	t_zpn_socket info;
	char buf[BUFSIZ];

	ret = zpn_connect_by_hostname (host, port, &info);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* 220 */
	ret = zpn_smtp_check_answer (&info, "220");
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* HELO */
	strncpy (buf, "HELO boy\r\n", sizeof (buf) - 1);
	ret = zpn_socket_send (&info, buf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* 250 */
	ret = zpn_smtp_check_answer (&info, "250");
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* If AUTH LOGIN */
	if (login != NULL && password != NULL) {

		/* AUTH LOGIN */
		strncpy (buf, "AUTH LOGIN\r\n", sizeof (buf) - 1);
		ret = zpn_socket_send (&info, buf);
		if (ret == ZPN_ERROR) return ZPN_ERROR;

		/* 334 */
		ret = zpn_smtp_check_answer (&info, "334");
		if (ret == ZPN_ERROR) return ZPN_ERROR;

		/* login mail in base64 */
		snprintf (buf, sizeof (buf) - 1, "%s\r\n", login);
		ret = zpn_socket_send (&info, buf);
		if (ret == ZPN_ERROR) return ZPN_ERROR;

		/* 334 */
		ret = zpn_smtp_check_answer (&info, "334");
		if (ret == ZPN_ERROR) return ZPN_ERROR;

		/* password */
		snprintf (buf, sizeof (buf) - 1, "%s\r\n", password);
		ret = zpn_socket_send (&info, buf);
		if (ret == ZPN_ERROR) return ZPN_ERROR;

		/* 235 */
		ret = zpn_smtp_check_answer (&info, "235");
		if (ret == ZPN_ERROR) return ZPN_ERROR;
	}

	/* MAIL FROM */
	snprintf (buf, sizeof (buf) - 1, "MAIL FROM: <%s>\r\n", sender);
	ret = zpn_socket_send (&info, buf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* 250 MAIL FROM:<...@...> OK */
	ret = zpn_smtp_check_answer (&info, "250");
	if (ret == ZPN_ERROR) return ZPN_ERROR;
	
	/* RCPT */
	snprintf (buf, sizeof (buf) - 1, "RCPT TO: <%s>\r\n", receptor);
	ret = zpn_socket_send (&info, buf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* 250 RCPT <...@...> OK */
	ret = zpn_smtp_check_answer (&info, "250");
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* DATA */
	snprintf (buf, sizeof (buf) - 1, "DATA\r\n");
	ret = zpn_socket_send (&info, buf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* 354 start mail */
	ret = zpn_smtp_check_answer (&info, "354");
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* subject and data */
	snprintf (buf, sizeof (buf) - 1, "from: %s <%s>\r\nsubject: %s\r\n%s\r\n.\r\n", 
		  name, sender, subject, data);
	ret = zpn_socket_send (&info, buf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;
	
	/* 250 Mail accepted */
	ret = zpn_smtp_check_answer (&info, "250");
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* QUIT */
	snprintf (buf, sizeof (buf) - 1, "QUIT\r\n");
	ret = zpn_socket_send (&info, buf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	close (info.sock);

	return ZPN_OK;
}

