#include "Task.h"
#include "Descriptor.h"

// 스케줄러 관련 자료구조
static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;

// 태스크 풀, 태스크 관련
// 태스크 풀 초기화
void kInitializeTCBPool( void ) {

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
TCB* kAllocateTCB( void ) {

    TCB* pstEmptyTCB;
    int i;

    if( gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount ) { return NULL; }

    for( i = 0; i < gs_stTCBPoolManager.iMaxCount; i++ ) {

        if( ( gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID >> 32 ) == 0 ) {

            pstEmptyTCB = &( gs_stTCBPoolManager.pstStartAddress[ i ] );
            break;

        }

    }

    pstEmptyTCB->stLink.qwID = ( ( QWORD ) gs_stTCBPoolManager.iAllocatedCount << 32 ) | i;
    gs_stTCBPoolManager.iUseCount++;
    gs_stTCBPoolManager.iAllocatedCount++;
    if( gs_stTCBPoolManager.iAllocatedCount == 0 ) { gs_stTCBPoolManager.iAllocatedCount = 1; }

    return pstEmptyTCB;

}

// TCB 해제
void kFreeTCB( QWORD qwID ) {

    int i;

    i = qwID & 0xFFFFFFFF;

    kMemSet( &( gs_stTCBPoolManager.pstStartAddress[ i ].stContext ), 0, sizeof( CONTEXT ) );
    gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;

    gs_stTCBPoolManager.iUseCount--;

}

// 태스크 생성
// 태스크 ID에 따라 스택 풀에서 스택 자동 할당
TCB* kCreateTask( QWORD qwFlags, QWORD qwEntryPointAddress ) {

    TCB* pstTask;
    void* pvStackAddress;

    pstTask = kAllocateTCB();
    if( pstTask == NULL ) { return NULL; }

    pvStackAddress = ( void* ) ( TASK_STACKPOOLADDRESS + ( TASK_STACKSIZE * ( pstTask->stLink.qwID & 0xFFFFFFFF ) ) );
    kSetUpTask( pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, TASK_STACKSIZE );
    kAddTaskToReadyList( pstTask );

    return pstTask;

}

// 파라미터 이용해서 TCB 설정
void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize ) {

    kMemSet( pstTCB->stContext.vqRegister, 0, sizeof( pstTCB->stContext.vqRegister ) );

    pstTCB->stContext.vqRegister[ TASK_RSPOFFSET ] = ( QWORD ) pvStackAddress + qwStackSize;
    pstTCB->stContext.vqRegister[ TASK_RBPOFFSET ] = ( QWORD ) pvStackAddress + qwStackSize;

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

    kInitializeTCBPool();

    kInitializeList( &( gs_stScheduler.stReadyList ) );

    gs_stScheduler.pstRunningTask = kAllocateTCB();

}

// 현재 수행중인 태스크 설정
void kSetRunningTask( TCB* pstTask ) {

    gs_stScheduler.pstRunningTask = pstTask;

}

// 현재 수행 중인 태스크 반환
TCB* kGetRunningTask( void ) {

    return gs_stScheduler.pstRunningTask;

}

// 태스크 리스트에서 다음으로 실행할 태스크 얻음
TCB* kGetNextTaskToRun( void ) {

    if( kGetListCount( &( gs_stScheduler.stReadyList ) ) == 0 ) { return NULL; }

    return ( TCB* ) kRemoveListFromHeader( &( gs_stScheduler.stReadyList ) );

}

// 태스크를 스케줄러의 준비 리스트에 삽입
void kAddTaskToReadyList( TCB* pstTask ) {

    kAddListToTail( &( gs_stScheduler.stReadyList ), pstTask );

}

// 다른 태스크를 찾아서 전환
// 인터럽트나 예외시에는 호출 X
void kSchedule( void ) {

    TCB* pstRunningTask, * pstNextTask;
    BOOL bPreviousFlag;

    if( kGetListCount( &( gs_stScheduler.stReadyList ) ) == 0 ) { return ; }

    bPreviousFlag = kSetInterruptFlag( FALSE );
    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL ) {

        kSetInterruptFlag( bPreviousFlag );
        return ;

    }

    pstRunningTask = gs_stScheduler.pstRunningTask;
    kAddTaskToReadyList( pstRunningTask );

    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    gs_stScheduler.pstRunningTask = pstNextTask;
    kSwitchContext( &( pstRunningTask->stContext ), &( pstNextTask->stContext ) );

    kSetInterruptFlag( bPreviousFlag );

}

// 인터럽트가 발생했을 대, 다른 태스크를 찾아 전환
// 인터럽트나 예외 발생 시 호출
BOOL kScheduleInInterrupt( void ) {

    TCB* pstRunningTask, * pstNextTask;
    char* pcContextAddress;

    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL ) { return FALSE; }

    pcContextAddress = ( char* ) IST_STARTADDRESS + IST_SIZE - sizeof( CONTEXT );

    pstRunningTask = gs_stScheduler.pstRunningTask;
    kMemCpy( &( pstRunningTask->stContext ), pcContextAddress, sizeof( CONTEXT ) );
    kAddTaskToReadyList( pstRunningTask );

    gs_stScheduler.pstRunningTask = pstNextTask;
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

    if( gs_stScheduler.iProcessorTime <= 0 ) { return TRUE; }

    return FALSE;

}