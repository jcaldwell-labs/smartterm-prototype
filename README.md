# cc-bash: Claude Code-style Bash Wrapper

**A simple terminal wrapper that executes bash commands by default with colored output**

[![CI](https://github.com/jcaldwell-labs/smartterm-prototype/actions/workflows/ci.yml/badge.svg)](https://github.com/jcaldwell-labs/smartterm-prototype/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/jcaldwell-labs/smartterm-prototype)](https://github.com/jcaldwell-labs/smartterm-prototype/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.cppreference.com/w/c/11)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

---

## What is cc-bash?

- **Bash-first**: Commands run in bash by default - no `!` prefix needed
- **Visual feedback**: Colored output (cyan commands, white stdout, red stderr) with a status bar
- **Smart input**: Tab completion, fuzzy history search (Ctrl+R), persistent history
- **Extensible**: Aliases, snippets, workflows, plugins, and themes
- **Two flavors**: Pure C version (fast, minimal) or Python + Claude AI version

---

## Quick Install

### Download Pre-built Binary (Recommended)

```bash
# Linux x86_64
curl -fsSL https://github.com/jcaldwell-labs/smartterm-prototype/releases/latest/download/cc-bash-linux-x86_64 -o cc-bash
chmod +x cc-bash
sudo mv cc-bash /usr/local/bin/

# Create config directory
mkdir -p ~/.cc-bash/plugins
curl -fsSL https://raw.githubusercontent.com/jcaldwell-labs/smartterm-prototype/master/cc-bashrc.template -o ~/.cc-bashrc
```

<details>
<summary><strong>Other installation methods</strong></summary>

### Build from Source

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

### Local User Install (no sudo)

```bash
make
make install PREFIX=$HOME/.local

# Add to PATH (add to ~/.bashrc)
export PATH="$HOME/.local/bin:$PATH"
```

### Homebrew (macOS/Linux)

```bash
brew tap jcaldwell-labs/cc-bash
brew install cc-bash
cp $(brew --prefix)/share/cc-bash/cc-bashrc.template ~/.cc-bashrc
mkdir -p ~/.cc-bash/plugins
```

### AUR (Arch Linux)

```bash
yay -S cc-bash
cp /usr/share/cc-bash/cc-bashrc.template ~/.cc-bashrc
mkdir -p ~/.cc-bash/plugins
```

### Install Script

```bash
curl -fsSL https://raw.githubusercontent.com/jcaldwell-labs/smartterm-prototype/master/install.sh | bash
```

</details>

---

## Quick Start

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

<details>
<summary><strong>Python + AI Version Quick Start</strong></summary>

The Python version includes Claude AI integration:

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

</details>

---

## Features

### Command Execution

- All input executes as bash commands by default
- **Full color support** - PTY-based execution means `ls`, `grep`, `bat` show colors automatically
- stdout displayed in white/default color
- stderr displayed in red
- Exit codes shown in status bar

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

<details>
<summary><strong>Plugins</strong></summary>

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

</details>

<details>
<summary><strong>Design Philosophy</strong></summary>

## Design Philosophy

cc-bash takes a **simple approach** using ANSI escape codes instead of ncurses:

1. **Output stays visible** - No TUI that hides when you type
2. **Readline for input** - Command history, line editing
3. **ANSI colors** - Works in any terminal
4. **Minimal dependencies** - Just libutil for PTY support

### Why Not ncurses?

An earlier POC used ncurses + readline integration. While the concept worked, the UX suffered:

- ncurses must suspend during readline input
- This causes the output area to disappear while typing
- Not the seamless experience we wanted

The current ANSI-based approach keeps all output visible at all times.

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

</details>

<details>
<summary><strong>Project Structure</strong></summary>

## Project Structure

```
smartterm-prototype/
├── cc-bash.c              # C implementation (~3300 LOC)
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

### Two Versions Available

| Version          | Language | AI Integration   | Dependencies                           |
| ---------------- | -------- | ---------------- | -------------------------------------- |
| `cc-bash`        | C        | No               | libutil (forkpty)                      |
| `cc-bash-sdk.py` | Python   | Yes (Claude SDK) | claude-agent-sdk, prompt_toolkit, rich |

</details>

<details>
<summary><strong>History</strong></summary>

## History

This project evolved from "smartterm-prototype":

1. **Original POC**: Attempted ncurses + readline integration
2. **Problem discovered**: ncurses suspend/resume causes output to disappear during input
3. **Library extraction**: Created libsmartterm v1.0 (still available in lib/)
4. **Repurposed**: Simplified to cc-bash using ANSI escape codes
5. **C version**: Simple, working Claude Code-style bash wrapper
6. **Python + SDK**: Added AI integration via Claude Agent SDK

See GitHub issue #14 for the repurposing discussion.

</details>

---

## Building

```bash
# Build cc-bash (default)
make

# Build and run
make run

# Run tests
make test

# Full quality check
make check

# Clean
make clean

# Show all targets
make help
```

---

## Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

Quick start:

```bash
git clone https://github.com/jcaldwell-labs/smartterm-prototype.git
cd smartterm-prototype
make
make check  # Run before committing
```

---

## License

MIT License - see [LICENSE](LICENSE) for details.

---

## Related Projects

cc-bash is part of the [jcaldwell-labs](https://github.com/jcaldwell-labs) portfolio:

| Project                                                            | Description                                   |
| ------------------------------------------------------------------ | --------------------------------------------- |
| [my-grid](https://github.com/jcaldwell-labs/my-grid)               | ASCII canvas editor with vim-style navigation |
| [boxes-live](https://github.com/jcaldwell-labs/boxes-live)         | Real-time ASCII box drawing                   |
| [terminal-stars](https://github.com/jcaldwell-labs/terminal-stars) | Starfield animation for terminals             |
| [my-context](https://github.com/jcaldwell-labs/my-context)         | Context tracking for development sessions     |

---

## Credits

Inspired by Claude Code's terminal interface and Warp's modern terminal UX.
