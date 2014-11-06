#include "fifo_swap.h"
#include <vmm.h>
#include <pmm.h>
#include <types.h>
#include "swap_manager.h"

size_t max_swap_offset;
// the list of pages for swap
typedef struct {
    list_entry_t swap_list;
    size_t nr_pages;
} swap_list_t;

// there are two swap list: active_list & inactive_list
// active page list. Items in active_list may move to inactive_list.
swap_list_t active_list;
// inactive page list, will be evicted if ucore need more free page frames
swap_list_t inactive_list;

#define nr_active_pages                 (active_list.nr_pages)
#define nr_inactive_pages               (inactive_list.nr_pages)

// the array element is used to record the offset of swap entry
// the value of array element is the reference number of swap out page
// page->ref+mem_map[offset]= the total reference number of a page(in mem OR swap space)
// the index of array element is the offset of swap space(disk)
extern unsigned short *mem_map;
extern list_entry_t swap_hash_list[HASH_LIST_SIZE];
static volatile bool swap_init_ok = 0;

static void check_swap(void);
void check_mm_swap(void);
void check_mm_shm_swap(void);

static volatile int pressure = 0;
#include "swap_manager.h"

// swap_list_init - initialize the swap list
static void swap_list_init(swap_list_t * list)
{
    list_init(&(list->swap_list));
    list->nr_pages = 0;
}

// swap_active_list_add - add the page to active_list
static inline void swap_active_list_add(struct Page *page)
{
    assert(PageSwap(page));
    SetPageActive(page);
    swap_list_t *list = &active_list;
    list->nr_pages++;
    list_add_before(&(list->swap_list), &(page->swap_link));
}

// swap_inactive_list_add - add the page to inactive_list
static inline void swap_inactive_list_add(struct Page *page)
{
    assert(PageSwap(page));
    ClearPageActive(page);
    swap_list_t *list = &inactive_list;
    list->nr_pages++;
    list_add_before(&(list->swap_list), &(page->swap_link));
}

// nur_swap_list_del - delete page from the swap list
void nur_swap_list_del(struct Page *page)
{
    assert(PageSwap(page));
    (PageActive(page) ? &active_list : &inactive_list)->nr_pages--;
    list_del(&(page->swap_link));
}

// swap_init - init swap fs, two swap lists, alloc memory & init for swap_entry record array mem_map
//           - init the hash list.
void swap_init(void)
{
    swap_list_init(&active_list);
    swap_list_init(&inactive_list);
    
    //check_mm_swap();
    check_mm_shm_swap();
    swap_init_ok = 1;
}

// nur_try_free_pages - calculate pressure to estimate the number(pressure<<5) of needed page frames in ucore currently, 
//                - then call kswapd kernel thread.
bool nur_try_free_pages(size_t n)
{
    if (!swap_init_ok || kswapd == NULL) {
        return 0;
    }
    if (current == kswapd) {
        panic("kswapd call nur_try_free_pages!!.\n");
    }
    if (n >= (1 << 7)) {
        return 0;
    }
    pressure += n;

    wait_t __wait, *wait = &__wait;

    bool intr_flag;
    local_intr_save(intr_flag);
    {
        wait_init(wait, current);
        current->state = PROC_SLEEPING;
        current->wait_state = WT_KSWAPD;
        wait_queue_add(&kswapd_done, wait);
        if (kswapd->wait_state == WT_TIMER) {
            wakeup_proc(kswapd);
        }
    }
    local_intr_restore(intr_flag);

    schedule();

    assert(!wait_in_queue(wait) && wait->wakeup_flags == WT_KSWAPD);
    return 1;
}


// nur_swap_copy_entry - copy a content of swap out page frame to a new page
//                 - set this new page PG_swap flag and add to swap active list
int nur_swap_copy_entry(swap_entry_t entry, swap_entry_t * store)
{
    if (store == NULL) {
        return -E_INVAL;
    }

    int ret = -E_NO_MEM;
    struct Page *page, *newpage;
    swap_duplicate(entry);
    if ((newpage = alloc_page()) == NULL) {
        goto failed;
    }
    if ((ret = nur_swap_in_page(entry, &page)) != 0) {
        goto failed_free_page;
    }
    ret = -E_NO_MEM;
    if (!swap_page_add(newpage, 0)) {
        goto failed_free_page;
    }
    swap_active_list_add(newpage);
    memcpy(page2kva(newpage), page2kva(page), PGSIZE);
    *store = newpage->index;
    ret = 0;
out:
    nur_swap_remove_entry(entry);
    return ret;

failed_free_page:
    free_page(newpage);
failed:
    goto out;
}

