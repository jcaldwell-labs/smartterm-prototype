# SmartTerm Test Suite

This directory contains tests for the SmartTerm library.

## Test Structure

- `test_framework.h` - Simple test framework with assertions
- `test_basic.c` - Basic API tests (config, initialization)
- `test_*.c` - Additional test files

## Running Tests

Build and run all tests:
```bash
make -f Makefile.lib test
```

Run individual test:
```bash
./build/test/test_basic
```

## Test Coverage

Currently tested:
- ✅ Configuration initialization
- ✅ Default settings
- ✅ Error code definitions
- ✅ Context type definitions

Planned tests:
- [ ] Output buffer operations
- [ ] Thread safety
- [ ] Search functionality
- [ ] Export functionality
- [ ] Status bar updates
- [ ] Theme management

## Note on Terminal Tests

SmartTerm requires a terminal for full functionality. Tests that require
a real terminal (ncurses rendering, readline input) are marked as such
and may be skipped in headless environments.

For full integration testing, run examples manually:
```bash
./build/bin/repl
./build/bin/chat_client
./build/bin/log_viewer
```

## Memory Testing

Check for memory leaks with valgrind:
```bash
valgrind --leak-check=full ./build/test/test_basic
```

## Adding New Tests

1. Create `test_name.c` in this directory
2. Include `test_framework.h`
3. Use `TEST_ASSERT` macros for checks
4. Add to Makefile.lib test target
5. Document what is being tested
