/* 
 *
 *  PRU Debug Program - command input function
 *  (c) Copyright 2011, 2013 by Arctica Technologies
 *  Written by Steven Anderson
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <readline/readline.h>
#include <readline/history.h>


#include "prudbg.h"

int cmd_input(char *prompt, char *cmd, char *cmdargs, unsigned int *argptrs, unsigned int *numargs)
{
	unsigned int		i, on_zero;
	unsigned int full_len;

	// collect command until space or return
	char * buf = readline (prompt);

	if (!buf)
		return -1;

	add_history(buf);

	// replace spaces and return with zeros
	full_len = strlen(buf);
	for (i=0; i<full_len; i++) if (buf[i] == ' ') buf[i] = 0;

	// copy command (first word) to cmd argument and shift to upper case
	for (i=0; i<(strlen(buf)+1); i++) cmd[i] = toupper(buf[i]);

	// build index array and count number of arguments
	for (i=strlen(cmd), on_zero=TRUE, numargs[0]=0; i<full_len; i++) {
		if (on_zero) {
			if (buf[i] != 0) {
				on_zero = FALSE;
				argptrs[numargs[0]++] = i;
			}
		} else {
			if (buf[i] == 0) on_zero = TRUE;
		}
	}

	for (i=0; i<full_len+1; i++) cmdargs[i] = buf[i];

	free(buf);

	return 0;
}

