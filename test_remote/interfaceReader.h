/************************************************************************
* 版权所有 (C)2001, 深圳市中兴通讯股份有限公司。
* 
* 文件名称： readerInterface.h
* 文件标识： 
* 内容摘要： 阅读器对外开发接口声明
* 其它说明： 
* 当前版本： 
* 作    者： luna
* 完成日期： 2012年8月1日
* 
* 修改记录1：// 修改历史记录，包括修改日期、修改者及修改内容
*    修改日期：
*    版 本 号：
*    修 改 人：
*    修改内容： 
* 修改记录2：…
************************************************************************/

#ifndef	_READER_INTERFACE_H_
#define	_READER_INTERFACE_H_


#pragma once



/**************************************************************************
 *                        常量                                            *
 **************************************************************************/



/**************************************************************************
 *                          宏定义                                         *
 **************************************************************************/
#define IN
#define OUT
#define __stdcall


#define PERIOD_RW_RESULT_BUFFER 1024*3

/**************************************************************************
 *                            数据类型                                     *
 **************************************************************************/

typedef struct
{
	int	fd;				                //链接标识符
	unsigned char address[20];			//IP地址或串口；如"COM1"/"tty1"，"10.86.10.56"
	int linkMode;					    //0-网络客户端；1-网络服务端;2-串口；3-U口
}ReaderItem;

//查询版本
typedef struct 
{
	unsigned char hVersion[128]; //硬件版本,不同硬件之间的版本通过’:’分开
	unsigned char fVersion[128]; //前台版本，V大版本号.中间版本号.小版本号
	unsigned char bVersion[128]; //后台版本，V大版本号.中间版本号.小版本号，例：V3.00.10
} Version;					

//阅读器名称结构
typedef struct 
{
	int length;							//阅读器名称长度
	unsigned char name[20];				//阅读器名称
} ReaderID;

//阅读器标识结构
typedef struct 
{
	int IDType;							//阅读器标识类型，0表示用mac地址标识阅读器，1表示用epc码标识阅读器
	ReaderID readerID;			//阅读器标识的字符数组
} Identification;			



//清点天线结构
typedef struct 
{
	unsigned char antenna[4];//天线1-4， 0表示天线未启用，1表示天线在用
} Antenna;	

//清点结果参数
typedef struct 
{
	short antennaID;	
	short tagIDLen;
	unsigned char tagID[64];	//标签内容	
}TagInfo;

typedef struct 
{	
	int num; //标签个数	
	TagInfo tagInfo[256];		
}InventoryResult;


//ISO18000-6-C标签结构体
typedef struct
{
	int  tagDataLen; 			    //标签数据长度，以字节为单位
	unsigned char aucTagData[64];  //变长EPC数据的第一个数据段
} ISO_6C_TagID;

//ISO18000-6-C读参数
typedef struct 
{	
	int antennaID;				 //天线号	
	int memBank;       		 //0-密码区；-EPC；-TID；-User
	int offset;  		 //读起始地址，偶数
	int length;     			 //读取长度，单位字节,不能为奇数
	unsigned char accessPassword[4];    //操作密码，4个字节
	ISO_6C_TagID  tagID;         //标签ID
} ISO_6C_TagRead;
//注意：	
//offset取值受memBank影响，length受memBank和offset的影响。
//（1）	memBank = 0时，读密码区，offset可从0读8个字节。
//（2）	memBank = 1时，读epc区，offset可从4读62个字节。
//（3）	memBank = 2时，读tid区，offset可从0读510个字节，不同厂商tid区大小不一。
//（4）	memBank = 3时，读user区，offset可从0读510个字节，不同厂商user区大小不一。

//ISO18000-6-C读结果
typedef struct 
{	
	int dataLen;      		  //数据长度，以字节为单位。
	unsigned char  readData[512]; 
} ReadResult;

//ISO18000-6-C写参数
typedef struct 
{ 
	int antennaID;      		//天线号	
	int memBank;       		//0：密码区；：EPC；：User
	int offset;  		 //写起始地址，不能为奇数
	int length;     			 //长度，单位字节,不能为奇数
	unsigned char accessPassword[4];    //操作密码，个字节
	unsigned char data[512];	//写内容
	ISO_6C_TagID  tagID;         //标签ID	

} ISO_6C_TagWrite;
//注意：
//(1)	memBank = 0时， offset取值只能是0或4，写数据的长度（字）的大小只能是4。当offset = 0时，表示写的是杀死密码，当offset = 4时，表示写的是访问密码。
//杀死密码在杀死标签操作时用到，操作密码用于读、写、锁定等操作。00000000代表无密码，当标签没有被锁定时，密码输入00000000即可。
//当锁定标签密码区时，读取密码时需要输入操作密码，否则不能读取成功，其他区域锁定后执行读操作不需密码。
//注意：杀死操作必须有密码才能进行，密码为00000000视为无密码。
//(2)	memBank = 1时， 写EPC区，写入数据起始地址+长度不能超过epc区长度，一般起始地址从4字节开始，长度为12个字节。
//(3)	memBank = 3时， 写user区，写入数据起始地址+长度不能超过user区长度， user区最大510字节。

//ISO18000-6-C锁参数
typedef struct 
{
	int antennaID;       //天线号
	int killBankOp;	     //杀死密码区域锁定操作。
	int accessBankOp;      //操作密码区域锁定操作。
	int epcBankOp;        // EPC区锁定操作。
	int tidBankOp;        // TID区锁定操作。
	int userBankOp;       // USER区锁定操作。
	unsigned char accessPassword[4];    //操作密码，4个字节
	ISO_6C_TagID  tagID;         //标签ID
} ISO_6C_TagLock;
//注意：killBankOp、accessBankOp、epcBankOp、tidBankOp和userBankOp的
//取值范围是[0,4]，其含义分别是：0：不操作；1：锁定；2：永久锁定3：永久解锁；4：解锁。

