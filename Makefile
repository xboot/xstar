export
sinclude $(wildcard .config)

CROSS_COMPILE	?= $(CONFIG_CROSS_COMPILE:"%"=%)
TOPDIR			:= $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
PRJSDIR			:= $(TOPDIR)/projects
PRJDIR			:= $(PRJSDIR)/$(CONFIG_PROJECT_NAME:"%"=%)
PKGDIR			:= $(TOPDIR)/packages
OUTDIR			:= $(PRJDIR)/output
OBJDIR			:= $(OUTDIR)/.obj
PRJKCFGS		:= $(wildcard $(PRJSDIR)/*/Kconfig)
XSTAR			:= $(OUTDIR)/xstar

AS				:= $(CROSS_COMPILE)gcc -x assembler-with-cpp
CC 				:= $(CROSS_COMPILE)gcc
CXX				:= $(CROSS_COMPILE)g++
LD				:= $(CROSS_COMPILE)ld
AR				:= $(CROSS_COMPILE)ar
SZ				:= $(CROSS_COMPILE)size
OC				:= $(CROSS_COMPILE)objcopy
OD				:= $(CROSS_COMPILE)objdump
STRIP			:= $(CROSS_COMPILE)strip
MKDIR			:= mkdir -p
CP				:= cp -af
RM				:= rm -fr
CD				:= cd
FIND			:= find
CPIO			:= cpio -o -H newc --quiet

ifneq ($(V), 1)
	Q			:= @
endif

ifeq ($(OS), Windows_NT)
	HOSTOS		:= windows
else
	ifneq (,$(findstring Linux, $(shell uname -a)))
		HOSTOS	:= linux
	endif
endif

ifeq ($(strip $(HOSTOS)), linux)
CONF			:= $(TOPDIR)/tools/linux/kconfig/conf
MCONF			:= $(TOPDIR)/tools/linux/kconfig/mconf
endif
ifeq ($(strip $(HOSTOS)), windows)
CONF			:= $(TOPDIR)/tools/windows/kconfig/conf.exe
MCONF			:= $(TOPDIR)/tools/windows/kconfig/mconf.exe
endif

INCLUDES		:= -include $(TOPDIR)/.config.h
INCLUDES		+= -I $(TOPDIR)/xstar
INCLUDES		+= -I $(PRJDIR)
INCLUDES		+= -I $(TOPDIR)/xstar/external/jpeg-9d
INCLUDES		+= -I $(TOPDIR)/xstar/external/libcg-1.1.3
INCLUDES		+= -I $(TOPDIR)/xstar/external/libpng-1.6.37
INCLUDES		+= -I $(TOPDIR)/xstar/external/zlib-1.2.11

ASFLAGS			:= $(INCLUDES) -g -ggdb -Wall $(CONFIG_OPTIMIZE_LEVEL:"%"=%) -std=gnu99
CFLAGS			:= $(INCLUDES) -g -ggdb -Wall $(CONFIG_OPTIMIZE_LEVEL:"%"=%) -std=gnu99
CXXFLAGS		:= $(INCLUDES) -g -ggdb -Wall $(CONFIG_OPTIMIZE_LEVEL:"%"=%)
LDFLAGS			:=
OCFLAGS			:= -v -O binary
ODFLAGS			:=
MCFLAGS			:=
LIBDIRS			:=
LIBS 			:=

sinclude $(PRJDIR)/xstar.mk
sinclude $(PKGDIR)/packages.mk

.DEFAULT_GOAL := all
.PHONY: all begin $(XSTAR) end $(PRJSDIR)/.kconfig.projects menuconfig clean distclean help

all: end

end: $(XSTAR)

$(XSTAR): begin
	$(Q)$(MKDIR) $(OUTDIR) $(OBJDIR)
	$(Q)$(RM) $(OUTDIR)/.objects.lst
	$(Q)$(FIND) $(OUTDIR) -name xstar.o -delete
	$(Q)$(FIND) $(OUTDIR) -name romdisk.o -delete
	$(Q)echo [ROMDISK] Packing romdisk
	$(Q)$(CD) $(PRJDIR) && $(FIND) romdisk | $(CPIO) > $(OBJDIR)/romdisk.cpio && $(CD) $(TOPDIR)
	$(Q)$(MAKE) --no-print-directory -f Makefile.rules dir=$(TOPDIR) all
	$(Q)echo [LD] Linking $(subst $(OUTDIR)/,,$@)
	$(Q)$(CC) $(LDFLAGS) $(LIBDIRS) -Wl,--cref,-Map=$@.map -o $@ @$(OUTDIR)/.objects.lst $(LIBS)
	$(Q)echo [SZ] Sizing $(subst $(OUTDIR)/,,$@)
	$(Q)$(SZ) $@

$(PRJSDIR)/.kconfig.projects: $(PRJKCFGS)
	$(Q){ $(foreach d,$(patsubst $(PRJSDIR)/%/Kconfig,%,$(PRJKCFGS)),echo 'if PROJECT_NAME = "$(d)"'; echo 'source "projects/$(d)/Kconfig"'; echo 'endif'; echo '';) } > $@

%.defconfig: $(PRJSDIR)/.kconfig.projects
	$(Q)echo [CFG] Configuring $@
	$(Q)$(CP) $(TOPDIR)/projects/$@ $(TOPDIR)/.config
	$(Q)$(CONF) --syncconfig Kconfig

menuconfig: $(PRJSDIR)/.kconfig.projects
	$(Q)$(MCONF) $(TOPDIR)/Kconfig
	$(Q)$(CONF) --syncconfig Kconfig

clean:
	$(Q)$(MAKE) --no-print-directory -f Makefile.rules dir=$(TOPDIR) clean
	$(Q)$(RM) $(OUTDIR)

distclean:
	$(Q)$(RM) $(OUTDIR)
	$(Q)$(RM) $(TOPDIR)/.config
	$(Q)$(RM) $(TOPDIR)/.config.old
	$(Q)$(RM) $(TOPDIR)/.config.h
	$(Q)$(RM) $(PRJSDIR)/.kconfig.projects

help:
	@echo 'usage:'
	@echo '  defconfig  - New config with default from project supplied defconfig'
	@echo '  menuconfig - Update current config utilising a menu based program'
	@echo '  clean      - Remove most generated files but keep the config'
	@echo '  distclean  - Remove all generated files files'

sinclude $(wildcard *.d)
