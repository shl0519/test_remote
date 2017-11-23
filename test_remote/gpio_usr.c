/*********************************************************************************************
#####         厦门路桥信息工程有限公司        #####
#####              嵌入式开发小组  		      #####
#####                        				  #####

* File：	usergpio_test.c  (s3c2440_user_gpio.c)
* Author:	Hailiang.su
* Desc：	user gpio test
* History:	May 11th 2012
*********************************************************************************************/


#include "include.h"
#include "uhfshell_usr.h"

void VehicleDetector(void);
void VehicleDetectorComm(bool VDState);

void FreeLocked(void);		//_by_dpj_2013-07-09

/* ����IO��д */
unsigned char const GPO_H[9]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0xFF};
unsigned char const GPO_L[9]={0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F,0x00};

const gponame GPO[8]={GPO1,GPO2,GPO3,GPO4,GPO5,GPO6,GPO7,GPO8};
const gpiname GPI[8]={GPI1,GPI2,GPI3,GPI4,GPI5,GPI6,GPI7,GPI8};

GPI_StatAndTime GPIStat[8];

char strlocaltime_VD[30];
static unsigned char gpo_reg=0;
laneclass_type LaneClass[2];

char s1[32];
char s2[32];

//===========================================================
//��ȡGPI�ӿ�

unsigned char gpi_read(void)
{
	int i;
	unsigned char j,k;
	j=0;
	k=0;
#ifdef _ZTE6600C2
	for(i=0;i<5;i++){			/* ��ȡ��������� */
		j=GPIO_GET_INPUTSTAT(i);
		k=k|(j<<i);
	}
	//printf("xxxxxxxxxxxxxxx:%x\r\n",k);
	unsigned char m=0;
	if((SysCfg.Lane[0].NetGpioCfg==NOCFG)		/* ���ӵ�����IO�壬��ȡ����״̬������ͬʱ������������ */
		&&(SysCfg.Lane[1].NetGpioCfg==NOCFG))
		return k;

	else if((SysCfg.Lane[0].NetGpioCfg==HAVECFG)/* ����0��������IO�壬��ô����0��GPIֵ�����4λ */
		&&(SysCfg.Lane[1].NetGpioCfg==NOCFG))
	{
		m= ~ read_netgpi();						//�͵�ƽ��Ч��������IO�Ķ�ȡ����෴
		k=((k<<4)&0xf0) | (m&0x0f);
		//printf("xxxxxxxxxxxxxxx:%x\r\n",k);
		return k;
	}

	else if((SysCfg.Lane[0].NetGpioCfg==NOCFG)	/* ����1��������IO�壬��ô����1��GPIֵ�����4λ */
		&&(SysCfg.Lane[1].NetGpioCfg==HAVECFG))
	{
		m = ~ read_netgpi();
		k=(k&0x0f) | (m&0xf0);
		return k;
	}

	else if((SysCfg.Lane[0].NetGpioCfg==HAVECFG)/* �����������ã�ֱ�ӷ��� */
		&&(SysCfg.Lane[1].NetGpioCfg==HAVECFG))
	{
		m= ~ read_netgpi();
		return m;
	}

	return 0;
#else
	#ifdef _ZLGM3352
	for(i=0;i<8;i++)		//_by_dpj_2014-10-20
	{
		j=zlgGPI_GetStat(i);
		k=k|(j<<i);
	}
	return k;
	#endif
#endif


}
//===========================================================
int gpo_write(unsigned char WriteGPIO_Date)
{
	unsigned char writep=0;
	writep=WriteGPIO_Date;

#ifdef _ZTE6600C2
	if((SysCfg.Lane[0].NetGpioCfg==NOCFG)		/* ���ӵ�����IO�壬�������������ͬʱ������������ */
		&&(SysCfg.Lane[1].NetGpioCfg==NOCFG))
	{
		int i;
		for(i=0;i<2;i++){
			if((writep&0x01)==1)GPIO_RELAY_CONTROL(i,1);
			else GPIO_RELAY_CONTROL(i,0);
			writep=writep>>1;
		}
	}

	else if((SysCfg.Lane[0].NetGpioCfg==HAVECFG)
		&&(SysCfg.Lane[1].NetGpioCfg==NOCFG))
	{
		unsigned char j=0;
		j=writep&0x0f;
		write_netgpo(j);

		j=writep>>4;
		int i;
		for(i=0;i<2;i++){
			if((j&0x01)==1)GPIO_RELAY_CONTROL(i,1);
			else GPIO_RELAY_CONTROL(i,0);
			j=j>>1;
		}
	}

	else if((SysCfg.Lane[0].NetGpioCfg==NOCFG)
		&&(SysCfg.Lane[1].NetGpioCfg==HAVECFG))
	{
		unsigned char j=0;
		j=writep&0xf0;
		write_netgpo(j);

		j=writep&0x0f;
		int i;
		for(i=0;i<2;i++){
			if((j&0x01)==1)GPIO_RELAY_CONTROL(i,1);
			else GPIO_RELAY_CONTROL(i,0);
			j=j>>1;
		}
	}

	else if((SysCfg.Lane[0].NetGpioCfg==HAVECFG)
		&&(SysCfg.Lane[1].NetGpioCfg==HAVECFG))
	{
		write_netgpo(writep);
	}
#else
	#ifdef _ZLGM3352
	int i;
	for(i=0;i<8;i++)
	{
		if((writep&0x01)==1)zlgGPO_Ctrl(i,1);
		else zlgGPO_Ctrl(i,0);
		writep=writep>>1;
	}
	#endif
#endif

	return 0;
}

/******************************************************************************/
/* ����ⲿ�̽ӣ�ͨ��Ӳ���ṹ������0��оƬ */
/* ����ⲿ�Ͽ���ͨ��Ӳ���ṹ������1��оƬ */
/* ��̬�ǶϿ����г�ѹ��Ȧʱ���ⲿ��̽� */
/******************************************************************************/
void ReadGPI_Process(unsigned char gpi_bool,GPI_StatAndTime *gpi_n)
{
	gpi_n->LState=gpi_n->State;
	if(gpi_bool==0)
	{
		gpi_n->Thistime=HIGHTLEVEL;
	}
	else gpi_n->Thistime=LOWLEVEL;
	if(gpi_n->Thistime != gpi_n->Lasttime)
	{
		gpi_n->TimeCount++;
		if(gpi_n->TimeCount>=GPIO_IntimalFilter_Time)/* GPIO�ڲ�������� */
		{
			gpi_n->TimeCount=0x00;
			gpi_n->Lasttime=gpi_n->Thistime;

			struct  timeval t;
			gettimeofday(&t,NULL);

			if(gpi_n->Thistime == HIGHTLEVEL)
			{
				gpi_n->State=HIGHTLEVEL;
			}
			else
			{
				gpi_n->State=LOWLEVEL;
			}
		}
	}
	else {
		gpi_n->Lasttime=gpi_n->Thistime;
		gpi_n->TimeCount=0x00;
	}
}



void ReadGPI_MachUpCFG(void)
{
	unsigned char i,j,m;
	j=0;
	j=gpi_read();
	for(i=0;i<8;i++){
		m=j&(0x01<<i);
		ReadGPI_Process(m,&GPIStat[i]);
	}
}
/*****************************************************************************
* WriteGPO_REG_MachUpCFG
* DESCRIPTION:

* @Param  : null
* @Return : null
* ----
******************************************************************************/
void WriteGPO_REG_MachUpCFG(gponame OutputCfg,bool level)
{
	unsigned char i;
	unsigned char j;
	for(i=0;i<8;i++){
		if(GPO[i]==OutputCfg){
			if(level==1)j=GPO_H[i];
			else j=GPO_L[i];
			//printf("mmmmmmmmmmmmmmmmm:%x\r\n",j);
			if((j==0xff)||(j==0x00))gpo_reg=j;
			else if(((j&0xf0)!=0)&&((j&0x0f)!=0))gpo_reg=gpo_reg&j;//��ʾ��д�͵�ƽ
			else gpo_reg=gpo_reg|j;//��ʾ��д�ߵ�ƽ

			break;
		}
	}
	//printf("nnnnnnnnnnnnnnnnnnnnn:%x\r\n",gpo_reg);
}
void WriteGPO_MachUpCFG()
{
	static unsigned char m_reg=0;
	if(m_reg!=gpo_reg){
		gpo_write(gpo_reg);
		m_reg=gpo_reg;
	}
}

