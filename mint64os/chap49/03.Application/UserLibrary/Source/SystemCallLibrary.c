#include "SystemCallLibrary.h"

// 콘솔에 문자열 출력
int ConsolePrintString( const char* pcBuffer ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pcBuffer;

    return ExecuteSystemCall( SYSCALL_CONSOLEPRINTSTRING, &stParameter );

}

// 커서의 위치 설정
BOOL SetCursor( int iX, int iY ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;

    return ExecuteSystemCall( SYSCALL_SETCURSOR, &stParameter );

}

//현재 커서의 위치를 반환
BOOL GetCursor( int *piX, int *piY ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) piX;
    PARAM( 1 ) = ( QWORD ) piY;

    return ExecuteSystemCall( SYSCALL_GETCURSOR, &stParameter );

}

// 전체 화면을 삭제
BOOL ClearScreen( void ) {

    return ExecuteSystemCall( SYSCALL_CLEARSCREEN, NULL );

}

// getch() 함수의 구현
BYTE getch( void ) {

    return ExecuteSystemCall( SYSCALL_GETCH, NULL );

}

// 메모리 할당
void* malloc( QWORD qwSize ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = qwSize;

    return ( void* ) ExecuteSystemCall( SYSCALL_MALLOC, &stParameter );

}

// 할당 받은 메모리를 해제
BOOL free( void* pvAddress ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pvAddress;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_FREE, &stParameter );

}

// 파일을 열거나 생성
FILE* fopen( const char* pcFileName, const char* pcMode ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pcFileName;
    PARAM( 1 ) = ( QWORD ) pcMode;

    return ( FILE* ) ExecuteSystemCall( SYSCALL_FOPEN, &stParameter );

}

// 파일을 읽어 버퍼로 복사
DWORD fread( void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pvBuffer;
    PARAM( 1 ) = ( QWORD ) dwSize;
    PARAM( 2 ) = ( QWORD ) dwCount;
    PARAM( 3 ) = ( QWORD ) pstFile

    return ExecuteSystemCall( SYSCALL_FREAD, &stParameter );

}

// 버퍼의 데이터를 파일에 씀
DWORD fwrite( const void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pvBuffer;
    PARAM( 1 ) = ( QWORD ) dwSize;
    PARAM( 2 ) = ( QWORD ) dwCount;
    PARAM( 3 ) = ( QWORD ) pstFile;

    return ExecuteSystemCall( SYSCALL_FWRITE, &stParameter );

}

// 파일 포인터의 위치를 이동
int fseek( FILE* pstFile, int iOffset, int iOrigin ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pstFile;
    PARAM( 1 ) = ( QWORD ) iOffset;
    PARAM( 2 ) = ( QWORD ) iOrigin;

    return ( int ) ExecuteSystemCall( SYSCALL_FSEEK, &stParameter );

}

// 파일을 닫음
int fclose( FILE* pstFile ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pstFile;

    return ( int ) ExecuteSystemCall( SYSCALL_FCLOSE, &stParameter );

}

// 파일을 삭제
int remove( const char* pcFileName ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pcFileName;

    return ( int ) ExecuteSystemCall( SYSCALL_REMOVE, &stParameter );

}

// 디렉터리를 엶
DIR* opendir( const char* pcDirectoryName ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pcDirectoryName;

    return ( DIR* ) ExecuteSystemCall( SYSCALL_OPENDIR, &stParameter ); 

}

// 디렉터리 엔트리를 반환하고 다음으로 이동
struct dirent* readdir( DIR* pstDirectory ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pstDirectory;

    return ( struct dirent* ) ExecuteSystemCall( SYSCALL_READDIR, &stParameter );

}

// 디렉터리 포인터를 디렉터리의 처음으로 이동
BOOL rewinddir( DIR* pstDirectory ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pstDirectory;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_REWINDDIR, &stParameter );

}

