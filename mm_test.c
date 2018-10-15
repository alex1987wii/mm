/*	2018年9月11日23:08:08
**	author: alex
*/
//内存池测试程序，先申请TEST_NUM[1-3]个随机大小空间，再随机释放全部申请空间
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include "page.h"
#include "memory.h"

void *MEM_START;
#define MAGIC	0x200
#define TEST_NUM1	1000	//普通测试次数
#define TEST_NUM2	100000	//小容量内存测试次数
#define TEST_NUM3	100	//大容量（超过PAGE_SIZE）测试次数

int main(int argc,const char **argv)
{	
	MEM_START = malloc(MEM_SIZE);
	if(MEM_START == NULL)
	{
		perror("malloc failed");
		exit(1);
	}
	printf("内存初始状态\n");
	mm_stat();//显示分配前内存状态
	getchar();
	srand(time(NULL));
	
	//genernel test
	printf("普通测试\n");
	void *addr[TEST_NUM1]={NULL};
	unsigned int i,size;
	for(i = 0; i< TEST_NUM1; ++i){
		size = rand()%MAGIC+1;
		printf("request memory size:%d\n",size);
		addr[i] = mm_malloc(size);//在已有空间使用自定义分配函数		
	}
	printf("分配后：\n");
	mm_stat();
	getchar();
	unsigned int index;
	for(i = 0;i < TEST_NUM1;++i){
		do{
			index = rand()%TEST_NUM1;
		}while(addr[index] == NULL);
		printf("free memory:%p\n",addr[index]);
		mm_free(addr[index]);//在已有空间使用自定义释放函数
		addr[index] = NULL;		
	}
	printf("释放后：\n");
	mm_stat();
	getchar();
	
	//小容量内存测试
	printf("小容量内存测试\n");
	void *addr1[TEST_NUM2]={NULL};

	for(i = 0; i< TEST_NUM2; ++i){
		size = rand()%4+1;
		printf("request memory size:%d\n",size);
		addr1[i] = mm_malloc(size);//在已有空间使用自定义分配函数		
	}
	printf("分配后：\n");
	mm_stat();
	getchar();
	
	for(i = 0;i < TEST_NUM2;++i){
		do{
			index = rand()%TEST_NUM2;
		}while(addr1[index] == NULL);
		printf("free memory:%p\n",addr1[index]);
		mm_free(addr1[index]);//在已有空间使用自定义释放函数
		addr1[index] = NULL;		
	}
	printf("释放后：\n");
	mm_stat();
	getchar();
	
	//0内存测试
	void *addr2 = mm_malloc(0);
	if(addr2 == NULL){
		printf("申请0内存空间成功返回NULL\n\n");
	}
	else{
		printf("申请0内存空间没有返回NULL\n\n");
	}
	
	//释放未使用的空间
	void *addr3 = (void *)(rand()%MEM_SIZE+MEM_START);
	printf("释放随机空间%p\n",addr3);
	mm_free(addr3);
	
	//给定错误的参数释放刚申请的空间
	size = rand()%MAGIC+1;	
	void *addr4 = mm_malloc(size);
	printf("\n给定错误的参数释放刚申请的空间\n");
	mm_stat();
	printf("\n申请的地址为%p,大小为%d\n",addr4,size);	
	printf("给定错误的地址释放刚申请的空间\n");
	mm_free(addr4+rand()%MAGIC-MAGIC/2);//几乎不可能刚好就是addr4，申请时会打印地址
	printf("\n给定错误的大小释放刚申请的空间\n");
	mm_free_s(addr4,size+rand()%MAGIC-MAGIC/2);//若给定的大小刚好在可能范围内，是能正常释放的
	printf("使用mm_free_s正常释放\n");
	mm_free_s(addr4,size);
	mm_stat();
	getchar();
	
	//申请超过PAGE_SIZE的任意大小空间
	printf("大容量（超过PAGE_SIZE）测试\n");
	void *addr5[TEST_NUM3]={NULL};
	int nr_malloc = 0;//成功申请的次数
	for(i = 0; i< TEST_NUM3; ++i){
		size = (rand()%16)*PAGE_SIZE+rand()%PAGE_SIZE;
		printf("request memory size:%d\n",size);
		addr5[i] = mm_malloc(size);//在已有空间使用自定义分配函数
		addr5[i]&&(++nr_malloc);//使用&&运算符的短路特性增加成功申请的次数
	}
	printf("分配后：\n");
	mm_stat();
	getchar();
	
	//释放大空间
	for(i = 0;i < nr_malloc;++i){
		do{
			index = rand()%TEST_NUM3;
		}while(addr5[index] == NULL && nr_malloc - i > 0);
		printf("free memory:%p\n",addr5[index]);
		mm_free(addr5[index]);//在已有空间使用自定义释放函数
		addr5[index] = NULL;		
	}
	printf("释放后：\n");
	mm_stat();
	getchar();
	
	
	//申请超大空间（超过内存总容量大小）
	printf("申请超大空间（超过内存总容量大小）\n");
	mm_malloc(MEM_SIZE + rand()%MAGIC+1);
	
	
	free(MEM_START);
	return 0;
}
