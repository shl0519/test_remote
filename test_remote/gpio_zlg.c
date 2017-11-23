/**********************************************************
* Copyright (c) 2013, 厦门市路桥信息工程有限公司
* All rights reserved.
*
* 文件名称: gpio_zlgm3517.c
* 文件标识: gpio_zlgm3517
* 摘要:
*
* 当前版本:
* 作者: 戴培君
* 修改日期:
* 修改地方描述:
***********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "gpio_zlg.h"

#define gpi_demo 0
#define gpo_demo 0
#define new_demo 0

#define judge 1

#define ZLGGPI_NUM 8
#define ZLGGPO_NUM 8


//M3517
//static char gpi[8][12] = {"154", "143", "65", "64", "150", "149", "156", "144"};
//static char gpo[8][12] = {"116", "117", "118", "119", "140", "141", "145", "142"};

//M3352
//static char gpo[8][12] = {"48","49", "50", "51", "52", "53", "54", "55"};
//_by_pbh_2014-11-07更改输出口的引脚排布方向，保证输入和输出IO口朝向一致
static char gpo[8][12] = {"55","54", "53", "52", "51", "50", "49", "48"};
static char gpi[8][12] = {"56", "57", "58", "59", "13", "12", "41", "40"};

int zlgGPIO_Del(void);

int zlgGPIO_Init(void)
{
	int i = -1;
	char buf[256];

	/* 需要删除原来建立的IO口 */
	zlgGPIO_Del();

	/* 建立输入口 */
	for( i = 0; i < ZLGGPI_NUM; i++)
	{
		memset(buf, 0, sizeof(buf));
		strcat(buf, "echo ");
		strcat(buf, gpi[i]);
		strcat(buf, " > /sys/class/gpio/export");
//		printf("buf is: %s\n", buf);
		system(buf);

		memset(buf, 0, sizeof(buf));
		strcat(buf, "echo in > ");
		strcat(buf, "/sys/class/gpio/gpio");
		strcat(buf, gpi[i]);
		strcat(buf, "/direction");
//		printf("buf is: %s\n", buf);
		system(buf);
	}

	/* 建立输出口并初始化输出0 */
	for( i = 0; i < ZLGGPO_NUM; i++)
	{
		memset(buf, 0, sizeof(buf));
		strcat(buf, "echo ");
		strcat(buf, gpo[i]);
		strcat(buf, " > /sys/class/gpio/export");
//		printf("buf is: %s\n", buf);
		system(buf);

		memset(buf, 0, sizeof(buf));
		strcat(buf, "echo out > ");
		strcat(buf, "/sys/class/gpio/gpio");
		strcat(buf, gpo[i]);
		strcat(buf, "/direction");
//		printf("buf is: %s\n", buf);
		system(buf);

		memset(buf, 0, sizeof(buf));
		strcat(buf, "echo 0 > ");
		strcat(buf, "/sys/class/gpio/gpio");
		strcat(buf, gpo[i]);
		strcat(buf, "/value");
//		printf("buf is %s\n", buf);
		system(buf);
	}

	return 0;
}

int zlgGPO_Ctrl(unsigned char outPortNO,int outputStat)
{
	char buf[256];
	memset(buf, 0, sizeof(buf));
	strcat(buf, "echo ");

	if(outputStat == 0 )
	{
		strcat(buf, "0");
	}
	else if(outputStat == 1)
	{
		strcat(buf, "1");
	}
	strcat(buf, " > ");
	strcat(buf, "/sys/class/gpio/gpio");
	strcat(buf, gpo[outPortNO]);
	strcat(buf,"/");
	strcat(buf, "value");
	//printf("buffer is %s\n", buffer);
	system(buf);
	return 0;
}

