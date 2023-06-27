#include "Task.h"
#include "Descriptor.h"
#include "MultiProcessor.h"

// 스케줄러 관련 자료구조
static SCHEDULER gs_vstScheduler[ MAXPROCESSORCOUNT ];
static TCBPOOLMANAGER gs_stTCBPoolManager;

// 태스크 풀, 태스크 관련
// 태스크 풀 초기화
static void kInitializeTCBPool( void ) {

    int i;
    
    kMemSet( &( gs_stTCBPoolManager ), 0, sizeof( gs_stTCBPoolManager ) );

    gs_stTCBPoolManager.pstStartAddress = ( TCB* ) TASK_TCBPOOLADDRESS;
    kMemSet( TASK_TCBPOOLADDRESS, 0, sizeof( TCB ) * TASK_MAXCOUNT );

    for( i = 0; i < TASK_MAXCOUNT; i++ ) {

        gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;

    }

    gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
    gs_stTCBPoolManager.iAllocatedCount = 1;

    kInitializeSpinLock( &gs_stTCBPoolManager.stSpinLock );

}

// TCB 할당
static TCB* kAllocateTCB( void ) {

    TCB* pstEmptyTCB;
    int i;

    kLockForSpinLock( &gs_stTCBPoolManager.stSpinLock );

    if( gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount ) { 
        
        kUnlockForSpinLock( &gs_stTCBPoolManager.stSpinLock );
        return NULL; 
        
    }

    for( i = 0; i < gs_stTCBPoolManager.iMaxCount; i++ ) {

        if( ( gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID >> 32 ) == 0 ) {

            pstEmptyTCB = &( gs_stTCBPoolManager.pstStartAddress[ i ] );
            break;

        }

    }

    pstEmptyTCB->stLink.qwID = ( ( QWORD ) gs_stTCBPoolManager.iAllocatedCount << 32 ) | i;
    gs_stTCBPoolManager.iUseCount++;
    gs_stTCBPoolManager.iAllocatedCount++;
    if( gs_stTCBPoolManager.iAllocatedCount == 0 ) { 
        
        gs_stTCBPoolManager.iAllocatedCount = 1; 
        
    }

    kUnlockForSpinLock( &gs_stTCBPoolManager.stSpinLock );

    return pstEmptyTCB;

}

// TCB 해제
static void kFreeTCB( QWORD qwID ) {

    int i;

    i = GETTCBOFFSET( qwID );

    kMemSet( &( gs_stTCBPoolManager.pstStartAddress[ i ].stContext ), 0, sizeof( CONTEXT ) );
    
    kLockForSpinLock( &gs_stTCBPoolManager.stSpinLock );

    gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;

    gs_stTCBPoolManager.iUseCount--;

    kUnlockForSpinLock( &gs_stTCBPoolManager.stSpinLock );

}

// 태스크 생성
// 태스크 ID에 따라 스택 풀에서 스택 자동 할당
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, QWORD qwEntryPointAddress, BYTE bAffinity ) {

    TCB* pstTask, *pstProcess;
    void* pvStackAddress;
    BYTE bCurrentAPICID;

    bCurrentAPICID = kGetAPICID();  

    pstTask = kAllocateTCB();
    if( pstTask == NULL ) { 
        
        return NULL; 
        
    }

    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    pstProcess = kGetProcessByThread( kGetRunningTask( bCurrentAPICID ) );
    if( pstProcess == NULL ) {

        kFreeTCB( pstTask->stLink.qwID );
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        
        return NULL;

    }

    if( qwFlags & TASK_FLAGS_THREAD ) {

        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pstProcess->pvMemoryAddress;
        pstTask->qwMemorySize = pstProcess->qwMemorySize;

        kAddListToTail( &( pstProcess->stChildThreadList ), &( pstTask->stThreadLink ) );

    } else {

        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pvMemoryAddress;
        pstTask->qwMemorySize = qwMemorySize;

    }

    pstTask->stThreadLink.qwID = pstTask->stLink.qwID;

    kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    pvStackAddress = ( void* ) ( TASK_STACKPOOLADDRESS + ( TASK_STACKSIZE * GETTCBOFFSET( pstTask->stLink.qwID ) ) );

    kSetUpTask( pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE );

    kInitializeList( &( pstTask->stChildThreadList ) );

    pstTask->bFPUUsed = FALSE;

    pstTask->bAPICID = bCurrentAPICID;

    pstTask->bAffinity = bAffinity;

    kAddTaskToSchedulerWithLoadBalancing( pstTask );

    return pstTask;

}

