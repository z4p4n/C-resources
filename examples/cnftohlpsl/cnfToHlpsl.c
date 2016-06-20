#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

#include "../../ZpnError.h"
#include "../../ZpnString.h"
#include "../../ZpnFiles.h"
#include "../../ZpnRegex.h"

#define PATTERN_CNF "([0-9\\-]+) "

/* CNF description */
typedef struct {

	int cla; /* Clause number */
	int lit; /* Literals number */
	int lpc; /* Literals per clauses */
	int **cnf; /* Content of cnf */

} *p_cnf, t_cnf;

/* Free CNF struct */
void free_cnf (p_cnf cnf) {

	int i;

	for (i = 0 ; i < cnf->cla ; i++)
		free (cnf->cnf[i]);

	free (cnf->cnf);
}

/* Allocate Conjonctive Normal Form Structure *
 * ------------------------------------------ *
 * p_cnf cnf  : cnf struct                    *
 * ------------------------------------------ *
 * return ZPN_ERROR if fail, ZPN_OK otherhise */
int malloc_cnf (p_cnf cnf) {
	
	int i, j;

	cnf->cnf = malloc (sizeof (int *) * cnf->cla);
	if (cnf->cnf == NULL) {
		perror ("malloc");
		return ZPN_ERROR;
	}

	for (i = 0 ; i < cnf->cla ; i++) {
		cnf->cnf[i] = malloc (sizeof (int) * cnf->lpc);
		/* Little trick to free memory */
		if (cnf->cnf[i] == NULL) {
			perror ("malloc");
			j = cnf->cla;
			cnf->cla = i;
			free_cnf (cnf);
			cnf->cla = j;
			return ZPN_ERROR;
		}
	}

	return ZPN_OK;
}

/* Create regex pattern for cnf             *
 * ---------------------------------------- *
 * p_cnf cnf  : struct who conains cnf data *
 * ---------------------------------------- *
 * return NULL if fail, pattern otherwhise  */
char *create_regex_pattern_for_cnf (p_cnf cnf) {

	int pat_size, i;
	char *pattern;

	pat_size = cnf->lpc * strlen (PATTERN_CNF) + 10;
	pattern = calloc (pat_size, sizeof (char));
	if (pattern == NULL) {
		perror ("calloc");
		return NULL;
	}
	pattern[0] = '\0';
	for (i = 0 ; i < cnf->lpc ; i++)
		strcat (pattern, PATTERN_CNF);
	strcat (pattern, "0");

	return pattern;
}

/* Parse cnf data                              *
 * ------------------------------------------- *
 * p_cnf cnf  : struct who get cnf data        *
 * char *data : dimacs file                    *
 * ------------------------------------------- *
 * return ZPN_ERROR if fail, ZPN_OK otherwhise */
int parse_cnf_data (p_cnf cnf, char *data) {

	int ret = 0, lit, cla;
	char *pattern = NULL;
	p_zpn_str str_struct, str_cursor;

	/* Get cnf informations in order to parse it */
	ret = zpn_regex_get (data, 
	  ".*a ([0-9]+)-C.*ad ([0-9]+) cl.*nd ([0-9]+) va", 
      &str_struct);
	if (ret >= ZPN_REGEX_MATCH) {
	
		/* what to do if matched */
		cnf->lpc = atoi (str_struct->str[1]);
		cnf->cla = atoi (str_struct->str[2]);
		cnf->lit = atoi (str_struct->str[3]);
		
		zpn_free_str (str_struct);
	
	} else if (ret == ZPN_ERROR) {
	
		zpn_print_error ();
		/* what to do if failed */
		return ZPN_ERROR;
	
	} else {
	
		/* what to do if it did not match */
		fprintf (stderr, "Error with file format [%d]\n", __LINE__);
		return ZPN_ERROR;
	}

	fprintf (stdout, 
	         "[?] Trying to parse %d-CNF with %d clauses and %d literals\n",
	         cnf->lpc, cnf->cla, cnf->lit);

	/* Allocation CNF memory */
	ret = malloc_cnf (cnf);
	if (ret == ZPN_ERROR) return ZPN_ERROR;

	/* Create pattern for regex analysis */
	pattern = create_regex_pattern_for_cnf (cnf);
	if (pattern == NULL) goto fail_create_pattern;

	ret = zpn_regex_get (data, pattern, &str_struct);
	if (ret >= ZPN_REGEX_MATCH) {
	
		/* what to do if matched */

		/* Malformed cnf detected */
		if (ret != cnf->cla) {
			fprintf (stderr, "[!] %d clause(s) detected, but %d was expected\n", ret, cnf->cla);
			goto fail_malformed_cnf;
		}

		/* Finally store CNF */
		str_cursor = str_struct;
		for (cla = 0 ; cla < cnf->cla ; cla++) {
			
			for (lit = 0 ; lit < cnf->lpc ; lit++) 
				cnf->cnf[cla][lit] = atoi (str_cursor->str[lit + 1]);
			str_cursor = str_cursor->next;
		}
		
		zpn_free_str (str_struct);
	
	} else if (ret == ZPN_ERROR) {
	
		/* what to do if failed */
		zpn_print_error ();
		goto fail_parse_cnf;

	} else {
	
		/* what to do if it did not match */
		fprintf (stderr, "Error with file format [%d]\n", __LINE__);
		goto fail_parse_cnf;
	}

	/* Do not forget to free pattern here */
	free (pattern);

	return ZPN_OK;

	fail_malformed_cnf:
	zpn_free_str (str_struct);

	fail_parse_cnf:
	free (pattern);

	fail_create_pattern: 

	free_cnf (cnf);
	return ZPN_ERROR;
}

void disp_nice_message () {

	fprintf (stdout, "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n"
	                 "| Transform CNF to HLPSL by Zapan |\n"
	                 "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");
}

int main (int argc, const char *argv[]) {

	int ret, size; 
	char *data;
	t_cnf cnf;

	if (argc < 2) {

		fprintf (stderr, "usage %s [CNF file]\n", argv[0]);
		exit (EXIT_FAILURE);
	}

	disp_nice_message ();
	
	/* Open CNF Files */
	size = zpn_map_file (argv[1], &data);
	if (size == ZPN_ERROR) {
		zpn_print_error ();
		exit (EXIT_FAILURE);
	}

	/* Parse CNF data */
	ret = parse_cnf_data (&cnf, data);
	if (ret == ZPN_ERROR) goto fail_parsing;

	free_cnf (&cnf);
	munmap (data, size);
	exit (EXIT_SUCCESS);

	fail_parsing :

	munmap (data, size);
	exit (EXIT_FAILURE);
}

