# Media Assets - Coming Soon!

The gallery currently contains:
- ✅ Architecture diagram (text version)
- ✅ Recording guidelines
- ⏳ Visual media assets (GIFs, screenshots)

## Why No Media Yet?

This project polish effort is being done in a CI/headless environment without graphical terminal capabilities. Creating high-quality terminal recordings requires:

1. **Graphical terminal emulator** (iTerm2, GNOME Terminal, etc.)
2. **Recording tools** (VHS, asciinema, or similar)
3. **Manual operation** to demonstrate features naturally

## You Can Help!

If you'd like to contribute media assets:

1. **Check out the recording guidelines** in [README.md](README.md)
2. **Record demos** of cc-bash features
3. **Optimize file sizes** (< 2MB per GIF)
4. **Submit a PR** with your media

### Priority Assets

Most valuable contributions:

1. **Hero GIF** (~5 seconds) - Basic command execution showing:
   - Colored output
   - Status bar
   - Command echo

2. **Feature GIFs** - One per feature:
   - Tab completion in action
   - Ctrl+R fuzzy history search
   - Theme customization (@theme, @edit, @reload)
   - Workflow execution

3. **Screenshots** - Static images showing:
   - Default theme appearance
   - Error output (red stderr)
   - Configuration file example

## Recording Tips

### Using VHS (Recommended)

VHS generates GIFs from script files, ensuring consistent, reproducible recordings:

```bash
# Install VHS
brew install vhs

# Create a .tape file (see examples in recording-scripts/)
# Run VHS
vhs demo.tape
```

### Using asciinema + agg

```bash
# Install
pip install asciinema
cargo install --git https://github.com/asciinema/agg

# Record
asciinema rec demo.cast

# Convert to GIF
agg demo.cast demo.gif --speed 1.5
```

## Questions?

Open an issue or discussion on GitHub. We appreciate your help in making cc-bash more visually appealing!
