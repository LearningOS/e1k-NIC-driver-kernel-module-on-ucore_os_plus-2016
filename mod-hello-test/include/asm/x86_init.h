#ifndef __ASM_X86_INIT_H__
#define __ASM_X86_INIT_H__

#include <asm/pgtable_types.h>
#include <asm/bootparam.h>

struct mpc_bus;
struct mpc_cpu;
struct mpc_table;
struct cpuinfo_x86;

/**
 * struct x86_init_mpparse - platform specific mpparse ops
 * @mpc_record:			platform specific mpc record accounting
 * @setup_ioapic_ids:		platform specific ioapic id override
 * @mpc_apic_id:		platform specific mpc apic id assignment
 * @smp_read_mpc_oem:		platform specific oem mpc table setup
 * @mpc_oem_pci_bus:		platform specific pci bus setup (default NULL)
 * @mpc_oem_bus_info:		platform specific mpc bus info
 * @find_smp_config:		find the smp configuration
 * @get_smp_config:		get the smp configuration
 */
struct x86_init_mpparse {
	void (*mpc_record)(unsigned int mode);
	void (*setup_ioapic_ids)(void);
	int (*mpc_apic_id)(struct mpc_cpu *m);
	void (*smp_read_mpc_oem)(struct mpc_table *mpc);
	void (*mpc_oem_pci_bus)(struct mpc_bus *m);
	void (*mpc_oem_bus_info)(struct mpc_bus *m, char *name);
	void (*find_smp_config)(void);
	void (*get_smp_config)(unsigned int early);
};

/**
 * struct x86_init_resources - platform specific resource related ops
 * @probe_roms:			probe BIOS roms
 * @reserve_resources:		reserve the standard resources for the
 *				platform
 * @memory_setup:		platform specific memory setup
 *
 */
struct x86_init_resources {
	void (*probe_roms)(void);
	void (*reserve_resources)(void);
	char *(*memory_setup)(void);
};

/**
 * struct x86_init_irqs - platform specific interrupt setup
 * @pre_vector_init:		init code to run before interrupt vectors
 *				are set up.
 * @intr_init:			interrupt init code
 * @trap_init:			platform specific trap setup
 */
struct x86_init_irqs {
	void (*pre_vector_init)(void);
	void (*intr_init)(void);
	void (*trap_init)(void);
};

/**
 * struct x86_init_oem - oem platform specific customizing functions
 * @arch_setup:			platform specific architecure setup
 * @banner:			print a platform specific banner
 */
struct x86_init_oem {
	void (*arch_setup)(void);
	void (*banner)(void);
};

/**
 * struct x86_init_paging - platform specific paging functions
 * @pagetable_init:	platform specific paging initialization call to setup
 *			the kernel pagetables and prepare accessors functions.
 *			Callback must call paging_init(). Called once after the
 *			direct mapping for phys memory is available.
 */
struct x86_init_paging {
	void (*pagetable_init)(void);
};

/**
 * struct x86_init_timers - platform specific timer setup
 * @setup_perpcu_clockev:	set up the per cpu clock event device for the
 *				boot cpu
 * @tsc_pre_init:		platform function called before TSC init
 * @timer_init:			initialize the platform timer (default PIT/HPET)
 * @wallclock_init:		init the wallclock device
 */
struct x86_init_timers {
	void (*setup_percpu_clockev)(void);
	void (*tsc_pre_init)(void);
	void (*timer_init)(void);
	void (*wallclock_init)(void);
};

/**
 * struct x86_init_iommu - platform specific iommu setup
 * @iommu_init:			platform specific iommu setup
 */
struct x86_init_iommu {
	int (*iommu_init)(void);
};

/**
 * struct x86_init_pci - platform specific pci init functions
 * @arch_init:			platform specific pci arch init call
 * @init:			platform specific pci subsystem init
 * @init_irq:			platform specific pci irq init
 * @fixup_irqs:			platform specific pci irq fixup
 */
struct x86_init_pci {
	int (*arch_init)(void);
	int (*init)(void);
	void (*init_irq)(void);
	void (*fixup_irqs)(void);
};

/**
 * struct x86_init_ops - functions for platform specific setup
 *
 */
struct x86_init_ops {
	struct x86_init_resources	resources;
	struct x86_init_mpparse		mpparse;
	struct x86_init_irqs		irqs;
	struct x86_init_oem		oem;
	struct x86_init_paging		paging;
	struct x86_init_timers		timers;
	struct x86_init_iommu		iommu;
	struct x86_init_pci		pci;
};

struct timespec;

struct IO_APIC_route_entry;
struct io_apic_irq_attr;
struct irq_data;
struct cpumask;

struct x86_io_apic_ops {
	void		(*init)   (void);
	unsigned int	(*read)   (unsigned int apic, unsigned int reg);
	void		(*write)  (unsigned int apic, unsigned int reg, unsigned int value);
	void		(*modify) (unsigned int apic, unsigned int reg, unsigned int value);
	void		(*disable)(void);
	void		(*print_entries)(unsigned int apic, unsigned int nr_entries);
	int		(*set_affinity)(struct irq_data *data,
					const struct cpumask *mask,
					bool force);
	int		(*setup_entry)(int irq, struct IO_APIC_route_entry *entry,
				       unsigned int destination, int vector,
				       struct io_apic_irq_attr *attr);
	void		(*eoi_ioapic_pin)(int apic, int pin, int vector);
};

extern struct x86_init_ops x86_init;

extern struct x86_io_apic_ops x86_io_apic_ops;

#endif /* ! __ASM_X86_INIT_H__ */
