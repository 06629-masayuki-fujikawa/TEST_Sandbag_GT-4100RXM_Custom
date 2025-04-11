/*
 * Description:  ESP algorithems
 *
 * Filename: trespcor.c
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: source/cryptlib/trespcor.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/10/12 02:40:11JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#include <trcrylib.h>

/* esp core */
/*    $KAME: esp_core.c,v 1.50 2000/11/02 12:27:38 itojun Exp $    */

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* NULL-ENCRYPT iv size is zero, otherwise, need one explicit initial value
 * when calculate padlen, ivsize should use 4 
 */
static const ttEspAlgorithm TM_CONST_QLF espAlgorithms[] = {
/* null */
    {  
       SADB_EALG_NULL,(tt8Bit)0, (tt8Bit)1,
       (tt16Bit)0, (tt16Bit)2048, (tt16Bit)0, 
       0,0,
       0,0 
    },
/*rc2-cbc */
    {
       SADB_EALG_RC2CBC,(tt8Bit)8, (tt8Bit)1, 
       (tt16Bit)8, (tt16Bit)1024,(tt16Bit)128, 
#ifdef TM_USE_RC2
       tfEspRc2SchedLen,     tfEspRc2Schedule,
       tfEspRc2BlockDecrypt, tfEspRc2BlockEncrypt
#else /* TM_USE_RC2 */
       0, 0, 0, 0/* NULL function pointer */
#endif /* TM_USE_RC2 */
    },
/* des-cbc */
    { 
       SADB_EALG_DESCBC,(tt8Bit)8, (tt8Bit)1, 
       (tt16Bit)64, (tt16Bit)64,(tt16Bit)64, 
#ifdef TM_USE_DES
       tfEspDesSchedLen, tfEspDesSchedule, 
       tfEspDesBlockDecrypt, tfEspDesBlockEncrypt 
#else /* TM_USE_DES */
       0, 0, 0, 0/* NULL function pointer */
#endif /* TM_USE_DES */
    },
/* 3des-cbc */
    { 
       SADB_EALG_3DESCBC,(tt8Bit)8, (tt8Bit)1, 
       (tt16Bit)192, (tt16Bit)192,(tt16Bit)192, 
#ifdef TM_USE_3DES
       tfEsp3DesSchedLen, tfEsp3DesSchedule,    
       tfEsp3DesBlockDecrypt, tfEsp3DesBlockEncrypt
#else /* TM_USE_3DES */
       0, 0, 0, 0/* NULL function pointer */
#endif /* TM_USE_3DES */
    },
/* blowfish-cbc */
    { 
       SADB_EALG_BLOWFISHCBC,(tt8Bit)8, (tt8Bit)1, 
       (tt16Bit)40, (tt16Bit)448, (tt16Bit)128, 
#ifdef TM_USE_BLOWFISH
       tfEspBlowfishSchedLen, tfEspBlowfishSchedule,
       tfEspBlowfishBlockDecrypt, tfEspBlowfishBlockEncrypt 
#else /* TM_USE_BLOWFISH */
       0, 0, 0, 0/* NULL function pointer */
#endif /* TM_USE_BLOWFISH */
    },
/* cast128-cbc */
    { 
       SADB_EALG_CAST128CBC,(tt8Bit)8, (tt8Bit)1, 
       (tt16Bit)40, (tt16Bit)128,  (tt16Bit)128,
#ifdef TM_USE_CAST128
       tfEspCast128SchedLen, tfEspCast128Schedule,
       tfEspCast128BlockDecrypt, tfEspCast128BlockEncrypt 
#else /* TM_USE_CAST128 */
       0, 0, 0, 0/* NULL function pointer */
#endif /* TM_USE_CAST128 */
    },
/* aes-aes-cbc */
    { 
       SADB_EALG_AESCBC,(tt8Bit)16,(tt8Bit)1, 
       (tt16Bit)128, (tt16Bit)256,  (tt16Bit)128,
#ifdef TM_USE_AES
       tfEspAesSchedLen, tfEspAesSchedule,
       tfEspAesBlockDecrypt, tfEspAesBlockEncrypt 
#else /*TM_USE_AES */
       0, 0, 0, 0/* NULL function pointer */
#endif /* TM_USE_AES */
    },
/* aes-aes-ctr */
    { 
       SADB_EALG_AESCTR,(tt8Bit)16,(tt8Bit)1, 
       (tt16Bit)128, (tt16Bit)256,  (tt16Bit)128,
#ifdef TM_USE_AES
       tfEspAesSchedLen, tfEspAesSchedule,
       tfEspAesBlockDecrypt, tfEspAesBlockEncrypt 
#else /* !TM_USE_AES */
       0, 0, 0, 0 /* NULL function pointer */
#endif /* TM_USE_AES */
    },
/* twofish-cbc */
    { 
       SADB_EALG_TWOFISHCBC,(tt8Bit)16, (tt8Bit)1, 
       (tt16Bit)128, (tt16Bit)256,  (tt16Bit)128,
#ifdef TM_USE_TWOFISH
       tfEspTwofishSchedLen, tfEspTwofishSchedule,
       tfEspTwofishBlockDecrypt, tfEspTwofishBlockEncrypt 
#else /*TM_USE_TWOFISH */
       0, 0, 0, 0/* NULL function pointer */
#endif /* TM_USE_TWOFISH */
    },
/* alleged rc four*/
    {
        SADB_EALG_ARCFOUR, (tt8Bit)1, (tt8Bit)1,
        (tt16Bit)40, (tt16Bit)2048, (tt16Bit)128,
#ifdef TM_USE_ARCFOUR
        tfEspArc4SchedLen, tfEspArc4Schedule,
/* ARC4 is not block cipher, we don't have block cipher
 * function
 */
        0,                 0
#else  /* TM_USE_ARCFOUR*/
        0, 0, 0, 0
#endif /* TM_USE_ARCFOUR*/
    }
#if (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST))
    ,
/* aes-ccm-8 */
    {
        SADB_EALG_AESCCM8, (tt8Bit)16, (tt8Bit)1,
        (tt16Bit)128, (tt16Bit)256,  (tt16Bit)128,
       0, 0, 0, 0/* NULL function pointer */
    },
/* aes-ccm-12 */
    {
        SADB_EALG_AESCCM12, (tt8Bit)16, (tt8Bit)1,
        (tt16Bit)128, (tt16Bit)256,  (tt16Bit)128,
       0, 0, 0, 0/* NULL function pointer */
    },
/* aes-ccm-16 */
    {
        SADB_EALG_AESCCM16, (tt8Bit)16, (tt8Bit)1,
        (tt16Bit)128, (tt16Bit)256,  (tt16Bit)128,
       0, 0, 0, 0/* NULL function pointer */
    }
#endif /* (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST)) */
};


