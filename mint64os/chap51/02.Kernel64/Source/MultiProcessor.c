#include "MultiProcessor.h"
#include "MPConfigurationTable.h"
#include "AssemblyUtility.h"
#include "LocalAPIC.h"
#include "PIT.h"

volatile int g_iWakeUpApplicationProcessorCount = 0;
volatile QWORD g_qwAPICIDAddress = 0;

// 로컬 APIC 활성화하고 AP를 활성화
BOOL kStartUpApplicationProcessor( void ) {

    if( kAnalysisMPConfigurationTable() == FALSE ) {

        return FALSE;

    }

    kEnableGlobalLocalAPIC();
    kEnableSoftwareLocalAPIC();

    if( kWakeUpApplicationProcessor() == FALSE ) {

        return FALSE;

    }

    return TRUE;

}

// AP 활성화
static BOOL kWakeUpApplicationProcessor( void ) {

    MPCONFIGURATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    QWORD qwLocalAPICBaseAddress;
    BOOL bInterruptFlag;
    int i;

    bInterruptFlag = kSetInterruptFlag( FALSE );

    pstMPManager = kGetMPConfigurationManager();
    pstMPHeader = pstMPManager->pstMPConfigurationTableHeader;
    qwLocalAPICBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;

    g_qwAPICIDAddress = qwLocalAPICBaseAddress + APIC_REGISTER_APICID;

    *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) = APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE | APIC_LEVEL_ASSERT | APIC_DESTINATIONMODE_PHYSICAL | APIC_DELIVERYMODE_INIT;

    kWaitUsingDirectPIT( MSTOCOUNT( 10 ) );

    if( *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) & APIC_DELIVERYSTATUS_PENDING ) {

        kInitializePIT( MSTOCOUNT( 1 ), TRUE );

        kSetInterruptFlag( bInterruptFlag );
        return FALSE;

    }

    for( i = 0; i < 2; i++ ) {

        *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) = APIC_DESTINATIONSHORTHAND_ALLEXCLUDINGSELF | APIC_TRIGGERMODE_EDGE | APIC_LEVEL_ASSERT | APIC_DESTINATIONMODE_PHYSICAL | APIC_DELIVERYMODE_STARTUP | 0x10;

        kWaitUsingDirectPIT( USTOCOUNT( 200 ) );

        if( *( DWORD* )( qwLocalAPICBaseAddress + APIC_REGISTER_ICR_LOWER ) & APIC_DELIVERYSTATUS_PENDING ) {

            kInitializePIT( MSTOCOUNT( 1 ), TRUE );
            
            kSetInterruptFlag( bInterruptFlag );

            return FALSE;

        }

    }

    kInitializePIT( MSTOCOUNT( 1 ), TRUE );

    kSetInterruptFlag( bInterruptFlag );

    while( g_iWakeUpApplicationProcessorCount < ( pstMPManager->iProcessorCount - 1 ) ) {

        kSleep( 50 );

    }

    return TRUE;

}

// APIC ID 레지스터에서 APIC ID를 반환
BYTE kGetAPICID( void ) {

    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    QWORD qwLocalAPICBaseAddress;

    if( g_qwAPICIDAddress == 0 ) {

        pstMPHeader = kGetMPConfigurationManager()->pstMPConfigurationTableHeader;
        if( pstMPHeader == NULL ) {

            return 0;

        }

        qwLocalAPICBaseAddress = pstMPHeader->dwMemoryMapIOAddressOfLocalAPIC;
        g_qwAPICIDAddress = qwLocalAPICBaseAddress + APIC_REGISTER_APICID;

    }

    return *( ( DWORD* ) g_qwAPICIDAddress ) >> 24;

}