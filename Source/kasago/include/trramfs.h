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
 * Description: Treck Ram File System Include file
 * Filename: trramfs.h
 * Author: Emmanuel Attia & Qin Zhang
 * Date Created: 7/25/02
 * $Source: include/trramfs.h $
 *
 * Modification History
 * $Revision: 6.0.2.6 $
 * $Date: 2012/09/19 00:27:23JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRRAMFS_H_

#define _TRRAMFS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* For suppressing PC-lint compile errors */
LINT_UNREF_MACRO(TM_RAMFS_ATTRIB_ARCH)
LINT_UNREF_MACRO(TM_RAMFS_ATTRIB_RDONLY)
LINT_UNREF_MACRO(TM_RAMFS_ATTRIB_SYSTEM)
LINT_UNREF_MACRO(TM_RAMFS_ATTRIB_HIDDEN)
LINT_UNREF_MACRO(tfRamfsCreate)

/* RAM file system functions return code */
#define TM_RAMFS_ERROR     -1
#define TM_RAMFS_OKAY       0

/* Max number of raw blocks */
#define TM_RAMFS_MAXRAWBLOCK        20

/* File access flags */
#define TM_RAMFS_ACCESS_READ        1
#define TM_RAMFS_ACCESS_WRITE       2

/* File open mode */
#define TM_RAMFS_OPEN_CREATE        0x10
#define TM_RAMFS_OPEN_EXCL          0x20
#define TM_RAMFS_OPEN_TRUNC         0x40
#define TM_RAMFS_OPEN_APPEND        0x80

/* File share mode */
#define TM_RAMFS_SHARE_READ         0x100
#define TM_RAMFS_SHARE_WRITE        0x200

/* File type */
#define TM_RAMFS_TYPE_BINARY        0
#define TM_RAMFS_TYPE_TEXT          0x1000

/* Seek method constants */
#define TM_RAMFS_SEEK_SET           0
#define TM_RAMFS_SEEK_CUR           1
#define TM_RAMFS_SEEK_END           2


#ifndef TM_RAMFS_BLOCKSIZE
#define TM_RAMFS_BLOCKSIZE          128
#endif /* !TM_RAMFS_BLOCKSIZE */

/*
 * TM_RAMFS_MAXFNAME cannot exceed
 * (TM_RAMFS_BLOCKSIZE - sizeof(tt32Bit) - 6 * sizeof(ttRamfsBlock))
 */
#if TM_RAMFS_BLOCKSIZE >= 128
#define TM_RAMFS_MAXFNAME           (128 - sizeof(tt32Bit) \
                                    - 6 * sizeof(ttRamfsBlock))
#else
#define TM_RAMFS_MAXFNAME           (TM_RAMFS_BLOCKSIZE - sizeof(tt32Bit) \
                                    - 6 * sizeof(ttRamfsBlock))
#endif
#define TM_RAMFS_MAXPATH            260
#define TM_RAMFS_MAXLABEL           12

#define TM_RAMFS_NOLINK             ((ttRamfsBlock)-1)

/* Directory entry flags */
#define TM_RAMFS_ATTRIB_DIR         0x1
#define TM_RAMFS_ATTRIB_ARCH        0x2
#define TM_RAMFS_ATTRIB_RDONLY      0x4
#define TM_RAMFS_ATTRIB_SYSTEM      0x8
#define TM_RAMFS_ATTRIB_HIDDEN      0x10

typedef ttUShort ttRamfsBlock;
typedef ttS32Bit  ttRamfsSize;
typedef ttRamfsBlock TM_FAR * ttRamfsBlockPtr;
LINT_UNREF_MEMBER_BEGIN
typedef ttRamfsSize TM_FAR * ttRamfsSizePtr;
LINT_UNREF_MEMBER_END


/* Low level parameters */
#define TM_RAMFS_DIRSEP             '/'
#define TM_RAMFS_INVALIDCHAR        "\\/*?\"<>|"
#define TM_RAMFS_MAXFILES           64

/* Disk flags */
#define TM_RAMFS_MOUNTED            (1 <<  0)
#define TM_RAMFS_LOADED             (1 <<  1)


/* ttRamfsFindInfo
 *
 *  Contains find information structure.
 *
 *  Members
 *      ulFlags : Combination of following file attributes
 *          TM_RAMFS_ATTRIB_DIR : The file is a directory
 *          TM_RAMFS_ATTRIB_ARCH : The file should be acrhived
 *          TM_RAMFS_ATTRIB_RDONLY : The file is read-only file
 *          TM_RAMFS_ATTRIB_SYSTEM : The file is a system file or directory
 *          TM_RAMFS_ATTRIB_HIDDEN : The file is hidden
 *      lSize : The file size
 *      szName : The file name
 */
