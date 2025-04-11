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
 * Description: Treck memcmp "C" library function
 *
 * Filename: trmemcmp.c
 * Author: Odile
 * Date Created: 02/18/99
 * $Source: source/sockapi/trmemcmp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:47JST $
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
 * Compare two buffers up to length bytes to see if they are equal
 */
int tfMemCmp( const    void TM_FAR *bufPtr1,
              const    void TM_FAR *bufPtr2,
              unsigned int          length )
{
    int          retCode;
    unsigned int i;

    retCode = TM_STRCMP_EQUAL;
    if (     (bufPtr1 != (const void TM_FAR *)0)
          && (bufPtr2 != (const void TM_FAR *)0) )
    {

        i = (unsigned int)0;
        while ( i < length )
        {
            if ( ((ttConstUCharPtr)bufPtr1)[i] < ((ttConstUCharPtr)bufPtr2)[i] )
            {
                retCode = TM_STRCMP_LESS;
                break;
            }
            if ( ((ttConstUCharPtr)bufPtr1)[i] > ((ttConstUCharPtr)bufPtr2)[i] )
            {  
                retCode = TM_STRCMP_GREATER;
                break;
            }
            i++;
        }
    }
    return(retCode);
}
