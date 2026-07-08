#
# Project Setting
#

ASFLAGS		+= -I $(PRJDIR)/startup/include
CFLAGS		+= -I $(PRJDIR)/startup/include
CXXFLAGS	+= -I $(PRJDIR)/startup/include

ASFLAGS		+= -I $(PRJDIR)/baremetal
CFLAGS		+= -I $(PRJDIR)/baremetal
CXXFLAGS	+= -I $(PRJDIR)/baremetal

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
MCFLAGS		+= -march=armv8.2-a -mcpu=cortex-a55 -mtune=cortex-a55 -mstrict-align
LIBDIRS		+=
LIBS 		+= -lgcc

end:
	$(Q)echo [OC] Objcopying xstar.bin
	$(Q)$(OC) -O binary $(XSTAR) $(XSTAR).bin
