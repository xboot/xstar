# 解释器 (shell)

XSTAR 的命令解释器，提供交互式命令输入、命令解析、命令查找与执行功能，支持分号分隔多命令、引号包裹参数和密码保护。

## 工作原理

### 交互式循环

`shell` 命令启动后进入交互式循环：

1. 显示提示符 `xstar: <当前路径># `
2. 通过 `shell_readline()` 读取用户输入
3. 调用 `shell_system()` 解析并执行命令
4. 回到步骤 1，循环等待下一次输入

### 密码保护

若配置了 `CONFIG_XSTAR_SHELL_PASSWORD`（SHA-256 哈希值，64 位十六进制字符串），启动 shell 前需输入密码验证：

1. 提示用户输入密码（`shell_password()`，输入不可见）
2. 对输入计算 SHA-256 哈希
3. 与配置的哈希值比较，匹配则进入 shell，否则重试

### 命令解析

`shell_system()` 对命令行的处理分为两层：

**第一层：分号分割**（`shell_splitcmd`）

按 `;` 分割命令行，支持引号内的分号不分割。每个子命令独立执行。

**第二层：分词**（`shell_tokenize`）

对每个子命令进行词法分析：

- 空白字符（空格、制表符）作为分隔符
- 双引号 `"` 包裹的内容作为单个参数，内部不进行分词
- 单引号 `'` 包裹的内容作为单个参数，内部不进行分词
- 引号不匹配或引号后紧跟非空白字符视为语法错误

### 命令查找与执行

分词后，第一个参数作为命令名，通过 `search_command()` 在已注册命令列表中查找：

- 找到则调用 `c->exec(argc, argv)` 执行
- 未找到则输出 `could not found '<name>' command`

## API

| 函数 | 说明 |
|------|------|
| `shell_system(cmdline)` | 解析并执行命令行字符串，支持分号分隔多命令，返回最后一条命令的执行结果 |

### 相关接口

以下接口由 shell 子系统的其他模块提供：

| 函数 | 头文件 | 说明 |
|------|--------|------|
| `shell_printf(fmt, ...)` | `kernel/shell/context.h` | 格式化输出到 shell 终端 |
| `shell_getcwd()` | `kernel/shell/context.h` | 获取当前工作目录 |
| `shell_setcwd(path)` | `kernel/shell/context.h` | 设置当前工作目录 |
| `shell_readline(prompt)` | `kernel/shell/readline.h` | 显示提示符并读取一行输入，返回动态分配的字符串 |
| `shell_password(prompt)` | `kernel/shell/readline.h` | 读取密码输入（不回显），返回动态分配的字符串 |

## 用法示例

### 编程式执行命令

```c
#include <kernel/shell/shell.h>

/* 执行单条命令 */
shell_system("ls /romdisk");

/* 执行多条命令（分号分隔） */
shell_system("cd /romdisk; ls; cat default.json");
```

### 注册自定义命令

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

## 说明

- `shell` 命令本身通过 `command_initcall()` 注册，是一个普通命令
- `shell_system()` 可在代码中直接调用，无需进入交互式 shell
- 命令行解析支持双引号和单引号，但不支持引号嵌套
- 分号 `;` 用于分隔多条命令，引号内的分号不被分割
- 密码保护通过 `CONFIG_XSTAR_SHELL_PASSWORD` 配置，值为密码的 SHA-256 哈希（64 位十六进制字符串），留空则无密码保护
