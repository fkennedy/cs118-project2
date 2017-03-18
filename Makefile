CC = gcc
CFLAGS = -lrt
REMOVE = rm -rf
TARGET = project2
UID1 = 404667930
UID2 = 404491851
FILES = README Makefile *.c *.h report.pdf

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
	$(REMOVE) *.tar

dist: default
	tar -cvzf $(TARGET)_$(UID1)_$(UID2).tar $(FILES)

dist-mac: mac
	tar -cvzf $(TARGET)_$(UID1)_$(UID2).tar $(FILES)