// 파라미터 이용해서 TCB 설정
static void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize ) {

    kMemSet( pstTCB->stContext.vqRegister, 0, sizeof( pstTCB->stContext.vqRegister ) );

    pstTCB->stContext.vqRegister[ TASK_RSPOFFSET ] = ( QWORD ) pvStackAddress + qwStackSize - 8;
    pstTCB->stContext.vqRegister[ TASK_RBPOFFSET ] = ( QWORD ) pvStackAddress + qwStackSize - 8;

    *( QWORD * ) ( ( QWORD ) pvStackAddress + qwStackSize - 8 ) = ( QWORD ) kExitTask;

    pstTCB->stContext.vqRegister[ TASK_CSOFFSET ] = GDT_KERNELCODESEGMENT;
    pstTCB->stContext.vqRegister[ TASK_DSOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_ESOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_FSOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_GSOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_SSOFFSET ] = GDT_KERNELDATASEGMENT;

    pstTCB->stContext.vqRegister[ TASK_RIPOFFSET ] = qwEntryPointAddress;

    pstTCB->stContext.vqRegister[ TASK_RFLAGSOFFSET ] |= 0x0200;
    
    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;

}

// 스케줄러 관련
// 스케줄러 초기화
// 스케줄러를 초기화하는데 필요한 TCB 풀과 init 태스크도 같이 초기화
void kInitializeScheduler( void ) {

    int i;
    int j;
    BYTE bCurrentAPICID;
    TCB* pstTask;

    bCurrentAPICID = kGetAPICID();

    if( bCurrentAPICID == 0 ) {

        kInitializeTCBPool();

        for( j = 0; j < MAXPROCESSORCOUNT; j++ ) {

            for( i = 0; i < TASK_MAXREADYLISTCOUNT; i++ ) {

                kInitializeList( &( gs_vstScheduler[ j ].vstReadyList[ i ] ) );
                gs_vstScheduler[ j ].viExecuteCount[ i ] = 0;

            }

            kInitializeList( &( gs_vstScheduler[ j ].stWaitList ) );

            kInitializeSpinLock( &( gs_vstScheduler[ j ].stSpinLock ) );

        }

    }

    pstTask = kAllocateTCB();
    gs_vstScheduler[ bCurrentAPICID ].pstRunningTask = pstTask;

    pstTask->bAPICID = bCurrentAPICID;
    pstTask->bAffinity = bCurrentAPICID;

    if( bCurrentAPICID == 0 ) {

        pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;

    } else {

        pstTask->qwFlags = TASK_FLAGS_LOWEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE;

    }

    pstTask->qwParentProcessID = pstTask->stLink.qwID;
    pstTask->pvMemoryAddress = ( void* ) 0x100000;
    pstTask->qwMemorySize = 0x500000;
    pstTask->pvStackAddress = ( void* ) 0x600000;
    pstTask->qwStackSize = 0x100000;

    gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask = 0;
    gs_vstScheduler[ bCurrentAPICID ].qwProcessorLoad = 0;

    gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID = TASK_INVALIDID;

}

// 현재 수행중인 태스크 설정
void kSetRunningTask( BYTE bAPICID, TCB* pstTask ) {

    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    gs_vstScheduler[ bAPICID ].pstRunningTask = pstTask;

    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

}

// 현재 수행 중인 태스크 반환
TCB* kGetRunningTask( BYTE bAPICID ) {

    TCB* pstRunningTask;

    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    pstRunningTask = gs_vstScheduler[ bAPICID ].pstRunningTask;

    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    return pstRunningTask;

}

