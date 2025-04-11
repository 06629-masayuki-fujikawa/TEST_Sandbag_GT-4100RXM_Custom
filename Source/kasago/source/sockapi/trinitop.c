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
 * Description: BSD Sockets Interface (tfInitTreckOptions)
 *
 * Filename: trinitop.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trinitop.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2013/06/10 22:19:22JST $
 * $Author: pcarney $
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



/*
 * Set initial Treck options (before a successfull call to tfStartTreck)
 */
int tfInitTreckOptions(int optionName, ttUser32Bit optionValue)
{

    int     errorCode;

    errorCode = TM_ENOERROR;
    if (tvGlobalsSet != TM_GLOBAL_MAGIC_NUMBER)
    {
/* For single context will initialize tvCurrentContext */
        tfInitTreckMultipleContext();
    }
#ifdef TM_MULTIPLE_CONTEXT
    if (    (optionName != TM_OPTION_TICK_LENGTH)
         && (tvCurrentContext == (ttContextPtr)0) )
    {
/* User forgot to create and set the current context. Needed for setting 
 * any context variable.
 */ 
        errorCode = TM_EFAULT;
    }
    else
#endif /* TM_MULTIPLE_CONTEXT */
    {
/*
 * If globals variables haven't been initialized to default values, do
 * it now.
 */
        if (   
             (optionName != TM_OPTION_TICK_LENGTH) &&
             (tm_context(tvContextGlobalsSet) != TM_GLOBAL_MAGIC_NUMBER)
           )
        {
/* Initialize global variables for this context */
            tfContextGlobalVarsInit();
        }
        switch (optionName)
        {
            case TM_OPTION_SOCKETS_MAX:
                if (    (optionValue == 0)
                     || (optionValue >= (tt32Bit)TM_SOC_NO_INDEX) )
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
                    tm_context(tvMaxNumberSockets) = optionValue;
                }
                break;
#ifdef TM_USE_SOCKET_HASH_LIST
#ifdef TM_USE_TCP
            case TM_OPTION_SOTB_TCP_CON_HASH_SIZE:
                if (optionValue == 0)
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
/* TCP sockets table hash list size */
                    tm_context(tvSocketTableListHashSize[TM_SOTB_TCP_CON]) =
                                                                  optionValue;
                }
                break;
#endif /* TM_USE_TCP */
            case TM_OPTION_SOTB_NON_CON_HASH_SIZE:
                if (optionValue == 0)
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
/* non TCP sockets table hash list size */
                    tm_context(tvSocketTableListHashSize[TM_SOTB_NON_CON]) =
                                                                  optionValue;
                }
                break;
#endif /* TM_USE_SOCKET_HASH_LIST */
#ifdef TM_USE_TCP
            case TM_OPTION_TIME_WAIT_HASH_SIZE:
                if (optionValue == 0)
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
/* TCP time wait vetors hash table size */
                    tm_context(tvTcpTmWtTableHashSize) = optionValue;
                }
                break;
#endif /* TM_USE_TCP */
            case TM_OPTION_TICK_LENGTH:
                if (optionValue != 0)
                {
                    tvTimerTickLength = (unsigned int)optionValue;
                }
                else
                {
                    errorCode = TM_EINVAL;
                }
                break;
#ifdef TM_USE_TIMER_CACHE
#ifdef TM_USE_TIMER_WHEEL
            case TM_OPTION_TIMER_MAX_WHEEL_INTV: 
/* Maximum timer interval on the timer wheel */
                if (optionValue >= (tt32Bit)tvTimerTickLength )
                {
                    tm_context(tvTimerMaxWheelIntv) = optionValue;
                }
                else
                {
                    errorCode = TM_EINVAL;
                }
                break;
#endif /* TM_USE_TIMER_WHEEL */
#endif /* TM_USE_TIMER_CACHE */
#ifdef TM_USE_IPV4
             case TM_OPTION_DHCP_MAX_ENTRIES:
                if ((int)optionValue < 0)
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
                    tm_context(tvMaxUserDhcpEntries) = (int)optionValue;
                }
                break;
#endif /* TM_USE_IPV4 */
             default:
                    errorCode = tfSetTreckOptions(optionName, optionValue);
        }
    } /* else */
    return errorCode;
}

