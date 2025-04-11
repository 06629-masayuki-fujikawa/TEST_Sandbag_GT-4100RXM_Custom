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
 * Description: Timer Functionality
 * Filename: trtimer.c
 * Author: Odile
 * Date Created: 02/08/09
 * $Source: source/trtimer.c $
 *
 * Modification History
 * $Revision: 6.0.2.6 $
 * $Date: 2011/02/16 07:21:39JST $
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

/* Execute and update are called by the user */

/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */

/* Call timer free routine */
static void tfTimerFree (ttTimerPtr timerPtr, tt8Bit flags);

#if defined(TM_USE_TIMER_INTERVAL_QUERY) || defined(TM_USE_TIMER_CB)
/* Get interval of next time to call tfTimerExecute */
static int tfTimerLockedIntervalQuery(ttUser32BitPtr intervalPtr
#ifdef TM_USE_TIMER_CACHE
                                    , ttTimeStructPtr  nextRunTimePtr
#endif /* TM_USE_TIMER_CACHE */
                                    );
#ifdef TM_USE_TIMER_CACHE
#ifdef TM_USE_TIMER_MULT_ACTIVE_LIST
/* Return first nont empty list */
static int tfTimerFirstNonEmptyList(void);
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST */
#ifdef TM_USE_TIMER_WHEEL
/* Return first non empty wheel slot list, and give the number of empty
 * slots
 */ 
static tt16Bit tfTimerFirstNonEmptySlot(ttUIntPtr countPtr);
#endif /* TM_USE_TIMER_WHEEL */
#endif /* TM_USE_TIMER_CACHE */
#endif /* defined(TM_USE_TIMER_INTERVAL_QUERY) || defined(TM_USE_TIMER_CB) */

#ifdef TM_USE_TIMER_CACHE
#ifndef TM_USE_TIMER_MULT_ACTIVE_LIST
/* Scan the active timer list and move expiring timers to the wheel */
static void tfTimerActiveListScan(void);
#else /* TM_USE_TIMER_MULT_ACTIVE_LIST */
/* Scan the active timer lists and move expiring timers to the wheel or other
 * lists.
 */
static int tfTimerActiveListsScan(ttTimeStructPtr expTimePtr);
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST */
/*
 * Spin the wheel, and execute the timers whose slot(s) has expired on the
 * wheel
 */
static tt32Bit tfTimerWheelSpinExecute(ttTimeStructPtr curTimePtr,
                                       tt32BitPtr      numTimersExecutedPtr);
/* Common timer add routine */
static void tfTimerAddCommon(ttTimerPtr      timerPtr,
                             tt32Bit         wheelOffset);
#ifdef TM_USE_TIMER_CB
static tt8Bit tfTimerNextCB(ttTimerPtr timerPtr);
#endif /* TM_USE_TIMER_CB */
#endif /* TM_USE_TIMER_CACHE */

#ifdef TM_USE_STOP_TRECK
static int tfTimerCBRemove (ttNodePtr nodePtr, ttGenericUnion genParm);
#endif /* TM_USE_STOP_TRECK */

/* local variable definitions */

/* static const tables */

/* macro definitions */

/* User owns the timer structure */
#define TM_TIM_USER_OWN     (tt8Bit)0x04

/* Timer not in a List? */
#define tm_timer_not_in_list(listNo)                                        \
    (listNo == TM_TIMER_NO_LIST)
/* Timer in a List? */
#define tm_timer_in_list(listNo)                                            \
    (listNo != TM_TIMER_NO_LIST)
/* Timer in an active List/wheel? */
#define tm_timer_active(listNo)                                             \
    (listNo >= TM_TIMER_ACTIVE_LIST)
/* Timer in the suspend List? */
#define tm_timer_in_suspend_list(listNo)                                    \
    (listNo == TM_TIMER_SUSPEND_LIST)

/* Timer list */
#define tm_tmr_list(listNo)                                                 \
   (tm_context(tvTimerCtrl).timerList[listNo - 1])

/* list in the Timer list */
#define tm_tmr_list_list(listNo)                                            \
   (tm_tmr_list(listNo).tmlList)

#ifdef TM_USE_TIMER_CACHE
#ifdef TM_USE_TIMER_WHEEL
/* timer wheel */
#define tm_tmr_wheel                                                        \
   (tm_context(tvTimerCtrl).timerWheel)

/* Head of a list at a wheel slot */
#define tm_tmr_wheel_list(slotNo)                                           \
    tm_tmr_wheel.twlListPtr[slotNo]

/* Timer current wheel slot */
#define tm_tmr_wheel_cur_slot                                               \
    (tm_tmr_wheel.twlCurSlotNo)

/* Timer current wheel slot run time */
#define tm_tmr_wheel_cur_time                                               \
    (tm_tmr_wheel.twlCurSlotRunTime)

/* Timer maximum wheel slot */
#define tm_tmr_wheel_number_slots                                           \
    (tm_tmr_wheel.twlNumberSlots)

/* Retrieve list the timer is on */
#define tm_tmr_list_wheel_list_ptr(listNo, slotNo, listPtr)                 \
  if (listNo == TM_TIMER_WHEEL)                                             \
  {                                                                         \
    listPtr = &(tm_tmr_wheel_list(slotNo));                                 \
  }                                                                         \
  else                                                                      \
  {                                                                         \
    listPtr = &(tm_tmr_list_list(listNo));                                  \
  }

/* Timer wheel slot list (based on interval left to fire) */
#define tm_tmr_get_wheel_slot(interval, listNo, slotNo, listPtr)            \
  listNo = TM_TIMER_WHEEL;                                                  \
  slotNo = (tt16Bit)((interval + tvTimerTickLength - 1)                     \
                     / tvTimerTickLength);                                  \
  slotNo = (tt16Bit)(slotNo + tm_tmr_wheel_cur_slot);                       \
  if (slotNo >= tm_tmr_wheel_number_slots)                                  \
  {                                                                         \
      slotNo = (tt16Bit)(slotNo - tm_tmr_wheel_number_slots);               \
  }                                                                         \
  listPtr = &(tm_tmr_wheel_list(slotNo));
#endif /* TM_USE_TIMER_WHEEL */
#endif /* TM_USE_TIMER_CACHE */

#ifdef TM_USE_TIMER_CACHE
/*
 * Do we need to run the given active list?
 */
#define tm_tmr_list_ready(curTime, listNo)                                  \
    tm_tmr_leq(tm_tmr_list(listNo).tmlNextRunTime, curTime)

/* Minimum interval of firing for a timer on that list */
#define tm_tmr_list_min_interval(listNo)                                    \
    tm_tmr_list(listNo).tmlMinInterval

#ifndef TM_USE_TIMER_MULT_ACTIVE_LIST
/* Timer Active List/wheel slot list (based on interval left to fire) */
/* Timer Active List/wheel slot list (based on interval left to fire) */
#define tm_tmr_get_list(interval, listNo, slotNo, period, listPtr)          \
  if (interval <= tm_tmr_list_min_interval(TM_TIMER_ACTIVE_LIST))           \
  {                                                                         \
      tm_tmr_get_wheel_slot(interval, listNo, slotNo, listPtr)              \
      period = tvTimerTickLength;                                           \
  }                                                                         \
  else                                                                      \
  {                                                                         \
      period = tm_tmr_list_min_interval(TM_TIMER_ACTIVE_LIST);              \
      slotNo = 0;                                                           \
      listPtr = &(tm_tmr_list_list(TM_TIMER_ACTIVE_LIST));                  \
      listNo = TM_TIMER_ACTIVE_LIST;                                        \
  }
#else /* TM_USE_TIMER_MULT_ACTIVE_LIST */
#define tm_tmr_get_list(interval, listNo, slotNo, period, listPtr)          \
  if (interval <= tm_tmr_list_min_interval(TM_TIMER_ACTIVE_LIST))           \
  {                                                                         \
      tm_tmr_get_wheel_slot(interval, listNo, slotNo, listPtr)              \
      period = tvTimerTickLength;                                           \
  }                                                                         \
  else                                                                      \
  {                                                                         \
    period = 0;                                                             \
    for (listNo = TM_TIMER_LAST_ACTIVE_LIST;                                \
         listNo >= TM_TIMER_ACTIVE_LIST;                                    \
         listNo--)                                                          \
    {                                                                       \
        if (interval > tm_tmr_list_min_interval(listNo))                    \
        {                                                                   \
            period = tm_tmr_list_min_interval(listNo);                      \
            break;                                                          \
        }                                                                   \
    }                                                                       \
    slotNo = 0;                                                             \
    listPtr = &(tm_tmr_list_list(listNo));                                  \
  }
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST */
#endif /* TM_USE_TIMER_CACHE */

/* move timer from a list to another one */
#define  tm_timer_move_from_to_list(timerPtr, fromListPtr, toListPtr)       \
    (void)tfListRemove(                                                     \
            fromListPtr,                                                    \
            &(timerPtr->tmrNode));                                          \
    tfListAddToTail(                                                        \
                toListPtr,                                                  \
                &timerPtr->tmrNode);

#ifndef TM_USE_TIMER_CACHE
/* move timer at the end of the list */
#define  tm_timer_move_to_end_list(timerPtr, listNo)                        \
    if (&(timerPtr->tmrNode) !=                                             \
              tm_tmr_list_list(listNo).listHeadNode.nodePrevPtr)            \
    {                                                                       \
        (void)tfListRemove(                                                 \
                &(tm_tmr_list_list(listNo)),                                \
                &(timerPtr->tmrNode));                                      \
        tfListAddToTail(                                                    \
                &(tm_tmr_list_list(listNo)),                                \
                &(timerPtr->tmrNode));                                      \
    }
#endif /* !TM_USE_TIMER_CACHE */

/*
 * Remove timer from a given list. Indicate that the timer is in no list.
 * Decrease owner count
 */
#define tm_timer_remove_from_list(timerPtr, listPtr)                        \
    (void)tfListRemove(                                                     \
                listPtr,                                                    \
                &(timerPtr->tmrNode));                                      \
    timerPtr->tmrListNo = TM_TIMER_NO_LIST;                                 \
    timerPtr->tmrOwnerCount--;


#ifndef tm_tmr_get_current_time
/* Get current global time */
#define tm_tmr_get_current_time(curTime)                                    \
    tm_kernel_set_critical;                                                 \
    curTime.timMsecs = (tt32Bit)tvTime;                                     \
    curTime.timRollover = tvTimeRollOver;                                   \
    tm_kernel_release_critical;
#endif /* !tm_tmr_get_current_time */

/* Init a timer stop time (sing current global time, and timer interval) */
#define tm_tmr_add_time(newTime, curTime, interval)                         \
    newTime.timMsecs =  curTime.timMsecs + interval;                        \
    newTime.timRollover = curTime.timRollover;                              \
    if (newTime.timMsecs < curTime.timMsecs)                                \
    {                                                                       \
        newTime.timRollover++;                                              \
    }

/* calculate the time remaining on the timer */
#define tm_tmr_remaining_time(result, timerPtr, curTime)                    \
    if (tm_tmr_leq(timerPtr->tmrStopTime, curTime))                         \
    {                                                                       \
        result = 0; /* expired */                                           \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        result = (timerPtr->tmrStopTime.timMsecs - curTime.timMsecs);       \
    }

/* Init a timer stop time (sing current global time, and timer interval) */
#define tm_tmr_init_stop_time(timerPtr, curTime, interval)                  \
    tm_tmr_add_time(timerPtr->tmrStopTime, curTime, interval)

/* less or equal Comparison between 2 time structures */
#define tm_tmr_leq(time1, time2)                                            \
   (     (    (time1.timRollover == time2.timRollover)                      \
           && (time1.timMsecs <= time2.timMsecs))                           \
     ||  (time1.timRollover < time2.timRollover) )

/* Copy from to time structures */
#define tm_tmr_copy(fromTime, toTime)                                       \
    toTime.timMsecs =  fromTime.timMsecs;                                   \
    toTime.timRollover =  fromTime.timRollover;

