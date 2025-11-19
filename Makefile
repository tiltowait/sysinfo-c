# Makefile for zpool_size and sysinfo

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -I. \
	-I/usr/src/sys/contrib/openzfs/include \
	-I/usr/include/cddl/lib/libzfs \
	-I/usr/include/cddl/compat/opensolaris \
	-I/usr/include/cddl/contrib/opensolaris/lib/libzfs/common \
	-I/usr/include/cddl/contrib/opensolaris/head
LDFLAGS = -lzfs -lnvpair

# Target executables
TARGET = zpool_size
SYSINFO_TARGET = sysinfo

# Header dependencies
HEADERS = libshare.h sys/mnttab.h zpool_size.h

# Installation paths
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Default target
all: $(TARGET) $(SYSINFO_TARGET)

# Build zpool_size standalone executable
$(TARGET): zpool_size_standalone.o
	$(CC) zpool_size_standalone.o -o $(TARGET) $(LDFLAGS)

# Build sysinfo executable (links with zpool_size.o)
$(SYSINFO_TARGET): sysinfo.o zpool_size.o
	$(CC) sysinfo.o zpool_size.o -o $(SYSINFO_TARGET) $(LDFLAGS)

# Compile zpool_size as standalone (with main function)
zpool_size_standalone.o: zpool_size.c $(HEADERS)
	$(CC) $(CFLAGS) -DZPOOL_SIZE_STANDALONE -c zpool_size.c -o zpool_size_standalone.o

# Compile zpool_size as library (without main function)
zpool_size.o: zpool_size.c $(HEADERS)
	$(CC) $(CFLAGS) -c zpool_size.c -o zpool_size.o

# Compile sysinfo
sysinfo.o: sysinfo.c $(HEADERS)
	$(CC) $(CFLAGS) -c sysinfo.c -o sysinfo.o

# Install target
install: $(TARGET) $(SYSINFO_TARGET)
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)
	install -m 755 $(SYSINFO_TARGET) $(BINDIR)

# Uninstall target
uninstall:
	rm -f $(BINDIR)/$(TARGET)
	rm -f $(BINDIR)/$(SYSINFO_TARGET)

# Clean build artifacts
clean:
	rm -f *.o $(TARGET) $(SYSINFO_TARGET)

# Rebuild everything
rebuild: clean all

# Phony targets
.PHONY: all clean install uninstall rebuild
