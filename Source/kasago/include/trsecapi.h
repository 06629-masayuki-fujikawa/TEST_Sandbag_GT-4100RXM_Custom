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
 * Description:
 *  The primary header file that the user needs to include in their
 *  application to be able to use IPsec/IKE.
 *
 * Filename: trsecapi.h
 * Author: Jin Zhang
 * Date Created: 03/03/02
 * $Source: include/trsecapi.h $
 *
 * Modification History
 * $Revision: 6.0.2.9 $
 * $Date: 2015/12/04 11:43:21JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRSECAPI_H_
#define _TRSECAPI_H_


#ifdef __cplusplus
extern "C" {
#endif

#ifdef TM_USE_IKE_DPD
#ifndef TM_USE_IKE_VENDOR_ATTR
#define TM_USE_IKE_VENDOR_ATTR
#endif /* TM_USE_IKE_VENDOR_ATTR */
/* Dead Peer Detection Modes set in IKE policy */
#define TM_IKE_DPD_ON_DEMAND        0
#define TM_IKE_DPD_PERIODIC         1
#endif /* TM_USE_IKE_DPD */

#ifdef TM_USE_NATT
#ifndef TM_USE_IKE_VENDOR_ATTR
#define TM_USE_IKE_VENDOR_ATTR
#endif /* TM_USE_IKE_VENDOR_ATTR */
#endif /* TM_USE_NATT */

#if (defined(TM_USE_SHA384) || defined(TM_USE_SHA512))
#ifndef TM_AALG_VAR_BLOCK_SIZE
#error TM_AALG_VAR_BLOCK_SIZE must be defined when using SHA-384 or SHA 512. \
       See the crypto manual for details.
#endif /* !TM_AALG_VAR_BLOCK_SIZE */
#endif /* (defined TM_USE_SHA384 || defined TM_USE_SHA512) */

/* IKE port */
#define TM_IKE_PORT                        500
#ifdef TM_USE_NATT
/* IKE port to be used (instead of 500) when NAT is detected */
#define TM_IKE_NAT_PORT                   4500
#define TM_IKE_NAT_PORT_ENDIAN            tm_const_htons(4500)
#endif /* TM_USE_NATT */


#define TM_CRYPTO_ENGINE_SOFTWARE            0
#define TM_CRYPTO_ENGINE_HIFN7951            1
#define TM_CRYPTO_ENGINE_MCF5235             2
#define TM_CRYPTO_ENGINE_MAX                 3

/* 
 * IPsec and IKE macros
 */

/*
 * This is the maximum number of IPsec SA pairs that can be present at one
 * time.  This value can be overridden in trsystem.h and must be a
 * tt16Bit value.
 */
#ifndef TM_IPSEC_SAD_MAX_SIZE
#define TM_IPSEC_SAD_MAX_SIZE 100
#endif /* TM_IPSEC_SAD_MAX_SIZE */

/*crypto engine request type */
#define TM_CEREQUEST_CRYPTO_HASH              1
#define TM_CEREQUEST_CRYPTO_HMACHASH          2
#define TM_CEREQUEST_CRYPTO_IPSEC             3
#define TM_CEREQUEST_CRYPTO_IKEV2             4
#define TM_CEREQUEST_CRYPTO_IKE               5
#define TM_CEREQUEST_PUBKEY_DHKEYPAIR         6
#define TM_CEREQUEST_PUBKEY_DHSHAREDKEY       7
/* For the following request, RSA/DSA keys needs to be stored
 * as asn.1 structure. Structure to be used is ttGeneralRsaPtr
 * and ttGeneralDsaPtr;
 */
#define TM_CEREQUEST_PUBKEY_RSASIGN           8
#define TM_CEREQUEST_PUBKEY_RSAVERIFY         9
#define TM_CEREQUEST_PUBKEY_DSASIGN          10
#define TM_CEREQUEST_PUBKEY_DSAVERIFY        11
#define TM_CEREQUEST_PUBKEY_RSAENCRYPT       12
#define TM_CEREQUEST_PUBKEY_RSADECRYPT       13
/* For SSL crypto request */
#define TM_CEREQUEST_CRYPTO_SSL              14
#define TM_CEREQUEST_CRYPTO_CMACHASH         15

/*compression engine request type */
#define TM_CEREQUEST_COMP_IPSEC               1

/* OAKLEY groups */
#define TM_DHGROUP_1                   1
#define TM_DHGROUP_2                   2
#define TM_DHGROUP_3                   3       /* not supported */
#define TM_DHGROUP_4                   4       /* not supported */
#define TM_DHGROUP_5                   5
#define TM_DHGROUP_14                  14
/* assign an arbitrary number to our exportable DH group. Got to be
 * 512 bits or less
 */
#define TM_DHGROUP_EXPORT              111
/* class value - encryption algorithm, during phase 1 */
/* The IKE_null encrypt algorithm is not interoperable with
 * other vendors. It only works as a testing tool, so that 
 * we don't use any encryption for IKE exchanges
 */
#define TM_IKE_DES_CBC          (ttUser16Bit)1
#define TM_IKE_IDEA_CBC         (ttUser16Bit)2       /* not supported */
#define TM_IKE_BLOWFISH_CBC     (ttUser16Bit)3
#define TM_IKE_RC5_R16_B64_CBC  (ttUser16Bit)4       /* not supported */
#define TM_IKE_3DES_CBC         (ttUser16Bit)5
#define TM_IKE_CAST_CBC         (ttUser16Bit)6
/* determined by IANA in RFC 3602*/
#define TM_IKE_AES_CBC          (ttUser16Bit)7
/* to be determined by IANA - class values 65001-65535 are for private use
   among mutually consenting parties */
#define TM_IKE_NULL_ENCRYPT     (ttUser16Bit)65001
#define TM_IKE_TWOFISH_CBC      (ttUser16Bit)65005
/* class value - Hash Algorithm attribute , during phase 1*/
#define TM_IKE_MD5              (ttUser16Bit)1
#define TM_IKE_SHA1             (ttUser16Bit)2
#define TM_IKE_TIGER            (ttUser16Bit)3       /* not supported */
#define TM_IKE_SHA256           (ttUser16Bit)4
#define TM_IKE_SHA384           (ttUser16Bit)5
#define TM_IKE_SHA512           (ttUser16Bit)6
/* to be determined by IANA - class values 65001-65535 are for private use
   among mutually consenting parties */
#define TM_IKE_RIPEMD           (ttUser16Bit)65001

/* class value - Authentication Method attribute, phase 1 */
#define TM_IKE_PRESHARED_KEY    (ttUser8Bit)1
#define TM_IKE_DSS_SIG          (ttUser8Bit)2
#define TM_IKE_RSA_SIG          (ttUser8Bit)3
#ifdef TM_USE_IKEV1
#define TM_IKE_RSA_ENC          (ttUser8Bit)4
#define TM_IKE_RSA_REVISED      (ttUser8Bit)5
#endif /* TM_USE_IKEV1 */

#ifdef TM_USE_IKEV2
#define TM_IKE_EAP_MSCHAPV2_CLIENT (ttUser8Bit)10
#define TM_IKE_EAP_MSCHAPV2_SERVER (ttUser8Bit)11
#define TM_IKE_EAP_SIM_CLIENT      (ttUser8Bit)12
#define TM_IKE_EAP_SIM_SERVER      (ttUser8Bit)13
#define TM_IKE_EAP_MD5_CLIENT      (ttUser8Bit)14
#define TM_IKE_EAP_MD5_SERVER      (ttUser8Bit)15
/* reserved value */
#define TM_IKE_EAP_AUTH            (ttUser8Bit)128
#if defined(TM_IKEV2_REMOTE_CLIENT) || defined(TM_IKEV2_REMOTE_SERVER)
#define TM_IKEV2_VIRTUAL_HOME_INDEX (TM_MAX_IPS_PER_IF - 1)
#endif /* defined(TM_IKEV2_REMOTE_CLIENT) || defined(TM_IKEV2_REMOTE_SERVER) */
#endif /* TM_USE_IKEV2 */

#define TM_IPSEC_CLEAR_MANUAL_SAS      0x0001

/* SSL uses TCP port 443 by default */
#define TM_SSL_PORT                    443

/*
 * Cryptography (hashing, encryption...)
 */
#define TM_ESP_ENCRYPT                       1
#define TM_ESP_DECRYPT                       0

/* authentication header algorithms
 * If adding a new algorithm, make sure the AH payload
 * length is represented below */
#define    SADB_AALG_NULL              0  /* not supported */
#define    SADB_AALG_MD5HMAC           1 
#define    SADB_AALG_SHA1HMAC          2 
#define    SADB_AALG_RIPEMDHMAC        3  
#define    SADB_AALG_MD2HMAC           4  /* not supported */
#define    SADB_AALG_SHA256HMAC        5
#define    SADB_AALG_SHA384HMAC        6
#define    SADB_AALG_SHA512HMAC        7
#define    SADB_AALG_AESXCBCMAC        8

/* Valid lengths of AH payload field in authentication header.
 * When adding a new valid length, modify tfAhCheckPkt as well */
/* Length only valid for NULL encryption */
#define SADB_AALG_AH_PAYLOAD_LEN_1 ((tt8Bit)1)
/* Length for MD5 and SHA1 */
#define SADB_AALG_AH_PAYLOAD_LEN_4 ((tt8Bit)4)
/* Length of SHA-256 payload */
#define SADB_AALG_AH_PAYLOAD_LEN_5 ((tt8Bit)5)
/* Length of SHA-384 payload */
#define SADB_AALG_AH_PAYLOAD_LEN_7 ((tt8Bit)7)
/* Length of SHA-512 payload */
#define SADB_AALG_AH_PAYLOAD_LEN_9 ((tt8Bit)9)

/* ESP algorithms */
#define    SADB_EALG_NULL              0
#define    SADB_EALG_RC2CBC            6  /* not supported */
#define    SADB_EALG_DESIV64CBC        7  /* not supported */
#define    SADB_EALG_DESCBC            8 
#define    SADB_EALG_3DESCBC           9 
#define    SADB_EALG_ARCFOUR           10  /* not supported */
#define    SADB_EALG_RC5CBC            11  /* not supported */
#define    SADB_EALG_IDEACBC           12  /* not supported */
#define    SADB_EALG_CAST128CBC        13  
#define    SADB_EALG_BLOWFISHCBC       14  
#define    SADB_EALG_3IDEACBC          15  /* not supported */
#define    SADB_EALG_AESCBC            16
#define    SADB_EALG_TWOFISHCBC        17  
#define    SADB_EALG_AESCTR            33
#if (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST))
#define    SADB_EALG_AESCCM8           34  /* not supported */
#define    SADB_EALG_AESCCM12          35  /* not supported */
#define    SADB_EALG_AESCCM16          36  /* not supported */
#endif /* (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST)) */

#define    TM_AESCTR_NONCE_LENGTH_BYTES 4
#define    TM_AESCTR_IV_LENGTH_BYTES    8

/* IPCOMP algorithms */
#define    SADB_CALG_NULL              0  /* not supported */
#define    SADB_CALG_LZS               1  /* not supported */
#define    SADB_CALG_OUI               2  /* not supported */
#define    SADB_CALG_DEFLATE           3
#define    SADB_CALG_LZJH              4

/* Public Key */
#define    SADB_PUBKEY_NULL            19
#define    SADB_PUBKEY_RSA             20
#define    SADB_PUBKEY_DSA             21
/* support Diffie-Hellman group 1 */
#define    SADB_PUBKEY_DIFFIEHELLMAN1  22
/* support Diffie-Hellman group 2 */
#define    SADB_PUBKEY_DIFFIEHELLMAN2  23
/* support Diffie-Hellman group 5 */
#define    SADB_PUBKEY_DIFFIEHELLMAN5  24
/* support Diffie-Hellman group 14 */
#define    SADB_PUBKEY_DIFFIEHELLMAN14 26
#define    SADB_PUBKEY_X509            25

#if (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST))
#define    SADB_ALGORITHM_MAX          36
#else /* (!defined(TM_USE_VIPER) && !defined(TM_USE_VIPER_TEST)) */
#define    SADB_ALGORITHM_MAX          31
#endif /* (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST)) */

typedef struct tsGenericKey {
    ttUser8BitPtr        keyDataPtr;
    int                  keyRounds;
    ttUser32Bit          keyBits;
    union{
        void TM_FAR *    keySchedulePointer;
        void TM_FAR *    keyContextPointer;
    }schd_context_ptr;
    union{
        ttUser16Bit      keyScheduleLength;
        ttUser16Bit      keyContextLength;
    }schd_context_len;
    
#define keySchedulePtr schd_context_ptr.keySchedulePointer    
#define keyContextPtr schd_context_ptr.keyContextPointer  
#define keyScheduleLen schd_context_len.keyScheduleLength  
#define keyContextLen schd_context_len.keyContextLength  
}ttGenericKey;
typedef ttGenericKey TM_FAR * ttGenericKeyPtr;

typedef struct tsCounterBlock {
    ttUser32Bit  nonce;
    ttUser8Bit   iv[8];
/* always store in network byte order */
    ttUser32Bit  ctr;
}ttCounterBlock;
typedef ttCounterBlock TM_FAR * ttCounterBlockPtr;

typedef struct tsAhAlgorithm {
    ttUser16Bit     aalgName;
    ttUser16Bit     aalgKeyMin;    /* in bits */
    ttUser16Bit     aalgKeyMax;    /* in bits */
    ttUser16Bit     aalgKeyDefault;
    ttUser16Bit     aalgDigestOutBits; /*in bits*/
    ttUser16Bit     aalgDigestTruncateBits; /*in bits*/
#ifdef TM_AALG_VAR_BLOCK_SIZE
    ttUser16Bit     aalgBlockSize; /* in bytes */
#endif /* TM_AALG_VAR_BLOCK_SIZE */
    void          (TM_CODE_FAR *aalgInitFuncPtr) (ttUserVoidPtr contextPtr);
    void          (TM_CODE_FAR *aalgUpdateFuncPtr) (
                                        ttUserVoidPtr  contextPtr, 
                                        ttUser8BitPtr  moreDataPtr, 
                                        ttPktLen       size, 
                                        ttPktLen       offset);
    void          (TM_CODE_FAR *aalgFinalFuncPtr)(
                                      ttUser8BitPtr outDigest, 
                                      ttUserVoidPtr contextPtr);
    int           (TM_CODE_FAR *aalgContextLenFuncPtr) (void);
}ttAhAlgorithm;

typedef ttAhAlgorithm TM_FAR * ttAhAlgorithmPtr;

typedef struct  tsEspAlgorithm {
    int             ealgName;
    ttUser8Bit      ealgBlockSize;  /* block size, in byte */
    ttUser8Bit      ealgPaddingFlag; 
    ttUser16Bit     ealgKeyMin;    /* in bits */
    ttUser16Bit     ealgKeyMax;    /* in bits */
    ttUser16Bit     ealgKeyDefault; /* use for IKE */
    int           (TM_CODE_FAR *ealgScheduleLenFuncPtr) (void);
    int           (TM_CODE_FAR *ealgScheduleFuncPtr) (
                                  struct tsEspAlgorithm TM_FAR * algPtr, 
                                  ttGenericKeyPtr                keyPtr);
    int           (TM_CODE_FAR *ealgBlockDecryptFuncPtr) (
                                  struct tsEspAlgorithm TM_FAR * algPtr,
                                  ttGenericKeyPtr                keyPtr, 
                                  ttUser8BitPtr                  srcPtr, 
                                  ttUser8BitPtr                  dstPtr);
    int           (TM_CODE_FAR *ealgBlockEncryptFuncPtr) (
                                 struct tsEspAlgorithm TM_FAR * algPtr, 
                                  ttGenericKeyPtr               keyPtr, 
                                  ttUser8BitPtr                 srcPtr, 
                                  ttUser8BitPtr                 dstPtr);
}ttEspAlgorithm;
typedef ttEspAlgorithm TM_FAR * ttEspAlgorithmPtr;


/* direction */
#define TM_IPSEC_INBOUND          (ttUser8Bit)0x01 
#define TM_IPSEC_OUTBOUND         (ttUser8Bit)0x02
#define TM_IPSEC_BOTH_DIRECTION   (ttUser8Bit)0x03 /* = 0x01 | 0x02 */

#define TM_CRYPTOREQUEST_ENCODE              0x01
#define TM_CRYPTOREQUEST_DECODE              0x02
#define TM_CRYPTOREQUEST_AH                  0x04
/* We don't have packetPtr to hold the data yet. For example, we are sending
 * the FINISHED handshake message, or NO_WAIT user data. In this case, we 
 * need do SSL encapsulation immediately on the raw data. Otherwise, we 
 * have a packetPtr (or zero copy buffer handle) to hold the data
 */
#define TM_CRYPTOREQUEST_DONE                0x80


/*IPsec options user can change in running time */
#define TM_IPSECOPT_ANTIREPLAY               1
#define TM_IPSECOPT_PFSKEY                   2
#define TM_IPSECOPT_AGGRESS                  3
#define TM_IPSECOPT_AGGRESSDHGROUP           4
#define TM_IPSECOPT_INITCONTACT              5
#define TM_IPSECOPT_ICMPBYPASS               6
#define TM_IPSECOPT_ICMPSRCCHKBYPASS         7
#define TM_IPSECOPT_ICMP6NDMLDBYPASS         8
#define TM_IPSECOPT_NESTIKE_BYPASS           9
/* set the dfbit options */
#define TM_IPSECOPT_DFBIT                    10
#define TM_IPSECOPT_DEAD_PEER_DETECTION      11
#define TM_IPSECOPT_COMMITBIT                12
/* IKEv2 options */
#define TM_IPSECOPT_IKEV2_COOKIE             13
#define TM_IPSECOPT_IKEV2_NOREKEY_CHILD_SA   14
#define TM_IPSECOPT_IKEV2_NOREKEY_IKE_SA     15

/* PKI options */
#define TM_PKIOPT_PKICERTCHECKALIVE          1
#define TM_PKIOPT_PKICERTNONVERIFY           2
/* Maximum allowed key size (in bits) */
#define TM_PKIOPT_PKIMAXKEYSIZE              3

/* IPsec policy contetnt flag */
#define TM_IPSEC_SFLAG_CHK_FALSE             0
#define TM_IPSEC_SFLAG_CHK_TRUE              1

#define TM_IPSEC_BDSCP_COPY                  1

/*how to treat DF bit with regarding to outer header */
#define TM_IPSEC_DFBIT_CLEAR                 0
#define TM_IPSEC_DFBIT_SET                   1
#define TM_IPSEC_DFBIT_COPY                  2

#ifdef TM_USE_IPSEC_IPCOMP
/* compression type */
#define TM_COMPREQUEST_COMP                  0x01
#define TM_COMPREQUEST_DECOMP                0x02

typedef int (TM_CODE_FAR *ttCompEngineInitFuncPtr)(
    ttUserVoidPtr           initParamPtr);
typedef int (TM_CODE_FAR *ttCompSessionOpenFuncPtr)(
    ttUserVoidPtr TM_FAR * sessionPtrPtr,
    ttUserVoidPtr          param1Ptr, 
    ttUserVoidPtr          param2Ptr);
typedef int (TM_CODE_FAR *ttCompSessionFuncPtr)(
    ttUserVoidPtr              sessionPtr);

typedef struct tsCompEngine
{
    ttUser32Bit                   compSupportFlags;
    ttCompEngineInitFuncPtr       compInitFuncPtr;
    ttCompSessionOpenFuncPtr      compSessionOpenFuncPtr;
    ttCompSessionFuncPtr          compSessionProcessFuncPtr;
    ttCompSessionFuncPtr          compSessionCloseFuncPtr;
}ttCompEngine;
typedef ttCompEngine TM_FAR * ttCompEnginePtr;
typedef ttCompEnginePtr TM_FAR * ttCompEnginePtrPtr;

typedef struct tsCompEngineList
{
    struct tsCompEngineList TM_FAR * compNextPtr;
    ttCompEnginePtr                  compEnginePtr;
}ttCompEngineList;
typedef ttCompEngineList TM_FAR * ttCompEngineListPtr;
#endif /* TM_USE_IPSEC_IPCOMP */

/* 
 * Policy Content Flags
 */

/* policy content flags masks, from the lowest bit: Bit 0 
 * bit 0: set means tunnel mode
 * bit 1: set means transport mode, you can't set both tunnel and transport
 * bit 2  set means AH
 * bit 3  set means ESP. For one policy content, you can't set both AH and ESP
 * Bit 4: set means each remote ip needs its own SA. Prohibit sharing.
           see RFC 2401, 4.4.1 
 * bit 5: set means each remote port needs its own SA. Prohibit sharing.
 * Bit 6: set means each local ip needs its own SA. Prohibit sharing.
 * bit 7: set means each local port needs its own SA. seldom used.
 * Bit 8: set means each protocol needs its own SA, prohibit sharing
 * Bit 9: set means bypass ipsec policy check
 * Bit 10: set means discard the matched packet
 * Bit 11: set means to log this packet. 
 *         (selector value and 40 bytes from the ulp.
 * Bit 12: This policy is used for the tunnelling interface between
 *         MN's Co@ its HA.
 * Bit 13: set means IP Compression (only negotiation is supported)
 * Bit 14: set means 64-bit sequence number support is enabled
 * Bit 15: set means send TFC padding not support
 */
#define     TM_PFLAG_TUNNEL           0x00000001 /* bit 0 */
#define     TM_PFLAG_TRANSPORT        0x00000002 /* bit 1 */
#define     TM_PFLAG_AH               0x00000004 /* bit 2 */
#define     TM_PFLAG_ESP              0x00000008 /* bit 3 */
#define     TM_PFLAG_RIP_PACKET       0x00000010 /* bit 4 */
#define     TM_PFLAG_RPT_PACKET       0x00000020 /* bit 5 */
#define     TM_PFLAG_LIP_PACKET       0x00000040 /* bit 6 */
#define     TM_PFLAG_LPT_PACKET       0x00000080 /* bit 7 */
#define     TM_PFLAG_PROTO_PACKET     0x00000100 /* bit 8 */
#define     TM_PFLAG_BYPASS           0x00000200 /* bit 9 */ 
#define     TM_PFLAG_DISCARD          0x00000400 /* bit 10 */
#define     TM_PFLAG_LOG              0x00000800 /* bit 11 */
#define     TM_PFLAG_MIPV6_HA_TUNNEL  0x00001000 /* bit 12 */
#define     TM_PFLAG_IPCOMP           0x00002000 /* bit 13 */
#define     TM_PFLAG_64BIT_SEQ_NO     0x00004000 /* bit 14 */
#define     TM_PFLAG_NO_TFC_PADDING   0x00008000 /* bit 15 */
#define     TM_PFLAG_ADD_MULTI_IP     0x00010000 /* bit 16 */

#define     TM_PFLAG_SELMASK          (TM_PFLAG_RIP_PACKET | \
                                       TM_PFLAG_RPT_PACKET | \
                                       TM_PFLAG_LIP_PACKET | \
                                       TM_PFLAG_LPT_PACKET | \
                                       TM_PFLAG_PROTO_PACKET)

