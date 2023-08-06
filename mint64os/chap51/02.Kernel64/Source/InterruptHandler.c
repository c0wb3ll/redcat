#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "HardDisk.h"
#include "Mouse.h"

static INTERRUPTMANAGER gs_stInterruptManager;

// 인터럽트 자료구조 초기화
void kInitializeHandler( void ) {

    kMemSet( &gs_stInterruptManager, 0, sizeof( gs_stInterruptManager ) );

}

// 인터럽트 처리 모드를 설정
void kSetSymmetricIOMode( BOOL bSymmetricIOMode ) {

    gs_stInterruptManager.bSymmetricIOMode = bSymmetricIOMode;

}

// 인터럽트 부하 분산 기능을 사용할지 여부 설정
void kSetInterruptLoadBalancing( BOOL bUseLoadBalancing ) {

    gs_stInterruptManager.bUseLoadBalancing = bUseLoadBalancing;

}

// 코어별 인터럽트 처리 횟수를 증가
void kIncreaseInterruptCount( int iIRQ ) {

    gs_stInterruptManager.vvqwCoreInterruptCount[ kGetAPICID() ][ iIRQ ]++;

}

// 현재 인터럽트 모드에 맞추어 EOI 전송
void kSendEOI( int iIRQ ) {

    if( gs_stInterruptManager.bSymmetricIOMode == FALSE ) {

        kSendEOIToPIC( iIRQ );

    } else {

        kSendEOIToLocalAPIC();

    }

}

// 인터럽트 핸들러 자료구조를 반환
INTERRUPTMANAGER* kGetInterruptManager( void ) {

    return &gs_stInterruptManager;

}

// 인터럽트 부하 분산 처리
void kProcessLoadBalancing( int iIRQ ) {

    QWORD qwMinCount = 0xFFFFFFFFFFFFFFFF;
    int iMinCountCoreIndex;
    int iCoreCount;
    int i;
    BOOL bResetCount = FALSE;
    BYTE bAPICID;

    bAPICID = kGetAPICID();

    if( ( gs_stInterruptManager.vvqwCoreInterruptCount[ bAPICID ][ iIRQ ] == 0 ) || ( ( gs_stInterruptManager.vvqwCoreInterruptCount[ bAPICID ][ iIRQ ] % INTERRUPT_LOADBALANCINGDIVIDOR ) != 0 ) || ( gs_stInterruptManager.bUseLoadBalancing == FALSE ) ) {

        return ;

    }

    iMinCountCoreIndex = 0;
    iCoreCount = kGetProcessorCount();
    for( i = 0; i < iCoreCount; i++ ) {

        if( ( gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] < qwMinCount ) ) {

            qwMinCount = gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ];
            iMinCountCoreIndex = i;

        } else if( gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] >= 0xFFFFFFFFFFFFFFFE ) {

            bResetCount = TRUE;

        }

    }

    kRoutingIRQToAPICID( iIRQ, iMinCountCoreIndex );

    if( bResetCount == TRUE ) {

        for( i = 0; i < iCoreCount; i++ ) {

            gs_stInterruptManager.vvqwCoreInterruptCount[ i ][ iIRQ ] = 0;

        }

    }

}

void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode ) {

    char vcBuffer[ 100 ];
    BYTE bAPICID;
    TCB* pstTask;

    bAPICID = kGetAPICID();
    pstTask = kGetRunningTask( bAPICID );


    kPrintStringXY( 0, 0, "==================================================" );
    kPrintStringXY( 0, 1, "                  Exception Occur~!!!!            " );
    kSPrintf( vcBuffer,   "    Vector:%d    Core ID:0x%X    ErrorCode:0x%X   ", iVectorNumber, bAPICID, qwErrorCode );
    kPrintStringXY( 0, 2, vcBuffer );
    kSPrintf( vcBuffer,   "                 Task ID:0x%Q ", pstTask->stLink.qwID );
    kPrintStringXY( 0, 3, vcBuffer );
    kPrintStringXY( 0, 3, "==================================================");

    if( pstTask->qwFlags & TASK_FLAGS_USERLEVEL ) {

        kEndTask( pstTask->stLink.qwID );

        while( 1 ) {

            ;

        }

    } else {

        while( 1 ) {

            ;

        }

    }

}

void kCommonInterruptHandler( int iVectorNumber ) {

    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;
    int iIRQ;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );

    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    kSendEOI( iIRQ );

    kIncreaseInterruptCount( iIRQ );

    kProcessLoadBalancing( iIRQ );

}

