# SmartTerm Documentation

Welcome to the SmartTerm documentation! This directory contains guides, tutorials, and examples to help you build terminal applications with SmartTerm.

---

## Documentation Structure

### Getting Started

- **[Main README](../README.md)** - Project overview, installation, and quick start
- **[API Reference](SMARTTERM-API.md)** - Complete function documentation
- **[Architecture](ARCHITECTURE.md)** - Design decisions and internals

### User Guides

- **[guides/](guides/)** - How-to guides for common tasks
  - Getting started with SmartTerm
  - Configuring themes and colors
  - Thread-safe output patterns

### Tutorials

- **[tutorials/](tutorials/)** - Step-by-step tutorials
  - Building a REPL from scratch
  - Creating a log viewer
  - Implementing custom key handlers

### Examples

- **[examples/](../examples/)** - Working example applications
  - `repl.c` - Calculator REPL with expression evaluation
  - `chat_client.c` - IRC-style chat simulation
  - `log_viewer.c` - Real-time log monitoring with search

---

## Navigation Guide

### New to SmartTerm?

1. Start with the **[Main README](../README.md)** for installation
2. Try the **Hello World** example in the Quick Start section
3. Run the example applications to see SmartTerm in action
4. Read the **[API Reference](SMARTTERM-API.md)** for detailed documentation

### Building Something Specific?

| Goal | Start Here |
|------|------------|
| Interactive REPL | [examples/repl.c](../examples/repl.c) |
| Log monitoring | [examples/log_viewer.c](../examples/log_viewer.c) |
| Chat interface | [examples/chat_client.c](../examples/chat_client.c) |
| Custom colors | [API Reference - Themes](SMARTTERM-API.md#themes) |
| Export output | [API Reference - Export](SMARTTERM-API.md#export) |

### Looking for Reference?

- **All functions** - [API Reference](SMARTTERM-API.md)
- **Architecture** - [Architecture Guide](ARCHITECTURE.md)
- **Context types** - [Main README - Context Types](../README.md#context-types)
- **Comparison** - [Main README - Comparison](../README.md#comparison)

---

## Quick Reference

### Initialization

```c
#include <smartterm.h>

// Default config
smartterm_ctx *ctx = smartterm_init(NULL);

// Custom config
smartterm_config_t config = smartterm_default_config();
config.max_lines = 5000;
smartterm_ctx *ctx = smartterm_init(&config);
```

### Output

```c
smartterm_write(ctx, "Message", CTX_INFO);
smartterm_write_fmt(ctx, CTX_ERROR, "Error: %s", msg);
```

### Input

```c
char *input = smartterm_read_line(ctx, "> ");
// use input
free(input);
```

### Status Bar

```c
smartterm_status_set(ctx, "AppName", "Status Text");
```

### Cleanup

```c
smartterm_cleanup(ctx);
```

---

## Contributing Documentation

Found a typo? Want to add a tutorial? Contributions are welcome!

1. Documentation files are in `docs/`
2. Use clear headings and code examples
3. Add your document to this index
4. Submit a PR with your changes

---

## Need Help?

- **Issues:** [GitHub Issues](https://github.com/jcaldwell-labs/smartterm-prototype/issues)
- **API Docs:** [SMARTTERM-API.md](SMARTTERM-API.md)
- **Examples:** [examples/](../examples/)

---

**[Back to Main README](../README.md)**
