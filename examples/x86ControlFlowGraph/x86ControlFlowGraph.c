#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

#include "../../ZpnError.h"
#include "../../ZpnRegex.h"
#include "../../ZpnFiles.h"

#include "graph.h"

#define NASM_MAX_LEN_IDENTIFIERS 4096

/* Jump to line number */
char *jump_to_line (char *data, const int nb) {

	int line_nb;
	char *line = data;

	for (line_nb = 0 ; line[line_nb] && line_nb < nb; line[line_nb] == '\n' ? line_nb++ : *line++);

	if (line_nb != nb) return NULL;
	else return &(line[line_nb]);
}

/* Search code section */
int search_code_section (char *data) {

	char *tofree = NULL, *line = NULL, *token = NULL;
	int i, ret;

	i = 1; /* Line counter */
	line = tofree = strdup (data);
	token = strsep (&line, "\n");
	while (token) {

		ret = zpn_regex_match (token, "^[\t ]+(SECTION|section)[\t ]+\\.text.*$");

		if (ret != ZPN_REGEX_NO_MATCH) free (tofree);
		if (ret == ZPN_REGEX_MATCH) return i;
		else if (ret == ZPN_ERROR) return ZPN_ERROR; 

		/* Parse next line */
		token = strsep (&line, "\n");
		i++;
	}

	free (tofree);
	return ZPN_REGEX_NO_MATCH;
}

void free_memory_for_gfc_building (char *tofree) {

	free (tofree);
	return;
}

int allocate_memory_for_gfc_building (char *data, p_graph graph) {

	char *line = data;
	int line_nb;

	/* Count line break (it's a little tricky) */
	for (line_nb = 0 ; line[line_nb] ; line[line_nb] == '\n' ? line_nb++ : *line++);

	/* Allocate memory for lookup_table in graph structure */
	graph->lookup_table = (p_node_list *) malloc (sizeof (p_node_list) * line_nb);
	graph->len_lookup_table = line_nb;
	if (graph->lookup_table == NULL) 
		return ZPN_ERROR;

	return ZPN_OK;
}

int search_label (char *data, char *label) {

	char *tofree = NULL, *line = NULL, *token = NULL;
	char pattern[NASM_MAX_LEN_IDENTIFIERS + 100];
	int i, ret;

	i = 1; /* Line counter */
	line = tofree = strdup (data);
	token = strsep (&line, "\n");
	while (token) {

		snprintf (pattern, sizeof (pattern), "^[\t ]*%s:.*$", label);
		ret = zpn_regex_match (token, pattern);

		if (ret != ZPN_REGEX_NO_MATCH) free (tofree);
		if (ret == ZPN_REGEX_MATCH) return i;
		else if (ret == ZPN_ERROR) return ZPN_ERROR_REGEX; 

		/* Parse next line */
		token = strsep (&line, "\n");
		i++;
	}

	free (tofree);
	return -1;

}

