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
 * Description: inet_pton socket API.
 *
 * Filename: trpton.c
 * Author: Bryan Wang
 * Date Created:
 * $Source: source/sockapi/trpton.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:56JST $
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
int inet_pton6(ttConstCharPtr src, tt16BitPtr dst);
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
int inet_pton4(ttConstCharPtr src, tt32BitPtr dst);
#endif /* TM_USE_IPV4 */

/* local variable definitions */

/* static const tables */

/* macro definitions */
#define TM_6_MAX_IP_SEGMENT   (TM_6_IP_ADDRESS_LENGTH / 2)

/****************************************************************************
* FUNCTION: inet_pton
*
* PURPOSE: converts an address in its standard text presentation form into
*   its numeric binary form.
*
* PARAMETERS:
*   af:
*       specifies the family of the address. Currently the AF_INET and
*       AF_INET6 address families are supported.
*   src:
*       points to the string being passed in.
*   dst:
*       points to a buffer into which the function stores the numeric address
*       The address is returned in network byte order. The calling
*       appliaction must ensure that the buffer referred to by dst is large
*       enough to hold the numeric address(4 bytes for V4 address, and
*       16 bytes for V6 address)
* RETURNS:
*   1:
*       Sucessful
*   0:
*       input is not a valid IPv4 dotted-decimal string or a valid IPv6
*       address string, or dst is NULL
*  -1:
*       af argument is unknown
* ERRORNO:
*       TM_EAFNOSUPPORT, when returns -1
* NOTES:
*       For ipv6 address, Many implementations of the existing inet_addr()
*       and inet_aton() functions accept nonstandard input: octal numbers,
*       hexadecimal numbers, and fewer than four numbers. inet_pton() does
*       not accept these formats.
*
*       DSP targets: These routines have been modified to work with
*                    correctly with 32-bit DSP's.  Since these routines
*                    always treat the output as a 16-bit array, no
*                    modification should be necessary for 16-bit DSP's.
*
*       We are returning 0 for NULL dst, though this is not specified by
*       RFC 2553
*
****************************************************************************/
int inet_pton(
    int                af,
    ttUserConstCharPtr src,
    ttUserVoidPtr      dst )
{
#ifdef TM_USE_IPV4
    struct in_addr  inAddr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    struct in6_addr in6Addr;
#endif /* TM_USE_IPV6 */
    int             errorCode;

    if ((src == (ttUserConstCharPtr)0) || (dst == (ttUserVoidPtr)0))
    {
        errorCode = 0;
        goto inetptonfinish;
    }

    switch(af)
    {
#ifdef TM_USE_IPV4
        case AF_INET:
            errorCode = inet_pton4(src, &(inAddr.s_addr));
            tm_bcopy(&inAddr,
                     dst,
                     sizeof(struct in_addr));
            break;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        case AF_INET6:
            errorCode = inet_pton6(src, &(in6Addr.s6SAddr[0]));
            tm_bcopy(&in6Addr,
                     dst,
                     sizeof(struct in6_addr));
            break;
#endif /* TM_USE_IPV6 */
        default:
            errorCode = -1;
            break;
    }

inetptonfinish:
    if (errorCode == -1)
    {
/* store errno into the non socket specific globle variable */
        tm_context(tvErrorCode) = TM_EAFNOSUPPORT;
    }

    return errorCode;
}


#ifdef TM_USE_IPV4
/*
 *  return:
 *      1:   sucess
 *      0:   invalid address string
 */
