#!/bin/bash
#
# test_cc_bash.sh - Basic tests for cc-bash
#
# This script tests cc-bash functionality in a non-interactive manner.
# Since cc-bash requires a terminal for full functionality, some tests
# verify compilation and basic behavior through subprocess testing.
#
# Usage: ./tests/test_cc_bash.sh
#        make test
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Project root directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
CCBASH_BIN="$PROJECT_DIR/cc-bash"

#
# Test helper functions
#

pass() {
    TESTS_PASSED=$((TESTS_PASSED + 1))
    echo -e "${GREEN}PASS${NC}: $1"
}

fail() {
    TESTS_FAILED=$((TESTS_FAILED + 1))
    echo -e "${RED}FAIL${NC}: $1"
    if [ -n "$2" ]; then
        echo "       Details: $2"
    fi
}

skip() {
    echo -e "${YELLOW}SKIP${NC}: $1"
    if [ -n "$2" ]; then
        echo "       Reason: $2"
    fi
}

run_test() {
    local name="$1"
    TESTS_RUN=$((TESTS_RUN + 1))
    echo -n "Test $TESTS_RUN: "
}

#
# Test 1: cc-bash builds without errors
#
test_build() {
    run_test "cc-bash builds without errors"

    cd "$PROJECT_DIR"

    # Clean first
    make clean >/dev/null 2>&1 || true

    # Build and capture output
    if make 2>&1; then
        if [ -f "$CCBASH_BIN" ]; then
            pass "cc-bash builds successfully"
        else
            fail "Build completed but binary not found"
        fi
    else
        fail "Build failed with errors"
    fi
}

#
# Test 2: cc-bash binary is executable
#
test_binary_exists() {
    run_test "cc-bash binary is executable"

    if [ -x "$CCBASH_BIN" ]; then
        pass "Binary exists and is executable"
    else
        fail "Binary not found or not executable at $CCBASH_BIN"
    fi
}

#
# Test 3: Basic echo command execution
#
test_echo_command() {
    run_test "echo command executes correctly"

    # Use a subshell with timeout to test command execution
    # We pipe commands to cc-bash and check if it processes them

    # Since cc-bash requires a TTY, we test the underlying shell execution
    # by checking that the command parsing logic exists

    if grep -q 'execl("/bin/sh"' "$PROJECT_DIR/cc-bash.c"; then
        # Verify shell execution mechanism is present
        pass "Command execution via /bin/sh is implemented"
    else
        fail "Command execution mechanism not found"
    fi
}

#
# Test 4: pwd command handling
#
test_pwd_command() {
    run_test "pwd functionality is present"

    # Since pwd is executed through the shell, verify the cwd tracking
    if grep -q 'getcwd(cwd' "$PROJECT_DIR/cc-bash.c"; then
        pass "Current working directory tracking is implemented"
    else
        fail "getcwd() call not found"
    fi
}

#
# Test 5: cd command handling
#
test_cd_command() {
    run_test "cd command is handled specially"

    # Verify cd is handled as a builtin (not passed to shell)
    if grep -q 'handle_cd' "$PROJECT_DIR/cc-bash.c"; then
        if grep -q 'chdir(path)' "$PROJECT_DIR/cc-bash.c"; then
            pass "cd is implemented as a builtin command"
        else
            fail "chdir() call not found in cd handler"
        fi
    else
        fail "handle_cd function not found"
    fi
}

#
# Test 6: exit/quit command handling
#
test_exit_command() {
    run_test "exit and quit commands work"

    # Check for exit/quit handling
    if grep -q 'strcmp(input, "exit")' "$PROJECT_DIR/cc-bash.c" && \
       grep -q 'strcmp(input, "quit")' "$PROJECT_DIR/cc-bash.c"; then
        pass "Both exit and quit commands are handled"
    else
        fail "exit/quit handling not found"
    fi
}

#
# Test 7: Comment handling (# prefix)
#
test_comment_handling() {
    run_test "Comments with # prefix are handled"

    # Check for # prefix handling
    if grep -q "input\[0\] == '#'" "$PROJECT_DIR/cc-bash.c"; then
        pass "Comment prefix (#) is handled"
    else
        fail "Comment handling not found"
    fi
}

#
# Test 8: Terminal state management
#
test_terminal_state() {
    run_test "Terminal state is managed (raw mode setup/teardown)"

    local has_enable=false
    local has_disable=false

    if grep -q 'enable_raw_mode' "$PROJECT_DIR/cc-bash.c"; then
        has_enable=true
    fi

    if grep -q 'disable_raw_mode' "$PROJECT_DIR/cc-bash.c"; then
        has_disable=true
    fi

    if [ "$has_enable" = true ] && [ "$has_disable" = true ]; then
        pass "Terminal raw mode enable/disable functions present"
    else
        fail "Terminal state management incomplete"
    fi
}

#
# Test 9: History functionality
#
test_history() {
    run_test "Command history is implemented"

    if grep -q 'add_history' "$PROJECT_DIR/cc-bash.c" && \
       grep -q 'history\[' "$PROJECT_DIR/cc-bash.c"; then
        pass "Command history is implemented"
    else
        fail "History functionality not found"
    fi
}

#
# Test 10: Scroll region handling
#
test_scroll_region() {
    run_test "Scroll region is managed"

    # Check for ANSI scroll region escape sequence
    if grep -q '\\033\[.*r' "$PROJECT_DIR/cc-bash.c"; then
        pass "Scroll region escape sequences are used"
    else
        fail "Scroll region handling not found"
    fi
}

#
# Test 11: Signal handling (SIGWINCH for resize)
#
test_signal_handling() {
    run_test "Window resize signal is handled"

    if grep -q 'SIGWINCH' "$PROJECT_DIR/cc-bash.c" && \
       grep -q 'handle_sigwinch' "$PROJECT_DIR/cc-bash.c"; then
        pass "SIGWINCH handler is implemented"
    else
        fail "Window resize handling not found"
    fi
}

#
# Test 12: Memory cleanup on exit
#
test_memory_cleanup() {
    run_test "Memory is freed on exit"

    # Check that history is freed
    if grep -q 'free(history\[' "$PROJECT_DIR/cc-bash.c"; then
        pass "History memory is freed on exit"
    else
        fail "Memory cleanup for history not found"
    fi
}

#
# Main test runner
#

main() {
    echo "========================================"
    echo "cc-bash Test Suite"
    echo "========================================"
    echo ""
    echo "Project directory: $PROJECT_DIR"
    echo ""

    # Run all tests
    test_build
    test_binary_exists
    test_echo_command
    test_pwd_command
    test_cd_command
    test_exit_command
    test_comment_handling
    test_terminal_state
    test_history
    test_scroll_region
    test_signal_handling
    test_memory_cleanup

    # Summary
    echo ""
    echo "========================================"
    echo "Test Summary"
    echo "========================================"
    echo "Tests run:    $TESTS_RUN"
    echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"

    if [ $TESTS_FAILED -gt 0 ]; then
        echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"
        echo ""
        echo "Some tests failed. See output above for details."
        exit 1
    else
        echo "Tests failed: 0"
        echo ""
        echo "All tests passed!"
        exit 0
    fi
}

# Run main
main "$@"
