#
# Project Setting
#

ASFLAGS		+= -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS		+= -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L
CXXFLAGS	+= -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L

#
# sdl
#
CFLAGS		+= -I $(PRJDIR)/win/SDL3-3.4.10/x86_64-w64-mingw32/include
CXXFLAGS	+= -I $(PRJDIR)/win/SDL3-3.4.10/x86_64-w64-mingw32/include
LIBDIRS		+= -L $(PRJDIR)/win/SDL3-3.4.10/x86_64-w64-mingw32/lib
LIBS		+= -lSDL3

ASFLAGS		+=
CFLAGS		+=
CXXFLAGS	+=
LDFLAGS		+= $(PRJDIR)/xstar.res
OCFLAGS		+=
ODFLAGS		+=
MCFLAGS		+=
LIBDIRS		+=
LIBS 		+= -lpthread

XSTAR		:= $(OUTDIR)/xstar.exe

end:
	$(Q)echo [STRIP] Striping xstar.exe
	$(Q)$(STRIP) $(XSTAR)
	$(Q)echo [ZIP] Ziping xstar.zip
	$(Q)@zip -rjFS $(OUTDIR)/xstar.zip $(XSTAR) $(PRJDIR)/win/pack
