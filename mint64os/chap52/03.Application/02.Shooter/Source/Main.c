#include "MINTOSLibrary.h"
#include "Main.h"

GAMEINFO g_stGameInfo = { 0, };

// 응용프로그램의 C언어 엔트리 포인트
int Main( char* pcArgument ) {

    QWORD qwWindowID;
    EVENT stEvent;
    QWORD qwLastTickCount;
    char* pcStartMessage = "Please LButton Down To Start";
    POINT stMouseXY;
    RECT stScreenArea;
    int iX;
    int iY;

    GetScreenArea( &stScreenArea );
    iX = ( GetRectangleWidth( &stScreenArea ) - WINDOW_WIDTH ) / 2;
    iY = ( GetRectangleHeight( &stScreenArea ) - WINDOW_HEIGHT ) / 2;
    qwWindowID = CreateWindow( iX, iY, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS_DEFAULT, "Bubble Shooter" );
    if( qwWindowID == WINDOW_INVALIDID ) {

        printf( "Window create fail\n" );
        return -1;

    }

    stMouseXY.iX = WINDOW_WIDTH / 2;
    stMouseXY.iY = WINDOW_HEIGHT / 2;

    if( Initialize() == FALSE ) {

        DeleteWindow( qwWindowID );
        return -1;

    }

    srand( GetTickCount() );

    DrawInformation( qwWindowID );
    DrawGameArea( qwWindowID, &stMouseXY );
    DrawText( qwWindowID, 5, 150, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), pcStartMessage, strlen( pcStartMessage ) );

    ShowWindow( qwWindowID, TRUE );

    qwLastTickCount = GetTickCount();
    while( 1 ) {

        if( ReceiveEventFromWindowQueue( qwWindowID, &stEvent ) == TRUE ) {

            switch( stEvent.qwType ) {

            case EVENT_MOUSE_LBUTTONDOWN:
                if( g_stGameInfo.bGameStart == FALSE ) {

                    Initialize();

                    g_stGameInfo.bGameStart = TRUE;
                    break;

                }

                DeleteBubbleUnderMouse( &( stEvent.stMouseEvent.stPoint ) );

                memcpy( &stMouseXY, &( stEvent.stMouseEvent.stPoint ), sizeof( stMouseXY ) );
                break;

            case EVENT_MOUSE_MOVE:
                memcpy( &stMouseXY, &( stEvent.stMouseEvent.stPoint ), sizeof( stMouseXY ) );
                break;

            case EVENT_WINDOW_CLOSE:
                DeleteWindow( qwWindowID );
                free( g_stGameInfo.pstBubbleBuffer );
                
                return 0;

                break;

            }

        }

        if( ( g_stGameInfo.bGameStart == TRUE ) && ( ( GetTickCount() - qwLastTickCount ) > 50 ) ) {

            qwLastTickCount = GetTickCount();

            if( ( rand() % 7 ) == 1 ) {

                CreateBubble();

            }

            MoveBubble();

            DrawGameArea( qwWindowID, &stMouseXY );

            DrawInformation( qwWindowID );

            if( g_stGameInfo.iLife <= 0 ) {

                g_stGameInfo.bGameStart = FALSE;

                DrawText( qwWindowID, 80, 130, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), "GameOver", 9 );
                DrawText( qwWindowID, 5, 150, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ), pcStartMessage, strlen( pcStartMessage ) );

            }

            ShowWindow( qwWindowID, TRUE );

        }
        else {

            Sleep( 0 );

        }

    }

    return 0;

}

// 게임에 관련된 정보를 초기화
BOOL Initialize( void ) {

    if( g_stGameInfo.pstBubbleBuffer == NULL ) {

        g_stGameInfo.pstBubbleBuffer = malloc( sizeof( BUBBLE ) * MAXBUBBLECOUNT );
        if( g_stGameInfo.pstBubbleBuffer == NULL ) {

            printf( "Memory allocate fail\n" );
            return FALSE;

        }

    }

    memset( g_stGameInfo.pstBubbleBuffer, 0, sizeof( BUBBLE ) * MAXBUBBLECOUNT );
    g_stGameInfo.iAliveBubbleCount = 0;

    g_stGameInfo.bGameStart = FALSE;
    g_stGameInfo.qwScore = 0;
    g_stGameInfo.iLife = MAXLIFE;

    return TRUE;

}

// 물방울을 생성
BOOL CreateBubble( void ) {

    BUBBLE* pstTarget;
    int i;

    if( g_stGameInfo.iAliveBubbleCount >= MAXBUBBLECOUNT ) {

        return FALSE;

    }

    for( i = 0; i < MAXBUBBLECOUNT; i++ ) {

        if( g_stGameInfo.pstBubbleBuffer[ i ].bAlive == FALSE ) {

            pstTarget = &( g_stGameInfo.pstBubbleBuffer[ i ] );

            pstTarget->bAlive = TRUE;
            pstTarget->qwSpeed = ( rand() % 8 ) + DEFAULTSPEED;

            pstTarget->qwX = rand() % ( WINDOW_WIDTH - 2 * RADIUS ) + RADIUS;
            pstTarget->qwY = INFORMATION_HEIGHT + WINDOW_TITLEBAR_HEIGHT + RADIUS + 1;

            pstTarget->stColor = RGB( rand() % 256, rand() % 256, rand() % 256 );

            g_stGameInfo.iAliveBubbleCount++;

            return TRUE;

        }

    }

    return FALSE;

}