// 열린 디렉터리를 닫음
int closedir( DIR* pstDirectory ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pstDirectory;

    return ( int ) ExecuteSystemCall( SYSCALL_CLOSEDIR, &stParameter );

}

// 핸들 풀을 검사하여 파일이 열려있는지 확인
BOOL IsFileOpened( const struct dirent* pstEntry ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pstEntry;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISFILEOPENED, &stParameter );

}

// 하드 디스크에 섹터를 읽음
int ReadHDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) bPrimary;
    PARAM( 1 ) = ( QWORD ) bMaster;
    PARAM( 2 ) = ( QWORD ) dwLBA;
    PARAM( 3 ) = ( QWORD ) iSectorCount;
    PARAM( 4 ) = ( QWORD ) pcBuffer;

    return ( int ) ExecuteSystemCall( SYSCALL_READHDDSECTOR, &stParameter );

}

// 하드 디스크에 섹터를 씀
int WriteHDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) bPrimary;
    PARAM( 1 ) = ( QWORD ) bMaster;
    PARAM( 2 ) = ( QWORD ) dwLBA;
    PARAM( 3 ) = ( QWORD ) iSectorCount;
    PARAM( 4 ) = ( QWORD ) pcBuffer;

    return ( int ) ExecuteSystemCall( SYSCALL_WRITEHDDSECTOR, &stParameter );

}

// 태스크 생성
QWORD CreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, QWORD qwEntryPointAddress, BYTE bAffinity ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) qwFlags;
    PARAM( 1 ) = ( QWORD ) pvMemoryAddress;
    PARAM( 2 ) = ( QWORD ) qwMemorySize;
    PARAM( 3 ) = ( QWORD ) qwEntryPointAddress;
    PARAM( 4 ) = ( QWORD ) bAffinity;

    return ExecuteSystemCall( SYSCALL_CREATETASK, &stParameter );

}

// 다른 태스크를 찾아서 전환
BOOL Schedule( void ) {

    return ( BOOL) ExecuteSystemCall( SYSCALL_SCHEDULE, NULL );

}

// 태스크의 우선 순위를 변경
BOOL ChangePriority( QWORD qwID, BYTE bPriority, BOOL bExecutedInInterrupt ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = qwID;
    PARAM( 1 ) = ( QWORD ) bPriority;
    PARAM( 2 ) = ( QWORD ) bExecutedInInterrupt;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_CHANGEPRIORITY, &stParameter );

}

// 태스크를 종료
BOOL EndTask( QWORD qwTaskID ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = qwTaskID;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_ENDTASK, &stParameter );

}

// 태스크가 자신을 종료
void exit( int iExitValue ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) iExitValue;

    ExecuteSystemCall( SYSCALL_EXITTASK, &stParameter );

}

//전체 태스크의 수를 반환
int GetTaskCount( BYTE bAPICID ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) bAPICID;

    return ( int ) ExecuteSystemCall( SYSCALL_GETTASKCOUNT, &stParameter );

}

// 태스크가 존재하는지 여부를 반환
BOOL IsTaskExist( QWORD qwID ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = qwID;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISTASKEXIST, &stParameter );

}

QWORD GetProcessorLoad( BYTE bAPICID ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) bAPICID;

    return ExecuteSystemCall( SYSCALL_GETPROCESSORLOAD, &stParameter );

}

BOOL ChangeProcessorAffinity( QWORD qwTaskID, BYTE bAffinity ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = qwTaskID;
    PARAM( 1 ) = ( QWORD ) bAffinity;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_CHANGEPROCESSORAFFINITY, &stParameter );

}

// 배경 윈도우의 ID를 반환
QWORD GetBackgroundWindowID( void ) {

    return ExecuteSystemCall( SYSCALL_GETBACKGROUNDWINDOWID, NULL );

}

void GetScreenArea( RECT* pstScreenArea ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) pstScreenArea;

    ExecuteSystemCall( SYSCALL_GETSCREENAREA, &stParameter );

}

