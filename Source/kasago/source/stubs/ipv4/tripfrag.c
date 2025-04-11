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
 * Description: stubs for IP fragmentation function
 * Filename: tripfrag.c
 * Author: Odile
 * Date Created: 12/05/97
 * $Source: source/stubs/ipv4/tripfrag.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:49JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>  /* For TM_IP_FRAGMENT */
#include <trmacro.h> /* For LINT_UNUSED_HEADER */

/*
 * TM_IP_FRAGMENT defined in trsystem.h if fragmentation and reassembly are
 * needed.
 */

#if (defined(TM_IP_FRAGMENT) && defined(TM_USE_IPV4))

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * Local functions
 */

ttPacketPtr tfIpReassemblePacket(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);

    return (ttPacketPtr) 0;
}

int tfIpFragmentPacket (ttPacketPtr origPacketPtr, ttPktLen devMtu)
{
    TM_UNREF_IN_ARG(origPacketPtr);
    TM_UNREF_IN_ARG(devMtu);

    return TM_EPROTONOSUPPORT;
}

#else /* ! (defined(TM_IP_FRAGMENT) && defined(TM_USE_IPV4)) */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_IP_FRAGMENT is not defined */
int tv4IpfragDummy = 0;

#endif /* ! (defined(TM_IP_FRAGMENT) && defined(TM_USE_IPV4)) */
