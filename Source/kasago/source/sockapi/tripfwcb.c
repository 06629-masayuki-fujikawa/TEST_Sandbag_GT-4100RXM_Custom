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
 * Description:     tfRegisterIpForwCB
 *
 * Filename:        tripfwcb.c
 * Author:          Odile
 * Date Created:    07/19/00
 * $Source: source/sockapi/tripfwcb.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:38JST $
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
#ifdef TM_LOCK_NEEDED
#include <trproto.h>
#endif /* TM_LOCK_NEEDED */
#include <trglobal.h>


#ifdef TM_USE_IPV4
/* 
 * tfRegisterIpForwCB function desription
 * Used to register a function for the Treck stack to call when a packet
 * cannot be forwarded. The call back function parameters will indicate
 * the source IP address, and destination IP address of the packet in
 * network byte order. This is useful to let the user know
 * that a packet can not be forwarded because a dial up interface is
 * closed for example. In that case the user call back function could
 * trigger a dial-up on demand to allow forwarding of subsequent packets.
 * If the user does not want to enable or configure the interface, then
 * the call back function should return a non zero error code. In that case
 * the stack will send a host unreachable ICMP error message as if
 * no call back function had been registered.
 * If the user wants to enable or configure the interface, then the
 * call back function should return TM_ENOERROR. In that case the Treck stack
 * will silently drop the packet without sending an ICMP error message,
 * allowing the sender to try and send more data.
 * Parameters
 * ipForwCBFuncPtr  Pointer to user call back function that returns
 *                  an integer as described above, and that takes
 *                  two parameters of type ttUserIpAddress, the
 *                  first one being the source IP address in network
 *                  byte order of the IP datagram to be forwarded,
 *                  and the second one being its destination IP
 *                  address in network byte order.
 * Return
 * TM_ENOERROR
 */
int tfRegisterIpForwCB( ttUserIpForwCBFuncPtr ipForwCBFuncPtr )
{
/*
 * lock the IP forward cache lock, so that the IP forwarding code
 * cannot access the global IP forward call back function, while the
 * user is setting/resetting its value.
 */
    tm_call_lock_wait(&tm_context(tvRtIpForwCacheLock));
    tm_context(tvIpForwCBFuncPtr) = ipForwCBFuncPtr;
    tm_call_unlock(&tm_context(tvRtIpForwCacheLock));
    return TM_ENOERROR;
}
#endif /* TM_USE_IPV4 */