void kKeyboardHandler( int iVectorNumber ) {

    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );

    if( kIsOutputBufferFull() == TRUE ) {

        if( kIsMouseDataInOutputBuffer() == FALSE ) {

            bTemp = kGetKeyboardScanCode();
            kConvertScanCodeAndPutQueue( bTemp );

        } else {

            bTemp = kGetKeyboardScanCode();
            kAccumulateMouseDataAndPutQueue( bTemp );

        }
        
    }

    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    kSendEOI( iIRQ );

    kIncreaseInterruptCount( iIRQ );

    kProcessLoadBalancing( iIRQ );

}

// 타이머 인터럽트 핸들러
void kTimerHandler( int iVectorNumber ) {

    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;
    int iIRQ;
    BYTE bCurrentAPICID;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = ( g_iTimerInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );

    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    kSendEOI( iIRQ );

    kIncreaseInterruptCount( iIRQ );

    bCurrentAPICID = kGetAPICID();
    if( bCurrentAPICID == 0 ) {

        g_qwTickCount++;

    }

    kDecreaseProcessorTime( bCurrentAPICID );
    if( kIsProcessorTimeExpired( bCurrentAPICID ) == TRUE ) {

        kScheduleInInterrupt();

    }

}

// Device Not Available 예외 핸들러
void kDeviceNotAvailableHandler( int iVectorNumber ) {

    TCB* pstFPUTask, * pstCurrentTask;
    QWORD qwLastFPUTaskID;
    BYTE bCurrentAPICID;

    char vcBuffer[] = "[EXC:  , ]";
    static int g_iFPUInterruptCount = 0;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iFPUInterruptCount;
    g_iFPUInterruptCount = ( g_iFPUInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );

    bCurrentAPICID = kGetAPICID();

    kClearTS();

    qwLastFPUTaskID = kGetLastFPUUsedTaskID( bCurrentAPICID );
    pstCurrentTask = kGetRunningTask( bCurrentAPICID );

    if( qwLastFPUTaskID == pstCurrentTask->stLink.qwID ) {

        return ;

    } else if( qwLastFPUTaskID != TASK_INVALIDID ) {

        pstFPUTask = kGetTCBInTCBPool( GETTCBOFFSET( qwLastFPUTaskID ) );

        if( ( pstFPUTask != NULL ) && ( pstFPUTask->stLink.qwID == qwLastFPUTaskID ) ) {

            kSaveFPUContext( pstFPUTask->vqwFPUContext );

        }

    }

    if( pstCurrentTask->bFPUUsed == FALSE ) {

        kInitializeFPU();
        pstCurrentTask->bFPUUsed = TRUE;

    } else {

        kLoadFPUContext( pstCurrentTask->vqwFPUContext );

    }

    kSetLastFPUUsedTaskID( bCurrentAPICID, pstCurrentTask->stLink.qwID );

}

// 하드디스크에서 발생하는 인터럽트의 핸들러
void kHDDHandler( int iVectorNumber ) {

    char vcBuffer[] = "[INT:  , ]";
    static int g_iHDDInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iHDDInterruptCount;
    g_iHDDInterruptCount = ( g_iHDDInterruptCount + 1 ) % 10;

    kPrintStringXY( 10, 0, vcBuffer );

    kSetHDDInterruptFlag( TRUE, TRUE );

    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    kSendEOI( iIRQ );

    kIncreaseInterruptCount( iIRQ );

    kProcessLoadBalancing( iIRQ );

}

// 마우스 인터럽트의 핸들러
void kMouseHandler( int iVectorNumber ) {

    char vcBuffer[] = "[INT:  , ]";
    static int g_iMouseInterruptCount = 0;
    BYTE bTemp;
    int iIRQ;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iMouseInterruptCount;
    g_iMouseInterruptCount = ( g_iMouseInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );

    if( kIsOutputBufferFull() == TRUE ) {

        if( kIsMouseDataInOutputBuffer() == FALSE ) {

            bTemp = kGetKeyboardScanCode();
            kConvertScanCodeAndPutQueue( bTemp );

        } else {

            bTemp = kGetKeyboardScanCode();
            kAccumulateMouseDataAndPutQueue( bTemp );

        }

    }

    iIRQ = iVectorNumber - PIC_IRQSTARTVECTOR;

    kSendEOI( iIRQ );

    kIncreaseInterruptCount( iIRQ );

    kProcessLoadBalancing( iIRQ );

}