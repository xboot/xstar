# Rockchip RK3506

## Compile source code and generate the target files at the output directory
```shell
cd projects/arm32-baremetal-rk3506
make defconfig
make CROSS_COMPILE=/path/to/arm-linux-gnueabihf-
```

## Enter maskrom mode and using xrock for program to RAM and execute
```shell
xrock extra maskrom --rc4 off --sram rk3506b_ddr_750MHz_v1.04.bin --delay 10
xrock extra maskrom-write-arm32 --rc4 off 0x00000000 xstar.bin;
xrock extra maskrom-exec-arm32 --rc4 off 0x00000000
```

## Download xrock source code
```shell
git clone https://github.com/xboot/xrock.git
```

## Make and install xrock
```shell
make
sudo make install
```
