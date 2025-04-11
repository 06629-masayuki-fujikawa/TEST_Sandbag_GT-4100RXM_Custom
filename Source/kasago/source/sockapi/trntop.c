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
 * Description: inet_ntop socket API.
 *
 * Filename: trntop.c
 * Author: Bryan Wang
 * Date Created:
 * $Source: source/sockapi/trntop.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:52JST $
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

/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */
#ifdef TM_USE_IPV6
void inet_ntop6(const void TM_FAR * src, char TM_FAR * dst);
#endif /* TM_USE_IPV6 */
void inet_ntop4(const void TM_FAR * src, char TM_FAR * dst);
/* local variable definitions */

/* static const tables */

/* macro definitions */

/****************************************************************************
* FUNCTION: inet_ntop
*
* PURPOSE:  Refer to [RFC2553]. Converts an IP address from the binary format 
*   to the standard text presentation format. 
*
* PARAMETERS:
* af               
*   The af argument specifies the family of the address. 
*   This can be AF_INET or AF_INET6.
* src
*   The src argument points to a buffer holding an IPv4 address if the af 
*   argument is AF_INET, or an IPv6 address if the af argument is AF_INET6,
*   the address must be in network byte order. 
* dst
*   The dst argument points to a buffer where the function will store the
*   resulting text string. The size argument specifies the size of this 
*   buffer. The application must specify a non-NULL dst argument. For IPv6
*   addresses, the buffer must be at least 46 octets. For IPv4 addresses,
*   the buffer must be at least 16-octets. In order to allow applications
*   to easily declare buffers of the proper size to store IPv4 and IPv6 
*   addresses in string form, the following two constants are defined:
*   #define INET_ADDRSTRLEN    16      
*   #define INET6_ADDRSTRLEN   46 
* size              
*   The buffer Size pointed by dst
*
* RETURNS:
*   a pointer to the buffer containing the text string if the succeeds
*   NULL otherwise
* ERRORNO:
*   TM_EAFNOSUPPORT the af argument is invalid
*   TM_ENOSPC       the size of the result buffer is inadequate
* NOTE:            The first sequence of zeros are abbreved as "::"
*                  ipv4 compatible address is presented as ::ddd.ddd.ddd.ddd
*                  ipv4 only address is presented as ::FFFF:ddd.ddd.ddd.ddd
*
****************************************************************************/

const char TM_FAR * inet_ntop(
    int af,
    const void TM_FAR * src,
    char TM_FAR * dst,
    int size )
{
    int errorCode;

    errorCode = TM_ENOERROR;

/* verify parameter, the rest are verified in the scitch block */
    if ( src == (const void TM_FAR*)0 || dst == (char TM_FAR *)0)
    {
        errorCode = TM_ENOSPC;
        goto inetntopfinish;
    }

    switch(af)
    {
#ifdef TM_USE_IPV4
        case AF_INET:
            if(size < INET_ADDRSTRLEN)
            {
                errorCode = TM_ENOSPC;
                goto inetntopfinish;
            }
            inet_ntop4(src, dst);
            break;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        case AF_INET6:
            if(size < INET6_ADDRSTRLEN)
            {
                errorCode = TM_ENOSPC;
                goto inetntopfinish;
            }
            inet_ntop6(src, dst);
            break;
#endif
        default:
        errorCode = TM_EAFNOSUPPORT;
        goto inetntopfinish;

    }

inetntopfinish:
    if (errorCode != TM_ENOERROR)
    {
/* store errno into the non socket specific globle variable */
        tm_context(tvErrorCode) = errorCode;
        dst = (char TM_FAR *)0;
    }

    return (const char TM_FAR *)dst;
}


/* converts a ipv4 address value into the dot decimal format */
void inet_ntop4(const void TM_FAR * src, char TM_FAR * dst)
{
    unsigned char TM_FAR * bytePtr;
    bytePtr = (unsigned char TM_FAR *)src;

    tm_sprintf((ttCharPtr) dst, 
               "%u.%u.%u.%u", 
               bytePtr[0], 
               bytePtr[1], 
               bytePtr[2], 
               bytePtr[3]);
}

/* converts a ipv6 address value into the test representation */
#ifdef TM_USE_IPV6
void inet_ntop6(const void TM_FAR * src, char TM_FAR * dst)
{
    int                     i;
    int                     maxShortIndex;
    int                     inZero;   /* in the middle of a zero sequence */
    int                     overZero; /* processed a zero sequence already*/
    unsigned short TM_FAR*  wordPtr;  /* pointing to the current word */
    int                     soFarZero;/* so far we've met zeros only */

    inZero    = 0;
    overZero  = 0;
    soFarZero = 1;

    wordPtr = (unsigned short TM_FAR*)src;

    maxShortIndex = (int)(TM_6_IP_ADDRESS_LENGTH / (int)sizeof(short)) - 1;
    for (i= 0; i < maxShortIndex; i++)
    {
        if (i==5 && soFarZero==1 && (wordPtr[6] && 0xFF00) != 0)
        {
            if(wordPtr[i]==0)
            {
/* ::d.d.d.d */
                inet_ntop4((const void TM_FAR *)(&wordPtr[6]), dst);
                goto inetntop6finish;
            }
            if(wordPtr[i]==0xffffL)
            {
/* ::FFFF:d.d.d.d */
                dst += tm_sprintf(dst, "FFFF:");
                inet_ntop4((const void TM_FAR *)(&wordPtr[6]), dst);
                goto inetntop6finish;
            }
        }

        if (wordPtr[i] ==0 && overZero == 0)
        {
/* first zero sequence */
            if (inZero == 0)
            {
                if (i==0)
                {
/* the address starts with 0 */
                    dst += tm_sprintf(dst, "::");
                }
                else
                {
/* a zero sequence in the middle of the address */
                    dst += tm_sprintf(dst, ":");
                }
/* note we are in a zero sequence */
                inZero = 1;
            }
        }
        else
        {
/* 
 * either not zero or not the first sequence of zero, 
 * convert as a normal value 
 */
            if (inZero ==1)
            {
                inZero = 0 ;
                overZero =1;
            }
    
            if (soFarZero == 1)
            {
                soFarZero = 0;
            }
            dst += tm_sprintf(dst, "%X:", ntohs(wordPtr[i]));
        }
    }
/* the last segment */
    if ( !((wordPtr[i] == 0) && (inZero == 1)) )
    {
        dst += tm_sprintf(dst, "%X", ntohs(wordPtr[i]));
    }
inetntop6finish:
    return;
}
#endif /* TM_USE_IPV6 */

