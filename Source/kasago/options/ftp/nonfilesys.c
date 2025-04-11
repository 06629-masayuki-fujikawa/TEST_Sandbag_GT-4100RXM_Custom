/*
	ファイルシステムがないので、独自に組み込む
*/


#include <trsocket.h>
#include <trmacro.h>


#ifdef TM_USE_RAM_FS

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#include "nonfilesys.h"
#include "system.h"
#include "mem_def.h"
#include "subtype.h"
#include "AppServ.h"
#include "flp_def.h"
#include "fla_def.h"
#include "ftpctrl.h"
#include "message.h"
#include "prm_tbl.h"
#include "mnt_def.h"
#include "subtype.h"
#include "remote_dl.h"
#include "updateope.h"

#define	_STRCPY(a,b)	strcpy((char*)a, (char*)b)
#define	_STRLEN(a)		(ushort)strlen((char*)a)
#define	_STRICMP(a,b)	stricmp((const char*)a, (const char*)b)

#define TM_RAMFS_READ_MAX_SIZE		128		// ToDo:読み込むサイズ
#define TM_RAMFS_WRITE_MAX_SIZE		128		// ToDo:書き込むサイズ

/* Column position for printing <DIR> in sub-directory directory entry */
#define TM_DIR_POS   15

/* Column position for printing size of file entry */
#define TM_SIZE_POS   24

/* Column position for printing file name */
#define TM_FILE_POS   39

/* This entry is a directory */
#define tm_fs_is_dir(fsDirPtr) (fsDirPtr->ulFlags & TM_RAMFS_ATTRIB_DIR)

/* Get the file name pointer of the entry */
#define tm_fs_file_name_ptr(fsDirPtr) (fsDirPtr->szName)

/* Get the file size of the entry */
#define tm_fs_file_size(fsDirPtr) (fsDirPtr->lSize)

#define tm_findnext( handle, dirPtr ) \
                     _findnext( handle, dirPtr )

// static 宣言部
static	long	Lock_Info_Size_Get(void);
static	long	Car_Sts_Size_Get(void);
static	long	Prog_Info_Size_Get(void);
static	long	MP_Data_Size_Get(void);
// MH810100(S) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
//static	long	MV_Data_Size_Get(void);
// MH810100(E) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
static	long	Main_Prog_Size_Get(void);
// MH810100(S) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
//static	long	SV_Data_Size_Get(void);
// MH810100(E) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
static	long	SP_Data_Size_Get(void);
static	long	Sale_Data_Size_Get(void);
static	long	Total_Data_Size_Get(void);
static	long	Err_Data_Size_Get(void);
static	long	Arm_Data_Size_Get(void);
static	long	Moni_Data_Size_Get(void);
static	long	Ope_Data_Size_Get(void);
static	long	Logo_Size_Get(void);
static	long	Header_Size_Get(void);
static	long	Footer_Size_Get(void);
static	long	Uketuke_Size_Get(void);
static	long	Shomei_Size_Get(void);
static	long	Kamei_Size_Get(void);
static	long	Edy_AT_Size_Get(void);
static	long	T_Footer_Size_Get(void);
static	long	AZU_Footer_Size_Get(void);
// 仕様変更(S) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
static	long	CreKBR_Footer_Size_Get(void);
static	long	EpayKBR_Footer_Size_Get(void);
// 仕様変更(E) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
static	long	Future_Footer_Size_Get(void);
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
static	long	EMG_Footer_Size_Get(void);
// MH810105(E) MH364301 QRコード決済対応
static	long	Chk_Result_Size_Get(void);
static	long	dummy_Size_Get(void);
static	long	RemoteDl_Size_Get(void);
static	uchar	fileOpenChk( const char *pname  );
static	void	tfRamfsGetEntryInfo( ttRamfsDrivePtr    ramfsPtr,
                                 ttRamfsFindInfoPtr pFindInfo,
                                 ttRamfsBlock       blEntry);
static	void	SetDefaultFileName( ttVoidPtr pHandle );

extern	ulong	DataSizeCheck( char *data, uchar kind );
extern	int		ftp_check_stby_param(void);
extern	long	ftp_on_file_send( void*, long );

extern	unsigned char	SysMnt_Work[SysMnt_Work_Size];		// 64KB

extern	long	Get_Etc_File_Existence( short no );
extern	uchar	ftp_file_read(char *file);

// MH810100(S) K.Onodera 2020/03/23 車番チケットレス(#4086 FTPサーバーにﾌﾟﾛｸﾞﾗﾑをアップロードできない)
//#define		PROG_INDEX  14
//#define		PROG_MAX  	3
#define		PROG_INDEX  12
#define		PROG_MAX  	1
// MH810100(E) K.Onodera 2020/03/23 車番チケットレス(#4086 FTPサーバーにﾌﾟﾛｸﾞﾗﾑをアップロードできない)

typedef long (*FTP_SIZE_GET)(void);

