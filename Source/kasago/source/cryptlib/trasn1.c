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
* Description: ASN.1 Encoding/Decoding functions
*
* Filename: trasn1.c
* Author: Jin Shang
* Date Created: 5/20/2002
 * $Source: source/cryptlib/trasn1.c $
 *
 * Modification History
 * $Revision: 6.0.2.6 $
 * $Date: 2012/09/15 16:08:53JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */



#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#ifdef TM_USE_PKI
#include <trpki.h>


/* BER/DER encoding/decoding, include:
 *  sequence,
 *  set
 *  object
 *  octet string
 *  bit   string
 *  integer
 *  null
 *  boolean
 *  tag head
 *  tag length
 *  context specific
 */



int tfAsn1SequenceEncode(ttVoidPtr      dump,
                         tt8BitPtrPtr   outPtrPtr,
                         int            length,
                         tt8Bit         classValue)
{
    tt8BitPtr   out8Ptr;
    int         len;
    tt8Bit      head;


    len  = 0;
    head =  (tt8Bit)(  TM_ASN1_TAG_SEQUENCE
                     | classValue
                     | TM_ASN1_CONSTRUCTED_TYPE);
    len++;
    if (outPtrPtr == (tt8BitPtrPtr)0)
    {
        out8Ptr = TM_8BIT_NULL_PTR;
    }
    else
    {
        out8Ptr = *outPtrPtr;
    }
    if (out8Ptr != TM_VOID_NULL_PTR)
    {
        *out8Ptr++ = head;
    }
    len +=tm_asn1_genlength_enc(dump,&out8Ptr,length);
    if (outPtrPtr != (tt8BitPtrPtr)0)
    {
        *outPtrPtr = out8Ptr;
    }
    return len;
}

int tfAsn1SequenceDecode(ttVoidPtr      dump,
                         tt8BitPtrPtr   in8PtrPtr,
                         tt8Bit         classValue)
{
    tt8BitPtr   in8Ptr;
    int         totalLen;
    int         length;
    int         retLen;
    tt8Bit      head;


    TM_UNREF_IN_ARG(dump);
    in8Ptr = *in8PtrPtr;
    if ((in8Ptr == TM_VOID_NULL_PTR) || (in8PtrPtr == TM_VOID_NULL_PTR))
    {
        retLen = -1;
        goto COMMONRETURN;
    }
    head =  (tt8Bit)(  TM_ASN1_TAG_SEQUENCE
                     | classValue
                     | TM_ASN1_CONSTRUCTED_TYPE);

    if (*in8Ptr != head)
    {
        retLen = -1;
        goto COMMONRETURN;
    }
    in8Ptr++;
    length = tm_asn1_genlength_dec(&totalLen, &in8Ptr);
    if (length == -1)
    {
        retLen = -1;
        goto COMMONRETURN;
    }

    *in8PtrPtr = in8Ptr;
/* tag + length */
    retLen = totalLen+length+1;

COMMONRETURN:
    return retLen;
}


int tfAsn1SetEncode(ttVoidPtr       dump,
                    tt8BitPtrPtr    outPtrPtr,
                    int             length,
                    tt8Bit          classValue)
{

    tt8BitPtr   out8Ptr;
    int         len;
    tt8Bit      head;

    len  = 0;
    out8Ptr = *outPtrPtr;
    head =  (tt8Bit)(TM_ASN1_TAG_SET|classValue | TM_ASN1_CONSTRUCTED_TYPE);
    len++;
    if (out8Ptr != TM_VOID_NULL_PTR)
    {
        *out8Ptr++ = head;
    }
    len +=tm_asn1_genlength_enc(dump,&out8Ptr,length);
    *outPtrPtr = out8Ptr;

    return len;
}

int tfAsn1SetDecode(ttVoidPtr       dump,
                    tt8BitPtrPtr    in8PtrPtr,
                    tt8Bit          classValue)
{
    tt8BitPtr   in8Ptr;
    int         tlen;
    int         clen;
    int         retLen;
    tt8Bit      head;

    TM_UNREF_IN_ARG(dump);
    if (in8PtrPtr == TM_VOID_NULL_PTR)
    {
        retLen = -1;
        goto COMMONRETURN;
    }
    in8Ptr = *in8PtrPtr;
    if (in8Ptr == TM_VOID_NULL_PTR)
    {
        retLen = -1;
        goto COMMONRETURN;
    }
    head =  (tt8Bit)(TM_ASN1_TAG_SET|
                    classValue |
                    TM_ASN1_CONSTRUCTED_TYPE);

    if (*in8Ptr != head)
    {
        retLen = 0;
        goto COMMONRETURN;
    }
    in8Ptr++;
    tlen = tm_asn1_genlength_dec(&clen, &in8Ptr);
    if (tlen == -1)
    {
        retLen = -1;
        goto COMMONRETURN;
    }

    *in8PtrPtr = in8Ptr;
/* tag + length part */
    retLen = clen+tlen+1;

COMMONRETURN:
    return retLen;
}

