/*
 * Description: Treck vscanf "C" library function
 *
 * $Source: source/sockapi/trvscanf.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:53JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
/*-
 * Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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
 *
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#ifdef TM_LOCK_NEEDED
#include <trproto.h>
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED */

#define BUF             513     /* Maximum length of numeric string. */

/*
 * Flags used during conversion.
 */
#define LONG            0x01    /* l: long or double */
#define SHORT           0x02    /* h: short */
#define SUPPRESS        0x04    /* suppress assignment */
#define POINTER         0x08    /* weird %p pointer (`fake hex') */
#define NOSKIP          0x10    /* do not skip blanks */

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */
#define SIGNOK          0x20    /* +/- is (still) legal */
#define NDIGITS         0x40    /* no digits detected */

#define PFXOK           0x80    /* 0x prefix is (still) legal */
#define NZDIGITS        0x10    /* no zero digits detected */

/*
 * Conversion types.
 */
#define CT_CHAR         0       /* %c conversion */
#define CT_CCL          1       /* %[...] conversion */
#define CT_STRING       2       /* %s conversion */
#define CT_INT          3       /* integer, i.e., strtol or strtoul */

typedef ttUser32Bit ( TM_CODE_FAR * ttStrToXLFuncPtr)(
                                const char          TM_FAR * nptr,
                                      char TM_FAR * TM_FAR * endptr,
                                      int                    base );

static unsigned char TM_FAR * __sccl(          char TM_FAR *tab,
                                      unsigned char TM_FAR *fmt );

static int __collate_range_cmp (int c1, int c2)
{
    int     retCode;

    c1 &= 255;
    c2 &= 255;
    retCode = c1 - c2;
    return retCode;
}

/*
 * Moved from stack to static area (too big for embedded).
 * Because of this, added lock to make tfVSScanF reentrant.
 */

/* buffer for numeric conversions */
static  char TM_GLOBAL_QLF buf[BUF];

/* character class table for %[...] */
static  char TM_GLOBAL_QLF ccltab[256];