// 태스크 리스트에서 다음으로 실행할 태스크를 얻음
static TCB* kGetNextTaskToRun( BYTE bAPICID ) {

    TCB* pstTarget = NULL;
    int iTaskCount, i, j;

    for( j = 0; j < 2 ; j++ ) {

        for( i = 0; i < TASK_MAXREADYLISTCOUNT; i++ ) {

            iTaskCount = kGetListCount( &( gs_vstScheduler[ bAPICID ].vstReadyList[ i ] ) );

            if( gs_vstScheduler[ bAPICID ].viExecuteCount[ i ] < iTaskCount ) {

                pstTarget = ( TCB* ) kRemoveListFromHeader( &( gs_vstScheduler[ bAPICID ].vstReadyList[ i ] ) );
                gs_vstScheduler[ bAPICID ].viExecuteCount[ i ]++;
                break;

            } else {

                gs_vstScheduler[ bAPICID ].viExecuteCount[ i ] = 0;

            }

        }

        if( pstTarget != NULL ) { 
            
            break; 
            
        }

    }

    return pstTarget;

}

// 태스크를 스케줄러의 준비 리스트에 삽입
static BOOL kAddTaskToReadyList( BYTE bAPICID, TCB* pstTask ) {

    BYTE bPriority;

    bPriority = GETPRIORITY( pstTask->qwFlags );
    if( bPriority == TASK_FLAGS_WAIT ) { 

        kAddListToTail( &( gs_vstScheduler[ bAPICID ].stWaitList ), pstTask );
        
        return TRUE;

     } else if ( bPriority >= TASK_MAXREADYLISTCOUNT ) { 

        return FALSE;

     }

    kAddListToTail( &( gs_vstScheduler[ bAPICID ].vstReadyList[ bPriority ] ), pstTask );
    
    return TRUE;

}

// 준비 큐에서 태스크를 제거
static TCB* kRemoveTaskFromReadyList( BYTE bAPICID, QWORD qwTaskID ) {

    TCB* pstTarget;
    BYTE bPriority;

    if( GETTCBOFFSET( qwTaskID ) >= TASK_MAXCOUNT ) { 
        
        return NULL; 
        
    }

    pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
    if( pstTarget->stLink.qwID != qwTaskID ) { 
        
        return NULL; 
        
    }

    bPriority = GETPRIORITY( pstTarget->qwFlags );
    if( bPriority >= TASK_MAXREADYLISTCOUNT ) {

        return NULL;

    }

    pstTarget = kRemoveList( &( gs_vstScheduler[ bAPICID ].vstReadyList[ bPriority ] ), qwTaskID );

    return pstTarget;

}

// 태스크가 포함된 스케줄러의 ID를 반환 후 해당 스케줄러의 스핀락을 잠금
static BOOL kFindSchedulerOfTaskAndLock( QWORD qwTaskID, BYTE* pbAPICID ) {

    TCB* pstTarget;
    BYTE bAPICID;

    while( 1 ) {

        pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
        if( ( pstTarget == NULL ) || ( pstTarget->stLink.qwID != qwTaskID ) ) {

            return FALSE;

        }

        bAPICID = pstTarget->bAPICID;

        kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

        pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
        if( pstTarget->bAPICID == bAPICID ) {

            break;

        }

        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    }

    *pbAPICID = bAPICID;

    return TRUE;

}

// 태스크의 우선순위를 변경함
BOOL kChangePriority( QWORD qwTaskID, BYTE bPriority ) {

    TCB* pstTarget;
    BYTE bAPICID;

    if( bPriority > TASK_MAXREADYLISTCOUNT ) { 
        
        return FALSE; 
        
    }

    if( kFindSchedulerOfTaskAndLock( qwTaskID, &bAPICID ) == FALSE ) {

        return FALSE;

    }

    pstTarget = gs_vstScheduler[ bAPICID ].pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID ) {

        SETPRIORITY( pstTarget->qwFlags, bPriority );

    } else {

        pstTarget = kRemoveTaskFromReadyList( bAPICID, qwTaskID );
        if( pstTarget == NULL ) {

            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL ) {

                SETPRIORITY( pstTarget->qwFlags, bPriority );

            }

        } else {

            SETPRIORITY( pstTarget->qwFlags, bPriority );
            kAddTaskToReadyList( bAPICID, pstTarget );

        }

    }

    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    return TRUE;

}

