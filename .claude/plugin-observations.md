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

_Add new observations below as work continues..._