int tfAsn1ObjectEncode (ttAsn1ObjectPtr objPtr, tt8BitPtrPtr outPtrPtr)
{
    tt8BitPtr   out8Ptr;
    int         clen;
    int         length;
    int         i;
    int         retLen;

    if (objPtr == TM_VOID_NULL_PTR)
    {
        retLen = -1;
        goto COMMONRETURN;

    }

    if (outPtrPtr != TM_VOID_NULL_PTR)
    {
        out8Ptr = *outPtrPtr;
    }
    else
    {
        out8Ptr = TM_VOID_NULL_PTR;
    }

    clen = 0;
    clen += tm_asn1_taghead_enc(TM_VOID_NULL_PTR,
                                &out8Ptr,
                                TM_ASN1_TAG_OBJECT);

    length = *((ttIntPtr)(objPtr->objDerStr));

    clen += tm_asn1_genlength_enc(TM_VOID_NULL_PTR,
                                  &out8Ptr,
                                  length);

    if ((out8Ptr == TM_VOID_NULL_PTR) || (outPtrPtr == TM_VOID_NULL_PTR))
    {
        retLen = clen + length;
        goto COMMONRETURN;
    }

/* assume all object has been initialized */
    for (i=0; i < length ; i++)
    {
        *out8Ptr++ = *((tt8BitPtr)(objPtr->objDerStr) + sizeof(int)+i);
    }

    *outPtrPtr = out8Ptr;
    retLen = clen+ length;

COMMONRETURN:
    return retLen;
}

int tfAsn1ObjectDecode(ttAsn1ObjectPtrPtr objPtrPtr, tt8BitPtrPtr in8PtrPtr)
{
    ttAsn1ObjectPtr         objPtr;
    tt8BitPtr               in8Ptr;
    int                     tag;
    int                     clen;
    int                     length;
    int                     retLen;

    if ((in8PtrPtr == TM_VOID_NULL_PTR)|| (*in8PtrPtr == TM_VOID_NULL_PTR))
    {
        retLen = -1;
        goto COMMONRETURN;
    }

    in8Ptr  = *in8PtrPtr;
    objPtr = *objPtrPtr;

    clen = 0;
    clen +=tm_asn1_taghead_dec (&tag, &in8Ptr);
    retLen = tm_asn1_genlength_dec (&length, &in8Ptr);
    if (retLen != -1)
    {
        clen += retLen;
        retLen = -1;
    }
    else
    {
        goto COMMONRETURN;
    }

    objPtr = tfPkiAsn1ObjIdSearch(in8Ptr,TM_PKI_OBJECT_SEARCH_DER,length);
#if defined(TM_LOG_CERT_WARNING) || defined(TM_DEBUG_LOGGING)
    if (objPtr == TM_VOID_NULL_PTR)
    {
#ifdef TM_LOG_CERT_WARNING
        tfEnhancedLogWrite(
            TM_LOG_MODULE_CERT,
            TM_LOG_LEVEL_WARNING,
            "tfAsn1ObjectDec: "\
            "OBJECT '%s' not found!",
            in8Ptr);
#endif /* TM_LOG_CERT_WARNING */
        tm_debug_log1("tfAsn1ObjectDec: Object '%s' not found!", in8Ptr);
/* Ignore unfound object, and continue processing */
    }
#endif /* defined(TM_LOG_CERT_WARNING) || defined(TM_DEBUG_LOGGING) */
    in8Ptr +=length;
    *objPtrPtr = objPtr;
    *in8PtrPtr = in8Ptr;

    retLen = clen + length;

COMMONRETURN:
    return retLen;
}

int tfAsn1NullEncode(tt8BitPtrPtr outPtrPtr)
{
    tt8BitPtr   out8Ptr;
    int         ret;

    if (outPtrPtr != TM_VOID_NULL_PTR)
    {
        out8Ptr = *outPtrPtr;
    }
    else
    {
        out8Ptr = TM_VOID_NULL_PTR;
    }

    if ((out8Ptr == TM_VOID_NULL_PTR) || (outPtrPtr == TM_VOID_NULL_PTR))
    {
        ret =  2;
        goto COMMONRETURN;
    }

    *out8Ptr++ = 0x05;
    *out8Ptr++ = 0x00;
    *outPtrPtr = out8Ptr;
    ret = 2;
COMMONRETURN:
    return ret;
}

int tfAsn1NullDecode(tt8BitPtrPtr in8PtrPtr)
{
    int ret;

    ret = 0;
    if ((in8PtrPtr == TM_VOID_NULL_PTR) || (*in8PtrPtr == TM_VOID_NULL_PTR))
    {
        ret = 0;
    }
    else if (*(*in8PtrPtr) == 0x05)
    {
        *in8PtrPtr = *in8PtrPtr + 2;
        ret = 2;
    }

    return ret;
}

int tfAsn1BooleanEncode(int value, tt8BitPtrPtr outPtrPtr)
{
    tt8BitPtr   out8Ptr;
    int         clen;

    if (outPtrPtr != TM_VOID_NULL_PTR)
    {
        out8Ptr = *outPtrPtr;
    }
    else
    {
        out8Ptr = TM_VOID_NULL_PTR;
    }

    if (value == 0)
    {
        clen = 0;
    }
    else
    {
        clen = 3;
    }
    if (   (out8Ptr == TM_VOID_NULL_PTR)
        || (outPtrPtr == TM_VOID_NULL_PTR)
        || (clen == 0) )
    {
        goto COMMONRETURN;
    }

    *out8Ptr++ = 0x01;
    *out8Ptr++ = 0x01;
    *out8Ptr++ = 0xff;
    *outPtrPtr = out8Ptr;

COMMONRETURN:
    return clen;
}


