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
 * Description: Check that an IP address is not in use by another host
 *              on a given network.
 *
 * Filename: trarpchk.c
 * Author: Odile
 * Date Created: 02/27/01
 * $Source: source/trarpchk.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:03JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h> /* For TM_USE_IPV4 */
#include <trmacro.h> /* For LINT_UNUSED_HEADER */

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * Local functions.
 */
void tfArpChkIncoming(
#ifdef TM_USE_STRONG_ESL
                       ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                       int              operation,
                       tt4IpAddress     senderNetAddress,
                       tt4IpAddress     targetNetAddress,
                       tt8BitPtr        enetAddress );

void tfArpChkTimeOut(ttVoidPtr      timerBlockPtr,
                     ttGenericUnion userParm1,
                     ttGenericUnion userParm2);

void tfArpChkSend(ttDeviceEntryPtr devPtr, tt4IpAddress ipAddress);

/*
 * tfUseCollisionDetection description
 * Allow the Treck stack to check that no other host is using a given IP
 * address. This command instructs the Treck stack to store the given
 * IP address, so that it can later on check for ARP requests, or replies
 * coming from any interface, not originated by this host, and whose
 * ARP sender addresses are the same as the given IP address. The collision
 * detection will only start after the user calls tfUserStartArpSend, or
 * tfStartArpSend. The collision detection will continue until the user calls
 * tfCancelCollisionDetection. If the interface given by tfUserStartArpSend
 * is not configured yet, the Treck stack will also check for ARP probes
 * sent by other hosts. The user gives a call back function to be called
 * when a match on an ARP request, ARP reply, or ARP probe occurs. The
 * interface handle passed to the call back function is the interface
 * handle passed to the tfUserStartArpSend function. The call
 * back function will also be called when the timeout
 * given to the tfUserStartArpSend expires. The collision detection will
 * continue until the user calls tfCancelCollisionDetection.
 * The user will pass the IP address to check, a call back function, and
 * a parameter to be passed as is to the call back function.
 * 1. tfUseCollisionDetection  will first check the ARP cache
 * for a match on the IP address to see whether another host is already
 * using that IP address. If a match is found, it will return
 * TM_EADDRINUSE, to indicate that another host is using the IP address.
 * In that case the collision detection will stop right away.
 * 2. Add an entry for that IP address in the global collision detection
 * list. When incoming ARP requests, and replies come, the ARP sender IP
 * address will be checked for a match with this entry in that list.
 * . If an ARP reply, or request, or probe (if interface is not configured)
 *   is received, while the IP address is being checked for collision,
 *   then the Treck stack will call the call back function in the context
 *   of the recv task, with TM_EADDRINUSE errorCode.
 * . If the tfUserStartArpSend timeout expires, then the Treck stack will
 *   call the call back function in the context of the timer task, with
 *   the TM_ENOERROR errorCode.
 * . Once started, the collision detection will stop only when the user
 *   calls tfCancelCollisionDetection.
 * . The user can call tfUseCollisionDetection at any time, after
 *   tfStartTreck has been called.
 * 
 * Parameters
 * Parameters Description
 * ipAddress  IP address to check.
 * userCbFunc Call back function. Called by the Treck stack when either an
 *            ARP reply or ARP request has been received whose sender IP
 *            address matches the above IP address. Also called if a matching
 *            ARP probe is received while the interface is not configured.
 *            Also called when the tfUserStartArpSend, or tfStartArpSend
 *            timeout expires.
 * userCbFuncParam
 *            Parameter to be passed as is to the user call back function.
 *
 * Returns
 * Value         Meaning
 * TM_EINVAL     userCbFunc is NULL.
 * TM_EADDRINUSE Another host on the network has been configured with
 *               the IP address. (We found the ARP mapping in the ARP cache.)
 * TM_EALREADY   tfUseCollisionDetection has already been called for
 *               that IP address.
 * TM_ENOBUFS    Not enough memory to allocate a collision detection entry.
 * TM_ENOERROR   No error.
 */
