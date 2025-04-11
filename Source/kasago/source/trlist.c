/*
 * Copyright Notice:
 * Copyright Treck Incorporated  1997 - 2016
 * Copyright Zuken Elmic Japan   1997 - 2016
 * No portions or the complete contents of this file may be copied or
 * distributed in any form (including but not limited to printed or electronic
 * forms) without the expressed written consent of Treck Incorporated OR
 * Zuken Elmic.  Copyright laws and International Treaties protect the
 * contents of this file.  Unauthorized use is prohibited.
 * All rights reserved.
 *
 * Description: Generic linked list, with count.
 *
 * Filename: trlist.c
 * Author: Bryan Wang
 * Date Created: 1/6/2003
 * $Source: source/trlist.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:52JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trglobal.h>
#include <trproto.h>

/* Initilazing the list, must be called for a list before it is used */
void tfListInit(ttListPtr listPtr)
{
    listPtr->listCount = 0;
/* Sentinel initialization (empty list) */
    listPtr->listHeadNode.nodeNextPtr = &(listPtr->listHeadNode);
    listPtr->listHeadNode.nodePrevPtr = &(listPtr->listHeadNode);

    return;
}

/* Add an node to the head of a list
 * ( == tfListAddAtNode(listPtr, nodePtr, listPtr->listHeadNode.nodeNextPtr) )
 */
void tfListAddToHead(ttListPtr listPtr, ttNodePtr nodePtr)
{
    ttNodePtr headPtr;
    ttNodePtr nextHeadPtr;

    headPtr = &(listPtr->listHeadNode);
    nextHeadPtr = headPtr->nodeNextPtr;
    tm_assert(tfListAddToHead, nextHeadPtr != (ttNodePtr)0);
/* Add the new nodePtr to the head of the list (i.e. after the head).  */
    nodePtr->nodeNextPtr = nextHeadPtr;
    nodePtr->nodePrevPtr = headPtr;
    nextHeadPtr->nodePrevPtr = nodePtr;
    headPtr->nodeNextPtr = nodePtr;
    listPtr->listCount++;
    return;
}

/* Add an node to the tail of a list
 * ( == tfListAddAtNode(listPtr, nodePtr, &(listPtr->listHeadNode)) )
 */
void tfListAddToTail(ttListPtr listPtr, ttNodePtr nodePtr)
{
    ttNodePtr headPtr;
    ttNodePtr prevHeadPtr;

    headPtr = &(listPtr->listHeadNode);
    prevHeadPtr = headPtr->nodePrevPtr;
    tm_assert(tfListAddToTail, prevHeadPtr != (ttNodePtr)0);
/* Add the new nodePtr to the end of the list (i.e. before the head).  */
    nodePtr->nodeNextPtr = headPtr;
    nodePtr->nodePrevPtr = prevHeadPtr;
    prevHeadPtr->nodeNextPtr = nodePtr;
    headPtr->nodePrevPtr = nodePtr;
    listPtr->listCount++;
    return;
}

#if defined(TM_MULTIPLE_CONTEXT) || defined(TM_6_USE_MIP_CN)
/* Check that nodePtr is in the list pointed to by listPtr */
tt8Bit tfListMember(ttListPtr listPtr, ttNodePtr nodePtr)
{
    ttNodePtr       curNodePtr;
    unsigned int    i;
    tt8Bit          retCode;

    retCode = TM_8BIT_NO;
    curNodePtr = &(listPtr->listHeadNode);
    for (i = 0; i < listPtr->listCount; i++)
    {
        curNodePtr = curNodePtr->nodeNextPtr;
        if (curNodePtr == nodePtr)
        {
            retCode = TM_8BIT_YES;
            break;
        }
    }
    return retCode;
}
#endif /* TM_MULTIPLE_CONTEXT || TM_6_USE_MIP_CN */

/* remove a node from the list */
void tfListRemove(ttListPtr listPtr, ttNodePtr nodePtr)
{
/* Unlink the node from a multiple node list.  */
/* nodePtr must already be in the list */
    tm_assert(tfListRemove, (nodePtr->nodePrevPtr != TM_NODE_NULL_PTR));
    nodePtr->nodePrevPtr->nodeNextPtr = nodePtr->nodeNextPtr;
    tm_assert(tfListRemove, (nodePtr->nodeNextPtr != TM_NODE_NULL_PTR));
    nodePtr->nodeNextPtr->nodePrevPtr = nodePtr->nodePrevPtr;
    listPtr->listCount--;
/* Node not in a list */
    nodePtr->nodeNextPtr = (ttNodePtr)0;
    nodePtr->nodePrevPtr = (ttNodePtr)0;

    return;
}