int tfAsn1BooleanDecode(ttIntPtr value, tt8BitPtrPtr in8PtrPtr)
{
    int ret;
    ret = 0;

    if ((in8PtrPtr == TM_VOID_NULL_PTR) || (*in8PtrPtr == TM_VOID_NULL_PTR))
    {
        ret = 0;
    }
    if (*(*in8PtrPtr) == 0x01)
    {
        *value =1;
        *in8PtrPtr = *in8PtrPtr + 3;
        ret = 3;
    }
    return ret;
}

int tfAsn1OctetStringEncode(ttAsn1OctetStringPtr    octPtr,
                            tt8BitPtrPtr            outPtrPtr)
{
    tt8BitPtr   out8Ptr;
    int         clen;
    int         i;
    int         retLen;

    if (octPtr == TM_VOID_NULL_PTR)
    {
        retLen = 0;
        goto COMMONRETURN;
    }

    if (outPtrPtr != TM_VOID_NULL_PTR)
    {
        out8Ptr = *outPtrPtr;
    }
    else
    {
        out8Ptr = TM_VOID_NULL_PTR;
    }

    clen = 0;
    clen += tm_asn1_taghead_enc(TM_VOID_NULL_PTR,
                                &out8Ptr,
                                TM_ASN1_TAG_OCTETSTRING);

    clen += tm_asn1_genlength_enc(TM_VOID_NULL_PTR,
                                  &out8Ptr,
                                  octPtr->asn1Length);

    if ((out8Ptr == TM_VOID_NULL_PTR) || (outPtrPtr == TM_VOID_NULL_PTR))
    {
        retLen = clen + octPtr->asn1Length;
        goto COMMONRETURN;
    }

    for (i=0; i < octPtr->asn1Length ; i++)
    {
        *out8Ptr++ = octPtr->asn1Data[i];
    }

    *outPtrPtr = out8Ptr;
    retLen = clen+ octPtr->asn1Length;

COMMONRETURN:
    return retLen;
}

int tfAsn1OctetStringDecode(ttAsn1OctetStringPtrPtr octPtrPtr,
                            tt8BitPtrPtr            in8PtrPtr)
{
    ttAsn1BitStringPtr      octPtr;
    tt8BitPtr               in8Ptr;
    int                     clen;
    int                     length;
    int                     i;
    int                     tag;
    int                     retLen;

    retLen = -1;

    if (in8PtrPtr == TM_VOID_NULL_PTR)
    {
        goto octStringDecExit;
    }
    if (*in8PtrPtr == TM_VOID_NULL_PTR)
    {
        goto octStringDecExit;
    }

    in8Ptr  = *in8PtrPtr;
    octPtr = *octPtrPtr;

    if (octPtr == TM_VOID_NULL_PTR)
    {
        octPtr = tfAsn1StringNew(TM_ASN1_TAG_OCTETSTRING);
        if (octPtr == (ttAsn1BitStringPtr)0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfAsn1OctStringDecode", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
            goto octStringDecExit;
        }
    }
    else
    {
        if (octPtr->asn1Data != TM_VOID_NULL_PTR)
        {
            tm_free_raw_buffer(octPtr->asn1Data);
        }
    }

    clen = 0;
    clen +=tm_asn1_taghead_dec (&tag, &in8Ptr);
    retLen = tm_asn1_genlength_dec (&length, &in8Ptr);
    if (retLen != -1)
    {
        clen += retLen;
        retLen = -1;
    }
    else
    {
        goto octStringDecExit;
    }

    if (length != 0)
    {
        octPtr->asn1Data = tm_get_raw_buffer((ttPktLen)length + 1);
        if (octPtr->asn1Data == TM_8BIT_NULL_PTR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfAsn1OctStringDecode", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
            tm_free_raw_buffer(octPtr);
            goto octStringDecExit;
        }
        else
        {
            tm_bzero(octPtr->asn1Data, length + 1);
        }
    }
    octPtr->asn1Length = length;
    for (i=0; i <length; i++)
    {
        octPtr->asn1Data[i] = *in8Ptr++;
    }
    *in8PtrPtr  = in8Ptr;
    *octPtrPtr = octPtr;
    retLen = clen + length;

octStringDecExit:
    return retLen;
}


int tfAsn1BitStringEncode(ttAsn1BitStringPtr  bitStringPtr,
                          tt8BitPtrPtr        outPtrPtr)
{
    tt8BitPtr   out8Ptr;
    tt8Bit      temp;
    int         clen;
    int         i;
    int         retLen;

    if (bitStringPtr == TM_VOID_NULL_PTR)
    {
        retLen = -1;
        goto COMMONRETURN;
    }

    if (outPtrPtr != TM_VOID_NULL_PTR)
    {
        out8Ptr = *outPtrPtr;
    }
    else
    {
        out8Ptr = TM_VOID_NULL_PTR;
    }

    clen = 0;
    clen += tm_asn1_taghead_enc(TM_VOID_NULL_PTR,
                                &out8Ptr,
                                TM_ASN1_TAG_BITSTRING);

    clen += tm_asn1_genlength_enc(TM_VOID_NULL_PTR,
                                  &out8Ptr,
                                  bitStringPtr->asn1Length+1);

    if ((out8Ptr == TM_VOID_NULL_PTR) || (outPtrPtr == TM_VOID_NULL_PTR))
    {
        retLen = clen+ bitStringPtr->asn1Length+1;
        goto COMMONRETURN;
    }


    temp = bitStringPtr->asn1Data[bitStringPtr->asn1Length-1] ;
    *out8Ptr = 0;
    if (temp != 0)
    {
        while (!(temp & 0x01))
        {
            temp>>=1;
            *out8Ptr = (tt8Bit)(*out8Ptr +1);
        }
    }
    out8Ptr++;

    for (i=0; i < bitStringPtr->asn1Length ; i++)
    {
        *out8Ptr++ = bitStringPtr->asn1Data[i];
    }

    *outPtrPtr = out8Ptr;
    retLen = clen+ bitStringPtr->asn1Length+1;

COMMONRETURN:
    return retLen;
}


