#include "Types.h"
#include "Page.h"
#include "ModeSwitch.h"

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area(void);
BOOL kIsMemoryEnough(void);
void kCopyKernel64ImageTo2Mbyte( void );

void Main(void){

    DWORD dwEAX, dwEBX, dwECX, dwEDX;
    char vcVendorString[ 13 ] = { 0, };

    kPrintString(0x00, 0x03, "[*] Protected Mode C Language Kernel Started [PASS]");

    kPrintString(0x00, 0x04, "[*] Minimum Memory Size Check [    ]");
    if(kIsMemoryEnough() == FALSE){

        kPrintString(0x1f, 0x04, "FAIL");
        kPrintString(0x00, 0x05, "[-] Not Enough Memory, MINT64 OS Requires Over 64Byte Memory");
        
        while(1);

    } else { kPrintString(0x1f, 0x04, "PASS"); }

    kPrintString(0x0, 0x05, "[*] IA-32e Kernel Area Initialization Complete [    ]");
    if (kInitializeKernel64Area() == FALSE){

        kPrintString(0x30, 0x05, "FAIL");
        kPrintString(0x00, 0x06, "[-] Kernel Area Initialization Fail");

        while(1);

    } else { kPrintString(0x30, 0x05, "PASS"); }

    kPrintString(0x0, 0x06, "[*] IA-32e Page Tables Initialize [    ]");
    kInitializePageTables();
    kPrintString(0x23, 0x06, "PASS");

    kReadCPUID(0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX);
    *( DWORD * ) vcVendorString = dwEBX;
    *( ( DWORD * ) vcVendorString + 1 ) = dwEDX;
    *( ( DWORD * ) vcVendorString + 2 ) = dwECX;

    kPrintString( 0x0, 0x07, "[*] Processor Vendor String : ");
    kPrintString( 0x1e, 0x07, vcVendorString );

    kReadCPUID( 0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX );
    kPrintString( 0x0, 0x08, "[*] 64bit Mode Support Check [    ]");
    if( dwEDX & ( 1 << 29 )) {
        kPrintString( 0x1e, 0x08, "PASS" );
    }
    else {
        kPrintString( 0x1e, 0x08, "FAIL" );
        kPrintString( 0x0, 0x09, "[-] This Processor does not support 64bit mode");
        while(1);
    }

    kPrintString( 0x00, 0x09, "[*] Copy IA-32e Kernel To 2M Address [    ]");
    kCopyKernel64ImageTo2Mbyte();
    kPrintString( 0x26, 0x09, "PASS");

    kPrintString( 0x00, 0x0a, "[*] Trying Switch To IA-32e Mode...");
    kSwitchAndExecute64bitKernel();
    
    while(1);

}

void kPrintString(int iX, int iY, const char* pcString){
    
    CHARACTER* pstScreen = (CHARACTER*) 0xb8000; 
    int i;

    pstScreen += (iY * 0x50) + iX;

    for(i = 0; pcString[i] != 0; i++) {

        pstScreen[i].bCharactor = pcString[i];

    }
}

BOOL kInitializeKernel64Area(void){
    
    DWORD* pdwCurrentAddress;
    pdwCurrentAddress = (DWORD*) 0x100000;

    while((DWORD) pdwCurrentAddress < 0x600000){
        
        *pdwCurrentAddress = 0x00;

        if(*pdwCurrentAddress != 0) { return FALSE; }

        pdwCurrentAddress++;

    }

    return TRUE;

}

BOOL kIsMemoryEnough(void){
    
    DWORD* pdwCurrentAddress;

    pdwCurrentAddress = (DWORD*) 0x100000;

    while((DWORD) pdwCurrentAddress < 0x4000000){
        
        *pdwCurrentAddress = 0x12345678;

        if(*pdwCurrentAddress != 0x12345678){ return FALSE; }

        pdwCurrentAddress += (0x100000 / 4);

    }

    return TRUE;

}

void kCopyKernel64ImageTo2Mbyte( void ) {

    WORD wKernel32SectorCount, wTotalKernelSectorCount;
    DWORD* pdwSourceAddress, * pdwDestinationAddress;
    int i;

    wTotalKernelSectorCount = *( ( WORD* ) 0x7c05);
    wKernel32SectorCount = *( ( WORD*) 0x7c07);

    pdwSourceAddress = ( DWORD* ) ( 0x10000 + (wKernel32SectorCount * 0x200) );
    pdwDestinationAddress = ( DWORD* ) 0x200000;

    for (i = 0; i < 512 * ( wTotalKernelSectorCount - wKernel32SectorCount) / 4; i++) {

        *pdwDestinationAddress = *pdwSourceAddress;
        pdwDestinationAddress++;
        pdwSourceAddress++;

    }

}