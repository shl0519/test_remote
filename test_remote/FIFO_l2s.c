
#include "include.h"


void Init_FifoQueue(Fifo_Qnode *Q)/*����ָ��Q*/
{
	Fifo_Node *p;				/*����ָ��p*/
	/*��ʹ��if���ж�,�����б�Ҫ��,�������ֽڵ�����*/
	if((p =(Fifo_Node *)malloc(sizeof(Fifo_Node)))==NULL)
	{
		writelog(ZLOG_LEVEL_SYS,"�����ϴ����пռ�����˴��˳�");
		exit(1);
	}
	Q->front=p;    						/*ָ��ͷָ��p*/
	Q->front->next=NULL;				/*�����ն���*/ 
	Q->rear=Q->front;					/*�ı�Q��ֵ*/	
	Q->Fifo_No=0;						/*���г�Ա����ͳ��*/
}

int FifoQueue_Push(Fifo_Qnode *Q,unsigned int cmd,char* datavalue)
{
	Fifo_Node *p; 
	/*��ʹ��if���ж�,�����б�Ҫ��,�������ֽڵ�����*/
	if((p =(Fifo_Node *)malloc(sizeof(Fifo_Node)))==NULL)
	{
		writelog(ZLOG_LEVEL_SYS,"�ϴ���������ڴ��������˳�");
		return -1;
	}
	p->Cmd=cmd;
	strcpy((char *)p->Data,datavalue);
	p->next=NULL;			/*ָ��β���*/
	Q->rear->next=p;		/*ָ��β�½��p�ĵ�ַ*/
	Q->rear=p;				/*ָ����β����*/
	Q->Fifo_No++;
	return 0;				/*�������*/
}
int FifoQueue_Pop(Fifo_Qnode *Q)
{
	Fifo_Node *p;						/*������ָ��*/
	if(Q->front->next==NULL)			/*�ж϶�ǰ�Ƿ�Ϊ�գ�����Ǿͽ���*/
	{
		writelog(ZLOG_LEVEL_SYS,"�ϴ������ǿյģ��޷����ӣ�����");
		return -1;
	}
	p=Q->front->next;					/*ָ�����Ը���Ա*/
	Q->front->next=p->next;				/*��������ѭ��*/
	if(Q->rear==p) Q->rear=Q->front;	/*��β���ͷ��ͬ*/
	//printf("The %d has been pop from the queue! \r\n",p->data);/*��ʾ���г�Ա*/
	free(p);
	Q->Fifo_No--;
	return 0;
}


int JudgeQueueNull(Fifo_Qnode *Q)
{
	if(Q->front->next==NULL)/*�ж϶�ǰ�Ƿ�Ϊ�գ�����Ǿͽ���*/
	{
		writelog(ZLOG_LEVEL_SYS,"�����ǿյģ�����");
		return -1;
	}
	return 0;
}


