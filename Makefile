# SmartTerm / cc-bash Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS_POC = -lncurses -lreadline
# -lutil provides forkpty() on Linux for PTY-based command execution (Issue #22)
LDFLAGS_CCBASH = -lutil

# Original POC
POC_TARGET = smartterm_poc
POC_SRC = smartterm_poc.c

# cc-bash (Claude Code-style bash wrapper)
CCBASH_TARGET = cc-bash
CCBASH_SRC = cc-bash.c

# Unit tests
TEST_UNIT_TARGET = test_unit
TEST_UNIT_SRC = tests/test_unit.c

.PHONY: all clean run poc cc-bash run-ccbash test test-unit help install uninstall

# Default: build cc-bash
all: $(CCBASH_TARGET)

# Build cc-bash
$(CCBASH_TARGET): $(CCBASH_SRC)
	$(CC) $(CFLAGS) -o $(CCBASH_TARGET) $(CCBASH_SRC) $(LDFLAGS_CCBASH)

# Build original POC
poc: $(POC_TARGET)

$(POC_TARGET): $(POC_SRC)
	$(CC) $(CFLAGS) -o $(POC_TARGET) $(POC_SRC) $(LDFLAGS_POC)

# Run cc-bash
run: $(CCBASH_TARGET)
	./$(CCBASH_TARGET)

# Run original POC
run-poc: $(POC_TARGET)
	./$(POC_TARGET)

clean:
	rm -f $(POC_TARGET) $(CCBASH_TARGET) $(TEST_UNIT_TARGET)

# Build unit tests
$(TEST_UNIT_TARGET): $(TEST_UNIT_SRC)
	$(CC) $(CFLAGS) -o $(TEST_UNIT_TARGET) $(TEST_UNIT_SRC)

# Run all tests (unit first, then static analysis)
test: $(CCBASH_TARGET) $(TEST_UNIT_TARGET)
	@echo "Running unit tests..."
	@./$(TEST_UNIT_TARGET)
	@echo ""
	@echo "Running static analysis tests..."
	@chmod +x tests/test_cc_bash.sh
	@./tests/test_cc_bash.sh

# Run only unit tests
test-unit: $(TEST_UNIT_TARGET)
	@./$(TEST_UNIT_TARGET)

# Installation directories
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
SYSCONFDIR = $(HOME)/.cc-bash
CONFIG_FILE = $(HOME)/.cc-bashrc

# Install cc-bash
install: $(CCBASH_TARGET)
	@echo "Installing cc-bash..."
	install -d $(BINDIR)
	install -m 755 $(CCBASH_TARGET) $(BINDIR)/
	@echo "Creating config directory..."
	install -d $(SYSCONFDIR)
	install -d $(SYSCONFDIR)/plugins
	@if [ ! -f $(CONFIG_FILE) ]; then \
		echo "Creating default config file..."; \
		install -m 644 cc-bashrc.template $(CONFIG_FILE); \
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
	rm -f $(BINDIR)/$(CCBASH_TARGET)
	@echo "Binary removed. Config files preserved in $(SYSCONFDIR)"

help:
	@echo "cc-bash: Claude Code-style bash wrapper"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build cc-bash (default)"
	@echo "  run       - Build and run cc-bash"
	@echo "  test      - Run cc-bash test suite"
	@echo "  install   - Install to $(PREFIX)/bin (use sudo)"
	@echo "  uninstall - Remove from $(PREFIX)/bin"
	@echo "  poc       - Build original smartterm POC"
	@echo "  run-poc   - Run original smartterm POC"
	@echo "  clean     - Remove binaries"
	@echo "  help      - Show this help"
	@echo ""
	@echo "cc-bash features:"
	@echo "  - Execute bash commands by default"
	@echo "  - Colored output: commands (cyan), stdout (white), stderr (red)"
	@echo "  - Status bar: cwd, exit code, time"
	@echo "  - Command history (up/down arrows)"
	@echo "  - Special prefixes: # (note), @ (internal command)"
