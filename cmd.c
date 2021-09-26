/*
 *
 *  PRU Debug Program
 *  (c) Copyright 2011, 2013 by Arctica Technologies
 *  Written by Steven Anderson
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <string.h>

#include "prudbg.h"

// breakpoint management
void cmd_print_breakpoints()
{
	int			i;

	printf("##  Address\n");
	for (i=0; i<MAX_BREAKPOINTS; i++) {
		if (bp[pru_num][i].state == BP_ACTIVE) {
			printf("%02u  0x%04x\n", i, bp[pru_num][i].address);
		} else {
			printf("%02u  UNUSED\n", i);
		}
	}
	printf("\n");
}

// set breakpoint
void cmd_set_breakpoint (unsigned int bpnum, unsigned int addr)
{
	bp[pru_num][bpnum].state = BP_ACTIVE;
	bp[pru_num][bpnum].address = addr;
}

// clear breakpoint
void cmd_clear_breakpoint (unsigned int bpnum)
{
	bp[pru_num][bpnum].state = BP_UNUSED;
}

// dump data memory
void cmd_dx_rows (const char * prefix, unsigned char * data, int offset, int addr, int len)
{
	int			i, j;

	for (i=0; i<len; ) {
		printf ("%s",prefix);

		printf ("[0x%05x]", addr+i);

		for (j=0; (i<len) && (j<8); ++i, ++j)
			printf (" %02x", data[offset+addr+i]);

		printf ("-");

		for (j=0; (i<len) && (j<8); ++i, ++j)
			printf ("%02x ", data[offset+addr+i]);

		printf ("\n");
	}
}

void cmd_d_rows (int offset, int addr, int len)
{
	cmd_dx_rows("", (unsigned char*)pru, offset, addr, len);
}

void cmd_d (int offset, int addr, int len)
{
	printf ("Absolute addr = 0x%05x, offset = 0x%05x, Len = %u\n",
		addr + offset, addr, len);
	cmd_d_rows(offset, addr, len);
	printf("\n");
}

// disassemble instruction memory
void cmd_dis (int offset, int addr, int len)
{
	int			i, k;
	char			inst_str[50];
	unsigned int		status_reg;
	const char		*br_str[] = {" ", "*"};
	int			on_br = 0;
	const char		*pc[] = {"  ", ">>"};
	int			pc_on = 0;

	status_reg = (pru[pru_ctrl_base[pru_num] + PRU_STATUS_REG]) & 0xFFFF;

	for (i=0; i<len; i++) {
		pc_on = (status_reg == (addr + i)) ? 1 : 0;

		on_br = 0;
		for (k=0; k<MAX_BREAKPOINTS; ++k) {
			if ((bp[pru_num][k].state == BP_ACTIVE) &&
			    (bp[pru_num][k].address == (addr + i))) {
				on_br = 1;
				break;
			}
		}

		disassemble(inst_str, pru[offset+addr+i]);
		printf("[0x%04x] 0x%08x %s %s %s\n",
		       addr+i, pru[offset+addr+i], br_str[on_br], pc[pc_on],
		       inst_str);
	}
	printf("\n");
}

// halt the current PRU
void cmd_halt()
{
	unsigned int		ctrl_reg;

	ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];
	ctrl_reg &= ~PRU_REG_PROC_EN;
	pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG] = ctrl_reg;
	printf("PRU%u Halted.\n", pru_num);
}

// load program into instruction memory
int cmd_loadprog(unsigned int addr, char *fn)
{
	int			f, r;
	struct stat		file_info;

	r = stat(fn, &file_info);
	if (r == -1) {
		printf("ERROR: could not open file\n");
		return 1;
	}
	if (((file_info.st_size/4)*4) != file_info.st_size) {
		printf("ERROR: file size is not evenly divisible by 4\n");
	} else {
		f = open(fn, O_RDONLY);
		if (f == -1) {
			printf("ERROR: could not open file 2\n");
		} else {
			if (read(f, &pru[pru_inst_base[pru_num] + addr], file_info.st_size) < 0) {
				perror("loadprog");
			}
			close(f);
			printf("Binary file of size %ld bytes loaded into PRU%u instruction RAM.\n", file_info.st_size, pru_num);
		}
	}
	return 0;
}

// print current PRU registers
void cmd_printregs()
{
	unsigned int		ctrl_reg, reset_pc, status_reg;
	char			*run_state, *single_step, *cycle_cnt_en, *pru_sleep, *proc_en;
	unsigned int		i;
	char			inst_str[50];

	ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];
	status_reg = pru[pru_ctrl_base[pru_num] + PRU_STATUS_REG];
	reset_pc = (ctrl_reg >> 16);
	if (ctrl_reg&PRU_REG_RUNSTATE)
		run_state = "RUNNING";
	else
		run_state = "STOPPED";

	if (ctrl_reg&PRU_REG_SINGLE_STEP)
		single_step = "SINGLE_STEP";
	else
		single_step = "FREE_RUN";

	if (ctrl_reg&PRU_REG_COUNT_EN)
		cycle_cnt_en = "COUNTER_ENABLED";
	else
		cycle_cnt_en = "COUNTER_DISABLED";

	if (ctrl_reg&PRU_REG_SLEEPING)
		pru_sleep = "SLEEPING";
	else
		pru_sleep = "NOT_SLEEPING";

	if (ctrl_reg&PRU_REG_PROC_EN)
		proc_en = "PROC_ENABLED";
	else
		proc_en = "PROC_DISABLED";

	printf("Register info for PRU%u\n", pru_num);
	printf("    Control register: 0x%08x\n", ctrl_reg);
	printf("      Reset PC:0x%04x  %s, %s, %s, %s, %s\n\n", reset_pc, run_state, single_step, cycle_cnt_en, pru_sleep, proc_en);

	disassemble(inst_str, pru[pru_inst_base[pru_num] + (status_reg&0xFFFF)]);
	printf("    Program counter: 0x%04x\n", (status_reg&0xFFFF));
	printf("      Current instruction: %s\n", inst_str);
	printf("      Cycle counter: %u\n\n", pru[pru_ctrl_base[pru_num] + PRU_CYCLE_REG]);

	if (ctrl_reg&PRU_REG_RUNSTATE) {
		printf("    Rxx registers not available since PRU is RUNNING.\n");
	} else {
		for (i=0; i<8; i++)
      printf("    R%02u: 0x%08x    R%02u: 0x%08x    R%02u: 0x%08x    R%02u: 0x%08x\n",
             i,    pru[pru_ctrl_base[pru_num] + PRU_INTGPR_REG + i],
             i+8,  pru[pru_ctrl_base[pru_num] + PRU_INTGPR_REG + i + 8],
             i+16, pru[pru_ctrl_base[pru_num] + PRU_INTGPR_REG + i + 16],
             i+24, pru[pru_ctrl_base[pru_num] + PRU_INTGPR_REG + i + 24]);
	}

	printf("\n");
}

// print current single specific PRU registers
void cmd_printreg(unsigned int i)
{
	unsigned int		ctrl_reg;

	ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];

	if (ctrl_reg&PRU_REG_RUNSTATE) {
		printf("Rxx registers not available since PRU is RUNNING.\n");
	} else {
		printf("R%02u: 0x%08x\n\n",
		       i, pru[pru_ctrl_base[pru_num] + PRU_INTGPR_REG + i]);
	}
}

// print current single specific PRU registers
void cmd_setreg(int i, unsigned int value)
{
	unsigned int		ctrl_reg;

	ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];

	if (ctrl_reg&PRU_REG_RUNSTATE) {
		printf("Rxx registers not available since PRU is RUNNING.\n");
	} else {
		pru[pru_ctrl_base[pru_num] + PRU_INTGPR_REG + i] = value;
	}
}

// print current single specific PRU registers
void cmd_print_ctrlreg(const char * name, unsigned int i)
{
	printf("%s: 0x%08x\n\n", name, pru[pru_ctrl_base[pru_num] + i]);
}

// print current single specific PRU registers
void cmd_print_ctrlreg_uint(const char * name, unsigned int i)
{
	printf("%s: %u\n\n", name, pru[pru_ctrl_base[pru_num] + i]);
}

// print current single specific PRU registers
void cmd_set_ctrlreg(unsigned int i, unsigned int value)
{
	pru[pru_ctrl_base[pru_num] + i] = value;
}

// print current single specific PRU registers
void cmd_set_ctrlreg_bits(unsigned int i, unsigned int bits)
{
	pru[pru_ctrl_base[pru_num] + i] |= bits;
}

// print current single specific PRU registers
void cmd_clr_ctrlreg_bits(unsigned int i, unsigned int bits)
{
	pru[pru_ctrl_base[pru_num] + i] &= ~bits;
}

// start PRU running
void cmd_run()
{
	unsigned int		ctrl_reg;

	// disable single step mode and enable processor
	ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];
	ctrl_reg |= PRU_REG_PROC_EN;
	ctrl_reg &= ~PRU_REG_SINGLE_STEP;
	pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG] = ctrl_reg;
}

// run PRU in a single stepping mode - used for breakpoints and watch variables
// if count is -1, iterate forever, otherwise count down till zero
void cmd_runss(long count)
{
	unsigned int		i, addr;
	unsigned int		done = 0;
	unsigned int		ctrl_reg;
	unsigned long		t_cyc = 0;
	fd_set			rd_fdset;
	struct timeval		tv;
	int			r;

	if (count > 0) {
		printf("Running (will run for %ld steps or until a breakpoint is hit or a key is pressed)....\n", count);
	} else {
		count = -1;
		printf("Running (will run until a breakpoint is hit or a key is pressed)....\n");
	}

	// enter single-step loop
	do {
		// decrease count
		if (count > 0)
			--count;

		// prep some 'select' magic to detect keypress to escape
		FD_ZERO(&rd_fdset);
		FD_SET(STDIN_FILENO, &rd_fdset);
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		// set single step mode and enable processor
		ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];
		ctrl_reg |= PRU_REG_PROC_EN | PRU_REG_SINGLE_STEP;
		pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG] = ctrl_reg;

		// check if we've hit a breakpoint
		addr = pru[pru_ctrl_base[pru_num] + PRU_STATUS_REG] & 0xFFFF;
		for (i=0; i<MAX_BREAKPOINTS; i++) if ((bp[pru_num][i].state == BP_ACTIVE) && (bp[pru_num][i].address == addr)) done = 1;

		// check if we've hit a watch point
//		addr = pru[pru_ctrl_base[pru_num] + PRU_STATUS_REG] & 0xFFFF;
		for (i=0; i<MAX_WATCH; ++i) {
			unsigned char *pru_u8 = (unsigned char*)pru;

			if ((wa[pru_num][i].state == WA_PRINT_ON_ANY) &&
			    (memcmp(wa[pru_num][i].old_value,
				    pru_u8 + pru_data_base[pru_num]*4
					   + wa[pru_num][i].address,
				    wa[pru_num][i].len) != 0)) {

				printf("@0x%04x  [0x%05x] t=%lu: ",
				       addr, wa[pru_num][i].address, t_cyc);
				cmd_d_rows(pru_data_base[pru_num]*4,
					   wa[pru_num][i].address,
					   wa[pru_num][i].len);

				memcpy(wa[pru_num][i].old_value,
				       pru_u8 + pru_data_base[pru_num]*4
					      + wa[pru_num][i].address,
				       wa[pru_num][i].len);
			} else if ((wa[pru_num][i].state == WA_HALT_ON_VALUE) &&
				   (memcmp(wa[pru_num][i].value,
					   pru_u8 + pru_data_base[pru_num]*4
						  + wa[pru_num][i].address,
					   wa[pru_num][i].len) == 0)) {

				printf("@0x%04x  [0x%05x] t=%lu: ",
				       addr, wa[pru_num][i].address, t_cyc);
				cmd_d_rows(pru_data_base[pru_num]*4,
					   wa[pru_num][i].address,
					   wa[pru_num][i].len);

				done = 1;
			}
		}

		// check if we are on a HALT instruction - if so, stop single step execution
		if (pru[pru_inst_base[pru_num] + addr] == 0x2a000000) {
			printf("\nHALT instruction hit.\n");
			done = 1;
		}

		// check if the user has attempted to stop execution of the PRU with a keypress
		r = select (STDIN_FILENO+1, &rd_fdset, NULL, NULL, &tv);

		// increase time
		t_cyc++;
	} while ((!done) && (r == 0) && (count != 0));

	// if there is a character in the stdin queue, read the character
	if (r > 0) getchar();

	printf("\n");

	// print the registers
	cmd_printregs();

	// disable single step mode and disable processor
	ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];
	ctrl_reg &= ~PRU_REG_PROC_EN;
	ctrl_reg &= ~PRU_REG_SINGLE_STEP;
	pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG] = ctrl_reg;
}

void cmd_single_step(unsigned int N)
{
	unsigned int		ctrl_reg;
	unsigned int i;

	for (i = 0; i < N; ++i ) {
		// set single step mode and enable processor
		ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];
		ctrl_reg |= PRU_REG_PROC_EN | PRU_REG_SINGLE_STEP;
		pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG] = ctrl_reg;
	}

	// print the registers
	cmd_printregs();

	// disable single step mode and disable processor
	ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];
	ctrl_reg &= ~PRU_REG_PROC_EN;
	ctrl_reg &= ~PRU_REG_SINGLE_STEP;
	pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG] = ctrl_reg;
}

void cmd_soft_reset()
{
	unsigned int		ctrl_reg;

	ctrl_reg = pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG];
	ctrl_reg &= ~PRU_REG_SOFT_RESET;
	pru[pru_ctrl_base[pru_num] + PRU_CTRL_REG] = ctrl_reg;

	printf("PRU%u reset.\n", pru_num);
}

// print list of watches
void cmd_print_watch()
{
	int			i;

	printf("##  Address  Value\n");
	for (i=0; i<MAX_WATCH; i++) {
		if (wa[pru_num][i].state == WA_PRINT_ON_ANY) {
			printf("%02u  0x%05x     Print on any change from:\n",
			       i, wa[pru_num][i].address);
			cmd_dx_rows("\t\t", wa[pru_num][i].old_value, 0, 0x0,
				    wa[pru_num][i].len);

		} else if (wa[pru_num][i].state == WA_HALT_ON_VALUE) {
			printf("%02u  0x%05x     Halt = \n",
			       i, wa[pru_num][i].address);
			cmd_dx_rows("\t\t", wa[pru_num][i].value, 0, 0x0,
				    wa[pru_num][i].len);

		} else {
			printf("%02u  UNUSED\n", i);
		}
	}
	printf("\n");
}

// clear a watch from list
void cmd_clear_watch (unsigned int wanum)
{
	wa[pru_num][wanum].state = WA_UNUSED;
}

inline unsigned int min(unsigned int a, unsigned int b) {
	return a < b ? a : b;
}

// set a watch for any change in value and no halt
void cmd_set_watch_any (unsigned int wanum, unsigned int addr, unsigned int len)
{
	unsigned char * pru_u8 = (unsigned char*)pru;
	wa[pru_num][wanum].state	= WA_PRINT_ON_ANY;
	wa[pru_num][wanum].address	= addr;
	wa[pru_num][wanum].len		= len;
	memcpy(wa[pru_num][wanum].old_value,
	       pru_u8 + pru_data_base[pru_num]*4 + addr,
	       min(len, MAX_WATCH_LEN));
}

// set a watch for a specific value and halt
void cmd_set_watch (unsigned int wanum, unsigned int addr,
		    unsigned int len, unsigned char * value)
{
	wa[pru_num][wanum].state	= WA_HALT_ON_VALUE;
	wa[pru_num][wanum].address	= addr;
	wa[pru_num][wanum].len		= len;
	memcpy(wa[pru_num][wanum].value, value,
	       min(len, MAX_WATCH_LEN));
}