// 물방울을 이동
void MoveBubble( void ) {

    BUBBLE* pstTarget;
    int i;

    for( i = 0; i < MAXBUBBLECOUNT; i++ ) {

        if( g_stGameInfo.pstBubbleBuffer[ i ].bAlive == TRUE ) {

            pstTarget = &( g_stGameInfo.pstBubbleBuffer[ i ] );

            pstTarget->qwY += pstTarget->qwSpeed;

            if( ( pstTarget->qwY + RADIUS ) >= WINDOW_HEIGHT ) {

                pstTarget->bAlive = FALSE;

                g_stGameInfo.iAliveBubbleCount--;
                if( g_stGameInfo.iLife > 0 ) {

                    g_stGameInfo.iLife--;

                }

            }

        }

    }

}

// 마우스 아래에 있는 물방울을 삭제하고 점수를 증가
void DeleteBubbleUnderMouse( POINT* pstMouseXY ) {

    BUBBLE* pstTarget;
    int i;
    QWORD qwDistance;

    for( i = MAXBUBBLECOUNT - 1; i >= 0; i-- ) {

        if( g_stGameInfo.pstBubbleBuffer[ i ].bAlive == TRUE ) {

            pstTarget = &( g_stGameInfo.pstBubbleBuffer[ i ] );

            qwDistance = ( ( pstMouseXY->iX - pstTarget->qwX ) *
                           ( pstMouseXY->iX - pstTarget->qwX ) ) +
                         ( ( pstMouseXY->iY - pstTarget->qwY ) *
                           ( pstMouseXY->iY - pstTarget->qwY ) );

            if( qwDistance < ( RADIUS * RADIUS ) ) {

                pstTarget->bAlive = FALSE;

                g_stGameInfo.iAliveBubbleCount--;
                g_stGameInfo.qwScore++;
                break;

            }

        }

    }

}

// 게임 정보를 화면에 출력
void DrawInformation( QWORD qwWindowID ) {

    char vcBuffer[ 200 ];
    int iLength;

    DrawRect( qwWindowID, 1, WINDOW_TITLEBAR_HEIGHT -1, WINDOW_WIDTH - 2, WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT, RGB( 192, 192, 192 ), TRUE );

    sprintf( vcBuffer, "Life: %d, Score: %d\n", g_stGameInfo.iLife, g_stGameInfo.qwScore );
    iLength = strlen( vcBuffer );

    DrawText( qwWindowID, ( WINDOW_WIDTH - iLength * FONT_ENGLISHWIDTH ) / 2, WINDOW_TITLEBAR_HEIGHT + 2, RGB( 0, 0, 0 ), RGB( 192, 192, 192 ), vcBuffer, strlen( vcBuffer ) );

}

// 게임 영역에 물방울을 표시
void DrawGameArea( QWORD qwWindowID, POINT* pstMouseXY ) {

    BUBBLE* pstTarget;
    int i;

    DrawRect( qwWindowID, 0, WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1, RGB( 255, 255, 255 ), TRUE );

    for( i = 0; i < MAXBUBBLECOUNT; i++ ) {

        if( g_stGameInfo.pstBubbleBuffer[ i ].bAlive == TRUE ) {

            pstTarget = &( g_stGameInfo.pstBubbleBuffer[ i ] );

            DrawCircle( qwWindowID, pstTarget->qwX, pstTarget->qwY, RADIUS, pstTarget->stColor, TRUE );
            DrawCircle( qwWindowID, pstTarget->qwX, pstTarget->qwY, RADIUS, ~pstTarget->stColor, FALSE );

        }

    }

    if( pstMouseXY->iY < ( WINDOW_TITLEBAR_HEIGHT + RADIUS ) ) {

        pstMouseXY->iY = WINDOW_TITLEBAR_HEIGHT + RADIUS;

    }

    DrawLine( qwWindowID, pstMouseXY->iX, pstMouseXY->iY - RADIUS, pstMouseXY->iX, pstMouseXY->iY + RADIUS, RGB( 255, 135, 135 ) );
    DrawLine( qwWindowID, pstMouseXY->iX - RADIUS, pstMouseXY->iY, pstMouseXY->iX + RADIUS, pstMouseXY->iY, RGB( 255, 135, 135 ) );

    DrawRect( qwWindowID, 0, WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1, RGB( 160, 255, 140 ), FALSE );

}