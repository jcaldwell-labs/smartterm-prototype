# cc-bash: Claude Code-style Bash Wrapper

**A simple terminal wrapper that executes bash commands by default with colored output**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

---

## Overview

cc-bash provides a Claude Code-inspired interface for interactive bash use:

- Commands execute in bash by default (no `!` prefix needed)
- Colored output: commands (cyan), stdout (white), stderr (red)
- Status bar showing current directory, exit code, and time
- Command history (up/down arrows via readline)
- Notes with `#` prefix (displayed, not executed)
- Internal commands with `@` prefix

---

## Quick Start

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libreadline-dev

# Build
make

# Run
./cc-bash
```

---

## Usage

```
cc-bash: Claude Code-style bash wrapper
Type @help for help, @quit or exit to quit

 ~/projects/myapp                                    [exit: 0] 14:30:00
────────────────────────────────────────────────────────────────────────
$ ls -la
$ ls -la
total 24
drwxr-xr-x 3 user user 4096 Dec 30 14:30 .
-rw-r--r-- 1 user user 1234 Dec 30 14:30 main.c

 ~/projects/myapp                                    [exit: 0] 14:30:05
────────────────────────────────────────────────────────────────────────
$ # This is a note - it won't execute
# This is a note - it won't execute

$ @help
cc-bash: Claude Code-style bash wrapper

Commands are executed in bash by default.

Special prefixes:
  # comment  - Add a note (yellow, not executed)
  @clear     - Clear screen
  @help      - Show this help
  @quit      - Exit cc-bash

Built-in commands:
  cd [path]  - Change directory
  exit       - Exit cc-bash
```

---

## Features

### Command Execution
- All input executes as bash commands by default
- stdout displayed in white/default color
- stderr displayed in red
- Exit codes shown in status bar

### Status Bar
- Current working directory (truncated if long)
- Last command exit code
- Current time
- Reverse video for visibility

### Special Prefixes
| Prefix | Action |
|--------|--------|
| `#` | Note/comment - displayed in yellow, not executed |
| `@` | Internal command (help, clear, quit) |

### Built-in Commands
| Command | Action |
|---------|--------|
| `cd [path]` | Change directory (supports `~` expansion) |
| `exit` | Exit cc-bash |
| `quit` | Exit cc-bash |

### Internal @ Commands
| Command | Action |
|---------|--------|
| `@help` / `@h` | Show help |
| `@clear` / `@c` | Clear screen |
| `@quit` / `@q` | Exit cc-bash |

---

## Building

```bash
# Build cc-bash (default)
make

# Build and run
make run

# Clean
make clean

# Show help
make help
```

---

## Design Philosophy

cc-bash takes a **simple approach** using ANSI escape codes instead of ncurses:

1. **Output stays visible** - No TUI that hides when you type
2. **Readline for input** - Command history, line editing
3. **ANSI colors** - Works in any terminal
4. **Minimal dependencies** - Just readline

### Why Not ncurses?

An earlier POC used ncurses + readline integration. While the concept worked, the UX suffered:
- ncurses must suspend during readline input
- This causes the output area to disappear while typing
- Not the seamless experience we wanted

The current ANSI-based approach keeps all output visible at all times.

---

## Project Structure

```
smartterm-prototype/
├── cc-bash.c            # Main implementation (~350 LOC)
├── Makefile             # Build configuration
├── smartterm_poc.c      # Original ncurses POC (archived)
└── README.md            # This file
```

---

## History

This project evolved from "smartterm-prototype":

1. **Original POC**: Attempted ncurses + readline integration
2. **Problem discovered**: ncurses suspend/resume causes output to disappear during input
3. **Repurposed**: Simplified to cc-bash using ANSI escape codes
4. **Result**: Simple, working Claude Code-style bash wrapper

See GitHub issue #14 for the repurposing discussion.

---

## License

MIT License - See LICENSE file for details

---

## Credits

Inspired by Claude Code's terminal interface.
