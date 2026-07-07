## CH347 

`CH347` 是南京沁恒微电子（WCH）生产的一款高速USB总线转接芯片，支持USB转UART、I2C、SPI、GPIO、JTAG等多种接口。

## 安装驱动程序

```shell
git clone https://github.com/WCHSoftGroup/ch341par_linux
cd ch341par_linux/driver
sudo make install
```

## 创建规则文件
```shell
sudo vim /etc/udev/rules.d/99-ch34x-pis.rules
```
```shell
KERNEL=="ch34x_pis[0-9]*", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55db", SYMLINK+="ch34x_pis", MODE="0666", GROUP="dialout"
KERNEL=="ch34x_pis[0-9]*", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55de", SYMLINK+="ch34x_pis", MODE="0666", GROUP="dialout"
```
## 应用规则文件

```shell
sudo udevadm control --reload-rules
sudo udevadm trigger
sudo systemctl restart systemd-udevd
```