LINT_UNREF_MEMBER_BEGIN
typedef struct tsRamfsFindInfo
{
    tt32Bit         rfiUlFlags;
    ttRamfsSize     rfiLSize;
    ttRamfsBlock    rfiBlEntry;
    char            TM_FAR rfiSzName[TM_RAMFS_MAXFNAME];
} ttRamfsFindInfo, TM_FAR * ttRamfsFindInfoPtr;
LINT_UNREF_MEMBER_END

/* ttRamfsDirInfo
 *
 *  Simplified ttRamfsDirEntry that contains directory
 *  information structure (used to read directories)
 *
 *  Members
 *      ulFlags : Combination of following file attributes
 *          TM_RAMFS_ATTRIB_DIR : The file is a directory
 *      cReserved : Reserved to fit with the original structure
 *      szName : The file name
 */
LINT_UNREF_MEMBER_BEGIN
typedef struct tsRamfsDirInfo
{
    tt32Bit             rdiUlFlags;
/* Has to match number of ttRamFsBlock in ttRamFsEntry */
    ttRamfsBlock        TM_FAR rdiReserved[6];
    char                TM_FAR rdiSzName[TM_RAMFS_MAXFNAME];
} ttRamfsDirInfo, TM_FAR * ttRamfsDirInfoPtr;
LINT_UNREF_MEMBER_END

/* ttRamfsEntry
 *
 *  Contains informations about an entry.
 *
 *  Members
 *      ulFlags : Combination of following entry attributes
 *          TM_RAMFS_ATTRIB_DIR : The entry is a directory
 *          TM_RAMFS_ATTRIB_ARCH : The entry should be acrhived
 *          TM_RAMFS_ATTRIB_RDONLY : The entry is read-only file
 *          TM_RAMFS_ATTRIB_SYSTEM : The entry is a system file or directory
 *          TM_RAMFS_ATTRIB_HIDDEN : The entry is hidden
 *      blParentEntry : Block address of the parent entry
 *      blAddress : Block address of the entry content
 *      blCount : Number of block used by the entry
 *      blLastBlockSize : Number of bytes used in the last block
 *          (zero-valued if the last block is full)
 *      blLastBlock : Address of the last block used by the entry
 *      blReserved : Reserved for alignment
 *      szName : Filename of the entry
 */
typedef struct tsRamfsEntryInternal
{
    tt32Bit             reiUlFlags;
    ttRamfsBlock        reiBlParentEntry;
    ttRamfsBlock        reiBlAddress;
    ttRamfsBlock        reiBlCount;
    ttRamfsBlock        reiBlLastBlockSize;
    ttRamfsBlock        reiBlLastBlock;
    ttRamfsBlock        reiReserved;
    char                TM_FAR reiSzName[TM_RAMFS_MAXFNAME];
} ttRamfsEntryInternal;

LINT_UNREF_MEMBER_BEGIN
typedef union tsRamfsEntry
{
    struct tsRamfsEntryInternal     TM_FAR reInternal;
    char                            TM_FAR reBlock[TM_RAMFS_BLOCKSIZE];
} ttRamfsEntry, TM_FAR * ttRamfsEntryPtr;
LINT_UNREF_MEMBER_END

#define reUlFlags           reInternal.reiUlFlags
#define reBlParentEntry     reInternal.reiBlParentEntry
#define reBlAddress         reInternal.reiBlAddress
#define reBlCount           reInternal.reiBlCount
#define reBlLastBlockSize   reInternal.reiBlLastBlockSize
#define reBlLastBlock       reInternal.reiBlLastBlock
#define reSzName            reInternal.reiSzName

/* ttRamfsRawBlock
 *
 *  A block content as it is in memory.
 *
 *  Members
 *      blBack : Back block linked with the block
 *      blNext : Next block linked with the block
 *      data : Content of the block
 */
LINT_UNREF_MEMBER_BEGIN
typedef struct tsRamfsRawBlock
{
    ttRamfsBlock        rrbBlBack;
    ttRamfsBlock        rrbBlNext;
    ttRamfsEntry        rrbEntry;
} ttRamfsRawBlock, TM_FAR * ttRamfsRawBlockPtr;
LINT_UNREF_MEMBER_END

/* ttRamfsDrive
 *
 *  The drive structure
 *
 *  Members
 *      ulFlags : Combination of the following flags
 *          TM_RAMFS_MOUNTED : The drive is mounted
 *      lSize : Size in byte of the drive
 *      blCount : Block count of the drive
 *      blCurDirAddress : Block address of the current directory
 *      entryDeleted : Entry of the deleted file
 *      szLabel : Label of the drive
 *      rawBlock : Address of the first block
 */
LINT_UNREF_MEMBER_BEGIN
typedef struct tsRamfsDrive
{
    tt32Bit         rdUlFlags;
    tt32Bit         rdLSize;
    ttRamfsBlock    rdBlCount;
    ttRamfsBlock    rdBlCurDirAddress;
    ttRamfsEntry    rdEntryDeleted;
    char            TM_FAR rdSzLabel[TM_RAMFS_MAXLABEL];
    ttRamfsRawBlock TM_FAR rdRawBlock[1];
} ttRamfsDrive, TM_FAR * ttRamfsDrivePtr;
LINT_UNREF_MEMBER_END

