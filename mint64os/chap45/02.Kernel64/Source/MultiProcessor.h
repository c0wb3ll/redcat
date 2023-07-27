#ifndef __MULTIPROCESSOR_H__
#define __MULTIPROCESSOR_H__

#include "Types.h"

// MultiProcessor 관련 매크로
#define BOOTSTRAPPROCESSOR_FLAGADDRESS              0x7C09
#define MAXPROCESSORCOUNT                           16

// 함수
BOOL kStartUpApplicationProcessor( void );
BYTE kGetAPICID( void );
static BOOL kWakeUpApplicationProcessor( void );

#endif /*__MULTIPROCESSOR_H__*/