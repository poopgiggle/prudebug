
CC=arm-linux-gnueabihf-gcc

objs = prudbg.o cmdinput.o cmd.o printhelp.o da.o uio.o

CFLAGS=-g -O3

prudebug : ${objs}
	${CC} ${objs} ${CFLAGS} -lreadline -o prudebug

clean :
	rm *.o
	rm prudebug