#define     TM_PFLAG_PROTOPORTMASK    (TM_PFLAG_RPT_PACKET | \
                                       TM_PFLAG_LPT_PACKET | \
                                       TM_PFLAG_PROTO_PACKET)

/* this protocol mask can be used to compare protocols of two plcyContents */
#define     TM_PFLAGS_PROTO_MASK   (TM_PFLAG_AH + TM_PFLAG_ESP + TM_PFLAG_IPCOMP)

/* 
 * SA options, used in tfSadbRecordSetOptions. When SA expires, we don't
 * rekey this SA, just remove it
 */
#define     TM_IPSEC_SADB_NOREKEYING        1
/* SA options, used in tfSadbRecordSetOptions to indicate that the SA will 
 * not expire at all. 
 */
#define     TM_IPSEC_SADB_NOEXPIRING        2

/* Policy query options */
/* query all policies including those inactive. This is very 
 * important for MIPv6 HA. Originally HA sets all policy with MN
 * to inactive. When MN send a quick mode negotiation, HA should
 * query all its policies (including inactive ones.). Later,
 * when IKE negotiation finishes, HA is responsible to activate
 * the inactive policies.
 */
#define     TM_PQUERY_INCLUDE_INACTIVE    0x01
/* query the policy on MIP virtual interface (tunnelling to HA)*/
#define     TM_PQUERY_MIP_VIR_INTERFACE   0x02
/* setting this bit means manual policies will not be returned */
#define     TM_PQUERY_ONLY_AUTO           0x04
/*
 * IPsec Selectors
 */

/* wild values */
#define     TM_SELECTOR_WILD_PORT       (ttUser16Bit)0
#define     TM_SELECTOR_WILD_PROTOCOL   (ttUser8Bit)0

#define     TM_SELECTOR_WILD_ICMP_TYPECODE (ttUser8Bit)1

/* Actually, to denote a wild ip address for either IPv4
 * and IPv6, we just store zero in the ss_family.
 */

/* local ip selector is a host IP address*/
#define     TM_SELECTOR_LOCIP_HOST      (ttUser16Bit)0x01
/* remote ip selector is a host IP address*/
#define     TM_SELECTOR_REMTIP_HOST     (ttUser16Bit)0x02
/* both ip selectors are host IP addresses */
#define     TM_SELECTOR_BOTHIP_HOST     (ttUser16Bit)0x03 /* = 0x01 | 0x02 */

/* local ip selector is a subnet*/
#define     TM_SELECTOR_LOCIP_SUBNET    (ttUser16Bit)0x04
/* remote ip selector is a subnet */
#define     TM_SELECTOR_REMTIP_SUBNET   (ttUser16Bit)0x08
/* both ip selectors are subnets  */
#define     TM_SELECTOR_BOTHIP_SUBNET   (ttUser16Bit)0x0c /* = 0x04 | 0x08 */

/* local ip selector is an ip range, min + max */
#define     TM_SELECTOR_LOCIP_RANGE     (ttUser16Bit)0x10
/* remote ip selector is an ip range, min + max */
#define     TM_SELECTOR_REMTIP_RANGE    (ttUser16Bit)0x20
/* both ip selectors are ip ranges */
#define     TM_SELECTOR_BOTHIP_RANGE    (ttUser16Bit)0x30 /* = 0x10 | 0x20 */

/* use prefix length to indicate a subnet, rather than input masks */
#define     TM_SELECTOR_USE_PREFIX_LENGTH  (ttUser16Bit)0x100

/* local ip selector type mask */
#define     TM_SELECTOR_LOC_OPTION      (ttUser16Bit)0x15
/*0x15 = localIP_HOST 0x01 + localIP_SUBNET 0x04 + localSRCIP_RANGE 0x10*/

/* remote ip selector type mask */
#define     TM_SELECTOR_REMT_OPTION     (ttUser16Bit)0x2a
/*0x2a = remoteIP_HOST 0x02 + remoteIP_SUBNET 0x08 + localSRCIP_RANGE 0x20*/


/*4.6.2.1 Identification Type Values*/
#define TM_DOI_ID_RESERVED                 (ttUser8Bit)0
#define TM_DOI_ID_IPV4_ADDR                (ttUser8Bit)1
#define TM_DOI_ID_FQDN                     (ttUser8Bit)2
#define TM_DOI_ID_USER_FQDN                (ttUser8Bit)3
#define TM_DOI_ID_IPV4_ADDR_SUBNET         (ttUser8Bit)4
#define TM_DOI_ID_IPV6_ADDR                (ttUser8Bit)5
#define TM_DOI_ID_IPV6_ADDR_SUBNET         (ttUser8Bit)6
#define TM_DOI_ID_IPV4_ADDR_RANGE          (ttUser8Bit)7
#define TM_DOI_ID_IPV6_ADDR_RANGE          (ttUser8Bit)8
#ifdef TM_USE_PKI
#define TM_DOI_ID_DER_ASN1_DN              (ttUser8Bit)9
#define TM_DOI_ID_DER_ASN1_GN              (ttUser8Bit)10
#endif /* TM_USE_PKI */
#define TM_DOI_ID_KEY_ID                   (ttUser8Bit)11

/* RFC 2408 */
#define TM_IKE_COOKIESECRET_REFRESH_RATE   900
#define TM_ISAKMP_COOKIE_SIZE              8
#define TM_ISAKMP_GENERICHEADER_SIZE       4
/* make sure NONCE is divisible by four */
#define TM_ISAKMP_NONCE_DEFAULT_SIZE       16
#define TM_MINIMUM_PUBLIC_VALUE_SIZE       32
#define TM_IKE_ATTRIBUTE_BASIC_LENGTH      4

/* definition for PKI IKE */
#define TM_PKI_RSA_KEY              0
#define TM_PKI_DSA_KEY              1
/* top-level CA ID */
#define TM_PKI_ROOT_CAID            "1.1.1.1"
#define TM_PKI_INTERMEDIARY_CAID    "2.2.2.2"