int tfUseCollisionDetection ( 
#ifdef TM_USE_STRONG_ESL
                              ttUserInterface    interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                              ttUserIpAddress    ipAddress,
                              ttArpChkCBFunc     userCbFunc,
                              ttUserGenericUnion userCbParam )
{
    ttArpMapping        arpMapping;
    ttArpChkEntryPtr    listArpChkEntryPtr;
    ttArpChkEntryPtr    arpChkEntryPtr;
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devPtr;
#endif /* TM_USE_STRONG_ESL */
    int                 retCode;
    int                 errorCode;

    if (    (userCbFunc == (ttArpChkCBFunc)0)
#ifdef TM_USE_STRONG_ESL
         || (tfValidInterface(interfaceHandle) != TM_ENOERROR)
#endif /* TM_USE_STRONG_ESL */
       )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
/* Query the ARP table for the IP address parameter */
        errorCode = TM_EADDRINUSE;
        tm_ip_copy(ipAddress, arpMapping.arpmIpAddress);
        arpMapping.arpmCommand = TM_4_ARP_GET_BY_IP;
        arpMapping.arpmPhysAddrLen = (tt8Bit)TM_ETHER_ADDRESS_LENGTH;
#ifdef TM_USE_STRONG_ESL
        devPtr = (ttDeviceEntryPtr)interfaceHandle;
        arpMapping.arpmDevPtr = devPtr; 
#endif /* TM_USE_STRONG_ESL */
        retCode = tfRtArpCommand(&arpMapping);
/* If entry is not in the ARP table (non zero return code) */
        if (retCode != TM_ENOERROR)
        {
            errorCode = TM_ENOERROR;
/* Scan through the list of ARP check entry for a match */
/* LOCK the ARP check list */
            tm_call_lock_wait(&tm_context(tvArpChkListLock));
            listArpChkEntryPtr = tm_context(tvArpChkList);
            while (listArpChkEntryPtr != (ttArpChkEntryPtr)0)
            {
                if (
#ifdef TM_USE_STRONG_ESL
                    (devPtr == listArpChkEntryPtr->arckDevPtr) &&
#endif /* TM_USE_STRONG_ESL */
                    (tm_ip_match(listArpChkEntryPtr->arckIpAddress, ipAddress))
                   )
                {
/* Match. The user is already cheking for that IP address on that interface */
                    errorCode = TM_EALREADY;
                    break;
                }
                listArpChkEntryPtr = listArpChkEntryPtr->arckNextPtr;
            }
            if (errorCode == TM_ENOERROR)
            {
                arpChkEntryPtr = (ttArpChkEntryPtr)tm_get_raw_buffer(
                                                    sizeof(ttArpChkEntry));
                if (arpChkEntryPtr != (ttArpChkEntryPtr)0)
                {
                    tm_bzero(arpChkEntryPtr, sizeof(ttArpChkEntry));
                    arpChkEntryPtr->arckCbFunc = userCbFunc;
                    arpChkEntryPtr->arckCbParam = userCbParam;
#ifdef TM_USE_STRONG_ESL
                    arpChkEntryPtr->arckDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL */
                    tm_ip_copy(ipAddress, arpChkEntryPtr->arckIpAddress);
/* Insert */
                    arpChkEntryPtr->arckNextPtr = tm_context(tvArpChkList);
                    if ( tm_context(tvArpChkList) != (ttArpChkEntryPtr)0 )
                    {
                        tm_context(tvArpChkList)->arckPrevPtr =
                                                              arpChkEntryPtr;
                    }
                    tm_context(tvArpChkList) = arpChkEntryPtr;
                }
                else
                {
/* No memory to allocate a collision detection entry */
                     errorCode = TM_ENOBUFS;
                }
            }
/* UNLOCK the ARP check list */
            tm_call_unlock(&tm_context(tvArpChkListLock));
        }
    }
    return errorCode;
}

/*
 * tfCancelCollisionDetection description.
 * Cancel a collision detection that had been registered with
 * tfUseCollisionDetection. The user will pass the IP address
 * that is being checked for collision. tfCancelCollisionDetection  will
 * search the list of collision detection entries, and remove the
 * matching entry and timer if any. Collision detection for that IP address
 * will stop.
 *
 * Parameters Description
 * ipAddress  IP address that is currently being checked for collision.
 * 
 * Returns
 * Value       Meaning
 * TM_ENOENT   No collision detection entry matching the IP address
 *             parameter has been found.
 * TM_ENOERROR No error.
 */
