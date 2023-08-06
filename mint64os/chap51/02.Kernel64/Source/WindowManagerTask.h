#ifndef __WINDOWMANAGER_H__
#define __WINDOWMANAGER_H__


#define WINDOWMANAGER_DATAACCUMULATECOUNT       20
#define WINDOWMANAGER_RESIZEMARKERSIZE          20
#define WINDOWMANAGER_COLOR_RESIZEMARKER        RGB( 255, 255, 255 )
#define WINDOWMANAGER_THICK_RESIZEMARKER        4

void kStartWindowManager( void );
BOOL kProcessMouseData( void );
BOOL kProcessKeyData( void );
BOOL kProcessEventQueueData( void );

void kDrawResizeMarker( const RECT* pstArea, BOOL bShowMarker );

#endif /*__WINDOWMANAGER_H__*/