/* certificate with PEM format */
#define TM_PKI_CERTIFICATE_PEM      (ttUser8Bit)0x01
/* certificate with DER format */
#define TM_PKI_CERTIFICATE_DER      (ttUser8Bit)0x02
/* the input certificate is already string with PEM, instead of file */
#define TM_PKI_CERTIFICATE_STRING   (ttUser8Bit)0x04

/* the certificate is owner self */
#define TM_PKI_CERT_LOCAL           0x01
/* general certificate including CA or other peer's certificate */
#define TM_PKI_CERT_NONLOCAL        0x02
/* the certificate is CRL */
#define TM_PKI_CERT_CRL             0x04
/* this certificate is a root ca's */
#define TM_PKI_CERT_ROOTCA          0x08



typedef struct sadb_sa
{
    ttUser32Bit sadb_sa_spi ;
    ttUser8Bit  sadb_sa_replay ;
    ttUser8Bit  sadb_sa_state ;
    ttUser8Bit  sadb_sa_auth ;
    ttUser8Bit  sadb_sa_encrypt ;
}
sadb_sa ;
typedef sadb_sa  TM_FAR * sadb_saPtr ;

typedef struct tsSaIdentity{
    struct sockaddr_storage  siSrcSockaddr;
    struct sockaddr_storage  siDstSockaddr;
#ifdef TM_USE_STRONG_ESL
/* Physical port mask */
    ttUser32Bit              siPortMask;
#endif /* TM_USE_STRONG_ESL */
    ttUser32Bit              siSpi;
    ttUser8Bit               siProto;
}ttSaIdentity;
typedef ttSaIdentity TM_FAR * ttSaIdentityPtr;


typedef int (TM_CODE_FAR *ttCryptoSessionOpenFuncPtr)(
    ttUserVoidPtr TM_FAR * sessionPtrPtr,
    ttUserVoidPtr          param1Ptr, 
    ttUserVoidPtr          param2Ptr);
typedef int (TM_CODE_FAR *ttCryptoSessionFuncPtr)(
    ttUserVoidPtr              sessionPtr);
typedef int (TM_CODE_FAR *ttCryptoEngineInitFuncPtr)(
    ttUserVoidPtr           initParamPtr);
typedef int (TM_CODE_FAR *ttCryptoGetRandomWordFuncPtr)(
    ttUser32BitPtr       dataPtr,
    ttUser16Bit          wordSize);

typedef struct tsCryptoEngine
{
    ttUser32Bit                   ceEngineId;
    ttUser32Bit                   ceSupportFlags;
    ttCryptoEngineInitFuncPtr     ceInitFuncPtr;
    ttCryptoGetRandomWordFuncPtr  ceRandomWordFuncPtr;
    ttCryptoSessionOpenFuncPtr    ceSessionOpenFuncPtr;
    ttCryptoSessionFuncPtr        ceSessionProcessFuncPtr;
    ttCryptoSessionFuncPtr        ceSessionCloseFuncPtr;
}ttCryptoEngine;
typedef ttCryptoEngine TM_FAR * ttCryptoEnginePtr;
typedef ttCryptoEnginePtr TM_FAR * ttCryptoEnginePtrPtr;

typedef struct tsIcmpType
{
    ttUser8Bit  type;
    ttUser8Bit  code;
    ttUser8Bit  flg;
    ttUser8Bit  padding;        /* padding */
} ttIcmpType;
typedef ttIcmpType TM_FAR * ttIcmpTypePtr;


typedef struct tsIpsecSelector
{
#ifdef TM_USE_STRONG_ESL
    ttUser32Bit                    selPortMask;
#endif /* TM_USE_STRONG_ESL */ 
/* a pair of IP */
    struct sockaddr_storage        selLocIp1; 
    struct sockaddr_storage        selRemtIp1;
/* if selector is a range, we need another pair */
    struct sockaddr_storage        selLocIp2;
    struct sockaddr_storage        selRemtIp2;
    ttIcmpType                     selLocIcmp;
    ttIcmpType                     selRemtIcmp;
#if (defined(TM_6_USE_MIP_MN) || defined(TM_6_USE_MIP_CN))
    ttUser16Bit                    selLocMhMtype;
    ttUser16Bit                    selRemtMhMtype;
#endif /* (defined(TM_6_USE_MIP_MN) || defined(TM_6_USE_MIP_CN)) */
/* port number is already there */
    ttUser16Bit                    selIpFlags; 
    ttUser8Bit                     selProtocol;
    ttUser8Bit                     selPadding;
} 
ttIpsecSelector;
typedef ttIpsecSelector TM_FAR * ttIpsecSelectorPtr;
#ifdef TM_USE_IPV6
#define selLocPort  selLocIp1.addr.ipv6.sin6_port
#define selRemtPort selRemtIp1.addr.ipv6.sin6_port
#define selLocEndPort  selLocIp2.addr.ipv6.sin6_port
#define selRemtEndPort selRemtIp2.addr.ipv6.sin6_port
#else /* ! TM_USE_IPV6 */
#define selLocPort  selLocIp1.addr.ipv4.sin_port
#define selRemtPort selRemtIp1.addr.ipv4.sin_port
#define selLocEndPort  selLocIp2.addr.ipv4.sin_port
#define selRemtEndPort selRemtIp2.addr.ipv4.sin_port
#endif /* TM_USE_IPV6 */

#define selLocIcmpType selLocIcmp.type
#define selLocIcmpCode selLocIcmp.code
#define selLocIcmpFlg selLocIcmp.flg
#define selRemtIcmpType selRemtIcmp.type
#define selRemtIcmpCode selRemtIcmp.code
#define selRemtIcmpFlg selRemtIcmp.flg

#define TM_IKE_MAX_TRANSFORMS     8
#define TM_IPSEC_MAX_TRANSFORMS   4

typedef struct tsPolicyContentInfo
{
    ttUser16Bit         pctiEncryptKeyBits;
#ifdef TM_USE_IKEV2
    ttUser16Bit         pctiEsn;
#endif /* TM_USE_IKEV2 */
    ttUser8Bit          pctiAuthAlg;
    ttUser8Bit          pctiEncryptAlg;
    ttUser8Bit          pctiP2PfsDhGroup;
#ifdef TM_USE_IKEV2
    ttUser8Bit          pctiPadding;
#else  /* TM_USE_IKEV2 */
    ttUser8Bit          pctiPadding[3];
#endif /* TM_USE_IKEV2 */
} ttPolicyContentInfo;
typedef ttPolicyContentInfo TM_FAR * ttPolicyContentInfoPtr;
typedef struct tsPolicyContent{
/* used for bundles, the outer policy content */
    struct tsPolicyContent TM_FAR *
                                pctOuterContentPtr; 
#ifdef TM_USE_STRONG_ESL
    ttUser32Bit                 pctPortMask;
#endif /* TM_USE_STRONG_ESL */ 
    ttPolicyContentInfo         pctPolicyContentInfo;
#ifdef TM_USE_IKEV2
/* Use IKEv2 negotiation only. nothing set this parameter. */
    ttPolicyContentInfo         pctPolicyV2ContentInfo;
#endif /* TM_USE_IKEV2 */
/* for router, it may have multiple ip, need to specify,
 * anyway, we don't care the port number here
 */
    struct sockaddr_storage     pctLocIpAddr; 
    struct sockaddr_storage     pctRemtIpAddr;
/* AH|ESP, tunnel|transport, policy|packet determination */
    ttUser32Bit                 pctRuleFlags; 
/* along with pctQueuePtr, indicates how many bytes data are waiting
 * for an IKE negotiation results
 */
    ttUser32Bit                 pctLifeSeconds;
#ifdef TM_USE_IPSEC_64BIT_KB_LIFETIMES
    ttUser64Bit                 pctLifeKbytes;
#else /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
    ttUser32Bit                 pctLifeKbytes;
#endif /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
    ttUser32Bit                 pctContentIndex;
    ttUser16Bit                 pctOwnerCount;
#ifdef TM_USE_IKEV2
    ttUser16Bit                 pctEncryptKeyBitsArray[TM_IKE_MAX_TRANSFORMS];
/* esp may also have authentication algorithm*/
    ttUser8Bit                  pctAuthAlgArray[TM_IKE_MAX_TRANSFORMS];
    ttUser8Bit                  pctEncryptAlgArray[TM_IKE_MAX_TRANSFORMS];
    ttUser8Bit                  pctP2PfsDhGroupArray[TM_IKE_MAX_TRANSFORMS];
    ttUser8Bit                  pctAuthAlgCount;
    ttUser8Bit                  pctEncryptCount;
    ttUser8Bit                  pctP2PfsDhGroupCount;
#endif /* TM_USE_IKEV2 */

#ifdef TM_USE_IPSEC_IPCOMP
    ttUser8Bit                  pctIpcompAlg;
#endif /* TM_USE_IPSEC_IPCOMP */
    ttUser8Bit                  pctBypassDfBit;
    ttUser8Bit                  pctDscpValue;
    ttUser8Bit                  pctBypassDscp;
    struct tsPolicyContent TM_FAR *
                                pctPrevPtr;
    ttUser8Bit                  pctProposalNumber;
    ttUser8Bit                  pctTransformNumber;
/* pctTransformCount is valid only for the first transform in a proposal */
    ttUser8Bit                  pctTransformCount;
}  ttPolicyContent;
typedef ttPolicyContent    TM_FAR * ttPolicyContentPtr;
typedef ttPolicyContentPtr TM_FAR * ttPolicyContentPtrPtr;

typedef struct tsSadbRecord
{
 
/* use single linked list for same-hash and same-policy
 * those with same hash key will be linked
 */
    struct tsSadbRecord  TM_FAR   * sadbNextSameHashPtr ;
/* those of the same policy must be linked in order for quick 
 * delete and search 
 */
    struct tsSadbRecord  TM_FAR   * sadbNextSamePolicyPtr; 
/* inbound sa  <----->   outbound sa. They are twins.*/
    struct tsSadbRecord  TM_FAR   * sadbTwinSaPtr; 
/* This is the back pointer to policy for policy check*/
    struct tsPolicyEntry TM_FAR   * sadbPolicyPtr; 
/* This is a pointer to the next outer content bundle SA */
    struct tsSadbRecord  TM_FAR   * sadbNextOuterBundlePtr;
/* This is a pointer to the next inner content bundle SA */
    struct tsSadbRecord  TM_FAR   * sadbNextInnerBundlePtr;
/* used by IKE rekeying */
    ttPolicyContentPtr              sadbPlcyContentPtr; 
    ttUser8BitPtr                   sadbReplayWindowMapPtr ;
    ttIpsecSelectorPtr              sadbSelectorPtr;
    ttUserVoidPtr                   sadbSoftTimerPtr; 
    ttUserVoidPtr                   sadbHardTimerPtr;
#ifdef TM_SA_PMTU
    ttUserVoidPtr                   sadbPmtuTimerPtr;
#endif /* TM_SA_PMTU */
/* Session pointer, may be used for hardware accelerator */
    ttUserVoidPtr                   sadbSessionPtr;
/* lifetime remain bytes */
#ifdef TM_USE_IPSEC_64BIT_KB_LIFETIMES
    ttUser64Bit                     sadbRemainBytes;
    ttUser64Bit                     sadbLifetimeBytesSoftLimit;
    ttUser64Bit                     sadbLifetimeBytesHard;
#else /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
    ttUser32Bit                     sadbRemainBytes;
    ttUser32Bit                     sadbLifetimeBytesSoftLimit;
    ttUser32Bit                     sadbLifetimeBytesHard;
#endif /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
    ttUser32Bit                     sadbLifeSeconds;
/* the Src and Dst IP is required by ipsec*/
    struct sockaddr_storage         sadbSrcIpAddress; 
/* they may be different with the selectors */
    struct sockaddr_storage         sadbDstIpAddress; 
#ifdef TM_USE_STRONG_ESL
    ttUser32Bit                     sadbPortMask;
#endif /* TM_USE_STRONG_ESL */
    struct tsGenericKey             sadbAuthKey;
    struct tsGenericKey             sadbEncryptKey;
/* for software crypto engine, we need to assign values for
 * the followign two fields. For hardware crypto engine,
 * the following two fields are not used.
 */
    ttAhAlgorithmPtr                sadbAuthAlgPtr;
    ttEspAlgorithmPtr               sadbEncryptAlgPtr;
    ttCryptoEnginePtr               sadbCryptoEnginePtr;
#ifdef TM_USE_IPSEC_IPCOMP
    ttCompEnginePtr                 sadbCompEnginePtr;
#endif /* TM_USE_IPSEC_IPCOMP */
    sadb_saPtr                      sadbSAPtr;
#ifdef TM_IPSEC_USE_64BIT_SEQ_NO
    ttUser64Bit                     sadbSequenceNumber;
#else
    ttUser32Bit                     sadbSequenceNumber;
#endif /* TM_IPSEC_USE_64BIT_SEQ_NO */
    ttUser32Bit                     sadbReplayWindowBegin;
#ifdef TM_SA_PMTU
/* Adjusted PMTU to account for IPsec overhead */
    ttUser32Bit                     sadbAdjustedPmtu;
#endif /* TM_SA_PMTU */

#ifdef TM_USE_IKE_DPD
    ttUser32Bit                     sadbIkeDpdTimestamp;
    ttUser32Bit                     sadbIkeDpdWorryMetric;
#endif /* TM_USE_IKE_DPD */
#ifdef TM_USE_SEC_STAT
    ttUser32Bit                     sadbTotalRekeys;
    ttUser32Bit                     sadbByteRekeys;
    ttUser32Bit                     sadbTimeRekeys;
    ttUser32Bit                     sadbPeerRekeys;
    ttUser32Bit                     sadbOtherRekeys;
#endif /* TM_USE_SEC_STAT */
    ttUser32Bit                     sadbSerialNumber;
    ttUser32Bit                     sadbNonceValue;
#if defined(TM_USE_VIPER) || defined (TM_USE_VIPER_TEST)
    ttUser32Bit                     sadbSaIndex;
#endif /* defined(TM_USE_VIPER) || defined (TM_USE_VIPER_TEST) */
    ttUser16Bit                     sadbFlags;
/* this record may cached in a socketentryptr */
    ttUser16Bit                     sadbCheckOut;
/* How many entities own this sadb record */
    ttUser16Bit                     sadbOwnerCount;
/* incoming or outgoing */
    ttUser8Bit                      sadbDirection; 
/* Ah, Esp save TM_IP_AH. or TM_IP_ESP */
    ttUser8Bit                      sadbSaProtocol; 
/* transport tunnel */
    ttUser8Bit                      sadbIpsecMode;
    ttUser8Bit                      sadbIsInitiator;
#ifdef TM_USE_SEC_STAT
    ttUser8Bit                      sadbExpiredFlags;
#endif /* TM_USE_SEC_STAT */
#if defined(TM_USE_VIPER) || defined (TM_USE_VIPER_TEST)
    ttUser8Bit                      sadbBundleSize;
#endif /* defined(TM_USE_VIPER) || defined (TM_USE_VIPER_TEST) */
#ifdef TM_USE_IKEV2
    ttUser8Bit                      sadbByIkev2;
#endif /* TM_USE_IKEV2 */
}
ttSadbRecord ;
typedef ttSadbRecord TM_FAR * ttSadbRecordPtr ;
typedef ttSadbRecordPtr  TM_FAR * ttSadbRecordPtrPtr ;