// 다른 태스크를 찾아서 전환
// 인터럽트나 예외시에는 호출 X
BOOL kSchedule( void ) {

    TCB* pstRunningTask, * pstNextTask;
    BOOL bPreviousInterrupt;
    BYTE bCurrentAPICID;

    bPreviousInterrupt = kSetInterruptFlag( FALSE );

    bCurrentAPICID = kGetAPICID();

    if( kGetReadyTaskCount( bCurrentAPICID ) < 1 ) { 
        
        kSetInterruptFlag( bPreviousInterrupt );
        return FALSE; 
        
    }

    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    pstNextTask = kGetNextTaskToRun( bCurrentAPICID );
    if( pstNextTask == NULL ) {

        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        kSetInterruptFlag( bPreviousInterrupt );
        return FALSE;

    }

    pstRunningTask = gs_vstScheduler[ bCurrentAPICID ].pstRunningTask;
    gs_vstScheduler[ bCurrentAPICID ].pstRunningTask = pstNextTask;

    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE ) {

        gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME - gs_vstScheduler[ bCurrentAPICID ].iProcessorTime;

    }

    if( gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID != pstNextTask->stLink.qwID ) {

        kSetTS();

    } else {

        kClearTS();

    }

    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK ) {

        kAddListToTail( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ), pstRunningTask );
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        kSwitchContext( NULL, &( pstNextTask->stContext ) );

    } else {

        kAddTaskToReadyList( bCurrentAPICID, pstRunningTask );
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        kSwitchContext( &( pstRunningTask->stContext ), &( pstNextTask->stContext ) );

    }

    gs_vstScheduler[ bCurrentAPICID ].iProcessorTime = TASK_PROCESSORTIME;
    kSetInterruptFlag( bPreviousInterrupt );

    return FALSE;

}

// 인터럽트가 발생했을 대, 다른 태스크를 찾아 전환
// 인터럽트나 예외 발생 시 호출
BOOL kScheduleInInterrupt( void ) {

    TCB* pstRunningTask, * pstNextTask;
    char* pcContextAddress;
    BYTE bCurrentAPICID;
    QWORD qwISTStartAddress;

    bCurrentAPICID = kGetAPICID();

    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    pstNextTask = kGetNextTaskToRun( bCurrentAPICID );
    if( pstNextTask == NULL ) { 
        
        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
        
        return FALSE; 
    
    }

    qwISTStartAddress = IST_STARTADDRESS + IST_SIZE - ( IST_SIZE / MAXPROCESSORCOUNT * bCurrentAPICID );
    pcContextAddress = ( char* ) qwISTStartAddress - sizeof( CONTEXT );

    pstRunningTask = gs_vstScheduler[ bCurrentAPICID ].pstRunningTask;
    gs_vstScheduler[ bCurrentAPICID ].pstRunningTask = pstNextTask;

    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE ) {

        gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;

    }

    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK ) {

        kAddListToTail( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ), pstRunningTask );

    } else {

        kMemCpy( &( pstRunningTask->stContext ), pcContextAddress, sizeof( CONTEXT ) );

    }

    if( gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID != pstNextTask->stLink.qwID ) {

        kSetTS();

    } else {

        kClearTS();

    }

    kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    kMemCpy( pcContextAddress, &( pstNextTask->stContext ), sizeof( CONTEXT ) );

    if( ( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK ) != TASK_FLAGS_ENDTASK ) {

        kAddTaskToSchedulerWithLoadBalancing( pstRunningTask );

    }

    gs_vstScheduler[ bCurrentAPICID ].iProcessorTime = TASK_PROCESSORTIME;

    return TRUE;

}

// 프로세서를 사용할 수 있는 시간 줄임
void kDecreaseProcessorTime( BYTE bAPICID ) {

    gs_vstScheduler[ bAPICID ].iProcessorTime--;

}