//=======================================================================
//����ʶ����ѹץ����Ȧ֮��ʶ������Ƹ����ж�
void LPR_Process(laneclass_type *LaneClass)
{
	QueryRes queryfields;
	QueryRes queryfields_tmp;
	int querystate=0;
	memset(&queryfields,0,sizeof(QueryRes));
	memset(&queryfields_tmp,0,sizeof(QueryRes));
	querystate=LPR_QueryLocDB_VM(tab_name[FILENO_FIXVM], &queryfields, LaneClass);	/* ��ѯ�̶����� */
	if(querystate == EXIST_EFFIC)
	{
		writelog(LaneLogLevel(LaneClass),"�ó����ڹ̶�����--���ڲ���Ч");
		#if 1
		if( queryfields.AccType==FIXVM )
		{
			writelog(LaneLogLevel(LaneClass),"����%d: �ó�Ϊ�̶���",LaneClass->LaneCfg->LaneNO);
			queryfields.AccType=FIXVM;
		}
		else if(queryfields.AccType==VIP)
		{
			writelog(LaneLogLevel(LaneClass),"����%d: �ó�ΪVIP��",LaneClass->LaneCfg->LaneNO);
			queryfields.AccType=VIP;
		}
		#endif
	}
	else if(querystate == EXIST_OVERDUE)	/* �̶���������ڵ������� */
	{
		writelog(LaneLogLevel(LaneClass),"�ó����ڹ̶�����--���ڵ�����");
		queryfields_tmp=queryfields;
		memset(&queryfields,0,sizeof(QueryRes));
		querystate=LPR_QueryLocDB_VM(tab_name[FILENO_MAAVM], &queryfields, LaneClass);	/* ��ѯԤԼ���� */
		if(querystate == EXIST_EFFIC)
		{
			writelog(LaneLogLevel(LaneClass),"�ó�����ԤԼ����--���ڲ���Ч");
			queryfields.AccType=MAAVM;
		}
		else if(querystate == EXIST_OVERDUE)	/* ���ԤԼ�������ڣ����ߴ��ڵ���Ч */
		{
			writelog(LaneLogLevel(LaneClass),"�ó�����ԤԼ����--���ڵ�����");
			queryfields=queryfields_tmp;
			queryfields.AccType=TEMPVM;
		}
		else {										/* ���ԤԼ�������ڣ����ߴ��ڵ���Ч */
			writelog(LaneLogLevel(LaneClass),"�ó�����ԤԼ����--������");
			queryfields=queryfields_tmp;			/* ��ô��ѯ����Ϣ�ѹ̶�����Ϣ��Ϊ׼ */
			queryfields.AccType=TEMPVM;				/* ��ʱ��,ö�� */
		}
	}
	else if(querystate == NOEXIST)	/* �̶��������ڸÿ��� */
	{
		writelog(LaneLogLevel(LaneClass),"�ó����ڹ̶�����--������");
		memset(&queryfields,0,sizeof(QueryRes));
		//querystate=QueryLocDB_MaaVM(&queryfields,pe);
		querystate=LPR_QueryLocDB_VM(tab_name[FILENO_MAAVM], &queryfields, LaneClass);	/* ��ѯԤԼ���� */
		if(querystate == EXIST_EFFIC)	/* ���ԤԼ���������Ч����ô����ΪԤԼ�� */
		{
			writelog(LaneLogLevel(LaneClass),"�ó�����ԤԼ����--���ڲ���Ч");
			queryfields.AccType=MAAVM;
		}
		else if(querystate == EXIST_OVERDUE)	/* ��ʱ������ */
		{
			writelog(LaneLogLevel(LaneClass),"�ó�����ԤԼ����--���ڵ�����");
			queryfields.AccType=TEMPVM;				/* ���źͳ��ƶ���ԤԼ���������ȡ */
		}
		else if(querystate == NOEXIST)
		{
			writelog(LaneLogLevel(LaneClass),"�ó�����ԤԼ����--������");
			strcpy(queryfields.TagNo,LaneClass->Obj_Snap[0].tagno);	/* �������ڣ����ڶ��еĿ��ŷ����ѯ����ṹ�壬ֻ�п��ţ�û�г��� */
			strcpy(queryfields.TagNo1,LaneClass->Obj_Snap[1].tagno);
			strcpy(queryfields.TagPlateNo,"");
			strcpy(queryfields.TagPlateColour,"");
			queryfields.AccType=TEMPVM;				/* ���ԤԼ������Ч���߲����ڣ���ô��Ϊ��ʱ�� */
		}
	}

	writelog(LaneLogLevel(LaneClass),"�ó���ǩ���ƺ���--%s%s",queryfields.TagPlateColour,queryfields.TagPlateNo);
	strcpy(LaneClass->Obj_Snap[0].tagno,queryfields.TagNo);/* ��ѯ�������Ŀ���� */
	strcpy(LaneClass->Obj_Snap[0].tagplateno,queryfields.TagPlateNo);
	strcpy(LaneClass->Obj_Snap[0].tagplatecl,queryfields.TagPlateColour);
	LaneClass->Obj_Snap[0].acctype=queryfields.AccType;

	strcpy(LaneClass->Obj_Snap[1].tagno,queryfields.TagNo1);/* ��ѯ�������Ŀ���� */
	strcpy(LaneClass->Obj_Snap[1].tagplateno,queryfields.TagPlateNo);//_by_shl_duptag_20140218
	strcpy(LaneClass->Obj_Snap[1].tagplatecl,queryfields.TagPlateColour);
	LaneClass->Obj_Snap[1].acctype=queryfields.AccType;

}
/************************************************************************************
* Judge_UpRail
* DESCRIPTION: ץ����Ȧ������,�������˫��ǩ�߼�����
* @LaneClass : ��������
* @iList	 : ʶ�����
* @Return 	 : no
* modify     : //_by_shl_inc_Compatible_with_double_label_20131213
*************************************************************************************/
void Judge_RFIDUpRail_Entry(laneclass_type *LaneClass, identity_node *iList)
{
	if( LaneClass->LaneCfg->Exemption == FULLEXEMPTION )		//_by_dpj_2015-01-08
	{
		if( LaneClass->WorkMode == Exemption )
		{
			writelog(LaneLogLevel(LaneClass),"����%d: ����ȫ��칤��ģʽ������̧��",LaneClass->LaneCfg->LaneNO);

			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown, RELEASE_DOWNRAIL);		/* �ͷ���� */
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, UPRAIL);					/* ̧�˷��� */
		}
	}

	struct  timeval t;
	char buf[32];
	identity_node *pi;					/* ���ʶ�����ʱ�䳬����Ч��Χ�ĵ�Ԫ */
	identity_node *pitmp;
	pi = iList->Next;
	/* �޳�������Ч���뾶ʱ��Ŀ� */
	while( pi != NULL )
	{
		pitmp = pi->Next;
		int j;
		j = (LaneClass->Time_SnapRedge.tv_sec - pi->t_tag.tv_sec)*1000
			+ (LaneClass->Time_SnapRedge.tv_usec - pi->t_tag.tv_usec)/1000;
		if( j > SysCfg.RFIDAheadTime_Ms )
		{
			writelog(LaneLogLevel(LaneClass),"����%d: ����ʱ�䳬����Чʱ��(%d) ���뾶��Χ,�˿�Ϊ��Ч��",LaneClass->LaneCfg->LaneNO, SysCfg.RFIDAheadTime_Ms);
			Delete_IdentityDupNode(pi);
		}
		pi = pitmp;
	}
	/* ����ʶ���ǩ�����洢������ͬʱ��ȡʶ������еĸ���*/
	unsigned char tag_quantity = 0;
	tag_quantity = Get_IdentityDupListLength(iList);
	printf("[Judge_RFIDUpRail_Entry]:tag_quantity is %d\n", tag_quantity);

	/*******************************************************************
	�ֿ�����
	����: ʹ�ÿ�����䣬ȷ����������ʶ�𵽵ı�ǩ����
	*********************************************************************/
	bool flag_2tag1Plate = LOWLEVEL; //�ֲ�λ�����������HIGHTLEVEL��˵�����������ſ���Ӧͬһ����ǩ
	switch( tag_quantity )
	{
		case 0:
		{
			LaneClass->Snap_TagQuantity = ZEROTAG_0;
			writelog(LaneLogLevel(LaneClass),"����%d: �ɼ���0�ű�ǩ����Ϊ�޿�������",LaneClass->LaneCfg->LaneNO);
			break;
		}
		case 1:
		{
			LaneClass->Snap_TagQuantity = ONETAG_1;
			writelog(LaneLogLevel(LaneClass),"����%d: �ɼ���1�ű�ǩ",LaneClass->LaneCfg->LaneNO);
			break;
		}
		case 2:
		{
			int len0 = 0, len1 = 0, len2 = 0, len3 = 0;

			/* ��ȡ��2�ſ���ǩ���ƺźͳ�����ɫ�ĳ���*/
			len0 = strlen(iList->Next->tagplateno);
			len1 = strlen(iList->Next->tagplatecl);
			/* ��ȡ��1�ſ���ǩ���ƺźͳ�����ɫ�ĳ���*/
			len2 = strlen(iList->Next->Next->tagplateno);
			len3 = strlen(iList->Next->Next->tagplatecl);

			if( (len0 == 0) && (len1 == 0) && (len2 == 0) && (len3 == 0) )		/* 2�ſ������ڿ� */
			{
				LaneClass->Snap_TagQuantity = ZEROTAG_2_N;
				writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�2�ű�ǩ,��ǩ���ƶ�Ϊ�գ���0�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
			}
			else if( (len0 > 0) && (len1 > 0) && (len2 > 0) && (len3 > 0) )			/* 2�ſ����ڿ�*/
			{
				char *p0 = NULL, *p1 = NULL, *p2 = NULL, *p3 = NULL;
				p0 = iList->Next->tagplateno;
				p1 = iList->Next->tagplatecl;
				p2 = iList->Next->Next->tagplateno;
				p3 = iList->Next->Next->tagplatecl;

				#if 0   //debug
				printf("[Judge_RFIDUpRail_Entry]:iList->Next->tagplateno is %s\n", iList->Next->tagplateno);
				printf("[Judge_RFIDUpRail_Entry]:iList->Next->tagplatecl is %s\n", iList->Next->tagplatecl);
				printf("[Judge_RFIDUpRail_Entry]:iList->Next->Next->tagplateno is %s\n", iList->Next->Next->tagplateno);
				printf("[Judge_RFIDUpRail_Entry]:iList->Next->Next->tagplatecl is %s\n", iList->Next->Next->tagplatecl);
				printf("[Judge_RFIDUpRail_Entry]:strcmp(p0,p2)is %d\n", strcmp(p0,p2));
				printf("[Judge_RFIDUpRail_Entry]:strcmp(p1,p3) is %d\n", strcmp(p1,p3));
				#endif

				/* ���ƺͳ�����ɫ����ͬ������1�ſ����� */
				if( (strcmp(p0,p2) == 0) && (strcmp(p1,p3) == 0) )
				{
					LaneClass->Snap_TagQuantity = ONETAG_2_Y_Y;
					writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�2�ű�ǩ,��ǩ������ͬ����1�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
					flag_2tag1Plate = HIGHTLEVEL;
				}
				/* ���ƻ�����ɫ����ͬ������0�ſ����� */
				else if( (strcmp(p0,p2) != 0) || (strcmp(p1,p3) != 0) )
				{
					LaneClass->Snap_TagQuantity = ZEROTAG_2_Y_N;
					writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�2�ű�ǩ,��ǩ���ƶ���Ϊ�գ����Ҳ���ͬ����ǩ���Ʋ�ͬ����0�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
					flag_2tag1Plate = LOWLEVEL;
				}
			}
			else	 											/* 1�ſ��ڿ⣬��1�ſ����ڿ�*/
			{
				LaneClass->Snap_TagQuantity = ONETAG_2_YoN;
				writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�2�ű�ǩ,1�ű�ǩ����Ϊ�գ���1�ű�ǩ���Ʋ�Ϊ�գ���1�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
			}
			break;
		}
		default:				/* ����2�ſ�*/
		{
			LaneClass->Snap_TagQuantity = ZEROTAG_M2;
			writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�������2�ű�ǩ,��0�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
			break;
		}
	}
	/******************************************************************************
	�ֿ����ܽ���
	*******************************************************************************/

	/*******************************************************************
	�������ƹ���
	����: ʹ�ÿ�����䣬���ݿ�����������Ӧ�Ķ���
	*********************************************************************/
	switch( LaneClass->Snap_TagQuantity )
	{
		case ZEROTAG_0:
		case ZEROTAG_2_N:
		case ZEROTAG_2_Y_N:
		case ZEROTAG_M2:
		{
			/* û�����ó���ʶ���޿�������Ϊ��ʱ�����ϴ���¼ */
			if( LaneClass->LaneCfg->LprCfg == NOCFG )
			{
				LaneClass->Obj_Snap[0].acctype = TEMPVM;
				LaneClass->CmfType = NONE_CFM;

				/* ��ʱ���ڳ����򿪺󣬲ſ���ȥ��ѯ�˻����Ʊ� */
				if( LaneClass->Flag_OpenLane == LOWLEVEL )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: ��̨���Ƴ����رգ���ֹͨ��",LaneClass->LaneCfg->LaneNO);

					LaneClass->Note = LIMIT_PASS;	//_by_dpj_2014-09-09

					ar_2FIFO(LaneClass);
					break;
				}

				LaneClass->Note = LIMIT_PASS;	//_by_dpj_2014-09-09

				writelog(LaneLogLevel(LaneClass),"����%d: δ���ó���ʶ�𣬶���0�ſ�����ֹͨ��",LaneClass->LaneCfg->LaneNO);
				ar_2FIFO(LaneClass);
			 }

			LaneClass->Obj_Snap[0].acctype = TEMPVM;		//_by_dpj_2014-09-25
			break;
		}
		case ONETAG_1:
		case ONETAG_2_Y_Y:
		{
			char tmp_pno[20];								/* ���泵�� */
			char tmp_cl[10];
			struct timeval tmp_t_snap;	//_by_dpj_2014-09-12	�޸�ԭ��: û�б���ʱ��ֵ�����¼�������

			memset(tmp_pno, 0, sizeof(tmp_pno));
			memset(tmp_cl, 0, sizeof(tmp_cl));
			strcpy(tmp_pno, LaneClass->Obj_Snap[0].plateno);
			strcpy(tmp_cl, LaneClass->Obj_Snap[0].platecl);
			tmp_t_snap = LaneClass->Obj_Snap[0].t_snap;		//_by_dpj_2014-09-12
			LaneClass->Obj_Snap[0] = *(iList->Next);
			strcpy(LaneClass->Obj_Snap[0].plateno, tmp_pno);
			strcpy(LaneClass->Obj_Snap[0].platecl, tmp_cl);
			LaneClass->Obj_Snap[0].t_snap = tmp_t_snap;		//_by_dpj_2014-09-12

			if( (strlen(iList->Next->tagplatecl) == 0 )     /* �����ڷ��鳵�ƣ�˵�����ݿⲻ���ڸÿ� */
				&& (strlen(iList->Next->tagplateno) == 0) )
			{
				if( LaneClass->LaneCfg->LprCfg == NOCFG )	/* û�����ó���ʶ�� */
				{
					LaneClass->CmfType = RFID_CFM;
					writelog(LaneLogLevel(LaneClass),"����%d: ��ʱ���������ڷ��鳵�ƣ������ó���ʶ�������޶�������",LaneClass->LaneCfg->LaneNO);
				}
				else		/* �����ó���ʶ�� */
				{
					writelog(LaneLogLevel(LaneClass),"����%d: ��ʱ���������ڷ��鳵�ƣ��ȴ�����ʶ���ٴ�ȷ��",LaneClass->LaneCfg->LaneNO);
					//break;
				}
			}

			LaneClass->CmfType = RFID_CFM;

			if( (iList->Next->acctype == TEMPVM)
				&& (LaneClass->Flag_OpenLane == LOWLEVEL) )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ��̨���Ƴ����رգ���ֹ��ʱ��ͨ��",LaneClass->LaneCfg->LaneNO);

				#if 1		//_by_dpj_2014-09-09
				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
					ar_2FIFO(LaneClass);
				}
				#endif

				break;
			}

			int j = -1;
			#if 1				//_by_dpj_2014-05-20	�޸�ԭ��: �޷���ʱ��ֵ�洢�����¸�����������
			//iList->Next->t_snap = LaneClass->Time_SnapRedge;
			LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;
			#endif
			j = QueryLocDB_LimitTab(iList->Next, LaneClass);
			if( j == EXIST_ALLOWPASS )			/* ��������ͨ�еģ����ǿ���ͨ�е� */
			{
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��",LaneClass->LaneCfg->LaneNO);

				/********************************̧�˶����ж�*************************************************/
				if( LaneClass->WorkMode == Common )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/* �ͷ���� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* ̧�˷��� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/* �ͷű��� */
					gettimeofday(&t,NULL);
					Chg_PreciseTime2Str(t, buf, sizeof(buf));

					writelog(LaneLogLevel(LaneClass),"����%d: ����̧��",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == FreePass )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ��������������ģʽ�����˳�̧",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Exemption )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ������칤��ģʽ�������Ѷ���",LaneClass->LaneCfg->LaneNO);
				}
				LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
				/****************************************************************************************************/
				#if 0
				printf("[]:LaneClass->Obj_Snap[0].t_snap.tv_sec is %d\n", LaneClass->Obj_Snap[0].t_snap.tv_sec);
				printf("[]:LaneClass->Time_SnapRedge is %d\n", LaneClass->Time_SnapRedge.tv_sec);
				#endif
				Insert_FilterDupNode(Filter_DupList,
									LaneClass->Obj_Snap[0].tagno,		/* ̧��֮�󣬼��뵽�˲����� */
									LaneClass->Obj_Snap[0].antennaID,
									LaneClass->Obj_Snap[0].t_snap);
				/* �����2��ǩ1���ƣ���ô2��ǩ�Ŷ����뵽�˲�������ͷ */
				if(flag_2tag1Plate == HIGHTLEVEL)
				{
					Insert_FilterDupNode(Filter_DupList,
										iList->Next->Next->tagno,
										iList->Next->Next->antennaID,
										iList->Next->Next->t_snap);
				}

				Clear_IdentityDupList(iList);						/* ���RFIDʶ�������Ӧ��Ԫ */
			}
			else if( j == EXIST_UNALLOWPASS_AL )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���������ͨ��",LaneClass->LaneCfg->LaneNO);

				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
			}
			else if( j == EXIST_UNALLOWPASS )
			{
				LaneClass->Note = LIMIT_PASS;
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��",LaneClass->LaneCfg->LaneNO);
			}
			#if 1			//_by_dpj_2014-05-20	�޸�ԭ��: �޸���������ʾ��־��Ϣ
			else if( j == UNEXIST )
			{
				#if 1		//_by_dpj_2014-09-12	�޸�ԭ��: �·������������Ʊ��������ݣ����²��ٷ�������ʶ��(С���ṩ)
				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
				#endif

				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ(Ĭ��)����ͨ��",LaneClass->LaneCfg->LaneNO);
			}
			#endif

			if( LaneClass->LaneCfg->LprCfg == NOCFG )		/* û�����ó���ʶ�𣬽�ֹͨ�г������ϴ���Ϣ */
			{
				ar_2FIFO(LaneClass);
			}
			break;
		}
		case ONETAG_2_YoN:
		{
			char tmp_pno[20];
			char tmp_cl[10];
			struct timeval tmp_t_snap;	//_by_dpj_2014-09-12	�޸�ԭ��: û�б���ʱ��ֵ�����¼�������

			/* ��1�ſ������ڷ��鳵�ƣ�˵�����ݿⲻ���ڸÿ� */
			printf("[]:LaneClass->Obj_Snap[0].tagplateno is %s\n", iList->Next->tagplateno);
			printf("[]:LaneClass->Obj_Snap[0].tagplatecl is %s\n", iList->Next->tagplatecl);
			printf("[]:LaneClass->Obj_Snap[1].tagplateno is %s\n", iList->Next->Next->tagplateno);
			printf("[]:LaneClass->Obj_Snap[1].tagplatecl is %s\n", iList->Next->Next->tagplatecl);
			if( (strlen(iList->Next->tagplateno) == 0 )
				&& (strlen(iList->Next->tagplatecl) == 0) )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ��2�ſ����������ݿ���",LaneClass->LaneCfg->LaneNO);

				memset(tmp_pno, 0, sizeof(tmp_pno));				/* �����1�ų��� ��Ϣ*/
				memset(tmp_cl, 0, sizeof(tmp_cl));
				strcpy(tmp_pno, LaneClass->Obj_Snap[1].plateno);
				strcpy(tmp_cl, LaneClass->Obj_Snap[1].platecl);
				tmp_t_snap = LaneClass->Obj_Snap[1].t_snap;		//_by_dpj_2014-09-12
				LaneClass->Obj_Snap[0] = *(iList->Next->Next);
				strcpy(LaneClass->Obj_Snap[0].plateno, tmp_pno);
				strcpy(LaneClass->Obj_Snap[0].platecl, tmp_cl);
				LaneClass->Obj_Snap[0].t_snap = tmp_t_snap;		//_by_dpj_2014-09-12
			}
			else if( (strlen(iList->Next->Next->tagplateno) == 0 )
				&& (strlen(iList->Next->Next->tagplatecl) == 0) )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ��1�ſ����������ݿ���",LaneClass->LaneCfg->LaneNO);

				memset(tmp_pno, 0, sizeof(tmp_pno));				/* �����2�ų��� ��Ϣ*/
				memset(tmp_cl, 0, sizeof(tmp_cl));
				strcpy(tmp_pno, LaneClass->Obj_Snap[0].plateno);
				strcpy(tmp_cl, LaneClass->Obj_Snap[0].platecl);
				tmp_t_snap = LaneClass->Obj_Snap[0].t_snap;		//_by_dpj_2014-09-12
				LaneClass->Obj_Snap[0] = *(iList->Next);
				strcpy(LaneClass->Obj_Snap[0].plateno, tmp_pno);
				strcpy(LaneClass->Obj_Snap[0].platecl, tmp_cl);
				LaneClass->Obj_Snap[0].t_snap = tmp_t_snap;		//_by_dpj_2014-09-12
			}

			LaneClass->CmfType = RFID_CFM;

			/* �п���ʱ���ڳ����򿪺󣬲ſ���ȥ��ѯ�˻����Ʊ� */
			if( (LaneClass->Obj_Snap[0].acctype== TEMPVM) && (LaneClass->Flag_OpenLane == LOWLEVEL) )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ��̨���Ƴ����رգ���ֹ��ʱ��ͨ��",LaneClass->LaneCfg->LaneNO);
				break;
			}

			int j = -1;
			LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;

			j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);
			if( j == EXIST_ALLOWPASS )			/* ��������ͨ�еģ����ǿ���ͨ�е� */
			{
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��",LaneClass->LaneCfg->LaneNO);

				/********************************̧�˶����ж�*************************************************/
				if( LaneClass->WorkMode == Common )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/* �ͷ���� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* ̧�˷��� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/* �ͷű��� */
					gettimeofday(&t,NULL);
					Chg_PreciseTime2Str(t, buf, sizeof(buf));

					writelog(LaneLogLevel(LaneClass),"����%d: ����̧��",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == FreePass )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ��������������ģʽ�����˳�̧",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Exemption )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ������칤��ģʽ�������Ѷ���",LaneClass->LaneCfg->LaneNO);
				}
				LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
				/****************************************************************************************************/

				Insert_FilterDupNode(Filter_DupList,
									LaneClass->Obj_Snap[0].tagno,		/* ̧��֮�󣬼��뵽�˲����� */
									LaneClass->Obj_Snap[0].antennaID,
									LaneClass->Obj_Snap[0].t_snap);

				Clear_IdentityDupList(iList);						/* ���RFIDʶ�������Ӧ��Ԫ */
			}
			else if( j == EXIST_UNALLOWPASS_AL )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���������ͨ��",LaneClass->LaneCfg->LaneNO);

				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
			}
			else if( j == EXIST_UNALLOWPASS )
			{
				LaneClass->Note = LIMIT_PASS;
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��",LaneClass->LaneCfg->LaneNO);
			}
			#if 1			//_by_dpj_2014-05-20	�޸�ԭ��: �޸���������ʾ��־��Ϣ
			else if( j == UNEXIST )
			{
				#if 1		//_by_dpj_2014-09-12	�޸�ԭ��: �·������������Ʊ��������ݣ����²��ٷ�������ʶ��(С���ṩ)
				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
				#endif
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ(Ĭ��)����ͨ��",LaneClass->LaneCfg->LaneNO);
			}
			#endif

			if(LaneClass->LaneCfg->LprCfg == NOCFG)		/* û�����ó���ʶ�𣬽�ֹͨ�г������ϴ���Ϣ */
			{
				ar_2FIFO(LaneClass);
			}
				break;
		}
		case TWOTAGS:
		{
			break;
		}
	}
	/******************************************************************************
	�������ƹ��ܽ���
	*******************************************************************************/

	return;
}

