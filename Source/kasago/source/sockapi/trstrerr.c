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
 * Description: Treck strerror "C" library function
 *
 * Filename: trstrerr.c
 * Author: Odile
 * Date Created: 02/19/99
 * $Source: source/sockapi/trstrerr.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:44JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h> /* For TM_CONST_QLF */

static const char  TM_CONST_QLF tlErrorNoError[] = "No error";
static const char  TM_CONST_QLF tlErrorUnknown[] = "Unknown error code";
static const char  TM_CONST_QLF tlErrorBase1Arr[] = "Operation not permitted";
static const char  TM_CONST_QLF tlErrorBase2Arr[] =
       "No such file or directory";
static const char  TM_CONST_QLF tlErrorBase3Arr[] = "No such process";
static const char  TM_CONST_QLF tlErrorBase4Arr[] = "Interrupted system call";
static const char  TM_CONST_QLF tlErrorBase5Arr[] = "Input/output error";
static const char  TM_CONST_QLF tlErrorBase6Arr[] = "Device not configured";
static const char  TM_CONST_QLF tlErrorBase9Arr[] = "Bad file descriptor";
static const char  TM_CONST_QLF tlErrorBase10Arr[] = "No child processes";
static const char  TM_CONST_QLF tlErrorBase12Arr[] = "Cannot allocate memory";
static const char  TM_CONST_QLF tlErrorBase13Arr[] = "Permission denied";
static const char  TM_CONST_QLF tlErrorBase14Arr[] = "Bad address";
static const char  TM_CONST_QLF tlErrorBase17Arr[] = "File exists";
static const char  TM_CONST_QLF tlErrorBase19Arr[] =
       "Operation not supported by device";
static const char  TM_CONST_QLF tlErrorBase20Arr[] = "Not a directory";
static const char  TM_CONST_QLF tlErrorBase21Arr[] = "Is a directory";
static const char  TM_CONST_QLF tlErrorBase22Arr[] = "Invalid argument";
static const char  TM_CONST_QLF tlErrorBase24Arr[] = "Too many open files";
static const char  TM_CONST_QLF tlErrorBase28Arr[] = "No space left on device";
static const char  TM_CONST_QLF tlErrorBase34Arr[] =
       "Result too large or too small";
static const char  TM_CONST_QLF tlErrorNet0Arr[] = "Operation would block";
static const char  TM_CONST_QLF tlErrorNet1Arr[] = "Operation now in progress";
static const char  TM_CONST_QLF tlErrorNet2Arr[] =
       "Operation already in progress";
static const char  TM_CONST_QLF tlErrorNet3Arr[] = "Socket operation on non-socket";
static const char  TM_CONST_QLF tlErrorNet4Arr[] =
       "Destination address required";
static const char  TM_CONST_QLF tlErrorNet5Arr[] = "Message too long";
static const char  TM_CONST_QLF tlErrorNet6Arr[] =
       "Protocol wrong type for socket";
static const char  TM_CONST_QLF tlErrorNet7Arr[] = "Protocol not available";
static const char  TM_CONST_QLF tlErrorNet8Arr[] = "Protocol not supported";
static const char  TM_CONST_QLF tlErrorNet9Arr[] = "Socket type not supported";
static const char  TM_CONST_QLF tlErrorNet10Arr[] = "Operation not supported";
static const char  TM_CONST_QLF tlErrorNet11Arr[] = "Protocol family not supported";
static const char  TM_CONST_QLF tlErrorNet12Arr[] =
       "Address family not supported by protocol family";
static const char  TM_CONST_QLF tlErrorNet13Arr[] = "Address already in use";
static const char  TM_CONST_QLF tlErrorNet14Arr[] =
       "Can't assign requested address";
static const char  TM_CONST_QLF tlErrorNet15Arr[] = "Network is down";
static const char  TM_CONST_QLF tlErrorNet16Arr[] = "Network is unreachable";
static const char  TM_CONST_QLF tlErrorNet17Arr[] =
       "Network dropped connection on reset";
static const char  TM_CONST_QLF tlErrorNet18Arr[] =
       "Software caused connection abort";
static const char  TM_CONST_QLF tlErrorNet19Arr[] = "Connection reset by peer";
static const char  TM_CONST_QLF tlErrorNet20Arr[] =
       "No buffer space available";
static const char  TM_CONST_QLF tlErrorNet21Arr[] =
       "Socket is already connected";
static const char  TM_CONST_QLF tlErrorNet22Arr[] = "Socket is not connected";
static const char  TM_CONST_QLF tlErrorNet23Arr[] =
       "Can't send after socket shutdown";