/* Copy from time structure ptr to time structure */
#define tm_tmr_copy_from_ptr(fromTimePtr, toTime)                           \
    toTime.timMsecs =  fromTimePtr->timMsecs;                               \
    toTime.timRollover =  fromTimePtr->timRollover;

/* Copy from time structure to time structure ptr */
#define tm_tmr_copy_to_ptr(fromTime, toTimePtr)                             \
    toTimePtr->timMsecs =  fromTime.timMsecs;                               \
    toTimePtr->timRollover =  fromTime.timRollover;

#define tm_tmr_copy_ptr(fromTimePtr, toTimePtr)                             \
    toTimePtr->timMsecs =  fromTimePtr->timMsecs;                           \
    toTimePtr->timRollover =  fromTimePtr->timRollover;

/* compare time structure to time structure ptr */
#define tm_tmr_lt_ptr2(time1, time2Ptr)                                     \
       (     (    ((time1).timRollover == (time2Ptr)->timRollover)          \
               && ((time1).timMsecs < (time2Ptr)->timMsecs))                \
         ||  ((time1).timRollover < (time2Ptr)->timRollover) )

/****************************************************************************
* FUNCTION: tfTimerAdd
*
* PURPOSE: 
*   Create a Timer Block and return it to the caller. Can be called from the
*   timer excute thread.
*
*   Call common routine tfTimerAddExt() with a null timerPtr,
*   and null clean up function pointer.
*
* PARAMETERS:
*   callBackFunctionPtr:
*       user-specified callback function called when the timer expires
*   userParm1
*       first user-specified union passed to the callback function
*   userParm2
*       second user-specified union passed to the callback function       
*   timerMilliSec:
*       timer timeout period in milliseconds
*   timerFlags:
*       TM_TIM_AUTO if an auto-fire timer is desired, otherwise timer is 
*       a one-shot timer that removes itself after it expires.
*       TM_TIM_SUSPENDED if the timer should be added in the suspended state,
*       otherwise it is added in the active state.
*
* RETURNS:
*   Pointer to the newly allocated timer.
*
* NOTES:
*   Failure to allocate a timer is a critical error.
*
****************************************************************************/
ttTimerPtr tfTimerAdd( ttTmCBFuncPtr  callBackFunctionPtr,
                       ttGenericUnion userParm1,
                       ttGenericUnion userParm2,
                       tt32Bit        timerMilliSec,
                       tt8Bit         timerFlags )
{
    return tfTimerAddExt((ttTimerPtr)0,
                         callBackFunctionPtr, (ttTmCUFuncPtr)0,
                         userParm1, userParm2, timerMilliSec, timerFlags);
}

/****************************************************************************
* FUNCTION: tfTimerAddExt
*
* PURPOSE: 
*   Create a Timer Block and return it to the caller. Can be called from the
*   timer execute thread.
*
*   If timer is not a user timer, allocate it.
*   Initialize the timer.
*   If timer is created in the suspended state, insert it in the suspend list.
*   If timer is created in the active state:
*    . If caching is not enabled, insert it in the active list. 
*    . If caching is enabled:
*       .. If wheel is empty
*          ... make sure to initialize the wheel current slot
*              time with the current time.
*       .. else
*          ... Make sure to add the offset from the timer wheel current slot
*              time to the current time to the interval of firing.
*       .. Call tfTimerAddCommon().
* PARAMETERS:
*
*   timerPtr: Either null, or pointer to user allocated timer.
*   callBackFunctionPtr:
*       user-specified callback function called when the timer expires
*   cleanupFunctionPtr:
*       user-specified cleanup function called when the timer is freed.
*   userParm1
*       first user-specified union passed to the callback function
*   userParm2
*       second user-specified union passed to the callback function       
*   timerMilliSec:
*       timer timeout period in milliseconds
*   timerFlags:
*       TM_TIM_AUTO if an auto-fire timer is desired, otherwise timer is 
*       a one-shot timer that removes itself after it expires.
*       TM_TIM_SUSPENDED if the timer should be added in the suspended state,
*       otherwise it is added in the active state.
*
* RETURNS:
*   Pointer to the newly allocated timer.
*
* NOTES:
*   . Failure to allocate a timer is a critical error.
*   . If a timer is added for the first time, the user has to bzero the
*     timer structure timerPtr points to. But if the user has removed it,
*     and is re-adding it, the user should not bzero the structure, as
*     the timer could still be owned by the timer execute thread if
*     the timer was removed and re-added in the timer execute call back
*     function.
*
****************************************************************************/
TM_PROTO_EXTERN ttTimerPtr tfTimerAddExt (
                                       ttTimerPtr     timerPtr,
                                       ttTmCBFuncPtr  callBackFunctionPtr,
                                       ttTmCUFuncPtr  cleanUpFunctionPtr,
                                       ttGenericUnion userParm1,
                                       ttGenericUnion userParm2,
                                       tt32Bit        timerMilliSec,
                                       tt8Bit         timerFlags )
{
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    ttUserTimerCBFuncPtr  userTimerCBFuncPtr;
    ttUserGenericUnion    userParam;
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
    ttTimeStruct          curTime;
#ifdef TM_USE_TIMER_CACHE
    tt32Bit               wheelOffset;
#endif /* TM_USE_TIMER_CACHE */

#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    userTimerCBFuncPtr = (ttUserTimerCBFuncPtr)0;
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
/* LOCK the timer lists */
    tm_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    userParam = tm_context(tvTimerCBParam);
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
    if (timerPtr == (ttTimerPtr)0)
    {
/* allocate the new timer */
        timerPtr = (ttTimerPtr)tm_recycle_alloc( (unsigned)sizeof(ttTimer),
                                                      TM_RECY_TIMER );
        if (timerPtr == TM_TMR_NULL_PTR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelError("tfTimerAddExt", "No memory to allocate a timer");
#endif /* TM_ERROR_CHECKING */
/* Severe error. Stop current thread */
            tm_thread_stop;
        }
    }
    else
    {
/* Pre-allocated timer */
/* Timer either has a zero owner count, or timer execute thread increased it */
        tm_assert(tfTimerAddExt, timerPtr->tmrOwnerCount <= 1);
/* Timer should have been removed, or not added */
        tm_assert(tfTimerAddExt, tm_timer_not_in_list(timerPtr->tmrListNo));
/* User owns the timer */
        timerFlags = (tt8Bit)(timerFlags | TM_TIM_USER_OWN);
/* Timer has been removed/re-added */
        timerPtr->tmrCallBackFlag = TM_8BIT_ZERO;
    }
    if (timerPtr != (ttTimerPtr)0)
     {
        tm_tmr_get_current_time(curTime);
/* start the timer */
        tm_tmr_init_stop_time(timerPtr, curTime, timerMilliSec);
        timerPtr->tmrInterval = timerMilliSec;
/* Save timerflags. Reset suspended flag.  */
        timerPtr->tmrFlags = (tt8Bit)(timerFlags & ~TM_TIM_SUSPENDED);
        timerPtr->tmrCallBackFunctionPtr = callBackFunctionPtr;
        timerPtr->tmrCleanUpFunctionPtr = cleanUpFunctionPtr;
        timerPtr->tmrUserParm1 = userParm1;
        timerPtr->tmrUserParm2 = userParm2;
        if (timerFlags & TM_TIM_SUSPENDED)
        {
/* In the suspend list */
            timerPtr->tmrListNo = TM_TIMER_SUSPEND_LIST;
#ifdef TM_USE_TIMER_CACHE
            timerPtr->tmrWheelSlot = TM_16BIT_ZERO;
#endif /* TM_USE_TIMER_CACHE */
/* Append the timer */
            tfListAddToTail(&tm_tmr_list_list(TM_TIMER_SUSPEND_LIST),
                            &timerPtr->tmrNode);
        }
        else
        {
#ifndef TM_USE_TIMER_CACHE
            timerPtr->tmrListNo = TM_TIMER_ACTIVE_LIST;
/* Append the timer */
            tfListAddToTail(&tm_tmr_list_list(TM_TIMER_ACTIVE_LIST),
                            &timerPtr->tmrNode);
#else /* TM_USE_TIMER_CACHE */
            if (tm_tmr_wheel.twlNumberTimers == 0)
            {
/* offset for timer wheel and all lists  */
                tm_tmr_copy(curTime, tm_tmr_wheel_cur_time);
                wheelOffset = timerMilliSec;
            }
            else
            {
                wheelOffset = timerMilliSec +
                                       curTime.timMsecs
                                     - tm_tmr_wheel_cur_time.timMsecs;
            }
            tfTimerAddCommon(timerPtr, wheelOffset);
#ifdef TM_USE_TIMER_CB
            if (tm_context(tvTimerCBFuncPtr) != (ttUserTimerCBFuncPtr)0)
            {
                if (tfTimerNextCB(timerPtr))
                {
                    userTimerCBFuncPtr = tm_context(tvTimerCBFuncPtr);
                }
            }
#endif /* TM_USE_TIMER_CB */
#endif /* TM_USE_TIMER_CACHE */
        }
/* List ownership */
        timerPtr->tmrOwnerCount++;
    }
/* UNLOCK the timer lists */
    tm_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    if (userTimerCBFuncPtr != (ttUserTimerCBFuncPtr)0)
    {
/* Notify the user */
        (*userTimerCBFuncPtr)(timerMilliSec, userParam);
    }
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
    return (timerPtr);
}

#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
/*
 * Determine if this timer is the next one to expire.
 *  This timer is the next to expire if:
 *   . there is no pending timer CB,
 *   . or if our timer expires before the next CB scheduled time 
 * Return
 * 1 if this timer is the next one to expire
 * 0 if not.
 */
static tt8Bit tfTimerNextCB(ttTimerPtr timerPtr)
{
    tt8Bit retCode;

    if (    (!(tm_context(tvTimerCtrl).timerCBPending))
         || (tm_tmr_lt(timerPtr->tmrStopTime,
                       tm_context(tvTimerCtrl).timerCBTime)) )
    {
/* no pending CB, or our timer expires before the next CB time */
        tm_tmr_copy(timerPtr->tmrStopTime,
                    tm_context(tvTimerCtrl).timerCBTime);
        tm_context(tvTimerCtrl).timerCBPending = TM_8BIT_YES;
        retCode = TM_8BIT_YES;
    }
    else
    {
        retCode = TM_8BIT_NO;
    }
    return retCode;
}
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */

/****************************************************************************
* FUNCTION: tfTimerAddCommon
*
* PURPOSE: 
*   Common timer add routine when caching is enabled.
*   Called by tfTimerAddExt()/tfTimerResume()/tfTimerNewTime().
*    . append timer to the active list/timer wheel slot list/
*      corresponding to the interval of firing.
*    . Make sure to increase the number of timers in the wheel, if the
*      timer is added to a wheel slot list.
*    . If the timer is added to an active list, and is the first timer in
*      that active list, make sure to update the next run time for that
*      list based on the current slot run time, and list minimum interval.
*
* PARAMETERS:
*
*   timerPtr: pointer to timer.
*   wheelOffset:
*       timer timeout interval in milliseconds from current wheel run time
*
* RETURNS:
*       NONE
*/
#ifdef TM_USE_TIMER_CACHE
static void tfTimerAddCommon (ttTimerPtr      timerPtr,
                              tt32Bit         wheelOffset)
{
    ttListPtr             listPtr;
    tt32Bit               period;
    int                   listNo;
    tt16Bit               slotNo;

    tm_tmr_get_list(wheelOffset, listNo,
                    slotNo, period, listPtr);
/* Append the timer */
    tfListAddToTail(listPtr, &timerPtr->tmrNode);
    timerPtr->tmrListNo = (tt8Bit)listNo;
    timerPtr->tmrWheelSlot = slotNo;
    if (listNo == TM_TIMER_WHEEL)
    {
/* Adding to the wheel */
        tm_tmr_wheel.twlNumberTimers++;
    }
    else if (listPtr->listCount == 1)
/* Adding to an empty list. Refresh next run time on that list */
    {
        tm_tmr_add_time(tm_tmr_list(listNo).tmlNextRunTime,
                        tm_tmr_wheel_cur_time, period);
    }
    return;
}
#endif /* TM_USE_TIMER_CACHE */

