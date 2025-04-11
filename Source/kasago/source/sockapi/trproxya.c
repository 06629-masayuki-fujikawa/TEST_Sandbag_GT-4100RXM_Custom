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
 * Description: Proxy ARP table API (tfAddProxyArpEntry, tfDelProxyArpEntry)
 *
 * Filename: trproxya.c
 * Author: Odile
 * Date Created: 01/13/99
 * $Source: source/sockapi/trproxya.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:55JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * tfAddProxyArpEntry function descrpition:
 * Add an entry to the Proxy ARP table for the given IP address.
 *
 * Parameters
 * Parameter    Description
 * arpIpAddrss  Ip address on behalf of which the system will reply to
 *              ARP requests.
 * Returns
 * Value        Meaning
 * TM_ENOERROR  success
 * TM_EINVAL    bad parameter (0 IP address parameter)
 * TM_EALREADY  entry already in PROXY ARP table.
 */
int tfAddProxyArpEntry (ttUserIpAddress arpIpAddress)
{
    ttProxyEntryPtr proxyEntryPtr;
    ttProxyEntryPtr oldProxyEntryPtr;
    int             errorCode;

    if ( tm_ip_zero(arpIpAddress) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_call_lock_wait(&tm_context(tvProxyLockEntry));
        oldProxyEntryPtr = tfArpFindProxyEntry((tt4IpAddress)arpIpAddress);
        if (oldProxyEntryPtr == (ttProxyEntryPtr)0)
        {
            proxyEntryPtr = (ttProxyEntryPtr)tm_get_raw_buffer(
                                         (unsigned)sizeof(ttProxyEntry));
            if (proxyEntryPtr != (ttProxyEntryPtr)0)
            {
                tm_ip_copy( (tt4IpAddress)arpIpAddress,
                            proxyEntryPtr->proxIpAddress );
                proxyEntryPtr->proxNextPtr = tm_context(tvProxyHeadPtr);
                proxyEntryPtr->proxPrevPtr = (ttProxyEntryPtr)0;
                if (tm_context(tvProxyHeadPtr) != (ttProxyEntryPtr)0)
                {
                    tm_context(tvProxyHeadPtr)->proxPrevPtr = proxyEntryPtr;
                }
                tm_context(tvProxyHeadPtr) = proxyEntryPtr;
                errorCode = TM_ENOERROR;
            }
            else
            {
                errorCode = TM_ENOBUFS;
            }
        }
        else
        {
            errorCode = TM_EALREADY;
        }
        tm_call_unlock(&tm_context(tvProxyLockEntry));
    }
    return errorCode;
}

/*
 * tfDelProxyArpEntry function descrpition:
 * Delete an entry from the Proxy ARP table for the given IP address.
 *
 * Parameters
 * Parameter    Description
 * arpIpAddrss  Ip address on behalf of which the system will stop replying to
 *              ARP requests.
 * Returns
 * Value        Meaning
 * TM_ENOERROR  success
 * TM_EINVAL    bad parameter (0 IP address parameter)
 * TM_ENOENT    entry was not in PROXY ARP table.
 */
int tfDelProxyArpEntry (ttUserIpAddress arpIpAddress)
{
    ttProxyEntryPtr proxyEntryPtr;
    ttProxyEntryPtr prevProxyEntryPtr;
    ttProxyEntryPtr nextProxyEntryPtr;
    int             errorCode;

    if ( tm_ip_zero(arpIpAddress) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_call_lock_wait(&tm_context(tvProxyLockEntry));
        proxyEntryPtr = tfArpFindProxyEntry((tt4IpAddress)arpIpAddress);
        if (proxyEntryPtr != (ttProxyEntryPtr)0)
        {
            prevProxyEntryPtr = proxyEntryPtr->proxPrevPtr;
            nextProxyEntryPtr = proxyEntryPtr->proxNextPtr;
            if (prevProxyEntryPtr != (ttProxyEntryPtr)0)
            {
                prevProxyEntryPtr->proxNextPtr = nextProxyEntryPtr;
            }
            else
            {
/* At head */
                tm_context(tvProxyHeadPtr) = nextProxyEntryPtr;
            }
            if (nextProxyEntryPtr != (ttProxyEntryPtr)0)
            {
                nextProxyEntryPtr->proxPrevPtr = prevProxyEntryPtr;
            }
/* Put it back into the recycle list */
            tm_free_raw_buffer(proxyEntryPtr);
            errorCode = TM_ENOERROR;
        }
        else
        {
            errorCode = TM_ENOENT;
        }
        tm_call_unlock(&tm_context(tvProxyLockEntry));
    }
    return errorCode;
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4ProxyaDummy = 0;
#endif /* ! TM_USE_IPV4 */
