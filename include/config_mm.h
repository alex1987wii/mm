#ifndef CONFIG_MM_H
#define CONFIG_MM_H
//config information
//MEM_START CAN'T BE 0x0
#if defined(DEBUG) && !defined(NDEBUG)
extern void *MEM_START;
#elif !defined(MEM_START)
#define MEM_START 	0x100000
#endif

#ifndef MEM_SIZE
#define MEM_SIZE	0x100000
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE	0x1000
#endif

//#define NR_BUCKET_DESC	(__builtin_ffs((PAGE_SIZE)))
//FIXME if you changed PAGE_SIZE,and NR_BUCKET_DESC must be log2(PAGE_SIZE)
#define NR_BUCKET_DESC	12

#if (PAGE_SIZE&(PAGE_SIZE-1)) || (MEM_SIZE%(PAGE_SIZE*32))
	#error "MEM_SIZE must aligned with PAGE_SIZE"
#endif
#if defined(CONFIG_COMPLIER) && !defined(__GNUC__)
	#error "I want gcc"
#endif

//arch information
#if defined(ARCH) && ( ARCH == 32 )
	#include "arch/x86_type.h"
#elif defined(ARCH) && (ARCH == 64 )
	#include "arch/x64_type.h"
#else
	#include "def_type.h"
#endif

#if defined(DEBUG) && !defined(NDEBUG)
#include <stddef.h>
#else
typedef unsigned int size_t;
typedef int ssize_t;
#endif

#define NULL ((void *)0)
#endif
