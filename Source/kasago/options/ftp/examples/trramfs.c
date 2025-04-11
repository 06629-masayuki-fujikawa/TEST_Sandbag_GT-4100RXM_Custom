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
 * Description: Treck Ram File System
 * Filename: trramfs.c
 * Author: Emmanuel Attia & Qin Zhang
 * Date Created: 7/25/02
 * $Source: source/trramfs.c $
 *
 * Modification History
 * $Revision: 6.0.2.10 $
 * $Date: 2012/09/19 00:27:22JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_RAM_FS

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#include <trramfs.h>

/* Unique file name format */
#define TM_UNIQUE_FILENAME      "F0000000.trk"

/* Bogus file creation date and time */
#define TM_BOGUS_DATE   "01-01-70  12:00AM"

/* Column position for printing <DIR> in sub-directory directory entry */
#define TM_DIR_POS   15

/* Column position for printing size of file entry */
#define TM_SIZE_POS   24

/* Column position for printing file name */
#define TM_FILE_POS   39

/* This entry is a directory */
#define tm_fs_is_dir(fsDirPtr) (fsDirPtr->rfiUlFlags & TM_RAMFS_ATTRIB_DIR)

/* Get the file name pointer of the entry */
#define tm_fs_file_name_ptr(fsDirPtr) (fsDirPtr->rfiSzName)

/* Get the file size of the entry */
#define tm_fs_file_size(fsDirPtr) (fsDirPtr->rfiLSize)


#define ttDirEntry   ttRamfsFindInfo
typedef ttDirEntry TM_FAR * ttDirEntryPtr;

typedef struct tsUserLogin
{
    char             userName[20];
    char             userPassword[20];
} ttUserLogin;
typedef const ttUserLogin TM_FAR * ttUserLoginPtr;

/* Define TM_FS_WORKINGDIR_SIZE so that sizeof ttUserFsData is 256 */
#define TM_FS_WORKINGDIR_SIZE        \
            (TM_RAMFS_BLOCKSIZE * 2) \
          - (3 * sizeof(ttVoidPtr))  \
          - sizeof(int)              \
          - sizeof(ttDirEntry)

#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */
typedef struct tsUserFsData
{
    ttUserLoginPtr   userLoginPtr;
    ttVoidPtr        userDirPtr;
    ttVoidPtr        userDirHandle;
/* Directory entry */
    ttDirEntry       userDirEntry;
/* Long or short directory open */
    int              userDirFlag;
/*
 * Note could add userErrorCode field where tlLastError could be stored
 * before unlocking to make the code completely re-entrant in which
 * case TM_FS_WORKINGDIR_SIZE need to be decreased by sizeof(int).
 *   int              userErrorCode;
 */
    char             userWorkingDirectory[TM_FS_WORKINGDIR_SIZE];
} ttUserFsData;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */

typedef ttUserFsData TM_FAR * ttUserFsDataPtr;

typedef const ttUserFsData TM_FAR * ttConstUserFsDataPtr;

/*
 * Function prototypes
 */
static void tfPrintDecimal(ttCharPtr bufPtr, int bufLen, tt32Bit number);

/*
 * Local variables
 */

static const ttUserLogin  TM_CONST_QLF tlFsNamePasswd[] =
{
/*   username,      password    */
    {"treck",       "treck"     },
    {"elmic",       "elmic"     }
};

/*
 * Note: with a DOS OS we need to open the files in binary mode, that
 * way there is no translation from CR,LF to LF on input, and from LF
 * to CR,LF on output. We can keep the network CR,LF and store that
 * in the file. Since we are opening the files in binary mode, then
 * we will have to explicitely write CR,LF for end of line ourselves.
 */
static const char  TM_CONST_QLF tlFsOpenMap[TM_FS_APPEND+1][4] =
                                    { "rb", "wb", "ab"};

/*
 * local functions
 */
static void tfFSUserDir(ttConstUserFsDataPtr userFsDataPtr);


/* tm_ramfs_drivesize
 *
 *    Gets the drive size.
 *
 *    Function Equivalent
 *        ttRamfsSize tm_ramfs_drivesize(ttRamfsBlock blCount);
 *
 *    Parameters
 *        blCount : block count of the drive
 *
 *    Return Values
 *        The drive size
 */
#define tm_ramfs_drivesize(bl)   \
    ((ttRamfsSize)sizeof(ttRamfsDrive) + \
    (ttRamfsSize)((ttRamfsSize)(bl - 1) * TM_RAMFS_RAWBLOCKSIZE))

/* tm_ramfs_getaddress
 *
 *    Gets the block address of an entry.
 *
 *    Function Equivalent
 *        ttRamfsBlock tm_ramfs_getaddress(ttRamfsDrivePtr pDrive,
 *                                         ttRamfsEntryPtr pEntry);
 *
 *    Parameters
 *        pDrive : pointer of the drive structure
 *        pEntry : pointer of the entry
 *
 *    Return Values
 *        The block address of the entry
 */
#define tm_ramfs_getaddress(r,p) \
    (ttRamfsBlock)(   (   (ttRamfsSize)(   (ttUserPtrCastToInt)(p) \
                                         - (ttUserPtrCastToInt)(r->rdRawBlock)) \
                        - (ttRamfsSize)(2 * sizeof(ttRamfsBlock)) ) \
                    / ((ttRamfsSize)sizeof(ttRamfsRawBlock)) )

/* tm_ramfs_getentry
 *
 *    Gets the pointer of an entry by its block address.
 *
 *    Function Equivalent
 *        ttRamfsEntryPtr tm_ramfs_getentry(ttRamfsDrivePtr pDrive,
 *                                            ttRamfsBlock blEntry);
 *
 *    Parameters
 *        pDrive : pointer of the drive structure
 *        blEntry : the entry's block address
 *
 *    Return Values
 *        The pointer of the entry
 */
#define tm_ramfs_getentry(r,b)        ((ttRamfsEntryPtr) \
                                    &(r->rdRawBlock[b].rrbEntry))

/* tm_ramfs_nextblock
 *
 *    Gets the next block linked with a block.
 *
 *    Function Equivalent
 *        ttRamfsBlock tm_ramfs_nextblock(    ttRamfsDrivePtr pDrive,
 *                                            ttRamfsBlock blBlock);
 *
 *    Parameters
 *        pDrive : pointer of the drive structure
 *        blBlock : the block address
 *
 *    Return Values
 *        The next block linked with the block
 */
#define tm_ramfs_nextblock(r,b) \
            (((ttRamfsRawBlockPtr)((r)->rdRawBlock+(b)))->rrbBlNext)

/* tm_ramfs_backblock
 *
 *    Gets the back block linked with a block.
 *
 *    Function Equivalent
 *        ttRamfsBlock tm_ramfs_backblock(    ttRamfsDrivePtr pDrive,
 *                                            ttRamfsBlock blBlock);
 *
 *    Parameters
 *        pDrive : pointer of the drive structure
 *        blBlock : the block address
 *
 *    Return Values
 *        The back block linked with the block
 */
#define tm_ramfs_backblock(r,b) \
            (((ttRamfsRawBlockPtr)((r)->rdRawBlock)+(b))->rrbBlBack)

#define tm_ramfs_fhandle(x)    ((ttRamfsFileHandlePtr)x)
#define tm_ramfs_getfhentry(ramfsPtr, x) tm_ramfs_getentry(ramfsPtr, \
            tm_ramfs_fhandle(x)->rfhBlEntry)

/*
 * Treck Ram File System File Handling Functions
 */
static int tfRamfsAddFileHandle(ttVoidPtr pHandle);
static int tfRamfsRemoveFileHandle(ttConstVoidPtr pHandle);
static int tfRamfsIsValidFileHandle(ttConstVoidPtr pHandle);
static int tfRamfsFindFileHandle(ttRamfsBlock blEntry, ttVoidPtrPtr pHandle);
static ttVoidPtr tfRamfsOpenDir(ttUserFsDataPtr     userFsDataPtr,
                                const char TM_FAR * pathNamePtr,
                                int                 flag);

/*
 * Treck Ram File System Low Level Functions
 */
static ttRamfsSize tfRamfsGetSizeFromBlock(ttRamfsBlock blCount,
                                      ttRamfsBlock blLastBlockSize);
static void tfRamfsGetEntryInfo( ttRamfsDrivePtr    ramfsPtr,
                                 ttRamfsFindInfoPtr pFindInfo,
                                 ttRamfsBlock       blEntry);
static ttVoidPtr tfRamfsFindFirstBlock( ttRamfsDrivePtr ramfsPtr,
                                        ttConstCharPtr  lpszFileName,
                                        ttRamfsBlockPtr pblFoundEntry);
static int tfRamfsFindNextBlock(ttRamfsDrivePtr ramfsPtr,
                                ttVoidPtr hFind,
                                ttRamfsBlockPtr pblFoundEntry);
static int tfRamfsIsValidFileName(ttConstCharPtr lpszFileName, int nDirectory);
static int tfRamfsSplitPath(ttRamfsDrivePtr ramfsPtr,
                            ttConstCharPtr lpszPathName,
                            ttConstCharPtr TM_FAR * lpszFileName,
                            ttRamfsBlockPtr pblDirEntry);
static int tfRamfsAllocBlock(ttRamfsDrivePtr ramfsPtr,
                             ttRamfsBlock blCount, ttRamfsEntryPtr pNewEntry);
static int tfRamfsFreeBlock( ttRamfsDrivePtr ramfsPtr,
                             ttRamfsBlock blAddress, ttRamfsEntryPtr pEntry);
static int tfRamfsFreeEntryBlocks( ttRamfsDrivePtr ramfsPtr,
                                   ttRamfsEntryPtr pEntry,
                                   ttRamfsBlock blCount );
static int tfRamfsLockedClose(ttVoidPtr pHandle);

/* Local Variable Definitions */
int             tlLastError;
ttVoidPtr       tlFileHandles[TM_RAMFS_MAXFILES];


/*
 * Treck Ram File System File Handling Functions
 */
/* tfRamfsAddFileHandle
 *
 *    Adds a file handle to the handle list.
 *
 *    Parameters
 *        pHandle : Handle to add
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                           details)
 *        TM_RAMFS_OKAY  : Handle successfully added
 */
static int tfRamfsAddFileHandle(ttVoidPtr pHandle)
{
    ttVoidPtrPtr    pHandleList;
    int             retCode;
    int             counter;

    tlLastError = TM_EMFILE;
    retCode = TM_RAMFS_ERROR;
    pHandleList = tlFileHandles;

/* Parse the whole handle list and look for a free entry */
    for(counter = 0; counter < TM_RAMFS_MAXFILES; counter++)
    {
        if((*pHandleList) == TM_VOID_NULL_PTR)
        {
            tlLastError = TM_ENOERROR;
            retCode = TM_RAMFS_OKAY;
/* Put the handle in the free entry */
            *pHandleList = pHandle;
            break;
        }
        else
        {
            pHandleList++;
        }
    }

    return retCode;
}


/*
 * tfRamfsRemoveFileHandle :
 *    Removes a file handle to the handle list.
 *
 *    Parameters
 *        pHandle : Handle to add
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                           details)
 *        TM_RAMFS_OKAY  : Handle successfully added
 */
static int tfRamfsRemoveFileHandle(ttConstVoidPtr pHandle)
{
    ttVoidPtrPtr    pHandleList;
    int             retCode;
    int             counter;

    tlLastError = TM_EMFILE;
    retCode = TM_RAMFS_ERROR;
    pHandleList = tlFileHandles;

/* Parse the whole handle list and look for a free entry */
    for(counter = 0; counter < TM_RAMFS_MAXFILES; counter++)
    {
        if((ttConstVoidPtr)(*pHandleList) == pHandle)
        {
            tlLastError = TM_ENOERROR;
            retCode = TM_RAMFS_OKAY;
/* Put the handle in the free entry */
            *pHandleList = TM_VOID_NULL_PTR;
            break;
        }
        else
        {
            pHandleList++;
        }
    }

    return retCode;
}


/*
 * tfRamfsIsValidFileHandle
 *    Checks the validity of a file handle.
 *
 *    Parameters
 *        pHandle : File handle
 *
 *    Return Values
 *        TM_RAMFS_ERROR : Invalid handle
 *        TM_RAMFS_OKAY  : Valid handle
 */
static int tfRamfsIsValidFileHandle(ttConstVoidPtr pHandle)
{
    ttVoidPtrPtr    pHandleList;
    int             retCode;

    retCode = TM_RAMFS_OKAY;
    pHandleList = tlFileHandles;

    if (pHandle == (ttConstVoidPtr)0)
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        /* Parse for the whole handle list and look for the asked handle */
        while ((ttConstVoidPtr)pHandleList[0] != pHandle)
        {
            pHandleList++;

            if (pHandleList == (tlFileHandles + TM_RAMFS_MAXFILES))
            {
                /* Handle not found in the list */
                retCode = TM_RAMFS_ERROR;
                break;
            }
        }
    }

    return retCode;
}


/*
 * tfRamfsFindFileHandle
 *    Finds files handle of a specified entry.
 *
 *    Parameters
 *        blEntry : Block address of the entry
 *        pHandle : Point to the returned handle (should point to NULL
 *            in order to start the search from the beginning)
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured or no more handle matches the
 *                       entry(call tfRamfsErrNo for more details)
 *        TM_RAMFS_OKAY  : Handle found
 */
static int tfRamfsFindFileHandle(ttRamfsBlock blEntry, ttVoidPtrPtr pHandle)
{
    ttVoidPtrPtr pHandleList;
    int          retCode;

    tlLastError = TM_ENOERROR;
    retCode = TM_RAMFS_OKAY;
    pHandleList = tlFileHandles;

    if ((pHandle == TM_VOID_NULL_PTR) || (blEntry == TM_RAMFS_NOLINK))
    {
/* Invalid pointer or invalid entry block address */
        tlLastError = TM_EINVAL;
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        if (pHandle[0] != TM_VOID_NULL_PTR)
        {
/* The search start at handle pHandle[0] so we look for it */
            while (pHandleList[0] != pHandle[0])
            {
                pHandleList++;

                if (pHandleList == (tlFileHandles + TM_RAMFS_MAXFILES))
                {
/* Handle not found */
                    tlLastError = TM_EINVAL;
                    retCode = TM_RAMFS_ERROR;
                    break;
                }
            }
        }

        if (tlLastError == TM_ENOERROR)
        {
            for (;;)
            {
/* Parse each handle */
                if (pHandleList[0] != TM_VOID_NULL_PTR)
                {
                    if (tm_ramfs_fhandle(pHandleList[0])->rfhBlEntry == blEntry)
                    {
/* Handle found */
                        break;
                    }
                }

                pHandleList++;

                if (pHandleList == (tlFileHandles + TM_RAMFS_MAXFILES))
                {
/* Handle not found */
                    tlLastError = TM_EINVAL;
                    retCode = TM_RAMFS_ERROR;
                    break;
                }
            }

            if (pHandleList < (tlFileHandles + TM_RAMFS_MAXFILES))
            {
/* Put the found handle in the handle pointer */
                *pHandle = *pHandleList;
            }
        }
    }

    return retCode;
}


/*
 * tfRamfsOpen
 *    Opens or creates a file.
 *
 *    Parameters
 *        lpszFileName : Filename to open
 *        ulFlags : Combination of following file access flags, open mode,
 *                share mode and file type
 *            TM_RAMFS_ACCESS_READ : Open the file for reading
 *            TM_RAMFS_ACCESS_WRITE : Open the file for writing
 *            TM_RAMFS_OPEN_CREATE : Create a new file if it doesn't exist
 *            TM_RAMFS_OPEN_EXCL : The file mustn't exist
 *            TM_RAMFS_OPEN_TRUNC : Truncate the file to zero length
 *                                  when opening
 *            TM_RAMFS_OPEN_APPEND : If the file exist, set the current
 *                                position to the EOF.
 *            TM_RAMFS_SHARE_READ : Allow shared reading
 *            TM_RAMFS_SHARE_WRITE : Allow shared writing
 *            TM_RAMFS_TYPE_BINARY : Open the file in binary mode
 *            TM_RAMFS_TYPE_TEXT : Open the file in text mode (obsolete)
 *
 *    Return Values
 *        NULL : An error as occured (call tfRamfsErrNo for more details)
 *        Other values : File handle
 */
