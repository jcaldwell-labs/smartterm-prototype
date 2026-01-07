/*
 * cc-bash: Claude Code-style bash shell wrapper
 *
 * This is a lightweight terminal shell wrapper that provides a structured
 * three-region layout for command execution, similar to the Claude Code
 * interface style.
 *
 * Architecture Overview:
 * ----------------------
 * The terminal is divided into three distinct regions:
 *
 * 1. Output area (top) - Command output scrolls here using ANSI scroll regions.
 *    Old output scrolls up and off the top. New output appears at bottom of
 *    this region.
 *
 * 2. Prompt area (middle) - Single line for user input. The prompt character
 *    is displayed here and user types commands. When Enter is pressed, the
 *    command is "scrolled" into the output area for history.
 *
 * 3. Status area (bottom) - Fixed 2-line area showing user@host:cwd and hints.
 *    This never scrolls and always remains visible.
 *
 * Key Design Decisions:
 * - Uses raw terminal mode (termios) for character-by-character input
 * - Uses ANSI escape sequences directly instead of ncurses for simplicity
 * - Commands are executed via fork/exec with /bin/sh -c for shell expansion
 * - cd is handled as a builtin since it must affect the parent process
 * - Comments (#) are displayed but not executed
 *
 * Dependencies: POSIX only (no ncurses, no readline)
 *
 * For contributors: See tests/test_cc_bash.sh for test coverage and
 * CONTRIBUTING.md for development guidelines.
 */

#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

/* ============================================================================
 * ANSI Escape Code Definitions
 * ============================================================================
 * These codes control terminal text formatting. We use them directly instead
 * of ncurses for simplicity and portability. All modern terminals support these.
 *
 * Default values - these are used as fallbacks when theme is not configured.
 */
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define WHITE   "\033[37m"

/* ============================================================================
 * Theme Configuration
 * ============================================================================
 * Configurable colors loaded from ~/.cc-bashrc using theme.* syntax:
 *   theme.prompt=cyan
 *   theme.error=red
 *   theme.comment=green
 *   theme.dim=dim
 *   theme.header=bold cyan
 *
 * Supported colors: black, red, green, yellow, blue, magenta, cyan, white
 * Modifiers: bold, dim (can be combined with color)
 */
typedef struct {
    char prompt[32];    /* Prompt symbol color (default: none) */
    char error[32];     /* Error/stderr color (default: red) */
    char comment[32];   /* Comment (#) color (default: yellow) */
    char dim[32];       /* Dim text color (default: dim gray) */
    char header[32];    /* Help headers color (default: cyan) */
    char status[32];    /* Status bar color (default: bold) */
    char scroll[32];    /* Scroll indicator color (default: cyan) */
} Theme;

static Theme theme = {
    .prompt = "",           /* No color by default */
    .error = "\033[31m",    /* Red */
    .comment = "\033[33m",  /* Yellow */
    .dim = "\033[2m",       /* Dim */
    .header = "\033[36m",   /* Cyan */
    .status = "\033[1m",    /* Bold */
    .scroll = "\033[36m"    /* Cyan */
};

/* Buffer and history size limits */
#define INPUT_BUF_SIZE 4096
#define MAX_HISTORY 100
#define MAX_OUTPUT_LINES 1000
#define SCROLL_PAGE_SIZE 10
#define SCROLL_LARGE_JUMP 50
#define MAX_COMPLETIONS 256
#define MAX_ALIASES 100
#define MAX_SNIPPETS 50
#define MAX_WORKFLOWS 20
#define MAX_WORKFLOW_STEPS 10
#define MAX_HOOKS 20
#define MAX_PLUGINS 20
#define MAX_PLUGIN_COMMANDS 10
#define MAX_PLUGIN_HOOKS 7   /* One per event type */
#define CONFIG_LINE_SIZE 1024
#define HISTORY_FILE ".cc-bash-history"
#define PLUGIN_DIR ".cc-bash/plugins"

/* ============================================================================
 * Event System Types
 * ============================================================================
 * The event system provides lifecycle hooks for internal features (like
 * @workflow) and future plugin extensibility. Events are emitted at key
 * points in the shell lifecycle.
 */

typedef enum {
    EVENT_STARTUP,       /* Shell started, config loaded */
    EVENT_SHUTDOWN,      /* Shell exiting */
    EVENT_PRE_COMMAND,   /* Before command execution */
    EVENT_POST_COMMAND,  /* After command execution */
    EVENT_CD,            /* Directory changed */
    EVENT_ALIAS_EXPAND,  /* Alias was expanded */
    EVENT_SNIPPET_EXPAND /* Snippet was expanded */
} EventType;

/* Event data passed to handlers */
typedef struct {
    EventType type;
    const char* command;      /* For PRE/POST_COMMAND, ALIAS/SNIPPET_EXPAND */
    const char* expanded;     /* For ALIAS/SNIPPET_EXPAND: the expanded form */
    int exit_code;            /* For POST_COMMAND */
    const char* old_cwd;      /* For CD: previous directory */
    const char* new_cwd;      /* For CD: new directory */
} Event;

/* Event handler function type */
typedef void (*EventHandler)(const Event* event, void* user_data);

/* Registered hook */
typedef struct {
    EventType type;
    EventHandler handler;
    void* user_data;
    int active;
} Hook;

/* ============================================================================
 * Global State
 * ============================================================================
 * Terminal configuration and layout state. These are module-level globals
 * because the terminal is inherently global state - there's only one terminal
 * per process.
 */

/* Terminal state - original settings saved for cleanup */
static struct termios orig_termios;
static int term_rows = 24;
static int term_cols = 80;
static char cwd[PATH_MAX];

/* Layout: output_rows + separator + prompt_rows + separator + status_rows */
static int output_rows;
static int prompt_row;      /* Single row for prompt */
static int status_start;    /* Where status begins */

/* History */
static char* history[MAX_HISTORY];
static int history_count = 0;
static int history_pos = 0;

/* Output buffer for scrollback */
typedef struct {
    char* text;
    int is_stderr;
} OutputLine;

static OutputLine output_buffer[MAX_OUTPUT_LINES];
static int output_count = 0;      /* Total lines stored */
static int output_start = 0;      /* Start index (circular) */
static int scroll_offset = 0;     /* 0 = at bottom, positive = scrolled up */

/* Aliases - loaded from ~/.cc-bashrc */
typedef struct {
    char* name;
    char* command;
} Alias;

static Alias aliases[MAX_ALIASES];
static int alias_count = 0;

/* Snippets - command templates with $1, $2, etc. substitution */
typedef struct {
    char* name;
    char* template;
} Snippet;

static Snippet snippets[MAX_SNIPPETS];
static int snippet_count = 0;

/* Workflows - named sequences of commands executed together
 * Config syntax: workflow name='cmd1 && cmd2 && cmd3'
 * The && separator means "stop on first failure"
 * Use ; separator for "continue regardless of failure"
 */
typedef struct {
    char* name;
    char* steps[MAX_WORKFLOW_STEPS];  /* Individual commands */
    int step_count;
    int stop_on_error;                /* 1 = stop on first failure (&&), 0 = continue (;) */
} Workflow;

static Workflow workflows[MAX_WORKFLOWS];
static int workflow_count = 0;

/* ============================================================================
 * Plugin System Types
 * ============================================================================
 * Plugins extend cc-bash with custom commands, hooks, aliases, snippets,
 * and workflows. Plugins are loaded from ~/.cc-bash/plugins/<name>/
 *
 * Plugin structure:
 *   ~/.cc-bash/plugins/my-plugin/
 *   ├── plugin.conf           # Manifest (name, version, description)
 *   ├── config.conf           # Optional: aliases, snippets, workflows
 *   └── hooks/                # Optional: shell scripts for event hooks
 *       ├── on_startup.sh
 *       ├── on_pre_command.sh
 *       └── on_cd.sh
 */

/* Plugin command - custom @ command provided by plugin */
typedef struct {
    char* name;          /* Command name (after @) */
    char* script_path;   /* Path to shell script */
    char* description;   /* Help text */
} PluginCommand;

/* Plugin - loaded from plugin directory */
typedef struct {
    char* name;
    char* version;
    char* description;
    char* path;                              /* Plugin directory path */
    int enabled;
    PluginCommand commands[MAX_PLUGIN_COMMANDS];
    int command_count;
    char* hook_scripts[MAX_PLUGIN_HOOKS];    /* Script paths indexed by EventType */
} Plugin;

static Plugin plugins[MAX_PLUGINS];
static int plugin_count = 0;

/* Last command exit code - displayed in prompt when non-zero */
static int last_exit = 0;

/* Event hooks - registered handlers for lifecycle events */
static Hook hooks[MAX_HOOKS];
static int hook_count = 0;

/* ============================================================================
 * Event System Functions
 * ============================================================================
 * Register, unregister, and emit events to registered handlers.
 */

/* Register a hook for an event type
 * Returns hook ID (>= 0) on success, -1 if hooks array is full
 */
static int register_hook(EventType type, EventHandler handler, void* user_data)
{
    if (hook_count >= MAX_HOOKS) return -1;

    hooks[hook_count].type = type;
    hooks[hook_count].handler = handler;
    hooks[hook_count].user_data = user_data;
    hooks[hook_count].active = 1;

    return hook_count++;
}

/* Unregister a hook by ID */
static void unregister_hook(int hook_id)
{
    if (hook_id >= 0 && hook_id < hook_count) {
        hooks[hook_id].active = 0;
    }
}

/* Forward declaration for plugin hook execution */
static void execute_plugin_hooks(EventType type, const Event* event);

/* Emit an event to all registered handlers of matching type */
static void emit_event(const Event* event)
{
    /* Call internal event handlers */
    for (int i = 0; i < hook_count; i++) {
        if (hooks[i].active && hooks[i].type == event->type) {
            hooks[i].handler(event, hooks[i].user_data);
        }
    }

    /* Call plugin hook scripts */
    execute_plugin_hooks(event->type, event);
}

/* Helper: Create and emit a simple event */
static void emit_simple_event(EventType type)
{
    Event event = {
        .type = type,
        .command = NULL,
        .expanded = NULL,
        .exit_code = 0,
        .old_cwd = NULL,
        .new_cwd = NULL
    };
    emit_event(&event);
}

/* Helper: Emit command event (PRE or POST) */
static void emit_command_event(EventType type, const char* command, int exit_code)
{
    Event event = {
        .type = type,
        .command = command,
        .expanded = NULL,
        .exit_code = exit_code,
        .old_cwd = NULL,
        .new_cwd = NULL
    };
    emit_event(&event);
}

/* Helper: Emit CD event */
static void emit_cd_event(const char* old_dir, const char* new_dir)
{
    Event event = {
        .type = EVENT_CD,
        .command = NULL,
        .expanded = NULL,
        .exit_code = 0,
        .old_cwd = old_dir,
        .new_cwd = new_dir
    };
    emit_event(&event);
}

