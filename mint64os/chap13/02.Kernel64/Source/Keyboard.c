#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"

BOOL kIsOutputBufferFull( void ) {

	if ( kInPortByte( 0x64 ) & 0x01 ) { return TRUE; }
	return FALSE;
}

BOOL kIsInputBufferFull( void ) {

	if ( kInPortByte( 0x64 ) & 0x02 ) { return TRUE; }
	return FALSE;

}

BOOL kActivateKeyboard( void ) {
	int i;
	int j;

	kOutPortByte( 0x64, 0xAE );

	for( i = 0 ; i < 0xFFFF ; i++ ) { if( kIsInputBufferFull() == FALSE ) { break; } }

	kOutPortByte( 0x60, 0xF4 );

	for( j = 0 ; j < 100 ; j++ ) {

		for( i = 0 ; i < 0xFFFF ; i++ ) { if( kIsOutputBufferFull() == TRUE ) { break; } }
		if( kInPortByte( 0x60 ) == 0xfa ) { return TRUE; }
	
    }
	
    return FALSE;

}

BYTE kGetKeyboardScanCode( void ) {

	while( kIsOutputBufferFull() == FALSE ) { ; }
	return kInPortByte( 0x60 );

}

BOOL kChangeKeyboardLED( BOOL bCapsLockOn, BOOL bNumLockOn, BOOL bScrollLockOn ) {
	
    int i, j;
	
	for( i = 0; i < 0xFFFF; i++ ) { if( kIsInputBufferFull() == FALSE ) { break; } }

	kOutPortByte( 0x60, 0xED );
	for( i = 0; i < 0xFFFF; i++ ) { if( kIsInputBufferFull() == FALSE ) { break; } }

	for( j = 0; j < 100; j++ ) { 

		for( i = 0; i < 0xFFFF; i++ ) { if( kIsOutputBufferFull() == TRUE ) { break; } }
		if( kInPortByte( 0x60 ) == 0xFA ) { break; }

	}

	if( j >= 100 ) { return FALSE; }

	kOutPortByte( 0x60, ( bCapsLockOn << 2 ) | ( bNumLockOn << 1 ) | bScrollLockOn );
	for( i = 0; i < 0xFFFF; i++ ) { if( kIsInputBufferFull() == FALSE ) { break; } }

	for( j = 0; j < 100; j++ ) { 

		for( i = 0; i < 0xFFFF; i++ ) { if( kIsOutputBufferFull() == TRUE ) { break; } }
		if( kInPortByte( 0x60 ) == 0xFA ) { break; }

	}

	if( j >= 100 ) { return FALSE; }

	return TRUE;
}

void kEnableA20Gate( void ) {

	BYTE bOutputPortData;
	int i;

	kOutPortByte( 0x64, 0xD0 );

	for(i = 0; i < 0xFFFF; i++){ if( kIsOutputBufferFull() == TRUE ) { break; } }

	bOutputPortData = kInPortByte( 0x60 );
	bOutputPortData != 0x02;

	for( i = 0; i < 0xFFFF; i++ ) { if( kIsInputBufferFull() == FALSE ) { break; } }

	kOutPortByte( 0x64, 0xD1 );
	kOutPortByte( 0x60, bOutputPortData );
}

void kReboot( void ) {
    
    int i;
    
    for( i = 0; i < 0xFFFF; i++ ){ if( kIsInputBufferFull() == FALSE ) { break; } }

    kOutPortByte( 0x64, 0xD1);
    kOutPortByte( 0x60, 0x00);

    while( 1 ) { ; }

}

static KEYBOARDMANAGER gs_stKeyboardManager = { 0, };

