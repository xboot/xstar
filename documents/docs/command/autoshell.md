# autoshell

等待并进入 Shell 模式。

## 用法

```
autoshell [millisecond]
```

## 说明

- 在倒计时期间等待按键。默认超时 1000ms
- 显示：`Press any key to enter shell mode:0.000`
- 按键则进入交互式 Shell 模式
- 超时则静默返回，允许启动流程继续
- 用于启动脚本中给用户中断启动的机会

## 示例

```bash
# 等待 1 秒
autoshell

# 等待 5 秒
autoshell 5000
```
