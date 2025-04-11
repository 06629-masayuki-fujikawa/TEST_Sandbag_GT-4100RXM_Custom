/*
 * Description: Treck qsort "C" library function
 *
 * $Source: source/sockapi/trqsort.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:58JST $
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



static char TM_FAR * med3( char TM_FAR * a,
                           char TM_FAR * b,
                           char TM_FAR * c,
                           ttCmpFuncPtr  cmpFuncPtr );

static void swapfunc( char TM_FAR * a, 
                      char TM_FAR * b, 
                      unsigned int  n, 
                      int           swaptype );

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */

#define SWAPINIT(a, es) \
    swaptype=(int)(((ttUserPtrCastToInt)(a)) % sizeof(ttS32Bit)||\
                es % sizeof(ttS32Bit) ? 2 : es == sizeof(ttS32Bit)? 0 : 1);

#define swap(a, b)                                          \
{                                                           \
    if (swaptype == 0)                                      \
    {                                                       \
        swapTempLong = *(ttS32Bit TM_FAR *)(a);             \
        *(ttS32Bit TM_FAR *)(a) = *(ttS32Bit TM_FAR *)(b);  \
        *(ttS32Bit TM_FAR *)(b) = swapTempLong;             \
    }                                                       \
    else                                                    \
    {                                                       \
        swapfunc(a, b, es, swaptype);                       \
    }                                                       \
}

#define vecswap(a, b, n, r)        if ((r) > 0) swapfunc(a, b, n, swaptype)

static char TM_FAR *
med3 ( char TM_FAR * a,
       char TM_FAR * b,
       char TM_FAR * c,
       ttCmpFuncPtr  cmpFuncPtr )
{
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
    return (*cmpFuncPtr)(a, b) < 0 ?
           ((*cmpFuncPtr)(b, c) < 0 ? b : ((*cmpFuncPtr)(a, c) < 0 ? c : a ))
          :((*cmpFuncPtr)(b, c) > 0 ? b : ((*cmpFuncPtr)(a, c) < 0 ? a : c ));
/* PRQA L:L1 */
}

static void
swapfunc ( char TM_FAR * a, char TM_FAR * b, unsigned int n, int swaptype )
{
    register char TM_FAR *     pa;
    register char TM_FAR *     pb;
    register ttS32Bit TM_FAR * pal;
    register ttS32Bit TM_FAR * pbl;
             ttS32Bit          i;
             ttS32Bit          tl;
             char              tc;

    if (swaptype <= 1)
    {
        pal = (ttS32Bit TM_FAR *)a;
        pbl = (ttS32Bit TM_FAR *)b;
        i = (ttS32Bit)(n / sizeof (ttS32Bit));
        do
        {
            tl = *pal;
            *pal++ = *pbl;
            *pbl++ = tl;
            i--;
        } while (i > 0);
    }
    else
    {
        pa = (char TM_FAR *)a;
        pb = (char TM_FAR *)b;
        i = (ttS32Bit)n;
        do
        {
            tc = *pa;
            *pa++ = *pb;
            *pb++ = tc;
            i--;
        } while (i > 0);
    }
}

void tfQSort ( void TM_FAR * a,
               unsigned int  n,
               unsigned int  es,
               ttCmpFuncPtr  cmpFuncPtr )
{
    char TM_FAR *pa;
    char TM_FAR *pb;
    char TM_FAR *pc;
    char TM_FAR *pd;
    char TM_FAR *pl;
    char TM_FAR *pm;
    char TM_FAR *pn;
    ttS32Bit    swapTempLong;
    int         d;
    int         r;
    int         swaptype;
    int         swap_cnt;

    if ( (a != (void TM_FAR *)0) && (cmpFuncPtr != (ttCmpFuncPtr)0) )
    {
loop:
        SWAPINIT(a, es);
        swap_cnt = 0;
        if (n < 7)
        {
            for ( pm = (char TM_FAR *)a + es; pm < (char TM_FAR *)a + n * es;
                  pm += es )
            {
                pl = pm;
                r = (*cmpFuncPtr)(pl - es, pl);
                for ( ; pl > (char TM_FAR *)a && r > 0; pl -= es)
                {
                    swap(pl, pl - es);
                    r = (*cmpFuncPtr)(pl - es, pl);
                }
            }
            goto qsortExit; /* return */
        }
        pm = (char TM_FAR *)a + (n / 2) * es;
        if (n > 7)
        {
/* JNS/8-2-99 BUGFIX 411 */                
            pl = (char TM_FAR *) a;
            pn = (char TM_FAR *)a + (n - 1) * es;
            if (n > 40)
            {
                d = (int)((n / 8) * es);
                pl = med3(pl, pl + d, pl + 2 * d, cmpFuncPtr);
                pm = med3(pm - d, pm, pm + d, cmpFuncPtr);
                pn = med3(pn - 2 * d, pn - d, pn, cmpFuncPtr);
            }
            pm = med3(pl, pm, pn, cmpFuncPtr);
        }
/* JNS/8-2-99 BUGFIX 411 */        
        swap((ttCharPtr) a, pm);
        pa = pb = (char TM_FAR *)a + es;

        pc = pd = (char TM_FAR *)a + (n - 1) * es;
        for (;;)
        {
            while (pb <= pc)
            {
                r = (*cmpFuncPtr)(pb, a);
                if (r > 0)
                {
                    break;
                }

                if (r == 0)
                {
                    swap_cnt = 1;
                    swap(pa, pb);
                    pa += es;
                }
                pb += es;
            }
            while (pb <= pc)
            {
                r = (*cmpFuncPtr)(pc, a);
                if (r < 0)
                {
                    break;
                }

                if (r == 0)
                {
                    swap_cnt = 1;
                    swap(pc, pd);
                    pd -= es;
                }
                pc -= es;
            }
            if (pb > pc)
            {
                break;
            }
            swap(pb, pc);
            swap_cnt = 1;
            pb += es;
            pc -= es;
        }
        if (swap_cnt == 0)
        {  /* Switch to insertion sort */
            for ( pm = (char TM_FAR *)a + es; pm < (char TM_FAR *)a + n * es;
                  pm += es)
            {
                pl = pm;
                r = (*cmpFuncPtr)(pl - es, pl);
                for ( ; pl > (char TM_FAR *)a && r > 0; pl -= es)
                {
                    swap(pl, pl - es);
                    r = (*cmpFuncPtr)(pl - es, pl);
                }
            }
            goto qsortExit; /* return */
        }

        pn = (char TM_FAR *)a + n * es;
        r = tm_min((int)(ttUserPtrCastToInt)(pa - (ttCharPtr)a),
                   (int)(ttUserPtrCastToInt)(pb - pa));
/* JNS/8-2-99 BUGFIX 411 */                
        vecswap((ttCharPtr)a, (ttCharPtr)(pb - r), (unsigned)r, r);
        r = tm_min((int)(ttUserPtrCastToInt)(pd - pc),
                   (int)((ttUserPtrCastToInt)(pn - pd) - es));
        vecswap(pb, (ttCharPtr)(pn - r), (unsigned)r, r);
        r = (int)(ttUserPtrCastToInt)(pb - pa);
        if (r > (int)es)
        {
            tfQSort(a, (unsigned)r / es, es, cmpFuncPtr);
        }
        r = (int)(ttUserPtrCastToInt)(pd - pc);
        if (r > (int)es)
        {
        /* Iterate rather than recurse to save stack space */
            a = pn - r;
            n = (unsigned)r / es;
            goto loop;
        }
    }
qsortExit:
    return;
}