// nur_swap_in_page - swap in a content of a page frame from swap space to memory
//              - set the PG_swap flag in this page and add this page to swap active list
int nur_swap_in_page(swap_entry_t entry, struct Page **pagep)
{
    if (pagep == NULL) {
        return -E_INVAL;
    }
    size_t offset = swap_offset(entry);
    assert(mem_map[offset] >= 0);

    int ret;
    struct Page *page, *newpage;
    if ((page = swap_hash_find(entry)) != NULL) {
        goto found;
    }

    newpage = alloc_page();

    down(&swap_in_sem);
    if ((page = swap_hash_find(entry)) != NULL) {
        if (newpage != NULL) {
            free_page(newpage);
        }
        goto found_unlock;
    }
    if (newpage == NULL) {
        ret = -E_NO_MEM;
        goto failed_unlock;
    }
    page = newpage;
    if (swapfs_read(entry, page) != 0) {
        free_page(page);
        ret = -E_SWAP_FAULT;
        goto failed_unlock;
    }
    swap_page_add(page, entry);
    swap_active_list_add(page);

found_unlock:
    up(&swap_in_sem);
found:
    *pagep = page;
    return 0;

failed_unlock:
    up(&swap_in_sem);
    return ret;
}


// nur_swap_remove_entry - call nur_swap_list_del to remove page from swap hash list,
//                   - and call swap_free_page to generate a free page 
void nur_swap_remove_entry(swap_entry_t entry)
{
    size_t offset = swap_offset(entry);
    assert(mem_map[offset] > 0);
    if (--mem_map[offset] == 0) {
        struct Page *page = swap_hash_find(entry);
        if (page != NULL) {
            if (page_ref(page) != 0) {
                return;
            }
            nur_swap_list_del(page);
            swap_free_page(page);
        }
        mem_map[offset] = SWAP_UNUSED;
    }
}

// page_launder - try to move page to swap_active_list OR swap_inactive_list, 
//              - and call swap_fs_write to swap out pages in swap_inactive_list
int page_launder(void)
{
    size_t maxscan = nr_inactive_pages, free_count = 0;
    list_entry_t *list = &(inactive_list.swap_list), *le = list_next(list);
    while (maxscan-- > 0 && le != list) {
        struct Page *page = le2page(le, swap_link);
        le = list_next(le);
        if (!(PageSwap(page) && !PageActive(page))) {
            panic("inactive: wrong swap list.\n");
        }
        nur_swap_list_del(page);
        if (page_ref(page) != 0) {
            swap_active_list_add(page);
            continue;
        }
        swap_entry_t entry = page->index;
        if (!try_free_swap_entry(entry)) {
            if (PageDirty(page)) {
                ClearPageDirty(page);
                swap_duplicate(entry);
                if (swapfs_write(entry, page) != 0) {
                    SetPageDirty(page);
                }
                mem_map[swap_offset(entry)]--;
                if (page_ref(page) != 0) {
                    swap_active_list_add(page);
                    continue;
                }
                if (PageDirty(page)) {
                    swap_inactive_list_add(page);
                    continue;
                }
                try_free_swap_entry(entry);
            }
        }
        free_count++;
        swap_free_page(page);
    }
    return free_count;
}

// refill_inactive_scan - try to move page in swap_active_list into swap_inactive_list
void refill_inactive_scan(void)
{
    size_t maxscan = nr_active_pages;
    list_entry_t *list = &(active_list.swap_list), *le = list_next(list);
    while (maxscan-- > 0 && le != list) {
        struct Page *page = le2page(le, swap_link);
        le = list_next(le);
        if (!(PageSwap(page) && PageActive(page))) {
            panic("active: wrong swap list.\n");
        }
        if (page_ref(page) == 0) {
            nur_swap_list_del(page);
            swap_inactive_list_add(page);
        }
    }
}

