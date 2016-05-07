#ifndef __UAPI_LINUX_CONST_H__
#define __UAPI_LINUX_CONST_H__

#define __AC(X,Y)	(X##Y)
#define _AC(X,Y)	__AC(X,Y)

#define _BITUL(x)	(_AC(1,UL) << (x))

#endif /* ! __UAPI_LINUX_CONST_H__ */