typedef void TM_FAR * ttIkePolicyHandle;
typedef ttIkePolicyHandle TM_FAR * ttIkePolicyHandlePtr;


typedef struct tsIpsecPolicyPair
{
    ttIkePolicyHandle         ippIkePlcyHandle;
    ttUser16Bit               ippPriority;
    ttUser16Bit               ippSelectorIndex;
    ttUser8Bit                ippPlcycontentIndex;
    ttUser8Bit                ippDirection;
    ttUser8Bit                ippProposalNumber;
    ttUser8Bit                ippTransformNumber;
    ttUser8Bit                ippTransformCount;
/* when non-zero, designates a manual-only policy (won't key w/IKE) */
    ttUser8Bit                ippManual;
}ttIpsecPolicyPair;
typedef struct tsIpsecPolicyPair TM_FAR * ttIpsecPolicyPairPtr;

typedef struct tsIpsecSelectorInString{
/* first pair of local and remote */
    char    TM_FAR     * selstrLocIp1;
    union 
    {
        int              selstr_loc_prefix_length;
        char TM_FAR    * selstr_loc_ip2;
    } selstrLocIp2Union;
/* if in range, we need another pair */
    char TM_FAR       *  selstrRemtIp1;
    union 
    {
        int              selstr_remt_prefix_length;
        char TM_FAR   *  selstr_remt_ip2;
    } selstrRemtIp2Union;
#ifdef TM_USE_STRONG_ESL
    ttUser32Bit          selstrPortMask;
#endif /* TM_USE_STRONG_ESL */
    ttUser16Bit          selstrIpFlags; 
/* local port and remote port */
    union 
    {
        ttUser16Bit          selstr_loc_port;
        ttUser16Bit          selstr_loc_mh_type;
    } selstrLocPortUnion;
    ttIcmpType           selStrLocIcmp;
    
    union
    {
        ttUser16Bit          selstr_remt_port;
        ttUser16Bit          selstr_remt_mh_type;
    } selstrRemtPortUnion;
    ttIcmpType           selStrRemtIcmp;

    ttUser16Bit          selstrProtocol;
#define selstrLocPrefixLength  selstrLocIp2Union.selstr_loc_prefix_length
#define selstrLocIp2           selstrLocIp2Union.selstr_loc_ip2
#define selstrRemtPrefixLength selstrRemtIp2Union.selstr_remt_prefix_length
#define selstrRemtIp2          selstrRemtIp2Union.selstr_remt_ip2
#define selstrLocPort          selstrLocPortUnion.selstr_loc_port
#define selstrLocMhType        selstrLocPortUnion.selstr_loc_mh_type
#define selstrLocIcmpType      selStrLocIcmp.type
#define selstrLocIcmpCode      selStrLocIcmp.code
#define selstrLocIcmpFlg       selStrLocIcmp.flg
#define selstrRemtPort         selstrRemtPortUnion.selstr_remt_port
#define selstrRemtMhType       selstrRemtPortUnion.selstr_remt_mh_type
#define selstrRemtIcmpType     selStrRemtIcmp.type
#define selstrRemtIcmpCode     selStrRemtIcmp.code
#define selstrRemtIcmpFlg      selStrRemtIcmp.flg

} ttIpsecSelectorInString;
typedef ttIpsecSelectorInString TM_FAR * ttIpsecSelectorInStringPtr;

#ifdef TM_USE_IKEV2
#define TM_IKEV2_PLCY_DONT_SEND_IKE 0x01
#endif /* TM_USE_IKEV2 */

typedef struct tsPolicyContentInString{
/* for router, it may have multiple ip, need to specify */
    char TM_FAR *             pctstrLocIpAddr; 
    char TM_FAR *             pctstrRemtIpAddr;
    ttPolicyContentInfo       pctstrPolicyContentInfo;
/* AH|ESP, tunnel|transport, policy|packet determination */
    ttUser32Bit               pctstrRuleFlags; 
#ifdef TM_USE_IKEV2
/* esp may also have authentication algorithm*/
    ttUser8Bit               pctstrAuthAlgArray[TM_IKE_MAX_TRANSFORMS];
    ttUser8Bit               pctstrEncryptAlgArray[TM_IKE_MAX_TRANSFORMS];
#endif /* TM_USE_IKEV2 */
#ifdef TM_USE_IPSEC_IPCOMP
    ttUser8Bit                pctstrIpcompAlg;
#endif /* TM_USE_IPSEC_IPCOMP */
/* BUG 565, we decide to add the following two fields of lifetime*/
/* lifetime value in seconds. zero value(0) means to use the 
 * default value TM_IPSECSA_TLIFETIME_DEFAULT 
 */
    ttUser32Bit               pctstrLifeSeconds;
/* lifetime value in kilo bytes. zero value(0) means to use the 
 * default value TM_IPSECSA_VLIFETIME_DEFAULT 
 */
#ifdef TM_USE_IPSEC_64BIT_KB_LIFETIMES
    ttUser64Bit               pctstrLifeKbytes;
#else /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
    ttUser32Bit               pctstrLifeKbytes;
#endif /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
#ifdef TM_USE_STRONG_ESL
    ttUser32Bit               pctstrPortMask;
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IKEV2
/* encryption algorithm key length in bits. 0 means to use the
 * the default key length 
 */
    ttUser16Bit               pctstrEncryptKeyBitsArray[TM_IKE_MAX_TRANSFORMS];
/* Phase 2 PFS DH Group - 0 means Phase 2 PFS is disabled */
    ttUser8Bit                pctstrP2PfsDhGroupArray[TM_IKE_MAX_TRANSFORMS];
    ttUser8Bit                pctstrAuthAlgCount;
    ttUser8Bit                pctstrEncryptCount;
    ttUser8Bit                pctstrP2PfsDhGroupCount;
#endif /* TM_USE_IKEV2 */
    ttUser8Bit                pctstrBypassDfbit;
    ttUser8Bit                pctstrDscpValue;
    ttUser8Bit                pctstrBypassDscp;
}  ttPolicyContentInString;
typedef ttPolicyContentInString TM_FAR * ttPolicyContentInStringPtr;

typedef struct tsPolicyEntry{
    struct tsPolicyEntry TM_FAR *   plcyNextPtr;
/* plcyContentPtr is the transform list, this is the tail */
    struct tsPolicyContent TM_FAR * plcyLastContentPtr;
/* link all SA of the same policy*/
    ttSadbRecordPtr                 plcyInboundSadbPtr; 
    ttSadbRecordPtr                 plcyOutboundSadbPtr;
    ttIpsecSelector                 plcySelector;
    ttPolicyContentPtr              plcyContentPtr;
    ttIkePolicyHandle               plcyIkePlcyHandle;
    ttUser32Bit                     plcySerialNumber;
/* How many entities own this policy entry */
    ttUser16Bit                     plcyOwnerCount;
/* How many entities own this policy content entry */
    ttUser16Bit                     plcyIndex; 
/* ipsec overhead header size*/
    ttUser16Bit                     plcyHdrLen; 
/* ipsec overhead trailer */
    ttUser16Bit                     plcyTrailerLen; 
    ttUserPacketPtr                 plcyQueuePtr;
    ttUserPacketPtr                 plcyLastQueuePtr;
    ttUser16Bit                     plcyQueueBytes;
    ttUser8Bit                      plcyProposalCount;
/* 1 for in,2 for out, 3 for both */
    ttUser8Bit                      plcyDirection; 
    ttUser8Bit                      plcyAction;
    ttUser16Bit                     plcyPriority;
#ifdef TM_IPSEC_POLICY_USER_DATA
    ttUserVoidPtr                   plcyUserDataPtr;
    ttUser32Bit                     plcyUserDataLen;
#endif /* TM_IPSEC_POLICY_USER_DATA */
#ifdef TM_USE_SEC_STAT
    ttUser16Bit                     plcySaInboundCount;
    ttUser16Bit                     plcySaOutboundCount;
#endif /* TM_USE_SEC_STAT */
/* Manual Policy indication */
    ttUser8Bit                      plcyFlags;
#ifdef TM_USE_IKEV2
    ttUser8Bit                      plcyIkev2Flags;
#endif /* TM_USE_IKEV2 */
}ttPolicyEntry;
typedef ttPolicyEntry TM_FAR *    ttPolicyEntryPtr;
typedef ttPolicyEntryPtr TM_FAR * ttPolicyEntryPtrPtr;

/*
 * This was moved from trike.h because it's now IKE policy specific,
 * thus exposed to public APIs declared later in this file.  We
 * leave the previous decl unchanged for legacy purposes - it serves
 * as a 2 member array off tvIkeGlobal.
 *
 */
typedef struct tsIsakmpTransInfo
{
    ttUser32Bit    transLifeSeconds;
    ttUser32Bit    transLifeKbytes;
    ttUser16Bit    transKeyLength[TM_IKE_MAX_TRANSFORMS];
    ttUser16Bit    transEncryptAlg[TM_IKE_MAX_TRANSFORMS];
    ttUser16Bit    transHashAlg[TM_IKE_MAX_TRANSFORMS];
#ifdef TM_USE_IKEV2
    ttUser16Bit    transPrfAlg[TM_IKE_MAX_TRANSFORMS];
#endif /* TM_USE_IKEV2 */
    ttUser8Bit     transDhgroup[TM_IKE_MAX_TRANSFORMS];
    ttUser8Bit     transEncryptCount;
    ttUser8Bit     transHashAlgCount;
#ifdef TM_USE_IKEV2
    ttUser8Bit     transPrfAlgCount;
#endif /* TM_USE_IKEV2 */
    ttUser8Bit     transDhgroupCount;
}ttIsakmpTransInfo;
typedef struct tsIsakmpTransInfo TM_FAR * ttIsakmpTransInfoPtr;

typedef struct tsIkeMyAuthDataInfo
{
    ttUserVoidPtr             ikaIkeIdPtr;
    ttUserVoidPtr             ikaAuthDataPtr;
    ttUser8Bit                ikaIkeIdType;
    ttUser8Bit                ikaIkeIdLength;
    ttUser8Bit                ikaAuthMethod;
}ttIkeMyAuthDataInfo;
typedef ttIkeMyAuthDataInfo TM_FAR * ttIkeMyAuthDataInfoPtr;

typedef struct tsPadInfo
{
    ttIkeMyAuthDataInfo       piMyAuthDataInfo;
    ttUserVoidPtr             piEntryIdPtr;
    ttUserVoidPtr             piAuthDataPtr;
#ifdef TM_USE_IKEV2
    ttUserVoidPtr             piGatewayInfoPtr;
#endif /* TM_USE_IKEV2 */
    ttUser16Bit               piEntryIdLen;
    ttUser16Bit               piAuthDataLen;
#ifdef TM_USE_IKEV2
    ttUser16Bit               piGatewayInfoLen;
#endif /* TM_USE_IKEV2 */
    ttUser8Bit                piEntryIdType;
    ttUser8Bit                piAuthMethod;
#ifdef TM_USE_IKEV2
    ttUser8Bit                piChildSaAuthData;
    ttUser8Bit                piGatewayInfoType;
#endif /* TM_USE_IKEV2 */
    ttUser8Bit                piAuthProtocol;
} ttPadbInfo;
typedef ttPadbInfo TM_FAR * ttPadbInfoPtr;
typedef ttPadbInfoPtr TM_FAR * ttPadbInfoPtrPtr;

/* IKE Policy definition - uses existing transform definition */
typedef struct tsIkePolicyInfo
{
    ttIpsecSelector           ikpSelector;
#ifdef TM_USE_STRONG_ESL
    ttUser32Bit               ikpPortMask;
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IKE_DPD
    ttUser32Bit               ikpDpdWorryMetric;
    ttUser32Bit               ikpDpdRetransDelay;
    ttUser8Bit                ikpDpdRetransLimit;
    ttUser8Bit                ikpDpdMode;
#endif /* TM_USE_IKE_DPD */
/* Defines IKE mode, role, etc.  Defined in trsecapi.h */
    ttUser16Bit               ikpAttributes;
#ifdef TM_USE_NATT
/* Keep Alive interval in seconds */
    ttUser8Bit                ikpNattKAInterval;
#endif /* TM_USE_NATT */
#if defined(TM_USE_IKEV2) && defined(TM_IPSEC_TFC_PADDING) 
    ttUser8Bit                ikpTfcPaddingEnable;
#endif /* defined(TM_USE_IKEV2) && defined(TM_IPSEC_TFC_PADDING) */
}ttIkePolicyInfo;
typedef ttIkePolicyInfo TM_FAR * ttIkePolicyInfoPtr;
typedef ttIkePolicyInfoPtr TM_FAR * ttIkePolicyInfoPtrPtr;

/* AH + ESP + IPcomp, AH + ESP, AH + IPcomp, ESP + IPcomp,
 * AH, ESP
 */
#define TM_IPSEC_MAX_PROPOSAL     5

typedef struct tsDiffieHellman
{
/* Prime used, for IKE, we don't use this, because we know the prime
 * given the dhgroup number. However for TLS, the prime is unknown
 */
    ttUser8BitPtr           dhPrimePtr;
/* my secret value, xi or xr depends */
    ttUser8BitPtr           dhSecretPtr;       
/* Initiator public value  g^xi % p */
    ttUser8BitPtr           dhGxiPtr;           
/* Responder public value  g^xr % p */
    ttUser8BitPtr           dhGxrPtr;
/* Our derived shared Secret */
    ttUser8BitPtr           dhGxyPtr;
/* generator can be at most 4 octets */
    ttUser32Bit             dhGenerator;
/* length for prime, group 1 is 96, and group 2 is 128 */
    ttUser16Bit             dhGroupLen;
    ttUser16Bit             dhGxiLen;
    ttUser16Bit             dhGxrLen;
    ttUser16Bit             dhGxyLen;
/* group number, 1-4 currently */ 
    ttUser8Bit              dhGroup;        
/* length for dhSecretPtr, maximum 256 bytes */
    ttUser8Bit              dhSecretLen;  

}ttDiffieHellman;
typedef struct tsDiffieHellman TM_FAR * ttDiffieHellmanPtr;

