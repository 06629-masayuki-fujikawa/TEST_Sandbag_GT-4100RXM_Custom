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
 * Description:     DNS Hosts Table
 * Filename:        trhosts.c
 * Author:          Lorne
 * Date Created:    6/3/09
 * $Source: source/trhosts.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:31JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


/* Signature longword for host entry allocation */
#define TM_HOST_ENTRY_SIGNATURE TM_UL(0x48454e54)    /* "HENT" */

/* Maximum hostname length (including null terminator) */
#define TM_HOST_MAX_HOSTNAME_LENGTH 256

/* Prevent infinite loop. */
#define TM_HOST_MAX_LOOP 30000

/* Maximum length of the hostname used for IPv4 reverse lookups
 * (xxx.xxx.xxx.xxx.in-addr.arpa) */
#define TM_MAX_IN_ARPA_LENGTH   30

/* Resource record query types */
#define TM_DNS_QTYPE_A          1       /* Domain name to host address */
#define TM_DNS_QTYPE_REV        12      /* Host address to domain name */

/* IPv4/IPv6 dependent macros */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
#define tm_is_af_valid(af) ( (af) == AF_INET || (af) == AF_INET6 )
#define tm_addr_in_struct(af, ap) ( ((af) == AF_INET6) \
            ? (ttVoidPtr)&((struct sockaddr_in6 TM_FAR *)(ap))->sin6_addr \
            : (ttVoidPtr)&((struct sockaddr_in  TM_FAR *)(ap))->sin_addr )
#define tm_addr_size(af) ( ((af) == AF_INET6) \
            ? sizeof(struct in6_addr) \
            : sizeof(struct in_addr) )
#else /* ! TM_USE_IPV6 */
#define tm_is_af_valid(af) ((af) == AF_INET)
#define tm_addr_in_struct(af, ap) ( \
            (ttVoidPtr)&((struct sockaddr_in TM_FAR *)(ap))->sin_addr )
#define tm_addr_size(af) sizeof(struct in_addr)
#endif /* TM_USE_IPV6 */
#else /* ! TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#define tm_is_af_valid(af) ((af) == AF_INET6)
#define tm_addr_in_struct(af, ap) ( \
            (ttVoidPtr)&((struct sockaddr_in6 TM_FAR *)(ap))->sin6_addr )
#define tm_addr_size(af) sizeof(struct in6_addr)
#endif /* TM_USE_IPV6 */
#endif /* TM_USE_IPV4 */

/* IN-ADDR.ARPA domain suffix */
static const char tlInAddrArpaDomain[] = "in-addr.arpa";


/*
 * tfFindHostByName
 *
 * Find the entry specified by name in the hosts table.
 * The pointer to this function is installed in tvDnsHostTableSearchFuncPtr
 * the first time an entry is added to the hosts table.
 *
 * Parameter
 * hostnameStr  Name of the host to find
 *
 * Returns      Pointer to the host entry if found.
 *              NULL if no entry found with the specified name.
 *
 *              Hosts table is locked.
 */
TM_NEARCALL static ttDnsHostEntry TM_FAR * tfFindHostByName(
    const char TM_FAR *     hostnameStr )
{
    ttDnsHostEntryPtr   entryPtr;
    ttDnsHostEntryPtr   listPtr;
    int                 maxLoops;

    maxLoops = TM_HOST_MAX_LOOP;       /* Prevent infinite loop */

/* Find the entry with the same hostname. */
    listPtr = (ttDnsHostEntryPtr)&tm_context(tvDnsHostTable);
    entryPtr = listPtr;
    tm_call_lock_wait(&tm_context(tvDnsHostTableLockEntry));
    do
    {
        entryPtr = (ttDnsHostEntryPtr)entryPtr->hostEntryNode.nodeNextPtr;
        if (entryPtr == listPtr || --maxLoops == 0)
        {
            entryPtr = (ttDnsHostEntryPtr)0;
            break;
        }
    } while (tm_strcmp(entryPtr->hostnameStr, hostnameStr) != 0);

/* Return the entry (NULL if no entry found). */
    return(entryPtr);
}

#ifdef TM_USE_IPV4
/*
 * tfFindHostByAddr
 *
 * Find the entry in the hosts table with the specified address.
 *
 * Parameter
 * addressPtr   Name of the host to find
 *
 * Returns      Pointer to the host entry if found.
 *              NULL if no entry found with the specified name.
 *
 *              Hosts table is locked.
 */
