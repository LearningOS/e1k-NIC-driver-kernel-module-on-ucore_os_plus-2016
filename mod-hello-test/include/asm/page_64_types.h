#ifndef __ASM_PAGE_64_TYPES_H__
#define __ASM_PAGE_64_TYPES_H__

#define THREAD_SIZE_ORDER	2
#define THREAD_SIZE  (PAGE_SIZE << THREAD_SIZE_ORDER)

#define IRQ_STACK_ORDER 2
#define IRQ_STACK_SIZE (PAGE_SIZE << IRQ_STACK_ORDER)

/*
 * Set __PAGE_OFFSET to the most negative possible address +
 * PGDIR_SIZE*16 (pgd slot 272).  The gap is to allow a space for a
 * hypervisor to fit.  Choosing 16 slots here is arbitrary, but it's
 * what Xen requires.
 */
#define __PAGE_OFFSET           _AC(0xffff880000000000, UL)

#define __START_KERNEL_map	_AC(0xffffffff80000000, UL)

#define __PHYSICAL_MASK_SHIFT	46

/*
 * Kernel image size is limited to 1GiB due to the fixmap living in the
 * next 1GiB (see level2_kernel_pgt in arch/x86/kernel/head_64.S). Use
 * 512MiB by default, leaving 1.5GiB for modules once the page tables
 * are fully set up. If kernel ASLR is configured, it can extend the
 * kernel page table mapping, reducing the size of the modules area.
 */
#define KERNEL_IMAGE_SIZE_DEFAULT      (512 * 1024 * 1024)

#endif /* ! __ASM_PAGE_64_TYPES_H__ */
