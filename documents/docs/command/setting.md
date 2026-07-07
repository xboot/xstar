# setting

全局设置管理工具。

## 用法

```
setting                     - 打印所有键值对
setting set <key> [<value>] - 设置或清除键
setting get <key> [...]     - 获取键值
setting clear               - 清除所有设置
```

## 说明

- 管理全局键值存储，用于系统配置持久化
- `set` 无值时清除该键
- `clear` 重置所有设置

## 示例

```bash
# 查看所有设置
setting

# 设置值
setting set wifi.ssid "MyNetwork"

# 获取值
setting get wifi.ssid

# 清除设置
setting clear
```