// 프로세서를 사용할 수 있는 시간 다 소요되었는지 여부 반환
BOOL kIsProcessorTimeExpired( BYTE bAPICID ) {

    if( gs_vstScheduler[ bAPICID ].iProcessorTime <= 0 ) { 
        
        return TRUE; 
        
    }

    return FALSE;

}

// 태스크를 종료
BOOL kEndTask( QWORD qwTaskID ) {

    TCB* pstTarget;
    BYTE bPriority;
    BYTE bAPICID;

    if( kFindSchedulerOfTaskAndLock( qwTaskID, &bAPICID ) == FALSE ) {

        return FALSE;

    }

    pstTarget = gs_vstScheduler[ bAPICID ].pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID ) {

        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );

        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

        if( kGetAPICID() == bAPICID ) {

            kSchedule();

            while( 1 ) {

                ;

            }

        }

        return TRUE;

    }

    pstTarget = kRemoveTaskFromReadyList( bAPICID, qwTaskID );
    if( pstTarget == NULL ) {

        pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
        if( pstTarget != NULL ) {

            pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
            SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );

        }

        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

        return TRUE;

    }

    pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
    SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
    kAddListToTail( &( gs_vstScheduler[ bAPICID ].stWaitList ), pstTarget );

    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    return TRUE;

}

// 태스크가 스스로 종료
void kExitTask( void ) {

    kEndTask( gs_vstScheduler[ kGetAPICID() ].pstRunningTask->stLink.qwID );

}

// 준비 큐에 있는 모든 태스크의 수를 반환
int kGetReadyTaskCount( BYTE bAPICID ) {

    int iTotalCount = 0;
    int i;

    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    for( i = 0; i < TASK_MAXREADYLISTCOUNT ; i++ ) {

        iTotalCount += kGetListCount( &( gs_vstScheduler[ bAPICID ].vstReadyList[ i ] ) );

    }

    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    return iTotalCount;

}

int kGetTaskCount( BYTE bAPICID ) {

    int iTotalCount;

    iTotalCount = kGetReadyTaskCount( bAPICID );

    kLockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    iTotalCount += kGetListCount( &( gs_vstScheduler[ bAPICID ].stWaitList ) ) + 1;

    kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    return iTotalCount;

}

// TCB 풀에서 해당 오프셋의 TCB를 반환
TCB* kGetTCBInTCBPool( int iOffset ) {

    if( ( iOffset < -1 ) && ( iOffset > TASK_MAXCOUNT ) ) { 
        
        return NULL; 
        
    }

    return &( gs_stTCBPoolManager.pstStartAddress[ iOffset ] );

}

BOOL kIsTaskExist( QWORD qwID ) {

    TCB* pstTCB;

    pstTCB = kGetTCBInTCBPool( GETTCBOFFSET( qwID ) );
    if( ( pstTCB == NULL ) || ( pstTCB->stLink.qwID != qwID ) ) { 
        
        return FALSE; 
        
    }

    return TRUE;
    
}

QWORD kGetProcessorLoad( BYTE bAPICID ) {

    return gs_vstScheduler[ bAPICID ].qwProcessorLoad;

}

// 스레드가 소속된 프로세서르르 반환
static TCB* kGetProcessByThread( TCB* pstThread ) {

    TCB* pstProcess;

    if( pstThread->qwFlags & TASK_FLAGS_PROCESS ) {

        return pstThread;

    }

    pstProcess = kGetTCBInTCBPool( GETTCBOFFSET( pstThread->qwParentProcessID ) );

    if( ( pstProcess == NULL ) || ( pstProcess->stLink.qwID != pstThread->qwParentProcessID ) ) {

        return NULL;

    }

    return pstProcess;

}

