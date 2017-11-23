#ifndef _include_h
#define _include_h

#define _DEBUG_REALTIME
//#define _DEBUG_RFID_
//#define _DEBUG_LOC2SER_

//#define _RFID_DEBUG_
#define _LPR_
//#define _LPR_XINLUTONG_
//#define _LPR_HIKVISION_




#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <time.h>
#include <signal.h>
#include <dlfcn.h>
#include <dirent.h>
#include </usr/include/net/if.h>
#include <net/route.h>

#include <unistd.h>
#include <sys/reboot.h>

#include "tcpsocket.h"
#include "crc32.h"

#include "sysconfig.h"
#include "realtime.h"
#include "gpio_usr.h"


#include "sqlite3.h"
#include "sqlite3_usr.h"

#include "FIFO_l2s.h"
#include "local2server.h"
#include "server2local.h"

#include "rfid_exist_duplist.h"
#include "rfid_filter_duplist.h"
#include "rfid_identity_duplist.h"

#include "msg.h"		//_by_dpj_2014-11-25

#include "zxris_rid6700.h"
#include "lpr_hik.h"
#include "libxm6c.h"
#include "encrypt.h"
#include "gpio_jjh.h"

#include "uhfshell_usr.h"
#include "zlog_usr.h"

#define gettid() syscall(__NR_gettid)

#define RECEOT 5
#define SENDOT 1
#define RECEOTC 5 //recevie over time count
#define L2SHBPTIME 5

#define MAXIMAGE 600		/* 图片存储数量最大上限值*/


#endif // lpr



