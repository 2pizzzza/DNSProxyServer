CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lyaml
SOURCES = main.c config.c dns.c blacklist.c log.c
EXECUTABLE = DNSProxyServer

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE) $(LDFLAGS)

clean:
	rm -f $(EXECUTABLE)

.PHONY: all clean
