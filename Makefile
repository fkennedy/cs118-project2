CC = gcc
CFLAGS = -g
REMOVE = rm -rf

.SILENT:

default: server client

server: server.o
	$(CC) $(CFLAGS) server.c helper.c -o server

client: client.o
	$(CC) $(CFLAGS) client.c helper.c -o client

clean:
	$(REMOVE) client server
	$(REMOVE) *.dSYM
	$(REMOVE) *.o
	$(REMOVE) data.log