ttVoidPtr tfRamfsOpen(ttConstCharPtr lpszFileName, tt32Bit ulFlags)
{
    ttRamfsBlock            blEntry, blBaseDir;
    ttRamfsFileHandlePtr    pHandle, pFindHandle;
    ttRamfsEntryPtr         pBaseDir, pEntry;
    ttConstCharPtr          lpszName;
    ttRamfsDrivePtr         ramfsPtr;
    ttVoidPtr               hFind;
    int                     nExist;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        pHandle = (ttRamfsFileHandlePtr)0;
        tlLastError = TM_ENOENT;
    }
    else
    {
        nExist = 0;
        tlLastError = TM_ENOERROR;
        pHandle = tm_malloc(sizeof(ttRamfsFileHandle));

        if (pHandle == TM_VOID_NULL_PTR)
        {
/* Out of memory */
            tlLastError = TM_ENOBUFS;
        }
        else
        {
            hFind = tfRamfsFindFirstBlock(ramfsPtr, lpszFileName, &blEntry);
            if (hFind != (ttVoidPtr)0)
            {
/* Opened file exists */
                nExist = 1;

                if (tm_ramfs_getentry(ramfsPtr, blEntry)->reUlFlags
                    & TM_RAMFS_ATTRIB_DIR)
                {
/* Is a directory */
                    if (ulFlags & (TM_RAMFS_OPEN_CREATE | TM_RAMFS_ACCESS_WRITE
                        | TM_RAMFS_OPEN_APPEND | TM_RAMFS_OPEN_TRUNC))
                    {
/* We can't create, write or truncate a directory */
                        tlLastError = TM_EISDIR;
                    }
                }

                if (ulFlags & TM_RAMFS_OPEN_CREATE)
                {
                    ulFlags ^= TM_RAMFS_OPEN_CREATE;
                }
                else if (ulFlags & TM_RAMFS_OPEN_EXCL)
                {
                    ulFlags ^= TM_RAMFS_OPEN_EXCL;
                }
/* End the search */
                (void)tfRamfsFindClose(hFind);
            }

            if ((nExist == 0)
                && ((ulFlags & TM_RAMFS_OPEN_CREATE) == 0))
            {
/* File doesn't exist and no create flags specified */
                tlLastError = TM_ENOENT;
            }
            else if ((nExist == 1) && (ulFlags & TM_RAMFS_OPEN_EXCL))
            {
/* File exist */
                tlLastError = TM_EEXIST;
            }
            else if (tfRamfsSplitPath(ramfsPtr, lpszFileName, &lpszName,
                                      &blBaseDir) == 0)
            {
                pBaseDir = tm_ramfs_getentry(ramfsPtr, blBaseDir);
                if (tfRamfsIsValidFileName(lpszName, 1) == 0)
                {
                    if (nExist == 0)
                    {
/*
 * File doesn't exist
 * Alloc one block for the entry in the base dir
 */
                        if (tfRamfsAllocBlock(ramfsPtr, 1, pBaseDir) == 0)
                        {
/* Get the block address of the entry */
                            blEntry = pBaseDir->reBlLastBlock;
                        }
                        else
                        {
/* Out of space */
                            blEntry = TM_RAMFS_NOLINK;
                        }
                    }

                    if (blEntry != TM_RAMFS_NOLINK)
                    {
                        pFindHandle = TM_VOID_NULL_PTR;

                        if (tfRamfsFindFileHandle(
                                blEntry, (ttVoidPtrPtr)(ttVoidPtr)(&pFindHandle)) == 0)
                        {
/* Other handles open the same file */
                            if (ulFlags & TM_RAMFS_ACCESS_READ)
                            {
/* Read flag */
                                if (   ( (  pFindHandle->rfhUlFlags
                                           & TM_RAMFS_SHARE_READ ) == 0 )
                                    || ((ulFlags & TM_RAMFS_SHARE_READ) == 0) )
                                {
/* No share read permission */
                                    tlLastError = TM_EACCES;
                                }
                            }

                            if (ulFlags & TM_RAMFS_ACCESS_WRITE)
                            {
/* Write flag */
                                if (   ( (  pFindHandle->rfhUlFlags
                                           & TM_RAMFS_SHARE_WRITE ) == 0 )
                                    || ((ulFlags & TM_RAMFS_SHARE_WRITE) == 0) )
                                {
/* No share write permission */
                                    tlLastError = TM_EACCES;
                                }
                            }
                            tlLastError = TM_EACCES;
                        }

                        if (tlLastError != TM_EACCES)
                        {
/* Clear the TM_EINVAL error */
                            tlLastError = TM_ENOERROR;
                        }

                        if (tlLastError == TM_ENOERROR)
                        {
                            pEntry = tm_ramfs_getentry(ramfsPtr, blEntry);

                            if ((pEntry->reUlFlags & TM_RAMFS_ATTRIB_DIR)
                                && (ulFlags & TM_RAMFS_ACCESS_WRITE))
                            {
/* Can't open a directory for writing */
                                tlLastError = TM_EACCES;
                            }
                            else
                            {
                                (void)tfRamfsAddFileHandle(pHandle);
/* Fill the handle structure */
                                pHandle->rfhUlFlags = ulFlags &
                                                    (  TM_RAMFS_ACCESS_READ
                                                     | TM_RAMFS_SHARE_READ
                                                     | TM_RAMFS_ACCESS_WRITE
                                                     | TM_RAMFS_SHARE_WRITE
                                                     | TM_RAMFS_TYPE_BINARY
                                                     | TM_RAMFS_TYPE_TEXT);
                                pHandle->rfhBlEntry = blEntry;
                                pHandle->rfhLPosition = 0;

                                if (nExist == 0)
                                {
/* File didn't exist, fill the entry structure */
                                    pEntry->reBlAddress = TM_RAMFS_NOLINK;
                                    tm_strcpy(pEntry->reSzName, lpszName);
                                }
                                else if (ulFlags & TM_RAMFS_OPEN_TRUNC)
                                {
/* Truncate the file */
                                    (void)tfRamfsSetEof(pHandle);
                                }
                                else if(ulFlags & TM_RAMFS_OPEN_APPEND)
                                {
/* Go to the end of file */
                                    tm_ramfs_fhandle(pHandle)->rfhLPosition
                                                = tfRamfsFileLength(pHandle);
                                }
                            }
                        }
                    }
                }
                else
                {
                    tlLastError = TM_EINVAL;
                }
            }
            else
            {
                tlLastError = TM_ENOENT;
            }
        }

        if ((tlLastError != TM_ENOERROR) && (pHandle != TM_VOID_NULL_PTR))
        {
            tm_free(pHandle);
            pHandle = (ttRamfsFileHandlePtr)0;
        }
    }

    return (ttVoidPtr)pHandle;
}


/*
 * tfRamfsFlush
 *    Flush a file.
 *
 *    Parameters
 *        pHandle : Handle of the file to Flush
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured or no more handle matches the
 *                       entry(call tfRamfsErrNo for more details)
 *        TM_RAMFS_OKAY  : Handle found
 */
int tfRamfsFlush(ttVoidPtr pHandle)
{
    TM_UNREF_IN_ARG(pHandle);
    return TM_RAMFS_OKAY;
}

/*
 * tfRamfsClose
 *    Closes a file.
 *
 *    Parameters
 *        pHandle : Handle of the file to close
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured or no more handle matches the
 *                       entry(call tfRamfsErrNo for more details)
 *        TM_RAMFS_OKAY  : Handle found
 */
int tfRamfsClose(ttVoidPtr pHandle)
{
    int retCode;

    tlLastError = TM_ENOERROR;

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    retCode = tfRamfsLockedClose(pHandle);

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
}

/*
 * tfRamfsCloseAll
 *    Closes each opened file.
 */
void tfRamfsCloseAll()
{
    int i;

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

/* Parse the handle list */
    for (i = 0; i < TM_RAMFS_MAXFILES; i++)
    {
        if (tlFileHandles[i] != TM_VOID_NULL_PTR)
        {
/* If entry isn't free then close the handle from the list */
            (void)tfRamfsLockedClose(tlFileHandles[i]);
        }
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return;
}

/*
 * tfRamfsLockedClose
 *
 * CALLED WITH THE LOCK ON.
 */
static int tfRamfsLockedClose(ttVoidPtr pHandle)
{
    int retCode;

    retCode = tfRamfsIsValidFileHandle(pHandle);
    if (retCode == TM_RAMFS_ERROR)
    {
/* Invalid handle */
        retCode = TM_EBADF;
    }
    else
    {
/* Remove the handle from the list */
        (void)tfRamfsRemoveFileHandle(pHandle);
        tm_free(pHandle);
        retCode = TM_RAMFS_OKAY;
    }

    return retCode;
}

/*
 * tfRamfsSeek
 *    Moves a file to the specified location.
 *
 *    Parameters
 *        pHandle : Handle of the opened file
 *        sizeOffset : Number of bytes from nOrigin
 *        nOrigin : Initial position
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured or no more handle matches the
 *                       entry(call tfRamfsErrNo for more details)
 *        Other value    : New position of the file
 */
ttRamfsSize tfRamfsSeek(ttVoidPtr pHandle,
                            ttRamfsSize lOffset,
                            int nOrigin)
{
    ttRamfsSize     lSize;
    ttRamfsSize     position;
    ttRamfsDrivePtr ramfsPtr;

    position = TM_RAMFS_ERROR;
    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
/* Get the actual file size */
        lSize = tfRamfsGetSizeFromBlock(
            tm_ramfs_getfhentry(ramfsPtr, pHandle)->reBlCount,
            tm_ramfs_getfhentry(ramfsPtr, pHandle)->reBlLastBlockSize);

        tlLastError = TM_ENOERROR;

        if (tfRamfsIsValidFileHandle(pHandle) == 0)
        {
            switch (nOrigin)
            {
            case TM_RAMFS_SEEK_SET:
/* Seek from begin */
                if ((lOffset < 0) || (lOffset > lSize))
                {
/* Invalid offset */
                    tlLastError = TM_EINVAL;
                }
                else
                {
/* Position is offset */
                    tm_ramfs_fhandle(pHandle)->rfhLPosition = lOffset;
                }
                break;
            case TM_RAMFS_SEEK_CUR:
/* Seek from current */
                if (((tm_ramfs_fhandle(pHandle)->rfhLPosition + lOffset) < 0)
                    || ((tm_ramfs_fhandle(pHandle)->rfhLPosition + lOffset)
                    >= lSize))
                {
/* Invalid offset */
                    tlLastError = TM_EINVAL;
                }
                else
                {
/* Add offset to the current position */
                    tm_ramfs_fhandle(pHandle)->rfhLPosition += lOffset;
                }
                break;
            case TM_RAMFS_SEEK_END:
/* Seek from end */
                if ((lOffset > 0) || (lSize + lOffset > 0))
                {
/* Invalid offset */
                    tlLastError = TM_EINVAL;
                }
                else
                {
/*
 * Position is the size plus the offset therefore the offset
 * has to be negative
 */
                    tm_ramfs_fhandle(pHandle)->rfhLPosition = lSize + lOffset;
                }
                break;
            default:
                break;
            }
        }
        else
        {
            tlLastError = TM_EBADF;
        }

        if (tlLastError == TM_ENOERROR)
        {
/* Return the current position */
            position = tm_ramfs_fhandle(pHandle)->rfhLPosition;
        }
    }

    return position;
}


/*
 * tfRamfsTell
 *    Retreives the position of a file.
 *
 *    Parameters
 *        pHandle : Handle of the opened file
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        Other value    : Current position of the file
 */
ttRamfsSize tfRamfsTell(ttConstVoidPtr pHandle)
{
    ttRamfsSize position;

    if (tfRamfsIsValidFileHandle(pHandle) == 0)
    {
/* Return the current position */
        tlLastError = TM_ENOERROR;
        position = tm_ramfs_fhandle(pHandle)->rfhLPosition;
    }
    else
    {
/* Invalid handle */
        tlLastError = TM_EBADF;
        position = TM_RAMFS_ERROR;
    }
    return position;
}


/*
 * tfRamfsFileLength
 *    Get the length of a file.
 *
 *    Parameters
 *        pHandle : Handle of the opened file
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        Other value : Length of the file
 */
ttRamfsSize tfRamfsFileLength(ttConstVoidPtr pHandle)
{
    ttRamfsSize     length;
    ttRamfsDrivePtr ramfsPtr;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    length = TM_RAMFS_ERROR;
    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        if (tfRamfsIsValidFileHandle(pHandle) == 0)
        {
/* Return the file length */
            tlLastError = TM_ENOERROR;
            length = tfRamfsGetSizeFromBlock(
                    tm_ramfs_getfhentry(ramfsPtr, pHandle)->reBlCount,
                    tm_ramfs_getfhentry(ramfsPtr, pHandle)->reBlLastBlockSize);
        }
        else
        {
/* Invalid handle */
            tlLastError = TM_EBADF;
        }
    }
    return length;
}


/* tfRamfsEof
 *
 *    Tests for end of file.
 *
 *    Parameters
 *        pHandle : Handle of the opened file
 *
 *    Return Values
 *        TM_RAMFS_ERROR : Not end of file or error occured
 *        TM_RAMFS_OKAY  : End of file
 */
int tfRamfsEof(ttConstVoidPtr pHandle)
{
    int retCode;

    tlLastError = TM_ENOERROR;

    if (tfRamfsIsValidFileHandle(pHandle) == TM_RAMFS_OKAY)
    {
        if (tm_ramfs_fhandle(pHandle)->rfhLPosition
            == tfRamfsFileLength(pHandle))
        {
/* End of file */
            retCode = TM_RAMFS_OKAY;
        }
        else
        {
            retCode = TM_RAMFS_ERROR;
        }
    }
    else
    {
/* Invalid handle */
        tlLastError = TM_EBADF;
        retCode = TM_RAMFS_ERROR;
    }

    return retCode;
}


/*
 * tfRamfsSetEof
 *    Sets the end of a file to its current position
 *
 *    Parameters
 *        pHandle : Handle of the opened file
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : End of file is successfully set
 */
int tfRamfsSetEof(ttConstVoidPtr pHandle)
{
    ttRamfsSize     lNewSize;
    ttRamfsBlock    blNewCount, blNewLastBlockSize, blToFree;
    ttRamfsEntryPtr pEntry;
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    retCode = TM_RAMFS_ERROR;

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        if (tfRamfsIsValidFileHandle(pHandle) == 0)
        {
            tlLastError = TM_ENOERROR;
            retCode = TM_RAMFS_OKAY;
/* Get the new size of the file and the new block count */
            lNewSize = tm_ramfs_fhandle(pHandle)->rfhLPosition;
            blNewCount = (ttRamfsBlock)(lNewSize / TM_RAMFS_BLOCKSIZE);
            pEntry = tm_ramfs_getfhentry(ramfsPtr, pHandle);

            blNewLastBlockSize =
                    (ttRamfsBlock)((tt32Bit)lNewSize % TM_RAMFS_BLOCKSIZE);
            if (blNewLastBlockSize)
            {
                blNewCount++;
            }

/* Count how many blocks to free */
            blToFree = (ttRamfsBlock)(pEntry->reBlCount - blNewCount);
            if (blToFree != 0)
            {
/* Descrease the file size */
                (void)tfRamfsFreeEntryBlocks(ramfsPtr, pEntry, blToFree);
            }
            pEntry->reBlLastBlockSize = blNewLastBlockSize;
        }
        else
        {
/* Invalid handle */
            tlLastError = TM_EBADF;
        }
    }

    return retCode;
}


/*
 * tfRamfsCommit
 *    Flush file to disk. (Obsolete)
 *
 *    Parameters
 *        pHandle : Handle of the opened file
 *
 *    Return Values
 *        0
 */
int tfRamfsCommit(ttVoidPtr pHandle)
{
/* Obsolete function in this type of file system */
    TM_UNREF_IN_ARG(pHandle);
    return 0;
}


/*
 * tfRamfsRead
 *    Read data from a file.
 *
 *    Parameters
 *        pHandle : Handle of the opened file
 *        pBuffer : Buffer where to put the data
 *        lCount : Maximum number of bytes to read
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        Other value : Number of bytes read
 */
ttRamfsSize tfRamfsRead( ttVoidPtr pHandle, ttVoidPtr pBuffer,
                         ttRamfsSize lCount)
{
    ttRamfsSize     lPos, lSize, lRead, lCurRead, lNum;
    ttRamfsBlock    blAddress;
    ttRamfsDrivePtr ramfsPtr;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        lRead  = TM_RAMFS_ERROR;
        tlLastError = TM_ENOENT;
    }
    else
    {
        lRead  = 0;
        tlLastError = TM_ENOERROR;
        if (tfRamfsIsValidFileHandle(pHandle) == 0)
        {
            if (tm_ramfs_fhandle(pHandle)->rfhUlFlags & TM_RAMFS_ACCESS_READ)
            {
                blAddress = tm_ramfs_getfhentry(ramfsPtr, pHandle)->reBlAddress;

                if (blAddress != TM_RAMFS_NOLINK)
                {
/* File is not empty Get actual position and size */
                    lPos = tfRamfsTell(pHandle);
                    lSize = tfRamfsFileLength(pHandle);

                    if (lCount + lPos > lSize)
                    {
/* Too many bytes many to read, reduce the byte count */
                        lCount = lSize - lPos;
                    }

/* Get the starting reading address */
                    for (lNum = lPos; lNum >= TM_RAMFS_BLOCKSIZE;
                                                lNum -= TM_RAMFS_BLOCKSIZE)
                    {
                        blAddress = tm_ramfs_nextblock(ramfsPtr, blAddress);
                    }

                    while (lRead < lCount)
                    {
/* Current reading size */
                        lCurRead = TM_RAMFS_BLOCKSIZE -
                            ((lPos + lRead) % TM_RAMFS_BLOCKSIZE);

                        if ((lRead + lCurRead) > lCount)
                        {
                            lCurRead = lCount - lRead;
                        }

/* Copy block content to the buffer */
                        tm_memcpy(((char *)pBuffer) + ((ttUser32Bit)lRead),
                            ramfsPtr->rdRawBlock[blAddress].rrbEntry.reBlock
                            + ((ttUser32Bit)(lPos + lRead)
                               % TM_RAMFS_BLOCKSIZE), lCurRead);
/* Next block to read */
                        blAddress = tm_ramfs_nextblock(ramfsPtr, blAddress);
                        lRead += lCurRead;
                    }

                    tm_ramfs_fhandle(pHandle)->rfhLPosition += lRead;
                }
                else
                {
                    tlLastError = TM_EBADF;
                }
            }
            else
            {
/* No permission for writing */
                tlLastError = TM_EACCES;
            }
        }
        else
        {
            tlLastError = TM_EBADF;
        }
    }

    if (tlLastError != TM_ENOERROR)
    {
        lRead = TM_RAMFS_ERROR;
    }

    return lRead;
}


