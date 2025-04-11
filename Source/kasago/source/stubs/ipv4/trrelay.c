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
 * Description: BOOTP relay agent stubs functions
 *
 * Filename:     stubs/trrelay.c
 * Author:       Jason
 * Date Created: 07/13/98
 * $Source: source/stubs/ipv4/trrelay.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:53JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


int tfStartBootRelayAgent( ttUserIpAddress ipAddress,
                           ttUserInterface interfaceHandle,
                           unsigned char   multiHomeIndex)
{
    TM_UNREF_IN_ARG(ipAddress);
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(multiHomeIndex);
    return TM_EPROTONOSUPPORT;
}

int tfStopBootRelayAgent (void)
{
    return TM_EPROTONOSUPPORT;
}
