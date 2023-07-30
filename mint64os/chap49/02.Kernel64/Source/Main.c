#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Task.h"
#include "PIT.h"
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "FileSystem.h"
#include "SerialPort.h"
#include "MultiProcessor.h"
#include "VBE.h"
#include "2DGraphics.h"
#include "MPConfigurationTable.h"
#include "WindowManagerTask.h"
#include "SystemCall.h"

// Application Processor를 위한 메인 함수
void MainForApplicationProcessor( void );
// 멀티코어 프로세서 또는 멀티 프로세서 모드로 전환하는 함수
BOOL kChangeToMultiCoreMode( void );
// 그래픽 모드 테스트 함수
void kStartGraphicModeTest( void );

void Main( void ) {

    int iCursorX, iCursorY;
    BYTE bButton;
    int iX;
    int iY;

    if( *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) == 0 ) {

        MainForApplicationProcessor();

    }

    *( ( BYTE* ) BOOTSTRAPPROCESSOR_FLAGADDRESS ) = 0;

    kInitializeConsole( 0, 10 );
    kPrintf( "[*] Switch To IA-32e Mode Success...\n" );
    kPrintf( "[*] IA-32e C Language Kernel Start [Pass]\n" );
    kPrintf( "[*] Initialize Console [Pass]\n" );

    kGetCursor( &iCursorX, &iCursorY );
    kPrintf( "[*] GDT Initialize And Switch For IA-32e Mode [    ]" );
    kInitializeGDTTableAndTSS();
    kLoadGDTR( GDTR_STARTADDRESS );
    kSetCursor( 47, iCursorY++ );
    kPrintf( "Pass\n" );

    kPrintf( "[*] TSS Segment Load [    ]" );
    kLoadTR( GDT_TSSSEGMENT );
    kSetCursor( 22, iCursorY++ );
    kPrintf( "Pass\n" );

    kPrintf( "[*] IDT Initialize [    ]");
    kInitializeIDTTables();
    kLoadIDTR( IDTR_STARTADDRESS );
    kSetCursor( 20, iCursorY++ );
    kPrintf( "Pass\n" );

    kPrintf( "[*] Total RAM Size Check [    ]" );
    kCheckTotalRAMSize();
    kSetCursor( 26, iCursorY++ );
    kPrintf( "Pass], Size = %d MB\n", kGetTotalRAMSize() );

    kPrintf( "[*] TCB Pool And Scheduler Intialize [Pass]\n");
    iCursorY++;
    kInitializeScheduler();

    kPrintf( "[*] Dynamic Memory Initialize [Pass]\n");
    iCursorY++;
    kInitializeDynamicMemory();

    kInitializePIT( MSTOCOUNT( 1 ), 1 );

    kPrintf( "[*] Keyboard Activate And Queue Initialize [    ]" );

    if( kInitializeKeyboard() == TRUE ) {
        kSetCursor( 44, iCursorY++ );
        kPrintf( "Pass\n" );
        kChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else{
        kSetCursor( 44, iCursorY++ );
        kPrintf( "Fail\n" );
        while( 1 );
    }

    kPrintf( "[*] Mouse Activate And Queue Initialize [    ]" );
    if( kInitializeMouse() == TRUE ) {

        kEnableMouseInterrupt();
        kSetCursor( 41, iCursorY++ );
        kPrintf( "Pass\n" );

    } else {

        kSetCursor( 41, iCursorY++ );
        kPrintf( "Fail\n" );
        while( 1 );

    }

    kPrintf( "[*] PIC Controller And Interrupt Initialize [    ]");
    kInitializePIC();
    kMaskPICInterrupt( 0 );
    kEnableInterrupt();
    kSetCursor( 0x2d, iCursorY++ );
    kPrintf( "Pass\n" );

    kPrintf( "[*] File System Initialize [    ]" );
    if( kInitializeFileSystem() == TRUE ) {

        kSetCursor( 0x1c, iCursorY++ );
        kPrintf( "Pass\n" );

    } else {

        kSetCursor( 0x1c, iCursorY++ );
        kPrintf( "Fail\n" );

    }

    kPrintf( "[*] Serial Port Initialize [Pass]\n" );
    iCursorY++;
    kInitializeSerialPort();

    kPrintf( "[*] Change To MultiCore Processor Mode [    ]" );
    if( kChangeToMultiCoreMode() == TRUE ) {

        kSetCursor( 40, iCursorY++ );
        kPrintf( "Pass\n" );

    } else {

        kSetCursor( 40, iCursorY++ );
        kPrintf( "Fail\n" );

    }

    kPrintf( "[*] System Call MSR Initialize [Pass]\n" );
    iCursorY++;
    kInitializeSystemCall();

    kCreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, ( QWORD ) kIdleTask, kGetAPICID() );

    if( *( BYTE* ) VBE_STARTGRAPHICMODEFLAGADDRESS == 0 ) {

        kStartConsoleShell();

    } else {

        kStartWindowManager();

    }

}

// Application Processor 용 C 언어 커널 엔트리 포인트
void MainForApplicationProcessor( void ) {

    QWORD qwTickCount;
    
    kLoadGDTR( GDTR_STARTADDRESS );
    kLoadTR( GDT_TSSSEGMENT + ( kGetAPICID() * sizeof( GDTENTRY16 ) ) );

    kLoadIDTR( IDTR_STARTADDRESS );

    kInitializeScheduler();

    kEnableSoftwareLocalAPIC();

    kSetTaskPriority( 0 );

    kInitializeLocalVectorTable();

    kEnableInterrupt();

    kInitializeSystemCall();

    kIdleTask();

}

// 멀티코어 프로세서 또는 멀티프로세서 모드로 전환하는 함수
BOOL kChangeToMultiCoreMode( void ) {

    MPCONFIGURATIONMANAGER* pstMPManager;
    BOOL bInterruptFlag;
    int i;

    if( kStartUpApplicationProcessor() == FALSE ) {

        return FALSE;

    }

    pstMPManager = kGetMPConfigurationManager();
    if( pstMPManager->bUsePICMode == TRUE ) {

        kOutPortByte( 0x22, 0x70 );
        kOutPortByte( 0x23, 0x01 );

    }

    kMaskPICInterrupt( 0xFFFF );

    kEnableGlobalLocalAPIC();

    kEnableSoftwareLocalAPIC();

    bInterruptFlag = kSetInterruptFlag( FALSE );

    kSetTaskPriority( 0 );

    kInitializeLocalVectorTable();

    kSetSymmetricIOMode( TRUE );

    kInitializeIORedirectionTable();

    kSetInterruptFlag( bInterruptFlag );

    kSetInterruptLoadBalancing( TRUE );

    for( i = 0; i < MAXPROCESSORCOUNT; i++ ) {

        kSetTaskLoadBalancing( i, TRUE );

    }

    return TRUE;

}