int tfAsn1BitStringDecode(ttAsn1BitStringPtrPtr   bitStringPtrPtr,
                          tt8BitPtrPtr            in8PtrPtr)
{
    ttAsn1BitStringPtr      bitStringPtr;
    tt8BitPtr               in8Ptr;
    int                     clen;
    int                     length;
    int                     tag;
    int                     retLen;
    tt8Bit                  nouse;

    retLen = -1;
    if ((in8PtrPtr == TM_VOID_NULL_PTR) || (*in8PtrPtr == TM_VOID_NULL_PTR))
    {
        goto bitStringDecExit;
    }

    in8Ptr  = *in8PtrPtr;
    bitStringPtr = *bitStringPtrPtr;

    clen = 0;
    clen +=tm_asn1_taghead_dec(&tag, &in8Ptr);
    retLen = tm_asn1_genlength_dec(&length, &in8Ptr);
    if (retLen != -1)
    {
        clen += retLen;
        retLen = -1;
    }
    else
    {
        goto bitStringDecExit;
    }

/*
 * first byte gives number of unused bits.  Not part of the string.
 */
    if (length > 1)
    {
        if (bitStringPtr == TM_VOID_NULL_PTR)
        {
            bitStringPtr = tfAsn1StringNew(TM_ASN1_TAG_BITSTRING);
            if (bitStringPtr == (ttAsn1BitStringPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfAsn1BitStringDecode", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
                goto bitStringDecExit;
            }
        }
        else
        {
            if (bitStringPtr->asn1Data != TM_VOID_NULL_PTR)
            {
                tm_free_raw_buffer(bitStringPtr->asn1Data);
                bitStringPtr->asn1Data = TM_8BIT_NULL_PTR;
            }
        }
        bitStringPtr->asn1Data = tm_get_raw_buffer((ttPktLen)length - 1);
        if (bitStringPtr->asn1Data == TM_8BIT_NULL_PTR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfAsn1BitStringDecode", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
            tm_free_raw_buffer(bitStringPtr);
            goto bitStringDecExit;
        }
        bitStringPtr->asn1Length = length - 1;
        nouse = *in8Ptr++; /* number of unused bits */
        tm_bcopy(in8Ptr, bitStringPtr->asn1Data, length - 1);
        in8Ptr += length - 1; /* pass over copied data */
/* Turn off unused bits (at end of string) */
        if (nouse != 0)
        {
            bitStringPtr->asn1Data[length - 2] &= (tt8Bit)(0xff<<nouse);
        }
        *bitStringPtrPtr = bitStringPtr;
        *in8PtrPtr = in8Ptr;
        retLen = clen + length;
    }
bitStringDecExit:
    return retLen;
}
/* two complented binary number */

int tfAsn1IntegerEncode(ttAsn1IntegerPtr asnIntPtr,tt8BitPtrPtr outPtrPtr)
{
    tt8BitPtr   out8Ptr;
    int         clen;
    int         i;
    int         needpad;
    int         retLen;

    if (asnIntPtr == TM_VOID_NULL_PTR)
    {
        retLen = 0;
        goto COMMONRETURN;
    }

    if (outPtrPtr != TM_VOID_NULL_PTR)
    {
        out8Ptr = *outPtrPtr;
    }
    else
    {
        out8Ptr = TM_VOID_NULL_PTR;
    }

    clen = 0;
    clen += tm_asn1_taghead_enc(TM_VOID_NULL_PTR,
                                &out8Ptr,
                                TM_ASN1_TAG_INTEGER);
    needpad = 0;
    if (asnIntPtr->asn1Data!= TM_VOID_NULL_PTR)
    {
        if (   (asnIntPtr->asn1Type != TM_ASN1_TAG_NEG_INTEGER)
            && (asnIntPtr->asn1Data[0] > 127) )
/* need 00 pad in first byte */
        {
            asnIntPtr->asn1Length++;
            needpad = 1;
        }
    }
    clen += tm_asn1_genlength_enc(TM_VOID_NULL_PTR,
                                  &out8Ptr,
                                  asnIntPtr->asn1Length);
    if (needpad)
    {
        *out8Ptr++=0;
    }
    if (outPtrPtr == TM_VOID_NULL_PTR)
    {
        retLen = asnIntPtr->asn1Length+clen;
        goto COMMONRETURN;
    }
    if (*outPtrPtr == TM_VOID_NULL_PTR)
    {
        retLen = asnIntPtr->asn1Length+clen;
        goto COMMONRETURN;
    }

    for (i=0 ; i < asnIntPtr->asn1Length-needpad; i++)
    {
        *out8Ptr++ = asnIntPtr->asn1Data[i];
    }
    *outPtrPtr = out8Ptr;
    retLen = asnIntPtr->asn1Length + clen;

COMMONRETURN:
    return retLen;
}

int tfAsn1IntegerDecode(ttAsn1IntegerPtrPtr     asnIntPtrPtr,
                        tt8BitPtrPtr            in8PtrPtr )
{
    tt8BitPtr           in8Ptr;
    ttAsn1IntegerPtr    asnIntPtr;
    int                 clen;
    int                 length;
    int                 tag;
    int                 i;
    int                 retLen;

    retLen = -1;

    if (in8PtrPtr == TM_VOID_NULL_PTR)
    {
        goto integerDecExit;
    }
    if (*in8PtrPtr == TM_VOID_NULL_PTR)
    {
        goto integerDecExit;
    }

    in8Ptr = *in8PtrPtr;
    asnIntPtr = *asnIntPtrPtr;
    clen = 0;
    clen +=tm_asn1_taghead_dec(&tag, &in8Ptr);
    retLen = tm_asn1_genlength_dec(&length, &in8Ptr);
    if (retLen != -1)
    {
        clen += retLen;
        retLen = -1;
    }
    else
    {
        goto integerDecExit;
    }

    if (asnIntPtr == TM_VOID_NULL_PTR)
    {
        asnIntPtr = tfAsn1StringNew(TM_ASN1_TAG_INTEGER);
        if (asnIntPtr == (ttAsn1IntegerPtr)0)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfAsn1IntegerDecode", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
            goto integerDecExit;
        }

    }
    else
    {
        if (asnIntPtr->asn1Data != TM_VOID_NULL_PTR)
        {
            tm_free_raw_buffer(asnIntPtr->asn1Data);
        }
        asnIntPtr->asn1Data = TM_VOID_NULL_PTR;
    }


    asnIntPtr->asn1Type   = TM_ASN1_TAG_INTEGER;
    if (*in8Ptr & 0x80)
    {
        asnIntPtr->asn1Type = TM_ASN1_TAG_NEG_INTEGER;
    }
    else if ((*in8Ptr == 0) && (length !=1))
    {
        length--;
        in8Ptr++;
    }

    asnIntPtr->asn1Data = tm_get_raw_buffer((ttPktLen)length);
    if (asnIntPtr->asn1Data == TM_8BIT_NULL_PTR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfAsn1IntegerDecode", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
        tm_free_raw_buffer(asnIntPtr);
        goto integerDecExit;
    }
    asnIntPtr->asn1Length = length;

    for (i=0; i < length ; i++)
    {
        asnIntPtr->asn1Data[i] = *in8Ptr++;
    }

    *in8PtrPtr = in8Ptr;
    *asnIntPtrPtr = asnIntPtr;
    retLen = length + clen;

integerDecExit:
    return retLen;
}




