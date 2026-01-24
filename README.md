# cc-bash: Claude Code-style Bash Wrapper

**A simple terminal wrapper that executes bash commands by default with colored output**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.cppreference.com/w/c/11)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/jcaldwell-labs/smartterm-prototype)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](https://github.com/jcaldwell-labs/smartterm-prototype/actions)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

![cc-bash demo](gallery/cc-bash-demo.gif)

> See more demos in the [gallery/](gallery/) directory!

---

## Quick Links

- [Installation](#installation) - Get started in under 1 minute
- [Features](#features) - What can cc-bash do?
- [Configuration](#configuration) - Customize your experience
- [Why cc-bash?](#why-cc-bash) - What makes it different?
- [Contributing](CONTRIBUTING.md) - Help improve cc-bash

---

## Why cc-bash?

cc-bash bridges the gap between traditional shells and modern terminal experiences.

### Key Benefits

✅ **No learning curve** - It's just bash, with better UX  
✅ **Enhanced visibility** - Colored output, clear status bar, persistent history  
✅ **Extensible** - Aliases, snippets, workflows, plugins, themes  
✅ **Lightweight** - Pure C, minimal dependencies (~3300 LOC)  
✅ **AI-ready** - Python version with Claude SDK integration

### Comparison Table

| Feature                     | Plain Bash | Fish/Zsh | Warp     | cc-bash       |
| --------------------------- | ---------- | -------- | -------- | ------------- |
| **Bash compatibility**      | ✅         | ⚠️       | ✅       | ✅            |
| **Colored output**          | Manual     | Built-in | Built-in | ✅            |
| **Status bar**              | ❌         | Plugin   | Built-in | ✅            |
| **History search**          | Ctrl+R     | Ctrl+R   | ✅       | ✅            |
| **Tab completion**          | ✅         | ✅       | ✅       | ✅            |
| **Command aliases**         | ✅         | ✅       | ✅       | ✅            |
| **Parameterized templates** | ❌         | ❌       | ❌       | ✅ (Snippets) |
| **Multi-command workflows** | ❌         | ❌       | ❌       | ✅            |
| **Plugin system**           | ❌         | ✅       | ✅       | ✅            |
| **Theme customization**     | ❌         | ✅       | ✅       | ✅            |
| **AI integration**          | ❌         | Plugin   | Built-in | ✅ (Python)   |
| **Resource usage**          | Minimal    | Low      | High     | Minimal       |
| **Open source**             | ✅         | ✅       | ❌       | ✅            |

### Use Cases

- **Development workflows** - Combine build, test, deploy commands into workflows
- **System administration** - Create snippets for common tasks with parameters
- **Learning** - Comments (#) let you annotate your command history
- **AI pair programming** - Use Python version with Claude for command suggestions
- **Server management** - Lightweight, runs over SSH with minimal dependencies

---

## Two Versions Available

| Version          | Language | AI Integration   | Dependencies                           |
| ---------------- | -------- | ---------------- | -------------------------------------- |
| `cc-bash`        | C        | No               | None (pure POSIX)                      |
| `cc-bash-sdk.py` | Python   | Yes (Claude SDK) | claude-agent-sdk, prompt_toolkit, rich |

**Most users want the C version** - it's fast, lightweight, and works everywhere.

---

## Quick Demo (30 seconds)

```bash
# Install and run
git clone https://github.com/jcaldwell-labs/smartterm-prototype.git
cd smartterm-prototype && make && ./cc-bash

# Try it out
$ ls -la                          # Commands in cyan, output in white
$ grep -r "TODO" .                # Colored grep output (automatic!)
$ # This is a note                # Comments in yellow (not executed)
$ @help                           # Internal commands with @
$ @alias ll='ls -la'              # Create shortcuts
$ ll                              # Use your alias
$ @quit                           # Exit (or Ctrl+D)
```

**What you get**:

- Every command shows in color
- Status bar with directory, exit code, time
- History with Ctrl+R fuzzy search
- Tab completion for files and commands

---

## Features at a Glance

### Core Features

- ⚡ **Zero prefix** - Type bash commands directly (no `!` or `$`)
- 🎨 **Auto-colored output** - PTY-based execution means ls, grep, etc. show colors
- 📊 **Status bar** - Current directory, last exit code, timestamp
- 🔍 **Ctrl+R search** - Fuzzy search through command history
- 📝 **Persistent history** - Saved to ~/.cc-bash-history

### Power User Features

- 🔗 **Aliases** - Shortcuts like `alias gs='git status'`
- 📋 **Snippets** - Templates with params: `snippet find='find . -name "$1"'`
- 🔄 **Workflows** - Multi-command sequences: `workflow build='make clean && make'`
- 🔌 **Plugins** - Shell script hooks for custom behavior
- 🎨 **Themes** - Customize all colors via config

### Developer Experience

- 💬 **Comments** - `# notes` displayed but not executed
- ⌨️ **Full line editing** - Left/right arrows, delete, backspace
- 📑 **Tab completion** - Files, directories, commands
- 🚀 **Minimal deps** - Pure C, no ncurses or readline needed

<details>
<summary><b>Expand for detailed feature comparison</b></summary>

| Feature                     | Plain Bash | Fish/Zsh | Warp     | cc-bash       |
| --------------------------- | ---------- | -------- | -------- | ------------- |
| **Bash compatibility**      | ✅         | ⚠️       | ✅       | ✅            |
| **Colored output**          | Manual     | Built-in | Built-in | ✅            |
| **Status bar**              | ❌         | Plugin   | Built-in | ✅            |
| **History search**          | Ctrl+R     | Ctrl+R   | ✅       | ✅            |
| **Tab completion**          | ✅         | ✅       | ✅       | ✅            |
| **Command aliases**         | ✅         | ✅       | ✅       | ✅            |
| **Parameterized templates** | ❌         | ❌       | ❌       | ✅ (Snippets) |
| **Multi-command workflows** | ❌         | ❌       | ❌       | ✅            |
| **Plugin system**           | ❌         | ✅       | ✅       | ✅            |
| **Theme customization**     | ❌         | ✅       | ✅       | ✅            |
| **AI integration**          | ❌         | Plugin   | Built-in | ✅ (Python)   |
| **Resource usage**          | Minimal    | Low      | High     | Minimal       |
| **Open source**             | ✅         | ✅       | ❌       | ✅            |

</details>

---

## Installation

### Quick Install (Recommended)

```bash
# Build from source (< 1 minute)
git clone https://github.com/jcaldwell-labs/smartterm-prototype.git
cd smartterm-prototype
make
sudo make install

# Or install locally (no sudo)
make install PREFIX=$HOME/.local
export PATH="$HOME/.local/bin:$PATH"
```

<details>
<summary><b>Alternative installation methods</b></summary>

### Option 1: Homebrew (macOS/Linux)

```bash
brew tap jcaldwell-labs/cc-bash
brew install cc-bash
cp $(brew --prefix)/share/cc-bash/cc-bashrc.template ~/.cc-bashrc
mkdir -p ~/.cc-bash/plugins
```

### Option 2: AUR (Arch Linux)

```bash
yay -S cc-bash
# or: paru -S cc-bash

cp /usr/share/cc-bash/cc-bashrc.template ~/.cc-bashrc
mkdir -p ~/.cc-bash/plugins
```

### Option 3: Install Script

```bash
curl -fsSL https://raw.githubusercontent.com/jcaldwell-labs/smartterm-prototype/master/install.sh | bash
```

### Option 4: Pre-built Binary

```bash
# Linux x86_64
curl -fsSL https://github.com/jcaldwell-labs/smartterm-prototype/releases/latest/download/cc-bash-linux-x86_64 -o cc-bash
chmod +x cc-bash
sudo mv cc-bash /usr/local/bin/

# Setup config
mkdir -p ~/.cc-bash/plugins
curl -fsSL https://raw.githubusercontent.com/jcaldwell-labs/smartterm-prototype/master/cc-bashrc.template -o ~/.cc-bashrc
```

</details>

---

## Quick Start

```bash
# Run cc-bash
cc-bash

# Try basic commands
$ ls -la
$ pwd
$ echo "Hello, cc-bash!"

# Try a note (displayed, not executed)
$ # This is my first note

# Get help
$ @help

# Exit
$ @quit
```

---

## Configuration

cc-bash is configured via `~/.cc-bashrc`. Edit with `@edit` and apply with `@reload`.

**Quick examples below** - See [docs/CONFIGURATION.md](docs/CONFIGURATION.md) for complete guide.

<details>
<summary><b>Aliases - Command shortcuts</b></summary>

```bash
# Define command shortcuts
alias ll='ls -la'
alias gs='git status'
alias gd='git diff'
```

Usage: Type `ll` and it expands to `ls -la`

</details>

<details>
<summary><b>Snippets - Parameterized templates</b></summary>

```bash
# Define templates with $1, $2, etc. placeholders
snippet find-name='find . -name "$1"'
snippet grep-r='grep -r "$1" .'
snippet mkdir-cd='mkdir -p $1 && cd $1'
```

Usage: `@snippet find-name "*.c"` → `find . -name "*.c"`

</details>

<details>
<summary><b>Workflows - Multi-step sequences</b></summary>

```bash
# Combine commands with && (stop on error) or ; (continue)
workflow build='make clean && make && make test'
workflow gitcheck='git status; git diff --stat'
```

Usage:

- `@workflow` - List all workflows
- `@workflow build` - Run the build workflow
- `@workflow build --dry-run` - Preview without executing

</details>

<details>
<summary><b>Themes - Customize colors</b></summary>

```bash
# Colors: black, red, green, yellow, blue, magenta, cyan, white
# Modifiers: bold, dim
theme.prompt=bold cyan
theme.error=bold red
theme.comment=green
theme.dim=dim
theme.header=bold white
theme.status=bold
theme.scroll=dim
```

</details>

<details>
<summary><b>Plugins - Extend functionality</b></summary>

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

See [full plugin documentation](docs/ARCHITECTURE.md) for details.

</details>

---

## Python + AI Version

For AI-powered assistance, use the Python version with Claude SDK integration.

<details>
<summary><b>Click to expand: Python setup and AI commands</b></summary>

### Quick Start

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

### Example Session

```
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

## Keyboard Shortcuts & Commands

<details>
<summary><b>Keyboard shortcuts</b></summary>

| Key          | Action                                              |
| ------------ | --------------------------------------------------- |
| `Ctrl+R`     | Fuzzy history search (type to search, Enter to use) |
| `Ctrl+C`     | Cancel current input or search                      |
| `Ctrl+D`     | Exit cc-bash                                        |
| `Up/Down`    | Navigate command history                            |
| `Left/Right` | Move cursor in input                                |
| `Tab`        | Complete command/file (double-Tab for options)      |
| `Esc`        | Cancel search mode                                  |

</details>

<details>
<summary><b>@ Commands reference</b></summary>

| Command                        | Shortcut | Description                                |
| ------------------------------ | -------- | ------------------------------------------ |
| `@help`                        | `@h`     | Show help                                  |
| `@clear`                       | `@c`     | Clear screen                               |
| `@quit`                        | `@q`     | Exit cc-bash                               |
| `@edit`                        | `@e`     | Edit config file (~/.cc-bashrc) in $EDITOR |
| `@reload`                      | `@r`     | Reload configuration                       |
| `@alias`                       | -        | List aliases (\* = session only)           |
| `@alias name=cmd`              | -        | Add session alias                          |
| `@alias save`                  | -        | Save session aliases to config file        |
| `@snippet [name args...]`      | -        | List snippets or run one with arguments    |
| `@workflow [name] [--dry-run]` | -        | List workflows, run one, or preview        |
| `@theme`                       | -        | Display current theme colors               |
| `@hooks`                       | -        | List registered event hooks                |
| `@plugins`                     | -        | List loaded plugins                        |

</details>

---

## Building from Source

```bash
# Build cc-bash (default)
make

# Build and run
make run

# Build with debug symbols
make debug

# Clean
make clean

# Show all targets
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

## Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Quick links**:

- [Good First Issues](https://github.com/jcaldwell-labs/smartterm-prototype/labels/good%20first%20issue) - Great for newcomers
- [GitHub Discussions](https://github.com/jcaldwell-labs/smartterm-prototype/discussions) - Questions and ideas
- [Code of Conduct](CODE_OF_CONDUCT.md) - Community standards

### Contributors

Thank you to everyone who has contributed to cc-bash!

<!-- ALL-CONTRIBUTORS-LIST:START -->
<!-- This section will be updated by all-contributors bot -->
<!-- ALL-CONTRIBUTORS-LIST:END -->

Want to see your name here? Check out our [contribution guide](CONTRIBUTING.md)!

---

## License

MIT License - see [LICENSE](LICENSE) for details.

---

## Support

Need help? Check out:

- [docs/SUPPORT.md](docs/SUPPORT.md) - Common issues and solutions
- [GitHub Issues](https://github.com/jcaldwell-labs/smartterm-prototype/issues) - Bug reports and feature requests
- [GitHub Discussions](https://github.com/jcaldwell-labs/smartterm-prototype/discussions) - Questions and community

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
