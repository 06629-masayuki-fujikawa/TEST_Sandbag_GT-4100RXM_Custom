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
 * Description:  File system interface for DOS/Windows that emulates UNIX 
 * Filename:     txdosunx.c
 * Author:       Tony Ferrell
 * Date Created: 07/29/2008
 * $Source: examples/txdosunx.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/06/15 06:12:44JST $
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

#ifdef TM_USE_EMU_UNIX_FS

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#include <direct.h>
#include <io.h>         /* for rename() */
#include <stdio.h>      /* for fopen */
#include <shlwapi.h>    /* for PathCanonicalize, StrStrI*/

#include <time.h>       /* Time functions for directory listings */

#ifndef intptr_t
#define intptr_t    long
#endif /* intptr_t */

/*
 * Local macro definitions
 */
/* Unique file name format */
#define TM_UNIQUE_FILENAME      "F0000000.trk"

/* Column position for printing <DIR> in sub-directory directory entry */
#define TM_DIR_POS   24

/* Column position for printing size of file entry */
#define TM_SIZE_POS   24

/* This entry is a directory */
#define tm_fs_is_dir(fsDirPtr) (fsDirPtr->ff_attrib & _A_SUBDIR)

/* This entry is read only */
#define tm_fs_is_read_only(fsDirPtr) (fsDirPtr->ff_attrib & _A_RDONLY)

/* Get the file name pointer of the entry */
#define tm_fs_file_name_ptr(fsDirPtr) (fsDirPtr->ff_name)

/* Get the file size of the entry */
#define tm_fs_file_size(fsDirPtr) (fsDirPtr->size)

/* Get the file time/date */
#define tm_fs_file_time(fsDirPtr) (fsDirPtr->ff_time)

#ifdef _MSC_VER

#if     _MSC_VER >= 800
#define tm_getcwd_cmd    _getcwd
#define tm_chdir_cmd     _chdir
#define tm_rmdir_cmd     _rmdir
#define tm_mkdir_cmd     _mkdir
#define ttDirEntry       struct _finddata_t
#define tm_findfirst_cmd( pathNamePtr, dirPtr, attribute ) \
                     _findfirst( pathNamePtr, dirPtr)
#define tm_findnext( handle, dirPtr ) \
                     _findnext( handle, dirPtr )
#define ff_name      name
#define ff_attrib    attrib
#define ff_time      time_write
#endif /*  _MSC_VER >= 800 */

#endif /* _MSC_VER */

#ifndef ttDirEntry
#define ttDirEntry   struct ffblk
#endif /* ttDirEntry */

typedef ttDirEntry TM_FAR * ttDirEntryPtr;

#ifndef tm_findfirst_cmd
#define tm_findfirst_cmd( pathNamePtr, dirPtr, attribute ) \
                     findfirst(pathNamePtr, dirPtr, attribute)
#endif /* tm_findfirst */

#ifndef tm_findnext_cmd
#define tm_findnext_cmd( handle, dirPtr ) \
                     findnext(dirPtr)
#endif /* tm_findnext */

#ifndef tm_getcwd_cmd
#define tm_getcwd_cmd    getcwd
#endif /* tm_getcwd */

#ifndef tm_chdir_cmd
#define tm_chdir_cmd     chdir
#endif /* tm_chdir */

#ifndef tm_rmdir_cmd
#define tm_rmdir_cmd     rmdir
#endif /* tm_rmdir */

#ifndef tm_mkdir_cmd
#define tm_mkdir_cmd     mkdir
#endif /* tm_mkdir */

#define tm_chdrive   _chdrive
#define tm_getdrive  _getdrive

#define tm_rename_cmd    rename
#define tm_remove_cmd    remove

#define tm_fopen_cmd(fh,pchParam)       \
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
 * Function prototypes for UNIX emulation
 */

/* UNIXto DOS path converions */
int tfFSDosPathConvert(ttCharPtr dosRootPath,
                       ttConstCharPtr unixWorkingPath,
                       ttCharPtr bufferPtr,
                       int                 bufferSize);
/* DOS to UNIX path converions */
int tfFSUnixPathConvert(ttCharPtr dosRootPath,
                        ttCharPtr dosCurrentPath,
                        ttCharPtr bufferPtr,
                        int  bufferSize);

/* System calls with conversions attached */
ttCharPtr tfGetcwdEmu(ttCharPtr bufferPtr, int bufferSize);
int       tfChdirEmu(ttConstCharPtr dirPtr);
int       tfRmDirEmu(ttConstCharPtr  dirPtr);
int       tfMkdirEmu(ttConstCharPtr dirPtr);
intptr_t  tfFindfirstEmu(ttConstCharPtr pathNamePtr,
                         ttDirEntryPtr  dirPtr, 
                         int            attribute);