static ttDnsHostEntryPtr tfFindHostByAddr(
    const struct sockaddr_storage TM_FAR *  addressPtr )
{
    ttDnsHostEntryPtr   entryPtr;
    ttDnsHostEntryPtr   listPtr;
    int                 maxLoops;
    int                 addressFamily;
    int                 addressLength;
    ttVoidPtr           ipAddressPtr;
    ttVoidPtr           entryAddressPtr;

    maxLoops = TM_HOST_MAX_LOOP;       /* Prevent infinite loop */

/* Find the entry with the same address. */
    addressFamily = addressPtr->ss_family;
    addressLength = tm_addr_size(addressFamily);
    ipAddressPtr = tm_addr_in_struct(addressFamily, addressPtr);
    listPtr = (ttDnsHostEntryPtr)&tm_context(tvDnsHostTable);
    entryPtr = listPtr;
    tm_call_lock_wait(&tm_context(tvDnsHostTableLockEntry));
    do
    {
        entryPtr = (ttDnsHostEntryPtr)entryPtr->hostEntryNode.nodeNextPtr;
        if (entryPtr == listPtr || --maxLoops == 0)
        {
            entryPtr = (ttDnsHostEntryPtr)0;
            break;
        }
        entryAddressPtr = tm_addr_in_struct(addressFamily,
                                            &entryPtr->hostAddress);
    } while ( addressFamily != entryPtr->hostAddress.ss_family
           || tm_memcmp(ipAddressPtr, entryAddressPtr, addressLength) != 0 );

/* Return the entry (NULL if no entry found). */
    return(entryPtr);
}
#endif /* TM_USE_IPV4 */

/*
 * tfAddHostEntry
 *
 * Add a new entry to the hosts table or replace an existing
 * entry with the same hostname. Return the pointer to the host entry
 * (optional).
 *
 * Parameters
 * hostnameStr      Name of host entry to add
 * addressPtr       IP address to associate with hostnameStr
 * entryPtrPtr      Storage to receive host entry pointer. May be NULL.
 *
 * Returns
 * TM_ENOERROR      Host entry added successfully.
 * TM_ENOMEM        Memory allocation failed.
 */
static int tfAddHostEntry(
    const char TM_FAR *                     hostnameStr,
    const struct sockaddr_storage TM_FAR *  addressPtr,
    ttDnsHostEntryPtr TM_FAR *              entryPtrPtr )
{
    int                 errorCode;
    ttDnsHostEntryPtr   entryPtr;
    int                 addressFamily;
    int                 addressLength;
    ttCharPtr           namePtr;
    ttVoidPtr           srcAddressPtr;
    ttVoidPtr           destAddressPtr;

    errorCode = TM_ENOERROR;
    addressFamily = addressPtr->ss_family;

/* Search for an existing host entry with the same hostname. Function
 * returns with hosts table locked. */
    entryPtr = tfFindHostByName(hostnameStr);

/* Create a new host entry. Allocate enough storage for the following:
 *  - Host entry structure.
 *  - A copy of the hostname string, including null terminator.
 */
    if (entryPtr == (ttDnsHostEntryPtr)0)
    {
        entryPtr = (ttDnsHostEntryPtr)tm_get_raw_buffer(
                    sizeof(ttDnsHostEntry)
                    + tm_strlen(hostnameStr) + 1);
        if (entryPtr == (ttDnsHostEntryPtr)0)
        {
/* Unlock the hosts table */
            tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));
            errorCode = TM_ENOMEM;
            goto addHostEntryExit;
        }

        namePtr = (ttCharPtr)(entryPtr + 1);
        tm_bzero(entryPtr, sizeof(ttDnsHostEntry));
        tm_strcpy(namePtr, hostnameStr);
        entryPtr->hostnameStr = namePtr;

/* Install the new host entry into the table */
        tfListAddToHead(&tm_context(tvDnsHostTable), (ttNodePtr)entryPtr);

/* Install search function on the first addition. */
        if (!tm_context(tvDnsHostTableSearchFuncPtr))
        {
            tm_context(tvDnsHostTableSearchFuncPtr) = tfFindHostByName;
        }
    }

/* Update the address information. */
    srcAddressPtr = tm_addr_in_struct(addressFamily, addressPtr);
    destAddressPtr = tm_addr_in_struct(addressFamily, &entryPtr->hostAddress);
    addressLength = tm_addr_size(addressFamily);
    entryPtr->hostAddress.ss_family = (u_char)addressFamily;
    tm_bcopy(srcAddressPtr, destAddressPtr, addressLength);

/* Unlock the hosts table. */
    tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));

/* Return the host entry to the caller. */
    if (entryPtrPtr != (ttDnsHostEntryPtrPtr)0)
    {
        *entryPtrPtr = entryPtr;
    }

addHostEntryExit:
    return(errorCode);
}

