# CLAUDE.md - AI Assistant Guide for cc-bash

> **Purpose**: This document provides AI assistants with comprehensive context about the cc-bash project, its architecture, development workflows, and conventions.

**Last Updated**: 2026-01-07
**Project Status**: Active Development

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Codebase Structure](#codebase-structure)
3. [Architecture & Design](#architecture--design)
4. [Development Workflows](#development-workflows)
5. [Key Conventions](#key-conventions)
6. [Common Tasks](#common-tasks)
7. [Project History](#project-history)
8. [References](#references)

---

## Project Overview

### What is cc-bash?

cc-bash is a **Claude Code-style bash wrapper** that provides a structured terminal interface for command execution:

- Commands execute in bash by default (no prefix needed)
- Colored output: commands (cyan), stdout (white), stderr (red)
- Fixed status bar showing cwd, exit code, and time
- PTY-based execution for automatic color support
- Command history with persistence
- Tab completion for commands and file paths
- Ctrl+R fuzzy history search
- Extensibility via aliases, snippets, workflows, plugins, and themes

### Two Versions

| Version          | Language | AI Integration   | Dependencies                           |
| ---------------- | -------- | ---------------- | -------------------------------------- |
| `cc-bash`        | C        | No               | libutil (forkpty)                      |
| `cc-bash-sdk.py` | Python   | Yes (Claude SDK) | claude-agent-sdk, prompt_toolkit, rich |

### Current Status

- **C version**: ~3300 LOC, feature-complete
- **Python version**: Claude AI integration for @ask, @explain, @fix, @cmd
- **Test suite**: 170 unit tests + static analysis
- **CI/CD**: GitHub Actions for build, test, and release

---

## Codebase Structure

### File Tree

```
smartterm-prototype/
├── cc-bash.c              # Main C implementation (~3300 LOC)
├── cc-bash-sdk.py         # Python + Claude SDK version
├── cc-bashrc.template     # Sample configuration file
├── Makefile               # Build system
├── install.sh             # Installation script
├── smartterm_poc.c        # Original POC (legacy, ~250 LOC)
│
├── tests/
│   ├── test_unit.c        # Unit tests (170 tests)
│   ├── test_framework.h   # Test framework header
│   ├── framework.c        # Test framework implementation
│   └── test_cc_bash.sh    # Static analysis tests
│
├── docs/
│   ├── README.md          # Docs index
│   ├── ARCHITECTURE.md    # Architecture documentation
│   ├── SMARTTERM-API.md   # Legacy library API reference
│   ├── guides/            # User guides
│   ├── tutorials/         # Tutorials
│   └── examples/          # Doc examples
│
├── lib/                   # Legacy smartterm library (v1.0)
│   └── smartterm/         # Modular library implementation
│       ├── smartterm_core.c
│       ├── smartterm_output.c
│       ├── smartterm_input.c
│       ├── smartterm_render.c
│       ├── smartterm_theme.c
│       ├── smartterm_status.c
│       ├── smartterm_scroll.c
│       ├── smartterm_search.c
│       ├── smartterm_export.c
│       ├── smartterm_keyhandler.c
│       └── smartterm_internal.h
│
├── include/
│   └── smartterm.h        # Legacy library public API
│
├── examples/              # Legacy library examples
│   ├── repl.c
│   ├── chat_client.c
│   ├── log_viewer.c
│   └── headless_demo.c
│
├── .github/
│   └── workflows/
│       ├── ci.yml         # CI pipeline
│       └── release.yml    # Release automation
│
├── README.md              # User-facing documentation
├── CONTRIBUTING.md        # Contribution guidelines
├── CHANGELOG.md           # Version history
├── SECURITY.md            # Security policy
├── VISION.md              # Project vision
└── llms.txt               # LLM context file
```

### Key Files Explained

#### `cc-bash.c` (~3300 lines)

**Purpose**: Main C implementation of the Claude Code-style bash wrapper

**Key Sections**:

- **Lines 1-100**: Header comments, includes, ANSI color definitions
- **Lines 100-200**: Theme configuration structures
- **Lines 200-400**: Configuration parsing (aliases, snippets, workflows, plugins)
- **Lines 400-600**: History management and fuzzy search
- **Lines 600-1000**: Input handling (raw terminal mode, key processing)
- **Lines 1000-1500**: Tab completion
- **Lines 1500-2000**: Command execution (PTY-based with forkpty)
- **Lines 2000-2500**: Built-in commands (@help, @clear, @edit, @reload, etc.)
- **Lines 2500-3000**: Plugin system and hooks
- **Lines 3000-3300**: Main loop and initialization

**Key Functions**:

- `execute_command()` - PTY-based command execution with color support
- `read_input()` - Raw terminal input with line editing
- `handle_key()` - Key event processing
- `fuzzy_search_history()` - Ctrl+R implementation
- `expand_alias()` - Alias expansion
- `run_workflow()` - Multi-step workflow execution
- `load_config()` - Parse ~/.cc-bashrc
- `load_plugins()` - Load ~/.cc-bash/plugins/

#### `cc-bash-sdk.py` (~300 lines)

**Purpose**: Python version with Claude AI integration

**AI Commands**:

- `@ask <question>` - Ask Claude anything
- `@explain` - Explain last command output
- `@fix` - Suggest fix for last error
- `@cmd <description>` - Generate command from natural language

#### `Makefile`

**Targets**:

- `make` / `make all` - Build cc-bash
- `make run` - Build and run
- `make test` - Run unit tests + static analysis
- `make test-unit` - Run only unit tests
- `make install` - Install to /usr/local/bin
- `make uninstall` - Remove installation
- `make poc` - Build original smartterm POC
- `make clean` - Remove binaries
- `make help` - Show help

---

## Architecture & Design

### Three-Region Layout

```
┌──────────────────────────────────────────────────────────────────────┐
│                        Output Area (scrolling)                        │
│  $ ls -la                                                            │
│  total 24                                                            │
│  drwxr-xr-x 3 user user 4096 Jan 7 .                                │
│  -rw-r--r-- 1 user user 1234 Jan 7 main.c                           │
│  ...                                                                 │
├──────────────────────────────────────────────────────────────────────┤
│ $ _                                                    (prompt area) │
├──────────────────────────────────────────────────────────────────────┤
│  ~/projects/myapp                              [exit: 0] 14:30:00   │
│ ──────────────────────────────────────────────────────────────────── │
└──────────────────────────────────────────────────────────────────────┘
   └── Status bar (fixed, shows cwd, exit code, time)
```

### Key Design Decisions

1. **ANSI Escape Codes (Not ncurses)**
   - Direct ANSI sequences for portability
   - No library dependencies for terminal control
   - Output stays visible at all times
   - Why not ncurses: must suspend during readline, causing flicker

2. **PTY-Based Command Execution** (Issue #22)
   - Uses `forkpty()` for pseudo-terminal allocation
   - Commands see a real terminal (isatty() returns true)
   - Automatic color output from ls, grep, etc.
   - Proper signal handling for Ctrl+C

3. **Raw Terminal Mode**
   - Character-by-character input via termios
   - Custom line editing (no readline dependency)
   - Full control over key handling
   - Enables Ctrl+R fuzzy search

4. **Configuration System**
   - `~/.cc-bashrc` for user configuration
   - Aliases, snippets, workflows, themes
   - Plugin system with hooks
   - Runtime reload with @reload

### Data Flow

```
User Input → Raw Terminal → Key Handler → Command Parser
                                              ↓
                    ┌─────────────────────────┴──────────────────────────┐
                    ↓                         ↓                          ↓
              @ Command               # Comment                    Shell Command
              (internal)              (display only)               (execute)
                    ↓                         ↓                          ↓
              Handle builtin           Display in yellow          PTY execution
                    ↓                         ↓                          ↓
              Update state            Add to history              Capture output
                    ↓                         ↓                          ↓
                    └─────────────────────────┴──────────────────────────┘
                                              ↓
                                    Render output + status bar
```

### Special Prefixes

| Prefix | Type             | Behavior                        |
| ------ | ---------------- | ------------------------------- |
| (none) | Shell command    | Execute in bash                 |
| `#`    | Comment/note     | Display in yellow, not executed |
| `@`    | Internal command | Handle as builtin               |

### Internal @ Commands

| Command       | Shortcut | Description                    |
| ------------- | -------- | ------------------------------ |
| `@help`       | `@h`     | Show help                      |
| `@clear`      | `@c`     | Clear screen                   |
| `@quit`       | `@q`     | Exit cc-bash                   |
| `@edit`       | `@e`     | Edit config in $EDITOR         |
| `@reload`     | `@r`     | Reload configuration           |
| `@alias`      | -        | List/add aliases               |
| `@alias save` | -        | Save session aliases to config |
| `@snippet`    | -        | List/run snippets              |
| `@workflow`   | -        | List/run workflows             |
| `@theme`      | -        | Display current theme          |
| `@hooks`      | -        | List registered hooks          |
| `@plugins`    | -        | List loaded plugins            |

### Keyboard Shortcuts

| Key          | Action                           |
| ------------ | -------------------------------- |
| `Ctrl+R`     | Fuzzy history search             |
| `Ctrl+C`     | Cancel input / interrupt command |
| `Ctrl+D`     | Exit (on empty line)             |
| `Up/Down`    | Navigate history                 |
| `Left/Right` | Move cursor                      |
| `Tab`        | Command/file completion          |
| `PgUp/PgDn`  | Scroll output buffer             |
| `Esc`        | Cancel search mode               |

---

## Development Workflows

### Building

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential

# Build cc-bash
make

# Build and run
make run

# Build original POC (requires ncurses + readline)
sudo apt-get install libncurses-dev libreadline-dev
make poc
```

### Testing

```bash
# Run all tests (unit + static analysis)
make test

# Run only unit tests (170 tests)
make test-unit

# View test output
./test_unit
```

### Installation

```bash
# System-wide install
sudo make install

# User-local install (no sudo)
make install PREFIX=$HOME/.local

# Uninstall
sudo make uninstall
```

### Git Workflow

**Branch naming**:

- Feature branches: `feature/<description>`
- Bug fixes: `fix/<description>`
- Claude sessions: `claude/<session-id>`

**Commit conventions**:

- Use conventional commits: `feat:`, `fix:`, `docs:`, `refactor:`, `test:`
- Focus on "why" rather than "what"
- Reference issues: `(#123)`

---

## Key Conventions

### Code Style (C)

1. **Naming**:
   - `snake_case` for functions and variables
   - `PascalCase` for structs and typedefs
   - `ALL_CAPS` for constants and defines

2. **Formatting**:
   - 4-space indentation
   - K&R brace style
   - Compiler flags: `-Wall -Wextra -std=c11`
   - Use `.clang-format` for consistency

3. **Comments**:
   - File header with purpose and architecture overview
   - Section headers with `/* ====== */` dividers
   - Inline comments for non-obvious logic

4. **Error Handling**:
   - Check return values of system calls
   - Use `perror()` for system errors
   - Graceful degradation where possible

### Configuration Format (~/.cc-bashrc)

```bash
# Aliases (command shortcuts)
alias ll='ls -la'
alias gs='git status'

# Snippets (parameterized templates)
snippet find-name='find . -name "$1"'
snippet grep-r='grep -r "$1" .'

# Workflows (multi-step sequences)
workflow build='make clean && make && make test'

# Theme colors
theme.prompt=cyan
theme.error=red
theme.comment=yellow

# Environment
export EDITOR=vim
```

### Plugin Structure

```
~/.cc-bash/plugins/<name>/
├── plugin.conf     # Required: manifest
├── config.conf     # Optional: aliases/snippets/workflows
└── hooks/          # Optional: event scripts
    ├── on_startup.sh
    ├── on_cd.sh
    └── on_post_command.sh
```

---

## Common Tasks

### Adding a New @ Command

1. Add command name to help text in `show_help()`
2. Add handler in the command dispatch section (~line 2200)
3. Implement the handler function
4. Add test case in `tests/test_unit.c`

### Adding a New Hook Event

1. Define event type in the hooks enum
2. Add parsing in `parse_hook_event()`
3. Call `trigger_hook()` at appropriate point
4. Document in README.md

### Modifying Theme System

1. Theme struct defined around line 100
2. Color parsing in `parse_color_spec()`
3. Default values in `init_theme()`
4. Loading in `load_config()` → `parse_theme_line()`

### Debugging Tips

```bash
# Run with strace to debug system calls
strace -f ./cc-bash 2>&1 | grep -E '(exec|fork|pty)'

# Check terminal capabilities
tput colors
echo $TERM

# Test PTY allocation
./cc-bash -c "ls --color=auto"  # Should show colors
```

---

## Project History

### Evolution Timeline

| Date       | Version | Milestone                                     |
| ---------- | ------- | --------------------------------------------- |
| 2025-11-17 | v0.1.0  | POC: ncurses + readline integration validated |
| 2025-11-17 | v1.0.0  | libsmartterm extracted as full library        |
| 2025-12    | -       | Pivot: Repurposed to cc-bash (ANSI-based)     |
| 2025-12    | -       | Added aliases, snippets, workflows            |
| 2025-12    | -       | Added plugin system and themes                |
| 2026-01    | -       | PTY-based execution (Issue #22)               |
| 2026-01    | -       | Ctrl+R fuzzy history search (Issue #23)       |
| 2026-01    | -       | @edit, @reload, @alias save (Issue #24)       |

### Why the Pivot?

The original POC used ncurses + readline. While the concept worked, the UX suffered:

1. **ncurses must suspend during readline input** - This causes output to disappear while typing
2. **Visible flicker** - The suspend/resume cycle is noticeable
3. **Complexity** - Managing two terminal libraries is error-prone

The ANSI-based approach:

- Keeps all output visible at all times
- Uses raw termios for input (no readline dependency)
- Simpler, more portable code
- Better user experience

### Legacy Components

The following are retained for reference but not actively maintained:

- `smartterm_poc.c` - Original ncurses POC
- `lib/smartterm/` - Full library implementation
- `include/smartterm.h` - Library API
- `examples/` - Library examples
- `docs/SMARTTERM-API.md` - Library API reference

---

## References

### Key Documentation

| File                   | Description               |
| ---------------------- | ------------------------- |
| `README.md`            | User-facing documentation |
| `CONTRIBUTING.md`      | Contribution guidelines   |
| `CHANGELOG.md`         | Version history           |
| `docs/ARCHITECTURE.md` | Detailed architecture     |
| `llms.txt`             | LLM context file          |

### External Resources

**Terminal Programming**:

- ANSI escape codes: https://en.wikipedia.org/wiki/ANSI_escape_code
- termios: `man termios`, `man tcsetattr`
- PTY: `man pty`, `man forkpty`

**Build System**:

- GNU Make: `man make`
- GCC: `man gcc`

### Code Patterns

**PTY-based Command Execution** (cc-bash.c):

```c
pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);
if (pid == 0) {
    // Child: execute command
    execl("/bin/sh", "sh", "-c", command, NULL);
    _exit(127);
}
// Parent: read output from master_fd
while ((n = read(master_fd, buf, sizeof(buf))) > 0) {
    write(STDOUT_FILENO, buf, n);
}
waitpid(pid, &status, 0);
```

**Raw Terminal Mode** (cc-bash.c):

```c
struct termios raw;
tcgetattr(STDIN_FILENO, &orig_termios);
raw = orig_termios;
raw.c_lflag &= ~(ECHO | ICANON | ISIG);
raw.c_cc[VMIN] = 1;
raw.c_cc[VTIME] = 0;
tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
```

**Fuzzy History Search** (cc-bash.c):

```c
// Ctrl+R triggers search mode
// As user types, filter history entries
// Display matches with highlighting
// Enter selects, Esc cancels
```

---

## For AI Assistants

### Best Practices

1. **Read cc-bash.c section headers** - The file is well-organized with clear sections
2. **Run tests after changes** - `make test` catches regressions
3. **Check README.md** - Keep user docs in sync with code
4. **Follow conventional commits** - Makes CHANGELOG generation easier
5. **Don't modify legacy lib/** - It's retained for reference only

### When Adding Features

1. Check if similar functionality exists
2. Follow existing patterns in the codebase
3. Add unit tests in `tests/test_unit.c`
4. Update `@help` output
5. Document in README.md if user-facing

### When Fixing Bugs

1. Write a failing test first
2. Fix the bug
3. Verify test passes
4. Check for similar issues elsewhere

### Key Areas to Understand

- **Input handling**: ~lines 600-1000 in cc-bash.c
- **Command execution**: ~lines 1500-2000
- **Configuration**: ~lines 200-400
- **Plugin system**: ~lines 2500-3000

---

**End of CLAUDE.md**

_This document should be updated when significant changes are made to the project._