int inet_pton4(ttConstCharPtr src, tt32BitPtr dst)
{
    tt32Bit addrVal;
    int     errorCode;
    int     isEqual;
    errorCode = 1;

/*
 * if inet_aton return 0, we can't tell wheter it is an error,
 * or the input address is 0.0.0.0, so we handle it oouselves
 */
    isEqual = tm_strcmp(src, "0.0.0.0");
    if( !isEqual )
    {
        *dst = TM_UL(0);
    }
    else
    {
        addrVal = inet_aton((char TM_FAR *)src);
        if (addrVal != 0)
        {
/* inet_aton returns 0 on error */
            *dst = addrVal;

        }
        else
        {
            errorCode = 0;
        }
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */

/*
 *  return:
 *      1:   sucess
 *      0:   invalid address string
 */
#ifdef TM_USE_IPV6
int inet_pton6(ttConstCharPtr src, tt16BitPtr dst)
{
    const               char digitArray[] ="0123456789abcdefABCDEF";
    int                 currChar;     /* current char                   */
    tt16Bit             segmentValue; /* value of the current segment   */
    int                 segmentCount; /* segment counter                */
    const char TM_FAR * segmentPtr;   /* for roll back when "." is met  */
    char TM_FAR *       digitBase;    /* address of the fist digit      */
    int                 digitCount;   /* consecutive digit counter,     */
    const char TM_FAR * digitPos;     /* relative to the digit array    */
    tt16BitPtr          dstBase;      /* remember the base when walking */
    int                 colonCount;   /* consecutive colon couter,      */
    int                 colonToGo;    /* colon counts in the left string*/
    int                 dotToGo;      /* dot counts in the left string  */
    tt8Bit              doubleColon;  /* indicate we've met one "::"    */
    char TM_FAR *       tempPtr;
    char                tempCh;
    int                 errorCode;
#ifdef TM_32BIT_DSP
    int                 outputByteCount;
#endif /* TM_32BIT_DSP */

    errorCode   = 1;
    doubleColon = 0;
    segmentValue= 0;
/* TM_6_MAX_IP_SEGMENT is 1-based, so 'segmentCount' must be too */
    segmentCount= 1;
    digitCount  = 0;
    colonCount  = 0;
    colonToGo   = 0;
    dotToGo     = 0;
    segmentPtr  = (const char TM_FAR *)0;
    dstBase     = dst;
    digitBase = (char TM_FAR *)digitArray;
#ifdef TM_32BIT_DSP
    outputByteCount = 0;
#endif /* TM_32BIT_DSP */

    if (*src == ':' && *(src + 1) != ':')
    {
/* illegal pattern */
        errorCode = 0;
        goto inetpton6finish;
    }

    tm_bzero(dst, TM_6PAK_IP_ADDRESS_LENGTH);
/* PRQA: QAC Message 3416: It is processing intended by the loop count */
/* PRQA: QAC Message 3416: processing */
/* PRQA S 3416 L1 */
    while ((currChar = *src++) != '\0')
/* PRQA L:L1 */
    {
        digitPos = tm_strchr(digitArray, currChar);
        if( digitPos != 0 )
        {
            colonCount = 0;
            if ((char TM_FAR *)digitPos - digitBase > 15)
            {
/* make upper case lower case */
                digitPos -= 6;
            }
            digitCount++;
            if( digitCount > 4 )
            {
/* too many digit in one segmentValue */
                errorCode = 0;
                goto inetpton6finish;
            }
            segmentValue <<= 4;
            segmentValue |= (tt16Bit) ((char TM_FAR *)digitPos - digitBase);
            if (*src == '\0')
            {
/* the end, pick up the last segmentValue and go*/
#ifdef TM_32BIT_DSP
                if (outputByteCount == 0)
                {
                    *dst++ = htons(segmentValue) << 16;
                }
                else
                {
                    *dst++ |= htons(segmentValue);
                }
#else /* !TM_32BIT_DSP */
                *dst++ = htons(segmentValue);
#endif /* TM_32BIT_DSP */
                goto inetpton6finish;
            }
            else
            {
/* not the end, shortcut to the next loop */
                continue;
            }
        }

        if(currChar == ':')
        {
            if (*src != ':')
            {
                if ((*src == '\0') && (colonCount == 0))
                {
/* Ended with a single ":" */
                    errorCode = 0;
                    goto inetpton6finish;
                }
/* Only increase the 'segmentCount' if we are not at a double colon. */
                segmentCount++;
            }

            if (segmentCount > TM_6_MAX_IP_SEGMENT)
            {
/* too many segments  */
                errorCode = 0;
                goto inetpton6finish;
            }

            segmentPtr = src;

            colonCount++;
            if( colonCount > 2 )
            {
/* ::: , illegal*/
                errorCode = 0;
                goto inetpton6finish;
            }

            if (digitCount != 0)
            {
/* xxxx: */
#ifdef TM_32BIT_DSP
                if (outputByteCount == 0)
                {
                    *dst = htons(segmentValue) << 16;
                    outputByteCount += 2;
                }
                else
                {
                    *dst |= htons(segmentValue) & 0x0000ffff;
                    dst++;
                    outputByteCount = 0;
                }
#else /* !TM_32BIT_DSP */
                *dst++ = htons(segmentValue);
#endif /* TM_32BIT_DSP */
                segmentValue = 0;
                digitCount = 0;
                continue;
            }
            else if (colonCount == 2)
            {
/* (xxxx):: */
                if (doubleColon == 1)
                {
/* a second "::" */
                    errorCode = 0;
                    goto inetpton6finish;
                }
                else
                {
                    doubleColon = 1;
                }
/* count the following ":" and "." to caculate where to resume */
                colonToGo = 0;
                dotToGo   = 0 ;
                tempPtr = (char TM_FAR *)src;
                tempCh  = *tempPtr;
                while( tempCh != '\0')
                {
                    if(tempCh == ':')
                    {
                        colonToGo ++;
                    }
                    if(tempCh == '.')
                    {
                        dotToGo ++;
                    }
                    tempPtr++;
                    tempCh  = *tempPtr;
                }

                if(dotToGo == 3)
                {
/* xxxx::(xxxx:)ddd.ddd.ddd.ddd */
                    if (segmentCount + colonToGo + 1 > TM_6_MAX_IP_SEGMENT)
                    {
                        errorCode = 0;
                        goto inetpton6finish;
                    }
                    else
                    {
#ifdef TM_32BIT_DSP
                        dst = dstBase /* 32 bits pointer */
                            + TM_6_IP_ADDRESS_LENGTH / 4
                            - TM_IP_ADDRESS_LENGTH / 4
                            - (colonToGo+1) / 2;
                        if (colonToGo & 1)
                        {
                            outputByteCount = 2;
                        }
                        else
                        {
                            outputByteCount = 0;
                        }
#else /* !TM_32BIT_DSP */
                        dst = dstBase /* 16 bits pointer */
                            + TM_6_IP_ADDRESS_LENGTH / 2
                            - TM_IP_ADDRESS_LENGTH / 2
                            - colonToGo;
#endif /* TM_32BIT_DSP */
                    }
                }
                else if (dotToGo == 0)
                {
                    if (segmentCount + colonToGo > TM_6_MAX_IP_SEGMENT)
                    {
/* too many ":" and "." */
                        errorCode = 0;
                        goto inetpton6finish;
                    }
#ifdef TM_32BIT_DSP
                    dst = dstBase /* 32 bits pointer */
                        + TM_6_IP_ADDRESS_LENGTH / 4
                        - ((colonToGo + 2) / 2);
                    if ( (colonToGo + 1) & 1)
                    {
                        outputByteCount = 2;
                    }
                    else
                    {
                        outputByteCount = 0;
                    }
#else /* !TM_32BIT_DSP */
                    dst = dstBase /* 16 bits pointer */
                        + TM_6_IP_ADDRESS_LENGTH / 2
                        - colonToGo
                        - 1;
#endif /* TM_32BIT_DSP */
                }
                else
                {
/* wrong number of "." */
                    errorCode = 0;
                    goto inetpton6finish;
                }
            }
        }

#ifdef TM_USE_IPV4
        else if(currChar == '.')
        {
            if (  digitCount == 0
                || segmentCount + 1 > TM_6_MAX_IP_SEGMENT
                || segmentCount < 2)
            {
/* "." not led  by digits, or too many segments, or too few segments  */
                errorCode = 0;
                goto inetpton6finish;
            }
            segmentCount++;
#ifdef TM_32BIT_DSP
            dst = dstBase /* 32-bit pointer */
                + TM_6_IP_ADDRESS_LENGTH / 4
                - TM_IP_ADDRESS_LENGTH / 4;
#else /* !TM_32BIT_DSP */
            dst = dstBase /* 16-bit pointer */
                + TM_6_IP_ADDRESS_LENGTH / 2
                - TM_IP_ADDRESS_LENGTH / 2;
#endif /* TM_32BIT_DSP */
            src = segmentPtr;
            errorCode = inet_pton4(src, (tt32BitPtr)(ttVoidPtr)dst);
            goto inetpton6finish;
        }
#endif /* TM_USE_IPV4 */
        else
        {
/* illegal character found */
            errorCode = 0;
            goto inetpton6finish;
        }
    }
inetpton6finish:
    if (segmentCount < TM_6_MAX_IP_SEGMENT && doubleColon == 0)
    {
/* not enough segments */
        errorCode = 0;
    }
    return errorCode;
}
#endif /* TM_USE_IPV6 */

/***************** End Of File *****************/
