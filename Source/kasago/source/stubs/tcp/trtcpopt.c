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
 * Description:     TCP options stub functions
 *
 * Filename:        stubs/trtcp.c
 * Author:          Odile
 * Date Created:    1/12/99
 * $Source: source/stubs/tcp/trtcpopt.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:27JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

int tfTcpSetOption (ttTcpVectPtr     tcpVectPtr,
                    int              optionName,
                    ttConstCharPtr   optionValuePtr,
                    int              optionLength)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    TM_UNREF_IN_ARG(optionName);
    TM_UNREF_IN_ARG(optionValuePtr);
    TM_UNREF_IN_ARG(optionLength);
    return TM_EPROTONOSUPPORT;
}

int tfTcpGetOption (ttTcpVectPtr   tcpVectPtr,
                    int            optionName,
                    ttCharPtr      optionValuePtr,
                    ttIntPtr       optionLengthPtr)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    TM_UNREF_IN_ARG(optionName);
    TM_UNREF_IN_ARG(optionValuePtr);
    TM_UNREF_IN_ARG(optionLengthPtr);
    return TM_EPROTONOSUPPORT;
}
