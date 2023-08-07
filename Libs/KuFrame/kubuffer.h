#ifndef __KU_BUFFER__
#define __KU_BUFFER__

#include <stdint.h>

typedef struct _KuBuffer
{
    uint8_t *datas;          /* 数据缓冲区 */
    uint16_t size;           /* 数据缓冲区大小 */
    volatile uint16_t pos;   /* 指针位置 */
    volatile uint16_t count; /* 数据量 */
} KuBuffer;

extern KuBuffer *buffer_new(uint16_t size);
extern void buffer_free(KuBuffer *buf);
extern void buffer_resize(KuBuffer *buf, uint16_t size);
extern int buffer_dataCount(KuBuffer *buf);
extern void buffer_put(KuBuffer *buf, uint8_t *datas, uint16_t length);
extern uint8_t *buffer_get(KuBuffer *buf, int offset);
extern void buffer_remove(KuBuffer *buf, uint16_t length);
extern void buffer_clear(KuBuffer *buf);
extern int buffer_find(KuBuffer *buf, int offset, uint8_t value);
extern int buffer_finds(KuBuffer *buf, int offset, uint8_t *values, uint16_t length);

#endif