// 윈도우를 생성
QWORD CreateWindow( int iX, int iY, int iWidth, int iHeight, DWORD dwFlags, const char* pcTitle ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;
    PARAM( 2 ) = ( QWORD ) iWidth;
    PARAM( 3 ) = ( QWORD ) iHeight;
    PARAM( 4 ) = ( QWORD ) dwFlags;
    PARAM( 5 ) = ( QWORD ) pcTitle;

    return ExecuteSystemCall( SYSCALL_CREATEWINDOW, &stParameter );

}

// 윈도우를 삭제
BOOL DeleteWindow( QWORD qwWindowID ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DELETEWINDOW, &stParameter ); 

}

// 윈도우를 화면에 나타내거나 숨김
BOOL ShowWindow( QWORD qwWindowID, BOOL bShow ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) bShow;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_SHOWWINDOW, &stParameter );

}

// 특정 위치를 포함하는 윈도우 중에서 가장 위에 있는 윈도우를 반환
QWORD FindWindowByPoint( int iX, int iY ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;

    return ExecuteSystemCall( SYSCALL_FINDWINDOWBYPOINT, &stParameter );

}

// 윈도우 제목이 일치하는 윈도우를 반환
QWORD FindWindowByTitle( const char* pcTitle ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) pcTitle;
    
    return ExecuteSystemCall( SYSCALL_FINDWINDOWBYTITLE, &stParameter );

}

// 윈도우가 존재하는지 여부 반환
BOOL IsWindowExist( QWORD qwWindowID ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISWINDOWEXIST, &stParameter );

}

// 최상위 윈도우의 ID를 반환
QWORD GetTopWindowID( void ) {

    return ExecuteSystemCall( SYSCALL_GETTOPWINDOWID, NULL );     

}

// 윈도우의 Z 순서를 최상위로 만듬
BOOL MoveWindowToTop( QWORD qwWindowID ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_MOVEWINDOWTOTOP, &stParameter );

}

// X,Y 좌표가 윈도우의 제목 표시줄 위치에 있는지 확인
BOOL IsInTitleBar( QWORD qwWindowID, int iX, int iY ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISINTITLEBAR, &stParameter );  

}

BOOL IsInCloseButton( QWORD qwWindowID, int iX, int iY ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_ISINCLOSEBUTTON, &stParameter );

}

// 윈도우를 해당 위치로 이동
BOOL MoveWindow( QWORD qwWindowID, int iX, int iY ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_MOVEWINDOW, &stParameter );

}

// 윈도우의 크기를 변경
BOOL ResizeWindow( QWORD qwWindowID, int iX, int iY, int iWidth, int iHeight ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) iWidth;
    PARAM( 4 ) = ( QWORD ) iHeight;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_RESIZEWINDOW, &stParameter );

}

// 윈도우 영역을 반환
BOOL GetWindowArea( QWORD qwWindowID, RECT* pstArea ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstArea;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_GETWINDOWAREA, &stParameter );

}

// 윈도우를 화면에 업데이트
BOOL UpdateScreenByID( QWORD qwWindowID ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_UPDATESCREENBYID, &stParameter );

}

// 윈도우의 내부를 화면에 업데이트
BOOL UpdateScreenByWindowArea( QWORD qwWindowID, const RECT* pstArea ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstArea;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_UPDATESCREENBYWINDOWAREA, &stParameter );

}

// 화면 좌표로 화면을 업데이트
BOOL UpdateScreenByScreenArea( const RECT* pstArea ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) pstArea;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_UPDATESCREENBYSCREENAREA, &stParameter );

}

// 윈도우로 이벤트를 전송
BOOL SendEventToWindow( QWORD qwWindowID, const EVENT* pstEvent ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstEvent;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_SENDEVENTTOWINDOW, &stParameter );

}

// 윈도우의 이벤트 큐에 저장된 이벤트를 수신
BOOL ReceiveEventFromWindowQueue( QWORD qwWindowID, EVENT* pstEvent ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstEvent;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_RECEIVEEVENTFROMWINDOWQUEUE, &stParameter );

}

