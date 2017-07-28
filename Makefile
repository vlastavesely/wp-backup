CC           = gcc
SHELL        = /bin/bash
CFLAGS       = -Wall -Iinclude `pkg-config --libs --cflags libsoup-2.4`

TARGET       = wp-backup
SRCDIR       = src
INCDIR       = include
BUILDDIR     = build
OBJDIR       = build/obj

OBJS =	debug.o			\
	http-client.o		\
	http-request.o		\
	http-response.o		\
	options.o		\
	password-resolver.o	\
	utils.o			\
	wordpress-connection.o

OBJECTS = $(OBJS:%.o=$(OBJDIR)/%.o)


.PHONY: all install uninstall dist clean

all: $(BUILDDIR)/$(TARGET)

install:
	cp -f $(BUILDDIR)/$(TARGET) /usr/bin/wp-backup
	chmod +x /usr/bin/wp-backup

uninstall:
	rm -f /usr/bin/wp-backup

$(BUILDDIR)/$(TARGET): $(OBJECTS) $(SRCDIR)/wp-backup.c include/wp-backup.h
	$(CC) $(SRCDIR)/wp-backup.c $(OBJECTS) $(CFLAGS) $(DEBUG) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c	\
		$(INCDIR)/wp-backup/%.h		\
		include/wp-backup.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(DEBUG) -c $< -o $@

dist:	clean
	cd ..; \
	tar cfvz $(TARGET)/$(TARGET).tar.gz --exclude=$(TARGET)/test.sh	\
		$(TARGET)/*

clean:
	rm -rf $(BUILDDIR) $(TARGET).tar.gz
