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
 * Implements enhanced logging design including the ability to change the logging
 * levels for modules at runtime.
 *
 * Filename: trloglvl.c
 * Author: Tim Carney
 * Date Created: 05/30/2006
 * $Source: source/trloglvl.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:56JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#ifdef TM_USE_LOGGING_LEVELS
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#ifndef tm_log_level_write
static int tfTreckLogWrite(tt32Bit module, 
                           tt8Bit level, 
                           ttCharPtr msgPtr, 
                           tt32Bit length);
#endif /* tm_log_level_write */

static void tfVEnhancedLogWrite(tt32Bit module, 
                                tt8Bit level, 
                                ttConstCharPtr msgPtr, 
                                va_list argp);
static int tfLogLevelCheckOut(tt32Bit module, tt8Bit level);
static void tfLogLevelCheckIn(tt32Bit module);


static const char  TM_CONST_QLF tlTreckModulePrefix0Arr[] =
       "[GEN LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix1Arr[] =
       "[TEMP LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix2Arr[] =
       "[IKE LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix3Arr[] =
       "[IKEV2 LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix4Arr[] =
       "[IPSEC LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix5Arr[] =
       "[SECURITY LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix6Arr[] =
       "[CRYPTO LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix7Arr[] =
       "[CERT LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix8Arr[] =
       "[IKE STATS LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix9Arr[] =
       "[IPSEC STATS LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix10Arr[] =
       "[IPSEC POLICY LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix11Arr[] =
       "[PRIORITIES LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix12Arr[] =
       "[MSG/SPI/CPI LOG] ";
static const char  TM_CONST_QLF tlTreckModulePrefix13Arr[] =
       "[SSL LOG] ";

static const char TM_FAR * const  TM_CONST_QLF tlTreckModulePrefix
                        [TM_LOG_MODULE_MAX - TM_LOG_MODULE_BASE + 1] =
{
    &tlTreckModulePrefix0Arr[0],
    &tlTreckModulePrefix1Arr[0],
    &tlTreckModulePrefix2Arr[0],
    &tlTreckModulePrefix3Arr[0],
    &tlTreckModulePrefix4Arr[0],
    &tlTreckModulePrefix5Arr[0],
    &tlTreckModulePrefix6Arr[0],
    &tlTreckModulePrefix7Arr[0],
    &tlTreckModulePrefix8Arr[0],
    &tlTreckModulePrefix9Arr[0],
    &tlTreckModulePrefix10Arr[0],
    &tlTreckModulePrefix11Arr[0],
    &tlTreckModulePrefix12Arr[0],
    &tlTreckModulePrefix13Arr[0]
};


