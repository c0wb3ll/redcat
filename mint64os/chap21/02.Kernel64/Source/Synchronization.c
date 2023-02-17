#include "Synchronization.h"
#include "Utility.h"
#include "Task.h"

// 시스템 전역에서 사용하는 데이터를 위한 잠금 함수
BOOL kLockForSystemData( void ) {

    return kSetInterruptFlag( FALSE );

}

// 잠금 해제 함수
void kUnlockForSystemData( BOOL bInterruptFlag ) {

    kSetInterruptFlag( bInterruptFlag );

}

// 뮤텍스 초기화
void kInitializeMutex( MUTEX* pstMutex ) {

    pstMutex->bLockFlag = FALSE;
    pstMutex->dwLockCount = 0;
    pstMutex->qwTaskID = TASK_INVALIDID;

}

// 태스크 사이에서 사용하는 데이터를 위한 잠금 함수
void kLock( MUTEX* pstMutex ) {

    if( kTestAndSet( &( pstMutex->bLockFlag ), 0, 1 ) == FALSE ) {

        if( pstMutex->qwTaskID == kGetRunningTask()->stLink.qwID ) {

            pstMutex->dwLockCount++;
            
            return ;

        }

        while( kTestAndSet( &( pstMutex->bLockFlag ), 0, 1 ) == FALSE ) {

            kSchedule();

        }

    }

    pstMutex->dwLockCount = 1;
    pstMutex->qwTaskID = kGetRunningTask()->stLink.qwID;

}

// 잠금 해제 함수 2
void kUnlock( MUTEX* pstMutex ) {

    if( ( pstMutex->bLockFlag == FALSE ) || ( pstMutex->qwTaskID != kGetRunningTask()->stLink.qwID ) ) {

        return ;

    }

    if( pstMutex->dwLockCount > 1 ) {

        pstMutex->dwLockCount--;

        return ;

    }

    pstMutex->qwTaskID = TASK_INVALIDID;
    pstMutex->dwLockCount = 0;
    pstMutex->bLockFlag = FALSE;

}