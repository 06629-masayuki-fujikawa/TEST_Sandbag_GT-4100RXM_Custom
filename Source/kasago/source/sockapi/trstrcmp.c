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
 * Description: Treck strcmp "C" library function
 *
 * Filename: trstrcmp.c
 * Author: Jason
 * Date Created: 07/01/98
 * $Source: source/sockapi/trstrcmp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:41JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* 
 * Compare two strings to see if they are equal
 * Does not need to be fast since it is so rarely called
 */

#include <trsocket.h>
#include <trmacro.h>

int tfStrCmp(const char TM_FAR *string1, const char TM_FAR *string2)
{
    int retCode;

    retCode = TM_STRCMP_EQUAL;
    
    if (    (string1 != (const char TM_FAR *)0)
         && (string2 != (const char TM_FAR *)0) )
    {
/* First look for non matching strings */
        while ( (*string1 != (char)0) && (*string2 != (char)0) )
        {
            if (*string1 < *string2)
            {
                retCode = TM_STRCMP_LESS;
                break;
            }
            if (*string1 > *string2)
            {
                retCode = TM_STRCMP_GREATER;
                break;
            }
            string1++;
            string2++;
        }
        if (retCode == TM_STRCMP_EQUAL)
        {
/* Now check for one string being longer than the other */
            if (*string1 != (char)0)
            {
                retCode = TM_STRCMP_GREATER;
            }
            else
            {
                if (*string2 != (char)0)
                {
                    retCode = TM_STRCMP_LESS;
                }
            }
        }
    }
    return(retCode);
}

int tfStriCmp(const char TM_FAR *string1, const char TM_FAR *string2)
{
    int      retCode;
    char     upper1;
    char     upper2;

    retCode = TM_STRCMP_EQUAL;
    
    if (    (string1 != (const char TM_FAR *)0)
         && (string2 != (const char TM_FAR *)0) )
    {
/* First look for non matching strings */
        while ( (*string1 != (char)0) && (*string2 != (char)0) )
        {
            upper1 = (char)(tm_toupper(*string1));
            upper2 = (char)(tm_toupper(*string2));

            if (upper1 < upper2)
            {
                retCode = TM_STRCMP_LESS;
                break;
            }
            if (upper1 > upper2)
            {
                retCode = TM_STRCMP_GREATER;
                break;
            }
            string1++;
            string2++;
        }
        if (retCode == TM_STRCMP_EQUAL)
        {
/* Now check for one string being longer than the other */
            if (*string1 != (char)0)
            {
                retCode = TM_STRCMP_GREATER;
            }
            else
            {
                if (*string2 != (char)0)
                {
                    retCode = TM_STRCMP_LESS;
                }
            }
        }
    }
    return(retCode);
}
