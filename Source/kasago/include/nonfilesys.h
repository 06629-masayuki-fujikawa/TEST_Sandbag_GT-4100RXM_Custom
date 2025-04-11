#ifndef _KSG_TRRAMFS_H_
#define _KSG_TRRAMFS_H_

#ifdef __cplusplus
extern "C" {
#endif

/// * For suppressing PC-lint compile errors * /
LINT_UNREF_MACRO(TM_RAMFS_ATTRIB_ARCH)
LINT_UNREF_MACRO(TM_RAMFS_ATTRIB_RDONLY)
LINT_UNREF_MACRO(TM_RAMFS_ATTRIB_SYSTEM)
LINT_UNREF_MACRO(TM_RAMFS_ATTRIB_HIDDEN)

/// * RAM file system functions return code * /
#define TM_RAMFS_ERROR     -1
#define TM_RAMFS_OKAY       0

/// * File access flags * /
#define TM_RAMFS_ACCESS_READ        1
#define TM_RAMFS_ACCESS_WRITE       2

/// * File open mode * /
#define TM_RAMFS_OPEN_CREATE        0x10
#define TM_RAMFS_OPEN_EXCL          0x20
#define TM_RAMFS_OPEN_TRUNC         0x40
#define TM_RAMFS_OPEN_APPEND        0x80

/// * File share mode * /
#define TM_RAMFS_SHARE_READ         0x100
#define TM_RAMFS_SHARE_WRITE        0x200

/// * File type * /
#define TM_RAMFS_TYPE_BINARY        0
#define TM_RAMFS_TYPE_TEXT          0x1000

/// * Seek method constants * /
#define TM_RAMFS_SEEK_SET           0
#define TM_RAMFS_SEEK_CUR           1
#define TM_RAMFS_SEEK_END           2


#define TM_RAMFS_BLOCKSIZE          128

#define TM_RAMFS_MAXFNAME           (TM_RAMFS_BLOCKSIZE - sizeof(tt32Bit) \
                                    - 6 * sizeof(ttRamfsBlock))
#define TM_RAMFS_MAXPATH            260

#define TM_RAMFS_NOLINK             ((ttRamfsBlock)-1)

/// * Directory entry flags * /
#define TM_RAMFS_ATTRIB_DIR         0x1
#define TM_RAMFS_ATTRIB_ARCH        0x2
#define TM_RAMFS_ATTRIB_RDONLY      0x4
#define TM_RAMFS_ATTRIB_SYSTEM      0x8
#define TM_RAMFS_ATTRIB_HIDDEN      0x10

typedef ttUShort ttRamfsBlock;
typedef ttS32Bit  ttRamfsSize;
typedef ttRamfsBlock TM_FAR * ttRamfsBlockPtr;
typedef ttRamfsSize TM_FAR * ttRamfsSizePtr;


/// * Low level parameters * /
#define TM_RAMFS_DIRSEP             '/'
#define TM_RAMFS_INVALIDCHAR        "\\/ *?\"<>|"
#define TM_RAMFS_MAXFILES           64

/// * Disk flags * /
#define TM_RAMFS_MOUNTED            1


// 検索用構造体
typedef struct tsRamfsFindInfo
{
    tt32Bit         ulFlags;
    ttRamfsSize     lSize;
    char            TM_FAR szName[TM_RAMFS_MAXFNAME];
} ttRamfsFindInfo, TM_FAR * ttRamfsFindInfoPtr;

// ファイル/ディレクトリ
typedef struct tsRamfsFileHandle
{
    tt32Bit         ulFlags;
    tt32Bit         lFileSize;
    char            TM_FAR szName[TM_RAMFS_MAXFNAME];
    ttRamfsSize     lReadPosition;
    short           number;
} ttRamfsFileHandle, TM_FAR * ttRamfsFileHandlePtr;

// ドライブ
typedef struct tsRamfsDrive
{
    tt32Bit         ulFlags;
    ttLockEntry     locEntry;
    tt32Bit         lSize;
    ttRamfsBlock    blCount;
    ttRamfsFileHandlePtr    ptrCurDirHandle;
	
} ttRamfsDrive, TM_FAR * ttRamfsDrivePtr;


/// *
// * Treck Ram File System User Interface Functions
// * /
TM_PROTO_EXTERN int tfRamfsErrNo(void);
TM_PROTO_EXTERN int tfRamfsInit(ttRamfsBlock blCount);
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

#endif // _KSG_TRRAMFS_H_
