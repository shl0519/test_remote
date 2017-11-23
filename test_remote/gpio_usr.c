/*********************************************************************************************
#####         Âé¶Èó®Ë∑ØÊ°•‰ø°ÊÅØÂ∑•Á®ãÊúâÈôêÂÖ¨Âè∏        #####
#####              ÂµåÂÖ•ÂºèÂºÄÂèëÂ∞èÁªÑ  		      #####
#####                        				  #####

* FileÔºö	usergpio_test.c  (s3c2440_user_gpio.c)
* Author:	Hailiang.su
* DescÔºö	user gpio test
* History:	May 11th 2012
*********************************************************************************************/


#include "include.h"
#include "uhfshell_usr.h"

void VehicleDetector(void);
void VehicleDetectorComm(bool VDState);

void FreeLocked(void);		//_by_dpj_2013-07-09

/* ∑Ω±„IO∂¡–¥ */
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
//∂¡»°GPIΩ”ø⁄

unsigned char gpi_read(void)
{
	int i;
	unsigned char j,k;
	j=0;
	k=0;
#ifdef _ZTE6600C2
	for(i=0;i<5;i++){			/* ∂¡»°±æµÿ ‰»Îø⁄ */
		j=GPIO_GET_INPUTSTAT(i);
		k=k|(j<<i);
	}
	//printf("xxxxxxxxxxxxxxx:%x\r\n",k);
	unsigned char m=0;
	if((SysCfg.Lane[0].NetGpioCfg==NOCFG)		/* ¡¨Ω”µΩÕ¯¬ÁIO∞Â£¨∂¡»° ‰»Î◊¥Ã¨£¨ºÊ»›Õ¨ ±π“…œ¡Ω∏ˆ∞Â◊” */
		&&(SysCfg.Lane[1].NetGpioCfg==NOCFG))
		return k;

	else if((SysCfg.Lane[0].NetGpioCfg==HAVECFG)/* ≥µµ¿0≈‰÷√Õ¯¬ÁIO∞Â£¨ƒ«√¥≥µµ¿0µƒGPI÷µ¥Ê»ÎµÕ4Œª */
		&&(SysCfg.Lane[1].NetGpioCfg==NOCFG))
	{
		m= ~ read_netgpi();						//µÕµÁ∆Ω”––ß£¨”ÎÕ¯¬ÁIOµƒ∂¡»°Ω·π˚œ‡∑¥
		k=((k<<4)&0xf0) | (m&0x0f);
		//printf("xxxxxxxxxxxxxxx:%x\r\n",k);
		return k;
	}

	else if((SysCfg.Lane[0].NetGpioCfg==NOCFG)	/* ≥µµ¿1≈‰÷√Õ¯¬ÁIO∞Â£¨ƒ«√¥≥µµ¿1µƒGPI÷µ¥Ê»Î∏ﬂ4Œª */
		&&(SysCfg.Lane[1].NetGpioCfg==HAVECFG))
	{
		m = ~ read_netgpi();
		k=(k&0x0f) | (m&0xf0);
		return k;
	}

	else if((SysCfg.Lane[0].NetGpioCfg==HAVECFG)/* ¡Ω∏ˆ∂º”–≈‰÷√£¨÷±Ω”∑µªÿ */
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
	if((SysCfg.Lane[0].NetGpioCfg==NOCFG)		/* ¡¨Ω”µΩÕ¯¬ÁIO∞Â£¨øÿ÷∆ ‰≥ˆ£¨ºÊ»›Õ¨ ±π“…œ¡Ω∏ˆ∞Â◊” */
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
/* »Áπ˚Õ‚≤ø∂ÃΩ”£¨Õ®π˝”≤º˛Ω·ππ£¨ ‰»Î0µΩ–æ∆¨ */
/* »Áπ˚Õ‚≤ø∂œø™£¨Õ®π˝”≤º˛Ω·ππ£¨ ‰»Î1µΩ–æ∆¨ */
/* ≥£Ã¨ «∂œø™£¨”–≥µ—πœﬂ»¶ ±£¨Õ‚≤øª·∂ÃΩ” */
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
		if(gpi_n->TimeCount>=GPIO_IntimalFilter_Time)/* GPIOƒ⁄≤„≤…—˘¥Œ ˝ */
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
			else if(((j&0xf0)!=0)&&((j&0x0f)!=0))gpo_reg=gpo_reg&j;//±Ì æ «–¥µÕµÁ∆Ω
			else gpo_reg=gpo_reg|j;//±Ì æ «–¥∏ﬂµÁ∆Ω

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
//≥µ≈∆ ∂±¥¶¿Ì£¨—π◊•≈ƒœﬂ»¶÷Æ∫Û£¨ ∂±≥ˆ≥µ≈∆∏˜÷÷≈–∂œ
void LPR_Process(laneclass_type *LaneClass)
{
	QueryRes queryfields;
	QueryRes queryfields_tmp;
	int querystate=0;
	memset(&queryfields,0,sizeof(QueryRes));
	memset(&queryfields_tmp,0,sizeof(QueryRes));
	querystate=LPR_QueryLocDB_VM(tab_name[FILENO_FIXVM], &queryfields, LaneClass);	/* ≤È—ØπÃ∂®≥µ±Ì */
	if(querystate == EXIST_EFFIC)
	{
		writelog(LaneLogLevel(LaneClass),"∏√≥µ≈∆‘⁄πÃ∂®≥µ±Ì--¥Ê‘⁄≤¢”––ß");
		#if 1
		if( queryfields.AccType==FIXVM )
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µŒ™πÃ∂®≥µ",LaneClass->LaneCfg->LaneNO);
			queryfields.AccType=FIXVM;
		}
		else if(queryfields.AccType==VIP)
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µŒ™VIP≥µ",LaneClass->LaneCfg->LaneNO);
			queryfields.AccType=VIP;
		}
		#endif
	}
	else if(querystate == EXIST_OVERDUE)	/* πÃ∂®≥µ¿Ô√Ê¥Ê‘⁄µ´π˝∆⁄¡À */
	{
		writelog(LaneLogLevel(LaneClass),"∏√≥µ≈∆‘⁄πÃ∂®≥µ±Ì--¥Ê‘⁄µ´π˝∆⁄");
		queryfields_tmp=queryfields;
		memset(&queryfields,0,sizeof(QueryRes));
		querystate=LPR_QueryLocDB_VM(tab_name[FILENO_MAAVM], &queryfields, LaneClass);	/* ≤È—Ø‘§‘º≥µ±Ì */
		if(querystate == EXIST_EFFIC)
		{
			writelog(LaneLogLevel(LaneClass),"∏√≥µ≈∆‘⁄‘§‘º≥µ±Ì--¥Ê‘⁄≤¢”––ß");
			queryfields.AccType=MAAVM;
		}
		else if(querystate == EXIST_OVERDUE)	/* »Áπ˚‘§‘º≥µ±Ì≤ª¥Ê‘⁄£¨ªÚ’ﬂ¥Ê‘⁄µ´Œﬁ–ß */
		{
			writelog(LaneLogLevel(LaneClass),"∏√≥µ≈∆‘⁄‘§‘º≥µ±Ì--¥Ê‘⁄µ´π˝∆⁄");
			queryfields=queryfields_tmp;
			queryfields.AccType=TEMPVM;
		}
		else {										/* »Áπ˚‘§‘º≥µ±Ì≤ª¥Ê‘⁄£¨ªÚ’ﬂ¥Ê‘⁄µ´Œﬁ–ß */
			writelog(LaneLogLevel(LaneClass),"∏√≥µ≈∆‘⁄‘§‘º≥µ±Ì--≤ª¥Ê‘⁄");
			queryfields=queryfields_tmp;			/* ƒ«√¥≤È—Øµƒ–≈œ¢“—πÃ∂®≥µ–≈œ¢±ÌŒ™◊º */
			queryfields.AccType=TEMPVM;				/* ¡Ÿ ±≥µ,√∂æŸ */
		}
	}
	else if(querystate == NOEXIST)	/* πÃ∂®≥µ≤ª¥Ê‘⁄∏√ø®∫≈ */
	{
		writelog(LaneLogLevel(LaneClass),"∏√≥µ≈∆‘⁄πÃ∂®≥µ±Ì--≤ª¥Ê‘⁄");
		memset(&queryfields,0,sizeof(QueryRes));
		//querystate=QueryLocDB_MaaVM(&queryfields,pe);
		querystate=LPR_QueryLocDB_VM(tab_name[FILENO_MAAVM], &queryfields, LaneClass);	/* ≤È—Ø‘§‘º≥µ±Ì */
		if(querystate == EXIST_EFFIC)	/* »Áπ˚‘§‘º≥µ±Ì¥Ê‘⁄”––ß£¨ƒ«√¥¿‡–ÕŒ™‘§‘º≥µ */
		{
			writelog(LaneLogLevel(LaneClass),"∏√≥µ≈∆‘⁄‘§‘º≥µ±Ì--¥Ê‘⁄≤¢”––ß");
			queryfields.AccType=MAAVM;
		}
		else if(querystate == EXIST_OVERDUE)	/* ¡Ÿ ±≥µπ˝∆⁄ */
		{
			writelog(LaneLogLevel(LaneClass),"∏√≥µ≈∆‘⁄‘§‘º≥µ±Ì--¥Ê‘⁄µ´π˝∆⁄");
			queryfields.AccType=TEMPVM;				/* ø®∫≈∫Õ≥µ≈∆∂º¥”‘§‘º≥µ±Ì¿Ô√Ê∂¡»° */
		}
		else if(querystate == NOEXIST)
		{
			writelog(LaneLogLevel(LaneClass),"∏√≥µ≈∆‘⁄‘§‘º≥µ±Ì--≤ª¥Ê‘⁄");
			strcpy(queryfields.TagNo,LaneClass->Obj_Snap[0].tagno);	/* ∂º≤ª¥Ê‘⁄£¨¥Ê‘⁄∂”¡–µƒø®∫≈∑≈»Î≤È—ØΩ·π˚Ω·ππÃÂ£¨÷ª”–ø®∫≈£¨√ª”–≥µ≈∆ */
			strcpy(queryfields.TagNo1,LaneClass->Obj_Snap[1].tagno);
			strcpy(queryfields.TagPlateNo,"");
			strcpy(queryfields.TagPlateColour,"");
			queryfields.AccType=TEMPVM;				/* »Áπ˚‘§‘º≥µ±ÌŒﬁ–ßªÚ’ﬂ≤ª¥Ê‘⁄£¨ƒ«√¥∂ºŒ™¡Ÿ ±≥µ */
		}
	}

	writelog(LaneLogLevel(LaneClass),"∏√≥µ±Í«©≥µ≈∆∫≈¬Î--%s%s",queryfields.TagPlateColour,queryfields.TagPlateNo);
	strcpy(LaneClass->Obj_Snap[0].tagno,queryfields.TagNo);/* ≤È—ØΩ·π˚∑≈»Îƒø±Í÷– */
	strcpy(LaneClass->Obj_Snap[0].tagplateno,queryfields.TagPlateNo);
	strcpy(LaneClass->Obj_Snap[0].tagplatecl,queryfields.TagPlateColour);
	LaneClass->Obj_Snap[0].acctype=queryfields.AccType;

	strcpy(LaneClass->Obj_Snap[1].tagno,queryfields.TagNo1);/* ≤È—ØΩ·π˚∑≈»Îƒø±Í÷– */
	strcpy(LaneClass->Obj_Snap[1].tagplateno,queryfields.TagPlateNo);//_by_shl_duptag_20140218
	strcpy(LaneClass->Obj_Snap[1].tagplatecl,queryfields.TagPlateColour);
	LaneClass->Obj_Snap[1].acctype=queryfields.AccType;

}
/************************************************************************************
* Judge_UpRail
* DESCRIPTION: ◊•≈ƒœﬂ»¶∂‘œÛ∫Ø ˝,º”»ÎºÊ»›À´±Í«©¬ﬂº≠¥¶¿Ì
* @LaneClass : ≥µµ¿∂‘œÛ
* @iList	 :  ∂±∂”¡–
* @Return 	 : no
* modify     : //_by_shl_inc_Compatible_with_double_label_20131213
*************************************************************************************/
void Judge_RFIDUpRail_Entry(laneclass_type *LaneClass, identity_node *iList)
{
	if( LaneClass->LaneCfg->Exemption == FULLEXEMPTION )		//_by_dpj_2015-01-08
	{
		if( LaneClass->WorkMode == Exemption )
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¥¶”⁄»´√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏ÀÃß∏À",LaneClass->LaneCfg->LaneNO);

			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown, RELEASE_DOWNRAIL);		/*  Õ∑≈¬‰∏À */
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, UPRAIL);					/* Ãß∏À∑≈–– */
		}
	}

	struct  timeval t;
	char buf[32];
	identity_node *pi;					/* «Â≥˝ ∂±∂”¡– ±º‰≥¨≥ˆ”––ß∑∂Œßµƒµ•‘™ */
	identity_node *pitmp;
	pi = iList->Next;
	/* Ãﬁ≥˝≥¨≥ˆ”––ß’˝∞Îæ∂ ±º‰µƒø® */
	while( pi != NULL )
	{
		pitmp = pi->Next;
		int j;
		j = (LaneClass->Time_SnapRedge.tv_sec - pi->t_tag.tv_sec)*1000
			+ (LaneClass->Time_SnapRedge.tv_usec - pi->t_tag.tv_usec)/1000;
		if( j > SysCfg.RFIDAheadTime_Ms )
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∂¡ø® ±º‰≥¨≥ˆ”––ß ±º‰(%d) ’˝∞Îæ∂∑∂Œß,¥Àø®Œ™Œﬁ–ßø®",LaneClass->LaneCfg->LaneNO, SysCfg.RFIDAheadTime_Ms);
			Delete_IdentityDupNode(pi);
		}
		pi = pitmp;
	}
	/* ∂®“Â ∂±±Í«© ˝¡ø¥Ê¥¢±‰¡ø£¨Õ¨ ±ªÒ»° ∂±∂”¡–÷–µƒ∏ˆ ˝*/
	unsigned char tag_quantity = 0;
	tag_quantity = Get_IdentityDupListLength(iList);
	printf("[Judge_RFIDUpRail_Entry]:tag_quantity is %d\n", tag_quantity);

	/*******************************************************************
	∑÷ø®π¶ƒ‹
	π¶ƒ‹:  π”√ø™πÿ”Ôæ‰£¨»∑∂®≥µµ¿∂‘œÛ ∂±µΩµƒ±Í«© ˝¡ø
	*********************************************************************/
	bool flag_2tag1Plate = LOWLEVEL; //æ÷≤øŒª±Í¡ø£¨»Áπ˚ «HIGHTLEVEL£¨Àµ√˜∂¡µΩµƒ¡Ω’≈ø®∂‘”¶Õ¨“ª∏ˆ±Í«©
	switch( tag_quantity )
	{
		case 0:
		{
			LaneClass->Snap_TagQuantity = ZEROTAG_0;
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØµΩ0’≈±Í«©£¨◊˜Œ™Œﬁø®≥µ¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
			break;
		}
		case 1:
		{
			LaneClass->Snap_TagQuantity = ONETAG_1;
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØµΩ1’≈±Í«©",LaneClass->LaneCfg->LaneNO);
			break;
		}
		case 2:
		{
			int len0 = 0, len1 = 0, len2 = 0, len3 = 0;

			/* «Û»°µ⁄2’≈ø®±Í«©≥µ≈∆∫≈∫Õ≥µ≈∆—’…´µƒ≥§∂»*/
			len0 = strlen(iList->Next->tagplateno);
			len1 = strlen(iList->Next->tagplatecl);
			/* «Û»°µ⁄1’≈ø®±Í«©≥µ≈∆∫≈∫Õ≥µ≈∆—’…´µƒ≥§∂»*/
			len2 = strlen(iList->Next->Next->tagplateno);
			len3 = strlen(iList->Next->Next->tagplatecl);

			if( (len0 == 0) && (len1 == 0) && (len2 == 0) && (len3 == 0) )		/* 2’≈ø®∂º≤ª‘⁄ø‚ */
			{
				LaneClass->Snap_TagQuantity = ZEROTAG_2_N;
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØ2’≈±Í«©,±Í«©≥µ≈∆∂ºŒ™ø’£¨µ±0’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
			}
			else if( (len0 > 0) && (len1 > 0) && (len2 > 0) && (len3 > 0) )			/* 2’≈ø®∂º‘⁄ø‚*/
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

				/* ≥µ≈∆∫Õ≥µ≈∆—’…´∂ºœ‡Õ¨£¨µ±≥…1’≈ø®¥¶¿Ì */
				if( (strcmp(p0,p2) == 0) && (strcmp(p1,p3) == 0) )
				{
					LaneClass->Snap_TagQuantity = ONETAG_2_Y_Y;
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØ2’≈±Í«©,±Í«©≥µ≈∆œ‡Õ¨£¨µ±1’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
					flag_2tag1Plate = HIGHTLEVEL;
				}
				/* ≥µ≈∆ªÚ≥µ≈∆—’…´≤ªœ‡Õ¨£¨µ±≥…0’≈ø®¥¶¿Ì */
				else if( (strcmp(p0,p2) != 0) || (strcmp(p1,p3) != 0) )
				{
					LaneClass->Snap_TagQuantity = ZEROTAG_2_Y_N;
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØ2’≈±Í«©,±Í«©≥µ≈∆∂º≤ªŒ™ø’£¨≤¢«“≤ªœ‡Õ¨£¨±Í«©≥µ≈∆≤ªÕ¨£¨µ±0’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
					flag_2tag1Plate = LOWLEVEL;
				}
			}
			else	 											/* 1’≈ø®‘⁄ø‚£¨¡Ì1’≈ø®≤ª‘⁄ø‚*/
			{
				LaneClass->Snap_TagQuantity = ONETAG_2_YoN;
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØ2’≈±Í«©,1’≈±Í«©≥µ≈∆Œ™ø’£¨¡Ì1’≈±Í«©≥µ≈∆≤ªŒ™ø’£¨µ±1’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
			}
			break;
		}
		default:				/* ¥Û”⁄2’≈ø®*/
		{
			LaneClass->Snap_TagQuantity = ZEROTAG_M2;
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØµΩ¥Û”⁄2’≈±Í«©,µ±0’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
			break;
		}
	}
	/******************************************************************************
	∑÷ø®π¶ƒ‹Ω· ¯
	*******************************************************************************/

	/*******************************************************************
	ø® ˝øÿ÷∆π¶ƒ‹
	π¶ƒ‹:  π”√ø™πÿ”Ôæ‰£¨∏˘æ›ø® ˝¿¥Ω¯––œ‡”¶µƒ∂Ø◊˜
	*********************************************************************/
	switch( LaneClass->Snap_TagQuantity )
	{
		case ZEROTAG_0:
		case ZEROTAG_2_N:
		case ZEROTAG_2_Y_N:
		case ZEROTAG_M2:
		{
			/* √ª”–≈‰÷√≥µ≈∆ ∂±£¨Œﬁø®≥µ£¨ ”Œ™¡Ÿ ±≥µ£¨…œ¥´º«¬º */
			if( LaneClass->LaneCfg->LprCfg == NOCFG )
			{
				LaneClass->Obj_Snap[0].acctype = TEMPVM;
				LaneClass->CmfType = NONE_CFM;

				/* ¡Ÿ ±≥µ‘⁄≥µµ¿¥Úø™∫Û£¨≤≈ø…“‘»•≤È—Ø’Àªßœﬁ÷∆±Ì */
				if( LaneClass->Flag_OpenLane == LOWLEVEL )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∫ÛÃ®øÿ÷∆≥µµ¿πÿ±’£¨Ω˚÷πÕ®––",LaneClass->LaneCfg->LaneNO);

					LaneClass->Note = LIMIT_PASS;	//_by_dpj_2014-09-09

					ar_2FIFO(LaneClass);
					break;
				}

				LaneClass->Note = LIMIT_PASS;	//_by_dpj_2014-09-09

				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: Œ¥≈‰÷√≥µ≈∆ ∂±£¨∂¡µΩ0’≈ø®£¨Ω˚÷πÕ®––",LaneClass->LaneCfg->LaneNO);
				ar_2FIFO(LaneClass);
			 }

			LaneClass->Obj_Snap[0].acctype = TEMPVM;		//_by_dpj_2014-09-25
			break;
		}
		case ONETAG_1:
		case ONETAG_2_Y_Y:
		{
			char tmp_pno[20];								/* ±£¥Ê≥µ≈∆ */
			char tmp_cl[10];
			struct timeval tmp_t_snap;	//_by_dpj_2014-09-12	–ﬁ∏ƒ‘≠“Ú: √ª”–±£¥Ê ±º‰÷µ£¨µº÷¬º∆ ˝≥ˆ¥Ì

			memset(tmp_pno, 0, sizeof(tmp_pno));
			memset(tmp_cl, 0, sizeof(tmp_cl));
			strcpy(tmp_pno, LaneClass->Obj_Snap[0].plateno);
			strcpy(tmp_cl, LaneClass->Obj_Snap[0].platecl);
			tmp_t_snap = LaneClass->Obj_Snap[0].t_snap;		//_by_dpj_2014-09-12
			LaneClass->Obj_Snap[0] = *(iList->Next);
			strcpy(LaneClass->Obj_Snap[0].plateno, tmp_pno);
			strcpy(LaneClass->Obj_Snap[0].platecl, tmp_cl);
			LaneClass->Obj_Snap[0].t_snap = tmp_t_snap;		//_by_dpj_2014-09-12

			if( (strlen(iList->Next->tagplatecl) == 0 )     /* ≤ª¥Ê‘⁄∑¥≤È≥µ≈∆£¨Àµ√˜ ˝æ›ø‚≤ª¥Ê‘⁄∏√ø® */
				&& (strlen(iList->Next->tagplateno) == 0) )
			{
				if( LaneClass->LaneCfg->LprCfg == NOCFG )	/* √ª”–≈‰÷√≥µ≈∆ ∂± */
				{
					LaneClass->CmfType = RFID_CFM;
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¡Ÿ ±≥µ£¨≤ª¥Ê‘⁄∑¥≤È≥µ≈∆£¨Œﬁ≈‰÷√≥µ≈∆ ∂±∆˜£¨Œﬁ∂Ø◊˜≤˙…˙",LaneClass->LaneCfg->LaneNO);
				}
				else		/* ”–≈‰÷√≥µ≈∆ ∂± */
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¡Ÿ ±≥µ£¨≤ª¥Ê‘⁄∑¥≤È≥µ≈∆£¨µ»¥˝≥µ≈∆ ∂±‘Ÿ¥Œ»∑»œ",LaneClass->LaneCfg->LaneNO);
					//break;
				}
			}

			LaneClass->CmfType = RFID_CFM;

			if( (iList->Next->acctype == TEMPVM)
				&& (LaneClass->Flag_OpenLane == LOWLEVEL) )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∫ÛÃ®øÿ÷∆≥µµ¿πÿ±’£¨Ω˚÷π¡Ÿ ±≥µÕ®––",LaneClass->LaneCfg->LaneNO);

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
			#if 1				//_by_dpj_2014-05-20	–ﬁ∏ƒ‘≠“Ú: Œﬁ∑®Ω´ ±º‰÷µ¥Ê¥¢£¨µº÷¬∏˙≥µº∆ ˝¥ÌŒÛ
			//iList->Next->t_snap = LaneClass->Time_SnapRedge;
			LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;
			#endif
			j = QueryLocDB_LimitTab(iList->Next, LaneClass);
			if( j == EXIST_ALLOWPASS )			/* ¥Ê‘⁄‘ –ÌÕ®––µƒ£¨≤≈ «ø…“‘Õ®––µƒ */
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢‘ –ÌÕ®––",LaneClass->LaneCfg->LaneNO);

				/********************************Ãß∏À∂Ø◊˜≈–∂®*************************************************/
				if( LaneClass->WorkMode == Common )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/*  Õ∑≈¬‰∏À */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* Ãß∏À∑≈–– */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/*  Õ∑≈±®æØ */
					gettimeofday(&t,NULL);
					Chg_PreciseTime2Str(t, buf, sizeof(buf));

					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏ÀÃß∏À",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == FreePass )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄◊‘”…¡˜π§◊˜ƒ£ Ω£¨¿∏∏À≥£Ãß",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Exemption )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏À“—∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				}
				LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
				/****************************************************************************************************/
				#if 0
				printf("[]:LaneClass->Obj_Snap[0].t_snap.tv_sec is %d\n", LaneClass->Obj_Snap[0].t_snap.tv_sec);
				printf("[]:LaneClass->Time_SnapRedge is %d\n", LaneClass->Time_SnapRedge.tv_sec);
				#endif
				Insert_FilterDupNode(Filter_DupList,
									LaneClass->Obj_Snap[0].tagno,		/* Ãß∏À÷Æ∫Û£¨º”»ÎµΩ¬À≤®∂”¡– */
									LaneClass->Obj_Snap[0].antennaID,
									LaneClass->Obj_Snap[0].t_snap);
				/* »Áπ˚ «2±Í«©1≥µ≈∆£¨ƒ«√¥2±Í«©∫≈∂ºº”»ÎµΩ¬À≤®∂”¡–¿ÔÕ∑ */
				if(flag_2tag1Plate == HIGHTLEVEL)
				{
					Insert_FilterDupNode(Filter_DupList,
										iList->Next->Next->tagno,
										iList->Next->Next->antennaID,
										iList->Next->Next->t_snap);
				}

				Clear_IdentityDupList(iList);						/* «Â≥˝RFID ∂±∂”¡–œ‡”¶µ•‘™ */
			}
			else if( j == EXIST_UNALLOWPASS_AL )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ¿‡–Õœﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);

				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
			}
			else if( j == EXIST_UNALLOWPASS )
			{
				LaneClass->Note = LIMIT_PASS;
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢œﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);
			}
			#if 1			//_by_dpj_2014-05-20	–ﬁ∏ƒ‘≠“Ú: Œﬁ∏√Ãıº˛µƒÃ· æ»’÷æ–≈œ¢
			else if( j == UNEXIST )
			{
				#if 1		//_by_dpj_2014-09-12	–ﬁ∏ƒ‘≠“Ú: œ¬∑¢≥µ¡æ¿‡–Õœﬁ÷∆±Ì÷–Œﬁƒ⁄»›£¨µº÷¬≤ª‘Ÿ∑¢…˙≥µ≈∆ ∂±(–°≈ÌÃ·π©)
				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
				#endif

				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢(ƒ¨»œ)œﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);
			}
			#endif

			if( LaneClass->LaneCfg->LprCfg == NOCFG )		/* √ª”–≈‰÷√≥µ≈∆ ∂±£¨Ω˚÷πÕ®––≥µ¡æ£¨…œ¥´–≈œ¢ */
			{
				ar_2FIFO(LaneClass);
			}
			break;
		}
		case ONETAG_2_YoN:
		{
			char tmp_pno[20];
			char tmp_cl[10];
			struct timeval tmp_t_snap;	//_by_dpj_2014-09-12	–ﬁ∏ƒ‘≠“Ú: √ª”–±£¥Ê ±º‰÷µ£¨µº÷¬º∆ ˝≥ˆ¥Ì

			/* µ⁄1’≈ø®≤ª¥Ê‘⁄∑¥≤È≥µ≈∆£¨Àµ√˜ ˝æ›ø‚≤ª¥Ê‘⁄∏√ø® */
			printf("[]:LaneClass->Obj_Snap[0].tagplateno is %s\n", iList->Next->tagplateno);
			printf("[]:LaneClass->Obj_Snap[0].tagplatecl is %s\n", iList->Next->tagplatecl);
			printf("[]:LaneClass->Obj_Snap[1].tagplateno is %s\n", iList->Next->Next->tagplateno);
			printf("[]:LaneClass->Obj_Snap[1].tagplatecl is %s\n", iList->Next->Next->tagplatecl);
			if( (strlen(iList->Next->tagplateno) == 0 )
				&& (strlen(iList->Next->tagplatecl) == 0) )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: µ⁄2’≈ø®≤ª¥Ê‘⁄ ˝æ›ø‚÷–",LaneClass->LaneCfg->LaneNO);

				memset(tmp_pno, 0, sizeof(tmp_pno));				/* ±£¥Êµ⁄1’≈≥µ≈∆ –≈œ¢*/
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
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: µ⁄1’≈ø®≤ª¥Ê‘⁄ ˝æ›ø‚÷–",LaneClass->LaneCfg->LaneNO);

				memset(tmp_pno, 0, sizeof(tmp_pno));				/* ±£¥Êµ⁄2’≈≥µ≈∆ –≈œ¢*/
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

			/* ”–ø®¡Ÿ ±≥µ‘⁄≥µµ¿¥Úø™∫Û£¨≤≈ø…“‘»•≤È—Ø’Àªßœﬁ÷∆±Ì */
			if( (LaneClass->Obj_Snap[0].acctype== TEMPVM) && (LaneClass->Flag_OpenLane == LOWLEVEL) )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∫ÛÃ®øÿ÷∆≥µµ¿πÿ±’£¨Ω˚÷π¡Ÿ ±≥µÕ®––",LaneClass->LaneCfg->LaneNO);
				break;
			}

			int j = -1;
			LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;

			j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);
			if( j == EXIST_ALLOWPASS )			/* ¥Ê‘⁄‘ –ÌÕ®––µƒ£¨≤≈ «ø…“‘Õ®––µƒ */
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢‘ –ÌÕ®––",LaneClass->LaneCfg->LaneNO);

				/********************************Ãß∏À∂Ø◊˜≈–∂®*************************************************/
				if( LaneClass->WorkMode == Common )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/*  Õ∑≈¬‰∏À */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* Ãß∏À∑≈–– */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/*  Õ∑≈±®æØ */
					gettimeofday(&t,NULL);
					Chg_PreciseTime2Str(t, buf, sizeof(buf));

					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏ÀÃß∏À",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == FreePass )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄◊‘”…¡˜π§◊˜ƒ£ Ω£¨¿∏∏À≥£Ãß",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Exemption )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏À“—∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				}
				LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
				/****************************************************************************************************/

				Insert_FilterDupNode(Filter_DupList,
									LaneClass->Obj_Snap[0].tagno,		/* Ãß∏À÷Æ∫Û£¨º”»ÎµΩ¬À≤®∂”¡– */
									LaneClass->Obj_Snap[0].antennaID,
									LaneClass->Obj_Snap[0].t_snap);

				Clear_IdentityDupList(iList);						/* «Â≥˝RFID ∂±∂”¡–œ‡”¶µ•‘™ */
			}
			else if( j == EXIST_UNALLOWPASS_AL )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ¿‡–Õœﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);

				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
			}
			else if( j == EXIST_UNALLOWPASS )
			{
				LaneClass->Note = LIMIT_PASS;
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢œﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);
			}
			#if 1			//_by_dpj_2014-05-20	–ﬁ∏ƒ‘≠“Ú: Œﬁ∏√Ãıº˛µƒÃ· æ»’÷æ–≈œ¢
			else if( j == UNEXIST )
			{
				#if 1		//_by_dpj_2014-09-12	–ﬁ∏ƒ‘≠“Ú: œ¬∑¢≥µ¡æ¿‡–Õœﬁ÷∆±Ì÷–Œﬁƒ⁄»›£¨µº÷¬≤ª‘Ÿ∑¢…˙≥µ≈∆ ∂±(–°≈ÌÃ·π©)
				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
				#endif
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢(ƒ¨»œ)œﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);
			}
			#endif

			if(LaneClass->LaneCfg->LprCfg == NOCFG)		/* √ª”–≈‰÷√≥µ≈∆ ∂±£¨Ω˚÷πÕ®––≥µ¡æ£¨…œ¥´–≈œ¢ */
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
	ø® ˝øÿ÷∆π¶ƒ‹Ω· ¯
	*******************************************************************************/

	return;
}

