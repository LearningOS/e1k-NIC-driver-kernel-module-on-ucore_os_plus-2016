#include <kio.h>
#include <types.h>
#include <mod.h>

#include "fifo_swap.h"
#include "swap_manager.h"

extern struct swap_manager *  def_swap_manager;
extern struct swap_manager  fifo_swap_manager;
static __init int swap_init() {
    kprintf("checkout to fifo swap manager\n");
    def_swap_manager = &(fifo_swap_manager);
    return 0;
}
	
static __exit void swap_exit() {
    kprintf("swap_exit: Goodbye, cruel world.\n");
}
	
module_init(swap_init);
module_exit(swap_exit);
	
struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
    .name = "mod-swap",
    .init = swap_init,
    .exit = swap_exit,
};
	
static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
	"depends=";
