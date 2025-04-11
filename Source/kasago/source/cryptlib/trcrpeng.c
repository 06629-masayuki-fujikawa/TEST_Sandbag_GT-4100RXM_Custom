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
 * Description: Crypto Engine Management
 *
 * Filename: trcryeng.c
 * Author: Jin Zhang
 * Date Created: 1/9/2003
 * $Source: source/cryptlib/trcrpeng.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2011/12/18 21:35:25JST $
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

#ifdef TM_USE_PKI
#if defined(TM_PUBKEY_USE_DSA) || defined (TM_PUBKEY_USE_RSA)
#include <tropnssl.h>
#endif /* defined(TM_PUBKEY_USE_DSA) || defined (TM_PUBKEY_USE_RSA) */
#include <trpki.h>
#endif /* TM_USE_PKI */

#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#include <trssl.h>
#endif /* TM_USE_SSL_CLIENT) || TM_USE_SSL_SERVER */

static const char  TM_CONST_QLF tlEncAlgDes[] =        "DES";
static const char  TM_CONST_QLF tlEncAlgIdea[] =       "IDEA";
static const char  TM_CONST_QLF tlEncAlgBlowfish[] =   "BLOWFISH";
static const char  TM_CONST_QLF tlEncAlgRC5R16B64[] =  "RC5-R16-B64";
static const char  TM_CONST_QLF tlEncAlg3Des[] =       "3DES";
static const char  TM_CONST_QLF tlEncAlgCast[] =       "CAST";
static const char  TM_CONST_QLF tlEncAlgAes[] =        "AES";
static const char  TM_CONST_QLF tlEncAlgNull[] =       "NULL";
static const char  TM_CONST_QLF tlEncAlgTwoFish[] =    "TWOFISH";
static const char  TM_CONST_QLF tlEncAlgNoSupport[] =  "NOT SUPPORTED";

static const char  TM_CONST_QLF tlHashAlgMd5[] =       "MD5";
static const char  TM_CONST_QLF tlHashAlgSha1[] =      "SHA1";
static const char  TM_CONST_QLF tlHashAlgTiger[] =     "TIGER";
static const char  TM_CONST_QLF tlHashAlgSha256[] =    "SHA256";
static const char  TM_CONST_QLF tlHashAlgSha384[] =    "SHA384";
static const char  TM_CONST_QLF tlHashAlgSha512[] =    "SHA512";
static const char  TM_CONST_QLF tlHashAlgRIPEMD[] =    "RIPEMD";
static const char  TM_CONST_QLF tlHashAlgNoSupport[] = "NOT SUPPORTED";

static ttCryptoEnginePtr tfCryptoEngineLookup(unsigned int engineId);
TM_NEARCALL static int tfCryptoDefaultGetRandomWord(ttUser32Bit   TM_FAR * dataPtr,
                                        ttUser16Bit            wordSize);
static int tfCryptoEngineRegisterLocked(
                           unsigned int                 engineId,
                           ttUserVoidPtr                initParamPtr,
                           ttCryptoEnginePtrPtr         newEnginePtrPtr,
                           ttCryptoEngineInitFuncPtr    engineInitFuncPtr,
                           ttCryptoGetRandomWordFuncPtr randomFuncPtr,
                           ttCryptoSessionOpenFuncPtr   sessionOpenFuncPtr,
                           ttCryptoSessionFuncPtr       sessionProcessFuncPtr,
                           ttCryptoSessionFuncPtr       sessionCloseFuncPtr);

#ifdef TM_USE_SW_CRYPTOENGINE
static int tfCryptoSoftwareHash(ttCryptoRequestHashPtr);
static int tfCryptoSoftwareMacHash(ttCryptoRequestPtr);

#ifdef TM_PUBKEY_USE_DIFFIEHELLMAN
static int tfCryptoSoftwareDhrequest(tt8Bit, ttCryptoRequestDhrequestPtr);
static int tfCryptoDhConstructPrimeVector(int                    dhgroup,
                                       ttCharVector TM_FAR * primeVectorPtr);
#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN */

#ifdef TM_USE_PKI
#ifdef TM_PUBKEY_USE_RSA
static int tfCryptoSoftwareRsarequest(tt8Bit,ttCryptoRequestRsarequestPtr);
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_PUBKEY_USE_DSA
static int tfCryptoSoftwareDsarequest(tt8Bit,ttCryptoRequestDsarequestPtr);
#endif /* TM_PUBKEY_USE_DSA */

#endif /* TM_USE_PKI */
#endif /* TM_USE_SW_CRYPTOENGINE */

/*
 * tfCryptoEngineLookup() Function Description
 * Called by tfCryptoEngineRegister or tfCryptoEngineAddAlgorithm
 * to find the crypto engine pointer given the engine id.
 *
 * Called with the Crypto Engine lock on.
 *
 * Parameter         description
 * engineId          Crypto Engine number, currently, we support 
 *                   TM_CRYPTO_ENGINE_SOFTWARE  and  TM_CRYPTO_ENGINE_HIFN7951
 *
 * Return value:     NULL pointer or pointer to the crypto engine
 */

static ttCryptoEnginePtr tfCryptoEngineLookup(unsigned int engineId)
{
    ttCryptoEngineListPtr    celPtr;
    ttCryptoEnginePtr        cenginePtr;
    
    cenginePtr = (ttCryptoEnginePtr)0;
    
    celPtr = ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))
        ->clgCryptoEngineHeadPtr;
    while(celPtr)
    {
        if(celPtr->celCryptoEnginePtr->ceEngineId == engineId)
        {
            cenginePtr = celPtr->celCryptoEnginePtr;
            break;
        }
        celPtr = celPtr->celNextPtr;
    }
    if(!celPtr)
    {
        cenginePtr = (ttCryptoEnginePtr)0;
    }
    return cenginePtr;
}

static int tfCryptoDefaultGetRandomWord(ttUser32Bit   TM_FAR * dataPtr,
                                        ttUser16Bit            wordSize)
{
    tfGetRandomBytes((tt8BitPtr)dataPtr, (int)(wordSize * sizeof(tt32Bit)));
    return TM_ENOERROR;
}


/*
 * tfCryptoEngineRegister() Function Description
 * Lock the crypto engine
 * Call common function tfCryptoEngineRegisterLocked()
 * unlock the crypto engine
 */

int tfCryptoEngineRegister(unsigned int                 engineId,
                           ttUserVoidPtr                initParamPtr,
                           ttCryptoEnginePtrPtr         newEnginePtrPtr,
                           ttCryptoEngineInitFuncPtr    engineInitFuncPtr,
                           ttCryptoGetRandomWordFuncPtr randomFuncPtr,
                           ttCryptoSessionOpenFuncPtr   sessionOpenFuncPtr,
                           ttCryptoSessionFuncPtr       sessionProcessFuncPtr,
                           ttCryptoSessionFuncPtr       sessionCloseFuncPtr)
{
    int errorCode;

    tm_call_lock_wait(&(tm_context(tvCryptoLockEntry)));
    errorCode = tfCryptoEngineRegisterLocked(engineId,
                                  initParamPtr,
                                  newEnginePtrPtr,
                                  engineInitFuncPtr,
                                  randomFuncPtr,
                                  sessionOpenFuncPtr,
                                  sessionProcessFuncPtr,
                                  sessionCloseFuncPtr);
    tm_call_unlock(&(tm_context(tvCryptoLockEntry)));
    return errorCode;
}

/*
 * tfCryptoEngineRegisterLocked() Function Description
 * Called by user space to register a crypto engine, or internally to register
 * the software crypto engine. User doesn't need to
 * call this function to register software crypto engine, unless user want
 * to replace Treck's software implementation. A crypto engine is able to
 * process multiple sessions, for each session, we define the following three
 * function pointers: openFuncPtr, processFuncPtr, closeFuncPtr. Session 
   example,
 * all packets to be processed using the same SA should use the same session.
 * When an SA is deleted, the session should be closed.
 *
 * Parameter         description
 * engineId          Crypto Engine number, currently, we support 
 *                   TM_CRYPTO_ENGINE_SOFTWARE  and  TM_CRYPTO_ENGINE_HIFN7951
 * initParamPtr      The initialization parameter
 * newEnginePtr      returns with pointer which points to the new engine
 * initFuncPtr       Crypto Engine initialization function
 * randomFuncPtr     If the hardware offers random number generation functionality,
 *                   input the function pointer. If it is NULL, we will use the
 *                   software random generator.
 * openFuncPtr       Crypto engine session open function pointer, which takes a
                     parameter of type ttSadbPtr, and constructs the session.
 * processFuncPtr    Crypto engine session process function pointer, which takes
                     a parameter of type ttCryptoRequestPtr
 * closeFuncPtr      Crypto engine session close function pointer, which takes
                     a parameter of type session. (for Hifn7951, it is 
                     (Hifn79xx_PktEngSessContext*)
 * needLock          need to lock crypto engine structure.

 *
 * Return value:     TM_ENOERROR, otherwise failed
 */