// 각 스케줄러의 태스크 수를 이용하여 적절한 스케줄러에 태스크 추가
void kAddTaskToSchedulerWithLoadBalancing( TCB* pstTask ) {

    BYTE bCurrentAPICID;
    BYTE bTargetAPICID;

    bCurrentAPICID = pstTask->bAPICID;

    if( ( gs_vstScheduler[ bCurrentAPICID ].bUseLoadBalancing == TRUE ) && ( pstTask->bAffinity == TASK_LOADBALANCINGID ) ) {

        bTargetAPICID = kFindSchedulerOfMinimumTaskCount( pstTask );

    } else if( ( pstTask->bAffinity != bCurrentAPICID ) && ( pstTask->bAffinity != TASK_LOADBALANCINGID ) ) {

        bTargetAPICID = pstTask->bAffinity;

    } else {

        bTargetAPICID = bCurrentAPICID;

    }

    kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    if( ( bCurrentAPICID != bTargetAPICID ) && ( pstTask->stLink.qwID == gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID ) ) {

        kClearTS();
        kSaveFPUContext( pstTask->vqwFPUContext );
        gs_vstScheduler[ bCurrentAPICID ].qwLastFPUUsedTaskID = TASK_INVALIDID;

    }

    kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

    kLockForSpinLock( &( gs_vstScheduler[ bTargetAPICID ].stSpinLock ) );

    pstTask->bAPICID = bTargetAPICID;
    kAddTaskToReadyList( bTargetAPICID, pstTask );

    kUnlockForSpinLock( &( gs_vstScheduler[ bTargetAPICID ].stSpinLock ) );

}

// 태스크를 추가할 스케줄러의 id 반환
static BYTE kFindSchedulerOfMinimumTaskCount( const TCB* pstTask ) {

    BYTE bPriority;
    BYTE i;
    int iCurrentTaskCount;
    int iMinTaskCount;
    BYTE bMinCoreIndex;
    int iTempTaskCount;
    int iProcessorCount;

    iProcessorCount = kGetProcessorCount();

    if( iProcessorCount == 1 ) {

        return pstTask->bAPICID;

    }

    bPriority = GETPRIORITY( pstTask->qwFlags );

    iCurrentTaskCount = kGetListCount( &( gs_vstScheduler[ pstTask->bAPICID ].vstReadyList[ bPriority ] ) );

    iMinTaskCount = TASK_MAXCOUNT;
    bMinCoreIndex = pstTask->bAPICID;
    for( i = 0; i < iProcessorCount; i++ ) {

        if( i == pstTask->bAPICID ) {

            continue;

        }

        iTempTaskCount = kGetListCount( &( gs_vstScheduler[ i ].vstReadyList[ bPriority ] ) );

        if( ( iTempTaskCount + 2 <= iCurrentTaskCount ) && ( iTempTaskCount < iMinTaskCount ) ) {

            bMinCoreIndex = i;
            iMinTaskCount = iTempTaskCount;

        }

    }

    return bMinCoreIndex;

}

// 파라미터로 전달된 코어에 태스크 부하 분산 기능 사용 여부 설정
BYTE kSetTaskLoadBalancing( BYTE bAPICID, BOOL bUseLoadBalancing ) {

    gs_vstScheduler[ bAPICID ].bUseLoadBalancing = bUseLoadBalancing;

}

// 프로세서 친화도 변경
BOOL kChangeProcessorAffinity( QWORD qwTaskID, BYTE bAffinity ) {

    TCB* pstTarget;
    BYTE bAPICID;

    if( kFindSchedulerOfTaskAndLock( qwTaskID, &bAPICID ) == FALSE ) {

        return FALSE;

    }

    pstTarget = gs_vstScheduler[ bAPICID ].pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID ) {

        pstTarget->bAffinity = bAffinity;

        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

    } else {

        pstTarget = kRemoveTaskFromReadyList( bAPICID, qwTaskID );
        if( pstTarget == NULL ) {

            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL ) {

                pstTarget->bAffinity = bAffinity;

            }

        } else {

            pstTarget->bAffinity = bAffinity;

        }

        kUnlockForSpinLock( &( gs_vstScheduler[ bAPICID ].stSpinLock ) );

        kAddTaskToSchedulerWithLoadBalancing( pstTarget );

    }

    return TRUE;

}

