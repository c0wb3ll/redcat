#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "Types.h"
#include "Synchronization.h"
#include "2DGraphics.h"
#include "List.h"
#include "Queue.h"
#include "Keyboard.h"

// 매크로
#define WINDOW_MAXCOUNT                                 2048
#define GETWINDOWOFFSET( x )                            ( ( x ) & 0xFFFFFFFF )
#define WINDOW_TITLEMAXLENGTH                           40
#define WINDOW_INVALIDID                                0xFFFFFFFFFFFFFFFF

#define WINDOW_FLAGS_SHOW                               0x00000001
#define WINDOW_FLAGS_DRAWFRAME                          0x00000002
#define WINDOW_FLAGS_DRAWTITLE                          0x00000004
#define WINDOW_FLAGS_DEFAULT                            ( WINDOW_FLAGS_SHOW | WINDOW_FLAGS_DRAWFRAME | WINDOW_FLAGS_DRAWTITLE )

#define WINDOW_TITLEBAR_HEIGHT                          21
#define WINDOW_XBUTTON_SIZE                             19

#define WINDOW_COLOR_FRAME                              RGB( 255, 255, 255 )
#define WINDOW_COLOR_BACKGROUND                         RGB( 0, 0, 0 )
#define WINDOW_COLOR_TITLEBARTEXT                       RGB( 0, 0, 0 )
#define WINDOW_COLOR_TITLEBARACTIVEBACKGROUND           RGB( 128, 128, 128 )
#define WINDOW_COLOR_TITLEBARINACTIVEBACKGROUND         RGB( 192, 192, 192 )
#define WINDOW_COLOR_TITLEBARBRIGHT1                    RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARBRIGHT2                    RGB( 255, 255, 255 )
#define WINDOW_COLOR_TITLEBARUNDERLINE                  RGB( 255, 255, 255 )
#define WINDOW_COLOR_BUTTONBRIGHT                       RGB( 255, 255, 255 )
#define WINDOW_COLOR_BUTTONWHITE                        RGB( 255, 255, 255 )
#define WINDOW_COLOR_SYSTEMBACKGROUND                   RGB( 80, 80, 80 )
#define WINDOW_COLOR_XBUTTONLINECOLOR                   RGB( 255, 255, 255 )

#define WINDOW_BACKGROUNDWINDOWTITLE                    "SYS_BACKGROUND"

#define MOUSE_CURSOR_WIDTH                              10
#define MOUSE_CURSOR_HEIGHT                             20

#define MOUSE_CURSOR_OUTERLINE                          RGB( 255, 255, 255 )
#define MOUSE_CURSOR_OUTER                              RGB( 0, 0, 0 )
#define MOUSE_CURSOR_INNER                              RGB( 0, 0, 0 )

#define EVENTQUEUE_WINDOWMAXCOUNT                       100
#define EVENTQUEUE_WINDOWMANAGERMAXCOUNT                WINDOW_MAXCOUNT

#define EVENT_UNKNOWN                                   0
#define EVENT_MOUSE_MOVE                                1
#define EVENT_MOUSE_LBUTTONDOWN                         2
#define EVENT_MOUSE_LBUTTONUP                           3
#define EVENT_MOUSE_RBUTTONDOWN                         4
#define EVENT_MOUSE_RBUTTONUP                           5
#define EVENT_MOUSE_MBUTTONDOWN                         6
#define EVENT_MOUSE_MBUTTONUP                           7

#define EVENT_WINDOW_SELECT                             8
#define EVENT_WINDOW_DESELECT                           9
#define EVENT_WINDOW_MOVE                               10
#define EVENT_WINDOW_RESIZE                             11
#define EVENT_WINDOW_CLOSE                              12

#define EVENT_KEY_DOWN                                  13
#define EVENT_KEY_UP                                    14

#define EVENT_WINDOWMANAGER_UPDATESCREENBYID            15
#define EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA    16
#define EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA    17

// 구조체
// 마우스 이벤트 자료구조
typedef struct kMouseEventStruct {

    QWORD qwWindowID;

    POINT stPoint;
    BYTE bButtonStatus;

} MOUSEEVENT;

// 키 이벤트 자료구조
typedef struct kKeyEventStruct {

    QWORD qwWindowID;

    BYTE bASCIICode;
    BYTE bScanCode;

    BYTE bFlags;

} KEYEVENT;

// 윈도우 이벤트 자료구조
typedef struct kWindowEventStruct {

    QWORD qwWindowID;

    RECT stArea;

} WINDOWEVENT;

// 이벤트 자료구조
typedef struct kEventStruct {

    QWORD qwType;

    union {

        MOUSEEVENT stMouseEvent;

        KEYEVENT stKeyEvent;

        WINDOWEVENT stWindowEvent;

        QWORD vqwData[ 3 ];

    };

} EVENT;

// 윈도우의 정보를 저장하는 자료구조
typedef struct kWindowStruct {

    LISTLINK stLink;

    MUTEX stLock;

    RECT stArea;

    COLOR* pstWindowBuffer;

    QWORD qwTaskID;

    DWORD dwFlags;

    QUEUE stEventQueue;
    EVENT* pstEventBuffer;

    char vcWindowTitle[ WINDOW_TITLEMAXLENGTH + 1 ];

} WINDOW;