ttEspAlgorithmPtr tfEspAlgorithmLookup(int idx)
{

    int                     i;
    ttEspAlgorithmPtr       ePtr;
    ttCryptoEngineListPtr   celPtr;
    ttCryptoEnginePtr       cryptoenginePtr;

    ePtr = (ttEspAlgorithmPtr) 0;


    if(idx == SADB_EALG_DESIV64CBC)
    {
        idx = SADB_EALG_DESCBC;
    }
/* at least one crypto engine supports this algorithm */
    celPtr = ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))
                              ->clgCryptoEngineHeadPtr;
    while(celPtr)
    {
        cryptoenginePtr = celPtr->celCryptoEnginePtr;
        if(cryptoenginePtr->ceSupportFlags & (TM_UL(1) << idx))
        {
            break;
        }
        celPtr = celPtr->celNextPtr;
    }

    if(!celPtr)
    {
/* no crypto engine supports this algorithm, return NULL */
        goto LOOKUP_RETURN;
    }

    for(i=0; i < (int)(sizeof(espAlgorithms)/sizeof(ttEspAlgorithm)); i++)
    {
        if(idx == espAlgorithms[i].ealgName)
        {
            ePtr = (ttEspAlgorithmPtr)&espAlgorithms[i];
            break;
        }
    }

LOOKUP_RETURN:
    return ePtr;
}


int tfEspSchedule(ttEspAlgorithmPtr algoPtr, ttGenericKeyPtr gkeyPtr)
{
    int error;

    error = TM_ENOERROR;

/* check for key length */
    if (gkeyPtr->keyBits < (tt32Bit)algoPtr->ealgKeyMin ||
        gkeyPtr->keyBits > (tt32Bit)algoPtr->ealgKeyMax )
    {
        error = TM_EINVAL;
        goto SCHEDULE_RETURN;
    }

/* already allocated */
    if ((gkeyPtr->keySchedulePtr!= 0) && (gkeyPtr->keyScheduleLen != 0))
    {
        goto SCHEDULE_RETURN;
    }
/* no schedule necessary */
    if (!algoPtr->ealgScheduleFuncPtr || !algoPtr->ealgScheduleLenFuncPtr)
    {
        goto SCHEDULE_RETURN;
    }

    gkeyPtr->keyScheduleLen = (ttUser16Bit)
                 algoPtr->ealgScheduleLenFuncPtr();
    gkeyPtr->keySchedulePtr = tm_get_raw_buffer(gkeyPtr->keyScheduleLen);
    if (!gkeyPtr->keySchedulePtr) 
    {
        gkeyPtr->keyScheduleLen = 0;
        error = TM_ENOBUFS;
        goto SCHEDULE_RETURN;

    }

    error = algoPtr->ealgScheduleFuncPtr(algoPtr, gkeyPtr);
    if (error) 
    {
        tfEspClearSchedule(gkeyPtr->keySchedulePtr);
        gkeyPtr->keySchedulePtr = TM_VOID_NULL_PTR;
        gkeyPtr->keyScheduleLen = 0;
    }

SCHEDULE_RETURN:
    return error;
}

void tfEspClearSchedule(ttUserVoidPtr  schedulePtr)
{
    if(schedulePtr)
    {
        tm_free_raw_buffer (schedulePtr);
    }
}

int tfEspDataCbcDecrypt(tt8BitPtr         ivPtr,
                        tt8BitPtr         dataSrcPtr,
                        tt8BitPtr         dataDstPtr,
                        ttPktLen          payloadLen,
                        ttGenericKeyPtr   gkeyPtr,
                        ttEspAlgorithmPtr algoPtr)
{
    int       i;
    tt8Bit    blockLen;
    tt8Bit    iv[TM_ESP_MAX_IV_LEN];
    tt8BitPtr p;
    ttPktLen  startAt;
    tt8Bit    q[TM_ESP_MAX_IV_LEN];
    int       errorCode;


    errorCode = TM_ENOERROR;
    blockLen = algoPtr->ealgBlockSize;

    if (blockLen > TM_ESP_MAX_IV_LEN) 
    {
        errorCode = TM_EINVAL;
        goto DATACDECRYPT_RETURN;
    }


    if(payloadLen % blockLen != 0)
    {
        errorCode = TM_EINVAL;
        goto DATACDECRYPT_RETURN;
    }

    startAt = 0;


    tm_bcopy(ivPtr, iv, blockLen);

    while (startAt < payloadLen) 
    {
        p = dataSrcPtr + startAt;
        tm_bcopy(p, q, blockLen); 
/*save the cipher text to use next step */
/* decrypt */
        if(algoPtr->ealgBlockDecryptFuncPtr)
        {
            algoPtr->ealgBlockDecryptFuncPtr(algoPtr, gkeyPtr, p, dataDstPtr);
        }

        for (i = 0; i < blockLen; i++) 
        {
            dataDstPtr[i] ^= iv[i]; 
        }

        dataDstPtr += blockLen;
        tm_bcopy(q, iv, blockLen);
        startAt += blockLen;
    }


DATACDECRYPT_RETURN:
    return errorCode;

}

/*  input data buffer is allocated outside tfEspDataCbcEncrypt(),
 *  already padded, ip,spi,seq,iv,data,pad,padLen.nextheader are all ready,
 *  authenticate field is reserved.
 *  The only thing is to encrypt the payload data and store them back.
 * 
 */
int tfEspDataCbcEncrypt(tt8BitPtr         ivPtr, 
                        tt8BitPtr         dataSrcPtr,
                        tt8BitPtr         dataDstPtr,
                        ttPktLen          payloadLen,
                        ttGenericKeyPtr   keyPtr,
                        ttEspAlgorithmPtr algoPtr)
{
    int       i;
    tt8Bit    blockLen;
    ttPktLen  startAt;
    tt8BitPtr p;
    tt8BitPtr q;
    int       errorCode;


    errorCode = TM_ENOERROR;

    blockLen = algoPtr->ealgBlockSize;

    if (blockLen > TM_ESP_MAX_IV_LEN) 
    {
        errorCode = TM_EINVAL;
        goto CBCDATAENCRYPT_RETURN;
    }

/* check data length must be multiple to blockLen */
    if(payloadLen % blockLen != 0)
    {
        errorCode = TM_EINVAL;
        goto CBCDATAENCRYPT_RETURN;
    }


    startAt = 0;

    while (startAt < payloadLen) 
    {
        p = dataSrcPtr + startAt;
        for (i = 0; i < blockLen; i++) 
        {
            p[i] ^= ivPtr[i]; 
/* xor cipher text q (iv initially) with the 
 * plain data p, use the new q as input block to des machine
 * store the cipher text into inPacketDataPtr 
 * and save the cipher text as q for next step 
 */
        }
/* encrypt */
        q = dataDstPtr + startAt;
        if(algoPtr->ealgBlockEncryptFuncPtr)
        {
            algoPtr->ealgBlockEncryptFuncPtr(algoPtr, keyPtr, p, q);
        }

        ivPtr = q;
        startAt += blockLen;
    }

CBCDATAENCRYPT_RETURN:
    return errorCode;
}

/* dataSrcPtr is encrypted, dataDstPtr will be clear */
int tfEspDataCtrDecrypt(tt32BitPtr        noncePtr,
                        tt8BitPtr         ivPtr,
                        tt32Bit           ivLen,
                        tt8BitPtr         dataSrcPtr,
                        tt8BitPtr         dataDstPtr,
                        ttPktLen          payloadLen,
                        ttGenericKeyPtr   keyPtr,
                        ttEspAlgorithmPtr algoPtr)
{
    return tfEspDataCtrEncrypt(noncePtr,
                               ivPtr,
                               ivLen,
                               dataSrcPtr,
                               dataDstPtr,
                               payloadLen,
                               keyPtr,
                               algoPtr);
}