/************************************************************************************
* Judge_UpRail
* DESCRIPTION: ץ����Ȧ������
* @LaneClass : ��������
* @iList	 : ʶ�����
* @Return 	 : no
* ----
*************************************************************************************/
void Judge_RFIDUpRail_Export(laneclass_type *LaneClass,identity_node *iList)
{
	if( LaneClass->LaneCfg->Exemption == FULLEXEMPTION )		//_by_dpj_2015-01-08
	{
		if( LaneClass->WorkMode == Exemption )
		{
			writelog(LaneLogLevel(LaneClass),"����%d: ����ȫ��칤��ģʽ������̧��",LaneClass->LaneCfg->LaneNO);

			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown, RELEASE_DOWNRAIL);		/* �ͷ���� */
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, UPRAIL);					/* ̧�˷��� */
		}
	}

	struct  timeval t;
	char buf[32];
	identity_node *pi;					/* ���ʶ�����ʱ�䳬����Ч��Χ�ĵ�Ԫ */
	identity_node *pitmp;
	pi = iList->Next;

	/* �޳�������Ч���뾶ʱ��Ŀ� */
	while( pi != NULL )
	{
		pitmp = pi->Next;
		int j;
		j = (LaneClass->Time_SnapRedge.tv_sec - pi->t_tag.tv_sec)*1000
			+ (LaneClass->Time_SnapRedge.tv_usec - pi->t_tag.tv_usec)/1000;
		if( j > SysCfg.RFIDAheadTime_Ms )
		{
			writelog(LaneLogLevel(LaneClass),"����%d: ����ʱ�䳬����Чʱ��(%d) ���뾶��Χ,�˿�Ϊ��Ч��",LaneClass->LaneCfg->LaneNO, SysCfg.RFIDAheadTime_Ms);
			Delete_IdentityDupNode(pi);
		}
		pi = pitmp;
	}

	/* ����ʶ���ǩ�����洢������ͬʱ��ȡʶ������еĸ���*/
	unsigned char tag_quantity = 0;
	tag_quantity = Get_IdentityDupListLength(iList);

	/*******************************************************************
	�ֿ�����
	����: ʹ�ÿ�����䣬ȷ����������ʶ�𵽵ı�ǩ����
	*********************************************************************/
	bool flag_2tag1Plate = LOWLEVEL; //�ֲ�λ�����������HIGHTLEVEL��˵�����������ſ���Ӧͬһ����ǩ
	switch( tag_quantity )
	{
		case 0:
		{
			LaneClass->Snap_TagQuantity = ZEROTAG_0;
			writelog(LaneLogLevel(LaneClass),"����%d: �ɼ���0�ű�ǩ����Ϊ�޿�������",LaneClass->LaneCfg->LaneNO);
			break;
		}
		case 1:
		{
			LaneClass->Snap_TagQuantity = ONETAG_1;
			writelog(LaneLogLevel(LaneClass),"����%d: �ɼ���1�ű�ǩ",LaneClass->LaneCfg->LaneNO);
			break;
		}
		case 2:
		{
			int len0 = 0, len1 = 0, len2 = 0, len3 = 0;

			/* ��ȡ��2�ſ���ǩ���ƺźͳ�����ɫ�ĳ���*/
			len0 = strlen(iList->Next->tagplateno);
			len1 = strlen(iList->Next->tagplatecl);
			/* ��ȡ��1�ſ���ǩ���ƺźͳ�����ɫ�ĳ���*/
			len2 = strlen(iList->Next->Next->tagplateno);
			len3 = strlen(iList->Next->Next->tagplatecl);

			if( (len0 == 0) && (len1 == 0) && (len2 == 0) && (len3 == 0) )		/* 2�ſ������ڿ� */
			{
				LaneClass->Snap_TagQuantity = ZEROTAG_2_N;
				writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�2�ű�ǩ,��ǩ���ƶ�Ϊ�գ���0�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
			}
			else if( (len0 > 0) && (len1 > 0) && (len2 > 0) && (len3 > 0) )			/* 2�ſ����ڿ�*/
			{
				char *p0 = NULL, *p1 = NULL, *p2 = NULL, *p3 = NULL;
				p0 = iList->Next->tagplateno;
				p1 = iList->Next->tagplatecl;
				p2 = iList->Next->Next->tagplateno;
				p3 = iList->Next->Next->tagplatecl;

				/* ���ƺͳ�����ɫ����ͬ������1�ſ����� */
				if( (strcmp(p0,p2) == 0) && (strcmp(p1,p3) == 0) )
				{
					LaneClass->Snap_TagQuantity = ONETAG_2_Y_Y;
					writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�2�ű�ǩ,��ǩ������ͬ����1�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
					flag_2tag1Plate = HIGHTLEVEL;
				}
				/* ���ƻ�����ɫ����ͬ������0�ſ����� */
				else if( (strcmp(p0,p2) != 0) || (strcmp(p1,p3) != 0) )		//_by_dpj_2014-04-08
				{
					LaneClass->Snap_TagQuantity = ZEROTAG_2_Y_N;
					writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�2�ű�ǩ,��ǩ���ƶ���Ϊ�գ����Ҳ���ͬ����ǩ���Ʋ�ͬ����0�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
					flag_2tag1Plate = LOWLEVEL;
				}
			}
			else	 											/* 1�ſ��ڿ⣬��1�ſ����ڿ�*/
			{
				LaneClass->Snap_TagQuantity = ONETAG_2_YoN;
				writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�2�ű�ǩ,1�ű�ǩ����Ϊ�գ���1�ű�ǩ���Ʋ�Ϊ�գ���1�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
			}
			break;
		}
		default:				/* ����2�ſ�*/
		{
			LaneClass->Snap_TagQuantity = ZEROTAG_M2;
			writelog(LaneLogLevel(LaneClass),"����%d: �ɼ�������2�ű�ǩ,��0�ű�ǩ����",LaneClass->LaneCfg->LaneNO);
			break;
		}
	}
	/******************************************************************************
	�ֿ����ܽ���
	*******************************************************************************/

	/*******************************************************************
	�������ƹ���
	����: ʹ�ÿ�����䣬���ݿ�����������Ӧ�Ķ���
	*********************************************************************/
	switch( LaneClass->Snap_TagQuantity )
	{
		case ZEROTAG_0:
		case ZEROTAG_2_N:
		case ZEROTAG_2_Y_N:
		case ZEROTAG_M2:
		{
			/* û�����ó���ʶ���޿�������Ϊ��ʱ�����ϴ���¼ */
			if( LaneClass->LaneCfg->LprCfg == NOCFG )
			{
				LaneClass->Obj_Snap[0].acctype = TEMPVM;
				LaneClass->CmfType = NONE_CFM;

				LaneClass->Note = LIMIT_PASS;	//_by_dpj_2014-09-09

				writelog(LaneLogLevel(LaneClass),"����%d: δ���ó���ʶ�𣬶���0�ſ�����ֹͨ��",LaneClass->LaneCfg->LaneNO);
				ar_2FIFO(LaneClass);
			 }
			break;
		}
		case ONETAG_1:
		case ONETAG_2_Y_Y:
		{
			char tmp_pno[20];								/* ���泵�� */
			char tmp_cl[10];
			struct timeval tmp_t_snap;	//_by_dpj_2014-09-15	�޸�ԭ��: û�б���ʱ��ֵ�����¼�������

			memset(tmp_pno, 0, sizeof(tmp_pno));
			memset(tmp_cl, 0, sizeof(tmp_cl));
			strcpy(tmp_pno, LaneClass->Obj_Snap[0].plateno);
			strcpy(tmp_cl, LaneClass->Obj_Snap[0].platecl);
			tmp_t_snap = LaneClass->Obj_Snap[0].t_snap;		//_by_dpj_2014-09-15
			LaneClass->Obj_Snap[0] = *(iList->Next);
			strcpy(LaneClass->Obj_Snap[0].plateno, tmp_pno);
			strcpy(LaneClass->Obj_Snap[0].platecl, tmp_cl);
			LaneClass->Obj_Snap[0].t_snap = tmp_t_snap;		//_by_dpj_2014-09-15

			if( (strlen(iList->Next->tagplatecl) == 0 )     /* �����ڷ��鳵�ƣ�˵�����ݿⲻ���ڸÿ� */
				&& (strlen(iList->Next->tagplateno) == 0) )
			{
				if( LaneClass->LaneCfg->LprCfg == NOCFG )	/* û�����ó���ʶ�� */
				{
					LaneClass->CmfType = RFID_CFM;
					writelog(LaneLogLevel(LaneClass),"����%d: ��ʱ���������ڷ��鳵�ƣ������ó���ʶ�������޶�������",LaneClass->LaneCfg->LaneNO);
				}
				else		/* �����ó���ʶ�� */
				{
					writelog(LaneLogLevel(LaneClass),"����%d: ��ʱ���������ڷ��鳵�ƣ��ȴ�����ʶ���ٴ�ȷ��",LaneClass->LaneCfg->LaneNO);
					//break;				//_by_dpj_2014-04-08
				}
			}

			LaneClass->CmfType = RFID_CFM;

			int j = -1;
			#if 1				//_by_dpj_2014-05-20
			//iList->Next->t_snap = LaneClass->Time_SnapRedge;
			LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;
			#endif

			j = QueryLocDB_LimitTab(iList->Next, LaneClass);
			if( j == EXIST_ALLOWPASS )			/* ��������ͨ�еģ����ǿ���ͨ�е� */
			{
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��",LaneClass->LaneCfg->LaneNO);

				/********************************̧�˶����ж�*************************************************/
				if( LaneClass->WorkMode == Common )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/* �ͷ���� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* ̧�˷��� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/* �ͷű��� */
					gettimeofday(&t,NULL);
					Chg_PreciseTime2Str(t, buf, sizeof(buf));

					writelog(LaneLogLevel(LaneClass),"����%d: ����̧��",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == FreePass )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ��������������ģʽ�����˳�̧",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Deployment )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ���ڲ�������ģʽ�����˲�����",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Exemption )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ������칤��ģʽ�������Ѷ���",LaneClass->LaneCfg->LaneNO);
				}
				LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
				/****************************************************************************************************/

				Insert_FilterDupNode(Filter_DupList,
									LaneClass->Obj_Snap[0].tagno,		/* ̧��֮�󣬼��뵽�˲����� */
									LaneClass->Obj_Snap[0].antennaID,
									LaneClass->Obj_Snap[0].t_snap);
				/* �����2��ǩ1���ƣ���ô2��ǩ�Ŷ����뵽�˲�������ͷ */
				if(flag_2tag1Plate == HIGHTLEVEL)
				{
					Insert_FilterDupNode(Filter_DupList,
										iList->Next->Next->tagno,
										iList->Next->Next->antennaID,
										iList->Next->Next->t_snap);
				}

				Clear_IdentityDupList(iList);						/* ���RFIDʶ�������Ӧ��Ԫ */
			}
			else if( j == EXIST_UNALLOWPASS_AL )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���������ͨ��",LaneClass->LaneCfg->LaneNO);

				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
			}
			else if( j == EXIST_UNALLOWPASS )
			{
				LaneClass->Note = LIMIT_PASS;
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��",LaneClass->LaneCfg->LaneNO);

			}
			#if 1			//_by_dpj_2014-05-20	�޸�ԭ��: �޸���������ʾ��־��Ϣ
			else if( j == UNEXIST )
			{
				#if 1		//_by_dpj_2014-09-12	�޸�ԭ��: �·������������Ʊ��������ݣ����²��ٷ�������ʶ��(С���ṩ)
				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
				#endif
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ(Ĭ��)����ͨ��",LaneClass->LaneCfg->LaneNO);
			}
			#endif

			if( LaneClass->LaneCfg->LprCfg == NOCFG )		/* û�����ó���ʶ�𣬽�ֹͨ�г������ϴ���Ϣ */
			{
				ar_2FIFO(LaneClass);
			}
			break;
		}
		case ONETAG_2_YoN:
		{
			char tmp_pno[20];
			char tmp_cl[10];
			struct timeval tmp_t_snap;	//_by_dpj_2014-09-15	�޸�ԭ��: û�б���ʱ��ֵ�����¼�������

			/* ��1�ſ������ڷ��鳵�ƣ�˵�����ݿⲻ���ڸÿ� */
			printf("[]:LaneClass->Obj_Snap[0].tagplateno is %s\n", iList->Next->tagplateno);
			printf("[]:LaneClass->Obj_Snap[0].tagplatecl is %s\n", iList->Next->tagplatecl);
			printf("[]:LaneClass->Obj_Snap[1].tagplateno is %s\n", iList->Next->Next->tagplateno);
			printf("[]:LaneClass->Obj_Snap[1].tagplatecl is %s\n", iList->Next->Next->tagplatecl);
			if( (strlen(iList->Next->tagplateno) == 0 )
				&& (strlen(iList->Next->tagplatecl) == 0) )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ��2�ſ����������ݿ���",LaneClass->LaneCfg->LaneNO);

				memset(tmp_pno, 0, sizeof(tmp_pno));				/* �����1�ų��� ��Ϣ*/
				memset(tmp_cl, 0, sizeof(tmp_cl));
				strcpy(tmp_pno, LaneClass->Obj_Snap[1].plateno);
				strcpy(tmp_cl, LaneClass->Obj_Snap[1].platecl);
				tmp_t_snap = LaneClass->Obj_Snap[1].t_snap;		//_by_dpj_2014-09-15
				LaneClass->Obj_Snap[0] = *(iList->Next->Next);
				strcpy(LaneClass->Obj_Snap[0].plateno, tmp_pno);
				strcpy(LaneClass->Obj_Snap[0].platecl, tmp_cl);
				LaneClass->Obj_Snap[0].t_snap = tmp_t_snap;		//_by_dpj_2014-09-15
			}
			else if( (strlen(iList->Next->Next->tagplateno) == 0 )
				&& (strlen(iList->Next->Next->tagplatecl) == 0) )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ��1�ſ����������ݿ���",LaneClass->LaneCfg->LaneNO);

				memset(tmp_pno, 0, sizeof(tmp_pno));				/* �����2�ų��� ��Ϣ*/
				memset(tmp_cl, 0, sizeof(tmp_cl));
				strcpy(tmp_pno, LaneClass->Obj_Snap[0].plateno);
				strcpy(tmp_cl, LaneClass->Obj_Snap[0].platecl);
				tmp_t_snap = LaneClass->Obj_Snap[0].t_snap;		//_by_dpj_2014-09-15
				LaneClass->Obj_Snap[0] = *(iList->Next);
				strcpy(LaneClass->Obj_Snap[0].plateno, tmp_pno);
				strcpy(LaneClass->Obj_Snap[0].platecl, tmp_cl);
				LaneClass->Obj_Snap[0].t_snap = tmp_t_snap;		//_by_dpj_2014-09-15
			}

			LaneClass->CmfType = RFID_CFM;

			#if 0
			/* �п���ʱ���ڳ����򿪺󣬲ſ���ȥ��ѯ�˻����Ʊ� */
			if( (LaneClass->Obj_Snap[0].acctype== TEMPVM) && (LaneClass->Flag_OpenLane == LOWLEVEL) )
			{
				printf("[RFID:]����%d: ��̨���Ƴ����رգ���ֹ��ʱ��ͨ��--------\r\n",
					LaneClass->LaneCfg->LaneNO);
				In_LogQueue(logqueue, "[RFID:]����%d: ��̨���Ƴ����رգ���ֹ��ʱ��ͨ��--------\r\n",
					LaneClass->LaneCfg->LaneNO, NULL, NULL, 9999, LOG_INFO);
				break;
			}
			#endif

			int j = -1;
			LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;

			j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);
			if( j == EXIST_ALLOWPASS )			/* ��������ͨ�еģ����ǿ���ͨ�е� */
			{
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��",LaneClass->LaneCfg->LaneNO);

				/********************************̧�˶����ж�*************************************************/
				if( LaneClass->WorkMode == Common )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/* �ͷ���� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* ̧�˷��� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/* �ͷű��� */
					gettimeofday(&t,NULL);
					Chg_PreciseTime2Str(t, buf, sizeof(buf));

					writelog(LaneLogLevel(LaneClass),"����%d: ����̧��",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == FreePass )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ��������������ģʽ�����˳�̧",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Deployment )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ���ڲ�������ģʽ�����˲�����",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Exemption )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: RFID: ������칤��ģʽ�������Ѷ���",LaneClass->LaneCfg->LaneNO);
				}
				LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
				/****************************************************************************************************/

				Insert_FilterDupNode(Filter_DupList,
									LaneClass->Obj_Snap[0].tagno,		/* ̧��֮�󣬼��뵽�˲����� */
									LaneClass->Obj_Snap[0].antennaID,
									LaneClass->Obj_Snap[0].t_snap);

				Clear_IdentityDupList(iList);						/* ���RFIDʶ�������Ӧ��Ԫ */
			}
			else if( j == EXIST_UNALLOWPASS_AL )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���������ͨ��",LaneClass->LaneCfg->LaneNO);

				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
			}
			else if( j == EXIST_UNALLOWPASS )
			{
				LaneClass->Note = LIMIT_PASS;
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��",LaneClass->LaneCfg->LaneNO);
			}
			#if 1			//_by_dpj_2014-05-20	�޸�ԭ��: �޸���������ʾ��־��Ϣ
			else if( j == UNEXIST )
			{
				#if 1		//_by_dpj_2014-09-12	�޸�ԭ��: �·������������Ʊ��������ݣ����²��ٷ�������ʶ��(С���ṩ)
				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
				#endif
				writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ(Ĭ��)����ͨ��",LaneClass->LaneCfg->LaneNO);
			}
			#endif

			if(LaneClass->LaneCfg->LprCfg == NOCFG)		/* û�����ó���ʶ�𣬽�ֹͨ�г������ϴ���Ϣ */
			{
				ar_2FIFO(LaneClass);
			}

			break;
		}
		case TWOTAGS:
		{
			break;
		}
	}
	/******************************************************************************
	�������ƹ��ܽ���
	*******************************************************************************/

	return;
}
/************************************************************************************
* Judge_LPRUpRail
* DESCRIPTION: ץ����Ȧ������
* @LaneClass : ��������
* @iList	 : ʶ�����
* @Return 	 : no
* ----
*************************************************************************************/
void Judge_LPRUpRail_Entry(laneclass_type * LaneClass,identity_node *iList)
{
	/* ʱ���ۼ���С�ڵ���ץ�ĺ���ʱ�䣬�г���ʶ�������Ҹý��δ������*/
	if( (LaneClass->TimeAcc_Snap <= SysCfg.LPRDelayTime_Ms)		/* ����������ִֻ��һ�� */
		&& (strlen(LaneClass->Obj_Snap[0].plateno) > 0)			/* �г���ʶ���� */
		&& (LaneClass->LprRes == NOLPRRES ) )
	{
		struct  timeval t;
		char buf[32];
		printf("����ʶ������:%s\r\n", LaneClass->Obj_Snap[0].plateno);

		/********************************************************************************
		����ʶ����_by_dpj_2014-09-29
		����: �ж��Ƿ���ʶ�����Ƿ�Ϊ���ƾ�ʶ
		********************************************************************************/
		if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )
		{
			writelog(LaneLogLevel(LaneClass),"����%d: ����һ��ʶ���г��ƽ��Ϊ���ƾ�ʶ, ���Խ������ʶ�����", LaneClass->LaneCfg->LaneNO);
			#if 1	//_by_dpj_2014-10-09	�޸�ԭ��: �����ܿ�����������Ϊ˫��ڣ�˫���ڣ�ʱ���ᵼ�¶���ʶ�����̴���
			CaptureImage(LaneClass->LPR->IP);
			#endif
			sleep(1);
		}
		/******************************************************************************
		����ʶ���ܽ���_by_dpj_2014-09-29
		*******************************************************************************/

		#if 1	//_by_dpj_2015-01-08
		if( LaneClass->LaneCfg->Exemption == CAREXEMPTION )
		{
			if( LaneClass->WorkMode == Exemption )
			{
				if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: ����������칤��ģʽ��%s�����˲�̧��", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
				}
				else if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") != 0 )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: ����������칤��ģʽ��%s������̧��", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown, RELEASE_DOWNRAIL);		/* �ͷ���� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, UPRAIL);					/* ̧�˷��� */
				}
			}
		}
		#endif

		LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;
		LaneClass->LprRes = HAVE_PROLPR;

		/* ��������Ϣ��RFIDʶ������б�����ͨ�б�����ͨ��ʱ�����ٽ��ж���ʶ��*/
		#if 1
		if( LaneClass->Note == LIMIT_PASS )
		{
			writelog(LaneLogLevel(LaneClass),"����%d: �����ó���ʶ���������ó���RFID�����б�����ͨ�У������г��ƽ�һ��ʶ����", LaneClass->LaneCfg->LaneNO);
			ar_2FIFO(LaneClass);
			return;
		}
		#endif

		switch( LaneClass->Snap_TagQuantity )
		{
			case ZEROTAG_0:
			case ZEROTAG_2_N:
			case ZEROTAG_2_Y_N:
			case ZEROTAG_M2:
			{
				LaneClass->CmfType = LPR_CFM;				//ʶ��ģʽ��־ΪLPRʶ��

				if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )			/* ����Ϊ���ƾ�ʶ����ֹͨ��*/
				{
					LaneClass->Obj_Snap[0].acctype = TEMPVM;
					writelog(LaneLogLevel(LaneClass),"����%d: ���ƾ�ʶ����ֹͨ��", LaneClass->LaneCfg->LaneNO);
					break;
				}
				else			/* ��ʶ���������Ϣ*/
				{
					LPR_Process(LaneClass);

					if((LaneClass->Obj_Snap[0].acctype == TEMPVM				/* �п���ʱ���ڳ����򿪺󣬲ſ���ȥ��ѯ�˻����Ʊ� */)
						&&(LaneClass->Flag_OpenLane == LOWLEVEL))
					{
						writelog(LaneLogLevel(LaneClass),"����%d,��̨���Ƴ����رգ���ֹ��ʱ��ͨ��", LaneClass->LaneCfg->LaneNO);
						break;
					}

					int j=-1;
					j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);
					if( j == EXIST_ALLOWPASS )
					{
						writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��", LaneClass->LaneCfg->LaneNO);

						/********************************̧�˶����ж�*************************************************/
						if( LaneClass->WorkMode == Common )
						{
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/* �ͷ���� */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* ̧�˷��� */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/* �ͷű��� */
							gettimeofday(&t,NULL);
							Chg_PreciseTime2Str(t, buf, sizeof(buf));

							writelog(LaneLogLevel(LaneClass),"����%d: ����̧��", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == FreePass )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ��������������ģʽ�����˳�̧", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Exemption )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ������칤��ģʽ�������Ѷ���", LaneClass->LaneCfg->LaneNO);
						}
						LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
						/********************************************************************************************************/

						/* ����з��鵽��ǩ�����뵽�˲����� */
						if( strlen(LaneClass->Obj_Snap[0].tagno) > 0 )
						{
							Insert_FilterDupNode(Filter_DupList,
												LaneClass->Obj_Snap[0].tagno,
												LaneClass->Obj_Snap[0].antennaID,
												LaneClass->Obj_Snap[0].t_snap);
						}
						if( strlen(LaneClass->Obj_Snap[1].tagno) > 0 )
						{
							Insert_FilterDupNode(Filter_DupList,
												LaneClass->Obj_Snap[1].tagno,
												LaneClass->Obj_Snap[1].antennaID,
												LaneClass->Obj_Snap[1].t_snap);
						}
					}
					else
					{
						LaneClass->Note = LIMIT_PASS;
						writelog(LaneLogLevel(LaneClass),"����%d: �ó���ֹͨ��", LaneClass->LaneCfg->LaneNO);
					}
				}

				break;
			}
			case ONETAG_1:
			case ONETAG_2_Y_Y:
			case ONETAG_2_YoN:
			{
				if( LaneClass->Obj_Snap[0].FlagUpRail == NO_URAILl )		/* ������������û̧�ˣ��������˻�����ͨ�е�*/
				{
					if( LaneClass->CmfType == NONE_CFM )		/* δ����ȷʶ��*/
					{
						LaneClass->CmfType = LPR_CFM;
					}

					if( LaneClass->Obj_Snap[0].acctype != TEMPVM ) 	 /* ����ʱ���������ٴ��ó���ȷ�� */
					{
						writelog(LaneLogLevel(LaneClass),"����%d: 1�ſ����ó�Ϊ����ʱ�������賵��ʶ��ȷ��", LaneClass->LaneCfg->LaneNO);
						break;
					}

					if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )  /* RFID��̧�ˣ��������ž�ʶ����������ֱ���ϴ� */
					{
						writelog(LaneLogLevel(LaneClass),"����%d: 1�ſ�������ʶ��Ϊ���ƾ�ʶ��������", LaneClass->LaneCfg->LaneNO);
						break;
					}

					if( (LaneClass->Obj_Snap[0].acctype == TEMPVM)
						&& (LaneClass->Flag_OpenLane == LOWLEVEL ) )
					{
						writelog(LaneLogLevel(LaneClass),"����%d,��̨���Ƴ����رգ���ֹ��ʱ��ͨ��", LaneClass->LaneCfg->LaneNO);
						break;
					}

					#if 1			/* �������־�ж�ԭ��*/
					if( (strcmp(LaneClass->Obj_Snap[0].plateno, iList->Next->tagplateno) == 0 )
					   && (strcmp(LaneClass->Obj_Snap[0].platecl,iList->Next->tagplatecl) == 0) )
					{
						writelog(LaneLogLevel(LaneClass),"����%d: 1�ſ���RFID���鳵����LPRʶ����һ��", LaneClass->LaneCfg->LaneNO);
					}
					else
					{
						writelog(LaneLogLevel(LaneClass),"����%d: 1�ſ���RFID���鳵����LPRʶ���Ʋ�һ��", LaneClass->LaneCfg->LaneNO);
					}
					#endif

					LaneClass->CmfType = LPR_CFM;
					LPR_Process(LaneClass);

					int j = -1;
					j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);

					/* ��������ͨ�л��߲�����,����Ϊ�ǿ���ͨ�е� */
					if( j == EXIST_ALLOWPASS )
					{
						writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��", LaneClass->LaneCfg->LaneNO);

						/********************************̧�˶����ж�*************************************************/
						if( LaneClass->WorkMode == Common )
						{
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/* �ͷ���� */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* ̧�˷��� */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/* �ͷű��� */
							gettimeofday(&t,NULL);
							Chg_PreciseTime2Str(t, buf, sizeof(buf));

							writelog(LaneLogLevel(LaneClass),"����%d: ����̧��", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == FreePass )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ��������������ģʽ�����˳�̧", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Exemption )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ������칤��ģʽ�������Ѷ���", LaneClass->LaneCfg->LaneNO);
						}
						LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
						/********************************************************************************************************/

						if( strlen(LaneClass->Obj_Snap[0].tagno) > 0 )		/* ����з��鵽��ǩ�����뵽�˲����� */
						{
							Insert_FilterDupNode(Filter_DupList,
												LaneClass->Obj_Snap[0].tagno,
												LaneClass->Obj_Snap[0].antennaID,
												LaneClass->Obj_Snap[0].t_snap);
						}
						if ( strlen(LaneClass->Obj_Snap[1].tagno) > 0 )
						{
							Insert_FilterDupNode(Filter_DupList,
												LaneClass->Obj_Snap[1].tagno,
												LaneClass->Obj_Snap[1].antennaID,
												LaneClass->Obj_Snap[1].t_snap);
						}
					}
					else
					{
						LaneClass->Note = LIMIT_PASS;
						writelog(LaneLogLevel(LaneClass),"����%d: �ó���ֹͨ��", LaneClass->LaneCfg->LaneNO);
					}
				}

				break;
			}
			case TWOTAGS:
			{
				break;
			}
		}

		ar_2FIFO(LaneClass);
	}
	else if( (LaneClass->TimeAcc_Snap > SysCfg.LPRDelayTime_Ms)
			&&(LaneClass->LprRes == NOLPRRES) )
	{
		LaneClass->Obj_Snap[0].t_snap=LaneClass->Time_SnapRedge;	/* ʱ�丳ֵ */
		strcpy(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ");			/* ֻ��ֵһ�� */

		#if 1	//_by_dpj_2015-01-08
		if( LaneClass->LaneCfg->Exemption == CAREXEMPTION )
		{
			if( LaneClass->WorkMode == Exemption )
			{
				if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: ����������칤��ģʽ��%s�����˲�̧��", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
				}
			}
		}
		#endif

		LaneClass->LprRes = HAVE_PROLPR;							/* ��λ�г���ʶ�����־ */
		printf("[LPR:]��������ʶ������޶�ʱ�䣬û��ʶ�������Զ��������ƾ�ʶ\r\n");
		switch(LaneClass->Snap_TagQuantity)
		{
			case ZEROTAG_0:
			case ZEROTAG_2_N:
			case ZEROTAG_2_Y_N:
			case ZEROTAG_M2:
			{
				LaneClass->Obj_Snap[0].acctype=TEMPVM;
				LaneClass->CmfType = LPR_CFM;
				break;
			}
			case ONETAG_1:
			case ONETAG_2_Y_Y:
			case ONETAG_2_YoN:
			{
				if(LaneClass->CmfType != RFID_CFM)
				{
					LaneClass->CmfType = LPR_CFM;
				}
				break;
			}
			case TWOTAGS:
			{
				LaneClass->CmfType = LPR_CFM;
				break;
			}
		}
		ar_2FIFO(LaneClass);
	}

	return;
}

