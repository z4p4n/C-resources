#ifndef ZPN_REGEX_H
#define ZPN_REGEX_H

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

/* This function match a string with regex				*
 * -------------------------------------------------------------------- *	
 * const char *str_request : the string you want analyze		*
 * const char *str_regex   : the pattern you want to use		*
 * -------------------------------------------------------------------- *	
 * return : ZPN_REGEX_MATCH if match ZPN_REGEX_NO_MATCH if match fail, 	*
 *          ZPN_ERROR if fail due to error				*/
int zpn_regex_match (const char *str_request, const char *str_regex);

/* zpn_regex_get match and store in a struct a specific pattern		*
 * -------------------------------------------------------------------- *
 * char *str_request       : the string you want analyze		*
 * const char *str_regex   : the pattern you want to use		*
 * p_zpn_str *str_struct   : the structure where match will be stored	*
 * -------------------------------------------------------------------- *
 * return : number of element(s) matched (>0) if match,			*
 *          ZPN_REGEX_NO_MATCH if no string was found, 			*
 *          ZPN_ERROR if fail due to error				*/
int zpn_regex_get (char *str_request, const char *str_regex, p_zpn_str *str_struct);

/* zpn_free_str free str_struct 		*
 * -------------------------------------------- *
 * p_zpn_str str_struct : struct to free	*/
void zpn_free_str (p_zpn_str str_struct);

#endif