/* Note at the end of this function,
 * dataSrcPtr is NOT changed,
 * ivPtr, noncePtr, and keyPtr are also unchanged.
 * Currently, this funtion is configured specifically for 
 * AES-CTR with a 4-byte nonce, 8-byte IV, and 4-byte counter.
 */
int tfEspDataCtrEncrypt(tt32BitPtr        noncePtr,
                        tt8BitPtr         ivPtr,
                        tt32Bit           ivLen,
                        tt8BitPtr         dataSrcPtr,
                        tt8BitPtr         dataDstPtr,
                        ttPktLen          payloadLen,
                        ttGenericKeyPtr   keyPtr,
                        ttEspAlgorithmPtr algoPtr)
{
    tt8Bit          blockLen;
    int             errorCode;
    ttPktLen        i;
    ttCounterBlock  counterBlockClear;
    ttCounterBlock  counterBlockCode;
    tt32Bit         blockCounter;
    tt8Bit          bytesInBlockTodo;

    errorCode = TM_ENOERROR;
    blockLen = algoPtr->ealgBlockSize;

    if (blockLen > TM_ESP_MAX_IV_LEN) 
    {
        errorCode = TM_EINVAL;
        goto CTRDATAENCRYPT_RETURN;
    }

/* the counter block is composed of the nonce (4 bytes), iv (8 Bytes), 
 * and block counter (4 Bytes) Nonce|IV|BlockCounter
 */

/* nonce MUST be a 32-bit value */
    counterBlockClear.nonce = *noncePtr;
    tm_memcpy(counterBlockClear.iv, ivPtr, ivLen);      
/* The counter ALWAYS starts at 1, it is set immidiately in the for loop. */
    blockCounter = 0;
        
    while (payloadLen) 
    {
        if (payloadLen > blockLen)
        {
            bytesInBlockTodo = blockLen;
        }
        else
        {
            bytesInBlockTodo = (tt8Bit)payloadLen;
        }
/* encrypt */
/* increase the value of the counter, store it in network byte order */
/* update the key stream once per block */
        blockCounter++;
        counterBlockClear.ctr = htonl(blockCounter);
        if(algoPtr->ealgBlockEncryptFuncPtr)
        {
            algoPtr->ealgBlockEncryptFuncPtr(algoPtr,
                                             keyPtr,
                                             (ttUser8BitPtr)&counterBlockClear,
                                             (ttUser8BitPtr)&counterBlockCode);
        }
        
        for (i = 0; i < bytesInBlockTodo; i++)
        {
/* after creating the key stream, xor it with plain text */
            dataDstPtr[i] = (tt8Bit)(dataSrcPtr[i] ^
                                    ((tt8BitPtr)&counterBlockCode)[i]);
        }
        payloadLen -= bytesInBlockTodo;
        dataDstPtr += bytesInBlockTodo;
        dataSrcPtr += bytesInBlockTodo;
    }
CTRDATAENCRYPT_RETURN:
    return errorCode;
}

int tfEspPacketCbcDecrypt(tt8BitPtr           ivPtr,
                          ttPacketPtr         packetPtr, 
                          ttPktLen            offData, 
                          ttPacketPtr         newPacketPtr,
                          ttPktLen            payloadLen,
                          ttGenericKeyPtr     keyPtr,
                          ttEspAlgorithmPtr   algoPtr,
                          ttVoidPtr           additionalData)
{
    int           i;
    tt8Bit        blockLen;
    tt8BitPtr     cipherPtr;
    tt8BitPtr     plainPtr;
    ttPacketPtr   tempPacketPtr;
    tt8Bit        savedCipherBlock[TM_ESP_MAX_IV_LEN];
    tt8Bit        xorData[TM_ESP_MAX_IV_LEN];
    tt8Bit        updateIv;
    ttPktLen      currentPayload;
    ttPktLen      origPayloadLen;
/*for scattered data, they may not be aligned*/
    tt8Bit        notAlignedData[TM_ESP_MAX_IV_LEN]; 
    int           remainData;
    int           copyLen;
    int           origRemainData;
    int           errorCode;
#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    ttCharPtr     printBufPtr;
    ttPacketPtr   origPacketPtr;
    ttPktLen      printLen;
    int           offset;

    origPacketPtr = packetPtr;
    offset = 0;
#endif /* (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG)) */

    updateIv = *(tt8BitPtr)additionalData;
    errorCode = TM_ENOERROR;
    plainPtr = (tt8BitPtr)0;
    blockLen = algoPtr->ealgBlockSize;
    origPayloadLen = payloadLen;

#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    tm_debug_log0("Decrypt iv = \n");
    tfCryptoDebugPrintf((ttCharPtrPtr)0, ivPtr, blockLen, 0);
    tm_debug_log0("Decrypt key = \n");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        keyPtr->keyDataPtr, (keyPtr->keyBits>>3), 0);
    tm_debug_log1("Before Decrypt, offset = %d, cipher data = \n", offData);
    origPayloadLen += offData;
    tm_debug_log1("Begining of decrypt packet, payloadLen(inc offData) = %d\n",
                  origPayloadLen);
    while(origPacketPtr && (origPayloadLen > 0))
    {
        if (origPayloadLen > origPacketPtr->pktLinkDataLength)
        {
            printLen = origPacketPtr->pktLinkDataLength;
        }
        else
        {
            printLen = origPayloadLen;
        }
        tm_debug_log5("packet =0x%x, linkNextPtr = 0x%x, linkDataLen = %d, " \
                "printLen = %d, offset = %d\n",
                origPacketPtr, origPacketPtr->pktLinkNextPtr, 
                origPacketPtr->pktLinkDataLength,
                printLen, offset);
        origPayloadLen -= printLen;
        origPacketPtr = origPacketPtr->pktLinkNextPtr;
    }
    origPacketPtr = packetPtr;
    origPayloadLen = payloadLen;
    printBufPtr = (ttCharPtr)0;
    while(origPacketPtr && (origPayloadLen > 0))
    {
        if (origPayloadLen > origPacketPtr->pktLinkDataLength)
        {
            printLen = origPacketPtr->pktLinkDataLength;
        }
        else
        {
            printLen = origPayloadLen;
        }
        tfCryptoDebugPrintf(&printBufPtr, origPacketPtr->pktLinkDataPtr,
                            printLen, &offset);
        origPayloadLen -= printLen;
        origPacketPtr = origPacketPtr->pktLinkNextPtr;
    }
    tfCryptoDebugFlush(&printBufPtr);
    tm_debug_log0("\nending of decrypt packet \n");
    origPacketPtr = packetPtr;
    origPayloadLen = payloadLen;
#endif /*TM_IPSEC_DEBUG || TM_SSL_DEBUG */

    if(payloadLen % blockLen != 0)
    {
        errorCode = TM_EINVAL;
        goto PACKETDECRYPT_RETURN;
    }

    if (packetPtr->pktLinkDataLength - offData > payloadLen)
    {
        currentPayload = payloadLen;
    }
    else
    {
        currentPayload = packetPtr->pktLinkDataLength - offData;
    }

    remainData = (int)currentPayload % blockLen;
    currentPayload -= (ttPktLen)remainData;
    tm_bcopy(ivPtr, &xorData[0], blockLen);
    cipherPtr = packetPtr->pktLinkDataPtr + offData; 
    if(newPacketPtr)
    {
/* the newPacketPtr may already have some built-in data, for example
 * SSL record header
 */
        plainPtr = newPacketPtr->pktLinkDataPtr + 
                    newPacketPtr->pktLinkDataLength;
    }

