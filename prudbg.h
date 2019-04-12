/* 
 *
 *  PRU Debug Program header file
 *  (c) Copyright 2011, 2013 by Arctica Technologies
 *  Written by Steven Anderson
 *
 */

#ifndef PRUDBG_H
#define PRUDBG_H

// default processor to use if none is specified on the command line when prudebug is started
#define DEFAULT_PROCESSOR_INDEX	AM335x

// list of processors to use in the define above (DEFAULT_PROCESSOR_INDEX)
// value for define must match the array index in the processor structure
// in the prudbg.c file.
#define AM1707			0
#define AM335x			1


// general settings
#define MAX_CMD_LEN		25
#define MAX_CMDARGS_LEN		200
#define MAX_COMMAND_LINE	(MAX_CMD_LEN + MAX_CMDARGS_LEN + 1)
#define MAX_ARGS		10
#define MAX_PRU_MEM		0xFFFF
#define NUM_OF_PRU		2
#define MAX_NUM_OF_PRUS		16					// maximum number of PRUs to expect in any processor
#define MAX_BREAKPOINTS		5
#define MAX_WATCH		5
#define MAX_PROC_NAME		20

// register offsets
#define PRU_CTRL_REG		0x0000
#define PRU_STATUS_REG		0x0001
#define PRU_INTGPR_REG		0x0100

// PRU control register bit flags
#define PRU_REG_PCRESET_MASK	0x0000FFFF
#define PRU_REG_RUNSTATE	0x00008000
#define PRU_REG_SINGLE_STEP	0x00000100
#define PRU_REG_COUNT_EN	0x00000008
#define PRU_REG_SLEEPING	0x00000004
#define PRU_REG_PROC_EN		0x00000002
#define PRU_REG_SOFT_RESET	0x00000001

// defines for PRU memory mapping method requeste by user
#define ACCESS_GUESS		0
#define ACCESS_UIO		1
#define ACCESS_MEM		2

// defines for command repeats
#define LAST_CMD_NONE		0
#define LAST_CMD_D		1
#define LAST_CMD_DD		2
#define LAST_CMD_DI		3
#define LAST_CMD_DIS		4
#define LAST_CMD_SS		5

// defines for structures below
#define BP_UNUSED		0
#define BP_ACTIVE		1

#define WA_UNUSED		0
#define WA_PRINT_ON_ANY		1
#define WA_HALT_ON_VALUE	2

#define TRUE			1
#define FALSE			0


// global structures
struct breakpoints {
	unsigned char		state;
	unsigned int		address;
};

struct watchvariable {
	unsigned char		state;
	unsigned int		address;
	unsigned int		value;
	unsigned int		old_value;
};


// global variables
extern unsigned int		*pru;
extern unsigned int		pru_inst_base[], pru_ctrl_base[], pru_data_base[];
extern unsigned int		pru_num;
extern struct breakpoints	bp[MAX_NUM_OF_PRUS][MAX_BREAKPOINTS];
extern struct watchvariable	wa[MAX_NUM_OF_PRUS][MAX_WATCH];


// function prototypes
int cmd_input(char *prompt, char *cmd, char *cmdargs, unsigned int *argptrs, unsigned int *numargs);
void printhelp(void);
void printhelpbrief(void);
int cmd_print_breakpoints(void);
int cmd_set_breakpoint (unsigned int bpnum, unsigned int addr);
int cmd_clear_breakpoint (unsigned int bpnum);
int cmd_d (int offset, int addr, int len);
int cmd_dis (int offset, int addr, int len);
void cmd_halt(void);
int cmd_loadprog(unsigned int addr, char *fn);
void cmd_printregs(void);
void cmd_run(void);
void cmd_runss(void);
void cmd_single_step(void);
void cmd_soft_reset(void);
void cmd_print_watch(void);
void cmd_clear_watch (unsigned int wanum);
void cmd_set_watch_any (unsigned int wanum, unsigned int addr);
void cmd_set_watch (unsigned int wanum, unsigned int addr, unsigned int value);
void disassemble(char *str, unsigned int inst);

#endif // PRUDBG_H

