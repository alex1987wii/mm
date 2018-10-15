#ifndef MEMORY_H
#define MEMORY_H

#if defined(MACH_SIZE) && (MACH_SIZE == 32)
	#define MACH_OFFSET 2
#else
	#define MACH_OFFSET 3
#endif

struct _ex_desc{//超过PAGE_SIZE的块描述符,分配策略：按PAGE_SIZE上取整后分配
	struct _ex_desc *next,*prev;
	void *start;
	size_t size;
};

extern struct _ex_desc *ex_desc;//已存放数据的结构体头指针
extern struct _ex_desc *free_ex_desc;//可使用的数据结构头指针

#define mm_free(addr)	mm_free_s(addr,0)
void *mm_malloc(size_t size);
void mm_free_s(void *addr,size_t size);
#if defined(DEBUG) && !defined(NDEBUG)
void mm_stat();
#endif

#endif