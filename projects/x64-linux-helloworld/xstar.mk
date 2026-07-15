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

SDKDIR		:= $(OUTDIR)/sdk

end:
	$(Q)echo "[SDK] Generating xstar sdk"
	$(Q)$(RM) $(SDKDIR)
	$(Q)$(MKDIR) $(SDKDIR) $(SDKDIR)/inc $(SDKDIR)/inc/project $(SDKDIR)/lib
	$(Q)cat $(OUTDIR)/.objects.lst | tr ' ' '\n' | grep '\.o$$' | grep -v 'main\.o' | xargs $(AR) rcs $(SDKDIR)/lib/libxstar.a
	$(Q)$(STRIP) --strip-debug $(SDKDIR)/lib/libxstar.a
	$(Q)$(CD) $(TOPDIR) && $(FIND) xstar -name '*.h' | xargs $(CP) --parents -t $(SDKDIR)/inc
	$(Q)$(CD) $(TOPDIR) && $(FIND) packages -name '*.h' | xargs $(CP) --parents -t $(SDKDIR)/inc
	$(Q)$(CD) $(PRJDIR) && $(FIND) . -name '*.h' -not -path './output/*' | xargs $(CP) --parents -t $(SDKDIR)/inc/project
	$(Q)$(CP) $(TOPDIR)/.config.h $(SDKDIR)/inc/
	$(Q)$(CP) $(TOPDIR)/.config $(SDKDIR)/
	$(Q)$(CP) $(PRJDIR)/main.c $(SDKDIR)/
	$(Q)$(CP) $(PKGDIR)/packages.mk $(SDKDIR)/
	$(Q)$(CP) $(PRJDIR)/Makefile.sdk $(SDKDIR)/Makefile