int tfCancelCollisionDetection (
#ifdef TM_USE_STRONG_ESL
                                 ttUserInterface interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                 ttUserIpAddress ipAddress
                               )
{
    ttArpChkEntryPtr arpChkEntryPtr;
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr devPtr;
#endif /* TM_USE_STRONG_ESL */
    int              errorCode;

#ifdef TM_USE_STRONG_ESL
    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    if (tfValidInterface(interfaceHandle) != TM_ENOERROR)
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_USE_STRONG_ESL */
    {
/* LOCK the ARP check list */
        tm_call_lock_wait(&tm_context(tvArpChkListLock));
        arpChkEntryPtr = tm_context(tvArpChkList);
        errorCode = TM_ENOENT;
        while (arpChkEntryPtr != (ttArpChkEntryPtr)0)
        {
            if (
#ifdef TM_USE_STRONG_ESL
                (devPtr == arpChkEntryPtr->arckDevPtr) && 
#endif /* TM_USE_STRONG_ESL */
                (tm_ip_match(arpChkEntryPtr->arckIpAddress, ipAddress))
               )
            {
/* Match. The user is cheking for that IP address */
                errorCode = TM_ENOERROR;
                break;
            }
            arpChkEntryPtr = arpChkEntryPtr->arckNextPtr;
        }
        if (arpChkEntryPtr != (ttArpChkEntryPtr)0)
        {
            if (tm_8bit_one_bit_set(arpChkEntryPtr->arckFlags,
                                    TM_ARCK_FLAG_ACTIVE))
            {
                tm_8bit_clr_bit( arpChkEntryPtr->arckFlags,
                                 TM_ARCK_FLAG_ACTIVE);
                tm_context(tvArpChkActiveEntries)--;
            }
            if (arpChkEntryPtr->arckTimerPtr != TM_TMR_NULL_PTR)
            {
                tm_timer_remove(arpChkEntryPtr->arckTimerPtr);
                arpChkEntryPtr->arckTimerPtr = TM_TMR_NULL_PTR;
            }
            if (arpChkEntryPtr == tm_context(tvArpChkList))
            {
                tm_context(tvArpChkList) = arpChkEntryPtr->arckNextPtr;
                if (arpChkEntryPtr->arckNextPtr != (ttArpChkEntryPtr)0)
                {
                    arpChkEntryPtr->arckNextPtr->arckPrevPtr =
                                                            (ttArpChkEntryPtr)0;
                }
            }
            else
            {
                arpChkEntryPtr->arckPrevPtr->arckNextPtr =
                                                     arpChkEntryPtr->arckNextPtr;
                if (arpChkEntryPtr->arckNextPtr != (ttArpChkEntryPtr)0)
                {
                    arpChkEntryPtr->arckNextPtr->arckPrevPtr =
                                                    arpChkEntryPtr->arckPrevPtr;
                }
            }
            tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)arpChkEntryPtr);
        }
/* UNLOCK the ARP check list */
        tm_call_unlock(&tm_context(tvArpChkListLock));
    }
    return errorCode;
}

/*
 * tfUserStartArpSend description
 * Send one or more ARP probes, or ARP requests on a given interface.
 * If the interface is not configured yet with that IP address on any
 * multi home, then this command instructs the Treck stack to send one
 * or more ARP probes. An ARP probe is an ARP request with the sender net
 * address set to zero.
 * If the interface has been configured with that IP address on one of
 * its multi-home, then this command instructs the Treck stack to send
 * one or more ARP requests with the configured IP address as the sender
 * address.
 * The user passes the interface handle, the IP address to check
 * for collision, a maximum number of ARP probes/requests, the interval
 * of time between ARP probes/requests, and a timeout parameter.
 *
 * 1. Check that the interface is a valid interface (return TM_EINVAL if not)
 * 2. Lock the device.
 * 3. Check that the interface is opened (return TM_ENXIO if not).
 * 4. Call tfStartArpSend with device locked.
 * 5. Unlock the device.
 * 6. return errorCode to user.
 *
 * Parameters       Description
 * interfaceHandle  Interface on which to send the ARP probe(s)/request(s).
 * ipAddress        Target IP address of the ARP probe(s)/request(s).
 *                  Sender IP address of the ARP request(s).
 * arpProbeInterval Interval in milliseconds between ARP probes/requests. 
 *                  If set to zero, the default TM_PROBE_INTERVAL
 *                  (2000 milliseconds) is used for the probe interval.
 * numberArpProbes  Maximum number of ARP probes/requests to send.
 *                  Interval between ARP requests is arpProbeInterval
 *                  in milliseconds. If set to zero, TM_MAX_PROBE (4)
 *                  is used instead.
 * timeout          number of milliseconds to wait after sending the
 *                  first ARP probe/Arp Request, and call the user call
 *                  back function set by the user with
 *                  tfUseCollisionDetection.
 *
 * Returns
 * Value            Meaning
 * TM_EINVAL        . Invalid interface, or NULL interface
 *                  . timeout less than numberArpProbes * arpProbeInterval
 * TM_ENOENT        The user is not checking the IP address for collision
 *                  detection.
 * TM_EALREADY      tfUserStartArpSend has already been called for
 *                  that IP address, and has not timed out yet.
 * TM_ENOBUFS       No memory to allocate a timer.
 * TM_EPERM         Interface is not a LAN interface, i.e. ARP
 *                  not permitted on that interface.
 * TM_ENXIO         Interface has not been opened.
 * TM_ENOERROR      No error.
 */

