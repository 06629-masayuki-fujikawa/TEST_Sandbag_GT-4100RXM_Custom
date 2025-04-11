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
 * Description:
 * Implements debug, error and trace logging to a circular buffer.
 *
 * Filename: trlog.c
 * Author: Ed Remmell
 * Date Created: 12/21/2001
 * $Source: source/trlog.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:55JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */

/* local variable definitions */

/* static const tables */

/* macro definitions */
#define TM_LOG_MSG_BEGIN_HDR    "HDR"

/****************************************************************************
* FUNCTION: tfLogAllocBuf
*
* PURPOSE: 
*   Allocate a circular log buffer, and a log control block to point to it.
*
* PARAMETERS:
*   maxLogMsgLen:
*       The expected maximum size of a log message. This value must be
*       fairly accurate, since it is used to add 1 max-sized log message
*       of padding at the end of the circular buffer so that we can avoid
*       writing past the end of the circular buffer when we log a message.
*       Specifying a value of 0 causes the default value
*       TM_DEF_MAX_LOG_MSG_LEN to be used.
*   numLogMsgs:
*       The number of max-sized log messages that the circular buffer can
*       hold. Specifying a value of 0 causes the default value
*       TM_DEF_NUM_LOG_MSGS to be used.
*
* RETURNS:
*   != NULL:
*       Pointer to the allocated log control block, which is passed as the
*       first argument to tfLogMsg.
*   == NULL:
*       Allocation failed.
*
* NOTES:
*
****************************************************************************/
ttLogCtrlBlkPtr tfLogAllocBuf(tt16Bit maxLogMsgLen, tt16Bit numLogMsgs)
{
    ttLogCtrlBlkPtr lcbPtr;
    tt32Bit bufLen;

/* initialize default values */
    lcbPtr = TM_LOG_CTRL_BLK_NULL_PTR;

    if (maxLogMsgLen == 0)
    {
        maxLogMsgLen = TM_DEF_MAX_LOG_MSG_LEN;
    }
    
    if (numLogMsgs == 0)
    {
        numLogMsgs = TM_DEF_NUM_LOG_MSGS;
    }

    if (numLogMsgs > 0)
    {
        numLogMsgs++; /* add padding at end of circular log buffer */
    }

/* factor in the log message header into the size of the log message */
    maxLogMsgLen += sizeof(ttLogMsgHdr) - TM_LOG_MSG_HDR_PAD_LEN;

/* pad the max log message length out to a 32-bit boundary */
#ifndef TM_DSP
    maxLogMsgLen = (tt16Bit) ((maxLogMsgLen + ((tt16Bit) TM_ROUND_SIZE))
                              & TM_ROUND_MASK_SIZE);
#endif /* TM_DSP */

/* compute the size of the circular log buffer */
    bufLen = maxLogMsgLen * numLogMsgs;

    if (bufLen > 0)
    {
/* allocate the log control block and circular log buffer */
        lcbPtr = (ttLogCtrlBlkPtr) tm_get_raw_buffer(
            (unsigned)sizeof(ttLogCtrlBlk) + (unsigned) bufLen);

        if (lcbPtr != TM_LOG_CTRL_BLK_NULL_PTR)
        {
            tm_bzero(lcbPtr, sizeof(ttLogCtrlBlk) + bufLen);

/* the circular log buffer starts immediately after the log control block */
            lcbPtr->lcbBufPtr =
                ((tt8BitPtr) lcbPtr) + (unsigned)sizeof(ttLogCtrlBlk);
            lcbPtr->lcbBufLen = bufLen;
            lcbPtr->lcbBufEndPadLen = maxLogMsgLen;
        }
    }

    return lcbPtr;
}

