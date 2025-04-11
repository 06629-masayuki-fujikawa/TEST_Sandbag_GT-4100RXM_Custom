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
 * Description: Treck strlen "C" library function
 *
 * Filename: trstrlen.c
 * Author: Jason
 * Date Created: 07/01/98
 * $Source: source/sockapi/trstrlen.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:45JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>

/* 
 * Copy from one string to another
 * Does not need to be fast since it is so rarely called
 */

unsigned int tfStrLen(const char TM_FAR * strPtr)
{
    unsigned int length;
    
    length = 0;

    if ( strPtr != (const char TM_FAR *)0 )
    {
        while ( strPtr[length] != '\0' )
        {
            length++;
        }
    }
    return length;
}
