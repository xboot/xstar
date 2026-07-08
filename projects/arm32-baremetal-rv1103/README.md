# Rockchip RV1103

## Compile source code and generate the target files at the output directory
```shell
cd projects/arm32-baremetal-rv1103
make defconfig
make CROSS_COMPILE=/path/to/arm-linux-gnueabihf-
```

## Enter maskrom mode and using xrock for program to RAM and execute
```shell
xrock extra maskrom --rc4 off --sram rv1103_ddr_924MHz_v1.15.bin --delay 10;
xrock extra maskrom-write-arm32 --rc4 off 0x00000000 xstar.bin;
xrock extra maskrom-exec-arm32 --rc4 off 0x00000000
```

## Enter maskrom mode and using xrock for burning spi nor flash (offset = 0x00010000, sector = 128)
```shell
xrock download download.bin
xrock flash write 128 xstarpak.bin;
xrock reset;
```

## Enter maskrom mode and using xrock for burning spi nand flash (offset = 0x00040000, sector = 512)
```shell
xrock download download.bin
xrock flash write 512 xstarpak.bin;
xrock reset;
```

## Enter maskrom mode and using xrock for burning emmc (offset = 0x00008000, sector = 64)
```shell
xrock download download.bin
xrock flash write 64 xstarpak.bin;
xrock reset;
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
