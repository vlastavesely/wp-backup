# SPDX-License-Identifier: GPL-2.0

PROGNAME = wp-backup

PREFIX = /usr
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

CFLAGS = -g -O2 -Wall
LDFLAGS =

CC = gcc
FIND = find
INSTALL = install
RM = rm -f

OBJFILES = $(patsubst %.c, %.o, $(shell $(FIND) src -type f -name "*.c"))
MANPAGES = $(patsubst %.adoc, %.gz, $(shell $(FIND) doc -type f -name "*.adoc"))


.PHONY: all doc install uninstall clean

all: $(PROGNAME)

$(PROGNAME): $(OBJFILES)

doc: $(MANPAGES)

doc/%: doc/%.adoc
	asciidoctor -d manpage -b manpage $< -o $@

doc/%.gz: doc/%
	gzip < $< > $@

install: all
	$(INSTALL) -m 755 $(PROGNAME) $(BINDIR)
	$(INSTALL) -m 644 doc/$(PROGNAME).1.gz $(MANDIR)/man1

uninstall:
	$(RM) $(BINDIR)/$(PROGNAME)
	$(RM) $(MANDIR)/man1/$(PROGNAME).1.gz

clean:
	$(RM) doc/*.[1-7] doc/*.gz
