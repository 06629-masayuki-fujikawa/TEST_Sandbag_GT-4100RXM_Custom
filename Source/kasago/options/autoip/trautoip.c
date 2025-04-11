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
 * Filename: trautoip.c
 * Author: Odile
 * Date Created: 02/27/01
 * $Source: source/trautoip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:04JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h> /* For TM_USE_AUTO_IP */
#include <trmacro.h> /* For LINT_UNUSED_HEADER */

#if (defined(TM_USE_AUTO_IP) && defined(TM_USE_IPV4))

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * tfAutoIpPickIpAddress function description
 * Picks a random IP address in the AUTO IP specified range.
 * Parameters
 * Return value
 * non zero IP address success
 * zero                failure
 */
ttUserIpAddress tfAutoIPPickIpAddress (void)
{
    tt32Bit randomNumber;
    tt16Bit random16;
    tt32Bit newIpAddress;
    tt16Bit host16;
    int     count;

    randomNumber = tfGetRandom();
    random16 = (tt16Bit)(randomNumber & 0xFFFF);
    if (random16 == (tt16Bit)0)
    {
        random16 = (tt16Bit)TM_IP_LOCAL_SMALLEST_HOST;
    }
    tm_ip_copy(tm_context(tvAutoIpAddress), newIpAddress);
/* Take lower 16 bits of the address in host byte order */
    host16 = (tt16Bit)(ntohl(newIpAddress) & 0xFFFF);
    count = 0;
    do
    {
        host16 = (tt16Bit)(host16 + random16);
        if (    (host16 <= TM_IP_LOCAL_BIGGEST_HOST)
             && (host16 >= TM_IP_LOCAL_SMALLEST_HOST) )
        {
            newIpAddress = ((tt32Bit)(host16) & TM_UL(0xFFFF));
            newIpAddress = htonl(newIpAddress);
/* Add local network to the address */
            tm_ip_copy( tm_ip_or(newIpAddress, TM_IP_LOCAL_NET),
                        newIpAddress );
        }
        random16 = (tt16Bit) (random16 + (tt16Bit)TM_IP_LOCAL_SMALLEST_HOST);
        count++;
        if (count > 1000)
        {
            tm_ip_copy(TM_IP_ZERO, newIpAddress);
            break;
        }
    }
    while ( tm_ip_match(newIpAddress, tm_context(tvAutoIpAddress)) );
    if (!tm_ip_zero(newIpAddress))
    {
        tm_ip_copy(newIpAddress, tm_context(tvAutoIpAddress));
    }
    return newIpAddress;
}

#else /* ! (defined(TM_USE_AUTO_IP) && defined(TM_USE_IPV4)) */
#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */
int tvAutoIpsDummy = 0; /* Auto IP source code dummy variable for linker */
#endif /* ! (defined(TM_USE_AUTO_IP) && defined(TM_USE_IPV4)) */
