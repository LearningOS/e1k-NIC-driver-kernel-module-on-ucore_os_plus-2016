#include <types.h>
#include <arch.h>
#include <picirq.h>
#include <trap.h>
#include <error.h>
#include <proc.h>

// I/O Addresses of the two programmable interrupt controllers
#define IO_PIC1             0x20	// Master (IRQs 0-7)
#define IO_PIC2             0xA0	// Slave (IRQs 8-15)

#define IRQ_SLAVE           2	// IRQ at which slave connects to master

//#define NR_IRQS             129
#define NR_IRQS             256

// Current IRQ mask.
// Initial IRQ mask has interrupt 2 enabled (for slave 8259A).
static uint16_t irq_mask = 0xFFFF & ~(1 << IRQ_SLAVE);
static bool did_init = 0;

struct trapframe* irq_tf;

struct irq_action {
  ucore_irq_handler_t handler;
  void *opaque;
};

struct irq_action actions[NR_IRQS];

static void pic_setmask(uint16_t mask) {
	irq_mask = mask;
	if (did_init) {
		outb(IO_PIC1 + 1, mask);
		outb(IO_PIC2 + 1, mask >> 8);
	}
}

void pic_enable(unsigned int irq) {
	pic_setmask(irq_mask & ~(1 << irq));
}

void pic_disable(unsigned int irq) {
  pic_setmask(irq_mask | (1 << irq));
}

/* pic_init - initialize the 8259A interrupt controllers */
void pic_init(void) {
	did_init = 1;

	// mask all interrupts
	outb(IO_PIC1 + 1, 0xFF);
	outb(IO_PIC2 + 1, 0xFF);

	// Set up master (8259A-1)

	// ICW1:  0001g0hi
	//    g:  0 = edge triggering, 1 = level triggering
	//    h:  0 = cascaded PICs, 1 = master only
	//    i:  0 = no ICW4, 1 = ICW4 required
	outb(IO_PIC1, 0x11);

	// ICW2:  Vector offset
	outb(IO_PIC1 + 1, IRQ_OFFSET);

	// ICW3:  (master PIC) bit mask of IR lines connected to slaves
	//        (slave PIC) 3-bit # of slave's connection to master
	outb(IO_PIC1 + 1, 1 << IRQ_SLAVE);

	// ICW4:  000nbmap
	//    n:  1 = special fully nested mode
	//    b:  1 = buffered mode
	//    m:  0 = slave PIC, 1 = master PIC
	//        (ignored when b is 0, as the master/slave role
	//         can be hardwired).
	//    a:  1 = Automatic EOI mode
	//    p:  0 = MCS-80/85 mode, 1 = intel x86 mode
	outb(IO_PIC1 + 1, 0x3);

	// Set up slave (8259A-2)
	outb(IO_PIC2, 0x11);	// ICW1
	outb(IO_PIC2 + 1, IRQ_OFFSET + 8);	// ICW2
	outb(IO_PIC2 + 1, IRQ_SLAVE);	// ICW3
	// NB Automatic EOI mode doesn't tend to work on the slave.
	// Linux source code says it's "to be investigated".
	outb(IO_PIC2 + 1, 0x3);	// ICW4

	// OCW3:  0ef01prs
	//   ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
	//    p:  0 = no polling, 1 = polling mode
	//   rs:  0x = NOP, 10 = read IRR, 11 = read ISR
	outb(IO_PIC1, 0x68);	// clear specific mask
	outb(IO_PIC1, 0x0a);	// read IRR by default

	outb(IO_PIC2, 0x68);	// OCW3
	outb(IO_PIC2, 0x0a);	// OCW3

	if (irq_mask != 0xFFFF) {
		pic_setmask(irq_mask);
	}
}

void irq_clear(unsigned int irq) {
  actions[irq].handler = NULL;
  actions[irq].opaque = NULL;
}

void irq_handler() {
  int irq = irq_tf->tf_trapno;
  if (irq >= 0 && irq < NR_IRQS && actions[irq].handler) {
    (*actions[irq].handler)(irq, actions[irq].opaque);
  } else {
		print_trapframe(irq_tf);
		if (current != NULL) {
			kprintf("unhandled trap 0x%02x(%d).\n", irq, irq);
			do_exit(-E_KILLED);
		}
		panic("unexpected trap in kernel.\n");
  }
}

void register_irq(int irq, ucore_irq_handler_t handler, void *opaque) {
  if (irq >= 0 && irq < NR_IRQS) {
    actions[irq].handler = handler;
    actions[irq].opaque = opaque;
  } else {
    kprintf("WARNING: register_irq: invalid irq 0x%02x(%d)\n", irq, irq);
  }
}