/****************************************************************************
* FUNCTION: tfTimerRemove
*
* PURPOSE: 
*   Remove a timer from its list and free it if its owner count is zero.
*
*   Indicate to the timer execute call back function that the timer has been
*   modified, by resetting the callBackFlag (tm_timer_not_reinit() is then
*   true).
*   Update the timer execute next timer pointer if this is the next timer
*   to be executed.
*   If caching is enabled, and timer was on a wheel slot list, decrement
*   the number of timers on the wheel.
*
* PARAMETERS:
*   timerPtr:
*       pointer to the timer to remove.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   If the timer is removed in the context of the timer execute thread, timer
*   will be freed in tfTimerExecute(), as tfTimerExecute() increases the owner
*   count before the call back function.
*
****************************************************************************/
void tfTimerRemove(ttTimerPtr timerPtr)
{
    ttListPtr   listPtr;
    int         listNo;

    tm_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);
    listNo = timerPtr->tmrListNo;
    if (tm_timer_in_list(listNo))
    {
        if (    !tm_timer_in_suspend_list(listNo)
             && (&(timerPtr->tmrNode)
                      == tm_context(tvTimerCtrl).timerExecNextNodePtr) )
        {
/* Update context global next timer */
            tm_context(tvTimerCtrl).timerExecNextNodePtr
                                = timerPtr->tmrNode.nodeNextPtr;
        }
        tm_assert(tfTimerRemove, timerPtr->tmrOwnerCount != 0);
#ifdef TM_USE_TIMER_CACHE
        if (listNo == TM_TIMER_WHEEL)
/* Removing from the wheel */
        {
            tm_tmr_wheel.twlNumberTimers--;
        }
/* Retrieve list the timer is on */
        tm_tmr_list_wheel_list_ptr(listNo, timerPtr->tmrWheelSlot, listPtr);
#else /* !TM_USE_TIMER_CACHE */
        listPtr = &(tm_tmr_list_list(listNo));
#endif /* !TM_USE_TIMER_CACHE */
        tm_timer_remove_from_list(timerPtr, listPtr);
/* reset tmrCallBackFlag for tm_timer_not_reinit() */
        timerPtr->tmrCallBackFlag = TM_8BIT_ZERO;
        if (timerPtr->tmrOwnerCount == 0)
        {
            tfTimerFree(timerPtr, TM_TIMER_CLEANUP_LOCKED);
        }
    }
    tm_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);
    return;
}

/****************************************************************************
* FUNCTION: tfTimerLockedRemove
*
* PURPOSE: 
*   Call tfTimerRemove() with the execute lock on.
*   This call guarantees that the timer is removed before it is fired.
*
* PARAMETERS:
*   timerPtr:
*       pointer to the timer to remove.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   This function CAN NOT be called from the timer execute.
*
****************************************************************************/
void tfTimerLockedRemove(ttTimerPtr timerPtr)
{
/* LOCK out the execute */
    tm_call_lock_wait(&tm_context(tvTimerCtrl).timerExecuteLockEntry);
    tfTimerRemove(timerPtr);
/* UNLOCK the execute */
    tm_call_unlock(&tm_context(tvTimerCtrl).timerExecuteLockEntry);
    return;
}

/****************************************************************************
* FUNCTION: tfTimerNewTime
*
* PURPOSE:
*   Update an auto timer with a new interval of firing.
*
*   If timer was suspended it is re-activated.
*
*   1, Update the interval of firing with the passed new value.
*   2. If this is called from the timer execute thread, do not do anything
*      else, as the timer execute thread will move and update the timer after
*      the call back returns.
*   3. else
*     . Update the timer execute next timer pointer if this is the next timer
*       to be executed.
*     . Initialize timer stop time.
*  IF TIMER CACHE IS NOT ENABLED
*     . If the timer is in the suspend list, or is not already at the end of
*       the active list move it to the end of the active list.
*  ELSE TIMER CACHE IS ENABLED
*     . If wheel is empty
*       .. make sure to initialize the wheel current slot
*          time with the current time.
*     . else
*       .. Make sure to add the offset from the timer wheel current slot
*          time to the current time to the interval of firing.
*    IF OPTIMIZE_SPEED IS NOT ENABLED
*     . remove timer from its current list
*     . If timer was on the wheel, decrease number of timers on the wheel.
*     . call tfTimerAddCommon().
*    ELSE OPTIMIZE SPEED IS ENABLED
*     . If the timer is not on the wheel, and staying in the same active list
*        .. If the timer is added to an active list, and is the first timer in
*           that active list, make sure to update the next run time for that
*           list using the wheel current slot time, and the list's minimum
*           interval.
*     . else
*        .. remove timer from its current list
*        .. If timer was on the wheel, decrease number of timers on the wheel.
*      IF TIMER CB IS ENABLED
*        .. call tfTimerAddCommon().
*      ELSE TIMER CB IS NOT ENABLED
*        .. Get list to which the timer needs to be appended
*        .. Append the timer to it
*        .. Initialize timer variables with corresponding list number, and
*           slot number.
*        .. If adding to the wheel, increase the number of timers on the wheel
*        .. else if first timer added to the list, refresh next run time on
*                                                  that list.
*      ENDIF TIMER CB IS NOT ENABLED
*    ENDIF OPTIMIZE SPEED IS ENABLED
*  ENDIF TIMER CACHE IS ENABLED
* PARAMETERS:
*   timerPtr:
*       pointer to the auto timer
*   timeParam:
*       the new timer period, in milliseconds
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tfTimerNewTime (ttTimerPtr timerPtr, tt32Bit timeParam)
{
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    ttUserTimerCBFuncPtr  userTimerCBFuncPtr;
    ttUserGenericUnion    userParam;
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
#ifdef TM_USE_TIMER_CACHE
    ttListPtr             listPtr;
#endif /* TM_USE_TIMER_CACHE */
    ttTimeStruct          curTime;
#ifdef TM_USE_TIMER_CACHE
    tt32Bit               wheelOffset;
    tt32Bit               period;
#endif /* TM_USE_TIMER_CACHE */
    int                   listNo;

/* 
 * Update the timer in place
 */
/* LOCK the timer lists */
    tm_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    userParam = tm_context(tvTimerCBParam);
    userTimerCBFuncPtr = (ttUserTimerCBFuncPtr)0;
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
    listNo = timerPtr->tmrListNo;
    if (tm_timer_in_list(listNo))
    {
/* set the new period for the timer */
        timerPtr->tmrInterval = timeParam;
        if (timerPtr->tmrCallBackFlag == TM_8BIT_NO)
/* Not in a call back */
        {
            if (    !tm_timer_in_suspend_list(listNo)
                 && (&(timerPtr->tmrNode)
                     == tm_context(tvTimerCtrl).timerExecNextNodePtr) )
            {
/* Update context global next timer */
                tm_context(tvTimerCtrl).timerExecNextNodePtr
                                = timerPtr->tmrNode.nodeNextPtr;
            }
/* Update timer stop time. */
            tm_tmr_get_current_time(curTime);
            tm_tmr_init_stop_time(timerPtr, curTime, timeParam);
#ifndef TM_USE_TIMER_CACHE
/* Simplest case: no cache */
            if (    (listNo == TM_TIMER_SUSPEND_LIST)
                 || (&(timerPtr->tmrNode) !=
                      tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).
                                        listHeadNode.nodePrevPtr) )
            {
/* In the suspend list, or not at the end of the active list */
                tm_timer_move_from_to_list(timerPtr,
                                   &(tm_tmr_list_list(listNo)),
                                   &(tm_tmr_list_list(TM_TIMER_ACTIVE_LIST)));
                timerPtr->tmrListNo = TM_TIMER_ACTIVE_LIST;
            }
#else /* TM_USE_TIMER_CACHE */
/* cache on case: */
            if (tm_tmr_wheel.twlNumberTimers == 0)
            {
/* offset for timer wheel and all lists  */
                tm_tmr_copy(curTime, tm_tmr_wheel_cur_time);
                wheelOffset = timeParam;
            }
            else
            {
                wheelOffset = timeParam +
                                       curTime.timMsecs
                                     - tm_tmr_wheel_cur_time.timMsecs;
            }
            if (
#ifndef TM_USE_TIMER_MULT_ACTIVE_LIST
                    (listNo == TM_TIMER_ACTIVE_LIST)
                 && (wheelOffset >
                         tm_tmr_list_min_interval(TM_TIMER_ACTIVE_LIST))
#else /* TM_USE_TIMER_MULT_ACTIVE_LIST */
                    (listNo != TM_TIMER_SUSPEND_LIST)
                 && (listNo != TM_TIMER_WHEEL)
                 && (wheelOffset > tm_tmr_list_min_interval(listNo))
                 && (    (listNo == TM_TIMER_LAST_ACTIVE_LIST)
                      || (wheelOffset <= tm_tmr_list_min_interval(listNo+1)))
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST */
               )
/* Statement added to save time when the timer stays on the same list */
            {
/* Staying on the same list */
                if (tm_tmr_list_list(listNo).listCount == 1)
                {
                    period = tm_tmr_list_min_interval(listNo);
/* Single timer in a list. Refresh next run time on that list */
                    tm_tmr_add_time(tm_tmr_list(listNo).tmlNextRunTime,
                                    tm_tmr_wheel_cur_time, period);
                }
            }
            else
            {
/* Retrieve list the timer is on */
                tm_tmr_list_wheel_list_ptr(listNo,
                                           timerPtr->tmrWheelSlot,
                                           listPtr);
/* Remove timer from its list */
                (void)tfListRemove(
                    listPtr,
                    &(timerPtr->tmrNode));
                if (listNo == TM_TIMER_WHEEL)
                {
/* Removing from the wheel */
                    tm_tmr_wheel.twlNumberTimers--;
                }
/* Add to new list (at wheelOffset) */
                tfTimerAddCommon(timerPtr, wheelOffset);
            }
#endif /* TM_USE_TIMER_CACHE */
        }
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
        if (tm_context(tvTimerCBFuncPtr) != (ttUserTimerCBFuncPtr)0)
        {
            if (tfTimerNextCB(timerPtr))
            {
                userTimerCBFuncPtr = tm_context(tvTimerCBFuncPtr);
            }
        }
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
    }
/* UNLOCK the timer lists */
    tm_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    if (userTimerCBFuncPtr != (ttUserTimerCBFuncPtr)0)
    {
/* Notify the user */
        (*userTimerCBFuncPtr)(timeParam, userParam);
    }
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
    return;
}

/****************************************************************************
* FUNCTION: tfTimerSuspend
*
* PURPOSE: 
*   Suspend a timer. Remove it from the active list and move it to the suspend
*   list. 
*
*   Update the timer execute next timer pointer if this is the next timer
*   to be executed.
*   Indicate to the timer execute call back function that the timer has been
*   modified, by resetting the callBackFlag (tm_timer_not_reinit() is then
*   true).
*   If caching is enabled, and timer was on a wheel slot list, decrement
*   the number of timers on the wheel.
*
* PARAMETERS:
*   timerPtr:
*       pointer to the timer to suspend.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tfTimerSuspend(ttTimerPtr timerPtr)
{
    ttListPtr listPtr;

    tm_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);
    if (tm_timer_active(timerPtr->tmrListNo))
    {
        if ((&(timerPtr->tmrNode)
                      == tm_context(tvTimerCtrl).timerExecNextNodePtr) )
        {
/* Update context global next timer */
            tm_context(tvTimerCtrl).timerExecNextNodePtr
                                = timerPtr->tmrNode.nodeNextPtr;
        }
#ifdef TM_USE_TIMER_CACHE
/* Retrieve list the timer is on */
        tm_tmr_list_wheel_list_ptr(timerPtr->tmrListNo, timerPtr->tmrWheelSlot,
                                   listPtr);
#else /* !TM_USE_TIMER_CACHE */
        listPtr = &(tm_tmr_list_list(TM_TIMER_ACTIVE_LIST));
#endif /* !TM_USE_TIMER_CACHE */
/* Remove from current list, and append to suspend list */
        tm_timer_move_from_to_list(timerPtr, listPtr,
                                   &tm_tmr_list_list(TM_TIMER_SUSPEND_LIST));
