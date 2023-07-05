#include "Window.h"
#include "VBE.h"
#include "Task.h"
#include "Font.h"
#include "DynamicMemory.h"

static WINDOWPOOLMANAGER gs_stWindowPoolManager;
static WINDOWMANAGER gs_stWindowManager;

static void kInitializeWindowPool( void ) {

    int i;
    void* pvWindowPoolAddress;
    kMemSet( &gs_stWindowPoolManager, 0, sizeof( gs_stWindowPoolManager ) );

    pvWindowPoolAddress = ( void* ) kAllocateMemory( sizeof( WINDOW ) * WINDOW_MAXCOUNT );
    if( pvWindowPoolAddress == NULL ) {

        kPrintf( "Window Pool Allocate Fail\n" );
        while( 1 ) {

            ;

        }

    }

    gs_stWindowPoolManager.pstStartAddress = ( WINDOW* ) pvWindowPoolAddress;
    kMemSet( pvWindowPoolAddress, 0, sizeof( WINDOW ) * WINDOW_MAXCOUNT );

    for( i = 0; i < WINDOW_MAXCOUNT; i++ ) {

        gs_stWindowPoolManager.pstStartAddress[ i ].stLink.qwID = i;

    }
    
    gs_stWindowPoolManager.iMaxCount = WINDOW_MAXCOUNT;
    gs_stWindowPoolManager.iAllocatedCount = 1;

    kInitializeMutex( &( gs_stWindowPoolManager.stLock ) );

}

// 윈도우 자료구조를 할당
static WINDOW* kAllocateWindow( void ) {

    WINDOW* pstEmptyWindow;
    int i;

    kLock( &( gs_stWindowPoolManager.stLock ) );

    if( gs_stWindowPoolManager.iUseCount == gs_stWindowPoolManager.iMaxCount ) {

        kUnlock( &gs_stWindowPoolManager.stLock );

        return NULL;

    }

    for( i = 0; i < gs_stWindowPoolManager.iMaxCount; i++ ) {

        if( ( gs_stWindowPoolManager.pstStartAddress[ i ].stLink.qwID >> 32 ) == 0 ) {

            pstEmptyWindow = &( gs_stWindowPoolManager.pstStartAddress[ i ] );
            break;

        }

    }

    pstEmptyWindow->stLink.qwID = ( ( QWORD ) gs_stWindowPoolManager.iAllocatedCount << 32 ) | i;

    gs_stWindowPoolManager.iUseCount++;
    gs_stWindowPoolManager.iAllocatedCount++;
    if( gs_stWindowPoolManager.iAllocatedCount == 0 ) {

        gs_stWindowPoolManager.iAllocatedCount = 1;

    }

    kUnlock( &( gs_stWindowPoolManager.stLock ) );

    kInitializeMutex( &( pstEmptyWindow->stLock ) );

    return pstEmptyWindow;

}

// 윈도우 자료구조 해제
static void kFreeWindow( QWORD qwID ) {

    int i;

    i = GETWINDOWOFFSET( qwID );

    kLock( &( gs_stWindowPoolManager.stLock ) );
    kMemSet( &( gs_stWindowPoolManager.pstStartAddress[ i ] ), 0, sizeof( WINDOW ) );
    gs_stWindowPoolManager.pstStartAddress[ i ].stLink.qwID = i;

    gs_stWindowPoolManager.iUseCount--;

    kUnlock( &( gs_stWindowPoolManager.stLock ) );

}

