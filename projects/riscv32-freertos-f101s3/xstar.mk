#
# Project Setting
#

ASFLAGS		+= -I $(PRJDIR)/startup/include
CFLAGS		+= -I $(PRJDIR)/startup/include
CXXFLAGS	+= -I $(PRJDIR)/startup/include

ASFLAGS		+= -I $(PRJDIR)/freertos
CFLAGS		+= -I $(PRJDIR)/freertos
CXXFLAGS	+= -I $(PRJDIR)/freertos

ASFLAGS		+= -I $(PRJDIR)/freertos/kernel/include -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions
CFLAGS		+= -I $(PRJDIR)/freertos/kernel/include -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions
CXXFLAGS	+= -I $(PRJDIR)/freertos/kernel/include -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V/chip_specific_extensions/RISCV_MTIME_CLINT_no_extensions

ASFLAGS		+= -DconfigENABLE_FPU=1 -DconfigENABLE_VPU=1
CFLAGS		+= -DconfigENABLE_FPU=1 -DconfigENABLE_VPU=1
CXXFLAGS	+= -DconfigENABLE_FPU=1 -DconfigENABLE_VPU=1

ASFLAGS		+= -ffunction-sections -fdata-sections
CFLAGS		+= -ffunction-sections -fdata-sections
CXXFLAGS	+= -ffunction-sections -fdata-sections
LDFLAGS		+= -Wl,-gc-sections

ASFLAGS		+= -Wa,--noexecstack -ffreestanding
CFLAGS		+= -Wa,--noexecstack -ffreestanding
CXXFLAGS	+= -Wa,--noexecstack -ffreestanding
LDFLAGS		+= -z noexecstack -T $(PRJDIR)/xstar.ld -nostartfiles -nostdinc -nostdlib
LDFLAGS		+= -march=rv32imafdcv_zifencei_zicbom_zicbop_zicboz_zicond_zihintntl_zihintpause_zawrs_zfa_zfbfmin_zfh_zba_zbb_zbc_zbs_zvfbfmin_zvfbfwma_svinval_svnapot_svpbmt_xtheadc_xtheadvdot -mabi=ilp32d
OCFLAGS		+=
ODFLAGS		+=
MCFLAGS		+= -march=rv32imafdcv_zifencei_zicbom_zicbop_zicboz_zicond_zihintntl_zihintpause_zawrs_zfa_zfbfmin_zfh_zba_zbb_zbc_zbs_zvfbfmin_zvfbfwma_svinval_svnapot_svpbmt_xtheadc_xtheadvdot -mabi=ilp32d -mtune=c907 -mcmodel=medany -fno-stack-protector -mstrict-align
LIBDIRS		+=
LIBS 		+= -lgcc

end:
	$(Q)echo [OC] Objcopying xstar.bin
	$(Q)$(OC) -O binary $(XSTAR) $(XSTAR).bin