/*
 * tfAddHost
 *
 * Add a new entry to the hosts table or replace an existing
 * entry with the same hostname.
 *
 * Parameters
 * hostnameStr      Name of host entry to add
 * addressPtr       IP address to associate with hostnameStr
 *
 * Returns
 * TM_ENOERROR      Host entry added successfully.
 * TM_EPERM         Call tfDnsInit() first.
 * TM_EINVAL        hostnameStr is NULL or string length is 0
 *                  or addressPtr is NULL.
 * TM_EAFNOSUPPORT  Address is not AF_INET or AF_INET6.
 * TM_ENOMEM        Memory allocation failed.
 */
int tfAddHost(
    const char TM_FAR *                    hostnameStr,
    const struct sockaddr_storage TM_FAR * addressPtr )
{
    int     errorCode;

/* Verify the input values. */
    if (tm_context(tvDnsInitialized) != TM_8BIT_YES)
    {
        errorCode = TM_EPERM;
    }
    else if (   hostnameStr == (ttConstCharPtr)0
            ||  *hostnameStr == '\0'
            ||  addressPtr == (ttConstSockAddrStoragePtr)0 )
    {
        errorCode = TM_EINVAL;
    }
    else if (!tm_is_af_valid(addressPtr->ss_family))
    {
        errorCode = TM_EAFNOSUPPORT;
    }
    else
    {
        errorCode = tfAddHostEntry(hostnameStr, addressPtr,
                                (ttDnsHostEntryPtrPtr)0);
    }

    return(errorCode);
}

/*
 * tfAddHostStr
 *
 * Add a new entry to the hosts table or replace an existing
 * entry with the same hostname.
 *
 * Parameters
 * hostnameStr      Name of host entry to add
 * addressStr       IP address to associate with hostnameStr
 * addressFamily    AF_INET or AF_INET6
 *
 * Returns
 * TM_ENOERROR      Host entry added successfully.
 * TM_EPERM         Call tfDnsInit() first.
 * TM_EINVAL        hostnameStr is NULL or string length is 0
 *                  or addressStr is NULL.
 * TM_EAFNOSUPPORT  addressFamily is not AF_INET or AF_INET6.
 * TM_ENOMEM        Memory allocation failed.
 */
int tfAddHostStr(
    const char TM_FAR *                    hostnameStr,
    const char TM_FAR *                    addressStr,
    int                                    addressFamily )
{
    int                     errorCode;
    struct sockaddr_storage ipAddress;
    ttVoidPtr               destAddressPtr;

/* Verify the input values. */
    if (tm_context(tvDnsInitialized) != TM_8BIT_YES)
    {
        errorCode = TM_EPERM;
    }
    else if (   hostnameStr == (ttConstCharPtr)0
            ||  *hostnameStr == '\0'
            ||  addressStr == (ttConstCharPtr)0 )
    {
        errorCode = TM_EINVAL;
    }
    else if (!tm_is_af_valid(addressFamily))
    {
        errorCode = TM_EAFNOSUPPORT;
    }
    else
    {
        destAddressPtr = tm_addr_in_struct(addressFamily, &ipAddress);
        errorCode = inet_pton(addressFamily, addressStr, destAddressPtr);
        switch (errorCode)
        {
        case 1:
            ipAddress.ss_family = (u_char)addressFamily;
            errorCode = tfAddHostEntry(hostnameStr, &ipAddress,
                                (ttDnsHostEntryPtrPtr)0);
            break;
        case -1:
            errorCode = TM_EAFNOSUPPORT;
            break;
        default:
            errorCode = TM_EINVAL;
        }
    }

    return(errorCode);
}

/*
 * tfDeleteHost
 *
 * Remove a host from the hosts table.
 *
 * Parameter
 * hostnameStr      Name of host entry to remove
 *
 * Returns
 * TM_ENOERROR      Host entry removed successfully.
 * TM_EPERM         Call tfDnsInit() first.
 * TM_ENOENT        Host not found in table.
 * TM_EINVAL        hostnameStr is NULL or length is 0.
 */
int tfDeleteHost(
    const char TM_FAR *     hostnameStr )
{
    int                 errorCode;
    ttDnsHostEntryPtr   entryPtr;

    errorCode = TM_ENOERROR;

/* Verify the input values. */
    if (tm_context(tvDnsInitialized) != TM_8BIT_YES)
    {
        errorCode = TM_EPERM;
    }
    else if (   hostnameStr == (ttConstCharPtr)0
            ||  *hostnameStr == '\0' )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
/* Search for an existing host entry with the same hostname. Function
 * returns with hosts table locked. */
        entryPtr = tfFindHostByName(hostnameStr);
        if (entryPtr == (ttDnsHostEntryPtr)0)
        {
            errorCode = TM_ENOENT;
        }
        else
        {
            tfListRemove(&tm_context(tvDnsHostTable), (ttNodePtr)entryPtr);
/* Release the memory used by the entry. */
            tm_free_raw_buffer((ttRawBufferPtr)entryPtr);
        }
/* Unlock the hosts table. */
        tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));
    }

    return(errorCode);
}