/************************************************************************************
* Judge_LPRUpRail
* DESCRIPTION: ץ����Ȧ������
* @LaneClass : ��������
* @iList	 : ʶ�����
* @Return 	 : no
* ----
*************************************************************************************/

void Judge_LPRUpRail_Export(laneclass_type * LaneClass,identity_node *iList)
{
	/* ʱ���ۼ���С�ڵ���ץ�ĺ���ʱ�䣬�г���ʶ�������Ҹý��δ������*/
	if( (LaneClass->TimeAcc_Snap <= SysCfg.LPRDelayTime_Ms)		/* ����������ִֻ��һ�� */
		&& (strlen(LaneClass->Obj_Snap[0].plateno) > 0)			/* �г���ʶ���� */
		&& (LaneClass->LprRes == NOLPRRES ) )
	{
		struct  timeval t;
		char buf[32];
		printf("����ʶ������:%s\r\n", LaneClass->Obj_Snap[0].plateno);

		/********************************************************************************
		����ʶ����_by_dpj_2014-09-29
		����: �ж��Ƿ���ʶ�����Ƿ�Ϊ���ƾ�ʶ
		********************************************************************************/
		if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )
		{
			writelog(LaneLogLevel(LaneClass),"����%d: ����һ��ʶ���г��ƽ��Ϊ���ƾ�ʶ, ���Խ������ʶ�����", LaneClass->LaneCfg->LaneNO);
			#if 1	//_by_dpj_2014-10-09	�޸�ԭ��: �����ܿ�����������Ϊ˫��ڣ�˫���ڣ�ʱ���ᵼ�¶���ʶ�����̴���
			CaptureImage(LaneClass->LPR->IP);
			#endif
			sleep(1);
		}
		/******************************************************************************
		����ʶ���ܽ���_by_dpj_2014-09-29
		*******************************************************************************/

		#if 1	//_by_dpj_2015-01-08
		if( LaneClass->LaneCfg->Exemption == CAREXEMPTION )
		{
			if( LaneClass->WorkMode == Exemption )
			{
				if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: ����������칤��ģʽ��%s�����˲�̧��", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
				}
				else if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") != 0 )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: ����������칤��ģʽ��%s������̧��", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown, RELEASE_DOWNRAIL);		/* �ͷ���� */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, UPRAIL);					/* ̧�˷��� */
				}
			}
		}
		#endif

		LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;
		LaneClass->LprRes = HAVE_PROLPR;

		/* ��������Ϣ��RFIDʶ������б�����ͨ�б�����ͨ��ʱ�����ٽ��ж���ʶ��*/
		#if 1
		if( LaneClass->Note == LIMIT_PASS )
		{
			writelog(LaneLogLevel(LaneClass),"����%d: �����ó���ʶ���������ó���RFID�����б�����ͨ�У������г��ƽ�һ��ʶ����", LaneClass->LaneCfg->LaneNO);
			return;
		}
		#endif

		switch( LaneClass->Snap_TagQuantity )
		{
			case ZEROTAG_0:
			case ZEROTAG_2_N:
			case ZEROTAG_2_Y_N:
			case ZEROTAG_M2:
			{
				LaneClass->CmfType = LPR_CFM;				//ʶ��ģʽ��־ΪLPRʶ��

				if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )			/* ����Ϊ���ƾ�ʶ����ֹͨ��*/
				{
					LaneClass->Obj_Snap[0].acctype = TEMPVM;
					writelog(LaneLogLevel(LaneClass),"����%d: ���ƾ�ʶ����ֹͨ��", LaneClass->LaneCfg->LaneNO);
					break;
				}
				else			/* ��ʶ���������Ϣ*/
				{
					LPR_Process(LaneClass);

					int j=-1;
					j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);
					if( j == EXIST_ALLOWPASS )
					{
						writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��", LaneClass->LaneCfg->LaneNO);

						/********************************̧�˶����ж�*************************************************/
						if( LaneClass->WorkMode == Common )
						{
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/* �ͷ���� */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* ̧�˷��� */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/* �ͷű��� */
							gettimeofday(&t,NULL);
							Chg_PreciseTime2Str(t, buf, sizeof(buf));

							writelog(LaneLogLevel(LaneClass),"����%d: ����̧��", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == FreePass )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ��������������ģʽ�����˳�̧", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Deployment )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ���ڲ�������ģʽ�����˲�����̧", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Exemption )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ������칤��ģʽ�������Ѷ���", LaneClass->LaneCfg->LaneNO);
						}
						LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
						/********************************************************************************************************/

						/* ����з��鵽��ǩ�����뵽�˲����� */
						if( strlen(LaneClass->Obj_Snap[0].tagno) > 0 )
						{
							Insert_FilterDupNode(Filter_DupList,
												LaneClass->Obj_Snap[0].tagno,
												LaneClass->Obj_Snap[0].antennaID,
												LaneClass->Obj_Snap[0].t_snap);
						}
						if( strlen(LaneClass->Obj_Snap[1].tagno) > 0 )
						{
							Insert_FilterDupNode(Filter_DupList,
												LaneClass->Obj_Snap[1].tagno,
												LaneClass->Obj_Snap[1].antennaID,
												LaneClass->Obj_Snap[1].t_snap);
						}
					}
					else
					{
						LaneClass->Note = LIMIT_PASS;
						writelog(LaneLogLevel(LaneClass),"����%d: �ó���ֹͨ��", LaneClass->LaneCfg->LaneNO);
					}
				}

				break;
			}
			case ONETAG_1:
			case ONETAG_2_Y_Y:
			case ONETAG_2_YoN:
			{
				if( LaneClass->Obj_Snap[0].FlagUpRail == NO_URAILl )		/* ������������û̧�ˣ��������˻�����ͨ�е�*/
				{
					if( LaneClass->CmfType == NONE_CFM )		/* δ����ȷʶ��*/
					{
						LaneClass->CmfType = LPR_CFM;
					}

					if( LaneClass->Obj_Snap[0].acctype != TEMPVM ) 	 /* ����ʱ���������ٴ��ó���ȷ�� */
					{
						writelog(LaneLogLevel(LaneClass),"����%d: 1�ſ����ó�Ϊ����ʱ�������賵��ʶ��ȷ��", LaneClass->LaneCfg->LaneNO);
						break;
					}

					if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )  /* RFID��̧�ˣ��������ž�ʶ����������ֱ���ϴ� */
					{
						writelog(LaneLogLevel(LaneClass),"����%d: 1�ſ�������ʶ��Ϊ���ƾ�ʶ��������", LaneClass->LaneCfg->LaneNO);
						break;
					}

					#if 1			/* �������־�ж�ԭ��*/
					if( (strcmp(LaneClass->Obj_Snap[0].plateno, iList->Next->tagplateno) == 0 )
					   && (strcmp(LaneClass->Obj_Snap[0].platecl,iList->Next->tagplatecl) == 0) )
					{
						writelog(LaneLogLevel(LaneClass),"����%d: 1�ſ���RFID���鳵����LPRʶ����һ��", LaneClass->LaneCfg->LaneNO);
					}
					else
					{
						writelog(LaneLogLevel(LaneClass),"����%d: 1�ſ���RFID���鳵����LPRʶ���Ʋ�һ��", LaneClass->LaneCfg->LaneNO);
					}
					#endif

					LaneClass->CmfType = LPR_CFM;
					LPR_Process(LaneClass);

					int j = -1;
					j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);

					/* ��������ͨ�л��߲�����,����Ϊ�ǿ���ͨ�е� */
					if( j == EXIST_ALLOWPASS )
					{
						writelog(LaneLogLevel(LaneClass),"����%d: �ó���Ϣ����ͨ��", LaneClass->LaneCfg->LaneNO);

						/********************************̧�˶����ж�*************************************************/
						if( LaneClass->WorkMode == Common )
						{
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/* �ͷ���� */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* ̧�˷��� */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/* �ͷű��� */
							gettimeofday(&t,NULL);
							Chg_PreciseTime2Str(t, buf, sizeof(buf));

							writelog(LaneLogLevel(LaneClass),"����%d: ����̧��", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == FreePass )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ��������������ģʽ�����˳�̧", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Deployment )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ���ڲ�������ģʽ�����˲�����", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Exemption )
						{
							writelog(LaneLogLevel(LaneClass),"����%d: LPR: ������칤��ģʽ�������Ѷ���", LaneClass->LaneCfg->LaneNO);
						}
						LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
						/********************************************************************************************************/

						if( strlen(LaneClass->Obj_Snap[0].tagno) > 0 )		/* ����з��鵽��ǩ�����뵽�˲����� */
						{
							Insert_FilterDupNode(Filter_DupList,
												LaneClass->Obj_Snap[0].tagno,
												LaneClass->Obj_Snap[0].antennaID,
												LaneClass->Obj_Snap[0].t_snap);
						}
						if ( strlen(LaneClass->Obj_Snap[1].tagno) > 0 )
						{
							Insert_FilterDupNode(Filter_DupList,
												LaneClass->Obj_Snap[1].tagno,
												LaneClass->Obj_Snap[1].antennaID,
												LaneClass->Obj_Snap[1].t_snap);
						}
					}
					else
					{
						LaneClass->Note = LIMIT_PASS;
						writelog(LaneLogLevel(LaneClass),"����%d: �ó���ֹͨ��", LaneClass->LaneCfg->LaneNO);
					}
				}

				break;
			}
			case TWOTAGS:
			{
				break;
			}
		}

		ar_2FIFO(LaneClass);
	}
	else if( (LaneClass->TimeAcc_Snap > SysCfg.LPRDelayTime_Ms)
			&&(LaneClass->LprRes == NOLPRRES) )
	{
		LaneClass->Obj_Snap[0].t_snap=LaneClass->Time_SnapRedge;	/* ʱ�丳ֵ */
		strcpy(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ");			/* ֻ��ֵһ�� */

		#if 1	//_by_dpj_2015-01-08
		if( LaneClass->LaneCfg->Exemption == CAREXEMPTION )
		{
			if( LaneClass->WorkMode == Exemption )
			{
				if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") == 0 )
				{
					writelog(LaneLogLevel(LaneClass),"����%d: ����������칤��ģʽ��%s�����˲�̧��", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
				}
			}
		}
		#endif

		LaneClass->LprRes = HAVE_PROLPR;							/* ��λ�г���ʶ�����־ */
		writelog(LaneLogLevel(LaneClass),"����%d: ��������ʶ������޶�ʱ�䣬û��ʶ�������Զ��������ƾ�ʶ", LaneClass->LaneCfg->LaneNO);
		switch(LaneClass->Snap_TagQuantity)
		{
			case ZEROTAG_0:
			case ZEROTAG_2_N:
			case ZEROTAG_2_Y_N:
			case ZEROTAG_M2:
			{
				LaneClass->Obj_Snap[0].acctype=TEMPVM;
				LaneClass->CmfType = LPR_CFM;
				break;
			}
			case ONETAG_1:
			case ONETAG_2_Y_Y:
			case ONETAG_2_YoN:
			{
				if(LaneClass->CmfType != RFID_CFM)
				{
					LaneClass->CmfType = LPR_CFM;
				}
				break;
			}
			case TWOTAGS:
			{
				LaneClass->CmfType = LPR_CFM;
				break;
			}
		}
		ar_2FIFO(LaneClass);
	}

	return;
}

/************************************************************************************
* SnapCoilEdge
* DESCRIPTION: ץ����Ȧ������
* @GPISnap 	 : ץ����Ȧ״̬�洢��
* @LaneClass : ��������
* @iList	 : ʶ�����
* @Return 	 : no
* ----
*************************************************************************************/

void SnapCoilEdge(GPI_StatAndTime *GPISnap, GPI_StatAndTime *GPIRailStat,
						laneclass_type *LaneClass, identity_node *iList, laneclass_type *LaneClassOther)
{
	/* ץ����Ȧ�����ض�������*/
	if( (GPISnap->LState == LOWLEVEL) && (GPISnap->State == HIGHTLEVEL) )
	{
		char buf[32];
		gettimeofday(&(LaneClass->Time_SnapRedge), NULL);	/* ��ȡѹץ����Ȧʱ�� */
		LaneClass->Lane_Info.SnapCoil = HIGHTLEVEL;		/* ����״̬�е�ץ����Ȧ״̬Ϊ�ߵ�ƽ*/
		LaneClass->TimeAcc_Snap = 0;						/* ��ն�ʱ�ۼ��� */
		LaneClass->CmfType = NONE_CFM;					/* ʶ��ģʽΪ��ʶ��ģʽ*/
		LaneClass->Obj_Snap[0].FlagUpRail = NO_URAILl;		/* ��ʼ��̧�˱�ʶΪ��̧��*/
		LaneClass->Flag_UpLoad = LOWLEVEL;				/* ��ʼ���ϴ�������¼��־Ϊ�޼�¼�ϴ�*/	//_by_dpj_2014-09-25
		LaneClass->Note = NO_NOTE;						/* ��ʼ����ע */
		LaneClass->Snap_TagQuantity = ZEROTAG_0;			/* ���ſ� */
		LaneClass->LprRes = NOLPRRES;					/* ��ʼ������ʶ��û�н�� */
		memset(LaneClass->ImageIndex, 0, sizeof(LaneClass->ImageIndex));

		LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;
		memset(buf, 0, sizeof(buf));
		Chg_PreciseTime2Str(LaneClass->Time_SnapRedge, buf, sizeof(buf));
		writelog(LaneLogLevel(LaneClass), "����%d: ����ץ����Ȧ",LaneClass->LaneCfg->LaneNO);

		/********************************************************************************
		ͬ��ͬ������_by_dpj_2014-01-02
		����: �ж��Ƿ����Ȧ������
		********************************************************************************/
		printf("[SnapCoilEdge]:SysCfg.LaneLogic = %d\n", SysCfg.LaneLogic );
		if( SysCfg.LaneLogic == USE_LANELOCK )
		{
			printf("[SnapCoilEdge]:LaneClass->LaneLockStat = %d\n", LaneClass->LaneLockStat );
			if( LaneClass->LaneLockStat == LOCK )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ץ����Ȧ���������벻����",LaneClass->LaneCfg->LaneNO);
				return;	/* ����ó��������������������� */
			}
		}
		/******************************************************************************
		ͬ��ͬ�����ܽ���_by_dpj_2014-01-02
		*******************************************************************************/

		/* ������RFIDʱ�ĳ�����������*/
		if( LaneClass->LaneCfg->RfidCfg == HAVECFG )
		{
			printf("[SnapCoilEdge]:LaneClass->LaneCfg->Direction is %d\n", LaneClass->LaneCfg->Direction);
			if( LaneClass->LaneCfg->Direction == ENTRY )
			{
				Judge_RFIDUpRail_Entry(LaneClass, iList);
			}
			else
			{
				Judge_RFIDUpRail_Export(LaneClass, iList);
			}
		}
		else
		{
			LaneClass->CmfType = LPR_CFM;
			//LaneClass->Procedure = RFID_NOTAG;
		}

	}

	//=================================================
	//ʱ�����,����ʶ��������
	//=================================================
	if(GPISnap->State == HIGHTLEVEL)
	{
		struct  timeval t;
		gettimeofday(&t,NULL);										/* ѹץ����Ȧʱ����� */
		LaneClass->TimeAcc_Snap =
			(t.tv_sec - LaneClass->Time_SnapRedge.tv_sec)*1000
			+ (t.tv_usec - LaneClass->Time_SnapRedge.tv_usec)/1000;
		if(LaneClass->LaneCfg->LprCfg == HAVECFG)
		{
			if(LaneClass->LaneCfg->Direction == ENTRY)
			{
				Judge_LPRUpRail_Entry(LaneClass,iList);
			}
			else
			{
				Judge_LPRUpRail_Export(LaneClass,iList);
			}
		}
	}

	/* ץ����Ȧ�½��ض�������*/
	if((GPISnap->LState==HIGHTLEVEL)&&(GPISnap->State==LOWLEVEL))
	{
		LaneClass->Lane_Info.SnapCoil=LOWLEVEL;			/* ����״̬��Ϣ */

		writelog(LaneLogLevel(LaneClass),"����%d: �뿪ץ����Ȧ",LaneClass->LaneCfg->LaneNO);

		/********************************************************************************
		ͬ��ͬ������_dpj_2014-01-02
		����: �ж��Ƿ����Ȧ������
		********************************************************************************/
		if( SysCfg.LaneLogic == USE_LANELOCK )
		{
			if( LaneClass->LaneLockStat == LOCK )		/* ����ó��������������������� */
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ץ����Ȧ�������뿪������",LaneClass->LaneCfg->LaneNO);
				return;
			}
		}
		/******************************************************************************
		ͬ��ͬ�����ܽ���_dpj_2014-01-02
		*******************************************************************************/

		LaneClass->TimeAcc_Snap=0;

#if 0	//_by_dpj_2014-09-29
		/********************************************************************************
		��������_by_lsm_2013-12-26
		����ͨ�������
		����: �����뿪ץ����Ȧ����������Ȧ
		********************************************************************************/
		if( LaneClass->Flag_HightSnapRail == HIGHTLEVEL
			&& LaneClass->Lane_Info.RailCoil == HIGHTLEVEL )
			//&& GPIRailStat->State==LOWLEVEL )				//_by_dpj_2014-05-07
		{
			//����+1
			printf("+1\n");
			gettimeofday(&(LaneClass->Time_VehicleCount),NULL);		//_by_dpj_2014_02_25
			InsLocDB_Vehicles(LaneClass);			//_dpj_2013-12-31
			In_LogQueue(logqueue, "����%d: ��������+1\r\n", LaneClass->LaneCfg->LaneNO,NULL,NULL, 9999, LOG_INFO);	//_by_dpj_2014-01-02
			parknum_2FIFO(LaneClass);
			//parknum_2FIFO(LaneClass->LaneCfg);
		}
		/******************************************************************************
		�������ܽ���_by_lsm_2013-12-26
		*******************************************************************************/

#endif
#if 1	//_by_dpj_2014-09-29
		/********************************************************************************
		��������_by_dpj_2014-08-04
		����ͨ�������
		����: �����뿪ץ����Ȧ����������Ȧ
		********************************************************************************/
		if( SysCfg.ParknumType == TWO_COIL )
		{
			if( LaneClass->Flag_HightSnapRail == HIGHTLEVEL
				&& LaneClass->Lane_Info.RailCoil == HIGHTLEVEL)
				//&& LaneClass->Lane_Info.Rail == LOWLEVEL )  _by_dpj_2014-04-22
			{
				//����+1
				printf("+1\n");
				gettimeofday(&(LaneClass->Time_VehicleCount),NULL);		//_by_dpj_2014_02_25
				InsLocDB_Vehicles(LaneClass);			//_dpj_2013-12-31
				writelog(LaneLogLevel(LaneClass),"����%d: ��������+1",LaneClass->LaneCfg->LaneNO);
				parknum_2FIFO(LaneClass);
			}
		}
		else if( SysCfg.ParknumType == ONE_COIL )
		{
			if( strcmp(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ") != 0 )
			{
				//����+1
				printf("+1\n");
				gettimeofday(&(LaneClass->Time_VehicleCount),NULL);		//_by_dpj_2014_02_25
				InsLocDB_Vehicles(LaneClass);			//_dpj_2013-12-31
				writelog(LaneLogLevel(LaneClass),"����%d: ��������+1",LaneClass->LaneCfg->LaneNO);
				parknum_2FIFO(LaneClass);
			}
		}
		/******************************************************************************
		�������ܽ���_by_dpj_2014-08-04
		*******************************************************************************/
#endif

		#if 1		//_by_dpj_2014-09-25
		//�޸�ԭ��: �����޽���ҳ�ѹץ����Ȧʱ��С�ڳ���ʶ�𸺰뾶ʱ��ʱ���޹�����¼����
		if(LaneClass->Flag_UpLoad == LOWLEVEL)
		{
			if(strlen(LaneClass->Obj_Snap[0].plateno)==0)
			{
				strcpy(LaneClass->Obj_Snap[0].plateno, "���ƾ�ʶ");
				LaneClass->Obj_Snap[0].acctype = TEMPVM;	//_by_dpj_2014-11-24	�޸�ԭ��:  ���ٽ����뿪ץ��ʱ��û�г��ƻص������û���ϴ���¼�������˺�����Ϊ0
			}
			ar_2FIFO(LaneClass);
		}
		#endif


		Delete_SExitDupNode(Exist_DupList, (identity_node *)&(LaneClass->Obj_Snap));	//_by_dpj_2013-08-05,�������һ����δ�������
		//Clear_IdentityDupList(iList);  //shl_2car_debug_20131017
		memset(&(LaneClass->Obj_Snap[0]),0,sizeof(identity_node));		/* ץ����Ȧ�½��أ������ץ��ʶ����� */
		memset(&(LaneClass->Obj_Snap[1]),0,sizeof(identity_node));		/* ץ����Ȧ�½��أ������ץ��ʶ����� */

		LaneClass->Obj_Snap[0].FlagUpRail = NO_URAILl;				//_by_dpj_2015-01-04
	}


	//_by_lsm_2013-09-22,��������ص�����ɶ�������δɾ������
/*	if((GPISnap->LState==LOWLEVEL)&&(GPISnap->State==LOWLEVEL)
		&&(strlen(LaneClass->Obj_Snap.plateno) > 0))
		{
			memset(LaneClass->Obj_Snap.plateno,0,sizeof(LaneClass->Obj_Snap.plateno));
		}*/
}

//=======================================================================
//�ش����Ӻ�����������Ȧ�������½���
//=======================================================================
void RailCoilEdge(GPI_StatAndTime *GPISnap,GPI_StatAndTime *GPIRailStat,GPI_StatAndTime *GPIRail,laneclass_type *LaneClass,laneclass_type *LaneClassOther)
{

	if((GPIRail->LState==LOWLEVEL)&&(GPIRail->State==HIGHTLEVEL))
	{
		LaneClass->Lane_Info.RailCoil=HIGHTLEVEL;		/* ����״̬��Ϣ */

		writelog(LaneLogLevel(LaneClass),"����%d: ��������������Ȧ",LaneClass->LaneCfg->LaneNO);

		/********************************************************************************
		ͬ��ͬ������_dpj_2014-01-02
		����: �ж��Ƿ����Ȧ������
		********************************************************************************/
		if( SysCfg.LaneLogic == USE_LANELOCK )
		{
			if( LaneClass->LaneLockStat == LOCK )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ������Ȧ���������벻����",LaneClass->LaneCfg->LaneNO);
				return;			/* ��������������ڣ��˳� */
			}
			else if( GPISnap->State == HIGHTLEVEL )		/* ���û����ס��������Ȧ������ʱ */
			{
				writelog(LaneLogLevel(LaneClass), "����%d: ������%d",LaneClass->LaneCfg->LaneNO,LaneClassOther->LaneCfg->LaneNO);
				LaneClassOther->LaneLockStat = LOCK;			/* ����ó���ץ����Ȧ���޳������ */
															/* ʾ�ǶԷ������������ĳ� */
			}
		}
		/******************************************************************************
		ͬ��ͬ�����ܽ���_dpj_2014-01-02
		*******************************************************************************/

#if 1	//_by_dpj_2013-4-22	������2 57�ŵؿ�ʵ���������right//_by_pbh_2015-01-14�޸��ͷ�̧�˵���"����������Ȧ"
		if( LaneClass->WorkMode != FreePass )
		{
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,RELEASE_UPRAIL);	/* �ͷ�̧�� */
			writelog(LaneLogLevel(LaneClass),"����%d: �������ͷ�̧��",LaneClass->LaneCfg->LaneNO);
		}
#endif

		#if 1		//_by_dpj_2013-5-3			����ж��Ƿ���������
		if( LaneClass->Lane_Info.RailCoil == HIGHTLEVEL &&
			LaneClass->Lane_Info.SnapCoil == HIGHTLEVEL )
		{
			LaneClass->Flag_HightSnapRail=HIGHTLEVEL;
			writelog(LaneLogLevel(LaneClass),"����%d: �ó�Ϊ����",LaneClass->LaneCfg->LaneNO);
		}
		else
		{
			LaneClass->Flag_HightSnapRail=LOWLEVEL;
			writelog(LaneLogLevel(LaneClass),"����%d: �ó�Ϊ������",LaneClass->LaneCfg->LaneNO);
		}
		#endif

		LaneClass->Obj_Rail[0] = LaneClass->Obj_Snap[0];
		LaneClass->Obj_Rail[1] = LaneClass->Obj_Snap[1];

		#if 0
		memset(s1, 0 , sizeof(s1));
		memset(s2, 0, sizeof(s2));
		Chg_PreciseTime2Str(LaneClass->Obj_Rail[0].t_snap, s1, sizeof(s1));
		Chg_PreciseTime2Str(LaneClass->Obj_Snap[0].t_snap, s2, sizeof(s2));
		printf("LaneClass->Flag_HightSnapRail is %d\n", LaneClass->Flag_HightSnapRail);
		printf("LaneClass->Lane_Info.Rail is %d\n", LaneClass->Lane_Info.Rail);
		printf("LaneClass->Obj_Rail.t_snap is %s\n", s1);
		printf("LaneClass->Obj_Snap.t_snap is %s\n", s2);
		#endif

		/* ����ö���ᵼ�¼����޷����ж�����ʱ��Ƚϣ��������޷���������*/
		#if 0		//_by_shl_2014-02-28	error
		memset(&(LaneClass->Obj_Snap[0]),0,sizeof(identity_node));	/* �������ץ����Ȧ�ϵĶ��� */
		memset(&(LaneClass->Obj_Snap[1]),0,sizeof(identity_node));
		#endif
	}
	else if((GPIRail->LState==HIGHTLEVEL)&&(GPIRail->State==LOWLEVEL))
	{
		LaneClass->Lane_Info.RailCoil=LOWLEVEL;						/* ����״̬��Ϣ */

		writelog(LaneLogLevel(LaneClass),"����%d: �뿪������Ȧ\r\n",LaneClass->LaneCfg->LaneNO);

		#if 1
		/********************************************************************************
		ͬ��ͬ������_dpj_2014-01-02
		����: �ж��Ƿ����Ȧ������
		********************************************************************************/
		if( SysCfg.LaneLogic == USE_LANELOCK )
		{
			printf("[RailCoilEdge]:LaneClass->LaneLockStat = %d\n", LaneClass->LaneLockStat );
			if( LaneClass->LaneLockStat == LOCK )
			{
				writelog(LaneLogLevel(LaneClass),"����%d: ������Ȧ�������뿪������",LaneClass->LaneCfg->LaneNO);
				//LaneClass->LaneLockStat = UNLOCK;		/* �������ܼ��е�һ��freeLockedȥ�ж�*/
				return;
			}
		}
		/******************************************************************************
		ͬ��ͬ�����ܽ���_dpj_2014-01-02
		*******************************************************************************/
		#endif

		/********************************************************************************
		��������_by_dpj_2014-08-04
		���������
		����: ����̧��, �����뿪������Ȧ, ץ�Ķ���������˶���
		********************************************************************************/
		if( SysCfg.ParknumType == TWO_COIL )
		{
			memset(s1, 0 , sizeof(s1));
			memset(s2, 0, sizeof(s2));
			Chg_PreciseTime2Str(LaneClass->Obj_Rail[0].t_snap, s1, sizeof(s1));
			Chg_PreciseTime2Str(LaneClass->Obj_Snap[0].t_snap, s2, sizeof(s2));
			#if 0
			printf("LaneClass->Flag_HightSnapRail is %d\n", LaneClass->Flag_HightSnapRail);
			printf("LaneClass->Lane_Info.Rail is %d\n", LaneClass->Lane_Info.Rail);
			printf("LaneClass->Obj_Rail.t_snap is %s\n", s1);
			printf("LaneClass->Obj_Snap.t_snap is %s\n", s2);
			#endif
			if( LaneClass->Flag_HightSnapRail == HIGHTLEVEL
				&& strcmp(s1, s2) == 0 )
				//&& LaneClass->Lane_Info.Rail == LOWLEVEL )  _by_dpj_2014-04-22
			{
				//����+1
				printf("+1\n");
				gettimeofday(&(LaneClass->Time_VehicleCount),NULL);		//_by_dpj_2014_02_25
				InsLocDB_Vehicles(LaneClass);			//_dpj_2013-12-31
				writelog(LaneLogLevel(LaneClass),"����%d: ��������+1",LaneClass->LaneCfg->LaneNO);
				parknum_2FIFO(LaneClass);
			}
		}
		/******************************************************************************
		�������ܽ���_by_dpj_2014-08-04
		*******************************************************************************/

		if(LaneClass->Flag_HightSnapRail==HIGHTLEVEL)					/* ͳ�� */
		{
			#if 0 //_shl_Cancel_Statistics_20140212
			LaneClass->Flag_HightSnapRail=LOWLEVEL;
			if(LaneClass->Obj_Rail[0].ReadCardMom == NoCard){
				InsLocDB_Statistics(LaneClass->LaneCfg->RFIDAntID,NoCard);	/* �޿�����antennaID */
			}
			else{															/* �п��� */
				InsLocDB_Statistics(LaneClass->Obj_Rail[0].antennaID,
					LaneClass->Obj_Rail[0].ReadCardMom);
			}
			#endif
		}

		//by_dpj_2013-4-19	���԰���ڲ��Է��ָö�����������error
		#if 0	//question
		if((GPISnap ->State != HIGHTLEVEL)  || (LaneClass->FlagUpRail  != HAD_URAILl))
		{
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,RELEASE_UPRAIL);	/* �ͷ�̧�� */
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ��� */
		}
		#endif

		#if 0	//_by_dpj_2015-01-04 //_by_pbh_2015-01-14�޸��ͷ�̧�˵���"����������Ȧ"
		if( LaneClass->WorkMode != FreePass )
		{
			if( LaneClass->LaneCfg->LprCfg == HAVECFG )
			{
				if( (strcmp(LaneClass->Obj_Rail[0].plateno, LaneClass->Obj_Snap[0].plateno) == 0)
					&& (strcmp(LaneClass->Obj_Rail[0].platecl, LaneClass->Obj_Snap[0].platecl) == 0) )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/* �ͷ�̧�� */
					//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ��� */
					writelog(LaneLogLevel(LaneClass),"����%d: �����ͷ�̧��",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->Obj_Snap[0].FlagUpRail == NO_URAILl)
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/* �ͷ�̧�� */
					//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ��� */
					writelog(LaneLogLevel(LaneClass),"����%d: �ͷ�̧��",LaneClass->LaneCfg->LaneNO);
				}

			}
			else if( LaneClass->LaneCfg->RfidCfg == HAVECFG )
			{
				if( (strcmp(LaneClass->Obj_Rail[0].tagno, LaneClass->Obj_Snap[0].tagno) == 0)
					|| ((strcmp(LaneClass->Obj_Rail[0].tagplateno, LaneClass->Obj_Snap[0].tagplateno) == 0)
					&& (strcmp(LaneClass->Obj_Rail[0].tagplatecl, LaneClass->Obj_Snap[0].tagplatecl) == 0)) )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/* �ͷ�̧�� */
					//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ��� */
					writelog(LaneLogLevel(LaneClass),"����%d: �����ͷ�̧��",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->Obj_Snap[0].FlagUpRail == NO_URAILl)
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/* �ͷ�̧�� */
					//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ��� */
					writelog(LaneLogLevel(LaneClass),"����%d: �ͷ�̧��",LaneClass->LaneCfg->LaneNO);
				}

			}
			#if 0	//_by_dpj_2014-10-03
			if( LaneClass->Obj_Snap.FlagUpRail == LOWLEVEL )
			{
				WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/* �ͷ�̧�� */
				//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ��� */
				printf("����%d: �ͷ�̧��\n");
				In_LogQueue(logqueue, "����%d: �ͷ�̧��\n", LaneClass->LaneCfg->LaneNO,NULL,NULL, 9999, LOG_INFO);
			}
			#endif
		}
		#endif

		memset(&(LaneClass->Obj_Rail[0]),0,sizeof(identity_node));	/* �������ץ����Ȧ�ϵĶ��� */
		memset(&(LaneClass->Obj_Rail[1]),0,sizeof(identity_node));	/* �������ץ����Ȧ�ϵĶ��� */
	}

}
//=======================================================================
//�ش����Ӻ���������״̬�������½���
void RailStateEdge(GPI_StatAndTime *GPIRailStat,laneclass_type *LaneClass)
{
	if((GPIRailStat->LState==LOWLEVEL)&&(GPIRailStat->State==HIGHTLEVEL))		/* ��ȡ����״̬ */
	{
		#if 0
		/********************************************************************************
		ͬ��ͬ������_dpj_2014-01-02
		����: �ж��Ƿ����Ȧ������
		********************************************************************************/
		if(SysCfg.LaneLogic==USE_LANELOCK)
		{
			if(LaneClass->LaneLockStat==LOCK)return;
		}
		/******************************************************************************
		ͬ��ͬ�����ܽ���_dpj_2014-01-02
		*******************************************************************************/
		#endif

		LaneClass->Lane_Info.Rail=LOWLEVEL;
		writelog(LaneLogLevel(LaneClass),"����%d: ����״̬Ϊ�͵�ƽ",LaneClass->LaneCfg->LaneNO);
	}
	else if((GPIRailStat->LState==HIGHTLEVEL)&&(GPIRailStat->State==LOWLEVEL))		/* ��ȡ����״̬ */
	{
		#if 0
		/********************************************************************************
		ͬ��ͬ������_dpj_2014-01-02
		����: �ж��Ƿ����Ȧ������
		********************************************************************************/
		if(SysCfg.LaneLogic==USE_LANELOCK)
		{
			if(LaneClass->LaneLockStat==LOCK)return;
		}
		/******************************************************************************
		ͬ��ͬ�����ܽ���_dpj_2014-01-02
		*******************************************************************************/
		#endif

		LaneClass->Lane_Info.Rail=HIGHTLEVEL;
		writelog(LaneLogLevel(LaneClass),"����%d: ����״̬Ϊ�ߵ�ƽ",LaneClass->LaneCfg->LaneNO);
	}
}



