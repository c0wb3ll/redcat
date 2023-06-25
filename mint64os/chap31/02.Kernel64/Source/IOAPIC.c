#include "IOAPIC.h"
#include "MPConfigurationTable.h"
#include "PIC.h"

static IOAPICMANAGER gs_stIOAPICManager;

// ISA 버스가 연결된 I/O APIC의 기준 어드레스를 반환
QWORD kGetIOAPICBaseAddressOfISA( void ) {

    MPCONFIGURATIONMANAGER* pstMPManager;
    IOAPICENTRY* pstIOAPICEntry;

    if( gs_stIOAPICManager.qwIOAPICBaseAddressOfISA == NULL ) {

        pstIOAPICEntry = kFindIOAPICEntryForISA();
        if( pstIOAPICEntry != NULL ) {

            gs_stIOAPICManager.qwIOAPICBaseAddressOfISA = pstIOAPICEntry->dwMemoryMapAddress & 0xFFFFFFFF;

        }

    }

    return gs_stIOAPICManager.qwIOAPICBaseAddressOfISA;

}

// I/O 리다이렉션 테이블 자료구조에 값을 설정
void kSetIOAPICRedirectionEntry( IOREDIRECTIONTABLE* pstEntry, BYTE bAPICID, BYTE bInterruptMask, BYTE bFlagsAndDeliveryMode, BYTE bVector ) {

    kMemSet( pstEntry, 0, sizeof( IOREDIRECTIONTABLE ) );

    pstEntry->bDestination = bAPICID;
    pstEntry->bFlagsAndDeliveryMode = bFlagsAndDeliveryMode;
    pstEntry->bInterruptMask = bInterruptMask;
    pstEntry->bVector = bVector;

}

// 인터럽트 입력 핀(INTIN)에 해당하는 I/O 리다이렉션 테이블에서 값을 얻음
void kReadIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry ) {

    QWORD* pqwData;
    QWORD qwIOAPICBaseAddress;

    qwIOAPICBaseAddress = kGetIOAPICBaseAddressOfISA();

    pqwData = ( QWORD* ) pstEntry;

    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE + iINTIN * 2;
    *pqwData = *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW );
    *pqwData = *pqwData << 32;

    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE + iINTIN * 2;
    *pqwData |= *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW );

}

// 인터럽트 입력 핀에 해당하는 I/O 리다이렉션 테이블에 값을 씀
void kWriteIOAPICRedirectionTable( int iINTIN, IOREDIRECTIONTABLE* pstEntry ) {

    QWORD* pqwData;
    QWORD qwIOAPICBaseAddress;

    qwIOAPICBaseAddress = kGetIOAPICBaseAddressOfISA();

    pqwData = ( QWORD* ) pstEntry;

    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = IOAPIC_REGISTERINDEX_HIGHIOREDIRECTIONTABLE + iINTIN * 2;
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW ) = *pqwData >> 32;

    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOREGISTERSELECTOR ) = IOAPIC_REGISTERINDEX_LOWIOREDIRECTIONTABLE + iINTIN * 2;
    *( DWORD* ) ( qwIOAPICBaseAddress + IOAPIC_REGISTER_IOWINDOW ) = *pqwData;

}

// I/O APIC에 연결된 모든 인터럽트 핀을 마스크하여 인터럽트가 전달되지 않도록 함
void kMaskAllInterruptInIOAPIC( void ) {

    IOREDIRECTIONTABLE stEntry;
    int i;

    for( i = 0; i < IOAPIC_MAXIOREDIRECTIONTABLECOUNT; i++ ) {

        kReadIOAPICRedirectionTable( i, &stEntry );
        stEntry.bInterruptMask = IOAPIC_INTERRUPT_MASK;
        kWriteIOAPICRedirectionTable( i, &stEntry );

    }

}

// I/O APIC의 I/O 리다이렉션 테이블을 초기화
void kInitializeIORedirectionTable( void ) {

    MPCONFIGURATIONMANAGER* pstMPManager;
    MPCONFIGURATIONTABLEHEADER* pstMPHeader;
    IOINTERRUPTASSIGNMENTENTRY* pstIOAssignmentEntry;
    IOREDIRECTIONTABLE stIORedirectionEntry;
    QWORD qwEntryAddress;
    BYTE bEntryType;
    BYTE bDestination;
    int i;

    kMemSet( &gs_stIOAPICManager, 0, sizeof( gs_stIOAPICManager ) );

    kGetIOAPICBaseAddressOfISA();

    for( i = 0; i < IOAPIC_MAXIRQTOINTINMAPCOUNT; i++ ) {

        gs_stIOAPICManager.vbIRQToINTINMap[ i ] = 0xFF;

    }

    kMaskAllInterruptInIOAPIC();

    pstMPManager = kGetMPConfigurationManager();
    pstMPHeader = pstMPManager->pstMPConfigurationTableHeader;
    qwEntryAddress = pstMPManager->qwBaseEntryStartAddress;

    for( i = 0; i < pstMPHeader->wEntryCount; i++ ) {

        bEntryType = *( BYTE* ) qwEntryAddress;
        switch( bEntryType ) {

        case MP_ENTRYTYPE_IOINTERRUPTASSIGNMENT:
            pstIOAssignmentEntry = ( IOINTERRUPTASSIGNMENTENTRY* ) qwEntryAddress;

            if( ( pstIOAssignmentEntry->bSourceBUSID == pstMPManager->bISABusID ) && ( pstIOAssignmentEntry->bInterruptType == MP_INTERRUPTTYPE_INT ) ) {

                if( pstIOAssignmentEntry->bSourceBUSIRQ == 0 ) {

                    bDestination = 0xFF;

                } else {

                    bDestination = 0x00;

                }

                kSetIOAPICRedirectionEntry( &stIORedirectionEntry, bDestination, 0x00, IOAPIC_TRIGGERMODE_EDGE | IOAPIC_POLARITY_ACTIVEHIGH | IOAPIC_DESTINATIONMODE_PHYSICALMODE | IOAPIC_DELIVERYMODE_FIXED, PIC_IRQSTARTVECTOR + pstIOAssignmentEntry->bSourceBUSIRQ );

                kWriteIOAPICRedirectionTable( pstIOAssignmentEntry->bDestinationIOAPICINTIN, &stIORedirectionEntry );

                gs_stIOAPICManager.vbIRQToINTINMap[ pstIOAssignmentEntry->bSourceBUSIRQ ] = pstIOAssignmentEntry->bDestinationIOAPICINTIN;

            }

            qwEntryAddress += sizeof( IOINTERRUPTASSIGNMENTENTRY );
            
            break;

        case MP_ENTRYTYPE_PROCESSOR:
            qwEntryAddress += sizeof( PROCESSORENTRY );

            break;

        case MP_ENTRYTYPE_BUS:
        case MP_ENTRYTYPE_IOAPIC:
        case MP_ENTRYTYPE_LOCALINTERRUPTASSIGNMENT:
            qwEntryAddress += 8;

            break;

        }

    }

}

// IRQ와 I/O APIC의 인터럽트 핀(INTIN) 간의 매핑 관계를 출력
void kPrintIRQToINTINMap( void ) {

    int i;

    kPrintf( "IRQ To I/O APIC INT IN Mapping Table: \n" );

    for( i = 0; i < IOAPIC_MAXIRQTOINTINMAPCOUNT; i++ ) {

        kPrintf( "  IRQ[%d] -> INIIN[%d]\n", i, gs_stIOAPICManager.vbIRQToINTINMap[ i ] );

    }

}