/************************************************************************************
* Judge_UpRail
* DESCRIPTION: ◊•≈ƒœﬂ»¶∂‘œÛ∫Ø ˝
* @LaneClass : ≥µµ¿∂‘œÛ
* @iList	 :  ∂±∂”¡–
* @Return 	 : no
* ----
*************************************************************************************/
void Judge_RFIDUpRail_Export(laneclass_type *LaneClass,identity_node *iList)
{
	if( LaneClass->LaneCfg->Exemption == FULLEXEMPTION )		//_by_dpj_2015-01-08
	{
		if( LaneClass->WorkMode == Exemption )
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¥¶”⁄»´√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏ÀÃß∏À",LaneClass->LaneCfg->LaneNO);

			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown, RELEASE_DOWNRAIL);		/*  Õ∑≈¬‰∏À */
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, UPRAIL);					/* Ãß∏À∑≈–– */
		}
	}

	struct  timeval t;
	char buf[32];
	identity_node *pi;					/* «Â≥˝ ∂±∂”¡– ±º‰≥¨≥ˆ”––ß∑∂Œßµƒµ•‘™ */
	identity_node *pitmp;
	pi = iList->Next;

	/* Ãﬁ≥˝≥¨≥ˆ”––ß’˝∞Îæ∂ ±º‰µƒø® */
	while( pi != NULL )
	{
		pitmp = pi->Next;
		int j;
		j = (LaneClass->Time_SnapRedge.tv_sec - pi->t_tag.tv_sec)*1000
			+ (LaneClass->Time_SnapRedge.tv_usec - pi->t_tag.tv_usec)/1000;
		if( j > SysCfg.RFIDAheadTime_Ms )
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∂¡ø® ±º‰≥¨≥ˆ”––ß ±º‰(%d) ’˝∞Îæ∂∑∂Œß,¥Àø®Œ™Œﬁ–ßø®",LaneClass->LaneCfg->LaneNO, SysCfg.RFIDAheadTime_Ms);
			Delete_IdentityDupNode(pi);
		}
		pi = pitmp;
	}

	/* ∂®“Â ∂±±Í«© ˝¡ø¥Ê¥¢±‰¡ø£¨Õ¨ ±ªÒ»° ∂±∂”¡–÷–µƒ∏ˆ ˝*/
	unsigned char tag_quantity = 0;
	tag_quantity = Get_IdentityDupListLength(iList);

	/*******************************************************************
	∑÷ø®π¶ƒ‹
	π¶ƒ‹:  π”√ø™πÿ”Ôæ‰£¨»∑∂®≥µµ¿∂‘œÛ ∂±µΩµƒ±Í«© ˝¡ø
	*********************************************************************/
	bool flag_2tag1Plate = LOWLEVEL; //æ÷≤øŒª±Í¡ø£¨»Áπ˚ «HIGHTLEVEL£¨Àµ√˜∂¡µΩµƒ¡Ω’≈ø®∂‘”¶Õ¨“ª∏ˆ±Í«©
	switch( tag_quantity )
	{
		case 0:
		{
			LaneClass->Snap_TagQuantity = ZEROTAG_0;
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØµΩ0’≈±Í«©£¨◊˜Œ™Œﬁø®≥µ¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
			break;
		}
		case 1:
		{
			LaneClass->Snap_TagQuantity = ONETAG_1;
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØµΩ1’≈±Í«©",LaneClass->LaneCfg->LaneNO);
			break;
		}
		case 2:
		{
			int len0 = 0, len1 = 0, len2 = 0, len3 = 0;

			/* «Û»°µ⁄2’≈ø®±Í«©≥µ≈∆∫≈∫Õ≥µ≈∆—’…´µƒ≥§∂»*/
			len0 = strlen(iList->Next->tagplateno);
			len1 = strlen(iList->Next->tagplatecl);
			/* «Û»°µ⁄1’≈ø®±Í«©≥µ≈∆∫≈∫Õ≥µ≈∆—’…´µƒ≥§∂»*/
			len2 = strlen(iList->Next->Next->tagplateno);
			len3 = strlen(iList->Next->Next->tagplatecl);

			if( (len0 == 0) && (len1 == 0) && (len2 == 0) && (len3 == 0) )		/* 2’≈ø®∂º≤ª‘⁄ø‚ */
			{
				LaneClass->Snap_TagQuantity = ZEROTAG_2_N;
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØ2’≈±Í«©,±Í«©≥µ≈∆∂ºŒ™ø’£¨µ±0’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
			}
			else if( (len0 > 0) && (len1 > 0) && (len2 > 0) && (len3 > 0) )			/* 2’≈ø®∂º‘⁄ø‚*/
			{
				char *p0 = NULL, *p1 = NULL, *p2 = NULL, *p3 = NULL;
				p0 = iList->Next->tagplateno;
				p1 = iList->Next->tagplatecl;
				p2 = iList->Next->Next->tagplateno;
				p3 = iList->Next->Next->tagplatecl;

				/* ≥µ≈∆∫Õ≥µ≈∆—’…´∂ºœ‡Õ¨£¨µ±≥…1’≈ø®¥¶¿Ì */
				if( (strcmp(p0,p2) == 0) && (strcmp(p1,p3) == 0) )
				{
					LaneClass->Snap_TagQuantity = ONETAG_2_Y_Y;
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØ2’≈±Í«©,±Í«©≥µ≈∆œ‡Õ¨£¨µ±1’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
					flag_2tag1Plate = HIGHTLEVEL;
				}
				/* ≥µ≈∆ªÚ≥µ≈∆—’…´≤ªœ‡Õ¨£¨µ±≥…0’≈ø®¥¶¿Ì */
				else if( (strcmp(p0,p2) != 0) || (strcmp(p1,p3) != 0) )		//_by_dpj_2014-04-08
				{
					LaneClass->Snap_TagQuantity = ZEROTAG_2_Y_N;
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØ2’≈±Í«©,±Í«©≥µ≈∆∂º≤ªŒ™ø’£¨≤¢«“≤ªœ‡Õ¨£¨±Í«©≥µ≈∆≤ªÕ¨£¨µ±0’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
					flag_2tag1Plate = LOWLEVEL;
				}
			}
			else	 											/* 1’≈ø®‘⁄ø‚£¨¡Ì1’≈ø®≤ª‘⁄ø‚*/
			{
				LaneClass->Snap_TagQuantity = ONETAG_2_YoN;
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØ2’≈±Í«©,1’≈±Í«©≥µ≈∆Œ™ø’£¨¡Ì1’≈±Í«©≥µ≈∆≤ªŒ™ø’£¨µ±1’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
			}
			break;
		}
		default:				/* ¥Û”⁄2’≈ø®*/
		{
			LaneClass->Snap_TagQuantity = ZEROTAG_M2;
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≤…ºØµΩ¥Û”⁄2’≈±Í«©,µ±0’≈±Í«©¥¶¿Ì",LaneClass->LaneCfg->LaneNO);
			break;
		}
	}
	/******************************************************************************
	∑÷ø®π¶ƒ‹Ω· ¯
	*******************************************************************************/

	/*******************************************************************
	ø® ˝øÿ÷∆π¶ƒ‹
	π¶ƒ‹:  π”√ø™πÿ”Ôæ‰£¨∏˘æ›ø® ˝¿¥Ω¯––œ‡”¶µƒ∂Ø◊˜
	*********************************************************************/
	switch( LaneClass->Snap_TagQuantity )
	{
		case ZEROTAG_0:
		case ZEROTAG_2_N:
		case ZEROTAG_2_Y_N:
		case ZEROTAG_M2:
		{
			/* √ª”–≈‰÷√≥µ≈∆ ∂±£¨Œﬁø®≥µ£¨ ”Œ™¡Ÿ ±≥µ£¨…œ¥´º«¬º */
			if( LaneClass->LaneCfg->LprCfg == NOCFG )
			{
				LaneClass->Obj_Snap[0].acctype = TEMPVM;
				LaneClass->CmfType = NONE_CFM;

				LaneClass->Note = LIMIT_PASS;	//_by_dpj_2014-09-09

				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: Œ¥≈‰÷√≥µ≈∆ ∂±£¨∂¡µΩ0’≈ø®£¨Ω˚÷πÕ®––",LaneClass->LaneCfg->LaneNO);
				ar_2FIFO(LaneClass);
			 }
			break;
		}
		case ONETAG_1:
		case ONETAG_2_Y_Y:
		{
			char tmp_pno[20];								/* ±£¥Ê≥µ≈∆ */
			char tmp_cl[10];
			struct timeval tmp_t_snap;	//_by_dpj_2014-09-15	–ﬁ∏ƒ‘≠“Ú: √ª”–±£¥Ê ±º‰÷µ£¨µº÷¬º∆ ˝≥ˆ¥Ì

			memset(tmp_pno, 0, sizeof(tmp_pno));
			memset(tmp_cl, 0, sizeof(tmp_cl));
			strcpy(tmp_pno, LaneClass->Obj_Snap[0].plateno);
			strcpy(tmp_cl, LaneClass->Obj_Snap[0].platecl);
			tmp_t_snap = LaneClass->Obj_Snap[0].t_snap;		//_by_dpj_2014-09-15
			LaneClass->Obj_Snap[0] = *(iList->Next);
			strcpy(LaneClass->Obj_Snap[0].plateno, tmp_pno);
			strcpy(LaneClass->Obj_Snap[0].platecl, tmp_cl);
			LaneClass->Obj_Snap[0].t_snap = tmp_t_snap;		//_by_dpj_2014-09-15

			if( (strlen(iList->Next->tagplatecl) == 0 )     /* ≤ª¥Ê‘⁄∑¥≤È≥µ≈∆£¨Àµ√˜ ˝æ›ø‚≤ª¥Ê‘⁄∏√ø® */
				&& (strlen(iList->Next->tagplateno) == 0) )
			{
				if( LaneClass->LaneCfg->LprCfg == NOCFG )	/* √ª”–≈‰÷√≥µ≈∆ ∂± */
				{
					LaneClass->CmfType = RFID_CFM;
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¡Ÿ ±≥µ£¨≤ª¥Ê‘⁄∑¥≤È≥µ≈∆£¨Œﬁ≈‰÷√≥µ≈∆ ∂±∆˜£¨Œﬁ∂Ø◊˜≤˙…˙",LaneClass->LaneCfg->LaneNO);
				}
				else		/* ”–≈‰÷√≥µ≈∆ ∂± */
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¡Ÿ ±≥µ£¨≤ª¥Ê‘⁄∑¥≤È≥µ≈∆£¨µ»¥˝≥µ≈∆ ∂±‘Ÿ¥Œ»∑»œ",LaneClass->LaneCfg->LaneNO);
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
			if( j == EXIST_ALLOWPASS )			/* ¥Ê‘⁄‘ –ÌÕ®––µƒ£¨≤≈ «ø…“‘Õ®––µƒ */
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢‘ –ÌÕ®––",LaneClass->LaneCfg->LaneNO);

				/********************************Ãß∏À∂Ø◊˜≈–∂®*************************************************/
				if( LaneClass->WorkMode == Common )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/*  Õ∑≈¬‰∏À */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* Ãß∏À∑≈–– */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/*  Õ∑≈±®æØ */
					gettimeofday(&t,NULL);
					Chg_PreciseTime2Str(t, buf, sizeof(buf));

					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏ÀÃß∏À",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == FreePass )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄◊‘”…¡˜π§◊˜ƒ£ Ω£¨¿∏∏À≥£Ãß",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Deployment )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄≤º∑¿π§◊˜ƒ£ Ω£¨¿∏∏À≤ª∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Exemption )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏À“—∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				}
				LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
				/****************************************************************************************************/

				Insert_FilterDupNode(Filter_DupList,
									LaneClass->Obj_Snap[0].tagno,		/* Ãß∏À÷Æ∫Û£¨º”»ÎµΩ¬À≤®∂”¡– */
									LaneClass->Obj_Snap[0].antennaID,
									LaneClass->Obj_Snap[0].t_snap);
				/* »Áπ˚ «2±Í«©1≥µ≈∆£¨ƒ«√¥2±Í«©∫≈∂ºº”»ÎµΩ¬À≤®∂”¡–¿ÔÕ∑ */
				if(flag_2tag1Plate == HIGHTLEVEL)
				{
					Insert_FilterDupNode(Filter_DupList,
										iList->Next->Next->tagno,
										iList->Next->Next->antennaID,
										iList->Next->Next->t_snap);
				}

				Clear_IdentityDupList(iList);						/* «Â≥˝RFID ∂±∂”¡–œ‡”¶µ•‘™ */
			}
			else if( j == EXIST_UNALLOWPASS_AL )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ¿‡–Õœﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);

				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
			}
			else if( j == EXIST_UNALLOWPASS )
			{
				LaneClass->Note = LIMIT_PASS;
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢œﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);

			}
			#if 1			//_by_dpj_2014-05-20	–ﬁ∏ƒ‘≠“Ú: Œﬁ∏√Ãıº˛µƒÃ· æ»’÷æ–≈œ¢
			else if( j == UNEXIST )
			{
				#if 1		//_by_dpj_2014-09-12	–ﬁ∏ƒ‘≠“Ú: œ¬∑¢≥µ¡æ¿‡–Õœﬁ÷∆±Ì÷–Œﬁƒ⁄»›£¨µº÷¬≤ª‘Ÿ∑¢…˙≥µ≈∆ ∂±(–°≈ÌÃ·π©)
				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
				#endif
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢(ƒ¨»œ)œﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);
			}
			#endif

			if( LaneClass->LaneCfg->LprCfg == NOCFG )		/* √ª”–≈‰÷√≥µ≈∆ ∂±£¨Ω˚÷πÕ®––≥µ¡æ£¨…œ¥´–≈œ¢ */
			{
				ar_2FIFO(LaneClass);
			}
			break;
		}
		case ONETAG_2_YoN:
		{
			char tmp_pno[20];
			char tmp_cl[10];
			struct timeval tmp_t_snap;	//_by_dpj_2014-09-15	–ﬁ∏ƒ‘≠“Ú: √ª”–±£¥Ê ±º‰÷µ£¨µº÷¬º∆ ˝≥ˆ¥Ì

			/* µ⁄1’≈ø®≤ª¥Ê‘⁄∑¥≤È≥µ≈∆£¨Àµ√˜ ˝æ›ø‚≤ª¥Ê‘⁄∏√ø® */
			printf("[]:LaneClass->Obj_Snap[0].tagplateno is %s\n", iList->Next->tagplateno);
			printf("[]:LaneClass->Obj_Snap[0].tagplatecl is %s\n", iList->Next->tagplatecl);
			printf("[]:LaneClass->Obj_Snap[1].tagplateno is %s\n", iList->Next->Next->tagplateno);
			printf("[]:LaneClass->Obj_Snap[1].tagplatecl is %s\n", iList->Next->Next->tagplatecl);
			if( (strlen(iList->Next->tagplateno) == 0 )
				&& (strlen(iList->Next->tagplatecl) == 0) )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: µ⁄2’≈ø®≤ª¥Ê‘⁄ ˝æ›ø‚÷–",LaneClass->LaneCfg->LaneNO);

				memset(tmp_pno, 0, sizeof(tmp_pno));				/* ±£¥Êµ⁄1’≈≥µ≈∆ –≈œ¢*/
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
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: µ⁄1’≈ø®≤ª¥Ê‘⁄ ˝æ›ø‚÷–",LaneClass->LaneCfg->LaneNO);

				memset(tmp_pno, 0, sizeof(tmp_pno));				/* ±£¥Êµ⁄2’≈≥µ≈∆ –≈œ¢*/
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
			/* ”–ø®¡Ÿ ±≥µ‘⁄≥µµ¿¥Úø™∫Û£¨≤≈ø…“‘»•≤È—Ø’Àªßœﬁ÷∆±Ì */
			if( (LaneClass->Obj_Snap[0].acctype== TEMPVM) && (LaneClass->Flag_OpenLane == LOWLEVEL) )
			{
				printf("[RFID:]≥µµ¿%d: ∫ÛÃ®øÿ÷∆≥µµ¿πÿ±’£¨Ω˚÷π¡Ÿ ±≥µÕ®––--------\r\n",
					LaneClass->LaneCfg->LaneNO);
				In_LogQueue(logqueue, "[RFID:]≥µµ¿%d: ∫ÛÃ®øÿ÷∆≥µµ¿πÿ±’£¨Ω˚÷π¡Ÿ ±≥µÕ®––--------\r\n",
					LaneClass->LaneCfg->LaneNO, NULL, NULL, 9999, LOG_INFO);
				break;
			}
			#endif

			int j = -1;
			LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;

			j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);
			if( j == EXIST_ALLOWPASS )			/* ¥Ê‘⁄‘ –ÌÕ®––µƒ£¨≤≈ «ø…“‘Õ®––µƒ */
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢‘ –ÌÕ®––",LaneClass->LaneCfg->LaneNO);

				/********************************Ãß∏À∂Ø◊˜≈–∂®*************************************************/
				if( LaneClass->WorkMode == Common )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/*  Õ∑≈¬‰∏À */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* Ãß∏À∑≈–– */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/*  Õ∑≈±®æØ */
					gettimeofday(&t,NULL);
					Chg_PreciseTime2Str(t, buf, sizeof(buf));

					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏ÀÃß∏À",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == FreePass )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄◊‘”…¡˜π§◊˜ƒ£ Ω£¨¿∏∏À≥£Ãß",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Deployment )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄≤º∑¿π§◊˜ƒ£ Ω£¨¿∏∏À≤ª∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->WorkMode == Exemption )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: RFID: ¥¶”⁄√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏À“—∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				}
				LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
				/****************************************************************************************************/

				Insert_FilterDupNode(Filter_DupList,
									LaneClass->Obj_Snap[0].tagno,		/* Ãß∏À÷Æ∫Û£¨º”»ÎµΩ¬À≤®∂”¡– */
									LaneClass->Obj_Snap[0].antennaID,
									LaneClass->Obj_Snap[0].t_snap);

				Clear_IdentityDupList(iList);						/* «Â≥˝RFID ∂±∂”¡–œ‡”¶µ•‘™ */
			}
			else if( j == EXIST_UNALLOWPASS_AL )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ¿‡–Õœﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);

				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
			}
			else if( j == EXIST_UNALLOWPASS )
			{
				LaneClass->Note = LIMIT_PASS;
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢œﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);
			}
			#if 1			//_by_dpj_2014-05-20	–ﬁ∏ƒ‘≠“Ú: Œﬁ∏√Ãıº˛µƒÃ· æ»’÷æ–≈œ¢
			else if( j == UNEXIST )
			{
				#if 1		//_by_dpj_2014-09-12	–ﬁ∏ƒ‘≠“Ú: œ¬∑¢≥µ¡æ¿‡–Õœﬁ÷∆±Ì÷–Œﬁƒ⁄»›£¨µº÷¬≤ª‘Ÿ∑¢…˙≥µ≈∆ ∂±(–°≈ÌÃ·π©)
				if( LaneClass->LaneCfg->LprCfg == NOCFG)
				{
					LaneClass->Note = LIMIT_PASS;
				}
				#endif
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢(ƒ¨»œ)œﬁ÷∆Õ®––",LaneClass->LaneCfg->LaneNO);
			}
			#endif

			if(LaneClass->LaneCfg->LprCfg == NOCFG)		/* √ª”–≈‰÷√≥µ≈∆ ∂±£¨Ω˚÷πÕ®––≥µ¡æ£¨…œ¥´–≈œ¢ */
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
	ø® ˝øÿ÷∆π¶ƒ‹Ω· ¯
	*******************************************************************************/

	return;
}
/************************************************************************************
* Judge_LPRUpRail
* DESCRIPTION: ◊•≈ƒœﬂ»¶∂‘œÛ∫Ø ˝
* @LaneClass : ≥µµ¿∂‘œÛ
* @iList	 :  ∂±∂”¡–
* @Return 	 : no
* ----
*************************************************************************************/
void Judge_LPRUpRail_Entry(laneclass_type * LaneClass,identity_node *iList)
{
	/*  ±º‰¿€º”∆˜–°”⁄µ»”⁄◊•≈ƒ∫Û—” ±º‰£¨”–≥µ≈∆ ∂±Ω·π˚£¨«“∏√Ω·π˚Œ¥±ª¥¶¿Ì*/
	if( (LaneClass->TimeAcc_Snap <= SysCfg.LPRDelayTime_Ms)		/* Ãıº˛¥•∑¢£¨÷ª÷¥––“ª¥Œ */
		&& (strlen(LaneClass->Obj_Snap[0].plateno) > 0)			/* ”–≥µ≈∆ ∂±Ω·π˚ */
		&& (LaneClass->LprRes == NOLPRRES ) )
	{
		struct  timeval t;
		char buf[32];
		printf("≥µ≈∆ ∂±Ω·π˚ «:%s\r\n", LaneClass->Obj_Snap[0].plateno);

		/********************************************************************************
		∂˛¥Œ ∂±π¶ƒ‹_by_dpj_2014-09-29
		Ãıº˛: ≈–∂œ «∑Ò≥µ≈∆ ∂±Ω·π˚ «∑ÒŒ™’˚≈∆æ‹ ∂
		********************************************************************************/
		if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ”…”⁄“ª¥Œ ∂±÷–≥µ≈∆Ω·π˚Œ™’˚≈∆æ‹ ∂, À˘“‘Ω¯»Î∂˛¥Œ ∂±π˝≥Ã", LaneClass->LaneCfg->LaneNO);
			#if 1	//_by_dpj_2014-10-09	–ﬁ∏ƒ‘≠“Ú: µ±÷«ƒ‹øÿ÷∆∆˜÷–≈‰÷√Œ™À´»Îø⁄£®À´≥ˆø⁄£© ±£¨ª·µº÷¬∂˛¥Œ ∂±¡˜≥Ã¥ÌŒÛ
			CaptureImage(LaneClass->LPR->IP);
			#endif
			sleep(1);
		}
		/******************************************************************************
		∂˛¥Œ ∂±π¶ƒ‹Ω· ¯_by_dpj_2014-09-29
		*******************************************************************************/

		#if 1	//_by_dpj_2015-01-08
		if( LaneClass->LaneCfg->Exemption == CAREXEMPTION )
		{
			if( LaneClass->WorkMode == Exemption )
			{
				if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¥¶”⁄∆˚≥µ√‚ºÏπ§◊˜ƒ£ Ω£¨%s£¨¿∏∏À≤ªÃß∏À", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
				}
				else if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") != 0 )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¥¶”⁄∆˚≥µ√‚ºÏπ§◊˜ƒ£ Ω£¨%s£¨¿∏∏ÀÃß∏À", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown, RELEASE_DOWNRAIL);		/*  Õ∑≈¬‰∏À */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, UPRAIL);					/* Ãß∏À∑≈–– */
				}
			}
		}
		#endif

		LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;
		LaneClass->LprRes = HAVE_PROLPR;

		/* µ±≥µ¡æ–≈œ¢‘⁄RFID ∂±π˝≥Ã÷–±ªœﬁ÷∆Õ®––±Ìœﬁ÷∆Õ®–– ±£¨≤ª‘ŸΩ¯––∂˛¥Œ ∂±*/
		#if 1
		if( LaneClass->Note == LIMIT_PASS )
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ”–≈‰÷√≥µ≈∆ ∂±∆˜£¨µ´∏√≥µ‘⁄RFIDπ˝≥Ã÷–±ªœﬁ÷∆Õ®––£¨≤ªΩ¯––≥µ≈∆Ω¯“ª≤Ω ∂±∂Ø◊˜", LaneClass->LaneCfg->LaneNO);
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
				LaneClass->CmfType = LPR_CFM;				// ∂±ƒ£ Ω±Í÷æŒ™LPR ∂±

				if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )			/* ≥µ≈∆Œ™’˚≈∆æ‹ ∂£¨Ω˚÷πÕ®––*/
				{
					LaneClass->Obj_Snap[0].acctype = TEMPVM;
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ’˚≈∆æ‹ ∂£¨Ω˚÷πÕ®––", LaneClass->LaneCfg->LaneNO);
					break;
				}
				else			/* ”– ∂±≥ˆ≥µ≈∆–≈œ¢*/
				{
					LPR_Process(LaneClass);

					if((LaneClass->Obj_Snap[0].acctype == TEMPVM				/* ”–ø®¡Ÿ ±≥µ‘⁄≥µµ¿¥Úø™∫Û£¨≤≈ø…“‘»•≤È—Ø’Àªßœﬁ÷∆±Ì */)
						&&(LaneClass->Flag_OpenLane == LOWLEVEL))
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d,∫ÛÃ®øÿ÷∆≥µµ¿πÿ±’£¨Ω˚÷π¡Ÿ ±≥µÕ®––", LaneClass->LaneCfg->LaneNO);
						break;
					}

					int j=-1;
					j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);
					if( j == EXIST_ALLOWPASS )
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢‘ –ÌÕ®––", LaneClass->LaneCfg->LaneNO);

						/********************************Ãß∏À∂Ø◊˜≈–∂®*************************************************/
						if( LaneClass->WorkMode == Common )
						{
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/*  Õ∑≈¬‰∏À */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* Ãß∏À∑≈–– */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/*  Õ∑≈±®æØ */
							gettimeofday(&t,NULL);
							Chg_PreciseTime2Str(t, buf, sizeof(buf));

							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏ÀÃß∏À", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == FreePass )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄◊‘”…¡˜π§◊˜ƒ£ Ω£¨¿∏∏À≥£Ãß", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Exemption )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏À“—∂Ø◊˜", LaneClass->LaneCfg->LaneNO);
						}
						LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
						/********************************************************************************************************/

						/* »Áπ˚”–∑¥≤ÈµΩ±Í«©£¨º”»ÎµΩ¬À≤®∂”¡– */
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
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µΩ˚÷πÕ®––", LaneClass->LaneCfg->LaneNO);
					}
				}

				break;
			}
			case ONETAG_1:
			case ONETAG_2_Y_Y:
			case ONETAG_2_YoN:
			{
				if( LaneClass->Obj_Snap[0].FlagUpRail == NO_URAILl )		/* ∂¡µΩø®£¨µ´ «√ªÃß∏À£¨ «œﬁ÷∆’Àªß¿‡–ÕÕ®––µƒ*/
				{
					if( LaneClass->CmfType == NONE_CFM )		/* Œ¥±ª’˝»∑ ∂±*/
					{
						LaneClass->CmfType = LPR_CFM;
					}

					if( LaneClass->Obj_Snap[0].acctype != TEMPVM ) 	 /* ∑«¡Ÿ ±≥µ£¨≤ª”√‘Ÿ¥Œ”√≥µ≈∆»∑»œ */
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: 1’≈ø®£¨∏√≥µŒ™∑«¡Ÿ ±≥µ£¨Œﬁ–Ë≥µ≈∆ ∂±»∑»œ", LaneClass->LaneCfg->LaneNO);
						break;
					}

					if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )  /* RFIDŒﬁÃß∏À£¨≤¢«“’˚≈≈æ‹ ∂£¨≤ª∂Ø◊˜£¨÷±Ω”…œ¥´ */
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: 1’≈ø®£¨≥µ≈∆ ∂±Œ™’˚≈∆æ‹ ∂£¨≤ª∂Ø◊˜", LaneClass->LaneCfg->LaneNO);
						break;
					}

					if( (LaneClass->Obj_Snap[0].acctype == TEMPVM)
						&& (LaneClass->Flag_OpenLane == LOWLEVEL ) )
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d,∫ÛÃ®øÿ÷∆≥µµ¿πÿ±’£¨Ω˚÷π¡Ÿ ±≥µÕ®––", LaneClass->LaneCfg->LaneNO);
						break;
					}

					#if 1			/* ∑Ω±„¥”»’÷æ≈–∂œ‘≠“Ú*/
					if( (strcmp(LaneClass->Obj_Snap[0].plateno, iList->Next->tagplateno) == 0 )
					   && (strcmp(LaneClass->Obj_Snap[0].platecl,iList->Next->tagplatecl) == 0) )
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: 1’≈ø®£¨RFID∑¥≤È≥µ≈∆”ÎLPR ∂±≥µ≈∆“ª÷¬", LaneClass->LaneCfg->LaneNO);
					}
					else
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: 1’≈ø®£¨RFID∑¥≤È≥µ≈∆”ÎLPR ∂±≥µ≈∆≤ª“ª÷¬", LaneClass->LaneCfg->LaneNO);
					}
					#endif

					LaneClass->CmfType = LPR_CFM;
					LPR_Process(LaneClass);

					int j = -1;
					j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);

					/* ¥Ê‘⁄‘ –ÌÕ®––ªÚ’ﬂ≤ª¥Ê‘⁄,∂º»œŒ™ «ø…“‘Õ®––µƒ */
					if( j == EXIST_ALLOWPASS )
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢‘ –ÌÕ®––", LaneClass->LaneCfg->LaneNO);

						/********************************Ãß∏À∂Ø◊˜≈–∂®*************************************************/
						if( LaneClass->WorkMode == Common )
						{
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/*  Õ∑≈¬‰∏À */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* Ãß∏À∑≈–– */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/*  Õ∑≈±®æØ */
							gettimeofday(&t,NULL);
							Chg_PreciseTime2Str(t, buf, sizeof(buf));

							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏ÀÃß∏À", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == FreePass )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄◊‘”…¡˜π§◊˜ƒ£ Ω£¨¿∏∏À≥£Ãß", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Exemption )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏À“—∂Ø◊˜", LaneClass->LaneCfg->LaneNO);
						}
						LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
						/********************************************************************************************************/

						if( strlen(LaneClass->Obj_Snap[0].tagno) > 0 )		/* »Áπ˚”–∑¥≤ÈµΩ±Í«©£¨º”»ÎµΩ¬À≤®∂”¡– */
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
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µΩ˚÷πÕ®––", LaneClass->LaneCfg->LaneNO);
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
		LaneClass->Obj_Snap[0].t_snap=LaneClass->Time_SnapRedge;	/*  ±º‰∏≥÷µ */
		strcpy(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂");			/* ÷ª∏≥÷µ“ª¥Œ */

		#if 1	//_by_dpj_2015-01-08
		if( LaneClass->LaneCfg->Exemption == CAREXEMPTION )
		{
			if( LaneClass->WorkMode == Exemption )
			{
				if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¥¶”⁄∆˚≥µ√‚ºÏπ§◊˜ƒ£ Ω£¨%s£¨¿∏∏À≤ªÃß∏À", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
				}
			}
		}
		#endif

		LaneClass->LprRes = HAVE_PROLPR;							/* ÷√Œª”–≥µ≈∆ ∂±¥¶¿Ì±Í÷æ */
		printf("[LPR:]≥¨π˝≥µ≈∆ ∂±∫Û—”œﬁ∂® ±º‰£¨√ª”– ∂±Ω·π˚£¨◊‘∂Ø∏≥”Ë’˚≈∆æ‹ ∂\r\n");
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
* DESCRIPTION: ◊•≈ƒœﬂ»¶∂‘œÛ∫Ø ˝
* @LaneClass : ≥µµ¿∂‘œÛ
* @iList	 :  ∂±∂”¡–
* @Return 	 : no
* ----
*************************************************************************************/

