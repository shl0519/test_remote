#ifndef _gpio_usr_h
#define _gpio_usr_h

#include "typedef_usr.h"
#include <time.h>

extern const gponame GPO[];
extern const gpiname GPI[];

extern unsigned char const GPO_H[];
extern unsigned char const GPO_L[];
extern GPI_StatAndTime GPIStat[];
extern laneclass_type LaneClass[];


extern void WriteGPO_REG_MachUpCFG(gponame OutputCfg,bool level);
extern void ReadGPI_MachUpCFG(void);
extern void thread_gpio(void);
extern void VehicleDetector(void);
extern void VehicleDetectorComm(bool VDState);
extern int gpio_init(void);
extern void Process_GPIEdge(void);			//_by_dpj_2014-11-24
extern void WriteGPO_MachUpCFG();		//_by_dpj_2014-11-24

#endif 