static	const	struct {
	FTP_SIZE_GET	func;
	char	TM_FAR*	szName;
	tt32Bit			ulFlags;
	short			number;
} dirs_files[] = {
	// ディレクトリ
/* 0 */		{dummy_Size_Get,		"/",							TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
/* 1 */		{dummy_Size_Get,		"/SYS",							TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
/* 2 */		{dummy_Size_Get,		"/SYS/ACT",						TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
// MH810100(S) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
///* 3 */		{dummy_Size_Get,		"/SYS/ACT/VOICE",				TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
// MH810100(E) Y.Yamauchi 2020/03/05 車番チケットレス（メンテナンス
/* 3 */		{dummy_Size_Get,		"/SYS/STBY",					TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
/* 4 */		{dummy_Size_Get,		"/SYS/STBY/MAIN",				TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
// MH810100(S) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
///* 6 */		{dummy_Size_Get,		"/SYS/STBY/VOICE",				TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
// MH810100(E) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
/* 5 */		{dummy_Size_Get,		"/LOG",							TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
/* 6 */		{dummy_Size_Get,		"/ETC",							TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
/* 7 */		{dummy_Size_Get,		"/TMP",							TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
	// ファイル
/*8 */		{Lock_Info_Size_Get,	"/SYS/LockInfo.csv",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_LOCKPARA		},
/*9 */		{Car_Sts_Size_Get,		"/SYS/CarStatusInfo.csv",		TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_CAR_INFO		},
/*10 */		{Prog_Info_Size_Get,	"/SYS/prog.txt",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_PROG			},
/*11 */		{MP_Data_Size_Get,		"/SYS/ACT/Parameter.csv",		TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT						 ,	LD_PARA			},
// MH810100(S) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
///*14 */		{MV_Data_Size_Get,		"/SYS/ACT/VOICE/",				TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_BINARY						 ,	LD_VOICE_A		},
// MH810100(E) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
/*12 */		{Main_Prog_Size_Get,	"/SYS/STBY/MAIN/",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_BINARY,LD_MAIN_S		},
// MH810100(S) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
///*16 */		{SV_Data_Size_Get,		"/SYS/STBY/VOICE/",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_BINARY,LD_VOICE_S		},
// MH810100(E) Y.Yamauchi 2020/03/05 車番チケットレス(メンテナンス)
/*13 */		{SP_Data_Size_Get,		"/SYS/STBY/Parameter.csv",		TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_PARA_S		},
/*14 */		{Sale_Data_Size_Get,	"/LOG/SaleLog.bin",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_BINARY,LD_SLOG			},
/*15 */		{Total_Data_Size_Get,	"/LOG/TotalLog.bin",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_BINARY,LD_TLOG			},
/*16 */		{Err_Data_Size_Get,		"/LOG/ErrLog.csv",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_ERR			},
/*17 */		{Arm_Data_Size_Get,		"/LOG/ArmLog.csv",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_ARM			},
/*18 */		{Ope_Data_Size_Get,		"/LOG/MonitorLog.csv",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_MONI			},
/*19 */		{Moni_Data_Size_Get,	"/LOG/OpeMonitorLog.csv",		TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_OPE_MONI		},
/*20 */		{dummy_Size_Get,		"/LOG/RemoteDLResult.txt",		TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_REMOTE_RES	},
/*21 */		{Logo_Size_Get,			"/ETC/Logo.bmp",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_BINARY,LD_LOGO			},
/*22 */		{Header_Size_Get,		"/ETC/Header.txt",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_HDR			},
/*23 */		{Footer_Size_Get,		"/ETC/Footer.txt",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_FTR			},
/*24 */		{Uketuke_Size_Get,		"/ETC/uketuke.txt",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_ACCEPTFTR	},
/*25 */		{Shomei_Size_Get,		"/ETC/shomeiran.txt",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_SYOM			},
/*26 */		{Kamei_Size_Get,		"/ETC/kameiten.txt",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_KAME			},
/*27 */		{Edy_AT_Size_Get,		"/ETC/EdyAtCmd.txt",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_EDYAT		},
/*28 */		{T_Footer_Size_Get,		"/ETC/TCardFooter.txt",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_TCARD_FTR	},
/*29 */		{AZU_Footer_Size_Get,	"/ETC/AzukariFooter.txt",		TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_AZU_FTR		},
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
/*30 */		{CreKBR_Footer_Size_Get,	"/ETC/CreKabaraiFooter.txt",	TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_CREKBR_FTR	},
/*31 */		{EpayKBR_Footer_Size_Get,	"/ETC/EpayKabaraiFooter.txt",	TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_EPAYKBR_FTR	},
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
/*32 */		{Future_Footer_Size_Get,	"/ETC/GojitsuFooter.txt",	TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_FUTURE_FTR	},
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
/*33 */		{dummy_Size_Get,		"/TMP/update10.txt",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_TYPE_TEXT						 ,	LD_AU_SCR		},
/*34 */		{Chk_Result_Size_Get,	(char*)Chk_info.dir_chkresult,			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_CHKMODE_RES	},
/*35 */		{dummy_Size_Get,		"/REMOTEDL",					TM_RAMFS_ATTRIB_DIR | TM_RAMFS_ACCESS_READ | TM_RAMFS_ACCESS_WRITE,	0	},
/*36 */		{RemoteDl_Size_Get,		"/REMOTEDL/",					TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_BINARY,LD_REMOTEDL		},
/*37 */		{dummy_Size_Get,		"/TMP/PARAMETER.TXT",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_TYPE_TEXT						 ,	LD_PARAM_UP		},
/*38 */		{dummy_Size_Get,		"/TMP/CONNECT.TXT",				TM_RAMFS_ACCESS_WRITE | TM_RAMFS_TYPE_TEXT						 ,	LD_CONNECT		},
// MH810100(S) K.Onodera 2019/11/28 車番チケットレス（→LCD パラメータ)
/*39 */		{dummy_Size_Get,		"/TMP/PARAMETER.DAT",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_TYPE_TEXT						 ,	LD_PARAM_UP_LCD	},
// MH810100(E) K.Onodera 2019/11/28 車番チケットレス（→LCD パラメータ)
// MH810105(S) MH364301 QRコード決済対応
/*40 */		{EMG_Footer_Size_Get,	"/ETC/emgfooter.txt",			TM_RAMFS_ACCESS_WRITE | TM_RAMFS_ACCESS_READ | TM_RAMFS_TYPE_TEXT,	LD_EMG			},
// MH810105(E) MH364301 QRコード決済対応
};

const char* MONTH_TBL[] = {
	"Dmy",	// Dammy
	"Jan",	// 1月
	"Feb",	// 2月
	"Mar",	// 3月
	"Apr",	// 4月
	"May",	// 5月
	"Jun",	// 6月
	"Jul",	// 7月
	"Aug",	// 8月
	"Sep",	// 9月
	"Oct",	// 10月
	"Nov",	// 11月
	"Dec",	// 12月
};

#define	LIST_REGIST_MAX	(sizeof(dirs_files) / sizeof(dirs_files[0]))
#define	LIST_AUTH_DIR	"drwxrwxrwx"
#define	LIST_AUTH_RW_DATA	"-rw-rw-rw-"
#define	LIST_AUTH_R_DATA	"-r--r--r--"

#define	LIST_FORMAT	"%s   1 owner    group           %ld %s %02d %02d:%02d %s"

char	temp_buff[256];

static const char	ip_adrr[2][4]={
	{192,168,1,1},
	{192,168,10,4}
};

static char filepath[TM_RAMFS_MAXFNAME];

#define ttDirEntry   ttRamfsFindInfo
typedef ttDirEntry TM_FAR * ttDirEntryPtr;

typedef struct tsUserLogin
{
    char             userName[20];
    char             userPassword[20];
} ttUserLogin;
typedef const ttUserLogin TM_FAR * ttUserLoginPtr;

// MH810100(S) K.Onodera 2019/11/15 車番チケットレス（→LCD パラメータ)
//static	const ttUserLogin  TM_CONST_QLF tlFsNamePasswd[2] =
static	const ttUserLogin  TM_CONST_QLF tlFsNamePasswd[3] =
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス（→LCD パラメータ)
{
/*   username,      password    */
    {"AMANO Parking",       "0454011441"     },				//
    {"", ""},
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス（→LCD パラメータ)
	{"amano",				"0454011441"	 },				// LCDログイン
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス（→LCD パラメータ)
};

#define TM_FS_WORKINGDIR_SIZE \
            256 - 3 * sizeof(ttVoidPtr) - sizeof(int) - sizeof(ttDirEntry)

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
 * local functions
 */
static void tfFSUserDir(ttConstUserFsDataPtr userFsDataPtr);


#define tm_ramfs_fhandle(x)    ((ttRamfsFileHandlePtr)x)

/*
 * Treck Ram File System File Handling Functions
 */
static int tfRamfsAddFileHandle(ttVoidPtr pHandle);
//static int tfRamfsRemoveFileHandle(ttConstVoidPtr pHandle);
static int tfRamfsIsValidFileHandle(ttConstVoidPtr pHandle);
//static int tfRamfsFindFileHandle(ttRamfsBlock blEntry, ttVoidPtrPtr pHandle);
static ttVoidPtr tfRamfsOpenDir(ttUserFsDataPtr     userFsDataPtr,
                                const char TM_FAR * pathNamePtr,
                                int                 flag);
static ttVoidPtr tfRamfsFileFind (const char TM_FAR * pathNamePtr);


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
//static int tfRamfsRemoveFileHandle(ttConstVoidPtr pHandle)
//{
//    ttVoidPtrPtr    pHandleList;
//    int             retCode;
//    int             counter;
//
//    tlLastError = TM_EMFILE;
//    retCode = TM_RAMFS_ERROR;
//    pHandleList = tlFileHandles;
//
///* Parse the whole handle list and look for a free entry */
//    for(counter = 0; counter < TM_RAMFS_MAXFILES; counter++)
//    {
//        if((*pHandleList) == pHandle)
//        {
//            tlLastError = TM_ENOERROR;
//            retCode = TM_RAMFS_OKAY;
///* Put the handle in the free entry */
//            *pHandleList = TM_VOID_NULL_PTR;
//            break;
//        }
//        else
//        {
//            pHandleList++;
//        }
//    }
//
//    return retCode;
//}


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

    if (pHandle == TM_VOID_NULL_PTR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
        /* Parse for the whole handle list and look for the asked handle */
        while (pHandleList[0] != pHandle)
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
//   ttRamfsBlock            blEntry;
    ttRamfsFileHandlePtr    pHandle;
	char			ret;
	short			msg;

    tlLastError = TM_ENOERROR;
	ret = GetSnd_RecvStatus();
    pHandle = tfRamfsFileFind(lpszFileName);
    if (pHandle != (ttVoidPtr)0)
    {
		if( fileOpenChk( lpszFileName ) )
		{
			if( ret == FCMD_REQ_SEND ){
				tm_strcpy(lpszFileName, pHandle->szName);
				ret = ftp_file_read( (char *)lpszFileName );
				if( ret == FRESP_NOFILE){					// ファイル名と一致しない場合
					tlLastError = TM_ENOENT;
					msg = FTP_NOTFILE_R;
				}else{
					msg = FTP_SENDING;
				}
			}else if( ret == FCMD_REQ_RECV ){
				tm_strcpy(lpszFileName, pHandle->szName);
				ret = ftp_file_write( (char*)lpszFileName );
				if( ret == FRESP_WP ){
					msg = FTP_NOTFILE_W;
				}else{
					msg = FTP_RECVING;
				}
			}else{
  		      tlLastError = TM_ENOENT;
  		      pHandle = (ttRamfsFileHandlePtr)0;
  			  return (ttVoidPtr)pHandle;
			}
			FTPmsgSend( msg, NULL );
		}
    }else{
        tlLastError = TM_ENOENT;
		if( ret == FCMD_REQ_SEND ){
			msg = FTP_NOTFILE_R;
		}else if( ret == FCMD_REQ_RECV ){
			msg = FTP_NOTFILE_W;
		}
		FTPmsgSend( msg, NULL );
   }

    if ((tlLastError != TM_ENOERROR) && (pHandle != TM_VOID_NULL_PTR))
    {
        pHandle = (ttRamfsFileHandlePtr)0;
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
	char	sts;
	retCode = TM_RAMFS_OKAY;
	sts = GetSnd_RecvStatus();
//	if( sts == FCMD_REQ_SEND ){
//		tfRamfsRead( pHandle, NULL , 0 );
//	}else if( sts == FCMD_REQ_RECV ){
	if( sts == FCMD_REQ_SEND ){
	}else if( sts == FCMD_REQ_RECV ){
		tfRamfsWrite( pHandle, NULL , 0 );
	}else{
      retCode = TM_ENOERROR;
	}
	SetDefaultFileName( pHandle );
	SetSnd_RecvStatus(FCMD_REQ_IDLE);
    return retCode;
}


/*
 * tfRamfsCloseAll
 *    Closes each opened file.
 */
void tfRamfsCloseAll()
{
    int i;

/* Parse the handle list */
    for (i = 0; i < TM_RAMFS_MAXFILES; i++)
    {
        if (tlFileHandles[i] != TM_VOID_NULL_PTR)
        {
/* If entry isn't free then close the handle from the list */
            (void)tfRamfsClose(tlFileHandles[i]);
        }
    }
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
        tlLastError = TM_ENOERROR;
        position = tm_ramfs_fhandle(pHandle)->lReadPosition;
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

    length = TM_RAMFS_ERROR;
    if (tfRamfsIsValidFileHandle(pHandle) == 0)
    {
        tlLastError = TM_ENOERROR;
        length = tm_ramfs_fhandle(pHandle)->lFileSize;
    }
    else
    {
        tlLastError = TM_EBADF;
    }
    return length;
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
    ttRamfsSize     lRead;

    lRead  = 0;
    tlLastError = TM_ENOERROR;
    if (tfRamfsIsValidFileHandle(pHandle) == 0)
    {
        if (tm_ramfs_fhandle(pHandle)->ulFlags & TM_RAMFS_ACCESS_READ)
        {
			// lPos現在の読み込んだサイズ
			lRead = ftp_on_file_send( pBuffer, lCount );
        }
        else
        {
            tlLastError = TM_EACCES;
        }
    }
    else
    {
        tlLastError = TM_EBADF;
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
    ttRamfsSize     lWritten;
    ttRamfsFileHandlePtr    fHandle = tm_ramfs_fhandle(pHandle);
	
    tlLastError  = TM_ENOERROR;
    lWritten    = 0;
    if (tfRamfsIsValidFileHandle(pHandle) == 0)
    {
        if (fHandle->ulFlags & TM_RAMFS_ACCESS_WRITE)
        {
			if( ftpRecvData((uchar *)pBuffer,(unsigned short)lCount) ){
				tlLastError = TM_EBADF;
			}
            lWritten = lCount;
	    }
        else
        {
            tlLastError = TM_EACCES;
			FTPmsgSend( FTP_NOTFILE_W, NULL );
        }
    }
    else
    {
        tlLastError = TM_EBADF;
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
	int				i;
    int             retCode;

    ttRamfsSize     mallocSize;
    ttRamfsDrivePtr ramfsPtr;
    ttRamfsFileHandlePtr    pHandle;

    tlLastError = TM_ENOERROR;
    retCode = TM_RAMFS_OKAY;

    if (blCount == 0)
    {
        tlLastError = TM_EINVAL;
        retCode = TM_RAMFS_ERROR;
    }
    else if (tm_global(tvRamfsPtr) != TM_VOID_NULL_PTR)
    {
		// 初期化済み
        tlLastError = TM_EACCES;
        retCode = TM_RAMFS_ERROR;
    }
    else
    {
		// ドライブとして
        mallocSize = sizeof(ttRamfsDrive);
        tm_global(tvRamfsPtr) = (ttVoidPtr)tm_kernel_malloc((unsigned int)mallocSize);
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
			//ロック
            tm_call_lock_wait(&ramfsPtr->locEntry);
            tm_bzero(tlFileHandles, sizeof(ttVoidPtr) * TM_RAMFS_MAXFILES);

			// 一応サイズを持っておく
            ramfsPtr->lSize = mallocSize;
			ramfsPtr->blCount = blCount;

			// ファイルとディレクトリの構成の組み立て処理			
			for (i = 0; i < LIST_REGIST_MAX ; i++) {
	     	   	pHandle = tm_malloc(sizeof(ttRamfsFileHandle));
		        if (pHandle == TM_VOID_NULL_PTR)
		        {
		            tlLastError = TM_ENOBUFS;
					break;
				}else{
			        (void)tfRamfsAddFileHandle(pHandle);
					pHandle->lReadPosition	= 0;
					pHandle->lFileSize 		= dirs_files[i].func();
					_STRCPY(pHandle->szName, dirs_files[i].szName);
					pHandle->ulFlags 		= dirs_files[i].ulFlags;
					pHandle->number			= dirs_files[i].number;
				}
			}
			// ロック解除
            tm_call_unlock(&ramfsPtr->locEntry);
        }
    }

    return retCode;
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
    int             retCode;
    ttVoidPtr       ramfsPtr;

    if (tm_global(tvRamfsPtr) != TM_VOID_NULL_PTR)
    {
/* Free the memory allocated for the drive */
        ramfsPtr = tm_global(tvRamfsPtr);
        tm_global(tvRamfsPtr) = TM_VOID_NULL_PTR;
        tm_kernel_free(ramfsPtr);
        retCode = TM_RAMFS_OKAY;
    }
    else
    {
        retCode = TM_RAMFS_ERROR;
    }

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
//    ttRamfsBlock    blAddress;
//    ttVoidPtr       hFind;
    int             retCode;
    ttRamfsDrivePtr ramfsPtr;
    ttRamfsFileHandlePtr    pHandle;

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    retCode = TM_RAMFS_OKAY;

    if (ramfsPtr == (ttRamfsDrivePtr)0)
    {
        tlLastError = TM_ENOENT;
    }
    else
    {
        tlLastError = TM_ENOERROR;

	    pHandle = tfRamfsFileFind(lpszDirName);
        if (pHandle != (ttVoidPtr)0)
        {
            if ((tm_ramfs_fhandle(pHandle)->ulFlags & TM_RAMFS_ATTRIB_DIR) == 0)
            {
                tlLastError = TM_ENOTDIR;
            }
            else
            {
                ramfsPtr->ptrCurDirHandle = pHandle;
            }
        }
        else
        {
            tlLastError = TM_ENOENT;
        }
    }
    if (tlLastError != TM_ENOERROR)
    {
        retCode = TM_RAMFS_ERROR;
    }
    return retCode;
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
//static int tfRamfsIsValidFileName(ttConstCharPtr lpszFileName, int nDirectory)
//{
//    int retCode;
//    int finished;
//
///* check for acceptable file name length */
//    if(tm_strlen(lpszFileName)>=TM_RAMFS_MAXFNAME)
//    {
//        retCode = TM_RAMFS_ERROR;
//        finished = 1;
//    }
//    else
//    {
//        retCode = TM_RAMFS_OKAY;
//        finished = 0;
//
//        switch(lpszFileName[0])
//        {
///* file name is null, return error */
//        case '\0':
//            retCode = TM_RAMFS_ERROR;
//            finished = 1;
//            break;
//
//        case '.':
///* file name begin with '.' */
//            switch(lpszFileName[1])
//            {
//            case '\0':
///* file name = '.', if not allowed, return error, else return okay */
//                if (nDirectory == 0)
//                {
//                    retCode = TM_RAMFS_ERROR;
//                }
//                finished = 1;
//                break;
//
//            case '.':
//                if (lpszFileName[2] == '\0')
//                {
///* file name = '..', if not allowed, return error, else return okay */
//                    if (nDirectory == 0)
//                    {
//                        retCode = TM_RAMFS_ERROR;
//                    }
//                    finished = 1;
//                    break;
//                }
//                break;
//
//            default:
//                break;
//            }
//            break;
//
//        default:
//            break;
//        }
//    }
//
///* all other cases, check for invalid char */
//    while((lpszFileName[0] != '\0')&&(!finished))
//    {
//       if (tm_strchr(TM_RAMFS_INVALIDCHAR, lpszFileName[0])
//                                                         != TM_VOID_NULL_PTR)
//        {
//            retCode = TM_RAMFS_ERROR;
//            finished = 1;
//        }
//        lpszFileName++;
//    }
//
//    return retCode;
//}



typedef struct tsRamfsPrivateFindStruct
{
    char szFileName[TM_RAMFS_MAXPATH];
    ttConstCharPtr lpszName;
    ttRamfsBlock blBaseDir;
    ttRamfsBlockPtr pblNext;
} ttRamfsPrivateFindStruct, TM_FAR *ttRamfsPrivateFindStructPtr;




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
    return 0;
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
    int               i;
    int               retCode;

    userFsDataPtr = (ttUserFsDataPtr)0;
    retCode = -1;
	i = 0;
	if( GetFTPMode() == _FTP_CLIENT && (DOWNLOADING() || remotedl_script_typeget() == PARAMTER_SCRIPT) ){
		i = 1;
	}
// MH810100(S)
	else if( remotedl_script_typeget() == LCD_NO_SCRIPT ){
		i = 2;
	}
// MH810100(E)

    if (    tm_strcmp(userNamePtr, tlFsNamePasswd[i].userName) == TM_STRCMP_EQUAL )
    {
        if (    (tlFsNamePasswd[i].userPassword[0] == '\0') || 
				(tm_strcmp( passwordPtr,
                            tlFsNamePasswd[i].userPassword ) == TM_STRCMP_EQUAL ) )
        {
            retCode = 0;
        }
    }
	if ((tlFsNamePasswd[i].userName[0] == '\0') && (tlFsNamePasswd[i].userPassword[0] == '\0')) {
		retCode = 0;
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
			// この通知はサーバーでしか使用しない
			FTPmsgSend( FTP_LOGIN, (void*)userFsDataPtr );
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
        tfFSUserDir(userDataPtr);
        tm_free_raw_buffer(userDataPtr);
		// この通知はサーバーでしか使用しない
		FTPmsgSend( FTP_LOGOUT, NULL );
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
	int size;

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

int tfFSRenameFile (const void TM_FAR * userDataPtr,
                    const char TM_FAR * fromPathNamePtr,
                    const char TM_FAR * toPathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(fromPathNamePtr);
    TM_UNREF_IN_ARG(toPathNamePtr);

    return -1;
}


int tfFSGetUniqueFileName ( void TM_FAR * userDataPtr,
                            char TM_FAR * bufferPtr,
                            int           bufferSize )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(bufferSize);

    return -1;

}


int tfFSDeleteFile (const void TM_FAR * userDataPtr,
                    const char TM_FAR * pathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);

    return -1;
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
        if( GetFTPMode() == _FTP_SERVER ){
        	SetSnd_RecvStatus( FCMD_REQ_SEND );
        }
        break;
    case TM_FS_WRITE:
        fileFlag =  TM_RAMFS_ACCESS_WRITE |
                    TM_RAMFS_SHARE_WRITE  |
                    TM_RAMFS_OPEN_CREATE  |
                    TM_RAMFS_OPEN_TRUNC   |
                    TM_RAMFS_TYPE_BINARY;
       	if( GetFTPMode() == _FTP_SERVER ){
	       	SetSnd_RecvStatus( FCMD_REQ_RECV );
       	}
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


    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        tm_call_lock_wait(&ramfsPtr->locEntry);
        tfFSUserDir(userFsDataPtr);
        if(*pathNamePtr != '/') {
            memset( filepath, 0, sizeof( filepath ));
            tm_strcpy(filepath, userFsDataPtr->userWorkingDirectory);
            if(tm_strlen(filepath) > 1) {
                tm_strcat(filepath, "/");
            }
            tm_strcat(filepath, pathNamePtr);
            fileHandle = (void TM_FAR *)tfRamfsOpen(filepath, fileFlag);
        }
        else {
            fileHandle = (void TM_FAR *)tfRamfsOpen(pathNamePtr, fileFlag);
        }
        tm_call_unlock(&ramfsPtr->locEntry);
    }
    else
    {
        fileHandle = (ttVoidPtr)0;
    }
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

    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        tm_call_lock_wait(&ramfsPtr->locEntry);
        retCode = (int)tfRamfsRead(fileDataPtr, bufferPtr, bufferSize);
        tm_call_unlock(&ramfsPtr->locEntry);
    }
    else
    {
        retCode = -1;
    }
    return retCode;
}


int tfFSReadFileRecord ( void TM_FAR * userDataPtr,
                         void TM_FAR * fileDataPtr,
                         char TM_FAR * bufferPtr,
                         int           bufferSize,
                         int  TM_FAR * eorPtr )
{
/* Not needed on DOS. We use the TM_FS_CRLF_FLAG */
//    TM_UNREF_IN_ARG(userDataPtr);
//    TM_UNREF_IN_ARG(fileDataPtr);
//    TM_UNREF_IN_ARG(bufferPtr);
//    TM_UNREF_IN_ARG(bufferSize);
//    TM_UNREF_IN_ARG(eorPtr);
//
    return tfFSReadFile( userDataPtr, fileDataPtr, bufferPtr, bufferSize );
;
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

    retCode = -1;
    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        tm_call_lock_wait(&ramfsPtr->locEntry);
        length = (int)tfRamfsWrite(fileDataPtr,bufferPtr,bytes);
        tm_call_unlock(&ramfsPtr->locEntry);
        if (length == bytes)
        {
            retCode = 0;
        }
    }
    return retCode;
}

int tfFSWriteFileRecord ( void TM_FAR * userDataPtr,
                          void TM_FAR * fileDataPtr,
                          char TM_FAR * bufferPtr,
                          int           bytes,
                          int           eor )
{
/* Not needed on DOS. We use the TM_FS_CRLF_FLAG */
//    TM_UNREF_IN_ARG(userDataPtr);
//    TM_UNREF_IN_ARG(fileDataPtr);
//    TM_UNREF_IN_ARG(bufferPtr);
//    TM_UNREF_IN_ARG(bytes);
//    TM_UNREF_IN_ARG(eor);
    return tfFSWriteFile( userDataPtr, fileDataPtr, bufferPtr, bytes );
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
    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        tm_call_lock_wait(&ramfsPtr->locEntry);
        (void)tfRamfsFlush(fileDataPtr);
        tm_call_unlock(&ramfsPtr->locEntry);
    }
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
    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
    if (ramfsPtr != (ttRamfsDrivePtr)0)
    {
        tm_call_lock_wait(&ramfsPtr->locEntry);
        (void)tfRamfsClose(fileDataPtr);
        tm_call_unlock(&ramfsPtr->locEntry);
    }
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

/* This command is only valid if a user has logged in. */
    retCode = -1;
    if (userDataPtr != TM_VOID_NULL_PTR)
    {
        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
        if (ramfsPtr != (ttRamfsDrivePtr)0)
        {
            tm_call_lock_wait(&ramfsPtr->locEntry);
            tfFSUserDir(userFsDataPtr);
            strcpy( temp_buff, pathNamePtr );
            if( !tm_strcmp( temp_buff, ".." )){
				retCode = tm_strlen(tm_ramfs_fhandle(ramfsPtr->ptrCurDirHandle)->szName);
				memcpy( temp_buff, tm_ramfs_fhandle(ramfsPtr->ptrCurDirHandle)->szName, retCode );
				for( ; (retCode-1) != 0; retCode-- ){
					if( temp_buff[retCode-1] == '/' ){
						temp_buff[retCode-1] = 0;
						break;
					}
					temp_buff[retCode-1] = 0;
				}
			}
            retCode = tfRamfsChDir(temp_buff);
            strcpy( userFsDataPtr->userWorkingDirectory, tm_ramfs_fhandle(ramfsPtr->ptrCurDirHandle)->szName);
//            if (retCode != -1)
//            {
//                (void)tfRamfsGetCwd(userFsDataPtr->userWorkingDirectory,
//                                    TM_FS_WORKINGDIR_SIZE);
//            }
            tm_call_unlock(&ramfsPtr->locEntry);
        }
    }
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

int tfFSRemoveDir(const void TM_FAR * userDataPtr,
                  const char TM_FAR * pathNamePtr )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);

    return -1;
}

int tfFSMakeDir (const void TM_FAR *    userDataPtr,
                 const char TM_FAR *    pathNamePtr,
                 char TM_FAR *          bufferPtr,
                 int                    bufferSize )
{
    TM_UNREF_IN_ARG(userDataPtr);
    TM_UNREF_IN_ARG(pathNamePtr);
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(bufferSize);

    return -1;
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

    retCode = -1;
    if (bufferPtr != (char TM_FAR *) 0)
    {
        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
        if (ramfsPtr != (ttRamfsDrivePtr)0)
        {
            tm_call_lock_wait(&ramfsPtr->locEntry);
/* Point to current user directory */
            tfFSUserDir(userFsDataPtr);
			strcpy( bufferPtr, userFsDataPtr->userWorkingDirectory );
			bufferSize = tm_strlen(bufferPtr);
			retCode = bufferSize;
        }
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
        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
        if (ramfsPtr != (ttRamfsDrivePtr)0)
        {
            if ( (pathNamePtr == (char TM_FAR *)0) || (*pathNamePtr == 0) )
            {
                pathNamePtr = "*.*";
            }
            tm_call_lock_wait(&ramfsPtr->locEntry);
            tfFSUserDir(userFsDataPtr);
            dirPtr = tfRamfsOpenDir(userFsDataPtr, pathNamePtr, flag);
            tm_call_unlock(&ramfsPtr->locEntry);
        }
    }
    return dirPtr;
#undef userFsDataPtr
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

    ttVoidPtr           dirPtr;
    dirPtr = tfRamfsFileFind( lpszFileName);
	
    pFindInfo->ulFlags = tm_ramfs_fhandle(dirPtr)->ulFlags;
    pFindInfo->lSize = tm_ramfs_fhandle(dirPtr)->lFileSize;
    tm_strcpy(pFindInfo->szName, tm_ramfs_fhandle(dirPtr)->szName);

    return dirPtr;
}

static ttVoidPtr tfRamfsOpenDir (ttUserFsDataPtr     userFsDataPtr,
                                 const char TM_FAR * pathNamePtr,
                                 int                 flag)
{
    ttRamfsDrivePtr     ramfsPtr;
    ttVoidPtr           dirPtr;
    
    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
/* Directory handle */
    dirPtr = tfRamfsFindFirstFile( pathNamePtr, &userFsDataPtr->userDirEntry);
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
    	dirPtr = ramfsPtr;
    }
    return dirPtr;
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
	        tfRamfsGetEntryInfo(ramfsPtr, pFindInfo, blEntry);
        }
    }
    return nResult;
}

