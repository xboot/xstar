#
# Project Makefile
#

export TINA_LINUX_DIR=/home/jiangjianjun/9tripod/v821-tina-v13
export STAGING_DIR=${TINA_LINUX_DIR}/out/v821/aitoy/openwrt/staging_dir
export CROSS_COMPILE=${TINA_LINUX_DIR}/prebuilt/rootfsbuilt/riscv/nds32le-linux-musl-v5d/bin/riscv32-linux-musl-

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