/* tfRamfsWrite
 *
 *    Writes data to a file.
 *
 *    Parameters
 *        pHandle : Handle of the opened file
 *        pBuffer : Buffer where to get the data
 *        lCount : Maximum number of bytes to write
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        Other value : Number of bytes written
 */
ttRamfsSize tfRamfsWrite(ttVoidPtr pHandle,
                           ttConstVoidPtr pBuffer,
                           ttRamfsSize lCount)
{
    ttRamfsSize     lPos, lSize, lWritten, lCurWritten, lGrow, lNum;
    ttRamfsBlock    blAddress, blGrow, blLastBlockSize;
    ttRamfsDrivePtr ramfsPtr;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        lWritten = TM_RAMFS_ERROR;
        tlLastError = TM_ENOENT;
    }
    else
    {
        tlLastError  = TM_ENOERROR;
        lWritten    = 0;
        if (tfRamfsIsValidFileHandle(pHandle) == 0)
        {
/* Check for the write permission */
            if (tm_ramfs_fhandle(pHandle)->rfhUlFlags & TM_RAMFS_ACCESS_WRITE)
            {
                blAddress = tm_ramfs_getfhentry(ramfsPtr, pHandle)->reBlAddress;
                blLastBlockSize =
                        tm_ramfs_getfhentry(ramfsPtr, pHandle)->reBlLastBlockSize;

/* Get the acutal position and size of the file */
                lPos = tfRamfsTell(pHandle);
                lSize = tfRamfsFileLength(pHandle);
                lWritten = 0;
                lNum = 0;

                if (blAddress != TM_RAMFS_NOLINK)
                {
/* File is not empty get the block address of the file position */
                    for (lNum = lPos; lNum > TM_RAMFS_BLOCKSIZE;
                                                lNum -= TM_RAMFS_BLOCKSIZE)
                    {
                        blAddress = tm_ramfs_nextblock(ramfsPtr, blAddress);
                    }
                }

/* File must be grown by lGrow bytes */
                lGrow = lCount - (lSize - lPos);

                if (lGrow > 0)
                {
                    blGrow = (ttRamfsBlock)(lGrow / TM_RAMFS_BLOCKSIZE);

                    if (blLastBlockSize != 0)
                    {
                        if ((lGrow % TM_RAMFS_BLOCKSIZE) + blLastBlockSize >
                            TM_RAMFS_BLOCKSIZE)
                        {
                            blGrow++;
                        }
                    }
                    else if (lGrow % TM_RAMFS_BLOCKSIZE)
                    {
                        blGrow++;
                    }

                    lSize += lGrow;
                    blLastBlockSize =
                    (ttRamfsBlock)((ttUser32Bit)lSize % TM_RAMFS_BLOCKSIZE);

                    tm_ramfs_getfhentry(ramfsPtr, pHandle)->reBlLastBlockSize =
                                                             blLastBlockSize;

/* Allocate new blocks to grow the file size, if we need to */
                    if (blGrow != 0)
                    {
                        (void)tfRamfsAllocBlock(ramfsPtr,
                                                blGrow,
                                                tm_ramfs_getfhentry(
                                                    ramfsPtr, pHandle));
                    }

                    if (lNum == TM_RAMFS_BLOCKSIZE)
/* At end of current block */
                    {
                        blAddress = tm_ramfs_nextblock(ramfsPtr, blAddress);
                    }
                }

                if (blAddress == TM_RAMFS_NOLINK)
                {
/* File was empty, so we get its address */
                    blAddress =
                            tm_ramfs_getfhentry(ramfsPtr, pHandle)->reBlAddress;
                }

                if (tlLastError == TM_ENOERROR)
                {
                    while (lWritten < lCount)
                    {
/* Bytes to read in the current block */
                        lCurWritten = TM_RAMFS_BLOCKSIZE -
                            ((lPos + lWritten) % TM_RAMFS_BLOCKSIZE);

                        if ((lWritten + lCurWritten) > lCount)
                        {
                            lCurWritten = lCount - lWritten;
                        }

/* Copy block content to the buffer */
                        tm_memcpy(ramfsPtr->rdRawBlock[blAddress].rrbEntry.reBlock
                            + ((lPos + lWritten) % TM_RAMFS_BLOCKSIZE),
                            ((char *)pBuffer) + ((ttUser32Bit)lWritten),
                            lCurWritten);
/* Get the next block */
                        blAddress = tm_ramfs_nextblock(ramfsPtr, blAddress);
                        lWritten += lCurWritten;
                    }

/* Increase the file's position */
                    tm_ramfs_fhandle(pHandle)->rfhLPosition += lWritten;
                }
            }
            else
            {
/* No write access */
                tlLastError = TM_EACCES;
            }
        }
        else
        {
/* Bad file handle */
            tlLastError = TM_EBADF;
        }
    }

    if (tlLastError != TM_ENOERROR)
    {
        lWritten = TM_RAMFS_ERROR;
    }

    return lWritten;
}


/*
 * Treck Ram File System User Interface Functions
 */
/* tfRamfsErrNo
 *
 *    Gets the last error number.
 *
 *    Return Values
 *        The last error code (see Trsocket.h for definition)
 */
int tfRamfsErrNo()
{
    return tlLastError;
}


/* tfRamfsInit
 *
 *    Mounts the drive.
 *
 *    Parameters
 *        blCount : Drive size in blocks
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : The operation was successful
 */
int tfRamfsInit(ttRamfsBlock blCount)
{
    ttRamfsBlock    blNum;
    int             retCode;
    ttRamfsSize     mallocSize;
    ttRamfsDrivePtr ramfsPtr;

    tlLastError = TM_ENOERROR;
    retCode = TM_RAMFS_OKAY;

    if (blCount == 0)
    {
        tlLastError = TM_EINVAL;
        retCode = TM_RAMFS_ERROR;
        goto RAMFS_INIT_EXIT;
    }

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    if (tm_global(tvRamfsPtr) != TM_VOID_NULL_PTR)
    {
/* Disk already initialized (only one disk allowed) */
        tlLastError = TM_EACCES;
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
/* Allocate the memory for the drive */
        mallocSize = tm_ramfs_drivesize(blCount);
        tm_global(tvRamfsPtr) = (ttVoidPtr)tm_kernel_single_malloc(
                                                (unsigned int)mallocSize);
        if (tm_global(tvRamfsPtr) == TM_VOID_NULL_PTR)
        {
            tlLastError = TM_ENOBUFS;
            retCode = TM_RAMFS_ERROR;
        }
        else
        {
/* Allocate the drive */
            ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
            tm_bzero(tm_global(tvRamfsPtr), mallocSize);
            tm_bzero(tlFileHandles, sizeof(ttVoidPtr) * TM_RAMFS_MAXFILES);

/* Set the drive size */
            ramfsPtr->rdLSize = mallocSize;
/* Set the drive block count */
            ramfsPtr->rdBlCount = blCount;
/* No current directory */
            ramfsPtr->rdBlCurDirAddress = 0;
/*
 * Set the delete file address to the second block (first is
 the root directory
 */
            ramfsPtr->rdEntryDeleted.reBlAddress = 1;
            ramfsPtr->rdEntryDeleted.reBlCount =
                            (ttRamfsBlock)(blCount - 1);
            ramfsPtr->rdEntryDeleted.reBlLastBlock =
                            (ttRamfsBlock)(blCount - 1);

/* Format each drive block */
            for (blNum = 1; blNum < blCount; blNum++)
            {
                tm_ramfs_nextblock(ramfsPtr, blNum - 1) = blNum;
                tm_ramfs_backblock(ramfsPtr, blNum) =
                                        (ttRamfsBlock)(blNum - 1);
            }
            tm_ramfs_backblock(ramfsPtr, 1) = TM_RAMFS_NOLINK;
            tm_ramfs_nextblock(ramfsPtr, blCount - 1) = TM_RAMFS_NOLINK;

/* Set the root directory attributes */
            tm_ramfs_getentry(ramfsPtr,0)->reUlFlags
                |= TM_RAMFS_ATTRIB_DIR;
            tm_ramfs_getentry(ramfsPtr,0)->reBlAddress = TM_RAMFS_NOLINK;
            tm_ramfs_getentry(ramfsPtr,0)->reBlParentEntry
                = TM_RAMFS_NOLINK;
            tm_ramfs_backblock(ramfsPtr,0) = TM_RAMFS_NOLINK;
            tm_ramfs_nextblock(ramfsPtr,0) = TM_RAMFS_NOLINK;

            ramfsPtr->rdUlFlags |= TM_RAMFS_MOUNTED;
        }
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

RAMFS_INIT_EXIT:
    return retCode;
}

/*
 * tfRamfsMountImage
 *
 *    Mounts the drive from a loaded image in RAM
 *
 *    Parameters
 *        pDriveBase     : base address of memory array
 *        mallocSize     : size of array
 *
 *    Return Values
 *        TM_RAMFS_OKAY  : The operation was successful
 */
int tfRamfsMountImage(ttVoidPtr pDriveBase, ttRamfsSize mallocSize)
{
    ttRamfsDrivePtr   ramfsPtr = (ttRamfsDrivePtr)pDriveBase;

/*
 * Set the flag indicating the file system was loaded rather than
 * initialized in place.
 */
    ramfsPtr->rdUlFlags |= TM_RAMFS_LOADED;
    ramfsPtr->rdLSize = mallocSize;
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    tm_global(tvRamfsPtr) = ramfsPtr;
    tm_bzero(tlFileHandles, sizeof(ttVoidPtr) * TM_RAMFS_MAXFILES);
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return TM_RAMFS_OKAY;
}

/* tfRamfsTerminate
 *
 *    Unmounts the drive.
 *
 *    Return Values
 *        TM_RAMFS_ERROR : No disk to destroy
 *        TM_RAMFS_OAKY  : The operation was successful
 */
int tfRamfsTerminate()
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        tm_global(tvRamfsPtr) = TM_VOID_NULL_PTR;
        if ((ramfsPtr->rdUlFlags & TM_RAMFS_LOADED) == 0)
        {
/*
 * Free the memory allocated for the drive, only if not mounted from a stored
 * image.
 */
            tm_kernel_single_free(ramfsPtr);
        }
        retCode = TM_RAMFS_OKAY;
    }
    else
    {
        retCode = TM_RAMFS_ERROR;
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
}


/* tfRamfsChDir
 *
 *    Changes the current working directory.
 *
 *    Parameters
 *        lpszDirName : Path of directory to be created
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : Directory was created successfully
 */
int tfRamfsChDir(ttConstCharPtr lpszDirName)
{
    ttRamfsBlock    blAddress;
    ttVoidPtr       hFind;
    int             retCode;
    ttRamfsDrivePtr ramfsPtr;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    retCode = TM_RAMFS_OKAY;

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        tlLastError = TM_ENOERROR;

/* Search for the directory address */
        hFind = tfRamfsFindFirstBlock(ramfsPtr, lpszDirName, &blAddress);
        if (hFind != (ttVoidPtr)0)
        {
            if ((tm_ramfs_getentry(ramfsPtr, blAddress)->reUlFlags &
                 TM_RAMFS_ATTRIB_DIR) == 0)
            {
/* Not a directory */
                tlLastError = TM_ENOTDIR;
            }
            else
            {
/* Set the new current address */
                ramfsPtr->rdBlCurDirAddress = blAddress;
            }
/* End the search */
            (void)tfRamfsFindClose(hFind);
        }
        else
        {
/* Directory not found */
            tlLastError = TM_ENOENT;
        }
    }
    if (tlLastError != TM_ENOERROR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    return retCode;
}

/* tfRamfsMkDir
 *
 *    Creates a new directory
 *
 *    Parameters
 *        lpszDirName : Path of directory to be created
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : Directory was created successfully
 */
int tfRamfsMkDir(ttConstCharPtr lpszDirName)
{
    ttRamfsBlock    blBaseDir;
    ttRamfsEntryPtr pNewEntry, pBaseDir;
    ttVoidPtr       hFind;
    ttCharPtr       lpszName;
    int             retCode;
    ttRamfsDrivePtr ramfsPtr;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        tlLastError = TM_ENOERROR;

        if (tfRamfsSplitPath(ramfsPtr, lpszDirName,
                             (ttConstCharPtr TM_FAR *)&lpszName,
                             &blBaseDir) != 0)
        {
            tlLastError = TM_ENOENT;
        }

        if ((tlLastError == TM_ENOERROR)
            && (tfRamfsIsValidFileName(lpszName, 0) == 0))
        {
/* Check if the directory doesn't exist already */
            hFind = tfRamfsFindFirstBlock(ramfsPtr, lpszDirName,
                                          TM_VOID_NULL_PTR);
            if (hFind == (ttVoidPtr)0)
            {
                pBaseDir = tm_ramfs_getentry(ramfsPtr, blBaseDir);
/* Grow the base directory entry */
                if (tfRamfsAllocBlock(ramfsPtr, 1, pBaseDir)
                    == 0)
                {
/* Fill in the new directory entry */
                    pNewEntry =
                        tm_ramfs_getentry(ramfsPtr, pBaseDir->reBlLastBlock);
                    pNewEntry->reUlFlags |= TM_RAMFS_ATTRIB_DIR;
                    pNewEntry->reBlParentEntry = blBaseDir;
                    pNewEntry->reBlAddress = TM_RAMFS_NOLINK;
                    pNewEntry->reBlLastBlockSize = 0;
                    tm_strcpy(pNewEntry->reSzName, lpszName);
                }
            }
            else
            {
/* Directory already exists */
                tlLastError = TM_EEXIST;
/* End the search */
                (void)tfRamfsFindClose(hFind);
            }
        }
        else
        {
/* Invalid directory name */
            tlLastError = TM_EINVAL;
        }
    }

    if (tlLastError != TM_ENOERROR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        retCode = TM_RAMFS_OKAY;
    }

    return retCode;
}


/* tfRamfsRmDir
 *
 *    Removes a directory.
 *
 *    Parameters
 *        lpszDirName : Path of directory to be removed
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : Directory was removed successfully
 */
int tfRamfsRmDir(ttConstCharPtr lpszDirName)
{
    ttRamfsBlock    blBaseDir, blEntry;
    ttRamfsEntryPtr pBaseDir, pEntry;
    ttVoidPtr       hFind;
    ttCharPtr       lpszName;
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        tlLastError = TM_ENOERROR;

        if (tfRamfsSplitPath(ramfsPtr,
                             lpszDirName, (ttConstCharPtr TM_FAR *)&lpszName,
                             &blBaseDir) != 0)
        {
            tlLastError = TM_ENOENT;
        }

        if ((tlLastError == TM_ENOERROR)
            && (tfRamfsIsValidFileName(lpszName, 0) == 0))
        {
            pBaseDir = tm_ramfs_getentry(ramfsPtr, blBaseDir);
/* Check if the directory exists */
            hFind = tfRamfsFindFirstBlock(ramfsPtr, lpszDirName, &blEntry);
            if (hFind != (ttVoidPtr)0)
            {
                pEntry = tm_ramfs_getentry(ramfsPtr, blEntry);
/* Check if it is a directory */
                if (pEntry->reUlFlags & TM_RAMFS_ATTRIB_DIR)
                {
/* Check if the directory is empty */
                    if (pEntry->reBlAddress == TM_RAMFS_NOLINK)
                    {
/* Free the base directory entry */
                        (void)tfRamfsFreeBlock( ramfsPtr,
                            tm_ramfs_getaddress(ramfsPtr,pEntry),
                            pBaseDir);
                    }
                    else
                    {
/* Directory not empty */
                        tlLastError = TM_EINVAL;
                    }
                }
                else
                {
/* Not a directory */
                    tlLastError = TM_ENOTDIR;
                }
/* End the search */
                (void)tfRamfsFindClose(hFind);
            }
            else
            {
/* Can't find directory */
                tlLastError = TM_ENOENT;
            }
        }
        else
        {
/* Invalid directory name */
            tlLastError = TM_EINVAL;
        }
    }

    if (tlLastError != TM_ENOERROR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        retCode = TM_RAMFS_OKAY;
    }

    return retCode;
}


