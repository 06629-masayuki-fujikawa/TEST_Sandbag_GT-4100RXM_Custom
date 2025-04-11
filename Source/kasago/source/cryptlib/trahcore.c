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
 * Description: AH algorithms
 *
 * Filename: trahcore.c
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: source/cryptlib/trahcore.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:35:15JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
/* have to include this, because tfMd5 stuff is there */
#include <trproto.h> 
#include <trglobal.h>


#include <trcrylib.h>

#ifdef TM_USE_MD2
static int tfMd2ContextLen(void);
#endif /* TM_USE_MD2 */

#ifdef TM_USE_MD5
static int tfMd5ContextLen(void);
#endif /* TM_USE_MD5 */

#ifdef TM_USE_SHA1
static int tfSha1ContextLen(void);
#endif /* TM_USE_SHA1 */

#ifdef TM_USE_RIPEMD
static int tfRmd160ContextLen(void);
#endif /* TM_USE_RIPEMD*/

#ifdef TM_USE_SHA256
static int tfSha256ContextLen(void);
#endif /* TM_USE_SHA256 */

#if ( defined(TM_USE_SHA384) || defined(TM_USE_SHA512) )
static int tfSha512ContextLen(void);
#endif /* defined(TM_USE_SHA384) || defined(TM_USE_SHA512) */

#ifdef TM_USE_AESXCBC
static int tfAesXcbcContextLen(void);
#endif /* TM_USE_AESXCBC */

static const ttAhAlgorithm ahAlgorithms[] = 
{
/* NONE_AUTH */
    { 
        SADB_AALG_NULL, 0, 0, 0, 0, 0,
        0, 0, 0, 0
    }, 
/* HMAC_MD5 */
    { 
        SADB_AALG_MD5HMAC, 128, 128, 128, 128, 96, 
#ifdef TM_AALG_VAR_BLOCK_SIZE
        64,
#endif /* TM_AALG_VAR_BLOCK_SIZE */
        
#ifdef TM_USE_MD5
        tfMd5Init, tfMd5Update, tfMd5Final,
        tfMd5ContextLen
#else /* TM_USE_MD5 */
        0, 0, 0, 0
#endif /* TM_USE_MD5 */
    },
/* HMAC_SHA1 */
    { 
        SADB_AALG_SHA1HMAC, 160, 160, 160, 160, 96,
#ifdef TM_AALG_VAR_BLOCK_SIZE
        64,
#endif /* TM_AALG_VAR_BLOCK_SIZE */
#ifdef TM_USE_SHA1
        tfSha1Init, tfSha1Update, tfSha1Final,
        tfSha1ContextLen
#else /* TM_USE_SHA1 */
        0, 0, 0, 0
#endif /* TM_USE_SHA1 */
    },
/* HMAC_SHA256 */
    { 
        SADB_AALG_SHA256HMAC, 256, 256, 256, 256,
#ifdef TM_USE_SHAHMAC_96
            96,
#else /* TM_USE_SHAHMAC_96 */
            128,
#endif /* TM_USE_SHAHMAC_96 */
#ifdef TM_AALG_VAR_BLOCK_SIZE
        64, /* block size */
#endif /* TM_AALG_VAR_BLOCK_SIZE */
#ifdef TM_USE_SHA256
        tfSha256Init, tfSha256Update, tfSha256Final,
        tfSha256ContextLen
#else /* !TM_USE_SHA256 */
        0, 0, 0, 0
#endif /* TM_USE_SHA256 */
    },
/* HMAC_SHA384 */
    { 
        SADB_AALG_SHA384HMAC, 384, 384, 384, 384,
#ifdef TM_USE_SHAHMAC_96
            96,
#else /* TM_USE_SHAHMAC_96 */
            192,
#endif /* TM_USE_SHAHMAC_96 */
#ifdef TM_AALG_VAR_BLOCK_SIZE
        128, /* block size */
#endif /* TM_AALG_VAR_BLOCK_SIZE */
#ifdef TM_USE_SHA384
        tfSha384Init, tfSha384Update, tfSha384Final,
        tfSha512ContextLen
#else /* !TM_USE_SHA384 */
        0, 0, 0, 0
#endif /* TM_USE_SHA384 */
    },
/* HMAC_SHA512 */
    { 
        SADB_AALG_SHA512HMAC, 512, 512, 512, 512,
#ifdef TM_USE_SHAHMAC_96
            96,
#else /* TM_USE_SHAHMAC_96 */
            256,
#endif /* TM_USE_SHAHMAC_96 */
#ifdef TM_AALG_VAR_BLOCK_SIZE
        128, /* block size */
#endif /* TM_AALG_VAR_BLOCK_SIZE */
#ifdef TM_USE_SHA512
        tfSha512Init, tfSha512Update, tfSha512Final,
        tfSha512ContextLen
#else /* !TM_USE_SHA512 */
        0, 0, 0, 0
#endif /* TM_USE_SHA512 */
    },
/* HMAC_RIPEMD */
    { 
        SADB_AALG_RIPEMDHMAC, 160, 160, 160, 160, 96,
#ifdef TM_AALG_VAR_BLOCK_SIZE
        64, /* block size */
#endif /* TM_AALG_VAR_BLOCK_SIZE */
#ifdef TM_USE_RIPEMD
        tfRmd160Init, tfRmd160Update, tfRmd160Final,
        tfRmd160ContextLen
#else /* TM_USE_RIPEMD */
        0, 0, 0, 0
#endif /* TM_USE_RIPEMD */
    },
/* MD2 */
    { 
        SADB_AALG_MD2HMAC, 128, 128, 128,128, 96,
#ifdef TM_AALG_VAR_BLOCK_SIZE
        64, /* block size */
#endif /* TM_AALG_VAR_BLOCK_SIZE */
#ifdef TM_USE_MD2
        tfMd2Init, tfMd2Update, tfMd2Final,
        tfMd2ContextLen
#else /* TM_USE_MD2 */
        0, 0, 0, 0
#endif /* TM_USE_MD2 */
    },
/* AES-XCBC */
    {
        SADB_AALG_AESXCBCMAC, 128, 128, 128, 128, 96,
#ifdef TM_AALG_VAR_BLOCK_SIZE
        16, /* block size */
#endif /* TM_AALG_VAR_BLOCK_SIZE */
#if (defined(TM_USE_AESXCBC) && defined(TM_USE_AES))
        tfAesXcbcInit, tfAesXcbcUpdate, tfAesXcbcFinal,
        tfAesXcbcContextLen
#else /* !(defined(TM_USE_AESXCBC) && defined(TM_USE_AES)) */
        0, 0, 0, 0
#endif /* (defined(TM_USE_AESXCBC) && defined(TM_USE_AES)) */
    }
};