// 유휴 태스크
// 대기 큐에 삭제 대기 중인 태스크를 정리
void kIdleTask( void ) {

    TCB* pstTask, * pstChildThread, * pstProcess;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;
    QWORD qwTaskID, qwChildThreadID;
    int i, iCount;
    void* pstThreadLink;
    BYTE bCurrentAPICID;
    BYTE bProcessAPICID;

    bCurrentAPICID = kGetAPICID();

    qwLastSpendTickInIdleTask = gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask;
    qwLastMeasureTickCount = kGetTickCount();

    while( 1 ) {

        qwCurrentMeasureTickCount = kGetTickCount();
        qwCurrentSpendTickInIdleTask = gs_vstScheduler[ bCurrentAPICID ].qwSpendProcessorTimeInIdleTask;

        if( qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0 ) {

            gs_vstScheduler[ bCurrentAPICID ].qwProcessorLoad = 0;

        } else {

            gs_vstScheduler[ bCurrentAPICID ].qwProcessorLoad = 100 - ( qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask ) * 100 / ( qwCurrentMeasureTickCount - qwLastMeasureTickCount );

        }

        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        kHaltProcessorByLoad( bCurrentAPICID );

        if( kGetListCount( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ) ) > 0 ) {

            while( 1 ) {

                kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                pstTask = kRemoveListFromHeader( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ) );
                kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                
                if( pstTask == NULL ) {     

                    break; 
                    
                }

                if( pstTask->qwFlags & TASK_FLAGS_PROCESS ) {

                    iCount = kGetListCount( &( pstTask->stChildThreadList ) );
                    for( i = 0; i < iCount; i++ ) {

                        kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

                        pstThreadLink = ( TCB* ) kRemoveListFromHeader( &( pstTask->stChildThreadList ) );
                        if( pstThreadLink == NULL ) { 
                            
                            kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                            break; 
                            
                        }

                        pstChildThread = GETTCBFROMTHREADLINK( pstThreadLink );

                        kAddListToTail( &( pstTask->stChildThreadList ), &( pstChildThread->stThreadLink ) );
                        qwChildThreadID = pstChildThread->stLink.qwID;
                        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );

                        kEndTask( qwChildThreadID );

                    }

                    if( kGetListCount( &( pstTask->stChildThreadList ) ) > 0 ) {
                        
                        kLockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                        kAddListToTail( &( gs_vstScheduler[ bCurrentAPICID ].stWaitList ), pstTask );
                        kUnlockForSpinLock( &( gs_vstScheduler[ bCurrentAPICID ].stSpinLock ) );
                        continue;

                    } else {

                        // TODO: 추후 코드 삽입

                    }

                } else if( pstTask->qwFlags & TASK_FLAGS_THREAD ) {

                    pstProcess = kGetProcessByThread( pstTask );
                    if( pstProcess != NULL ) {

                        if( kFindSchedulerOfTaskAndLock( pstProcess->stLink.qwID, &bProcessAPICID ) == TRUE ) {

                            kRemoveList( &( pstProcess->stChildThreadList ), pstTask->stLink.qwID );
                            kUnlockForSpinLock( &( gs_vstScheduler[ bProcessAPICID ].stSpinLock ) );

                        }

                    }

                }

                qwTaskID = pstTask->stLink.qwID;
                kFreeTCB( qwTaskID );
                kPrintf( "IDLE: Task ID[0x%q] is completely ended.\n", qwTaskID );

            }

        }

        kSchedule();

    }

}

void kHaltProcessorByLoad( BYTE bAPICID ) {

    if( gs_vstScheduler[ bAPICID ].qwProcessorLoad < 40 ) {

        kHlt();
        kHlt();
        kHlt();

    } else if( gs_vstScheduler[ bAPICID ].qwProcessorLoad < 80 ) {

        kHlt();
        kHlt();

    } else if( gs_vstScheduler[ bAPICID ].qwProcessorLoad < 95 ) {

        kHlt();

    }

}

// FPU 관련
QWORD kGetLastFPUUsedTaskID( BYTE bAPICID ) {

    return gs_vstScheduler[ bAPICID ].qwLastFPUUsedTaskID;

}

void kSetLastFPUUsedTaskID( BYTE bAPICID, QWORD qwTaskID ) {

    gs_vstScheduler[ bAPICID ].qwLastFPUUsedTaskID = qwTaskID;

}