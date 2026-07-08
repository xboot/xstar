# Allwinner T113-S3

## Compile source code and generate the target files at the output directory
```shell
cd projects/arm32-baremetal-t113s3
make defconfig
make CROSS_COMPILE=/path/to/arm-linux-gnueabihf-
```

## Program to RAM and execute
```shell
sudo xfel ddr t113-s3; sleep 3; sudo xfel write 0x40000000 xstar.bin; sudo xfel exec 0x40000000;
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
