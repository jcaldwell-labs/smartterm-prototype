# cc-bash Makefile
#
# Project structure:
#   src/         - Source files
#   tests/       - Test files
#   config/      - Configuration templates
#   scripts/     - Utility scripts
#   docs/        - Documentation
#   packaging/   - Package configurations

CC = gcc
# Strict warnings to catch issues early (see CONTRIBUTING.md)
CFLAGS = -Wall -Wextra -Wpedantic -Wunused -Wshadow -Wformat=2 -std=c11
# Additional warnings for development (enable with: make STRICT=1)
ifdef STRICT
CFLAGS += -Werror
endif
# -lutil provides forkpty() on Linux for PTY-based command execution
LDFLAGS = -lutil

# Static analysis tool (install: apt install cppcheck / brew install cppcheck)
CPPCHECK = cppcheck
CPPCHECK_FLAGS = --enable=warning,style,performance,portability --error-exitcode=1 \
                 --suppress=missingIncludeSystem --quiet

# Directories
SRC_DIR = src
TEST_DIR = tests
CONFIG_DIR = config

# cc-bash (Claude Code-style bash wrapper)
TARGET = cc-bash
SRC = $(SRC_DIR)/cc-bash.c

# Unit tests
TEST_TARGET = test_unit
TEST_SRC = $(TEST_DIR)/test_unit.c

.PHONY: all clean run test test-unit help install uninstall lint check release debug

# Default: build cc-bash
all: $(TARGET)

# Build cc-bash
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Build with debug symbols
debug: $(SRC)
	$(CC) $(CFLAGS) -g -O0 -DDEBUG -o $(TARGET) $(SRC) $(LDFLAGS)
	@echo "Built debug binary with symbols: $(TARGET)"
	@echo "Use with: gdb ./$(TARGET)"

# Run cc-bash
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(TEST_TARGET)

# Build optimized release binary
release: $(SRC)
	$(CC) $(CFLAGS) -O2 -DNDEBUG -o $(TARGET) $(SRC) $(LDFLAGS)
	@echo "Built optimized release binary: $(TARGET)"

# Build unit tests
$(TEST_TARGET): $(TEST_SRC)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRC)

# Run all tests (unit first, then static analysis)
test: $(TARGET) $(TEST_TARGET)
	@echo "Running unit tests..."
	@./$(TEST_TARGET)
	@echo ""
	@echo "Running static analysis tests..."
	@chmod +x $(TEST_DIR)/test_cc_bash.sh
	@./$(TEST_DIR)/test_cc_bash.sh

# Run only unit tests
test-unit: $(TEST_TARGET)
	@./$(TEST_TARGET)

# Static analysis with cppcheck
lint:
	@echo "Running static analysis..."
	@if command -v $(CPPCHECK) >/dev/null 2>&1; then \
		$(CPPCHECK) $(CPPCHECK_FLAGS) $(SRC) $(TEST_SRC) && \
		echo "Static analysis passed!"; \
	else \
		echo "Warning: cppcheck not found. Install with: apt install cppcheck"; \
		echo "Skipping static analysis."; \
	fi

# Full quality check (run before committing)
check: lint test
	@echo ""
	@echo "=========================================="
	@echo "All quality checks passed!"
	@echo "=========================================="

# Installation directories
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
SYSCONFDIR = $(HOME)/.cc-bash
CONFIG_FILE = $(HOME)/.cc-bashrc
CONFIG_TEMPLATE = $(CONFIG_DIR)/cc-bashrc.template

# Install cc-bash
install: $(TARGET)
	@echo "Installing cc-bash..."
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/
	@echo "Creating config directory..."
	install -d $(SYSCONFDIR)
	install -d $(SYSCONFDIR)/plugins
	@if [ ! -f $(CONFIG_FILE) ]; then \
		echo "Creating default config file..."; \
		install -m 644 $(CONFIG_TEMPLATE) $(CONFIG_FILE); \
	else \
		echo "Config file already exists, skipping..."; \
	fi
	@echo ""
	@echo "Installation complete!"
	@echo "  Binary:  $(BINDIR)/cc-bash"
	@echo "  Config:  $(CONFIG_FILE)"
	@echo "  Plugins: $(SYSCONFDIR)/plugins/"
	@echo ""
	@echo "Run 'cc-bash' to start"

# Uninstall cc-bash
uninstall:
	@echo "Uninstalling cc-bash..."
	rm -f $(BINDIR)/$(TARGET)
	@echo "Binary removed. Config files preserved in $(SYSCONFDIR)"

help:
	@echo "═══════════════════════════════════════════════════════════════"
	@echo "  cc-bash: Claude Code-style Bash Wrapper"
	@echo "═══════════════════════════════════════════════════════════════"
	@echo ""
	@echo "BUILD:"
	@echo "  make              Build cc-bash binary (default)"
	@echo "  make all          Same as 'make'"
	@echo "  make debug        Build with debug symbols (-g -O0)"
	@echo "  make run          Build and run cc-bash"
	@echo "  make release      Build optimized release binary (-O2)"
	@echo "  make clean        Remove build artifacts"
	@echo ""
	@echo "TESTING:"
	@echo "  make test         Run all tests (unit + static analysis)"
	@echo "  make test-unit    Run unit tests only"
	@echo "  make lint         Run static analysis (cppcheck)"
	@echo ""
	@echo "QUALITY:"
	@echo "  make check        Full quality check (lint + test) ⭐ USE BEFORE COMMIT"
	@echo "  make STRICT=1     Build with -Werror (warnings as errors)"
	@echo ""
	@echo "INSTALL:"
	@echo "  make install      Install to /usr/local/bin (requires sudo)"
	@echo "                    Or: make install PREFIX=~/.local (no sudo)"
	@echo "  make uninstall    Remove installation"
	@echo ""
	@echo "VARIABLES:"
	@echo "  PREFIX=$(PREFIX)"
	@echo "  CC=$(CC)"
	@echo "  CFLAGS=$(CFLAGS)"
	@echo ""
	@echo "PROJECT STRUCTURE:"
	@echo "  src/       Source files (cc-bash.c, cc-bash-sdk.py)"
	@echo "  tests/     Test files"
	@echo "  config/    Configuration templates"
	@echo "  scripts/   Utility scripts (install.sh)"
	@echo "  docs/      Documentation"
	@echo ""
	@echo "DEVELOPMENT WORKFLOW:"
	@echo "  1. Make changes in src/"
	@echo "  2. Run 'make check' before committing"
	@echo "  3. Enable pre-commit hook: git config core.hooksPath .githooks"
	@echo ""
	@echo "See CONTRIBUTING.md for detailed guidelines"
	@echo "═══════════════════════════════════════════════════════════════"
