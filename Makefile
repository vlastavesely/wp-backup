# SPDX-License-Identifier: GPL-2.0

# Define V=1 to have a more verbose compile.

PROGNAME = wp-backup

PREFIX = /usr
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
MANDIR = $(DATADIR)/man

-include config.mak # FIXME

CFLAGS = -g -O2 -Wall
LDFLAGS =

CC = gcc
FIND = find
INSTALL = install
RM = rm -f
ADOC = asciidoctor

ifneq ($(findstring s, $(MAKEFLAGS)), s)
ifndef V
	QUIET_CC       = @echo '   ' CC $@;
	QUIET_LINK     = @echo '   ' LINK $@;
	QUIET_GEN      = @echo '   ' GEN $@;
	export V
endif
endif

# ------------------------------------------------------------------------------

CURL_CFLAGS = $(shell pkg-config --cflags libcurl)
XML_CFLAGS = $(shell pkg-config --cflags libxml-2.0)
CFLAGS += $(CURL_CFLAGS) $(XML_CFLAGS) $(GCOV_CFLAGS)

CURL_LDFLAGS = $(shell pkg-config --libs libcurl)
XML_LDFLAGS = $(shell pkg-config --libs libxml-2.0)
LDFLAGS += $(CURL_LDFLAGS) $(XML_LDFLAGS)

OBJFILES = $(patsubst %.c, %.o, $(shell $(FIND) src -type f -name "*.c"))
depfile = $(dir $@).depend/$(notdir $@).d

.PHONY: FORCE

$(PROGNAME): $(OBJFILES)
	$(QUIET_LINK)$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(PROGNAME) $(LDFLAGS)

.cflags: FORCE
	@if ! test "x$(CFLAGS)" = "x$(shell cat $@ 2>/dev/null)"; then	\
		echo >&2 "    * new build flags";			\
		echo "$(CFLAGS)" >$@;					\
	fi

%.depend:
	mkdir -p $@

%.o: %.c Makefile .cflags
	@mkdir -p $(dir $@)/.depend
	$(QUIET_CC)$(CC) $(CFLAGS) -MMD -MP -MF $(depfile) -c $< -o $@

# ------------------------------------------------------------------------------

.PHONY: install uninstall clean

install: all
	$(INSTALL) -m 755 $(PROGNAME) $(BINDIR)
	$(INSTALL) -m 644 doc/$(PROGNAME).1.gz $(MANDIR)/man1

uninstall:
	$(RM) $(BINDIR)/$(PROGNAME)
	$(RM) $(MANDIR)/man1/$(PROGNAME).1.gz

clean:
	$(RM) $(PROGNAME) tests/test
	$(RM) */*.o */*.gcno */*.gcda
	$(RM) -r */.depend tests/coverage
	$(RM) doc/*.[1-7] doc/*.gz

# ------------------------------------------------------------------------------

ADOC_FLAGS = -d manpage -b manpage
MANPAGES = $(patsubst %.adoc, %.gz, $(shell $(FIND) doc -type f -name "*.adoc"))

PHONY: doc

doc: $(MANPAGES)

doc/%.gz: doc/%.adoc
	$(QUIET_GEN)$(ADOC) $(ADOC_FLAGS) $< -o $(<:.adoc=) && gzip -f $(<:.adoc=)

# ------------------------------------------------------------------------------

TEST_OBJFILES = $(patsubst %.c, %.o, $(shell $(FIND) tests -type f -name "*.c"))
TEST_OBJFILES += $(filter-out src/wp-backup.o, $(OBJFILES))

.PHONY: test

test: tests/test
	tests/test

tests/test: $(TEST_OBJFILES)
	$(QUIET_LINK)$(CC) $(CFLAGS) $(LDFLAGS) $^ -o tests/test -lcunit

# ------------------------------------------------------------------------------

GCOV_FLAGS = --preserve-paths --branch-probabilities --all-blocks
COVERAGE_FILE = tests/coverage/coverage.info

.PHONY: coverage

coverage: $(COVERAGE_FILE) coverage-report

$(COVERAGE_FILE):
	@mkdir -p tests/coverage
	$(MAKE) GCOV_CFLAGS="-fno-inline -fprofile-arcs -ftest-coverage" test

coverage-report: $(COVERAGE_FILE)
	gcov $(GCOV_FLAGS) --object-directory=. *.c
	lcov --directory src -c -o $(COVERAGE_FILE)
	genhtml -o tests/coverage -t "$(PROGNAME) Test Coverage" $(COVERAGE_FILE)

