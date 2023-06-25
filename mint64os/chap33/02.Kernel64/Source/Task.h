#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"
#include "List.h"    
#include "Synchronization.h"   

// SS, RSP, RFLAGS, CS, RIP + IS        R 에서 저장하는 19개의 레지스터 매크로
#define TASK_REGISTERCOUNT              ( 5 + 19 )
#define TASK_REGISTERSIZE               8

// CONTEXT 자료구조의 레지스터      오프셋
#define TASK_GSOFFSET                   0
#define TASK_FSOFFSET                   1
#define TASK_ESOFFSET                   2
#define TASK_DSOFFSET                   3
#define TASK_R15OFFSET                  4
#define TASK_R14OFFSET                  5
#define TASK_R13OFFSET                  6
#define TASK_R12OFFSET                  7
#define TASK_R11OFFSET                  8
#define TASK_R10OFFSET                  9
#define TASK_R9OFFSET                   10
#define TASK_R8OFFSET                   11
#define TASK_RSIOFFSET                  12
#define TASK_RDIOFFSET                  13
#define TASK_RDXOFFSET                  14
#define TASK_RCXOFFSET                  15
#define TASK_RBXOFFSET                  16
#define TASK_RAXOFFSET                  17
#define TASK_RBPOFFSET                  18
#define TASK_RIPOFFSET                  19
#define TASK_CSOFFSET                   20
#define TASK_RFLAGSOFFSET               21
#define TASK_RSPOFFSET                  22
#define TASK_SSOFFSET                   23

// 태스크 풀의 어드레스     
#define TASK_TCBPOOLADDRESS             0x800000
#define TASK_MAXCOUNT                   1024
// 스택 풀과 스택의 크기        
#define TASK_STACKPOOLADDRESS           ( TASK_TCBPOOLADDRESS + sizeof( TCB ) * TASK_MAXCOUNT )
#define TASK_STACKSIZE                  8192
// 유효하지 않은 태스크 ID      
#define TASK_INVALIDID                  0xFFFFFFFFFFFFFFFF
// 태스크가 최대로 쓸 수 있는 프        로세서 시간 ms
#define TASK_PROCESSORTIME              5
// 준비 리스트의 수     
#define TASK_MAXREADYLISTCOUNT          5

// 태스크의 우선 순위       
#define TASK_FLAGS_HIGHEST              0
#define TASK_FLAGS_HIGH                 1
#define TASK_FLAGS_MEDIUM               2
#define TASK_FLAGS_LOW                  3
#define TASK_FLAGS_LOWEST               4
#define TASK_FLAGS_WAIT                 0xFF

// 태스크의 플래그      
#define TASK_FLAGS_ENDTASK              0x8000000000000000
#define TASK_FLAGS_SYSTEM               0x4000000000000000
#define TASK_FLAGS_PROCESS              0x2000000000000000
#define TASK_FLAGS_THREAD               0x1000000000000000
#define TASK_FLAGS_IDLE                 0x0800000000000000

// 함수 매크로      
#define GETPRIORITY( x )                ( ( x ) & 0xFF )
#define SETPRIORITY( x, priority )      ( ( x ) = ( ( x ) & 0xFFFFFFFFFFFFFF00 ) | ( priority ) )
#define GETTCBOFFSET( x )               ( ( x ) & 0xFFFFFFFF )

#define GETTCBFROMTHREADLINK( x )       ( TCB* ) ( ( QWORD ) ( x ) - offsetof( TCB, stThreadLink ) )

#pragma pack( push, 1 )

// 콘텍스트 관련 자료구조
typedef struct kContextStruct {

    QWORD vqRegister[ TASK_REGISTERCOUNT ];

} CONTEXT;

// 태스크의 상태를 관리하는 자료구조
typedef struct kTaskControlBlockStruct {

    LISTLINK stLink;

    QWORD qwFlags;

    void* pvMemoryAddress;
    QWORD qwMemorySize;

    LISTLINK stThreadLink;
    
    QWORD qwParentProcessID;
    QWORD vqwFPUContext[ 512 / 8 ];

    LIST stChildThreadList;

    CONTEXT stContext;

    void* pvStackAddress;
    QWORD qwStackSize;

    BOOL bFPUUsed;

    char vcPadding[ 11 ];

} TCB;

// TCB 풀의 상태를 관리하는 자료구조
typedef struct kTCBPoolManagerStruct {

    TCB* pstStartAddress;
    int iMaxCount;
    int iUseCount;

    int iAllocatedCount;

} TCBPOOLMANAGER;

// 스케줄러 상태를 관리하는 자료구조
typedef struct kSchedulerStruct {

    SPINLOCK stSpinLock;

    TCB* pstRunningTask;

    int iProcessorTime;

    LIST vstReadyList[ TASK_MAXREADYLISTCOUNT ];

    LIST stWaitList;

    int viExecuteCount[ TASK_MAXREADYLISTCOUNT ];

    QWORD qwProcessorLoad;

    QWORD qwSpendProcessorTimeInIdleTask;

    QWORD qwLastFPUUsedTaskID;

} SCHEDULER;

#pragma pack( pop )

// 태스크 풀, 태스크 관련 함수
static void kInitializeTCBPool( void );
static TCB* kAllocateTCB( void );
static void kFreeTCB( QWORD qwID );
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, QWORD qwEntryPointAddress );
static void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress, void* pvStackAddress, QWORD qwStackSize );

// 스케줄러 관련
void kInitializeScheduler( void );
void kSetRunningTask( TCB* pstTask );
TCB* kGetRunningTask( void );
static TCB* kGetNextTaskToRun( void );
static BOOL kAddTaskToReadyList( TCB* pstTask );
void kSchedule( void );
BOOL kScheduleInInterrupt( void );
void kDecreaseProcessorTime( void );
BOOL kIsProcessorTimeExpired( void );
static TCB* kRemoveTaskFromReadyList( QWORD qwTaskID );
BOOL kChangePriority( QWORD qwID, BYTE bPriority );
BOOL kEndTask( QWORD qwTaskID );
void kExitTask( void );
int kGetReadyTaskCount( void );
int kGetTaskCount( void );
TCB* kGetTCBInTCBPool( int iOffset );
BOOL kIsTaskExist( QWORD qwID );
QWORD kGetProcessorLoad( void );
static TCB* kGetProcessByThread( TCB* pstThread );

// 유휴 태스크 관련
void kIdleTask( void );
void kHaltProcessorByLoad( void );

// FPU 관련
QWORD kGetLastFPUUsedTaskID( void );
void kSetLastFPUUsedTaskID( QWORD qwTaskID );

#endif /*__TASK_H__*/