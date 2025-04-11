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
 * Description: Treck tfBcopy, tfBzero library functions
 *
 * Filename: trcmplib.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trcmplib.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:11JST $
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
#ifdef TM_DSP
#include <trproto.h>
#endif /* TM_DSP */

#ifdef TM_DSP
#include <string.h>
#endif /* TM_DSP */

/* To Copy Data */
void tfBcopy ( const void   TM_FAR * sourcePtr,
               void         TM_FAR * destinationPtr,
               unsigned int length )
{

#ifdef TM_DSP
#ifdef TM_32BIT_DSP
    tt32BitPtr              destLongPtr;
    tt32BitPtr              srcLongPtr;
    unsigned int            index;
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
    tt16BitPtr              destShortPtr;
    tt16BitPtr              srcShortPtr;    
    unsigned int            index;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
    char           TM_FAR * destCharPtr;
    char           TM_FAR * srcCharPtr;
    tt16BitPtr              destShortPtr;
    tt16BitPtr              srcShortPtr;    
    tt32BitPtr              destLongPtr;
    tt32BitPtr              srcLongPtr;
#endif /* !TM_DSP */

    
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
/* No byte or 16 bit word access. Everything is a 32 bit word */
    destLongPtr = (tt32BitPtr) destinationPtr;
    srcLongPtr  = (tt32BitPtr) sourcePtr;
/* 
 * This loop construct will generate more efficient code for the TI compiler 
 * With the proper opimization level selected, one assembly instruction is 
 * generated 
 */
    for (index = 0; index < length; index++ )
    {
        destLongPtr[index] = srcLongPtr[index];
    }
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP

    destShortPtr = (tt16BitPtr) destinationPtr;
    srcShortPtr  = (tt16BitPtr) sourcePtr;

/* 
 * This loop construct will generate more efficient code for the TI compiler 
 * With the proper opimization level selected, one assembly instruction is 
 * generated 
 */
    for (index = 0; index < length; index++ )
    {
        destShortPtr[index] = srcShortPtr[index];
    }
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
    if (    ( (ttUserPtrCastToInt) sourcePtr & TM_ROUND_PTR )
         || ( (ttUserPtrCastToInt) destinationPtr & TM_ROUND_PTR ) )
    {
/* Byte Aligned */ 
        if (    ( (ttUserPtrCastToInt) sourcePtr & TM_UL(1) )
             || ( (ttUserPtrCastToInt) destinationPtr & TM_UL(1) ) )
        {
/* No choice but to copy one byte at a time */
            destCharPtr = (char TM_FAR *)destinationPtr;
            srcCharPtr = (char TM_FAR *)sourcePtr;
            while ( length > 0 )
            {
                *destCharPtr++ = *srcCharPtr++;
                length--;
            }
        }
/* Short Word Aligned */
        else
        {
/* Copy as much as we can short word aligned */
            destShortPtr = (unsigned short TM_FAR *)destinationPtr;
            srcShortPtr = (unsigned short TM_FAR *)sourcePtr;
            while ( length >= 32 * sizeof(unsigned short) )
            {
                destShortPtr[0] = srcShortPtr[0];
                destShortPtr[1] = srcShortPtr[1];
                destShortPtr[2] = srcShortPtr[2];
                destShortPtr[3] = srcShortPtr[3];
                destShortPtr[4] = srcShortPtr[4];
                destShortPtr[5] = srcShortPtr[5];
                destShortPtr[6] = srcShortPtr[6];
                destShortPtr[7] = srcShortPtr[7];
                destShortPtr[8] = srcShortPtr[8];
                destShortPtr[9] = srcShortPtr[9];
                destShortPtr[10] = srcShortPtr[10];
                destShortPtr[11] = srcShortPtr[11];
                destShortPtr[12] = srcShortPtr[12];
                destShortPtr[13] = srcShortPtr[13];
                destShortPtr[14] = srcShortPtr[14];
                destShortPtr[15] = srcShortPtr[15];
                destShortPtr[16] = srcShortPtr[16];
                destShortPtr[17] = srcShortPtr[17];
                destShortPtr[18] = srcShortPtr[18];
                destShortPtr[19] = srcShortPtr[19];
                destShortPtr[20] = srcShortPtr[20];
                destShortPtr[21] = srcShortPtr[21];
                destShortPtr[22] = srcShortPtr[22];
                destShortPtr[23] = srcShortPtr[23];
                destShortPtr[24] = srcShortPtr[24];
                destShortPtr[25] = srcShortPtr[25];
                destShortPtr[26] = srcShortPtr[26];
                destShortPtr[27] = srcShortPtr[27];
                destShortPtr[28] = srcShortPtr[28];
                destShortPtr[29] = srcShortPtr[29];
                destShortPtr[30] = srcShortPtr[30];
                destShortPtr[31] = srcShortPtr[31];
                destShortPtr += 32;
                srcShortPtr += 32;
                length -= 32 * sizeof(unsigned short);
            }
            while ( length >= 8 * sizeof(unsigned short) )
            {
                destShortPtr[0] = srcShortPtr[0];
                destShortPtr[1] = srcShortPtr[1];
                destShortPtr[2] = srcShortPtr[2];
                destShortPtr[3] = srcShortPtr[3];
                destShortPtr[4] = srcShortPtr[4];
                destShortPtr[5] = srcShortPtr[5];
                destShortPtr[6] = srcShortPtr[6];
                destShortPtr[7] = srcShortPtr[7];
                destShortPtr += 8;
                srcShortPtr += 8;
                length -= 8 * sizeof(unsigned short);
            }
            while ( length >= sizeof(unsigned short) )
            {
                *destShortPtr++ = *srcShortPtr++;
                length -= sizeof(unsigned short);
            }        
            destCharPtr = (char TM_FAR *)destShortPtr;
            srcCharPtr = (char TM_FAR *)srcShortPtr;
            while ( length > 0 )
            {
                *destCharPtr++ = *srcCharPtr++;
                length--;
            }
        }
    }
    else
    {
/* Long Word Aligned */
        destLongPtr = (tt32BitPtr) destinationPtr;
        srcLongPtr  = (tt32BitPtr) sourcePtr;
/* Copy as much as we can long word aligned */
        while ( length >= 32 * sizeof(tt32Bit) )
        {
            destLongPtr[0] = srcLongPtr[0];
            destLongPtr[1] = srcLongPtr[1];
            destLongPtr[2] = srcLongPtr[2];
            destLongPtr[3] = srcLongPtr[3];
            destLongPtr[4] = srcLongPtr[4];
            destLongPtr[5] = srcLongPtr[5];
            destLongPtr[6] = srcLongPtr[6];
            destLongPtr[7] = srcLongPtr[7];
            destLongPtr[8] = srcLongPtr[8];
            destLongPtr[9] = srcLongPtr[9];
            destLongPtr[10] = srcLongPtr[10];
            destLongPtr[11] = srcLongPtr[11];
            destLongPtr[12] = srcLongPtr[12];
            destLongPtr[13] = srcLongPtr[13];
            destLongPtr[14] = srcLongPtr[14];
            destLongPtr[15] = srcLongPtr[15];
            destLongPtr[16] = srcLongPtr[16];
            destLongPtr[17] = srcLongPtr[17];
            destLongPtr[18] = srcLongPtr[18];
            destLongPtr[19] = srcLongPtr[19];
            destLongPtr[20] = srcLongPtr[20];
            destLongPtr[21] = srcLongPtr[21];
            destLongPtr[22] = srcLongPtr[22];
            destLongPtr[23] = srcLongPtr[23];
            destLongPtr[24] = srcLongPtr[24];
            destLongPtr[25] = srcLongPtr[25];
            destLongPtr[26] = srcLongPtr[26];
            destLongPtr[27] = srcLongPtr[27];
            destLongPtr[28] = srcLongPtr[28];
            destLongPtr[29] = srcLongPtr[29];
            destLongPtr[30] = srcLongPtr[30];
            destLongPtr[31] = srcLongPtr[31];
            destLongPtr += 32;
            srcLongPtr += 32;
            length -= 32 * sizeof(tt32Bit);
        }
        while ( length >= 8 * sizeof(tt32Bit) )
        {
            destLongPtr[0] = srcLongPtr[0];
            destLongPtr[1] = srcLongPtr[1];
            destLongPtr[2] = srcLongPtr[2];
            destLongPtr[3] = srcLongPtr[3];
            destLongPtr[4] = srcLongPtr[4];
            destLongPtr[5] = srcLongPtr[5];
            destLongPtr[6] = srcLongPtr[6];
            destLongPtr[7] = srcLongPtr[7];
            destLongPtr += 8;
            srcLongPtr += 8;
            length -= 8 * sizeof(tt32Bit);
        }
        while ( length >= 4 * sizeof(tt32Bit) )
        {
            destLongPtr[0] = srcLongPtr[0];
            destLongPtr[1] = srcLongPtr[1];
            destLongPtr[2] = srcLongPtr[2];
            destLongPtr[3] = srcLongPtr[3];
            destLongPtr += 4;
            srcLongPtr += 4;
            length -= 4 * sizeof(tt32Bit);
        }
        while ( length >= sizeof(tt32Bit) )
        {
            *destLongPtr++ = *srcLongPtr++;
            length -= sizeof(tt32Bit);
        }        
        destCharPtr = (char TM_FAR *)destLongPtr;
        srcCharPtr = (char TM_FAR *)srcLongPtr;
        while ( length > 0 )
        {
            *destCharPtr++ = *srcCharPtr++;
            length--;
        }
    }
#endif /* !TM_DSP */
}

