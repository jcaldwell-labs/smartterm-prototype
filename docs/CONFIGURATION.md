# cc-bash Configuration Guide

Complete guide to configuring and customizing cc-bash.

## Table of Contents

- [Configuration File Location](#configuration-file-location)
- [Configuration Syntax](#configuration-syntax)
- [Aliases](#aliases)
- [Snippets](#snippets)
- [Workflows](#workflows)
- [Themes](#themes)
- [Plugins](#plugins)
- [Environment Variables](#environment-variables)
- [Advanced Configuration](#advanced-configuration)
- [Examples](#examples)

---

## Configuration File Location

cc-bash uses `~/.cc-bashrc` for user configuration.

**Creating your config**:

```bash
# Copy template (recommended)
cp /path/to/cc-bashrc.template ~/.cc-bashrc

# Or create from scratch
touch ~/.cc-bashrc
```

**Editing and reloading**:

```bash
# Quick edit (uses $EDITOR)
$ @edit

# Reload after changes
$ @reload
```

---

## Configuration Syntax

The config file uses a simple key-value format:

```bash
# Comments start with #
# Blank lines are ignored

# Aliases
alias name='command'

# Snippets (with parameters)
snippet name='command with $1 $2'

# Workflows (multi-step)
workflow name='cmd1 && cmd2'

# Theme colors
theme.element=color

# Environment variables
export VAR=value
```

---

## Aliases

Aliases are command shortcuts that expand when you type them.

### Basic Syntax

```bash
alias name='command'
```

### Examples

```bash
# Common shortcuts
alias ll='ls -la'
alias la='ls -A'
alias l='ls -CF'

# Git aliases
alias gs='git status'
alias ga='git add'
alias gc='git commit'
alias gp='git push'
alias gl='git pull'
alias gd='git diff'
alias gb='git branch'
alias gco='git checkout'

# Directory navigation
alias ..='cd ..'
alias ...='cd ../..'
alias ....='cd ../../..'

# System admin
alias ports='netstat -tulanp'
alias meminfo='free -m -l -t'
alias psmem='ps auxf | sort -nr -k 4'
alias pscpu='ps auxf | sort -nr -k 3'

# Safety aliases
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'

# Utility
alias grep='grep --color=auto'
alias egrep='egrep --color=auto'
alias fgrep='fgrep --color=auto'
```

### Usage

```bash
# List all aliases
$ @alias

# Add session alias (not saved to config)
$ @alias tmp='echo temporary'

# Save session aliases to config
$ @alias save

# Use an alias
$ ll
```

### Alias Expansion Rules

- Aliases expand before command execution
- Aliases can call other aliases
- Use quotes for commands with spaces
- Session aliases marked with `*` in `@alias` output

---

## Snippets

Snippets are parameterized command templates. Use `$1`, `$2`, etc. for parameters.

### Basic Syntax

```bash
snippet name='command with $1 $2'
```

### Examples

```bash
# File operations
snippet find-name='find . -name "$1"'
snippet find-type='find . -type $1'
snippet grep-r='grep -r "$1" .'
snippet grep-files='grep -r "$1" --include="*.$2" .'

# Directory operations
snippet mkdir-cd='mkdir -p $1 && cd $1'
snippet backup='cp -r $1 $1.backup.$(date +%Y%m%d)'

# Archive operations
snippet extract-tar='tar -xzvf $1'
snippet compress-tar='tar -czvf $1.tar.gz $1'
snippet extract-zip='unzip $1'
snippet compress-zip='zip -r $1.zip $1'

# Development
snippet serve='python3 -m http.server $1'
snippet watch='watch -n $1 "$2"'
snippet docker-exec='docker exec -it $1 bash'
snippet k8s-logs='kubectl logs -f $1'

# Git operations
snippet git-clone-depth='git clone --depth $1 $2'
snippet git-branch-delete='git branch -d $1 && git push origin --delete $1'

# System info
snippet disk-usage='du -sh $1 | sort -hr | head -n $1'
snippet process-grep='ps aux | grep $1'
```

### Usage

```bash
# List all snippets
$ @snippet

# Run snippet with arguments
$ @snippet find-name "*.c"
$ @snippet mkdir-cd my-new-dir
$ @snippet grep-files "TODO" "py"

# Multiple arguments
$ @snippet git-clone-depth 1 https://github.com/user/repo.git
```

### Parameter Rules

- `$1`, `$2`, ... `$9` for positional parameters
- Use quotes around `$N` for parameters with spaces: `"$1"`
- Missing parameters are replaced with empty string
- Snippet names cannot contain spaces

---

## Workflows

Workflows are multi-step command sequences.

### Basic Syntax

```bash
workflow name='cmd1 && cmd2 ; cmd3'
```

### Operators

- `&&` - Execute next command only if previous succeeded (exit code 0)
- `;` - Always execute next command (ignore previous exit code)
- `||` - Execute next command only if previous failed (exit code != 0)

### Examples

```bash
# Build workflows
workflow build='make clean && make && make test'
workflow build-release='make clean && make CFLAGS="-O2 -DNDEBUG" && strip cc-bash'
workflow build-debug='make clean && make debug && gdb cc-bash'

# Git workflows
workflow gitcheck='git status; git diff --stat'
workflow gitpush='git add -A && git commit && git push'
workflow gitsync='git pull && git push'
workflow git-update='git fetch --all && git pull --rebase'

# Development workflows
workflow test-all='make clean && make && make test && make lint'
workflow deploy='make clean && make release && scp cc-bash server:/usr/local/bin/'
workflow docs='make clean && make && ./generate-docs.sh && open docs/index.html'

# System maintenance
workflow update='sudo apt update && sudo apt upgrade -y && sudo apt autoremove -y'
workflow cleanup='make clean; rm -f *.log; rm -rf tmp/'
workflow backup-home='tar -czf ~/backup-$(date +%Y%m%d).tar.gz ~/ --exclude=node_modules'

# Docker workflows
workflow docker-build='docker build -t myapp . && docker run -p 8080:8080 myapp'
workflow docker-clean='docker stop $(docker ps -aq) ; docker rm $(docker ps -aq)'
```

### Usage

```bash
# List all workflows
$ @workflow

# Run workflow
$ @workflow build

# Preview workflow (dry-run)
$ @workflow build --dry-run

# See commands that will execute
$ @workflow gitpush --dry-run
```

### Workflow Best Practices

- Use `&&` for critical sequences (build, test, deploy)
- Use `;` for independent commands (status checks)
- Keep workflows focused (single purpose)
- Use descriptive names
- Test with `--dry-run` first

---

## Themes

Customize cc-bash colors with theme settings.

### Available Colors

**Base colors**: `black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`

**Modifiers**: `bold`, `dim`

### Theme Elements

```bash
theme.prompt=color       # Command prompt color
theme.error=color        # Error message color
theme.comment=color      # Comment/note color
theme.dim=color          # Dim text color
theme.header=color       # Header text color
theme.status=color       # Status bar color
theme.scroll=color       # Scrollback indicator
```

### Example Themes

**Default theme**:

```bash
theme.prompt=bold cyan
theme.error=bold red
theme.comment=green
theme.dim=dim
theme.header=bold white
theme.status=bold
theme.scroll=dim
```

**Solarized Dark**:

```bash
theme.prompt=cyan
theme.error=red
theme.comment=yellow
theme.dim=dim white
theme.header=bold cyan
theme.status=bold white
theme.scroll=dim cyan
```

**Gruvbox**:

```bash
theme.prompt=bold yellow
theme.error=bold red
theme.comment=bold green
theme.dim=dim white
theme.header=bold yellow
theme.status=bold white
theme.scroll=dim yellow
```

**Nord**:

```bash
theme.prompt=bold cyan
theme.error=red
theme.comment=bold blue
theme.dim=dim white
theme.header=bold white
theme.status=bold cyan
theme.scroll=dim cyan
```

### Usage

```bash
# Display current theme
$ @theme

# Edit theme in config
$ @edit
# (modify theme.* lines)

# Reload to apply
$ @reload
```

---

## Plugins

Plugins extend cc-bash with custom hooks and commands.

### Plugin Directory Structure

```
~/.cc-bash/plugins/<plugin-name>/
├── plugin.conf          # Required: plugin manifest
├── config.conf          # Optional: additional aliases/snippets/workflows
└── hooks/               # Optional: event hook scripts
    ├── on_startup.sh
    ├── on_cd.sh
    ├── on_pre_command.sh
    └── on_post_command.sh
```

### Plugin Manifest (plugin.conf)

```ini
name=my-plugin
version=1.0.0
description=My custom plugin
author=Your Name

# Hook definitions
hook.startup=hooks/on_startup.sh
hook.shutdown=hooks/on_shutdown.sh
hook.cd=hooks/on_cd.sh
hook.pre_command=hooks/on_pre_command.sh
hook.post_command=hooks/on_post_command.sh
```

### Available Hooks

| Hook           | Trigger                     | Environment Variables                           |
| -------------- | --------------------------- | ----------------------------------------------- |
| `startup`      | cc-bash starts              | None                                            |
| `shutdown`     | cc-bash exits               | None                                            |
| `pre_command`  | Before command executes     | `CCBASH_COMMAND`                                |
| `post_command` | After command completes     | `CCBASH_COMMAND`, `CCBASH_EXIT_CODE`            |
| `cd`           | Directory changes           | `CCBASH_OLD_CWD`, `CCBASH_NEW_CWD`              |

### Example Plugin: Directory Logger

**plugin.conf**:

```ini
name=dirlog
version=1.0.0
description=Log directory changes
hook.cd=hooks/on_cd.sh
```

**hooks/on_cd.sh**:

```bash
#!/bin/bash
# Log directory changes with timestamp
echo "[$(date '+%Y-%m-%d %H:%M:%S')] cd: $CCBASH_OLD_CWD -> $CCBASH_NEW_CWD" >> ~/.cc-bash/cd.log
```

### Example Plugin: Command Timer

**plugin.conf**:

```ini
name=timer
version=1.0.0
description=Time command execution
hook.pre_command=hooks/on_pre_command.sh
hook.post_command=hooks/on_post_command.sh
```

**hooks/on_pre_command.sh**:

```bash
#!/bin/bash
echo $(date +%s) > /tmp/ccbash_timer_start
```

**hooks/on_post_command.sh**:

```bash
#!/bin/bash
if [ -f /tmp/ccbash_timer_start ]; then
    start=$(cat /tmp/ccbash_timer_start)
    end=$(date +%s)
    duration=$((end - start))
    if [ $duration -gt 5 ]; then
        echo "Command took ${duration}s"
    fi
    rm /tmp/ccbash_timer_start
fi
```

### Usage

```bash
# List loaded plugins
$ @plugins

# List plugin hooks
$ @hooks
```

---

## Environment Variables

Set environment variables in `~/.cc-bashrc`:

```bash
# Editor preference
export EDITOR=vim
export VISUAL=vim

# Pager
export PAGER=less
export LESS='-R -F -X'

# History
export HISTSIZE=10000
export HISTFILESIZE=20000

# Paths
export PATH="$HOME/.local/bin:$PATH"
export LD_LIBRARY_PATH="$HOME/.local/lib:$LD_LIBRARY_PATH"

# Development
export CC=gcc
export CFLAGS="-Wall -Wextra -O2"

# Colors for ls
export CLICOLOR=1
export LSCOLORS=GxFxCxDxBxegedabagaced

# GPG/SSH
export GPG_TTY=$(tty)
```

---

## Advanced Configuration

### Conditional Configuration

Use shell conditionals in `~/.cc-bashrc`:

```bash
# OS-specific aliases
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    alias open='xdg-open'
elif [[ "$OSTYPE" == "darwin"* ]]; then
    alias ports='lsof -PiTCP -sTCP:LISTEN'
fi

# Hostname-specific
if [[ "$HOSTNAME" == "work-laptop" ]]; then
    alias vpn='sudo openvpn /etc/openvpn/work.conf'
fi

# Load work aliases only on work machine
if [[ -f ~/.cc-bashrc.work ]]; then
    source ~/.cc-bashrc.work
fi
```

### Multiple Config Files

```bash
# In ~/.cc-bashrc
source ~/.cc-bashrc.aliases
source ~/.cc-bashrc.work
source ~/.cc-bashrc.personal
```

### Dynamic Aliases

```bash
# Alias that changes based on directory
if [[ $(pwd) =~ "project-a" ]]; then
    alias test='make test-all'
else
    alias test='pytest'
fi
```

---

## Examples

### Developer Setup

```bash
# Git shortcuts
alias gs='git status'
alias ga='git add'
alias gc='git commit -m'
alias gp='git push'
alias gl='git log --oneline --graph'

# Development snippets
snippet serve='python3 -m http.server $1'
snippet clone='git clone $1 && cd $(basename $1 .git)'
snippet new-branch='git checkout -b $1'

# Build workflows
workflow dev='make clean && make debug && ./cc-bash'
workflow test='make clean && make test'
workflow deploy='make release && scp cc-bash server:/opt/bin/'

# Theme
theme.prompt=bold green
theme.error=bold red
theme.comment=yellow
```

### System Admin Setup

```bash
# System aliases
alias update='sudo apt update && sudo apt upgrade'
alias ports='sudo netstat -tulpn'
alias logs='journalctl -xe'

# Admin snippets
snippet docker-logs='docker logs -f $1'
snippet service-status='systemctl status $1'
snippet disk-usage='du -sh $1 | sort -hr | head -20'

# Maintenance workflows
workflow backup='tar -czf backup-$(date +%Y%m%d).tar.gz /etc /home'
workflow cleanup='apt autoremove && apt autoclean'
```

---

## See Also

- [README.md](../README.md) - Main documentation
- [CONTRIBUTING.md](../CONTRIBUTING.md) - Development guide
- [gallery/architecture.txt](../gallery/architecture.txt) - Technical architecture
