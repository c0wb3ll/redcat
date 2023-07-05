#include "Types.h"
#include "Window.h"
#include "WindowManagerTask.h"
#include "VBE.h"
#include "Mouse.h"
#include "Task.h"

// 윈도우 매니저 태스크
void kStartWindowManager( void ) {

    VBEMODEINFOBLOCK* pstVBEMode;
    int iRelativeX, iRelativeY;
    int iMouseX, iMouseY;
    BYTE bButton;
    QWORD qwWindowID;
    TCB* pstTask;
    char vcTempTitle[ WINDOW_TITLEMAXLENGTH ];
    int iWindowCount = 0;

    pstTask = kGetRunningTask( kGetAPICID() );

    kInitializeGUISystem();

    kGetCursorPosition( &iMouseX, &iMouseY );
    kMoveCursor( iMouseX, iMouseY );

    while( 1 ) {

        if( kGetMouseDataFromMouseQueue( &bButton, &iRelativeX, &iRelativeY ) == FALSE ) {

            kSleep( 0 );
            continue;

        }

        kGetCursorPosition( &iMouseX, &iMouseY );

        iMouseX += iRelativeX;
        iMouseY += iRelativeY;

        if( bButton & MOUSE_LBUTTONDOWN ) {

            kSPrintf( vcTempTitle, "MINT64 OS Test Window %d", iWindowCount++ );
            qwWindowID = kCreateWindow( iMouseX - 10, iMouseY - WINDOW_TITLEBAR_HEIGHT / 2, 400, 200, WINDOW_FLAGS_DRAWFRAME | WINDOW_FLAGS_DRAWTITLE, vcTempTitle );

            kDrawText( qwWindowID, 10, WINDOW_TITLEBAR_HEIGHT + 10, RGB( 255, 255, 255 ), WINDOW_COLOR_BACKGROUND, "This is real Window", 19 );
            kDrawText( qwWindowID, 10, WINDOW_TITLEBAR_HEIGHT + 30, RGB( 255, 255, 255 ), WINDOW_COLOR_BACKGROUND, "No more prototype", 17 );
            kShowWindow( qwWindowID, TRUE );

        } else if( bButton & MOUSE_RBUTTONDOWN ) {

            kDeleteAllWindowInTaskID( pstTask->stLink.qwID );
            iWindowCount = 0;

        }

        kMoveCursor( iMouseX, iMouseY );

    }

}