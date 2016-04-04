#ifndef ZPN_FILES_H
#define ZPN_FILES_H

/* zpn_map_file map filename into memory						*
 * ------------------------------------------------------------ *
 * const char *filename : file name of file to mapping			*
 * char **data : pages in memory								*
 * ------------------------------------------------------------ *
 * return : size of mapping, else ZPN_ERROR						*/
int zpn_map_file (const char *filename, char **data);

#endif
