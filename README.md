# SmartTerm Simple Library

**Purpose**: Minimal terminal UI library for building interactive command-line applications
**Status**: Extracted from POC, ready for use (Path B implementation)
**Version**: 0.1.0

---

## Overview

SmartTerm Simple is a lightweight C library that provides:
- **Scrolling output buffer** - Clean history without prompt duplication
- **Context-aware coloring** - Color code output based on context (commands, comments, etc.)
- **Status bar** - Fixed status display separate from output
- **Readline integration** - Full editing, history, and input features

This library was extracted from the SmartTerm POC and is ready to use in real applications.

---

## Quick Start

### Build and Run

```bash
make              # Build library and example
make run          # Run the example program
make run-poc      # Run the original POC
make clean        # Clean all build artifacts
```

### Prerequisites

```bash
# Debian/Ubuntu
sudo apt-get install libncurses-dev libreadline-dev

# macOS
brew install ncurses readline
```

---

## Library API

### Initialization

```c
#include "smartterm_simple.h"

int main() {
    // Initialize the library
    if (smartterm_init() != 0) {
        fprintf(stderr, "Failed to initialize\n");
        return 1;
    }

    // ... your application code ...

    // Cleanup when done
    smartterm_cleanup();
    return 0;
}
```

### Adding Output

```c
// Add lines with different contexts
smartterm_add_output("Normal message", SMARTTERM_CTX_NORMAL);
smartterm_add_output("System command", SMARTTERM_CTX_COMMAND);   // Yellow
smartterm_add_output("Comment text", SMARTTERM_CTX_COMMENT);     // Green
smartterm_add_output("Special action", SMARTTERM_CTX_SPECIAL);   // Cyan
smartterm_add_output("Search query", SMARTTERM_CTX_SEARCH);      // Magenta
```

### Reading Input

```c
// Read a line from the user
char *input = smartterm_readline("> ");
if (input) {
    // Process input...
    free(input);  // Don't forget to free!
}
```

### Detecting Context

```c
// Detect context from user input
smartterm_context_t ctx = smartterm_detect_context("!command");
// Returns SMARTTERM_CTX_COMMAND

// Strip context prefix
const char *text = smartterm_strip_context("!command");
// Returns "command" (without the !)
```

### Status Bar

```c
// Update status bar (left and right sides)
smartterm_set_status("App Name", "v1.0.0");
smartterm_set_status("Ready", "Lines: 42");
```

### Other Functions

```c
// Clear all output
smartterm_clear_output();
```

---

## Context Types

The library supports five context types for color-coded output:

| Context | Prefix | Color | Use Case |
|---------|--------|-------|----------|
| `SMARTTERM_CTX_NORMAL` | (none) | White | Regular output |
| `SMARTTERM_CTX_COMMAND` | `!` | Yellow | System commands |
| `SMARTTERM_CTX_COMMENT` | `#` | Green | Comments/notes |
| `SMARTTERM_CTX_SPECIAL` | `@` | Cyan | Special actions |
| `SMARTTERM_CTX_SEARCH` | `/` | Magenta | Search queries |

---

## Example Program

See `example.c` for a complete demonstration. The example shows:
- Basic initialization and cleanup
- Adding output with different contexts
- Reading user input with readline
- Context detection and processing
- Status bar updates
- Command handling

---

## Project Structure

```
smartterm-prototype/
├── smartterm_simple.h      # Public API header
├── smartterm_simple.c      # Implementation
├── example.c               # Example program
├── smartterm_poc.c         # Original POC (reference)
├── Makefile                # Build system
├── README.md               # This file
├── DECISION.md             # Implementation decision log
├── NEXT-STEPS.md           # Opportunity cost analysis
└── SESSION-SUMMARY.md      # Development session notes
```

---

## Using the Library in Your Project

### Option 1: Static Linking

```bash
# Build the library
make library

# Compile your program
gcc -o myapp myapp.c -L. -lsmartterm_simple -lncurses -lreadline
```

### Option 2: Copy Source Files

```bash
# Copy the library files to your project
cp smartterm_simple.{h,c} /path/to/your/project/

# Include in your build
gcc -o myapp myapp.c smartterm_simple.c -lncurses -lreadline
```

---

## What This Library Demonstrates

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

## Testing

### Running the Tests

```bash
make test     # Show testing instructions
make run      # Run the example program
```

### Manual Testing

1. **Basic Output**
   ```
   > hello world
   Echo [NORMAL]: hello world
   ```
   Verify: Clean output without prompt duplication

2. **Context Detection**
   ```
   > !system command
   Echo [COMMAND]: system command
   > #this is a comment
   Echo [COMMENT]: this is a comment
   > @special action
   Echo [SPECIAL]: special action
   > /search query
   Echo [SEARCH]: search query
   ```
   Verify: Different colors for each context

3. **Commands**
   ```
   > help
   > clear
   > quit
   ```
   Verify: Commands work as expected

4. **History & Editing**
   - **Up/Down arrows**: Navigate history
   - **Ctrl+A**: Jump to beginning of line
   - **Ctrl+E**: Jump to end of line
   - **Ctrl+K**: Delete to end of line
   - **Ctrl+U**: Delete entire line

   Verify: All readline features work

5. **Scrolling**
   - Enter many lines until screen fills
   - Verify: Old lines scroll off, recent lines visible

---

## Architecture

```
┌──────────────────────────────────┐
│     Output Window (scrolling)    │  ← Output buffer, no prompt duplication
│  Line 1                          │
│  Line 2                          │
│  ...                             │
│  Line N (most recent)            │
├──────────────────────────────────┤
│  Status Bar (fixed)              │  ← Status: App info, counts, state
├──────────────────────────────────┤
│  (readline input area)           │  ← Input: readline handles editing
│  > _                             │
└──────────────────────────────────┘
```

### Design Decisions

**Output Buffer**
- Stored separately from ncurses windows
- Rendered on demand for flexibility
- Supports up to 1000 lines (configurable)

**Context System**
- Prefix character indicates context (!, #, @, /)
- Stored with each line for rendering
- Easy to extend to new contexts

**Terminal Integration**
- ncurses for display (output + status)
- readline for input (editing + history)
- Suspend/resume pattern for cooperation

**Status Bar**
- Separate ncurses window
- Fixed at bottom (above input)
- Displays app name, version, stats, etc.

---

## Limitations & Future Work

### Current Limitations

⚠️ **Known Issues**:
- Readline owns full terminal (slight flicker on input)
- Single-line input only (readline limitation)
- Fixed color scheme (no themes yet)
- Basic memory management (no overflow protection)

### Future Enhancements (Phase 2)

If validated by real usage, Phase 2 could add:
- Multi-line expanding input area
- Custom themes and color schemes
- Tab completion support
- Scrollback navigation
- Export/save output buffer
- Better error handling
- Comprehensive documentation
- Test suite

See `NEXT-STEPS.md` for the full roadmap.

---

## Development History

This library follows **Path B→A** (Ship First, Refactor Later):
- ✅ **Phase 1** (Current): Extract POC to minimal library
- ⏳ **Phase 2** (Future): Build full library based on learnings

For detailed decision rationale, see `DECISION.md`.
For opportunity cost analysis, see `NEXT-STEPS.md`.
For session notes, see `SESSION-SUMMARY.md`.

---

## Original POC

The original proof-of-concept is preserved as `smartterm_poc.c`.
Run it with: `make run-poc`

The POC validates the core concepts before extraction to a reusable library.
