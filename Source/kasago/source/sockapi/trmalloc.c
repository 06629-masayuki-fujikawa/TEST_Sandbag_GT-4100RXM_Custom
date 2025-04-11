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
 * Description: Treck User Malloc/Free functions
 *
 * Filename: trmalloc.c
 * Author: Odile
 * Date Created: 02/18/99
 * $Source: source/sockapi/trmalloc.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:45JST $
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
 
/*
 * User malloc, and free functions to be used with the Treck raw 
 * buffers.
 */
void TM_FAR *  tfMalloc(unsigned int size)
{
    return tm_get_raw_buffer(size);
}

void tfFree(void TM_FAR * ptr)
{
    tm_free_raw_buffer(ptr);
}
