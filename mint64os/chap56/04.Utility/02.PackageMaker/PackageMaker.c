#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

// 매크로
#define BYTESOFSECTOR           512

#define PACKAGESIGNATURE        "MINT64OSPACKAGE"

#define MAXFILENAMELENGTH       24

#define DWORD                   unsigned int

// 구조체
#pragma pack( push, 1 )

typedef struct PackageItemStruct {

    char vcFileName[ MAXFILENAMELENGTH ];

    DWORD dwFileLength;

} PACKAGEITEM;

typedef struct PackageHeaderStruct {

    char vcSignature[ 16 ];

    DWORD dwHeaderSize;

    PACKAGEITEM vstItem[ 0 ];

} PACKAGEHEADER;

#pragma pack( pop )

// 함수
int AdjustInSectorSize( int iFd, int iSourceSize );
int CopyFile( int iSourceFd, int iTargetFd );

// main
int main( int argc, char* argv[] ) {

    int iSourceFd;
    int iTargetFd;
    int iSourceSize;
    int i;
    struct stat stFileData;
    PACKAGEHEADER stHeader;
    PACKAGEITEM stItem;

    if( argc < 2 ) {

        fprintf( stderr, "[ERROR] PackageMaker <filename1> <filename2> data.txt ...\n" );
        exit( -1 );

    }

    if( ( iTargetFd = open( "Package.img", O_RDWR | O_CREAT | O_TRUNC, __S_IREAD | __S_IWRITE ) ) == -1 ) {

        fprintf( stderr, "[ERROR] Package open fail\n" );
        exit( -1 );

    }

    printf( "[INFO] Create Package Header...\n" );

    memcpy( stHeader.vcSignature, PACKAGESIGNATURE, sizeof( stHeader.vcSignature ) );
    stHeader.dwHeaderSize = sizeof( PACKAGEHEADER ) + ( argc - 1 ) * sizeof( PACKAGEITEM );

    if( write( iTargetFd, &stHeader, sizeof( stHeader ) ) != sizeof( stHeader ) ) {

        fprintf( stderr, "[ERROR] Data write fail\n" );
        exit( -1 );

    }

    for( i = 1; i < argc; i++ ) {

        if( stat( argv[ i ], &stFileData ) != 0 ) {

            fprintf( stderr, "[ERROR] %s file open fail\n" );
            exit( -1 );

        }

        memset( stItem.vcFileName, 0, sizeof( stItem.vcFileName ) );
        strncpy( stItem.vcFileName, argv[ i ], sizeof( stItem.vcFileName ) );
        stItem.vcFileName[ sizeof( stItem.vcFileName ) - 1 ] = '\0';
        stItem.dwFileLength = stFileData.st_size;

        if( write( iTargetFd, &stItem, sizeof( stItem ) ) != sizeof( stItem ) ) {

            fprintf( stderr, "[ERROR] Data write fail\n" );
            exit( -1 );

        }

        printf( "[%d] file: %s, size: %d Byte\n", i, argv[ i ], stFileData.st_size );

    }

    printf( "[INFO] Create complete\n" );

    printf( "[INFO] Copy data file to package...\n" );
    iSourceSize = 0;
    for( i = 1; i < argc; i++ ) {

        if( ( iSourceFd = open( argv[ i ], O_RDONLY ) ) == -1 ) {

            fprintf( stderr, "[ERROR] %s open fail\n", argv[ 1 ] );
            exit( -1 );

        }

        iSourceSize += CopyFile( iSourceFd, iTargetFd );
        close( iSourceFd );

    }

    AdjustInSectorSize( iTargetFd, iSourceSize + stHeader.dwHeaderSize );

    printf( "[INFO] Total %d Byte copy complete\n", iSourceSize );
    printf( "[INFO] Package file create complete\n" );

    close( iTargetFd );
    
    return 0;

}

// 현재 위치에서 512 바이트 배수 위치까지 맞추어 0x00 으로 채움
int AdjustInSectorSize( int iFd, int iSourceSize ) {

    int i;
    int iAdjustSizeToSector;
    char cCh;
    int iSectorCount;

    iAdjustSizeToSector = iSourceSize % BYTESOFSECTOR;
    cCh = 0x00;

    if( iAdjustSizeToSector != 0 ) {

        iAdjustSizeToSector = 512 - iAdjustSizeToSector;
        for( i = 0; i < iAdjustSizeToSector; i++ ) {

            write( iFd, &cCh, 1 );

        }

    }
    else {

        printf( "[INFO] File size is aligned 512 byte\n" );

    }

    iSectorCount = ( iSourceSize + iAdjustSizeToSector ) / BYTESOFSECTOR;

    return iSectorCount;

}

// 소스 파일(Source FD)의 내용을 목표 파일(Target FD)에 복사하고 그 크기를 되돌려줌
int CopyFile( int iSourceFd, int iTargetFd ) {

    int iSourceFileSize;
    int iRead;
    int iWrite;
    char vcBuffer[ BYTESOFSECTOR ];

    iSourceFileSize = 0;
    while( 1 ) {

        iRead = read( iSourceFd, vcBuffer, sizeof( vcBuffer ) );
        iWrite = write( iTargetFd, vcBuffer, iRead );

        if( iRead != iWrite ) {

            fprintf( stderr, "[ERROR] iRead != iWrite...\n" );
            exit( -1 );

        }
        iSourceFileSize += iRead;

        if( iRead != sizeof( vcBuffer ) ) {

            break;

        }

    }

    return iSourceFileSize;

}