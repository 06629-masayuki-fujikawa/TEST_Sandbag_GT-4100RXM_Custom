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
 * Description: Crypto Library .h file
 *
 * Filename: trcrylib.h
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: include/trcrylib.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:18JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRCRYLIB_H_
#define _TRCRYLIB_H_


#ifdef __cplusplus
extern "C" {
#endif


/* 32Bit integer is our maximum*/
#define TM_DH_THIRTY_TWO_BIT 
/* the maximum signed integer our machine can handle */
#define TM_INTEGER_MAX                   TM_UL(0x7fffffff)
/*
 * Some 32-bit compilers may not be able to handle 64-bit longs.
 * For such compilers, define TM_NO_LONG64 ins TRSYSTEM.H,
 * thereby allowing the 16-bit macros to be used since they only
 * depend on 32-bit longs.
 */
#ifdef TM_NO_LONG64
#undef TM_DH_THIRTY_TWO_BIT
#define TM_DH_SIXTEEN_BIT
#endif /* TM_NO_LONG64 */

#define TM_AHALGORITHM_NULL_PTR          (ttAhAlgorithmPtr)0
#define TM_ESPALGORITHM_NULL_PTR         (ttEspAlgorithmPtr)0
#define TM_DH_GENERATOR_2                2
#define TM_DH_GENERATOR_5                5

#define TM_ISAKMP_RESPONDER              (tt8Bit)0
#define TM_ISAKMP_INITIATOR              (tt8Bit)1

/* Macros used to determine how to interpret encryption and hashing macros */
#define TM_CRYPTO_MODULE_IKE             (tt8Bit)1

/* 
 *  Diffie-Hellman macroes 
 */

#define TM_OAKLEY_PRIME_MODP512 \
    "FFFFFFFF FFFFFFFF C90FDAA2 2168C234" \
    "C4C6628B 80DC1CD1 29024E08 8A67CC74" \
    "CD3A431B 302B0A6D F25F1437 625E7EC6" \
    "F44C42E9 A63A3620 FFFFFFFF FFFFFFFF"

#define TM_OAKLEY_PRIME_MODP768 \
    "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1" \
    "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD" \
    "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245" \
    "E485B576 625E7EC6 F44C42E9 A63A3620 FFFFFFFF FFFFFFFF"

#define TM_OAKLEY_PRIME_MODP1024 \
    "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1" \
    "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD" \
    "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245" \
    "E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED" \
    "EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE65381" \
    "FFFFFFFF FFFFFFFF"

#define TM_OAKLEY_PRIME_MODP1536 \
    "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1" \
    "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD" \
    "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245" \
    "E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED" \
    "EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE45B3D" \
    "C2007CB8 A163BF05 98DA4836 1C55D39A 69163FA8 FD24CF5F" \
    "83655D23 DCA3AD96 1C62F356 208552BB 9ED52907 7096966D" \
    "670C354E 4ABC9804 F1746C08 CA237327 FFFFFFFF FFFFFFFF"

#define TM_OAKLEY_PRIME_MODP2048 \
    "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1" \
    "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD" \
    "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245" \
    "E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED" \
    "EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE45B3D" \
    "C2007CB8 A163BF05 98DA4836 1C55D39A 69163FA8 FD24CF5F" \
    "83655D23 DCA3AD96 1C62F356 208552BB 9ED52907 7096966D" \
    "670C354E 4ABC9804 F1746C08 CA18217C 32905E46 2E36CE3B" \
    "E39E772C 180E8603 9B2783A2 EC07A28F B5C55DF0 6F4C52C9" \
    "DE2BCBF6 95581718 3995497C EA956AE5 15D22618 98FA0510" \
    "15728E5A 8AACAA68 FFFFFFFF FFFFFFFF"

/*character vector */
typedef struct tsCharVector 
{
/* length of the value */
    unsigned int    cvLength;    
/* place holder to the value in buffer */
    char TM_FAR *   cvValuePtr;  
}ttCharVector;
typedef struct tsCharVector TM_FAR * ttCharVectorPtr;
typedef ttCharVectorPtr TM_FAR * ttCharVectorPtrPtr;


/* ttMd5Ctx, because the ppp-chap option will use md5context sturcture, 
 * so this part is defined in
 * trtype.h 
*/

#ifdef TM_USE_BLOWFISH
/*esp - blowfish */
#define TM_BLOWFISH_ROUNDS    16
typedef struct bf_key_st {
    tt32Bit TM_FAR P[TM_BLOWFISH_ROUNDS+2];
    tt32Bit TM_FAR S[4*256];
} ttBlowfishKey;
typedef ttBlowfishKey TM_FAR * ttBlowfishKeyPtr;
#endif /* TM_USE_BLOWFISH*/

#ifdef TM_USE_RC2
#define TM_RC2_KEY_SCHEDULE_SIZE    64
typedef struct tsRc2Key
{
    tt32Bit TM_FAR rc2KeyData[TM_RC2_KEY_SCHEDULE_SIZE];
}ttRc2Key;
typedef ttRc2Key TM_FAR * ttRc2KeyPtr;
#endif /* RC2 */

#ifdef TM_USE_AES
/* esp-aes */
/* # of ASCII char's needed to represent a key */
#define     TM_AES_MAX_KEY_SIZE         64 
/* # bytes needed to represent an IV  */
#define     TM_AES_MAX_IV_SIZE          16 
#define     TM_AES_MAXROUNDS            14

/*    The structure for key information */
typedef struct {
    tt32Bit TM_FAR rd_key[4 *(TM_AES_MAXROUNDS + 1)];
    int rounds;
} ttAesKeyInstance;
typedef ttAesKeyInstance TM_FAR * ttAesKeyInstancePtr;
#endif /* TM_USE_AES*/

#ifdef TM_USE_TWOFISH
/* esp-twofish */
/* # of bytes needed to represent an IV */
#define TM_TWOFISH_MAX_IV_SIZE      16    
/* number of bits per block */
#define TM_TWOFISH_BLOCK_SIZE       128 
/* # of ASCII chars needed to represent a key */
#define TM_TWOFISH_MAX_KEY_SIZE     64    
/* # of ASCII chars needed to represent a key */
#define TM_TWOFISH_MAX_KEY_BITS     256 
/* min number of bits of key (zero pad) */
#define TM_TWOFISH_MIN_KEY_BITS     128 

/* max # rounds (for allocating subkey array) */
#define TM_TWOFISH_MAX_ROUNDS       16 
#define TM_TWOFISH_INPUT_WHITEN     0   /* subkey array indices */
#define TM_TWOFISH_OUTPUT_WHITEN    \
            (TM_TWOFISH_INPUT_WHITEN + TM_TWOFISH_BLOCK_SIZE/32)
/* use 2 * (# rounds) */
#define TM_TWOFISH_ROUND_SUBKEYS    \
            (TM_TWOFISH_OUTPUT_WHITEN + TM_TWOFISH_BLOCK_SIZE/32)   

#define TM_TWOFISH_TOTAL_SUBKEYS    \
            (TM_TWOFISH_ROUND_SUBKEYS + 2*TM_TWOFISH_MAX_ROUNDS)
/* nonzero forces reentrant code (slightly slower) */
#define TM_TWOFISH_REENTRANT           1

#ifdef TM_LITTLE_ENDIAN
/* need dword alignment? (no for Pentium) */
#define TM_TWOFISH_ALIGN32             0
#else
/* need dword alignment? (no for Pentium) */
#define  TM_TWOFISH_ALIGN32            1        
#endif

/* The structure for key information */
typedef struct 
{
    tt8Bit direction;            /* Key used for encrypting or decrypting? */
#if TM_TWOFISH_ALIGN32
    tt8Bit TM_FAR dummyAlign[3]; /* keep 32-bit alignment */
#endif
    int  keyLen;                 /* Length of the key */
    tt8Bit TM_FAR keyMaterial[TM_TWOFISH_MAX_KEY_SIZE+4];/*key data in ASCII */
    
    /* Twofish-specific parameters: */
    tt32Bit keySig;              /* set to VALID_SIG by makeKey() */
    int   numRounds;             /* number of rounds in cipher */
/* actual key bits, in dwords */
    tt32Bit TM_FAR key32[TM_TWOFISH_MAX_KEY_BITS/32];    
/* key bits used for S-boxes */
    tt32Bit TM_FAR sboxKeys[TM_TWOFISH_MAX_KEY_BITS/64];
/* round subkeys, input/output whitening bits */
    tt32Bit TM_FAR subKeys[TM_TWOFISH_TOTAL_SUBKEYS];    
#if TM_TWOFISH_REENTRANT 
    tt32Bit TM_FAR sBox8x32[4][256];/*fully expanded S-box */
#if defined(COMPILE_KEY) && defined(USE_ASM)
#undef  VALID_SIG
/* 'COMP':  C is compiled with -DCOMPILE_KEY */
#define VALID_SIG    0x504D4F43     
/* set after first "compile" (zero at "init") */
    tt32Bit          cSig1;
/* ptr to asm encrypt function */
    ttVoidPtr        encryptFuncPtr; 
/* ptr to asm decrypt function */
    ttVoidPtr        decryptFuncPtr;
/* size of compiledCode */
    tt32Bit          codeSize;
/* set after first "compile" */
    tt32Bit          cSig2;
/* make room for the code itself */
    tt8Bit           TM_FAR compiledCode[5000];
#endif
#endif
} ttTwofishKeyInstance;
typedef ttTwofishKeyInstance TM_FAR * ttTwofishKeyInstancePtr;
#endif /* TM_USE_TWOFISH*/

#ifdef TM_USE_ARCFOUR
#define   TM_ARC4_STATE_SIZE    256
typedef struct tsArc4Context
{
    tt8Bit        TM_FAR arc4State[TM_ARC4_STATE_SIZE];
    tt8Bit        arc4X;
    tt8Bit        arc4Y;
} ttArc4Context;
typedef ttArc4Context TM_FAR * ttArc4ContextPtr;
#endif /* TM_USE_ARCFOUR */



#ifdef TM_USE_RC5
/* esp-rc5 */
#define TM_RC5_ROUNDS_8     8
#define TM_RC5_ROUNDS_12    12
#define TM_RC5_ROUNDS_16    16

#define TM_RC5_DEFAULT_ROUND 16

/*    The structure for key information */
typedef struct ttRc5KeyInstance {
    int      rounds; 
    tt32Bit  TM_FAR rc5KeySched[2*(TM_RC5_ROUNDS_16+1)];
} ttRc5KeyInstance;
typedef ttRc5KeyInstance TM_FAR * ttRc5KeyInstancePtr;
#endif /* TM_USE_RC5*/

typedef  int  (*ttEspPktEncryptDecryptPtr)(tt8BitPtr           ivPtr,
                                           ttPacketPtr         packetPtr, 
                                           ttPktLen            offData, 
                                           ttPacketPtr         newPacketPtr,
                                           ttPktLen            payloadLen,
                                           ttGenericKeyPtr     keyPtr,
                                           ttEspAlgorithmPtr   algoPtr,
                                           ttVoidPtr           additionalData);

TM_PROTO_EXTERN int tfCryptoEngineInit(void);
TM_PROTO_EXTERN void tfCryptoEngineUninitialize(void);



TM_PROTO_EXTERN int tfAhContext(ttAhAlgorithmPtr algoPtr, 
                                ttGenericKeyPtr gkeyPtr);
TM_PROTO_EXTERN void tfAhClearContext(ttVoidPtr  contextPtr);
TM_PROTO_EXTERN int tfAhMac(ttPacketPtr      packetPtr,
                            int              startOff, 
/*TLS when using HMAC on packets, need hash in sequence number
 * which is not in network data. 
 */
                             tt8BitPtr        extraDataPtr,
                             ttPktLen         extraLen,
                             ttPktLen         totalAuth,
                             tt8BitPtr        outPositionPtr,
                             ttGenericKeyPtr  gkeyPtr, 
                             ttAhAlgorithmPtr algoPtr,
                             tt8Bit           needTruncate,
                             tt8Bit           doHmac);

TM_PROTO_EXTERN int tfEspDataCbcEncrypt(tt8BitPtr           ivPtr,
                                        tt8BitPtr           dataSrcPtr, 
                                        tt8BitPtr           dataDstPtr,
                                        ttPktLen            payloadLen,
                                        ttGenericKeyPtr     keyPtr,
                                        ttEspAlgorithmPtr   algoPtr);

TM_PROTO_EXTERN int tfEspDataCbcDecrypt (tt8BitPtr       ivPtr, 
                                         tt8BitPtr       dataSrcPtr, 
                                         tt8BitPtr       dataDstPtr,
                                         ttPktLen        payloadLen,
                                         ttGenericKeyPtr keyPtr, 
                                         ttEspAlgorithmPtr algoPtr);

TM_PROTO_EXTERN int tfEspDataCtrEncrypt(tt32BitPtr        nonce,
                                        tt8BitPtr         ivPtr,
                                        tt32Bit           ivLen,
                                        tt8BitPtr         dataSrcPtr,
                                        tt8BitPtr         dataDstPtr,
                                        ttPktLen          payloadLen,
                                        ttGenericKeyPtr   keyPtr,
                                        ttEspAlgorithmPtr algoPtr);

TM_PROTO_EXTERN int tfEspDataCtrDecrypt(tt32BitPtr        nonce,
                                        tt8BitPtr         ivPtr,
                                        tt32Bit           ivLen,
                                        tt8BitPtr         dataSrcPtr,
                                        tt8BitPtr         dataDstPtr,
                                        ttPktLen          payloadLen,
                                        ttGenericKeyPtr   keyPtr,
                                        ttEspAlgorithmPtr algoPtr);

TM_PROTO_EXTERN int tfEspPacketCbcEncrypt(tt8BitPtr           ivPtr,
                                          ttPacketPtr         packetPtr, 
                                          ttPktLen            offData,
/* if newPacketPtr is NULL, we write back to the original packet */
                                          ttPacketPtr         newPacketPtr,
                                          ttPktLen            payloadLen,
                                          ttGenericKeyPtr     keyPtr,
                                          ttEspAlgorithmPtr   algoPtr,
/* update IV */
                                          ttVoidPtr           additionalData);
TM_PROTO_EXTERN int tfEspPacketCtrModeCrypto(tt8BitPtr           ivPtr,
                                             ttPacketPtr         packetPtr, 
                                             ttPktLen            offData, 
                                             ttPacketPtr         newPacketPtr,
                                             ttPktLen            payloadLen,
                                             ttGenericKeyPtr     keyPtr,
                                             ttEspAlgorithmPtr   algoPtr,
                                             ttVoidPtr           additionalData);



TM_PROTO_EXTERN int tfEspPacketCbcDecrypt(tt8BitPtr           ivPtr,
                                          ttPacketPtr         packetPtr, 
                                          ttPktLen            offData, 
                                          ttPacketPtr         newPacketPtr,
                                          ttPktLen            payloadLen,
                                          ttGenericKeyPtr     keyPtr,
                                          ttEspAlgorithmPtr   algoPtr,
                                          ttVoidPtr           additionalData);


TM_PROTO_EXTERN int tfEspSchedule(ttEspAlgorithmPtr, 
                                  ttGenericKeyPtr);
TM_PROTO_EXTERN void tfEspClearSchedule(ttUserVoidPtr     schedulePtr);



#ifdef TM_PUBKEY_USE_DIFFIEHELLMAN
void tfCvFree(ttCharVectorPtr vptr);

void tfCvMalloc(ttCharVectorPtrPtr    cvPtrPtr, 
                tt32Bit               length);

int tfDhGenerate(ttCharVectorPtr              prime,
                 tt32Bit                      g, 
                 tt32Bit                      privLength, 
                 ttCharVectorPtrPtr           pub, 
                 ttCharVectorPtrPtr           priv);

int tfDhComputeSharedSecret(ttCharVectorPtr      prime, 
                            ttCharVectorPtr      priv, 
                            ttCharVectorPtr      pub2, 
                            ttCharVectorPtr      keyPtr);


int tfCryptoDhGenerate(int           dhgroup,
                       tt32Bit       generator,
                       tt16Bit       mySecretLen,
                       tt8BitPtr     primePtr,
                       tt16Bit       primeLength,
                       tt8BitPtrPtr  mySecretPtrPtr,
                       tt8BitPtrPtr  publicPtrPtr,
                       tt16BitPtr    publicLenPtr);

int tfCryptoDhGetSharedSecret(int           dhgroup,
                              tt8BitPtr     mySecretPtr,
                              tt16Bit       mySecretLen,
                              tt8BitPtr     primePtr,
                              tt16Bit       primeLength,
                              tt8BitPtr     yourPublicPtr,
                              tt16Bit       yourPublicLen,
                              tt8BitPtrPtr  ourSecretPtrPtr,
                              tt16BitPtr    ourSecretLenPtr);
#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN */


#ifdef TM_USE_RC2
TM_PROTO_EXTERN int tfEspRc2Schedule (ttEspAlgorithmPtr algPtr,
                                      ttGenericKeyPtr   keyPtr);
TM_PROTO_EXTERN int tfEspRc2SchedLen (void);
TM_PROTO_EXTERN int tfEspRc2BlockDecrypt (ttEspAlgorithmPtr algPtr,
                                          ttGenericKeyPtr   keyPtr, 
                                          ttUser8BitPtr     srcPtr, 
                                          ttUser8BitPtr     dstPtr);
TM_PROTO_EXTERN int tfEspRc2BlockEncrypt (ttEspAlgorithmPtr algPtr,
                                          ttGenericKeyPtr   keyPtr, 
                                          ttUser8BitPtr     srcPtr, 
                                          ttUser8BitPtr     dstPtr);
#endif /* TM_USE_RC2 */

#ifdef TM_USE_DES
TM_PROTO_EXTERN void tfDesSetParity(ttUser8BitPtr keyPtr);
TM_PROTO_EXTERN int tfDesSetKey (ttUser8BitPtr  key, 
                                 ttUser32BitPtr schedule);

TM_PROTO_EXTERN void tfDesEcbEncrypt (ttUser8BitPtr  inputblock, 
                                      ttUser8BitPtr  outputblock,
                                      ttUser32BitPtr schedule, 
                                      int            direction);
TM_PROTO_EXTERN int tfEspDesSchedule (ttEspAlgorithmPtr algPtr,
                                      ttGenericKeyPtr   keyPtr);
TM_PROTO_EXTERN int tfEspDesSchedLen (void);
TM_PROTO_EXTERN int tfEspDesBlockDecrypt (ttEspAlgorithmPtr algPtr,
                                          ttGenericKeyPtr   keyPtr, 
                                          ttUser8BitPtr     srcPtr, 
                                          ttUser8BitPtr     dstPtr);
TM_PROTO_EXTERN int tfEspDesBlockEncrypt (ttEspAlgorithmPtr algPtr,
                                          ttGenericKeyPtr   keyPtr, 
                                          ttUser8BitPtr     srcPtr, 
                                          ttUser8BitPtr     dstPtr);
#endif /* TM_USE_DES */

#ifdef TM_USE_3DES 
TM_PROTO_EXTERN int tfEsp3DesSchedLen (void);
TM_PROTO_EXTERN int tfEsp3DesSchedule (ttEspAlgorithmPtr   algPtr,
                                       ttGenericKeyPtr     keyPtr);
TM_PROTO_EXTERN int tfEsp3DesBlockDecrypt (ttEspAlgorithmPtr algPtr,
                                          ttGenericKeyPtr   keyPtr, 
                                          ttUser8BitPtr     srcPtr, 
                                          ttUser8BitPtr     dstPtr);
TM_PROTO_EXTERN int tfEsp3DesBlockEncrypt (ttEspAlgorithmPtr algPtr,
                                          ttGenericKeyPtr   keyPtr, 
                                          ttUser8BitPtr     srcPtr, 
                                          ttUser8BitPtr     dstPtr);
#endif /* TM_USE_3DES */

#ifdef TM_USE_BLOWFISH
TM_PROTO_EXTERN void tfBlowfishSetKey (ttBlowfishKeyPtr  blwKeyPtr, 
                                       tt32Bit               keyLen, 
                                       tt8BitPtr         data);
TM_PROTO_EXTERN void tfBlowfishEncrypt (tt32BitPtr       dataPtr, 
                                        ttBlowfishKeyPtr keyPtr, 
                                        int              length);
TM_PROTO_EXTERN int tfEspBlowfishSchedule (ttEspAlgorithmPtr  algPtr,
                                           ttGenericKeyPtr    keyPtr );
TM_PROTO_EXTERN int tfEspBlowfishSchedLen (void);
TM_PROTO_EXTERN int tfEspBlowfishBlockDecrypt(ttEspAlgorithmPtr  algPtr,
                                              ttGenericKeyPtr    keyPtr, 
                                              tt8BitPtr          srcPtr, 
                                              tt8BitPtr          dstPtr);
TM_PROTO_EXTERN int tfEspBlowfishBlockEncrypt (ttEspAlgorithmPtr algPtr,
                                               ttGenericKeyPtr   keyPtr, 
                                               tt8BitPtr         srcPtr, 
                                               tt8BitPtr         dstPtr);
#endif /* TM_USE_BLOWFISH*/

#ifdef TM_USE_CAST128
TM_PROTO_EXTERN  void tfSetCast128Subkey (tt32BitPtr subKeyPtr, 
                                          tt8BitPtr  keyPtr);
TM_PROTO_EXTERN  void tfCast128EncryptRound16 (tt8BitPtr       c, 
                                               ttConst8BitPtr  m,
                                               tt32BitPtr      subkey);
TM_PROTO_EXTERN  void tfCast128DecryptRound16 (tt8BitPtr       m, 
                                               ttConst8BitPtr  c,
                                               tt32BitPtr      subkey);
TM_PROTO_EXTERN  void tfCast128EncryptRound12 (tt8BitPtr       c, 
                                               ttConst8BitPtr  m,
                                               tt32BitPtr      subkey);
TM_PROTO_EXTERN  void tfCast128DecryptRound12 (tt8BitPtr       m, 
                                               ttConst8BitPtr  c,
                                               tt32BitPtr      subkey);
TM_PROTO_EXTERN int tfEspCast128Schedule (ttEspAlgorithmPtr    algPtr,
                                          ttGenericKeyPtr      keyPtr);
TM_PROTO_EXTERN int tfEspCast128SchedLen (void);
TM_PROTO_EXTERN int tfEspCast128BlockDecrypt(ttEspAlgorithmPtr algPtr ,
                                             ttGenericKeyPtr   keyPtr, 
                                             tt8BitPtr         srcPtr,  
                                             tt8BitPtr         dstPtr);
TM_PROTO_EXTERN int tfEspCast128BlockEncrypt(ttEspAlgorithmPtr algPtr ,
                                             ttGenericKeyPtr   keyPtr, 
                                             tt8BitPtr         srcPtr,  
                                             tt8BitPtr         dstPtr);
#endif /* TM_USE_CAST128*/

#ifdef TM_USE_AES

TM_PROTO_EXTERN int tfAesMakeKey(ttAesKeyInstancePtr key, 
                                      tt8Bit         direction, 
                                      int            keyLen, 
                                      tt8BitPtr      keyMaterial);
TM_PROTO_EXTERN int tfAesBlockEncrypt(ttAesKeyInstancePtr key,
                                      tt8BitPtr           input, 
                                      tt8BitPtr           outBuffer);
TM_PROTO_EXTERN  int tfAesBlockDecrypt(ttAesKeyInstancePtr  key,
                                       tt8BitPtr            input, 
                                       tt8BitPtr            outBuffer);
TM_PROTO_EXTERN int tfEspAesSchedLen (void);
TM_PROTO_EXTERN int tfEspAesSchedule (ttEspAlgorithmPtr   algPtr,
                                      ttGenericKeyPtr     keyPtr);
TM_PROTO_EXTERN int tfEspAesBlockDecrypt(ttEspAlgorithmPtr algPtr ,
                                         ttGenericKeyPtr   keyPtr, 
                                         tt8BitPtr         srcPtr, 
                                         tt8BitPtr         dstPtr);
TM_PROTO_EXTERN int tfEspAesBlockEncrypt(ttEspAlgorithmPtr algPtr ,
                                         ttGenericKeyPtr   keyPtr, 
                                         tt8BitPtr         srcPtr, 
                                         tt8BitPtr         dstPtr);
#endif /* TM_USE_AES*/

#ifdef TM_USE_TWOFISH
TM_PROTO_EXTERN int tfTwofishMakeKey (ttTwofishKeyInstancePtr  keyPtr, 
                                      tt8Bit                   direction, 
                                      int                      keyLength, 
                                      tt8BitPtr                keyMaterial);
TM_PROTO_EXTERN int tfTwofishBlockEncrypt(ttTwofishKeyInstancePtr keyPtr, 
                                          ttConst8BitPtr          input,
                                          int                     inputLen, 
                                          tt8BitPtr               outPtr);
TM_PROTO_EXTERN int tfTwofishBlockDecrypt(ttTwofishKeyInstancePtr keyPtr, 
                                          ttConst8BitPtr          input,
                                          int                     inputLen, 
                                          tt8BitPtr               outPtr);
TM_PROTO_EXTERN int tfEspTwofishSchedLen (void);
TM_PROTO_EXTERN int tfEspTwofishSchedule(ttEspAlgorithmPtr   algPtr,
                                         ttGenericKeyPtr     keyPtr);
TM_PROTO_EXTERN int tfEspTwofishBlockDecrypt(ttEspAlgorithmPtr algPtr ,
                                             ttGenericKeyPtr   keyPtr, 
                                             tt8BitPtr         srcPtr, 
                                             tt8BitPtr         dstPtr);
TM_PROTO_EXTERN int tfEspTwofishBlockEncrypt(ttEspAlgorithmPtr algPtr ,
                                             ttGenericKeyPtr   keyPtr, 
                                             tt8BitPtr         srcPtr, 
                                             tt8BitPtr         dstPtr);
#endif /* TM_USE_TWOFISH*/

#ifdef TM_USE_ARCFOUR
TM_PROTO_EXTERN void tfArc4Encrypt(ttArc4ContextPtr ctxPtr, 
                                   tt8BitPtr        destPtr, 
                                   ttConst8BitPtr   srcPtr, 
                                   ttPktLen         len);
TM_PROTO_EXTERN int tfEspArc4SchedLen(void);
TM_PROTO_EXTERN int tfEspArc4Schedule(ttEspAlgorithmPtr algoPtr,
                                      ttGenericKeyPtr   gkeyPtr);
#endif /* TM_USE_ARCFOUR */

/*
 * Put data in hex format, flush to log buffer when full,
 * and at the end if offSetPtr is null.
 */
TM_PROTO_EXTERN void tfCryptoDebugPrintf(ttUserCharPtrPtr  printBufPtrPtr,
                                         ttUser8BitPtr  dataPtr,
                                         ttUser32Bit    dataLength,
                                         ttUserIntPtr   offsetPtr);

/*
 * Flush to log buffer. Only needed to be called if last call to
 * tfCryptoDebugPrintf had a non null offsetPtr, and we are done.
 */
TM_PROTO_EXTERN void tfCryptoDebugFlush(ttUserCharPtrPtr printBufPtrPtr);

#ifdef __cplusplus
}
#endif


#endif /* _TRCRYLIB_H_ */

