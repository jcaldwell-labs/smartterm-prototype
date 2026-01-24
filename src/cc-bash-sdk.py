#!/usr/bin/env python3
"""
cc-bash-sdk: Claude Code-style bash wrapper with AI integration

A Python rewrite of cc-bash that integrates the Claude Agent SDK for
AI-assisted shell operations.

Features:
- Execute bash commands with colored output
- @ask <question> - Ask Claude anything
- @explain - Have Claude explain the last command output
- @fix - Have Claude suggest fix for last error
- @cmd <description> - Generate command from description
- Status bar, history, readline-like input

Usage:
    source ~/venvs/claude-agent/bin/activate
    python cc-bash-sdk.py
"""

import asyncio
import os
import subprocess
import sys
from datetime import datetime
from pathlib import Path

from prompt_toolkit import PromptSession
from prompt_toolkit.history import FileHistory
from prompt_toolkit.styles import Style
from rich.console import Console
from rich.panel import Panel
from rich.text import Text

# Claude SDK imports
from claude_agent_sdk import query, ClaudeAgentOptions
from claude_agent_sdk.types import AssistantMessage, ResultMessage, TextBlock


# =============================================================================
# Configuration
# =============================================================================

HISTORY_FILE = Path.home() / ".cc-bash-sdk-history"

STYLE = Style.from_dict({
    'prompt': 'cyan bold',
    'command': 'cyan',
})

console = Console()


# =============================================================================
# Helper Functions
# =============================================================================

def extract_text(content) -> str:
    """Extract text from Claude message content."""
    if isinstance(content, str):
        return content
    if isinstance(content, list):
        return ''.join(
            block.text for block in content
            if isinstance(block, TextBlock) or hasattr(block, 'text')
        )
    return str(content)


def get_status_bar() -> str:
    """Generate status bar content."""
    user = os.environ.get('USER', 'user')
    hostname = os.uname().nodename
    cwd = os.getcwd()
    # Truncate long paths
    if len(cwd) > 40:
        cwd = "..." + cwd[-37:]
    time_str = datetime.now().strftime("%H:%M:%S")
    return f"  {user}@{hostname}:{cwd}  [{time_str}]"


def print_separator():
    """Print a dim separator line."""
    width = console.width or 80
    console.print("─" * width, style="dim")


def print_status():
    """Print status bar."""
    console.print(get_status_bar(), style="bold")
    console.print("  ⏵⏵ @help for AI commands, exit to quit", style="dim")


# =============================================================================
# Command Execution
# =============================================================================

def run_bash_command(cmd: str) -> tuple[str, str, int]:
    """Execute a bash command and return (stdout, stderr, exit_code)."""
    try:
        result = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            text=True,
            timeout=300,  # 5 minute timeout
            cwd=os.getcwd(),
        )
        return result.stdout, result.stderr, result.returncode
    except subprocess.TimeoutExpired:
        return "", "Command timed out after 5 minutes", 124
    except Exception as e:
        return "", str(e), 1


def handle_cd(path: str) -> bool:
    """Handle cd command, return True if successful."""
    if not path:
        path = str(Path.home())
    else:
        path = os.path.expanduser(path)

    try:
        os.chdir(path)
        return True
    except FileNotFoundError:
        console.print(f"cd: no such directory: {path}", style="red")
        return False
    except PermissionError:
        console.print(f"cd: permission denied: {path}", style="red")
        return False


# =============================================================================
# Claude AI Integration
# =============================================================================

async def ask_claude(prompt: str, context: str = "") -> str:
    """Send a prompt to Claude and get response."""
    full_prompt = prompt
    if context:
        full_prompt = f"Context from terminal session:\n```\n{context}\n```\n\n{prompt}"

    options = ClaudeAgentOptions(
        cwd=os.getcwd(),
        permission_mode="default",
        max_turns=1,
    )

    response_text = ""
    try:
        async for message in query(prompt=full_prompt, options=options):
            if isinstance(message, AssistantMessage):
                response_text += extract_text(message.content)
            elif isinstance(message, ResultMessage) and message.result:
                if not response_text:
                    response_text = message.result
    except Exception as e:
        response_text = f"Error communicating with Claude: {e}"

    return response_text


async def handle_ask(question: str, last_output: str):
    """Handle @ask command."""
    if not question.strip():
        console.print("Usage: @ask <your question>", style="yellow")
        return

    console.print(f"[dim]Asking Claude...[/dim]")
    response = await ask_claude(question, context=last_output if last_output else "")
    console.print(Panel(response, title="Claude", border_style="cyan"))


async def handle_explain(last_command: str, last_output: str):
    """Handle @explain command."""
    if not last_command:
        console.print("No previous command to explain", style="yellow")
        return

    console.print(f"[dim]Asking Claude to explain...[/dim]")
    prompt = f"Explain what this command does and what the output means:\n\nCommand: {last_command}\n\nOutput:\n{last_output[:2000]}"
    response = await ask_claude(prompt)
    console.print(Panel(response, title="Explanation", border_style="green"))


