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
 * Description: if_nametoindex sock API
 *
 * Filename: trifnmid.c
 * Author: Bryan Wang
 * Date Created:
 * $Source: source/sockapi/trifnmid.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:32JST $
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
* FUNCTION: if_nametoindex
*
* PURPOSE:  maps an interface name into its corresponding index
*
* PARAMETERS:
*   ifindex:
*       interface index
*   ifname
*       point to a buffer of at least IF_NAMESIZE bytes into which the 
*       interface name corresponding to the specified index is returned.
*       This pointer is also the return value of the function
* RETURNS:
*   index of the interface, if successful
*   0, if fails, errno is set
* ERRNO:
*   TM_EXIO
*       there is no interface corresponding to the specified name    
*   TM_ENOMEM
*       running out of memory
*   TM_EINVAL
*       ifname was a NULL pointer
* NOTES:
*
****************************************************************************/
unsigned int if_nametoindex(const char TM_FAR * ifname)
{
    int              errorCode;
    int              ifIndex;
    int              i;

    ifIndex   = 0;
    errorCode =  TM_ENOERROR;

    if (ifname == (const char TM_FAR *)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        for (i= 1; i <= tm_context(tvAddedInterfaces); i++)
        {
            errorCode = tm_strcmp( 
                            ifname,
                            tm_context(tvDevIndexToDevPtr[i])->devNameArray);
            if( errorCode == TM_STRCMP_EQUAL )
            {
                errorCode = TM_ENOERROR;
                ifIndex = i;
                break;
            }
        }
        if (ifIndex == 0)
        {
            errorCode = TM_ENXIO;
        }
    }
    tm_context(tvErrorCode) = errorCode;
	return (unsigned int)ifIndex;
}
/***************** End Of File *****************/
