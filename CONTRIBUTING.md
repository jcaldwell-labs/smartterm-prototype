# Contributing to cc-bash

Thank you for your interest in contributing to cc-bash! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Quick Start: Quality Workflow](#quick-start-quality-workflow)
- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Quality Checks](#quality-checks)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Issue Guidelines](#issue-guidelines)

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

## Code of Conduct

This project follows standard open source collaboration practices:

- Be respectful and professional
- Focus on technical merit
- Provide constructive feedback
- Help maintain a welcoming environment

---

## Getting Started

### Prerequisites

- C compiler (gcc or clang)
- ncurses development library
- readline development library
- Make build system
- Git

**Ubuntu/Debian:**

```bash
sudo apt-get install build-essential libncurses-dev libreadline-dev git
```

**macOS:**

```bash
brew install ncurses readline make
```

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
- [ ] All tests pass
- [ ] Documentation updated
- [ ] Commit messages are clear and descriptive
- [ ] No compiler warnings
- [ ] Memory safety verified (valgrind clean)

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
