#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "../../ZpnError.h"
#include "../../ZpnSocket.h"
#include "../../ZpnHTTP.h"
#include "../../ZpnTor.h"

int main (int argc, const char *argv[]) {

	t_zpn_socket info;
	int ret, i;
	t_zpn_http http;
	char request[] = "";

	ret = zpn_torrify (&info, htons (80), /* IP TODO */, NULL);
	if (ret == ZPN_ERROR) {
		zpn_print_error ();
	}

	ret =  zpn_http_post_proxy ("/REQUEST", "HOST NEEDED PLEASE", 
								NULL, NULL, "Keep-Alive", 
								"POST DATA LOLOL", &http, &info);
	fprintf (stdout, "%s\n", http.content);
	zpn_http_free (&http);
	close (info.sock);
	if (ret == ZPN_ERROR) {
		zpn_print_error ();
	}

	exit (EXIT_SUCCESS);
}

