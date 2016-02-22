#ifndef ZPN_ERROR_H
#define ZPN_ERROR_H

#define ZPN_ERROR_HTTP         -7
#define ZPN_ERROR_SMTP         -6
#define ZPN_ERROR_ERRNO        -5
#define ZPN_ERROR_SOCKET       -4
#define ZPN_ERROR_REGEX        -3
#define ZPN_ERROR_MALLOC       -2
#define ZPN_ERROR              -1
#define ZPN_REGEX_NO_MATCH      0
#define ZPN_OK                  0
#define ZPN_REGEX_MATCH         1

#include <regex.h>
#include <stdio.h>

/* Usefull variable for detect error */
extern int __zpn_error__;
extern int __zpn_error_type__;
extern regex_t __zpn_preg__;
extern char __zpn_error_buf__[BUFSIZ];

/* This function display last error */
void zpn_print_error ();

#define SET_ERROR(error, type) { __zpn_error__ = error; __zpn_error_type__ = type; }
#define SET_ERROR_BUF(error, str) { \
				    __zpn_error__ = error; \
				    strncpy (__zpn_error_buf__, str, BUFSIZ - 1); \
				  }

#endif
