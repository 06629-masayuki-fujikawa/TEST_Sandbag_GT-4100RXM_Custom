/*
 * Filename: trvsprnf.c
 * Author: Emmanuel
 * Date Created: 02/23/1999
 * $Source: source/sockapi/trvsprnf.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2016/04/13 18:16:33JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
/*-
 * Copyright (c) 1992, 1993
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

#ifndef LONG_MAX
#define LONG_MAX      TM_L(2147483647)   /* maximum (signed) long value */
#endif /* LONG_MAX */

static char TM_FAR * __ultoa(u_long val,  char TM_FAR * endp,
                             int    base, int octzero, char TM_FAR * xdigs);

/*
 * Macros for converting digits to letters and vice versa
 */
#define to_digit(c)     ((c) - '0')
#define is_digit(c)     ((unsigned int)to_digit(c) <= 9)
#define to_char(n)      (char)((n) + '0')

/*
 * Convert an unsigned long to ASCII for printf purposes, returning
 * a pointer to the first character of the string representation.
 * Octal numbers can be forced to have a leading zero; hex numbers
 * use the given digits.
 */
static char TM_FAR * __ultoa(u_long val,  char TM_FAR * endp,
                             int    base, int octzero, char TM_FAR * xdigs)
{
    char TM_FAR * cp;
    ttUserS32Bit sval;

    cp = endp;
/*
 * Handle the three cases separately, in the hope of getting
 * better/faster code.
 */
    switch (base)
    {
        case 10:
            if (val < 10)
            { 
/* many numbers are 1 digit */
                *--cp = to_char(val);
                goto ultoaExit;
            }
/*
 * On many machines, unsigned arithmetic is harder than
 * signed arithmetic, so we do at most one unsigned mod and
 * divide; this is sufficient to reduce the range of
 * the incoming value to where signed arithmetic works.
 */
            if (val > LONG_MAX)
            {
                *--cp = to_char(val % 10);
                sval = (ttUserS32Bit)(val / 10);
            }
            else
            {
                sval = (ttUserS32Bit)val;
            }
            do
            {
                *--cp = to_char(sval % 10);
                sval /= 10;
            }
            while (sval != 0);

            break;

        case 8:
            do
            {
                *--cp = to_char(val & 7);
                val >>= 3;
            }
            while (val);

            if (octzero && *cp != '0')
            {
                *--cp = '0';
            }
            break;

        case 16:
            do
            {
                *--cp = xdigs[(int)val & 15];
                val >>= 4;
            }
            while (val);

            break;

        default:
            cp = (char TM_FAR *)0;
            goto ultoaExit;
    }
ultoaExit:
    return (cp);
}

#ifdef BUF
#undef BUF
#endif /* BUF */
#define BUF             68

/*
 * Flags used during conversion.
 */
#define ALT             0x001       /* alternate form */
#define HEXPREFIX       0x002       /* add 0x or 0X prefix */
#define LADJUST         0x004       /* left adjustment */
#define LONGINT         0x008       /* long integer */
#define SHORTINT        0x010       /* short integer */
#define ZEROPAD         0x020       /* zero (as opposed to blank) pad */

/*
 * Choose PADSIZE to trade efficiency vs. size.  If larger printf
 * fields occur frequently, increase PADSIZE and make the initialisers
 * below longer.
 */