static int tfCryptoEngineRegisterLocked(
                           unsigned int                 engineId,
                           ttUserVoidPtr                initParamPtr,
                           ttCryptoEnginePtrPtr         newEnginePtrPtr,
                           ttCryptoEngineInitFuncPtr    engineInitFuncPtr,
                           ttCryptoGetRandomWordFuncPtr randomFuncPtr,
                           ttCryptoSessionOpenFuncPtr   sessionOpenFuncPtr,
                           ttCryptoSessionFuncPtr       sessionProcessFuncPtr,
                           ttCryptoSessionFuncPtr       sessionCloseFuncPtr)
{
    ttCryptoEngineListPtr    newListPtr;
    ttCryptoEnginePtr        newEnginePtr;
    ttCryptoEnginePtr        findEnginePtr;
    int                      errorCode;


    errorCode = TM_ENOERROR;
    newEnginePtr = (ttCryptoEnginePtr)0;
    if(!tm_context(tvCryptoLibPtr))
    {
        errorCode = TM_EINVAL;
        goto REG_RETURN;
    }

/* firstly check if this engineId exists or not */

    if(engineId > TM_CRYPTO_ENGINE_MAX)
    {
        errorCode = -1;
        goto REG_RETURN;
    }

    findEnginePtr = tfCryptoEngineLookup( engineId );
    if( findEnginePtr != (ttCryptoEnginePtr)0 )
    {
/* this crypto engine has been registered before */
        errorCode = -1;
        goto REG_RETURN;
    }

    {
        newEnginePtr = tm_get_raw_buffer(sizeof(ttCryptoEngine));
        if(newEnginePtr == (ttCryptoEnginePtr)0)
        {
            errorCode = TM_ENOBUFS;
        }
        else
        {
            newEnginePtr->ceEngineId = engineId;
            newEnginePtr->ceSupportFlags = (tt32Bit)1;
            newEnginePtr->ceInitFuncPtr = engineInitFuncPtr;
            if(randomFuncPtr)
            {
                newEnginePtr->ceRandomWordFuncPtr = randomFuncPtr;
            }
            else
            {
                newEnginePtr->ceRandomWordFuncPtr = 
                             tfCryptoDefaultGetRandomWord;
            }
            newEnginePtr->ceSessionOpenFuncPtr = sessionOpenFuncPtr;
            newEnginePtr->ceSessionProcessFuncPtr = sessionProcessFuncPtr;
            newEnginePtr->ceSessionCloseFuncPtr = sessionCloseFuncPtr;


            newListPtr = tm_get_raw_buffer(sizeof(ttCryptoEngineList));
            if(newListPtr == (ttCryptoEngineListPtr)0)
            {
                errorCode = TM_ENOBUFS;
                tm_free_raw_buffer((ttRawBufferPtr)newEnginePtr);
                newEnginePtr = (ttCryptoEnginePtr)0;
            }
            else
            {
/* Initialize the engine */
                if(engineInitFuncPtr)
                {
                    errorCode = engineInitFuncPtr(initParamPtr);
                }

                if(errorCode == TM_ENOERROR)
                {
/* add the new crypto engine to the head of crypto engine list */
                    newListPtr->celCryptoEnginePtr = newEnginePtr;
                    newListPtr->celNextPtr = ((ttCryptoLibGlobalPtr)
                              tm_context(tvCryptoLibPtr))
                              ->clgCryptoEngineHeadPtr;
                    ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))
                              ->clgCryptoEngineHeadPtr = newListPtr;
                }
                else
                {
                    tm_free_raw_buffer((ttRawBufferPtr)newEnginePtr);
                    tm_free_raw_buffer((ttRawBufferPtr)newListPtr);
                    newEnginePtr = (ttCryptoEnginePtr)0;
                }
            }
        }
    }

REG_RETURN:
    *newEnginePtrPtr = newEnginePtr;
    return errorCode;
}



/*
 * tfCryptoEngineDeRegister() Function Description
 * Called to de-register the crypto engine
 *
 * Parameter         description
 * engineId          Crypto Engine id used to find the previously registered 
 *                   crypto engine.
 *                   TM_CRYPTO_ENGINE_SOFTWARE  and  TM_CRYPTO_ENGINE_HIFN7951
 *
 * Return value:     TM_ENOERROR, otherwise failed to find
 */
int tfCryptoEngineDeRegister(unsigned int               engineId)
{
/*
 * TBD: To do this safely we need to add an owner count incremented for every
 * tfCryptoEngineGet(), and need to add a tfCryptoEngineUnGet() etc..
 */
    ttCryptoEngineListPtr    celPtr;
    ttCryptoEngineListPtr    prevPtr;
    int                      errorCode;

    errorCode = TM_ENOERROR;
    prevPtr = (ttCryptoEngineListPtr)0;
    tm_call_lock_wait(&(tm_context(tvCryptoLockEntry)));
    if(!tm_context(tvCryptoLibPtr))
    {
        errorCode = TM_EINVAL;
        goto DEREG_RETURN;
    }

    celPtr = ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))
             ->clgCryptoEngineHeadPtr;

    while(celPtr)
    {
        if(celPtr->celCryptoEnginePtr->ceEngineId == engineId)
        {
            break;
        }
        prevPtr = celPtr;
        celPtr = celPtr->celNextPtr;
    }

    if(celPtr)
    {
        if(!prevPtr)
        {
/* it is the head */
            ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))
             ->clgCryptoEngineHeadPtr = celPtr->celNextPtr;
        }
        else
        {
            prevPtr->celNextPtr = celPtr->celNextPtr;
        }
/* now we are safe to free them */
        tm_free_raw_buffer((ttRawBufferPtr)celPtr->celCryptoEnginePtr);
        tm_free_raw_buffer((ttRawBufferPtr)celPtr);

    }
    else
    {
/* we didn't find anything */
        errorCode = TM_ENOENT;
    }
DEREG_RETURN:
    tm_call_unlock(&(tm_context(tvCryptoLockEntry)));
    return errorCode;
}


/*
 * tfCryptoEngineAddAlgorithm() Function Description
 * Add supported algorithm to the crypto engine..
 *
 * Parameter         description
 * engineId          Crypto Engine number, currently, we support 
 *                   TM_CRYPTO_ENGINE_SOFTWARE  and  TM_CRYPTO_ENGINE_HIFN7951
 * algorithmId       Algorithm ID information, for example, SADB_AALG_MD5HMAC,
 *                   SADB_AALG_SHA1HMAC... SADB_EALG_3DESCBC...
 *
 * Return value:     TM_ENOERROR, otherwise failed
 */


int tfCryptoEngineAddAlgorithm(ttCryptoEnginePtr      cenginePtr,
                               unsigned int           algorithmId)
{
    int                 errorCode;

    errorCode = TM_ENOERROR;

    if(algorithmId > SADB_ALGORITHM_MAX 
               || cenginePtr == (ttCryptoEnginePtr)0)
    {
        errorCode = -1;
        goto ADDALG_RETURN;
    }

    cenginePtr->ceSupportFlags |= (TM_UL(1) << algorithmId);


ADDALG_RETURN:
    return errorCode;
}

/*DESCRIPTION: This function initialize the crypto library. It is called
 * when tfUseIpsec, or tfUsePki is called. If software engine is defined,
 * this call will register software engine and add all necessory algorithms
 */
int tfCryptoEngineInit(void)
{
    int               errorCode;
#ifdef TM_USE_SW_CRYPTOENGINE
    ttCryptoEnginePtr softEnginePtr;
#endif /* TM_USE_SW_CRYPTOENGINE */
#ifdef TM_USE_MCF5235_CRYPTOENGINE
    ttCryptoEnginePtr mcf5235EnginePtr;
#endif /* TM_USE_MCF5235_CRYPTOENGINE */

    errorCode = TM_ENOERROR;

    tm_call_lock_wait(&(tm_context(tvCryptoLockEntry)));
    if(!tm_context(tvCryptoLibPtr))
    {
        tm_context(tvCryptoLibPtr) = tm_get_raw_buffer(
                sizeof(ttCryptoLibGlobal));
        if(!tm_context(tvCryptoLibPtr))
        {
            errorCode = TM_ENOBUFS;
            goto CEINIT_RETURN;
        }
        tm_bzero(tm_context(tvCryptoLibPtr), sizeof(ttCryptoLibGlobal));

#ifdef TM_USE_SW_CRYPTOENGINE
        softEnginePtr = tfCryptoEngineLookup( TM_CRYPTO_ENGINE_SOFTWARE );
        if( softEnginePtr == (ttCryptoEnginePtr)0 )
        {
            errorCode = tfCryptoEngineRegisterLocked(
                                       TM_CRYPTO_ENGINE_SOFTWARE,
                                       (ttUserVoidPtr)0,
                                       &softEnginePtr,
                                       (ttCryptoEngineInitFuncPtr)0,
                                       (ttCryptoGetRandomWordFuncPtr)0,
                                       (ttCryptoSessionOpenFuncPtr)0,
                                       tfCryptoSoftwareEngineSessionProcess,
                                       (ttCryptoSessionFuncPtr)0);
            if(errorCode == TM_ENOERROR && softEnginePtr)
            {
/* add the following supported algorithms, you could just use public API
 * tfCryptoEngineAddAlgorithm one algorithm by one algoirthm, for example,
 * tfCryptoEngineAddAlgorithm(softEnginePtr, SADB_AALG_MD5HMAC );
 */
#ifdef TM_USE_MD2
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_AALG_MD2HMAC);
#endif /* TM_USE_MD2 */
#ifdef TM_USE_MD5
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_AALG_MD5HMAC);
#endif /* TM_USE_MD5 */
#ifdef TM_USE_SHA1
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_AALG_SHA1HMAC);
#endif /* TM_USE_SHA1 */
#ifdef TM_USE_SHA256
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_AALG_SHA256HMAC);
#endif /* TM_USE_SHA256 */
#ifdef TM_USE_SHA384
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_AALG_SHA384HMAC);
#endif /* TM_USE_SHA384 */
#ifdef TM_USE_SHA512
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_AALG_SHA512HMAC);
#endif /* TM_USE_SHA512 */
#ifdef TM_USE_RIPEMD
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_AALG_RIPEMDHMAC);
#endif /* TM_USE_RIPEMD */
#ifdef TM_USE_RC2 
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_RC2CBC);
#endif /* TM_USE_DES */
/* DES and 3DES uses the same module */
#ifdef TM_USE_DES 
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_DESIV64CBC);
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_DESCBC);
#endif /* TM_USE_DES */
#ifdef TM_USE_3DES
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_3DESCBC);
#endif /* TM_USE_3DES */

