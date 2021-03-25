XBE_TITLE = nxdk_xbe_launch
GEN_XISO = $(XBE_TITLE).iso
SRCS = $(CURDIR)/main.c
NXDK_DIR = $(CURDIR)/externals/nxdk
NXDK_SDL = y

include $(NXDK_DIR)/Makefile
