# SPDX-License-Identifier: GPL-2.0

PROGNAME = wp-backup

PREFIX = /usr
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
MANDIR = $(DATADIR)/man

CFLAGS = -g -O2 -Wall
LDFLAGS =

-include config.mak

CFLAGS += -I/usr/include/libxml2 -lxml2
CFLAGS += -I/usr/include/x86_64-linux-gnu -lcurl

CC = gcc
FIND = find
INSTALL = install
RM = rm -f

OBJFILES = $(patsubst %.c, %.o, $(shell $(FIND) src -type f -name "*.c"))
MANPAGES = $(patsubst %.adoc, %.gz, $(shell $(FIND) doc -type f -name "*.adoc"))

depfile = $(dir $@).depend/$(notdir $@).d


.PHONY: all doc test install uninstall clean

all: $(PROGNAME)

$(PROGNAME): $(OBJFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(PROGNAME)

%.depend:
	mkdir -p $@

%.o: %.c Makefile
	@mkdir -p $(dir $@)/.depend
	$(CC) $(CFLAGS) -MMD -MP -MF $(depfile) -c $< -o $@

doc: $(MANPAGES)

doc/%: doc/%.adoc
	asciidoctor -d manpage -b manpage $< -o $@

doc/%.gz: doc/%
	gzip < $< > $@

test:
	@echo "TODO"

install: all
	$(INSTALL) -m 755 $(PROGNAME) $(BINDIR)
	$(INSTALL) -m 644 doc/$(PROGNAME).1.gz $(MANDIR)/man1

uninstall:
	$(RM) $(BINDIR)/$(PROGNAME)
	$(RM) $(MANDIR)/man1/$(PROGNAME).1.gz

clean:
	$(RM) *.o $(PROGNAME)
	$(RM) -r */.depend
	$(RM) doc/*.[1-7] doc/*.gz
