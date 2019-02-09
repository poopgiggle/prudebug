/* 
 *
 *  PRU Debug Program - UIO routines
 *  (c) Copyright 2011,2013 by Arctica Technologies
 *  Written by Steven Anderson
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "uio.h"


// get UIO devices and put into structure
int uio_getprussfile(char *devname, size_t len)
{
	DIR			*d;
	struct dirent		*dent;
	char			fn[UIO_MAX_UIO_FILEPATH];
	FILE			*fd;
	char			s_name[UIO_MAX_DEV_NAME];
	
	// clear user buffer
	devname[0] = 0;
	
	// open directory /dev and scan for uio* files
	d = opendir("/dev");
	
	// read in first entry
	dent = readdir(d);
	
	// scan the entries
	while (dent != NULL) {
		// determine if this is a uio* file
		if (dent->d_name[0] == 'u' && dent->d_name[1] == 'i' && dent->d_name[2] == 'o' && devname[0] == 0) {
			// get uio device name and version
			snprintf(fn, UIO_MAX_UIO_FILEPATH, "/sys/class/uio/%s/name", dent->d_name);
			fd = fopen (fn, "r");
			if (fgets(s_name, UIO_MAX_DEV_NAME, fd) == NULL) {
				printf("Could not read from /sys/class/uio/%s/name\n", dent->d_name);
			}
			s_name[strlen(s_name)-1] = 0;
			if (!strncmp(s_name, "pruss", 5)) {
				snprintf(devname, len, "/dev/%s", dent->d_name);
			}
			fclose(fd);
		}
		
		// read next directory entry
		dent = readdir(d);
	}
	
	// close directory handle
	closedir(d);
	
	return 0;
}

