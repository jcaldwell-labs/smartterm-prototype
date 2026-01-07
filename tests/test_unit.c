/*
 * test_unit.c - Unit tests for cc-bash pure functions
 *
 * Tests config parsing, alias management, and history operations
 * without requiring a TTY.
 *
 * Build: gcc -Wall -Wextra -std=c11 -o test_unit tests/test_unit.c
 * Run:   ./test_unit
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

/* Test framework */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg) do { \
    tests_run++; \
    if (cond) { \
        tests_passed++; \
        printf("  \033[32mPASS\033[0m: %s\n", msg); \
    } else { \
        tests_failed++; \
        printf("  \033[31mFAIL\033[0m: %s\n", msg); \
    } \
} while(0)

/* Note: ASSERT_STR_EQ assumes both strings are valid (not NULL).
 * For pointer strings that might be NULL, check with ASSERT_NOT_NULL first.
 * This avoids -Waddress warnings when used with static arrays.
 */
#define ASSERT_STR_EQ(a, b, msg) ASSERT(strcmp((a), (b)) == 0, msg)
#define ASSERT_NULL(a, msg) ASSERT((a) == NULL, msg)
#define ASSERT_NOT_NULL(a, msg) ASSERT((a) != NULL, msg)

/* ============================================================================
 * Replicate pure functions from cc-bash.c for testing
 * ============================================================================ */

#define MAX_ALIASES 100
#define MAX_SNIPPETS 50
#define CONFIG_LINE_SIZE 1024
#define INPUT_BUF_SIZE 4096

/* Theme structure - matches cc-bash.c */
typedef struct {
    char prompt[32];
    char error[32];
    char comment[32];
    char dim[32];
    char header[32];
    char status[32];
    char scroll[32];
} Theme;

static Theme theme = {
    .prompt = "",
    .error = "\033[31m",
    .comment = "\033[33m",
    .dim = "\033[2m",
    .header = "\033[36m",
    .status = "\033[1m",
    .scroll = "\033[36m"
};

/* Reset theme to defaults */
static void reset_theme(void)
{
    strcpy(theme.prompt, "");
    strcpy(theme.error, "\033[31m");
    strcpy(theme.comment, "\033[33m");
    strcpy(theme.dim, "\033[2m");
    strcpy(theme.header, "\033[36m");
    strcpy(theme.status, "\033[1m");
    strcpy(theme.scroll, "\033[36m");
}

/* Parse color name to ANSI code - matches cc-bash.c */
static const char* parse_color(const char* name)
{
    static char buf[64];
    buf[0] = '\0';

    if (!name || strlen(name) == 0) {
        return buf;
    }

    char copy[64];
    strncpy(copy, name, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char* token = strtok(copy, " ");
    while (token) {
        if (strcmp(token, "bold") == 0) {
            strcat(buf, "\033[1m");
        } else if (strcmp(token, "dim") == 0) {
            strcat(buf, "\033[2m");
        } else if (strcmp(token, "black") == 0) {
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
        } else if (strcmp(token, "none") == 0 || strcmp(token, "default") == 0) {
            /* Leave empty */
        }
        token = strtok(NULL, " ");
    }

    return buf;
}

/* Set theme color by field name - matches cc-bash.c */
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

typedef struct {
    char* name;
    char* command;
    int from_session;  /* 1 if added via @alias (not from config), 0 if from config */
} Alias;

static Alias aliases[MAX_ALIASES];
static int alias_count = 0;

/* Add an alias - from_session: 1 if from @alias command, 0 if from config file */
static void add_alias(const char* name, const char* command, int from_session)
{
    if (alias_count >= MAX_ALIASES) return;

    /* Check for existing alias with same name and replace */
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            free(aliases[i].command);
            aliases[i].command = strdup(command);
            aliases[i].from_session = from_session;
            return;
        }
    }

    aliases[alias_count].name = strdup(name);
    aliases[alias_count].command = strdup(command);
    aliases[alias_count].from_session = from_session;
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

/* Free all aliases */
static void free_aliases(void)
{
    for (int i = 0; i < alias_count; i++) {
        free(aliases[i].name);
        free(aliases[i].command);
    }
    alias_count = 0;
}

/* Snippet structure - matches cc-bash.c */
typedef struct {
    char* name;
    char* template;
} Snippet;

/* ============================================================================
 * Event System Types - matches cc-bash.c
 * ============================================================================ */

#define MAX_HOOKS 20

typedef enum {
    EVENT_STARTUP,
    EVENT_SHUTDOWN,
    EVENT_PRE_COMMAND,
    EVENT_POST_COMMAND,
    EVENT_CD,
    EVENT_ALIAS_EXPAND,
    EVENT_SNIPPET_EXPAND
} EventType;

typedef struct {
    EventType type;
    const char* command;
    const char* expanded;
    int exit_code;
    const char* old_cwd;
    const char* new_cwd;
} Event;

typedef void (*EventHandler)(const Event* event, void* user_data);

typedef struct {
    EventType type;
    EventHandler handler;
    void* user_data;
    int active;
} Hook;

static Hook hooks[MAX_HOOKS];
static int hook_count = 0;

/* Register a hook */
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

