#include "LocalAPIC.h"
#include "MPConfigurationTable.h"

// 로컬 APIC의 메모리 맵 I/O 어드레스를 반환
QWORD kGetLocalAPICBaseAddress( void ) {

    MPCONFIGURATIONTABLEHEADER* pstMPHeader;

    pstMPHeader = kGetMPConfigurationManager()->pstMPConfigurationTableHeader;

    return pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;

}

// 의사 인터럽트 벡터 레지스터에 있는 apic 소프트웨어 활성/비활성 필드를 1로 설정하여 로컬 APIC 활성화
void kEnableSoftwareLocalAPIC( void ) {

    QWORD qwLocalAPICBaseAddress;

    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();

    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_SVR ) |= 0x100;

}

// 로컬 APIC에 EOI를 전송
void kSendEOIToLocalAPIC( void ) {

    QWORD qwLocalAPICBaseAddress;
    
    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();

    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_EOI ) = 0;

}

// 태스크 우선순위 레지스터 설정
void kSetTaskPriority( BYTE bPriority ) {

    QWORD qwLocalAPICBaseAddress;

    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();

    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_TASKPRIORITY ) = bPriority;

}

// 로컬 벡터 테이블 초기화
void kInitializeLocalVectorTable( void ) {

    QWORD qwLocalAPICBaseAddress;
    DWORD dwTempValue;

    qwLocalAPICBaseAddress = kGetLocalAPICBaseAddress();

    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_TIMER ) |= APIC_INTERRUPT_MASK;
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_LINT0 ) |= APIC_INTERRUPT_MASK;
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_LINT1 ) = APIC_TRIGGERMODE_EDGE | APIC_POLARITY_ACTIVEHIGH | APIC_DELIVERYMODE_NMI;
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_ERROR ) |= APIC_INTERRUPT_MASK;
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_PERFORMANCEMONITORINGCOUNTER ) |= APIC_INTERRUPT_MASK;
    *( DWORD* ) ( qwLocalAPICBaseAddress + APIC_REGISTER_THERMALSENSOR ) |= APIC_INTERRUPT_MASK;

}