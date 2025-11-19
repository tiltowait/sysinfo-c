# Makefile for sysinfo

CC = cc
CFLAGS = -Wall -Wextra -O2 -I. \
	-I/usr/src/sys/contrib/openzfs/include \
	-I/usr/include/cddl/lib/libzfs \
	-I/usr/include/cddl/compat/opensolaris \
	-I/usr/include/cddl/contrib/opensolaris/lib/libzfs/common \
	-I/usr/include/cddl/contrib/opensolaris/head
LDFLAGS = -lzfs -lnvpair
PREFIX ?= /usr/local

all: sysinfo

sysinfo: sysinfo.c zpool_size.c zpool_size.h libshare.h sys/mnttab.h
	$(CC) $(CFLAGS) sysinfo.c zpool_size.c -o sysinfo $(LDFLAGS)

clean:
	rm -f sysinfo

install: sysinfo
	install -d $(PREFIX)/bin
	install -m 755 sysinfo $(PREFIX)/bin

.PHONY: all clean install