// GUI 시스템 초기화
void kInitializeGUISystem( void ) {

    VBEMODEINFOBLOCK* pstModeInfo;
    QWORD qwBackgroundWindowID;

    kInitializeWindowPool();

    pstModeInfo = kGetVBEModeInfoBlock();

    gs_stWindowManager.pstVideoMemory = ( COLOR* ) ( ( QWORD ) pstModeInfo->dwPhysicalBasePointer & 0xFFFFFFFF );

    gs_stWindowManager.iMouseX = pstModeInfo->wXResolution / 2;
    gs_stWindowManager.iMouseY = pstModeInfo->wYResolution / 2;

    gs_stWindowManager.stScreenArea.iX1 = 0;
    gs_stWindowManager.stScreenArea.iY1 = 0;
    gs_stWindowManager.stScreenArea.iX2 = pstModeInfo->wXResolution - 1;
    gs_stWindowManager.stScreenArea.iY2 = pstModeInfo->wYResolution - 1;

    kInitializeMutex( &( gs_stWindowManager.stLock ) );

    kInitializeList( &( gs_stWindowManager.stWindowList ) );

    qwBackgroundWindowID = kCreateWindow( 0, 0, pstModeInfo->wXResolution, pstModeInfo->wYResolution, 0, WINDOW_BACKGROUNDWINDOWTITLE );
    gs_stWindowManager.qwBackgroundWindowID = qwBackgroundWindowID;
    kDrawRect( qwBackgroundWindowID, 0, 0, pstModeInfo->wXResolution - 1, pstModeInfo->wYResolution - 1, WINDOW_COLOR_SYSTEMBACKGROUND, TRUE );
    kShowWindow( qwBackgroundWindowID, TRUE );

}

// 윈도우 매니저를 반환
WINDOWMANAGER* kGetWindowManager( void ) {

    return &gs_stWindowManager;

}

// 배경 윈도우의 ID를 반환
QWORD kGetBackgroundWindowID( void ) {

    return gs_stWindowManager.qwBackgroundWindowID;

}

// 화면 영역의 크기를 반환
void kGetScreenArea( RECT* pstScreenArea ) {

    kMemCpy( pstScreenArea, &( gs_stWindowManager.stScreenArea ), sizeof( RECT ) );

}
// 윈도우를 생성
QWORD kCreateWindow( int iX, int iY, int iWidth, int iHeight, DWORD dwFlags, const char* pcTitle ) {

    WINDOW* pstWindow;
    TCB* pstTask;

    if( ( iWidth <= 0 ) || ( iHeight <= 0 ) ) {

        return WINDOW_INVALIDID;

    }

    pstWindow = kAllocateWindow();
    if( pstWindow == NULL ) {

        return WINDOW_INVALIDID;

    }

    pstWindow->stArea.iX1 = iX;
    pstWindow->stArea.iY1 = iY;
    pstWindow->stArea.iX2 = iX + iWidth - 1;
    pstWindow->stArea.iY2 = iY + iHeight - 1;

    kMemCpy( pstWindow->vcWindowTitle, pcTitle, WINDOW_TITLEMAXLENGTH );
    pstWindow->vcWindowTitle[ WINDOW_TITLEMAXLENGTH ] = '\0';

    pstWindow->pstWindowBuffer = ( COLOR* ) kAllocateMemory( iWidth * iHeight * sizeof( COLOR ) );
    if( pstWindow == NULL ) {

        kFreeWindow( pstWindow->stLink.qwID );
        return WINDOW_INVALIDID;

    }

    pstTask = kGetRunningTask( kGetAPICID() );
    pstWindow->qwTaskID = pstTask->stLink.qwID;

    pstWindow->dwFlags = dwFlags;

    kDrawWindowBackground( pstWindow->stLink.qwID );

    if( dwFlags & WINDOW_FLAGS_DRAWFRAME ) {

        kDrawWindowFrame( pstWindow->stLink.qwID );

    }

    if( dwFlags & WINDOW_FLAGS_DRAWTITLE ) {

        kDrawWindowTitle( pstWindow->stLink.qwID, pcTitle );

    }

    kLock( &( gs_stWindowManager.stLock ) );

    kAddListToTail( &gs_stWindowManager.stWindowList, pstWindow );

    kUnlock( &( gs_stWindowManager.stLock ) );

    if( dwFlags & WINDOW_FLAGS_SHOW ) {

        kRedrawWindowByArea( &( pstWindow->stArea ) );

    }

    return pstWindow->stLink.qwID;

}

