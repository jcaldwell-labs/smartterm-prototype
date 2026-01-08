# cc-bash: Claude Code-style Bash Wrapper

**A simple terminal wrapper that executes bash commands by default with colored output**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.cppreference.com/w/c/11)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

---

## Two Versions Available

| Version          | Language | AI Integration   | Dependencies                           |
| ---------------- | -------- | ---------------- | -------------------------------------- |
| `cc-bash`        | C        | No               | libreadline                            |
| `cc-bash-sdk.py` | Python   | Yes (Claude SDK) | claude-agent-sdk, prompt_toolkit, rich |

---

## Overview

cc-bash provides a Claude Code-inspired interface for interactive bash use:

- Commands execute in bash by default (no `!` prefix needed)
- Colored output: commands (cyan), stdout (white), stderr (red)
- Status bar showing current directory, exit code, and time
- Command history with persistence (~/.cc-bash-history)
- Tab completion for commands and file paths
- **Aliases** - command shortcuts (`alias ll='ls -la'`)
- **Snippets** - parameterized templates (`snippet greet='echo Hello, $1!'`)
- **Workflows** - multi-step command sequences (`workflow build='make && make test'`)
- **Plugins** - extensible via shell-script hooks
- **Themes** - customizable colors
- Notes with `#` prefix (displayed, not executed)
- Internal commands with `@` prefix

---

## Installation

### Option 1: Homebrew (macOS/Linux)

```bash
# Add the tap and install
brew tap jcaldwell-labs/cc-bash
brew install cc-bash

# Copy sample config
cp $(brew --prefix)/share/cc-bash/cc-bashrc.template ~/.cc-bashrc
mkdir -p ~/.cc-bash/plugins
```

### Option 2: AUR (Arch Linux)

```bash
# Using yay
yay -S cc-bash

# Or using paru
paru -S cc-bash

# Setup
cp /usr/share/cc-bash/cc-bashrc.template ~/.cc-bashrc
mkdir -p ~/.cc-bash/plugins
```

### Option 3: Install Script

```bash
# One-command install (builds from source)
curl -fsSL https://raw.githubusercontent.com/jcaldwell-labs/smartterm-prototype/master/install.sh | bash
```

### Option 4: Download Pre-built Binary