/* Helper: Emit alias/snippet expand event */
static void emit_expand_event(EventType type, const char* original, const char* expanded)
{
    Event event = {
        .type = type,
        .command = original,
        .expanded = expanded,
        .exit_code = 0,
        .old_cwd = NULL,
        .new_cwd = NULL
    };
    emit_event(&event);
}

/* Get event type name (for debugging/display) */
static const char* event_type_name(EventType type)
{
    switch (type) {
        case EVENT_STARTUP:        return "STARTUP";
        case EVENT_SHUTDOWN:       return "SHUTDOWN";
        case EVENT_PRE_COMMAND:    return "PRE_COMMAND";
        case EVENT_POST_COMMAND:   return "POST_COMMAND";
        case EVENT_CD:             return "CD";
        case EVENT_ALIAS_EXPAND:   return "ALIAS_EXPAND";
        case EVENT_SNIPPET_EXPAND: return "SNIPPET_EXPAND";
        default:                   return "UNKNOWN";
    }
}

/* ============================================================================
 * Terminal Setup and Teardown
 * ============================================================================
 * Functions for initializing the terminal, setting up the three-region layout,
 * and restoring the terminal to its original state on exit.
 */

/* Get terminal size using ioctl and calculate region layout */
static void get_term_size(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        term_rows = ws.ws_row;
        term_cols = ws.ws_col;
    }

    /* Layout calculation:
     * - Status area: 2 lines (user@host + hint)
     * - Separator: 1 line
     * - Prompt area: 1 line
     * - Separator: 1 line
     * - Output area: rest
     */
    status_start = term_rows - 1;  /* Last 2 lines for status */
    prompt_row = term_rows - 4;    /* Prompt row */
    output_rows = prompt_row - 2;  /* Output area (above first separator) */
}

/* Save cursor position */
static void cursor_save(void) { printf("\033[s"); }

/* Restore cursor position */
static void cursor_restore(void) { printf("\033[u"); }

/* Move cursor to row, col (1-based) */
static void cursor_move(int row, int col) { printf("\033[%d;%dH", row, col); }

/* Clear entire line */
static void clear_line(void) { printf("\033[2K"); }

/* ============================================================================
 * Configuration and Aliases
 * ============================================================================
 * Load ~/.cc-bashrc config file and manage shell aliases.
 * Config format:
 *   alias ll='ls -la'
 *   alias gs='git status'
 *   export GLAMOUR_STYLE=dark
 */

/* ============================================================================
 * Theme Color Parsing
 * ============================================================================
 * Convert color names like "red", "bold cyan" to ANSI escape sequences.
 */

/* Parse a color name to ANSI code, returns pointer to static buffer */
static const char* parse_color(const char* name)
{
    static char buf[64];
    buf[0] = '\0';

    if (!name || strlen(name) == 0) {
        return buf;  /* Empty = no color */
    }

    /* Make a copy for tokenizing */
    char copy[64];
    strncpy(copy, name, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    /* Parse tokens (e.g., "bold cyan" -> "\033[1m\033[36m") */
    char* token = strtok(copy, " ");
    while (token) {
        /* Modifiers */
        if (strcmp(token, "bold") == 0) {
            strcat(buf, "\033[1m");
        } else if (strcmp(token, "dim") == 0) {
            strcat(buf, "\033[2m");
        }
        /* Colors */
        else if (strcmp(token, "black") == 0) {
            strcat(buf, "\033[30m");
        } else if (strcmp(token, "red") == 0) {
            strcat(buf, "\033[31m");
        } else if (strcmp(token, "green") == 0) {
            strcat(buf, "\033[32m");
        } else if (strcmp(token, "yellow") == 0) {
            strcat(buf, "\033[33m");
        } else if (strcmp(token, "blue") == 0) {
            strcat(buf, "\033[34m");
        } else if (strcmp(token, "magenta") == 0) {
            strcat(buf, "\033[35m");
        } else if (strcmp(token, "cyan") == 0) {
            strcat(buf, "\033[36m");
        } else if (strcmp(token, "white") == 0) {
            strcat(buf, "\033[37m");
        }
        /* Default/none */
        else if (strcmp(token, "none") == 0 || strcmp(token, "default") == 0) {
            /* Leave empty */
        }
        token = strtok(NULL, " ");
    }

    return buf;
}

/* Set a theme field by name */
static void set_theme_color(const char* field, const char* value)
{
    const char* code = parse_color(value);

    if (strcmp(field, "prompt") == 0) {
        strncpy(theme.prompt, code, sizeof(theme.prompt) - 1);
    } else if (strcmp(field, "error") == 0) {
        strncpy(theme.error, code, sizeof(theme.error) - 1);
    } else if (strcmp(field, "comment") == 0) {
        strncpy(theme.comment, code, sizeof(theme.comment) - 1);
    } else if (strcmp(field, "dim") == 0) {
        strncpy(theme.dim, code, sizeof(theme.dim) - 1);
    } else if (strcmp(field, "header") == 0) {
        strncpy(theme.header, code, sizeof(theme.header) - 1);
    } else if (strcmp(field, "status") == 0) {
        strncpy(theme.status, code, sizeof(theme.status) - 1);
    } else if (strcmp(field, "scroll") == 0) {
        strncpy(theme.scroll, code, sizeof(theme.scroll) - 1);
    }
}

/* Add an alias */
static void add_alias(const char* name, const char* command)
{
    if (alias_count >= MAX_ALIASES) return;

    /* Check for existing alias with same name and replace */
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            free(aliases[i].command);
            aliases[i].command = strdup(command);
            return;
        }
    }

    aliases[alias_count].name = strdup(name);
    aliases[alias_count].command = strdup(command);
    alias_count++;
}

/* Look up an alias, returns command or NULL */
static const char* get_alias(const char* name)
{
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            return aliases[i].command;
        }
    }
    return NULL;
}

/* ============================================================================
 * Snippet Management
 * ============================================================================
 * Snippets are command templates with $1, $2, $N placeholder substitution.
 * Unlike aliases, snippets support positional arguments.
 *
 * Config syntax: snippet name='command with $1 and $2'
 * Usage: @snippet name arg1 arg2
 */

/* Add a snippet */
static void add_snippet(const char* name, const char* template)
{
    if (snippet_count >= MAX_SNIPPETS) return;

    /* Check for existing snippet with same name and replace */
    for (int i = 0; i < snippet_count; i++) {
        if (strcmp(snippets[i].name, name) == 0) {
            free(snippets[i].template);
            snippets[i].template = strdup(template);
            return;
        }
    }

    snippets[snippet_count].name = strdup(name);
    snippets[snippet_count].template = strdup(template);
    snippet_count++;
}

/* Look up a snippet, returns template or NULL */
static const char* get_snippet(const char* name)
{
    for (int i = 0; i < snippet_count; i++) {
        if (strcmp(snippets[i].name, name) == 0) {
            return snippets[i].template;
        }
    }
    return NULL;
}

/* Expand snippet template with arguments
 * Replaces $1, $2, ... $9 with corresponding args
 * Returns newly allocated string (caller must free)
 */
static char* expand_snippet(const char* template, char** args, int arg_count)
{
    char* result = malloc(INPUT_BUF_SIZE);
    if (!result) return NULL;

    char* out = result;
    const char* in = template;
    size_t remaining = INPUT_BUF_SIZE - 1;

    while (*in && remaining > 0) {
        if (*in == '$' && in[1] >= '1' && in[1] <= '9') {
            /* Found $N placeholder */
            int arg_idx = in[1] - '1';  /* Convert '1'-'9' to 0-8 */
            if (arg_idx < arg_count && args[arg_idx]) {
                size_t arg_len = strlen(args[arg_idx]);
                if (arg_len <= remaining) {
                    strcpy(out, args[arg_idx]);
                    out += arg_len;
                    remaining -= arg_len;
                }
            }
            in += 2;  /* Skip $N */
        } else {
            *out++ = *in++;
            remaining--;
        }
    }
    *out = '\0';

    return result;
}

/* Free snippet memory */
static void free_snippets(void)
{
    for (int i = 0; i < snippet_count; i++) {
        free(snippets[i].name);
        free(snippets[i].template);
    }
    snippet_count = 0;
}

/* ============================================================================
 * Workflow Management
 * ============================================================================
 * Workflows are named sequences of commands. They integrate with the event
 * system to emit progress events as each step executes.
 *
 * Config syntax: workflow name='cmd1 && cmd2 && cmd3'
 * Usage: @workflow name
 */

/* Add a workflow from config string
 * Parses the command string into steps, using && or ; as separators
 * && means stop on first failure, ; means continue regardless
 */
static void add_workflow(const char* name, const char* commands)
{
    if (workflow_count >= MAX_WORKFLOWS) return;

    /* Check for existing workflow with same name and replace */
    int idx = -1;
    for (int i = 0; i < workflow_count; i++) {
        if (strcmp(workflows[i].name, name) == 0) {
            /* Free old steps */
            for (int j = 0; j < workflows[i].step_count; j++) {
                free(workflows[i].steps[j]);
            }
            idx = i;
            break;
        }
    }

    if (idx < 0) {
        idx = workflow_count++;
        workflows[idx].name = strdup(name);
    }

    workflows[idx].step_count = 0;
    workflows[idx].stop_on_error = 1;  /* Default: stop on error (&&) */

    /* Make a copy for tokenizing */
    char* cmd_copy = strdup(commands);
    if (!cmd_copy) return;

    /* Detect separator type: && or ; */
    if (strstr(cmd_copy, "&&")) {
        workflows[idx].stop_on_error = 1;
        /* Split by && */
        char* saveptr;
        char* step = strtok_r(cmd_copy, "&", &saveptr);
        while (step && workflows[idx].step_count < MAX_WORKFLOW_STEPS) {
            /* Skip empty tokens (from &&) */
            while (*step == '&' || *step == ' ') step++;
            if (*step) {
                /* Trim trailing spaces */
                char* end = step + strlen(step) - 1;
                while (end > step && *end == ' ') *end-- = '\0';
                if (*step) {
                    workflows[idx].steps[workflows[idx].step_count++] = strdup(step);
                }
            }
            step = strtok_r(NULL, "&", &saveptr);
        }
    } else {
        /* Split by ; */
        workflows[idx].stop_on_error = 0;
        char* saveptr;
        char* step = strtok_r(cmd_copy, ";", &saveptr);
        while (step && workflows[idx].step_count < MAX_WORKFLOW_STEPS) {
            /* Trim leading/trailing spaces */
            while (*step == ' ') step++;
            if (*step) {
                char* end = step + strlen(step) - 1;
                while (end > step && *end == ' ') *end-- = '\0';
                if (*step) {
                    workflows[idx].steps[workflows[idx].step_count++] = strdup(step);
                }
            }
            step = strtok_r(NULL, ";", &saveptr);
        }
    }

    free(cmd_copy);
}

