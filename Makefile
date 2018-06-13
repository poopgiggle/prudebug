
CC=arm-linux-gnueabihf-gcc

objs = prudbg.o cmdinput.o cmd.o printhelp.o da.o uio.o privs.o

CFLAGS=-g -O3 -Wall

prudebug : ${objs}
	${CC} ${objs} ${CFLAGS} -lreadline -o prudebug

clean :
	$(RM) *.o
	$(RM) prudebug
