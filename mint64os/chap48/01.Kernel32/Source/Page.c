#include "Page.h"
#include "../../02.Kernel64/Source/Task.h"

#define DYNAMICMEMORY_START_ADDRESS ( ( TASK_STACKPOOLADDRESS + 0x1fffff ) & 0xffe00000 )

void kInitializePageTables( void ) {
    PML4TENTRY* pstPML4TEntry;
    PDPTENTRY* pstPDPTEntry;
    PDENTRY* pstPDEntry;
    DWORD dwMappingAddress;
    DWORD dwPageEntryFlags;
    int i;

    pstPML4TEntry = ( PML4TENTRY* ) 0x100000;
    kSetPageEntryData( &( pstPML4TEntry[ 0 ] ), 0x00, 0x101000, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_US, 0x0 );
    for( i = 1; i < PAGE_MAXENTRYCOUNT; i++ ) {

        kSetPageEntryData( &( pstPML4TEntry[ i ] ), 0, 0, 0, 0 );

    }

    pstPDPTEntry = ( PDPTENTRY* ) 0x101000;
    for( i = 0; i < 64; i++ ){

        kSetPageEntryData( &( pstPDPTEntry[ i ] ), 0, 0x102000 + ( i * PAGE_TABLESIZE ), PAGE_FLAGS_DEFAULT | PAGE_FLAGS_US, 0x0 );

    }
    for( i = 64; i < PAGE_MAXENTRYCOUNT; i++ ) {

        kSetPageEntryData( &( pstPDPTEntry[ i ] ), 0, 0, 0, 0 );

    }

    pstPDEntry = ( PDENTRY* ) 0x102000;
    dwMappingAddress = 0x0;
    for( i = 0; i < PAGE_MAXENTRYCOUNT * 64 ; i++ ) {

        if( i < ( ( DWORD ) DYNAMICMEMORY_START_ADDRESS / PAGE_DEFAULTSIZE ) ) {

            dwPageEntryFlags = PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS;

        } else {

            dwPageEntryFlags = PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS | PAGE_FLAGS_US;

        }

        kSetPageEntryData( &( pstPDEntry[ i ] ), ( i * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, dwMappingAddress, dwPageEntryFlags, 0x0 );
        dwMappingAddress += PAGE_DEFAULTSIZE;

    }
}

void kSetPageEntryData( PTENTRY* pstEntry, DWORD dwUpperBaseAddress, DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags ) {
    pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
    pstEntry->dwUpperBaseAddressAndEXB = ( dwUpperBaseAddress & 0xff ) | dwUpperFlags;
}