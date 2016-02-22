#include <stdio.h>

#include <stdlib.h>
#include <regex.h>
#include <string.h>

#include "ZpnRegex.h"
#include "ZpnError.h"

extern int __zpn_error__;
extern int __zpn_error_type__;
extern regex_t __zpn_preg__;

/* zpn_clean_str clean str_struct 		*
 * -------------------------------------------- *
 * p_zpn_str str_struct : struct to free	*/
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

/* zpn_regex_get match and store in a struct a specific pattern		*
 * -------------------------------------------------------------------- *
 * char *str_request       : the string you want analyze		*
 * const char *str_regex   : the pattern you want to use		*
 * p_zpn_str *str_struct   : the structure where match will be stored	*
 * -------------------------------------------------------------------- *
 * return : number of element(s) matched (>0) if match,			*
 *          ZPN_REGEX_NO_MATCH if no string was found, 			*
 *          ZPN_ERROR if fail due to error				*/
int zpn_regex_get (char *str_request, const char *str_regex, p_zpn_str *str_struct) {

	int nb_match = 0, ret, i, match, start, end;
	p_zpn_str tmp;
	size_t nmatch = 0, size;
	regmatch_t *pmatch = NULL;
	char *request = str_request;

	*str_struct = (p_zpn_str) malloc (sizeof (t_zpn_str));
	tmp = *str_struct;
	zpn_clean_str (tmp);

	ret = regcomp (&__zpn_preg__, str_regex, REG_EXTENDED); 
	if (ret == 0) {

		nmatch = __zpn_preg__.re_nsub + 1;
		pmatch = (regmatch_t *) malloc (sizeof (regmatch_t) * nmatch);

		if (pmatch == NULL) {

			__zpn_error__ = ZPN_ERROR_MALLOC;
			zpn_free_str (*str_struct);
			return ZPN_ERROR;
		}
		
		while ((match = regexec (&__zpn_preg__, request, nmatch, pmatch, 0)) == 0) {

			tmp->next = (p_zpn_str) malloc (sizeof (t_zpn_str));
			zpn_clean_str (tmp->next);
			if (tmp->next == NULL) {

				regfree (&__zpn_preg__);
				free (pmatch);
				__zpn_error__ = ZPN_ERROR_MALLOC;
				zpn_free_str (*str_struct);
				return ZPN_ERROR;
			}

			tmp->str = (char **) malloc (nmatch * sizeof (char *));
			if (tmp->str == NULL) {

				regfree (&__zpn_preg__);
				free (pmatch);
				__zpn_error__ = ZPN_ERROR_MALLOC;
				zpn_free_str (*str_struct);
				return ZPN_ERROR;
			}

			for (i = 0; i < nmatch ; i++) {

				start = pmatch[i].rm_so;
				end   = pmatch[i].rm_eo;
				size = end - start;

				tmp->str[i] = malloc ((size + 1) * sizeof (char)); 
				if (tmp->str == NULL) {

					regfree (&__zpn_preg__);
					free (pmatch);
					__zpn_error__ = ZPN_ERROR_MALLOC;
					zpn_free_str (*str_struct);
					return ZPN_ERROR;
				}

				strncpy (tmp->str[i], &request[start], size);
				tmp->str[i][size] = '\0';

				tmp->n += 1;
			}

			nb_match++;
			tmp = tmp->next;

			/* Continue to read the rest of the string */
			start = pmatch[0].rm_so;
			end   = pmatch[0].rm_eo;
			size = end - start;
			request = &request[start + size];
		}

		regfree (&__zpn_preg__);

		if (match == REG_NOMATCH && nb_match == 0) {

			free (pmatch);
			return ZPN_REGEX_NO_MATCH;

		} else if (match != REG_NOMATCH) {

			free (pmatch);
			SET_ERROR (ZPN_ERROR_REGEX, ret);
			return ZPN_ERROR;
		}
	
	} else {		

		SET_ERROR (ZPN_ERROR_REGEX, ret);
		return ZPN_ERROR;
	}

	free (pmatch);

	return nb_match;
}

/* This function match a string with regex				*
 * -------------------------------------------------------------------- *	
 * const char *str_request : the string you want analyze		*
 * const char *str_regex   : the pattern you want to use		*
 * -------------------------------------------------------------------- *	
 * return : ZPN_REGEX_MATCH if match ZPN_REGEX_NO_MATCH if match fail, 	*
 *          ZPN_ERROR if fail due to error				*/
int zpn_regex_match (const char *str_request, const char *str_regex) {

	int ret, match;

	ret = regcomp (&__zpn_preg__, str_regex, REG_NOSUB | REG_EXTENDED); 

	if (ret == 0) {

		match = regexec (&__zpn_preg__, str_request, 0, NULL, 0);
		regfree (&__zpn_preg__);

		if (match == 0) 
			return ZPN_REGEX_MATCH;
		else if (match == REG_NOMATCH)
			return ZPN_REGEX_NO_MATCH;
		else {

			SET_ERROR (ZPN_ERROR_REGEX, ret);
			return ZPN_ERROR;
		}
	
	} else {		

		SET_ERROR (ZPN_ERROR_REGEX, ret);
		return ZPN_ERROR;
	}
}

