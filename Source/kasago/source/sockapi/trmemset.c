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
 * Description: Treck memset "C" library function
 *
 * Filename: trmemset.c
 * Author: Jason
 * Date Created: 07/01/98
 * $Source: source/sockapi/trmemset.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:49JST $
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

void tfMemSet( void TM_FAR       *buffer,
               int               fillCharacter,
               unsigned int      length)
{
    char          TM_FAR * bufCharPtr;
    tt32BitPtr             bufLongPtr;
    tt32Bit                fillLong;
    unsigned char          fillChar;

    if (    (buffer != (void TM_FAR *)0)
         && (length != 0) )
    {
        fillChar = (unsigned char)fillCharacter;
        fillLong = (tt32Bit)fillCharacter;
        if (fillCharacter != 0)
        {
            fillLong |= fillLong << 8;
            fillLong |= fillLong << 8;
            fillLong |= fillLong << 8;
        }
    
        bufCharPtr = (char TM_FAR *)buffer;
        
        while (    ((ttUserPtrCastToInt) bufCharPtr & TM_ROUND_PTR)
                && (length != 0) )
        {
            *bufCharPtr++ = (char)fillChar;
            length--;
        }
        bufLongPtr = (tt32BitPtr)bufCharPtr;
        while ( length >= 32 * sizeof(tt32Bit) )
        {
            bufLongPtr[0] = fillLong;
            bufLongPtr[1] = fillLong;
            bufLongPtr[2] = fillLong;
            bufLongPtr[3] = fillLong;
            bufLongPtr[4] = fillLong;
            bufLongPtr[5] = fillLong;
            bufLongPtr[6] = fillLong;
            bufLongPtr[7] = fillLong;
            bufLongPtr[8] = fillLong;
            bufLongPtr[9] = fillLong;
            bufLongPtr[10] = fillLong;
            bufLongPtr[11] = fillLong;
            bufLongPtr[12] = fillLong;
            bufLongPtr[13] = fillLong;
            bufLongPtr[14] = fillLong;
            bufLongPtr[15] = fillLong;
            bufLongPtr[16] = fillLong;
            bufLongPtr[17] = fillLong;
            bufLongPtr[18] = fillLong;
            bufLongPtr[19] = fillLong;
            bufLongPtr[20] = fillLong;
            bufLongPtr[21] = fillLong;
            bufLongPtr[22] = fillLong;
            bufLongPtr[23] = fillLong;
            bufLongPtr[24] = fillLong;
            bufLongPtr[25] = fillLong;
            bufLongPtr[26] = fillLong;
            bufLongPtr[27] = fillLong;
            bufLongPtr[28] = fillLong;
            bufLongPtr[29] = fillLong;
            bufLongPtr[30] = fillLong;
            bufLongPtr[31] = fillLong;
            bufLongPtr += 32;
            length -= 32 * sizeof(tt32Bit);
        }
        while ( length >= 16 * sizeof(tt32Bit) )
        {
            bufLongPtr[0] = fillLong;
            bufLongPtr[1] = fillLong;
            bufLongPtr[2] = fillLong;
            bufLongPtr[3] = fillLong;
            bufLongPtr[4] = fillLong;
            bufLongPtr[5] = fillLong;
            bufLongPtr[6] = fillLong;
            bufLongPtr[7] = fillLong;
            bufLongPtr[8] = fillLong;
            bufLongPtr[9] = fillLong;
            bufLongPtr[10] = fillLong;
            bufLongPtr[11] = fillLong;
            bufLongPtr[12] = fillLong;
            bufLongPtr[13] = fillLong;
            bufLongPtr[14] = fillLong;
            bufLongPtr[15] = fillLong;
            bufLongPtr += 16;
            length -= 16 * sizeof(tt32Bit);
        }
        while ( length >= 8 * sizeof(tt32Bit) )
        {
            bufLongPtr[0] = fillLong;
            bufLongPtr[1] = fillLong;
            bufLongPtr[2] = fillLong;
            bufLongPtr[3] = fillLong;
            bufLongPtr[4] = fillLong;
            bufLongPtr[5] = fillLong;
            bufLongPtr[6] = fillLong;
            bufLongPtr[7] = fillLong;
            bufLongPtr += 8;
            length -= 8 * sizeof(tt32Bit);
        }
        while ( length >= 4 * sizeof(tt32Bit) )
        {
            bufLongPtr[0] = fillLong;
            bufLongPtr[1] = fillLong;
            bufLongPtr[2] = fillLong;
            bufLongPtr[3] = fillLong;
            bufLongPtr += 4;
            length -= 4 * sizeof(tt32Bit);
        }
        while ( length >= sizeof(tt32Bit) )
        {
            *bufLongPtr++ = fillLong;
            length -= sizeof(tt32Bit);
        }
        bufCharPtr = (char TM_FAR *)bufLongPtr;
        while (length != 0)
        {
            *bufCharPtr++ = (char)fillChar;
            length--;
        }
    }
}