int       tfRenameEmu( ttConstCharPtr fromPathNamePtr,
                       ttConstCharPtr toPathNamePtr);
int       tfRemoveEmu( ttConstCharPtr pathNamePtr );
FILE*     tfFopenEmu ( ttConstCharPtr fromPathNamePtr,
                       ttConstCharPtr toPathNamePtr);

/* Macro redefines for UNIX emulation */
#define tm_getcwd    tfGetcwdEmu
#define tm_chdir     tfChdirEmu
#define tm_rmdir     tfRmDirEmu
#define tm_mkdir     tfMkdirEmu
#define tm_findfirst tfFindfirstEmu
#define tm_rename    tfRenameEmu
#define tm_remove    tfRemoveEmu
#define tm_fopen     tfFopenEmu

/*
 * File system function prototypes for
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

ttVoidPtr tfFSUserLogin (ttConstCharPtr userNamePtr,
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
    tm_strncpy(bufferPtr, "UNIX", bufferSize);
    return 4;
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
                        ttConstCharPtr pathNamePtr)
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
                    ttConstCharPtr toPathNamePtr)
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
                           int          bufferSize)
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
                    ttConstCharPtr pathNamePtr)
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
                        int            structure)
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
                 int       bufferSize)
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
                        ttIntPtr  eorPtr)
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
int tfFSWriteFile (ttVoidPtr       userDataPtr,
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
int tfFSWriteFileRecord (ttVoidPtr userDataPtr,
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
int tfFSFlushFile (ttVoidPtr userDataPtr, ttVoidPtr fileDataPtr)
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
int tfFSCloseFile (ttVoidPtr userDataPtr, ttVoidPtr fileDataPtr)
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
                   ttConstCharPtr  pathNamePtr)
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
int tfFSChangeParentDir (ttVoidPtr userDataPtr)
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
                  ttConstCharPtr   pathNamePtr)
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
                 int               bufferSize)
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
int tfFSGetWorkingDir (ttConstVoidPtr userDataPtr,
                       ttCharPtr      bufferPtr,
                       int            bufferSize)
{
    int retCode;

    tfFSUserDir((ttUserFsDataPtr)userDataPtr);
    (void)tm_getcwd(bufferPtr, bufferSize);
    if (bufferPtr != (ttCharPtr) 0)
    {
/* tm_getcwd ensures bufferPtr is null terminated */
        retCode = (int)tm_strlen(bufferPtr);
    }
    else
    {
        retCode = -1;
    }

    return retCode;
}
/* 
 * tfFSGetInitWorkingDir function description:
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
int tfFSGetInitWorkingDir (ttConstVoidPtr  userDataPtr,
                           ttCharPtr       bufferPtr,
                           int             bufferSize)
{
    int retCode;

    tfFSUserDir((ttUserFsDataPtr)userDataPtr);
    (void)tm_getcwd_cmd(bufferPtr, bufferSize);
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
int tfFSGetNextDirEntry (ttVoidPtr userDataPtr,
                         ttVoidPtr dirDataPtr,
                         ttCharPtr bufferPtr,
                         int       bufferSize)
{
    ttCharPtr          attributeString;
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
                if( tm_fs_is_dir(userFsDirPtr) )
                {
                    bufferPtr[retCode] = 'd';
                }
                else
                {
                    bufferPtr[retCode] = '-';
                }                
                ++retCode;
                
/* Add file rights: we can only get read rights, so that's what we use */
                if ( tm_fs_is_read_only(userFsDirPtr) )
                {
                    attributeString = "r--r--r-- 1 ftp ftp";
                }
                else
                {
                    attributeString = "rw-rw-rw- 1 ftp ftp";
                }
/* Add file rights: this doesn't exist for DOS files, so we fake it */
                tm_bcopy(attributeString, &bufferPtr[retCode], 19);
                retCode += 20;

/* Get the file size and store it in a temp buffer */
                tfPrintDecimal( &bufferPtr[retCode], 14, tm_fs_file_size(userFsDirPtr));
                retCode += 15;

/* Add the last modification time to the listing */
                timeStruct = localtime(&tm_fs_file_time(userFsDirPtr));
                strftime(&bufferPtr[retCode], 14, "%b %d %H:%M ", timeStruct);
                retCode += 13;