static const char  TM_CONST_QLF tlErrorNet24Arr[] =
       "Too many references: can't splice";
static const char  TM_CONST_QLF tlErrorNet25Arr[] = "Operation timed out";
static const char  TM_CONST_QLF tlErrorNet26Arr[] = "Connection refused";
static const char  TM_CONST_QLF tlErrorNet29Arr[] = "Host is down";
static const char  TM_CONST_QLF tlErrorNet30Arr[] = "No route to host";

/* Error extensions to handle ICMP errors */
static const char  TM_CONST_QLF tlErrorExt0Arr[] =
       "ICMP type unreach with code >= 13";
static const char  TM_CONST_QLF tlErrorExt1Arr[] = "ICMP source quench";
static const char  TM_CONST_QLF tlErrorExt2Arr[] =
       "ICMP time xceeded in transit";
static const char  TM_CONST_QLF tlErrorExt3Arr[] =
       "ICMP time xceeded in reassemly";
/* Additional error codes for FTP application program interface */
static const char  TM_CONST_QLF tlErrorExt4Arr[] =
       "Command requires user to be loggedin, and user is not.";
static const char  TM_CONST_QLF tlErrorExt5Arr[] =
       "Temporary server error. Re-sending the same command could succeed";
static const char  TM_CONST_QLF tlErrorExt6Arr[] =
       "Permanent server error. Re-sending the same command will always fail";
static const char  TM_CONST_QLF tlErrorExt7Arr[] =
       "Server replies are out of SYNC. FTP user should call tfFtpQuit()";
static const char  TM_CONST_QLF tlErrorExt8Arr[] =
       "ICMPv6 error of unknown type";
/* Additional Posix generic protocol error */
static const char  TM_CONST_QLF tlErrorExt9Arr[] = "Protocol error";

#ifdef TM_USE_IPSEC
static const char  TM_CONST_QLF tlErrorIpsec0Arr[] =
       "IPsec policy not found. Add IPsec policy.";
static const char  TM_CONST_QLF tlErrorIpsec1Arr[] =
       "IPsec SA not found. Add SA manually or start IKE.";
static const char  TM_CONST_QLF tlErrorIpsec2Arr[] =
       "IPsec policy requires to discard the packet.";
static const char  TM_CONST_QLF tlErrorIpsec3Arr[] =
       "IPsec AH send path fails, check the AH policy content and SAs";
static const char  TM_CONST_QLF tlErrorIpsec4Arr[] =
       "IPsec ESP send path fails, check the ESP policy content and SAs";
static const char  TM_CONST_QLF tlErrorIpsec5Arr[] =
       "SA or Bundle SA needed. Add SA manually or start IKE.";
static const char  TM_CONST_QLF tlErrorIpsec6Arr[] =
       "IPsec requires seperate head| shared buffer. Failed to allocate.";
static const char  TM_CONST_QLF tlErrorIpsec7Arr[] =
       "Send path fails. Not supported IPsec protocol";
static const char  TM_CONST_QLF tlErrorIpsec8Arr[] =
       "Send packet is queued, waiting for IKE to return.";
static const char  TM_CONST_QLF tlErrorIpsec9Arr[] =
    "Indicates partial cleanup of an operation failing to add a resource.";
static const char  TM_CONST_QLF tlErrorIpsec10Arr[] =
    "Indicates an SA insertion failed due to SPI/CPI collision.";
static const char  TM_CONST_QLF tlErrorIpsec11Arr[] =
    "Indicates that the SPI/CPI value is not in the proper range.";
#ifdef TM_SA_PMTU
static const char  TM_CONST_QLF tlErrorIpsec12Arr[] =
    "SA's PMTU sucessfully updated.";
static const char  TM_CONST_QLF tlErrorIpsec13Arr[] =
    "SA's PMTU sucessfully verified.";
#endif /* TM_SA_PMTU */
static const char  TM_CONST_QLF tlErrorIpsec28Arr[] =
    "icv not match.";
static const char  TM_CONST_QLF tlErrorIpsec31Arr[] =
    "data size is not multiple of blocksize.";
static const char  TM_CONST_QLF tlErrorIpsec32Arr[] =
    "pad data error.";
static const char  TM_CONST_QLF tlErrorIpsec33Arr[] =
    "replay bitmap error.";
static const char  TM_CONST_QLF tlErrorIpsec34Arr[] =
    "sequence number is zero.";
