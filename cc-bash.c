/*
 * cc-bash: Claude Code-style bash shell wrapper
 *
 * Three regions:
 * 1. Output area (top) - command output scrolls here
 * 2. Prompt area (middle) - input + command history scrolls here
 * 3. Status area (bottom) - fixed, shows user@host:path + hints
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

/* ANSI codes */
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

#define INPUT_BUF_SIZE 4096
#define MAX_HISTORY 100

/* Terminal state */
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

/* Get terminal size */
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

/* Clear from cursor to end of line */
static void clear_to_eol(void) { printf("\033[K"); }

/* Clear entire line */
static void clear_line(void) { printf("\033[2K"); }

/* Draw separator line */
static void draw_separator(int row)
{
    cursor_move(row, 1);
    printf("%s", DIM);
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
    printf("  %s%s@%s%s:%s", BOLD, user, hostname, RESET, cwd);

    /* Line 2: hint */
    cursor_move(term_rows, 1);
    clear_line();
    printf("  %s⏵⏵ run bash commands (exit to quit)%s", DIM, RESET);

    fflush(stdout);
}

/* Initialize screen layout */
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

/* Restore terminal */
static void cleanup_screen(void)
{
    /* Reset scroll region */
    printf("\033[r");
    cursor_move(term_rows, 1);
    printf("\n%scc-bash exited.%s\n", DIM, RESET);
    fflush(stdout);
}

/* Print output in the output region */
static void print_output(const char* text, int is_stderr)
{
    cursor_save();

    /* Move to output area (scroll region is already set) */
    cursor_move(prompt_row - 2, 1);

    if (is_stderr) {
        printf("%s%s%s\n", RED, text, RESET);
    } else {
        printf("%s\n", text);
    }

    cursor_restore();
    fflush(stdout);
}

/* Execute command */
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

/* Handle cd */
static int handle_cd(const char* path)
{
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
        char err[512];
        snprintf(err, sizeof(err), "cd: %s: %s", path, strerror(errno));
        print_output(err, 1);
        return 1;
    }

    getcwd(cwd, sizeof(cwd));
    draw_status();
    return 0;
}

/* Add to history */
static void add_history(const char* cmd)
{
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    }
    history_pos = history_count;
}

/* Read a line with basic editing */
static char* read_input(void)
{
    static char buf[INPUT_BUF_SIZE];
    int pos = 0;
    int len = 0;

    buf[0] = '\0';

    /* Position cursor at prompt */
    cursor_move(prompt_row, 1);
    clear_line();
    printf("› ");
    fflush(stdout);

    while (1) {
        int c = getchar();

        if (c == EOF || c == 4) {  /* Ctrl+D */
            return NULL;
        }

        if (c == '\n' || c == '\r') {
            buf[len] = '\0';

            /* Scroll the prompt into output area */
            cursor_save();
            cursor_move(prompt_row - 2, 1);
            printf("› %s\n", buf);
            cursor_restore();

            return buf;
        }

        if (c == 127 || c == 8) {  /* Backspace */
            if (pos > 0) {
                memmove(&buf[pos-1], &buf[pos], len - pos + 1);
                pos--;
                len--;
                cursor_move(prompt_row, 3);
                printf("%s ", buf);
                cursor_move(prompt_row, 3 + pos);
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
                        cursor_move(prompt_row, 1);
                        clear_line();
                        printf("› %s", buf);
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
                    cursor_move(prompt_row, 1);
                    clear_line();
                    printf("› %s", buf);
                    fflush(stdout);
                } else if (c3 == 'C') {  /* Right arrow */
                    if (pos < len) {
                        pos++;
                        cursor_move(prompt_row, 3 + pos);
                        fflush(stdout);
                    }
                } else if (c3 == 'D') {  /* Left arrow */
                    if (pos > 0) {
                        pos--;
                        cursor_move(prompt_row, 3 + pos);
                        fflush(stdout);
                    }
                }
            }
            continue;
        }

        if (c == 3) {  /* Ctrl+C */
            buf[0] = '\0';
            cursor_move(prompt_row, 1);
            clear_line();
            printf("› ");
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
            cursor_move(prompt_row, 3);
            printf("%s", buf);
            cursor_move(prompt_row, 3 + pos);
            fflush(stdout);
        }
    }
}

/* Set terminal to raw mode */
static void enable_raw_mode(void)
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/* Signal handler */
static volatile sig_atomic_t resize_flag = 0;

static void handle_sigwinch(int sig)
{
    (void)sig;
    resize_flag = 1;
}

int main(void)
{
    getcwd(cwd, sizeof(cwd));

    /* Set up signal handlers */
    signal(SIGWINCH, handle_sigwinch);

    enable_raw_mode();
    init_screen();

    int running = 1;
    int last_exit = 0;

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
        else if (input[0] == '#') {
            /* Comment - just show it */
            char msg[INPUT_BUF_SIZE + 16];
            snprintf(msg, sizeof(msg), "%s%s%s", YELLOW, input, RESET);
            print_output(msg, 0);
        }
        else {
            last_exit = execute_command(input);
        }
    }

    disable_raw_mode();
    cleanup_screen();

    /* Free history */
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return last_exit;
}