/*
 * tfRamfsUnlink
 *    Deletes a file.
 *
 *    Parameters
 *        lpszFileName : Filename to delete
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : File successfully deleted
 */
int tfRamfsUnlink(ttConstCharPtr lpszFileName)
{
    ttRamfsBlock    blBaseDir, blEntry;
    ttRamfsEntryPtr pBaseDir, pEntry;
    ttVoidPtr       hFind;
    ttCharPtr       lpszName;
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        tlLastError = TM_ENOERROR;

        if (tfRamfsSplitPath(ramfsPtr,
                             lpszFileName, (ttConstCharPtr TM_FAR *)&lpszName,
                             &blBaseDir) != 0)
        {
            tlLastError = TM_ENOENT;
        }

        if ((tlLastError == TM_ENOERROR)
            && (tfRamfsIsValidFileName(lpszName, 0) == 0))
        {
            pBaseDir = tm_ramfs_getentry(ramfsPtr, blBaseDir);
/* Check if the directory exists */
            hFind = tfRamfsFindFirstBlock(ramfsPtr, lpszFileName, &blEntry);
            if (hFind != (ttVoidPtr)0)
            {
                pEntry = tm_ramfs_getentry(ramfsPtr, blEntry);
/* Check if it is a directory */
                if ((pEntry->reUlFlags & TM_RAMFS_ATTRIB_DIR) == 0)
                {
/* Free the entry content */
                    if (tfRamfsFreeEntryBlocks(ramfsPtr, pEntry, 0) == 0)
                    {
/* Free the base directory entry */
                        (void)tfRamfsFreeBlock( ramfsPtr,
                                    tm_ramfs_getaddress(ramfsPtr, pEntry),
                                    pBaseDir );
                    }
                }
                else
                {
/* Is a directory */
                    tlLastError = TM_EISDIR;
                }
/* End the search */
                (void)tfRamfsFindClose(hFind);
            }
            else
            {
/* Can't find directory */
                tlLastError = TM_ENOENT;
            }
        }
        else
        {
/* Invalid directory name */
            tlLastError = TM_EINVAL;
        }
    }

    if (tlLastError != TM_ENOERROR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        retCode = TM_RAMFS_OKAY;
    }

    return retCode;
}

/*
 * tfRamfsFindFirstFile
 *    Provides informations about the first instance of a filename
 *    that matches the file specified in the lpszFileName argument.
 *
 *    Parameters
 *        lpszFileName : Search pattern
 *        pFindInfo : Information structure pointer
 *
 *    Return Values
 *        (ttVoidPtr)0 : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        Other values : Handle for tfRamfsFindNextBlock or tfRamfsFindClose
 */
ttVoidPtr tfRamfsFindFirstFile(ttConstCharPtr lpszFileName,
                                ttRamfsFindInfoPtr pFindInfo)
{
    ttRamfsBlock    blEntry;
    ttVoidPtr       hResultPtr;
    ttRamfsDrivePtr ramfsPtr;

    hResultPtr = (ttVoidPtr)0; /* assume failure */
    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        if (pFindInfo == TM_VOID_NULL_PTR)
        {
/* Invalid pointer */
            tlLastError = TM_EINVAL;
        }
        else
        {
/* Find the block address */
            if (lpszFileName[0] == '\0')
            {
                hResultPtr = tfRamfsFindFirstBlock(ramfsPtr, "*", &blEntry);
            }
            else
            {
                hResultPtr = tfRamfsFindFirstBlock(ramfsPtr, lpszFileName,
                                                &blEntry);
            }

            if (hResultPtr != (ttVoidPtr)0)
            {
/* Fill in the information structure */
                tfRamfsGetEntryInfo(ramfsPtr, pFindInfo, blEntry);
            }
        }
    }

    return hResultPtr;
}


/*
 * tfRamfsFindNextFile
 *    Finds the next, if any, that matches the lpszFileName argument in
 *    a previous call to tfRamfsFindFirstFile.
 *
 *    Parameters
 *        hFind : Search handle returned by a previous call to
 *            tfRamfsFindFirstFile
 *        pFindInfo : Information structure pointer
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured or no more matching(call
 *                       tfRamfsErrNo for more details)
 *        TM_RAMFS_OKAY  : A matching was found
 */
int tfRamfsFindNextFile(ttVoidPtr hFind, ttRamfsFindInfoPtr pFindInfo)
{
    ttRamfsDrivePtr ramfsPtr;
    ttRamfsBlock    blEntry;
    int             nResult;

    nResult = TM_RAMFS_ERROR;
    blEntry = (ttRamfsBlock)0;
    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        if (pFindInfo == TM_VOID_NULL_PTR)
        {
/* Invalid pointer */
            tlLastError = TM_EINVAL;
        }
        else
        {
/* Find the block address */
            nResult = tfRamfsFindNextBlock(ramfsPtr, hFind, &blEntry);

            if (nResult != TM_RAMFS_ERROR)
            {
                if (pFindInfo->rfiBlEntry != blEntry)
                {
/* New entry */
/* Fill in the information structure */
                    tfRamfsGetEntryInfo(ramfsPtr, pFindInfo, blEntry);
                }
                else
                {
/* Next entry is the same, return an error */
                    nResult = TM_RAMFS_ERROR;
                }
            }
        }
    }
    return nResult;
}


/*
 * tfRamfsFindClose
 *    Closes the specified search handle and releases associated resources.
 *
 *    Parameters
 *        hFind : Handle to close
 *
 *    Return Values
 *        TM_RAMFS_ERROR : Invalid handle value
 *        TM_RAMFS_OKAY  : The operation was successful
 */
int tfRamfsFindClose(ttVoidPtr hFind)
{
    int retCode;

    if ((hFind == (ttVoidPtr)-1) || (hFind == TM_VOID_NULL_PTR))
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
/* Free the find handle */
        tm_free(hFind);
        retCode = TM_RAMFS_OKAY;
    }

    return retCode;
}


/*
 * tfRamfsMatchFileName
 *    Matches a filename with a mask.
 *
 *    Parameters
 *        lpszFileName : Filename to match
 *        lpszMask : Mask
 *
 *    Return Values
 *        TM_RAMFS_ERROR : lpszFileName doesn't match with lpszMask
 *        TM_RAMFS_OKAY  : lpszFileName matches with lpszMask
 */
int tfRamfsMatchFileName(ttConstCharPtr lpszFileName, ttConstCharPtr lpszMask)
{
    int retCode;

    retCode = TM_RAMFS_OKAY;
    for (;;)
    {
        if (lpszMask[0] == '?')
        {
            if (lpszFileName[0] == '\0')
            {
                retCode = TM_RAMFS_ERROR;
                break;
            }

            lpszMask++;
            lpszFileName++;
        }
        else if (lpszMask[0] == '*')
        {
            if (lpszMask[1] == lpszFileName[0])
            {
                lpszMask++;
            }
            else
            {
                lpszFileName++;
            }
        }
        else if (lpszMask[0] != lpszFileName[0])
        {
            retCode = TM_RAMFS_ERROR;
            break;
        }
        else if (lpszMask[0] == '\0')
        {
            break;
        }
        else
        {
            lpszMask++;
            lpszFileName++;
        }
    }

    return retCode;
}

/*
 * tfRamfsGetCwd
 *    Gets the current working directory.
 *
 *    Parameters
 *        lpszDirName : Directory buffer
 *        nBufLen : Buffer size
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : Directory was successfully got
 */
int tfRamfsGetCwd(ttCharPtr lpszDirName, int nBufLen)
{
    int             nBufPos;
    ttRamfsBlock    blCur;
    ttRamfsEntryPtr pEntry;
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        tlLastError = TM_ENOERROR;

        if ((lpszDirName != TM_VOID_NULL_PTR) && (nBufLen > 0))
        {
            nBufPos = nBufLen - 1;
            lpszDirName[nBufPos] = '\0';
            nBufPos--;

            if (nBufPos > 0)
            {
/* Root */
                lpszDirName[0] = TM_RAMFS_DIRSEP;
                lpszDirName[1] = '\0';

/* Get the current directory address */
                blCur = ramfsPtr->rdBlCurDirAddress;
                lpszDirName[nBufPos] = TM_RAMFS_DIRSEP;
                nBufPos--;

                while (tm_ramfs_getentry(ramfsPtr, blCur)->reBlParentEntry
                        != TM_RAMFS_NOLINK)
                {
/* Get the parent directory entry */
                    pEntry = tm_ramfs_getentry(ramfsPtr, blCur);
/* Get the upper directory */
                    blCur = pEntry->reBlParentEntry;
                    if(tm_ramfs_getentry(ramfsPtr, blCur)->reBlParentEntry
                            != TM_RAMFS_NOLINK)
                    {
                        nBufPos -= (int)tm_strlen(pEntry->reSzName);
                        if (nBufPos >= 0 )
                        {
/* Add the directory name to the start of the path name */
                            tm_memcpy(lpszDirName + nBufPos + 1,
                                      pEntry->reSzName,
                                      tm_strlen(pEntry->reSzName));
                            lpszDirName[nBufPos] = TM_RAMFS_DIRSEP;
                            nBufPos--;
                        }
                        else
                        {
/* The directory name buffer length is not enough */
                            tlLastError = TM_EINVAL;
                            break;
                        }
                    }
                    else
                    {
/* Just under root directory, copy all the directory name, we are finished */
                        tm_memcpy(lpszDirName + 1,
                                  pEntry->reSzName,
                                  tm_strlen(pEntry->reSzName));
                        tm_memcpy(lpszDirName + tm_strlen(pEntry->reSzName) + 1,
                                  lpszDirName + nBufPos + 1,
                                  (nBufLen - nBufPos) - 1);
                    }
                }
            }
            else
            {
/* The directory name buffer length is not enough */
                tlLastError = TM_EINVAL;
            }
        }
        else
        {
/* Invalid parameters */
            tlLastError = TM_EINVAL;
        }
    }

    if (tlLastError != TM_ENOERROR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        retCode = TM_RAMFS_OKAY;
    }

    return retCode;
}


/* tfRamfsGetDiskFreeSpace
 *
 *    Retrieves information about the specified disk,
 *    including the amount of free space on the disk.
 *
 *    Parameters
 *        pBlockSize : Pointer to the size of the block
 *        pFreeBlockCount : Pointer to the free block count
 *        pTotalBlockCount : Pointer to the total block count
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : Informations successfully got
 */
int tfRamfsGetDiskFreeSpace( ttRamfsBlockPtr pBlockSize,
                             ttRamfsBlockPtr pFreeBlockCount,
                             ttRamfsBlockPtr pTotalBlockCount )
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;

    retCode = TM_RAMFS_ERROR;
    if ((pBlockSize == TM_VOID_NULL_PTR)
        || (pFreeBlockCount == TM_VOID_NULL_PTR)
        || (pTotalBlockCount == TM_VOID_NULL_PTR))
    {
        tlLastError = TM_EINVAL;
    }
    else
    {
        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

        if (ramfsPtr == (ttRamfsDrivePtr)0)
        {
            tlLastError = TM_ENOENT;
        }
        else
        {
            tlLastError = TM_ENOERROR;
            *pBlockSize = TM_RAMFS_BLOCKSIZE;
            *pFreeBlockCount = ramfsPtr->rdEntryDeleted.reBlCount;
            *pTotalBlockCount = ramfsPtr->rdBlCount;
            retCode = TM_RAMFS_OKAY;
        }
    }

    return retCode;
}


/* tfRamfsRename
 *
 *    Renames or moves a file.
 *
 *    Parameters
 *        lpszOldPath : Old pathname
 *        lpszNewPath : New pathname
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : File successfully renamed/moved
 */
int tfRamfsRename(ttConstCharPtr lpszOldPath, ttConstCharPtr lpszNewPath)
{
    ttConstCharPtr  lpszOldName, lpszNewName;
    ttRamfsBlock    blOldBaseDir, blNewBaseDir, blOldEntry, blNewEntry;
    ttVoidPtr       hFind;
    ttCharPtr       pszNewPath;
    ttRamfsDrivePtr ramfsPtr;
    ttSShort        sLen;
    int             retCode;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        tlLastError = TM_ENOERROR;

        if ( tfRamfsSplitPath( ramfsPtr,
                               lpszOldPath,
                               (ttConstCharPtr TM_FAR *)&lpszOldName,
                               &blOldBaseDir) == 0)
        {
/* Find the old file */
            hFind = tfRamfsFindFirstBlock(ramfsPtr, lpszOldPath, &blOldEntry);

            if (hFind != (ttVoidPtr)0)
            {
/* we found the old file, close it */
                (void)tfRamfsFindClose(hFind);

                pszNewPath = tm_malloc(TM_RAMFS_MAXPATH + 1);

                if (pszNewPath == TM_VOID_NULL_PTR)
                {
/* Out of memory */
                    tlLastError = TM_ENOBUFS;
                }
                else
                {
/* see if a file with the new filename already exists */
                    hFind = tfRamfsFindFirstBlock(
                        ramfsPtr, lpszNewPath, &blNewEntry);
                    tm_strcpy(pszNewPath, lpszNewPath);

                    if (hFind != (ttVoidPtr)0)
                    {
/* a file with the new filename already exists - check if it is a directory */
                        if (tm_ramfs_getentry(ramfsPtr, blNewEntry)->reUlFlags
                            & TM_RAMFS_ATTRIB_DIR)
                        {
/* we are moving a file to a directory */
                            sLen = (ttSShort)tm_strlen(pszNewPath);
/* New path is a directory we concatenate the directory with the old name */
                            if (pszNewPath[sLen - 1] != TM_RAMFS_DIRSEP)
                            {
                                pszNewPath[sLen] = TM_RAMFS_DIRSEP;
                                pszNewPath[sLen + 1] = '\0';
                            }

                            (void)tm_strcat(pszNewPath, lpszOldName);

                            (void)tfRamfsFindClose(hFind);
/* we allow moving a file to a directory that already exists */
                            hFind = (ttVoidPtr)0;
                        }
                    }
                    else
                    {
/* reset our global error code to success, we expect the new file does not
   exist */
                        tlLastError = TM_ENOERROR;
                    }

                    if (hFind == (ttVoidPtr)0)
                    {
/* we are either moving a file to a new filename that doesn't exist, or
   to a directory that already exists */
                        if (tfRamfsSplitPath(ramfsPtr,
                                             pszNewPath, &lpszNewName,
                                             &blNewBaseDir) == 0)
                        {
                            if (tfRamfsIsValidFileName(lpszNewName, 0) == 0)
                            {
                                if (blOldBaseDir == blNewBaseDir)
                                {
/* Simple rename */
                                    tm_strcpy(tm_ramfs_getentry(ramfsPtr,
                                        blOldEntry)->reSzName, lpszNewName);
                                }
                                else
                                {
/* Move operation: allocate a block in the new base dir */
                                    if (tfRamfsAllocBlock(ramfsPtr, 1,
                                         tm_ramfs_getentry(ramfsPtr,
                                                           blNewBaseDir)) == 0)
                                    {
/* Get the allocated block */
                                        blNewEntry = tm_ramfs_getentry(
                                            ramfsPtr,
                                            blNewBaseDir)->reBlLastBlock;

/* Copy the entry from the old base dir to the new base dir */
                                        tm_memcpy(
                                            tm_ramfs_getentry(ramfsPtr,
                                            blNewEntry), tm_ramfs_getentry(
                                            ramfsPtr, blOldEntry),
                                            sizeof(ttRamfsEntry));

/* Copy the new entry name */
                                        tm_strcpy(tm_ramfs_getentry(ramfsPtr,
                                            blNewEntry)->reSzName, lpszNewName);

/* Free the entry block in the old base dir */
                                        (void)tfRamfsFreeBlock(
                                               ramfsPtr,
                                               blOldEntry,
                                               tm_ramfs_getentry(ramfsPtr,
                                                                blOldBaseDir));
                                    }
                                    else
                                    {
/* No space left */
                                        tlLastError = TM_ENOSPC;
                                    }
                                }
                            }
                            else
                            {
/* Invalid file name */
                                tlLastError = TM_EINVAL;
                            }
                        }
                        else
                        {
/* Base dir of new pathname not found */
                            tlLastError = TM_ENOENT;
                        }
                    }
                    else
                    {
/* New pathname already exist */
                        (void)tfRamfsFindClose(hFind);
                        tlLastError = TM_EEXIST;
                    }

                    tm_free(pszNewPath);
                }
            }
            else
            {
/* File not found */
                tlLastError = TM_ENOENT;
            }
        }
        else
        {
/* File not found */
            tlLastError = TM_ENOENT;
        }
    }

    if (tlLastError != TM_ENOERROR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        retCode = TM_RAMFS_OKAY;
    }
    return retCode;
}


/* tfRamfsCopy
 *
 *    Copies a file.
 *
 *    Parameters
 *        lpszSrcPath : Source pathname
 *        lpszDestPath : Destination pathname
 *        nOverwrite : 0 -> error if destination pathname exists
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : File successfully copied
 */
