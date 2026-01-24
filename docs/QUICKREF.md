# cc-bash Quick Reference

One-page cheat sheet for cc-bash commands and shortcuts.

## Installation

```bash
git clone https://github.com/jcaldwell-labs/smartterm-prototype.git
cd smartterm-prototype && make && sudo make install
```

## Starting

```bash
cc-bash          # Start cc-bash
```

## Keyboard Shortcuts

| Key          | Action                        |
| ------------ | ----------------------------- |
| `Ctrl+R`     | Fuzzy history search          |
| `Ctrl+C`     | Cancel input                  |
| `Ctrl+D`     | Exit cc-bash                  |
| `Up/Down`    | Navigate history              |
| `Left/Right` | Move cursor                   |
| `Tab`        | Command/file completion       |
| `Esc`        | Cancel search                 |

## @ Commands

| Command                | Action                          |
| ---------------------- | ------------------------------- |
| `@help` / `@h`         | Show help                       |
| `@clear` / `@c`        | Clear screen                    |
| `@quit` / `@q`         | Exit cc-bash                    |
| `@edit` / `@e`         | Edit config in $EDITOR          |
| `@reload` / `@r`       | Reload config                   |
| `@alias`               | List aliases                    |
| `@alias name=cmd`      | Add session alias               |
| `@alias save`          | Save session aliases to config  |
| `@snippet [name ...]`  | List/run snippets               |
| `@workflow [name]`     | List/run workflows              |
| `@theme`               | Show current theme              |
| `@hooks`               | List plugin hooks               |
| `@plugins`             | List loaded plugins             |

## Configuration (~/.cc-bashrc)

### Aliases

```bash
alias ll='ls -la'
alias gs='git status'
```

Usage: `ll` → executes `ls -la`

### Snippets

```bash
snippet find-name='find . -name "$1"'
snippet grep-r='grep -r "$1" .'
```

Usage: `@snippet find-name "*.c"` → `find . -name "*.c"`

### Workflows

```bash
workflow build='make clean && make && make test'
workflow deploy='make release && scp cc-bash server:/usr/local/bin/'
```

Usage: `@workflow build` → runs entire sequence

### Themes

```bash
theme.prompt=bold cyan
theme.error=bold red
theme.comment=green
```

## Special Prefixes

| Prefix | Behavior                               |
| ------ | -------------------------------------- |
| `#`    | Comment (displayed in yellow, not run) |
| `@`    | Internal command                       |
| (none) | Bash command (default)                 |

## Status Bar

```
 ~/path/to/directory                [exit: 0] 14:30:05
───────────────────────────────────────────────────────
```

- Left: Current working directory
- Right: Last exit code, current time

## Examples

```bash
# Basic usage
$ ls -la
$ pwd
$ echo "Hello"

# With notes
$ # This is a note - won't execute

# Using aliases
$ @alias ll='ls -la'
$ ll

# Using snippets
$ @snippet find-name "*.c"

# Using workflows
$ @workflow build
```

## Development

```bash
make              # Build
make test         # Run tests
make check        # Full quality check
make install      # Install (requires sudo)
make help         # Show all targets
```

## Getting Help

- `@help` - Built-in help
- `make help` - Build system help
- [README.md](../README.md) - Full documentation
- [docs/CONFIGURATION.md](../docs/CONFIGURATION.md) - Configuration guide
- [SUPPORT.md](../SUPPORT.md) - Troubleshooting
- [GitHub Issues](https://github.com/jcaldwell-labs/smartterm-prototype/issues)

## Quick Tips

1. **Edit config**: `@edit` then `@reload` to apply
2. **Save aliases**: Add aliases in session, then `@alias save`
3. **Preview workflows**: Use `@workflow name --dry-run`
4. **Search history**: `Ctrl+R` then type to filter
5. **Complete commands**: Press `Tab` (press twice for options)

---

**Print this page for reference while learning cc-bash!**
