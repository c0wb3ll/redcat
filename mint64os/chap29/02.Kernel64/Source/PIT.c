#include "PIT.h"

void kInitializePIT( WORD wCount, BOOL bPeriodic ) {

    kOutPortByte( PIT_PORT_CONTROL, PIT_COUNTER0_ONCE );

    if( bPeriodic == TRUE ) {

        kOutPortByte( PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC );

    }

    kOutPortByte( PIT_PORT_COUNTER0, wCount );
    kOutPortByte( PIT_PORT_COUNTER0, wCount >> 8 );

}

WORD kReadCounter0( void ) {

    BYTE bHighByte, bLowByte;
    WORD wTemp = 0;

    kOutPortByte( PIT_PORT_CONTROL, PIT_COUNTER0_LATCH );

    bLowByte = kInPortByte( PIT_PORT_COUNTER0 );
    bHighByte = kInPortByte( PIT_PORT_COUNTER0 );

    wTemp = bHighByte;
    wTemp = ( wTemp << 8 ) | bLowByte;
    
    return wTemp;

}

// 카운터 0를 직접 설정하여 일정 시간 이상 대기
//  함수를 호출하면 PIT 컨트롤러의 설정이 바뀌므로, 이후에 PIT 컨트롤러를 재설정해야 함
//  정확하게 측정하려면 함수 사용 전에 인터럽트를 비활성화 하는 것이 좋음
//   약 50ms까지 측정 가능
void kWaitUsingDirectPIT( WORD wCount ) {

    WORD wLastCounter0;
    WORD wCurrentCounter0;

    kInitializePIT( 0, TRUE );

    wLastCounter0 = kReadCounter0();
    while( 1 ) {

        wCurrentCounter0 = kReadCounter0();
        if( ( ( wLastCounter0 - wCurrentCounter0 ) & 0xFFFF ) >= wCount ) { break; }

    }

}