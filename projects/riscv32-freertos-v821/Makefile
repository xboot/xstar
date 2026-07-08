#
# Project Makefile
#

XSTAR_DIR		:= $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST))))/../..)
XSTAR_DEFCONFIG	:= $(notdir $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))/xstar.defconfig

.PHONY: all defconfig menuconfig clean distclean

all:
	@$(MAKE) -s -C $(XSTAR_DIR) all

defconfig:
	@$(MAKE) -s -C $(XSTAR_DIR) $(XSTAR_DEFCONFIG)

menuconfig:
	@$(MAKE) -s -C $(XSTAR_DIR) menuconfig

clean:
	@$(MAKE) -s -C $(XSTAR_DIR) clean

distclean:
	@$(MAKE) -s -C $(XSTAR_DIR) distclean
