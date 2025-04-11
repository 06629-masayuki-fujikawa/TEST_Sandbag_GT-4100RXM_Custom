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
 * Description: PPP Link Layer (for async implementations)
 *
 * Filename: trppp.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trppp.c $
 *
 * Modification History
 * $Revision: 6.0.2.9 $
 * $Date: 2012/11/28 04:26:57JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#ifdef TM_USE_PPP

/*
 *
 * Macros
 *
 */

/* Define unreferenced macro for PC-LINT compile error suppression */
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_PPP_LCP_HOST_PROTOCOL)
LINT_UNREF_MACRO(TM_PPP_IPCP_HOST_PROTOCOL)
LINT_UNREF_MACRO(TM_PPP_COMP_TCP_HOST_PROTOCOL)
LINT_UNREF_MACRO(TM_PPP_UNCOMP_TCP_HOST_PROTOCOL)
LINT_UNREF_MACRO(TM_PPP_IP_HOST_PROTOCOL)
LINT_UNREF_MACRO(TM_PPP_IDENT)
LINT_UNREF_MACRO(TM_PPP_TIME_REMAIN)
LINT_UNREF_MACRO(TM_PPP_RESET_REQUEST)
LINT_UNREF_MACRO(TM_PPP_RESET_REPLY)
LINT_UNREF_MACRO(TM_CHAP_MS)
LINT_UNREF_MACRO(TM_PPP_IPCP_MAX_SIZE)
LINT_UNREF_MACRO(TM_PPPS_RESET_SENT)
LINT_UNREF_MACRO(TM_PPPS_RESET_ACK)
LINT_UNREF_MACRO(TM_PPPS_INVALID)
LINT_UNREF_MACRO(TM_PPPE_DOWN)
LINT_UNREF_MACRO(devPtr)
LINT_UNREF_MACRO(TM_PPPE_CODE_PRO_REJ)
LINT_UNREF_MACRO(TM_PPPE_CODE_PRO_ERROR)
LINT_UNREF_MACRO(TM_PPP_VJ_COMP_SLOT_ID)
LINT_UNREF_MACRO(TM_PPP_STATE_FUNC_NULL_PTR)
LINT_UNREF_MACRO(TM_UNCOMPRESSED_TCP)
LINT_UNREF_MACRO(TM_PPP_LQR_HOST_PROTOCOL)
LINT_UNREF_MACRO(TM_PPP_COMP_TCP_PROTOCOL_BYTE)
LINT_UNREF_MACRO(TM_VJ_SWU)
LINT_UNREF_MACRO(TM_VJ_SA)
LINT_UNREF_MACRO(TM_LQM_VECT_NULL_PTR)
LINT_UNREF_MACRO(TM_PPP_LCP_MAX_SIZE)
LINT_UNREF_MACRO(TM_PPP_PAP_MAX_SIZE)
LINT_UNREF_MACRO(TM_PPP_CHAP_MAX_SIZE)
#endif /* TM_LINT */

/* Protocol Field Numbers */
#define TM_PPP_LCP_HOST_PROTOCOL             0xc021
#define TM_PPP_IPCP_HOST_PROTOCOL            0x8021
#define TM_PPP_PAP_HOST_PROTOCOL             0xc023
#define TM_PPP_CHAP_HOST_PROTOCOL            0xc223
#define TM_PPP_EAP_HOST_PROTOCOL             0xc227
#define TM_PPP_COMP_TCP_HOST_PROTOCOL        0x002d
#define TM_PPP_UNCOMP_TCP_HOST_PROTOCOL      0x002f
#define TM_PPP_IP_HOST_PROTOCOL              0x0021
#define TM_PPP_LQR_HOST_PROTOCOL             0xc025
#define TM_PPP_CBCP_HOST_PROTOCOL            0xc029

/* LCP & IPCP Codes */
#define TM_PPP_CONFIG_REQUEST   1
#define TM_PPP_CONFIG_ACK       2
#define TM_PPP_CONFIG_NAK       3
#define TM_PPP_CONFIG_REJECT    4
#define TM_PPP_TERM_REQUEST     5
#define TM_PPP_TERM_ACK         6
#define TM_PPP_CODE_REJECT      7
#define TM_PPP_PROTO_REJECT     8
#define TM_PPP_ECHO_REQUEST     9
#define TM_PPP_ECHO_REPLY      10
#define TM_PPP_DISCARD_REQUEST 11
#define TM_PPP_IDENT           12
#define TM_PPP_TIME_REMAIN     13
#define TM_PPP_RESET_REQUEST   14
#define TM_PPP_RESET_REPLY     15

/* PPP Defaults (per RFC1661) */
#define TM_PPP_TERM_MAX_RETRY     2
#define TM_PPP_CFG_MAX_RETRY      10
#define TM_PPP_RETRY_TIMEOUT      3
/* maximum failures for PAP or CHAP
 * currently,we don't offer retry. Once PAP/CHAP fails,
 * we let the peer know and tear down the connection
 *
 */
#define TM_PPP_AUTH_MAX_FAILURES  3
#define TM_PPP_MAX_FAILURES       5
#define TM_LCP_MAGIC_NUM_FAILURES 3


/* Frame Check Sequence Macros */
#define TM_PPP_FCS_INITIAL  0xffff
#define TM_PPP_FCS_FINAL    0xf0b8

/* PAP Codes */
#define TM_PAP_AUTH_REQUEST     1
#define TM_PAP_AUTH_ACK         2
#define TM_PAP_AUTH_NAK         3
#define TM_PAP_AUTH_REJECT      4

/* CHAP Codes */
#define TM_CHAP_CHALLENGE       1
#define TM_CHAP_RESPONSE        2
#define TM_CHAP_SUCCESS         3
#define TM_CHAP_FAILURE         4
#define TM_CHAP_CHANGEPASSWORD  6


/* CHAP Defaults */
#define TM_CHAP_VALUE_SIZE           48 /* Must be divisible by 4 */

#ifdef TM_USE_PPP_MSCHAP
#define TM_CHAP_MSV1_CHALLENGE_SIZE  8 /* Must be divisible by 4 */
#endif /* TM_USE_PPP_MSCHAP */

#define TM_PAK_CHAP_VALUE_SIZE  tm_packed_byte_count(TM_CHAP_VALUE_SIZE)

#ifdef TM_USE_PPP_CALLBACK
/* Callback Control Protocol codes. */
#define TM_CBCP_REQUEST         1
#define TM_CBCP_RESPONSE        2
#define TM_CBCP_ACK             3

/* Default Callback values (time values in seconds). */
#define TM_CBCP_MAX_RETRY       10      /* Number of send attempts. */
#define TM_CBCP_RETRY_TIMEOUT   3       /* Time between sends attempts. */
#define TM_CBCP_CALL_DELAY      10      /* Delay before calling back. */
#define TM_CBCP_DISC_DELAY      2       /* Delay before disconnect. */
#endif /* TM_USE_PPP_CALLBACK */

/* PPP ACCM Mapping */
#define TM_PPP_FLAG_CHAR    (tt8Bit)0x7E
#define TM_PPP_ESC_CHAR     (tt8Bit)0x7D
#define TM_PPP_STUFF_CHAR   (tt8Bit)0x20

/* Maximum size of NCP packets */
#define TM_PPP_LCP_MAX_SIZE  tm_packed_byte_count(256)
#define TM_PPP_IPCP_MAX_SIZE tm_packed_byte_count(256)
#define TM_PPP_PAP_MAX_SIZE  tm_packed_byte_count(256)
#define TM_PPP_CHAP_MAX_SIZE tm_packed_byte_count(256)

#define TM_PPP_LCP_MAX_BYTES   256
#define TM_PPP_IPCP_MAX_BYTES  256
#define TM_PPP_IP6CP_MAX_BYTES 256
#define TM_PPP_PAP_MAX_BYTES   256
#define TM_PPP_CHAP_MAX_BYTES  256

#define TM_IPCP_OPT_DNS_PRI (TM_IPCP_DNS_PRI + 100)
#define TM_IPCP_OPT_DNS_SEC (TM_IPCP_DNS_SEC + 100)

#define TM_PPP_VECT_NULL_PTR (ttPppVectPtr)0
#define TM_LQM_VECT_NULL_PTR (ttLqmVectPtr)0

#ifdef TM_USE_PPP_MSCHAP
#define TM_PPP_MSCHAP_MAX_PASSWDFAILURE          3
#define TM_PPP_MSCHAP_ERROR_PASSWDEXPIRE       648
#define TM_PPP_MSCHAP_ERROR_AUTHFAILURE        691
#endif /* TM_USE_PPP_MSCHAP */

#define tm_8bit_cur_id(curId, nextId) \
    (curId == (tt8Bit)(0xFF & ((0xFF & nextId) - 1)))

#define tm_ppp_need_stuff(inChar,accm) \
 ( ( (tt8Bit)inChar < 32 ) ?   ( (tlPppAccmMap[(int)inChar] & accm)? 1 : 0 ) \
                             : (    (tt8Bit)inChar == TM_PPP_FLAG_CHAR \
                                 || (tt8Bit)inChar == TM_PPP_ESC_CHAR ) )

/* if tvPppAuthPriority is zero, we set tvPppAuthPriority to the following
 * default value, which correspons to  MSCHAP2 > CHAP > PAP > MSCHAP1, each
 * authMethod saves its priority value (1,2, 3, or 4) into 4 bits
 */
/* we use 4 bits to save the priority value for each PPP auth method.
 * the tvPppAuthPriority has 32 bits, which means we can support 32/4=8
 * authentication methods
 */
#define TM_PPP_AUTHPRIORITY_BITS               4

/*4 means TM_PPP_AUTHPRIORITY_BITS */
#ifdef TM_USE_PPP_MSCHAP
/* TM_PPP_AUTH_DEFAULT_PRIORITY = 0x4321 */
#define TM_PPP_AUTH_DEFAULT_PRIORITY                         \
            (((tt32Bit) 1 << ((TM_PPP_AUTHMETHOD_EAP       - 1) *4)) | \
             ((tt32Bit) 2 << ((TM_PPP_AUTHMETHOD_CHAP      - 1) *4)) | \
             ((tt32Bit) 3 << ((TM_PPP_AUTHMETHOD_PAP       - 1) *4)) | \
             ((tt32Bit) 4 << ((TM_PPP_AUTHMETHOD_MSCHAP_V1 - 1) *4)))
#else /* !TM_USE_PPP_MSCHAP */
/* TM_PPP_AUTH_DEFAULT_PRIORITY = 0x321 */
#define TM_PPP_AUTH_DEFAULT_PRIORITY                         \
            (((tt32Bit) 1 << ((TM_PPP_AUTHMETHOD_EAP       - 1) *4)) | \
             ((tt32Bit) 2 << ((TM_PPP_AUTHMETHOD_CHAP      - 1) *4)) | \
             ((tt32Bit) 3 << ((TM_PPP_AUTHMETHOD_PAP       - 1) *4)))
#endif /* TM_USE_PPP_MSCHAP */
/* x can be any PPP authentication methods, such as TM_PPP_AUTHMETHOD_MSCHAP_V2,
 * TM_PPP_AUTHMETHOD_CHAP, TM_PPP_AUTHMETHOD_PAP */
#define tm_ppp_auth_get_priority(x) \
             (tt8Bit)((tm_context(tvPppAuthPriority) &      \
                       (tt32Bit)(0x0f << (((x) - 1) *4)))   \
                       >> (((x) - 1) *4))

/*
 * DSP Macros for the 'C3x & 'C5x DSPs
 */

/*
 * Macros describing the number of octets.
 * These are used rather than sizeof or literals when needed, since sizeof()
 * can't be used for this purpose on a DSP with a different word size.
 */
#define TM_64BIT_BYTE_COUNT     8
#define TM_32BIT_BYTE_COUNT     4
#define TM_16BIT_BYTE_COUNT     2
#define TM_8BIT_BYTE_COUNT      1

/*
 * Set 8-bit value at an index in a packed buffer.
 */
#ifdef TM_DSP
#define tm_ppp_put_char_index(buffer,index,dataChar) \
{ \
    (buffer)[tm_packed_byte_count((index)+1)-1] &= \
        (~(0xff << ((TM_DSP_ROUND_PTR - \
        ((index) % TM_DSP_BYTES_PER_WORD)) << 3))); \
    (buffer)[tm_packed_byte_count((index)+1)-1] |= \
        ((dataChar) << ((TM_DSP_ROUND_PTR - \
        ((index) % TM_DSP_BYTES_PER_WORD)) << 3)); \
}
#else /* !TM_DSP */
#define tm_ppp_put_char_index(buffer, index, dataChar) \
{ \
    (buffer)[index] = dataChar; \
}
#endif /* TM_DSP */

/*
 * Set an 8-bit value at the current pointer location, and move to next byte.
 */
#ifdef TM_DSP
#define tm_ppp_put_char_next(buffer, offset, dataChar) \
{ \
    *buffer &= ~(0xff << ((TM_DSP_ROUND_PTR - \
                ((offset) % TM_DSP_BYTES_PER_WORD)) << 3)); \
    *buffer |= (dataChar << ((TM_DSP_ROUND_PTR - \
               ((offset) % TM_DSP_BYTES_PER_WORD)) << 3)); \
    if ( (++(offset) % TM_DSP_BYTES_PER_WORD) == 0) \
    { \
        offset=0; \
        buffer++; \
    } \
}
#else /* !TM_DSP */
#ifdef TM_LINT
LINT_LOSS_PREC_MACRO(tm_ppp_put_char_next)
LINT_NULL_PTR_MACRO(tm_ppp_put_char_next)
#endif /* TM_LINT */
#define tm_ppp_put_char_next(buffer, offset, dataChar) \
    (*(tt8BitPtr)buffer++ = (tt8Bit)dataChar)

#endif /* TM_DSP */

/*
 * Get an 8-bit value from the current pointer location, and move to next byte.
 */
#ifdef TM_DSP
#define tm_ppp_get_char_next(buffer, offset) \
((*(buffer) >> ((TM_DSP_ROUND_PTR - \
               ((offset) % TM_DSP_BYTES_PER_WORD)) << 3)) & 0xff); \
{ \
    if ( (++(offset) % TM_DSP_BYTES_PER_WORD) == 0) \
    { \
        offset=0; \
        (buffer)++; \
    } \
}
#else /* !TM_DSP */
#define tm_ppp_get_char_next(buffer, offset)  *(buffer)++;
#endif /* TM_DSP */


/*
 * Get 8-bit value from index in a packed buffer.
 */
#ifdef TM_DSP
#define tm_ppp_get_char_offset(buffer, offset) \
(  (*(buffer) >> ((TM_DSP_ROUND_PTR - \
    (offset % TM_DSP_BYTES_PER_WORD)) << 3))   & 0xff)
#else /* !TM_DSP */
/* we don't have offset if we are using non-DSP CPU type */
#define tm_ppp_get_char_offset(buffer, offset)  (*buffer)
#endif /* TM_DSP */

/*
 * Get 8-bit value from index of char array in a packed buffer.
 */
#ifdef TM_DSP
#define tm_ppp_get_char_index(buffer, index) \
(  ((buffer)[(index)/TM_DSP_BYTES_PER_WORD] >> ((TM_DSP_ROUND_PTR - \
    ((index) % TM_DSP_BYTES_PER_WORD)) << 3))   & 0xff)
#else /* !TM_DSP */
#define tm_ppp_get_char_index(buffer, index)  ((buffer)[index])
#endif /* TM_DSP */


/*
 * Move to next byte.
 */
#ifdef TM_DSP
#define tm_ppp_inc_ptr_n(buffer, offset, n) \
    { \
        (buffer) += ((offset)+ (n))>> (TM_DSP_BYTES_PER_WORD >> 1); \
        (offset) = ((offset)+ (n)) % TM_DSP_BYTES_PER_WORD; \
    }
#else /* !TM_DSP */
#define tm_ppp_inc_ptr_n(buffer, offset, n) \
    (buffer) += n;
#endif /* TM_DSP */

#ifdef TM_DSP
#define tm_zero_dsp_offset(offset) (offset = 0)
#define tm_set_dsp_offset(dstOffset, srcOffset) (dstOffset = srcOffset)
#define tm_inc_dsp_offset(dstOffset, incValue) (dstOffset += incValue)
#else /* !TM_DSP */
#define tm_zero_dsp_offset(offset)
#define tm_set_dsp_offset(dstOffset, srcOffset)
#define tm_inc_dsp_offset(dstOffset, incValue)
#endif /* TM_DSP */


/*
 * Perform a byte by byte copy.
 */
#ifdef TM_DSP
#define tm_ppp_byte_copy(sourcePtr,sourceOffset,destPtr,destOffset,bytes) \
tfPppByteCopy( (int *) (sourcePtr), \
                  sourceOffset, \
                  (int *) (destPtr), \
                  destOffset, \
                  bytes )
#else /* !TM_DSP */
#define tm_ppp_byte_copy(sourcePtr,sourceOffset,destPtr,destOffset,bytes) \
tm_bcopy(sourcePtr, destPtr, bytes)
#endif /* TM_DSP */

/*
 * Move pointer to the next option in packet
 */
#ifdef TM_DSP
#define tm_ppp_move_next_option(optPtr, optLength, offset) \
optPtr += (optLength + offset) >> (TM_DSP_BYTES_PER_WORD >> 1); \
offset  = (optLength + offset) % TM_DSP_BYTES_PER_WORD
#else /* !TM_DSP */
#define tm_ppp_move_next_option(optPtr, optLength, offset) \
(optPtr += optLength)
#endif /* TM_DSP */


/*
 * Macros for creating NAK, Reject & Request packets.  On a DSP platform
 * in addition to the normal information required to create these packets
 * it is also necessary to specify byte offsets since the required option
 * may not lie on a 16-bit boundary.
 *
 * These macros are used both for calling these functions as well as declaring
 * them.  The 'functionName' parameter of the macros is not actually used - it
 * is included to easily identify the true function name that the macros are
 * referring to.
 */
#ifdef TM_DSP

#define tm_ppp_add_nak(functionName, inDataPtr, optPtr, outPtr, optLen,        \
               rejPktPtr, nakPktPtr,  inOffset, outOffsetPtr)                  \
tfPppAddNak(inDataPtr, optPtr, outPtr, optLen, rejPktPtr, nakPktPtr, inOffset, \
            outOffsetPtr)

#define tm_ppp_add_reject(functionName, inDataPtr, outDataPtrPtr, optLen, \
              rejPktPtr, nakPktPtr, inOffset, outOffsetPtr)               \
tfPppAddReject(inDataPtr, outDataPtrPtr, optLen, rejPktPtr, nakPktPtr,    \
               inOffset, outOffsetPtr)

#define tm_ppp_send_option(functionName, pktPtr, negOpt, optNum, optValPtr, \
               optSize, optParmPtr, optParmSize, offsetPtr)                 \
tfPppSendOption(pktPtr, negOpt, optNum, optValPtr, optSize, optParmPtr,     \
                optParmSize, offsetPtr)

#define tm_ppp_parse_finish(functionName, pktPtr, rejPktPtr, nakPktPtr, \
                dataPtr, eventPtr, ident, offset)                       \
tfPppParseFinish(pktPtr, rejPktPtr, nakPktPtr, dataPtr, eventPtr, ident, offset)

#else /* !TM_DSP */

/*
 * Macros for creating NAK,Reject & Request packets.  Does not include byte
 * offset information.
 */
#define tm_ppp_add_nak(functionName, inDataPtr, optPtr, outPtr, optLen, \
               rejPktPtr, nakPktPtr,  inOffset, outOffsetPtr)           \
tfPppAddNak(inDataPtr, optPtr, outPtr, optLen, rejPktPtr, nakPktPtr)

#define tm_ppp_add_reject(functionName, inDataPtr, outDataPtrPtr, optLen, \
              rejPktPtr, nakPktPtr, inOffset, outOffsetPtr)               \
tfPppAddReject(inDataPtr, outDataPtrPtr, optLen, rejPktPtr, nakPktPtr)

#define tm_ppp_send_option(functionName, pktPtr, negOpt, optNum, optValPtr, \
               optSize, optParmPtr, optParmSize, offsetPtr)                 \
tfPppSendOption(pktPtr, negOpt, optNum, optValPtr, optSize, optParmPtr,     \
                optParmSize)

#define tm_ppp_parse_finish(functionName, pktPtr, rejPktPtr, nakPktPtr, \
                dataPtr, eventPtr, ident, offset)                       \
tfPppParseFinish(pktPtr, rejPktPtr, nakPktPtr, dataPtr, eventPtr, ident)
#endif /* TM_DSP */

#ifdef TM_DSP
#define tm_ppp_memcmp(memPtr1, memPtr2, byteCount) \
tfDspMemcmp((ttCharPtr) (memPtr1), 0, (ttCharPtr) (memPtr2), 0, byteCount)
#else /* !TM_DSP */
#define tm_ppp_memcmp(memPtr1, memPtr2, byteCount) \
tm_memcmp(memPtr1, memPtr2, byteCount)
#endif /* TM_DSP */

/*
 * Determines the length of a packed string.
 */
#ifdef TM_DSP
#define tm_pppstrlen(charDataPtr) tfPackedStringLength((tt8BitPtr) charDataPtr)
#else /* !TM_DSP */
#define tm_pppstrlen(charDataPtr) tm_strlen(charDataPtr)
#endif /* TM_DSP */

/*
 * PPP State Machine Defines
 */

/* NCP index into function table */
#define TM_PPP_LCP_INDEX            0
#define TM_PPP_IPCP_INDEX           1
#define TM_PPP_PAP_INDEX            2
#define TM_PPP_CHAP_INDEX           3
#define TM_PPP_IPV6CP_INDEX         4
/* The following are not real indices, just used in tfPppNextLayer */
#define TM_PPP_EAP_INDEX            5
#define TM_PPP_CBCP_INDEX           6

/* Function pointer indexes */
#define TM_PPP_FN_LAYER_START       0
#define TM_PPP_FN_LAYER_FINISH      1
#define TM_PPP_FN_LAYER_UP          2
#define TM_PPP_FN_LAYER_DOWN        3
#define TM_PPP_FN_INIT_RESTART      4
#define TM_PPP_FN_ZERO_RESTART      5
#define TM_PPP_FN_SEND_CFG_REQ      6
#define TM_PPP_FN_SEND_CFG_ACK      7
#define TM_PPP_FN_SEND_CFG_NAK      8
#define TM_PPP_FN_SEND_COD_REJ      9
#define TM_PPP_FN_SEND_TRM_REQ      10
#define TM_PPP_FN_SEND_TRM_ACK      11
#define TM_PPP_FN_SEND_ECHO         12
#define TM_PPP_FN_ERROR             13

/*
 * PPP States
 */
#define TM_PPPS_INITIAL             0
#define TM_PPPS_STARTING            1
#define TM_PPPS_CLOSED              2
#define TM_PPPS_STOPPED             3
#define TM_PPPS_CLOSING             4
#define TM_PPPS_STOPPING            5
#define TM_PPPS_REQ_SENT            6
#define TM_PPPS_ACK_REC             7
#define TM_PPPS_ACK_SENT            8
#define TM_PPPS_OPENED              9
#define TM_PPPS_RESET_SENT          10
#define TM_PPPS_RESET_ACK           11
#define TM_PPPS_INVALID          0xff

#ifdef TM_USE_PPP_CALLBACK
/*
 * CBCP Additional States
 */
#define TM_CBCPS_REQ_WAIT           101
#define TM_CBCPS_RESP_SENT          102
#define TM_CBCPS_DISCONNECT_DELAY   103
#define TM_CBCPS_CALLBACK_DELAY     104
#define TM_CBCPS_CALLING            105
#define TM_CBCPS_STARTING2          106
#endif /* TM_USE_PPP_CALLBACK */

/*
 * PPP Events
 */
#define TM_PPPE_UP                  0
#define TM_PPPE_DOWN                1
#define TM_PPPE_OPEN                2
#define TM_PPPE_CLOSE               3
#define TM_PPPE_TIMEOUT             4
#define TM_PPPE_LAST_TIMEOUT        5
#define TM_PPPE_GOOD_CFG_REQ        6
#define TM_PPPE_BAD_CFG_REQ         7
#define TM_PPPE_CFG_ACK             8
#define TM_PPPE_CFG_NAK             9
#define TM_PPPE_TRM_REQ             10
#define TM_PPPE_TRM_ACK             11
#define TM_PPPE_UNKNOWN_CODE        12
#define TM_PPPE_CODE_PRO_REJ        13
#define TM_PPPE_CODE_PRO_ERROR      14
#define TM_PPPE_ECHO_DIS_REQP       15

#define TM_PPPE_NO_EVENT            (tt8Bit)255

#ifdef TM_USE_PPP_CALLBACK
/*
 * CBCP Additional Events
 */
#define TM_CBCPE_CFG_RESPONSE       101
#define TM_CBCPE_PHY_UP             102
#endif /* TM_USE_PPP_CALLBACK */

/*
 * Action Procedure Index Defines
 * These are tied to the Function Table
 * If this is changed, the function table
 * MUST be changed as well and visa-versa.
 */
#define TM_PPPM_LAYER_START         0
#define TM_PPPM_LAYER_STOP          1
#define TM_PPPM_LAYER_UP            2
#define TM_PPPM_LAYER_UP_ACK        3
#define TM_PPPM_LAYER_DOWN          5
#define TM_PPPM_INIT_RESTART_CNT    6
#define TM_PPPM_1SND_CFG_REQ        7
#define TM_PPPM_OSND_CFG_REQ        9
#define TM_PPPM_RSND_CFG_REQ        11
#define TM_PPPM_SND_CFG_ACK         12
#define TM_PPPM_SND_CFG_NAK         13
#define TM_PPPM_SND_CODE_REJECT     14
#define TM_PPPM_1SND_CFG_REQ_ACK    15
#define TM_PPPM_OSND_CFG_REQ_ACK    18
#define TM_PPPM_1SND_CFG_REQ_NAK    21
#define TM_PPPM_OSND_CFG_REQ_NAK    24
#define TM_PPPM_1SND_TERM_REQ       26
#define TM_PPPM_OSND_TERM_REQ       28
#define TM_PPPM_RSND_TERM_REQ       31
#define TM_PPPM_SND_TERM_ACK        32
#define TM_PPPM_OSND_TERM_ACK       33
#define TM_PPPM_SND_ECHO_REPLY      36
#define TM_PPPM_TRANSITION_ERROR    37
#define TM_PPPM_LAST_INDEX          38

/*
 * VJ Field offsets in the fixed (24byte) size structure
 */
#define TM_PPP_VJ_FLAG_FLAG         0
#define TM_PPP_VJ_CONNECT_FLAG      1
#define TM_PPP_VJ_CHECKSUM_FLAG     2
#define TM_PPP_VJ_URGENT_FLAG       4
#define TM_PPP_VJ_WINDOW_FLAG       8
#define TM_PPP_VJ_ACK_FLAG          12
#define TM_PPP_VJ_SEQUENCE_FLAG     16
#define TM_PPP_VJ_IDENT_FLAG        20
#define TM_PPP_VJ_PADDING_FLAG      24

#define TM_PPP_VJ_COMP_SLOT_ID      1

/* PPP packet offsets */
#define TM_PPP_ADDR_FIELD           0
#define TM_PPP_PROTOCOL_FIELD       2
#define TM_PPP_DATA_FIELD           4

/* PPP frame types (null,broadcast,etc) */
#define TM_PPP_NULL_CHAR            (unsigned char)0x00
#define TM_PPP_BROADCAST_CHAR       (unsigned char)0xff
#define TM_PPP_UA_FRAME_CHAR        (unsigned char)0x03
#define TM_PPP_STATE_FUNC_NULL_PTR  (ttPppStateFunctPtr)0

/* State machine no op index */
#define TM_MATRIX_NOOP              TM_PPPM_LAST_INDEX

#define TM_UNCOMPRESSED_TCP 0x07

/* VJ packet types */
#define TM_PPP_COMP_TCP_PROTOCOL_BYTE        0x2d
#define TM_PPP_UNCOMP_TCP_PROTOCOL_BYTE      0x2f

/* VJ Compression bits */
#define TM_VJ_U             0x01
#define TM_VJ_W             0x02
#define TM_VJ_A             0x04
#define TM_VJ_S             0x08
#define TM_VJ_P             0x10
#define TM_VJ_I             0x20
#define TM_VJ_C             0x40
#define TM_VJ_SAWU          (TM_VJ_S | TM_VJ_A | TM_VJ_W | TM_VJ_U)
#define TM_VJ_SWU           (TM_VJ_S | TM_VJ_W | TM_VJ_U)
#define TM_VJ_SA            (TM_VJ_S | TM_VJ_A)
#define TM_VJ_STAR_SA       0x0B /* *SA: 1011 */
#define TM_VJ_STAR_S        0x0F /*  *S: 1111 */

#define TM_PPP_CLIENT_LINK  (tt8Bit)0
#define TM_PPP_SERVER_LINK  (tt8Bit)1

/* IPV6CP options: */
#define TM_IPV6CP_INTERFACE_ID 1

#define TM_6PAK_INTERFACE_ID_LEN tm_packed_byte_count(8)

/* Minimum length of IPHC IPCP/IPV6CP configuration option. */
#define TM_PPP_IPHC_MIN_OPT_LEN 12

/* RFC-2507 style compressed packet types. */
#ifndef TM_LITTLE_ENDIAN
#define TM_PPP_IPHC_FULL_HDR         (unsigned short)(0x0061)
#define TM_PPP_IPHC_COMP_TCP         (unsigned short)(0x0063)
#define TM_PPP_IPHC_COMP_TCP_NODELTA (unsigned short)(0x2063)
#define TM_PPP_IPHC_COMP_NON_TCP     (unsigned short)(0x0065)
#define TM_PPP_IPHC_COMP_UDP_8       (unsigned short)(0x0067)
#define TM_PPP_IPHC_COMP_UDP_16      (unsigned short)(0x2067)
#define TM_PPP_IPHC_CTX_STATE        (unsigned short)(0x2065)
#else /* TM_LITTLE_ENDIAN */
#define TM_PPP_IPHC_FULL_HDR         (unsigned short)(0x6100)
#define TM_PPP_IPHC_COMP_TCP         (unsigned short)(0x6300)
#define TM_PPP_IPHC_COMP_TCP_NODELTA (unsigned short)(0x6320)
#define TM_PPP_IPHC_COMP_NON_TCP     (unsigned short)(0x6500)
#define TM_PPP_IPHC_COMP_UDP_8       (unsigned short)(0x6700)
#define TM_PPP_IPHC_COMP_UDP_16      (unsigned short)(0x6720)
#define TM_PPP_IPHC_CTX_STATE        (unsigned short)(0x6520)
#endif /* TM_LITTLE_ENDIAN */

/*
 * PPP Header
 * The header is always fixed to 32 bits inside for processing
 */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
/* Default is Little Endian for 32-bit DSP's for backward compatibility */
typedef struct tsPppHdr
{
    unsigned int   pppHdrProtocol     :16;
    unsigned int   pppHdrControl      :8;
    unsigned int   pppHdrAddress      :8;
} ttPppHdr;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct tsPppHdr
{
    unsigned int   pppHdrAddress      :8;
    unsigned int   pppHdrControl      :8;
    unsigned int   pppHdrProtocol     :16;
} ttPppHdr;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct tsPppHdr
{
    unsigned int   pppHdrAddress      :8;
    unsigned int   pppHdrControl      :8;
    tt16Bit        pppHdrProtocol;
} ttPppHdr;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct tsPppHdr
{
    tt8Bit   pppHdrAddress;
    tt8Bit   pppHdrControl;
    tt16Bit  pppHdrProtocol;
} ttPppHdr;
#endif /* !TM_DSP */

typedef ttPppHdr TM_FAR * ttPppHdrPtr;

typedef int (TM_CODE_FAR * ttPppIncomingFuncPtr)(ttPppVectPtr, ttPacketPtr packetPtr);

typedef union tuPppAuthFunc
{
    ttPapAuthenticateFunctPtr    pppaPapFuncPtr;
    ttChapAuthenticateFunctPtr   pppaChapFuncPtr;
#ifdef TM_USE_PPP_MSCHAP
    ttMsChapAuthenticateFunctPtr pppaMsChapFuncPtr;
#endif /*  TM_USE_PPP_MSCHAP */
} ttPppAuthFunc;

/*
 * PAP Functions
 */
ttVoid tfPapInit(ttPppVectPtr pppVectPtr);
ttVoid tfPapDefaultInit(ttPppVectPtr pppVectPtr);
void tfPapTimeout (ttVoidPtr      timerBlockPtr,
                   ttGenericUnion userParm1,
                   ttGenericUnion userParm2);
void tfPapRemoteReqTimeout (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2);
int tfPapSendAuthRequest(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr);
int tfPapSendCodeReject(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfPapThisLayerUp(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfPapThisLayerFinish(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr);
int tfPapThisLayerStart(ttPppVectPtr pppVectPtr,
                        ttPacketPtr  packetPtr);
int tfPapSendAuthNak(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfPapInitRestartCounter(ttPppVectPtr pppVectPtr,
                            ttPacketPtr packetPtr);
int tfPapZeroRestartCounter(ttPppVectPtr pppVectPtr,
                            ttPacketPtr packetPtr);
int tfPapIncomingPacket(ttPppVectPtr pppVectPtr,ttPacketPtr packetPtr);
int tfPapSendTermAck(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
ttPacketPtr tfPapParseAuthRequest(ttPppVectPtr pppVectPtr,
                                  ttPacketPtr   packetPtr,
                                  tt16Bit       length,
                                  tt8BitPtr     packetType);

/*
 * CHAP Functions
 */
ttVoid tfChapInit(ttPppVectPtr pppVectPtr);
ttVoid tfChapDefaultInit(ttPppVectPtr pppVectPtr);
void tfChapTimeout (ttVoidPtr      timerBlockPtr,
                    ttGenericUnion userParm1,
                    ttGenericUnion userParm2);
void tfChapResponseTimeout (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2);
int tfChapSendChallenge(ttPppVectPtr pppVectPtr,
                        ttPacketPtr packetPtr);
int tfChapSendCodeReject(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr);
int tfChapThisLayerUp(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfChapThisLayerFinish(ttPppVectPtr pppVectPtr,
                          ttPacketPtr packetPtr);
int tfChapThisLayerStart(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr);
int tfChapSendSuccess(ttPppVectPtr pppVectPtr,
                      ttPacketPtr packetPtr);
int tfChapSendFailure(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfChapInitRestartCounter(ttPppVectPtr pppVectPtr,
                             ttPacketPtr packetPtr);
int tfChapZeroRestartCounter(ttPppVectPtr pppVectPtr,
                             ttPacketPtr packetPtr);
int tfChapIncomingPacket(ttPppVectPtr pppVectPtr,ttPacketPtr packetPtr);
int tfChapSendAuthNak(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfChapSendTermAck(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfChapChallenge(ttPppVectPtr  pppVectPtr,
                    ttPacketPtr   packetPtr,
                    tt16Bit       length);
ttPacketPtr tfChapParseAuthResponse(ttPppVectPtr pppVectPtr,
                                    ttPacketPtr packetPtr,
                                    tt16Bit optionsLength,
                                    tt8BitPtr packetType);
static int tfPppRegisterAuthenticate(ttUserInterface           interfaceId,
                                     ttPppAuthFunc             authFuncUnion,
                                     tt16Bit                   protocol);
static int tfChapStandardChallenge(tt8BitPtr     identPtr,
                                   tt8BitPtr     passwordPtr,
                                   tt16Bit       passwordLen,
                                   tt8BitPtr     valuePtr,
                                   tt16Bit       valueSize,
                                   tt8Bit        valuePtrOffset,
                                   tt8BitPtr     responsePtr);
static void tfChapStandardParseAuth(tt8BitPtr       secretPtr,
                                    int             secretLength,
                                    tt8BitPtr       identPtr,
                                    tt8BitPtr       challengePtr,
                                    tt8BitPtr       responsePtr);
#ifdef TM_USE_PPP_MSCHAP
static int tfMChapV1SendChangePasswordPacket(
                           ttPppVectPtr    pppVectPtr,
                           ttPacketPtr     packetPtr,
                           tt8BitPtr       newChallengePtr);

static void tfMChapV1SetFailurePacket(
                              ttPppVectPtr  pppVectPtr,
                              ttCharPtr     chapMsgPtr,
                              tt8BitPtr     chapMsgLenPtr,
                              tt8Bit        retryCount,
                              int           failureType);

static void tfMChapV1ProcessFailurePacket(
                                ttPppVectPtr     pppVectPtr,
                                ttPacketPtr      packetPtr,
                                tt16Bit          optionsLength,
                                tt8BitPtr        retryPtr);

#endif /* TM_USE_PPP_MSCHAP */


static void tfPppCheckAuthProtocolBits(ttPppVectPtr pppVectPtr);

static int tfPppGetAuthProtocolBit(tt16Bit     protocol,
                                   tt8Bit      chapAlgorithm);

static tt8Bit tfLcpGetNextAuthMethod(tt8Bit     authProtocolBits,
                                     tt8Bit     currentAuthMethod,
                                     tt8BitPtr  algorithmPtr);
static tt16Bit tfPppFromMethodToProtocol(tt8Bit authMethod);
#ifdef TM_USE_EAP
static void tfPppEapAuthNotify(ttDeviceEntryPtr devPtr,
                               int              authResult,
                               int              isAuthLocal,
                               tt8Bit           needLockFlag);
#endif /* TM_USE_EAP */

/*
 * IPCP Functions
 */
#ifdef TM_USE_IPV4
static ttVoid tfIpcpInit(ttPppVectPtr pppVectPtr);
static ttVoid tfIpcpDefaultInit(ttPppVectPtr pppVectPtr);
static ttVoid tfIpcpInitCommon(ttPppVectPtr pppVectPtr);
void tfIpcpTimeout (ttVoidPtr      timerBlockPtr,
                    ttGenericUnion userParm1,
                    ttGenericUnion userParm2);
int tfIpcpSendCfgRequest(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr);
int tfIpcpSendCodeReject(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr);
int tfIpcpThisLayerUp(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfIpcpThisLayerFinish(ttPppVectPtr pppVectPtr,
                          ttPacketPtr packetPtr);
int tfIpcpThisLayerStart(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr);
int tfIpcpSendCfgNak(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfIpcpInitRestartCounter(ttPppVectPtr pppVectPtr,
                             ttPacketPtr packetPtr);
int tfIpcpZeroRestartCounter(ttPppVectPtr pppVectPtr,
                             ttPacketPtr packetPtr);
int tfIpcpIncomingPacket(ttPppVectPtr pppVectPtr,ttPacketPtr packetPtr);
int tfIpcpSendTermAck(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfIpcpParseNak(ttPppVectPtr pppVectPtr,
                   ttPacketPtr  packetPtr,
                   tt16Bit      optionsLength);
int tfIpcpParseReject(ttPppVectPtr pppVectPtr,
                      ttPacketPtr  packetPtr,
                      tt16Bit      optionsLength);
ttPacketPtr tfIpcpParseConfig(ttPppVectPtr  pppVectPtr,
                              ttPacketPtr   packetPtr,
                              tt16Bit       length,
                              tt8BitPtr     pppEventPtr);
static ttVoid tfIpcpReInit(ttPppVectPtr pppVectPtr);
/*
 * VJ Funtions (only used with IPv4)
 */
#ifndef TM_DSP
static int tfVjCompIncomingPacket(ttPppVectPtr pppVectPtr,
                                  ttPacketPtr  packetPtr);
static int tfVjCompSendPacket(ttPppVectPtr  pppVectPtr,
                              ttPacketPtr   packetPtr,
                              tt16BitPtr    pktTypePtr);
static int tfVjMemCmp( ttCharPtr     memPtr1,
                       ttCharPtr     memPtr2,
                       unsigned      memLength);
#endif /* !TM_DSP */

#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
static ttVoid tfIp6cpInit (ttPppVectPtr pppVectPtr);
static ttVoid tfIp6cpInitCommon (ttPppVectPtr pppVectPtr);
static ttVoid tfIp6cpDefaultInit (ttPppVectPtr pppVectPtr);
ttVoid tfIp6cpTimeout(ttVoidPtr      timerBlockPtr,
                      ttGenericUnion userParm1,
                      ttGenericUnion userParm2);
int tfIp6cpSendCfgRequest(ttPppVectPtr pppVectPtr,
                          ttPacketPtr  packetPtr);
int tfIp6cpSendCodeReject(ttPppVectPtr  pppVectPtr,
                          ttPacketPtr   packetPtr);
int tfIp6cpThisLayerUp (ttPppVectPtr    pppVectPtr,
                        ttPacketPtr     packetPtr);
int tfIp6cpThisLayerFinish (ttPppVectPtr pppVectPtr,
                            ttPacketPtr  packetPtr);
int tfIp6cpThisLayerStart(ttPppVectPtr  pppVectPtr,
                          ttPacketPtr   packetPtr);
int tfIp6cpThisLayerDown(ttPppVectPtr   pppVectPtr,
                         ttPacketPtr    packetPtr);
int tfIp6cpSendCfgNak(ttPppVectPtr      pppVectPtr,
                      ttPacketPtr       packetPtr);
int tfIp6cpInitRestartCounter(ttPppVectPtr      pppVectPtr,
                              ttPacketPtr       packetPtr);
int tfIp6cpZeroRestartCounter(ttPppVectPtr      pppVectPtr,
                              ttPacketPtr       packetPtr);
int tfIp6cpIncomingPacket (ttPppVectPtr pppVectPtr,
                           ttPacketPtr  packetPtr);
int tfIp6cpSendTermAck (ttPppVectPtr    pppVectPtr,
                        ttPacketPtr     packetPtr);
int tfIp6cpParseNak (ttPppVectPtr    pppVectPtr,
                     ttPacketPtr     packetPtr,
                     tt16Bit         optionsLength);
int tfIp6cpParseReject (ttPppVectPtr pppVectPtr,
                        ttPacketPtr  packetPtr,
                        tt16Bit      optionsLength);
ttPacketPtr tfIp6cpParseConfig (ttPppVectPtr    pppVectPtr,
                                ttPacketPtr     packetPtr,
                                tt16Bit         length,
                                tt8BitPtr       pppEventPtr);
static ttVoid tfIp6cpReInit (ttPppVectPtr pppVectPtr);
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_PPP_CALLBACK
/*
 * Callback control protocol functions
 */
static int tfCbcpStateMachine(      ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr,
                                    unsigned int    event );
static void tfCbcpInit(             ttPppVectPtr    pppVectPtr,
                                    tt8Bit          pppLinkLayer );
static void tfCbcpDefaultInit(      ttPppVectPtr    pppVectPtr );
static int tfCbcpIncomingPacket(    ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static void tfCbcpTimeout(          ttVoidPtr       timerBlockPtr,
                                    ttGenericUnion  userParm1,
                                    ttGenericUnion  userParm2 );
static int tfCbcpThisLayerStart(    ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpThisLayerFinish(   ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpThisLayerUp(       ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpThisLayerDown(     ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpLcpUp(             ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpLcpDown(           ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpLcpFinish(         ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpHangUp(            ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpDial(              ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpDelayBeforeHangUp( ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpCallbackDelay(     ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpSendRequest(       ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpSendResponse(      ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static int tfCbcpSendAck(           ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr );
static void tfCbcpTimerAdd(         ttPppVectPtr    pppVectPtr,
                                    unsigned int    expireSeconds );
static void tfCbcpTimerRemove(      ttPppVectPtr    pppVectPtr );
#endif /* TM_USE_PPP_CALLBACK */

/*
 * LCP Functions
 */
ttVoid tfLcpInit(ttPppVectPtr pppVectPtr);
ttVoid tfLcpDefaultInit(ttPppVectPtr pppVectPtr);
void tfLcpTimeout (ttVoidPtr      timerBlockPtr,
                   ttGenericUnion userParm1,
                   ttGenericUnion userParm2);
int tfLcpSendCfgRequest(ttPppVectPtr pppVectPtr,
                        ttPacketPtr packetPtr);
int tfLcpSendTermRequest(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr);
int tfLcpSendTermAck(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfLcpThisLayerDown(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfLcpThisLayerUp(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfLcpThisLayerFinish(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr);
int tfLcpThisLayerStart(ttPppVectPtr pppVectPtr,
                        ttPacketPtr packetPtr);
int tfLcpSendCodeReject(ttPppVectPtr pppVectPtr,
                        ttPacketPtr packetPtr);
int tfLcpSendCfgNak(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfLcpInitRestartCounter(ttPppVectPtr pppVectPtr,
                            ttPacketPtr packetPtr);
int tfLcpZeroRestartCounter(ttPppVectPtr pppVectPtr,
                            ttPacketPtr packetPtr);
int tfLcpIncomingPacket(ttPppVectPtr pppVectPtr,ttPacketPtr packetPtr);
int tfLcpSendProtocolReject(ttPppVectPtr pppVectPtr,
                            ttPacketPtr  packetPtr);
ttPacketPtr tfLcpParseConfig(ttPppVectPtr pppVectPtr,
                             ttPacketPtr   packetPtr,
                             tt16Bit       length,
                             tt8BitPtr     packetType);
int tfLcpParseNak(ttPppVectPtr pppVectPtr,
                  ttPacketPtr  packetPtr,
                  tt16Bit      length);
int tfLcpParseReject(ttPppVectPtr pppVectPtr,
                     ttPacketPtr  packetPtr,
                     tt16Bit      length);
#ifdef TM_PPP_LQM
void tfPppParseEchoReply(ttPppVectPtr pppVectPtr,
                           ttPacketPtr  packetPtr,
                           tt16Bit      length);
#endif /* TM_PPP_LQM */
#ifdef TM_USE_PPP_CALLBACK
static void tfLcpSendCallbackOptions(
                    ttPppVectPtr            pppVectPtr,
                    ttPacketPtr             packetPtr
#ifdef TM_DSP
                    , unsigned int TM_FAR * outOffsetPtr
#endif /* TM_DSP */
                    );
#endif /* TM_USE_PPP_CALLBACK */

/*
 * PPP Functions
 */
int tfPppStateError(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfPppSendCfgAck(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfPppSendTermAck(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfPppSendEchoReply(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfPppThisLayerDown(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfAuthThisLayerDown(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr);
int tfPppAsyncRecv(void TM_FAR *interfaceId, void TM_FAR *bufferHandle);
int tfPppStateMachine( ttPppVectPtr pppVectPtr,
                       ttPacketPtr  packetPtr,
                       tt8Bit       event,
                       tt8Bit       controlProtoNum);
int tfPppStuffPacket(ttPacketPtr packetPtr);
int tfPppNextLayer(ttPppVectPtr pppVectPtr, tt8Bit currentProto);
ttVoid tfPppNcpOpenFail (ttPppVectPtr pppVectPtr);
ttVoid tfPppOpenFailTimeout (ttVoidPtr      timerBlockPtr,
                             ttGenericUnion userParm1,
                             ttGenericUnion userParm2);
int tfPppSendPacket(ttPacketPtr packetPtr);
int tfPppIoctl(int operation);
int tfPppError(int type);


/*
 * Common PPP sub-routines.
 *
 * The following functions (tfPppAddNak, tfPppAddReject, tfPppSendOption and
 * tfPppParseFinish) are wrapped in macros because some parameters are only
 * included when being used on a DSP platform.  Please see the definitions of
 * these macros above.
 *
 * Example:
 *   On a non-DSP platform, tm_ppp_add_nak below expands to:
 *
 * static ttPacketPtr tm_ppp_add_nak(tfPppAddNak,
 *                                   tt8BitPtr         inDataPtr,
 *                                   tt8BitPtr         newOptionValuePtr,
 *                                   tt8BitPtr TM_FAR *outDataPtrPtr,
 *                                   tt16Bit           optionLength,
 *                                   ttPacketPtr       rejectPacketPtr,
 *                                   ttPacketPtr       nakPacketPtr);
 *
 * However, on a DSP platform (TM_DSP is defined), tm_ppp_add_nak
 * below expands to:
 *
 * static ttPacketPtr tm_ppp_add_nak(tfPppAddNak,
 *                                   tt8BitPtr         inDataPtr,
 *                                   tt8BitPtr         newOptionValuePtr,
 *                                   tt8BitPtr TM_FAR *outDataPtrPtr,
 *                                   tt16Bit           optionLength,
 *                                   ttPacketPtr       rejectPacketPtr,
 *                                   ttPacketPtr       nakPacketPtr,
 *                                   unsigned int      inDataOffset,
 *                                   unsigned int *    outDataOffsetPtr);
 *
 * This definition includes the inDataOffset and outDataOffsetPtr, which are
 * byte offset values and are only needed on a DSP platform.
 *
 */

/*
 * Add a NAK value to the specified packet.
 *
 * inDataOffset and outDataOffsetPtr are byte offset values and are only
 * included when using a DSP platform.
 */
static ttPacketPtr tm_ppp_add_nak(tfPppAddNak,
                                  tt8BitPtr         inDataPtr,
                                  tt8BitPtr         newOptionValuePtr,
                                  tt8BitPtr TM_FAR *outDataPtrPtr,
                                  tt16Bit           optionLength,
                                  ttPacketPtr       rejectPacketPtr,
                                  ttPacketPtr       nakPacketPtr,
                                  unsigned int      inDataOffset,
                                  unsigned int *    outDataOffsetPtr);

/*
 * Add a REJECT value to the specified packet.
 *
 * inDataOffset and outDataOffsetPtr are byte offset values and are only
 * included when using a DSP platform.
 */
static ttPacketPtr tm_ppp_add_reject(tfPppAddReject,
                                     tt8BitPtr         inDataPtr,
                                     tt8BitPtr TM_FAR *outDataPtrPtr,
                                     tt16Bit           optionLength,
                                     ttPacketPtr       rejectPacketPtr,
                                     ttPacketPtr       nakPacketPtr,
                                     unsigned int      inDataOffsetPtr,
                                     unsigned int *    outDataOffsetPtr);

/*
 * Add a CONFIG value to the specified packet.
 *
 * packetOffsetPtr is a byte offset value and is only
 * included when using a DSP platform.
 */
static void tm_ppp_send_option( tfPppSendOption,
                                ttPacketPtr     packetPtr,
                                tt32Bit         negotiateOption,
                                tt8Bit          optionNumber,
                                tt8BitPtr       optionValuePtr,
                                tt8Bit          optionSize,
                                tt8BitPtr       optionParmPtr,
                                tt8Bit          optionParmSize,
                                unsigned int *  packetOffsetPtr );

/*
 * Common routine to finish parsing LCP config, IPCP config, or PAP request
 *
 * outDataOffset is a byte offset value and is only
 * included when using a DSP platform.
 */
static ttPacketPtr tm_ppp_parse_finish( tfPppParseFinish,
                                        ttPacketPtr      packetPtr,
                                        ttPacketPtr      rejectPacketPtr,
                                        ttPacketPtr      nakPacketPtr,
                                        tt8BitPtr        outDataPtr,
                                        tt8BitPtr        pppEventPtr,
                                        tt8Bit           ident,
                                        unsigned int     outDataOffset);

/* Common code for PPP client open, and PPP server open */
static int tfPppOpen( ttVoidPtr       interfaceId,
                      tt8Bit          pppLinkLayer,
                      int             protocolFamily);

/*
 * Copy user string for PAP/CHAP password/user name
 */
static int tfPppCopyString( ttConstCharPtr optionValuePtr,
                            tt8BitPtrPtr   namePtrPtr,
                            int            optionLength);

/* Check for windows CLIENT string, and reply to it */
static void tfPppClientWindows(
                ttDeviceEntryPtr    devPtr,
                ttPppVectPtr        pppVectPtr,
                tt8Bit              inDataChar);

/*
 * Free all allocated raw buffers + timers associated with PPP state vector
 */
static void tfPppFreeBuffersTimers(ttPppVectPtr pppVectPtr);

/*
 * Free all PPP timers.
 */
static void tfPppFreeTimers(ttPppVectPtr pppVectPtr);

/*
 * Default initialization
 */
static void tfPppDefaultInit(ttPppVectPtr pppVectPtr);

/*
 * Remove and reset a PPP retries timer
 */
static void tfPppTimerRemove( ttCpStatePtr stateInfoPtr );

/*
 * Common code to add a PPP layer retransmission timer
 */
static void tfPppTimerAdd( ttPppVectPtr   pppVectPtr,
                           ttTmCBFuncPtr  timeoutFuncPtr,
                           ttCpStatePtr   stateInfoPtr );


/* Send a success of failure CHAP message */
static int tfChapSendMessage( ttPppVectPtr pppVectPtr,
                              ttPacketPtr  packetPtr,
                              tt8Bit       chapCode);

/* Common routine for server and client PPP link layer initialization */
static ttUserLinkLayer tfPppAsyncInit( ttLnkNotifyFuncPtr notifyFuncPtr,
                                       tt8Bit             pppLinkLayer );

static void tfPppNoBuffer(tt8BitPtr pppEventPtr, ttPacketPtr packetPtr);

static void tfPppTimeout( ttPppVectPtr pppVectPtr,
                          ttCpStatePtr stateInfoPtr,
                          tt8Bit       pppIndex );

/* Free allocated timer and buffer associated with a CHAP response */
static void tfChapsResponseCleanup(ttChapStatePtr chapStatePtr);

/* LCP or IPCP init restart counter */
static void tfPppInitRestartCounter(ttCpStatePtr stateInfoPtr);

/* Send a code reject to the peer, common code for all layers */
static int tfPppSendCodeReject(ttPppVectPtr pppVectPtr,
                               ttPacketPtr  packetPtr,
                               ttCpStatePtr stateInfoPtr,
                               tt16Bit      pppProtocol);

#ifdef TM_USE_IPHC
static void tfPppSetIphcValues(ttPppVectPtr pppVectPtr,
                               tt16Bit      compressProto,
                               int          protoFamily);
#endif /* TM_USE_IPHC */


#ifdef TM_DSP
/* compute a packed string length */
tt8Bit tfPackedStringLength(tt8BitPtr namePtr);

/* compare octets */
int tfDspMemcmp( ttCharPtr strPtr1, unsigned int off1,
                 ttCharPtr strPtr2, unsigned int off2, unsigned int bytes);
#endif /* TM_DSP */

/*
 *
 * TYPEDEFS
 *
 */

/*
 * Control Protocol Packet Type
 */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
/* Default is Little Endian for 32-bit DSP's for backward compatibility */
typedef struct tsPppCpPacketType
{
    unsigned int  cpLength        :16;
    unsigned int  cpIdentifier    :8;
    unsigned int  cpCode          :8;
} ttPppCpPacketType;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
/* Default is Little Endian for 32-bit DSP's for backward compatibility */
typedef struct tsPppCpPacketType
{
    unsigned int  cpCode          :8;
    unsigned int  cpIdentifier    :8;
    unsigned int  cpLength        :16;
} ttPppCpPacketType;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct tsPppCpPacketType
{
    unsigned int  cpCode          :8;
    unsigned int  cpIdentifier    :8;
    tt16Bit       cpLength;
} ttPppCpPacketType;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct tsPppCpPacketType
{
    tt8Bit  cpCode;
    tt8Bit  cpIdentifier;
    tt16Bit cpLength;
} ttPppCpPacketType;
#endif /* TM_DSP */

typedef ttPppCpPacketType TM_FAR *ttPppCpPacketTypePtr;

/*
 * Post input processing of the vjfields results in
 * the following fixed size structure (for alignment)
*/
#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */
typedef struct tsVjCompFields
{
    tt8Bit  vjFlags;
    tt8Bit  vjConnection;
    tt16Bit vjTcpCheckSum;
    tt32Bit vjUrgent;
    tt32Bit vjWindow;
    tt32Bit vjAck;
    tt32Bit vjSequence;
    tt32Bit vjIdent;
    tt8Bit  vjFiller[16];
} ttVjCompFields;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */

typedef ttVjCompFields TM_FAR * ttVjCompFieldsPtr;


/*
 * Transition matrix Entry for PPP event/state processing
 */
#ifdef TM_DSP
typedef struct tsTransitionMatrixEntry
{
    unsigned int mtxTransition         :8;
    unsigned int mtxFunctionIndex      :8;
} ttTransitionMatrixEntry;
#else /* !TM_DSP */
typedef struct tsTransitionMatrixEntry
{
    tt8Bit mtxTransition;
    tt8Bit mtxFunctionIndex;
} ttTransitionMatrixEntry;
#endif /* TM_DSP */
/*
 * No need for TM_FAR here (no heap involved)
 */
typedef ttTransitionMatrixEntry TM_CONST_QLF * ttTransitionMatrixEntryPtr;

/*
 * PPP state function entry
 */
#ifdef TM_DSP
typedef struct tsPppStateFunctEntry
{
    unsigned int    pstaIndex        :8;
    unsigned int    pstaFunctIndex   :8;
} ttPppStateFunctEntry;
#else /* !TM_DSP */
typedef struct tsPppStateFunctEntry
{
    tt8Bit          pstaIndex;
    tt8Bit          pstaFunctIndex;
} ttPppStateFunctEntry;
#endif /* TM_DSP */

typedef ttPppStateFunctEntry TM_CONST_QLF * ttPppStateFunctEntryPtr;

/*
 *
 * CONSTANTS
 *
 */

/* PPP server link layer ID */
static const char  TM_CONST_QLF tlPppAsyncServerId[] = "PPSERVASYNC";

/* PPP client link layer ID */
static const char  TM_CONST_QLF tlPppAsyncId[] = "PPPASYNC";

#define  TM_PPP_CHAP_CONST_MESSAGE_SIZE   18
static const char  TM_CONST_QLF tlChapSuccessMsg[] = "Login successful.";
static const char  TM_CONST_QLF tlChapFailedMsg[] = "Login failed.";

/*
 * Frame Check Sequence (RFC 1662)
 */
static const unsigned short  TM_CONST_QLF tlFcsLookupCrc16[256] =
{
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/*
 * PPP ACCM Mapping
 */
static const tt32Bit  TM_CONST_QLF tlPppAccmMap[32]=
{
    TM_UL(0x00000001), TM_UL(0x00000002), TM_UL(0x00000004), TM_UL(0x00000008),
    TM_UL(0x00000010), TM_UL(0x00000020), TM_UL(0x00000040), TM_UL(0x00000080),
    TM_UL(0x00000100), TM_UL(0x00000200), TM_UL(0x00000400), TM_UL(0x00000800),
    TM_UL(0x00001000), TM_UL(0x00002000), TM_UL(0x00004000), TM_UL(0x00008000),
    TM_UL(0x00010000), TM_UL(0x00020000), TM_UL(0x00040000), TM_UL(0x00080000),
    TM_UL(0x00100000), TM_UL(0x00200000), TM_UL(0x00400000), TM_UL(0x00800000),
    TM_UL(0x01000000), TM_UL(0x02000000), TM_UL(0x04000000), TM_UL(0x08000000),
    TM_UL(0x10000000), TM_UL(0x20000000), TM_UL(0x40000000), TM_UL(0x80000000)
};

/*
 * Minimum packet sizes for various LCP code types.  Used to ensure that
 * incoming packets are valid.
 */
static const tt16Bit  TM_CONST_QLF tlLcpMinPktLen[TM_PPP_ECHO_REPLY] =
{
    4,      /* Configure-Req */
    4,      /* Configure-Ack */
    4,      /* Configure-Nak */
    4,      /* Configure-Rej */
    4,      /* Term-Rej      */
    4,      /* Term-Ack      */
    4,      /* Code-Rej      */
    6,      /* Proto-Rej     */
    8,      /* Echo-Req      */
    8       /* Echo-Reply    */
};

/*
 * Defaults Per RFC1661 & RFC 1662 DO NOT CHANGE!!!
 */
static const ttLcpOptions  TM_CONST_QLF tlLcpDefaultOptions =
{
#ifdef TM_USE_PPP_CALLBACK
/* Callback operation flags. */
    TM_32BIT_ZERO,
#endif /* TM_USE_PPP_CALLBACK */
/* Magic Number */
    TM_UL(0),
/* ACCM */
    TM_UL(0xFFFFFFFF),
#ifdef TM_PPP_LQM
/* LQM Reporting-Period */
    TM_UL(0),
#endif /* TM_PPP_LQM */
/* MRU */
    (tt16Bit) TM_PPP_DEF_MRU,
/* Quality Protocol */
#ifdef TM_PPP_LQM
    (tt16Bit) TM_PPP_LQR_HOST_PROTOCOL,
#else /* not TM_PPP_LQM */
    (tt16Bit)0,
#endif /* not TM_PPP_LQM */
/* Auth Protocol Bits */
#ifdef TM_NO_NESTED_BRACES
    (tt8Bit)0,
#else /* !TM_NO_NESTED_BRACES */
    {(tt8Bit)0},
#endif /* !TM_NO_NESTED_BRACES */
    {(tt8Bit)0, (tt8Bit)0, (tt8Bit)0} /* filler */
};

static const ttPppStateFunctPtr  TM_CONST_QLF tlLcpStateFunctTable[] =
{
    (ttPppStateFunctPtr)tfLcpThisLayerStart,    /* 00 */
    (ttPppStateFunctPtr)tfLcpThisLayerFinish,   /* 01 */
    (ttPppStateFunctPtr)tfLcpThisLayerUp,       /* 02 */
    (ttPppStateFunctPtr)tfLcpThisLayerDown,     /* 03 */
    (ttPppStateFunctPtr)tfLcpInitRestartCounter,/* 04 */
    (ttPppStateFunctPtr)tfLcpZeroRestartCounter,/* 05 */
    (ttPppStateFunctPtr)tfLcpSendCfgRequest,    /* 06 */
    (ttPppStateFunctPtr)tfPppSendCfgAck,        /* 07 */
    (ttPppStateFunctPtr)tfLcpSendCfgNak,        /* 08 */
    (ttPppStateFunctPtr)tfLcpSendCodeReject,    /* 09 */
    (ttPppStateFunctPtr)tfLcpSendTermRequest,   /* 10 */
    (ttPppStateFunctPtr)tfLcpSendTermAck,       /* 11 */
    (ttPppStateFunctPtr)tfPppSendEchoReply,     /* 12 */
    (ttPppStateFunctPtr)tfPppStateError         /* 13 */
};

#ifdef TM_USE_IPV4
static const ttPppStateFunctPtr  TM_CONST_QLF tlIpcpStateFunctTable[] =
{
    (ttPppStateFunctPtr)tfIpcpThisLayerStart,    /* 00 */
    (ttPppStateFunctPtr)tfIpcpThisLayerFinish,   /* 01 */
    (ttPppStateFunctPtr)tfIpcpThisLayerUp,       /* 02 */
    (ttPppStateFunctPtr)tfPppThisLayerDown,      /* 03 */
    (ttPppStateFunctPtr)tfIpcpInitRestartCounter,/* 04 */
    (ttPppStateFunctPtr)tfIpcpZeroRestartCounter,/* 05 */
    (ttPppStateFunctPtr)tfIpcpSendCfgRequest,    /* 06 */
    (ttPppStateFunctPtr)tfPppSendCfgAck,         /* 07 */
    (ttPppStateFunctPtr)tfIpcpSendCfgNak,        /* 08 */
    (ttPppStateFunctPtr)tfIpcpSendCodeReject,    /* 09 */
    (ttPppStateFunctPtr)tfLcpSendTermRequest,    /* 10 */
    (ttPppStateFunctPtr)tfIpcpSendTermAck,        /* 11 */
    (ttPppStateFunctPtr)tfPppSendEchoReply,      /* 12 */
    (ttPppStateFunctPtr)tfPppStateError          /* 13 */
};
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
static const ttPppStateFunctPtr  TM_CONST_QLF tlIp6cpStateFunctTable[] =
{
    (ttPppStateFunctPtr)tfIp6cpThisLayerStart,     /* 00 */
    (ttPppStateFunctPtr)tfIp6cpThisLayerFinish,    /* 01 */
    (ttPppStateFunctPtr)tfIp6cpThisLayerUp,        /* 02 */
    (ttPppStateFunctPtr)tfPppThisLayerDown,        /* 03 */
    (ttPppStateFunctPtr)tfIp6cpInitRestartCounter, /* 04 */
    (ttPppStateFunctPtr)tfIp6cpZeroRestartCounter, /* 05 */
    (ttPppStateFunctPtr)tfIp6cpSendCfgRequest,     /* 06 */
    (ttPppStateFunctPtr)tfPppSendCfgAck,           /* 07 */
    (ttPppStateFunctPtr)tfIp6cpSendCfgNak,         /* 08 */
    (ttPppStateFunctPtr)tfIp6cpSendCodeReject,     /* 09 */
    (ttPppStateFunctPtr)tfLcpSendTermRequest,      /* 10 */
    (ttPppStateFunctPtr)tfIp6cpSendTermAck,        /* 11 */
    (ttPppStateFunctPtr)tfPppSendEchoReply,        /* 12 */
    (ttPppStateFunctPtr)tfPppStateError            /* 13 */
};
#endif /* TM_USE_IPV6 */

static const ttPppStateFunctPtr  TM_CONST_QLF tlPapStateFunctTable[] =
{
    (ttPppStateFunctPtr)tfPapThisLayerStart,        /* 00 */
    (ttPppStateFunctPtr)tfPapThisLayerFinish,       /* 01 */
    (ttPppStateFunctPtr)tfPapThisLayerUp,           /* 02 */
    (ttPppStateFunctPtr)tfAuthThisLayerDown,        /* 03 */
    (ttPppStateFunctPtr)tfPapInitRestartCounter,    /* 04 */
    (ttPppStateFunctPtr)tfPapZeroRestartCounter,    /* 05 */
    (ttPppStateFunctPtr)tfPapSendAuthRequest,       /* 06 */
    (ttPppStateFunctPtr)tfPppSendCfgAck,            /* 07 */
    (ttPppStateFunctPtr)tfPapSendAuthNak,           /* 08 */
    (ttPppStateFunctPtr)tfPapSendCodeReject,        /* 09 */
    (ttPppStateFunctPtr)tfLcpSendTermRequest,       /* 10 */
    (ttPppStateFunctPtr)tfPapSendTermAck,           /* 11 */
    (ttPppStateFunctPtr)tfPppSendEchoReply,         /* 12 */
    (ttPppStateFunctPtr)tfPppStateError             /* 13 */
};

static const ttPppStateFunctPtr  TM_CONST_QLF tlChapStateFunctTable[] =
{
    (ttPppStateFunctPtr)tfChapThisLayerStart,       /* 00 */
    (ttPppStateFunctPtr)tfChapThisLayerFinish,      /* 01 */
    (ttPppStateFunctPtr)tfChapThisLayerUp,          /* 02 */
    (ttPppStateFunctPtr)tfAuthThisLayerDown,        /* 03 */
    (ttPppStateFunctPtr)tfChapInitRestartCounter,   /* 04 */
    (ttPppStateFunctPtr)tfChapZeroRestartCounter,   /* 05 */
    (ttPppStateFunctPtr)tfChapSendChallenge,        /* 06 */
    (ttPppStateFunctPtr)tfChapSendSuccess,          /* 07 */
    (ttPppStateFunctPtr)tfChapSendFailure,          /* 08 */
    (ttPppStateFunctPtr)tfChapSendCodeReject,       /* 09 */
    (ttPppStateFunctPtr)tfLcpSendTermRequest,       /* 10 */
    (ttPppStateFunctPtr)tfChapSendTermAck,           /* 11 */
    (ttPppStateFunctPtr)tfPppSendEchoReply,         /* 12 */
    (ttPppStateFunctPtr)tfPppStateError             /* 13 */
};

static const ttPppStateFunctEntry  TM_CONST_QLF tlPppStateFunctTable[] =
{
    {TM_PPPM_LAYER_START,      TM_PPP_FN_LAYER_START    }, /* 00 */
    {TM_PPPM_LAYER_STOP,       TM_PPP_FN_LAYER_FINISH   }, /* 01 */
    {TM_PPPM_LAYER_UP,         TM_PPP_FN_LAYER_UP       }, /* 02 */
    {TM_PPPM_LAYER_UP_ACK,     TM_PPP_FN_SEND_CFG_ACK   }, /* 03 */
    {TM_PPPM_LAYER_UP_ACK,     TM_PPP_FN_LAYER_UP       }, /* 03 */
    {TM_PPPM_LAYER_DOWN,       TM_PPP_FN_LAYER_DOWN     }, /* 05 */
    {TM_PPPM_INIT_RESTART_CNT, TM_PPP_FN_INIT_RESTART   }, /* 06 */
    {TM_PPPM_1SND_CFG_REQ,     TM_PPP_FN_INIT_RESTART   }, /* 07 */
    {TM_PPPM_1SND_CFG_REQ,     TM_PPP_FN_SEND_CFG_REQ   }, /* 07 */
    {TM_PPPM_OSND_CFG_REQ,     TM_PPP_FN_LAYER_DOWN     }, /* 09 */
    {TM_PPPM_OSND_CFG_REQ,     TM_PPP_FN_SEND_CFG_REQ   }, /* 09 */
    {TM_PPPM_RSND_CFG_REQ,     TM_PPP_FN_SEND_CFG_REQ   }, /* 11 */
    {TM_PPPM_SND_CFG_ACK,      TM_PPP_FN_SEND_CFG_ACK   }, /* 12 */
    {TM_PPPM_SND_CFG_NAK,      TM_PPP_FN_SEND_CFG_NAK   }, /* 13 */
    {TM_PPPM_SND_CODE_REJECT,  TM_PPP_FN_SEND_COD_REJ   }, /* 14 */
    {TM_PPPM_1SND_CFG_REQ_ACK, TM_PPP_FN_INIT_RESTART   }, /* 15 */
    {TM_PPPM_1SND_CFG_REQ_ACK, TM_PPP_FN_SEND_CFG_REQ   }, /* 15 */
    {TM_PPPM_1SND_CFG_REQ_ACK, TM_PPP_FN_SEND_CFG_ACK   }, /* 15 */
    {TM_PPPM_OSND_CFG_REQ_ACK, TM_PPP_FN_LAYER_DOWN     }, /* 18 */
    {TM_PPPM_OSND_CFG_REQ_ACK, TM_PPP_FN_SEND_CFG_REQ   }, /* 18 */
    {TM_PPPM_OSND_CFG_REQ_ACK, TM_PPP_FN_SEND_CFG_ACK   }, /* 18 */
    {TM_PPPM_1SND_CFG_REQ_NAK, TM_PPP_FN_INIT_RESTART   }, /* 21 */
    {TM_PPPM_1SND_CFG_REQ_NAK, TM_PPP_FN_SEND_CFG_REQ   }, /* 21 */
    {TM_PPPM_1SND_CFG_REQ_NAK, TM_PPP_FN_SEND_CFG_NAK   }, /* 21 */
    {TM_PPPM_OSND_CFG_REQ_NAK, TM_PPP_FN_SEND_CFG_REQ   }, /* 24 */
    {TM_PPPM_OSND_CFG_REQ_NAK, TM_PPP_FN_SEND_CFG_NAK   }, /* 24 */
    {TM_PPPM_1SND_TERM_REQ,    TM_PPP_FN_INIT_RESTART   }, /* 26 */
    {TM_PPPM_1SND_TERM_REQ,    TM_PPP_FN_SEND_TRM_REQ   }, /* 26 */
    {TM_PPPM_OSND_TERM_REQ,    TM_PPP_FN_LAYER_DOWN     }, /* 28 */
    {TM_PPPM_OSND_TERM_REQ,    TM_PPP_FN_INIT_RESTART   }, /* 28 */
    {TM_PPPM_OSND_TERM_REQ,    TM_PPP_FN_SEND_TRM_REQ   }, /* 28 */
    {TM_PPPM_RSND_TERM_REQ,    TM_PPP_FN_SEND_TRM_REQ   }, /* 31 */
    {TM_PPPM_SND_TERM_ACK,     TM_PPP_FN_SEND_TRM_ACK   }, /* 32 */
    {TM_PPPM_OSND_TERM_ACK,    TM_PPP_FN_LAYER_DOWN     }, /* 33 */
    {TM_PPPM_OSND_TERM_ACK,    TM_PPP_FN_SEND_TRM_ACK   }, /* 33 */
    {TM_PPPM_OSND_TERM_ACK,    TM_PPP_FN_ZERO_RESTART   }, /* 33 */
    {TM_PPPM_SND_ECHO_REPLY,   TM_PPP_FN_SEND_ECHO      }, /* 36 */
    {TM_PPPM_TRANSITION_ERROR, TM_PPP_FN_ERROR          }, /* 37 */
    {TM_PPPM_LAST_INDEX,       255                      }  /* 38 */
};

/* Our procedure for the state machine is that.
 * 1) For a given state and given event, look up the tlPppTransitionMatrix
 *    to find the next state, and the action procedure index we should
 *    take. Action Procedure Index looks like TM_PPPM_xxxx
 * 2) Using that action procedure index number and look up
 *    tlPppStateFunctTable, to get all the function indices (TM_PPP_FN_***)
 *    which we should call one by one.
 * 3) Using function index TM_PPP_FN_* to call function
 *    tlPap(Lcp, Ipcp)StateFunctTable[TM_PPP_FN_*]
 */

static const ttTransitionMatrixEntry  TM_CONST_QLF tlPppTransitionMatrix
                                [10][16] =
{
    {/* State == TM_PPPS_INITIAL */
    /*  Event                     Next State       Action Procedure Index */
    {/* TM_PPPE_UP             */ TM_PPPS_CLOSED,  TM_MATRIX_NOOP          },
    {/* TM_PPPE_DOWN,          */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_OPEN,          */ TM_PPPS_STARTING,TM_PPPM_LAYER_START     },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_INITIAL, TM_MATRIX_NOOP          },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_INITIAL, TM_PPPM_TRANSITION_ERROR}
    },

    {/* State == TM_PPPS_STARTING */
    /*  Event                     Next State       Action Procedure Index  */
    {/* TM_PPPE_UP,            */ TM_PPPS_REQ_SENT,TM_PPPM_1SND_CFG_REQ    },
    {/* TM_PPPE_DOWN,          */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_OPEN,          */ TM_PPPS_STARTING,TM_MATRIX_NOOP          },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_INITIAL, TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_STARTING,TM_PPPM_TRANSITION_ERROR}
    },

    {/* State == TM_PPPS_CLOSED */
    /*  Event                     Next State       Action Procedure Index  */
    {/* TM_PPPE_UP,            */ TM_PPPS_CLOSED,  TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_DOWN,          */ TM_PPPS_INITIAL, TM_MATRIX_NOOP          },
    {/* TM_PPPE_OPEN,          */ TM_PPPS_REQ_SENT,TM_PPPM_1SND_CFG_REQ    },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_CLOSED,  TM_MATRIX_NOOP          },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_CLOSED,  TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_CLOSED,  TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_CLOSED,  TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_CLOSED,  TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_CLOSED,  TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_CLOSED,  TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_CLOSED,  TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_CLOSED,  TM_MATRIX_NOOP          },
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_CLOSED,  TM_PPPM_SND_CODE_REJECT },
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_CLOSED,  TM_MATRIX_NOOP          },
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_CLOSED,  TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_CLOSED,  TM_PPPM_TRANSITION_ERROR}
    },

    {/* State == TM_PPPS_STOPPED */
    /*  Event                     Next State       Action Procedure Index  */
    {/* TM_PPPE_UP,            */ TM_PPPS_STOPPED, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_DOWN,          */ TM_PPPS_STARTING,TM_PPPM_LAYER_START     },
    {/* TM_PPPE_OPEN,          */ TM_PPPS_STOPPED, /*R*/TM_MATRIX_NOOP     },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_CLOSED,  TM_MATRIX_NOOP          },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_STOPPED, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_STOPPED, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_ACK_SENT,TM_PPPM_1SND_CFG_REQ_ACK},
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_REQ_SENT,TM_PPPM_1SND_CFG_REQ_NAK},
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_STOPPED, TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_STOPPED, TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_STOPPED, TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_STOPPED, TM_MATRIX_NOOP          },
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_STOPPED, TM_PPPM_SND_CODE_REJECT },
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_STOPPED, TM_MATRIX_NOOP          },
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_STOPPED, TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_STOPPED, TM_PPPM_TRANSITION_ERROR}
    },

    {/* State == TM_PPPS_CLOSING */
    /*  Event                     Next State       Action Procedure Index  */
    {/* TM_PPPE_UP,            */ TM_PPPS_CLOSING, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_DOWN,          */ TM_PPPS_INITIAL, TM_MATRIX_NOOP          },
    {/* TM_PPPE_OPEN,          */ TM_PPPS_STOPPING,/*R*/TM_MATRIX_NOOP     },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_CLOSING, TM_MATRIX_NOOP          },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_CLOSING, TM_PPPM_RSND_TERM_REQ   },
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_CLOSED,  TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_CLOSING, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_CLOSING, TM_MATRIX_NOOP          },
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_CLOSING, TM_MATRIX_NOOP          },
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_CLOSING, TM_MATRIX_NOOP          },
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_CLOSING, TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_CLOSED,  TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_CLOSING, TM_PPPM_SND_CODE_REJECT },
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_CLOSING, TM_MATRIX_NOOP          },
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_CLOSED,  TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_CLOSING, TM_PPPM_TRANSITION_ERROR}
    },

    {/* State == TM_PPPS_STOPPING */
    /*  Event                     Next State       Action Procedure Index  */
    {/* TM_PPPE_UP,            */ TM_PPPS_STOPPING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_DOWN,          */ TM_PPPS_STARTING,TM_MATRIX_NOOP          },
    {/* TM_PPPE_OPEN,          */ TM_PPPS_STOPPING,/*R*/TM_MATRIX_NOOP     },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_CLOSING, TM_MATRIX_NOOP          },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_STOPPING,TM_PPPM_RSND_TERM_REQ   },
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_STOPPED, TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_STOPPING,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_STOPPING,TM_MATRIX_NOOP          },
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_STOPPING,TM_MATRIX_NOOP          },
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_STOPPING,TM_MATRIX_NOOP          },
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_STOPPING,TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_STOPPED, TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_STOPPING,TM_PPPM_SND_CODE_REJECT },
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_STOPPING,TM_MATRIX_NOOP          },
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_STOPPED, TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_STOPPING,TM_PPPM_TRANSITION_ERROR}
    },

    {/* State == TM_PPPS_REQ_SENT */
    /*  Event                     Next State       Action Procedure Index  */
    {/* TM_PPPE_UP,            */ TM_PPPS_REQ_SENT,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_DOWN,          */ TM_PPPS_STARTING,TM_MATRIX_NOOP          },
    {/* TM_PPPE_OPEN,          */ TM_PPPS_REQ_SENT,TM_MATRIX_NOOP          },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_CLOSING, TM_PPPM_1SND_TERM_REQ   },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_REQ_SENT,TM_PPPM_RSND_CFG_REQ    },
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_STOPPED, /*P*/TM_PPPM_LAYER_STOP },
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_ACK_SENT,TM_PPPM_SND_CFG_ACK     },
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_REQ_SENT,TM_PPPM_SND_CFG_NAK     },
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_ACK_REC, TM_PPPM_INIT_RESTART_CNT},
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_REQ_SENT,TM_PPPM_1SND_CFG_REQ    },
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_REQ_SENT,TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_REQ_SENT,TM_MATRIX_NOOP          },
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_REQ_SENT,TM_PPPM_SND_CODE_REJECT },
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_REQ_SENT,TM_MATRIX_NOOP          },
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_STOPPED, TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_REQ_SENT,TM_PPPM_TRANSITION_ERROR}
    },

    {/* State == TM_PPPS_ACK_REC */
    /*  Event                     Next State       Action Procedure Index  */
    {/* TM_PPPE_UP,            */ TM_PPPS_ACK_REC, TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_DOWN,          */ TM_PPPS_STARTING,TM_MATRIX_NOOP          },
    {/* TM_PPPE_OPEN,          */ TM_PPPS_ACK_REC, TM_MATRIX_NOOP          },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_CLOSING, TM_PPPM_1SND_TERM_REQ   },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_REQ_SENT,TM_PPPM_RSND_CFG_REQ    },
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_STOPPED, /*P*/TM_PPPM_LAYER_STOP },
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_OPENED,  TM_PPPM_LAYER_UP_ACK    },
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_ACK_REC, TM_PPPM_SND_CFG_NAK     },
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_REQ_SENT,/*X*/TM_PPPM_RSND_CFG_REQ},
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_REQ_SENT,/*X*/TM_PPPM_RSND_CFG_REQ},
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_REQ_SENT,TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_REQ_SENT,TM_MATRIX_NOOP          },
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_ACK_REC, TM_PPPM_SND_CODE_REJECT },
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_REQ_SENT,TM_MATRIX_NOOP          },
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_STOPPED, TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_ACK_REC, TM_PPPM_TRANSITION_ERROR}
    },

    {/* State == TM_PPPS_ACK_SENT */
    /*  Event                     Next State       Action Procedure Index  */
    {/* TM_PPPE_UP,            */ TM_PPPS_ACK_SENT,TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_DOWN,          */ TM_PPPS_STARTING,TM_MATRIX_NOOP          },
    {/* TM_PPPE_OPEN,          */ TM_PPPS_ACK_SENT,TM_MATRIX_NOOP          },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_CLOSING, TM_PPPM_1SND_TERM_REQ   },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_ACK_SENT,TM_PPPM_RSND_CFG_REQ    },
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_STOPPED, /*P*/TM_PPPM_LAYER_STOP },
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_ACK_SENT,TM_PPPM_SND_CFG_ACK     },
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_REQ_SENT,TM_PPPM_SND_CFG_NAK     },
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_OPENED,  TM_PPPM_LAYER_UP        },
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_ACK_SENT,TM_PPPM_1SND_CFG_REQ    },
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_REQ_SENT,TM_PPPM_SND_TERM_ACK    },
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_ACK_SENT,TM_MATRIX_NOOP          },
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_ACK_SENT,TM_PPPM_SND_CODE_REJECT },
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_ACK_SENT,TM_MATRIX_NOOP          },
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_STOPPED, TM_PPPM_LAYER_STOP      },
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_ACK_SENT,TM_PPPM_TRANSITION_ERROR}
    },

    {/* State == TM_PPPS_OPENED */
    /*  Event                     Next State       Action Procedure Index  */
    {/* TM_PPPE_UP,            */ TM_PPPS_OPENED,  TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_DOWN,          */ TM_PPPS_STARTING,TM_PPPM_LAYER_DOWN      },
    {/* TM_PPPE_OPEN,          */ TM_PPPS_OPENED,  /*R*/TM_MATRIX_NOOP     },
    {/* TM_PPPE_CLOSE,         */ TM_PPPS_CLOSING, TM_PPPM_OSND_TERM_REQ   },
    {/* TM_PPPE_TIMEOUT,       */ TM_PPPS_OPENED,  TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_LAST_TIMEOUT,  */ TM_PPPS_OPENED,  TM_PPPM_TRANSITION_ERROR},
    {/* TM_PPPE_GOOD_CFG_REQ,  */ TM_PPPS_ACK_SENT,TM_PPPM_OSND_CFG_REQ_ACK},
    {/* TM_PPPE_BAD_CFG_REQ,   */ TM_PPPS_REQ_SENT,TM_PPPM_OSND_CFG_REQ_NAK},
    {/* TM_PPPE_CFG_ACK,       */ TM_PPPS_OPENED,  /*X*/TM_MATRIX_NOOP     },
    {/* TM_PPPE_CFG_NAK,       */ TM_PPPS_OPENED,  /*X*/TM_MATRIX_NOOP     },
    {/* TM_PPPE_TRM_REQ,       */ TM_PPPS_STOPPING,TM_PPPM_OSND_TERM_ACK   },
    {/* TM_PPPE_TRM_ACK,       */ TM_PPPS_REQ_SENT,TM_PPPM_OSND_CFG_REQ    },
    {/* TM_PPPE_UNKNOWN_CODE,  */ TM_PPPS_OPENED,  TM_PPPM_SND_CODE_REJECT },
    {/* TM_PPPE_CODE_PRO_REJ,  */ TM_PPPS_OPENED,  TM_MATRIX_NOOP          },
    {/* TM_PPPE_CODE_PRO_ERROR,*/ TM_PPPS_STOPPING,TM_PPPM_OSND_TERM_REQ   },
    {/* TM_PPPE_ECHO_DIS_REQP, */ TM_PPPS_OPENED,  TM_PPPM_SND_ECHO_REPLY  }
    }
};

/*
 * Structure to map flags to VJ options with
 * padded length of option
 * "*" is padded fields (to length)
 */
#ifdef TM_USE_IPV4
static const struct
{
    tt8Bit flag;
    tt8Bit length;
} TM_CONST_QLF tlVjFlags[] = { {0x80,0}, /* flags      00 */
                               {0x40,1}, /* Connection 01**/
                               {0x80,1}, /* TCP CHKSUM 02 */
                               {0x80,1}, /* continuted 03 */
                               {0x01,4}, /* Urgent     04**/
                               {0x01,1}, /* continuted 05 */
                               {0x01,1}, /* continuted 06 */
                               {0x01,1}, /* continuted 07 */
                               {0x02,4}, /* Window     08**/
                               {0x02,1}, /* continuted 09 */
                               {0x02,1}, /* continuted 10 */
                               {0x02,1}, /* continuted 11 */
                               {0x04,4}, /* Ack        12**/
                               {0x04,1}, /* continuted 13 */
                               {0x04,1}, /* continuted 14 */
                               {0x04,1}, /* continuted 15 */
                               {0x08,4}, /* Sequence   16**/
                               {0x08,1}, /* continuted 17 */
                               {0x08,1}, /* continuted 18 */
                               {0x08,1}, /* continuted 19 */
                               {0x20,4}, /* Ident      20**/
                               {0x20,1}, /* continuted 21 */
                               {0x20,1}, /* continuted 22 */
                               {0x20,1}, /* continuted 23 */
                               {0x00,16},/* padding    24 */
                               {0x00,1}, /* continued  25 */
                               {0x00,1}, /* continued  26 */
                               {0x00,1}, /* continued  27 */
                               {0x00,1}, /* continued  28 */
                               {0x00,1}, /* continued  29 */
                               {0x00,1}, /* continued  30 */
                               {0x00,1}, /* continued  31 */
                               {0x00,1}, /* continued  32 */
                               {0x00,1}, /* continued  33 */
                               {0x00,1}, /* continued  34 */
                               {0x00,1}, /* continued  35 */
                               {0x00,1}, /* continued  36 */
                               {0x00,1}, /* continued  37 */
                               {0x00,1}, /* continued  38 */
                               {0x00,1}  /* continued  39 */
};
#endif /* TM_USE_IPV4 */

/******************************************
 *
 *
 *
 * PPP Main Routines
 *
 *
 *
 *****************************************/

/*DESCRIPTION:
 * User calls this routine to set LCP authentication method priority. The priority value
 * can be any integar between 0x01 and 0x0f (which can fit in a 4-bit range).
 * If the priority value ties, the first match will be used.
 * If user forgets to set priority value, the default value will be used. The
 * default priority sequence is:
 * EAP       (No.1)
 * CHAP      (No.2)
 * PAP       (No.3)
 * MSCHAP_v1 (No.4)
 *PARAMETER:
 * authMethod:   can be one of the following values
 * TM_PPP_AUTHMETHOD_CHAP
 * TM_PPP_AUTHMETHOD_PAP
 * TM_PPP_AUTHMETHOD_MSCHAP_V1
 * priorityValue: Any integar between 0x01 and 0x0f.
 * USAGE:
 * For example, if user prefers the following priority sequence:
 * MSCHAPv1-------CHAP-----PAP, then user may call
 * error = tfPppSetAuthPriority(TM_PPP_AUTHMETHOD_MSCHAP_V1, 1);
 * error = tfPppSetAuthPriority(TM_PPP_AUTHMETHOD_CHAP, 2);
 * error = tfPppSetAuthPriority(TM_PPP_AUTHMETHOD_PAP, 3);
 * RETURN:
 * TM_ENOERROR       successful return
 * TM_EINVAL         invalid parameter
 */


int tfPppSetAuthPriority(ttUser8Bit     authMethod,
                         ttUser8Bit     priorityValue)
{
    int      errorCode;

    errorCode = TM_ENOERROR;

    if( (authMethod == TM_PPP_AUTHMETHOD_MIN) ||
        (priorityValue == TM_PPP_AUTHPRIORITY_MIN) ||
        (authMethod > TM_PPP_AUTHMETHOD_MAX) ||
        (priorityValue > TM_PPP_AUTHPRIORITY_MAX))
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_context(tvPppAuthPriority) |=
            (priorityValue << ((authMethod - 1) * TM_PPP_AUTHPRIORITY_BITS));
    }

    return errorCode;

}


/*
 * Used to initialize the link layer in the link layer list
 * for a PPP *client*
 */

ttUserLinkLayer tfUseAsyncPpp(ttLnkNotifyFuncPtr notifyFuncPtr)
{
    return tfPppAsyncInit(notifyFuncPtr, TM_PPP_CLIENT_LINK);
}

/*
 * Used to initialize the link layer in the link layer list
 *   for a PPP *server*
 */

ttUserLinkLayer tfUseAsyncServerPpp(ttLnkNotifyFuncPtr notifyFuncPtr)
{
    return tfPppAsyncInit(notifyFuncPtr, TM_PPP_SERVER_LINK);
}


static ttUserLinkLayer tfPppAsyncInit ( ttLnkNotifyFuncPtr notifyFuncPtr,
                                        tt8Bit             pppLinkLayer )
{
    ttLinkLayerEntryPtr   linkLayerEntryPtr;
    ttCharPtr             pppAsyncIdPtr;
    int                   isEqual;

    if (pppLinkLayer == TM_PPP_CLIENT_LINK)
    {
        pppAsyncIdPtr = (ttCharPtr)tlPppAsyncId;
    }
    else
    {
        pppAsyncIdPtr = (ttCharPtr)tlPppAsyncServerId;
    }
/* Search for PPP Protocol in Link Layer List */
/* Lock the Link Layer list */
    tm_call_lock_wait(&tm_context(tvLinkLayerListLock));
    linkLayerEntryPtr = tm_context(tvLinkLayerListPtr);
/* Check for duplicate */
    while (linkLayerEntryPtr != TM_LINK_NULL_PTR)
    {
        isEqual = tm_strcmp( (ttCharPtr)(linkLayerEntryPtr->lnkNameArray),
                             pppAsyncIdPtr );
        if( isEqual == TM_STRCMP_EQUAL )
        {
            goto pppAsyncExit;
        }
        linkLayerEntryPtr = linkLayerEntryPtr->lnkNextPtr;
    }
/* if none exists then malloc an entry */
    linkLayerEntryPtr = (ttLinkLayerEntryPtr)tm_kernel_malloc((unsigned)
                                               sizeof(ttLinkLayerEntry));
    if (linkLayerEntryPtr == TM_LINK_NULL_PTR)
    {
/* if the malloc fails then return an error */
        goto pppAsyncExit;
    }
    tm_bzero(linkLayerEntryPtr, sizeof(ttLinkLayerEntry));
    linkLayerEntryPtr->lnkMtu = TM_PPP_DEF_MRU;
    if (pppLinkLayer == TM_PPP_CLIENT_LINK)
    {
        linkLayerEntryPtr->lnkOpenFuncPtr = tfPppClientOpen;
    }
    else
    {
        linkLayerEntryPtr->lnkOpenFuncPtr = tfPppServerOpen;
    }
    linkLayerEntryPtr->lnkCloseFuncPtr = tfPppClose;
    linkLayerEntryPtr->lnkSendFuncPtr = tfPppSendPacket;
    linkLayerEntryPtr->lnkRecvFuncPtr = tfPppAsyncRecv;
    linkLayerEntryPtr->lnkIoctlFuncPtr = tfPppIoctl;
    linkLayerEntryPtr->lnkErrorFuncPtr = tfPppError;
    linkLayerEntryPtr->lnkNotifyFuncPtr = notifyFuncPtr;
    linkLayerEntryPtr->lnkPostFuncPtr = tfPppStuffPacket;
    linkLayerEntryPtr->lnkHeaderLength = TM_PPP_HDR_BYTES;
    linkLayerEntryPtr->lnkLinkLayerProtocol = TM_LINK_LAYER_PPP;
    tm_strcpy( (ttCharPtr)(linkLayerEntryPtr->lnkNameArray),
               pppAsyncIdPtr );
/* Stuff it into the table */
    linkLayerEntryPtr->lnkNextPtr = tm_context(tvLinkLayerListPtr);
    tm_context(tvLinkLayerListPtr) = linkLayerEntryPtr;

pppAsyncExit:
    tm_call_unlock(&tm_context(tvLinkLayerListLock));
    return (ttUserLinkLayer)linkLayerEntryPtr;
/* return the link layer pointer */
}

/*
 * Used to set options for PPP on a particular device
 * API To user to allow them to configure what parameters that they wish
 * to use for a device.
 * Must be called before configure interface, because the state machine
 * is started when config interface is called.
 * If multiple authentication protocols are allowed, then we go from the
 * strongest to the weakest allowed
 *
 * This function only needs to be called if the user wishes to deviate from
 * the default setup
 */
int tfPppSetOption (ttUserInterface    interfaceId,
                    int                protocolLevel,
                    int                remoteLocalFlag,
                    int                optionName,
                    ttConstCharPtr     optionValuePtr,
                    int                optionLength)
{

    ttDeviceEntryPtr devPtr;
    ttPppVectPtr     pppVectPtr;
    ttPapOptionsPtr  papInfoPtr;
    ttChapOptionsPtr chapInfoPtr;
    ttLcpStatePtr    pppLcpStatePtr;
#ifdef TM_USE_PPP_CALLBACK
    ttCallbackStatePtr      cbcpStatePtr;
    tt8BitPtr               bytePtr;
    unsigned int            index;
    unsigned int            length;
#endif /* TM_USE_PPP_CALLBACK */
    tt32Bit          optionType;
    tt32Bit          temp32;
    int              errorCode;
    tt16Bit          temp16;
    tt8Bit           temp8;

    optionType = (tt32Bit)optionName;

    devPtr = (ttDeviceEntryPtr)interfaceId;
    errorCode = tfValidInterface(devPtr);
    if (     (optionValuePtr != (ttConstCharPtr)0)
         &&  (optionLength >= 0)
         &&  (errorCode == TM_ENOERROR) )
    {
        tm_call_lock_wait(&(devPtr->devLockEntry));
        if ( !tm_ll_uses_ppp(devPtr) )
        {
            errorCode = TM_EPERM;
            goto SetOptionEnd;
        }
/* Did the user call us before? */
        pppVectPtr = (ttPppVectPtr)(devPtr->devStateVectorPtr);
        if (pppVectPtr == TM_PPP_VECT_NULL_PTR)
        {
/* If not we need to create a PPP State Vector here */
            pppVectPtr = tfPppVectAlloc(devPtr);
            if ( pppVectPtr == TM_PPP_VECT_NULL_PTR )
            {
                errorCode = TM_ENOBUFS;
                goto SetOptionEnd;
            }
        }
        pppLcpStatePtr = &pppVectPtr->pppsLcpState;

        temp8 = *((tt8BitPtr)(optionValuePtr));
        temp16 = (tt16Bit)0;
        temp32 = TM_UL(0);
        if ((unsigned)optionLength >= sizeof(tt32Bit))
        {
            tm_bcopy(optionValuePtr, &temp32, sizeof(tt32Bit));
        }
        if ((unsigned)optionLength >= sizeof(tt16Bit))
        {
            tm_bcopy(optionValuePtr, &temp16, sizeof(tt16Bit));
        }

        if (protocolLevel == (int)TM_PPP_LCP_PROTOCOL)
        {

/* Check the PPP option for correctness and set it if it is correct */
            switch (optionName)
            {
/*
 * The following are Boolean Options so we just turn them
 * on and off
 */
/* Protocol Field Compression */
                case TM_LCP_PROTOCOL_COMP:
/* No Break Because both options are processed the same */

/* Address/Control Field Compression */
                case TM_LCP_ADDRCONTROL_COMP:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (temp8 == (tt8Bit)0)
                        {
/* Turn it Off */
                            if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                            {
/* Don't let the remote use this option */
                                pppLcpStatePtr->
                                    lcpsRemoteNegotiateOption &=
                                    ~(TM_UL(1) <<(tt8Bit)optionType);
                            }
                            else
                            {
/* We dont want to use this option */
                                pppLcpStatePtr->
                                    lcpsLocalNegotiateOption &=
                                    ~(TM_UL(1)<<(tt8Bit)optionType);
                            }
                        }
                        else
                        {
/* Turn it On */
#ifdef TM_USE_PPPOE
                            if (tm_ll_is_pppoe(devPtr))
                            {
/*
 * PPPoE must not request, and must reject ACFC, and protocol field
 * compression is not recommended
 */
                                errorCode = TM_EPERM;
                            }
                            else
#endif /* TM_USE_PPPOE */
                            {
                                if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                                {
                                    pppLcpStatePtr->
                                        lcpsRemoteNegotiateOption |=
                                        (TM_UL(1)<<(tt8Bit)optionType);
                                }
                                else
                                {
                                    pppLcpStatePtr->
                                        lcpsLocalNegotiateOption |=
                                        (TM_UL(1)<<(tt8Bit)optionType);
                                }
                            }
                        }
                    }
                    break;
/* Magic Number */
                case TM_LCP_MAGIC_NUMBER:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (temp8 == (tt8Bit)0)
                        {
/* Turn it Off */
                            if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                            {
/* Don't let the remote use this option */
                                pppLcpStatePtr->
                                    lcpsRemoteNegotiateOption &=
                                    ~(TM_UL(1)<<(tt8Bit)optionType);
                            }
                            else
                            {
/* We dont want to use this option */
                                pppLcpStatePtr->
                                    lcpsLocalNegotiateOption &=
                                    ~(TM_UL(1)<<(tt8Bit)optionType);
                            }
                        }
                        else
                        {
/* Turn it On */
                            if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                            {
                                pppLcpStatePtr->
                                    lcpsRemoteNegotiateOption |=
                                    (TM_UL(1)<<(tt8Bit)optionType);
                            }
                            else
                            {
                                temp32 = tfGetRandom();
                                pppLcpStatePtr->lcpsLocalWantOptions.
                                    lcpMagicNumber = temp32;
                                pppLcpStatePtr->
                                    lcpsLocalNegotiateOption |=
                                    (TM_UL(1)<<(tt8Bit)optionType);
                            }
                        }
                    }
                    break;

#ifdef TM_PPP_LQM
/* Quality Protocol (LQR timer period for Link-Quality-Report) */
            case TM_LCP_QUALITY_PROTOCOL:
                if ((unsigned)optionLength < sizeof(tt32Bit))
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
                    if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                    {
/* allow remote to request that we use Link-Quality-Report quality protocol
   with the specified Reporting-Period */
                        pppLcpStatePtr->
                            lcpsRemoteNegotiateOption |=
                            (TM_UL(1)<<(tt8Bit)optionType);
                        pppLcpStatePtr->
                            lcpsRemoteAllowOptions.lqmReportingPeriod =
                            temp32;
                    }
/* we want remote to use Link-Quality-Report quality protocol with the
   specified Reporting-Period */
                    else
                    {
                        pppLcpStatePtr->
                            lcpsLocalNegotiateOption |=
                            (TM_UL(1)<<(tt8Bit)optionType);
                        pppLcpStatePtr->
                            lcpsLocalWantOptions.lqmReportingPeriod =
                            temp32;
                    }
                }
                break;
#endif /* TM_PPP_LQM */

/*
 * Max Receive Unit - for IPv6 mode, the MRU must not be set to less than
 * 1280.
 */
                case TM_LCP_MAX_RECV_UNIT:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
/* The smallest that we will allow Default = 1500  (1492 for PPPoE.)*/
                            pppLcpStatePtr->
                                lcpsRemoteAllowOptions.lcpMaxReceiveUnit =
                                    temp16;
                            pppLcpStatePtr->
                                lcpsRemoteNegotiateOption |=
                                (TM_UL(1) <<(tt8Bit)optionType);
                        }
                        else
                        {
/* The largest that we will use */
                            pppLcpStatePtr->
                                lcpsLocalWantOptions.lcpMaxReceiveUnit =
                                    temp16;
                            pppLcpStatePtr->
                                lcpsLocalNegotiateOption |=
                                (TM_UL(1)<<(tt8Bit)optionType);
                        }
                    }
                    break;

/* The Async Control Character Map RFC1662*/
                case TM_LCP_ACCM:
                    if ((unsigned)optionLength < sizeof(tt32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
                            pppLcpStatePtr->
                                lcpsRemoteNegotiateOption |=
                                (TM_UL(1)<<(tt8Bit)optionType);
                        }
                        else
                        {
/* The ACCM we wish to use */
                            pppLcpStatePtr->
                                lcpsLocalWantOptions.lcpAccm = temp32;
                            pppLcpStatePtr->
                                lcpsLocalNegotiateOption |=
                                (TM_UL(1) <<(tt8Bit)optionType);
                        }
                    }
                    break;

/* Set the best PPP Authorization protocol that we can handle */
                case TM_LCP_AUTH_PROTOCOL:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
/* JNS/2-18-00 BUGFIX 485
 *   PPP Authentication protocol option was being stored in the wrong order,
 *   causing the check below to fail, or the auth protocol field to be sent
 *   out in the wrong order.
 */
                        temp16 = ntohs(temp16);
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
/* Check to make sure that this is a valid protocol number */
                            if (   (temp16 == TM_PPP_PAP_HOST_PROTOCOL)
                                || (temp16 == TM_PPP_CHAP_HOST_PROTOCOL)
#ifdef TM_USE_EAP
                                || (temp16 == TM_PPP_EAP_HOST_PROTOCOL)
#endif /* TM_USE_EAP */
                                )
                            {
                                pppLcpStatePtr->
                                    lcpsRemoteNegotiateOption |=
                                    (TM_UL(1)<<(tt8Bit) optionType);
                                pppLcpStatePtr->
                                    lcpsRemoteAllowOptions.
                                    lcpAuthProtocolBits |=
                                    (1 << tfPppGetAuthProtocolBit
/* if set CHAP protocol, it is standard CHAP (MD5) */
                                    (temp16, TM_CHAP_MD5));
                            }
                        }
                        else
                        {
                           pppLcpStatePtr->
                               lcpsLocalNegotiateOption |=
                               (TM_UL(1)<<(tt8Bit)optionType);
                            pppLcpStatePtr->
                                lcpsLocalWantOptions.lcpAuthProtocolBits |=
                                    (1 << tfPppGetAuthProtocolBit
                                    (temp16, TM_CHAP_MD5));
                        }
                    }
                    break;

                case TM_LCP_TERM_RETRY:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppLcpStatePtr->lcpsStateInfo.
                            cpsInitTrmRetryCnt = (tt8Bit)(temp8 + 1);
                    }


                    break;

                case TM_LCP_CONFIG_RETRY:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppLcpStatePtr->lcpsStateInfo.
                            cpsInitCfgRetryCnt = (tt8Bit)(temp8 + 1);
                    }
                    break;

                case TM_LCP_TIMEOUT:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppLcpStatePtr->lcpsStateInfo.
                            cpsRetryTimeoutSeconds = temp8;
                    }
                    break;

/*
 * Max-failures is the maximum number of configuration-NAK packets we'll
 * send in a particular session.  Once this value hits zero, we stop sending
 * NAKs and start rejecting the configuration requests (see RFC1661 p25).
 */
                case TM_LCP_MAX_FAILURES:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppLcpStatePtr->lcpsStateInfo.
                            cpsInitMaxFailures = temp8;
                    }
                    break;

#ifdef TM_USE_PPP_CALLBACK
/* The Callback option is an LCP extension (RFC1570). */
                case TM_LCP_CALLBACK:
                    if (optionLength <= 0)
                    {
                        errorCode = TM_EINVAL;
                    }
                    switch (temp8)
                    {
                    case TM_CALLBACK_OP_DISABLE:
/* Do not callback, i.e. reject requests to configure this option. */
                        pppLcpStatePtr->lcpsLocalNegotiateOption
                                    &= ~(TM_UL(1) << TM_LCP_CALLBACK);
                        pppLcpStatePtr->lcpsRemoteNegotiateOption
                                    &= ~(TM_UL(1) << TM_LCP_CALLBACK);
                        break;

                    case TM_CALLBACK_OP_CBCP:
/* Use Callback Control Protocol (CBCP): i.e. negotiate callback options
 * after LCP and authentication (optional) have completed sucessfully. */
                        pppLcpStatePtr->lcpsRemoteNegotiateOption
                                    |= TM_UL(1) << TM_LCP_CALLBACK;
                        pppLcpStatePtr->lcpsRemoteAllowOptions
                                    .lcpCallbackOpFlags
                                    |= TM_UL(1) << TM_CALLBACK_OP_CBCP;
                        if (remoteLocalFlag == TM_PPP_OPT_WANT)
                        {
                            pppLcpStatePtr->lcpsLocalNegotiateOption
                                    |= TM_UL(1) << TM_LCP_CALLBACK;
                            pppLcpStatePtr->lcpsLocalWantOptions
                                    .lcpCallbackOpFlags
                                    |= TM_UL(1) << TM_CALLBACK_OP_CBCP;
                        }

                        cbcpStatePtr = &pppVectPtr->pppsCallbackState;
                        cbcpStatePtr->cbcpsRole = TM_CBCP_ROLE_DEFAULT;

                        break;
/* No other options are supported at this time. */
                    default:
                        errorCode = TM_EINVAL;
                    }
                    break;
#endif /* TM_USE_PPP_CALLBACK */

                default:
                    errorCode = TM_ENOPROTOOPT;
                    break;
            }
        }
        else

/* set IPCP (IP control protocol) options */
#ifdef TM_USE_IPV4
        if (protocolLevel == (int)TM_PPP_IPCP_PROTOCOL)
        {
            switch (optionName)
            {
                case TM_IPCP_VJ_SLOTS:
                    if (temp8 < 1)
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
/* Number of send slots */
                        {
                            pppVectPtr->pppsIpcpState.
                            ipcpsRemoteAllowOptions.ipcpVjMaxSlots = temp8;
                        }
                        else
/* Number of receive slots */
                        {
                            pppVectPtr->pppsIpcpState.
                                ipcpsLocalWantOptions.ipcpVjMaxSlots = temp8;
                        }
                    }
                    break;
                case TM_IPCP_COMP_PROTOCOL:
/* VJ header compression not supported on DSP platforms, or on PPPoE links */
#ifndef TM_DSP
                    if (tm_ll_is_pppoe(devPtr))
                    {
                        errorCode = TM_EPROTONOSUPPORT;
                    }
                    else
                    {
                        if ((unsigned)optionLength < sizeof(tt16Bit))
                        {
                            errorCode = TM_EINVAL;
                        }
                        else
                        {
#ifdef TM_USE_IPHC
                            if (temp16 == TM_PPP_IPHC_PROTOCOL)
                            {
                                if (pppVectPtr->pppsIphcVectPtr
                                                          == (ttIphcVectPtr)0)
                                {
/* User has not called tfUseIpHdrComp(). */
                                    errorCode = TM_EPROTONOSUPPORT;
                                    break;
                                }
                            }
#endif /* TM_USE_IPHC */
                            if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                            {
/* allow remote to use VJ header compression */
                                pppVectPtr->pppsIpcpState.
                                    ipcpsRemoteNegotiateOption |=
                                    (TM_UL(1)<<(tt8Bit)optionType);
                                pppVectPtr->pppsIpcpState.
                                    ipcpsRemoteAllowOptions.
                                    ipcpCompressProtocol = temp16;
                            }
                            else
                            {
/* Limit size to 2 bytes (RFC1332) */
/* the protocol compression we wish to use */
                                pppVectPtr->pppsIpcpState.
                                  ipcpsLocalWantOptions.ipcpCompressProtocol
                                                                = temp16;
                                pppVectPtr->pppsIpcpState.
                                    ipcpsLocalNegotiateOption |=
                                                 (TM_UL(1)<<(tt8Bit)optionType);
                            }
                        }
                   }
#else /* TM_DSP */
                    errorCode = TM_EPROTONOSUPPORT;
#endif /* TM_DSP */
                    break;

                case TM_IPCP_IP_ADDRESS:
                    if ((unsigned)optionLength < sizeof(tt32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
/* allow remote to set their IP address */
                            pppVectPtr->pppsIpcpState.
                                ipcpsRemoteNegotiateOption |=
                                (TM_UL(1)<<(tt8Bit)optionType);
                            tm_ip_copy(temp32,
                                pppVectPtr->pppsIpcpState.
                                ipcpsRemoteAllowOptions.
                                ipcpIpAddress);
                        }
/* the IP address we wish to use */
                        else
                        {
                            pppVectPtr->pppsIpcpState.
                                ipcpsLocalWantOptions.ipcpIpAddress = temp32;
                            pppVectPtr->pppsIpcpState.
                                ipcpsLocalNegotiateOption |=
                                                (TM_UL(1)<<(tt8Bit)optionType);
                        }
                    }
                    break;

                case TM_IPCP_RETRY:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIpcpState.ipcpsStateInfo.
                            cpsInitCfgRetryCnt = (tt8Bit)(temp8+1);
                    }
                    break;

                case TM_IPCP_TIMEOUT:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIpcpState.ipcpsStateInfo.
                            cpsRetryTimeoutSeconds = temp8;
                    }
                    break;

/*
 * Max-failures is the maximum number of configuration-NAK packets we'll
 * send in a particular session.  Once this value hits zero, we stop sending
 * NAKs and start rejecting the configuration requests (see RFC1661 p25).
 */
                case TM_IPCP_MAX_FAILURES:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIpcpState.ipcpsStateInfo.
                            cpsInitMaxFailures = temp8;
                    }
                    break;

                case TM_IPCP_DNS_PRI:
                    if ((unsigned)optionLength < sizeof(tt32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
/* allow remote to Request their Domain Server */
                            pppVectPtr->pppsIpcpState.
                                ipcpsRemoteNegotiateOption |=
                                (TM_UL(1)<<(tt8Bit)optionType);
                            tm_ip_copy(temp32,
                                pppVectPtr->pppsIpcpState.
                                ipcpsRemoteAllowOptions.
                                ipcpDnsPriAddress);
                        }
/* the Domain Server we wish to use */
                        else
                        {
                            pppVectPtr->pppsIpcpState.
                                ipcpsLocalWantOptions.ipcpDnsPriAddress =
                                                temp32;
                            pppVectPtr->pppsIpcpState.
                                ipcpsLocalNegotiateOption |=
                                                (TM_UL(1)<<(tt8Bit)optionType);
                        }
                    }
                    break;

                case TM_IPCP_DNS_SEC:
                    if ((unsigned)optionLength < sizeof(tt32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
/* allow remote to Request their Domain Server */
                            pppVectPtr->pppsIpcpState.
                                ipcpsRemoteNegotiateOption |=
                                (TM_UL(1)<<(tt8Bit)optionType);
                            tm_ip_copy(temp32,
                                pppVectPtr->pppsIpcpState.
                                ipcpsRemoteAllowOptions.
                                ipcpDnsSecAddress);
                        }
/* the Domain Server we wish to use */
                        else
                        {
                            pppVectPtr->pppsIpcpState.
                                ipcpsLocalWantOptions.ipcpDnsSecAddress =
                                                temp32;
                            pppVectPtr->pppsIpcpState.
                                ipcpsLocalNegotiateOption |=
                                                (TM_UL(1)<<(tt8Bit)optionType);
                        }
                    }
                    break;

#ifdef TM_USE_IPHC
/*
 * IP Header Compression: The maximum number of slots used to store TCP header
 * compression info; this value is determined by the maximum number of
 * concurrent TCP sessions that are expected over this link.
 */
                case TM_IPCP_IPHC_TCP_SPACE:
                    if (((unsigned)optionLength < sizeof(tt16Bit))
                                                    || (temp16 > 255))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
                            pppVectPtr->pppsIpcpState.ipcpsRemoteAllowOptions.
                                ipcpCompTcpSpace = temp16;
                        }
                        else
                        {
                            pppVectPtr->pppsIpcpState.ipcpsLocalWantOptions.
                                ipcpCompTcpSpace = temp16;
                        }
                    }
                    break;

/*
 * IP Header Compression:The maximum number of slots used to store non-TCP
 * (UDP, etc) header compression info; this value is determined by the maximum
 * number of concurrent non-TCP sessions that are expected across this link.
 */
                case TM_IPCP_IPHC_NON_TCP_SPACE:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
                            pppVectPtr->pppsIpcpState.ipcpsRemoteAllowOptions.
                                ipcpCompNonTcpSpace = temp16;
                        }
                        else
                        {
                            pppVectPtr->pppsIpcpState.ipcpsLocalWantOptions.
                                ipcpCompNonTcpSpace = temp16;
                        }
                    }
                    break;

/* IP Header Compression:The maximum interval between sending full headers. */

                case TM_IPCP_IPHC_MAX_PERIOD:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIpcpState.ipcpsLocalWantOptions.
                            ipcpCompMaxPeriod = temp16;
                    }

                    break;

/*
 * IP Header Compression:The maximum time interval, in seconds, between
 * sending full headers.
 */
                case TM_IPCP_IPHC_MAX_TIME:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIpcpState.ipcpsLocalWantOptions.
                            ipcpCompMaxTime = temp16;
                    }
                    break;

/*
 * IP Header Compression:The largest header size in octets that may be
 * compressed.
 */
                case TM_IPCP_IPHC_MAX_HEADER:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIpcpState.ipcpsLocalWantOptions.
                            ipcpCompMaxHeader = temp16;
                    }
                    break;

#endif /* TM_USE_IPHC */


/* Any other options are invalid for this protocol */
               default:
                   errorCode = TM_ENOPROTOOPT;
                   break;
            }
        }
        else
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        if (protocolLevel == (int)TM_PPP_IPV6CP_PROTOCOL)
        {
            switch (optionName)
            {
                case TM_IPV6CP_RETRY:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.
                            cpsInitCfgRetryCnt = (tt8Bit)(temp8+1);
                    }
                    break;

                case TM_IPV6CP_TIMEOUT:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.
                            cpsRetryTimeoutSeconds = temp8;
                    }
                    break;

                case TM_IPV6CP_MAX_FAILURES:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.
                            cpsInitMaxFailures = temp8;
                    }
                    break;

#ifdef TM_USE_IPHC
                case TM_IPV6CP_COMP_PROTOCOL:
/* Header compression not supported on DSP platforms */
#ifndef TM_DSP
                    if (tm_ll_is_pppoe(devPtr))
                    {
                        errorCode = TM_EPROTONOSUPPORT;
                    }
                    else
                    {
                        if ((unsigned)optionLength < sizeof(tt16Bit))
                        {
                            errorCode = TM_EINVAL;
                        }
                        else
                        {
                            if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                            {
/* allow remote to use VJ header compression */
                                pppVectPtr->pppsIp6cpState.
                                    ip6cpsRemoteNegotiateOption |=
                                    (TM_UL(1)<<(tt8Bit)optionType);
                                pppVectPtr->pppsIp6cpState.
                                    ip6cpsRemoteAllowOptions.
                                    ip6cpCompressProtocol = temp16;
                            }
                            else
                            {
/* Limit size to 2 bytes (RFC1332) */
/* the protocol compression we wish to use */
                                pppVectPtr->pppsIp6cpState.
                                  ip6cpsLocalWantOptions.
                                  ip6cpCompressProtocol = temp16;
                                pppVectPtr->pppsIp6cpState.
                                    ip6cpsLocalNegotiateOption |=
                                                 (TM_UL(1)<<(tt8Bit)optionType);
                            }
                        }
                    }
#else /* TM_DSP */
                    errorCode = TM_EPROTONOSUPPORT;
#endif /* TM_DSP */
                    break;

/*
 * IP Header Compression: The maximum number of slots used to store TCP header
 * compression info; this value is determined by the maximum number of
 * concurrent TCP sessions that are expected over this link.
 */
                case TM_IPV6CP_IPHC_TCP_SPACE:
                    if (((unsigned)optionLength < sizeof(tt16Bit))
                                                     || (temp16 > 255))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
                            pppVectPtr->pppsIp6cpState.ip6cpsRemoteAllowOptions.
                                ip6cpCompTcpSpace = temp16;
                        }
                        else
                        {
                            pppVectPtr->pppsIp6cpState.ip6cpsLocalWantOptions.
                                ip6cpCompTcpSpace = temp16;
                        }
                    }
                    break;

/*
 * IP Header Compression:The maximum number of slots used to store non-TCP
 * (UDP, etc) header compression info; this value is determined by the maximum
 * number of concurrent non-TCP sessions that are expected across this link.
 */
                case TM_IPV6CP_IPHC_NON_TCP_SPACE:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
                            pppVectPtr->pppsIp6cpState.ip6cpsRemoteAllowOptions.
                                ip6cpCompNonTcpSpace = temp16;
                        }
                        else
                        {
                            pppVectPtr->pppsIp6cpState.ip6cpsLocalWantOptions.
                                ip6cpCompNonTcpSpace = temp16;
                        }
                    }
                    break;

/* IP Header Compression:The maximum interval between sending full headers. */

                case TM_IPV6CP_IPHC_MAX_PERIOD:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIp6cpState.ip6cpsLocalWantOptions.
                            ip6cpCompMaxPeriod = temp16;
                    }
                    break;

/*
 * IP Header Compression:The maximum time interval, in seconds, between
 * sending full headers.
 */
                case TM_IPV6CP_IPHC_MAX_TIME:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIp6cpState.ip6cpsLocalWantOptions.
                            ip6cpCompMaxTime = temp16;
                    }
                    break;

/*
 * IP Header Compression:The largest header size in octets that may be
 * compressed.
 */
                case TM_IPV6CP_IPHC_MAX_HEADER:
                    if ((unsigned)optionLength < sizeof(tt16Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsIp6cpState.ip6cpsLocalWantOptions.
                            ip6cpCompMaxHeader = temp16;
                    }
                    break;

#endif /* TM_USE_IPHC */

                default:
                    errorCode = TM_ENOPROTOOPT;
                    break;
            }
        }
        else
#endif /* TM_USE_IPV6 */

/* Set PAP protocol options (username, password) */

        if (protocolLevel == (int)TM_PPP_PAP_PROTOCOL)
        {
            papInfoPtr = &pppVectPtr->pppsPapState.papsLocalOurInfo;
            switch (optionName)
            {
                case TM_PAP_USERNAME:
                    errorCode = tfPppCopyString( optionValuePtr,
                                                 &papInfoPtr->papUserNamePtr,
                                                 optionLength);
                    if (errorCode != TM_ENOERROR)
                    {
                        goto SetOptionEnd;
                    }
                    break;

                case TM_PAP_PASSWORD:
                    errorCode = tfPppCopyString( optionValuePtr,
                                                 &papInfoPtr->papPasswordPtr,
                                                 optionLength);
                    if (errorCode != TM_ENOERROR)
                    {
                        goto SetOptionEnd;
                    }
                    break;

                case TM_PAP_RETRY:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsPapState.papsStateInfo.
                            cpsInitCfgRetryCnt = (tt8Bit)(temp8 + 1);
                    }
                    break;

                case TM_PAP_TIMEOUT:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsPapState.papsStateInfo.
                            cpsRetryTimeoutSeconds = temp8;
                    }
                    break;

                default:
                    errorCode = TM_ENOPROTOOPT;
                    break;

            }
        }
        else

        if (protocolLevel == (int)TM_PPP_CHAP_PROTOCOL)
        {
            switch (optionName)
            {

                case TM_CHAP_USERNAME:
                    if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
/* Set our name as authenticator */
                    {
                        chapInfoPtr = &pppVectPtr->pppsChapState.
                                                     chapsRemoteAllowInfo;
                    }
                    else
/* Set our name as peer */
                    {
                        chapInfoPtr = &pppVectPtr->pppsChapState.
                                                     chapsLocalOurInfo;
                    }
                    errorCode = tfPppCopyString(
                                            optionValuePtr,
                                            &chapInfoPtr->chapUserNamePtr,
                                            optionLength);
                    if (errorCode != TM_ENOERROR)
                    {
                        goto SetOptionEnd;
                    }
                    break;

                case TM_CHAP_SECRET:
                    chapInfoPtr = &pppVectPtr->pppsChapState.
                                                     chapsLocalOurInfo;
                    errorCode = tfPppCopyString(
                                            optionValuePtr,
                                            &chapInfoPtr->chapSecretPtr,
                                            optionLength);
                    if (errorCode != TM_ENOERROR)
                    {
                        goto SetOptionEnd;
                    }
                    break;
#ifdef TM_USE_PPP_MSCHAP
                case TM_CHAP_MSSECRET:
                    chapInfoPtr = &pppVectPtr->pppsChapState.
                                                     chapsLocalOurInfo;
                    errorCode = tfPppCopyString(
                                            optionValuePtr,
                                            &chapInfoPtr->chapMsSecretPtr,
                                            optionLength);
                    if (errorCode != TM_ENOERROR)
                    {
                        goto SetOptionEnd;
                    }
/* MSCHAP uses unicode password, for example "AB" will be exchanged as "00410042"
 * if we use strlen(password) to get the password length, it is wrong.
 * User needs to specify how many octets for the password, and saves the
 * password into char array. Such as pw[0]=0x0, pw[1]=0x41,pw[2]=0x0, pw[3]=0x42,
 * and passwordLength = 4
 */
                    chapInfoPtr->chapMsSecretLength =(tt16Bit)optionLength;
                    break;
#endif /* TM_USE_PPP_MSCHAP */

                case TM_CHAP_ALG_ADD:
                    if ( temp8 == TM_CHAP_MD5
#ifdef TM_USE_PPP_MSCHAP
                             || temp8 == TM_CHAP_MSV1
                             || temp8 == TM_CHAP_MSV2
#endif /* TM_USE_PPP_MSCHAP */
                     )
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
                            chapInfoPtr = &pppVectPtr->pppsChapState.
                                chapsRemoteAllowInfo;
#ifdef TM_USE_PPP_MSCHAP
                            if(temp8 == TM_CHAP_MSV1)
                            {
                                pppLcpStatePtr->
                                    lcpsRemoteAllowOptions.lcpAuthProtocolBits |=
                                    (1 << ( TM_PPP_AUTHMETHOD_MSCHAP_V1 - 1));
                            }
#endif /*TM_USE_PPP_MSCHAP*/

                        }
                        else
                        {
                            chapInfoPtr = &pppVectPtr->pppsChapState.
                                chapsLocalOurInfo;
#ifdef TM_USE_PPP_MSCHAP
                            if(temp8 == TM_CHAP_MSV1)
                            {
                                pppLcpStatePtr->lcpsLocalWantOptions.
                                    lcpAuthProtocolBits |=
                                    (1 << ( TM_PPP_AUTHMETHOD_MSCHAP_V1 - 1));
                            }
#endif /*TM_USE_PPP_MSCHAP*/
                        }

                    }
                    else
                    {
                         errorCode = TM_EINVAL;
                         goto SetOptionEnd;
                    }
                    break;
                case TM_CHAP_ALG_DEL:
                    if ( temp8 == TM_CHAP_MD5
#ifdef TM_USE_PPP_MSCHAP
                             || temp8 == TM_CHAP_MSV1
                             || temp8 == TM_CHAP_MSV2
#endif /* TM_USE_PPP_MSCHAP */
                     )
                    {
                        if (remoteLocalFlag == TM_PPP_OPT_ALLOW)
                        {
                            chapInfoPtr = &pppVectPtr->pppsChapState.
                                chapsRemoteAllowInfo;
#ifdef TM_USE_PPP_MSCHAP
                            if(temp8 == TM_CHAP_MSV1)
                            {
                                tm_8bit_clr_bit( pppLcpStatePtr->
                                  lcpsRemoteAllowOptions.lcpAuthProtocolBits,
                                  (1 << ( TM_PPP_AUTHMETHOD_MSCHAP_V1 - 1)) );
                            }
                            else
#endif /*TM_USE_PPP_MSCHAP*/
                            if(temp8 == TM_CHAP_MD5)
                            {
                                tm_8bit_clr_bit( pppLcpStatePtr->
                                  lcpsRemoteAllowOptions.lcpAuthProtocolBits,
                                  (1 << ( TM_PPP_AUTHMETHOD_CHAP - 1)) );
                            }

                        }
                        else
                        {
                            chapInfoPtr = &pppVectPtr->pppsChapState.
                                chapsLocalOurInfo;
#ifdef TM_USE_PPP_MSCHAP
                            if(temp8 == TM_CHAP_MSV1)
                            {
                                tm_8bit_clr_bit( pppLcpStatePtr->
                                    lcpsLocalWantOptions.lcpAuthProtocolBits,
                                    (1 << ( TM_PPP_AUTHMETHOD_MSCHAP_V1 - 1)) );
                            }
                            else
#endif /*TM_USE_PPP_MSCHAP*/
                            if(temp8 == TM_CHAP_MD5)
                            {
                                tm_8bit_clr_bit( pppLcpStatePtr->
                                    lcpsLocalWantOptions.lcpAuthProtocolBits,
                                    (1 << ( TM_PPP_AUTHMETHOD_CHAP - 1)) );
                            }
                        }

                    }
                    else
                    {
                         errorCode = TM_EINVAL;
                         goto SetOptionEnd;
                    }
                    break;
                case TM_CHAP_RETRY:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsChapState.chapsStateInfo.
                            cpsInitCfgRetryCnt = (tt8Bit)(temp8 + 1);
                    }
                    break;

                case TM_CHAP_TIMEOUT:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsChapState.chapsStateInfo.
                            cpsRetryTimeoutSeconds = temp8;
                    }
                    break;

                default:
                    errorCode = TM_ENOPROTOOPT;
                    break;



            }
        }
        else
/*
 * This section is for general PPP options that don't fit into the above
 * protocol based categories.
 */
        if (protocolLevel == (int) TM_PPP_PROTOCOL)
        {
            switch (optionName)
            {
/* Open timeout is only used in dual-stack mode */
#if (defined(TM_USE_IPV4) && defined(TM_USE_IPV6))
                case TM_PPP_OPEN_TIMEOUT:
                    if ((unsigned)optionLength != sizeof(tt8Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsOpenFailTimeout = temp8;
                    }
                    break;
#endif /* TM_USE_IPV4 && TM_USE_IPV6 */

/* Sets the send buffer length: must be 16-bits and greater than zero */
                case TM_PPP_SEND_BUFFER_SIZE:
                    if (((unsigned)optionLength != sizeof(tt16Bit)) ||
                        (temp16 < 1))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        pppVectPtr->pppsUserSendBufSize = temp16;
                    }
                    break;

                default:
                    errorCode = TM_ENOPROTOOPT;
                    break;
            }

        }
        else

#ifdef TM_USE_PPP_CALLBACK
/*
 * Callback Control Protocol.
 */
        if (protocolLevel == (int)TM_PPP_CBCP_PROTOCOL)
        {
            cbcpStatePtr = &pppVectPtr->pppsCallbackState;
            switch (optionName)
            {
/*** Set the role of this PPP interface: caller or answerer. ***/
                case TM_CBCP_OP_ROLE:
                    if (    optionLength != 1
                        ||  temp8 > (tt8Bit)TM_CBCP_ROLE_ANSWERER )
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        cbcpStatePtr->cbcpsRole = temp8;
                    }
                    break;

/*** Select the type of callback we allow. ***/
                case TM_CBCP_OP_TYPES:
                    if (optionLength < 1)
                    {
                        errorCode = TM_EINVAL;
                        break;
                    }
                    bytePtr = cbcpStatePtr->cbcpsTypeSet;
                    cbcpStatePtr->cbcpsTypeSetCount = TM_16BIT_ZERO;
                    for (length = 0; ; )
                    {
                        switch (temp8)
                        {
/*
 * Supported Callback types:
 * 1. Do not use callback,
 * 2. Callback to a user-specifiable number (number provided by caller),
 * 3. Callback to a pre-specified or administrator specified number (number
 *    provided by answerer).
 */
                        case TM_CBCP_CTYPE_NOCALL:
                        case TM_CBCP_CTYPE_CALLER:
                        case TM_CBCP_CTYPE_ANSWERER:
/* Append the Callback types to the end of our list, if the type does not
 * already exist. */
                            index = 0;
                            do
                            {
                                if (index == length)
                                {
                                    bytePtr[length++] = temp8;
                                    break;
                                }
                            } while (bytePtr[index++] != temp8);
                            break;
/* No other options are supported at this time. */
                        default:
                            errorCode = TM_EINVAL;
                        }
                        if (errorCode != TM_ENOERROR)
                        {
                            break;
                        }
/* Pick up the next option, if any. */
                        if (--optionLength == 0)
                        {
                            cbcpStatePtr->cbcpsTypeSetCount = (tt8Bit)length;
                            break;
                        }
                        temp8 = (tt8Bit)*++optionValuePtr;
                    }
                    break;

/*** Maximum number of send attempts. ***/
                case TM_CBCP_OP_RETRY:
                    if (optionLength != 1)
                    {
                        errorCode = TM_EINVAL;
                        break;
                    }
                    cbcpStatePtr->cbcpsMaxRetry = temp8;
                    break;

/*** Time between sends attempts (seconds). ***/
                case TM_CBCP_OP_TIMEOUT:
                    if (optionLength != 1)
                    {
                        errorCode = TM_EINVAL;
                        break;
                    }
                    cbcpStatePtr->cbcpsRetryTimeout = temp8;
                    break;

/*** Delay before disconnecting (seconds).. ***/
                case TM_CBCP_OP_DISC_DELAY:
                    if (optionLength != 1)
                    {
                        errorCode = TM_EINVAL;
                        break;
                    }
                    cbcpStatePtr->cbcpsDisconnectDelay = temp8;
                    break;

/*** Set the callback delay (seconds). ***/
                case TM_CBCP_OP_CALL_DELAY:
                    if (optionLength != 1)
                    {
                        errorCode = TM_EINVAL;
                        break;
                    }
                    cbcpStatePtr->cbcpsDelaySet = temp8;
                    break;

/*** Set the callback address for caller or answerer. ***/
                case TM_CBCP_OP_ADDRESS:
                    if (    optionLength < 2
                        ||  temp8 != (tt8Bit)TM_CBCP_ATYPE_PSTN )
                    {
                        errorCode = TM_EINVAL;
                        break;
                    }
/* Copy the callback address from the user's storage to the PPP vector.
 * Allocate a new buffer, if necessary. Allow for an extra null byte
 * if the user has not provided it. */
                    bytePtr = cbcpStatePtr->cbcpsAddressSetPtr;
                    if (cbcpStatePtr->cbcpsAddressSetLength < optionLength
                                                                        + 1)
                    {
                        if (bytePtr != TM_8BIT_NULL_PTR)
                        {
                            tm_free_raw_buffer((ttRawBufferPtr)bytePtr);
                        }
                        bytePtr = (tt8BitPtr)tm_get_raw_buffer(optionLength
                                                                        + 1);
                        cbcpStatePtr->cbcpsAddressSetPtr = bytePtr;
                        if (bytePtr == TM_8BIT_NULL_PTR)
                        {
                            errorCode = TM_ENOBUFS;
                            break;
                        }
                    }
                    tm_bcopy(optionValuePtr, bytePtr, optionLength);
/* Add the required null byte, if the user has not. */
                    if (bytePtr[optionLength - 1] != TM_8BIT_ZERO)
                    {
                        bytePtr[optionLength++] = TM_8BIT_ZERO;
                    }
                    cbcpStatePtr->cbcpsAddressSetLength = (tt16Bit)optionLength;
                    break;

                default:
                    errorCode = TM_ENOPROTOOPT;
            }
        }
        else
#endif /* TM_USE_PPP_CALLBACK */

        {

            errorCode = TM_ENOPROTOOPT;
        }
SetOptionEnd:
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/*
 * Open a PPP client session
 */
int tfPppClientOpen(void TM_FAR * interfaceId,
                    tt16Bit       multiHomeIndex,
                    int           protocolFamily)
{
    TM_UNREF_IN_ARG(multiHomeIndex);
    return tfPppOpen(interfaceId, TM_PPP_CLIENT_LINK, protocolFamily);
}

/*
 * Open a PPP server session
 */
int tfPppServerOpen(void TM_FAR * interfaceId,
                    tt16Bit       multiHomeIndex,
                    int           protocolFamily)
{
    TM_UNREF_IN_ARG(multiHomeIndex);
    return tfPppOpen(interfaceId, TM_PPP_SERVER_LINK, protocolFamily);
}

/* check if we do have sufficient parameter to support the AUTH
 * method we claimed.
 */
static void tfPppCheckAuthProtocolBits(ttPppVectPtr   pppVectPtr)
{
    tt8Bit               allowAuthBits;
    ttPapOptionsPtr      papPtr;
    ttChapOptionsPtr     chapPtr;
    ttLcpStatePtr        pppLcpStatePtr;

    pppLcpStatePtr = &pppVectPtr->pppsLcpState;

/* if peer allow authenticator to set PAP and CHAP in its LCP request,
 * peer needs to have its name and secret ready
 */
/* bit 0: EAP        0x01
 * bit 1: CHAP       0x02
 * Bit 2: PAP        0x04
 * Bit 3: MSCHAP1    0x08
 */
    allowAuthBits = pppLcpStatePtr->lcpsRemoteAllowOptions.lcpAuthProtocolBits;

/* normal CHAP authentication ? */
    if( tm_8bit_one_bit_set(allowAuthBits,
                           (1 << (TM_PPP_AUTHMETHOD_CHAP - 1))))
    {
        chapPtr =  &pppVectPtr->pppsChapState.chapsLocalOurInfo;
        if(chapPtr->chapUserNamePtr == 0 ||
            chapPtr->chapSecretPtr == 0 )
        {
/* we can't say that we suport this auth method, clear it */
            tm_8bit_clr_bit( allowAuthBits,
                             ((1 <<(TM_PPP_AUTHMETHOD_CHAP - 1))) );
        }
    }

#ifdef TM_USE_PPP_MSCHAP
/*MS-CHAPv1 ? */
    if( tm_8bit_one_bit_set(allowAuthBits,
                           (1 << (TM_PPP_AUTHMETHOD_MSCHAP_V1 - 1))))
    {
        chapPtr =  &pppVectPtr->pppsChapState.chapsLocalOurInfo;
        if(chapPtr->chapUserNamePtr == 0 ||
            chapPtr->chapMsSecretPtr == 0 ||
            chapPtr->chapMsSecretLength == 0)
        {
/* we can't say that we suport this auth method, clear it */
            allowAuthBits &= ~(1 << (TM_PPP_AUTHMETHOD_MSCHAP_V1 - 1));
        }
    }
#endif /* TM_USE_PPP_MSCHAP */

/* PAP authentication ? */
    if(allowAuthBits & (1 << (TM_PPP_AUTHMETHOD_PAP - 1)))
    {
        papPtr =  &pppVectPtr->pppsPapState.papsLocalOurInfo;
        if(papPtr->papUserNamePtr == 0 ||
               papPtr->papPasswordPtr == 0 )
        {
/* we can't say that we suport this auth method, clear it */
            tm_8bit_clr_bit( allowAuthBits,
                             (1 << (TM_PPP_AUTHMETHOD_PAP - 1)) );
        }
    }

#ifdef TM_USE_EAP
    if(allowAuthBits & (1 << (TM_PPP_AUTHMETHOD_EAP - 1)))
    {
        if(pppVectPtr->pppsEapVectPtr == (ttVoidPtr)0)
        {
            allowAuthBits &= ~(1 << (TM_PPP_AUTHMETHOD_EAP - 1));
        }

    }
#endif /* TM_USE_EAP */


    pppLcpStatePtr->lcpsRemoteAllowOptions.
        lcpAuthProtocolBits = allowAuthBits;
    return;
}


static int tfPppGetAuthProtocolBit(tt16Bit     protocol,
                                   tt8Bit      chapAlgorithm)
{
/* when we call this funciton, we have already verified that protocol
 * can be only TM_PPP_PAP_HOST_PROTOCOL, TM_PPP_CHAP_HOST_PROTOCOL or
 * TM_PPP_EAP_HOST_PROTOCOL if EAP is used.
 */
    int    bitsOffset;

    bitsOffset = 0;

    switch (protocol)
    {
    case TM_PPP_PAP_HOST_PROTOCOL:
        bitsOffset = TM_PPP_AUTHMETHOD_PAP - 1;
        break;
    case TM_PPP_CHAP_HOST_PROTOCOL:
        if(chapAlgorithm == TM_CHAP_MD5)
        {
            bitsOffset = TM_PPP_AUTHMETHOD_CHAP - 1;
        }
#ifdef TM_USE_PPP_MSCHAP
        else if(chapAlgorithm == TM_CHAP_MSV1)
        {
            bitsOffset = TM_PPP_AUTHMETHOD_MSCHAP_V1 - 1;
        }
#endif /* TM_USE_PPP_MSCHAP */
        break;
#ifdef TM_USE_EAP
    case TM_PPP_EAP_HOST_PROTOCOL:
        bitsOffset = TM_PPP_AUTHMETHOD_EAP - 1;
        break;
#endif /* TM_USE_EAP */
    default:
/* should never reach here*/
        bitsOffset = 0;
    }
    return bitsOffset;
}

/* get the next authentication protocol (in host order), according
 * to our priority sequence, and if the protocol is CHAP, we need
 * return the algorithm.
 * the currentAuthMethod has been NAKed, choose another one with
 * highest priority
 */
static tt8Bit tfLcpGetNextAuthMethod(tt8Bit     authProtocolBits,
                                     tt8Bit     currentAuthMethod,
                                     tt8BitPtr  algorithmPtr)
{
    int           i;
    tt8Bit        algor;
    tt8Bit        authMethod;
    tt8Bit        origPriority;
    tt8Bit        currentHighestPriority; /*with lowest priority number*/
    tt8Bit        nextPriority;

    algor = TM_8BIT_ZERO;
    authMethod = TM_PPP_AUTHMETHOD_NULL;
    currentHighestPriority = 0;
    origPriority = tm_ppp_auth_get_priority(currentAuthMethod);

    for(i = TM_PPP_AUTHMETHOD_MIN; i < TM_PPP_AUTHMETHOD_MAX; i++)
    {
        if(i != TM_PPP_AUTHMETHOD_NULL &&
           i != currentAuthMethod &&
           tm_8bit_one_bit_set(authProtocolBits, (1<<(i - 1)))
/* we need support this authenticaiton method */
           )
        {
            nextPriority = tm_ppp_auth_get_priority(i);
            if(nextPriority < origPriority)
            {
/* this one has higher priority than the original one, find next */
                continue;
            }
            else if(currentHighestPriority == 0 ||
                ((nextPriority < currentHighestPriority) &&
                  nextPriority != 0))
/* if user sets two authmethod with the same priority value,
 * it will skip it
 */
            {
                currentHighestPriority = nextPriority;
                authMethod = (tt8Bit)i;
            }
        }
    }


    switch(authMethod)
    {
    case TM_PPP_AUTHMETHOD_CHAP:
        algor = TM_CHAP_MD5;
        break;
#ifdef TM_USE_PPP_MSCHAP
    case TM_PPP_AUTHMETHOD_MSCHAP_V1:
        algor = TM_CHAP_MSV1;
        break;
#endif /* TM_USE_PPP_MSCHAP */
    default:
        break;
    }

    *algorithmPtr = algor;
    return authMethod;
}


static tt16Bit tfPppFromMethodToProtocol(tt8Bit authMethod)
{
    tt16Bit   authHostProtocol;

    authHostProtocol = (tt16Bit)0;
    switch(authMethod)
    {
    case TM_PPP_AUTHMETHOD_CHAP:
#ifdef TM_USE_PPP_MSCHAP
    case TM_PPP_AUTHMETHOD_MSCHAP_V1:
#endif /* TM_USE_PPP_MSCHAP */
        authHostProtocol = TM_PPP_CHAP_HOST_PROTOCOL;
        break;
    case TM_PPP_AUTHMETHOD_PAP:
        authHostProtocol = TM_PPP_PAP_HOST_PROTOCOL;
        break;

#ifdef TM_USE_EAP
    case TM_PPP_AUTHMETHOD_EAP:
        authHostProtocol = TM_PPP_EAP_HOST_PROTOCOL;
        break;
#endif /* TM_USE_EAP */

    default:
        break;
    }
    return authHostProtocol;
}


/*
 * Open a PPP session.
 * This could be called for both IPv6 and IPv4.
 */
static int tfPppOpen ( void TM_FAR   * interfaceId,
                       tt8Bit          pppLinkLayer,
                       int             protocolFamily)
{
    ttPppVectPtr pppVectPtr;
    int          errorCode;
    tt8Bit       pppVectWasAllocated;
    ttLcpStatePtr pppLcpStatePtr;
#ifdef TM_USE_IPV4
    tt8Bit       ipcpWasStarted;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt8Bit       ip6cpWasStarted;
#endif /* TM_USE_IPV6 */

    errorCode = TM_ENOERROR;

/*
 * Don't need to lock in this function because the device is already locked
 * by tfConfigInterface.
 */

/* Did the user call configurePpp? */
    pppVectPtr = (ttPppVectPtr)(((ttDeviceEntryPtr)interfaceId)->
                                                         devStateVectorPtr);

    if (pppVectPtr == TM_PPP_VECT_NULL_PTR)
    {
/* If not we need to create a PPP State Vector here */
        pppVectWasAllocated = TM_8BIT_ZERO;
        pppVectPtr = tfPppVectAlloc( (ttDeviceEntryPtr)interfaceId );
        if (pppVectPtr == TM_PPP_VECT_NULL_PTR )
        {
            errorCode = TM_ENOBUFS;
        }
    }
    else
    {
        pppVectWasAllocated = TM_8BIT_YES;
    }

    if (pppVectPtr != TM_PPP_VECT_NULL_PTR)
    {

        pppLcpStatePtr = &pppVectPtr->pppsLcpState;
/*
 * Only go through initialization if the connection has not yet been started.
 * This could be the case when running in dual stack mode, since tfPppOpen
 * could be called twice.
 */
        if (pppLcpStatePtr->lcpsStateInfo.cpsState == TM_PPPS_INITIAL)
        {

/* Initialize each of the layers */
            if(tm_context(tvPppAuthPriority) == 0)
            {
                tm_context(tvPppAuthPriority) =
                         TM_PPP_AUTH_DEFAULT_PRIORITY;
            }
            tfLcpInit(pppVectPtr);
            tfChapInit(pppVectPtr);
            tfPapInit(pppVectPtr);
#ifdef TM_USE_IPV4
            tfIpcpInit(pppVectPtr);
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
            tfIp6cpInit(pppVectPtr);
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_PPP_CALLBACK
            tfCbcpInit(pppVectPtr, pppLinkLayer);
#endif /* TM_USE_PPP_CALLBACK */

/* If there has been a change in the send buffer size since last session... */
            if (pppVectPtr->pppsUserSendBufSize != pppVectPtr->pppsSendBufSize)
            {
/* Change to new buffer size. */
                pppVectPtr->pppsSendBufSize = pppVectPtr->pppsUserSendBufSize;
                if (pppVectPtr->pppsSendBufPtr != &(pppVectPtr->pppsSendBuf[0]))
/* If we had previously allocated a send buffer, free it */
                {
                    tm_free_raw_buffer(
                        (ttRawBufferPtr)pppVectPtr->pppsSendBufPtr);
                }
                if (pppVectPtr->pppsSendBufSize > TM_PPP_DEF_BUF_SIZE)
/*
 * If the user wants a send buffer size bigger than the default one,
 * allocate it.
 */
                {
                    pppVectPtr->pppsSendBufPtr = (ttCharPtr)
                        tm_get_raw_buffer(pppVectPtr->pppsUserSendBufSize);
                    if (pppVectPtr->pppsSendBufPtr == (ttCharPtr)0)
                    {
/* If the memory allocation fails, use pre-allocated send buffer size */
                        pppVectPtr->pppsSendBufSize = TM_PPP_DEF_BUF_SIZE;
                    }
                }
                if (pppVectPtr->pppsSendBufSize <= TM_PPP_DEF_BUF_SIZE)
/* If size is less or equal to the pre-allocated buffer. Use it instead */
                {
                    pppVectPtr->pppsSendBufPtr = &(pppVectPtr->pppsSendBuf[0]);
                }
            }
/*
 *  Free saved send and recv VJ headers, plus
 *  current incoming packet from a previous connection.
 */
            if (pppVectWasAllocated != TM_8BIT_ZERO)
            {
                tfPppFreeBuffersTimers(pppVectPtr);
            }
        }

/* Check to make sure that we are not in the process of closing */
        if (pppLcpStatePtr->lcpsStateInfo.cpsState == TM_PPPS_CLOSING)
        {
            errorCode = TM_EAGAIN;
        }
        else
        {
/* Start the LCP layer if not in the process of opening.*/
            if (    pppLcpStatePtr->lcpsStateInfo.cpsState
                 == TM_PPPS_INITIAL )
            {
                if (errorCode == TM_ENOERROR)
                {
                    ((ttDeviceEntryPtr)interfaceId)->devStateVectorPtr =
                                                    (ttVoidPtr)pppVectPtr;
/* Save the device entry in the state vector */
                    pppVectPtr->pppsDeviceEntryPtr =
                                                (ttDeviceEntryPtr)interfaceId;

#ifdef TM_USE_IPV6
                    if (protocolFamily == PF_INET6)
                    {
                        ((ttDeviceEntryPtr)interfaceId)->dev6Flags |=
                            TM_6_LL_CONNECTING;
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
                        tm_kernel_set_critical;
                        tm_context(tvIpData).ipv6InterfaceTableLastChange =
                            tm_snmp_time_ticks(tvTime);
                        tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
                    }
#endif /* TM_USE_IPV6 */

                    if (pppLinkLayer == TM_PPP_CLIENT_LINK)
                    {
/* Start with LCP */
                        errorCode = tfPppStateMachine( pppVectPtr,
                                                       TM_PACKET_NULL_PTR,
                                                       TM_PPPE_UP,
                                                       TM_PPP_LCP_INDEX);
                        if (errorCode == TM_ENOERROR)
                        {
/* Now send the config request */
                            errorCode = tfPppStateMachine( pppVectPtr,
                                                           TM_PACKET_NULL_PTR,
                                                           TM_PPPE_OPEN,
                                                           TM_PPP_LCP_INDEX);
                        }
                    }
                    else
                    {

/* Since this is a PPP *server*, we don't actually start the state machine,
 *  but just start off in the STOPPED state (and just wait for incoming
 *  packets)
 */
                        pppVectPtr->pppsStateInfoPtrArray[TM_PPP_LCP_INDEX]
                                                    ->cpsState=TM_PPPS_STOPPED;

                    }
                }
            }
            else
            {
/* Determine if IPCP has been started. */
#ifdef TM_USE_IPV4
                if (   pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState
                    == TM_PPPS_INITIAL)
                {
/* LCP has already been opened, but IPCP has not - make sure that IPCP is
   initialized. */
                    tfIpcpInit(pppVectPtr);
                    ipcpWasStarted = TM_8BIT_ZERO;
                }
                else
                {
                    ipcpWasStarted = TM_8BIT_YES;
                }
#endif /* TM_USE_IPV4 */

/* Determine if IPV6CP has been started. */
#ifdef TM_USE_IPV6
                if (   pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState
                    == TM_PPPS_INITIAL)
                {
/* LCP has already been opened, but IPV6CP has not - make sure that IPV6CP is
   initialized. */
                    tfIp6cpInit(pppVectPtr);
                    ip6cpWasStarted = TM_8BIT_ZERO;
                    if (protocolFamily == PF_INET6)
                    {
                        ((ttDeviceEntryPtr)interfaceId)->dev6Flags |=
                            TM_6_LL_CONNECTING;
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
                        tm_kernel_set_critical;
                        tm_context(tvIpData).ipv6InterfaceTableLastChange =
                            tm_snmp_time_ticks(tvTime);
                        tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
                    }
                }
                else
                {
                    ip6cpWasStarted = TM_8BIT_YES;
                }
#endif /* TM_USE_IPV6 */


#ifdef TM_USE_IPV4
/*
 * Opening IPv4 on this link.  If IPCP hasn't already been started, and we've
 * reached the NCP phase (i.e. IPV6CP has been started) start the IPCP layer.
 * If we haven't reached the NCP phase, just wait; IPCP will be started after
 * LCP and authentication have completed.
 */
                if (protocolFamily == PF_INET)
                {
                    if (ipcpWasStarted)
                    {
                        errorCode = TM_EALREADY;
                    }
#ifdef TM_USE_IPV6
                    else
                    {
                        if (ip6cpWasStarted)
                        {
                            errorCode = tfPppStateMachine(pppVectPtr,
                                                          TM_PACKET_NULL_PTR,
                                                          TM_PPPE_UP,
                                                          TM_PPP_IPCP_INDEX);

                            if (errorCode == TM_ENOERROR)
                            {
                                errorCode =
                                    tfPppStateMachine(pppVectPtr,
                                                      TM_PACKET_NULL_PTR,
                                                      TM_PPPE_OPEN,
                                                      TM_PPP_IPCP_INDEX);
                            }
                        }
                    }
#endif /* TM_USE_IPV6 */
                }
#endif /* TM_USE_IPV4 */


#ifdef TM_USE_IPV6
/*
 * Opening IPv6 on this link.  If IPV6CP hasn't already been started, and
 * we've reached the NCP phase (i.e. IPCP has been started) start the IPV6CP
 * layer.  If we haven't reached the NCP phase, just wait; IPV6CP will be
 * started after LCP and authentication have completed.
 */
                if (protocolFamily == PF_INET6)
                {
                    if (ip6cpWasStarted)
                    {
                        errorCode = TM_EALREADY;
                    }
                    else
                    {
#ifdef TM_USE_IPV4
                        if (ipcpWasStarted)
                        {
                            errorCode = tfPppStateMachine(pppVectPtr,
                                                          TM_PACKET_NULL_PTR,
                                                          TM_PPPE_UP,
                                                          TM_PPP_IPV6CP_INDEX);

                            if (errorCode == TM_ENOERROR)
                            {
                                errorCode =
                                    tfPppStateMachine(pppVectPtr,
                                                      TM_PACKET_NULL_PTR,
                                                      TM_PPPE_OPEN,
                                                      TM_PPP_IPV6CP_INDEX);
                            }
                        }
#endif /* TM_USE_IPV4 */
                    }
                }
#endif /* TM_USE_IPV6 */
            }
        }
    }

/* If no problems occurred, the negotiation started, but has not yet finished. */
    if (errorCode == TM_ENOERROR)
    {
        errorCode = TM_EINPROGRESS;
    }

    return errorCode;
}


/*
 * Close a PPP Session
 */
int tfPppClose ( void TM_FAR *interfaceId )
{
    ttPppVectPtr            pppVectPtr;
    ttLcpStatePtr           pppLcpStatePtr;
    ttDeviceEntryPtr        devPtr;
#ifdef TM_USE_STOP_TRECK
#ifdef TM_USE_PPP_CALLBACK
    ttCallbackStatePtr      cbcpStatePtr;
#endif /* TM_USE_PPP_CALLBACK */
#endif /* TM_USE_STOP_TRECK */
#ifdef TM_USE_IPHC
    ttLnkHdrCompressFuncPtr hdrCompressFuncPtr;
#endif /* TM_USE_IPHC */
    int                     retCode;

    devPtr = (ttDeviceEntryPtr)interfaceId;

/* Assume ENOERROR */
    retCode = TM_ENOERROR;

    pppVectPtr = (ttPppVectPtr)devPtr->devStateVectorPtr;

    if (pppVectPtr != (ttPppVectPtr)0)
    {
        pppLcpStatePtr = &pppVectPtr->pppsLcpState;
        if (    (pppLcpStatePtr->lcpsStateInfo.cpsState <= TM_PPPS_STOPPING)
#ifdef TM_USE_STOP_TRECK
             || (tm_16bit_one_bit_set(devPtr->devFlag2,
                                      TM_DEVF2_UNINITIALIZING))
#endif /* TM_USE_STOP_TRECK */
           )
        {
/* We are either closed or in a state we can jump to close, or uninitializing */
/*
 * Free saved send and recv VJ headers, plus current incoming packet if any.
 * Plus timers (if any).
 */
            tfPppFreeBuffersTimers(pppVectPtr);
#ifdef TM_PPP_LQM
            if (pppVectPtr->pppsLqmVectPtr)
            {
/*
 * The user called tfUsePppLqm, disable LQM on the link.
 */
                tfLqmDisable(pppVectPtr);
            }
#endif /* TM_PPP_LQM */

/*
 * Close IP Header Compression.  This must be done before PPP restores it's
 * default values (below) else memory could be lost.
 */
#ifdef TM_USE_IPHC
            if (pppVectPtr->pppsIphcVectPtr != (ttIphcVectPtr)0)
            {
                hdrCompressFuncPtr =
                    devPtr->devLinkLayerProtocolPtr->lnkHdrCompressFuncPtr;
                if (hdrCompressFuncPtr != (ttLnkHdrCompressFuncPtr)0)
                {
                     retCode =
                            (*(hdrCompressFuncPtr))(
                            TM_PACKET_NULL_PTR,
                            TM_32BIT_NULL_PTR,
                            (ttVoidPtrPtr) &(pppVectPtr->pppsIphcVectPtr),
                            TM_IPHC_CLOSE);
                }
            }
#endif /* TM_USE_IPHC */

#ifdef TM_USE_EAP
            tfEapClose(pppVectPtr->pppsEapVectPtr);
#endif /* TM_USE_EAP */

/*
 * Restore default settings, free PAP and CHAP strings, and set all of
 * our states back to the initial state
 */
            tfPppDefaultInit(pppVectPtr);
#ifdef TM_USE_STOP_TRECK
            if (tm_16bit_one_bit_set(devPtr->devFlag2,
                                     TM_DEVF2_UNINITIALIZING))
            {
#ifdef TM_USE_EAP
                tfEapDeInit(&pppVectPtr->pppsEapVectPtr);
#endif /* TM_USE_EAP */
#ifdef TM_USE_PPP_CALLBACK
                cbcpStatePtr = &pppVectPtr->pppsCallbackState;
                if (cbcpStatePtr->cbcpsAddressSetPtr != (tt8BitPtr)0)
                {
                    tm_free_raw_buffer(cbcpStatePtr->cbcpsAddressSetPtr);
                    cbcpStatePtr->cbcpsAddressSetPtr = (tt8BitPtr)0;
                }
#endif /* TM_USE_PPP_CALLBACK */
                if (    (pppVectPtr->pppsSendBufPtr !=
                                        &(pppVectPtr->pppsSendBuf[0]))
                     && (pppVectPtr->pppsSendBufPtr != (ttCharPtr)0) )
/* If we had previously allocated a send buffer, free it */
                {
                    tm_free_raw_buffer(
                        (ttRawBufferPtr)pppVectPtr->pppsSendBufPtr);
                    pppVectPtr->pppsSendBufSize = TM_PPP_DEF_BUF_SIZE;
                    pppVectPtr->pppsSendBufPtr = &(pppVectPtr->pppsSendBuf[0]);
#if (TM_PPP_DEF_SEND_SIZE > TM_PPP_DEF_BUF_SIZE)
/* Do not let the user make the send size bigger than the buffer size */
                    pppVectPtr->pppsSendBufSize = TM_PPP_DEF_BUF_SIZE;
#else /* !(TM_PPP_DEF_SEND_SIZE > TM_PPP_DEF_BUF_SIZE) */
                    pppVectPtr->pppsSendBufSize = TM_PPP_DEF_SEND_SIZE;
#endif /* !(TM_PPP_DEF_SEND_SIZE > TM_PPP_DEF_BUF_SIZE) */
                    pppVectPtr->pppsUserSendBufSize =
                                            pppVectPtr->pppsSendBufSize;
                }
                if (pppVectPtr->pppsPapState.papsLocalOurInfo.papUserNamePtr
                                != (tt8BitPtr)0)
                {
                    tm_free_raw_buffer(pppVectPtr->pppsPapState.
                                       papsLocalOurInfo.papUserNamePtr);
                    pppVectPtr->pppsPapState.papsLocalOurInfo.papUserNamePtr =
                                                                (tt8BitPtr)0;
                }
                if (pppVectPtr->pppsPapState.papsLocalOurInfo.papPasswordPtr
                                != (tt8BitPtr)0)
                {
                    tm_free_raw_buffer(pppVectPtr->pppsPapState.
                                       papsLocalOurInfo.papPasswordPtr);
                    pppVectPtr->pppsPapState.papsLocalOurInfo.papPasswordPtr =
                                                  (tt8BitPtr)0;
                }
                if (pppVectPtr->pppsChapState.chapsLocalOurInfo.chapUserNamePtr
                                != (tt8BitPtr)0)
                {
                    tm_free_raw_buffer(pppVectPtr->pppsChapState.
                                       chapsLocalOurInfo.chapUserNamePtr);
                    pppVectPtr->pppsChapState.chapsLocalOurInfo.
                                                chapUserNamePtr = (tt8BitPtr)0;
                }
                if (pppVectPtr->pppsChapState.chapsRemoteAllowInfo.chapUserNamePtr
                                != (tt8BitPtr)0)
                {
                    tm_free_raw_buffer(pppVectPtr->pppsChapState.
                                       chapsRemoteAllowInfo.chapUserNamePtr);
                    pppVectPtr->pppsChapState.chapsRemoteAllowInfo.
                                        chapUserNamePtr = (tt8BitPtr)0;
                }
                if (pppVectPtr->pppsChapState.chapsLocalOurInfo.chapSecretPtr
                                != (tt8BitPtr)0)
                {
                    tm_free_raw_buffer(pppVectPtr->pppsChapState.
                                       chapsLocalOurInfo.chapSecretPtr);
                    pppVectPtr->pppsChapState.chapsLocalOurInfo.
                                                chapSecretPtr = (tt8BitPtr)0;
                }
#ifdef TM_USE_PPP_MSCHAP
                if (pppVectPtr->pppsChapState.chapsLocalOurInfo.chapMsSecretPtr
                                != (tt8BitPtr)0)
                {
                    tm_free_raw_buffer(pppVectPtr->pppsChapState.
                                       chapsLocalOurInfo.chapMsSecretPtr);
                    pppVectPtr->pppsChapState.
                            chapsLocalOurInfo.chapMsSecretPtr = (tt8BitPtr)0;
                }
#endif /* TM_USE_PPP_MSCHAP */
#ifdef TM_USE_IPHC
                 if (pppVectPtr->pppsIphcVectPtr != (ttIphcVectPtr)0)
                 {
                    tm_free_raw_buffer(pppVectPtr->pppsIphcVectPtr);
                    pppVectPtr->pppsIphcVectPtr = (ttIphcVectPtr)0;
                 }
#endif /* TM_USE_IPHC */
                tm_free_raw_buffer(devPtr->devStateVectorPtr);
                devPtr->devStateVectorPtr = (ttPppVectPtr)0;
            }
#endif /* TM_USE_STOP_TRECK */
        }
        else
        {
/*
 * PPP is not closed yet (ie, we haven't recieved a terminate-ACK or
 * timed out on our terminate requests) so kick the state machine so
 * it send terminate requests and return EWOULDBLOCK
 * PM: Modified to only kick the state machine if we are in a state
 *     to all the sending of term requests
 */

            (void)tfPppStateMachine(pppVectPtr,
                              TM_PACKET_NULL_PTR,
                              TM_PPPE_CLOSE,
                              TM_PPP_LCP_INDEX);
            retCode = TM_EINPROGRESS;
        }
    }

    return retCode;
}

/*
 * Create the header for a packet destined for a PPP link
*/
#ifdef TM_LINT
LINT_UNACCESS_SYM_BEGIN(remoteOption)
#endif /* TM_LINT */
int tfPppSendPacket (ttPacketPtr packetPtr)
{
    ttPppHdrPtr             pppHdrPtr;
    ttPppVectPtr            pppVectPtr;
    ttLcpStatePtr           pppLcpStatePtr;
    ttDeviceEntryPtr        devPtr;
    tt32Bit                 accm;
    ttPktLen                pppHdrLength;
    int                     retCode;
#ifndef TM_DSP
    tt8Bit                  checkAddressCompression;
    tt8Bit                  checkProtocolCompression;
#endif /* !TM_DSP */
    tt32Bit                 remoteOption;
    tt16Bit                 pktType;
#ifndef TM_DSP
#if (defined(TM_USE_IPV4) || defined(TM_USE_IPHC))
    tt16Bit                 compressProto;
#endif /* TM_USE_IPV4 || TM_USE_IPHC */
#ifdef TM_USE_IPV4
    ttIpHeaderPtr           ipHdrPtr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPHC
    tt32Bit                 iphcPktType;
    ttLnkHdrCompressFuncPtr hdrCompressFuncPtr;
    tt8Bit                  compressHdr;
#endif /* TM_USE_IPHC */
#endif /* !TM_DSP */


    devPtr = packetPtr->pktDeviceEntryPtr;
    pppVectPtr = (ttPppVectPtr)(devPtr->devStateVectorPtr);
    pppLcpStatePtr = &pppVectPtr->pppsLcpState;
    tm_lock_wait(&(devPtr->devLockEntry));
    retCode = TM_ENOERROR;

    remoteOption = pppLcpStatePtr->lcpsRemoteAckedOptions;

#ifndef TM_DSP
/* Assume no compression */
    checkAddressCompression  = TM_8BIT_ZERO;
    checkProtocolCompression = TM_8BIT_ZERO;
#endif /* !TM_DSP */

    pktType = TM_16BIT_ZERO; /* avoid compiler warning. */

    if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
    {
        pktType = TM_PPP_IP_PROTOCOL;
    }
    else if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
    {
        pktType = TM_PPP_IPV6_PROTOCOL;
    }

#ifndef TM_DSP

/*
 * Do header compression (either VJ or IPHC) before forming the PPP header.
 * All header compression disabled for DSP targets.
 */
#ifdef TM_USE_IPHC
    compressHdr = TM_8BIT_ZERO;
#endif /* TM_USE_IPHC */

#ifdef TM_USE_IPV4
    if (  pppVectPtr->pppsIpcpState.ipcpsRemoteAckedOptions
        & (TM_UL(1)<<TM_IPCP_COMP_PROTOCOL))
    {
        compressProto =
           pppVectPtr->pppsIpcpState.ipcpsRemoteSetOptions.ipcpCompressProtocol;

        ipHdrPtr = (ttIpHeaderPtr)(packetPtr->pktLinkDataPtr);

        if (    (compressProto == TM_PPP_COMP_TCP_PROTOCOL)
             && (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
             && (ipHdrPtr->iphUlp == TM_IP_TCP))
        {
/* Van Jacobson TCP Header Compression */
            retCode = tfVjCompSendPacket(pppVectPtr,
                                         packetPtr,
                                         &pktType);
        }
#ifdef TM_USE_IPHC
        else
        {
            if (    (compressProto == TM_PPP_IPHC_PROTOCOL)
                 && (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER) )
            {
                compressHdr = TM_8BIT_YES;
            }
        }
#endif /* TM_USE_IPHC */
    }
#endif /* TM_USE_IPV4 */

#if (defined(TM_USE_IPV6) && defined(TM_USE_IPHC))
    if (  pppVectPtr->pppsIp6cpState.ip6cpsRemoteAckedOptions
        & (TM_UL(1)<<TM_IPV6CP_COMP_PROTOCOL))
    {
        compressProto = pppVectPtr->pppsIp6cpState.
                        ip6cpsRemoteSetOptions.ip6cpCompressProtocol;

        if (    (compressProto == TM_PPP_IPHC_PROTOCOL)
             && (packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER) )
        {
            compressHdr = TM_8BIT_YES;
        }
    }
#endif /* TM_USE_IPV6 && TM_USE_IPHC */

#ifdef TM_USE_IPHC
    if (compressHdr == TM_8BIT_YES)
    {
        iphcPktType = (tt16Bit)pktType;
        hdrCompressFuncPtr =
            devPtr->devLinkLayerProtocolPtr->lnkHdrCompressFuncPtr;
        if (hdrCompressFuncPtr != (ttLnkHdrCompressFuncPtr)0)
        {
            retCode =
                (*(hdrCompressFuncPtr))(
                    packetPtr,
                    &iphcPktType,
                    (ttVoidPtrPtr) &(pppVectPtr->pppsIphcVectPtr),
                    TM_IPHC_COMPRESS);
        }
        pktType = (tt16Bit)iphcPktType;
    }
#endif /* TM_USE_IPHC */

#endif /* !TM_DSP */

/* Compute PPP header length */
    pppHdrLength = TM_PPP_HDR_BYTES;

/* Point to PPP header */
    pppHdrPtr =
        (ttPppHdrPtr) (packetPtr->pktLinkDataPtr - TM_PAK_PPP_HEADER_SIZE);

/* Check to see if this packet came from PPP itself */
    if (packetPtr->pktNetworkLayer == TM_NETWORK_LINK_LAYER)
    {
        if (    packetPtr->pktSharedDataPtr->dataLinkLayerProtocol
             == TM_16BIT_ZERO)
        {
/* We are reusing a packet so we keep the header */
            packetPtr->pktSharedDataPtr->dataLinkLayerProtocol =
                                               pppHdrPtr->pppHdrProtocol;
#ifndef TM_DSP
            if (pppHdrPtr->pppHdrProtocol != TM_PPP_LCP_PROTOCOL)
            {
                checkAddressCompression = TM_8BIT_YES;
            }
#endif /* !TM_DSP */
        }
        else
        {
            pppHdrPtr->pppHdrAddress = TM_PPP_BROADCAST_CHAR;
            pppHdrPtr->pppHdrControl = TM_PPP_UA_FRAME_CHAR;
            pppHdrPtr->pppHdrProtocol = packetPtr->pktSharedDataPtr->
                                                        dataLinkLayerProtocol;
#ifndef TM_DSP
            if (pppHdrPtr->pppHdrProtocol != TM_PPP_LCP_PROTOCOL)
            {
                checkAddressCompression = TM_8BIT_YES;
                checkProtocolCompression = TM_8BIT_YES;
            }
#endif /* !TM_DSP */
        }
    }
    else
    {
/* An IP Frame so check for compression */
        pppHdrPtr->pppHdrAddress  = TM_PPP_BROADCAST_CHAR;
        pppHdrPtr->pppHdrControl  = TM_PPP_UA_FRAME_CHAR;
#ifdef TM_DSP
/*
 * Note that VJ compression is not supported on DSP platform. So there
 * is no need to worry about pppHdrProtocol being on an odd byte boundary.
 */
        pppHdrPtr->pppHdrProtocol = pktType;
#else /* !TM_DSP */
        checkAddressCompression   = TM_8BIT_YES;
        checkProtocolCompression  = TM_8BIT_YES;
/*
 * Note that because of VJ compression &pppHdrPtr->pppHdrProtocol could
 * be on an od byte boundary.
 */
        ((tt8BitPtr)&(pppHdrPtr->pppHdrProtocol))[0] = ((tt8BitPtr)&pktType)[0];
        ((tt8BitPtr)&(pppHdrPtr->pppHdrProtocol))[1] = ((tt8BitPtr)&pktType)[1];
#endif /* !TM_DSP */
    }
#ifndef TM_DSP
/* No address/protocol field compression or VJ compression on DSP */
#ifdef TM_LINT
LINT_BOOLEAN_BEGIN
#endif /* TM_LINT */
    if (    (checkAddressCompression != TM_8BIT_ZERO)
         && (remoteOption & (TM_UL(1)<<TM_LCP_ADDRCONTROL_COMP)) )
#ifdef TM_LINT
LINT_BOOLEAN_END
#endif /* TM_LINT */
    {
/* Address field is compressed */
        pppHdrLength -=   TM_8BIT_BYTE_COUNT + TM_8BIT_BYTE_COUNT;
    }
#ifdef TM_LINT
LINT_BOOLEAN_BEGIN
#endif /* TM_LINT */
    if (    (checkProtocolCompression != TM_8BIT_ZERO)
         && (remoteOption & (TM_UL(1)<<TM_LCP_PROTOCOL_COMP)) )
#ifdef TM_LINT
LINT_BOOLEAN_END
#endif /* TM_LINT */
    {
/* Protocol field is compressed. Move control and address fields down */
/*
 * Note that because of VJ compression &pppHdrPtr->pppHdrProtocol could
 * be on an odd byte boundary;
 */
        if (((tt8BitPtr)&(pppHdrPtr->pppHdrProtocol))[0] == 0x00)
        {
            ((tt8BitPtr)&(pppHdrPtr->pppHdrProtocol))[0] =
                                                    pppHdrPtr->pppHdrControl;
            pppHdrPtr->pppHdrControl = pppHdrPtr->pppHdrAddress;
            pppHdrLength--;
        }
    }
#endif /* !TM_DSP */
/* Adjust packet pointer and lengths with PPP header size */
    packetPtr->pktLinkDataPtr -= tm_packed_byte_count(pppHdrLength);
    packetPtr->pktChainDataLength += pppHdrLength;
    packetPtr->pktLinkDataLength  += pppHdrLength;

/* Most common case */
#ifdef TM_LINT
LINT_BOOLEAN_BEGIN
#endif /* TM_LINT */
    if (retCode == TM_ENOERROR)
#ifdef TM_LINT
LINT_BOOLEAN_END
#endif /* TM_LINT */
    {
        accm = pppLcpStatePtr->lcpsRemoteSetOptions.lcpAccm;
        if (    ( packetPtr->pktNetworkLayer == TM_NETWORK_LINK_LAYER )
             && (    packetPtr->pktSharedDataPtr->dataLinkLayerProtocol
                  == (unsigned int) TM_PPP_LCP_PROTOCOL ) )
        {
            accm = tlLcpDefaultOptions.lcpAccm;
        }
        packetPtr->pktPppAccm = accm;
    }
    tm_call_unlock(&(devPtr->devLockEntry));
    return (retCode);
}
#ifdef TM_LINT
LINT_UNACCESS_SYM_END(remoteOption)
#endif /* TM_LINT */

/*
 * This is the post link send function. It is called without
 * the device lock on, but with the device driver lock on.
 * Called
 * either
 *  by tfDeviceSend() after packet has been queued, and with
 *  the device driver lock on, if the transmit task option has not
 *  been set by the user,
 * or
 *  from the tfXmitInterface() function, itself called in the context
 *  of the transmit task, and with the device driver lock on if the
 *  transmit task option has been set by the user.
 *
 * Stuff the bytes of the PPP packet so it can be passed to the
 * device driver (piece at a time)
 */
int tfPppStuffPacket (ttPacketPtr packetPtr)
{
    ttDevSendFuncPtr devSendFuncPtr;
    ttDeviceEntryPtr devPtr;
    ttPppVectPtr     pppVectPtr;
    ttCharPtr        sendStartPtr;
    ttCharPtr        sendWorkingPtr;
    ttCharPtr        dataPtr;
    tt32Bit          accm;
    ttPktLen         length;
    int              i;
    int              endStuffMaxIndex;
    int              errorCode;
    tt16Bit          workingFcs;
    tt16Bit          sendSize;
    tt16Bit          increasedSendSize;
    tt16Bit          sendBufSize;
    char             dataChar;
    char             endStuffChar[5];
#ifdef TM_DSP
    unsigned int     dataOffset;
#endif /* TM_DSP */

#ifdef TM_SNMP_MIB
    tt32Bit          outOctets;
#endif /* TM_SNMP_MIB */
#ifdef TM_PPP_LQM
    tt32Bit          lqmOutOctets;
#endif /* TM_PPP_LQM */

#ifdef TM_SNMP_MIB
    outOctets = TM_UL(0);
#endif /* TM_SNMP_MIB */
#ifdef TM_PPP_LQM
    lqmOutOctets = TM_UL(0);
#endif /* TM_PPP_LQM */

    devPtr = packetPtr->pktDeviceEntryPtr;
    devSendFuncPtr = devPtr->devSendFuncPtr;
    pppVectPtr = (ttPppVectPtr)devPtr->devStateVectorPtr;
/*
 * If we're using a send buffer that's larger than one byte
 * (must be explicitly set by user), then pppsSendBufPtr points
 * to a buffer allocated in tfPppOpen().
 */
    sendBufSize = pppVectPtr->pppsSendBufSize;
    sendStartPtr = pppVectPtr->pppsSendBufPtr;
    sendWorkingPtr=sendStartPtr;
    sendSize = (tt16Bit)0;
/* sendSize will loop among 0,1,2,3(C3) or 0,1 (C5)*/
    increasedSendSize=sendSize;
/* increasedSendSize keeps increasing until reaches sendBufSize*/
    length = packetPtr->pktLinkDataLength;
    dataPtr = (ttCharPtr)packetPtr->pktLinkDataPtr;
#ifdef TM_DSP
    dataOffset = (unsigned int) packetPtr->pktLinkDataByteOffset;
#endif /* TM_DSP */
    accm = packetPtr->pktPppAccm;
    workingFcs = TM_PPP_FCS_INITIAL;
#ifdef TM_SNMP_MIB
    outOctets += length + 1; /* TM_PPP_FLAG_CHAR + length */
#endif /* TM_SNMP_MIB */
#ifdef TM_PPP_LQM
    lqmOutOctets += length + 1; /* TM_PPP_FLAG_CHAR + length */
#endif /* TM_PPP_LQM */
/* Copy TM_PPP_FLAG_CHAR */
/* Send the start of frame flag character in send buffer*/
    tm_ppp_put_char_next(sendWorkingPtr, sendSize, TM_PPP_FLAG_CHAR);
    increasedSendSize++;
/*
 * If the data character fills out the buffer (buffer size == 1) then send
 * it out to the driver.
 */
    if (increasedSendSize == sendBufSize)
    {
/* Send the buffer to the driver */
        (void)(*(devSendFuncPtr))( (ttUserInterface)devPtr,
                                   (ttCharPtr)sendStartPtr,
                                   sendBufSize,
                                   TM_USER_BUFFER_MORE );
/* Reset write index to start. */
        sendSize = (tt16Bit)0;
        increasedSendSize=0;
/* Reset the sendWorkingPtr to start. */
        sendWorkingPtr=sendStartPtr;

    }
/* Loop to send all the characters */
    while (length != (ttPktLen)0)
    {
        dataChar = tm_ppp_get_char_next(dataPtr, dataOffset);
/* Compute the FCS */
        workingFcs = (tt16Bit)((workingFcs>>8)^
                       tlFcsLookupCrc16[(workingFcs^dataChar)&0xff]);
/* Does it need stuffing */
        if (tm_ppp_need_stuff(dataChar,accm))
        {
/* It does so first send the escape */
#ifdef TM_SNMP_MIB
            outOctets++;
#endif /* TM_SNMP_MIB */
/*
 * Add this character to the buffer. If this fills out the buffer then send
 * it out to the driver.
 */
            tm_ppp_put_char_next(sendWorkingPtr, sendSize, TM_PPP_ESC_CHAR);
            increasedSendSize++;

            if (increasedSendSize == sendBufSize)
            {
/* Send the buffer to the driver */
                (void)(*(devSendFuncPtr))( (ttUserInterface)devPtr,
                                           (ttCharPtr)sendStartPtr,
                                           sendBufSize,
                                           TM_USER_BUFFER_MORE );
/* Reset write index to start. */
                sendSize = (tt16Bit)0;
                increasedSendSize=0;
/* Reset the sendWorkingPtr to start. */
                sendWorkingPtr=sendStartPtr;
            }


/* Now send the character (XOR'ed with 0x20) */
            dataChar ^= TM_PPP_STUFF_CHAR;
        }
/*
 * Add this character to the buffer. If this fills out the buffer then send
 * it out to the driver.
 */
        tm_ppp_put_char_next(sendWorkingPtr, sendSize, dataChar);
        increasedSendSize++;

        if (increasedSendSize == sendBufSize)
        {
/* Send the buffer to the driver */
            (void)(*(devSendFuncPtr))( (ttUserInterface)devPtr,
                                       (ttCharPtr)sendStartPtr,
                                       sendBufSize,
                                       TM_USER_BUFFER_MORE );
/* Reset write index to start. */
            sendSize = (tt16Bit)0;
            increasedSendSize=0;
/* Reset the sendWorkingPtr to start. */
            sendWorkingPtr=sendStartPtr;
        }
        length--;
        if( length == (ttPktLen)0 )
        {
            packetPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
            if (packetPtr != TM_PACKET_NULL_PTR)
            {
                length = packetPtr->pktLinkDataLength;
                dataPtr = (ttCharPtr)packetPtr->pktLinkDataPtr;
#ifdef TM_DSP
                dataOffset = (unsigned int) packetPtr->pktLinkDataByteOffset;
#endif /* TM_DSP */
#ifdef TM_SNMP_MIB
                outOctets += length;
#endif /* TM_SNMP_MIB */
#ifdef TM_PPP_LQM
                lqmOutOctets += length;
#endif /* TM_PPP_LQM */
            }
        }
    }
/*
 * Copy the end of frame sequence and end of frame flag into a local
 * array before calling the driver send to reduce code size.
 */

/* The end of frame sequence */
    workingFcs ^= (unsigned short)0xffff;
    dataChar = (char)(workingFcs & (tt16Bit)0x00ff);
    endStuffMaxIndex = 0;
    if (tm_ppp_need_stuff(dataChar,accm))
    {
        endStuffChar[0] = TM_PPP_ESC_CHAR;
        endStuffMaxIndex++;
        dataChar ^= TM_PPP_STUFF_CHAR;
    }
    endStuffChar[endStuffMaxIndex++] = dataChar;
    dataChar = (char)((workingFcs>>8) & (tt16Bit)0x00ff);
    if (tm_ppp_need_stuff(dataChar,accm))
    {
        endStuffChar[endStuffMaxIndex++] = TM_PPP_ESC_CHAR;
        dataChar ^= TM_PPP_STUFF_CHAR;
    }
    endStuffChar[endStuffMaxIndex++] = dataChar;
    endStuffChar[endStuffMaxIndex] = TM_PPP_FLAG_CHAR;
#ifdef TM_SNMP_MIB
    outOctets = outOctets + (unsigned)endStuffMaxIndex + 1;
#endif /* TM_SNMP_MIB */
#ifdef TM_PPP_LQM
    lqmOutOctets = lqmOutOctets + 2; /* count 2-byte FCS */
#endif /* TM_PPP_LQM */
    for ( i = 0; i < endStuffMaxIndex; i++ )
    {
/*
 * Add this character to the buffer. Send the buffer to the driver, if
 * this is the last character, or if the buffer is full.
 */
        tm_ppp_put_char_next(sendWorkingPtr, sendSize, endStuffChar[i]);
        increasedSendSize++;


        if ( increasedSendSize == sendBufSize )
        {
/* Send the buffer to the driver */
            (void)(*(devSendFuncPtr))( (ttUserInterface)devPtr,
                                       (ttCharPtr)sendStartPtr,
                                       sendBufSize,
                                       TM_USER_BUFFER_MORE );
/* Reset write index to start. */
            sendSize = (tt16Bit)0;
            increasedSendSize=0;
/* Reset the sendWorkingPtr to start. */
            sendWorkingPtr=sendStartPtr;

        }
    }
    tm_ppp_put_char_next(sendWorkingPtr, sendSize,
                         (endStuffChar[endStuffMaxIndex]));
    increasedSendSize++;
/*
 * If the device driver function returns an error on the last buffer,
 * this function returns an error to its caller (either tfDeviceSend(),
 * or tfXmitInterface() (if a transmit task is used), in which case
 * the caller will call tfDeviceSendFailed() which will remove the
 * last queued packet from the send queue.
 */
/* Send the buffer to the driver */
    errorCode = (*(devSendFuncPtr))((ttUserInterface)devPtr,
                                    (ttCharPtr)sendStartPtr,
                                    increasedSendSize,
                                    TM_USER_BUFFER_LAST);
#ifdef TM_SNMP_MIB
#ifdef TM_USE_IPV4
    if (errorCode == TM_ENOERROR)
    {
        tm_64Bit_augm(devPtr->dev4Ipv4Mib.ipIfStatsHCOutOctets,
                      outOctets);
#ifdef TM_USE_NG_MIB2
        tm_64Bit_augm(tm_context(tvDevIpv4Data).ipIfStatsHCOutOctets,
                      outOctets);
#endif /* TM_USE_NG_MIB2 */
    }
#endif /* TM_USE_IPV4 */
/*
 * In case of an error interface SNMP MIB out variables updated in
 * tfDeviceSendFailed(), itself called from either tfXmitInterface() or
 * tfDeviceSend() when this function returns an error.
 */
#endif /* TM_SNMP_MIB */
#ifdef TM_PPP_LQM
    if (errorCode == TM_ENOERROR)
    {
        devPtr->devLqmOutOctets += lqmOutOctets;
    }
#endif /* TM_PPP_LQM */
    return errorCode;
}

/*
 * Process an incoming PPP Frame (Async Only)
 * This is called by the deviceRecv which is called by the user.
 * The user calls device receive to get the data into a packet
 * WE DO NOT EXPECT ANY FRAME DELINIATION WHEN THIS FUNCTION IS CALLED
*/
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(outDataPtr)
LINT_NULL_PTR_BEGIN(pppHdrPtr)
#endif /* TM_LINT */
int tfPppAsyncRecv(void TM_FAR *interfaceId, void TM_FAR *bufferHandle)
{
    ttPppVectPtr     pppVectPtr;
    ttLcpStatePtr    pppLcpStatePtr;
    ttPppHdrPtr      pppHdrPtr;
    ttPacketPtr      packetPtr;
    ttPacketPtr      devPacketPtr;
#ifdef TM_USE_DRV_SCAT_RECV
    ttPacketPtr      nextDevPacketPtr;
#endif /* TM_USE_DRV_SCAT_RECV */
    tt8BitPtr        inDataPtr;
    tt8BitPtr        outDataPtr;
    ttDeviceEntryPtr devPtr;
    tt32Bit          location;
    tt32Bit          inputMask;
    ttPktLen         incomingPacketLen;
#ifdef TM_PPP_LQM
    tt32Bit          inGoodFrameOctets;
#endif /* TM_PPP_LQM */
    unsigned         maxCopySize;
    int              errorCode;
    ttPktLen         linkDataLength;
    tt16Bit          pppProtocol;
    tt16Bit          workingFcs;
#ifdef TM_USE_IPV4
    tt8Bit           i;
    tt8Bit           vjCompFlag;
    tt8Bit           vjOptionFlags;
    tt8Bit           vjOffset;
#endif /* TM_USE_IPV4 */
    tt8Bit           byteStuffFlag;
    tt8Bit           needFreePacket;
    tt8Bit           inDataChar;
    tt8Bit           sendRejectStatus;
    tt8Bit           ipPacket;
#ifdef TM_USE_IPHC
    ttLnkHdrCompressFuncPtr hdrDecompressFuncPtr;
    tt32Bit          pktType;
#endif /* TM_USE_IPHC */
#ifdef TM_DSP
    unsigned int     inDataOffset;
    unsigned int     outDataOffset;
#endif /* TM_DSP */


    devPtr = (ttDeviceEntryPtr)interfaceId;
    devPacketPtr = (ttPacketPtr)bufferHandle;

/*
 * We need to receive a low as one byte at a time, we will remove the
 * byte stuffing (if there is any) and  align the packet for
 * address/control, protocol and VJ compression as the packet is
 * read in.  We also compute the FCS here to avoid multiple loops
 * through the packet.
 * Even though we can handle one byte at a time, it is EXTREMELY
 * inefficent to do so because the packet handle is allocated for
 * each call into the stack from the driver.
 */

    tm_zero_dsp_offset(inDataOffset);

    errorCode = TM_ENOERROR;
    tm_lock_wait(&(devPtr->devLockEntry));

    pppVectPtr = (ttPppVectPtr)devPtr->devStateVectorPtr;

/*
 * Check if the PPP state vector has not been allocated.  This could occur
 * if the user has not set any PPP options and data is received after the
 * device is opened but before PPP is opened.  Free the incoming data, unlock
 * the device and return.
 */
    if (pppVectPtr == TM_PPP_VECT_NULL_PTR)
    {
        goto pppAsyncRecvReturn;
    }
    pppLcpStatePtr = &pppVectPtr->pppsLcpState;

/* Restore our last receive session */
#ifdef TM_PPP_LQM
    inGoodFrameOctets = pppVectPtr->pppsInGoodFrameOctets;
#endif /* TM_PPP_LQM */
    packetPtr = pppVectPtr->pppsPacketPtr;
    byteStuffFlag = pppVectPtr->pppsLastByteStuffState;

#ifdef TM_USE_IPV4
    vjCompFlag = pppVectPtr->pppsVjCompFlag;
    vjOffset = pppVectPtr->pppsVjOffset;
    vjOptionFlags = pppVectPtr->pppsVjOptionFlags;
#endif /* TM_USE_IPV4 */

    workingFcs = pppVectPtr->pppsWorkingFcs;
    inputMask = pppLcpStatePtr->lcpsLocalGotOptions.lcpAccm;
    tm_set_dsp_offset(outDataOffset, pppVectPtr->pppsWorkingPtrOffset);

/* Enough room to expand the PPP header and VJ header (in bytes) */
    maxCopySize =
        pppLcpStatePtr->lcpsLocalGotOptions.lcpMaxReceiveUnit +
        TM_PPP_HDR_BYTES + TM_VJ_COMP_SIZE + 2;

    pppHdrPtr = (ttPppHdrPtr)0;
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        outDataPtr = pppVectPtr->pppsWorkingPtr;
        pppHdrPtr = (ttPppHdrPtr)packetPtr->pktLinkDataPtr;
    }
    else
    {
        outDataPtr = TM_8BIT_NULL_PTR;
    }
#ifdef TM_USE_DRV_SCAT_RECV
    nextDevPacketPtr = devPacketPtr;
    do
    {
        linkDataLength = nextDevPacketPtr->pktLinkDataLength;
        inDataPtr = nextDevPacketPtr->pktLinkDataPtr;
#else /* !TM_USE_DRV_SCAT_RECV */
        linkDataLength = devPacketPtr->pktLinkDataLength;
        inDataPtr = devPacketPtr->pktLinkDataPtr;
#endif /* TM_USE_DRV_SCAT_RECV */
        while (linkDataLength != (ttPktLen)0)
        {
            inDataChar = tm_ppp_get_char_offset(inDataPtr,inDataOffset);
            if (    (    inDataChar
                      == TM_PPP_FLAG_CHAR )
                 || (pppVectPtr->pppsEndFrame == TM_8BIT_YES))
            {
/* Either the start or end of a frame */
                if (packetPtr != TM_PACKET_NULL_PTR)
/* End of frame */
                {
/* Compute the length of the incoming data already handled */
                    incomingPacketLen = (ttPktLen)
                         tm_byte_count(outDataPtr - packetPtr->pktLinkDataPtr);
                    tm_inc_dsp_offset(incomingPacketLen, outDataOffset);
                    if ( incomingPacketLen != (ttPktLen)0 )
                    {
                        pppVectPtr->pppsEndFrame = TM_8BIT_YES;
/* Disconnect this packet from the state machine */
                        pppVectPtr->pppsPacketPtr = TM_PACKET_NULL_PTR;
/* Check the FCS that we were working on, and check packet size */
                        if (    (workingFcs == TM_PPP_FCS_FINAL)
                             && (incomingPacketLen >= (ttPktLen)
                                    (TM_PPP_HDR_BYTES + TM_16BIT_BYTE_COUNT)) )
                        {
/* No need to free the packet unless an error occurs */
                            needFreePacket = TM_8BIT_ZERO;
/* Move the link data ptr past the ppp Header*/
                            packetPtr->pktLinkDataPtr += TM_PAK_PPP_HEADER_SIZE;
/* Compute the size less the PPP header, and less the frame check sequence */
                            packetPtr->pktLinkDataLength =
                                         incomingPacketLen -
                                         (ttPktLen)(TM_PPP_HDR_BYTES +
                                         TM_16BIT_BYTE_COUNT); /* workingFCS */
                            packetPtr->pktChainDataLength =
                                                packetPtr->pktLinkDataLength;
                            workingFcs = TM_PPP_FCS_INITIAL;
                            pppProtocol = pppHdrPtr->pppHdrProtocol;
/*
 * Check to make sure that we are NOT negotiating LCP when another
 * protocol comes in.  If it does than the RFC1661 says to silently
 * discard it.
 */
                            if (    (pppProtocol != TM_PPP_LCP_PROTOCOL)
                                 && (pppLcpStatePtr->lcpsStateInfo.cpsState
                                                         != TM_PPPS_OPENED ) )
                            {
/* Silently discard non lcp packets when LCP is NOT in the open state */
                                needFreePacket = TM_8BIT_YES;
                            }
                            else
                            {

#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
/* [RFC1989].R2.3:20 -
   SaveInPackets must include the expected values for this LQR. */
                                devPtr->devInUcastPkts++;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */

#ifdef TM_PPP_LQM
/* [RFC1989].R2.3:20 -
   SaveInOctets must include the expected values for this LQR. */
                                devPtr->devLqmInGoodOctets +=
                                                           inGoodFrameOctets;
#endif /* TM_PPP_LQM */

/* It's either an LCP packet or LCP is OPEN */
#ifdef TM_USE_IPHC
processIncomingPkt:
#endif /* TM_USE_IPHC */
                                sendRejectStatus = TM_8BIT_NO;
                                ipPacket = TM_8BIT_NO;
                                switch (pppProtocol)
                                {
#ifdef TM_USE_IPV4
/* VJ compression disabled for DSP targets. */
#ifndef TM_DSP
/* A compressed or uncompressed TCP/IPv4 datagram */
                                    case TM_PPP_COMP_TCP_PROTOCOL:
                                    case TM_PPP_UNCOMP_TCP_PROTOCOL:
                                        errorCode =
                                            tfVjCompIncomingPacket(pppVectPtr,
                                                                   packetPtr);
                                        if (errorCode != TM_ENOERROR)
                                        {
/* VJ uncompression failed. Discard the packet */
                                            needFreePacket = TM_8BIT_YES;
#ifdef TM_PPP_LQM
/* [RFC1989].R2.2:30, [RFC1989].R2.2:80 -
   InGoodOctets MUST NOT count octets for frames that are counted in
   ifInDiscards and ifInErrors. */
                                            devPtr->devLqmInGoodOctets -=
                                                    inGoodFrameOctets;
#endif /* TM_PPP_LQM */
                                            break;
                                        }
/* If VJ decompression suceeds, fall through to IPv4 case */
                                        pppProtocol = TM_PPP_IP_PROTOCOL;
#endif /* !TM_DSP */

/* An IPv4 datagram */
                                    case TM_PPP_IP_PROTOCOL:
                                        if (pppVectPtr->pppsIpcpState.
                                                ipcpsStateInfo.cpsState ==
                                            TM_PPPS_OPENED)
                                        {
                                            tm_unlock(&(devPtr->devLockEntry));
                                            packetPtr->pktDeviceEntryPtr =
                                                                  devPtr;
                                            tfIpIncomingPacket(
                                                packetPtr
#ifdef TM_USE_IPSEC_TASK
                                                , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                                );
                                            tm_lock_wait(
                                                &(devPtr->devLockEntry));
                                        }
                                        else
                                        {
                                            needFreePacket = TM_8BIT_YES;
                                        }
                                        break;

/* Internet Protocol Control Protocol (IPCP) a NCP */
                                    case TM_PPP_IPCP_PROTOCOL:
                                        (void)tfIpcpIncomingPacket(pppVectPtr,
                                                                   packetPtr);
                                        break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
/* IPv6 datagram
 *     Only process IPv6 packets if IPV6CP is open.
 *     ([RFC2472].R2:10, [RFC2472].R3:20)
 */
                                    case TM_PPP_IPV6_PROTOCOL:
                                        if (pppVectPtr->pppsIp6cpState.
                                                ip6cpsStateInfo.cpsState ==
                                            TM_PPPS_OPENED)
                                        {
                                            ipPacket = TM_8BIT_YES;
                                        }
                                        else
                                        {
/* IPV6CP not up yet, so drop any IPv6 packets received on the link. */
                                            needFreePacket = TM_8BIT_YES;
                                        }
                                        break;

/*
 * Internet Protocol Control Protocol (IPCP) a NCP
 *    Only process IPV6CP packets if PPP has reached the NCP phase.
 *    ([RFC2472].R3:10, [RFC2472].R3:20)
 */
                                    case TM_PPP_IPV6CP_PROTOCOL:
                                        if (    (pppVectPtr->pppsIp6cpState.
                                                 ip6cpsStateInfo.cpsState >
                                                 TM_PPPS_INITIAL)
#ifdef TM_USE_IPV4
                                             || (pppVectPtr->pppsIpcpState.
                                                 ipcpsStateInfo.cpsState >
                                                 TM_PPPS_INITIAL)
#endif /* TM_USE_IPV4 */
                                            )
                                        {
                                            tfIp6cpIncomingPacket(pppVectPtr,
                                                                  packetPtr);
                                        }
                                        else
                                        {
/* LCP/authentication not up yet, drop any NCP packets. */
                                            needFreePacket = TM_8BIT_YES;
                                        }
                                        break;

#endif /* TM_USE_IPV6 */

/* Link Control Protocol (LCP) */
                                    case TM_PPP_LCP_PROTOCOL:
                                        (void)tfLcpIncomingPacket( pppVectPtr,
                                                                   packetPtr );
                                        break;

/* Simple Password Authentication Protocol */
                                    case TM_PPP_PAP_PROTOCOL:
                                        (void)tfPapIncomingPacket( pppVectPtr,
                                                                   packetPtr );
                                        break;

/* Challenge Handshake Authentication Protocol */
                                    case TM_PPP_CHAP_PROTOCOL:
                                        (void)tfChapIncomingPacket(
                                            pppVectPtr, packetPtr );
                                        break;

#ifdef TM_PPP_LQM
                                    case TM_PPP_LQR_PROTOCOL:
                                        if (tfLqmIsEnabled(pppVectPtr))
                                        {
/* [RFC1989].R2.2:20 -
   InLQRs is a 32-bit counter which increases by one for each received
   Link-Quality-Report packet. */
                                            devPtr->devLqmInLqrs++;

                                            (void)tfLqmIncomingPacket(
                                                    pppVectPtr, packetPtr);
                                        }
                                        else
                                        {
/* else, LQM is disabled on the link, send a protocol reject */
                                            sendRejectStatus = TM_8BIT_YES;
                                        }
                                        break;
#endif /* TM_PPP_LQM */

#ifdef TM_USE_EAP
                                    case TM_PPP_EAP_PROTOCOL:
                                        tfEapIncomingPacket(
                                                pppVectPtr->pppsEapVectPtr,
                                                packetPtr);
                                        break;
#endif /* TM_USE_EAP */

#ifdef TM_USE_PPP_CALLBACK
                                    case TM_PPP_CBCP_PROTOCOL:
                                        tfCbcpIncomingPacket( pppVectPtr,
                                                              packetPtr );
                                        break;
#endif /* TM_USE_PPP_CALLBACK */

#ifdef TM_USE_IPHC
/*
 * When a compressed packet is received, pass it to IPHC for decompression
 * (through lnkHdrCompressFuncPtr).  If the returned packet is an IP packet,
 * pass it to the appropriate (IPv4 or IPv6) routine; otherwise, if there was
 * an error during decompression, just drop the packet.
 */
                                    case TM_PPP_IPHC_FULL_HDR:
                                    case TM_PPP_IPHC_COMP_TCP:
                                    case TM_PPP_IPHC_COMP_NON_TCP:
                                    case TM_PPP_IPHC_CTX_STATE:

                                        pktType = pppProtocol;
                                        hdrDecompressFuncPtr =
                                            devPtr->
                                            devLinkLayerProtocolPtr->
                                            lnkHdrCompressFuncPtr;
                                        if (hdrDecompressFuncPtr
                                                != (ttLnkHdrCompressFuncPtr)0)
                                        {
                                            errorCode =
                                            (*(hdrDecompressFuncPtr))(
                                                packetPtr,
                                                &pktType,
                                                (ttVoidPtrPtr)
                                                 &(pppVectPtr->pppsIphcVectPtr),
                                                TM_IPHC_DECOMPRESS);
                                        }
                                        if (errorCode == TM_ENOERROR)
                                        {
/* Packet is decompressed - jump back and allow decompressed packet to be
   processed and passed up the stack. */
                                            pppProtocol = (tt16Bit) pktType;
                                            goto processIncomingPkt;
                                        }
                                        else
                                        {
/* An error occured while decompressing, so discard packet. */
                                            needFreePacket = TM_8BIT_YES;
                                        }
                                        break;
#endif /* TM_USE_IPHC */

                                    default:
/* If we hit the default case then we do NOT support this protocol */
                                        sendRejectStatus = TM_8BIT_YES;
                                        break;
                                }
                                if (ipPacket != TM_8BIT_NO)
                                {
/* Unlock the device */
                                    tm_unlock(&(devPtr->devLockEntry));
                                    packetPtr->pktDeviceEntryPtr =
                                                                  devPtr;
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPDUAL
                                    if (pppProtocol == TM_PPP_IP_PROTOCOL)
#endif /* TM_USE_IPDUAL */
                                    {
                                        tfIpIncomingPacket(
                                                 packetPtr
#ifdef TM_USE_IPSEC_TASK
                                                 , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                                 );
                                    }
#ifdef TM_USE_IPDUAL
                                    else
#endif /* TM_USE_IPDUAL */
#endif /* TM_USE_IPV4 */
                                    {
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPDUAL
                                        if (pppProtocol ==
                                                        TM_PPP_IPV6_PROTOCOL)
#endif /* TM_USE_IPDUAL */
                                        {
                                            tf6IpIncomingPacket(
                                                packetPtr
#ifdef TM_USE_IPSEC_TASK
                                                , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                                );
                                        }
#endif /* TM_USE_IPV6 */
                                    }
/* Relock the device */
                                    tm_lock_wait(&(devPtr->devLockEntry));
                                }
                                else
                                {
                                    if (sendRejectStatus != TM_8BIT_NO)
                                    {
/* send a protocol reject */
#ifdef TM_SNMP_MIB
#ifdef TM_USE_IPV4
                                        devPtr->dev4Ipv4Mib.
                                            ipIfStatsInUnknownProtos++;
#ifdef TM_USE_NG_MIB2
                                        tm_context(tvDevIpv4Data).
                                            ipIfStatsInUnknownProtos++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV4 */
#endif /* TM_SNMP_MIB */
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
                                        devPtr->devInUcastPkts--;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */

/* Send a protocol reject for this packet */
                                        errorCode = tfLcpSendProtocolReject(
                                            pppVectPtr,
                                            packetPtr);
                                    }
                                }
                            }
                        }
                        else
                        {
/* Bad packet so Silently discard it */
                            needFreePacket = TM_8BIT_YES;
                        }
                        if (needFreePacket != TM_8BIT_ZERO)
                        {
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
/* Bad packet */
                            devPtr->devInErrors++;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
                            tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
                        }
                        packetPtr = TM_PACKET_NULL_PTR;
                    }
                }
                else
/* Start of frame */
                {
                    pppVectPtr->pppsEndFrame = TM_8BIT_NO;
#ifdef TM_PPP_LQM
/* [RFC1989].R2.3:10 - count FCS octets and one flag octet per frame */
                    inGoodFrameOctets = sizeof(workingFcs) + 1;
                    pppVectPtr->pppsInGoodFrameOctets = 0;
#endif /* TM_PPP_LQM */

/*
 * We had no packet before, so create a new one . Allow room to grow upward
 * (TM_MAX_HEADERS_SIZE) for uncompression and forwarding.
 */
                    packetPtr = tfGetSharedBuffer(
/*
 * Enough room to expand IP options, TCP options, another link layer
 * (if we forward)
 */
                                (int)(  TM_MAX_HEADERS_SIZE
                                      - TM_VJ_COMP_SIZE
                                      + pppVectPtr->pppsHdrDecompressSpace),
/* Enough room to expand the PPP header and VJ header */
                                    maxCopySize,
                                    TM_16BIT_ZERO);
                    workingFcs = TM_PPP_FCS_INITIAL;
                    byteStuffFlag = 0;
                    if (packetPtr == TM_PACKET_NULL_PTR)
                    {
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
#ifdef TM_USE_IPV4
/* Discarded for lack of memory */
                        devPtr->dev4Ipv4Mib.ipIfStatsInDiscards++;
#ifdef TM_USE_NG_MIB2
                        tm_context(tvDevIpv4Data).ipIfStatsInDiscards++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV4 */
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
                        goto pppAsyncRecvFinish;
                    }
                    pppVectPtr->pppsPacketPtr = packetPtr;
                    pppHdrPtr = (ttPppHdrPtr)packetPtr->pktLinkDataPtr;
                    outDataPtr = packetPtr->pktLinkDataPtr;
                    tm_zero_dsp_offset(outDataOffset);
                    if (inDataChar != TM_PPP_FLAG_CHAR)
                    {
/* Did not get beginning FLAG character. Do not consume. */
                        continue;
                    }
                }
            }
            else
            {
/* We are working on a packet */
                if (packetPtr != TM_PACKET_NULL_PTR)
                {
                    if (inDataChar != TM_PPP_ESC_CHAR)
                    {
                        if (!(tm_ppp_need_stuff(inDataChar,inputMask)))
                        {
                            if (byteStuffFlag)
                            {
                                inDataChar = (tt8Bit)(inDataChar ^
                                                         TM_PPP_STUFF_CHAR);
                                byteStuffFlag = 0;
                            }
/* Do the FCS as we input */
                            workingFcs = (tt16Bit)((workingFcs>>8)^
                               tlFcsLookupCrc16[(workingFcs^inDataChar)&0xff]);
                            location = (tt32Bit)
                                (outDataPtr - (tt8BitPtr) pppHdrPtr);
#ifdef TM_DSP
                            location = tm_byte_count(location);
                            tm_inc_dsp_offset(location,outDataOffset);
#endif /* TM_DSP */

#ifdef TM_PPP_LQM
/* [RFC1989].R2.3:10 - count octets that are included in the FCS calculation */
                            inGoodFrameOctets++;
#endif /* TM_PPP_LQM */

/* Bug ID 1726: Check location to possibly avoid switching for performance*/
                            if (location <= TM_PPP_DATA_FIELD)
                            {
/* Switch on which field we are working on */
                                switch(location)
                                {
                                    case TM_PPP_ADDR_FIELD:
                                        if (inDataChar != TM_PPP_BROADCAST_CHAR)
                                        {
/* No Address Control field */
                                            tm_ppp_put_char_next(outDataPtr,
                                                         outDataOffset,
                                                         TM_PPP_BROADCAST_CHAR);
                                            tm_ppp_put_char_next(outDataPtr,
                                                         outDataOffset,
                                                         TM_PPP_UA_FRAME_CHAR);
/* Now we have to check for a compressed protocol */
                                            if (inDataChar & 0x0001)
                                            {
/* Protocol field was compressed */
                                                tm_ppp_put_char_next(outDataPtr,
                                                             outDataOffset,
                                                             TM_PPP_NULL_CHAR);
                                            }
                                        }
                                        break;
                                    case TM_PPP_PROTOCOL_FIELD:
                                        if (inDataChar & 0x0001)
                                        {
/* Protocol field was compressed */
                                            tm_ppp_put_char_next(outDataPtr,
                                                             outDataOffset,
                                                             TM_PPP_NULL_CHAR);
                                        }
                                        break;
                                    case TM_PPP_DATA_FIELD:
#ifdef TM_USE_IPV4
                                        if (pppHdrPtr->pppHdrProtocol ==
                                                    TM_PPP_COMP_TCP_PROTOCOL)
                                        {
/* An Compressed TCP so pad out the fields */
                                            vjCompFlag = 1;
                                            vjOptionFlags=(tt8Bit)(inDataChar|
                                                    ((tt8Bit) 0x80));
/* Handle the special S* and SA* cases (no fields are present) */
                                            if (    (vjOptionFlags &
                                                            TM_VJ_STAR_SA)
                                                 == TM_VJ_STAR_SA)
/* Special case: *SA (1011), *S (1111) */
                                            {
                                                tm_8bit_clr_bit(
                                                               vjOptionFlags,
                                                               TM_VJ_SAWU );
                                            }
                                            vjOffset = 0;
                                        }
#endif /* TM_USE_IPV4 */
                                        break;
                                    default:
                                        break;
                                }
                            }
#ifdef TM_USE_IPV4
/* Now pad out the VJ Compressed packet so the data aligns */
                            if (vjCompFlag)
                            {
/* Option is not present so add it in */
                                while(   ( !(   vjOptionFlags
                                          & tlVjFlags[vjOffset].flag) )
                                       && ( vjOffset <= TM_PPP_VJ_PADDING_FLAG))
                                {
                                    for ( i = 0;
                                         i < tlVjFlags[vjOffset].length;
                                         i++ )
                                    {
                                        *outDataPtr = TM_PPP_NULL_CHAR;
                                        outDataPtr++;
                                    }
                                    vjOffset = (tt8Bit)(vjOffset+i);
                                }
/* If the option is present we need to check the first byte of it */
                                switch(vjOffset)
                                {
                                    case TM_PPP_VJ_FLAG_FLAG:
/* Fall through because flag field is one byte */
                                    case TM_PPP_VJ_CONNECT_FLAG:
/* Connection option is present so do nothing because it is one byte */
                                    case TM_PPP_VJ_CHECKSUM_FLAG:
/* Fall through because checksum field is not padded */
                                        vjOffset++;
                                        break;
                                    case TM_PPP_VJ_URGENT_FLAG:
/* Fall through because all these field are padded to 4 bytes */
                                    case TM_PPP_VJ_WINDOW_FLAG:
/* Fall through because all these field are padded to 4 bytes */
                                    case TM_PPP_VJ_ACK_FLAG:
/* Fall through because all these field are padded to 4 bytes */
                                    case TM_PPP_VJ_SEQUENCE_FLAG:
/* Fall through because all these field are padded to 4 bytes */
                                    case TM_PPP_VJ_IDENT_FLAG:
                                        if (inDataChar == TM_PPP_NULL_CHAR)
                                        {
/* Option is 3 bytes so pad one byte */
                                            *outDataPtr = TM_PPP_NULL_CHAR;
                                            outDataPtr++;
                                            vjOffset += 2;
                                        }
                                        else
                                        {
/* Option is 1 byte so pad 3 bytes */
                                            for ( i = 0;
                                                  i <
                                          (tt8Bit) ( tlVjFlags[vjOffset].length
                                               - ((tt8Bit)1) );
                                                  i++ )
                                            {
                                                *outDataPtr = TM_PPP_NULL_CHAR;
                                                outDataPtr++;
                                            }
                                            vjOffset = (tt8Bit)(vjOffset +
                                                tlVjFlags[vjOffset].length);
                                        }
                                        break;
                                    case TM_PPP_VJ_PADDING_FLAG:
/* Falls through because we're just padding */
                                    default:
                                        vjOffset++;
                                }
/*
 * Check to see of we processed the last flag is so then only TCP
 * data follows
 */

                                if (vjOffset > TM_PPP_VJ_PADDING_FLAG)
                                {
                                    vjCompFlag = 0;
                                }
                            }
#endif /* TM_USE_IPV4 */
                            if (location < (tt32Bit)maxCopySize)
/* Only copy data if we have room for it. Discard bytes over the MRU. */
                            {
                                tm_ppp_put_char_next(outDataPtr,
                                                    outDataOffset,
                                                    inDataChar);
                            }
                        }
/* Otherwise the link inserted (not the remote host) so drop it */
                    }
                    else
                    {
/* A byte stuffed character */
                        byteStuffFlag = 1;
                    }
                    if (pppLcpStatePtr->lcpsStateInfo.cpsState
                                                         != TM_PPPS_OPENED )
/* If we are not in the LCP opened state, the only valid packets are LCP packets */
                    {
                        incomingPacketLen = (ttPktLen)
                         tm_byte_count(outDataPtr - packetPtr->pktLinkDataPtr);
                        tm_inc_dsp_offset(incomingPacketLen, outDataOffset);
                        if (     (incomingPacketLen >= (ttPktLen)(TM_PPP_HDR_BYTES))
                             &&  (pppHdrPtr->pppHdrProtocol !=
                                                        TM_PPP_LCP_PROTOCOL))
                        {
/* Silently discard non lcp packets when LCP is NOT in the open state. Check here
 * as well as at the end of the packet to make sure that noise does not make us
 * write beyond allocated memory for the packet. */
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
/* Bad packet */
                            devPtr->devInErrors++;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
/* OL: Might have to check if part of CLIENT is in the first 4 bytes */
                            tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
                            packetPtr = TM_PACKET_NULL_PTR;
                            pppVectPtr->pppsPacketPtr = TM_PACKET_NULL_PTR;
                        }
/* If we are not in the LCP opened state, also look for the CLIENT string */
                        tfPppClientWindows(devPtr, pppVectPtr, inDataChar);
                    }
                }
                else
                {
/*
 * Out of frame noise. Check if it contains a Windows client prompt.
 */
                    tfPppClientWindows(devPtr, pppVectPtr, inDataChar);
                }
            }
/* Consume the incoming data */
            tm_ppp_inc_ptr_n(inDataPtr,inDataOffset,1);
            linkDataLength--;
        }
#ifdef TM_USE_DRV_SCAT_RECV
        nextDevPacketPtr = (ttPacketPtr)nextDevPacketPtr->pktLinkNextPtr;
    }
    while (nextDevPacketPtr != (ttPacketPtr)0);
#endif /* TM_USE_DRV_SCAT_RECV */
/* Save our receive session back into the state table */
    tm_set_dsp_offset(pppVectPtr->pppsWorkingPtrOffset, outDataOffset);

    pppVectPtr->pppsWorkingPtr = outDataPtr;

#ifdef TM_USE_IPV4
    pppVectPtr->pppsVjCompFlag = vjCompFlag;
    pppVectPtr->pppsVjOffset = vjOffset;
    pppVectPtr->pppsVjOptionFlags = vjOptionFlags;
#endif /* TM_USE_IPV4 */

pppAsyncRecvFinish:
#ifdef TM_PPP_LQM
    pppVectPtr->pppsInGoodFrameOctets = inGoodFrameOctets;
#endif /* TM_PPP_LQM */
    pppVectPtr->pppsWorkingFcs = workingFcs;
    pppVectPtr->pppsLastByteStuffState = byteStuffFlag;

/* Free the incoming packet since we are done with it */
pppAsyncRecvReturn:
    tfFreePacket(devPacketPtr,TM_SOCKET_UNLOCKED);

    tm_unlock(&(devPtr->devLockEntry));
    return errorCode;
}
#ifdef TM_LINT
LINT_NULL_PTR_END(outDataPtr)
LINT_NULL_PTR_END(pppHdrPtr)
#endif /* TM_LINT */

/* Check for windows CLIENT string, and reply to it */
static void tfPppClientWindows(
                ttDeviceEntryPtr    devPtr,
                ttPppVectPtr        pppVectPtr,
                tt8Bit              inDataChar)
{
    tt8BitPtr       csStrPtr;
    tt16Bit         bytesRemaining;
    tt16Bit         bytesToSend;
    tt8Bit          state;
    tt8Bit          newState;

/* Get previous state */
    state = pppVectPtr->pppsClientState;
/* Transition to new state based on current input character */
    if (inDataChar == 'C')
    {
/* Restart. Unconditional change of state */
        newState = inDataChar;
    }
    else
    {
        newState = (tt8Bit)0; /* Reset */
/* Conditional change of state */
        switch (state)
        {
        case 'C':
            if (inDataChar == 'L')
            {
                newState = inDataChar;
            }
            break;
        case 'L':
            if (inDataChar == 'I')
            {
                newState = inDataChar;
            }
            break;
        case 'I':
            if (inDataChar == 'E')
            {
                newState = inDataChar;
            }
            break;
        case 'E':
            if (inDataChar == 'N')
            {
                newState = inDataChar;
            }
            break;
        case 'N':
            if (inDataChar == 'T')
            {
                newState = inDataChar;
            }
            break;
        default:
            break;
        }
    }
    if (newState == 'T')
    {
/*
 * This is the Windows client prompt "CLIENT" when starting a PPP connection.
 * Send the "CLIENTSERVER" reply.
 */
        pppVectPtr->pppsClientState = (tt8Bit)0; /* Reset before unlocking */
        csStrPtr = (tt8BitPtr)"CLIENTSERVER";
        bytesRemaining = 12;
/* Lock the device driver */
        tm_call_lock_wait(&(devPtr->devDriverLockEntry));
/* Loop through the string to make sure we respect the driver's max bufsize */
        while (bytesRemaining)
        {
            if (bytesRemaining <=
                pppVectPtr->pppsUserSendBufSize)
            {
                bytesToSend = bytesRemaining;
            }
            else
            {
                bytesToSend =
                        pppVectPtr->pppsUserSendBufSize;
            }
            (void)(*(devPtr->devSendFuncPtr))(
                                 (ttUserInterface)devPtr,
                                 (char TM_FAR *)csStrPtr,
                                 (int)bytesToSend,
                                 TM_USER_BUFFER_MORE);
            csStrPtr += bytesToSend;
            bytesRemaining = (tt16Bit)
                            (bytesRemaining - bytesToSend);
        }
/* Unlock the device driver */
        tm_call_unlock(&(devPtr->devDriverLockEntry));
    }
    else
    {
        pppVectPtr->pppsClientState = newState; /* Transition */
    }
    return;
}

/*
 * Set up an option in the outgoing option list
 *
 * packetOffsetPtr is a byte offset value and is only
 * included when using a DSP platform.
 */
static void tm_ppp_send_option ( tfPppSendOption,
                                 ttPacketPtr     packetPtr,
                                 tt32Bit         negotiateOption,
                                 tt8Bit          optionNumber,
                                 tt8BitPtr       optionValuePtr,
                                 tt8Bit          optionSize,
                                 tt8BitPtr       optionParmPtr,
                                 tt8Bit          optionParmSize,
                                 unsigned int *  packetOffsetPtr)
{
    tt8Bit i;
    tt8Bit       valueChar;
#ifdef TM_DSP
    unsigned int valueOffset;
#endif /* TM_DSP */


    tm_zero_dsp_offset(valueOffset);
    if ( ((tt32Bit)(TM_UL(1)<<optionNumber)) & negotiateOption )
    {
#ifdef TM_USE_IPV4
        if (optionNumber == TM_IPCP_DNS_PRI)
        {
            optionNumber = TM_IPCP_OPT_DNS_PRI;
        }
        else
        {
            if (optionNumber == TM_IPCP_DNS_SEC)
            {
                optionNumber = TM_IPCP_OPT_DNS_SEC;
            }
        }
#endif /* TM_USE_IPV4 */

/* Save the option number */
        tm_ppp_put_char_next( (packetPtr->pktLinkDataPtr),
                              *packetOffsetPtr,
                              optionNumber);

/* Save the size of the option */
        tm_ppp_put_char_next( (packetPtr->pktLinkDataPtr),
                              *packetOffsetPtr,
                              (optionSize + TM_8BIT_BYTE_COUNT + TM_8BIT_BYTE_COUNT +
                                    optionParmSize));
/* Load up the option */
        for (i = 0; i < optionSize; i++)
        {
            valueChar= tm_ppp_get_char_next(optionValuePtr, valueOffset);
            tm_ppp_put_char_next((packetPtr->pktLinkDataPtr),
                                 *packetOffsetPtr, valueChar);

        }

/* Load up the osptions parameter */
        tm_zero_dsp_offset(valueOffset);
        for (i = 0; i < optionParmSize; i++)
        {
/* for 16-bit DSP's, md5 Algorithm value is 00 05, we need get 05 */
            valueChar=tm_ppp_get_char_next(optionParmPtr, valueOffset);
#ifdef TM_DSP
            if(valueChar==0)
               valueChar=tm_ppp_get_char_next(optionParmPtr, valueOffset);
#endif /* TM_DSP */
            tm_ppp_put_char_next((packetPtr->pktLinkDataPtr),
                                 *packetOffsetPtr, valueChar);
        }
    }
}


int tfPppIoctl(int operation)
{
    TM_UNREF_IN_ARG(operation);
    return TM_LL_OKAY;
}

int tfPppError(int type)
{
    tfKernelError("PPP Error","tfPppError");
    TM_UNREF_IN_ARG(type);
    return TM_LL_OKAY;
}

/*
 * An invalid state transition occured.  This is most likely due to our
 * peer being non-compliant and sending packets that shouldn't be sent.
 * We just silently discard the packet.  Since this packet won't go through
 * its normal path, if we were passed a packet, free it here.
 */
int tfPppStateError(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{

    TM_UNREF_IN_ARG(pppVectPtr);

/*
 * Since this packet arrived in an invalid state, it won't be processed
 * normally, and therefore won't be free, so free it here.
 */
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        tfFreePacket(packetPtr,TM_SOCKET_UNLOCKED);
    }

    return TM_ENOERROR;
}


/*
 * Reject an incoming option
 *
 * inDataOffset and outDataOffsetPtr are byte offset values and are only
 * included when using a DSP platform.
 */
static ttPacketPtr tm_ppp_add_reject (tfPppAddReject,
                                      tt8BitPtr         inDataPtr,
                                      tt8BitPtr TM_FAR *outDataPtrPtr,
                                      tt16Bit           optionLength,
                                      ttPacketPtr       rejectPacketPtr,
                                      ttPacketPtr       nakPacketPtr,
                                      unsigned int      inDataOffset,
                                      unsigned int *    outDataOffsetPtr)
{
    ttPacketPtr retPacketPtr;
    tt8Bit      rejChar;

    if (rejectPacketPtr != TM_PACKET_NULL_PTR)
    {
        retPacketPtr = rejectPacketPtr;
    }
    else
    {
        if (nakPacketPtr == TM_PACKET_NULL_PTR)
        {
/* allocate a packet */
            retPacketPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES
                                              + TM_PPP_CP_BYTES,
                                              TM_PPP_LCP_MAX_BYTES
                                              - TM_PPP_ALLOC_HDR_BYTES
                                              - TM_PPP_CP_BYTES,
                                              TM_16BIT_ZERO );
        }
        else
        {
/* use the old nak packet */
            retPacketPtr = nakPacketPtr;
        }
/* Overwrite NAK data, if we are using old NAK packet */
        if (retPacketPtr != TM_PACKET_NULL_PTR)
        {
            *outDataPtrPtr = retPacketPtr->pktLinkDataPtr;
            tm_zero_dsp_offset(*outDataOffsetPtr);
        }
    }
    if (*outDataPtrPtr != (tt8BitPtr)0)
    {
/* copy the type */
        rejChar = tm_ppp_get_char_next(inDataPtr, inDataOffset);
        tm_ppp_put_char_next((*outDataPtrPtr), *outDataOffsetPtr, rejChar);

/* copy the length */
        rejChar = tm_ppp_get_char_next(inDataPtr, inDataOffset);
        tm_ppp_put_char_next((*outDataPtrPtr), *outDataOffsetPtr, rejChar);
/* copy the new option */
        while( optionLength )
        {
            rejChar = tm_ppp_get_char_next(inDataPtr, inDataOffset);
            tm_ppp_put_char_next((*outDataPtrPtr), *outDataOffsetPtr, rejChar);
            optionLength--;
        }
    }
    return (retPacketPtr);
}

/*
 * NAK an Option and add it to the NAK packet if
 * we are not rejecting other options
 *
 * inDataOffset and outDataOffsetPtr are byte offset values and are only
 * included when using a DSP platform.
 */
static ttPacketPtr tm_ppp_add_nak (tfPppAddNak,
                                   tt8BitPtr         inDataPtr,
                                   tt8BitPtr         newOptionValuePtr,
                                   tt8BitPtr TM_FAR *outDataPtrPtr,
                                   tt16Bit           optionLength,
                                   ttPacketPtr       rejectPacketPtr,
                                   ttPacketPtr       nakPacketPtr,
                                   unsigned int      inDataOffset,
                                   unsigned int *    outDataOffsetPtr)

{
    ttPacketPtr retPacketPtr;
    tt8Bit       rejChar;
#ifdef TM_DSP
    unsigned int valueDataOffset;
#endif /* TM_DSP */


    tm_zero_dsp_offset(valueDataOffset);
    if (rejectPacketPtr != TM_PACKET_NULL_PTR)
    {
/* We already are rejecting so ignore NAK's for now */
        retPacketPtr = TM_PACKET_NULL_PTR;
    }
    else
    {
/* Check to see if we have a nak packet pointer */
        if (nakPacketPtr == TM_PACKET_NULL_PTR)
        {
/* We don't so lets get one */
            retPacketPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES
                                               + TM_PPP_CP_BYTES,
                                              TM_PPP_LCP_MAX_BYTES
                                               - TM_PPP_ALLOC_HDR_BYTES
                                               - TM_PPP_CP_BYTES,
                                              TM_16BIT_ZERO );
            if (retPacketPtr != TM_PACKET_NULL_PTR)
            {
                *outDataPtrPtr = retPacketPtr->pktLinkDataPtr;
            }
        }
        else
        {
/* We do, so lets continue to use it */
            retPacketPtr = nakPacketPtr;
        }
        if (*outDataPtrPtr != (tt8BitPtr)0)
        {
/* copy the type */
            rejChar = tm_ppp_get_char_next(inDataPtr, inDataOffset);
            tm_ppp_put_char_next((*outDataPtrPtr), *outDataOffsetPtr, rejChar);
/* copy the length */
            rejChar = tm_ppp_get_char_next(inDataPtr, inDataOffset);
            tm_ppp_put_char_next((*outDataPtrPtr), *outDataOffsetPtr, rejChar);
/* copy the new option */
            while( optionLength )
            {
                rejChar = tm_ppp_get_char_next(newOptionValuePtr,
                                               valueDataOffset);
                tm_ppp_put_char_next((*outDataPtrPtr), *outDataOffsetPtr,
                                     rejChar);
                optionLength--;
            }
        }
    }
    return retPacketPtr;
}

/*
 * Initialize the LCP Layer
 */
ttVoid tfLcpInit(ttPppVectPtr pppVectPtr)
{
    ttLcpStatePtr             pppLcpStatePtr;
    tt8Bit                    chapAlgorithm;

    pppLcpStatePtr = &pppVectPtr->pppsLcpState;

/* check if we do have sufficient parameter to support the AUTH
 * method we claimed.
 */
    tfPppCheckAuthProtocolBits(pppVectPtr);

    pppVectPtr->pppsStateInfoPtrArray[TM_PPP_LCP_INDEX] =
            (ttCpStatePtr)pppLcpStatePtr;
    pppLcpStatePtr->lcpsStateInfo.cpsStateFuncList =
            (ttPppStateFunctPtr TM_FAR *)tlLcpStateFunctTable;
    tm_bcopy(&pppLcpStatePtr->lcpsLocalWantOptions,
             &pppLcpStatePtr->lcpsLocalNegOptions,
             sizeof(ttLcpOptions));
/* for Authentication method, just set our most desired method only
 * in lcpsLocalNegOptions
 */
    pppLcpStatePtr->lcpsLocalNegOptions.lcpAuthMethod =
             tfLcpGetNextAuthMethod(pppLcpStatePtr->
                               lcpsLocalWantOptions.lcpAuthProtocolBits,
                               TM_PPP_AUTHMETHOD_NULL,
/* if it is CHAP,we need to get the algorithm*/
                               &chapAlgorithm );


/*
 * Initialize this layer's negotiation counter.  This value is set to zero when
 * the device is first opened (ie, from a tfOpenInterface call).  Each time
 * this layer is renegotiated *within the same session* this value is
 * incremented.
 */
    pppLcpStatePtr->lcpsStateInfo.cpsNegotiateCounter = 0;

    pppLcpStatePtr->lcpsStateInfo.cpsMagicNumberFailures =
                                                    TM_LCP_MAGIC_NUM_FAILURES;

/* ecr added 01/24/01 */
    (void)tfLcpInitRestartCounter(pppVectPtr, (ttPacketPtr) 0);

/*
 * Initialize the current MTU value to 1500.  This is the default MTU to use
 * while performing LCP negotiation (i.e., before an MTU value has been
 * negotiated). (1492 for PPPoE.)
 */
    pppLcpStatePtr->lcpsLocalGotOptions.lcpMaxReceiveUnit =
#ifdef TM_USE_PPPOE
/* Get the MRU from the link layer */
            pppVectPtr->pppsDeviceEntryPtr->devLinkLayerProtocolPtr->lnkMtu;
#else /* !TM_USE_PPPOE */
            TM_PPP_DEF_MRU;
#endif /* TM_USE_PPPOE */

}

/*
 * Initialize the LCP Layer
 */
ttVoid tfLcpDefaultInit(ttPppVectPtr pppVectPtr)
{
    ttLcpStatePtr             pppLcpStatePtr;
    ttCpStatePtr              pppCpStatePtr;
#ifdef TM_USE_PPPOE
    tt16Bit                   mru;
#endif /* TM_USE_PPPOE */

    pppLcpStatePtr = &pppVectPtr->pppsLcpState;
    tm_bcopy( &tlLcpDefaultOptions,
              &pppLcpStatePtr->lcpsLocalWantOptions,
              sizeof(ttLcpOptions) );
#ifdef TM_USE_PPPOE
/* Get the MRU from the link layer */
    mru = pppVectPtr->pppsDeviceEntryPtr->devLinkLayerProtocolPtr->lnkMtu;
    pppLcpStatePtr->lcpsLocalWantOptions.lcpMaxReceiveUnit = mru;
#endif /* TM_USE_PPPOE */

/* By default, we do not negotiate any options, clear any ALLOW/WANT authentication
 * method bits.
 */
    tm_context(tvPppAuthPriority) = 0;
    pppLcpStatePtr->lcpsLocalWantOptions.lcpAuthProtocolBits = (tt8Bit)0;
    pppLcpStatePtr->lcpsRemoteAllowOptions.lcpAuthProtocolBits = (tt8Bit)0;
    pppLcpStatePtr->lcpsLocalNegotiateOption=(tt32Bit)0;
    pppLcpStatePtr->lcpsRemoteAllowOptions.lcpMaxReceiveUnit =
#ifdef TM_USE_PPPOE
        mru;
#else /* !TM_USE_PPPOE */
        (tt16Bit) TM_PPP_DEF_MRU;
#endif /* !TM_USE_PPPOE */
    pppLcpStatePtr->lcpsRemoteAllowOptions.lcpMagicNumber = (tt8Bit)0;

#ifdef TM_PPP_LQM
    pppLcpStatePtr->lcpsRemoteAllowOptions.lcpQualityProtocol =
        TM_PPP_LQR_HOST_PROTOCOL;
    pppLcpStatePtr->lcpsRemoteAllowOptions.lqmReportingPeriod = (tt32Bit) 0;
#endif /* TM_PPP_LQM */

    pppCpStatePtr = &pppLcpStatePtr->lcpsStateInfo;
    pppCpStatePtr->cpsInitTrmRetryCnt = TM_PPP_TERM_MAX_RETRY;
    pppCpStatePtr->cpsInitCfgRetryCnt = TM_PPP_CFG_MAX_RETRY;
    pppCpStatePtr->cpsRetryTimeoutSeconds = TM_PPP_RETRY_TIMEOUT;
    pppCpStatePtr->cpsInitMaxFailures = TM_PPP_MAX_FAILURES;

#ifdef TM_USE_PPPOE
    if (tm_ll_is_pppoe(pppVectPtr->pppsDeviceEntryPtr))
    {
/* By default we allow the remote to negotiate these options */
        pppLcpStatePtr->lcpsRemoteNegotiateOption =
            (  (TM_UL(1)<< TM_LCP_MAX_RECV_UNIT)
             | (TM_UL(1)<< TM_LCP_MAGIC_NUMBER)
#ifdef TM_PPP_LQM
             | (TM_UL(1)<< TM_LCP_QUALITY_PROTOCOL)
#endif /* TM_PPP_LQM */
            );
    }
    else
#endif /* TM_USE_PPPOE */
    {
/* By default we allow the remote to negotiate these options */
        pppLcpStatePtr->lcpsRemoteNegotiateOption =
            ((TM_UL(1)<< TM_LCP_MAX_RECV_UNIT) |
             (TM_UL(1)<< TM_LCP_ACCM) |
             (TM_UL(1)<< TM_LCP_MAGIC_NUMBER) |
#ifdef TM_PPP_LQM
             (TM_UL(1)<< TM_LCP_QUALITY_PROTOCOL) |
#endif /* TM_PPP_LQM */
             (TM_UL(1)<< TM_LCP_PROTOCOL_COMP) |
             (TM_UL(1)<< TM_LCP_ADDRCONTROL_COMP));
    }
    pppCpStatePtr->cpsState = TM_PPPS_INITIAL;
    pppVectPtr->pppsClientState = (tt8Bit)0; /* Reset */
}


#ifdef TM_PPP_LQM
/****************************************************************************
* FUNCTION: tfPppParseEchoReply
*
* PURPOSE:
*   Process a received Echo-Reply message, calling the user-defined handler
*   if one was registered (refer to tfPppSendEchoRequest).
*
* PARAMETERS:
*   pppVectPtr:
*       Pointer to the PPP state vector.
*   packetPtr:
*       Pointer to the Echo-Reply packet.
*   length:
*       Length of the Echo-Reply message, including the LCP header.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tfPppParseEchoReply(ttPppVectPtr pppVectPtr,
                         ttPacketPtr  packetPtr,
                         tt16Bit      length)
{
#define devPtr (pppVectPtr->pppsDeviceEntryPtr)
    ttLcpStatePtr               pppLcpStatePtr;
    tt8BitPtr                   dataPtr;
    ttEchoReplyFuncPtr          echoReplyFuncPtr;
    ttPppCpPacketTypePtr        cpHdrPtr;
    tt32Bit                     temp32;
/*    int                         errorCode; */
    tt8Bit                      echoRequestId;

    tm_assert_is_locked(tfPppParseEchoReply, &(devPtr->devLockEntry));

    cpHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
    dataPtr = packetPtr->pktLinkDataPtr;
    pppLcpStatePtr =  &pppVectPtr->pppsLcpState;
/*
 * Verify that this has at least the LCP header and the Magic-Number field
 * (Already verified in tfLcpIncomingPacket)
 */
    length -= (TM_PPP_CP_BYTES + 4);

/* Parse the echoRequestId from the LCP header. */
    echoRequestId = cpHdrPtr->cpIdentifier;

/* Parse the magic number */
    tm_bcopy(dataPtr, &temp32, sizeof(temp32));
    dataPtr += sizeof(
        pppLcpStatePtr->lcpsLocalGotOptions.lcpMagicNumber);
    if ((pppLcpStatePtr->lcpsLocalGotOptions.lcpMagicNumber != 0)
        &&
        (ntohl(temp32)
         == pppLcpStatePtr->lcpsLocalGotOptions.lcpMagicNumber))
    {
/* If the received magic number indicates that the link is looped back,
   then call tfDeviceClose to drop the link and notify the user that the
   link is dead */
        tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_CLOSE_COMPLETE);
    }
    else
    {
        if (pppVectPtr->pppsEchoReplyFuncPtr != 0)
        {
            echoReplyFuncPtr = pppVectPtr->pppsEchoReplyFuncPtr;
            tm_unlock(&(devPtr->devLockEntry));

/* Call the user's handler passing it the Echo-Reply. */
            /*errorCode = */(void)(*echoReplyFuncPtr)(
                (ttUserInterface)devPtr,
                (ttUser8Bit) echoRequestId,
                (ttConstCharPtr) dataPtr,
                (int) length);
/* errorCode never used */

            tm_lock_wait(&(devPtr->devLockEntry));
        }
    }


    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    return;
#undef devPtr
}
#endif /* TM_PPP_LQM */


/*
 * Parse a Reject
 * The remote didn't like some things that we were trying to config
 * so he said to turn them off
 */
int tfLcpParseReject(ttPppVectPtr pppVectPtr,
                     ttPacketPtr  packetPtr,
                     tt16Bit      optionsLength)
{
    ttLcpStatePtr pppLcpStatePtr;
    tt8BitPtr    dataPtr;
    tt8BitPtr    optionValuePtr;
    tt8Bit       optionType;
    tt8Bit       optionTotalLength;
    tt8Bit       optionLength;
    tt8Bit       needFreePacket;
    tt32Bit      localNegOption;
    int          errorCode;
#ifdef TM_DSP
    unsigned int dataOffset;
    unsigned int valueDataOffset;
#endif /* TM_DSP */

    needFreePacket = TM_8BIT_YES;
    tm_zero_dsp_offset(dataOffset);
    pppLcpStatePtr = &pppVectPtr->pppsLcpState;
    packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
    dataPtr = packetPtr->pktLinkDataPtr;
    optionsLength -= TM_PPP_CP_BYTES;
    localNegOption = pppLcpStatePtr->lcpsLocalNegotiateOption;
    errorCode = TM_ENOERROR;
    while( ((ttS16Bit)optionsLength > 0) && (errorCode == TM_ENOERROR) )
    {
/*
 * inside the while loop, use optionValuePtr rather than dataPtr
 * and use valueDataOffset in loop, leave dataOffset untouched each loop
 */
        tm_set_dsp_offset(valueDataOffset, dataOffset);
        optionValuePtr=dataPtr;
        optionType = tm_ppp_get_char_next(optionValuePtr,valueDataOffset);

        if (optionType == TM_LCP_AUTH_PROTOCOL)
        {
/* Our peer rejected authentication, so terminate the connection */
            (void)tfPppStateMachine(pppVectPtr,
                              packetPtr,
                              TM_PPPE_CLOSE,
                              TM_PPP_LCP_INDEX);
            optionsLength = 0;
/*BUG 581 we free this packet when we close interface */
            needFreePacket = TM_8BIT_ZERO;
        }
        else
        {
            optionTotalLength = tm_ppp_get_char_next(optionValuePtr,
                                                     valueDataOffset);

            optionLength = (tt8Bit)(optionTotalLength - TM_8BIT_BYTE_COUNT
                                    - TM_8BIT_BYTE_COUNT);

            switch (optionType)
            {
/* Both ACCM and Magic-Number options are 32-bits long */
                case TM_LCP_ACCM:
                case TM_LCP_MAGIC_NUMBER:
                    if (optionLength != TM_32BIT_BYTE_COUNT)
                    {
                        errorCode = TM_EINVAL;
                    }
                    break;

                case TM_LCP_MAX_RECV_UNIT:
                    if (optionLength != TM_16BIT_BYTE_COUNT)
                    {
                        errorCode = TM_EINVAL;
                    }
                    break;

/* Both protocol and addr/control compression options are zero bytes long. */
                case TM_LCP_PROTOCOL_COMP:
                case TM_LCP_ADDRCONTROL_COMP:
                    if (optionLength != 0)
                    {
                        errorCode = TM_EINVAL;
                    }
                    break;
#ifdef TM_USE_PPP_CALLBACK
                case TM_LCP_CALLBACK:
                    if (optionLength < 1)
                    {
                        errorCode = TM_EINVAL;
                    }
                    pppLcpStatePtr->lcpsLocalNegOptions.lcpCallbackOpFlags
                                                        = TM_32BIT_ZERO;
                    break;
#endif /* TM_USE_PPP_CALLBACK */

/* Unrecognized option - since this is a Config-Rej, all options should always
   be recognized (since this is in response to our own Config-Req) so ignore
   this packet. */
                default:
                    errorCode = TM_EINVAL;
                    break;

            }

            if (optionType < (tt8Bit)32)
            {
                localNegOption &= ~(TM_UL(1)<< optionType);
            }

            tm_ppp_move_next_option(dataPtr, optionTotalLength, dataOffset);

            optionsLength = (tt16Bit)(optionsLength - optionTotalLength);
        }
    }

/* No problems found in Conf-Rej packet, so record the results. */
    if (errorCode == TM_ENOERROR)
    {
        pppLcpStatePtr->lcpsLocalNegotiateOption = localNegOption;
    }

    if(needFreePacket == TM_8BIT_YES)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }

    return errorCode;
}


/*
 * Parse a nak
 * The remote didn't like some things that we were trying to config
 * so he suggested new values
 */
int tfLcpParseNak(ttPppVectPtr pppVectPtr,
                  ttPacketPtr  packetPtr,
                  tt16Bit      optionsLength)
{

    ttLcpStatePtr pppLcpStatePtr;
    tt8BitPtr    optionValuePtr;
    tt8BitPtr    dataPtr;
    tt32Bit      temp32;
    tt16Bit      temp16;
    tt8Bit       optionType;
    tt8Bit       optionTotalLength;
    tt8Bit       optionLength;
    tt8Bit       needFreePacket;
    tt8Bit        chapAlgorithm;
    int          errorCode;
#ifdef TM_DSP
    unsigned int dataOffset;
    unsigned int valueDataOffset;
#endif /* TM_DSP */

    errorCode = TM_ENOERROR;
    needFreePacket = TM_8BIT_YES;

    pppLcpStatePtr = &pppVectPtr->pppsLcpState;
    tm_zero_dsp_offset(dataOffset);
    packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
    dataPtr = packetPtr->pktLinkDataPtr;
    optionsLength -= TM_PPP_CP_BYTES;
    while( (ttS16Bit)optionsLength > 0 )
    {
        tm_set_dsp_offset(valueDataOffset,dataOffset);
        optionValuePtr=dataPtr;

        optionType = tm_ppp_get_char_next(optionValuePtr,
                                          valueDataOffset);
        optionTotalLength = tm_ppp_get_char_next(optionValuePtr,
                                                 valueDataOffset);

/* Get the length of the option data (without length and type fields */
        optionLength =
            (tt8Bit) (optionTotalLength - TM_8BIT_BYTE_COUNT
                      - TM_8BIT_BYTE_COUNT);

/* JNS/8-2-99 BUGFIX 413 */
/*
 * In addition to our normal option processing, now we also need to deal with
 * possibly broken PPP implementations on the other side.  There are some
 * PPP implementations that will NAK with the same value we request (violates
 * RFC1661).  Now, we will treat those as option-rejects.  So for each option
 * we process, also ensure that this isn't a bad NAK.
 */
        switch(optionType)
        {
            case TM_LCP_MAX_RECV_UNIT:
                if (optionLength != TM_16BIT_BYTE_COUNT)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto lcpParseNakFinish;
                }
                tm_ppp_byte_copy(optionValuePtr, valueDataOffset,
                                 &temp16, 0,TM_16BIT_BYTE_COUNT);
                if (ntohs(temp16) == pppLcpStatePtr->
                    lcpsLocalNegOptions.lcpMaxReceiveUnit)
                {
/* Bad NAK, treat as reject */
                    pppLcpStatePtr->lcpsLocalNegotiateOption &=
                            ~(TM_UL(1)<< optionType);

                }
                else
                {
                    pppLcpStatePtr->lcpsLocalNegOptions.
                        lcpMaxReceiveUnit = ntohs(temp16);
                    if (pppLcpStatePtr->lcpsLocalNegOptions.lcpMaxReceiveUnit
                        == (tt16Bit) TM_PPP_DEF_MRU)
                    {
/* should not negotiate the default value for an option */
                        pppLcpStatePtr->lcpsLocalNegotiateOption &=
                            ~(TM_UL(1)<< optionType);
                    }
                }
                break;
            case TM_LCP_ACCM:
                if (optionLength != TM_32BIT_BYTE_COUNT)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto lcpParseNakFinish;
                }
                tm_ppp_byte_copy(optionValuePtr, valueDataOffset,
                                 &temp32, 0,TM_32BIT_BYTE_COUNT);
                if (ntohl(temp32) == pppLcpStatePtr->
                    lcpsLocalNegOptions.lcpAccm)
                {
/* Bad NAK, treat as reject */
                    pppLcpStatePtr->lcpsLocalNegotiateOption &=
                        ~(TM_UL(1) << optionType);
                }
                else
                {
                    pppLcpStatePtr->lcpsLocalNegOptions.lcpAccm
                       = ntohl(temp32);
                }
                break;

            case TM_LCP_AUTH_PROTOCOL:
/* If that protocol is Configure-Nak'd, then the implementation SHOULD attempt
 * the next most desirable protocol in  the next Configure-Request.
 */
                if (optionLength < TM_16BIT_BYTE_COUNT)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto lcpParseNakFinish;
                }
/* we don't care what the peer's value. Instead, we go ahead to find our next
 * most preferred auth method.
 */
                pppLcpStatePtr->lcpsLocalNegOptions.lcpAuthMethod =
                            tfLcpGetNextAuthMethod(
/* all our supported auth protocols is in localwant, saved as bitmap  */
                            pppLcpStatePtr->lcpsLocalWantOptions.
                            lcpAuthProtocolBits,
/* our previous auth method (which was naked), is saved in localneg, as 8bit value */
                            pppLcpStatePtr->lcpsLocalNegOptions.
                            lcpAuthMethod,
                            &chapAlgorithm);
                if(pppLcpStatePtr->lcpsLocalNegOptions.lcpAuthMethod == 0)
                {
/* You NAKed all of our authentication method, we don't have any more alternatives.
 * we have to close the connection
 */
                    (void)tfPppStateMachine(pppVectPtr,
                              packetPtr,
                              TM_PPPE_CLOSE,
                              TM_PPP_LCP_INDEX);
/*BUG 581, we free this packet when we close interface */
                    needFreePacket = TM_8BIT_ZERO;

                }
                break;

#ifdef TM_PPP_LQM
        case TM_LCP_QUALITY_PROTOCOL:
            tm_bcopy(optionValuePtr, &temp16, sizeof(temp16));
            optionValuePtr += sizeof(temp16);
            if ((temp16 != TM_PPP_LQR_PROTOCOL)
                || (pppVectPtr->pppsLqmVectPtr == TM_LQM_VECT_NULL_PTR))
            {
/* the peer wants us to use a quality protocol other than
   Link-Quality-Report, or else the user didn't call tfUsePppLqm:
   treat as reject */
                pppLcpStatePtr->lcpsLocalNegotiateOption &=
                    ~(TM_UL(1) << optionType);
            }
            else
            {
                tm_bcopy(optionValuePtr, &temp32, sizeof(temp32));
                if (ntohl(temp32) == pppLcpStatePtr->
                    lcpsLocalNegOptions.lqmReportingPeriod)
                {
/* Bad NAK, treat as reject */
                    pppLcpStatePtr->lcpsLocalNegotiateOption &=
                        ~(TM_UL(1) << optionType);
                }
                else
                {
                    pppVectPtr->
                        pppsLcpState.lcpsLocalNegOptions.lqmReportingPeriod
                        = ntohl(temp32);
                }
            }
            break;
#endif /* TM_PPP_LQM */

            case TM_LCP_MAGIC_NUMBER:
                if (optionLength != TM_32BIT_BYTE_COUNT)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto lcpParseNakFinish;
                }
                tm_ppp_byte_copy(optionValuePtr, valueDataOffset,
                                 &temp32, 0,TM_32BIT_BYTE_COUNT);
                if (ntohl(temp32) == pppLcpStatePtr->
                    lcpsRemoteAllowOptions.lcpMagicNumber)
                {
                    temp32 = tfGetRandom();
                    pppLcpStatePtr->lcpsLocalNegOptions.
                        lcpMagicNumber = temp32;
                }
                else
                {
                    pppLcpStatePtr->lcpsLocalNegOptions.lcpMagicNumber
                                                = ntohl(temp32);
                }
                break;
/*
 * Just in case we have a BAD implmentation of PPP
 * we treat a NAK for protocol and address/control field compression as
 * a Reject
 */
            case TM_LCP_PROTOCOL_COMP:
                if (optionLength != 0)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto lcpParseNakFinish;
                }
                pppLcpStatePtr->lcpsLocalNegotiateOption &=
                        ~(TM_UL(1)<< TM_LCP_PROTOCOL_COMP);
                break;
            case TM_LCP_ADDRCONTROL_COMP:
                if (optionLength != 0)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto lcpParseNakFinish;
                }
                pppLcpStatePtr->lcpsLocalNegotiateOption &=
                        ~(TM_UL(1)<< TM_LCP_ADDRCONTROL_COMP);
                break;
#ifdef TM_USE_PPP_CALLBACK
            case TM_LCP_CALLBACK:
                if (optionLength < 1)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto lcpParseNakFinish;
                }
                pppLcpStatePtr->lcpsLocalNegotiateOption
                        &= ~(TM_UL(1) << TM_LCP_CALLBACK);
                pppLcpStatePtr->lcpsLocalNegOptions.lcpCallbackOpFlags
                        = TM_32BIT_ZERO;
                break;
#endif /* TM_USE_PPP_CALLBACK */

/* Unrecognized option - since this is a Config-Nak, all options should always
   be recognized (since this is in response to our own Config-Req) so ignore
   this packet. */
            default:
                errorCode = TM_EINVAL;
                goto lcpParseNakFinish;
        }

        tm_ppp_move_next_option(dataPtr, optionTotalLength, dataOffset);

        optionsLength = (tt16Bit)(optionsLength - optionTotalLength);
    }

lcpParseNakFinish:

    if(needFreePacket == TM_8BIT_YES)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
    return errorCode;
}

/*
 * Parse the incoming LCP config request
 */
ttPacketPtr tfLcpParseConfig(ttPppVectPtr pppVectPtr,
                             ttPacketPtr  packetPtr,
                             tt16Bit      length,
                             tt8BitPtr    pppEventPtr)
{
    ttPppCpPacketTypePtr cpHdrPtr;
    ttLcpStatePtr        pppLcpStatePtr;
    ttPacketPtr          rejectPacketPtr;
    ttPacketPtr          nakPacketPtr;
    ttPacketPtr          retPacketPtr;
    tt8BitPtr            tempDataPtr;
    tt8BitPtr            dataPtr;
    tt8BitPtr            optionValuePtr;
    tt8BitPtr            outDataPtr;
    ttLcpOptions         remoteOptions;
    tt32Bit              tempData;
    tt32Bit              temp32;
    tt32Bit              remoteOptionsType;
#ifdef TM_PPP_LQM
    tt32Bit              tempDataArray[2];
#endif
    unsigned int         magicNumFailure;
    tt16Bit              temp16;
#ifdef TM_USE_PPPOE
    tt16Bit              mru;
#endif /* TM_USE_PPPOE */
    tt16Bit              nakAuthType;
    tt8Bit               optionType;
    tt8Bit               optionLength;
    tt8Bit               optionTotalLength;
    tt8Bit               ident;
    tt8Bit               chapData;
    tt8Bit               ackFlag;
    tt8Bit               userAuthBits;
    tt8Bit               chapAlgorithm;
#ifdef TM_USE_PPP_CALLBACK
    tt8Bit               optionOperation;
#endif /* TM_USE_PPP_CALLBACK */

#ifdef TM_DSP
    unsigned int         inDataOffset;
    unsigned int         outDataOffset;
    unsigned int         valueDataOffset;
#endif /* TM_DSP */

    chapAlgorithm = (tt8Bit)0;
    pppLcpStatePtr = &pppVectPtr->pppsLcpState;
    tm_zero_dsp_offset(inDataOffset);
    tm_zero_dsp_offset(outDataOffset);
    rejectPacketPtr = TM_PACKET_NULL_PTR;
    nakPacketPtr = TM_PACKET_NULL_PTR;
    outDataPtr = (tt8BitPtr)0;
    cpHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    dataPtr = packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE;
    ident = cpHdrPtr->cpIdentifier;
    tm_bcopy(&tlLcpDefaultOptions,&remoteOptions, sizeof(ttLcpOptions));
#ifdef TM_USE_PPPOE
/* Get the MRU from the link layer */
    mru = pppVectPtr->pppsDeviceEntryPtr->devLinkLayerProtocolPtr->lnkMtu;
    remoteOptions.lcpMaxReceiveUnit = mru;
#endif /* TM_USE_PPPOE */
    remoteOptionsType = TM_UL(0);
    length -= TM_PPP_CP_BYTES;
    retPacketPtr = TM_PACKET_NULL_PTR;
    while ((ttS16Bit)length > 0)
    {
        tm_set_dsp_offset(valueDataOffset, inDataOffset);
        optionValuePtr=dataPtr;
        optionType = tm_ppp_get_char_next(optionValuePtr,
                                          valueDataOffset);
        optionTotalLength = tm_ppp_get_char_next(optionValuePtr,
                                                 valueDataOffset);

/* Check for INVALID length (length cannot be less than 2) */
        if ((tt16Bit)optionTotalLength > length)
        {
            optionTotalLength = (tt8Bit)(length&0xff);
        }
        if (optionTotalLength < (tt8Bit)2)
        {
            optionTotalLength = (tt8Bit)2;
        }
        optionLength = (tt8Bit)(   optionTotalLength - TM_8BIT_BYTE_COUNT
                                 - TM_8BIT_BYTE_COUNT );
/* Make sure the value is in our bit range (only 32 bit options) */
        if (optionType < 32)
        {
/* Check to make sure we allow the remote to negotiate this option */
            if (   pppLcpStatePtr->lcpsRemoteNegotiateOption
                 & (TM_UL(1)<< optionType) )
            {
                switch(optionType)
                {
                    case TM_LCP_MAX_RECV_UNIT:
                        if (optionLength != TM_16BIT_BYTE_COUNT)
                        {
/* Reject because of bad length */
                            rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto lcpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {
/* Get the requested option value */
                            tm_ppp_byte_copy(optionValuePtr,valueDataOffset,
                                     &temp16,0, TM_16BIT_BYTE_COUNT);
                            temp16 = htons(temp16);
/*
 * Check to see if we allow everything or it is the desired value
 * (Regardless of user's settings, MRU may not be less than 4).
 */
                            if ( (pppLcpStatePtr->
                                            lcpsRemoteAllowOptions.
                                            lcpMaxReceiveUnit >= temp16) &&
                                 (temp16 >= 4) )
                            {
                                remoteOptionsType |= (TM_UL(1)<< optionType);
                                remoteOptions.lcpMaxReceiveUnit = temp16;

/* Set the MTU for the device */
#ifdef TM_USE_IPV4
                                pppVectPtr->pppsDeviceEntryPtr->devMtu =
                                                                      temp16;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
                                if (temp16 < TM_6_PMTU_MINIMUM)
                                {
/* For IPv6, we cannot reduce the MTU to be less than 1280 bytes */
                                    pppVectPtr->pppsDeviceEntryPtr->dev6Mtu =
                                        TM_6_PMTU_MINIMUM;
                                }
                                else
                                {
                                    pppVectPtr->pppsDeviceEntryPtr->dev6Mtu =
                                        temp16;
                                }
#endif /* TM_USE_IPV6 */
                            }
                            else
                            {
/*
 * The requested value is not allowed.  First, check that we have not exceeded
 * our maximum failure value.  If we haven't, then send our peer a NAK with
 * the allowed parameter value.  If we have exceeded max-failures, reject the
 * option completely.
 */
                                if (pppLcpStatePtr->lcpsStateInfo.
                                      cpsFailures > 0)
                                {
                                    temp16 = pppLcpStatePtr->
                                             lcpsRemoteAllowOptions.
                                             lcpMaxReceiveUnit;
                                    temp16 = htons(temp16);
                                    nakPacketPtr = tm_ppp_add_nak(
                                                           tfPppAddNak,
                                                           dataPtr,
                                                           (tt8BitPtr)&temp16,
                                                           &outDataPtr,
                                                           optionLength,
                                                           rejectPacketPtr,
                                                           nakPacketPtr,
                                                           inDataOffset,
                                                           &outDataOffset);
                                    if ( (nakPacketPtr == TM_PACKET_NULL_PTR)
                                     && (rejectPacketPtr
                                         == TM_PACKET_NULL_PTR))
                                    {
                                       goto lcpParseConfigFinish;
                                    }
                                }
                                else
                                {
/* We've NAKed more than 'max-failure' times - start rejecting this option */
                                    rejectPacketPtr = tm_ppp_add_reject(
                                                               tfPppAddReject,
                                                               dataPtr,
                                                               &outDataPtr,
                                                               optionLength,
                                                               rejectPacketPtr,
                                                               nakPacketPtr,
                                                               inDataOffset,
                                                               &outDataOffset);
                                    if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                                    {
                                        goto lcpParseConfigFinish;
                                    }
                                    nakPacketPtr = TM_PACKET_NULL_PTR;

                                }
                            }
                        }
                        break;
                    case TM_LCP_ACCM:
                        if (optionLength != TM_32BIT_BYTE_COUNT)
                        {
/* Reject because of bad length */
                            rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto lcpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {
/* Get the requested option value */
                            tm_ppp_byte_copy(optionValuePtr,valueDataOffset,
                                     &temp32,0, TM_32BIT_BYTE_COUNT);
                            remoteOptionsType |= (TM_UL(1)<< optionType);
                            remoteOptions.lcpAccm = ntohl(temp32);
                        }
                        break;

/*
 * Determine type of authentication (PAP or CHAP).  The response to the
 * peer's request for authentication is based on the user's configuration
 * (the authentication type set and which username/password strings are set).
 */
                    case TM_LCP_AUTH_PROTOCOL:
                        chapData = 0;
                        tm_bcopy(optionValuePtr, &temp16, sizeof(temp16));
                        temp16 = ntohs(temp16);
                        userAuthBits = pppLcpStatePtr->
                            lcpsRemoteAllowOptions.lcpAuthProtocolBits;

                        ackFlag       = TM_8BIT_ZERO;
                        nakAuthType = 0;
/* here our logic is simple. If peer proposes an auth method we support,
 * we will ACK it. Otherwise, if we don't support peer's proposal,
 * we NAK it with our most desired auth method. (anyway, the authenticator
 * may ignore our suggest and send back its second most preferred
 * authentication method.
 */
                        chapData =
                               *( (tt8BitPtr)optionValuePtr + sizeof(temp16) );
                        if(tm_8bit_one_bit_set(userAuthBits,
                                    (1 << tfPppGetAuthProtocolBit
                                     (temp16, chapData))))
                        {
                            ackFlag = TM_8BIT_YES;
                        }
                        else
                        {
                            nakAuthType = tfPppFromMethodToProtocol
                                  (tfLcpGetNextAuthMethod(pppLcpStatePtr->
                                  lcpsRemoteAllowOptions.lcpAuthProtocolBits,
                                  TM_PPP_AUTHMETHOD_NULL, &chapAlgorithm));
                        }

/*
 *  LcpParseConfigAuthFinish, we either send ACK, NAK, or REJECT
 */
                        if ( userAuthBits == 0)
                        {
/* Reject because we don't support ANY authentication */
                            rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto lcpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {

                            if ( ackFlag == TM_8BIT_YES )
                            {
/* We support this protocol, so set our values */
                                remoteOptionsType |= (TM_UL(1)<<optionType);
                                remoteOptions.lcpAuthMethod =
                                     (tt8Bit)(tfPppGetAuthProtocolBit
                                     (temp16, chapData) + 1);
                            }
                            else
                            {
                                if (pppLcpStatePtr->lcpsStateInfo.
                                      cpsFailures > 0)
                                {
/* We DON'T support this protocol, so send a NAK, with our preferred value */
                                    temp16 = htons(nakAuthType);
                                    if (temp16 == TM_PPP_CHAP_PROTOCOL)
                                    {
                                        optionLength = TM_16BIT_BYTE_COUNT
                                                       + TM_8BIT_BYTE_COUNT;
                                        tempDataPtr = (tt8BitPtr) &tempData;
                                        tm_ppp_put_char_index(tempDataPtr,2,
                                                  chapAlgorithm);
                                    }
                                    else
                                    {
                                        optionLength = TM_16BIT_BYTE_COUNT;
                                        tempDataPtr = (tt8BitPtr)&tempData;
                                    }
                                    tm_bcopy(&temp16,tempDataPtr,
                                             sizeof(temp16));
                                    tm_ppp_put_char_index(
                                        dataPtr,1,
                                        ((tt8Bit)(TM_16BIT_BYTE_COUNT
                                                  +optionLength)) );
                                    nakPacketPtr = tm_ppp_add_nak(
                                                       tfPppAddNak,
                                                       dataPtr,
                                                       (tt8BitPtr)tempDataPtr,
                                                       &outDataPtr,
                                                       optionLength,
                                                       rejectPacketPtr,
                                                       nakPacketPtr,
                                                       inDataOffset,
                                                       &outDataOffset);
                                    if ((nakPacketPtr == TM_PACKET_NULL_PTR)
                                     && (rejectPacketPtr
                                         == TM_PACKET_NULL_PTR))
                                    {
                                        goto lcpParseConfigFinish;
                                    }
                                }
                                else
                                {
/* We've NAKed more than 'max-failure' times - start rejecting this option */
                                    rejectPacketPtr = tm_ppp_add_reject(
                                                               tfPppAddReject,
                                                               dataPtr,
                                                               &outDataPtr,
                                                               optionLength,
                                                               rejectPacketPtr,
                                                               nakPacketPtr,
                                                               inDataOffset,
                                                               &outDataOffset);
                                    if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                                    {
                                        goto lcpParseConfigFinish;
                                    }
                                    nakPacketPtr = TM_PACKET_NULL_PTR;
                                }
                            }
                        }
                        break;

#ifdef TM_PPP_LQM
                case TM_LCP_QUALITY_PROTOCOL:
                    tm_bcopy(optionValuePtr, &temp16, sizeof(temp16));
                    optionValuePtr += sizeof(temp16);

/* ecr fixed 3/29/02, ANVL PPP Test 15.4:
   If an Option is received (Quality Protocol) in a Configure-Request but
   with an incorrect Length, a Configure-Nak SHOULD be transmitted. */
                    if (((temp16 == TM_PPP_LQR_PROTOCOL) &&
                        (optionLength != (sizeof(tt16Bit) + sizeof(tt32Bit))))
                        || (optionLength < sizeof(tt16Bit))
                        || (pppVectPtr->pppsLqmVectPtr
                            == TM_LQM_VECT_NULL_PTR))
                    {
/* Reject because the option length is not correct, or because the
   user didn't call tfUsePppLqm. */
                        rejectPacketPtr = tm_ppp_add_reject(tfPppAddReject,
                                                            dataPtr,
                                                            &outDataPtr,
                                                            optionLength,
                                                            rejectPacketPtr,
                                                            nakPacketPtr,
                                                            inDataOffset,
                                                            &outDataOffset);
                        if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                        {
                            goto lcpParseConfigFinish;
                        }
                        nakPacketPtr = TM_PACKET_NULL_PTR;
                    }
                    else if (temp16 != TM_PPP_LQR_PROTOCOL)
                    {
/* we don't support quality protocols other than Link-Quality-Report */
                        goto lcpParseConfigNakQualityProt;
                    }
                    else
                    {
/* Get the requested LQR Reporting-Period value */
                        tm_bcopy(optionValuePtr, &temp32, sizeof(temp32));

                        if ((temp32 == (tt32Bit) 0) &&
                            (pppLcpStatePtr->lcpsLocalNegOptions.
                             lqmReportingPeriod == (tt32Bit) 0))
                        {
/* [RFC1989].R2.5:40 - a Reporting-Period value of zero MUST be Nak'd by the
   peer with an appropriate non-zero value when that peer has sent or will
   send a Configure-Request packet containing the Quality-Protocol
   Configuration Option for the Link-Quality-Report with a zero
   Reporting-Period. */

lcpParseConfigNakQualityProt:
                            if ((pppVectPtr->
                                 pppsLcpState.lcpsStateInfo.cpsFailures > 0) &&
                                (pppVectPtr->
                                 pppsLcpState.lcpsRemoteAllowOptions.
                                 lqmReportingPeriod != (tt32Bit) 0))

                            {
/* We haven't exceeded our maximum failure value, and the user called
   tfPppSetOption specifying a non-zero value that the peer is allowed
   to negotiate (i.e. remoteLocalFlag set to TM_PPP_OPT_ALLOW):
   send our peer a NAK with the allowed value for Reporting-Period. */
                                tempDataPtr = (tt8BitPtr) tempDataArray;
/* temp16 is already set to TM_PPP_LQR_PROTOCOL */
                                tm_bcopy(&temp16,
                                         tempDataPtr,
                                         sizeof(temp16));
                                temp32 = htonl(
                                    pppVectPtr->
                                    pppsLcpState.lcpsRemoteAllowOptions.
                                    lqmReportingPeriod);
                                tm_bcopy(&temp32,
                                         tempDataPtr + sizeof(temp16),
                                         sizeof(temp32));
                                nakPacketPtr = tm_ppp_add_nak(
                                    tfPppAddNak,
                                    dataPtr,
                                    tempDataPtr,
                                    &outDataPtr,
                                    optionLength,
                                    rejectPacketPtr,
                                    nakPacketPtr,
                                    inDataOffset,
                                    &outDataOffset);
                                if ( (nakPacketPtr == TM_PACKET_NULL_PTR)
                                     && (rejectPacketPtr
                                         == TM_PACKET_NULL_PTR))
                                {
                                    goto lcpParseConfigFinish;
                                }
                            }
                            else
                            {
/* We've NAKed more than 'max-failure' times - start rejecting this option */
                                rejectPacketPtr = tm_ppp_add_reject(
                                    tfPppAddReject,
                                    dataPtr,
                                    &outDataPtr,
                                    optionLength,
                                    rejectPacketPtr,
                                    nakPacketPtr,
                                    inDataOffset,
                                    &outDataOffset);
                                if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                                {
                                    goto lcpParseConfigFinish;
                                }
                                nakPacketPtr = TM_PACKET_NULL_PTR;
                            }
                        }
                        else
                        {
/* Accept the requested LQR Reporting-Period value */
                            remoteOptionsType |= (TM_UL(1)<< optionType);
                            remoteOptions.lqmReportingPeriod = ntohl(temp32);
                        }
                    }
                    break;
#endif /* TM_PPP_LQM */

                    case TM_LCP_MAGIC_NUMBER:
                        if (optionLength != TM_32BIT_BYTE_COUNT)
                        {
/* Reject because of bad length */
                            rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto lcpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {
/* Get the requested option value */
                            tm_ppp_byte_copy(optionValuePtr, valueDataOffset,
                                 (&temp32), 0,TM_32BIT_BYTE_COUNT);
                            magicNumFailure = (pppLcpStatePtr->
                                lcpsStateInfo.cpsMagicNumberFailures);
                            temp32 = ntohl(temp32);
                            if (   (temp32 !=
                                        pppLcpStatePtr->
                                        lcpsLocalNegOptions.lcpMagicNumber)
                                || (!(pppLcpStatePtr->lcpsLocalNegotiateOption
                                    & (1UL<<(tt8Bit) TM_LCP_MAGIC_NUMBER))) )
                            {
                                remoteOptionsType |= (TM_UL(1)<< optionType);
                                remoteOptions.lcpMagicNumber = temp32;
/*
 * We received a magic number option that didn't match our own, or we
 * received a magic number option when we did not send on in our Request,
 * the link is not looped back on itself.
 */
                                pppLcpStatePtr->lcpsStateInfo.
                                           cpsMagicNumberFailures=
                                    TM_LCP_MAGIC_NUM_FAILURES;

                            }
                            else
                            {
/*
 * There is a possibility that the link is in loopback mode.  To verify that we
 * didn't just happen to pick the same magic number as our peer, we send a NAK
 * with a new Magic-Number.  If we are not in loopback, the peer will send a
 * request with this new number.  Otherwise, we will receive a looped back NAK
 * with this new value (see RFC 1661 6.4).
 */
                                if (magicNumFailure > 0)
                                {

                                    (pppLcpStatePtr->lcpsStateInfo.
                                                 cpsMagicNumberFailures)--;

                                    temp32 = tfGetRandom();
                                    pppLcpStatePtr->
                                        lcpsRemoteAllowOptions.
                                        lcpMagicNumber = ntohl(temp32);
                                    nakPacketPtr = tm_ppp_add_nak(
                                                          tfPppAddNak,
                                                          dataPtr,
                                                          (tt8BitPtr)&temp32,
                                                          &outDataPtr,
                                                          optionLength,
                                                          rejectPacketPtr,
                                                          nakPacketPtr,
                                                          inDataOffset,
                                                          &outDataOffset);
                                    if ((nakPacketPtr == TM_PACKET_NULL_PTR)
                                         && (rejectPacketPtr ==
                                         TM_PACKET_NULL_PTR) )
                                    {
                                        goto lcpParseConfigFinish;
                                    }


                                }
                                else
                                {
/* We've decided the link is definitely in loopback so close it down */
                                    retPacketPtr = TM_PACKET_NULL_PTR;
                                    goto lcpParseConfigFinish;
                                }
                            }
                        }
                        break;
                    case TM_LCP_PROTOCOL_COMP:
/* Fall through because both options are processed the same */
                    case TM_LCP_ADDRCONTROL_COMP:
                        if (optionLength != 0)
                        {
/* Reject because of bad length */
                            rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto lcpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {
                            remoteOptionsType |= (TM_UL(1)<< optionType);
                        }
                        break;
#ifdef TM_USE_PPP_CALLBACK
                    case TM_LCP_CALLBACK:
                        if (optionLength < 1)
                        {
/* Reject because of bad length */
                            rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto lcpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {
/* Get the PPP Callback operation value. */
                            optionOperation = tm_ppp_get_char_next(
                                                        optionValuePtr,
                                                        valueDataOffset );
                            if (optionOperation == TM_CALLBACK_OP_CBCP)
                            {
/* Callback Control Protocol (CBCP). */
                                remoteOptionsType
                                        |= TM_UL(1) << TM_LCP_CALLBACK;
                                remoteOptions.lcpCallbackOpFlags
                                        |= TM_UL(1) << TM_CALLBACK_OP_CBCP;
                            }
                            else
                            {
/* Option not supported, so reject it. */
                                rejectPacketPtr = tm_ppp_add_reject(
                                                                 tfPppAddReject,
                                                                 dataPtr,
                                                                 &outDataPtr,
                                                                 optionLength,
                                                                 rejectPacketPtr,
                                                                 nakPacketPtr,
                                                                 inDataOffset,
                                                                 &outDataOffset);
                                if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                                {
                                    goto lcpParseConfigFinish;
                                }
                                nakPacketPtr = TM_PACKET_NULL_PTR;
                            }
                        }
                        break;
#endif /* TM_USE_PPP_CALLBACK */
                    default:
/* Option not supported so reject it*/
                        rejectPacketPtr = tm_ppp_add_reject(
                                                         tfPppAddReject,
                                                         dataPtr,
                                                         &outDataPtr,
                                                         optionLength,
                                                         rejectPacketPtr,
                                                         nakPacketPtr,
                                                         inDataOffset,
                                                         &outDataOffset);
                        if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                        {
                            goto lcpParseConfigFinish;
                        }
                        nakPacketPtr = TM_PACKET_NULL_PTR;
                        break;
                }
            }
            else
            {
/* Option not allowed so reject it */
                rejectPacketPtr = tm_ppp_add_reject(
                                                 tfPppAddReject,
                                                 dataPtr,
                                                 &outDataPtr,
                                                 optionLength,
                                                 rejectPacketPtr,
                                                 nakPacketPtr,
                                                 inDataOffset,
                                                 &outDataOffset);
                if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                {
                    goto lcpParseConfigFinish;
                }
                nakPacketPtr = TM_PACKET_NULL_PTR;
            }
        }
        else
        {
/* Option value out of range so reject it*/
            rejectPacketPtr = tm_ppp_add_reject(tfPppAddReject,
                                                dataPtr,
                                                &outDataPtr,
                                                optionLength,
                                                rejectPacketPtr,
                                                nakPacketPtr,
                                                inDataOffset,
                                                &outDataOffset);
            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
            {
                goto lcpParseConfigFinish;
            }
            nakPacketPtr = TM_PACKET_NULL_PTR;
        }

        tm_ppp_move_next_option(dataPtr, optionTotalLength, inDataOffset);

        length = (tt16Bit)(length - optionTotalLength);
    }

/*
 * If we're going to send a NAK (ie, nakPacketPtr != NULL), decrement the
 * max-failure count.
 */
    if (nakPacketPtr != TM_PACKET_NULL_PTR)
    {
        --pppLcpStatePtr->lcpsStateInfo.cpsFailures;
    }

    retPacketPtr = tm_ppp_parse_finish (tfPppParseFinish,
                                        packetPtr, rejectPacketPtr,
                                        nakPacketPtr, outDataPtr, pppEventPtr,
                                        ident, outDataOffset);
    if (retPacketPtr == packetPtr)
    {
/* Packet is an ACK (not an NAK, or Reject) */
        tm_bcopy( &remoteOptions,
                  &(pppLcpStatePtr->lcpsRemoteSetOptions),
                  sizeof(ttLcpOptions));
        pppLcpStatePtr->lcpsRemoteAckedOptions = remoteOptionsType;
    }
lcpParseConfigFinish:
    if (retPacketPtr == TM_PACKET_NULL_PTR)
    {
        tfPppNoBuffer(pppEventPtr, packetPtr);
    }
    return retPacketPtr;
}


/*
 * Process an incoming LCP packet
 */
int tfLcpIncomingPacket(ttPppVectPtr pppVectPtr,ttPacketPtr packetPtr)
{
#define devPtr (pppVectPtr->pppsDeviceEntryPtr)
    ttPppCpPacketTypePtr lcpHdrPtr;
    ttLcpStatePtr        lcpStatePtr;
    int                  isEqual;
    int                  errorCode;
    tt16Bit              optionsLength;
    tt16Bit              temp16;
    tt16Bit              maxMru;
    tt8Bit               event;
    tt8Bit               needFreePacket;
    tt8Bit               needTimerRemove;
    tt8Bit               needIpcpTimerRemove;
    tt8Bit               validAck;
    tt8Bit               cpCode;

    errorCode = TM_ENOERROR;
    lcpHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    tm_ntohs(lcpHdrPtr->cpLength, optionsLength);
    needFreePacket = TM_8BIT_ZERO;
    needTimerRemove = TM_8BIT_ZERO;
    needIpcpTimerRemove = TM_8BIT_ZERO;
    event = TM_PPPE_NO_EVENT;
    lcpStatePtr = &(pppVectPtr->pppsLcpState);
    cpCode = lcpHdrPtr->cpCode;

/* Validate LCP option length:
 *   1. Length must not be less than minimum length for this code type
 *          (e.g. 4 bytes for Configure-Req)
 *   2. Length must not be greater than actual amount of data in packet.
*/
#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(optionsLength)
#endif /* TM_LINT */
    if (    ((ttPktLen)optionsLength > packetPtr->pktChainDataLength)
         || (    ((cpCode <= TM_PPP_ECHO_REPLY) && (cpCode > 0))
              && (optionsLength < tlLcpMinPktLen[lcpHdrPtr->cpCode-1]) ) )
#ifdef TM_LINT
LINT_UNINIT_SYM_END(optionsLength)
#endif /* TM_LINT */
    {
/* Bad packet - free it and exit routine */
        needFreePacket = TM_8BIT_YES;
        goto lcpIncomingPacketFinish;
    }

    switch(cpCode)
    {
        case TM_PPP_CONFIG_REQUEST:
/* The remote sent us a config request so we gotta parse it */
#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(optionsLength)
#endif /* TM_LINT */
            packetPtr = tfLcpParseConfig(pppVectPtr,
                                         packetPtr,
                                         optionsLength,
                                         &event);
#ifdef TM_LINT
LINT_UNINIT_SYM_END(optionsLength)
#endif /* TM_LINT */
            if (event == TM_PPPE_GOOD_CFG_REQ)
            {
                if (lcpStatePtr->lcpsStateInfo.cpsState == TM_PPPS_OPENED)
                {
                    if (lcpStatePtr->lcpsStateInfo.cpsPeerReq == 0)
/*
 * Could be a duplicate. Ignore first LCP conf request when in LCP opened
 * state
 */
                    {
                        lcpStatePtr->lcpsStateInfo.cpsPeerReq = 1;
                        needFreePacket = TM_8BIT_YES;
                        event = TM_PPPE_NO_EVENT;
                    }
                }
            }
            break;
        case TM_PPP_CONFIG_ACK:
/*
 * Verify that the identifier on this Conf-ACK matches the last Conf-REQ
 * that was sent (RFC1661, section 5.2).
 */

            validAck = TM_8BIT_YES;
            if (!tm_8bit_cur_id(lcpHdrPtr->cpIdentifier,
                                lcpStatePtr->lcpsStateInfo.cpsIdentifier))
            {
                validAck = TM_8BIT_ZERO;
            }
            else
            {
/*
 * Verify that the length of the Conf-ACK is correct and the options are
 * identical to those sent in our last Conf-REQ.
 */
                if (optionsLength !=
                    (tt16Bit)(pppVectPtr->pppsLastConfReqLen
                              + TM_PPP_CP_BYTES))
                {
                    validAck = TM_8BIT_ZERO;
                }
                else
                {
                    isEqual = tm_ppp_memcmp(
                            packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE,
                            pppVectPtr->pppsLastConfReq,
                            pppVectPtr->pppsLastConfReqLen);
                    if( isEqual != TM_STRCMP_EQUAL )
                    {
                        validAck = TM_8BIT_ZERO;
                    }
                }
            }

            if (validAck == TM_8BIT_YES)
            {
/* He acked our options so now we can use them */
                tm_bcopy(&(lcpStatePtr->lcpsLocalNegOptions),
                         &(lcpStatePtr->lcpsLocalGotOptions),
                         sizeof(ttLcpOptions));

/*
 * If the peer rejected our MRU option, or the negotiated MRU is less than
 * 1500, set actual MRU to 1500 since the RFC requires that regardless of the
 * value negotiated, all PPP implementations be able to receive
 * *AT LEAST* 1500 bytes. (Mofified to 1492 for PPPoE.)
 */
#ifdef TM_USE_PPPOE
                maxMru = pppVectPtr->pppsDeviceEntryPtr->
                                            devLinkLayerProtocolPtr->lnkMtu;
#else /* !TM_USE_PPPOE */
                maxMru = TM_PPP_DEF_MRU;
#endif /* !TM_USE_PPPOE */
                if ( (lcpStatePtr->lcpsLocalGotOptions.lcpMaxReceiveUnit
                      < maxMru)
                     || ( (lcpStatePtr->lcpsLocalNegotiateOption &
                           (TM_UL(1)<<TM_LCP_MAX_RECV_UNIT)) == 0) )
                {
                    lcpStatePtr->lcpsLocalGotOptions.lcpMaxReceiveUnit = maxMru;
                }

/* Free the Packet */
                needFreePacket = TM_8BIT_YES;
/* Remove our retry timer */
                needTimerRemove = TM_8BIT_YES;
/* Crank the state machine to let it know we got an ACK */
                event = TM_PPPE_CFG_ACK;
            }
            else
            {
/* Invalid Conf-Ack packet - discard */
                needFreePacket  = TM_8BIT_YES;
                needTimerRemove = TM_8BIT_ZERO;
/*                event           = TM_PPPE_NO_EVENT;*/
            }

            break;
        case TM_PPP_CONFIG_NAK:
            if (tm_8bit_cur_id(lcpHdrPtr->cpIdentifier,
                               lcpStatePtr->lcpsStateInfo.cpsIdentifier))
            {
                errorCode = tfLcpParseNak(pppVectPtr,
                                          packetPtr,
                                          optionsLength);
                packetPtr = TM_PACKET_NULL_PTR; /* freed in tfLcpParseNak */
/*
 * Normally, when we receive a NAK the retry timer should be removed and
 * a CFG_NAK event issued to the FSM.  However, if the peer NAK'ed the
 * authentication type, the link should be closed. This will have occured in
 * tfLcpParseNak above.  In this case, we don't want to remove the timers
 * as this will remove our Terminate-Req timer and there is no need to issue
 * an event to the state machine.
 */
                if (    (lcpStatePtr->lcpsStateInfo.cpsState >
                                                           TM_PPPS_STOPPING)
                     && (errorCode == TM_ENOERROR) )
                {
/* Remove our retry timer */
                    needTimerRemove = TM_8BIT_YES;
/* Crank the state machine to let it know we got a NAK (REJECT) */
                    event = TM_PPPE_CFG_NAK;
                }
                else
                {
                    needTimerRemove = TM_8BIT_ZERO;
/*                    event = TM_PPPE_NO_EVENT;*/
                }
            }
            else
            {
/* Invalid Conf-Nak packet - discard */
                needFreePacket = TM_8BIT_YES;
/*                event = TM_PPPE_NO_EVENT;*/
            }
            break;

        case TM_PPP_CONFIG_REJECT:
            if (tm_8bit_cur_id(lcpHdrPtr->cpIdentifier,
                               lcpStatePtr->lcpsStateInfo.cpsIdentifier))
            {
                errorCode = tfLcpParseReject(pppVectPtr,
                                             packetPtr,
                                             optionsLength);
                packetPtr = TM_PACKET_NULL_PTR; /* freed in tfLcpParseReject */
                if (errorCode == TM_ENOERROR)
                {
/* Remove our retry timer */
                    needTimerRemove = TM_8BIT_YES;
/* Crank the state machine to let it know we got a NAK (REJECT) */
                    event = TM_PPPE_CFG_NAK;
                }
                else
                {
                    needTimerRemove = TM_8BIT_ZERO;
/*                    event = TM_PPPE_NO_EVENT;*/
                }
            }
            else
            {
/* Invalid Conf-Rej packet - discard */
                needFreePacket = TM_8BIT_YES;
/*                event = TM_PPPE_NO_EVENT;*/
            }
            break;

        case TM_PPP_TERM_REQUEST:
/* Remove our IPCP retry timer */
            needIpcpTimerRemove = TM_8BIT_YES;
/* Crank the state machine */
            event = TM_PPPE_TRM_REQ;
            break;
        case TM_PPP_TERM_ACK:
/* We got our term ack so crank the state machine */
            needFreePacket = TM_8BIT_YES;
/*
 * JNS/3-21-00 BUGFIX 492
 * It is not necessary to remove any timers here.  If this is actually a valid
 * state (ie, this Term-ACK was received in response to a Term-REQ and is not
 * an erroneous packet) the timers will be removed in tfPppClose().
 */
            event = TM_PPPE_TRM_ACK;
            break;

/*
 * We received a protocol reject from the peer.  If this is for a
 * mandatory protocol (such as IP) fall through to the next case (code reject)
 * which will drop the connection.  If it is for an option protocol (such
 * as LQM) disable that protocol.  If the protocol is unrecognized, discard
 * the packet.
 */
        case TM_PPP_PROTO_REJECT:
/* Only process protocol rejects if LCP is open. */
            if (lcpStatePtr->lcpsStateInfo.cpsState != TM_PPPS_OPENED)
            {
                needFreePacket = TM_8BIT_YES;
                break;
            }
/* Verify that this has at least the Rejected-Protocol field */
            if (optionsLength >= (TM_PPP_CP_BYTES + 2))
            {
                packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;

/* Parse the Rejected-Protocol field */
                tm_bcopy(
                    ((tt8BitPtr) packetPtr->pktLinkDataPtr),
                    &temp16,
                    sizeof(temp16));

                if (    (temp16 != TM_PPP_LCP_PROTOCOL)
                     && (temp16 != TM_PPP_PAP_PROTOCOL)
                     && (temp16 != TM_PPP_CHAP_PROTOCOL)
                     && (temp16 != TM_PPP_COMP_TCP_PROTOCOL)
                     && (temp16 != TM_PPP_UNCOMP_TCP_PROTOCOL)
#ifdef TM_USE_IPV4
                     && (temp16 != TM_PPP_IP_PROTOCOL)
                     && (temp16 != TM_PPP_IPCP_PROTOCOL)
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
                     && (temp16 != TM_PPP_IPV6_PROTOCOL)
                     && (temp16 != TM_PPP_IPV6CP_PROTOCOL)
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_PPP_CALLBACK
                     && !(  temp16 == TM_PPP_CBCP_PROTOCOL
                         && pppVectPtr->pppsCallbackState.cbcpsMandatory )
#endif /* TM_USE_PPP_CALLBACK */
                    )
                {

/*
 * Not a mandatory protocol - either an optional or unrecognized protocol.
 * If optional, disable the specified protocol.  In either case, free
 * packet and break from the switch.
 */
#ifdef TM_PPP_LQM
                    if (temp16 == TM_PPP_LQR_PROTOCOL)
                    {
/* If the Protocol-Reject is for LQR, then disable LQM on the link. */
                        tfLqmDisable(pppVectPtr);
                    }
#endif /* TM_PPP_LQM */
                    needFreePacket = TM_8BIT_YES;
                    break;
                }
                else
                {
/*
 * If this is an NCP (IPCP, IPV6CP) call tfPppNcpOpenFail to handle this reject
 * This will correctly handle timeouts, etc. when an NCP fails, since we don't
 * always want to kill the link.  For all other protocols, simply kill the
 * link (by falling through to the Code-Rej case).
 */
                    if (    (temp16 == TM_PPP_IPCP_PROTOCOL)
                         || (temp16 == TM_PPP_IPV6CP_PROTOCOL))
                    {
#ifdef TM_USE_IPV4
                        if (temp16 == TM_PPP_IPCP_PROTOCOL)
                        {
                            tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr,
                                            TM_LL_IP4_OPEN_FAILED);
                            tfPppTimerRemove(
                                &pppVectPtr->pppsIpcpState.ipcpsStateInfo);
                            tfIpcpDefaultInit(pppVectPtr);
                        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
                        if (temp16 == TM_PPP_IPV6CP_PROTOCOL)
                        {
                            tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr,
                                            TM_LL_IP6_OPEN_FAILED);
                            tfPppTimerRemove(
                                &pppVectPtr->pppsIp6cpState.ip6cpsStateInfo);
                            tfIp6cpDefaultInit(pppVectPtr);
                        }
#endif /* TM_USE_IPV6 */
                        tfPppNcpOpenFail(pppVectPtr);
                        needFreePacket  = TM_8BIT_YES;
                        needTimerRemove = TM_8BIT_YES;
                        break;
                    }
                }
            }

/* Fallthrough, execute TM_PPP_CODE_REJECT code */
        case TM_PPP_CODE_REJECT:
/* He rejected one of our codes, so kill the connection */
            needFreePacket = TM_8BIT_YES;
/* Remove our retry timer */
            needTimerRemove = TM_8BIT_YES;
            event = TM_PPPE_CLOSE;
/* JNS/9-2-99 BUGFIX 423 */
            break;

        case TM_PPP_ECHO_REQUEST:
/* We go an Echo request so send an echo reply */
            event = TM_PPPE_ECHO_DIS_REQP;
            break;

        case TM_PPP_ECHO_REPLY:
/* We got an echo reply */
#ifdef TM_PPP_LQM
            tfPppParseEchoReply(pppVectPtr, packetPtr, optionsLength);
            packetPtr = TM_PACKET_NULL_PTR; /* freed in tfPppParseEchoReply */
            break;
#endif /* TM_PPP_LQM */
/* Fall through, since same code as TM_PPP_DISCARD_REQUEST */
        case TM_PPP_DISCARD_REQUEST:
/* we go a discard request, so throw it away */
            needFreePacket = TM_8BIT_YES;
            break;
        default:
/* We don't know what the code is so send an unknown code */
            event = TM_PPPE_UNKNOWN_CODE;
            break;
    }

lcpIncomingPacketFinish:
    if (needFreePacket != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        packetPtr = TM_PACKET_NULL_PTR;
    }
    if (needTimerRemove != TM_8BIT_ZERO)
    {
        tfPppTimerRemove(&lcpStatePtr->lcpsStateInfo);
    }
    if (needIpcpTimerRemove != TM_8BIT_ZERO)
    {
#ifdef TM_USE_IPV4
        tfPppTimerRemove(&pppVectPtr->pppsIpcpState.ipcpsStateInfo);
#endif /* TM_USE_IPV4 */
    }
    if (event != TM_PPPE_NO_EVENT)
    {
        errorCode = tfPppStateMachine(pppVectPtr,
                                      packetPtr,
                                      event,
                                      TM_PPP_LCP_INDEX);
    }
    return errorCode;
#undef devPtr
}

/*
 * Send a protocol reject to the peer
 */
int tfLcpSendProtocolReject(ttPppVectPtr pppVectPtr,
                            ttPacketPtr packetPtr)
{
    ttPppCpPacketTypePtr protocolRejectPtr;
    ttPacketPtr          protoRejectPacketPtr;
    ttPppHdrPtr          badPppHdrPtr;
    int                  errorCode;
    tt16Bit              peerMru;
    tt16Bit              oldPktLen;
    tt16Bit              newPktLen;

    errorCode = TM_ENOERROR;

/* Save the length of the old packet */
    oldPktLen = (tt16Bit) (  (tt16Bit)packetPtr->pktLinkDataLength
                            + TM_16BIT_BYTE_COUNT ); /* protocol field*/

/*
 * Get the peer's MRU.  The MRU will always have been negotiated (or set to
 * the default) at this point, because we only send protocol rejects once
 * LCP has opened.
 */
    peerMru =   (tt16Bit)
            (pppVectPtr->pppsLcpState.lcpsRemoteSetOptions.lcpMaxReceiveUnit);

/* Compute the new packet length, trimming to the peer's MRU if necessary */
    newPktLen = (tt16Bit) (oldPktLen + TM_PPP_CP_BYTES);
    if (newPktLen > peerMru)
    {
        newPktLen = peerMru;
    }

    protoRejectPacketPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                              newPktLen,
                                              TM_16BIT_ZERO );
    if (protoRejectPacketPtr != TM_PACKET_NULL_PTR)
    {
        protocolRejectPtr = (ttPppCpPacketTypePtr)(protoRejectPacketPtr->
                                                           pktLinkDataPtr);
/* Back up to the protocol field */
        badPppHdrPtr =   (ttPppHdrPtr)(   packetPtr->pktLinkDataPtr
                                        - TM_PAK_PPP_HEADER_SIZE );
/* Copy the old packet to the end of our new packet */
#ifdef TM_DSP
        tm_ppp_byte_copy(badPppHdrPtr, 2,
                         protoRejectPacketPtr->pktLinkDataPtr +
                         TM_PAK_PPP_CP_SIZE, 0, newPktLen - TM_PPP_CP_BYTES);
#else /* !TM_DSP */
        tm_bcopy( &(badPppHdrPtr->pppHdrProtocol),
                  protoRejectPacketPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE,
                  (unsigned) newPktLen - TM_PPP_CP_BYTES );
#endif /* TM_DSP */
/* Free the old packet */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);

        protoRejectPacketPtr->pktLinkDataLength = (ttPktLen)newPktLen;
/* Update our fields */
        tm_htons(newPktLen, protocolRejectPtr->cpLength);
        protocolRejectPtr->cpCode = TM_PPP_PROTO_REJECT;
        protocolRejectPtr->cpIdentifier = pppVectPtr->
                                 pppsLcpState.lcpsStateInfo.cpsIdentifier++;
        (void) tfPppDeviceSend( pppVectPtr,
                                protoRejectPacketPtr,
                                TM_PPP_LCP_PROTOCOL);

    }
    else
    {
/* Free the old packet */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        errorCode = TM_ENOBUFS;
    }
    return errorCode;
}

/*
 * An LCP Timeout has occured
 */
void tfLcpTimeout (ttVoidPtr      timerBlockPtr,
                   ttGenericUnion userParm1,
                   ttGenericUnion userParm2)
{
    ttPppVectPtr pppVectPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    pppVectPtr = (ttPppVectPtr) userParm1.genVoidParmPtr;
    tfPppTimeout( pppVectPtr,
                  &pppVectPtr->pppsLcpState.lcpsStateInfo,
                  TM_PPP_LCP_INDEX);
}

/******************************************
 *
 *
 *
 * PPP State Machine
 *
 *
 *
 *****************************************/


 /*
 * State machine main function. Advance to next state and calls the
 * appropriate PPP function(s) based on state vector current State,
 * and event. Returns error value.
 *
 * Parameters:
 * pppVectPtr  PPP state vector pointer to be passed to the
 *             PPP function. Also used to extract/store state, and
 *             to unlock.
 * packetPtr   PPP incoming packet pointer to be passed to the
 *             PPP function.
 * event       Event (to be used as index in tl??pTransitionMatrix[][]
 *             and also to be passed to the PPP function.
 *
 * Return value
 * error
 */
int tfPppStateMachine (ttPppVectPtr pppVectPtr, /* PPP state vector */
                       ttPacketPtr  packetPtr,
/* event (to be used as index in tlPppTransitionMatrix[][] */
                       tt8Bit       event,
                       tt8Bit       controlProtoNum)
{
    ttTransitionMatrixEntryPtr matrixEntryPtr;
    ttPppStateFunctEntryPtr    pppStateFunctEntryPtr;
    ttCpStatePtr               pppStateInfoPtr;
    int                        errCode;
    tt8Bit                     curState;
    tt8Bit                     index;

/* Save current state */
    curState = pppVectPtr->pppsStateInfoPtrArray[controlProtoNum]->cpsState;
/* point to transition matrix for the current state, event pair */
    matrixEntryPtr = (ttTransitionMatrixEntryPtr)
                                    &tlPppTransitionMatrix[curState][event];
    pppStateInfoPtr = pppVectPtr->pppsStateInfoPtrArray[controlProtoNum];
/* Store next state for the event */
    pppStateInfoPtr->cpsState = matrixEntryPtr->mtxTransition;
/* Index into state function table */
    index = matrixEntryPtr->mtxFunctionIndex;
/* Check whether there is any function to call */
    if (index < TM_PPPM_LAST_INDEX)
    {
/* State function table entry pointer */
        pppStateFunctEntryPtr = (ttPppStateFunctEntryPtr)
                                                &tlPppStateFunctTable[index];
/* Call all functions corresponding to index */
        do
        {
            errCode = (*(pppStateInfoPtr->
                    cpsStateFuncList[pppStateFunctEntryPtr->pstaFunctIndex]))
                    (pppVectPtr, packetPtr);
/* If state function returned an error */
            if (errCode != TM_ENOERROR)
            {
/* restore previous state, and return */
                pppStateInfoPtr->cpsState = curState;
                break;
            }
/* Point to next entry in state function table */
            pppStateFunctEntryPtr++;
        } while (pppStateFunctEntryPtr->pstaIndex == index);
    }
    else
    {
/* No function to call, just a state transition */
        errCode = 0;
    }

    return errCode;
}


/******************************************
 *
 *
 *
 * PPP State Functions
 *
 *
 *
 *****************************************/

/*
 *
 *
 * Common LCP/IPCP State Functions
 * These funtions do the same thing in both layers
 *
 *
 */

/*
 * Send an echo reply to the peer
 * We send out the echo request with the xCP fields and the magic number
 * updated.
 */
int tfPppSendEchoReply(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    ttPppCpPacketTypePtr echoReplyPtr;
    ttLcpStatePtr        pppLcpStatePtr;
    ttLcpOptionsPtr      optionPtr;
    tt8BitPtr            dataPtr;
    tt32Bit              magicNum;
    tt16Bit              peerMru;

    pppLcpStatePtr = &(pppVectPtr->pppsLcpState);
    echoReplyPtr = (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
    echoReplyPtr->cpCode = TM_PPP_ECHO_REPLY;

/* Get the peer's magic number from the echo request */
    dataPtr = ( ((tt8BitPtr) echoReplyPtr) + TM_PAK_PPP_CP_SIZE );
    tm_bcopy( dataPtr,
              &magicNum,
              sizeof(magicNum) );

/*
 * If the peer did not negotiate the magic number option, they must send
 * echo requests with a zero magic number.  If the peer did negotiate
 * a magic number, this value must be included in the echo request.  Check
 * for these, and discard the packet if they fail.
 */
    if ( pppLcpStatePtr->lcpsRemoteSetOptions.lcpMagicNumber ==
            ntohl(magicNum) )
    {

/*
 * Copy OUR magic number into the response packet.  If no magic number has
 * been negotiated we set this field to zero (per RFC).  No need to check
 * if this option has been negotiated or not: if it hasn't, the option
 * will be set to zero.  Need to bcopy since the packet could be
 * misaligned.
 */

        optionPtr = &(pppLcpStatePtr->lcpsLocalGotOptions);
        tm_htonl(optionPtr->lcpMagicNumber, magicNum);


        tm_bcopy( &magicNum,
                  dataPtr,
                  sizeof(magicNum) );

/* Trim echo reply to peer's MRU */
        peerMru =
               pppLcpStatePtr->lcpsRemoteSetOptions.lcpMaxReceiveUnit;
        if (packetPtr->pktLinkDataLength > (ttPktLen)peerMru)
        {
            packetPtr->pktLinkDataLength  = (ttPktLen)peerMru;
            tm_htons(peerMru, echoReplyPtr->cpLength);
        }

/*
 * Because we are reusing the input packet, we pass a type of zero to tell
 * the linkLayer to just update the pointers to the header
 */
        (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);
    }
    else
    {
/* Bad magic number - discard the packet */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }

    return TM_ENOERROR;
}



/*
 * Send a Terminate ACK to the peer
 * We have already built the reject on input processing
 */
int tfPppSendTermAck(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    ttPppCpPacketTypePtr termAckPtr;

    termAckPtr = (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
    termAckPtr->cpCode = TM_PPP_TERM_ACK;
/*
 * Because we are reusing the input packet, we pass a type of zero to tell
 * the linkLayer to just update the pointers to the header
 */
    (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);

#ifdef TM_USE_IPV4
    pppVectPtr->pppsStateInfoPtrArray[TM_PPP_IPCP_INDEX]->
        cpsState = TM_PPPS_INITIAL;
    (void)tfIpcpInitRestartCounter(pppVectPtr, packetPtr);
#endif /* TM_USE_IPV4 */

    return TM_ENOERROR;
}


/*
 * Send a config ACK to the peer
 * We reuse the input packet for the ACK
 */
int tfPppSendCfgAck(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
/*
 * Because we are reusing the input packet, we pass a type of zero to tell
 * the linkLayer to just update the pointers to the header, this allows us
 * to use common routines for multiple layers
 */
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);
    }

    return TM_ENOERROR;
}

/*
 *
 *
 * LCP State Functions
 *
 *
 */



/*
 * Initialize the Restart/Retry Counter for Terminate and configure requests
 */
int tfLcpInitRestartCounter(ttPppVectPtr pppVectPtr,
                            ttPacketPtr packetPtr)
{
/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
    tfPppInitRestartCounter(&pppVectPtr->pppsLcpState.lcpsStateInfo);
    return TM_ENOERROR;
}

/*
 * Zero the Restart/Retry Counter
 */
int tfLcpZeroRestartCounter(ttPppVectPtr pppVectPtr,
                            ttPacketPtr packetPtr)
{
/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
    pppVectPtr->pppsLcpState.lcpsStateInfo.cpsRetryCounter = 0;
    return TM_ENOERROR;
}


/*
 * Send a config NAK/Reject to the peer
 * We have already built the nak/reject on input processing
 */
int tfLcpSendCfgNak(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_PPP_LCP_PROTOCOL);

    return TM_ENOERROR;
}

/*
 * Send a code reject to the peer
 */
int tfLcpSendCodeReject(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    return tfPppSendCodeReject(pppVectPtr, packetPtr,
                               &pppVectPtr->pppsLcpState.lcpsStateInfo,
                               TM_PPP_LCP_PROTOCOL);
}

/*
 * Indicate to the lower layers the we are starting the automation
 * We are suppposed to wait for a lower layer up event, but we know
 * that the device is opened before we start the state machine so
 * we just go ahead and signal the "UP" event
 */
int tfLcpThisLayerStart(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
#ifdef TM_USE_PPP_CALLBACK
/* If the Callback option is active there is nothing to do. We get here
 * by signaling that the physical layer is down when LCP is stopped. So,
 * assuming that we can signal an Up event from the physical layer is
 * wrong. The Up event will come later after the callback re-establishes
 * the physical layer. */
    if (pppVectPtr->pppsCallbackState.cbcpsIsActive)
    {
        return TM_ENOERROR;
    }
#endif /* TM_USE_PPP_CALLBACK */
    return tfPppStateMachine(pppVectPtr, packetPtr, TM_PPPE_UP,
                             TM_PPP_LCP_INDEX);
}

/*
 * Indicate to the device driver that we are done with the link
 * In our case we call the link layer notify function to tell the user
 * that the link is down.
 * Since we just notify later, we need to signal the "DOWN" event here.
 */
int tfLcpThisLayerFinish( ttPppVectPtr pppVectPtr,
                          ttPacketPtr packetPtr)
{
    ttDeviceEntryPtr devPtr;

    TM_UNREF_IN_ARG(packetPtr);
    tfPppFreeBuffersTimers(pppVectPtr);

#ifdef TM_USE_PPP_CALLBACK
/* Do not close the interface or notify the user if the Callback option
 * is active. */
    tfCbcpLcpFinish(pppVectPtr, packetPtr);
    if (pppVectPtr->pppsCallbackState.cbcpsIsActive)
    {
        goto LcpThisLayerFinishReturn;
    }
#endif /* TM_USE_PPP_CALLBACK */

    devPtr = pppVectPtr->pppsDeviceEntryPtr;
/* Close the interface, and notify the user that the link is dead */
    tfPppUserNotify(devPtr, TM_LL_CLOSE_COMPLETE);

#ifdef TM_USE_PPP_CALLBACK
LcpThisLayerFinishReturn:
#endif /* TM_USE_PPP_CALLBACK */
    return TM_ENOERROR;
}

/*
 * for LCP we just start the next layer (authentication, CBCP or IPCP)
 */
int tfLcpThisLayerUp(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    int    errorCode;

/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);

/* Reset check for duplicates LCP conf request in opened state */
    pppVectPtr->pppsLcpState.lcpsStateInfo.cpsPeerReq = 0;
/* Increment this layer's negotiation counter */
    pppVectPtr->pppsLcpState.lcpsStateInfo.cpsNegotiateCounter++;

/* Remove all upper layer timers if any */
    tfPppFreeTimers(pppVectPtr);
/* Re-initialize all upper layers & reset state machines */
#ifdef TM_USE_IPV4
    tfIpcpReInit(pppVectPtr);
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tfIp6cpReInit(pppVectPtr);
#endif /* TM_USE_IPV6 */

    tfPapInit(pppVectPtr);
    pppVectPtr->pppsPapState.papsStateInfo.cpsState   = TM_PPPS_INITIAL;

    tfChapInit(pppVectPtr);
/*
 * NOTE: This must happen *after* tfChapInit has been called, because
 *       that routine needs to be able to determine the current state of the
 *       CHAP FSM.
 */
    pppVectPtr->pppsChapState.chapsStateInfo.cpsState = TM_PPPS_INITIAL;

#ifdef TM_USE_PPP_CALLBACK
    tfCbcpLcpUp(pppVectPtr, packetPtr);
#endif /* TM_USE_PPP_CALLBACK */

/* Start the state machine */
    errorCode = tfPppNextLayer(pppVectPtr,TM_PPP_LCP_INDEX);
    tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_LCP_UP);

    return errorCode;

}

/*
 * Indicate to the upper layer that there is no link to the remote
 * NOTE: For us, we simply mark the route as being down if it is not already
 * done by IPCP or the authentication protocols
 * We also notify the user (if he asked us to)
 */
int tfLcpThisLayerDown(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
/*
 *
 * Since the LCP layer is now down, the negotiated MTU value is no longer
 * valid. Initialize the current MTU value to 1500.  This is the default
 * MTU to use while performing LCP negotiation (i.e., before an MTU value has
 * been negotiated). (Mofified to 1492 for PPPoE.)
 */
    pppVectPtr->pppsLcpState.lcpsLocalGotOptions.lcpMaxReceiveUnit =
#ifdef TM_USE_PPPOE
/* Get the MRU from the link layer */
        pppVectPtr->pppsDeviceEntryPtr->devLinkLayerProtocolPtr->lnkMtu;
#else /* !TM_USE_PPPOE */
        (tt16Bit) TM_PPP_DEF_MRU;
#endif /* TM_USE_PPPOE */

/* Update the MTU for the device */
#ifdef TM_USE_IPV4
    pppVectPtr->pppsDeviceEntryPtr->devMtu =
            pppVectPtr->pppsDeviceEntryPtr->devLinkLayerProtocolPtr->lnkMtu;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    pppVectPtr->pppsDeviceEntryPtr->dev6Mtu =
#ifdef TM_6_PMTU_DISC
            pppVectPtr->pppsDeviceEntryPtr->devLinkLayerProtocolPtr->lnkMtu;
#else /* ! TM_6_PMTU_DISC */
            TM_6_PMTU_MINIMUM;
#endif /* ! TM_6_PMTU_DISC */
#endif /* TM_USE_IPV6 */


/* Free all allocated buffers and timers. */
    tfPppFreeBuffersTimers(pppVectPtr);
/* Since LCP is not really done, or is brought down, re-initialize IPCP */
#ifdef TM_USE_IPV4
    tfIpcpReInit(pppVectPtr);
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tfIp6cpReInit(pppVectPtr);
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_PPP_CALLBACK
/* Do not notify the user if the Callback option is active. */
    if (pppVectPtr->pppsCallbackState.cbcpsIsActive)
    {
        goto LcpThisLayerDownReturn;
    }
    tfCbcpLcpDown(pppVectPtr, packetPtr);
#endif /* TM_USE_PPP_CALLBACK */
/* Notify the user that the link is dead */
    tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_CLOSE_STARTED);

#ifdef TM_USE_PPP_CALLBACK
LcpThisLayerDownReturn:
#endif /* TM_USE_PPP_CALLBACK */
    return TM_ENOERROR;
}


/*
 * Create a LCP config request packet and send it
 */
int tfLcpSendCfgRequest(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
/* Passed in value is not used here */
    ttPppCpPacketTypePtr configRequestPtr;
    ttLcpStatePtr        pppLcpStatePtr;
    tt8BitPtr            chapAlgPtr;
    tt32Bit              negotiate;
    tt32Bit              magicNumber;
    tt32Bit              accm;
#ifdef TM_PPP_LQM
    tt32Bit              reportingPeriod;
#endif /* TM_PPP_LQM */
    int                  errorCode;
    tt16Bit              temp16;
#ifdef TM_DSP
    tt16Bit              optionPtrOffset;
#endif /* TM_DSP */
    tt8Bit               temp8;
    tt8Bit               chapAlgorithm;
    tt8Bit               chapAlgLength;


    errorCode = TM_ENOERROR;
    tm_zero_dsp_offset(optionPtrOffset);
    packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                   TM_PPP_LCP_MAX_BYTES
                                   - TM_PPP_ALLOC_HDR_BYTES,
                                   TM_16BIT_ZERO );
    pppLcpStatePtr = &(pppVectPtr->pppsLcpState);

    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        configRequestPtr = (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
        packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
        negotiate = pppLcpStatePtr->lcpsLocalNegotiateOption;
/* Magic Number for loopback Detection */
        magicNumber = htonl(pppLcpStatePtr->lcpsLocalNegOptions.
                lcpMagicNumber);
        tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate,
               TM_LCP_MAGIC_NUMBER, (tt8BitPtr)&magicNumber,
               TM_32BIT_BYTE_COUNT, TM_8BIT_NULL_PTR, 0,
               (unsigned int *) &optionPtrOffset);
/* The Async Control Character Map */
        accm = htonl(pppLcpStatePtr->lcpsLocalNegOptions.
                   lcpAccm);
        tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate, TM_LCP_ACCM,
               (tt8BitPtr)&accm, TM_32BIT_BYTE_COUNT,
               TM_8BIT_NULL_PTR, 0, (unsigned int *) &optionPtrOffset);
/* The maximum receive unit that we support */
        temp16 = pppLcpStatePtr->lcpsLocalNegOptions.lcpMaxReceiveUnit;
        temp16 = htons(temp16);
        tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate,
               TM_LCP_MAX_RECV_UNIT, (tt8BitPtr)&temp16,
               TM_16BIT_BYTE_COUNT, TM_8BIT_NULL_PTR, 0,
               (unsigned int *) &optionPtrOffset);
/* The authentication protocol that we require to us. We will search all our
 * wanted authentication methods and send to peer the most preferred.
 */
        temp8 =  pppLcpStatePtr->lcpsLocalNegOptions.lcpAuthMethod;
        if(temp8 == TM_PPP_AUTHMETHOD_CHAP)
        {
            chapAlgorithm = TM_CHAP_MD5;
        }
#ifdef TM_USE_PPP_MSCHAP
        else if(temp8 == TM_PPP_AUTHMETHOD_MSCHAP_V1)
        {
            chapAlgorithm = TM_CHAP_MSV1;
        }
#endif /* TM_USE_PPP_MSCHAP */

        temp16 = tfPppFromMethodToProtocol(temp8);

        if (temp16 == TM_PPP_CHAP_HOST_PROTOCOL)
        {
/* Currently for CHAP MD5, MSchapv1, msChapv2 are supported */
            chapAlgPtr = &chapAlgorithm;
            chapAlgLength = TM_8BIT_BYTE_COUNT;
        }
        else
        {
/* PAP is the only non CHAP protocol supported */
            chapAlgPtr = (tt8BitPtr)0;
            chapAlgLength = (tt8Bit)0;
        }
        temp16 = htons(temp16);
        tm_ppp_send_option( tfPppSendOption,
                            packetPtr,
                            negotiate,
                            TM_LCP_AUTH_PROTOCOL,
                            (tt8BitPtr)&temp16,
                            TM_16BIT_BYTE_COUNT,
                            chapAlgPtr,
                            chapAlgLength,
                            (unsigned int *) &optionPtrOffset);

/* The link quality protocol we wish to use */
        temp16 =
            pppLcpStatePtr->lcpsLocalNegOptions.lcpQualityProtocol;
        temp16 = htons(temp16);
#ifdef TM_PPP_LQM
        if (pppVectPtr->pppsLqmVectPtr)
        {
/* if the user called tfUsePppLqm, then negotiate the quality protocol
   configuration option */
            reportingPeriod = htonl(
                pppVectPtr->
                pppsLcpState.lcpsLocalNegOptions.lqmReportingPeriod);
            tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate,
                               TM_LCP_QUALITY_PROTOCOL, (tt8BitPtr)&temp16,
                               TM_16BIT_BYTE_COUNT,
                               (tt8BitPtr)&reportingPeriod,
                               TM_32BIT_BYTE_COUNT,
                               (unsigned int *) &optionPtrOffset);
        }
#else /* not TM_PPP_LQM */
        tm_ppp_send_option(
            tfPppSendOption,packetPtr, negotiate,
            TM_LCP_QUALITY_PROTOCOL, (tt8BitPtr)&temp16, TM_16BIT_BYTE_COUNT,
            TM_8BIT_NULL_PTR,0, (unsigned int *) &optionPtrOffset);
#endif /* not TM_PPP_LQM */

#ifdef TM_USE_PPP_CALLBACK
/* Callback option */
        if (pppVectPtr->pppsCallbackState.cbcpsState != TM_CBCPS_STARTING2)
        {
            tfLcpSendCallbackOptions(   pppVectPtr,
                                        packetPtr
#ifdef TM_DSP
                                      , (unsigned int *)&optionPtrOffset
#endif /* TM_DSP */
                                        );
        }
#endif /* TM_USE_PPP_CALLBACK */

/* Turn on address/control field compression */
        tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate,
               TM_LCP_ADDRCONTROL_COMP, TM_8BIT_NULL_PTR, 0,
               TM_8BIT_NULL_PTR, 0, (unsigned int *) &optionPtrOffset);
/* Turn on protocol field compression */
        tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate,
               TM_LCP_PROTOCOL_COMP, TM_8BIT_NULL_PTR, 0,
               TM_8BIT_NULL_PTR, 0, (unsigned int *) &optionPtrOffset);
/* Set up our config request header */
        configRequestPtr->cpCode = TM_PPP_CONFIG_REQUEST;
        configRequestPtr->cpIdentifier = pppLcpStatePtr->
                lcpsStateInfo.cpsIdentifier++;

        temp16 = (tt16Bit)
            (tm_byte_count(
                packetPtr->pktLinkDataPtr - (tt8BitPtr)configRequestPtr));
#ifdef TM_DSP
        temp16 += optionPtrOffset;
#endif /* TM_DSP */
        packetPtr->pktLinkDataLength = (ttPktLen)(temp16);
        tm_htons(temp16, configRequestPtr->cpLength);
        packetPtr->pktLinkDataPtr = (tt8BitPtr)configRequestPtr;

/*
 * Save a copy of the Conf-Req and the length of the Conf-Req so these can
 * be validated when a Conf-Ack is received.
 */
        pppVectPtr->pppsLastConfReqLen = (tt8Bit)(temp16 - TM_PPP_CP_BYTES);
        tm_ppp_byte_copy(packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE,
                         0,
                         pppVectPtr->pppsLastConfReq,
                         0,
                         temp16 - TM_PPP_CP_BYTES);



/* Start our retry timer */
        tfPppTimerAdd( pppVectPtr,
                       tfLcpTimeout,
                       &pppLcpStatePtr->lcpsStateInfo );
/* Send it! */
        (void) tfPppDeviceSend( pppVectPtr,
                                packetPtr,
                                TM_PPP_LCP_PROTOCOL);
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }

    return errorCode;
}

#ifdef TM_USE_PPP_CALLBACK
/*
 * Pack the callback configuration options into a packet, ready to send.
 *
 * More than one form of callback may be supported and there may be a
 * variable length message attached to the option.
 */
static void tfLcpSendCallbackOptions(
                    ttPppVectPtr            pppVectPtr,
                    ttPacketPtr             packetPtr
#ifdef TM_DSP
                    , unsigned int TM_FAR * packetOffsetPtr
#endif /* TM_DSP */
                    )
{
/*
 * Note: This function only supports PPP Callback operation 6,
 * Callback Control Protocol.
 */
    ttLcpStatePtr   pppLcpStatePtr;
    tt32Bit         negotiate;
    tt32Bit         callbackOpFlags;

/* Return if Callback is not wanted. */
    pppLcpStatePtr = &pppVectPtr->pppsLcpState;
    negotiate = pppLcpStatePtr->lcpsLocalNegotiateOption;
    if ( (negotiate & (TM_UL(1) << TM_LCP_CALLBACK)) == (tt32Bit)0 )
    {
        goto LcpSendCallbackOptionsReturn;
    }

    callbackOpFlags = pppLcpStatePtr->lcpsLocalNegOptions.lcpCallbackOpFlags;

/* Callback Control Protocol (CBCP). */
    if ( (callbackOpFlags & (TM_UL(1) << TM_CALLBACK_OP_CBCP)) != (tt32Bit)0 )
    {
        tm_ppp_put_char_next(   (packetPtr->pktLinkDataPtr),
                                *packetOffsetPtr,
                                TM_LCP_CALLBACK );          /* Type */
        tm_ppp_put_char_next(   (packetPtr->pktLinkDataPtr),
                                *packetOffsetPtr,
                                3 );                        /* Length */
        tm_ppp_put_char_next(   (packetPtr->pktLinkDataPtr),
                                *packetOffsetPtr,
                                TM_CALLBACK_OP_CBCP );      /* Operation */
    }
LcpSendCallbackOptionsReturn:
    return;
}
#endif /* TM_USE_PPP_CALLBACK */

/*
 * Send a terminate request
 */
int tfLcpSendTermRequest(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr)
{
    /* Passed in value is not used here */
    ttPppCpPacketTypePtr configRequestPtr;
    int                  errorCode;
    tt16Bit              temp16;

    errorCode = TM_ENOERROR;

/*
 * If we need to, get a new packet.  Otherwise, simply reuse the one passed
 * in.
 */
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                       TM_PPP_LCP_MAX_BYTES
                                       - TM_PPP_ALLOC_HDR_BYTES,
                                       TM_16BIT_ZERO );
    }
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        configRequestPtr = (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
/* Set up our config request header */
        configRequestPtr->cpCode = TM_PPP_TERM_REQUEST;
        configRequestPtr->cpIdentifier = pppVectPtr->pppsLcpState.
                                            lcpsStateInfo.cpsIdentifier++;
        temp16 = (tt16Bit)TM_PPP_CP_BYTES;
        packetPtr->pktLinkDataLength = (ttPktLen)(temp16);
        tm_htons(temp16, configRequestPtr->cpLength);
/* Start our retry timer */
        tfPppTimerAdd( pppVectPtr,
                       tfLcpTimeout,
                       &pppVectPtr->pppsLcpState.lcpsStateInfo);
/* Send it! */
        (void) tfPppDeviceSend( pppVectPtr,
                                packetPtr,
                                TM_PPP_LCP_PROTOCOL );
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }

    return errorCode;
}

/*
 * Send a term ack to the peer
 * We have already built the reject on input processing
 */
int tfLcpSendTermAck(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    ttPppCpPacketTypePtr termAckPtr;
    int                  errorCode;
    tt16Bit              temp16;
    tt8Bit               lcpState;

    errorCode = TM_ENOERROR;

#ifdef TM_LCP_XMIT_TASK_TERM_ACK_FIX
    lcpState = pppVectPtr->pppsStateInfoPtrArray[TM_PPP_LCP_INDEX]->cpsState;
    if ((lcpState != TM_PPPS_CLOSED) &&
        (lcpState != TM_PPPS_STOPPED))
    {
        temp16 = TM_PF2_PPP_CLOSE_DEVICE;
    }
    else
    {
        temp16 = 0;
    }
#endif /* TM_LCP_XMIT_TASK_TERM_ACK_FIX */

/*
 * If we need to, get a new packet.  Otherwise, simply reuse the one passed
 * in.
 */
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                       TM_PPP_LCP_MAX_BYTES
                                       - TM_PPP_ALLOC_HDR_BYTES,
                                       TM_16BIT_ZERO );
        if (packetPtr != TM_PACKET_NULL_PTR)
        {
#ifdef TM_LCP_XMIT_TASK_TERM_ACK_FIX
            packetPtr->pktFlags2 |= temp16;
#endif /* TM_LCP_XMIT_TASK_TERM_ACK_FIX */
            termAckPtr = (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
/* Set up our config request header */
            termAckPtr->cpCode = TM_PPP_TERM_ACK;
            termAckPtr->cpIdentifier = pppVectPtr->pppsLcpState.
                                            lcpsStateInfo.cpsIdentifier++;
            temp16 = (tt16Bit)TM_PPP_CP_BYTES;
            packetPtr->pktLinkDataLength = (ttPktLen)temp16;
            tm_htons(temp16, termAckPtr->cpLength);

/* Send it! */
            (void) tfPppDeviceSend( pppVectPtr,
                                    packetPtr,
                                    TM_PPP_LCP_PROTOCOL );
        }
        else
        {
            errorCode = TM_ENOBUFS;
        }
    }
    else
    {

#ifdef TM_LCP_XMIT_TASK_TERM_ACK_FIX
        packetPtr->pktFlags2 |= temp16;
#endif /* TM_LCP_XMIT_TASK_TERM_ACK_FIX */
        termAckPtr = (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
        termAckPtr->cpCode = TM_PPP_TERM_ACK;
/*
 * Because we are reusing the input packet, we pass a type of zero to tell
 * the linkLayer to just update the pointers to the header
 */
        (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);
    }


/*
 * If LCP is not in the CLOSED or STOPPED state, sending a Term-Ack indicates
 * that the device should be closed, which is done below.  In the CLOSED or
 * STOPPED state, an Term-Ack is sent if the peer sends an out-of-state packet
 * (such as a Conf-Ack before we've sent any Conf-Reqs).  This should cause the
 * peer to restart LCP negotiation and so the link should remain open.
 */
#ifndef TM_LCP_XMIT_TASK_TERM_ACK_FIX
    lcpState = pppVectPtr->pppsStateInfoPtrArray[TM_PPP_LCP_INDEX]->cpsState;
#endif /* !TM_LCP_XMIT_TASK_TERM_ACK_FIX */
    if ( (errorCode == TM_ENOERROR) &&
         (lcpState != TM_PPPS_CLOSED) &&
         (lcpState != TM_PPPS_STOPPED) )
    {

/*
 * We got a terminate request (and sent an ACK), so stop the state machine
 * and reset all of our timers
 */
        if (    pppVectPtr->pppsStateInfoPtrArray[TM_PPP_LCP_INDEX]->cpsState
             != TM_PPPS_INITIAL)
/*
 * If we have not reached the initial state yet (this check necessary only
 * for intramachine testing)
 */
        {
            pppVectPtr->pppsStateInfoPtrArray[TM_PPP_LCP_INDEX]->
                cpsState = TM_PPPS_STOPPED;
#ifdef TM_USE_IPV4
            pppVectPtr->pppsStateInfoPtrArray[TM_PPP_IPCP_INDEX]->
                cpsState = TM_PPPS_INITIAL;
#endif /* TM_USE_IPV4 */
            pppVectPtr->pppsStateInfoPtrArray[TM_PPP_PAP_INDEX]->
                cpsState = TM_PPPS_INITIAL;
            pppVectPtr->pppsStateInfoPtrArray[TM_PPP_CHAP_INDEX]->
                cpsState = TM_PPPS_INITIAL;
        }

        (void)tfLcpInitRestartCounter(pppVectPtr, packetPtr);
#ifdef TM_USE_IPV4
        (void)tfIpcpInitRestartCounter(pppVectPtr, packetPtr);
#endif /* TM_USE_IPV4 */
        (void)tfPapInitRestartCounter(pppVectPtr, packetPtr);
        (void)tfChapInitRestartCounter(pppVectPtr, packetPtr);

#ifdef TM_USE_PPP_CALLBACK
/* Do not close the interface or notify the user if the Callback option
 * is active. */
        tfCbcpLcpFinish(pppVectPtr, packetPtr);
        if (pppVectPtr->pppsCallbackState.cbcpsIsActive)
        {
            goto LcpSendTermAckReturn;
        }
#endif /* TM_USE_PPP_CALLBACK */

/*
 * If the XMIT task is being used, we can't close down the interface until
 * the packet has been sent.
 */
#ifdef TM_LCP_XMIT_TASK_TERM_ACK_FIX
        if (!(pppVectPtr->pppsDeviceEntryPtr->devFlag & TM_DEV_XMIT_TASK))
#endif /* TM_LCP_XMIT_TASK_TERM_ACK_FIX */
        {
/*
 * Notify the user that the link is going down because of an LCP terminate
 * request (ie, the other side has closed the connection).
 */

/* JNS/8-9-99 BUGFIX 414 */
/* JNS/6-24-99 BUGFIX 397 */
/* Close the interface, and notify the user that the link is dead */
            tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr,TM_LL_CLOSE_COMPLETE);
/* BUGFIX-END */
        }
    }

#ifdef TM_USE_PPP_CALLBACK
LcpSendTermAckReturn:
#endif /* TM_USE_PPP_CALLBACK */
    return errorCode;
}

/*
 *
 *
 * PAP Functions
 *
 *
 */

/*
 * Process an incoming PAP packet
 */
int tfPapIncomingPacket(ttPppVectPtr pppVectPtr,ttPacketPtr packetPtr)
{
    ttPppCpPacketTypePtr papHdrPtr;
    int                  errorCode;
    tt16Bit              optionsLength;
    tt8Bit               event;
    tt8Bit               needFreePacket;
    tt8Bit               needTimerRemove;
    tt8BitPtr            dataPtr;
    tt8Bit               msgLength;
    tt8Bit               validAck;
    tt8Bit               index;

    errorCode = TM_ENOERROR;
    index = TM_PPP_PAP_INDEX;
    papHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    dataPtr = packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE;
    tm_ntohs(papHdrPtr->cpLength, optionsLength);
    needFreePacket = TM_8BIT_ZERO;
    needTimerRemove = TM_8BIT_ZERO;
    switch(papHdrPtr->cpCode)
    {
        case TM_PAP_AUTH_REQUEST:
/*
 * The remote sent us a config request.  Only parse it if PAP was negotiated
 * by the peer.  Otherwise, silently discard the packet.
 */
            if (pppVectPtr->pppsLcpState.lcpsLocalGotOptions.lcpAuthMethod  ==
                    TM_PPP_AUTHMETHOD_PAP)
            {
#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(optionsLength)
#endif /* TM_LINT */
                packetPtr = tfPapParseAuthRequest(pppVectPtr,
                                                  packetPtr,
                                                  optionsLength,
                                                  (tt8BitPtr)&event);
#ifdef TM_LINT
LINT_UNINIT_SYM_END(optionsLength)
#endif /* TM_LINT */
                if (event == TM_PPPE_CLOSE)
                {
/*
 * We ran out of buffers. We will close the connection (all the way to the
 * LCP layer at the end of this routine. (packetPtr is null, because was freed
 * in tfPapParseAuthRequest() when tfPppNoBuffer() was called.)
 */
                    index = TM_PPP_LCP_INDEX;
                }
                else
                {
                    errorCode = tfPppStateMachine(pppVectPtr,
                                                  packetPtr,
                                                  event,
                                                  TM_PPP_PAP_INDEX);
/* Packet freed in the state machine or before */
                    packetPtr = TM_PACKET_NULL_PTR;
                    if (event == TM_PPPE_BAD_CFG_REQ)
                    {
/*
 * Bad password sent by peer: close the connection.  Packet has been
 * freed in tm_ppp_parse_finish.
 */
/*
 * JNS/8-2-99 BUGFIX 413
 *   Previously, PPP state machine was called with a index for PAP, which
 *   would successfully close the connection, but wouldn't close the LCP
 *   layer (and therefore, the device) so when the session was restarted,
 *   tfConfigInterface would fail with a TM_EALREADY.
 */
                        event = TM_PPPE_CLOSE;
                        index = TM_PPP_LCP_INDEX;
                    }
                    else
                    {
/* Crank the state machine to let it know we got an ACK */
/* Good password, go to the next layer.  However, don't do this if we're not
 * only authenticated the peer, but being authenticated using PAP.
 */
                        if (    (pppVectPtr->pppsLcpState.
                                 lcpsRemoteSetOptions.lcpAuthMethod)
                             != TM_PPP_AUTHMETHOD_PAP)
                        {
                            event = TM_PPPE_CFG_ACK;
                        }
                        else
                        {
                            event = TM_PPPE_NO_EVENT;
                        }
                    }
                }
            }
            else
            {
/* Peer sent a Auth-Req when PAP wasn't negotiated: drop the packet */
                event = TM_PPPE_NO_EVENT;
                needFreePacket = TM_8BIT_YES;
            }
            break;
        case TM_PAP_AUTH_ACK:
/*
 * Verify that the identifier on this Conf-ACK matches the last Conf-REQ
 * that was sent (RFC1661, section 5.2).
 */
            validAck = TM_8BIT_YES;
            if (!tm_8bit_cur_id(papHdrPtr->cpIdentifier,
                    pppVectPtr->pppsPapState.papsStateInfo.cpsIdentifier))
            {
                validAck = TM_8BIT_ZERO;
            }
            else
            {
/* Verify that the stated length equals the actual length of the message */
                if ((ttPktLen)optionsLength != packetPtr->pktLinkDataLength)
                {
                    validAck = TM_8BIT_ZERO;
                }
                else
                {

/* Verify that the stated length equals the header size plus message length */
/* BUG 953: Some peers may not send the msgLength field at all when they don't
 *          send ACK messages. (RFC 1334 requires a messageLength 0 anyway). We
 *          should still accept this ACK.
 */
                    if(optionsLength > TM_PPP_CP_BYTES)
                    {
                        msgLength = tm_ppp_get_char_offset(dataPtr,0);
                        if (optionsLength
                            != (tt16Bit)(TM_PPP_CP_BYTES + msgLength + 1))
                        {
                            validAck = TM_8BIT_ZERO;
                        }
                    }
                }
            }

            if (validAck == TM_8BIT_YES)
            {
/* Remove our retry timer */
                needTimerRemove = TM_8BIT_YES;
                event = TM_PPPE_CFG_ACK;
            }
            else
            {
/* Identifier does not match last Conf-Req, discard packet */
                needFreePacket  = TM_8BIT_YES;
                needTimerRemove = TM_8BIT_ZERO;
                event           = TM_PPPE_NO_EVENT;
            }
/* Free the Packet */
            needFreePacket = TM_8BIT_YES;

            break;
        case TM_PAP_AUTH_REJECT:
        case TM_PAP_AUTH_NAK:
/* Free the Packet */
            needFreePacket = TM_8BIT_YES;
/* Remove our retry timer */
            needTimerRemove = TM_8BIT_YES;
            if (pppVectPtr->pppsPapState.papsStateInfo.cpsFailures != (tt8Bit)0)
            {
/* Crank the state machine to let it know we got a NAK (REJECT) */
                event = TM_PPPE_CFG_NAK;
                pppVectPtr->pppsPapState.papsStateInfo.cpsFailures--;
            }
            else
            {
                event = TM_PPPE_CLOSE;
                index = TM_PPP_LCP_INDEX;
            }
            break;
        default:
            event = TM_PPPE_UNKNOWN_CODE;
            break;
    }
    if (needFreePacket != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        packetPtr = TM_PACKET_NULL_PTR;
    }
    if (needTimerRemove != TM_8BIT_ZERO)
    {
        tfPppTimerRemove(&pppVectPtr->pppsPapState.papsStateInfo);
    }
    if (event != TM_PPPE_NO_EVENT)
    {
        if (event == TM_PPPE_CLOSE)
        {
            if (pppVectPtr->pppsPapState.papsRemoteReqTimerPtr
                                                 != TM_TMR_NULL_PTR)
            {
                tm_timer_remove(
                            pppVectPtr->pppsPapState.papsRemoteReqTimerPtr);
                pppVectPtr->pppsPapState.papsRemoteReqTimerPtr =
                                                        TM_TMR_NULL_PTR;
            }
        }
        errorCode = tfPppStateMachine(pppVectPtr,
                                      packetPtr,
                                      event,
                                      index);

    }
    return errorCode;
}

/*
 * Initialize the Restart/Retry Counter for Authenticate requests
 */
int tfPapInitRestartCounter ( ttPppVectPtr pppVectPtr,
                              ttPacketPtr packetPtr )
{
    ttCpStatePtr    stateInfoPtr;

/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
    stateInfoPtr = &pppVectPtr->pppsPapState.papsStateInfo;
/* We are initializing the authenticate retry value */
    stateInfoPtr->cpsRetryCounter = stateInfoPtr->cpsInitCfgRetryCnt;
    return TM_ENOERROR;
}

/*
 * Zero the Restart/Retry Counter
 */
int tfPapZeroRestartCounter(ttPppVectPtr pppVectPtr,
                             ttPacketPtr packetPtr)
{
/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
    pppVectPtr->pppsPapState.papsStateInfo.cpsRetryCounter = 0;
    return TM_ENOERROR;
}

/*
 * Indicate to the lower layers the we are starting the automation
 * We are suppposed to wait for a lower layer up event, but we know
 * that the PAP opened before we start the state machine for PAP so
 * we just go ahead and signal the "UP" event
 */
int tfPapThisLayerStart(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    return  tfPppStateMachine(pppVectPtr, packetPtr, TM_PPPE_UP,
                              TM_PPP_PAP_INDEX);
}

/*
 * Indicate to the PAP Layer that we have failed authenticaton
 * Post a "DOWN" event to PAP
 */
int tfPapThisLayerFinish(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
    tfPppTimerRemove(&pppVectPtr->pppsPapState.papsStateInfo);
    (void)tfPppStateMachine(pppVectPtr,
                            TM_PACKET_NULL_PTR,
                            TM_PPPE_CLOSE,
                            TM_PPP_LCP_INDEX);
    return TM_ENOERROR;
}

/*
 * for PAP we just start the next layer IPCP
 */
int tfPapThisLayerUp(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{

    int errorCode;

/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);

/* Increment this layer's negotiation counter */
    pppVectPtr->pppsPapState.papsStateInfo.cpsNegotiateCounter++;

/* Re-initialize the IPCP layer */
#ifdef TM_USE_IPV4
    tfIpcpReInit(pppVectPtr);
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tfIp6cpReInit(pppVectPtr);
#endif /* TM_USE_IPV6 */

    errorCode=tfPppNextLayer(pppVectPtr,TM_PPP_PAP_INDEX);

    tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_PAP_UP);
    return errorCode;
}


/*
 * Send a authenticate NAK to the peer
 * We have already built the NAK on input processing
 */
int tfPapSendAuthNak(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_PPP_PAP_PROTOCOL);

    return TM_ENOERROR;
}

/*
 * Send a code reject to the peer (The remote sent us an unknown code
 * VERY BAD)
 */
int tfPapSendCodeReject(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    return tfPppSendCodeReject(pppVectPtr, packetPtr,
                               &pppVectPtr->pppsPapState.papsStateInfo,
                               TM_PPP_PAP_PROTOCOL);
}

/*
 * Create an PAP auth request packet and send it
 */
int tfPapSendAuthRequest(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr)
{
/* Passed in value is not used here */
    ttPppCpPacketTypePtr configRequestPtr;
    tt8BitPtr            strPtr;
    tt8BitPtr            dataPtr;
    tt8Bit               len;
    int                  errorCode;
    tt16Bit              temp16;
#ifdef TM_DSP
    unsigned int         offset;
#endif /* TM_DSP */


    errorCode = TM_ENOERROR;

/*
 * If the remote did not ask for PAP when we negotiated the link then
 * we do not send an authenticate request here since we are waiting
 * for him to send us one (One of us asked for PAP for us to be here)
 */

    if (pppVectPtr->pppsLcpState.lcpsRemoteSetOptions.
                                lcpAuthMethod == TM_PPP_AUTHMETHOD_PAP)
    {
        packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                       TM_PPP_PAP_MAX_BYTES
                                       - TM_PPP_ALLOC_HDR_BYTES,
                                       TM_16BIT_ZERO );
        if (packetPtr != TM_PACKET_NULL_PTR)
        {
            configRequestPtr =
                            (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
            dataPtr =    ((tt8BitPtr)configRequestPtr)
                       + TM_PAK_PPP_CP_SIZE;

/*
 * Fill in our username/password on the remote system
 */
            strPtr = pppVectPtr->pppsPapState.papsLocalOurInfo.
                                                            papUserNamePtr;
            len = (tt8Bit) tm_pppstrlen(strPtr); /* user name length*/

            tm_zero_dsp_offset(offset);

            tm_ppp_put_char_next(dataPtr,offset,len);
            tm_ppp_byte_copy(strPtr,0, dataPtr,offset,len);
            tm_ppp_inc_ptr_n(dataPtr, offset, len);

            strPtr = pppVectPtr->pppsPapState.papsLocalOurInfo.
                                                               papPasswordPtr;
            len = (tt8Bit) tm_pppstrlen(strPtr); /* password length */
            tm_ppp_put_char_next(dataPtr,offset,len);
            tm_ppp_byte_copy(strPtr,0, dataPtr, offset, len);
            tm_ppp_inc_ptr_n(dataPtr, offset, len);

/* Set up our auth request header */
            configRequestPtr->cpCode = TM_PPP_CONFIG_REQUEST;
            configRequestPtr->cpIdentifier = pppVectPtr->pppsPapState.
                                                papsStateInfo.cpsIdentifier++;
            temp16 = (tt16Bit)
                      tm_byte_count( (dataPtr - (tt8BitPtr)configRequestPtr));
            tm_inc_dsp_offset(temp16, offset);

/* this offset should be the length counting in octets */
            packetPtr->pktLinkDataLength = (ttPktLen)(temp16);
            tm_htons(temp16, configRequestPtr->cpLength);
/*
 * If we did not ask for PAP when we negotiated the link then we need to
 * crank the state machine by simulating a received authenticate
 * request
 */
            (void) tfPppStateMachine(pppVectPtr,
                                     TM_PACKET_NULL_PTR,
                                     TM_PPPE_GOOD_CFG_REQ,
                                     TM_PPP_PAP_INDEX);

/* Start our retry timer */
            tfPppTimerAdd( pppVectPtr,
                           tfPapTimeout,
                           &pppVectPtr->pppsPapState.papsStateInfo );
/* Send it! */
            (void) tfPppDeviceSend( pppVectPtr,
                                    packetPtr,
                                    TM_PPP_PAP_PROTOCOL);
        }
        else
        {
            errorCode = TM_ENOBUFS;
        }
    }

    return errorCode;
}

/*
 * An PAP Timeout has occured
 */
void tfPapTimeout (ttVoidPtr      timerBlockPtr,
                   ttGenericUnion userParm1,
                   ttGenericUnion userParm2)
{
    ttPppVectPtr pppVectPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    pppVectPtr = (ttPppVectPtr) userParm1.genVoidParmPtr;
/* Crank the state machine to let it know we got a timeout */
    tfPppTimeout( pppVectPtr,
                  &pppVectPtr->pppsPapState.papsStateInfo,
                  TM_PPP_PAP_INDEX );
}


/*
 * The remote peer is supposed to be authenticating to us using PAP, but we
 * haven't received any Authenticate-Reqs.  Terminate the connection.
 */
void tfPapRemoteReqTimeout (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2)
{
    ttPppVectPtr     pppVectPtr;
#ifdef TM_LOCK_NEEDED
    ttDeviceEntryPtr devPtr;

    devPtr = ((ttPppVectPtr)(userParm1.genVoidParmPtr))->pppsDeviceEntryPtr;
#endif /* TM_LOCK_NEEDED */

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    tm_call_lock_wait(&(devPtr->devLockEntry));

    pppVectPtr = (ttPppVectPtr)(userParm1.genVoidParmPtr);
    if (pppVectPtr->pppsPapState.papsRemoteReqTimerPtr != TM_TMR_NULL_PTR)
    {
        pppVectPtr->pppsPapState.papsRemoteReqTimerPtr = TM_TMR_NULL_PTR;
        (void)tfPppStateMachine(pppVectPtr,
                                TM_PACKET_NULL_PTR,
                                TM_PPPE_CLOSE,
                                TM_PPP_LCP_INDEX);

    }
    tm_call_unlock(&(devPtr->devLockEntry));
}

/*
 * Initialize the PAP Layer
 */
ttVoid tfPapInit(ttPppVectPtr pppVectPtr)
{
    ttPapStatePtr papStatePtr;

    pppVectPtr->pppsStateInfoPtrArray[TM_PPP_PAP_INDEX] =
            (ttCpStatePtr)&pppVectPtr->pppsPapState;
    pppVectPtr->pppsPapState.papsStateInfo.cpsStateFuncList =
            (ttPppStateFunctPtr TM_FAR *)tlPapStateFunctTable;

    papStatePtr = &(pppVectPtr->pppsPapState);

    papStatePtr->papsRemoteReqTimerPtr = TM_TMR_NULL_PTR;
/*
 * Initialize this layer's negotiation counter.  This value is set to zero when
 * the device is first opened (ie, from a tfOpenInterface call).  Each time
 * this layer is renegotiated *within the same session* this value is
 * incremented.
 */
    papStatePtr->papsStateInfo.cpsNegotiateCounter = 0;

    papStatePtr->papsStateInfo.cpsFailures = TM_PPP_AUTH_MAX_FAILURES;
/* ecr added 01/24/01 */
    (void)tfPapInitRestartCounter(pppVectPtr, (ttPacketPtr) 0);
}

/*
 * Initialize the PAP Layer
 */
ttVoid tfPapDefaultInit(ttPppVectPtr pppVectPtr)
{
    ttPapStatePtr    papStatePtr;
    ttCpStatePtr     pppCpStatePtr;

/* JNS/8-2-99 BUGFIX 424
 *   Removed code to zero out PAP username and password buffers.
 */

    papStatePtr = &pppVectPtr->pppsPapState;
    pppCpStatePtr = &papStatePtr->papsStateInfo;
    pppCpStatePtr->cpsInitTrmRetryCnt = TM_PPP_TERM_MAX_RETRY;
    pppCpStatePtr->cpsInitCfgRetryCnt = TM_PPP_CFG_MAX_RETRY;
    pppCpStatePtr->cpsRetryTimeoutSeconds = TM_PPP_RETRY_TIMEOUT;
    pppCpStatePtr->cpsState = TM_PPPS_INITIAL;
}

/*
 * Parse the incoming PAP authentication request
 */
ttPacketPtr tfPapParseAuthRequest(ttPppVectPtr  pppVectPtr,
                                  ttPacketPtr   packetPtr,
                                  tt16Bit       length,
                                  tt8BitPtr     packetTypePtr)
{
    ttPppCpPacketTypePtr cpHdrPtr;
    ttPacketPtr          nakPacketPtr;
    ttPacketPtr          retPacketPtr;
    tt8BitPtr            dataPtr;
    tt8BitPtr            outDataPtr;
    tt8BitPtr            username;
    tt8BitPtr            password;
    ttConstCharPtr       chapMsgPtr;
    int                  authReturn;
    tt16Bit              temp16;
    tt8Bit               ident;
    tt8Bit               userLength;
    tt8Bit               passLength;
    tt8Bit               msgLength;
    unsigned int         usernameOffset;
    unsigned int         passwordOffset;
    unsigned int         i;
    tt8Bit               dataChar;
#ifdef TM_DSP
    ttCharPtr            tempUsername;
    ttCharPtr            tempPassword;
    unsigned int         inDataOffset;
    unsigned int         outDataOffset;
#endif /* TM_DSP */

    tm_zero_dsp_offset(inDataOffset);
    tm_zero_dsp_offset(outDataOffset);
    passwordOffset = 0;
    usernameOffset = 0;
    TM_UNREF_IN_ARG(length);
    nakPacketPtr = TM_PACKET_NULL_PTR;
    retPacketPtr = TM_PACKET_NULL_PTR;
    outDataPtr = (tt8BitPtr)0;
    cpHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    dataPtr = packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE;
    ident = cpHdrPtr->cpIdentifier;

    userLength = tm_ppp_get_char_next(dataPtr, inDataOffset);

    username = dataPtr;
    tm_set_dsp_offset(usernameOffset, inDataOffset);

    tm_ppp_inc_ptr_n(dataPtr,inDataOffset,userLength);

    passLength = tm_ppp_get_char_next(dataPtr, inDataOffset);

    password = dataPtr;
    tm_set_dsp_offset(passwordOffset, inDataOffset);

    tm_ppp_put_char_index(username, (usernameOffset+userLength), 0);
    tm_ppp_put_char_index(password, (passwordOffset+passLength), 0);

    authReturn = 0;

#ifdef TM_DSP
/*
 * We need to copy the packed char to unpacked format
 * for the convenience of customer, otherwise, the user provided function
 * won't recognize the packed char
 */
    if (pppVectPtr->pppsPapState.papsLocalOurInfo.papAuthenticatePtr != 0)
    {
        tempUsername = (ttCharPtr)tm_get_raw_buffer((ttPktLen)(userLength+1));
        tempPassword = (ttCharPtr)tm_get_raw_buffer((ttPktLen)(passLength+1));

        for(i=0;i<userLength;i++)
        {
            dataChar=tm_ppp_get_char_next(username,usernameOffset);
            *tempUsername++=dataChar;
        }
        *tempUsername=0;
        tempUsername -=userLength;

        for(i=0;i<passLength;i++)
        {
            dataChar=tm_ppp_get_char_next(password,passwordOffset);
            *tempPassword++=dataChar;
        }
        *tempPassword=0;
        tempPassword -= passLength;

        authReturn = (*(pppVectPtr->pppsPapState.papsLocalOurInfo.
            papAuthenticatePtr))( (ttCharPtr) tempUsername,
                                  (ttCharPtr) tempPassword );
        tm_free_raw_buffer((ttRawBufferPtr)tempUsername);
        tm_free_raw_buffer((ttRawBufferPtr)tempPassword);
    }
#else /* !TM_DSP */
    if (pppVectPtr->pppsPapState.papsLocalOurInfo.papAuthenticatePtr != 0)
    {
        authReturn = (*(pppVectPtr->pppsPapState.papsLocalOurInfo.
            papAuthenticatePtr))( (ttCharPtr) username,
                                  (ttCharPtr) password );
    }
#endif /* TM_DSP */
    if (!authReturn)
    {
/* invalid password (build NAK) */
        nakPacketPtr = tm_ppp_add_nak(tfPppAddNak,
                                      dataPtr,
                                      (tt8BitPtr)&temp16,
                                      &outDataPtr,
                                      0,                 /* no option */
                                      TM_PACKET_NULL_PTR,/* no reject packet */
                                      TM_PACKET_NULL_PTR,/* no NAK packet yet */
                                      inDataOffset,
                                      &outDataOffset);

        if (nakPacketPtr == TM_PACKET_NULL_PTR)
        {
            tfPppNoBuffer(packetTypePtr, packetPtr);
            goto papAuthFinish;
        }
/* Invalid password message */
        chapMsgPtr = &tlChapFailedMsg[0];
/* pktLinkDataPtr already pointing beyond ttPppCpPacketType */
        dataPtr = nakPacketPtr->pktLinkDataPtr;
    }
    else
    {
/* Valid Password Message */
        chapMsgPtr = &tlChapSuccessMsg[0];
/* Copy valid password message beyond ttPppCpPacketType */
        dataPtr = packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE;
/* Connection is established - remove remote request timer */
        if (pppVectPtr->pppsPapState.papsRemoteReqTimerPtr != TM_TMR_NULL_PTR)
        {
            tm_timer_remove(pppVectPtr->pppsPapState.papsRemoteReqTimerPtr);
            pppVectPtr->pppsPapState.papsRemoteReqTimerPtr = TM_TMR_NULL_PTR;
        }
    }
/* dataPtr is relocated here, and inDataOffset expired */
    tm_zero_dsp_offset(inDataOffset);
/* Copy invalid or valid password message */
    msgLength = (tt8Bit) tm_strlen(chapMsgPtr);
    tm_ppp_put_char_next(dataPtr, inDataOffset, msgLength);

    for(i=0; i < (unsigned int) msgLength; i++){
        dataChar=(unsigned char)*chapMsgPtr++;
        tm_ppp_put_char_next(dataPtr,inDataOffset,dataChar);
    }

    retPacketPtr = tm_ppp_parse_finish (tfPppParseFinish,
                                        packetPtr, TM_PACKET_NULL_PTR,
                                        nakPacketPtr, dataPtr, packetTypePtr,
                                        ident, inDataOffset);
papAuthFinish:
    return (retPacketPtr);
}

static int tfPppRegisterAuthenticate(ttUserInterface           interfaceId,
                                     ttPppAuthFunc             authFuncUnion,
                                     tt16Bit                   protocol)
{
    ttPppVectPtr     pppVectPtr;
    ttDeviceEntryPtr devPtr;
    int              errorCode;


    devPtr = (ttDeviceEntryPtr)interfaceId;
    errorCode = tfValidInterface( devPtr );
    if (errorCode == TM_ENOERROR)
    {
        tm_call_lock_wait(&(devPtr->devLockEntry));
        if (tm_ll_uses_ppp(devPtr) )
        {
            pppVectPtr = (ttPppVectPtr)(((ttDeviceEntryPtr)interfaceId)->
                                                                    devStateVectorPtr);

            switch (protocol)
            {
            case TM_PPP_PAP_HOST_PROTOCOL:
                pppVectPtr->pppsPapState.papsLocalOurInfo.papAuthenticatePtr =
                                authFuncUnion.pppaPapFuncPtr;
                break;
            case TM_PPP_CHAP_HOST_PROTOCOL:
                pppVectPtr->pppsChapState.chapsLocalOurInfo.chapAuthenticatePtr =
                                authFuncUnion.pppaChapFuncPtr;
                break;
#ifdef TM_USE_PPP_MSCHAP
            case TM_PPP_CHAP_HOST_PROTOCOL+1: /* MSCHAP Authentication function */
                pppVectPtr->pppsChapState.chapsLocalOurInfo.chapMsAuthenticatePtr =
                                authFuncUnion.pppaMsChapFuncPtr;
                break;
            case TM_PPP_CHAP_HOST_PROTOCOL+2: /* MSCHAP New password function */
                pppVectPtr->pppsChapState.chapsLocalOurInfo.chapMsNewPasswordFuncPtr =
                                authFuncUnion.pppaMsChapFuncPtr;
                break;
#endif /* TM_USE_PPP_MSCHAP */
            default:
                errorCode = TM_EPROTONOSUPPORT;
                break;
            }
        }
        else
        {
            errorCode = TM_EPERM;
        }
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/*
 * Register a function to be called when we recieve a request
 *  to authenticate our peer.
 */

int tfPapRegisterAuthenticate(ttUserInterface interfaceId,
                              ttPapAuthenticateFunctPtr funcPtr)
{
    ttPppAuthFunc authFuncUnion;
    int           retCode;

    authFuncUnion.pppaPapFuncPtr = funcPtr;
    retCode = tfPppRegisterAuthenticate(interfaceId, authFuncUnion,
                                        TM_PPP_PAP_HOST_PROTOCOL);
    return retCode;
}

/*
 *
 *
 * CHAP Functions
 *
 *
 */

/*
 * Process an incoming CHAP packet
 */
int tfChapIncomingPacket(ttPppVectPtr pppVectPtr,ttPacketPtr packetPtr)
{
    ttPppCpPacketTypePtr chapHdrPtr;
    int                  errorCode;
    tt16Bit              optionsLength;
    tt8Bit               event;
    tt8Bit               index;
    tt8Bit               needFreePacket;
    tt8Bit               authMethod;
#ifdef TM_USE_PPP_MSCHAP
    tt8Bit               retry;
#endif /* TM_USE_PPP_MSCHAP */

    TM_UNREF_IN_ARG(pppVectPtr);
    errorCode = TM_ENOERROR;
    chapHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    tm_ntohs(chapHdrPtr->cpLength, optionsLength);
    event = TM_PPPE_NO_EVENT;
    index = TM_PPP_CHAP_INDEX;
    needFreePacket = TM_8BIT_ZERO;
    authMethod = pppVectPtr->pppsLcpState.lcpsRemoteSetOptions.lcpAuthMethod;

    switch(chapHdrPtr->cpCode)
    {

        case TM_CHAP_CHALLENGE:
/* Only accept CHAP challenge if CHAP was negotiated in LCP */
            if(authMethod == TM_PPP_AUTHMETHOD_CHAP
#ifdef TM_USE_PPP_MSCHAP
                || (authMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1)
/* we don't support v2 yet */
#endif /* TM_USE_PPP_MSCHAP */
                )
            {
/*
 * in case we receive a CHAP challenge while we're in the process of
 * retransmitting a response
 */
                tfChapsResponseCleanup(&pppVectPtr->pppsChapState);
/* Send CHAP Response */
#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(optionsLength)
#endif /* TM_LINT */
                (void)tfChapChallenge(pppVectPtr,
                                      packetPtr,
                                      optionsLength);
#ifdef TM_LINT
LINT_UNINIT_SYM_END(optionsLength)
#endif /* TM_LINT */
/*
 * we don't update the state machine for challenges because we
 * just send the response
 */
            }
            else
            {
/* Received CHAP challenge, but CHAP wasn't negotiated - drop the packet. */
                needFreePacket = TM_8BIT_YES;
            }
            break;

        case TM_CHAP_RESPONSE:
/* Remove our retry timer */
            tfPppTimerRemove(&pppVectPtr->pppsChapState.chapsStateInfo);
            if (pppVectPtr->pppsStateInfoPtrArray[TM_PPP_CHAP_INDEX]->
                cpsState != TM_PPPS_OPENED)
            {
                packetPtr = tfChapParseAuthResponse(pppVectPtr,
                                                    packetPtr,
                                                    optionsLength,
                                                    &event);
                errorCode = tfPppStateMachine(pppVectPtr,
                                              packetPtr,
                                              event,
                                              TM_PPP_CHAP_INDEX);

                packetPtr = TM_PACKET_NULL_PTR;
                if (event == TM_PPPE_GOOD_CFG_REQ)
                {
                    event = TM_PPPE_CFG_ACK;
                }
                else
                {
                    event = TM_PPPE_NO_EVENT;
                    if( !pppVectPtr->pppsChapState.chapsStateInfo.cpsFailures)
                    {
/*
 * Bad password sent by peer: and we try all our MAX_AUTH_FAILUREs,
 * then close the connection.  Packet will be freed
 * when we send a Terminate request.
 */
/*
 * JNS/8-2-99 BUGFIX 413
 *   Previously, PPP state machine was called with a index for CHAP, which
 *   would successfully close the connection, but wouldn't close the LCP
 *   layer (and therefore, the device) so when the session was restarted,
 *   tfConfigInterface would fail with a TM_EALREADY.
 */
                        errorCode = tfPppStateMachine(pppVectPtr,
                                                      packetPtr,
                                                      TM_PPPE_CLOSE,
                                                      TM_PPP_LCP_INDEX);
                    }
                }

            }
            else
            {
/*
 * If CHAP is already is opened and we receive a duplicate Response packet
 * (identifier matches the last Challenge packet sent) resend the Success
 * packet.  No need to resend Failure - link should already be closed.
 */
                if (tm_8bit_cur_id(chapHdrPtr->cpIdentifier,
                    pppVectPtr->pppsChapState.chapsStateInfo.cpsIdentifier))
                {
                    (void)tfChapSendSuccess(pppVectPtr, packetPtr);
                }

            }
            break;

        case TM_CHAP_SUCCESS:
            needFreePacket = TM_8BIT_YES;
/* Free our timer and extra packet */
            tfChapsResponseCleanup(&pppVectPtr->pppsChapState);
/* Crank the state machine to let it know we got a SUCCESS */
            if (pppVectPtr->pppsStateInfoPtrArray[TM_PPP_CHAP_INDEX]->
                cpsState != TM_PPPS_OPENED)
            {
                errorCode = tfPppStateMachine(pppVectPtr,
                                              TM_PACKET_NULL_PTR,
                                              TM_PPPE_CFG_ACK,
                                              TM_PPP_CHAP_INDEX);
/*
 * Manipulate the state machine because we don't want to actually
 * send a config ACK, just move to the opened state and call
 * this layer up.
 */
                if (    (pppVectPtr->pppsLcpState.
                         lcpsLocalGotOptions.lcpAuthMethod)
                        != TM_PPP_AUTHMETHOD_CHAP )
                {
                    pppVectPtr->pppsStateInfoPtrArray[TM_PPP_CHAP_INDEX]->
                        cpsState = TM_PPPS_OPENED;
                }
                (void)tfChapThisLayerUp(pppVectPtr, TM_PACKET_NULL_PTR);
            }
            break;
        case TM_CHAP_FAILURE:
/* Bad authentication so terminate the link if
 * try all the times for MS-CHAP */
            if(authMethod == TM_PPP_AUTHMETHOD_CHAP)
            {
                event = TM_PPPE_CLOSE;
                needFreePacket = TM_8BIT_YES;
            }
#ifdef TM_USE_PPP_MSCHAP
            else
            {
/* for MSCHAP, we are going to process the failure packet */

                tfMChapV1ProcessFailurePacket(pppVectPtr,
                                              packetPtr,
                                              optionsLength,
                                              &retry);
                if(retry == 0)
                {
                    event = TM_PPPE_CLOSE;
                    needFreePacket = TM_8BIT_YES;
                }
                else
                {
                    event = TM_PPPE_CFG_NAK;
/* packet will be freed in tfMChapV1ProcessFailurePacket */
                }
            }
#endif /* TM_USE_MSCHAP*/
/*
 * By default, state machine events are issued for the CHAP layer.  However,
 * since we're closing the entire link in this case, the event should be on
 * the LCP layer.
 */
            index = TM_PPP_LCP_INDEX;
/* Free our timer and extra packet */
            tfChapsResponseCleanup(&pppVectPtr->pppsChapState);
            break;
        default:
            event = TM_PPPE_UNKNOWN_CODE;
            break;

    }
    if (needFreePacket == TM_8BIT_YES)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        packetPtr = TM_PACKET_NULL_PTR;
    }
    if (event != TM_PPPE_NO_EVENT)
    {
        errorCode = tfPppStateMachine(pppVectPtr,
                                      packetPtr,
                                      event,
                                      index);
    }
    return errorCode;
}

/*
 * Parse an incoming CHAP response
 */
ttPacketPtr tfChapParseAuthResponse(ttPppVectPtr pppVectPtr,
                                    ttPacketPtr  packetPtr,
                                    tt16Bit      optionsLength,
                                    tt8BitPtr    pppEventPtr)
{

    tt8BitPtr             dataPtr;
    tt8BitPtr             namePtr;
    tt8BitPtr             secretPtr;
    tt8BitPtr             valuePtr;
    ttPppCpPacketTypePtr  cpHdrPtr;
    tt8BitPtr             responsePtr;
    int                   i;
    int                   errorCode;
    int                   result;
    int                   secretLength;
    tt16Bit               responseSize;
    tt8Bit                authMethod;
    tt8Bit                ident;
    tt8Bit                nameSize;
#ifdef TM_DSP
    unsigned int          valuePtrOffset;
    unsigned int          nameOffset;
    ttCharPtr             username;
    tt8Bit                dataChar;
#endif /* TM_DSP */

    tm_zero_dsp_offset(nameOffset);
    TM_UNREF_IN_ARG(optionsLength);
    result = 0;
    responsePtr = (tt8BitPtr)0;
    errorCode = TM_ENOERROR;
    cpHdrPtr = (ttPppCpPacketTypePtr) packetPtr->pktLinkDataPtr;
    dataPtr = ((tt8BitPtr) cpHdrPtr) + TM_PAK_PPP_CP_SIZE;
    ident = cpHdrPtr->cpIdentifier;
    responseSize = tm_ppp_get_char_offset(dataPtr,0);
    namePtr=dataPtr;
    tm_ppp_inc_ptr_n(namePtr,nameOffset,(responseSize+1));
    nameSize = (tt8Bit)(ntohs(cpHdrPtr->cpLength) - responseSize -
                TM_PPP_CP_BYTES-1);
    tm_ppp_put_char_index(dataPtr,(nameSize+responseSize+1), 0);

/* Copy our value back from MD5 structure */
    valuePtr = pppVectPtr->pppsChapState.chapsRemoteAllowInfo.chapValue;

    secretPtr = 0;
    secretLength = 0;

    authMethod = pppVectPtr->pppsLcpState.lcpsLocalGotOptions.lcpAuthMethod;

/* Get our secret */
    if ((authMethod == TM_PPP_AUTHMETHOD_CHAP &&
         pppVectPtr->pppsChapState.chapsLocalOurInfo.
            chapAuthenticatePtr != 0)
#ifdef TM_USE_PPP_MSCHAP
        || ((authMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1 )&&
             pppVectPtr->pppsChapState.chapsLocalOurInfo.
             chapMsAuthenticatePtr != 0 )
#endif /* TM_USE_PPP_MSCHAP */
       )

    {
#ifdef TM_DSP
        username=(char*) tm_get_raw_buffer((ttPktLen)(nameSize+1));
        if(!username)
        {
            errorCode = TM_ENOBUFS;
            goto CHAPPARSE_RETURN;
        }

        for(i=0;i<nameSize;i++)
        {
            dataChar=tm_ppp_get_char_next(namePtr,nameOffset);
            *username++=dataChar;
        }
        *username = 0;
        username -= nameSize;
        secretLength = 0;

#ifdef TM_USE_PPP_MSCHAP
        if (authMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1)
        {
            secretPtr = (tt8BitPtr)(*(pppVectPtr->pppsChapState.
                chapsLocalOurInfo.chapMsAuthenticatePtr))(username,
                                                          &secretLength);
        }
        else
#endif /* TM_USE_PPP_MSCHAP */
        if(authMethod == TM_PPP_AUTHMETHOD_CHAP)
        {
            secretPtr = (tt8BitPtr)(*(pppVectPtr->pppsChapState.
                chapsLocalOurInfo.chapAuthenticatePtr))(username);
            if (secretPtr != (tt8BitPtr) TM_CHAP_INVALID_USER)
            {
                secretLength = (int)tm_strlen(secretPtr);
            }
        }
        tm_free_raw_buffer((ttRawBufferPtr)username);

#else /* !TM_DSP */
#ifdef TM_USE_PPP_MSCHAP
        if (authMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1)
/* we don't support v2 yet */
        {
            secretPtr = (tt8BitPtr)(*(pppVectPtr->pppsChapState.
                chapsLocalOurInfo.chapMsAuthenticatePtr))((ttCharPtr) namePtr,
                                                          &secretLength);
        }
        else
#endif /* TM_USE_PPP_MSCHAP */
        if(authMethod == TM_PPP_AUTHMETHOD_CHAP)
        {
            secretPtr = (tt8BitPtr)(*(pppVectPtr->pppsChapState.
                chapsLocalOurInfo.chapAuthenticatePtr))((ttCharPtr) namePtr);
            if (secretPtr != (tt8BitPtr) TM_CHAP_INVALID_USER)
            {
                secretLength = (int)tm_strlen(secretPtr);
            }
        }
#endif /* TM_DSP */
    }


/*
 * JNS/12-8-99 BUGFIX 449
 *   Fixes problem where if passed in a NULL username, the user had no method
 *   for indicating this result, and so the user would be authenticated with
 *   a null username/password.
 */
    if (secretPtr != (tt8BitPtr) TM_CHAP_INVALID_USER)
    {
        tm_zero_dsp_offset(valuePtrOffset);
        if(authMethod == TM_PPP_AUTHMETHOD_CHAP)
        {
/* for standard CHAP, it is a 16byte hash of MD5 */
            responseSize = TM_MD5_HASH_SIZE;
            responsePtr = tm_get_raw_buffer(responseSize);
            if(!responsePtr)
            {
                errorCode = TM_ENOBUFS;
                goto CHAPPARSE_RETURN;
            }

            tfChapStandardParseAuth(secretPtr,
                                    secretLength,
                                    &ident,
                                    valuePtr,
                                    responsePtr);
/* move dataPtr to point to value, not valueSize */
            tm_ppp_inc_ptr_n(dataPtr, valuePtrOffset, 1);
        }
#ifdef TM_USE_PPP_MSCHAP
        else if(authMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1)
        {
/* for MSCHAPv1, it is a 24 octets NT challenge response */
            responseSize = TM_MSCHAP_RESPONSE_SIZE;
            responsePtr = tm_get_raw_buffer(responseSize);
            if(!responsePtr)
            {
                errorCode = TM_ENOBUFS;
                goto CHAPPARSE_RETURN;
            }
            tfMChapV1NtChallengeResponse(valuePtr,
                                         secretPtr,
                                         secretLength,
                                         responsePtr);
/* move dataPtr to point to windows NT compatible challenge */
            tm_ppp_inc_ptr_n(dataPtr,
                             valuePtrOffset,
                             (1 + TM_MSCHAP_RESPONSE_SIZE));
        }
#endif /* TM_USE_PPP_MSCHAP */
        else
        {
            responsePtr = (tt8BitPtr)0;
            responseSize = 0;
        }

/* Compare our result with the reponse that the peer sent us */
#ifdef TM_DSP
        if(!tfDspMemcmp(
               (char*) responsePtr,0,(char*)dataPtr,valuePtrOffset,
               responseSize))
        {
            result = 1;
        }

#else /* !TM_DSP */
        result = 1;
        for (i=0; i < (int)responseSize; i++)
        {
            if (responsePtr[i] != dataPtr[i])
            {
                result = 0;
                break;
            }
        }
#endif /* TM_DSP */
    }
    else
    {
/* Authenication failed due to invalid username */
        result = 0;
    }


    if (result == 0)
    {
/* Send CHAP failure */
        *pppEventPtr = TM_PPPE_BAD_CFG_REQ;
/* for MS-CHAP, we need to construct the Failure packet */
    }
    else
    {
/* Send CHAP success */
        *pppEventPtr = TM_PPPE_GOOD_CFG_REQ;
    }

    if(responsePtr)
    {
        tm_free_raw_buffer(responsePtr);
    }

CHAPPARSE_RETURN:
    if(errorCode != TM_ENOERROR)
    {
/* error like TM_ENOBUFS, we need close the link */
        *pppEventPtr = TM_PPPE_CLOSE;
    }

    return packetPtr;

}


static void tfChapStandardParseAuth(tt8BitPtr       secretPtr,
                                    int             secretLength,
                                    tt8BitPtr       identPtr,
                                    tt8BitPtr       challengePtr,
                                    tt8BitPtr       responsePtr)
{

    ttMd5Ctx   md5Context;
/*
 * JNS/12-8-99 BUGFIX 449
 *   Fixes problem where if passed in a NULL username, the user had no method
 *   for indicating this result, and so the user would be authenticated with
 *   a null username/password.
 */
    if (secretPtr != (tt8BitPtr) TM_CHAP_INVALID_USER)
    {

/* Compute our MD5 result */
        tfMd5Init((ttVoidPtr) &md5Context);

/* 9 indicates unpacked char */
        tfMd5Update((ttVoidPtr)&md5Context, identPtr,
                    TM_8BIT_BYTE_COUNT, TM_DSP_UNPACKED_CHAR);
        tfMd5Update((ttVoidPtr)&md5Context, secretPtr, (ttPktLen)secretLength,
                    TM_DSP_UNPACKED_CHAR);
        tfMd5Update((ttVoidPtr)&md5Context, challengePtr, TM_CHAP_VALUE_SIZE, 0);
        tfMd5Final(responsePtr,(ttVoidPtr)&md5Context);

    }
    return;
}

/*
 * Initialize the Restart/Retry Counter for Authenticate requests
 */
int tfChapInitRestartCounter ( ttPppVectPtr pppVectPtr,
                               ttPacketPtr packetPtr )
{
    ttCpStatePtr stateInfoPtr;
    tt8Bit       chapState;

/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
    stateInfoPtr = &pppVectPtr->pppsChapState.chapsStateInfo;

/*
 * We are initializing the authenticate retry value.  Only do this if the
 * previous authentication was successful (i.e., we were not in the process
 * of authenticating when LCP was renegotiated).
 * See RFC-1334, "Security Considerations".
 */
    chapState = pppVectPtr->pppsChapState.chapsStateInfo.cpsState;
    if ( (chapState <= TM_PPPS_STOPPING) || (chapState == TM_PPPS_OPENED))
    {
        stateInfoPtr->cpsRetryCounter = stateInfoPtr->cpsInitCfgRetryCnt;
    }
    return TM_ENOERROR;
}

/*
 * Zero the Restart/Retry Counter
 */
int tfChapZeroRestartCounter(ttPppVectPtr pppVectPtr,
                             ttPacketPtr packetPtr)
{
/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
    pppVectPtr->pppsChapState.chapsStateInfo.cpsRetryCounter = 0;
    return TM_ENOERROR;
}

/*
 * Indicate to the lower layers the we are starting the automation
 * We are suppposed to wait for a lower layer up event, but we know
 * that the CHAP opened before we start the state machine for CHAP so
 * we just go ahead and signal the "UP" event
 */
int tfChapThisLayerStart(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr)
{
    return tfPppStateMachine(pppVectPtr, packetPtr, TM_PPPE_UP,
                             TM_PPP_CHAP_INDEX);
}

/*
 * Indicate to the CHAP Layer that we have failed authenticaton
 * Post a "DOWN" event to CHAP
 */
int tfChapThisLayerFinish(ttPppVectPtr pppVectPtr,
                          ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
    tfPppTimerRemove(&pppVectPtr->pppsChapState.chapsStateInfo);
    tfChapsResponseCleanup(&pppVectPtr->pppsChapState);
    (void)tfPppStateMachine(pppVectPtr,
                            TM_PACKET_NULL_PTR,
                            TM_PPPE_CLOSE,
                            TM_PPP_LCP_INDEX);
    return TM_ENOERROR;
}

/*
 * for CHAP we just start the next layer IPCP
 */
int tfChapThisLayerUp(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{

    int errorCode;

/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);

/* Increment this layer's negotiation counter */
    pppVectPtr->pppsChapState.chapsStateInfo.cpsNegotiateCounter++;

/* Re-initialize the IPCP layer */
#ifdef TM_USE_IPV4
    tfIpcpReInit(pppVectPtr);
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tfIp6cpReInit(pppVectPtr);
#endif /* TM_USE_IPV6 */

    errorCode=tfPppNextLayer(pppVectPtr,TM_PPP_CHAP_INDEX);
    tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_CHAP_UP);
    return errorCode;
}


int tfChapRegisterAuthenticate(ttUserInterface interfaceId,
                               ttChapAuthenticateFunctPtr funcPtr)
{
    ttPppAuthFunc authFuncUnion;

    authFuncUnion.pppaChapFuncPtr = funcPtr;
    return tfPppRegisterAuthenticate(interfaceId, authFuncUnion,
                                     TM_PPP_CHAP_HOST_PROTOCOL);
}

#ifdef TM_USE_PPP_MSCHAP
int tfMsChapRegisterAuthenticate(ttUserInterface              interfaceId,
                                 ttMsChapAuthenticateFunctPtr funcPtr)
{
    ttPppAuthFunc authFuncUnion;

    authFuncUnion.pppaMsChapFuncPtr = funcPtr;
    return tfPppRegisterAuthenticate(interfaceId, authFuncUnion,
                                     TM_PPP_CHAP_HOST_PROTOCOL+1);
}
#endif /* TM_USE_PPP_MSCHAP */

/*
 * Send a authenticate NAK to the peer
 * We have already built the NAK on input processing
 */
int tfChapSendAuthNak(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_PPP_CHAP_PROTOCOL);

    return TM_ENOERROR;
}

/*
 * Send a code reject to the peer (The remote sent us an unknown code
 * VERY BAD)
 */
int tfChapSendCodeReject(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr)
{
    return tfPppSendCodeReject(pppVectPtr, packetPtr,
                               &pppVectPtr->pppsChapState.chapsStateInfo,
                               TM_PPP_CHAP_PROTOCOL);
}

int tfChapSendChallenge(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{

    ttPppCpPacketTypePtr configRequestPtr;
    tt8BitPtr            dataPtr;
    tt8BitPtr            copyDataPtr;
    tt8BitPtr            namePtr;
    tt32Bit              temp32;
    int                  errorCode;
    int                  valueSize;
    int                  i;
    unsigned int         nameLength;
    tt16Bit              dataLength;
    tt8Bit               valueChar;
    tt8Bit               authMethod;
#ifdef TM_DSP
    unsigned int         copyDataOffset;
    unsigned int         dataOffset;
#endif /* TM_DSP */

    tm_zero_dsp_offset(dataOffset);
    tm_zero_dsp_offset(copyDataOffset);
    errorCode = TM_ENOERROR;
    valueSize = 0;
    authMethod = pppVectPtr->pppsLcpState.lcpsLocalGotOptions.lcpAuthMethod;

    if ( authMethod == TM_PPP_AUTHMETHOD_CHAP)
    {
        valueSize = TM_CHAP_VALUE_SIZE;
    }
#ifdef TM_USE_PPP_MSCHAP
    else if(authMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1)
    {
        valueSize = TM_CHAP_MSV1_CHALLENGE_SIZE;
    }
#endif /* TM_USE_PPP_MSCHAP */

    if(valueSize > 0)
    {
        packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                       TM_PPP_CHAP_MAX_BYTES
                                       - TM_PPP_ALLOC_HDR_BYTES,
                                       TM_16BIT_ZERO );
        if (packetPtr != TM_PACKET_NULL_PTR)
        {
            configRequestPtr = (ttPppCpPacketTypePtr)(
                                                   packetPtr->pktLinkDataPtr);
            dataPtr = ((tt8BitPtr)configRequestPtr) + TM_PAK_PPP_CP_SIZE;
            copyDataPtr=dataPtr; /* use this when saving into chapMd5.in*/

/* Set up our CHAP value */
            tm_ppp_put_char_next(dataPtr, dataOffset, valueSize);
            tm_ppp_inc_ptr_n(copyDataPtr,copyDataOffset,1);

            for (i = 0; i < (valueSize >> 2); i++)
            {
                temp32 = tfGetRandom();
                tm_ppp_byte_copy(&temp32,0,dataPtr,dataOffset,
                                 TM_32BIT_BYTE_COUNT);
                tm_ppp_inc_ptr_n(dataPtr,dataOffset,TM_32BIT_BYTE_COUNT);
            }

/* Save our CHAP value temporarily in chapValue char array */
            tm_ppp_byte_copy(copyDataPtr,copyDataOffset,
                pppVectPtr->pppsChapState.chapsRemoteAllowInfo.chapValue,0,
                valueSize);
#ifdef TM_USE_PPP_MSCHAP
            /* MSCHAP doesn't provide information in the name field */
            if (authMethod != TM_PPP_AUTHMETHOD_MSCHAP_V1)
#endif /* TM_USE_PPP_MSCHAP */

            {
/* Set up our CHAP username, such as 'I_am_Dial-up_Server'. This field is not
 * processed in Treck PPP client, as well as many other PPP client implementations,
 * (The client doesn't care what the server's name is).
 * Thus here we just send out the peer's username. For MS-CHAP, Microsoft realizes
 * that this field is not used, MS-CHAP doesn't send name field at all.
 */
                namePtr = pppVectPtr->pppsChapState.chapsRemoteAllowInfo.
                          chapUserNamePtr;
                if( namePtr == (tt8BitPtr)0 )
                {
                    nameLength = 0;
                }
                else
                {
                    nameLength= (unsigned int) tm_pppstrlen(namePtr);
                    i=0;
                    while( nameLength )
                    {
                        valueChar=tm_ppp_get_char_next(namePtr,i);
                        tm_ppp_put_char_next(dataPtr, dataOffset, valueChar);
                        nameLength--;
                    }
                }
            }

/* Set up our auth request header */
            configRequestPtr->cpCode = TM_CHAP_CHALLENGE;
            configRequestPtr->cpIdentifier = pppVectPtr->pppsChapState.
                                               chapsStateInfo.cpsIdentifier++;

            dataLength = (tt16Bit)
                           tm_byte_count(dataPtr - (tt8BitPtr)configRequestPtr);
            tm_inc_dsp_offset(dataLength, dataOffset);

            packetPtr->pktLinkDataLength = (ttPktLen)dataLength;

            tm_htons(dataLength, configRequestPtr->cpLength);

/* Start our retry timer */
            tfPppTimerAdd( pppVectPtr,
                           tfChapTimeout,
                           &pppVectPtr->pppsChapState.chapsStateInfo );
/* Send it! */
            (void) tfPppDeviceSend( pppVectPtr,
                                    packetPtr,
                                    TM_PPP_CHAP_PROTOCOL);
        }
        else
        {
            errorCode = TM_ENOBUFS;
        }
    }

    return errorCode;
}

/*
 * Create a CHAP success packet
 */

int tfChapSendSuccess(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    return tfChapSendMessage(pppVectPtr, packetPtr, (tt8Bit)TM_CHAP_SUCCESS);
}

/*
 * Create a CHAP failure packet
 */

int tfChapSendFailure(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    return tfChapSendMessage(pppVectPtr, packetPtr, (tt8Bit)TM_CHAP_FAILURE);
}


/*
 * An CHAP Timeout has occured
 */
void tfChapTimeout (ttVoidPtr      timerBlockPtr,
                    ttGenericUnion userParm1,
                    ttGenericUnion userParm2)
{
    ttPppVectPtr pppVectPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    pppVectPtr = (ttPppVectPtr) userParm1.genVoidParmPtr;
/* Crank the state machine to let it know we got a timeout */
    tfPppTimeout( pppVectPtr,
                  &pppVectPtr->pppsChapState.chapsStateInfo,
                  TM_PPP_CHAP_INDEX);
}

/*
 * Initialize the CHAP Layer
 */
ttVoid tfChapInit(ttPppVectPtr pppVectPtr)
{
    ttChapStatePtr  chapStatePtr;

    pppVectPtr->pppsStateInfoPtrArray[TM_PPP_CHAP_INDEX] =
            (ttCpStatePtr)&pppVectPtr->pppsChapState;

    chapStatePtr = &pppVectPtr->pppsChapState;
    chapStatePtr->chapsStateInfo.cpsStateFuncList =
            (ttPppStateFunctPtr TM_FAR *)tlChapStateFunctTable;
    chapStatePtr->chapsResponseTimerPtr = TM_TMR_NULL_PTR;
    chapStatePtr->chapsResponsePacketPtr = TM_PACKET_NULL_PTR;
    chapStatePtr->chapsStateInfo.cpsFailures = TM_PPP_AUTH_MAX_FAILURES;
/*
 * Initialize this layer's negotiation counter.  This value is set to zero when
 * the device is first opened (ie, from a tfOpenInterface call).  Each time
 * this layer is renegotiated *within the same session* this value is
 * incremented.
 */
    pppVectPtr->pppsChapState.chapsStateInfo.cpsNegotiateCounter = 0;

/* ecr added 01/24/01 */
    (void)tfChapInitRestartCounter(pppVectPtr, (ttPacketPtr) 0);
}

/*
 * Initialize the CHAP Layer back to default values
 */
ttVoid tfChapDefaultInit(ttPppVectPtr pppVectPtr)
{
    ttChapStatePtr  chapStatePtr;
    ttCpStatePtr    pppCpStatePtr;


/* JNS/8-2-99 BUGFIX 424
 *   Removed code to zero out PAP username and password buffers.
 */

    chapStatePtr = &pppVectPtr->pppsChapState;
    pppCpStatePtr = &chapStatePtr->chapsStateInfo;
    pppCpStatePtr->cpsInitTrmRetryCnt = TM_PPP_TERM_MAX_RETRY;
    pppCpStatePtr->cpsInitCfgRetryCnt = TM_PPP_CFG_MAX_RETRY;
    pppCpStatePtr->cpsRetryTimeoutSeconds = TM_PPP_RETRY_TIMEOUT;
    pppCpStatePtr->cpsState = TM_PPPS_INITIAL;
}

/*
 * Create a Chap Response from a chap challenge
*/
#ifdef TM_LINT
LINT_UNACCESS_SYM_BEGIN(length)
#endif /* TM_LINT */
int tfChapChallenge(ttPppVectPtr  pppVectPtr,
                    ttPacketPtr   packetPtr,
                    tt16Bit       length)
{
    tt8BitPtr             dataPtr;
    tt8BitPtr             valuePtr;
    tt8BitPtr             usernamePtr;
    tt8BitPtr             passwordPtr;
    ttChapOptionsPtr      chapInfoPtr;
    ttPppCpPacketTypePtr  cpHdrPtr;
    ttPacketPtr           dupPacketPtr;
    ttPacketPtr           newPacketPtr;
    tt8BitPtr             responsePtr;
    ttTimerPtr            timerPtr;
    ttGenericUnion        timerParm1;
    ttPktLen              allocSize;
    unsigned int          valuePtrOffset;
    int                   errorCode;
    tt16Bit               usernameLength;
    tt16Bit               passwordLength;
    tt16Bit               responseSize;
    tt16Bit               responseTotalSize;
    tt8Bit                authMethod;
    tt8Bit                ident;
    tt8Bit                valueSize;

    TM_UNREF_IN_ARG(length);
    errorCode = TM_ENOERROR;
    cpHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    dataPtr = ((tt8BitPtr)cpHdrPtr) + TM_PAK_PPP_CP_SIZE;
    ident = cpHdrPtr->cpIdentifier;
    valuePtrOffset = 0;
    valuePtr = dataPtr;
    valueSize = tm_ppp_get_char_next(valuePtr,valuePtrOffset);
    if (valueSize > TM_CHAP_MAX_VALUE_SIZE)
    {
        valueSize = TM_CHAP_MAX_VALUE_SIZE; /* size of array we copy into */
    }
/* Save the CHAP value in chapValue char array */
    tm_ppp_byte_copy(valuePtr, valuePtrOffset,
                pppVectPtr->pppsChapState.chapsLocalOurInfo.chapValue,
                0,
                valueSize);
    chapInfoPtr = &pppVectPtr->pppsChapState.chapsLocalOurInfo;
    authMethod = pppVectPtr->pppsLcpState.lcpsRemoteSetOptions.lcpAuthMethod;
    usernamePtr = chapInfoPtr->chapUserNamePtr;
    usernameLength = (tt8Bit) tm_pppstrlen(usernamePtr);
    passwordPtr = chapInfoPtr->chapSecretPtr;
    passwordLength = TM_16BIT_ZERO;
#ifdef TM_USE_PPP_MSCHAP
    if(authMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1)
    {
        passwordPtr = chapInfoPtr->chapMsSecretPtr;
    }
#endif /* TM_USE_PPP_MSCHAP */

    if(authMethod == TM_PPP_AUTHMETHOD_CHAP)
    {
        passwordLength = (tt8Bit) tm_pppstrlen(passwordPtr);
/* responsePtr contains the response data ONLY, no PPP cpPacketTypeHeader,
 * nor name field.
 */
/* for standard CHAP, it is a 16byte hash of MD5 */
        responseSize = TM_MD5_HASH_SIZE;
        responsePtr = tm_get_raw_buffer(responseSize);
        if(!responsePtr)
        {
            errorCode = TM_ENOBUFS;
            goto CHAPCHAL_RETURN;
        }
/* the total response length = cpHeader + value_size + value + namelength */
        responseTotalSize = (tt16Bit) (TM_PPP_CP_BYTES + 1 + TM_MD5_HASH_SIZE
                          + usernameLength);
    }
#ifdef TM_USE_PPP_MSCHAP
    else if(authMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1)
    {
        passwordLength = chapInfoPtr->chapMsSecretLength;
/* for MSCHAPv1, it is a 24 octets NT challenge response */
        responseSize = TM_MSCHAP_RESPONSE_SIZE;
        responsePtr = tm_get_raw_buffer(responseSize);
        if(!responsePtr)
        {
            errorCode = TM_ENOBUFS;
            goto CHAPCHAL_RETURN;
        }
/* the total response length = cpHeader + value_size + 2*24 + 1 (flag)
 + namelength */
        responseTotalSize = (tt16Bit) (TM_PPP_CP_BYTES + 1 +
                 TM_MSCHAP_RESPONSE_PACKET_SIZE + usernameLength);
    }
#endif /* TM_USE_PPP_MSCHAP */
    else
    {
        responsePtr = (tt8BitPtr)0;
        responseSize = 0;
        responseTotalSize = 0;
    }
    if (responsePtr != (tt8BitPtr)0)
    {
        if (
#ifdef TM_USE_PPPOE
               (tm_ll_is_pppoe(pppVectPtr->pppsDeviceEntryPtr)) ||
#endif /* TM_USE_PPPOE */
               ((int)(packetPtr->pktSharedDataPtr->dataBufLastPtr -
                (tt8BitPtr)cpHdrPtr) < (int)responseTotalSize)
            )
        {
/* The response packet data could be longer than
 * the challenge data. Needs to check if we can fit in. If not,
 * we have to allocate another shared data block.
 * For PPPoE the packet is the original driver buffer, so it is best
 * to allocate a new packet, since we are keeping the packet for a while.
 */
#ifdef TM_USE_PPPOE
/*
 * For PPPoE the new packet could be smaller. We have to make sure that it
 * is at least as big, as we are copying the input user name to verify it.
 */
            if (packetPtr->pktLinkDataLength > (ttPktLen)responseTotalSize)
            {
                allocSize = packetPtr->pktLinkDataLength;
            }
            else
#endif /* TM_USE_PPPOE */
            {
                allocSize = (ttPktLen)(responseTotalSize);
            }
            newPacketPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                              allocSize,
                                              TM_16BIT_ZERO );
            if (newPacketPtr == (ttPacketPtr)0 )
            {
                errorCode = TM_ENOBUFS;
                goto CHAPCHAL_RETURN;
            }
/*
 * Note for PPPoE this will not copy the whole header, but copying the PPP
 * protocol field is enough because PPPoE will add the header anyway.
 */
            tm_bcopy(packetPtr->pktLinkDataPtr - TM_PAK_PPP_HEADER_SIZE,
                     newPacketPtr->pktLinkDataPtr - TM_PAK_PPP_HEADER_SIZE,
                     packetPtr->pktLinkDataLength + TM_PPP_HDR_BYTES);
            cpHdrPtr = (ttPppCpPacketTypePtr)newPacketPtr->pktLinkDataPtr;
            dataPtr = ((tt8BitPtr)cpHdrPtr) + TM_PAK_PPP_CP_SIZE;
            valuePtr = dataPtr;
            valuePtrOffset = 0;
            tm_ppp_inc_ptr_n(valuePtr,valuePtrOffset,1);
            tfFreePacket(packetPtr, (tt8Bit)0);
            packetPtr = newPacketPtr;
        }
        if(authMethod == TM_PPP_AUTHMETHOD_CHAP)
        {
            errorCode = tfChapStandardChallenge(&ident,
                                                passwordPtr,
                                                passwordLength,
                                                valuePtr,
                                                valueSize,
                                                (tt8Bit)valuePtrOffset,
                                                responsePtr);
        }
#ifdef TM_USE_PPP_MSCHAP
        else
        {
            tfMChapV1NtChallengeResponse
                               (valuePtr,
                                passwordPtr,
                                passwordLength,
                                responsePtr);
        }
#endif /* TM_USE_PPP_MSCHAP */
    }

/* Update the packet with our response */
    packetPtr->pktLinkDataLength = (ttPktLen)responseTotalSize;
    tm_htons(responseTotalSize, cpHdrPtr->cpLength);
    cpHdrPtr->cpCode = TM_CHAP_RESPONSE;

    valuePtrOffset=0;

    if(authMethod == TM_PPP_AUTHMETHOD_CHAP)
    {
        tm_ppp_put_char_next(dataPtr, valuePtrOffset, responseSize);
/* we must do this copy, rather than pass the linkDataPtr to the
 * challenge computing procedure, because it is not 4-byte aligned
 * provided the valueSize field
 */
        tm_ppp_byte_copy(responsePtr,0, dataPtr,valuePtrOffset,
                     TM_MD5_HASH_SIZE);

        tm_ppp_inc_ptr_n(dataPtr,valuePtrOffset,TM_MD5_HASH_SIZE);
    }
#ifdef TM_USE_PPP_MSCHAP
    else
    {
/* 24 octets of LAN manager compatible challenge response always zero.
 * 24 octets Windows NT compatible challenge response
 * 1 octet of flags always 1
 */
        tm_bzero(dataPtr, TM_MSCHAP_RESPONSE_PACKET_SIZE);
        tm_ppp_put_char_next(dataPtr, valuePtrOffset,
                             TM_MSCHAP_RESPONSE_PACKET_SIZE);
        tm_ppp_inc_ptr_n(dataPtr, valuePtrOffset, responseSize);
        tm_ppp_byte_copy(responsePtr,0, dataPtr,valuePtrOffset,
                         responseSize);
        tm_ppp_inc_ptr_n(dataPtr, valuePtrOffset, responseSize);
/* set the flag to 1 and increases the dataPtr*/
        tm_ppp_put_char_next(dataPtr, valuePtrOffset, 0x01);
/* construct the response field for MSCHAP,*/
    }
#endif /* TM_USE_PPP_MSCHAP */

/* construct the name field */
    tm_ppp_byte_copy(usernamePtr,0, dataPtr, valuePtrOffset,usernameLength);

    if(responsePtr != (tt8BitPtr)0)
    {
        tm_free_raw_buffer(responsePtr);
    }


    errorCode = TM_ENOERROR;
    if (packetPtr != TM_PACKET_NULL_PTR)
    {

        tfChapsResponseCleanup(&pppVectPtr->pppsChapState);
/* Duplicate the packet so it won't be free until we get a success */
        dupPacketPtr = tfDuplicatePacket(packetPtr);
        if (dupPacketPtr == TM_PACKET_NULL_PTR)
        {
/* No more buffers */
            errorCode = TM_ENOBUFS;
/* Store original packet so it will be free at the end of this routine */
            pppVectPtr->pppsChapState.chapsResponsePacketPtr = packetPtr;
        }
        else
        {
            pppVectPtr->pppsChapState.chapsResponsePacketPtr = dupPacketPtr;
            (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);
        }
    }

    if (errorCode == TM_ENOERROR)
    {
/* If no error occured add a timer to retransmit the response */
        pppVectPtr->pppsChapState.chapsResponseCounter =
            pppVectPtr->pppsChapState.chapsStateInfo.cpsInitCfgRetryCnt;
/* Start our retry timer */
        timerParm1.genVoidParmPtr = (ttVoidPtr)pppVectPtr;
        timerPtr = tfTimerAdd( tfChapResponseTimeout,
                               timerParm1,
                               timerParm1, /* unused */
                               (tt32Bit)(pppVectPtr->pppsChapState.
                               chapsStateInfo.cpsRetryTimeoutSeconds) *
                                    TM_UL(1000),
                               0);
        if (timerPtr == TM_TMR_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
        }
        else
        {
            pppVectPtr->pppsChapState.chapsResponseTimerPtr = timerPtr;
            --pppVectPtr->pppsChapState.chapsResponseCounter;
        }
    }
    if (errorCode != TM_ENOERROR)
    {
/* If an error occured, free allocated packet if any */
        tfChapsResponseCleanup(&pppVectPtr->pppsChapState);
/* If an error occurred, crank the state machine and terminate the link */
        (void)tfPppStateMachine(pppVectPtr,
                          TM_PACKET_NULL_PTR,
                          TM_PPPE_LAST_TIMEOUT,
                          TM_PPP_CHAP_INDEX);
    }
CHAPCHAL_RETURN:
    return errorCode;
}
#ifdef TM_LINT
LINT_UNACCESS_SYM_END(length)
#endif /* TM_LINT */


static int tfChapStandardChallenge(tt8BitPtr     identPtr,
                                   tt8BitPtr     passwordPtr,
                                   tt16Bit       passwordLen,
                                   tt8BitPtr     valuePtr,
                                   tt16Bit       valueSize,
/* value pointer is not on a 16bit aligned position (value_size field is)
 * |Code + ID + LENG TH | value_size + VALUE
 */
                                   tt8Bit        valuePtrOffset,
                                   tt8BitPtr     responsePtr)
{
    ttMd5Ctx             md5Context;

    tfMd5Init((ttVoidPtr)(&md5Context));
/* ident is an unpacked char */
    tfMd5Update((ttVoidPtr)(&md5Context),identPtr,
              (tt16Bit)(TM_8BIT_BYTE_COUNT), TM_DSP_UNPACKED_CHAR);
/* password is saved in packed format, with original offset zero */
    tfMd5Update((ttVoidPtr)(&md5Context),passwordPtr, passwordLen, 0);
    tfMd5Update((ttVoidPtr)(&md5Context),
                valuePtr, (tt16Bit)valueSize, valuePtrOffset);
    tfMd5Final(responsePtr, (ttVoidPtr)&md5Context);

    return TM_ENOERROR;
}

/*
 * A CHAP Timeout has occured
 */
void tfChapResponseTimeout (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2)
{

    ttPppVectPtr     pppVectPtr;
    ttTimerPtr       timerPtr;
    ttPacketPtr      packetPtr;
    ttPacketPtr      dupPacketPtr;
    ttChapStatePtr   chapStatePtr;
#ifdef TM_LOCK_NEEDED
    ttDeviceEntryPtr devPtr;
#endif /* TM_LOCK_NEEDED */
    int              errorCode;
    ttGenericUnion   timerParm1;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    pppVectPtr = (ttPppVectPtr)userParm1.genVoidParmPtr;
    errorCode = TM_ENOERROR;
#ifdef TM_LOCK_NEEDED
    devPtr = pppVectPtr->pppsDeviceEntryPtr;
#endif /* TM_LOCK_NEEDED */

    tm_call_lock_wait(&(devPtr->devLockEntry));

    chapStatePtr = &pppVectPtr->pppsChapState;
    timerPtr = chapStatePtr->chapsResponseTimerPtr;
    if (timerPtr != TM_TMR_NULL_PTR)
    {
/* If timer has not been removed yet */
        chapStatePtr->chapsResponseTimerPtr = TM_TMR_NULL_PTR;
        packetPtr = chapStatePtr->chapsResponsePacketPtr;
        if (packetPtr != TM_PACKET_NULL_PTR)
        {
            dupPacketPtr = tfDuplicatePacket(packetPtr);
            if (dupPacketPtr == TM_PACKET_NULL_PTR)
            {
                errorCode = TM_ENOBUFS;
            }
            else
            {
                chapStatePtr->chapsResponsePacketPtr = dupPacketPtr;
                (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);
            }
            if (errorCode == TM_ENOERROR)
            {
/* Start our retry timer */
                chapStatePtr->chapsResponseCounter--;
                if( chapStatePtr->chapsResponseCounter == 0 )
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfChapResponseTimeout",
                                    "Max retries reached");
#endif /* TM_ERROR_CHECKING */
                    errorCode = TM_ETIMEDOUT;
                }
                else
                {
                    timerParm1.genVoidParmPtr = (ttVoidPtr)pppVectPtr;
                    timerPtr = tfTimerAdd( tfChapResponseTimeout,
                                       timerParm1,
                                       timerParm1, /* unused */
                                       (tt32Bit)(chapStatePtr->
                                        chapsStateInfo.cpsRetryTimeoutSeconds)
                                       *  TM_UL(1000),
                                       0);
                    if (timerPtr != TM_TMR_NULL_PTR)
                    {
                        chapStatePtr->chapsResponseTimerPtr = timerPtr;
                    }
                    else
                    {
                        errorCode = TM_ENOBUFS;
                    }
                }
            }
            if (errorCode != TM_ENOERROR)
            {
                tfChapsResponseCleanup(chapStatePtr);
/* Crank the state machine and terminate the link */
                (void)tfPppStateMachine( pppVectPtr,
                                   TM_PACKET_NULL_PTR,
                                   TM_PPPE_LAST_TIMEOUT,
                                   TM_PPP_CHAP_INDEX);
            }
        }
    }
    tm_call_unlock(&(devPtr->devLockEntry));
}

static int tfChapSendMessage( ttPppVectPtr pppVectPtr,
                              ttPacketPtr  packetPtr,
                              tt8Bit       chapCode)
{
    ttCpStatePtr         chapInfoPtr;
    ttPppCpPacketTypePtr cpHdrPtr;
    tt8BitPtr            dataPtr;
    ttCharPtr            chapMsgPtr;
    tt16Bit              temp16;
    tt8Bit               chapMsgLength;
#ifdef TM_USE_PPP_MSCHAP
    tt8Bit               authMethod;
#endif /* TM_USE_PPP_MSCHAP */
#ifdef TM_DSP
    tt8Bit               dataChar;
    unsigned int         i;
    unsigned int         dataOffset;
#endif /* TM_DSP */
    int                  errorCode;

    errorCode = TM_ENOERROR;
    chapMsgPtr = (ttCharPtr)0;
    chapInfoPtr = &pppVectPtr->pppsChapState.chapsStateInfo;
    cpHdrPtr = (ttPppCpPacketTypePtr) packetPtr->pktLinkDataPtr;
    cpHdrPtr->cpCode = chapCode;
    if (chapCode == (tt8Bit)TM_CHAP_SUCCESS)
    {
        chapMsgPtr = tm_get_raw_buffer(TM_PPP_CHAP_CONST_MESSAGE_SIZE);
        if (chapMsgPtr == (ttCharPtr)0)
        {
            errorCode = TM_ENOBUFS;
            goto CHAPSEND_RETURN;
        }
        else
        {
            chapMsgLength = (tt8Bit) tm_strlen(tlChapSuccessMsg);
            tm_bcopy(tlChapSuccessMsg, chapMsgPtr, chapMsgLength);
            chapMsgPtr[chapMsgLength] = '\0';
        }
    }
    else
    {
        --chapInfoPtr->cpsFailures;

#ifdef TM_USE_PPP_MSCHAP
        authMethod = pppVectPtr->pppsLcpState.lcpsLocalGotOptions.lcpAuthMethod;
        if (authMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1)
        {
/* our MS-CHAPv1 failure packet will be at most 32 octets
 * "E=eee R=r C=cccccccccccccccc V=v"
 */
            chapMsgPtr = tm_get_raw_buffer(32);
            if (chapMsgPtr == (ttCharPtr)0)
            {
                errorCode = TM_ENOBUFS;
                goto CHAPSEND_RETURN;
            }
            tfMChapV1SetFailurePacket(
                         pppVectPtr,
                         chapMsgPtr,
                         &chapMsgLength,
                         chapInfoPtr->cpsFailures,
                         TM_PPP_MSCHAP_ERROR_AUTHFAILURE);
        }
        else
#endif /* TM_USE_PPP_MSCHAP */
        {
            chapMsgPtr = tm_get_raw_buffer(TM_PPP_CHAP_CONST_MESSAGE_SIZE);
            if (chapMsgPtr == (ttCharPtr)0)
            {
                errorCode = TM_ENOBUFS;
                goto CHAPSEND_RETURN;
            }
            else
            {
                chapMsgLength = (tt8Bit) tm_strlen(tlChapFailedMsg);
                tm_bcopy(tlChapFailedMsg, chapMsgPtr, chapMsgLength);
                chapMsgPtr[chapMsgLength] = '\0';
            }
        }
    }

    dataPtr = ((tt8BitPtr)cpHdrPtr) + TM_PAK_PPP_CP_SIZE;


#ifdef TM_DSP
    dataOffset = 0;
    for(i=0; i<chapMsgLength; i++)
    {
        dataChar = *chapMsgPtr++;
        tm_ppp_put_char_next(dataPtr, dataOffset, dataChar);
    }
#else /* !TM_DSP */
    tm_ppp_byte_copy(chapMsgPtr,0,dataPtr,0,chapMsgLength);
#endif /* TM_DSP */

    temp16 = (tt16Bit)(TM_PPP_CP_BYTES + chapMsgLength);
    packetPtr->pktLinkDataLength = (ttPktLen)(temp16);
    tm_htons(temp16, cpHdrPtr->cpLength);

    if(chapMsgPtr)
    {
        tm_free_raw_buffer((ttRawBufferPtr)chapMsgPtr);
    }
    (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);

CHAPSEND_RETURN:
    return errorCode;
}

#ifdef TM_USE_PPP_MSCHAP

/* Used to register a function to return new password, and new
 * password length
 */
int tfMsChapRegisterNewPassword(
                  ttUserInterface              interfaceId,
                  ttMsChapAuthenticateFunctPtr funcPtr )
{
    ttPppAuthFunc authFuncUnion;

    authFuncUnion.pppaMsChapFuncPtr = funcPtr;
    return tfPppRegisterAuthenticate(interfaceId, authFuncUnion,
                                     TM_PPP_CHAP_HOST_PROTOCOL+2);
}

static int tfMChapV1SendChangePasswordPacket(
                           ttPppVectPtr    pppVectPtr,
                           ttPacketPtr     packetPtr,
                           tt8BitPtr       newChallengePtr)
{
    ttChapOptionsPtr     chapInfoPtr;
    ttMsChapAuthenticateFunctPtr
                         getNewPasswordFuncPtr;
    ttPppCpPacketTypePtr cpHdrPtr;
    tt8BitPtr            dataPtr;
    tt8BitPtr            newPasswordPtr;
    tt8BitPtr            oldPasswordPtr;
    int                  newPasswordLen;
    int                  oldPasswordLen;
#ifdef TM_DSP
    int                  offset;
#endif /* TM_DSP */

    tm_zero_dsp_offset(offset);
    chapInfoPtr = &pppVectPtr->pppsChapState.
                    chapsLocalOurInfo;
    getNewPasswordFuncPtr = chapInfoPtr->chapMsNewPasswordFuncPtr;
    cpHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    cpHdrPtr->cpCode = TM_CHAP_CHANGEPASSWORD;
    cpHdrPtr->cpIdentifier ++;
    tm_htons(TM_MSCHAP_CHANGEPASSWORD_SIZE, cpHdrPtr->cpLength);
    packetPtr->pktLinkDataLength = TM_MSCHAP_CHANGEPASSWORD_SIZE;
    packetPtr->pktChainDataLength = TM_MSCHAP_CHANGEPASSWORD_SIZE;
    dataPtr = packetPtr->pktLinkDataPtr;

    tm_ppp_inc_ptr_n(dataPtr, offset, TM_PPP_CP_BYTES);
/* dataPtr points to the beginning of 'password encrypted with old NT hash',
 * 516 octets. build this field. We may just pass the linkdataPtr to the building
 * procedure because it is four-byte aligned.
 */
    newPasswordPtr = (tt8BitPtr)getNewPasswordFuncPtr(
                       (ttCharPtr)chapInfoPtr->chapUserNamePtr,
                       &newPasswordLen);
    oldPasswordPtr = chapInfoPtr->chapMsSecretPtr;
    oldPasswordLen = chapInfoPtr->chapMsSecretLength;

    tfMChapV1NewPwEncwithOldNtPwhash(
                              newPasswordPtr,
                              newPasswordLen,
                              oldPasswordPtr,
                              oldPasswordLen,
                              dataPtr);

    tm_ppp_inc_ptr_n(dataPtr, offset, 516);
/* dataPtr points to the beginning of 'old NT Hash encrypted with new NT hash'
 * 16 octets , build this field
 */
    tfMChapOldNtPwhashEncNewNtPwhash(
                              newPasswordPtr,
                              newPasswordLen,
                              oldPasswordPtr,
                              oldPasswordLen,
                              dataPtr);

    tm_ppp_inc_ptr_n(dataPtr, offset, 572);
/* dataPtr now points to "Windows NT compatible challenge response "
 * 24 octets. Build this field  */
    tfMChapV1NtChallengeResponse(newChallengePtr,
                                 newPasswordPtr,
                                 newPasswordLen,
                                 dataPtr);
    tm_ppp_inc_ptr_n(dataPtr, offset, 24);
/* dataPtr now points to the flags,  */
    tm_ppp_put_char_next(dataPtr, offset, 0);
/* Set the lowest signaficant bit to 1, NT compatible challenge response */
    tm_ppp_put_char_next(dataPtr, offset, 0x01);

    (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);

/* we don't use our old password any more because it is already expired */
    chapInfoPtr->chapMsSecretLength = (tt16Bit)newPasswordLen;
    chapInfoPtr->chapMsSecretPtr = newPasswordPtr;

    return TM_ENOERROR;
}

/* DESCRIPTION:
 * To set the failure packet into chapMsgPtr. And set the chapMsgLength.
 * If retryCount > 0, set R=1, otherwise, set R = 0;
 * Failure type could be 691 and 648
 *
 * PARAMETER:
 * chapMsgPtr :       Where to store the failure message
 * chapMsgLenPtr:     returns the failure message length
 * retryCount:        how many times we allow the peer to try
 * failureType:       decimail error code, we support only to three digits
 * The message format is:
 * "E=eee R=r C=cccccccccccccccc V=v"
 */

static void tfMChapV1SetFailurePacket(
                              ttPppVectPtr  pppVectPtr,
                              ttCharPtr     chapMsgPtr,
                              tt8BitPtr     chapMsgLenPtr,
                              tt8Bit        retryCount,
                              int           failureType)
{
    ttCharPtr    dataPtr;
    int          i;
    int          j;
    tt8Bit       length;
    tt32Bit      random;
    char         buffer[17];
    tt8BitPtr    valuePtr;
#ifdef TM_DSP
    int          offset;
    int          valueOffset;
#endif /* TM_DSP */

    tm_zero_dsp_offset(offset);
    tm_zero_dsp_offset(valueOffset);
    retryCount = (tt8Bit)((retryCount > 0)? 1: 0);
    tm_ppp_put_char_next(chapMsgPtr, offset, 'E');
    tm_ppp_put_char_next(chapMsgPtr, offset, '=');
    tm_sprintf(buffer, "%3d", failureType);
    tm_ppp_byte_copy(buffer, 0, chapMsgPtr, offset, 3);
    tm_ppp_inc_ptr_n(chapMsgPtr, offset, 3);

    tm_ppp_put_char_next(chapMsgPtr, offset, ' ');
    tm_ppp_put_char_next(chapMsgPtr, offset, 'R');
    tm_ppp_put_char_next(chapMsgPtr, offset, '=');
    tm_ppp_put_char_next(chapMsgPtr, offset, (retryCount+0x30));
    length = 9;
    if(retryCount)
    {
        tm_ppp_put_char_next(chapMsgPtr, offset, ' ');
        tm_ppp_put_char_next(chapMsgPtr, offset, 'C');
        tm_ppp_put_char_next(chapMsgPtr, offset, '=');
        dataPtr = buffer;
        valuePtr = pppVectPtr->pppsChapState.chapsRemoteAllowInfo.chapValue;
        for(i=0; i<2; i ++)
        {
            random = tfGetRandom();
/* here we treat it as big-endian */
            tm_ppp_byte_copy(&random, 0, valuePtr, 0, 4);
            tm_ppp_inc_ptr_n(valuePtr, valueOffset, 4);
/* we need to treat it as big-endian */
            random = htonl(random);
            tm_sprintf(dataPtr, "%8x", random);
            for(j = 0; j < 8; j ++)
            {
                if(tm_ppp_get_char_index(dataPtr, j) == ' ')
                {
                    tm_ppp_put_char_index(dataPtr, j, '0');
                }
            }
            tm_ppp_inc_ptr_n(dataPtr, valueOffset, 8);
        }
        tm_ppp_byte_copy(buffer, 0, chapMsgPtr, offset, 16);
        tm_ppp_inc_ptr_n(chapMsgPtr, offset, 16);
        tm_ppp_put_char_next(chapMsgPtr, offset, ' ');
        tm_ppp_put_char_next(chapMsgPtr, offset, 'V');
        tm_ppp_put_char_next(chapMsgPtr, offset, '=');
        tm_ppp_put_char_next(chapMsgPtr, offset, '2');
        length += 23;
    }
    *chapMsgLenPtr = length;

    return;
}


/* if R=1 and E=648, we send change_password Packet.
 * If R=0, we set event to be CLOSE
 * Process paked Packet format.
 * Don't use strchr, because strchr hasn't been ported to TI's DSP yet.
 "E=xxxxxx R=r C=cccccccccccccccc V=vvvvvvv"
 */
static void tfMChapV1ProcessFailurePacket(
                                ttPppVectPtr     pppVectPtr,
                                ttPacketPtr      packetPtr,
                                tt16Bit          optionsLength,
                                tt8BitPtr        retryPtr)
{
    tt8BitPtr        dataPtr;
    ttPacketPtr      newPacketPtr;
    tt8BitPtr        newChallengePtr;
    ttChapOptionsPtr chapInfoPtr;
    char             formatChar;
    int              beginCopyFlag;
    int              newChallengeExists;
    int              i;
    int              j;
    char             temp[17];
    char             ch;
    char             v;
    char             r;
    int              error;
#ifdef TM_DSP
    int              offset;
#endif /* TM_DSP */

    tm_zero_dsp_offset(offset);
    dataPtr = packetPtr->pktLinkDataPtr;
    tm_ppp_inc_ptr_n(dataPtr, offset, TM_PPP_CP_BYTES) ;
/* the first format Char */
    formatChar = tm_ppp_get_char_offset(dataPtr, 0); /* should be 'E' */
    beginCopyFlag = 0;
    error = 0;
/* assume there is no new challenge value. If there is, it will eventually
 * be saved in temp[17];
 */
    newChallengeExists = 0;
    v = '1';
    r = '0';
    j = 0;
    for(i = 1; i < (int)(optionsLength - TM_PPP_CP_BYTES); i ++)
    {
        ch = tm_ppp_get_char_index(dataPtr, i);
        if(ch == '=')
        {
            beginCopyFlag = 1;
            continue;
        }
        else if(ch == ' ')
        {
            beginCopyFlag = 0;
            temp[j] = '\0';
            if(formatChar == 'E')
            {
                error = (int) tm_strtol(temp, (ttCharPtrPtr)0, 10);
            }
            j = 0;
            continue;
        }
        else if(ch == 'R' || ch == 'V')
        {
            formatChar = ch;
            continue;
        }
        else if(formatChar == 'C' && ch == 'C')
        {
            newChallengeExists = 1;
            continue;
        }

        if( (beginCopyFlag == 1)  &&
            (formatChar != 'V')   &&
            (formatChar != 'R'))
        {
            temp[j++] = ch;
        }
        else if(formatChar == 'V')
        {
            v = ch;
        }
        else if(formatChar == 'R')
        {
            r = ch;
/* prepare to copy C and if there is no C, formatChar will be overwritten by 'V' */
            formatChar = 'C';
        }
    }

/* assume we are going to try more times */
    *retryPtr = 1;
    chapInfoPtr = &pppVectPtr->pppsChapState.chapsLocalOurInfo;
    newChallengePtr = chapInfoPtr->chapValue;

    if(newChallengeExists)
    {
/* needs convert hexdecimal string to hexdecimal array.
 */
        tfHexByteStr2HexByteArr(temp,
                                TM_CHAP_MSV1_CHALLENGE_SIZE,
                                (ttCharPtr)chapInfoPtr->chapValue);
    }
    else /* the old challenge plus 23 */
    {
        char     firstChar;
        firstChar = tm_ppp_get_char_index(newChallengePtr, 0);
        firstChar = (char)(firstChar + 23);
        tm_ppp_put_char_index(newChallengePtr, 0, firstChar);
    }

    if( error == TM_PPP_MSCHAP_ERROR_AUTHFAILURE && r != '0')
    {
/* send new response packet*/
/* since we call tfChapChallenge, we should copy the new Challenge to the packet*/
        dataPtr = packetPtr->pktLinkDataPtr;
        tm_zero_dsp_offset(offset);
        tm_ppp_inc_ptr_n(dataPtr, offset, TM_PPP_CP_BYTES);
        tm_ppp_put_char_next(dataPtr, offset, TM_CHAP_MSV1_CHALLENGE_SIZE);
        tm_ppp_byte_copy(newChallengePtr,
                         0,
                         dataPtr,
                         offset,
                         TM_CHAP_MSV1_CHALLENGE_SIZE);

        tfChapChallenge(pppVectPtr,
                        packetPtr,
                        0);
    }
    else if(error == TM_PPP_MSCHAP_ERROR_PASSWDEXPIRE && (v >= '2'))
    {
/* we send ChangePasswordPacket, it is a big packet, we most likely needs
 * a brand new packet buffer.
 */
        newPacketPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                          TM_MSCHAP_CHANGEPASSWORD_SIZE,
                                          TM_16BIT_ZERO );
        if (newPacketPtr != TM_PACKET_NULL_PTR)
        {
/* copy the identifier too */
/*
 * Note for PPPoE this will not copy the whole header, but copying the PPP
 * protocol field is enough because PPPoE will add the header anyway.
 */
            tm_bcopy(packetPtr->pktLinkDataPtr - TM_PAK_PPP_HEADER_SIZE,
                newPacketPtr->pktLinkDataPtr - TM_PAK_PPP_HEADER_SIZE,
                TM_PPP_HDR_BYTES + TM_PPP_CP_BYTES);
            tfFreePacket(packetPtr, (tt8Bit)0);
            packetPtr = newPacketPtr;
            (void)tfMChapV1SendChangePasswordPacket(pppVectPtr,
                packetPtr,
                newChallengePtr);
        }
    }
    else
    {
        *retryPtr = 0;
    }

    return;
}
#endif /* TM_USE_PPP_MSCHAP */


#ifdef TM_USE_EAP
/*
 * tfPppEapAuthNotify Function Description
 * Called from EAP (via function pointer) to notify PPP that authentication
 * has completed, successfully or unsuccessfully.
 *
 * Internals
 * 1. If authentication failed, in either direction, close the link.
 * 2. If authentication was successful:
 *     2.1. Indicate that authentication was successful for the specified side
 *          of the link (either local or remote).
 *     2.2. Depending on the LCP authentication type negotiation, determine if
 *          authentication is complete.  That is, if EAP was negotiated for
 *          both the local and remote side, both sides must have complete
 *          authentication.  If so, start the NCP layer by call tfPppNextLayer.
 *     2.3. Notify the user that the authentication layer is up with the
 *          TM_LL_EAP_UP message.
 *
 * Parameters
 * Parameter    Description
 * devPtr       Device on which authentication has completed.
 * authResult   Result of the authentication:
 *              TM_EAP_AUTH_SUCCESS or TM_EAP_AUTH_FAILURE.
 * isAuthLocal  Specifies which end of the connection authentication has
 *              completed on: TM_EAP_LOCAL or TM_EAP_REMOTE.
 *
 * Returns
 * None
 */
static void tfPppEapAuthNotify(ttDeviceEntryPtr devPtr,
                               int              authResult,
                               int              isAuthLocal,
                               tt8Bit           needLockFlag)
{
    ttPppVectPtr pppVectPtr;

#ifdef TM_LOCK_NEEDED
    if (needLockFlag == TM_8BIT_YES)
    {
/* BUG ID 1300, most likely, the devPtr has already been locked
 * by tfPppAsyncRecv, in which case, we don't need to lock again
 */
        tm_call_lock_wait(&(devPtr->devLockEntry));
    }
#else /* !TM_LOCK_NEEDED*/
    TM_UNREF_IN_ARG(needLockFlag);
#endif /* !TM_LOCK_NEEDED */

    pppVectPtr = (ttPppVectPtr)(devPtr->devStateVectorPtr);

/* 1. If authentication failed, in either direction, close the link. */
    if (authResult == TM_EAP_AUTH_FAILURE)
    {
        (void)tfPppStateMachine(pppVectPtr,
                                TM_PACKET_NULL_PTR,
                                TM_PPPE_CLOSE,
                                TM_PPP_LCP_INDEX);
    }
    else
    {

/* 2. If authentication was successful: */

/*
 *     2.1. Indicate that authentication was successful for the specified side
 *          of the link (either local or remote).
 */
        if (isAuthLocal == TM_EAP_LOCAL)
        {
            pppVectPtr->pppsEapLocalResult = 1;
        }
        else
        {
            pppVectPtr->pppsEapRemoteResult = 1;
        }

/*
 *     2.2. Depending on the LCP authentication type negotiation, determine if
 *          authentication is complete.  That is, if EAP was negotiated for
 *          both the local and remote side, both sides must have complete
 *          authentication.  If so, start the NCP layer by call tfPppNextLayer.
 */
        tfPppNextLayer(pppVectPtr, TM_PPP_EAP_INDEX);

/*
 *     2.3. Notify the user that the authentication layer is up with the
 *          TM_LL_EAP_UP message.
 */
        tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_EAP_UP);
    }

#ifdef TM_LOCK_NEEDED
    if(needLockFlag == TM_8BIT_YES)
    {
        tm_call_unlock(&(devPtr->devLockEntry));
    }
#endif /* TM_LOCK_NEEDED */
}
#endif /* TM_USE_EAP */

/*
 *  This layer down for IPCP, CHAP, or PAP. Do nothing.
 */
int tfPppThisLayerDown(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(pppVectPtr);
    return TM_ENOERROR;
}

int tfAuthThisLayerDown(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
/* Since Pap/Chap were not really done, or are brought down, re-initialize IPCP */
#ifdef TM_USE_IPV4
    tfIpcpReInit(pppVectPtr);
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tfIp6cpReInit(pppVectPtr);
#endif /* TM_USE_IPV6 */
    return TM_ENOERROR;
}


/*
 * Allocate a PPP state vector
 */
ttPppVectPtr tfPppVectAlloc (ttDeviceEntryPtr devPtr)
{
    ttPppVectPtr     pppVectPtr;
#ifdef TM_USE_EAP
    int              errorCode;
#endif /* TM_USE_EAP */

    pppVectPtr = (ttPppVectPtr)tm_get_raw_buffer((ttPktLen)sizeof(ttPppVect));
    if (pppVectPtr != TM_PPP_VECT_NULL_PTR)
    {
        tm_bzero(pppVectPtr, sizeof(ttPppVect));
/* Save the state vector so that we can configure it */
        pppVectPtr->pppsDeviceEntryPtr = devPtr;
        devPtr->devStateVectorPtr = pppVectPtr;

/*
 * Initialize send buffer size to default (send data to driver
 * TM_PPP_DEF_SEND_SIZE bytes at a time). This needs to be in tfPppVectAlloc
 * because it must be performed only once.
 */
        pppVectPtr->pppsSendBufPtr = &(pppVectPtr->pppsSendBuf[0]);
#if (TM_PPP_DEF_SEND_SIZE > TM_PPP_DEF_BUF_SIZE)
/* Do not let the user make the send size bigger than the buffer size */
        pppVectPtr->pppsSendBufSize = TM_PPP_DEF_BUF_SIZE;
#else /* !(TM_PPP_DEF_SEND_SIZE > TM_PPP_DEF_BUF_SIZE) */
        pppVectPtr->pppsSendBufSize = TM_PPP_DEF_SEND_SIZE;
#endif /* !(TM_PPP_DEF_SEND_SIZE > TM_PPP_DEF_BUF_SIZE) */
        pppVectPtr->pppsUserSendBufSize = pppVectPtr->pppsSendBufSize;

#ifdef TM_USE_IPV6
/*
 * Get the interface identifiers to be used for IPV6CP negotiation (one for
 * the local end of the link, the other for the peer).
 * It is necessary to do this at initialize time (rather then during
 * negotiation or when opening the interfaces) so that the local identifier
 * remains consistent across opens & closes of IPV6CP.
 * ([RFC2472].R4.1:80, [RFC2472].R4.1:50)
 */
        if ( tm_16bit_one_bit_set(devPtr->dev6Flags,
                                  TM_6_DEV_INTERFACE_ID_FLAG) )
        {
/* If the user has set an EUI-64 identifier, use that one. */
            pppVectPtr->ppps6RandomLocalId[0] = devPtr->dev6InterfaceId[0];
            pppVectPtr->ppps6RandomLocalId[1] = devPtr->dev6InterfaceId[1];
        }
        else
        {
/* There was no identifier set by the user, so generate one randomly. */
            pppVectPtr->ppps6RandomLocalId[0] = tfGetRandom();
            pppVectPtr->ppps6RandomLocalId[1] = tfGetRandom();
/*
 * Clear the "u" bit from the local identifier, indicating that it is not unique
 * ([RFC2472].R4.1:90)
 */
            pppVectPtr->ppps6RandomLocalId[0] &= ~0x02000000UL;

        }

/*
 * Generate a random identifier for the peer.  Basing the low order 32-bits
 * of the remote identifier on the local identifier guarantees that they will
 * never match.
 */
        pppVectPtr->ppps6RandomPeerId[0]  = tfGetRandom();
        pppVectPtr->ppps6RandomPeerId[1]  = ~pppVectPtr->ppps6RandomLocalId[1];

/*
 * Clear the "u" bit from the remote identifier, indicating that it is not
 * unique.  ([RFC2472].R4.1:90)
 */
        pppVectPtr->ppps6RandomPeerId[0]  &= ~0x02000000UL;


#endif /* TM_USE_IPV6 */

        tfPppDefaultInit(pppVectPtr);

/* Initialize open failure timeout & timer */
        pppVectPtr->pppsOpenFailTimeout = TM_PPP_DEF_OPEN_FAIL_TIMEOUT;
        pppVectPtr->pppsOpenFailTimerPtr = TM_TMR_NULL_PTR;

#ifdef TM_USE_EAP
/* Call tfEapInit to allocate a new EAP structure.  Attach this to the PPP
   state vector. */
        errorCode = tfEapInit(&pppVectPtr->pppsEapVectPtr,
                              pppVectPtr->pppsDeviceEntryPtr,
                              tfPppEapAuthNotify);

/* There was a problem initializing EAP.  Free up the PPP state vector and
   return an error. */
        if (errorCode != TM_ENOERROR)
        {
            tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)pppVectPtr);
            pppVectPtr = (ttPppVectPtr)0;
        }
#endif /* TM_USE_EAP */
    }
    return pppVectPtr;
}


/*
 * Copy a user name or user password passed by the user for CHAP or PAP
 * into a buffer pointed to by the state vector.
 */
static int tfPppCopyString ( const char TM_FAR    * optionValuePtr,
                             tt8BitPtr  TM_FAR    * namePtrPtr,
                             int                    optionLength)
{
    tt8BitPtr   namePtr;
    int         errorCode;
#ifdef TM_DSP
    unsigned int index;
#endif /* TM_DSP */

    tm_zero_dsp_offset(index);
    namePtr = *namePtrPtr;
    if (namePtr != (tt8BitPtr)0)
    {
        tm_free_raw_buffer((ttRawBufferPtr)namePtr);
        *namePtrPtr = (tt8BitPtr)0;
    }
    namePtr = (tt8BitPtr)tm_get_raw_buffer((ttPktLen)(optionLength + 1));
#ifdef TM_DSP
    if (namePtr != (tt8BitPtr)0)
    {
        errorCode = TM_ENOERROR;
        *namePtrPtr = namePtr;
        while (optionLength--)
        {
            tm_ppp_put_char_next(namePtr,
                                 index,
                                 *optionValuePtr++);
        }
        tm_ppp_put_char_index(namePtr,index,0);
    }
#else /* !TM_DSP */
    if (namePtr != (tt8BitPtr)0)
    {
        errorCode = TM_ENOERROR;
        *namePtrPtr = namePtr;
        if (optionLength != 0)
        {
            tm_bcopy( optionValuePtr,
                      namePtr,
                      optionLength);
        }
        tm_ppp_put_char_index(namePtr,optionLength,0);
    }
#endif /* TM_DSP */
    else
    {
        errorCode = TM_ENOBUFS;
    }
    return errorCode;
}

/*
 * If any PAP or CHAP user name or password buffers, or
 * any send or recv compressed TCP/IP headers, or CHAP response timer or
 * packet have been allocated, free them, and re-initialize the
 * corresponding PPP state vector pointers.
 */
static void tfPppFreeBuffersTimers (ttPppVectPtr pppVectPtr)
{
#ifdef TM_USE_IPV4
    ttVjCompDataPtr  vjCompDataPtr;
    tt8Bit           currentSlot;
    tt8Bit           numSlots;

    vjCompDataPtr = pppVectPtr->pppsVjSendDataPtr;
    if (vjCompDataPtr != (ttVjCompDataPtr)0)
    {
/*
 * The list of VJ structures may have been rearranged to keep it in
 * 'most recently used' order, so pppsVjSendDataPtr may not point
 * at the beginning of the buffer that was allocated.  Scan through
 * the list for the structure with connection ID of 0, which is
 * the start of the list.
 */
        numSlots = (tt8Bit) (pppVectPtr->pppsIpcpState.
                             ipcpsRemoteSetOptions.ipcpVjMaxSlots + (tt8Bit)1);

        currentSlot = 0;
        while (currentSlot < numSlots)
        {
            if (vjCompDataPtr->vjsConnId == 0)
            {
                tm_free_raw_buffer((ttRawBufferPtr)vjCompDataPtr);
                vjCompDataPtr = (ttVjCompDataPtr)0;
                break;
            }

            vjCompDataPtr = vjCompDataPtr->vjsNextPtr;
            currentSlot++;
        }

#ifdef TM_ERROR_CHECKING
        if (vjCompDataPtr != (ttVjCompDataPtr) 0)
        {
            tfKernelError("tfPppFreeBuffersTimers",
                          "Corrupted VJ structure list");
        }
#endif /* TM_ERROR_CHECKING */
    }
/*
 * All entries have been freed; clear the pointer so new VJ structures will be
 * allocated the next time the link is opened.
 */
    pppVectPtr->pppsVjSendDataPtr = (ttVjCompDataPtr)0;


    vjCompDataPtr = pppVectPtr->pppsVjRecvDataPtr;
    if (vjCompDataPtr != (ttVjCompDataPtr)0)
    {
        tm_free_raw_buffer((ttRawBufferPtr)vjCompDataPtr);
        pppVectPtr->pppsVjRecvDataPtr = (ttVjCompDataPtr)0;
    }
#endif /* TM_USE_IPV4 */

    if (pppVectPtr->pppsPacketPtr != TM_PACKET_NULL_PTR)
    {
        tfFreePacket(pppVectPtr->pppsPacketPtr, TM_SOCKET_UNLOCKED);
        pppVectPtr->pppsPacketPtr = TM_PACKET_NULL_PTR;
    }
    pppVectPtr->pppsEndFrame = TM_8BIT_NO;
    pppVectPtr->pppsClientState = (tt8Bit)0; /* Reset */
    tfPppFreeTimers(pppVectPtr);
}

static void tfPppFreeTimers (ttPppVectPtr pppVectPtr)
{
/* Remove any timer */
    tfPppTimerRemove(&pppVectPtr->pppsLcpState.lcpsStateInfo);
#ifdef TM_USE_IPV4
    tfPppTimerRemove(&pppVectPtr->pppsIpcpState.ipcpsStateInfo);
#endif /* TM_USE_IPV4 */
    tfPppTimerRemove(&pppVectPtr->pppsPapState.papsStateInfo);
    tfPppTimerRemove(&pppVectPtr->pppsChapState.chapsStateInfo);
    tfChapsResponseCleanup(&pppVectPtr->pppsChapState);
    if (pppVectPtr->pppsPapState.papsRemoteReqTimerPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(pppVectPtr->pppsPapState.papsRemoteReqTimerPtr);
        pppVectPtr->pppsPapState.papsRemoteReqTimerPtr = TM_TMR_NULL_PTR;
    }
}

/*
 * Init all layers to their default values.
 */
static void tfPppDefaultInit (ttPppVectPtr pppVectPtr)
{
    pppVectPtr->pppsHdrDecompressSpace = 0;

/* Use default values for LCP */
    tfLcpDefaultInit(pppVectPtr);
/* Use default values for CHAP */
    tfChapDefaultInit(pppVectPtr);
/* Use default values for PAP */
    tfPapDefaultInit(pppVectPtr);

/* Use default values for IPCP */
#ifdef TM_USE_IPV4
    tfIpcpDefaultInit(pppVectPtr);
#endif /* TM_USE_IPV4 */

/* Use default values for IPV6CP */
#ifdef TM_USE_IPV6
    tfIp6cpDefaultInit(pppVectPtr);
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_PPP_CALLBACK
    tfCbcpDefaultInit(pppVectPtr);
#endif /* TM_USE_PPP_CALLBACK */
}

/*
 * Common timeout routine. Crank the state machine with a timeout event
 * if this is not the last retry, otherwise crank the state machine
 * with the last timeout event.
 */
static void tfPppTimeout( ttPppVectPtr pppVectPtr,
                          ttCpStatePtr stateInfoPtr,
                          tt8Bit       pppIndex )
{
#ifdef TM_LOCK_NEEDED
    ttDeviceEntryPtr devPtr;
#endif /* TM_LOCK_NEEDED */
    tt8Bit           pppEvent;

#ifdef TM_LOCK_NEEDED
    devPtr = pppVectPtr->pppsDeviceEntryPtr;
#endif /* TM_LOCK_NEEDED */
    tm_call_lock_wait(&(devPtr->devLockEntry));
    if (stateInfoPtr->cpsTmrPtr != TM_TMR_NULL_PTR)
/* If timer has not been removed */
    {
/* Non auto timer, mark it as removed */
        stateInfoPtr->cpsTmrPtr = TM_TMR_NULL_PTR;
/* Crank the state machine to let it know we got a timeout */
        if (stateInfoPtr->cpsRetryCounter > 0)
        {
            pppEvent = (tt8Bit)TM_PPPE_TIMEOUT;
        }
        else
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfPppTimeout", "Max retries reached");
#endif /* TM_ERROR_CHECKING */
            pppEvent = (tt8Bit)TM_PPPE_LAST_TIMEOUT;
        }
        (void)tfPppStateMachine(pppVectPtr,
                                TM_PACKET_NULL_PTR,
                                pppEvent,
                                pppIndex);
    }
    tm_call_unlock(&(devPtr->devLockEntry));
}

/*
 * Remove and reset the timer for the layer corresponding to stateInfoPtr
 */
static void tfPppTimerRemove ( ttCpStatePtr stateInfoPtr )
{
    ttTimerPtr  timerPtr;

    timerPtr = stateInfoPtr->cpsTmrPtr;
    if (timerPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(timerPtr);
        stateInfoPtr->cpsTmrPtr = TM_TMR_NULL_PTR;
    }
}


/*
 * Add a timer that will call the timeoutFuncPtr
 */
static void tfPppTimerAdd ( ttPppVectPtr   pppVectPtr,
                            ttTmCBFuncPtr  timeoutFuncPtr,
                            ttCpStatePtr   stateInfoPtr )
{
    ttTimerPtr     timerPtr;
    ttGenericUnion timerParm1;

    if (stateInfoPtr->cpsTmrPtr == (ttTimerPtr)0)
    {
        timerParm1.genVoidParmPtr = (ttVoidPtr)pppVectPtr;
        timerPtr = tfTimerAdd( timeoutFuncPtr,
                               timerParm1,
                               timerParm1, /* unused */
                               (tt32Bit)
                                    (stateInfoPtr->cpsRetryTimeoutSeconds) *
                                    TM_UL(1000),
                               0);
        if (timerPtr != TM_TMR_NULL_PTR)
        {
            stateInfoPtr->cpsTmrPtr = timerPtr;
            --stateInfoPtr->cpsRetryCounter;
        }
    }
}

/*
 * Clean up CHAP response packet pointer and CHAP response timer pointer
 * (both pointers used by client when receiving a CHAP challenge from the
 *  server).
 */
static void tfChapsResponseCleanup (ttChapStatePtr chapStatePtr)
{
    ttPacketPtr packetPtr;
    ttTimerPtr  timerPtr;
/* Free our Chap response timer and chap extra packet */

    packetPtr = chapStatePtr->chapsResponsePacketPtr;
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        chapStatePtr->chapsResponsePacketPtr = TM_PACKET_NULL_PTR;
    }
    timerPtr = chapStatePtr->chapsResponseTimerPtr;
    if (timerPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(timerPtr);
        chapStatePtr->chapsResponseTimerPtr = TM_TMR_NULL_PTR;
    }
}

/*
 * Could not continue processing an incoming PPP packet because we did not
 * have any more buffer. So free the incoming buffer, and crank the
 * state machine with a close event
 */
static void tfPppNoBuffer(tt8BitPtr pppEventPtr, ttPacketPtr packetPtr)
{
    *pppEventPtr = TM_PPPE_CLOSE;
    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
}

/*
 * Common code to finish filling the response to an IPCP config request,
 * or an LCP config request, or a PAP request. We fill in the
 * configuration header based on our earlier parsing.
 *
 * outDataOffset is a byte offset value and is only
 * included when using a DSP platform.
 */
static ttPacketPtr tm_ppp_parse_finish ( tfPppParseFinish,
                                         ttPacketPtr  packetPtr,
                                         ttPacketPtr  rejectPacketPtr,
                                         ttPacketPtr  nakPacketPtr,
                                   /* outdataPtr is used to compute length */
                                         tt8BitPtr    outDataPtr,
                                         tt8BitPtr    pppEventPtr,
                                         tt8Bit       ident,
                                         unsigned int outDataOffset)
{
    ttPacketPtr          retPacketPtr;
    ttPppCpPacketTypePtr cpHdrPtr;
    tt16Bit              temp16;
    tt8Bit               pppCpCode;
    tt8Bit               pppEvent;

    if (rejectPacketPtr != TM_PACKET_NULL_PTR)
    {
/* A Reject packet. Sending back a reject. */
        retPacketPtr = rejectPacketPtr;
        pppCpCode = TM_PPP_CONFIG_REJECT;
    }
    else
    {
        if (nakPacketPtr != TM_PACKET_NULL_PTR)
        {
/* A Nak packet. Sending back a NAK. */
            retPacketPtr = nakPacketPtr;
            pppCpCode = TM_PPP_CONFIG_NAK;
        }
        else
        {
/* sending back an ACK in the original packet */
            retPacketPtr = packetPtr;
            pppCpCode = TM_PPP_CONFIG_ACK;
        }
    }
    if (pppCpCode == TM_PPP_CONFIG_NAK || pppCpCode == TM_PPP_CONFIG_REJECT)
    {
/*
 * common code for reject, or nak. Point to cp header. Fill in Identifier,
 * event is BAD config request, free original packet.
 */
        cpHdrPtr = (ttPppCpPacketTypePtr)(   retPacketPtr->pktLinkDataPtr
                                           - TM_PAK_PPP_CP_SIZE );
        retPacketPtr->pktLinkDataPtr = (tt8BitPtr)cpHdrPtr;
        cpHdrPtr->cpIdentifier = ident;
        pppEvent = TM_PPPE_BAD_CFG_REQ;
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
    else
    {
/* sending back an ACK in the original packet */
        cpHdrPtr = (ttPppCpPacketTypePtr)retPacketPtr->pktLinkDataPtr;
        pppEvent = TM_PPPE_GOOD_CFG_REQ;
    }
/* Common code for all cases. Fill in cpCode and PPP event */
    cpHdrPtr->cpCode = pppCpCode;
    *pppEventPtr = pppEvent;
    if (outDataPtr != (tt8BitPtr)0)
/*
 * Fill in the packet length, common code for reject, nak, or PAP
 * good packet
 */
    {
        temp16 = tm_byte_count((tt16Bit)(outDataPtr - (tt8BitPtr)cpHdrPtr));
        tm_inc_dsp_offset(temp16, outDataOffset);
        retPacketPtr->pktLinkDataLength = (ttPktLen)temp16;
        tm_htons(temp16, cpHdrPtr->cpLength);
    }
    return retPacketPtr;
}

/*
 * Called from the PPP layer itself to send a PPP protocol packet.
 * . Initialize ptkDeviceEntryPtr, pktFlags for PPP, dataLinkLayerProtocol
 * . unlock device
 * . call tfDeviceSend
 * . relock the device.
 */
int tfPppDeviceSend( ttPppVectPtr pppVectPtr,
                     ttPacketPtr  packetPtr,
                     tt16Bit      pppProtocol )
{
    ttDeviceEntryPtr devPtr;
    int              errorCode;

    devPtr = pppVectPtr->pppsDeviceEntryPtr;

    tm_assert_is_locked(tfPppDeviceSend, &(devPtr->devLockEntry));

    tm_call_unlock(&(devPtr->devLockEntry));

    packetPtr->pktChainDataLength = packetPtr->pktLinkDataLength;
    packetPtr->pktSharedDataPtr->dataLinkLayerProtocol = pppProtocol;
    packetPtr->pktDeviceEntryPtr = devPtr;
    packetPtr->pktFlags = 0;
    packetPtr->pktNetworkLayer = TM_NETWORK_LINK_LAYER;
    errorCode = tfDeviceSend(packetPtr);

    tm_call_lock_wait(&(devPtr->devLockEntry));

    return errorCode;
}

/*
 * . If llEvent is TM_LL_CLOSE_COMPLETE, close the device.
 * . Notify the user of a link layer event. Called by PPP, and also by LQM.
 */
void tfPppUserNotify( ttDeviceEntryPtr devPtr,
                      int              llEvent )
{
    ttLinkLayerEntryPtr devLinkLayerPtr;
#ifdef TM_PPP_LQM
    ttPppVectPtr        pppVectPtr;
    ttLcpStatePtr       pppLcpStatePtr;
#endif /* TM_PPP_LQM */

#ifdef TM_PPP_LQM
    tm_assert_is_locked(tfPppUserNotify, &(devPtr->devLockEntry));

    pppVectPtr = (ttPppVectPtr) devPtr->devStateVectorPtr;
    pppLcpStatePtr =  &pppVectPtr->pppsLcpState;
#endif /* TM_PPP_LQM */

    devLinkLayerPtr = devPtr->devLinkLayerProtocolPtr;
    if (   (llEvent == TM_LL_CLOSE_COMPLETE)
#ifdef TM_USE_PPPOE
/* Device closed in PPPoE */
        && (    devPtr->devLinkLayerProtocolPtr->lnkNotifyFuncPtr
             != tfPppoePppCB) /* direct user notification */
#endif /* TM_USE_PPPOE */
        )
    {
        tfDeviceClose(devPtr);
    }
    if (devLinkLayerPtr->lnkNotifyFuncPtr != TM_LL_NOTIFY_FUNC_NULL_PTR )
    {
#ifdef TM_USE_PPPOE
        if (devLinkLayerPtr->lnkLinkLayerProtocol != TM_LINK_LAYER_PPPOE)
#endif /* TM_USE_PPPOE */
        {
            tm_call_unlock(&(devPtr->devLockEntry));
        }
        (*(devPtr->devLinkLayerProtocolPtr->lnkNotifyFuncPtr))(
                                                devPtr, (unsigned int)llEvent);
#ifdef TM_USE_PPPOE
        if (devLinkLayerPtr->lnkLinkLayerProtocol != TM_LINK_LAYER_PPPOE)
#endif /* TM_USE_PPPOE */
        {
            tm_call_lock_wait(&(devPtr->devLockEntry));
        }
    }

#ifdef TM_PPP_LQM
    if (pppVectPtr->pppsLqmVectPtr)
    {
/* the user called tfUsePppLqm, so check if the link is being opened. */
        switch(llEvent)
        {
        case TM_LL_OPEN_COMPLETE:
/* enable LQM on the link when the link opens */
/* if the peer rejected our negotiation of LQM, then set the LQM local
   got value for LQR timer to the default. */
            if (!(pppLcpStatePtr->lcpsLocalNegotiateOption &
                  (1UL<<(tt8Bit) TM_LCP_QUALITY_PROTOCOL)))
            {
                pppLcpStatePtr->lcpsLocalGotOptions.
                    lqmReportingPeriod =
                    tlLcpDefaultOptions.lqmReportingPeriod;
            }
            tfLqmEnable(
                pppVectPtr,
                pppVectPtr->
                pppsLcpState.lcpsRemoteSetOptions.lqmReportingPeriod,
                pppVectPtr->
                pppsLcpState.lcpsLocalGotOptions.lqmReportingPeriod);
            break;

        default:
            break;
        }
    }
#endif /* TM_PPP_LQM */
}

/* LCP or IPCP init restart counter */
static void tfPppInitRestartCounter (ttCpStatePtr stateInfoPtr)
{
    tt8Bit retryCount;
    tt8Bit pppState;

/* Check if we are initializing the Terminate Retry */
    pppState = stateInfoPtr->cpsState;
    if ( ( pppState == TM_PPPS_CLOSING ) ||  ( pppState == TM_PPPS_STOPPING) )
    {
        retryCount = stateInfoPtr->cpsInitTrmRetryCnt;
    }
    else
    {
/* We are initializing the config retry value */
        retryCount = stateInfoPtr->cpsInitCfgRetryCnt;
    }
    stateInfoPtr->cpsRetryCounter = retryCount;
/* Initialize the configuration failure counter */
    stateInfoPtr->cpsFailures = stateInfoPtr->cpsInitMaxFailures;
}

static int tfPppSendCodeReject(ttPppVectPtr pppVectPtr,
                               ttPacketPtr  packetPtr,
                               ttCpStatePtr stateInfoPtr,
                               tt16Bit      pppProtocol)
{
    ttPppCpPacketTypePtr codeRejectPtr;
    ttPacketPtr          codeRejectPacketPtr;
    int                  errorCode;
    tt16Bit              oldPktLen;
    tt16Bit              peerMru;

    errorCode = TM_ENOERROR;

/* Save the length of this packet */
    oldPktLen = (tt16Bit)packetPtr->pktLinkDataLength;

/*
 * Trim protocol reject to peer's MRU
 * Code rejects could occur before LCP negotiation has completed and a MRU
 * has been received from the peer.  If LCP is not in the OPENED state, use
 * the default MRU.
*/
    if (pppVectPtr->pppsLcpState.lcpsStateInfo.cpsState == TM_PPPS_OPENED)
    {
        peerMru =
            pppVectPtr->pppsLcpState.lcpsRemoteSetOptions.lcpMaxReceiveUnit;
    }
    else
    {
#ifdef TM_USE_PPPOE
/* Get the MRU from the link layer */
        peerMru = pppVectPtr->pppsDeviceEntryPtr->
            devLinkLayerProtocolPtr->lnkMtu;
#else /* !TM_USE_PPPOE */
        peerMru = tlLcpDefaultOptions.lcpMaxReceiveUnit;
#endif /* !TM_USE_PPPOE */
    }

/* Save room for header */
    peerMru -= TM_PPP_CP_BYTES;

    if (oldPktLen > peerMru)
    {
        oldPktLen = peerMru;
    }

    codeRejectPacketPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                             oldPktLen + TM_PPP_CP_BYTES,
                                             TM_16BIT_ZERO );

    if (codeRejectPacketPtr != TM_PACKET_NULL_PTR)
    {
/* Allow for the PPP Header */
        codeRejectPtr = (ttPppCpPacketTypePtr)
                                        (codeRejectPacketPtr->pktLinkDataPtr);

/* Copy the old packet to the end of our new packet */
        tm_bcopy( packetPtr->pktLinkDataPtr,
                  ((tt8BitPtr)codeRejectPtr) + TM_PAK_PPP_CP_SIZE,
                  (unsigned)tm_packed_byte_count(oldPktLen));

/* Add header */
        oldPktLen += TM_PPP_CP_BYTES;

        codeRejectPacketPtr->pktLinkDataLength = (ttPktLen)oldPktLen;

/* Free the old packet */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
/* Update our fields */
        tm_htons(oldPktLen, codeRejectPtr->cpLength);
        codeRejectPtr->cpCode = TM_PPP_CODE_REJECT;
        codeRejectPtr->cpIdentifier = stateInfoPtr->cpsIdentifier++;
        (void) tfPppDeviceSend( pppVectPtr,
                                codeRejectPacketPtr,
                                pppProtocol);
    }
    else
    {
/* Free the old packet */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        errorCode = TM_ENOBUFS;
    }

    return errorCode;
}

/****************************************************************************
* FUNCTION: tfPppIsOpen
*
* PURPOSE:
*   This function is called by LQM to check if the PPP link is open.
*
* PARAMETERS:
*   pppVectPtr:
*       Pointer to the PPP state vector.
*
* RETURNS:
*   1:
*       PPP link is open.
*   0:
*       PPP link is not open.
*
* NOTES:
*
****************************************************************************/
int tfPppIsOpen(
    ttPppVectPtr    pppVectPtr)
{
    if (pppVectPtr == TM_PPP_VECT_NULL_PTR)
    {
        return 0; /* PPP hasn't been initialized */
    }

    if (pppVectPtr->pppsLcpState.lcpsStateInfo.cpsState != TM_PPPS_OPENED)
    {
        return 0; /* PPP link is not open */
    }

    return 1; /* PPP link is open */
}




/*
 * tfPppNextLayer
 *
 * This routine is called when a PPP layer is complete and is used to make the
 * transition (if any) to the next layer.  When LCP is complete, the next
 * phase is either authentication (local and/or remote) or IPCP.  When all
 * authentication is completed, the next phase is IPCP.
 *
 */
int tfPppNextLayer(ttPppVectPtr pppVectPtr, tt8Bit currentProto)
{
    ttPapStatePtr    papStatePtr;
    ttDeviceEntryPtr devPtr;
    ttGenericUnion   timerParm1;
    tt32Bit          timeout;
    int              errorCode;
    tt8Bit           localAuthMethod;
    tt8Bit           remoteAuthMethod;
    tt8Bit           localAuthState;
    tt8Bit           remoteAuthState;
    tt8Bit           localAuthIndex;
    tt8Bit           remoteAuthIndex;

    errorCode   = TM_ENOERROR;

    localAuthState = (tt8Bit)0;
    remoteAuthState = (tt8Bit)0;
    localAuthIndex  = 0;
    remoteAuthIndex = 0;
/*
 * Type of authentication (both local and remote).  If none, should be set
 * to zero.
 */
    localAuthMethod =
        pppVectPtr->pppsLcpState.lcpsRemoteSetOptions.lcpAuthMethod;
    remoteAuthMethod =
        pppVectPtr->pppsLcpState.lcpsLocalGotOptions.lcpAuthMethod;

/* Get the state of the local authentication protocol */
#ifdef TM_USE_EAP
    if (localAuthMethod == TM_PPP_AUTHMETHOD_EAP)
    {
        localAuthIndex = TM_PPP_EAP_INDEX;
        if (pppVectPtr->pppsEapLocalResult == TM_8BIT_YES)
        {
            localAuthState = TM_PPPS_OPENED;
        }
        else
        {
/* Doesn't matter which state, so long as it's not OPENED. */
            localAuthState = TM_PPPS_STARTING;
        }
    }
    else
#endif /* TM_USE_EAP */
    if (localAuthMethod == TM_PPP_AUTHMETHOD_CHAP
#ifdef TM_USE_PPP_MSCHAP
        || (localAuthMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1)
#endif /* TM_USE_PPP_MSCHAP */
        )
    {
        localAuthState = pppVectPtr->pppsChapState.chapsStateInfo.cpsState;
        localAuthIndex = TM_PPP_CHAP_INDEX;
    }
    else if(localAuthMethod == TM_PPP_AUTHMETHOD_PAP)
    {
/* TM_PPP_PAP_HOST_PROTOCOL */
        localAuthState = pppVectPtr->pppsPapState.papsStateInfo.cpsState;
        localAuthIndex = TM_PPP_PAP_INDEX;
    }

/* Get the state of the remote authentication protocol */
#ifdef TM_USE_EAP
    if (remoteAuthMethod == TM_PPP_AUTHMETHOD_EAP)
    {
        if (pppVectPtr->pppsEapRemoteResult == TM_8BIT_YES)
        {
            remoteAuthState = TM_PPPS_OPENED;
            remoteAuthIndex = TM_PPP_EAP_INDEX;
        }
        else
        {
/* Doesn't matter which state, so long as it's not OPENED. */
            remoteAuthState = TM_PPPS_STARTING;
        }
    }
    else
#endif /* TM_USE_EAP */
    if (remoteAuthMethod == TM_PPP_AUTHMETHOD_CHAP
#ifdef TM_USE_PPP_MSCHAP
        || (remoteAuthMethod == TM_PPP_AUTHMETHOD_MSCHAP_V1 )
#endif /* TM_USE_PPP_MSCHAP */
        )
    {
        remoteAuthState = pppVectPtr->pppsChapState.chapsStateInfo.cpsState;
        remoteAuthIndex = TM_PPP_CHAP_INDEX;
    }
    else if (remoteAuthMethod == TM_PPP_AUTHMETHOD_PAP)
    {
/* TM_PPP_PAP_HOST_PROTOCOL */
        remoteAuthState = pppVectPtr->pppsPapState.papsStateInfo.cpsState;
        remoteAuthIndex = TM_PPP_PAP_INDEX;
    }

    switch (currentProto)
    {

        case TM_PPP_LCP_INDEX:

/*
 * If local authentication is needed, start either the PAP or
 * CHAP layer now.
 */
            if (localAuthMethod != 0)
            {
#ifdef TM_USE_EAP
                if (localAuthMethod == TM_PPP_AUTHMETHOD_EAP)
                {
/*
 * Call tfEapOpen if the negotiated authentication type was EAP.  Before
 * calling, reset the EAP result fields in the PPP state vector
 */
                    pppVectPtr->pppsEapLocalResult = 0;
                    errorCode = tfEapOpen(pppVectPtr->pppsEapVectPtr,
                                          TM_EAP_PEER_FLAG);
                }
                else
#endif /* TM_USE_EAP */
                {
                    errorCode = tfPppStateMachine(pppVectPtr,
                                                  TM_PACKET_NULL_PTR,
                                                  TM_PPPE_UP,
                                                  localAuthIndex);

                    if (errorCode == TM_ENOERROR)
                    {
/* Now send the config request */
                        errorCode = tfPppStateMachine(pppVectPtr,
                                                      TM_PACKET_NULL_PTR,
                                                      TM_PPPE_OPEN,
                                                      localAuthIndex);
                    }
                }
            }

/*
 * If remote authentication is needed, start either the PAP or
 * CHAP layer now.
 */
            if (remoteAuthMethod != 0)
            {
#ifdef TM_USE_EAP
                if (remoteAuthMethod == TM_PPP_AUTHMETHOD_EAP)
                {
/*
 * Call tfEapOpen if the negotiated authentication type was EAP.  Before
 * calling, reset the EAP result fields in the PPP state vector
 */
                    pppVectPtr->pppsEapRemoteResult = 0;
                    errorCode = tfEapOpen(pppVectPtr->pppsEapVectPtr,
                                          TM_EAP_AUTH_FLAG);
                }
                else
#endif /* TM_USE_EAP */
                {
/*
 * If the remote peer is authenticating to us using PAP, we need to add a
 * timer that will close the connection if the peer doesn't send us any
 * Auth-Reqs, otherwise the negotiation could hang.  We use the same timeout
 * and retry values for this as we do for sending PAP requests.
 */
                    if (remoteAuthMethod == TM_PPP_AUTHMETHOD_PAP)
                    {
                        papStatePtr = &(pppVectPtr->pppsPapState);
                        timeout = papStatePtr->papsStateInfo.cpsInitCfgRetryCnt *
                            papStatePtr->papsStateInfo.cpsRetryTimeoutSeconds *
                            TM_UL(1000);

                        timerParm1.genVoidParmPtr = (ttVoidPtr)pppVectPtr;
                        papStatePtr->papsRemoteReqTimerPtr =
                            tfTimerAdd( tfPapRemoteReqTimeout,
                                        timerParm1,
                                        timerParm1, /* unused */
                                        timeout,
                                        0 );
                    }

                    errorCode = tfPppStateMachine(pppVectPtr,
                                                  TM_PACKET_NULL_PTR,
                                                  TM_PPPE_UP,
                                                  remoteAuthIndex);

                    if (errorCode == TM_ENOERROR)
                    {
/* Now send the config request */
                        errorCode = tfPppStateMachine(pppVectPtr,
                                                      TM_PACKET_NULL_PTR,
                                                      TM_PPPE_OPEN,
                                                      remoteAuthIndex);
                    }
                }
            }

/*
 * This case drops through because if no authentication has been negotiated
 * the end of LCP acts like the end of PAP/CHAP and so the next case will
 * move directly to the IPCP (NCP) layer.
 */
        case TM_PPP_EAP_INDEX:
        case TM_PPP_CHAP_INDEX:
        case TM_PPP_PAP_INDEX:


/*
 * Do not advance to the next layer if authorization is not fully complete.
 */
            if ( !( ( (localAuthMethod == 0) ||
                                    (localAuthState == TM_PPPS_OPENED) ) &&
                 ( (remoteAuthMethod == 0) ||
                                    (remoteAuthState == TM_PPPS_OPENED) ) ) )
            {
                break;
            }

#ifdef TM_USE_PPP_CALLBACK
/*
 * Authentication layers (if any) have completed successfully.
 * Move to the Callback Control Protocol layer, if it has been negotiated.
 */
            if ( (pppVectPtr->pppsLcpState.lcpsLocalGotOptions
                                    .lcpCallbackOpFlags
                                    & (TM_UL(1) << TM_CALLBACK_OP_CBCP))
                                    != TM_32BIT_ZERO )
            {
/* Callback Control Protocol has been negotiated. Signal it. */
                errorCode = tfCbcpStateMachine( pppVectPtr,
                                                TM_PACKET_NULL_PTR,
                                                TM_PPPE_UP );
                break;
            }
/*
 * This case drops through to IPCP (NCP) in case Callback Control Protocol
 * has not been negotiated.
 */
        case TM_PPP_CBCP_INDEX:
#endif /* TM_USE_PPP_CALLBACK */

/*
 * Authentication layers (if any) have completed successfully.
 * Callback Control Protocol layer (if any) has completed successfully.
 * Move to the IPCP and/or IPV6CP (NCP) layer.
 */
            devPtr = pppVectPtr->pppsDeviceEntryPtr;
#ifdef TM_USE_IPV4
/* Start IPCP */
            if (    (pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState
                                                     == TM_PPPS_INITIAL)
                 && (tm_16bit_one_bit_set(devPtr->devFlag,
                                          TM_DEV_CONNECTING)) )
            {
                errorCode = tfPppStateMachine(pppVectPtr,
                                              TM_PACKET_NULL_PTR,
                                              TM_PPPE_UP,
                                              TM_PPP_IPCP_INDEX);

                if (errorCode == TM_ENOERROR)
                {
                    errorCode = tfPppStateMachine(pppVectPtr,
                                                  TM_PACKET_NULL_PTR,
                                                  TM_PPPE_OPEN,
                                                  TM_PPP_IPCP_INDEX);
                }
            }
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
            if (    (pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState
                                                     == TM_PPPS_INITIAL)
                 && (tm_16bit_one_bit_set(devPtr->dev6Flags,
                                          TM_6_LL_CONNECTING)) )
            {
                errorCode = tfPppStateMachine(pppVectPtr,
                                              TM_PACKET_NULL_PTR,
                                              TM_PPPE_UP,
                                              TM_PPP_IPV6CP_INDEX);

                if (errorCode == TM_ENOERROR)
                {
                    errorCode = tfPppStateMachine(pppVectPtr,
                                                  TM_PACKET_NULL_PTR,
                                                  TM_PPPE_OPEN,
                                                  TM_PPP_IPV6CP_INDEX);
                }
            }
#endif /* TM_USE_IPV6 */

            break;

/* IPCP does not move onto another layer once complete, so do nothing */
        case TM_PPP_IPCP_INDEX:
            break;

        default:
            break;
    }

    return errorCode;

}

/*
 * tfPppNcpOpenFail
 * Function Description
 * Called when an NCP fails to open (such as IPCP or IPV6CP).  Depending on
 * which other NCP's could be used, either close the interface or wait for
 * user to start a different layer.  We can't unconditionally close the
 * interface if an NCP fails, because the user may want to open another
 * protocol (IPv4 or IPv6) without the link going down.
 *
 * 1. If the stack is built in dual-stack mode, check if IPCP or IPV6CP is in
 *    the 'opened' or 'opening' states.
 * 2. If so, exit without closing the interface since another network protocol
 *    is available on this link.
 * 3. If another NCP is not opened or in the process of opening, start the
 *    'open failed' timer.  When this timer expires, the link will be closed
 *    if the user hasn't already closed it.
 * 4. If the stack is build in either IPv4 or IPv6 only mode, close the
 *    interface, since there is only one possible NCP. (current
 *    behavior). Notify the user with a TM_LL_OPEN_FAILED message.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * pppProtocol  Type of protocol (IPCP, IPV6CP) that has failed.
 *
 * Returns
 * None
 */
ttVoid tfPppNcpOpenFail (ttPppVectPtr   pppVectPtr)
{

#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
    tt8Bit         ipcpState;
    tt8Bit         ip6cpState;
    ttGenericUnion timerParm1;

/*
 * 1. If the stack is built in dual-stack mode, check if IPCP or IPV6CP is in
 *    the 'opened' or 'opening' states.
 * 2. If so, exit without closing the interface since another network protocol
 *    is available on this link.
 */
    ipcpState = pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState;
    ip6cpState = pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState;
    if (   (ipcpState <= TM_PPPS_STOPPING)
        && (ip6cpState <= TM_PPPS_STOPPING))
    {
/*
 * 3. If another NCP is not opened or in the proceses of opening, start the
 *    'open failed' timer.  When this timer expires, the link will be closed
 *    if the user hasn't already closed it.
 */
        timerParm1.genVoidParmPtr = (ttVoidPtr)pppVectPtr;
        pppVectPtr->pppsOpenFailTimerPtr =
            tfTimerAdd(tfPppOpenFailTimeout,
                       timerParm1,
                       timerParm1, /* unused */
                       pppVectPtr->pppsOpenFailTimeout * TM_UL(1000),
                       0);
    }
#else /* !TM_USE_IPV6 || !TM_USE_IPV4 */
/*
 * 4. If the stack is build in either IPv4 or IPv6 only mode, close the
 *    interface, since there is only one possible NCP. (current
 *    behavior). Notify the user with a TM_LL_OPEN_FAILED message.
 */
    tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_OPEN_FAILED);
    tfPppClose((ttUserInterface)pppVectPtr->pppsDeviceEntryPtr);
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */

}

/*
 * tfPppOpenFailTimeout
 * Function Description
 * A timeout here indicates that an NCP (IPCP or IPV6CP) previously failed to
 * open.  After this failure, the user neither opened a new network protocol
 * nor manually closed the link, so we're going to force the link closed at
 * this point.
 *
 * 1. Verify that neither IPCP nor IPV6CP are already opened or are in the
 *    process of opening.
 * 2. Notify the user with a TM_LL_OPEN_FAILED message.
 * 3. Close the PPP link  (through tfPppClose).

 * Parameters
 * Parameter    Description
 * timerArg     Pointer to the current PPP state vector.
 *
 * Returns
 * None
 */
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
ttVoid tfPppOpenFailTimeout (ttVoidPtr      timerBlockPtr,
                             ttGenericUnion userParm1,
                             ttGenericUnion userParm2)
{
    tt8Bit           ipcpState;
    tt8Bit           ip6cpState;
    ttPppVectPtr     pppVectPtr;

    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    pppVectPtr  = (ttPppVectPtr)userParm1.genVoidParmPtr;

    tm_call_lock_wait(&(pppVectPtr->pppsDeviceEntryPtr->devLockEntry));
    if (pppVectPtr->pppsOpenFailTimerPtr != TM_TMR_NULL_PTR)
    {
        pppVectPtr->pppsOpenFailTimerPtr = TM_TMR_NULL_PTR;
/*
 * 1. Verify that neither IPCP nor IPV6CP are already opened or are in the
 *    process of opening.
 */
        ipcpState  = pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState;
        ip6cpState = pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState;

        if (   (ipcpState <= TM_PPPS_STOPPING)
            && (ip6cpState <= TM_PPPS_STOPPING))
        {
/* 2. Notify the user with a TM_LL_OPEN_FAILED message. */
            tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_OPEN_FAILED);

/* 3. Close the PPP link  (through tfPppClose). */
            tfPppClose((ttUserInterface)pppVectPtr->pppsDeviceEntryPtr);
        }
    }

    tm_call_unlock(&(pppVectPtr->pppsDeviceEntryPtr->devLockEntry));

}
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */

/*
 * JNS/9-2-99 BUGFIX 422
 *   Added the following 3 functions (tfIpcpSendTermAck, tfPapSendTermAck,
 *   tfChapSendTermAck) and modified the PPP FSM function tables to call
 *   these functions (rather than tfPppSendTermAck).
 */


/*
 * Unlike IPCP and LCP, PAP doesn't have a Terminate ACK packet.  So this
 * state transition function, instead of sending a ack packet, simply
 * changes to the correct state and restarts the retry timers.
 */
int tfPapSendTermAck(ttPppVectPtr pppVectPtr,
                      ttPacketPtr packetPtr)
{

    if (pppVectPtr->pppsStateInfoPtrArray[TM_PPP_PAP_INDEX]->
        cpsState > TM_PPPS_STOPPED)
    {
        pppVectPtr->pppsStateInfoPtrArray[TM_PPP_PAP_INDEX]->
            cpsState = TM_PPPS_INITIAL;
    }

    (void)tfPapInitRestartCounter(pppVectPtr, packetPtr);

    return TM_ENOERROR;
}

/*
 * Unlike IPCP and LCP, CHAP doesn't have a Terminate ACK packet.  So this
 * state transition function, instead of sending a ack packet, simply
 * changes to the correct state and restarts the retry timers.
 */
int tfChapSendTermAck(ttPppVectPtr pppVectPtr,
                      ttPacketPtr packetPtr)
{

    if (pppVectPtr->pppsStateInfoPtrArray[TM_PPP_CHAP_INDEX]->
        cpsState > TM_PPPS_STOPPED)
    {
       pppVectPtr->pppsStateInfoPtrArray[TM_PPP_CHAP_INDEX]->
            cpsState = TM_PPPS_INITIAL;
    }

    (void)tfChapInitRestartCounter(pppVectPtr, packetPtr);

    return TM_ENOERROR;
}


#ifdef TM_USE_IPV4
/*
 * Initialize the IPCP Layer
 */
static ttVoid tfIpcpInit(ttPppVectPtr pppVectPtr)
{
/*
 * Initialize this layer's negotiation counter.  This value is set to zero when
 * the device is first opened (ie, from a tfOpenInterface call).  Each time
 * this layer is renegotiated *within the same session* this value is
 * incremented.
 */
    pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsNegotiateCounter = 0;
    tfIpcpInitCommon(pppVectPtr);
    return;
}

static ttVoid tfIpcpInitCommon(ttPppVectPtr pppVectPtr)
{
    ttIpcpStatePtr  ipcpStatePtr;

    ipcpStatePtr = &pppVectPtr->pppsIpcpState;
    pppVectPtr->pppsStateInfoPtrArray[TM_PPP_IPCP_INDEX] =
                                                (ttCpStatePtr)ipcpStatePtr;
    ipcpStatePtr->ipcpsStateInfo.cpsStateFuncList =
            (ttPppStateFunctPtr TM_FAR *)tlIpcpStateFunctTable;
    tm_bcopy(&ipcpStatePtr->ipcpsLocalWantOptions,
             &ipcpStatePtr->ipcpsLocalNegOptions,
             sizeof(ttIpcpOptions));
    tm_ip_copy( tm_ip_dev_addr(pppVectPtr->pppsDeviceEntryPtr, 0),
                ipcpStatePtr->ipcpsLocalNegOptions.ipcpIpAddress );
    ipcpStatePtr->ipcpsLocalNegotiateOption |= (TM_UL(1)<< TM_IPCP_IP_ADDRESS);
/* ecr added 01/24/01 */
    (void)tfIpcpInitRestartCounter(pppVectPtr, (ttPacketPtr) 0);
    return;
}

/*
 * Initialize the IPCP Layer to Default Values
 */
static ttVoid tfIpcpDefaultInit(ttPppVectPtr pppVectPtr)
{
    ttIpcpStatePtr      ipcpStatePtr;
    ttCpStatePtr        pppCpStatePtr;
    ttIpcpOptionsPtr    ipcpOptionsPtr;
#ifdef TM_USE_IPHC
    ttLinkLayerEntryPtr devLinkLayerPtr;
    ttIphcOptionsPtr    iphcOptionsPtr;
#endif /* TM_USE_IPHC */

    ipcpStatePtr = &pppVectPtr->pppsIpcpState;

    ipcpOptionsPtr = &ipcpStatePtr->ipcpsLocalWantOptions;
    ipcpOptionsPtr->ipcpCompressProtocol = TM_UL(0);
    tm_ip_copy( tm_ip_dev_addr(pppVectPtr->pppsDeviceEntryPtr, 0),
                ipcpOptionsPtr->ipcpIpAddress );
    ipcpOptionsPtr->ipcpVjMaxSlots = 1;
    ipcpOptionsPtr->ipcpVjConnComp = 1;

    pppCpStatePtr = &ipcpStatePtr->ipcpsStateInfo;
    pppCpStatePtr->cpsInitTrmRetryCnt = TM_PPP_TERM_MAX_RETRY;
    pppCpStatePtr->cpsInitCfgRetryCnt = TM_PPP_CFG_MAX_RETRY;
    pppCpStatePtr->cpsRetryTimeoutSeconds = TM_PPP_RETRY_TIMEOUT;
    pppCpStatePtr->cpsInitMaxFailures = TM_PPP_MAX_FAILURES;

    ipcpStatePtr->ipcpsLocalNegotiateOption = (TM_UL(1)<< TM_IPCP_IP_ADDRESS);
    ipcpStatePtr->ipcpsRemoteNegotiateOption = (tt32Bit)0;

    ipcpStatePtr->ipcpsRemoteAllowOptions.ipcpVjMaxSlots = 1;
    ipcpStatePtr->ipcpsRemoteAllowOptions.ipcpVjConnComp = 1;

#ifdef TM_USE_IPHC
/* If RFC-2507 header compression is enabled, initialize options to default
   values .*/
    {
        devLinkLayerPtr =
                pppVectPtr->pppsDeviceEntryPtr->devLinkLayerProtocolPtr;
        if (devLinkLayerPtr->lnkHdrCompressFuncPtr !=
                                                   (ttLnkHdrCompressFuncPtr)0)
        {
            (*(devLinkLayerPtr->lnkHdrCompressFuncPtr))
                (TM_PACKET_NULL_PTR,
                 TM_32BIT_NULL_PTR,
                 (ttVoidPtr) &pppVectPtr->pppsIphcVectPtr,
                 TM_IPHC_INIT);
             iphcOptionsPtr = &pppVectPtr->pppsIphcVectPtr->iphcOpts;

             ipcpOptionsPtr = &ipcpStatePtr->ipcpsLocalWantOptions;
             ipcpOptionsPtr->ipcpCompTcpSpace =
                                             iphcOptionsPtr->iphcoInTcpSpace;
             ipcpOptionsPtr->ipcpCompNonTcpSpace =
                                             iphcOptionsPtr->iphcoInUdpSpace;
             ipcpOptionsPtr->ipcpCompMaxPeriod = iphcOptionsPtr->iphcoMaxPeriod;
             ipcpOptionsPtr->ipcpCompMaxTime = iphcOptionsPtr->iphcoMaxTime;
             ipcpOptionsPtr->ipcpCompMaxHeader = iphcOptionsPtr->iphcoMaxHeader;

             ipcpOptionsPtr = &ipcpStatePtr->ipcpsRemoteAllowOptions;
             ipcpOptionsPtr->ipcpCompTcpSpace = iphcOptionsPtr->iphcoInTcpSpace;
             ipcpOptionsPtr->ipcpCompNonTcpSpace =
                                             iphcOptionsPtr->iphcoInUdpSpace;
             ipcpOptionsPtr->ipcpCompMaxPeriod = iphcOptionsPtr->iphcoMaxPeriod;
             ipcpOptionsPtr->ipcpCompMaxTime = iphcOptionsPtr->iphcoMaxTime;
             ipcpOptionsPtr->ipcpCompMaxHeader = iphcOptionsPtr->iphcoMaxHeader;

        }
    }
#endif /* TM_USE_IPHC */

    pppCpStatePtr->cpsState = TM_PPPS_INITIAL;
    return;
}

static ttVoid tfIpcpReInit(ttPppVectPtr pppVectPtr)
{
    tfPppTimerRemove(&pppVectPtr->pppsIpcpState.ipcpsStateInfo);
    tfIpcpInitCommon(pppVectPtr);
    pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState = TM_PPPS_INITIAL;
    return;
}

/*
 * Parse a Reject
 * The remote didn't like some things that we were trying to config
 * so he said to turn them off
 */
int tfIpcpParseReject(ttPppVectPtr pppVectPtr,
                      ttPacketPtr  packetPtr,
                      tt16Bit      optionsLength)
{
    tt8BitPtr       dataPtr;
    tt8BitPtr       optionValuePtr;
    tt8Bit          optionType;
    tt8Bit          optionTotalLength;
    tt8Bit          optionLength;
    int             errorCode;
    tt32Bit         localNegOption;
    tt16Bit         compressProto;
#ifdef TM_DSP
    unsigned int    dataOffset;
    unsigned int    valueDataOffset;
#endif /* TM_DSP */

    tm_zero_dsp_offset(dataOffset);
    packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
    dataPtr = packetPtr->pktLinkDataPtr;
    optionsLength -= TM_PPP_CP_BYTES;
    errorCode = TM_ENOERROR;
    localNegOption = pppVectPtr->pppsIpcpState.ipcpsLocalNegotiateOption;
    while ( ((ttS16Bit)optionsLength > 0) && (errorCode == TM_ENOERROR) )
    {
/*
 * inside the while loop, use optionValuePtr rather than dataPtr
 * and use valueDataOffset in loop, leave dataOffset untouched each loop
 */
        tm_set_dsp_offset(valueDataOffset, dataOffset);

        optionValuePtr=dataPtr;
        optionType = tm_ppp_get_char_next(optionValuePtr, valueDataOffset);

/* Adjust to internal representation */
        if (optionType == TM_IPCP_OPT_DNS_PRI)
        {
            optionType = TM_IPCP_DNS_PRI;
        }
        else
        {
            if (optionType == TM_IPCP_OPT_DNS_SEC)
            {
                optionType = TM_IPCP_DNS_SEC;
            }
        }

        optionTotalLength = tm_ppp_get_char_next(optionValuePtr,
                                                 valueDataOffset);


        optionLength = (tt8Bit)(optionTotalLength - TM_8BIT_BYTE_COUNT
                                - TM_8BIT_BYTE_COUNT);

        switch (optionType)
        {
/* IP-Address, DNS servers and compression protocol options are 32-bits long. */
            case TM_IPCP_IP_ADDRESS:
            case TM_IPCP_DNS_SEC:
            case TM_IPCP_DNS_PRI:
                if (optionLength != TM_32BIT_BYTE_COUNT)
                {
                    errorCode = TM_EINVAL;
                }
                break;

            case TM_IPCP_COMP_PROTOCOL:
/* Get the requested option value */
                tm_bcopy(optionValuePtr,
                         &compressProto,
                         sizeof(compressProto));
                if (    (    (compressProto == TM_PPP_COMP_TCP_PROTOCOL)
                          && (optionLength != TM_32BIT_BYTE_COUNT))
                     || (    (compressProto == TM_PPP_IPHC_PROTOCOL)
                          && (optionLength < TM_PPP_IPHC_MIN_OPT_LEN)))
                {
                    errorCode = TM_EINVAL;
                }
                break;

/* Unrecognized option - since this is a Config-Rej, all options should always
   be recognized (since this is in response to our own Config-Req) so ignore
   this packet. */
            default:
                errorCode = TM_EINVAL;
                break;
        }

        if (optionType < (tt8Bit)32)
        {
            localNegOption &= ~(TM_UL(1)<< optionType);
        }

        tm_ppp_move_next_option(dataPtr, optionTotalLength, dataOffset);

        optionsLength = (tt16Bit)(optionsLength - optionTotalLength);
    }

/* No problems found in Conf-Rej packet, so record the results. */
    if (errorCode == TM_ENOERROR)
    {
        pppVectPtr->pppsIpcpState.ipcpsLocalNegotiateOption = localNegOption;
    }

    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);

    return errorCode;
}


/*
 * Parse a nak
 * The remote didn't like some things that we were trying to config
 * so he suggested new values
 */
int tfIpcpParseNak(ttPppVectPtr pppVectPtr,
                   ttPacketPtr  packetPtr,
                   tt16Bit      optionsLength)
{

    ttIpcpOptionsPtr    ipcpLocalNegOptionsPtr;
    tt8BitPtr           optionValuePtr;
    tt8BitPtr           dataPtr;
    tt8Bit              optionType;
    tt8Bit              optionTotalLength;
    tt8Bit              optionLength;
    int                 errorCode;
    tt32Bit             temp32;
#ifndef TM_DSP
    tt16Bit             compressProto;
#ifdef TM_USE_IPHC
    tt16Bit             iphcOpts[5];
#endif /* TM_USE_IPHC */
#endif /* !TM_DSP */
#ifdef TM_DSP
    unsigned int        dataOffset;
    unsigned int        valueDataOffset;
#endif /* TM_DSP */

    errorCode = TM_ENOERROR;

    tm_zero_dsp_offset(dataOffset);
    ipcpLocalNegOptionsPtr =
                    &pppVectPtr->pppsIpcpState.ipcpsLocalNegOptions;
    packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
    dataPtr = packetPtr->pktLinkDataPtr;
    optionsLength -= TM_PPP_CP_BYTES;
    while((ttS16Bit)optionsLength>0)
    {
        tm_set_dsp_offset(valueDataOffset, dataOffset);
        optionValuePtr=dataPtr;

        optionType = tm_ppp_get_char_next(optionValuePtr, valueDataOffset);
        optionTotalLength = tm_ppp_get_char_next(optionValuePtr,
                                                 valueDataOffset);

/* Get the length of the option data (without length and type fields */
        optionLength =
            (tt8Bit) (optionTotalLength - TM_8BIT_BYTE_COUNT
                      - TM_8BIT_BYTE_COUNT);

/* Adjust to internal representation */
        if (optionType == TM_IPCP_OPT_DNS_PRI)
        {
            optionType = TM_IPCP_DNS_PRI;
        }
        else
        {
            if (optionType == TM_IPCP_OPT_DNS_SEC)
            {
                optionType = TM_IPCP_DNS_SEC;
            }
        }

/* JNS/8-2-99 BUGFIX 413 */
/*
 * In addition to our normal option processing, now we also need to deal with
 * possibly broken PPP implementations on the other side.  There are some
 * PPP implementations that will NAK with the same value we request (violates
 * RFC1661).  Now, we will treat those as option-rejects.  So for each option
 * we process, also ensure that this isn't a bad NAK.
 */
        switch(optionType)
        {
            case TM_IPCP_IP_ADDRESS:
                if (optionLength != TM_32BIT_BYTE_COUNT)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto ipcpParseNakFinish;
                }
/* IP Addresses are always kept in network byte order */
                tm_ppp_byte_copy(optionValuePtr, valueDataOffset,
                                 &temp32, 0,TM_32BIT_BYTE_COUNT);
                if (tm_ip_match(temp32, ipcpLocalNegOptionsPtr->ipcpIpAddress))
                {
/* Bad NAK, treat as reject */
                    pppVectPtr->pppsIpcpState.ipcpsLocalNegotiateOption &=
                        ~(TM_UL(1)<< optionType);
                }
                else
                {
                    tm_ppp_byte_copy( optionValuePtr,valueDataOffset,
                              &(ipcpLocalNegOptionsPtr->ipcpIpAddress), 0,
                              TM_32BIT_BYTE_COUNT);
                }
                break;
            case TM_IPCP_DNS_PRI:
                if (optionLength != TM_32BIT_BYTE_COUNT)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto ipcpParseNakFinish;
                }
/* IP Addresses are always kept in network byte order */
                tm_ppp_byte_copy( optionValuePtr, valueDataOffset,
                                    &temp32, 0, TM_32BIT_BYTE_COUNT);
                if (tm_ip_match(temp32,
                    ipcpLocalNegOptionsPtr->ipcpDnsPriAddress))
                {
/* Bad NAK, treat as reject */
                    pppVectPtr->pppsIpcpState.ipcpsLocalNegotiateOption &=
                        ~(TM_UL(1) << optionType);
                }
                else
                {
                    tm_ppp_byte_copy( optionValuePtr, valueDataOffset,
                              &(ipcpLocalNegOptionsPtr->ipcpDnsPriAddress),0,
                              TM_32BIT_BYTE_COUNT);
                }
                break;
            case TM_IPCP_DNS_SEC:
                if (optionLength != TM_32BIT_BYTE_COUNT)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto ipcpParseNakFinish;
                }
/* IP Addresses are always kept in network byte order */
                tm_ppp_byte_copy( optionValuePtr, valueDataOffset,
                                    &temp32, 0, TM_32BIT_BYTE_COUNT);
                if (tm_ip_match(temp32,
                    ipcpLocalNegOptionsPtr->ipcpDnsSecAddress))
                {
                    pppVectPtr->pppsIpcpState.ipcpsLocalNegotiateOption &=
                        ~(TM_UL(1) << optionType);
                }
                else
                {
                    tm_ppp_byte_copy( optionValuePtr,valueDataOffset,
                              &(ipcpLocalNegOptionsPtr->ipcpDnsSecAddress),0,
                              TM_32BIT_BYTE_COUNT);
                }
                break;
#ifndef TM_DSP
/* VJ header compression not supported on DSP platforms */
            case TM_IPCP_COMP_PROTOCOL:
                tm_bcopy( optionValuePtr,
                          &compressProto,
                          sizeof(tt16Bit));
                if (compressProto == TM_PPP_COMP_TCP_PROTOCOL)
                {
                    if (optionLength != TM_32BIT_BYTE_COUNT)
                    {
                        errorCode = TM_EINVAL;
                        goto ipcpParseNakFinish;
                    }
                    else
                    {
                        ipcpLocalNegOptionsPtr->ipcpCompressProtocol =
                            compressProto;
                        optionValuePtr += sizeof(tt16Bit);
                        ipcpLocalNegOptionsPtr->ipcpVjMaxSlots =
                            *optionValuePtr++;
                        ipcpLocalNegOptionsPtr->ipcpVjConnComp =
                            *optionValuePtr;
                    }
                }
#ifdef TM_USE_IPHC
                else if (compressProto == TM_PPP_IPHC_PROTOCOL)
                {
                    if (optionLength < TM_PPP_IPHC_MIN_OPT_LEN)
                    {
                        errorCode = TM_EINVAL;
                        goto ipcpParseNakFinish;
                    }
                    else
                    {
                        if (   pppVectPtr->pppsIphcVectPtr == (ttIphcVectPtr) 0)
                        {
/* IPHC is not enabled, so treat this NAK as a reject. */
                            pppVectPtr->pppsIpcpState.
                                ipcpsLocalNegotiateOption &=
                                ~(TM_UL(1)<< optionType);
                        }
                        else
                        {
                            ipcpLocalNegOptionsPtr->ipcpCompressProtocol =
                                compressProto;
                            optionValuePtr += sizeof(tt16Bit);

                            tm_bcopy(optionValuePtr,
                                     iphcOpts,
                                     sizeof(iphcOpts));
/* Only NAK if the peer requests a different sized compression space; leave
   all other options as is. */
                            ipcpLocalNegOptionsPtr->ipcpCompTcpSpace =
                                ntohs(iphcOpts[0]);
                            ipcpLocalNegOptionsPtr->ipcpCompNonTcpSpace =
                                ntohs(iphcOpts[1]);
                            ipcpLocalNegOptionsPtr->ipcpCompMaxPeriod =
                                ntohs(iphcOpts[2]);
                            ipcpLocalNegOptionsPtr->ipcpCompMaxTime =
                                ntohs(iphcOpts[3]);
                            ipcpLocalNegOptionsPtr->ipcpCompMaxHeader =
                                ntohs(iphcOpts[4]);
                        }
                    }
                }
#endif /* TM_USE_IPHC */
                else
                {
/* Unrecognized compression protocol type, treat as a reject. */
                    pppVectPtr->pppsIpcpState.
                        ipcpsLocalNegotiateOption &= ~(TM_UL(1)<< optionType);
                }
                break;
#endif /* !TM_DSP */

/* Unrecognized option - since this is a Config-Nak, all options should always
   be recognized (since this is in response to our own Config-Req) so ignore
   this packet. */
            default:
                errorCode = TM_EINVAL;
                goto ipcpParseNakFinish;
        }

        tm_ppp_move_next_option(dataPtr, optionTotalLength, dataOffset);

        optionsLength = (tt16Bit)(optionsLength - optionTotalLength);
    }

ipcpParseNakFinish:

    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    return errorCode;
}

/*
 * Parse the incoming IPCP config request
 */
ttPacketPtr tfIpcpParseConfig(ttPppVectPtr pppVectPtr,
                              ttPacketPtr  packetPtr,
                              tt16Bit      length,
                              tt8BitPtr    pppEventPtr)
{
    ttPppCpPacketTypePtr cpHdrPtr;
    ttPacketPtr          rejectPacketPtr;
    ttPacketPtr          nakPacketPtr;
    ttPacketPtr          retPacketPtr;
    ttIpcpStatePtr       pppIpcpStatePtr;
    ttIpcpOptionsPtr     ipcpRemoteAllowOptionsPtr;
    tt8BitPtr            dataPtr;
    tt8BitPtr            optionValuePtr;
    tt8BitPtr            outDataPtr;
    ttIpcpOptions        remoteOptions;
    tt4IpAddress         remoteAllowIpAddress;
    tt4IpAddress         remoteAllowDnsPriAddress;
    tt4IpAddress         remoteAllowDnsSecAddress;
    tt32Bit              temp32;
    tt32Bit              remoteOptionsType;
    tt16Bit              reqCompProto;
    tt8Bit               optionType;
    tt8Bit               optionLength;
    tt8Bit               optionTotalLength;
    tt8Bit               ident;
    tt8Bit               sendNak;
    tt16Bit              param[6];
#ifdef TM_USE_IPHC
    tt16Bit              iphcOpts[5];
#endif /* TM_USE_IPHC */
#ifdef TM_DSP
    unsigned int         inDataOffset;
    unsigned int         outDataOffset;
    unsigned int         valueDataOffset;
#endif /* TM_DSP */


    tm_zero_dsp_offset(inDataOffset);
    tm_zero_dsp_offset(outDataOffset);
    rejectPacketPtr = TM_PACKET_NULL_PTR;
    nakPacketPtr = TM_PACKET_NULL_PTR;
    outDataPtr = (tt8BitPtr)0;
    cpHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    dataPtr = packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE;
    ident = cpHdrPtr->cpIdentifier;
    tm_bzero(&remoteOptions, sizeof(remoteOptions));
    remoteOptionsType = TM_UL(0);
    length -= TM_PPP_CP_BYTES;
    retPacketPtr = TM_PACKET_NULL_PTR; /* assume failure */
    pppIpcpStatePtr = &pppVectPtr->pppsIpcpState;
    ipcpRemoteAllowOptionsPtr = &pppIpcpStatePtr->ipcpsRemoteAllowOptions;
    tm_ip_copy( ipcpRemoteAllowOptionsPtr->ipcpIpAddress,
                remoteAllowIpAddress);
    tm_ip_copy( ipcpRemoteAllowOptionsPtr->ipcpDnsPriAddress,
                remoteAllowDnsPriAddress);
    tm_ip_copy( ipcpRemoteAllowOptionsPtr->ipcpDnsSecAddress,
                remoteAllowDnsSecAddress);
    while ( (ttS16Bit)length > 0 )
    {
        tm_set_dsp_offset(valueDataOffset,inDataOffset);
        optionValuePtr=dataPtr;
        optionType = tm_ppp_get_char_next(optionValuePtr, valueDataOffset);
        optionTotalLength = tm_ppp_get_char_next(optionValuePtr,
                                                 valueDataOffset);
/* Check for INVALID length (length cannot be less than 2) */
        if ((tt16Bit)optionTotalLength > length)
        {
            optionTotalLength = (tt8Bit)(length&0xff);
        }
        if (optionTotalLength < (tt8Bit)2)
        {
            optionTotalLength = (tt8Bit)2;
        }
        optionLength = (tt8Bit)(   optionTotalLength - TM_8BIT_BYTE_COUNT
                                 - TM_8BIT_BYTE_COUNT );
/* Adjust to internal representation */
        if (optionType == TM_IPCP_OPT_DNS_PRI)
        {
            optionType = TM_IPCP_DNS_PRI;
        }
        else
        {
            if (optionType == TM_IPCP_OPT_DNS_SEC)
            {
                optionType = TM_IPCP_DNS_SEC;
            }
        }
/* Make sure the value is in our bit range (only 32 bit options) */
        if (optionType < 32)
        {
/* Check to make sure we allow the remote to negotiate this option */
            if (   (pppIpcpStatePtr->ipcpsRemoteNegotiateOption)
                 & (TM_UL(1)<< optionType) )
            {
                switch(optionType)
                {
                    case TM_IPCP_COMP_PROTOCOL:
/* Get the requested option value */
                        tm_bcopy(optionValuePtr,
                                 &reqCompProto,
                                 sizeof(reqCompProto));
                        optionValuePtr += sizeof(reqCompProto);
/* If the peer request RFC-2507 IP header compression, we can't predict the
   length since the peer may include  an arbitrary number of suboptions.  For
   VJ header compression, the size must always be 4 bytes. */
                        if (    (    (reqCompProto == TM_PPP_COMP_TCP_PROTOCOL)
                                  && (optionLength != TM_32BIT_BYTE_COUNT))
                             || ( (reqCompProto == TM_PPP_IPHC_PROTOCOL)
                                  && (optionLength < TM_PPP_IPHC_MIN_OPT_LEN))
                             || (ipcpRemoteAllowOptionsPtr->
                                           ipcpCompressProtocol == TM_UL(0)))
                        {
/* Reject because of bad length */
                            rejectPacketPtr = tm_ppp_add_reject (
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto ipcpParseConfigFinish;
                            }
                        }
                        else
                        {
/* Check to see if we allow everything or it is the desired value */
                            sendNak = TM_8BIT_ZERO;
                            if ( ipcpRemoteAllowOptionsPtr->ipcpCompressProtocol
                                 == reqCompProto )
                            {
                                if (reqCompProto == TM_PPP_COMP_TCP_PROTOCOL)
                                {
                                    if (ipcpRemoteAllowOptionsPtr->ipcpVjMaxSlots
                                        == *optionValuePtr )
                                    {
                                        remoteOptionsType |=
                                            (TM_UL(1)<< optionType);
                                        remoteOptions.ipcpCompressProtocol =
                                            reqCompProto;
                                        remoteOptions.ipcpVjMaxSlots =
                                            *optionValuePtr;
                                        remoteOptions.ipcpVjConnComp =
                                            *(optionValuePtr+1);
                                    }
                                    else
                                    {
                                        sendNak = TM_8BIT_YES;
                                    }
                                }
#ifdef TM_USE_IPHC
                                else
                                {
                                    tm_bcopy(optionValuePtr,
                                             iphcOpts,
                                             sizeof(iphcOpts));
/* Only NAK if the peer requests a different sized compression space; leave
   all other options as is. */
                                    if (   (ipcpRemoteAllowOptionsPtr->
                                            ipcpCompTcpSpace
                                            == ntohs(iphcOpts[0]))
                                        && (ipcpRemoteAllowOptionsPtr->
                                            ipcpCompNonTcpSpace
                                            == ntohs(iphcOpts[1])))
                                    {
                                        remoteOptionsType |=
                                            (TM_UL(1)<< optionType);
                                        remoteOptions.ipcpCompressProtocol =
                                            reqCompProto;
                                        remoteOptions.ipcpCompTcpSpace =
                                            ntohs(iphcOpts[0]);
                                        remoteOptions.ipcpCompNonTcpSpace =
                                             ntohs(iphcOpts[1]);
                                        remoteOptions.ipcpCompMaxPeriod =
                                             ntohs(iphcOpts[2]);
                                        remoteOptions.ipcpCompMaxTime =
                                             ntohs(iphcOpts[3]);
                                        remoteOptions.ipcpCompMaxHeader =
                                             ntohs(iphcOpts[4]);
                                    }
                                    else
                                    {
                                        sendNak = TM_8BIT_YES;
                                    }

                                }
#endif /* TM_USE_IPHC */

                            }
                            else
                            {
                                sendNak = TM_8BIT_YES;
                            }



                            if (sendNak == TM_8BIT_YES)
                            {
                                if (pppIpcpStatePtr->ipcpsStateInfo.
                                      cpsFailures > 0)
                                {
/* The requested value is not allowed so NAK with an allowed value */
                                    if (reqCompProto == TM_PPP_COMP_TCP_PROTOCOL)
                                    {
                                        optionLength = sizeof(tt32Bit);
                                        tm_bcopy(&(ipcpRemoteAllowOptionsPtr->
                                                   ipcpCompressProtocol),
                                                 &param[0],sizeof(tt16Bit));
                                        tm_bcopy(&(ipcpRemoteAllowOptionsPtr->
                                                   ipcpVjMaxSlots),
                                                 &param[1], sizeof(tt16Bit));
                                    }
#ifdef TM_USE_IPHC
                                    else
                                    {
                                        optionLength = 12;
                                        tm_bcopy(&(ipcpRemoteAllowOptionsPtr->
                                                   ipcpCompressProtocol),
                                                 &param[0],sizeof(tt16Bit));
                                        param[1] =
                                            htons(ipcpRemoteAllowOptionsPtr->
                                                  ipcpCompTcpSpace);
                                        param[2] =
                                            htons(ipcpRemoteAllowOptionsPtr->
                                            ipcpCompNonTcpSpace);
                                        param[3] =
                                            htons(ipcpRemoteAllowOptionsPtr->
                                            ipcpCompMaxPeriod);
                                        param[4] =
                                            htons(ipcpRemoteAllowOptionsPtr->
                                            ipcpCompMaxTime);
                                        param[5] =
                                            htons(ipcpRemoteAllowOptionsPtr->
                                            ipcpCompMaxHeader);
                                    }
#endif /* TM_USE_IPHC */
                                    nakPacketPtr = tm_ppp_add_nak(
                                                            tfPppAddNak,
                                                            dataPtr,
                                                            (tt8BitPtr)&param[0],
                                                            &outDataPtr,
                                                            optionLength,
                                                            rejectPacketPtr,
                                                            nakPacketPtr,
                                                            inDataOffset,
                                                            &outDataOffset);
                                    if (    (nakPacketPtr
                                             == TM_PACKET_NULL_PTR)
                                         && (rejectPacketPtr
                                             == TM_PACKET_NULL_PTR) )
                                    {
                                        goto ipcpParseConfigFinish;
                                    }
                                }
                                else
                                {
/* We've NAKed more than 'max-failure' times - start rejecting this option */
                                    rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                                    if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                                    {
                                        goto ipcpParseConfigFinish;
                                    }
                                    nakPacketPtr = TM_PACKET_NULL_PTR;

                                }
                            }
                        }
                        break;
                    case TM_IPCP_IP_ADDRESS:
                        if (optionLength != TM_32BIT_BYTE_COUNT)
                        {
/* Reject because of bad length */
                            rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto ipcpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {
/* Get the requested option value */
                            tm_ppp_byte_copy(optionValuePtr,valueDataOffset,
                                     &temp32,0, TM_32BIT_BYTE_COUNT);
                            if (temp32 != (tt32Bit)0)
                            {
/*
 * The peer is trying to set his IP address. Check to see if this is
 * allowed
 */
                                if (pppIpcpStatePtr->
                                    ipcpsRemoteNegotiateOption &
                                    (TM_UL(1)<<optionType))
                                {
/*
 * Now, check if:
 * 1) The user set what he wants the remote's IP address to be as 0
 *    (allow anything), if so, allow the option.
 * 2) The user set what he wants the remote IP address to be as an actual
 *    address.  If it matches what the peer sent us, allow.  Otherwise,
 *    NAK with what we think the IP address SHOULD be.
 */
                                    if (   tm_ip_zero(remoteAllowIpAddress)
                                        || tm_ip_match(temp32,
                                                       remoteAllowIpAddress) )
                                    {
/* Set the peer's IP address */
                                        remoteOptionsType |=
                                                         (TM_UL(1)<<optionType);
                                        tm_ip_copy(temp32,
                                               remoteOptions.ipcpIpAddress);
                                    }
                                    else
                                    {

                                        if (pppIpcpStatePtr->ipcpsStateInfo.
                                              cpsFailures > 0)
                                        {
/* NAK with a better IP address */
                                            nakPacketPtr = tm_ppp_add_nak(
                                           tfPppAddNak,
                                           dataPtr,
                                           (tt8BitPtr)&(remoteAllowIpAddress),
                                           &outDataPtr,
                                           optionLength,
                                           rejectPacketPtr,
                                           nakPacketPtr,
                                           inDataOffset,
                                           &outDataOffset);
                                            if ((nakPacketPtr
                                                  == TM_PACKET_NULL_PTR)
                                               && (rejectPacketPtr
                                                  == TM_PACKET_NULL_PTR) )
                                            {
                                                goto ipcpParseConfigFinish;
                                            }
                                        }
                                        else
                                        {
/* We've NAKed more than 'max-failure' times - start rejecting this option */
                                            rejectPacketPtr =
                                              tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                                            if (rejectPacketPtr ==
                                                    TM_PACKET_NULL_PTR)
                                            {
                                                goto ipcpParseConfigFinish;
                                            }
                                            nakPacketPtr = TM_PACKET_NULL_PTR;
                                        }
                                    }
                                }
                            }
                            else
                            {
/*
 * The peer requested an IP address of zero, meaning that he wants us to NAK
 * with a valid IP address for him
 */
                                if (    tm_ip_not_zero(remoteAllowIpAddress)
                                     && (   pppIpcpStatePtr->
                                               ipcpsRemoteNegotiateOption
                                          & (TM_UL(1)<<optionType)) &&
                                    (pppIpcpStatePtr->ipcpsStateInfo.
                                       cpsFailures > 0))
                                {
/* Zero is allowed, so NAK with IP address */
                                    nakPacketPtr = tm_ppp_add_nak(
                                        tfPppAddNak,
                                        dataPtr,
                                        (tt8BitPtr)&(remoteAllowIpAddress),
                                        &outDataPtr,
                                        optionLength,
                                        rejectPacketPtr,
                                        nakPacketPtr,
                                        inDataOffset,
                                        &outDataOffset);
                                    if (    (    nakPacketPtr
                                              == TM_PACKET_NULL_PTR )
                                         && (    rejectPacketPtr
                                              == TM_PACKET_NULL_PTR) )
                                    {
                                        goto ipcpParseConfigFinish;
                                    }
                                }
                                else
                                {
/*
 * Either the user specifically disallowed setting the remote user's
 * IP address, or we've hit our 'max-failure' point, so reject this
 * option.
 */
                                    rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                                    if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                                    {
                                        goto ipcpParseConfigFinish;
                                    }
                                    nakPacketPtr = TM_PACKET_NULL_PTR;
                                }
                            }
                        }
                        break;
                    case TM_IPCP_DNS_PRI:
                        if (optionLength != TM_32BIT_BYTE_COUNT)
                        {
/* Reject because of bad length */
                            rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto ipcpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {
/* Get the requested option value */
                            tm_ppp_byte_copy(optionValuePtr,valueDataOffset,
                                  &temp32, 0, TM_32BIT_BYTE_COUNT);
                            if (temp32 != (tt32Bit)0)
                            {
/*
 * The peer is trying to set his IP address. Check to see if this is
 * allowed
 */
                                if (pppIpcpStatePtr->
                                    ipcpsRemoteNegotiateOption &
                                    (TM_UL(1)<<TM_IPCP_DNS_PRI))
                                {
/*
 * Now, check if:
 * 1) The user set what he wants the remote's IP address to be as 0
 *    (allow anything), if so, allow the option.
 * 2) The user set what he wants the remote IP address to be as an actual
 *    address.  If it matches what the peer sent us, allow.  Otherwise,
 *    NAK with what we think the IP address SHOULD be.
 */
                                    if (   tm_ip_zero(
                                                   remoteAllowDnsPriAddress)
                                        || tm_ip_match(temp32,
                                                   remoteAllowDnsPriAddress) )
                                    {
/* Set the peer's IP address */
                                        remoteOptionsType |=
                                            (TM_UL(1)<<TM_IPCP_DNS_PRI);
                                        tm_ip_copy(
                                            temp32,
                                            remoteOptions.ipcpDnsPriAddress);
                                    }
                                    else
                                    {

                                        if (pppIpcpStatePtr->ipcpsStateInfo.
                                              cpsFailures > 0)
                                        {
/* NAK with a better IP address */
                                            nakPacketPtr = tm_ppp_add_nak(
                                        tfPppAddNak,
                                        dataPtr,
                                        (tt8BitPtr)&(remoteAllowDnsPriAddress),
                                        &outDataPtr,
                                        optionLength,
                                        rejectPacketPtr,
                                        nakPacketPtr,
                                        inDataOffset,
                                        &outDataOffset);
                                            if ((nakPacketPtr
                                                  == TM_PACKET_NULL_PTR)
                                             && (rejectPacketPtr
                                                  == TM_PACKET_NULL_PTR) )
                                            {
                                               goto ipcpParseConfigFinish;
                                            }
                                        }
                                        else
                                        {
/* We've NAKed more than 'max-failure' times - start rejecting this option */
                                            rejectPacketPtr =
                                                tm_ppp_add_reject(
                                                        tfPppAddReject,
                                                        dataPtr,
                                                        &outDataPtr,
                                                        optionLength,
                                                        rejectPacketPtr,
                                                        nakPacketPtr,
                                                        inDataOffset,
                                                        &outDataOffset);

                                            if (rejectPacketPtr ==
                                                TM_PACKET_NULL_PTR)
                                            {
                                                goto ipcpParseConfigFinish;
                                            }
                                            nakPacketPtr = TM_PACKET_NULL_PTR;

                                        }
                                    }
                                }
                            }
                            else
                            {
/*
 * The peer requested an IP address of zero, meaning that he wants us to NAK
 * with a valid IP address for him
 */
                                if (  ( tm_ip_not_zero(
                                                     remoteAllowDnsPriAddress)
                                     && (   pppIpcpStatePtr->
                                               ipcpsRemoteNegotiateOption
                                          & (TM_UL(1)<<TM_IPCP_DNS_PRI)) ) ||
                                    (pppIpcpStatePtr->ipcpsStateInfo.
                                       cpsFailures) > 0)
                                {
/* Zero is allowed, so NAK with IP address */
                                    nakPacketPtr = tm_ppp_add_nak(
                                        tfPppAddNak,
                                        dataPtr,
                                        (tt8BitPtr)&(remoteAllowDnsPriAddress),
                                        &outDataPtr,
                                        optionLength,
                                        rejectPacketPtr,
                                        nakPacketPtr,
                                        inDataOffset,
                                        &outDataOffset);
                                    if (    (    nakPacketPtr
                                              == TM_PACKET_NULL_PTR )
                                         && (    rejectPacketPtr
                                              == TM_PACKET_NULL_PTR) )
                                    {
                                        goto ipcpParseConfigFinish;
                                    }
                                }
                                else
                                {
/*
 * Either the user specifically disallowed setting the remote user's
 * IP address, or we've hit our 'max-failure' point, so reject this
 * option.
 */
                                    rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                                    if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                                    {
                                        goto ipcpParseConfigFinish;
                                    }
                                    nakPacketPtr = TM_PACKET_NULL_PTR;
                                }
                            }
                        }
                        break;
                    case TM_IPCP_DNS_SEC:
                        if (optionLength != TM_32BIT_BYTE_COUNT)
                        {
/* Reject because of bad length */
                            rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto ipcpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {
/* Get the requested option value */
                            tm_ppp_byte_copy(optionValuePtr,valueDataOffset,
                                     &temp32,0, TM_32BIT_BYTE_COUNT);
                            if (temp32 != (tt32Bit)0)
                            {
/*
 * The peer is trying to set his IP address. Check to see if this is
 * allowed
 */
                                if (pppIpcpStatePtr->
                                    ipcpsRemoteNegotiateOption &
                                    (TM_UL(1)<<TM_IPCP_DNS_SEC))
                                {
/*
 * Now, check if:
 * 1) The user set what he wants the remote's IP address to be as 0
 *    (allow anything), if so, allow the option.
 * 2) The user set what he wants the remote IP address to be as an actual
 *    address.  If it matches what the peer sent us, allow.  Otherwise,
 *    NAK with what we think the IP address SHOULD be.
 */
                                    if (   tm_ip_zero(
                                                  remoteAllowDnsSecAddress)
                                        || tm_ip_match(
                                                  temp32,
                                                  remoteAllowDnsSecAddress) )
                                    {
/* Set the peer's IP address */
                                        remoteOptionsType |=
                                                    (TM_UL(1)<<TM_IPCP_DNS_SEC);
                                        tm_ip_copy(
                                             temp32,
                                             remoteOptions.ipcpDnsSecAddress);
                                    }
                                    else
                                    {

/* NAK with a better IP address */
                                        if (pppIpcpStatePtr->ipcpsStateInfo.
                                              cpsFailures > 0)
                                        {
                                            nakPacketPtr = tm_ppp_add_nak(
                               tfPppAddNak,
                               dataPtr,
                               (tt8BitPtr)&(remoteAllowDnsSecAddress),
                               &outDataPtr,
                               optionLength,
                               rejectPacketPtr,
                               nakPacketPtr,
                               inDataOffset,
                               &outDataOffset);
                                            if (    (    nakPacketPtr
                                                  == TM_PACKET_NULL_PTR)
                                                 && (    rejectPacketPtr
                                                      == TM_PACKET_NULL_PTR) )
                                            {
                                                goto ipcpParseConfigFinish;
                                            }
                                        }
                                        else
                                        {
/* We've NAKed more than 'max-failure' times - start rejecting this option */
                                            rejectPacketPtr =
                                                tm_ppp_add_reject(
                                                        tfPppAddReject,
                                                        dataPtr,
                                                        &outDataPtr,
                                                        optionLength,
                                                        rejectPacketPtr,
                                                        nakPacketPtr,
                                                        inDataOffset,
                                                        &outDataOffset);

                                            if (rejectPacketPtr ==
                                                TM_PACKET_NULL_PTR)
                                            {
                                                goto ipcpParseConfigFinish;
                                            }
                                            nakPacketPtr = TM_PACKET_NULL_PTR;

                                        }
                                    }
                                }
                            }
                            else
                            {
/*
 * The peer requested an IP address of zero, meaning that he wants us to NAK
 * with a valid IP address for him
 */
                                if (  ( tm_ip_not_zero(
                                                    remoteAllowDnsSecAddress)
                                     && (   pppIpcpStatePtr->
                                               ipcpsRemoteNegotiateOption
                                          & (TM_UL(1)<<TM_IPCP_DNS_SEC)) ) ||
                                    (pppIpcpStatePtr->ipcpsStateInfo.
                                       cpsFailures > 0))
                                {
/* Zero is allowed, so NAK with IP address */
                                    nakPacketPtr = tm_ppp_add_nak(
                                        tfPppAddNak,
                                        dataPtr,
                                        (tt8BitPtr)&(remoteAllowDnsSecAddress),
                                        &outDataPtr,
                                        optionLength,
                                        rejectPacketPtr,
                                        nakPacketPtr,
                                        inDataOffset,
                                        &outDataOffset);
                                    if (    (    nakPacketPtr
                                              == TM_PACKET_NULL_PTR )
                                         && (    rejectPacketPtr
                                              == TM_PACKET_NULL_PTR) )
                                    {
                                        goto ipcpParseConfigFinish;
                                    }
                                }
                                else
                                {
/*
 * Either zero is not allowed or we've sent more than 'max-failure' NAK
 * packets - reject this option.
 */
                                    rejectPacketPtr = tm_ppp_add_reject(
                                                             tfPppAddReject,
                                                             dataPtr,
                                                             &outDataPtr,
                                                             optionLength,
                                                             rejectPacketPtr,
                                                             nakPacketPtr,
                                                             inDataOffset,
                                                             &outDataOffset);
                                    if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                                    {
                                        goto ipcpParseConfigFinish;
                                    }
                                    nakPacketPtr = TM_PACKET_NULL_PTR;
                                }
                            }
                        }
                        break;
                    default:
/* Option not supported so reject it*/
                        rejectPacketPtr = tm_ppp_add_reject(
                                                         tfPppAddReject,
                                                         dataPtr,
                                                         &outDataPtr,
                                                         optionLength,
                                                         rejectPacketPtr,
                                                         nakPacketPtr,
                                                         inDataOffset,
                                                         &outDataOffset);
                        if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                        {
                            goto ipcpParseConfigFinish;
                        }
                        nakPacketPtr = TM_PACKET_NULL_PTR;
                        break;
                }
            }
            else
            {
/* Option not allowed so reject it */
                rejectPacketPtr = tm_ppp_add_reject(
                                                 tfPppAddReject,
                                                 dataPtr,
                                                 &outDataPtr,
                                                 optionLength,
                                                 rejectPacketPtr,
                                                 nakPacketPtr,
                                                 inDataOffset,
                                                 &outDataOffset);
                if (rejectPacketPtr == TM_PACKET_NULL_PTR)
                {
                    goto ipcpParseConfigFinish;
                }
                nakPacketPtr = TM_PACKET_NULL_PTR;
            }
        }
        else
        {
/* Option value out of range so reject it*/
            rejectPacketPtr = tm_ppp_add_reject(
                                             tfPppAddReject,
                                             dataPtr,
                                             &outDataPtr,
                                             optionLength,
                                             rejectPacketPtr,
                                             nakPacketPtr,
                                             inDataOffset,
                                             &outDataOffset);
            if (rejectPacketPtr == TM_PACKET_NULL_PTR)
            {
                goto ipcpParseConfigFinish;
            }
            nakPacketPtr = TM_PACKET_NULL_PTR;
        }

        tm_ppp_move_next_option(dataPtr, optionTotalLength, inDataOffset);

        length = (tt16Bit)(length - optionTotalLength);
    }

/*
 * If we're going to send a NAK (ie, nakPacketPtr != NULL), decrement the
 * max-failure count.
 */
    if (nakPacketPtr != TM_PACKET_NULL_PTR)
    {
        --pppIpcpStatePtr->ipcpsStateInfo.cpsFailures;
    }
    retPacketPtr = tm_ppp_parse_finish (tfPppParseFinish,
                                        packetPtr, rejectPacketPtr,
                                        nakPacketPtr, outDataPtr, pppEventPtr,
                                        ident, outDataOffset);
    if (retPacketPtr == packetPtr)
    {
/* Packet is an ACK (not an NAK, or Reject) */
        tm_bcopy( &remoteOptions,
                  &(pppIpcpStatePtr->ipcpsRemoteSetOptions),
                  sizeof(ttIpcpOptions));
        pppIpcpStatePtr->ipcpsRemoteAckedOptions = remoteOptionsType;
    }


ipcpParseConfigFinish:
    if (retPacketPtr == TM_PACKET_NULL_PTR)
    {
        tfPppNoBuffer(pppEventPtr, packetPtr);
    }
    return retPacketPtr;
}


/*
 * Process an inbound IPCP request
 */
int tfIpcpIncomingPacket(ttPppVectPtr pppVectPtr,ttPacketPtr packetPtr)
{
    ttPppCpPacketTypePtr ipcpHdrPtr;
    int                  isEqual;
    int                  errorCode;
    tt16Bit              optionsLength;
    tt8Bit               needFreePacket;
    tt8Bit               needTimerRemove;
    tt8Bit               event;
    tt8Bit               validAck;

    errorCode = TM_ENOERROR;
    ipcpHdrPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
    optionsLength = ipcpHdrPtr->cpLength;
    optionsLength = htons(optionsLength);
    needFreePacket = TM_8BIT_ZERO;
    needTimerRemove = TM_8BIT_ZERO;
    event = TM_PPPE_NO_EVENT;

/* Verify that the length from the IPCP header is not greater than the actual
   length of the packet. */
    if ((ttPktLen)optionsLength > packetPtr->pktChainDataLength)
    {
        needFreePacket = TM_8BIT_YES;
        goto IpcpIncomingPacketFinish;
    }

    switch(ipcpHdrPtr->cpCode)
    {
        case TM_PPP_CONFIG_REQUEST:
/* The remote sent us a config request so we gotta parse it */
            packetPtr = tfIpcpParseConfig(pppVectPtr,
                                          packetPtr,
                                          optionsLength,
                                          &event);
/* Crank state machine with event */
            break;
        case TM_PPP_CONFIG_ACK:

/*
 * Verify that the identifier on this Conf-ACK matches the last Conf-REQ
 * that was sent (RFC1661, section 5.2).
 */
            validAck = TM_8BIT_YES;
            if (!tm_8bit_cur_id(ipcpHdrPtr->cpIdentifier,
                    pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsIdentifier))
            {
                validAck = TM_8BIT_ZERO;
            }
            else
            {
/*
 * Verify that the length of the Conf-ACK is correct and the options are
 * identical to those sent in our last Conf-REQ.
 */
                if (optionsLength !=
                    (tt16Bit)(pppVectPtr->pppsLastConfReqLen
                              + TM_PPP_CP_BYTES))
                {
                    validAck = TM_8BIT_ZERO;
                }
                else
                {
                    isEqual = tm_ppp_memcmp(
                              packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE,
                              pppVectPtr->pppsLastConfReq,
                              pppVectPtr->pppsLastConfReqLen);
                    if( isEqual != TM_STRCMP_EQUAL )
                    {
                        validAck = TM_8BIT_ZERO;
                    }
                }
            }

            if (validAck == TM_8BIT_YES)
            {
/* If RFC-2507 IP header compression is being used and if operating in dual
   IPv4/IPv6 mode, and IPV6CP is up we need to merge the two sets of
   configuration values (IPCP & IPV6CP): */
#ifdef TM_USE_IPHC
                tfPppSetIphcValues(pppVectPtr,
                                   pppVectPtr->pppsIpcpState.
                                     ipcpsLocalNegOptions.ipcpCompressProtocol,
                                   AF_INET);
#endif /* TM_USE_IPHC */

/* He acked our options so now we can use them */
                tm_bcopy(&(pppVectPtr->pppsIpcpState.ipcpsLocalNegOptions),
                         &(pppVectPtr->pppsIpcpState.ipcpsLocalGotOptions),
                         sizeof(ttIpcpOptions));
/* Remove our retry timer */
                needTimerRemove = TM_8BIT_YES;
/* Free the Packet */
                needFreePacket = TM_8BIT_YES;
/* Crank the state machine to let it know we got an ACK */
                event = TM_PPPE_CFG_ACK;
            }
            else
            {
/* Invalid Conf-Ack packet - discard */
                needFreePacket  = TM_8BIT_YES;
                needTimerRemove = TM_8BIT_ZERO;
/*              event is defaulted to be TM_PPPE_NO_EVENT;*/
            }
            break;
        case TM_PPP_CONFIG_NAK:
            if (tm_8bit_cur_id(ipcpHdrPtr->cpIdentifier,
                    pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsIdentifier))
            {
                errorCode = tfIpcpParseNak(pppVectPtr,
                                           packetPtr,
                                           optionsLength);
/* Packet freed in tfIpcpParseNak */
                packetPtr = TM_PACKET_NULL_PTR;
                if (errorCode == TM_ENOERROR)
                {
/* Remove our retry timer */
                    needTimerRemove = TM_8BIT_YES;
/* Crank the state machine to let it know we got a NAK (REJECT) */
                    event = TM_PPPE_CFG_NAK;
                }
                else
                {
/* Conf-Nak was invalid, so ignore it. */
                    needTimerRemove = TM_8BIT_ZERO;
/*                  event default: TM_PPPE_NO_EVENT;*/
                }
            }
            else
            {
/* Old Conf-Nak packet (bad identifier) - discard */
                needFreePacket  = TM_8BIT_YES;
                needTimerRemove = TM_8BIT_ZERO;
/*                event       default to be TM_PPPE_NO_EVENT;*/
            }
            break;
        case TM_PPP_CONFIG_REJECT:
            if (tm_8bit_cur_id(ipcpHdrPtr->cpIdentifier,
                    pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsIdentifier))
            {
                errorCode = tfIpcpParseReject(pppVectPtr,
                                              packetPtr,
                                              optionsLength);
/* Packet freed in tfIpcpParseReject */
                packetPtr = TM_PACKET_NULL_PTR;
                if (errorCode == TM_ENOERROR)
                {
/* Remove our retry timer */
                    needTimerRemove = TM_8BIT_YES;
/* Crank the state machine to let it know we got a NAK (REJECT) */
                    event = TM_PPPE_CFG_NAK;
                }
                else
                {
/* Conf-Rej was invalid, so ignore it. */
                    needTimerRemove = TM_8BIT_ZERO;
/*                    event = TM_PPPE_NO_EVENT;*/
                }
            }
            else
            {
/* Old Conf-Rej packet (bad identifier) - discard */
                needFreePacket  = TM_8BIT_YES;
                needTimerRemove = TM_8BIT_ZERO;
/*                event           = TM_PPPE_NO_EVENT;*/
            }
            break;
        case TM_PPP_TERM_REQUEST:
/* Remove our retry timer */
            needTimerRemove = TM_8BIT_YES;
/* We got a term request so send a term ack */
            event = TM_PPPE_TRM_REQ;
            break;
        case TM_PPP_TERM_ACK:
/* we go a term ack so start the close */
            needFreePacket = TM_8BIT_YES;
/* Remove our retry timer */
            needTimerRemove = TM_8BIT_YES;
            event = TM_PPPE_TRM_ACK;
            break;
        case TM_PPP_CODE_REJECT:
/* He rejected one of our codes so close */
            needFreePacket = TM_8BIT_YES;
/* Remove our retry timer */
            needTimerRemove = TM_8BIT_YES;
            event = TM_PPPE_CLOSE;
            break;
        default:
/* We didn't understand his code so send an unknown code to him */
            event = TM_PPPE_UNKNOWN_CODE;
            break;
    }

IpcpIncomingPacketFinish:
    if (needTimerRemove != TM_8BIT_ZERO)
    {
        tfPppTimerRemove(&pppVectPtr->pppsIpcpState.ipcpsStateInfo);
    }
    if (needFreePacket != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        packetPtr = TM_PACKET_NULL_PTR;
    }
    if (event != TM_PPPE_NO_EVENT)
    {
        errorCode = tfPppStateMachine(pppVectPtr,
                                      packetPtr,
                                      event,
                                      TM_PPP_IPCP_INDEX);
    }
    return errorCode;
}

/*
 *
 *
 * IPCP State Functions
 *
 *
 */


/*
 * An IPCP Timeout has occured
 */
void tfIpcpTimeout (ttVoidPtr      timerBlockPtr,
                    ttGenericUnion userParm1,
                    ttGenericUnion userParm2)
{
    ttPppVectPtr pppVectPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    pppVectPtr = (ttPppVectPtr) userParm1.genVoidParmPtr;
/* Crank the state machine to let it know we got a timeout */
    tfPppTimeout( pppVectPtr,
                  &pppVectPtr->pppsIpcpState.ipcpsStateInfo,
                  TM_PPP_IPCP_INDEX);

}

/*
 * Initialize the Restart/Retry Counter for Terminate and configure requests
 */
int tfIpcpInitRestartCounter ( ttPppVectPtr pppVectPtr,
                               ttPacketPtr packetPtr)
{
/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
    tfPppInitRestartCounter(&pppVectPtr->pppsIpcpState.ipcpsStateInfo);
    return TM_ENOERROR;
}

/*
 * Zero the Restart/Retry Counter
 */
int tfIpcpZeroRestartCounter(ttPppVectPtr pppVectPtr,
                             ttPacketPtr packetPtr)
{
/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);
    pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsRetryCounter = 0;
    return TM_ENOERROR;
}


/*
 * Indicate to the lower layers the we are starting the automation
 * We are suppposed to wait for a lower layer up event, but we know
 * that the LCP opened before we start the state machine for IPCP so
 * we just go ahead and signal the "UP" event
 */
int tfIpcpThisLayerStart(ttPppVectPtr pppVectPtr,
                         ttPacketPtr  packetPtr)
{
    return tfPppStateMachine(pppVectPtr, packetPtr, TM_PPPE_UP,
                             TM_PPP_IPCP_INDEX);
}


/*
 * Indicate to the LCP Layer that we are done with the link
 * Post a "DOWN" event to LCP
 */
int tfIpcpThisLayerFinish(ttPppVectPtr pppVectPtr,
                          ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);

    if (pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState == TM_PPPS_STOPPED)
    {
        tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_IP4_OPEN_FAILED);
        tfPppNcpOpenFail(pppVectPtr);
    }
    else
    {
#ifdef TM_USE_IPV6
        if (pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState ==
            TM_PPPS_INITIAL)
#endif /* TM_USE_IPV6 */
        {
            (void)tfPppStateMachine(pppVectPtr,
                                    TM_PACKET_NULL_PTR,
                                    TM_PPPE_CLOSE,
                                    TM_PPP_LCP_INDEX);
        }

    }


    tfPppTimerRemove(&pppVectPtr->pppsIpcpState.ipcpsStateInfo);
    return TM_ENOERROR;
}

/*
 * for IPCP we just notify the user and add the device to the router
 * Since PPP is only established once (even for multihome), we
 * use index 0 for the route when the link is up
 */
int tfIpcpThisLayerUp(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    ttDeviceEntryPtr devPtr;
    tt4IpAddress     peerIpAddress;
    int              errorCode;

/* Unused Parameter */
    TM_UNREF_IN_ARG(packetPtr);

/* Increment this layer's negotiation counter */
    pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsNegotiateCounter++;

    devPtr = pppVectPtr->pppsDeviceEntryPtr;

    (void)tfPppNextLayer(pppVectPtr,TM_PPP_IPCP_INDEX);

/*
 * If this is a renegotiation, first remove the device.  This is to ensure
 * that the active interface count remains correct.
 */
    if (pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsNegotiateCounter > 1)
    {
        (void)tfRemoveInterface(devPtr, TM_16BIT_ZERO, PF_INET);
    }

/* Note device netmask set in tfConfigInterface */
    tm_ip_copy( pppVectPtr->pppsIpcpState.ipcpsLocalGotOptions.ipcpIpAddress,
                tm_ip_dev_addr(devPtr,0) );
    tm_ip_copy( pppVectPtr->pppsIpcpState.ipcpsRemoteSetOptions.ipcpIpAddress,
                peerIpAddress );
    if (tm_ip_not_zero(peerIpAddress))
    {
/* Only copy remote IP address if not zero */
        tm_ip_copy(peerIpAddress, devPtr->devPt2PtPeerIpAddr);
    }

/*
 * Since PPP can trivially support multicasting set the TM_DEV_MCAST_ENB
 * flag for this device.
 */
    devPtr->devFlag |= TM_DEV_MCAST_ENB;

/* Add it to the routing table */
    errorCode = tfDeviceStart(devPtr, TM_16BIT_ZERO, PF_INET);
    if (errorCode == TM_ENOERROR)
    {
/* Remove the open failure timer since at least one NCP is up */
        if (pppVectPtr->pppsOpenFailTimerPtr != TM_TMR_NULL_PTR)
        {
            tm_timer_remove(pppVectPtr->pppsOpenFailTimerPtr);
            pppVectPtr->pppsOpenFailTimerPtr = TM_TMR_NULL_PTR;
        }

/* Notify the user that the link is up */
        tfPppUserNotify(devPtr, TM_LL_IP4_OPEN_COMPLETE);

/*
 * If this is the first NCP open for this session (i.e., IPV6CP isn't open),
 * notify the user. (If IPv6 isn't enabled, we do this unconditionally)
 */
#ifdef TM_USE_IPV6
        if (pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.
                cpsState != TM_PPPS_OPENED)
#endif /* TM_USE_IPV6 */
        {
            tfPppUserNotify(devPtr, TM_LL_OPEN_COMPLETE);
        }

    }
    else
    {
/* Fail the link */
        tfPppNcpOpenFail(pppVectPtr);
    }

    return errorCode;
}

/*
 * Send a config NAK/Reject to the peer
 * We have already built the nak/reject on input processing
 */
int tfIpcpSendCfgNak(ttPppVectPtr pppVectPtr, ttPacketPtr packetPtr)
{
    (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_PPP_IPCP_PROTOCOL);

    return TM_ENOERROR;
}

/*
 * Send a code reject to the peer
 */
int tfIpcpSendCodeReject(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr)
{
    return tfPppSendCodeReject(pppVectPtr, packetPtr,
                               &pppVectPtr->pppsIpcpState.ipcpsStateInfo,
                               TM_PPP_IPCP_PROTOCOL);
}

/*
 * Create an IPCP config request packet and send it
 */
int tfIpcpSendCfgRequest(ttPppVectPtr pppVectPtr,
                         ttPacketPtr packetPtr)
{
/* Passed in value is not used here */
    ttPppCpPacketTypePtr configRequestPtr;
    ttIpcpOptionsPtr     localNegOptionPtr;
    tt8BitPtr            vjSlots1Ptr;
    tt8BitPtr            vjSlots2Ptr;
    tt32Bit              negotiate;
    int                  errorCode;
/* Enough to hold both VJ and IPHC options. */
    tt16Bit              param[5];
    tt16Bit              temp16;
    tt16Bit              copyOptionValue;
    tt8Bit               paramSize;
#ifdef TM_DSP
    unsigned int         optionPtrOffset;
#endif /* TM_DSP */

    tm_zero_dsp_offset(optionPtrOffset);
    errorCode = TM_ENOERROR;
    packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                   TM_PPP_IPCP_MAX_BYTES
                                   - TM_PPP_ALLOC_HDR_BYTES,
                                   TM_16BIT_ZERO);
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        configRequestPtr = (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
        packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
        negotiate = pppVectPtr->pppsIpcpState.ipcpsLocalNegotiateOption;
/* The IP address we want to use (zero if we expect the remote to set
 * it for us)
 */
        tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate,
               TM_IPCP_IP_ADDRESS,
               (tt8BitPtr)&pppVectPtr->pppsIpcpState.
                   ipcpsLocalNegOptions.ipcpIpAddress,
               TM_32BIT_BYTE_COUNT, TM_8BIT_NULL_PTR, 0,
               (unsigned int *) &optionPtrOffset);

/* The compression protocol we want to use */
        copyOptionValue=pppVectPtr->pppsIpcpState.
                ipcpsLocalNegOptions.ipcpCompressProtocol;
        localNegOptionPtr = &pppVectPtr->pppsIpcpState.ipcpsLocalNegOptions;
        paramSize = TM_16BIT_BYTE_COUNT;
        if (copyOptionValue == TM_PPP_COMP_TCP_PROTOCOL)
        {
            vjSlots1Ptr = ((tt8BitPtr)&param[0]);
            vjSlots2Ptr = ((tt8BitPtr)&param[0])+1;
            *vjSlots1Ptr = localNegOptionPtr->ipcpVjMaxSlots;
            *vjSlots2Ptr = localNegOptionPtr->ipcpVjConnComp;
        }
#ifdef TM_USE_IPHC
        else
        {
/* Assume RFC-2507 IP header compression. */
            param[0] = htons(localNegOptionPtr->ipcpCompTcpSpace);
            param[1] = htons(localNegOptionPtr->ipcpCompNonTcpSpace);
            param[2] = htons(localNegOptionPtr->ipcpCompMaxPeriod);
            param[3] = htons(localNegOptionPtr->ipcpCompMaxTime);
            param[4] = htons(localNegOptionPtr->ipcpCompMaxHeader);
            paramSize = 10;
        }
#endif /* TM_USE_IPHC */

        tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate,
                           TM_IPCP_COMP_PROTOCOL,
                           (tt8BitPtr)&copyOptionValue, TM_16BIT_BYTE_COUNT,
                           (tt8BitPtr)&param[0],paramSize,
                           (unsigned int *) &optionPtrOffset);

/* The DNS Server we want to use (zero if we expect the remote to set
 * it for us)
 */
        tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate,
               TM_IPCP_DNS_PRI,
               (tt8BitPtr)&pppVectPtr->pppsIpcpState.
                   ipcpsLocalNegOptions.ipcpDnsPriAddress,
               TM_32BIT_BYTE_COUNT, TM_8BIT_NULL_PTR, 0,
               (unsigned int *) &optionPtrOffset);

/* The DNS Server we want to use (zero if we expect the remote to set
 * it for us)
 */
        tm_ppp_send_option(tfPppSendOption, packetPtr, negotiate,
               TM_IPCP_DNS_SEC,
               (tt8BitPtr)&pppVectPtr->pppsIpcpState.
                   ipcpsLocalNegOptions.ipcpDnsSecAddress,
               TM_32BIT_BYTE_COUNT, TM_8BIT_NULL_PTR, 0,
               (unsigned int *) &optionPtrOffset);

/* Set up our config request header */
        configRequestPtr->cpCode = TM_PPP_CONFIG_REQUEST;
        configRequestPtr->cpIdentifier = pppVectPtr->pppsIpcpState.
                                               ipcpsStateInfo.cpsIdentifier++;
        temp16 = (tt16Bit)
            (tm_byte_count(
                packetPtr->pktLinkDataPtr - (tt8BitPtr)configRequestPtr));
#ifdef TM_DSP
        temp16 += optionPtrOffset;
#endif /* TM_DSP */
        packetPtr->pktLinkDataLength = (ttPktLen)temp16;
        tm_htons(temp16, configRequestPtr->cpLength);
        packetPtr->pktLinkDataPtr = (tt8BitPtr)configRequestPtr;

/*
 * Save a copy of the Conf-Req and the length of the Conf-Req so these can
 * be validated when a Conf-Ack is received.
 */
        pppVectPtr->pppsLastConfReqLen = (tt8Bit)(temp16 - TM_PPP_CP_BYTES);
        tm_ppp_byte_copy(packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE,
                         0,
                         pppVectPtr->pppsLastConfReq,
                         0,
                         temp16 - TM_PPP_CP_BYTES);

/* Start our retry timer */
        tfPppTimerAdd( pppVectPtr,
                       tfIpcpTimeout,
                       &pppVectPtr->pppsIpcpState.ipcpsStateInfo );
        (void) tfPppDeviceSend( pppVectPtr,
                                packetPtr,
                                TM_PPP_IPCP_PROTOCOL);
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }
    return errorCode;
}

/*
 * Send an IPCP Terminate Ack.  It is rare for an implementation to send a
 * *IPCP* terminate request (rather than a LCP), however, sending a IPCP
 * terminate ACK serves as a hint to our peer that we are in the closed or
 * stopped state, and in need of re-negotiation.  Each of the functions
 * (IPCP,PAP,CHAP) need to be seperate because we can't simply re-use the
 * previous packet, so we have to form a completely new packet.
 *
 */
int tfIpcpSendTermAck(ttPppVectPtr pppVectPtr,
                      ttPacketPtr packetPtr)
{
    /* Passed in value is not used here */
    ttPppCpPacketTypePtr termAckPtr;
    int                  errorCode;
    tt16Bit              temp16;


    errorCode = TM_ENOERROR;

/*
 * If we need to, get a new packet.  Otherwise, simply reuse the one passed
 * in.
 */
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                       TM_PPP_IPCP_MAX_BYTES
                                       - TM_PPP_ALLOC_HDR_BYTES,
                                       TM_16BIT_ZERO);
        if (packetPtr != TM_PACKET_NULL_PTR)
        {
            termAckPtr = (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
/* Set up our config request header */
            termAckPtr->cpCode = TM_PPP_TERM_ACK;
            termAckPtr->cpIdentifier = pppVectPtr->pppsIpcpState.
                                            ipcpsStateInfo.cpsIdentifier++;
            temp16 = (tt16Bit)(TM_PPP_CP_BYTES);
            packetPtr->pktLinkDataLength = (ttPktLen)temp16;
            tm_htons(temp16, termAckPtr->cpLength);

/* Send it! */
            (void) tfPppDeviceSend( pppVectPtr,
                                    packetPtr,
                                    TM_PPP_IPCP_PROTOCOL );
        }
        else
        {
            errorCode = TM_ENOBUFS;
        }
    }
    else
    {

        termAckPtr = (ttPppCpPacketTypePtr)(packetPtr->pktLinkDataPtr);
        termAckPtr->cpCode = TM_PPP_TERM_ACK;
/*
 * Because we are reusing the input packet, we pass a type of zero to tell
 * the linkLayer to just update the pointers to the header
 */
         (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);
    }

    if (errorCode == TM_ENOERROR)
    {
        if (pppVectPtr->pppsStateInfoPtrArray[TM_PPP_IPCP_INDEX]->
            cpsState > TM_PPPS_STOPPED)
        {
            pppVectPtr->pppsStateInfoPtrArray[TM_PPP_IPCP_INDEX]->
                cpsState = TM_PPPS_INITIAL;
        }
        (void)tfIpcpInitRestartCounter(pppVectPtr, packetPtr);
    }

    return errorCode;
}

/* VJ header compression disabled on DSP targets. */
#ifndef TM_DSP
/*
 * Compare two strings to see if they are equal
 * Does not need to be fast since it is so rarely called
 */
static int tfVjMemCmp(char TM_FAR * memPtr1,
                      char TM_FAR * memPtr2,
                      unsigned      memLength)
{
    int     retCode;

    retCode = TM_STRCMP_EQUAL;

    while (memLength != (unsigned)0)
    {
        if (*memPtr1 < *memPtr2)
        {
            retCode = TM_STRCMP_LESS;
            break;
        }
        if (*memPtr1 > *memPtr2)
        {
            retCode = TM_STRCMP_GREATER;
            break;
        }
        memPtr1++;
        memPtr2++;
        memLength--;
    }
    return retCode;
}

static int tfVjCompIncomingPacket(ttPppVectPtr pppVectPtr,
                                  ttPacketPtr  packetPtr)
{

    ttVjCompFieldsPtr dataPtr;
    ttVjCompDataPtr   vjCompDataPtr;
    ttIpHeaderPtr     ipHdrPtr;
    ttTcpHeaderPtr    tcpHdrPtr;
    tt32Bit           temp32;
    ttPktLen          optionsLength;
    int               errorCode;
    unsigned int      compHdrLength;
    tt16Bit           temp16;
    tt16Bit           ipHdrLength;
    tt16Bit           tcpHdrLength;
    tt16Bit           tcpIpHdrLength;
    tt16Bit           prevSegDataLength;
    tt8Bit            connId;
    tt8Bit            vjFlags;
    tt8Bit            maxRecvSlots;

    errorCode = TM_ENOERROR;
    if (pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState != TM_PPPS_OPENED)
    {
        errorCode = TM_EINVAL;
        goto vjCompIncomingExit;
    }
/* If we haven't allocated memory yet, do it now */
    maxRecvSlots = (tt8Bit)(pppVectPtr->pppsIpcpState.ipcpsLocalGotOptions.
                                                          ipcpVjMaxSlots + 1);
    if (pppVectPtr->pppsVjRecvDataPtr == (ttVjCompDataPtr)0)
    {
        compHdrLength = (unsigned int)
                        ((unsigned int)maxRecvSlots * sizeof(ttVjCompData));
        vjCompDataPtr = (ttVjCompDataPtr)tm_get_raw_buffer(
                                                     (ttPktLen)compHdrLength);
        if (vjCompDataPtr != (ttVjCompDataPtr)0)
        {
            tm_bzero(vjCompDataPtr, compHdrLength);
            pppVectPtr->pppsVjRecvDataPtr = vjCompDataPtr;
        }
        else
        {
            errorCode = TM_ENOBUFS;
        }
    }

    if (errorCode == TM_ENOBUFS)
    {
        goto vjCompIncomingExit;
    }

    dataPtr = (ttVjCompFieldsPtr)packetPtr->pktLinkDataPtr;

    if (*((tt8BitPtr) dataPtr-1) == TM_PPP_UNCOMP_TCP_PROTOCOL_BYTE)
/* Uncompressed TCP */
    {
        ipHdrLength =
            tm_ip_hdr_len(((ttIpHeaderPtr)(ttVoidPtr)dataPtr)->iphVersLen);
        tcpHdrPtr = (ttTcpHeaderPtr)(((tt8BitPtr)dataPtr) + ipHdrLength);
        tcpHdrLength = tm_tcp_hdrLen(tcpHdrPtr->tcpDataOffset);
        tcpIpHdrLength = (tt16Bit)(tcpHdrLength + ipHdrLength);
/* Update our state */
        connId = ((ttIpHeaderPtr)(ttVoidPtr)dataPtr)->iphUlp;
        if (connId >= maxRecvSlots)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfVjCompIncomingPacket",
                            "VJ slot ID too large");
#endif /* TM_ERROR_CHECKING */
            errorCode = TM_EINVAL;
            goto vjCompIncomingExit;
        }
        pppVectPtr->pppsLastRecvConnId = connId;
        vjCompDataPtr = &(pppVectPtr->pppsVjRecvDataPtr[connId]);
/* Replace the protocol field with TCP */
        ((ttIpHeaderPtr)(ttVoidPtr)dataPtr)->iphUlp = IP_PROTOTCP;
        tm_bcopy( dataPtr,
                 &(vjCompDataPtr->vjsIpHeader),
                 tcpIpHdrLength );
        vjCompDataPtr->vjsIpHdrLength = (tt8Bit)ipHdrLength;
        vjCompDataPtr->vjsTcpHdrLength = (tt8Bit)tcpHdrLength;
/* Zero IP header checksum */
        vjCompDataPtr->vjsIpHeader.iphChecksum = (tt16Bit)0;
    }
    else
/* Compressed TCP */
    {
        vjFlags = dataPtr->vjFlags;
        if (vjFlags & TM_VJ_C)
        {
            connId = dataPtr->vjConnection;

            if (connId >= maxRecvSlots)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfVjCompIncomingPacket",
                                "VJ slot ID too large");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_EINVAL;
                goto vjCompIncomingExit;
            }

            pppVectPtr->pppsLastRecvConnId = connId;
        }
        else
        {
            connId = pppVectPtr->pppsLastRecvConnId;
        }
        vjCompDataPtr = &(pppVectPtr->pppsVjRecvDataPtr[connId]);
        ipHdrPtr = &vjCompDataPtr->vjsIpHeader;
        ipHdrLength = (tt16Bit)vjCompDataPtr->vjsIpHdrLength;
        tcpHdrPtr = (ttTcpHeaderPtr)(   ((tt8BitPtr)ipHdrPtr)
                                      + ipHdrLength );
        tcpHdrLength = (tt16Bit)vjCompDataPtr->vjsTcpHdrLength;
        if (    (tcpHdrLength < TM_PAK_TCP_HDR_LEN)
             || (ipHdrLength < TM_4PAK_IP_MIN_HDR_LEN) )
        {
           errorCode = TM_EINVAL;
           goto vjCompIncomingExit;
        }
        tcpIpHdrLength = (tt16Bit)( ipHdrLength + tcpHdrLength );
/* Set ACK flag */
        tcpHdrPtr->tcpControlBits |= TM_TCP_ACK;

/* TCP checksum */
        tcpHdrPtr->tcpChecksum = dataPtr->vjTcpCheckSum;

        if (vjFlags & TM_VJ_P)
/* Push flag */
        {
            tcpHdrPtr->tcpControlBits |= TM_TCP_PSH;
        }
        else
        {
            tm_8bit_clr_bit( tcpHdrPtr->tcpControlBits, TM_TCP_PSH );
        }

        if ((vjFlags & TM_VJ_STAR_SA) == TM_VJ_STAR_SA)
/* Special case: *SA (1011), *S (1111) */
        {
/* Common code for *SA and *S */
            prevSegDataLength = (tt16Bit)(   ntohs(ipHdrPtr->iphTotalLength)
                                           - tcpIpHdrLength );
            temp32 =   ntohl(tcpHdrPtr->tcpSeqNo)
                     + prevSegDataLength;
            tm_htonl(temp32, tcpHdrPtr->tcpSeqNo);
            if ((vjFlags & TM_VJ_STAR_S) == TM_VJ_STAR_SA)
            {
/* *SA (1011) only */
                 temp32 =   ntohl(tcpHdrPtr->tcpAckNo)
                          + prevSegDataLength;
                 tm_htonl(temp32, tcpHdrPtr->tcpAckNo);
            }
        }
        else
        {
/* Normal SAWU case */

/* TCP Sequence # */
            if (vjFlags & TM_VJ_S)
            {
                temp32 =   ntohl(dataPtr->vjSequence)
                         + ntohl(tcpHdrPtr->tcpSeqNo);
                tm_htonl(temp32, tcpHdrPtr->tcpSeqNo);
            }
/* TCP acknowledgement */
            if (vjFlags & TM_VJ_A)
            {
                temp32 =   ntohl(dataPtr->vjAck)
                         + ntohl(tcpHdrPtr->tcpAckNo);
                tm_htonl(temp32, tcpHdrPtr->tcpAckNo);
            }
/* TCP Window */
            if (vjFlags & TM_VJ_W)
            {
                tm_ntohs(tcpHdrPtr->tcpWindowSize, temp16);
                tm_ntohl(dataPtr->vjWindow, temp32);
#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(temp16)
#endif /* TM_LINT */
                temp16 = (tt16Bit)(temp16 + (tt16Bit)temp32);
#ifdef TM_LINT
LINT_UNINIT_SYM_END(temp16)
#endif /* TM_LINT */
                tm_htons(temp16, tcpHdrPtr->tcpWindowSize);
            }
/* TCP Urgent Offset */
            if (vjFlags & TM_VJ_U)
            {
/* It is necessary to convert the urgent pointer from network order to host
   order, and then back again, since the urgent pointer in the VJ header
   is a 32-bit value, and the urgent pointer in the TCP header is a 16-bit
   value.  To correctly cast, we have to correctly convert between byte
   orders. */
                tm_ntohl(dataPtr->vjUrgent, temp32);
                temp16 = (tt16Bit)temp32;
                tm_htons(temp16, tcpHdrPtr->tcpUrgentPtr);
                tcpHdrPtr->tcpControlBits |= TM_TCP_URG;
            }
            else
            {
                tm_8bit_clr_bit( tcpHdrPtr->tcpControlBits, TM_TCP_URG );
            }
        }
/* IP identification */
        tm_ntohs(ipHdrPtr->iphId, temp16);
        if (vjFlags & TM_VJ_I)
        {
            tm_ntohl(dataPtr->vjIdent, temp32);
            temp16 = (tt16Bit)(temp16 + (tt16Bit)temp32);
        }
        else
        {
            temp16 += 1;
        }
        tm_htons(temp16, ipHdrPtr->iphId);
/*
 * If compressed packet contains options, add options length
 * and leave room for options.
 */
        if ( tcpIpHdrLength > TM_VJ_COMP_SIZE )
        {
            optionsLength = tcpIpHdrLength - TM_VJ_COMP_SIZE;
            packetPtr->pktChainDataLength += optionsLength;
            packetPtr->pktLinkDataPtr -= (unsigned int)optionsLength;
        }
/*
 * Copy the updated packet header to packetPtr (to be processed by IP, TCP
 * layers).
 */
        packetPtr->pktLinkDataLength = packetPtr->pktChainDataLength;
        ipHdrPtr->iphTotalLength = htons(packetPtr->pktLinkDataLength);
/* ULP was set to IP_PROTOTCP after we had copied the headers */
/* IP header checksum was set to zero after we had copied the headers */
        tm_bcopy( ipHdrPtr,
                  packetPtr->pktLinkDataPtr,
                  tcpIpHdrLength);
        ipHdrPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
        if (ipHdrLength == TM_4PAK_IP_MIN_HDR_LEN)
        {
            ipHdrPtr->iphChecksum =
                tfIpHdr5Checksum((tt16BitPtr)(ttVoidPtr)ipHdrPtr);
        }
        else
        {
            ipHdrPtr->iphChecksum = tfPacketChecksum( packetPtr,
                                                      (ttPktLen)ipHdrLength,
                                                      TM_16BIT_NULL_PTR,
                                                      TM_16BIT_ZERO);
        }
    }
vjCompIncomingExit:
    return errorCode;
}

static int tfVjCompSendPacket(ttPppVectPtr  pppVectPtr,
                              ttPacketPtr   packetPtr,
                              tt16BitPtr    pktTypePtr)
{
    int              i;
    ttVjCompDataPtr  vjDataPtr;
    ttVjCompDataPtr  vjOldDataPtr;
    ttIpHeaderPtr    vjIpHdrPtr;
    ttIpHeaderPtr    ipHdrPtr;
    ttTcpHeaderPtr   tcpHdrPtr;
    ttTcpHeaderPtr   vjTcpHdrPtr;
    tt8BitPtr        dataPtr;
    tt8BitPtr        newLinkDataPtr;
    tt8BitPtr        oldLinkDataPtr;
    tt8BitPtr        flagPtr;
    tt32Bit          deltaAck;
    tt32Bit          deltaSeq;
    ttPktLen         compTcpIpHdrLength;
    ttPktLen         trimmedTcpIpHdrBytes;
    tt16Bit          tcpChecksum;
    tt16Bit          urgOff;
    tt16Bit          deltaWin;
    tt16Bit          deltaIpId;
    tt16Bit          temp16;
    tt16Bit          ipHdrLength;
    tt16Bit          tcpHdrLength;
    tt16Bit          tcpIpHdrLength;
    tt16Bit          prevSegDataLength;
    tt8Bit           flagBits;
    tt8Bit           connId;
    tt8Bit           dataLength;
    tt8Bit           bufSize;
    int              isEqual;
    int              retCode;

    retCode = TM_ENOERROR;
    ipHdrPtr = (ttIpHeaderPtr) packetPtr->pktLinkDataPtr;
    ipHdrLength = packetPtr->pktIpHdrLen;
    tcpHdrPtr = (ttTcpHeaderPtr)(((tt8BitPtr)ipHdrPtr)+ipHdrLength);
/*
 * Check to see if this packet is uncompressible (ie, ACK isn't set OR one of
 * SYN, FIN, RST is set, OR this is an IP fragment)
 */
    if (   ( (   tcpHdrPtr->tcpControlBits
               & (TM_TCP_ACK|TM_TCP_FIN|TM_TCP_SYN|TM_TCP_RST))
                                                           != TM_TCP_ACK )
        || ( ipHdrPtr->iphFlagsFragOff & TM_IP_MORE_FRAG_OFFSET) )
    {
/* Can't compress, so leave packet as is and return. */
        *pktTypePtr = TM_PPP_IP_PROTOCOL;
        goto VjSendReturn;
    }
/* It is compressible. */
    tcpHdrLength = tm_tcp_hdrLen(tcpHdrPtr->tcpDataOffset);
    tcpIpHdrLength = (tt16Bit)(ipHdrLength + tcpHdrLength);
    dataPtr = (tt8BitPtr)ipHdrPtr;
    vjDataPtr = pppVectPtr->pppsVjSendDataPtr;
    bufSize = (tt8Bit) (pppVectPtr->pppsIpcpState.
                        ipcpsRemoteSetOptions.ipcpVjMaxSlots + (tt8Bit)1);
    if ( vjDataPtr == (ttVjCompDataPtr)0 )
    {
        vjDataPtr = (ttVjCompDataPtr)tm_get_raw_buffer((ttPktLen)
                                              (bufSize * sizeof(ttVjCompData)));
        if (vjDataPtr == (ttVjCompDataPtr)0)
        {
            retCode = TM_ENOBUFS;
/* return with error, packet freed in tfDeviceSend */
            goto VjSendReturn;
        }
        for (dataLength = 0; dataLength < bufSize; dataLength++)
        {
            (vjDataPtr+dataLength)->vjsUsed = 0;
            (vjDataPtr+dataLength)->vjsConnId = dataLength;
            (vjDataPtr+dataLength)->vjsNextPtr = vjDataPtr+dataLength+1;
        }
        pppVectPtr->pppsVjSendDataPtr = vjDataPtr;

    }

/*
 * Turn off TCP header recycling because VJ compression modifies the
 * header. Both Compressed and uncompressed changes the header
 */
    tm_16bit_clr_bit( packetPtr->pktSharedDataPtr->dataFlags,
                      TM_BUF_TCP_HDR_BLOCK );

/* Check for previous entry in data table */

    dataLength = 0;
    vjOldDataPtr = vjDataPtr;
    while (dataLength < bufSize && vjDataPtr->vjsUsed == 1)
    {
        vjIpHdrPtr = &vjDataPtr->vjsIpHeader;
        if (    (vjIpHdrPtr->iphSrcAddr == ipHdrPtr->iphSrcAddr)
             && (vjIpHdrPtr->iphDestAddr == ipHdrPtr->iphDestAddr) )
        {
            vjTcpHdrPtr = (ttTcpHeaderPtr)(   ((tt8BitPtr)vjIpHdrPtr)
                                                 + vjDataPtr->vjsIpHdrLength);
            if (    (vjTcpHdrPtr->tcpSrcPort == tcpHdrPtr->tcpSrcPort)
                 && (vjTcpHdrPtr->tcpDstPort == tcpHdrPtr->tcpDstPort) )
            {
                break;
            }
        }
        dataLength++;
        if (dataLength < bufSize)
        {
/* Check on dataLength to make sure we do not point beyond allocated data */
            vjOldDataPtr = vjDataPtr;
            vjDataPtr = vjDataPtr->vjsNextPtr;
        }
    }

    if (dataLength == bufSize)
/* No entry found, re-use the last entry (least used) */
    {
/*
 * vjDataPtr, vjTcpHdrPtr and vjIpHdrPtr still point to last entry headers,
 * and vjOldDataPtr point to previous entry which will allow us to move
 * to front of the list. Without check on dataLength above, memory could
 * be corrupted.
 */
/* Invalidate last entry, so that we can start using it for this connection */
        vjDataPtr->vjsUsed = 0;
    }

    if (dataLength != 0)
/*
 * If entry is not firt, move this entry to the front of the list
 * (most recently used)
 */
    {
        vjOldDataPtr->vjsNextPtr = vjDataPtr->vjsNextPtr;
        vjDataPtr->vjsNextPtr = pppVectPtr->pppsVjSendDataPtr;
        pppVectPtr->pppsVjSendDataPtr = vjDataPtr;
    }

    if (vjDataPtr->vjsUsed == 0)
/* New uncompressed header is sent */
    {
        vjDataPtr->vjsUsed = 1;
        goto VjSendUncompressed;
    }
/*
 * Although we know that vjIpHdrPtr, and vjTcpHdrPtr have been initialized
 * in the search above, we re-initialize them here to avoid compiler
 * warnings
 */
    vjIpHdrPtr = &vjDataPtr->vjsIpHeader;
    vjTcpHdrPtr = (ttTcpHeaderPtr)(((tt8BitPtr)vjIpHdrPtr)
                                               + vjDataPtr->vjsIpHdrLength);

/*
 * Check to make sure this packet is actually compressible (ie, nothing
 * changed that we didn't expect - IP vers/length, TOS, "don't fragment",
 * and TTL, TCP data Offset).
 */

    if (    (vjIpHdrPtr->iphVersLen != ipHdrPtr->iphVersLen)
         || (vjIpHdrPtr->iphTos != ipHdrPtr->iphTos)
         || (    (vjIpHdrPtr->iphFlagsFragOff & TM_IP_FRAG_BITS)
              != (ipHdrPtr->iphFlagsFragOff & TM_IP_FRAG_BITS))
         || (vjIpHdrPtr->iphTtl != ipHdrPtr->iphTtl)
         || (vjTcpHdrPtr->tcpDataOffset != tcpHdrPtr->tcpDataOffset) )
    {
        goto VjSendUncompressed;
    }
/*
 * check for changes in IP header options.
 */
    if (ipHdrLength != TM_4PAK_IP_MIN_HDR_LEN)
/* If we have any IP options, check for changes in IP options area */
    {
        isEqual = tfVjMemCmp(
                        ((ttCharPtr)vjIpHdrPtr) + TM_4PAK_IP_MIN_HDR_LEN,
                        ((ttCharPtr)ipHdrPtr) + TM_4PAK_IP_MIN_HDR_LEN,
                        ipHdrLength - TM_4PAK_IP_MIN_HDR_LEN);
        if ( isEqual != TM_STRCMP_EQUAL )
        {
            goto VjSendUncompressed;
        }
    }
/*
 * check for changes in TCP header options.
 */
    if (tcpHdrLength != TM_PAK_TCP_HDR_LEN)
/* If we have any TCP options, check for changes in TCP options area */
    {
        isEqual = tfVjMemCmp( ((ttCharPtr)vjTcpHdrPtr) + TM_PAK_TCP_HDR_LEN,
                              ((ttCharPtr)tcpHdrPtr) + TM_PAK_TCP_HDR_LEN,
                              tcpHdrLength - TM_PAK_TCP_HDR_LEN);
        if ( isEqual != TM_STRCMP_EQUAL )
        {
            goto VjSendUncompressed;
        }
    }

/* Start trying to compress this packet */
    flagBits = 0x00;
/*
 * Store integers value in host byte order in:
 * variable         corresponding to
 * urgOff            urgOff (U)
 * deltaWin          &Win   (W)
 * deltaAck          &Ack   (A)
 * deltaSeq          &Seq   (S)
 * deltaIpId         &IpId   (I)
 * since we  will need to do integer comparisons on those fields to figure
 * out how many bytes to store (between 0, and 3) in the header. Of course
 * we will store those values in network byte order in the header before
 * sending the data on the network.
 */
/* Initialize variables to avoid compiler warnings */
    urgOff = (tt16Bit)0;
    deltaWin = (tt16Bit)0;
    deltaSeq = (tt32Bit)0;
    deltaAck = (tt32Bit)0;
    connId = (tt8Bit)0;
/* Set flags for changed fields  */
    if (tcpHdrPtr->tcpWindowSize != vjTcpHdrPtr->tcpWindowSize)
    {
        flagBits |= TM_VJ_W;
/* Host byte order */
        deltaWin = (tt16Bit)(   ntohs(tcpHdrPtr->tcpWindowSize)
                              - ntohs(vjTcpHdrPtr->tcpWindowSize));
    }
    if (tcpHdrPtr->tcpAckNo != vjTcpHdrPtr->tcpAckNo)
    {
/* Host byte order */
        deltaAck =   ntohl(tcpHdrPtr->tcpAckNo)
                   - ntohl(vjTcpHdrPtr->tcpAckNo);
        if (deltaAck > TM_UL(0xFFFF))
        {
/*
 * Since we can only store up to 2 bytes, we cannot compress if the difference
 * is less than zero or greater than 65535
 */
            goto VjSendUncompressed;
        }
        flagBits |= TM_VJ_A;
    }
    if (tcpHdrPtr->tcpSeqNo != vjTcpHdrPtr->tcpSeqNo)
    {
/* Host byte order */
        deltaSeq =   ntohl(tcpHdrPtr->tcpSeqNo)
                   - ntohl(vjTcpHdrPtr->tcpSeqNo);
        if (deltaSeq > TM_UL(0xFFFF))
        {
/*
 * Since we can only store up to 2 bytes, we cannot compress if the difference
 * is less than zero or greater than 65535
 */
            goto VjSendUncompressed;
        }
        flagBits |= TM_VJ_S;
    }

/*
 * If nothing changed, and ( if the last packet contained data, or we are
 * sending the same length packet ), it's probably a retransmit,
 * retransmitted ack or window probe.  Send it uncompressed
 * in case the other side missed the compressed version.
 */
    if (    (flagBits == 0)
         && (    ( vjIpHdrPtr->iphTotalLength != htons(tcpIpHdrLength) )
              || (    ipHdrPtr->iphTotalLength
                   == vjIpHdrPtr->iphTotalLength)) )
    {
        goto VjSendUncompressed;
    }
    if ( (tcpHdrPtr->tcpControlBits & TM_TCP_URG) == TM_TCP_URG )
/* Urgent data */
    {
/*
 * If urgent bit is set, store urgent offset in host byte order in vjFields.
 */
        flagBits |= TM_VJ_U;
        urgOff = ntohs(tcpHdrPtr->tcpUrgentPtr);
    }
    else
    {
        if (tcpHdrPtr->tcpUrgentPtr != vjTcpHdrPtr->tcpUrgentPtr)
        {
/*
 * Urgent bit not set, but urgent offset changes. We need to send
 * uncompressed
 */
            goto VjSendUncompressed;
        }
    }

/* Network byte order, since we need to do a straight copy here */
    tcpChecksum = tcpHdrPtr->tcpChecksum;

/*
 * Special case, S*WU (1011, or 1111), (i.e S, W, and U all set), send
 * uncompressed, since we want to use this special values for *SA or *S.
 */
    if ((flagBits & TM_VJ_SWU) == TM_VJ_SWU)
    {
        goto VjSendUncompressed;
    }
/* Check whether Ident field has increased by one */
    deltaIpId = (tt16Bit) (   ntohs(ipHdrPtr->iphId)
                            - ntohs(vjIpHdrPtr->iphId) );

    if (deltaIpId > 1)
    {
        flagBits |= TM_VJ_I;
    }
/* Check whether the push bit is set */
    if ( tcpHdrPtr->tcpControlBits & TM_TCP_PSH)
    {
        flagBits |= TM_VJ_P;
    }
/* Check whether we changed connection ID */
    if ( pppVectPtr->pppsLastSentConnId != vjDataPtr->vjsConnId )
    {
        flagBits |= TM_VJ_C;
/* Store connection ID byte */
        connId = vjDataPtr->vjsConnId;
    }

/*
 * Compute Data length of previous packet (we use tcpIpHdrLength because we
 * already checked that the previous segment has same IP and TCP header
 * lengths as the current segment)
 */
    prevSegDataLength = (tt16Bit)(   ntohs(vjIpHdrPtr->iphTotalLength)
                                   - tcpIpHdrLength );

/*
 * We're definitely going to compress the packet at this point,
 * so go ahead and save this packet as "last sent" and start modifying
 * the actual packet
 */
    tm_bcopy( dataPtr,
              &(vjDataPtr->vjsIpHeader),
              tcpIpHdrLength );
    vjDataPtr->vjsIpHdrLength = (tt8Bit)ipHdrLength;
    vjDataPtr->vjsTcpHdrLength = (tt8Bit)tcpHdrLength;


/* Update the 'last connection ID sent' field */

    pppVectPtr->pppsLastSentConnId = vjDataPtr->vjsConnId;

/* Form the actual VJ compressed packet */
/* Store only the second byte in case the protocol itself is compressed */
/* Don't have to worry about DSP here - VJ code is disabled for DSP */
    *pktTypePtr = TM_PPP_COMP_TCP_PROTOCOL;

    flagPtr = dataPtr++; /* will store flagBits later */
    compTcpIpHdrLength = 1; /* Compressed TCP/IP header length: flag byte*/
/* ConnID */
    if (flagBits & TM_VJ_C)
    {
        *((tt8BitPtr)dataPtr++) = connId;
        compTcpIpHdrLength++; /* connection ID byte */
    }
/* Checksum in network byte order already, so keep same byte ordering */
    dataPtr[0] = ((tt8BitPtr)(&tcpChecksum))[0];
    dataPtr[1] = ((tt8BitPtr)(&tcpChecksum))[1];
    dataPtr += sizeof(tt16Bit);
    compTcpIpHdrLength += sizeof(tt16Bit); /* 2-byte checksum field */
    if (    ( (flagBits & TM_VJ_SAWU) == TM_VJ_SA )
         && ( deltaSeq == (tt32Bit)prevSegDataLength)
         && ( deltaSeq == deltaAck) )
/* Special case *SA (1011) */
    {
        flagBits = (tt8Bit)((flagBits & ~TM_VJ_SAWU) | TM_VJ_STAR_SA);
    }
    else
    {
        if (    ( ( flagBits & TM_VJ_SAWU ) == TM_VJ_S )
             && ( deltaSeq == (tt32Bit)prevSegDataLength ) )
/* Special case *S (1111) */
        {
            flagBits = (tt8Bit)(flagBits | TM_VJ_STAR_S);
        }
        else
        {
            if (flagBits & TM_VJ_U)
            {
                if (urgOff > 0xFF || urgOff == 0)
                {
                    *((tt8BitPtr) dataPtr++) = 0;
/* Network byte order copy: copy MS byte in low memory */
                    dataPtr[0] = (tt8Bit)(urgOff >> 8);
/* Network byte order copy: copy LS byte in high memory */
                    dataPtr[1] = (tt8Bit)urgOff;
                    dataPtr += sizeof(tt16Bit);
                    compTcpIpHdrLength += 3;
                }
                else
                {
                    *((tt8BitPtr)dataPtr++) = (tt8Bit)urgOff;
                    compTcpIpHdrLength++;
                }
            }
            if (flagBits & TM_VJ_W)
            {
                if (deltaWin > 0xFF)
                {
                    *((tt8BitPtr) dataPtr++) = 0;
/* Network byte order copy: copy MS byte in low memory */
                    dataPtr[0] = (tt8Bit)(deltaWin >> 8);
/* Network byte order copy: copy LS byte in high memory */
                    dataPtr[1] = (tt8Bit)deltaWin;
                    dataPtr += sizeof(tt16Bit);
                    compTcpIpHdrLength += 3;
                }
                else
                {
                    *((tt8BitPtr)dataPtr++) = (tt8Bit)deltaWin;
                    compTcpIpHdrLength++;
                }
            }
            if (flagBits & TM_VJ_A)
            {
                temp16 = (tt16Bit)deltaAck;
                if (temp16 > 0xFF)
                {
                    *((tt8BitPtr) dataPtr++) = 0;
/* Network byte order copy: copy MS byte in low memory */
                    dataPtr[0] = (tt8Bit)(temp16 >> 8);
/* Network byte order copy: copy LS byte in high memory */
                    dataPtr[1] = (tt8Bit)temp16;
                    dataPtr += sizeof(tt16Bit);
                    compTcpIpHdrLength += 3;
                }
                else
                {
                    *((tt8BitPtr) dataPtr++) = (tt8Bit)temp16;
                    compTcpIpHdrLength++;
                }

            }
            if (flagBits & TM_VJ_S)
            {
                temp16 = (tt16Bit)deltaSeq;
                if (temp16 > 0xFF)
                {
                    *((tt8BitPtr) dataPtr++) = 0;
/* Network byte order copy: copy MS byte in low memory */
                    dataPtr[0] = (tt8Bit)(temp16 >> 8);
/* Network byte order copy: copy LS byte in high memory */
                    dataPtr[1] = (tt8Bit)temp16;
                    dataPtr += sizeof(tt16Bit);
                    compTcpIpHdrLength += 3;
                }
                else
                {
                    *((tt8BitPtr) dataPtr++) = (tt8Bit)temp16;
                    compTcpIpHdrLength++;
                }
            }
        }
    }
    if (flagBits & TM_VJ_I)
    {
        if (deltaIpId > 0xFF || deltaIpId == 0)
        {
            *((tt8BitPtr) dataPtr++) = 0;
/* Network byte order copy: copy MS byte in low memory */
            dataPtr[0] = (tt8Bit)(deltaIpId >> 8);
/* Network byte order copy: copy LS byte in high memory */
            dataPtr[1] = (tt8Bit)deltaIpId;
            dataPtr += sizeof(tt16Bit);
            compTcpIpHdrLength += 3;
        }
        else
        {
            *((tt8BitPtr)dataPtr++) = (tt8Bit)deltaIpId;
            compTcpIpHdrLength++;
        }
    }
    *flagPtr = flagBits;

/*
 * compTcpIpHdrLength contains the new TCP/IP header length.
 * Compute the amount we trimmed from our TCP/IP headers.
 */
    trimmedTcpIpHdrBytes = tcpIpHdrLength - compTcpIpHdrLength;
    if (packetPtr->pktLinkDataLength > (ttPktLen)tcpIpHdrLength)
    {
/*
 * If we are fowarding the packet, then the TCP/IP header is contiguous
 * with the data. We need to copy the the compressed TCP IP header
 * (compTcpIpHdrLength). We need to ajust the linkDataPtr by the amount
 * of bytes we trimmed (this is the gap to the rest of the data).
 */
        oldLinkDataPtr = packetPtr->pktLinkDataPtr;
/* Pointer to new linkDataPtr. */
        newLinkDataPtr = oldLinkDataPtr + trimmedTcpIpHdrBytes;
/* Copy from the end (to avoid writing over potentially overlapping header) */
        for ( i = (int)(compTcpIpHdrLength - 1);
              i >= 0;
              i-- )
        {
            newLinkDataPtr[i] = oldLinkDataPtr[i];
        }
/* New link data Pointer */
        packetPtr->pktLinkDataPtr = newLinkDataPtr;
    }
/*
 * Adjust the link and chain data lengths. Remove the number of bytes that
 * we trimmed from our TCP/IP headers.
 */
    packetPtr->pktLinkDataLength -= trimmedTcpIpHdrBytes;
    packetPtr->pktChainDataLength -= trimmedTcpIpHdrBytes;

/* Packet compressed, return to normal PPP send routine. */
    goto VjSendReturn;

VjSendUncompressed:
/* New entry - send uncompressed packet and add it to the (end of) table */
/*
 * Store only the second byte in case the protocol field is compressed
 */
/* Don't have to worry about DSP here - VJ code is disabled for DSP */
    *pktTypePtr = TM_PPP_UNCOMP_TCP_PROTOCOL;
    ipHdrPtr->iphUlp = vjDataPtr->vjsConnId;
    tm_bcopy( dataPtr,
              &(vjDataPtr->vjsIpHeader),
              ipHdrLength + tcpHdrLength);
    vjDataPtr->vjsIpHdrLength = (tt8Bit)ipHdrLength;
    vjDataPtr->vjsTcpHdrLength = (tt8Bit)tcpHdrLength;
/* Update the 'last connection ID sent' field */
    pppVectPtr->pppsLastSentConnId = vjDataPtr->vjsConnId;

VjSendReturn:
    return(retCode);
}
#endif /* !TM_DSP */

#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
/*
 * IPV6CP
 */

/*
 * tfIp6cpInit
 * Function Description
 * Initialize the IPV6CP layer.
 * . Call common initialization routine for IP6CP.
 * . Initialize the IPV6CP negotiation counter.

 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 *
 * Returns
 * None
 */
static ttVoid tfIp6cpInit (ttPppVectPtr pppVectPtr)
{
    tfIp6cpInitCommon(pppVectPtr);
/*
 * Initialize this layer's negotiation counter.  This value is set to zero when
 * the device is first opened (ie, from a tfNgOpenInterface call).  Each time
 * this layer is renegotiated *within the same session* this value is
 * incremented.
 */
    pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsNegotiateCounter = 0;
    return;
}

/*
 * tfIp6cpInitCommon
 * Function Description
 * Initialize the IPV6CP layer.
 *
 * 1. Initialize PPP state vector array with IPCP state structure and function
 *    table.
 * 2. Copy the 'requested options' into the 'negotiated options' to prepare
 *    for negotiation.
 * 3. Initialize the restart counters.

 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 *
 * Returns
 * None
 */
static ttVoid tfIp6cpInitCommon (ttPppVectPtr pppVectPtr)
{
    ttIp6cpStatePtr  ip6cpStatePtr;

/*
 * 1. Initialize PPP state vector array with IPCP state structure and function
 *    table.
 */
    ip6cpStatePtr = &pppVectPtr->pppsIp6cpState;
    pppVectPtr->pppsStateInfoPtrArray[TM_PPP_IPV6CP_INDEX] =
                                                (ttCpStatePtr)ip6cpStatePtr;
    ip6cpStatePtr->ip6cpsStateInfo.cpsStateFuncList =
            (ttPppStateFunctPtr TM_FAR *)tlIp6cpStateFunctTable;

/*
 * 2. Copy the 'requested options' into the 'negotiated options' to prepare
 *    for negotiation.
 */
    tm_bcopy(&ip6cpStatePtr->ip6cpsLocalWantOptions,
             &ip6cpStatePtr->ip6cpsLocalNegOptions,
             sizeof(ttIp6cpOptions));

/* 3. Initialize the restart counters. */
    (void)tfIp6cpInitRestartCounter(pppVectPtr, (ttPacketPtr) 0);
    return;
}

/*
 * tfIp6cpDefaultInit
 * Function Description
 * Initialize the IPV6CP layer with default configuration values.
 *
 * 1. Initialize the maximum retry counts, timeout values and maximum failure
 *    value.
 * 2. Initialize the local and remote negotiation options.  Defaults:
 *     2.1. Interface-Identifier: negotiate local option, allow remote option.
 *          ([RFC2472].R4.1:190)
 * 3. Set the IPV6CP state machine to the 'initial' state.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 *
 * Returns
 * None
 */
static ttVoid tfIp6cpDefaultInit (ttPppVectPtr pppVectPtr)
{
    ttIp6cpStatePtr   ip6cpStatePtr;
    ttCpStatePtr      pppCpStatePtr;
#ifdef TM_USE_IPHC
    ttIp6cpOptionsPtr   ip6cpOptionsPtr;
    ttLinkLayerEntryPtr devLinkLayerPtr;
    ttIphcOptionsPtr    iphcOptionsPtr;
#endif /* TM_USE_IPHC */

    ip6cpStatePtr = &pppVectPtr->pppsIp6cpState;

/*
 * 1. Initialize the maximum retry counts, timeout values and maximum failure
 *    value.
 */
    pppCpStatePtr = &ip6cpStatePtr->ip6cpsStateInfo;
    pppCpStatePtr->cpsInitTrmRetryCnt = TM_PPP_TERM_MAX_RETRY;
    pppCpStatePtr->cpsInitCfgRetryCnt = TM_PPP_CFG_MAX_RETRY;
    pppCpStatePtr->cpsRetryTimeoutSeconds = TM_PPP_RETRY_TIMEOUT;
    pppCpStatePtr->cpsInitMaxFailures = TM_PPP_MAX_FAILURES;

/*
 * 2. Initialize the local and remote negotiation options.  Defaults:
 *     2.1. Interface-Identifier: negotiate local option, allow remote option.
 *          ([RFC2472].R4.1:190)
 */
    ip6cpStatePtr->ip6cpsLocalNegotiateOption  =
        (TM_UL(1) << TM_IPV6CP_INTERFACE_ID);
    ip6cpStatePtr->ip6cpsRemoteNegotiateOption =
        (TM_UL(1) << TM_IPV6CP_INTERFACE_ID);
    ip6cpStatePtr->ip6cpsLocalWantOptions.ip6cpInterfaceId[0] =
        pppVectPtr->ppps6RandomLocalId[0];
    ip6cpStatePtr->ip6cpsLocalWantOptions.ip6cpInterfaceId[1] =
        pppVectPtr->ppps6RandomLocalId[1];

#ifdef TM_USE_IPHC
/* If RFC-2507 header compression is enabled, initialize options to default
   values .*/
    {
        devLinkLayerPtr =
                     pppVectPtr->pppsDeviceEntryPtr->devLinkLayerProtocolPtr;
        if (devLinkLayerPtr->lnkHdrCompressFuncPtr !=
                                                 (ttLnkHdrCompressFuncPtr)0)
        {
            (*(devLinkLayerPtr->lnkHdrCompressFuncPtr))
                (TM_PACKET_NULL_PTR,
                 TM_32BIT_NULL_PTR,
                 (ttVoidPtr) &pppVectPtr->pppsIphcVectPtr,
                 TM_IPHC_INIT);
             iphcOptionsPtr = &pppVectPtr->pppsIphcVectPtr->iphcOpts;
             ip6cpOptionsPtr = &ip6cpStatePtr->ip6cpsLocalWantOptions;
             ip6cpOptionsPtr->ip6cpCompTcpSpace =
                                             iphcOptionsPtr->iphcoInTcpSpace;
             ip6cpOptionsPtr->ip6cpCompNonTcpSpace =
                                             iphcOptionsPtr->iphcoInUdpSpace;
             ip6cpOptionsPtr->ip6cpCompMaxPeriod =
                                             iphcOptionsPtr->iphcoMaxPeriod;
             ip6cpOptionsPtr->ip6cpCompMaxTime = iphcOptionsPtr->iphcoMaxTime;
             ip6cpOptionsPtr->ip6cpCompMaxHeader =
                                             iphcOptionsPtr->iphcoMaxHeader;

             ip6cpOptionsPtr = &ip6cpStatePtr->ip6cpsRemoteAllowOptions;
             ip6cpOptionsPtr->ip6cpCompTcpSpace =
                                             iphcOptionsPtr->iphcoInTcpSpace;
             ip6cpOptionsPtr->ip6cpCompNonTcpSpace =
                                             iphcOptionsPtr->iphcoInUdpSpace;
             ip6cpOptionsPtr->ip6cpCompMaxPeriod =
                                             iphcOptionsPtr->iphcoMaxPeriod;
             ip6cpOptionsPtr->ip6cpCompMaxTime = iphcOptionsPtr->iphcoMaxTime;
             ip6cpOptionsPtr->ip6cpCompMaxHeader =
                                             iphcOptionsPtr->iphcoMaxHeader;

        }
    }
#endif /* TM_USE_IPHC */


/* 3. Set the IPV6CP state machine to the 'initial' state. */
    pppCpStatePtr->cpsState = TM_PPPS_INITIAL;
    return;
}

static ttVoid tfIp6cpReInit (ttPppVectPtr pppVectPtr)
{
    tfPppTimerRemove(&pppVectPtr->pppsIp6cpState.ip6cpsStateInfo);
    pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState = TM_PPPS_INITIAL;
    tfIp6cpInitCommon(pppVectPtr);
    return;
}


/*
 * tfIp6cpTimeout
 * Function Description
 * Called when an IPV6CP event (such as a Configure-Request) times out.
 *
 * 1. If the retry count has not expired, issue a timeout event to the state
 *    machine for this layer.
 * 2. If it has expired, issue a 'last timeout' event to the FSM.
 *
 * Parameters
 * Parameter    Description
 * timerArg     Pointer to the current PPP state vector.
 *
 * Returns
 * None
 */
ttVoid tfIp6cpTimeout(ttVoidPtr      timerBlockPtr,
                      ttGenericUnion userParm1,
                      ttGenericUnion userParm2)
{
    ttPppVectPtr pppVectPtr;

    TM_UNREF_IN_ARG(timerBlockPtr);
    TM_UNREF_IN_ARG(userParm2);

    pppVectPtr = (ttPppVectPtr) userParm1.genVoidParmPtr;
/*
 * 1. If the retry count has not expired, issue a timeout event to the state
 *    machine for this layer.
 * 2. If it has expired, issue a 'last timeout' event to the FSM.
 */
    tfPppTimeout( pppVectPtr,
                  &pppVectPtr->pppsIp6cpState.ip6cpsStateInfo,
                  TM_PPP_IPV6CP_INDEX );
}

/*
 * tfIp6cpSendCfgRequest
 * Function Description
 * Create an IPV6CP Configure-Request packet and sends it.
 *
 * 1. Allocate buffer for configuration request packet.
 * 2. If the peer has not previously rejected the Interface-Id option, add
 *    this option to the Conf-Req packet.  The value was randomly generated at
 *    init time or was returned to us from the peer in a Conf-Nak.
 *    [RFC2472].R4.1:40, [RFC2472].R4.1:10
 * 3. If configured, add a Compression-Protocol option.
 * 4. Set up Conf-Req/Conf-Rej header.
 * 5. Start retry timer.
 * 6. Send Conf-Req/Config-Rej to device.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed.
 *
 * Returns
 * Value        Meaning
 * 0            Conf-Req successfully sent.
 * TM_ENOBUFS   No memory available for new Conf-Req packet.
 */
int tfIp6cpSendCfgRequest(ttPppVectPtr pppVectPtr,
                          ttPacketPtr  packetPtr)
{
    ttPppCpPacketTypePtr cfgReqPtr;
    int                  errorCode;
    tt32Bit              negotiate;
    tt16Bit              temp16;
#ifdef TM_USE_IPHC
    ttIp6cpOptionsPtr    localNegOptionPtr;
    tt16Bit              param[5];
    tt8Bit               paramSize;
#endif /* TM_USE_IPHC */
#ifdef TM_DSP
    unsigned int         optionPtrOffset;
#endif /* TM_DSP */

    errorCode = TM_ENOERROR;
    tm_zero_dsp_offset(optionPtrOffset);

/* 1. Allocate buffer for configuration request packet. */
    packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                   TM_PPP_IP6CP_MAX_BYTES
                                   - TM_PPP_ALLOC_HDR_BYTES,
                                   TM_16BIT_ZERO );

    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        cfgReqPtr = (ttPppCpPacketTypePtr) packetPtr->pktLinkDataPtr;
        packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
        negotiate = pppVectPtr->pppsIp6cpState.ip6cpsLocalNegotiateOption;
/*
 * 2. If the peer has not previously rejected the Interface-Id option, add
 *    this option to the Conf-Req packet.  The value was randomly generated at
 *    init time or was returned to us from the peer in a Conf-Nak.
 *    [RFC2472].R4.1:40, [RFC2472].R4.1:10
 */
        tm_ppp_send_option(
            tfPppSendOption, packetPtr, negotiate, TM_IPV6CP_INTERFACE_ID,
            (tt8BitPtr) &pppVectPtr->pppsIp6cpState.ip6cpsLocalNegOptions.
            ip6cpInterfaceId, TM_64BIT_BYTE_COUNT, TM_8BIT_NULL_PTR, 0,
            &optionPtrOffset);

/* 3. If configured, add a Compression-Protocol option. */
#ifdef TM_USE_IPHC
        localNegOptionPtr = &pppVectPtr->pppsIp6cpState.ip6cpsLocalNegOptions;
        param[0] = htons(localNegOptionPtr->ip6cpCompTcpSpace);
        param[1] = htons(localNegOptionPtr->ip6cpCompNonTcpSpace);
        param[2] = htons(localNegOptionPtr->ip6cpCompMaxPeriod);
        param[3] = htons(localNegOptionPtr->ip6cpCompMaxTime);
        param[4] = htons(localNegOptionPtr->ip6cpCompMaxHeader);
        paramSize = 10;

        tm_ppp_send_option(
            tfPppSendOption, packetPtr, negotiate,
            TM_IPCP_COMP_PROTOCOL,
            (tt8BitPtr)&localNegOptionPtr->ip6cpCompressProtocol,
            TM_16BIT_BYTE_COUNT, (tt8BitPtr)&param[0],paramSize,
            (unsigned int *) &optionPtrOffset);
#endif /* TM_USE_IPHC */

/* 4. Set up Conf-Req/Conf-Rej header. */
        cfgReqPtr->cpCode = TM_PPP_CONFIG_REQUEST;
        cfgReqPtr->cpIdentifier =
            pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsIdentifier++;
        temp16 = (tt16Bit)
            (tm_byte_count(
                packetPtr->pktLinkDataPtr - (tt8BitPtr)cfgReqPtr));
#ifdef TM_DSP
        temp16 += optionPtrOffset;
#endif /* TM_DSP */
        packetPtr->pktLinkDataLength = (ttPktLen)temp16;
        tm_htons(temp16, cfgReqPtr->cpLength);
        packetPtr->pktLinkDataPtr = (tt8BitPtr)cfgReqPtr;

/*
 * Save the length and data of a Conf-Req; used later to verify that the
 * ACK is valid.
 */
        pppVectPtr->ppps6LastConfReqLen = (tt8Bit)(temp16 - TM_PPP_CP_BYTES);
        tm_ppp_byte_copy(packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE,
                         0,
                         pppVectPtr->ppps6LastConfReq,
                         0,
                         temp16 - TM_PPP_CP_BYTES);

/* 5. Start retry timer. */
        tfPppTimerAdd( pppVectPtr,
                       tfIp6cpTimeout,
                       &pppVectPtr->pppsIp6cpState.ip6cpsStateInfo );

/* 6. Send Conf-Req/Config-Rej to device. */
        (void) tfPppDeviceSend( pppVectPtr,
                                packetPtr,
                                TM_PPP_IPV6CP_PROTOCOL);

    }
    else
    {
        errorCode = TM_ENOBUFS;
    }

    return errorCode;
}

/*
 * tfIp6cpSendCodeReject
 * Function Description
 * Send a Code-Reject message to the peer.  Called when we receive an invalid
 * code from the peer.
 *
 * 1. Allocate new packet buffer.
 * 2. Copy old packet  to the end of our new buffer (to be returned to peer).
 * 3. Free the old packets.
 * 4. Update the new packet fields and send it.

 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed .
 *
 * Returns
 * Value        Meaning
 * 0            Code-Rej successfully sent.
 * TM_ENOBUFS   No memory available for new Code-Rej packet.
 *
 */
int tfIp6cpSendCodeReject(ttPppVectPtr  pppVectPtr,
                          ttPacketPtr   packetPtr)
{

    return tfPppSendCodeReject(pppVectPtr,
                               packetPtr,
                               &pppVectPtr->pppsIp6cpState.ip6cpsStateInfo,
                               TM_PPP_IPV6CP_PROTOCOL);
}

/*
 * tfIp6cpThisLayerUp
 * Function Description
 * The IPV6CP layer has successfully completed negotiation.
 *
 * 1. If started, remove the open failure timer.
 * 2. If the interface identifier option was successfully negotiated, set the
 *    DupAddrDetectTransmits autoconfiguration variable to zero (in
 *    ttDeviceEntry). [RFC2472].R5:20
 * 3. If the local interface identifier was successfully negotiated, call
 *    tf6SetInterfaceId to set the interface ID for this
 *    device. [RFC2472].R4.1:200
 * 4. If the remote interface identifier was successfully negotiated, create a
 *    link local IPv6 address from the identifier and copy it into
 *    ttDevice.dev6Pt2PtPeerIpAddr.  This is used to create a local route to
 *    the peer and for future retrieval by the user.
 * 5. Since PPP can trivially support multicasting set the TM_DEV_MCAST_ENB
 *    flag for this device.
 * 6. Call tf6ConfigInterfaceId to start stateless address autoconfiguration.
 * 7. If tf6ConfigInterfaceId was successful in configuring a link local
 *    address, call tf6FinishOpenInterface to finish opening the interface.
 * 8. If tf6ConfigInterfaceId cannot form a link local IPv6 address because
 *    Interface-ID option negotiation failed and the user did not manually
 *    configure an interface-ID, TM_EPERM will be returned.  Upon receiving
 *    this error, the IPV6CP layer should be closed. [RFC2472].R2:10
 * 9. If opened successfully:
 *     9.1. clear TM_6_LL_DEV_CONNECTING in dev6Flags.
 *     9.2. set TM_6_LL_DEV_CONNECTED in dev6Flags.
 *     9.3. notify the user with a TM_LL_IP6_OPEN_COMPLETE message.
 * 10. If this is the first NCP successfully opened for this session, also
 *     notify the user with a TM_LL_OPEN_COMPLETE message.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed .
 *
 * Returns
 * Value        Meaning
 * 0
 */
int tfIp6cpThisLayerUp (ttPppVectPtr    pppVectPtr,
                        ttPacketPtr     packetPtr)
{
    ttDeviceEntryPtr devPtr;
    ttIpAddressPtr   peerIpAddrPtr;
    int              errorCode;

    TM_UNREF_IN_ARG(packetPtr);

    devPtr = pppVectPtr->pppsDeviceEntryPtr;

/* Increment this layer's negotiation counter */
    pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsNegotiateCounter++;

/*
 * If this is a renegotiation, first remove the device.  This is to ensure
 * that the active interface count remains correct.
 */
    if (pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsNegotiateCounter > 1)
    {
        (void)tfRemoveInterface(devPtr, TM_16BIT_ZERO, PF_INET6);
    }

/* 1. If started, remove the open failure timer. */
    if (pppVectPtr->pppsOpenFailTimerPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(pppVectPtr->pppsOpenFailTimerPtr);
        pppVectPtr->pppsOpenFailTimerPtr = TM_TMR_NULL_PTR;
    }

/*
 * 2. If the interface identifier option was successfully negotiated, set the
 *    DupAddrDetectTransmits autoconfiguration variable to zero (in
 *    ttDeviceEntry). [RFC2472].R5:20
 * 3. If the local interface identifier was successfully negotiated, call
 *    tf6SetInterfaceId to set the interface ID for this
 *    device. [RFC2472].R4.1:200
 */
    if (pppVectPtr->pppsIp6cpState.ip6cpsLocalNegotiateOption &
            (TM_UL(1) << TM_IPV6CP_INTERFACE_ID))
    {
        devPtr->dev6DupAddrDetectTransmits = 0;
        tf6SetInterfaceId(devPtr,
                          (tt8BitPtr) &pppVectPtr->pppsIp6cpState.
                                       ip6cpsLocalGotOptions.ip6cpInterfaceId);
    }


/*
 * 4. If the remote interface identifier was successfully negotiated, create a
 *    link local IPv6 address from the identifier and copy it into
 *    ttDevice.dev6Pt2PtPeerIpAddr.  This is used to create a local route to
 *    the peer and for future retrieval by the user.
 */
    if (pppVectPtr->pppsIp6cpState.ip6cpsRemoteNegotiateOption &
            (TM_UL(1)<<TM_IPV6CP_INTERFACE_ID))
    {
        peerIpAddrPtr = &devPtr->dev6Pt2PtPeerIpAddr;
#ifdef TM_LITTLE_ENDIAN
        peerIpAddrPtr->s6LAddr[0] = TM_UL(0x000080FE);
#else /* TM_BIG_ENDIAN */
        peerIpAddrPtr->s6LAddr[0] = TM_UL(0xFE800000);
#endif /* TM_BIG_ENDIAN */
        peerIpAddrPtr->s6LAddr[1] = TM_UL(0);
        peerIpAddrPtr->s6LAddr[2] =
            pppVectPtr->pppsIp6cpState.
            ip6cpsRemoteSetOptions.ip6cpInterfaceId[0];
        peerIpAddrPtr->s6LAddr[3] =
            pppVectPtr->pppsIp6cpState.
            ip6cpsRemoteSetOptions.ip6cpInterfaceId[1];

/* Embed interface index or site ID in link-local or site-local addresses */
        tm_6_dev_scope_addr(peerIpAddrPtr, devPtr);
    }

/*
 * 5. Since PPP can trivially support multicasting set the TM_DEV_MCAST_ENB
 *    flag for this device.
 */
#ifdef TM_USE_IPV4
    devPtr->devFlag |= TM_DEV_MCAST_ENB;
#endif /* TM_USE_IPV4 */

/* 6. Call tf6ConfigInterfaceId to start stateless address autoconfiguration. */
    errorCode = tf6ConfigInterfaceId(devPtr);

/*
 * 7. If tf6ConfigInterfaceId was successful in configuring a link local
 *    address, call tf6FinishOpenInterface to finish opening the interface.
 */
    if (errorCode == TM_ENOERROR)
    {
/*
 * Get new interface identifiers, which will be used if IPV6CP renegotiates
 * or if the connection is closed and re-opened.  If one is set, we re-use
 * the current identifier (which was either set by the user, or randomly
 * generated and negotiated through IPV6CP).
 * ([RFC2472].R4.1:80, [RFC2472].R4.1:50)
 */
        if ( tm_16bit_one_bit_set(devPtr->dev6Flags,
                                  TM_6_DEV_INTERFACE_ID_FLAG) )
        {
/* If the user has set an EUI-64 identifier, use that one. */
            pppVectPtr->ppps6RandomLocalId[0] = devPtr->dev6InterfaceId[0];
            pppVectPtr->ppps6RandomLocalId[1] = devPtr->dev6InterfaceId[1];
        }
        else
        {
/* There was no identifier set by the user, so generate one randomly. */
            pppVectPtr->ppps6RandomLocalId[0] = tfGetRandom();
            pppVectPtr->ppps6RandomLocalId[1] = tfGetRandom();
/*
 * Clear the "u" bit from the local identifier, indicating that it is not unique
 * ([RFC2472].R4.1:90)
 */
            pppVectPtr->ppps6RandomLocalId[0] &= ~0x02000000UL;

        }

/*
 * Generate a random identifier for the peer.  Basing the low order 32-bits
 * of the remote identifier on the local identifier guarantees that they will
 * never match.
 */
        pppVectPtr->ppps6RandomPeerId[0]  = tfGetRandom();
        pppVectPtr->ppps6RandomPeerId[1]  = ~pppVectPtr->ppps6RandomLocalId[1];

/*
 * Clear the "u" bit from the remote identifier, indicating that it is not
 * unique.  ([RFC2472].R4.1:90)
 */
        pppVectPtr->ppps6RandomPeerId[0]  &= ~0x02000000UL;


        errorCode =
            tf6FinishOpenInterface(devPtr,
                                   devPtr->dev6LinkLocalMhomeIndex);
    }
    else
    {

/*
 * 8. If tf6ConfigInterfaceId cannot form a link local IPv6 address because
 *    Interface-ID option negotiation failed and the user did not manually
 *    configure an interface-ID, TM_EPERM will be returned.  Upon receiving
 *    this error, the IPV6CP layer should be closed. [RFC2472].R2:10
 */
        tfPppNcpOpenFail(pppVectPtr);
    }

    if (errorCode == TM_ENOERROR)
    {
/*
 * 9. If opened successfully:
 *     9.1. clear TM_6_LL_DEV_CONNECTING in dev6Flags.
 *     9.2. set TM_6_LL_DEV_CONNECTED in dev6Flags.
 *     9.3. notify the user with a TM_LL_IP6_OPEN_COMPLETE message.
 * 10. If this is the first NCP successfully opened for this session, also
 *     notify the user with a TM_LL_OPEN_COMPLETE message.
 */
        tm_16bit_clr_bit( devPtr->dev6Flags, TM_6_LL_CONNECTING );
        devPtr->dev6Flags |= TM_6_LL_CONNECTED;

        tfPppUserNotify(devPtr, TM_LL_IP6_OPEN_COMPLETE);

/* Remove the open failure timer since at least one NCP is up */
        if (pppVectPtr->pppsOpenFailTimerPtr != TM_TMR_NULL_PTR)
        {
            tm_timer_remove(pppVectPtr->pppsOpenFailTimerPtr);
            pppVectPtr->pppsOpenFailTimerPtr = TM_TMR_NULL_PTR;
        }

#ifdef TM_USE_IPV4
        if (pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState != TM_PPPS_OPENED)
#endif /* TM_USE_IPV4 */
        {
            tfPppUserNotify(devPtr, TM_LL_OPEN_COMPLETE);
        }
    }


    return errorCode;
}

/*
 * tfIp6cpThisLayerFinish
 * Function Description
 * The IPV6CP layer is finished (closed).  Notify the user and (possibly)
 * close the link.
 *
 * 1. If the current IPV6CP state is 'stopped', this indicates that the
 *    initial negotiation has failed.
 *     1.1. Notify the user with a TM_LL_IP6_OPEN_FAILED message.
 *     1.2. Call tfPppNcpOpenFail to process this failure.
 * 2. Otherwise, if IPV6CP is not in the 'stopped' state, IPV6CP has been
 *    closed some time after negotiation has completed.  We may need to close
 *    the link (issue CLOSE event to LCP) if:
 *     2.1. The stack was built in IPv6 only mode, OR
 *     2.2. The stack was built in dual-stack mode and IPCP is not currently
 *          open.
 * 3. Clear TM_6_LL_CONNECTED and TM_6_LL_CONNECTING from dev6Flags.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed .
 *
 * Returns
 * Value        Meaning
 * 0            IPV6CP layer successfully closed.
 */
int tfIp6cpThisLayerFinish (ttPppVectPtr pppVectPtr,
                            ttPacketPtr  packetPtr)
{

    TM_UNREF_IN_ARG(packetPtr);

/*
 * 1. If the current IPV6CP state is 'stopped', this indicates that the
 *    initial negotiation has failed.
 *     1.1. Notify the user with a TM_LL_IP6_OPEN_FAILED message.
 *     1.2. Call tfPppNcpOpenFail to process this failure.
 */
    if (pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState == TM_PPPS_STOPPED)
    {
        tfPppUserNotify(pppVectPtr->pppsDeviceEntryPtr, TM_LL_IP6_OPEN_FAILED);
        tfPppNcpOpenFail(pppVectPtr);
    }
    else
    {
/*
 * 2. Otherwise, if IPV6CP is not in the 'stopped' state, IPV6CP has been
 *    closed some time after negotiation has completed.  We may need to close
 *    the link (issue CLOSE event to LCP) if:
 *     2.1. The stack was built in IPv6 only mode, OR
 *     2.2. The stack was built in dual-stack mode and IPCP is not currently
 *          open.
 */
#ifdef TM_USE_IPV4
        if (pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState ==
                TM_PPPS_INITIAL)
#endif /* TM_USE_IPV4 */
        {
            (void)tfPppStateMachine(pppVectPtr,
                                    TM_PACKET_NULL_PTR,
                                    TM_PPPE_CLOSE,
                                    TM_PPP_LCP_INDEX);
        }

/* 3. Clear TM_6_LL_CONNECTED and TM_6_LL_CONNECTING from dev6Flags. */
        tm_16bit_clr_bit( pppVectPtr->pppsDeviceEntryPtr->dev6Flags,
                          (TM_6_LL_CONNECTED | TM_6_LL_CONNECTING) );

#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
        tm_kernel_set_critical;
        tm_context(tvIpData).ipv6InterfaceTableLastChange =
            tm_snmp_time_ticks(tvTime);
        tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
    }

    tfPppTimerRemove(&pppVectPtr->pppsIp6cpState.ip6cpsStateInfo);
    return TM_ENOERROR;
}

/*
 * tfIp6cpThisLayerStart
 * Function Description
 * Starts the IPV6CP layer.  Called when a lower PPP layer (either LCP or
 * authentication) opens IPV6CP.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed .
 *
 * Returns
 * Value        Meaning
 * 0            IPV6CP layer successfully started.
 */
int tfIp6cpThisLayerStart(ttPppVectPtr  pppVectPtr,
                          ttPacketPtr   packetPtr)
{
    return tfPppStateMachine(pppVectPtr, packetPtr, TM_PPPE_UP,
                             TM_PPP_IPV6CP_INDEX);
}

/*
 * tfIp6cpThisLayerDown
 * Function Description
 * The IPV6CP layer has gone down, possibly due to an impending
 * re-negotiation.  Unconfigure the interface ID for this device, if set.
 *
 * 1. If the Interface-ID option was negotiated, call tf6UnConfigInterfaceId
 *    to unconfigure the interface ID.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed .
 *
 * Returns
 * Value        Meaning
 * 0            IPV6CP layer gone down successfully.
 */
int tfIp6cpThisLayerDown(ttPppVectPtr   pppVectPtr,
                         ttPacketPtr    packetPtr)
{
    int errorCode;

    TM_UNREF_IN_ARG(packetPtr);

    errorCode = TM_ENOERROR;
/*
 * 1. If the Interface-ID option was negotiated, call tf6UnConfigInterfaceId
 *    to unconfigure the interface ID.
 */
    if (pppVectPtr->pppsIp6cpState.ip6cpsLocalNegotiateOption &
            (TM_UL(1) << TM_IPV6CP_INTERFACE_ID))
    {
        errorCode = tf6UnConfigInterfaceId(pppVectPtr->pppsDeviceEntryPtr);
    }

    return errorCode;
}

/*
 * tfIp6cpSendCfgNak
 * Function Description
 * Send a Configure-NAK or Configure-REJ to the peer.  The packet has already
 * been built on input processing so this packet must just be sent to the
 * device.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed .
 *
 * Returns
 * Value        Meaning
 * 0            Conf-Nak or Conf-Rej successfully sent.
 * TM_ENOBUFS   No memory available for new Conf-Nak or Conf-Rej packet.
 */
int tfIp6cpSendCfgNak(ttPppVectPtr      pppVectPtr,
                      ttPacketPtr       packetPtr)
{
    (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_PPP_IPV6CP_PROTOCOL);

    return TM_ENOERROR;
}

/*
 * tfIp6cpInitRestartCounter
 * Function Description
 * Initialize the IPV6CP configure retry and max failure counters.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed .
 *
 * Returns
 * Value        Meaning
 * 0            Counters initialized successfully.
 */
int tfIp6cpInitRestartCounter(ttPppVectPtr      pppVectPtr,
                              ttPacketPtr       packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);

    tfPppInitRestartCounter(&pppVectPtr->pppsIp6cpState.ip6cpsStateInfo);

    return TM_ENOERROR;
}

/*
 * tfIp6cpZeroRestartCounter
 * Function Description
 * Zero the IPV6CP retry counter.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed.
 *
 * Returns
 * Value        Meaning
 * 0            Counters zero'ed successfully.
 */
int tfIp6cpZeroRestartCounter(ttPppVectPtr      pppVectPtr,
                              ttPacketPtr       packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
    pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsRetryCounter = 0;
    return TM_ENOERROR;
}

/*
 * tfIp6cpIncomingPacket
 * Function Description
 * Processes an incoming IPV6CP packet.
 *
 * 1. Get length and code from IPV6CP header.
 * 2. Switch based on packet type (code).
 *     2.1. Conf-Req, Conf-Nak, Conf-Rej:
 *              Parse packet, determine appropriate event and form response
 *              packet (if any).
 *     2.2. Conf-Ack:
 *              Remote peer acknowledged our options, so we can start using
 *              the currently negotiated options.
 *     2.3. Term-Ack, Term-Req,Code-Rej,Echo-Req:
 *              set appropriate state machine event.
 *     2.4. Echo-Rep, Disc-Req:
 *              Silently discard incoming packet.
 *     2.5. Unknown: Set state machine event for unknown code event.
 * 3. Free IPV6CP timer, if needed.
 * 4. Free incoming packet, if the buffer is not being used for a response
 *    packet.
 * 5. Issue event (determined above) to the IPV6CP state machine.
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed .
 *
 * Returns
 * Value        Meaning
 * 0            Incoming packet processed successfully.
 */
int tfIp6cpIncomingPacket (ttPppVectPtr pppVectPtr,
                           ttPacketPtr  packetPtr)
{
    ttPppCpPacketTypePtr ip6cpHdrPtr;
    int                  isEqual;
    int                  errorCode;
    tt16Bit              optionsLength;
    tt8Bit               event;
    tt8Bit               needTimerRemove;
    tt8Bit               needFreePacket;
    tt8Bit               validAck;

    errorCode = TM_ENOERROR;
    event     = TM_PPPE_NO_EVENT;
    needTimerRemove = TM_8BIT_ZERO;
    needFreePacket  = TM_8BIT_ZERO;

/* 1. Get length and code from IPV6CP header. */
    ip6cpHdrPtr = (ttPppCpPacketTypePtr) packetPtr->pktLinkDataPtr;
    tm_ntohs(ip6cpHdrPtr->cpLength, optionsLength);

/* Verify that the length from the IPV6CP header is not greater than the actual
   length of the packet. */
    if ((ttPktLen)optionsLength > packetPtr->pktChainDataLength)
    {
        needFreePacket = TM_8BIT_YES;
        goto Ip6cpIncomingPacketFinish;
    }

/* 2. Switch based on packet type (code). */
    switch (ip6cpHdrPtr->cpCode)
    {

/*
 *     2.1. Conf-Req, Conf-Nak, Conf-Rej:
 *              Parse packet, determine appropriate event and form response
 *              packet (if any).
 */
        case TM_PPP_CONFIG_REQUEST:
            packetPtr = tfIp6cpParseConfig(pppVectPtr,
                                           packetPtr,
                                           optionsLength,
                                           &event);
            break;

        case TM_PPP_CONFIG_NAK:
            if (tm_8bit_cur_id(ip6cpHdrPtr->cpIdentifier,
                    pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsIdentifier))
            {
                errorCode = tfIp6cpParseNak(pppVectPtr,
                                            packetPtr,
                                            optionsLength);
/* Packet freed in tfIp6cpParseNak */
                packetPtr       = TM_PACKET_NULL_PTR;

                if (errorCode == TM_ENOERROR)
                {
                    needTimerRemove = TM_8BIT_YES;
                    event           = TM_PPPE_CFG_NAK;
                }
                else
                {
/* Invalid NAK - ignore packet. */
                    needTimerRemove = TM_8BIT_ZERO;
                    event           = TM_PPPE_NO_EVENT;
                }
            }
            else
            {
/* Old Conf-Nak packet (bad identifier) - discard */
                needFreePacket  = TM_8BIT_YES;
                needTimerRemove = TM_8BIT_ZERO;
                event           = TM_PPPE_NO_EVENT;
            }
            break;

        case TM_PPP_CONFIG_REJECT:
            if (tm_8bit_cur_id(ip6cpHdrPtr->cpIdentifier,
                    pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsIdentifier))
            {
                errorCode = tfIp6cpParseReject(pppVectPtr,
                                               packetPtr,
                                               optionsLength);
/* Packet free in tfIp6cpParseReject */
                packetPtr       = TM_PACKET_NULL_PTR;
                if (errorCode == TM_ENOERROR)
                {
                    needTimerRemove = TM_8BIT_YES;
                    event           = TM_PPPE_CFG_NAK;
                }
                else
                {
/* Invalid REJ - ignore packet. */
                    needTimerRemove = TM_8BIT_ZERO;
                    event           = TM_PPPE_NO_EVENT;
                }
            }
            else
            {
/* Old Conf-Rej packet (bad identifier) - discard */
                needFreePacket  = TM_8BIT_YES;
                needTimerRemove = TM_8BIT_ZERO;
                event           = TM_PPPE_NO_EVENT;
            }
            break;

/*
 *     2.2. Conf-Ack:
 *              Remote peer acknowledged our options, so we can start using
 *              the currently negotiated options.
 */
        case TM_PPP_CONFIG_ACK:
            validAck = TM_8BIT_YES;
/*
 * Verify that the identifier on this Conf-ACK matches the last Conf-REQ
 * that was sent (RFC1661, section 5.2).
 */
            if (!tm_8bit_cur_id(ip6cpHdrPtr->cpIdentifier,
                    pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsIdentifier))
            {
                validAck = TM_8BIT_ZERO;
            }
            else
            {
/*
 * Verify that the length of the Conf-ACK is correct and the options are
 * identical to those sent in our last Conf-REQ.
 */
                if (optionsLength !=
                        ((tt16Bit)pppVectPtr->ppps6LastConfReqLen +
                         (tt16Bit)(TM_PPP_CP_BYTES)))
                {
                    validAck = TM_8BIT_ZERO;
                }
                else
                {
                    isEqual = tm_ppp_memcmp(
                              packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE,
                              pppVectPtr->ppps6LastConfReq,
                              pppVectPtr->ppps6LastConfReqLen);
                    if( isEqual != TM_STRCMP_EQUAL )
                    {
                        validAck = TM_8BIT_ZERO;
                    }
                }
            }
            if (validAck == TM_8BIT_YES)
            {
#ifdef TM_USE_IPHC
/* If RFC-2507 IP header compression is being used and if operating in dual
   IPv4/IPv6 mode, and IPV6CP is up we need to merge the two sets of
   configuration values (IPCP & IPV6CP): */
                tfPppSetIphcValues(pppVectPtr,
                                   pppVectPtr->pppsIp6cpState.
                                     ip6cpsLocalNegOptions.ip6cpCompressProtocol,
                                   AF_INET6);
#endif /* TM_USE_IPHC */

/* Our options have been acked, so save them and start using them. */
                tm_bcopy(&(pppVectPtr->pppsIp6cpState.ip6cpsLocalNegOptions),
                         &(pppVectPtr->pppsIp6cpState.ip6cpsLocalGotOptions),
                         sizeof(ttIp6cpOptions));
                needTimerRemove = TM_8BIT_YES;
                needFreePacket  = TM_8BIT_YES;
                event           = TM_PPPE_CFG_ACK;
            }
            else
            {
/* Invalid Conf-Ack packet - discard */
                needFreePacket  = TM_8BIT_YES;
                needTimerRemove = TM_8BIT_ZERO;
                event           = TM_PPPE_NO_EVENT;
            }
            break;

/*
 *     2.3. Term-Ack, Term-Req,Code-Rej,Echo-Req:
 *              set appropriate state machine event.
 */
        case TM_PPP_TERM_ACK:
            needFreePacket  = TM_8BIT_YES;
            needTimerRemove = TM_8BIT_YES;
            event           = TM_PPPE_TRM_ACK;
            break;

        case TM_PPP_TERM_REQUEST:
            needTimerRemove = TM_8BIT_YES;
            event           = TM_PPPE_TRM_REQ;
            break;

        case TM_PPP_CODE_REJECT:
            needFreePacket = TM_8BIT_YES;
/*
 * The peer rejected one of our codes. Since we don't send any optional codes,
 * close the connection.
 */
            event          = TM_PPPE_CLOSE;
            break;

/*     2.5. Unknown: Set state machine event for unknown code event. */
        default:
            event = TM_PPPE_UNKNOWN_CODE;
            break;
    }

Ip6cpIncomingPacketFinish:

/* 3. Free IPV6CP timer, if needed. */
    if (needTimerRemove != TM_8BIT_ZERO)
    {
        tfPppTimerRemove(&pppVectPtr->pppsIp6cpState.ip6cpsStateInfo);
    }

/*
 * 4. Free incoming packet, if the buffer is not being used for a response
 *    packet.
 */
    if (needFreePacket != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        packetPtr = TM_PACKET_NULL_PTR;
    }

/* 5. Issue event (determined above) to the IPV6CP state machine. */
    if (event != TM_PPPE_NO_EVENT)
    {
        errorCode = tfPppStateMachine(pppVectPtr,
                                      packetPtr,
                                      event,
                                      TM_PPP_IPV6CP_INDEX);
    }

    return errorCode;
}

/*
 * tfIp6cpSendTermAck
 * Function Description
 * Send a IPV6CP Terminate-Ack.  In the IPV6CP layer, this is rarely used to
 * acknowledge a Terminate-Req as it is in the LCP layer.  Rather, it causes
 * our peer to renegotiate the IPV6CP options (because we are in a stopped
 * state, for instance).
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed.
 *
 * Returns
 * Value        Meaning
 * 0            Term-Ack successfully sent.
 * TM_ENOBUFS   No memory available for new Term-Ack packet.
 */
int tfIp6cpSendTermAck (ttPppVectPtr    pppVectPtr,
                        ttPacketPtr     packetPtr)
{
    ttPppCpPacketTypePtr termAckPtr;
    int                  errorCode;

    errorCode = TM_ENOERROR;

    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                       TM_PPP_IP6CP_MAX_BYTES
                                       - TM_PPP_ALLOC_HDR_BYTES,
                                       TM_16BIT_ZERO );
        if (packetPtr != TM_PACKET_NULL_PTR)
        {
            termAckPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
            termAckPtr->cpCode = TM_PPP_TERM_ACK;
            termAckPtr->cpIdentifier =
                pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsIdentifier++;
            packetPtr->pktLinkDataLength = TM_PPP_CP_BYTES;
            termAckPtr->cpLength = tm_const_htons(TM_PPP_CP_BYTES);

            (void) tfPppDeviceSend( pppVectPtr,
                                    packetPtr,
                                    TM_PPP_IPV6CP_PROTOCOL );
        }
        else
        {
            errorCode = TM_ENOBUFS;
        }

    }
    else
    {
        termAckPtr = (ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr;
        termAckPtr->cpCode = TM_PPP_TERM_ACK;
/*
 * Because we are reusing the input packet, we pass a type of zero to tell
 * the linkLayer to just update the pointers to the header
 */
         (void) tfPppDeviceSend(pppVectPtr, packetPtr, TM_16BIT_ZERO);
    }

    if (errorCode == TM_ENOERROR)
    {
        if (pppVectPtr->pppsStateInfoPtrArray[TM_PPP_IPV6CP_INDEX]->
            cpsState > TM_PPPS_STOPPED)
        {
            pppVectPtr->pppsStateInfoPtrArray[TM_PPP_IPV6CP_INDEX]->
                cpsState = TM_PPPS_INITIAL;
        }
        (void)tfIp6cpInitRestartCounter(pppVectPtr, packetPtr);
    }

    return errorCode;
}

/*
 * tfIp6cpParseNak
 * Function Description
 * Parse and process an IPV6CP Configure-Nak sent from our peer.  A Nak
 * indicates that the peer supports an option that we were trying to
 * negotiate, but did not accept the value of the option and is now suggesting
 * new values.
 *
 * 1. For each option in Conf-Nak, switch based on option type:
 * 2. Interface-Identifier:
 *     2.1. If the value in the Conf-Nak is identical to value that we
 *          requested or is set to zero, treat this as a reject and disable
 *          local negotiation of this option.
 *     2.2. Otherwise, save this value as the current tentative interface
 *          identifier.  When this function returns the state machine will
 *          cause a Conf-Req containing this value to be sent.
 *          [RFC2472].R4.1:150, [RFC2472].R4.1:130
 *
 * Parameters
 * Parameter     Description
 * pppVectPtr    Pointer to the current PPP state vector.
 * packetPtr     Pointer to the current packet being processed .
 * optionsLength Length of options.
 *
 * Returns
 * None
 */
int tfIp6cpParseNak (ttPppVectPtr    pppVectPtr,
                     ttPacketPtr     packetPtr,
                     tt16Bit         optionsLength)
{
    ttIp6cpOptionsPtr localOptionsPtr;
    tt8BitPtr         optionValuePtr;
    tt8BitPtr         dataPtr;
    tt32Bit           interfaceId[2];
    tt32BitPtr        localInterfaceIdPtr;
    tt8Bit            optionType;
    tt8Bit            optionTotalLength;
    tt8Bit            optionLength;
    int               errorCode;
#ifdef TM_USE_IPHC
    ttIp6cpOptionsPtr ip6cpLocalNegOptionsPtr;
    tt16Bit           compressProto;
    tt16Bit           iphcOpts[5];
#endif /* TM_USE_IPHC */
#ifdef TM_DSP
    unsigned int        dataOffset;
    unsigned int        valueDataOffset;
#endif /* TM_DSP */

    errorCode = TM_ENOERROR;

    tm_zero_dsp_offset(dataOffset);
    localOptionsPtr = &pppVectPtr->pppsIp6cpState.ip6cpsLocalNegOptions;
    packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
    optionsLength -= TM_PPP_CP_BYTES;
    dataPtr = packetPtr->pktLinkDataPtr;


/* 1. For each option in Conf-Nak, switch based on option type: */
    while ((ttS16Bit)optionsLength > 0)
    {
        tm_set_dsp_offset(valueDataOffset, dataOffset);
        optionValuePtr = dataPtr;

/* Get option type and length */
        optionType = tm_ppp_get_char_next(optionValuePtr, valueDataOffset);
        optionTotalLength = tm_ppp_get_char_next(optionValuePtr,
                                                 valueDataOffset);

/* Get the length of the option data (without length and type fields) */
        optionLength =
            (tt8Bit) (optionTotalLength - TM_8BIT_BYTE_COUNT
                      - TM_8BIT_BYTE_COUNT);

        switch (optionType)
        {

/* 2. Interface-Identifier: */
            case TM_IPV6CP_INTERFACE_ID:
                if (optionLength != TM_64BIT_BYTE_COUNT)
                {
/* Bad option length - ignore packet. */
                    errorCode = TM_EINVAL;
                    goto ip6cpParseNakFinish;
                }
                tm_ppp_byte_copy(optionValuePtr, valueDataOffset,
                                 &interfaceId[0], 0, TM_32BIT_BYTE_COUNT);
                tm_ppp_byte_copy(optionValuePtr + sizeof(tt32Bit),
                                 valueDataOffset,
                                 &interfaceId[1], 0, TM_32BIT_BYTE_COUNT);
/*
 *     2.1. If the value in the Conf-Nak is identical to value that we
 *          requested or is set to zero, treat this as a reject and disable
 *          local negotiation of this option.
 */
                localInterfaceIdPtr = localOptionsPtr->ip6cpInterfaceId;
                if (   (   (interfaceId[0] == 0UL)
                        && (interfaceId[1] == 0UL))
                    || (   (interfaceId[0] == localInterfaceIdPtr[0])
                        && (interfaceId[1] == localInterfaceIdPtr[1])))
                {
                    pppVectPtr->pppsIp6cpState.ip6cpsLocalNegotiateOption &=
                        ~(TM_UL(1) << TM_IPV6CP_INTERFACE_ID);
                }
                else
                {
/*
 *     2.2. Otherwise, save this value as the current tentative interface
 *          identifier.  When this function returns the state machine will
 *          cause a Conf-Req containing this value to be sent.
 *          [RFC2472].R4.1:150, [RFC2472].R4.1:130
 */
                    localInterfaceIdPtr[0] = interfaceId[0];
                    localInterfaceIdPtr[1] = interfaceId[1];
                }
                break;

            case TM_IPV6CP_COMP_PROTOCOL:
#ifdef TM_USE_IPHC
                tm_bcopy( optionValuePtr,
                          &compressProto,
                          sizeof(tt16Bit));
                if (compressProto == TM_PPP_IPHC_PROTOCOL)
                {
                    if (optionLength < TM_PPP_IPHC_MIN_OPT_LEN)
                    {
/* Bad option length - ignore packet. */
                        errorCode = TM_EINVAL;
                        goto ip6cpParseNakFinish;
                    }
                    else
                    {
                        if (   pppVectPtr->pppsIphcVectPtr == (ttIphcVectPtr) 0)
                        {
/* IPHC is not enabled, so treat this NAK as a reject. */
                            pppVectPtr->pppsIp6cpState.
                                ip6cpsLocalNegotiateOption &=
                                ~(TM_UL(1)<< optionType);
                        }
                        else
                        {
                            ip6cpLocalNegOptionsPtr =
                                &pppVectPtr->pppsIp6cpState.
                                ip6cpsLocalNegOptions;
                            ip6cpLocalNegOptionsPtr->ip6cpCompressProtocol =
                                compressProto;
                            optionValuePtr += sizeof(tt16Bit);

                            tm_bcopy(optionValuePtr,
                                     iphcOpts,
                                     sizeof(iphcOpts));
/* Only NAK if the peer requests a different sized compression space; leave
   all other options as is. */
                            ip6cpLocalNegOptionsPtr->ip6cpCompTcpSpace =
                                ntohs(iphcOpts[0]);
                            ip6cpLocalNegOptionsPtr->ip6cpCompNonTcpSpace =
                                ntohs(iphcOpts[1]);
                            ip6cpLocalNegOptionsPtr->ip6cpCompMaxPeriod =
                                ntohs(iphcOpts[2]);
                            ip6cpLocalNegOptionsPtr->ip6cpCompMaxTime =
                                ntohs(iphcOpts[3]);
                            ip6cpLocalNegOptionsPtr->ip6cpCompMaxHeader =
                                ntohs(iphcOpts[4]);
                        }
                    }
                }
                else
#endif /* TM_USE_IPHC */
                {
/* Unrecognized compression protocol type, treat as a reject. */
                    pppVectPtr->pppsIp6cpState.
                        ip6cpsLocalNegotiateOption &= ~(TM_UL(1)<< optionType);
                }
                break;

/* Unrecognized option - since this is a Config-Nak, all options should always
   be recognized (since this is in response to our own Config-Req) so ignore
   this packet. */
            default:
                errorCode = TM_EINVAL;
                goto ip6cpParseNakFinish;

        }

        tm_ppp_move_next_option(dataPtr, optionTotalLength, dataOffset);

        optionsLength = (tt16Bit)(optionsLength - optionTotalLength);
    }

ip6cpParseNakFinish:

    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);

    return errorCode;
}


/*
 * tfIp6cpParseReject
 * Function Description
 * Parse and process an IPV6CP Configure-Rej sent from our peer.  A reject
 * indicates that our peer does not support an option that we were trying to
 * negotiate.
 *
 * 1. For each option in Conf-Rej, disable local negotiation of this option
 *    since the peer rejected our request for this option. [RFC2472].R4.1:140
 *
 * Parameters
 * Parameter     Description
 * pppVectPtr    Pointer to the current PPP state vector.
 * packetPtr     Pointer to the current packet being processed .
 * optionsLength Total length of the options in this packet
 *
 * Returns
 * None
 */
int tfIp6cpParseReject (ttPppVectPtr pppVectPtr,
                        ttPacketPtr  packetPtr,
                        tt16Bit      optionsLength)
{
    tt8BitPtr       dataPtr;
    tt8BitPtr       optionValuePtr;
    tt8Bit          optionType;
    tt8Bit          optionTotalLength;
    tt8Bit          optionLength;
    tt32Bit         localNegOption;
    int             errorCode;
#ifdef TM_DSP
    unsigned int    dataOffset;
    unsigned int    valueDataOffset;
#endif /* TM_DSP */

    tm_zero_dsp_offset(dataOffset);
    packetPtr->pktLinkDataPtr += TM_PAK_PPP_CP_SIZE;
    dataPtr = packetPtr->pktLinkDataPtr;
    optionsLength -= TM_PPP_CP_BYTES;
    localNegOption = pppVectPtr->pppsIp6cpState.ip6cpsLocalNegotiateOption;
    errorCode = TM_ENOERROR;
    while ( ((ttS16Bit)optionsLength > 0) && (errorCode == TM_ENOERROR) )
    {
/*
 * inside the while loop, use optionValuePtr rather than dataPtr
 * and use valueDataOffset in loop, leave dataOffset untouched each loop
 */
        tm_set_dsp_offset(valueDataOffset, dataOffset);

        optionValuePtr = dataPtr;
        optionType = tm_ppp_get_char_next(optionValuePtr, valueDataOffset);

        optionTotalLength = tm_ppp_get_char_next(optionValuePtr,
                                                 valueDataOffset);

/* Get the length of the option data (without length and type fields) */
        optionLength =
            (tt8Bit) (optionTotalLength - TM_8BIT_BYTE_COUNT
                      - TM_8BIT_BYTE_COUNT);

        switch (optionType)
        {
            case TM_IPV6CP_INTERFACE_ID:
                if (optionLength != TM_64BIT_BYTE_COUNT)
                {
                    errorCode = TM_EINVAL;
                }
                break;

#ifdef TM_USE_IPHC
            case TM_IPV6CP_COMP_PROTOCOL:
                if (optionLength < TM_PPP_IPHC_MIN_OPT_LEN)
                {
                    errorCode = TM_EINVAL;
                }
                break;
#endif /* TM_USE_IPHC */

/* Unrecognized option - since this is a Config-Rej, all options should always
   be recognized (since this is in response to our own Config-Req) so ignore
   this packet. */
            default:
                errorCode = TM_EINVAL;
                break;
        }

        if (optionType < (tt8Bit)32)
        {
            localNegOption &= ~(TM_UL(1)<< optionType);
        }

        tm_ppp_move_next_option(dataPtr, optionTotalLength, dataOffset);

        optionsLength = (tt16Bit)(optionsLength - optionTotalLength);
    }

/* No problems found in Conf-Rej packet, so record the results. */
    if (errorCode == TM_ENOERROR)
    {
        pppVectPtr->pppsIp6cpState.ip6cpsLocalNegotiateOption = localNegOption;
    }

    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);

    return errorCode;
}

/*
 * tfIp6cpParseConfig
 * Function Description
 * Parse and process an IPV6CP Configure-Req sent from our peer.  We will
 * generate a Ack, Nak or Reject in response.
 *
 * 1. For each option in request:
 * 2. Check that the length of the option is valid (e.g., 8 bytes for
 *    Interface-Identifier option).  If length is bad, send Conf-Rej packet to
 *    peer.
 * 3. Interface-Identifier: ([RFC2472].R4.1:110)
 *     3.1.Compare the Interface-Identifier option that the peer is requesting
 *         with our own (tentative) interface identifier.
 *     3.2. If the identifiers are different and the peer's identifier is
 *          zero, send a Conf-Nak with a suggested interface identifier.  This
 *          identifier MUST be different than our own.  This value will either
 *          be set by the user or will be generated randomly when this PPP
 *          vector is initialized.
 *     3.3. If they are different and the peer's Interface-Identifier is not
 *          zero, acknowledge this option.
 *     3.4. If the two identifiers are equal and non-zero, send a Conf-Nak
 *          with a suggested identifier to be used by the peer.  This value
 *          must be non-zero and different from the current interface
 *          identifier. [RFC2472].R4.1:20]
 *     3.5. If both the peer's interface identifier and our local identifier
 *          are zero, send a Conf-Rej for this option and disable any future
 *          negotiation of this option.
 * 4. IPv6-Compression-Protocol:
 *     4.1. If the option length (excluding the type and length fields) is
 *          less than 12 or no compression protocol has been configured, send
 *          a Conf-Rej to the peer.
 *     4.2. If all of the requested values (TCP_SPACE, F_MAX_TIME, etc) match,
 *          save the values and send a Conf-Ack.
 *     4.3. Otherwise, Nak with our preferred option values.
 * 5. Other options: We currently only support the Interface-Identifier
 *    option, so all other option types should result in a Conf-Rej sent to
 *    the peer.
 * 6. If the request does not include an Interface-Identifier option and
 *    negotiation of this option has not been disabled (either by the user or
 *    through a Conf-Rej from the peer), we should send a Conf-Nak to the peer
 *    which includes a suggested identifier value.  This must only be done
 *    once in the course of an IPV6CP negotiation.  If we have already sent a
 *    Conf-Nak in this manner and are receiving a second Conf-Req without the
 *    Interface-Identifier, we should now assume that the peer cannot
 *    negotiate this option. [RFC2472].R4.1:170, [RFC2472].R4.1:180
 *
 * Parameters
 * Parameter    Description
 * pppVectPtr   Pointer to the current PPP state vector.
 * packetPtr    Pointer to the current packet being processed .
 * length       Total length of the options in this packet
 * pppEventPtr  Set to the state machine event to execute upon returning.
 *
 * Returns
 * Value        Meaning
 * ttPacketPtr
 *
 */
ttPacketPtr tfIp6cpParseConfig (ttPppVectPtr    pppVectPtr,
                                ttPacketPtr     packetPtr,
                                tt16Bit         length,
                                tt8BitPtr       pppEventPtr)
{
    tt8BitPtr            dataPtr;
    tt8BitPtr            optionValuePtr;
    ttIp6cpStatePtr      ip6cpStatePtr;
    tt32BitPtr           localInterfaceIdPtr;
    tt32Bit              interfaceId[2];
    tt32Bit              remoteOptionsType;
    tt8Bit               optionType;
    tt8Bit               optionLength;
    tt8Bit               optionTotalLength;
    tt8Bit               needSendRej;
    tt8Bit               needSendNak;
    tt8Bit               ident;
    ttPacketPtr          rejPacketPtr;
    ttPacketPtr          nakPacketPtr;
    ttPacketPtr          retPacketPtr;
    tt8BitPtr            outDataPtr;
    ttIp6cpOptions       remoteOptions;
    tt8Bit               tempIdData[2];
#ifdef TM_USE_IPHC
    ttIp6cpOptionsPtr    ip6cpRemoteAllowOptionsPtr;
    tt16Bit              reqCompProto;
    tt16Bit              iphcOpts[6];
#endif /* TM_USE_IPHC */
#ifdef TM_DSP
    unsigned int         inDataOffset;
    unsigned int         outDataOffset;
    unsigned int         valueDataOffset;
#endif /* TM_DSP */

    tm_zero_dsp_offset(inDataOffset);
    tm_zero_dsp_offset(outDataOffset);
    outDataPtr = (tt8BitPtr)0;
    length -= TM_PPP_CP_BYTES;
    ident = ((ttPppCpPacketTypePtr)packetPtr->pktLinkDataPtr)->cpIdentifier;
    dataPtr = packetPtr->pktLinkDataPtr + TM_PAK_PPP_CP_SIZE;
    ip6cpStatePtr = &pppVectPtr->pppsIp6cpState;
    nakPacketPtr = TM_PACKET_NULL_PTR;
    rejPacketPtr = TM_PACKET_NULL_PTR;
    retPacketPtr = TM_PACKET_NULL_PTR; /* assume failure */
    remoteOptionsType = TM_UL(0);
    tm_bzero(&remoteOptions, sizeof(remoteOptions));

/* 1. For each option in request: */
    while ((ttS16Bit)length > 0)
    {
        needSendNak = TM_8BIT_ZERO;
        needSendRej = TM_8BIT_ZERO;
        tm_set_dsp_offset(valueDataOffset, inDataOffset);
        optionValuePtr = dataPtr;
        optionType = tm_ppp_get_char_next(optionValuePtr, valueDataOffset);
        optionTotalLength = tm_ppp_get_char_next(optionValuePtr,
                                                 valueDataOffset);
        optionLength = (tt8Bit)(   optionTotalLength - TM_8BIT_BYTE_COUNT
                                 - TM_8BIT_BYTE_COUNT );
/*
 * 2. Check that the length of the option is valid (e.g., 8 bytes for
 *    Interface-Identifier option).  If length is bad, send Conf-Rej packet to
 *    peer.
 */
        if (optionType < 32)
        {
            if ( ip6cpStatePtr->ip6cpsRemoteNegotiateOption &
                 (TM_UL(1) << optionType))
            {

                switch (optionType)
                {

/* 3. Interface-Identifier: ([RFC2472].R4.1:110) */
                    case TM_IPV6CP_INTERFACE_ID:
                        if (optionTotalLength != 10)
                        {
/* Bad length, reject this option */
                            needSendRej = TM_8BIT_YES;
                        }
                        else
                        {
                            tm_ppp_byte_copy(optionValuePtr, valueDataOffset,
                                             &interfaceId[0], 0,
                                             TM_32BIT_BYTE_COUNT);
                            tm_ppp_byte_copy(optionValuePtr + sizeof(tt32Bit),
                                             valueDataOffset,
                                             &interfaceId[1], 0,
                                             TM_32BIT_BYTE_COUNT);
/*
 *     3.1.Compare the Interface-Identifier option that the peer is requesting
 *         with our own (tentative) interface identifier.
 */
                            localInterfaceIdPtr = pppVectPtr->pppsIp6cpState.
                                ip6cpsLocalNegOptions.ip6cpInterfaceId;
                            if (   (localInterfaceIdPtr[0] != interfaceId[0])
                                || (localInterfaceIdPtr[1] != interfaceId[1]))
                            {
/*
 *     3.2. If the identifiers are different and the peer's identifier is
 *          zero, send a Conf-Nak with a suggested interface identifier.  This
 *          identifier MUST be different than our own.  This value will either
 *          be set by the user or will be generated randomly when this PPP
 *          vector is initialized.
 */
                                if (   (interfaceId[0] == TM_UL(0))
                                    && (interfaceId[1] == TM_UL(0)))
                                {
                                    needSendNak = TM_8BIT_YES;
                                }
                                else
                                {
/*
 *     3.3. If they are different and the peer's Interface-Identifier is not
 *          zero, acknowledge this option.
 */
                                    remoteOptionsType |= (TM_UL(1)<<optionType);
                                    tm_bcopy(&interfaceId,
                                             &remoteOptions.ip6cpInterfaceId,
                                             TM_6PAK_INTERFACE_ID_LEN);
                                }
                            }
                            else
                            {
/*
 *     3.4. If the two identifiers are equal and non-zero, send a Conf-Nak
 *          with a suggested identifier to be used by the peer.  This value
 *          must be non-zero and different from the current interface
 *          identifier. [RFC2472].R4.1:20]
 */
                                if (   (interfaceId[0] != TM_UL(0))
                                    && (interfaceId[1] != TM_UL(0))
                                    && (localInterfaceIdPtr[0] != TM_UL(0))
                                    && (localInterfaceIdPtr[1] != TM_UL(0)))
                                {
                                    if (ip6cpStatePtr->ip6cpsStateInfo.
                                            cpsFailures > 0)
                                    {
                                        needSendNak = TM_8BIT_YES;
                                    }
                                    else
                                    {
                                        needSendRej = TM_8BIT_YES;
                                    }
                                }
                                else
                                {
/*
 *     3.5. If both the peer's interface identifier and our local identifier
 *          are zero, send a Conf-Rej for this option and disable any future
 *          negotiation of this option.
 */
                                    needSendRej = TM_8BIT_YES;
                                }
                            }
                        }
                        if (needSendRej)
                        {
                            rejPacketPtr =
                                tm_ppp_add_reject(tfPppAddReject,
                                                  dataPtr,
                                                  &outDataPtr,
                                                  optionLength,
                                                  rejPacketPtr,
                                                  nakPacketPtr,
                                                  inDataOffset,
                                                  &outDataOffset);
                            if (rejPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto ip6cpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;

                        }
                        else if (needSendNak)
                        {
                            nakPacketPtr =
                                tm_ppp_add_nak(tfPppAddNak,
                                               dataPtr,
                                               (tt8BitPtr)
                                                 &pppVectPtr->ppps6RandomPeerId,
                                               &outDataPtr,
                                               optionLength,
                                               rejPacketPtr,
                                               nakPacketPtr,
                                               inDataOffset,
                                               &outDataOffset);
                        }
                        break;

#ifdef TM_USE_IPHC
/* 4. IPv6-Compression-Protocol: */
                    case TM_IPV6CP_COMP_PROTOCOL:
/* Get the requested option value */
                        tm_bcopy(optionValuePtr,
                                 &reqCompProto,
                                 sizeof(reqCompProto));
                        optionValuePtr += sizeof(reqCompProto);
                        ip6cpRemoteAllowOptionsPtr =
                            &ip6cpStatePtr->ip6cpsRemoteAllowOptions;
/*
 *     4.1. If the option length (excluding the type and length fields) is
 *          less than 12 or no compression protocol has been configured, send
 *          a Conf-Rej to the peer.
 */
                        if (    (optionLength < TM_PPP_IPHC_MIN_OPT_LEN)
                             || (ip6cpRemoteAllowOptionsPtr->
                                           ip6cpCompressProtocol == TM_UL(0)))
                        {
                            rejPacketPtr =
                                tm_ppp_add_reject(tfPppAddReject,
                                                  dataPtr,
                                                  &outDataPtr,
                                                  optionLength,
                                                  rejPacketPtr,
                                                  nakPacketPtr,
                                                  inDataOffset,
                                                  &outDataOffset);
                            if (rejPacketPtr == TM_PACKET_NULL_PTR)
                            {
                                goto ip6cpParseConfigFinish;
                            }
                            nakPacketPtr = TM_PACKET_NULL_PTR;
                        }
                        else
                        {

/*
 *     4.2. If all of the requested values (TCP_SPACE & NON_TCP_SPACE) match,
 *          save the values and send a Conf-Ack.
 */
                            tm_bcopy(optionValuePtr,
                                     iphcOpts,
                                     sizeof(iphcOpts));
/* Only NAK if the peer requests a different sized compression space; leave
   all other options as is. */
                            if (   (ip6cpRemoteAllowOptionsPtr->
                                    ip6cpCompressProtocol == reqCompProto)
                                && (ip6cpRemoteAllowOptionsPtr->
                                    ip6cpCompTcpSpace == htons(iphcOpts[0]))
                                && (ip6cpRemoteAllowOptionsPtr->
                                    ip6cpCompNonTcpSpace == htons(iphcOpts[1])))
                            {
                                remoteOptionsType |=
                                    (TM_UL(1)<< optionType);
                                remoteOptions.ip6cpCompressProtocol =
                                    reqCompProto;
                                remoteOptions.ip6cpCompTcpSpace    =
                                    ntohs(iphcOpts[0]);
                                remoteOptions.ip6cpCompNonTcpSpace =
                                    ntohs(iphcOpts[1]);
                                remoteOptions.ip6cpCompMaxPeriod   =
                                    ntohs(iphcOpts[2]);
                                remoteOptions.ip6cpCompMaxTime     =
                                    ntohs(iphcOpts[3]);
                                remoteOptions.ip6cpCompMaxHeader   =
                                    ntohs(iphcOpts[4]);
                            }
                            else
                            {
/*     4.3. Otherwise, Nak with our preferred option values. */
                                optionLength = 10;
                                iphcOpts[0] =
                                    htons(ip6cpRemoteAllowOptionsPtr->
                                          ip6cpCompressProtocol);
                                iphcOpts[1] =
                                    htons(ip6cpRemoteAllowOptionsPtr->
                                          ip6cpCompTcpSpace);
                                iphcOpts[2] =
                                    htons(ip6cpRemoteAllowOptionsPtr->
                                          ip6cpCompNonTcpSpace);
                                iphcOpts[3] =
                                    htons(ip6cpRemoteAllowOptionsPtr->
                                          ip6cpCompMaxPeriod);
                                iphcOpts[4] =
                                    htons(ip6cpRemoteAllowOptionsPtr->
                                          ip6cpCompMaxTime);
                                iphcOpts[5] =
                                    htons(ip6cpRemoteAllowOptionsPtr->
                                          ip6cpCompMaxHeader);

                                nakPacketPtr =
                                    tm_ppp_add_nak(tfPppAddNak,
                                                   dataPtr,
                                                   (tt8BitPtr) &iphcOpts[0],
                                                   &outDataPtr,
                                                   optionLength,
                                                   rejPacketPtr,
                                                   nakPacketPtr,
                                                   inDataOffset,
                                                   &outDataOffset);
                            }
                        }
                        break;
#endif /* TM_USE_IPHC */

/*
 * 5. Other options: We currently only support the Interface-Identifier
 *    option, so all other option types should result in a Conf-Rej sent to
 *    the peer.
 */
                    default:
                        rejPacketPtr =
                            tm_ppp_add_reject(tfPppAddReject,
                                              dataPtr,
                                              &outDataPtr,
                                              optionLength,
                                              rejPacketPtr,
                                              nakPacketPtr,
                                              inDataOffset,
                                              &outDataOffset);
                        if (rejPacketPtr == TM_PACKET_NULL_PTR)
                        {
                            goto ip6cpParseConfigFinish;
                        }
                        nakPacketPtr = TM_PACKET_NULL_PTR;
                }

            }
        }

        tm_ppp_move_next_option(dataPtr, optionTotalLength, inDataOffset);
        length = (tt16Bit)(length - optionTotalLength);
    }

/*
 * 6. If the request does not include an Interface-Identifier option and
 *    negotiation of this option has not been disabled (either by the user or
 *    through a Conf-Rej from the peer), we should send a Conf-Nak to the peer
 *    which includes a suggested identifier value.  This must only be done
 *    once in the course of an IPV6CP negotiation.  If we have already sent a
 *    Conf-Nak in this manner and are receiving a second Conf-Req without the
 *    Interface-Identifier, we should now assume that the peer cannot
 *    negotiate this option. [RFC2472].R4.1:170, [RFC2472].R4.1:180
 */
    if (    !(remoteOptionsType & (TM_UL(1) << TM_IPV6CP_INTERFACE_ID))
         && (   ip6cpStatePtr->ip6cpsStateInfo.cpsFailures
             == ip6cpStatePtr->ip6cpsStateInfo.cpsInitMaxFailures))
    {
/* Need to create some temporary data since we don't have data from the packet,
 * (type, length) like we would for a 'real' NAK.
 */
        tempIdData[0] = TM_IPV6CP_INTERFACE_ID;
        tempIdData[1] = 10;
        nakPacketPtr = tm_ppp_add_nak(tfPppAddNak,
                                      tempIdData,
                                      (tt8BitPtr) &pppVectPtr->ppps6RandomPeerId,
                                      &outDataPtr,
                                      8,
                                      rejPacketPtr,
                                      nakPacketPtr,
                                      inDataOffset,
                                      &outDataOffset);
    }

/*
 * If we're going to send a NAK (ie, nakPacketPtr != NULL), decrement the
 * max-failure count.
 */
    if (nakPacketPtr != TM_PACKET_NULL_PTR)
    {
        --ip6cpStatePtr->ip6cpsStateInfo.cpsFailures;
    }

    retPacketPtr = tm_ppp_parse_finish (tfPppParseFinish,
                                        packetPtr, rejPacketPtr,
                                        nakPacketPtr, outDataPtr, pppEventPtr,
                                        ident, outDataOffset);
    if (retPacketPtr == packetPtr)
    {
/* Packet is an ACK (not an NAK, or Reject) */
        tm_bcopy( &remoteOptions,
                  &(ip6cpStatePtr->ip6cpsRemoteSetOptions),
                  sizeof(ttIp6cpOptions));
        ip6cpStatePtr->ip6cpsRemoteAckedOptions = remoteOptionsType;
    }


ip6cpParseConfigFinish:

    if (retPacketPtr == TM_PACKET_NULL_PTR)
    {
        tfPppNoBuffer(pppEventPtr, packetPtr);
    }
    return retPacketPtr;
}

#endif /* TM_USE_IPV6 */


#ifdef TM_USE_IPHC
static void tfPppSetIphcValues(ttPppVectPtr pppVectPtr,
                               tt16Bit      compressProto,
                               int          protoFamily)
{
    ttIphcOptionsPtr  iphcOptPtr;
#ifdef TM_USE_IPV4
    ttIpcpOptionsPtr  ipcpOptPtr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    ttIp6cpOptionsPtr ip6cpOptPtr;
#endif /* TM_USE_IPV6 */

    iphcOptPtr = &pppVectPtr->pppsIphcVectPtr->iphcOpts;

    if (compressProto == TM_PPP_IPHC_PROTOCOL)
    {
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
        if (    (    (protoFamily == AF_INET)
                  && (pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState
                      == TM_PPPS_OPENED))
             || (    (protoFamily == AF_INET6)
                  && (pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState
                      == TM_PPPS_OPENED)) )
        {
            ipcpOptPtr  = &pppVectPtr->pppsIpcpState.ipcpsLocalNegOptions;
            ip6cpOptPtr = &pppVectPtr->pppsIp6cpState.ip6cpsLocalNegOptions;

/* For TCP_SPACE and NON_TCP_SPACE, the largest value should be chosen */
            iphcOptPtr->iphcoInTcpSpace = (tt8Bit)
                (tm_max( ipcpOptPtr->ipcpCompTcpSpace,
                        ip6cpOptPtr->ip6cpCompTcpSpace ) & 0xFF);

            iphcOptPtr->iphcoInUdpSpace = (tt16Bit)
                tm_max( ipcpOptPtr->ipcpCompNonTcpSpace,
                        ip6cpOptPtr->ip6cpCompNonTcpSpace );

/* For F_MAX_PERIOD, F_MAX_TIME and MAX_HEADER the smallest value should be
   chosen. */
            iphcOptPtr->iphcoMaxPeriod = (tt16Bit)
                tm_min( ipcpOptPtr->ipcpCompMaxPeriod,
                        ip6cpOptPtr->ip6cpCompMaxPeriod );

            iphcOptPtr->iphcoMaxTime = (tt16Bit)
                tm_min( ipcpOptPtr->ipcpCompMaxTime,
                        ip6cpOptPtr->ip6cpCompMaxTime );

            iphcOptPtr->iphcoMaxHeader = (tt16Bit)
                tm_min( ipcpOptPtr->ipcpCompMaxHeader,
                        ip6cpOptPtr->ip6cpCompMaxHeader );

        }
        else
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
        {
/* Otherwise, just copy the IPHC configuration values from the IPCP results
   into the IPHC option structure. */
#ifdef TM_USE_IPV6
            if (protoFamily == AF_INET6)
            {
                ip6cpOptPtr =
                    &pppVectPtr->pppsIp6cpState.ip6cpsLocalNegOptions;
                iphcOptPtr->iphcoInTcpSpace =
                    (tt8Bit) ip6cpOptPtr->ip6cpCompTcpSpace;
                iphcOptPtr->iphcoInUdpSpace =
                    ip6cpOptPtr->ip6cpCompNonTcpSpace;
                iphcOptPtr->iphcoMaxPeriod = ip6cpOptPtr->ip6cpCompMaxPeriod;
                iphcOptPtr->iphcoMaxTime   = ip6cpOptPtr->ip6cpCompMaxTime;
                iphcOptPtr->iphcoMaxHeader = ip6cpOptPtr->ip6cpCompMaxHeader;
            }
            else
#endif /* TM_USE_IPV6 */
            if (protoFamily == AF_INET)
            {
#ifdef TM_USE_IPV4
                ipcpOptPtr  = &pppVectPtr->pppsIpcpState.ipcpsLocalNegOptions;
                iphcOptPtr->iphcoInTcpSpace =
                    (tt8Bit) ipcpOptPtr->ipcpCompTcpSpace;
                iphcOptPtr->iphcoInUdpSpace =
                    ipcpOptPtr->ipcpCompNonTcpSpace;
                iphcOptPtr->iphcoMaxPeriod = ipcpOptPtr->ipcpCompMaxPeriod;
                iphcOptPtr->iphcoMaxTime   = ipcpOptPtr->ipcpCompMaxTime;
                iphcOptPtr->iphcoMaxHeader = ipcpOptPtr->ipcpCompMaxHeader;
#endif /* TM_USE_IPV4 */
            }

        }

/* Make sure that enough space is allocated ahead of received packets to
   decompress incoming packets. */
        pppVectPtr->pppsHdrDecompressSpace = iphcOptPtr->iphcoMaxHeader;
    }
}
#endif /* TM_USE_IPHC */

#ifdef TM_USE_PPPOE

/*
 * tfPppPppoeIncomingPacket function description
 * Process an incoming PPP packet without the PPP header (sync Only)
 * This is called by the tfPppoeIncomingPacket() which is itself called by
 * tfEtherRecv(), the link layer recv function for PPPoE.
 * Called with device locked in tfPppoeIncomingPacket().
 * Parameters
 * Parameter   Meaning
 * packetPtr   Pointer to packet.
 * pppProtocol PPP protocol from PPPoE/PPP header.
 */
void tfPppPppoeIncomingPacket(ttPacketPtr packetPtr, tt16Bit pppProtocol)
{
    ttPppVectPtr            pppVectPtr;
    ttLcpStatePtr           pppLcpStatePtr;
    ttDeviceEntryPtr        devPtr;
    ttPppIncomingFuncPtr    pppIncomingFuncPtr;
    tt8Bit                  needFreePacket;
    tt8Bit                  sendRejectStatus;
    tt8Bit                  ipPacket;

    devPtr = packetPtr->pktDeviceEntryPtr;

    pppVectPtr = (ttPppVectPtr)devPtr->devStateVectorPtr;

/*
 * Check if the PPP state vector has not been allocated.  This could occur
 * if the user has not set any PPP options and data is received after the
 * device is opened but before PPP is opened.  Free the incoming data, unlock
 * the device and return.
 */
    if (pppVectPtr == TM_PPP_VECT_NULL_PTR)
    {
        needFreePacket = TM_8BIT_YES;
    }
    else
    {
/* Assume no error occurs */
        needFreePacket = TM_8BIT_NO;
        pppLcpStatePtr = &pppVectPtr->pppsLcpState;
/*
 * Check to make sure that we are NOT negotiating LCP when another
 * protocol comes in.  If it does than the RFC1661 says to silently
 * discard it.
 */
        if (    (pppProtocol != TM_PPP_LCP_PROTOCOL)
             && ( pppLcpStatePtr->lcpsStateInfo.cpsState
                                   != TM_PPPS_OPENED ) )
        {
/* Silently discard non lcp packets when LCP is NOT in the open state */
            needFreePacket = TM_8BIT_YES;
        }
        else
        {
#ifdef TM_PPP_LQM
/* [RFC1989].R2.3:20 -
   SaveInOctets must include the expected values for this LQR. */
            devPtr->devLqmInGoodOctets += packetPtr->pktChainDataLength;
#endif /* TM_PPP_LQM */
/* It's either an LCP packet or LCP is OPEN */
            sendRejectStatus = TM_8BIT_NO;
            ipPacket = TM_8BIT_NO;
            pppIncomingFuncPtr = (ttPppIncomingFuncPtr)0;
            switch (pppProtocol)
            {
#ifdef TM_USE_IPV4
/* An IPv4 datagram */
                case TM_PPP_IP_PROTOCOL:
                    ipPacket = TM_8BIT_YES;
                    break;

/* Internet Protocol Control Protocol (IPCP) a NCP */
                case TM_PPP_IPCP_PROTOCOL:
                    pppIncomingFuncPtr = tfIpcpIncomingPacket;
                    break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
/* IPv6 datagram
 *     Only process IPv6 packets if IPV6CP is open.
 *     ([RFC2472].R2:10, [RFC2472].R3:20)
 */
                case TM_PPP_IPV6_PROTOCOL:
                    if (pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState
                                                            == TM_PPPS_OPENED)
                    {
                        ipPacket = TM_8BIT_YES;
                    }
                    else
                    {
/* IPV6CP not up yet, so drop any IPv6 packets received on the link. */
                        needFreePacket = TM_8BIT_YES;
                    }
                    break;

/*
 * Internet Protocol Control Protocol (IPCP) a NCP
 *    Only process IPV6CP packets if PPP has reached the NCP phase.
 *    ([RFC2472].R3:10, [RFC2472].R3:20)
 */
                case TM_PPP_IPV6CP_PROTOCOL:
                    if (   (pppVectPtr->pppsIp6cpState.ip6cpsStateInfo.cpsState
                                                             > TM_PPPS_INITIAL)
#ifdef TM_USE_IPV4
                        || (pppVectPtr->pppsIpcpState.ipcpsStateInfo.cpsState
                                                             > TM_PPPS_INITIAL)
#endif /* TM_USE_IPV4 */
                        )
                        {
                            pppIncomingFuncPtr = tfIp6cpIncomingPacket;
                        }
                        else
                        {
/* LCP/authentication not up yet, drop any NCP packets. */
                            needFreePacket = TM_8BIT_YES;
                        }
                        break;

#endif /* TM_USE_IPV6 */

/* Link Control Protocol (LCP) */
                case TM_PPP_LCP_PROTOCOL:
                    pppIncomingFuncPtr = tfLcpIncomingPacket;
                    break;

/* Simple Password Authentication Protocol */
                case TM_PPP_PAP_PROTOCOL:
                    pppIncomingFuncPtr = tfPapIncomingPacket;
                    break;

/* Challenge Handshake Authentication Protocol */
                case TM_PPP_CHAP_PROTOCOL:
                    pppIncomingFuncPtr = tfChapIncomingPacket;
                    break;

#ifdef TM_PPP_LQM
                case TM_PPP_LQR_PROTOCOL:
                    if (tfLqmIsEnabled(pppVectPtr))
                    {
/* [RFC1989].R2.2:20 -
   InLQRs is a 32-bit counter which increases by one for each received
   Link-Quality-Report packet. */
                        devPtr->devLqmInLqrs++;
                        pppIncomingFuncPtr = tfLqmIncomingPacket;
                    }
                    else
                    {
/* else, LQM is disabled on the link, send a protocol reject */
                        sendRejectStatus = TM_8BIT_YES;
                    }
                    break;
#endif /* TM_PPP_LQM */
#ifdef TM_USE_EAP
                case TM_PPP_EAP_PROTOCOL:
                    break;
#endif /* TM_USE_EAP */

                default:
/* If we hit the default case then we do NOT support this protocol */
                    sendRejectStatus = TM_8BIT_YES;
                    break;
            }
            if (ipPacket != TM_8BIT_NO)
            {
/* Unlock the device */
                tm_unlock(&(devPtr->devLockEntry));
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPDUAL
                if (pppProtocol == TM_PPP_IP_PROTOCOL)
#endif /* TM_USE_IPDUAL */
                {
                    tfIpIncomingPacket(packetPtr
#ifdef TM_USE_IPSEC_TASK
                                       , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                       );
                }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPDUAL
                if (pppProtocol == TM_PPP_IPV6_PROTOCOL)
#endif /* TM_USE_IPDUAL */
                {
                    tf6IpIncomingPacket(packetPtr
#ifdef TM_USE_IPSEC_TASK
                                        , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                        );
                }
#endif /* TM_USE_IPV6 */
/* Lock the device */
                tm_lock_wait(&(devPtr->devLockEntry));
            }
            else
            {
                if (sendRejectStatus != TM_8BIT_NO)
                {
/* send a protocol reject */
#ifdef TM_SNMP_MIB
#ifdef TM_USE_IPV4
                    devPtr->dev4Ipv4Mib.ipIfStatsInUnknownProtos++;
#ifdef TM_USE_NG_MIB2
                    tm_context(tvDevIpv4Data).ipIfStatsInUnknownProtos++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV4 */
#endif /* TM_SNMP_MIB */
/* Send a protocol reject for this packet */
                    pppIncomingFuncPtr = tfLcpSendProtocolReject;
                }

#ifdef TM_USE_DRV_SCAT_RECV
/*
 * PPP expects all data to be contiguous
 * If PPPoE/PPP header is not contiguous, make it contiguous
 */
               if (    (needFreePacket == TM_8BIT_NO)
                    && (tm_pkt_hdr_is_not_cont(packetPtr,
                                           packetPtr->pktChainDataLength)) )
               {
                    packetPtr = tfContiguousSharedBuffer(packetPtr,
                                        packetPtr->pktChainDataLength);
                    if (packetPtr == (ttPacketPtr)0)
                    {
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
/* Allocation failed */
                        devPtr->devInErrors++;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
                        goto pppPppoeIncomingExit;
                    }
               }
#endif /* TM_USE_DRV_SCAT_RECV */
                if (pppIncomingFuncPtr != (ttPppIncomingFuncPtr)0)
                {
                    (*pppIncomingFuncPtr)(pppVectPtr, packetPtr);
                }
#ifdef TM_USE_EAP
                else
                {
                    if (pppProtocol == TM_PPP_EAP_PROTOCOL)
                    {
                        tfEapIncomingPacket(pppVectPtr->pppsEapVectPtr,
                                            packetPtr);
                    }
                }
#endif /* TM_USE_EAP */
            }
        }
    }
    if (needFreePacket != TM_8BIT_NO)
    {
/* Bad packet so Silently discard it */
        tfFreePacket(packetPtr,TM_SOCKET_UNLOCKED);
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
/* Bad packet */
        devPtr->devInErrors++;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
    }
#ifdef TM_USE_DRV_SCAT_RECV
pppPppoeIncomingExit:
#endif /* TM_USE_DRV_SCAT_RECV */
    return;
}

/*
 * Called by PPPoE upon receiving a TERMINATE message.
 * . Do not allow PPP to send any more messages.
 * . Close PPP
 * Return
 * Value    Meaning
 * 0        no PPP connection
 * 1        There was a PPP connection.
 */
int tfPppPppoeTerminate (ttPppVectPtr pppVectPtr)
{
    int                     retCode;

    retCode = 0; /* Assume PPP not alive */

    if (pppVectPtr != (ttPppVectPtr)0)
    {
        if (pppVectPtr->pppsLcpState.lcpsStateInfo.cpsState > TM_PPPS_STOPPING)
        {
/* PPP alive */
            retCode = 1;
            pppVectPtr->pppsLcpState.lcpsStateInfo.cpsState = TM_PPPS_INITIAL;
        }
        tfPppClose((ttUserInterface)pppVectPtr->pppsDeviceEntryPtr);
    }
    return retCode;
}

#endif /* TM_USE_PPPOE */


#ifdef TM_DSP
/*
 * DSP specific functions
 */

/* TM_DSP use this function to copy octets, notice that
 * both sourcePtr and destPtr may have offset
 */
void tfPppByteCopy( int *        sourcePtr,
                    unsigned int sourceOffset,
                    int *        destPtr,
                    unsigned int destOffset,
                    unsigned int bytes)
{
/* We don't want to change the original pointer
 */
    int *         src;
    int *         dst;
    unsigned int  srcOff;
    unsigned int  dstOff;
    unsigned char srcChar;
    unsigned int  i;

    src    = sourcePtr;
    dst    = destPtr;
    srcOff = sourceOffset;
    dstOff = destOffset;

/*
 * Make sure offsets are in range 0-3 on a 32-bit DSP,
 * and in range 0-2 on a 16-Bit DSP.
 */
    if (srcOff >= TM_DSP_BYTES_PER_WORD)
    {
        src = src + ((srcOff) >> TM_DSP_SHIFT);
        srcOff = srcOff % TM_DSP_BYTES_PER_WORD;
    }
    if (dstOff >= TM_DSP_BYTES_PER_WORD)
    {
        dst = dst + ((dstOff) >> TM_DSP_SHIFT);
        dstOff = dstOff % TM_DSP_BYTES_PER_WORD;
    }
    for (i = 0; i < bytes; i++)
    {
        srcChar = tm_ppp_get_char_next(src, srcOff);
        tm_ppp_put_char_next(dst, dstOff, srcChar);
    }


}


tt8Bit tfPackedStringLength(tt8BitPtr namePtr)
{
/*
 * an unpacked string "AB" in 16bit DSP is saved as '0041 0042 0000'
 * while a packed string "AB" in 16bit DSP is saved as "4142 00xx"
 */
    tt8Bit      length=(tt8Bit)0;
    tt8Bit      dataChar;
    tt8BitPtr   dataPtr=namePtr;
    unsigned int offset=0;

    do
    {
        dataChar = tm_ppp_get_char_next(dataPtr,offset);
        length++;
    }
    while(dataChar != (tt8Bit)0);

    length--;

    return length;
}


int tfDspMemcmp( ttCharPtr strPtr1, unsigned int offset1,
                 ttCharPtr strPtr2, unsigned int offset2, unsigned int bytes)
{
/* Compare octets by octets.
 * We don't want to change the original pointer
 */
    ttCharPtr       sPtr1;
    ttCharPtr       sPtr2;
    unsigned int    off1;
    unsigned int    off2;
    char            ch1;
    char            ch2;
    int             i;

    sPtr1 = strPtr1;
    sPtr2 = strPtr2;
    off1  = offset1;
    off2  = offset2;

    for (i = 0; i < bytes; i++)
    {
        ch1 = tm_ppp_get_char_next(sPtr1,off1);
        ch2 = tm_ppp_get_char_next(sPtr2,off2);
        if(ch1 > ch2 )
        {
            return 1;
        }
        else if(ch1 < ch2)
        {
            return -1;
        }

    }
    return 0; /* they are equal */
}

#endif /* TM_DSP */

#ifdef TM_USE_PPP_CALLBACK
/*
 * Callback Control Protocol packet format:
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |     Code      |  Identifier   |            Length             |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |    Data ...
 *   +-+-+-+-+
 *
 * Configuration option format:
 *    0                   1                   2
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   | Callback Type |    Length     |Callback delay |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   | Callback Address(es) ...
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 *
 * Callback address format:
 *    0                   1                   2
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   | Address Type  |    ASCIIZ address
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/*
 * Macros for accessing the CBCP information.
 */
#define tm_cbcp_get8(offset) \
        ((unsigned int)tm_ppp_get_char_index(cbcpHdrPtr, offset))
#define tm_cbcp_get16(offset) \
        ((tm_cbcp_get8(offset) << 8) | tm_cbcp_get8((offset) + 1))

#define tm_cbcp_put8(offset, data) \
        tm_ppp_put_char_index(cbcpHdrPtr, offset, (tt8Bit)(data))
#define tm_cbcp_put16(offset, data) \
        { \
            tm_cbcp_put8(offset, (data) >> 8) \
            tm_cbcp_put8((offset) + 1, data) \
        }

#define TM_CBCP_OFS_PKT_CODE    0
#define TM_CBCP_OFS_PKT_IDENT   1
#define TM_CBCP_OFS_PKT_LENGTH  2
#define TM_CBCP_OFS_PKT_DATA    4

#define TM_CBCP_OFS_OPT_TYPE    0
#define TM_CBCP_OFS_OPT_LENGTH  1
#define TM_CBCP_OFS_OPT_DELAY   2
#define TM_CBCP_OFS_OPT_ADDRESS 3

#define TM_CBCP_OFS_ADDR_TYPE   0
#define TM_CBCP_OFS_ADDR_DATA   1

#define TM_CBCP_MIN_PKT_LENGTH  6

/***
 ***
 *** Alternative state machine for Callback Control Protocol.
 ***
 ***/
static int tfCbcpStateMachine(  ttPppVectPtr    pppVectPtr,
                                ttPacketPtr     packetPtr,
                                unsigned int    event )
{
    ttCallbackStatePtr      cbcpStatePtr;
    int                     errorCode;
    tt8Bit                  state;

    errorCode = TM_ENOERROR;

/* Callback Control Protocol state information. */
    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
/* Branch to state-specific code. */
    state = cbcpStatePtr->cbcpsState;
    switch (state)
    {
    case TM_PPPS_INITIAL:
        switch (event)
        {
        case TM_PPPE_UP:
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            break;
        case TM_PPPE_OPEN:
            cbcpStatePtr->cbcpsState = TM_PPPS_STARTING;
            errorCode = tfCbcpThisLayerStart(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_CLOSE:
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_PPPS_STARTING:
        switch (event)
        {
        case TM_PPPE_UP:
            if (cbcpStatePtr->cbcpsRole == TM_CBCP_ROLE_CALLER)
            {
/* Wait for a configuration request. */
                cbcpStatePtr->cbcpsState = TM_CBCPS_REQ_WAIT;
            }
            else    /* TM_CBCP_ROLE_ANSWERER */
            {
/* Send a configuration request. */
                cbcpStatePtr->cbcpsState = TM_PPPS_REQ_SENT;
                cbcpStatePtr->cbcpsRetryCounter
                                        = cbcpStatePtr->cbcpsMaxRetry;
                errorCode = tfCbcpSendRequest(pppVectPtr, packetPtr);
            }
            break;
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
            cbcpStatePtr->cbcpsState = TM_PPPS_INITIAL;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_PPPS_CLOSED:
        switch (event)
        {
        case TM_PPPE_DOWN:
            cbcpStatePtr->cbcpsState = TM_PPPS_INITIAL;
            break;
        case TM_PPPE_OPEN:
            if (cbcpStatePtr->cbcpsRole == TM_CBCP_ROLE_CALLER)
            {
/* Wait for a configuration request. */
                cbcpStatePtr->cbcpsState = TM_CBCPS_REQ_WAIT;
            }
            else    /* TM_CBCP_ROLE_ANSWERER */
            {
/* Send a configuration request. */
                cbcpStatePtr->cbcpsState = TM_PPPS_REQ_SENT;
                cbcpStatePtr->cbcpsRetryCounter
                                        = cbcpStatePtr->cbcpsMaxRetry;
                errorCode = tfCbcpSendRequest(pppVectPtr, packetPtr);
            }
            break;
        case TM_PPPE_CLOSE:
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_PPPS_STOPPED:
        switch (event)
        {
        case TM_PPPE_DOWN:
            cbcpStatePtr->cbcpsState = TM_PPPS_STARTING;
            errorCode = tfCbcpThisLayerStart(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_PPPS_REQ_SENT:
        switch (event)
        {
        case TM_PPPE_DOWN:
            cbcpStatePtr->cbcpsState = TM_PPPS_STARTING;
            break;
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
        case TM_PPPE_LAST_TIMEOUT:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_TIMEOUT:
/* Retransmit request. */
            errorCode = tfCbcpSendRequest(pppVectPtr, packetPtr);
            break;
        case TM_CBCPE_CFG_RESPONSE:
/* Stop the timer. */
            tfCbcpTimerRemove(pppVectPtr);
/* Send config ack. This will complete the CBCP negotiation.
 * If "no callback" is negotiated, we go to the open state.
 * Otherwise, we have to wait for the other side to close. */
            if (cbcpStatePtr->cbcpsTypeAck == (tt8Bit)TM_CBCP_CTYPE_NOCALL)
            {
                cbcpStatePtr->cbcpsState = TM_PPPS_OPENED;
                cbcpStatePtr->cbcpsRetryCounter = 0;
                errorCode = tfCbcpSendAck(pppVectPtr, packetPtr);
                if (errorCode == TM_ENOERROR)
                {
                    errorCode = tfCbcpThisLayerUp(pppVectPtr, packetPtr);
                }
            }
            else
            {
                cbcpStatePtr->cbcpsState = TM_PPPS_ACK_SENT;
/* Prevent LCP from closing the interface when it finishes. */
                pppVectPtr->pppsCallbackState.cbcpsIsActive = TM_8BIT_YES;
                cbcpStatePtr->cbcpsRetryCounter
                                        = cbcpStatePtr->cbcpsMaxRetry;
                errorCode = tfCbcpSendAck(pppVectPtr, packetPtr);
            }
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_PPPS_ACK_SENT:
        switch (event)
        {
        case TM_PPPE_DOWN:
/* Stop the timer. */
            tfCbcpTimerRemove(pppVectPtr);
/* LCP is down but there is probably an LCP term-ack packet in the
 * process of being sent. To avoid disrupting transmission and messing
 * up the other end, we will delay for a couple of seconds before hanging
 * up. */
            cbcpStatePtr->cbcpsState = TM_CBCPS_DISCONNECT_DELAY;
            errorCode = tfCbcpDelayBeforeHangUp(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
        case TM_PPPE_LAST_TIMEOUT:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        case TM_CBCPE_CFG_RESPONSE:
        case TM_PPPE_TIMEOUT:
/* Retransmit ack. */
            errorCode = tfCbcpSendAck(pppVectPtr, packetPtr);
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_CBCPS_DISCONNECT_DELAY:
        switch (event)
        {
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_LAST_TIMEOUT:
        case TM_PPPE_TIMEOUT:
/* LCP is down. Hang up and delay for the specified time before calling
 * back. */
            cbcpStatePtr->cbcpsState = TM_CBCPS_CALLBACK_DELAY;
            errorCode = tfCbcpHangUp(pppVectPtr, packetPtr);
            if (errorCode == TM_ENOERROR)
            {
                if (cbcpStatePtr->cbcpsDelayCfg == TM_8BIT_ZERO)
                {
/* No delay. Initiate callback immediately. */
                    cbcpStatePtr->cbcpsState = TM_CBCPS_CALLING;
                    errorCode = tfCbcpDial(pppVectPtr, packetPtr);
                }
                else
                {
                    errorCode = tfCbcpCallbackDelay(pppVectPtr, packetPtr);
                }
            }
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_CBCPS_CALLBACK_DELAY:
        switch (event)
        {
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_TIMEOUT:
        case TM_PPPE_LAST_TIMEOUT:
/* Delay is complete. Initiate callback. */
            cbcpStatePtr->cbcpsState = TM_CBCPS_CALLING;
            errorCode = tfCbcpDial(pppVectPtr, packetPtr);
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_CBCPS_CALLING:
        switch (event)
        {
        case TM_CBCPE_PHY_UP:
/* The physical layer is up and we can start LCP again. */
            cbcpStatePtr->cbcpsState = TM_CBCPS_STARTING2;
/* Resume without the Callback gates. */
            pppVectPtr->pppsCallbackState.cbcpsIsActive = TM_8BIT_NO;
            errorCode = tfCbcpThisLayerStart(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_UP:
/* Callback completed and LCP, authentication has been renegotiated. */
            cbcpStatePtr->cbcpsState = TM_PPPS_OPENED;
            errorCode = tfCbcpThisLayerUp(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_CBCPS_STARTING2:
        switch (event)
        {
        case TM_PPPE_UP:
/* Callback completed and LCP, authentication has been renegotiated. */
            cbcpStatePtr->cbcpsState = TM_PPPS_OPENED;
            errorCode = tfCbcpThisLayerUp(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_CBCPS_REQ_WAIT:
        switch (event)
        {
        case TM_PPPE_DOWN:
            cbcpStatePtr->cbcpsState = TM_PPPS_STARTING;
            break;
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
        case TM_PPPE_TIMEOUT:
        case TM_PPPE_LAST_TIMEOUT:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_GOOD_CFG_REQ:
/* Stop the timer. */
            tfCbcpTimerRemove(pppVectPtr);
/* Send config response. */
            cbcpStatePtr->cbcpsState = TM_CBCPS_RESP_SENT;
            cbcpStatePtr->cbcpsRetryCounter
                                        = cbcpStatePtr->cbcpsMaxRetry;
            errorCode = tfCbcpSendResponse(pppVectPtr, packetPtr);
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_CBCPS_RESP_SENT:
        switch (event)
        {
        case TM_PPPE_DOWN:
            cbcpStatePtr->cbcpsState = TM_PPPS_STARTING;
            break;
        case TM_PPPE_OPEN:
            break;
        case TM_PPPE_CLOSE:
        case TM_PPPE_LAST_TIMEOUT:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_GOOD_CFG_REQ:
        case TM_PPPE_TIMEOUT:
/* Retransmit response. */
            errorCode = tfCbcpSendResponse(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_CFG_ACK:
/* Stop the timer. */
            tfCbcpTimerRemove(pppVectPtr);
/* Callback Control Protocol negotiation is complete.
 * If "no callback" is negotiated, we go to the open state.
 * Otherwise, we initiate a shutdown and wait for it to complete. */
            cbcpStatePtr->cbcpsRetryCounter = 0;
            if (cbcpStatePtr->cbcpsTypeRespond
                                        == (tt8Bit)TM_CBCP_CTYPE_NOCALL)
            {
                cbcpStatePtr->cbcpsState = TM_PPPS_OPENED;
                errorCode = tfCbcpThisLayerUp(pppVectPtr, packetPtr);
            }
            else
            {
                cbcpStatePtr->cbcpsState = TM_PPPS_ACK_REC;
/* Prevent LCP from closing the interface when it finishes. */
                pppVectPtr->pppsCallbackState.cbcpsIsActive = TM_8BIT_YES;
/* Close LCP. */
                errorCode = tfPppStateMachine(  pppVectPtr,
                                                TM_PACKET_NULL_PTR,
                                                TM_PPPE_CLOSE,
                                                TM_PPP_LCP_INDEX );
            }
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_PPPS_ACK_REC:
        switch (event)
        {
        case TM_PPPE_DOWN:
/* LCP is down. Hang up and wait for the callback. */
            cbcpStatePtr->cbcpsState = TM_CBCPS_CALLING;
            errorCode = tfCbcpHangUp(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_OPEN:
        case TM_PPPE_CFG_ACK:
            break;
        case TM_PPPE_CLOSE:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    case TM_PPPS_OPENED:
        switch (event)
        {
        case TM_PPPE_DOWN:
            cbcpStatePtr->cbcpsState = TM_PPPS_STARTING;
            errorCode = tfCbcpThisLayerDown(pppVectPtr, packetPtr);
            break;
        case TM_PPPE_OPEN:
        case TM_CBCPE_PHY_UP:
        case TM_PPPE_LAST_TIMEOUT:
        case TM_PPPE_TIMEOUT:
            break;
        case TM_PPPE_CLOSE:
/* Close CBCP. */
            cbcpStatePtr->cbcpsState = TM_PPPS_CLOSED;
            errorCode = tfCbcpThisLayerFinish(pppVectPtr, packetPtr);
            break;
        case TM_CBCPE_CFG_RESPONSE:
/* Retransmit ack. */
            errorCode = tfCbcpSendAck(pppVectPtr, packetPtr);
            break;
        default:
            errorCode = tfPppStateError(pppVectPtr, packetPtr);
        }
        break;

    default:
        errorCode = tfPppStateError(pppVectPtr, packetPtr);
    }

    if (errorCode != TM_ENOERROR)
    {
        cbcpStatePtr->cbcpsState = state;
    }
    return errorCode;
}

/***
 ***
 *** Callback Control Protocol functions.
 ***
 ***/

/**
 ** Initialize the Callback Control Protocol.
 **/
static void tfCbcpInit(ttPppVectPtr pppVectPtr, tt8Bit pppLinkLayer)
{
    ttCallbackStatePtr      cbcpStatePtr;

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
/*
 * If not explicitly set by the user, record the link layer type:
 * client (caller) or server (answerer).
 * The Callback Control Protocol proceeds as follows:
 *   1. Callback Request    ( Answerer -> Caller )
 *   2. Callback Response   ( Caller   -> Answerer )
 *   3. Callback Ack        ( Answerer -> Caller )
 */
    if (cbcpStatePtr->cbcpsRole == TM_CBCP_ROLE_DEFAULT)
    {
        cbcpStatePtr->cbcpsRole = (tt8Bit)((pppLinkLayer == TM_PPP_CLIENT_LINK)
                                            ? TM_CBCP_ROLE_CALLER
                                            : TM_CBCP_ROLE_ANSWERER);
    }
}

/**
 ** Load the default state information.
 **/
static void tfCbcpDefaultInit(ttPppVectPtr pppVectPtr)
{
    ttCallbackStatePtr      cbcpStatePtr;

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
    cbcpStatePtr->cbcpsState = TM_PPPS_INITIAL;

    cbcpStatePtr->cbcpsMaxRetry         = TM_CBCP_MAX_RETRY;
    cbcpStatePtr->cbcpsRetryTimeout     = TM_CBCP_RETRY_TIMEOUT;
    cbcpStatePtr->cbcpsDelaySet         = TM_CBCP_CALL_DELAY;
    cbcpStatePtr->cbcpsDisconnectDelay  = TM_CBCP_DISC_DELAY;
}

/**
 ** Process an inbound Callback Control Protocol packet.
 **/
static int tfCbcpIncomingPacket(    ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr )
{
    ttCallbackStatePtr      cbcpStatePtr;
    tt8BitPtr               cbcpHdrPtr;
    tt8BitPtr               typePtr;
    tt8BitPtr               bytePtr;
    int                     errorCode;
    unsigned int            optionLength;
    unsigned int            optionOffset;
    unsigned int            optionType;
    unsigned int            counter;
    unsigned int            typeCount;
    unsigned int            packetLength;
    unsigned int            packedBytes;
    tt8Bit                  event;

    errorCode = TM_ENOERROR;
    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
    cbcpHdrPtr = packetPtr->pktLinkDataPtr;
    event = TM_PPPE_NO_EVENT;

/* Check the minimum length requirements. */
    if (packetPtr->pktLinkDataLength < TM_CBCP_MIN_PKT_LENGTH)
    {
        goto CbcpIncomingPacketFinish;
    }

    packetLength = tm_cbcp_get16(TM_CBCP_OFS_PKT_LENGTH);

/* All options must fit within one packet. */
    if (packetLength > packetPtr->pktLinkDataLength)
    {
        goto CbcpIncomingPacketFinish;
    }

    switch (tm_cbcp_get8(TM_CBCP_OFS_PKT_CODE))
    {
/*** CBCP request contains a list of all permissible Callback types. ***/
    case TM_CBCP_REQUEST:
/* Scan the request packet for Callback types that we will allow.
 * Our list is ordered by preference, so we check if our first choice is
 * anywhere in the packet before moving on to our second choice. */
        optionOffset = TM_CBCP_OFS_PKT_DATA;
        typePtr = cbcpStatePtr->cbcpsTypeSet;
        typeCount = cbcpStatePtr->cbcpsTypeSetCount;
/* Check if our list or their list is empty. */
        if (typeCount == 0 || optionOffset + 2 > packetLength)
        {
            goto CbcpIncomingPacketFinish;
        }
/* Loop until we find a match or exhaust both lists. */
        for (counter = 0; ; )
        {
            optionLength = tm_cbcp_get8(optionOffset
                                                + TM_CBCP_OFS_OPT_LENGTH);
            if (optionOffset + optionLength > packetLength)
            {
                goto CbcpIncomingPacketFinish;
            }
/* Exit loop on match. */
            if (tm_cbcp_get8(optionOffset + TM_CBCP_OFS_OPT_TYPE)
                                                    == typePtr[counter])
            {
                break;
            }
/* Check the next Callback type in the request packet. */
            optionOffset += optionLength;
/* If end of packet, check the next Callback type in our list. */
            if (optionOffset + 2 >= packetLength)
            {
/* If end of our list, we failed to agree on a Callback type. */
                if (++counter >= typeCount)
                {
                    goto CbcpIncomingPacketFinish;
                }
/* Start scanning from the beginning of the packet data again. */
                optionOffset = TM_CBCP_OFS_PKT_DATA;
            }
        }
/* Record the Callback type and identifier we will respond with later. */
        cbcpStatePtr->cbcpsTypeRespond = typePtr[counter];
        cbcpStatePtr->cbcpsRespondId
                            = (tt8Bit)tm_cbcp_get8(TM_CBCP_OFS_PKT_IDENT);
        event = TM_PPPE_GOOD_CFG_REQ;
        break;

/*** CBCP response contains one Callback type from the list we sent. ***/
    case TM_CBCP_RESPONSE:
/* Check for minimum packet length. */
        if (TM_CBCP_OFS_PKT_DATA + 2 > packetLength)
        {
            goto CbcpIncomingPacketFinish;
        }
/* Check that the identifier matches our request. */
        if (tm_cbcp_get8(TM_CBCP_OFS_PKT_IDENT)
                                        != cbcpStatePtr->cbcpsRequestId)
        {
            goto CbcpIncomingPacketFinish;
        }
        optionLength = tm_cbcp_get8(TM_CBCP_OFS_PKT_DATA
                                            + TM_CBCP_OFS_OPT_LENGTH);
        typePtr = cbcpStatePtr->cbcpsTypeSet;
        typeCount = cbcpStatePtr->cbcpsTypeSetCount;
/* Check if our list or their list is empty. */
        if (typeCount == 0 || TM_CBCP_OFS_PKT_DATA + optionLength
                                                        > packetLength)
        {
            goto CbcpIncomingPacketFinish;
        }
        optionType = tm_cbcp_get8(TM_CBCP_OFS_PKT_DATA
                                            + TM_CBCP_OFS_OPT_TYPE);
/* Verify that we actually do allow the Callback type indicated. */
        for (counter = 0; optionType != typePtr[counter]; )
        {
/* If end of our list, the Callback type in the packet is invalid. */
            if (++counter >= typeCount)
            {
                goto CbcpIncomingPacketFinish;
            }
        }
/* Save the data from the response packet for use in ack later. */
        bytePtr = cbcpStatePtr->cbcpsAckDataPtr;
        packedBytes = tm_packed_byte_count(packetLength);
        if (cbcpStatePtr->cbcpsAckDataLength < packetLength)
        {
            if (bytePtr != TM_8BIT_NULL_PTR)
            {
                tm_free_raw_buffer((ttRawBufferPtr)bytePtr);
            }
            bytePtr = (tt8BitPtr)tm_get_raw_buffer(packedBytes);
            cbcpStatePtr->cbcpsAckDataPtr = bytePtr;
            if (bytePtr == TM_8BIT_NULL_PTR)
            {
                errorCode = TM_ENOBUFS;
                goto CbcpIncomingPacketFinish;
            }
        }
        cbcpStatePtr->cbcpsAckDataLength = (tt16Bit)packetLength;
        tm_bcopy(cbcpHdrPtr, bytePtr, packedBytes);
        tm_ppp_put_char_index(bytePtr, 0, TM_CBCP_ACK);
/* Record the Callback type we will ack later. */
        cbcpStatePtr->cbcpsTypeAck = (tt8Bit)optionType;
/* Record the Callback delay we may use later. */
        cbcpStatePtr->cbcpsDelayCfg = (tt8Bit)tm_cbcp_get8(
                        TM_CBCP_OFS_PKT_DATA + TM_CBCP_OFS_OPT_DELAY);
        event = TM_CBCPE_CFG_RESPONSE;
        break;

/*** CBCP ack contains a copy of the response we sent. ***/
    case TM_CBCP_ACK:
/* Check the length. */
        if (packetLength != cbcpStatePtr->cbcpsResponseDataLength)
        {
            goto CbcpIncomingPacketFinish;
        }
/* Check the contents against our saved response data. */
        bytePtr = cbcpStatePtr->cbcpsResponseDataPtr;
        for (counter = 1; counter < packetLength; ++counter)
        {
            if (tm_ppp_get_char_index(cbcpHdrPtr, counter)
                            != tm_ppp_get_char_index(bytePtr, counter))
            {
                goto CbcpIncomingPacketFinish;
            }
        }
        event = TM_PPPE_CFG_ACK;
        break;

/* Invalid Callback code. */
    default:
        ;
    }

CbcpIncomingPacketFinish:
/* Free the incoming packet. */
    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    if (event != TM_PPPE_NO_EVENT)
    {
        errorCode = tfCbcpStateMachine( pppVectPtr,
                                        TM_PACKET_NULL_PTR,
                                        event );
    }
    return errorCode;
}

/***
 ***
 *** Callback Control Protocol state functions.
 ***
 ***/

/**
 ** Timeout waiting for peer.
 **/
static void tfCbcpTimeout ( ttVoidPtr       timerBlockPtr,
                            ttGenericUnion  userParm1,
                            ttGenericUnion  userParm2 )
{
    ttPppVectPtr        pppVectPtr;
    ttCallbackStatePtr  cbcpStatePtr;
    unsigned int        event;
#ifdef TM_LOCK_NEEDED
    ttDeviceEntryPtr    devPtr;
#endif /* TM_LOCK_NEEDED */

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    pppVectPtr = (ttPppVectPtr)userParm1.genVoidParmPtr;
#ifdef TM_LOCK_NEEDED
    devPtr = pppVectPtr->pppsDeviceEntryPtr;
#endif /* TM_LOCK_NEEDED */
    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
    tm_call_lock_wait(&(devPtr->devLockEntry));
/* If timer has not been removed */
    if (cbcpStatePtr->cbcpsTmrPtr != TM_TMR_NULL_PTR)
    {
/* One-shot timer, mark it as removed */
        cbcpStatePtr->cbcpsTmrPtr = TM_TMR_NULL_PTR;
/* Crank the state machine to let it know we got a timeout */
        if (cbcpStatePtr->cbcpsRetryCounter > TM_8BIT_ZERO)
        {
            if (--cbcpStatePtr->cbcpsRetryCounter == TM_8BIT_ZERO)
            {
                event = TM_PPPE_LAST_TIMEOUT;
            }
            else
            {
                event = TM_PPPE_TIMEOUT;
            }
            tfCbcpStateMachine( pppVectPtr,
                                TM_PACKET_NULL_PTR,
                                event );
        }
    }
    tm_call_unlock(&(devPtr->devLockEntry));
}

/**
 ** Indicate to the lower layers the we are starting the automation.
 ** Start LCP.
 **/
static int tfCbcpThisLayerStart(    ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr )
{
    int     errorCode;

    TM_UNREF_IN_ARG(packetPtr);
/* Make sure LCP is in the Starting state, ready for an Up signal. */
    errorCode = tfPppStateMachine(  pppVectPtr,
                                    TM_PACKET_NULL_PTR,
                                    TM_PPPE_OPEN,
                                    TM_PPP_LCP_INDEX );
    if (errorCode == TM_ENOERROR)
    {
        errorCode = tfPppStateMachine(  pppVectPtr,
                                        TM_PACKET_NULL_PTR,
                                        TM_PPPE_UP,
                                        TM_PPP_LCP_INDEX );
    }
    return errorCode;
}

/**
 ** Indicate to the LCP Layer that we are done with the link.
 **/
static int tfCbcpThisLayerFinish(   ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr )
{
    ttCallbackStatePtr          cbcpStatePtr;

    TM_UNREF_IN_ARG(packetPtr);

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;

    cbcpStatePtr->cbcpsIsActive = TM_8BIT_NO;
    cbcpStatePtr->cbcpsAddressCfgLength = TM_16BIT_ZERO;
    cbcpStatePtr->cbcpsResponseDataLength = TM_16BIT_ZERO;
    cbcpStatePtr->cbcpsAckDataLength = TM_16BIT_ZERO;

/* Deallocate all pointers in the state block. */
    if (cbcpStatePtr->cbcpsTmrPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(cbcpStatePtr->cbcpsTmrPtr);
        cbcpStatePtr->cbcpsTmrPtr = TM_TMR_NULL_PTR;
    }
    if (cbcpStatePtr->cbcpsAddressCfgPtr != TM_8BIT_NULL_PTR)
    {
        tm_free_raw_buffer((ttRawBufferPtr)cbcpStatePtr->cbcpsAddressCfgPtr);
        cbcpStatePtr->cbcpsAddressCfgPtr = TM_8BIT_NULL_PTR;
    }
    if (cbcpStatePtr->cbcpsResponseDataPtr != TM_8BIT_NULL_PTR)
    {
        tm_free_raw_buffer((ttRawBufferPtr)cbcpStatePtr->cbcpsResponseDataPtr);
        cbcpStatePtr->cbcpsResponseDataPtr = TM_8BIT_NULL_PTR;
    }
    if (cbcpStatePtr->cbcpsAckDataPtr != TM_8BIT_NULL_PTR)
    {
        tm_free_raw_buffer((ttRawBufferPtr)cbcpStatePtr->cbcpsAckDataPtr);
        cbcpStatePtr->cbcpsAckDataPtr = TM_8BIT_NULL_PTR;
    }

    return tfPppStateMachine(   pppVectPtr,
                                TM_PACKET_NULL_PTR,
                                TM_PPPE_CLOSE,
                                TM_PPP_LCP_INDEX );
}

/**
 ** Signal upper layer that CBCP is open.
 **/
static int tfCbcpThisLayerUp(   ttPppVectPtr    pppVectPtr,
                                ttPacketPtr     packetPtr )
{
    int         errorCode;

    TM_UNREF_IN_ARG(packetPtr);

/* Stop the timer. */
    tfCbcpTimerRemove(pppVectPtr);

/* Allow LCP to close the interface when it finishes. */
    pppVectPtr->pppsCallbackState.cbcpsIsActive = TM_8BIT_NO;

    errorCode = tfPppNextLayer(pppVectPtr, TM_PPP_CBCP_INDEX);
    return errorCode;
}

/**
 ** Signal upper layer that CBCP is closing.
 ** We arrived here because LCP is closing. Since LCP notifies all
 ** layers, we don't have to notify the upper layer.
 **/
static int tfCbcpThisLayerDown( ttPppVectPtr    pppVectPtr,
                                ttPacketPtr     packetPtr )
{
    TM_UNREF_IN_ARG(pppVectPtr);
    TM_UNREF_IN_ARG(packetPtr);

/* Interface is closing. We need to close CBCP to free up its resources. */
    return tfCbcpStateMachine(  pppVectPtr,
                                TM_PACKET_NULL_PTR,
                                TM_PPPE_CLOSE );
}

/**
 ** LCP is up, which means Callback option negotiation is complete.
 **/
static int tfCbcpLcpUp( ttPppVectPtr    pppVectPtr,
                        ttPacketPtr     packetPtr )
{
    int         errorCode;

    TM_UNREF_IN_ARG(packetPtr);

    errorCode = TM_ENOERROR;
    if ( (pppVectPtr->pppsLcpState.lcpsLocalGotOptions.lcpCallbackOpFlags
                    & (TM_UL(1) << TM_CALLBACK_OP_CBCP)) != TM_32BIT_ZERO )
    {
/* Callback Control Protocol has been negotiated. Open it. */
        errorCode = tfCbcpStateMachine( pppVectPtr,
                                        TM_PACKET_NULL_PTR,
                                        TM_PPPE_OPEN );
    }
    return errorCode;
}

/**
 ** LCP is going down and it's not because of PPP Callback.
 ** Close CBCP to release memory, etc.
 **/
static int tfCbcpLcpDown(   ttPppVectPtr    pppVectPtr,
                            ttPacketPtr     packetPtr )
{
    int     errorCode;

    TM_UNREF_IN_ARG(packetPtr);

    errorCode = tfCbcpStateMachine( pppVectPtr,
                                    TM_PACKET_NULL_PTR,
                                    TM_PPPE_CLOSE );
    return errorCode;
}

/**
 ** LCP is finished (i.e. LCP is in Stopping or Closing state).
 ** We need to tell LCP that the physical layer is down (which it soon
 ** will be) to finalize the orderly LCP shutdown.
 ** We are then able to hang up and initiate or wait for a callback.
 **/
static int tfCbcpLcpFinish( ttPppVectPtr    pppVectPtr,
                            ttPacketPtr     packetPtr )
{
    int     errorCode;

    TM_UNREF_IN_ARG(packetPtr);

    errorCode = tfPppStateMachine(  pppVectPtr,
                                    TM_PACKET_NULL_PTR,
                                    TM_PPPE_DOWN,
                                    TM_PPP_LCP_INDEX );
    if (errorCode == TM_ENOERROR)
    {
        errorCode = tfCbcpStateMachine( pppVectPtr,
                                        TM_PACKET_NULL_PTR,
                                        TM_PPPE_DOWN );
    }
    return errorCode;
}

/**
 ** Disconnect the physical medium.
 **/
static int tfCbcpHangUp(    ttPppVectPtr    pppVectPtr,
                            ttPacketPtr     packetPtr )
{
    ttPppCallbackUserFunctPtr   userFuncPtr;
    ttDeviceEntryPtr            devPtr;

    TM_UNREF_IN_ARG(packetPtr);

    userFuncPtr = pppVectPtr->pppsCallbackState.cbcpsUserFuncPtr;
    if (userFuncPtr != (ttPppCallbackUserFunctPtr)0)
    {
        devPtr = pppVectPtr->pppsDeviceEntryPtr;
/* Unlock the device. */
        tm_call_unlock(&(devPtr->devLockEntry));
/* Ask the user to hang up the phone. */
        (*userFuncPtr)( (ttUserInterface)devPtr,
                        TM_PPP_USER_ACTION_DISCONNECT,
                        (ttCharPtr)0,
                        0);
/* Lock the device. */
        tm_call_lock_wait(&(devPtr->devLockEntry));
    }
    return TM_ENOERROR;
}

/**
 ** Connect the physical medium to an address.
 **/
static int tfCbcpDial(  ttPppVectPtr    pppVectPtr,
                        ttPacketPtr     packetPtr )
{
    ttCallbackStatePtr          cbcpStatePtr;
    ttPppCallbackUserFunctPtr   userFuncPtr;
    ttDeviceEntryPtr            devPtr;
    ttCharPtr                   addressPtr;
    int                         addressLength;

    TM_UNREF_IN_ARG(packetPtr);

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
    userFuncPtr = cbcpStatePtr->cbcpsUserFuncPtr;
    if (userFuncPtr != (ttPppCallbackUserFunctPtr)0)
    {
/* Use the negotiated address, if it exists. */
        addressPtr = (ttCharPtr)cbcpStatePtr->cbcpsAddressCfgPtr;
        addressLength = (int)cbcpStatePtr->cbcpsAddressCfgLength;
        if (addressPtr == (ttCharPtr)0)
        {
/* Otherwise, use our address, if it exists. */
            addressPtr = (ttCharPtr)cbcpStatePtr->cbcpsAddressSetPtr;
            addressLength = (int)cbcpStatePtr->cbcpsAddressSetLength;
            if (addressPtr == (ttCharPtr)0)
            {
                goto CbcpDialFinish;
            }
        }

        devPtr = pppVectPtr->pppsDeviceEntryPtr;
/* Unlock the device. */
        tm_call_unlock(&(devPtr->devLockEntry));
/* Ask the user to dial the specified phone number. */
        (*userFuncPtr)( (ttUserInterface)devPtr,
                        TM_PPP_USER_ACTION_CONNECT,
                        addressPtr,
                        addressLength);
/* Lock the device. */
        tm_call_lock_wait(&(devPtr->devLockEntry));
    }
CbcpDialFinish:
    return TM_ENOERROR;
}

/**
 ** LCP is down but there is probably an LCP term-ack packet in the
 ** process of being sent. To avoid disrupting transmission and messing
 ** up the other end, we will delay for a couple of seconds before hanging
 ** up.
 **/
static int tfCbcpDelayBeforeHangUp( ttPppVectPtr    pppVectPtr,
                                    ttPacketPtr     packetPtr )
{
    ttCallbackStatePtr      cbcpStatePtr;

    TM_UNREF_IN_ARG(packetPtr);

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
    cbcpStatePtr->cbcpsRetryCounter = 1;
/* Start our retry timer */
    tfCbcpTimerAdd(pppVectPtr, cbcpStatePtr->cbcpsDisconnectDelay);
    return TM_ENOERROR;
}

/**
 ** Delay for the required time before calling back.
 **/
static int tfCbcpCallbackDelay( ttPppVectPtr    pppVectPtr,
                                ttPacketPtr     packetPtr )
{
    ttCallbackStatePtr      cbcpStatePtr;

    TM_UNREF_IN_ARG(packetPtr);

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
    cbcpStatePtr->cbcpsRetryCounter = 1;
/* Start our retry timer */
    tfCbcpTimerAdd(pppVectPtr, cbcpStatePtr->cbcpsDelayCfg);
    return TM_ENOERROR;
}

/**
 ** Create a CBCP config request packet and send it.
 **/
static int tfCbcpSendRequest(   ttPppVectPtr    pppVectPtr,
                                ttPacketPtr     packetPtr )
{
    ttCallbackStatePtr      cbcpStatePtr;
    tt8BitPtr               cbcpHdrPtr;
    int                     errorCode;
    unsigned int            optionOffset;
    unsigned int            counter;
    tt8Bit                  needFreePacket;
    tt8Bit                  optionCount;
    tt8Bit                  optionType;


    errorCode = TM_ENOERROR;
    needFreePacket = TM_8BIT_NO;

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
/* An empty list of options should not occur. */
    optionCount = cbcpStatePtr->cbcpsTypeSetCount;
    if (optionCount == 0)
    {
        goto CbcpSendRequestFinish;
    }
/*
 * Allocate a packet.
 * Largest request packet (Callback type 4 is not supported):
 *      4 bytes - CBCP header
 *    + 2 bytes - Callback type 1
 *    + 5 bytes - Callback type 2
 *    + 3 bytes - Callback type 3
 *    ---------
 *     14 bytes.
 */
    packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                   14,
                                   TM_16BIT_ZERO);
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        errorCode = TM_ENOBUFS;
        goto CbcpSendRequestFinish;
    }

/* CBCP header. */
    cbcpHdrPtr = packetPtr->pktLinkDataPtr;

/* Concatenate the Callback types that we will allow. */
    optionOffset = TM_CBCP_OFS_PKT_DATA;
    for (counter = 0; counter < optionCount; ++counter)
    {
        optionType = cbcpStatePtr->cbcpsTypeSet[counter];
/* Write the Callback type to the packet. */
        tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_TYPE, optionType);
        switch (optionType)
        {
/* No callback. */
        case TM_CBCP_CTYPE_NOCALL:
/* Option bytes: type + length */
            tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_LENGTH, 2);
            optionOffset += 2;
            break;
/* Caller provides callback address. */
        case TM_CBCP_CTYPE_CALLER:
/* Option bytes: type + length + delay + address type + null */
            tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_LENGTH, 5);
            tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_DELAY, 0);
            tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_ADDRESS
                            + TM_CBCP_OFS_ADDR_TYPE, TM_CBCP_ATYPE_PSTN);
            tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_ADDRESS
                            + TM_CBCP_OFS_ADDR_DATA, 0);
            optionOffset += 5;
            break;
/* Answerer uses a pre-defined callback address. */
        case TM_CBCP_CTYPE_ANSWERER:
/* Option bytes: type + length + delay */
            tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_LENGTH, 3);
            tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_DELAY, 0);
            optionOffset += 3;
            break;
        default:
/* Unsupported Callback type. */
            errorCode = TM_EINVAL;
            needFreePacket = TM_8BIT_YES;
            goto CbcpSendRequestFinish;
        }
    }
/* Fill in the CBCP header. */
/* Callback code = REQUEST */
    tm_cbcp_put8(TM_CBCP_OFS_PKT_CODE, TM_CBCP_REQUEST);
/* New Identifier for each request sent */
    ++cbcpStatePtr->cbcpsRequestId;
    tm_cbcp_put8(TM_CBCP_OFS_PKT_IDENT, cbcpStatePtr->cbcpsRequestId);
/* Length of data */
    packetPtr->pktLinkDataLength = optionOffset;
    tm_cbcp_put16(TM_CBCP_OFS_PKT_LENGTH, optionOffset);
/* Start our retry timer */
    tfCbcpTimerAdd(pppVectPtr, cbcpStatePtr->cbcpsRetryTimeout);
/* Send the packet. */
    tfPppDeviceSend(pppVectPtr, packetPtr, TM_PPP_CBCP_PROTOCOL);

CbcpSendRequestFinish:
    if (needFreePacket && packetPtr != TM_PACKET_NULL_PTR)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
    return errorCode;
}

/**
 ** Create a CBCP config response packet and send it.
 **/
static int tfCbcpSendResponse(  ttPppVectPtr    pppVectPtr,
                                ttPacketPtr     packetPtr )
{
    ttCallbackStatePtr      cbcpStatePtr;
    tt8BitPtr               cbcpHdrPtr;
    tt8BitPtr               addressPtr;
    tt8BitPtr               bytePtr;
    int                     errorCode;
    unsigned int            optionOffset;
    unsigned int            addressLength;
    unsigned int            counter;
    unsigned int            packetLength;
    unsigned int            packedBytes;
    tt8Bit                  needFreePacket;
    tt8Bit                  optionType;
    tt8Bit                  delay;


    errorCode = TM_ENOERROR;
    needFreePacket = TM_8BIT_NO;

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
/* An empty list of options should not occur. */
    if (cbcpStatePtr->cbcpsTypeSetCount == 0)
    {
        goto CbcpSendResponseFinish;
    }
/* Allocate a packet. */
    addressLength = cbcpStatePtr->cbcpsAddressSetLength;
    packetLength = TM_CBCP_OFS_PKT_DATA + TM_CBCP_OFS_OPT_ADDRESS
                                                    + addressLength;
    packedBytes = tm_packed_byte_count(packetLength);
    packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                   packedBytes,
                                   TM_16BIT_ZERO);
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        errorCode = TM_ENOBUFS;
        goto CbcpSendResponseFinish;
    }

/* CBCP header. */
    cbcpHdrPtr = packetPtr->pktLinkDataPtr;

/* We respond with only one Callback type. */
    delay = cbcpStatePtr->cbcpsDelaySet;
    optionOffset = TM_CBCP_OFS_PKT_DATA;
    optionType = cbcpStatePtr->cbcpsTypeRespond;
/* Write the Callback type into the packet. */
    tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_TYPE, optionType);
/* Write type-specific information. */
    switch (optionType)
    {
/* No callback. */
    case TM_CBCP_CTYPE_NOCALL:
/* Option bytes: type + length */
        tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_LENGTH, 2);
        optionOffset += 2;
        break;

/* Caller provides callback address. */
    case TM_CBCP_CTYPE_CALLER:
/* Option bytes: type + length + delay + address */
        tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_LENGTH,
                                                    3 + addressLength);
        tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_DELAY, delay);
        optionOffset += TM_CBCP_OFS_OPT_ADDRESS;
        addressPtr = cbcpStatePtr->cbcpsAddressSetPtr;
        for (counter = 0; counter < addressLength; ++counter)
        {
            tm_cbcp_put8(optionOffset + counter, addressPtr[counter]);
        }
        optionOffset += addressLength;
        break;

/* Answerer uses a pre-defined callback address. */
    case TM_CBCP_CTYPE_ANSWERER:
/* Option bytes: type + length + delay */
        tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_LENGTH, 3);
        tm_cbcp_put8(optionOffset + TM_CBCP_OFS_OPT_DELAY, delay);
        optionOffset += 3;
        break;

    default:
/* Unsupported Callback type. */
        errorCode = TM_EINVAL;
        needFreePacket = TM_8BIT_YES;
        goto CbcpSendResponseFinish;
    }
/* Fill in the CBCP header. */
/* Callback code = RESPONSE */
    tm_cbcp_put8(TM_CBCP_OFS_PKT_CODE, TM_CBCP_RESPONSE);
/* Identifier (from received request) */
    tm_cbcp_put8(TM_CBCP_OFS_PKT_IDENT, cbcpStatePtr->cbcpsRespondId);
/* Total length of data */
    packetPtr->pktLinkDataLength = optionOffset;
    tm_cbcp_put16(TM_CBCP_OFS_PKT_LENGTH, optionOffset);
/* Copy response data so we can check it against the ack we receive. */
    bytePtr = cbcpStatePtr->cbcpsResponseDataPtr;
    packedBytes = tm_packed_byte_count(optionOffset);
    if (cbcpStatePtr->cbcpsResponseDataLength < optionOffset)
    {
        if (bytePtr != TM_8BIT_NULL_PTR)
        {
            tm_free_raw_buffer((ttRawBufferPtr)bytePtr);
        }
        bytePtr = (tt8BitPtr)tm_get_raw_buffer(packedBytes);
        cbcpStatePtr->cbcpsResponseDataPtr = bytePtr;
        if (bytePtr == TM_8BIT_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
            needFreePacket = TM_8BIT_YES;
            goto CbcpSendResponseFinish;
        }
    }
    cbcpStatePtr->cbcpsResponseDataLength = (tt16Bit)optionOffset;
    tm_bcopy(cbcpHdrPtr, bytePtr, packedBytes);
/* Start our retry timer */
    tfCbcpTimerAdd(pppVectPtr, cbcpStatePtr->cbcpsRetryTimeout);
/* Send the packet. */
    tfPppDeviceSend(pppVectPtr, packetPtr, TM_PPP_CBCP_PROTOCOL);

CbcpSendResponseFinish:
    if (needFreePacket && packetPtr != TM_PACKET_NULL_PTR)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
    return errorCode;
}

/**
 ** Create a CBCP config ack packet and send it.
 **/
static int tfCbcpSendAck(   ttPppVectPtr    pppVectPtr,
                            ttPacketPtr     packetPtr )
{
    ttCallbackStatePtr      cbcpStatePtr;
    tt8BitPtr               ackDataPtr;
    tt8BitPtr               bytePtr;
    unsigned int            dataLength;
    unsigned int            dataOffset;
    int                     errorCode;
    tt16Bit                 packedBytes;


    errorCode = TM_ENOERROR;

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
/* Any data to send? */
    ackDataPtr = cbcpStatePtr->cbcpsAckDataPtr;
    if (ackDataPtr == TM_8BIT_NULL_PTR)
    {
        goto CbcpSendAckFinish;
    }
/* If we are acknowledging a user-specified address, unpack the
 * address so that we can provide it to the user for dialing. */
    if (cbcpStatePtr->cbcpsTypeAck == (tt8Bit)TM_CBCP_CTYPE_CALLER)
    {
/* Derive the length of the address from the length of the Callback option. */
        dataOffset = TM_CBCP_OFS_PKT_DATA;
        dataLength = tm_ppp_get_char_index(ackDataPtr, dataOffset
                                            + TM_CBCP_OFS_OPT_LENGTH);
        dataOffset += TM_CBCP_OFS_OPT_ADDRESS;
        dataLength -= TM_CBCP_OFS_OPT_ADDRESS;
/* Allocate storage for the unpacked address. */
        bytePtr = cbcpStatePtr->cbcpsAddressCfgPtr;
        if (cbcpStatePtr->cbcpsAddressCfgLength < dataLength)
        {
            if (bytePtr != TM_8BIT_NULL_PTR)
            {
                tm_free_raw_buffer((ttRawBufferPtr)bytePtr);
            }
            bytePtr = (tt8BitPtr)tm_get_raw_buffer(dataLength);
            cbcpStatePtr->cbcpsAddressCfgPtr = bytePtr;
            if (bytePtr == TM_8BIT_NULL_PTR)
            {
                errorCode = TM_ENOBUFS;
                goto CbcpSendAckFinish;
            }
        }
        cbcpStatePtr->cbcpsAddressCfgLength = (tt16Bit)dataLength;
/* Unpack the address. */
        while (dataLength-- > 0)
        {
            *bytePtr++ = (tt8Bit)tm_ppp_get_char_index(ackDataPtr,
                                                            dataOffset);
            ++dataOffset;
        }
    }
/* Allocate a packet. */
    packedBytes = tm_packed_byte_count(cbcpStatePtr->cbcpsAckDataLength);
    packetPtr = tfGetSharedBuffer( TM_PPP_ALLOC_HDR_BYTES,
                                   packedBytes,
                                   TM_16BIT_ZERO );
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        errorCode = TM_ENOBUFS;
        goto CbcpSendAckFinish;
    }
/* Copy the ack data to the packet. */
    tm_bcopy(ackDataPtr, packetPtr->pktLinkDataPtr, packedBytes);
/* Start our retry timer */
    tfCbcpTimerAdd(pppVectPtr, cbcpStatePtr->cbcpsRetryTimeout);
/* Send the packet. */
    tfPppDeviceSend(pppVectPtr, packetPtr, TM_PPP_CBCP_PROTOCOL);

CbcpSendAckFinish:
    return errorCode;
}

/**
 ** Add a timer that will call the CBCP timeout function.
 **/
static void tfCbcpTimerAdd( ttPppVectPtr    pppVectPtr,
                            unsigned int    expireSeconds )
{
    ttCallbackStatePtr  cbcpStatePtr;
    ttTimerPtr          timerPtr;
    ttGenericUnion      timerParm1;

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
    if (cbcpStatePtr->cbcpsTmrPtr == TM_TMR_NULL_PTR)
    {
        timerParm1.genVoidParmPtr = (ttVoidPtr)pppVectPtr;
        timerPtr = tfTimerAdd( tfCbcpTimeout,
                               timerParm1,
                               timerParm1, /* unused */
                               expireSeconds * TM_UL(1000),
                               0);
        if (timerPtr != TM_TMR_NULL_PTR)
        {
            cbcpStatePtr->cbcpsTmrPtr = timerPtr;
        }
    }
}

/**
 ** Remove the CBCP timer.
 **/
static void tfCbcpTimerRemove(ttPppVectPtr pppVectPtr)
{
    ttCallbackStatePtr  cbcpStatePtr;

    cbcpStatePtr = &pppVectPtr->pppsCallbackState;
    if (cbcpStatePtr->cbcpsTmrPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(cbcpStatePtr->cbcpsTmrPtr);
        cbcpStatePtr->cbcpsTmrPtr = TM_TMR_NULL_PTR;
    }
}

/**
 ** Request an action or provide notification of an event
 ** associated with the PPP Callback option.
 **/
int tfPppCallbackIoctl( ttUserInterface interfaceHandle,
                        int             type,
                        char TM_FAR *   dataPtr,
                        int             dataLength )
{
    int                 errorCode;
    ttDeviceEntryPtr    devPtr;
    ttPppVectPtr        pppVectPtr;

    TM_UNREF_IN_ARG(dataPtr);
    TM_UNREF_IN_ARG(dataLength);

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(devPtr);
    if (errorCode == TM_ENOERROR)
    {
/* Lock the device. */
        tm_call_lock_wait(&(devPtr->devLockEntry));
        if (tm_ll_uses_ppp(devPtr))
        {
            pppVectPtr = (ttPppVectPtr)devPtr->devStateVectorPtr;

            switch (type)
            {
/*** Connection established at the physical layer. ***/
            case TM_PPP_USER_EVENT_CONNECTED:
/* The physical layer is up. */
                errorCode = tfCbcpStateMachine( pppVectPtr,
                                                TM_PACKET_NULL_PTR,
                                                TM_CBCPE_PHY_UP );
                break;
            default:
                errorCode = TM_EINVAL;
            }
        }
        else
        {
            errorCode = TM_EPERM;
        }
/* Unlock the device. */
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/**
 ** Register a user function to receive PPP Callback requests and
 ** notifications.
 **/
int tfPppCallbackRegister(  ttUserInterface             interfaceHandle,
                            ttPppCallbackUserFunctPtr   funcPtr )
{
    int                 errorCode;
    ttDeviceEntryPtr    devPtr;
    ttPppVectPtr        pppVectPtr;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(devPtr);
    if (errorCode == TM_ENOERROR)
    {
/* Lock the device. */
        tm_call_lock_wait(&(devPtr->devLockEntry));
        if (tm_ll_uses_ppp(devPtr))
        {
            pppVectPtr = (ttPppVectPtr)devPtr->devStateVectorPtr;
            pppVectPtr->pppsCallbackState.cbcpsUserFuncPtr = funcPtr;
        }
        else
        {
            errorCode = TM_EPERM;
        }
/* Unlock the device. */
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}
#endif /* TM_USE_PPP_CALLBACK */

#else /* !TM_USE_PPP */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_PPP is not defined */
int tvPppDummy = 0;
#endif /* TM_USE_PPP */

/***************** End Of File *****************/