/* Look up a workflow by name */
static Workflow* get_workflow(const char* name)
{
    for (int i = 0; i < workflow_count; i++) {
        if (strcmp(workflows[i].name, name) == 0) {
            return &workflows[i];
        }
    }
    return NULL;
}

/* Free workflow memory */
static void free_workflows(void)
{
    for (int i = 0; i < workflow_count; i++) {
        free(workflows[i].name);
        for (int j = 0; j < workflows[i].step_count; j++) {
            free(workflows[i].steps[j]);
        }
    }
    workflow_count = 0;
}

/* ============================================================================
 * Plugin System Functions
 * ============================================================================
 * Load, manage, and execute plugins from ~/.cc-bash/plugins/
 */

/* Forward declaration for execute_command */
static int execute_command(const char* cmd);

/* Free plugin memory */
static void free_plugins(void)
{
    for (int i = 0; i < plugin_count; i++) {
        free(plugins[i].name);
        free(plugins[i].version);
        free(plugins[i].description);
        free(plugins[i].path);
        for (int j = 0; j < plugins[i].command_count; j++) {
            free(plugins[i].commands[j].name);
            free(plugins[i].commands[j].script_path);
            free(plugins[i].commands[j].description);
        }
        for (int j = 0; j < MAX_PLUGIN_HOOKS; j++) {
            if (plugins[i].hook_scripts[j]) {
                free(plugins[i].hook_scripts[j]);
            }
        }
    }
    plugin_count = 0;
}

/* Parse a config file (plugin.conf or config.conf) into current state */
static void parse_plugin_config(const char* filepath, const char* plugin_path)
{
    FILE* fp = fopen(filepath, "r");
    if (!fp) return;

    char line[CONFIG_LINE_SIZE];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#') continue;

        /* Handle alias, snippet, workflow - same syntax as main config */
        if (strncmp(p, "alias ", 6) == 0) {
            p += 6;
            while (*p == ' ') p++;
            char* eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char* name = p;
                char* cmd = eq + 1;
                size_t cmd_len = strlen(cmd);
                if (cmd_len >= 2 &&
                    ((cmd[0] == '\'' && cmd[cmd_len-1] == '\'') ||
                     (cmd[0] == '"' && cmd[cmd_len-1] == '"'))) {
                    cmd[cmd_len-1] = '\0';
                    cmd++;
                }
                add_alias(name, cmd);
            }
        }
        else if (strncmp(p, "snippet ", 8) == 0) {
            p += 8;
            while (*p == ' ') p++;
            char* eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char* name = p;
                char* tmpl = eq + 1;
                size_t tmpl_len = strlen(tmpl);
                if (tmpl_len >= 2 &&
                    ((tmpl[0] == '\'' && tmpl[tmpl_len-1] == '\'') ||
                     (tmpl[0] == '"' && tmpl[tmpl_len-1] == '"'))) {
                    tmpl[tmpl_len-1] = '\0';
                    tmpl++;
                }
                add_snippet(name, tmpl);
            }
        }
        else if (strncmp(p, "workflow ", 9) == 0) {
            p += 9;
            while (*p == ' ') p++;
            char* eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char* name = p;
                char* cmds = eq + 1;
                size_t cmds_len = strlen(cmds);
                if (cmds_len >= 2 &&
                    ((cmds[0] == '\'' && cmds[cmds_len-1] == '\'') ||
                     (cmds[0] == '"' && cmds[cmds_len-1] == '"'))) {
                    cmds[cmds_len-1] = '\0';
                    cmds++;
                }
                add_workflow(name, cmds);
            }
        }
    }

    (void)plugin_path;  /* For future use */
    fclose(fp);
}

/* Load a single plugin from a directory */
static int load_plugin(const char* plugin_dir)
{
    if (plugin_count >= MAX_PLUGINS) return -1;

    /* Check for plugin.conf */
    char manifest_path[PATH_MAX];
    snprintf(manifest_path, sizeof(manifest_path), "%s/plugin.conf", plugin_dir);

    FILE* fp = fopen(manifest_path, "r");
    if (!fp) return -1;  /* No manifest, not a valid plugin */

    Plugin* p = &plugins[plugin_count];
    memset(p, 0, sizeof(Plugin));
    p->path = strdup(plugin_dir);
    p->enabled = 1;

    /* Parse plugin.conf for metadata and hooks */
    char line[CONFIG_LINE_SIZE];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        char* ptr = line;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        if (*ptr == '\0' || *ptr == '#') continue;

        /* Parse key=value */
        char* eq = strchr(ptr, '=');
        if (!eq) continue;
        *eq = '\0';
        char* key = ptr;
        char* val = eq + 1;

        /* Strip quotes from value */
        size_t val_len = strlen(val);
        if (val_len >= 2 &&
            ((val[0] == '\'' && val[val_len-1] == '\'') ||
             (val[0] == '"' && val[val_len-1] == '"'))) {
            val[val_len-1] = '\0';
            val++;
        }

        /* Metadata fields */
        if (strcmp(key, "name") == 0) {
            p->name = strdup(val);
        } else if (strcmp(key, "version") == 0) {
            p->version = strdup(val);
        } else if (strcmp(key, "description") == 0) {
            p->description = strdup(val);
        } else if (strcmp(key, "enabled") == 0) {
            p->enabled = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
        }
        /* Hook scripts */
        else if (strncmp(key, "hook.", 5) == 0) {
            char* hook_type = key + 5;
            char script_path[PATH_MAX];
            snprintf(script_path, sizeof(script_path), "%s/%s", plugin_dir, val);

            if (strcmp(hook_type, "startup") == 0) {
                p->hook_scripts[EVENT_STARTUP] = strdup(script_path);
            } else if (strcmp(hook_type, "shutdown") == 0) {
                p->hook_scripts[EVENT_SHUTDOWN] = strdup(script_path);
            } else if (strcmp(hook_type, "pre_command") == 0) {
                p->hook_scripts[EVENT_PRE_COMMAND] = strdup(script_path);
            } else if (strcmp(hook_type, "post_command") == 0) {
                p->hook_scripts[EVENT_POST_COMMAND] = strdup(script_path);
            } else if (strcmp(hook_type, "cd") == 0) {
                p->hook_scripts[EVENT_CD] = strdup(script_path);
            }
        }
        /* Custom commands */
        else if (strncmp(key, "command.", 8) == 0 && p->command_count < MAX_PLUGIN_COMMANDS) {
            char* cmd_name = key + 8;
            char script_path[PATH_MAX];
            snprintf(script_path, sizeof(script_path), "%s/%s", plugin_dir, val);

            p->commands[p->command_count].name = strdup(cmd_name);
            p->commands[p->command_count].script_path = strdup(script_path);
            p->commands[p->command_count].description = NULL;
            p->command_count++;
        }
    }
    fclose(fp);

    /* Set default name if not specified */
    if (!p->name) {
        /* Extract directory name as plugin name */
        const char* last_slash = strrchr(plugin_dir, '/');
        p->name = strdup(last_slash ? last_slash + 1 : plugin_dir);
    }
    if (!p->version) p->version = strdup("1.0");
    if (!p->description) p->description = strdup("");

    /* Load plugin's config.conf for aliases/snippets/workflows */
    char config_path[PATH_MAX];
    snprintf(config_path, sizeof(config_path), "%s/config.conf", plugin_dir);
    if (p->enabled) {
        parse_plugin_config(config_path, plugin_dir);
    }

    plugin_count++;
    return 0;
}

/* Load all plugins from plugin directory */
static void load_plugins(void)
{
    const char* home = getenv("HOME");
    if (!home) return;

    char plugins_dir[PATH_MAX];
    snprintf(plugins_dir, sizeof(plugins_dir), "%s/%s", home, PLUGIN_DIR);

    DIR* dir = opendir(plugins_dir);
    if (!dir) return;  /* Plugin directory doesn't exist, that's fine */

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. */
        if (entry->d_name[0] == '.') continue;

        /* Build full path */
        char plugin_path[PATH_MAX];
        snprintf(plugin_path, sizeof(plugin_path), "%s/%s", plugins_dir, entry->d_name);

        /* Check if it's a directory */
        struct stat st;
        if (stat(plugin_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            load_plugin(plugin_path);
        }
    }

    closedir(dir);
}

/* Execute plugin hooks for an event type */
static void execute_plugin_hooks(EventType type, const Event* event)
{
    for (int i = 0; i < plugin_count; i++) {
        if (!plugins[i].enabled) continue;
        if (!plugins[i].hook_scripts[type]) continue;

        /* Build command with event data as environment variables */
        char cmd[INPUT_BUF_SIZE * 2];
        const char* script = plugins[i].hook_scripts[type];

        /* Set environment variables based on event type */
        if (type == EVENT_PRE_COMMAND || type == EVENT_POST_COMMAND) {
            snprintf(cmd, sizeof(cmd),
                     "CCBASH_COMMAND='%s' CCBASH_EXIT_CODE=%d /bin/sh '%s' 2>/dev/null",
                     event->command ? event->command : "",
                     event->exit_code,
                     script);
        } else if (type == EVENT_CD) {
            snprintf(cmd, sizeof(cmd),
                     "CCBASH_OLD_CWD='%s' CCBASH_NEW_CWD='%s' /bin/sh '%s' 2>/dev/null",
                     event->old_cwd ? event->old_cwd : "",
                     event->new_cwd ? event->new_cwd : "",
                     script);
        } else {
            snprintf(cmd, sizeof(cmd), "/bin/sh '%s' 2>/dev/null", script);
        }

        /* Execute hook script (ignore return value) */
        system(cmd);
    }
}

/* Find a plugin command by name */
static PluginCommand* find_plugin_command(const char* name)
{
    for (int i = 0; i < plugin_count; i++) {
        if (!plugins[i].enabled) continue;
        for (int j = 0; j < plugins[i].command_count; j++) {
            if (strcmp(plugins[i].commands[j].name, name) == 0) {
                return &plugins[i].commands[j];
            }
        }
    }
    return NULL;
}

/* Parse and load config file
 * Supports:
 *   alias name='command'
 *   alias name="command"
 *   export VAR=value
 *   # comments
 */