// 윈도우를 삭제
BOOL kDeleteWindow( QWORD qwWindowID ) {

    WINDOW* pstWindow;
    RECT stArea;

    kLock( &( gs_stWindowManager.stLock ) );

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        kUnlock( &( gs_stWindowManager.stLock ) );
        return FALSE;

    }

    kMemCpy( &stArea, &( pstWindow->stArea ), sizeof( RECT ) );

    if( kRemoveList( &( gs_stWindowManager.stWindowList ), qwWindowID ) == NULL ) {

        kUnlock( &( pstWindow->stLock ) );
        kUnlock( &( gs_stWindowManager.stLock ) );
        return FALSE;

    }

    kLock( &( pstWindow->stLock ) );

    kFreeMemory( pstWindow->pstWindowBuffer );

    kUnlock( &( pstWindow->stLock ) );

    kFreeWindow( qwWindowID );

    kUnlock( &( gs_stWindowManager.stLock ) );

    kRedrawWindowByArea( &stArea );

    return TRUE;

}

// 태스크 ID가 일치하는 모든 윈도우를 삭제
BOOL kDeleteAllWindowInTaskID( QWORD qwTaskID ) {

    WINDOW* pstWindow;
    WINDOW* pstNextWindow;

    kLock( &( gs_stWindowManager.stLock ) );

    pstWindow = kGetHeaderFromList( &( gs_stWindowManager.stWindowList ) );
    while( pstWindow != NULL ) {

        pstNextWindow = kGetNextFromList( &( gs_stWindowManager.stWindowList ), pstWindow );

        if( ( pstWindow->stLink.qwID != gs_stWindowManager.qwBackgroundWindowID ) && ( pstWindow->qwTaskID == qwTaskID ) ) {

            kDeleteWindow( pstWindow->stLink.qwID );

        }

        pstWindow = pstNextWindow;

    }

    kUnlock( &( gs_stWindowManager.stLock ) );

}

// 윈도우 ID로 윈도우 포인터를 반환
WINDOW* kGetWindow( QWORD qwWindowID ) {

    WINDOW* pstWindow;

    if( GETWINDOWOFFSET( qwWindowID ) >= WINDOW_MAXCOUNT ) {

        return NULL;

    }

    pstWindow = &gs_stWindowPoolManager.pstStartAddress[ GETWINDOWOFFSET( qwWindowID ) ];
    if( pstWindow->stLink.qwID == qwWindowID ) {

        return pstWindow;

    }

    return NULL;

}

// 윈도우 ID로 윈도우 포인터를 찾아 윈도우 뮤텍스를 잠근 뒤 반환
WINDOW* kGetWindowWithWindowLock( QWORD qwWindowID ) {

    WINDOW* pstWindow;
    BOOL bResult;

    pstWindow = kGetWindow( qwWindowID );
    if( pstWindow == NULL ) {

        return NULL;

    }

    kLock( &( pstWindow->stLock ) );
    pstWindow = kGetWindow( qwWindowID );
    if( pstWindow == NULL ) {

        kUnlock( &( pstWindow->stLock ) );
        return NULL;

    }

    return pstWindow;

}

// 윈도우를 화면에 나타내거나 숨김
BOOL kShowWindow( QWORD qwWindowID, BOOL bShow ) {

    WINDOW* pstWindow;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    if( bShow == TRUE ) {

        pstWindow->dwFlags |= WINDOW_FLAGS_SHOW;

    } else {

        pstWindow->dwFlags &= ~WINDOW_FLAGS_SHOW;

    }

    kUnlock( &( pstWindow->stLock ) );

    kRedrawWindowByArea( &( pstWindow->stArea ) );

    return TRUE;

}

