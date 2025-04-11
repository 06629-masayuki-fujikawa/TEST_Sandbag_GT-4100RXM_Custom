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
 * Description: BSD Sockets Interface (tfInetToAscii)
 *
 * Filename: trnetasc.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trnetasc.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:50JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>

/*
 * Convert an IP address to a dotted string 
 * The input must be in network byte order
 */

void tfInetToAscii(ttUser32Bit      ipAddress,
                   char    TM_FAR * outputIpAddressString)
{
   char          TM_FAR *outDataPtr;
   unsigned int          value;
   unsigned int          charValue;
   int                   index;
   tt8Bit                leadingZeroDigit;
#ifndef TM_DSP
   unsigned char TM_FAR *inputAddrPtr;
      
   inputAddrPtr=(unsigned char TM_FAR *)&ipAddress;
#endif /* TM_DSP */   
   outDataPtr=outputIpAddressString;
   for (index=0; index < (int)(tm_byte_count(sizeof(tt4IpAddress))); index++)
   {  
/* Assume leading digit is zero */
       leadingZeroDigit = TM_8BIT_YES;
#ifdef TM_DSP
       value = (unsigned int)((ipAddress >> (24 - (index << 3))) & 0x0ff);
#else /* !TM_DSP */
       value = (unsigned int)*inputAddrPtr;
       inputAddrPtr++;
#endif /* TM_DSP */
       charValue = value/100;
       if (charValue != 0)
/* Do not output leading zero digit */
       {
           *outDataPtr++ = (char)((unsigned char) (charValue) + '0');
           value -= charValue * 100;
           leadingZeroDigit = TM_8BIT_ZERO; /* leading digit is not zero */
       }
       charValue = value/10;
       if ( (charValue != 0) || (leadingZeroDigit == TM_8BIT_ZERO) )
/*
 * Output middle digit if it is non zero, or if first leading digit was not
 * a zero digit.
 */
       {
           *outDataPtr++ = (char)((unsigned char) (charValue) + '0');
           if (charValue != 0)
           {
               value -= ((unsigned int) charValue)*10;
           }
       }
       charValue = value;
/* Always output last digit */
       *outDataPtr++ = (char)((unsigned char)charValue + '0');
       *outDataPtr++ = '.';
   }   
   *(outDataPtr-1) = '\0'; 


   return; 
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4NetascDummy = 0;
#endif /* ! TM_USE_IPV4 */