void Judge_LPRUpRail_Export(laneclass_type * LaneClass,identity_node *iList)
{
	/*  ±º‰¿€º”∆˜–°”⁄µ»”⁄◊•≈ƒ∫Û—” ±º‰£¨”–≥µ≈∆ ∂±Ω·π˚£¨«“∏√Ω·π˚Œ¥±ª¥¶¿Ì*/
	if( (LaneClass->TimeAcc_Snap <= SysCfg.LPRDelayTime_Ms)		/* Ãıº˛¥•∑¢£¨÷ª÷¥––“ª¥Œ */
		&& (strlen(LaneClass->Obj_Snap[0].plateno) > 0)			/* ”–≥µ≈∆ ∂±Ω·π˚ */
		&& (LaneClass->LprRes == NOLPRRES ) )
	{
		struct  timeval t;
		char buf[32];
		printf("≥µ≈∆ ∂±Ω·π˚ «:%s\r\n", LaneClass->Obj_Snap[0].plateno);

		/********************************************************************************
		∂˛¥Œ ∂±π¶ƒ‹_by_dpj_2014-09-29
		Ãıº˛: ≈–∂œ «∑Ò≥µ≈∆ ∂±Ω·π˚ «∑ÒŒ™’˚≈∆æ‹ ∂
		********************************************************************************/
		if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ”…”⁄“ª¥Œ ∂±÷–≥µ≈∆Ω·π˚Œ™’˚≈∆æ‹ ∂, À˘“‘Ω¯»Î∂˛¥Œ ∂±π˝≥Ã", LaneClass->LaneCfg->LaneNO);
			#if 1	//_by_dpj_2014-10-09	–ﬁ∏ƒ‘≠“Ú: µ±÷«ƒ‹øÿ÷∆∆˜÷–≈‰÷√Œ™À´»Îø⁄£®À´≥ˆø⁄£© ±£¨ª·µº÷¬∂˛¥Œ ∂±¡˜≥Ã¥ÌŒÛ
			CaptureImage(LaneClass->LPR->IP);
			#endif
			sleep(1);
		}
		/******************************************************************************
		∂˛¥Œ ∂±π¶ƒ‹Ω· ¯_by_dpj_2014-09-29
		*******************************************************************************/

		#if 1	//_by_dpj_2015-01-08
		if( LaneClass->LaneCfg->Exemption == CAREXEMPTION )
		{
			if( LaneClass->WorkMode == Exemption )
			{
				if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¥¶”⁄∆˚≥µ√‚ºÏπ§◊˜ƒ£ Ω£¨%s£¨¿∏∏À≤ªÃß∏À", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
				}
				else if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") != 0 )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¥¶”⁄∆˚≥µ√‚ºÏπ§◊˜ƒ£ Ω£¨%s£¨¿∏∏ÀÃß∏À", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown, RELEASE_DOWNRAIL);		/*  Õ∑≈¬‰∏À */
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, UPRAIL);					/* Ãß∏À∑≈–– */
				}
			}
		}
		#endif

		LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;
		LaneClass->LprRes = HAVE_PROLPR;

		/* µ±≥µ¡æ–≈œ¢‘⁄RFID ∂±π˝≥Ã÷–±ªœﬁ÷∆Õ®––±Ìœﬁ÷∆Õ®–– ±£¨≤ª‘ŸΩ¯––∂˛¥Œ ∂±*/
		#if 1
		if( LaneClass->Note == LIMIT_PASS )
		{
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ”–≈‰÷√≥µ≈∆ ∂±∆˜£¨µ´∏√≥µ‘⁄RFIDπ˝≥Ã÷–±ªœﬁ÷∆Õ®––£¨≤ªΩ¯––≥µ≈∆Ω¯“ª≤Ω ∂±∂Ø◊˜", LaneClass->LaneCfg->LaneNO);
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
				LaneClass->CmfType = LPR_CFM;				// ∂±ƒ£ Ω±Í÷æŒ™LPR ∂±

				if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )			/* ≥µ≈∆Œ™’˚≈∆æ‹ ∂£¨Ω˚÷πÕ®––*/
				{
					LaneClass->Obj_Snap[0].acctype = TEMPVM;
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ’˚≈∆æ‹ ∂£¨Ω˚÷πÕ®––", LaneClass->LaneCfg->LaneNO);
					break;
				}
				else			/* ”– ∂±≥ˆ≥µ≈∆–≈œ¢*/
				{
					LPR_Process(LaneClass);

					int j=-1;
					j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);
					if( j == EXIST_ALLOWPASS )
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢‘ –ÌÕ®––", LaneClass->LaneCfg->LaneNO);

						/********************************Ãß∏À∂Ø◊˜≈–∂®*************************************************/
						if( LaneClass->WorkMode == Common )
						{
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/*  Õ∑≈¬‰∏À */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* Ãß∏À∑≈–– */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/*  Õ∑≈±®æØ */
							gettimeofday(&t,NULL);
							Chg_PreciseTime2Str(t, buf, sizeof(buf));

							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏ÀÃß∏À", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == FreePass )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄◊‘”…¡˜π§◊˜ƒ£ Ω£¨¿∏∏À≥£Ãß", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Deployment )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄≤º∑¿π§◊˜ƒ£ Ω£¨¿∏∏À≤ª∂Ø◊˜Ãß", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Exemption )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏À“—∂Ø◊˜", LaneClass->LaneCfg->LaneNO);
						}
						LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
						/********************************************************************************************************/

						/* »Áπ˚”–∑¥≤ÈµΩ±Í«©£¨º”»ÎµΩ¬À≤®∂”¡– */
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
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µΩ˚÷πÕ®––", LaneClass->LaneCfg->LaneNO);
					}
				}

				break;
			}
			case ONETAG_1:
			case ONETAG_2_Y_Y:
			case ONETAG_2_YoN:
			{
				if( LaneClass->Obj_Snap[0].FlagUpRail == NO_URAILl )		/* ∂¡µΩø®£¨µ´ «√ªÃß∏À£¨ «œﬁ÷∆’Àªß¿‡–ÕÕ®––µƒ*/
				{
					if( LaneClass->CmfType == NONE_CFM )		/* Œ¥±ª’˝»∑ ∂±*/
					{
						LaneClass->CmfType = LPR_CFM;
					}

					if( LaneClass->Obj_Snap[0].acctype != TEMPVM ) 	 /* ∑«¡Ÿ ±≥µ£¨≤ª”√‘Ÿ¥Œ”√≥µ≈∆»∑»œ */
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: 1’≈ø®£¨∏√≥µŒ™∑«¡Ÿ ±≥µ£¨Œﬁ–Ë≥µ≈∆ ∂±»∑»œ", LaneClass->LaneCfg->LaneNO);
						break;
					}

					if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )  /* RFIDŒﬁÃß∏À£¨≤¢«“’˚≈≈æ‹ ∂£¨≤ª∂Ø◊˜£¨÷±Ω”…œ¥´ */
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: 1’≈ø®£¨≥µ≈∆ ∂±Œ™’˚≈∆æ‹ ∂£¨≤ª∂Ø◊˜", LaneClass->LaneCfg->LaneNO);
						break;
					}

					#if 1			/* ∑Ω±„¥”»’÷æ≈–∂œ‘≠“Ú*/
					if( (strcmp(LaneClass->Obj_Snap[0].plateno, iList->Next->tagplateno) == 0 )
					   && (strcmp(LaneClass->Obj_Snap[0].platecl,iList->Next->tagplatecl) == 0) )
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: 1’≈ø®£¨RFID∑¥≤È≥µ≈∆”ÎLPR ∂±≥µ≈∆“ª÷¬", LaneClass->LaneCfg->LaneNO);
					}
					else
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: 1’≈ø®£¨RFID∑¥≤È≥µ≈∆”ÎLPR ∂±≥µ≈∆≤ª“ª÷¬", LaneClass->LaneCfg->LaneNO);
					}
					#endif

					LaneClass->CmfType = LPR_CFM;
					LPR_Process(LaneClass);

					int j = -1;
					j = QueryLocDB_LimitTab(&(LaneClass->Obj_Snap[0]), LaneClass);

					/* ¥Ê‘⁄‘ –ÌÕ®––ªÚ’ﬂ≤ª¥Ê‘⁄,∂º»œŒ™ «ø…“‘Õ®––µƒ */
					if( j == EXIST_ALLOWPASS )
					{
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µ–≈œ¢‘ –ÌÕ®––", LaneClass->LaneCfg->LaneNO);

						/********************************Ãß∏À∂Ø◊˜≈–∂®*************************************************/
						if( LaneClass->WorkMode == Common )
						{
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,RELEASE_DOWNRAIL);	/*  Õ∑≈¬‰∏À */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,UPRAIL);				/* Ãß∏À∑≈–– */
							WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->Alarm,RELEASE_ALARM);		/*  Õ∑≈±®æØ */
							gettimeofday(&t,NULL);
							Chg_PreciseTime2Str(t, buf, sizeof(buf));

							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏ÀÃß∏À", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == FreePass )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄◊‘”…¡˜π§◊˜ƒ£ Ω£¨¿∏∏À≥£Ãß", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Deployment )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄≤º∑¿π§◊˜ƒ£ Ω£¨¿∏∏À≤ª∂Ø◊˜", LaneClass->LaneCfg->LaneNO);
						}
						else if( LaneClass->WorkMode == Exemption )
						{
							writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: LPR: ¥¶”⁄√‚ºÏπ§◊˜ƒ£ Ω£¨¿∏∏À“—∂Ø◊˜", LaneClass->LaneCfg->LaneNO);
						}
						LaneClass->Obj_Snap[0].FlagUpRail = HAD_URAILl;	//_by_dpj_2015-01-04
						/********************************************************************************************************/

						if( strlen(LaneClass->Obj_Snap[0].tagno) > 0 )		/* »Áπ˚”–∑¥≤ÈµΩ±Í«©£¨º”»ÎµΩ¬À≤®∂”¡– */
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
						writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µΩ˚÷πÕ®––", LaneClass->LaneCfg->LaneNO);
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
		LaneClass->Obj_Snap[0].t_snap=LaneClass->Time_SnapRedge;	/*  ±º‰∏≥÷µ */
		strcpy(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂");			/* ÷ª∏≥÷µ“ª¥Œ */

		#if 1	//_by_dpj_2015-01-08
		if( LaneClass->LaneCfg->Exemption == CAREXEMPTION )
		{
			if( LaneClass->WorkMode == Exemption )
			{
				if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") == 0 )
				{
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¥¶”⁄∆˚≥µ√‚ºÏπ§◊˜ƒ£ Ω£¨%s£¨¿∏∏À≤ªÃß∏À", LaneClass->LaneCfg->LaneNO, LaneClass->Obj_Snap[0].plateno);
				}
			}
		}
		#endif

		LaneClass->LprRes = HAVE_PROLPR;							/* ÷√Œª”–≥µ≈∆ ∂±¥¶¿Ì±Í÷æ */
		writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≥¨π˝≥µ≈∆ ∂±∫Û—”œﬁ∂® ±º‰£¨√ª”– ∂±Ω·π˚£¨◊‘∂Ø∏≥”Ë’˚≈∆æ‹ ∂", LaneClass->LaneCfg->LaneNO);
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
* DESCRIPTION: ◊•≈ƒœﬂ»¶∂‘œÛ∫Ø ˝
* @GPISnap 	 : ◊•≈ƒœﬂ»¶◊¥Ã¨¥Ê¥¢∆˜
* @LaneClass : ≥µµ¿∂‘œÛ
* @iList	 :  ∂±∂”¡–
* @Return 	 : no
* ----
*************************************************************************************/

void SnapCoilEdge(GPI_StatAndTime *GPISnap, GPI_StatAndTime *GPIRailStat,
						laneclass_type *LaneClass, identity_node *iList, laneclass_type *LaneClassOther)
{
	/* ◊•≈ƒœﬂ»¶…œ…˝—ÿ∂Ø◊˜∂®“Â*/
	if( (GPISnap->LState == LOWLEVEL) && (GPISnap->State == HIGHTLEVEL) )
	{
		char buf[32];
		gettimeofday(&(LaneClass->Time_SnapRedge), NULL);	/* ∂¡»°—π◊•≈ƒœﬂ»¶ ±º‰ */
		LaneClass->Lane_Info.SnapCoil = HIGHTLEVEL;		/* ≥µµ¿◊¥Ã¨÷–µƒ◊•≈ƒœﬂ»¶◊¥Ã¨Œ™∏ﬂµÁ∆Ω*/
		LaneClass->TimeAcc_Snap = 0;						/* «Âø’∂® ±¿€º”∆˜ */
		LaneClass->CmfType = NONE_CFM;					/*  ∂±ƒ£ ΩŒ™Œﬁ ∂±ƒ£ Ω*/
		LaneClass->Obj_Snap[0].FlagUpRail = NO_URAILl;		/* ≥ı ºªØÃß∏À±Í ∂Œ™ŒﬁÃß∏À*/
		LaneClass->Flag_UpLoad = LOWLEVEL;				/* ≥ı ºªØ…œ¥´π˝≥µº«¬º±Í÷æŒ™Œﬁº«¬º…œ¥´*/	//_by_dpj_2014-09-25
		LaneClass->Note = NO_NOTE;						/* ≥ı ºªØ±∏◊¢ */
		LaneClass->Snap_TagQuantity = ZEROTAG_0;			/* ¡„’≈ø® */
		LaneClass->LprRes = NOLPRRES;					/* ≥ı ºªØ≥µ≈∆ ∂±ªπ√ª”–Ω·π˚ */
		memset(LaneClass->ImageIndex, 0, sizeof(LaneClass->ImageIndex));

		LaneClass->Obj_Snap[0].t_snap = LaneClass->Time_SnapRedge;
		memset(buf, 0, sizeof(buf));
		Chg_PreciseTime2Str(LaneClass->Time_SnapRedge, buf, sizeof(buf));
		writelog(LaneLogLevel(LaneClass), "≥µµ¿%d: Ω¯»Î◊•≈ƒœﬂ»¶",LaneClass->LaneCfg->LaneNO);

		/********************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹_by_dpj_2014-01-02
		Ãıº˛: ≈–∂œ «∑Ò∏√œﬂ»¶±ªÀ¯∂®
		********************************************************************************/
		printf("[SnapCoilEdge]:SysCfg.LaneLogic = %d\n", SysCfg.LaneLogic );
		if( SysCfg.LaneLogic == USE_LANELOCK )
		{
			printf("[SnapCoilEdge]:LaneClass->LaneLockStat = %d\n", LaneClass->LaneLockStat );
			if( LaneClass->LaneLockStat == LOCK )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ◊•≈ƒœﬂ»¶“—À¯£¨Ω¯»Î≤ª∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				return;	/* »Áπ˚∏√≥µµ¿±ªÀ¯∂®£¨æÕ¡¢º¥∑µªÿ */
			}
		}
		/******************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹Ω· ¯_by_dpj_2014-01-02
		*******************************************************************************/

		/* ”–≈‰÷√RFID ±µƒ≥µµ¿∂Ø◊˜∂®“Â*/
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
	// ±º‰º∆ ˝,≥µ≈∆ ∂±Ω·π˚¥¶¿Ì
	//=================================================
	if(GPISnap->State == HIGHTLEVEL)
	{
		struct  timeval t;
		gettimeofday(&t,NULL);										/* —π◊•≈ƒœﬂ»¶ ±º‰º∆ ˝ */
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

	/* ◊•≈ƒœﬂ»¶œ¬Ωµ—ÿ∂Ø◊˜∂®“Â*/
	if((GPISnap->LState==HIGHTLEVEL)&&(GPISnap->State==LOWLEVEL))
	{
		LaneClass->Lane_Info.SnapCoil=LOWLEVEL;			/* ≥µµ¿◊¥Ã¨–≈œ¢ */

		writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿Îø™◊•≈ƒœﬂ»¶",LaneClass->LaneCfg->LaneNO);

		/********************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹_dpj_2014-01-02
		Ãıº˛: ≈–∂œ «∑Ò∏√œﬂ»¶±ªÀ¯∂®
		********************************************************************************/
		if( SysCfg.LaneLogic == USE_LANELOCK )
		{
			if( LaneClass->LaneLockStat == LOCK )		/* »Áπ˚∏√≥µµ¿±ªÀ¯∂®£¨æÕ¡¢º¥∑µªÿ */
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ◊•≈ƒœﬂ»¶“—À¯£¨¿Îø™≤ª∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				return;
			}
		}
		/******************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹Ω· ¯_dpj_2014-01-02
		*******************************************************************************/

		LaneClass->TimeAcc_Snap=0;

#if 0	//_by_dpj_2014-09-29
		/********************************************************************************
		º∆ ˝π¶ƒ‹_by_lsm_2013-12-26
		’˝≥£Õ®≥µ«Èøˆœ¬
		Ãıº˛: ≥µ¡æ¿Îø™◊•≈ƒœﬂ»¶Ω¯»Î¿∏∏Àœﬂ»¶
		********************************************************************************/
		if( LaneClass->Flag_HightSnapRail == HIGHTLEVEL
			&& LaneClass->Lane_Info.RailCoil == HIGHTLEVEL )
			//&& GPIRailStat->State==LOWLEVEL )				//_by_dpj_2014-05-07
		{
			//º∆ ˝+1
			printf("+1\n");
			gettimeofday(&(LaneClass->Time_VehicleCount),NULL);		//_by_dpj_2014_02_25
			InsLocDB_Vehicles(LaneClass);			//_dpj_2013-12-31
			In_LogQueue(logqueue, "≥µµ¿%d: ≥µ¡æº∆ ˝+1\r\n", LaneClass->LaneCfg->LaneNO,NULL,NULL, 9999, LOG_INFO);	//_by_dpj_2014-01-02
			parknum_2FIFO(LaneClass);
			//parknum_2FIFO(LaneClass->LaneCfg);
		}
		/******************************************************************************
		º∆ ˝π¶ƒ‹Ω· ¯_by_lsm_2013-12-26
		*******************************************************************************/

#endif
#if 1	//_by_dpj_2014-09-29
		/********************************************************************************
		º∆ ˝π¶ƒ‹_by_dpj_2014-08-04
		’˝≥£Õ®≥µ«Èøˆœ¬
		Ãıº˛: ≥µ¡æ¿Îø™◊•≈ƒœﬂ»¶Ω¯»Î¿∏∏Àœﬂ»¶
		********************************************************************************/
		if( SysCfg.ParknumType == TWO_COIL )
		{
			if( LaneClass->Flag_HightSnapRail == HIGHTLEVEL
				&& LaneClass->Lane_Info.RailCoil == HIGHTLEVEL)
				//&& LaneClass->Lane_Info.Rail == LOWLEVEL )  _by_dpj_2014-04-22
			{
				//º∆ ˝+1
				printf("+1\n");
				gettimeofday(&(LaneClass->Time_VehicleCount),NULL);		//_by_dpj_2014_02_25
				InsLocDB_Vehicles(LaneClass);			//_dpj_2013-12-31
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≥µ¡æº∆ ˝+1",LaneClass->LaneCfg->LaneNO);
				parknum_2FIFO(LaneClass);
			}
		}
		else if( SysCfg.ParknumType == ONE_COIL )
		{
			if( strcmp(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂") != 0 )
			{
				//º∆ ˝+1
				printf("+1\n");
				gettimeofday(&(LaneClass->Time_VehicleCount),NULL);		//_by_dpj_2014_02_25
				InsLocDB_Vehicles(LaneClass);			//_dpj_2013-12-31
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≥µ¡æº∆ ˝+1",LaneClass->LaneCfg->LaneNO);
				parknum_2FIFO(LaneClass);
			}
		}
		/******************************************************************************
		º∆ ˝π¶ƒ‹Ω· ¯_by_dpj_2014-08-04
		*******************************************************************************/
#endif

		#if 1		//_by_dpj_2014-09-25
		//–ﬁ∏ƒ‘≠“Ú: ≥µ≈∆ŒﬁΩ·π˚«“≥µ—π◊•≈ƒœﬂ»¶ ±º‰–°”⁄≥µ≈∆ ∂±∏∫∞Îæ∂ ±º‰ ±£¨Œﬁπ˝≥µº«¬º≤˙…˙
		if(LaneClass->Flag_UpLoad == LOWLEVEL)
		{
			if(strlen(LaneClass->Obj_Snap[0].plateno)==0)
			{
				strcpy(LaneClass->Obj_Snap[0].plateno, "’˚≈∆æ‹ ∂");
				LaneClass->Obj_Snap[0].acctype = TEMPVM;	//_by_dpj_2014-11-24	–ﬁ∏ƒ‘≠“Ú:  øÏÀŸΩ¯»Î¿Îø™◊•≈ƒ ±£¨√ª”–≥µ≈∆ªÿµ˜Ω·π˚£¨√ª”–…œ¥´º«¬º£¨≥µ¡æ’À∫≈¿‡–ÕŒ™0
			}
			ar_2FIFO(LaneClass);
		}
		#endif


		Delete_SExitDupNode(Exist_DupList, (identity_node *)&(LaneClass->Obj_Snap));	//_by_dpj_2013-08-05,Ω‚æˆ∏˙≥µ“ªø®∂‡¥Œ¥Ê‘⁄Œ Ã‚
		//Clear_IdentityDupList(iList);  //shl_2car_debug_20131017
		memset(&(LaneClass->Obj_Snap[0]),0,sizeof(identity_node));		/* ◊•≈ƒœﬂ»¶œ¬Ωµ—ÿ£¨«Â≥˝µÙ◊•≈ƒ ∂±∂‘œÛ */
		memset(&(LaneClass->Obj_Snap[1]),0,sizeof(identity_node));		/* ◊•≈ƒœﬂ»¶œ¬Ωµ—ÿ£¨«Â≥˝µÙ◊•≈ƒ ∂±∂‘œÛ */

		LaneClass->Obj_Snap[0].FlagUpRail = NO_URAILl;				//_by_dpj_2015-01-04
	}


	//_by_lsm_2013-09-22,Ω‚æˆ∫£øµªÿµ˜¬˝‘Ï≥…∂‘œÛ ˝æ›Œ¥…æ≥˝Œ Ã‚
/*	if((GPISnap->LState==LOWLEVEL)&&(GPISnap->State==LOWLEVEL)
		&&(strlen(LaneClass->Obj_Snap.plateno) > 0))
		{
			memset(LaneClass->Obj_Snap.plateno,0,sizeof(LaneClass->Obj_Snap.plateno));
		}*/
}

//=======================================================================
//—ÿ¥¶¿Ì◊”∫Ø ˝£¨¿∏∏Àœﬂ»¶…œ…˝°¢œ¬Ωµ—ÿ
//=======================================================================
void RailCoilEdge(GPI_StatAndTime *GPISnap,GPI_StatAndTime *GPIRailStat,GPI_StatAndTime *GPIRail,laneclass_type *LaneClass,laneclass_type *LaneClassOther)
{

	if((GPIRail->LState==LOWLEVEL)&&(GPIRail->State==HIGHTLEVEL))
	{
		LaneClass->Lane_Info.RailCoil=HIGHTLEVEL;		/* ≥µµ¿◊¥Ã¨–≈œ¢ */

		writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≥µ¡æΩ¯»Î¿∏∏Àœﬂ»¶",LaneClass->LaneCfg->LaneNO);

		/********************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹_dpj_2014-01-02
		Ãıº˛: ≈–∂œ «∑Ò∏√œﬂ»¶±ªÀ¯∂®
		********************************************************************************/
		if( SysCfg.LaneLogic == USE_LANELOCK )
		{
			if( LaneClass->LaneLockStat == LOCK )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏Àœﬂ»¶“—À¯£¨Ω¯»Î≤ª∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				return;			/* »Áπ˚±æ≥µµ¿±ªÀ¯‘⁄£¨ÕÀ≥ˆ */
			}
			else if( GPISnap->State == HIGHTLEVEL )		/* »Áπ˚√ª±ªÀ¯◊°£¨¿∏∏Àœﬂ»¶…œ…˝—ÿ ± */
			{
				writelog(LaneLogLevel(LaneClass), "≥µµ¿%d: À¯≥µµ¿%d",LaneClass->LaneCfg->LaneNO,LaneClassOther->LaneCfg->LaneNO);
				LaneClassOther->LaneLockStat = LOCK;			/* »Áπ˚∏√≥µµ¿◊•≈ƒœﬂ»¶ «Œﬁ≥µ£¨‘Ú±Ì */
															/*  æ «∂‘∑Ω≥µµ¿ø™π˝¿¥µƒ≥µ */
			}
		}
		/******************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹Ω· ¯_dpj_2014-01-02
		*******************************************************************************/

#if 1	//_by_dpj_2013-4-22	∏˘æ›»Ì2 57∫≈µÿø‚ µº «Èøˆ∂¯∏ƒright//_by_pbh_2015-01-14–ﬁ∏ƒ Õ∑≈Ãß∏Àµ„‘⁄"Ω¯»Î¿∏∏Àœﬂ»¶"
		if( LaneClass->WorkMode != FreePass )
		{
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,RELEASE_UPRAIL);	/*  Õ∑≈Ãß∏À */
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: Ω¯»Î∫Ûµÿ Õ∑≈Ãß∏À",LaneClass->LaneCfg->LaneNO);
		}
#endif

		#if 1		//_by_dpj_2013-5-3			ÃÌº”≈–∂œ «∑Ò∆˚≥µÃıº˛
		if( LaneClass->Lane_Info.RailCoil == HIGHTLEVEL &&
			LaneClass->Lane_Info.SnapCoil == HIGHTLEVEL )
		{
			LaneClass->Flag_HightSnapRail=HIGHTLEVEL;
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µŒ™∆˚≥µ",LaneClass->LaneCfg->LaneNO);
		}
		else
		{
			LaneClass->Flag_HightSnapRail=LOWLEVEL;
			writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏√≥µŒ™∑«∆˚≥µ",LaneClass->LaneCfg->LaneNO);
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

		/* «Â≥˝∏√∂‘œÛª·µº÷¬º∆ ˝Œﬁ∑®Ω¯––∂‘œÛº‰µƒ ±º‰±»Ωœ£¨º∆ ˝ª·Œﬁ∑®≤˙…˙∂Ø◊˜*/
		#if 0		//_by_shl_2014-02-28	error
		memset(&(LaneClass->Obj_Snap[0]),0,sizeof(identity_node));	/* «Â≥˝∂‘œÛ◊•≈ƒœﬂ»¶…œµƒ∂‘œÛ */
		memset(&(LaneClass->Obj_Snap[1]),0,sizeof(identity_node));
		#endif
	}
	else if((GPIRail->LState==HIGHTLEVEL)&&(GPIRail->State==LOWLEVEL))
	{
		LaneClass->Lane_Info.RailCoil=LOWLEVEL;						/* ≥µµ¿◊¥Ã¨–≈œ¢ */

		writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿Îø™¿∏∏Àœﬂ»¶\r\n",LaneClass->LaneCfg->LaneNO);

		#if 1
		/********************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹_dpj_2014-01-02
		Ãıº˛: ≈–∂œ «∑Ò∏√œﬂ»¶±ªÀ¯∂®
		********************************************************************************/
		if( SysCfg.LaneLogic == USE_LANELOCK )
		{
			printf("[RailCoilEdge]:LaneClass->LaneLockStat = %d\n", LaneClass->LaneLockStat );
			if( LaneClass->LaneLockStat == LOCK )
			{
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏Àœﬂ»¶“—À¯£¨¿Îø™≤ª∂Ø◊˜",LaneClass->LaneCfg->LaneNO);
				//LaneClass->LaneLockStat = UNLOCK;		/* Ω‚À¯π¶ƒ‹ºØ÷–µΩ“ª¥¶freeLocked»•≈–∂œ*/
				return;
			}
		}
		/******************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹Ω· ¯_dpj_2014-01-02
		*******************************************************************************/
		#endif

		/********************************************************************************
		º∆ ˝π¶ƒ‹_by_dpj_2014-08-04
		∏˙≥µ«Èøˆœ¬
		Ãıº˛: ¿∏∏ÀÃß∆, ≥µ¡æ¿Îø™¿∏∏Àœﬂ»¶, ◊•≈ƒ∂‘œÛµ»”⁄¿∏∏À∂‘œÛ
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
				//º∆ ˝+1
				printf("+1\n");
				gettimeofday(&(LaneClass->Time_VehicleCount),NULL);		//_by_dpj_2014_02_25
				InsLocDB_Vehicles(LaneClass);			//_dpj_2013-12-31
				writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ≥µ¡æº∆ ˝+1",LaneClass->LaneCfg->LaneNO);
				parknum_2FIFO(LaneClass);
			}
		}
		/******************************************************************************
		º∆ ˝π¶ƒ‹Ω· ¯_by_dpj_2014-08-04
		*******************************************************************************/

		if(LaneClass->Flag_HightSnapRail==HIGHTLEVEL)					/* Õ≥º∆ */
		{
			#if 0 //_shl_Cancel_Statistics_20140212
			LaneClass->Flag_HightSnapRail=LOWLEVEL;
			if(LaneClass->Obj_Rail[0].ReadCardMom == NoCard){
				InsLocDB_Statistics(LaneClass->LaneCfg->RFIDAntID,NoCard);	/* Œﬁø®≥µŒﬁantennaID */
			}
			else{															/* ”–ø®≥µ */
				InsLocDB_Statistics(LaneClass->Obj_Rail[0].antennaID,
					LaneClass->Obj_Rail[0].ReadCardMom);
			}
			#endif
		}

		//by_dpj_2013-4-19	»Ìº˛‘∞∂˛∆⁄≤‚ ‘∑¢œ÷∏√∂Ø◊˜¥Ê‘⁄Œ Ã‚error
		#if 0	//question
		if((GPISnap ->State != HIGHTLEVEL)  || (LaneClass->FlagUpRail  != HAD_URAILl))
		{
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp,RELEASE_UPRAIL);	/*  Õ∑≈Ãß∏À */
			WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ¬‰∏À */
		}
		#endif

		#if 0	//_by_dpj_2015-01-04 //_by_pbh_2015-01-14–ﬁ∏ƒ Õ∑≈Ãß∏Àµ„‘⁄"Ω¯»Î¿∏∏Àœﬂ»¶"
		if( LaneClass->WorkMode != FreePass )
		{
			if( LaneClass->LaneCfg->LprCfg == HAVECFG )
			{
				if( (strcmp(LaneClass->Obj_Rail[0].plateno, LaneClass->Obj_Snap[0].plateno) == 0)
					&& (strcmp(LaneClass->Obj_Rail[0].platecl, LaneClass->Obj_Snap[0].platecl) == 0) )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/*  Õ∑≈Ãß∏À */
					//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ¬‰∏À */
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏˙≥µ Õ∑≈Ãß∏À",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->Obj_Snap[0].FlagUpRail == NO_URAILl)
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/*  Õ∑≈Ãß∏À */
					//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ¬‰∏À */
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d:  Õ∑≈Ãß∏À",LaneClass->LaneCfg->LaneNO);
				}

			}
			else if( LaneClass->LaneCfg->RfidCfg == HAVECFG )
			{
				if( (strcmp(LaneClass->Obj_Rail[0].tagno, LaneClass->Obj_Snap[0].tagno) == 0)
					|| ((strcmp(LaneClass->Obj_Rail[0].tagplateno, LaneClass->Obj_Snap[0].tagplateno) == 0)
					&& (strcmp(LaneClass->Obj_Rail[0].tagplatecl, LaneClass->Obj_Snap[0].tagplatecl) == 0)) )
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/*  Õ∑≈Ãß∏À */
					//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ¬‰∏À */
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ∏˙≥µ Õ∑≈Ãß∏À",LaneClass->LaneCfg->LaneNO);
				}
				else if( LaneClass->Obj_Snap[0].FlagUpRail == NO_URAILl)
				{
					WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/*  Õ∑≈Ãß∏À */
					//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ¬‰∏À */
					writelog(LaneLogLevel(LaneClass),"≥µµ¿%d:  Õ∑≈Ãß∏À",LaneClass->LaneCfg->LaneNO);
				}

			}
			#if 0	//_by_dpj_2014-10-03
			if( LaneClass->Obj_Snap.FlagUpRail == LOWLEVEL )
			{
				WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailUp, RELEASE_UPRAIL);	/*  Õ∑≈Ãß∏À */
				//WriteGPO_REG_MachUpCFG(LaneClass->LaneCfg->RailDown,DOWNRAIL);		/* ¬‰∏À */
				printf("≥µµ¿%d:  Õ∑≈Ãß∏À\n");
				In_LogQueue(logqueue, "≥µµ¿%d:  Õ∑≈Ãß∏À\n", LaneClass->LaneCfg->LaneNO,NULL,NULL, 9999, LOG_INFO);
			}
			#endif
		}
		#endif

		memset(&(LaneClass->Obj_Rail[0]),0,sizeof(identity_node));	/* «Â≥˝∂‘œÛ◊•≈ƒœﬂ»¶…œµƒ∂‘œÛ */
		memset(&(LaneClass->Obj_Rail[1]),0,sizeof(identity_node));	/* «Â≥˝∂‘œÛ◊•≈ƒœﬂ»¶…œµƒ∂‘œÛ */
	}

}
//=======================================================================
//—ÿ¥¶¿Ì◊”∫Ø ˝£¨¿∏∏À◊¥Ã¨…œ…˝°¢œ¬Ωµ—ÿ
void RailStateEdge(GPI_StatAndTime *GPIRailStat,laneclass_type *LaneClass)
{
	if((GPIRailStat->LState==LOWLEVEL)&&(GPIRailStat->State==HIGHTLEVEL))		/* ∂¡»°¿∏∏À◊¥Ã¨ */
	{
		#if 0
		/********************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹_dpj_2014-01-02
		Ãıº˛: ≈–∂œ «∑Ò∏√œﬂ»¶±ªÀ¯∂®
		********************************************************************************/
		if(SysCfg.LaneLogic==USE_LANELOCK)
		{
			if(LaneClass->LaneLockStat==LOCK)return;
		}
		/******************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹Ω· ¯_dpj_2014-01-02
		*******************************************************************************/
		#endif

		LaneClass->Lane_Info.Rail=LOWLEVEL;
		writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏À◊¥Ã¨Œ™µÕµÁ∆Ω",LaneClass->LaneCfg->LaneNO);
	}
	else if((GPIRailStat->LState==HIGHTLEVEL)&&(GPIRailStat->State==LOWLEVEL))		/* ∂¡»°¿∏∏À◊¥Ã¨ */
	{
		#if 0
		/********************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹_dpj_2014-01-02
		Ãıº˛: ≈–∂œ «∑Ò∏√œﬂ»¶±ªÀ¯∂®
		********************************************************************************/
		if(SysCfg.LaneLogic==USE_LANELOCK)
		{
			if(LaneClass->LaneLockStat==LOCK)return;
		}
		/******************************************************************************
		Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹Ω· ¯_dpj_2014-01-02
		*******************************************************************************/
		#endif

		LaneClass->Lane_Info.Rail=HIGHTLEVEL;
		writelog(LaneLogLevel(LaneClass),"≥µµ¿%d: ¿∏∏À◊¥Ã¨Œ™∏ﬂµÁ∆Ω",LaneClass->LaneCfg->LaneNO);
	}
}