// 특정 영역을 포함하는 윈도우는 모두 그림
BOOL kRedrawWindowByArea( const RECT* pstArea ) {

    WINDOW* pstWindow;
    WINDOW* pstTargetWindow = NULL;
    RECT stOverlappedArea;
    RECT stCursorArea;

    if( kGetOverlappedRectangle( &( gs_stWindowManager.stScreenArea ), pstArea, &stOverlappedArea ) == FALSE ) {

        return FALSE;

    }

    kLock( &( gs_stWindowManager.stLock ) );

    pstWindow = kGetHeaderFromList( &( gs_stWindowManager.stWindowList ) );
    while( pstWindow != NULL ) {

        if( ( pstWindow->dwFlags & WINDOW_FLAGS_SHOW ) && ( kIsRectangleOverlapped( &( pstWindow->stArea ), &stOverlappedArea ) == TRUE ) ) {

            kLock( &( pstWindow->stLock ) );

            kCopyWindowBufferToFrameBuffer( pstWindow, &stOverlappedArea );

            kUnlock( &( pstWindow->stLock ) );

        }

        pstWindow = kGetNextFromList( &( gs_stWindowManager.stWindowList ), pstWindow );

    }

    kUnlock( &( gs_stWindowManager.stLock ) );

    kSetRectangleData( gs_stWindowManager.iMouseX, gs_stWindowManager.iMouseY, gs_stWindowManager.iMouseX + MOUSE_CURSOR_WIDTH, gs_stWindowManager.iMouseY + MOUSE_CURSOR_HEIGHT, &stCursorArea );

    if( kIsRectangleOverlapped( &stOverlappedArea, &stCursorArea ) == TRUE ) {

        kDrawCursor( gs_stWindowManager.iMouseX, gs_stWindowManager.iMouseY );

    }

}

// 윈도우 화면 버퍼의 일부 또는 전체를 프레임 버퍼로 복사
static void kCopyWindowBufferToFrameBuffer( const WINDOW* pstWindow, const RECT* pstCopyArea ) {

    RECT stTempArea;
    RECT stOverlappedArea;
    int iOverlappedWidth;
    int iOverlappedHeight;
    int iScreenWidth;
    int iWindowWidth;
    int i;
    COLOR* pstCurrentVideoMemoryAddress;
    COLOR* pstCurrentWindowBufferAddress;

    if( kGetOverlappedRectangle( &( gs_stWindowManager.stScreenArea ), pstCopyArea, &stTempArea ) == FALSE ) {
        
        return ;

    }

    if( kGetOverlappedRectangle( &stTempArea, &( pstWindow->stArea ), &stOverlappedArea ) == FALSE ) {

        return ;

    }

    iScreenWidth = kGetRectangleWidth( &( gs_stWindowManager.stScreenArea ) );
    iWindowWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iOverlappedWidth = kGetRectangleWidth( &stOverlappedArea );
    iOverlappedHeight = kGetRectangleHeight( &stOverlappedArea );

    pstCurrentVideoMemoryAddress = gs_stWindowManager.pstVideoMemory + stOverlappedArea.iY1 * iScreenWidth + stOverlappedArea.iX1;

    pstCurrentWindowBufferAddress = pstWindow->pstWindowBuffer + ( stOverlappedArea.iY1 - pstWindow->stArea.iY1 ) * iWindowWidth + ( stOverlappedArea.iX1 - pstWindow->stArea.iX1 );

    for( i = 0; i < iOverlappedHeight; i++ ) {

        kMemCpy( pstCurrentVideoMemoryAddress, pstCurrentWindowBufferAddress, iOverlappedWidth * sizeof( COLOR ) );

        pstCurrentVideoMemoryAddress += iScreenWidth;
        pstCurrentWindowBufferAddress += iWindowWidth;

    }

}

// 윈도우 화면 버퍼에 윈도우 테두리 그리기
BOOL kDrawWindowFrame( QWORD qwWindowID ) {

    WINDOW* pstWindow;
    RECT stArea;
    int iWidth;
    int iHeight;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    iWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iHeight = kGetRectangleHeight( &( pstWindow->stArea ) );

    kSetRectangleData( 0, 0, iWidth - 1, iHeight - 1, &stArea );

    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer, 0, 0, iWidth - 1, iHeight - 1, WINDOW_COLOR_FRAME, FALSE );
    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer, 1, 1, iWidth - 2, iHeight - 2, WINDOW_COLOR_FRAME, FALSE );

    kUnlock( &( pstWindow->stLock ) );

    return TRUE;

}