static const char  TM_CONST_QLF tlErrorIpsec35Arr[] =
    "too old seq.";
static const char  TM_CONST_QLF tlErrorIpsec36Arr[] =
    "overflowed sequence.";
static const char  TM_CONST_QLF tlErrorIpsec37Arr[] =
    "sequence is already seen.";
static const char  TM_CONST_QLF tlErrorIpsec41Arr[] =
    "inputLen not a multiple of block size.";
static const char  TM_CONST_QLF tlErrorIpsec42Arr[] =
    "invalid parameters.";
static const char  TM_CONST_QLF tlErrorIpsec43Arr[] =
    "invalid IV text.";
static const char  TM_CONST_QLF tlErrorIpsec44Arr[] =
    "incorrect 32-bit alignment.";
static const char  TM_CONST_QLF tlErrorIpsec45Arr[] =
    "no spi available.";
static const char  TM_CONST_QLF tlErrorIpsec46Arr[] =
    "not valid spi.";
static const char  TM_CONST_QLF tlErrorIpsec47Arr[] =
    "sadb extention already there.";
static const char  TM_CONST_QLF tlErrorIpsec48Arr[] =
    "key parity not good.";
static const char  TM_CONST_QLF tlErrorIpsec49Arr[] =
    "weak key or semi-weak key.";
static const char  TM_CONST_QLF tlErrorIpsec50Arr[] =
    "Encryption error.";
static const char  TM_CONST_QLF tlErrorIpsec51Arr[] =
    "Decryption error.";
static const char  TM_CONST_QLF tlErrorIpsec52Arr[] =
    "twofish requires.";
static const char  TM_CONST_QLF tlErrorIpsec53Arr[] =
    "schedule length negative.";
static const char  TM_CONST_QLF tlErrorIpsec54Arr[] =
    "block size greate than max IV size.";
static const char  TM_CONST_QLF tlErrorIpsec55Arr[] =
    "ipsec not initialized.";
static const char  TM_CONST_QLF tlErrorIpsec56Arr[] =
    "ike not started.";
static const char  TM_CONST_QLF tlErrorIpsec57Arr[] =
    "not supported extention.";
static const char  TM_CONST_QLF tlErrorIpsec58Arr[] =
    "3DES k1=k2 or k2=k3.";
static const char  TM_CONST_QLF tlErrorIpsec59Arr[] =
    "Policy is invalid , for example, both tunnel and transport bit set.";
static const char  TM_CONST_QLF tlErrorIpsec60Arr[] =
    "the SADBRECORD check fails.";
static const char  TM_CONST_QLF tlErrorIpsec61Arr[] =
    "specified spi is not found.";
static const char  TM_CONST_QLF tlErrorIpsec62Arr[] =
    "direction is not inbound ||outbound ||both.";
static const char  TM_CONST_QLF tlErrorIpsec63Arr[] =
    "need to discard this packet.";
static const char  TM_CONST_QLF tlErrorIpsec64Arr[] =
    "Recycled message ID found.";
#endif /* TM_USE_IPSEC*/

/* 
 * Error macros (added TM_ERR_BASE to BSD 4.4 sys\types.h definitions) +
 * skipped unneeded errors.
 */
static const char TM_FAR * const  TM_CONST_QLF tlTreckNetError
                                     [TM_ERR_BASE_LAST - TM_EWOULDBLOCK + 1] =
{
    &tlErrorNet0Arr[0],
    &tlErrorNet1Arr[0],
    &tlErrorNet2Arr[0],
    &tlErrorNet3Arr[0],
    &tlErrorNet4Arr[0],
    &tlErrorNet5Arr[0],
    &tlErrorNet6Arr[0],
    &tlErrorNet7Arr[0],
    &tlErrorNet8Arr[0],
    &tlErrorNet9Arr[0],
    &tlErrorNet10Arr[0],
    &tlErrorNet11Arr[0],
    &tlErrorNet12Arr[0],
    &tlErrorNet13Arr[0],
    &tlErrorNet14Arr[0],
    &tlErrorNet15Arr[0],
    &tlErrorNet16Arr[0],
    &tlErrorNet17Arr[0],
    &tlErrorNet18Arr[0],
    &tlErrorNet19Arr[0],
    &tlErrorNet20Arr[0],
    &tlErrorNet21Arr[0],
    &tlErrorNet22Arr[0],
    &tlErrorNet23Arr[0],
    &tlErrorNet24Arr[0],
    &tlErrorNet25Arr[0],
    &tlErrorNet26Arr[0],
    &tlErrorUnknown[0],
    &tlErrorUnknown[0],
    &tlErrorNet29Arr[0],
    &tlErrorNet30Arr[0]
};