/*
 * tfGetHostCount
 *
 * Return the number of entries in the hosts table.
 */
int tfGetHostCount(void)
{
    int         loopCount;

    loopCount = 0;

/* Make sure DNS subsystem is initialized. */
    if (tm_context(tvDnsInitialized) == TM_8BIT_YES)
    {
/* Lock the hosts table, in case load needs more than 1 instruction.
 * Retrieve the list length.
 * Unlock the hosts table. */
        tm_call_lock_wait(&tm_context(tvDnsHostTableLockEntry));
        loopCount = tm_context(tvDnsHostTable).listCount;
        tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));
    }

    return(loopCount);
}

/*
 * tfGetHostRecord
 *
 * Return the host record at the specified index.
 * The first host record is at index 1.
 *
 * Parameter
 * index             Index of host entry starting at 1
 * hostnameStr       Storage for name of host entry
 * hostnameStrLength Size of hostnameStr buffer (incl. null terminator)
 * addressPtr        Storage for IP address to associate with hostnameStr
 * addressFamilyPtr  Storage for address family
 *
 * Returns
 * TM_ENOERROR      Host entry removed successfully.
 * TM_EPERM         Call tfDnsInit() first.
 * TM_EINVAL        index < 1,
 *                  hostnameStr is NULL,
 *                  hostnameStrLength < 1,
 *                  addressPtr is NULL, or
 *                  addressFamilyPtr is NULL.
 * TM_ERANGE        index is too large.
 * TM_ENOSPC        hostnameStr storage is too small.
 */
int tfGetHostRecord(
    int                                 index,
    char TM_FAR *                       hostnameStr,
    int                                 hostnameStrLength,
    struct sockaddr_storage TM_FAR *    addressPtr,
    int TM_FAR *                        addressFamilyPtr )
{
    ttDnsHostEntryPtr   entryPtr;
    ttDnsHostEntryPtr   listPtr;
    int                 errorCode;

    errorCode = TM_ENOERROR;

/* Verify the input values. */
    if (tm_context(tvDnsInitialized) != TM_8BIT_YES)
    {
        errorCode = TM_EPERM;
        goto getHostRecordExit;
    }
    if ( index <= 0
    ||   hostnameStr == (ttCharPtr)0
    ||   hostnameStrLength == 0
    ||   addressPtr == TM_SOCKADDR_STORAGE_NULL_PTR
    ||   addressFamilyPtr == (ttIntPtr)0 )
    {
        errorCode = TM_EINVAL;
        goto getHostRecordExit;
    }

/* Find the entry at the specified index. */
    listPtr = (ttDnsHostEntryPtr)&tm_context(tvDnsHostTable);
    entryPtr = listPtr;
/* Lock the hosts table. */
    tm_call_lock_wait(&tm_context(tvDnsHostTableLockEntry));
    if ((unsigned int)index > tm_context(tvDnsHostTable).listCount)
    {
/* Unlock the hosts table. */
        tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));
        errorCode = TM_ERANGE;
        goto getHostRecordExit;
    }
    do
    {
        entryPtr = (ttDnsHostEntryPtr)entryPtr->hostEntryNode.nodeNextPtr;
    } while (entryPtr != listPtr && --index > 0);

/* Copy the entry data to the user, if possible. */
    if (entryPtr == listPtr)
    {
        errorCode = TM_ERANGE;
    }
    else if ((unsigned int)hostnameStrLength
                    < tm_strlen(entryPtr->hostnameStr) + 1)
    {
        errorCode = TM_ENOSPC;
    }
    else
    {
        tm_strcpy(hostnameStr, entryPtr->hostnameStr);
        tm_bcopy(&entryPtr->hostAddress, addressPtr,
                            sizeof(struct sockaddr_storage));
        *addressFamilyPtr = entryPtr->hostAddress.ss_family;
    }

/* Unlock the hosts table. */
    tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));

getHostRecordExit:
    return errorCode;
}

#ifdef TM_USE_IPV4
/*
 * tfGetHostByName
 *
 * Return a pointer to a host entry structure corresponding to the
 * hostname specified.
 * This function only supports IPv4 addresses.
 * If successful, the user must subsequently call tfFreeHostEntry() to
 * release the host entry storage returned by this function.
 *
 * Parameter
 * hostnameStr      Name of host to query
 * hostEntryPtrPtr  Storage for pointer to the host entry structure
 *
 * Returns
 * TM_ENOERROR      Host lookup successful.
 * TM_EPERM         Call tfDnsInit() first.
 * TM_EINVAL        Invalid host name string or host entry pointer.
 * TM_ENOMEM        Memory allocation failed.
 * TM_EAFNOSUPPORT  Address returned from server is not AF_INET or AF_INET6.
 * TM_EWOULDBLOCK   DNS lookup in progress.  The user should continue to call
 *                  tfGetHostByName with the same parameters until it returns
 *                  a value other than TM_EWOULDBLOCK.
 * Other values returned by tfDnsGenerateQuery():
 * TM_ETIMEDOUT
 * TM_ENOBUFS
 * TM_DNS_EANSWER
 * TM_DNS_ECACHE_FULL
 * TM_EINPROGRESS
 * ... and values returned from tfZeroCopySendTo()
 */