Download from [GitHub Releases](https://github.com/jcaldwell-labs/smartterm-prototype/releases):

```bash
# Linux x86_64
curl -fsSL https://github.com/jcaldwell-labs/smartterm-prototype/releases/latest/download/cc-bash-linux-x86_64 -o cc-bash
chmod +x cc-bash
sudo mv cc-bash /usr/local/bin/

# Create config
mkdir -p ~/.cc-bash/plugins
curl -fsSL https://raw.githubusercontent.com/jcaldwell-labs/smartterm-prototype/master/cc-bashrc.template -o ~/.cc-bashrc
```

### Option 5: Build from Source

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential

# Clone and build
git clone https://github.com/jcaldwell-labs/smartterm-prototype.git
cd smartterm-prototype
make

# Install system-wide
sudo make install

# Or run locally
./cc-bash
```

### Option 6: Local User Install (no sudo)

```bash
# Install to ~/.local/bin
make
make install PREFIX=$HOME/.local

# Add to PATH (add to ~/.bashrc)
export PATH="$HOME/.local/bin:$PATH"
```

---

## Quick Start (Python + AI Version)

```bash
# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt

# Run
python cc-bash-sdk.py
```

### AI Commands

The Python version includes Claude AI integration:

| Command              | Description                                  |
| -------------------- | -------------------------------------------- |
| `@ask <question>`    | Ask Claude anything                          |
| `@explain`           | Have Claude explain the last command output  |
| `@fix`               | Have Claude suggest a fix for the last error |
| `@cmd <description>` | Generate a command from natural language     |

### Example AI Session

```
› ls -la
$ ls -la
total 24
drwxr-xr-x 3 user user 4096 Dec 30 .
-rw-r--r-- 1 user user 1234 Dec 30 main.c

› @explain
Asking Claude to explain...
╭─ Explanation ─────────────────────────────────────────╮
│ The `ls -la` command lists all files including       │
│ hidden ones, showing permissions, owner, size...     │
╰───────────────────────────────────────────────────────╯

› @cmd find files larger than 100MB
Generating command...
Suggested: find . -size +100M -type f

› some-command --wrong-flag
error: unknown flag --wrong-flag

› @fix
Asking Claude for fix...
╭─ Suggested Fix ───────────────────────────────────────╮
│ The flag --wrong-flag doesn't exist. Try using...    │
╰───────────────────────────────────────────────────────╯
```

---

## Usage (C Version)

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
- **Full color support** - PTY-based execution means `ls`, `grep`, `bat` show colors automatically
- stdout displayed in white/default color
- stderr displayed in red
- Exit codes shown in status bar

### Status Bar

- Current working directory (truncated if long)
- Last command exit code
- Current time
- Reverse video for visibility

### Special Prefixes

| Prefix | Action                                           |
| ------ | ------------------------------------------------ |
| `#`    | Note/comment - displayed in yellow, not executed |
| `@`    | Internal command (help, clear, quit)             |

### Keyboard Shortcuts

| Key          | Action                                              |
| ------------ | --------------------------------------------------- |
| `Ctrl+R`     | Fuzzy history search (type to search, Enter to use) |
| `Ctrl+C`     | Cancel current input or search                      |
| `Ctrl+D`     | Exit cc-bash                                        |
| `Up/Down`    | Navigate command history                            |
| `Left/Right` | Move cursor in input                                |
| `Tab`        | Complete command/file (double-Tab for options)      |
| `PgUp/PgDn`  | Scroll output buffer                                |
| `Esc`        | Cancel search mode                                  |

### Built-in Commands

| Command     | Action                                    |
| ----------- | ----------------------------------------- |
| `cd [path]` | Change directory (supports `~` expansion) |
| `exit`      | Exit cc-bash                              |
| `quit`      | Exit cc-bash                              |

### Internal @ Commands

| Command                        | Action                                     |
| ------------------------------ | ------------------------------------------ |
| `@help` / `@h`                 | Show help                                  |
| `@clear` / `@c`                | Clear screen                               |
| `@quit` / `@q`                 | Exit cc-bash                               |
| `@edit` / `@e`                 | Edit config file (~/.cc-bashrc) in $EDITOR |
| `@reload` / `@r`               | Reload config (aliases, snippets, etc.)    |
| `@alias`                       | List aliases (\* = session only)           |
| `@alias name=cmd`              | Add session alias                          |
| `@alias save`                  | Save session aliases to config file        |
| `@snippet [name args...]`      | List snippets or run one with arguments    |
| `@workflow [name] [--dry-run]` | List workflows, run one, or preview        |
| `@theme`                       | Display current theme colors               |
| `@hooks`                       | List registered event hooks                |
| `@plugins`                     | List loaded plugins                        |

---

## Configuration

cc-bash uses `~/.cc-bashrc` for configuration. Create it manually or copy from `cc-bashrc.template`.

You can edit the config file directly with `@edit`, then apply changes with `@reload`.

### Aliases

```bash
# Define command shortcuts
alias ll='ls -la'
alias gs='git status'
alias gd='git diff'
```

Usage: Type `ll` and it expands to `ls -la`

### Snippets

```bash
# Define parameterized templates ($1, $2, etc.)
snippet find-name='find . -name "$1"'
snippet grep-r='grep -r "$1" .'
snippet mkdir-cd='mkdir -p $1 && cd $1'
```

Usage: `@snippet find-name "*.c"` → `find . -name "*.c"`

### Workflows

```bash
# Multi-step command sequences
workflow build='make clean && make && make test'
workflow gitcheck='git status; git diff --stat'
```

- Use `&&` to stop on first error
- Use `;` to continue regardless of errors

Usage:

- `@workflow` - list all workflows
- `@workflow build` - run the build workflow
- `@workflow build --dry-run` - preview without executing

### Themes

```bash
# Customize colors (black, red, green, yellow, blue, magenta, cyan, white)
# Modifiers: bold, dim
theme.prompt=bold cyan
theme.error=bold red
theme.comment=green
theme.dim=dim
theme.header=bold white
theme.status=bold
theme.scroll=dim
```

### Environment Variables

```bash
export EDITOR=vim
export PAGER=less
```

---

## Plugins

Plugins extend cc-bash with custom hooks, commands, aliases, and workflows.

### Plugin Location

```
~/.cc-bash/plugins/<plugin-name>/
├── plugin.conf     # Required: plugin manifest
├── config.conf     # Optional: aliases/snippets/workflows
└── hooks/          # Optional: event hook scripts
    ├── on_startup.sh
    ├── on_cd.sh
    └── on_post_command.sh
```

### Plugin Manifest (plugin.conf)

```ini
name=my-plugin
version=1.0
description=My custom plugin

# Event hooks (shell scripts)
hook.startup=hooks/on_startup.sh
hook.cd=hooks/on_cd.sh
hook.post_command=hooks/on_post_command.sh
```

### Event Types

| Event          | Environment Variables                |
| -------------- | ------------------------------------ |
| `startup`      | (none)                               |
| `shutdown`     | (none)                               |
| `pre_command`  | `CCBASH_COMMAND`                     |
| `post_command` | `CCBASH_COMMAND`, `CCBASH_EXIT_CODE` |
| `cd`           | `CCBASH_OLD_CWD`, `CCBASH_NEW_CWD`   |

### Example Hook Script

```bash
#!/bin/sh
# hooks/on_cd.sh - Log directory changes
echo "[$(date)] cd: $CCBASH_OLD_CWD -> $CCBASH_NEW_CWD" >> ~/.cc-bash/cd.log
```

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
├── cc-bash.c              # C implementation (~2700 LOC)
├── cc-bash-sdk.py         # Python + Claude SDK implementation
├── cc-bashrc.template     # Sample configuration file
├── install.sh             # Installation script
├── Makefile               # Build configuration
├── tests/
│   ├── test_unit.c        # Unit tests (170 tests)
│   └── test_cc_bash.sh    # Static analysis tests
├── .github/
│   └── workflows/
│       ├── ci.yml         # CI pipeline (build, test, lint)
│       └── release.yml    # Release automation
├── lib/                   # SmartTerm library (legacy)
├── include/               # Library headers (legacy)
└── examples/              # Example applications (legacy)
```

### User Files

```
~/.cc-bashrc               # Configuration (aliases, snippets, workflows, themes)
~/.cc-bash-history         # Command history
~/.cc-bash/plugins/        # Plugin directory
```

---

## History

This project evolved from "smartterm-prototype":

1. **Original POC**: Attempted ncurses + readline integration
2. **Problem discovered**: ncurses suspend/resume causes output to disappear during input
3. **Library extraction**: Created libsmartterm v1.0 (still available in lib/)
4. **Repurposed**: Simplified to cc-bash using ANSI escape codes
5. **C version**: Simple, working Claude Code-style bash wrapper
6. **Python + SDK**: Added AI integration via Claude Agent SDK

See GitHub issue #14 for the repurposing discussion.

---

## License

MIT License - see [LICENSE](LICENSE) for details.

---

## Related jcaldwell-labs Projects

cc-bash is part of the [jcaldwell-labs](https://github.com/jcaldwell-labs) portfolio:

### Terminal/TUI Projects

| Project                                                            | Description                                   |
| ------------------------------------------------------------------ | --------------------------------------------- |
| [my-grid](https://github.com/jcaldwell-labs/my-grid)               | ASCII canvas editor with vim-style navigation |
| [boxes-live](https://github.com/jcaldwell-labs/boxes-live)         | Real-time ASCII box drawing                   |
| [terminal-stars](https://github.com/jcaldwell-labs/terminal-stars) | Starfield animation for terminals             |
| [atari-style](https://github.com/jcaldwell-labs/atari-style)       | Retro visual effects for terminal apps        |

### CLI Tools

| Project                                                    | Description                               |
| ---------------------------------------------------------- | ----------------------------------------- |
| [my-context](https://github.com/jcaldwell-labs/my-context) | Context tracking for development sessions |
| [fintrack](https://github.com/jcaldwell-labs/fintrack)     | Personal finance tracking CLI             |

### Game Engines

| Project                                                                      | Description                       |
| ---------------------------------------------------------------------------- | --------------------------------- |
| [adventure-engine-v2](https://github.com/jcaldwell-labs/adventure-engine-v2) | Multiplayer text adventure engine |

---

## Credits

Inspired by Claude Code's terminal interface and Warp's modern terminal UX.
