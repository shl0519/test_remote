

#include "include.h"




//#define RsEdgeFiTimes 250		//上升沿过滤次数
//#define TrEdgeFiTimes 25		//下降沿过滤次数
//#define JJH_GPIO_SIP "192.168.0.81"		
//#define JJH_GPIO_SPORT 502
static int l2netgpio_socket[2]={0,0};/* 客户端socket 句柄值*/

int Get_NetGPIO_GPIStat(int *client_socket);
int L2NetGpio_SendRecv(int *client_socket,char *cmd,int cmdlen,char *res,int reslen);
int netGpio_init(void);
unsigned char read_netgpi(void);

int L2NetGpio_SendRecv(int *client_socket,char *cmd,int cmdlen,char *res,int reslen)
{
	//-----send command or data
	
	struct timeval tv_send;
	tv_send.tv_sec = SENDOT;
	tv_send.tv_usec = 0;
	setsockopt(*client_socket, SOL_SOCKET, SO_SNDTIMEO , &tv_send, sizeof(tv_send));
	if(send(*client_socket,cmd,cmdlen,0)<0)
	{
		writelog(ZLOG_LEVEL_TCPIP,"发送到网络IO板超时!!");
		return -1;
	}	
	//-----waitting for the replay
	struct timeval tv_recv;
	tv_recv.tv_sec = RECEOT;
	tv_recv.tv_usec = 0;
	setsockopt(*client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv_recv, sizeof(tv_recv));
	if(recv(*client_socket,res,reslen,0)<0)	//等待接收超时
	{
		writelog(ZLOG_LEVEL_TCPIP,"接收网络IO超时!!");
		return -1;
	}
	return 0;
}


int Get_NetGPIO_GPIStat(int *client_socket)
{
	char cmd[12];
	char res[12];
	memset(res, 0, sizeof(res));

	cmd[0] = 0x00;
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	cmd[4] = 0x00;

	cmd[5] = 0x06;
	cmd[6] = 0x01;
	cmd[7] = 0x01;

	cmd[8] = 0x00;
	cmd[9] = 0x0A;

	cmd[10] =0x00;
	cmd[11] =0x00;		//no important or important

/*
	printf("cmd is 0x%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n", cmd[0], cmd[1],
		cmd[2],cmd[3], cmd[4],cmd[5],cmd[6], cmd[7],
		cmd[8],cmd[9], cmd[10],cmd[11]);
*/
	if(L2NetGpio_SendRecv(client_socket,cmd,sizeof(cmd),res,sizeof(res))!=0)
	{
		writelog(ZLOG_LEVEL_TCPIP,"网络IO板异常!!");
		return -1;
	}
/*
	printf("res is 0x%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n", res[0], res[1],
	res[2],res[3], res[4],res[5],res[6], res[7],
	res[8],res[9], res[10],res[11]);
*/	
	int i;
	for(i=0;i<5;i++)		/* 核查返回值 */
	{
		if(res[i] == 0x00)continue;
		else
		{
			writelog(ZLOG_LEVEL_TCPIP,"网络IO板接收信息有误0");
			return -1;
		}
	}
	if(res[5] != 0x04)		/* 核查返回值 */
	{
		writelog(ZLOG_LEVEL_TCPIP,"网络IO板接收信息有误1");
		return -1;
	}
	for(i=6;i<9;i++)		/* 核查返回值 */
	{
		if(res[i] == 0x01)continue;
		else
		{
			writelog(ZLOG_LEVEL_TCPIP,"网络IO板接收信息有误2");
			return -1;
		}
	}

	//printf("net gpio get gpi state is: 0x%x\r\n", res[9]);
	return res[9];
}

int Ctrl_NetGPIO_GPOStat(int *client_socket,unsigned char Port, unsigned int Switch)
{

	char cmd[12];
	char res[12];
	memset(res, 0, sizeof(res));

	cmd[0] = 0x00;
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	cmd[4] = 0x00;

	cmd[5] = 0x06;
	cmd[6] = 0x01;
	cmd[7] = 0x05;

	cmd[8] = 0x00;
	Port = Port + 0x1D;
	cmd[9] = Port;

	if( Switch == 1 )
	{
		cmd[10] = 0xFF;
	}
	else if( Switch == 0 ){
		cmd[10] =0x00;
	}
	cmd[11] = 0x00;
	/*
	printf("cmd is 0x%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n", cmd[0], cmd[1],
		cmd[2],cmd[3], cmd[4],cmd[5],cmd[6], cmd[7],
		cmd[8],cmd[9], cmd[10],cmd[11]);
	*/


	if(L2NetGpio_SendRecv(client_socket,cmd,sizeof(cmd),res,sizeof(res))!=0)
	{
		writelog(ZLOG_LEVEL_TCPIP,"网络IO板异常!!");
		return -1;
	}
	int i;
	/*
	printf("res is 0x%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n", res[0], res[1],
	res[2],res[3], res[4],res[5],res[6], res[7],
	res[8],res[9], res[10],res[11]);
	*/
	
	for(i=0;i<11;i++)		/* 核查返回值 */
	{
		if(cmd[i]==res[i])continue;
		else
		{
			writelog(ZLOG_LEVEL_TCPIP,"网络IO板接收信息有误!!");
			return -1;
		}
	}
	return 0;
}