int tfAsn1TagHeadEncode(tt8BitPtrPtr    outPtrPtr,
                        tt8Bit          classValue,
                        tt8Bit          typeValue,
                        int             tag)
{
    tt8BitPtr   out8Ptr;
    int         clen;
    int         encode;
    tt8Bit      head;

    encode = 1;

    if (tag < 0)
    {
        clen = 0;
        goto COMMONRETURN;
    }
    clen = 1;

    if (   (tag == TM_ASN1_TAG_SEQUENCE)
        || (tag == TM_ASN1_TAG_SET) )
    {
        typeValue = TM_ASN1_CONSTRUCTED_TYPE;
    }

    head = (tt8Bit)(classValue|typeValue);

    if ((outPtrPtr == TM_VOID_NULL_PTR) || (*outPtrPtr == TM_VOID_NULL_PTR))
    {
        encode = 0;
        out8Ptr = TM_VOID_NULL_PTR;
    }
    else
    {
        out8Ptr = *outPtrPtr;
    }
    if (tag >= 31)
    {
        head |= 0x1f;
        if (encode == 1)
        {
            *out8Ptr++ = head;
        }
        clen += tm_asn1_taglength_enc(TM_VOID_NULL_PTR, &out8Ptr, tag);
    }
    else
    {
        head |= (tt8Bit)tag;
        if (encode == 1)
        {
            *out8Ptr++ = head;
        }
    }

    *outPtrPtr = out8Ptr;

COMMONRETURN:
    return clen;
}

int tfAsn1TagHeadDecode(ttIntPtr        tagPtr,
                        tt8BitPtrPtr    in8PtrPtr,
                        tt8BitPtr       classPtr,
                        tt8BitPtr       typePtr )
{
    tt8BitPtr       in8Ptr;
    int             clen;

    clen = 0;
    if ((in8PtrPtr == TM_VOID_NULL_PTR) || (*in8PtrPtr == TM_VOID_NULL_PTR))
    {
        clen =  -1;
        goto COMMONRETURN;
    }
    in8Ptr = *in8PtrPtr;

    if (classPtr != TM_VOID_NULL_PTR)
    {
        *classPtr = (tt8Bit)(*in8Ptr & 0xc0);
    }
    if (typePtr != TM_VOID_NULL_PTR)
    {
        *typePtr  = (tt8Bit)(*in8Ptr & 0x20);
    }

    *tagPtr = (tt8Bit)(*in8Ptr++ & 0x1f);
    clen++;
    if (*tagPtr == 31)
    {
        clen += tm_asn1_taglength_dec(tagPtr, &in8Ptr);
    }
    *in8PtrPtr = in8Ptr;

COMMONRETURN:
    return clen;
}