/*first ciphered text block*/
    payloadLen -= currentPayload;

/* Note that, if currentPayload could be zero if it is not at least
 * one cipher block size
 */
    while((currentPayload != (ttPktLen)0) || 
           (payloadLen != 0))
    {
        if(currentPayload != 0)
        {
/* save the cipher text to used in next step*/
            tm_bcopy(cipherPtr, savedCipherBlock,blockLen);
            if(newPacketPtr == (ttPacketPtr)0)
            {
                plainPtr = cipherPtr;
            }
/* decrypt */
            if(algoPtr->ealgBlockDecryptFuncPtr)
            {
                algoPtr->ealgBlockDecryptFuncPtr(algoPtr, 
                       keyPtr, cipherPtr, plainPtr);
            }

/* xor the decrypted result with previous cipher */
            for (i = 0; i < blockLen; i++) 
            {
                plainPtr[i] ^= xorData[i]; 
            }

/* copy previous cipher into xorData for next turn */
            tm_bcopy(savedCipherBlock, xorData, blockLen);
            cipherPtr += blockLen;
            plainPtr += blockLen;
            currentPayload -= blockLen;
        }
        else
        {

            if(packetPtr->pktLinkNextPtr != TM_PACKET_NULL_PTR)
            {
                packetPtr = packetPtr->pktLinkNextPtr;
                tempPacketPtr = packetPtr;
                copyLen = 0;
                if(remainData != 0) 
/* since notAlignedData may be used as the ivPtr, don't make copy
 * before this line 
 */
                {
                    tm_bcopy(cipherPtr, notAlignedData, remainData);
                    origRemainData = remainData;
                    remainData = (tt16Bit)(blockLen - remainData);

                    while(remainData) 
                    { 
                        if(packetPtr->pktLinkDataLength >= 
                                   (tt32Bit)remainData)
                        { 
                            copyLen = remainData;
                        }
                        else
                        {
                            copyLen = (tt16Bit)
                                (packetPtr->pktLinkDataLength);
                        }
                        tm_bcopy(packetPtr->pktLinkDataPtr, 
                            &notAlignedData[blockLen - remainData],
                            copyLen);
                        remainData = (tt16Bit)(remainData - copyLen);
                        if(remainData > 0 || ((ttPktLen) copyLen == 
                                  packetPtr->pktLinkDataLength))
                        {
                            if(packetPtr->pktLinkNextPtr != TM_PACKET_NULL_PTR)
                            {
                                packetPtr = packetPtr->pktLinkNextPtr;
                                if(remainData == 0) 
                                {
                                    copyLen = 0;
                                }
                            }
                            else if(payloadLen > (ttPktLen)blockLen)
                            {
/* if payloadLen > blockLen, we must have more packetPtr,
 * if payloadLen = blockLen, the current packetPtr should be the last one.
 */
#ifdef TM_ERROR_CHECKING
                                tfKernelWarning("tfEspPacketCbcDecrypt", 
                                    "Payload length error");
#endif /* TM_ERROR_CHECKING */
                                errorCode = TM_EINVAL;
                                goto PACKETDECRYPT_RETURN;
                            }
                        }
                    }
/* saved for IV use*/
                    tm_bcopy(notAlignedData, savedCipherBlock, blockLen);
                    if(newPacketPtr == (ttPacketPtr)0)
                    {
                        plainPtr = notAlignedData;
                    }
/* decrypt this block first */
                    if(algoPtr->ealgBlockDecryptFuncPtr)
                    {
                        algoPtr->ealgBlockDecryptFuncPtr(algoPtr, 
                             keyPtr, notAlignedData, plainPtr);
                    }
/* xor with previous cipher */
                    for (i = 0; i < blockLen; i++) 
                    {
                        plainPtr[i] ^= xorData[i]; 
                    }
                    tm_bcopy(savedCipherBlock, xorData, blockLen);

                    if(newPacketPtr == (ttPacketPtr)0)
                    {
/* copy the decrypted data to packet */
                        tm_bcopy(notAlignedData, cipherPtr, origRemainData);
                        remainData = (tt16Bit)(blockLen - origRemainData);
                        while(remainData)
                        {
                            if(tempPacketPtr->pktLinkDataLength >= 
                                      (tt32Bit)remainData)
                            {
                                copyLen = remainData;
                            }
                            else
                            {
                                copyLen = (tt16Bit)
                                        (tempPacketPtr->pktLinkDataLength);
                            }
                            tm_bcopy(&notAlignedData[blockLen - remainData], 
                                     tempPacketPtr->pktLinkDataPtr, 
                                     copyLen);
                            remainData = (tt16Bit)(remainData - copyLen);
                            if(remainData > 0 || ((ttPktLen) copyLen 
                                == tempPacketPtr->pktLinkDataLength))
                            {
                                tempPacketPtr = tempPacketPtr->pktLinkNextPtr;
                                copyLen = 0;
                            }
                        }
                    }
                    else
                    {
                        plainPtr += blockLen;
                    }
                    payloadLen -= blockLen;
                }
                if(payloadLen != 0)
                {
                    cipherPtr = packetPtr->pktLinkDataPtr + copyLen; 
/* remainData: how much already copy to notAlignedData */
                    if ((packetPtr->pktLinkDataLength - (ttPktLen)copyLen) > payloadLen)
                    {
                        currentPayload = payloadLen;
                    }
                    else
                    {
                        currentPayload = 
                            (packetPtr->pktLinkDataLength - (ttPktLen)copyLen);
                    }
                    remainData = (tt16Bit) (currentPayload % blockLen); 
/* this shared data may not aligned either */
                    currentPayload -= (ttPktLen)remainData;
                    payloadLen -= currentPayload;
                }
            }
            else if(payloadLen != 0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfEspPacketCbcDecrypt", 
                                "Payload length error");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_EINVAL;
                goto PACKETDECRYPT_RETURN;
            }
        }
    }

    if(newPacketPtr)
    {
        newPacketPtr->pktLinkDataLength += origPayloadLen;
    }

    if(updateIv != (tt8Bit)0)
    {
        tm_bcopy(savedCipherBlock, ivPtr, blockLen);
    }

#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    tm_debug_log0("\nDecrypt success! plain data = \n");
    offset = 0;
    printLen = origPayloadLen + offData;
    if(newPacketPtr == (ttPacketPtr)0)
    {
        while(origPacketPtr && printLen > 0)
        {
            if (printLen > origPacketPtr->pktLinkDataLength)
            {

                tfCryptoDebugPrintf(&printBufPtr,
                                    origPacketPtr->pktLinkDataPtr,
                                    origPacketPtr->pktLinkDataLength,
                                    &offset);
                printLen -= origPacketPtr->pktLinkDataLength;
            }
            else
            {
                tfCryptoDebugPrintf(&printBufPtr,
                                    origPacketPtr->pktLinkDataPtr,
                                    printLen,
                                    &offset);
                printLen -= printLen;
            }
            origPacketPtr = origPacketPtr->pktLinkNextPtr;
        }
        tfCryptoDebugFlush(&printBufPtr);
    }
    else
    {
        tfCryptoDebugPrintf((ttCharPtrPtr)0,
                newPacketPtr->pktLinkDataPtr +
                newPacketPtr->pktLinkDataLength - origPayloadLen,
                origPayloadLen,
                0);
    }
