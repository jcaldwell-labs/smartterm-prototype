# Contributing to cc-bash

Thank you for your interest in contributing to cc-bash! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Quick Start: Quality Workflow](#quick-start-quality-workflow)
- [Good First Issues](#good-first-issues)
- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Running Tests Locally](#running-tests-locally)
- [Quality Checks](#quality-checks)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Issue Guidelines](#issue-guidelines)
- [Where to Ask Questions](#where-to-ask-questions)

---

## Quick Start: Quality Workflow

**TL;DR: Run `make check` before every commit.**

```bash
# Enable pre-commit hooks (one-time setup)
git config core.hooksPath .githooks

# Before committing, always run:
make check
```

This runs static analysis and all tests, catching issues before they reach code review.

---

## Good First Issues

New to the project? Look for issues labeled [`good first issue`](https://github.com/jcaldwell-labs/smartterm-prototype/labels/good%20first%20issue).

These issues are:

- **Well-defined**: Clear acceptance criteria
- **Self-contained**: Don't require deep codebase knowledge
- **Mentored**: Maintainers will help guide you

**Great starting points**:

- Documentation improvements
- Adding tests for existing functionality
- Bug fixes with clear reproduction steps
- Small feature additions

**Not sure where to start?** Comment on an issue asking for guidance, or check out [GitHub Discussions](https://github.com/jcaldwell-labs/smartterm-prototype/discussions) for ideas.

---

## Code of Conduct

This project adheres to the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md).

By participating, you are expected to uphold this code. Please report unacceptable behavior by opening an issue or contacting the project maintainers.

**Key principles**:

- Be respectful and professional
- Focus on technical merit
- Provide constructive feedback
- Help maintain a welcoming environment

---

## Getting Started

### Prerequisites

**Prerequisites**:

- C compiler (gcc or clang)
- Make build system
- Git
- cppcheck (for static analysis, optional but recommended)

**Ubuntu/Debian:**

```bash
sudo apt-get install build-essential git cppcheck
```

**macOS:**

```bash
# Install Xcode Command Line Tools (includes gcc, make, git)
xcode-select --install

# Install cppcheck
brew install cppcheck
```

**Note**: cc-bash has minimal dependencies - no ncurses or readline required! It uses raw terminal mode and ANSI escape codes.

### Fork and Clone

1. Fork the repository on GitHub
2. Clone your fork:
   ```bash
   git clone https://github.com/YOUR_USERNAME/smartterm-prototype.git
   cd smartterm-prototype
   ```
3. Add upstream remote:
   ```bash
   git remote add upstream https://github.com/jcaldwell-labs/smartterm-prototype.git
   ```

---

## Development Setup

### Quick Start (5 minutes to running tests)

```bash
# 1. Clone and enter directory
git clone https://github.com/YOUR_USERNAME/smartterm-prototype.git
cd smartterm-prototype

# 2. Build cc-bash
make

# 3. Run cc-bash
./cc-bash

# 4. Exit with Ctrl+D or @quit

# 5. Run tests
make test

# 6. Run quality checks
make check
```

**You're ready to contribute!**

### Building cc-bash

cc-bash is a lightweight Claude Code-style bash wrapper built with pure C (no external dependencies beyond POSIX):

```bash
# Build cc-bash
make

# Run cc-bash
make run

# Run tests
make test

# Clean
make clean
```

### cc-bash Development Notes

When working on cc-bash, keep in mind:

- **No external dependencies**: cc-bash uses only POSIX C and standard terminal control
- **Three-region layout**: Output area (top), prompt area (middle), status area (bottom)
- **Raw terminal mode**: Uses `termios` for non-canonical input handling
- **ANSI escape sequences**: Direct terminal control without ncurses

Key source files:

- `cc-bash.c` - Main implementation (~3300 lines)
- `tests/test_unit.c` - Unit tests (193 tests)
- `tests/test_cc_bash.sh` - Static analysis tests (12 tests)

Key features implemented:

- Aliases, snippets, workflows
- Theme customization
- Event/hook system
- Plugin system

---

## Quality Checks

### Pre-Commit Hooks

Enable automatic quality checks before every commit:

```bash
# Option 1: Configure git (recommended)
git config core.hooksPath .githooks

# Option 2: Symlink
ln -sf ../../.githooks/pre-commit .git/hooks/pre-commit
```

The pre-commit hook runs:

1. Build with strict warnings (`-Werror`)
2. Static analysis (cppcheck)
3. Full test suite

### Manual Quality Checks

```bash
# Full quality check (ALWAYS run before committing)
make check

# Individual checks
make lint        # Static analysis only
make test        # Full test suite
make test-unit   # Unit tests only
make STRICT=1    # Build with -Werror
```

### Self-Review Checklist

Before pushing, verify:

- [ ] **No unused code** - Remove dead functions/variables
- [ ] **No duplicate logic** - Combine identical code paths
- [ ] **State consistency** - Update cursor position after display changes
- [ ] **Test coverage** - New functions have tests
- [ ] **Edge cases** - Document unhandled edge cases
- [ ] **No debug code** - Remove printf debugging, TODO comments

### Common Issues to Avoid

| Issue                  | Prevention                                     |
| ---------------------- | ---------------------------------------------- |
| Unused functions       | Compile with `make STRICT=1`                   |
| Duplicate code paths   | Review conditionals - do branches differ?      |
| Missing cursor updates | After `print_*()` calls, check cursor position |
| Missing tests          | Write tests before or with implementation      |
| Platform issues        | Run `make lint` for portability warnings       |

### Installing Static Analysis Tools

```bash
# Ubuntu/Debian
sudo apt install cppcheck

# macOS
brew install cppcheck
```

---

## Coding Standards

### C Code Style

SmartTerm follows these conventions:

**Naming:**

- `snake_case` for functions and variables
- `PascalCase` for structs and enums
- `ALL_CAPS` for constants and macros
- Prefix public API functions with `smartterm_`
- Prefix internal functions with `_smartterm_` or make them static

**Formatting:**

- 4-space indentation (no tabs)
- 100 character line limit
- Opening braces on new line for functions, same line for control structures
- Use clang-format with provided `.clang-format` config

**Code Organization:**

- Group related functions together
- Add clear section comments
- Document all public API functions
- Keep functions focused and reasonably sized

**Example:**

```c
// Public API function
int smartterm_write(smartterm_ctx *ctx, const char *text, context_type_t ctx_type)
{
    if (!ctx || !text) {
        return -1;
    }

    // Implementation...
    return 0;
}

// Internal helper function
static void _smartterm_internal_helper(void)
{
    // Implementation...
}
```

### Formatting Code

Before submitting changes, format your code:

```bash
# Format all C files
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "./.git/*" -exec clang-format -i {} \;

# Format specific file
clang-format -i path/to/file.c
```

### Documentation

**Code Comments:**

- Document all public API functions with purpose, parameters, and return values
- Use inline comments sparingly for non-obvious logic
- Keep comments up-to-date with code changes

**API Documentation:**

- Update `docs/SMARTTERM-API.md` for API changes
- Update `docs/ARCHITECTURE.md` for architectural changes
- Update README.md for user-facing changes

---

## Running Tests Locally

cc-bash has comprehensive test coverage with unit tests and static analysis.

### Quick Test Commands

```bash
# Run all tests (recommended before committing)
make test

# Run only unit tests (fast feedback)
make test-unit

# Run only static analysis
make lint

# Run full quality check (lint + test)
make check
```

### Understanding Test Output

**Unit Tests** (`make test-unit`):

```
Running unit tests...
[PASS] test_parse_alias
[PASS] test_parse_snippet
[FAIL] test_fuzzy_search - Expected 3, got 2
...
Tests: 193 passed, 0 failed
```

**Static Analysis** (`make lint`):

```
Running static analysis...
Checking cc-bash.c...
Static analysis passed!
```

### Test Organization

- **`tests/test_unit.c`** - Unit tests (193 tests)
  - Configuration parsing
  - Alias/snippet expansion
  - History management
  - String utilities

- **`tests/test_cc_bash.sh`** - Integration tests
  - Static analysis with cppcheck
  - Build verification
  - Compiler warning checks

### Running Specific Tests

To run specific unit tests, modify `test_unit.c` temporarily:

```c
// Comment out tests you don't want to run
// RUN_TEST(test_parse_alias);
RUN_TEST(test_fuzzy_search);  // Only this one runs
// RUN_TEST(test_parse_snippet);
```

### Debugging Test Failures

```bash
# Build with debug symbols
make debug

# Run under gdb
gdb ./test_unit
(gdb) run
(gdb) backtrace

# Or use valgrind to check for memory issues
valgrind --leak-check=full ./test_unit
```

### Writing New Tests

Add tests to `tests/test_unit.c`:

```c
// Test function
void test_my_feature(void) {
    // Arrange
    char *input = "test input";

    // Act
    int result = my_function(input);

    // Assert
    ASSERT_EQ(result, expected_value);
}

// Register test in main()
int main(void) {
    // ... other tests ...
    RUN_TEST(test_my_feature);
    // ...
}
```

---

## Testing

### Running Tests

```bash
# Build and run test suite
make -f Makefile.lib test

# Run specific test
./build/test/test_output
```

### Writing Tests

When adding new functionality:

1. Add tests to appropriate test file in `tests/`
2. Follow existing test patterns
3. Test both success and failure cases
4. Check for memory leaks with valgrind:
   ```bash
   valgrind --leak-check=full ./build/test/test_name
   ```

### Test Coverage

Aim for good coverage of:

- Core API functions
- Error handling paths
- Edge cases
- Thread safety (if applicable)

---

## Submitting Changes

### Creating a Feature Branch

```bash
# Update your local main branch
git checkout main
git pull upstream main

# Create feature branch
git checkout -b feature/your-feature-name
```

### Making Commits

**Commit Message Format:**

```
Short summary (50 chars or less)

More detailed explanation if needed. Wrap at 72 characters.
Explain the "why" not the "what" (code shows the "what").

- Bullet points for multiple changes
- Reference issues with #123 syntax
```

**Examples:**

```
Add thread-safe buffer access with mutex locking

Implement pthread mutex to protect output buffer access from
multiple threads. This prevents race conditions when concurrent
writes occur in multi-threaded applications.

Fixes #42
```

### Pull Request Process

1. **Update your branch:**

   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Run checks:**

   ```bash
   # Format code
   make -f Makefile.lib format

   # Build library
   make -f Makefile.lib lib

   # Run tests
   make -f Makefile.lib test

   # Check for compiler warnings
   make -f Makefile.lib CFLAGS="-Wall -Wextra -Werror"
   ```

3. **Push to your fork:**

   ```bash
   git push origin feature/your-feature-name
   ```

4. **Create pull request** on GitHub:
   - Provide clear description
   - Reference related issues
   - Include test results
   - Note any breaking changes

### Pull Request Checklist

- [ ] Code follows project style guidelines
- [ ] Code has been formatted with clang-format
- [ ] Tests added for new functionality
- [ ] All tests pass (`make test`)
- [ ] Documentation updated
- [ ] Commit messages are clear and descriptive
- [ ] No compiler warnings
- [ ] Memory safety verified (valgrind clean)
- [ ] Ran `make check` before committing

---

## Where to Ask Questions

We welcome questions! Here's where to ask depending on your needs:

### GitHub Issues

Use [GitHub Issues](https://github.com/jcaldwell-labs/smartterm-prototype/issues) for:

- **Bug reports** - Something isn't working
- **Feature requests** - Ideas for new functionality
- **Technical problems** - Build failures, installation issues

**Before opening an issue**:

1. Search existing issues to avoid duplicates
2. Use the appropriate issue template
3. Provide clear reproduction steps for bugs

### GitHub Discussions

Use [GitHub Discussions](https://github.com/jcaldwell-labs/smartterm-prototype/discussions) for:

- **How-to questions** - "How do I configure X?"
- **General discussion** - Ideas, use cases, feedback
- **Show and tell** - Share your cool cc-bash setup
- **Q&A** - Questions about architecture, design decisions

Discussions are better for:

- Open-ended questions
- Ideas that aren't fully formed yet
- Community conversation
- Getting to know other users

### Documentation

Before asking, check:

- **[README.md](README.md)** - Usage, installation, features
- **[docs/SUPPORT.md](docs/SUPPORT.md)** - Common issues and solutions
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - Technical design
- **[docs/CLAUDE.md](docs/CLAUDE.md)** - AI assistant guide (for maintainers)

### Response Times

This is a community-driven project with volunteer maintainers:

- **Critical bugs**: Aim for 48 hours
- **General issues**: Usually within 1 week
- **Discussions**: Varies by topic

Please be patient and respectful. If you don't get a response, bump the thread after a week.

### Direct Contact

For private matters (security issues, code of conduct violations), contact the maintainers via:

- Email: Open an issue and we'll provide contact info
- Or create a private security advisory on GitHub

---

## Issue Guidelines

### Reporting Bugs

When reporting bugs, include:

- SmartTerm version
- Operating system and version
- Terminal emulator
- Steps to reproduce
- Expected behavior
- Actual behavior
- Minimal code example if applicable

### Requesting Features

For feature requests, provide:

- Clear use case
- Proposed API (if applicable)
- Why it benefits the project
- Willingness to implement (if you can contribute)

### Security Issues

For security vulnerabilities:

- **Do not** open a public issue
- Email the maintainer directly
- Provide detailed description
- Allow time for fix before disclosure

---

## Development Guidelines

### API Design Principles

- **Simplicity**: Keep API minimal and focused
- **Consistency**: Follow established patterns
- **Safety**: Check parameters, handle errors gracefully
- **Documentation**: Document all public functions thoroughly

### Performance Considerations

- Avoid allocations in hot paths where possible
- Consider memory fragmentation with frequent allocations
- Profile before optimizing
- Don't sacrifice clarity for minor performance gains

### Thread Safety

- Document thread-safety guarantees clearly
- Use appropriate synchronization primitives
- Avoid deadlocks (lock ordering, timeouts)
- Test multi-threaded scenarios

### Error Handling

- Return error codes (negative values or NULL)
- Set errno when appropriate
- Log errors to stderr for debugging
- Don't crash on user errors

---

## Building Documentation

SmartTerm's documentation is in Markdown:

```bash
# API reference
docs/SMARTTERM-API.md

# Architecture
docs/ARCHITECTURE.md

# User guide
README.md
```

After making changes:

- Update relevant documentation
- Keep examples in sync with code
- Check markdown syntax
- Update version numbers if releasing

---

## Release Process

(For maintainers)

1. Update CHANGELOG.md with version and changes
2. Update version in include/smartterm.h
3. Tag release: `git tag -a v1.x.x -m "Version 1.x.x"`
4. Push tag: `git push upstream --tags`
5. Create GitHub release with notes

---

## Getting Help

- **Documentation**: Check docs/ directory
- **Examples**: See examples/ for working code
- **Issues**: Search existing issues on GitHub
- **Questions**: Open a discussion issue

---

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

## Recognition

Contributors are recognized in the project README and release notes. Thank you for helping make SmartTerm better!
