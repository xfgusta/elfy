CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=gnu11 -O2
LDFLAGS=-lelf

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man

INSTALL=install -p
INSTALL_MAN=install -p -m 0644

all: elfy

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 elfy $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	$(INSTALL_MAN) elfy.1 $(DESTDIR)$(MANDIR)/man1

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/elfy
	rm -f $(DESTDIR)$(MANDIR)/man1/elfy.1

clean:
	rm -f elfy