#endif /*TM_IPSEC_DEBUG || TM_SSL_DEBUG */


PACKETDECRYPT_RETURN:
#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    if(errorCode != TM_ENOERROR)
    {
        tm_debug_log0("Decrypt failed. \n");
    }
#endif /*TM_IPSEC_DEBUG || TM_SSL_DEBUG */

    return errorCode;
}


/*  input data buffer is allocated outside tfEspPacketCbcEncrypt(),
 *  already padded, ip,spi,seq,iv,data,pad,padLen.nextheader are all ready,
 *  authenticate field is reserved.
 *  The only thing is to encrypt the payload data and store them back.
 *  is: encrypt the payload data and store back
 * 
 */
int tfEspPacketCbcEncrypt(tt8BitPtr           ivPtr,
                          ttPacketPtr         packetPtr, 
                          ttPktLen            offData,
                          ttPacketPtr         newPacketPtr,
                          ttPktLen            payloadLen,
                          ttGenericKeyPtr     keyPtr,
                          ttEspAlgorithmPtr   algoPtr,
                          ttVoidPtr           additionalData)
{
    int          i;
    tt8Bit       blockLen;
    tt8BitPtr    plainPtr;
    tt8BitPtr    cipherPtr;
    tt8BitPtr    xorPtr;
    ttPacketPtr  tempPacketPtr;
    ttPktLen     currentPayload;
    ttPktLen     origPayloadLen;
/* for scattered data, they may not be aligned, need to copy here */
    tt8Bit       notAlignedData[TM_ESP_MAX_IV_LEN]; 
    tt8Bit       xorNotAlignedData[TM_ESP_MAX_IV_LEN];
    tt8Bit       updateIv;
    tt16Bit      remainData;
    tt16Bit      origRemainData;
    tt16Bit      copyLen;
    int          errorCode;
#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    ttCharPtr   printBufPtr;
    ttPacketPtr origPacketPtr;
    ttPktLen    printLen;
    int         offset;

    origPacketPtr = packetPtr;
    offset = 0;
#endif /* (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG)) */
    updateIv = *(tt8BitPtr)additionalData;
    errorCode = TM_ENOERROR;
    blockLen = algoPtr->ealgBlockSize;
    copyLen = 0;
    cipherPtr = (tt8BitPtr)0;
    origPayloadLen = payloadLen;

#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    tm_debug_log0("Encrypt iv = \n");
    tfCryptoDebugPrintf((ttCharPtrPtr)0, ivPtr, blockLen, 0);
    tm_debug_log0("Encrypt key = \n");
    tfCryptoDebugPrintf((ttCharPtrPtr)0, keyPtr->keyDataPtr,
                        (keyPtr->keyBits>>3), 0);
    tm_debug_log1("Before encrypt, offset = %d, plain data = \n", offData);
    origPayloadLen += offData;
    tm_debug_log1("Begining of encrypt packet, payloadLen(inc offData) = %d\n",
                  origPayloadLen);
    while(origPacketPtr && (origPayloadLen > 0))
    {
        if (origPayloadLen > origPacketPtr->pktLinkDataLength)
        {
            printLen = origPacketPtr->pktLinkDataLength;
        }
        else
        {
            printLen = origPayloadLen;
        }
        tm_debug_log5("packet =0x%x, linkNextPtr = 0x%x, linkDataLen = %d, " \
                "printLen = %d, offset = %d\n",
                origPacketPtr, origPacketPtr->pktLinkNextPtr, 
                origPacketPtr->pktLinkDataLength,
                printLen, offset);
        origPayloadLen -= printLen;
        origPacketPtr = origPacketPtr->pktLinkNextPtr;
    }
    origPacketPtr = packetPtr;
    origPayloadLen = payloadLen;
    printBufPtr = (ttCharPtr)0;
    while(origPacketPtr && origPayloadLen > 0)
    {
        if (origPayloadLen > origPacketPtr->pktLinkDataLength)
        {
            printLen = origPacketPtr->pktLinkDataLength;
        }
        else
        {
            printLen = origPayloadLen;
        }

        tfCryptoDebugPrintf(&printBufPtr, origPacketPtr->pktLinkDataPtr,
                           printLen,
                           &offset);
        origPayloadLen -= printLen;
        origPacketPtr = origPacketPtr->pktLinkNextPtr;
    }
    tfCryptoDebugFlush(&printBufPtr);
    tm_debug_log0("\nending of encrypt packet \n");
    origPacketPtr = packetPtr;
    origPayloadLen = payloadLen;
#endif /*TM_IPSEC_DEBUG || TM_SSL_DEBUG */

/* check data length must be multiple to blockLen */
    if(payloadLen % blockLen != 0)
    {
        errorCode = TM_EINVAL;
        goto PACKETENCRYPT_RETURN;
    }
    
    if ((packetPtr->pktLinkDataLength - offData) > payloadLen)
    {
        currentPayload = payloadLen;
    }
    else
    {
        currentPayload = packetPtr->pktLinkDataLength - offData;
    }

    remainData = (tt16Bit)(currentPayload % blockLen);
    currentPayload -= remainData;
    
    xorPtr = ivPtr; 
/* initial xor with IV */

    plainPtr = packetPtr->pktLinkDataPtr + offData;
    if(newPacketPtr)
    {
/* the newPacketPtr may already have some built-in data, for example
 * SSL record header
 */
        cipherPtr = newPacketPtr->pktLinkDataPtr + 
                    newPacketPtr->pktLinkDataLength;
    }
    payloadLen -= currentPayload;
    
    while ((currentPayload != (ttPktLen)0) || (payloadLen != 0))
    {
        
        if(currentPayload != 0)
        {
            for (i = 0; i < blockLen; i++)
            {
                notAlignedData[i] = (tt8Bit)(plainPtr[i] ^ xorPtr[i]); 
/* xor cipher text (iv ) with the plain data p, 
 * use the new q as input block to des machine
 * store the cipher text into inPacketDataPtr and save 
 * the cipher text as q for next step 
 */
            }
            
/* encrypt */
            if(newPacketPtr == (ttPacketPtr)0)
            {
                cipherPtr = plainPtr;
            }
            if(algoPtr->ealgBlockEncryptFuncPtr)
            {
                algoPtr->ealgBlockEncryptFuncPtr(algoPtr, 
                         keyPtr, notAlignedData, cipherPtr);
            }
            xorPtr = cipherPtr; 
            plainPtr += blockLen;
            cipherPtr += blockLen;
            currentPayload -= blockLen;
        }
        else 
/* currentPayload */
        {
            if(packetPtr->pktLinkNextPtr != TM_PACKET_NULL_PTR)
/* there is a next link */
            {
                packetPtr = packetPtr->pktLinkNextPtr;
                tempPacketPtr = packetPtr;
                copyLen = 0;
                if (remainData != 0)
/* since notAlignedData may be used as the ivPtr, don't make copy before 
 * this line 
 */
                {
                    tm_bcopy(plainPtr, notAlignedData, remainData);
                    origRemainData = remainData;
                    remainData = (tt16Bit)(blockLen - remainData);
/* need copy from the front to make one block */
                    while(remainData)
                    {
                        if(packetPtr->pktLinkDataLength >=
                           (ttPktLen)remainData)
                        { 
                            copyLen = remainData;
                        }
                        else
                        {
                            copyLen = (tt16Bit)
                                       (packetPtr->pktLinkDataLength);
                        }
                        tm_bcopy(packetPtr->pktLinkDataPtr,
                                 &notAlignedData[blockLen - remainData],
                                 copyLen);
                        remainData = (tt16Bit)(remainData - copyLen);
                        if(remainData > 0 || ((ttPktLen)copyLen
                                  == packetPtr->pktLinkDataLength))
                        {
                            if(packetPtr->pktLinkNextPtr != TM_PACKET_NULL_PTR)
                            {
                                packetPtr = packetPtr->pktLinkNextPtr;
                                if(remainData == 0) 
                                {
                                    copyLen = 0;
                                }
                            }
                            else if(payloadLen > (ttPktLen)blockLen)
                            {
/* if payloadLen > blockLen, we must have more packetPtr,
 * if payloadLen = blockLen, the current packetPtr should be the last one.
 */
#ifdef TM_ERROR_CHECKING
                                tfKernelWarning("tfEspPacketCbcEncrypt", 
                                    "Payload length error");
#endif /* TM_ERROR_CHECKING */
                                errorCode = TM_EINVAL;
                                goto PACKETENCRYPT_RETURN;
                            }
                        }
                    }

                    for (i = 0; i < blockLen; i++) 
                    {
                         notAlignedData[i] ^= xorPtr[i]; 
                    }

/* encrypt this block first */
                    if(newPacketPtr == (ttPacketPtr)0)
                    {
                        cipherPtr = notAlignedData;
                    }
                    if(algoPtr->ealgBlockEncryptFuncPtr)
                    {
                        algoPtr->ealgBlockEncryptFuncPtr(algoPtr, 
                            keyPtr, notAlignedData, cipherPtr);
                    }
                    if(newPacketPtr == (ttPacketPtr)0)
                    {
/* copy the cipher to packet */
                        tm_bcopy(notAlignedData, plainPtr, origRemainData);
                        remainData = (tt16Bit)(blockLen - origRemainData);
                        while(remainData)
                        {
                            if(tempPacketPtr->pktLinkDataLength >=
                               (ttPktLen)remainData)
                            {
                                copyLen = remainData;
                            }
                            else
                            {
                                copyLen = (tt16Bit)
                                        (tempPacketPtr->pktLinkDataLength);
                            }
                            tm_bcopy(&notAlignedData[blockLen - remainData], 
                                     tempPacketPtr->pktLinkDataPtr, 
                                     copyLen);
                            remainData = (tt16Bit)(remainData - copyLen);
                            if(remainData > 0 || ((ttPktLen)copyLen
                                == tempPacketPtr->pktLinkDataLength))
                            {
                                tempPacketPtr = tempPacketPtr->pktLinkNextPtr;
                                copyLen = 0;
                            }
                        }
/* for encryption, we copy the notAlignedData (ciphers)
 * into xor for next step
 */
                        tm_bcopy(notAlignedData, xorNotAlignedData, blockLen);
                        xorPtr = &xorNotAlignedData[0]; 
                    }
                    else
                    {
                        xorPtr = cipherPtr;
                        cipherPtr += blockLen;
                    }
/* used as xor next turn */
                    payloadLen -= blockLen;
                }

                if(payloadLen != 0)
                {
                    plainPtr = packetPtr->pktLinkDataPtr + copyLen; 
                    if ((packetPtr->pktLinkDataLength - copyLen) > payloadLen)
                    {
                        currentPayload =  payloadLen;
                    }
                    else
                    {
                        currentPayload =
                                     (packetPtr->pktLinkDataLength - copyLen);
                    }
                    remainData = (tt16Bit)(currentPayload % blockLen); 
                    currentPayload -= remainData;
                    payloadLen -= currentPayload;
                }
            }
            else if(payloadLen != 0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfEspPacketCbcEncrypt", 
                    "Payload length error");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_EINVAL;
                goto PACKETENCRYPT_RETURN;
            }
        }
    }

    if(newPacketPtr)
    {
        newPacketPtr->pktLinkDataLength += 
                  origPayloadLen;
    }
    if(updateIv != (tt8Bit)0)
    {
        tm_bcopy(cipherPtr - blockLen, ivPtr, blockLen);
    }


