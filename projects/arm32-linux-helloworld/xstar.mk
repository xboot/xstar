#
# Project Setting
#

ASFLAGS		+= -D_GNU_SOURCE
CFLAGS		+= -D_GNU_SOURCE
CXXFLAGS	+= -D_GNU_SOURCE

ASFLAGS		+= -ffunction-sections -fdata-sections
CFLAGS		+= -ffunction-sections -fdata-sections
CXXFLAGS	+= -ffunction-sections -fdata-sections
LDFLAGS		+= -Wl,-gc-sections

ASFLAGS		+= -Wa,--noexecstack -ffreestanding
CFLAGS		+= -Wa,--noexecstack -ffreestanding
CXXFLAGS	+= -Wa,--noexecstack -ffreestanding
LDFLAGS		+= -z noexecstack
OCFLAGS		+=
ODFLAGS		+=
MCFLAGS		+=
LIBDIRS		+=
LIBS 		+= -lc -lm -lgcc -lrt -lpthread -lasound
