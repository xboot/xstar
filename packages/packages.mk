#
# Packages
#

ifdef CONFIG_PKG_LIBC
CFLAGS		+= -I $(PKGDIR)/libc-0.0.0/inc -I $(PKGDIR)/libc-0.0.0/inc/$(CONFIG_ARCH)
CXXFLAGS	+= -I $(PKGDIR)/libc-0.0.0/inc -I $(PKGDIR)/libc-0.0.0/inc/$(CONFIG_ARCH)
endif

ifdef CONFIG_PKG_LIBM
CFLAGS		+= -I $(PKGDIR)/libm-0.0.0/inc
CXXFLAGS	+= -I $(PKGDIR)/libm-0.0.0/inc
endif

ifdef CONFIG_PKG_LIBCJSON
CFLAGS		+= -I $(PKGDIR)/libcjson-0.0.0
CXXFLAGS	+= -I $(PKGDIR)/libcjson-0.0.0
endif

ifdef CONFIG_PKG_LIBXNES
CFLAGS		+= -I $(PKGDIR)/libxnes-1.0.0
CXXFLAGS	+= -I $(PKGDIR)/libxnes-1.0.0
endif

ifdef CONFIG_PKG_LVGL
CFLAGS		+= -I $(PKGDIR)/lvgl-9.4.0
CXXFLAGS	+= -I $(PKGDIR)/lvgl-9.4.0
endif

ifdef CONFIG_PKG_PLMPEG
CFLAGS		+= -I $(PKGDIR)/plmpeg-0.0.0
CXXFLAGS	+= -I $(PKGDIR)/plmpeg-0.0.0
endif

ifdef CONFIG_PKG_VWW
CFLAGS		+= -I $(PKGDIR)/vww-0.0.0
CXXFLAGS	+= -I $(PKGDIR)/vww-0.0.0
endif

ifdef CONFIG_PKG_WBOXTEST
CFLAGS		+= -I $(PKGDIR)/wboxtest-0.0.0
CXXFLAGS	+= -I $(PKGDIR)/wboxtest-0.0.0
endif

ifdef CONFIG_PKG_XAF
CFLAGS		+= -I $(PKGDIR)/xaf-0.0.0
CXXFLAGS	+= -I $(PKGDIR)/xaf-0.0.0
endif

ifdef CONFIG_PKG_XUI
CFLAGS		+= -I $(PKGDIR)/xui-0.0.0
CXXFLAGS	+= -I $(PKGDIR)/xui-0.0.0
endif
