#include <asm/checksum_64.h>
//#include <asm/dma-mapping.h>
#include <asm/io.h>
#include <asm/page_64.h>
#include <asm/smp.h>
#include <asm/string_64.h>
#include <asm/thread_info.h>
#include <asm-generic/bitops/find.h>
#include <asm-generic/bug.h>
#include <asm-generic/delay.h>
#include <asm-generic/iomap.h>
#include <linux/bottom_half.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dynamic_queue_limits.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/gfp.h>
#include <linux/hardirq.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/pci.h>
#include <linux/pm_wakeup.h>
#include <linux/printk.h>
#include <linux/rtnetlink.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/slub_def.h>
#include <linux/spinlock_api_smp.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>

#include <stdarg.h>

#define DDE_WEAK __attribute__((weak))

#define dde_dummy_printf(...) kprintf(__VA_ARGS__)
#define dde_printf(...) dde_dummy_printf(__VA_ARGS__)

void *ucore_kmalloc(size_t size);
#define PTR_ALIGN(p, a)         ((typeof(p))ALIGN((unsigned long)(p), (a)))


DDE_WEAK int _dev_info(const struct device * a, const char * fmt, ...) {
	va_list ap;
	int cnt;
	va_start(ap, fmt);
	cnt = vkprintf(fmt, ap);
	va_end(ap);
	return cnt;
}

DDE_WEAK int dev_err(const struct device * a, const char * b, ...) {
	dde_printf("dev_err not implemented\n");
	return 0;
}

//int netdev_err(const struct net_device * a, const char * fmt, ...) {
//    kprintf("netdev_err: ");
//	va_list ap;
//	int cnt;
//	va_start(ap, fmt);
//	cnt = vkprintf(fmt, ap);
//	va_end(ap);
//	return cnt;
//}


DDE_WEAK void pci_clear_mwi(struct pci_dev * a) {
	dde_printf("pci_clear_mwi not implemented\n");
}

/*
 */
DDE_WEAK void pci_disable_device(struct pci_dev * a) {
	dde_printf("pci_disable_device not implemented\n");
}

/*
 */
DDE_WEAK int pci_enable_device_mem(struct pci_dev * a) {
	dde_printf("pci_enable_device_mem not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK int __pci_enable_wake(struct pci_dev * a, pci_power_t b, bool c, bool d) {
	dde_printf("__pci_enable_wake not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK int pci_prepare_to_sleep(struct pci_dev * a) {
	dde_printf("pci_prepare_to_sleep not implemented\n");
	return 0;
}

/* Proper probing supporting hot-pluggable devices */
DDE_WEAK int __pci_register_driver(struct pci_driver * a, struct module * b, const char * c) {
	dde_printf("__pci_register_driver not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK void pci_release_selected_regions(struct pci_dev * a, int b) {
	dde_printf("pci_release_selected_regions not implemented\n");
}

/*
 */
DDE_WEAK int pci_request_selected_regions(struct pci_dev * a, int b, const char * c) {
	dde_printf("pci_request_selected_regions not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK void pci_restore_state(struct pci_dev * a) {
	dde_printf("pci_restore_state not implemented\n");
}

/* Power management related routines */
//DDE_WEAK int pci_save_state(struct pci_dev * a) {
//	dde_printf("pci_save_state not implemented\n");
//	return 0;
//}

/*
 */
int pci_select_bars(struct pci_dev * dev, unsigned long flags) {
    int i, bars = 0;
    for (i = 0; i < PCI_NUM_RESOURCES; i++)
        if (pci_resource_flags(dev, i) & flags)
            bars |= (1 << i);
    return bars;
}

/*
 */
void pci_set_master(struct pci_dev * dev) {
    u16 old_cmd, cmd;

    pci_read_config_word(dev, PCI_COMMAND, &old_cmd);
    cmd = old_cmd | PCI_COMMAND_MASTER;
    if (cmd != old_cmd) {
        kprintf("enabling bus mastering\n");
        pci_write_config_word(dev, PCI_COMMAND, cmd);
    }
    dev->is_busmaster = 1;
}

/*
 */
DDE_WEAK int pci_set_mwi(struct pci_dev * a) {
	dde_printf("pci_set_mwi not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK int pci_set_power_state(struct pci_dev * a, pci_power_t b) {
	dde_printf("pci_set_power_state not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK void pci_unregister_driver(struct pci_driver * a) {
	dde_printf("pci_unregister_driver not implemented\n");
}

/*
 */
DDE_WEAK int pci_wake_from_d3(struct pci_dev * a, bool b) {
	dde_printf("pci_wake_from_d3 not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK int pcix_get_mmrbc(struct pci_dev * a) {
	dde_printf("pcix_get_mmrbc not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK int pcix_set_mmrbc(struct pci_dev * a, int b) {
	dde_printf("pcix_set_mmrbc not implemented\n");
	return 0;
}

DDE_WEAK void warn_slowpath_null(const char * a, const int b) {
	dde_printf("warn_slowpath_null not implemented\n");
}

DDE_WEAK void * ioremap_nocache(resource_size_t a, unsigned long b) {
    void *ucore_ioremap(uint32_t pa, uint32_t size);
    void *ret = ucore_ioremap(a, b);
    kprintf("ioremap %x %x to %x\n", (uint32_t)a, (uint32_t)b, (uint32_t)ret);
    return ret;
}



