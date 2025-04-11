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
 * Description:     Common DNS functions
 * Filename:        trdnscmn.c
 * Author:          Jared
 * Date Created:    4/22/07
 * $Source: source/trdnscmn.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:17JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h> 
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


/*
 * tfDnsLabelToAscii
 * 
 * Converts a DNS label formatted string to ASCII representation.  Assumes that
 * all pointer parameters are valid.
 *
 * Parameter    Description
 * labelPtr     DNS label formatted string to convert    
 * asciiPtr     Buffer to place ASCII formatted string
 * pktDataPtr   Pointer to beginning fo DNS packet (needed for decompression)
 *              If null, compression isn't allowed for this label
 * labelType    TM_DNS_LABEL_PART if the input is a partial name,
 *              TM_DNS_LABEL_FULL if it's a full name
 * labelLen     Length of the label. This is only necessary if labelType
 *              is TM_DNS_LABEL_PART.
 *
 * Returns
 * Value           Meaning
 * errorCode       Success or failure
 *
*/
int tfDnsLabelToAscii( tt8BitPtr labelPtr,
                       tt8BitPtr asciiPtr,
                       tt8BitPtr pktDataPtr,
                       tt8Bit    labelType,
                       int       labelLen)
{
    int     errorCode;
    tt16Bit inputIdx;
    tt16Bit outputIdx;
    tt8Bit  charCount;

    errorCode = TM_ENOERROR;
    charCount = 0;
    inputIdx  = 0;
    outputIdx = 0;

    while (labelPtr[inputIdx] != 0)
    {
        if (charCount > 0)
        {
            if (   (tm_isalnum(labelPtr[inputIdx]) == 0)
                && (labelPtr[inputIdx] != '-') )
            {
                errorCode = TM_EINVAL;
                break;
            }
            asciiPtr[outputIdx++] = labelPtr[inputIdx++];
            charCount--;
        }
        else
        {
/* If this is a partial label... */
            if (labelType == TM_DNS_LABEL_PART)
            {
/* ... And we've reached the end of the input, exit */
                if ((int)inputIdx >= labelLen)
                {
                    break;
                }
            }
/* Check if the label length is actually a pointer */
            if (labelPtr[inputIdx] & TM_DNS_POINTER_MASK)
            {
                if (pktDataPtr != (tt8BitPtr)0)
                {
/* If it's a pointer, the lower 14 bits are the offset */
                    labelPtr =   pktDataPtr
                               + ((labelPtr[inputIdx]
                                   & ~TM_DNS_POINTER_MASK) << 8)
                               + labelPtr[inputIdx + 1];
                    inputIdx = 0;
                }
                else
                {
/* If pktDataPtr is 0, compression isn't allowed */
                    errorCode = TM_EINVAL;
                    break;
                }
            }
            else
            {
                charCount = labelPtr[inputIdx];
                inputIdx++;

/* Domain cannot start or end with a hyphen */
                if (   (labelPtr[inputIdx] == '-')
                    || (labelPtr[inputIdx+charCount-1] == '-') )
                {
                    errorCode = TM_EINVAL;
                    break;
                }

                if (outputIdx > 0)
                {
                    asciiPtr[outputIdx++] = '.';
                } 
            }
        }
    }

    asciiPtr[outputIdx] = 0;

    return errorCode;
}

/*
 * tfDnsLabelAsciiLen
 * 
 * Gives the ASCII length of a DNS label (without doing the translation).
 * Length includes null-terminator.
 *
 * Parameter    Description
 * labelPtr     DNS label formatted string to convert    
 *
 * Returns
 * Value           Meaning
 * length          Length of new ASCII string 
 *
 */
int tfDnsLabelAsciiLen( tt8BitPtr labelPtr )
{
    int charCount;

    charCount  = 0;

    while (labelPtr[charCount] != 0)
    {
        charCount += ((int)labelPtr[charCount]) + 1;
    }

    return charCount;
}

/*
 * tfDnsLabelAsciiLen
 * 
 * Translate a domain name in ASCII format to DNS label format.
 * Note: This function returns only partial domain names. To mark a label
 *       as fully qualified, a trailing 0 length must be appended
 *
 * Parameter    Description
 * hostnamePtr  Pointer to domain name in ASCII format
 * hostnameLen  Length of domain name in ASCII format
 * labelPtr     Buffer to hold domain name in DNS-label format
 * labelLen     Length of buffer to hold domain name in DNS-label format
 *
 * Returns
 * Value           Meaning
 * length          Length of the DNS-label
 * -1              Error
 *
 */
int tfDnsAsciiToLabel(ttConstCharPtr  hostnamePtr,
                      int             hostnameLen,
                      tt8BitPtr       labelPtr,
                      int             labelLen)
{
    int count;
    int index;
    int retVal;

/* The label buffer isn't long enough */
    if (labelLen < (hostnameLen + 1))
    {
        retVal = -1;
    }
    else
    {
        count = 0;
        for (index = 0; index < hostnameLen; index++)
        {
/* In ASCII, a '.' means the end of a segment. In DNS-label format, the
 * length of the segment goes before the segment */
            if (hostnamePtr[index] != '.')
            {
                count++;
                labelPtr[index+1] = (tt8Bit)hostnamePtr[index];
            }
/* Otherwise, just copy over the byte */
            else
            {
                labelPtr[index-count] = (tt8Bit)count;
                count = 0;
            }
        }

/* Save the last segment length */
        labelPtr[index-count] = (tt8Bit)count;
        retVal = index + 1;
    }

    return retVal;
}
