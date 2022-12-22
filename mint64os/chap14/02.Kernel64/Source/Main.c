#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "PIC.h"

void kPrintString( int iX, int iY, const char* pcString);

void Main( void ) {

    char vcTemp[ 2 ] = { 0, };
    BYTE bFlags;
    BYTE bTemp;
    int i = 0;
    KEYDATA stData;

    kPrintString( 0x23, 0x0a, "Complete");
    kPrintString( 0x00, 0x0b, "[*] IA-32e C Language Kernel Start [PASS]");

    kPrintString( 0x00, 0x0c, "[*] GDT Initialize And Switch For IA-32e Mode [    ]");
    kInitializeGDTTableAndTSS();
    kLoadGDTR( GDTR_STARTADDRESS );
    kPrintString( 0x2f, 0x0c, "PASS" );

    kPrintString( 0x00, 0x0d, "[*] TSS Segment Load [    ]");
    kLoadTR( GDT_TSSSEGMENT );
    kPrintString( 0x16, 0x0d, "PASS");

    kPrintString( 0x00, 0x0e, "[*] IDT Initialize [    ]");
    kInitializeIDTTables();
    kLoadIDTR( IDTR_STARTADDRESS );
    kPrintString( 0x14, 0x0e, "PASS");

    kPrintString( 0x00, 0x0f, "[*] Keyboard Activate And Queue Initialize [    ]");

    if( kInitializeKeyboard() == TRUE) {
        kPrintString( 0x2c, 0x0f, "PASS");
        kChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else{
        kPrintString( 0x2c, 0x0f, "FAIL");
        while( 1 );
    }

    kPrintString( 0x0, 0x10, "[*] PIC Controller And Interrupt Initialize [    ]");
    kInitializePIC();
    kMaskPICInterrupt( 0 );
    kEnableInterrupt();
    kPrintString( 0x2d, 0x10, "PASS");

    while( 1 ) {
        if( kGetKeyFromKeyQueue( &stData ) == TRUE ) {
            if( stData.bFlags & KEY_FLAGS_DOWN ) {
                vcTemp[ 0 ] = stData.bASCIICode;
                kPrintString( i++, 17, vcTemp);

                if( vcTemp[0] == '0' ) { bTemp = bTemp / 0; }
            }
        }
    }
}

void kPrintString(int iX, int iY, const char* pcString){
    
    CHARACTER* pstScreen = (CHARACTER*) 0xb8000; 
    int i;

    pstScreen += (iY * 0x50) + iX;

    for(i = 0; pcString[i] != 0; i++) {

        pstScreen[i].bCharactor = pcString[i];

    }
}