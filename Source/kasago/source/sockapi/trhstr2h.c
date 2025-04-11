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
 * Description: Hexidecimal byte string to Hexidecimal byte array
 *
 * Filename: trhstr2h.c
 * Author: Jin Zhang
 * Date Created:
 * $Source: source/sockapi/trhstr2h.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:29JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* Include */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


/****************************************************************************
 * FUNCTION: tfHexByteStr2HexByteArr
 *
 * PURPOSE: converts a hexidecimal byte string to a hexidecimal byte array.
 *          for example, char *chs= "1A2B", it will be converted to 
 *          char cha[]={0x1A, 0x2B}.  (not 0x1A2B, but 0x1A, 0x2B
 *
 * PARAMETER:
 * INPUT:      src       const pointer pointing to the hexidecimal string
 *             len       Number of bytes in the result array, for the 
 *                       example "1A2B", or "1A 2B", the length is 2.
 *             dst       pointer to the hexideciamil byte array.
 *        
 * RETURN:     0     successful return
 *            -1     failed.
 */
int tfHexByteStr2HexByteArr(const char TM_FAR * src,
                            int                 len,
                            char TM_FAR       * dst)
{
    int       i;
    char      ch;
    char      val = 0;

    for(i = 0; i < len; i++)
    {
/* the higher hexidecimal digit */
        ch = *src++;
        if( ch >= '0' && ch <= '9')
        {
            val = (char)((ch - '0') << 4 );
        } 
        else if( ch >= 'a' && ch <= 'f')
        {
            val = (char)( (ch - 'a' + 10) << 4);
        } 
        else if( ch >= 'A' && ch <= 'F')
        {
            val = (char)((ch - 'A' + 10) << 4);
        }
        else if( ch == ' ' || ch == '\t')
        {
/* space or table key should not affect */
            i--;
            continue;
        }
        else
        {
            return -1;
        }
        
/* the lower hexidecimal digit */
        ch = *src++;
        if( ch >= '0' && ch <= '9')
        {
            val =(char)( val + (ch - '0'));
        } 
        else if( ch >= 'a' && ch <= 'f')
        {
            val = (char)(val+ (ch - 'a' + 10));
        } 
        else if( ch >= 'A' && ch <= 'F')
        {
            val =(char)(val + (ch - 'A' + 10));
        }
        else
        {
            return -1;
        }
        *dst++ = val;
    }
    return 0;
}