ttAhAlgorithmPtr tfAhAlgorithmLookup(int idx)
{
    int               i;
    ttAhAlgorithmPtr  aPtr;

    aPtr = TM_AHALGORITHM_NULL_PTR;

    for(i=0; i < (int)(sizeof(ahAlgorithms)/sizeof(ttAhAlgorithm)); i++)
    {
        if((ttUser16Bit)idx == ahAlgorithms[i].aalgName)
        {
            aPtr = (ttAhAlgorithmPtr)&ahAlgorithms[i];
            break;
        }
    }

    return aPtr;
}

void tfAhClearContext(ttVoidPtr  contextPtr)
{
    if(contextPtr)
    {
        tm_free_raw_buffer(contextPtr);
    }

}

#ifdef TM_USE_MD5
static int tfMd5ContextLen(void)
{
    return sizeof(ttMd5Ctx);
}
#endif /* TM_USE_MD5 */

#ifdef TM_USE_MD2
static int tfMd2ContextLen(void)
{
    return sizeof(ttMd2Ctx);
}
#endif /* TM_USE_MD2 */

#ifdef TM_USE_SHA1
static int tfSha1ContextLen(void)
{
    return sizeof(ttSha1Ctx);
}
#endif /* TM_USE_SHA1 */


#ifdef TM_USE_SHA256
static int tfSha256ContextLen(void)
{
    return sizeof(ttSha256Ctx);
}
#endif /* TM_USE_SHA256 */

#if ( defined(TM_USE_SHA384) || defined(TM_USE_SHA512) )
static int tfSha512ContextLen(void)
{
    return sizeof(ttSha512Ctx);
}
#endif /* TM_USE_SHA384 || TM_USE_SHA512 */

#ifdef TM_USE_AESXCBC
static int tfAesXcbcContextLen(void)
{
    return sizeof(ttAesXcbcCtx);
}
#endif /* TM_USE_AESXCBC */

#ifdef TM_USE_RIPEMD
static int tfRmd160ContextLen(void)
{
    return sizeof(ttRmd160Ctx);
}
#endif /* TM_USE_RIPEMD*/