int zlgGPI_GetStat(unsigned char inPortNO)
{
	char buf[256];
	FILE *fp;

	memset(buf, 0, sizeof(buf));
	strcat(buf, "/sys/class/gpio/gpio");
	strcat(buf, gpi[inPortNO]);
	strcat(buf, "/value");

	if( (fp = fopen(buf, "r")) == NULL )
	{
		writelog(ZLOG_LEVEL_SYS,"The %s was not exist,Error:%s,errno:%d",buf,strerror(errno),errno);
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	fgets(buf, sizeof(buf)+1, fp);
	//printf("buf is %s\n", buf);
	fclose(fp);
	return atoi(buf);;
}

int zlgGPIO_Del(void)
{
	int i = -1;
	char buf[256];

	for( i = 0; i < ZLGGPI_NUM; i++)
	{
		memset(buf, 0, sizeof(buf));
		strcat(buf, "echo ");
		strcat(buf, gpi[i]);
		strcat(buf, " > /sys/class/gpio/unexport");
//		printf("buf: is %s\n", buf);
		system(buf);
	}

	for( i = 0; i < ZLGGPO_NUM; i++)
	{
		memset(buf, 0, sizeof(buf));
		strcat(buf, "echo ");
		strcat(buf, gpo[i]);
		strcat(buf, " > /sys/class/gpio/unexport");
//		printf("buf: is %s\n", buf);
		system(buf);
	}

	return 0;
}
/**************************************************************
**修改文件时间
**
**
***************************************************************/
int zlgChg_LocTime(char *timeValue)
{
	char buf[128];
	char buf_time[32];
	int i=0;
	int len=0;

	memset(buf, 0, sizeof(buf));
	memset(buf_time, 0, sizeof(buf_time));
	strcpy(buf_time,timeValue);

	len=strlen(buf_time);

	for(i=0; i<len; i++) /* 符合通信协议格式 */
	{
		if(buf_time[i] == '-')
		{
			buf_time[i]='.';
		}
		else if(buf_time[i] == ' ')
		{
			buf_time[i]='-';
		}
	}

	strcat(buf, "date ");
	//strcat(buf, "");
	strcat(buf, buf_time);
	//strcat(buf, ".");
	//printf("xxxxxxxxxxxx:%s\r\n",buf);
	system(buf);
	usleep(10000);
	system("hwclock -w");

	return 0;

}




#if 0
int main()
{
	GPIO_Delete();
	GPIO_Create();
	GPIO_PORT_INIT(ZLG_PORT0, GPIO_OUTPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT1, GPIO_OUTPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT2, GPIO_OUTPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT3, GPIO_OUTPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT4, GPIO_OUTPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT5, GPIO_OUTPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT6, GPIO_OUTPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT7, GPIO_OUTPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT8, GPIO_INPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT9, GPIO_INPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT10, GPIO_INPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT11, GPIO_INPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT12, GPIO_INPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT13, GPIO_INPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT14, GPIO_INPUT, 0);
	GPIO_PORT_INIT(ZLG_PORT15, GPIO_INPUT, 0);

#if gpi_demo
	for( ; ; )
	{
		WORD32 statue_0 = 1;
		WORD32 statue_1 = 1;
		WORD32 statue_2 = 1;
		WORD32 statue_3 = 1;
		WORD32 statue_4 = 1;
		WORD32 statue_5 = 1;
		WORD32 statue_6 = 1;
		WORD32 statue_7 = 1;

		statue_0 = GPIO_GET_INPUTSTAT(ZLG_PORT8);
		statue_1 = GPIO_GET_INPUTSTAT(ZLG_PORT9);
		statue_2 = GPIO_GET_INPUTSTAT(ZLG_PORT10);
		statue_3 = GPIO_GET_INPUTSTAT(ZLG_PORT11);
		statue_4 = GPIO_GET_INPUTSTAT(ZLG_PORT12);
		statue_5 = GPIO_GET_INPUTSTAT(ZLG_PORT13);
		statue_6 = GPIO_GET_INPUTSTAT(ZLG_PORT14);
		statue_7 = GPIO_GET_INPUTSTAT(ZLG_PORT15);

		printf("	%d	  ",statue_0);
		printf("%d	  ",statue_1);
		printf("%d	  ",statue_2);
		printf("%d	  ",statue_3);
		printf("%d	  ",statue_4);
		printf("%d	  ",statue_5);
		printf("%d	  ",statue_6);
		printf("%d	  \n",statue_7);

		sleep(1);

	}
#endif

#if gpo_demo
	int iport = 8;
	int iswitch = -1;

	for(;;)
	{
		printf("请输入命令格式以控制相应的GPO,例如0 1\n");
		printf("0表示输出端口号(0-7), 1表示有效控制(0/1)\n");
		scanf("%d%d", &iport, &iswitch);

		GPIO_RELAY_CONTROL(iport, iswitch);
	}
#endif

#if new_demo
		int i = -1;
		int st = -1;
		struct timeval tpstart, tpend;
		memset(&tpstart, 0, sizeof(tpstart));
		memset(&tpend, 0, sizeof(tpend));
		float timeuse = 0;

		printf("开始测试一个循环!\n");
		gettimeofday(&tpstart,NULL);
		GPIO_RELAY_CONTROL(0, 1);
		while(1)
		{
			st = GPIO_GET_INPUTSTAT(ZLG_PORT8);
			printf("st is %d\n", st);
			if( st == 0 )
			{
				break;
			}
		}
		gettimeofday(&tpend,NULL);
		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+ tpend.tv_usec-tpstart.tv_usec;
		timeuse/=1000000;
			printf("Used Time:%f(s)\n",timeuse);

		GPIO_RELAY_CONTROL(0, 0);
#endif

#if judge
	LogInit();
	In_LogQueue(logqueue, "开始监测GPI\n", 9999, NULL, NULL, 9999, LOG_DEBUG);
	LogThread();
	GPOjudgeThread();
	GPIjudgeThread();

	while(1)
	{
		;
	}
	return 0;

#endif

	GPIO_Delete();

	return 0;
}

void GPOjudgeThread(void)
{
	int result;
	pthread_t thread_gpojudge_id;

	result = pthread_create(&thread_gpojudge_id, NULL, (void *) thread_gpojudge, NULL );
	if( 0 != result )
	{
		printf("[main]:create the gpo thread is failed!\n");
	}
	else
	{
		printf("[main]:create the gpo thread is success!\n");
	}
	return;
}

void thread_gpojudge(void)
{
	int result;
	int xtnport = 0;

	while(1)
	{
		GPIO_RELAY_CONTROL(xtnport, 1);
		sleep(1);
		GPIO_RELAY_CONTROL(xtnport, 0);
		xtnport++;
		if( xtnport >= 8)
		{
			xtnport = 0;
		}
	}
	return;
}

void GPIjudgeThread(void)
{
	int result;
	pthread_t thread_gpijudge_id;

	result = pthread_create(&thread_gpijudge_id, NULL, (void *) thread_gpijudge, NULL );
	if( 0 != result )
	{
		printf("[main]:create the gpo thread is failed!\n");
	}
	else
	{
		printf("[main]:create the gpo thread is success!\n");
	}
	return;
}

void thread_gpijudge(void)
{
	int result;

	while(1)
	{
		unsigned int statue_0 = 1;
		unsigned int statue_1 = 1;
		unsigned int statue_2 = 1;
		unsigned int statue_3 = 1;
		unsigned int statue_4 = 1;
		unsigned int statue_5 = 1;
		unsigned int statue_6 = 1;
		unsigned int statue_7 = 1;
		statue_0 = GPIO_GET_INPUTSTAT(0);
		statue_1 = GPIO_GET_INPUTSTAT(1);
		statue_2 = GPIO_GET_INPUTSTAT(2);
		statue_3 = GPIO_GET_INPUTSTAT(3);
		statue_4 = GPIO_GET_INPUTSTAT(4);
		statue_5 = GPIO_GET_INPUTSTAT(5);
		statue_6 = GPIO_GET_INPUTSTAT(6);
		statue_7 = GPIO_GET_INPUTSTAT(7);

		char buffer[256];
		if( (statue_0 == 0) || (statue_1 == 0) || (statue_2 ==0)
			|| (statue_3 == 0) || (statue_4 == 0) || (statue_5 == 0)
			|| (statue_6 == 0) || (statue_7 == 0))
		{
			sprintf(buffer, "	 %d    %d	 %d    %d	 %d    %d	 %d    %d	 \n", statue_0, statue_1, statue_2, statue_3, statue_4, statue_5, statue_6, statue_7);
			printf("buffer is %s\n", buffer);
			In_LogQueue(logqueue, buffer, 9999, NULL, NULL, 9999, LOG_DEBUG);
		}
	}
	return;
}
#endif

