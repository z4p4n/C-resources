#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

#include "ZpnError.h"
#include "ZpnFiles.h"

extern int __zpn_error__;
extern int __zpn_error_type__;
extern char __zpn_error_buf__[BUFSIZ];

/* zpn_map_file map filename into memory						*
 * ------------------------------------------------------------ *
 * const char *filename : file name of file to mapping			*
 * char **data : pages in memory								*
 * ------------------------------------------------------------ *
 * return : size of mapping, else ZPN_ERROR						*/
int zpn_map_file (const char *filename, char **data) {

	int fd;
	struct stat sbuf;

	if ((fd = open (filename, O_RDONLY)) == -1) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		return ZPN_ERROR;	
	}

	if (stat (filename, &sbuf) == -1) {

		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		return ZPN_ERROR;	
	}

    *data = mmap((caddr_t)0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (*data == (caddr_t)(-1)) {
		SET_ERROR (ZPN_ERROR_ERRNO, errno);
		return ZPN_ERROR;	
    }

	close (fd);
	return sbuf.st_size;
}