int tfAhContext(ttAhAlgorithmPtr algoPtr, ttGenericKeyPtr gkeyPtr)
{

    int      errorCode;
    errorCode = TM_ENOERROR; 
/* check for key length */
    if (gkeyPtr->keyBits < (ttUser32Bit)algoPtr->aalgKeyMin ||
        gkeyPtr->keyBits > (ttUser32Bit)algoPtr->aalgKeyMax )
    {
/* rather, we also could hash out a good key length */
        errorCode =  TM_EINVAL;
        goto CONTEXT_RETURN;
    }

/* already allocated */
    if (gkeyPtr->keyContextPtr && gkeyPtr->keyContextLen != 0)
    {
        goto CONTEXT_RETURN;
    }
/* no context necessary */
    if (!algoPtr->aalgContextLenFuncPtr)
    {
        goto CONTEXT_RETURN;
    }

    gkeyPtr->keyContextLen = (tt16Bit)
             algoPtr->aalgContextLenFuncPtr();
    gkeyPtr->keyContextPtr = tm_get_raw_buffer(gkeyPtr->keyContextLen);
    if (!gkeyPtr->keyContextPtr) 
    {
        gkeyPtr->keyContextLen = 0;
        errorCode =  TM_ENOBUFS;
        goto CONTEXT_RETURN;
    }

CONTEXT_RETURN:
    return errorCode;
}

/* tfAhMac handles both CMAC and HMAC
 * startOff: from where to authenticate, offset from the pktLinkDataPtr 
 * outPositionPtr:  where to write the authenticate data 
 * We have extraDataPtr and extraLen because SSL is going to add some
 * offline data into the hmac hashing. Those data is not in the network
 * data stream.
 *
 * This function is called to compute the hash of a packet.  If the
 * encryption key was generated by IKE, key length will not be an issue.
 * If a manual SA is used, the key length is specific to the algorithm.
 * Since no algorithm supports a 64-byte key, we do not need to worry about
 * supporting large keys as in tfCryptoSoftwareMacHash.
 */