#ifdef TM_USE_TIMER_CACHE
        if (timerPtr->tmrListNo == TM_TIMER_WHEEL)
/* Removing from the wheel */
        {
            tm_tmr_wheel.twlNumberTimers--;
        }
        timerPtr->tmrWheelSlot = (tt16Bit)0;
#endif /* TM_USE_TIMER_CACHE */
        timerPtr->tmrListNo = TM_TIMER_SUSPEND_LIST;
/* reset tmrCallBackFlag for tm_timer_not_reinit() */
        timerPtr->tmrCallBackFlag = TM_8BIT_ZERO;
    }
    tm_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);
    return;
}

/****************************************************************************
* FUNCTION: tfTimerResume
*
* PURPOSE:
*   Resume a timer that was previously suspended
*
*   . Remove the timer from the suspend list.
*   . If caching is not enabled, move the timer to the end of the active list.
*   . If caching is enabled:
*       .. If wheel is empty
*          ... make sure to initialize the wheel current slot
*              time with the current time.
*      .. Compute the time left on the timer from the wheel current slot run
*         time
*      .. Call tfTimerAddCommon().
*
* PARAMETERS:
*   timerPtr:
*       pointer to the timer to resume.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tfTimerResume (ttTimerPtr timerPtr)
{
#ifdef TM_USE_TIMER_CACHE
    ttTimeStruct            curTime;
#endif /* TM_USE_TIMER_CACHE */
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    ttUserTimerCBFuncPtr    userTimerCBFuncPtr;
    ttUserGenericUnion      userParam;
    tt32Bit                 interval;
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
#ifdef TM_USE_TIMER_CACHE
    tt32Bit                 wheelOffset;
#endif /* TM_USE_TIMER_CACHE */

#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    interval = TM_32BIT_ZERO;
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
/* 
 * Update the timer in place
 */
/* LOCK the timer lists */
    tm_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    userParam = tm_context(tvTimerCBParam);
    userTimerCBFuncPtr = (ttUserTimerCBFuncPtr)0;
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
    if (tm_timer_in_suspend_list(timerPtr->tmrListNo))
    {
/* If this timer was suspended, we resume it. */
/* Remove timer from the suspend list */
        (void)tfListRemove(
            &(tm_tmr_list_list(TM_TIMER_SUSPEND_LIST)),
            &(timerPtr->tmrNode));
#ifndef TM_USE_TIMER_CACHE
/*
 * append the timer to the appropriate list
 */
        tfListAddToTail(
                &(tm_tmr_list_list(TM_TIMER_ACTIVE_LIST)),
                &timerPtr->tmrNode);
        timerPtr->tmrListNo = TM_TIMER_ACTIVE_LIST;
#else /* TM_USE_TIMER_CACHE */
/* Find which list/wheel slot to insert the timer in based on the amount of
 * time before it expires. Use the current time on the wheel.
 */
        tm_tmr_get_current_time(curTime);
        if (tm_tmr_wheel.twlNumberTimers == 0)
        {
/* offset for timer wheel and all lists  */
            tm_tmr_copy(curTime, tm_tmr_wheel_cur_time);
        }
        tm_tmr_remaining_time(wheelOffset, timerPtr, tm_tmr_wheel_cur_time);
        tfTimerAddCommon(timerPtr, wheelOffset);
#ifdef TM_USE_TIMER_CB
        if (tm_context(tvTimerCBFuncPtr) != (ttUserTimerCBFuncPtr)0)
        {
            if (tfTimerNextCB(timerPtr))
            {
                userTimerCBFuncPtr = tm_context(tvTimerCBFuncPtr);
                tm_tmr_remaining_time(interval, timerPtr, curTime);
            }
        }
#endif /* TM_USE_TIMER_CB */
#endif /* TM_USE_TIMER_CACHE */
    }
/* UNLOCK the timer lists */
    tm_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);
#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)
    if (userTimerCBFuncPtr != (ttUserTimerCBFuncPtr)0)
    {
/* Notify the user */
        (*userTimerCBFuncPtr)(interval, userParam);
    }
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */
    return;
}

/****************************************************************************
* FUNCTION: tfTimerMsecLeft
*
* PURPOSE:
*   This function returns the number of milliseconds remaining in the
*   specified timer period if the timer is running (or suspended),
*   Called with the timer in the active or suspended list.
*   Called with either the timer lists lock on, or with the user structure
*   associated with the timer locked, so that the timer cannot be removed
*   during that call.
*
* PARAMETERS:
*   timerPtr:
*       Pointer to the timer for which we want time remaining.
*
* RETURNS:
*       The number of milliseconds remaining in the specified timer period.
*
* NOTES:
*
****************************************************************************/
tt32Bit tfTimerMsecLeft(ttTimerPtr timerPtr)
{
    tt32Bit         timeRemaining;
    ttTimeStruct    curTime;

/* grab a consistent snapshot of the timer state, and while we're at it,
   the current tick count. */
    tm_tmr_get_current_time(curTime);
    tm_tmr_remaining_time(timeRemaining, timerPtr, curTime);
    return timeRemaining;
}

/*
 * FUNCTION: tfTimerFree
 *
 * PURPOSE: 
 * Free the timer.
 * 
 * . If the user set a clean up function, call it.
 * . If the user does not own the timer, recycle it.
 * Called with the timer lists lock on.
 *
 * PARAMETERS:
 *  timerPtr: pointer to timer to free.
 *
 *  flags:    Indicate to the cleanup function whether the resource associated
 *            with the timer is locked (if tfTimerFree() is not called from
 *            the execute thread.)  
 *
 * RETURNS:
 *  None. 
 *
 * Note: Timer lists Unlocked/Relocked before/after the clean up function
 *
 */
static void tfTimerFree (ttTimerPtr timerPtr, tt8Bit flags)
{
    ttTmCUFuncPtr   cleanUpFunctionPtr; 
    tt8Bit          userOwnTimer;

/* Save this bit before the clean up function! */
    userOwnTimer = (tt8Bit)
                    (timerPtr->tmrFlags & TM_TIM_USER_OWN);
    cleanUpFunctionPtr = timerPtr->tmrCleanUpFunctionPtr;
    if (cleanUpFunctionPtr != (ttTmCUFuncPtr)0)
    {
/* UNLOCK the timer lists */
        tm_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);
        (*(cleanUpFunctionPtr))(timerPtr, flags);
/* LOCK the timer lists */
        tm_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);
    }
    if (userOwnTimer == TM_8BIT_NO)
/* Allocated by the timer */
    {
        tm_recycle_free(timerPtr, TM_RECY_TIMER);
    }
    return;
}


#ifndef TM_USE_TIMER_CACHE 
/****************************************************************************
* FUNCTION: tfTimerExecute
*
* PURPOSE: 
*   . Walk the Timer Active List and execute timers which have expired.
*   
*   . Lock the execute lock, and the active lists lock during tfTimerExecute.
*   . We run the (single) timer active list.
*   . Remove the one shot timer, BEFORE calling the call back function, since
*     the user could re-add the timer in the call back routine.
*   . Increase the timer owner count BEFORE calling the timer call
*     back routine, so that the timer is not freed if it is removed in the
*     call back routine. The timer owner count is then decreased AFTER the call
*     back function returns, and the timer is freed if it had been removed in
*     the call back.
*   . AFTER the call back routine, if the timer is an auto fire timer, (and the
*     user did not remove/add/suspend the timer),
*     .. re-initialize the timer stop time.
*     .. move timer to the end of the list.
*        . Make sure to decrease the number of timers in the wheel, if the
*   . If the user set the option to limit the maximum number of timers executed
*     per tfTimerExecute call, break out of the loop(s), if the limit is
*     reached.
*
* PARAMETERS:
*   None
*
* RETURNS:
*   Nothing
*
* NOTES:
*   Note that in this thread, in the call back function, any of the timer
*   routines could be called on the timer being executed or on another timer
*   (to remove, suspend, change the interval time, resume (another timer) or
*   add a new timer entry).  For this reason the timer active list is unlocked
*   during the call back.
*
*   Note that in this thread The timer execute is locked and not unlocked, and
*   for this reason tfTimerLockedRemove() should never be called from a timer
*   execute call back function.
*
****************************************************************************/
void tfTimerExecute (void)
{
    ttTimerPtr              timerPtr;
    ttTmCBFuncPtr           callBackFunctionPtr;
    ttTimeStruct            curTime;
/* Number of timers executed during the list walk */
    tt32Bit                 numTimersExecuted;
    int                     count;
    int                     walkCount;

/* Generate new values for tvVector1/tvVector2 */
    tfRandNew();
/* Number of timers executed */
    numTimersExecuted = (tt32Bit)0;
/* Execute LOCK (to lock out timer removal (tfTimerLockedRemove())) */
    tm_lock_wait(&tm_context(tvTimerCtrl).timerExecuteLockEntry);
/* LOCK the timer lists */
    tm_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);

    if (tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listCount != 0)
    {
/*
 * Save the next timer to execute in context global because we unlock for the
 * call back, and possibly in tfTimerFree()
 */
        tm_context(tvTimerCtrl).timerExecNextNodePtr = 
               tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listHeadNode.nodeNextPtr;
/*
 * Walk the acive timer list, but no more than what is already in the list
 * before we run.
 */
        count = 0;
        walkCount = (int)tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listCount;
        tm_tmr_get_current_time(curTime);
        do
        {
/* Timer to execute */
            timerPtr = (ttTimerPtr)(ttVoidPtr)
                                tm_context(tvTimerCtrl).timerExecNextNodePtr;
/* Save pointer to the next timer (before possibly unlocking) */
            tm_context(tvTimerCtrl).timerExecNextNodePtr =
                   tm_context(tvTimerCtrl).timerExecNextNodePtr->nodeNextPtr;
/* 
 *  check to see if the timer has expired:
 */
            if (tm_tmr_leq(timerPtr->tmrStopTime, curTime))
            {
/*
 * Execute ownership (before we potentially remove the timer from the list,
 * and call back)
 */
                timerPtr->tmrOwnerCount++;
                if (!(timerPtr->tmrFlags & TM_TIM_AUTO))
/* Non AUTO timer, remove it */
                {
                    tm_timer_remove_from_list(timerPtr,
                                  &(tm_tmr_list_list(TM_TIMER_ACTIVE_LIST)));
                }
/* Mark current timer for the call back function */
                timerPtr->tmrCallBackFlag = TM_8BIT_YES;
/* Save the call back function before unlocking */
                callBackFunctionPtr = timerPtr->tmrCallBackFunctionPtr;
/* UNLOCK the timer lists */
                tm_unlock(&(tm_context(tvTimerCtrl).timerListLockEntry));
/* Call the call back function with the parameters */
                (*(callBackFunctionPtr))(timerPtr,
                                         timerPtr->tmrUserParm1,
                                         timerPtr->tmrUserParm2);

/* LOCK the timer lists */
                tm_lock_wait(&(tm_context(tvTimerCtrl).timerListLockEntry));
                tm_assert(tfTimerExecute, timerPtr->tmrOwnerCount != 0);
/* Execute ownership */
                timerPtr->tmrOwnerCount--;
                if (timerPtr->tmrOwnerCount == 0)
/* 
 * Free the timer if is has been removed. 
 */
                {
                    tm_assert(tfTimerExecute,
                              tm_timer_not_in_list(timerPtr->tmrListNo));
                    tfTimerFree(timerPtr, TM_8BIT_ZERO);
                }
                else
                {
                    if (    (timerPtr->tmrFlags & TM_TIM_AUTO)
                         && (tm_timer_not_reinit(timerPtr)))
/*
 * Auto timer, (and the user did not add/remove/suspend/call tfTimerNewTime()
 * that caused the timer to move to a different active list),
 * re-initialize the timer stop time, and move the timer to the end of the
 * appropriate list.
 */
                    {
/* restart the timer */
                        tm_tmr_init_stop_time(timerPtr,
                                          curTime,
                                          timerPtr->tmrInterval);
/* Append the timer */
                        tm_timer_move_to_end_list(timerPtr,
                                                        TM_TIMER_ACTIVE_LIST);
                    }
/* Reset timer call back flag */
                    timerPtr->tmrCallBackFlag = TM_8BIT_ZERO;
                }
                if (tm_context(tvTimerMaxExecute) != 0)
/* Update the number of executed timers (if the user set a limit) */
                {
                    (numTimersExecuted)++;
                    if (numTimersExecuted
                                     >= (tt32Bit)tm_context(tvTimerMaxExecute))
                    {
/* 
 * Break here as soon as we reach the maximum.
 */
                        break; /* do while */
                    }
                }
            }
            count++;
        } while ( (count < walkCount) &&
                  (tm_context(tvTimerCtrl).timerExecNextNodePtr
                     != &(tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listHeadNode))
/*
 * End of the list. This could happen even if we had not reached walkCount,
 * if timers had been removed while we unlocked (for the call back function,
 * or in tfTimerFree())
 */
                 );
/* Done with this active list. Reset context global next timer */
        tm_context(tvTimerCtrl).timerExecNextNodePtr = (ttNodePtr)0;
    } /* if list count */
