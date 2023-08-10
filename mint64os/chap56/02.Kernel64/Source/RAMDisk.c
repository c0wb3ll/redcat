#include "RAMDisk.h"
#include "Utility.h"
#include "DynamicMemory.h"

static RDDMANAGER gs_stRDDManager;

// 램 디스크 디바이스 드라이버 초기화 함수
BOOL kInitializeRDD( DWORD dwTotalSectorCount ) {

    kMemSet( &gs_stRDDManager, 0, sizeof( gs_stRDDManager ) );

    gs_stRDDManager.pbBuffer = ( BYTE* ) kAllocateMemory( dwTotalSectorCount * 512 );
    if( gs_stRDDManager.pbBuffer == NULL ) {

        return FALSE;

    }

    gs_stRDDManager.dwTotalSectorCount = dwTotalSectorCount;
    kInitializeMutex( &( gs_stRDDManager.stMutex ) );
    
    return TRUE;

}

// 램 디스크의 정보를 반환
BOOL kReadRDDInformation( BOOL bPrimary, BOOL bMaster, HDDINFORMATION* pstHDDInformation ) {

    kMemSet( pstHDDInformation, 0, sizeof( HDDINFORMATION ) );

    pstHDDInformation->dwTotalSectors = gs_stRDDManager.dwTotalSectorCount;
    kMemCpy( pstHDDInformation->vwSerialNumber, "0000-0000", 9 );
    kMemCpy( pstHDDInformation->vwModelNumber, "MINT RAM Disk v1.0", 18 );

    return TRUE;

}

// 램 디스크에서 여러 섹터를 읽어서 반환
int kReadRDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer ) {

    int iRealReadCount;

    iRealReadCount = MIN( gs_stRDDManager.dwTotalSectorCount - ( dwLBA + iSectorCount ), iSectorCount );

    kMemCpy( pcBuffer, gs_stRDDManager.pbBuffer + ( dwLBA * 512 ), iRealReadCount * 512 );

    return iRealReadCount;

}

// 램 디스크에서 여러 섹터를 씀
int kWriteRDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer ) {

    int iRealWriteCount;

    iRealWriteCount = MIN( gs_stRDDManager.dwTotalSectorCount - ( dwLBA + iSectorCount ), iSectorCount );

    kMemCpy( gs_stRDDManager.pbBuffer + ( dwLBA * 512 ), pcBuffer, iRealWriteCount * 512 );

    return iRealWriteCount;

}