int tfUserStartArpSend ( ttUserInterface interfaceHandle,
                         ttUserIpAddress ipAddress,
                         int             numberArpProbes,
                         ttUser32Bit     arpProbeInterval,
                         ttUser32Bit     timeout )
{
    ttDeviceEntryPtr userDevPtr;
    int              errorCode;

    errorCode = TM_EINVAL;
    userDevPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(userDevPtr);
    if ( errorCode == TM_ENOERROR )
    {
/* Match on interface specified by the user, or no interface specified */
/* LOCK the device */
        tm_call_lock_wait(&(userDevPtr->devLockEntry));
        if (tm_4_ll_is_lan(userDevPtr))
        {
            if ( tm_16bit_one_bit_set( userDevPtr->devFlag,
                                       TM_DEV_OPENED) )
            {
                errorCode = tfStartArpSend( userDevPtr,
                                            ipAddress,
                                            numberArpProbes,
                                            arpProbeInterval,
                                            timeout);
            }
            else
            {
/* Interface is not opened */
                errorCode = TM_ENXIO;
            }
        }
        else
        {
/*
 * Interface is not a LAN interface.
 */
            errorCode = TM_EPERM;
        }
/* UNLOCK the device */
        tm_call_unlock(&(userDevPtr->devLockEntry));
    }
    return errorCode;
}

/*
 * tfStartArpSend description
 * Send one or more ARP probes, or ARP requests on a given interface.
 * If the interface is not configured yet with that IP address on any
 * multi home, then this command instructs the Treck stack to send one
 * or more ARP probes. An ARP probe is an ARP request with the sender net
 * address set to zero.
 * If the interface has been configured with that IP address on one of
 * its multi-home, then this command instructs the Treck stack to send
 * one or more ARP requests with the configured IP address as the sender
 * address.
 * The caller passes a pointer to the device entry, the IP address to check
 * for collision, a maximum number of ARP probes/requests, the interval
 * of time between ARP probes/requests, and a timeout parameter.
 * If number ArpProbes is negative, return TM_EINVAL
 * If numberArpProbes is set to zero, use default TM_MAX_PROBE
 * If arpProbeInterval is set to zero, use default TM_PROBE_INTERVAL
 * if timeout is set to zero, use default TM_MAX_PROBE * TM_PROBE_INTERVAL
 * Send an ARP probe/request on the interface(s) specified by the interface
 * handle, create a timer and return. If numberArpProbes is bigger than 1,
 * then the Treck stack, in the context of the timer task, will send
 * (numberArpProbes - 1) additional ARP probes/requests when the
 * arpProbeInterval expires.
 * If a "matching" ARP reply/request/probe is received before all ARP probes
 * have been sent, the Treck stack will stop sending any more ARP
 * probes, and cancel the timer.
 * If a "matching" ARP reply/request is received before all ARP requests
 * have been sent, the Treck stack will stop sending any more ARP
 * requests, and cancel the timer.
 * When the timeout expires, the Treck stack will stop sending any more
 * ARP probe(s) request(s), and will call the user call back function
 * passed by the user in tfUseCollisionDetection, with a
 * TM_ENOERROR errorCode.
 * Interface configuration
 * tfStartArpSend can be called before the interface has been
 * configured with that IP address. In that case, the caller has to have
 * at least opened the interface. This call could be made from within
 * DHCP code to check that no other host is using a given IP address,
 * or AUTO IP to check that no other host is using a picked local IP address.
 * tfStartArpSend can be called, after the interface has been
 * configured with the IP address parameter. In that case ARP requests
 * are being sent, instead of ARP probes.
 *
 * Parameters       Description
 * interfaceHandle  Interface on which to send the ARP probe(s)/request(s).
 * ipAddress        Target IP address of the ARP probe(s)/request(s).
 *                  Sender IP address of the ARP request(s).
 * arpProbeInterval Interval in milliseconds between ARP probes/requests. 
 *                  If set to zero, the default TM_PROBE_INTERVAL
 *                  (2000 milliseconds) is used for the probe interval.
 * numberArpProbes  Maximum number of ARP probes/requests to send.
 *                  Interval between ARP requests is arpProbeInterval
 *                  in milliseconds. If set to zero, TM_MAX_PROBE (4)
 *                  is used instead.
 * timeout          number of milliseconds to wait after sending the
 *                  first ARP probe/Arp Request, and call the user call
 *                  back function set by the user with
 *                  tfUseCollisionDetection.
 *
 * Returns
 * Value            Meaning
 * TM_EINVAL        . Invalid interface, or NULL interface
 *                  . timeout less than numberArpProbes * arpProbeInterval
 * TM_ENOENT        The user is not checking the IP address for collision
 *                  detection.
 * TM_EALREADY      tfUserStartArpSend/tfStartArpSend has already been
 *                  called for that IP address, and has not timed out yet.
 * TM_ENOBUFS       No memory to allocate a timer.
 * TM_ENOERROR      No error.
 */