/* UNLOCK Active list */
    tm_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);
/* UNLOCK Execute */
    tm_unlock(&tm_context(tvTimerCtrl).timerExecuteLockEntry);
}

#else /* TM_USE_TIMER_CACHE */
/****************************************************************************
* FUNCTION: tfTimerExecute
*
* PURPOSE: 
*   Walk the Timer Active List(s)/wheel and execute timers which have expired.
*   There is a timer wheel, and multiple (four) timer active lists.
*
*      Each slot in the timer wheel has a list of timers that expire within
*      one tick. After a timer tick has elapsed, the wheel moves to the next
*      slot. The wheel has an upper limit of tvTimerMaxWheelIntv ms by
*      default, so there are
*      ((tvTimerMaxWheelIntv + tvTimerTickLength -1)/tvTimerTickLength) + 1)
*      slots.
*
*      if TM_USE_TIMER_MULT_ACTIVE_LIST is defined:
*      Each active list thereafter contains timers that are due to expire
*      in more than tvTimerMaxWheelIntv ms. (the first one in more than
*      tvTimerMaxWheelIntv ms, the second one in more than
*      2 * tvTimerMaxWheelIntv ms, the third one in more than
*      8 * tvTimerMaxWheelIntv ms, and the fourth one in more than
*      64 * tvTimerMaxWheelIntv ms).
*      Every tvTimerMaxWheelIntv ms the first active list is run to check
*      for timers that need to be moved to the wheel.
*      Every 2 * tvTimerMaxWheelIntv ms, the second active list is run to
*      check for timer that need to be moved to a lesser list
*      Every 8 * tvTimerMaxWheelIntv, the third active list is run to
*      check for timer that need to be moved to a lesser list
*      Every 64 * tvTimerMaxWheelIntv, the fourth active list is run to
*      check for timer that need to be moved to a lesser list
*
*      if TM_USE_TIMER_MULT_ACTIVE_LIST is not defined:
*      The active list thereafter contains timers that are due to expire
*      in more than tvTimerMaxWheelIntv ms. 
*      Every tvTimerMaxWheelIntv ms the active list is run to check
*      for timers that need to be moved to the wheel.
*
*   . Lock the execute lock, and the active lists/wheel lock during
*     tfTimerExecute.
*   . If timer wheel is empty, make sure to set the wheel current slot run
*     time.
*   . In a loop do while the wheel is spinning and there are more timers 
*     ready in the active lists:
*     .. Scan the active list(s) for timers to move to the wheel, (or if
*        TM_USE_TIMER_MULT_ACTIVE_LIST is defined) to another active list).
*     .. If the wheel has timers ready to run, or needs spinning, call
*        tfTimerWheelSpinExecute to spin the wheel and/or execute the expired
*        timers.
*
* PARAMETERS:
*   None
*
* RETURNS:
*   Nothing
*
* NOTES:
*   Note that in this thread, in the call back function, any of the timer
*   routines could be called on the timer being executed or on another timer
*   (to remove, suspend, change the interval time, resume (another timer) or
*   add a new timer entry).  For this reason the timer active list is unlocked
*   during the call back.
*
*   Note that in this thread The timer execute is locked and not unlocked, and
*   for this reason tfTimerLockedRemove() should never be called from a timer
*   execute call back function.
*
****************************************************************************/
void tfTimerExecute(void)
{
#ifdef TM_USE_TIMER_CB
    ttUserTimerCBFuncPtr    userTimerCBFuncPtr;
    ttUserGenericUnion      userParam;
#endif /* TM_USE_TIMER_CB */
    ttTimeStruct            curTime;
#ifdef TM_USE_TIMER_MULT_ACTIVE_LIST
    ttTimeStruct            activeListsExpTime;
#endif /* !TM_USE_TIMER_MULT_ACTIVE_LIST */
#ifdef TM_USE_TIMER_CB
    tt32Bit                 interval;
#endif /* TM_USE_TIMER_CB */
/* Number of timers executed during the timer execute */
    tt32Bit                 numTimersExecuted;
/* Number of wheel slots spun during one timer wheel execute */
    tt32Bit                 numSpunSlots;
#ifdef TM_USE_TIMER_MULT_ACTIVE_LIST
    int                     activeLists;
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST */
#ifdef TM_USE_TIMER_CB
    int                     retCode;
#endif /* TM_USE_TIMER_CB */

/* Generate new values for tvVector1/tvVector2 */
    tfRandNew();
/* Number of timers executed */
    numTimersExecuted = (tt32Bit)0;
/* Execute LOCK (to lock out timer removal (tfTimerLockedRemove())) */
    tm_lock_wait(&tm_context(tvTimerCtrl).timerExecuteLockEntry);
    
/* LOCK the timer lists */
    tm_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);
    tm_tmr_get_current_time(curTime);
    if (tm_tmr_wheel.twlNumberTimers == 0)
    {
/* offset for timer wheel and all lists  */
        tm_tmr_copy(curTime, tm_tmr_wheel_cur_time);
    }
#ifdef TM_USE_TIMER_CB
/* Save user timer CB function */
    userTimerCBFuncPtr = tm_context(tvTimerCBFuncPtr);
/* Reset user timer CB function so that no CB occurs during timer execute */
    tm_context(tvTimerCBFuncPtr) = (ttUserTimerCBFuncPtr)0;
#endif /* TM_USE_TIMER_CB */
    do
    {
        numSpunSlots = 0;
#ifndef TM_USE_TIMER_MULT_ACTIVE_LIST
        if (   (tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listCount != 0)
             && (tm_tmr_list_ready(tm_tmr_wheel_cur_time,
                                   TM_TIMER_ACTIVE_LIST)))
/* List is non empty */
        {
            tfTimerActiveListScan();
        }
#else /* TM_USE_TIMER_MULT_ACTIVE_LIST */
/* See if some timers are ready to be moved to the wheel */
        activeLists = tfTimerActiveListsScan(&activeListsExpTime);
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST */
        if (    (tm_tmr_wheel.twlNumberTimers != 0)
/* Some timers on the wheel */
             && (    (tm_tmr_lt(tm_tmr_wheel_cur_time, curTime))
/* and either wheel's current slot is behind (we can spin) */
                  || (tm_tmr_wheel_list(tm_tmr_wheel_cur_slot).listCount != 0))
/*     or the list at the current wheel slot non empty (we can execute) */
           )
        {
            numSpunSlots = tfTimerWheelSpinExecute(&curTime,
                                                   &numTimersExecuted);
            if (tm_tmr_wheel.twlNumberTimers == 0)
            {
/* Refresh wheel starting point */
/* offset for timer wheel and all lists  */
                tm_tmr_copy(curTime, tm_tmr_wheel_cur_time);
            }
        }
    }
#ifndef TM_USE_TIMER_MULT_ACTIVE_LIST
/*
 * while we are spinning the wheel, and there are expiring timers in the
 * active list
 */
    while (   (numSpunSlots != 0)
            && (tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listCount != 0)
            && (tm_tmr_list_ready(tm_tmr_wheel_cur_time,
                                  TM_TIMER_ACTIVE_LIST))
          );
#else /* TM_USE_TIMER_MULT_ACTIVE_LIST */
/*
 * while we are spinning the wheel, and there are expiring timers in the
 * active lists
 */
    while (    (numSpunSlots != 0)
            && (activeLists != 0)
            && (tm_tmr_leq(activeListsExpTime, tm_tmr_wheel_cur_time))
          );
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST */
#ifdef TM_USE_TIMER_CB
    userParam = tm_context(tvTimerCBParam);
    interval = TM_32BIT_ZERO;
    if (userTimerCBFuncPtr != (ttUserTimerCBFuncPtr)0)
/* If the user set the global CB function */
    {
/* Restore global cb function */
        tm_context(tvTimerCBFuncPtr) = userTimerCBFuncPtr;
/* Get the Call Back interval, and store the next Call Back expiration time */
        retCode = tfTimerLockedIntervalQuery(&interval
#ifdef TM_USE_TIMER_CACHE
                                   ,&(tm_context(tvTimerCtrl.timerCBTime))
#endif /* TM_USE_TIMER_CACHE */
                                   );
        if (retCode == -1)
        {
/* No need to call the user CB function */
            userTimerCBFuncPtr = (ttUserTimerCBFuncPtr)0;
            tm_context(tvTimerCtrl).timerCBPending = TM_8BIT_NO;
        }
        else
        {
            tm_context(tvTimerCtrl).timerCBPending = TM_8BIT_YES;
        }
    }
#endif /* TM_USE_TIMER_CB */
/* UNLOCK Active list */
    tm_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);

/* UNLOCK Execute */
    tm_unlock(&tm_context(tvTimerCtrl).timerExecuteLockEntry);
#ifdef TM_USE_TIMER_CB
    if (userTimerCBFuncPtr != (ttUserTimerCBFuncPtr)0)
    {
/* Notify the user of the interval before the next timer expires. */
        (*userTimerCBFuncPtr)(interval, userParam);
    }
#endif /* TM_USE_TIMER_CB */
    return;
} /* tfTimerExecute */

