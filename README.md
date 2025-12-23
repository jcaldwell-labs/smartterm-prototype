# SmartTerm

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.cppreference.com/w/c/11)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

**A production-ready C library for terminal UIs with scrolling output, readline input, and rich features.**

> *Build terminal applications that feel native - scrolling history, command editing, context-aware coloring, all in one library.*

---

## Why SmartTerm?

Traditional terminal UIs suffer from **prompt duplication** and poor readline integration. SmartTerm solves this by separating output from input while maintaining a native terminal feel.

**Key Benefits:**
- **No prompt duplication** - Output buffer stores history cleanly
- **Full readline integration** - Command history, line editing, tab completion
- **Context-aware coloring** - Semantic colors for different message types
- **Thread-safe by design** - Write from multiple threads safely
- **Export anywhere** - Save output to plain text, ANSI, Markdown, or HTML

**Perfect for:**
- Interactive REPLs and shells
- Log viewers and monitoring tools
- Chat clients and messaging apps
- Text adventure games
- Any terminal app needing rich input/output

---

## Demo

```
┌──────────────────────────────────┐
│  [INFO] Application started      │  ← Scrolling output
│  [CMD] Loading config...         │     (no prompt duplication)
│  [OK] Ready for input            │
├──────────────────────────────────┤
│  SmartTerm v1.0 | Lines: 42      │  ← Fixed status bar
├──────────────────────────────────┤
│  > _                             │  ← Readline input
└──────────────────────────────────┘
```

### Try It Yourself

```bash
# Build and run the calculator REPL
make -f Makefile.lib examples
./build/bin/repl
```

---

## Quick Start

### Installation

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libncurses-dev libreadline-dev

# Or macOS
brew install ncurses readline

# Clone and build
git clone https://github.com/jcaldwell-labs/smartterm-prototype.git
cd smartterm-prototype
make -f Makefile.lib lib
```

### First Steps

**1. Create a simple REPL:**

```c
#include <smartterm.h>

int main(void) {
    smartterm_ctx *ctx = smartterm_init(NULL);

    smartterm_write(ctx, "Hello, SmartTerm!", CTX_INFO);
    smartterm_status_set(ctx, "My App", "Ready");

    char *input = smartterm_read_line(ctx, "> ");
    smartterm_write_fmt(ctx, CTX_SUCCESS, "You said: %s", input);

    free(input);
    smartterm_cleanup(ctx);
    return 0;
}
```

**2. Compile and run:**

```bash
gcc hello.c -Iinclude -Lbuild -lsmartterm -lncurses -lreadline -lpthread -o hello
./hello
```

**3. Explore the examples:**

```bash
./build/bin/repl          # Calculator REPL
./build/bin/chat_client   # IRC-style chat simulation
./build/bin/log_viewer    # Real-time log monitoring
```

---

## Features

### Core Functionality

| Feature | Description | Example |
|---------|-------------|---------|
| **Scrolling Output** | History without prompt duplication | `smartterm_write(ctx, msg, CTX_INFO)` |
| **Readline Input** | History, editing, completion | `smartterm_read_line(ctx, "> ")` |
| **Context Colors** | Semantic message coloring | `CTX_NORMAL`, `CTX_ERROR`, `CTX_SUCCESS` |
| **Status Bar** | Fixed info display | `smartterm_status_set(ctx, "App", "Ready")` |
| **Thread-Safe** | Write from any thread | Lock-free output buffer |
| **Search** | Find in buffer (text/regex) | `smartterm_search(ctx, "error", ...)` |
| **Export** | Save to multiple formats | `smartterm_export(ctx, "out.html", EXPORT_HTML, ...)` |
| **Themes** | Built-in and custom themes | `smartterm_set_theme(ctx, THEME_DARK)` |

### Context Types

| Context | Color | Use Case |
|---------|-------|----------|
| `CTX_NORMAL` | White | Default output |
| `CTX_COMMAND` | Yellow | System commands |
| `CTX_COMMENT` | Green | Comments, notes |
| `CTX_SPECIAL` | Cyan | Special actions |
| `CTX_SEARCH` | Magenta | Search results |
| `CTX_ERROR` | Red | Error messages |
| `CTX_SUCCESS` | Green | Success messages |
| `CTX_INFO` | Blue | Informational |

---

## Comparison

| Feature | SmartTerm | Raw ncurses | Raw readline | rlwrap |
|---------|:---------:|:-----------:|:------------:|:------:|
| Scrolling output | ✅ | ⚠️ Manual | ❌ | ❌ |
| Readline integration | ✅ | ❌ | ✅ | ✅ |
| No prompt duplication | ✅ | ⚠️ Manual | ❌ | ❌ |
| Context-aware colors | ✅ | ⚠️ Manual | ❌ | ❌ |
| Status bar | ✅ | ⚠️ Manual | ❌ | ❌ |
| Thread-safe output | ✅ | ❌ | ❌ | ❌ |
| Export to formats | ✅ | ❌ | ❌ | ❌ |
| Search buffer | ✅ | ❌ | ✅ | ✅ |

**SmartTerm = ncurses + readline + batteries included**

---

## Documentation

- **[API Reference](docs/SMARTTERM-API.md)** - Complete function documentation
- **[Architecture](docs/ARCHITECTURE.md)** - Design decisions and internals
- **[Examples](examples/)** - Working example applications
- **[Contributing](CONTRIBUTING.md)** - How to contribute

---

## Project Structure

```
smartterm-prototype/
├── include/
│   └── smartterm.h          # Public API header
├── lib/smartterm/           # Library implementation
│   ├── smartterm_core.c
│   ├── smartterm_output.c
│   ├── smartterm_input.c
│   ├── smartterm_render.c
│   ├── smartterm_theme.c
│   ├── smartterm_status.c
│   ├── smartterm_scroll.c
│   ├── smartterm_search.c
│   ├── smartterm_export.c
│   └── smartterm_keyhandler.c
├── examples/                # Example applications
│   ├── repl.c              # Calculator REPL
│   ├── chat_client.c       # IRC-style chat
│   └── log_viewer.c        # Log monitoring
├── docs/                    # Documentation
└── smartterm_poc.c          # Original POC (250 LOC)
```

---

## API Highlights

### Initialization

```c
smartterm_config_t config = smartterm_default_config();
config.max_lines = 5000;
config.history_enabled = true;
config.prompt = "$ ";

