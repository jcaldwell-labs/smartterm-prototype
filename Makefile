# SmartTerm Simple Library Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lncurses -lreadline

# Library targets
LIB_NAME = libsmartterm_simple.a
LIB_SRC = smartterm_simple.c
LIB_OBJ = smartterm_simple.o
LIB_HEADER = smartterm_simple.h

# Example targets
EXAMPLE = example
EXAMPLE_SRC = example.c

# Original POC
POC_TARGET = smartterm_poc
POC_SRC = smartterm_poc.c

.PHONY: all library example poc clean run test help

# Default: build library and example
all: library example

# Build the library
library: $(LIB_NAME)

$(LIB_NAME): $(LIB_OBJ)
	ar rcs $(LIB_NAME) $(LIB_OBJ)
	@echo "Library built: $(LIB_NAME)"

$(LIB_OBJ): $(LIB_SRC) $(LIB_HEADER)
	$(CC) $(CFLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)

# Build the example
example: $(EXAMPLE)

$(EXAMPLE): $(EXAMPLE_SRC) $(LIB_NAME)
	$(CC) $(CFLAGS) -o $(EXAMPLE) $(EXAMPLE_SRC) -L. -lsmartterm_simple $(LDFLAGS)
	@echo "Example built: $(EXAMPLE)"

# Build the original POC
poc: $(POC_TARGET)

$(POC_TARGET): $(POC_SRC)
	$(CC) $(CFLAGS) -o $(POC_TARGET) $(POC_SRC) $(LDFLAGS)

# Run the example
run: example
	./$(EXAMPLE)

# Run the POC
run-poc: poc
	./$(POC_TARGET)

# Clean all build artifacts
clean:
	rm -f $(LIB_OBJ) $(LIB_NAME) $(EXAMPLE) $(POC_TARGET)
	@echo "Clean complete"

# Test the library
test: example
	@echo "Testing SmartTerm Simple Library..."
	@echo "1. Run: make run"
	@echo "2. Try commands: help, clear, quit"
	@echo "3. Try contexts: !command, #comment, @special, /search"
	@echo "4. Test history: Use arrow keys (up/down)"
	@echo "5. Test editing: Ctrl+A (home), Ctrl+E (end), Ctrl+K (kill line)"
	@echo ""
	@echo "Or run the original POC: make run-poc"

# Help
help:
	@echo "SmartTerm Simple Library"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build library and example (default)"
	@echo "  library   - Build libsmartterm_simple.a"
	@echo "  example   - Build example program"
	@echo "  poc       - Build original POC"
	@echo "  run       - Build and run example"
	@echo "  run-poc   - Build and run POC"
	@echo "  clean     - Remove all build artifacts"
	@echo "  test      - Show testing instructions"
	@echo "  help      - Show this help"
