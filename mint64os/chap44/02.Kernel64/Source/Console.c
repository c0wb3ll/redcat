#include <stdarg.h>
#include "Console.h"
#include "Keyboard.h"

CONSOLEMANAGER gs_stConsoleManager = { 0, };

static CHARACTER gs_vstScreenBuffer[ CONSOLE_WIDTH * CONSOLE_HEIGHT ];

static KEYDATA gs_vstKeyQueueBuffer[ CONSOLE_GUIKEYQUEUE_MAXCOUNT ];

void kInitializeConsole( int iX, int iY ) {
        
    kMemSet( &gs_stConsoleManager, 0, sizeof( gs_stConsoleManager ) );
    kMemSet( &gs_vstScreenBuffer, 0, sizeof( gs_vstScreenBuffer ) );

    if( kIsGraphicMode() == FALSE ) {

        gs_stConsoleManager.pstScreenBuffer = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;

    } else {

        gs_stConsoleManager.pstScreenBuffer = gs_vstScreenBuffer;

        kInitializeQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), gs_vstKeyQueueBuffer, CONSOLE_GUIKEYQUEUE_MAXCOUNT, sizeof( KEYDATA ) );
        kInitializeMutex( &( gs_stConsoleManager.stLock ) );

    }

    kSetCursor( iX, iY );

}