static KEYMAPPINGENTRY gs_vstKeyMappingTable[ KEY_MAPPINGTABLEMAXCOUNT ] = {
    /* 0x00 */  { KEY_NONE,         KEY_NONE },
    /* 0x01 */  { KEY_ESC,          KEY_ESC },
    /* 0x02 */  { '1',              '!' },
    /* 0x03 */  { '2',              '@' },
    /* 0x04 */  { '3',              '#' },
    /* 0x05 */  { '4',              '$' },
    /* 0x06 */  { '5',              '%' },
    /* 0x07 */  { '6',              '^' },
    /* 0x08 */  { '7',              '&' },
    /* 0x09 */  { '8',              '*' },
    /* 0x0a */  { '9',              '(' },
    /* 0x0b */  { '0',              ')' },
    /* 0x0c */  { '-',              '_' },
    /* 0x0d */  { '=',              '+' },
    /* 0x0e */  { KEY_BACKSPACE,    KEY_BACKSPACE },
    /* 0x0f */  { KEY_TAB,          KEY_TAB },
    /* 0x10 */  { 'q',              'Q' },
    /* 0x11 */  { 'w',              'W' },
    /* 0x12 */  { 'e',              'E' },
    /* 0x13 */  { 'r',              'R' },
    /* 0x14 */  { 't',              'T' },
    /* 0x15 */  { 'y',              'Y' },
    /* 0x16 */  { 'u',              'U' },
    /* 0x17 */  { 'i',              'I' },
    /* 0x18 */  { 'o',              'O' },
    /* 0x19 */  { 'p',              'P' },
    /* 0x1a */  { '[',              '{' },
    /* 0x1b */  { ']',              '}' },
    /* 0x1c */  { '\n',             '\n' },
    /* 0x1d */  { KEY_CTRL,         KEY_CTRL },
    /* 0x1e */  { 'a',              'A' },
    /* 0x1f */  { 's',              'S' },
    /* 0x20 */  { 'd',              'D' },
    /* 0x21 */  { 'f',              'F' },
    /* 0x22 */  { 'g',              'G' },
    /* 0x23 */  { 'h',              'H' },
    /* 0x24 */  { 'j',              'J' },
    /* 0x25 */  { 'k',              'K' },
    /* 0x26 */  { 'l',              'L' },
    /* 0x27 */  { ';',              ':' },
    /* 0x28 */  { '\'',             '\"' },
    /* 0x29 */  { '`',              '~' },
    /* 0x2a */  { KEY_LSHIFT,       KEY_LSHIFT },
    /* 0x2b */  { '\\',             '|' },
    /* 0x2c */  { 'z',              'Z' },
    /* 0x2d */  { 'x',              'X' },
    /* 0x2e */  { 'c',              'C' },
    /* 0x2f */  { 'v',              'V' },
    /* 0x30 */  { 'b',              'B' },
    /* 0x31 */  { 'n',              'N' },
    /* 0x32 */  { 'm',              'M' },
    /* 0x33 */  { ',',              '<' },
    /* 0x34 */  { '.',              '>' },
    /* 0x35 */  { '/',              '?' },
    /* 0x36 */  { KEY_RSHIFT,       KEY_RSHIFT },
    /* 0x37 */  { '*',              '*' },
    /* 0x38 */  { KEY_LALT,         KEY_LALT },
    /* 0x39 */  { ' ',              ' ' },
    /* 0x3a */  { KEY_CAPSLOCK,     KEY_CAPSLOCK },
    /* 0x3b */  { KEY_F1,           KEY_F1 },
    /* 0x3c */  { KEY_F2,           KEY_F2 },
    /* 0x3d */  { KEY_F3,           KEY_F3 },
    /* 0x3e */  { KEY_F4,           KEY_F4 },
    /* 0x3f */  { KEY_F5,           KEY_F5 },
    /* 0x40 */  { KEY_F6,           KEY_F6 },
    /* 0x41 */  { KEY_F7,           KEY_F7 },
    /* 0x42 */  { KEY_F8,           KEY_F8 },
    /* 0x43 */  { KEY_F9,           KEY_F9 },
    /* 0x44 */  { KEY_F10,          KEY_F10 },
    /* 0x45 */  { KEY_NUMLOCK,      KEY_NUMLOCK },
    /* 0x46 */  { KEY_SCROLLLOCK,   KEY_SCROLLLOCK },
    /* 0x47 */  { KEY_HOME,         '7' },
    /* 0x48 */  { KEY_UP,           '8' },
    /* 0x49 */  { KEY_PAGEUP,       '9' },
    /* 0x4a */  { '-',              '-' },
    /* 0x4b */  { KEY_LEFT,         '4' },
    /* 0x4c */  { KEY_CENTER,       '5' },
    /* 0x4d */  { KEY_RIGHT,        '6' },
    /* 0x4e */  { '+',              '+' },
    /* 0x4f */  { KEY_END,          '1' },
    /* 0x50 */  { KEY_DOWN,         '2' },
    /* 0x51 */  { KEY_PAGEDOWN,     '3' },
    /* 0x52 */  { KEY_INS,          '0' },
    /* 0x53 */  { KEY_DEL,          '.' },
    /* 0x54 */  { KEY_NONE,         KEY_NONE },
    /* 0x55 */  { KEY_NONE,         KEY_NONE },
    /* 0x56 */  { KEY_NONE,         KEY_NONE },
    /* 0x57 */  { KEY_F11,          KEY_F11 },    
    /* 0x58 */  { KEY_F12,          KEY_F12 },
};

BOOL kIsAlphabetScanCode( BYTE bScanCode ) {
    
	if( ( 'a' <= gs_vstKeyMappingTable[ bScanCode ].bNormalCode ) && ( gs_vstKeyMappingTable[ bScanCode ].bNormalCode <= 'z' ) ) { return TRUE; }
	return FALSE;

}

