# SmartTerm Library

**A production-ready C library for terminal UIs with scrolling output, readline input, and rich features**

**Status**: Production Library v1.0.0 + Original POC

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

---

## Overview

SmartTerm provides a clean API for building terminal applications with:

- 📜 **Scrolling Output Buffer** - No prompt duplication, clean history
- ⌨️ **Readline Integration** - Command history, line editing, tab completion
- 🎨 **Context-Aware Coloring** - Semantic coloring for different message types
- 📊 **Status Bar** - Fixed status display with customizable content
- 🔍 **Search & Navigation** - Search buffer, scrollback with PgUp/PgDn
- 💾 **Export** - Save output to text, ANSI, Markdown, or HTML
- 🧵 **Thread-Safe** - Write from multiple threads safely
- 🎭 **Themeable** - Built-in and custom color themes

---

## Quick Start

### Installation

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libncurses-dev libreadline-dev

# Or macOS
brew install ncurses readline

# Build library
make -f Makefile.lib lib

# Build examples
make -f Makefile.lib examples

# Run examples
./build/bin/repl          # Calculator REPL
./build/bin/chat_client   # IRC-style chat
./build/bin/log_viewer    # Log monitoring
```

### Hello World

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

Compile with:
```bash
gcc hello.c -Iinclude -Lbuild -lsmartterm -lncurses -lreadline -lpthread -o hello
```

---

## Features

### Core Functionality

**Output Management**
- Thread-safe output buffer (configurable size)
- Context-aware message coloring
- Automatic scrolling or manual control
- Line metadata (timestamps, tags, context)

**Input Handling**
- Readline integration (history, editing, completion)
- Custom prompt strings
- Tab completion framework
- Multi-line input support (basic)

**Visual Features**
- Status bar with left/right alignment
- Configurable themes
- Scrollback navigation (PgUp/PgDn)
- Search with highlighting

**Data Management**
- Export to multiple formats (plain, ANSI, Markdown, HTML)
- Search buffer (plain text or regex)
- Access to raw buffer data
- Line-by-line retrieval

**Advanced**
- Custom key handlers
- Terminal resize handling
- Signal-safe writes
- Configurable behavior

---

## Documentation

- 📖 **[API Reference](docs/SMARTTERM-API.md)** - Complete API documentation
- 🏗️ **[Architecture](docs/ARCHITECTURE.md)** - Design decisions and internals
- 💡 **[Examples](examples/)** - Working example applications
- 📝 **[Decision History](DECISION.md)** - Development path decisions
- 🔄 **[Next Steps](NEXT-STEPS.md)** - Opportunity cost analysis

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
│   ├── smartterm_keyhandler.c
│   └── smartterm_internal.h
├── examples/                # Example applications
│   ├── repl.c              # Calculator REPL
│   ├── chat_client.c       # IRC-style chat
│   └── log_viewer.c        # Log monitoring
├── docs/                    # Documentation
│   ├── SMARTTERM-API.md    # API reference
│   └── ARCHITECTURE.md     # Architecture docs
├── smartterm_poc.c          # Original POC (250 LOC)
├── Makefile                 # POC build
└── Makefile.lib             # Library build
```

---

## Examples

### Calculator REPL

```bash
./build/bin/repl
```

Features: Expression evaluation, history, export

### Chat Client

```bash
./build/bin/chat_client
```

Features: Simulated messages, IRC-style interface, thread-safe output

### Log Viewer

```bash
./build/bin/log_viewer
```

Features: Real-time log generation, search, pause/resume, export

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

### Input

```c
char *input = smartterm_read_line(ctx, "Enter command: ");
// Use input
free(input);
```

### Search

```c
smartterm_search_result_t *results;
int count;
smartterm_search(ctx, "error", false, &results, &count);
smartterm_free_search_results(results);
```

### Export

```c
smartterm_export(ctx, "output.txt", EXPORT_PLAIN, 0, -1, true);
smartterm_export(ctx, "output.html", EXPORT_HTML, 0, -1, true);
```

See [API Reference](docs/SMARTTERM-API.md) for complete documentation.

---

## POC (Proof of Concept)

## What This Demonstrates

✅ **Scrolling Output Buffer**
- History scrolls up (oldest lines drop off)
- No prompt duplication in output
- Clean display of past interactions

✅ **Readline Integration**
- Multi-line input support
- Command history (up/down arrows)
- Line editing (Ctrl+A, Ctrl+E, etc.)
- Tab completion ready (not implemented)

✅ **Context Awareness**
- `> normal` - Default input
- `! command` - System command context (yellow)
- `# comment` - Comment context (green)
- `@ special` - Special action context (cyan)
- `/ search` - Search context (magenta)

✅ **Status Bar**
- Fixed at bottom (above input)
- Shows app state and info
- Separates output from input area

✅ **ncurses + readline**
- Proves the integration works
- Both libraries cooperate
- Terminal suspends/resumes cleanly

---

The original proof of concept (smartterm_poc.c, ~250 LOC) validated the core concepts:

### Building the POC

```bash
make        # Build POC
make run    # Build and run POC
make clean  # Clean up
```

Or manually:
```bash
gcc -o smartterm_poc smartterm_poc.c -lncurses -lreadline
./smartterm_poc
```

### What the POC Demonstrates

✅ Scrolling output without prompt duplication
✅ Readline integration (history, editing)
✅ Context-aware coloring
✅ Status bar
✅ ncurses + readline coexistence

