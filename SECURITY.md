# Security Policy

## Reporting Security Issues

**Please do not report security vulnerabilities through public GitHub issues.**

Instead, please email security concerns to the repository maintainer directly. You should receive a response within 48 hours.

Please include:
- Description of the vulnerability
- Steps to reproduce
- Potential impact
- Suggested fix (if you have one)

## Security Review Status

**Last Review:** 2025-11-23
**Status:** Phase 2 Security Audit Complete

### Summary

A comprehensive security audit identified **17 security issues** across the SmartTerm library implementation:

- **Critical:** 2 issues
- **High:** 4 issues
- **Medium:** 5 issues
- **Low:** 6 issues

## Critical Issues Identified

### 1. Buffer Overflow via sprintf (TRACKED)

**Location:** `lib/smartterm/smartterm_export.c` (multiple locations)
**Impact:** Potential buffer overflow if size calculations are incorrect
**Status:** Documented for fix in future release
**Mitigation:** Current buffer size calculations appear safe, but should be replaced with `snprintf()`

### 2. Null Pointer Dereference from localtime()

**Location:** `lib/smartterm/smartterm_export.c` (multiple locations)
**Impact:** Crash if `localtime()` returns NULL for invalid timestamps
**Status:** Documented for fix
**Mitigation:** Add NULL checks for `localtime()` return value before calling `strftime()`

## High Priority Issues

### 3. Race Condition - Use After Unlock

**Location:** `lib/smartterm/smartterm_output.c:146-157`
**Impact:** Use-after-free in multi-threaded scenarios
**Status:** Documented for redesign
**Mitigation:** Document API requirement or return copies

### 4. Undefined Behavior - Uninitialized Mutex

**Location:** `lib/smartterm/smartterm_output.c`
**Impact:** Undefined behavior when destroying uninitialized mutex
**Status:** Documented for fix
**Mitigation:** Only destroy mutex if it was initialized

### 5. Thread Safety - Global State

**Location:** `lib/smartterm/smartterm_input.c:14`
**Impact:** Tab completion not thread-safe
**Status:** Known limitation, documented
**Mitigation:** Use thread-local storage or redesign completion API

### 6. Memory Allocation Failure Not Checked

**Location:** `lib/smartterm/smartterm_core.c:145-147`
**Impact:** Crash if memory allocation fails
**Status:** Documented for fix
**Mitigation:** Add NULL checks after all allocations

## Medium Priority Issues

- Potential null pointer after strdup_safe failure
- Missing bounds check before array access
- Integer underflow in scroll calculation
- Format string vulnerability risk (user-provided format strings)
- Other validation and edge case issues

See full security audit report for details.

## Low Priority Issues

- Use of unsafe `strcpy()` (with constant strings)
- Missing pthread_mutex_lock error checking
- Potential integer overflow in buffer size calculation
- Other best practices improvements

## Planned Fixes

### Version 1.0.1 (Security Patch - Planned)

**Critical Fixes:**
- Replace all `sprintf()` with `snprintf()` in export.c
- Add NULL checks after `localtime()` calls

**High Priority Fixes:**
- Fix mutex initialization/cleanup logic
- Add NULL checks after all memory allocations
- Document thread safety limitations

**Medium Priority Fixes:**
- Add bounds validation in buffer operations
- Check all `strdup_safe()` return values

### Version 1.1.0 (Future)

**Low Priority:**
- Replace all `strcpy()` with safe alternatives
- Comprehensive pthread error checking
- Static analysis integration

## Current Mitigations

While these issues are tracked for fixing, current mitigations include:

1. **Library Usage:** SmartTerm is designed for terminal UI applications, not security-critical or network-facing code
2. **Buffer Calculations:** Current size calculations in export.c are conservative and have margin
3. **Thread Safety:** Documentation clearly states thread-safety requirements
4. **Memory Allocations:** Failures are unlikely on modern systems with virtual memory
5. **Testing:** Examples demonstrate proper usage patterns

## Security Best Practices for Users

When using SmartTerm:

1. **Don't pass untrusted format strings** to `smartterm_write_fmt()`
2. **Check return values** from all API functions
3. **Use thread-safe mode** if writing from multiple threads
4. **Validate user input** before displaying it
5. **Test with valgrind** to catch memory issues in your code

## Secure Coding Guidelines

Contributors should follow these practices:

1. **Never use unsafe functions:** `sprintf()`, `strcpy()`, `strcat()`, `gets()`
2. **Always check return values:** `malloc()`, `calloc()`, `strdup()`, pthread functions
3. **Use safe alternatives:** `snprintf()`, `strncpy()` (with manual null-termination), or `strlcpy()` (BSD systems only)
4. **Validate inputs:** Check pointers for NULL, bounds for integers, sizes for buffers
5. **Test edge cases:** Zero-length buffers, NULL pointers, maximum sizes
6. **Review for races:** Consider multi-threaded scenarios for all shared state

## Static Analysis

Future releases will integrate:
- **clang-analyzer** for static code analysis
- **valgrind** for memory leak detection
- **ThreadSanitizer** for race condition detection
- **AddressSanitizer** for memory safety issues

## Disclosure Policy

- Security issues will be disclosed after a fix is available
- Critical issues will be patched within 7 days of verification
- High priority issues within 30 days
- Medium/Low priority issues in next minor/major release
- CVEs will be requested for externally reported critical issues

## Security Contact

For security concerns, contact the repository maintainer through GitHub.

---

**Note:** SmartTerm is a terminal UI library for interactive applications. It is not designed for security-critical or network-facing scenarios. Use appropriate caution when handling untrusted input.
