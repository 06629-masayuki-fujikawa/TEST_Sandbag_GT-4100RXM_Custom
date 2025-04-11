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
 * Description: Header file for Keil compilers huge memory type library functions
 *
 * Filename: trkeil.h
 * Author: Bryan Wang
 * Date Created: 11/1/2003
 * $Source: include/trkeil.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:25JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRKEIL_H_

#define _TRKEIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Keil library functions for huge pointer arguments */

#define tm_bcopy(a, b, c)       hmemcpy( (ttVoidPtr)(b), \
                                         (ttConstVoidPtr)(a), \
                                         (unsigned int)(c) )

#define tm_memcmp(a, b, c)      hmemcmp( (ttConstVoidPtr)(a), \
                                        (ttConstVoidPtr)(b), \
                                        (unsigned int)(c) )

#define tm_memcpy(a, b, c)      hmemcpy( (ttVoidPtr)(a), \
                                          (ttConstVoidPtr)(b), \
                                          (unsigned int)(c))

#define tm_strcmp(a, b)         hstrcmp( (ttConstCharPtr)(a), \
                                        (ttConstCharPtr)(b))

#define tm_strcpy(a, b)         (void)hstrcpy( (ttCharPtr)(a), \
                                              (ttConstCharPtr)(b))

#define tm_toupper(asciiCh)     toupper(asciiCh)
#define tm_isspace(c)           isspace(c)
#define tm_isupper(c)           isupper(c)
#define tm_islower(c)           islower(c)
#define tm_isdigit(c)           isdigit(c)
#define tm_isxdigit(c)          isxdigit(c)
#define tm_isalpha(c)           isalpha(c)
#define tm_isprint(c)           isprint(c)

#ifdef __cplusplus
}
#endif

#endif /*_TRKEIL_H_*/
