#ifndef __ASM_PGTABLE_64_TYPES_H__
#define __ASM_PGTABLE_64_TYPES_H__

#include <asm/sparsemem.h>

#include <linux/types.h>

/*
 * These are used to make use of C type-checking..
 */
typedef unsigned long	pteval_t;
typedef unsigned long	pmdval_t;
typedef unsigned long	pudval_t;
typedef unsigned long	pgdval_t;
typedef unsigned long	pgprotval_t;

typedef struct { pteval_t pte; } pte_t;

#define PAGETABLE_LEVELS	4

/*
 * PMD_SHIFT determines the size of the area a middle-level
 * page table can map
 */
#define PMD_SHIFT	21

/*
 * entries per page directory level
 */
#define PTRS_PER_PTE	512

#endif /* ! __ASM_PGTABLE_64_TYPES_H__ */