/****************************************************************************
* FUNCTION: tfTimerWheelSpinExecute
*
* PURPOSE: 
*   . Execute all the timers on expired slots on the wheel up to a maximum
*     of timers (if set by the user).
*   . Called by tfTimerExecute() with the timer execute lock, and the timer
*     active lists lock.
*      .. Each slot in the timer wheel has a list of timers that expire within
*         one tick. After a timer tick has elapsed, the wheel moves to the next
*         slot. The wheel has an upper limit of tvTimerMaxWheelIntv ms by
*         default, so there are
*        ((tvTimerMaxWheelIntv + tvTimerTickLength -1)/tvTimerTickLength) + 1)
*         slots.
*
*   . If the current wheel slot run time has expired, execute all the timers
*     on the list at that slot. Loop for more slots that are ready.
*   . If the user set a limit of timers executed per tfTimerExecute call
*     break out of the expired timers loop when that limit is reached.
*   . The wheel/active list(s) lock is removed during the call back function
*     of an expired timer, and could also be removed in tfTimerFree() if the
*     expired timer is freed and there is a cleanup function associated with
*     it.
*   . Save the next timer to run in context global, so that it can be updated
*     by any other timer routine that were to remove/move that next timer
*     (when the lock is removed).
*   . Remove the one shot timer, BEFORE calling the call back function, since
*     the user could re-add the timer in the call back routine.
*   . Increase the timer owner count BEFORE calling the timer call
*     back routine, so that the timer is not freed if it is removed in the
*     call back routine. The timer owner count is then decreased AFTER the call
*     back function returns, and the timer is freed if it had been removed in
*     the call back.
*   . AFTER the call back routine, if the timer is an auto fire timer, (and the
*     user did not remove/add/suspend the timer),
*     .. re-initialize the timer stop time.
*     .. Find out which active list/wheel slot list the timer should be appended
*        (based on interval of firing)
*     .. move timer to the end of the new list.
*        . Make sure to decrease the number of timers in the wheel, if the
*          timer is removed from the wheel.
*        . If the timer is added to an active list, and is the first timer in
*          that active list, make sure to update the next run time for that
*          list.
*   . If the user set the option to limit the maximum number of timers executed
*     per tfTimerExecute call, break out of the loop(s), if the limit is
*     reached and the current timer list has not finished executing.
*
* PARAMETERS:
*  curTimePtr           Pointer to a time structure containing the current time.
*  numTimersExecutedPtr Pointer to number of timers executed so far
*   None
*
* RETURNS:
* Number of slots spun
*
* NOTES:
*   Note that in this thread, in the call back function, any of the timer
*   routines could be called on the timer being executed or on another timer
*   (to remove, suspend, change the interval time, resume (another timer) or
*   add a new timer entry). For this reason the timer active list is unlocked
*   during the call back.
*
*   Note that in this thread The timer execute is locked and not unlocked, and
*   for this reason tfTimerLockedRemove() should never be called from a timer
*   execute call back function.
*
****************************************************************************/
static tt32Bit tfTimerWheelSpinExecute (ttTimeStructPtr curTimePtr,
                                        tt32BitPtr      numTimersExecutedPtr)
{
    ttTimerPtr              timerPtr;
    ttTmCBFuncPtr           callBackFunctionPtr;
    ttListPtr               listPtr;
    ttListPtr               toListPtr;
    ttTimeStruct            curTime;
    ttTimeStruct            nextRunTime;
    ttTimeStruct            wheelRunTime;
    tt32Bit                 interval;
    tt32Bit                 period;
    tt32Bit                 numTimersExecuted;
    tt32Bit                 numSpunSlots;
    ttGenericUnion          timerParm1;
    ttGenericUnion          timerParm2;
    int                     listNo;
    tt16Bit                 slotNo;
    tt16Bit                 toSlotNo;

    tm_tmr_copy_from_ptr(curTimePtr, curTime);
    numSpunSlots = TM_32BIT_ZERO;
    numTimersExecuted = *numTimersExecutedPtr;
    do
/* All the timers on the list at the current slot are ready to run */
    {
        slotNo = tm_tmr_wheel_cur_slot;
        listPtr = &tm_tmr_wheel_list(slotNo);
        if (listPtr->listCount != 0)
        {
/*
 * Save the next timer to execute in context global because we unlock for the
 * call back, and possibly in tfTimerFree()
 */
            tm_context(tvTimerCtrl).timerExecNextNodePtr = 
                                   listPtr->listHeadNode.nodeNextPtr;
            while (    (tm_context(tvTimerMaxExecute) == 0)
                    || (numTimersExecuted
                                     < (tt32Bit)tm_context(tvTimerMaxExecute))
/* 
 * while we have not reached the maximum number of timers 
 */
                  )
            {
/* Timer to execute */
                timerPtr = (ttTimerPtr)(ttVoidPtr)
                                tm_context(tvTimerCtrl).timerExecNextNodePtr;
/* Save pointer to the next timer (before possibly unlocking) */
                tm_context(tvTimerCtrl).timerExecNextNodePtr =
                     tm_context(tvTimerCtrl).timerExecNextNodePtr->nodeNextPtr;
/*
 * Execute ownership (before we potentially remove the timer from the list,
 * and call back)
 */
                timerPtr->tmrOwnerCount++;
                if (!(timerPtr->tmrFlags & TM_TIM_AUTO))
/* Non AUTO timer, remove it */
                {
                    tm_tmr_wheel.twlNumberTimers--;
                    tm_timer_remove_from_list(timerPtr, listPtr);
                }
/* Mark current timer for the call back function */
                timerPtr->tmrCallBackFlag = TM_8BIT_YES;
/* Save the call back function pointer and parameters before unlocking */
                callBackFunctionPtr = timerPtr->tmrCallBackFunctionPtr;
                timerParm1 = timerPtr->tmrUserParm1;
                timerParm2 = timerPtr->tmrUserParm2;
/* UNLOCK the timer lists */
                tm_unlock(&(tm_context(tvTimerCtrl).timerListLockEntry));
/* Call the call back function with the parameters */
                (*(callBackFunctionPtr))(timerPtr,
                                         timerParm1,
                                         timerParm2);

/* LOCK the timer lists */
                tm_lock_wait(&(tm_context(tvTimerCtrl).timerListLockEntry));
                tm_assert(tfTimerWheelSpinExecute,
                          timerPtr->tmrOwnerCount != 0);
/* Execute ownership */
                timerPtr->tmrOwnerCount--;
                if (timerPtr->tmrOwnerCount == 0)
/* 
 * Free the timer if is has been removed. 
 */
                {
                    tm_assert(tfTimerWheelSpinExecute,
                              tm_timer_not_in_list(timerPtr->tmrListNo));
                    tfTimerFree(timerPtr, TM_8BIT_ZERO);
                }
                else
                {
                    if (    (timerPtr->tmrFlags & TM_TIM_AUTO)
                         && (tm_timer_not_reinit(timerPtr)))
/*
 * Auto timer, (and the user did not add/remove/suspend/call tfTimerNewTime()
 * that caused the timer to move to a different active list),
 * re-initialize the timer stop time, and move the timer to the end of the
 * appropriate list.
 */
                    {
/* restart the timer */
                        tm_tmr_init_stop_time(timerPtr,
                                              curTime,
                                              timerPtr->tmrInterval);
/* Which list/wheel slot should the timer go to? */
                        interval = timerPtr->tmrInterval;
                        if (    (tm_tmr_wheel.twlNumberTimers != 0)
                             && (tm_tmr_lt(tm_tmr_wheel_cur_time, curTime)))
                        {
                            interval = interval +
                                               curTime.timMsecs
                                             - tm_tmr_wheel_cur_time.timMsecs;
                        }
                        tm_tmr_get_list(interval,
                                        listNo,
                                        toSlotNo,
                                        period, toListPtr);
/* Move list */
                        tm_timer_move_from_to_list(timerPtr,
                                                   listPtr,
                                                   toListPtr);
                        if (timerPtr->tmrListNo != (tt8Bit)listNo)
/* moving from the wheel to a list */
                        {
                            tm_tmr_wheel.twlNumberTimers--;
                            if (toListPtr->listCount == 1)
/* Moving to an empty list. Refresh next run time on that list */
                            {
                                tm_tmr_add_time(
                                            tm_tmr_list(listNo).tmlNextRunTime,
                                            tm_tmr_wheel_cur_time, period);
                            }
                            timerPtr->tmrListNo = (tt8Bit)listNo;
                        }
                        timerPtr->tmrWheelSlot = toSlotNo;
                    }
/* Reset timer call back flag */
                    timerPtr->tmrCallBackFlag = TM_8BIT_ZERO;
                }
                if (tm_context(tvTimerMaxExecute) != 0)
/* Update the number of executed timers (if the user set a limit) */
                {
                    (numTimersExecuted)++;
                }
                if (tm_context(tvTimerCtrl).timerExecNextNodePtr
                                          == &(listPtr->listHeadNode))
                {
/* End of the list */
                    break; /* while */
                }
            }
/* Done with this active list. Reset context global next timer */
            tm_context(tvTimerCtrl).timerExecNextNodePtr = (ttNodePtr)0;
        } /* if list count */
        if (    (listPtr->listCount != 0)
/*
 * Did not execute all timers in the current wheel slot list. Could happen
 * if we reached the maximum number of timers to execute, before executing
 * all the timers in a list.
 */
             || (tm_tmr_wheel.twlNumberTimers == 0)
/* No more timers on the wheel, no need to execute */
             || (tm_tmr_eq(curTime, tm_tmr_wheel_cur_time))
/* wheel is at current time, no need to spin */
           )
        {
            break; /* do while */
        }
        tm_tmr_copy(tm_tmr_wheel_cur_time, wheelRunTime);
        tm_tmr_add_time(nextRunTime, wheelRunTime,
                        tvTimerTickLength); 
/* Check if we should spin the wheel */
        if (tm_tmr_lt(curTime, nextRunTime))
/*
 * Next slot is ahead of current time, has not expired yet. This check only
 * necessary if tvTime is not updated by adding the tick length.
 */
        {
            break; /* do while */
        }
        numSpunSlots++;
        slotNo++;
        if (slotNo >= tm_tmr_wheel_number_slots)
/* circular list */
        {
            slotNo = 0;
        }
/* spin the wheel to the next slot */
        tm_tmr_wheel_cur_slot = slotNo;
        tm_tmr_copy(nextRunTime, tm_tmr_wheel_cur_time);
/* while wheel is spinning. Always true when we reach here. */
    } while (numSpunSlots);
    *numTimersExecutedPtr = numTimersExecuted;
    return numSpunSlots;
}

