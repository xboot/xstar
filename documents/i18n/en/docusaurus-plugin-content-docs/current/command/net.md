# net

Network device transfer tool.

## Usage

```
net <device> server <type> <port>
net <device> client <type> <host> <port>
```

## Notes

- Provides an interactive network terminal over TCP or UDP
- **Server mode**: Listens on a port, accepts one connection, relays keyboard input to the network, outputs received data to the console
- **Client mode**: Connects to a remote host port with the same interactive relay
- `<type>`: `tcp` or `udp`
- Non-printable characters are displayed as `.`
- Press Ctrl-C to disconnect

## Examples

```bash
# Start a TCP server
net net-linux.0 server tcp 8080

# Connect to a remote server
net net-linux.0 client tcp 192.168.1.100 8080
```
