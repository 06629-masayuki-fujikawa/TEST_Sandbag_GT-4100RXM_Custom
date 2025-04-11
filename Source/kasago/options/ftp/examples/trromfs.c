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
 * Description: Treck Rom File System
 * Filename: trromfs.c
 * Author: Xiangqian Wang
 * Date Created: 4/22/02
 * $Source: source/trromfs.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:48:23JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#ifdef TM_USE_ROM_FS

#include <trrombld.h>

static int tfRomFsFindFile(const char TM_FAR * pathNamePtr);

typedef struct tsRomFileDesc
{
    ttRomFilePtr        romFilePtr;
    tt32Bit             position;
}ttRomFileDesc;
typedef ttRomFileDesc TM_FAR * ttRomFileDescPtr;

int tfFSUserAllowed(const char TM_FAR * userNamePtr)
{
/* Allowed */
    TM_UNREF_IN_ARG(userNamePtr);
    return TM_ENOERROR;
}

void TM_FAR * tfFSUserLogin(
    const char TM_FAR * userNamePtr,
    const char TM_FAR * passwordPtr )
{
    TM_UNREF_IN_ARG(userNamePtr);
    TM_UNREF_IN_ARG(passwordPtr);
/* Allowing everybody to log in */
    return (ttVoidPtr)0x0a0a0a0a;
}

void tfFSUserLogout(void TM_FAR *  userDataPtr)
{
    TM_UNREF_IN_ARG(userDataPtr);
}

int tfFSSystem(
    char TM_FAR * bufferPtr,
    int bufferSize)
{
    unsigned int size;

    size = tm_min((unsigned int)bufferSize, (unsigned int)tm_strlen("ROM"));
    (void)tm_strncpy(bufferPtr, "ROM", size);
    return (int)size;
}

int tfFSStructureMount(
    void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);

    return -1;
}

int tfFSRenameFile(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * fromPathNamePtr,
    const char TM_FAR * toPathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(fromPathNamePtr);
    TM_UNREF_IN_ARG(toPathNamePtr);

    return -1;
}

int tfFSGetUniqueFileName(
    void TM_FAR * userDataPtr,
    char TM_FAR * bufferPtr,
    int           bufferSize )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(bufferSize);

    return -1;
}

int tfFSDeleteFile(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);

    return -1;
}




/* Find the index of a file by path name */
/* TBD: to use hash array for faster locating */

static int tfRomFsFindFile(const char TM_FAR * pathNamePtr)
{
    int fileId;
    int i;

    fileId = TM_SOCKET_ERROR;
    for (i = 0; i< tlRomFiles; i++)
    {
        if (!tm_strcmp(tlRomFile[i].romFileNamePtr, pathNamePtr))
        {
/* Found the file , TBD: to be improved to use the hash array for searching*/
            fileId = i;
            break;
        }
    }

    return fileId;
}

ttUser32Bit tfFSGetFileLen(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr )
{
    int     fileId;
    tt32Bit fileLen;
    TM_UNREF_IN_ARG(userDataPtr);

    fileId = tfRomFsFindFile(pathNamePtr);
    if(fileId != TM_SOCKET_ERROR)
    {
        fileLen = tlRomFile[fileId].romFileLen;
    }
    else
    {
        fileLen = 0;
    }

    return fileLen;
}

int tfFSFileExists(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr )
{
    int errorCode;

    TM_UNREF_IN_ARG(userDataPtr);

    if (pathNamePtr == (const void TM_FAR *)0)
    {
        errorCode = TM_EINVAL;
    }
    else if (tfRomFsFindFile(pathNamePtr) != TM_SOCKET_ERROR)
    {
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_ENOENT;
    }

    return errorCode;
}


/*
 * flag      TM_FS_READ, TM_FS_WRITE, TM_FS_APPEND,
 * type      TM_TYPE_ASCII, TM_TYPE_BINARY,
 * structure TM_STRU_STREAM, TM_STRU_RECORD
 */
void TM_FAR * tfFSOpenFile(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr,
    int                 flag,
    int                 type,
    int                 structure )
{
    ttRomFileDescPtr romFileDescPtr;
    int              fileId;

    romFileDescPtr = (ttRomFileDescPtr)0;
    TM_UNREF_IN_ARG(userDataPtr);
    type        = type;
    structure   = structure;

    if (flag == TM_FS_READ)
    {
        fileId = tfRomFsFindFile(pathNamePtr);
        if(fileId != TM_SOCKET_ERROR)
        {
            romFileDescPtr = tm_get_raw_buffer(sizeof(ttRomFileDesc));
            romFileDescPtr->romFilePtr = &tlRomFile[fileId];
            romFileDescPtr->position = 0;
        }
    }

    return (void TM_FAR *)romFileDescPtr;
}


/* Returns
 * Value         Meaning
 * > 0           number of copied bytes
 *  0            end of file
 *  -1           failure
 */