static void load_config(void)
{
    char config_path[PATH_MAX];
    const char* home = getenv("HOME");
    if (!home) return;

    snprintf(config_path, sizeof(config_path), "%s/.cc-bashrc", home);

    FILE* fp = fopen(config_path, "r");
    if (!fp) return;  /* Config file doesn't exist, that's fine */

    char line[CONFIG_LINE_SIZE];
    while (fgets(line, sizeof(line), fp)) {
        /* Remove trailing newline */
        line[strcspn(line, "\n")] = '\0';

        /* Skip empty lines and comments */
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#') continue;

        /* Handle 'alias name=command' */
        if (strncmp(p, "alias ", 6) == 0) {
            p += 6;
            while (*p == ' ') p++;

            char* eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char* name = p;
                char* cmd = eq + 1;

                /* Strip quotes from command */
                size_t cmd_len = strlen(cmd);
                if (cmd_len >= 2 &&
                    ((cmd[0] == '\'' && cmd[cmd_len-1] == '\'') ||
                     (cmd[0] == '"' && cmd[cmd_len-1] == '"'))) {
                    cmd[cmd_len-1] = '\0';
                    cmd++;
                }

                add_alias(name, cmd);
            }
        }
        /* Handle 'export VAR=value' */
        else if (strncmp(p, "export ", 7) == 0) {
            p += 7;
            while (*p == ' ') p++;

            char* eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char* var = p;
                char* val = eq + 1;

                /* Strip quotes from value */
                size_t val_len = strlen(val);
                if (val_len >= 2 &&
                    ((val[0] == '\'' && val[val_len-1] == '\'') ||
                     (val[0] == '"' && val[val_len-1] == '"'))) {
                    val[val_len-1] = '\0';
                    val++;
                }

                setenv(var, val, 1);
            }
        }
        /* Handle 'theme.field=value' */
        else if (strncmp(p, "theme.", 6) == 0) {
            p += 6;  /* Skip "theme." */

            char* eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char* field = p;
                char* val = eq + 1;

                /* Strip quotes from value */
                size_t val_len = strlen(val);
                if (val_len >= 2 &&
                    ((val[0] == '\'' && val[val_len-1] == '\'') ||
                     (val[0] == '"' && val[val_len-1] == '"'))) {
                    val[val_len-1] = '\0';
                    val++;
                }

                set_theme_color(field, val);
            }
        }
        /* Handle 'snippet name=template' */
        else if (strncmp(p, "snippet ", 8) == 0) {
            p += 8;
            while (*p == ' ') p++;

            char* eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char* name = p;
                char* tmpl = eq + 1;

                /* Strip quotes from template */
                size_t tmpl_len = strlen(tmpl);
                if (tmpl_len >= 2 &&
                    ((tmpl[0] == '\'' && tmpl[tmpl_len-1] == '\'') ||
                     (tmpl[0] == '"' && tmpl[tmpl_len-1] == '"'))) {
                    tmpl[tmpl_len-1] = '\0';
                    tmpl++;
                }

                add_snippet(name, tmpl);
            }
        }
        /* Handle 'workflow name=cmd1 && cmd2' */
        else if (strncmp(p, "workflow ", 9) == 0) {
            p += 9;
            while (*p == ' ') p++;

            char* eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char* name = p;
                char* cmds = eq + 1;

                /* Strip quotes from commands */
                size_t cmds_len = strlen(cmds);
                if (cmds_len >= 2 &&
                    ((cmds[0] == '\'' && cmds[cmds_len-1] == '\'') ||
                     (cmds[0] == '"' && cmds[cmds_len-1] == '"'))) {
                    cmds[cmds_len-1] = '\0';
                    cmds++;
                }

                add_workflow(name, cmds);
            }
        }
    }

    fclose(fp);
}

/* Free alias memory */
static void free_aliases(void)
{
    for (int i = 0; i < alias_count; i++) {
        free(aliases[i].name);
        free(aliases[i].command);
    }
    alias_count = 0;
}

/* ============================================================================
 * Persistent History
 * ============================================================================
 * Load/save command history from ~/.cc-bash-history
 */

/* Load history from file */
static void load_history(void)
{
    char path[PATH_MAX];
    const char* home = getenv("HOME");
    if (!home) return;

    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILE);

    FILE* fp = fopen(path, "r");
    if (!fp) return;

    char line[INPUT_BUF_SIZE];
    while (fgets(line, sizeof(line), fp) && history_count < MAX_HISTORY) {
        /* Remove trailing newline */
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) > 0) {
            history[history_count++] = strdup(line);
        }
    }
    history_pos = history_count;

    fclose(fp);
}

/* Save history to file */
static void save_history(void)
{
    char path[PATH_MAX];
    const char* home = getenv("HOME");
    if (!home) return;

    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILE);

    FILE* fp = fopen(path, "w");
    if (!fp) return;

    /* Save last MAX_HISTORY entries */
    int start = (history_count > MAX_HISTORY) ? history_count - MAX_HISTORY : 0;
    for (int i = start; i < history_count; i++) {
        fprintf(fp, "%s\n", history[i]);
    }

    fclose(fp);
}

/* ============================================================================
 * Screen Drawing Functions
 * ============================================================================
 * Functions for drawing the TUI elements: separators, status bar, and output.
 */

/* Draw separator line using Unicode box-drawing character */
static void draw_separator(int row)
{
    cursor_move(row, 1);
    printf("%s", theme.dim);
    for (int i = 0; i < term_cols; i++) printf("─");
    printf("%s", RESET);
    fflush(stdout);
}

/* Draw status bar (bottom 2 lines) */
static void draw_status(void)
{
    char hostname[64];
    gethostname(hostname, sizeof(hostname));
    const char* user = getenv("USER");
    if (!user) user = "user";

    /* Line 1: user@host:path */
    cursor_move(term_rows - 1, 1);
    clear_line();
    printf("  %s%s@%s%s:%s", theme.status, user, hostname, RESET, cwd);

    /* Line 2: hint */
    cursor_move(term_rows, 1);
    clear_line();
    printf("  %s⏵⏵ @help for commands, exit to quit%s", theme.dim, RESET);

    fflush(stdout);
}

/* Initialize screen layout
 * Sets up the three-region TUI: clears screen, draws separators, draws status
 * bar, and establishes scroll region for the output area.
 * The scroll region (\033[1;Nr) restricts scrolling to rows 1 through N.
 */
static void init_screen(void)
{
    get_term_size();

    /* Clear screen */
    printf("\033[2J");

    /* Draw separators */
    draw_separator(prompt_row - 1);  /* Above prompt */
    draw_separator(prompt_row + 1);  /* Below prompt */

    /* Draw status */
    draw_status();

    /* Set scroll region for output area (top portion only) */
    printf("\033[1;%dr", prompt_row - 2);

    /* Move cursor to prompt */
    cursor_move(prompt_row, 1);
    printf("› ");
    fflush(stdout);
}

/* Restore terminal
 * Resets the scroll region to full screen and moves cursor to bottom.
 * Called on exit to leave terminal in a usable state.
 */
static void cleanup_screen(void)
{
    /* Reset scroll region */
    printf("\033[r");
    cursor_move(term_rows, 1);
    printf("\n%scc-bash exited.%s\n", DIM, RESET);
    fflush(stdout);
}

/* ============================================================================
 * Output Scrollback - Issue #18
 * ============================================================================
 * Stores output lines in a circular buffer for scrollback access.
 * Users can scroll through output history using PgUp/PgDn keys.
 */

/* Add line to output buffer (circular) */
static void add_to_output_buffer(const char* text, int is_stderr)
{
    int index;
    if (output_count < MAX_OUTPUT_LINES) {
        index = output_count;
        output_count++;
    } else {
        /* Buffer full - overwrite oldest line */
        index = output_start;
        free(output_buffer[index].text);
        output_start = (output_start + 1) % MAX_OUTPUT_LINES;
    }
    output_buffer[index].text = strdup(text);
    output_buffer[index].is_stderr = is_stderr;
}

/* Get line from buffer by logical index (0 = oldest) */
static OutputLine* get_output_line(int logical_index)
{
    if (logical_index < 0 || logical_index >= output_count) {
        return NULL;
    }
    int actual_index = (output_start + logical_index) % MAX_OUTPUT_LINES;
    return &output_buffer[actual_index];
}

/* Redraw output area from buffer with current scroll offset */
static void redraw_output(void)
{
    /* Calculate which lines to display */
    int visible_lines = output_rows;
    int total_lines = output_count;

    /* Determine start line based on scroll offset */
    int end_line = total_lines - scroll_offset;
    int start_line = end_line - visible_lines;

    if (start_line < 0) start_line = 0;
    if (end_line < 0) end_line = 0;

    /* Clear and redraw output area */
    for (int row = 1; row <= visible_lines; row++) {
        cursor_move(row, 1);
        clear_line();

        int line_index = start_line + (row - 1);
        if (line_index < end_line && line_index < total_lines) {
            OutputLine* line = get_output_line(line_index);
            if (line && line->text) {
                if (line->is_stderr) {
                    printf("%s%s%s", theme.error, line->text, RESET);
                } else {
                    printf("%s", line->text);
                }
            }
        }
    }

    /* Show scroll indicator in status area if scrolled up */
    cursor_move(term_rows, 1);
    clear_line();
    if (scroll_offset > 0) {
        printf("  %s[Scrolled: %d/%d lines - PgDn to scroll down]%s",
               theme.scroll, scroll_offset, total_lines, RESET);
    } else {
        printf("  %s>> run bash commands (exit to quit)%s", theme.dim, RESET);
    }

    fflush(stdout);
}

/* Scroll output by delta lines (positive = up, negative = down) */
static void scroll_output(int delta)
{
    int max_scroll = output_count - output_rows;
    if (max_scroll < 0) max_scroll = 0;

    scroll_offset += delta;

    /* Clamp to valid range */
    if (scroll_offset < 0) scroll_offset = 0;
    if (scroll_offset > max_scroll) scroll_offset = max_scroll;

    redraw_output();

    /* Restore cursor to prompt */
    cursor_move(prompt_row, 3);
    fflush(stdout);
}

/* Print output in the output region
 *
 * Issue #16 - ANSI Color Passthrough:
 * ANSI escape sequences in text are passed through unchanged, allowing
 * colored output from commands like 'ls --color', 'bat', 'glow' to display.
 * The %s format preserves all bytes including escape sequences.
 *
 * Note: Many programs only emit colors when isatty() returns true. Since we
 * use pipes (not a PTY), programs may not auto-detect color support. Users
 * can force colors with command flags like 'ls --color=always' or by setting
 * environment variables like CLICOLOR_FORCE=1.
 */
static void print_output(const char* text, int is_stderr)
{
    /* Add to scrollback buffer (Issue #18) */
    add_to_output_buffer(text, is_stderr);

    /* Auto-scroll to bottom when new output arrives */
    scroll_offset = 0;

    cursor_save();

    /* Move to output area (scroll region is already set) */
    cursor_move(prompt_row - 2, 1);

    if (is_stderr) {
        /* Prefix stderr with error color, but still allow embedded ANSI codes */
        printf("%s%s%s\n", theme.error, text, RESET);
    } else {
        /* Pass through stdout as-is, preserving any ANSI escape sequences */
        printf("%s\n", text);
    }

    cursor_restore();
    fflush(stdout);
}