int tfGetHostByName(
    const char TM_FAR *                 hostnameStr,
    struct tsHostent TM_FAR * TM_FAR *  hostEntryPtrPtr )
{
    int                         errorCode;
    ttDnsHostEntryPtr           tableEntryPtr;
    struct tsHostent TM_FAR *   hostEntryPtr;
    ttDnsCacheEntryPtr          dnsEntryPtr;
    struct addrinfo TM_FAR *    addrInfoPtr;
    int                         addrInfoCount;
    ttCharPtrPtr                aliasArray;
    ttCharPtrPtr                addrPtrArray;
    tt32BitPtr                  signaturePtr;
    struct in_addr TM_FAR *     inAddrArray;
    ttCharPtr                   namePtr;
    ttSockAddrPtrUnion          sockAddrPtr;
    int                         i;

    errorCode = TM_ENOERROR;
    *hostEntryPtrPtr = 0;

/* Verify the input values. */
    if (tm_context(tvDnsInitialized) != TM_8BIT_YES)
    {
        errorCode = TM_EPERM;
        goto getHostByNameExit;
    }
    if (hostnameStr == (ttConstCharPtr)0
    ||  *hostnameStr == '\0'
    ||  hostEntryPtrPtr == (ttHostentPtrPtr)0)
    {
        errorCode = TM_EINVAL;
        goto getHostByNameExit;
    }

/* Search the hosts table. */
    tableEntryPtr = tfFindHostByName(hostnameStr);
    if (tableEntryPtr != (ttDnsHostEntryPtr)0
    &&  tableEntryPtr->hostAddress.ss_family == AF_INET)
    {
/* Entry found in hosts table.
 * Copy the data from the entry in the hosts table.
 * Allocate enough storage for the following:
 *  - Host entry structure.
 *  - A longword signature.
 *  - An NULL terminated array of name alias string pointers.
 *  - An NULL terminated array of IP address pointers.
 *  - struct in_addr.
 *  - The hostname, including null terminator.
 */
        hostEntryPtr = (ttHostentPtr)tm_get_raw_buffer(
                        sizeof(ttHostent)
                        + sizeof(tt32Bit)
                        + 1 * sizeof(ttCharPtr)
                        + 2 * sizeof(ttCharPtr)
                        + sizeof(struct in_addr)
                        + tm_strlen(hostnameStr) + 1 );
        if (hostEntryPtr == (ttHostentPtr)0)
        {
/* Unlock the hosts table */
            tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));
            errorCode = TM_ENOMEM;
            goto getHostByNameExit;
        }

/* Copy the address from the hosts table entry. */
        signaturePtr = (tt32BitPtr)(hostEntryPtr + 1);
        aliasArray = (ttCharPtrPtr)(signaturePtr + 1);
        addrPtrArray = aliasArray + 1;
        inAddrArray = (struct in_addr TM_FAR *)(addrPtrArray + 2);
        namePtr = (ttCharPtr)(inAddrArray + 1);
        inAddrArray[0].s_addr =
                    tableEntryPtr->hostAddress.addr.ipv4.sin_addr.s_addr;
/* Unlock the hosts table */
        tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));

/* Add a signature longword.
 * Fill in the host entry structure. */
        *signaturePtr = TM_HOST_ENTRY_SIGNATURE;
        hostEntryPtr->h_addrtype = AF_INET;
        hostEntryPtr->h_length = sizeof(struct in_addr);
        hostEntryPtr->h_name = namePtr;
        tm_strcpy(namePtr, hostnameStr);
        hostEntryPtr->h_aliases = aliasArray;
        aliasArray[0] = (ttCharPtr)0;               /* No aliases */
        hostEntryPtr->h_addr_list = addrPtrArray;
        addrPtrArray[0] = (ttCharPtr)inAddrArray;   /* One address */
        addrPtrArray[1] = (ttCharPtr)0;
    }
    else
    {
/* No entry in the hosts table.
 * Unlock the hosts table.
 * Lock the DNS global variables. */
        tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));
        tm_call_lock_wait(&tm_context(tvDnsLockEntry));