int tfFSReadFile(
    void TM_FAR * userDataPtr,
    void TM_FAR * fileDataPtr,
    char TM_FAR * bufferPtr,
    int           bytes )
{
    ttRomFileDescPtr        fileDescPtr;
    ttRomFilePtr            filePtr; /* Pointer to the ROM file descriptor */
    unsigned int            fileLen; /* remaining length of the file */
    const unsigned char *   readPtr; /* pointer to the location for read */
    const unsigned char *   pagePtr; /* pointer to the start of the page*/
    tt32Bit                 pageLen; /* remaining len the current page */
    tt32Bit                 position;
    int                     readLen;
    int                     pageReadLen;
    int                     skipLen;


    TM_UNREF_IN_ARG(userDataPtr);
    readPtr = (const unsigned char *)0;
    pageLen = 0;

/* Get the rom file descriptor Pointer */
    fileDescPtr= (ttRomFileDescPtr)fileDataPtr;

/* Get the ROM file pointer and the len */
    filePtr = fileDescPtr->romFilePtr;
    fileLen = filePtr->romFileLen;

/* Make sure we don't go beyond the file's location */
    tm_assert(tfFSReadFile, fileDescPtr->position <= fileLen);

/* Find the position for read */
    position = 0;
    do
    {
/* The length of the part of file on this page */
        pageLen = tm_min(tlRomPageSize - filePtr->romFilePageOffset, fileLen);

        if (position + pageLen < fileDescPtr->position)
        {
/* Go to next page */
            filePtr ++;
/* update positoin */
            position += pageLen;
            fileLen -=  pageLen;
        }
        else
        {
/* The read position is in this page */
            pagePtr =  filePtr->romFilePagePtr + filePtr->romFilePageOffset;
            skipLen =  fileDescPtr->position - position;
            readPtr =  pagePtr + skipLen;
            fileLen -= skipLen;
            pageLen -= skipLen;
            break;
        }
    }while (position < fileDescPtr->position);

/*    filePtr, fileLen, pageLen, readPtr, => bufferPtr, bytes */

/* We can't read more than we have in the file */
    if (bytes > (int)fileLen)
    {
        bytes = fileLen;
    }

    readLen = 0;
    while(readLen < bytes)
    {
        pageReadLen = tm_min((tt32Bit)(bytes - readLen), pageLen);

        tm_memcpy(bufferPtr, readPtr, pageReadLen);
        bufferPtr += pageReadLen;
        readLen += pageReadLen;

        if (readLen == bytes)
        {
/* We've finished reading */
            break;
        }
        else
        {
/* Go to next page */
            filePtr ++;
            pageLen = filePtr->romFileLen;
            readPtr = filePtr->romFilePagePtr + filePtr->romFilePageOffset;
        }
    }

    fileDescPtr->position += readLen;

    return readLen;
}

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

int tfFSWriteFile(
    void TM_FAR *       userDataPtr,
    void TM_FAR *       fileDataPtr,
    const char TM_FAR * bufferPtr,
    int                 bytes )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(fileDataPtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(bytes);
    return -1;
}

int tfFSWriteFileRecord(
    void TM_FAR * userDataPtr,
    void TM_FAR * fileDataPtr,
    char TM_FAR * bufferPtr,
    int           bytes,
    int           eor )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(fileDataPtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(bytes);
    TM_UNREF_IN_ARG(eor);
    return -1;
}

int tfFSFlushFile(
    void TM_FAR * userDataPtr,
    void TM_FAR * fileDataPtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(fileDataPtr);
    return -1;
}

int tfFSCloseFile(
    void TM_FAR * userDataPtr,
    void TM_FAR * fileDataPtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    tm_free_raw_buffer(fileDataPtr);
    return TM_ENOERROR;
}

/* Flag is either TM_DIR_SHORT or TM_DIR_LONG */
void TM_FAR * tfFSOpenDir(
    void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr,
    int           flag )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);
    TM_UNREF_IN_ARG(flag);

    return (void TM_FAR *)0;
}

void tfFSCloseDir(
    void TM_FAR * userDataPtr,
    void TM_FAR * dirDataPtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(dirDataPtr);
}

int tfFSChangeDir(
    void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);

    return -1;
}

int tfFSChangeParentDir(void TM_FAR * userDataPtr)
{
    TM_UNREF_IN_ARG(userDataPtr);
    return -1;
}

int tfFSRemoveDir(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);

    return -1;
}

int tfFSMakeDir(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr,
    char TM_FAR *       bufferPtr,
    int                 bufferSize )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(bufferSize);

    return -1;
}

int tfFSGetWorkingDir(
    const void TM_FAR * userDataPtr,
    char TM_FAR *       bufferPtr,
    int                 bufferSize )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(bufferSize);

    (void)tm_strncpy(bufferPtr, "/", bufferSize);
    return tm_min((int)tm_strlen("/"), (int)bufferSize);
}

int tfFSGetNextDirEntry(
    void TM_FAR * userDataPtr,
    void TM_FAR * dirDataPtr,
    char TM_FAR * bufferPtr,
    int           maxBytes )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(dirDataPtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(maxBytes);

    return -1;
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
    ttRomFileDescPtr    fileDescPtr;
    int                 errorCode;
    tt32Bit             position;
    tt32Bit             fileLen;

    TM_UNREF_IN_ARG(userDataPtr);
    errorCode = TM_ENOERROR;

    if (fileDataPtr == (void TM_FAR *)0)
    {
        errorCode = TM_EINVAL;
    }
    else 
    {
/* Get the rom file descriptor Pointer */
        fileDescPtr= (ttRomFileDescPtr)fileDataPtr;

/* Get the ROM file len */
        fileLen = fileDescPtr->romFilePtr->romFileLen;

        switch (whence)
        {
        case TM_FS_SEEK_SET:
            position = offset;
            break;
        case TM_FS_SEEK_CUR:
            position = fileDescPtr->position + offset;
            break;
        case TM_FS_SEEK_END:
            position = fileLen + offset;
            break;
        default:
            position = 0;
            break;
        }

        if ((position < 0) || (position > fileLen))
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            fileDescPtr->position = position;
        }
    }
    return errorCode;
}


#endif /* TM_USE_ROM_FS */
