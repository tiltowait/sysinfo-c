# Makefile for sysinfo

CC = cc
CFLAGS = -Wall -Wextra -O2 -D_FORTIFY_SOURCE=2 -fstack-protector-strong -Wformat-security -Isrc \
	-I/usr/src/sys/contrib/openzfs/include \
	-I/usr/include/cddl/lib/libzfs \
	-I/usr/include/cddl/compat/opensolaris \
	-I/usr/include/cddl/contrib/opensolaris/lib/libzfs/common \
	-I/usr/include/cddl/contrib/opensolaris/head
LDFLAGS = -lzfs -lnvpair
PREFIX ?= /usr/local

all: sysinfo

sysinfo: src/sysinfo.c src/zfs.c src/zfs.h src/libshare.h src/sys/mnttab.h
	$(CC) $(CFLAGS) src/sysinfo.c src/zfs.c -o sysinfo $(LDFLAGS)

clean:
	rm -f sysinfo

install: sysinfo
	install -d $(PREFIX)/bin
	install -m 755 sysinfo $(PREFIX)/bin

.PHONY: all clean install
