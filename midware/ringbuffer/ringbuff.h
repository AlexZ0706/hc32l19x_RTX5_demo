/**
******************************************************************************

******************************************************************************
*/ 
#ifndef __RING_BUFF_H__
#define __RING_BUFF_H__

#include "stdbool.h"
#include "stdint.h"

#define RINGBUFF_OK            0
#define RINGBUFF_ERR          -1
#define RINGBUFF_EMPTY        -3
#define RINGBUFF_FULL         -4
#define RINGBUFF_TOO_SHORT    -5

typedef struct _ring_buff_
{
  uint32_t  size;
  uint32_t  readpoint;
  uint32_t  writepoint;
  char*  buffer;
  bool full;
} sRingbuff;

typedef sRingbuff*  ring_buff_t;

int ring_buff_init(sRingbuff* ring_buff, char* buff, uint32_t size );
int ring_buff_flush(sRingbuff* ring_buff);
int ring_buff_push_data(sRingbuff* ring_buff, uint8_t *pData, int len);
int ring_buff_pop_data(sRingbuff* ring_buff, uint8_t *pData, int len);
int ring_buff_get_len(sRingbuff* ring_buff);
#endif // __ringbuff_h__