//ISO18000-6-C杀死参数
typedef struct 
{
	int antennaID;	
	unsigned char killPassword[4];    //杀死密码，4个字节
	ISO_6C_TagID  tagID;         //标签ID
} ISO_6C_TagKill;

//ISO18000-6-B读参数
typedef struct
{
	int  tagDataLen; 			    //默认8，目前不可改变
	unsigned char aucTagData[8];  
} ISO_6B_TagID;

typedef struct 
{
	int antennaID;      			  //天线号	
	int  offset;       //开始字节地址
	int  length;      //长度
	ISO_6B_TagID  tagID;
} ISO_6B_TagRead;
//注意：offset取值范围是[8,223]，length的取值范围是[1,224 - offset]。


//ISO18000-6-B写参数
typedef struct 
{
	int antennaID;     		       //天线号
	int offset;         //开始地址
	int length;         //长度
	unsigned char data[256];           //数据内容，进制字符串，不能为奇数
	ISO_6B_TagID  tagID;
}ISO_6B_TagWrite;
//注意：offset取值范围是[8，223]，写长度（多少个字节）的取值范围是[1 , 224 - offset]。

//ISO18000-6-B锁参数
typedef struct 
{
	int antennaID;      		       //天线号	
	int offset;        //开始地址
	int length;       //锁长度 
	ISO_6B_TagID  tagID;
}ISO_6B_TagLock;
//注意：offset取值范围是[8，223]，length的取值范围是[1, 224 - offset]。

//ISO18000-6-B锁结果
typedef struct 
{	
	unsigned char  lockData[256]; //0表示锁定成功，其他表示锁定失败
} LockResult;

//ISO18000-6-B锁查询参数
typedef struct 
{
	int antennaID;					//天线号
	int  offset;       	//开始地址
	int  length;	//锁查询长度
	ISO_6B_TagID  tagID;
} ISO_6B_TagQueryLock;
//注意：offset取值范围是[8，223]，length的取值范围是[1, 224 - offset]。

//ISO18000-6-B锁查询结果
typedef struct 
{	
	unsigned char  queryLockData[256]; //0：未锁定，1：锁定，其它：查询错误
} QueryLockResult;

//Alien标签块锁操作参数
typedef struct 
{	int antennaID;     		     //天线号
unsigned char  ReadLock[8];  //0：读锁定；1：读解锁
unsigned char  WriteLock[8]; //0：写解锁；1：写保护；2：不操作
unsigned char  accessPassword[4]; //操作密码，4个字节
ISO_6C_TagID  tagID;            //标签ID
} Alien_TagLock;



//读写参数（变长）
typedef struct 
{		
	unsigned short  protocolType;             //协议类型,0-6C,1-6B,2-Impinj
	unsigned short  RWOpNum;              //下发的读写操作的总数量
	unsigned short  tagMaskMsgLen;              //掩码信息长度
	unsigned short  totalLenOfRWOpInf;              //标签操作总长度
	unsigned char accessPassword[4];    //操作密码，4个字节	 
	unsigned char*   maskAndCmdBuffer;	  //缓存区内容
} TagReadWrite;
//注：缓存区内容，TagMaskInf+RWOpInf+RWOpInf…，RWOpInf最大八个


typedef struct 
{
	int RWNum;  
	TagReadWrite tagReadWrite[3]; 
}Period_Read_Write;

//循环读写结果
typedef struct 
{	
	short  tagNum;         //标签数量，当为0时，表明没有清点到标签
	short  totalLenOfSingleRwTagInfo;       //返回所有标签的SingleRwTagInfo总长度
	unsigned char * SingleRwTagInfoBuffer;//返回单个标签读写信息的结果叠加
		//缓存内容：SingleRwTagInfo+SingleRwTagInfo+…
} RWResult;

typedef struct 
{	
	short  tagNum;         //标签数量，当为0时，表明没有清点到标签
	short  totalLenOfSingleRwTagInfo;       //返回所有标签的SingleRwTagInfo总长度
	unsigned char SingleRwTagInfoBuffer[PERIOD_RW_RESULT_BUFFER];//返回单个标签读写信息的结果叠加
	//缓存内容：SingleRwTagInfo+SingleRwTagInfo+…
} RWResultTmp;



//Impinj公私有转换参数
typedef struct 
{	
	int	antennaID;      		       //天线号	
	int dataProfile;				 //公私有转化方向，1-私转公，0-公转私，
	int persistence;       		 //公私有转换性质，0-临时转换，1-永久转换
	unsigned char accessPassword[4];    //操作密码，4个字节
	ISO_6C_TagID  tagID;         //标签ID
} TagMode;



//Impinj公私有转换查询参数
typedef struct 
{	
	int antennaID;      		       //天线号	
	unsigned char accessPassword[4];    //操作密码，4个字节
	ISO_6C_TagID  tagID;         //标签ID
} TagModeQuery;




/**************************************************************************
 *                         全局变量声明                                    *
 **************************************************************************/


/**************************************************************************
 *                        全局函数原型                                     *
 **************************************************************************/
extern int __stdcall Connect(IN const char *dev, OUT int *fd);/*  设置本单板复位  */
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