// 윈도우 화면 버퍼에 배경 그리기
BOOL kDrawWindowBackground( QWORD qwWindowID ) {

    WINDOW* pstWindow;
    int iWidth;
    int iHeight;
    RECT stArea;
    int iX;
    int iY;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    iWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iHeight = kGetRectangleHeight( &( pstWindow->stArea ) );

    kSetRectangleData( 0, 0, iWidth - 1, iHeight - 1, &stArea );

    if( pstWindow->dwFlags & WINDOW_FLAGS_DRAWTITLE ) {

        iY = WINDOW_TITLEBAR_HEIGHT;

    } else {

        iY = 0;

    }

    if( pstWindow->dwFlags & WINDOW_FLAGS_DRAWFRAME ) {

        iX = 2;

    } else {

        iX = 0;

    }

    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer, iX, iY, iWidth - 1 -iX, iHeight - 1 - iX, WINDOW_COLOR_BACKGROUND, TRUE );

    kUnlock( &( pstWindow->stLock ) );

    return TRUE;

}

// 윈도우 화면 버퍼에 윈도우 제목 표시줄 그리기
BOOL kDrawWindowTitle( QWORD qwWindowID, const char* pcTitle ) {

    WINDOW* pstWindow;
    int iWidth;
    int iHeight;
    int iX;
    int iY;
    RECT stArea;
    RECT stButtonArea;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    iWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iHeight = kGetRectangleHeight( &( pstWindow->stArea ) );

    kSetRectangleData( 0, 0, iWidth - 1, iHeight - 1, &stArea );

    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer, 0, 3, iWidth - 1, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_COLOR_TITLEBARBACKGROUND, TRUE );

    kInternalDrawText( &stArea, pstWindow->pstWindowBuffer, 6, 3, WINDOW_COLOR_TITLEBARTEXT, WINDOW_COLOR_TITLEBARBACKGROUND, pcTitle, kStrLen( pcTitle ) );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, 1, 1, iWidth - 1, 1, WINDOW_COLOR_TITLEBARBRIGHT1 );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, 1, 2, iWidth - 1, 2, WINDOW_COLOR_TITLEBARBRIGHT2 );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, 1, 2, 1, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_COLOR_TITLEBARBRIGHT1 );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, 2, 2, 2, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_COLOR_TITLEBARBRIGHT2 );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, 2, WINDOW_TITLEBAR_HEIGHT - 2, iWidth - 2, WINDOW_TITLEBAR_HEIGHT - 2, WINDOW_COLOR_TITLEBARUNDERLINE );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, 2, WINDOW_TITLEBAR_HEIGHT - 1, iWidth - 2, WINDOW_TITLEBAR_HEIGHT - 1, WINDOW_COLOR_TITLEBARUNDERLINE );

    kUnlock( &( pstWindow->stLock ) );

    stButtonArea.iX1 = iWidth - WINDOW_XBUTTON_SIZE - 1;
    stButtonArea.iY1 = 1;
    stButtonArea.iX2 = iWidth - 2;
    stButtonArea.iY2 = WINDOW_XBUTTON_SIZE - 1;
    kDrawButton( qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND, "", WINDOW_COLOR_BACKGROUND );

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, iWidth - 2 - 18 + 4, 1 + 4, iWidth - 2 - 4, WINDOW_TITLEBAR_HEIGHT - 6, WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, iWidth - 2 - 18 + 5, 1 + 4, iWidth - 2 - 4, WINDOW_TITLEBAR_HEIGHT - 7, WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, iWidth - 2 - 18 + 4, 1 + 5, iWidth - 2 - 5, WINDOW_TITLEBAR_HEIGHT - 6, WINDOW_COLOR_XBUTTONLINECOLOR );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, iWidth - 2 - 18 + 4, 19 - 4, iWidth - 2 - 4, 1 + 4, WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, iWidth - 2 - 18 + 5, 19 - 4, iWidth - 2 - 4, 1 + 5, WINDOW_COLOR_XBUTTONLINECOLOR );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, iWidth - 2 - 18 + 4, 19 - 5, iWidth - 2 - 5, 1 + 4, WINDOW_COLOR_XBUTTONLINECOLOR );

    kUnlock( &( pstWindow->stLock ) );

    return TRUE;

}

