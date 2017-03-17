CC = gcc
CFLAGS = -lrt
REMOVE = rm -rf
TARGET = project2
FILES = README.md Makefile *.c *.h

.SILENT:

default: clean server client

mac: clean server-mac client-mac

server: server.o
	$(CC) server.c helper.c -o server $(CFLAGS)

server-mac: server.o
	$(CC) server.c helper.c -o server

client: client.o
	$(CC) client.c helper.c -o client $(CFLAGS)

client-mac: client.o
	$(CC) client.c helper.c -o client

clean:
	$(REMOVE) client server
	$(REMOVE) *.dSYM
	$(REMOVE) *.o
	$(REMOVE) received.data
	$(REMOVE) *.tar.gz

dist: default
	tar -cvzf $(TARGET).tar.gz $(FILES)
