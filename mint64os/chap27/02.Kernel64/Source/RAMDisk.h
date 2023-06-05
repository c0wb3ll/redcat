#ifndef __RAMDISK_H__
#define __RAMDISK_H__

#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"

#define RDD_TOTALSECTORCOUNT            ( 8 * 1024 * 1024 / 512 )

#pragma pack( push, 1 )

// 램 디스크의 자료구조를 저장하는 구조체
typedef struct kRDDManagerSturct {

    BYTE* pbBuffer;
    DWORD dwTotalSectorCount;
    MUTEX stMutex;

} RDDMANAGER;

#pragma pack( pop )

// 함수
BOOL kInitializeRDD( DWORD dwTotalSectorCount );
BOOL kReadRDDInformation( BOOL bPrimary, BOOL bMaster, HDDINFORMATION* pstHDDInformation );
int kReadRDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer );
int kWriteRDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, char* pcBuffer );

#endif /*__RAMDISK_H__*/