int tfStartArpSend ( ttDeviceEntryPtr devPtr,
                     ttUserIpAddress  ipAddress,
                     int              numberArpProbes,
                     ttUser32Bit      arpProbeInterval,
                     ttUser32Bit      timeout )
{
    ttArpChkEntryPtr arpChkEntryPtr;
    tt32Bit          residualTime;
    ttGenericUnion   timerParm1;
    int              errorCode;
    tt8Bit           sendArp;

    errorCode = TM_ENOERROR;
    residualTime = TM_UL(0);
    sendArp = TM_8BIT_ZERO;
    if (numberArpProbes == 0)
    {
/* Use default value */
        numberArpProbes = TM_MAX_PROBE;
    }
    if (numberArpProbes <= 0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        if (arpProbeInterval == TM_UL(0))
        {
/* Use default value */
            arpProbeInterval = TM_PROBE_INTERVAL;
        }
        if (timeout == TM_UL(0))
        {
/* Use default value */
            residualTime = arpProbeInterval;
        }
        else
        {
            if (timeout < (((tt32Bit)numberArpProbes) * arpProbeInterval))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                residualTime =    timeout
                                - ((tt32Bit)(numberArpProbes - 1) * arpProbeInterval);
            }
        }
    }
    if (errorCode == TM_ENOERROR)
    {
/* LOCK the ARP check list */
        tm_call_lock_wait(&tm_context(tvArpChkListLock));
        if (tm_context(tvArpChkInCBFunc) == (ttArpChkIn)0)
        {
            tm_context(tvArpChkInCBFunc) = tfArpChkIncoming;
        }
        arpChkEntryPtr = tm_context(tvArpChkList);
        errorCode = TM_ENOENT;
        while (arpChkEntryPtr != (ttArpChkEntryPtr)0)
        {
            if (
#ifdef TM_USE_STRONG_ESL
                (devPtr == arpChkEntryPtr->arckDevPtr) && 
#endif /* TM_USE_STRONG_ESL */
                (tm_ip_match(arpChkEntryPtr->arckIpAddress, ipAddress))
               )
            {
/* Match. The user is checking for that IP address */
                errorCode = TM_ENOERROR;
                break;
            }
            arpChkEntryPtr = arpChkEntryPtr->arckNextPtr;
        }
        if (arpChkEntryPtr != (ttArpChkEntryPtr)0)
        {
            if (    (arpChkEntryPtr->arckDevPtr != (ttDeviceEntryPtr)0)
                 && (arpChkEntryPtr->arckDevPtr != devPtr) )
            {
                errorCode = TM_EALREADY;
            }
            else
            {
                if (arpChkEntryPtr->arckTimerPtr == TM_TMR_NULL_PTR)
                {
                    timerParm1.genVoidParmPtr = (ttVoidPtr) arpChkEntryPtr;
                    arpChkEntryPtr->arckTimerPtr = tfTimerAdd(
                                                    tfArpChkTimeOut,
                                                    timerParm1,
                                                    timerParm1, /* unused */
                                                    arpProbeInterval,
                                                    TM_TIM_AUTO );
                    if (arpChkEntryPtr->arckTimerPtr == (ttTimerPtr)0)
                    {
                       errorCode = TM_ENOBUFS;
                    }
                    else
                    {
                        if (tm_8bit_bits_not_set(arpChkEntryPtr->arckFlags,
                                                 TM_ARCK_FLAG_ACTIVE))
                        {
                            arpChkEntryPtr->arckFlags |= TM_ARCK_FLAG_ACTIVE;
                            tm_context(tvArpChkActiveEntries)++;
                        }
                        arpChkEntryPtr->arckDevPtr = devPtr;
                        arpChkEntryPtr->arckNumberProbes = numberArpProbes;
                        arpChkEntryPtr->arckTimeout = residualTime;
                        sendArp = TM_8BIT_YES;
                    }
                }
                else
                {
                    errorCode = TM_EALREADY;
                }
            }
        }
/* UNLOCK the ARP check list */
        tm_call_unlock(&tm_context(tvArpChkListLock));
        if (sendArp != TM_8BIT_ZERO)
        {
            tfArpChkSend(devPtr, ipAddress);
/*
 * Relock, because device unlocked in tfArpSendRequest called by
 * tfArpChkSend
 */
/* LOCK the device */
            tm_call_lock_wait(&(devPtr->devLockEntry));
        }
    }
    return errorCode;
}