#ifdef TM_USE_BLOWFISH
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_BLOWFISHCBC);
#endif /* TM_USE_BLOWFISH */

#ifdef TM_USE_CAST128
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_CAST128CBC);
#endif /* TM_USE_CAST128 */

#ifdef TM_USE_AES
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_AESCBC);
#endif /* TM_USE_AES */

#ifdef TM_USE_ARCFOUR
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_ARCFOUR);
#endif /* TM_USE_ARCFOUR */

#ifdef TM_USE_TWOFISH
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_TWOFISHCBC);
#endif /* TM_USE_TWOFISH */

#ifdef TM_PUBKEY_USE_DIFFIEHELLMAN /* we support group 1, 2, 5, and 14 */
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_PUBKEY_DIFFIEHELLMAN1);
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_PUBKEY_DIFFIEHELLMAN2);
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_PUBKEY_DIFFIEHELLMAN5);
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_PUBKEY_DIFFIEHELLMAN14);
#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN */

#ifdef TM_PUBKEY_USE_DSA
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_PUBKEY_DSA);
#endif /* TM_PUBKEY_USE_DSA */

#ifdef TM_PUBKEY_USE_RSA
                softEnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_PUBKEY_RSA);
#endif /* TM_PUBKEY_USE_RSA */
            }
        }
#endif /*  TM_USE_SW_CRYPTOENGINE */

#ifdef TM_USE_MCF5235_CRYPTOENGINE
        mcf5235EnginePtr = tfCryptoEngineLookup( TM_CRYPTO_ENGINE_MCF5235 );
        if( mcf5235EnginePtr == (ttCryptoEnginePtr)0 )
        {
            errorCode = tfCryptoEngineRegisterLocked(
                                               TM_CRYPTO_ENGINE_MCF5235,
                                               (ttUserVoidPtr)0,
                                               &mcf5235EnginePtr,
                                               tfMcf5235EngineInit,
                                               tfMcf5235GetRandomWord,
                                               (ttCryptoSessionOpenFuncPtr)0,
                                               tfMcf5235SessionProcess,
                                               (ttCryptoSessionFuncPtr)0);
            if(errorCode == TM_ENOERROR && mcf5235EnginePtr)
            {
/* add the following supported algorithms, you could just use public API
 * tfCryptoEngineAddAlgorithm one algorithm by one algoirthm, for example,
 * tfCryptoEngineAddAlgorithm(softEnginePtr, SADB_AALG_MD5HMAC );
 */
#ifdef TM_USE_MD5
                mcf5235EnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_AALG_MD5HMAC);
#endif /* TM_USE_MD5 */
#ifdef TM_USE_SHA1
                mcf5235EnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_AALG_SHA1HMAC);
#endif /* TM_USE_SHA1 */
/* DES and 3DES uses the same module */
#ifdef TM_USE_DES 
                mcf5235EnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_DESCBC);
#endif /* TM_USE_DES */
#ifdef TM_USE_3DES
                mcf5235EnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_3DESCBC);
#endif /* TM_USE_3DES */

#ifdef TM_USE_AES
                mcf5235EnginePtr->ceSupportFlags |=
                    (TM_UL(1) << SADB_EALG_AESCBC);
#endif /* TM_USE_AES */
            }
        }
#endif /* TM_USE_MCF5235_CRYPTOENGINE */
    }

CEINIT_RETURN:
    ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))->cryptoOwnerCount++;
    tm_call_unlock(&(tm_context(tvCryptoLockEntry)));
    return errorCode;
}


void tfCryptoEngineUninitialize(void)
{
/*
 * TBD: To do this safely we need to add an owner count incremented for every
 * tfCryptoEngineGet(), and need to add a tfCryptoEngineUnGet() etc..
 */
    ttCryptoEngineListPtr     celPtr;
    ttCryptoEngineListPtr     prevCelPtr;

    tm_call_lock_wait(&(tm_context(tvCryptoLockEntry)));
    if(tm_context(tvCryptoLibPtr)!= (ttVoidPtr)0)
    {
        if ( ((ttCryptoLibGlobalPtr)
                        tm_context(tvCryptoLibPtr))->cryptoOwnerCount != 0)
        {
            ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))->
                                                            cryptoOwnerCount--;
        }

        if ( ((ttCryptoLibGlobalPtr)
                        tm_context(tvCryptoLibPtr))->cryptoOwnerCount == 0)
        {
            celPtr = ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))
                                  ->clgCryptoEngineHeadPtr;
            while(celPtr)
            {
                prevCelPtr = celPtr;
                celPtr = celPtr->celNextPtr;
                tm_free_raw_buffer(
                            (ttRawBufferPtr)prevCelPtr->celCryptoEnginePtr);
                tm_free_raw_buffer((ttRawBufferPtr)prevCelPtr);
            }
            tm_free_raw_buffer(tm_context(tvCryptoLibPtr));
            tm_context(tvCryptoLibPtr) = 0;
        }
    }
    tm_call_unlock(&(tm_context(tvCryptoLockEntry)));
    return;
}

ttCryptoEnginePtr tfCryptoEngineGetFirst (void)
{
    ttCryptoEngineListPtr    celPtr;
    ttCryptoEnginePtr        cePtr;

    cePtr = (ttCryptoEnginePtr)0;
    tm_call_lock_wait(&(tm_context(tvCryptoLockEntry)));
    if (tm_context(tvCryptoLibPtr) != (ttVoidPtr)0)
    {
        celPtr = ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))
                                  ->clgCryptoEngineHeadPtr;
        if (celPtr != (ttCryptoEngineListPtr)0)
        {
            cePtr = celPtr->celCryptoEnginePtr;
        }
    }
    tm_call_unlock(&(tm_context(tvCryptoLockEntry)));
    return cePtr;
}

/* find a crypto engine which supports the given algorithm*/
ttCryptoEnginePtr tfCryptoEngineGet (int       algorithm)
{
    return tfCryptoEngineGet2(algorithm, -1);
}

/* find a crypto engine which supports the given 2 algorithms */
ttCryptoEnginePtr tfCryptoEngineGet2 (int algorithm1, int algorithm2)
{
    ttCryptoEngineListPtr    celPtr;
    ttCryptoEnginePtr        cePtr;
    
    cePtr = (ttCryptoEnginePtr)0;
    tm_call_lock_wait(&(tm_context(tvCryptoLockEntry)));
    if (tm_context(tvCryptoLibPtr) != (ttVoidPtr)0)
    {
        celPtr = ((ttCryptoLibGlobalPtr)tm_context(tvCryptoLibPtr))
                                  ->clgCryptoEngineHeadPtr;
        while (celPtr)
        {
            cePtr = celPtr->celCryptoEnginePtr;
            if (cePtr->ceSupportFlags & (TM_UL(1) << algorithm1))
            {
                if (algorithm2 == -1)
                {
                    break;
                }
                else
                {
                    if (cePtr->ceSupportFlags & (TM_UL(1) << algorithm2))
                    {
                        break;
                    }
                }
            }
            celPtr = celPtr->celNextPtr;
        }
        if (celPtr == (ttCryptoEngineListPtr)0)
        {
            cePtr = (ttCryptoEnginePtr)0;
        }
    }
    tm_call_unlock(&(tm_context(tvCryptoLockEntry)));
    return cePtr;
}

#ifdef TM_USE_IPSEC
/*
 * Encrypt user data into a new buffer, rather than replacing the user data.
 *
 * Public API called by the user before IPSec encryption occurs. This
 * function check for the presence of user buffers in the outgoing packet. If
 * user buffers are present, it will allocate a new buffer to store the
 * encrypted data into. It will also set up the IPSec request structure to
 * let the encryption engine know about the new buffer. The user must call
 * tfCryptoEngineSwapUserBufsFinish after the encryption has been done to
 * complete this process.
 */
