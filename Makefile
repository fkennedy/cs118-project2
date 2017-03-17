CC = gcc
CFLAGS = -g
REMOVE = rm -rf
TARGET = project2
FILES = README.md Makefile *.c *.h

.SILENT:

default: clean server client

server: server.o
	$(CC) $(CFLAGS) server.c helper.c -o server

client: client.o
	$(CC) $(CFLAGS) client.c helper.c -o client

clean:
	$(REMOVE) client server
	$(REMOVE) *.dSYM
	$(REMOVE) *.o
	$(REMOVE) data.log
	$(REMOVE) *.tar.gz

dist: default
	tar -cvzf $(TARGET).tar.gz $(FILES)