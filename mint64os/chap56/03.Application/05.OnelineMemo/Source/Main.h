#ifndef __MAIN_H__
#define __MAIN_H__

// 매크로
#define MAXOUTPUTLENGTH     60

// 구조체
typedef struct BufferManagerStruct {

    char vcInputBuffer[ 20 ];
    int iInputBufferLength;

    char vcOutputBufferForProcessing[ 3 ];
    char vcOutputBufferForComplete[ 3 ];

    char vcOutputBuffer[ MAXOUTPUTLENGTH ];
    int iOutputBufferLength;

} BUFFERMANAGER;

// 함수
void ConvertJaumMoumToLowerCharactor( BYTE* pbInput );

#endif /*__MAIN_H__*/