// swap_out_vma - try unmap pte & move pages into swap active list.
static int
swap_out_vma(struct mm_struct *mm, struct vma_struct *vma, uintptr_t addr,
         size_t require)
{
    if (require == 0 || !(addr >= vma->vm_start && addr < vma->vm_end)) {
        return 0;
    }
    uintptr_t end;
    size_t free_count = 0;
    addr = ROUNDDOWN(addr, PGSIZE), end = ROUNDUP(vma->vm_end, PGSIZE);
    while (addr < end && require != 0) {
        pte_t *ptep = get_pte(mm->pgdir, addr, 0);
        if (ptep == NULL) {
            if (get_pud(mm->pgdir, addr, 0) == NULL) {
                addr = ROUNDDOWN(addr + PUSIZE, PUSIZE);
            } else if (get_pmd(mm->pgdir, addr, 0) == NULL) {
                addr = ROUNDDOWN(addr + PMSIZE, PMSIZE);
            } else {
                addr = ROUNDDOWN(addr + PTSIZE, PTSIZE);
            }
            continue;
        }
        if (ptep_present(ptep)) {
            struct Page *page = pte2page(*ptep);
            assert(!PageReserved(page));
            if (ptep_accessed(ptep)) {
                ptep_unset_accessed(ptep);
                mp_tlb_invalidate(mm->pgdir, addr);
                goto try_next_entry;
            }
            if (!PageSwap(page)) {
                if (!swap_page_add(page, 0)) {
                    goto try_next_entry;
                }
                swap_active_list_add(page);
            } else if (ptep_dirty(ptep)) {
                SetPageDirty(page);
            }
            swap_entry_t entry = page->index;
            swap_duplicate(entry);
            page_ref_dec(page);
            ptep_copy(ptep, &entry);
            mp_tlb_invalidate(mm->pgdir, addr);
            mm->swap_address = addr + PGSIZE;
            free_count++, require--;
            if ((vma->vm_flags & VM_SHARE) && page_ref(page) == 1) {
                uintptr_t shmem_addr =
                    addr - vma->vm_start + vma->shmem_off;
                pte_t *sh_ptep =
                    shmem_get_entry(vma->shmem, shmem_addr, 0);
                assert(sh_ptep != NULL
                       && !ptep_invalid(sh_ptep));
                if (ptep_present(sh_ptep)) {
                    shmem_insert_entry(vma->shmem,
                               shmem_addr, entry);
                }
            }
        }
try_next_entry:
        addr += PGSIZE;
    }
    return free_count;
}

// swap_out_mm - call swap_out_vma to try to unmap a set of vma ('require' NUM pages).
int swap_out_mm(struct mm_struct *mm, size_t require)
{
    assert(mm != NULL);
    if (require == 0 || mm->map_count == 0) {
        return 0;
    }
    assert(!list_empty(&(mm->mmap_list)));

    uintptr_t addr = mm->swap_address;
    struct vma_struct *vma;

    if ((vma = find_vma(mm, addr)) == NULL) {
        addr = mm->swap_address = 0;
        vma = le2vma(list_next(&(mm->mmap_list)), list_link);
    }
    assert(vma != NULL && addr <= vma->vm_end);

    if (addr < vma->vm_start) {
        addr = vma->vm_start;
    }

    int i;
    size_t free_count = 0;
    for (i = 0; i <= mm->map_count; i++) {
        int ret = swap_out_vma(mm, vma, addr, require);
        free_count += ret, require -= ret;
        if (require == 0) {
            break;
        }
        list_entry_t *le = list_next(&(vma->list_link));
        if (le == &(mm->mmap_list)) {
            le = list_next(le);
        }
        vma = le2vma(le, list_link);
        addr = vma->vm_start;
    }
    return free_count;
}

int kswapd_main(void *arg)
{
    int guard = 0;
    while (1) {
        if (pressure > 0) {
            int needs = (pressure << 5), rounds = 16;
            list_entry_t *list = &proc_mm_list;
            assert(!list_empty(list));
            while (needs > 0 && rounds-- > 0) {
                list_entry_t *le = list_next(list);
                list_del(le);
                list_add_before(list, le);
                struct mm_struct *mm = le2mm(le, proc_mm_link);
                needs -=
                    swap_out_mm(mm, (needs < 32) ? needs : 32);
            }
        }
        pressure -= page_launder();
        refill_inactive_scan();
        if (pressure > 0) {
            if ((++guard) >= 1000) {
                guard = 0;
                warn("kswapd: may out of memory");
            }
            continue;
        }
        pressure = 0, guard = 0;
        kswapd_wakeup_all();
        do_sleep(1000);
    }
}