/* Send a request to the DNS resolver. */
        errorCode = tfDnsGenerateQuery( hostnameStr,
                                        TM_DNS_QTYPE_A,
                                        AF_INET,
                                        &dnsEntryPtr );
        if (errorCode != TM_ENOERROR || dnsEntryPtr == (ttDnsCacheEntryPtr)0)
        {
/* Unlock the DNS global variables */
            tm_call_unlock(&tm_context(tvDnsLockEntry));
            goto getHostByNameExit;
        }

/* Host address resolved.
 * Count the number of addresses for this host. */
        addrInfoCount = 0;
        addrInfoPtr = dnsEntryPtr->dnscAddrInfoPtr;
        while (addrInfoPtr != (struct addrinfo TM_FAR *)0)
        {
            if (addrInfoPtr->ai_addr->sa_family == AF_INET)
            {
                ++addrInfoCount;
            }
            addrInfoPtr = addrInfoPtr->ai_next;
        }

/* Copy the data from the DNS cache entry.
 * Allocate enough storage for the following:
 *  - Host entry structure.
 *  - A longword signature.
 *  - An NULL terminated array of name alias string pointers.
 *  - An NULL terminated array of IP address pointers.
 *  - An array of struct in_addr.
 *  - The hostname, including null terminator.
 */
        hostEntryPtr = (ttHostentPtr)tm_get_raw_buffer(
                        sizeof(ttHostent)
                        + sizeof(tt32Bit)
                        + 1 * sizeof(ttCharPtr)
                        + (addrInfoCount + 1) * sizeof(ttCharPtr)
                        + addrInfoCount * sizeof(struct in_addr)
                        + tm_strlen(hostnameStr) + 1 );
        if (hostEntryPtr == (ttHostentPtr)0)
        {
/* Unlock the DNS global variables */
            tm_call_unlock(&tm_context(tvDnsLockEntry));
            errorCode = TM_ENOMEM;
            goto getHostByNameExit;
        }

/* Copy the addresses from the DNS cache entry. */
        signaturePtr = (tt32BitPtr)(hostEntryPtr + 1);
        aliasArray = (ttCharPtrPtr)(signaturePtr + 1);
        addrPtrArray = aliasArray + 1;
        inAddrArray = (struct in_addr TM_FAR *)
                                    (addrPtrArray + addrInfoCount + 1);
        namePtr = (ttCharPtr)(inAddrArray + addrInfoCount);
        addrInfoPtr = dnsEntryPtr->dnscAddrInfoPtr;
        for (i = 0; i < addrInfoCount; )
        {
            sockAddrPtr.sockPtr = addrInfoPtr->ai_addr;
            if (sockAddrPtr.sockPtr->sa_family == AF_INET)
            {
                addrPtrArray[i] = (ttCharPtr)&inAddrArray[i];
                inAddrArray[i++].s_addr =
                            sockAddrPtr.sockInPtr->sin_addr.s_addr;
            }
            addrInfoPtr = addrInfoPtr->ai_next;
        }
        addrPtrArray[i] = (ttCharPtr)0;
/* Unlock the DNS global variables */
        tm_call_unlock(&tm_context(tvDnsLockEntry));

/* Add a signature longword.
 * Fill in the host entry structure. */
        *signaturePtr = TM_HOST_ENTRY_SIGNATURE;
        hostEntryPtr->h_addrtype = AF_INET;
        hostEntryPtr->h_length = sizeof(struct in_addr);
        hostEntryPtr->h_name = namePtr;
        tm_strcpy(namePtr, hostnameStr);
        hostEntryPtr->h_aliases = aliasArray;
        aliasArray[0] = (ttCharPtr)0;               /* No aliases */
        hostEntryPtr->h_addr_list = addrPtrArray;
    }

/* Return the host entry to the caller. */
    *hostEntryPtrPtr = hostEntryPtr;

getHostByNameExit:
    return(errorCode);
}

/*
 * tfGetHostByAddr
 *
 * Return a pointer to the host entry structure corresponding to the
 * host address specified.
 * This function only supports IPv4 addresses.
 * If successful, the user must subsequently call tfFreeHostEntry() to
 * release the host entry storage returned by this function.
 *
 * Parameter
 * addressPtr       Address of host to query
 * hostEntryPtrPtr  Storage for pointer to the host entry structure
 *
 * Returns
 * TM_ENOERROR      Host lookup successful.
 * TM_EPERM         Call tfDnsInit() first.
 * TM_EINVAL        Invalid host name string or host entry pointer.
 * TM_EAFNOSUPPORT  Address is not AF_INET or AF_INET6.
 * TM_ENOMEM        Memory allocation failed.
 * TM_EWOULDBLOCK   DNS lookup in progress.  The user should continue to call
 *                  tfGetHostByName with the same parameters until it returns
 *                  a value other than TM_EWOULDBLOCK.
 * Other values returned by tfDnsGenerateQuery():
 * TM_ETIMEDOUT
 * TM_ENOBUFS
 * TM_DNS_EANSWER
 * TM_DNS_ECACHE_FULL
 * TM_EINPROGRESS
 * ... and values returned from tfZeroCopySendTo()
 */