typedef struct tsCryptoRequestDhrequest
{
    ttDiffieHellmanPtr    crdDhinfoPtr;
    ttUser32Bit           crdIsInitialtor;
}ttCryptoRequestDhrequest;
typedef ttCryptoRequestDhrequest TM_FAR * ttCryptoRequestDhrequestPtr;


#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
typedef struct tsCryptoRequestSsl
{
    ttUserPacketPtr       crsPacketPtr;
    ttUserVoidPtr         crsConnectStatePtr;
    ttUserVoidPtr         crsSslHeaderPtr;
    ttUser8BitPtr         crsMacOutPtr;
/* This field is used for both input and output. For input, it contains
 * the length of data-to-be-encrypt or data-to-be-decrypt. (Not include
 * the SSL header length)
 * For encode output, this returns the record length when send into the 
 * network; For decode output, this returns the original record length, 
 * after stripping of MAC output, padLength field and paddings
 */
    ttUser16BitPtr        crsRecordLenPtr;
    ttUser16Bit           crsCryptHeadLength;
}
ttCryptoRequestSsl;
typedef ttCryptoRequestSsl TM_FAR * ttCryptoRequestSslPtr;
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */

#ifdef TM_USE_IKEV2
#define TM_IKE_CHD_OPT_FLAGS_ESN_ON 0x01
#define TM_IKE_CHD_OPT_FLAGS_TFC_ON 0x02
#endif /* TM_USE_IKEV2 */

/* the information used to generate brandnew SA or rekeying sa.*/
typedef struct tsChildSaInfo
{
    ttPolicyEntryPtr                  chdPlcyPtr;
    ttPolicyContentPtr                chdPlcyContentPtr;
    ttIpsecSelector                   chdPacketSelector;
    ttUser32Bit                       chdMySpi; /* in host order*/
    ttUser32Bit                       chdPeerSpi;
    ttUser32Bit                       chdLifetimeSeconds;
#ifdef TM_USE_IPSEC_64BIT_KB_LIFETIMES
    ttUser64Bit                       chdLifetimeKbytes;
#else /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
    ttUser32Bit                       chdLifetimeKbytes;
#endif /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
    ttUser16Bit                       chdAuthKeyBytes;
    ttUser16Bit                       chdEncryptKeyBytes;
#ifdef TM_USE_IKE_DPD
    ttUser32Bit                       chdIkeDpdWorryMetric;
#endif /* TM_USE_IKE_DPD */
#ifdef TM_USE_SEC_STAT
    ttUser32Bit                       chdTotalRekeys;
    ttUser32Bit                       chdByteRekeys;
    ttUser32Bit                       chdTimeRekeys;
    ttUser32Bit                       chdPeerRekeys;
    ttUser32Bit                       chdOtherRekeys;
#endif /* TM_USE_SEC_STAT */
#ifdef TM_USE_NATT
    ttUser32Bit                       chdNatOaDstAddr;
    ttUser32Bit                       chdNatOaSrcAddr;
    ttUser16Bit                       chdLocalPort;
    ttUser16Bit                       chdPeerPort;
#endif /* TM_USE_NATT */
    ttUser8Bit                        chdSpiPending;
#ifdef TM_USE_IKE_VENDOR_ATTR
#ifdef TM_USE_TRECK_ONE_SIDE_REKEY
    ttUser8Bit                        chdVendorNegotFlags;
#endif /* TM_USE_TRECK_ONE_SIDE_REKEY */
#endif /* TM_USE_IKE_VENDOR_ATTR */
#ifdef TM_USE_VIPER
    ttUser8Bit                        chdRenewSa;
#endif /* TM_USE_VIPER */
#ifdef TM_USE_NATT
/* Save NAT detection info for IKE rekey */
    ttUser8Bit                        chdNatPresentFlags;
#endif /* TM_USE_NATT */
#ifdef TM_USE_IKEV2
    ttUser8Bit                        chdIsIke2;
    ttUser8Bit                        chdOptFlags;
#endif /* TM_USE_IKEV2 */
}ttChildSaInfo;
typedef ttChildSaInfo TM_FAR * ttChildSaInfoPtr;


typedef struct tsChildSaInfoList
{
    struct tsChildSaInfoList TM_FAR *   chdlstNextPtr;
    struct tsChildSaInfo                chdlstSaInfo;
    struct tsChildSaInfoList TM_FAR *   chdlstNextOuterPropPtr;
    ttUser16Bit                         chdlstOwnerCount;
    ttUser16Bit                         chdlstFlags;
} ttChildSaInfoList;
typedef struct tsChildSaInfoList TM_FAR * ttChildSaInfoListPtr;

#if defined(TM_USE_IKEV2) && defined(TM_USE_EAP_MSCHAPV2)
typedef struct tsPadbEapMsChapV2Info
{
    char   TM_FAR * userNamePtr;
    char   TM_FAR * passwordPtr;
} ttPadbEapMsChapV2Info;
typedef ttPadbEapMsChapV2Info TM_FAR * ttPadbEapMsChapV2InfoPtr;
#endif /* TM_USE_IKEV2 && TM_USE_EAP_MSCHAPV2 */

typedef struct tsCryptoEngineList
{
    struct tsCryptoEngineList TM_FAR * celNextPtr;
    ttCryptoEnginePtr                  celCryptoEnginePtr;
}ttCryptoEngineList;
typedef ttCryptoEngineList TM_FAR * ttCryptoEngineListPtr;

typedef struct tsCryptoChainData
{
    struct tsCryptoChainData TM_FAR * ccdNextPtr;
    ttUser8BitPtr                     ccdDataPtr;
    ttPktLen                          ccdDataLen;
}ttCryptoChainData;
typedef ttCryptoChainData TM_FAR * ttCryptoChainDataPtr;

typedef struct tsCryptoRequestHash
{
    ttAhAlgorithmPtr      crhAlgPtr;
    ttCryptoChainDataPtr  crhInDataPtr;
    ttUser8BitPtr         crhOutPtr;
    ttUser16Bit           crhHashAlg;
    ttUser16Bit           crhDataLength;
}ttCryptoRequestHash;
typedef ttCryptoRequestHash TM_FAR * ttCryptoRequestHashPtr;

typedef struct tsCryptoRequestHmacHash
{
    ttAhAlgorithmPtr      crhhAlgPtr;
    ttCryptoChainDataPtr  crhhInDataPtr;
    ttUser8BitPtr         crhhOutPtr;
    ttUser8BitPtr         crhhKeyPtr;
    ttUser32Bit           crhhDataLength;
    ttUser16Bit           crhhKeyLen;
    ttUser16Bit           crhhHashAlg;
}ttCryptoRequestHmacHash;
typedef ttCryptoRequestHmacHash TM_FAR * ttCryptoRequestHmacHashPtr;

typedef struct tsCryptoRequestIpsec
{
    ttUserPacketPtr       criPacketPtr;
    ttUserPacketPtr       criNewPacketPtr;
    ttSadbRecordPtr       criSadbPtr;
    ttUser8BitPtr         criIvPtr;
    ttUser8BitPtr         criMacOutPtr;
    ttUser8BitPtr         criOrigMacPtr;
    ttUser16Bit           criCryptHeadLength;
    ttUser16Bit           criCryptDataLength;
    ttUser16Bit           criMacHeadLength;
    ttUser16Bit           criMacDataLength;
}ttCryptoRequestIpsec;
typedef ttCryptoRequestIpsec TM_FAR * ttCryptoRequestIpsecPtr;

#if defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2)
typedef struct tsCryptoRequestIke
{
    struct tsIsakmpState TM_FAR * criIsakmpPtr;
    ttUser8BitPtr                 criDataPtr;
    ttUser32Bit                   criDataLen;
}ttCryptoRequestIke;
typedef ttCryptoRequestIke TM_FAR * ttCryptoRequestIkePtr;
#endif /* defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2) */


#ifdef TM_USE_PKI

#define TM_CERTFLAG_BADCERT      0x01    /* critical error! */
#define TM_CERTFLAG_REVOKED      0x02    /* revoked cert    */
#define TM_CERTFLAG_EXPIRED      0x04    /* expired cert    */
/* The certificate doesn't belong to the identity we are used to verify*/
#define TM_CERTFLAG_UNMATCHED    0x08
/* CA unknown */
#define TM_CERTFLAG_UNKNOWNCA    0x10
/* The mentioned CA fails to verify the certificate */
#define TM_CERTFLAG_NOTVERIFIED  0x20

typedef struct tsAsn1IntegerSt
{
/* for big number integer, 
 * asn1Length: total bytes to respresent the integer
 * asn1Type  : value type (not useful for crypto engine )
 * asn1Data  : date pointer to the integer or value, 
 *             MSB (most important bit is first )
 */
    int             asn1Length;
    int             asn1Type;
    ttUser8BitPtr   asn1Data;
} ttAsn1Integer;
typedef ttAsn1Integer    TM_FAR *  ttAsn1IntegerPtr;
typedef ttAsn1IntegerPtr TM_FAR *  ttAsn1IntegerPtrPtr;


#ifdef TM_PUBKEY_USE_DSA
typedef struct tsGeneralDsa
{
/* DSA key, p,q,g, Pub_key, Pri_key, r,s  */
    ttAsn1IntegerPtr    dsaP;
    ttAsn1IntegerPtr    dsaQ;
    ttAsn1IntegerPtr    dsaG;
    ttAsn1IntegerPtr    dsaPubKey;
    ttAsn1IntegerPtr    dsaPriKey;
    ttAsn1IntegerPtr    dsaR;
    ttAsn1IntegerPtr    dsaS;
} ttGeneralDsa;  
typedef ttGeneralDsa TM_FAR *  ttGeneralDsaPtr;

typedef struct tsCryptoRequestDsarequest
{
/* signature is stored in crdDsainfoPtr */
    ttGeneralDsaPtr                 crdDsainfoPtr;
/* hashed message (data) pointer */
    ttUser8BitPtr                   crdDsaDataPtr;
/* cryption (signature) pointer */
    ttUser8BitPtr                   crdDsaTransPtr;
/* hashed message length */
    int                             crdDsaDataLength;
/* signature length pointer */
    int                     TM_FAR *crdDsaTransLenPtr;
}ttCryptoRequestDsarequest;
typedef ttCryptoRequestDsarequest TM_FAR * ttCryptoRequestDsarequestPtr;

#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_PUBKEY_USE_RSA
/* definition for rsa/dsa */
typedef struct tsGeneralRsa
{
/* RSA key, n,e,d,p,q, Pm,Qm, Pinv */
    ttAsn1IntegerPtr    rsaN;
    ttAsn1IntegerPtr    rsaE;
    ttAsn1IntegerPtr    rsaD;
    ttAsn1IntegerPtr    rsaP;
    ttAsn1IntegerPtr    rsaQ;
    ttAsn1IntegerPtr    rsaPm;
    ttAsn1IntegerPtr    rsaQm;
    ttAsn1IntegerPtr    rsaPinv;
} ttGeneralRsa;  
typedef ttGeneralRsa TM_FAR *  ttGeneralRsaPtr;

typedef struct tsCryptoRequestRsarequest
{
/* rsa key info ptr */
    ttGeneralRsaPtr                 crrRsainfoPtr;
/* hashed message (data) pointer */
    ttUser8BitPtr                   crrRsaDataPtr;
/* cryption (signature) pointer */
    ttUser8BitPtr                   crrRsaTransPtr;
/* hashed message length */
    int                             crrRsaDataLength;
/* signature length pointer */
    int                     TM_FAR *crrRsaTransLenPtr;
}ttCryptoRequestRsarequest;
typedef ttCryptoRequestRsarequest TM_FAR * ttCryptoRequestRsarequestPtr;
#endif /* TM_PUBKEY_USE_RSA */

typedef ttUserVoidPtr * ttCertHandle;
typedef ttCertHandle * ttCertHandlePtr;

#endif /* TM_USE_PKI */


typedef struct tsCertificateEntry {
    const char TM_FAR * certDataPtr;/* points to start of raw certificate*/
    int                 certLength; /* length of raw certificate */
    int                 certFlags;  /* flag to indicate status of this cert*/
}ttCertificateEntry;
typedef ttCertificateEntry TM_FAR * ttCertificateEntryPtr;

typedef struct tsCryptoRequest
{
    union
    {
        ttCryptoRequestHashPtr        crHashParamPtr;
        ttCryptoRequestHmacHashPtr    crHmacHashParamPtr;
#ifdef TM_USE_IPSEC
        ttCryptoRequestIpsecPtr       crIpsecParamPtr;
#endif /* TM_USE_IPSEC */
#if defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2)
        ttCryptoRequestIkePtr         crIkeParamPtr;
#endif /* defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2) */
        ttCryptoRequestDhrequestPtr   crDhgroupParamPtr;
#ifdef TM_PUBKEY_USE_RSA
        ttCryptoRequestRsarequestPtr  crRsaParamPtr;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
        ttCryptoRequestDsarequestPtr  crDsaParamPtr;
#endif /* TM_PUBKEY_USE_DSA */
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
        ttCryptoRequestSslPtr         crSslParamPtr;
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
    }crParamUnion;

    ttUser16Bit                       crResult;
    ttUser8Bit                        crType;
    ttUser8Bit                        crFlags;
}ttCryptoRequest;
typedef ttCryptoRequest TM_FAR * ttCryptoRequestPtr;

#ifdef TM_USE_IPSEC
#ifdef TM_USE_IPSEC_IPCOMP
typedef struct  tsCompRequestIpsec
{       
    ttUserPacketPtr                   comprIpsecPacketPtr;
    ttUser32BitPtr                    comprIpsecTotalLenPtr;
    ttUser8BitPtr                     comprIpsecNewPacketPtr;
    ttSadbRecordPtr                   comprIpsecSadbPtr;
    ttUser16Bit                       comprIpsecHeadLength;
    ttUser16Bit                       comprIpsecDataLength;
}ttCompRequestIpsec;
typedef ttCompRequestIpsec TM_FAR * ttCompRequestIpsecPtr;