int tfAhMac(ttPacketPtr       packetPtr,
             int              startOff, 
             tt8BitPtr        extraDataPtr,
             ttPktLen         extraLen,
             ttPktLen         totalAuth, /* not including extraLen */
             tt8BitPtr        outPositionPtr,  
             ttGenericKeyPtr  gkeyPtr,
             ttAhAlgorithmPtr algoPtr,
             tt8Bit           needTruncate,
             tt8Bit           doHmac)
{
    ttGenericCmacCtx     genCtx;
    tt8BitPtr            outputPtr;
    tt8BitPtr            keyInPadPtr;
    tt8BitPtr            keyOutPadPtr;
    tt8BitPtr            tempDigestPtr;
    int                  i;
    ttPktLen             currentAuth;
    int                  errorCode;
    unsigned int         blockSize;
#if (defined(TM_USE_IPSEC) && defined(TM_USE_IPV6))
    ttPktLen             beforeFrag;
    ttPktLen             afterFrag;
    tt8Bit               hdrType;
    tt6GenExtHdrPtr      extPtr;
    tt6IpHeaderPtr       iph6Ptr;
    tt8BitPtr            nextHdrPtr;
    ttPktLen             thisExtLen;
    tt8Bit               origHdrType;
#endif /* (defined(TM_USE_IPSEC) && defined(TM_USE_IPV6)) */

/* To avoid compiler warnings */
    keyInPadPtr = (tt8BitPtr)0;
    keyOutPadPtr = (tt8BitPtr)0;
    blockSize = (unsigned int)0;
    
#ifdef TM_AALG_VAR_BLOCK_SIZE
#ifdef TM_USE_SHAHMAC_96
    blockSize = TM_HMAC_BLOCK_SIZE;
#else /* !TM_USE_SHAHMAC_96 */
    blockSize = algoPtr->aalgBlockSize;
#endif /* TM_USE_SHAHMAC_96 */
#else /* !TM_AALG_VAR_BLOCK_SIZE */
    blockSize = TM_HMAC_BLOCK_SIZE;
#endif /* TM_AALG_VAR_BLOCK_SIZE */

    if (doHmac)
    {
/* Only create one buffer to avoid fragmentation  */
        tempDigestPtr = tm_get_raw_buffer((algoPtr->aalgDigestOutBits << 3) +
                                          2*blockSize);
        keyInPadPtr = tempDigestPtr + blockSize;
        keyOutPadPtr = keyInPadPtr + blockSize;
    }
    else
    {
        tempDigestPtr = tm_get_raw_buffer(algoPtr->aalgDigestOutBits>>3);
    }

    if (tempDigestPtr == (tt8BitPtr)0)
    {
        errorCode = TM_ENOBUFS;
        goto AH_MAC_RETURN;
    }

    if(needTruncate == TM_8BIT_YES)
    {
/* if truncate is used, the outPositionPtr may be only 12 bytes (IPsec)
 * we must save temprary hash results into tempDigest buffer;
 * */
        outputPtr = tempDigestPtr;
    }
    else
    {
        outputPtr = outPositionPtr;
    }

    errorCode = TM_ENOERROR;
    if(algoPtr->aalgName == SADB_AALG_NULL)
    {
        goto AH_MAC_RETURN;
    }
    
    if (doHmac)
    {
/*
 * the HMAC_SHA transform looks like:
 * SHA(K XOR opad, SHA(K XOR ipad, text))
 * where K is an n byte key
 * ipad is the byte 0x36 repeated 64 times
 * opad is the byte 0x5c repeated 64 times
 * and text is the data being protected
 */
/* start out by storing key in pads */
        tm_bzero( keyInPadPtr, blockSize);
        tm_bzero( keyOutPadPtr, blockSize);
        tm_bcopy( gkeyPtr->keyDataPtr, keyInPadPtr, gkeyPtr->keyBits >>3);
        tm_bcopy( gkeyPtr->keyDataPtr, keyOutPadPtr, gkeyPtr->keyBits >>3);

/* XOR key with ipad and opad values */
        for (i=0; i < (int)blockSize; i++) 
        {
            keyInPadPtr[i] ^= TM_CRYPTO_HMAC_INNER_PAD;
            keyOutPadPtr[i] ^= TM_CRYPTO_HMAC_OUTER_PAD;
        }

/*
 * perform inner Authentication algorithm
 */
        algoPtr->aalgInitFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr));
        algoPtr->aalgUpdateFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr), 
                                   keyInPadPtr, 
                                   blockSize, 
                                   0);
    }
    else
    {
/* Since the only CMAC algorithm supported at this time is AES-XCBC-MAC,
 * test for default key length since 128 bits is the only size supported
 * by AES-XCBC-MAC-96
 */
        if (gkeyPtr->keyBits != algoPtr->aalgKeyDefault)
        {
            errorCode = TM_EINVAL;
            goto AH_MAC_RETURN;
        }

        genCtx.keyPtr = gkeyPtr->keyDataPtr;
        genCtx.contextPtr = gkeyPtr->keyContextPtr;
        algoPtr->aalgInitFuncPtr((ttVoidPtr)&genCtx);
    }

    if(extraDataPtr)
    {
        algoPtr->aalgUpdateFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr), 
                               extraDataPtr, 
                               extraLen, 
                               0);    
    }

#if (defined(TM_USE_IPSEC) && defined(TM_USE_IPV6))
    if(tm_16bit_one_bit_set(packetPtr->pktFlags2, 
                TM_PF2_FRAGHEADER_NOT_IN_AH))
    {
/* this packet is from fragment reassembling. we have a fragment
 * header that should not be included in the AH computation
 */     if ((packetPtr->pktLinkDataLength - (ttPktLen)startOff) > totalAuth)
        {
            currentAuth = totalAuth;
        }
        else
        {
            currentAuth = (packetPtr->pktLinkDataLength - (ttPktLen)startOff);
        }

        iph6Ptr = (tt6IpHeaderPtr)(packetPtr->pktLinkDataPtr);
        hdrType = iph6Ptr->iph6Nxt;
        beforeFrag = TM_6_IP_MIN_HDR_LEN;
        extPtr = (tt6GenExtHdrPtr)((tt8BitPtr)iph6Ptr + beforeFrag);
        nextHdrPtr = (tt8BitPtr)(iph6Ptr) + 6;

        while(hdrType != IPPROTO_FRAGMENT)
        {
            hdrType = extPtr->geNextHdr;
            thisExtLen = (extPtr->geExtLen + 1) << 3;
            beforeFrag += thisExtLen;
            nextHdrPtr = (tt8BitPtr)extPtr;
            extPtr = (tt6GenExtHdrPtr)((tt8BitPtr)extPtr + thisExtLen);
        }
   
        origHdrType = *nextHdrPtr;
        *nextHdrPtr = extPtr->geNextHdr;
        afterFrag = currentAuth - beforeFrag - TM_6_IP_FRAG_EXT_HDR_LEN 
                    + (ttPktLen)startOff;

/* get the fragment extension header position. so that 
 * currentAuth = (beforeFrag - startoff) + Fragment Header + afterFrag;
 * or 
 */
        if(beforeFrag > (tt32Bit)startOff)
        {
            algoPtr->aalgUpdateFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr), 
                                       packetPtr->pktLinkDataPtr + startOff,
                                       beforeFrag - (ttPktLen)startOff, 
                                       0);
        