int build_gfc_linearly (char *data, p_graph graph) {

	int ret, start, previous_label = -1, i, label = -1;
	int search_next_label = 1;
	char *line = NULL, *tofree = NULL, *token = NULL;
	p_zpn_str str_struct = NULL;

	/* Allocate memory for gfc building */
	ret = allocate_memory_for_gfc_building (data, graph);
	if (ret == ZPN_ERROR) {

			fprintf (stderr, "%s:%d error with malloc function\n", __FILE__, __LINE__);
			goto free_and_fail;
	}

	/* Search code section */
	start = search_code_section (data);
	if (start == ZPN_ERROR) {

		zpn_print_error ();
		goto free_and_fail;

	} else if (start == ZPN_REGEX_NO_MATCH) {

		fprintf (stderr, "Cannot find .text section !\n");
		goto free_and_fail;

	} else fprintf (stdout, "Found new section at line %d\n", start);

	/* Find again start of .text section */
	line = jump_to_line (data, start + 1);
	if (line == NULL) {
		fprintf (stderr, "Cannot jump to right line number (start of section)\n");
		goto free_and_fail;
	}

	tofree = line = strdup (line);
	token = strsep (&line, "\n");

	i = start + 2;
	while (token) {
		
		/* Terminate on new section */
		ret = zpn_regex_match (token, 
		  "^[\t ]+(SECTION|section|SEGMENT|segment)"
		  "[\t ]+\\.(text|bss|data).*$");
		if (ret == ZPN_REGEX_MATCH) {
			fprintf (stdout, "Found new section at line %d\n", i);
			break;
		} else if (ret == ZPN_ERROR) {
			zpn_print_error ();
			goto free_and_fail;
		} 

		/* Match label */
		ret = zpn_regex_match (token, 
		  "^[\t ]*[a-zA-Z]+[a-zA-Z0-9_$#.~?@]*\\:.*$");
		if (ret == ZPN_REGEX_MATCH) {
		
			if (previous_label != -1)
				add_node_to_list (graph, previous_label, i);
			previous_label = i;
			search_next_label = 0;
			add_node_to_list (graph, i, -1);

		} else if (ret == ZPN_ERROR) {
			zpn_print_error ();
			goto free_and_fail;
		} 

		if (search_next_label) goto parse_next_line;

		/* Match ret instruction */
		ret = zpn_regex_match (token, "^[\t ]*ret.*$");
		if (ret == ZPN_REGEX_MATCH) previous_label = i + 1;
		else if (ret == ZPN_ERROR) {
			zpn_print_error ();
			goto free_and_fail;
		} 

		/* Match jmp or conditional jump and get label */
		ret = zpn_regex_get (token, "^[\t ]+(jmp|jo|jno|js|jns|je|"
		  "jz|jne|jnz|jb|jnae|jc|jnb|jae|jnc|jbe|jna|ja|jnbe|jl|jnge|"
		  "jge|jnl|jle|jng|jg|jnle|jp|jpe|jnp|jpo|jcxz|jecxz)[\t ]+"
		  "([a-zA-Z]+[a-zA-Z0-9]*).*$", &str_struct); 

		if (ret == ZPN_ERROR) {

			zpn_print_error ();
			goto free_and_fail;

		} else if (ret >= ZPN_REGEX_MATCH){

			if (str_struct->n == 3) {
				
				label = search_label (data, str_struct->str[2]);
				if (label == ZPN_ERROR_REGEX) {

					zpn_print_error ();
					goto free_and_fail2;
				}

				add_node_to_list (graph, previous_label, label);
				if (strcmp (str_struct->str[1], "jmp")) {
					add_node_to_list (graph, previous_label, i+1);
					previous_label = i + 1;
				} else {
					search_next_label = 1;
					previous_label = -1;
				} 
			}
		}
		zpn_free_str (str_struct);

		parse_next_line:
		/* Parse next line */
		token = strsep (&line, "\n");

		i++;
	}
	
	/* Free memory */
	free_memory_for_gfc_building (tofree);
	return ZPN_OK;

	free_and_fail2:
	zpn_free_str (str_struct);

	free_and_fail:
	free_memory_for_gfc_building (tofree);
	return ZPN_ERROR;
}

int main (int argc, const char *argv[]) {

	FILE *fd;
	char *data = NULL;
	int size, ret;
	t_graph graph;

    /* Usage */
    if (argc < 3) {
    	
    	fprintf (stderr, "Usage: %s x86_assembly_file dot_file\n", argv[0]);
    	exit (EXIT_FAILURE);
    }
    
	/* Initialize misc. data */
	init_graph (&graph);

	/* Map file into memory */
	size = zpn_map_file (argv[1], &data);
	if (size == ZPN_ERROR) {
		zpn_print_error ();
		exit (EXIT_FAILURE);
	}

	/* build graph flow control linearly */
	ret = build_gfc_linearly (data, &graph);
	munmap (data, size);

	if (ret == ZPN_ERROR) {
		free_graph (&graph);
		exit (EXIT_FAILURE);
	}

	/* Write directed graph in GraphViz format */
	fd = fopen (argv[2], "w");
	if (fd == NULL) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		zpn_print_error ();
		free_graph (&graph);
	}
	
	write_directed_graph (fd, &graph);
	free_graph (&graph);

	fclose (fd);

	exit (EXIT_SUCCESS);
}

