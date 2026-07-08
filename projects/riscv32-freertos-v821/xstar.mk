#
# Project Setting
#

ASFLAGS		+= -I $(PRJDIR)/startup/include
CFLAGS		+= -I $(PRJDIR)/startup/include
CXXFLAGS	+= -I $(PRJDIR)/startup/include

ASFLAGS		+= -I $(PRJDIR)/freertos/kernel/include -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V/chip_specific_extensions/andes_a27l2
CFLAGS		+= -I $(PRJDIR)/freertos/kernel/include -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V/chip_specific_extensions/andes_a27l2
CXXFLAGS	+= -I $(PRJDIR)/freertos/kernel/include -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V -I $(PRJDIR)/freertos/kernel/portable/GCC/RISC-V/chip_specific_extensions/andes_a27l2

ASFLAGS		+= -ffunction-sections -fdata-sections
CFLAGS		+= -ffunction-sections -fdata-sections
CXXFLAGS	+= -ffunction-sections -fdata-sections
LDFLAGS		+= -Wl,-gc-sections

ASFLAGS		+= -Wa,--noexecstack -ffreestanding
CFLAGS		+= -Wa,--noexecstack -ffreestanding
CXXFLAGS	+= -Wa,--noexecstack -ffreestanding
LDFLAGS		+= -z noexecstack -T $(PRJDIR)/xstar.ld -nostartfiles -nostdinc -nostdlib
OCFLAGS		+=
ODFLAGS		+=
MCFLAGS		+= -march=rv32imfdcxandes -mabi=ilp32d -mcmodel=medany -fno-stack-protector -mstrict-align
LIBDIRS		+=
LIBS 		+= -lgcc

end:
	$(Q)echo [OC] Objcopying xstar.bin
	$(Q)$(OC) -O binary $(XSTAR) $(XSTAR).bin
