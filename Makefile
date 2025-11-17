# SmartTerm Proof of Concept Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lncurses -lreadline

TARGET = smartterm_poc
SRC = smartterm_poc.c

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

test: $(TARGET)
	@echo "Testing SmartTerm POC..."
	@echo "1. Run: make run"
	@echo "2. Try commands: help, clear"
	@echo "3. Try contexts: !command, #comment, @special"
	@echo "4. Test multi-line: Use arrow keys, history (up/down)"
	@echo "5. Exit: type 'quit'"

help:
	@echo "SmartTerm Proof of Concept"
	@echo ""
	@echo "Targets:"
	@echo "  all   - Build the POC (default)"
	@echo "  run   - Build and run"
	@echo "  clean - Remove binary"
	@echo "  test  - Show testing instructions"
	@echo "  help  - Show this help"
