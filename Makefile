XBE_TITLE = BORZOI2001
SRCS = $(CURDIR)/main.c
NXDK_SDL = y
NXDK_DIR ?= $(CURDIR)/../nxdk
include $(NXDK_DIR)/Makefile