int tfVSScanF(const char TM_FAR * buf0, char const TM_FAR * fmt0, va_list ap)
{
    unsigned char TM_FAR *fmt;
    /* character from format, or conversion */
             int          c;
    unsigned int          width;        /* field width, or 0 */
    /* points into all kinds of strings */
             char TM_FAR *p;
             int          n;            /* handy integer */
             int          flags;        /* flags as defined above */
             /* number of fields assigned */
             int          nassigned;
             int          nconversions; /* number of conversions */
             /* base argument to strtol/strtoul */
             int          base;
    unsigned int          decal;
             /* conversion function (strtol/strtoul) */
    ttStrToXLFuncPtr      ccfn;
    unsigned int          bufpos;
             char TM_FAR *psz;
    unsigned int          len;
    ttUser32Bit           res;

/* `basefix' is used to avoid `if' tests in the integer scanner */
static short TM_FAR basefix[17] =
            { 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

    fmt = (unsigned char TM_FAR *)fmt0;
    nassigned = 0;
    nconversions = 0;
    base = 0;
    bufpos = 0;
    ccfn = (ttStrToXLFuncPtr)tfStrToL;
    tm_call_lock_wait(&tm_context(tvScanfLockEntry));
    for (;;)
    {
        c = *fmt++;
        if (c == 0)
        {
            break; /* return */
        }
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
        if (tm_isspace(c))
/* PRQA L:L1 */
        {
            for (;;)
            {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                if (!tm_isspace(buf0[bufpos]))
/* PRQA L:L1 */
                {
                    break;
                }
                bufpos++;
            }
            continue;
        }
        if (c != '%')
        {
            goto literal;
        }
        width = 0;
        flags = 0;
/*
 * switch on the format.  continue if done;
 * break once format type is derived.
 */
again:
        c = *fmt++;
        switch (c)
        {
            case '%':
literal:
                if (buf0[bufpos] != c)
                {
                    goto matchFailure;
                }
                bufpos++;
                continue;

            case '*':
                flags |= SUPPRESS;
                goto again;
            case 'l':
                flags |= LONG;
                goto again;
            case 'h':
                flags |= SHORT;
                goto again;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                width = width * 10 + (unsigned)c - '0';
                goto again;

/*
 * Conversions.
 * Those marked `compat' are for 4.[123]BSD compatibility.
 *
 * (According to ANSI, E and X formats are supposed
 * to the same as e and x.  Sorry about that.)
 */
            case 'D':       /* compat */
                flags |= LONG;
                /* FALLTHROUGH */
            case 'd':
                c = CT_INT;
                base = 10;
                break;
    
            case 'i':
                c = CT_INT;
                base = 0;
                break;

            case 'O':       /* compat */
                flags |= LONG;
                /* FALLTHROUGH */
            case 'o':
                c = CT_INT;
                ccfn = (ttStrToXLFuncPtr)tfStrToUl;
                base = 8;
                break;

            case 'u':
                c = CT_INT;
                ccfn = (ttStrToXLFuncPtr)tfStrToUl;
                base = 10;
                break;

            case 'X':       /* compat   XXX */
                flags |= LONG;
                /* FALLTHROUGH */
            case 'x':
                flags |= PFXOK; /* enable 0x prefixing */
                c = CT_INT;
                ccfn = (ttStrToXLFuncPtr)tfStrToUl;
                base = 16;
                break;
        
            case 's':
                c = CT_STRING;
                break;

            case '[':
                fmt = __sccl(ccltab, fmt);
                flags |= NOSKIP;
                c = CT_CCL;
                break;
    
            case 'c':
                flags |= NOSKIP;
                c = CT_CHAR;
                break;

            case 'p':       /* pointer format is like hex */
                flags |= POINTER | PFXOK;
                c = CT_INT;
                ccfn = (ttStrToXLFuncPtr)tfStrToUl;
                base = 16;
                break;
    
            case 'n':
                nconversions++;
                if (flags & SUPPRESS)   /* ??? */
                {
                    continue;
                }
                if (flags & SHORT)
                {
                    *va_arg(ap, ttS16BitPtr) = (short)bufpos;
                }
                else
                {
                    if (flags & LONG)
                    {
                        *va_arg(ap, ttS32BitPtr) =
                            (ttUserS32Bit)bufpos;
                    }
                    else
                    {
/* PRQA: QAC Message 850: 'TM_FAR' macro is an empty macro in systems */
/* PRQA: QAC Message 850:  other than x86 cpu family */
/* PRQA S 850 L1 */
                        *va_arg(ap, int TM_FAR *) = (int)bufpos;
/* PRQA L:L1 */
                    }
                    continue;
                }
    
/*
 * Disgusting backwards compatibility hacks.    XXX
 */
            case '\0':      /* compat */
                nassigned = -1;
                goto matchFailure; /* return */
    
            default:        /* compat */
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                if (tm_isupper(c))
/* PRQA L:L1 */
                {
                    flags |= LONG;
                }
                c = CT_INT;
                base = 10;
                break;
        }

/*
 * Consume leading white space, except for formats
 * that suppress this.
 */
        if ((flags & NOSKIP) == 0)
        {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
            while (tm_isspace(buf0[bufpos]))
/* PRQA L:L1 */
            {
                bufpos++;
            }
/*
 * Note that there is at least one character in
 * the buffer, so conversions that do not set NOSKIP
 * ca no longer result in an input failure.
 */
        }

/*
 * Do the conversion.
 */
        switch (c)
        {
            case CT_CHAR:
/* scan arbitrary characters (sets NOSKIP) */
                if (width == 0)
                {
                    width = 1;
                }
                if (flags & SUPPRESS)
                {
                    bufpos += width;
                }
                else
                {
                    psz = va_arg(ap, ttCharPtr);
                    len = (unsigned int) tm_strlen(&buf0[bufpos]);
                    if (len >= width)
                    {
                        len = width;
                    }
                    tm_memcpy(psz, &buf0[bufpos], len);
                    psz[len] = '\0';
                    bufpos += len;
                    nassigned++;
                }
                nconversions++;
                break;

            case CT_CCL:
/* scan a (nonempty) character class (sets NOSKIP) */
                if (width == 0)
                {
                    width = ~(unsigned int)0;       /* `infinity' */
                }
/* take only those things in the class */
                n = 0;
                if (flags & SUPPRESS)
                {
                    while (ccltab[(unsigned) (buf0[bufpos + (unsigned)n])])
                    {
                        n++;
                        width--;
                        if( width == 0 )
                        {
                            break;
                        }
                    }
                    if (n == 0)
                    {
                        goto matchFailure;
                    }
                }
                else
                {
                    p = va_arg(ap, ttCharPtr);
                    while (ccltab[(unsigned) (buf0[bufpos + (unsigned)n])])
                    {
                        n++;
                        p++[0] = buf0[bufpos + (unsigned)n];
                        width--;
                        if( width == 0 )
                        {
                            break;
                        }
                    }
                    p[0] = '\0';
                    if (n == 0)
                    {
                        goto matchFailure;
                    }
                    nassigned++;
                }
                bufpos += (unsigned)n;
                nconversions++;
                break;

            case CT_STRING:
/* like CCL, but zero-length string OK, & no NOSKIP */
                if (width == 0)
                {
                    width = ~(unsigned int)0;
                }
                if (flags & SUPPRESS)
                {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                    while (!tm_isspace(buf0[bufpos]))
/* PRQA L:L1 */
                    {
                        bufpos++;
                        width--;
                        if( width == 0 )
                        {
                            break;
                        }
                    }
                }
                else
                {
                    p = va_arg(ap, ttCharPtr);
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                    while (!tm_isspace(buf0[bufpos]))
/* PRQA L:L1 */
                    {
                        *p++ = buf0[bufpos++];
                        width--;
                        if( width == 0 )
                        {
                            break;
                        }
                    }
                    *p = 0;
                    nassigned++;
                }
                nconversions++;
                continue;

            case CT_INT:
/* scan an integer as if by strtol/strtoul */
/* size_t is unsigned, hence this optimisation */
                width--;
                if( width > sizeof(buf) - 2 )
                {
                    width = sizeof(buf) - 2;
                }
                width++;
                flags |= SIGNOK | NDIGITS | NZDIGITS;
                decal = 0;

                for (p = buf; width; width--)
                {
                    c = buf0[bufpos + decal];
/*
 * Switch on the character; `goto ok'
 * if we accept it as a part of number.
 */
                    switch (c)
                    {

/*
 * The digit 0 is always legal, but is
 * special.  For %i conversions, if no
 * digits (zero or nonzero) have been
 * scanned (only signs), we will have
 * base==0.  In that case, we should set
 * it to 8 and enable 0x prefixing.
 * Also, if we have not scanned zero digits
 * before this, do not turn off prefixing
 * (someone else will turn it off if we
 * have scanned any nonzero digits).
 */
                    case '0':
                        if (base == 0)
                        {
                            base = 8;
                            flags |= PFXOK;
                        }
                        if (flags & NZDIGITS)
                        {
                            flags &= ~(SIGNOK|NZDIGITS|NDIGITS);
                        }
                        else
                        {
                            flags &= ~(SIGNOK|PFXOK|NDIGITS);
                        }
                        goto ok;

/* 1 through 7 always legal */
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                        base = basefix[base];
                        flags &= ~(SIGNOK | PFXOK | NDIGITS);
                        goto ok;

/* digits 8 and 9 ok iff decimal or hex */
                    case '8':
                    case '9':
                        base = basefix[base];
                        if (base <= 8)
                        {
                            break;  /* not legal here */
                        }
                        flags &= ~(SIGNOK | PFXOK | NDIGITS);
                        goto ok;

/* letters ok iff hex */
                    case 'A':
                    case 'B':
                    case 'C':
                    case 'D':
                    case 'E':
                    case 'F':
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                    case 'f':

/* no need to fix base here */
                        if (base <= 10)
                        {
                            break;  /* not legal here */
                        }
                        flags &= ~(SIGNOK | PFXOK | NDIGITS);
                        goto ok;

/* sign ok only as first character */
                    case '+':
                    case '-':
                        if (flags & SIGNOK)
                        {
                            flags &= ~SIGNOK;
                            goto ok;
                        }
                        break;

/* x ok iff flag still set & 2nd char */
                    case 'x':
                    case 'X':
                        if (flags & PFXOK && p == (char TM_FAR *)(buf + 1))
                        {
                            base = 16;      /* if %i */
                            flags &= ~PFXOK;
                            goto ok;
                        }
                        break;
                    default:
                        break;
                    }

/*
 * If we got here, c is not a legal character
 * for a number.  Stop accumulating digits.
 */
                    break;
ok:
/*
 * c is legal: store it and look at the next.
 */
                    *p++ = (char)c;
                    decal++;
                }
/*
 * If we had only a sign, it is no good; push
 * back the sign.  If the number ends in `x',
 * it was [sign] '0' 'x', so push back the x
 * and treat it as [sign] '0'.
 */
                if (flags & NDIGITS)
                {
                    if (p > (char TM_FAR *)buf)
                    {
                        --p;
                    }
                    goto matchFailure;
                }
                c = ((unsigned char TM_FAR *)p)[-1];
                if (c == 'x' || c == 'X')
                {
                    --p;
                }
                if ((flags & SUPPRESS) == 0)
                {

                    *p = 0;
                    res = (*ccfn)((char TM_FAR *)buf,
                                  (char TM_FAR * TM_FAR *)0,
                                  base);
                    if (flags & POINTER)
                    {
                        *va_arg(ap, ttVoidPtrPtr) =
                            (ttVoidPtr)(ttUserPtrCastToInt)res;
                    }
                    else
                    {
                        if (flags & SHORT)
                        {
                            *va_arg(ap, ttS16BitPtr) = (short)res;
                        }
                        else
                        {
                            if (flags & LONG)
                            {
                                *va_arg(ap, ttS32BitPtr) =
                                    (ttUserS32Bit)res;
                            }
                            else
                            {
/* PRQA: QAC Message 850: 'TM_FAR' macro is an empty macro in systems */
/* PRQA: QAC Message 850:  other than x86 cpu family */
/* PRQA S 850 L1 */
                                *va_arg(ap, int TM_FAR *) = (int)res;
/* PRQA L:L1 */
                            }
                        }
                    }
                    nassigned++;
                }
                bufpos += (unsigned)(p - (char TM_FAR *)buf);
                nconversions++;
                break;

            default:
                break;
        }
        if (nconversions == 0)
        {
            nassigned = -1;
            break; /* return */
        }
    }
matchFailure:
    tm_call_unlock(&tm_context(tvScanfLockEntry));
    return (nassigned);
}

/*
 * Fill in the given table from the scanset at the given format
 * (just after `[').  Return a pointer to the character past the
 * closing `]'.  The table has a 1 wherever characters should be
 * considered part of the scanset.
 */
static unsigned char TM_FAR * __sccl(          char TM_FAR *tab,
                                      unsigned char TM_FAR *fmt )
{
    unsigned char TM_FAR * fmtReturn;
             int           c;
             int           n;
             int           v;
             int           i;

        /* first `clear' the whole table */
    c = *fmt++;             /* first char hat => negated scanset */
    if (c == '^')
    {
        v = 1;          /* default => accept */
        c = *fmt++;     /* get new first char */
    }
    else
    {
        v = 0;          /* default => reject */
    }
    tm_memset(tab, v, 256);
    if (c == 0)
    {
        /* format ended before closing ] */
        fmtReturn = fmt - 1;
        goto scclExit;
    }
    /*
     * Now set the entries corresponding to the actual scanset
     * to the opposite of the above.
     *
     * The first character may be ']' (or '-') without being special;
     * the last character may be '-'.
     */
    v = 1 - v;
    for (;;)
    {
        tab[c] = (char)v;               /* take character c */
doswitch:
        n = *fmt++;             /* and examine the next */
        switch (n)
        {

            case 0:                 /* format ended too soon */
                fmtReturn = fmt - 1;
                goto scclExit;

            case '-':
                /*
                 * A scanset of the form
                 *      [01+-]
                 * is defined as `the digit 0, the digit 1,
                 * the character +, the character -', but
                 * the effect of a scanset such as
                 *      [a-zA-Z0-9]
                 * is implementation defined.  The V7 Unix
                 * scanf treats `a-z' as `the letters a through
                 * z', but treats `a-a' as `the letter a, the
                 * character -, and the letter a'.
                 *
                 * For compatibility, the `-' is not considerd
                 * to define a range if the character following
                 * it is either a close bracket (required by ANSI)
                 * or is not numerically greater than the character
                 * we just stored in the table (c).
                 */
                n = *fmt;
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                if (n == ']' || __collate_range_cmp (n, c) < 0)
/* PRQA L:L1 */
                {
                    c = '-';
                    break;  /* resume the for(;;) */
                }
                fmt++;
                /* fill in the range */
                for (i = 0; i < 256; i ++)
                {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                    if (   __collate_range_cmp (c, i) < 0
                        && __collate_range_cmp (i, n) <= 0
                       )
/* PRQA L:L1 */
                    {
                        tab[i] = (char)v;
                    }
                }
#if 1   /* XXX another disgusting compatibility hack */
                c = n;
                /*
                 * Alas, the V7 Unix scanf also treats formats
                 * such as [a-c-e] as `the letters a through e'.
                 * This too is permitted by the standard....
                 */
                goto doswitch;
#else
                c = *fmt++;
                if (c == 0)
                {
                    fmtReturn = fmt - 1;
                    goto scclExit;
                }
                if (c == ']')
                {
                    fmtReturn = fmt;
                    goto scclExit;
                }
                break;
#endif

            case ']':               /* end of scanset */
                fmtReturn = fmt;
                goto scclExit;

            default:                /* just another character */
                c = n;
                break;
        }
    }
scclExit:
    return fmtReturn;
}
