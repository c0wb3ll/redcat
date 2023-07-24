#include "Types.h"
#include "Window.h"
#include "WindowManagerTask.h"
#include "VBE.h"
#include "Mouse.h"
#include "Task.h"
#include "GUITask.h"
#include "Font.h"

// 윈도우 매니저 태스크
void kStartWindowManager( void ) {

    int iMouseX, iMouseY;
    BOOL bMouseDataResult;
    BOOL bKeyDataResult;
    BOOL bEventQueueResult;

    QWORD qwLastTickCount;
    QWORD qwPreviousLoopExecutionCount;
    QWORD qwLoopExecutionCount;
    QWORD qwMinLoopExecutionCount;
    char vcTemp[ 40 ];
    RECT stLoopCountArea;
    QWORD qwBackgroundWindowID;

    kInitializeGUISystem();

    kGetCursorPosition( &iMouseX, &iMouseY );
    kMoveCursor( iMouseX, iMouseY );

    qwLastTickCount = kGetTickCount();
    qwPreviousLoopExecutionCount = 0;
    qwLoopExecutionCount = 0;
    qwMinLoopExecutionCount = 0xFFFFFFFFFFFFFFFF;
    qwBackgroundWindowID = kGetBackgroundWindowID();

    while( 1 ) {

        if( kGetTickCount() - qwLastTickCount > 1000 ) {

            qwLastTickCount = kGetTickCount();

            if( ( qwLoopExecutionCount - qwPreviousLoopExecutionCount ) < qwMinLoopExecutionCount ) {

                qwMinLoopExecutionCount = qwLoopExecutionCount - qwPreviousLoopExecutionCount;

            }

            qwPreviousLoopExecutionCount = qwLoopExecutionCount;
            
            kSPrintf( vcTemp, "MIN Loop Execution Count:%d ", qwMinLoopExecutionCount );
            kDrawText( qwBackgroundWindowID, 0, 0, RGB( 255, 255, 255 ), RGB( 0, 0, 0 ), vcTemp, kStrLen( vcTemp ) );

            kSetRectangleData( 0, 0, kStrLen( vcTemp ) * FONT_ENGLISHWIDTH, FONT_ENGLISHHEIGHT, &stLoopCountArea );
            kRedrawWindowByArea( &stLoopCountArea, qwBackgroundWindowID );

        }

        qwLoopExecutionCount++;

        bMouseDataResult = kProcessMouseData();

        bKeyDataResult = kProcessKeyData();

        bEventQueueResult = FALSE;
        while( kProcessEventQueueData() == TRUE ) {

            bEventQueueResult = TRUE;

        }

        if( ( bMouseDataResult == FALSE ) && ( bKeyDataResult == FALSE ) && ( bEventQueueResult == FALSE ) ) {

            kSleep( 0 );

        }

    }

}

