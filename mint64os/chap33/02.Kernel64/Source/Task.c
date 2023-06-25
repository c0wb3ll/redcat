#include "Task.h"
#include "Descriptor.h"

// 스케줄러 관련 자료구조
static SCHEDULER gs_stScheduler;
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

}

// TCB 할당
static TCB* kAllocateTCB( void ) {

    TCB* pstEmptyTCB;
    int i;

    if( gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount ) { 
        
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

    return pstEmptyTCB;

}

// TCB 해제
static void kFreeTCB( QWORD qwID ) {

    int i;

    i = GETTCBOFFSET( qwID );

    kMemSet( &( gs_stTCBPoolManager.pstStartAddress[ i ].stContext ), 0, sizeof( CONTEXT ) );
    gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;

    gs_stTCBPoolManager.iUseCount--;

}

// 태스크 생성
// 태스크 ID에 따라 스택 풀에서 스택 자동 할당
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize,  QWORD qwEntryPointAddress ) {

    TCB* pstTask, * pstProcess;
    void* pvStackAddress;

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );    

    pstTask = kAllocateTCB();
    if( pstTask == NULL ) { 
        
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        return NULL; 
        
    }

    pstProcess = kGetProcessByThread( kGetRunningTask() );
    if( pstProcess == NULL ) {

        kFreeTCB( pstTask->stLink.qwID );
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        
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

    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    pvStackAddress = ( void* ) ( TASK_STACKPOOLADDRESS + ( TASK_STACKSIZE * GETTCBOFFSET( pstTask->stLink.qwID ) ) );

    kSetUpTask( pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE );

    kInitializeList( &( pstTask->stChildThreadList ) );

    pstTask->bFPUUsed = FALSE;

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    kAddTaskToReadyList( pstTask );

    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

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
    TCB* pstTask;

    kInitializeTCBPool();

    for( i = 0; i < TASK_MAXREADYLISTCOUNT; i++ ) {

        kInitializeList( &( gs_stScheduler.vstReadyList[ i ] ) );
        gs_stScheduler.viExecuteCount[ i ] = 0;

    }
    kInitializeList( &( gs_stScheduler.stWaitList ) );

    pstTask = kAllocateTCB();
    gs_stScheduler.pstRunningTask = pstTask;
    pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
    pstTask->qwParentProcessID = pstTask->stLink.qwID;
    pstTask->pvMemoryAddress = ( void* ) 0x100000;
    pstTask->qwMemorySize = 0x500000;
    pstTask->pvStackAddress = ( void* ) 0x600000;
    pstTask->qwStackSize = 0x100000;

    gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
    gs_stScheduler.qwProcessorLoad = 0;

    gs_stScheduler.qwLastFPUUsedTaskID = TASK_INVALIDID;

    kInitializeSpinLock( &( gs_stScheduler.stSpinLock ) );

}

// 현재 수행중인 태스크 설정
void kSetRunningTask( TCB* pstTask ) {

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    gs_stScheduler.pstRunningTask = pstTask;

    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

}

// 현재 수행 중인 태스크 반환
TCB* kGetRunningTask( void ) {

    TCB* pstRunningTask;

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    pstRunningTask = gs_stScheduler.pstRunningTask;

    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    return pstRunningTask;

}

// 태스크 리스트에서 다음으로 실행할 태스크를 얻음
static TCB* kGetNextTaskToRun( void ) {

    TCB* pstTarget = NULL;
    int iTaskCount, i, j;

    for( j = 0; j < 2 ; j++ ) {

        for( i = 0; i < TASK_MAXREADYLISTCOUNT; i++ ) {

            iTaskCount = kGetListCount( &( gs_stScheduler.vstReadyList[ i ] ) );

            if( gs_stScheduler.viExecuteCount[ i ] < iTaskCount ) {

                pstTarget = ( TCB* ) kRemoveListFromHeader( &( gs_stScheduler.vstReadyList[ i ] ) );
                gs_stScheduler.viExecuteCount[ i ]++;
                break;

            } else {

                gs_stScheduler.viExecuteCount[ i ] = 0;

            }

        }

        if( pstTarget != NULL ) { 
            
            break; 
            
        }

    }

    return pstTarget;

}

// 태스크를 스케줄러의 준비 리스트에 삽입
static BOOL kAddTaskToReadyList( TCB* pstTask ) {

    BYTE bPriority;

    bPriority = GETPRIORITY( pstTask->qwFlags );
    if( bPriority == TASK_FLAGS_WAIT ) { 

        kAddListToTail( &( gs_stScheduler.stWaitList ), pstTask );
        
        return TRUE;

     } else if ( bPriority >= TASK_MAXREADYLISTCOUNT ) { 

        return FALSE;

     }

    kAddListToTail( &( gs_stScheduler.vstReadyList[ bPriority ] ), pstTask );
    
    return TRUE;

}

