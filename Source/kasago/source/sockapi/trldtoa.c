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
 * Description: Convert an unsigned long decimal integer to string 
 *
 * Filename: trltoa.c
 * Author: Odile Lapidus
 * Date Created: 05/27/04
 * $Source: source/sockapi/trldtoa.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:42JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

/*
 * tfUlongDecimalToString description:
 *   Converts an unsigned long decimal integer into a string and copies it 
 *   into the specified buffer, returning a pointer to the end of this buffer.
 *
 * Parameters:
 *  Value               Meaning
 *  destPtr             buffer to place the new data into
 *  ulongDecNum         unsigned long decimal number to convert
 *
 * Returns:
 *  Value               Meaning
 *                      pointer to the end of destPtr
 */
ttCharPtr tfUlongDecimalToString (ttCharPtr destPtr, tt32Bit ulongDecNum)
{
    int     i;
    int     j;
    tt32Bit tempUlong;

/* 
 * Compute the number of digits needed to convert 'ulongDecNum' to ASCII 
 * string
 */
    i = 0;
    tempUlong = ulongDecNum;
    do
    {
        tempUlong = tempUlong / 10;
        i ++;
    } while (tempUlong != TM_UL(0));
/* convert 'ulongDecNum' to ASCII and place into destPtr */
    for ( j = 0; j < i; j ++ )
    {
        tempUlong = ulongDecNum % 10;

        destPtr[i - j - 1] = (char)(tempUlong | 0x30);
        ulongDecNum = ulongDecNum / 10;
    }
    destPtr[j] = '\0';
/* Returns a pointer to the end of destPtr */
    return (destPtr + j);
}
