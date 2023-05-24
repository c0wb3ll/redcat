#include "FileSystem.h"
#include "HardDisk.h"
#include "DynamicMemory.h"

static FILESYSTEMMANAGER gs_stFileSystemManager;
static BYTE gs_vbTempBuffer[ FILESYSTEM_SECTORSPERCLUSTER * 512 ];

fReadHDDInformation gs_pfReadHDDInformation = NULL;
fReadHDDSector gs_pfReadHDDSector = NULL;
fWriteHDDSector gs_pfWriteHDDSector = NULL;


// 파일 시스템 초기화
BOOL kInitializeFileSystem( void ) {

    kMemSet( &gs_stFileSystemManager, 0, sizeof( gs_stFileSystemManager ) );
    kInitializeMutex( &( gs_stFileSystemManager.stMutex ) );

    if( kInitializeHDD() == TRUE ) {

        gs_pfReadHDDInformation = kReadHDDInformation;
        gs_pfReadHDDSector = kReadHDDSector;
        gs_pfWriteHDDSector = kWriteHDDSector;

    } else {

        return FALSE;

    }

    if( kMount() == FALSE ) {

        return FALSE;

    }

    return TRUE;

}

// 저수준 함수
// 하드디스크에서 MBR을 읽어 민트 파일 시스템인지 확인, 민트파일 시스템이면 각종 정보를 읽어 자료구조에 삽입
BOOL kMount( void ) {

    MBR* pstMBR;

    kLock( &( gs_stFileSystemManager.stMutex ) );

    if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE ) {

        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;

    }

    pstMBR = ( MBR* ) gs_vbTempBuffer;
    if( pstMBR->dwSignature != FILESYSTEM_SIGNATURE) {

        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;

    }

    gs_stFileSystemManager.bMounted = TRUE;

    gs_stFileSystemManager.dwReservedSectorCount = pstMBR->dwReservedSectorCount;
    gs_stFileSystemManager.dwClusterLinkAreaStartAddress = pstMBR->dwReservedSectorCount + 1;
    gs_stFileSystemManager.dwClusterLinkAreaSize = pstMBR->dwClusterLinkSectorCount;
    gs_stFileSystemManager.dwDataAreaStartAddress = pstMBR->dwReservedSectorCount + pstMBR->dwClusterLinkSectorCount + 1;
    gs_stFileSystemManager.dwTotalClusterCount = pstMBR->dwTotalClusterCount;

    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return TRUE;

}

// 하드디스크에 파일 시스템 생성
BOOL kFormat( void ) {

    HDDINFORMATION* pstHDD;
    MBR* pstMBR;
    DWORD dwTotalSectorCount, dwRemainSectorCount;
    DWORD dwMaxClusterCount, dwClusterCount;
    DWORD dwClusterLinkSectorCount;
    DWORD i;

    kLock( &( gs_stFileSystemManager.stMutex ) );

    pstHDD = ( HDDINFORMATION* ) gs_vbTempBuffer;
    if( gs_pfReadHDDInformation(TRUE, TRUE, pstHDD) == FALSE ) {

        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;

    }
    dwTotalSectorCount = pstHDD->dwTotalSectors;

    dwMaxClusterCount = dwTotalSectorCount / FILESYSTEM_SECTORSPERCLUSTER;

    dwClusterLinkSectorCount = ( dwMaxClusterCount + 127 ) / 128;

    dwRemainSectorCount = dwTotalSectorCount - dwClusterLinkSectorCount - 1;
    dwClusterCount = dwRemainSectorCount / FILESYSTEM_SECTORSPERCLUSTER;

    dwClusterLinkSectorCount = ( dwClusterCount + 127 ) / 128;

    if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE ) {

        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;

    }

    pstMBR = ( MBR* ) gs_vbTempBuffer;
    kMemSet( pstMBR->vstPartition, 0, sizeof( pstMBR->vstPartition ) );
    pstMBR->dwSignature = FILESYSTEM_SIGNATURE;
    pstMBR->dwReservedSectorCount = 0;
    pstMBR->dwClusterLinkSectorCount = dwClusterLinkSectorCount;
    pstMBR->dwTotalClusterCount = dwClusterCount;

    if( gs_pfWriteHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE ) {

        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;

    }

    kMemSet( gs_vbTempBuffer, 0, 512 );
    for( i = 0; i < ( dwClusterLinkSectorCount + FILESYSTEM_SECTORSPERCLUSTER ); i++ ) {

        if( i == 0 ) {

            ( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_LASTCLUSTER;

        } else {

            ( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_FREECLUSTER;

        }

        if( gs_pfWriteHDDSector( TRUE, TRUE, i + 1, 1, gs_vbTempBuffer ) == FALSE ) {

            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return FALSE;

        }

    }

    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return TRUE;

}

// 파일 시스템에 연결된 하드 디스크의 정보를 반환
BOOL kGetHDDInformation( HDDINFORMATION* pstInformation ) {

    BOOL bResult;

    kLock( &( gs_stFileSystemManager.stMutex ) );

    bResult = gs_pfReadHDDInformation( TRUE, TRUE, pstInformation );

    kUnlock( &( gs_stFileSystemManager.stMutex ) );

    return bResult;

}

// 클러스터 링크 테이블 내의 오프셋에서 한 섹터를 읽음
BOOL kReadClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer ) {

    return gs_pfReadHDDSector( TRUE, TRUE, dwOffset + gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );

}

// 클러스터 링크 테이블 내의 오프셋에 한 섹테럴 씀
BOOL kWriteClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer ) {

    return gs_pfWriteHDDSector( TRUE, TRUE, dwOffset + gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );

}