static void tfRamfsGetEntryInfo( ttRamfsDrivePtr    ramfsPtr,
                                 ttRamfsFindInfoPtr pFindInfo,
                                 ttRamfsBlock       blEntry )
{
//    ttRamfsEntryPtr pEntry;

	pFindInfo->ulFlags = tm_ramfs_fhandle(ramfsPtr->ptrCurDirHandle)->ulFlags;
    pFindInfo->lSize = tm_ramfs_fhandle(ramfsPtr->ptrCurDirHandle)->lFileSize;
    tm_strcpy(pFindInfo->szName, tm_ramfs_fhandle(ramfsPtr->ptrCurDirHandle)->szName);
    return;
}

static ttVoidPtr tfRamfsFileFind (const char TM_FAR * pathNamePtr)
{
    ttVoidPtr       filePtr;
    ttVoidPtrPtr    pHandleList;
    int             counter,size,i;
	char			wname[TM_RAMFS_MAXFNAME];
    ttRamfsDrivePtr ramfsPtr;
    ttRamfsFileHandlePtr    pHandle;
    ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);

    pHandleList = tlFileHandles;

	filePtr = (ttVoidPtr)0;
	size = (int)tm_strlen(pathNamePtr);
	memset( wname, 0, sizeof( wname ));
	if(size > 1) {
		if(*(pathNamePtr + (size - 1)) == '/') {
			--size;
		}
	}
	memcpy( wname, pathNamePtr, size );
    for(counter = 0; counter < LIST_REGIST_MAX; counter++)
    {
		pHandle = tm_ramfs_fhandle(*pHandleList);
		if( pHandle != 0 ){
			for( i = 0; i < PROG_MAX; i++ ){
//				if( tm_strstr( pathNamePtr, dirs_files[PROG_INDEX+i].szName ) != NULL && counter > PROG_INDEX ){
				if( tm_strstr( pathNamePtr, pHandle->szName ) != NULL && (counter >= PROG_INDEX && counter < (PROG_INDEX+PROG_MAX)) ){
					get_script_file_name( pHandle->szName, pathNamePtr );
					break;
				}
			}
			if( tm_strstr( pHandle->szName, "/REMOTEDL/" ) != NULL ){
				get_remotedl_filename( pHandle->szName );
			}

			if( size > 1 ){
				if( wname[0] != '/' ){
					size = tm_strlen(tm_ramfs_fhandle(ramfsPtr->ptrCurDirHandle)->szName);
					memmove(&wname[size],&wname[0],tm_strlen(wname));
					memcpy( wname, tm_ramfs_fhandle(ramfsPtr->ptrCurDirHandle)->szName, size );
					if( size > 1 ){
						wname[size++] = '/';
						memcpy( &wname[size], pathNamePtr, tm_strlen(pathNamePtr) );
					}
				}
			}

	 	    if (!tm_stricmp(pHandle->szName, wname) )		// 大文字小文字を区別しない
	      	{
	            tlLastError = TM_ENOERROR;
	            filePtr = *pHandleList;
	            break;
	        }
	        else
	        {
	            pHandleList++;
	        }
        }
    }
	if( filePtr == 0 ){
		SetSnd_RecvStatus( FCMD_REQ_IDLE );
	}
	return filePtr;
}

