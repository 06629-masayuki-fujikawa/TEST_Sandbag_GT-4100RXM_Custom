/*
 * Copyright Notice:
 * Copyright Treck Incorporated  1997 - 2016
 * Copyright Zuken Elmic Japan   1997 - 2016
 *No portions or the complete contents of this file may be copied or
 * distributed in any form (including but not limited to printed or electronic
 * forms) without the expressed written consent of Treck Incorporated OR
 * Zuken Elmic.  Copyright laws and International Treaties protect the
 * contents of this file.  Unauthorized use is prohibited.
 * All rights reserved.
 *
 * Description:  File system interface for DOS/Windows
 * Filename:     txfs.c
 * Author:       Odile
 * Date Created: 12/17/98
 * $Source: examples/txdosfs.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:31:06JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
/* 
 * This file provides example implementation of file system interface from the
 * Turbo Treck stack to the DOS or WINDOWS file system. 
 * Note that the interface to the Turbo Treck RAM file system, or Linux 
 * file system can be found as noted below.
 *      RAM file system     Implemented in trramfs.c
 *      Linux file system   Implemented in txlinuxa.c
 */
/* DOS file system interface */

#include <dos.h>
#include <windows.h>

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_DOS_FS

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#include <direct.h>
#include <io.h>         /* for rename() */
#include <stdio.h>      /* for fopen */
#include <shlwapi.h>    /* for PathCanonicalize, StrStrI*/

#include <time.h>       /* Time functions for directory listings */

/*
 * Local macro definitions
 */
/* Unique file name format */
#define TM_UNIQUE_FILENAME      "F0000000.trk"

/* This entry is a directory */
#define tm_fs_is_dir(fsDirPtr) (fsDirPtr->ff_attrib & _A_SUBDIR)

/* Get the file name pointer of the entry */
#define tm_fs_file_name_ptr(fsDirPtr) (fsDirPtr->ff_name)

/* Get the file size of the entry */
#define tm_fs_file_size(fsDirPtr) (fsDirPtr->size)

/* Get the file time/date */
#define tm_fs_file_time(fsDirPtr) (fsDirPtr->ff_time)

#ifdef _MSC_VER

#if     _MSC_VER >= 800
#define tm_getcwd    _getcwd
#define tm_chdir     _chdir
#define tm_rmdir     _rmdir
#define tm_mkdir     _mkdir
#define ttDirEntry   struct _finddata_t
#define tm_findfirst( pathNamePtr, dirPtr, attribute ) \
                     _findfirst( pathNamePtr, dirPtr)
#define tm_findnext( handle, dirPtr ) \
                     _findnext( handle, dirPtr )
#define ff_name      name
#define ff_attrib    attrib
#define ff_time      time_write
#endif /*  _MSC_VER >= 800 */

#endif

#ifndef ttDirEntry
#define ttDirEntry   struct ffblk
#endif /* ttDirEntry */

typedef ttDirEntry TM_FAR * ttDirEntryPtr;

#ifndef tm_findfirst
#define tm_findfirst( pathNamePtr, dirPtr, attribute ) \
                     findfirst(pathNamePtr, dirPtr, attribute)
#endif /* tm_findfirst */

#ifndef tm_findnext
#define tm_findnext( handle, dirPtr ) \
                     findnext(dirPtr)
#endif /* tm_findnext */

#ifndef tm_getcwd
#define tm_getcwd    getcwd
#endif /* tm_getcwd */

#ifndef tm_chdir
#define tm_chdir     chdir
#endif /* tm_chdir */

#ifndef tm_rmdir
#define tm_rmdir     rmdir
#endif /* tm_rmdir */

#ifndef tm_mkdir
#define tm_mkdir     mkdir
#endif /* tm_mkdir */

#define tm_chdrive   _chdrive
#define tm_getdrive  _getdrive

#define tm_rename    rename
#define tm_remove    remove

