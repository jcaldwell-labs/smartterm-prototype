# SmartTerm Library Architecture

**Version**: 1.0.0
**Last Updated**: 2025-11-18

---

## Overview

SmartTerm is a C library that provides a split-screen terminal UI with:
- Scrolling output buffer (top region)
- Input area with readline support (bottom)
- Status bar (separator)
- Context-aware coloring
- Thread-safe operations

---

## Architecture Diagram

```
┌────────────────────────────────────────────────────────┐
│  Terminal Window (ncurses managed)                     │
│                                                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Output Window (scrollable)                      │  │
│  │  ┌────────────────────────────────────────────┐  │  │
│  │  │  OutputBuffer (thread-safe)                │  │  │
│  │  │  - Lines array                             │  │  │
│  │  │  - Metadata (context, timestamp)           │  │  │
│  │  │  - Mutex protection                        │  │  │
│  │  └────────────────────────────────────────────┘  │  │
│  │  Rendering: buffer → ncurses window             │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Status Bar (1 line, fixed)                     │  │
│  │  Left: Mode/State    Right: Info/Stats          │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Input Area (readline managed)                   │  │
│  │  > _                                             │  │
│  │  Features: history, editing, completion          │  │
│  └──────────────────────────────────────────────────┘  │
│                                                         │
└────────────────────────────────────────────────────────┘
```

---

## Component Architecture

### Layer 1: Core Data Structures

```
OutputBuffer
├── lines[]           # Array of output lines
├── metadata[]        # Context, timestamps per line
├── count             # Current line count
├── capacity          # Maximum lines
├── scroll_offset     # Current scroll position
└── mutex             # Thread-safe access

Theme
├── colors[]          # Color pairs for contexts
├── symbols           # UI symbols (prompts, borders)
└── attributes        # Text attributes (bold, dim)

SmartTermContext
├── output_buffer     # Output buffer instance
├── output_win        # ncurses output window
├── status_win        # ncurses status window
├── theme             # Active theme
├── config            # Configuration options
└── state             # Runtime state
```

### Layer 2: Core Functions

```
Initialization & Cleanup
├── smartterm_init()
│   ├── Initialize ncurses
│   ├── Setup colors and windows
│   ├── Create output buffer
│   └── Load configuration
└── smartterm_cleanup()
    ├── Free output buffer
    ├── Cleanup ncurses windows
    └── Restore terminal state

Output Management
├── smartterm_write()
│   ├── Add line to buffer (thread-safe)
│   ├── Apply context/color
│   └── Trigger render
├── smartterm_write_fmt()
│   └── Format string then write
└── smartterm_render()
    ├── Lock buffer
    ├── Render visible lines
    └── Unlock buffer

Input Management
├── smartterm_read_line()
│   ├── Suspend ncurses
│   ├── Call readline
│   ├── Resume ncurses
│   └── Return input
└── smartterm_set_prompt()
    └── Customize prompt string

Status Bar
├── smartterm_status_set()
│   ├── Set left/right text
│   └── Render status bar
└── smartterm_status_update()
    └── Incremental updates

Advanced Features
├── smartterm_scroll()        # Scrollback navigation
├── smartterm_search()        # Search in buffer
├── smartterm_export()        # Export buffer to file
└── smartterm_set_completer() # Tab completion callback
```

### Layer 3: Advanced Features

```
Multi-line Input
├── Custom input handler (not readline)
├── Text buffer with line wrapping
├── Visual feedback during input
└── Submit on double-newline or Ctrl+D

Scrollback Navigation
├── PgUp/PgDn: Scroll by page
├── Ctrl+Home/End: Jump to top/bottom
├── Scroll indicator in status bar
└── Auto-scroll to bottom on new output

Search
├── Ctrl+F: Enter search mode
├── Incremental search in buffer
├── Highlight matches
└── n/N: Next/previous match

Export
├── Write buffer to file
├── Include/exclude metadata
├── Format options (plain, ANSI, markdown)
└── Configurable line range
```

---

## Threading Model

### Thread Safety Guarantees

**OutputBuffer**: Thread-safe
- All write operations protected by mutex
- Multiple threads can safely call `smartterm_write()`
- Render operations acquire read lock

**Input Operations**: Single-threaded
- readline is not thread-safe
- Only call `smartterm_read_line()` from main thread
- Signal handlers should queue writes, not read

**Rendering**: Main thread only
- ncurses is not thread-safe
- All ncurses operations on main thread
- Automatic render triggered by writes (on main thread)

