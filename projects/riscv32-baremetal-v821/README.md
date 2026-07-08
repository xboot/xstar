# Allwinner V821

## Compile source code and generate the target files at the output directory
```shell
cd projects/riscv32-baremetal-v821
make defconfig
make CROSS_COMPILE=/path/to/riscv32-elf-
```

## Program to RAM and execute
```shell
xfel ddr; xfel write 0x80000000 output/xstar.bin; xfel extra exec riscv 0x80000000;
```

## Download andes a27l2 riscv32 toolchain
```
https://github.com/andestech/Andes-Development-Kit/releases
nds32le-elf-newlib-v5d.txz
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
