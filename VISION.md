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
2. **Lightweight** - ~350 LOC C, ~280 LOC Python
3. **Hackable** - Simple codebase, easy to extend
4. **Works everywhere** - SSH sessions, containers, old terminals

---

## Roadmap

### Phase 1: Solid Foundation (Complete)

- [x] C implementation with ANSI regions
- [x] Python + Claude SDK implementation
- [x] Basic command execution, history, cd
- [x] Status bar with cwd, exit code
- [x] Notes with `#` prefix
- [x] Internal `@` commands

### Phase 2: Polish & Blockers (~2-3 weeks)

Fix the known limitations from issue #14:

| Issue | Description              | Priority |
| ----- | ------------------------ | -------- |
| #15   | Tab completion           | High     |
| #16   | ANSI color passthrough   | High     |
| #17   | `clear` command handling | Medium   |
| #18   | Output scrollback access | Medium   |

Additional polish:

- [ ] Persistent history file
- [ ] Exit code display in prompt
- [ ] Better resize handling
- [ ] Configuration file (~/.cc-bashrc)

### Phase 3: Differentiation (~4-6 weeks)

Features that set us apart:

| Feature       | Description                               |
| ------------- | ----------------------------------------- |
| `@workflow`   | Save and replay command sequences         |
| `@snippet`    | Reusable command templates with variables |
| `@alias`      | Custom command shortcuts                  |
| Plugin system | Load extensions dynamically (#8)          |
| Themes        | Customizable colors and status bar (#11)  |

### Phase 4: Advanced (~8+ weeks)

Long-term features for power users:

| Feature              | Description                                |
| -------------------- | ------------------------------------------ |
| Multiple AI backends | Ollama, OpenAI, local models               |
| Block navigation     | Jump between command blocks (Ctrl+Up/Down) |
| Command search       | Fuzzy search through history               |
| Shared workflows     | Export/import workflow files               |
| Tool integrations    | Git status, docker, kubectl in status bar  |

---

## Design Principles

### 1. Simplicity First

Keep the core small. cc-bash should remain understandable by reading the source in one sitting (~300-500 LOC).

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

### Short-term (3 months)

- [ ] All Phase 2 blockers resolved
- [ ] 100+ GitHub stars
- [ ] 5+ contributors
- [ ] Published to package managers (brew, apt)

### Medium-term (6 months)

- [ ] Phase 3 features complete
- [ ] Active community (Discord/discussions)
- [ ] Featured in developer blogs/newsletters
- [ ] Multiple AI backend support

### Long-term (12 months)

- [ ] 1000+ GitHub stars
- [ ] Phase 4 features in progress
- [ ] Plugin ecosystem with 10+ plugins
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

- Fix blockers #15-18
- Test on different terminals/platforms
- Documentation improvements
- Plugin system design (#8)

---

## Related Documents

- [README.md](README.md) - User documentation
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [GitHub Issues](https://github.com/jcaldwell-labs/smartterm-prototype/issues) - Backlog
- [Issue #14](https://github.com/jcaldwell-labs/smartterm-prototype/issues/14) - Original cc-bash proposal

---

_Last updated: 2026-01-06_
