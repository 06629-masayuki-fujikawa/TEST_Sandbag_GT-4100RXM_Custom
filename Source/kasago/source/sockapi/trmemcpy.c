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
 * Description: Treck memcpy "C" library function
 *
 * Filename: trmemcpy.c
 * Author: Jason
 * Date Created: 07/01/98
 * $Source: source/sockapi/trmemcpy.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:48JST $
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
 * Sets memSize bytes at memAddr to valueToSet
 */

void tfMemCpy( void       TM_FAR * destination,
               const void TM_FAR * source,
               unsigned            length)
{
    if (    (destination != (void TM_FAR *)0)
         && (source != (const void TM_FAR *)0)
         && (length != 0) )
    {
        tm_bcopy(source, destination, length);
    }
}
