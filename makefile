CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=gnu11 -O2
LDFLAGS=-lelf

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin

INSTALL=install -p

all: elfy

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 elfy $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/elfy

clean:
	rm -f elfy