// 윈도우 내부에 버튼 그리기
BOOL kDrawButton( QWORD qwWindowID, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor ) {

    WINDOW* pstWindow;
    RECT stArea;
    int iWindowWidth;
    int iWindowHeight;
    int iTextLength;
    int iTextWidth;
    int iButtonWidth;
    int iButtonHeight;
    int iTextX;
    int iTextY;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    iWindowWidth = kGetRectangleWidth( &( pstWindow->stArea ) );
    iWindowHeight = kGetRectangleHeight( &( pstWindow->stArea ) );
    kSetRectangleData( 0, 0, iWindowWidth - 1, iWindowHeight - 1, &stArea );

    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer, pstButtonArea->iX1, pstButtonArea->iY1, pstButtonArea->iX2, pstButtonArea->iY2, stBackgroundColor, TRUE );

    iButtonWidth = kGetRectangleWidth( pstButtonArea );
    iButtonHeight = kGetRectangleHeight( pstButtonArea );
    iTextLength = kStrLen( pcText );
    iTextWidth = iTextLength * FONT_ENGLISHWIDTH;

    iTextX = ( pstButtonArea->iX1 + iButtonWidth / 2 ) - iTextWidth / 2;
    iTextY = ( pstButtonArea->iY1 + iButtonHeight / 2 ) - FONT_ENGLISHHEIGHT / 2;
    kInternalDrawText( &stArea, pstWindow->pstWindowBuffer, iTextX, iTextY, stTextColor, stBackgroundColor, pcText, iTextLength );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, pstButtonArea->iX1, pstButtonArea->iY1, pstButtonArea->iX2, pstButtonArea->iY1, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, pstButtonArea->iX1, pstButtonArea->iY1 + 1, pstButtonArea->iX2 - 1, pstButtonArea->iY1 + 1, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, pstButtonArea->iX1, pstButtonArea->iY1, pstButtonArea->iX1, pstButtonArea->iY2, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, pstButtonArea->iX1 + 1, pstButtonArea->iY1, pstButtonArea->iX1 + 1, pstButtonArea->iY2 - 1, WINDOW_COLOR_BUTTONBRIGHT );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, pstButtonArea->iX1 + 1, pstButtonArea->iY2, pstButtonArea->iX2, pstButtonArea->iY2, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, pstButtonArea->iX1 + 2, pstButtonArea->iY2 - 1, pstButtonArea->iX2, pstButtonArea->iY2 - 1, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, pstButtonArea->iX2, pstButtonArea->iY1 + 1, pstButtonArea->iX2, pstButtonArea->iY2, WINDOW_COLOR_BUTTONBRIGHT );
    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, pstButtonArea->iX2 - 1, pstButtonArea->iY1 + 2, pstButtonArea->iX2 - 1, pstButtonArea->iY2, WINDOW_COLOR_BUTTONBRIGHT );

    kUnlock( &( pstWindow->stLock ) );

    return TRUE;

}

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

static void kDrawCursor( int iX, int iY ) {

    int i;
    int j;
    BYTE* pbCurrentPos;

    pbCurrentPos = gs_vwMouseBuffer;

    for( j = 0; j < MOUSE_CURSOR_HEIGHT; j++ ) {

        for( i = 0; i < MOUSE_CURSOR_WIDTH; i++ ) {

            switch( *pbCurrentPos ) {

            case 0:
                break;

            case 1:
                kInternalDrawPixel( &( gs_stWindowManager.stScreenArea ), gs_stWindowManager.pstVideoMemory, i + iX, j + iY, MOUSE_CURSOR_OUTERLINE );
                break;

            case 2:
                kInternalDrawPixel( &( gs_stWindowManager.stScreenArea ), gs_stWindowManager.pstVideoMemory, i + iX, j + iY, MOUSE_CURSOR_OUTER );
                break;

            case 3:
                kInternalDrawPixel( &( gs_stWindowManager.stScreenArea ), gs_stWindowManager.pstVideoMemory, i + iX, j + iY, MOUSE_CURSOR_INNER );
                break;

            }

            pbCurrentPos++;

        }

    }

}

