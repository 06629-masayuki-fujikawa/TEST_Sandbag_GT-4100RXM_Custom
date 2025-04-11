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
 * Description:  BSD sockets inet_ntoa function
 *
 * Filename: trinetnt.c
 * Author: Odile
 * Date Created: 2/19/99
 * $Source: source/sockapi/trinetnt.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:35JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>

#ifndef tm_get_byte_ptr
/* backward compatibility */
#ifdef TM_DSP
/* 
 * Retrieve an 8-bit byte at offset in packed data pointed to by dataPtr
 * Equivalent to dataPtr[offset] on a general purpose processor.
 */
#define tm_get_byte_ptr(dataPtr, offset) \
 (((dataPtr[((offset) >> TM_DSP_SHIFT)]) \
     >> ((TM_DSP_ROUND_PTR - (offset % TM_DSP_BYTES_PER_WORD)) << 3)) & 0xff)
#else /* !TM_DSP */
/* retrieve byte at offset in d data pointed to by dataPtr */
#define tm_get_byte_ptr(dataPtr, offset) \
        (((int)dataPtr[offset]) & 0xFF)
#endif /* !TM_DSP */
#endif /* !tm_get_bypte_ptr */

static char TM_FAR tlInetBuffer[16];


/*
 * Convert a socket address to the "h1.h2.h3.h4\0" format.
 * Parameters
 * Value          Meaning
 * in_addr        IN address structure containing the IP address
 *
 * Return value:
 * Pointer to converted string.
 */
char TM_FAR * inet_ntoa ( struct in_addr  inAddr )
{
        ttCharPtr   aHostPtr;
auto    tt4IpAddress hostIpAddress;
        int         commandIndex;
        int         dotIndex;
        int         tempInt;
        int         digit;
        char        delimiterChar;

static  const char   TM_CONST_QLF tlAsciiDigit[11] = "0123456789";

    tm_ip_copy(inAddr.s_addr, hostIpAddress);
    aHostPtr = (ttCharPtr)&hostIpAddress;
    commandIndex = 0;
    dotIndex = 0;
    delimiterChar = '.';
    while ( (unsigned)dotIndex < tm_byte_count(sizeof(tt4IpAddress) ) )
    {
        tempInt = (int)tm_get_byte_ptr(aHostPtr, dotIndex);
        if (tempInt >= 100)
        {
            digit = tempInt/100;
            tempInt = tempInt % 100;
            tlInetBuffer[commandIndex] = tlAsciiDigit[digit];
            commandIndex++;
            if (tempInt < 10)
/* Middle digit is zero */
            {
                tlInetBuffer[commandIndex] = tlAsciiDigit[0];
                commandIndex++;
            }
        }
        if (tempInt >= 10)
        {
            digit = tempInt/10;
            tempInt = tempInt % 10;
            tlInetBuffer[commandIndex] = tlAsciiDigit[digit];
            commandIndex++;
        }
        tlInetBuffer[commandIndex] = tlAsciiDigit[tempInt];
        commandIndex++;
        dotIndex++;
        if ( (unsigned)dotIndex == (tm_byte_count(sizeof(tt4IpAddress))) )
        {
            delimiterChar = '\0';
        }
        tlInetBuffer[commandIndex++] = delimiterChar;
    }
    return tlInetBuffer;
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4InetntDummy = 0;
#endif /* ! TM_USE_IPV4 */
