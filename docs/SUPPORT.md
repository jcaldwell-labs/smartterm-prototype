# Support

Thank you for using cc-bash! This document provides guidance on how to get help with the project.

## Getting Help

### Documentation

Before seeking help, please check our documentation:

- **[README.md](../README.md)** - Main documentation, installation, and usage
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Technical architecture details
- **[CONTRIBUTING.md](../CONTRIBUTING.md)** - Development setup and contributing guide
- **[CHANGELOG.md](../CHANGELOG.md)** - Version history and release notes

### GitHub Issues

For bugs, feature requests, and technical questions:

1. **Search existing issues** - Your question may already be answered
2. **Open a new issue** using the appropriate template:
   - [Bug Report](.github/ISSUE_TEMPLATE/bug_report.md) - For bugs and unexpected behavior
   - [Feature Request](.github/ISSUE_TEMPLATE/feature_request.md) - For new features or enhancements

### GitHub Discussions

For general questions, ideas, and community discussion:

- Use [GitHub Discussions](https://github.com/jcaldwell-labs/smartterm-prototype/discussions) for:
  - How-to questions
  - Sharing your use cases
  - Discussing ideas before opening feature requests
  - Community support

## Common Issues

### Installation Problems

**Issue**: `command not found: cc-bash` after installation

**Solution**: Ensure the installation directory is in your PATH:

```bash
# For system install
echo $PATH | grep "/usr/local/bin"

# For user install
echo $PATH | grep "$HOME/.local/bin"
```

**Issue**: Build fails with `undefined reference to forkpty`

**Solution**: Ensure you have the required dependencies:

```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# The Makefile automatically links -lutil for forkpty
```

### Runtime Issues

**Issue**: Colors not showing in output

**Solution**: Check your terminal supports ANSI colors:

```bash
echo $TERM  # Should be xterm-256color or similar
tput colors # Should show 8 or 256
```

**Issue**: Command history not persisting

**Solution**: Check if `~/.cc-bash-history` is writable:

```bash
ls -la ~/.cc-bash-history
# Should show -rw-r--r-- permissions
```

### Configuration Issues

**Issue**: Aliases/snippets not loading

**Solution**: Check your config file syntax:

```bash
# Edit config
@edit

# Verify no syntax errors, then reload
@reload

# Check if aliases loaded
@alias
```

## Reporting Security Issues

**Do not** open public issues for security vulnerabilities.

See [SECURITY.md](SECURITY.md) for instructions on reporting security issues responsibly.

## Getting Involved

Want to contribute to cc-bash?

- Read our [Contributing Guide](CONTRIBUTING.md)
- Check out [Good First Issues](https://github.com/jcaldwell-labs/smartterm-prototype/labels/good%20first%20issue)
- Join the discussion in [GitHub Discussions](https://github.com/jcaldwell-labs/smartterm-prototype/discussions)

## Commercial Support

cc-bash is an open source project maintained by [jcaldwell-labs](https://github.com/jcaldwell-labs). For commercial support or consulting, please open a discussion or reach out via GitHub.

## Response Times

This is a community-driven open source project. Response times may vary:

- **Critical bugs**: We aim to respond within 48 hours
- **General issues**: Usually within 1 week
- **Feature requests**: May take longer depending on complexity and roadmap

Please be patient and respectful - all maintainers are volunteers.

## Stay Updated

- Watch the repository for updates
- Check the [CHANGELOG](CHANGELOG.md) for new releases
- Follow releases to get notified of new versions

---

Thank you for being part of the cc-bash community!
