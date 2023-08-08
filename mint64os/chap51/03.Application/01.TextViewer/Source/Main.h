#ifndef __MAIN_H__
#define __MAIN_H__

// 매크로
#define MAXLINECOUNT        ( 256 * 1024 )
#define MARGIN              5
#define TABSPACE            4

// 구조체
// 텍스트 정보를 저장하는 구조체
typedef struct TextInformationStruct {

    BYTE* pbFileBuffer;
    DWORD dwFileSize;

    int iColumnCount;
    int iRowCount;

    DWORD* pdwFileOffsetOfLine;

    int iMaxLineCount;
    int iCurrentLineIndex;

    char vcFileName[ 100 ];

} TEXTINFO;

// 함수
BOOL ReadFileToBuffer( const char* pcFileName, TEXTINFO* pstInfo );
void CalculateFileOffsetOfLine( int iWidth, int iHeight, TEXTINFO* pstInfo );
BOOL DrawTextBuffer( QWORD qwWindowID, TEXTINFO* pstInfo );

#endif /*__MAIN_H__*/