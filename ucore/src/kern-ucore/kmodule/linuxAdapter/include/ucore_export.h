#ifndef __UCORE_EXPORT_H__
#define __UCORE_EXPORT_H__

#ifndef MODULE_SYMBOL_PREFIX
#define MODULE_SYMBOL_PREFIX ""
#endif

#define __EXPORT_SYMBOL(sym, sec) 				\
	extern typeof(sym) sym; 			\
static const char __kstrtab_##sym[] \
__attribute__((section("__ksymtab_strings"), aligned(1))) \
= MODULE_SYMBOL_PREFIX #sym; 		\
static const struct kernel_symbol __ksymtab_##sym \
	  __used								\
__attribute__((section("__ksymtab" sec), unused)) \
= { (unsigned long)&sym, __kstrtab_##sym }

#define EXPORT_SYMBOL(sym) __EXPORT_SYMBOL(sym, "")

#endif
