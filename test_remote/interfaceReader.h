/************************************************************************
* ��Ȩ���� (C)2001, ����������ͨѶ�ɷ����޹�˾��
* 
* �ļ����ƣ� readerInterface.h
* �ļ���ʶ�� 
* ����ժҪ�� �Ķ������⿪���ӿ�����
* ����˵���� 
* ��ǰ�汾�� 
* ��    �ߣ� luna
* ������ڣ� 2012��8��1��
* 
* �޸ļ�¼1��// �޸���ʷ��¼�������޸����ڡ��޸��߼��޸�����
*    �޸����ڣ�
*    �� �� �ţ�
*    �� �� �ˣ�
*    �޸����ݣ� 
* �޸ļ�¼2����
************************************************************************/

#ifndef	_READER_INTERFACE_H_
#define	_READER_INTERFACE_H_


#pragma once



/**************************************************************************
 *                        ����                                            *
 **************************************************************************/



/**************************************************************************
 *                          �궨��                                         *
 **************************************************************************/
#define IN
#define OUT
#define __stdcall


#define PERIOD_RW_RESULT_BUFFER 1024*3

/**************************************************************************
 *                            ��������                                     *
 **************************************************************************/

typedef struct
{
	int	fd;				                //���ӱ�ʶ��
	unsigned char address[20];			//IP��ַ�򴮿ڣ���"COM1"/"tty1"��"10.86.10.56"
	int linkMode;					    //0-����ͻ��ˣ�1-��������;2-���ڣ�3-U��
}ReaderItem;

//��ѯ�汾
typedef struct 
{
	unsigned char hVersion[128]; //Ӳ���汾,��ͬӲ��֮��İ汾ͨ����:���ֿ�
	unsigned char fVersion[128]; //ǰ̨�汾��V��汾��.�м�汾��.С�汾��
	unsigned char bVersion[128]; //��̨�汾��V��汾��.�м�汾��.С�汾�ţ�����V3.00.10
} Version;					

//�Ķ������ƽṹ
typedef struct 
{
	int length;							//�Ķ������Ƴ���
	unsigned char name[20];				//�Ķ�������
} ReaderID;

//�Ķ�����ʶ�ṹ
typedef struct 
{
	int IDType;							//�Ķ�����ʶ���ͣ�0��ʾ��mac��ַ��ʶ�Ķ�����1��ʾ��epc���ʶ�Ķ���
	ReaderID readerID;			//�Ķ�����ʶ���ַ�����
} Identification;			



//������߽ṹ
typedef struct 
{
	unsigned char antenna[4];//����1-4�� 0��ʾ����δ���ã�1��ʾ��������
} Antenna;	

//���������
typedef struct 
{
	short antennaID;	
	short tagIDLen;
	unsigned char tagID[64];	//��ǩ����	
}TagInfo;

typedef struct 
{	
	int num; //��ǩ����	
	TagInfo tagInfo[256];		
}InventoryResult;


//ISO18000-6-C��ǩ�ṹ��
typedef struct
{
	int  tagDataLen; 			    //��ǩ���ݳ��ȣ����ֽ�Ϊ��λ
	unsigned char aucTagData[64];  //�䳤EPC���ݵĵ�һ�����ݶ�
} ISO_6C_TagID;

//ISO18000-6-C������
typedef struct 
{	
	int antennaID;				 //���ߺ�	
	int memBank;       		 //0-��������-EPC��-TID��-User
	int offset;  		 //����ʼ��ַ��ż��
	int length;     			 //��ȡ���ȣ���λ�ֽ�,����Ϊ����
	unsigned char accessPassword[4];    //�������룬4���ֽ�
	ISO_6C_TagID  tagID;         //��ǩID
} ISO_6C_TagRead;
//ע�⣺	
//offsetȡֵ��memBankӰ�죬length��memBank��offset��Ӱ�졣
//��1��	memBank = 0ʱ������������offset�ɴ�0��8���ֽڡ�
//��2��	memBank = 1ʱ����epc����offset�ɴ�4��62���ֽڡ�
//��3��	memBank = 2ʱ����tid����offset�ɴ�0��510���ֽڣ���ͬ����tid����С��һ��
//��4��	memBank = 3ʱ����user����offset�ɴ�0��510���ֽڣ���ͬ����user����С��һ��

//ISO18000-6-C�����
typedef struct 
{	
	int dataLen;      		  //���ݳ��ȣ����ֽ�Ϊ��λ��
	unsigned char  readData[512]; 
} ReadResult;

//ISO18000-6-Cд����
typedef struct 
{ 
	int antennaID;      		//���ߺ�	
	int memBank;       		//0������������EPC����User
	int offset;  		 //д��ʼ��ַ������Ϊ����
	int length;     			 //���ȣ���λ�ֽ�,����Ϊ����
	unsigned char accessPassword[4];    //�������룬���ֽ�
	unsigned char data[512];	//д����
	ISO_6C_TagID  tagID;         //��ǩID	

} ISO_6C_TagWrite;
//ע�⣺
//(1)	memBank = 0ʱ�� offsetȡֵֻ����0��4��д���ݵĳ��ȣ��֣��Ĵ�Сֻ����4����offset = 0ʱ����ʾд����ɱ�����룬��offset = 4ʱ����ʾд���Ƿ������롣
//ɱ��������ɱ����ǩ����ʱ�õ��������������ڶ���д�������Ȳ�����00000000���������룬����ǩû�б�����ʱ����������00000000���ɡ�
//��������ǩ������ʱ����ȡ����ʱ��Ҫ����������룬�����ܶ�ȡ�ɹ�����������������ִ�ж������������롣
//ע�⣺ɱ������������������ܽ��У�����Ϊ00000000��Ϊ�����롣
//(2)	memBank = 1ʱ�� дEPC����д��������ʼ��ַ+���Ȳ��ܳ���epc�����ȣ�һ����ʼ��ַ��4�ֽڿ�ʼ������Ϊ12���ֽڡ�
//(3)	memBank = 3ʱ�� дuser����д��������ʼ��ַ+���Ȳ��ܳ���user�����ȣ� user�����510�ֽڡ�

//ISO18000-6-C������
typedef struct 
{
	int antennaID;       //���ߺ�
	int killBankOp;	     //ɱ��������������������
	int accessBankOp;      //����������������������
	int epcBankOp;        // EPC������������
	int tidBankOp;        // TID������������
	int userBankOp;       // USER������������
	unsigned char accessPassword[4];    //�������룬4���ֽ�
	ISO_6C_TagID  tagID;         //��ǩID
} ISO_6C_TagLock;
//ע�⣺killBankOp��accessBankOp��epcBankOp��tidBankOp��userBankOp��
//ȡֵ��Χ��[0,4]���京��ֱ��ǣ�0����������1��������2����������3�����ý�����4��������

//ISO18000-6-Cɱ������
typedef struct 
{
	int antennaID;	
	unsigned char killPassword[4];    //ɱ�����룬4���ֽ�
	ISO_6C_TagID  tagID;         //��ǩID
} ISO_6C_TagKill;

//ISO18000-6-B������
typedef struct
{
	int  tagDataLen; 			    //Ĭ��8��Ŀǰ���ɸı�
	unsigned char aucTagData[8];  
} ISO_6B_TagID;

typedef struct 
{
	int antennaID;      			  //���ߺ�	
	int  offset;       //��ʼ�ֽڵ�ַ
	int  length;      //����
	ISO_6B_TagID  tagID;
} ISO_6B_TagRead;
//ע�⣺offsetȡֵ��Χ��[8,223]��length��ȡֵ��Χ��[1,224 - offset]��


//ISO18000-6-Bд����
typedef struct 
{
	int antennaID;     		       //���ߺ�
	int offset;         //��ʼ��ַ
	int length;         //����
	unsigned char data[256];           //�������ݣ������ַ���������Ϊ����
	ISO_6B_TagID  tagID;
}ISO_6B_TagWrite;
//ע�⣺offsetȡֵ��Χ��[8��223]��д���ȣ����ٸ��ֽڣ���ȡֵ��Χ��[1 , 224 - offset]��

