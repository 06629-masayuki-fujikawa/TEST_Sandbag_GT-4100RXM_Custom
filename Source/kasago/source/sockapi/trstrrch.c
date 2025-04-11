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
 * Description: Treck strrchr "C" library function
 *
 * Filename: trstrrch.c
 * Author: Odile
 * Date Created: 02/19/99
 * $Source: source/sockapi/trstrrch.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:48JST $
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
 * Return pointer to last instance of character inside string. If no match
 * return NULL.
 */
char TM_FAR * tfStrRChr(const char TM_FAR * stringPtr, int character)
{
    char     TM_FAR * retStringPtr;
    int               i;

    retStringPtr = (char TM_FAR *)0;
    i = (int)(tm_strlen(stringPtr) - 1);
    while (i >= 0)
    {
        if (stringPtr[i] == (char)character)
        {
            retStringPtr = (char TM_FAR *)&stringPtr[i];
            break;
        }
        i--;
    }
    return retStringPtr;
}
