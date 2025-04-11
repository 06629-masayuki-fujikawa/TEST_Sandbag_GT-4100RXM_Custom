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
 * Description: Queue routines used by Telnet and Ftp servers
 * Filename: trserver.c
 * Author: Odile
 * Date Created: 01/04/99
 * $Source: source/trserver.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:48:26JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

/*
 * tfServerAppendToQueue function description:
 * Insert entry at the end of runq, or open connection list 
 * as specified by queueIndex, if entry is not already in the queue
 *
 * Parameters
 * servQueueHeadPtr     Head of the run Q and open connection list
 * servQueuePtr         Entry to append to queue
 * queueIndex           Queue index (run Q or open connection list)
 *
 * Return
 * No return
 */
void tfServerAppendToQueue ( ttServerQueueHeadPtr  servQueueHeadPtr,
                             ttServerQueuePtr      servQueuePtr,
                             int                   queueIndex )
{
    ttServerQueuePtr  prevServQueuePtr;

    if (    servQueuePtr->servNextQEntryPtr(queueIndex)
         == (ttServerQueuePtr)0 )
    {
        prevServQueuePtr = 
         (ttServerQueuePtr) servQueueHeadPtr->servHeadPrevQEntryPtr(queueIndex);
        servQueuePtr->servNextQEntryPtr(queueIndex) = servQueueHeadPtr;
        servQueuePtr->servPrevQEntryPtr(queueIndex) = prevServQueuePtr;
        prevServQueuePtr->servNextQEntryPtr(queueIndex) = servQueuePtr;
        servQueueHeadPtr->servHeadPrevQEntryPtr(queueIndex) = servQueuePtr;
        servQueueHeadPtr->servQHeadCount[queueIndex]++;
    }
}

/*
 * tfServerRemoveFromQueue function description:
 * Remove entry from runq, or open connection list as specificed by
 * queueIndex if entry is in the queue.
 *
 * Parameters
 * servQueuePtr         Entry to remove from queue
 * queueIndex           Queue index (run Q or open connection list)
 *
 * Return
 * No return
 */
void tfServerRemoveFromQueue ( ttServerQueueHeadPtr  servQueueHeadPtr,
                               ttServerQueuePtr      servQueuePtr,
                               int                   queueIndex)
{
    ttServerQueuePtr  nextServQueuePtr;
    ttServerQueuePtr  prevServQueuePtr;

    if (    servQueuePtr->servPrevQEntryPtr(queueIndex)
         != (ttServerQueuePtr)0 )
    {
        nextServQueuePtr = 
                 (ttServerQueuePtr) servQueuePtr->servNextQEntryPtr(queueIndex);
        prevServQueuePtr = 
                 (ttServerQueuePtr) servQueuePtr->servPrevQEntryPtr(queueIndex);
        prevServQueuePtr->servNextQEntryPtr(queueIndex) = nextServQueuePtr;
        nextServQueuePtr->servPrevQEntryPtr(queueIndex) = prevServQueuePtr;
        servQueuePtr->servNextQEntryPtr(queueIndex) = (ttServerQueuePtr)0;
        servQueuePtr->servPrevQEntryPtr(queueIndex) = (ttServerQueuePtr)0;
        servQueueHeadPtr->servQHeadCount[queueIndex]--;
    }
}