/*
 * tfArpChkTimeOut description
 * Function called from the timer task, after arpProbeInterval has
 * expired.
 * 1. Check that the collision detection entry is still in the list and
 *    if the entry is still in the list, that the timer pointer in the
 *    entry is still non null.
 *   (i.e. the user did not call tfCancelCollisionDetection (race condition))
 * 2. If number ARP probes has already reached zero, we need to notify
 *    the user of the timeout with no collision, by calling the call
 *    back function with a TM_ENOERROR errorCode.
 * 3. Otherwise decrease the number ARP probes, and send an additional
 *    ARP probe/request.
 *
 */
void tfArpChkTimeOut(ttVoidPtr      timerBlockPtr,
                     ttGenericUnion userParm1,
                     ttGenericUnion userParm2)
{
    ttArpChkEntryPtr   arpChkEntryPtr;
    ttArpChkEntryPtr   listArpChkEntryPtr;
    ttArpChkCBFunc     userCBFunc;
    ttUserGenericUnion userCbParam;
    ttDeviceEntryPtr   devPtr;
    tt4IpAddress       ipAddress;
    tt8Bit             timedOut;
    tt8Bit             sendArpProbe;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);
   
/* search the list for one (or more matches) */
    arpChkEntryPtr = (ttArpChkEntryPtr)userParm1.genVoidParmPtr;
    userCBFunc = arpChkEntryPtr->arckCbFunc;
    userCbParam = arpChkEntryPtr->arckCbParam;
    timedOut = TM_8BIT_ZERO;
    sendArpProbe = TM_8BIT_ZERO;
/* LOCK the ARP check list */
    tm_call_lock_wait(&tm_context(tvArpChkListLock));
    devPtr = arpChkEntryPtr->arckDevPtr;
    tm_ip_copy(arpChkEntryPtr->arckIpAddress, ipAddress);
    listArpChkEntryPtr = tm_context(tvArpChkList);
/* Check that the timer parameter pointer is still valid */
    while (listArpChkEntryPtr != (ttArpChkEntryPtr)0)
    {
        if (arpChkEntryPtr == listArpChkEntryPtr)
        {
/* Match. */
            break;
        }
        listArpChkEntryPtr = listArpChkEntryPtr->arckNextPtr;
    }
    if (    (listArpChkEntryPtr != (ttArpChkEntryPtr)0)
         && (arpChkEntryPtr->arckTimerPtr != TM_TMR_NULL_PTR)
         && (devPtr != (ttDeviceEntryPtr)0) )
    {
        if (arpChkEntryPtr->arckNumberProbes != 0)
        {
            arpChkEntryPtr->arckNumberProbes--;
            if (arpChkEntryPtr->arckNumberProbes == 0)
            {
/* Wait for timeout until notifying the user */
                tm_timer_new_time( arpChkEntryPtr->arckTimerPtr,
                                   arpChkEntryPtr->arckTimeout );
            }
            else
            {
/*
 * Send a probe if device is not configured for that IP address, othewise
 * send an ARP request
 */
                sendArpProbe = TM_8BIT_YES;
            }
        }
        else
        {
/* Arp probes/requests timed out with no answer. We need to notify the user */
            tm_timer_remove(arpChkEntryPtr->arckTimerPtr);
            arpChkEntryPtr->arckTimerPtr = (ttTimerPtr)0;
            timedOut = TM_8BIT_YES;
        }
    }
/* UNLOCK the ARP check list */
    tm_call_unlock(&tm_context(tvArpChkListLock));
/* Check on null devPtr not necessary. For PC-LINT benefit */
    if (sendArpProbe && (devPtr != (ttDeviceEntryPtr)0))
    {
/* LOCK the device */
        tm_call_lock_wait(&(devPtr->devLockEntry));
        tfArpChkSend(devPtr, ipAddress);
/* Device unlocked in tfArpSendRequest, called by tfArpChkSend */
    }
    if (timedOut)
    {
/* We timed out. No more probe to send. Call back with no error. */
        (void)(*userCBFunc)( (ttUserInterface)devPtr,
                             ipAddress,
                             TM_ENOERROR,
                             userCbParam );
    }
    return;
}

/*
 * tfArpChkSend function description.
 * Send one ARP probe or request on the given interface, for the given
 * IP address
 * 1. Check if the interface is configured with the IP address, and
 *    find the corresponding multi home index.
 * 2. If it is not configured for the given IP address, set the mhome Index
 *    to an illegal value. That will indicate to the ARP send request
 *    function, that a probe should be sent, instead of an ARP request.
 * 3. Call the link layer send request function.
 */
