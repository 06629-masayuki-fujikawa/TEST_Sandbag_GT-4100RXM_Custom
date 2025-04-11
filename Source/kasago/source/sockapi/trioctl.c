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
 * Description: BSD Sockets Interface (tfIoctl)
 *
 * Filename: trioctl.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trioctl.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:37JST $
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
#if defined(TM_USE_BSD_DOMAIN) && defined(TM_ERROR_CHECKING)
#include <trglobal.h>
#endif /* defined(TM_USE_BSD_DOMAIN) && defined(TM_ERROR_CHECKING) */

typedef struct ip_msfilter TM_FAR * ttIpmsfilterPtr;
/*
 * BSD ioctl. Valid 'request' values are:
 *             FIONBIO (can also be done with tfBlockingState()),
 *             FIONREAD (can also be done with tfGetWaitingBytes()), and
 *             SIOCATMARK (can also be done with tfGetOobDataOffset()).
 * tfIoctl will return an error for any other request.
 */
int tfIoctl(int socketDescriptor, ttUser32Bit request, int TM_FAR * dataArg)
{
    ttSocketEntryPtr            socketEntryPtr;
#ifdef TM_USE_IGMPV3
    ttIpmsfilterPtr             ipmsfilterPtr;
#endif /* TM_USE_IGMPV3 */
    int                         errorCode;
    int                         retCode;
#ifdef TM_USE_BSD_DOMAIN
    int                         af;
#endif /* TM_USE_BSD_DOMAIN */
    
/* PARAMETER CHECK */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(tfIoctl, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdIoctl( (ttVoidPtr)socketEntryPtr, request, 
                              (ttCharPtr)dataArg );
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
        {
            errorCode = TM_ENOERROR;

            if (dataArg == (int TM_FAR *)0)
            {
                errorCode = TM_EINVAL;
                goto ioctlExit;
            }
            switch (request)
            {
                case FIONBIO:
                    if (*dataArg == 0)
                    {
/* clear non blocking IO */
                        tm_16bit_clr_bit( socketEntryPtr->socFlags,
                                      TM_SOCF_NONBLOCKING );
                    }
                    else
                    {
/* set non blocking IO */
                        socketEntryPtr->socFlags |= TM_SOCF_NONBLOCKING;
                    }
                    break;
                case FIONREAD:
                    *dataArg = (int)(socketEntryPtr->socRecvQueueBytes);
                    break;
                case SIOCATMARK:
                    if (tm_16bit_one_bit_set(socketEntryPtr->socFlags,
                                             TM_SOCF_RCVATMARK))
                    {
/* At out of band mark */
                        *dataArg = 1;
                    }
                    else
                    {
/* Not at out of band mark */
                        *dataArg = 0;
                    }
                    break;
#if defined(TM_USE_IPV4) && defined(TM_IGMP) && defined(TM_USE_IGMPV3)
/*
 * To retrieve the list of source addresses
 * that comprise the source filter along with the current filter
 * mode for a given socket, and multicast group address.
 */
                case SIOCGIPMSFILTER:
                    ipmsfilterPtr = (ttIpmsfilterPtr)(ttVoidPtr)dataArg; 
                    errorCode = tfComGetipv4sourcefilter(
                                                socketEntryPtr,
                                                ipmsfilterPtr->imsf_interface,
                                                ipmsfilterPtr->imsf_multiaddr,
                                                &ipmsfilterPtr->imsf_fmode,
                                                &ipmsfilterPtr->imsf_numsrc,
                                                &ipmsfilterPtr->imsf_slist[0]);
                    break;
/*
 * To set or modify the source filter
 * content (e.g., unicast source address list) or mode (exclude or
 * include) for a given socket, and multicast group address.
 */
                case SIOCSIPMSFILTER:
                    ipmsfilterPtr = (ttIpmsfilterPtr)(ttVoidPtr)dataArg; 
                    errorCode = tfComSetipv4sourcefilter(
                                                socketEntryPtr,
                                                ipmsfilterPtr->imsf_interface, 
                                                ipmsfilterPtr->imsf_multiaddr,
                                                ipmsfilterPtr->imsf_fmode,
                                                ipmsfilterPtr->imsf_numsrc,
                                                &ipmsfilterPtr->imsf_slist[0]);
                    break;
#endif /* TM_USE_IPV4 && TM_IGMP && TM_USE_IGMPV3 */
                default:
                    errorCode = TM_EINVAL;
                    break;
            }
        }
        else
        {
            errorCode = TM_EBADF;
        }
ioctlExit:
        retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, TM_ENOERROR);
    }
    return retCode;
}

