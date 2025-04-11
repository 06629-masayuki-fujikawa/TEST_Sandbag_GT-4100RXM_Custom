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
 * Description: NAT - Network Address Translation
 * Filename: trnat.c
 * Author: Bob Stein
 * Date Created: 09/24/01
 * $Source: source/stubs/nat/trnat.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:22JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * NAT
 * ---
 * The point of NAT is to free a network from certain limitations of IP, such
 * as the requirement for each machine to have a fixed unique address.  The
 * two main uses are when there is only one IP address to be shared by several
 * machines, or there are multiple IP addresses that need to be flexibly
 * assigned to multiple machines (either the machines change often or the set
 * of IP addresses may change).
 *
 * This module enables the Treck TCP/IP stack to become a NAT Router, the
 * link between a private and public network.  Resources inside the private
 * network (clients and servers) are identified differently on the two
 * networks.  A common example is a home or office LAN with a private
 * addressing scheme, and the public Internet with publically assigned IP
 * addresses.
 *
 * CAUTION:  NAT IS NOT A SECURITY FEATURE AND CANNOT SUBSTITUTE FOR A
 * FIREWALL.  A NAT ROUTER SHOULD ONLY BE USED BETWEEN NETWORKS OF EQUAL TRUST
 * LEVELS.
 *
 */


/*
 * Includes
 */
#include <trsocket.h>

/*
 *
 *
 * NAT API (Application Programming Interface) Functions
 *
 *
 */

int tfNatConfig ( ttUserInterface interfaceHandle )
{
    TM_UNREF_IN_ARG(interfaceHandle);
    return TM_EPROTONOSUPPORT;
}

int tfNatConfigNapt ( ttUserInterface interfaceHandle,
                      ttUserIpAddress ipPublic,
                      ttUserIpPort    portPublicMin,
                      ttUserIpPort    portPublicMax)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    ipPublic      = ipPublic;
    TM_UNREF_IN_ARG(portPublicMin);
    TM_UNREF_IN_ARG(portPublicMax);
    return TM_EPROTONOSUPPORT;
}

int tfNatConfigInnerTcpServer ( ttUserInterface interfaceHandle,
                                ttUserIpAddress ipPublic,
                                ttUserIpAddress ipPrivate,
                                ttUserIpPort    portPublic,
                                ttUserIpPort    portPrivate)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    ipPublic    = ipPublic;
    ipPrivate   = ipPrivate;
    portPublic  = portPublic;
    TM_UNREF_IN_ARG(portPrivate);
    return TM_EPROTONOSUPPORT;
}

int tfNatConfigInnerUdpServer ( ttUserInterface interfaceHandle,
                                ttUserIpAddress ipPublic,
                                ttUserIpAddress ipPrivate,
                                ttUserIpPort    portPublic,
                                ttUserIpPort    portPrivate)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    ipPublic    = ipPublic;
    ipPrivate   = ipPrivate;
    portPublic  = portPublic;
    TM_UNREF_IN_ARG(portPrivate);
    return TM_EPROTONOSUPPORT;
}

int tfNatConfigInnerFtpServer ( ttUserInterface interfaceHandle,
                                ttUserIpAddress ipPublic,
                                ttUserIpAddress ipPrivate,
                                ttUserIpPort    portPublicMin,
                                ttUserIpPort    portPublicMax )
{
    TM_UNREF_IN_ARG(interfaceHandle);
    ipPublic      = ipPublic;
    ipPrivate     = ipPrivate;
    TM_UNREF_IN_ARG(portPublicMin);
    TM_UNREF_IN_ARG(portPublicMax);
    return TM_EPROTONOSUPPORT;
}

int tfNatConfigStatic ( ttUserInterface interfaceHandle,
                        ttUserIpAddress ipPublic,
                        ttUserIpAddress ipPrivate)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    ipPublic      = ipPublic;
    ipPrivate     = ipPrivate;
    return TM_EPROTONOSUPPORT;
}

int tfNatConfigDynamic ( ttUserInterface interfaceHandle,
                         ttUserIpAddress ipPublic)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    ipPublic      = ipPublic;
    return TM_EPROTONOSUPPORT;
}

#ifdef TM_NAT_DUMP
void tfNatDump ( ttUserInterface interfaceHandle )
{
    TM_UNREF_IN_ARG(interfaceHandle);
}
#endif /* TM_NAT_DUMP */

