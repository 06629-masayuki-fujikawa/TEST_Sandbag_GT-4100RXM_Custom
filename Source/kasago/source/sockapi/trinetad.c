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
 * Description: BSD Sockets Interface (inet_addr, and inet_aton)
 *
 * Filename: trinetad.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trinetad.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:34JST $
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

/* Common code for inet_aton() and inet_addr() */
static ttUser32Bit tfInetAtoN(char TM_FAR *strPtr, int badRetCode);


static const int  TM_CONST_QLF tlPower10[4]={1,10,100,1};
/*
 * Convert from dotted string to unsigned long 
 * The IP address is stored in network byte order
 * On failure, return (unsigned long)-1.
 */
ttUser32Bit inet_addr(char TM_FAR *strPtr)
{
    return tfInetAtoN(strPtr, TM_SOCKET_ERROR);
}

/*
 * Convert from dotted string to unsigned long 
 * The IP address is stored in network byte order
 * On failure, return (unsigned long)0.
 */
ttUser32Bit inet_aton(char TM_FAR *strPtr)
{
    return tfInetAtoN(strPtr, 0);
}

/*
 * Called from inet_addr() or inet_aton().
 * Convert from dotted string to unsigned long 
 * The IP address is stored in network byte order
 * On failure, return (unsigned long)badRetcode.
 */
static ttUser32Bit tfInetAtoN(char TM_FAR *strPtr, int badRetCode)
{
    int                   value[4];
    int                   strValIndex;
    int                   valueIndex;
    int                   errorCode;
    ttUser32Bit           retAddr;
#ifdef TM_DSP
    ttUser32Bit           addrVal;
#else /* !TMS320_C3_C5 */
    unsigned char TM_FAR *retAddrPtr;
#endif /* TM_DSP */    
    
    strValIndex=0;
    valueIndex=0;
    value[0] = -1;
    value[1] = -1;
    value[2] = -1;
    value[3] = -1;
/* Loop until we hit the end of the string */
    while (*strPtr != '\0')
    {
        if ( ( (*strPtr >='0') && (*strPtr <='9') ) || (*strPtr=='.') )
        {
            if (*strPtr=='.')
            {
/* Shift the value to right (base10) by the number of digits missing */
                value[valueIndex] /= tlPower10[3-strValIndex];
/* Dots cause us to go to the next value */
                strValIndex=0;
                valueIndex++;
                if( valueIndex > 3 )
                {
/* Check for too many DOTS */
                    value[0] = -1;
                    break;
                }
            }
            else
            {    
                if (strValIndex > 2)
                {
/* Check for too many digits (we only support 3 per number) */
                    value[0] = -1;
                    break;
                }
                if (value[valueIndex] == -1)
                {
/* First time we are adding into this value so zero it */
                    value[valueIndex]=0;
                }
                value[valueIndex] +=   (*strPtr-'0')
                                     * (tlPower10[(2-strValIndex)]);
                ++strValIndex;
            }
        }        
        else
        {
/* Check for leading/trailing spaces */
            if (*strPtr == ' ')
            {
/* Leading Space check */
                if ((strValIndex != 0) || (valueIndex  != 0))
                {
/* Trailing space check */
                    if((valueIndex == 3) && (strValIndex !=0))
                    {
/* Trailing spaces cause us to drop out of the loop */
                        break;
                    }
                    else
                    {
/* Somewhere in the middle so it is an error */
                        value[0] = -1;
                        break;   
                    }
                }
/* They both were zero so it must be a leading space, so ignore it */
            }
/* All other characters cause an error */
            else
            {
                value[0] = -1;
                break;
            }
        }
/* Look at the next character */
        ++strPtr;
    }    
    if (strValIndex < 4)
    {
/* Shift the value to right (base10) by the number of digits missing */
        value[valueIndex] /= tlPower10[3-strValIndex];
    }
    errorCode=TM_ENOERROR;
/* We are going to write the address in network byte order in the long */
#ifdef TM_DSP
    retAddr = 0;
#else /* !TM_DSP */
    retAddrPtr=(unsigned char TM_FAR *)&retAddr;
#endif /* TM_DSP */
    for (valueIndex=0; valueIndex<4; valueIndex++)
    {
/* Make sure the number was set and that it is not too large */
        if (value[valueIndex] > 255 || value[valueIndex] == -1)
        {
/* Gice up on any value beign out of range */
            errorCode=TM_EINVAL;
            break;
        }
/* Save each value as a unsigned char */
#ifdef TM_DSP 
/* Need to shift 24, 16, 8, and 0 bits */
        addrVal = (ttUser32Bit) (value[valueIndex] & 0xff);
        retAddr |= addrVal << ((unsigned int)(3 - valueIndex) << 3);   
#else /* !TM_DSP */
        *retAddrPtr++ = (unsigned char)(value[valueIndex]);
#endif /* TM_DSP */
    }    
    if (errorCode==TM_EINVAL)
    {
/* If we had an error, return Error Code */
        retAddr = (ttUser32Bit)(badRetCode);
    }
    return (retAddr);
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4InetadDummy = 0;
#endif /* ! TM_USE_IPV4 */
