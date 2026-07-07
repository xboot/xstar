#
# Project Setting
#

ASFLAGS		+= -D_GNU_SOURCE
CFLAGS		+= -D_GNU_SOURCE
CXXFLAGS	+= -D_GNU_SOURCE

#
# ch347
#
CFLAGS		+= -I $(PRJDIR)/driver/ch347
CXXFLAGS	+= -I $(PRJDIR)/driver/ch347
LIBDIRS		+= -L $(PRJDIR)/driver/ch347/lib
LIBS 		+= -lch347

#
# sdl
#
CFLAGS		+= $(shell pkg-config --cflags sdl3)
CXXFLAGS	+= $(shell pkg-config --cflags sdl3)
LIBDIRS		+=
LIBS		+= $(shell pkg-config --libs sdl3)

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