#define tm_fopen(fh,pchParam)       \
        fopen(fh,pchParam)

#define tm_fread(bufPtr,size,count,filePtr) \
        fread(bufPtr,(size_t)size,(size_t)count,filePtr)

#define tm_fwrite(bufPtr,size,count,filePtr) \
        fwrite(bufPtr,(size_t)size,(size_t)count,filePtr)

#define tm_fflush       fflush
#define tm_fclose       fclose

/* Type defines */

typedef struct tsUserLogin
{
    char             userName[20];
    char             userPassword[20];
} ttUserLogin;
typedef const ttUserLogin TM_FAR * ttUserLoginPtr;

#define TM_FS_WORKINGDIR_SIZE 256

#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */
typedef struct tsUserFsData
{
    ttUserLoginPtr     userLoginPtr;
    ttVoidPtr          userDirPtr;
    char               userWorkingDirectory[TM_FS_WORKINGDIR_SIZE];
/* Long or short directory open */
    int                userDirFlag;
#if     _MSC_VER >= 800
/* Directory handle */
    intptr_t           userDirHandle;
#endif /*  _MSC_VER >= 800 */

/* Directory entry */
    ttDirEntry         userDirEntry;
/* current drive 0, for A:, 1 for B:, etc. */
    int                userDrive;
} ttUserFsData;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */

typedef ttUserFsData TM_FAR * ttUserFsDataPtr;
typedef const ttUserFsData TM_FAR * ttConstUserFsDataPtr;

/*
 * Function prototypes
 */
static void tfPrintDecimal(ttCharPtr bufPtr, int bufLen, int number);

/*
 * Local variables
 */

static char    tlFSInitWorkingDirectory[TM_FS_WORKINGDIR_SIZE];

