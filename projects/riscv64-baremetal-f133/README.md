# Allwinner F133

## Compile source code and generate the target files at the output directory
```shell
cd projects/riscv64-baremetal-f133
make defconfig
make CROSS_COMPILE=/path/to/riscv64-unknown-elf-
```

## Program to RAM and execute
```shell
sudo xfel ddr f133; sudo xfel write 0x40000000 xstar.bin; sudo xfel exec 0x40000000;
```

## Download riscv toolchain
```
https://occ.t-head.cn/
Xuantie-900-gcc-elf-newlib-x86_64-V3.2.0-20250627.tar.gz
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

