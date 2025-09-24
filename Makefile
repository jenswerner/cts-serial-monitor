# Makefile for CTS Monitor
# Linux C project build configuration

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic
CFLAGS += -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L
DEBUG_CFLAGS = -g -O0 -DDEBUG
RELEASE_CFLAGS = -O2 -DNDEBUG

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
TESTDIR = tests
DOCDIR = docs

# Target executable
TARGET = cts_monitor

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
DEPS = $(OBJECTS:.o=.d)

# Include directories
INCLUDES = -I$(INCDIR)

# Libraries
LIBS = 

# Check for libftdi1 support
HAS_LIBFTDI1 := $(shell pkg-config --exists libftdi1 && echo 1)
ifeq ($(HAS_LIBFTDI1),1)
    CFLAGS += -DHAVE_LIBFTDI1 $(shell pkg-config --cflags libftdi1)
    LIBS += $(shell pkg-config --libs libftdi1)
    $(info Building with libftdi1 support)
else
    $(info Building without libftdi1 support - install libftdi1-dev for FTDI device enhancement)
endif

# Default build type
BUILD_TYPE ?= debug

# Set flags based on build type
ifeq ($(BUILD_TYPE),release)
    CFLAGS += $(RELEASE_CFLAGS)
else
    CFLAGS += $(DEBUG_CFLAGS)
endif

# Default target
.PHONY: all
all: $(TARGET)

# Create build directory
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

# Build target executable
$(TARGET): $(BUILDDIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)
	@echo "Built $(TARGET) ($(BUILD_TYPE) mode)"

# Build object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPS)

# Debug build
.PHONY: debug
debug:
	$(MAKE) BUILD_TYPE=debug

# Release build
.PHONY: release
release:
	$(MAKE) BUILD_TYPE=release

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
	rm -f $(TARGET)
	@echo "Cleaned build artifacts"

# Install target
.PHONY: install
install: $(TARGET)
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/
	@echo "Installed $(TARGET) to /usr/local/bin/"

# Uninstall target
.PHONY: uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled $(TARGET)"

# Run the program
.PHONY: run
run: $(TARGET)
	./$(TARGET)

# Run with verbose output
.PHONY: run-verbose
run-verbose: $(TARGET)
	./$(TARGET) -v

# Run in background
.PHONY: run-daemon
run-daemon: $(TARGET)
	./$(TARGET) -d

# Format source code (requires clang-format)
.PHONY: format
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		find $(SRCDIR) $(INCDIR) -name "*.c" -o -name "*.h" | xargs clang-format -i; \
		echo "Code formatted"; \
	else \
		echo "clang-format not found, skipping formatting"; \
	fi

# Static analysis (requires cppcheck)
.PHONY: analyze
analyze:
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --inconclusive --std=c99 $(INCLUDES) $(SRCDIR)/; \
	else \
		echo "cppcheck not found, skipping static analysis"; \
	fi

# Memory check with valgrind
.PHONY: memcheck
memcheck: $(TARGET)
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) -v; \
	else \
		echo "valgrind not found, skipping memory check"; \
	fi

# Create documentation (requires doxygen)
.PHONY: docs
docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
		echo "Documentation generated in $(DOCDIR)/"; \
	else \
		echo "doxygen not found, skipping documentation generation"; \
	fi

# Show build information
.PHONY: info
info:
	@echo "CTS Monitor Build Information"
	@echo "============================="
	@echo "Target: $(TARGET)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Compiler: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "Sources: $(SOURCES)"
	@echo "Objects: $(OBJECTS)"
	@echo "Includes: $(INCLUDES)"

# Check for required tools
.PHONY: check-tools
check-tools:
	@echo "Checking for required tools..."
	@which gcc > /dev/null || echo "gcc not found"
	@which make > /dev/null || echo "make not found" 
	@which valgrind > /dev/null || echo "valgrind not found (optional)"
	@which cppcheck > /dev/null || echo "cppcheck not found (optional)"
	@which clang-format > /dev/null || echo "clang-format not found (optional)"
	@which doxygen > /dev/null || echo "doxygen not found (optional)"
	@echo "Tool check complete"

# Help target
.PHONY: help
help:
	@echo "CTS Monitor - Available Make Targets"
	@echo "===================================="
	@echo "Build Targets:"
	@echo "  all          - Build the project (default: debug mode)"
	@echo "  debug        - Build in debug mode"
	@echo "  release      - Build in release mode"
	@echo "  clean        - Remove build artifacts"
	@echo ""
	@echo "Installation:"
	@echo "  install      - Install to /usr/local/bin/"
	@echo "  uninstall    - Remove from /usr/local/bin/"
	@echo ""
	@echo "Execution:"
	@echo "  run          - Run the program"
	@echo "  run-verbose  - Run with verbose output"
	@echo "  run-daemon   - Run as daemon"
	@echo ""
	@echo "Development:"
	@echo "  format       - Format source code (requires clang-format)"
	@echo "  analyze      - Static analysis (requires cppcheck)"
	@echo "  memcheck     - Memory check (requires valgrind)"
	@echo "  docs         - Generate documentation (requires doxygen)"
	@echo ""
	@echo "Utilities:"
	@echo "  info         - Show build information"
	@echo "  check-tools  - Check for required/optional tools"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Build Variables:"
	@echo "  BUILD_TYPE   - Set to 'debug' or 'release' (default: debug)"

# Prevent make from deleting intermediate files
.PRECIOUS: $(BUILDDIR)/%.o