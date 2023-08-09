#ifndef __SYSTEMCALL_H__
#define __SYSTEMCALL_H__

// 매크로
#define SYSTEMCALL_MAXPARAMETERCOUNT        20

// 구조체
#pragma pack( push, 1 )

typedef struct kSystemCallParameterTableStruct {

    QWORD vqwValue[ SYSTEMCALL_MAXPARAMETERCOUNT ];

} PARAMETERTABLE;

#pragma pack( pop )

#define PARAM( x )  ( pstParameter->vqwValue[ ( x ) ] )

// 함수
void kSystemCallEntryPoint( QWORD qwServiceNumber, PARAMETERTABLE* pstParameter );
QWORD kProcessSystemCall( QWORD qwServiceNumber, PARAMETERTABLE* pstParameter );

void kSystemCallTestTask( void );

#endif /*__SYSTEMCALL_H__*/