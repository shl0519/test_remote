/**********************************************************
* Copyright (c) 2013, 厦门市路桥信息工程有限公司
* All rights reserved.
*
* 文件名称: gpio_zlgm3352_h
* 文件标识: gpio_zlgm3352_h
* 摘要:
*
* 当前版本:
* 作者: 戴培君
* 修改日期:
* 修改地方描述:
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

