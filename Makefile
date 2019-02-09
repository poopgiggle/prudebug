
#CC=arm-linux-gnueabihf-gcc

objs = prudbg.o cmdinput.o cmd.o printhelp.o da.o uio.o privs.o

prefix ?=/usr

GIT_VERSION=$(shell git describe --tags 2> /dev/null || echo prudebug-0.25)
VERSION=$(subst prudebug-,,$(GIT_VERSION))


CFLAGS=-g -O3 -Wall -DVERSION=\"$(VERSION)\"

prudebug : ${objs}
	${CC} ${objs} ${CFLAGS} -lreadline -o prudebug

install : prudebug
	mkdir -p $(prefix)/bin
	install -m 0755 prudebug $(prefix)/bin/

clean :
	$(RM) *.o
	$(RM) prudebug
