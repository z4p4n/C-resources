#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

#include "ZpnError.h"

int __zpn_error__ = 0;
int __zpn_error_type__ = 0;
regex_t __zpn_preg__;
char __zpn_error_buf__[BUFSIZ];

/* This function display last error */
void zpn_print_error () {

	char *msg = NULL;
	size_t size = 0;

	if (__zpn_error__ == ZPN_ERROR_SMTP) {

		fprintf (stderr, "[SMTP] %s\n", __zpn_error_buf__);

	} else if (__zpn_error__ == ZPN_ERROR_HTTP) {

		fprintf (stderr, "[HTTP] %s\n", __zpn_error_buf__);

	} else if (__zpn_error__ == ZPN_ERROR_ERRNO) {

		fprintf (stderr, "[errno] %s\n", strerror (__zpn_error_type__));

	} else if (__zpn_error__ == ZPN_ERROR_SOCKET) {
	
		fprintf (stderr, "[h_errno] %s\n", hstrerror (__zpn_error_type__));
	
	} else if (__zpn_error__ == ZPN_ERROR_MALLOC) 

		fprintf (stderr, "Error with malloc function\n");

	else if (__zpn_error__ == ZPN_ERROR_REGEX) {

		size = regerror (__zpn_error_type__, &__zpn_preg__, NULL, 0);
		msg = malloc (sizeof (*msg) * size);

		if (msg != NULL) {

			regerror (__zpn_error_type__, &__zpn_preg__, msg, size);
			fprintf (stderr, "[regex] %s\n", msg);
			free (msg);
		}

	} else 
		fprintf (stderr, "Undefined error\n");

	return;
}


