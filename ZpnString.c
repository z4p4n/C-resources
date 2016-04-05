#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ZpnError.h"
#include "ZpnString.h"

extern int __zpn_error__;
extern int __zpn_error_type__;

/* zpn_clean_str clean str_struct               *
 * -------------------------------------------- *
 * p_zpn_str str_struct : struct to free        */
void zpn_clean_str (p_zpn_str str_struct) {

	str_struct->next = NULL;
	str_struct->str  = NULL;
	str_struct->n  = 0;
}

/* zpn_free_str free str_struct 		*
 * -------------------------------------------- *
 * p_zpn_str str_struct : struct to free	*/
void zpn_free_str (p_zpn_str str_struct) {

	p_zpn_str tmp1 = str_struct, tmp2 = NULL;
	int i;

	while (tmp1 != NULL) {

		if (tmp1->str != NULL) {

			for (i = 0 ; i < tmp1->n ; i++)
				free (tmp1->str[i]);
			free (tmp1->str);
		}

		tmp2 = tmp1->next;
		free (tmp1);
		tmp1 = tmp2;
	}

	return;
}


/* zpn_add_str add str to str_struct                    *
 * ---------------------------------------------------- *
 * p_zpn_str str_struct : struct on which we add string *
 * ---------------------------------------------------- *
 * return : next str_struct for adding next string      *
 *          NULL if fail due to memory					*/
p_zpn_str *zpn_add_str (p_zpn_str *str_struct, char *s) {

	*str_struct = (p_zpn_str) malloc (sizeof (t_zpn_str));
	if (*str_struct == NULL) {
		__zpn_error__ = ZPN_ERROR_MALLOC;
		return NULL;
	}

	zpn_clean_str (*str_struct);

	(*str_struct)->n = 1;
	(*str_struct)->str = (char **) malloc (sizeof (char *));
	if ((*str_struct)->str == NULL) {

		free (*str_struct);
		__zpn_error__ = ZPN_ERROR_MALLOC;
		return NULL;
	}

	(*str_struct)->str[0] = strdup (s);
	if ((*str_struct)->str[0] == NULL) {

		free (*str_struct);
		free ((*str_struct)->str);
		__zpn_error__ = ZPN_ERROR_MALLOC;
		return NULL;
	}
	
	return &((*str_struct)->next);
}


