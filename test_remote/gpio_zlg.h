/**********************************************************
* Copyright (c) 2013, ������·����Ϣ�������޹�˾
* All rights reserved.
*
* �ļ�����: gpio_zlgm3352_h
* �ļ���ʶ: gpio_zlgm3352_h
* ժҪ:
*
* ��ǰ�汾:
* ����: �����
* �޸�����:
* �޸ĵط�����:
***********************************************************/
#ifndef _gpio_zlg_h
#define _gpio_zlg_h
#include "include.h"

extern int zlgGPIO_Init(void);
extern int zlgGPIO_Del(void);
extern int zlgGPO_Ctrl(unsigned char outPortNO,int outputStat);
extern int zlgGPI_GetStat(unsigned char inPortNO);
extern int zlgChg_LocTime(char *timeValue);

#endif

