/*
 * $Source: source/sockapi/trstrtox.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:51JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
/*-
 * Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>

/* Define unreferenced macro for PC-LINT compile error suppression */
#ifdef TM_LINT
LINT_UNREF_MACRO(LONG_MIN)
#endif /* TM_LINT */

#ifndef LONG_MAX
#define LONG_MAX    TM_L(2147483647)     /* maximum (signed) long value */
#endif /* !LONG_MAX */
#ifndef LONG_MIN
#define LONG_MIN    -TM_L(2147483648)    /* minimum (signed) long value */
#endif /* !LONG_MIN */
#ifndef M_LONG_MIN
#define M_LONG_MIN  TM_UL(2147483648)    /* - LONG_MIN */
#endif /* !M_LONG_MIN */
#ifndef ULONG_MAX
#define ULONG_MAX   TM_UL(0xffffffff)    /* maximum unsigned long value */
#endif /* !ULONG_MAX */

static ttUser32Bit tfStrToXL( const char TM_FAR          * nptr,
                                    char TM_FAR * TM_FAR * endptr,
                                    int                    base,
                                    int                    isUnsigned);

/*
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
ttUserS32Bit tfStrToL( const char TM_FAR          * nptr,
                             char TM_FAR * TM_FAR * endptr,
                             int                    base )
{
    return (ttUserS32Bit) tfStrToXL(nptr, endptr, base, 0);
}

ttUser32Bit tfStrToUl( const char TM_FAR          * nptr,
                             char TM_FAR * TM_FAR * endptr,
                             int                    base)
{
    return tfStrToXL(nptr, endptr, base, 1);
}

static ttUser32Bit tfStrToXL (  const char TM_FAR          * nptr,
                                      char TM_FAR * TM_FAR * endptr,
                                      int                    base,
                                      int                    isUnsigned )
{
    const    char TM_FAR *s;
    ttUser32Bit          acc;
    unsigned char        c;
    ttUser32Bit          cutoff;
    ttUser32Bit          cutlim;
             int         neg;
             int         any;

    /*
     * See strtol for comments as to the logic used.
     */
    s = nptr;
    neg = 0;
    acc = 0;
    if (nptr == (const char TM_FAR *)0)
    {
        goto strtoxExit;
    }
    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do
    {
        c = (unsigned char)*s++;
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
    } while (tm_isspace(c));
/* PRQA L:L1 */

    if (c == '-')
    {
        neg = 1;
        c = (unsigned char)*s++;
    }
    else
    {
        if (c == '+')
        {
            c = (unsigned char)*s++;
        }
    }
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X'))
    {
        c = (unsigned char)s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
    {
        base = c == '0' ? 8 : 10;
    }
    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    if (isUnsigned == 0)
    {
/* tfStrToL() */
        if (neg)
        {
            cutoff = M_LONG_MIN;
        }
        else
        {
            cutoff = LONG_MAX;
        }
        cutlim = cutoff % (ttUser32Bit)base;
        cutoff /= (ttUser32Bit)base;
    }
    else
    {
/* tfStrToUL() */
        cutoff = (ttUser32Bit)ULONG_MAX / (ttUser32Bit)base;
        cutlim = (ttUser32Bit)ULONG_MAX % (ttUser32Bit)base;
    }
    for (any = 0;; c = (unsigned char)*s++)
    {
        if (!tm_isascii(c))
        {
            break;
        }
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
        if (tm_isdigit(c))
/* PRQA L:L1 */
        {
            c -= '0';
        }
        else
        {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
            if (tm_isupper(c))
/* PRQA L:L1 */
            {
                c -= 'A' - 10;
            }
            else
            {

/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                if (tm_islower(c))
/* PRQA L:L1 */
                {
                    c -= 'a' - 10;
                }
                else
                {
                    break;
                }
            }
        }
        if (c >= ((unsigned char) base) )
        {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && 
                                                    c > (unsigned char) cutlim))
        {
            any = -1;
        }
        else
        {
            any = 1;
            acc *= (ttUser32Bit)base;
            acc += c;
        }
    }
    if (any < 0)
    {
        if (isUnsigned == 0)
        {
/* tfStrToL() */
            if (neg)
            {
                acc = M_LONG_MIN;
            }
            else
            {
                acc = LONG_MAX;
            }
        }
        else
        {
/* tfStrToUL() */
            acc = ULONG_MAX;
        }
    }
    else
    {
        if (neg)
        {
            acc = (ttUser32Bit) -(ttUserS32Bit)acc;
        }
    }
    if ( endptr != (char TM_FAR * TM_FAR *)0 )
    {
        *endptr = (char TM_FAR *)(any ? s - 1 : nptr);
    }
strtoxExit:
    return (acc);
}

#if (defined(TM_USE_UPNP_DEVICE) || defined(TM_USE_SOAP))
/* Function Description
 *  Converts a decimal digit string into long value,
 *
 * Parameter Descriptions
 *  strPtr      :   decimal digit string to be converted
 *  numberPtr   :   Points to the result decimal number
 *
 * Return Values
 *  TM_EINVAL   :   if the input string is not pure digits
 *  TM_ENOERROR :   if the input is a string of digits
 */
int tfStr2Int(ttConstCharPtr strPtr, tt32BitPtr numberPtr)
{
    tt32Bit number;
    tt8Bit  digit;
    int     errorCode;

    number = 0;
    errorCode = TM_ENOERROR;

    while (*strPtr != 0)
    {
        if (!tm_isdigit(*strPtr))
        {
            errorCode = TM_EINVAL;
        }
        digit = (tt8Bit)(*strPtr - '0');
        number = number * 10 + digit;
        strPtr ++;
    }

    *numberPtr = number;
    return errorCode;
}
#endif /* TM_USE_UPNP_DEVICE || TM_USE_SOAP */