// 수신된 마우스 데이터 처리
BOOL kProcessMouseData( void ) {

    QWORD qwWindowIDUnderMouse;
    BYTE bButtonStatus;
    int iRelativeX, iRelativeY;
    int iMouseX, iMouseY;
    int iPreviousMouseX, iPreviousMouseY;
    BYTE bChangedButton;
    RECT stWindowArea;
    EVENT stEvent;
    WINDOWMANAGER* pstWindowManager;
    char vcTempTitle[ WINDOW_TITLEMAXLENGTH ];
    int i;

    pstWindowManager = kGetWindowManager();

    for( i = 0; i < WINDOWMANAGER_DATAACCUMULATECOUNT; i++ ) {

        if( kGetMouseDataFromMouseQueue( &bButtonStatus, &iRelativeX, &iRelativeY ) == FALSE ) {

            if( i == 0 ) {

                return FALSE;

            } else {

                break;

            }

        }

        kGetCursorPosition( &iMouseX, &iMouseY );

        if( i == 0 ) {

            iPreviousMouseX = iMouseX;
            iPreviousMouseY = iMouseY;

        }

        iMouseX += iRelativeX;
        iMouseY += iRelativeY;

        kMoveCursor( iMouseX, iMouseY );
        kGetCursorPosition( &iMouseX, &iMouseY );

        bChangedButton = pstWindowManager->bPreviousButtonStatus ^ bButtonStatus;

        if( bChangedButton != 0 ) {

            break;

        }

    }

    qwWindowIDUnderMouse = kFindWindowByPoint( iMouseX, iMouseY );

    if( bChangedButton & MOUSE_LBUTTONDOWN ) {

        if( bButtonStatus & MOUSE_LBUTTONDOWN ) {

            if( qwWindowIDUnderMouse != pstWindowManager->qwBackgroundWindowID ) {

                kMoveWindowToTop( qwWindowIDUnderMouse );

            }

            if( kIsInTitleBar( qwWindowIDUnderMouse, iMouseX, iMouseY ) == TRUE ) {

                if( kIsInCloseButton( qwWindowIDUnderMouse, iMouseX, iMouseY ) == TRUE ) {

                    kSetWindowEvent( qwWindowIDUnderMouse, EVENT_WINDOW_CLOSE, &stEvent );
                    kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );

                } else {

                    pstWindowManager->bWindowMoveMode = TRUE;

                    pstWindowManager->qwMovingWindowID = qwWindowIDUnderMouse;

                }

            } else {

                kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_LBUTTONDOWN, iMouseX, iMouseY, bButtonStatus, &stEvent );
                kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );

            }
            
        } else {

            if( pstWindowManager->bWindowMoveMode == TRUE ) {

                pstWindowManager->bWindowMoveMode = FALSE;
                pstWindowManager->qwMovingWindowID = WINDOW_INVALIDID;

            } else {

                kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_LBUTTONUP, iMouseX, iMouseY, bButtonStatus, &stEvent );
                kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );

            }

        }

    } else if( bChangedButton & MOUSE_RBUTTONDOWN ) {

        if( bButtonStatus & MOUSE_RBUTTONDOWN ) {

            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_RBUTTONDOWN, iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );

            kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, NULL, NULL, ( QWORD ) kHelloWorldGUITask, TASK_LOADBALANCINGID );

        } else {

            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_RBUTTONUP, iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );

        }

    } else if( bChangedButton & MOUSE_MBUTTONDOWN ) {

        if( bButtonStatus & MOUSE_MBUTTONDOWN ) {

            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MBUTTONDOWN, iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );

        } else {

            kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MBUTTONUP, iMouseX, iMouseY, bButtonStatus, &stEvent );
            kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );

        }

    } else {

        kSetMouseEvent( qwWindowIDUnderMouse, EVENT_MOUSE_MOVE, iMouseX, iMouseY, bButtonStatus, &stEvent );
        kSendEventToWindow( qwWindowIDUnderMouse, &stEvent );

    }

    if( pstWindowManager->bWindowMoveMode == TRUE ) {

        if( kGetWindowArea( pstWindowManager->qwMovingWindowID, &stWindowArea ) == TRUE ) {

            kMoveWindow( pstWindowManager->qwMovingWindowID, stWindowArea.iX1 + iMouseX - iPreviousMouseX, stWindowArea.iY1 + iMouseY - iPreviousMouseY );

        } else {

            pstWindowManager->bWindowMoveMode = FALSE;
            pstWindowManager->qwMovingWindowID = WINDOW_INVALIDID;

        }

    }

    pstWindowManager->bPreviousButtonStatus = bButtonStatus;

    return TRUE;

}

// 수신된 키 데이터를 처리
BOOL kProcessKeyData( void ) {

    KEYDATA stKeyData;
    EVENT stEvent;
    QWORD qwActiveWindowID;

    if( kGetKeyFromKeyQueue( &stKeyData ) == FALSE ) {

        return FALSE;

    }

    qwActiveWindowID = kGetTopWindowID();
    kSetKeyEvent( qwActiveWindowID, &stKeyData, &stEvent );
    
    return kSendEventToWindow( qwActiveWindowID, &stEvent );

}