/****************************************************************************
* FUNCTION: tfLogFreeBuf
*
* PURPOSE: 
*   Frees the specified log control block, and the log buffer that it points
*   to.
*
* PARAMETERS:
*   lcbPtr:
*       Pointer to the log control block to deallocate.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tfLogFreeBuf(ttLogCtrlBlkPtr lcbPtr)
{
    tm_assert(tfLogFreeBuf, (lcbPtr != TM_LOG_CTRL_BLK_NULL_PTR));

#ifndef TM_ERROR_CHECKING
    if (lcbPtr != TM_LOG_CTRL_BLK_NULL_PTR)
#endif /* TM_ERROR_CHECKING */
    {
        tm_free_raw_buffer(lcbPtr);
    }

    return;
}

/****************************************************************************
* FUNCTION: tfLogStop
*
* PURPOSE: 
*   Stop logging and free the logging buffer and the log information used by
*   this protocol stack.
*
* PARAMETERS:
*   Nothing
*
* RETURNS:
*   Nothing
*
* NOTES:
*   Do not call tfLogSetLevel after calling this function.
*   To enable log function, please call tfStartTreck. 
*
****************************************************************************/
void tfLogStop(void)
{
#ifdef TM_USE_LOGGING_LEVELS
    int index;
#endif /* TM_USE_LOGGING_LEVELS */

#if (defined(TM_DEBUG_LOGGING) || defined(TM_TRACE_LOGGING) || \
     defined(TM_ERROR_LOGGING) || defined(TM_MEMORY_LOGGING) || \
     defined(TM_USE_LOGGING_LEVELS))
    if (tm_context(tvLogCtrlBlkPtr) != (ttLogCtrlBlkPtr)0)
    {
#ifdef TM_USE_LOGGING_LEVELS
        if (tm_context(tvLogLevelStoragePtr) != (ttLogLevelStoragePtr)0)
        {
            /* Stop logging for all moludes*/
            for (index = 0; index <= TM_LOG_MODULE_MAX; index++)
            {
#ifdef TM_LOCK_NEEDED
                tm_call_lock_wait(&(tm_context(tvLogLevelStoragePtr)->
                                    llsLogModuleLock[index]));
#endif /* TM_LOCK_NEEDED */
                tm_context(tvLogLevelStoragePtr)->llsLogLevelArray[index]
                    = TM_LOG_LEVEL_NONE;
#ifdef TM_LOCK_NEEDED
                tm_call_unlock(&(tm_context(tvLogLevelStoragePtr)->
                                 llsLogModuleLock[index]));
#endif /* TM_LOCK_NEEDED */
            }
            /* Free the logging buffer including LCB */
            tfLogFreeBuf(tm_context(tvLogCtrlBlkPtr));
            tm_context(tvLogCtrlBlkPtr) = (ttLogCtrlBlkPtr)0;

            /* Free the log level information */
            tm_kernel_free(tm_context(tvLogLevelStoragePtr));
            tm_context(tvLogLevelStoragePtr) = (ttLogLevelStoragePtr)0; 
        }
        else
        {
            /* Invalid global context */
            tfKernelWarning("tfLogStop", "Context has not been initialised\n");
        }   
#else  /* TM_USE_LOGGING_LEVELS */
        /* Free the logging buffer including LCB */
        tfLogFreeBuf(tm_context(tvLogCtrlBlkPtr));
        tm_context(tvLogCtrlBlkPtr) = (ttLogCtrlBlkPtr)0;
#endif /* TM_USE_LOGGING_LEVELS */
    }
    else
    {
        /* Invalid global context */
        tfKernelWarning("tfLogStop", "Context has not been initialised\n");
    }
#endif
}