// 데이터 영역의 오프셋에서 한 클러스트를 읽음
BOOL kReadCluster( DWORD dwOffset, BYTE* pbBuffer ) {

    return gs_pfReadHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) + gs_stFileSystemManager.dwDataAreaStartAddress, FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );

}

// 데이터 영역의 오프셋에 한 클러스터를 씀
BOOL kWriteCluster( DWORD dwOffset, BYTE* pbBuffer ) {

    return gs_pfWriteHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) + gs_stFileSystemManager.dwDataAreaStartAddress, FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );

}

// 클러스터 링크 테이블 영역에서 빈 클러스터를 검색
DWORD kFindFreeCluster( void ) {

    DWORD dwLinkCountInSector;
    DWORD dwLastSectorOffset, dwCurrentSectorOffset;
    DWORD i, j;

    if( gs_stFileSystemManager.bMounted == FALSE ) {

        return FILESYSTEM_LASTCLUSTER;

    }

    dwLastSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;

    for( i = 0; i < gs_stFileSystemManager.dwClusterLinkAreaSize; i++ ) {

        if( ( dwLastSectorOffset + i ) == ( gs_stFileSystemManager.dwClusterLinkAreaSize - 1 ) ) {

            dwLinkCountInSector = gs_stFileSystemManager.dwTotalClusterCount % 128;

        } else {

            dwLinkCountInSector = 128;

        }

        dwCurrentSectorOffset = ( dwLastSectorOffset + i ) % gs_stFileSystemManager.dwClusterLinkAreaSize;
        if( kReadClusterLinkTable( dwCurrentSectorOffset, gs_vbTempBuffer ) == FALSE ) {

            return FILESYSTEM_LASTCLUSTER;

        }

        for( j = 0; j < dwLinkCountInSector; j ++ ) {

            if( ( ( DWORD* ) gs_vbTempBuffer )[ j ] == FILESYSTEM_FREECLUSTER ) {

                break;

            }

        }

        if( j != dwLinkCountInSector ) {

            gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = dwCurrentSectorOffset;

            return ( dwCurrentSectorOffset * 128 ) + j;

        }

    }

    return FILESYSTEM_LASTCLUSTER;

}

// 클러스터 링크테이블의 값을 설정
BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData ) {

    DWORD dwSectorOffset;

    if( gs_stFileSystemManager.bMounted == FALSE ) {

        return FALSE;

    }

    dwSectorOffset = dwClusterIndex / 128;

    if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE ) {

        return FALSE;

    }

    ( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ] = dwData;

    if( kWriteClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE ) {

        return FALSE;

    }

    return TRUE;

}

// 클러스터 링크 테이블의 값을 반환
BOOL kGetClusterLinkData( DWORD dwClusterIndex, DWORD* pdwData ) {

    DWORD dwSectorOffset;

    if( gs_stFileSystemManager.bMounted == FALSE ) {

        return FALSE;

    }

    dwSectorOffset = dwClusterIndex / 128;

    if( dwSectorOffset > gs_stFileSystemManager.dwClusterLinkAreaSize ) {

        return FALSE;

    }

    if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE ) {

        return FALSE;
        
    }

    *pdwData = ( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ];

    return TRUE;

}

// 루트 디렉토리에서 빈 디레토리 엔트리 반환
int kFindFreeDirectoryEntry( void ) {

    DIRECTORYENTRY* pstEntry;
    int i;

    if( gs_stFileSystemManager.bMounted == FALSE ) {

        return -1;

    }

    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE ) {

        return -1;

    }

    pstEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    for( i = 0; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT; i++ ) {

        if( pstEntry[ i ].dwStartClusterIndex == 0 ) {

            return i;

        }

    }

    return -1;

}

// 루트 디렉터리의 해당 인덱스에 디렉터리 엔트리를 설정
BOOL kSetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry ) {

    DIRECTORYENTRY* pstRootEntry;

    if( ( gs_stFileSystemManager.bMounted == FALSE ) || ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) ) {

        return FALSE;

    }

    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE ) {

        return FALSE;

    }

    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstRootEntry + iIndex, pstEntry, sizeof( DIRECTORYENTRY ) );

    if( kWriteCluster( 0, gs_vbTempBuffer ) == FALSE ) {

        return FALSE;

    }

    return TRUE;

}

// 루트 디렉터리의 해당 인덱스의 위치하는 디렉터리 엔트리를 반환
BOOL kGetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry ) {

    DIRECTORYENTRY* pstRootEntry;

    if( ( gs_stFileSystemManager.bMounted == FALSE ) || ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) ) {

        return FALSE;
        
    }

    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE ) {

        return FALSE;

    }

    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstEntry, pstRootEntry + iIndex, sizeof( DIRECTORYENTRY ) );

    return TRUE;

}

// 루트 디렉터리에서 파일 이름이 일치하는 엔트리를 찾아서 인덱스를 반환
int kFindDirectoryEntry( const char* pcFileName, DIRECTORYENTRY* pstEntry ) {

    DIRECTORYENTRY* pstRootEntry;
    int i;
    int iLength;

    if( gs_stFileSystemManager.bMounted == FALSE ) {

        return -1;
    
    }

    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE ) {
        
        return -1;
    
    }

    iLength = kStrLen( pcFileName );
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    for( i = 0 ; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT ; i++ ) {
        
        if( kMemCmp( pstRootEntry[ i ].vcFileName, pcFileName, iLength ) == 0 ) {
            
            kMemCpy( pstEntry, pstRootEntry + i, sizeof( DIRECTORYENTRY ) );
            return i;

        }
    
    }
    
    return -1;

}

// 파일 시스템의 정보를 반환
void kGetFileSystemInformation( FILESYSTEMMANAGER* pstManager ) {

    kMemCpy( pstManager, &gs_stFileSystemManager, sizeof( gs_stFileSystemManager ) );

}