/* e. g.
    1???????? 1???????? 0????????

**/

int tfAsn1TagLengthEncode(ttVoidPtr       dump,
                          tt8BitPtrPtr    outPtrPtr,
                          int             length)
{
    tt8BitPtr       out8Ptr;
    int             tlen;
    int             clen;
    int             value;

    TM_UNREF_IN_ARG(dump);
    value = length;
    clen = 0;
    while (value > 127)
    {
        value = value/128;
        clen++;
    }
    clen++;

    if (outPtrPtr == TM_VOID_NULL_PTR)
    {
        goto ASN1_TAG_LENGTH_ENCODE_EXIT;
    }

    out8Ptr = *outPtrPtr;
    if (out8Ptr == TM_VOID_NULL_PTR)
    {
        goto ASN1_TAG_LENGTH_ENCODE_EXIT;
    }
    tlen = clen;
    out8Ptr[--tlen] = (tt8Bit)(length%128);
    while (length > 127)
    {
        length = length / 128;
        out8Ptr[--tlen] = (tt8Bit)((length%128) | 0x80);
    }
    *outPtrPtr +=clen;

ASN1_TAG_LENGTH_ENCODE_EXIT:
    return clen;
}

int tfAsn1TagLengthDecode(ttIntPtr tagPtr, tt8BitPtrPtr in8PtrPtr)
{
    tt8BitPtr       in8Ptr;
    int             clen;

    if ((in8PtrPtr == TM_VOID_NULL_PTR) || (*in8PtrPtr == TM_VOID_NULL_PTR))
    {
        clen =  -1;
        goto COMMONRETURN;
    }
    *tagPtr = 0;
    in8Ptr = *in8PtrPtr;

    while (*in8Ptr & 0x80)
    {
        *tagPtr = (int)((*in8Ptr++ & 0x7f) + (*tagPtr) * 128 );

    }
    *tagPtr  += *in8Ptr++;
    clen  = (int)(in8Ptr - *in8PtrPtr);
    *in8PtrPtr = in8Ptr;

COMMONRETURN:
    return clen;
}


/*  definite form to represent length,
     ttS32Bit or short type
e.g.   38 -> 00100110
       201-> 10000001 11001001 */

int tfAsn1GenLengthEncode(ttVoidPtr       dump,
                          tt8BitPtrPtr    outPtrPtr,
                          int             length)
{
    int         clen;
    int         tlen;
    int         cvalue;
    tt8BitPtr   out8Ptr;

    dump    = dump;
    clen    = 0;
    cvalue  = length;
    out8Ptr  = *outPtrPtr;

    clen++;
    if (cvalue >= 128)
    {
        clen++;
    }
    while (cvalue >= 256)
    {
        cvalue = cvalue/256;
        clen++;
    }

    if ((outPtrPtr == TM_VOID_NULL_PTR) || (out8Ptr == TM_VOID_NULL_PTR))
    {
        goto ASN1_GEN_LENGTH_ENCODE_EXIT;
    }
    tlen = clen;

    while (length > 0)
    {
        out8Ptr[--tlen] = (tt8Bit)(length%256);
        length = length/256;
    }
    if (tlen == 1)
    {
        *out8Ptr = (tt8Bit)(clen - 1 + 128);
    }

    *outPtrPtr += clen;

ASN1_GEN_LENGTH_ENCODE_EXIT:
    return clen;
}

int tfAsn1GenLengthDecode(ttIntPtr lenPtr, tt8BitPtrPtr in8PtrPtr)
{
    tt8BitPtr   in8Ptr;
    int         length;
    int         value;
    int         i;
    int         retLen;

    in8Ptr = *in8PtrPtr;
    if ((in8Ptr == TM_VOID_NULL_PTR) || (in8PtrPtr == TM_VOID_NULL_PTR))
    {
        retLen = -1;
        goto COMMONRETURN;
    }

    i=0;
    value = 0;
    length = (int)(*in8Ptr++);

/* check value */
    if (length <= 0x7F)
    {
        *in8PtrPtr = in8Ptr;
        *lenPtr = length;
        retLen = 1;
        goto COMMONRETURN;
    }
    else if (length > 0x84)
    {
/*
 * We will store the length in an integer (4 bytes)
 * Therefore, the length in the packet can't be longer
 * than 4 bytes.
 */
        retLen = -1;
        goto COMMONRETURN;
    }

    while (i < (length - 0x80))
    {
        value =(int)(value*256 + (*in8Ptr++));
        i++;
    }

    if (value >= 0)
    {
        *in8PtrPtr = in8Ptr;
        *lenPtr = value;
        retLen = length - 0x80 + 1;
    }
    else
    {
        retLen = -1;
    }

COMMONRETURN:
    return retLen;
}


/* standard DER Encoding *
 * objDerStr need to be allocated before calling the function
 * Input: source: pointer, objDerStr: output pointer, type *
 * Output:
 *        TM_ENOERROR: successful
 */