/****************************************************************************
* FUNCTION: tfLogMsg
*
* PURPOSE: 
*   Log a message to the circular log buffer.
*
* PARAMETERS:
*   lcbPtr:
*       Pointer to the log control block, which specifies which circular
*       log buffer the message will be written to.
*   pszMsgPrefix:
*       An optional prefix string to add to the front of the message. Used
*       by macros that call this function. Specify a NULL pointer if no 
*       prefix string is used.
*   pszFormat:
*       Format string used by tfVSPrintF to format the message written to
*       the log buffer.
*   ...:
*       The variable argument list specifying the arguments to log.
*
* RETURNS:
*   > 0:
*       The number of characters written to the log buffer.
*   <= 0:
*       A non-specific error occurred.
*
* NOTES:
*   The log message is padded using '\0' characters to end on a 32-bit 
*   boundary. This is done so that the type ttLogMsgHdr can always be used
*   to map the beginning of a log message.
*   TM_LOG_MSG_HDR_PAD_LEN is used as the size of the message data at the
*   end of the log message header structure so that it is easier to display
*   the message data in the debugger (i.e. as part of the log message header
*   structure).
*
****************************************************************************/
int tfLogMsg(
    ttLogCtrlBlkPtr lcbPtr,
    const char TM_FAR *pszMsgPrefix,
    const char TM_FAR *pszFormat,
    ...)
{
    tt32Bit curTime;
    char preBuffer[72];
    int logMsgTotalLen = sizeof(ttLogMsgHdr) - TM_LOG_MSG_HDR_PAD_LEN;
    int len;
    int retCode = -1; /* assume a non-specific error occurred */
    ttLogMsgHdrPtr prevLogMsgPtr, currLogMsgPtr;
    va_list args;
 
    if (lcbPtr != TM_LOG_CTRL_BLK_NULL_PTR)
    {
        va_start(args, pszFormat);
    
        tm_lock_wait_no_log(&(lcbPtr->lcbLockEntry));

        prevLogMsgPtr = lcbPtr->lcbLastMsgPtr;

/* determine where to write the new log message */
        if (prevLogMsgPtr != TM_LOG_MSG_HDR_NULL_PTR)
        {
/* Before we write to the log buffer, we make sure there is room at the
   end for 2 max-sized log messages, and if not, then we wrap to the top. */
            len = (int) ((lcbPtr->lcbBufPtr + (unsigned) lcbPtr->lcbBufLen)
                - (((tt8BitPtr) prevLogMsgPtr) + prevLogMsgPtr->lmhMsgLen));

            tm_assert(tfLogMsg, (len >= 0));
        
            if (len < (int)(lcbPtr->lcbBufEndPadLen * 2))
            {
/* wrap to the top of the circular buffer */
                lcbPtr->lcbLastMsgPtr = (ttLogMsgHdrPtr) lcbPtr->lcbBufPtr;
            }
            else
            {
                lcbPtr->lcbLastMsgPtr = (ttLogMsgHdrPtr)
                    (((tt8BitPtr) prevLogMsgPtr) + prevLogMsgPtr->lmhMsgLen);
            }
        }
        else
        {
/* write the first log message at the top of the circular buffer */
            lcbPtr->lcbLastMsgPtr = (ttLogMsgHdrPtr) lcbPtr->lcbBufPtr;
        }

        currLogMsgPtr = lcbPtr->lcbLastMsgPtr;
        currLogMsgPtr->lmhUnion.lmhMsgPrevPtr = prevLogMsgPtr;
        tm_strcpy(currLogMsgPtr->lmhBeginHdr, TM_LOG_MSG_BEGIN_HDR);
        currLogMsgPtr->lmhMsgSeqNo = lcbPtr->lcbMsgSeqNo;

/* increment the sequence number for the next message */
        lcbPtr->lcbMsgSeqNo++;
        lcbPtr->lcbMsgSeqNo &= 0x7fff; /* high-order bit is always 0 */
        
/* write the optional message prefix to the buffer */
        if (((tt8BitPtr) pszMsgPrefix) != TM_8BIT_NULL_PTR)
        {
/* prepend a timestamp */
            tm_kernel_set_critical; 
            curTime = tvTime;
            tm_kernel_release_critical;
#ifdef TM_MULTIPLE_CONTEXT
/* Prepend the current context */
            tm_sprintf(preBuffer, "0x%p:(t=%lu) %s ", tvCurrentContext,
                                                    curTime, pszMsgPrefix);
#else /* !TM_MULTIPLE_CONTEXT */
            tm_sprintf(preBuffer, "(t=%lu) %s ", curTime, pszMsgPrefix);
#endif /* !TM_MULTIPLE_CONTEXT */
            tm_strcpy(currLogMsgPtr->lmhMsgData, preBuffer);
            logMsgTotalLen += (int)tm_strlen(preBuffer);
        }

/* write the log message to the bufer */
        len = tm_vsprintf(
            (ttCharPtr) (((tt8BitPtr) currLogMsgPtr) + logMsgTotalLen),
            pszFormat,
            args);
        tm_assert(tfLogMsg, (len >= 0));
        logMsgTotalLen += len + 1; /* include terminating null */

/* pad the total message length out to a 32-bit boundary */
        len = logMsgTotalLen;
#ifndef TM_DSP
        logMsgTotalLen = 
                (int)(  ((tt32Bit)logMsgTotalLen + TM_ROUND_SIZE) 
                      & TM_ROUND_MASK_SIZE );
#endif /* TM_DSP */

/* set pad bytes to '\0' */
        for (;len < logMsgTotalLen; len++)
        {
            *(((tt8BitPtr) currLogMsgPtr) + len) = '\0';
        }

        tm_assert(tfLogMsg, (logMsgTotalLen
                             <= (int)(lcbPtr->lcbBufEndPadLen * 2)));
        currLogMsgPtr->lmhMsgLen = (tt16Bit) logMsgTotalLen;
        
        tm_unlock_no_log(&(lcbPtr->lcbLockEntry));
        retCode = logMsgTotalLen;
    }

    return retCode;
}