int tfRamfsCopy(ttConstCharPtr lpszOldPath,
                 ttConstCharPtr lpszNewPath,
                 int nOverwrite)
{
    ttConstCharPtr  lpszOldName, lpszNewName;
    ttRamfsBlock    blOldBaseDir, blNewBaseDir, blOldEntry, blNewEntry, blCount;
    ttRamfsBlock    blSrcAddress, blDestAddress, blNum, blLastBlock;
    ttRamfsEntryPtr pSrcEntry, pDestEntry;
    ttRamfsDrivePtr ramfsPtr;
    ttVoidPtr       hFind;
    ttCharPtr       pszNewPath;
    ttSShort        sLen;
    int             retCode;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        tlLastError = TM_ENOERROR;

        if (tfRamfsSplitPath(ramfsPtr,
                             lpszOldPath,
                             (ttConstCharPtr TM_FAR *)&lpszOldName,
                             &blOldBaseDir) == 0)
        {
/* Find the old file */
            hFind = tfRamfsFindFirstBlock(ramfsPtr, lpszOldPath, &blOldEntry);

            if (hFind != (ttVoidPtr)0)
            {
/* Close the find */
                (void)tfRamfsFindClose(hFind);

                pszNewPath = tm_malloc(TM_RAMFS_MAXPATH + 1);

                if (pszNewPath == TM_VOID_NULL_PTR)
                {
/* Out of memory */
                    tlLastError = TM_ENOBUFS;
                }
                else
                {
                    tm_strcpy(pszNewPath, lpszNewPath);
                    if (tfRamfsSplitPath( ramfsPtr, pszNewPath,
                                       (ttConstCharPtr TM_FAR *)&lpszNewName,
                                       &blNewBaseDir) == 0)
                    {
                        hFind = tfRamfsFindFirstBlock(ramfsPtr,
                                                      lpszNewPath, &blNewEntry);

                        if (hFind != (ttVoidPtr)0)
                        {
                            if (tm_ramfs_getentry(ramfsPtr,
                                                    blNewEntry)->reUlFlags
                                & TM_RAMFS_ATTRIB_DIR)
                            {
                                sLen = (ttSShort)tm_strlen(pszNewPath);
/* New path is a directory. We concatenate the directory with the old name */
                                if (pszNewPath[sLen - 1] != TM_RAMFS_DIRSEP)
                                {
                                    pszNewPath[sLen] = TM_RAMFS_DIRSEP;
                                    pszNewPath[sLen + 1] = '\0';
                                }

                                (void)tm_strcat(pszNewPath, lpszOldName);

/* Get the new base dir */
                                (void)tfRamfsSplitPath( ramfsPtr,
                                       pszNewPath,
                                       (ttConstCharPtr TM_FAR *)&lpszNewName,
                                       &blNewBaseDir);

                                (void)tfRamfsFindClose(hFind);
                                hFind = (ttVoidPtr)0;
                            }
                            else
                            {
/* File already exists */
                                if (nOverwrite != 0)
                                {
/* Overwrite the file */
                                    if (blNewEntry != blOldEntry)
                                    {
/*
 * Source and destination are not the same
 * Free the entry block of the file to overwrite
 */
                                        (void)tfRamfsFreeBlock(
                                               ramfsPtr,
                                               blNewEntry,
                                               tm_ramfs_getentry(
                                                  ramfsPtr, blNewBaseDir));

                                        (void)tfRamfsFindClose(hFind);
                                        hFind = (ttVoidPtr)0;
                                    }
                                }
                            }
                        }

                        if (hFind == (ttVoidPtr)0)
                        {
                            if (tfRamfsIsValidFileName(lpszNewName, 0) == 0)
                            {
/* Allocate a block in the new base dir */
                                if (tfRamfsAllocBlock(ramfsPtr, 1,
                                           tm_ramfs_getentry(
                                    ramfsPtr, blNewBaseDir)) == 0)
                                {
/* Get the allocated block */
                                    blNewEntry = tm_ramfs_getentry(ramfsPtr,
                                        blNewBaseDir)->reBlLastBlock;

                                    blCount = tm_ramfs_getentry(ramfsPtr,
                                        blOldEntry)->reBlCount;

                                    pSrcEntry = tm_ramfs_getentry(ramfsPtr,
                                        blOldEntry);
                                    pDestEntry = tm_ramfs_getentry(ramfsPtr,
                                        blNewEntry);

/* Allocate blocks for the destination file */
                                    if (tfRamfsAllocBlock(ramfsPtr, blCount,
                                        tm_ramfs_getentry(ramfsPtr, blNewEntry))
                                        == 0)
                                    {
                                        blSrcAddress = pSrcEntry->reBlAddress;
                                        blDestAddress = pDestEntry->reBlAddress;
                                        blLastBlock = pDestEntry->reBlLastBlock;

                                        tm_memcpy(pDestEntry, pSrcEntry,
                                                        sizeof(ttRamfsEntry));

                                        pDestEntry->reBlAddress = blDestAddress;
                                        pDestEntry->reBlLastBlock = blLastBlock;
                                        tm_strcpy(pDestEntry->reSzName,
                                                                lpszNewName);

/* Copy the file block by block */
                                        for ( blNum = 0; blNum < blCount;
                                              blNum++ )
                                        {
                                            if (blSrcAddress != TM_RAMFS_NOLINK)
                                            {
/* Copy one block */
                                                tm_memcpy(ramfsPtr->rdRawBlock[
                                                    blDestAddress].rrbEntry.reBlock,
                                                    ramfsPtr->rdRawBlock[
                                                    blSrcAddress].rrbEntry.reBlock,
                                                    TM_RAMFS_BLOCKSIZE);
/* Get next blocks */
                                                blSrcAddress =
                                                        tm_ramfs_nextblock(
                                                    ramfsPtr, blSrcAddress);
                                                blDestAddress =
                                                        tm_ramfs_nextblock(
                                                            ramfsPtr,
                                                            blDestAddress);
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }
                                    }
                                    else
                                    {
/* No space left */
                                        tlLastError = TM_ENOSPC;
                                    }
                                }
                                else
                                {
/* No space left */
                                    tlLastError = TM_ENOSPC;
                                }
                            }
                            else
                            {
/* Invalid file name */
                                tlLastError = TM_EINVAL;
                            }
                        }
                        else
                        {
/* New pathname already exist */
                            (void)tfRamfsFindClose(hFind);
                            tlLastError = TM_EEXIST;
                        }
                    }
                    else
                    {
/* Base dir of new pathname not found */
                        tlLastError = TM_ENOENT;
                    }

                    tm_free(pszNewPath);
                }
            }
            else
            {
/* File not found */
                tlLastError = TM_ENOENT;
            }
        }
        else
        {
/* File not found */
            tlLastError = TM_ENOENT;
        }
    }

    if (tlLastError != TM_ENOERROR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        retCode = TM_RAMFS_OKAY;
    }

    return retCode;
}

/*
 * Treck Ram File System Low Level Functions
 */

/* tfRamfsGetDrive
 *
 *    Gets the pointer of the address of the drive structure.
 *
 *    Return Values
 *        Pointer to the drive structure
 */
ttRamfsDrivePtr tfRamfsGetDrive(void)
{
    ttRamfsDrivePtr ramfsPtr;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    return ramfsPtr;
}

/* tfRamfsAllocBlock
 *
 *    Allocates block(s) or grow allocated block for the entry.
 *
 *    Parameters
 *        blCount : Number of blocks to allocate
 *        pEntry : Entry linked with allocated blocks
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : blCount Blocks were successfully allocated
 */
static int tfRamfsAllocBlock(ttRamfsDrivePtr ramfsPtr,
                             ttRamfsBlock blCount, ttRamfsEntryPtr pEntry)
{
    ttRamfsBlock    blNum, blNext, blLast, bOldLast;
    int             retCode;

    tlLastError = TM_ENOERROR;

    blLast = 0; /* suppress compiler warning */
    if (blCount > ramfsPtr->rdEntryDeleted.reBlCount)
    {
/* Not enough free blocks */
        tlLastError = TM_ENOSPC;
    }
    else if (blCount > 0)
    {
/* Note: drive locked (in tfFSxxx() functions) */
        {
/* Get the next free block address */
            blNext = ramfsPtr->rdEntryDeleted.reBlAddress;

            if (pEntry->reBlCount == 0)
            {
/* New block array to allocate. Set the block array address */
                pEntry->reBlAddress = blNext;
                bOldLast = TM_RAMFS_NOLINK;
            }
            else
            {
/* Grow the block array */
                tm_ramfs_nextblock(ramfsPtr, pEntry->reBlLastBlock) = blNext;
/* Save the last block of the entry */
                bOldLast = pEntry->reBlLastBlock;
            }

/* Follow the links between blocks to get the last one */
            for (blNum = 0; blNum < blCount; blNum++)
            {
                blLast = blNext;
                blNext = tm_ramfs_nextblock(ramfsPtr, blNext);
            }

/* Update the directory entry */
            pEntry->reBlCount = (ttRamfsBlock)(pEntry->reBlCount + blCount);
            pEntry->reBlLastBlock = blLast;

/* Break the link at the first block and at the last block */
            tm_ramfs_backblock(ramfsPtr,ramfsPtr->rdEntryDeleted.reBlAddress)
                                                            = bOldLast;
            tm_ramfs_nextblock(ramfsPtr, blLast) = TM_RAMFS_NOLINK;

            if (blNext != TM_RAMFS_NOLINK)
            {
                tm_ramfs_backblock(ramfsPtr, blNext) = TM_RAMFS_NOLINK;
            }

/* Update the deleted file entry */
            ramfsPtr->rdEntryDeleted.reBlCount =
                (ttRamfsBlock)(ramfsPtr->rdEntryDeleted.reBlCount - blCount);
            ramfsPtr->rdEntryDeleted.reBlAddress = blNext;
        }
    }
    else
    {
/* Invalid parameter */
        tlLastError = TM_EINVAL;
    }

    if (tlLastError != TM_ENOERROR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        retCode = TM_RAMFS_OKAY;
    }

    return retCode;
}


/* tfRamfsFreeBlock
 *
 *    Frees an allocated block.
 *
 *    Parameters
 *        blAddress : Address of the block to free
 *        pEntry : Entry of the address (can be NULL)
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : blAddress was successfully freed
 */
static int tfRamfsFreeBlock( ttRamfsDrivePtr ramfsPtr,
                             ttRamfsBlock blAddress, ttRamfsEntryPtr pEntry)
{
    ttRamfsBlock    blBack, blNext, blOldBack, blOldNext;
    int             retCode;

    tlLastError = TM_ENOERROR;
    retCode = TM_RAMFS_OKAY;

    blOldBack = tm_ramfs_backblock(ramfsPtr, blAddress);
    blOldNext = tm_ramfs_nextblock(ramfsPtr, blAddress);

    if (blAddress < ramfsPtr->rdEntryDeleted.reBlAddress)
    {
/* The block to delete is before the first free block */

/* Link the block as the beginning of the free space */
        tm_ramfs_backblock(ramfsPtr, blAddress) =
            TM_RAMFS_NOLINK;
        tm_ramfs_nextblock(ramfsPtr, blAddress) =
            ramfsPtr->rdEntryDeleted.reBlAddress;

        if (ramfsPtr->rdEntryDeleted.reBlAddress != TM_RAMFS_NOLINK)
        {
            tm_ramfs_backblock(ramfsPtr, ramfsPtr->rdEntryDeleted.reBlAddress) =
                blAddress;
        }

/* Set the new starting address of the free space */
        ramfsPtr->rdEntryDeleted.reBlAddress = blAddress;
    }
    else if (blAddress < ramfsPtr->rdBlCount)
    {
/* The block to delete is after the free block */

/* Find the nearest free block before the block */
        blBack = ramfsPtr->rdEntryDeleted.reBlAddress;

        while (tm_ramfs_nextblock(ramfsPtr, blBack) < blAddress)
        {
            blBack = tm_ramfs_nextblock(ramfsPtr, blBack);
        }

        blNext = tm_ramfs_nextblock(ramfsPtr, blBack);

/* Link the block within the free space */
        tm_ramfs_backblock(ramfsPtr, blAddress) = blBack;

        if (blNext != TM_RAMFS_NOLINK)
        {
            tm_ramfs_backblock(ramfsPtr, blNext) = blAddress;
        }

        tm_ramfs_nextblock(ramfsPtr, blAddress) = blNext;
        tm_ramfs_nextblock(ramfsPtr, blBack) = blAddress;
    }
    else
    {
        tlLastError = TM_EFAULT;
        retCode = TM_RAMFS_ERROR;
    }

    if (tlLastError == TM_ENOERROR)
    {
/* Link the old back block an the old next block together */

        if (blOldNext != TM_RAMFS_NOLINK)
        {
            tm_ramfs_backblock(ramfsPtr, blOldNext)
                = blOldBack;
        }

        if (blOldBack != TM_RAMFS_NOLINK)
        {
            tm_ramfs_nextblock(ramfsPtr, blOldBack)
                = blOldNext;
        }

        if (tm_ramfs_nextblock(ramfsPtr, blAddress) == TM_RAMFS_NOLINK)
        {
/* The block is the last deleted block */
            ramfsPtr->rdEntryDeleted.reBlLastBlock = blAddress;
        }

/* Grow the free space */
        ramfsPtr->rdEntryDeleted.reBlCount++;

/* Empty the deleted block */
        tm_memset(&ramfsPtr->rdRawBlock[blAddress].rrbEntry, 0,
            sizeof(ttRamfsEntry));

        if (pEntry != TM_VOID_NULL_PTR)
        {
/* Descrease the space used by the entry */
            pEntry->reBlCount--;

            if (pEntry->reBlCount == 0)
            {
/* No more block in the entry */
                pEntry->reBlAddress = TM_RAMFS_NOLINK;
            }

            if (blOldBack == TM_RAMFS_NOLINK)
            {
/* update parent directory first block index to new first directory entry */
                pEntry->reBlAddress = blOldNext;
            }

            if (blOldNext == TM_RAMFS_NOLINK)
            {
/* update parent directory last block index to new last directory entry */
                pEntry->reBlLastBlock = blOldBack;
            }
        }
    }

    return retCode;
}


/* tfRamfsFreeEntryBlocks
 *
 *    Frees blCount last block of an entry.
 *
 *    Parameters
 *        pEntry : Entry whose content to free
 *        blCount : Number of block to free (0 to free the whole content)
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more
 *                       details)
 *        TM_RAMFS_OKAY  : Blocks were successfully freed
 */
static int tfRamfsFreeEntryBlocks( ttRamfsDrivePtr ramfsPtr,
                                   ttRamfsEntryPtr pEntry, ttRamfsBlock blCount)
{
    ttRamfsBlock    blNum, blAddress, blLastBlock, tempBlAddress;
    int             retCode;

    tlLastError = TM_ENOERROR;

/* Check if the entry address is valid (not out of range) */
    if (pEntry->reBlAddress < ramfsPtr->rdBlCount)
    {
        if ((blCount == 0) || (blCount >= pEntry->reBlCount))
        {
            blCount = pEntry->reBlCount;
            blAddress = pEntry->reBlAddress;
            blLastBlock = TM_RAMFS_NOLINK;
        }
        else
        {
/* Find the starting block address of the area to free */
            for (blNum = 1, blAddress = pEntry->reBlLastBlock;
                blNum < blCount; blNum++)
            {
                blAddress = tm_ramfs_backblock(ramfsPtr, blAddress);
            }
/* Find the new last block of the entry */
            blLastBlock = tm_ramfs_backblock(ramfsPtr, blAddress);
        }

/* empty each deleted block */
        for (blNum = 0, tempBlAddress = blAddress; blNum < blCount; blNum++)
        {
            tm_memset(&ramfsPtr->rdRawBlock[tempBlAddress].rrbEntry, 0,
                      sizeof(ttRamfsEntry));
            tempBlAddress = tm_ramfs_nextblock(ramfsPtr, tempBlAddress);
        }

/* link the last block to delete to the first in the list of deleted blocks */
        tm_ramfs_nextblock(ramfsPtr, pEntry->reBlLastBlock) =
            ramfsPtr->rdEntryDeleted.reBlAddress;
        if (ramfsPtr->rdEntryDeleted.reBlAddress != TM_RAMFS_NOLINK)
        {
/* backlink from the first in the list of deleted blocks to the last block to
   delete */
            tm_ramfs_backblock(ramfsPtr, ramfsPtr->rdEntryDeleted.reBlAddress) =
                pEntry->reBlLastBlock;
        }
/* unlink the first block to delete's backlink */
        tm_ramfs_backblock(ramfsPtr, blAddress) = TM_RAMFS_NOLINK;
/* add the blocks to delete to the list of deleted blocks */
        ramfsPtr->rdEntryDeleted.reBlAddress = blAddress;

        if (blLastBlock != TM_RAMFS_NOLINK)
        {
            tm_ramfs_nextblock(ramfsPtr, blLastBlock) = TM_RAMFS_NOLINK;
        }

/* Inscrease the free space */
        ramfsPtr->rdEntryDeleted.reBlCount =
            (ttRamfsBlock)(ramfsPtr->rdEntryDeleted.reBlCount + blCount);

/* Update the entry */
        pEntry->reBlCount = (ttRamfsBlock)(pEntry->reBlCount - blCount);
        pEntry->reBlLastBlock = blLastBlock;
        pEntry->reBlLastBlockSize = 0;

        if (pEntry->reBlCount == 0)
        {
            pEntry->reBlAddress = TM_RAMFS_NOLINK;
        }
        retCode = TM_RAMFS_OKAY;
    }
    else
    {
        tlLastError = TM_EBADF;
        retCode = TM_RAMFS_ERROR;
    }

    return retCode;
}