#define PADSIZE 16              /* pad chunk size */
static const char TM_FAR blanks[PADSIZE] =
         {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
static const char TM_FAR zeroes[PADSIZE] =
         {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

/*
 * BEWARE, these `goto error' on error, and PAD uses `n'.
 */
#define PRINT(ptr, len)                           \
{                                                 \
    tm_memcpy(buf0 + bufpos, ptr, (unsigned)len); \
    bufpos += (unsigned)len;                      \
}


#define PAD(howmany, with)              \
{                                       \
    n = howmany;                        \
    if( n > 0 )                         \
    {                                   \
        while( n > PADSIZE )            \
        {                               \
            PRINT(with, PADSIZE);       \
            n -= PADSIZE;               \
        }                               \
        PRINT(with, n);                 \
    }                                   \
}

#define FLUSH() buf0[bufpos] = (char)0;
/*
 * To extend shorts properly, we need both signed and unsigned
 * argument extraction methods.
 */
#define SARG() \
        (flags&LONGINT ? va_arg(ap, ttUserS32Bit) : \
            flags&SHORTINT ? (ttUserS32Bit)(short)va_arg(ap, int) : \
            (ttUserS32Bit)va_arg(ap, int))
#define UARG() \
        (flags&LONGINT ? va_arg(ap, u_long) : \
            flags&SHORTINT ? (u_long)(u_short)va_arg(ap, int) : \
            (u_long)va_arg(ap, u_int))


int tfVSPrintF(char TM_FAR * buf0, const char TM_FAR * fmt0, va_list ap)
{
             char TM_FAR *fmt;       /* format string */
             int          ch;        /* character from fmt */
             int          n;         /* handy integer (short term usage) */
/* handy char pointer (short term usage) */
             char TM_FAR *cp;
             char TM_FAR *endp;             
             int          flags;     /* flags as above */
             int          ret;       /* return value accumulator */
             int          width;     /* width from format (%8d), or 0 */
             int          prec;      /* precision from format (%.3d), or -1 */
             char         sign;      /* sign prefix (' ', '+', '-', or \0) */
      ttUser32Bit         ulval;     /* integer arguments %[diouxX] */
             int          base;      /* base for [diouxX] conversion */
/* a copy of prec if [diouxX], 0 otherwise */
             int          dprec;
/* field size expanded by dprec, sign, etc */
             int          realsz;
             int          size;      /* size of converted field or string */
             char TM_FAR  *xdigs;    /* digits for [xX] conversion */
             char TM_FAR  *p;
             char         buf[BUF];  /* space for %c, %[diouxX], %[eEfgG] */
             char         ox[2];     /* space for 0x hex-prefix */
    unsigned int          bufpos;    /* buffer position for PRINT */

    
    bufpos = 0;
    ret = 0;
    fmt = (char TM_FAR *)fmt0;
    xdigs = (char TM_FAR *)0;

    if (buf0 == (char TM_FAR *)0)
    {
        goto noflushExit;
    }

    if (fmt0 == (const char TM_FAR *)0)
    {
        goto done;
    }

/*
 * Scan the format for conversions (`%' character).
 */
    for (;;)
    {
        for(cp = fmt, ch = *fmt; ch != '\0' && ch != '%'; ch = *++fmt);

        n = (int)(fmt - cp);
        if (n != 0)
        {
            PRINT(cp, n);
            ret += n;
        }
        if (ch == '\0')
        {
            goto done;
        }
        fmt++;          /* skip over '%' */

        flags = 0;
        dprec = 0;
        width = 0;
        prec = -1;
        sign = '\0';

rflag:  ch = *fmt++;
reswitch:
        switch (ch)
        {
            case ' ':
/*
 * ``If the space and + flags both appear, the space
 * flag will be ignored.''
 *      -- ANSI X3J11
 */
                if (!sign)
                {
                    sign = ' ';
                }
                goto rflag;
            case '#':
                flags |= ALT;
                goto rflag;
            case '*':
/*
 * ``A negative field width argument is taken as a
 * - flag followed by a positive field width.''
 *      -- ANSI X3J11
 * They don't exclude field widths read from args.
 */
                width = va_arg(ap, int);
                if (width >= 0)
                {
                    goto rflag;
                }
                width = -width;
                        /* FALLTHROUGH */
            case '-':
                flags |= LADJUST;
                goto rflag;
            case '+':
                sign = '+';
                goto rflag;
            case '.':
                ch = *fmt++;
                if( ch == '*' )
                {
                    n = va_arg(ap, int);
                    prec = n < 0 ? -1 : n;
                    goto rflag;
                }
                n = 0;
                while (is_digit(ch))
                {
                    n = 10 * n + to_digit(ch);
                    ch = *fmt++;
                }
                prec = n < 0 ? -1 : n;
                goto reswitch;
            case '0':
/*
 * ``Note that 0 is taken as a flag, not as the
 * beginning of a field width.''
 *      -- ANSI X3J11
 */
                flags |= ZEROPAD;
                goto rflag;
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                n = 0;
                do {
                    n = 10 * n + to_digit(ch);
                    ch = *fmt++;
                } while (is_digit(ch));
                width = n;
                goto reswitch;
#ifdef FLOATING_POINT
            case 'L':
                flags |= LONGDBL;
                goto rflag;
#endif
            case 'h':
                flags |= SHORTINT;
                goto rflag;
            case 'l':
                flags |= LONGINT;
                goto rflag;
            case 'c':
                cp = buf;
                *cp = (char)va_arg(ap, int);
                size = 1;
                sign = '\0';
                break;
            case 'D':
                flags |= LONGINT;
                /*FALLTHROUGH*/
            case 'd':
            case 'i':
                ulval = (ttUser32Bit)SARG();
                if ((ttUserS32Bit)ulval < 0)
                {
                    ulval = (ttUser32Bit)-((ttUserS32Bit)ulval);
                    sign = '-';
                }
                base = 10;
                goto number;
            case 'n':
                if (flags & LONGINT)
                {
                    *va_arg(ap, ttS32BitPtr) = (ttUserS32Bit)ret;
                }
                else
                {
                    if (flags & SHORTINT)
                    {
                        *va_arg(ap, ttS16BitPtr) = (short)ret;
                    }
                    else
                    {
/* PRQA: QAC Message 850: 'TM_FAR' macro is an empty macro in systems */
/* PRQA: QAC Message 850:  other than x86 cpu family */
/* PRQA S 850 L1 */
                        *va_arg(ap, int TM_FAR *) = ret;
/* PRQA L:L1 */
                    }
                }
                continue;       /* no output */
            case 'O':
                flags |= LONGINT;
            /*FALLTHROUGH*/
            case 'o':
                ulval = UARG();
                base = 8;
                goto nosign;
            case 'p':
/*
 * ``The argument shall be a pointer to void.  The
 * value of the pointer is converted to a sequence
 * of printable characters, in an implementation-
 * defined manner.''
 *      -- ANSI X3J11
 */
                ulval = (u_long)
                    (ttUserPtrCastToInt) va_arg(ap, ttVoidPtr);
                base = 16;
                xdigs = "0123456789abcdef";
                flags = flags | HEXPREFIX;
                ch = 'x';
                goto nosign;
            case 's':
                cp = va_arg(ap, ttCharPtr);
                if (cp == (char TM_FAR *)0)
                {
                    cp = "(null)";
                }
                if (prec >= 0)
                {
/*
 * can't use strlen; can only look for the
 * NUL in the first `prec' characters, and
 * strlen() will go further.
 */
/* JNS/8-2-99 BUGFIX 411 */
                    p = (char TM_FAR *) tm_memchr(cp, 0, (unsigned int)prec);

                    if (p != (char *)0)
                    {
                        size = (int)(p - cp);
                        if (size > prec)
                        {
                            size = prec;
                        }
                    }
                    else
                    {
                        size = prec;
                    }
                }
                else
                {
                    size = (int)tm_strlen(cp);
                }
                sign = '\0';
                break;
            case 'U':
                flags |= LONGINT;
                /*FALLTHROUGH*/
            case 'u':
                ulval = UARG();
                base = 10;
                goto nosign;
            case 'X':
                xdigs = "0123456789ABCDEF";
                goto hex;
            case 'x':
                xdigs = "0123456789abcdef";
hex:
                ulval = UARG();
                base = 16;
/* leading 0x/X only if non-zero */
                if (flags & ALT && (ulval != 0))
                {
                    flags |= HEXPREFIX;
                }
/* unsigned conversions */
nosign:         sign = '\0';
/*
 * ``... diouXx conversions ... if a precision is
 * specified, the 0 flag will be ignored.''
 *      -- ANSI X3J11
 */
number:
                dprec = prec;
                if( dprec >= 0 )
                {
                    flags &= ~ZEROPAD;
                }
/*
 * ``The result of converting a zero value with an
 * explicit precision of zero is no characters.''
 *      -- ANSI X3J11
 */
                endp = buf + (BUF - 1);
                endp++;
                if (ulval != 0 || prec != 0)
                {
                    cp = __ultoa(ulval, endp, base, flags & ALT, xdigs);
                }
                size = (int)(endp - cp);
                break;
            default:        /* "%?" prints ?, unless ? is NUL */
                if (ch == '\0')
                {
                    goto done;
                }
/* pretend it was %c with argument ch */
                cp = buf;
                *cp = (char)ch;
                size = 1;
                sign = '\0';
                break;
        }

/*
 * All reasonable formats wind up here.  At this point, `cp'
 * points to a string which (if not flags&LADJUST) should be
 * padded out to `width' places.  If flags&ZEROPAD, it should
 * first be prefixed by any sign or other prefix; otherwise,
 * it should be blank padded before the prefix is emitted.
 * After any left-hand padding and prefixing, emit zeroes
 * required by a decimal [diouxX] precision, then print the
 * string proper, then emit zeroes required by any leftover
 * floating precision; finally, if LADJUST, pad with blanks.
 *
 * Compute actual size, so we know how much to pad.
 * size excludes decimal prec; realsz includes it.
 */
        realsz = dprec > size ? dprec : size;
        if (sign)
        {
            realsz++;
        }
        else
        {
            if (flags & HEXPREFIX)
            {
                realsz += 2;
            }
        }

/* right-adjusting blank padding */
        if ((flags & (LADJUST|ZEROPAD)) == 0)
        {
            PAD((width - realsz), blanks);
        }

/* prefix */
        if (sign)
        {
            PRINT(&sign, 1);
        }
        else
        {
            if (flags & HEXPREFIX)
            {
                ox[0] = '0';
                ox[1] = (char)ch;
                PRINT(ox, 2);
            }
        }

/* right-adjusting zero padding */
        if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
        {
            PAD((width - realsz), zeroes);
        }

/* leading zeroes from decimal precision */
        PAD((dprec - size), zeroes);

/* the string or number proper */
        PRINT(cp, size);

/* left-adjusting padding (always blank) */
        if (flags & LADJUST)
        {
            PAD((width - realsz), blanks);
        }

/* finally, adjust ret */
        ret += width > realsz ? width : realsz;

    }
done:
    FLUSH();
noflushExit:
    return (ret);
/* NOTREACHED */
}
