
#ifndef EERTOS_H_
#define EERTOS_H_

#include "HAL.h"
#include "EERTOSHAL.h"

extern void InitRTOS(void);
extern void Idle(void);

typedef void (*TPTR)(void);

extern void SetTask(TPTR TS);
extern void SetTimerTask(TPTR TS, u16 NewTime);

extern void TaskManager(void);
extern void TimerService(void);

#define TaskSetOk			 'A'
#define TaskQueueOverflow	 'B'
#define TimerUpdated		 'C'
#define TimerSetOk			 'D'
#define TimerOverflow		 'E'


#endif /* EERTOS_H_ */