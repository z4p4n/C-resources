#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>

#include "../../ZpnHTTP.h"

#define HOST_KSAT 		"www.toughsat.appspot.com"
#define POST_DL_KSAT 	"http://toughsat.appspot.com/generate"
#define FILENAME_KSAT 	"gen/%d_lpc%d_lit%d_cla%d.cnf"
#define DIRNAME_KSAT 	"gen"
#define POST_REQUEST    "type=random_ksat&lits_per_clause=%d&numvariables=%d&numclauses=%d&generate=Generate"

int main (int argc, const char *argv[]) {

	t_zpn_http http;
	int ret, i, lit, cla, reco_cla, nb, lpc;
	double c;
	char request[BUFSIZ], filename[BUFSIZ], post[BUFSIZ];
	FILE *fd;


	/* Usage */
	if (argc < 5) {
		
		fprintf (stderr, "Usage: %s [literals per clause] [number of literals] [number of clauses] [Number of random ksat wanted]\n", argv[0]);
		exit (EXIT_FAILURE);
	}

	/* Get values */
	lpc = strtol(argv[1], NULL, 10);
	lit = strtol(argv[2], NULL, 10);
	cla = strtol(argv[3], NULL, 10);
	nb  = strtol(argv[4], NULL, 10);

	fprintf (stdout, "Random KSAT generator with www.toughsat.appspot.com\n\n");
	fprintf (stdout, "[+] Literals per clause : %d\n", lpc);
	fprintf (stdout, "[+] Literals number     : %d\n", lit);
	fprintf (stdout, "[+] Clauses number      : %d\n\n", cla);

	fprintf (stdout, "[?] Formula : c = log(2^k/(2^k - 1))^-1\n");
	c = pow (log2 (pow (2, lpc) / (pow (2, lpc) - 1)), -1);
	reco_cla = (int) (c * (double) lit);
	fprintf (stdout, "[?] Recommended number of clauses : %d\n", reco_cla);
	
	/* Create directory */
	ret = mkdir (DIRNAME_KSAT, 0700);
	if (ret != 0 && errno != EEXIST) {

		fprintf (stderr, "Cannot create directory %s\n", DIRNAME_KSAT);
		exit (EXIT_FAILURE);
	}

	/* Generate K-Sat files */
	for (i = 0; i < nb; i++) {

		fprintf (stdout, "Generate random %d-Sat number %d\n", lpc, i);
		snprintf (request, BUFSIZ, POST_DL_KSAT);

		snprintf (post, BUFSIZ, POST_REQUEST, lpc, lit, cla);

		/* Send request and receive advert */
		ret =  zpn_http_post (request, HOST_KSAT, NULL, NULL, "keep-alive", post, &http);

		if (ret == -1) {

			fprintf (stdout, "Fail with %s\n", HOST_KSAT);
			continue;
		}

		/* Save K-Sat */
		snprintf (filename, BUFSIZ, FILENAME_KSAT, i, lpc, lit, cla);
		fd = fopen (filename, "wb");

		if (fd == NULL) {

			fprintf (stderr, "Cannot create file %s\n", filename);
			zpn_http_free (&http);
			exit (EXIT_FAILURE);
		}
		
		fwrite (http.content, sizeof (char), http.content_length, fd);

		/* Release ksat (dimacs) data */
		zpn_http_free (&http);
	}
	
	exit (EXIT_SUCCESS);
}

