#include "CacheManager.h"
#include "FileSystem.h"
#include "DynamicMemory.h"

static CACHEMANAGER gs_stCacheManager;

// 파일 시스템 캐시를 초기화
BOOL kInitializeCacheManager( void ) {

    int i;

    kMemSet( &gs_stCacheManager, 0, sizeof( gs_stCacheManager ) );

    gs_stCacheManager.vdwAccessTime[ CACHE_CLUSTERLINKTABLEAREA ] = 0;
    gs_stCacheManager.vdwAccessTime[ CACHE_DATAAREA ] = 0;

    gs_stCacheManager.vdwMaxCount[ CACHE_CLUSTERLINKTABLEAREA ] = CACHE_MAXCLUSTERLINKTABLEAREACOUNT;
    gs_stCacheManager.vdwMaxCount[ CACHE_DATAAREA ] = CACHE_MAXDATAAREACOUNT;

    gs_stCacheManager.vpbBuffer[ CACHE_CLUSTERLINKTABLEAREA ] = ( BYTE* ) kAllocateMemory( CACHE_MAXCLUSTERLINKTABLEAREACOUNT * 512 );
    if( gs_stCacheManager.vpbBuffer[ CACHE_CLUSTERLINKTABLEAREA ] == NULL ) {

        return FALSE;

    }

    for( i = 0; i < CACHE_MAXCLUSTERLINKTABLEAREACOUNT; i++ ) {

        gs_stCacheManager.vvstCacheBuffer[ CACHE_CLUSTERLINKTABLEAREA ][ i ].pbBuffer = gs_stCacheManager.vpbBuffer[ CACHE_CLUSTERLINKTABLEAREA ] + ( i * 512 );

        gs_stCacheManager.vvstCacheBuffer[ CACHE_CLUSTERLINKTABLEAREA ][ i ].dwTag = CACHE_INVALIDTAG;

    }

    gs_stCacheManager.vpbBuffer[ CACHE_DATAAREA ] = ( BYTE* ) kAllocateMemory( CACHE_MAXDATAAREACOUNT * FILESYSTEM_CLUSTERSIZE );
    if( gs_stCacheManager.vpbBuffer[ CACHE_DATAAREA ] == NULL ) {

        kFreeMemory( gs_stCacheManager.vpbBuffer[ CACHE_CLUSTERLINKTABLEAREA ] );
        return FALSE;

    }

    for( i = 0; i < CACHE_MAXDATAAREACOUNT; i++ ) {

        gs_stCacheManager.vvstCacheBuffer[ CACHE_DATAAREA ][ i ].pbBuffer = gs_stCacheManager.vpbBuffer[ CACHE_DATAAREA ] + ( i * FILESYSTEM_CLUSTERSIZE );

        gs_stCacheManager.vvstCacheBuffer[ CACHE_DATAAREA ][ i ].dwTag = CACHE_INVALIDTAG;

    }

    return TRUE;

}

// 캐시 버퍼에서 빈 것을 찾아서 반환
CACHEBUFFER* kAllocateCacheBuffer( int iCacheTableIndex ) {

    CACHEBUFFER* pstCacheBuffer;
    int i;

    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX ) {

        return FALSE;

    }

    kCutDownAccessTime( iCacheTableIndex );

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( i = 0; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ]; i++ ) {

        if( pstCacheBuffer[ i ].dwTag == CACHE_INVALIDTAG ) {

            pstCacheBuffer[ i ].dwTag == CACHE_INVALIDTAG - 1;

            pstCacheBuffer[ i ].dwAccessTime = gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ]++;

            return &( pstCacheBuffer[ i ] );

        }

    }

    return NULL;

}

// 캐시 버퍼에서 태그가 일치하는 것을 찾아서 반환 
CACHEBUFFER* kFindCacheBuffer( int iCacheTableIndex, DWORD dwTag ) {

    CACHEBUFFER* pstCacheBuffer;
    int i;

    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX ) {

        return FALSE;

    }

    kCutDownAccessTime( iCacheTableIndex );

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( i = 0; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ]; i++ ) {

        if( pstCacheBuffer[ i ].dwTag == dwTag ) {

            pstCacheBuffer[ i ].dwAccessTime = gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ]++;

            return &( pstCacheBuffer[ i ] );

        }

    }

    return NULL;

}

// 접근한 시간을 전체적으로 낮춤
static void kCutDownAccessTime( int iCacheTableIndex ) {

    CACHEBUFFER stTemp;
    CACHEBUFFER* pstCacheBuffer;
    BOOL bSorted;
    int i, j;

    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX ) {

        return ;
        
    }

    if( gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ] < 0xfffffffe ) {

        return ;

    }

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( j = 0; j < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ] - 1; j++ ) {

        bSorted = TRUE;
        for( i = 0; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ] - 1 - j; i++ ) {

            if( pstCacheBuffer[ i ].dwAccessTime > pstCacheBuffer[ i + 1 ].dwAccessTime ) {

                bSorted = FALSE;
                
                kMemCpy( &stTemp, &( pstCacheBuffer[ i ] ), sizeof( CACHEBUFFER ) );
                kMemCpy( &( pstCacheBuffer[ i ] ), &( pstCacheBuffer[ i + 1 ] ), sizeof(CACHEBUFFER ) );
                kMemCpy( &( pstCacheBuffer[ i + 1 ] ), &stTemp, sizeof( CACHEBUFFER ) );

            }

        }

        if( bSorted == TRUE ) {

            break;

        }

    }

    for( i = 0; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ]; i++ ) {

        pstCacheBuffer[ i ].dwAccessTime = i;

    }

    gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ] = i;

}

// 캐시 버퍼에서 내보낼 것을 찾음
CACHEBUFFER* kGetVictimInCacheBuffer( int iCacheTableIndex ) {

    DWORD dwOldTime;
    CACHEBUFFER* pstCacheBuffer;
    int iOldIndex;
    int i;

    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX ) {

        return FALSE;

    }

    iOldIndex = -1;
    dwOldTime = 0xFFFFFFFF;

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( i = 0; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ]; i++ ) {

        if( pstCacheBuffer[ i ].dwTag == CACHE_INVALIDTAG ) {

            iOldIndex = i;
            break;

        }

        if( pstCacheBuffer[ i ].dwAccessTime < dwOldTime ) {

            dwOldTime = pstCacheBuffer[ i ].dwAccessTime;
            iOldIndex = i;

        }

    }

    if( iOldIndex == -1 ) {

        kPrintf( " Cache Buffer Find Error\n" );

        return NULL;

    }

    pstCacheBuffer[ iOldIndex ].dwAccessTime = gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ]++;

    return &( pstCacheBuffer[ iOldIndex ] );

}

// 캐시버퍼에 내용을 모두 비움
void kDiscardAllCacheBuffer( int iCacheTableIndex ) {

    CACHEBUFFER* pstCacheBuffer;
    int i;

    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( i = 0; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ]; i++ ) {

        pstCacheBuffer[ i ].dwTag = CACHE_INVALIDTAG;

    }

    gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ] = 0;

}

// 캐시 버퍼의 포인터와 최대 개수를 반환
BOOL kGetCacheBufferAndCount( int iCacheTableIndex, CACHEBUFFER** ppstCacheBuffer, int* piMaxCount ) {

    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX ) {

        return FALSE;

    }

    *ppstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    *piMaxCount = gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ];
    
    return TRUE;

}