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
#include "Mouse.h"

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

    kCreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, ( QWORD ) kIdleTask, kGetAPICID() );

    if( *( BYTE* ) VBE_STARTGRAPHICMODEFLAGADDRESS == 0 ) {

        kStartConsoleShell();

    } else {

        kStartGraphicModeTest();

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

    // kPrintf( "Application Processor[APIC ID: %d] is Activated\n", kGetAPICID() );

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

// 윈도우 프레임 그리기
void kDrawWindowFrame( int iX, int iY, int iWidth, int iHeight, const char* pcTitle ) {

    char* pcTestString1 = "This is MINT64 OS's Window prototype";
    char* pcTestString2 = "c0wb3ll";
    VBEMODEINFOBLOCK* pstVBEMode;
    COLOR* pstVideoMemory;
    RECT stScreenArea;

    pstVBEMode = kGetVBEModeInfoBlock();

    stScreenArea.iX1 = 0;
    stScreenArea.iY1 = 0;
    stScreenArea.iX2 = pstVBEMode->wXResolution - 1;
    stScreenArea.iY2 = pstVBEMode->wYResolution - 1;

    pstVideoMemory = ( COLOR* ) ( ( QWORD )pstVBEMode->dwPhysicalBasePointer & 0xFFFFFFFF );

    // 윈도우 프레임 가장자리
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX, iY, iX + iWidth, iY + iHeight, RGB( 255, 255, 255 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + 1, iY + 1, iX + iWidth - 1, iY + iHeight - 1, RGB( 255, 255, 255 ), FALSE );

    // 제목 표시줄 채움
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX, iY + 3, iX + iWidth - 1, iY + 21, RGB( 255, 255, 255 ), TRUE );

    // 윈도우 제목 표시
    kInternalDrawText( &stScreenArea, pstVideoMemory, iX + 6, iY + 3, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), pcTitle, kStrLen( pcTitle ) );

    // 제목 표시줄을 입체로 보이게
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 1, iY + 1, iX + iWidth - 1, iY + 1, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 1, iY + 2, iX + iWidth - 1, iY + 2, RGB( 255, 255, 255 ) );

    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 1, iY + 2, iX + 1, iY + 20, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 2, iY + 2, iX + 2, iY + 20, RGB( 255, 255, 255 ) );

    // 제목 표시줄 아래 선
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 2, iY + 19, iX + iWidth - 2, iY + 19, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 2, iY + 20, iX + iWidth - 2, iY + 20, RGB( 255, 255, 255 ) );

    // 닫기 버튼 그리기
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2, iY + 19, RGB( 0, 0, 0 ), TRUE );

    // 닫기 버튼을 입체로 보이게
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2, iY + 1, iX + iWidth - 2, iY + 19 - 1, RGB( 255, 255, 255 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 1, iY + 1, iX + iWidth - 2 - 1, iY + 19 - 1, RGB( 255, 255, 255 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 1, iY + 19, iX + iWidth - 2, iY + 19, RGB( 255, 255, 255 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 1, iY + 19 - 1, iX + iWidth - 2, iY + 19 - 1, RGB( 255, 255, 255 ), TRUE );

    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 1, iY + 1, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18, iY + 1 + 1, iX + iWidth - 2 - 2, iY + 1 + 1, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 18, iY + 19, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 1, iY + 1, iX + iWidth - 2 - 18 + 1, iY + 19 - 1, RGB( 255, 255, 255 ) );

    // 대각선 x를 그림
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 4, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 4, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 5, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 5, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 4, iY + 1 + 5, iX + iWidth - 2 - 5, iY + 19 - 4, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 4, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 4, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 5, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 5, RGB( 255, 255, 255 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 4, iY + 19 - 5, iX + iWidth - 2 - 5, iY + 1 + 4, RGB( 255, 255, 255 ) );

    // 내부를 그림
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + 2, iY + 21, iX + iWidth - 2, iY + iHeight - 2, RGB( 0, 0, 0 ), TRUE );

    // 테스트 문자 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, iX + 10, iY + 30, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ), pcTestString1, kStrLen( pcTestString1 ) );
    kInternalDrawText( &stScreenArea, pstVideoMemory, iX + 10, iY + 50, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ), pcTestString2, kStrLen( pcTestString2 ) );

}

#define MOUSE_CURSOR_WIDTH  10
#define MOUSE_CURSOR_HEIGHT 20

