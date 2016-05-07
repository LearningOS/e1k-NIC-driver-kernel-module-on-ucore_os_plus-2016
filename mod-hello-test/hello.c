/*  Kernel Programming */
#include <autoconf.h>
#include <linux/module.h>	// Needed by all modules
#include <linux/kernel.h>	// Needed for KERN_ALERT
#include <linux/init.h>		// Needed for the macros
#include <linux/device.h>
#include <linux/kobject.h>

#define KERN_ALERT ""

extern int printf(const char* a, ...);

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
  printf(KERN_ALERT "bus register");
  return bus_register(&test_bus_type);
}

static int test_driver_register(struct test_device_driver *test_driver) {
  test_driver->driver.name = test_driver->name;
  test_driver->driver.bus = &test_bus_type;
  return driver_register(&test_driver->driver);
}

static int hello_init(void) {
  int ret = -1;
  printf(KERN_ALERT "Hello, world\n");
  ret = test_bus_init();
  if (ret) {
    printf(KERN_ALERT "REG ERR\n");
  } else {
    printf(KERN_ALERT "REG OK\n");
  }
  ret = test_driver_register(&test_driver);
  if (ret)
    printf(KERN_ALERT "REG ERR\n");
  else
    printf(KERN_ALERT "REG OK\n");
  return 0;
}

static void hello_exit(void) {
  printf(KERN_ALERT "Goodbye, world\n");
}

module_init(hello_init);
module_exit(hello_exit);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
    .name = "mod-hello",
    .init = hello_init,
    .exit = hello_exit,
};


int main() {
  printf("This mod-hello test\n");
  struct module test_module;
  printf("Module size: %d\n", sizeof(test_module));
  printf("list: %d\n", (int)&(test_module.list) - (int)&(test_module));
  printf("name: %d\n", (int)&(test_module.name) - (int)&(test_module));
  printf("kernel_symbol: %d\n", (int)&(test_module.syms) - (int)&(test_module));
  printf("init: %d\n", (int)&(test_module.init) - (int)&(test_module));
  printf("module_init: %d\n", (int)&(test_module.module_init) - (int)&(test_module));
  printf("module_core: %d\n", (int)&(test_module.module_core) - (int)&(test_module));
  printf("symtab: %d\n", (int)&(test_module.symtab) - (int)&(test_module));
  printf("percpu: %d\n", (int)&(test_module.percpu) - (int)&(test_module));
  printf("exit: %d\n", (int)&(test_module.exit) - (int)&(test_module));
  return 0;
}



