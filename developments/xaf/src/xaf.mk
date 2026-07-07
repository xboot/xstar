#
# xaf top directory
#
XAF_DIR = $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))

#
# xaf
#
INCDIRS		+=	$(XAF_DIR)
SRCDIRS		+=	$(XAF_DIR)