/****************************************************************************
* FUNCTION: tfLogSetLevel
*
* PURPOSE: 
*   Set the logging level on all or a singular module.
*
* PARAMETERS:
*   module:
*       The module to change the logging level on.  A list can be found
*       in trmacro.h.  TM_LOG_MODULE_ALL will change the level on all
*       modules.
*   level:
*       The level of information to be written to the log.  Valid values are:
*           TM_LOG_LEVEL_ERROR    Errors only
*           TM_LOG_LEVEL_WARNING  Errors and warnings only
*           TM_LOG_LEVEL_NOTICE   All previous levels plus notices
*           TM_LOG_LEVEL_INFO     All previous levels plus informational info
*           TM_LOG_LEVEL_DEBUG    All previous levels plus debugging info
*           TM_LOG_LEVEL_MAX      Same as TM_LOG_LEVEL_DEBUG
*   verbosity:
*       The new level of verbosity to set. Valid values are TM_LOG_VERBOSE
*       and TM_LOG_CONCISE
* RETURNS:
*   TM_ENOERROR:
*       Successful.
*   TM_EINVAL:
*       One of the parameters was invalid.
*
* NOTES:
*
****************************************************************************/
int tfLogSetLevel(tt32Bit module, tt8Bit level, tt8Bit verbosity)
{
    int errorCode;
    int index;

    errorCode = TM_ENOERROR;

#ifdef TM_ERROR_CHECKING
    if (tm_context(tvLogLevelStoragePtr) == (ttLogLevelStoragePtr)0)
    {
        tfKernelWarning("tfLogSetLevel", "context has not been initialised");
        errorCode = TM_EPERM;
    }
    else
#endif /* TM_ERROR_CHECKING */
    {
        if ((level >= TM_LOG_LEVEL_MIN)
            && (level <= TM_LOG_LEVEL_MAX)
            && ((verbosity == TM_LOG_CONCISE)
                || (verbosity == TM_LOG_VERBOSE)))
        {
            if (module <= TM_LOG_MODULE_MAX)
            {
#ifdef TM_LOCK_NEEDED
                tm_call_lock_wait(&(tm_context(tvLogLevelStoragePtr)->
                                    llsLogModuleLock[module]));
#endif /* TM_LOCK_NEEDED */
                tm_context(tvLogLevelStoragePtr)->llsLogLevelArray[module] 
                    = level;
                tm_context(tvLogLevelStoragePtr)->llsLogVerbosityArray[module] 
                    = verbosity;
#ifdef TM_LOCK_NEEDED
                tm_call_unlock(&(tm_context(tvLogLevelStoragePtr)->
                             llsLogModuleLock[module]));
#endif /* TM_LOCK_NEEDED */
            }
            else if (module == TM_LOG_MODULE_ALL)
            {
                for (index = 0; index <= TM_LOG_MODULE_MAX; index++)
                {
#ifdef TM_LOCK_NEEDED
                    tm_call_lock_wait(&(tm_context(tvLogLevelStoragePtr)->
                                        llsLogModuleLock[index]));
#endif /* TM_LOCK_NEEDED */
                    tm_context(tvLogLevelStoragePtr)->llsLogLevelArray[index] 
                        = level;
                    tm_context(tvLogLevelStoragePtr)->
                        llsLogVerbosityArray[index] = verbosity;
#ifdef TM_LOCK_NEEDED
                    tm_call_unlock(&(tm_context(tvLogLevelStoragePtr)->
                                     llsLogModuleLock[index]));
#endif /* TM_LOCK_NEEDED */
                }
            }
            else
            {
                /* module is out of the range */
                errorCode = TM_EINVAL;
            }
        }
        else
        {
            /* Invalid level or verbosity */
            errorCode = TM_EINVAL;       
        }
    }
    
    return errorCode;
}

/****************************************************************************
* FUNCTION: tfLogGetLevel
*
* PURPOSE: 
*   Get the logging level on a singular module.
*
* PARAMETERS:
*   module:
*       The module to retrieve the logging level for.
*   levelPtr:
*       A pointer to a tt8Bit value to store the corresponding logging level.
*           TM_LOG_LEVEL_ERROR    Errors only
*           TM_LOG_LEVEL_WARNING  Errors and warnings only
*           TM_LOG_LEVEL_NOTICE   All previous levels plus notices
*           TM_LOG_LEVEL_INFO     All previous levels plus informational info
*           TM_LOG_LEVEL_DEBUG    All previous levels plus debugging info
*           TM_LOG_LEVEL_MAX      Same as TM_LOG_LEVEL_DEBUG
*   verbosityPtr:
*       A pointer to a tt8Bit value to store the corresponding verbosity.
*           TM_LOG_VERBOSE
*           TM_LOG_CONCISE
* RETURNS:
*   TM_ENOERROR:
*       Successful.
*   TM_EINVAL:
*       The input module was invalid.
*
* NOTES:
*
****************************************************************************/
int tfLogGetLevel(tt32Bit module, tt8BitPtr levelPtr, tt8BitPtr verbosityPtr)
{
    int errorCode;

    errorCode = TM_ENOERROR;
    
    if ((levelPtr == (tt8BitPtr)0)
        || (verbosityPtr == (tt8BitPtr)0))
    {
        /* Invalid argument */
        errorCode = TM_EINVAL;
    }
    else
    {
        if (module <= TM_LOG_MODULE_MAX)
        {
            errorCode = TM_ENOERROR;
#ifdef TM_LOCK_NEEDED
            tm_call_lock_wait(&(tm_context(tvLogLevelStoragePtr)->
                                llsLogModuleLock[module]));
#endif /* TM_LOCK_NEEDED */
            *levelPtr = 
                tm_context(tvLogLevelStoragePtr)->llsLogLevelArray[module];
            *verbosityPtr =
                tm_context(tvLogLevelStoragePtr)->llsLogVerbosityArray[module];
#ifdef TM_LOCK_NEEDED
            tm_call_unlock(&(tm_context(tvLogLevelStoragePtr)->
                             llsLogModuleLock[module]));
#endif /* TM_LOCK_NEEDED */
        }
        else
        {
            /* module is out of the range */
            errorCode = TM_EINVAL;
        }
    }

    return errorCode;
}

