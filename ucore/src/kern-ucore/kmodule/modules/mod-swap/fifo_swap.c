#include "fifo_swap.h"
#include <vmm.h>
#include <pmm.h>
#include <types.h>
#include <memlayout.h>
#include <swap_manager.h>

extern unsigned short* mem_map;
void fifo_tick_event(){
    kprintf("this is fifo tick event\n");
}

void fifo_init(){

}

void fifo_swap_list_del(struct Page* page){
    assert(PageSwap(page));
    list_del(&(page->swap_link));
}

void fifo_swap_remove_entry(swap_entry_t entry){
    size_t offset= swap_offset(entry);
    assert(mem_map[offset] >0);
    if (--mem_map[offset] == 0){
        struct Page* page= swap_hash_find(entry);
        if (page != NULL){
            if (page_ref(page) != 0){
                return ;
            }
            fifo_swap_list_del(page);
            swap_free_page(page);
        }
        mem_map[offset] = SWAP_UNUSED;
    }
}

struct swap_manager fifo_swap_manager = {
    .name = "first in first out",
    .init = fifo_init,
    .init_mm = NULL,
    .tick_event = fifo_tick_event,
    .swap_remove_entry = NULL,
    .swap_list_del = NULL,
    .swap_in_page = NULL,
    .swap_out_victim = NULL,
    .check_swap = NULL,
};

