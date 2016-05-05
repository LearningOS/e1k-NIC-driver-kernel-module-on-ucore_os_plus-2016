#include <assert.h>
#include <string.h>
#include <kio.h>
#include <file.h>
#include <stat.h>
#include <slab.h>
#include <elf.h>
#include <mmu.h>
#include <vmm.h>
#include <vfs.h>
#include <inode.h>
#include <iobuf.h>
#include <ide.h>
#include <moduleloader.h>
#include <mod.h>
#include <sem.h>
#include <stdlib.h>
#include <error.h>
#include <stdio.h>

void *ucore_memset(void *s, char c, size_t n) {
	return memset(s, c, n);
}
EXPORT_SYMBOL(ucore_memset);

void *ucore_strcpy(void *dst, const void *src){
	return strcpy(dst,src);
}
EXPORT_SYMBOL(ucore_strcpy);

char *ucore_strncpy(char *dst, const char *src, size_t len){
	return strncpy(dst,src,len);
}
EXPORT_SYMBOL(ucore_strncpy);

void *ucore_kmalloc(size_t size) {
    void *ret = kmalloc(size);
    memset(ret, 0, size);
    return ret;
}
EXPORT_SYMBOL(ucore_kmalloc);

void ucore_kfree(void *objp){
	return kfree(objp);
}
EXPORT_SYMBOL(ucore_kfree);