// check_swap - check the correctness of swap & page replacement algorithm
static void check_swap(void)
{
    size_t nr_used_pages_store = nr_used_pages();
    size_t slab_allocated_store = slab_allocated();

    size_t offset;
    for (offset = 2; offset < max_swap_offset; offset++) {
        mem_map[offset] = 1;
    }

    struct mm_struct *mm = mm_create();
    assert(mm != NULL);

    extern struct mm_struct *check_mm_struct;
    assert(check_mm_struct == NULL);

    check_mm_struct = mm;

    pgd_t *pgdir = mm->pgdir = init_pgdir_get();
    assert(pgdir[PGX(TEST_PAGE)] == 0);

    struct vma_struct *vma =
        vma_create(TEST_PAGE, TEST_PAGE + PTSIZE, VM_WRITE | VM_READ);
    assert(vma != NULL);

    insert_vma_struct(mm, vma);

    struct Page *rp0 = alloc_page(), *rp1 = alloc_page();
    assert(rp0 != NULL && rp1 != NULL);

    pte_perm_t perm;
    ptep_unmap(&perm);
    ptep_set_u_write(&perm);
    int ret = page_insert(pgdir, rp1, TEST_PAGE, perm);
    assert(ret == 0 && page_ref(rp1) == 1);

    page_ref_inc(rp1);
    ret = page_insert(pgdir, rp0, TEST_PAGE, perm);
    assert(ret == 0 && page_ref(rp1) == 1 && page_ref(rp0) == 1);

    // check try_alloc_swap_entry

    swap_entry_t entry = try_alloc_swap_entry();
    assert(swap_offset(entry) == 1);
    mem_map[1] = 1;
    assert(try_alloc_swap_entry() == 0);

    // set rp1, Swap, Active, add to swap_hash_list, active_list

    swap_page_add(rp1, entry);
    swap_active_list_add(rp1);
    assert(PageSwap(rp1));

    mem_map[1] = 0;
    entry = try_alloc_swap_entry();
    assert(swap_offset(entry) == 1);
    assert(!PageSwap(rp1));

    // check nur_swap_remove_entry

    assert(swap_hash_find(entry) == NULL);
    mem_map[1] = 2;
    nur_swap_remove_entry(entry);
    assert(mem_map[1] == 1);

    swap_page_add(rp1, entry);
    swap_inactive_list_add(rp1);
    nur_swap_remove_entry(entry);
    assert(PageSwap(rp1));
    assert(rp1->index == entry && mem_map[1] == 0);

    // check page_launder, move page from inactive_list to active_list

    assert(page_ref(rp1) == 1);
    assert(nr_active_pages == 0 && nr_inactive_pages == 1);
    assert(list_next(&(inactive_list.swap_list)) == &(rp1->swap_link));

    page_launder();
    assert(nr_active_pages == 1 && nr_inactive_pages == 0);
    assert(PageSwap(rp1) && PageActive(rp1));

    entry = try_alloc_swap_entry();
    assert(swap_offset(entry) == 1);
    assert(!PageSwap(rp1) && nr_active_pages == 0);
    assert(list_empty(&(active_list.swap_list)));

    // set rp1 inactive again

    assert(page_ref(rp1) == 1);
    swap_page_add(rp1, 0);
    assert(PageSwap(rp1) && swap_offset(rp1->index) == 1);
    swap_inactive_list_add(rp1);
    mem_map[1] = 1;
    assert(nr_inactive_pages == 1);
    page_ref_dec(rp1);

    size_t count = nr_used_pages();
    nur_swap_remove_entry(entry);
    assert(nr_inactive_pages == 0 && nr_used_pages() == count - 1);

    // check swap_out_mm

    pte_t *ptep0 = get_pte(pgdir, TEST_PAGE, 0), *ptep1;
    assert(ptep0 != NULL && pte2page(*ptep0) == rp0);

    ret = swap_out_mm(mm, 0);
    assert(ret == 0);

    ret = swap_out_mm(mm, 10);
    assert(ret == 1 && mm->swap_address == TEST_PAGE + PGSIZE);

    ret = swap_out_mm(mm, 10);
    assert(ret == 0 && *ptep0 == entry && mem_map[1] == 1);
    assert(PageDirty(rp0) && PageActive(rp0) && page_ref(rp0) == 0);
    assert(nr_active_pages == 1
           && list_next(&(active_list.swap_list)) == &(rp0->swap_link));

    // check refill_inactive_scan()

    refill_inactive_scan();
    assert(!PageActive(rp0) && page_ref(rp0) == 0);
    assert(nr_inactive_pages == 1
           && list_next(&(inactive_list.swap_list)) == &(rp0->swap_link));

    page_ref_inc(rp0);
    page_launder();
    assert(PageActive(rp0) && page_ref(rp0) == 1);
    assert(nr_active_pages == 1
           && list_next(&(active_list.swap_list)) == &(rp0->swap_link));

    page_ref_dec(rp0);
    refill_inactive_scan();
    assert(!PageActive(rp0));

    // save data in rp0

    int i;
    for (i = 0; i < PGSIZE; i++) {
        ((char *)page2kva(rp0))[i] = (char)i;
    }

    page_launder();
    assert(nr_inactive_pages == 0
           && list_empty(&(inactive_list.swap_list)));
    assert(mem_map[1] == 1);

    rp1 = alloc_page();
    assert(rp1 != NULL);
    ret = swapfs_read(entry, rp1);
    assert(ret == 0);

    for (i = 0; i < PGSIZE; i++) {
        assert(((char *)page2kva(rp1))[i] == (char)i);
    }

    // page fault now

    *(char *)(TEST_PAGE) = 0xEF;

    rp0 = pte2page(*ptep0);
    assert(page_ref(rp0) == 1);
    assert(PageSwap(rp0) && PageActive(rp0));

    entry = try_alloc_swap_entry();
    assert(swap_offset(entry) == 1 && mem_map[1] == SWAP_UNUSED);
    assert(!PageSwap(rp0) && nr_active_pages == 0
           && nr_inactive_pages == 0);

    // clear accessed flag

    assert(rp0 == pte2page(*ptep0));
    assert(!PageSwap(rp0));

    ret = swap_out_mm(mm, 10);
    assert(ret == 0);
    assert(!PageSwap(rp0) && ptep_present(ptep0));

    // change page table

    ret = swap_out_mm(mm, 10);
    assert(ret == 1);
    assert(*ptep0 == entry && page_ref(rp0) == 0 && mem_map[1] == 1);

    count = nr_used_pages();
    refill_inactive_scan();
    page_launder();
    assert(count - 1 == nr_used_pages());

    ret = swapfs_read(entry, rp1);
    assert(ret == 0 && *(char *)(page2kva(rp1)) == (char)0xEF);
    free_page(rp1);

    // duplictate *ptep0

    ptep1 = get_pte(pgdir, TEST_PAGE + PGSIZE, 0);
    assert(ptep1 != NULL && ptep_invalid(ptep1));
    swap_duplicate(*ptep0);
    ptep_copy(ptep1, ptep0);
    mp_tlb_invalidate(pgdir, TEST_PAGE + PGSIZE);

    // page fault again
    // update for copy on write

    *(char *)(TEST_PAGE + 1) = 0x88;
    *(char *)(TEST_PAGE + PGSIZE) = 0x8F;
    *(char *)(TEST_PAGE + PGSIZE + 1) = 0xFF;
    assert(pte2page(*ptep0) != pte2page(*ptep1));
    assert(*(char *)(TEST_PAGE) == (char)0xEF);
    assert(*(char *)(TEST_PAGE + 1) == (char)0x88);
    assert(*(char *)(TEST_PAGE + PGSIZE) == (char)0x8F);
    assert(*(char *)(TEST_PAGE + PGSIZE + 1) == (char)0xFF);

    rp0 = pte2page(*ptep0);
    rp1 = pte2page(*ptep1);
    assert(!PageSwap(rp0) && PageSwap(rp1) && PageActive(rp1));

    entry = try_alloc_swap_entry();
    assert(!PageSwap(rp0) && !PageSwap(rp1));
    assert(swap_offset(entry) == 1 && mem_map[1] == SWAP_UNUSED);
    assert(list_empty(&(active_list.swap_list)));
    assert(list_empty(&(inactive_list.swap_list)));

    ptep_set_accessed(&perm);
    page_insert(pgdir, rp0, TEST_PAGE + PGSIZE, perm);

    // check swap_out_mm

    *(char *)(TEST_PAGE) = *(char *)(TEST_PAGE + PGSIZE) = 0xEE;
    mm->swap_address = TEST_PAGE + PGSIZE * 2;
    ret = swap_out_mm(mm, 2);
    assert(ret == 0);
    assert(ptep_present(ptep0) && !ptep_accessed(ptep0));
    assert(ptep_present(ptep1) && !ptep_accessed(ptep1));

    ret = swap_out_mm(mm, 2);
    assert(ret == 2);
    assert(mem_map[1] == 2 && page_ref(rp0) == 0);

    refill_inactive_scan();
    page_launder();
    assert(mem_map[1] == 2 && swap_hash_find(entry) == NULL);

    // check copy entry

    nur_swap_remove_entry(entry);
    ptep_unmap(ptep1);
    assert(mem_map[1] == 1);

    swap_entry_t store;
    ret = nur_swap_copy_entry(entry, &store);
    assert(ret == -E_NO_MEM);
    mem_map[2] = SWAP_UNUSED;

    ret = nur_swap_copy_entry(entry, &store);
    assert(ret == 0 && swap_offset(store) == 2 && mem_map[2] == 0);
    mem_map[2] = 1;
    ptep_copy(ptep1, &store);

    assert(*(char *)(TEST_PAGE + PGSIZE) == (char)0xEE
           && *(char *)(TEST_PAGE + PGSIZE + 1) == (char)0x88);

    *(char *)(TEST_PAGE + PGSIZE) = 1, *(char *)(TEST_PAGE + PGSIZE + 1) =
        2;
    assert(*(char *)TEST_PAGE == (char)0xEE
           && *(char *)(TEST_PAGE + 1) == (char)0x88);

    ret = nur_swap_in_page(entry, &rp0);
    assert(ret == 0);
    ret = nur_swap_in_page(store, &rp1);
    assert(ret == 0);
    assert(rp1 != rp0);

    // free memory

    nur_swap_list_del(rp0), nur_swap_list_del(rp1);
    swap_page_del(rp0), swap_page_del(rp1);

    assert(page_ref(rp0) == 1 && page_ref(rp1) == 1);
    assert(nr_active_pages == 0 && list_empty(&(active_list.swap_list)));
    assert(nr_inactive_pages == 0
           && list_empty(&(inactive_list.swap_list)));

    for (i = 0; i < HASH_LIST_SIZE; i++) {
        assert(list_empty(swap_hash_list + i));
    }

    page_remove(pgdir, TEST_PAGE);
    page_remove(pgdir, (TEST_PAGE + PGSIZE));

#if PMXSHIFT != PUXSHIFT
    free_page(pa2page(PMD_ADDR(*get_pmd(pgdir, TEST_PAGE, 0))));
#endif
#if PUXSHIFT != PGXSHIFT
    free_page(pa2page(PUD_ADDR(*get_pud(pgdir, TEST_PAGE, 0))));
#endif
    free_page(pa2page(PGD_ADDR(*get_pgd(pgdir, TEST_PAGE, 0))));
    pgdir[PGX(TEST_PAGE)] = 0;

    mm->pgdir = NULL;
    mm_destroy(mm);
    check_mm_struct = NULL;

    assert(nr_active_pages == 0 && nr_inactive_pages == 0);
    for (offset = 0; offset < max_swap_offset; offset++) {
        mem_map[offset] = SWAP_UNUSED;
    }

    assert(nr_used_pages_store == nr_used_pages());
    assert(slab_allocated_store == slab_allocated());

    kprintf("check_swap() succeeded.\n");
}

struct swap_manager fifo_swap_manager = {
    .name = "not use recently",
    .init = swap_init,
    .init_mm = NULL,
    .tick_event = kswapd_main,
    .swap_remove_entry = nur_swap_remove_entry,
    .swap_list_del = nur_swap_list_del,
    .swap_in_page = nur_swap_in_page,
    .swap_out_victim = nur_try_free_pages,
    .check_swap = check_swap,
};

