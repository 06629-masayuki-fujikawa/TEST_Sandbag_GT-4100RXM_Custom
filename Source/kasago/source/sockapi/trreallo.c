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
 * Description: Treck User Realloc "C" library function
 *
 * Filename: trreallo.c
 * Author: Odile
 * Date Created: 02/18/99
 * $Source: source/sockapi/trreallo.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:03JST $
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
 * Re-allocate the previously allocated block
 * in userPtr, making the new block size bytes long.
 * Unconditionally allocate and copy to a new region.
 */
ttVoidPtr tfRealloc(ttVoidPtr userPtr, unsigned size)
{
    ttVoidPtr newUserPtr;

    newUserPtr = tm_malloc(size);
    if ( userPtr != (ttVoidPtr)0 )
    {
/*
 * Make sure that userPtr, newUserPtr, and size are non zero, since
 * tfBcopy assumes that ptr, newPtr, and size are non zero
 */
        if (    (newUserPtr != (ttVoidPtr)0)
             && (size != 0) )
        {
            tm_bcopy(userPtr, newUserPtr, size);
        }
        tm_free(userPtr);
    }
    return newUserPtr;
}