#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    tm_debug_log0("\nEncrypt success! Cipher data = \n");
    offset = 0;
    printLen = origPayloadLen + offData;
    if(newPacketPtr == (ttPacketPtr)0)
    {
        while(origPacketPtr && printLen > 0)
        {
            if (printLen > origPacketPtr->pktLinkDataLength)
            {

                tfCryptoDebugPrintf(&printBufPtr,
                                    origPacketPtr->pktLinkDataPtr,
                                    origPacketPtr->pktLinkDataLength,
                                    &offset);
                printLen -= origPacketPtr->pktLinkDataLength;
            }
            else
            {
                tfCryptoDebugPrintf(&printBufPtr,
                                    origPacketPtr->pktLinkDataPtr,
                                    printLen,
                                    &offset);
                printLen -= printLen;
            }
            origPacketPtr = origPacketPtr->pktLinkNextPtr;
        }
        tfCryptoDebugFlush(&printBufPtr);
    }
    else
    {
        tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        newPacketPtr->pktLinkDataPtr + 
                            newPacketPtr->pktLinkDataLength - origPayloadLen,
                        origPayloadLen, 0);
    }
#endif /*TM_IPSEC_DEBUG || TM_SSL_DEBUG */

PACKETENCRYPT_RETURN:

#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    if(errorCode != TM_ENOERROR)
    {
        tm_debug_log0("Encrypt failed. \n");
    }
#endif /*TM_IPSEC_DEBUG || TM_SSL_DEBUG */
    return errorCode;
}

/*  input data buffer is allocated outside tfEspPacketCtrModeCrypto(),
 *  already padded, ip,spi,seq,iv,data,pad,padLen.nextheader are all ready,
 *  authenticate field is reserved.
 *  The only thing is to encrypt the payload data and store them back.
 *  is: encrypt the payload data and store back 
 */