---

## Testing the POC

### Test 1: Basic Output
```
> hello
Echo [NORMAL]: hello
> world
Echo [NORMAL]: world
```

**Verify**: No prompt duplication, clean history

### Test 2: Context Detection
```
> !system command
Echo [CMD]: system command
> # this is a comment
Echo [COMMENT]: this is a comment
> @ special action
Echo [SPECIAL]: special action
```

**Verify**: Colors change based on context

### Test 3: Commands
```
> help
Available commands:
  help    - Show this help
  clear   - Clear screen
  quit    - Exit program
  ...
```

**Verify**: Output scrolls properly

### Test 4: History & Editing
- Press **Up Arrow**: Previous command
- Press **Down Arrow**: Next command
- **Ctrl+A**: Beginning of line
- **Ctrl+E**: End of line
- **Ctrl+K**: Kill to end of line
- **Ctrl+U**: Kill entire line

**Verify**: Readline features work

### Test 5: Multi-line Input
```
> this is a very long line that exceeds the terminal width and should wrap properly without breaking
```

**Verify**: Long lines wrap correctly

### Test 6: Scrolling
Enter many commands to fill screen:
```
> line 1
> line 2
> line 3
... (continue until screen fills)
```

**Verify**: Old lines scroll off top, new lines appear at bottom

---

## What's Missing (Intentionally)

This is a POC, not a full library. Missing features:

❌ **Not Implemented**:
- Tab completion (readline supports it, not wired up)
- Actual multi-line expanding input (readline is single-line)
- Persistent history file
- Configurable themes
- Custom key bindings
- Full API surface
- Error handling
- Memory safety checks
- Documentation beyond this file

✅ **Core Concept Proven**:
- Output buffer without prompt duplication ✅
- Context-aware coloring ✅
- Status bar ✅
- ncurses + readline integration ✅

---

## Architecture

```
┌──────────────────────────────────┐
│     Output Window (scrolling)    │  ← OutputBuffer, no prompt duplication
│  Line 1                          │
│  Line 2                          │
│  ...                             │
│  Line N                          │
├──────────────────────────────────┤
│  Status Bar (info)               │  ← Status info, counts, mode
├──────────────────────────────────┤
│  (readline input area)           │  ← Input handled by readline
│  > _                             │     (appears below window)
└──────────────────────────────────┘
```

### Key Design Decisions

**1. Output Buffer Separate from Display**
- Store lines in array (no ncurses dependency)
- Render buffer to window on demand
- Allows scrollback, search, export

**2. readline Suspends ncurses**
- ncurses paused during input
- readline handles editing
- ncurses resumes for display

**3. Context as Prefix**
- Store context marker with each line
- Render with appropriate color
- Easy to extend to more contexts

**4. Status Bar as Separate Window**
- Fixed position
- Always visible
- Easy to update independently

---

## Code Stats

- **Lines**: ~250 LOC
- **Dependencies**: ncurses, readline
- **Time to Write**: ~30-45 minutes
- **Complexity**: Low (single file)

---

## Learnings from POC

### What Works Well ✅

1. **ncurses + readline integration**: Clean suspend/resume
2. **Output buffer design**: Prevents prompt duplication
3. **Context coloring**: Easy to parse and display
4. **Status bar**: Good separation of concerns
5. **Code simplicity**: Proves concept without over-engineering

### What Needs Improvement ⚠️

1. **Multi-line input**: readline is single-line, need custom handling
2. **Input area sizing**: readline uses full terminal, conflicts with layout
3. **Suspend/resume**: Flickers slightly on each input
4. **Color coordination**: Hardcoded colors, should be theme-based
5. **Memory management**: No bounds checking, could overflow

### Surprising Discoveries 💡

1. **readline owns the terminal**: Hard to constrain to bottom region
2. **Multi-line is complex**: May need custom input handler, not readline
3. **Suspend/resume works**: But not ideal UX (flickers)
4. **Context detection is easy**: Simple prefix check works well
5. **Buffer management straightforward**: Array of strings is simple and effective

---

## Opportunity Costs Analysis

See `NEXT-STEPS.md` for detailed analysis of paths forward.

---

## Development History

### POC Phase (Complete)
- ✅ Validated core concepts
- ✅ Proved ncurses + readline integration
- ✅ ~250 LOC, 1 hour investment
- ✅ Decision framework created

### Library Phase (Complete)
- ✅ Production-ready API designed
- ✅ Thread-safe implementation
- ✅ Comprehensive features (search, export, themes)
- ✅ Example applications
- ✅ Full documentation

See [DECISION.md](DECISION.md) for development path analysis and [NEXT-STEPS.md](NEXT-STEPS.md) for opportunity costs.

---

## Contributing

Contributions welcome! This is a jcaldwell-labs project.

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests/examples if applicable
5. Submit a pull request

---

## License

MIT License - See LICENSE file for details

---

## Credits

**Original POC**: smartterm-prototype proof of concept (2025-11-17)
**Library Design**: Extracted and enhanced for production use
**Inspired by**: readline, rlwrap, rlfe, Haskell Brick

---

## Status

- **POC**: ✅ Complete and validated
- **Library**: ✅ Production-ready v1.0.0
- **Documentation**: ✅ Comprehensive
- **Examples**: ✅ Three working applications
- **Next**: Ready for use in projects

For questions or issues, see the [API documentation](docs/SMARTTERM-API.md) or create an issue.
