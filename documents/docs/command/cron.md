# cron

基于 cron 表达式的定时任务管理工具，到点自动执行指定的 shell 命令。

## 用法

```
cron                             - 列出所有任务
cron add <name> <expr> <cmd>     - 添加周期性任务
cron oneshot <name> <expr> <cmd> - 添加一次性任务
cron remove <name>               - 移除任务
```

## 说明

- `expr` 为标准 5 段 cron 表达式，含空格需用双引号包裹：`"<minute> <hour> <day> <month> <week>"`
- `cmd` 为到点执行的 shell 命令，含空格需用双引号包裹
- 时区从全局设置 `timezone` 读取，未设置时默认 `Asia/Shanghai`
- `add` 添加的任务（periodic）每次匹配都会执行；`oneshot` 任务执行一次后自动移除
- 任务 `name` 需唯一，重名添加或表达式非法都会失败
- 表达式语法详见 [Cron 定时任务调度器](../subsys/cron/cron.md)

### 表达式字段

| 字段 | 范围 | 说明 |
| --- | --- | --- |
| 分 | 0-59 | 分钟 |
| 时 | 0-23 | 小时 |
| 日 | 1-31 | 每月第几天 |
| 月 | 1-12 或 JAN-DEC | 月份，可用名称缩写 |
| 周 | 0-6 或 SUN-SAT | 星期，0=周日（7 等价于 0）|

每字段支持 `*`、`n`、`a-b`、`*/s`、`a,b,c` 等写法。当日和星期都被显式指定时，命中其一即匹配（或），否则需同时匹配（与）。

## 示例

```bash
# 列出所有任务
cron

# 每分钟执行
cron add t1 "* * * * *" "echo hi"

# 每 5 分钟执行一次
cron add t2 "*/5 * * * *" "echo tick"

# 元旦 0 点执行一次（一次性）
cron oneshot newyear "0 0 1 1 *" "echo happy new year"

# 每周一 8:30 执行
cron add weekly "30 8 * * 1" "echo monday"

# 移除任务
cron remove t1
```
