
CC=arm-linux-gnueabihf-gcc

objs = prudbg.o cmdinput.o cmd.o printhelp.o da.o uio.o

prudebug : ${objs}
	${CC} ${objs} -lreadline -o prudebug

clean :
	rm *.o
	rm prudebug
