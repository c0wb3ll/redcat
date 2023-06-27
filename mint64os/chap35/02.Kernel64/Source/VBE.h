#ifndef __VBE_H__
#define __VBE_H__

#include "Types.h"

// 매크로
#define VBE_MODEINFOBLOCKADDRESS                0x7E00
#define VBE_STARTGRAPHICMODEFLAGADDRESS         0x7C0A

#pragma pack( push, 1 )

typedef struct kVBEInfoBlockStruct {

    // 모든 vbe 공통 부분
    WORD wModeAttribute;
    BYTE bWinAAttribute;
    BYTE bWinBAttribute;
    WORD wWinGranulity;
    WORD wWinSize;
    WORD wWinASegment;
    WORD wWinBSegment;
    DWORD dwWinFuncPtr;
    WORD wBytesPerScanLine;

    // vbe 1.2 이상 공통 부분
    WORD wXResolution;
    WORD wYResolution;
    BYTE bXCharSize;
    BYTE bYCharSize;
    BYTE bNumberOfPlane;
    BYTE bBitsPerPixel;
    BYTE bNumberOfBanks;
    BYTE bMemoryModel;
    BYTE bBankSize;
    BYTE bNumberOfImagePages;
    BYTE bReserved;

    // 다이렉트 컬러에 관련된 필드
    BYTE bRedMaskSize;
    BYTE bRedFieldPosition;
    BYTE bGreenMaskSize;
    BYTE bGreenFieldPosition;
    BYTE bBlueMaskSize;
    BYTE bBlueFieldPosition;
    BYTE bReservedMaskSize;
    BYTE bReservedFieldPosition;
    BYTE bDirectColorModeInfo;

    // vbe 2.0 이상 공통 부분
    DWORD dwPhysicalBasePointer;
    DWORD dwReserved1;
    DWORD dwReserved2;

    // vbe 3.0 이상 공통 부분
    WORD wLinearBytesPerScanLine;

    BYTE bBankNumberOfImagePages;
    BYTE bLinearNumberOfImagePages;

    BYTE bLinearRedMaskSize;
    BYTE bLinearRedFieldPosition;
    BYTE bLinearGreenMaskSize;
    BYTE bLinearGreenFieldPosition;
    BYTE bLinearBlueMaskSize;
    BYTE bLinearBlueFieldPosition;
    BYTE bLinearReservedMaskSize;
    BYTE bLinearReservedFieldPosition;
    DWORD dwMaxPixelClock;

    BYTE vbReserved[ 189 ];

} VBEMODEINFOBLOCK;

#pragma pack( pop )

// 함수
VBEMODEINFOBLOCK* kGetVBEModeInfoBlock( void );

#endif /*__VBE_H__*/