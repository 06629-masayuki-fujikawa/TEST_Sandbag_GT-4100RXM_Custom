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
 * Description: Mobile IP (node) stub functions
 *
 * Filename: stubs/trmoblip.c
 * Author: Jason
 * Date Created: 4/27/01
 * $Source: source/stubs/ipv4/trmoblip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:51JST $
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


int tfMobileIpReRegister(ttUserInterface interfaceHandle,
                         int             flags)
{
    interfaceHandle=interfaceHandle;
    flags=flags;
    return TM_EPROTONOSUPPORT;
}


int tfMobileIpMoveDetect(ttUserInterface interfaceHandle)
{
    interfaceHandle=interfaceHandle;
    return TM_EPROTONOSUPPORT;
}

     
int tfMobileIpMoveNotify(ttUserInterface interfaceHandle,
                         int             flags)
{
    interfaceHandle=interfaceHandle;
    flags=flags;
    return TM_EPROTONOSUPPORT;
}

int tfUseMobileIp(ttUserInterface           interfaceHandle,
                  ttUserMobileNotifyFuncPtr notifyFuncPtr,
                  ttUser32Bit               flags)
{
    interfaceHandle=interfaceHandle;
    notifyFuncPtr=notifyFuncPtr;
    flags=flags;
    return TM_EPROTONOSUPPORT;
}
















  
