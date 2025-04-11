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
 * Description: Treck strcspn "C" library function
 *
 * Filename: trstrcsp.c
 * Author: Odile
 * Date Created: 02/19/99
 * $Source: source/sockapi/trstrcsp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:43JST $
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
 * Return the length of the initial segment of bufferString that consists
 * entirely of characters not included in string.
 */
unsigned int tfStrCSpn( const char TM_FAR * bufferStringPtr,
                        const char TM_FAR * stringPtr )
{
    unsigned int stringLength;
    unsigned int retCode;
    char TM_FAR * temp8Ptr;

/* tfStrLen checks for null pointer */
    stringLength = (unsigned int) tm_strlen(bufferStringPtr);
    for (retCode = 0; retCode < stringLength; retCode++)
    {
        temp8Ptr = tfStrChr(stringPtr, bufferStringPtr[retCode]);
        if (temp8Ptr != (char TM_FAR *)0)
        {
            break;
        }
    }

    return retCode;
}