static const ttUserLogin  TM_CONST_QLF tlFsNamePasswd[] =
{
/*   username,      password    */
    {"treck",       "treck"     },
    {"elmic",       "elmic"     },
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

/* Validate whether this is a good path and is under tvRootDirPtr
 * returns: TM_ENOERROR
 *          TM_EINVAL
 *          TM_EACESS
 */
static int tfValidateDosPath(ttConstCharPtr pathNamePtr);

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
int tfFSUserAllowed (ttConstCharPtr userNamePtr )
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

ttVoidPtr tfFSUserLogin ( ttConstCharPtr userNamePtr,
                          ttConstCharPtr passwordPtr )
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
            tm_bzero(userFsDataPtr, sizeof(ttUserFsData));
/* set the initial dirve */
            if (tvFSInitDrive == TM_8BIT_ZERO)
            {
                tvFSInitDrive = tm_getdrive();
            }
            userFsDataPtr->userDrive = tvFSInitDrive;
/* set the initial working directory */
            if (tvFSInitWorkingDirSet == TM_8BIT_ZERO)
            {
                (void)tm_getcwd(tlFSInitWorkingDirectory, 
                                TM_FS_WORKINGDIR_SIZE );
                tvFSInitWorkingDirSet = TM_8BIT_YES;

            }
            tm_strncpy(userFsDataPtr->userWorkingDirectory, 
                       tlFSInitWorkingDirectory,
                       TM_FS_WORKINGDIR_SIZE);
        }
    }
    return (ttVoidPtr)userFsDataPtr;
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
void tfFSUserLogout (ttVoidPtr userDataPtr)
{
    if (userDataPtr)
    {
        tfFSUserDir(userDataPtr);
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
int tfFSSystem (ttCharPtr bufferPtr, int bufferSize)
{
    tm_strncpy(bufferPtr, "DOS", bufferSize);
    return 3;
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
int tfFSStructureMount (ttVoidPtr      userDataPtr,
                        ttConstCharPtr pathNamePtr )
{
    int  disk;
    int  drive;
    int  retCode;
    char driveChar;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

    driveChar = (char)tm_toupper(pathNamePtr[0]);
    retCode = -1;

    if (tfValidateDosPath(pathNamePtr) != TM_ENOERROR)
    {
        retCode = -1;
    }
    else if (tm_isupper(driveChar))
    {
        tfFSUserDir(userFsDataPtr);
        drive = (driveChar - 'A') + 1;
        (void)tm_chdrive(drive);
        disk = tm_getdrive();
        if (disk == drive)
        {
/* Store current drive, only if a user has logged in */
            if (userDataPtr != TM_VOID_NULL_PTR)
            {
                userFsDataPtr->userDrive = drive;
/* Store current working directory */
                (void)tm_getcwd( userFsDataPtr->userWorkingDirectory,
                                 TM_FS_WORKINGDIR_SIZE );
            }
            retCode = 0;
        }
    }
#undef userFsDataPtr

    return retCode;
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
int tfFSRenameFile (ttConstVoidPtr userDataPtr,
                    ttConstCharPtr fromPathNamePtr,
                    ttConstCharPtr toPathNamePtr )
{
    int retCode;

    if (   tfValidateDosPath(fromPathNamePtr) == TM_ENOERROR
        && tfValidateDosPath(toPathNamePtr) == TM_ENOERROR)
    {
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)
        tfFSUserDir(userFsDataPtr);
        retCode = tm_rename(fromPathNamePtr, toPathNamePtr);
#undef userFsDataPtr
    }
    else
    {
        retCode = -1;
    }

    return retCode;
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
int tfFSGetUniqueFileName (ttVoidPtr    userDataPtr,
                           ttCharPtr    bufferPtr,
                           int          bufferSize )
{
    ttCharPtr   tmpPtr;
    tt32Bit     fileCounter;
    int         nameLen;
    int         retCode;

    retCode = -1;

    nameLen = (int)tm_strlen(TM_UNIQUE_FILENAME);
    if(bufferSize > nameLen)
    {
        tm_strcpy(bufferPtr, TM_UNIQUE_FILENAME);
        tmpPtr = (ttCharPtr)tm_strstr(bufferPtr, ".");
        if(tmpPtr != (ttCharPtr)0)
        {
            nameLen = (int)(ttUserPtrCastToInt)(tmpPtr - bufferPtr);
        }
        fileCounter = 0;
        while(fileCounter < 0x10000000)
        {
            tfPrintDecimal( bufferPtr, nameLen, fileCounter++);
            if(tfFSOpenDir(userDataPtr, bufferPtr, 0) == (ttVoidPtr)0)
            {
                retCode = (int)tm_strlen(bufferPtr);
                break;
            }
        }
    }

    return retCode;
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
int tfFSDeleteFile (ttConstVoidPtr userDataPtr, 
                    ttConstCharPtr pathNamePtr )
{
    int retCode;


    if (tfValidateDosPath(pathNamePtr) == TM_ENOERROR)
    {
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)
        tfFSUserDir(userFsDataPtr);
        retCode = tm_remove(pathNamePtr);
#undef userFsDataPtr
    }
    else
    {
        retCode = -1; /* no permission */
    }

    return retCode;
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
 * (ttVoidPtr)0  failure
 * fileDataPtr       pointer to newly allocated file data structure.
 */

ttVoidPtr tfFSOpenFile (ttConstVoidPtr userDataPtr,
                        ttConstCharPtr pathNamePtr,
                        int            flag,
                        int            type,
                        int            structure )
{
    ttVoidPtr fileHandle;

    TM_UNREF_IN_ARG(type);
    TM_UNREF_IN_ARG(structure);
    tfFSUserDir(userDataPtr);

    if (tfValidateDosPath(pathNamePtr) == TM_ENOERROR)
    {
        fileHandle = (ttVoidPtr)tm_fopen(pathNamePtr, tlFsOpenMap[flag]);
    }
    else
    {
        fileHandle = (ttVoidPtr)0; /* don't allow access */
    }

    return fileHandle;
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
int tfFSReadFile(ttVoidPtr userDataPtr,
                 ttVoidPtr fileDataPtr,
                 ttCharPtr bufferPtr,
                 int       bufferSize )
{
    int retCode;

    TM_UNREF_IN_ARG(userDataPtr);
    retCode = (int)tm_fread(bufferPtr, 1, bufferSize, fileDataPtr);

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
int tfFSReadFileRecord (ttVoidPtr userDataPtr,
                        ttVoidPtr fileDataPtr,
                        ttCharPtr bufferPtr,
                        int       bufferSize,
                        ttIntPtr  eorPtr )
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
int tfFSWriteFile ( ttVoidPtr       userDataPtr,
                    ttVoidPtr       fileDataPtr,
                    ttConstCharPtr bufferPtr,
                    int                 bytes)
{
    int     retCode;
    int     length;

    TM_UNREF_IN_ARG(userDataPtr);
    length = (int)tm_fwrite(bufferPtr, 1, bytes, fileDataPtr);
    if (length == bytes)
    {
        retCode = 0;
    }
    else
    {
        retCode = -1;
    }

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
int tfFSWriteFileRecord ( ttVoidPtr userDataPtr,
                          ttVoidPtr fileDataPtr,
                          ttCharPtr bufferPtr,
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
int tfFSFlushFile ( ttVoidPtr userDataPtr, ttVoidPtr fileDataPtr)
{
    TM_UNREF_IN_ARG(userDataPtr);
    (void)tm_fflush(fileDataPtr);

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
int tfFSCloseFile ( ttVoidPtr userDataPtr, ttVoidPtr fileDataPtr)
{
    TM_UNREF_IN_ARG(userDataPtr);
    (void)tm_fclose(fileDataPtr);

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
int tfFSChangeDir (ttVoidPtr        userDataPtr, 
                   ttConstCharPtr  pathNamePtr )
{
    int retCode;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

    if (    (userDataPtr != TM_VOID_NULL_PTR)
         && (tfValidateDosPath(pathNamePtr) == TM_ENOERROR))
    {
        tfFSUserDir(userFsDataPtr);
        retCode = tm_chdir(pathNamePtr);
        if (retCode != -1)
        {
            (void)tm_getcwd(userFsDataPtr->userWorkingDirectory,
                            TM_FS_WORKINGDIR_SIZE);
        }
#undef userFsDataPtr
    }
    else
    {
/* This command is only valid if a user has logged in. */
        retCode = -1;
    }
    
    return retCode;
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
int tfFSChangeParentDir ( ttVoidPtr userDataPtr )
{
    int retCode;

    if (userDataPtr != TM_VOID_NULL_PTR)
    {
        retCode = tfFSChangeDir(userDataPtr, "..");
    }
    else
    {
/* This command is only valid if a user has logged in. */        
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
int tfFSRemoveDir(ttConstVoidPtr   userDataPtr, 
                  ttConstCharPtr   pathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);

#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)
/* Point to current user directory */
    tfFSUserDir(userFsDataPtr);
#undef userFsDataPtr

    if (tfValidateDosPath(pathNamePtr) == TM_ENOERROR)
    {
        return tm_rmdir(pathNamePtr);
    }
    else
    {
        return -1; /* don't allow directory removal */
    }
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
int tfFSMakeDir (ttConstVoidPtr    userDataPtr,
                 ttConstCharPtr    pathNamePtr,
                 ttCharPtr         bufferPtr,
                 int               bufferSize )
{
    if (tfValidateDosPath(pathNamePtr) == TM_ENOERROR)
    {
        int     retCode;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)
    /* Point to current user directory */
        tfFSUserDir(userFsDataPtr);
        retCode = tm_mkdir(pathNamePtr);
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
#undef userFsDataPtr

        return retCode;
    }
    else
    {
        TM_UNREF_IN_ARG(userDataPtr);
        TM_UNREF_IN_ARG(pathNamePtr);
        TM_UNREF_IN_ARG(bufferPtr);
        TM_UNREF_IN_ARG(bufferSize);
        return -1; /* don't allow directory creation */
    }
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
int tfFSGetWorkingDir ( ttConstVoidPtr userDataPtr,
                        ttCharPtr       bufferPtr,
                        int                 bufferSize )
{
    int retCode;

    tfFSUserDir((ttUserFsDataPtr)userDataPtr);
    (void)tm_getcwd(bufferPtr, bufferSize);
    if (bufferPtr != (ttCharPtr) 0)
    {
        bufferPtr[bufferSize - 1] = '\0';
        retCode = (int)tm_strlen(bufferPtr);
    }
    else
    {
        retCode = -1;
    }

    return retCode;
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
 * (ttVoidPtr)0  failure
 * dirDataPtr       Pointer to newly allocated directory data structure
 */

#ifdef TM_LINT
LINT_UNACCESS_SYM_BEGIN(attribute)
#endif /* TM_LINT */
ttVoidPtr tfFSOpenDir (ttVoidPtr       userDataPtr,
                       ttConstCharPtr  pathNamePtr,
                       int             flag )
{
    intptr_t            retCode;
    int                 attribute;
    ttVoidPtr           retDirPtr;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)

    if (userDataPtr != TM_VOID_NULL_PTR)
    {
        if (tfValidateDosPath(pathNamePtr) != TM_ENOERROR)
        {
/* do not allow FTP client access to files in other directories */
            pathNamePtr = "*.*";
        }

        if(pathNamePtr[0] == '-')
/* We do not support any sort of list arguments like -a */    
        {
            pathNamePtr = "*.*";
        }

        if (flag == TM_DIR_SHORT)
        {
            attribute = _A_NORMAL;
        }
        else
        {
/* Long listing */
            attribute = _A_NORMAL | _A_SUBDIR;
        }

        tfFSUserDir(userDataPtr);

        retCode = tm_findfirst( pathNamePtr,
                                &userFsDataPtr->userDirEntry,
                                attribute );

        if (retCode == TM_L(-1))
        {
            userFsDataPtr->userDirPtr = (ttVoidPtr)0;
        }
        else
        {
#ifdef _MSC_VER
#if     _MSC_VER >= 800
            userFsDataPtr->userDirHandle = retCode;
#endif /*  _MSC_VER >= 800 */
#endif /* _MSC_VER */
            userFsDataPtr->userDirPtr = (ttVoidPtr)&userFsDataPtr->userDirEntry;
            userFsDataPtr->userDirFlag = flag;
        }

        retDirPtr = userFsDataPtr->userDirPtr;
    }
    else
    {
/* This command is only valid if a user has logged in. */
        retDirPtr = TM_VOID_NULL_PTR;
    }

#undef userFsDataPtr    
    return retDirPtr;
}
#ifdef TM_LINT
LINT_UNACCESS_SYM_END(attribute)
#endif /* TM_LINT */

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
int tfFSGetNextDirEntry ( ttVoidPtr userDataPtr,
                          ttVoidPtr dirDataPtr,
                          ttCharPtr bufferPtr,
                          int       bufferSize )
{
    struct tm TM_FAR * timeStruct;
    int                retCode;
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)
#define userFsDirPtr  ((ttDirEntryPtr)dirDataPtr)

    if (userDataPtr != TM_VOID_NULL_PTR)
    {
        retCode = 0;
        tfFSUserDir(userDataPtr);
        if (userFsDataPtr->userDirPtr != (ttVoidPtr)0)
        {
            while (tm_fs_is_dir(userFsDirPtr))
            {
/*
 * Pass over any directories entry for TM_DIR_SHORT listing, and
 * '.', or '..' for TM_DIR_LONG.
 */
                if (userFsDataPtr->userDirFlag == TM_DIR_LONG)
                {
                    if (    ( tm_strcmp(tm_fs_file_name_ptr(userFsDirPtr), ".")
                          != TM_STRCMP_EQUAL)
                         && ( tm_strcmp(tm_fs_file_name_ptr(userFsDirPtr), "..")
                          != TM_STRCMP_EQUAL) )
                    {
                        break;
                    }
                }
                if (    tm_findnext(userFsDataPtr->userDirHandle,
                                    userFsDirPtr)
                    == -1 )
                {
                    userFsDataPtr->userDirPtr = (ttVoidPtr)0;
                    break;
                }
            }
            if (userFsDataPtr->userDirPtr != (ttVoidPtr)0)
            {
                tm_memset(bufferPtr, ' ', (unsigned)bufferSize);
                
                timeStruct = localtime(&tm_fs_file_time(userFsDirPtr));
                strftime(&bufferPtr[retCode],
                         21,
                         "%m/%d/%Y  %H:%M %p",
                         timeStruct);
                retCode += 20;
                
                if (userFsDataPtr->userDirFlag == TM_DIR_LONG)
                {
                    if ( tm_fs_is_dir(userFsDirPtr) )
                    {
/* Long listing, append more info to the buffer */
                        if (bufferSize >= retCode + 19)
                        {
                            tm_bcopy( "    <DIR>          ",
                                      &bufferPtr[retCode], 
                                      18);
                            retCode += 19;
                        }
                    }
                    else
                    {
/* List the size of the file */
                        if (bufferSize >= retCode + 19)
                        {
                            tfPrintDecimal( &bufferPtr[retCode],
                                            18,
                                            tm_fs_file_size(userFsDirPtr));
                            bufferPtr[retCode] = ' ';
                            retCode += 19;
                        }
                    }
                }

                tm_bcopy( tm_fs_file_name_ptr(userFsDirPtr),
                          &bufferPtr[retCode],
                          tm_strlen(tm_fs_file_name_ptr(userFsDirPtr)));
                retCode += (int)tm_strlen(tm_fs_file_name_ptr(userFsDirPtr));
                bufferPtr[retCode] = '\0';
/* Point to next directory entry */
                if (    tm_findnext(userFsDataPtr->userDirHandle, userFsDirPtr)
                    == -1 )
                {
                    userFsDataPtr->userDirPtr = (ttVoidPtr)0;
                }
            }
        }
    }
    else
    {
        retCode = -1;
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
void tfFSCloseDir (ttVoidPtr userDataPtr, ttVoidPtr dirDataPtr )
{
#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)
    TM_UNREF_IN_ARG(dirDataPtr);

    if (userDataPtr != TM_VOID_NULL_PTR)
    {
        userFsDataPtr->userDirPtr = (ttVoidPtr)0;
#ifdef _MSC_VER
#if     _MSC_VER >= 800
        tfFSUserDir(userDataPtr);
        (void)_findclose(userFsDataPtr->userDirHandle);
#endif /*  _MSC_VER >= 800 */
#endif /* _MSC_VER */
    }
#undef userFsDataPtr

    return;
}

static void tfFSUserDir(ttConstUserFsDataPtr userFsDataPtr)
{
    if (userFsDataPtr != (ttConstUserFsDataPtr) 0)
    {
        (void)tm_chdrive(userFsDataPtr->userDrive);
        (void)tm_chdir(userFsDataPtr->userWorkingDirectory);
    }

    return;
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
static void tfPrintDecimal(ttCharPtr bufPtr, int bufLen, int number)
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
    ttConstVoidPtr userDataPtr,
    ttConstCharPtr pathNamePtr )
{
    intptr_t            handle;
    int                 errorCode;
    struct _finddata_t  findData;

    TM_UNREF_IN_ARG(userDataPtr);

    if (pathNamePtr == (ttConstVoidPtr)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        handle = tm_findfirst(pathNamePtr, &findData, 0);
        if (handle == TM_L(-1))
        {
            errorCode = TM_ENOENT;
        }
        else
        {
            errorCode = TM_ENOERROR;
        }
    }

    return errorCode;
}


ttUser32Bit tfFSGetFileLen( 
    ttConstVoidPtr userDataPtr,
    ttConstCharPtr pathNamePtr )
{
    intptr_t            handle;
    tt32Bit             fileLen;
    struct _finddata_t  findData;

    TM_UNREF_IN_ARG(userDataPtr);

    handle = tm_findfirst(pathNamePtr, &findData, 0);
    if (handle == TM_L(-1))
    {
        fileLen = 0;
    }
    else
    {
        fileLen = findData.size;
    }
    return fileLen;
}

/* Set the DOS root */
void tfSetDosRootDir(ttConstCharPtr rootDirPtr)
{
/* free if not big enough */
    if (   tm_global(tvRootDirPtr)
        &&   tm_strlen(tm_global(tvRootDirPtr))
           < tm_strlen(rootDirPtr) + 1/* null terminator */)
    {
        tm_kernel_free(tm_global(tvRootDirPtr));
        tm_global(tvRootDirPtr) = (ttCharPtr)0;
    }

/* allocate if not allocated */
    if (tm_global(tvRootDirPtr) == (ttCharPtr)0)
    {
/* +1: null terminator */
        tm_global(tvRootDirPtr) = tm_kernel_malloc(
                                    (ttPktLen)tm_strlen(rootDirPtr) + 1);
    }
/* Copy the root dir in */
    tm_strcpy(tm_global(tvRootDirPtr), rootDirPtr);
}
 
/* Validate whether this is a good path and is under tvRootDirPtr
 * returns: TM_ENOERROR
 *          TM_EINVAL
 *          TM_EACESS
 */
int tfValidateDosPath(ttConstCharPtr pathNamePtr)
{
    char fullPath[MAX_PATH];
    char canFullPath[MAX_PATH];
    int  errorCode;

    if ( (pathNamePtr == (ttCharPtr)0) || (*pathNamePtr == 0) )
    {
        fullPath[0] = 0;
        errorCode = TM_EINVAL;
    }
    else if (strchr(pathNamePtr, ':'))
    {
/* the pathNamePtr is an absolute path start with the drive */
        tm_strcpy(fullPath, pathNamePtr);
        fullPath[MAX_PATH - 1] = '\0';
    }
    else if ((pathNamePtr[0] ==  '/') || (pathNamePtr[0] == '\\'))
    {
/* the pathNamePtr is an absolute path */
        tm_strncpy(fullPath, pathNamePtr, MAX_PATH);
    }
    else
    {
/* Append the cwd in front of the relative path */
        _getcwd(fullPath, MAX_PATH);
        tm_strcat(fullPath, "\\");
        tm_strcat(fullPath, pathNamePtr);
    }

    if (!PathCanonicalize(canFullPath, fullPath))
    {
        errorCode = TM_EINVAL;
    }
    else if (StrStrI(canFullPath, tm_global(tvRootDirPtr)) == canFullPath)
    {
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_EACCES;
    }

    return errorCode;
}


void tfInitDosFs(void)
{
    char  workingDir[MAX_PATH];

/* Set the root directory to be the current directory */
    tfFSGetWorkingDir (TM_VOID_NULL_PTR, workingDir, MAX_PATH);
    tfSetDosRootDir(workingDir);
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
    ttVoidPtr       userDataPtr,
    ttVoidPtr       fileDataPtr,
    long            offset,
    int             whence)
{
    int                 errorCode;

    TM_UNREF_IN_ARG(userDataPtr);
    if (fileDataPtr == (ttVoidPtr)0)
    {
        errorCode = TM_EINVAL;
    }
    else if (fseek(fileDataPtr, offset, whence) == 0)
    {
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_EIO;
    }

    return errorCode;
}

#endif /* TM_USE_DOS_FS */