/* Add the filename to the listing */
                tm_bcopy( tm_fs_file_name_ptr(userFsDirPtr),
                            &bufferPtr[retCode],
                            tm_strlen(tm_fs_file_name_ptr(userFsDirPtr)));
                retCode += (int)tm_strlen(tm_fs_file_name_ptr(userFsDirPtr));

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
        tm_strcat(fullPath, "/");
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
    else if (StrStrI(canFullPath, "/") == canFullPath)
    {
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_EACCES;
    }

    return errorCode;
}


/* 
 * tfInitEmuUnixFs function description:
 * This function gets the DOS format working directory and sets it as the root
 * working directory for the server.
 *
 * Parameters
 * Parameter     Description
 * n/a
 * Returns
 * n/a
 */
void tfInitEmuUnixFs(void)
{
    char  workingDir[MAX_PATH];

/* Set the root directory to be the current directory */
    tfFSGetInitWorkingDir (TM_VOID_NULL_PTR, workingDir, MAX_PATH);
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


/* 
 * tfGetcwdEmu function description:
 * This function gets the DOS format working directory and returns it in UNIX
 * format.
 *
 * Parameters
 * Parameter     Description
 * bufferPtr     The pointer to fill with the name of the current working dir.
 * bufferSize    The size of bufferPtr.
 *
 * Returns
 * bufferPtr     A pointer to the current working directory.
 */
ttCharPtr tfGetcwdEmu (ttCharPtr bufferPtr, int bufferSize)
{
    ttCharPtr tempBuffer;

    tempBuffer = tm_get_raw_buffer(bufferSize);
    if (tempBuffer)
    {
        tm_getcwd_cmd(tempBuffer, bufferSize);
        tfFSUnixPathConvert(tm_global(tvRootDirPtr),
                        tempBuffer,
                        bufferPtr,
                        bufferSize);
    
        tm_free_raw_buffer(tempBuffer);
    }
    return bufferPtr;
}


/* 
 * tfChdirEmu function description:
 * Takes in a UNIX format directory, changes it to DOS format and
 * changes to that directory in the filesystem.
 *
 * Parameters
 * Parameter     Description
 * dirPtr        The name of the directory to change to.
 *  
 * Returns
 * 0             Directory change succesful.
 * -1            Unable to change to that directory.
 */
int tfChdirEmu (ttConstCharPtr dirPtr)
{
    ttCharPtr tempBuffer;
    int       retCode;
    
    retCode = -1; /* assume failure */

    tempBuffer = tm_get_raw_buffer(MAX_PATH);
    if (tempBuffer)
    {
        tfFSDosPathConvert(tm_global(tvRootDirPtr), 
                                dirPtr, 
                                tempBuffer, 
                                MAX_PATH);
        retCode = tm_chdir_cmd(tempBuffer);
    
        tm_free_raw_buffer(tempBuffer);
    }
    return retCode;
}


/* 
 * tfRmDirEmu function description:
 * Takes in a UNIX format directory, changes it to DOS format and removes the
 * directory.
 *
 * Parameters
 * Parameter     Description
 * dirPtr        The name of the directory to remove.
 *  
 * Returns
 * 0             Directory removal succesful.
 * -1            Unable to remove that directory.
 */
int tfRmDirEmu ( ttConstCharPtr  dirPtr)
{
    ttCharPtr tempBuffer;
    int retCode;

    retCode = -1; /* assume failure */

    tempBuffer = tm_get_raw_buffer(MAX_PATH);
    if (tempBuffer)
    {
        tfFSDosPathConvert(tm_global(tvRootDirPtr), 
                                dirPtr, 
                                tempBuffer, 
                                MAX_PATH);
        retCode = tm_rmdir_cmd(tempBuffer);
        tm_free_raw_buffer(tempBuffer);
    }
    return retCode;
}


/* 
 * tfMkdirEmu function description:
 * Takes in a UNIX format directory , changes it to DOS format and creates the
 * directory.
 *
 * Parameters
 * Parameter     Description
 * dirPtr        The name of the directory to create.
 *  
 * Returns
 * 0             Directory creation succesful.
 * -1            Unable to create to that directory.
 */
int tfMkdirEmu (ttConstCharPtr dirPtr)
{
    ttCharPtr tempBuffer;
    int retCode;

    retCode = -1; /* assume failure */
    tempBuffer = tm_get_raw_buffer(MAX_PATH);
    if (tempBuffer)
    {
        tfFSDosPathConvert(tm_global(tvRootDirPtr), 
                                        dirPtr, 
                                        tempBuffer, 
                                        MAX_PATH);
        retCode = tm_mkdir_cmd(tempBuffer);
        tm_free_raw_buffer(tempBuffer);
    }
    return retCode;
}


/* 
 * tfFindfirstEmu function description:
 * Takes in a UNIX format path, directory pointer, and an attribute list.
 * It changes the directory listing to DOS format and finds the next item that
 * matches the attributes.
 *
 * Parameters
 * Parameter    Description
 * pathNamePtr  The path to search.
 * dirPtr       The directory pointer
 * attribute    Attributes to match
 *  
 * Returns
 * retCode       The handle of the first item that matches attribute.
 */
intptr_t           tfFindfirstEmu(ttConstCharPtr  pathNamePtr,
                                  ttDirEntryPtr   dirPtr,
                                  int             attribute)
{
    ttCharPtr          tempBuffer;
    intptr_t           retCode;

    retCode = TM_L(-1); /* findfirst returns a negative value on an error */
    TM_UNREF_IN_ARG(attribute);

    tempBuffer = tm_get_raw_buffer(MAX_PATH);
    if (tempBuffer)
    {
        tfFSDosPathConvert(tm_global(tvRootDirPtr),
                       pathNamePtr,
                       tempBuffer,
                       MAX_PATH);
        retCode = tm_findfirst_cmd( (ttCharPtr)tempBuffer, dirPtr, attribute );
        tm_free_raw_buffer(tempBuffer);
    }
    return retCode;
}


/* 
 * tfRenameEmu function description:
 * Takes in UNIX format from and to names and changes them to DOS format 
 * and renames them.
 *
 * Parameters
 * Parameter        Description
 * fromPathNamePtr  The UNIX formated filename to change from.
 * toPathNamePtr    THE UNIX formated filename to change to.
 *  
 * Returns
 * 0                Directory creation succesful.
 * -1               Unable to create to that directory.
 */
int tfRenameEmu(ttConstCharPtr fromPathNamePtr, ttConstCharPtr toPathNamePtr)
{
    ttCharPtr fromTempPtr;
    ttCharPtr toTempPtr;
    int retCode;

    retCode = -1;

    fromTempPtr = tm_get_raw_buffer(MAX_PATH);
    toTempPtr   = tm_get_raw_buffer(MAX_PATH);

    if (fromTempPtr && toTempPtr)
    {
        tfFSDosPathConvert(tm_global(tvRootDirPtr),
                       fromPathNamePtr,
                       fromTempPtr,
                       MAX_PATH);

        tfFSDosPathConvert(tm_global(tvRootDirPtr),
                       toPathNamePtr,
                       toTempPtr,
                       MAX_PATH);

        retCode = tm_rename_cmd(fromTempPtr, toTempPtr);
        tm_free_raw_buffer(fromTempPtr);
        tm_free_raw_buffer(toTempPtr);
    }
    return retCode;
}


/* 
 * tfRemoveEmu function description:
 * Takes in UNIX format filename, converts it to DOS format and removes it.
 *
 * Parameters
 * Parameter        Description
 * fromPathNamePtr  The UNIX formated filename to remove.
 *  
 * Returns
 * 0                Removal succesful.
 * -1               Unable to remove that item.
 */
int tfRemoveEmu(ttConstCharPtr pathNamePtr)
{
    ttCharPtr tempBuffer;
    int retCode;

    retCode = -1;
    tempBuffer = tm_get_raw_buffer(MAX_PATH);

    if (tempBuffer)
    {
        tfFSDosPathConvert(tm_global(tvRootDirPtr),
                       pathNamePtr,
                       tempBuffer,
                       MAX_PATH);

        retCode = tm_remove_cmd(tempBuffer);
        tm_free_raw_buffer(tempBuffer);
    }
    return retCode;
}

/* 
 * tfFopenEmu function description:
 * Open a file for read, write, or append
 *
 * Parameters
 * Parameter        Description
 * pathNamePtr   Pointer to a null terminated string containing file name.
 * mode          TM_FS_READ, TM_FS_WRITE, TM_FS_APPEND
 *  
 * Returns
 * (FILE TM_FAR *)0  failure
 * fileDataPtr       pointer to newly allocated file data structure.
 */
FILE TM_FAR * tfFopenEmu(ttConstCharPtr pathNamePtr, ttConstCharPtr mode)
{
    ttCharPtr tempBuffer;
    FILE TM_FAR * retCode;

    retCode = (FILE TM_FAR *)0;
    tempBuffer = tm_get_raw_buffer(MAX_PATH);

    if (tempBuffer)
    {
        tfFSDosPathConvert(tm_global(tvRootDirPtr),
                           pathNamePtr,
                           tempBuffer,
                           MAX_PATH);

        retCode = tm_fopen_cmd(tempBuffer, mode);
        tm_free_raw_buffer(tempBuffer);
    }
    return retCode;
}

/*
 * tfFSDosPathConvert function description:
 * Converts a DOS file path to a Unix file path using the starting 
 * directory of the server as  the root folder, and any subsequent 
 * changes as paths down from the root.
 *
 * 1. Convert all instances of '/' with '\'
 * 2. Identify whether or not the path is the root path
 * 3. Return converted path
 *
 * Parameters:
 * Value                Meaning
 * dosRootPath          Server root directory
 * unixWorkingPath      Unix path off of the root directory.
 *
 * Return:
 * retCode              Length of the path.
 */

int tfFSDosPathConvert(ttCharPtr dosRootPath,
                       ttConstCharPtr unixWorkingPath,
                       ttCharPtr bufferPtr,
                       int                 bufferSize)
{
    ttCharPtr   slashLocation;
    ttCharPtr   temp;
    int         retCode;
    int         tempStringLength1;
    int         tempStringLength2;

/*
 * Allocate room for the maximum path size allowed, plus room for a null
 *  terminator, just in case.
 */
    temp = tm_get_raw_buffer( MAX_PATH + 1 );

/* Check if the allocation was successful */
    if (temp && bufferSize)
    {
/* Create a temp copy of the path and ensure it is NULL terminated */
        tm_strncpy(temp, unixWorkingPath, MAX_PATH);
        temp[MAX_PATH] = '\0';

/* Replace all '/'s with '\\'s */ 
        while((slashLocation = tm_strchr(temp, '/')) != NULL)
        {
            slashLocation[0] = '\\';
        }

        if (temp[0] == '\\')
/* This is an absolute path */
        {
            tm_strncpy(bufferPtr, dosRootPath, bufferSize);
            bufferPtr[bufferSize - 1] = '\0';

            tempStringLength1 = (int)tm_strlen( bufferPtr );
            tempStringLength2 = (int)tm_strlen(temp);

            if( (tempStringLength1 + tempStringLength2 ) <  bufferSize)
            {
                tm_strcat(bufferPtr,temp);
            }
            else
            {
                retCode = -1;
            }
        }
        else
        {
            tm_memset (bufferPtr, '\0', bufferSize);
/* 
 * We can only copy bufferSize - 1 bytes into bufferPtr to make sure that we 
 * have room for the null terminator so we can call tm_strlen later.
 */
            tm_strncpy(bufferPtr, temp, (bufferSize-1));
        }

        retCode = (int)tm_strlen(bufferPtr);
    }
    else
    {
/* A negative return code indicates an unsuccessful operation */
        retCode = -1;   
    }
    if (temp)
    {
        tm_free_raw_buffer(temp);
    }
    return retCode;
}

/*
 * tfFSUnixPathConvert function description:
 * Converts a DOS file path to a Unix file path using the starting 
 * directory of the server as the root folder, and any subsequent 
 * changes as paths down from the root.
 *
 * 1. Remove non-root addition to path.
 * 2. Convert all instances of '\' with '/'
 * 3. Return converted path
 *
 * Parameters:
 * Value                 Meaning
 * dosRootPath           Server root directory
 * dosCurrentPath        Server current directory. Must be in canonical form.
 * bufferPtr             Buffer to fill with the converted path name.
 * bufferSize            Size of bufferPtr.
 * Return:
 * retCode               Length of the path.
 */
int tfFSUnixPathConvert(ttCharPtr dosRootPath,
                        ttCharPtr dosCurrentPath,
                        ttCharPtr bufferPtr,
                        int  bufferSize)
{
    ttCharPtr   slashLocation;
    ttCharPtr   nonRootPath;
    int         retCode;
    
/* Get the portion of the directory that is above the root */
    nonRootPath = &dosCurrentPath[tm_strlen(dosRootPath)];
    tm_strncpy( bufferPtr, nonRootPath, bufferSize);

/* Ensure we have a null terminator */
    bufferPtr[bufferSize-1] = '\0';

/* Replace all '\\'s with '/'s */
    while((slashLocation = tm_strchr(bufferPtr, '\\')) != NULL)
    {
        slashLocation[0] = '/';
    }

/* Takes care of the root directory. Has no affect on other listings */
    bufferPtr[0] = '/';

    retCode = (int)tm_strlen(bufferPtr);
    return retCode;   
}
#endif /* TM_USE_EMU_UNIX_FS */
