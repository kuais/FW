#include "kubuffer.h"
#include "ku.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

KuBuffer *buffer_new(uint16_t size)
{
    KuBuffer *buf = (KuBuffer *)kumalloc(sizeof(KuBuffer));
    if (size > 0)
    {
        buf->datas = (uint8_t *)kucalloc(size, sizeof(uint8_t));
        buf->size = size;
    }
    else
    {
        buf->datas = NULL;
        buf->size = size;
    }
    buf->pos = 0;
    buf->count = 0;
    return buf;
}

void buffer_free(KuBuffer *buf)
{
    if (buf->datas != NULL)
    {
        kufree(buf->datas);
        buf->datas = 0;
    }
    kufree(buf);
}
void buffer_resize(KuBuffer *buf, uint16_t size)
{
    uint16_t dataCount = buffer_dataCount(buf);
    uint8_t *p = &buf->datas[0];
    buf->size = size;
    buf->datas = (uint8_t *)kumalloc(size * sizeof(uint8_t));
    memcpy(&buf->datas[0], (p + buf->pos), buffer_dataCount(buf));
    kufree(p);
    buf->pos = 0;
    buf->count = dataCount;
}

int buffer_dataCount(KuBuffer *buf) { return buf->count - buf->pos; }
void buffer_put(KuBuffer *buf, uint8_t *datas, uint16_t length)
{
    if ((buf->count + length) > buf->size)
    {
        uint16_t dataCount = buffer_dataCount(buf);
        if (dataCount + length > buf->size)
        { // 需要调整大小
            buffer_resize(buf, dataCount + length);
        }
        else if (buf->pos > 0)
        { /* 将数据移到缓冲区头部 */
            memmove(&buf->datas[0], &buf->datas[buf->pos], dataCount);
            buf->pos = 0;
            buf->count = dataCount;
        }
    }
    memcpy(&buf->datas[buf->count], datas, length);
    buf->count += length;
}

uint8_t *buffer_get(KuBuffer *buf, int offset) { return &buf->datas[buf->pos + offset]; }
void buffer_remove(KuBuffer *buf, uint16_t length)
{
    uint16_t dataCount = buffer_dataCount(buf);
    length = (length > dataCount) ? dataCount : length;
    buf->pos += length;
}
void buffer_clear(KuBuffer *buf) { buf->pos = buf->count = 0; }
int buffer_find(KuBuffer *buf, int offset, uint8_t value)
{
    uint16_t dataCount = buffer_dataCount(buf);
    uint8_t *p = buffer_get(buf, 0);
    while (offset < dataCount)
    {
        if (*(p + offset) == value)
            return offset;
        offset++;
    }
    return -1;
}

int buffer_finds(KuBuffer *buf, int offset, uint8_t *values, uint16_t length)
{
    uint16_t dataCount = buffer_dataCount(buf);
    uint8_t *p = buffer_get(buf, 0);
    uint16_t i;
    while ((offset + length) < dataCount)
    {
        offset = buffer_find(buf, offset, values[0]);
        if (offset < 0)
            break;
        i = 0;
        while (1)
        {
            if (i == (length - 1))
                return offset;
            i++;
            if (*(p + offset + i) != values[i])
                break;
        }
        offset++;
    }
    return -1;
}