typedef struct  tsCompRequest
{
    ttCompRequestIpsecPtr             comprIpsecParamPtr;
    ttUser16Bit                       comprResult;
    ttUser8Bit                        comprType;
    ttUser8Bit                        comprFlags;
}ttCompRequest; 
typedef ttCompRequest TM_FAR * ttCompRequestPtr;
#endif /* TM_USE_IPSEC_IPCOMP*/
#endif /* TM_USE_IPSEC */
/*
 * AH, ESP algorithm lookup API
 */
TM_PROTO_EXTERN ttAhAlgorithmPtr tfAhAlgorithmLookup(int idx);
TM_PROTO_EXTERN ttEspAlgorithmPtr tfEspAlgorithmLookup(int idx);

/*
 * Use IPsec API 
 */
TM_PROTO_EXTERN int tfUseIpsec(void);

TM_PROTO_EXTERN int tfIpsecUninitialize(void);

TM_PROTO_EXTERN int tfIpsecSetOptions(ttUser32Bit       option,
                                      ttUser8Bit        value);
TM_PROTO_EXTERN int tfPkiSetOptions(ttUser32Bit         option,
                                    ttUser32Bit         value);

/* Get crypto engine that supports algorithm */
TM_PROTO_EXTERN ttCryptoEnginePtr tfCryptoEngineGet(int       algorithm);
/* Get crypto engine that supports both algorithms */
TM_PROTO_EXTERN ttCryptoEnginePtr tfCryptoEngineGet2(
                                        int algorithm1, int algorithm2);
/* Get first crypto engine */
TM_PROTO_EXTERN ttCryptoEnginePtr tfCryptoEngineGetFirst(void);

TM_PROTO_EXTERN int tfCryptoEngineRegister(
                           unsigned int                 engineId,
                           ttUserVoidPtr                initParamPtr,
                           ttCryptoEnginePtrPtr         newEnginePtrPtr,
                           ttCryptoEngineInitFuncPtr    engineInitFuncPtr,
                           ttCryptoGetRandomWordFuncPtr randomFuncPtr,
                           ttCryptoSessionOpenFuncPtr   sessionOpenFuncPtr,
                           ttCryptoSessionFuncPtr       sessionProcessFuncPtr,
                           ttCryptoSessionFuncPtr       sessionCloseFuncPtr);
TM_PROTO_EXTERN int tfCryptoEngineDeRegister(unsigned int    engineId);

TM_PROTO_EXTERN int tfCryptoEngineAddAlgorithm(
                           ttCryptoEnginePtr          cenginePtr,
                           unsigned int               algorithmId);

TM_PROTO_EXTERN int tfCryptoEngineSwapUserBufsStart(
                           ttCryptoRequestIpsecPtr    reqPtr);

TM_PROTO_EXTERN void tfCryptoEngineSwapUserBufsFinish(
                           ttCryptoRequestIpsecPtr    reqPtr);

TM_PROTO_EXTERN void tfCryptoEncAlgToStr(ttUser8Bit module, 
                                         ttUser16Bit encAlg, 
                                         ttUserConstCharPtrPtr bufferPtrPtr);
TM_PROTO_EXTERN void tfCryptoHashAlgToStr(ttUser8Bit module, 
                                          ttUser16Bit hashAlg, 
                                          ttUserConstCharPtrPtr bufferPtrPtr);


#ifdef TM_USE_SW_CRYPTOENGINE
int tfCryptoSoftwareEngineSessionProcess(ttUserVoidPtr cryptoRequestPtr);
#endif /* TM_USE_SW_CRYPTOENGINE */
#ifdef TM_USE_MCF5235_CRYPTOENGINE
int tfMcf5235EngineInit(ttUserVoidPtr initParamPtr);
int tfMcf5235GetRandomWord(ttUser32Bit TM_FAR * dataPtr,
                           ttUser16Bit          wordSize);
int tfMcf5235SessionProcess(ttUserVoidPtr cryptoRequestPtr);
#endif /* TM_USE_MCF5235_CRYPTOENGINE */




#ifdef TM_USE_SW_COMPENGINE
int tfCompSoftwareEngineSessionProcess(ttUserVoidPtr compRequestPtr);
TM_PROTO_EXTERN ttCompEnginePtr tfCompEngineGet2(int compAlg);
#endif /* TM_USE_SW_COMPENGINE */


#ifdef TM_USE_ASYNC_CRYPTO
int tfCryptoAsyncWait(void);
int tfCryptoAsyncSignal(void);
#endif /* TM_USE_ASYNC_CRYPTO */

#if defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2)
/*
 * Expire any outbound SA, or any mature confirmed inbound SA,
 * or mature inbound SAs past the first one.
 * Link the new SA ahead of the previous ones.
 */
TM_PROTO_EXTERN void tfSadbSelectorExpireLink(ttSadbRecordPtr prevSadbPtr,
                                              ttSadbRecordPtr sadbPtr);
/* 
 * User IKE API 
 */
#ifdef TM_USE_IKE_TASK
/* Execute IKE task either in blocking mode or non blocking mode */
TM_PROTO_EXTERN int tfIkeUserExecute(int blockingState);
/* Shutdown IKE */
TM_PROTO_EXTERN void tfIkeTaskShutdown(void);
#endif /* TM_USE_IKE_TASK */

TM_PROTO_EXTERN int tfStartIke(unsigned int      ipaddrType, 
                               unsigned int      ipaddrLength,
                               ttUserVoidPtr     ipaddrPtr
#ifdef TM_USE_STRONG_ESL
                              ,ttUserInterface   interfaceHandle
#endif /* TM_USE_STRONG_ESL */
                               );

TM_PROTO_EXTERN int tfStartEnhancedIke(
    unsigned int              idType, 
    unsigned int              idLength,
    ttUserVoidPtr             idPtr,
    const struct sockaddr_storage TM_FAR * ikeSockaddrPtr
#ifdef TM_USE_STRONG_ESL
   ,ttUserInterface           interfaceHandle
#endif /* TM_USE_STRONG_ESL */
    );

typedef ttUserVoidPtr ttIkeHandle;
typedef ttIkeHandle TM_FAR * ttIkeHandlePtr;

TM_PROTO_EXTERN int tfStartConfigIke(
    const struct sockaddr_storage TM_FAR * ikeSockaddrPtr,
#ifdef TM_USE_STRONG_ESL
    ttUserInterface           interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    ttIkeHandlePtr            ikeHandlePtr
    );

TM_PROTO_EXTERN int tfIkeUninitializeBySource(
    struct sockaddr_storage TM_FAR * myAddrPtr);

TM_PROTO_EXTERN int tfIkeUninitialize(ttIkeHandle ikeHandle);

/* find the IKE socket number using the socket address. For Mobile Node,
 * you have to specify the home address to find the socket.
 */
TM_PROTO_EXTERN int tfIkeGetSocketNumber(
    const struct sockaddr_storage TM_FAR * myAddrPtr
#ifdef TM_USE_STRONG_ESL
    , ttUserInterface interfaceHandle
#endif /* TM_USE_STRONG_ESL */
    );

/* Delete all the phase 1 SAs associated with an interface */
TM_PROTO_EXTERN int tfIkeDeletePhase1SAByInterface(
                    ttUserInterface          interfaceHandle);

TM_PROTO_EXTERN void tfIkeClearSas(ttUser16Bit options);

/*
 * This addition provides a user-level structure for IKE
 * policy definition.  When adding IKE policies, these
 * are translated to internal structures and managed by
 * a list referenced in tvIkeGlobal.
 *
 */

/* IKE policy attributes, ttIkePolicy.attributes */
#define TM_IKE_ROLE_INITIATOR         0x0001
#define TM_IKE_ROLE_RESPONDER         0x0002
#define TM_IKE_MODE_MAIN              0x0004
#define TM_IKE_MODE_AGGRESSIVE        0x0008
#define TM_IKE_PHASE1_PFS             0x0010 /* not supported          */
#define TM_IKE_INITIAL_CONTACT        0x0020
#define TM_IKE_DPD                    0x0040 /* Dead Peer Detection    */
#define TM_IKE_NATT                   0x0080 /* NAT Traversal(RFC3947) */
#define TM_IKE_NATT_DRAFT02           0x0100 /* NAT Traversal(draft02) */
#define TM_IKE_TRECK                  0x0200 /* Treck IKE Vendor ID    */  
#define TM_IKE_ROLE_REMOTE_CLIENT     0x0400 /* IKEv2 IRAC             */
#define TM_IKE_ROLE_REMOTE_SERVER     0x0800 /* IKEv2 IRAS             */

#endif /* defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2) */

#ifdef TM_USE_SEC_STAT
#ifndef TM_SEC_STAT_MAX_IKE_ADDRESSES
#define TM_SEC_STAT_MAX_IKE_ADDRESSES 1
#elif TM_SEC_STAT_MAX_IKE_ADDRESSES > 32767
#error TM_SEC_STAT_MAX_IKE_ADDRESSES too big! Must not be > 32767.
#endif /* TM_SEC_STAT_MAX_IKE_ADDRESSES */

/*
 * The 2 MSBs are flags for initiator, responder, IKE, IPsec, etc.
 * The 2 LSBs represent the error code
 */
#define TM_STAT_INIT                        0x1000
#define TM_STAT_RESP                        0x2000

#define TM_STAT_IKE                         0x4000
#define TM_STAT_IPSEC                       0x8000

#define TM_STAT_IKE_FAIL_INVAL_COOKIE       0x0001
#define TM_STAT_IKE_FAIL_NORESPONSE         0x0002
#define TM_STAT_IKE_FAIL_BADMESSAGE         0x0003
#define TM_STAT_IKE_FAIL_LOGIC              0x0004
#define TM_STAT_IKE_SEND                    0x0005
#define TM_STAT_IKE_RECV                    0x0006
#define TM_STAT_IKE_DPD_RUT_SENT            0x0007
#define TM_STAT_IKE_DPD_RUT_SENT_RETRANS    0x0008
#define TM_STAT_IKE_DPD_RUT_RECV            0x0009
#define TM_STAT_IKE_DPD_ACK_SENT            0x000A
#define TM_STAT_IKE_DPD_ACK_RECV            0x000B
#define TM_STAT_IKE_DPD_RUT_SEQNUM_ERROR    0x000C
#define TM_STAT_IKE_DPD_ACK_SEQNUM_ERROR    0x000D
#define TM_STAT_IKE_DPD_COOKIE_ERROR        0x000E

#define TM_STAT_IPSEC_FAIL_INVAL_COOKIE     0x0081
#define TM_STAT_IPSEC_FAIL_NORESPONSE       0x0082
#define TM_STAT_IPSEC_FAIL_BADMESSAGE       0x0083
#define TM_STAT_IPSEC_FAIL_LOGIC            0x0084

#define TM_STAT_ERROR_MASK                  0x00FF

#define TM_STAT_DECREMENT                   (int)-1
#define TM_STAT_INCREMENT                   (int) 1

#define TM_STAT_FLAG_VALID                  0x0001
#define TM_STAT_FLAG_INVAL                  (ttS16Bit)-1

#define TM_IPSEC_EXPIRE_SOFT_BYTE           (tt8Bit)1
#define TM_IPSEC_EXPIRE_SOFT_TIME           (tt8Bit)2
#define TM_IPSEC_EXPIRE_PEER                (tt8Bit)3


typedef struct tsIsakmpStats
{
    struct sockaddr_storage     iksIpAddress;
/* IKE */
/* Byte and packet counts */
    ttUser64Bit                 iksIkeTotalPacketsIn;
    ttUser64Bit                 iksIkeTotalPacketsOut;
    ttUser64Bit                 iksIkeTotalBytesIn;
    ttUser64Bit                 iksIkeTotalBytesOut;
/* Negotiation statistics */
    ttUser32Bit                 iksIkeCurrentNumOfSas;
    ttUser32Bit                 iksIkeCurrentInitiatedSas;
    ttUser32Bit                 iksIkeCurrentRespondedSas;
    ttUser32Bit                 iksIkeTotalInitiatedSas;
    ttUser32Bit                 iksIkeTotalRespondedSas;
    ttUser32Bit                 iksIkeTotalAttempts;
    ttUser32Bit                 iksIkeTotalAMAttemptsAsInit;
    ttUser32Bit                 iksIkeTotalMMAttemptsAsInit;
    ttUser32Bit                 iksIkeTotalAMAttemptsAsResp;
    ttUser32Bit                 iksIkeTotalMMAttemptsAsResp;
/* Error statistics */
    ttUser32Bit                 iksIkeInitiatorFails;
    ttUser32Bit                 iksIkeResponderFails;
    ttUser32Bit                 iksIkeInvalCookieFails;
    ttUser32Bit                 iksIkeNoResponseFails;
    ttUser32Bit                 iksIkeBadMessageFails;
    ttUser32Bit                 iksIkeLogicFails;
/* IPsec */
/* Negotiation statistics */
    ttUser32Bit                 iksIpsecCurrentNumOfSas;
    ttUser32Bit                 iksIpsecCurrentInitiatedSas;
    ttUser32Bit                 iksIpsecCurrentRespondedSas;
    ttUser32Bit                 iksIpsecTotalInitiatedSas;
    ttUser32Bit                 iksIpsecTotalRespondedSas;
    ttUser32Bit                 iksIpsecTotalAttempts;
    ttUser32Bit                 iksIpsecTotalQMAttemptsAsInit;
    ttUser32Bit                 iksIpsecTotalQMAttemptsAsResp;
/* Error statistics */
    ttUser32Bit                 iksIpsecInitiatorFails;
    ttUser32Bit                 iksIpsecResponderFails;
    ttUser32Bit                 iksIpsecInvalCookieFails;
    ttUser32Bit                 iksIpsecNoResponseFails;
    ttUser32Bit                 iksIpsecBadMessageFails;
    ttUser32Bit                 iksIpsecLogicFails;
} ttIsakmpStats;
typedef ttIsakmpStats TM_FAR * ttIsakmpStatsPtr;

