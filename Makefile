# Makefile for sysinfo

CC = cc
CFLAGS = -Wall -Wextra -I. \
	-I/usr/src/sys/contrib/openzfs/include \
	-I/usr/include/cddl/lib/libzfs \
	-I/usr/include/cddl/compat/opensolaris \
	-I/usr/include/cddl/contrib/opensolaris/lib/libzfs/common \
	-I/usr/include/cddl/contrib/opensolaris/head
LDFLAGS = -lzfs -lnvpair
PREFIX ?= /usr/local

all: sysinfo

sysinfo: sysinfo.o zpool_size.o
	$(CC) sysinfo.o zpool_size.o -o sysinfo $(LDFLAGS)

sysinfo.o: sysinfo.c zpool_size.h
	$(CC) $(CFLAGS) -c sysinfo.c

zpool_size.o: zpool_size.c zpool_size.h libshare.h sys/mnttab.h
	$(CC) $(CFLAGS) -c zpool_size.c

clean:
	rm -f sysinfo.o zpool_size.o sysinfo

install: sysinfo
	install -d $(PREFIX)/bin
	install -m 755 sysinfo $(PREFIX)/bin

.PHONY: all clean install
