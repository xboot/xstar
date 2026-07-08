# Allwinner T527

## Compile source code and generate the target files at the output directory
```shell
cd projects/arm64-baremetal-t527
make defconfig
make CROSS_COMPILE=/path/to/aarch64-linux-gnu-
```

## Program to RAM and execute
```shell
sudo xfel ddr lpddr4; sudo xfel write 0x40000000 xstar.bin; sudo xfel extra exec arm64 0x40000000;
```

## Download xfel tool's source code
```shell
git clone https://github.com/xboot/xfel.git
```

## Make and install xfel tool
```shell
make
sudo make install
```