/* tfRamfsIsValidFileName
 *
 *    Checks the validity of a filename (not a pathname).
 *
 *    Parameters
 *        lpszFileName : Filename to check
 *        nDirectory : lpszFileName can be a special directory name (".." or ".")
 *
 *    Return Values
 *        TM_RAMFS_ERROR : Invalid filename
 *        TM_RAMFS_OKAY  : Valid filename
 */
static int tfRamfsIsValidFileName(ttConstCharPtr lpszFileName, int nDirectory)
{
    int retCode;
    int finished;

/* check for acceptable file name length */
    if(tm_strlen(lpszFileName)>=TM_RAMFS_MAXFNAME)
    {
        retCode = TM_RAMFS_ERROR;
        finished = 1;
    }
    else
    {
        retCode = TM_RAMFS_OKAY;
        finished = 0;

        switch(lpszFileName[0])
        {
/* file name is null, return error */
        case '\0':
            retCode = TM_RAMFS_ERROR;
            finished = 1;
            break;

        case '.':
/* file name begin with '.' */
            switch(lpszFileName[1])
            {
            case '\0':
/* file name = '.', if not allowed, return error, else return okay */
                if (nDirectory == 0)
                {
                    retCode = TM_RAMFS_ERROR;
                }
                finished = 1;
                break;

            case '.':
                if (lpszFileName[2] == '\0')
                {
/* file name = '..', if not allowed, return error, else return okay */
                    if (nDirectory == 0)
                    {
                        retCode = TM_RAMFS_ERROR;
                    }
                    finished = 1;
                    break;
                }
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
    }

/* all other cases, check for invalid char */
    while((lpszFileName[0] != '\0')&&(!finished))
    {
       if (tm_strchr(TM_RAMFS_INVALIDCHAR, lpszFileName[0])
                                                         != TM_VOID_NULL_PTR)
        {
            retCode = TM_RAMFS_ERROR;
            finished = 1;
        }
        lpszFileName++;
    }

    return retCode;
}


/*
 * tfRamfsSplitPath
 *    Splits a pathname into its name part and the pointer to its
 *    parent directory entry.
 *
 *    Parameters
 *        lpszPathName : Pathname to split
 *        lpszFileName : Pointer to the name part of lpszPathName
 *        pblDirEntry : Pointer to the parent directory of lpszPathName
 *
 *    Return Values
 *        TM_RAMFS_ERROR : Invalid handle value
 *        TM_RAMFS_OKAY  : The operation was successful
 */
static int tfRamfsSplitPath( ttRamfsDrivePtr ramfsPtr,
                             ttConstCharPtr lpszPathName,
                             ttConstCharPtr TM_FAR * lpszFileName,
                             ttRamfsBlockPtr pblDirEntry )
{
    ttCharPtr       lpszDir;
    ttVoidPtr       hFind;
    int             nResult;
    int             nDirSep;

    nResult = TM_RAMFS_ERROR;

/* Search for the last path separator */
    if ((*lpszFileName = tm_strrchr(lpszPathName, TM_RAMFS_DIRSEP))
                                                     != (ttConstCharPtr)0)
    {
/* Get the address of the name part */
        (*lpszFileName)++;
        nDirSep = (int)(ttUserPtrCastToInt)(*lpszFileName - lpszPathName);

        if (nDirSep == 1)
        {
/* Root directory case */
            *pblDirEntry = 0;
            nResult = TM_RAMFS_OKAY;
        }
        else
        {
            lpszDir = (ttCharPtr)tm_malloc((unsigned int)nDirSep);

            if (lpszDir != TM_VOID_NULL_PTR)
            {
/* Copy the base directory name */
                (void)tm_strncpy(lpszDir, lpszPathName, nDirSep - 1);
                lpszDir[nDirSep - 1] = '\0';

/* Get the base directory entry pointer */
                hFind = tfRamfsFindFirstBlock(ramfsPtr, lpszDir, pblDirEntry);
                if (hFind != (ttVoidPtr)0)
                {
                    nResult = TM_RAMFS_OKAY;
                    (void)tfRamfsFindClose(hFind);
                }

                tm_free(lpszDir);
            }
        }
    }
    else
    {
/* No path separator name part = pathname */
        nResult = TM_RAMFS_OKAY;
        lpszDir = TM_VOID_NULL_PTR;
        *lpszFileName = (ttCharPtr)lpszPathName;
        *pblDirEntry = ramfsPtr->rdBlCurDirAddress;
    }

    return nResult;
}


typedef struct tsRamfsPrivateFindStruct
{
    char                rpfsSzFileName[TM_RAMFS_MAXPATH];
    ttConstCharPtr      rpfsLpszName;
    ttRamfsBlock        rpfsBlBaseDir;
    ttRamfsBlockPtr     rpfsPblNext;
} ttRamfsPrivateFindStruct, TM_FAR *ttRamfsPrivateFindStructPtr;


/* tfRamfsFindFirstBlock
 *
 *    Provides block address of the first instance of a filename
 *    that matches the file specified in the lpszFileName argument.
 *
 *    Parameters
 *        lpszFileName : Search pattern
 *        pblFoundEntry : Directory entry block address of the found file
 *
 *    Return Values
 *        TM_RAMFS_ERROR : An error as occured (call tfRamfsErrNo for more details)
 *        Other values : Handle for tfRamfsFindNextBlock or tfRamfsFindClose
 */
static ttVoidPtr tfRamfsFindFirstBlock( ttRamfsDrivePtr ramfsPtr,
                                        ttConstCharPtr lpszFileName,
                                        ttRamfsBlockPtr pblFoundEntry)
{
    ttRamfsPrivateFindStructPtr pResult;
    ttCharPtr                   lpszName;
    int                         nResult;

    tlLastError = TM_ENOENT; /* Assume failure */
    pResult = TM_VOID_NULL_PTR; /* Assume failure */

    if (lpszFileName[0] == (char)0)
    {
        tlLastError = TM_EINVAL; /* overwrite default error code */
    }
    else
    {
        pResult = (ttRamfsPrivateFindStructPtr)
            tm_malloc(sizeof(ttRamfsPrivateFindStruct));

        if (pResult == TM_VOID_NULL_PTR)
        {
            tlLastError = TM_ENOBUFS; /* overwrite default error code */
        }
        else
        {
/* Get the path base directory and its block address */
            if (tfRamfsSplitPath( ramfsPtr,
                                  lpszFileName,
                                  (ttConstCharPtr TM_FAR *)&lpszName,
                                  &pResult->rpfsBlBaseDir) == 0)
            {
/* Fill in the handle structure */
                pResult->rpfsPblNext = &tm_ramfs_getentry(ramfsPtr,
                    pResult->rpfsBlBaseDir)->reBlAddress;
                tm_strcpy(pResult->rpfsSzFileName, lpszFileName);
                pResult->rpfsLpszName = pResult->rpfsSzFileName +
                    ((ttConstCharPtr)lpszName - lpszFileName);

/* Find the next file */
                nResult = tfRamfsFindNextBlock(ramfsPtr,
                                               (ttVoidPtr)pResult,
                                               pblFoundEntry);

                if (nResult != TM_RAMFS_ERROR)
                {
                    tlLastError = TM_ENOERROR; /* Success */
                }

            }
        }
    }
    if (tlLastError != TM_ENOERROR)
    {
/* Failure */
        if (pResult != TM_VOID_NULL_PTR)
        {
            tm_free(pResult);
        }
        pResult = (ttVoidPtr)0;
    }
    return (ttVoidPtr)pResult;
}


/* tfRamfsFindNextBlock
 *
 *    Finds the next, if any, that matches the lpszFileName argument in
 *    a previous call to tfRamfsFindFirstBlock.
 *
 *    Parameters
 *        hFind : Search handle returned by a previous call to
 *            tfRamfsFindFirstBlock
 *        pblFoundEntry : Directory entry block address of the found file
 *
 *    Return Values
 *        TM_RAMFS_ERROR: An error as occured or no more matching(call
 *                      tfRamfsErrNo for more details)
 *        TM_RAMFS_OKAY : A matching was found
 */
static int tfRamfsFindNextBlock(ttRamfsDrivePtr ramfsPtr,
                                ttVoidPtr hFind,
                                ttRamfsBlockPtr pblFoundEntry)
{
    ttRamfsBlock                blNext;
    ttRamfsPrivateFindStructPtr pHandle;
    ttRamfsEntryPtr             pEntry;
    int                         nResult;

    tlLastError = TM_ENOERROR;

    pHandle = (ttRamfsPrivateFindStructPtr)hFind;
    nResult = TM_RAMFS_ERROR;

    blNext = *pHandle->rpfsPblNext;

    if (pHandle->rpfsLpszName[0] == '.')
    {
        if (pHandle->rpfsLpszName[1] == '\0')
        {
/* The name part is "." */
            *pblFoundEntry = pHandle->rpfsBlBaseDir;
            nResult = TM_RAMFS_OKAY;
        }
        else if ((pHandle->rpfsLpszName[1] == '.')
            && (pHandle->rpfsLpszName[2] == '\0'))
        {
/* The name part is ".." */
            if (tm_ramfs_getentry(ramfsPtr,
                    pHandle->rpfsBlBaseDir)->reBlParentEntry == TM_RAMFS_NOLINK)
            {
/* No parent directory */
                tlLastError = TM_ENOENT;
            }
            else
            {
                *pblFoundEntry = tm_ramfs_getentry(ramfsPtr,
                    pHandle->rpfsBlBaseDir)->reBlParentEntry;
                nResult = TM_RAMFS_OKAY;
            }
        }
    }

    if (pHandle->rpfsLpszName[0] == '\0')
    {
/* Root directory */
        if (pblFoundEntry != TM_VOID_NULL_PTR)
        {
            *pblFoundEntry = pHandle->rpfsBlBaseDir;
        }

        nResult = TM_RAMFS_OKAY;
    }
    else
    {
/* Parse each block of the directory entry */
        while ((blNext != TM_RAMFS_NOLINK) && (nResult == TM_RAMFS_ERROR)
            && (tlLastError == TM_ENOERROR))
        {
/* Get the next block pointer */
            pEntry = tm_ramfs_getentry(ramfsPtr, blNext);
            pHandle->rpfsPblNext = &tm_ramfs_nextblock(ramfsPtr, blNext);

            if (tfRamfsMatchFileName(pEntry->reSzName, pHandle->rpfsLpszName) == 0)
            {
/* The filename match with the mask */
                if (pblFoundEntry != TM_VOID_NULL_PTR)
                {
                    *pblFoundEntry = blNext;
                }

                nResult = TM_RAMFS_OKAY;
            }

            blNext = *pHandle->rpfsPblNext;
        }
    }

    if (nResult == TM_RAMFS_ERROR)
    {
        if (pblFoundEntry != TM_VOID_NULL_PTR)
        {
            *pblFoundEntry = TM_RAMFS_NOLINK;
        }

/* No file found */
        tlLastError = TM_ENOENT;
    }

    return nResult;
}


/* tfRamfsGetSizeFromBlock
 *
 *    Gets a entry size from its block count members.
 *
 *    Parameters
 *        blCount : blCount member of the ttRamfsEntry structure
 *        blLastBlockSize : blLastBlockSize member of the ttRamfsEntry structure
 *
 *    Return Values
 *        The computed size
 */
static ttRamfsSize tfRamfsGetSizeFromBlock(ttRamfsBlock blCount,
                                           ttRamfsBlock blLastBlockSize)
{
    ttRamfsSize size;
    if (blLastBlockSize == 0)
    {
        size = blCount * TM_RAMFS_BLOCKSIZE;
    }
    else
    {
        size = (blCount - 1) * TM_RAMFS_BLOCKSIZE + blLastBlockSize;
    }
    return size;
}


/* tfRamfsGetEntryInfo
 *
 *    Gets an entry info.
 *
 *    Parameters
 *        pFindInfo : Information structure pointer
 *        blEntry : Block address of the entry
 */
static void tfRamfsGetEntryInfo( ttRamfsDrivePtr    ramfsPtr,
                                 ttRamfsFindInfoPtr pFindInfo,
                                 ttRamfsBlock       blEntry )
{
    ttRamfsEntryPtr pEntry;

    pEntry = tm_ramfs_getentry(ramfsPtr, blEntry);
    pFindInfo->rfiUlFlags = pEntry->reUlFlags;
    pFindInfo->rfiBlEntry = blEntry;
    pFindInfo->rfiLSize =
        tfRamfsGetSizeFromBlock(pEntry->reBlCount, pEntry->reBlLastBlockSize);
    tm_strcpy(pFindInfo->rfiSzName, pEntry->reSzName);
    return;
}


/*
 * Function Description
 * Verifies whether a user is allowed on the system.
 *
 * Parameters
 *
 * Parameter       Description
 * userNamePtr     Pointer to a null terminated string containing the
 *                 user name.
 *
 * Returns
 * Value   Meaning
 * 0       Success
 * -1      User is not allowed on the system
 */
int tfFSUserAllowed (const char TM_FAR *  userNamePtr )
{
    int            maxEntries;
    int            retCode;
    int            i;

    retCode = -1;
    maxEntries = sizeof(tlFsNamePasswd)/sizeof(tlFsNamePasswd[0]);
    for (i = 0; i < maxEntries; i++)
    {
        if (    tm_strcmp(userNamePtr, tlFsNamePasswd[i].userName)
             == TM_STRCMP_EQUAL )
        {
            retCode = 0;
            break;
        }
    }

    return retCode;
}

/*
 * tfFSUserLogin Function Description
 * Given a User name string , and Password string, login the user
 * if the user is allowed, and password correct. Allocate a data structure
 * to store information unique to the just logged in user, such as its
 * current working directory.
 *
 * Parameters
 *
 * Parameter       Description
 * userNamePtr     Pointer to a null terminated string containing the
 *                 user name.
 * passwordPtr     Pointer to a null terminated string containing the
 *                 password.
 *
 * Returns
 * Value         Meaning
 * (void *)0     Failure.
 * userDataPtr   Pointer to a unique user data structure containing
 *               information about the given user (such as its working
 *               directory.)
 */

void TM_FAR * tfFSUserLogin ( const char TM_FAR *  userNamePtr,
                              const char TM_FAR *  passwordPtr )
{
    ttUserFsDataPtr   userFsDataPtr;
    int               maxEntries;
    int               i;
    int               retCode;

    userFsDataPtr = (ttUserFsDataPtr)0;
    retCode = -1;
    maxEntries = sizeof(tlFsNamePasswd)/sizeof(tlFsNamePasswd[0]);
    for (i = 0; i < maxEntries; i++)
    {
        if (    tm_strcmp(userNamePtr, tlFsNamePasswd[i].userName)
             == TM_STRCMP_EQUAL )
        {
            if (    (tlFsNamePasswd[i].userPassword[0] == '\0')
                 || (    tm_strcmp( passwordPtr,
                                    tlFsNamePasswd[i].userPassword )
                      == TM_STRCMP_EQUAL ) )
            {
                retCode = 0;
            }
            break;
        }
    }
    if (retCode == 0)
    {
        userFsDataPtr = (ttUserFsDataPtr)tm_get_raw_buffer(
                                                        sizeof(ttUserFsData));
        if (userFsDataPtr != (ttUserFsDataPtr)0)
        {
/* Zero the structure including the working directory */
            tm_bzero(userFsDataPtr, sizeof(ttUserFsData));
/* set the initial working directory */
            userFsDataPtr->userWorkingDirectory[0] = TM_RAMFS_DIRSEP;
        }
    }

    return (void TM_FAR *)userFsDataPtr;
}

/*
 * tfFSUserLogout Function Description
 * Given the unique user pointer as returned by tfFSUserLogin, log the
 * user out, free the structure pointed to by userDataPtr.
 *
 * Parameters
 * Parameter   Description
 * userDataPtr Pointer to user data structure as returned by tfFSUserLogin.
 * Returns
 * No return value.
 */
void tfFSUserLogout (void TM_FAR *  userDataPtr)
{
    if (userDataPtr != (ttVoidPtr)0)
    {
/* Lock the drive */
        tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

        tfFSUserDir(userDataPtr);

/* Unlock the drive */
        tm_call_unlock(&tm_global(tvRamfsLockEntry));

        tm_free_raw_buffer(userDataPtr);
    }

    return;
}

/*
 * tfFSSystem function description:
 * Copy into bufferPtr (up to bufferSize bytes), the official system name,
 * as assigned in the list of "OPERATING SYSTEM NAMES" in the
 * "Assigned Numbers" RFC (RFC 1700). For example the DOS operating system
 * has been assigned DOS as system name. If the file system is a DOS file
 * system, then this function should copy "DOS" into bufferPtr. If the
 * system has not been assigned a system name in the RFC, then this function
 * should return -1.
 * Parameters
 * Parameter    Description
 * bufferPtr    Pointer to a buffer where to copy the system name
 * bufferSize   size in bytes of the buffer.
 * Returns
 * Value        Meaning
 * -1           failure
 *  > 0         number of bytes copied.
 */
static const char TM_CONST_QLF tlRamSystem[] = "Windows_NT";

int tfFSSystem (char TM_FAR * bufferPtr, int bufferSize)
{
    unsigned int size;

    size = tm_min((unsigned)bufferSize, (sizeof(tlRamSystem) - 1));
    (void)tm_strncpy(bufferPtr, tlRamSystem, size);
    return size;
}

/*
 * tfFSStructureMount function description:
 * Given the unique user data pointer as returned by tfFSUserLogin, and
 * a file system, mount the user to the new file system.
 * Parameters
 * Parameter    Description
 * userDataPtr  Pointer to user data structure as returned by tfFsUserLogin.
 * pathNamePtr  Pointer to a null terminated string containing a file system
 *              name.
 * Returns
 * Value         Meaning
 *  0            success
 *  -1           failure
 */
int tfFSStructureMount ( void TM_FAR *      userDataPtr,
                         const char TM_FAR * pathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);

    return -1;
}

/*
 * tfFSRenameFile function description:
 * Given the unique user data pointer as returned by tfFSUserLogin, and
 * a current file name, and a new file name, rename the file to the new
 * file name.
 * Parameters
 * Parameter       Description
 * userDataPtr     Pointer to user data structure as returned by tfFsUserLogin
 * fromPathNamePtr Pointer to null terminated string containing current
 *                 file name
 * toPathNamePtr   Pointer to null terminated string containing new
 *                 file name
 *
 * Returns
 * Value         Meaning
 *  0            success
 *  -1           failure
 */
int tfFSRenameFile (const void TM_FAR * userDataPtr,
                    const char TM_FAR * fromPathNamePtr,
                    const char TM_FAR * toPathNamePtr )
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        tfFSUserDir(userFsDataPtr);
        retCode = tfRamfsRename(fromPathNamePtr, toPathNamePtr);
/*
 * Note: if need be, could store tlLastError in userFsDataPtr->userErrorCode
 * if userFsDataPtr is non null.
 */
    }
    else
    {
        retCode = -1;
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
#undef userFsDataPtr
}

