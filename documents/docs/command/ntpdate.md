# ntpdate

通过 NTP 设置日期和时间。

## 用法

```
ntpdate [device] [host]
```

## 说明

- 通过网络时间协议同步系统时间
- `device`：网络设备名称（可选）
- `host`：NTP 服务器地址（可选，默认 `pool.ntp.org`）
- 成功时打印当前 UTC 时间
- 失败时显示 `Can't sync date and time via ntp.`

## 示例

```bash
# 默认同步
ntpdate

# 指定设备和服务器
ntpdate net-linux.0 pool.ntp.org
```
