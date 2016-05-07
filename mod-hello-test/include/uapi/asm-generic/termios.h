#ifndef __UAPI_ASM_GENERIC_TERMIOS_H__
#define __UAPI_ASM_GENERIC_TERMIOS_H__

#include <asm/termbits.h>
#include <asm/ioctls.h>

struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

#define NCC 8

#endif /* ! __UAPI_ASM_GENERIC_TERMIOS_H__ */