typedef struct tsSadbDetails
{
    struct sockaddr_storage     sadbDetLocalIp;
    struct sockaddr_storage     sadbDetRemoteIp;
    ttUser64Bit                 sadbDetInLifetimeHardByteLimit;
    ttUser64Bit                 sadbDetOutLifetimeHardByteLimit;
    ttUser64Bit                 sadbDetInLifetimeSoftByteLimit;
    ttUser64Bit                 sadbDetOutLifetimeSoftByteLimit;
    ttUser64Bit                 sadbDetInLifetimeBytesProcessed;
    ttUser64Bit                 sadbDetOutLifetimeBytesProcessed;
#ifdef TM_USE_VIPER
    ttUser64Bit                 sadbDetInLifetimeByteCount;
    ttUser64Bit                 sadbDetInUserByteCount;
    ttUser64Bit                 sadbDetInPacketCount;
    ttUser64Bit                 sadbDetInCompressedByteCount;
    ttUser64Bit                 sadbDetInCompressedPacketCount;
    ttUser32Bit                 sadbDetInAuthErrCount;
    ttUser32Bit                 sadbDetInPadErrCount;
    ttUser32Bit                 sadbDetInReplayErrCount;
    ttUser64Bit                 sadbDetOutLifetimeByteCount;
    ttUser64Bit                 sadbDetOutUserByteCount;
    ttUser64Bit                 sadbDetOutPacketCount;
    ttUser64Bit                 sadbDetOutCompressedByteCount;
    ttUser64Bit                 sadbDetOutCompressedPacketCount;
#endif /* TM_USE_VIPER */
    ttUser32Bit                 sadbDetInLifetimeHardTimeLimit;
    ttUser32Bit                 sadbDetOutLifetimeHardTimeLimit;
    ttUser32Bit                 sadbDetInLifetimeSoftTimeLimit;
    ttUser32Bit                 sadbDetOutLifetimeSoftTimeLimit;
    ttUser32Bit                 sadbDetInLifetimeTimeRemain;
    ttUser32Bit                 sadbDetOutLifetimeTimeRemain;
    ttUser32Bit                 sadbDetInSpi;
    ttUser32Bit                 sadbDetOutSpi;
    ttUser32Bit                 sadbDetTotalRekeys;
    ttUser32Bit                 sadbDetByteRekeys;
    ttUser32Bit                 sadbDetTimeRekeys;
    ttUser32Bit                 sadbDetPeerRekeys;
    ttUser32Bit                 sadbDetOtherRekeys;
    ttUser32Bit                 sadbDetEncKeyLen;
    int                         sadbDetEncAlg;
    int                         sadbDetNumOfSas;
    ttUser16Bit                 sadbDetAuthAlg;
    ttUser8Bit                  sadbDetMode;
    ttUser8Bit                  sadbDetLocalProtocol;
    ttUser8Bit                  sadbDetRemoteProtocol;
    ttUser8Bit                  sadbDetKind;
    ttUser8Bit                  sadbDetDirection;
#ifdef TM_USE_IPSEC_IPCOMP
    ttUser8Bit                  sadbDetCompressAlg;
#endif /* TM_USE_IPSEC_IPCOMP */
} ttSadbDetails;
typedef ttSadbDetails TM_FAR * ttSadbDetailsPtr;

typedef struct tsSadbTotals
{
    ttUser96Bit                 sadbInUserByteCount;
    ttUser96Bit                 sadbInPacketCount;
    ttUser96Bit                 sadbInCompressedByteCount;
    ttUser96Bit                 sadbInCompressedPacketCount;
    ttUser96Bit                 sadbOutUserByteCount;
    ttUser96Bit                 sadbOutPacketCount;
    ttUser96Bit                 sadbOutCompressedByteCount;
    ttUser96Bit                 sadbOutCompressedPacketCount;
    ttUser64Bit                 sadbInAuthErrCount;
    ttUser64Bit                 sadbInPadErrCount;
    ttUser64Bit                 sadbInReplayErrCount;
} ttSadbTotals;
typedef ttSadbTotals TM_FAR * ttSadbTotalsPtr;

typedef struct tsSadbCount
{
    ttUser64Bit                 sadbTotalInEspSas;
    ttUser64Bit                 sadbTotalInAhSas;
    ttUser64Bit                 sadbTotalOutEspSas;
    ttUser64Bit                 sadbTotalOutAhSas;
#ifdef TM_USE_IPSEC_IPCOMP
    ttUser64Bit                 sadbTotalInIpcompSas;
    ttUser64Bit                 sadbTotalOutIpcompSas;
    ttUser32Bit                 sadbCurrentInIpcompSas;
    ttUser32Bit                 sadbCurrentOutIpcompSas;
#endif /* TM_USE_IPSEC_IPCOMP */
    ttUser32Bit                 sadbCurrentInEspSas;
    ttUser32Bit                 sadbCurrentInAhSas;
    ttUser32Bit                 sadbCurrentOutEspSas;
    ttUser32Bit                 sadbCurrentOutAhSas;
    
} ttSadbCount;
typedef ttSadbCount TM_FAR * ttSadbCountPtr;

typedef struct tsIsaStateEntryDetails
{
    struct sockaddr_storage     iedLocalIp;
    struct sockaddr_storage     iedRemoteIp;
    ttUser64Bit                 iedInPackets;
    ttUser64Bit                 iedOutPackets;
    ttUser64Bit                 iedInBytes;
    ttUser64Bit                 iedOutBytes;
    ttUser32Bit                 iedLifetimeBytesHard;
    ttUser32Bit                 iedLifetimeBytesSoft;
    ttUser32Bit                 iedLifetimeBytesProcessed;
    ttUser32Bit                 iedLifetimeSecondsHard;
    ttUser32Bit                 iedLifetimeSecondsSoft;
    ttUser32Bit                 iedLifetimeSecondsRemain;
    ttUser32Bit                 iedDoi;
    ttUser16Bit                 iedEncryptAlg;
    ttUser16Bit                 iedEncryptKeyLen;
    ttUser16Bit                 iedHashAlg;
    ttUser8Bit                  iedIcookie[TM_ISAKMP_COOKIE_SIZE];
    ttUser8Bit                  iedRcookie[TM_ISAKMP_COOKIE_SIZE];
    ttUser8Bit                  iedState;
    ttUser8Bit                  iedIsInitiator;
    ttUser8Bit                  iedPeerVersion;
    ttUser8Bit                  iedExchange;
} ttIsaStateEntryDetails;
typedef ttIsaStateEntryDetails TM_FAR * ttIsaStateEntryDetailsPtr;

#ifdef TM_USE_IKE_DPD
typedef struct tsDpdStatistics
{
    struct sockaddr_storage     dpdIsakmpIpAddress;
    ttUser32Bit                 dpdRutSentTotal;
    ttUser32Bit                 dpdRutRecvTotal;
    ttUser32Bit                 dpdRutSentRetrans;
    ttUser32Bit                 dpdAcksSentTotal;
    ttUser32Bit                 dpdAcksRecvTotal;
    ttUser32Bit                 dpdSeqNoErrTotal;
    ttUser32Bit                 dpdAcksSeqNoErrTotal;
    ttUser32Bit                 dpdCookieErrTotal;
} ttDpdStatistics;
typedef ttDpdStatistics TM_FAR * ttDpdStatisticsPtr;

TM_PROTO_EXTERN int tfGetDpdTotals(
                             ttUser8Bit               unit,
                             ttDpdStatisticsPtr       dpdStatisticsPtr);
#endif /* TM_USE_IKE_DPD */

TM_PROTO_EXTERN int tfGetIsaStateEntryDetails(
                             ttUser8Bit                unit,
                             ttUser16Bit               priority,
                             ttUser32Bit               saNumber,
                             ttIsaStateEntryDetailsPtr isaStateEntryDetailsPtr);

TM_PROTO_EXTERN int tfSadbRecordGetByPolicy(ttUser32Bit         options,
                                            ttUser16Bit         priority,
                                            ttUser8Bit          direction,
                                            ttUserVoidPtrPtr    iteratorPtr,
                                            ttSadbDetailsPtr    sadbDetailsPtr);

TM_PROTO_EXTERN int tfGetIsakmpStats(ttUser8Bit             unit,
                                     ttIsakmpStatsPtr       isakmpStatsPtr);

TM_PROTO_EXTERN int tfGetSadbDetails(ttUser8Bit               unit,
                                     ttUser16Bit              priority,
                                     ttUser32Bit              saNumber,
                                     ttSadbRecordPtr          sadbRecordPtr,
                                     ttSadbDetailsPtr         sadbDetailsPtr);

TM_PROTO_EXTERN int tfGetSadbTotals(ttUser8Bit              unit,
                                    ttSadbTotalsPtr         sadbTotalsPtr);

TM_PROTO_EXTERN int tfGetSadbCount(ttUser8Bit               unit,
                                   ttSadbCountPtr           sadbCountPtr);

TM_PROTO_EXTERN int tfGetIkeAddrIndex(
                      const struct sockaddr_storage TM_FAR * sockAddrPtr);

TM_PROTO_EXTERN int tfRegisterIkeAddr(
                      const struct sockaddr_storage TM_FAR * sockAddrPtr);

TM_PROTO_EXTERN int tfUnregisterIkeAddr(
                      const struct sockaddr_storage TM_FAR * sockAddrPtr);

TM_PROTO_EXTERN void tfStatisticsUpdateCount(
                             int             index,
                             ttUser16Bit     counter,
                             int             addend);

#endif /* TM_USE_SEC_STAT */

TM_PROTO_EXTERN
int tfIkePolicyAdd(ttIkePolicyInfoPtr     ikePlcyInfoPtr,
                   ttPadbInfoPtr          padbInfoPtr,
                   ttIsakmpTransInfoPtr   transInfoListPtr,
                   ttUser8Bit             transCount,
                   ttIkePolicyHandlePtr   ikePolicyHandlePtr);
                   
TM_PROTO_EXTERN int tfIkePolicyPadAdd(ttIkePolicyHandle ikePolicyHandle,
                                      ttPadbInfoPtr     padbInfoPtr);


TM_PROTO_EXTERN int tfIkePolicyDelete(ttIkePolicyHandle ikePolicyHandle);

TM_PROTO_EXTERN int tfIkePolicyClear(void);

/*
 *  IPsec Policy Database Management API 
 */
TM_PROTO_EXTERN int tfPolicyRestore(ttIpsecPolicyPairPtr        pairListPtr, 
                                    ttIpsecSelectorInStringPtr  sListPtr,
                                    ttPolicyContentInStringPtr  cListPtr,
                                    int                         npair);

TM_PROTO_EXTERN ttPolicyEntryPtr tfPolicyAdd(
                                ttIpsecSelectorPtr  selectorPtr,
                                ttPolicyContentPtr  contentPtr,
                                ttUser8Bit          direction,
                                ttIkePolicyHandle   ikePolicyHandle,
                                ttUser16Bit         priority,
                                ttUser8Bit          manual,
                                int   TM_FAR      * errorPtr);

#ifdef TM_IPSEC_POLICY_USER_DATA
TM_PROTO_EXTERN ttPolicyEntryPtr tfPolicyAddWithUserData(
                                             ttIpsecSelectorPtr  selectorPtr,
                                             ttPolicyContentPtr  contentPtr,
                                             ttUser8Bit          direction,
                                             ttUser16Bit         ikePolicy,
                                             ttUser16Bit         priority,
                                             ttUser8Bit          manual,
                                             int   TM_FAR      * errorPtr,
                                             ttUserVoidPtr       userDataPtr,
                                             ttUser32Bit         userDataLen);
#endif /* TM_IPSEC_POLICY_USER_DATA */
TM_PROTO_EXTERN int tfPolicyAddBundle(ttPolicyEntryPtr    plcyPtr,
                                      ttPolicyContentPtr  outerContentPtr);

TM_PROTO_EXTERN int tfPolicyValidate(ttPolicyEntryPtr  plcyPtr);

TM_PROTO_EXTERN 
int tfIpsecPolicyQueryBySelector(ttUser32Bit         options,
                                 ttIpsecSelectorPtr  keySelectorPtr, 
                                 ttPolicyEntryPtrPtr plcyPtrPtr,
                                 ttUser8Bit          direction);

/* get the applied policy, call with IPSEC not already locked */
TM_PROTO_EXTERN int tfIpsecPolicyQueryByIndex(int                 index,
                                              ttPolicyEntryPtrPtr plcyPtrPtr);

TM_PROTO_EXTERN int tfPolicyDeleteByPriority(ttUser16Bit priority,
                                             ttUser8Bit  direction);

TM_PROTO_EXTERN int tfPolicyDelete(ttPolicyEntryPtr   plcyPtr,
                                   ttUser8Bit         direction);

TM_PROTO_EXTERN void tfPolicyClear(void);

/* 
 * SA Database Management API 
 */
int tfSadbRecordManualAddByPriority(
                    ttUser16Bit                priority,
                    ttUser32Bit                inboundIndex,
                    ttUser32Bit                outboundIndex,
                    const char TM_FAR *        inboundKeyPtr,
                    ttUser32Bit                inboundKeyLength,
                    const char TM_FAR *        outboundKeyPtr,
                    ttUser32Bit                outboundKeyLength,
                    ttUser8Bit                 innermostSa);

TM_PROTO_EXTERN int tfSadbRecordGenerate(ttSadbRecordPtrPtr   saPtrPtr,
#ifdef TM_USE_STRONG_ESL
                                         ttUserInterface      interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                         ttChildSaInfoPtr     saInfoPtr,
                                         ttUser8BitPtr        keyMatPtr,
                                         ttUser16Bit          keyMatLen,
                                         ttUser8Bit           direction);

TM_PROTO_EXTERN int tfSadbRecordGet(ttUser32Bit          options,
                                    ttSaIdentityPtr      saIdPtr, 
                                    ttSadbRecordPtrPtr   sadbPtrPtr);

TM_PROTO_EXTERN int tfSadbRecordFind(ttUser32Bit             options,
                                     ttIpsecSelectorPtr      pktSelectorPtr, 
                                     ttSadbRecordPtrPtr      sadbRecordPtrPtr,
                                     ttPolicyEntryPtr        plcyPtr,
                                     ttPolicyContentPtr      plcyContentPtr,
                                     ttUser8Bit              direction);


TM_PROTO_EXTERN int tfSadbRecordDelete(ttUser32Bit      options,
                                       ttSadbRecordPtr  sadbPtr
#if defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2)
                                       ,ttUser8Bit      sendNotifications
#endif /* defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2) */
                                       );

TM_PROTO_EXTERN int tfSadbRecordDeleteByDestination(
    ttUser32Bit                      options,
    struct sockaddr_storage TM_FAR * destination
#if defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2)
    ,ttUser8Bit                      sendNotifications
#endif /* defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2) */
                                     );

TM_PROTO_EXTERN int tfSadbRecordDeleteByPolicy(
    ttUser32Bit            options,
    ttPolicyEntryPtr       plcyPtr,
    ttUser8Bit             direction);

TM_PROTO_EXTERN void tfSadbRecordClear(ttUser32Bit options);