// 윈도우 화면 버퍼에 윈도우 테두리 그리기
BOOL DrawWindowFrame( QWORD qwWindowID ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;

    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWWINDOWFRAME, &stParameter );

}

// 윈도우 화면 버퍼에 배경 그리기
BOOL DrawWindowBackground( QWORD qwWindowID ) {

    PARAMETERTABLE stParameter;

    PARAM( 0 ) = qwWindowID;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWWINDOWBACKGROUND, &stParameter );

}

// 윈도우 화면 버퍼에 윈도우 제목 표시줄 그리기
BOOL DrawWindowTitle( QWORD qwWindowID, const char* pcTitle, BOOL bSelectedTitle ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pcTitle;
    PARAM( 2 ) = ( QWORD ) bSelectedTitle;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWWINDOWTITLE, &stParameter );        

}

// 윈도우 내부에 버튼 그리기
BOOL DrawButton( QWORD qwWindowID, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) pstButtonArea;
    PARAM( 2 ) = ( QWORD ) stBackgroundColor;
    PARAM( 3 ) = ( QWORD ) pcText;
    PARAM( 4 ) = ( QWORD ) stTextColor;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWBUTTON, &stParameter );

}

// 윈도우 내부에 점 그리기
BOOL DrawPixel( QWORD qwWindowID, int iX, int iY, COLOR stColor ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) stColor;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWPIXEL, &stParameter );

}

// 윈도우 내부의 직선 그리기
BOOL DrawLine( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX1;
    PARAM( 2 ) = ( QWORD ) iY1;
    PARAM( 3 ) = ( QWORD ) iX2;
    PARAM( 4 ) = ( QWORD ) iY2;
    PARAM( 5 ) = ( QWORD ) stColor;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWLINE, &stParameter );

}

// 윈도우 내부에 사각형 그리기
BOOL DrawRect( QWORD qwWindowID, int iX1, int iY1, int iX2, int iY2, COLOR stColor, BOOL bFill ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX1;
    PARAM( 2 ) = ( QWORD ) iY1;
    PARAM( 3 ) = ( QWORD ) iX2;
    PARAM( 4 ) = ( QWORD ) iY2;
    PARAM( 5 ) = ( QWORD ) stColor;
    PARAM( 6 ) = ( QWORD ) bFill;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWRECT, &stParameter );

}

// 윈도우 내부에 원 그리기
BOOL DrawCircle( QWORD qwWindowID, int iX, int iY, int iRadius, COLOR stColor, BOOL bFill ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) iRadius;
    PARAM( 4 ) = ( QWORD ) stColor;
    PARAM( 5 ) = ( QWORD ) bFill;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWCIRCLE, &stParameter );

}

// 윈도우 내부에 문자 출력
BOOL DrawText( QWORD qwWindowID, int iX, int iY, COLOR stTextColor, COLOR stBackgroundColor, const char* pcString, int iLength ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) stTextColor;
    PARAM( 4 ) = ( QWORD ) stBackgroundColor;
    PARAM( 5 ) = ( QWORD ) pcString;
    PARAM( 6 ) = ( QWORD ) iLength;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_DRAWTEXT, &stParameter );

}

// 마우스 커서를 해당 위치로 이동해서 그려줌
void MoveCursor( int iX, int iY ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) iX;
    PARAM( 1 ) = ( QWORD ) iY;
    
    ExecuteSystemCall( SYSCALL_MOVECURSOR, &stParameter );

}

// 현재 마우스 커서의 위치를 변환
void GetCursorPosition( int* piX, int* piY ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) piX;
    PARAM( 1 ) = ( QWORD ) piY;
    
    ExecuteSystemCall( SYSCALL_GETCURSORPOSITION, &stParameter );

}