//=======================================================================
//—ÿ ¬º˛¥¶¿Ì
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

	//by_lsm_20131226		π˝≥µº∆ ˝
	for(i=0;i<8;i++)
	{
		if(GPI[i] == LaneClass[0].LaneCfg->RailStat) r[0]=&(GPIStat[i]);
		else if(GPI[i] == LaneClass[1].LaneCfg->RailStat) r[1]=&(GPIStat[i]);
	}

	/********************************************************************************
	Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹_by_dpj_2014-01-02
	Ãıº˛: ≈–∂œ «∑Ò∏√œﬂ»¶±ªÀ¯∂®
	********************************************************************************/
	if( SysCfg.LaneLogic == USE_LANELOCK )
	{
		FreeLocked();
	}
	/******************************************************************************
	Õ¨Ω¯Õ¨≥ˆπ¶ƒ‹Ω· ¯_by_dpj_2014-01-02
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
* DESCRIPTION: GPIOø⁄≥ı ºªØ
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
	gpo_write(GPO_L[8]);		//_by_dpj_2014-09-17	–ﬁ∏ƒ‘≠“Ú: GPO≥ı º∏≥÷µ¥ÌŒÛ

	/* Ω®¡¢¥Ê‘⁄∂”¡–,¬À≤®∂”¡–∫Õ ∂±∂”¡– */
	//struct  timeval t; 	/* ∂¡»°æ´»∑ ±º‰ */
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

	#if 1		/* ≥ı ºªØº∆ ˝≥µ¡æ¿‡–Õ πƒ‹_by_dpj_2014-09-02*/
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

	/* ≥ı ºªØ…œ¥´ µ ± ˝æ›∂”¡– */
	if( (upload_queue=(Fifo_Qnode*)malloc(sizeof(Fifo_Qnode))) == NULL )
	{
		writelog(ZLOG_LEVEL_SYS,"∑÷≈‰ µ ±…œ¥´∂”¡–µƒƒ⁄¥Êø’º‰ ß∞‹£¨¥À¥¶ÕÀ≥ˆ");		return -1;
	}
	Init_FifoQueue(upload_queue);
	writelog(ZLOG_LEVEL_SYS,"…œ¥´ µ ± ˝æ›∂”¡–¥¥Ω®≤¢≥ı ºªØ≥…π¶!!");


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
			writelog(LaneLogLevel(planeclass), "≥µµ¿%d: Ω‚À¯", LaneClass[0].LaneCfg->LaneNO);
		}
		else if( LaneClass[1].LaneLockStat == LOCK )
		{
			LaneClass[1].LaneLockStat = UNLOCK;
			planeclass = &LaneClass[1];
			writelog(LaneLogLevel(planeclass), "≥µµ¿%d: Ω‚À¯", LaneClass[1].LaneCfg->LaneNO);
		}
	}
	return;
}



