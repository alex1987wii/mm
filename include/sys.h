#ifndef SYS_H
#define SYS_H
//若要让调试信息重定向，可更改此处Dprintf的宏定义
#define sys_warning(fmt...) 	Dprintf(fmt)
#define sys_ret(ret,fmt...) 	{Dprintf(fmt);return ret;}
#define sys_quit(err,fmt...)	{Dprintf(fmt);exit(err);}
#define sys_info(fmt...)		sys_warning(fmt)
#if defined(DEBUG) && !defined(NDEBUG)
	#include <stdio.h>
	#include <stdlib.h>
	#define Dprintf(fmt...) 	printf(fmt)
#else
	#define Dprintf(fmt...)
#endif
#endif