### Usage Pattern

```c
// Worker thread
void* worker_thread(void* arg) {
    smartterm_ctx *ctx = arg;

    while (running) {
        // SAFE: Write from any thread
        smartterm_write(ctx, "Worker output", CTX_NORMAL);

        // UNSAFE: Don't read from worker thread
        // smartterm_read_line(ctx, "> ");  // WRONG!
    }
    return NULL;
}

// Main thread
int main() {
    smartterm_ctx *ctx = smartterm_init(NULL);

    pthread_t worker;
    pthread_create(&worker, NULL, worker_thread, ctx);

    // SAFE: Read from main thread only
    while (running) {
        char *input = smartterm_read_line(ctx, "> ");
        process_command(input);
        free(input);
    }

    pthread_join(worker, NULL);
    smartterm_cleanup(ctx);
}
```

---

## Design Decisions

### 1. Output Buffer Separate from ncurses

**Decision**: Store output in memory array, not ncurses pad

**Rationale**:
- Enables search, export, scrollback
- Thread-safe access with mutex
- Survives terminal resize
- Can format independently of display

**Trade-offs**:
- Memory overhead (1MB for 10k lines)
- Double buffering cost
- More complex than direct ncurses

### 2. readline for Input (Not Custom)

**Decision**: Use readline for single-line input, custom handler for multi-line

**Rationale**:
- readline provides: history, editing, completion (mature, tested)
- Custom multi-line is simpler than expected (just text buffer + display)
- Hybrid approach: readline for single-line, custom for multi-line

**Trade-offs**:
- readline owns terminal (suspend/resume needed)
- Flicker on each input cycle
- Two code paths (single vs multi-line)

### 3. Suspend/Resume ncurses for Input

**Decision**: Temporarily exit ncurses during readline

**Rationale**:
- readline and ncurses both want terminal control
- Suspend/resume is cleanest integration
- Proven by POC to work reliably

**Trade-offs**:
- Visible flicker (minor, acceptable)
- Slightly slower than native
- Could improve with terminal manipulation

### 4. Context as Metadata, Not Part of Text

**Decision**: Store context separately, apply on render

**Rationale**:
- Clean separation of data and presentation
- Easy to change themes
- Supports multiple output formats (ANSI, plain, markdown)
- Searchable text without formatting

**Trade-offs**:
- Slightly more complex storage
- Rendering loop must apply context
- Memory overhead (few bytes per line)

### 5. Theme System with Presets

**Decision**: Configurable themes, multiple built-in presets

**Rationale**:
- Users have different color preferences
- Terminal capabilities vary (256-color vs basic)
- Accessibility needs (high contrast, color blind)
- Easy to add custom themes

**Trade-offs**:
- Theme API complexity
- Testing across themes
- Fallback logic for limited terminals

---

## Performance Considerations

### Memory Usage

**Output Buffer**:
- ~100 bytes per line (text + metadata)
- Default: 1000 lines = ~100KB
- Configurable via `max_lines` option
- Circular buffer (old lines drop)

**ncurses Windows**:
- Output window: ~(width × height × 4 bytes)
- Status window: ~(width × 4 bytes)
- Minimal overhead: ~20-50KB

**Total**: ~150-200KB typical

### CPU Usage

**Rendering**:
- O(visible_lines) per render
- ~20-50 lines typical
- ~0.1ms per render (fast)
- Only render on: write, scroll, resize

**Thread Synchronization**:
- Mutex lock/unlock: ~0.01ms
- Minimal contention (writes rare vs CPU cycles)
- Lock-free reads possible (future optimization)

**Input**:
- readline: ~0.1ms per character
- History search: O(history_size)
- Tab completion: O(completion_candidates)

### Optimization Strategies

1. **Lazy Rendering**: Only render on visible changes
2. **Dirty Regions**: Track changed lines, partial render
3. **Double Buffering**: Prepare render off-screen
4. **Line Pooling**: Reuse line allocations
5. **Lock-free Reads**: Atomic pointers for read path (future)

---

## Error Handling

### Error Reporting

**Return Codes**:
```c
SMARTTERM_OK           = 0    // Success
SMARTTERM_ERROR        = -1   // Generic error
SMARTTERM_NOMEM        = -2   // Out of memory
SMARTTERM_INVALID      = -3   // Invalid argument
SMARTTERM_NOTINIT      = -4   // Not initialized
```

**Error Strings**:
```c
const char* smartterm_error_string(int code);
```

