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
 * Description: Treck memchr "C" library function
 *
 * Filename: trmemchr.c
 * Author: Emmanuel
 * Date Created: 02/23/99
 * $Source: source/sockapi/trmemchr.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:46JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
#include <trsocket.h>

void TM_FAR * tfMemChr( const    void TM_FAR * pBuf,
                                 int           nByte,
                        unsigned int           uCount )
{
             void TM_FAR * pResult;
    unsigned int           uPos;

    pResult = (void TM_FAR *)0;
    if (pBuf != (const void TM_FAR *)0)
    {
        for (uPos = 0; uPos < uCount; uPos++)
        {
            if (    ((const unsigned char TM_FAR *)pBuf)[uPos]
                 == (unsigned char)nByte )
            {
                pResult = (void TM_FAR *)
                            &((const unsigned char TM_FAR *)pBuf)[uPos];
                break;
            }
        }
    }
    return pResult;
}
