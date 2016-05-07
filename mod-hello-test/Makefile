CCFLAG = -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ie1000.d/ -Ie1000.d/uapi/ -L/usr/lib32
CC = gcc

hello: hello.o hello.dummy.o
	$(CC) -m32 -L/usr/lib32 hello.o hello.dummy.o -o hello

hello.o: hello.c
	$(CC) $(CCFLAG) hello.c -c

hello.dummy.o: hello.dummy.c
	$(CC) $(CCFLAG) hello.dummy.c -c