/* ============================================================================
 * Command Execution
 * ============================================================================
 * Functions for executing shell commands and handling builtins (cd, clear).
 * Regular commands are executed via fork/exec with /bin/sh for shell expansion.
 */

/* Execute command via fork/exec
 * Creates pipes for stdout/stderr, forks, and runs the command through /bin/sh.
 * Output is captured line-by-line and displayed in the output region.
 * Returns the exit code of the command.
 */
static int execute_command(const char* cmd)
{
    int stdout_pipe[2], stderr_pipe[2];
    if (pipe(stdout_pipe) < 0 || pipe(stderr_pipe) < 0) {
        print_output("Error: Failed to create pipes", 1);
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        print_output("Error: Failed to fork", 1);
        close(stdout_pipe[0]); close(stdout_pipe[1]);
        close(stderr_pipe[0]); close(stderr_pipe[1]);
        return -1;
    }

    if (pid == 0) {
        /* Child */
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        /*
         * Issue #16 - ANSI Color Passthrough:
         * Set environment variables to encourage color output from child processes.
         * TERM=xterm-256color tells programs the terminal supports 256 colors.
         * COLORTERM=truecolor indicates 24-bit color support.
         * CLICOLOR_FORCE=1 forces color output even when not connected to a TTY.
         * FORCE_COLOR=1 is respected by Node.js and many npm packages.
         *
         * Note: This doesn't make pipes into TTYs, so programs using isatty()
         * may still disable colors. For such programs, use:
         *   - ls --color=always
         *   - glow -s dark (or set GLAMOUR_STYLE=dark)
         *   - grep --color=always
         *   - bat --color=always
         */
        setenv("TERM", "xterm-256color", 0);  /* Don't override if already set */
        setenv("COLORTERM", "truecolor", 0);
        setenv("CLICOLOR_FORCE", "1", 1);     /* Force this one */
        setenv("FORCE_COLOR", "1", 1);        /* Node.js and npm packages */

        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(127);
    }

    /* Parent */
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    FILE* stdout_fp = fdopen(stdout_pipe[0], "r");
    FILE* stderr_fp = fdopen(stderr_pipe[0], "r");

    char line[4096];

    while (fgets(line, sizeof(line), stdout_fp)) {
        line[strcspn(line, "\n")] = '\0';
        print_output(line, 0);
    }

    while (fgets(line, sizeof(line), stderr_fp)) {
        line[strcspn(line, "\n")] = '\0';
        print_output(line, 1);
    }

    fclose(stdout_fp);
    fclose(stderr_fp);

    int status;
    waitpid(pid, &status, 0);

    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* Handle cd builtin command
 * cd must be a builtin because chdir() only affects the current process.
 * If we ran cd via fork/exec, only the child would change directory.
 * Supports ~ expansion for home directory.
 */
static int handle_cd(const char* path)
{
    /* Save old directory for CD event */
    char old_cwd[PATH_MAX];
    strcpy(old_cwd, cwd);

    if (!path || strlen(path) == 0) {
        path = getenv("HOME");
    }

    char expanded[PATH_MAX];
    if (path && path[0] == '~') {
        const char* home = getenv("HOME");
        if (home) {
            snprintf(expanded, sizeof(expanded), "%s%s", home, path + 1);
            path = expanded;
        }
    }

    if (chdir(path) != 0) {
        char err[PATH_MAX + 128];
        snprintf(err, sizeof(err), "cd: %s: %s", path, strerror(errno));
        print_output(err, 1);
        return 1;
    }

    getcwd(cwd, sizeof(cwd));

    /* Emit CD event with old and new directories */
    emit_cd_event(old_cwd, cwd);

    draw_status();
    return 0;
}

/* Print help message for @help command */
static void print_help(void)
{
    char buf[256];
    print_output("", 0);
    snprintf(buf, sizeof(buf), "%scc-bash%s - Claude Code-style bash wrapper", theme.header, RESET);
    print_output(buf, 0);
    print_output("", 0);
    snprintf(buf, sizeof(buf), "%sCommands:%s", theme.status, RESET);
    print_output(buf, 0);
    print_output("  <command>       Execute bash command", 0);
    print_output("  cd <path>       Change directory (~ supported)", 0);
    print_output("  clear           Clear output area (preserves TUI)", 0);
    print_output("  exit / quit     Exit shell", 0);
    print_output("", 0);
    snprintf(buf, sizeof(buf), "%s@ Commands:%s", theme.status, RESET);
    print_output(buf, 0);
    print_output("  @help / @h      Show this help", 0);
    print_output("  @clear / @c     Clear output area", 0);
    print_output("  @quit / @q      Exit shell", 0);
    print_output("  @alias          List aliases", 0);
    print_output("  @alias x='cmd'  Add alias (session only)", 0);
    print_output("  @snippet        List snippets", 0);
    print_output("  @snippet name args  Run snippet with arguments", 0);
    print_output("  @theme          Show current theme", 0);
    print_output("  @hooks          Show registered event hooks", 0);
    print_output("  @workflow       List workflows", 0);
    print_output("  @workflow name  Run workflow (--dry-run to preview)", 0);
    print_output("  @plugins        List loaded plugins", 0);
    print_output("", 0);
    snprintf(buf, sizeof(buf), "%sFiles:%s", theme.status, RESET);
    print_output(buf, 0);
    print_output("  ~/.cc-bashrc         Config (aliases, exports, snippets, theme, workflows)", 0);
    print_output("  ~/.cc-bash-history   Command history (auto-saved)", 0);
    print_output("", 0);
    snprintf(buf, sizeof(buf), "%sSpecial:%s", theme.status, RESET);
    print_output(buf, 0);
    print_output("  # <note>        Comment (displayed, not executed)", 0);
    print_output("  Tab             File/command completion", 0);
    print_output("  PgUp/PgDn       Scroll output history", 0);
    print_output("  Up/Down         Command history", 0);
    print_output("", 0);
}

/* Handle clear command - Issue #17
 *
 * The standard 'clear' command sends escape sequences that clear the entire
 * screen, which disrupts our TUI layout (separators, status bar, scroll region).
 *
 * Our builtin 'clear' only clears the output scroll region, then redraws
 * the TUI frame (separators and status bar) to keep the layout intact.
 *
 * Issue #18: Also clears the scrollback buffer since the visible output is gone.
 */
static void handle_clear(void)
{
    cursor_save();

    /* Clear the scrollback buffer (Issue #18) */
    for (int i = 0; i < output_count; i++) {
        int index = (output_start + i) % MAX_OUTPUT_LINES;
        free(output_buffer[index].text);
        output_buffer[index].text = NULL;
    }
    output_count = 0;
    output_start = 0;
    scroll_offset = 0;

    /* Clear only the output area (rows 1 to prompt_row - 2) */
    for (int row = 1; row <= prompt_row - 2; row++) {
        cursor_move(row, 1);
        clear_line();
    }

    /* Move cursor to top of output area */
    cursor_move(1, 1);

    /* Redraw the TUI frame to ensure it's intact */
    draw_separator(prompt_row - 1);  /* Above prompt */
    draw_separator(prompt_row + 1);  /* Below prompt */
    draw_status();

    /* Re-establish scroll region */
    printf("\033[1;%dr", prompt_row - 2);

    cursor_restore();

    /* Position cursor at prompt */
    cursor_move(prompt_row, 1);
    clear_line();
    printf("› ");
    fflush(stdout);
}

/* ============================================================================
 * Input Processing
 * ============================================================================
 * Functions for reading user input with line editing and history navigation.
 * Uses raw terminal mode for character-by-character input processing.
 */

/* Add command to history buffer */
static void add_history(const char* cmd)
{
    /* Skip if same as last command (no consecutive duplicates) */
    if (history_count > 0 && strcmp(history[history_count - 1], cmd) == 0) {
        history_pos = history_count;
        return;
    }

    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    }
    history_pos = history_count;
}

/* ============================================================================
 * Tab Completion Support - Issue #15
 * ============================================================================
 * Implements file/directory and command completion:
 * - Single Tab: Complete if single match, or extend to common prefix
 * - Double Tab: Show available completions if multiple matches
 * - File completion for paths containing '/' or starting with '.'
 * - Command completion from PATH for first word
 */

/* Completion result structure */
typedef struct {
    char** matches;
    int count;
    int capacity;
} CompletionResult;

/* Initialize completion result */
static void completion_init(CompletionResult* cr)
{
    cr->matches = malloc(sizeof(char*) * MAX_COMPLETIONS);
    cr->count = 0;
    cr->capacity = MAX_COMPLETIONS;
}

/* Add a match to completion result */
static void completion_add(CompletionResult* cr, const char* match)
{
    if (cr->count < cr->capacity) {
        cr->matches[cr->count++] = strdup(match);
    }
}

/* Free completion result */
static void completion_free(CompletionResult* cr)
{
    for (int i = 0; i < cr->count; i++) {
        free(cr->matches[i]);
    }
    free(cr->matches);
    cr->matches = NULL;
    cr->count = 0;
}

/* Check if file is executable */
static int is_executable(const char* path)
{
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return (st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH);
}

/* Check if path is a directory */
static int is_directory(const char* path)
{
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

/* Complete file/directory names */
static void complete_files(const char* prefix, CompletionResult* cr)
{
    char dir_path[PATH_MAX];
    char file_prefix[PATH_MAX];
    const char* slash = strrchr(prefix, '/');

    if (slash) {
        /* Has directory component */
        size_t dir_len = (size_t)(slash - prefix + 1);
        strncpy(dir_path, prefix, dir_len);
        dir_path[dir_len] = '\0';
        strcpy(file_prefix, slash + 1);
    } else {
        /* Current directory */
        strcpy(dir_path, ".");
        strcpy(file_prefix, prefix);
    }

    /* Handle ~ expansion for directory */
    char expanded_dir[PATH_MAX];
    if (dir_path[0] == '~') {
        const char* home = getenv("HOME");
        if (home) {
            snprintf(expanded_dir, sizeof(expanded_dir), "%s%s", home, dir_path + 1);
            strcpy(dir_path, expanded_dir);
        }
    }

    DIR* dir = opendir(dir_path);
    if (!dir) return;

    size_t prefix_len = strlen(file_prefix);
    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. unless explicitly typed */
        if (entry->d_name[0] == '.' && file_prefix[0] != '.') continue;

        if (strncmp(entry->d_name, file_prefix, prefix_len) == 0) {
            char full_match[PATH_MAX];

            if (slash) {
                /* Include the directory part in the completion */
                size_t dir_len = (size_t)(slash - prefix + 1);
                strncpy(full_match, prefix, dir_len);
                full_match[dir_len] = '\0';
                strcat(full_match, entry->d_name);
            } else {
                strcpy(full_match, entry->d_name);
            }

            /* Add trailing slash for directories */
            char check_path[PATH_MAX * 2];  /* Extra room to avoid truncation warning */
            if (strcmp(dir_path, ".") == 0) {
                snprintf(check_path, sizeof(check_path), "%s", entry->d_name);
            } else {
                snprintf(check_path, sizeof(check_path), "%s/%s", dir_path, entry->d_name);
            }
            if (is_directory(check_path)) {
                strcat(full_match, "/");
            }

            completion_add(cr, full_match);
        }
    }

    closedir(dir);
}

