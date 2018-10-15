#ifndef PAGE_H
#define PAGE_H
#include "config_mm.h"
#include "sys.h"
	
struct bucket_desc
{
	struct bucket_desc *next;
	void *page;
	void *free_ptr;
	size_t nlink;	
};
struct _bucket_dir
{
	struct bucket_desc *head;
	size_t size;
};

extern u32 page_table[(MEM_SIZE/PAGE_SIZE+31)/32];//one bit for one page (bitmap)
extern struct _bucket_dir bucket_dir[NR_BUCKET_DESC+1];
extern struct bucket_desc *free_bdesc;

void *get_free_page(void);
void *get_free_nr_page(u32 nr);
void free_page(void *addr);
void free_nr_page(void *addr,size_t nr);
//取data的二进制位中1的个数
static inline u32 count_bit(u32 data)
{
	u32 cnt = 0;
	while(data){
		data &= (data-1);
		++cnt;
	}
	return cnt;
}
//对data进行2的指数上取整
static inline u32 ceil2(u32 data)
{	
	--data;
	data |= data>>1;
	data |= data>>2;
	data |= data>>4;
	data |= data>>8;
	data |= data>>16;
	return data+1;
}
//对data取2的对数，data必须为2^n
static inline u32 mm_log2(u32 data)
{
#if defined(DEBUG) && !defined(NDEBUG)	
	if(data != ceil2(data) || data == 0)
		sys_quit(-1,"%s:%d:data = %d argment error\n",__FUNCTION__,__LINE__,data);
#endif
	u32 index = 0;
	while((data & (1<<index)) == 0)
		++index;
	return index;
}
#if defined(DEBUG) && !defined(NDEBUG)
void show_page(void);
#endif

#endif