//ISO18000-6-B������
typedef struct 
{
	int antennaID;      		       //���ߺ�	
	int offset;        //��ʼ��ַ
	int length;       //������ 
	ISO_6B_TagID  tagID;
}ISO_6B_TagLock;
//ע�⣺offsetȡֵ��Χ��[8��223]��length��ȡֵ��Χ��[1, 224 - offset]��

//ISO18000-6-B�����
typedef struct 
{	
	unsigned char  lockData[256]; //0��ʾ�����ɹ���������ʾ����ʧ��
} LockResult;

//ISO18000-6-B����ѯ����
typedef struct 
{
	int antennaID;					//���ߺ�
	int  offset;       	//��ʼ��ַ
	int  length;	//����ѯ����
	ISO_6B_TagID  tagID;
} ISO_6B_TagQueryLock;
//ע�⣺offsetȡֵ��Χ��[8��223]��length��ȡֵ��Χ��[1, 224 - offset]��

//ISO18000-6-B����ѯ���
typedef struct 
{	
	unsigned char  queryLockData[256]; //0��δ������1����������������ѯ����
} QueryLockResult;

//Alien��ǩ������������
typedef struct 
{	int antennaID;     		     //���ߺ�
unsigned char  ReadLock[8];  //0����������1��������
unsigned char  WriteLock[8]; //0��д������1��д������2��������
unsigned char  accessPassword[4]; //�������룬4���ֽ�
ISO_6C_TagID  tagID;            //��ǩID
} Alien_TagLock;



//��д�������䳤��
typedef struct 
{		
	unsigned short  protocolType;             //Э������,0-6C,1-6B,2-Impinj
	unsigned short  RWOpNum;              //�·��Ķ�д������������
	unsigned short  tagMaskMsgLen;              //������Ϣ����
	unsigned short  totalLenOfRWOpInf;              //��ǩ�����ܳ���
	unsigned char accessPassword[4];    //�������룬4���ֽ�	 
	unsigned char*   maskAndCmdBuffer;	  //����������
} TagReadWrite;
//ע�����������ݣ�TagMaskInf+RWOpInf+RWOpInf����RWOpInf���˸�


typedef struct 
{
	int RWNum;  
	TagReadWrite tagReadWrite[3]; 
}Period_Read_Write;

//ѭ����д���
typedef struct 
{	
	short  tagNum;         //��ǩ��������Ϊ0ʱ������û����㵽��ǩ
	short  totalLenOfSingleRwTagInfo;       //�������б�ǩ��SingleRwTagInfo�ܳ���
	unsigned char * SingleRwTagInfoBuffer;//���ص�����ǩ��д��Ϣ�Ľ������
		//�������ݣ�SingleRwTagInfo+SingleRwTagInfo+��
} RWResult;

typedef struct 
{	
	short  tagNum;         //��ǩ��������Ϊ0ʱ������û����㵽��ǩ
	short  totalLenOfSingleRwTagInfo;       //�������б�ǩ��SingleRwTagInfo�ܳ���
	unsigned char SingleRwTagInfoBuffer[PERIOD_RW_RESULT_BUFFER];//���ص�����ǩ��д��Ϣ�Ľ������
	//�������ݣ�SingleRwTagInfo+SingleRwTagInfo+��
} RWResultTmp;



//Impinj��˽��ת������
typedef struct 
{	
	int	antennaID;      		       //���ߺ�	
	int dataProfile;				 //��˽��ת������1-˽ת����0-��ת˽��
	int persistence;       		 //��˽��ת�����ʣ�0-��ʱת����1-����ת��
	unsigned char accessPassword[4];    //�������룬4���ֽ�
	ISO_6C_TagID  tagID;         //��ǩID
} TagMode;



//Impinj��˽��ת����ѯ����
typedef struct 
{	
	int antennaID;      		       //���ߺ�	
	unsigned char accessPassword[4];    //�������룬4���ֽ�
	ISO_6C_TagID  tagID;         //��ǩID
} TagModeQuery;




/**************************************************************************
 *                         ȫ�ֱ�������                                    *
 **************************************************************************/


/**************************************************************************
 *                        ȫ�ֺ���ԭ��                                     *
 **************************************************************************/
