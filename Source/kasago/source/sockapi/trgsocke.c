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
 * Description: BSD Sockets Interface (tfGetSocketError)
 *
 * Filename: trgsocke.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgsocke.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:24JST $
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
#include <trglobal.h>


/*
 * Treck extended socket calls
 */

int tfGetSocketError(int socketDescriptor)
{
    int errorCode;

    if ( (tt32Bit)socketDescriptor >= tm_context(tvMaxNumberSockets) )
    {
        errorCode = tm_context(tvErrorCode);
    }
    else
    {
        errorCode = (int)tm_context(tvSocketErrorArray)[socketDescriptor];
    }
    return(errorCode);
}

