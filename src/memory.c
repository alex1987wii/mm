/* 	2018年9月10日22:51:18
**	author: alex
*/

#include "page.h"
#include "sys.h"
#include "memory.h"

struct _ex_desc *ex_desc = NULL;
struct _ex_desc *free_ex_desc = NULL;

static inline void init_bucket_desc(void)
{
	struct bucket_desc *bdesc = get_free_page();
	if(!bdesc)
		return ;
	//从后往前初始化
	struct bucket_desc *pdesc = bdesc + (PAGE_SIZE/sizeof(struct bucket_desc))-1;
	pdesc->next = free_bdesc;
	pdesc->page = pdesc->free_ptr = NULL;
	pdesc->nlink = 0;
	--pdesc;
	while(pdesc >= bdesc){
		pdesc->next = pdesc+1;
		pdesc->page = pdesc->free_ptr = NULL;
		pdesc->nlink = 0;
		--pdesc;
	}
	free_bdesc = bdesc;
}

static inline void init_ex_desc(void)
{
	struct _ex_desc *edesc = get_free_page();
	if(!edesc)
		return;
	struct _ex_desc *pdesc = edesc + (PAGE_SIZE/sizeof(struct _ex_desc))-1;
	pdesc->next = free_ex_desc;
	if(free_ex_desc)
		free_ex_desc->prev = pdesc;
	pdesc->prev = pdesc-1;
	pdesc->start = NULL;
	pdesc->size = 0;
	--pdesc;
	while(pdesc >= edesc){
		pdesc->next = pdesc+1;
		pdesc->prev = pdesc-1;
		pdesc->start = NULL;
		pdesc->size = 0;
		--pdesc;
	}
	edesc->prev = NULL;
	free_ex_desc = edesc;
}

static inline void init_page(void *addr,size_t size)
{
	//若为小空间,则所指向的page不用初始化
	if(size < MACH_SIZE/8)
		return;
	void *tmp_addr = addr + PAGE_SIZE - size;
	//这里指转为struct bucket_desc *,其实类型不一定非得这个，只要改变所指地址空间的第一个
	//成员(一个void*字长)即可
	((struct bucket_desc *)tmp_addr)->next = NULL;
	tmp_addr -= size;
	while(tmp_addr >= addr){
		((struct bucket_desc *)tmp_addr)->next = tmp_addr+size;
		tmp_addr -= size;
	}
}

void *mm_malloc(size_t size)
{
	if(size == 0)
		return NULL;
	else if(size > MEM_SIZE){
		sys_ret(NULL,"request memory size(%d) is too large\n",MEM_SIZE);
	}
	else if(size > PAGE_SIZE){
		u32 nr_page = (size + PAGE_SIZE -1)/PAGE_SIZE;//求所需PAGE数量
		void *page = get_free_nr_page(nr_page);
		if(page == NULL)
			goto oom;
		//空间申请成功后再申请ex_desc
		if(free_ex_desc == NULL){
			init_ex_desc();
			if(free_ex_desc == NULL){//初始化失败，即内存不够
				free_nr_page(page,nr_page);
				goto oom;
			}
		}
		//先将_ex_desc从free_ex_desc链表中取出来,初始化后加入ex_desc链表中
		struct _ex_desc *edesc = free_ex_desc;
		free_ex_desc = edesc->next;
		if(free_ex_desc)
			free_ex_desc->prev = NULL;
		edesc->next = ex_desc;
		if(ex_desc)
			ex_desc->prev = edesc;
		//edesc->prev = NULL;从free_ex_desc中取出的第一个结点，其prev域已经是NULL;
		edesc->start = page;
		edesc->size = size;		
		ex_desc = edesc;
		return page;
	}
	u32 alloc_size = ceil2(size);
	u32 bucket_index =mm_log2(alloc_size);
	void *free_addr;	
	struct bucket_desc *pdesc = bucket_dir[bucket_index].head;
	while(1){
		if(pdesc == NULL){
			if(free_bdesc == NULL){
				init_bucket_desc();
				if(free_bdesc == NULL)
					goto oom;
			}
			pdesc = free_bdesc;
			free_bdesc = free_bdesc->next;
			pdesc->next = bucket_dir[bucket_index].head;
			bucket_dir[bucket_index].head = pdesc;						
		}
		if(pdesc->page == NULL){
			pdesc->page = get_free_page();
			if(pdesc->page == NULL)
				goto oom;
			init_page(pdesc->page,bucket_dir[bucket_index].size);
			pdesc->free_ptr = pdesc->page;
		}
		else if(pdesc->free_ptr){
			free_addr = pdesc->free_ptr;
			if(alloc_size < MACH_SIZE/8){
				pdesc->free_ptr += bucket_dir[bucket_index].size;
				if(pdesc->free_ptr == pdesc->page + PAGE_SIZE)
					pdesc->free_ptr = NULL;
			}
			else{
				pdesc->free_ptr = (void *)(((struct bucket_desc *)free_addr)->next);				
			}
			++pdesc->nlink;
			return free_addr;
		}
		else{
			pdesc = pdesc->next;			
		}		
	}
	sys_quit(-1,"%s:%d:Can't be here\n",__FUNCTION__,__LINE__);
oom:
	sys_ret(NULL,"%s:%d:out of memory\n",__FUNCTION__,__LINE__);
}