#ifndef TM_USE_TIMER_MULT_ACTIVE_LIST
/****************************************************************************
* FUNCTION: tfTimerActiveListScan
*
* PURPOSE: 
*   Scan the timer active list and move expiring timers to the wheel, if
*   warranted, based on the time remanining on the timer.
*   Called by tfTimerExecute() with the timer execute lock, and the timer
*   active list lock, when the timer active list needs to be scanned.
*      Every tvTimerMaxWheelIntv ms the active list is run to check
*      for timers that need to be moved to the wheel.
*
*   . In a loop, san the active list, and move the expiring timers to the
*     wheel.
*   . If the active list is still not empty, refresh the active list next run
*     time (by adding the active list minimum interval to the current slot run
*     time)
*   . Return
* PARAMETERS:
*  NONE
*
* RETURNS:
*  NONE
*
* NOTES:
*
****************************************************************************/
static void tfTimerActiveListScan (void)
{
    ttTimerPtr              timerPtr;
    ttNodePtr               nextNodePtr;
    ttListPtr               listPtr;
    tt32Bit                 interval;
    int                     listNo;
    int                     toListNo;
    tt16Bit                 slotNo;

    listNo = TM_TIMER_ACTIVE_LIST;
/* First node on the list */
    nextNodePtr = tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listHeadNode.nodeNextPtr;
    do
    {
/* Timer to check */
        timerPtr = (ttTimerPtr)(ttVoidPtr)nextNodePtr; 
/* Save pointer to the next timer (before possibly removing the timer) */
        nextNodePtr = nextNodePtr->nodeNextPtr;
/* remaining time (from current wheel slot time) before timer fires */
        tm_tmr_remaining_time(interval, timerPtr,
                              tm_tmr_wheel_cur_time);
/* 
 *  check to see if the time left on the timer is less than or equal to miminum
 *  on the list
 */
        if (interval <= tm_tmr_list(TM_TIMER_ACTIVE_LIST).tmlMinInterval)
/* change active list */
        {
            tm_tmr_get_wheel_slot(interval,
                                  toListNo, slotNo,
                                  listPtr);
            tm_tmr_wheel.twlNumberTimers++;
            tm_timer_move_from_to_list(timerPtr,
                                   &(tm_tmr_list_list(TM_TIMER_ACTIVE_LIST)),
                                   listPtr);
            timerPtr->tmrListNo = (tt8Bit)toListNo;
            timerPtr->tmrWheelSlot = slotNo;
        }
    } while (nextNodePtr !=
                      &(tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listHeadNode));
    if (tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listCount != 0)
    {
/* Done running the list. Refresh next run time */
        tm_tmr_add_time(tm_tmr_list(listNo).tmlNextRunTime,
                        tm_tmr_wheel_cur_time,
                        tm_tmr_list_min_interval(listNo));
    }
    return;
}
#else /* TM_USE_TIMER_MULT_ACTIVE_LIST */
/****************************************************************************
* FUNCTION: tfTimerActiveListsScan
*
* PURPOSE: 
*   Scan the timer lists and move expiring timers to a list on the wheel or
*   to another active list based on the time remanining on the timer.
*
*   Called by tfTimerExecute() with the timer execute lock, and the timer
*   active lists lock.
*         Each active list contains timers that are due to expire
*         in more than tvTimerMaxWheelIntv ms. (the first one in more than
*         tvTimerMaxWheelIntv ms, the second one in more than
*         2 * tvTimerMaxWheelIntv ms1000ms, the third one in more than
*         8 * tvTimerMaxWheelIntv ms4000ms, and the fourth one in more than
*         64 * tvTimerMaxWheelIntv ms).
*         Every tvTimerMaxWheelIntv ms the first active list is run to check
*         for timers that need to be moved to the wheel.
*         Every 2 * tvTimerMaxWheelIntv ms, the second active list is run to
*         check for timer that need to be moved to a lesser list
*         Every 8 * tvTimerMaxWheelIntv, the third active list is run to
*         check for timer that need to be moved to a lesser list
*         Every 64 * tvTimerMaxWheelIntv, the fourth active list is run to
*         check for timer that need to be moved to a lesser list
*
*   . Initialize the number of active lists to 0
*   . In a loop san all the active lists.
*   . If an active list has timers
*      .. If it is ready to run:
*          ... In a loop move the expiring timers to their appropriate lists
*              (either at a wheel slot, or to another active list)
*          ... Refresh the active list next run time (by adding the active list
*              minimum interval to the current slot run time)
*      .. update the active lists expiration time (using passed pointer) if
*         it has not been initialized yet of if it less than the previous one.
*      .. Increase the number of active lists
*   . Return the number of active lits.
* PARAMETERS:
* Pointer to next active lists expiration time. To be updated in this function.
*
* RETURNS:
* Number of active lists that have timers
*
* NOTES:
*
****************************************************************************/
static int tfTimerActiveListsScan (ttTimeStructPtr expTimePtr)
{
    ttTimerPtr              timerPtr;
    ttNodePtr               nextNodePtr;
    ttListPtr               listPtr;
    tt32Bit                 interval;
    tt32Bit                 period;
    int                     listNo;
    int                     toListNo;
    int                     activeLists;
    tt16Bit                 slotNo;

    activeLists = 0;
    for (listNo = TM_TIMER_ACTIVE_LIST;
         listNo <= TM_TIMER_LAST_ACTIVE_LIST;
         listNo++)
    {
        if (tm_tmr_list_list(listNo).listCount != 0)
/* List is non empty */
        {
            if (tm_tmr_list_ready(tm_tmr_wheel_cur_time, listNo))
/* List ready to be run */
            {
/* First node on the list */
                nextNodePtr = tm_tmr_list_list(listNo).listHeadNode.nodeNextPtr;
                do
                {
/* Timer to check */
                    timerPtr = (ttTimerPtr)(ttVoidPtr)nextNodePtr; 
/* Save pointer to the next timer (before possibly removing the timer) */
                    nextNodePtr = nextNodePtr->nodeNextPtr;
/* remaining time (from current wheel slot time) before timer fires */
                    tm_tmr_remaining_time(interval, timerPtr,
                                          tm_tmr_wheel_cur_time);
/* 
 *  check to see if the time left on the timer is less than or equal to miminum
 *  on the list
 */
                    if (interval <= tm_tmr_list(listNo).tmlMinInterval)
/* change active list */
                    {
                        tm_tmr_get_list(interval,
                                        toListNo, slotNo, period, listPtr);
                        if (toListNo == TM_TIMER_WHEEL)
                        {
/* Adding to the wheel */
                            tm_tmr_wheel.twlNumberTimers++;
                        }
                        else if (listPtr->listCount == 0)
/* Adding to an empty list. */
                        {
/* Refresh next run time on that list. Use offset from the current wheel
 * slot time.
 */
                            tm_tmr_add_time(
                                        tm_tmr_list(toListNo).tmlNextRunTime,
                                        tm_tmr_wheel_cur_time, period);
/* Keep track of the smallest time when a list expires */
                            if (    (activeLists == 0)
                                 || (tm_tmr_lt_ptr2(
                                           tm_tmr_list(toListNo).tmlNextRunTime,
                                            expTimePtr))
                               )
                            {
                                tm_tmr_copy_to_ptr(
                                          tm_tmr_list(toListNo).tmlNextRunTime,
                                          expTimePtr);
                            }
                            activeLists++;
                        }
                        tm_timer_move_from_to_list(timerPtr,
                                                   &(tm_tmr_list_list(listNo)),
                                                   listPtr);
                        timerPtr->tmrListNo = (tt8Bit)toListNo;
                        timerPtr->tmrWheelSlot = slotNo;
                    }
                } while (nextNodePtr !=
                                &(tm_tmr_list_list(listNo).listHeadNode));
                if (tm_tmr_list_list(listNo).listCount != 0)
                {
/* Done running the list. Refresh next run time */
                    tm_tmr_add_time(tm_tmr_list(listNo).tmlNextRunTime,
                                    tm_tmr_wheel_cur_time,
                                    tm_tmr_list_min_interval(listNo));
                }
            }
            if (tm_tmr_list_list(listNo).listCount != 0)
            {
/* List non empty */
                if (    (activeLists == 0)
                     || (tm_tmr_lt_ptr2(tm_tmr_list(listNo).tmlNextRunTime,
                                        expTimePtr))
                   )
/* Keep track of the smallest time when a list expires */
                {
                    tm_tmr_copy_to_ptr(tm_tmr_list(listNo).tmlNextRunTime,
                                       expTimePtr);
                }
                activeLists++;
            }
        }
    }
    return activeLists;
}
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST */
#endif /* TM_USE_TIMER_CACHE */

#ifndef tvTime
/****************************************************************************
* FUNCTION: tfTimerUpdateIsr
*
* PURPOSE: 
*   Method 1: Update the timers from an ISR
*   We just update the global time value
*
* PARAMETERS:
*   None
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tfTimerUpdateIsr(void)
{
    tt32Bit tempTime;

    tempTime = tvTime;
    tvTime += tvTimerTickLength;
    if (tempTime > tvTime)
    {
        tvTimeRollOver++;
    }
    return;
}
#endif /* tvTime */


#ifndef tvTime
/****************************************************************************
* FUNCTION: tfTimerUpdate
*
* PURPOSE: 
*   Method 2: Update the timers from a Task
*   tfTimerUpdate and tfTimerExecute are called from
*   the same thread.
*   Call tfTimerUpdateIsr().
*
* PARAMETERS:
*   None
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tfTimerUpdate (void)
{
    tfTimerUpdateIsr();
    return;
}
#endif /* tvTime */

#ifdef TM_DEMO_TIME_LIMIT
void tfTimerDemoCallBack (ttVoidPtr timerVoidPtr,
                          ttGenericUnion userParm1,
                          ttGenericUnion userParm2)
{
    ttTimerPtr  timerPtr;

    TM_UNREF_IN_ARG(userParm1);
    TM_UNREF_IN_ARG(userParm2);
    timerPtr = (ttTimerPtr)timerVoidPtr;
    timerPtr->tmrUserParm1.gen32bitParm = 
              timerPtr->tmrUserParm1.gen32bitParm
            + (timerPtr->tmrInterval / TM_UL(1000));
    if (timerPtr->tmrUserParm1.gen32bitParm >= TM_DEMO_LIMIT_SEC)
    {
        tfKernelError("tfTimerDemoCallBack", "demo time limit has expired.");
    }
    return;
}
#endif /* TM_DEMO_TIME_LIMIT */

#ifdef TM_USE_TIMER_INTERVAL_QUERY
/****************************************************************************
* FUNCTION: tfTimerIntervalQuery
*
* PURPOSE:
*   Get the interval of time before tfTimerExecute needs to be called.
*
*   Lock the timer list(s) locks
*   Call tfTimerLockedIntervalQuery
*   UnLock the timer list(s) lock
*
* PARAMETERS:
*   intervalPtr:
*       pointer to the interval to update.
*
* RETURNS:
*   0   interval was updated
*   -1  no need to call tfTimerExecute()
*
*
****************************************************************************/
int tfTimerIntervalQuery(ttUser32BitPtr intervalPtr)
{
    int retCode;

    retCode = -1;
    if (intervalPtr != (ttUser32BitPtr)0)
    {
/* LOCK the timer lists */
        tm_call_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);
        retCode = tfTimerLockedIntervalQuery(intervalPtr
#ifdef TM_USE_TIMER_CACHE
                                           , (ttTimeStructPtr)0
#endif /* TM_USE_TIMER_CACHE */
                                            );
/* UNLOCK the timer lists */
        tm_call_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);
    }
    return retCode;
}
#endif /* TM_USE_TIMER_INTERVAL_QUERY */

#if defined(TM_USE_TIMER_INTERVAL_QUERY) || defined(TM_USE_TIMER_CB)
/****************************************************************************
* FUNCTION: tfTimerLockedIntervalQuery
*
* PURPOSE:
*   Get the interval of time before tfTimerExecute needs to be called.
*   Called with the timer lists locks on.
*
*   . If caching is enabled:
*     .. If there is a timer on the wheel:
*       ... use the number of empty slots before the first non empty slot list
*           on the wheel
*     .. otherwise the nextRunTime on the first active list that has a timer.
*
*   . If caching is not emabled, walk the active list to find the next timer
*     to expire.
*
* PARAMETERS:
*   intervalPtr:
*       pointer to the interval to update.
*
* RETURNS:
*   0   interval was updated
*   -1  no need to call tfTimerExecute()
*
****************************************************************************/
static int tfTimerLockedIntervalQuery (ttUser32BitPtr   intervalPtr
#ifdef TM_USE_TIMER_CACHE
                                     , ttTimeStructPtr  nextRunTimePtr
#endif /* TM_USE_TIMER_CACHE */
                                      )
{
    ttTimeStruct    curTime;
#ifdef TM_USE_TIMER_CACHE
    ttTimeStruct    expTime;
    ttTimeStructPtr expTimePtr;
    tt32Bit         interval;
#else /* !TM_USE_TIMER_CACHE */
    ttTimerPtr      timerPtr;
    ttNodePtr       nextNodePtr;
    ttTimeStruct    expStopTime;
    tt32Bit         smallestInterval;
    unsigned int    walkCount;
#endif /* !TM_USE_TIMER_CACHE */
    unsigned int    count;
#ifdef TM_USE_TIMER_CACHE
    int             listNo;
#endif /* TM_USE_TIMER_CACHE */
    int             retCode;
#ifndef TM_USE_TIMER_CACHE
    tt8Bit          savedInterval;
#endif /* !TM_USE_TIMER_CACHE */

    retCode = -1;
    tm_tmr_get_current_time(curTime);
#ifdef TM_USE_TIMER_CACHE
    expTimePtr = &curTime;
    if (tm_tmr_wheel.twlNumberTimers != 0)
    {
/* Make sure we have at least one timer on the wheel */
        retCode = 0;
        (void)tfTimerFirstNonEmptySlot(&count);
        interval = count * tvTimerTickLength;
        tm_tmr_add_time(expTime, tm_tmr_wheel_cur_time, interval);
        if (tm_tmr_leq(expTime, curTime))
        {
            *intervalPtr = 0;
        }
        else
        {
            *intervalPtr = expTime.timMsecs - curTime.timMsecs;
            expTimePtr = &expTime;
        }
    }
    else
    {
#ifdef TM_USE_TIMER_MULT_ACTIVE_LIST
        listNo = tfTimerFirstNonEmptyList();
        if (listNo <= TM_TIMER_LAST_ACTIVE_LIST)
#else /* !TM_USE_TIMER_MULT_ACTIVE_LIST */
        listNo = TM_TIMER_ACTIVE_LIST;
        if (tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listCount != 0)
#endif /* !TM_USE_TIMER_MULT_ACTIVE_LIST */
        {
            retCode = 0;
            if (tm_tmr_leq(tm_tmr_list(listNo).tmlNextRunTime,
                            curTime))
            {
                *intervalPtr = 0;
            }
            else
            {
                *intervalPtr =
                          tm_tmr_list(listNo).tmlNextRunTime.timMsecs
                        - curTime.timMsecs;
                expTimePtr = &(tm_tmr_list(listNo).tmlNextRunTime);
            }
        }
    }
    if (nextRunTimePtr != (ttTimeStructPtr)0)
    {
        tm_tmr_copy_ptr(expTimePtr, nextRunTimePtr);
    }
#else /* !TM_USE_TIMER_CACHE */
    walkCount = tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listCount;
    if (walkCount != 0)
    {
/* at least one timer */
        retCode = 0;
        smallestInterval = TM_32BIT_ZERO;
        savedInterval = TM_8BIT_NO;
        expStopTime.timMsecs = TM_32BIT_ZERO;
        expStopTime.timRollover = TM_32BIT_ZERO;
        nextNodePtr =
                tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listHeadNode.nodeNextPtr;
/* walk the active timer list */
        for (count = 0; count < walkCount; count++)
        {
            tm_assert(tfTimerIntervalQuery,
                      nextNodePtr != 
                      &(tm_tmr_list_list(TM_TIMER_ACTIVE_LIST).listHeadNode));
            timerPtr = (ttTimerPtr)(ttVoidPtr)nextNodePtr;
/* calculate the time remaining on the timer */
            if (    (savedInterval == TM_8BIT_NO)
                 || (tm_tmr_lt(timerPtr->tmrStopTime, expStopTime)) )
            {
/* Keep track of next timer to expire */
                tm_tmr_copy(timerPtr->tmrStopTime, expStopTime);
                tm_tmr_remaining_time(smallestInterval, timerPtr, curTime);
                savedInterval = TM_8BIT_YES;
                if (smallestInterval == TM_32BIT_ZERO)
                {
                    break;
                }
            }
/* Go to the next timer */
            nextNodePtr = nextNodePtr->nodeNextPtr;
        }
        *intervalPtr = smallestInterval;
    }
#endif /* !TM_USE_TIMER_CACHE */
    return retCode;
}