int tfFSGetNextDirEntry ( void TM_FAR * userDataPtr,
                          void TM_FAR * dirDataPtr,
                          char TM_FAR * bufferPtr,
                          int           bufferSize )
{
//    TM_UNREF_IN_ARG(userDataPtr);
//    TM_UNREF_IN_ARG(dirDataPtr);
//    TM_UNREF_IN_ARG(bufferPtr);
//    return -1;
    ttRamfsDrivePtr ramfsPtr;
    int             retCode;
    struct _ft_respdir_    *pPtr;
	char			*AUTH_TBL[] = {
		LIST_AUTH_DIR,
		LIST_AUTH_RW_DATA,
		LIST_AUTH_R_DATA
	};
	char			auth;

#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)
#define userFsDirPtr  ((ttDirEntryPtr)dirDataPtr)
/* This command is only supported if a user has logged in. */
    retCode = -1;
    if (   (userDataPtr != TM_VOID_NULL_PTR) 
        && ((unsigned)bufferSize >= TM_RAMFS_MAXFNAME + TM_FILE_POS + 1))
    {
        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
        if (ramfsPtr != (ttRamfsDrivePtr)0)
        {
            retCode = 0; /* End of directory */
            tm_call_lock_wait(&ramfsPtr->locEntry);
            tfFSUserDir(userFsDataPtr);
            if (userFsDataPtr->userDirPtr != (ttVoidPtr)0)
            {
/*
 * Pass over '.', or '..' for TM_DIR_LONG.
*/
				pPtr = ftp_get_dir();
				
				if( pPtr->entrys ){
		            tm_memset(bufferPtr, 0, (unsigned)bufferSize);
		            retCode = (int)
		                    tm_strlen(pPtr->ent[pPtr->count].name);
		            if (retCode > 0)
		            {
						
						if( pPtr->ent[pPtr->count].attr & 0x10 ){
							auth = 0;
						}else{
							switch(( pPtr->ent[pPtr->count].attr & 0x03 )){
								case	1:
									auth = 2;
									break;
								case	0:
								case	2:
								case	3:
									auth = 1;
									break;
							}
						}
						sprintf( &bufferPtr[0], LIST_FORMAT, AUTH_TBL[auth],
															 pPtr->ent[pPtr->count].size,
															 MONTH_TBL[CLK_REC.mont], 
															 CLK_REC.date,
															 CLK_REC.hour,
															 CLK_REC.seco,
															 pPtr->ent[pPtr->count].name );
															 
						retCode = tm_strlen(bufferPtr);
		                pPtr->entrys--;
		                pPtr->count++;
						
		            }
	            }
            }
            tm_call_unlock(&ramfsPtr->locEntry);
        }
	
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
//    ttRamfsDrivePtr ramfsPtr;
//#define userFsDataPtr ((ttUserFsDataPtr)userDataPtr)
//
//    TM_UNREF_IN_ARG(dirDataPtr);
//    if (userDataPtr != TM_VOID_NULL_PTR)
//    {
//        ramfsPtr = (ttRamfsDrivePtr)tm_global(tvRamfsPtr);
//        if (ramfsPtr != (ttRamfsDrivePtr)0)
//        {
//            tm_call_lock_wait(&ramfsPtr->locEntry);
//            if (userFsDataPtr->userDirPtr != (ttVoidPtr)0)
//            {
//                (void)tfRamfsFindClose(userFsDataPtr->userDirHandle);
//                userFsDataPtr->userDirPtr = (ttVoidPtr)0;
//            }
//            tm_call_unlock(&ramfsPtr->locEntry);
//        }
//    }
    return;
//#undef userFsDataPtr
}

static void tfFSUserDir(ttConstUserFsDataPtr userFsDataPtr)
{
    if (userFsDataPtr != (ttConstUserFsDataPtr)0)
    {
        (void)tfRamfsChDir(userFsDataPtr->userWorkingDirectory);
    }
}

int tfFSFileExists(
    const void TM_FAR * userDataPtr,
    const char TM_FAR * pathNamePtr )
{
    int             errorCode;
    ttVoidPtr       hFind;

    TM_UNREF_IN_ARG(userDataPtr);

    if (pathNamePtr == (const void TM_FAR *)0)
    {
        errorCode = TM_EINVAL;
    }
    else if ((hFind = tfRamfsFileFind(pathNamePtr))
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
//   ttRamfsFindInfo findInfo;
    ttVoidPtr       hFind;

    TM_UNREF_IN_ARG(userDataPtr);

    if (pathNamePtr == (const void TM_FAR *)0)
    {
        fileLen = 0;
        tlLastError = TM_EINVAL;
    }
    else if ((hFind = tfRamfsFileFind(pathNamePtr))
             != (ttVoidPtr)0)
    {
        fileLen = tm_ramfs_fhandle(hFind)->lFileSize;
        tlLastError = TM_ENOERROR;
        (void)tfRamfsFindClose(hFind);
    }
    else
    {
        fileLen = 0;
        tlLastError = TM_ENOENT;
    }

    return (ttUser32Bit)fileLen;
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
            position =   tm_ramfs_fhandle(fileDataPtr)->lReadPosition
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
            tm_ramfs_fhandle(fileDataPtr)->lReadPosition = position;
        }
    }
    return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ログインユーザー名、パスワード取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	GetFTPLoginPass( char type, char *user, char *pass )
{
	
	if( user !=  NULL ){
		if (type == REMOTE_LONGIN) {
			remotedl_ftp_user_get(user, 20);
		}
		else {
			memcpy( user, tlFsNamePasswd[type].userName, sizeof(tlFsNamePasswd[0].userName));
		}
	}
	if( pass != NULL ){
		if (type == REMOTE_LONGIN) {
			remotedl_ftp_passwd_get(pass, 20);
		}
		else {
			memcpy( pass, tlFsNamePasswd[type].userPassword, sizeof(tlFsNamePasswd[0].userPassword));
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			接続先IPアドレス取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Get_IP_adrr( char *ip, char task )
{
// MH810100(S) S.Nishimoto 2020/04/09 静的解析(20200407:644)対応
//	char	wip[4];
	char	wip[4] = {0, 0, 0, 0};
// MH810100(E) S.Nishimoto 2020/04/09 静的解析(20200407:644)対応
	
	if( task == OPETCBNO && remotedl_script_typeget() != PARAMTER_SCRIPT ){
// MH810100(S) S.Takahashi 2019/12/20
		if(remotedl_script_typeget() == LCD_NO_SCRIPT) {
			wip[0] = (char)prm_get( COM_PRM, S_PKT, 1, 3, 1 );
			wip[1] = (char)prm_get( COM_PRM, S_PKT, 2, 3, 1 );
			wip[2] = (char)prm_get( COM_PRM, S_PKT, 3, 3, 1 );
			wip[3] = (char)prm_get( COM_PRM, S_PKT, 4, 3, 1 );
		}
		else {
// MH810100(E) S.Takahashi 2019/12/20
		memcpy(&wip, &ip_adrr[0], sizeof(ip_adrr[0]));
// MH810100(S) S.Takahashi 2019/12/20
		}
// MH810100(E) S.Takahashi 2019/12/20
	}else{
// MH810100(S) S.Takahashi 2019/12/20
//// TODO:宛先IPアドレスは標準用の共通パラメータを取得すること
//		wip[0] = 192;
//		wip[1] = 168;
//		wip[2] = 100;
//		wip[3] = 50;
//		if( !wip[0] && !wip[1] && !wip[2] && !wip[3]){
//			// IPがALL０なら再度ROM固定のIPをコピーし直す
//			memcpy(&wip, &ip_adrr[1], sizeof(ip_adrr[1]));
//		}
// MH810100(E) S.Takahashi 2019/12/20
	}
	sprintf( ip, "%d.%d.%d.%d", wip[0],wip[1],wip[2],wip[3]);
	
}

//[]----------------------------------------------------------------------[]
///	@brief			ダミー関数
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	dummy_Size_Get(void)
{
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			車室パラメータデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Lock_Info_Size_Get(void)
{
	ulong	size;
	
	size = (LOCK_MAX*6*3);

	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			車室情報データサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Car_Sts_Size_Get(void)
{
	ulong	size;
	
	size = (sizeof(FLAPDT));

	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			プログラム情報データサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Prog_Info_Size_Get(void)
{
	ulong	size;
	
	size = (60*4);

	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			共通パラメータデータサイズ取得（運用面）
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	MP_Data_Size_Get(void)
{
	ulong	size,section,work;
	t_AppServ_ParamInfoTbl	*param_tbl;

	param_tbl =	(t_AppServ_ParamInfoTbl*)FLT_GetParamTable();
	
	work = 0;
	for (section = 0; section < param_tbl->csection_num; section++) {
		work += (int)param_tbl->csection[section].item_num;
	}
	size = (work*sizeof(long));

	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			共通パラメータデータサイズ取得（待機面）
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	SP_Data_Size_Get(void)
{
	ulong	size,section,work;
	t_AppServ_ParamInfoTbl	*param_tbl;

	size = 0;
	if( ftp_check_stby_param() ){
		memset( SysMnt_Work, 0, sizeof(SysMnt_Work));
		param_tbl = (t_AppServ_ParamInfoTbl*)SysMnt_Work;
		FLT_ReadStbyParamTbl(param_tbl);
		
		work = 0;
		for (section = 0; section < param_tbl->csection_num; section++) {
			work += param_tbl->csection[section].item_num;
		}
		size = (work*sizeof(long));
	}

	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			音声データサイズ取得（運用面）
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// long	MV_Data_Size_Get(void)
// {
// 	ulong	size = 0;
// 	uchar	buff[sizeof(PRG_HDR)];

// 	FLT_read_wave_sum_version( (BootInfo.sw.wave == OPERATE_AREA1)?0:1, buff );
// //	Flash2Read( buff, (ulong)GET_VAPP(0), sizeof(PRG_HDR) );
	
// 	size = ((PRG_HDR*)buff)->proglen + sizeof(PRG_HDR);
	
// 	return (long)size;
// }
//[]----------------------------------------------------------------------[]
///	@brief			音声データサイズ取得（待機面）
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// long	SV_Data_Size_Get(void)
// {
// 	ulong	size = 0;
// 	uchar	buff[sizeof(PRG_HDR)];

// 	FLT_read_wave_sum_version( (BootInfo.sw.wave == OPERATE_AREA1)?1:0, buff );
// //	Flash2Read( buff, (ulong)GET_VAPP(1), sizeof(PRG_HDR) );
	
// 	size = ((PRG_HDR*)buff)->proglen + sizeof(PRG_HDR);

// 	return (long)size;
// }
//[]----------------------------------------------------------------------[]
///	@brief			メインプログラムデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Main_Prog_Size_Get(void)
{
	ulong	size;
	uchar	buff[sizeof(PRG_HDR)];

//	Flash2Read( buff, (ulong)GET_MAIN_H, sizeof(PRG_HDR) );
	FLT_read_program_version( buff );
	size = ((PRG_HDR*)buff)->proglen + sizeof(PRG_HDR);
	
	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			精算データサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Sale_Data_Size_Get(void)
{
	ulong size;
	
	size = Ope_Log_TotalCountGet( eLOG_PAYMENT );
	size *= FLT_SALELOG_RECORD_SIZE;
	
	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			集計データサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Total_Data_Size_Get(void)
{
	ulong size;
	
	size = Ope_Log_TotalCountGet( eLOG_TTOTAL );
	size *= FLT_TOTALLOG_RECORD_SIZE;
	
	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			エラーデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Err_Data_Size_Get(void)
{
	ulong	size;
	
	size = DataSizeCheck( (char*)SysMnt_Work, 0);

	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			アラームデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Arm_Data_Size_Get(void)
{
	ulong	size;
	
	size = DataSizeCheck( (char*)SysMnt_Work, 1);

	return (long)size;
}
//[]----------------------------------------------------------------------[]
///	@brief			モニタデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Moni_Data_Size_Get(void)
{
	ulong	size;
	
	size = DataSizeCheck( (char*)SysMnt_Work, 2);

	return (long)size;
	
}
//[]----------------------------------------------------------------------[]
///	@brief			操作モニタデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Ope_Data_Size_Get(void)
{
	ulong	size;
	
	size = DataSizeCheck( (char*)SysMnt_Work, 3);

	return (long)size;
}

//[]----------------------------------------------------------------------[]
///	@brief			ロゴデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Logo_Size_Get(void)
{
	
	return Get_Etc_File_Existence(LD_LOGO);
}
//[]----------------------------------------------------------------------[]
///	@brief			ヘッダーデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Header_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_HDR);
}
//[]----------------------------------------------------------------------[]
///	@brief			フッターデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Footer_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_FTR);
}
//[]----------------------------------------------------------------------[]
///	@brief			受付券フッターデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Uketuke_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_ACCEPTFTR);
}
//[]----------------------------------------------------------------------[]
///	@brief			署名欄データサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Shomei_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_SYOM);
}
//[]----------------------------------------------------------------------[]
///	@brief			加盟店名データサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Kamei_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_KAME);
}
//[]----------------------------------------------------------------------[]
///	@brief			Edy ATコマンドデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Edy_AT_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_EDYAT);
}
//[]----------------------------------------------------------------------[]
///	@brief			Tカードフッターデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	T_Footer_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_TCARD_FTR);
}
//[]----------------------------------------------------------------------[]
///	@brief			預り証フッターデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-17<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	AZU_Footer_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_AZU_FTR);
}
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
//[]----------------------------------------------------------------------[]
///	@brief			過払いフッターデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2016-10-31<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
long	CreKBR_Footer_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_CREKBR_FTR);
}
long	EpayKBR_Footer_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_EPAYKBR_FTR);
}
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
//[]----------------------------------------------------------------------[]
///	@brief			後日支払額フッターデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2016-11-08<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
long	Future_Footer_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_FUTURE_FTR);
}
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター

long	Chk_Result_Size_Get(void)
{
	return FLT_CHKRESULT_SIZE;
}

static long RemoteDl_Size_Get(void)
{
	return SysMnt_Work_Size;
}

//[]----------------------------------------------------------------------[]
///	@brief			ファイル名チェック関数
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			taskID	:タスクID
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	fileOpenChk( const char *pname  )
{
	ushort	i;
	uchar	ret = 0;
	for( i=0; (i<tm_strlen(pname) && pname[i] != 0); i++ ){
		if( pname[i] == 0x2e ){							// .(ピリオド)発見
			ret = 1;									// 次のデータから比較対象とする
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ファイル名チェック関数
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			taskID	:タスクID
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
char	GetCurrentDir( char *pname, char *currentdir  )
{
	char	ret = 0;

	ttUserFsDataPtr p = (ttUserFsDataPtr)pname;
	strcpy( temp_buff, p->userWorkingDirectory );
	if( p->userWorkingDirectory != 0 ){
		strcpy( currentdir, p->userWorkingDirectory );
		ret = 1;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ファイルサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			taskID	:タスクID
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	GetFileSize( short fileNum )
{
    ttVoidPtrPtr    pHandleList = tlFileHandles;
    ttRamfsFileHandlePtr    fHandle;
	long	size = 0;
    short	i;
    
    for(i = 0; i < LIST_REGIST_MAX; i++, pHandleList++)
    {
		fHandle = tm_ramfs_fhandle(*pHandleList);
		if( fHandle->number == fileNum ){
			size = fHandle->lFileSize;
			break;
		}
	}
	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			デフォルトのファイル名を再生成する
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	SetDefaultFileName( ttVoidPtr pHandle )
{
	short	i;
    ttRamfsFileHandlePtr    fHandle = tm_ramfs_fhandle(pHandle);
    
    for(i = 0; i < LIST_REGIST_MAX; i++)
    {
		if( dirs_files[i].number != 0 && dirs_files[i].number == fHandle->number ){
			_STRCPY(fHandle->szName, dirs_files[i].szName);
			break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			デフォルトのファイル名を再生成する
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	FileSystemUpdate( void )
{
    ttVoidPtrPtr    pHandleList = tlFileHandles;
    ttRamfsFileHandlePtr    pHandle;
    short	i;

    for(i = 0; i < LIST_REGIST_MAX; i++, pHandleList++)
    {
	    pHandle = tm_ramfs_fhandle(*pHandleList);
	    if( pHandle == NULL ){
			break;
		}else{
			if( pHandle->number ){
				pHandle->lFileSize = dirs_files[i].func();
				/* FTPInit時にハンドルのファイル名を更新する。 */
				_STRCPY(pHandle->szName, dirs_files[i].szName);
				WACDOG;
			}
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
///	@brief			スクリプトファイル名をファイルシステムに登録
//[]----------------------------------------------------------------------[]
///	@param[in]		*p			: ファイルシステムポインタ
///	@param[in]		remote_path	: リモートファイル名
///	@return			void
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-05-04<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	set_script_file_name( unsigned char* p )
{
    ttVoidPtrPtr    pHandleList = tlFileHandles;
    short	i;
    
    for(i = 0; i < TM_RAMFS_MAXFILES; i++, pHandleList++)
    {
		if( tm_ramfs_fhandle(*pHandleList)->number == LD_AU_SCR ){
			_STRCPY(tm_ramfs_fhandle(*pHandleList)->szName, p );
			break;
		}
	}
}

#endif /* !TM_USE_RAM_FS */

// MH810105(S) MH364301 QRコード決済対応
//[]----------------------------------------------------------------------[]
///	@brief			障害連絡票フッターデータサイズ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/04/08
///					Update	:	
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
long EMG_Footer_Size_Get(void)
{
	return Get_Etc_File_Existence(LD_EMG);
}
// MH810105(E) MH364301 QRコード決済対応