int tfAsn1Enc(ttVoidPtr source, ttVoidPtr objDerStr, int type)
{
    int             tlen,clen;
    int             firsttwo;
    tt32Bit         firstvalue;
    tt32Bit         value,cvalue;
    tt8BitPtr       temp,dest;

    dest        = (tt8BitPtr)objDerStr;
    tlen        = 0;
    firsttwo    = 2;
    firstvalue  = 0;

    switch (type)
    {
        case TM_ASN1_TAG_OBJECT:
/* format example: 1.2.840.10040.4.3 */
            temp = (tt8BitPtr)(((ttAsn1ObjectPtr)source)->objIdStr);
            while (*temp!='\0')
            {
                clen = 0;
                value = 0;
                cvalue = 0;
/* find a number value till dot (.) */
                while ((*temp !='.') && (*temp!='\0'))
                {
                    if ((*temp <'0') || (*temp > '9'))
                    {
#ifdef TM_ERROR_CHECKING
                        tfKernelWarning("tfAsn1Enc", "OBJECT error!\n");
#endif /* TM_ERROR_CHECKING */
                        tlen = -1;
                        goto COMMONRETURN;
                    }
                    value = value*10 + (tt32Bit)((*temp++)-'0');
                }
                if (*temp == '.')
                {
                    temp++;
                }
                firsttwo--;
                if (firsttwo == 1)
                {
                    firstvalue = value*40;
                    continue;
                }
                else if (firsttwo == 0)
                {
                    value = firstvalue + value;
                }
/* check the character length to store the value */
                cvalue = value;
                while (cvalue >= 128)
                {
                    cvalue = cvalue/128;
                    clen++;
                }
                clen++;
                tlen += clen;
                if (dest != TM_VOID_NULL_PTR)
                {
/* store the value to the character array, see X.690 Object DER */
                    dest[--tlen] = (tt8Bit)(value%128);
                    while (value >= 128)
                    {
                        value = value/128;
                        dest[--tlen] = (tt8Bit)(value%128 + 128);
                    }
                    tlen += clen;
                }
            }
            break;
        default:
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfAsn1Enc", "Default error!\n");
#endif /* TM_ERROR_CHECKING */
            tlen = -1;
            goto COMMONRETURN;
/* never reached*/
    }

COMMONRETURN:
    return tlen;
}




/* two complemented */

int tfAsn1IntegerSet(ttAsn1IntegerPtr a, ttS32Bit value)
{
    ttS32Bit    cvalue;
    int         clen;
    int         errorCode;
    tt8Bit      mod;

    errorCode = TM_ENOERROR;
    mod = 0;
    if  (a == TM_VOID_NULL_PTR)
    {
        errorCode = -1;
        goto ASN1_INTEGER_SET_EXIT;
    }

    if (a->asn1Data == TM_VOID_NULL_PTR)
    {
        a->asn1Data = tm_get_raw_buffer(sizeof(ttS32Bit)+1);
        if (a->asn1Data == TM_VOID_NULL_PTR)
        {
            errorCode = -1;
            goto ASN1_INTEGER_SET_EXIT;
        }
    }
    a->asn1Type=TM_ASN1_TAG_INTEGER;
    if (value < 0)
    {
        value= -value+1;
        a->asn1Type=TM_ASN1_TAG_NEG_INTEGER;
        mod = 0xff;
    }

    clen = 0;

    cvalue = value;

    while (cvalue > 127)
    {
        cvalue = cvalue/256;
        clen++;
    }
    clen++;
    a->asn1Length = clen;

    while (value > 127)
    {
        a->asn1Data[--clen] = (tt8Bit)(((tt8Bit)(value%256)) ^ mod);
        value = value/256;
    }
    a->asn1Data[--clen] = (tt8Bit)((tt8Bit)value ^ mod);

ASN1_INTEGER_SET_EXIT:
    return errorCode;
}

ttS32Bit tfAsn1IntegerGet(ttAsn1IntegerPtr a)
{
    ttS32Bit    value;
    int         i;
    int         neg;
    tt8Bit      mod;

    if ((a == TM_VOID_NULL_PTR)||(a->asn1Data == TM_VOID_NULL_PTR))
    {
        value = 0;
        goto ASN1_INTEGER_GET_EXIT;
    }
    if ((unsigned)(a->asn1Length) > sizeof(ttS32Bit) )
    {
        value = 0;
        goto ASN1_INTEGER_GET_EXIT;
    }

    i=a->asn1Type;
    neg =0;
    mod = 0;
    if (i == TM_ASN1_TAG_NEG_INTEGER)
    {
        neg = 1;
    }
    else if (i != TM_ASN1_TAG_INTEGER)
    {
        value = 0;
        goto ASN1_INTEGER_GET_EXIT;
    }
    if (neg == 1)
    {
        mod = 0xff;
    }

    value = 0;
    value = (ttS32Bit)((*(tt8BitPtr)a->asn1Data ^mod) & 0x7f);
    for (i=1 ; i < a->asn1Length; i++)
    {
        value =
            (ttS32Bit)(value*256 + ((*((tt8BitPtr)(a->asn1Data + i)))^(mod)));
    }

    if (neg == 1)
    {
        value = -value - 1;
    }

ASN1_INTEGER_GET_EXIT:
    return value;
}