//=======================================================================
//���¼�����
void Process_GPIEdge(void)
{
	unsigned char i;
	GPI_StatAndTime *p[2];
	GPI_StatAndTime *r[2];

	for(i=0;i<8;i++)
	{
		if(GPI[i] == LaneClass[0].LaneCfg->SnapCoil) p[0]=&(GPIStat[i]);
		else if(GPI[i] == LaneClass[1].LaneCfg->SnapCoil) p[1]=&(GPIStat[i]);
	}

	//by_lsm_20131226		��������
	for(i=0;i<8;i++)
	{
		if(GPI[i] == LaneClass[0].LaneCfg->RailStat) r[0]=&(GPIStat[i]);
		else if(GPI[i] == LaneClass[1].LaneCfg->RailStat) r[1]=&(GPIStat[i]);
	}

	/********************************************************************************
	ͬ��ͬ������_by_dpj_2014-01-02
	����: �ж��Ƿ����Ȧ������
	********************************************************************************/
	if( SysCfg.LaneLogic == USE_LANELOCK )
	{
		FreeLocked();
	}
	/******************************************************************************
	ͬ��ͬ�����ܽ���_by_dpj_2014-01-02
	*******************************************************************************/

	for(i=0;i<8;i++){
		if(LaneClass[0].LaneCfg->LaneOC == OPEN)
		{
			if( (LaneClass[0].Flag_Sleep == LOWLEVEL) && (Flag_SysWork == HIGHTLEVEL) )	//_by_dpj_2014-08-16
			{
				if(GPI[i] == SysCfg.Lane[0].SnapCoil)
					SnapCoilEdge(&(GPIStat[i]),r[0],&(LaneClass[0]),Identity_DupList[0],&(LaneClass[1]));

				else if(GPI[i] == SysCfg.Lane[0].RailCoil)
					RailCoilEdge(p[0],r[0],&(GPIStat[i]),&(LaneClass[0]),&(LaneClass[1]));

				else if(GPI[i] == SysCfg.Lane[0].RailStat)
					RailStateEdge(&(GPIStat[i]),&(LaneClass[0]));
			}
		}

		//====================================================================================
		if(LaneClass[1].LaneCfg->LaneOC == OPEN)
		{
			if( (LaneClass[1].Flag_Sleep == LOWLEVEL) && (Flag_SysWork == HIGHTLEVEL) )	//_by_dpj_2014-08-16
			{
				if(GPI[i] == SysCfg.Lane[1].SnapCoil)
					SnapCoilEdge(&(GPIStat[i]),r[1],&(LaneClass[1]),Identity_DupList[1],&(LaneClass[0]));

				else if(GPI[i] == SysCfg.Lane[1].RailCoil)
					RailCoilEdge(p[1],r[1],&(GPIStat[i]),&(LaneClass[1]),&(LaneClass[0]));

				else if(GPI[i] == SysCfg.Lane[1].RailStat)
					RailStateEdge(&(GPIStat[i]),&(LaneClass[1]));
			}

		}
	}

}

