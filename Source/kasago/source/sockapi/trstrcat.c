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
 * Description: Treck "C" strcat library function
 *
 * Filename: trstrcat.c
 * Author: Odile
 * Date Created: 02/18/99
 * $Source: source/sockapi/trstrcat.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:39JST $
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
 * Append stringSrcPtr to stringDestPtr. Return stringDestPtr.
 */
char TM_FAR * tfStrCat(       char TM_FAR * stringDestPtr,
                        const char TM_FAR * stringSrcPtr )
{
    unsigned int stringLength;

    if (stringDestPtr != (char TM_FAR *)0)
    {
        stringLength = (unsigned int) tm_strlen(stringDestPtr);
        tm_strcpy(&stringDestPtr[stringLength], stringSrcPtr);
    }
    return stringDestPtr;
}