int netGpio_init(void)
{
	int j = -1;
	if(SysCfg.Lane[0].NetGpioCfg==HAVECFG)
		j=client_connect(SysCfg.NET_GPIO[0].IP,SysCfg.NET_GPIO[0].SP1,&(l2netgpio_socket[0]));
	if(j!=0)return -1;

	if(SysCfg.Lane[1].NetGpioCfg==HAVECFG)
		j=client_connect(SysCfg.NET_GPIO[1].IP,SysCfg.NET_GPIO[1].SP1,&(l2netgpio_socket[1]));
	if(j!=0)return -1;

	return 0;
}

unsigned char read_netgpi(void)
{
	unsigned char j=0,k=0;
	int m=0;
	if(SysCfg.Lane[0].NetGpioCfg==HAVECFG)
	{
		m=Get_NetGPIO_GPIStat(&(l2netgpio_socket[0]));
		if(m<0)
		{
			close_client(&(l2netgpio_socket[0]));
			//sleep(300);		//_by_dpj_2014-07-30
			sleep(1);		//_by_dpj_2014-07-30
			client_connect(SysCfg.NET_GPIO[0].IP,SysCfg.NET_GPIO[0].SP1,&(l2netgpio_socket[0]));
			m=Get_NetGPIO_GPIStat(&(l2netgpio_socket[0]));
		}
		//printf("kkkkkkk0000000:0x%x\r\n",j);
		j=m&0x000f;
	}
	if(SysCfg.Lane[1].NetGpioCfg==HAVECFG)
	{
		m=Get_NetGPIO_GPIStat(&(l2netgpio_socket[1]));
		if(m<0)
		{
			close_client(&(l2netgpio_socket[1]));
			//sleep(300);		//_by_dpj_2014-07-30
			sleep(1);		//_by_dpj_2014-07-30
			j=client_connect(SysCfg.NET_GPIO[1].IP,SysCfg.NET_GPIO[1].SP1,&(l2netgpio_socket[1]));
			m=Get_NetGPIO_GPIStat(&(l2netgpio_socket[1]));
		}

		//printf("kkkkkkk1111111:0x%x\r\n",j);
		k=m&0x000f;
	}
	j=j|(k<<4);
	return j;
}

void write_netgpo(unsigned char value)
{
	int j=0;
	int i=0;
	unsigned char l;
	unsigned char k=0;
	if(SysCfg.Lane[0].NetGpioCfg==HAVECFG)
	{
		l=value&0x0f;
		for(i=0;i<4;i++)
		{
			k=(l>>i)&0x01;
			j=Ctrl_NetGPIO_GPOStat(&(l2netgpio_socket[0]),i+1,k);
			if(j<0)
			{
				close_client(&(l2netgpio_socket[0]));
				//sleep(300);		//_by_dpj_2014-07-30
				sleep(1);		//_by_dpj_2014-07-30
				client_connect(SysCfg.NET_GPIO[0].IP,SysCfg.NET_GPIO[0].SP1,&(l2netgpio_socket[0]));
			}
		}
	}

	if(SysCfg.Lane[1].NetGpioCfg==HAVECFG)
	{
		l=value>>4;
		for(i=1;i<4;i++)
		{
			k=(l>>i)&0x01;
			j=Ctrl_NetGPIO_GPOStat(&(l2netgpio_socket[1]),i+1,k);
			if(j<0)
			{
				close_client(&(l2netgpio_socket[1]));
				//sleep(300);		//_by_dpj_2014-07-30
				sleep(1);		//_by_dpj_2014-07-30
				client_connect(SysCfg.NET_GPIO[1].IP,SysCfg.NET_GPIO[1].SP1,&(l2netgpio_socket[1]));
			}
			
		}
	}
}


