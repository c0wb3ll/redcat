#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"

void kPrintString( int iX, int iY, const char* pcString);

void Main( void ) {

    int iCursorX, iCursorY;

    kInitializeConsole( 0, 10 );
    kPrintf( "[*] Switch To IA-32e Mode Success...\n" );
    kPrintf( "[*] IA-32e C Language Kernel Start [Pass]\n" );
    kPrintf( "[*] Initialize Console [Pass]\n" );

    kGetCursor( &iCursorX, &iCursorY );
    kPrintf( "[*] GDT Initialize And Switch For IA-32e Mode [    ]" );
    kInitializeGDTTableAndTSS();
    kLoadGDTR( GDTR_STARTADDRESS );
    kSetCursor( 47, iCursorY++ );
    kPrintf( "Pass\n" );

    kPrintf( "[*] TSS Segment Load [    ]" );
    kLoadTR( GDT_TSSSEGMENT );
    kSetCursor( 22, iCursorY++ );
    kPrintf( "Pass\n" );

    kPrintf( "[*] IDT Initialize [    ]");
    kInitializeIDTTables();
    kLoadIDTR( IDTR_STARTADDRESS );
    kSetCursor( 20, iCursorY++ );
    kPrintf( "Pass\n" );

    kPrintf( "[*] Total RAM Size Check [    ]" );
    kCheckTotalRAMSize();
    kSetCursor( 26, iCursorY++ );
    kPrintf( "Pass], SIze = %d MB\n", kGetTotalRAMSize() );

    kPrintf( "[*] Keyboard Activate And Queue Initialize [    ]" );

    if( kInitializeKeyboard() == TRUE) {
        kSetCursor( 44, iCursorY++ );
        kPrintf( "Pass\n" );
        kChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else{
        kPrintf( 44, iCursorY++ );
        kPrintf( "Fail\n" );
        while( 1 );
    }

    kPrintf( "[*] PIC Controller And Interrupt Initialize [    ]");
    kInitializePIC();
    kMaskPICInterrupt( 0 );
    kEnableInterrupt();
    kSetCursor( 0x2d, iCursorY++ );
    kPrintf( "Pass\n" );

    kStartConsoleShell();

}