// 준비 큐에서 태스크를 제거
static TCB* kRemoveTaskFromReadyList( QWORD qwTaskID ) {

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

    pstTarget = kRemoveList( &( gs_stScheduler.vstReadyList[ bPriority ] ), qwTaskID );

    return pstTarget;

}

// 태스크의 우선순위를 변경함
BOOL kChangePriority( QWORD qwTaskID, BYTE bPriority ) {

    TCB* pstTarget;

    if( bPriority > TASK_MAXREADYLISTCOUNT ) { 
        
        return FALSE; 
        
    }

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    pstTarget = gs_stScheduler.pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID ) {

        SETPRIORITY( pstTarget->qwFlags, bPriority );

    } else {

        pstTarget = kRemoveTaskFromReadyList( qwTaskID );
        if( pstTarget == NULL ) {

            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL ) {

                SETPRIORITY( pstTarget->qwFlags, bPriority );

            }

        } else {

            SETPRIORITY( pstTarget->qwFlags, bPriority );
            kAddTaskToReadyList( pstTarget );

        }

    }

    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    return TRUE;

}

// 다른 태스크를 찾아서 전환
// 인터럽트나 예외시에는 호출 X
void kSchedule( void ) {

    TCB* pstRunningTask, * pstNextTask;
    BOOL bPreviousInterrupt;

    if( kGetReadyTaskCount() < 1 ) { 
        
        return ; 
        
    }

    bPreviousInterrupt = kSetInterruptFlag( FALSE );

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL ) {

        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        kSetInterruptFlag( bPreviousInterrupt );
        return ;

    }

    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;

    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE ) {

        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;

    }

    if( gs_stScheduler.qwLastFPUUsedTaskID != pstNextTask->stLink.qwID ) {

        kSetTS();

    } else {

        kClearTS();

    }

    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK ) {

        kAddListToTail( &( gs_stScheduler.stWaitList ), pstRunningTask );
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        kSwitchContext( NULL, &( pstNextTask->stContext ) );

    } else {

        kAddTaskToReadyList( pstRunningTask );
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        kSwitchContext( &( pstRunningTask->stContext ), &( pstNextTask->stContext ) );

    }

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    kSetInterruptFlag( bPreviousInterrupt );

}

// 인터럽트가 발생했을 대, 다른 태스크를 찾아 전환
// 인터럽트나 예외 발생 시 호출
BOOL kScheduleInInterrupt( void ) {

    TCB* pstRunningTask, * pstNextTask;
    char* pcContextAddress;

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL ) { 
        
        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
        
        return FALSE; 
    
    }

    pcContextAddress = ( char* ) IST_STARTADDRESS + IST_SIZE - sizeof( CONTEXT );

    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;

    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE ) {

        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;

    }

    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK ) {

        kAddListToTail( &( gs_stScheduler.stWaitList ), pstRunningTask );

    } else {

        kMemCpy( &( pstRunningTask->stContext ), pcContextAddress, sizeof( CONTEXT ) );
        kAddTaskToReadyList( pstRunningTask );

    }

    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    if( gs_stScheduler.qwLastFPUUsedTaskID != pstNextTask->stLink.qwID ) {

        kSetTS();

    } else {

        kClearTS();

    }

    kMemCpy( pcContextAddress, &( pstNextTask->stContext ), sizeof( CONTEXT ) );

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    return TRUE;

}

// 프로세서를 사용할 수 있는 시간 줄임
void kDecreaseProcessorTime( void ) {

    if( gs_stScheduler.iProcessorTime > 0 ) {

        gs_stScheduler.iProcessorTime--;

    }

}

// 프로세서를 사용할 수 있는 시간 다 소요되었는지 여부 반환
BOOL kIsProcessorTimeExpired( void ) {

    if( gs_stScheduler.iProcessorTime <= 0 ) { 
        
        return TRUE; 
        
    }

    return FALSE;

}

// 태스크를 종료
BOOL kEndTask( QWORD qwTaskID ) {

    TCB* pstTarget;
    BYTE bPriority;

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    pstTarget = gs_stScheduler.pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID ) {

        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );

        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

        kSchedule();

        while( 1 );

    } else {

        pstTarget = kRemoveTaskFromReadyList( qwTaskID );
        if( pstTarget == NULL ) {

            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL ) {

                pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
                SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );

            }

            kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

            return TRUE;

        }

        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        kAddListToTail( &( gs_stScheduler.stWaitList ), pstTarget );

    }

    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    return TRUE;

}

