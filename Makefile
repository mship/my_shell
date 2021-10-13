#Makefile
CC = gcc
INCLUDE = /usr/lib

all: shell

shell: shell.c
	$(CC) -o shell shell.c $(CFLAGS) $(LIBS)

clean:
	rm -f shell



