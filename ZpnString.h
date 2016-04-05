#ifndef ZPN_STRING_H
#define ZPN_STRING_H

/* Usefull for storing strings		*
 * ------------------------------------ *
 * char **str : array of strings	*
 * int n      : number of strings	*
 * p_zpn_str next : next string		*/
typedef struct _zpn_str t_zpn_str, *p_zpn_str;
struct _zpn_str {

	char **str;
	int n;
	p_zpn_str next;

};

/* zpn_free_str free str_struct 		*
 * -------------------------------------------- *
 * p_zpn_str str_struct : struct to free	*/
void zpn_free_str (p_zpn_str str_struct);

/* zpn_add_str add str to str_struct                    *
 * ---------------------------------------------------- *
 * p_zpn_str str_struct : struct on which we add string *
 * ---------------------------------------------------- *
 * return : next str_struct for adding next string      *
 *          NULL if fail due to memory					*/
p_zpn_str *zpn_add_str (p_zpn_str *str_struct, char *s);

#endif