async def handle_fix(last_command: str, last_stderr: str):
    """Handle @fix command."""
    if not last_stderr:
        console.print("No error to fix (last command succeeded)", style="yellow")
        return

    console.print(f"[dim]Asking Claude for fix...[/dim]")
    prompt = f"This command failed. Suggest how to fix it:\n\nCommand: {last_command}\n\nError:\n{last_stderr[:2000]}"
    response = await ask_claude(prompt)
    console.print(Panel(response, title="Suggested Fix", border_style="red"))


async def handle_cmd(description: str):
    """Handle @cmd command - generate command from description."""
    if not description.strip():
        console.print("Usage: @cmd <what you want to do>", style="yellow")
        return

    console.print(f"[dim]Generating command...[/dim]")
    prompt = f"Generate a bash command to: {description}\n\nRespond with ONLY the command, no explanation."
    response = await ask_claude(prompt)
    console.print(f"Suggested: [cyan]{response.strip()}[/cyan]")
    console.print("[dim]Copy and run if correct[/dim]")


# =============================================================================
# Main REPL
# =============================================================================

def print_help():
    """Print help message."""
    help_text = """
[bold cyan]cc-bash-sdk[/bold cyan] - Claude Code-style bash with AI

[bold]Commands:[/bold]
  <command>      Execute bash command
  cd <path>      Change directory
  exit / quit    Exit shell

[bold]AI Commands:[/bold]
  @ask <question>     Ask Claude anything
  @explain           Explain last command output
  @fix               Suggest fix for last error
  @cmd <description> Generate command from description

[bold]Other:[/bold]
  @help / @h     Show this help
  @clear / @c    Clear screen
  # <note>       Add note (not executed)
"""
    console.print(help_text)


async def main():
    """Main REPL loop."""
    console.clear()
    console.print("[bold cyan]cc-bash-sdk[/bold cyan] - Claude Code-style bash with AI")
    console.print("[dim]Type @help for commands[/dim]\n")

    # Initialize prompt session with history
    session = PromptSession(
        history=FileHistory(str(HISTORY_FILE)),
        style=STYLE,
    )

    last_command = ""
    last_stdout = ""
    last_stderr = ""
    last_exit_code = 0

    while True:
        try:
            # Print status
            print_separator()
            print_status()
            print_separator()

            # Get input
            user_input = await session.prompt_async(
                [('class:prompt', '› ')],
            )

            user_input = user_input.strip()

            if not user_input:
                continue

            # Handle exit
            if user_input in ('exit', 'quit', '@quit', '@q'):
                console.print("[dim]Goodbye![/dim]")
                break

            # Handle notes (# prefix)
            if user_input.startswith('#'):
                console.print(f"[yellow]{user_input}[/yellow]")
                continue

            # Handle @ commands
            if user_input.startswith('@'):
                cmd_parts = user_input[1:].split(maxsplit=1)
                cmd = cmd_parts[0].lower()
                arg = cmd_parts[1] if len(cmd_parts) > 1 else ""

                if cmd in ('help', 'h'):
                    print_help()
                elif cmd in ('clear', 'c'):
                    console.clear()
                elif cmd == 'ask':
                    await handle_ask(arg, last_stdout)
                elif cmd == 'explain':
                    await handle_explain(last_command, last_stdout + last_stderr)
                elif cmd == 'fix':
                    await handle_fix(last_command, last_stderr)
                elif cmd == 'cmd':
                    await handle_cmd(arg)
                else:
                    console.print(f"Unknown command: @{cmd}", style="red")
                continue

            # Handle cd specially
            if user_input == 'cd' or user_input.startswith('cd '):
                path = user_input[3:].strip() if user_input.startswith('cd ') else ""
                handle_cd(path)
                continue

            # Execute bash command
            console.print(f"[cyan]$ {user_input}[/cyan]")
            stdout, stderr, exit_code = run_bash_command(user_input)

            # Store for AI commands
            last_command = user_input
            last_stdout = stdout
            last_stderr = stderr
            last_exit_code = exit_code

            # Display output
            if stdout:
                console.print(stdout, end="")
            if stderr:
                console.print(stderr, style="red", end="")

            # Show exit code if non-zero
            if exit_code != 0:
                console.print(f"[red][exit: {exit_code}][/red]")

        except KeyboardInterrupt:
            console.print("\n[dim]^C[/dim]")
            continue
        except EOFError:
            console.print("\n[dim]Goodbye![/dim]")
            break
        except Exception as e:
            console.print(f"[red]Error: {e}[/red]")


if __name__ == "__main__":
    asyncio.run(main())
