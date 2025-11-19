# Makefile for sysinfo

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -I. \
	-I/usr/src/sys/contrib/openzfs/include \
	-I/usr/include/cddl/lib/libzfs \
	-I/usr/include/cddl/compat/opensolaris \
	-I/usr/include/cddl/contrib/opensolaris/lib/libzfs/common \
	-I/usr/include/cddl/contrib/opensolaris/head
LDFLAGS = -lzfs -lnvpair

# Target executable
TARGET = sysinfo

# Source files
SRCS = sysinfo.c zpool_size.c
OBJS = $(SRCS:.c=.o)

# Header dependencies
HEADERS = libshare.h sys/mnttab.h zpool_size.h

# Installation paths
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Default target
all: $(TARGET)

# Build sysinfo executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Install target
install: $(TARGET)
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)

# Uninstall target
uninstall:
	rm -f $(BINDIR)/$(TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Rebuild everything
rebuild: clean all

# Phony targets
.PHONY: all clean install uninstall rebuild
