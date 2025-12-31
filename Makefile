# SmartTerm / cc-bash Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS_POC = -lncurses -lreadline
LDFLAGS_CCBASH =

# Original POC
POC_TARGET = smartterm_poc
POC_SRC = smartterm_poc.c

# cc-bash (Claude Code-style bash wrapper)
CCBASH_TARGET = cc-bash
CCBASH_SRC = cc-bash.c

.PHONY: all clean run poc cc-bash run-ccbash help

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
	rm -f $(POC_TARGET) $(CCBASH_TARGET)

help:
	@echo "cc-bash: Claude Code-style bash wrapper"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build cc-bash (default)"
	@echo "  run       - Build and run cc-bash"
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