/* To Zero a Data Area */
void tfBzero(void TM_FAR * bufferPtr, unsigned int length)
{
#ifdef TM_INFINEON_C166_TEST

  __asm  
  {
    xor  r4,r4              ; Clear R4 & CY
    mov  r1,r8
    mov  r2, r8             ; r2 stores the star buffer adddr
    mov  [r1], r4           ; set the first word

    shr  r10,#1             ; Divide by 2 (leaving number of DWORDS). 
    addc r5, r4             ; R5 = 1, if odd, R5 = 0, if even
                                    ; r10 is now word count
    shr  r10,#1             ; Divide by 2 (leaving number of DWORDS). 
                            ; CY will be set if a single
    jmp  cc_nc MemSet_DWORD ; WORD must be calculated
    
    exts r9,#1              ; Load and set
    mov  [r1+], [r2]

MemSet_DWORD:               ; QWORD must be calculated
    shr  r10,#1                ; Divide by 2 (leaving number of QWORDS). 
                            ; CY will be set if a single
    jmp  cc_nc MemSet_QWORD ; WORD must be calculated

    exts r9,#2              ; Load and set 2 WORDS
    mov  [r1+], [r2]
    mov  [r1+], [r2]

    mov  r10,r10            ; Ensure ZR flag is Set

MemSet_QWORD:
    jmp  cc_nz MemSet_Even

MemSet_QWORDLoop:
    exts r9,#4              ; Load and set
    mov  [r1+], [r2]
    mov  [r1+], [r2]
    mov  [r1+], [r2]
    mov  [r1+], [r2]
    sub  r10,#1
    jmp  cc_nz  MemSet_QWORDLoop

MemSet_Even:
    mov  r5, r5
    jmp cc_nz MemSet_Even
    exts r9, #1
    movb [r1+], [r2]
MemSet_Exit:
  }

#else /* !TM_INFINEON_C166_TEST */

#ifdef TM_DSP
#ifdef TM_32BIT_DSP
    tt32BitPtr            bufULongPtr;
    unsigned int          index;
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
    tt16BitPtr            bufUShortPtr;
    unsigned int          index;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
    tt32BitPtr            bufULongPtr;
    char          TM_FAR *bufCharPtr;
#endif /* !TM_DSP */


#ifdef TM_DSP
#ifdef TM_32BIT_DSP
    bufULongPtr = (tt32BitPtr) bufferPtr; 
/* 
 * This loop construct will generate more efficient code for the TI compiler 
 * With the proper opimization level selected, one assembly instruction is 
 * generated 
 */
    for (index = 0; index < length; index++ )
        bufULongPtr[index] = TM_UL(0);

#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP

    bufUShortPtr = (tt16BitPtr) bufferPtr;
    
    for (index = 0; index < length; index++ )
        bufUShortPtr[index] = 0;    

#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
    bufCharPtr = (char TM_FAR *)bufferPtr;
    
    while (    ( (ttUserPtrCastToInt) bufCharPtr & TM_ROUND_PTR )
            && ( length != 0 ) )
    {
        *bufCharPtr++ = (char)0;
        length--;
    }
    bufULongPtr = (tt32BitPtr)bufCharPtr;
    while ( length >= 32 * sizeof(tt32Bit) )
    {
        bufULongPtr[0] = TM_UL(0);
        bufULongPtr[1] = TM_UL(0);
        bufULongPtr[2] = TM_UL(0);
        bufULongPtr[3] = TM_UL(0);
        bufULongPtr[4] = TM_UL(0);
        bufULongPtr[5] = TM_UL(0);
        bufULongPtr[6] = TM_UL(0);
        bufULongPtr[7] = TM_UL(0);
        bufULongPtr[8] = TM_UL(0);
        bufULongPtr[9] = TM_UL(0);
        bufULongPtr[10] = TM_UL(0);
        bufULongPtr[11] = TM_UL(0);
        bufULongPtr[12] = TM_UL(0);
        bufULongPtr[13] = TM_UL(0);
        bufULongPtr[14] = TM_UL(0);
        bufULongPtr[15] = TM_UL(0);
        bufULongPtr[16] = TM_UL(0);
        bufULongPtr[17] = TM_UL(0);
        bufULongPtr[18] = TM_UL(0);
        bufULongPtr[19] = TM_UL(0);
        bufULongPtr[20] = TM_UL(0);
        bufULongPtr[21] = TM_UL(0);
        bufULongPtr[22] = TM_UL(0);
        bufULongPtr[23] = TM_UL(0);
        bufULongPtr[24] = TM_UL(0);
        bufULongPtr[25] = TM_UL(0);
        bufULongPtr[26] = TM_UL(0);
        bufULongPtr[27] = TM_UL(0);
        bufULongPtr[28] = TM_UL(0);
        bufULongPtr[29] = TM_UL(0);
        bufULongPtr[30] = TM_UL(0);
        bufULongPtr[31] = TM_UL(0);
        bufULongPtr += 32;
        length -= 32 * sizeof(tt32Bit);
    }
    while ( length >= 16 * sizeof(tt32Bit) )
    {
        bufULongPtr[0] = TM_UL(0);
        bufULongPtr[1] = TM_UL(0);
        bufULongPtr[2] = TM_UL(0);
        bufULongPtr[3] = TM_UL(0);
        bufULongPtr[4] = TM_UL(0);
        bufULongPtr[5] = TM_UL(0);
        bufULongPtr[6] = TM_UL(0);
        bufULongPtr[7] = TM_UL(0);
        bufULongPtr[8] = TM_UL(0);
        bufULongPtr[9] = TM_UL(0);
        bufULongPtr[10] = TM_UL(0);
        bufULongPtr[11] = TM_UL(0);
        bufULongPtr[12] = TM_UL(0);
        bufULongPtr[13] = TM_UL(0);
        bufULongPtr[14] = TM_UL(0);
        bufULongPtr[15] = TM_UL(0);
        bufULongPtr += 16;
        length -= 16 * sizeof(tt32Bit);
    }
    while ( length >= 8 * sizeof(tt32Bit) )
    {
        bufULongPtr[0] = TM_UL(0);
        bufULongPtr[1] = TM_UL(0);
        bufULongPtr[2] = TM_UL(0);
        bufULongPtr[3] = TM_UL(0);
        bufULongPtr[4] = TM_UL(0);
        bufULongPtr[5] = TM_UL(0);
        bufULongPtr[6] = TM_UL(0);
        bufULongPtr[7] = TM_UL(0);
        bufULongPtr += 8;
        length -= 8 * sizeof(tt32Bit);
    }
    while ( length >= 4 * sizeof(tt32Bit) )
    {
        bufULongPtr[0] = TM_UL(0);
        bufULongPtr[1] = TM_UL(0);
        bufULongPtr[2] = TM_UL(0);
        bufULongPtr[3] = TM_UL(0);
        bufULongPtr += 4;
        length -= 4 * sizeof(tt32Bit);
    }
    while ( length >= sizeof(tt32Bit) )
    {
        *bufULongPtr++ = TM_UL(0);
        length -= sizeof(tt32Bit);
    }        
    bufCharPtr = (char TM_FAR *)bufULongPtr;
    while ( length != 0 )
    {
        *bufCharPtr++ = (char)0;
        length--;
    }
#endif /* !TM_DSP */    

#endif /* !TM_INFINEON_C166 */
}