/****************************************************************************
* FUNCTION: tfEnhancedLogWrite
*
* PURPOSE: 
*   Write a message to the log.
*
* PARAMETERS:
*   module:
*       The module to change the logging level on.  A list can be found
*       in trmacro.h.  TM_LOG_MODULE_ALL will change the level on all
*       modules.
*   level:
*       The level of information to be written to the log.  Valid values are:
*           TM_LOG_LEVEL_ERROR    Errors only
*           TM_LOG_LEVEL_WARNING  Errors and warnings only
*           TM_LOG_LEVEL_NOTICE   All previous levels plus notices
*           TM_LOG_LEVEL_INFO     All previous levels plus informational info
*           TM_LOG_LEVEL_DEBUG    All previous levels plus debugging info
*           TM_LOG_LEVEL_MAX      Same as TM_LOG_LEVEL_DEBUG
*   msgPtr:
*       The format control string of the message to log.
*   Additional arguments as required by the format control string:
*       Arguments for the values in the format string.
* RETURNS:
*   None
*
* NOTES:
*
****************************************************************************/
void tfEnhancedLogWrite(tt32Bit module, 
                        tt8Bit level, 
                        ttConstCharPtr msgPtr, 
                        ...)
{
    va_list     argp;    

    va_start(argp, msgPtr);
    tfVEnhancedLogWrite(module, level, msgPtr, argp);
    va_end(argp);
}

static void tfVEnhancedLogWrite(tt32Bit module, 
                                tt8Bit level, 
                                ttConstCharPtr msgPtr, 
                                va_list argp)
{
    tt32Bit     length;
    int         errorCode;
    ttCharPtr   tempCharPtr;

#ifdef TM_ERROR_CHECKING
    if (
#if (TM_LOG_MODULE_BASE != 0)
        (module >= TM_LOG_MODULE_BASE)
        && 
#endif /* #if (TM_LOG_MODULE_BASE != 0) */
        (module <= TM_LOG_MODULE_MAX)
        && (level <= TM_LOG_LEVEL_MAX)
        && (msgPtr != (ttConstCharPtr)0))
#endif /* TM_ERROR_CHECKING */
    {
/* Check to see if we can log this message, if so obtain the logging lock */
        errorCode = tfLogLevelCheckOut(module, level);
        if (errorCode == TM_ENOERROR)
        {
            /* We can log this message now */
            length = tm_vsprintf(
                tm_context(tvLogLevelStoragePtr)->llsLogLevelMessage[module],
                msgPtr,
                argp);
/* If verbose logging is not enabled, trim the verbose output from the string */
            if ((tm_context(tvLogLevelStoragePtr)->
                 llsLogVerbosityArray[module]) == TM_LOG_CONCISE)
            {
                tempCharPtr = tm_strstr(tm_context(tvLogLevelStoragePtr)->
                                        llsLogLevelMessage[module], 
                                        TM_LOG_DELIMITER);
                if(tempCharPtr)
                {
                    tempCharPtr[0] = '\0';
                }
            }
            tm_log_level_write_func(module, 
                                    level, 
                                    tm_context(tvLogLevelStoragePtr)->
                                    llsLogLevelMessage[module], 
                                    length);
/* Release the logging lock */
            tfLogLevelCheckIn(module);
        }
    }
#ifdef TM_ERROR_CHECKING
    else
    {
        tfKernelWarning("tfVEnhancedLogWrite", "Invalid argument");
    }
#endif /* TM_ERROR_CHECKING */
}
static int tfLogLevelCheckOut(tt32Bit module, tt8Bit level)
{
    int errorCode;
    errorCode = TM_EPERM;
    if (tm_context(tvLogLevelStoragePtr)->llsLogLevelArray[module] >= level)
    {
#ifdef TM_LOCK_NEEDED
        tm_call_lock_wait(&(tm_context(tvLogLevelStoragePtr)->
                                            llsLogModuleLock[module]));
#endif /* TM_LOCK_NEEDED */
        errorCode = TM_ENOERROR;
    }
    return errorCode;
}

