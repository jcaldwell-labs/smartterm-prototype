# cc-bash Vision: Open Source Warp Alternative

**Goal**: A lightweight, open source terminal wrapper that brings modern AI-assisted shell capabilities to any terminal.

---

## Positioning

### What We Are

- **Terminal wrapper**, not replacement - runs in your existing terminal
- **Two modes**: C (fast, offline) + Python (AI-powered)
- **Open source** (MIT) - transparent, extensible, community-driven
- **Portable** - works anywhere with a C compiler or Python

### What We're Not

- Not a full terminal emulator (use iTerm2, Alacritty, etc.)
- Not trying to replicate every Warp feature
- Not dependent on a single AI provider

---

## Comparison with Warp

| Capability           | Warp               | cc-bash                |
| -------------------- | ------------------ | ---------------------- |
| Block-based output   | Native             | ANSI regions           |
| AI integration       | Proprietary        | Claude SDK (pluggable) |
| Terminal replacement | Full app           | Wrapper                |
| Cross-platform       | macOS + Linux beta | Anywhere               |
| Open source          | No                 | MIT                    |
| Offline mode         | No                 | C version              |
| Custom AI models     | No                 | Planned                |
| Price                | Freemium           | Free                   |

### Our Advantages

1. **No vendor lock-in** - Use any terminal, switch AI providers
2. **Feature-rich** - ~3300 LOC C with aliases, snippets, workflows, plugins, themes
3. **Hackable** - Well-organized codebase, easy to extend
4. **Works everywhere** - SSH sessions, containers, old terminals

---

## Roadmap

### Phase 1: Solid Foundation ✅ COMPLETE

- [x] C implementation with ANSI regions
- [x] Python + Claude SDK implementation
- [x] Basic command execution, history, cd
- [x] Status bar with cwd, exit code
- [x] Notes with `#` prefix
- [x] Internal `@` commands

### Phase 2: Polish & Blockers ✅ COMPLETE

All blockers from issue #14 resolved:

| Issue | Description              | Status                |
| ----- | ------------------------ | --------------------- |
| #15   | Tab completion           | ✅ Complete           |
| #16   | ANSI color passthrough   | ✅ Complete (PTY #22) |
| #17   | `clear` command handling | ✅ Complete           |
| #18   | Output scrollback access | ✅ Complete           |

Additional polish completed:

- [x] Persistent history file (~/.cc-bash-history)
- [x] Exit code display in status bar
- [x] Terminal resize handling
- [x] Configuration file (~/.cc-bashrc)

### Phase 3: Differentiation ✅ COMPLETE

All differentiating features implemented:

| Feature       | Description                               | Status      |
| ------------- | ----------------------------------------- | ----------- |
| `@workflow`   | Save and replay command sequences         | ✅ Complete |
| `@snippet`    | Reusable command templates with variables | ✅ Complete |
| `@alias`      | Custom command shortcuts                  | ✅ Complete |
| Plugin system | Load extensions dynamically               | ✅ Complete |
| Themes        | Customizable colors and status bar        | ✅ Complete |

Additional Phase 3 features:

- [x] @edit - Edit config in $EDITOR (#24)
- [x] @reload - Reload configuration at runtime (#24)
- [x] @alias save - Persist session aliases (#24)

### Phase 4: Advanced 🔄 IN PROGRESS

Long-term features for power users:

| Feature              | Description                                | Status            |
| -------------------- | ------------------------------------------ | ----------------- |
| Command search       | Fuzzy search through history (Ctrl+R)      | ✅ Complete (#23) |
| PTY execution        | Automatic color support for commands       | ✅ Complete (#22) |
| Multi-line input     | Support for multi-line command editing     | 🔄 Open (#25)     |
| Multiple AI backends | Ollama, OpenAI, local models               | ⏳ Planned        |
| Block navigation     | Jump between command blocks (Ctrl+Up/Down) | ⏳ Planned        |
| Shared workflows     | Export/import workflow files               | ⏳ Planned        |
| Tool integrations    | Git status, docker, kubectl in status bar  | ⏳ Planned        |

### Phase 5: Distribution & Community 📋 PLANNED

| Goal               | Description                    | Status      |
| ------------------ | ------------------------------ | ----------- |
| Homebrew formula   | `brew install cc-bash`         | ⏳ Planned  |
| APT package        | Debian/Ubuntu package          | ⏳ Planned  |
| AUR package        | Arch Linux package             | ⏳ Planned  |
| GitHub Releases    | Pre-built binaries             | ✅ Complete |
| Documentation site | GitHub Pages or dedicated docs | ⏳ Planned  |

---

## Design Principles

### 1. Organized Complexity

The codebase has grown to ~3300 LOC but remains well-organized with clear section headers. Each major feature (input, execution, plugins, etc.) is self-contained.

### 2. Progressive Enhancement

Start with basic bash wrapper, add AI and advanced features as opt-in layers.

### 3. No Magic

Users should understand what's happening. Don't hide complexity, make it optional.

### 4. Respect the Terminal

Work with existing terminal semantics, don't fight them. ANSI codes over ncurses.

### 5. Offline by Default

The C version should work without network. AI features are in the Python version.

---

## Target Users

### Primary: Developers who want AI assistance

- Already using terminal daily
- Want Claude-like AI help without leaving terminal
- Don't want to switch terminal emulators

### Secondary: Terminal power users

- Want better UX than raw bash
- Appreciate blocks/regions concept
- Want customization without bloat

### Tertiary: Teams/Organizations

- Want open source alternative to Warp
- Need control over AI provider
- Want to extend for internal tools

---

## Success Metrics

### Short-term (3 months) - Progress

- [x] All Phase 2 blockers resolved ✅
- [x] Phase 3 features complete ✅
- [ ] 100+ GitHub stars
- [ ] 5+ contributors
- [ ] Published to package managers (brew, apt)

### Medium-term (6 months)

- [ ] Active community (Discord/discussions)
- [ ] Featured in developer blogs/newsletters
- [ ] Multiple AI backend support
- [ ] Multi-line input support (#25)

### Long-term (12 months)

- [ ] 1000+ GitHub stars
- [ ] Phase 4 & 5 features complete
- [ ] Plugin ecosystem with 10+ community plugins
- [ ] Corporate adoption stories

---

## Non-Goals

Things we explicitly won't do:

1. **GPU-accelerated rendering** - We're a wrapper, not an emulator
2. **Teams/collaboration features** - Keep it simple, let users share files
3. **Web interface** - Terminal only
4. **Mobile apps** - Desktop terminal focus
5. **Paid tiers** - Stay open source

---

## How to Contribute

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Immediate needs:**

- Multi-line input support (#25)
- Package manager distribution (brew, apt, AUR)
- Test on different terminals/platforms
- Multiple AI backend support (Ollama, OpenAI)
- Community building and outreach

---

## Related Documents

- [README.md](README.md) - User documentation
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [GitHub Issues](https://github.com/jcaldwell-labs/smartterm-prototype/issues) - Backlog
- [Issue #14](https://github.com/jcaldwell-labs/smartterm-prototype/issues/14) - Original cc-bash proposal

---

_Last updated: 2026-01-07_
