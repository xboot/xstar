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
MCFLAGS		+= -march=rv64gcv0p7_zfh_xtheadc_xtheadcmo_xtheadsync -mabi=lp64d -mtune=c906 -mcmodel=medlow -fno-stack-protector -mstrict-align
LIBDIRS		+=
LIBS 		+= -lgcc

end:
	$(Q)echo [OC] Objcopying xstar.bin
	$(Q)$(OC) -O binary $(XSTAR) $(XSTAR).bin
