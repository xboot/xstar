# net

网络设备传输工具。

## 用法

```
net <device> server <type> <port>
net <device> client <type> <host> <port>
```

## 说明

- 通过 TCP 或 UDP 提供交互式网络终端
- **服务端模式**：监听端口，接受一个连接，中继键盘输入到网络，将接收数据输出到控制台
- **客户端模式**：连接到远程主机端口，同样的交互式中继
- `<type>`：`tcp` 或 `udp`
- 不可打印字符显示为 `.`
- 按 Ctrl-C 断开连接

## 示例

```bash
# 启动 TCP 服务
net net-linux.0 server tcp 8080

# 连接到远程服务
net net-linux.0 client tcp 192.168.1.100 8080
```
