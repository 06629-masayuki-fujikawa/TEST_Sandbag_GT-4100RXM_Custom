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
 * Description: if_indextoname socket API
 *
 * Filename: trifidnm.c
 * Author: Bryan Wang
 * Date Created:
 * $Source: source/sockapi/trifidnm.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:31JST $
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

/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */

/* local variable definitions */

/* static const tables */

/* macro definitions */

/****************************************************************************
* FUNCTION: if_indextoname
*
* PURPOSE:  maps an interface index into its corresponding name
*
* PARAMETERS:
*   ifindex:
*       interface index
*   ifname
*       point to a buffer of at least IF_NAMESIZE bytes into which the 
*       interface name corresponding to the specified index is returned.
*       This pointer is also the return value of the function
* RETURNS:
*   Pointer to the name string, same as ifname if sucess,
*   NULL error if failed
* ERRNO:
*   TM_EXIO
*       there is no interface corresponding to the specified index    
*   TM_ENOMEM
*       running out of memory
*   TM_EINVAL
*       ifname was a NULL pointer
* NOTES:
*
****************************************************************************/
char TM_FAR * if_indextoname(unsigned int ifindex, char TM_FAR * ifname)
{
    ttDeviceEntryPtr devPtr;
    int              errorCode;

    errorCode =  TM_ENOERROR;
    devPtr = (ttDeviceEntryPtr) 0;

    if (ifname == (char*)0)
    {
        errorCode = TM_EINVAL;
    }
    else if ((ifindex > 0)
             && (ifindex <= (unsigned int)(tm_context (tvAddedInterfaces))))
    {
        devPtr = tm_context(tvDevIndexToDevPtr[ifindex]);
        tm_memcpy(ifname, devPtr->devNameArray, IF_NAMESIZE);
    }
    else
    {
        ifname = (char*)0;
        errorCode = TM_ENXIO;
    }
    tm_context(tvErrorCode) = errorCode;
    return ifname;
}
/***************** End Of File *****************/