// 태스크가 스스로 종료
void kExitTask( void ) {

    kEndTask( gs_stScheduler.pstRunningTask->stLink.qwID );

}

// 준비 큐에 있는 모든 태스크의 수를 반환
int kGetReadyTaskCount( void ) {

    int iTotalCount = 0;
    int i;

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    for( i = 0; i < TASK_MAXREADYLISTCOUNT ; i++ ) {

        iTotalCount += kGetListCount( &( gs_stScheduler.vstReadyList[ i ] ) );

    }

    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    return iTotalCount;

}

int kGetTaskCount( void ) {

    int iTotalCount;

    iTotalCount = kGetReadyTaskCount();

    kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );

    iTotalCount += kGetListCount( &( gs_stScheduler.stWaitList ) ) + 1;

    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

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

QWORD kGetProcessorLoad( void ) {

    return gs_stScheduler.qwProcessorLoad;

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

// 유휴 태스크
// 대기 큐에 삭제 대기 중인 태스크를 정리
void kIdleTask( void ) {

    TCB* pstTask, * pstChildThread, * pstProcess;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;
    int i, iCount;
    QWORD qwTaskID;
    void* pstThreadLink;

    qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
    qwLastMeasureTickCount = kGetTickCount();

    while( 1 ) {

        qwCurrentMeasureTickCount = kGetTickCount();
        qwCurrentSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;

        if( qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0 ) {

            gs_stScheduler.qwProcessorLoad = 0;

        } else {

            gs_stScheduler.qwProcessorLoad = 100 - ( qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask ) * 100 / ( qwCurrentMeasureTickCount - qwLastMeasureTickCount );

        }

        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        kHaltProcessorByLoad();

        if( kGetListCount( &( gs_stScheduler.stWaitList ) ) >= 0 ) {

            while( 1 ) {

                kLockForSpinLock( &( gs_stScheduler.stSpinLock ) );
                pstTask = kRemoveListFromHeader( &( gs_stScheduler.stWaitList ) );
                if( pstTask == NULL ) { 
                    
                    kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

                    break; 
                    
                }

                if( pstTask->qwFlags & TASK_FLAGS_PROCESS ) {

                    iCount = kGetListCount( &( pstTask->stChildThreadList ) );
                    for( i = 0; i < iCount; i++ ) {

                        pstThreadLink = ( TCB * ) kRemoveListFromHeader( &( pstTask->stChildThreadList ) );
                        if( pstThreadLink == NULL ) { 
                            
                            break; 
                            
                        }

                        pstChildThread = GETTCBFROMTHREADLINK( pstThreadLink );

                        kAddListToTail( &( pstTask->stChildThreadList ), &( pstChildThread->stThreadLink ) );

                        kEndTask( pstChildThread->stLink.qwID );

                    }

                    if( kGetListCount( &( pstTask->stChildThreadList ) ) > 0 ) {

                        kAddListToTail( &( gs_stScheduler.stWaitList ), pstTask );

                        kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );
                        continue;

                    } else {

                        // TODO: 추후 코드 삽입

                    }

                } else if( pstTask->qwFlags & TASK_FLAGS_THREAD ) {

                    pstProcess = kGetProcessByThread( pstTask );
                    if( pstProcess != NULL ) {

                        kRemoveList( &( pstProcess->stChildThreadList ), pstTask->stLink.qwID );

                    }

                }

                qwTaskID = pstTask->stLink.qwID;
                kFreeTCB( qwTaskID );
                
                kUnlockForSpinLock( &( gs_stScheduler.stSpinLock ) );

                kPrintf( "IDLE: Task ID[0x%q] is completely ended.\n", qwTaskID );

            }

        }

        kSchedule();

    }

}

void kHaltProcessorByLoad( void ) {

    if( gs_stScheduler.qwProcessorLoad < 40 ) {

        kHlt();
        kHlt();
        kHlt();

    } else if( gs_stScheduler.qwProcessorLoad < 80 ) {

        kHlt();
        kHlt();

    } else if( gs_stScheduler.qwProcessorLoad < 95 ) {

        kHlt();

    }

}

// FPU 관련
QWORD kGetLastFPUUsedTaskID( void ) {

    return gs_stScheduler.qwLastFPUUsedTaskID;

}

void kSetLastFPUUsedTaskID( QWORD qwTaskID ) {

    gs_stScheduler.qwLastFPUUsedTaskID = qwTaskID;

}