static void tfLogLevelCheckIn(tt32Bit module)
{
#ifdef TM_LOCK_NEEDED
    tm_call_unlock(&(tm_context(tvLogLevelStoragePtr)->
                                            llsLogModuleLock[module]));
#else /* TM_LOCK_NEEDED */
    TM_UNREF_IN_ARG(module);
#endif /* TM_LOCK_NEEDED */
}

#ifndef tm_log_level_write
static int tfTreckLogWrite(tt32Bit      module, 
                           tt8Bit       level,
                           ttCharPtr    msgPtr, 
                           tt32Bit      length)
{

    TM_UNREF_IN_ARG(length);
#if !((defined(TM_KERNEL_WIN32_X86) || defined(TM_KERNEL_VISUAL_X86)) \
      && (!defined(TM_TRECK_DEMO)))
    TM_UNREF_IN_ARG(level);
#endif /* !TM_TRECK_DEMO &&(TM_KERNEL_WIN32_X86 | TM_KERNEL_VISUAL_X86) */

#if ((defined(TM_KERNEL_WIN32_X86) || defined(TM_KERNEL_VISUAL_X86)) \
      && (!defined(TM_TRECK_DEMO)))
    switch (level)
    {
        case TM_LOG_LEVEL_ERROR:
            level = TM_PRINT_TYPE_FAIL;
            break;
        case TM_LOG_LEVEL_WARNING:
            level = TM_PRINT_TYPE_WARN;
            break;
        case TM_LOG_LEVEL_NOTICE:
        case TM_LOG_LEVEL_INFO:
        case TM_LOG_LEVEL_DEBUG:
        default:
            level = TM_PRINT_TYPE_FILE;
            break;
    }
    (void)tfKernelPrint(level, "%s%s\n", tlTreckModulePrefix[module], msgPtr);
#else /* !TM_TRECK_DEMO &&(TM_KERNEL_WIN32_X86 | TM_KERNEL_VISUAL_X86) */
/* Log this message */
    tfLogMsg(tm_context(tvLogCtrlBlkPtr), 
                        tlTreckModulePrefix[module], 
                        msgPtr);
#endif /* !TM_TRECK_DEMO &&(TM_KERNEL_WIN32_X86 | TM_KERNEL_VISUAL_X86) */
    return TM_ENOERROR;
}
#endif /* tm_log_level_write */

void tfInitEnhancedLogging()
{
    unsigned int allocSize;

    allocSize = sizeof(ttLogLevelStorage);
    tm_context(tvLogLevelStoragePtr) =
                    (ttLogLevelStoragePtr)tm_kernel_malloc(allocSize);
    if (tm_context(tvLogLevelStoragePtr) == (ttLogLevelStoragePtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelError("tfInitEnhancedLogging",
                "FATAL: Unable to malloc enhanced logging structure");
#endif
        tm_thread_stop;
    }
    tm_bzero((ttVoidPtr)tm_context(tvLogLevelStoragePtr), allocSize);
}

#else /* TM_USE_LOGGING_LEVELS */
/* To allow link for builds when TM_USE_LOGGING_LEVELS is not defined */
int tv4LogLevelDummy = 0;
#endif /* TM_USE_LOGGING_LEVELS */

/***************** End Of File *****************/