//윈도우 풀의 상태를 관리하는 자료구조
typedef struct kWindowPoolManagerStruct {

    MUTEX stLock;

    WINDOW* pstStartAddress;
    int iMaxCount;
    int iUseCount;

    int iAllocatedCount;

} WINDOWPOOLMANAGER;

// 윈도우 매니저 자료구조
typedef struct kWindowManagerStruct {

    MUTEX stLock;

    LIST stWindowList;

    int iMouseX;
    int iMouseY;

    RECT stScreenArea;

    COLOR* pstVideoMemory;

    QWORD qwBackgroundWindowID;

    QUEUE stEventQueue;
    EVENT* pstEventBuffer;

    BYTE bPreviousButtonStatus;

    QWORD qwMovingWindowID;
    BOOL bWindowMoveMode;

} WINDOWMANAGER;

// 함수
static void kInitializeWindowPool( void );
static WINDOW* kAllocateWindow( void );
static void kFreeWindow( QWORD qwID );

// 윈도우와 윈도우 매니저 관련
void kInitializeGUISystem( void );
WINDOWMANAGER* kGetWindowManager( void );
QWORD kGetBackgroundWindowID( void );
void kGetScreenArea( RECT* pstScreenArea );
QWORD kCreateWindow( int iX, int iY, int iWidth, int iHeight, DWORD dwFlags, const char* pcTitle );
BOOL kDeleteWindow( QWORD qwWindowID );
BOOL kDeleteAllWindowInTaskID( QWORD qwTaskID );
WINDOW* kGetWindow( QWORD qwWindowID );
WINDOW* kGetWindowWithWindowLock( QWORD qwWindowID );
BOOL kShowWindow( QWORD qwWindowID, BOOL bShow );
BOOL kRedrawWindowByArea( const RECT* pstArea );
static void kCopyWindowBufferToFrameBuffer( const WINDOW* pstWindow, const RECT* pstCopyArea );
QWORD kFindWindowByPoint( int iX, int iY );
QWORD kFindWindowByTitle( const char* pcTitle );
BOOL kIsWindowExist( QWORD qwWindowID );
QWORD kGetTopWindowID( void );
BOOL kMoveWindowToTop( QWORD qwWindowID );
BOOL kIsInTitleBar( QWORD qwWindowID, int iX, int iY );
BOOL kIsInCloseButton( QWORD qwWindowID, int iX, int iY );
BOOL kMoveWindow( QWORD qwWindowID, int iX, int iY );
static BOOL kUpdateWindowTitle( QWORD qwWindowID, BOOL bSelectedTitle );

BOOL kGetWindowArea( QWORD qwWindowID, RECT* pstArea );
BOOL kConvertPointScreenToClient( QWORD qwWindowID, const POINT* pstXY, POINT* pstXYInWindow );
BOOL kConvertPointClientToScreen( QWORD qwWindowID, const POINT* pstXY, POINT* pstXYInScreen );
BOOL kConvertRectScreenToClient( QWORD qwWindowID, const RECT* pstArea, RECT* pstAreaInWindow );
BOOL kConvertRectClientToScreen( QWORD qwWindowID, const RECT* pstArea, RECT* pstAreaInScreen );

BOOL kUpdateScreenByID( QWORD qwWindowID );
BOOL kUpdateScreenByWindowArea( QWORD qwWindowID, const RECT* pstArea );
BOOL kUpdateScreenByScreenArea( const RECT* pstArea );

BOOL kSendEventToWindow( QWORD qwWindowID, const EVENT* pstEvent );
BOOL kReceiveEventFromWindowQueue( QWORD qwWindowID, EVENT* pstEvent );
BOOL kSendEventToWindowManager( const EVENT* pstEvent );
BOOL kReceiveEventFromWindowManagerQueue( EVENT* pstEvent );
BOOL kSetMouseEvent( QWORD qwWindowID, QWORD qwEventType, int iMouseX, int iMouseY, BYTE bButtonStatus, EVENT* pstEvent );
BOOL kSetWindowEvent( QWORD qwWindowID, QWORD qwEventType, EVENT* pstEvent );
void kSetKeyEvent( QWORD qwWindow, const KEYDATA* pstKeyData, EVENT* pstEvent );

BOOL kDrawWindowFrame( QWORD qwWindowID );
BOOL kDrawWindowBackground( QWORD qwWindowID );
BOOL kDrawWindowTitle( QWORD qwWindowID, const char* pcTitle, BOOL bSelectedTitle );
BOOL kDrawButton( QWORD qwWindowID, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor );
BOOL kDrawPixel( QWORD qwWindowID, int iX, int iY, COLOR stColor );
BOOL kDrawLine( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor );
BOOL kDrawRect( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor, BOOL bFill );
BOOL kDrawCircle( QWORD qwWindowID, int iX, int iY, int iRadius, COLOR stColor, BOOL bFill );
BOOL kDrawText( QWORD qwWindowID, int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, const char* pcString, int iLength );
static void kDrawCursor( int iX, int iY );
void kMoveCursor( int iX, int iY );
void kGetCursorPosition( int* piX, int* piY );

#endif /*__WINDOW_H__*/