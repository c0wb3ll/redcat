#ifndef __SYNCHRONIZATION_H__
#define __SYNCHRONIZATION_H__

#include "Types.h"

#pragma pack( push, 1 )

// 뮤텍스 자료구조
typedef struct kMutexStruct {

    volatile QWORD qwTaskID;
    volatile DWORD dwLockCount;

    volatile BOOL bLockFlag;

    BYTE vbPadding[ 3 ];

} MUTEX;

// 스핀락 자료구조
typedef struct kSpinLockStruct {

    volatile DWORD dwLockCount;
    volatile BYTE bAPICID;

    volatile BOOL bLockFlag;

    volatile BOOL bInterruptFlag;

    BYTE vbPadding[ 1 ];

} SPINLOCK;

#pragma pack( pop )

// 함수
#if 0
BOOL kLockForSystemData( void );
void kUnlockForSystemData( BOOL bInterruptFlag );
#endif

void kInitializeSpinLock( SPINLOCK* pstSpinLock );
void kLockForSpinLock( SPINLOCK* pstSpinLock );
void kUnlockForSpinLock( SPINLOCK* pstSpinLock );

void kInitializeMutex( MUTEX* pstMutex );
void kLock( MUTEX* pstMutex );
void kUnlock( MUTEX* pstMutex );

#endif /*__SYNCHRONIZATION_H__*/