int tfEspPacketCtrModeCrypto(tt8BitPtr           ivPtr,
                             ttPacketPtr         packetPtr,
                             ttPktLen            offData,
                             ttPacketPtr         newPacketPtr,
                             ttPktLen            payloadLen,
                             ttGenericKeyPtr     keyPtr,
                             ttEspAlgorithmPtr   algoPtr,
                             ttVoidPtr           additionalData)
{
/* for scattered data, they may not be aligned */
    tt8Bit       notAlignedData[TM_ESP_MAX_IV_LEN];
    ttCounterBlock  counterBlockClear;
    ttCounterBlock  counterBlockCode;
    tt32Bit      i;
    tt32Bit      nonce;
    tt32Bit      blockCounter;
/* In counter mode, encryption and decryption is the same process,
 * so we do not need to know if we are encryption or decrypting
 * or if the data is encrypted or clear
 *
 * For encryption, preCryptoDataPtr points to plaintext
 * for decryption, preCryptoDataPtr points to ciphertext
 */
    tt8BitPtr    preCryptoDataPtr;
/* For encryption, preCryptoDataPtr points to ciphertext
 * for decryption, preCryptoDataPtr points to plaintext
 */
    tt8BitPtr    postCryptoDataPtr;
    ttPacketPtr  tempPacketPtr;
    ttPktLen     currentPayload;
    ttPktLen     origPayloadLen;
    int          errorCode;
    tt16Bit      remainData;
    tt16Bit      copyLen;
    tt16Bit      origRemainData;
    tt8Bit       blockLen;
    tt8Bit       bytesInBlockTodo;

#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    ttCharPtr   printBufPtr;
    ttPacketPtr origPacketPtr;
    ttPktLen    printLen;
    int         offset;

    origPacketPtr = packetPtr;
    offset = 0;
#endif /* (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG)) */

    nonce = *(tt32BitPtr)additionalData;
    errorCode = TM_ENOERROR;
    preCryptoDataPtr = (tt8BitPtr)0;
    postCryptoDataPtr = (tt8BitPtr)0;
    blockLen = algoPtr->ealgBlockSize;
    copyLen = 0;
    origPayloadLen = payloadLen;

#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    tm_debug_log0("CTR mode iv = \n");
    tfCryptoDebugPrintf((ttCharPtrPtr)0, ivPtr, TM_AESCTR_IV_LENGTH_BYTES, 0);
    tm_debug_log0("CTR mode key = \n");
    tfCryptoDebugPrintf((ttCharPtrPtr)0,
                        keyPtr->keyDataPtr, (keyPtr->keyBits>>3), 0);
    tm_debug_log1("Before CTR mode crypto, offset = %d, \
                  preCryptoDataPtr data = \n", offData);
    origPayloadLen += offData;
    tm_debug_log1("Begining of CTR mode crypto packet, \
                  payloadLen(inc offData) = %d\n", origPayloadLen);
    while(origPacketPtr && origPayloadLen > 0)
    {
        if (origPayloadLen > origPacketPtr->pktLinkDataLength)
        {
            printLen = origPacketPtr->pktLinkDataLength;
        }
        else
        {
            printLen = origPayloadLen;
        }
        tm_debug_log5("packet =0x%x, linkNextPtr = 0x%x, linkDataLen = %d, " \
                "printLen = %d, offset = %d\n",
                origPacketPtr, origPacketPtr->pktLinkNextPtr, 
                origPacketPtr->pktLinkDataLength,
                printLen, offset);
        origPayloadLen -= printLen;
        origPacketPtr = origPacketPtr->pktLinkNextPtr;
    }
    origPacketPtr = packetPtr;
    origPayloadLen = payloadLen;
    printBufPtr = (ttCharPtr)0;
    while(origPacketPtr && (origPayloadLen > 0))
    {
        if (origPayloadLen > origPacketPtr->pktLinkDataLength)
        {
            printLen = origPacketPtr->pktLinkDataLength;
        }
        else
        {
            printLen = origPayloadLen;
        }
        tfCryptoDebugPrintf(&printBufPtr, origPacketPtr->pktLinkDataPtr,
                            printLen, &offset);
        origPayloadLen -= printLen;
        origPacketPtr = origPacketPtr->pktLinkNextPtr;
    }
    tfCryptoDebugFlush(&printBufPtr);
    tm_debug_log0("\nending of CTR mode crypto packet \n");
    origPacketPtr = packetPtr;
    origPayloadLen = payloadLen;
#endif /*TM_IPSEC_DEBUG || TM_SSL_DEBUG */

/* currentPayload is either 'payloadLen', if the entire payload will fit
 * in one link, or the rest of the link if it will not
 */
    if (packetPtr->pktLinkDataLength - offData > payloadLen)
    {
        currentPayload = payloadLen;
    }
    else
    {
        currentPayload = packetPtr->pktLinkDataLength - offData;
    }
/* remainData is 0 if this payload will fit in one link
 * or the partial block size on the end if it does not
 */
    if (packetPtr->pktLinkDataLength - offData > payloadLen)
    {
        remainData = (tt16Bit)0;
    }
    else
    {
        remainData = (tt16Bit)(currentPayload % blockLen);
    }
/* Make current payload a multiple of block size 
 * (or not if the whole payload will fit in one link)
 */
    currentPayload -= remainData;

/* nonce MUST be a 32-bit value */
    counterBlockClear.nonce = nonce;
/* length of IV for AES-CTR is 8 bytes, this may change for other algorithms */
    tm_memcpy(counterBlockClear.iv, ivPtr, TM_AESCTR_IV_LENGTH_BYTES);
/* The counter ALWAYS starts at 1, it is set immidiately in the for loop. */
    blockCounter = 0;

    preCryptoDataPtr = packetPtr->pktLinkDataPtr + offData;

    if(newPacketPtr)
    {
/* the newPacketPtr may already have some built-in data, for example
 * SSL record header
 */
        postCryptoDataPtr = newPacketPtr->pktLinkDataPtr +
                       newPacketPtr->pktLinkDataLength;
    }

/* first ciphered text block */
    payloadLen -= currentPayload;

/* Note that, if currentPayload could be zero if it is not at least
 * one cipher block size
 */
    while ((currentPayload != (ttPktLen)0) || (payloadLen != 0))
    {
/* This case takes care of full blocks that fit in one link or
 * the last partial block of a message that fits in one link
 */
        if(currentPayload != 0)
        {
/* encrypt */
            if (currentPayload > blockLen)
            {
                bytesInBlockTodo = blockLen;
            }
            else
            {
                bytesInBlockTodo = (tt8Bit)currentPayload;
            }
/* increase the value of the counter, store it in network byte order */
            blockCounter++;
            counterBlockClear.ctr = htonl(blockCounter);
/* update the key stream once per block */
            if(algoPtr->ealgBlockEncryptFuncPtr)
            {
                algoPtr->ealgBlockEncryptFuncPtr(
                                    algoPtr,
                                    keyPtr,
                                    (ttUser8BitPtr)&counterBlockClear,
                                    (ttUser8BitPtr)&counterBlockCode);
            }

            if(newPacketPtr == (ttPacketPtr)0)
            {
                postCryptoDataPtr = preCryptoDataPtr;
            }

            for (i = 0; i < bytesInBlockTodo; i++)
            {
                postCryptoDataPtr[i] = 
                        (tt8Bit)(((tt8BitPtr)&counterBlockCode)[i] ^
                                 preCryptoDataPtr[i]);
            }
            
            postCryptoDataPtr += bytesInBlockTodo;
            preCryptoDataPtr += bytesInBlockTodo;
            currentPayload -= bytesInBlockTodo;
        }
/* This case does full and partial (final) blocks which must be split */
        else
        {
/* get the next link */
            if(packetPtr->pktLinkNextPtr != TM_PACKET_NULL_PTR)
            {
                packetPtr = packetPtr->pktLinkNextPtr;
                tempPacketPtr = packetPtr;
                copyLen = 0;
                if(remainData != 0)
/* since notAlignedData may be used as the ivPtr, don't make copy
 * before this line
 */
                {
/* get bytes from end of previous link (less than block size) */
                    tm_memcpy(notAlignedData, preCryptoDataPtr, remainData);
                    origRemainData = remainData;
/* now set remainData to the amount in this block to copy from the next link */
/* if we have enough data to complete this block */
                    if (payloadLen > blockLen)
                    {
                        remainData = (tt16Bit)(blockLen - remainData);
                    }
                    else
                    {
                        remainData = (tt16Bit)(payloadLen - remainData);
                    }

                    while(remainData) 
                    {
/* Test to see if the amount of remaining data will fit in this link
 * This test is trivial in most cases becase block size for algs is
 * less than link size
 */
                        if (packetPtr->pktLinkDataLength >= 
                                   (tt32Bit)remainData)
                        { 
                            copyLen = remainData;
                        }
                        else
                        {
                            copyLen = (tt16Bit)
                                (packetPtr->pktLinkDataLength);
                        }
/* copy data from this link to notAlignedData, beginning where we left off
 * This will copy enough to complete the split block or finish the message
 */
                        tm_memcpy(
                            &notAlignedData[origRemainData],
                            packetPtr->pktLinkDataPtr,
                            copyLen);
/* Probably zero because copyLen is most likely remainData */
                        remainData = (tt16Bit)(remainData - copyLen);
                        if(remainData > 0 || ((ttPktLen) copyLen == 
                                  packetPtr->pktLinkDataLength))
                        {
/* We shouldn't get in here... */
                            if(packetPtr->pktLinkNextPtr != TM_PACKET_NULL_PTR)
                            {
                                packetPtr = packetPtr->pktLinkNextPtr;
                                if(remainData == 0) 
                                {
                                    copyLen = 0;
                                }
                            }
                            else if(payloadLen > (ttPktLen)blockLen)
                            {
/* if payloadLen > blockLen, we must have more packetPtr,
 * if payloadLen = blockLen, the current packetPtr should be the last one.
 */
#ifdef TM_ERROR_CHECKING
                                tfKernelWarning("tfEspPacketCtrModeCrypto", 
                                    "Payload length error");
#endif /* TM_ERROR_CHECKING */
                                errorCode = TM_EINVAL;
                                goto PACKET_CTRMODE_CRYPTO_RETURN;
                            }
                        }
                    }

/* encrypt */
/* increase the value of the counter, store it in network byte order */
                    blockCounter++;
                    counterBlockClear.ctr = htonl(blockCounter);
/* update the key stream once per block */
                    if(algoPtr->ealgBlockEncryptFuncPtr)
                    {
                        algoPtr->ealgBlockEncryptFuncPtr(
                                    algoPtr,
                                    keyPtr,
                                    (ttUser8BitPtr)&counterBlockClear,
                                    (ttUser8BitPtr)&counterBlockCode);
                    }

                    if(newPacketPtr == (ttPacketPtr)0)
                    {
                        postCryptoDataPtr = notAlignedData;
                    }
/* copyLen is how much data was copied from the new link
 * origRemainData is how much data was copied from the old link
 * it might be a full block, but it might be less
 */
                    for (i = 0; i < (tt32Bit)(copyLen + origRemainData); i++)
                    {
                         postCryptoDataPtr[i] =
                                (tt8Bit)(((tt8BitPtr)&counterBlockCode)[i] ^ 
                                         notAlignedData[i]);
                    }

                    if(newPacketPtr == (ttPacketPtr)0)
                    {
/* copy the decrypted data to first link (origRemainData in length) */
                        tm_memcpy(preCryptoDataPtr,
                                  notAlignedData,
                                  origRemainData);
/* The decrypted data in this link is blockLen-origRemainData in length
 * or less if the message does not complete a block. 
 */
                        if(payloadLen > blockLen)
                        {
                            remainData = (tt16Bit)(blockLen - origRemainData);
                        }
                        else
                        {
                            remainData =
                                    (tt16Bit)(payloadLen - origRemainData);
                        }
/* This should only run one time, since remainData is almost certainly 
 * less than the link size */
                        while(remainData)
                        {
/* copyLen will most likely equal remainData */
                            if(tempPacketPtr->pktLinkDataLength >= 
                                      (tt32Bit)remainData)
                            {
                                copyLen = remainData;
                            }
                            else
                            {
                                copyLen = (tt16Bit)
                                        (tempPacketPtr->pktLinkDataLength);
                            }
/* Copy rest of block (or message) into this link */
                            tm_memcpy(tempPacketPtr->pktLinkDataPtr,
                                      &notAlignedData[origRemainData],
                                      copyLen);
/* Should be zero */
                            remainData = (tt16Bit)(remainData - copyLen);
/* Shouldn't go in here */
                            if(remainData > 0 || ((ttPktLen) copyLen 
                                == tempPacketPtr->pktLinkDataLength))
                            {
                                tempPacketPtr = tempPacketPtr->pktLinkNextPtr;
                                copyLen = 0;
                            }
                        }
                    }
                    else
                    {
                        postCryptoDataPtr += (copyLen + origRemainData);
                    }
/* We just decrypted a split block, so subtract it from what's remaining */
                    payloadLen -= (copyLen + origRemainData);
                }
                if(payloadLen != 0)
                {
/* Move the ciphetPtr up to the next encrypted data */
                    preCryptoDataPtr = packetPtr->pktLinkDataPtr + copyLen; 
/* currentPayload gets the amount of data that will fit in this link */
                    if ((packetPtr->pktLinkDataLength - copyLen) > payloadLen)
                    {
                        currentPayload = payloadLen;
                    }
                    else
                    {
                        currentPayload =
                            (packetPtr->pktLinkDataLength - copyLen);
                    }
/* this shared data may not aligned either */
/* remainData is 0 if this payload will fit in one link
 * or the partial block size on the end if it does not
 */
                    if (packetPtr->pktLinkDataLength - copyLen > payloadLen)
                    {
                        remainData = (tt16Bit)0;
                    }
                    else
                    {
                        remainData = (tt16Bit)(currentPayload % blockLen);
                    }
/* If the message is too long still, subtract off the split block */
                    currentPayload -= remainData;
/* Subtract off the full blocks from payloadLen */
                    payloadLen -= currentPayload;
                }
            }
            else if(payloadLen != 0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfEspPacketCtrModeCrypto", 
                                "Payload length error");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_EINVAL;
                goto PACKET_CTRMODE_CRYPTO_RETURN;
            }
        }
    }

    if(newPacketPtr)
    {
        newPacketPtr->pktLinkDataLength += origPayloadLen;
    }