int tfGetHostByAddr(
    const struct sockaddr_storage TM_FAR *  addressPtr,
    struct tsHostent TM_FAR * TM_FAR *      hostEntryPtrPtr )
{
    int                         errorCode;
    ttDnsHostEntryPtr           tableEntryPtr;
    struct tsHostent TM_FAR *   hostEntryPtr;
    ttDnsCacheEntryPtr          dnsEntryPtr;
    ttCharPtrPtr                aliasArray;
    ttCharPtrPtr                addrPtrArray;
    tt32BitPtr                  signaturePtr;
    struct in_addr TM_FAR *     inAddrArray;
    ttCharPtr                   namePtr;
    int                         i;
    char                        inAddressBuf[TM_MAX_IN_ARPA_LENGTH];
    ttCharPtr                   inAddressPtr;
    int                         byte;
    tt32Bit                     word;

    errorCode = TM_ENOERROR;
    *hostEntryPtrPtr = 0;

/* Verify the input values. */
    if (tm_context(tvDnsInitialized) != TM_8BIT_YES)
    {
        errorCode = TM_EPERM;
        goto getHostByAddrExit;
    }
    if (addressPtr == (ttConstSockAddrStoragePtr)0
    ||  hostEntryPtrPtr == (ttHostentPtrPtr)0 )
    {
        errorCode = TM_EINVAL;
        goto getHostByAddrExit;
    }
    if (addressPtr->ss_family != AF_INET)
    {
        errorCode = TM_EAFNOSUPPORT;
        goto getHostByAddrExit;
    }

/* Search the hosts table. */
    tableEntryPtr = tfFindHostByAddr(addressPtr);
    if (tableEntryPtr)
    {
/* Entry found in hosts table.
 * Copy the data from the entry in the hosts table.
 * Allocate enough storage for the following:
 *  - Host entry structure.
 *  - A longword signature.
 *  - An NULL terminated array of IP address pointers.
 *  - An NULL terminated array of name alias string pointers.
 *  - The hostname, including null terminator.
 */
        hostEntryPtr = (ttHostentPtr)tm_get_raw_buffer(
                        sizeof(ttHostent)
                        + sizeof(tt32Bit)
                        + 1 * sizeof(ttCharPtr)
                        + 2 * sizeof(ttCharPtr)
                        + sizeof(struct in_addr)
                        + tm_strlen(tableEntryPtr->hostnameStr) + 1 );
        if (hostEntryPtr == (ttHostentPtr)0)
        {
/* Unlock the hosts table */
            tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));
            errorCode = TM_ENOMEM;
            goto getHostByAddrExit;
        }

/* Copy the hostname from the hosts table entry. */
        signaturePtr = (tt32BitPtr)(hostEntryPtr + 1);
        aliasArray = (ttCharPtrPtr)(signaturePtr + 1);
        addrPtrArray = aliasArray + 1;
        inAddrArray = (struct in_addr TM_FAR *)(addrPtrArray + 2);
        namePtr = (ttCharPtr)(inAddrArray + 1);
        tm_strcpy(namePtr, tableEntryPtr->hostnameStr);
/* Unlock the hosts table */
        tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));