BOOL kIsNumberOrSymbolScanCode( BYTE bScanCode ) {

	if( ( 2 <= bScanCode ) && ( bScanCode <= 53 ) && (kIsAlphabetScanCode( bScanCode ) == FALSE ) ) { return TRUE; }
	return FALSE;
}

BOOL kIsNumberPadScanCode( BYTE bScanCode ) {
	if( ( 71 <= bScanCode ) && ( bScanCode <= 83 ) ) { return TRUE; }
	return FALSE;
}

BOOL kIsUseCombinedCode( BYTE bScanCode ) {

	BYTE bDownScanCode;
	BOOL bUseCombinedKey = FALSE;

	bDownScanCode = bScanCode & 0x7F;

	if( kIsAlphabetScanCode( bDownScanCode ) == TRUE ) {

		if( gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn ) { bUseCombinedKey = TRUE; }
		else { bUseCombinedKey = FALSE; }

	}
	else if( kIsNumberOrSymbolScanCode( bDownScanCode ) == TRUE ) {

		if( gs_stKeyboardManager.bShiftDown == TRUE ) { bUseCombinedKey = TRUE; }
		else { bUseCombinedKey = FALSE; }

	}
    else if( ( kIsNumberPadScanCode( bDownScanCode ) == TRUE ) &&
     (gs_stKeyboardManager.bExtendedCodeIn == FALSE ) ) {
     
        if( gs_stKeyboardManager.bNumLockOn == TRUE ) { bUseCombinedKey = TRUE; }
        else { bUseCombinedKey = FALSE; }
     }
    
    return bUseCombinedKey;

}

void UpdateCombinationKeyStatusAndLED( BYTE bScanCode ) {
	
    BOOL bDown;
	BYTE bDownScanCode;
	BOOL bLEDStatusChanged = FALSE;

	if( bScanCode & 0x80 ) {
	
    	bDown = FALSE;
		bDownScanCode = bScanCode & 0x7F;
	
    }
	else {
	
    	bDown = TRUE;
		bDownScanCode = bScanCode;
	
    }

	if( ( bDownScanCode == 42 ) || ( bDownScanCode == 54 ) ){ 
        
     gs_stKeyboardManager.bShiftDown = bDown; 
        
    }
	else if( ( bDownScanCode == 58 ) && ( bDown == TRUE ) ) {
		
     gs_stKeyboardManager.bCapsLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	
    }
	else if( ( bDownScanCode == 69 ) && ( bDown == TRUE ) ) {
		
     gs_stKeyboardManager.bNumLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	
    }
	else if( ( bDownScanCode == 70 ) && ( bDown == TRUE ) ) {
	
     gs_stKeyboardManager.bScrollLockOn ^= TRUE;
		bLEDStatusChanged = TRUE;
	
    }

	if( bLEDStatusChanged == TRUE ) { kChangeKeyboardLED( gs_stKeyboardManager.bCapsLockOn, gs_stKeyboardManager.bNumLockOn, gs_stKeyboardManager.bScrollLockOn ); }

}

BOOL kConvertScanCodeToASCIICode( BYTE bScanCode, BYTE* pbASCIICode, BOOL* pbFlags ) {

	BOOL bUseCombinedKey;

	if( gs_stKeyboardManager.iSkipCountForPause > 0 ) {

		gs_stKeyboardManager.iSkipCountForPause--;
		return FALSE;

	}

	if( bScanCode == 0xE1 ){

		*pbASCIICode = KEY_PAUSE;
		*pbFlags = KEY_FLAGS_DOWN;
		gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;

		return TRUE;

	}
	else if( bScanCode == 0xE0 ) {
	
    	gs_stKeyboardManager.bExtendedCodeIn = TRUE;
		return FALSE;
	
    }

	bUseCombinedKey = kIsUseCombinedCode( bScanCode );

	if( bUseCombinedKey == TRUE ) { *pbASCIICode = gs_vstKeyMappingTable[ bScanCode & 0x7f ].bCombinedCode; }
	else { *pbASCIICode = gs_vstKeyMappingTable[ bScanCode & 0x7f ].bNormalCode; }

	if( gs_stKeyboardManager.bExtendedCodeIn == TRUE ) {
	
    	*pbFlags = KEY_FLAGS_EXTENDEDKEY;
		gs_stKeyboardManager.bExtendedCodeIn = FALSE;
	
    }
	else { *pbFlags = 0; }
	
	if( ( bScanCode & 0x80 ) == 0 ){ *pbFlags |= KEY_FLAGS_DOWN; }

	UpdateCombinationKeyStatusAndLED( bScanCode );

	return TRUE;

}