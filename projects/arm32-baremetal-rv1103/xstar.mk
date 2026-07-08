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
MCFLAGS		+= -march=armv7-a -mtune=cortex-a7 -mfpu=vfpv4 -mfloat-abi=hard -marm -mno-thumb-interwork -mno-unaligned-access
LIBDIRS		+=
LIBS 		+= -lgcc

ifeq ($(strip $(HOSTOS)), linux)
BOOT_MERGER	:= $(PRJDIR)/tools/linux/boot_merger
endif
ifeq ($(strip $(HOSTOS)), windows)
BOOT_MERGER	:= $(PRJDIR)/tools/windows/boot_merger.exe
endif
INIFILE		:= $(PRJDIR)/tools/images/rv1103.ini

end:
	$(Q)echo [OC] Objcopying xstar.bin
	$(Q)$(OC) -O binary $(XSTAR) $(XSTAR).bin
	$(Q)echo Packing rockchip binrary for irom booting
	$(Q)$(CD) $(PRJDIR) && $(BOOT_MERGER) $(INIFILE)
