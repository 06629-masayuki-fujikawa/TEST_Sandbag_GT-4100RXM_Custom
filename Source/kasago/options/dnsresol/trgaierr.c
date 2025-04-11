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
 * Description: gai_strerror
 *
 * Filename: trgaierr.c
 * Author: Jason Schmidlapp
 * Date Created: 1/16/2003
 * $Source: source/sockapi/trgaierr.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:11JST $
 * $Author: odile $
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

/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */

/* local variable definitions */

/* static const tables */

/* EAI_AGAIN */
const char  TM_CONST_QLF tlErrorEai1Arr[] =
"The name could not be resolved at this time. Future attempts may succeed.";

/* EAI_BADFLAGS */
const char  TM_CONST_QLF tlErrorEai2Arr[] =
"The flags parameter had an invalid value.";

/* EAI_FAIL */
const char  TM_CONST_QLF tlErrorEai3Arr[] = 
"A non-recoverable error occurred when attempting to resolve the name.";

/* EAI_FAMILY */
const char  TM_CONST_QLF tlErrorEai4Arr[] =
"The address family was not recognized.";

/* EAI_MEMORY */
const char  TM_CONST_QLF tlErrorEai5Arr[] =
"There was a memory allocation failure when trying to allocate storage \
for the return value.";

/* EAI_NONAME */
const char  TM_CONST_QLF tlErrorEai6Arr[] =
"The name does not resolve for the supplied parameters. Neither nodename \
 nor servname were supplied. At least one of these must be supplied.";

/* EAI_SERVICE */
const char  TM_CONST_QLF tlErrorEai7Arr[] =
"The service passed was not recognized for the specified socket type.";

/* EAI_SOCKTYPE */
const char  TM_CONST_QLF tlErrorEai8Arr[] =
"The intended socket type was not recognized.";

/* EAI_SYSTEM */
const char  TM_CONST_QLF tlErrorEai9Arr[] =
"A system error occurred";

/* EAI_OVERFLOW */
const char  TM_CONST_QLF tlErrorEai10Arr[] =
"An argument buffer overflowed.";

/* Unknown error */
const char  TM_CONST_QLF tlErrorEaiUnknownArr[] =
"Unknown error.";

#define TM_NUM_EAI_ERRORS 10

/*
 * Table of error strings for EAI_XXX error codes.
 * Arranged in reverse order, since EAI_XXX error codes are negative values.
 */ 
const char TM_FAR * const  TM_CONST_QLF tlTreckEaiError[TM_NUM_EAI_ERRORS+1] =
{
    &tlErrorEai10Arr[0],
    &tlErrorEai9Arr[0],
    &tlErrorEai8Arr[0],
    &tlErrorEai7Arr[0],
    &tlErrorEai6Arr[0],
    &tlErrorEai5Arr[0],
    &tlErrorEai4Arr[0],
    &tlErrorEai3Arr[0],
    &tlErrorEai2Arr[0],
    &tlErrorEai1Arr[0],
    &tlErrorEaiUnknownArr[0]
};

/* macro definitions */


/*
 * gai_strerror Function Description
 * Returns a string describing the error code, returned from getaddrinfo
 * or getnameinfo.
 *
 * Parameters
 * Parameter    Description
 * errorCode    EAI_xxx error code.
 *
 * Returns
 * Pointer to string buffer describing the error.
 *
 * 1. From the specified error code, compute the offset into the EAI error
 *    string table.
 * 2. Return a pointer to the corresponding error code string.  If the error
 *    code is unrecognized, return a pointer to the string indicating an
 *    unknown error.
 */    
char TM_FAR * gai_strerror(int errorCode)
{
    int errorStringIndex;
    
    if ( (errorCode > EAI_AGAIN) || (errorCode < EAI_OVERFLOW) )
    {
/* Last entry in table is "unknown error" */
        errorStringIndex = TM_NUM_EAI_ERRORS;
    }
    else
    {
        errorStringIndex = errorCode + TM_NUM_EAI_ERRORS;
    }

    return (char TM_FAR *) tlTreckEaiError[errorStringIndex];
}

/***************** End Of File *****************/
