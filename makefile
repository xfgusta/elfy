CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=gnu11 -O2
LDFLAGS=-lelf

DESTDIR=
PREFIX=$(DESTDIR)/usr/local
BINDIR=$(PREFIX)/bin

INSTALL=install -p

all: elfy

install: all
	mkdir -p $(BINDIR)
	$(INSTALL) -m 0755 elfy $(BINDIR)

uninstall:
	rm -f $(BINDIR)/elfy

clean:
	rm -f elfy
