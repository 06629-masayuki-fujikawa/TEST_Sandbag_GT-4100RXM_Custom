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
 * Description: Treck strncmp "C" library function
 *
 * Filename: trstrncm.c
 * Author: Odile
 * Date Created: 02/19/99
 * $Source: source/sockapi/trstrncm.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:46JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h>

/* 
 * Compare two strings to see if they are equal up to n bytes.
 */
int tfStrNCmp( const char TM_FAR *stringPtr1,
               const char TM_FAR *stringPtr2,
               unsigned int       nBytes )
{
    unsigned int i;
             int retCode;

    retCode = TM_STRCMP_EQUAL;
    if (    (stringPtr1 != (const char TM_FAR *)0)
         && (stringPtr2 != (const char TM_FAR *)0) )
    {
    
        i = 0;
/* First look for non matching strings */
        while (    (stringPtr1[i] != (char)0)
                && (stringPtr2[i] != (char)0)
                && (i < nBytes) )
        {
            if (stringPtr1[i] < stringPtr2[i])
            {
                retCode = TM_STRCMP_LESS;
                break;
            }
            if (stringPtr1[i] > stringPtr2[i])
            {  
                retCode = TM_STRCMP_GREATER;
                break;
            }
            i++;
        }
        if (    (retCode == TM_STRCMP_EQUAL)
             && (i != nBytes) )
        {
/* Now check for one string being longer than the other */
            if (stringPtr1[i] != (char)0)
            {
                retCode = TM_STRCMP_GREATER;
            }
            else
            {
                if (stringPtr2[i] != (char)0)
                {
                    retCode = TM_STRCMP_LESS;
                }
            }
        }
    }

    return(retCode);
}