/* skip the Fragment extension header */
            algoPtr->aalgUpdateFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr), 
                                       packetPtr->pktLinkDataPtr 
                                       + beforeFrag 
                                       + TM_6_IP_FRAG_EXT_HDR_LEN, 
                                       afterFrag, 
                                       0);
        }
        else
        {
            algoPtr->aalgUpdateFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr), 
                                       packetPtr->pktLinkDataPtr + startOff, 
                                       currentAuth, 
                                       0);
        }
        tm_16bit_clr_bit(packetPtr->pktFlags2, TM_PF2_FRAGHEADER_NOT_IN_AH);
        *nextHdrPtr = origHdrType;
    }
    else
#endif /* (defined(TM_USE_IPSEC) && defined(TM_USE_IPV6)) */
    {
        if ((packetPtr->pktLinkDataLength - (ttPktLen)startOff) > totalAuth)
        {
            currentAuth = totalAuth;
        }
        else
        {
            currentAuth = (packetPtr->pktLinkDataLength - (ttPktLen)startOff);
        }

        algoPtr->aalgUpdateFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr), 
                                   packetPtr->pktLinkDataPtr + startOff, 
                                   currentAuth, 
                                   0);
/* then text of datagram */
    }
        
    totalAuth -= currentAuth;

    while( totalAuth != 0 && 
           packetPtr->pktLinkNextPtr != TM_PACKET_NULL_PTR)
    {
        packetPtr = packetPtr->pktLinkNextPtr;
        if (packetPtr->pktLinkDataLength  > totalAuth)
        {
            currentAuth = totalAuth;
        }
        else
        {
            currentAuth = packetPtr->pktLinkDataLength;
        }
        algoPtr->aalgUpdateFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr), 
                                   packetPtr->pktLinkDataPtr,
                                   currentAuth, 
                                   0);
/* then text of datagram */

        if (totalAuth >= currentAuth)
        {
            totalAuth -= currentAuth;
        }
    } 
/* authenticate the packet link */

    if(totalAuth != 0) 
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfAhHmac", 
                        "MAC version authentication chained data error");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_EINVAL;
        goto AH_MAC_RETURN;
    }

    algoPtr->aalgFinalFuncPtr(outputPtr, 
                              (ttVoidPtr)(gkeyPtr->keyContextPtr));

    if (doHmac)
    {
/* finish up 1st pass */

/* perform outer Authentication algorithm */
        algoPtr->aalgInitFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr));
        algoPtr->aalgUpdateFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr), 
                                   keyOutPadPtr, 
                                   blockSize, 
                                   0);   
/* start with outer pad */
        algoPtr->aalgUpdateFuncPtr((ttVoidPtr)(gkeyPtr->keyContextPtr), 
                                   outputPtr, 
                                   algoPtr->aalgDigestOutBits>>3, 
                                   0);  
/* then results of 1st hash */
        algoPtr->aalgFinalFuncPtr(outputPtr, 
                                  (ttVoidPtr)(gkeyPtr->keyContextPtr));
/* finish up 2nd pass */
    }

    if(needTruncate)
    {
        tm_memcpy(outPositionPtr,
                  outputPtr,
                  algoPtr->aalgDigestTruncateBits>>3);
    }
#ifdef TM_IPSEC_DEBUG
    tm_debug_log0("Mac = ");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        tempDigestPtr, TM_ESP_AH_TRAILER_SIZE, (ttIntPtr)0);
#endif /* TM_IPSEC_DEBUG */

AH_MAC_RETURN:
    if (tempDigestPtr != (tt8BitPtr)0)
    {
        tm_free_raw_buffer(tempDigestPtr);
    }

    return errorCode;
}
