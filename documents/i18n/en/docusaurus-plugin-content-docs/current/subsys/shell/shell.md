# Shell (shell)

XSTAR's command-line interpreter, providing interactive command input, command parsing, command lookup and execution, with support for semicolon-separated multiple commands, quoted arguments, and password protection.

## How It Works

### Interactive Loop

The `shell` command starts an interactive loop:

1. Display the prompt `xstar: <current_path># `
2. Read user input via `shell_readline()`
3. Parse and execute the command via `shell_system()`
4. Return to step 1, looping for the next input

### Password Protection

If `CONFIG_XSTAR_SHELL_PASSWORD` is configured (a SHA-256 hash value, 64-character hexadecimal string), password verification is required before entering the shell:

1. Prompt the user for a password (`shell_password()`, input is not echoed)
2. Compute the SHA-256 hash of the input
3. Compare with the configured hash value; if matched, enter the shell; otherwise, retry

### Command Parsing

`shell_system()` processes the command line in two layers:

**Layer 1: Semicolon Splitting** (`shell_splitcmd`)

Splits the command line by `;`, with support for semicolons inside quotes not being treated as delimiters. Each sub-command is executed independently.

**Layer 2: Tokenization** (`shell_tokenize`)

Performs lexical analysis on each sub-command:

- Whitespace characters (spaces, tabs) act as delimiters
- Content wrapped in double quotes `"` is treated as a single argument with no internal tokenization
- Content wrapped in single quotes `'` is treated as a single argument with no internal tokenization
- Mismatched quotes or non-whitespace characters immediately following a closing quote are treated as syntax errors

### Command Lookup and Execution

After tokenization, the first argument is used as the command name, and `search_command()` looks it up in the registered command list:

- If found, `c->exec(argc, argv)` is called to execute
- If not found, outputs `could not found '<name>' command`

## API

| Function | Description |
|----------|-------------|
| `shell_system(cmdline)` | Parse and execute a command line string, supports semicolon-separated multiple commands, returns the result of the last command |

### Related Interfaces

The following interfaces are provided by other modules of the shell subsystem:

| Function | Header | Description |
|----------|--------|-------------|
| `shell_printf(fmt, ...)` | `kernel/shell/context.h` | Formatted output to the shell terminal |
| `shell_getcwd()` | `kernel/shell/context.h` | Get the current working directory |
| `shell_setcwd(path)` | `kernel/shell/context.h` | Set the current working directory |
| `shell_readline(prompt)` | `kernel/shell/readline.h` | Display a prompt and read a line of input, returns a dynamically allocated string |
| `shell_password(prompt)` | `kernel/shell/readline.h` | Read password input (no echo), returns a dynamically allocated string |

## Usage Examples

### Programmatic Command Execution

```c
#include <kernel/shell/shell.h>

/* Execute a single command */
shell_system("ls /romdisk");

/* Execute multiple commands (semicolon-separated) */
shell_system("cd /romdisk; ls; cat default.json");
```

### Registering a Custom Command

```c
#include <kernel/command/command.h>

static void my_usage(void)
{
    shell_printf("usage:\r\n");
    shell_printf("    mycmd [options]\r\n");
}

static int do_mycmd(int argc, char ** argv)
{
    shell_printf("hello from mycmd\n");
    return 0;
}

static struct command_t mycmd = {
    .name  = "mycmd",
    .desc  = "my custom command",
    .usage = my_usage,
    .exec  = do_mycmd,
};

command_initcall(register_command_wrapper);
```

## Notes

- The `shell` command itself is registered via `command_initcall()` and is a regular command
- `shell_system()` can be called directly in code without entering the interactive shell
- Command line parsing supports double and single quotes, but not nested quotes
- Semicolons `;` separate multiple commands; semicolons inside quotes are not treated as delimiters
- Password protection is configured via `CONFIG_XSTAR_SHELL_PASSWORD`, the value is the SHA-256 hash of the password (64-character hexadecimal string); empty means no password protection
