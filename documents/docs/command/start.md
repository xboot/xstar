# start

在后台线程中运行一条 shell 命令，立即返回，不阻塞当前 shell。

## 用法

```
start <command> [args...]
```

## 说明

- 将 `start` 之后的全部参数按空格拼接成一条命令，在新线程中交给 `shell_system()` 执行
- 命令在后台异步运行，`start` 立即返回，不等待执行结束
- 需要平台支持线程特性，不支持时提示 `start: thread is not supported on this platform`
- 后台命令的输出会与当前 shell 的输出交错显示
- 用双引号将整行命令包裹时，可使用 `;` 在后台链式执行多条命令

## 示例

```bash
# 后台延时 1 秒，shell 不被阻塞
start delay 1000

# 后台遍历目录，继续做其他事情
start ls -l /romdisk

# 引号包裹整行，链式执行：先延时再输出
start "delay 1000; echo done"
```
