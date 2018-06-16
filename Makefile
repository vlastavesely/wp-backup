# SPDX-License-Identifier: GPL-2.0

# Define V=1 to have a more verbose compile.

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

TEST_OBJFILES = $(patsubst %.c, %.o, $(shell $(FIND) tests -type f -name "*.c"))
TEST_OBJFILES += $(filter-out src/wp-backup.o, $(OBJFILES))

depfile = $(dir $@).depend/$(notdir $@).d

ifneq ($(findstring s, $(MAKEFLAGS)), s)
ifndef V
	QUIET_CC       = @echo '   ' CC $@;
	QUIET_LINK     = @echo '   ' LINK $@;
	export V
endif
endif


.PHONY: all doc test install uninstall clean

all: $(PROGNAME)

$(PROGNAME): $(OBJFILES)
	$(QUIET_LINK)$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(PROGNAME)

%.depend:
	mkdir -p $@

%.o: %.c Makefile
	@mkdir -p $(dir $@)/.depend
	$(QUIET_CC)$(CC) $(CFLAGS) -MMD -MP -MF $(depfile) -c $< -o $@

test: tests/test
	tests/test

tests/test: $(TEST_OBJFILES)
	$(QUIET_LINK)$(CC) $(CFLAGS) $(LDFLAGS) $^ -o tests/test -lcunit

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
	$(RM) */*.o $(PROGNAME) tests/test
	$(RM) */*.gcda */*.gcno
	$(RM) -r */.depend
	$(RM) doc/*.[1-7] doc/*.gz

distclean: clean
	$(RM) -rf autom4te.cache aclocal.m4
	$(RM) -f autoscan.log configure.scan configure
	$(RM) -f config.log config.status config.mak
