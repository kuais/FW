#ifndef __KU_ALLOC__
#define __KU_ALLOC__

#include <stdint.h>
#include <stdio.h>

typedef struct
{
	uint8_t *pool;
	uint16_t *table;
	uint32_t poolsize;
	uint8_t blocksize;
	uint32_t tablesize;
	uint8_t isinited;
	void (*init)(void);
	void *(*malloc)(uint32_t);
	void *(*realloc)(void *, uint32_t);
	void *(*calloc)(uint32_t, uint32_t);
	uint8_t (*free)(void *);
} MemMgr;

extern void ku_memset(void *s, uint8_t v, uint32_t n);
extern void ku_memcpy(void *des, void *src, uint32_t n);
extern void ku_meminit(MemMgr *mgr);
extern void *ku_malloc(MemMgr *mgr, uint32_t size);
extern void *ku_calloc(MemMgr *mgr, uint32_t n, uint32_t size);
extern void *ku_realloc(MemMgr *mgr, void *ptr, uint32_t size);
extern uint8_t ku_free(MemMgr *mgr, void *ptr);

#endif