#ifdef TM_USE_TIMER_CACHE
#ifdef TM_USE_TIMER_WHEEL
/****************************************************************************
* FUNCTION: tfTimerFirstNonEmptySlot
*
* PURPOSE:
*   Returns first non empty wheel slot list, and give the number of empty
*   slots before it.
*   Called with the timer lists lock on.
*   Called only if the timer wheel is not empty.
*
*   . In a loop, check for first slot that has a non empty list, and keep
*     track of number of slots that are skipped to reach that first non
*     empty slot.
*   . Update count of empty slots pointed to by the caller.
*   . Return first non empty slot.
*
* PARAMETERS:
*   countPtr:
*       pointer to count of number of empty slots to update.
*
* RETURNS:
*   slotNo:
*       First non empty slot
*
****************************************************************************/
static tt16Bit tfTimerFirstNonEmptySlot(ttUIntPtr countPtr)
{
    unsigned int   count;
    tt16Bit        slotNo;
    tt16Bit        firstSlotNo;
    tt16Bit        numberSlots;

    firstSlotNo = tm_tmr_wheel_cur_slot;
    numberSlots = tm_tmr_wheel_number_slots;
    slotNo = firstSlotNo;
    count = 0;
    do
    {
        if (tm_tmr_wheel_list(slotNo).listCount != 0)
        {
            break;
        }
        count++;
        slotNo++;
        if (slotNo >= numberSlots)
        {
            slotNo = 0;
        }
    } while (slotNo != firstSlotNo);
    if (countPtr)
    {
        *countPtr = count;
    }
    return slotNo;
}
#endif /* TM_USE_TIMER_WHEEL */
#endif /* TM_USE_TIMER_CACHE */

#ifdef TM_USE_TIMER_CACHE
#ifdef TM_USE_TIMER_MULT_ACTIVE_LIST
/****************************************************************************
* FUNCTION: tfTimerFirstNonEmptyList
*
* PURPOSE:
*   Return first non empty list
*   Called with the timer lists lock on.
*   Called only if the timer wheel is not empty.
*
*   . In a loop, check for first list that is not empty
*   . Return first non empty list.
*   . If all lists are empty, returns TM_TIMER_LAST_ACTIVE_LIST + 1.
*
* PARAMETERS:
*   NONE
*
* RETURNS:
*   First non empty list, if there is one, TM_TIMER_LAST_ACTIVE_LIST + 1
*   otherwise.
*
*
****************************************************************************/
static int tfTimerFirstNonEmptyList(void)
{
    int listNo;

    for (listNo = TM_TIMER_ACTIVE_LIST;
         listNo <= TM_TIMER_LAST_ACTIVE_LIST;
         listNo++)
    {
        if (tm_tmr_list_list(listNo).listCount != 0)
/* Make sure we have at least one timer on the list */
        {
            break;
        }
    }
    return listNo;
}
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST */
#endif /* TM_USE_TIMER_CACHE */
#endif /* defined(TM_USE_TIMER_INTERVAL_QUERY) || defined(TM_USE_TIMER_CB) */

#if defined(TM_USE_TIMER_CB) && defined(TM_USE_TIMER_CACHE)

/****************************************************************************
* FUNCTION: tfRegisterTimerCB
*
* PURPOSE:
*
* User can register at timer CB function that the stack will call
* with the next interval (in milliseconds) of tfTimerExecute() execution.
* The user will then call tfTimerExecute() after that interval has elapsed.
* userParm, is a user defined parameter that is passed as is to the
* user CB function, when the user CB function is called.
* Protected with the TIMER EXECUTE LOCK, and TIMER LISTS LOCK.
*
* PARAMETERS:
*   userTimerExpFunc:
*       User timer CB function.
*   userParam:
*       user defined parameter.
*
* RETURNS:
*   TM_ENOERROR     success
*   TM_EPERM        Treck has not started.
*
* NOTES:
*
****************************************************************************/
int tfRegisterTimerCB(ttUserTimerCBFuncPtr userTimerExpFunc,
                      ttUserGenericUnion   userParam)
{
    tt32Bit                 interval;
    int                     errorCode;
    int                     retCode;

    retCode = -1;
/* LOCK the timer lists */
    if (    (tvGlobalsSet == TM_UL(0))
#ifdef TM_MULTIPLE_CONTEXT
         && (tvCurrentContext != (ttContextPtr)0)
#endif /* TM_MULTIPLE_CONTEXT */
         && (tm_context(tvContextGlobalsSet) == TM_UL(0))
       )
/* tfStartTreck() has been called */
    {
        tm_call_lock_wait(&tm_context(tvTimerCtrl).timerExecuteLockEntry);
        tm_call_lock_wait(&tm_context(tvTimerCtrl).timerListLockEntry);
        tm_context(tvTimerCBFuncPtr) = userTimerExpFunc;
        tm_context(tvTimerCBParam) = userParam;
        if (userTimerExpFunc != (ttUserTimerCBFuncPtr)0)
        {
/* Check if a timer has already been added */
            retCode = tfTimerLockedIntervalQuery(&interval
#ifdef TM_USE_TIMER_CACHE
                                               , (ttTimeStructPtr)0
#endif /* TM_USE_TIMER_CACHE */
                                   );
        }
        else
        {
            interval = TM_32BIT_ZERO; /* compiler warning */
        }
/* UNLOCK the timer lists */
        tm_call_unlock(&tm_context(tvTimerCtrl).timerListLockEntry);
        tm_call_unlock(&tm_context(tvTimerCtrl).timerExecuteLockEntry);
        errorCode = TM_ENOERROR;
        if (retCode == 0)
        {
/* Notify the user of the next timer expiration */
            (*userTimerExpFunc)(interval, userParam);
        }
    }
    else
    {
        errorCode = TM_EPERM;
    }
    return errorCode;
}
#endif /* TM_USE_TIMER_CB && TM_USE_TIMER_CACHE */

#ifdef TM_USE_STOP_TRECK
static int tfTimerCBRemove (ttNodePtr nodePtr, ttGenericUnion genParm)
{
    ttListPtr       listPtr;
    ttTimerPtr      timerPtr;
    ttTmCUFuncPtr   cleanUpFunctionPtr; 
#ifdef TM_MULTIPLE_CONTEXT
    ttVoidPtr       contextHandle;
#endif /* TM_MULTIPLE_CONTEXT */
    tt8Bit          userOwnTimer;

    listPtr = ((ttListCBRemovePtr)(genParm.genVoidParmPtr))->lcbrListPtr;
#ifdef TM_MULTIPLE_CONTEXT
    contextHandle = ((ttListCBRemovePtr)(genParm.genVoidParmPtr))->
                                                            lcbrContextHandle;
#endif /* TM_MULTIPLE_CONTEXT */
    timerPtr = (ttTimerPtr)(ttVoidPtr)nodePtr;
    tm_timer_remove_from_list(timerPtr, listPtr);
    if (timerPtr->tmrOwnerCount == 0)
    {
/* Save this bit before the clean up function! */
        userOwnTimer = (tt8Bit)
                    (timerPtr->tmrFlags & TM_TIM_USER_OWN);
        cleanUpFunctionPtr = timerPtr->tmrCleanUpFunctionPtr;
        if (cleanUpFunctionPtr != (ttTmCUFuncPtr)0)
        {
/* UNLOCK the timer lists */
            tm_unlock(&tm_context_var(tvTimerCtrl).timerListLockEntry);
            (*(cleanUpFunctionPtr))(timerPtr, TM_8BIT_ZERO);
/* LOCK the timer lists */
            tm_lock_wait(&tm_context_var(tvTimerCtrl).timerListLockEntry);
        }
        if (userOwnTimer == TM_8BIT_NO)
/* Allocated by the timer */
        {
            tm_kernel_free(timerPtr);
        }
    }
    return TM_8BIT_NO;
}
#endif /* TM_USE_STOP_TRECK */

#ifdef TM_USE_STOP_TRECK

#ifndef TM_TIMER_LAST_ACTIVE_LIST
#define TM_TIMER_LAST_ACTIVE_LIST  TM_TIMER_ACTIVE_LIST 
#endif /* !TM_TIMER_LAST_ACTIVE_LIST */

void tfTimerDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
    ttListPtr               listPtr;
    ttGenericUnion          genParm;
    struct tsListCBRemove   listCBRemoveStruct;
    unsigned                i;
#ifdef TM_USE_TIMER_CACHE 
#ifdef TM_USE_TIMER_WHEEL
    unsigned                numSlots;
#endif /* TM_USE_TIMER_WHEEL */
#endif /* TM_USE_TIMER_CACHE  */

/* Execute LOCK (to lock out timer removal (tfTimerLockedRemove())) */
    tm_lock_wait(&tm_context_var(tvTimerCtrl).timerExecuteLockEntry);
    
/* LOCK the timer lists */
    tm_lock_wait(&tm_context_var(tvTimerCtrl).timerListLockEntry);
    tm_context_var(tvTimerCtrl).timerExecNextNodePtr = (ttNodePtr)0;
#ifdef TM_MULTIPLE_CONTEXT
    listCBRemoveStruct.lcbrContextHandle = contextHandle;
#endif /* TM_MULTIPLE_CONTEXT */
    genParm.genVoidParmPtr = (ttVoidPtr)&listCBRemoveStruct;
    for (i = TM_TIMER_SUSPEND_LIST;
         i <= TM_TIMER_LAST_ACTIVE_LIST;
         i++)
    {
        listPtr = &(tm_context_var(tvTimerCtrl).timerList[i - 1].tmlList);
        if (listPtr->listCount != 0)
        {
            listCBRemoveStruct.lcbrListPtr = listPtr;
            tfListWalk(listPtr, tfTimerCBRemove, genParm);
        }
    }
#ifdef TM_USE_TIMER_CACHE 
#ifdef TM_USE_TIMER_WHEEL
    if (tm_tmr_wheel.twlNumberTimers != 0)
    {
        numSlots = (unsigned)
                        tm_context_var(tvTimerCtrl).timerWheel.twlNumberSlots;
        for (i = 0; i < numSlots; i++)
        {
            listPtr = &(tm_context_var(tvTimerCtrl).timerWheel.twlListPtr[i]);
            if (listPtr->listCount != 0)
            {
                listCBRemoveStruct.lcbrListPtr = listPtr;
                tfListWalk(listPtr, tfTimerCBRemove, genParm);
            }
        }
    }
#endif /* TM_USE_TIMER_WHEEL */
#endif /* TM_USE_TIMER_CACHE  */
/* UNLOCK Active list */
    tm_unlock(&tm_context_var(tvTimerCtrl).timerListLockEntry);
/* UNLOCK Execute */
    tm_unlock(&tm_context_var(tvTimerCtrl).timerExecuteLockEntry);
}
#endif /* TM_USE_STOP_TRECK */