// 마우스 커서를 해당 위치로 이동해서 그려줌
void kMoveCursor( int iX, int iY ) {

    RECT stPreviousArea;

    if( iX < gs_stWindowManager.stScreenArea.iX1 ) {

        iX = gs_stWindowManager.stScreenArea.iX1;

    } else if( iX > gs_stWindowManager.stScreenArea.iX2 ) {

        iX = gs_stWindowManager.stScreenArea.iX2;

    }

    if( iY < gs_stWindowManager.stScreenArea.iY1 ) {

        iY = gs_stWindowManager.stScreenArea.iY1;

    } else if( iY > gs_stWindowManager.stScreenArea.iY2 ) {

        iY = gs_stWindowManager.stScreenArea.iY2;

    }

    kLock( &( gs_stWindowManager.stLock ) );

    stPreviousArea.iX1 = gs_stWindowManager.iMouseX;
    stPreviousArea.iY1 = gs_stWindowManager.iMouseY;
    stPreviousArea.iX2 = gs_stWindowManager.iMouseX + MOUSE_CURSOR_WIDTH - 1;
    stPreviousArea.iY2 = gs_stWindowManager.iMouseY + MOUSE_CURSOR_HEIGHT - 1;

    gs_stWindowManager.iMouseX = iX;
    gs_stWindowManager.iMouseY = iY;

    kUnlock( &( gs_stWindowManager.stLock ) );

    kRedrawWindowByArea( &stPreviousArea );

    kDrawCursor( iX, iY );

}

// 현재 마우스 커서의 위치를 반환
void kGetCursorPosition( int* piX, int* piY ) {

    *piX = gs_stWindowManager.iMouseX;
    *piY = gs_stWindowManager.iMouseY;

}

// 윈도우 내부에 점 그리기
BOOL kDrawPixel( QWORD qwWindowID, int iX, int iY, COLOR stColor ) {

    WINDOW* pstWindow;
    RECT stArea;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );

    kInternalDrawPixel( &stArea, pstWindow->pstWindowBuffer, iX, iY, stColor );

    kUnlock( &pstWindow->stLock );

    return TRUE;

}

BOOL kDrawLine( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor ) {

    WINDOW* pstWindow;
    RECT stArea;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );

    kInternalDrawLine( &stArea, pstWindow->pstWindowBuffer, iX1, iY1, iX2, iY2, stColor );

    kUnlock( &pstWindow->stLock );

    return TRUE;

}

// 윈도우 내부에 사각형 그리기
BOOL kDrawRect( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor, BOOL bFill ) {

    WINDOW* pstWindow;
    RECT stArea;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );

    kInternalDrawRect( &stArea, pstWindow->pstWindowBuffer, iX1, iY1, iX2, iY2, stColor, bFill );

    kUnlock( &pstWindow->stLock );

    return TRUE;

}

// 윈도우 내부에 원 그리기
BOOL kDrawCircle( QWORD qwWindowID, int iX, int iY, int iRadius, COLOR stColor, BOOL bFill ) {

    WINDOW* pstWindow;
    RECT stArea;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );

    kInternalDrawCircle( &stArea, pstWindow->pstWindowBuffer, iX, iY, iRadius, stColor, bFill );

    kUnlock( &pstWindow->stLock );
    
    return TRUE;

}

// 윈도우 내부에 문자 출력
BOOL kDrawText( QWORD qwWindowID, int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, const char* pcString, int iLength ) {

    WINDOW* pstWindow;
    RECT stArea;

    pstWindow = kGetWindowWithWindowLock( qwWindowID );
    if( pstWindow == NULL ) {

        return FALSE;

    }

    kSetRectangleData( 0, 0, pstWindow->stArea.iX2 - pstWindow->stArea.iX1, pstWindow->stArea.iY2 - pstWindow->stArea.iY1, &stArea );

    kInternalDrawText( &stArea, pstWindow->pstWindowBuffer, iX, iY, stTextColor, stBackgroundColor, pcString, iLength );

    kUnlock( &pstWindow->stLock );

    return TRUE;

}