int tfAsn1StringSet(ttAsn1StringPtr strPtr, ttVoidPtr dataPtr, int len)
{
    int retValue;

    retValue = -1;
    if ((dataPtr == TM_VOID_NULL_PTR) || (strPtr == TM_VOID_NULL_PTR))
    {
        goto stringSetExit;
    }

    if ((strPtr->asn1Length < len) || (strPtr->asn1Data == TM_VOID_NULL_PTR))
    {
        if (strPtr->asn1Data != TM_VOID_NULL_PTR)
        {
            tm_free_raw_buffer(strPtr->asn1Data);
        }
        strPtr->asn1Data=tm_get_raw_buffer((ttPktLen)len+1);
        if (strPtr->asn1Data == TM_8BIT_NULL_PTR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfAsn1StringSet", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
            goto stringSetExit;
        }
    }
    strPtr->asn1Length=len;

    (void)tm_memcpy(strPtr->asn1Data,dataPtr,len);
    strPtr->asn1Data[len]='\0';
    retValue = TM_ENOERROR;

stringSetExit:
    return retValue;
}



int tfAsn1ContextSpecificEncode(tt8Bit       tag,
                                tt8BitPtrPtr outPtrPtr,
                                int          length)
{
    tt8BitPtr       out8Ptr;
    int             clen;
    tt8Bit          head;

    head = (tt8Bit)(tag|
                    TM_ASN1_CONTEXT_SPECIFIC_CLASS|
                    TM_ASN1_CONSTRUCTED_TYPE);

    out8Ptr = TM_VOID_NULL_PTR;
    if (outPtrPtr != TM_VOID_NULL_PTR)
    {
        out8Ptr = *outPtrPtr;
    }
    clen = 1;
    if (out8Ptr != TM_VOID_NULL_PTR)
    {
        *out8Ptr++ = head;
    }
    clen += tm_asn1_genlength_enc(TM_VOID_NULL_PTR,
                                  &out8Ptr,
                                  length);
    *outPtrPtr = out8Ptr;
    return clen;
}


int tfAsn1ContextSpecificDecode (ttIntPtr lenPtr, tt8BitPtrPtr in8PtrPtr)

{
    tt8BitPtr       in8Ptr;
    int             clen;

    if (in8PtrPtr == TM_VOID_NULL_PTR)
    {
        clen = -1;
        goto COMMONRETURN;
    }
    if (*in8PtrPtr == TM_VOID_NULL_PTR)
    {
        clen = -1;
        goto COMMONRETURN;
    }

    in8Ptr = *in8PtrPtr;

    clen = tm_asn1_genlength_dec(lenPtr, &in8Ptr);
    if (clen != -1)
    {
        clen += 1;
    }
    else
    {
        goto COMMONRETURN;
    }

    *in8PtrPtr = in8Ptr;

COMMONRETURN:
    return clen;
}


ttAsn1StringPtr tfAsn1StringNew(int type)
{
    ttAsn1StringPtr     retPtr;

    retPtr=(ttAsn1StringPtr)tm_get_raw_buffer(sizeof(ttAsn1String));
    if (retPtr == TM_VOID_NULL_PTR)
    {
        goto COMMONRETURN;
    }
    retPtr->asn1Length     = 0;
    retPtr->asn1Type       = type;
    retPtr->asn1Data       = TM_VOID_NULL_PTR;

COMMONRETURN:
    return(retPtr);
}

int tfAsn1StringFree(ttAsn1StringPtr strPtr)
{
    if (strPtr == TM_VOID_NULL_PTR)
    {
        goto COMMONRETURN;
    }
    if (strPtr->asn1Data != TM_VOID_NULL_PTR)
    {
        tm_free_raw_buffer(strPtr->asn1Data);
    }
    tm_free_raw_buffer((tt8BitPtr)strPtr);

COMMONRETURN:
    return TM_ENOERROR;
}

void tfAsn1StringInit(ttAsn1StringPtr   strPtr)
{
    if (strPtr == TM_VOID_NULL_PTR)
    {
        goto COMMONRETURN;
    }
    strPtr->asn1Length  = 0;
    strPtr->asn1Data    = TM_VOID_NULL_PTR;
    strPtr->asn1Type    = 0;

COMMONRETURN:
    return;
}

ttAsn1TimePtr tfAsn1TimeNew(ttS32Bit offset)
{
    ttAsn1TimePtr   tPtr;
    ttS32Bit        value;

    tm_kernel_set_critical;
    value = (ttS32Bit)(tvTime/1000);
    tm_kernel_release_critical;
    value +=offset;

    tPtr = tfAsn1StringNew(TM_ASN1_TAG_UTCTIME);
    if (tPtr == TM_VOID_NULL_PTR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfAsn1TimeNew", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
        goto asn1TimeNewExit;
    }

    tPtr->asn1Data   = tm_get_raw_buffer(20);
    if (tPtr->asn1Data == TM_8BIT_NULL_PTR)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfAsn1TimeNew", "no memory!\n");
#endif /* TM_ERROR_CHECKING */
        tm_free_raw_buffer(tPtr);
        tPtr = (ttAsn1TimePtr)0;
        goto asn1TimeNewExit;
    }

    (void)tm_sprintf((ttCharPtr)tPtr->asn1Data,"%12ld",value);
    tPtr->asn1Length = (int) tm_strlen(tPtr->asn1Data);

asn1TimeNewExit:
    return tPtr;
}


#else /* !TM_USE_PKI */
/* To allow link for builds when TM_USE_PKI is not defined */
int tvAsn1Dummy = 0;
#endif /* TM_USE_PKI */
