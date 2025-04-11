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
 * Description: Treck "C" strchr library function
 *
 * Filename: trstrchr.c
 * Author: Odile
 * Date Created: 02/19/99
 * $Source: source/sockapi/trstrchr.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/07/31 03:53:12JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h>

/* 
 * Return pointer to first instance of character inside string. If no match
 * return NULL.
 */
char TM_FAR * tfStrChr(const char TM_FAR * stringPtr, int character)
{
    char     TM_FAR * retStringPtr;

    retStringPtr = (char TM_FAR *)0;

    if (stringPtr != (const char TM_FAR *)0)
    {
        while (*stringPtr != '\0')
        {
            if (*stringPtr == (char)character)
            {
                retStringPtr = (char TM_FAR *)stringPtr;
                break;
            }
            stringPtr++;
        }
        if ((char)character == '\0')
        {
            retStringPtr = (char TM_FAR *)stringPtr;
        }
    }

    return retStringPtr;
}
