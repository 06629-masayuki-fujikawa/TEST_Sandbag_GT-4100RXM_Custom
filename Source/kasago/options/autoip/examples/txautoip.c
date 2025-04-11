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
 * Description: Test of collision detection logic (for AUTO IP)
 * Filename: txautoip.c
 * Author: Odile Lapidus
 * Date Created: 03/16/2001
 * $Source: examples/txautoip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:31:03JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>

#ifdef TM_USE_AUTO_IP
int tfConfigAutoIp( ttUserInterface interfaceHandle,
                    int             mhomdeIndex );

int tfAutoIpFinish ( ttUserInterface    interfaceHandle,
                      ttUserIpAddress    ipAddress,
                      int                errorCode,
                      ttUserGenericUnion autoIpParam );

int tlConfigured;

void main (void)
{
    ttUserLinkLayer linkLayerHandle;
    ttUserInterface interfaceHandle;
    int             errorCode;
    int             failed;
#ifdef TM_MULTIPLE_CONTEXT
    ttUserContext   contextHandle;
#endif /* TM_MULTIPLE_CONTEXT */

    failed = 0;
#ifndef TM_MULTIPLE_CONTEXT
    tfInitTreck();
#else /* TM_MULTIPLE_CONTEXT */
    tfInitTreckMultipleContext();
    contextHandle = tfCreateTreckContext();
    if (contextHandle == (void TM_FAR *)0)
    {
        printf("Failed to allocate Treck context\n");
        failed = 1;
    }
    else
    {
        tfSetCurrentContext(contextHandle);
    }
#endif /* TM_MULTIPLE_CONTEXT */
/* Setting the tick length at 500 milliseconds */
    if (failed == 0)
    {
        errorCode = tfInitTreckOptions( TM_OPTION_TICK_LENGTH,
                                        (ttUser32Bit)500);
        if (errorCode != TM_ENOERROR)
        {
            printf( "tfInitTreckOptions failed, '%s'\n",
                     tfStrError(errorCode));
            failed = 1;
        }
    }
/* Start Treck */
    if (failed == 0)
    {
        errorCode = tfStartTreck();
        if (errorCode != TM_ENOERROR)
        {
            printf( "tfStartTreck failed, '%s'\n", tfStrError(errorCode));
            failed = 1;
        }
    }
    if (failed == 0)
    {
/* Add loop back driver interface below Ethernet link layer */
        linkLayerHandle = tfUseEthernet();
        interfaceHandle = tfUseScatIntfDriver("TEST", linkLayerHandle,
                                              &errorCode);
        if (errorCode != TM_ENOERROR)
        {
            printf( "Adding interface loop back driver failed '%s'",
                    tfStrError(errorCode) );
            failed = 1;
        }
        else
        {
/* Open the interface with one scattered buffer (mhome zero configuration) */
            errorCode = tfOpenInterface(interfaceHandle, 0, 0,
                                        TM_DEV_IP_USER_BOOT, 1);
            if (errorCode != 0)
            {
                printf("tfOpenInterface failed %d\n", errorCode);
                failed = 1;
            }
            else
            {
/* Configure interface with AUTO IP on mhome index 0 */
                errorCode = tfConfigAutoIp(interfaceHandle, 0);
                if (errorCode != TM_ENOERROR)
                {
                    printf( "Configuring interface with AUTO IP failed '%s'",
                             tfStrError(errorCode) );
                    failed = 1;
                }
            }
        }
    }
    if (failed == 0)
    {
        while (tlConfigured == 0)
        {
            tfTimerUpdate();
            tfTimerExecute();
        }
    }
}

int tfConfigAutoIp( ttUserInterface interfaceHandle,
                    int             mhomeIndex )
{
    ttUserGenericUnion autoIpParam;
    ttUserIpAddress    ipAddress;
    int                errorCode;

    do
    {
/* Pick a random AUTO IP address */
        ipAddress = tfAutoIPPickIpAddress();
        if (ipAddress != (ttUserIpAddress)0)
        {
            autoIpParam.genIntParm = mhomeIndex;
/* Register the call back function for that IP address with the stack */
             errorCode = tfUseCollisionDetection( ipAddress,
                                                  tfAutoIpFinish,
                                                  autoIpParam );

        }
        else
        {
            errorCode = TM_ENOENT;
        }
    } while (errorCode == TM_EADDRINUSE);
    if (errorCode == TM_ENOERROR)
    {
/* 
 * Selected AUTO IP address is not in the ARP cache 
 * Start sending ARP probes on the interface 
 * We use the default probe interval (2s), and number of probes (4) *
 */
        errorCode = tfUserStartArpSend (interfaceHandle, ipAddress, 0, 0, 0);
    }
    return errorCode;
}


int tfAutoIpFinish ( ttUserInterface    interfaceHandle,
                     ttUserIpAddress    ipAddress,
                     int                errorCode,
                     ttUserGenericUnion autoIpParam )

{
    int mhomeIndex;

/* Cancel the collision detection check on that IP address */
    (void)tfCancelCollisionDetection(ipAddress);
    mhomeIndex = autoIpParam.genIntParm;
    if (errorCode == TM_ENOERROR)
    {
/* No collision occurred. Finish configuring the interface */
        tlConfigured = 1;
        if (mhomeIndex == 0)
        {
/* User used tfOpenInterface with TM_DEV_IP_USER_BOOT */
            errorCode = tfFinishOpenInterface( interfaceHandle,
                                               ipAddress,
                                               TM_IP_LOCAL_NETMASK );
            if (errorCode == TM_ENOERROR)
            {
                printf("tfFinishOpenInterface with 0x%x\n", ipAddress);
            }
            else
            {
                printf("tfFinishOpenInterface with 0x%x failed '%s'\n",
                        ipAddress, tfStrError(errorCode));
            }
        }
        else
        {
/* User had already opened the interface on another mhome */
            errorCode = tfConfigInterface( interfaceHandle,
                                           ipAddress,
                                           TM_IP_LOCAL_NETMASK,
                                           0, /* not used. */
                                           1, /* not used. has to be one */
                                           (unsigned char)mhomeIndex );

            if (errorCode == TM_ENOERROR)
            {
                printf("tfConfigInterface with 0x%x\n", ipAddress);
            }
            else
            {
                printf("tfConfigInterface with 0x%x failed '%s'\n",
                        ipAddress, tfStrError(errorCode));
            }
        }
    }
    else
    {
/* A collision occurred on the IP address, try another IP address */
       tfConfigAutoIp(interfaceHandle, mhomeIndex);
    }
    return 0;
}
#else /* !TM_USE_AUTO_IP */
int tvAutoIpDummy = 0;
#endif /* TM_USE_AUTO_IP */