/* Add a signature longword.
 * Fill in the host entry structure. */
        *signaturePtr = TM_HOST_ENTRY_SIGNATURE;
        hostEntryPtr->h_addrtype = AF_INET;
        hostEntryPtr->h_length = sizeof(struct in_addr);
        hostEntryPtr->h_name = namePtr;
        hostEntryPtr->h_aliases = aliasArray;
        aliasArray[0] = (ttCharPtr)0;               /* No aliases */
        hostEntryPtr->h_addr_list = addrPtrArray;
        addrPtrArray[0] = (ttCharPtr)inAddrArray;   /* One address */
        inAddrArray[0].s_addr = addressPtr->addr.ipv4.sin_addr.s_addr;
        addrPtrArray[1] = (ttCharPtr)0;
    }
    else
    {
/* No entry in the hosts table.
 * Unlock the hosts table */
        tm_call_unlock(&tm_context(tvDnsHostTableLockEntry));

/* Create a hostname in the IN-ADDR.ARPA domain, appropriate for a reverse
 * DNS query.
 * Lock the DNS global variables.
 * Send a request to the DNS resolver. */
        inAddressPtr = inAddressBuf + sizeof(inAddressBuf)
                                    - sizeof(tlInAddrArpaDomain);
        tm_strcpy(inAddressPtr, tlInAddrArpaDomain);
        word = (tt32Bit)ntohl(addressPtr->addr.ipv4.sin_addr.s_addr);
        for (i = 24; i >= 0; i -= 8)
        {
            *--inAddressPtr = '.';
            byte = (int)(word >> i) & 0xFF;
            do
            {
                *--inAddressPtr = (char)('0' + (byte % 10));
                byte /= 10;
            } while (byte);
        }
        tm_call_lock_wait(&tm_context(tvDnsLockEntry));
        errorCode = tfDnsGenerateQuery( inAddressPtr,
                                        TM_DNS_QTYPE_REV,
                                        AF_INET,
                                        &dnsEntryPtr );
        if (errorCode != TM_ENOERROR || dnsEntryPtr == (ttDnsCacheEntryPtr)0)
        {
/* Unlock the DNS global variables */
            tm_call_unlock(&tm_context(tvDnsLockEntry));
            goto getHostByAddrExit;
        }

/* Copy the data from the DNS cache entry.
 * Allocate enough storage for the following:
 *  - Host entry structure.
 *  - A longword signature.
 *  - An NULL terminated array of name alias string pointers.
 *  - An NULL terminated array of IP address pointers.
 *  - An array of struct in_addr.
 *  - The hostname, including null terminator.
 */
        hostEntryPtr = (ttHostentPtr)tm_get_raw_buffer(
                        sizeof(ttHostent)
                        + sizeof(tt32Bit)
                        + 1 * sizeof(ttCharPtr)
                        + 2 * sizeof(ttCharPtr)
                        + 1 * sizeof(struct in_addr)
                        + tm_strlen(dnsEntryPtr->dnscRevHostnameStr) + 1 );
        if (hostEntryPtr == (ttHostentPtr)0)
        {
/* Unlock the DNS global variables */
            tm_call_unlock(&tm_context(tvDnsLockEntry));
            errorCode = TM_ENOMEM;
            goto getHostByAddrExit;
        }

/* Copy the hostname from the DNS cache entry. */
        signaturePtr = (tt32BitPtr)(hostEntryPtr + 1);
        aliasArray = (ttCharPtrPtr)(signaturePtr + 1);
        addrPtrArray = aliasArray + 1;
        inAddrArray = (struct in_addr TM_FAR *)(addrPtrArray + 2);
        namePtr = (ttCharPtr)(inAddrArray + 1);
        tm_strcpy(namePtr, dnsEntryPtr->dnscRevHostnameStr);
/* Unlock the DNS global variables */
        tm_call_unlock(&tm_context(tvDnsLockEntry));

/* Add a signature longword.
 * Fill in the host entry structure. */
        *signaturePtr = TM_HOST_ENTRY_SIGNATURE;
        hostEntryPtr->h_addrtype = AF_INET;
        hostEntryPtr->h_length = sizeof(struct in_addr);
        hostEntryPtr->h_name = namePtr;
        hostEntryPtr->h_aliases = aliasArray;
        aliasArray[0] = (ttCharPtr)0;               /* No aliases */
        hostEntryPtr->h_addr_list = addrPtrArray;
        addrPtrArray[0] = (ttCharPtr)inAddrArray;   /* One address */
        inAddrArray[0].s_addr = addressPtr->addr.ipv4.sin_addr.s_addr;
        addrPtrArray[1] = (ttCharPtr)0;
    }

/* Return the host entry to the caller. */
    *hostEntryPtrPtr = hostEntryPtr;

getHostByAddrExit:
    return(errorCode);
}
#endif /* TM_USE_IPV4 */

/*
 * tfFreeHostEntry
 *
 * Release the host entry structure returned by tfGetHostByName or
 * tfGetHostByAddr.
 *
 * Parameter
 * hostnameStr      Name of host to query
 * hostEntryPtrPtr  Storage for pointer to the host entry structure
 *
 * Returns
 * TM_ENOERROR      Successful.
 * TM_EPERM         Call tfDnsInit() first.
 * TM_EINVAL        Invalid host entry pointer: NULL pointer or signature
 *                  test failed.
 */
int tfFreeHostEntry(
    struct tsHostent TM_FAR *   hostEntryPtr )
{
    int     errorCode;

    errorCode = TM_ENOERROR;

/* Verify the input values. */
    if (tm_context(tvDnsInitialized) != TM_8BIT_YES)
    {
        errorCode = TM_EPERM;
    }
    else if (   hostEntryPtr == (ttHostentPtr)0
            ||  *(tt32BitPtr)(hostEntryPtr + 1) != TM_HOST_ENTRY_SIGNATURE )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_free_raw_buffer((ttRawBufferPtr)hostEntryPtr);
    }

    return(errorCode);
}