TM_PROTO_EXTERN int tfSadbRecordClearAll(ttUser16Bit options);

TM_PROTO_EXTERN int tfSadbRecordClearByPriority(ttUser16Bit priority);

TM_PROTO_EXTERN void tfSadbClearSocketCache(ttSadbRecordPtr   sadbPtr);

TM_PROTO_EXTERN void tfSadbRecordSetOptions(ttSadbRecordPtr  sadbPtr, 
                                            ttUser16Bit      optionType,
                                            ttUser8Bit       optionValue);

TM_PROTO_EXTERN int tfSadbRecordCKGenerate(ttSadbRecordPtrPtr   saPtrPtr,
#ifdef TM_USE_STRONG_ESL
                                           ttUserInterface      interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                           ttChildSaInfoPtr     saInfoPtr,
                                           ttUser8BitPtr        keyMatPtr,
                                           ttUser16Bit          keyMatLen,
                                           ttUser8Bit           direction);

TM_PROTO_EXTERN int tfSadbRecordCKGenerateBundle(
                                           ttSadbRecordPtrPtr   innerSaPtrPtr,
#ifdef TM_USE_STRONG_ESL
                                           ttUserInterface      interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                           ttChildSaInfoListPtr saInfoPtr,
                                           ttUser8BitPtr        keyMatPtr[],
                                           ttUser16Bit          keyMatLen[],
                                           ttUser8Bit           direction);

TM_PROTO_EXTERN int tfSadbRecordCKGet(ttUser32Bit          options,
                                      ttSaIdentityPtr      saIdPtr, 
                                      ttSadbRecordPtrPtr   sadbPtrPtr);

TM_PROTO_EXTERN int tfSadbRecordCKFind(ttUser32Bit             options,
                                       ttIpsecSelectorPtr      pktSelectorPtr, 
                                       ttSadbRecordPtrPtr      sadbRecordPtrPtr,
                                       ttPolicyEntryPtr        plcyPtr,
                                       ttPolicyContentPtr      plcyContentPtr,
                                       ttUser8Bit              direction);

/* 
 * Unget an sadb record pointer, obtained via tfSadbRecordCKManualAdd(),
 * tfSadbRecordCKGenerate(), tfSadbRecordCKFind(), or tfSadbRecordCKGet(),
 * after the caller no longer needs to access the sadb Record Pointer.
 * Failure to do so, will result in memory leak.
 */
TM_PROTO_EXTERN void tfSadbRecordUnget(ttUser32Bit     options,
                                       ttSadbRecordPtr sadbRecordPtr);

#ifdef TM_USE_IPSEC_DEBUG_TASK
TM_PROTO_EXTERN void tfSadbDumpSaTable(ttUser32Bit options);
#endif /* TM_USE_IPSEC_DEBUG_TASK */

#ifdef TM_USE_PKI
TM_PROTO_EXTERN int tfUsePki(void);
TM_PROTO_EXTERN void tfPkiUninitialize(void);
TM_PROTO_EXTERN int tfPkiCertificateAdd (   ttUserConstVoidPtr  fileNamePtr, 
                                            int                 fileFormat,
                                            int                 certType,
                                            ttUserConstVoidPtr  idPtr,
                                            int                 idLength,
                                            ttUserConstVoidPtr  caIdPtr );

TM_PROTO_EXTERN int tfPkiOwnKeyPairAdd  (   ttUserConstVoidPtr  fileNamePtr,
                                            int                 fileFormat,
                                            int                 keyType);

TM_PROTO_EXTERN int tfPkiOwnKeyPairDelete ( void);

TM_PROTO_EXTERN int tfPkiOwnCertificateAdd(int              typeFormat,
                                           ttUserVoidPtr    keyPairDataPtr,
                                           int              keyType,
                                           ttUserVoidPtr    certDataPtr,
                                           ttUserVoidPtr    caIdPtr,
                                           ttCertHandlePtr  certHandlePtr,
                                           ttUserVoidPtr    idPtr,
                                           int              idLength,
                                           int             *certFlagPtr);

TM_PROTO_EXTERN int tfPkiCertificateDelete (
                                            ttUserConstVoidPtr  idPtr, 
                                            int                 idLength,
                                            int                 certType);

TM_PROTO_EXTERN int tfPkiCertificateDeleteByHandle(ttCertHandle certHandle,
                                                   int          certType);

TM_PROTO_EXTERN int tfPkiCertificateClear ( int        certType);

#endif /* TM_USE_PKI */

/* Use Log */
TM_PROTO_EXTERN int tfUseIpsecLogging (ttUser16Bit      numLogMsgs);
TM_PROTO_EXTERN int tfIpsecLogWalk(ttLogWalkFuncPtr     funcPtr,
                                   int                  msgSeqNo,
                                   ttUserGenericUnion   genParam); 
#if defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)
TM_PROTO_EXTERN int tfIpsecInitializeFromFile(
    ttUserInterface   interfaceHandle,
    char TM_FAR     * fileName, 
    int TM_FAR      * lineNumberPtr);
#endif /* TM_KERNEL_VISUAL_X86 || TM_KERNEL_WIN32_X86 */

/*
 *  IKEv2 EAP User account Management API 
 */
#ifdef TM_USE_IKEV2
#ifdef TM_USE_IKEV2_EAP_AUTH
#if (defined(TM_USE_IKEV2_EAP_MD5) || defined(TM_USE_IKEV2_EAP_MSCHAPV2))

typedef void                 TM_FAR * ttIkev2EapUserHandle;
typedef ttIkev2EapUserHandle TM_FAR * ttIkev2EapUserHandlePtr;

typedef struct tsIkev2EapUserData
{
    ttUserConstCharPtr      userNamePtr;
    ttUserConstCharPtr      passWordPtr;
}ttIkev2EapUserData;
typedef ttIkev2EapUserData TM_FAR * ttIkev2EapUserDataPtr;

TM_PROTO_EXTERN int tfIkev2EapAddUser(
    ttUserConstCharPtr      userNamePtr, 
    ttUserConstCharPtr      passWordPtr,
    ttIkev2EapUserHandlePtr eapUserHandlePtr);

TM_PROTO_EXTERN int tfIkev2EapDeleteHandle(ttIkev2EapUserHandle eapUserHandle);

TM_PROTO_EXTERN int tfIkev2EapDeleteAllUser(void);

TM_PROTO_EXTERN int tfIkev2EapAddUserSameHandle(
    ttUserConstCharPtr       userNamePtr,
    ttUserConstCharPtr       passWordPtr,
    ttIkev2EapUserHandle     eapUserHandle);
#endif /* ((TM_USE_IKEV2_EAP_MD5) || (TM_USE_IKEV2_EAP_MSCHAPV2)) */
#endif /* TM_USE_IKEV2_EAP_AUTH */
#endif /* TM_USE_IKEV2 */

/*
 * SSL public apis
 */
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))

#define TM_SSL_PROPOSAL_NUMBER                   6

#define TM_SSL_OPTNAM_CLIENTAUTH                 1
#define TM_SSL_OPTNAM_SERVERPROP                 2
#define TM_SSL_OPTNAM_NO_SEND_EMPTY_FRAG         3
#define TM_SSL_OPTNAM_CLIENT_NOCERT              4
#define TM_SSL_OPTNAM_CLIENT_CACHEID             5

#define TM_SSL_OPT_CLIENTAUTH               0x0001
#define TM_SSL_OPT_SERVERPROP               0x0002
#define TM_SSL_OPT_NO_SEND_EMPTY_FRAG       0x0004
#define TM_SSL_OPT_CLIENT_NOCERT            0x0008
#define TM_SSL_OPT_CLIENT_CACHEID           0x0010

#define TM_SSL_VERSION_30              0x01
#define TM_SSL_VERSION_31              0x02 /* i.e. TLS v1.0*/
#define TM_SSL_VERSION_32              0x04 /* i.e. TLS v1.1*/
#define TM_SSL_VERSION_33              0x08 /* i.e. TLS v1.2*/

#define TM_TLS_RSA_NULL_MD5                0x0001
#define TM_TLS_RSA_NULL_SHA                0x0002
#define TM_TLS_RSA_NULL_SHA256             0x003B
#define TM_TLS_RSA_EXPORT_RC4_40_MD5       0x0003
#define TM_TLS_RSA_RC4_128_MD5             0x0004
#define TM_TLS_RSA_RC4_128_SHA             0x0005
#define TM_TLS_RSA_EXPORT_RC2_40_MD5       0x0006
#define TM_TLS_RSA_IDEA_SHA                0x0007
#define TM_TLS_RSA_EXPORT_DES40_CBC_SHA    0x0008
#define TM_TLS_RSA_DES_CBC_SHA             0x0009
#define TM_TLS_RSA_3DES_EDE_CBC_SHA        0x000A
#define TM_TLS_DH_DSS_EPT_DES40_CBC_SHA    0x000B
#define TM_TLS_DH_DSS_DES_CBC_SHA          0x000C
#define TM_TLS_DH_DSS_3DES_EDE_CBC_SHA     0x000D
#define TM_TLS_DH_RSA_EPT_DES40_CBC_SHA    0x000E
#define TM_TLS_DH_RSA_DES_CBC_SHA          0x000F
#define TM_TLS_DH_RSA_3DES_EDE_CBC_SHA     0x0010
#define TM_TLS_DHE_DSS_EPT_DES40_CBC_SHA   0x0011
#define TM_TLS_DHE_DSS_DES_CBC_SHA         0x0012
#define TM_TLS_DHE_DSS_3DES_EDE_CBC_SHA    0x0013
#define TM_TLS_DHE_RSA_EPT_DES40_CBC_SHA   0x0014
#define TM_TLS_DHE_RSA_DES_CBC_SHA         0x0015
#define TM_TLS_DHE_RSA_3DES_EDE_CBC_SHA    0x0016

/* new ciphers for AES */
#define TM_TLS_RSA_AES_128_CBC_SHA         0x002F
#define TM_TLS_DH_DSS_AES_128_CBC_SHA      0x0030
#define TM_TLS_DH_RSA_AES_128_CBC_SHA      0x0031
#define TM_TLS_DHE_DSS_AES_128_CBC_SHA     0x0032
#define TM_TLS_DHE_RSA_AES_128_CBC_SHA     0x0033
#define TM_TLS_RSA_AES_256_CBC_SHA         0x0035
#define TM_TLS_DH_DSS_AES_256_CBC_SHA      0x0036
#define TM_TLS_DH_RSA_AES_256_CBC_SHA      0x0037
#define TM_TLS_DHE_DSS_AES_256_CBC_SHA     0x0038
#define TM_TLS_DHE_RSA_AES_256_CBC_SHA     0x0039
#define TM_TLS_RSA_AES_128_CBC_SHA256      0x003C
#define TM_TLS_RSA_AES_256_CBC_SHA256      0x003D
#define TM_TLS_DHE_DSS_AES_128_CBC_SHA256  0x0040


/* The following macros are defined in draft 
 * draft-ietf-tls-56-bit-ciphersuites-01.txt
 * EPT1K means EXPORT1024
 */
#define TM_TLS_RSA_EPT1K_DES_CBC_SHA       0x0062
#define TM_TLS_DHE_DSS_EPT1K_DES_CBC_SHA   0x0063
#define TM_TLS_RSA_EPT1K_RC4_56_SHA        0x0064
#define TM_TLS_DHE_DSS_EPT1K_RC4_56_SHA    0x0065
#define TM_TLS_DHE_DSS_RC4_128_SHA         0x0066
#define TM_TLS_DHE_RSA_AES_128_CBC_SHA256  0x0067
#define TM_TLS_DHE_DSS_AES_256_CBC_SHA256  0x006A
#define TM_TLS_DHE_RSA_AES_256_CBC_SHA256  0x006B

typedef int (TM_CODE_FAR *ttSslCertCallbackFuncPtr)(
             int                   sessionId,
             int                   sockDescriptor,
             ttCertificateEntryPtr entryArray,
             int                   arraySize);

TM_PROTO_EXTERN int  tfUseSsl(int maximumSession);
TM_PROTO_EXTERN void tfSslUninitialize(void);
TM_PROTO_EXTERN int  tfSslSetSessionOptions(int              sessionId,
                                            int              optionName,
                                            int              optionValue);
TM_PROTO_EXTERN int tfSslSetSessionProposals(int           sessionId,
                                             int TM_FAR *  propArray,
                                             int           propSize);
TM_PROTO_EXTERN int tfSslUserSendFlush(int     socketDescriptor);
TM_PROTO_EXTERN int tfSslConnectUserClose(int  socketDescriptor);
TM_PROTO_EXTERN int tfSslSessionUserClose(int            sessionNumber);
TM_PROTO_EXTERN int tfSslNewSession(  
                            ttUserConstVoidPtr           certIdentity,
                            unsigned int                 maxCaches,
                            int                          version,
                            unsigned int                 option);

TM_PROTO_EXTERN int tfSslRegisterCertificateCB(
                       int                        sessionId, 
                       ttSslCertCallbackFuncPtr   certCBFuncPtr);

#ifdef TM_USE_SSL_CLIENT
TM_PROTO_EXTERN int tfSslClientUserStart(int           socketDescriptor,
                                         char TM_FAR * serverCommonName);
#endif /* TM_USE_SSL_CLIENT */

#ifdef TM_USE_SSL_SERVER
#ifdef TM_SSL_USE_MUTUAL_AUTH
TM_PROTO_EXTERN int tfSslServerAddClientAuthCa(int              sessionIndex, 
                                               char TM_FAR    * caIdPtr);
TM_PROTO_EXTERN int tfSslServerRemoveClientAuthCa(int           sessionIndex,
                                                  char TM_FAR * caIdPtr);
#endif /* TM_SSL_USE_MUTUAL_AUTH */

#ifdef TM_PUBKEY_USE_RSA
TM_PROTO_EXTERN int tfSslRemoveEphemeralRsaKeys(int           sessionNumber);
TM_PROTO_EXTERN int tfSslAddEphemeralRsaKeys(int              sessionNumber, 
                                             ttUserVoidPtr    fileNamePtr,
                                             int              typeFormat);
#endif /* TM_PUBKEY_USE_RSA */
TM_PROTO_EXTERN int tfSslServerUserCreate(int  socketDescriptor);
TM_PROTO_EXTERN int tfSslServerUserHelloRequest(int      socketDescriptor,
                                                int      option);

#endif /* TM_USE_SSL_SERVER */
TM_PROTO_EXTERN int tfSslUserProcessHandshake(int socketDescriptor);
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TRSECAPI_H_ */
