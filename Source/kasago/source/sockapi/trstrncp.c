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
 * Description: Treck strncpy "C" library function
 *
 * Filename: trstrncp.c
 * Author: Odile
 * Date Created: 02/19/99
 * $Source: source/sockapi/trstrncp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:47JST $
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

/* 
 * Copy stringSrc to stringDest up to nBytes. If nBytes bigger than
 * string length of stringSrc, append 0 up to nbytes.
 * Return pointer to Destination string.
 */
char TM_FAR * tfStrNCpy (       char TM_FAR * stringDestPtr,
                          const char TM_FAR * stringSrcPtr,
                                unsigned int  nBytes )
{
    unsigned int strLength;

    if ( (stringDestPtr != (char TM_FAR *)0) && (nBytes != 0) )
    {   
/* tfStrLen checks null pointer */
        strLength = (unsigned int) tm_strlen(stringSrcPtr);

        if (strLength > nBytes)
        {
            strLength = nBytes;
        }
        if (strLength != 0)
        {
            tm_bcopy(stringSrcPtr, stringDestPtr, strLength);
            if (strLength < nBytes)
            {
                tm_bzero(&stringDestPtr[strLength], nBytes - strLength);
            }
        }
    }

    return stringDestPtr;
}