static const char TM_FAR * const  TM_CONST_QLF tlTreckExtError
                            [TM_EXT_ERROR_LAST - TM_EXT_ERROR_BASE] =
{
/* Error extensions to handle ICMP errors */
    &tlErrorExt0Arr[0],
    &tlErrorExt1Arr[0],
    &tlErrorExt2Arr[0],
    &tlErrorExt3Arr[0],
/* Additional error codes for FTP server application program interface */
    &tlErrorExt4Arr[0],
    &tlErrorExt5Arr[0],
    &tlErrorExt6Arr[0],
    &tlErrorExt7Arr[0],
/* Error extension to handle ICMPv6 errors */
    &tlErrorExt8Arr[0],
/* POSIX error codes that aren't yet implemented by BSD */
    &tlErrorExt9Arr[0]
};

#ifdef TM_USE_IPSEC
static const char TM_FAR * const  TM_CONST_QLF tlTreckIpsecError
                        [TM_IPSEC_ERROR_LAST - TM_IPSEC_ERROR_BASE + 1] =
{
    &tlErrorIpsec0Arr[0],
    &tlErrorIpsec1Arr[0],
    &tlErrorIpsec2Arr[0],
    &tlErrorIpsec3Arr[0],
    &tlErrorIpsec4Arr[0],
    &tlErrorIpsec5Arr[0],
    &tlErrorIpsec6Arr[0],
    &tlErrorIpsec7Arr[0],
    &tlErrorIpsec8Arr[0],
    &tlErrorIpsec9Arr[0],
    &tlErrorIpsec10Arr[0],
    &tlErrorIpsec11Arr[0],
#ifdef TM_SA_PMTU
    &tlErrorIpsec12Arr[0],
    &tlErrorIpsec13Arr[0],
#else  /* TM_SA_PMTU */
    &tlErrorUnknown[0],          /* 12 */
    &tlErrorUnknown[0],          /* 13 */
#endif /* TM_SA_PMTU */
    &tlErrorUnknown[0],          /* 14 */
    &tlErrorUnknown[0],          /* 15 */    
    &tlErrorUnknown[0],          /* 16 */
    &tlErrorUnknown[0],          /* 17 */
    &tlErrorUnknown[0],          /* 18 */
    &tlErrorUnknown[0],          /* 19 */
    &tlErrorUnknown[0],          /* 20 */
    &tlErrorUnknown[0],          /* 21 */  
    &tlErrorUnknown[0],          /* 22 */    
    &tlErrorUnknown[0],          /* 23 */
    &tlErrorUnknown[0],          /* 24 */
    &tlErrorUnknown[0],          /* 25 */
    &tlErrorUnknown[0],          /* 26 */
    &tlErrorUnknown[0],          /* 27 */
    &tlErrorIpsec28Arr[0],
    &tlErrorUnknown[0],          /* 29 */
    &tlErrorUnknown[0],          /* 30 */
    &tlErrorIpsec31Arr[0],
    &tlErrorIpsec32Arr[0],
    &tlErrorIpsec33Arr[0],
    &tlErrorIpsec34Arr[0],
    &tlErrorIpsec35Arr[0],
    &tlErrorIpsec36Arr[0],
    &tlErrorIpsec37Arr[0],
    &tlErrorUnknown[0],          /* 38 */
    &tlErrorUnknown[0],          /* 39 */
    &tlErrorUnknown[0],          /* 40 */
    &tlErrorIpsec41Arr[0],
    &tlErrorIpsec42Arr[0],
    &tlErrorIpsec43Arr[0],
    &tlErrorIpsec44Arr[0],
    &tlErrorIpsec45Arr[0],
    &tlErrorIpsec46Arr[0],
    &tlErrorIpsec47Arr[0],
    &tlErrorIpsec48Arr[0],
    &tlErrorIpsec49Arr[0],
    &tlErrorIpsec50Arr[0],
    &tlErrorIpsec51Arr[0],
    &tlErrorIpsec52Arr[0],
    &tlErrorIpsec53Arr[0],
    &tlErrorIpsec54Arr[0],
    &tlErrorIpsec55Arr[0],
    &tlErrorIpsec56Arr[0],
    &tlErrorIpsec57Arr[0],
    &tlErrorIpsec58Arr[0],
    &tlErrorIpsec59Arr[0],
    &tlErrorIpsec60Arr[0],
    &tlErrorIpsec61Arr[0],
    &tlErrorIpsec62Arr[0],
    &tlErrorIpsec63Arr[0],
    &tlErrorIpsec64Arr[0]
};
#endif /* TM_USE_IPSEC */

