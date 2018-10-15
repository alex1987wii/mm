/*	2018年9月10日21:05:06
**	author:	alex
*/
#include "page.h"
#include "sys.h"
#define NR_PAGE_TABLE	((MEM_SIZE/PAGE_SIZE+31)/32)
u32 page_table[NR_PAGE_TABLE] = {0};
struct _bucket_dir bucket_dir[NR_BUCKET_DESC+1] = {
	{NULL,1},
	{NULL,2},
	{NULL,4},
	{NULL,8},
	{NULL,16},
	{NULL,32},
	{NULL,64},
	{NULL,128},
	{NULL,256},
	{NULL,512},
#if (NR_BUCKET_DESC < 10)
	#error "PAGE_SIZE too small"
#endif
#if (NR_BUCKET_DESC >= 10)
	{NULL,1024},
#endif
#if (NR_BUCKET_DESC >= 11)
	{NULL,2048},
#endif
#if (NR_BUCKET_DESC >= 12)
	{NULL,4096},
#endif
#if (NR_BUCKET_DESC >= 13)
	{NULL,8192},
#endif
#if (NR_BUCKET_DESC >= 14)
	{NULL,16384},
#endif
#if (NR_BUCKET_DESC >= 15)
	{NULL,32768},
#endif
#if (NR_BUCKET_DESC == 16)
	{NULL,65535},
#endif
#if (NR_BUCKET_DESC > 16)
	#error "PAGE_SIZE too large"
#endif
};
struct bucket_desc *free_bdesc = NULL;
void *get_free_page(void)
{
	u32 *bp = page_table;
	u32 nr_bit = 0;
	while(bp != page_table+NR_PAGE_TABLE && (*bp == 0xffffffff))
		++bp;
	if(bp == page_table+NR_PAGE_TABLE){
		sys_warning("out of memory\n");
		return NULL;
	}	
	while(*bp & (1<<nr_bit++))
		;
	--nr_bit;
	*bp |= (1<<nr_bit);
	return (void *)(MEM_START + ((bp-page_table)*32 + nr_bit)*PAGE_SIZE);
}
void *get_free_nr_page(u32 nr)
{
	u32 sq_page = 0,nr_bit = 0;
	u32 *pg = page_table;
	void *addr;
	if(nr > NR_PAGE_TABLE*32){
		sys_warning("request too many pages\n");
		return NULL;
	}
	while(pg != page_table+ NR_PAGE_TABLE){		
		if(*pg & (1<<nr_bit)){
			sq_page = 0;
		}
		else{
			++sq_page;
		}
		if(sq_page == nr){//set nr bitmap before return the address	
			addr = (void *)(MEM_START + ((pg-page_table)*32 + nr_bit - nr)*PAGE_SIZE);//获得可使用的空间开始地址
			while(sq_page){
				*pg |= (1<<nr_bit);
				if(nr_bit){
					--nr_bit;
				}
				else{
					--pg;
					nr_bit = 31;
				}
				--sq_page;
			}
			return addr;
		}		
		++nr_bit;
		pg += nr_bit/32;
		nr_bit %= 32;
	}
	sys_info("nr = %u : sq_page = %u\n",nr,sq_page);
	sys_warning("%s:%d:out of memory\n",__FUNCTION__,__LINE__);
	return NULL;
}
void free_page(void *addr)
{
#if	defined(DEBUG) && !defined(NDEBUG)
	if((addr-MEM_START) & (PAGE_SIZE-1))
		sys_quit(-1,"%s:%d:%p is not a page address\n",__FUNCTION__,__LINE__,addr);
#endif
	u32 nr_page = ((long)(addr-MEM_START)&~(PAGE_SIZE-1))/PAGE_SIZE;
	u32 nr_index = nr_page/32;
	u32 nr_bit = nr_page%32;
	page_table[nr_index] &= ~(1<<nr_bit);	
}

void free_nr_page(void *addr,u32 nr)
{
#if	defined(DEBUG) && !defined(NDEBUG)
	if((addr-MEM_START) & (PAGE_SIZE-1))
		sys_quit(-1,"%s:%d:%p is not a page address\n",__FUNCTION__,__LINE__,addr);
#endif
	u32 nr_page = ((long)(addr-MEM_START)&~(PAGE_SIZE-1))/PAGE_SIZE;	
	u32 nr_index;
	u32 nr_bit;
	u32 i;
	for(i = nr; i; --i){
		nr_index = nr_page/32;
		nr_bit = nr_page%32;
		page_table[nr_index] &= ~(1<<nr_bit);
		++nr_page;
	}	
}

#if defined(DEBUG) && !defined(NDEBUG)
void show_page(void)
{
	u32 page_used = 0;
	u32 page_free;
	u32 page_total = NR_PAGE_TABLE * 32;
	u32 index = 0;
	while(index < NR_PAGE_TABLE){
		page_used += count_bit(page_table[index]);
		++index;
	}
	page_free = page_total - page_used;
	sys_info("page info:\nused:%d\tfree:%d\ttotal:%d\tused_rate:%d%%\n",\
	page_used,page_free,page_total,(page_used*100)/page_total);
}	
#endif
