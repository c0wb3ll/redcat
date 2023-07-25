#ifndef __APPLICATIONPANELTASK_H__
#define __APPLICATIONPANELTASK_H__

#include "Types.h"
#include "Window.h"
#include "Font.h"

#define APPLICATIONPANEL_HEIGHT             31
#define APPLICATIONPANEL_TITLE              "SYS_APPLICATIONPANEL"
#define APPLICATIONPANEL_CLOCKWIDTH         ( 8 * FONT_ENGLISHWIDTH )
#define APPLICATIONPANEL_LISTITEMHEIGHT     ( FONT_ENGLISHHEIGHT + 4 )
#define APPLICATIONPANEL_LISTTITLE          "SYS_APPLICATIONLIST"

#define APPLICATIONPANEL_COLOR_OUTERLINE    RGB( 255, 255, 255 )
#define APPLICATIONPANEL_COLOR_MIDDLELINE   RGB( 255, 255, 255 )
#define APPLICATIONPANEL_COLOR_INNERLINE    RGB( 255, 255, 255 )
#define APPLICATIONPANEL_COLOR_BACKGROUND   RGB( 192, 192, 192 )
#define APPLICATIONPANEL_COLOR_ACTIVE       RGB( 128, 128, 128 )

// 애플리케이션 패널이 사용하는 정보를 저장하는 자료구조
typedef struct kApplicationPanelDataStruct {

    QWORD qwApplicationPanelID;

    QWORD qwApplicationListID;

    RECT stButtonArea;

    int iApplicationListWidth;

    int iPreviousMouseOverIndex;

    BOOL bApplicationWindowVisible;

} APPLICATIONPANELDATA;

// GUI 태스크의 정보를 저장하는 자료구조
typedef struct kApplicationEntryStruct {

    char* pcApplicationName;

    void* pvEntryPoint;

} APPLICATIONENTRY;

void kApplicationPanelGUITask( void );

static void kDrawClockInApplicationPanel( QWORD qwApplicationPanelID );
static BOOL kCreateApplicationPanelWindow( void );
static void kDrawDigitalClock( QWORD qwApplicationPanelID );
static BOOL kCreateApplicationListWindow( void );
static void kDrawApplicationListItem( int iIndex, BOOL bSelected );
static BOOL kProcessApplicationPanelWindowEvent( void );
static BOOL kProcessApplicationListWindowEvent( void );
static int kGetMouseOverItemIndex( int iMouseY );

#endif /*__APPLICATIONPANELTASK_H__*/