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
 * Description: BSD Sockets Interface (tfGetPppDnsIpAddress)
 * Get the DNS Address as given to us by the remote
 *
 * Filename: trgpppdn.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgpppdn.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:21JST $
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
#ifdef TM_LOCK_NEEDED
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED */


/* 
 * Get the IP address of a PPP DNS server (primary, or secondary), as given
 * to us by the peer PPP.
 */
int tfGetPppDnsIpAddress (ttUserInterface         interfaceHandle,
                          ttUserIpAddress TM_FAR *dnsIpAddressPtr,
                          int                     flag)
{
    ttDeviceEntryPtr devEntryPtr;
    ttPppVectPtr     pppVectPtr;
    int              errorCode;
    
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidConfigInterface( devEntryPtr,
                                        TM_16BIT_ZERO );
    if (errorCode == TM_ENOERROR)
    {
        errorCode = TM_EINVAL; /* assume failure */
        if (    (dnsIpAddressPtr != (ttUserIpAddress TM_FAR *)0)
             && (flag == TM_DNS_PRIMARY || flag == TM_DNS_SECONDARY) )
        {
/* Zero the outgoing address in the event we have an error */
            tm_ip_copy(TM_IP_ZERO, *dnsIpAddressPtr);
            if (tm_ll_uses_ppp(devEntryPtr))
            {
                tm_call_lock_wait(&(devEntryPtr->devLockEntry));
                pppVectPtr = (ttPppVectPtr)devEntryPtr->devStateVectorPtr;
                if ( pppVectPtr != (ttPppVectPtr)0 )
                {
/* CHeck to see which DNS server the caller wants */
                    if (flag == TM_DNS_PRIMARY)
                    {
/* Copy the Primary DNS IP from the PPP State Vector */
                        tm_ip_copy( pppVectPtr->pppsIpcpState.
                                       ipcpsLocalNegOptions.ipcpDnsPriAddress,
                                    *dnsIpAddressPtr );
                    }
                    else
                    {
/* Copy the Secondary DNS IP from the PPP State Vector */
                        tm_ip_copy(  pppVectPtr->pppsIpcpState.
                                      ipcpsLocalNegOptions.ipcpDnsSecAddress,
                                    *dnsIpAddressPtr );
                    }
                    errorCode = TM_ENOERROR; /* copied address */
                }
                tm_call_unlock(&(devEntryPtr->devLockEntry));
            }
        }
    }
    return errorCode;
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4GpppdnDummy = 0;
#endif /* ! TM_USE_IPV4 */
