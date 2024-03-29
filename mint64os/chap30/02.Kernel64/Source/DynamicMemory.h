#ifndef __DYNAMICMEMORY_H__
#define __DYNAMICMEMORY_H__

#include "Types.h"

#define DYNAMICMEMORY_START_ADDRESS     ( ( TASK_STACKPOOLADDRESS + ( TASK_STACKSIZE * TASK_MAXCOUNT ) + 0xfffff ) & 0xfffffffffff00000 )
#define DYNAMICMEMORY_MIN_SIZE          ( 1 * 1024 )

#define DYNAMICMEMORY_EXIST             0x01
#define DYNAMICMEMORY_EMPTY             0x00

typedef struct kBitmapStruct {

    BYTE* pbBitmap;
    QWORD qwExistBitCount;

} BITMAP;

typedef struct kDynamicMemoryManagerStruct {

    int iMaxLevelCount;
    int iBlockCountOfSmallestBlock;
    QWORD qwUsedSize;

    QWORD qwStartAddress;
    QWORD qwEndAddress;

    BYTE* pbAllocatedBlockListIndex;
    BITMAP* pstBitmapOfLevel;

} DYNAMICMEMORY;

// function
void kInitializeDynamicMemory( void );
void* kAllocatedMemory( QWORD qwSize );
BOOL kFreeMemory( void* pvAddress );
void kGetDynamicMemoryInformation( QWORD* pqwDynamicMemoryStartAddress, QWORD* pqwDynamicMemoryTotalSize, QWORD* pqwMetaDataSize, QWORD* pqwUsedMemorySize );
DYNAMICMEMORY* kGetDynamicMemoryManager( void );

static QWORD kCalculateDynamicMemorySize( void );
static int kCalculateMetaBlockCount( QWORD qwDynamicRAMSize );
static int kAllocationBuddyBlock( QWORD qwAlignedSize );
static QWORD kGetBuddyBlockSize( QWORD qwSize );
static int kGetBlockListIndexOfMatchSize( QWORD qwAlignedSize );
static int kFindFreeBlockInBitmap( int iBlockListIndex );
static void kSetFlagInBitmap( int iBlockListIndex, int iOffset, BYTE bFlag );
static BOOL kFreeBuddyBlock( int iBlockListIndex, int iBlockOffset );
static BYTE kGetFlagInBitmap( int iBlockListIndex, int iOffset );

#endif /*__DYNAMICMEMORY_H__*/