/* Complete command names from PATH */
static void complete_commands(const char* prefix, CompletionResult* cr)
{
    const char* path_env = getenv("PATH");
    if (!path_env) return;

    size_t prefix_len = strlen(prefix);
    char* path_copy = strdup(path_env);
    char* saveptr;
    char* dir = strtok_r(path_copy, ":", &saveptr);

    while (dir) {
        DIR* d = opendir(dir);
        if (d) {
            struct dirent* entry;
            while ((entry = readdir(d)) != NULL) {
                if (strncmp(entry->d_name, prefix, prefix_len) == 0) {
                    /* Check if executable */
                    char full_path[PATH_MAX];
                    snprintf(full_path, sizeof(full_path), "%s/%s", dir, entry->d_name);
                    if (is_executable(full_path)) {
                        /* Avoid duplicates */
                        int found = 0;
                        for (int i = 0; i < cr->count; i++) {
                            if (strcmp(cr->matches[i], entry->d_name) == 0) {
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            completion_add(cr, entry->d_name);
                        }
                    }
                }
            }
            closedir(d);
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(path_copy);
}

/* Find longest common prefix among completions */
static void find_common_prefix(CompletionResult* cr, char* common, size_t common_size)
{
    if (cr->count == 0) {
        common[0] = '\0';
        return;
    }

    strncpy(common, cr->matches[0], common_size - 1);
    common[common_size - 1] = '\0';

    for (int i = 1; i < cr->count; i++) {
        size_t j = 0;
        while (common[j] && cr->matches[i][j] && common[j] == cr->matches[i][j]) {
            j++;
        }
        common[j] = '\0';
    }
}

/* Extract word at cursor position for completion */
static void get_word_at_cursor(const char* buf, int pos, char* word, int* word_start)
{
    /* Find start of word (after last space or at beginning) */
    int start = pos;
    while (start > 0 && buf[start - 1] != ' ') {
        start--;
    }

    *word_start = start;
    int len = pos - start;
    strncpy(word, buf + start, (size_t)len);
    word[len] = '\0';
}

/* Perform tab completion - returns number of matches */
static int do_completion(const char* buf, int pos, int is_double_tab,
                         char* completion, int* word_start)
{
    char word[PATH_MAX];
    get_word_at_cursor(buf, pos, word, word_start);

    if (strlen(word) == 0) {
        completion[0] = '\0';
        return 0;
    }

    CompletionResult cr;
    completion_init(&cr);

    /* Determine completion type:
     * - If word contains '/' or starts with '.', do file completion
     * - If at start of line (first word), do command completion first, then files
     * - Otherwise, do file completion
     */
    int at_start = (*word_start == 0);

    if (strchr(word, '/') || word[0] == '.') {
        /* File/directory completion */
        complete_files(word, &cr);
    } else if (at_start) {
        /* First word - try command completion first */
        complete_commands(word, &cr);
        /* Also add file completions (for running scripts, etc.) */
        complete_files(word, &cr);
    } else {
        /* Not first word - file completion only */
        complete_files(word, &cr);
    }

    int match_count = cr.count;

    if (match_count == 0) {
        completion[0] = '\0';
    } else if (match_count == 1) {
        /* Single match - complete fully */
        strcpy(completion, cr.matches[0]);
    } else {
        /* Multiple matches */
        char common[PATH_MAX];
        find_common_prefix(&cr, common, sizeof(common));

        if (strlen(common) > strlen(word)) {
            /* Can extend with common prefix */
            strcpy(completion, common);
        } else if (is_double_tab) {
            /* Show available completions on double-tab */
            cursor_save();
            cursor_move(prompt_row - 2, 1);
            printf("%sCompletions:%s ", theme.dim, RESET);
            for (int i = 0; i < cr.count && i < 10; i++) {
                printf("%s ", cr.matches[i]);
            }
            if (cr.count > 10) {
                printf("... (%d more)", cr.count - 10);
            }
            printf("\n");
            cursor_restore();
            fflush(stdout);
            completion[0] = '\0';
        } else {
            /* First tab with no extension - mark for double-tab */
            strcpy(completion, word);
        }
    }

    completion_free(&cr);
    return match_count;
}

/* Draw prompt with optional exit code
 * Shows "[N] › " if last_exit is non-zero, otherwise just "› "
 * Returns the cursor column after the prompt (for positioning)
 */
static int draw_prompt(void)
{
    cursor_move(prompt_row, 1);
    clear_line();
    if (last_exit != 0) {
        printf("%s[%d]%s %s›%s ", theme.error, last_exit, RESET, theme.prompt, RESET);
        return 7 + (last_exit >= 10 ? 1 : 0) + (last_exit >= 100 ? 1 : 0);
    } else {
        printf("%s›%s ", theme.prompt, RESET);
        return 3;
    }
}

/* Read a line with basic editing
 * Handles: backspace, left/right arrows, up/down for history, Tab completion,
 * PgUp/PgDn for scrollback, Ctrl+C, Ctrl+D.
 * Returns NULL on EOF (Ctrl+D), otherwise returns the input buffer.
 * Note: Uses a static buffer - caller should not free or store the pointer.
 */
static char* read_input(void)
{
    static char buf[INPUT_BUF_SIZE];
    static int last_was_tab = 0;  /* Track double-tab for completions */
    int pos = 0;
    int len = 0;
    int prompt_col;  /* Column after prompt (varies with exit code) */

    buf[0] = '\0';

    /* Draw prompt with exit code if non-zero */
    prompt_col = draw_prompt();
    fflush(stdout);

    while (1) {
        int c = getchar();

        if (c == EOF || c == 4) {  /* Ctrl+D */
            return NULL;
        }

        if (c == '\n' || c == '\r') {
            buf[len] = '\0';
            last_was_tab = 0;

            /* Scroll the prompt into output area */
            cursor_save();
            cursor_move(prompt_row - 2, 1);
            printf("› %s\n", buf);
            cursor_restore();

            return buf;
        }

        /* Tab completion - Issue #15 */
        if (c == '\t') {
            char completion[PATH_MAX];
            int word_start;
            int matches = do_completion(buf, pos, last_was_tab, completion, &word_start);

            if (matches > 0 && strlen(completion) > 0) {
                /* Replace word with completion */
                int word_len = pos - word_start;
                int comp_len = strlen(completion);

                /* Remove old word and insert completion */
                memmove(&buf[word_start + comp_len], &buf[pos], len - pos + 1);
                memcpy(&buf[word_start], completion, comp_len);

                len = len - word_len + comp_len;
                pos = word_start + comp_len;

                /* Redraw line with prompt */
                prompt_col = draw_prompt();
                printf("%s", buf);
                cursor_move(prompt_row, prompt_col + pos);
                fflush(stdout);
            }

            last_was_tab = 1;
            continue;
        }

        /* Reset tab tracking for non-tab keys */
        last_was_tab = 0;

        if (c == 127 || c == 8) {  /* Backspace */
            if (pos > 0) {
                memmove(&buf[pos-1], &buf[pos], len - pos + 1);
                pos--;
                len--;
                cursor_move(prompt_row, prompt_col);
                printf("%s ", buf);
                cursor_move(prompt_row, prompt_col + pos);
            }
            fflush(stdout);
            continue;
        }

        if (c == 27) {  /* Escape sequence */
            int c2 = getchar();
            if (c2 == '[') {
                int c3 = getchar();
                if (c3 == 'A') {  /* Up arrow - history */
                    if (history_pos > 0) {
                        history_pos--;
                        strcpy(buf, history[history_pos]);
                        len = pos = strlen(buf);
                        prompt_col = draw_prompt();
                        printf("%s", buf);
                        fflush(stdout);
                    }
                } else if (c3 == 'B') {  /* Down arrow */
                    if (history_pos < history_count - 1) {
                        history_pos++;
                        strcpy(buf, history[history_pos]);
                        len = pos = strlen(buf);
                    } else {
                        history_pos = history_count;
                        buf[0] = '\0';
                        len = pos = 0;
                    }
                    prompt_col = draw_prompt();
                    printf("%s", buf);
                    fflush(stdout);
                } else if (c3 == 'C') {  /* Right arrow */
                    if (pos < len) {
                        pos++;
                        cursor_move(prompt_row, prompt_col + pos);
                        fflush(stdout);
                    }
                } else if (c3 == 'D') {  /* Left arrow */
                    if (pos > 0) {
                        pos--;
                        cursor_move(prompt_row, prompt_col + pos);
                        fflush(stdout);
                    }
                } else if (c3 == '5' || c3 == '6') {
                    /* PgUp/PgDn - Issue #18: Output scrollback
                     * PgUp: \033[5~  PgDn: \033[6~
                     * Shift+PgUp/PgDn: \033[5;2~ and \033[6;2~ (larger jump)
                     */
                    int c4 = getchar();
                    if (c4 == '~') {
                        /* Regular PgUp/PgDn */
                        if (c3 == '5') {
                            scroll_output(SCROLL_PAGE_SIZE);  /* PgUp - scroll up */
                        } else {
                            scroll_output(-SCROLL_PAGE_SIZE); /* PgDn - scroll down */
                        }
                    } else if (c4 == ';') {
                        /* Modifier present (e.g., Shift) */
                        int c5 = getchar();  /* modifier number */
                        int c6 = getchar();  /* should be ~ */
                        if (c5 == '2' && c6 == '~') {
                            /* Shift+PgUp/PgDn - larger jump */
                            if (c3 == '5') {
                                scroll_output(SCROLL_LARGE_JUMP);
                            } else {
                                scroll_output(-SCROLL_LARGE_JUMP);
                            }
                        }
                    }
                }
            }
            continue;
        }

        if (c == 3) {  /* Ctrl+C */
            buf[0] = '\0';
            prompt_col = draw_prompt();
            fflush(stdout);
            len = pos = 0;
            continue;
        }

        /* Regular character */
        if (len < INPUT_BUF_SIZE - 1 && c >= 32 && c < 127) {
            memmove(&buf[pos+1], &buf[pos], len - pos + 1);
            buf[pos] = c;
            pos++;
            len++;
            cursor_move(prompt_row, prompt_col);
            printf("%s", buf);
            cursor_move(prompt_row, prompt_col + pos);
            fflush(stdout);
        }
    }
}

/* ============================================================================
 * Raw Terminal Mode
 * ============================================================================
 * Functions for switching terminal between raw and cooked (canonical) modes.
 * Raw mode allows character-by-character input without waiting for Enter.
 */

/* Set terminal to raw mode
 * Disables ECHO (don't print typed characters) and ICANON (don't wait for Enter).
 * VMIN=1 means read() returns after 1 character; VTIME=0 means no timeout.
 * Original settings are saved in orig_termios for restoration on exit.
 */
static void enable_raw_mode(void)
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/* Restore terminal to original (cooked) mode */
static void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/* ============================================================================
 * Signal Handling
 * ============================================================================
 * Handle terminal resize (SIGWINCH) to redraw the TUI at new dimensions.
 */

/* Signal handler for window resize */
static volatile sig_atomic_t resize_flag = 0;

static void handle_sigwinch(int sig)
{
    (void)sig;
    resize_flag = 1;
}

/* ============================================================================
 * Main Entry Point
 * ============================================================================
 * The main event loop: read input, process commands, repeat until exit.
 */
int main(void)
{
    getcwd(cwd, sizeof(cwd));

    /* Load config file (~/.cc-bashrc) */
    load_config();

    /* Load plugins (~/.cc-bash/plugins/) */
    load_plugins();

    /* Load command history (~/.cc-bash-history) */
    load_history();

    /* Set up signal handlers */
    signal(SIGWINCH, handle_sigwinch);

    enable_raw_mode();
    init_screen();

    /* Emit startup event - config loaded, screen ready */
    emit_simple_event(EVENT_STARTUP);

    int running = 1;

    while (running) {
        if (resize_flag) {
            resize_flag = 0;
            init_screen();
        }

        char* input = read_input();

        if (!input) {
            running = 0;
            break;
        }

        /* Skip empty */
        if (strlen(input) == 0) {
            continue;
        }

        add_history(input);

        /* Process */
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            running = 0;
        }
        else if (strncmp(input, "cd", 2) == 0 &&
                 (input[2] == '\0' || input[2] == ' ')) {
            const char* path = (input[2] == ' ') ? input + 3 : NULL;
            last_exit = handle_cd(path);
        }
        else if (strcmp(input, "clear") == 0) {
            /* Issue #17: Intercept 'clear' as builtin to preserve TUI frame */
            handle_clear();
            last_exit = 0;
        }
        else if (input[0] == '#') {
            /* Comment - just show it */
            char msg[INPUT_BUF_SIZE + 64];
            snprintf(msg, sizeof(msg), "%s%s%s", theme.comment, input, RESET);
            print_output(msg, 0);
        }
        else if (input[0] == '@') {
            /* @ commands - internal shell commands */
            const char* cmd = input + 1;  /* Skip the @ */
            if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
                print_help();
                last_exit = 0;
            }
            else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "c") == 0) {
                handle_clear();
                last_exit = 0;
            }
            else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
                running = 0;
            }
            else if (strncmp(cmd, "alias", 5) == 0) {
                /* @alias - list or add aliases */
                const char* arg = cmd + 5;
                while (*arg == ' ') arg++;

                if (*arg == '\0') {
                    /* List all aliases */
                    if (alias_count == 0) {
                        print_output("No aliases defined.", 0);
                        print_output("Add to ~/.cc-bashrc: alias ll='ls -la'", 0);
                    } else {
                        char msg[CONFIG_LINE_SIZE];
                        for (int i = 0; i < alias_count; i++) {
                            snprintf(msg, sizeof(msg), "alias %s='%s'",
                                     aliases[i].name, aliases[i].command);
                            print_output(msg, 0);
                        }
                    }
                } else {
                    /* Add new alias: @alias name=command */
                    char* eq = strchr(arg, '=');
                    if (eq) {
                        char name[256];
                        size_t name_len = (size_t)(eq - arg);
                        if (name_len < sizeof(name)) {
                            strncpy(name, arg, name_len);
                            name[name_len] = '\0';
                            const char* cmd_val = eq + 1;
                            /* Strip quotes */
                            size_t cmd_len = strlen(cmd_val);
                            char cmd_clean[CONFIG_LINE_SIZE];
                            if (cmd_len >= 2 &&
                                ((cmd_val[0] == '\'' && cmd_val[cmd_len-1] == '\'') ||
                                 (cmd_val[0] == '"' && cmd_val[cmd_len-1] == '"'))) {
                                strncpy(cmd_clean, cmd_val + 1, cmd_len - 2);
                                cmd_clean[cmd_len - 2] = '\0';
                            } else {
                                strncpy(cmd_clean, cmd_val, sizeof(cmd_clean) - 1);
                                cmd_clean[sizeof(cmd_clean) - 1] = '\0';
                            }
                            add_alias(name, cmd_clean);
                            char msg[CONFIG_LINE_SIZE + 256 + 16];  /* room for name + cmd + "alias =''" */
                            snprintf(msg, sizeof(msg), "alias %s='%s'", name, cmd_clean);
                            print_output(msg, 0);
                        }
                    } else {
                        print_output("Usage: @alias name='command'", 1);
                    }
                }
                last_exit = 0;
            }
            else if (strcmp(cmd, "theme") == 0) {
                /* @theme - show current theme colors */
                char buf[256];
                print_output("", 0);
                snprintf(buf, sizeof(buf), "%sTheme Configuration:%s", theme.status, RESET);
                print_output(buf, 0);
                print_output("", 0);
                snprintf(buf, sizeof(buf), "  %sprompt%s  = (this color)", theme.prompt, RESET);
                print_output(buf, 0);
                snprintf(buf, sizeof(buf), "  %serror%s   = (this color)", theme.error, RESET);
                print_output(buf, 0);
                snprintf(buf, sizeof(buf), "  %scomment%s = (this color)", theme.comment, RESET);
                print_output(buf, 0);
                snprintf(buf, sizeof(buf), "  %sdim%s     = (this color)", theme.dim, RESET);
                print_output(buf, 0);
                snprintf(buf, sizeof(buf), "  %sheader%s  = (this color)", theme.header, RESET);
                print_output(buf, 0);
                snprintf(buf, sizeof(buf), "  %sstatus%s  = (this color)", theme.status, RESET);
                print_output(buf, 0);
                snprintf(buf, sizeof(buf), "  %sscroll%s  = (this color)", theme.scroll, RESET);
                print_output(buf, 0);
                print_output("", 0);
                print_output("Configure in ~/.cc-bashrc:", 0);
                print_output("  theme.prompt=cyan", 0);
                print_output("  theme.error=bold red", 0);
                print_output("  theme.comment=green", 0);
                print_output("", 0);
                print_output("Colors: black red green yellow blue magenta cyan white", 0);
                print_output("Modifiers: bold dim (combine with space: 'bold cyan')", 0);
                print_output("", 0);
                last_exit = 0;
            }
            else if (strncmp(cmd, "snippet", 7) == 0) {
                /* @snippet - list or run snippets */
                const char* arg = cmd + 7;
                while (*arg == ' ') arg++;

                if (*arg == '\0') {
                    /* List all snippets */
                    if (snippet_count == 0) {
                        print_output("No snippets defined.", 0);
                        print_output("", 0);
                        print_output("Add to ~/.cc-bashrc:", 0);
                        print_output("  snippet deploy='git push origin $1'", 0);
                        print_output("  snippet logs='docker logs -f --tail $1 $2'", 0);
                        print_output("", 0);
                        print_output("Usage: @snippet name arg1 arg2 ...", 0);
                    } else {
                        char msg[CONFIG_LINE_SIZE];
                        snprintf(msg, sizeof(msg), "%sSnippets:%s", theme.status, RESET);
                        print_output(msg, 0);
                        for (int i = 0; i < snippet_count; i++) {
                            snprintf(msg, sizeof(msg), "  %s%s%s = %s",
                                     theme.header, snippets[i].name, RESET,
                                     snippets[i].template);
                            print_output(msg, 0);
                        }
                    }
                    last_exit = 0;
                } else {
                    /* Run snippet: @snippet name arg1 arg2 ... */
                    char snippet_name[256];
                    char* args[9] = {NULL};  /* $1 through $9 */
                    int arg_count = 0;

                    /* Parse snippet name and arguments */
                    const char* p = arg;
                    char* dst = snippet_name;
                    while (*p && *p != ' ' && (size_t)(dst - snippet_name) < sizeof(snippet_name) - 1) {
                        *dst++ = *p++;
                    }
                    *dst = '\0';

                    /* Skip space and parse arguments */
                    while (*p == ' ') p++;

                    /* Tokenize remaining arguments */
                    if (*p) {
                        char arg_buf[INPUT_BUF_SIZE];
                        strncpy(arg_buf, p, sizeof(arg_buf) - 1);
                        arg_buf[sizeof(arg_buf) - 1] = '\0';

                        char* token = strtok(arg_buf, " ");
                        while (token && arg_count < 9) {
                            args[arg_count++] = strdup(token);
                            token = strtok(NULL, " ");
                        }
                    }

                    /* Look up and expand snippet */
                    const char* tmpl = get_snippet(snippet_name);
                    if (tmpl) {
                        char* expanded = expand_snippet(tmpl, args, arg_count);
                        if (expanded) {
                            /* Emit snippet expand event */
                            emit_expand_event(EVENT_SNIPPET_EXPAND, snippet_name, expanded);

                            /* Show what we're running */
                            char msg[INPUT_BUF_SIZE + 64];
                            snprintf(msg, sizeof(msg), "%s→ %s%s", theme.dim, expanded, RESET);
                            print_output(msg, 0);

                            /* Execute the expanded command with command events */
                            emit_command_event(EVENT_PRE_COMMAND, expanded, 0);
                            last_exit = execute_command(expanded);
                            emit_command_event(EVENT_POST_COMMAND, expanded, last_exit);
                            free(expanded);
                        } else {
                            print_output("Error: Failed to expand snippet", 1);
                            last_exit = 1;
                        }
                    } else {
                        char msg[512];
                        snprintf(msg, sizeof(msg), "%sUnknown snippet: %s%s", theme.error, snippet_name, RESET);
                        print_output(msg, 1);
                        last_exit = 1;
                    }

                    /* Free argument copies */
                    for (int i = 0; i < arg_count; i++) {
                        free(args[i]);
                    }
                }
            }
            else if (strcmp(cmd, "hooks") == 0) {
                /* @hooks - list registered event hooks (for debugging/introspection) */
                char buf[256];
                print_output("", 0);
                snprintf(buf, sizeof(buf), "%sEvent Hooks:%s", theme.status, RESET);
                print_output(buf, 0);
                print_output("", 0);

                if (hook_count == 0) {
                    print_output("  No hooks registered.", 0);
                } else {
                    for (int i = 0; i < hook_count; i++) {
                        snprintf(buf, sizeof(buf), "  [%d] %s%s%s %s",
                                 i, theme.header, event_type_name(hooks[i].type), RESET,
                                 hooks[i].active ? "(active)" : "(inactive)");
                        print_output(buf, 0);
                    }
                }

                print_output("", 0);
                snprintf(buf, sizeof(buf), "%sEvent Types:%s", theme.status, RESET);
                print_output(buf, 0);
                print_output("  STARTUP        - Shell started, config loaded", 0);
                print_output("  SHUTDOWN       - Shell exiting", 0);
                print_output("  PRE_COMMAND    - Before command execution", 0);
                print_output("  POST_COMMAND   - After command execution (has exit code)", 0);
                print_output("  CD             - Directory changed (has old/new path)", 0);
                print_output("  ALIAS_EXPAND   - Alias was expanded", 0);
                print_output("  SNIPPET_EXPAND - Snippet was expanded", 0);
                print_output("", 0);
                last_exit = 0;
            }
            else if (strncmp(cmd, "workflow", 8) == 0) {
                /* @workflow - list or run workflows */
                const char* arg = cmd + 8;
                while (*arg == ' ') arg++;

                if (*arg == '\0') {
                    /* List all workflows */
                    if (workflow_count == 0) {
                        print_output("No workflows defined.", 0);
                        print_output("", 0);
                        print_output("Add to ~/.cc-bashrc:", 0);
                        print_output("  workflow build='make clean && make && make test'", 0);
                        print_output("  workflow deploy='git add . && git commit -m \"deploy\" && git push'", 0);
                        print_output("", 0);
                        print_output("Usage: @workflow name [--dry-run]", 0);
                    } else {
                        char msg[CONFIG_LINE_SIZE];
                        snprintf(msg, sizeof(msg), "%sWorkflows:%s", theme.status, RESET);
                        print_output(msg, 0);
                        for (int i = 0; i < workflow_count; i++) {
                            snprintf(msg, sizeof(msg), "  %s%s%s (%d steps, %s)",
                                     theme.header, workflows[i].name, RESET,
                                     workflows[i].step_count,
                                     workflows[i].stop_on_error ? "stop on error" : "continue on error");
                            print_output(msg, 0);
                            /* Show steps indented */
                            for (int j = 0; j < workflows[i].step_count; j++) {
                                snprintf(msg, sizeof(msg), "    %s%d.%s %s",
                                         theme.dim, j + 1, RESET, workflows[i].steps[j]);
                                print_output(msg, 0);
                            }
                        }
                    }
                    last_exit = 0;
                } else {
                    /* Run workflow: @workflow name [--dry-run] */
                    char workflow_name[256];
                    int dry_run = 0;

                    /* Parse name and optional --dry-run flag */
                    const char* p = arg;
                    char* dst = workflow_name;
                    while (*p && *p != ' ' && (size_t)(dst - workflow_name) < sizeof(workflow_name) - 1) {
                        *dst++ = *p++;
                    }
                    *dst = '\0';

                    /* Check for --dry-run flag */
                    while (*p == ' ') p++;
                    if (strncmp(p, "--dry-run", 9) == 0) {
                        dry_run = 1;
                    }

                    /* Look up workflow */
                    Workflow* wf = get_workflow(workflow_name);
                    if (wf) {
                        char msg[INPUT_BUF_SIZE + 128];
                        if (dry_run) {
                            snprintf(msg, sizeof(msg), "%s[dry-run] Workflow: %s%s", theme.dim, wf->name, RESET);
                            print_output(msg, 0);
                            for (int j = 0; j < wf->step_count; j++) {
                                snprintf(msg, sizeof(msg), "  %s%d.%s %s",
                                         theme.dim, j + 1, RESET, wf->steps[j]);
                                print_output(msg, 0);
                            }
                            last_exit = 0;
                        } else {
                            /* Execute workflow */
                            snprintf(msg, sizeof(msg), "%s▶ Running workflow: %s%s (%d steps)",
                                     theme.header, wf->name, RESET, wf->step_count);
                            print_output(msg, 0);

                            int workflow_failed = 0;
                            for (int j = 0; j < wf->step_count; j++) {
                                snprintf(msg, sizeof(msg), "%s[%d/%d]%s %s",
                                         theme.status, j + 1, wf->step_count, RESET, wf->steps[j]);
                                print_output(msg, 0);

                                /* Emit pre-command event */
                                emit_command_event(EVENT_PRE_COMMAND, wf->steps[j], 0);

                                /* Execute step */
                                int step_exit = execute_command(wf->steps[j]);

                                /* Emit post-command event */
                                emit_command_event(EVENT_POST_COMMAND, wf->steps[j], step_exit);

                                if (step_exit != 0) {
                                    workflow_failed = 1;
                                    last_exit = step_exit;
                                    if (wf->stop_on_error) {
                                        snprintf(msg, sizeof(msg), "%s✗ Step %d failed (exit %d), stopping workflow%s",
                                                 theme.error, j + 1, step_exit, RESET);
                                        print_output(msg, 1);
                                        break;
                                    } else {
                                        snprintf(msg, sizeof(msg), "%s⚠ Step %d failed (exit %d), continuing...%s",
                                                 theme.comment, j + 1, step_exit, RESET);
                                        print_output(msg, 0);
                                    }
                                }
                            }

                            if (!workflow_failed) {
                                snprintf(msg, sizeof(msg), "%s✓ Workflow completed successfully%s",
                                         GREEN, RESET);
                                print_output(msg, 0);
                                last_exit = 0;
                            }
                        }
                    } else {
                        char msg[512];
                        snprintf(msg, sizeof(msg), "%sUnknown workflow: %s%s", theme.error, workflow_name, RESET);
                        print_output(msg, 1);
                        last_exit = 1;
                    }
                }
            }
            else if (strcmp(cmd, "plugins") == 0) {
                /* @plugins - list loaded plugins */
                char msg[CONFIG_LINE_SIZE];
                if (plugin_count == 0) {
                    print_output("No plugins loaded.", 0);
                    print_output("", 0);
                    print_output("Create plugins in: ~/.cc-bash/plugins/<name>/", 0);
                    print_output("", 0);
                    print_output("Plugin structure:", 0);
                    print_output("  plugin.conf    - Manifest (name, version, hooks)", 0);
                    print_output("  config.conf    - Optional aliases/snippets/workflows", 0);
                    print_output("  hooks/         - Optional event hook scripts", 0);
                } else {
                    snprintf(msg, sizeof(msg), "%sLoaded Plugins:%s", theme.status, RESET);
                    print_output(msg, 0);
                    print_output("", 0);
                    for (int i = 0; i < plugin_count; i++) {
                        snprintf(msg, sizeof(msg), "  %s%s%s v%s %s",
                                 theme.header, plugins[i].name, RESET,
                                 plugins[i].version,
                                 plugins[i].enabled ? "" : "(disabled)");
                        print_output(msg, 0);
                        if (plugins[i].description && strlen(plugins[i].description) > 0) {
                            snprintf(msg, sizeof(msg), "    %s%s%s",
                                     theme.dim, plugins[i].description, RESET);
                            print_output(msg, 0);
                        }
                        /* Show commands */
                        if (plugins[i].command_count > 0) {
                            print_output("    Commands:", 0);
                            for (int j = 0; j < plugins[i].command_count; j++) {
                                snprintf(msg, sizeof(msg), "      @%s",
                                         plugins[i].commands[j].name);
                                print_output(msg, 0);
                            }
                        }
                        /* Show hooks */
                        int has_hooks = 0;
                        for (int j = 0; j < MAX_PLUGIN_HOOKS; j++) {
                            if (plugins[i].hook_scripts[j]) has_hooks = 1;
                        }
                        if (has_hooks) {
                            print_output("    Hooks:", 0);
                            if (plugins[i].hook_scripts[EVENT_STARTUP])
                                print_output("      startup", 0);
                            if (plugins[i].hook_scripts[EVENT_SHUTDOWN])
                                print_output("      shutdown", 0);
                            if (plugins[i].hook_scripts[EVENT_PRE_COMMAND])
                                print_output("      pre_command", 0);
                            if (plugins[i].hook_scripts[EVENT_POST_COMMAND])
                                print_output("      post_command", 0);
                            if (plugins[i].hook_scripts[EVENT_CD])
                                print_output("      cd", 0);
                        }
                    }
                }
                last_exit = 0;
            }
            else {
                /* Check if it's a plugin command */
                PluginCommand* pcmd = find_plugin_command(cmd);
                if (pcmd) {
                    /* Execute plugin command script */
                    char script_cmd[INPUT_BUF_SIZE * 2];
                    snprintf(script_cmd, sizeof(script_cmd), "/bin/sh '%s'", pcmd->script_path);
                    last_exit = execute_command(script_cmd);
                } else {
                    char msg[INPUT_BUF_SIZE + 64];
                    snprintf(msg, sizeof(msg), "%sUnknown @ command: %s%s", theme.error, cmd, RESET);
                    print_output(msg, 1);
                    last_exit = 1;
                }
            }
        }
        else {
            /* Check for alias expansion on first word */
            char first_word[INPUT_BUF_SIZE];
            const char* rest = "";
            char* space = strchr(input, ' ');
            if (space) {
                size_t len = (size_t)(space - input);
                strncpy(first_word, input, len);
                first_word[len] = '\0';
                rest = space;  /* includes the space */
            } else {
                strcpy(first_word, input);
            }

            const char* alias_cmd = get_alias(first_word);
            if (alias_cmd) {
                /* Expand alias */
                char expanded[INPUT_BUF_SIZE];
                snprintf(expanded, sizeof(expanded), "%s%s", alias_cmd, rest);

                /* Emit alias expand event */
                emit_expand_event(EVENT_ALIAS_EXPAND, input, expanded);

                /* Emit pre-command event */
                emit_command_event(EVENT_PRE_COMMAND, expanded, 0);
                last_exit = execute_command(expanded);
                /* Emit post-command event */
                emit_command_event(EVENT_POST_COMMAND, expanded, last_exit);
            } else {
                /* Emit pre-command event */
                emit_command_event(EVENT_PRE_COMMAND, input, 0);
                last_exit = execute_command(input);
                /* Emit post-command event */
                emit_command_event(EVENT_POST_COMMAND, input, last_exit);
            }
        }
    }

    /* Emit shutdown event before cleanup */
    emit_simple_event(EVENT_SHUTDOWN);

    disable_raw_mode();
    cleanup_screen();

    /* Save command history */
    save_history();

    /* Free history */
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    /* Free output buffer (Issue #18) */
    for (int i = 0; i < output_count; i++) {
        int index = (output_start + i) % MAX_OUTPUT_LINES;
        free(output_buffer[index].text);
    }

    /* Free aliases */
    free_aliases();

    /* Free snippets */
    free_snippets();

    /* Free workflows */
    free_workflows();

    /* Free plugins */
    free_plugins();

    return last_exit;
}