void mm_free_s(void *addr,size_t size)
{
	u32 bucket_index = 0;
	struct bucket_desc *pdesc;
	void *page = ((addr-MEM_START) - (addr-MEM_START)%PAGE_SIZE)+MEM_START;
	if(addr == page){//若给定地址是page地址，则先在ex_desc链表中查找
		struct _ex_desc *edesc = ex_desc;
		while(edesc){
			if(edesc->start == page){
				if(size && (size != edesc->size)){//找到了对应的page,但给定的size与edesc的size不符
					sys_warning("%s:%d:invalid size %d\n",__FUNCTION__,__LINE__,(u32)size);
					return ;
				}
				free_nr_page(edesc->start,(edesc->size + PAGE_SIZE -1)/PAGE_SIZE);
				//将edesc所指向的描述符从ex_desc链表中删除再加入free_ex_desc链表中
				if(edesc->prev)
					edesc->prev->next = edesc->next;
				else{
#if defined(DEBUG) && !defined(NDEBUG)
					if(ex_desc != edesc)
						sys_quit(-1,"%s:%d:fatal error\n",__FUNCTION__,__LINE__);
#endif
					ex_desc = edesc->next;					
				}
				if(edesc->next)
					edesc->next->prev = edesc->prev;
				edesc->prev = NULL;
				edesc->next = free_ex_desc;
				edesc->start = NULL;
				edesc->size = 0;
				if(free_ex_desc)
					free_ex_desc->prev = edesc;
				free_ex_desc = edesc;
				return ;
			}
			edesc = edesc->next;
		}
	}
	//在ex_desc链表中没找到，则进入bucket_desc中查找
	for(;bucket_index <= NR_BUCKET_DESC; ++bucket_index){
		if(bucket_dir[bucket_index].size < size)
			continue;
		for(pdesc = bucket_dir[bucket_index].head; pdesc; pdesc = pdesc->next){
			if(pdesc->page == page)
				goto found;
		}
	}
unused:
	sys_warning("%s:%d:%p is not used\n",__FUNCTION__,__LINE__,addr);
ret:
	return ;
found:
//若所在空间的size为1，2，4（64位机)字节，则只合并在free_ptr前面的一个单位
	if((addr - page)%bucket_dir[bucket_index].size){//防止传入参数给定地址没有向bucket_dir[bucket_index].size对齐
		sys_warning("%s:%d:%p:invalid address\n",__FUNCTION__,__LINE__,addr);
		goto ret;
	}
	if(bucket_index < MACH_OFFSET){
		if(addr + bucket_dir[bucket_index].size == pdesc->free_ptr \
		|| (pdesc->free_ptr == NULL && addr == pdesc->page + PAGE_SIZE - bucket_dir[bucket_index].size))
			pdesc->free_ptr = addr;
		else if(pdesc->free_ptr != NULL && addr >= pdesc->free_ptr)//若给定地址已经在自由空间范围内
			goto unused;
	}
	else {
		//检查给定地址是不是在free list上
		struct bucket_desc *tmp_addr = (struct bucket_desc *)pdesc->free_ptr;
		while(tmp_addr){
			if(tmp_addr == addr)
				goto unused;
			tmp_addr = tmp_addr->next;
		}
		((struct bucket_desc *)addr)->next = (struct bucket_desc *)(pdesc->free_ptr);
		pdesc->free_ptr = addr;
	}
	
	--pdesc->nlink;
	if(pdesc->nlink == 0){
		free_page(pdesc->page);//release data page;
		pdesc->page = pdesc->free_ptr = NULL;
#ifdef CONFIG_RELEASE_DESC_PAGE		
		page = (void *)((((long)pdesc-MEM_START) - ((long)pdesc-MEM_START)%PAGE_SIZE)+MEM_START);//get bucket_desc page address
		struct bucket_desc *tmp = (struct bucket_desc *)page;		
		for(; tmp < page + PAGE_SIZE; ++tmp){
			if(tmp->page)
				break;
		}
		if(tmp >= page + PAGE_SIZE){
			//release bucket_desc page
			//unfinished
			free_page(page);
		}
		else{
			//put pdesc into free_bdesc;
			//unfinished
		}
#endif		
	}
}
#if defined(DEBUG) && !defined(NDEBUG)
void mm_stat(void)
{
	u32 nr_bucket_desc[NR_BUCKET_DESC+1];
	u32 mem_used[NR_BUCKET_DESC+1];
	u32 total_desc = 0;
	u32 total_mem_used = 0;
	u32 i,nr_free_desc = 0;
	u32 nr_free_ex_desc,nr_ex_desc,ex_mem_used;
	struct bucket_desc *pdesc;
	show_page();
	sys_info("memory info:\n");
	sys_info("nr_bucket_desc\tmem_used\n");
	for(i = 0; i <= NR_BUCKET_DESC; ++i){
		nr_bucket_desc[i] = 0;
		mem_used[i] = 0;
		for(pdesc = bucket_dir[i].head; pdesc; pdesc = pdesc->next){
			++nr_bucket_desc[i];
			mem_used[i] += pdesc->nlink;
		}
		mem_used[i] *= bucket_dir[i].size;
		total_desc += nr_bucket_desc[i];
		total_mem_used += mem_used[i];
		sys_info("%d\t\t%d\n",nr_bucket_desc[i],mem_used[i]);
	}
	pdesc = free_bdesc;
	while(pdesc){
		++nr_free_desc;
		pdesc = pdesc->next;
	}
	
	nr_free_ex_desc = nr_ex_desc = ex_mem_used = 0;
	struct _ex_desc *edesc = free_ex_desc;
	while(edesc){
		++nr_free_ex_desc;
		edesc = edesc->next;
	}
	edesc = ex_desc;
	while(edesc){
		++nr_ex_desc;
		ex_mem_used += edesc->size;
		edesc = edesc->next;
	}
	total_mem_used += ex_mem_used;
	sys_info("total:%d\t%d\tfree_desc:%d\tmem_size:%d\tused_rate:%d%%\n",\
	total_desc,total_mem_used,nr_free_desc,MEM_SIZE,(total_mem_used*100)/MEM_SIZE);	
	sys_info("ex_desc:%d\tfree_ex_desc:%d\tex_mem_used:%d\n",nr_ex_desc,nr_free_ex_desc,ex_mem_used);
}
#endif
