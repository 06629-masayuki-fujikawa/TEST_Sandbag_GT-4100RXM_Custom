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
 * Description: Compress Library .h file
 *
 * Filename: trcomplib.h
 * Author: Jin Zhang
 * Date Created: 08/01/2001
 * $Source: include/trcomplib.h $
 *
 * Modification History
 * $Revision: 6.0.1.1 $
 * $Date: 2010/01/18 19:30:28GMT+09:00 $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRCOMPLIB_H_
#define _TRCOMPLIB_H_


#ifdef __cplusplus
extern "C" {
#endif

#define TM_COMP_SIZE       3000
#define TM_DECOMP_SIZE     3000


typedef int (*ttIpsecPktCompDecompFuncPtr)(ttPacketPtr packetPtr, 
                                           ttPktLen    offData,
                                           tt8BitPtr   compDecompPtr,
                                           tt32BitPtr  compDecompOutLenPtr,
                                           ttPktLen    payloadLen,
                                           tt32Bit     compAlg,
                                           tt32Bit     len);


TM_PROTO_EXTERN int tfCompEngineInit(void);
TM_PROTO_EXTERN void tfCompEngineUninitialize(void);


TM_PROTO_EXTERN int tfIpsecPacketComp(ttPacketPtr  packetPtr,
                                      ttPktLen     offData,
                                      tt8BitPtr    compDecompPtr,
                                      tt32BitPtr   compDecompLenPtr,
                                      ttPktLen     payloadLen,
                                      tt32Bit      compAlg,
                                      tt32Bit      len);


TM_PROTO_EXTERN int tfIpsecPacketDeComp(ttPacketPtr  packetPtr,
                                        ttPktLen     offData,
                                        tt8BitPtr    compDecompPtr,
                                        tt32BitPtr   compDecompLenPtr,
                                        ttPktLen     payloadLen,
                                        tt32Bit      compAlg,
                                        tt32Bit      len);

TM_PROTO_EXTERN void tfCompDebugPrintf(ttUserCharPtrPtr  printBufPtrPtr,
                                         ttUser8BitPtr  dataPtr,
                                         ttUser32Bit    dataLength,
                                         ttUserIntPtr   offsetPtr);

TM_PROTO_EXTERN void tfCompDebugFlush(ttUserCharPtrPtr printBufPtrPtr);

#endif /* _TRCOMPLIB_H_ */
