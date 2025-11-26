# GitHub Copilot Instructions

This file provides guidance to GitHub Copilot when working with code in this repository.

## Project Overview

SmartTerm is a production-ready C library for terminal UIs with scrolling output, readline integration, context-aware coloring, status bar, search, and export features. Originally developed as a proof of concept, it has been extracted into a modular library.

**Status**: POC Complete, Library Extracted
**Language**: C (C11 standard)
**Dependencies**: ncurses, readline, pthread (optional)

Key features:
- Scrolling output buffer (configurable size)
- Context-aware message coloring (5 context types)
- Fixed status bar with left/right alignment
- Readline integration with history
- Export to multiple formats (plain text, ANSI, Markdown, HTML)
- Search and filter functionality
- Tab completion framework

## Build System

The project has two Makefiles - one for the original POC and one for the full library:

### POC Build (Makefile)
```bash
make              # Build POC binary
make run          # Build and run POC
make clean        # Remove artifacts
make test         # Show testing instructions
make help         # Show available targets
```

### Library Build (Makefile.lib)
```bash
make -f Makefile.lib lib       # Build libsmartterm.a library
make -f Makefile.lib examples  # Build example applications
make -f Makefile.lib tests     # Build test suite
make -f Makefile.lib test      # Build and run all tests
make -f Makefile.lib poc       # Build original POC
make -f Makefile.lib format    # Format code with clang-format
make -f Makefile.lib install   # Install library system-wide
make -f Makefile.lib clean     # Remove build artifacts
make -f Makefile.lib help      # Show all targets
```

**Dependencies:**
- **Required**: GCC, ncurses (`libncurses-dev`), readline (`libreadline-dev`)
- **Optional**: pthread, clang-format

**Compiler Flags**: `-Wall -Wextra -std=c11`

## Architecture

### Directory Structure

```
smartterm-prototype/
├── smartterm_poc.c          # Original POC (~250 LOC)
├── Makefile                 # POC build
├── Makefile.lib             # Library build
├── include/
│   └── smartterm.h          # Public API header
├── lib/smartterm/           # Library implementation (11 modules)
│   ├── smartterm_core.c     # Core initialization
│   ├── smartterm_output.c   # Output buffer
│   ├── smartterm_input.c    # Input handling
│   ├── smartterm_render.c   # Rendering
│   ├── smartterm_theme.c    # Color themes
│   ├── smartterm_status.c   # Status bar
│   ├── smartterm_scroll.c   # Scrollback
│   ├── smartterm_search.c   # Search functionality
│   ├── smartterm_export.c   # Export (plain/ANSI/HTML/Markdown)
│   ├── smartterm_keyhandler.c  # Key binding
│   └── smartterm_internal.h # Internal API
├── examples/                # Example applications
│   ├── repl.c               # Calculator REPL
│   ├── chat_client.c        # IRC-style chat
│   └── log_viewer.c         # Log monitoring
├── tests/                   # Test suite
│   └── test_*.c
└── docs/
    ├── SMARTTERM-API.md     # API reference
    └── ARCHITECTURE.md      # Design documentation
```

### Core Components

**Output Buffer**: Stores output lines without prompt duplication
```c
typedef struct {
    char **lines;        // Array of output lines
    int count;           // Current line count
    int capacity;        // Max lines (configurable)
    int scroll_offset;   // For scrollback
} OutputBuffer;
```

**Context Types**: For color-coded output
```c
typedef enum {
    CTX_NORMAL,    // Default (white)
    CTX_COMMAND,   // ! prefix (yellow)
    CTX_COMMENT,   // # prefix (green)
    CTX_SPECIAL,   // @ prefix (cyan)
    CTX_SEARCH     // / prefix (magenta)
} ContextType;
```

**Context Markers**:
- `>` or no prefix → Normal input (white)
- `!` → System command (yellow)
- `#` → Comment (green)
- `@` → Special action (cyan)
- `/` → Search (magenta)

### Key Design Pattern: ncurses + readline Integration

```c
char* readline_with_ncurses(const char *prompt) {
    // Suspend ncurses
    def_prog_mode();
    endwin();

    // Use readline
    char *input = readline(prompt);

    // Resume ncurses
    reset_prog_mode();
    refresh();

    return input;
}
```

## Code Style and Conventions

- **Standard**: C11
- **Naming**: `snake_case` for functions/variables, `PascalCase` for structs/enums, `ALL_CAPS` for constants
- **Formatting**: 4-space indentation, K&R brace style
- **Memory**: Proper allocation/deallocation patterns
- **Error handling**: Check all I/O operations

**Module Organization**:
- Each module handles one concern
- Public API in `include/smartterm.h`
- Internal API in `lib/smartterm/smartterm_internal.h`

## Before Committing (Required Steps)

Run these commands before every commit:

1. **Build Library**: `make -f Makefile.lib clean && make -f Makefile.lib lib`
2. **Build Examples**: `make -f Makefile.lib examples`
3. **Test**: `make -f Makefile.lib test`
4. **Manual test**: Run an example to verify functionality

```bash
# Quick pre-commit check
make -f Makefile.lib clean && make -f Makefile.lib lib examples test
```

## Common Development Tasks

### Adding a New Context Type
1. Add enum value to `ContextType` in header
2. Add color pair in `init_ui()`
3. Update `render_output()` for new color
4. Document the marker prefix

### Creating a New Export Format
1. Add function in `smartterm_export.c`
2. Add declaration to header
3. Add test case
4. Update documentation

### Adding a New Example
1. Create `examples/my_example.c`
2. Call `init_ui()` on startup
3. Use `add_output_line()` for text
4. Call `cleanup_ui()` on exit
5. Build with `make -f Makefile.lib examples`

## Pull Request Standards

When creating PRs, follow these rules:

1. **Always link the issue**: Use `Fixes #N` or `Closes #N`
2. **Fill in all sections**: Never leave placeholder text

**Required PR format:**
```markdown
## Summary
[2-3 sentences describing what and why]

Fixes #[issue-number]

## Changes
- [Actual change 1]
- [Actual change 2]

## Testing
- [x] Library builds (`make -f Makefile.lib lib`)
- [x] All tests pass (`make -f Makefile.lib test`)
- [x] Examples work (`make -f Makefile.lib examples`)

## Type
- [x] New feature | Bug fix | Refactor | Docs | CI
```

## Known Limitations (POC Scope)

**Intentionally Limited in POC**:
- Tab completion not wired up (readline supports it)
- No persistent history file
- Minimal error handling
- No configurable themes

**Core Concepts Proven**:
- Output buffer without prompt duplication
- Context-aware coloring
- Status bar
- ncurses + readline integration

## Additional Documentation

- **README.md** - POC documentation and testing guide
- **docs/SMARTTERM-API.md** - API reference
- **docs/ARCHITECTURE.md** - Design documentation
- **DECISION.md** - Implementation path decision tracking
- **NEXT-STEPS.md** - Opportunity cost analysis