/* 
 * Map error code to an error string.
 */
char TM_FAR * tfStrError( int errorCode )
{
    const char TM_FAR * retPtr;

    retPtr = &tlErrorUnknown[0];
    if (errorCode > TM_ERR_BASE)
    {
        if (errorCode <= TM_ERR_IO_LAST)
/* First set of errors */
        {
            switch (errorCode)
            {
/* Operation not permitted */
                case TM_EPERM:
                    retPtr = &tlErrorBase1Arr[0];
                    break;
/* No such file or directory */
                case TM_ENOENT:
                    retPtr = &tlErrorBase2Arr[0];
                    break;
/* No such process */
                case TM_ESRCH:
                    retPtr = &tlErrorBase3Arr[0];
                    break;
/* Interrupted system call */
                case TM_EINTR:
                    retPtr = &tlErrorBase4Arr[0];
                    break;
/* Input/output error */
                case TM_EIO:
                    retPtr = &tlErrorBase5Arr[0];
                    break;
/* Device not configured */
                case TM_ENXIO:
                    retPtr = &tlErrorBase6Arr[0];
                    break;
/* Bad file descriptor */
                case TM_EBADF:
                    retPtr = &tlErrorBase9Arr[0];
                    break;
/* No child processes */
                case TM_ECHILD:
                    retPtr = &tlErrorBase10Arr[0];
                    break;
/* Cannot allocate memory */
                case TM_ENOMEM:
                    retPtr = &tlErrorBase12Arr[0];
                    break;
/* Permission denied */
                case TM_EACCES:
                    retPtr = &tlErrorBase13Arr[0];
                    break;
/* Bad address */
                case TM_EFAULT:
                    retPtr = &tlErrorBase14Arr[0];
                    break;
/* File exists */
                case TM_EEXIST:
                    retPtr = &tlErrorBase17Arr[0];
                    break;
/* Operation not supported by device */
                case TM_ENODEV:
                    retPtr = &tlErrorBase19Arr[0];
                    break;
/* Not a directory */
            case TM_ENOTDIR:
                    retPtr = &tlErrorBase20Arr[0];
                    break;
/* Is a directory */
                case TM_EISDIR:
                    retPtr = &tlErrorBase21Arr[0];
                    break;
/* Invalid argument */
                case TM_EINVAL:
                    retPtr = &tlErrorBase22Arr[0];
                    break;
/* Too many open files */
                case TM_EMFILE:
                    retPtr = &tlErrorBase24Arr[0];
                    break;
/* No space left on device */
                case TM_ENOSPC:
                    retPtr = &tlErrorBase28Arr[0];
                    break;
/* result too large (range error) */
                case TM_ERANGE:
                    retPtr = &tlErrorBase34Arr[0];
                    break;
                default:
                    retPtr = &tlErrorUnknown[0];
                    break;
            }
        }
        else
        {
#if (TM_EWOULDBLOCK != TM_ERR_IO_LAST+1)
            if (errorCode >= TM_EWOULDBLOCK)
#endif /* (TM_EWOULDBLOCK != TM_ERR_IO_LAST+1) */
            {
                if (errorCode <= TM_ERR_BASE_LAST)
/* Network errors */
                {
                    retPtr = tlTreckNetError[errorCode - TM_EWOULDBLOCK];
                }
                else
                {
                    if (errorCode > TM_EXT_ERROR_BASE)
                    {
                        if (errorCode <= TM_EXT_ERROR_LAST)
                        {
/* Extended errors (for ICMP, and FTP server application program interface) */
                            retPtr = tlTreckExtError[   errorCode
                                               - (TM_EXT_ERROR_BASE + 1)];
                        }
#ifdef TM_USE_IPSEC
                        else
                        {
                            if(errorCode >= TM_IPSEC_ERROR_BASE)
                            {
                                if(errorCode <= TM_IPSEC_ERROR_LAST)
                                {
                                    retPtr = tlTreckIpsecError[errorCode
                                            - TM_IPSEC_ERROR_BASE];
                                }
                            }
                        }
#endif /* TM_USE_IPSEC */
                    }
                }
            }
        }
    }
    else if(errorCode == TM_ENOERROR)
    {
        retPtr = &tlErrorNoError[0];
    }

    return (char TM_FAR *)retPtr;
}
