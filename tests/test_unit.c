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

#define ASSERT_STR_EQ(a, b, msg) ASSERT((a) && (b) && strcmp((a), (b)) == 0, msg)
#define ASSERT_NULL(a, msg) ASSERT((a) == NULL, msg)
#define ASSERT_NOT_NULL(a, msg) ASSERT((a) != NULL, msg)

/* ============================================================================
 * Replicate pure functions from cc-bash.c for testing
 * ============================================================================ */

#define MAX_ALIASES 100
#define CONFIG_LINE_SIZE 1024

typedef struct {
    char* name;
    char* command;
} Alias;

static Alias aliases[MAX_ALIASES];
static int alias_count = 0;

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

/* Free all aliases */
static void free_aliases(void)
{
    for (int i = 0; i < alias_count; i++) {
        free(aliases[i].name);
        free(aliases[i].command);
    }
    alias_count = 0;
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

            add_alias(name, cmd);
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

    add_alias("ll", "ls -la");
    ASSERT(alias_count == 1, "alias_count is 1 after adding one alias");
    ASSERT_STR_EQ(get_alias("ll"), "ls -la", "get_alias returns correct command");
    ASSERT_NULL(get_alias("nonexistent"), "get_alias returns NULL for unknown alias");

    free_aliases();
}

void test_alias_replace(void)
{
    printf("\n[Alias Replacement]\n");
    free_aliases();

    add_alias("ll", "ls -la");
    add_alias("ll", "ls -lah");
    ASSERT(alias_count == 1, "alias_count stays 1 after replacing");
    ASSERT_STR_EQ(get_alias("ll"), "ls -lah", "alias command is updated");

    free_aliases();
}

void test_alias_multiple(void)
{
    printf("\n[Multiple Aliases]\n");
    free_aliases();

    add_alias("ll", "ls -la");
    add_alias("gs", "git status");
    add_alias("gd", "git diff");

    ASSERT(alias_count == 3, "alias_count is 3 after adding three aliases");
    ASSERT_STR_EQ(get_alias("ll"), "ls -la", "first alias correct");
    ASSERT_STR_EQ(get_alias("gs"), "git status", "second alias correct");
    ASSERT_STR_EQ(get_alias("gd"), "git diff", "third alias correct");

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
