#include "KuFrame/kualloc.h"

/*	 程序内存池	*/

#define MEM_BLOCKSIZE 32						/* 内存块大小 */
#define MEM_MAXSIZE 0x14000						/* 内存池容量 80K*/
#define MEM_TBLSIZE MEM_MAXSIZE / MEM_BLOCKSIZE /* 内存使用表大小 */
#define MEM_ADDR 0X20008000						/* 内存缓冲区起始地址 */

// 内存池(32字节对齐)
static __align(MEM_BLOCKSIZE) uint8_t membuf[MEM_MAXSIZE] __attribute__((at(MEM_ADDR))); // 内存池
																						 // 可用地址 0x2008000-2001CFFF
																						 // 分配地址 0x2008000-2001CFFF
static uint16_t memtable[MEM_TBLSIZE] __attribute__((at(MEM_ADDR + MEM_MAXSIZE)));		 // 内存状态表起始地址 0x2001C000-2001D400
static MemMgr *pMgr;

/*	私有函数定义  */
static void _init(void)
{
	ku_meminit(pMgr);
}
static void *_malloc(uint32_t size)
{
	return ku_malloc(pMgr, size);
}
static void *_realloc(void *ptr, uint32_t size)
{
	return ku_realloc(pMgr, ptr, size);
}
static void *_calloc(uint32_t n, uint32_t size)
{
	return ku_calloc(pMgr, n, size);
}
static uint8_t _free(void *ptr)
{
	return ku_free(pMgr, ptr);
}

MemMgr mmgr1 = {
	membuf,
	memtable,
	MEM_MAXSIZE,
	MEM_BLOCKSIZE,
	MEM_TBLSIZE,
	0,
	_init,
	_malloc,
	_realloc,
	_calloc,
	_free};
static MemMgr *pMgr = (MemMgr *)&mmgr1;