#if (defined(TM_IPSEC_DEBUG) || defined(TM_SSL_DEBUG))
    tm_debug_log0("\nCTR mode crypto success! packet data = \n");
    offset = 0;
    printLen = origPayloadLen + offData;
    if(newPacketPtr == (ttPacketPtr)0)
    {
        while(origPacketPtr && printLen > 0)
        {
            if (printLen > origPacketPtr->pktLinkDataLength)
            {
                tfCryptoDebugPrintf(&printBufPtr,
                                    origPacketPtr->pktLinkDataPtr,
                                    origPacketPtr->pktLinkDataLength,
                                    &offset);
                printLen -= origPacketPtr->pktLinkDataLength;
            }
            else
            {   
                tfCryptoDebugPrintf(&printBufPtr,
                                    origPacketPtr->pktLinkDataPtr,
                                    printLen,
                                    &offset);
                printLen -= printLen;
            }

            origPacketPtr = origPacketPtr->pktLinkNextPtr;
        }
        tfCryptoDebugFlush(&printBufPtr);
    }
    else
    {
        tfCryptoDebugPrintf((ttCharPtrPtr)0,
                newPacketPtr->pktLinkDataPtr +
                newPacketPtr->pktLinkDataLength - origPayloadLen,
                origPayloadLen,
                0);
    }
#endif /*TM_IPSEC_DEBUG || TM_SSL_DEBUG */

PACKET_CTRMODE_CRYPTO_RETURN:

    return errorCode;
}