/****************************************************************************
* FUNCTION: tfLogWalk
*
* PURPOSE: 
*   Walk the messages in the circular log buffer in the sequence in
*   which they were logged, optionally starting with a specified message
*   sequence number. If the specified message sequence number is not
*   found, then all messages are walked.
*
* PARAMETERS:
*   funcPtr:
*       Pointer to user-specified function called for each log message.
*       This function returns 0 to continue the walk, otherwise returns 
*       non-zero to terminate the walk. The function prototype for the
*       user-specified function is:
*               int myLogWalk(int msgSeqNo, const char TM_FAR *pszLogMsg,
*                   ttUserGenericUnion genParam);
*       where msgSeqNo is the 15-bit message sequence number, and pszLogMsg
*       is a pointer to the null-terminated log message. The log is walked
*       in the forward direction, so message sequence numbers will be in
*       ascending order.
*   lcbPtr:
*       Pointer to the log control block, which specifies which circular
*       log buffer to walk.
*   msgSeqNo:
*       An optional 15-bit message sequence number specifying where to
*       start the walk. Specify -1 if there is no specific sequence
*       number, and instead you want to walk all messages.
*   genParam
*       a generic parameter that is passed on to the funcPtr when it's called
*
* RETURNS:
*   TM_ENOERROR:
*       Successful.
*   TM_ENOBUFS:
*       Failed to allocate memory for copying the log buffer.
*   TM_EPERM:
*       The log buffer was not available.
*
* NOTES:
*   This function makes a copy of the log buffer before walking it.
*
****************************************************************************/
int tfLogWalkNoAlloc( ttLogWalkFuncPtr funcPtr,
                      ttLogCtrlBlkPtr lcbPtr,
                      ttLogCtrlBlkPtr lcbCopyPtr,
                      int lcbCopyLen,
                      int msgSeqNo,
                      ttUserGenericUnion genParam)
{
    int errorCode;
    int retCode = TM_ENOERROR; /* assume success */
    tt32Bit len;
    ttLogMsgHdrPtr firstLogMsgPtr, prevLogMsgPtr, nextLogMsgPtr, 
        lastLogMsgPtr, tempLogMsgPtr;
    tt8BitPtr origBufPtr;

    if ((lcbPtr == TM_LOG_CTRL_BLK_NULL_PTR) || (lcbPtr->lcbBufLen == 0)
        || (lcbCopyPtr == TM_LOG_CTRL_BLK_NULL_PTR)
        || (((unsigned) lcbCopyLen)
            < ((unsigned)sizeof(ttLogCtrlBlk) + (unsigned) lcbPtr->lcbBufLen)))
    {
        retCode = TM_EPERM;
    }
    else
    {
/* make a copy of the log buffer */
        len = lcbPtr->lcbBufLen;
        origBufPtr = lcbPtr->lcbBufPtr; /* used to fixup pointers */

        tm_lock_wait_no_log(&(lcbPtr->lcbLockEntry));

/* copy the log buffer control block */
        tm_bcopy(lcbPtr, lcbCopyPtr, sizeof(ttLogCtrlBlk));

        lcbCopyPtr->lcbBufPtr =
            ((tt8BitPtr) lcbCopyPtr) + (unsigned)sizeof(ttLogCtrlBlk);

/* copy the circular log buffer */
        tm_bcopy(origBufPtr, lcbCopyPtr->lcbBufPtr, len);

        tm_unlock_no_log(&(lcbPtr->lcbLockEntry));

/* if we successfully copied the log buffer, and there are messages in the
   buffer, then walk it. NOTE: we have a shallow copy, but we need to
   make a deep copy, so fixup the pointers to point into the copy. */
        if (lcbCopyPtr->lcbLastMsgPtr != TM_LOG_MSG_HDR_NULL_PTR)
        {
            tempLogMsgPtr = (ttLogMsgHdrPtr)
                (lcbCopyPtr->lcbBufPtr
                 + (unsigned) ((tt8BitPtr) lcbCopyPtr->lcbLastMsgPtr
                               - origBufPtr));
/* fixup the pointer to point into the copy */
            lcbCopyPtr->lcbLastMsgPtr = tempLogMsgPtr;

            lastLogMsgPtr = lcbCopyPtr->lcbLastMsgPtr;
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
            tm_assert(tfLogWalk,
                      (tm_strcmp(
                          lastLogMsgPtr->lmhBeginHdr, TM_LOG_MSG_BEGIN_HDR)
                       == 0));
/* PRQA L:L1 */
/* backtrack to find the first log message in the circular log buffer */
            for (  firstLogMsgPtr = lastLogMsgPtr,
                       prevLogMsgPtr = lastLogMsgPtr;
                   prevLogMsgPtr != TM_LOG_MSG_HDR_NULL_PTR;)
            {
/* if this is not a valid message header, then we are done backtracking */
                errorCode = tm_strcmp( prevLogMsgPtr->lmhBeginHdr,
                                       TM_LOG_MSG_BEGIN_HDR );
                if(errorCode != 0)
                {
                    break;
                }
                
                if (prevLogMsgPtr->lmhUnion.lmhMsgPrevPtr
                    != TM_LOG_MSG_HDR_NULL_PTR)
                {
                    tempLogMsgPtr = (ttLogMsgHdrPtr)
                        (lcbCopyPtr->lcbBufPtr + 
                         (unsigned) ((tt8BitPtr) prevLogMsgPtr->
                                     lmhUnion.lmhMsgPrevPtr - origBufPtr));

/* check if somehow we ended up back where we started: this is tricky, we
   allow the backtracking to fixup a pointer to point into the copy twice so
   that it no longer points into the copy of the buffer which is our indication
   that we've backtracked too far. */
                    if ((((tt8BitPtr) tempLogMsgPtr)
                         < lcbCopyPtr->lcbBufPtr)
                        || (((tt8BitPtr) tempLogMsgPtr) >=
                            (lcbCopyPtr->lcbBufPtr + (unsigned) len)))
                    {
                        break;
                    }

/* fixup the pointer to point into the copy */
                    prevLogMsgPtr->lmhUnion.lmhMsgPrevPtr = tempLogMsgPtr;
                }

/* remember the last valid message header */
                nextLogMsgPtr = firstLogMsgPtr;
                firstLogMsgPtr = prevLogMsgPtr;
/* backtrack */
                prevLogMsgPtr = prevLogMsgPtr->lmhUnion.lmhMsgPrevPtr;
                
/* change the prev ptr to be a pointer to next, so that we can more easily
   walk the log buffer in the forward direction. */
                firstLogMsgPtr->lmhUnion.lmhMsgNextPtr = nextLogMsgPtr;
                
/*
 * check if this message header is the one that the user wants to start 
 * with 
 */
                if ((int)(firstLogMsgPtr->lmhMsgSeqNo) == msgSeqNo)
                {
                    break;
                }
            }
            
/*
 * NULL terminate the linked-list so we can walk it in the forward direction
 */
            lastLogMsgPtr->lmhUnion.lmhMsgNextPtr = TM_LOG_MSG_HDR_NULL_PTR;

/* walk the buffer, if there are messages to walk */
            for (nextLogMsgPtr = firstLogMsgPtr;
                 nextLogMsgPtr != TM_LOG_MSG_HDR_NULL_PTR;
                 nextLogMsgPtr = nextLogMsgPtr->lmhUnion.lmhMsgNextPtr)
            {
                tm_assert(
                    tfLogWalk,
                    ((((tt8BitPtr) nextLogMsgPtr) >= lcbCopyPtr->lcbBufPtr)
                     && (((tt8BitPtr) nextLogMsgPtr) <
                         (lcbCopyPtr->lcbBufPtr + (unsigned) len))));
                
/* walk until the user-specified function returns an error */
                errorCode = (*funcPtr)(
                            nextLogMsgPtr->lmhMsgSeqNo,
                            (const char TM_FAR *) nextLogMsgPtr->lmhMsgData,
                            genParam );
                if( errorCode!= TM_ENOERROR )
                {
                    break;
                }

                if (nextLogMsgPtr == lastLogMsgPtr)
                {
                    break;
                }
            }
        }
    }

    return retCode;
}
    
