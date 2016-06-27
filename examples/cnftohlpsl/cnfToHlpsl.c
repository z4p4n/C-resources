#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>

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

	fprintf (stdout, "\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n"
	                 "| Transform CNF to HLPSL by Zapan |\n"
	                 "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");
}

/* Transform ksat to hlpsl file                *
 * ------------------------------------------- *
 * p_cnf cnf  : struct who get cnf data        *
 * char *path : output file                    *
 * ------------------------------------------- *
 * return ZPN_ERROR if fail, ZPN_OK otherwhise */
int write_hlpsl_from_ksat (p_cnf cnf, const char *path) {

	int i,j,k;
	FILE *fd;

	fprintf (stdout, "[?] Trying to transform sat formula to HLPSL\n");

	fd = fopen (path, "w");
	if (fd == NULL) {
		perror ("fopen");
		return ZPN_ERROR;
	}

	/* Write C0 role */
	fprintf (fd, 
	  "role role_C0(C0:agent,C1:agent,True,False,Key,MEnd:public_key,SND,RCV:channel(dy))\n"
	  "played_by C0\ndef=\n\tlocal\n\t\tState:nat,Secret:text");
	for (i = 1 ; i < cnf->lit + 1 ; i++) fprintf (fd, ",X%d:public_key", i);
	fprintf (fd, "\n\tinit\n\t\tState := 0\n\ttransition\n\t\t1. State=0 /\\ RCV(");
	for (i = 0 ; i < cnf->lit ; i++) {
		if (i == 0) fprintf (fd, "X1'"); else fprintf (fd, ".X%d'", i + 1);
	}
	fprintf (fd, ") =|> State':=1 /\\ SND({");
	for (i = 0 ; i < cnf->cla ; i++) {
		for (j = 0 ; j < cnf->lpc ; j++) {
			if (i != 0 || j != 0) fprintf (fd, ".");
			fprintf (fd, "X%d'", abs (cnf->cnf[i][j])); 
		}
	}
	fprintf (fd, ".MEnd}_Key)\n\t\t2. State=1 /\\ RCV(Secret') =|> State':=2 /\\ "
	  "secret(Secret',sec,{})\nend role\n\n");

	/* Write C1 to C(nb clauses) */
	for (i = 0 ; i < cnf->cla ; i++) {
		fprintf (fd, 
		  "role role_C%d(C%d:agent,C%d:agent,True,False,Key,MEnd:public_key,SND,RCV:"
		  "channel(dy))\nplayed_by C%d\ndef=\n\tlocal\n\t\tState:nat,Secret:text",i+1,i+1,i+2,i+1);
		for (j = (i+1) * cnf->lpc ; j < cnf->cla * cnf->lpc ; j++) 
			fprintf (fd, ",X%d:public_key", j);
		for (j = 0 ; j < cnf->lpc ; j++) fprintf (fd, ",K%d:public_key", j);
		fprintf (fd, "\n\tinit\n\t\tState := 0\n\ttransition\n");
		for (j = 0 ; j < cnf->lpc ; j++) {
			fprintf (fd, "\t\t%d. State=0 /\\ RCV({", j);
			if (j == 0 && cnf->cnf[i][j] > 0) fprintf (fd, "True.K1'.K2'");
			if (j == 0 && cnf->cnf[i][j] < 0) fprintf (fd, "False.K1'.K2'");
			if (j == 1 && cnf->cnf[i][j] > 0) fprintf (fd, "K0'.True.K2'");
			if (j == 1 && cnf->cnf[i][j] < 0) fprintf (fd, "K0'.False.K2'");
			if (j == 2 && cnf->cnf[i][j] > 0) fprintf (fd, "K0'.K1'.True");
			if (j == 2 && cnf->cnf[i][j] < 0) fprintf (fd, "K0'.K2'.False");

			for (k = (i+1) * cnf->lpc ; k < cnf->cla * cnf->lpc ; k++) {
				fprintf (fd, ".X%d'", k);
			}
			fprintf (fd, ".MEnd}_Key) =|> State':=1 /\\ SND({");
			for (k = (i+1) * cnf->lpc ; k < cnf->cla * cnf->lpc ; k++) {
				if (k == (i+1) * cnf->lpc) fprintf (fd, "X%d'", k);
				else fprintf (fd, ".X%d'", k);
			}
			if (i == cnf->cla - 1) fprintf (fd, "MEnd}_Key)\n");
			else                   fprintf (fd, ".MEnd}_Key)\n");
		}
		fprintf (fd, "end role\n\n");
	}

	/* Write End Role */
	fprintf (fd, 
	  "role role_End(C0:agent,End:agent,True,False,Key,MEnd:public_key,SND,RCV:channel(dy))\n"
	  "played_by End\ndef=\n\tlocal\n\t\tState:nat,Secret:text\n\tinit\n\t\tState := 0\n"
	  "\ttransition\n\t\t1. State=0 /\\ RCV({MEnd}_Key) =|> State':=1 /\\ Secret':=new() /\\ "
	  "secret(Secret',sec,{}) /\\ SND(Secret')\nend role\n\n");
	
	/* Write session function role */
	fprintf (fd, "role session(True:public_key,False:public_key,Key:public_key,MEnd:public_key");
	for (i = 0 ; i < cnf->cla + 1 ; i++) fprintf (fd, ",C%d:agent", i);
	fprintf (fd, ",End:agent)\ndef=\n\tlocal\n\t\t");
	for (i = 0 ; i < cnf->cla + 1; i++) {
		if (i == 0) fprintf (fd, "SND0,RCV0");
		else fprintf (fd, ",SND%d,RCV%d", i, i);
	}
	fprintf (fd, ",SNDEnd,RCVEnd:channel(dy)\n\tcomposition\n\t\t");
	for (i = 0 ; i < cnf->cla + 1 ; i++) {
		if (i != 0) fprintf (fd, " /\\ ");
		if (i == cnf->cla) 
			fprintf (fd, "role_C%d(C%d,End,True,False,Key,MEnd,SND%d,RCV%d)", i, i, i, i);
		else fprintf (fd, "role_C%d(C%d,C%d,True,False,Key,MEnd,SND%d,RCV%d)", i, i, i + 1, i, i);
	}
	fprintf (fd, " /\\ role_End(C0,End,True,False,Key,MEnd,SNDEnd,RCVEnd)\nend role\n\n");
	
	/* Write environment function role */
	fprintf (fd, "role environment()\ndef=\n\tconst\n"
		"\t\thash_0:function,pk1:public_key,pk2:public_key,pk3:public_key,pk4:public_key"
	    ",sec:protocol_id");
	for (i = 0 ; i < cnf->cla + 1; i++) fprintf (fd, ",c%d:agent", i);
	fprintf (fd, ",theend:agent");
	fprintf (fd, "\n\tintruder_knowledge = {pk1,pk2}\n\tcomposition\n\t\tsession(pk1,pk2,pk3,pk4");
	for (i = 0 ; i < cnf->cla + 1 ; i++) fprintf (fd, ",c%d", i);
	fprintf (fd, ",theend)\nend role\n\n");

	/* Write goal */
	fprintf (fd, "goal\n\tsecrecy_of sec\nend goal\n\n");
	
	/* Write calling environment instance */
	fprintf (fd, "environment()\n");

	fclose (fd);

	return ZPN_OK;
}

int main (int argc, const char *argv[]) {

	int ret, size; 
	char *data;
	t_cnf cnf;

	if (argc < 3) {

		fprintf (stderr, "usage %s CNF_file output_file\n", argv[0]);
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
	munmap (data, size);
	if (ret == ZPN_ERROR) exit (EXIT_FAILURE);

	/* Write HLPSL From k-Sat */
	ret = write_hlpsl_from_ksat (&cnf, argv[2]);
	free_cnf (&cnf);
	if (ret == ZPN_ERROR) exit (EXIT_FAILURE);

	fprintf (stdout, "[+] Success\n");

	exit (EXIT_SUCCESS);
}

