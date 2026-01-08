# SmartTerm / cc-bash Makefile

CC = gcc
# Strict warnings to catch issues early (see CONTRIBUTING.md)
CFLAGS = -Wall -Wextra -Wpedantic -Wunused -Wshadow -Wformat=2 -std=c11
# Additional warnings for development (enable with: make STRICT=1)
ifdef STRICT
CFLAGS += -Werror
endif
LDFLAGS_POC = -lncurses -lreadline
# -lutil provides forkpty() on Linux for PTY-based command execution (Issue #22)
LDFLAGS_CCBASH = -lutil

# Static analysis tool (install: apt install cppcheck / brew install cppcheck)
CPPCHECK = cppcheck
CPPCHECK_FLAGS = --enable=warning,style,performance,portability --error-exitcode=1 \
                 --suppress=missingIncludeSystem --quiet

# Original POC
POC_TARGET = smartterm_poc
POC_SRC = smartterm_poc.c

# cc-bash (Claude Code-style bash wrapper)
CCBASH_TARGET = cc-bash
CCBASH_SRC = cc-bash.c

# Unit tests
TEST_UNIT_TARGET = test_unit
TEST_UNIT_SRC = tests/test_unit.c

.PHONY: all clean run poc cc-bash run-ccbash test test-unit help install uninstall lint check

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

# Static analysis with cppcheck
lint:
	@echo "Running static analysis..."
	@if command -v $(CPPCHECK) >/dev/null 2>&1; then \
		$(CPPCHECK) $(CPPCHECK_FLAGS) $(CCBASH_SRC) $(TEST_UNIT_SRC) && \
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
	@echo "Build Targets:"
	@echo "  all       - Build cc-bash (default)"
	@echo "  run       - Build and run cc-bash"
	@echo "  clean     - Remove binaries"
	@echo ""
	@echo "Quality Targets:"
	@echo "  check     - Run all quality checks (lint + test) [USE BEFORE COMMIT]"
	@echo "  lint      - Run static analysis (cppcheck)"
	@echo "  test      - Run full test suite (unit + static)"
	@echo "  test-unit - Run unit tests only"
	@echo ""
	@echo "Install Targets:"
	@echo "  install   - Install to $(PREFIX)/bin (use sudo)"
	@echo "  uninstall - Remove from $(PREFIX)/bin"
	@echo ""
	@echo "Legacy Targets:"
	@echo "  poc       - Build original smartterm POC"
	@echo "  run-poc   - Run original smartterm POC"
	@echo ""
	@echo "Development workflow:"
	@echo "  1. Make changes"
	@echo "  2. Run 'make check' before committing"
	@echo "  3. Or use git hooks: .githooks/pre-commit"
