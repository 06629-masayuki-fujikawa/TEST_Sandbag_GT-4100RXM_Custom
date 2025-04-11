#ifndef	_FTPCTRL_H_
#define	_FTPCTRL_H_
//[]----------------------------------------------------------------------[]
///	@mainpage		
///	FTP control<br><br>
///	<b>Copyright(C) 2007 AMANO Corp.</b>
///	CREATE			2007/08/23 MATSUSHITA<br>
///	UPDATE			
///	@file			ftpctrl.h
///	@date			2007/08/23
///	@version		MH644410
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
#include "trsocket.h"

/*--------------------------------------------------------------------------*/
/*		FTP NEW Protcol Stack												*/
/*--------------------------------------------------------------------------*/
// define 宣言部
				 
#define		MAX_CONNECTIONS	3					// 同時に受け入れることができるFTP接続の数
#define		MAX_BACKLOG		1					// FTPサーバソケットで許可する接続保留中キューの最大数
#define		IDLE_TIMEOUT	(600*3)				// 接続がアイドル状態になってから、FTPサーバが接続をあきらめてクローズするまでの時間（30分）

#define		PC_LONGIN	0
#define		REMOTE_LONGIN	1
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス（→LCD パラメータ)
#define		LCD_LOGIN		2
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス（→LCD パラメータ)
// 関数プロトタイプ宣言部
extern	void 	FTPmsgSend( unsigned	short msg, void *tmp );
extern	void	FTP_Event_Analize( unsigned	char comd, unsigned	char *data );
extern	void 	GetFTPLoginPass(char ,char*, char* );
extern	char	ftp_file_write(char *file);
extern	unsigned	char	ftp_read_write(char *file);
extern	void 	MntFtpExecChk(void);
extern	void	SetTaskID( unsigned	char );
extern	unsigned	char	GetTaskID( void );
extern	char 	GetCurrentDir( char*, char* );
extern	void 	FTP_Execute( void );
extern	long	GetFileSize( short );
extern	void	InitialFileSystem( void );
extern	void	FileSystemUpdate( void );
extern	void	set_script_file_name( unsigned	char * );
extern	char	GetFTP_Handle_Status( void *pHandle, char	type );
extern	unsigned	long	GetWriteTopAddress( void );
extern	unsigned	long	GetWriteHeaderAddress( void );
extern	unsigned	char	GetFailSafeTimerNo( void );
extern	int NLSTcmd_Callback(ttUserFtpHandle ftpSessionPtr, char TM_FAR *bufferPtr, int bufferSize);
extern	void get_remotedl_filename(char *szName);


/*--------------------------------------------------------------------------*/
/*		FTP Definitions														*/
/*--------------------------------------------------------------------------*/
// MH810100(S) Y.Yamauchi 2019/11/20 車番チケットレス(遠隔ダウンロード)
//#define	PARAMETER_CSV_MAX	65536	// 32KB⇒64KB
#define	PARAMETER_CSV_MAX	(128*1024)	// 64KB⇒128KB
// MH810100(E) Y.Yamauchi 2019/11/20 車番チケットレス(遠隔ダウンロード)
// MH810100(S) K.Onodera 2020/04/09 静的解析
#define	PARAM_CSV_OLD_FORM_MAX	65536	// 32KB⇒64KB
// MH810100(E) K.Onodera 2020/04/09 静的解析
#define	AUTO_SCRIPT_MAX		8192

enum {
	_FTP_SERVER = 0,
	_FTP_CLIENT,
};

enum {
	FCMD_REQ_IDLE = 0,
	FCMD_REQ_SEND,
	FCMD_REQ_RECV,
};

enum {
	FRESP_OK,
	FRESP_NOUSER,		// specified user is undefined
	FRESP_REJECT,		// already login/logout
	FRESP_NOFILE,		// specified file/dir is not exist
	FRESP_WP,			// write protect
	
	FRESP_ABOR = 10,	// recieve ABOR
	FRESP_LOST,			// disconnect
	FRESP_WE,			// write error
	
	_FRESP_MAX
};

typedef	struct {
	unsigned	char	name[128];
	unsigned	char	attr;
	unsigned	long	size;
} _dir_entry;

enum {
	__attr_dir	= 0x10,
	__attr_w	= 0x02,
	__attr_r	= 0x01
};

struct _ft_resp_ {
	unsigned	char	code;
	unsigned	char	kind;
	unsigned	short	result;
};

struct _ft_respdir_ {
	unsigned	char	code;
	unsigned	char	kind;
	unsigned	short	count;
	unsigned	short	entrys;
	_dir_entry	ent[20];
};

struct _ft_login_ {
	unsigned	char	code;		// FCMD_REQ_LOGIN/FCMD_LOGIN
	unsigned	char	kind;
	unsigned	char	user;
};

struct _ft_logout_ {
	unsigned	char	code;		// FCMD_REQ_LOGOUT/FCMD_LOGOUT
	unsigned	char	kind;
};

struct _ft_sr_ {
	unsigned	char	code;		// FCMD_REQ_SEND/FCMD_REQ_RECV
	unsigned	char	kind;
	unsigned	char	local[64];
	unsigned	char	remote[256];
// GG120600(S) // Phase9 サーバフォルダ構成変更対応
	unsigned	char	mkd[256];
// GG120600(E) // Phase9 サーバフォルダ構成変更対応
};

struct _ft_rw_ {
	unsigned	char	code;		// FCMD_READ/FCMD_WRITE/FCMD_DIR
	unsigned	char	kind;
	unsigned	char	local[64];
};

struct _ft_common_ {
	unsigned	char	code;		// FCMD_CAN/FCMD_QUIT/FCMD_REQ_STS
	unsigned	char	kind;
};

struct _ft_start_ {
	unsigned	char	code;		// FCMD_REQ_START
	unsigned	char	kind;
	unsigned	char	mode;
};

typedef	union {
	struct _ft_common_	com;
	struct _ft_resp_	resp;
	struct _ft_login_	login;
	struct _ft_logout_	logout;
	struct _ft_rw_		rw;
} FTP_RCVCMD;

typedef	union {
	struct _ft_common_	com;
	struct _ft_resp_	resp;
	struct _ft_respdir_	dir;
	struct _ft_login_	login;
	struct _ft_logout_	logout;
	struct _ft_sr_		sr;
	struct _ft_start_	start;
} FTP_SNDCMD;

int	ftpRecvCtrl(unsigned	char *data, unsigned	short dlen);
int	ftpRecvData(unsigned	char *data, unsigned	short dlen);
void ftp_make_dir( char* );
struct _ft_respdir_* ftp_get_dir( void );

#endif
