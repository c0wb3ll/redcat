#ifndef __HANGULINPUT_H__
#define __HANGULINPUT_H__

// 매크로
#define TOLOWER( x ) ( ( ( 'A' <= ( x ) ) && ( ( x ) <= 'Z' ) ) ? ( ( x ) - 'A' + 'a' ) : ( x ) )

// 구조체
typedef struct HangulInputTableItemStruct {

    char* pcHangul;

    char* pcInput;

} HANGULINPUTITEM;

typedef struct HangulIndexTableItemStruct {

    char cStartCharactor;

    DWORD dwIndex;

} HANGULINDEXITEM;

// 함수
BOOL IsJaum( char cInput );
BOOL IsMoum( char cInput );
BOOL FindLongestHangulInTable( const char* pcInputBuffer, int iBufferCount, int* piMatchIndex, int* piMatchLength );
BOOL ComposeHangul( char* pcInputBuffer, int* piInputBufferLength, char* pcOutputBufferForProcessing, char* pcOutputBufferForComplete );

#endif /*__HANGULINPUT_H__*/