void kSetCursor( int iX, int iY ) {

    int iLinearValue;
    int iOldX;
    int iOldY;
    int i;

    iLinearValue = iY * CONSOLE_WIDTH + iX;

    if( kIsGraphicMode() == FALSE ) {

        kOutPortByte( VGA_PORT_INDEX, VGA_INDEX_UPPERCURSOR );
        kOutPortByte( VGA_PORT_DATA, iLinearValue >> 8 );

        kOutPortByte( VGA_PORT_INDEX, VGA_INDEX_LOWERCURSOR );
        kOutPortByte( VGA_PORT_DATA, iLinearValue & 0xFF );

    } else {

        for( i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++ ) {

            if( ( gs_stConsoleManager.pstScreenBuffer[ i ].bCharactor == '_' ) && ( gs_stConsoleManager.pstScreenBuffer[ i ].bAttribute == 0x00 ) ) {

                gs_stConsoleManager.pstScreenBuffer[ i ].bCharactor = ' ';
                gs_stConsoleManager.pstScreenBuffer[ i ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
                break;

            }

        }

        gs_stConsoleManager.pstScreenBuffer[ iLinearValue ].bCharactor = '_';
        gs_stConsoleManager.pstScreenBuffer[ iLinearValue ].bAttribute = 0x00;

    }

    gs_stConsoleManager.iCurrentPrintOffset = iLinearValue;

}

void kGetCursor( int *piX, int *piY ) {

    *piX = gs_stConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
    *piY = gs_stConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;

}

void kPrintf( const char* pcFormatString, ... ) {
    
    va_list ap;
    char vcBuffer[ 1024 ];
    int iNextPrintOffset;

    va_start( ap, pcFormatString );
    kVSPrintf( vcBuffer, pcFormatString, ap );
    va_end( ap );

    iNextPrintOffset = kConsolePrintString( vcBuffer );

    kSetCursor( iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH );

}

int kConsolePrintString( const char* pcBuffer ) {

    CHARACTER* pstScreen;
    int i, j;
    int iLength;
    int iPrintOffset;

    pstScreen = gs_stConsoleManager.pstScreenBuffer;

    iPrintOffset = gs_stConsoleManager.iCurrentPrintOffset;

    iLength = kStrLen( pcBuffer );
    for( i = 0; i < iLength ; i++ ) {

        if( pcBuffer[ i ] == '\n' ) {

            iPrintOffset += ( CONSOLE_WIDTH - ( iPrintOffset % CONSOLE_WIDTH ) );

        } else if( pcBuffer[ i ] == '\t' ) {

            iPrintOffset += ( 8 - ( iPrintOffset % 8 ) );

        } else {

            pstScreen[ iPrintOffset ].bCharactor = pcBuffer[ i ];
            pstScreen[ iPrintOffset ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
            iPrintOffset++;

        }

        if( iPrintOffset >= ( CONSOLE_HEIGHT * CONSOLE_WIDTH ) ) {

            kMemCpy( pstScreen, pstScreen + CONSOLE_WIDTH, ( CONSOLE_HEIGHT - 1 ) * CONSOLE_WIDTH * sizeof( CHARACTER ) );

            for( j = ( CONSOLE_HEIGHT - 1 ) * ( CONSOLE_WIDTH ); j < ( CONSOLE_HEIGHT * CONSOLE_WIDTH ); j++ ) {

                pstScreen[ j ].bCharactor = ' ';
                pstScreen[ j ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;

            }

            iPrintOffset = ( CONSOLE_HEIGHT - 1 ) * CONSOLE_WIDTH;

        }

    }

    return iPrintOffset;

}

void kClearScreen( void ) {

    CHARACTER* pstScreen;
    int i;

    pstScreen = gs_stConsoleManager.pstScreenBuffer;

    for( i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++ ) {

        pstScreen[ i ].bCharactor = ' ';
        pstScreen[ i ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;

    }

    kSetCursor( 0, 0 );

}

BYTE kGetCh( void ) {
    
    KEYDATA stData;

    while( 1 ) { 

        if( kIsGraphicMode() == FALSE ) {

            while( kGetKeyFromKeyQueue( &stData ) == FALSE ) { 
                
                kSchedule(); 
                
            }

        } else {

            while( kGetKeyFromGUIKeyQueue( &stData ) == FALSE ) {

                if( gs_stConsoleManager.bExit == TRUE ) {

                    return 0xFF;

                }

                kSchedule();

            }

        }

        if( stData.bFlags & KEY_FLAGS_DOWN ) { 
            
            return stData.bASCIICode; 
            
        }

    }

}

void kPrintStringXY( int iX, int iY, const char* pcString) {

    CHARACTER* pstScreen;
    int i;

    pstScreen = gs_stConsoleManager.pstScreenBuffer;

    pstScreen += ( iY * CONSOLE_WIDTH ) + iX;
    for( i = 0; pcString[ i ] != 0; i++ ) {

        pstScreen[ i ].bCharactor = pcString[ i ];
        pstScreen[ i ].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;

    }

}

// 콘솔을 관리하는 자료구조를 반환
CONSOLEMANAGER* kGetConsoleManager( void ) {

    return &gs_stConsoleManager;

}

// 그래픽 모드용 키 큐에서 키 데이터를 제거
BOOL kGetKeyFromGUIKeyQueue( KEYDATA* pstData ) {

    BOOL bResult;

    if( kIsQueueEmpty( &( gs_stConsoleManager.stKeyQueueForGUI ) ) == TRUE ) {

        return FALSE;

    }

    kLock( &( gs_stConsoleManager.stLock ) );

    bResult = kGetQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), pstData );

    kUnlock( &( gs_stConsoleManager.stLock ) );

    return bResult;

}

// 그래픽 모드용 키 큐에 데이터를 삽입
BOOL kPutKeyToGUIKeyQueue( KEYDATA* pstData ) {

    BOOL bResult;

    if( kIsQueueFull( &( gs_stConsoleManager.stKeyQueueForGUI ) ) == TRUE ) {

        return FALSE;

    }

    kLock( &( gs_stConsoleManager.stLock ) );

    bResult = kPutQueue( &( gs_stConsoleManager.stKeyQueueForGUI ), pstData );

    kUnlock( &( gs_stConsoleManager.stLock ) );

    return bResult;

}

// 콘솔 셸 태스크 종료 플래그를 설정
void kSetConsoleShellExitFlag( BOOL bFlag ) {

    gs_stConsoleManager.bExit = bFlag;
    
}