/*
 * tfFSGetUniqueFileName function description:
 * Given the unique user data pointer as returned by tfFSUserLogin,
 * find and copy a unique file name that does not
 * conflict with any existing file name in the user working
 * directory in the buffer pointed to by bufferPtr, up to bufferSize bytes.
 * Returns -1 on failure, otherwise the number of copyied bytes.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFsUserLogin
 * bufferPtr     Pointer to buffer where to store the unique file name.
 * bufferSize    size in bytes of the buffer.
 * Returns
 * Value         Meaning
 *  -1           failure
 *  > 0          number of copyied bytes
 */
int tfFSGetUniqueFileName ( void TM_FAR * userDataPtr,
                            char TM_FAR * bufferPtr,
                            int           bufferSize )
{
    int     retCode;
    char  * tmpPtr;
    tt32Bit fileCounter;
    int     nameLen;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

    retCode = -1;

    nameLen = (int)tm_strlen(TM_UNIQUE_FILENAME);
    if(bufferSize > nameLen)
    {
        tm_strcpy(bufferPtr, TM_UNIQUE_FILENAME);
        tmpPtr = (char *)tm_strstr(bufferPtr, ".");
        if(tmpPtr != (char *)0)
        {
            nameLen = (int)(ttUserPtrCastToInt)(tmpPtr - bufferPtr);
        }
        fileCounter = 0;
        while(fileCounter < TM_UL(0x10000000))
        {
            tfPrintDecimal( bufferPtr, nameLen, (tt32Bit)fileCounter++);

            tfFSOpenDir(userDataPtr, bufferPtr, 0);

/* Lock the drive */
            tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

            tfFSUserDir(userFsDataPtr);

/* Unlock the drive */
            tm_call_unlock(&tm_global(tvRamfsLockEntry));

            if (userFsDataPtr->userDirPtr == (ttVoidPtr)0)
            {
                retCode = (int)tm_strlen(bufferPtr);
                break;
            }
        }
    }

    return retCode;
#undef userFsDataPtr
}

/*
 * tfFSDeleteFile function description:
 * Given the unique user data pointer as returned by tfFSUserLogin,
 * and a file name, delete the file.
 *
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFsUserLogin
 * pathNamePtr   Pointer to a null terminated string containing file name.
 * Returns
 * Value         Meaning
 *  0            success
 *  -1           failure
 */
int tfFSDeleteFile (const void TM_FAR * userDataPtr,
                    const char TM_FAR * pathNamePtr )
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        tfFSUserDir(userFsDataPtr);
        retCode = tfRamfsUnlink(pathNamePtr);
    }
    else
    {
        retCode = -1;
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
#undef userFsDataPtr
}

/*
 * tfFSOpenFile function description:
 * Given the unique user data pointer as returned by tfFSUserLogin,
 * and a file name, open the file for either read (if flag is
 * TM_FS_READ), write (if flag is TM_FS_WRITE), or apppend (if flag is
 * TM_FS_APPEND). Parameter type specifies if file type is ascii
 * (TM_TYPE_ASCII), or (TM_TYPE_BINARY). Parameter structure specifies if
 * the file structure is stream (TM_STRU_STREAM) or record
 * (TM_STRU_RECORD).). Allocate a file data structure to store
 * the file pointer, file type, file structure, etc..
 * Note: This call should fail if the file name is a directory.
 * Returns pointer to the newly allocated file structure on success,
 * null pointer on failure.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFsUserLogin
 * pathNamePtr   Pointer to a null terminated string containing file name.
 * flag          open flag: TM_FS_READ, TM_FS_WRITE, TM_FS_APPEND
 * type          file type: TM_TYPE_ASCII, or TM_TYPE_BINARY
 * structure     file structure: TM_STRU_RECORD, or TM_STRU_STREAM
 * Returns
 * Value             Meaning
 * (void TM_FAR *)0  failure
 * fileDataPtr       pointer to newly allocated file data structure.
 */

void TM_FAR * tfFSOpenFile (const void TM_FAR * userDataPtr,
                            const char TM_FAR * pathNamePtr,
                            int                 flag,
                            int                 type,
                            int                 structure )
{
    ttRamfsDrivePtr ramfsPtr;
    void TM_FAR   * fileHandle;
    tt32Bit         fileFlag;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

    TM_UNREF_IN_ARG(flag);
    TM_UNREF_IN_ARG(type);
    TM_UNREF_IN_ARG(structure);

    switch(flag)
    {
    case TM_FS_READ:
        fileFlag =  TM_RAMFS_ACCESS_READ |
                    TM_RAMFS_SHARE_READ  |
                    TM_RAMFS_TYPE_BINARY;
        break;
    case TM_FS_WRITE:
        fileFlag =  TM_RAMFS_ACCESS_WRITE |
                    TM_RAMFS_SHARE_WRITE  |
                    TM_RAMFS_OPEN_CREATE  |
                    TM_RAMFS_OPEN_TRUNC   |
                    TM_RAMFS_TYPE_BINARY;
        break;
    case TM_FS_APPEND:
        fileFlag =  TM_RAMFS_ACCESS_WRITE |
                    TM_RAMFS_OPEN_CREATE  |
                    TM_RAMFS_OPEN_APPEND  |
                    TM_RAMFS_TYPE_BINARY;
        break;
    default:
        fileFlag = 0; /* Avoid compiler warning */
        break;
    }

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        tfFSUserDir(userFsDataPtr);
        fileHandle = (void TM_FAR *)tfRamfsOpen(pathNamePtr, fileFlag);
    }
    else
    {
        fileHandle = (ttVoidPtr)0;
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return fileHandle;
#undef userFsDataPtr
}

/*
 * tfFSReadFile function description:
 * Given the unique user data pointer as returned by tfFSUserLogin,
 * and a file data pointer as returned by tfFSOpenFile, read up to
 * bufferSize in the buffer. Return the number of bytes actually read, 0 if
 * end of file has been reached, -1 on error.
 *
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * fileDataPtr   Pointer to file data structure as returned by tfFSOpenFile.
 * bufferPtr     Pointer to buffer where to copy the data from the file.
 * bufferSize    size in bytes of the buffer.
 *
 * Returns
 * Value         Meaning
 * > 0           number of copied bytes
 *  0            end of file
 *  -1           failure
 */
int tfFSReadFile( void TM_FAR * userDataPtr,
                  void TM_FAR * fileDataPtr,
                  char TM_FAR * bufferPtr,
                  int           bufferSize )
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;

    TM_UNREF_IN_ARG(userDataPtr);

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        retCode = (int)tfRamfsRead(fileDataPtr, bufferPtr, bufferSize);
    }
    else
    {
        retCode = -1;
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
}

/*
 * tfFSReadFileRecord function description:
 * Given the unique user data pointer as returned by tfFSUserLogin,
 * and a file data pointer as returned by tfFSOpenFile, read up to
 * bufferSize in the buffer, or end of record whichever comes first.
 * If end of record has been reached, store 1 in the integer pointed to
 * by eorPtr, otherwise store 0. If the file system does not support
 * records, then the system end of line (i.e <CR><LF> for DOS, <LF> for
 * Unix) is an end of record, and this routine should convert every end of
 * line (i.e <CR><LF> for DOS, <LF> for Unix) to an end of record, i.e
 * store 1 in eorPtr when the end of line character(s) have been read;
 * the end of line character(s) themselves should not be copied into
 * the buffer pointed to by bufferPtr.
 *
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * fileDataPtr   Pointer to file data structure as returned by tfFSOpenFile.
 * bufferPtr     Pointer to buffer where to copy the data from the file.
 * bufferSize    size in bytes of the buffer.
 * eorPtr        Pointer to end of record.
 * Returns
 * Value         Meaning
 * > 0           number of copied bytes (not including end of record)
 *  0            end of file has been reached
 *  -1           failure
 */
int tfFSReadFileRecord ( void TM_FAR * userDataPtr,
                         void TM_FAR * fileDataPtr,
                         char TM_FAR * bufferPtr,
                         int           bufferSize,
                         int  TM_FAR * eorPtr )
{
/* Not needed on DOS. We use the TM_FS_CRLF_FLAG */
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(fileDataPtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(bufferSize);
    TM_UNREF_IN_ARG(eorPtr);

    return -1;
}

/*
 * tfFSWriteFile function description:
 * Given the unique user data pointer as returned by tfFSUserLogin,
 * and a file data pointer as returned by tfFSOpenFile, write
 * bytes from the buffer pointed to by bufferPtr to the file.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * fileDataPtr   Pointer to file data structure as returned by tfFSOpenFile.
 * bufferPtr     Pointer to buffer data to copy into the file.
 * bytes         size in bytes of the data in the buffer.
 * Returns
 * Value         Meaning
 *  0            success
 *  -1           failure
 */
int tfFSWriteFile ( void TM_FAR *       userDataPtr,
                    void TM_FAR *       fileDataPtr,
                    const char TM_FAR * bufferPtr,
                    int                 bytes)
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;
    int             length;

    TM_UNREF_IN_ARG(userDataPtr);

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    retCode = -1;
    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        length = (int)tfRamfsWrite(fileDataPtr,bufferPtr,bytes);
        if (length == bytes)
        {
            retCode = 0;
        }
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
}

/*
 * tfFSWriteFileRecord function description:
 * Given the unique user data pointer as returned by tfFSUserLogin,
 * and a file data pointer as returned by tfFSOpenFile, write up to
 * bytes from the buffer, plus an end of record if eor is set to 1.
 * If the file system does not support records, then the system end of
 * line (i.e <CR><LF> for DOS, <LF> for Unix) should be written to the
 * file after all the bytes from the buffer have been written if eor is
 * set to 1.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * fileDataPtr   Pointer to file data structure as returned by tfFSOpenFile.
 * bufferPtr     Pointer to buffer data to copy into the file.
 * bytes         size in bytes of the data in the buffer.
 * eor           end of record (1 if end of record need to be written)
 * Returns
 * Value         Meaning
 *  0            success
 *  -1           failure
 */
int tfFSWriteFileRecord ( void TM_FAR * userDataPtr,
                          void TM_FAR * fileDataPtr,
                          char TM_FAR * bufferPtr,
                          int           bytes,
                          int           eor )
{
/* Not needed on DOS. We use the TM_FS_CRLF_FLAG */
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(fileDataPtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(bytes);
    TM_UNREF_IN_ARG(eor);

    return -1;
}

/*
 * tfFSFlushFile Function description:
 * Given a unique user data pointer as returned by tfFSUserLogin, and a
 * file data pointer as returned by tfFSOpenFile, flush the file.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * fileDataPtr   Pointer to file data structure as returned by tfFSOpenFile.
 * Returns
 * No return value.
 */
int tfFSFlushFile ( void TM_FAR * userDataPtr, void TM_FAR * fileDataPtr)
{
    ttRamfsDrivePtr ramfsPtr;

    TM_UNREF_IN_ARG(userDataPtr);

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        (void)tfRamfsFlush(fileDataPtr);
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return 0;
}


/*
 * tfFSCloseFile Function description:
 * Given a unique user data pointer as returned by tfFSUserLogin, and a
 * file data pointer as returned by tfFSOpenFile, close the file,
 * and free the file data structure pointed to by fileDataPtr.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * fileDataPtr   Pointer to file data structure as returned by tfFSOpenFile.
 * Returns
 * No return value.
 */
int tfFSCloseFile ( void TM_FAR * userDataPtr, void TM_FAR * fileDataPtr)
{
    ttRamfsDrivePtr ramfsPtr;

    TM_UNREF_IN_ARG(userDataPtr);

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        (void)tfRamfsLockedClose(fileDataPtr);
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return 0;
}

/*
 * tfFSChangeDir function description:
 * Given the unique user data pointer as returned by tfFSUserLogin, and
 * a directory name, change the user working directory to the new
 * directory.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * pathNamePtr   Pointer to a null terminated string containing directory
 *               path name.
 * Returns
 * Value         Meaning
 *  0            success
 *  -1           failure
 */
int tfFSChangeDir (void TM_FAR *        userDataPtr,
                   const char TM_FAR *  pathNamePtr )
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

/* This command is only valid if a user has logged in. */
    retCode = -1;
    if (userDataPtr != TM_VOID_NULL_PTR)
    {
        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
        if (ramfsPtr != (ttRamfsDrivePtr)0)
        {
            tfFSUserDir(userFsDataPtr);
            retCode = tfRamfsChDir(pathNamePtr);
            if (retCode != -1)
            {
                (void)tfRamfsGetCwd(userFsDataPtr->userWorkingDirectory,
                                    TM_FS_WORKINGDIR_SIZE);
            }
        }
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
#undef userFsDataPtr
}

/*
 * tfFSChangeParentDir function description:
 * Given the unique user data pointer as returned by tfFSUserLogin
 * change the user working directory to its parent directory.
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * Returns
 * Value         Meaning
 *  0            success
 *  -1           failure
 */
int tfFSChangeParentDir ( void TM_FAR * userDataPtr )
{
    int retCode;

    if (userDataPtr != TM_VOID_NULL_PTR)
    {
        retCode = tfFSChangeDir(userDataPtr, "..");
    }
    else
    {
/* This command is not supported if a user has not logged in. */
        retCode = -1;
    }

    return retCode;
}

/*
 * tfFSRemoveDir function description:
 * Given the unique user data pointer as returned by tfFSUserLogin, and
 * a directory name, remove the directory.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * pathNamePtr   Pointer to a null terminated string containing directory
 *               path name.
 * Returns
 * Value         Meaning
 *  0            success
 *  -1           failure
 */
int tfFSRemoveDir(const void TM_FAR * userDataPtr,
                  const char TM_FAR * pathNamePtr )
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
/* Point to current user directory */
        tfFSUserDir(userFsDataPtr);
        retCode = tfRamfsRmDir(pathNamePtr);
    }
    else
    {
        retCode = -1;
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
#undef userFsDataPtr
}

/*
 * tfFSMakeDir function description:
 * Given the unique user data pointer as returned by tfFSUserLogin, and
 * a directory name, create the directory. If successfull, copy the
 * directory path name in bufferPtr (up to bufferSize). The directory
 * path name could be either absolute or relative to the user working
 * directory path, but should be such that a subsequent tfFSChangeDir
 * with that pathname as an argument should not fail.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * pathNamePtr   Pointer to a null terminated string containing directory
 *               path name.
 * bufferPtr     pointer to a buffer where to copy the pathname of the newly
 *               created directory.
 * bufferSize    size in bytes of the buffer.
 * Returns
 * Value         Meaning
 *  -1           failure
 *  > 0          number of bytes copyied into the buffer pointed to by
 *               bufferPtr.
 */
