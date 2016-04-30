TARGET_CFLAGS += -fno-common -D__LINUX_X86_ARCH__ -D'KBUILD_BASENAME="dde"' -D'KBUILD_MODNAME="dde"' -include linux/kconfig.h
#e1000-objs :=  e1000_main.o e1000_hw.o e1000_ethtool.o e1000_param.o e1000.dummy.o e1000_dde.o dev_addr_lists.o skbuff.o
obj-m := mod-e1000.o
obj-mod-e1000 := e1000.o e1000_hw.o e1000_ethtool.o e1000_param.o e1000.dummy.o e1000_dde.o dev_addr_lists.o skbuff.o
dirs-y += pci

