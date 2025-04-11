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
 * Description: 802.3 Ethernet Link Layer
 *
 * Filename: tr8023.c
 * Author: Paul
 * Date Created: 11/12/04
 * $Source: source/stubs/8023/tr8023.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:26JST $
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

#ifdef TM_USE_ETHER_8023_LL

/* 
 * Used to initialize the link layer in the link layer list
 */
ttUserLinkLayer tfUseE8023(int llMode)
{
    TM_UNREF_IN_ARG(llMode);
    return((ttUserLinkLayer)0);    
}

#else /* !TM_USE_ETHER_8023_LL */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_ETHER_8023_LL is not defined */
int tv8023Dummy = 0;

#endif /* !TM_USE_ETHER_8023_LL */
