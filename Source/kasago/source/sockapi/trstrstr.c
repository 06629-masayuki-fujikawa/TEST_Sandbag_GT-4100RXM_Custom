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
 * Description: Treck strstr "C" library function
 *
 * Filename: trstrstr.c
 * Author: Odile
 * Date Created: 02/18/99
 * $Source: source/sockapi/trstrstr.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:49JST $
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
 * Find a match of string inside bufferstring.
 */
char TM_FAR * tfStrStr( const char TM_FAR * bufferStringPtr,
                        const char TM_FAR * stringPtr )
{
    char TM_FAR * retStringPtr;
    int           stringLength;
    int           bufferStringLength;
    int           retCode;
    int           i;

    retStringPtr = (char TM_FAR *)0;
/* tfStrLen checks for null pointer */
    stringLength = (int)tm_strlen(stringPtr);
    bufferStringLength = (int)tm_strlen(bufferStringPtr);
    if (stringLength > 0)
    {
        i = 0;
        while ( bufferStringLength >= stringLength )
        {
            retCode = tm_memcmp( &bufferStringPtr[i],
                                 stringPtr,
                                 stringLength );
            if (retCode == TM_STRCMP_EQUAL)
            {
                retStringPtr = (char TM_FAR *)&bufferStringPtr[i];
                break;
            }
            i++;
            bufferStringLength--;
        }
    }
    return retStringPtr;
}
