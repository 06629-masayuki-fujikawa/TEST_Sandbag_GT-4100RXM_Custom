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
 * Description: BSD Sockets Interface (rresvport)
 *
 * Filename: trrsvprt.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trrsvprt.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:09JST $
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

/* 
 * Reserve a port within a range
 * IPPORT_RESERVEDSTART to IPPORT_RESERVED
 */
int rresvport(int TM_FAR *portToReservePtr)
{
    struct sockaddr_in ourAddress;
    int                socketDescriptor;
    int                retCode;
    int                errorCode;
    int                resvPort;
    ttSockAddrPtrUnion sockAddrPtr;
    
    resvPort = 0;
    sockAddrPtr.sockInPtr = &ourAddress;
/* Make sure that the port number is greater than the minimum */
    if (    (portToReservePtr == (int TM_FAR *)0)
         || (*portToReservePtr < IPPORT_RESERVEDSTART) )
    {
        socketDescriptor = TM_SOCKET_ERROR;
        errorCode = TM_EINVAL;
    }
    else
    {
/* Get the socket descriptor */
        socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);
        if (socketDescriptor != TM_SOCKET_ERROR)
        {
            resvPort = *portToReservePtr;
            do
            {
                ourAddress.sin_len = sizeof(struct sockaddr_in);
                ourAddress.sin_family = PF_INET;
                ourAddress.sin_addr.s_addr = 0;
                ourAddress.sin_port=htons((tt16Bit)resvPort);
/* try to bind to the port number that they requested */
                retCode=bind(socketDescriptor,
                      sockAddrPtr.sockPtr,
                      sizeof(ourAddress));
                if (retCode == TM_SOCKET_ERROR)
                {
                    errorCode=tfGetSocketError(socketDescriptor);
                    if (errorCode != TM_EADDRINUSE)
                    {
/* Bind failed for a reason other than address being used */
                        break;
                    }
                }
                else
                {
/* successfull bind */
                    errorCode=TM_ENOERROR;
                    break; /* out of the loop */
                }
                resvPort--;
/*
 * Loop while the bind fails with EADDRINUSE error and while we are not
 * at the minimum.
 */
            }
            while (resvPort > IPPORT_RESERVEDSTART);

            if (errorCode != TM_ENOERROR)
            {
                if (errorCode == TM_EADDRINUSE)
                {
/* No bind within range */
                    errorCode = TM_EAGAIN;
                }
/* Close the new socket that we created because of an error */
                (void)tfClose(socketDescriptor);
                socketDescriptor=TM_SOCKET_ERROR;
            }
            else
            {
                *portToReservePtr = resvPort;
            }
        }
        else
        {
            errorCode=tfGetSocketError(socketDescriptor);
        }
    }   
    if (errorCode != TM_ENOERROR)
    {
        tfSocketErrorRecord(TM_SOCKET_ERROR, errorCode);
    }
    return (socketDescriptor);
}

