# Makefile for AZ Editor v1.8.0

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -D_POSIX_C_SOURCE=200809L -Wno-sign-compare -Wno-stringop-truncation
LIBS = -lncurses
TARGET = az
SOURCE = az.c
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

all: $(TARGET)

$(TARGET): $(SOURCE)
	@echo "Compiling AZ Editor..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)
	@echo "✓ Build complete: ./$(TARGET)"

install: $(TARGET)
	@echo "Installing to $(BINDIR)..."
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/
	@echo "✓ Installed: $(BINDIR)/$(TARGET)"
	@echo ""
	@echo "Usage: az filename.txt"

uninstall:
	@echo "Removing $(BINDIR)/$(TARGET)..."
	rm -f $(BINDIR)/$(TARGET)
	@echo "✓ Uninstalled"

clean:
	@echo "Cleaning build files..."
	rm -f $(TARGET)
	@echo "✓ Clean"

test: $(TARGET)
	./$(TARGET) test.txt

help:
	@echo "AZ Editor v1.8.0 - Build Commands"
	@echo ""
	@echo "  make          - Compile the editor"
	@echo "  make install  - Install to $(BINDIR)"
	@echo "  make uninstall- Remove from system"
	@echo "  make clean    - Remove build files"
	@echo "  make test     - Run with test file"
	@echo ""

.PHONY: all install uninstall clean test help