int tfCryptoEngineSwapUserBufsStart(ttCryptoRequestIpsecPtr reqPtr)
{
    ttPacketPtr             pktPtr;
    int                     retCode;
#ifdef TM_IPSEC_USE_64BIT_SEQ_NO
    ttPacketPtr             tempPacketPtr;
    tt8BitPtr               seqHiPtr;
    tt16Bit                 seqHiOffset;
#endif /* TM_IPSEC_USE_64BIT_SEQ_NO */

    retCode = TM_ENOERROR;
    pktPtr = (ttPacketPtr)reqPtr->criPacketPtr;

/* If we don't already have a new packet set up */
    if (reqPtr->criNewPacketPtr == (ttUserPacketPtr)0)
    {
/* Search for user data */
        while (pktPtr != (ttPacketPtr)0)
        {
            if (    (pktPtr->pktSharedDataPtr->dataFlags & TM_BUF_USER_DATA)
                 || (pktPtr->pktUserFlags & TM_PKTF_USER_DATA_SENT)
               )
            {
/* There could be extra space in the buffer for the trailing MAC if
 * authentication is being used. Calculate the length of the new buffer:
 * total length of the current packet minus the header size after encryption
 */
                pktPtr = tfGetSharedBuffer(
                    0,
                    ((ttPacketPtr)reqPtr->criPacketPtr)->pktChainDataLength -
                        reqPtr->criCryptHeadLength,
                    0);
                if (pktPtr == (ttPacketPtr)0)
                {
                    retCode = TM_ENOBUFS;
                    break; /* return */
                }
                pktPtr->pktLinkDataLength = 0;
                reqPtr->criNewPacketPtr = (ttUserPacketPtr)pktPtr;
/* Adjust the MAC pointer */
                reqPtr->criMacOutPtr =
                    pktPtr->pktLinkDataPtr + reqPtr->criCryptDataLength;
#ifdef TM_IPSEC_USE_64BIT_SEQ_NO
#ifdef TM_USE_IKEV2
                if (tm_sadb_is_esn_on(reqPtr->criSadbPtr))
#endif /* TM_USE_IKEV2 */
                {
                    seqHiOffset = reqPtr->criCryptDataLength +
                                                reqPtr->criCryptHeadLength;
                    retCode = tfGetPositionFromLink(
                                        (ttPacketPtr)reqPtr->criPacketPtr,
                                        seqHiOffset,
                                        &seqHiPtr,
                                        &tempPacketPtr,
                                        TM_IPSEC_OUTBOUND);
                    if (retCode != TM_ENOERROR) 
                    {
                        break;
                    }
                    tm_memcpy(reqPtr->criMacOutPtr, seqHiPtr, sizeof(tt32Bit));
                    reqPtr->criMacOutPtr += sizeof(tt32Bit);
                }
#endif /* TM_IPSEC_USE_64BIT_SEQ_NO */ 
                break; /* return */
            }
            pktPtr = pktPtr->pktLinkNextPtr;
        }
    }
    return retCode;
}

/*
 * Swap a buffer containing encrypted data into the data packet
 *
 * This function completes the process started by
 * tfCryptoEngineSwapUserBufsStart. It will check for the presence of a
 * buffer containing encrypted data. If this buffer is present, it will
 * insert the buffer into the proper part of the packet chain, adjust
 * buffer lengths as necessary, and free the plain-text user buffers.
 * This function must be called immediately after encryption occurs.
 *
 * Assumptions: This function assumes that the pktChainDataLength member of
 *   the packet pointer stored in reqPtr->criNewPacketPtr contains the
 *   correct length of the data in that packet. The encryption engine can
 *   modify pktLinkDataLength, so tfCryptoEngineSwapUserBufsStart sets
 *   pktChainDataLength to the appropriate value. We set pktLinkDataLength
 *   here based on pktChainDataLength.
 */
void tfCryptoEngineSwapUserBufsFinish(ttCryptoRequestIpsecPtr reqPtr)
{
    ttPacketPtr pktPtr;
    ttPacketPtr newPktPtr;
    ttPacketPtr oldPktPtr;
    tt8Bit      lockFlag;

/* If we put the data in a new buffer */
    if (reqPtr->criNewPacketPtr)
    {
        newPktPtr = (ttPacketPtr)reqPtr->criNewPacketPtr;
        newPktPtr->pktLinkDataLength = newPktPtr->pktChainDataLength;

/* Get the first packet in the chain */
        pktPtr = (ttPacketPtr)reqPtr->criPacketPtr;

/* Crypto engine assumes all headers are in the first link, so we can make
 * the same assumption */
        pktPtr->pktLinkDataLength = reqPtr->criCryptHeadLength;
/* Free everything after the headers */
        oldPktPtr = (ttPacketPtr)pktPtr->pktLinkNextPtr;
        if (oldPktPtr != (ttPacketPtr)0)
        {
#ifdef TM_LOCK_NEEDED
            if (    (   (oldPktPtr->pktSharedDataPtr->dataFlags)
                      & TM_BUF_SOCKETENTRYPTR)
                 && (    oldPktPtr->pktSharedDataPtr->dataSocketEntryPtr
                      != (ttSocketEntryPtr)0)
                 && (pktPtr->pktSendLockEntryPtr != (ttLockEntryPtr)0)
                 && (    pktPtr->pktSendLockEntryPtr
                      == &(oldPktPtr->pktSharedDataPtr->dataSocketEntryPtr
                                                        ->socLockEntry))
                 )
            {
                lockFlag = TM_SOCKET_LOCKED;
            }
            else
            {
                lockFlag = TM_SOCKET_UNLOCKED;
            }
#else /* !TM_LOCK_NEEDED */
            lockFlag = TM_SOCKET_UNLOCKED;
#endif /* !TM_LOCK_NEEDED */
            tfFreePacket(oldPktPtr, lockFlag);
        }
/* Insert the encrypted data into the chain */
        pktPtr->pktLinkNextPtr = newPktPtr;
        pktPtr->pktLinkExtraCount = 1;
    }
}

#endif /* TM_USE_IPSEC */

#if (defined(TM_IPSEC_DEBUG) || defined(TM_IKE_DEBUG) || defined(TM_SSL_DEBUG))

