#ifndef ZPN_REGEX_H
#define ZPN_REGEX_H

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

#endif
