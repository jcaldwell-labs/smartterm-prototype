# Changelog

All notable changes to SmartTerm will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.2.1] - 2026-01-24

### Added

- Multi-line input support with continuation detection (#25, #27)
  - `needs_continuation()` detects trailing `\`, unclosed quotes (`'`, `"`, `` ` ``)
  - Continuation prompt (`"> "`) shown when more input needed
  - History load/save handles embedded newlines via `\x00` escaping
- Quality check workflow for development
  - `make lint` for cppcheck static analysis
  - `make check` runs lint + all tests
  - `make STRICT=1` for `-Werror` builds
  - Pre-commit hook (`.githooks/pre-commit`) blocks commits on failures
- 39 new unit tests for continuation detection (240 total tests)
- Automated release workflow via GitHub Actions

### Fixed

- macOS compatibility: Added `strcasestr` fallback for non-glibc systems
- Package sha256 checksums for v1.2.0 binaries

### Changed

- Stricter compiler warnings: `-Wpedantic -Wunused -Wshadow -Wformat=2`

## [1.2.0] - 2026-01-08

### Added

- Homebrew formula (`Formula/cc-bash.rb`) for macOS/Linux installation
- AUR PKGBUILD (`packaging/arch/PKGBUILD`) for Arch Linux
- Debian packaging files (`packaging/debian/`) for .deb builds
- Updated README with package manager installation options
- Updated CLAUDE.md to reflect current cc-bash project state
- Updated VISION.md roadmap with completed milestones

### Packaging

- Homebrew: `brew tap jcaldwell-labs/cc-bash && brew install cc-bash`
- AUR: `yay -S cc-bash` (pending AUR submission)
- Debian: Build with `dpkg-buildpackage` (packaging files included)

## [1.1.0] - 2026-01-07

### Added

- PTY-based command execution for automatic color support (#22)
- Ctrl+R fuzzy history search (#23)
- Persistent config commands: @edit, @reload, @alias save (#24)
- Editor compatibility fixes (#26)

### Changed

- Commands now run in pseudo-terminal, enabling colored output from ls, grep, etc.

## [1.0.0] - 2025-11-17

### Added

- Production-ready library API extracted from POC
- Thread-safe output buffer implementation
- Context-aware message coloring system
- Readline integration for input handling
- Fixed status bar with customizable content
- Search functionality with highlighting
- Export to multiple formats (plain text, ANSI, Markdown, HTML)
- Scrollback navigation with PgUp/PgDn
- Theme system with built-in and custom themes
- Custom key handler support
- Terminal resize handling
- Multi-line input support (basic)
- Tab completion framework

### Library Structure

- `include/smartterm.h` - Public API header
- `lib/smartterm/` - Modular implementation:
  - `smartterm_core.c` - Core initialization and context management
  - `smartterm_output.c` - Output buffer management
  - `smartterm_input.c` - Input handling with readline
  - `smartterm_render.c` - Display rendering
  - `smartterm_theme.c` - Theme management
  - `smartterm_status.c` - Status bar functionality
  - `smartterm_scroll.c` - Scrollback navigation
  - `smartterm_search.c` - Buffer search
  - `smartterm_export.c` - Export functionality
  - `smartterm_keyhandler.c` - Custom key handling
  - `smartterm_internal.h` - Internal API

### Examples

- `examples/repl.c` - Calculator REPL demonstrating basic usage
- `examples/chat_client.c` - IRC-style chat showing thread-safe output
- `examples/log_viewer.c` - Log monitoring with search and export

### Documentation

- `docs/SMARTTERM-API.md` - Comprehensive API reference
- `docs/ARCHITECTURE.md` - Architecture and design decisions
- Updated README with features, installation, and quick start

### Build System

- `Makefile.lib` - Library and examples build system
- Support for building static library (`libsmartterm.a`)
- Install target for system-wide installation
- Clean separation between library, examples, and POC

## [0.1.0] - 2025-11-17

### Added - POC Phase

- Initial proof of concept (`smartterm_poc.c`, ~250 LOC)
- Validated core concepts:
  - Scrolling output buffer without prompt duplication
  - ncurses + readline integration
  - Context-aware coloring (5 context types)
  - Fixed status bar
  - Three-region terminal layout

### Documentation - POC Phase

- README with POC testing guide
- DECISION.md documenting development path choices
- NEXT-STEPS.md with opportunity cost analysis
- SESSION-SUMMARY.md with findings and learnings
- CLAUDE.md with AI assistant guide

### Build System - POC Phase

- Simple Makefile for POC
- Build targets: all, run, clean, test, help

### Decision Framework

- Analyzed 5 implementation paths (A through E)
- Chose Path B→A: Ship first, validate, then refactor if needed
- Documented opportunity costs and break-even analysis
- Decided to build full library based on validation

## Key Design Decisions

### Architecture

- Output buffer separate from display (enables search, export, scrollback)
- ncurses suspended during readline input (clean integration)
- Context stored as metadata with output lines
- Status bar as separate ncurses window
- Thread-safe operations with mutex locking

### API Design

- Simple initialization with configuration struct
- Opaque context handle for encapsulation
- Formatted output functions (printf-style)
- Clean resource management (init/cleanup)
- Extensible context types for user customization

### Implementation Choices

- C11 standard for portability
- Static library for easy integration
- Minimal dependencies (ncurses, readline, pthread)
- POC retained for reference and testing

---

## Development Philosophy

SmartTerm follows a POC-first approach:

1. Build minimal proof of concept
2. Validate architecture and feasibility
3. Analyze opportunity costs
4. Make informed decision
5. Ship fast, iterate based on real usage

This approach:

- Reduces upfront investment
- Validates concepts before full implementation
- Maintains development momentum
- Prevents over-engineering
- Ensures library meets real needs

---

## Version History Summary

- **v1.0.0** (2025-11-17): Production library with full feature set
- **v0.1.0** (2025-11-17): Proof of concept validating core concepts

---

## Upgrade Guide

### From POC to v1.0.0

The library API is significantly different from the POC. Key changes:

**Initialization:**

```c
// POC
init_ui();

// v1.0.0
smartterm_ctx *ctx = smartterm_init(NULL);
```

**Output:**

```c
// POC
add_output_line("message", CTX_NORMAL);

// v1.0.0
smartterm_write(ctx, "message", CTX_NORMAL);
```

**Cleanup:**

```c
// POC
cleanup_ui();

// v1.0.0
smartterm_cleanup(ctx);
```

See `docs/SMARTTERM-API.md` for complete migration guide.

---

## Links

- [Repository](https://github.com/jcaldwell-labs/smartterm-prototype)
- [API Documentation](docs/SMARTTERM-API.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Contributing](CONTRIBUTING.md)

---

_This changelog follows the [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) format._