typedef struct tsRamfsFileHandle
{
    tt32Bit         rfhUlFlags;
    ttRamfsBlock    rfhBlEntry;
    ttRamfsSize     rfhLPosition;
} ttRamfsFileHandle, TM_FAR * ttRamfsFileHandlePtr;


#define TM_RAMFS_RAWBLOCKSIZE   sizeof(ttRamfsRawBlock)




/* tfRamfsCreate
 *
 *  Creates a file.
 *
 *  Parameters
 *      lpszFileName : Filename to create
 *      ulFlags : Combination of file access flags,
 *              share mode and file type (see tfRamfsOpen)
 *
 *  Return Values
 *      -1 : An error as occured (call tfRamfsErrNo for more details)
 *      Other values : File handle
 */
#define tfRamfsCreate(a, b) tfRamfsOpen(a, b | TM_RAMFS_OPEN_CREATE | \
                                    TM_RAMFS_OPEN_TRUNC)


/*
 * Treck Ram File System User Interface Functions
 */
TM_PROTO_EXTERN int tfRamfsErrNo(void);
TM_PROTO_EXTERN int tfRamfsInit(ttRamfsBlock blCount);
TM_PROTO_EXTERN int tfRamfsMountImage(ttVoidPtr pDriveBase,
                                      ttRamfsSize mallocSize);
TM_PROTO_EXTERN int tfRamfsTerminate(void);
TM_PROTO_EXTERN ttVoidPtr tfRamfsFindFirstFile(ttConstCharPtr lpszFileName,
                                               ttRamfsFindInfoPtr pFindInfo);
TM_PROTO_EXTERN int tfRamfsFindNextFile(ttVoidPtr hFind,
                                        ttRamfsFindInfoPtr pFindInfo);
TM_PROTO_EXTERN int tfRamfsFindClose(ttVoidPtr hFind);
TM_PROTO_EXTERN int tfRamfsGetCwd(ttCharPtr lpszDirName, int nBufLen);
TM_PROTO_EXTERN int tfRamfsChDir(ttConstCharPtr lpszDirName);
TM_PROTO_EXTERN int tfRamfsMkDir(ttConstCharPtr lpszDirName);
TM_PROTO_EXTERN int tfRamfsRmDir(ttConstCharPtr lpszDirName);
TM_PROTO_EXTERN int tfRamfsMatchFileName(ttConstCharPtr lpszFileName,
                          ttConstCharPtr lpszMask);
TM_PROTO_EXTERN int tfRamfsGetDiskFreeSpace(ttRamfsBlockPtr pBlockSize,
                                     ttRamfsBlockPtr pFreeBlockCount,
                                     ttRamfsBlockPtr pTotalBlockCount);
TM_PROTO_EXTERN int tfRamfsUnlink(ttConstCharPtr lpszFileName);
TM_PROTO_EXTERN int tfRamfsRename(ttConstCharPtr lpszOldPath,
                                  ttConstCharPtr lpszNewPath);
TM_PROTO_EXTERN int tfRamfsCopy( ttConstCharPtr lpszSrcPath,
                                 ttConstCharPtr lpszDestPath,
                                 int            nOverwrite);
TM_PROTO_EXTERN ttVoidPtr tfRamfsOpen(ttConstCharPtr lpszFileName,
                                      tt32Bit ulFlags);
TM_PROTO_EXTERN int tfRamfsFlush(ttVoidPtr pHandle);
TM_PROTO_EXTERN int tfRamfsClose(ttVoidPtr pHandle);
TM_PROTO_EXTERN void tfRamfsCloseAll(void);
TM_PROTO_EXTERN ttRamfsSize tfRamfsSeek(ttVoidPtr pHandle,
                                        ttRamfsSize lOffset,
                                        int nOrigin);
TM_PROTO_EXTERN ttRamfsSize tfRamfsTell(ttConstVoidPtr pHandle);
TM_PROTO_EXTERN int tfRamfsEof(ttConstVoidPtr pHandle);
TM_PROTO_EXTERN int tfRamfsSetEof(ttConstVoidPtr pHandle);
TM_PROTO_EXTERN int tfRamfsCommit(ttVoidPtr pHandle);
TM_PROTO_EXTERN ttRamfsSize tfRamfsFileLength(ttConstVoidPtr pHandle);
TM_PROTO_EXTERN ttRamfsSize tfRamfsRead( ttVoidPtr pHandle,
                                         ttVoidPtr pBuffer,
                                         ttRamfsSize lCount);
TM_PROTO_EXTERN ttRamfsSize tfRamfsWrite( ttVoidPtr pHandle,
                                          ttConstVoidPtr pBuffer,
                                          ttRamfsSize lCount);
TM_PROTO_EXTERN ttRamfsDrivePtr tfRamfsGetDrive(void);

#ifdef __cplusplus
}
#endif

#endif /* _TRRAMFS_H_ */
