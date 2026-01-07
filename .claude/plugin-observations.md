# Plugin Discovery Observations

> Collecting pain points and insights during daily work for plugin requirements gathering.
> Date started: 2026-01-06

---

## Session: 2026-01-06 - cc-bash Phase 2 Development

### Pain Points Observed

1. **isatty() Color Detection Problem**
   - Programs like `glow`, `bat`, `ls` check `isatty(stdout)` to decide colors
   - Our pipe-based execution means colors are disabled by default
   - **Workaround**: Environment vars (GLAMOUR_STYLE, --color=always flags)
   - **Plugin idea**: Auto-detect common commands and add color flags?

2. **@ Commands Inconsistency**
   - Python SDK has @ask, @explain, @fix, @cmd (AI features)
   - C version only has @help, @clear, @quit
   - Users might expect AI commands in C version
   - **Plugin idea**: Plugin system could add @commands dynamically

3. **Parallel Agent Coordination**
   - Spawned 4 agents to work on different issues
   - Some agents got stuck in loops (git config)
   - Code merged cleanly because features touched different areas
   - **Insight**: Parallel agents work well for isolated features

4. **Test Coverage Gaps**
   - Tests are static code analysis (grep), not runtime tests
   - Can't easily test interactive TUI behavior
   - **Plugin idea**: Test harness that can simulate keystrokes?

### Workflow Insights

1. **TodoWrite Integration**
   - Helpful for tracking multi-step tasks
   - Status updates visible to user during long operations
   - Works well with parallel agent model

2. **GitHub Issue Management**
   - `gh issue close --comment` is efficient for bulk updates
   - Milestones help organize phase-based work
   - Would be nice to auto-link commits to issues

3. **Build-Test-Commit Cycle**
   - `make clean && make && make test` is a common pattern
   - Could be automated as a pre-commit hook

### Feature Requests (for cc-bash)

- [ ] PTY-based execution for true isatty() support
- [ ] Persistent history file (~/.cc-bash-history)
- [ ] Config file (~/.cc-bashrc) for aliases, env vars
- [ ] @alias command for custom shortcuts

---

## Retro Questions for Plugin Phase

1. What repetitive tasks could a plugin automate?
2. Which pain points are cc-bash specific vs general Claude Code?
3. What would make the parallel agent experience better?
4. How should plugins extend @commands?

---

## Update: Config File Implementation (same session)

### What Was Added

- `~/.cc-bashrc` config file support
- `alias name='command'` syntax
- `export VAR=value` syntax
- `@alias` command to list/add aliases at runtime
- Alias expansion when executing commands

### New Observations

5. **Config Format Choice**
   - Chose shell-like syntax (alias/export) over TOML/JSON
   - Simpler to implement, familiar to users
   - **Plugin idea**: Support multiple config formats via parser plugins?

6. **Session vs Persistent Aliases**
   - `@alias` adds only for current session
   - Would need to write back to file for persistence
   - **Trade-off**: Simple implementation vs full feature

---

## Update: Theme System Implementation (same session)

### What Was Added

- `theme.*` syntax in ~/.cc-bashrc for configuring colors
- `@theme` command to display current theme colors
- 7 configurable theme fields: prompt, error, comment, dim, header, status, scroll
- Color parsing: black, red, green, yellow, blue, magenta, cyan, white
- Modifier support: bold, dim (combinable with colors: "bold cyan")
- 32 new unit tests for theme parsing

### Observations

7. **Theme Design Simplicity**
   - Used simple string-based theme fields vs. elaborate theme struct
   - Color parsing via simple tokenizer (no complex parsing needed)
   - **Plugin idea**: Theme presets? ("theme.preset=solarized-dark")

8. **Progressive Enhancement Pattern**
   - Started with hardcoded #define macros
   - Replaced with runtime-configurable theme struct
   - Defaults maintained - no breaking changes for existing users
   - **Insight**: Good pattern for plugin development too

9. **Unit Test Value**
   - Theme tests caught edge cases (NULL, empty, unknown colors)
   - Pure functions easy to test without TTY
   - 71 tests now (vs 39 before themes)
   - **Plugin idea**: Test harness for plugin pure functions?

---

## Update: Snippet System Implementation (same session)

### What Was Added

- `snippet name='template with $1 $2'` syntax in ~/.cc-bashrc
- `@snippet` command to list all snippets or run with arguments
- $1-$9 placeholder expansion for positional arguments
- Shows expanded command before execution (dim color)
- 21 new unit tests for snippet functionality

### Observations

10. **Snippet vs Alias Design**
    - Aliases: simple substitution of first word
    - Snippets: full template expansion with arguments
    - Both use same config pattern (name='value') but different prefix
    - **Plugin idea**: Could unify into single "macro" concept?

11. **Argument Parsing Complexity**
    - Simple strtok-based parsing works for basic cases
    - Doesn't handle quoted arguments: `@snippet foo "arg with spaces"`
    - **Trade-off**: Simple parsing now vs. shell-like complexity
    - **Plugin idea**: Use shell to parse arguments?