void tfArpChkSend (ttDeviceEntryPtr devPtr, tt4IpAddress ipAddress)
{
#ifndef TM_SINGLE_INTERFACE_HOME
    int     errorCode;
#endif /* TM_SINGLE_INTERFACE_HOME */
    tt16Bit mhomeIndex;

#ifdef TM_SINGLE_INTERFACE_HOME
    if (    tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr, 0),
                                 TM_DEV_IP_CONFIG)
         && tm_ip_match( ipAddress,
                         tm_ip_dev_addr(devPtr,0)) )
    {
/* Interface is configured with this IP address */
        mhomeIndex = TM_16BIT_ZERO;
    }
    else
    {
/* Interface not configured. Set to invalid mhome index to indicate a probe */
        mhomeIndex = (tt16Bit) TM_MAX_IPS_PER_IF;
    }
#else /* TM_SINGLE_INTERFACE_HOME */
    errorCode = tfMhomeAddrMatch( devPtr, ipAddress, &mhomeIndex );
    if ( errorCode != TM_ENOERROR )
    {
/*
 * No configured mhome on this interface with this IP address. Set to
 * invalid mhome index to indicate a probe.
 */
        mhomeIndex = (tt16Bit) TM_MAX_IPS_PER_IF;
    }
#endif /* TM_SINGLE_INTERFACE_HOME */
/* send a probe, or request */
    (void)(*(devPtr->devLinkLayerProtocolPtr->lnkSendRequestFuncPtr))(
                                                     (ttVoidPtr)devPtr,
                                                     mhomeIndex,
                                                     ipAddress,
                                                     (ttVoidPtr)0 );
}

/*
 * tfArpChkIncoming function description.
 * Called from the ARP incoming logic, when at least one active
 * collision detection has been started by the user, i.e., the user
 * has called tfUseCollisionDetection(), and tfStartArpSend() for the
 * same IP address.
 * Check whether the ARP packet indicates a collision on an IP
 * address checked by the user. If it is, call the user call back function.
 * 1. Check if the ARP request is an ARP probe, and extract the
 *    probed IP address (targetNetAddress). This is the IP address
 *    to check for collision.
 * 2. Otherwise the IP address to check for collision is the senderNetAddress.
 * 3. Check whether the user is checking for a collision on
 *    that IP address. If the user had not registered for a collision
 *    detection on that IP address, this is not a collision. If the user
 *    had registered for a collision detection on that IP address, but
 *    had not called tfUserStartArpSend() or tfStartArpSend(), then we
 *    cannot notify the user either, since we do not have a registered
 *    interface handle to use in the user call back function.
 * 4. Check whether the ARP packet was originated by us on any one of
 *    our interfaces. We check on a match of the sender Ethernet address. If
 *    there is a match with any one of our interfaces ethernet address, this
 *    is not a collision.
 * 5. Also if the ARP packet is a probe, it is not a collision if the
 *    interface checked by the user is already configured for that
 *    IP address, on any of its multihomes.
 * 6. Notify the user, if we found out that there was a collision.
 */
