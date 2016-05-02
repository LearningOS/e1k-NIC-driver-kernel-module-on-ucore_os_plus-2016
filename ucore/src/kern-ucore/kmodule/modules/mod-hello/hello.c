/*  Kernel Programming */
#include <linux/kconfig.h>
#include <linux/module.h>	// Needed by all modules
#include <linux/kernel.h>	// Needed for KERN_ALERT
#include <linux/init.h>		// Needed for the macros
#include <linux/device.h>
#include <linux/kobject.h>
#include <linux/types.h>

#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>


struct test_device_driver {
  char *name;
  struct device_driver driver;
};

static struct test_device_driver test_driver = {
    .name = "test_driver",
};

static int test_bus_match(struct device *dev, struct device_driver *drv) {
  return 1;
}

struct bus_type test_bus_type = {
    .name = "test_bus",
    .match = test_bus_match,
};

static int __init test_bus_init() {
  printk("bus register");
  return bus_register(&test_bus_type);
}

static int test_driver_register(struct test_device_driver *test_driver) {
  test_driver->driver.name = test_driver->name;
  test_driver->driver.bus = &test_bus_type;
  return driver_register(&test_driver->driver);
}

struct pci_func {
    struct pci_bus *bus;	// Primary bus for bridges

    uint32_t dev;
    uint32_t func;

    uint32_t dev_id;
    uint32_t dev_class;

    uint32_t reg_base[6];
    uint32_t reg_size[6];
    uint8_t irq_line;
};

struct pci_func *pcif_handler;
void test_e1000()
{
	e1000_pcif_get(pcif_handler);//get
}
static int hello_init(void) {
  int ret = -1;
  printk("Hello, world\n");
  kprintf("This is hello_init\n");
  ret = test_bus_init();
  if (ret) {
    printk("REG ERR\n");
  } else {
    printk("REG OK\n");
  }
  ret = test_driver_register(&test_driver);
  if (ret)
    printk("REG ERR\n");
  else
    printk("REG OK\n");

	test_e1000();

  return 0;
}

static void hello_exit(void) {
  printk("Goodbye, world\n");
}

module_init(hello_init);
module_exit(hello_exit);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
    .name = "mod-hello",
    .init = hello_init,
    .exit = hello_exit,
};