12. **User Feedback Pattern**
    - Showing expanded command before execution helps user understand what runs
    - Consistent with alias expansion (which doesn't show)
    - **Insight**: Transparency builds trust

---

## Update: Event System Implementation (same session)

### What Was Added

- 7 event types: STARTUP, SHUTDOWN, PRE_COMMAND, POST_COMMAND, CD, ALIAS_EXPAND, SNIPPET_EXPAND
- Event struct with contextual data (command, expanded, exit_code, old_cwd, new_cwd)
- Hook registration system with IDs for unregistration
- Helper functions for emitting different event types
- `@hooks` command to list registered hooks and event types
- 48 new unit tests for event system (140 total tests now)

### Observations

13. **Observer Pattern in C**
    - Simple function pointer + user_data works well
    - Hook IDs allow selective unregistration
    - Active flag avoids array compaction on unregister
    - **Plugin idea**: This is exactly how plugins would register handlers!

14. **Event Data Design**
    - Single Event struct covers all event types
    - NULL fields for inapplicable data (e.g., exit_code for CD event)
    - Helper functions hide struct creation details
    - **Trade-off**: Flexible but wastes some memory per event

15. **@hooks as Debugging Tool**
    - Shows registered hooks and their state (active/inactive)
    - Documents event types inline - self-documenting API
    - Useful for @workflow development (can see what's registered)
    - **Insight**: Internal tools should be user-accessible for debugging

16. **Test Strategy for Events**
    - Stateless handlers: counting_handler, command_handler
    - user_data for passing test state
    - Edge cases: wrong type, multiple handlers, overflow
    - **Insight**: Pure handler functions are easy to test

17. **Foundation for @workflow**
    - PRE_COMMAND can cancel/modify commands
    - POST_COMMAND can track success/failure
    - CD events enable directory-aware workflows
    - Expand events enable alias/snippet chains
    - **Next step**: @workflow will use this to orchestrate multi-step tasks

---

## Update: Workflow System Implementation (same session)

### What Was Added

- Workflow struct with name, steps array, step_count, stop_on_error flag
- Config syntax: `workflow name='cmd1 && cmd2'` (stop on error) or `workflow name='cmd1; cmd2'` (continue)
- `@workflow` command to list all workflows with their steps
- `@workflow name` to run a workflow with step-by-step progress display
- `@workflow name --dry-run` to preview without executing
- Event integration: emits PRE_COMMAND/POST_COMMAND for each step
- 30 new unit tests for workflow parsing (170 total tests now)

### Observations

18. **Separator-Based Error Handling**
    - `&&` = stop on first failure (like shell)
    - `;` = continue regardless (like shell)
    - Familiar semantics for shell users
    - **Plugin idea**: Could add more separators like `||` (run on failure)?

19. **Step-by-Step Progress Display**
    - Shows `[1/3] command` before each step
    - Clear success (✓) or failure (✗) indicator
    - Exit code shown on failure
    - **Insight**: Visual progress builds confidence in automation

20. **Dry-Run Pattern**
    - `--dry-run` shows what would execute
    - Helps users verify before running destructive workflows
    - **Plugin idea**: Could add `--verbose` for more detail?

21. **Event Integration**
    - Each workflow step emits PRE_COMMAND/POST_COMMAND events
    - Future plugins could hook into workflow execution
    - Could add WORKFLOW_START/WORKFLOW_END events
    - **Insight**: Events make automation composable

22. **Parsing Complexity**
    - Using strtok_r for thread safety (though not needed yet)
    - Edge case: commands containing && or ; in strings
    - Current parsing is naive but works for common cases
    - **Trade-off**: Simple parsing vs full shell parser

---

## Update: Plugin System Implementation (same session)

### What Was Added

- Plugin directory structure: `~/.cc-bash/plugins/<name>/`
- plugin.conf manifest with name, version, description, hooks, commands
- config.conf for plugin-provided aliases/snippets/workflows
- Hook scripts triggered on events with environment variables
- `@plugins` command to list loaded plugins with details
- Plugin commands: custom @ commands via shell scripts
- Example plugin demonstrating all features

### Plugin Architecture

```
~/.cc-bash/plugins/
└── my-plugin/
    ├── plugin.conf     # Manifest (required)
    ├── config.conf     # Aliases/snippets/workflows (optional)
    └── hooks/          # Event hook scripts (optional)
        ├── on_startup.sh
        ├── on_cd.sh
        └── on_post_command.sh
```

### Observations

23. **Directory-Based Plugin Model**
    - Each plugin is a directory, not a single file
    - Allows multiple files: manifest, config, hooks, commands
    - Similar to VSCode extensions, npm packages
    - **Insight**: Directory model scales better than single-file

24. **Shell-Based Hooks**
    - Hook scripts are plain shell scripts
    - Event data passed via environment variables (CCBASH_COMMAND, etc.)
    - Simple to write, debug, and understand
    - **Trade-off**: Fork overhead vs simplicity

25. **Plugin-Provided Config**
    - Plugins can provide aliases, snippets, workflows
    - Uses same syntax as ~/.cc-bashrc
    - No learning curve for plugin authors
    - **Insight**: Reuse existing config syntax for extensions

26. **Discovery via Directory Scan**
    - opendir/readdir to find plugin directories
    - Each directory with plugin.conf is a valid plugin
    - No registry or manifest file needed
    - **Trade-off**: Simple discovery vs explicit activation

27. **Plugin Commands**
    - `command.NAME=path/to/script.sh` in plugin.conf
    - Creates `@NAME` command that runs the script
    - Output captured and displayed in TUI
    - **Plugin idea**: Pass arguments to plugin commands?

28. **Event-Environment Bridge**
    - Events carry data (command, exit_code, cwd changes)
    - Bridge to shell scripts via environment variables
    - Hook scripts can read but not modify behavior
    - **Future**: Add return value handling for blocking hooks?

---

_Add new observations below as work continues..._
