#include <types.h>
#include <stdio.h>
#include <string.h>
#include <console.h>
#include <kdebug.h>
#include <picirq.h>
#include <trap.h>
#include <clock.h>
#include <intr.h>
#include <pmm.h>
#include <vmm.h>
#include <ide.h>
#include <fs.h>
#include <swap.h>
#include <proc.h>
#include <sched.h>
#include <kio.h>
#include <mp.h>
#include <mod.h>

int kern_init(void) __attribute__ ((noreturn));

int kern_init(void)
{
	extern char edata[], end[];
	memset(edata, 0, end - edata);

	cons_init();		// init the console

	const char *message = "(THU.CST) os is loading ...";
	kprintf("%s\n\n", message);

	print_kerninfo();

	/* Only to initialize lcpu_count. */
	mp_init();

	debug_init();		// init debug registers
	pmm_init();		// init physical memory management
	pmm_init_ap();
    lapic_init();

	pic_init();		// init interrupt controller
	idt_init();		// init interrupt descriptor table
    ioapic_init();

	vmm_init();		// init virtual memory management
	sched_init();		// init scheduler
	proc_init();		// init process table
	sync_init();		// init sync struct

	//ide_init();		// init ide devices
	ide_init();
    //check_initrd();
    //ramdisk_init();
#ifdef UCONFIG_SWAP
	swap_init();		// init swap
#endif
	fs_init();		// init fs

	//clock_init();		// init clock interrupt
	clock_init();		// init clock interrupt

	mod_init();

    pci_init();

	intr_enable();		// enable irq interrupt

    ioapicenable(IRQ_KBD, 0);
    ioapicenable(IRQ_COM1, 0);

    enable_e1000();
    test_transmission();

    init_lwip_dev();
	/* do nothing */
	cpu_idle();		// run idle process
}