### Error Handling Strategy

1. **Initialization**: Check all allocations, return NULL on failure
2. **Write Operations**: Never fail (queue or drop if full)
3. **Read Operations**: Return NULL on failure, set error code
4. **Cleanup**: Always succeeds, safe to call multiple times

### Robustness

- **Null Checks**: All public functions check for NULL context
- **Bounds Checks**: Array access validated
- **Signal Safety**: Safe to write from signal handlers (async-signal-safe)
- **Terminal State**: Cleanup restores terminal even on crash (atexit handler)

---

## Extending the Library

### Adding New Context Types

```c
// 1. Define context enum value
typedef enum {
    CTX_CUSTOM = 100,  // User-defined contexts start at 100
} ContextType;

// 2. Register with theme
smartterm_theme_set_color(theme, CTX_CUSTOM, COLOR_CYAN, COLOR_BLACK);

// 3. Use in writes
smartterm_write(ctx, "Custom message", CTX_CUSTOM);
```

### Custom Themes

```c
smartterm_theme *theme = smartterm_theme_create("my_theme");
smartterm_theme_set_color(theme, CTX_NORMAL, COLOR_WHITE, COLOR_BLACK);
smartterm_theme_set_color(theme, CTX_ERROR, COLOR_RED, COLOR_BLACK);
smartterm_theme_set_symbol(theme, SYM_PROMPT, "$ ");
smartterm_theme_set_attribute(theme, CTX_NORMAL, A_BOLD);

smartterm_ctx *ctx = smartterm_init(NULL);
smartterm_set_theme(ctx, theme);
```

### Tab Completion

```c
char** my_completer(const char *text, int start, int end) {
    // Return array of completion candidates
    // Last element must be NULL
    return candidates;
}

smartterm_set_completer(ctx, my_completer);
```

### Custom Key Bindings

```c
void my_key_handler(smartterm_ctx *ctx, int key, void *data) {
    if (key == KEY_F1) {
        smartterm_write(ctx, "F1 pressed!", CTX_NORMAL);
    }
}

smartterm_register_key_handler(ctx, KEY_F1, my_key_handler, NULL);
```

---

## Future Enhancements

### Planned Features (v1.x)

- [ ] Mouse support (click to position cursor, select text)
- [ ] Copy/paste integration (clipboard support)
- [ ] Unicode/UTF-8 full support
- [ ] Split windows (multiple output regions)
- [ ] Tabs (multiple sessions in one terminal)

### Under Consideration (v2.x)

- [ ] TUI widgets (buttons, menus, dialogs)
- [ ] Layout engine (flexible window management)
- [ ] Themes: load from file (JSON/TOML)
- [ ] Network transparency (remote sessions)
- [ ] Recording/replay (session logging)

### Not Planned

- ❌ GUI support (use ncurses alternatives)
- ❌ Windows native (use WSL or PDCurses)
- ❌ Async I/O (use threads)
- ❌ Embedded systems (too much memory)

---

## Comparison to Alternatives

### vs Raw ncurses

**Advantages**:
- Higher-level API (less boilerplate)
- Thread-safe output buffer
- readline integration included
- Context-aware coloring built-in

**Disadvantages**:
- Less flexibility (opinionated layout)
- Memory overhead
- Learning curve (new API)

### vs readline Only

**Advantages**:
- Scrollable output history
- Context-aware coloring
- Status bar
- Better visual separation

**Disadvantages**:
- More complex
- ncurses dependency
- Terminal compatibility issues

### vs Haskell Brick

**Advantages**:
- C language (no Haskell dependency)
- Simpler mental model (imperative)
- Smaller binary size
- Easier to embed

**Disadvantages**:
- Less sophisticated (no declarative UI)
- No type safety
- Manual memory management

---

## References

### Dependencies

- **ncurses** (>= 6.0): Terminal UI framework
- **readline** (>= 8.0): Line editing and history
- **pthread** (POSIX): Thread synchronization

### Documentation

- `docs/SMARTTERM-API.md`: Complete API reference
- `examples/`: Example applications
- `README.md`: Quick start guide

### Standards

- **POSIX.1-2001**: Thread safety, signal handling
- **C99**: Language standard
- **XDG Base Directory**: Config file locations

---

## Credits

**Original POC**: SmartTerm proof of concept (2025-11-17)
**Architecture**: Designed for jcaldwell-labs projects
**Inspired by**: readline, rlwrap, rlfe, Haskell Brick
