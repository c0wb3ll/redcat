#include "List.h"

// 리스트 초기화
void kInitializeList( LIST* pstList ) {

    pstList->iItemCount = 0;
    pstList->pvHeader = NULL;
    pstList->pvTail = NULL;

}

// 리스트에 포함된 아이템 수 반환
int kGetListCount( const LIST* pstList ) {

    return pstList->iItemCount;

}

// 리스트에 데이터를 더함
void kAddListToTail( LIST* pstList, void* pvItem ) {

    LISTLINK* pstLink;

    pstLink = ( LISTLINK* ) pvItem;
    pstLink->pvNext = NULL;

    if( pstList->pvHeader == NULL ) {

        pstList->pvHeader = pvItem;
        pstList->pvTail = pvItem;
        pstList->iItemCount = 1;

        return ;

    }

    pstLink = ( LISTLINK* ) pstList->pvTail;
    pstLink->pvNext = pvItem;

    pstList->pvTail = pvItem;
    pstList->iItemCount++;

}

// 리스트의 첫 부분에 데이터를 더함
void kAddListToHeader( LIST* pstList, void* pvItem ) {

    LISTLINK* pstLink;

    pstLink = ( LISTLINK* ) pvItem;
    pstLink->pvNext = pstList->pvHeader;

    if( pstList->pvHeader == NULL ) {

        pstList->pvHeader = pvItem;
        pstList->pvTail = pvItem;
        pstList->iItemCount = 1;

        return ;

    }

    pstList->pvHeader = pvItem;
    pstList->iItemCount++;

}

// 리스트에서 데이터를 제거한 후, 데이터의 포인터를 반환
void* kRemoveList( LIST* pstList, QWORD qwID ) {

    LISTLINK* pstLink;
    LISTLINK* pstPreviousLink;

    pstPreviousLink = ( LISTLINK* ) pstList->pvHeader;
    for( pstLink = pstPreviousLink; pstLink != NULL; pstLink = pstLink->pvNext ) {

        if( pstLink->qwID == qwID) {

            if( ( pstLink == pstList->pvHeader ) && ( pstLink == pstList->pvTail ) ) {

                pstList->pvHeader = NULL;
                pstList->pvTail = NULL;

            } else if( pstLink == pstList->pvHeader ) {

                pstList->pvHeader = pstLink->pvNext;

            } else if( pstLink == pstList->pvTail ) {

                pstList->pvTail = pstPreviousLink;

            } else {

                pstPreviousLink->pvNext = pstLink->pvNext;

            }

            pstList->iItemCount--;
            
            return pstLink;

        }

        pstPreviousLink = pstLink;

    }

    return NULL;

}

// 리스트의 첫 번째 데이터를 제거하여 반환
void* kRemoveListFromHeader( LIST* pstList ) {

    LISTLINK* pstLink;

    if( pstList->iItemCount == 0 ) { return NULL; }

    pstLink = ( LISTLINK* ) pstList->pvHeader;
    
    return kRemoveList( pstList, pstLink->qwID );

}

// 리스트의 마지막 데이터를 제거하여 반환
void* kRemoveListFromTail( LIST* pstList ) {

    LISTLINK* pstLink;

    if( pstList->iItemCount == 0 ) { return NULL; }

    pstLink = ( LISTLINK* ) pstList->pvTail;
    
    return kRemoveList( pstList, pstLink->qwID );

}

// 리스트에서 아이템을 찾음
void* kFindList( const LIST* pstList, QWORD qwID ) {

    LISTLINK* pstLink;

    for( pstLink = ( LISTLINK* ) pstList->pvHeader; pstLink != NULL; pstLink = pstLink->pvNext ) {

        if( pstLink->qwID == qwID ) { return pstLink; }

    }

    return NULL;

}

// 리스트의 헤더 반환
void* kGetHeaderFromList( const LIST* pstList ) {

    return pstList->pvHeader;

}

// 리스트의 테일 반환
void* kGetTailFromList( const LIST* pstList ) {

    return pstList->pvTail;

}

// 현재 아이템의 다음 아이템을 반환
void* kGetNextFromList( const LIST* pstList, void* pstCurrent ) {

    LISTLINK* pstLink;

    pstLink = ( LISTLINK* ) pstCurrent;

    return pstLink->pvNext;

}