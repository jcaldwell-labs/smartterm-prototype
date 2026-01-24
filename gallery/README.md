# cc-bash Media Gallery

This directory contains visual assets for documentation and promotion.

## Overview

Visual documentation dramatically improves project engagement and helps newcomers understand cc-bash quickly.

## Asset Index

### Hero Media

- **`hero.gif`** - Primary demo showing basic command execution with colored output
  - Duration: ~5 seconds
  - Shows: Command input, colored output, status bar
  - Terminal: 80x24 or 120x30
  - File size: < 2MB

### Feature Demonstrations

- **`tab-completion.gif`** - Tab completion in action
  - Shows: Double-tab for options, file/command completion
- **`ctrl-r-search.gif`** - Ctrl+R fuzzy history search
  - Shows: Search activation, typing filter, selecting result
- **`multi-line-input.gif`** - Multi-line input with continuation
  - Shows: Entering multi-line commands with proper continuation
- **`theme-customization.gif`** - Theme customization
  - Shows: @theme command, editing config, @reload to apply
- **`workflow-execution.gif`** - Workflow execution
  - Shows: @workflow command, listing workflows, running one

### Screenshots

- **`default-theme.png`** - Default color theme
- **`status-bar.png`** - Status bar detail view
- **`error-output.png`** - Error output showing red stderr
- **`config-example.png`** - Configuration file example

### Architecture

- **`architecture.png`** or **`architecture.txt`** - Three-region layout diagram
  - Output area (scrolling)
  - Prompt area (fixed)
  - Status bar (fixed)

## Recording Guidelines

When creating new media assets:

### Terminal Configuration

- **Size**: Use 80x24 or 120x30 for consistency
- **Colors**: Use default theme or a high-contrast theme
- **Font**: Monospace font, 12-14pt size

### GIF Recording

- **Tools**: Use asciinema + asciicast2gif, or terminalizer, or VHS
- **Duration**: Keep under 10 seconds for GIFs
- **File size**: Optimize to < 2MB (use gifsicle or similar)
- **Frame rate**: 10-15 FPS is sufficient

### Screenshot Capture

- **Format**: PNG preferred for screenshots
- **Resolution**: Full terminal window, no unnecessary chrome
- **Annotations**: Avoid unless necessary for clarity

## Tools for Recording

### Recommended Tools

1. **[VHS](https://github.com/charmbracelet/vhs)** - Generate terminal GIFs from code

   ```bash
   # Install
   brew install vhs

   # Record with script
   vhs demo.tape
   ```

2. **[asciinema](https://asciinema.org/)** - Record terminal sessions

   ```bash
   # Install
   pip install asciinema

   # Record
   asciinema rec demo.cast

   # Convert to GIF with agg
   cargo install --git https://github.com/asciinema/agg
   agg demo.cast demo.gif
   ```

3. **[Terminalizer](https://terminalizer.com/)** - Record and share terminal sessions
   ```bash
   npm install -g terminalizer
   terminalizer record demo
   terminalizer render demo
   ```

### GIF Optimization

```bash
# Optimize with gifsicle
gifsicle -O3 --colors 256 input.gif -o output.gif

# Further compression
gifsicle -O3 --lossy=80 input.gif -o output.gif
```

## Usage in Documentation

### In README.md

```markdown
![cc-bash demo](gallery/hero.gif)
```

### In GitHub Issue/PR

```markdown
![Feature demo](../gallery/tab-completion.gif?raw=true)
```

## Contributing Media

If you'd like to contribute media assets:

1. Follow the recording guidelines above
2. Optimize file sizes
3. Name files descriptively
4. Update this README with the new asset
5. Submit a PR with the media file

## Status

**Current Status**: Core demos available

**Available Assets**:

- [x] `cc-bash-demo.gif` - Basic command execution demo (~418KB)
- [x] `cc-bash-showcase.gif` - Feature showcase (~1.8MB)

**TODO**:

- [ ] Record additional feature GIFs (tab completion, Ctrl+R search)
- [ ] Capture screenshots (4 total)
- [ ] Create architecture diagram image
- [ ] Optimize showcase GIF for file size

## License

Media assets in this directory are licensed under the same MIT license as the project.
