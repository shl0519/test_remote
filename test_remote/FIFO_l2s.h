#ifndef _FIFO_l2s_h
#define _FIFO_l2s_h

#include "typedef_usr.h"

extern void Init_FifoQueue(Fifo_Qnode *Q);/*∂®“Â÷∏’ÎQ*/
extern int FifoQueue_Push(Fifo_Qnode *Q,unsigned int cmd,char* datavalue);
extern int FifoQueue_Pop(Fifo_Qnode *Q);


#endif

