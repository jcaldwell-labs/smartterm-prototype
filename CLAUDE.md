# CLAUDE.md - AI Assistant Guide for SmartTerm Prototype

> **Purpose**: This document provides AI assistants with comprehensive context about the SmartTerm POC codebase, its architecture, development workflows, and conventions.

**Last Updated**: 2025-11-17
**Project Status**: POC Complete, Decision Made (Path B→A)

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Codebase Structure](#codebase-structure)
3. [Architecture & Design](#architecture--design)
4. [Development Workflows](#development-workflows)
5. [Key Conventions](#key-conventions)
6. [Implementation Decision](#implementation-decision)
7. [Common Tasks](#common-tasks)
8. [Important Context](#important-context)
9. [References](#references)

---

## Project Overview

### What is SmartTerm?

SmartTerm is a **proof of concept** for a terminal UI library that provides:
- Scrolling output region (no prompt duplication)
- Context-aware input with color coding
- Fixed status bar
- Integration of ncurses + readline

**This is NOT a production library** - it's a ~250 LOC prototype to validate the architecture before deciding whether to build a full library.

### Project Goals

1. **Validate Architecture**: Prove that ncurses + readline can coexist
2. **Test Concepts**: Output buffer, context awareness, status bar
3. **Inform Decision**: Gather data to decide between 5 possible paths forward
4. **Enable Adventure Engine**: Provide UI foundation for text adventure game

### Current Status

- ✅ POC implementation complete (smartterm_poc.c)
- ✅ Architecture validated successfully
- ✅ Decision made: **Path B→A** (ship adventure engine first, library later)
- ⏳ Next step: Extract POC for adventure engine use

---

## Codebase Structure

### File Tree

```
smartterm-prototype/
├── smartterm_poc.c       # Main POC implementation (~250 LOC)
├── Makefile              # Build system (gcc + ncurses + readline)
├── .gitignore            # Standard C project ignores
├── README.md             # POC documentation & testing guide
├── DECISION.md           # Implementation path decision & tracking
├── NEXT-STEPS.md         # Detailed opportunity cost analysis
├── SESSION-SUMMARY.md    # Session notes & findings
└── CLAUDE.md            # This file (AI assistant guide)
```

### Key Files Explained

#### `smartterm_poc.c` (266 lines)
**Purpose**: Single-file proof of concept implementation

**Key Components**:
- `OutputBuffer` struct - stores output lines without prompt duplication
- `ContextType` enum - tracks input context (NORMAL, COMMAND, COMMENT, SPECIAL, SEARCH)
- `init_ui()` / `cleanup_ui()` - ncurses setup/teardown
- `render_output()` - display output buffer with context-based coloring
- `render_status()` - fixed status bar at bottom
- `readline_with_ncurses()` - suspend/resume wrapper for readline integration
- `main()` - event loop with command handling

**Context Markers**:
- `>` or no prefix - Normal input (white)
- `!` - System command (yellow)
- `#` - Comment (green)
- `@` - Special action (cyan)
- `/` - Search (magenta)

#### `Makefile` (40 lines)
**Purpose**: Simple build system

**Targets**:
- `make` or `make all` - Build binary
- `make run` - Build and execute
- `make clean` - Remove artifacts
- `make test` - Show testing instructions
- `make help` - Display available targets

**Build Configuration**:
- Compiler: gcc with `-Wall -Wextra -std=c11`
- Dependencies: `-lncurses -lreadline`

#### `README.md` (255 lines)
**Purpose**: User-facing documentation

**Contents**:
- What the POC demonstrates
- Build prerequisites (Debian/Ubuntu, macOS)
- Testing guide (6 test scenarios)
- Architecture diagram
- Known limitations
- Learnings and surprises

#### `DECISION.md` (179 lines)
**Purpose**: Track implementation path decision

**Contains**:
- Decision point context
- 5 available paths with estimates
- Chosen path: **B→A** (ship first, refactor later)
- Rationale and timeline commitments
- Milestone tracking
- Retrospective placeholders

#### `NEXT-STEPS.md` (503 lines)
**Purpose**: Comprehensive opportunity cost analysis

**Analyzes**:
- Path A: Full library (3-4 weeks, 45-60 hours)
- Path B: Use POC as-is (1-2 days, 7-9 hours)
- Path C: Pivot to different approach (1-2 weeks)
- Path B→A: Ship first, library later (recommended)
- Path A+B: Incremental development

**Includes**:
- Break-even calculations
- Risk assessments
- Decision framework (5 key questions)
- Concrete next actions for each path

#### `SESSION-SUMMARY.md` (458 lines)
**Purpose**: Session notes and findings

**Documents**:
- What was created
- Key findings from POC
- Architecture validation
- Learnings and surprises
- Time investment summary
- Success criteria

---

## Architecture & Design

### Three-Region Layout

```
┌──────────────────────────────────┐
│     Output Window (scrolling)    │  ← OutputBuffer, no prompt duplication
│  Line 1                          │
│  Line 2                          │
│  ...                             │
│  Line N                          │
├──────────────────────────────────┤
│  Status Bar (info)               │  ← Fixed info display
├──────────────────────────────────┤
│  (readline input area)           │  ← Input handled by readline
│  > _                             │     (appears below window)
└──────────────────────────────────┘
```

### Key Design Decisions

1. **Output Buffer Separate from Display**
   - Store lines in array (no ncurses dependency)
   - Render buffer to window on demand
   - Enables scrollback, search, export
   - Prevents prompt duplication

2. **readline Suspends ncurses**
   - ncurses paused during input (`endwin()`)
   - readline handles editing
   - ncurses resumes for display (`reset_prog_mode()`)

3. **Context as Prefix**
   - Store context marker with each line
   - Render with appropriate color
   - Easy to extend to more contexts

4. **Status Bar as Separate Window**
   - Fixed position (always visible)
   - Independent update cycle
   - Shows app state and info

### Data Structures

```c
// Output buffer - core data structure
typedef struct {
    char **lines;        // Array of output lines
    int count;          // Current line count
    int capacity;       // Max lines (1000)
    int scroll_offset;  // For scrollback (not implemented)
} OutputBuffer;

// Context types for color coding
typedef enum {
    CTX_NORMAL,    // Default (white)
    CTX_COMMAND,   // ! prefix (yellow)
    CTX_COMMENT,   // # prefix (green)
    CTX_SPECIAL,   // @ prefix (cyan)
    CTX_SEARCH     // / prefix (magenta)
} ContextType;
```

### Color Scheme

| Context | Prefix | Color | COLOR_PAIR |
|---------|--------|-------|------------|
| Normal | `>` or none | White | 1 |
| Command | `!` | Yellow | 2 |
| Comment | `#` | Green | 3 |
| Special | `@` | Cyan | 4 |
| Search | `/` | Magenta | 5 |

### Known Limitations

**Intentionally Not Implemented** (POC scope):
- ❌ Tab completion (readline supports it, not wired up)
- ❌ True multi-line expanding input
- ❌ Persistent history file
- ❌ Configurable themes
- ❌ Custom key bindings
- ❌ Full API surface
- ❌ Production error handling
- ❌ Memory safety checks
- ❌ Comprehensive documentation

**Core Concept Proven**:
- ✅ Output buffer without prompt duplication
- ✅ Context-aware coloring
- ✅ Status bar
- ✅ ncurses + readline integration

---

## Development Workflows

### Building the POC

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt-get install libncurses-dev libreadline-dev

# Install dependencies (macOS)
brew install ncurses readline

# Build
make

# Build and run
make run

# Clean artifacts
make clean
```

### Testing the POC

**Test 1: Basic Output**
```
> hello
Echo [NORMAL]: hello
```
Verify: No prompt duplication, clean history

**Test 2: Context Detection**
```
> !system command
Echo [CMD]: system command
```
Verify: Colors change based on context

**Test 3: Commands**
```
> help
Available commands:
  help    - Show this help
  ...
```
Verify: Output scrolls properly

**Test 4: History & Editing**
- Up/Down arrows: Navigate history
- Ctrl+A: Beginning of line
- Ctrl+E: End of line
- Ctrl+K: Kill to end
- Ctrl+U: Kill entire line

**Test 5: Scrolling**
Enter many commands to verify old lines scroll off top

### Git Workflow

**Current Branch**: `claude/claude-md-mi3opz4monq14xm7-011dWrWja2WYnrVXZweJGhpK`

**Commit Conventions**:
- Use clear, descriptive messages
- Focus on "why" rather than "what"
- Follow existing commit style (see `git log`)

**Push Commands**:
```bash
# Always use -u flag
git push -u origin claude/claude-md-mi3opz4monq14xm7-011dWrWja2WYnrVXZweJGhpK

# Retry on network errors (up to 4 times with exponential backoff)
```

**Important**: Branch names must start with `claude/` and end with session ID for push to succeed.

---

## Key Conventions

### Code Style (C)

1. **Naming**:
   - `snake_case` for functions and variables
   - `PascalCase` for structs and enums
   - `ALL_CAPS` for constants and defines

2. **Formatting**:
   - 4-space indentation
   - K&R brace style
   - Compiler flags: `-Wall -Wextra -std=c11`

3. **Comments**:
   - File header explains purpose
   - Brief inline comments for non-obvious code
   - No excessive commenting in POC

### Documentation Style

1. **Markdown Files**:
   - Clear hierarchical structure (##, ###)
   - Use tables for comparisons
   - Include concrete examples
   - Add visual diagrams with ASCII art

2. **Decision Documents**:
   - Context before decision
   - List alternatives with estimates
   - Explain rationale
   - Track milestones
   - Include retrospective sections

### Project Organization

1. **Single File POC**: Keep POC as one file for simplicity
2. **Comprehensive Docs**: Detailed documentation compensates for POC simplicity
3. **Decision Tracking**: Document decisions and reasoning
4. **Opportunity Costs**: Analyze trade-offs explicitly

---

## Implementation Decision

### Chosen Path: B→A (Ship First, Refactor Later)

**Decision Date**: 2025-11-17

**Rationale**:
- Validate library need with real usage before investing 50+ hours
- POC proves concept works technically
- Don't yet know what features adventure engine actually needs
- Get adventure engine working THIS WEEK vs next month
- Gather real-world requirements from actual usage
- Build library later with informed design (if needed at all)

**Timeline Commitment**:

- **Week 1 (Nov 17-24)**: Adventure engine MVP - 8-10 hours
  - Extract POC to simple library (2-3 hours)
  - Build core engine (parser, world system, basic commands) (5-7 hours)

- **Week 2-3 (Nov 25-Dec 8)**: Content creation - 8-10 hours
  - Create 2-3 adventure worlds with rich narrative
  - Test gameplay and gather feedback

- **Evaluation Point (Dec 8)**: Assess if library is needed
  - Is POC code limiting development? → Consider Path A
  - Is POC sufficient? → Continue with content focus
  - Building more terminal apps? → Library justified

### Milestones

- [ ] Extract POC to smartterm_simple library (Target: Nov 17-18)
- [ ] Adventure engine MVP with basic commands (Target: Nov 19-21)
- [ ] First playable world with narrative (Target: Nov 22-24)
- [ ] 2-3 additional worlds, gameplay testing (Target: Nov 25-Dec 1)
- [ ] Evaluation - library needed? (Target: Dec 8)

### Alternative Paths (Not Chosen)

**Path A: Full Library** (3-4 weeks, 45-60 hours)
- Professional `libsmartterm.a`
- Reusable across multiple projects
- Portfolio quality code
- **Why Not**: Too much upfront investment without validation

**Path B: Use POC As-Is** (1-2 days)
- Extract and use immediately
- No refactor planned
- **Why Not**: Need to keep library option open

**Path C: Pivot** (1-2 weeks)
- Explore Haskell Brick or alternatives
- **Why Not**: POC validates current approach

**Path A+B: Iterative** (ongoing)
- Improve incrementally
- **Why Not**: Prefer validation checkpoint at Dec 8

---

## Common Tasks

### For AI Assistants: Key Tasks and How to Handle Them

#### Task 1: Extract POC to Simple Library

**When**: Week 1 of Path B→A (next immediate task)

**Steps**:
1. Create `lib/` directory in adventure-engine project
2. Split `smartterm_poc.c` into:
   - `lib/smartterm_simple.h` - public API
   - `lib/smartterm_simple.c` - implementation
3. Extract core functions:
   - `init_output_buffer()`, `add_output_line()`
   - `init_ui()`, `cleanup_ui()`
   - `render_output()`, `render_status()`
   - `readline_with_ncurses()`
4. Keep Makefile simple: build static library
5. Document minimal API in header

**Don't**:
- Don't over-engineer
- Don't add features beyond POC
- Don't create elaborate build system

#### Task 2: Integrate with Adventure Engine

**When**: Week 1 of Path B→A (after extraction)

**Steps**:
1. Link against `libsmartterm_simple.a`
2. Initialize UI on engine startup
3. Use `add_output_line()` for game text
4. Use context types for different message types
5. Update status bar with game state
6. Use `readline_with_ncurses()` for input

**Don't**:
- Don't modify smartterm code for game-specific features
- Don't tightly couple - keep clean interface

#### Task 3: Document Pain Points

**When**: Throughout adventure engine development

**Steps**:
1. Keep notes file: `SMARTTERM-LIMITATIONS.md`
2. Document when POC code is limiting
3. Note missing features that would help
4. Track refactor/workaround time
5. Use at Dec 8 evaluation point

#### Task 4: Build Full Library (If Triggered)

**When**: After Dec 8 evaluation, if needed

**Steps**:
1. Reference planning docs from session
2. Create new `smartterm` repository
3. Follow Path A from NEXT-STEPS.md
4. Design API based on real usage learnings
5. Implement incrementally
6. Migrate adventure engine when stable

---

## Important Context

### Project Timeline

- **2025-11-17**: POC created and completed
- **2025-11-17**: Decision made (Path B→A)
- **Target Nov 17-24**: Adventure engine MVP
- **Target Dec 8**: Evaluation checkpoint

### Related Projects

1. **terminal-stars**: Another jcaldwell-labs terminal game project
2. **adventure-engine**: Previous version with work/private context pollution
3. **jcaldwell-labs**: Portfolio of professional public repos

### Learnings from POC

**What Works Well**:
- ncurses + readline integration is clean
- Output buffer design prevents prompt duplication
- Context coloring is simple and effective
- Status bar provides good UX
- Single-file POC proved concept quickly

**What Needs Improvement**:
- Multi-line input: readline is single-line
- Input area sizing: readline uses full terminal
- Suspend/resume: flickers slightly
- Memory management: no bounds checking
- Error handling: minimal in POC

**Surprising Discoveries**:
- readline owns the terminal (hard to constrain)
- Multi-line may need custom input handler
- Context detection is trivially easy
- Buffer management is straightforward

### Dependencies

**System Libraries**:
- `ncurses` / `libncurses-dev` - Terminal UI
- `readline` / `libreadline-dev` - Input editing

**Standard C Libraries**:
- `stdio.h`, `stdlib.h`, `string.h`

**C Standard**: C11 (`-std=c11`)

---

## References

### Key Documentation Files

- `README.md` - User-facing POC documentation
- `DECISION.md` - Implementation path decision tracking
- `NEXT-STEPS.md` - Opportunity cost analysis (20 KB)
- `SESSION-SUMMARY.md` - Session notes and findings

### External Resources

**ncurses**:
- Man pages: `man ncurses`, `man curs_window`
- Tutorial: https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/

**readline**:
- Man pages: `man readline`, `man history`
- GNU Manual: https://tiswww.case.edu/php/chet/readline/rltop.html

**Build System**:
- GNU Make: `man make`
- gcc: `man gcc`

### Code Patterns to Reference

**Output Buffer Pattern** (smartterm_poc.c:47-73):
```c
void init_output_buffer() {
    output.capacity = MAX_OUTPUT_LINES;
    output.lines = malloc(sizeof(char*) * output.capacity);
    output.count = 0;
}

void add_output_line(const char *text, ContextType ctx) {
    if (output.count >= output.capacity) {
        // Drop oldest line
        free(output.lines[0]);
        memmove(output.lines, output.lines + 1,
                sizeof(char*) * (output.capacity - 1));
        output.count--;
    }
    // Store with context prefix
    char prefix = (ctx == CTX_COMMAND) ? '!' : ...;
    output.lines[output.count] = malloc(strlen(text) + 3);
    snprintf(output.lines[output.count], len, "%c %s", prefix, text);
    output.count++;
}
```

**ncurses + readline Integration** (smartterm_poc.c:162-175):
```c
char* readline_with_ncurses(const char *prompt) {
    // Suspend ncurses
    def_prog_mode();
    endwin();

    // Use readline
    char *input = readline(prompt);

    // Resume ncurses
    reset_prog_mode();
    refresh();

    return input;
}
```

---

## For AI Assistants: Best Practices

### When Working on This Project

1. **Read DECISION.md First**: Understand current phase and next milestones
2. **Respect POC Status**: Don't over-engineer or add speculative features
3. **Follow Path B→A**: Focus on adventure engine, not library perfection
4. **Document Limitations**: Track pain points for Dec 8 evaluation
5. **Keep It Simple**: Resist urge to build full library now

### When Extracting POC Code

1. **Minimal Changes**: Keep as close to original as possible
2. **Simple API**: Don't create elaborate interfaces
3. **Single Purpose**: UI only, no game logic
4. **Easy to Replace**: Design for eventual library swap

### When Building Adventure Engine

1. **Use Smartterm Simply**: Call init, add lines, cleanup
2. **Don't Extend**: If you need features, document instead
3. **Focus on Content**: Game narrative > UI polish
4. **Track Friction**: Note when POC code is limiting

### When Evaluating at Dec 8

1. **Review SMARTTERM-LIMITATIONS.md**: What was painful?
2. **Count Projects**: How many terminal apps in pipeline?
3. **Estimate Refactor**: POC fix-up vs library build cost?
4. **Check Motivation**: Energy for 40-hour library project?
5. **Decide Path**: Full library (A) or keep POC?

---

## Decision Framework for Future Choices

### Should We Build Full Library?

**Ask These Questions**:

1. **Is POC code limiting adventure engine development?**
   - Missing features we need?
   - Code quality blocking progress?
   - Refactoring becoming painful?

2. **Are we building additional terminal projects?**
   - If yes → Library justified (reuse across projects)
   - If no → Keep POC (one project doesn't justify library)

3. **Has real usage revealed better requirements?**
   - What features actually matter?
   - What planning assumptions were wrong?
   - What should library API look like?

**Possible Outcomes**:
- **Build Library (Path A)**: If building 2+ more terminal apps or POC is limiting
- **Keep POC**: If sufficient and no other terminal projects planned
- **Pivot**: If different approach emerges from usage experience

---

## Appendix: Project History

### Creation Session (2025-11-17)

**Duration**: ~1 hour
**Deliverables**:
- POC implementation (250 LOC)
- Makefile and build system
- README with testing guide
- NEXT-STEPS with opportunity analysis (20 KB)
- DECISION tracking document
- SESSION-SUMMARY with findings

**Time Invested**: 1 hour
**Time Saved**: 10-20 hours (validated before full investment)
**ROI**: 10-20x (prevented building wrong thing)

### Design Philosophy

**POC-First Approach**:
1. Build minimal proof of concept
2. Validate architecture and feasibility
3. Analyze opportunity costs
4. Make informed decision
5. Ship fast, iterate based on real usage

**Why This Works**:
- Low upfront investment
- Real data over speculation
- Maintains momentum
- Reduces risk of over-engineering
- Validates need before building

---

**End of CLAUDE.md**

*This document should be updated as the project evolves, especially after the Dec 8 evaluation checkpoint.*
