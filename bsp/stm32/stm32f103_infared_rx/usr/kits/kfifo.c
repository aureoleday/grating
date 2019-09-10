/*
 * kfifo.c
 *
 *  Created on: 2019��3��11��
 *      Author: Administrator
 */

/*
 * =====================================================================================
 *       Copyright (c), 2013-2020, Jz.
 *       Filename:  ringbuf.c λ��KDIR/kernel/kfifo.c
 *
 *    Description:  ���λ�������ʵ��
 *         Others:  1.min�����ã�(��֤ʣ����Ч�ռ��Ҫ��Ҫ��������д��ռ� ȡ��Сֵ)
 *                  2.����unsigned int �Ļػ�,in �� outһֱ�ڼӣ��ӵ�0xffffffff���Ϊ0����Ȼ�������ƫ�Ƶȡ�
 *                  3.��Ϊ2������copy��һΪ��ǰƫ�Ƶ�size-1 ��Ϊʣ�ಿ��0��(len��ȥһ�еĸ���)
 *                  4.unsiged int�µ�(in - out)ʼ��Ϊin��out֮��ľ��룬(in�����in:0x1 - out:0xffffffff = 2��Ȼ����)(��������δ�������).
 *                  5.����ƫ��(in) & (size - 1) <==> in%size
 *        Version:  1.0
 *        Date:  Monday, April 28, 2016 16:00:53 HKT
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joy (Joy),
 *   Organization:  jz
 *        History:   Created by housir
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kfifo.h"

#define min(a,b)  ((a)>(b) ? (b) : (a))            /*  */

#define roundup_pow_of_two(n)   \
    (1UL    <<                             \
        (                                  \
            (                              \
            (n) & (1UL << 31) ? 31 :       \
            (n) & (1UL << 30) ? 30 :       \
            (n) & (1UL << 29) ? 29 :       \
            (n) & (1UL << 28) ? 28 :       \
            (n) & (1UL << 27) ? 27 :       \
            (n) & (1UL << 26) ? 26 :       \
            (n) & (1UL << 25) ? 25 :       \
            (n) & (1UL << 24) ? 24 :       \
            (n) & (1UL << 23) ? 23 :       \
            (n) & (1UL << 22) ? 22 :       \
            (n) & (1UL << 21) ? 21 :       \
            (n) & (1UL << 20) ? 20 :       \
            (n) & (1UL << 19) ? 19 :       \
            (n) & (1UL << 18) ? 18 :       \
            (n) & (1UL << 17) ? 17 :       \
            (n) & (1UL << 16) ? 16 :       \
            (n) & (1UL << 15) ? 15 :       \
            (n) & (1UL << 14) ? 14 :       \
            (n) & (1UL << 13) ? 13 :       \
            (n) & (1UL << 12) ? 12 :       \
            (n) & (1UL << 11) ? 11 :       \
            (n) & (1UL << 10) ? 10 :       \
            (n) & (1UL <<  9) ?  9 :       \
            (n) & (1UL <<  8) ?  8 :       \
            (n) & (1UL <<  7) ?  7 :       \
            (n) & (1UL <<  6) ?  6 :       \
            (n) & (1UL <<  5) ?  5 :       \
            (n) & (1UL <<  4) ?  4 :       \
            (n) & (1UL <<  3) ?  3 :       \
            (n) & (1UL <<  2) ?  2 :       \
            (n) & (1UL <<  1) ?  1 :       \
            (n) & (1UL <<  0) ?  0 : -1    \
            ) + 1                          \
        )                                  \
)
/*  */

/**
 * @brief
 *
 * @param fifo    �����������ݽṹ
 * @param buffer
 * @param size    �������Ĵ�С
 */
void kfifo_init(struct kfifo *fifo, void *buffer, unsigned int size)
{
    /* ���ʱ����2��ָ���� */
    fifo->buffer = buffer;
    fifo->size = size;

    fifo->in = fifo->out = 0;
}

void kfifo_reset(struct kfifo *fifo)
{
    fifo->in = fifo->out = 0;
}

/**
 * @brief
 *
 * @param fifo
 * @param size
 *
 * @return
 */
int kfifo_alloc(struct kfifo *fifo, unsigned int size/*, gfp_t gfp_mask*/)
{
    unsigned char *buffer;
    /* ���ʱ����2��ָ����  */
    if (size & (size-1))
    {
        //        size = 1UL << (roundup_pow_of_two(size) + 1);
        size = roundup_pow_of_two(size);
    }
    buffer = (unsigned char *)malloc(size);
    if (!buffer)
    {
        kfifo_init(fifo, NULL, 0);
        return -1;
    }

    kfifo_init(fifo, buffer, size);

    return 0;
}

/**
 * @brief ���뻺�����
 *
 * @param fifo
 * @param from
 * @param len
 *
 * @return ʵ�ʷ��뻺�����ĳ���(����С��len����Ϊ0)
 */
unsigned int kfifo_in(struct kfifo *fifo, const void *from,
        unsigned int len)
{
    unsigned int l,off;

    /* min(��С - �Ѿ����˵� , len) ����ʣ����ÿռ� �ô��� */
    len = min(fifo->size - (fifo->in - fifo->out), len);
    /*��������(0 ... size-1)*/
    off = (fifo->in + 0) & (fifo->size - 1);/*==> (in+0) % size*/
    /* �ֳ�2��,1.��ǰλ�õ�size-1 2.0��0����0��len-l(ʣ���) */
    l = min(len, fifo->size - off);
    memcpy(fifo->buffer + off, from, l);

    /* ��ʣ���copy��buffer�����len == l��ɶҲ����,�ô��� */
    memcpy(fifo->buffer, (char *)from + l, len - l);

    fifo->in += len;

    return len;
}

/**
 * @brief
 *
 * @param fifo
 * @param to
 * @param len
 *
 * @return ʵ�ʶ����������ĳ���(����С��len,����Ϊ0)
 */
unsigned int kfifo_out(struct kfifo *fifo, void *to, unsigned int len)
{
    unsigned int off;
    unsigned int l;
    /* ��֤len�Ƿ���ڻ��������ֵ  */
    len = min(fifo->in - fifo->out, len);

    off = (fifo->out + 0) & (fifo->size - 1);/*==> (out+0) % size*/
    /* first get the data from fifo->out until the end of the buffer */
    l = min(len, fifo->size - off);
    memcpy(to, fifo->buffer + off, l);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy((char *)to + l, fifo->buffer, len - l);

    fifo->out += len;

    return len;
}

unsigned int kfifo_out_peek(struct kfifo *fifo, void *to, unsigned int len)
{
    unsigned int off;
    unsigned int l;
    /* ��֤len�Ƿ���ڻ��������ֵ  */
    len = min(fifo->in - fifo->out, len);

    off = (fifo->out + 0) & (fifo->size - 1);/*==> (out+0) % size*/
    /* first get the data from fifo->out until the end of the buffer */
    l = min(len, fifo->size - off);
    memcpy(to, fifo->buffer + off, l);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy((char *)to + l, fifo->buffer, len - l);

    return len;
}

unsigned int kfifo_len(struct kfifo *fifo)
{
    return fifo->in - fifo->out;
}