/*****************************************************************************
* gpio_init
* DESCRIPTION: GPIO�ڳ�ʼ��
* @Param  : no
* @Return : no
******************************************************************************/
int gpio_init(void)
{
	memset(&GPIStat,0,sizeof(GPI_StatAndTime)*8);
	memset(&LaneClass,0,sizeof(laneclass_type)*2);

	unsigned char i;
	struct  timeval t;
	gettimeofday(&t,NULL);
	for(i=0;i<8;i++)
	{
		GPIStat[i].LEfficRedge_SysTime=t;
		GPIStat[i].LEfficFedge_SysTime=t;
	}
	gpo_write(GPO_L[8]);		//_by_dpj_2014-09-17	�޸�ԭ��: GPO��ʼ��ֵ����

	/* �������ڶ���,�˲����к�ʶ����� */
	//struct  timeval t; 	/* ��ȡ��ȷʱ�� */
	//gettimeofday(&t,NULL);
	Exist_DupList = Create_ExistDupList("Filter_tagno",0xff,t);
	Filter_DupList = Create_FilterDupList("Filter_tagno",0xff,t);

	Identity_DupList[0]=Create_IdentityDupList("identity_L0_tagno",SysCfg.Lane[0].RFIDAntID,t);
	Identity_DupList[1]=Create_IdentityDupList("identity_L1_tagno",SysCfg.Lane[1].RFIDAntID,t);

	LaneClass[0].LaneCfg=&(SysCfg.Lane[0]);
	LaneClass[1].LaneCfg=&(SysCfg.Lane[1]);

	#if 1	//_by_dpj_2014-10-09
	LaneClass[0].LPR = &(SysCfg.LPR[0]);
	LaneClass[1].LPR = &(SysCfg.LPR[1]);
	#endif

	LaneClass[0].Lane_Info.DoorNO = SysCfg.Lane[0].DoorNO;
	LaneClass[0].Lane_Info.LaneNO= SysCfg.Lane[0].LaneNO;
	LaneClass[1].Lane_Info.DoorNO = SysCfg.Lane[1].DoorNO;
	LaneClass[1].Lane_Info.LaneNO= SysCfg.Lane[1].LaneNO;

	LaneClass[0].WorkMode = Common;
	LaneClass[1].WorkMode = Common;
	LaneClass[0].Flag_OpenLane = HIGHTLEVEL;
	LaneClass[1].Flag_OpenLane = HIGHTLEVEL;

	#if 1		/* ��ʼ��������������ʹ��_by_dpj_2014-09-02*/
	LaneClass[0].cpass.fixvm=HIGHTLEVEL;
	LaneClass[0].cpass.tempvm=HIGHTLEVEL;
	LaneClass[0].cpass.vipvm=HIGHTLEVEL;
	LaneClass[0].cpass.maavm=HIGHTLEVEL;
	LaneClass[0].cpass.pervm=HIGHTLEVEL;
	LaneClass[1].cpass.fixvm=HIGHTLEVEL;
	LaneClass[1].cpass.tempvm=HIGHTLEVEL;
	LaneClass[1].cpass.vipvm=HIGHTLEVEL;
	LaneClass[1].cpass.maavm=HIGHTLEVEL;
	LaneClass[1].cpass.pervm=HIGHTLEVEL;
	#endif

	/* ��ʼ���ϴ�ʵʱ���ݶ��� */
	if( (upload_queue=(Fifo_Qnode*)malloc(sizeof(Fifo_Qnode))) == NULL )
	{
		writelog(ZLOG_LEVEL_SYS,"����ʵʱ�ϴ����е��ڴ�ռ�ʧ�ܣ��˴��˳�");		return -1;
	}
	Init_FifoQueue(upload_queue);
	writelog(ZLOG_LEVEL_SYS,"�ϴ�ʵʱ���ݶ��д�������ʼ���ɹ�!!");


	flag_BigLittleEndian=check_cpu_BigLitteEndian();

	return 0;
}