#define TM_CRYPTO_NUM_32BITS 6 /* display at most 6 32-bits per line */
#define TM_CRYPTO_SIZE_32BIT 4 /* size of a 32-bit */
#define TM_CRYPTO_LEN_OFFSET 4 /* room to store offset in print buffer */
void tfCryptoDebugPrintf(ttCharPtrPtr  printBufPtrPtr,
                         tt8BitPtr     dataPtr,
                         tt32Bit       dataLength,
                         ttIntPtr      offPtr)
{
    ttCharPtr printBufPtr;
    int       printOffset;
    int       highDataByte;
    int       dataByte;
    int       i;
    int       j;
    int       k;
    static const char  TM_CONST_QLF tlAsciiHex[16] = \
            {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

    if (printBufPtrPtr != (ttCharPtrPtr)0)
    {
        printBufPtr = *printBufPtrPtr;
    }
    else
    {
        printBufPtr = (ttCharPtr)0;
    }
    if (printBufPtr == (ttCharPtr)0)
    {
        printOffset = TM_CRYPTO_LEN_OFFSET;
/*
 * Enough room for
 *   TM_CRYPTO_LEN_OFFSET bytes at the beginning (to store offset)
 * + TM_CRYPTO_NUM_32BITS in hex with space in between * and line return
 * + final null
 */
        printBufPtr = tm_get_raw_buffer(
                     TM_CRYPTO_LEN_OFFSET 
                   + (TM_CRYPTO_NUM_32BITS * ((TM_CRYPTO_SIZE_32BIT * 2) + 1))
                   + 1 );
    }
    else
    {
        printOffset = (int)(*((tt32BitPtr)(ttVoidPtr)printBufPtr));
    }
    if (printBufPtr != (ttCharPtr)0)
    {
        j = 0;
        if (offPtr != (ttIntPtr)0)
        {
            k = *offPtr;
        }
        else
        {
            k = 0;
        }
        for (i = 0; i < (int)dataLength; i++)
        {
            dataByte = ((int)(dataPtr[i])) & 0xFF;
            highDataByte = dataByte / 16; /* hex base */
            dataByte = dataByte & 0xF;
            printBufPtr[printOffset++] = tlAsciiHex[highDataByte]; 
            printBufPtr[printOffset++] = tlAsciiHex[dataByte]; /* low Byte */
            j++;
            k++;
            if (j >= TM_CRYPTO_SIZE_32BIT)
            {
                j = 0;
            }
            if (k >= (TM_CRYPTO_NUM_32BITS * TM_CRYPTO_SIZE_32BIT))
            {
                k = 0;
            }
            if (k == 0)
            {
/* Flush + allocate */
                *((tt32BitPtr)(ttVoidPtr)printBufPtr) = (tt32Bit)printOffset;
                tfCryptoDebugFlush(&printBufPtr);
                if (i == ((int)dataLength - 1))
                {
/* done with user data */
                    break;
                }
/*
 * new allocation 
 * Enough room for
 *   TM_CRYPTO_LEN_OFFSET bytes at the beginning (to store offset)
 * + TM_CRYPTO_NUM_32BITS in hex with space in between * and line return
 * + final null
 */
                printBufPtr = tm_get_raw_buffer(
                     TM_CRYPTO_LEN_OFFSET 
                   + (TM_CRYPTO_NUM_32BITS * ((TM_CRYPTO_SIZE_32BIT * 2) + 1))
                   + 1 );
                if (printBufPtr == (ttCharPtr)0)
                {
                    break;
                }
                printOffset = 4;
            }
            else if (j == 0)
            {
                printBufPtr[printOffset++] = ' ';
            }
        }
        if (printBufPtr != (ttCharPtr)0)
        {
/* Keep print offset into buffer */
            *((tt32BitPtr)(ttVoidPtr)printBufPtr) = (tt32Bit)printOffset;
        }
        if ( (offPtr != (ttIntPtr)0) && (printBufPtrPtr != (ttCharPtrPtr)0) )
        {
            *offPtr = k;
        }
        else
        {
/*
 * Flush.
 * Done with that data, or user is not saving the print buffer.
 * No choice but to flush it. printBufPtr is null upon return. 
 */ 
            tfCryptoDebugFlush(&printBufPtr);
            if (offPtr != (ttIntPtr)0)
/* Reset */
            {
                *offPtr = 0;
            }
        }
        if (printBufPtrPtr != (ttCharPtrPtr)0)
        {
            *printBufPtrPtr = printBufPtr;
        }
    }
    return;
}

void tfCryptoDebugFlush(ttCharPtrPtr printBufPtrPtr)
{
    ttCharPtr printBufPtr;
    int       printOffset;

    if (printBufPtrPtr != (ttCharPtrPtr)0)
    {
        printBufPtr = *printBufPtrPtr;
        if (printBufPtr != (ttCharPtr)0)
        {
            printOffset = (int)(*((tt32BitPtr)(ttVoidPtr)printBufPtr));
            if (printOffset > TM_CRYPTO_LEN_OFFSET)
            {
                if (printBufPtr[printOffset - 1] == ' ')
/* Replace space separator with '\n' */
                {
                    printOffset--;
                }
                printBufPtr[printOffset++] = '\n';
                printBufPtr[printOffset++] = '\0';
/* Flush */
                tm_debug_log0(&printBufPtr[4]);
            }
            tm_free_raw_buffer(printBufPtr);
            *printBufPtrPtr = (ttCharPtr)0;
        }
    }
    return;
}

#endif /*TM_IPSEC_DEBUG || TM_IKE_DEBUG || TM_SSL_DEBUG */


#ifdef TM_USE_SW_CRYPTOENGINE
int tfCryptoSoftwareEngineSessionProcess(ttUserVoidPtr cryptoRequestPtr)
{
    ttCryptoRequestPtr crqPtr;
    int                errorCode;


    errorCode = TM_ENOERROR;
    crqPtr = (ttCryptoRequestPtr)cryptoRequestPtr;

    switch(crqPtr->crType)
    {
    case TM_CEREQUEST_CRYPTO_HASH:
        errorCode = tfCryptoSoftwareHash(
                           crqPtr->crParamUnion.crHashParamPtr);
        break;
    case TM_CEREQUEST_CRYPTO_HMACHASH:
    case TM_CEREQUEST_CRYPTO_CMACHASH:
        errorCode = tfCryptoSoftwareMacHash(crqPtr);
        break;
#ifdef TM_USE_IPSEC
    case TM_CEREQUEST_CRYPTO_IPSEC:
        errorCode = tfCryptoSoftwareIpsec(crqPtr);
        break;
#ifdef TM_USE_IKEV2
    case TM_CEREQUEST_CRYPTO_IKEV2: 
        errorCode = tfCryptoSoftwareIkev2(crqPtr);
        break;
#endif /* TM_USE_IKEV2 */
#ifdef TM_USE_IKEV1
    case TM_CEREQUEST_CRYPTO_IKE: 
        errorCode = tfCryptoSoftwareIke(crqPtr);
        break;
#endif /* TM_USE_IKEV1 */
#endif /* TM_USE_IPSEC */
#ifdef TM_PUBKEY_USE_DIFFIEHELLMAN
    case TM_CEREQUEST_PUBKEY_DHKEYPAIR: 
    case TM_CEREQUEST_PUBKEY_DHSHAREDKEY:
        errorCode = tfCryptoSoftwareDhrequest(
                            crqPtr->crType,
                            crqPtr->crParamUnion.crDhgroupParamPtr);
        break;
#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN */
#ifdef TM_USE_PKI
#ifdef TM_PUBKEY_USE_RSA
    case TM_CEREQUEST_PUBKEY_RSASIGN: 
    case TM_CEREQUEST_PUBKEY_RSAVERIFY:
    case TM_CEREQUEST_PUBKEY_RSAENCRYPT:
    case TM_CEREQUEST_PUBKEY_RSADECRYPT:
        errorCode = tfCryptoSoftwareRsarequest(
                            crqPtr->crType,
                            crqPtr->crParamUnion.crRsaParamPtr);
        break;
#endif /* TM_PUBKEY_USE_RSA */
#ifdef TM_PUBKEY_USE_DSA
    case TM_CEREQUEST_PUBKEY_DSASIGN: 
    case TM_CEREQUEST_PUBKEY_DSAVERIFY:
        errorCode = tfCryptoSoftwareDsarequest(
                            crqPtr->crType,
                            crqPtr->crParamUnion.crDsaParamPtr);
        break;
#endif /* TM_PUBKEY_USE_DSA */
#endif /* TM_USE_PKI */
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
    case TM_CEREQUEST_CRYPTO_SSL:
        errorCode = tfSslSoftwareRequestProcess(crqPtr);
        break;
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */


    default:
        errorCode = -1;
        break;
    }

    crqPtr->crResult = (tt16Bit)errorCode;
    return errorCode;
}

static int tfCryptoSoftwareHash(ttCryptoRequestHashPtr crhPtr)
{
    int                   errorCode;
    ttAhAlgorithmPtr      hashAlgPtr;
    ttCryptoChainDataPtr  ccdPtr;
    ttVoidPtr             contextPtr;

    errorCode = TM_ENOERROR;

    if(!crhPtr || !crhPtr->crhOutPtr)
    {
        errorCode = -1;
        goto HASH_RETURN;
    }

    hashAlgPtr =  crhPtr->crhAlgPtr;
    if(!hashAlgPtr)
    {
        hashAlgPtr = tfAhAlgorithmLookup(crhPtr->crhHashAlg);
        if(!hashAlgPtr)
        {
            errorCode = -1;
            goto HASH_RETURN;
        }
    }

    contextPtr = tm_get_raw_buffer(
                            (ttPktLen)hashAlgPtr->aalgContextLenFuncPtr());
    if(!contextPtr)
    {
        errorCode = TM_ENOBUFS;
        goto HASH_RETURN;
    }

    hashAlgPtr ->aalgInitFuncPtr(contextPtr);
    ccdPtr = crhPtr->crhInDataPtr;
    while(ccdPtr)
    {
        hashAlgPtr ->aalgUpdateFuncPtr(contextPtr, 
                                       ccdPtr->ccdDataPtr, 
                                       ccdPtr->ccdDataLen, 
                                       0);
        ccdPtr = ccdPtr->ccdNextPtr;
    }
    hashAlgPtr->aalgFinalFuncPtr(crhPtr->crhOutPtr, contextPtr);
    tm_free_raw_buffer(contextPtr);
HASH_RETURN:

    return errorCode;
}

/* This function can be used by any CMAC or HMAC with init, update, and final
 * Currently, only default key length is supported for CMAC.
 * See RFC 3566 for AES-XCBC-MAC-96.
 */
static int tfCryptoSoftwareMacHash(ttCryptoRequestPtr crqPtr)
{
    ttGenericCmacCtx           genCtx;
    ttAhAlgorithmPtr           hashAlgPtr;
    ttCryptoRequestHmacHashPtr hhashPtr;
    ttCryptoChainDataPtr       ccdPtr;
    ttVoidPtr                  contextPtr;
    int                        i;
    int                        errorCode;
    int                        blockLen;
/* inner padding*/
    tt8BitPtr                  keyInPad;
/* outer padding*/
    tt8BitPtr                  keyOutPad;
#ifdef TM_AALG_VAR_BLOCK_SIZE
    tt8BitPtr                  bigKeyHashPtr;
#endif /* TM_AALG_VAR_BLOCK_SIZE */

    errorCode = TM_ENOERROR;
    keyInPad = (tt8BitPtr)0;
    keyOutPad = (tt8BitPtr)0;
    contextPtr = (ttVoidPtr)0;
#ifdef TM_AALG_VAR_BLOCK_SIZE
    bigKeyHashPtr = (ttVoidPtr)0;
#endif /* TM_AALG_VAR_BLOCK_SIZE */

    hhashPtr = crqPtr->crParamUnion.crHmacHashParamPtr;
    if(!hhashPtr || !hhashPtr->crhhOutPtr)
    {
        errorCode = TM_EINVAL;
        goto MAC_HASH_RETURN;
    }

    hashAlgPtr = hhashPtr->crhhAlgPtr;
    if(!hashAlgPtr)
    {
        hashAlgPtr = tfAhAlgorithmLookup(hhashPtr->crhhHashAlg);
        if(!hashAlgPtr)
        {
            errorCode = TM_EINVAL;
            goto MAC_HASH_RETURN;
        }
    }

#ifdef TM_AALG_VAR_BLOCK_SIZE
    blockLen = hashAlgPtr->aalgBlockSize;
#else /* !TM_AALG_VAR_BLOCK_SIZE */
    blockLen = TM_HMAC_BLOCK_SIZE;
#endif /* TM_AALG_VAR_BLOCK_SIZE */

    contextPtr = tm_get_raw_buffer(
                            (ttPktLen)hashAlgPtr->aalgContextLenFuncPtr());
    if(!contextPtr)
    {
        errorCode = TM_ENOBUFS;
        goto MAC_HASH_RETURN;
    }

    if (crqPtr->crType == TM_CEREQUEST_CRYPTO_HMACHASH)
    {
        keyInPad = tm_get_raw_buffer((ttPktLen)blockLen);

        if(!keyInPad)
        {
            errorCode = TM_ENOBUFS;
            goto MAC_HASH_RETURN;
        }
        keyOutPad = tm_get_raw_buffer((ttPktLen)blockLen);
        if(!keyOutPad)
        {
            errorCode = TM_ENOBUFS;
            goto MAC_HASH_RETURN;
        }   
    
/* start out by storing key in pads */
        tm_bzero(keyInPad, blockLen);
        tm_bzero(keyOutPad, blockLen);

#ifdef TM_AALG_VAR_BLOCK_SIZE
/* RFC 4868 states that for keys which are longer than the block size 
 * of the algorithm, the hash of the key is used instead, then padded
 */
        if (hhashPtr->crhhKeyLen > blockLen)
        {
            bigKeyHashPtr = tm_get_raw_buffer(hashAlgPtr->aalgDigestOutBits>>3);
            if(!bigKeyHashPtr)
            {
                errorCode = TM_ENOBUFS;
                goto MAC_HASH_RETURN;
            }
/* compute the hash of the key */
            hashAlgPtr->aalgInitFuncPtr(contextPtr);
            hashAlgPtr->aalgUpdateFuncPtr(contextPtr, 
                                hhashPtr->crhhKeyPtr, 
                                hhashPtr->crhhKeyLen, 
                                0);
            hashAlgPtr->aalgFinalFuncPtr(bigKeyHashPtr,
                                contextPtr);
/* copy the hash to the i-pad and o-pad, where there is already zero padding */
            tm_bcopy(bigKeyHashPtr, keyInPad, hashAlgPtr->aalgDigestOutBits>>3);
            tm_bcopy(bigKeyHashPtr, keyOutPad, hashAlgPtr->aalgDigestOutBits>>3);
            if(bigKeyHashPtr)
            {
                tm_free_raw_buffer(bigKeyHashPtr);
            }
        }
        else
#endif /* TM_AALG_VAR_BLOCK_SIZE */
        {
#ifndef TM_AALG_BLOCK_SIZE
/* If we are NOT using TM_AALG_BLOCK_SIZE, check to see if the key is longer
 * than HMAC_BLOCK_SIZE. This will cause memory corruption.
 */
            if (hhashPtr->crhhKeyLen > blockLen)
            {
                errorCode = TM_EINVAL;
                goto MAC_HASH_RETURN;
            }
#endif /* !TM_AALG_VAR_BLOCK_SIZE */
/* if the key is less than or equal to the block size, 
 * copy the entire key into the pads, which already have zero padding.
 */
            tm_bcopy(hhashPtr->crhhKeyPtr, keyInPad, hhashPtr->crhhKeyLen);
            tm_bcopy(hhashPtr->crhhKeyPtr, keyOutPad, hhashPtr->crhhKeyLen);
        }
/* XOR key with ipad and opad values */
        for (i=0; i < blockLen; i++)
        {
            keyInPad[i] ^= TM_CRYPTO_HMAC_INNER_PAD;
            keyOutPad[i] ^= TM_CRYPTO_HMAC_OUTER_PAD;
        }
        hashAlgPtr->aalgInitFuncPtr(contextPtr);

        hashAlgPtr->aalgUpdateFuncPtr(contextPtr, 
                                    keyInPad, 
                                    (ttPktLen)blockLen, 
                                    0);
/* start with inner pad */
    }
    else
    {
/* The context pointer must be a Cmac context pointer of some kind
 * with a tt8BitPtr keyPtr listed first
 */
        if (hhashPtr->crhhKeyLen != hhashPtr->crhhAlgPtr->aalgKeyDefault>>3)
        {
            errorCode = TM_EINVAL;
            goto MAC_HASH_RETURN;
        }

        genCtx.keyPtr = hhashPtr->crhhKeyPtr;
        genCtx.contextPtr = contextPtr;

        hashAlgPtr->aalgInitFuncPtr((ttVoidPtr)&genCtx);
    }


    ccdPtr = hhashPtr->crhhInDataPtr;
    while(ccdPtr)
    {
        hashAlgPtr ->aalgUpdateFuncPtr(contextPtr, 
                                       ccdPtr->ccdDataPtr, 
                                       ccdPtr->ccdDataLen, 
                                       0);
        ccdPtr = ccdPtr->ccdNextPtr;
    }
/* output this hash into the buffer allocated in tucrypto */
    hashAlgPtr->aalgFinalFuncPtr(hhashPtr->crhhOutPtr, contextPtr);

    if (crqPtr->crType == TM_CEREQUEST_CRYPTO_HMACHASH)
    {

/* the outer (second) round */
        hashAlgPtr->aalgInitFuncPtr(contextPtr);

        hashAlgPtr->aalgUpdateFuncPtr(contextPtr, 
                                      keyOutPad, 
                                      (ttPktLen)blockLen, 
                                      0);

        hashAlgPtr->aalgUpdateFuncPtr(contextPtr, 
                                      hhashPtr->crhhOutPtr, 
                                      (hashAlgPtr->aalgDigestOutBits>>3), 
                                      0);  
        hashAlgPtr->aalgFinalFuncPtr(hhashPtr->crhhOutPtr,
                                     contextPtr);
    }

MAC_HASH_RETURN:
    if(contextPtr)
    {
        tm_free_raw_buffer(contextPtr);
    }
    if(keyInPad)
    {
        tm_free_raw_buffer(keyInPad);
    }
    if(keyOutPad)
    {
        tm_free_raw_buffer(keyOutPad);
    }

    return errorCode;
}

#ifdef TM_PUBKEY_USE_DIFFIEHELLMAN
int tfCryptoDhGenerate(int           dhgroup,
                    tt32Bit       generator,
                    tt16Bit       mySecretLen,
                    tt8BitPtr     primePtr,
                    tt16Bit       primeLength,
                    tt8BitPtrPtr  mySecretPtrPtr,
                    tt8BitPtrPtr  publicPtrPtr,
                    tt16BitPtr    publicLenPtr)
{
    ttCharVector         primeVector;
    ttCharVectorPtr      privVectorPtr;
    ttCharVectorPtr      pubVectorPtr;
    int                  errorCode;
    tt8Bit               needFreePrime;


    errorCode = TM_ENOERROR;
    needFreePrime = TM_8BIT_YES;

    if(!primePtr)
    {
/* ignore primeLength field. We get everything from dhgroup */
        errorCode = tfCryptoDhConstructPrimeVector(dhgroup, &primeVector);
        if(errorCode != TM_ENOERROR)
        {
             goto DHGEN_RETURN;
        }
    }
    else
    {
/* construc tprimeVector from memory */
        primeVector.cvLength = primeLength;
        primeVector.cvValuePtr = (char TM_FAR *)primePtr;
        needFreePrime = TM_8BIT_NO;
    }

    if(generator == 0)
    {
        generator = TM_DH_GENERATOR_2;
    }

    errorCode = tfDhGenerate(&primeVector, 
                             generator, 
                             mySecretLen*8, /* in bits */
                             &pubVectorPtr, 
                             &privVectorPtr);

    if(errorCode != TM_ENOERROR)
    {
         goto DHGEN_RETURN;
    }
    *mySecretPtrPtr = (tt8Bit TM_FAR *)privVectorPtr->cvValuePtr;

#ifdef TM_USE_DIFFIEHELLMAN_PADDING
    *publicLenPtr = (tt16Bit)(primeVector.cvLength);
    if(pubVectorPtr->cvLength == primeVector.cvLength)
    {
        *publicPtrPtr = (tt8Bit TM_FAR *)pubVectorPtr->cvValuePtr;
    }
    else
    {
/* the mod calculation result is not full length. We need pad zero 
 * TO THE FRONT,
 */
        unsigned int diff;
        *publicPtrPtr = tm_get_raw_buffer(primeVector.cvLength);
        if(*publicPtrPtr == (tt8BitPtr)0)
        {
            errorCode = TM_ENOBUFS;
            goto DHGEN_RETURN;
        }
        diff = primeVector.cvLength - pubVectorPtr->cvLength;
        tm_bzero(*publicPtrPtr, diff);
        tm_bcopy(pubVectorPtr->cvValuePtr, 
                 *publicPtrPtr + diff, 
                 pubVectorPtr->cvLength);
        tm_free_raw_buffer((ttRawBufferPtr)pubVectorPtr->cvValuePtr);
    }
#else /* TM_USE_DIFFIEHELLMAN_PADDING */
    *publicLenPtr = (tt16Bit)(pubVectorPtr->cvLength);
    *publicPtrPtr = (tt8Bit TM_FAR *)pubVectorPtr->cvValuePtr;
#endif /* TM_USE_DIFFIEHELLMAN_PADDING  */
    privVectorPtr->cvValuePtr = 0;

    pubVectorPtr->cvValuePtr = 0;

    tfCvFree(pubVectorPtr);
    tfCvFree(privVectorPtr);
    if(primeVector.cvValuePtr)
    {
        if(needFreePrime == TM_8BIT_YES)
        {
           tm_free_raw_buffer((tt8BitPtr)primeVector.cvValuePtr);
        }
    }
DHGEN_RETURN:

    return errorCode;
}

static int tfCryptoDhConstructPrimeVector(int                    dhgroup,
                                       ttCharVector TM_FAR * primeVectorPtr)
{
    char TM_FAR *        primePtr;
    int                  errorCode;

    errorCode = TM_ENOERROR;

    switch(dhgroup)
    {
    case TM_DHGROUP_1:
        primePtr = TM_OAKLEY_PRIME_MODP768;
        primeVectorPtr->cvLength  = 96;
        break;
    case TM_DHGROUP_2:
        primePtr = TM_OAKLEY_PRIME_MODP1024;
        primeVectorPtr->cvLength  = 128;
        break;
    case TM_DHGROUP_5:
        primePtr = TM_OAKLEY_PRIME_MODP1536;
        primeVectorPtr->cvLength  = 192;
        break;
    case TM_DHGROUP_14:
        primePtr = TM_OAKLEY_PRIME_MODP2048;
        primeVectorPtr->cvLength  = 256;
        break;
    default:
        primePtr = (char TM_FAR *)0;
        errorCode = -1;
    }

    if(errorCode == TM_ENOERROR)
    {
        primeVectorPtr->cvValuePtr = tm_get_raw_buffer(
                 primeVectorPtr->cvLength);
        if(primeVectorPtr->cvValuePtr == 0)
        {
            errorCode = TM_ENOBUFS;
            goto DHCONSTR_RETURN;
        }
        errorCode = tfHexByteStr2HexByteArr
                                (primePtr, 
                                 (int)primeVectorPtr->cvLength,
                                 primeVectorPtr->cvValuePtr);
        if(errorCode != TM_ENOERROR)
        {
            tm_free_raw_buffer((ttRawBufferPtr)(primeVectorPtr->cvValuePtr));
        }
    }

DHCONSTR_RETURN:   
    return errorCode;
}

/* use my secret value and your public value to compute our shared secret.*/

int tfCryptoDhGetSharedSecret(int           dhgroup,
                              tt8BitPtr     mySecretPtr,
                              tt16Bit       mySecretLen,
                              tt8BitPtr     primePtr,
                              tt16Bit       primeLength,
                              tt8BitPtr     yourPublicPtr,
                              tt16Bit       yourPublicLen,
                              tt8BitPtrPtr  ourSecretPtrPtr,
                              tt16BitPtr    ourSecretLenPtr)
{
    ttCharVector         primeVector;
    ttCharVector         privVector;
    ttCharVector         pubVector;
    ttCharVectorPtr      sharedSecretPtr;
    int                  errorCode;
#ifdef TM_USE_DIFFIEHELLMAN_PADDING
    unsigned int         diff;
#endif /* TM_USE_DIFFIEHELLMAN_PADDING */
    tt8Bit               needFreePrime;

    errorCode = TM_ENOERROR;
    needFreePrime = TM_8BIT_YES;

    if(!primePtr)
    {
/* ignore primeLength field. We get everything from dhgroup */
        errorCode = tfCryptoDhConstructPrimeVector(dhgroup, &primeVector);
        if(errorCode != TM_ENOERROR)
        {
             goto GET_RETURN;
        }
    }
    else
    {
/* construct primeVector from memory */
        primeVector.cvLength = primeLength;
        primeVector.cvValuePtr = (char TM_FAR *)primePtr;
        needFreePrime = TM_8BIT_NO;
    }

    privVector.cvLength = mySecretLen;
    privVector.cvValuePtr = (char TM_FAR *)mySecretPtr;
    pubVector.cvValuePtr = (char TM_FAR *)yourPublicPtr;
    pubVector.cvLength = yourPublicLen;

    sharedSecretPtr = (ttCharVectorPtr)tm_get_raw_buffer(
              sizeof(ttCharVector));
    if(!sharedSecretPtr)
    {
        errorCode = TM_ENOBUFS;
        goto DHGETSHARED_RETURN;
    }
    tm_bzero(sharedSecretPtr, sizeof(ttCharVector));

    errorCode = tfDhComputeSharedSecret(&primeVector, 
                                        &privVector, 
                                        &pubVector,
                                        sharedSecretPtr);
    if(errorCode != TM_ENOERROR)
    {
        goto DHGETSHARED_RETURN;
    }
#ifdef TM_USE_DIFFIEHELLMAN_PADDING
    *ourSecretLenPtr = (tt16Bit)(primeVector.cvLength);
     if(sharedSecretPtr->cvLength == primeVector.cvLength)
    {
        *ourSecretPtrPtr = (tt8Bit TM_FAR *)sharedSecretPtr->cvValuePtr;
    }
    else
    {
/* the mod calculation result is not full length. We need pad zero TO 
 * THE FRONT,*/

        *ourSecretPtrPtr = tm_get_raw_buffer(primeVector.cvLength);
        if(*ourSecretPtrPtr == (tt8BitPtr)0)
        {
            errorCode = TM_ENOBUFS;
            goto DHGETSHARED_RETURN;
        }
        diff = primeVector.cvLength - sharedSecretPtr->cvLength;
        tm_bzero(*ourSecretPtrPtr, diff);
        tm_bcopy(sharedSecretPtr->cvValuePtr, 
                 *ourSecretPtrPtr + diff, 
                 sharedSecretPtr->cvLength);
        tm_free_raw_buffer((ttRawBufferPtr)sharedSecretPtr->cvValuePtr);
    }
#else /* TM_USE_DIFFIEHELLMAN_PADDING */
    *ourSecretLenPtr = (tt16Bit)sharedSecretPtr->cvLength;
    *ourSecretPtrPtr = (tt8BitPtr)sharedSecretPtr->cvValuePtr;
#endif /* TM_USE_DIFFIEHELLMAN_PADDING */
    sharedSecretPtr->cvValuePtr = (char TM_FAR *)0;

DHGETSHARED_RETURN:
    tfCvFree(sharedSecretPtr);
    if(primeVector.cvValuePtr)
    {
        if(needFreePrime == TM_8BIT_YES)
        {
           tm_free_raw_buffer((tt8BitPtr)primeVector.cvValuePtr);
        }
    }
    
GET_RETURN:
    return errorCode;
}

static int tfCryptoSoftwareDhrequest(tt8Bit                       type,
                                    ttCryptoRequestDhrequestPtr  dhInfoPtr)
{
    tt8BitPtrPtr   kePtrPtr;
    tt8BitPtr      otherPubPtr;
    tt16BitPtr     keLengthPtr;
    int            errorCode;

    errorCode = TM_ENOERROR;
    if(type == TM_CEREQUEST_PUBKEY_DHKEYPAIR)
    {
/* any Diffie Hellman exchange, we need to know who is initiator/responder,
 * so that we know where to put the results
 */
        if(dhInfoPtr->crdIsInitialtor == TM_ISAKMP_INITIATOR)
        {
            kePtrPtr = &dhInfoPtr->crdDhinfoPtr->dhGxiPtr;
            keLengthPtr = &dhInfoPtr->crdDhinfoPtr->dhGxiLen;
        }
        else
        {
            kePtrPtr = &dhInfoPtr->crdDhinfoPtr->dhGxrPtr;
            keLengthPtr = &dhInfoPtr->crdDhinfoPtr->dhGxrLen;
        }
    
        errorCode = tfCryptoDhGenerate(dhInfoPtr->crdDhinfoPtr->dhGroup,
                        dhInfoPtr->crdDhinfoPtr->dhGenerator,
                        dhInfoPtr->crdDhinfoPtr->dhSecretLen,
                        dhInfoPtr->crdDhinfoPtr->dhPrimePtr,
                        dhInfoPtr->crdDhinfoPtr->dhGroupLen,
                        &dhInfoPtr->crdDhinfoPtr->dhSecretPtr,
                        kePtrPtr,
                        keLengthPtr);

    }
    else if(type == TM_CEREQUEST_PUBKEY_DHSHAREDKEY)
    {
        if(dhInfoPtr->crdIsInitialtor == TM_ISAKMP_INITIATOR)
        {
            otherPubPtr = dhInfoPtr->crdDhinfoPtr->dhGxrPtr;
            keLengthPtr = &dhInfoPtr->crdDhinfoPtr->dhGxrLen;
        }
        else
        {
            otherPubPtr = dhInfoPtr->crdDhinfoPtr->dhGxiPtr;
            keLengthPtr = &dhInfoPtr->crdDhinfoPtr->dhGxiLen;
        }
        errorCode = tfCryptoDhGetSharedSecret(dhInfoPtr->crdDhinfoPtr->dhGroup,
                    dhInfoPtr->crdDhinfoPtr->dhSecretPtr,
                    dhInfoPtr->crdDhinfoPtr->dhSecretLen,
                    dhInfoPtr->crdDhinfoPtr->dhPrimePtr,
                    dhInfoPtr->crdDhinfoPtr->dhGroupLen,
                    otherPubPtr,
                    *keLengthPtr,
                    &dhInfoPtr->crdDhinfoPtr->dhGxyPtr,
                    &dhInfoPtr->crdDhinfoPtr->dhGxyLen);

    }

    return errorCode;
}
#endif /* TM_PUBKEY_USE_DIFFIEHELLMAN */

#ifdef TM_USE_PKI
#ifdef TM_PUBKEY_USE_RSA
static int tfCryptoSoftwareRsarequest(tt8Bit                       type,
                                    ttCryptoRequestRsarequestPtr  rsaReqPtr)
{
    
    int         errorCode;
    ttVoidPtr   keyPtr;
    
    errorCode = TM_ENOERROR;
/* key transform, from binary to bignumber */
    keyPtr = tfRsaKeyAsn1ToBn(rsaReqPtr->crrRsainfoPtr); 
    if ( type == TM_CEREQUEST_PUBKEY_RSASIGN )
    {
/* call software engine sign funtion */
        tfRSASign(  rsaReqPtr->crrRsaDataLength,
                    rsaReqPtr->crrRsaDataPtr,
                    rsaReqPtr->crrRsaTransPtr,
                    (ttRSAPtr)keyPtr,
                    rsaReqPtr->crrRsaTransLenPtr);
        
    }
    else if ( type == TM_CEREQUEST_PUBKEY_RSAVERIFY )
    {
/* call software engine verify funtion.
 * if errorCode = 0, then success, otherwise failed
 */
        errorCode = tfRSAVerify ( 
                    rsaReqPtr->crrRsaDataLength,
                    rsaReqPtr->crrRsaDataPtr,
                    rsaReqPtr->crrRsaTransPtr,
                    (ttRSAPtr)keyPtr,
                    *rsaReqPtr->crrRsaTransLenPtr);
    }
    else if(type == TM_CEREQUEST_PUBKEY_RSAENCRYPT)
    {
        errorCode = tfRSAPublicEncrypt(rsaReqPtr->crrRsaDataLength,
                                       rsaReqPtr->crrRsaDataPtr,
                                       rsaReqPtr->crrRsaTransPtr,
                                       (ttRSAPtr)keyPtr,
                                       rsaReqPtr->crrRsaTransLenPtr);
             
    }
    else if(type == TM_CEREQUEST_PUBKEY_RSADECRYPT)
    {
        errorCode = tfRSAPrivateDecrypt(rsaReqPtr->crrRsaDataLength,
                                        rsaReqPtr->crrRsaDataPtr,
                                        rsaReqPtr->crrRsaTransPtr,
                                        (ttRSAPtr)keyPtr,
                                        rsaReqPtr->crrRsaTransLenPtr);
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    if ( keyPtr != (ttVoidPtr)0)
    {
        tfRsaFree((ttRSAPtr)keyPtr);
    }
    return errorCode;
}
#endif /* TM_PUBKEY_USE_RSA */

#ifdef TM_PUBKEY_USE_DSA
static int tfCryptoSoftwareDsarequest(tt8Bit                       type,
                                    ttCryptoRequestDsarequestPtr  dsaReqPtr)
{
    
    int         errorCode;
    ttVoidPtr   keyPtr;
    
    errorCode = TM_ENOERROR;
/* key transform, from binary to bignumber */
    keyPtr = tfDsaKeyAsn1ToBn(dsaReqPtr->crdDsainfoPtr); 

    if ( type == TM_CEREQUEST_PUBKEY_DSASIGN )
    {
/* call software engine sign funtion */
        errorCode = tfDSASign(  dsaReqPtr->crdDsaDataLength,
                    dsaReqPtr->crdDsaDataPtr,
                    dsaReqPtr->crdDsaTransPtr,
                    (ttDSA *)keyPtr,
                    dsaReqPtr->crdDsaTransLenPtr);
        
    }
    else if ( type == TM_CEREQUEST_PUBKEY_DSAVERIFY )
    {
/* call software engine verify funtion.
 * if errorCode = 0, then success, otherwise failed
 */
        errorCode = tfDSAVerify ( 
                    dsaReqPtr->crdDsaDataLength,
                    dsaReqPtr->crdDsaDataPtr,
                    dsaReqPtr->crdDsaTransPtr,
                    (ttDSA *)keyPtr,
                    *dsaReqPtr->crdDsaTransLenPtr);
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    if ( keyPtr != (ttVoidPtr)0)
    {
        tfDsaFree((ttDSA *)keyPtr);
    }
    return errorCode;
}
#endif /* TM_PUBKEY_USE_DSA */
#endif /* TM_USE_PKI */

#endif /* TM_USE_SW_CRYPTOENGINE */

#ifdef TM_USE_ASYNC_CRYPTO
/*
 * tfCryptoAsyncWait() Function Description
 * Pend until tfCryptoAsyncSignal() or tfCryptoAsyncSignalIsr() is called.
 * Called to put the running task to sleep while waiting for encryption
 * hardware to finish working
 *
 * Parameter Description
 * NONE
 * 
 * Returns
 * Value       Meaning
 * TM_ENOERROR Success
 */
int tfCryptoAsyncWait(void)
{
    tm_lock_wait(&(tm_context(tvAsyncCryptoLockEntry)));
    tm_kernel_pend_event(&tm_context(tvAsyncCryptoEvent).eventUnion);
    tm_unlock(&(tm_context(tvAsyncCryptoLockEntry)));
    
    return TM_ENOERROR;
}

/*
 * tfCryptoAsyncSignal() Function Description
 * Awaken tfCryptoAsyncWait(). Called from the application to awaken a task
 * waiting for encryption hardware to finish working.
 *
 * Parameter Description
 * NONE
 * 
 * Returns
 * Value       Meaning
 * TM_ENOERROR Success
 */
int tfCryptoAsyncSignal(void)
{
    tfKernelTaskPostEvent(&tm_context(tvAsyncCryptoEvent).eventUnion);
    
    return TM_ENOERROR;
}

/*
 * tfCryptoAsyncSignalIsr() Function Description
 * Awaken tfCryptoAsyncWait(). Called from the application to awaken a task
 * waiting for encryption hardware to finish working.
 *
 * Parameter Description
 * NONE
 * 
 * Returns
 * Value       Meaning
 * TM_ENOERROR Success
 */
int tfCryptoAsyncSignalIsr(void)
{
    tfKernelIsrPostEvent(&tm_context(tvAsyncCryptoEvent).eventUnion);
    
    return TM_ENOERROR;
}
#endif /* TM_USE_ASYNC_CRYPTO */
void tfCryptoEncAlgToStr(ttUser8Bit module, 
                         ttUser16Bit encAlg, 
                         ttUserConstCharPtrPtr bufferPtrPtr)
{
    switch(module)
    {
    case TM_CRYPTO_MODULE_IKE:
        {
            switch(encAlg)
            {
            case TM_IKE_DES_CBC:
                {
                    *bufferPtrPtr = tlEncAlgDes;
                    break;
                }
            case TM_IKE_IDEA_CBC:
                {
                    *bufferPtrPtr = tlEncAlgIdea;
                    break;
                }
            case TM_IKE_BLOWFISH_CBC:
                {
                    *bufferPtrPtr = tlEncAlgBlowfish;
                    break;
                }
            case TM_IKE_RC5_R16_B64_CBC:
                {
                    *bufferPtrPtr = tlEncAlgRC5R16B64;
                    break;
                }
            case TM_IKE_3DES_CBC:
                {
                    *bufferPtrPtr = tlEncAlg3Des;
                    break;
                }
            case TM_IKE_CAST_CBC:
                {
                    *bufferPtrPtr = tlEncAlgCast;
                    break;
                }
            case TM_IKE_AES_CBC:
                {
                    *bufferPtrPtr = tlEncAlgAes;
                    break;
                }
            case TM_IKE_NULL_ENCRYPT:
                {
                    *bufferPtrPtr = tlEncAlgNull;
                    break;
                }
            case TM_IKE_TWOFISH_CBC:
                {
                    *bufferPtrPtr = tlEncAlgTwoFish;
                    break;
                }
            default:
                {
                    *bufferPtrPtr = tlEncAlgNoSupport;
                    break;
                }            }

        }
    default:
        break;
    }
    *bufferPtrPtr = tlEncAlgDes;
}

void tfCryptoHashAlgToStr(ttUser8Bit module, 
                          ttUser16Bit hashAlg, 
                          ttUserConstCharPtrPtr bufferPtrPtr)
{
    switch(module)
    {
    case TM_CRYPTO_MODULE_IKE:
        {
            switch(hashAlg)
            {
            case TM_IKE_MD5:
                {
                    *bufferPtrPtr = tlHashAlgMd5;
                    break;
                }
            case TM_IKE_SHA1:
                {
                    *bufferPtrPtr = tlHashAlgSha1;
                    break;
                }
            case TM_IKE_SHA256:
                {
                    *bufferPtrPtr = tlHashAlgSha256;
                    break;
                }
            case TM_IKE_SHA384:
                {
                    *bufferPtrPtr = tlHashAlgSha384;
                    break;
                }
            case TM_IKE_SHA512:
                {
                    *bufferPtrPtr = tlHashAlgSha512;
                    break;
                }
            case TM_IKE_TIGER:
                {
                    *bufferPtrPtr = tlHashAlgTiger;
                    break;
                }
            case TM_IKE_RIPEMD:
                {
                    *bufferPtrPtr = tlHashAlgRIPEMD;
                    break;
                }
            default:
                {
                    *bufferPtrPtr = tlHashAlgNoSupport;
                    break;
                }
            }
        }
    default:
        break;
    }
}