// 윈도우 화면 버퍼의 내용을 한번에 전송
BOOL BitBlt( QWORD qwWindowID, int iX, int iY, COLOR* pstBuffer, int iWidth, int iHeight ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwWindowID;
    PARAM( 1 ) = ( QWORD ) iX;
    PARAM( 2 ) = ( QWORD ) iY;
    PARAM( 3 ) = ( QWORD ) pstBuffer;
    PARAM( 4 ) = ( QWORD ) iWidth;
    PARAM( 5 ) = ( QWORD ) iHeight;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_BITBLT, &stParameter );

}

// JPEG 이미지 파일의 전체가 담긴 파일 버퍼와 크기를 이용해 JPEG 자료구조 초기화
BOOL JPEGInit( JPEG* jpeg, BYTE* pbFileBuffer, DWORD dwFileSize ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) jpeg;
    PARAM( 1 ) = ( QWORD ) pbFileBuffer;
    PARAM( 2 ) = ( QWORD ) dwFileSize;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_JPEGINIT, &stParameter );

}

// JPEG 자료구조에 저장된 정보를 이용하여 디코딩한 결과를 출력 버퍼에 저장
BOOL JPEGDecode( JPEG* jpeg, COLOR* rgb ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) jpeg;
    PARAM( 1 ) = ( QWORD ) rgb;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_JPEGDECODE, &stParameter );

}

// CMOS 메모리에서 rtc 컨트롤러가 저장한 현재시간을 읽음
BOOL ReadRTCTime( BYTE* pbHour, BYTE* pbMinute, BYTE* pbSecond ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) pbHour;
    PARAM( 1 ) = ( QWORD ) pbMinute;
    PARAM( 2 ) = ( QWORD ) pbSecond;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_READRTCTIME, &stParameter );

}

// CMOS 메모리에서 rtc 컨트롤러가 저장한 현재 일자를 읽음
BOOL ReadRTCDate( WORD* pwYear, BYTE* pbMonth, BYTE* pbDayOfMonth, BYTE* pbDayOfWeek ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) pwYear;
    PARAM( 1 ) = ( QWORD ) pbMonth;
    PARAM( 2 ) = ( QWORD ) pbDayOfMonth;
    PARAM( 3 ) = ( QWORD ) pbDayOfWeek;
    
    return ( BOOL ) ExecuteSystemCall( SYSCALL_READRTCDATE, &stParameter );

}

void SendSerialData( BYTE* pbBuffer, int iSize ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) pbBuffer;
    PARAM( 1 ) = ( QWORD ) iSize;
    
    ExecuteSystemCall( SYSCALL_SENDSERIALDATA, &stParameter );

}

// 시리얼 포트에서 데이터를 읽음
int ReceiveSerialData( BYTE* pbBuffer, int iSize ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = ( QWORD ) pbBuffer;
    PARAM( 1 ) = ( QWORD ) iSize;
    
    return ( int ) ExecuteSystemCall( SYSCALL_RECEIVESERIALDATA, &stParameter );

}

// 시리얼 포트 컨트롤러의 FIFO 초기화 
void ClearSerialFIFO( void ) {

    ExecuteSystemCall( SYSCALL_CLEARSERIALFIFO, NULL );

}

// RAM 크기 반환
QWORD GetTotalRAMSize( void ) {

    return ExecuteSystemCall( SYSCALL_GETTOTALRAMSIZE, NULL );

}

QWORD GetTickCount( void ) {

    return ExecuteSystemCall( SYSCALL_GETTICKCOUNT, NULL );

}

// 밀리세컨드 동안 대기
void Sleep( QWORD qwMillisecond ) {

    PARAMETERTABLE stParameter;
    
    PARAM( 0 ) = qwMillisecond;

    ExecuteSystemCall( SYSCALL_SLEEP, &stParameter );

}

// 그래픽 모드인지 확인
BOOL IsGraphicMode( void ) {

    ExecuteSystemCall( SYSCALL_ISGRAPHICMODE, NULL );

}