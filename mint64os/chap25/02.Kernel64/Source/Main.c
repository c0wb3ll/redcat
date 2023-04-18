#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Task.h"
#include "PIT.h"
#include "DynamicMemory.h"
#include "HardDisk.h"

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

    kPrintf( "[*] TCB Pool And Scheduler Intialize [Pass]\n");
    iCursorY++;
    kInitializeScheduler();

    kPrintf( "[*] Dynamic Memory Initialize [Pass]\n");
    iCursorY++;
    kInitializeDynamicMemory();

    kInitializePIT( MSTOCOUNT( 1 ), 1 );

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

    kPrintf( "[*] HDD Initialize [    ]" );
    if( kInitializeHDD() == TRUE ) {

        kSetCursor( 0x14, iCursorY++ );
        kPrintf( "Pass\n" );

    } else {

        kSetCursor( 0x14, iCursorY++ );
        kPrintf( "Fail\n" );

    }

    kCreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE, 0, 0, ( QWORD ) kIdleTask );
    kStartConsoleShell();

}