CC = gcc
CFLAGS = -Wall -Wextra -g -I src
LDFLAGS = -lyaml
SOURCES = src/main.c src/core/dns.c src/config/config.c src/blacklist/blacklist.c src/log/log.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = DNSProxyServer

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean