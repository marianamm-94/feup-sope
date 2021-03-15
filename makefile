
CC = gcc
CFLAGS = -Wall


xmod: xmod.o
	$(CC) xmod.c -o xmod -lm

clean:
	rm -f xmod *.o