/* move a node to the front and keep the order of the rest of the nodes */
void tfListMoveToFront(ttListPtr listPtr, ttNodePtr nodePtr)
{
/* Do nothing if this is the already at the head */
    if (nodePtr != listPtr->listHeadNode.nodeNextPtr)
    {
        tfListRemove(listPtr, nodePtr);
        tfListAddToHead(listPtr, nodePtr);
    }

    return;
}

/* Replace a node with a new one (swap) */
void tfListReplace( ttNodePtr oldNodePtr, ttNodePtr newNodePtr)
{
    newNodePtr->nodeNextPtr = oldNodePtr->nodeNextPtr;
    newNodePtr->nodePrevPtr = oldNodePtr->nodePrevPtr;

    oldNodePtr->nodePrevPtr->nodeNextPtr = newNodePtr;
    oldNodePtr->nodeNextPtr->nodePrevPtr = newNodePtr;

    return;
}

/*
 * Walk the list, for each node in the list, call the provided call
 * back function listCBFuncPtr with genParam,
 *
 * continues if listCBFuncPtr returns TM_8BIT_NO,
 * stops and return the current node if the listCBFuncPtr returns TM_8BIT_YES
 * return TM_NODE_NULL_PTR if the whole list is walked without getting
 * a TM_8BIT_YES from the call back function.
 */
ttNodePtr tfListWalk(ttListPtr          listPtr,
                     ttListCBFuncPtr    listCBFuncPtr,
                     ttGenericUnion     genParam)
{
    ttNodePtr    nodePtr;
    ttNodePtr    nextNodePtr;
    unsigned int i;
    unsigned int count;
    int          errorCode;
    
    nodePtr = listPtr->listHeadNode.nodeNextPtr;
    count = listPtr->listCount;

    for (i = 0; i < count; i++)
    {
        tm_assert(tfListWalk, nodePtr != &(listPtr->listHeadNode));
/* Call back could remove node entry from the list */
        nextNodePtr = nodePtr->nodeNextPtr;
        errorCode = listCBFuncPtr( nodePtr, genParam );
        if( errorCode == TM_8BIT_YES )
        {
            return nodePtr;
        }
        nodePtr = nextNodePtr;
    }

    return TM_NODE_NULL_PTR;
}

/*
 * Initialize a list iterator.
 */
void tfIterateThis(ttIteratorPtr itPtr, ttListPtr listPtr)
{
    itPtr->itNextPtr = listPtr->listHeadNode.nodeNextPtr;
    itPtr->itCount = listPtr->listCount;
}

/*
 * Return TRUE if not at end of list.
 */
int tfIterateHasNext(ttIteratorPtr itPtr)
{
    return itPtr->itCount > 0;
}

/*
 * Return the next item in the list.
 */
ttVoidPtr tfIterateNext(ttIteratorPtr itPtr)
{
    ttNodePtr   nextPtr;

    if (itPtr->itCount > 0)
    {
        --itPtr->itCount;
        nextPtr = itPtr->itNextPtr;
        itPtr->itNextPtr = nextPtr->nodeNextPtr;
    }
    else
    {
        nextPtr = TM_NODE_NULL_PTR;
    }
    return((ttVoidPtr)nextPtr);
}

/* Free all the node in this list, you can only call tfListFree() if all the 
 * nodes that have been allocated with tm_get_raw_buffer(), since tfListfree()
 * will call tm_free_raw_buffer for every node in the list.
 */
void tfListFree(ttListPtr listPtr)
{
    tt32Bit     i;
    ttNodePtr   nextNodePtr;
    ttNodePtr   curNodePtr;

    nextNodePtr = listPtr->listHeadNode.nodeNextPtr;
    tm_assert(tfListFree, nextNodePtr != (ttNodePtr)0);
    for(i=0; i < listPtr->listCount; i++)
    {
        curNodePtr = nextNodePtr;
        tm_assert(tfListFree, curNodePtr != &(listPtr->listHeadNode));
        nextNodePtr = curNodePtr->nodeNextPtr;
/* free this node, do it after we've gotten the next node */
        tm_free_raw_buffer((tt8BitPtr)curNodePtr);
    }
    tfListInit(listPtr);

    return;
}
