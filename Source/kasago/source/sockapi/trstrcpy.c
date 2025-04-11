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
 * Description: Treck strcpy "C" library function
 *
 * Filename: trstrcpy.c
 * Author: Odile
 * Date Created: 2/10/99
 * $Source: source/sockapi/trstrcpy.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:42JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

/* 
 * Copy from one string to another
 * Does not need to be fast since it is so rarely called
 */

void tfStrCpy(char TM_FAR * destination, const char TM_FAR * source)
{
    if (    (destination != (char TM_FAR *)0)
         && (source != (const char TM_FAR *)0) )
    {
        while( *source != (char)0 )
        {
            *destination++ = *source++;
        }
        *destination = *source; /* '\0' */
    }
}

/*
 * Duplicate a string
 */

char * tfStrDup(const char TM_FAR * source)
{
    char * newString;

    newString = (char *)0;
    if (source != (const char TM_FAR *)0)
    {
        newString = (char *)tm_malloc(sizeof(char) * (tm_strlen(source)+1));
        if (newString != (char *)0)
        {
            tm_strcpy(newString, source);
        }
    }

    return newString;
}
