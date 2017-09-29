/* 
 *
 *  PRU Debug Program
 *  (c) Copyright 2011 by Arctica Technologies
 *  Written by Steven Anderson
 *
 */

#include <stdio.h>

#include "prudbg.h"

void printhelp()
{
	printf("Command help\n\n");
	printf("General hints:\n");
	printf("    - Commands are case insensitive\n");
	printf("    - Address and numeric values can be dec (ex 12), hex (ex "
			"0xC), octal\n");
	printf("      (ex 014), or binary (ex 0b101010)\n");
	printf("    - Memory addresses are byte addressed.\n");
	printf("    - Pressing 'Enter' without a command will rerun a previous"
			"command.  For the\n");
	printf("      `d`, `dd`, or `di` commands subsequent iterations will "
			"display the next\n");
	printf("      block\n\n");

	printf("BR [breakpoint_number [address]]\n");
	printf("    View or set an instruction breakpoint\n");
	printf("     - 'b' by itself will display current breakpoints\n");
	printf("     - breakpoint_number is the breakpoint reference and ranges"
			" from 0 to %u\n", MAX_BREAKPOINTS - 1);
	printf("     - address is the instruction word address that the "
			"processor should stop\n");
	printf("       at (instruction is not executed)\n");
	printf("     - if no address is provided, then the breakpoint is "
			"cleared\n\n");

	printf("CYCLE [clear | off | on ]\n");
	printf("    Display, clear, disable, or enable the cycle count "
			"register.\n\n");

	printf("D <address> [length]\n");
	printf("    Raw dump of PRU data memory (byte offset from beginning of "
			"full PRU memory\n");
	printf("    block - all PRUs)\n\n");

	printf("DD <address> [length]\n");
	printf("    Dump data memory (byte offset from beginning of PRU data "
			"memory)\n\n");

	printf("DI <address> [length]\n");
	printf("    Dump instruction memory (byte offset from beginning of PRU "
			"instruction\n");
	printf("    memory)\n\n");

	printf("DIS <32bit-address> [length]\n");
	printf("    Disassemble instruction memory (32-bit word offset from "
			"beginning of PRU\n");
	printf("    instruction memory)\n\n");

	printf("G\n");
	printf("    Start processor execution of instructions (at current "
			"IP)\n\n");

	printf("GSS [<count>]\n");
	printf("    Start processor execution using automatic single stepping "
			"- this allows\n");
	printf("    running a program with breakpoints.  If the optional "
			"<count> parameter is\n");
	printf("    given, only <count> steps will be made.  If <count> is "
			"either not specified\n");
	printf("    or given as '0', stepping will continue until otherwise "
			"interrupted.\n\n");

	printf("HALT\n");
	printf("    Halt the processor\n\n");

	printf("L <32bit-address> file_name\n");
	printf("    Load program file into instruction memory at 32-bit word "
			"address provided\n");
	printf("    (offset from beginning of instruction memory\n\n");

	printf("PRU <pru_number>\n");
	printf("    Set the active PRU where pru_number ranges from 0 to %u\n",
			NUM_OF_PRU - 1);
	printf("    Some debugger commands do action on active PRU (such as "
			"halt and reset)\n\n");

	printf("Q\n");
	printf("    Quit the debugger and return to shell prompt.\n\n");

	printf("R [value]\n");
	printf("    Display or modify the current PRU registers.\n\n");

	printf("RESET\n");
	printf("    Reset the current PRU\n\n");

	printf("SS\n");
	printf("    Single step the current instruction.\n\n");

	printf("WA [watch_num [<address> [ (len | : value0 [value1 ...]) ]]]\n");
	printf("    Clear or set a watch point\n");
	printf("    For the `WA` command, the <address> may also utilize "
			"aliases to certain\n");
	printf("    registers:\n");
	printf("     * rN    -- address of Nth register (N is in range "
			"0..31)\n");
	printf("     * cycle -- address of CYCLE count register, which, if "
			"enabled, provides a\n");
	printf("       count of PRU cycles that go by for each instruction "
			"executed\n");
	printf("    The various formats have the following effects\n");
	printf("     - format 1:  wa -- print watch point list\n");
	printf("     - format 2:  wa watch_num -- clear watch point "
			"watch_num\n");
	printf("     - format 3:  wa watch_num address -- set a watch point "
			"(watch_num) so any\n");
	printf("       change at that byte address in data memory will be "
			"printed during program\n");
	printf("       execution with gss command\n");
	printf("     - format 4:  wa watch_num address len -- set a watch point"
			" (watch_num) so\n");
	printf("       any change at that byte address for <len> bytes in data "
			"memory will be\n");
	printf("       printed during program execution with gss command\n");
	printf("     - format 5:  wa watch_num address : value0 value1 ... -- "
			"set a watch point\n");
	printf("       (watch_num) so that the program (run with gss) will be "
			"halted when the\n");
	printf("       memory span at that location location equals the values "
			"specified\n");
	printf("     NOTE: for watchpoints to work, you must use gss command "
			"to run the program\n\n");

	printf("WR <address> value1 [value2 [value3 ...]]\n");
	printf("    Write a byte value to a raw (offset from beginning of full "
			"PRU memory\n");
	printf("    block--all PRUs)\n");
	printf("    <address> is a byte address from the beginning of the PRU "
			"subsystem memory\n");
	printf("    block\n\n");

	printf("WRD <address> value1 [value2 [value3 ...]]\n");
	printf("    Write a byte value to PRU data memory (byte offset from "
			"beginning of PRU\n");
	printf("    data memory)\n\n");

	printf("WRI <address> value1 [value2 [value3 ...]]\n");
	printf("    Write a byte value to PRU instruction memory (byte offset "
			"from beginning of\n");
	printf("    PRU instruction memory)\n\n");

	printf("A brief version of help is available with the command hb\n");

	printf("\n");
}

void printhelpbrief()
{
	printf("Command help\n\n");
	printf("    BR [breakpoint_number [address]] - View or set an instruction breakpoint\n");
	printf("    D <address> [length] - Raw dump of PRU data memory (byte offset from beginning of full PRU memory block - all PRUs)\n");
	printf("    DD <address> [length] - Dump data memory (byte offset from beginning of PRU data memory)\n");
	printf("    DI <address> [length] - Dump instruction memory (byte offset from beginning of PRU instruction memory)\n");
	printf("    DIS <32bit-address> [length] - Disassemble instruction memory (32-bit word offset from beginning of PRU instruction memory)\n");
	printf("    G - Start processor execution of instructions (at current IP)\n");
	printf("    GSS - Start processor execution using automatic single stepping - this allows running a program with breakpoints\n");
	printf("    HALT - Halt the processor\n");
	printf("    L <32bit-address> file_name - Load program file into instruction memory\n");
	printf("    PRU pru_number - Set the active PRU where pru_number ranges from 0 to %u\n", NUM_OF_PRU - 1);
	printf("    Q - Quit the debugger and return to shell prompt.\n");
	printf("    R - Display the current PRU registers.\n");
	printf("    RESET - Reset the current PRU\n");
	printf("    SS - Single step the current instruction.\n");
	printf("    WA [watch_num [address [ (len | : value0 [value1 ...]) ]]] - Clear or set a watch point\n");
	printf("    WR <address> value1 [value2 [value3 ...]] - Write a byte value to a raw (offset from beginning of full PRU memory block)\n");
	printf("    WRD <address> value1 [value2 [value3 ...]] - Write a byte value to PRU data memory for current PRU\n");
	printf("    WRI <address> value1 [value2 [value3 ...]] - Write a byte value to PRU instruction memory for current PRU\n");

	printf("\n");
}