int tfLogWalk(ttLogWalkFuncPtr   funcPtr, 
              ttLogCtrlBlkPtr    lcbPtr, 
              int                msgSeqNo,
              ttUserGenericUnion genParam)
{
    int retCode = TM_ENOERROR; /* assume success */
    ttLogCtrlBlkPtr lcbCopyPtr = TM_LOG_CTRL_BLK_NULL_PTR;
    tt32Bit len;
    int lcbCopyLen; 
    int nextMsgSeqNo;

    if ((lcbPtr == TM_LOG_CTRL_BLK_NULL_PTR) || (lcbPtr->lcbBufLen == 0)
#ifdef TM_ERROR_CHECKING
        ||  (funcPtr == (ttLogWalkFuncPtr)0)
#endif /* TM_ERROR_CHECKING */
        )
    {
        retCode = TM_EPERM;
    }
    else
    {
        if (msgSeqNo != -1)
        {
/* Handle wrap around */
            msgSeqNo &= 0x7fff; /* high-order bit is always 0 */
        }
        tm_lock_wait_no_log(&(lcbPtr->lcbLockEntry));
        nextMsgSeqNo = (int)lcbPtr->lcbMsgSeqNo;
        tm_unlock_no_log(&(lcbPtr->lcbLockEntry));
/* If there is a message not yet read */
        if (    (nextMsgSeqNo != msgSeqNo)
             && (lcbPtr->lcbLastMsgPtr != (ttLogMsgHdrPtr)0))
        {
/* attempt to make a copy of the log buffer */
            len = lcbPtr->lcbBufLen;
/* attempt to allocate a log buffer control block and a circular log
   buffer for the copy */
            lcbCopyLen = (int)((unsigned)sizeof(ttLogCtrlBlk) + (unsigned)len);
            lcbCopyPtr = (ttLogCtrlBlkPtr) tm_get_raw_buffer((ttPktLen)lcbCopyLen);
            if (lcbCopyPtr == TM_LOG_CTRL_BLK_NULL_PTR)
            {
                retCode = TM_ENOBUFS;
            }
            else
            {
                retCode = tfLogWalkNoAlloc(funcPtr, lcbPtr, lcbCopyPtr,
                                           lcbCopyLen, msgSeqNo, genParam);

/* free the copy we made of the log buffer */
                tm_free_raw_buffer(lcbCopyPtr);
            }
        }
    }

    return retCode;
}



/***************** End Of File *****************/
