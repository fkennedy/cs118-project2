CC = gcc
REMOVE = rm -rf

.SILENT:

default: server client

server: server.c
	$(CC) server.c -o server

client: client.c
	$(CC) client.c -o client

clean:
	$(REMOVE) client server
	$(REMOVE) *.o