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

void MainForApplicationProcessor( void );
// 그래픽 모드 테스트 함수
void kStartGraphicModeTest();

void Main( void ) {

    int iCursorX, iCursorY;

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

    kPrintf( "Application Processor[APIC ID: %d] is Activated\n", kGetAPICID() );

    kIdleTask();

}

// x를 절대값으로 변환하는 매크로
#define ABS( x )    ( ( ( x ) >= 0 ) ? ( x ) : -( x ) )

// 임의의 XY 좌표 반환
void kGetRandomXY( int *piX, int *piY) {

    int iRandom;

    iRandom = kRandom();
    *piX = ABS( iRandom ) % 1000;

    iRandom = kRandom();
    *piY = ABS( iRandom ) % 700;

}

// 임의의 색 반환
COLOR kGetRandomColor( void ) {

    int iR, iG, iB;
    int iRandom;

    iRandom = kRandom();
    iR = ABS( iRandom ) % 256;

    iRandom = kRandom();
    iG = ABS( iRandom ) % 256;

    iRandom = kRandom();
    iB = ABS( iRandom ) % 256;

    return RGB( iR, iG, iB );

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
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX, iY, iX + iWidth, iY + iHeight, RGB( 109, 218, 22 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + 1, iY + 1, iX + iWidth - 1, iY + iHeight - 1, RGB( 109, 218, 22 ), FALSE );

    // 제목 표시줄 지움
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX, iY + 3, iX + iWidth - 1, iY + 21, RGB( 79, 204, 11 ), TRUE );

    // 윈도우 제목 표시
    kInternalDrawText( &stScreenArea, pstVideoMemory, iX + 6, iY + 3, RGB( 255, 255, 255 ), RGB( 79, 204, 11 ), pcTitle, kStrLen( pcTitle ) );

    // 제목 표시줄을 입체로 보이게
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 1, iY + 1, iX + iWidth - 1, iY + 1, RGB( 183, 249, 171 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 1, iY + 2, iX + iWidth - 1, iY + 2, RGB( 150, 210, 140 ) );

    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 1, iY + 2, iX + 1, iY + 20, RGB( 183, 249, 171 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 2, iY + 2, iX + 2, iY + 20, RGB( 150, 210, 140 ) );

    // 제목 표시줄 아래 선
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 2, iY + 19, iX + iWidth - 2, iY + 19, RGB( 46, 59, 30 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + 2, iY + 20, iX + iWidth - 2, iY + 20, RGB( 46, 59, 30 ) );

    // 닫기 버튼 그리기
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2, iY + 19, RGB( 255, 255, 255 ), TRUE );

    // 닫기 버튼을 입체로 보이게
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2, iY + 1, iX + iWidth - 2, iY + 19 - 1, RGB( 86, 86, 86 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 1, iY + 1, iX + iWidth - 2 - 1, iY + 19 - 1, RGB( 86, 86, 86 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 1, iY + 19, iX + iWidth - 2, iY + 19, RGB( 86, 86, 86 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 1, iY + 19 - 1, iX + iWidth - 2, iY + 19 - 1, RGB( 86, 86, 86 ), TRUE );

    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 1, iY + 1, RGB( 229, 229, 229 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18, iY + 1 + 1, iX + iWidth - 2 - 2, iY + 1 + 1, RGB( 229, 229, 229 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18, iY + 1, iX + iWidth - 2 - 18, iY + 19, RGB( 229, 229, 229 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 1, iY + 1, iX + iWidth - 2 - 18 + 1, iY + 19 - 1, RGB( 229, 229, 229 ) );

    // 대각선 x를 그림
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 4, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 4, RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 5, iY + 1 + 4, iX + iWidth - 2 - 4, iY + 19 - 5, RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 4, iY + 1 + 5, iX + iWidth - 2 - 5, iY + 19 - 4, RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 4, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 4, RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 5, iY + 19 - 4, iX + iWidth - 2 - 4, iY + 1 + 5, RGB( 71, 199, 21 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, iX + iWidth - 2 - 18 + 4, iY + 19 - 5, iX + iWidth - 2 - 5, iY + 1 + 4, RGB( 71, 199, 21 ) );

    // 내부를 그림
    kInternalDrawRect( &stScreenArea, pstVideoMemory, iX + 2, iY + 21, iX + iWidth - 2, iY + iHeight - 2, RGB( 255, 255, 255 ), TRUE );

    // 테스트 문자 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, iX + 10, iY + 30, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), pcTestString1, kStrLen( pcTestString1 ) );
    kInternalDrawText( &stScreenArea, pstVideoMemory, iX + 10, iY + 50, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), pcTestString2, kStrLen( pcTestString2 ) );

}

void kStartGraphicModeTest() {

    VBEMODEINFOBLOCK* pstVBEMode;
    int iX1, iY1, iX2, iY2;
    COLOR stColor1, stColor2;
    int i;
    char* vpcString[] = { "Pixel", "Line", "Rectangle", "Circle", "MINT64 OS", "c0wb3ll" };
    COLOR* pstVideoMemory;
    RECT stScreenArea;

    pstVBEMode = kGetVBEModeInfoBlock();

    stScreenArea.iX1 = 0;
    stScreenArea.iY1 = 0;
    stScreenArea.iX2 = pstVBEMode->wXResolution - 1;
    stScreenArea.iY2 = pstVBEMode->wYResolution - 1;

    pstVideoMemory = ( COLOR* ) ( ( QWORD )pstVBEMode->dwPhysicalBasePointer & 0xFFFFFFFF );

    // (0, 0)에 Pixel이란 문자열을 검은색 바탕에 흰색으로 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, 0, 0, RGB( 255, 255, 255), RGB( 0, 0, 0 ), vpcString[ 0 ], kStrLen( vpcString[ 0 ] ) );
    // 픽셀을 (1, 20), (2, 20)에 흰색으로 출력
    kInternalDrawPixel( &stScreenArea, pstVideoMemory, 1, 20, RGB( 255, 255, 255 ) );
    kInternalDrawPixel( &stScreenArea, pstVideoMemory, 2, 20, RGB( 255, 255, 255 ) );

    // (0, 25)에 Line이란 문자열을 검은색 바탕에 빨간색으로 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, 0, 25, RGB( 255, 0, 0), RGB( 0, 0, 0 ), vpcString[ 1 ], kStrLen( vpcString[ 1 ] ) );
    // (20, 50)을 중심으로 (1000, 50) (1000, 100), (1000, 150), (1000, 200), 
    // (1000, 250)까지 빨간색으로 출력
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 50, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 100, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 150, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 200, RGB( 255, 0, 0 ) );
    kInternalDrawLine( &stScreenArea, pstVideoMemory, 20, 50, 1000, 250, RGB( 255, 0, 0 ) );

    // (0, 180)에 Rectangle이란 문자열을 검은색 바탕에 녹색으로 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, 0, 180, RGB( 0, 255, 0), RGB( 0, 0, 0 ), vpcString[ 2 ], kStrLen( vpcString[ 2 ] ) );
    // (20, 200)에서 시작하여 길이가 각각 50, 100, 150, 200인 사각형을 녹색으로 출력
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 20, 200, 70, 250, RGB( 0, 255, 0 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 120, 200, 220, 300, RGB( 0, 255, 0 ), TRUE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 270, 200, 420, 350, RGB( 0, 255, 0 ), FALSE );
    kInternalDrawRect( &stScreenArea, pstVideoMemory, 470, 200, 670, 400, RGB( 0, 255, 0 ), TRUE );

    // (0, 550)에 Circle이란 문자열을 검은색 바탕에 파란색으로 출력
    kInternalDrawText( &stScreenArea, pstVideoMemory, 0, 550, RGB( 0, 0, 255), RGB( 0, 0, 0 ), vpcString[ 3 ], kStrLen( vpcString[ 3 ] ) );
    // (45, 600)에서 시작하여 반지름이 25, 50, 75, 100인 원을 파란색으로 출력
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 45, 600, 25, RGB( 0, 0, 255 ), FALSE ) ;
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 170, 600, 50, RGB( 0, 0, 255 ), TRUE ) ;
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 345, 600, 75, RGB( 0, 0, 255 ), FALSE ) ;
    kInternalDrawCircle( &stScreenArea, pstVideoMemory, 570, 600, 100, RGB( 0, 0, 255 ), TRUE ) ;

    kGetCh();

    do {

        // 점 그리기
        for( i = 0; i < 100; i++ ) {

            kGetRandomXY( &iX1, &iY1 );
            stColor1 = kGetRandomColor();

            kInternalDrawPixel( &stScreenArea, pstVideoMemory, iX1, iY1, stColor1 );

        }

        // 선 그리기
        for( i = 0; i < 100; i++ ) {

            kGetRandomXY( &iX1, &iY1 );
            kGetRandomXY( &iX2, &iY2 );
            stColor1 = kGetRandomColor();

            kInternalDrawLine( &stScreenArea, pstVideoMemory, iX1, iY1, iX2, iY2, stColor1 );

        }

        // 사각형 그리기
        for( i = 0; i < 20; i++ ) {

            kGetRandomXY( &iX1, &iY1 );
            kGetRandomXY( &iX2, &iY2 );
            stColor1 = kGetRandomColor();

            kInternalDrawRect( &stScreenArea, pstVideoMemory, iX1, iY1, iX2, iY2, stColor1, kRandom() % 2 );

        }

        // 원 그리기
        for( i = 0; i < 100; i++ ) {

            kGetRandomXY( &iX1, &iY1 );
            stColor1 = kGetRandomColor();

            kInternalDrawCircle( &stScreenArea, pstVideoMemory, iX1, iY1, ABS( kRandom() % 50 + 1 ), stColor1, kRandom() % 2 );

        }

        for( i = 0; i < 100; i++ ) {

            kGetRandomXY( &iX1, &iY1 );
            stColor1 = kGetRandomColor();
            stColor2 = kGetRandomColor();

            kInternalDrawText( &stScreenArea, pstVideoMemory, iX1, iY1, stColor1, stColor2, vpcString[ 5 ], kStrLen( vpcString[ 5 ] ) );

        }

    } while( kGetCh() != 'q' );

    while( 1 ) {

        // 배경 출력
        kInternalDrawRect( &stScreenArea, pstVideoMemory, 0, 0, 1024, 768, RGB( 232, 255, 232 ), TRUE );

        for( i = 0; i < 3; i++ ) {

            kGetRandomXY( &iX1, &iY1 );
            kDrawWindowFrame( iX1, iY1, 400, 200, "Mint64 OS Test Window" );

        }

        kGetCh();

    }

}