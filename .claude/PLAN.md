# cc-bash Development Plan

> Running plan tracking decisions, completed work, and next actions.
> Updated each session.

**Last Updated**: 2026-01-08
**Current Focus**: Multi-line input support (#25)

---

## Session Log

### 2026-01-08: Documentation & Packaging

**Completed**:

- ✅ CLAUDE.md rewritten for cc-bash project
- ✅ VISION.md roadmap updated with completed milestones
- ✅ Homebrew formula created (`Formula/cc-bash.rb`)
- ✅ AUR PKGBUILD created (`packaging/arch/PKGBUILD`)
- ✅ Debian packaging created (`packaging/debian/`)
- ✅ v1.2.0 released with packaging support

**Decisions**:

- Homebrew tap repo creation deferred to future session
- AUR submission deferred to future session
- Focus shifted to #25 (multi-line input) as priority feature

---

## Current Sprint: Multi-line Input (#25)

### Problem

cc-bash only supports single-line input. Users cannot:

- Enter multi-line commands naturally
- Edit previous lines before submission
- Paste multi-line content

### Options Considered

| Option | Approach                | Pros                 | Cons                          |
| ------ | ----------------------- | -------------------- | ----------------------------- |
| **A**  | Shift+Enter for newline | Simple, familiar     | May not work in all terminals |
| **B**  | Continuation detection  | Automatic, bash-like | Complex parsing               |
| **C**  | Replace readline        | Full control         | Major refactor                |

### Decision

**Option B: Continuation Detection** - Bash-like automatic continuation.

Rationale:

- Shift+Enter not reliably detected across terminals
- Continuation detection works everywhere
- Familiar bash behavior (trailing `\`, unclosed quotes)
- No major refactoring needed

### Tasks

- [ ] Research Shift+Enter detection in raw terminal mode
- [ ] Prototype continuation detection for trailing `\`
- [ ] Design visual expansion of input area
- [ ] Implement chosen approach
- [ ] Update history to handle multi-line entries
- [ ] Test across terminals (xterm, iTerm2, Windows Terminal)

---

## Backlog

### Phase 4: Advanced Features

| Item                 | Issue | Priority | Notes                             |
| -------------------- | ----- | -------- | --------------------------------- |
| Multi-line input     | #25   | **HIGH** | Current focus                     |
| Multiple AI backends | -     | Medium   | Ollama, OpenAI for Python version |
| Block navigation     | -     | Low      | Ctrl+Up/Down to jump blocks       |
| Shared workflows     | -     | Low      | Export/import workflow files      |
| Tool integrations    | -     | Low      | Git, docker in status bar         |

### Phase 5: Distribution

| Item               | Status  | Notes                          |
| ------------------ | ------- | ------------------------------ |
| Homebrew tap       | Ready   | Create `homebrew-cc-bash` repo |
| AUR submission     | Ready   | Submit PKGBUILD to AUR         |
| Debian PPA         | Ready   | Set up Launchpad PPA           |
| Documentation site | Planned | GitHub Pages                   |

### Success Metrics

| Metric               | Target | Current |
| -------------------- | ------ | ------- |
| Phase 4 features     | 7/7    | 2/7     |
| Phase 5 distribution | 5/5    | 1/5     |
| GitHub stars         | 100+   | TBD     |
| Contributors         | 5+     | TBD     |

---

## Quick Reference

### Build & Test

```bash
make              # Build cc-bash
make test         # Run tests
make run          # Build and run
```

### Key Files for #25

```
cc-bash.c         # Main implementation
  Lines 600-1000  # Input handling
  Lines 400-600   # History management
```

### Git Workflow

```bash
git checkout -b feature/multi-line-input
# ... work ...
git push -u origin feature/multi-line-input
gh pr create
```

---

_This plan is updated each session. See VISION.md for long-term roadmap._