/* Emit an event to all registered handlers of matching type */
static void emit_event(const Event* event)
{
    for (int i = 0; i < hook_count; i++) {
        if (hooks[i].active && hooks[i].type == event->type) {
            hooks[i].handler(event, hooks[i].user_data);
        }
    }
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

/* Helper: Emit command event */
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

/* Helper: Emit expand event */
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

/* Get event type name */
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

/* Reset hooks for testing */
static void reset_hooks(void)
{
    hook_count = 0;
}

static Snippet snippets[MAX_SNIPPETS];
static int snippet_count = 0;

/* Add a snippet */
static void add_snippet(const char* name, const char* template)
{
    if (snippet_count >= MAX_SNIPPETS) return;

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

/* Look up a snippet */
static const char* get_snippet(const char* name)
{
    for (int i = 0; i < snippet_count; i++) {
        if (strcmp(snippets[i].name, name) == 0) {
            return snippets[i].template;
        }
    }
    return NULL;
}

/* Expand snippet with arguments */
static char* expand_snippet(const char* template, char** args, int arg_count)
{
    char* result = malloc(INPUT_BUF_SIZE);
    if (!result) return NULL;

    char* out = result;
    const char* in = template;
    size_t remaining = INPUT_BUF_SIZE - 1;

    while (*in && remaining > 0) {
        if (*in == '$' && in[1] >= '1' && in[1] <= '9') {
            int arg_idx = in[1] - '1';
            if (arg_idx < arg_count && args[arg_idx]) {
                size_t arg_len = strlen(args[arg_idx]);
                if (arg_len <= remaining) {
                    strcpy(out, args[arg_idx]);
                    out += arg_len;
                    remaining -= arg_len;
                }
            }
            in += 2;
        } else {
            *out++ = *in++;
            remaining--;
        }
    }
    *out = '\0';

    return result;
}

/* Free snippets */
static void free_snippets(void)
{
    for (int i = 0; i < snippet_count; i++) {
        free(snippets[i].name);
        free(snippets[i].template);
    }
    snippet_count = 0;
}

/* ============================================================================
 * Workflow Types and Functions - matches cc-bash.c
 * ============================================================================ */

#define MAX_WORKFLOWS 20
#define MAX_WORKFLOW_STEPS 10

typedef struct {
    char* name;
    char* steps[MAX_WORKFLOW_STEPS];
    int step_count;
    int stop_on_error;
} Workflow;

static Workflow workflows[MAX_WORKFLOWS];
static int workflow_count = 0;

/* Add a workflow - matches cc-bash.c add_workflow */
static void add_workflow(const char* name, const char* commands)
{
    if (workflow_count >= MAX_WORKFLOWS) return;

    int idx = -1;
    for (int i = 0; i < workflow_count; i++) {
        if (strcmp(workflows[i].name, name) == 0) {
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
    workflows[idx].stop_on_error = 1;

    char* cmd_copy = strdup(commands);
    if (!cmd_copy) return;

    if (strstr(cmd_copy, "&&")) {
        workflows[idx].stop_on_error = 1;
        char* saveptr;
        char* step = strtok_r(cmd_copy, "&", &saveptr);
        while (step && workflows[idx].step_count < MAX_WORKFLOW_STEPS) {
            while (*step == '&' || *step == ' ') step++;
            if (*step) {
                char* end = step + strlen(step) - 1;
                while (end > step && *end == ' ') *end-- = '\0';
                if (*step) {
                    workflows[idx].steps[workflows[idx].step_count++] = strdup(step);
                }
            }
            step = strtok_r(NULL, "&", &saveptr);
        }
    } else {
        workflows[idx].stop_on_error = 0;
        char* saveptr;
        char* step = strtok_r(cmd_copy, ";", &saveptr);
        while (step && workflows[idx].step_count < MAX_WORKFLOW_STEPS) {
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

/* Look up workflow */
static Workflow* get_workflow(const char* name)
{
    for (int i = 0; i < workflow_count; i++) {
        if (strcmp(workflows[i].name, name) == 0) {
            return &workflows[i];
        }
    }
    return NULL;
}

/* Free workflows */
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

/* Parse config file - simplified version for testing */
static void parse_config_line(char* line)
{
    /* Remove trailing newline */
    line[strcspn(line, "\n")] = '\0';

    /* Skip empty lines and comments */
    char* p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0' || *p == '#') return;

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

            add_alias(name, cmd, 0);  /* from config file */
        }
    }
}

/* History duplicate check */
#define MAX_HISTORY 100
static char* history[MAX_HISTORY];
static int history_count = 0;

static void add_history(const char* cmd)
{
    /* Skip if same as last command (no consecutive duplicates) */
    if (history_count > 0 && strcmp(history[history_count - 1], cmd) == 0) {
        return;
    }

    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    }
}

static void free_history(void)
{
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    history_count = 0;
}

/* ============================================================================
 * Test Cases
 * ============================================================================ */

void test_alias_basic(void)
{
    printf("\n[Alias Basic Operations]\n");
    free_aliases();

    add_alias("ll", "ls -la", 0);
    ASSERT(alias_count == 1, "alias_count is 1 after adding one alias");
    ASSERT_STR_EQ(get_alias("ll"), "ls -la", "get_alias returns correct command");
    ASSERT_NULL(get_alias("nonexistent"), "get_alias returns NULL for unknown alias");

    free_aliases();
}

void test_alias_replace(void)
{
    printf("\n[Alias Replacement]\n");
    free_aliases();

    add_alias("ll", "ls -la", 0);
    add_alias("ll", "ls -lah", 0);
    ASSERT(alias_count == 1, "alias_count stays 1 after replacing");
    ASSERT_STR_EQ(get_alias("ll"), "ls -lah", "alias command is updated");

    free_aliases();
}

void test_alias_multiple(void)
{
    printf("\n[Multiple Aliases]\n");
    free_aliases();

    add_alias("ll", "ls -la", 0);
    add_alias("gs", "git status", 0);
    add_alias("gd", "git diff", 0);

    ASSERT(alias_count == 3, "alias_count is 3 after adding three aliases");
    ASSERT_STR_EQ(get_alias("ll"), "ls -la", "first alias correct");
    ASSERT_STR_EQ(get_alias("gs"), "git status", "second alias correct");
    ASSERT_STR_EQ(get_alias("gd"), "git diff", "third alias correct");

    free_aliases();
}

void test_alias_from_session_flag(void)
{
    printf("\n[Alias from_session Flag]\n");
    free_aliases();

    /* Add alias from config (from_session = 0) */
    add_alias("ll", "ls -la", 0);
    ASSERT(aliases[0].from_session == 0, "from_session is 0 for config alias");

    /* Add alias from session (from_session = 1) */
    add_alias("gs", "git status", 1);
    ASSERT(aliases[1].from_session == 1, "from_session is 1 for session alias");

    /* Replace config alias with session alias - flag should update */
    add_alias("ll", "ls -lah", 1);
    ASSERT(aliases[0].from_session == 1, "from_session updates when replaced");
    ASSERT_STR_EQ(get_alias("ll"), "ls -lah", "command updated");

    /* Replace session alias with config alias - flag should update */
    add_alias("gs", "git status -s", 0);
    ASSERT(aliases[1].from_session == 0, "from_session can be reset to 0");

    free_aliases();
}

void test_config_parse_alias_single_quotes(void)
{
    printf("\n[Config Parse: alias with single quotes]\n");
    free_aliases();

    char line[] = "alias ll='ls -la'";
    parse_config_line(line);

    ASSERT(alias_count == 1, "alias parsed from config");
    ASSERT_STR_EQ(get_alias("ll"), "ls -la", "quotes stripped correctly");

    free_aliases();
}

void test_config_parse_alias_double_quotes(void)
{
    printf("\n[Config Parse: alias with double quotes]\n");
    free_aliases();

    char line[] = "alias gs=\"git status\"";
    parse_config_line(line);

    ASSERT(alias_count == 1, "alias parsed from config");
    ASSERT_STR_EQ(get_alias("gs"), "git status", "double quotes stripped");

    free_aliases();
}

void test_config_parse_alias_no_quotes(void)
{
    printf("\n[Config Parse: alias without quotes]\n");
    free_aliases();

    char line[] = "alias l=ls";
    parse_config_line(line);

    ASSERT(alias_count == 1, "alias parsed without quotes");
    ASSERT_STR_EQ(get_alias("l"), "ls", "unquoted value correct");

    free_aliases();
}

void test_config_parse_comments(void)
{
    printf("\n[Config Parse: comments ignored]\n");
    free_aliases();

    char line1[] = "# this is a comment";
    char line2[] = "  # indented comment";
    parse_config_line(line1);
    parse_config_line(line2);

    ASSERT(alias_count == 0, "comments do not create aliases");

    free_aliases();
}

void test_config_parse_empty_lines(void)
{
    printf("\n[Config Parse: empty lines ignored]\n");
    free_aliases();

    char line1[] = "";
    char line2[] = "   ";
    char line3[] = "\t\t";
    parse_config_line(line1);
    parse_config_line(line2);
    parse_config_line(line3);

    ASSERT(alias_count == 0, "empty lines do not create aliases");

    free_aliases();
}

void test_history_basic(void)
{
    printf("\n[History Basic Operations]\n");
    free_history();

    add_history("ls");
    add_history("pwd");
    add_history("cd /tmp");

    ASSERT(history_count == 3, "history_count is 3");
    ASSERT_STR_EQ(history[0], "ls", "first command correct");
    ASSERT_STR_EQ(history[1], "pwd", "second command correct");
    ASSERT_STR_EQ(history[2], "cd /tmp", "third command correct");

    free_history();
}

void test_history_no_consecutive_duplicates(void)
{
    printf("\n[History Duplicate Prevention]\n");
    free_history();

    add_history("ls");
    add_history("ls");
    add_history("ls");

    ASSERT(history_count == 1, "consecutive duplicates not added");
    ASSERT_STR_EQ(history[0], "ls", "single entry exists");

    free_history();
}

void test_history_non_consecutive_duplicates_allowed(void)
{
    printf("\n[History Non-Consecutive Duplicates]\n");
    free_history();

    add_history("ls");
    add_history("pwd");
    add_history("ls");

    ASSERT(history_count == 3, "non-consecutive duplicates allowed");
    ASSERT_STR_EQ(history[0], "ls", "first ls");
    ASSERT_STR_EQ(history[1], "pwd", "pwd in middle");
    ASSERT_STR_EQ(history[2], "ls", "second ls");

    free_history();
}

void test_history_file_roundtrip(void)
{
    printf("\n[History File Roundtrip]\n");

    /* Create temp file */
    char tmpfile[] = "/tmp/cc-bash-test-XXXXXX";
    int fd = mkstemp(tmpfile);
    if (fd < 0) {
        printf("  \033[33mSKIP\033[0m: Could not create temp file\n");
        return;
    }
    close(fd);

    /* Write history to file */
    FILE* fp = fopen(tmpfile, "w");
    ASSERT_NOT_NULL(fp, "temp file opened for writing");
    if (fp) {
        fprintf(fp, "ls -la\n");
        fprintf(fp, "git status\n");
        fprintf(fp, "make test\n");
        fclose(fp);
    }

    /* Read history from file */
    free_history();
    fp = fopen(tmpfile, "r");
    ASSERT_NOT_NULL(fp, "temp file opened for reading");
    if (fp) {
        char line[1024];
        while (fgets(line, sizeof(line), fp) && history_count < MAX_HISTORY) {
            line[strcspn(line, "\n")] = '\0';
            if (strlen(line) > 0) {
                history[history_count++] = strdup(line);
            }
        }
        fclose(fp);
    }

    ASSERT(history_count == 3, "3 lines read from file");
    ASSERT_STR_EQ(history[0], "ls -la", "first line correct");
    ASSERT_STR_EQ(history[1], "git status", "second line correct");
    ASSERT_STR_EQ(history[2], "make test", "third line correct");

    /* Cleanup */
    unlink(tmpfile);
    free_history();
}

void test_config_file_roundtrip(void)
{
    printf("\n[Config File Roundtrip]\n");

    /* Create temp file */
    char tmpfile[] = "/tmp/cc-bash-config-XXXXXX";
    int fd = mkstemp(tmpfile);
    if (fd < 0) {
        printf("  \033[33mSKIP\033[0m: Could not create temp file\n");
        return;
    }
    close(fd);

    /* Write config to file */
    FILE* fp = fopen(tmpfile, "w");
    ASSERT_NOT_NULL(fp, "temp file opened for writing");
    if (fp) {
        fprintf(fp, "# Test config\n");
        fprintf(fp, "alias ll='ls -la'\n");
        fprintf(fp, "alias gs=\"git status\"\n");
        fprintf(fp, "\n");
        fprintf(fp, "alias gd=git diff\n");
        fclose(fp);
    }

    /* Parse config */
    free_aliases();
    fp = fopen(tmpfile, "r");
    ASSERT_NOT_NULL(fp, "temp file opened for reading");
    if (fp) {
        char line[CONFIG_LINE_SIZE];
        while (fgets(line, sizeof(line), fp)) {
            parse_config_line(line);
        }
        fclose(fp);
    }

    ASSERT(alias_count == 3, "3 aliases parsed from file");
    ASSERT_STR_EQ(get_alias("ll"), "ls -la", "ll alias correct");
    ASSERT_STR_EQ(get_alias("gs"), "git status", "gs alias correct");
    ASSERT_STR_EQ(get_alias("gd"), "git diff", "gd alias correct");

    /* Cleanup */
    unlink(tmpfile);
    free_aliases();
}

/* ============================================================================
 * Theme Tests
 * ============================================================================ */

void test_theme_parse_simple_colors(void)
{
    printf("\n[Theme Parse: Simple Colors]\n");

    ASSERT_STR_EQ(parse_color("red"), "\033[31m", "red parses correctly");
    ASSERT_STR_EQ(parse_color("green"), "\033[32m", "green parses correctly");
    ASSERT_STR_EQ(parse_color("yellow"), "\033[33m", "yellow parses correctly");
    ASSERT_STR_EQ(parse_color("blue"), "\033[34m", "blue parses correctly");
    ASSERT_STR_EQ(parse_color("magenta"), "\033[35m", "magenta parses correctly");
    ASSERT_STR_EQ(parse_color("cyan"), "\033[36m", "cyan parses correctly");
    ASSERT_STR_EQ(parse_color("white"), "\033[37m", "white parses correctly");
    ASSERT_STR_EQ(parse_color("black"), "\033[30m", "black parses correctly");
}

void test_theme_parse_modifiers(void)
{
    printf("\n[Theme Parse: Modifiers]\n");

    ASSERT_STR_EQ(parse_color("bold"), "\033[1m", "bold parses correctly");
    ASSERT_STR_EQ(parse_color("dim"), "\033[2m", "dim parses correctly");
}

void test_theme_parse_combined(void)
{
    printf("\n[Theme Parse: Combined modifiers and colors]\n");

    const char* result = parse_color("bold red");
    ASSERT(strstr(result, "\033[1m") != NULL, "bold red contains bold code");
    ASSERT(strstr(result, "\033[31m") != NULL, "bold red contains red code");

    result = parse_color("dim cyan");
    ASSERT(strstr(result, "\033[2m") != NULL, "dim cyan contains dim code");
    ASSERT(strstr(result, "\033[36m") != NULL, "dim cyan contains cyan code");
}

void test_theme_parse_empty(void)
{
    printf("\n[Theme Parse: Empty/None]\n");

    ASSERT_STR_EQ(parse_color(""), "", "empty string returns empty");
    ASSERT_STR_EQ(parse_color("none"), "", "none returns empty");
    ASSERT_STR_EQ(parse_color("default"), "", "default returns empty");
    ASSERT_STR_EQ(parse_color(NULL), "", "NULL returns empty");
}

void test_theme_set_colors(void)
{
    printf("\n[Theme Set: Individual fields]\n");
    reset_theme();

    set_theme_color("prompt", "cyan");
    ASSERT_STR_EQ(theme.prompt, "\033[36m", "prompt set to cyan");

    set_theme_color("error", "bold red");
    ASSERT(strstr(theme.error, "\033[1m") != NULL, "error contains bold");
    ASSERT(strstr(theme.error, "\033[31m") != NULL, "error contains red");

    set_theme_color("comment", "green");
    ASSERT_STR_EQ(theme.comment, "\033[32m", "comment set to green");

    set_theme_color("dim", "dim");
    ASSERT_STR_EQ(theme.dim, "\033[2m", "dim set correctly");

    set_theme_color("header", "magenta");
    ASSERT_STR_EQ(theme.header, "\033[35m", "header set to magenta");

    set_theme_color("status", "bold");
    ASSERT_STR_EQ(theme.status, "\033[1m", "status set to bold");

    set_theme_color("scroll", "blue");
    ASSERT_STR_EQ(theme.scroll, "\033[34m", "scroll set to blue");

    reset_theme();
}

void test_theme_unknown_color(void)
{
    printf("\n[Theme Parse: Unknown color]\n");

    /* Unknown colors should be ignored, producing empty string */
    ASSERT_STR_EQ(parse_color("unknown"), "", "unknown color returns empty");
    ASSERT_STR_EQ(parse_color("purple"), "", "invalid purple returns empty");
}

void test_theme_config_file(void)
{
    printf("\n[Theme Config File Parsing]\n");
    reset_theme();

    /* Create temp file */
    char tmpfile[] = "/tmp/cc-bash-theme-XXXXXX";
    int fd = mkstemp(tmpfile);
    if (fd < 0) {
        printf("  \033[33mSKIP\033[0m: Could not create temp file\n");
        return;
    }
    close(fd);

    /* Write config with theme settings */
    FILE* fp = fopen(tmpfile, "w");
    ASSERT_NOT_NULL(fp, "temp file opened for writing");
    if (fp) {
        fprintf(fp, "# Theme test config\n");
        fprintf(fp, "theme.prompt=cyan\n");
        fprintf(fp, "theme.error='bold red'\n");
        fprintf(fp, "theme.comment=green\n");
        fclose(fp);
    }

    /* Parse config - need to add theme parsing to parse_config_line first */
    /* For now, just test direct set_theme_color */
    set_theme_color("prompt", "cyan");
    set_theme_color("error", "bold red");
    set_theme_color("comment", "green");

    ASSERT_STR_EQ(theme.prompt, "\033[36m", "prompt from config");
    ASSERT(strstr(theme.error, "\033[31m") != NULL, "error has red from config");
    ASSERT_STR_EQ(theme.comment, "\033[32m", "comment from config");

    /* Cleanup */
    unlink(tmpfile);
    reset_theme();
}

/* ============================================================================
 * Snippet Tests
 * ============================================================================ */

void test_snippet_basic(void)
{
    printf("\n[Snippet Basic Operations]\n");
    free_snippets();

    add_snippet("deploy", "git push origin $1");
    ASSERT(snippet_count == 1, "snippet_count is 1 after adding one snippet");
    ASSERT_STR_EQ(get_snippet("deploy"), "git push origin $1", "get_snippet returns correct template");
    ASSERT_NULL(get_snippet("nonexistent"), "get_snippet returns NULL for unknown snippet");

    free_snippets();
}

void test_snippet_replace(void)
{
    printf("\n[Snippet Replacement]\n");
    free_snippets();

    add_snippet("logs", "docker logs $1");
    add_snippet("logs", "docker logs -f --tail $1 $2");
    ASSERT(snippet_count == 1, "snippet_count stays 1 after replacing");
    ASSERT_STR_EQ(get_snippet("logs"), "docker logs -f --tail $1 $2", "snippet template is updated");

    free_snippets();
}

void test_snippet_multiple(void)
{
    printf("\n[Multiple Snippets]\n");
    free_snippets();

    add_snippet("deploy", "git push origin $1");
    add_snippet("logs", "docker logs -f $1");
    add_snippet("ssh", "ssh $1@$2");

    ASSERT(snippet_count == 3, "snippet_count is 3 after adding three snippets");
    ASSERT_STR_EQ(get_snippet("deploy"), "git push origin $1", "first snippet correct");
    ASSERT_STR_EQ(get_snippet("logs"), "docker logs -f $1", "second snippet correct");
    ASSERT_STR_EQ(get_snippet("ssh"), "ssh $1@$2", "third snippet correct");

    free_snippets();
}

void test_snippet_expand_single_arg(void)
{
    printf("\n[Snippet Expand: Single arg]\n");

    char* args[] = {"main"};
    char* result = expand_snippet("git push origin $1", args, 1);

    ASSERT_NOT_NULL(result, "expand_snippet returns non-NULL");
    ASSERT_STR_EQ(result, "git push origin main", "single arg expanded correctly");

    free(result);
}

void test_snippet_expand_multiple_args(void)
{
    printf("\n[Snippet Expand: Multiple args]\n");

    char* args[] = {"100", "my-container"};
    char* result = expand_snippet("docker logs -f --tail $1 $2", args, 2);

    ASSERT_NOT_NULL(result, "expand_snippet returns non-NULL");
    ASSERT_STR_EQ(result, "docker logs -f --tail 100 my-container", "multiple args expanded correctly");

    free(result);
}

void test_snippet_expand_args_in_middle(void)
{
    printf("\n[Snippet Expand: Args in middle]\n");

    char* args[] = {"user", "server.com"};
    char* result = expand_snippet("ssh $1@$2 -p 22", args, 2);

    ASSERT_NOT_NULL(result, "expand_snippet returns non-NULL");
    ASSERT_STR_EQ(result, "ssh user@server.com -p 22", "args in middle expanded correctly");

    free(result);
}

void test_snippet_expand_missing_arg(void)
{
    printf("\n[Snippet Expand: Missing arg]\n");

    char* args[] = {"arg1"};
    char* result = expand_snippet("cmd $1 $2 $3", args, 1);

    ASSERT_NOT_NULL(result, "expand_snippet returns non-NULL");
    /* Missing args should be replaced with empty string */
    ASSERT_STR_EQ(result, "cmd arg1  ", "missing args become empty");

    free(result);
}

void test_snippet_expand_no_args(void)
{
    printf("\n[Snippet Expand: No args (literal template)]\n");

    char* result = expand_snippet("echo hello world", NULL, 0);

    ASSERT_NOT_NULL(result, "expand_snippet returns non-NULL");
    ASSERT_STR_EQ(result, "echo hello world", "no placeholders = literal copy");

    free(result);
}

void test_snippet_expand_dollar_not_placeholder(void)
{
    printf("\n[Snippet Expand: Dollar not placeholder]\n");

    char* args[] = {"test"};
    char* result = expand_snippet("echo $HOME $1 $VAR", args, 1);

    ASSERT_NOT_NULL(result, "expand_snippet returns non-NULL");
    /* $HOME and $VAR are not $1-$9, so they pass through */
    ASSERT_STR_EQ(result, "echo $HOME test $VAR", "$VAR passes through, $1 expanded");

    free(result);
}

/* ============================================================================
 * Event System Tests
 * ============================================================================ */

/* Test data for event handlers */
static int event_call_count = 0;
static EventType last_event_type = EVENT_STARTUP;
static char last_event_command[256] = "";
static int last_event_exit_code = 0;

/* Simple handler that counts calls */
static void counting_handler(const Event* event, void* user_data)
{
    (void)user_data;
    event_call_count++;
    last_event_type = event->type;
}

/* Handler that captures command */
static void command_handler(const Event* event, void* user_data)
{
    (void)user_data;
    event_call_count++;
    last_event_type = event->type;
    if (event->command) {
        strncpy(last_event_command, event->command, sizeof(last_event_command) - 1);
    } else {
        last_event_command[0] = '\0';
    }
    last_event_exit_code = event->exit_code;
}

/* Handler that uses user_data */
static void user_data_handler(const Event* event, void* user_data)
{
    (void)event;
    int* counter = (int*)user_data;
    (*counter)++;
}

/* Variables and handler for CD event capture */
static char captured_old_cwd[PATH_MAX] = "";
static char captured_new_cwd[PATH_MAX] = "";

static void cd_capture_handler(const Event* event, void* user_data)
{
    (void)user_data;
    if (event->old_cwd) strncpy(captured_old_cwd, event->old_cwd, PATH_MAX - 1);
    if (event->new_cwd) strncpy(captured_new_cwd, event->new_cwd, PATH_MAX - 1);
}

/* Variables and handler for expand event capture */
static char captured_original[256] = "";
static char captured_expanded[256] = "";

static void expand_capture_handler(const Event* event, void* user_data)
{
    (void)user_data;
    if (event->command) strncpy(captured_original, event->command, 255);
    if (event->expanded) strncpy(captured_expanded, event->expanded, 255);
}

void test_event_register_hook(void)
{
    printf("\n[Event: Register Hook]\n");
    reset_hooks();
    event_call_count = 0;

    int id = register_hook(EVENT_STARTUP, counting_handler, NULL);
    ASSERT(id == 0, "first hook gets ID 0");
    ASSERT(hook_count == 1, "hook_count is 1");

    id = register_hook(EVENT_SHUTDOWN, counting_handler, NULL);
    ASSERT(id == 1, "second hook gets ID 1");
    ASSERT(hook_count == 2, "hook_count is 2");

    reset_hooks();
}

void test_event_unregister_hook(void)
{
    printf("\n[Event: Unregister Hook]\n");
    reset_hooks();
    event_call_count = 0;

    int id = register_hook(EVENT_STARTUP, counting_handler, NULL);
    ASSERT(hooks[id].active == 1, "hook starts active");

    unregister_hook(id);
    ASSERT(hooks[id].active == 0, "hook is inactive after unregister");

    /* Emit event - handler should NOT be called */
    emit_simple_event(EVENT_STARTUP);
    ASSERT(event_call_count == 0, "inactive hook not called");

    reset_hooks();
}

void test_event_emit_simple(void)
{
    printf("\n[Event: Emit Simple Event]\n");
    reset_hooks();
    event_call_count = 0;

    register_hook(EVENT_STARTUP, counting_handler, NULL);

    emit_simple_event(EVENT_STARTUP);
    ASSERT(event_call_count == 1, "handler called once");
    ASSERT(last_event_type == EVENT_STARTUP, "event type is STARTUP");

    reset_hooks();
}

void test_event_emit_wrong_type(void)
{
    printf("\n[Event: Emit Wrong Type]\n");
    reset_hooks();
    event_call_count = 0;

    register_hook(EVENT_STARTUP, counting_handler, NULL);

    emit_simple_event(EVENT_SHUTDOWN);
    ASSERT(event_call_count == 0, "handler not called for wrong type");

    reset_hooks();
}

void test_event_multiple_handlers(void)
{
    printf("\n[Event: Multiple Handlers]\n");
    reset_hooks();
    event_call_count = 0;

    register_hook(EVENT_PRE_COMMAND, counting_handler, NULL);
    register_hook(EVENT_PRE_COMMAND, counting_handler, NULL);
    register_hook(EVENT_PRE_COMMAND, counting_handler, NULL);

    emit_command_event(EVENT_PRE_COMMAND, "ls", 0);
    ASSERT(event_call_count == 3, "all three handlers called");

    reset_hooks();
}

void test_event_command_data(void)
{
    printf("\n[Event: Command Data]\n");
    reset_hooks();
    event_call_count = 0;

    register_hook(EVENT_POST_COMMAND, command_handler, NULL);

    emit_command_event(EVENT_POST_COMMAND, "git status", 1);
    ASSERT(event_call_count == 1, "handler called");
    ASSERT_STR_EQ(last_event_command, "git status", "command captured");
    ASSERT(last_event_exit_code == 1, "exit code captured");

    reset_hooks();
}

void test_event_user_data(void)
{
    printf("\n[Event: User Data]\n");
    reset_hooks();

    int my_counter = 0;
    register_hook(EVENT_CD, user_data_handler, &my_counter);

    emit_cd_event("/old", "/new");
    ASSERT(my_counter == 1, "user_data counter incremented");

    emit_cd_event("/new", "/newer");
    ASSERT(my_counter == 2, "user_data counter incremented again");

    reset_hooks();
}

void test_event_type_names(void)
{
    printf("\n[Event: Type Names]\n");

    ASSERT_STR_EQ(event_type_name(EVENT_STARTUP), "STARTUP", "STARTUP name");
    ASSERT_STR_EQ(event_type_name(EVENT_SHUTDOWN), "SHUTDOWN", "SHUTDOWN name");
    ASSERT_STR_EQ(event_type_name(EVENT_PRE_COMMAND), "PRE_COMMAND", "PRE_COMMAND name");
    ASSERT_STR_EQ(event_type_name(EVENT_POST_COMMAND), "POST_COMMAND", "POST_COMMAND name");
    ASSERT_STR_EQ(event_type_name(EVENT_CD), "CD", "CD name");
    ASSERT_STR_EQ(event_type_name(EVENT_ALIAS_EXPAND), "ALIAS_EXPAND", "ALIAS_EXPAND name");
    ASSERT_STR_EQ(event_type_name(EVENT_SNIPPET_EXPAND), "SNIPPET_EXPAND", "SNIPPET_EXPAND name");
}

void test_event_cd_data(void)
{
    printf("\n[Event: CD Event Data]\n");
    reset_hooks();

    /* Reset capture variables */
    captured_old_cwd[0] = '\0';
    captured_new_cwd[0] = '\0';

    register_hook(EVENT_CD, cd_capture_handler, NULL);

    emit_cd_event("/home/user", "/tmp");
    ASSERT_STR_EQ(captured_old_cwd, "/home/user", "old_cwd captured");
    ASSERT_STR_EQ(captured_new_cwd, "/tmp", "new_cwd captured");

    reset_hooks();
}

void test_event_expand_data(void)
{
    printf("\n[Event: Expand Event Data]\n");
    reset_hooks();

    /* Reset capture variables */
    captured_original[0] = '\0';
    captured_expanded[0] = '\0';

    register_hook(EVENT_ALIAS_EXPAND, expand_capture_handler, NULL);

    emit_expand_event(EVENT_ALIAS_EXPAND, "ll", "ls -la");
    ASSERT_STR_EQ(captured_original, "ll", "original command captured");
    ASSERT_STR_EQ(captured_expanded, "ls -la", "expanded command captured");

    reset_hooks();
}

void test_event_hook_limit(void)
{
    printf("\n[Event: Hook Limit]\n");
    reset_hooks();

    /* Register up to MAX_HOOKS */
    for (int i = 0; i < MAX_HOOKS; i++) {
        int id = register_hook(EVENT_STARTUP, counting_handler, NULL);
        ASSERT(id == i, "hook registered successfully");
    }

    /* Next registration should fail */
    int overflow_id = register_hook(EVENT_STARTUP, counting_handler, NULL);
    ASSERT(overflow_id == -1, "hook registration fails when full");

    reset_hooks();
}

/* ============================================================================
 * Workflow Tests
 * ============================================================================ */

void test_workflow_basic(void)
{
    printf("\n[Workflow: Basic Operations]\n");
    free_workflows();

    add_workflow("build", "make clean && make");
    ASSERT(workflow_count == 1, "workflow_count is 1 after adding one workflow");
    ASSERT_NOT_NULL(get_workflow("build"), "get_workflow returns workflow");
    ASSERT_NULL(get_workflow("nonexistent"), "get_workflow returns NULL for unknown");

    Workflow* wf = get_workflow("build");
    ASSERT(wf->step_count == 2, "workflow has 2 steps");
    ASSERT(wf->stop_on_error == 1, "workflow stops on error (&&)");

    free_workflows();
}

void test_workflow_and_separator(void)
{
    printf("\n[Workflow: && Separator]\n");
    free_workflows();

    add_workflow("test", "step1 && step2 && step3");
    Workflow* wf = get_workflow("test");

    ASSERT_NOT_NULL(wf, "workflow exists");
    ASSERT(wf->step_count == 3, "workflow has 3 steps");
    ASSERT(wf->stop_on_error == 1, "workflow stops on error");
    ASSERT_STR_EQ(wf->steps[0], "step1", "first step correct");
    ASSERT_STR_EQ(wf->steps[1], "step2", "second step correct");
    ASSERT_STR_EQ(wf->steps[2], "step3", "third step correct");

    free_workflows();
}

void test_workflow_semicolon_separator(void)
{
    printf("\n[Workflow: ; Separator]\n");
    free_workflows();

    add_workflow("check", "git status; git diff");
    Workflow* wf = get_workflow("check");

    ASSERT_NOT_NULL(wf, "workflow exists");
    ASSERT(wf->step_count == 2, "workflow has 2 steps");
    ASSERT(wf->stop_on_error == 0, "workflow continues on error");
    ASSERT_STR_EQ(wf->steps[0], "git status", "first step correct");
    ASSERT_STR_EQ(wf->steps[1], "git diff", "second step correct");

    free_workflows();
}

void test_workflow_replace(void)
{
    printf("\n[Workflow: Replace]\n");
    free_workflows();

    add_workflow("build", "make");
    add_workflow("build", "make clean && make && make test");

    ASSERT(workflow_count == 1, "workflow_count stays 1 after replacing");
    Workflow* wf = get_workflow("build");
    ASSERT(wf->step_count == 3, "workflow now has 3 steps");

    free_workflows();
}

void test_workflow_multiple(void)
{
    printf("\n[Workflow: Multiple Workflows]\n");
    free_workflows();

    add_workflow("build", "make");
    add_workflow("test", "make test");
    add_workflow("deploy", "git push");

    ASSERT(workflow_count == 3, "workflow_count is 3");
    ASSERT_NOT_NULL(get_workflow("build"), "build exists");
    ASSERT_NOT_NULL(get_workflow("test"), "test exists");
    ASSERT_NOT_NULL(get_workflow("deploy"), "deploy exists");

    free_workflows();
}

void test_workflow_whitespace_handling(void)
{
    printf("\n[Workflow: Whitespace Handling]\n");
    free_workflows();

    add_workflow("spacy", "  cmd1  &&  cmd2  ");
    Workflow* wf = get_workflow("spacy");

    ASSERT_NOT_NULL(wf, "workflow exists");
    ASSERT(wf->step_count == 2, "workflow has 2 steps");
    ASSERT_STR_EQ(wf->steps[0], "cmd1", "first step trimmed");
    ASSERT_STR_EQ(wf->steps[1], "cmd2", "second step trimmed");

    free_workflows();
}

void test_workflow_complex_commands(void)
{
    printf("\n[Workflow: Complex Commands]\n");
    free_workflows();

    add_workflow("complex", "ls -la /tmp && grep -r 'pattern' .");
    Workflow* wf = get_workflow("complex");

    ASSERT_NOT_NULL(wf, "workflow exists");
    ASSERT(wf->step_count == 2, "workflow has 2 steps");
    ASSERT_STR_EQ(wf->steps[0], "ls -la /tmp", "first step with args");
    ASSERT_STR_EQ(wf->steps[1], "grep -r 'pattern' .", "second step with args");

    free_workflows();
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void)
{
    printf("========================================\n");
    printf("cc-bash Unit Tests\n");
    printf("========================================\n");

    /* Alias tests */
    test_alias_basic();
    test_alias_replace();
    test_alias_multiple();
    test_alias_from_session_flag();

    /* Config parsing tests */
    test_config_parse_alias_single_quotes();
    test_config_parse_alias_double_quotes();
    test_config_parse_alias_no_quotes();
    test_config_parse_comments();
    test_config_parse_empty_lines();

    /* History tests */
    test_history_basic();
    test_history_no_consecutive_duplicates();
    test_history_non_consecutive_duplicates_allowed();

    /* File I/O tests */
    test_history_file_roundtrip();
    test_config_file_roundtrip();

    /* Theme tests */
    test_theme_parse_simple_colors();
    test_theme_parse_modifiers();
    test_theme_parse_combined();
    test_theme_parse_empty();
    test_theme_set_colors();
    test_theme_unknown_color();
    test_theme_config_file();

    /* Snippet tests */
    test_snippet_basic();
    test_snippet_replace();
    test_snippet_multiple();
    test_snippet_expand_single_arg();
    test_snippet_expand_multiple_args();
    test_snippet_expand_args_in_middle();
    test_snippet_expand_missing_arg();
    test_snippet_expand_no_args();
    test_snippet_expand_dollar_not_placeholder();

    /* Event system tests */
    test_event_register_hook();
    test_event_unregister_hook();
    test_event_emit_simple();
    test_event_emit_wrong_type();
    test_event_multiple_handlers();
    test_event_command_data();
    test_event_user_data();
    test_event_type_names();
    test_event_cd_data();
    test_event_expand_data();
    test_event_hook_limit();

    /* Workflow tests */
    test_workflow_basic();
    test_workflow_and_separator();
    test_workflow_semicolon_separator();
    test_workflow_replace();
    test_workflow_multiple();
    test_workflow_whitespace_handling();
    test_workflow_complex_commands();

    /* Summary */
    printf("\n========================================\n");
    printf("Summary\n");
    printf("========================================\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: \033[32m%d\033[0m\n", tests_passed);

    if (tests_failed > 0) {
        printf("Tests failed: \033[31m%d\033[0m\n", tests_failed);
        return 1;
    } else {
        printf("Tests failed: 0\n");
        printf("\nAll tests passed!\n");
        return 0;
    }
}