void FreeLocked(void)
{
	laneclass_type *planeclass;
	if( LaneClass[0].Lane_Info.SnapCoil == LOWLEVEL && LaneClass[0].Lane_Info.RailCoil == LOWLEVEL
		&& LaneClass[1].Lane_Info.SnapCoil == LOWLEVEL && LaneClass[1].Lane_Info.RailCoil == LOWLEVEL )
	{
		#if 0
		printf("LaneClass[0].Lane_Info.SnapCoil = %d\n",LaneClass[0].Lane_Info.SnapCoil);
		printf("LaneClass[0].Lane_Info.RailCoil = %d\n",LaneClass[0].Lane_Info.RailCoil);
		printf("LaneClass[1].Lane_Info.SnapCoil = %d\n",LaneClass[1].Lane_Info.SnapCoil);
		printf("LaneClass[1].Lane_Info.RailCoil = %d\n",LaneClass[1].Lane_Info.RailCoil);
		#endif

		if( LaneClass[0].LaneLockStat == LOCK )
		{
			LaneClass[0].LaneLockStat = UNLOCK;
			planeclass = &LaneClass[0];
			writelog(LaneLogLevel(planeclass), "����%d: ����", LaneClass[0].LaneCfg->LaneNO);
		}
		else if( LaneClass[1].LaneLockStat == LOCK )
		{
			LaneClass[1].LaneLockStat = UNLOCK;
			planeclass = &LaneClass[1];
			writelog(LaneLogLevel(planeclass), "����%d: ����", LaneClass[1].LaneCfg->LaneNO);
		}
	}
	return;
}