int tfFSMakeDir (const void TM_FAR *    userDataPtr,
                 const char TM_FAR *    pathNamePtr,
                 char TM_FAR *          bufferPtr,
                 int                    bufferSize )
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
/* Point to current user directory */
        tfFSUserDir(userFsDataPtr);
        retCode = tfRamfsMkDir(pathNamePtr);
        if (retCode == 0)
        {
/* created directory, copy path */
            retCode = (int)tm_strlen(pathNamePtr);
            if (retCode >= bufferSize)
            {
                retCode = bufferSize - 1;
            }
            tm_bcopy(pathNamePtr, bufferPtr, retCode + 1);
            bufferPtr[retCode] = '\0';
        }
    }
    else
    {
        retCode = -1;
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
#undef userFsDataPtr
}

/*
 * tfFSGetWorkingDir function description:
 * Given the unique user data pointer as returned by tfFSUserLogin, copy
 * the user working directory in the buffer pointed to by bufferPtr (up
 * to bufferSize).
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * bufferPtr     pointer to a buffer where to copy the pathname of the user
 *               working directory.
 * bufferSize    size in bytes of the buffer.
 * Returns
 * Value         Meaning
 *  -1           failure
 *  > 0          number of bytes copyied into the buffer pointed to by
 *               bufferPtr.
 */
int tfFSGetWorkingDir ( const void TM_FAR * userDataPtr,
                        char TM_FAR *       bufferPtr,
                        int                 bufferSize )
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

/* Lock the drive */
    tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

    retCode = -1;
    if (bufferPtr != (char TM_FAR *) 0)
    {
        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
        if (ramfsPtr != (ttRamfsDrivePtr)0)
        {
/* Point to current user directory */
            tfFSUserDir(userFsDataPtr);
            retCode = tfRamfsGetCwd(bufferPtr, bufferSize);
            if (retCode != TM_RAMFS_ERROR)
            {
                bufferPtr[bufferSize - 1] = '\0';
                retCode = (int)tm_strlen(bufferPtr);
            }
        }
    }

/* Unlock the drive */
    tm_call_unlock(&tm_global(tvRamfsLockEntry));

    return retCode;
#undef userFsDataPtr
}


/*
 * tfFSOpenDir function description:
 * Given the unique user data pointer as returned by tfFSUserLogin, and
 * a pointer to a path name, and a flag, open the directory corresponding
 * to path for reading either a long directory (flag == TM_DIR_LONG) or
 * short directory (TM_DIR_SHORT). Subsequent calls to tfFSGetNextDirEntry
 * will fetch each entry in the directory matching the pattern as pointed to
 * by pathNamePtr.
 * The tfFSOpenDir implementor should allocate a directory data structure
 * to keep track of the path name matching pattern, the reading position
 * in the directory, and the directory read flag (TM_DIR_LONG or
 * TM_DIR_SHORT). Note that if pathNamePtr points to a directory name,
 * then the matching pattern is "*.*". If pathNamePtr points to "*.*",
 * then the user working directory should be open, and the matching pattern
 * is "*.*".
 * Returns a pointer to a just allocated directory structure pointer on
 * success, null pointer on failure.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * pathNamePtr   Pointer to a null terminated string containing directory
 *               path name, or matching pattern path name.
 * flag          Either TM_DIR_LONG or TM_DIR_SHORT
 * Returns
 * Value            Meaning
 * (void TM_FAR*)0  failure
 * dirDataPtr       Pointer to newly allocated directory data structure
 */

void TM_FAR * tfFSOpenDir ( void TM_FAR *       userDataPtr,
                            const char TM_FAR * pathNamePtr,
                            int                 flag )
{
    ttRamfsDrivePtr     ramfsPtr;
    ttVoidPtr           dirPtr;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

/* This command is not supported if a user has not logged in. */
    dirPtr = TM_VOID_NULL_PTR;
    if (userDataPtr != TM_VOID_NULL_PTR)
    {
/* Lock the drive */
        tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
        if (ramfsPtr != (ttRamfsDrivePtr)0)
        {
            if (   (pathNamePtr == (const char TM_FAR *)0)
                || (*pathNamePtr == 0) )
            {
                pathNamePtr = "*.*";
            }
            tfFSUserDir(userFsDataPtr);
            dirPtr = tfRamfsOpenDir(userFsDataPtr, pathNamePtr, flag);
        }

/* Unlock the drive */
        tm_call_unlock(&tm_global(tvRamfsLockEntry));
    }

    return dirPtr;
#undef userFsDataPtr
}

static ttVoidPtr tfRamfsOpenDir (ttUserFsDataPtr     userFsDataPtr,
                                 const char TM_FAR * pathNamePtr,
                                 int                 flag)
{
    ttRamfsDrivePtr     ramfsPtr;
    ttVoidPtr           dirPtr;
    ttCharPtr           lpszName;
    ttRamfsBlock        blBaseDir;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
/* Directory handle */
    dirPtr = tfRamfsFindFirstFile( pathNamePtr,
                                   &userFsDataPtr->userDirEntry );
    if (dirPtr == (ttVoidPtr)0)
    {
        userFsDataPtr->userDirPtr = (ttVoidPtr)0;
    }
    else
    {
        userFsDataPtr->userDirHandle = dirPtr;
        userFsDataPtr->userDirPtr =
                                 (ttVoidPtr)&userFsDataPtr->userDirEntry;
        userFsDataPtr->userDirFlag = flag;
    }
/* Directory Entry Pointer */
    dirPtr = userFsDataPtr->userDirPtr;

    if (dirPtr == (ttVoidPtr)0)
    {
/* Either the directory is invalid or empty */
        if (tfRamfsSplitPath( ramfsPtr,
                          pathNamePtr,
                          (ttConstCharPtr TM_FAR *)&lpszName,
                          &blBaseDir) == TM_RAMFS_OKAY)
        {
/* We have an empty file system (if blBaseDir == 0) or just an empty
 * directory.  Either way we need to return an non-null value for
 * dirPtr.  This will work since we only use userFsDataPtr->userDirPtr
 * and it has been set.
*/
            dirPtr = ramfsPtr;
        }
    }
    return dirPtr;
}

/*
 * tfFSGetNextDirEntry function description:
 * Given the unique user data pointer as returned by tfFSUserLogin, and
 * a directory data pointer as returned by tfFSOpenDir, get the next
 * entry in the directory that matches the path given as an argument to
 * tfFSOpenDir. The next entry should be either a long listing
 * of the next directory entry (either volume name, sub-directory,
 * or file name with its attribute), or a short listing of the directory
 * entry (file name only without any attribute), and should be stored in
 * the buffer pointed to by bufferPtr up to bufferSize.
 * The FTP server will keep on calling tfFSGetNextDirEntry until it returns
 * 0 bytes, to read all the matching directory entries. It is up to the
 * implementor of tfFSGetNextDirEntry to keep track of how many entries of
 * the listing have been read so far, and to keep track of whether the
 * directory was open for long or short listing, and to keep track of the
 * matching pattern (using dirDataPtr).
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * dirDataPtr    Pointer to newly allocated directory data structure
 * bufferPtr     pointer to a buffer where to copy the next directory entry
 * bufferSize    size in bytes of the buffer.
 *
 * Returns
 * Value         Meaning
 *  -1           failure
 *  > 0          number of bytes copyied into the buffer pointed to by
 *               bufferPtr.
 *   0           end of directory.
 */
int tfFSGetNextDirEntry ( void TM_FAR * userDataPtr,
                          void TM_FAR * dirDataPtr,
                          char TM_FAR * bufferPtr,
                          int           bufferSize )
{
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;
    ttRamfsSize     fileSize;
    int             numDigits;


#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)
#define userFsDirPtr  ((ttDirEntryPtr)dirDataPtr)
/* This command is only supported if a user has logged in. */
    retCode = -1;
    fileSize = (ttRamfsSize)0;
    numDigits = 0;
    if (   (userDataPtr != TM_VOID_NULL_PTR)
        && ((unsigned)bufferSize >= TM_RAMFS_MAXFNAME + TM_FILE_POS + 1))
    {
/* Lock the drive */
        tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
        if (ramfsPtr != (ttRamfsDrivePtr)0)
        {
            retCode = 0; /* End of directory */
            tfFSUserDir(userFsDataPtr);
            if (userFsDataPtr->userDirPtr != (ttVoidPtr)0)
            {
/*
 * Pass over '.', or '..' for TM_DIR_LONG.
*/
                while (tm_fs_is_dir(userFsDirPtr) &&
                        userFsDataPtr->userDirFlag == TM_DIR_LONG)
                {
                    if (    (  tm_strcmp(
                                    tm_fs_file_name_ptr(userFsDirPtr), ".")
                            != TM_STRCMP_EQUAL)
                         && (  tm_strcmp(
                                   tm_fs_file_name_ptr(userFsDirPtr), "..")
                            != TM_STRCMP_EQUAL) )
                    {
                        break;
                    }
                    if (    tfRamfsFindNextFile( userFsDataPtr->userDirHandle,
                                             userFsDirPtr)
                         == TM_RAMFS_ERROR )
                    {
                        (void)tfRamfsFindClose(userFsDataPtr->userDirHandle);

                        userFsDataPtr->userDirPtr = (ttVoidPtr)0;
                        break;
                    }
                }
                if (userFsDataPtr->userDirPtr != (ttVoidPtr)0)
                {
                    if (userFsDataPtr->userDirFlag == TM_DIR_LONG)
                    {
                        tm_memset(bufferPtr, ' ', (unsigned)bufferSize);
                        retCode = (int)
                                tm_strlen(tm_fs_file_name_ptr(userFsDirPtr));
                        if (retCode > 0)
                        {
/* We do not track the file creation date and time */
/* Insert bogus date and time */
                            tm_bcopy( TM_BOGUS_DATE,
                                      &bufferPtr[0],
                                      tm_strlen(TM_BOGUS_DATE));
                            tm_bcopy( tm_fs_file_name_ptr(userFsDirPtr),
                                      &bufferPtr[TM_FILE_POS],
                                      retCode);
                            if ( tm_fs_is_dir(userFsDirPtr) )
                            {
                                tm_bcopy("<DIR>", &bufferPtr[TM_SIZE_POS], 5);
                            }
                            else
                            {
/* List the size of the file */
                                fileSize = tm_fs_file_size(userFsDirPtr);
                                while (fileSize > (ttRamfsSize)0)
                                {
                                    numDigits++;
                                    fileSize /= 10;
                                }
                                fileSize = tm_fs_file_size(userFsDirPtr);
                                tfPrintDecimal(&bufferPtr
                                                [TM_FILE_POS-(numDigits+1)],
                                                numDigits,
                                               (tt32Bit)fileSize);
                            }
                            retCode += TM_FILE_POS;
                        }
                    } /* if (userFsDataPtr->userDirFlag == TM_DIR_LONG) */
                    else if (userFsDataPtr->userDirFlag == TM_DIR_SHORT)
                    {
                        tm_memset(bufferPtr, ' ', (unsigned)bufferSize);
                        retCode = (int)
                                    tm_strlen(tm_fs_file_name_ptr(userFsDirPtr));
                        tm_bcopy( tm_fs_file_name_ptr(userFsDirPtr),
                                  bufferPtr,
                                  retCode);
                    } /* if (userFsDataPtr->userDirFlag == TM_DIR_SHORT) */

                    bufferPtr[retCode] = '\0';
/* Point to next directory entry */
                    if (    tfRamfsFindNextFile( userFsDataPtr->userDirHandle,
                                                 userFsDirPtr)
                            == TM_RAMFS_ERROR )
                    {
                        (void)tfRamfsFindClose(userFsDataPtr->userDirHandle);

                        userFsDataPtr->userDirPtr = (ttVoidPtr)0;
                    }
                }
            }
        }

/* Unlock the drive */
        tm_call_unlock(&tm_global(tvRamfsLockEntry));
    }
#undef userFsDataPtr
#undef userFsDirPtr
    return retCode;
}


/*
 * tfFSCloseDir function description:
 * Given a unique user data pointer as returned by tfFSUserLogin, and a
 * directory data pointer as returned by tfFSOpenDir, close the directory,
 * and free the directory data structure pointed to by dirDataPtr.
 * Parameters
 * Parameter     Description
 * userDataPtr   Pointer to user data structure as returned by tfFSUserLogin
 * dirDataPtr    Pointer to newly allocated directory data structure
 *
 * Returns
 * no return value
 */
void tfFSCloseDir ( void TM_FAR * userDataPtr, void TM_FAR * dirDataPtr )
{
    ttRamfsDrivePtr ramfsPtr;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

    TM_UNREF_IN_ARG(dirDataPtr);
    if (userDataPtr != TM_VOID_NULL_PTR)
    {
/* Lock the drive */
        tm_call_lock_wait(&tm_global(tvRamfsLockEntry));

        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
        if (ramfsPtr != (ttRamfsDrivePtr)0)
        {
            if (userFsDataPtr->userDirPtr != (ttVoidPtr)0)
            {
                (void)tfRamfsFindClose(userFsDataPtr->userDirHandle);
                userFsDataPtr->userDirPtr = (ttVoidPtr)0;
            }
        }

/* Unlock the drive */
        tm_call_unlock(&tm_global(tvRamfsLockEntry));
    }
    return;
#undef userFsDataPtr
}

static void tfFSUserDir(ttConstUserFsDataPtr userFsDataPtr)
{
    if (userFsDataPtr != (ttConstUserFsDataPtr)0)
    {
        (void)tfRamfsChDir(userFsDataPtr->userWorkingDirectory);
    }
}

/*
 * tfPrintDecimal
 * Convert the hex number to decimal number and write it to the buffer in
 * ASCII format.
 *
 * PARAMETERS:
 *   bufPtr     Output buffer pointer
 *   bufLen     Output buffer length.
 *   number     The number to be printed.
 */
static void tfPrintDecimal(ttCharPtr bufPtr, int bufLen, tt32Bit number)
{
    int     i;
    char    tmpChar;

    for(i = 1; i <= bufLen; i++)
    {
        tmpChar = (char)(number % 10);
        number = number / 10;
        *(bufPtr + bufLen - i) = (char)(tmpChar + '0');
        if(number == 0)
        {
            break;
        }
    }

    return;
}

int tfFSFileExists(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr )
{
    int             errorCode;
    ttRamfsFindInfo findInfo;
    ttVoidPtr       hFind;

    TM_UNREF_IN_ARG(userDataPtr);

    if (pathNamePtr == (const void TM_FAR *)0)
    {
        errorCode = TM_EINVAL;
    }
    else if ((hFind = tfRamfsFindFirstFile(pathNamePtr, &findInfo))
             != (ttVoidPtr)0)
    {
        (void)tfRamfsFindClose(hFind);
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_ENOENT;
    }
    return errorCode;
}



ttUser32Bit tfFSGetFileLen(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr )
{
    ttRamfsSize     fileLen;
    ttRamfsFindInfo findInfo;
    ttVoidPtr       hFind;

    TM_UNREF_IN_ARG(userDataPtr);

    if (pathNamePtr == (const void TM_FAR *)0)
    {
        fileLen = 0;
        tlLastError = TM_EINVAL;
    }
    else if ((hFind = tfRamfsFindFirstFile(pathNamePtr, &findInfo))
             != (ttVoidPtr)0)
    {
        (void)tfRamfsFindClose(hFind);
        fileLen = findInfo.rfiLSize;
        tlLastError = TM_ENOERROR;
    }
    else
    {
        fileLen = 0;
        tlLastError = TM_ENOENT;
    }

    return fileLen;
}

/* This interface function expects the user to provide the most efficient
 * file block size. Normally it would be the cluster size of the
 * underlying file system. This is used to optimize the file read and
 * write performance. If returns zero, the default size
 * TM_FS_FILE_BLOCK_SIZE (512) will be used by the stack.
 */
int tfFSGetFileBlockSize(void)
{
    return 0;
}



/* Seek to a file position */
int tfFSSeekFile(
    void TM_FAR *   userDataPtr,
    void TM_FAR *   fileDataPtr,
    long            offset,
    int             whence)
{
    int                 errorCode;
    ttRamfsSize         position;
    ttRamfsSize         fileLen;

    TM_UNREF_IN_ARG(userDataPtr);
    errorCode = TM_ENOERROR;

    if (fileDataPtr == (void TM_FAR *)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {

/* Get the RAM file len */
        fileLen = tfRamfsFileLength(fileDataPtr);

        switch (whence)
        {
        case TM_RAMFS_SEEK_SET:
            position = (ttRamfsSize)offset;
            break;
        case TM_RAMFS_SEEK_CUR:
            position =   tm_ramfs_fhandle(fileDataPtr)->rfhLPosition
                       + (ttRamfsSize)offset;
            break;
        case TM_RAMFS_SEEK_END:
            position = fileLen + (ttRamfsSize)offset;
            break;
        default:
            position = (ttRamfsSize)0;
            break;
        }

        if ((position < (ttRamfsSize)0) || (position > fileLen))
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            tm_ramfs_fhandle(fileDataPtr)->rfhLPosition = position;
        }
    }
    return errorCode;
}

#else /* !TM_USE_RAM_FS */

LINT_UNUSED_HEADER
/* To allow link for builds when TM_IGMP is not defined */
int tvRamFsDummy = 0;

#endif /* !TM_USE_RAM_FS */