// 이벤트 큐에 수신된 이벤트 처리
BOOL kProcessEventQueueData( void ) {

    EVENT vstEvent[ WINDOWMANAGER_DATAACCUMULATECOUNT ];
    int iEventCount;
    WINDOWEVENT* pstWindowEvent;
    WINDOWEVENT* pstNextWindowEvent;
    QWORD qwWindowID;
    RECT stArea;
    RECT stOverlappedArea;
    int i;
    int j;

    for( i = 0; i < WINDOWMANAGER_DATAACCUMULATECOUNT; i++ ) {

        if( kReceiveEventFromWindowManagerQueue( &( vstEvent[ i ] ) ) == FALSE ) {

            if( i == 0 ) {

                return FALSE;

            } else {

                break;

            }

        }

        pstWindowEvent = &( vstEvent[ i ].stWindowEvent );

        if( vstEvent[ i ].qwType == EVENT_WINDOWMANAGER_UPDATESCREENBYID ) {

            if( kGetWindowArea( pstWindowEvent->qwWindowID, &stArea ) == FALSE ) {

                kSetRectangleData( 0, 0, 0, 0, &( pstWindowEvent->stArea ) );

            } else {

                kSetRectangleData( 0, 0, kGetRectangleWidth( &stArea ) - 1, kGetRectangleHeight( &stArea ) - 1, &( pstWindowEvent->stArea ) );

            }

        }

    }

    iEventCount = i;

    for( j = 0; j < iEventCount; j++ ) {

        pstWindowEvent = &( vstEvent[ j ].stWindowEvent );
        if( ( vstEvent[ i ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYID ) && ( vstEvent[ j ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA ) && ( vstEvent[ j ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA ) ) {

            continue;

        }

        for( i = j + 1; i < iEventCount; i++ ) {

            pstNextWindowEvent = &( vstEvent[ i ].stWindowEvent );
            if( ( ( vstEvent[ i ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYID ) && ( vstEvent[ i ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA ) && ( vstEvent[ i ].qwType != EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA ) ) || ( pstWindowEvent->qwWindowID != pstNextWindowEvent->qwWindowID ) ) {

                continue;

            }

            if( kGetOverlappedRectangle( &( pstWindowEvent->stArea ), &( pstNextWindowEvent->stArea ), &stOverlappedArea ) == FALSE ) {

                continue;

            }

            if( kMemCmp( &( pstWindowEvent->stArea ), &stOverlappedArea, sizeof( RECT ) ) == 0 ) {

                kMemCpy( &( pstWindowEvent->stArea ), &( pstNextWindowEvent->stArea ), sizeof( RECT ) );
                vstEvent[ i ].qwType = EVENT_UNKNOWN;

            } else if( kMemCmp( &( pstNextWindowEvent->stArea ), &stOverlappedArea, sizeof( RECT ) ) == 0 ) {

                vstEvent[ i ].qwType = EVENT_UNKNOWN;

            }

        }

    }

    for( i = 0; i < iEventCount; i++ ) {

        pstWindowEvent = &( vstEvent[ i ].stWindowEvent );

        switch( vstEvent[ i ].qwType ) {

        case EVENT_WINDOWMANAGER_UPDATESCREENBYID:
        case EVENT_WINDOWMANAGER_UPDATESCREENBYWINDOWAREA:
            if( kConvertRectClientToScreen( pstWindowEvent->qwWindowID, &( pstWindowEvent->stArea ), &stArea ) == TRUE ) {

                kRedrawWindowByArea( &stArea, pstWindowEvent->qwWindowID );

            }

            break;

        case EVENT_WINDOWMANAGER_UPDATESCREENBYSCREENAREA:
            kRedrawWindowByArea( &( pstWindowEvent->stArea ), WINDOW_INVALIDID );

            break;

        default:

            break;

        }

    }

    return TRUE;

}