void tfArpChkIncoming(
#ifdef TM_USE_STRONG_ESL
                       ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                       int              operation,
                       tt4IpAddress     senderNetAddress,
                       tt4IpAddress     targetNetAddress,
                       tt8BitPtr        enetAddress )
{
    ttArpChkEntryPtr    listArpChkEntryPtr;
    ttArpChkCBFunc      userCBFunc;
    ttUserGenericUnion  userCbParam;
    ttDeviceEntryPtr    userDevPtr;
    tt4IpAddress        ipAddress;
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit             mhomeIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */
    int                 errorCode;
    tt8Bit              isProbe;
    tt8Bit              match;

    userDevPtr = (ttDeviceEntryPtr)0;
    userCBFunc = (ttArpChkCBFunc)0;
    userCbParam.gen32bitParm = TM_UL(0);
    if (    (operation == TM_ARP_REQUEST)
         && (tm_ip_zero(senderNetAddress)) )
    {
/* Probe */
        tm_ip_copy(targetNetAddress, ipAddress);
        isProbe = TM_8BIT_YES;
    }
    else
    {
/* Request, or reply */
        tm_ip_copy(senderNetAddress, ipAddress);
        isProbe = TM_8BIT_ZERO;
    }

/* LOCK the ARP check list */
    tm_lock_wait(&tm_context(tvArpChkListLock));
/* Check whether we are monitoring for this IP address */
    listArpChkEntryPtr = tm_context(tvArpChkList);
    while (listArpChkEntryPtr != (ttArpChkEntryPtr)0)
    {
        if (
#ifdef TM_USE_STRONG_ESL
            (devPtr == listArpChkEntryPtr->arckDevPtr) &&
#endif /* TM_USE_STRONG_ESL */
            (tm_ip_match(listArpChkEntryPtr->arckIpAddress, ipAddress))
           )
        {
/* Match. The user registered that IP address for collision */
            userCBFunc = listArpChkEntryPtr->arckCbFunc;
            userCbParam = listArpChkEntryPtr->arckCbParam;
            userDevPtr = listArpChkEntryPtr->arckDevPtr;
            break;
        }
        listArpChkEntryPtr = listArpChkEntryPtr->arckNextPtr;
    }
/* UNLOCK the ARP check list */
    tm_unlock(&tm_context(tvArpChkListLock));
    if (    (listArpChkEntryPtr != (ttArpChkEntryPtr)0)
         && (tm_8bit_one_bit_set(listArpChkEntryPtr->arckFlags,
                                 TM_ARCK_FLAG_ACTIVE)) )
/*
 * Can only call back if the user has registered for collision detection,
 * and tried and sent probes on the selected interface, i.e.
 * TM_ARCK_FLAG_ACTIVE is set.
 * If listArpChkEntryPtr is non null, and TM_ARCK_FLAG_ACTIVE is set,
 * userDevPtr is non null.
 */
    {
/*
 * Only notify user of ARP packets coming from other hosts, i.e. ignore 
 * ARP packets sent by us.
 * Only notify user of probes if our interface has not been configured yet
 * for that IP address, i.e. ignore probes for an IP address that we have
 * been configured with.
 */
        match = tfEtherMatchOurs(userDevPtr, enetAddress);
        if (match)
        {
/* We originated the ARP request/probe. Not a collision. */
            errorCode = TM_ENOERROR;
        }
        else
        {
/*
 * We did not originate the ARP packet.
 * If a probe, check to see if we are already configued for that
 * IP address. In that case, we should ignore the probe.
 */
            errorCode = TM_ENOENT;
            if (isProbe)
            {
#ifdef TM_SINGLE_INTERFACE_HOME
                if (    tm_8bit_one_bit_set( tm_ip_dev_conf_flag(userDevPtr, 0),
                                             TM_DEV_IP_CONFIG)
                     && tm_ip_match( ipAddress,
                                     tm_ip_dev_addr(userDevPtr,0)) )
                {
/* Ignore Probe if we are configured for that IP address */
                    errorCode = TM_ENOERROR;
                }
#else /* TM_SINGLE_INTERFACE_HOME */
/*
 * If we are configured for that IP address on any one of the mhome, this
 * function will return TM_ENOERROR, making us ignore the probe below.
 * Otherwise this function returns TM_ENOENT.
 */
                errorCode = tfMhomeAddrMatch(userDevPtr, ipAddress,
                                             &mhomeIndex);
#endif /* TM_SINGLE_INTERFACE_HOME */
            }
        }
        if (errorCode != TM_ENOERROR)
        {
/* LOCK the ARP check list */
            tm_lock_wait(&tm_context(tvArpChkListLock));
/* find our matching ARP check entry again, so we can cancel the timer */
            listArpChkEntryPtr = tm_context(tvArpChkList);
            while (listArpChkEntryPtr != (ttArpChkEntryPtr)0)
            {
                if (tm_ip_match(listArpChkEntryPtr->arckIpAddress, ipAddress))
                {
/* Match. The user registered that IP address for collision */
/* if there is an ARP check timer, then cancel it */
                    if (listArpChkEntryPtr->arckTimerPtr != TM_TMR_NULL_PTR)
                    {
                        tm_timer_remove(listArpChkEntryPtr->arckTimerPtr);
                        listArpChkEntryPtr->arckTimerPtr = TM_TMR_NULL_PTR;
                    }

                    break;
                }
                listArpChkEntryPtr = listArpChkEntryPtr->arckNextPtr;
            }
/* UNLOCK the ARP check list */
            tm_unlock(&tm_context(tvArpChkListLock));

/* Call back with address in use message */
/* This check is not necessary. For PC-LINT benefit only */
            if (userCBFunc != (ttArpChkCBFunc)0)
            {
                (void)(*userCBFunc)( (ttUserInterface)userDevPtr,
                                     ipAddress,
                                     TM_EADDRINUSE,
                                     userCbParam );
            }
        }
    }
}

#else /* !TM_USE_IPV4 */
#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */
int tvArpChkDummy = 0;
#endif /* !TM_USE_IPV4 */
