#include "KuFrame/kualloc.h"

/*	 �����ڴ��	*/

#define MEM_BLOCKSIZE 32						/* �ڴ���С */
#define MEM_MAXSIZE 0x14000						/* �ڴ������ 80K*/
#define MEM_TBLSIZE MEM_MAXSIZE / MEM_BLOCKSIZE /* �ڴ�ʹ�ñ��С */
#define MEM_ADDR 0X20008000						/* �ڴ滺������ʼ��ַ */

// �ڴ��(32�ֽڶ���)
static __align(MEM_BLOCKSIZE) uint8_t membuf[MEM_MAXSIZE] __attribute__((at(MEM_ADDR))); // �ڴ��
																						 // ���õ�ַ 0x2008000-2001CFFF
																						 // �����ַ 0x2008000-2001CFFF
static uint16_t memtable[MEM_TBLSIZE] __attribute__((at(MEM_ADDR + MEM_MAXSIZE)));		 // �ڴ�״̬����ʼ��ַ 0x2001C000-2001D400
static MemMgr *pMgr;

/*	˽�к�������  */
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