static BYTE gs_vwMouseBuffer[ MOUSE_CURSOR_WIDTH * MOUSE_CURSOR_HEIGHT ] = {

    1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 1, 0, 0, 0, 0, 0, 0,
    1, 2, 3, 2, 1, 0, 0, 0, 0, 0,
    1, 2, 3, 3, 2, 1, 0, 0, 0, 0,
    1, 2, 3, 3, 3, 2, 1, 0, 0, 0,
    1, 2, 3, 3, 3, 3, 2, 1, 0, 0,
    1, 2, 3, 3, 3, 3, 2, 2, 1, 0,
    1, 2, 3, 3, 3, 2, 2, 2, 2, 1,
    1, 2, 3, 3, 3, 1, 1, 1, 1, 0,
    1, 2, 2, 1, 2, 1, 0, 0, 0, 0,
    1, 2, 1, 1, 2, 2, 1, 0, 0, 0,
    1, 1, 0, 0, 1, 2, 1, 0, 0, 0,
    1, 0, 0, 0, 1, 2, 2, 1, 0, 0,
    0, 0, 0, 0, 0, 1, 2, 1, 0, 0,
    0, 0, 0, 0, 0, 1, 2, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

};

#define MOUSE_CURSOR_OUTERLINE  RGB( 255, 255, 255 )
#define MOUSE_CURSOR_OUTER      RGB( 255, 255, 255 )
#define MOUSE_CURSOR_INNER      RGB( 0, 0, 0 )

void kDrawCursor( RECT* pstArea, COLOR* pstVideoMemory, int iX, int iY ) {

    int i;
    int j;
    BYTE* pbCurrentPos;

    pbCurrentPos = gs_vwMouseBuffer;

    for( j = 0; j < MOUSE_CURSOR_HEIGHT; j++ ) {

        for( i = 0; i < MOUSE_CURSOR_WIDTH; i++ ) {

            switch( *pbCurrentPos ) {

            case 0:
                // nothing
                break;

            case 1:
                kInternalDrawPixel( pstArea, pstVideoMemory, i + iX, j + iY, MOUSE_CURSOR_OUTERLINE );
                break;

            case 2:
                kInternalDrawPixel( pstArea, pstVideoMemory, i + iX, j + iY, MOUSE_CURSOR_OUTER );

            case 3:
                kInternalDrawPixel( pstArea, pstVideoMemory, i + iX, j + iY, MOUSE_CURSOR_INNER );
                break;

            }

            pbCurrentPos++;

        }

    }

}

void kStartGraphicModeTest() {

    VBEMODEINFOBLOCK* pstVBEMode;
    int iX, iY;
    COLOR* pstVideoMemory;
    RECT stScreenArea;
    int iRelativeX, iRelativeY;
    BYTE bButton;

    pstVBEMode = kGetVBEModeInfoBlock();

    stScreenArea.iX1 = 0;
    stScreenArea.iY1 = 0;
    stScreenArea.iX2 = pstVBEMode->wXResolution - 1;
    stScreenArea.iY2 = pstVBEMode->wYResolution - 1;

    pstVideoMemory = ( COLOR* ) ( ( QWORD )pstVBEMode->dwPhysicalBasePointer & 0xFFFFFFFF );

    iX = pstVBEMode->wXResolution / 2;
    iY = pstVBEMode->wYResolution / 2;

    kInternalDrawRect( &stScreenArea, pstVideoMemory, stScreenArea.iX1, stScreenArea.iY1, stScreenArea.iX2, stScreenArea.iY2, RGB( 60, 80, 100 ), TRUE );

    kDrawCursor( &stScreenArea, pstVideoMemory, iX, iY );

    while( 1 ) {

        if( kGetMouseDataFromMouseQueue( &bButton, &iRelativeX, &iRelativeY ) == FALSE ) {

            kSleep( 0 );
            continue;

        }

        kInternalDrawRect( &stScreenArea, pstVideoMemory, iX, iY, iX + MOUSE_CURSOR_WIDTH, iY + MOUSE_CURSOR_HEIGHT, RGB( 60, 80, 100 ), TRUE );

        iX += iRelativeX;
        iY += iRelativeY;

        if( iX < stScreenArea.iX1 ) {

            iX = stScreenArea.iX1;

        } else if( iX > stScreenArea.iX2 ) {

            iX = stScreenArea.iX2;

        }

        if( iY < stScreenArea.iY1 ) {

            iY = stScreenArea.iY1;

        } else if( iY > stScreenArea.iY2 ) {

            iY = stScreenArea.iY2;

        }

        if( bButton & MOUSE_LBUTTONDOWN ) {

            kDrawWindowFrame( iX - 10, iY - 10, 400, 200, "Mint64 OS Test Window with c0wb3ll");

        } else if( bButton & MOUSE_RBUTTONDOWN ) {

            kInternalDrawRect( &stScreenArea, pstVideoMemory, stScreenArea.iX1, stScreenArea.iY1, stScreenArea.iX2, stScreenArea.iY2, RGB( 60, 80, 100 ), TRUE );

        }

        kDrawCursor( &stScreenArea, pstVideoMemory, iX, iY );

    }

}