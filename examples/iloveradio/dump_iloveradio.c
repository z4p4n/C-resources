#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "../../ZpnHTTP.h"

#define HOST_ILRA 		"www.iloveradioadvertising.be"
#define GET_DL_ILRA 	"/download.ashx?id=%d"
#define FILENAME_ILRA 	"advertising/id_%d.mp3"
#define DIRNAME_ILRA 	"advertising"
#define MIN_ID_ILRA 	1
#define MAX_ID_ILRA 	100000

int main (int argc, const char *argv[]) {

	t_zpn_http http;
	int ret, id, min_id, max_id;
	char request[BUFSIZ], filename[BUFSIZ];
	FILE *fd;

	/* Usage */
	if (argc < 3 || strtol (argv[1], NULL, 10) < MIN_ID_ILRA || strtol (argv[2], NULL, 10) > MAX_ID_ILRA) {
		
		fprintf (stderr, "Usage: %s [id start (>= %d)] [id end (<= %d)] \n", argv[0], MIN_ID_ILRA, MAX_ID_ILRA);
		exit (EXIT_FAILURE);
	}

	min_id = strtol(argv[1], NULL, 10);
	max_id = strtol(argv[2], NULL, 10);
	
	/* Create directory */
	ret = mkdir (DIRNAME_ILRA, 0700);
	if (ret != 0 && errno != EEXIST) {

		fprintf (stderr, "Cannot create directory %s\n", DIRNAME_ILRA);
		exit (EXIT_FAILURE);
	}

	/* Download the mp3 files */
	for (id = min_id ; id <= max_id ; id++) {

		fprintf (stdout, "Download radio advertising number %d\n", id);
		snprintf (request, BUFSIZ, GET_DL_ILRA, id);

		/* Send request and receive advert */
		ret =  zpn_http_get (request, HOST_ILRA, NULL, NULL, "keep-alive", &http);

		if (ret == -1) {

			fprintf (stdout, "advert nÂ%d may not exists\n",id);
			continue;
		}

		/* Save advert */
		snprintf (filename, BUFSIZ, FILENAME_ILRA, id);
		fd = fopen (filename, "wb");

		if (fd == NULL) {

			fprintf (stderr, "Cannot create file %s\n", filename);
			zpn_http_free (&http);
			exit (EXIT_FAILURE);
		}
		
		fwrite (http.content, sizeof (char), http.content_length, fd);

		/* Release mp3 data */
		zpn_http_free (&http);
	}
	
	exit (EXIT_SUCCESS);
}

