#include "kualloc.h"
#include "ku.h"
#include <RTL.h>

/* private */
static uint32_t _malloc(MemMgr *mgr, uint32_t size)
{
	uint32_t blockcnt;	// 需要的内存块数
	uint32_t count = 0; // 连续空内存块数
	uint32_t offset = 0;
	if (!mgr->isinited)
		mgr->init();										   // 未初始化,先执行初始化
	blockcnt = (size + (mgr->blocksize) - 1) / mgr->blocksize; // 获取需要分配的连续内存块数
	while (offset < mgr->tablesize)
	{
		if (mgr->table[offset])
		{
			count = 0;
			offset += mgr->table[offset];
		}
		else
		{
			count++;
			if (count == blockcnt)
			{
				while (count-- > 0)
				{
					mgr->table[offset--] = (uint16_t)blockcnt;
				}
				offset++;
				return (uint32_t)mgr->pool + offset * mgr->blocksize;
			}
			offset++;
		}
		//		os_dly_wait(1);
	}
	return 0;
}
/**
 * @brief  释放空间
 * @param  mgr 内存管理器
 * @param  ptr 要释放的内存的起始指针
 * @return     0-成功， 1-出错， 2-无效地址
 */
static uint8_t _free(MemMgr *mgr, void *ptr)
{
	if (!mgr->isinited)
	{
		mgr->init(); // 未初始化,先执行初始化
		return 1;
	}
	uint32_t offset = (uint32_t)ptr - (uint32_t)mgr->pool;
	if (offset < mgr->poolsize)
	{
		uint32_t index = offset / mgr->blocksize; // 偏移所在内存块序号
		uint32_t count = mgr->table[index];		  // 内存块数量
		ku_memset(&mgr->table[index], 0, count * 2);
		ku_memset(ptr, 0, count * mgr->blocksize);
		return 0;
	}
	else
		return 2; // 超出地址范围
}

/**
 * @brief  设置一段内存的值
 * @param  s 要设置的内存的起始指针
 * @param  v 要设置的值
 * @param  n 数量
 * @return   无
 */
void ku_memset(void *s, uint8_t v, uint32_t n)
{
	uint8_t *xs = s;
	while (n--)
		*xs++ = v;
}
/**
 * @brief  复制内存
 * @param  des 目标内存指针
 * @param  src 源内存指针
 * @param  n   复制数据数
 * @return     无
 */
void ku_memcpy(void *des, void *src, uint32_t n)
{
	uint8_t *xdes = des;
	uint8_t *xsrc = src;
	while (n--)
		*xdes++ = *xsrc++;
}
/**
 * @brief  内存管理器置初值
 * @param  mgr 内存管理器
 * @return     无
 */
void ku_meminit(MemMgr *mgr)
{
	ku_memset(mgr->table, 0, mgr->tablesize * 2); // 内存状态表数据清零
	ku_memset(mgr->pool, 0, mgr->poolsize);		  // 内存池所有数据清零
	mgr->isinited = 1;
}
/**
 * @brief 申请内存空间
 * @param mgr  内存管理器
 * @param size 要申请的大小
 */
void *ku_malloc(MemMgr *mgr, uint32_t size)
{
	if (size == 0)
		return 0; // 不需要分配
	uint32_t addr = _malloc(mgr, size);
	if (addr == 0)
		return 0;
	return (void *)addr;
}
/**
 * @brief 重新申请空间
 * @param mgr  内存管理器
 * @param ptr  原来的内存空间
 * @param size 新空间大小
 */
void *ku_realloc(MemMgr *mgr, void *ptr, uint32_t size)
{
	if (size == 0)
	{
		ku_free(mgr, ptr); // 释放旧内存
		return 0;
	}
	else
	{
		uint32_t addr = _malloc(mgr, size);
		if (addr == 0)
			return 0;
		ku_memcpy((void *)addr, ptr, size); // 拷贝旧内存内容到新内存
		ku_free(mgr, ptr);					// 释放旧内存
		return (void *)addr;				// 返回新内存首地址
	}
}
/**
 * @brief 申请多段内存空间
 * @param mgr  内存管理器
 * @param n    内存段数
 * @param size 单端内存大小
 */
void *ku_calloc(MemMgr *mgr, uint32_t n, uint32_t size)
{
	void *p = ku_malloc(mgr, n * size);
	if (p != 0)
		ku_memset(p, 0, n * size);
	return p;
}
/**
 * @brief  释放内存空间
 * @param  mgr 内存管理器
 * @param  ptr 内存段数
 * @return     0:成功， 1:失败 	2:无效参数
 */
uint8_t ku_free(MemMgr *mgr, void *ptr)
{
	if (ptr == 0)
		return 1;
	return _free(mgr, ptr);
}
