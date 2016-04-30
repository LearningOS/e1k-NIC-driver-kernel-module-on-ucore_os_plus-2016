#include <kio.h>
#include <mod.h>
#include <string.h>
//extern int test1(void);

int test2_init(void)
{
   // printk("get from test1(): %d\n", test1());
	kprintf("This mod-hello test\n");
	struct module test_module;
	kprintf("Module size: %d\n", sizeof(test_module));
	kprintf("list: %d\n", (int)&(test_module.list) - (int)&(test_module));
	kprintf("name: %d\n", (int)&(test_module.name) - (int)&(test_module));
	kprintf("kernel_symbol: %d\n", (int)&(test_module.syms) - (int)&(test_module));
	kprintf("init: %d\n", (int)&(test_module.init) - (int)&(test_module));
	kprintf("module_init: %d\n", (int)&(test_module.module_init) - (int)&(test_module));
	kprintf("module_core: %d\n", (int)&(test_module.module_core) - (int)&(test_module));
	kprintf("symtab: %d\n", (int)&(test_module.symtab) - (int)&(test_module));
	kprintf("percpu: %d\n", (int)&(test_module.percpu) - (int)&(test_module));
	kprintf("exit: %d\n", (int)&(test_module.exit) - (int)&(test_module));


    return 0;
}


static __exit void test2_exit() {	
}

module_init(test2_init);
module_exit(test2_exit);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
    .name = "mod-test2",
    .init = test2_init,
    .exit = test2_exit,
};