extern int __stdcall Connect(IN const char *dev, OUT int *fd);/*  ���ñ����帴λ  */
extern int __stdcall Disconnect(IN int fd);
extern int __stdcall StartListen(IN int port, IN int maxLinkNum);
extern int __stdcall StopListen();
extern int __stdcall GetReaders(OUT ReaderItem *info, IN int maxCount, OUT int* count);
extern int __stdcall GetReaderStatus(IN int fd, OUT ReaderItem *info);

extern int __stdcall ISO_6C_Inventory(IN int fd, IN const Antenna *antennaID, OUT InventoryResult *result);
extern int __stdcall ISO_6C_Read(IN int fd, IN const ISO_6C_TagRead *readPara, OUT ReadResult *result);
extern int __stdcall ISO_6C_Write(IN int fd, IN const ISO_6C_TagWrite *writePara);
extern int __stdcall ISO_6C_Lock(IN int fd, IN const ISO_6C_TagLock *lockPara);
extern int __stdcall ISO_6C_Kill(IN int fd, IN const ISO_6C_TagKill *killPara);
extern int __stdcall ISO_6C_StartPeriodInventory(IN int fd, IN const Antenna *antennaID, IN int timespan);
extern int __stdcall ISO_6C_GetPeriodInventoryResult (IN int fd, OUT InventoryResult *result, IN int timeOut);
extern int __stdcall ISO_6C_StopPeriodInventory(IN int fd);
extern int __stdcall ISO_6C_BlockWrite(IN int fd, IN const ISO_6C_TagWrite *writePara);

extern int __stdcall ISO_6B_Inventory(IN int fd, IN const Antenna *antennaID , OUT InventoryResult *result);
extern int __stdcall ISO_6B_Read(IN int fd, IN const ISO_6B_TagRead *readPara, OUT ReadResult *result);
extern int __stdcall ISO_6B_Write(IN int fd, IN const ISO_6B_TagWrite *writePara);
extern int __stdcall ISO_6B_Lock (IN int fd, IN const ISO_6B_TagLock *lockPara, OUT LockResult *result);
extern int __stdcall ISO_6B_QueryLock(IN int fd, IN ISO_6B_TagQueryLock *queryLockPara, OUT QueryLockResult *result);
extern int __stdcall ISO_6B_StartPeriodInventory(IN int fd, IN const Antenna *antennaID , IN int timespan);
extern int __stdcall ISO_6B_GetPeriodInventoryResult(IN int fd, OUT InventoryResult *result, IN int timeOut);
extern int __stdcall ISO_6B_StopPeriodInventory(IN int fd);

extern int __stdcall Impinj_Invent(IN int fd, IN const Antenna *antennaID, OUT InventoryResult *result);
extern int __stdcall Impinj_Read(IN int fd, IN const ISO_6C_TagRead *readPara, OUT ReadResult *result);
extern int __stdcall Impinj_Write(IN int fd, IN const ISO_6C_TagWrite *writePara);
extern int __stdcall Impinj_Lock(IN int fd, IN const ISO_6C_TagLock *lockPara);
extern int __stdcall Impinj_Kill(IN int fd, IN const ISO_6C_TagKill *killPara);
extern int __stdcall Impinj_StartPeriodInventory(IN int fd, IN const Antenna *antennaID, IN int timespan);
extern int __stdcall Impinj_GetPeriodInventoryResult(IN int fd,OUT  InventoryResult *result,IN int timeOut);
extern int __stdcall Impinj_StopPeriodInventory(IN int fd);
extern int __stdcall StartPeriodRW(IN int fd, IN const Antenna *antennaID, IN const Period_Read_Write *RWPara , IN int timespan);
extern int __stdcall GetPeriodRWResult (IN int fd, OUT RWResult *result, IN int timeOut);
extern int __stdcall StopPeriodRW(IN int fd);
extern int __stdcall Impinj_SetMode (IN int fd, IN const TagMode *modePara);
extern int __stdcall Impinj_GetMode(IN int fd, IN const TagModeQuery *tagModeQuery, OUT int *result);


#endif  /* _READER_INTERFACE_H_ */