smartterm_ctx *ctx = smartterm_init(&config);
```

### Output

```c
smartterm_write(ctx, "Normal message", CTX_NORMAL);
smartterm_write(ctx, "Error occurred!", CTX_ERROR);
smartterm_write_fmt(ctx, CTX_SUCCESS, "Count: %d", count);
```

### Search & Export

```c
// Search buffer
smartterm_search_result_t *results;
int count;
smartterm_search(ctx, "error", false, &results, &count);
smartterm_free_search_results(results);

// Export to HTML
smartterm_export(ctx, "output.html", EXPORT_HTML, 0, -1, true);
```

See **[API Reference](docs/SMARTTERM-API.md)** for complete documentation.

---

## Roadmap

See [.github/planning/](.github/planning/) for development roadmap and backlog.

**Current Status:**
- ✅ POC validated (smartterm_poc.c)
- ✅ Production library v1.0.0
- ✅ Three example applications
- ✅ Comprehensive documentation

---

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Quick Start:**
1. Fork the repository
2. Create a feature branch
3. Make changes with tests
4. Format code: `make -f Makefile.lib format`
5. Submit a pull request

---

## Community

- **Issues:** [GitHub Issues](https://github.com/jcaldwell-labs/smartterm-prototype/issues)
- **Questions:** Check the [API docs](docs/SMARTTERM-API.md) first, then open an issue
- **Author:** jcaldwell-labs

---

## License

MIT License - see [LICENSE](LICENSE) for details.

---

## Related jcaldwell-labs Projects

SmartTerm is part of the [jcaldwell-labs](https://github.com/jcaldwell-labs) portfolio. These projects share synergies and can be combined:

### Terminal/TUI Projects

| Project | Description | Synergy with SmartTerm |
|---------|-------------|------------------------|
| [my-grid](https://github.com/jcaldwell-labs/my-grid) | ASCII canvas editor with vim-style navigation, zones, PTY support | Use SmartTerm's readline for command input in my-grid |
| [boxes-live](https://github.com/jcaldwell-labs/boxes-live) | Real-time ASCII box drawing with joystick support | Share ncurses patterns; SmartTerm for status display |
| [terminal-stars](https://github.com/jcaldwell-labs/terminal-stars) | Starfield animation for terminals | SmartTerm can embed starfield as background layer |
| [atari-style](https://github.com/jcaldwell-labs/atari-style) | Retro visual effects and shaders for terminal apps | Apply atari-style shaders to SmartTerm themes |

### CLI Tools

| Project | Description | Synergy with SmartTerm |
|---------|-------------|------------------------|
| [my-context](https://github.com/jcaldwell-labs/my-context) | Context tracking for development sessions (Go CLI) | Track SmartTerm dev sessions; integrate context display |
| [fintrack](https://github.com/jcaldwell-labs/fintrack) | Personal finance tracking CLI (Go) | SmartTerm could provide richer TUI for fintrack |
| [tario](https://github.com/jcaldwell-labs/tario) | Terminal-based platformer game (Go) | Share terminal rendering techniques |

### Game Engines

| Project | Description | Synergy with SmartTerm |
|---------|-------------|------------------------|
| [adventure-engine-v2](https://github.com/jcaldwell-labs/adventure-engine-v2) | Multiplayer text adventure engine (C) | SmartTerm provides the terminal UI layer |

### Meta/Organization

| Project | Description |
|---------|-------------|
| [capability-catalog](https://github.com/jcaldwell-labs/capability-catalog) | Skill/capability definitions for AI agents |

---

## Credits

**Original POC:** smartterm-prototype (2025-11-17)
**Library Design:** Extracted and enhanced for production use
**Inspired by:** readline, rlwrap, rlfe, Haskell Brick
