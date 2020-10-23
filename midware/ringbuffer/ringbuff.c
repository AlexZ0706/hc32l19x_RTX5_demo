/**
******************************************************************************

******************************************************************************
*/ 
#include <stdio.h>
#include "ringbuff.h"
#include "string.h "

int ring_buff_init(sRingbuff* ring_buff, char* buff, uint32_t size )
{
	ring_buff->buffer     = buff;
    ring_buff->size       = size;
    ring_buff->readpoint  = 0;
    ring_buff->writepoint = 0;
	memset(ring_buff->buffer, 0, ring_buff->size);
	ring_buff->full = false;
	
    return RINGBUFF_OK;
}

int ring_buff_flush(sRingbuff* ring_buff)
{
    ring_buff->readpoint  = 0;
    ring_buff->writepoint = 0;
	memset(ring_buff->buffer, 0, ring_buff->size);
	ring_buff->full = false;
	
    return RINGBUFF_OK;
}

int ring_buff_push_data(sRingbuff* ring_buff, uint8_t *pData, int len)
{
	int i;

	if(len > ring_buff->size)
	{
		return RINGBUFF_TOO_SHORT;
	}

	for(i = 0; i < len; i++)
	{
		if(((ring_buff->writepoint + 1) % ring_buff->size) == ring_buff->readpoint)
	    {
	    	ring_buff->full = true;
	        return RINGBUFF_FULL;
	    }
	    else
	    {
	        if(ring_buff->writepoint < (ring_buff->size - 1)) 
	        {
				ring_buff->writepoint ++;
			}         
	        else
	        {
				ring_buff->writepoint = 0;
			}	            
			ring_buff->buffer[ring_buff->writepoint] = pData[i];
	    }
	}

	return RINGBUFF_OK;
}

int ring_buff_pop_data(sRingbuff* ring_buff, uint8_t *pData, int len)
{
	int i;

	if(len > ring_buff->size)
	{
		 return RINGBUFF_TOO_SHORT;
	}

	for(i = 0; i < len; i++)
	{
		if(ring_buff->writepoint == ring_buff->readpoint)
		{
			break;
		}
		else
		{
			if(ring_buff->readpoint == (ring_buff->size - 1))
			{
				ring_buff->readpoint = 0;				
			}

			else
			{
				ring_buff->readpoint++;
			}
			pData[i] = ring_buff->buffer[ring_buff->readpoint];
		}
	}
	
	return i;
}

int ring_buff_get_len(sRingbuff* ring_buff)
{
    int len = 0;
    
    if(ring_buff->writepoint >= ring_buff->readpoint)
    {
        len = ring_buff->writepoint - ring_buff->readpoint;
    }
    else
    {
        len = ring_buff->writepoint + ring_buff->size - ring_buff->readpoint;
    }
    return len;
}
