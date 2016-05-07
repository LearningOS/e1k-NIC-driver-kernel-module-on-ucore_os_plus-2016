#include <autoconf.h>
#include <linux/device.h>
#include <linux/printk.h>


#define DDE_WEAK __attribute__((weak))

#define dde_dummy_printf(...)
#define dde_printf(...) printf(__VA_ARGS__)

/*
 */
DDE_WEAK int bus_register(struct bus_type * a) {
	dde_printf("bus_register not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK int driver_register(struct device_driver * a) {
	dde_printf("driver_register not implemented\n");
	return 0;
}

/*
 */
DDE_WEAK int printk(const char * a, ...) {
	dde_printf("printk not implemented\n");
	return 0;
}

