
#include "include.h"


void Init_FifoQueue(Fifo_Qnode *Q)/*定义指针Q*/
{
	Fifo_Node *p;				/*定义指针p*/
	/*请使用if来判断,这是有必要的,分配结点字节的容量*/
	if((p =(Fifo_Node *)malloc(sizeof(Fifo_Node)))==NULL)
	{
		writelog(ZLOG_LEVEL_SYS,"分配上传队列空间出错，此处退出");
		exit(1);
	}
	Q->front=p;    						/*指定头指针p*/
	Q->front->next=NULL;				/*建立空队列*/ 
	Q->rear=Q->front;					/*改变Q的值*/	
	Q->Fifo_No=0;						/*队列成员数量统计*/
}

int FifoQueue_Push(Fifo_Qnode *Q,unsigned int cmd,char* datavalue)
{
	Fifo_Node *p; 
	/*请使用if来判断,这是有必要的,分配结点字节的容量*/
	if((p =(Fifo_Node *)malloc(sizeof(Fifo_Node)))==NULL)
	{
		writelog(ZLOG_LEVEL_SYS,"上传队列入队内存分配错误，退出");
		return -1;
	}
	p->Cmd=cmd;
	strcpy((char *)p->Data,datavalue);
	p->next=NULL;			/*指定尾结点*/
	Q->rear->next=p;		/*指定尾新结点p的地址*/
	Q->rear=p;				/*指定队尾结束*/
	Q->Fifo_No++;
	return 0;				/*程序结束*/
}
int FifoQueue_Pop(Fifo_Qnode *Q)
{
	Fifo_Node *p;						/*定义结点指针*/
	if(Q->front->next==NULL)			/*判断对前是否为空，如果是就结束*/
	{
		writelog(ZLOG_LEVEL_SYS,"上传队列是空的，无法出队，返回");
		return -1;
	}
	p=Q->front->next;					/*指向下以个成员*/
	Q->front->next=p->next;				/*依次向下循环*/
	if(Q->rear==p) Q->rear=Q->front;	/*队尾与对头相同*/
	//printf("The %d has been pop from the queue! \r\n",p->data);/*显示队列成员*/
	free(p);
	Q->Fifo_No--;
	return 0;
}


int JudgeQueueNull(Fifo_Qnode *Q)
{
	if(Q->front->next==NULL)/*判断对前是否为空，如果是就结束*/
	{
		writelog(ZLOG_LEVEL_SYS,"队列是空的，返回");
		return -1;
	}
	return 0;
}


