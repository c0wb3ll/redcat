#ifndef __LIBRARY_H__
#define __LIBRARY_H__

#include "Types.h"
#include <stdarg.h>

void memset( void* pvDestination, BYTE bData, int iSize );
int memcpy( void* pvDestination, const void* pvSource, int iSize );
int memcmp( const void* pvDestination, const void* pvSource, int iSize );
int strcpy( char* pcDestination, const char* pcSource );
int strcmp( char* pcDestination, const char* pcSource );
int strlen( const char* pcBuffer );
long atoi( const char* pcBuffer, int iRadix );
QWORD HexStringToQword( const char* pcBuffer );
long DecimalStringToLong( const char* pcBuffer );
int itoa( long lValue, char* pcBuffer, int iRadix );
int HexToString( QWORD qwValue, char* pcBuffer );
int DecimalToString( long lValue, char* pcBuffer );
void ReverseString( char* pcBuffer );
int sprintf( char* pcBuffer, const char* pcFormatString, ... );
int vsprintf( char* pcBuffer, const char* pcFormatString, va_list ap );
void printf( const char* pcFormatString, ... );
void srand( QWORD qwSeed );
QWORD rand( void );

void memset( void* pvDestination, BYTE bData, int iSize );
int memcpy( void* pvDestination, const void* pvSource, int iSize );
int memcmp( const void* pvDestination, const void* pvSource, int iSize );
int strcpy( char* pcDestination, const char* pcSource );
int strcmp( char* pcDestination, const char* pcSource );
int strlen( const char* pcBuffer );
long atoi( const char* pcBuffer, int iRadix );
QWORD HexStringToQword( const char* pcBuffer );
long DecimalStringToLong( const char* pcBuffer );
int itoa( long lValue, char* pcBuffer, int iRadix );
int HexToString( QWORD qwValue, char* pcBuffer );
int DecimalToString( long lValue, char* pcBuffer );
void ReverseString( char* pcBuffer );
int sprintf( char* pcBuffer, const char* pcFormatString, ... );
int vsprintf( char* pcBuffer, const char* pcFormatString, va_list ap );
void printf( const char* pcFormatString, ... );
void srand( QWORD qwSeed );
QWORD rand( void );

#endif /*__LIBRARY_H__*/