# Allwinner F101S3

## Compile source code and generate the target files at the output directory
```shell
cd projects/riscv64-baremetal-f101s3
make defconfig
make CROSS_COMPILE=~/software/Xuantie-900-gcc-elf-newlib-x86_64-V3.2.0/bin/riscv64-unknown-elf-
```

## Program to RAM and execute
```shell
xfel ddr f101-s3; xfel write 0x40000000 output/xstar.bin; xfel extra exec riscv64 0x40000000;
```

## Download xuantie riscv toolchain
```
wget -c https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource//1751370399722/Xuantie-900-gcc-elf-newlib-x86_64-V3.2.0-20250627.tar.gz
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
