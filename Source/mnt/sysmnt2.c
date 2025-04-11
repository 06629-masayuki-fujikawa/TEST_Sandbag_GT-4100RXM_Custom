//[]----------------------------------------------------------------------[]
///	@brief		
///	System Maintenance (2)<br>
///	FTP<br><br>
///	<b>Copyright(C) 2012 AMANO Corp.</b>
///	CREATE			2012.05.09 Namioka<br>
///	UPDATE			
///	@file			sysmnt2.c�i�v���g�R���X�^�b�N�ύX�ɂ��S�ʉ����j
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"pri_def.h"
#include	"mnt_def.h"
#include	"sysmnt_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"lcd_def.h"
#include	"strdef.h"
#include	"subtype.h"
#include	"flashdef.h"
#include	"script.h"
#include	"lkmain.h"
#include	"flp_def.h"

#include	"ftpctrl.h"
#include	"mdl_def.h"
#include	"remote_dl.h"
#include	"trsocket.h"
#include	"FlashSerial.h"
#include	"updateope.h"
#include	"aes_sub.h"
#include	"zlib_sub.h"
#include	"ntnet_def.h"
#include 	"ksg_def.h"
#include	"raudef.h"
#include	"trsntp.h"

extern	unsigned short	GetMessage( void );
extern	int stricmp(const char *d, const char *s);

#define	_STRCPY(a,b)	strcpy((char*)a, (char*)b)
#define	_STRLEN(a)		(ushort)strlen((char*)a)
#define	_STRICMP(a,b)	stricmp((const char*)a, (const char*)b)

static	BOOL	SysMnt2ModeChange;
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//#define	_sysmnt2_start_FTP()		SysMnt2ModeChange = FALSE
static	BOOL	SysMnt2ModeLcdDisconnect;
#define	_sysmnt2_start_FTP()	SysMnt2ModeChange = SysMnt2ModeLcdDisconnect = FALSE
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define	_sysmnt2_start_sys_switch()		_sysmnt2_start_FTP()

/* system switch prototype */
static	ulong	sw_update_proc(void);	// FTP�̖ʐ؂�ւ����ɂ��g�p
/* system switch static data */
static 	SW_DATA	sw_update;	// �ؑ֗\��
static	void	_displclr_6Line(void);			// ��ʕ\�������p�̃o�C�p�X����
#define	FTP_DISP_MASK()	(( DOWNLOADING() || Param_Upload_type_get() == 1 ))
static void	ftp_set_write_info(void);
static void	ftp_get_write_info(void);
static int	ftp_read_param_All(char *buff,int nSize);
// MH810100(S) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
static ulong ftp_output_dat_param( char *buff,int nSize );
// MH810100(E) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
static void ftp_remote_auto_update_end_check(t_prog_chg_info* dl);
static int	compare_filename(const void *str1, const void *str2);
// GG120600(S) // Phase9 CRC�`�F�b�N
static void ftp_set_write_crc(uchar by1,uchar by2);
static ushort ftp_get_write_crc(void);

#define HIBYTE(s)					(uchar)((ushort)(s) >> 8)
#define LOBYTE(s)					(uchar)((ushort)(s))
#define	MAKEWORD(low, high)			(ushort)(((ushort)(high) << 8) + (ushort)(low))	
// GG120600(E) // Phase9 CRC�`�F�b�N

static	void	FAIL_SAFE_TIMER_START(uchar a, uchar b);

static		void	ftp_connection(void);
static		int		ftp_send_login(int);
static		int		ftp_get_script(int);
static		int		ftp_script_analize(int);
static		int		ftp_send_cancel(int);
static		int		ftp_send_quit(int);
static		int		ftp_free_session(int);
static		int		ftp_close_req(int);
static		int		ftp_send_req_cmd( struct _ft_sr_ *cmd );
static		int		ftp_file_recv_Data(int end);
static		int		ftp_send_login_remote( int res );
static		void	ftp_connect_remote(void);
static		int		ftp_login_wait( int res  );
static		int		ftp_cwd_wait(int);
static		int		ftp_nlst_wait(int);
static		int		ftp_mkd_wait( int res );
static		int		ftp_type_wait( int res );
static		int		ftp_get_file(int);
static		int		ftp_send_nlst_cmd(struct _ft_sr_ *cmd);
static		int		ftp_send_cwd_cmd(struct _ft_sr_ *cmd);
static		int		ftp_send_mkd_cmd(struct _ft_sr_ *cmd);
static		int		ftp_send_type_cmd(void);
static		int		ftp_send_param(int);
static		int		ftp_send_param_wait( int res );
static		int		ftp_test_up_wait(int res);
static		int		ftp_test_down(int);
static		int		ftp_test_down_wait(int);
static		int		ftp_test_del(int);
static		int		ftp_test_del_wait(int);
static		int		ftp_send_quit_remote(int);
static		int		ftp_quit_remote_wait(int);
static		void	FTP_Execute_Cliant( void );
static		void	FTP_Execute_Server( void );
// MH810100(S) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^
static 		void	ftp_new_session_for_LCD( void );
static		void	ftp_connect_for_LCD( void );
static		int		ftp_send_login_for_LCD( int res );
static		int		ftp_login_wait_for_LCD( int res  );
static		int		ftp_send_type_cmd_for_LCD( void );
static		int		ftp_type_wait_for_LCD( int res );
static		int		ftp_param_wait_for_LCD( int res );
// MH810100(E) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
#if 0
static		void	ftps_pki_use( void );
#endif
static		int		ftps_add_rootCertificate( int res );
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j

typedef int (*FTP_Res_Analize)( int res );

static	FTP_Res_Analize func_analize[] = {
	{NULL},					// FTP_IDLE
	{NULL},					// FTP_STOP
	{NULL},					// FTP_START
	{NULL},					// FTP_NEW_SESSION
	{ftp_send_login},		// FTP_CONNECT
	{ftp_get_script},		// FTP_LOGIN_REQ
	{ftp_script_analize},	// FTP_SCRIPT_REQ
	{ftp_script_analize},	// FTP_SD_RD_REQ
	{ftp_send_cancel},		// FTP_ABOR_REQ
	{ftp_send_quit},		// FTP_QUITE_REQ
	{ftp_close_req},		// FTP_CLOSE_REQ
	{ftp_free_session},		// FTP_FREE_REQ
	{ftp_send_login_remote},// FTP_CONNECT_REMOTE
	{ftp_login_wait},		// FTP_LOGIN_WAIT
// MH810100(S) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^�j
	{ftp_send_login_for_LCD},// FTP_CONNECT_LCD
	{ftp_login_wait_for_LCD},// FTP_LOGIN_LCD
	{ftp_type_wait_for_LCD},// FTP_TYPE_REQ_LCD
	{ftp_param_wait_for_LCD},// FTP_PARAM_WAIT_LCD
// MH810100(E) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^�j
	{ftp_type_wait},		// FTP_TYPE_REQ
	{ftp_cwd_wait},			// FTP_CWD_REQ
	{ftp_nlst_wait},		// FTP_NLST_REQ
	{ftp_get_file},			// FTP_RETR_REQ
	{ftp_mkd_wait},			// FTP_MKD_REQ
	{ftp_send_param},		// FTP_PARAM_UP_REQ
	{ftp_send_param_wait},	// FTP_PARAM_UP_WAIT
	{ftp_test_up_wait},		// FTP_TEST_UP_WAIT
	{ftp_test_down},		// FTP_TEST_DOWN
	{ftp_test_down_wait},	// FTP_TEST_DOWN_WAIT
	{ftp_test_del},			// FTP_TEST_DEL
	{ftp_test_del_wait},	// FTP_TEST_DEL_WAIT
	{ftp_send_quit_remote},	// FTP_QUITE_REMOTE_REQ
	{ftp_quit_remote_wait},	// FTP_QUITE_REMOTE_WAIT
	{NULL},					// FTP_QUITE_WAIT
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
	{ftps_add_rootCertificate},	// FTPS_PKI_START
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j

};

#define		FILESYS	(TM_FS_CWD_FLAG|TM_FS_RETR_FLAG	|TM_FS_RETR_FLAG|TM_FS_STOR_FLAG|TM_FS_PWD_FLAG|TM_FS_LIST_FLAG|TM_FS_NLST_FLAG)
			// �J�����g�f�B���N�g���̕ύX���T�|�[�g����

#define		BLOCKING_STATUS	TM_BLOCKING_OFF		// �u���b�L���O���[�h�I��
#define		FTP_TIMEOUT	(50*1)
#define		INTERVAL_TIMER_START( No )	( Lagtim((GetTaskID()), (No&0x00FF), 1) )

static	const	char	BinType[2][3] = {
	"RXM",
	"WAV"
};

static	struct _ft_sr_ cmd;
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
static eFTP_REQ_TYPE g_FtpRequestFlg;
extern	t_AppServ_BinCsvSave	AppServ_BinCsvSave;
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)

//[]----------------------------------------------------------------------[]
///	@brief			sysment2_change_mode(change mode sw)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	void	sysment2_change_mode(void)
{
	if (!SysMnt2ModeChange) {
		SysMnt2ModeChange = TRUE;
	}
	else {
		SysMnt2ModeChange = FALSE;
	}
}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//[]----------------------------------------------------------------------[]
///	@brief			sysment2_lcd_disconnect(lcd disconnect sw)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			Y.Yamauchi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/01/06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	void	sysment2_lcd_disconnect(void)
{
	if (!SysMnt2ModeLcdDisconnect) {
		SysMnt2ModeLcdDisconnect = TRUE;
	}
	else {
		SysMnt2ModeLcdDisconnect = FALSE;
	}
}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)

/*--------------------------------------------------------------------------*/
/*		FTP Global Data														*/
/*--------------------------------------------------------------------------*/
static
struct {
	int		dir;
	int		file;
	uchar	sw;
	ushort	tra_start;
	struct {
		uchar		*buff;
		ulong		size;
		ulong		ofs;
		ulong		len;
	} recv;

	struct {
		ulong		top_addr;
		ulong		base_addr;
		ulong		header_addr;
		PRG_HDR		header;
		ushort		sum;
		int			error;
	} write;

	struct {
		int			step;
		uchar		*buff;
		long		len;
		long		index;
		ulong		top_addr;
		ulong		base_addr;
		ulong		header_addr;
		uchar		finish;
	} send;
} ftpdata;

static	t_AppServ_ParamInfoTbl	param_tbl_wk;		// for standby parameter

#define	_REMOTE_MAX_PATH	256
#define _MAX_DIR_ENT_CNT	36
static
struct {
	int		start;
	int		login;
	ulong	etc_update;
	int		result;
	int		au_sts;
	ushort	script;
	uchar	filename[36];
	uchar	mode;
	uchar	taskID;
	uchar	fail_safe;
	ttUserFtpHandle ftpHandle;
	char	loginbuff[2][24];
	char	ip[16];
	uchar	filename_size;
	char	cur_dir_ent;
	char	dir_ent_cnt;
	char	dir_ent_name[_MAX_DIR_ENT_CNT][_REMOTE_MAX_PATH];

	struct {
		FTP_SNDCMD	buff;
	} send;
	int		watch_timer;
	uchar		send_cmd;
// GG120600(S) // Phase9 CRC�`�F�b�N
	union{
		uchar	crcTemp[2];
		ushort  uscrcTemp;
	}CRC16;
// GG120600(E) // Phase9 CRC�`�F�b�N
} ftpctrl;

enum {
	FTP_IDLE = 0,
	FTP_STOP,
	FTP_START,
	FTP_NEW_SESSION,
	FTP_CONNECT,
	FTP_LOGIN_REQ,
	FTP_SCRIPT_REQ,
	FTP_SD_RD_REQ,
	FTP_ABOR_REQ,
	FTP_QUITE_REQ,	
	FTP_CLOSE_REQ,	
	FTP_FREE_REQ,	
	FTP_CONNECT_REMOTE,
	FTP_LOGIN_WAIT,
// MH810100(S) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^�j
	FTP_CONNECT_LCD,		// 14
	FTP_LOGIN_LCD,			// 15
	FTP_TYPE_REQ_LCD,		// 16
	FTP_PARAM_WAIT_LCD,		// 17
// MH810100(E) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^�j
	FTP_TYPE_REQ,
	FTP_CWD_REQ,
	FTP_NLST_REQ,
	FTP_RETR_REQ,
	FTP_MKD_REQ,
	FTP_PARAM_UP_REQ,
	FTP_PARAM_UP_WAIT,
	FTP_TEST_UP_WAIT,
	FTP_TEST_DOWN,
	FTP_TEST_DOWN_WAIT,
	FTP_TEST_DEL,
	FTP_TEST_DEL_WAIT,
	FTP_QUITE_REMOTE_REQ,
	FTP_QUITE_REMOTE_WAIT,
	FTP_QUITE_WAIT,	
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
	FTPS_PKI_START,
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
};

#define FTP_ERROR  FTP_STOP

enum {
	AU_DWL_SCR = 0,
	AU_CONFIRM,
	AU_EXEC,

	AU_NORMAL_END = 10,
	AU_ABORT_END,
	AU_NO_UPDATE,
	AU_CANCEL,
	AU_DISCONNECT,
	AU_LOGIN_ERR,
	AU_LOGOUT_ERR,
};

enum{
	STS_IDLE = 0,
	STS_SENDING,
	STS_RECVING,
	STS_WAITING,

};

#define	FTP_CHK_UP		0
#define	FTP_CHK_DOWN	1

#define	_MAX_PATH			sizeof(ftpctrl.send.buff.sr.local)
#define	IsETCExits(p)	((p[0] ^ p[1]) == 0xff)

#define _SET_WATCH_TIMER()	(ftpctrl.watch_timer = 5)
#define _SET_TIMEOUT()		(ftpctrl.watch_timer = 1)

/* work buffer (each 2048) */
extern	T_SYSMNT_PKT	SysMnt_SndBuf;
extern	T_SYSMNT_PKT	SysMnt_RcvBuf;
#define	tmpwork1		SysMnt_SndBuf.data.data
#define	tmpwork2		SysMnt_RcvBuf.data.data
extern	uchar	SysMnt_Work[SysMnt_Work_Size];		// 64KB
extern	char	SysMnt_Work2[4][36];
extern	char	SysMnt_Work3[6][20];
extern	char	SysMnt_UP_DW_Text[T_FOOTER_GYO_MAX][36];

/* sys */
extern	const uchar	SYSMNT_PARAM_FILENAME[];
extern	const uchar	SYSMNT_LOCKPARAM_FILENAME[];
static	const uchar	fn_prog[] = "prog.txt";
/* log */
static	const uchar	fn_salelog[]  = "SaleLog.bin";
static	const uchar	fn_totallog[] = "TotalLog.bin";
/* etc */
extern	const uchar	SYSMNT_LOGO_FILENAME[];
extern	const uchar	SYSMNT_HEADER_FILENAME[];
extern	const uchar	SYSMNT_FOOTER_FILENAME[];
extern	const uchar	SYSMNT_ACCEPTFTR_FILENAME[];
extern	const uchar	SYSMNT_SYOMEI_FILENAME[];
extern	const uchar	SYSMNT_KAMEI_FILENAME[];
extern	const uchar	SYSMNT_EDYAT_FILENAME[];
extern	const uchar	SYSMNT_USERDEF_FILENAME[];
extern	const uchar	SYSMNT_ACCEPTFTR_FILENAME[];

extern	const uchar	SYSMNT_ERR_FILENAME[];
extern	const uchar	SYSMNT_ARM_FILENAME[];
extern	const uchar	SYSMNT_NONI_FILENAME[];
extern	const uchar	SYSMNT_OPE_MONI_FILENAME[];

extern	ulong		DataSizeCheck( char *data, uchar kind );
extern	const uchar	SYSMNT_CAR_INFO_FILENAME[];
extern	const uchar	SYSMNT_TCARDFTR_FILENAME[];
extern	const uchar	SYSMNT_AZUFTR_FILENAME[];
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
extern	const uchar	SYSMNT_CREKBRFTR_FILENAME[];
extern	const uchar	SYSMNT_EPAYKBRFTR_FILENAME[];
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
extern	const uchar	SYSMNT_FUTUREFTR_FILENAME[];
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern	const uchar	SYSMNT_EMGFOOTER_FILENAME[];
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
extern	const uchar	SYSMNT_REMOTE_DL_RESULT_FILENAME[];
extern	const uchar	SYSMNT_REMOTE_DL_PARAM[];
extern	const uchar	SYSMNT_REMOTE_DL_CONNECT[];
// MH810100(S) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
extern	const uchar	SYSMNT_UP_PARAM_LCD[];
// MH810100(E) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)


enum {
	_DIR_ROOT,
	_DIR_SYS,
	_DIR_SYS_ACT,
	_DIR_SYS_STBY,
	_DIR_SYS_STBY_MAIN,
	_DIR_SYS_STBY_VOICE,
	_DIR_LOG,
	_DIR_ETC,
	_DIR_TMP,
	_DIR_REMOTEDL,
	_DIR_MAX
};
static	const	char	*const path[] = {
	{"/"},
	{"/SYS"},
	{"/SYS/ACT"},
	{"/SYS/STBY"},
	{"/SYS/STBY/MAIN"},
	{"/SYS/STBY/VOICE"},
	{"/LOG"},
	{"/ETC"},
	{"/TMP"},
	{"/REMOTEDL"},
	NULL
};

static	const	uchar	*const dir_root[] = {
	{"SYS"},	{"LOG"},	{"ETC"},	NULL
};
static	const	uchar	*const dir_sys[] = {
	{"ACT"},	{"STBY"},	NULL
};
static	const	uchar	*const dir_stby[] = {
// MH810100(S) K.Onodera 2020/03/05 �Ԕԃ`�P�b�g���X(�����f�B���N�g���폜)
//	{"MAIN"},	{"VOICE"},	NULL
	{"MAIN"},	NULL
// MH810100(E) K.Onodera 2020/03/05 �Ԕԃ`�P�b�g���X(�����f�B���N�g���폜)
};

static	const	uchar	*const *dirs[] = {
	dir_root,
	dir_sys, NULL, dir_stby, NULL, NULL, NULL,
	NULL,
	NULL
};

#define	_fsize(m)	(ushort)sizeof(((_PRNSECT_IMG*)0)->m)
#define	_offs(s,m)	(ulong)(&(((s*)0)->m))

static	const	struct {
	ulong		ofs;
	ushort		size;
	const uchar	*name;
	ushort		fileno;
} etc_files[] = {
	{_offs(_PRNSECT_IMG, logo),		_fsize(dlogo),		SYSMNT_LOGO_FILENAME,		LD_LOGO},
	{_offs(_PRNSECT_IMG, header),	_fsize(dheader),	SYSMNT_HEADER_FILENAME,		LD_HDR},
	{_offs(_PRNSECT_IMG, footer),	_fsize(dfooter),	SYSMNT_FOOTER_FILENAME,		LD_FTR},
	{_offs(_PRNSECT_IMG, syomei),	_fsize(dsyomei),	SYSMNT_SYOMEI_FILENAME,		LD_SYOM},
	{_offs(_PRNSECT_IMG, kamei),	_fsize(dkamei),		SYSMNT_KAMEI_FILENAME,		LD_KAME},
	{_offs(_PRNSECT_IMG, atcmd),	_fsize(datcmd),		SYSMNT_EDYAT_FILENAME,		LD_EDYAT},
	{_offs(_PRNSECT_IMG, acceptfooter),	_fsize(dacceptfooter),	SYSMNT_ACCEPTFTR_FILENAME,	LD_ACCEPTFTR},
	{_offs(_PRNSECT_IMG, tcardftr),	_fsize(dtcardftr),	SYSMNT_TCARDFTR_FILENAME,	LD_TCARD_FTR},
	{_offs(_PRNSECT_IMG, azuftr),	_fsize(dazuftr),	SYSMNT_AZUFTR_FILENAME,		LD_AZU_FTR},
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
	{_offs(_PRNSECT_IMG, crekbrftr),	_fsize(dcrekbrftr),	SYSMNT_CREKBRFTR_FILENAME,		LD_CREKBR_FTR},
	{_offs(_PRNSECT_IMG, epaykbrftr),	_fsize(depaykbrftr),	SYSMNT_EPAYKBRFTR_FILENAME,	LD_EPAYKBR_FTR},
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
	{_offs(_PRNSECT_IMG, futureftr),	_fsize(dfutureftr),	SYSMNT_FUTUREFTR_FILENAME,	LD_FUTURE_FTR},
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	{_offs(_PRNSECT_IMG, emgfooter),	_fsize(demgfooter),	SYSMNT_EMGFOOTER_FILENAME,	LD_EMG},
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	{0, 0, NULL, 0}
};
#define	__accept_bit	(1L<<6)		// 6�Ԗڂ̃G���g���[
#define	__tcardftr_bit	(1L<<7)		// 7�Ԗڂ̃G���g���[
#define	__azuftr_bit	(1L<<8)		// 8�Ԗڂ̃G���g���[
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
#define	__crekbrftr_bit	(1L<<9)		// 9�Ԗڂ̃G���g���[
#define	__epaykbrftr_bit	(1L<<10)	// 10�Ԗڂ̃G���g���[
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
#define	__futureftr_bit	(1L<<11)	// 11�Ԗڂ̃G���g���[
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define	__emgftr_bit	(1L<<12)	// 12�Ԗڂ̃G���g���[
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�


/*--------------------------------------------------------------------------*/
/*		FTP																	*/
/*--------------------------------------------------------------------------*/
//[]----------------------------------------------------------------------[]
///	@brief			ftpRecvData(receive file data)
//[]----------------------------------------------------------------------[]
///	@param[in]		data	:	
///	@param[in]		dlen	:	
///	@return			1/0		: OK(1)/  (0)
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
int	ftpRecvData(uchar *data, ushort dlen)
{
	ulong	ofs;
	ushort	len;
	int		ret;
	
	_SET_WATCH_TIMER();
	while( 1 ){
		if (ftpdata.recv.buff == NULL){		// receiving?
			ret = 0;						// no, sikp data
			break;
		}
		if( GetFTP_Handle_Status( ftpctrl.ftpHandle, 0 ) ){
			ret = 0;						// no, sikp data
			break;
		}
		if (dlen == 0) {
			if( !ftpdata.tra_start ){
				ret = 0;						// no, sikp data
				break;
			}
			ftpdata.recv.buff = NULL;
			ret = ftp_file_recv_Data( 1 );
			break;
		}

		ftpdata.tra_start = 1;
		while(( ftpdata.recv.len + dlen ) > ftpdata.recv.size ){
			ret = ftp_file_recv_Data( 0 );
			if( ret ){
				return ret;						// err 
			}
			taskchg(IDLETSKNO);
		}
		ofs = ftpdata.recv.ofs + ftpdata.recv.len;			// write offset
		if (ofs > ftpdata.recv.size) {
			ofs -= ftpdata.recv.size;						// round offset
			memcpy(&ftpdata.recv.buff[ofs], data, (size_t)dlen);
			ftpdata.recv.len += dlen;
		}
		else {
			len = (ushort)(ftpdata.recv.size - ofs);		// remain size
			if (len <= dlen) {
			// copy half until buffer end
				memcpy(&ftpdata.recv.buff[ofs], data, (size_t)len);
				ftpdata.recv.len += len;
				data += len;
				dlen -= len;
				ofs = 0;
			}
			if (dlen) {
			// copy all or copy remain half
				memcpy(&ftpdata.recv.buff[ofs], data, (size_t)dlen);
				ftpdata.recv.len += dlen;
			}
		}
		ret = ftp_file_recv_Data( 0 );
		break;
	}
	return ret;
}


static	int	ftp_send_req_cmd( struct _ft_sr_ *pcmd )
{
	
	int		errorCode;
	
	if( pcmd->code == FCMD_REQ_SEND ){		// ���M�v��
		errorCode = tfFtpStor(ftpctrl.ftpHandle,
								NULL,
								(char*)pcmd->local,
								(char*)pcmd->remote);
		switch( errorCode ){
			case	TM_ENOERROR:			// ����
			case	TM_EWOULDBLOCK:			// ����FTP�Z�b�V�����̓m���u���b�L���O�ŁA�I�y���[�V�������������Ă��Ȃ�
			case	TM_FTP_XFERSTART:		// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
			case	TM_FTP_FILEOKAY:		// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
				errorCode = OK;
				ftpctrl.send_cmd = STS_SENDING;
				break;
			default:
				errorCode = NG;
				break;
		}
	}else{
		errorCode = tfFtpRetr(ftpctrl.ftpHandle,
								NULL,
								(char*)pcmd->remote,
								(char*)pcmd->local);
		
		switch( errorCode ){
			case	TM_ENOERROR:			// ����
			case	TM_EWOULDBLOCK:			// ����FTP�Z�b�V�����̓m���u���b�L���O�ŁA�I�y���[�V�������������Ă��Ȃ�
			case	TM_FTP_XFERSTART:		// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
			case	TM_FTP_FILEOKAY:		// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
				errorCode = OK;
				ftpctrl.send_cmd = STS_RECVING;
				break;
			case	TM_EACCES:				// �O�̃R�}���h�̃I�y���[�V�������������Ă��Ȃ�
				_SET_WATCH_TIMER();
				ftpctrl.result = errorCode;
				return errorCode;
				break;
			default:
				errorCode = NG;
				break;
		}
	}
	
	if( errorCode != OK ){
		ftpctrl.start = FTP_QUITE_REQ;
		if( errorCode == TM_ENOTLOGIN || ftpctrl.login == -1 ){
			ftpctrl.result = AU_LOGIN_ERR;
			remotedl_monitor_info_set(16);
			remotedl_result_set(LOGIN_ERR);
		}else if( errorCode == TM_ENOBUFS ){
			ftpctrl.result = SCRIPT_FILE_NONE;
			if (remotedl_script_typeget() != REMOTE_NO_SCRIPT) {
				remotedl_result_set(((!ftpctrl.script||remotedl_status_get()!=R_DL_EXEC)?SCRIPT_FILE_NONE:PROG_DL_NONE));
			}
			else {
				ftpctrl.start = FTP_QUITE_REMOTE_REQ;
				if (remotedl_connect_type_get() != CTRL_PROG_DL) {
					remotedl_monitor_info_set(17);
					remotedl_result_set(PROG_DL_NONE);
				}
				else {
					remotedl_monitor_info_set(18);
					remotedl_result_set(FOMA_COMM_ERR);
				}
			}
		}else{
			ftpctrl.result = AU_DISCONNECT;
			remotedl_monitor_info_set(19);
			remotedl_result_set(FOMA_COMM_ERR);
		}
	}else{
		if (remotedl_script_typeget() != REMOTE_NO_SCRIPT) {
			ftpctrl.start = FTP_SD_RD_REQ;
		}
		_SET_WATCH_TIMER();
	}
	
	return errorCode;
}


//[]----------------------------------------------------------------------[]
///	@brief			ftp_new_session(create ftp session request )
//[]----------------------------------------------------------------------[]
///	@param[in]		mode	: server or client
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
void	ftp_new_session( void )
{
	ftpctrl.ftpHandle = tfFtpNewSession(0,
									TM_BLOCKING_OFF,
									ftpctrl.loginbuff[0],
									ftpctrl.loginbuff[1]);
	if( !ftpctrl.ftpHandle ){
		displclr(6);
	}else{
		ftpctrl.start = FTP_NEW_SESSION;
		Lagtim(OPETCBNO, 7, FTP_TIMEOUT);	// ��ϰ�Ď�����(1s)
		_SET_WATCH_TIMER();
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_connection(ftp connecrion request)
//[]----------------------------------------------------------------------[]
///	@param[in]		mode	: server or client
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	void	ftp_connection( void )
{
	
	int		errorCode;
	
	errorCode = tfFtpConnect( ftpctrl.ftpHandle, ftpctrl.ip );
	switch( errorCode ){
		case TM_ENOERROR:						// ����
		case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
			ftpctrl.start = FTP_CONNECT;
			_SET_WATCH_TIMER();
			break;
		default:
			displclr(6);
			ftpctrl.start = FTP_STOP;
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_connect_remote(ftp connecrion request)
//[]----------------------------------------------------------------------[]
///	@param[in]		mode	: server or client
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static	void	ftp_connect_remote( void )
{
	
	int		errorCode;
	char	ip[20];								// IP�A�h���X�ϊ��p
	ulong	ipaddr;
	struct sockaddr_storage ss;

	// FOMA������H
	if (prm_get(COM_PRM, S_CEN, 51, 1, 3) == 0) {
		// FTP�T�[�o��ParkingWeb�̈��悪�قȂ邩�H
		sprintf(ip, "%03d.%03d.%03d.%03d",
				prm_get(COM_PRM, S_CEN, 62, 3, 4),
				prm_get(COM_PRM, S_CEN, 62, 3, 1),
				prm_get(COM_PRM, S_CEN, 63, 3, 4),
				prm_get(COM_PRM, S_CEN, 63, 3, 1));
		ipaddr = inet_addr(ip);
		if (ipaddr != remotedl_ftp_ipaddr_get()) {
			tfAddStaticRoute(gInterfaceHandle_PPP_rau,
								remotedl_ftp_ipaddr_get(),
								inet_addr("255.0.0.0"),
								inet_addr("0.0.0.0"),
								14);
		}
	}

	memset(&ss, 0, sizeof(ss));
	ss.ss_len = sizeof(struct sockaddr_in);
	ss.ss_family = AF_INET;
	ss.ss_port = remotedl_ftp_port_get();
	ss.addr.ipv4.sin_addr.s_addr = remotedl_ftp_ipaddr_get();
	errorCode = tfNgFtpConnect(ftpctrl.ftpHandle, &ss);
	switch( errorCode ){
		case TM_ENOERROR:						// ����
		case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
			ftpctrl.start = FTP_CONNECT_REMOTE;
			_SET_WATCH_TIMER();
			break;
		default:
			displclr(6);
			ftpctrl.start = FTP_STOP;
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_get_script(recv script file request)
//[]----------------------------------------------------------------------[]
///	@param[in]		mode	: server or client
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_get_script( int res  )
{
	
	int		errorCode = res;
	uchar	status = remotedl_status_get();
	
	switch( errorCode ){
		case	TM_ENOERROR:
			// ���O�C�������I
			ftpctrl.login = 1;
			if( DOWNLOADING() ){
			}else{
				if( remotedl_script_typeget() == MANUAL_SCRIPT ){
					dispclr();
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[35]);			/* "���I�[�g�A�b�v�f�[�g���@�@�@�@" */
					grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[36]);			/* "�@�T�[�o�[�ɖ₢���킹���ł��@" */
					grachr(5, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	/* "�@�@���΂炭���҂��������@�@�@" */
					Fun_Dsp(FUNMSG[0]);														/* [0]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
				}
				else{
					if(remotedl_script_typeget() == PARAMTER_SCRIPT){
						grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[12] );	// "���p�����[�^�A�b�v���[�h���@�@"
					}
				}
			}
			au_get_cmd(&cmd);										// �X�N���v�g�t�@�C���̃t�@�C��������
			errorCode = tfFtpRetr(ftpctrl.ftpHandle,
									NULL,
									(char*)cmd.remote,
									(char*)cmd.local);
							
			
			switch( errorCode ){
				case TM_ENOERROR:						// ����
				case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
				case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
				case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
					ftpctrl.start = FTP_SCRIPT_REQ;
					ftpctrl.send_cmd = STS_RECVING;
					_SET_WATCH_TIMER();
					break;
				default:
					ftpctrl.start = FTP_QUITE_WAIT;
					remotedl_result_set( SCRIPT_FILE_NONE );		// ���ʏ��t�@�C���A�b�v���[�h���s
					break;
			}
			break;
		case TM_EWOULDBLOCK:						// �I�y���[�V�������������Ă��Ȃ�
			_SET_WATCH_TIMER();
			break;
		default:
			ftpctrl.start = FTP_QUITE_WAIT;
			ftpctrl.result = AU_LOGIN_ERR;
			remotedl_result_set( LOGIN_ERR );		// ���O�C�����s
			break;
			
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_script_analize(script analize)
//[]----------------------------------------------------------------------[]
///	@param[in]		mode	: server or client
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_script_analize( int res )
{
	
	int		errorCode = res;
													//
	if( !ftpctrl.result || ftpctrl.result == TM_EACCES ){
		switch( errorCode ){
			case TM_ENOERROR:							// ����
				if( ftpctrl.script && ftpctrl.au_sts == AU_DWL_SCR ){
					if( DOWNLOADING() ){
						ftpctrl.au_sts = AU_EXEC;
					}else{
						grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[38]);		/* "�@ �A�b�v�f�[�g�����s���܂� �@" */
						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);		/* "�@�@�@ ��낵���ł����H �@�@�@" */
						Fun_Dsp(FUNMSG[19]);												/* "�@�@�@�@�@�@ �͂� �������@�@�@" */
						ftpctrl.au_sts = AU_CONFIRM;
						break;
					}
				}
				if( GetSnd_RecvStatus() != FCMD_REQ_IDLE ){
					if( ftpctrl.result == TM_EACCES ){
						ftpctrl.result = ftp_send_req_cmd( &cmd );
					}
					_SET_WATCH_TIMER();
					break;
				}
				if(( ftpctrl.au_sts == AU_EXEC || DOWNLOADING() ) ){
					switch( au_get_cmd(&cmd) ){
						case	0:							// �v���������Ȃ�
						default:
							ftpctrl.result = AU_NORMAL_END;
							break;
						case	1:							// ����M�v������
							ftp_send_req_cmd( &cmd );
							break;
						case	-1:							// �X�N���v�g�ُ�
							ftpctrl.result = SCRIPT_FILE_ERR;
							remotedl_result_set(ftpctrl.result);
							break;
					}
				}
				break;
			case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
			case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
			case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
				_SET_WATCH_TIMER();
				break;

			default:
				if( errorCode == TM_ETIMEDOUT ){
					ftpctrl.result = AU_DISCONNECT;
					remotedl_result_set(FOMA_COMM_ERR);
				}else{
					ftpctrl.result = SCRIPT_FILE_NONE;
					remotedl_result_set(((!ftpctrl.script||remotedl_status_get()!=R_DL_EXEC)?SCRIPT_FILE_NONE:PROG_DL_NONE));
				}
				break;
		}
	}

	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_close_req(all ftp session close req )
//[]----------------------------------------------------------------------[]
///	@param[in]		mode	: server or client
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_close_req( int res )
{
	
	int		errorCode = res;
	
	errorCode = tfFtpClose( ftpctrl.ftpHandle );
	ftp_free_session(res);
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_quit(send quit request)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_send_quit( int res )
{
	int	errorCode = res;
	
	if( ftpctrl.start != FTP_QUITE_REQ ){
		errorCode = tfFtpQuit(ftpctrl.ftpHandle );
	}
	switch( errorCode ){
		case	TM_ENOERROR:
		case	TM_EWOULDBLOCK:
			if( ftpctrl.start != FTP_QUITE_REQ ){
				ftpctrl.start = FTP_QUITE_REQ;
			}else{
				if( errorCode == TM_ENOERROR ){
					ftp_free_session(res);
				}
			}
			_SET_WATCH_TIMER();
			break;

		case	TM_EACCES:
		case	TM_ENOTCONN:
		case	TM_ENOTLOGIN:
		case	TM_EINVAL:
		case	TM_FTP_SYNTAXCMD:
		default:
			ftpctrl.start = FTP_CLOSE_REQ;
			break;
	}
	
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_free(send session free request)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_free_session( int res )
{
	int	errorCode = res;
	
	if( ftpctrl.start != FTP_FREE_REQ ){
		errorCode = tfFtpFreeSession(ftpctrl.ftpHandle );
		ftpctrl.start = FTP_FREE_REQ;
	}
	switch( errorCode ){
		case	TM_ENOERROR:
			if( errorCode == TM_ENOERROR ){
				ftpctrl.start = FTP_IDLE;
			}
		case	TM_EWOULDBLOCK:
			_SET_WATCH_TIMER();
			break;

		case	TM_EACCES:
		case	TM_ENOTCONN:
		case	TM_ENOTLOGIN:
		case	TM_EINVAL:
		case	TM_FTP_SYNTAXCMD:
		default:
			ftpctrl.start = FTP_IDLE;
			break;
	}
	
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_login(send login request)
//[]----------------------------------------------------------------------[]
///	@param[in]		user	: user number
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_send_login( int res )
{
	int	errorCode = res;
	
	if(( ftpctrl.result & 0x8000 )){
		return errorCode;
	}
	if( ftpctrl.result != AU_LOGIN_ERR ){
		switch( errorCode ){
			case	TM_ENOERROR:
				errorCode = tfFtpLogin(ftpctrl.ftpHandle,
										ftpctrl.loginbuff[0],
										ftpctrl.loginbuff[1],
										NULL );
				
				switch( errorCode ){
					case TM_ENOERROR:						// ����
						ftpctrl.login = 1;
						// no break
					case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
						ftpctrl.start = FTP_LOGIN_REQ;
						_SET_WATCH_TIMER();
						break;
					default:
						ftpctrl.result = AU_LOGIN_ERR | 0x8000;
						break;
				}
				break;
			case TM_EWOULDBLOCK:						// �I�y���[�V�������������Ă��Ȃ�
				_SET_WATCH_TIMER();
				break;
			default:
				ftpctrl.result = AU_LOGIN_ERR | 0x8000;
				break;
		}
	}
	if( ftpctrl.result == (AU_LOGIN_ERR| 0x8000) ){
		if (errorCode == TM_ESHUTDOWN) {
			remotedl_result_set(CONN_TIMEOUT_ERR);	// �ڑ��^�C���A�E�g
		}
		else {
			remotedl_result_set( LOGIN_ERR );// ���O�C�����s
		}
		ftpctrl.start = FTP_CLOSE_REQ;
	}else if(ftpctrl.result == TM_ENOERROR && ftpctrl.login == 1){
		// �����Ȃ̂Ń��O�C�������𓊂���
		rmon_regist(RMON_FTP_LOGIN_OK);
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_login_remote(���O�C�����M�j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static	int	ftp_send_login_remote( int res )
{
	int	errorCode = res;
	
	if(( ftpctrl.result & 0x8000 )){
		return errorCode;
	}
	if( ftpctrl.result != AU_LOGIN_ERR ){
		switch( errorCode ){
			case	TM_ENOERROR:
				errorCode = tfFtpLogin(ftpctrl.ftpHandle,
										ftpctrl.loginbuff[0],
										ftpctrl.loginbuff[1],
										NULL );
				
				switch( errorCode ){
					case TM_ENOERROR:						// ����
						ftpctrl.login = 1;
						// no break
					case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
						ftpctrl.start = FTP_LOGIN_WAIT;
						_SET_WATCH_TIMER();
						break;
					default:
						ftpctrl.result = AU_LOGIN_ERR | 0x8000;
						break;
				}
				break;
			case TM_EWOULDBLOCK:						// �I�y���[�V�������������Ă��Ȃ�
				_SET_WATCH_TIMER();
				break;
			default:
				ftpctrl.result = AU_LOGIN_ERR | 0x8000;
				break;
		}
	}
	if( ftpctrl.result == (AU_LOGIN_ERR| 0x8000) ){
		if (errorCode == TM_ESHUTDOWN) {
			remotedl_monitor_info_set(3);
			remotedl_result_set(CONN_TIMEOUT_ERR);	// �ڑ��^�C���A�E�g
		}
		else {
			remotedl_monitor_info_set(4);
			remotedl_result_set( LOGIN_ERR );// ���O�C�����s
		}
		ftpctrl.start = FTP_CLOSE_REQ;
	}else if(ftpctrl.result == TM_ENOERROR && ftpctrl.login == 1){
		// �����Ȃ̂Ń��O�C�������𓊂���
		rmon_regist(RMON_FTP_LOGIN_OK);
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_login_wait(���O�C���҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static	int	ftp_login_wait( int res  )
{
	
	int		errorCode = res;
	uchar	status = remotedl_status_get();
	
	switch( errorCode ){
		case	TM_ENOERROR:
			// ���O�C�������I
			if( ftpctrl.login != 1){
				ftpctrl.login = 1;
				// �����Ȃ̂Ń��O�C�������𓊂���
				rmon_regist(RMON_FTP_LOGIN_OK);
			}
			// �ڑ����g���C�񐔃N���A
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			retry_info_clr(RETRY_KIND_CONNECT);
			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
				retry_info_clr(RETRY_KIND_CONNECT_PRG);
				break;
			case CTRL_PARAM_DL:			//�p�����[�^�_�E�����[�h
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
				break;
			case CTRL_PARAM_DIF_DL:		//�����p�����[�^�_�E�����[�h
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
				break;
			case CTRL_PARAM_UPLOAD:
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
				break;
			}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

			// ���u�����e�i���X�v�����Ɋe�R�}���h�𔭍s
			memset(&cmd, 0, sizeof(cmd));
			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
			case CTRL_PARAM_DL:			//�p�����[�^�_�E�����[�h
// MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			case CTRL_PARAM_DIF_DL:		//�����p�����[�^�_�E�����[�h
// MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				// CWD�R�}���h���s
				MakeRemoteFileName(cmd.remote, cmd.local, MAKE_FILENAME_SW);
				ftp_send_cwd_cmd(&cmd);
				break;
			case CTRL_PARAM_UPLOAD:
				// MKD�R�}���h���s
				MakeRemoteFileName(cmd.remote, cmd.local, MAKE_FILENAME_PARAM_MKD);
// GG120600(S) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
				memset(cmd.mkd, 0, sizeof(cmd.mkd));
// GG120600(E) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
				ftp_send_mkd_cmd(&cmd);							// �f�B���N�g���쐬
				break;
			case CTRL_CONNECT_CHK:
				// TYPE�R�}���h���s
				ftp_send_type_cmd();
				break;
			}
			break;
		case TM_EWOULDBLOCK:						// �I�y���[�V�������������Ă��Ȃ�
			_SET_WATCH_TIMER();
			break;
		default:
			ftpctrl.start = FTP_QUITE_WAIT;
			ftpctrl.result = AU_LOGIN_ERR;
			remotedl_monitor_info_set(5);
			remotedl_result_set( LOGIN_ERR );		// ���O�C�����s
			break;
			
	}
	return errorCode;
}


//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_cancel(send cancel)
//[]----------------------------------------------------------------------[]
///	@param[in]		result	: result code
///	@return			void
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]-----------------------------FTP_ABOR_REQ-------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_send_cancel( int res )
{
	int	errorCode = res;
	
	if( GetFTPMode() == _FTP_CLIENT ){
		if( ftpctrl.start != FTP_ABOR_REQ ){
			errorCode = tfFtpAbor(ftpctrl.ftpHandle);
			ftpctrl.start = FTP_ABOR_REQ;
		}
		
		switch( errorCode ){
			case	TM_ENOERROR:			// ����
				if( ftpctrl.start != FTP_ABOR_REQ ){
					ftpctrl.start = FTP_ABOR_REQ;
				}else{
					ftpctrl.start = FTP_QUITE_REQ;
				}
				ftpdata.write.error &= 0x7F;
				if( !ftpdata.write.error ){
					ftpctrl.result = AU_ABORT_END;
				}
				_SET_WATCH_TIMER();
				break;
			case	TM_EWOULDBLOCK:			// ����FTP�Z�b�V�����̓m���u���b�L���O�ŁA�I�y���[�V�������������Ă��Ȃ�
				_SET_WATCH_TIMER();
				break;
			default:
				break;
		}
	}else{
		tfFtpdUserStop();
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_getfilename(separate dir path & file name)
//[]----------------------------------------------------------------------[]
///	@param[in]		path	: 
///	@return			p		: *file name
///							: NULL
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	uchar	*ftp_getfilename(uchar *path)
{
	int		i;
	uchar	*p;
	i = 0;
	for (p = path; *p; p++) {
		i++;
		if (i >= _MAX_PATH)
			return NULL;
	}
	do {
		p--;
		i--;
		if (i < 0)
			return NULL;
	 } while(*p != '/');
	*p = '\0';
	return ++p;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_check_dir(check directory)
//[]----------------------------------------------------------------------[]
///	@param[in]		dir		: 
///	@return			i/-1	: i = dir. no.
///							: -1
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int		ftp_check_dir(char *dir)
{
	int		i;
	for (i = 0; path[i] != NULL; i++) {
		if (_STRICMP(dir, path[i]) == 0)
			return i;
	}
	return -1;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_set_subdir(set sub directory info.)
//[]----------------------------------------------------------------------[]
///	@param[in]		dir		: 
///	@param[in]		entry	: _dir_entry
///	@return			cnt		: 
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int		ftp_set_subdir(int dir, _dir_entry *entry)
{
	int		cnt = 0;
	const uchar	*const *d = dirs[dir];
	if (d != NULL) {
		while(*d != NULL) {
			_STRCPY(entry->name, *d);
			entry->attr = (__attr_dir|__attr_r|__attr_w);
			entry->size = 0;
			entry++;
			cnt++;
			d++;
		}
	}
	return cnt;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_disp_file_title(display file title of connection data)
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: send=0 recieve=1
///	@return			1/0		: OK(1)/  (0)
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_disp_file_title(uchar kind)
{
	switch(ftpdata.file) {
	case	LD_PROG:
	case	LD_PARA:
	case	LD_PARA_S:
	case	LD_SLOG:
	case	LD_TLOG:
	case	LD_MAIN_S:
	case	LD_VOICE_A:
	case	LD_VOICE_S:
	case	LD_LOGO:
	case	LD_HDR:
	case	LD_FTR:
	case	LD_SYOM:
	case	LD_KAME:
	case	LD_EDYAT:
	case	LD_AU_SCR:
	case	LD_LOCKPARA:
	case	LD_ACCEPTFTR:
	case	LD_ERR:
	case	LD_ARM:
	case	LD_MONI:
	case	LD_OPE_MONI:
	case	LD_CAR_INFO:
	case	LD_TCARD_FTR:
	case	LD_AZU_FTR:
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
	case	LD_CREKBR_FTR:
	case	LD_EPAYKBR_FTR:
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
	case	LD_FUTURE_FTR:
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	case	LD_EMG:
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		if( FTP_DISP_MASK() ){									// ���u�_�E�����[�h��/�p�����[�^�A�b�v���[�h�i�V���[�g�j�͉�ʕ\�������Ȃ�
			break;
		}
		displclr(6);
		grachr(6, 0, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, (kind)? SMSTR3[27] : SMSTR3[28]);	/* ��M�F�@�@�@�@�@�@�@�@�@�@�@�@ */
																/* ���M�F�@�@�@�@�@�@�@�@�@�@�@�@ */
		grachr(6, 6, _STRLEN(ftpctrl.filename), 0, COLOR_BLACK, LCD_BLINK_OFF, ftpctrl.filename);		/* file tiltle */
		break;
	default:
		return 0;
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_edit_proginfo(edit program information)
//[]----------------------------------------------------------------------[]
///	@param[out]		buff	: data buffer (out)
///	@param[in]		title	: line title
///	@param[in]		prg		: program
///	@param[in]		ver		: version
///	@return			data length
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]

static	ushort	ftp_edit_proginfo(char *buff, const char *title, uchar type )
{
	uchar	version[VER_SIZE+1];
	PRG_HDR	*prg;

	switch( type ){
// MH810100(S) S.Fujii 2020/07/06 SYS�t�H���_�ɕs�v�ȃt�@�C�������݂���
//		case	0:
//			FLT_read_wave_sum_version( (BootInfo.sw.wave == OPERATE_AREA1)?0:1, tmpwork1 );
//			break;
// MH810100(E) S.Fujii 2020/07/06 SYS�t�H���_�ɕs�v�ȃt�@�C�������݂���
		case	1:
			FLT_read_program_version( tmpwork1 );
			break;
// MH810100(S) S.Fujii 2020/07/06 SYS�t�H���_�ɕs�v�ȃt�@�C�������݂���
//		case	2:
//			FLT_read_wave_sum_version( (BootInfo.sw.wave == OPERATE_AREA1)?1:0, tmpwork1 );
//			break;
		default:
			return 0;
// MH810100(E) S.Fujii 2020/07/06 SYS�t�H���_�ɕs�v�ȃt�@�C�������݂���

	}
	prg = (PRG_HDR*)tmpwork1;
	if (IsProgExist(prg)) {
		memset(version, 0, sizeof(version));
		memcpy( version, prg->version, sizeof( prg->version ) );
	}else{
		return (ushort)sprintf(buff, "%s\r\n", title);
	}
	return (ushort)sprintf(buff, "%s  %s  %08ld  0x%04hx\r\n", title, version,
										prg->proglen + sizeof(PRG_HDR), prg->sum);

}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_make_progfile(make progfile)
//[]----------------------------------------------------------------------[]
///	@param[out]		buff	: data buffer (out)
///	@return			data length
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ushort	ftp_make_progfile(char *buff)
{
	ushort	len = 0;
	uchar	i;
	
	static const char	*const title[] = {
	{" ACT VOICE"},	{" STBY MAIN"},	{"     VOICE"}
	};	
	static const char header[] = "/*---program informations---*/\r\n\r\n            version_  size____  sum___\r\n";

	_STRCPY(buff, header);
	len += _STRLEN(header);

	for( i=0; i<3; i++ ){
		len += ftp_edit_proginfo(&buff[len], title[i], i);
	}
	return len;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_check_stby_param(check standby parameter is exist)
//[]----------------------------------------------------------------------[]
///	@return			1/0		: exist(1)/  (0)
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
int		ftp_check_stby_param(void)
{
	ulong	addr;
	uchar	sum[2];

	if(BootInfo.sw.parm == OPERATE_AREA1) {
		addr = FLT_PARAM_SECTOR_2;
	} else {
		addr = FLT_PARAM_SECTOR_1;
	}
	Flash2Read( sum, addr, 2);

	return ((sum[0] ^ sum[1]) == 0xff)? 1 : 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_make_dir(make directory)
//[]----------------------------------------------------------------------[]
///	@param[in]		rcv		: FTP_RCVCMD
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
void	ftp_make_dir(char *pdir )
{
	int		dir;
	struct _ft_respdir_	*resp;
	_dir_entry	*entry;
	PRG_HDR	*prg;
	ulong	datasize;
	const uchar	*p;
	int		i;
	uchar	sw;
	
	FAIL_SAFE_TIMER_START( GetTaskID(), GetFailSafeTimerNo() );
	resp = &ftpctrl.send.buff.dir;
	dir = ftp_check_dir(pdir);
	if (dir < 0) {
		resp->entrys = 0;
	}
	else {
		resp->count = 0;
		memset(resp->ent, 0, sizeof(resp->ent));
		entry = resp->ent;
		resp->entrys = ftp_set_subdir(dir, entry);
		entry += resp->entrys;
		switch(dir) {
		case	_DIR_SYS:
// MH810100(S) S.Fujii 2020/07/03 SYS�t�H���_�ɕs�v�ȃt�@�C�������݂���
//			_STRCPY(entry->name, SYSMNT_LOCKPARAM_FILENAME);
//			entry->attr = __attr_r|__attr_w;
//			entry->size = (LOCK_MAX*6*3);	// �S���b�N���ux�U�A�C�e��x���ςR�o�C�g
//			resp->entrys++;
//			entry++;
//
//			_STRCPY(entry->name, SYSMNT_CAR_INFO_FILENAME);
//			entry->attr = __attr_r|__attr_w;
//			entry->size = sizeof(FLAPDT);
//			resp->entrys++;
//			entry++;
// MH810100(E) S.Fujii 2020/07/03 SYS�t�H���_�ɕs�v�ȃt�@�C�������݂���

			_STRCPY(entry->name, fn_prog);
			entry->attr = __attr_r;
			entry->size = 60*4;
			resp->entrys++;
			if(OPECTL.Mnt_mod == 5){
				datasize = GetFileSize(LD_CHKMODE_RES);
				if( GetFileSize(LD_CHKMODE_RES) ){
					_STRCPY(entry->name, Chk_info.fn_chkresult);
					entry->attr = __attr_r|__attr_w;
					entry->size = datasize;
					resp->entrys++;
					entry++;
				}
			}
			break;
		case	_DIR_SYS_STBY:
			if (! ftp_check_stby_param())
				break;
		case	_DIR_SYS_ACT:
		{
			t_AppServ_ParamInfoTbl	*param_tbl;
			ushort	section;

			_STRCPY(entry->name, SYSMNT_PARAM_FILENAME);
			entry->attr = __attr_r;
			if (dir == _DIR_SYS_ACT) {
				param_tbl =	(void*)FLT_GetParamTable();
			}
			else {
				param_tbl = &param_tbl_wk;
				FLT_ReadStbyParamTbl(param_tbl);
				entry->attr |= __attr_w;
			}
			i = 0;
			for (section = 0; section < param_tbl->csection_num; section++) {
				i += (int)param_tbl->csection[section].item_num;
			}
			entry->size = ((ulong)i) * sizeof(long);
			resp->entrys++;
			break;
		}
		case	_DIR_SYS_STBY_MAIN:
			memset( SysMnt_Work, 0, sizeof( SysMnt_Work ));
			FLT_read_program_version( SysMnt_Work );
			prg = (PRG_HDR*)SysMnt_Work;
			if (IsProgExist(prg)) {
				entry->attr = __attr_r | __attr_w;
				_STRCPY((char*)entry->name, prg->version);
				for( i=(VER_SIZE-1); i!=0; i-- ){
					if( entry->name[i] == 0x20 ){	// �󔒂��������
						entry->name[i] = 0;
					}else{							// �󔒈ȊO���o�Ă������_�Ńo�[�W�����m��
						break;
					}
				}
				strcat((char*)entry->name, ".BIN");
				entry->size = prg->proglen + sizeof(PRG_HDR);
				resp->entrys++;
			}
			break;
		case	_DIR_SYS_STBY_VOICE:
			if( BootInfo.sw.wave == OPERATE_AREA1 ){
				sw = 1;
			}else{
				sw = 0;
			}
			memset( SysMnt_Work, 0, sizeof( SysMnt_Work ));
			FLT_read_wave_sum_version(sw, SysMnt_Work);
			prg = (PRG_HDR*)SysMnt_Work;
			if (IsProgExist(prg)) {
				prg = (PRG_HDR*)SysMnt_Work;
				entry->attr = __attr_r;
				if (dir == _DIR_SYS_STBY_VOICE) {
					entry->attr |= __attr_w;
				}
				_STRCPY(entry->name, prg->version);
				for( i=(VER_SIZE-1); i!=0; i-- ){
					if( entry->name[i] == 0x20 ){	// �󔒂��������
						entry->name[i] = 0;
					}else{							// �󔒈ȊO���o�Ă������_�Ńo�[�W�����m��
						break;
					}
				}
				strcat((char*)entry->name, ".BIN");
				entry->size = prg->proglen + sizeof(PRG_HDR);
				resp->entrys++;
			}
			break;

		case	_DIR_LOG:
		{
			datasize = GetFileSize(LD_SLOG);
			if ( datasize ) {
				_STRCPY(entry->name, fn_salelog);
				entry->attr = __attr_r;
				entry->size = datasize;
				resp->entrys++;
				entry++;
			}

			datasize = GetFileSize(LD_TLOG);
			if (datasize) {
				_STRCPY(entry->name, fn_totallog);
					entry->attr = __attr_r;
					entry->size = datasize;
				resp->entrys++;
				entry++;
			}

			datasize = GetFileSize(LD_ERR);
			if( datasize ){
				_STRCPY(entry->name, SYSMNT_ERR_FILENAME);
				entry->attr = __attr_r;
				entry->size = datasize;
				resp->entrys++;
				entry++;
			}

			datasize = GetFileSize(LD_ARM);
			if( GetFileSize(LD_ARM) ){
				_STRCPY(entry->name, SYSMNT_ARM_FILENAME);
				entry->attr = __attr_r;
				entry->size = datasize;
				resp->entrys++;
				entry++;
			}

			datasize = GetFileSize(LD_MONI);
			if( GetFileSize(LD_MONI) ){
				_STRCPY(entry->name, SYSMNT_NONI_FILENAME);
				entry->attr = __attr_r;
				entry->size = datasize;
				resp->entrys++;
				entry++;
			}

			datasize = GetFileSize(LD_OPE_MONI);
			if( GetFileSize(LD_OPE_MONI) ){
				_STRCPY(entry->name, SYSMNT_OPE_MONI_FILENAME);
				entry->attr = __attr_r;
				entry->size = datasize;
				resp->entrys++;
				entry++;
			}
				break;
			}
			case	_DIR_ETC:
				for (i = 0; etc_files[i].size; i++) {
					p = (uchar*)&ETC_cache[etc_files[i].ofs];
					if (IsETCExits(p)) {
						_STRCPY(entry->name, etc_files[i].name);
						entry->attr = __attr_r|__attr_w;
						entry->size = etc_files[i].size;
						resp->entrys++;
						entry++;
					}
				}
				break;
			default:
				break;
			}
	}
	return;
}

struct _ft_respdir_ *	ftp_get_dir( void )
{
	return (struct _ft_respdir_ *)&ftpctrl.send.buff.dir;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_file_read(start file upload)
//[]----------------------------------------------------------------------[]
///	@param[in]		rcv		: FTP_RCVCMD
///	@return			response code
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
uchar	ftp_file_read(char *file)
{
	uchar	*wfile;
	PRG_HDR	*prg;
	uchar	*p;
	int		i;
	ulong	datasize = 0;
	
	wfile = ftp_getfilename((uchar*)file);
	ftpdata.dir = ftp_check_dir(file);
	ftpdata.file = 0;
	memset( &ftpdata.send, 0, sizeof( ftpdata.send ));
	switch(ftpdata.dir) {
	case	_DIR_SYS:
		if (stricmp((char*)wfile, (char*)SYSMNT_LOCKPARAM_FILENAME) == 0) {
			ftpdata.file = LD_LOCKPARA;
			break;
		}
		else if (stricmp((char*)wfile, (char*)SYSMNT_CAR_INFO_FILENAME) == 0) {
			ftpdata.file = LD_CAR_INFO;
			break;
		}
		else if(_STRICMP(wfile, Chk_info.fn_chkresult) == 0){
			if(OPECTL.Mnt_mod != 5){
				return FRESP_NOFILE;
			}
			ftpdata.file = LD_CHKMODE_RES;
			break;
		}
		if (stricmp((char*)wfile, (char*)fn_prog) != 0)
			return FRESP_NOFILE;
		ftpdata.file = LD_PROG;
		break;
	case	_DIR_SYS_STBY:
		if (! ftp_check_stby_param())
			return FRESP_NOFILE;
	case	_DIR_SYS_ACT:
		if (stricmp((char*)wfile, (char*)SYSMNT_PARAM_FILENAME) != 0)
			return FRESP_NOFILE;
		ftpdata.file = (ftpdata.dir == _DIR_SYS_ACT)? LD_PARA : LD_PARA_S;
		break;
	case	_DIR_SYS_STBY_MAIN:
	case	_DIR_SYS_STBY_VOICE:
		if (ftpdata.dir == _DIR_SYS_STBY_MAIN) {
			ftpdata.file = LD_MAIN_S;
			ftpdata.send.header_addr = (ulong)GET_MAIN_H;
			ftpdata.send.top_addr = (ulong)GET_MAPP;
		}
		else {
			ftpdata.sw = ((BootInfo.sw.wave == OPERATE_AREA1)?1:0);
			ftpdata.file = LD_VOICE_S;
			if( BootInfo.sw.wave == OPERATE_AREA1 ){
				ftpdata.sw = 1;
			}else{
				ftpdata.sw = 0;
			}
			ftpdata.send.header_addr = (ulong)GET_VOICE_H(ftpdata.sw);
			ftpdata.send.top_addr = (ulong)GET_VAPP(ftpdata.sw);
		}
		Flash2Read( SysMnt_Work, ftpdata.send.header_addr, sizeof(PRG_HDR) );
		prg = (PRG_HDR*)SysMnt_Work;
		
		if (!IsProgExist(prg)){
			return FRESP_NOFILE;
		}
		break;
	case	_DIR_LOG:
		if (_STRICMP(wfile, fn_salelog) == 0) {
			datasize = GetFileSize(LD_SLOG);
			if( datasize ){
				ftpdata.file = LD_SLOG;
			}
		}
		else if (_STRICMP(wfile, fn_totallog) == 0) {
			datasize = GetFileSize(LD_TLOG);
			if( datasize ){
				ftpdata.file = LD_TLOG;
			}
		}
		else if (_STRICMP(wfile, SYSMNT_ERR_FILENAME) == 0) {
			datasize = GetFileSize(LD_ERR);
			if( datasize ){
				ftpdata.file = LD_ERR;
			}
		}
		else if (_STRICMP(wfile, SYSMNT_ARM_FILENAME) == 0) {
			datasize = GetFileSize(LD_ARM);
			if( datasize ){
				ftpdata.file = LD_ARM;
			}
		}
		else if (_STRICMP(wfile, SYSMNT_NONI_FILENAME) == 0) {
			datasize = GetFileSize(LD_MONI);
			if( datasize ){
				ftpdata.file = LD_MONI;
			}
		}
		else if (_STRICMP(wfile, SYSMNT_OPE_MONI_FILENAME) == 0) {
			datasize = GetFileSize(LD_OPE_MONI);
			if( datasize ){
				ftpdata.file = LD_OPE_MONI;
			}
		}
		else if (_STRICMP(wfile, SYSMNT_REMOTE_DL_RESULT_FILENAME) == 0) {
		}
		if (ftpdata.file == 0)
			return FRESP_NOFILE;
		break;
	case	_DIR_ETC:
		for (i = 0; ; i++) {
			if (etc_files[i].size == 0)
				return FRESP_NOFILE;
			if (_STRICMP(wfile, etc_files[i].name) == 0) {
				break;
			}
		}
		ftpdata.file = etc_files[i].fileno;
		p = (uchar*)&ETC_cache[etc_files[i].ofs];
		if (!IsETCExits(p))
			return FRESP_NOFILE;
		break;
	case	_DIR_TMP:
		if (stricmp((char*)wfile, (char*)SYSMNT_REMOTE_DL_PARAM) == 0){
			ftpdata.file = LD_PARAM_UP;
		}else if( stricmp((char*)wfile, (char*)SYSMNT_REMOTE_DL_CONNECT) == 0){
			ftpdata.file = LD_CONNECT;
		}
// MH810100(S) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
		else if( stricmp((char*)wfile, (char*)SYSMNT_UP_PARAM_LCD) == 0 ){
			ftpdata.file = LD_PARAM_UP_LCD;
		}
// MH810100(E) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
		if (ftpdata.file == 0){
			return FRESP_NOFILE;
		}
		break;
	default:
		return FRESP_NOFILE;
	}
	_STRCPY(ftpctrl.filename, wfile); // ��ʕ\���p
	ftpdata.send.step = 1;
	ftpdata.tra_start = 0;

	memset( SysMnt_Work, 0, sizeof( SysMnt_Work ) );
	ftp_disp_file_title(0);
	return FRESP_OK;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_file_write(start file download)
//[]----------------------------------------------------------------------[]
///	@param[in]		rcv		: FTP_RCVCMD
///	@return			response code
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
char	ftp_file_write(char *file)
{
	uchar	*wfile;
	int		i;

	wfile = ftp_getfilename((uchar*)file);
	ftpdata.dir = ftp_check_dir(file);
	ftpdata.file = 0;
	if ((ftpdata.dir != _DIR_REMOTEDL) || (ftpctrl.cur_dir_ent <= 1)) {
		memset(&ftpdata.write, 0, sizeof(ftpdata.write));
	}
	switch(ftpdata.dir) {
	case	_DIR_SYS:
		if (stricmp((char*)wfile, (char*)SYSMNT_LOCKPARAM_FILENAME) == 0){
			ftpdata.file = LD_LOCKPARA;
		}else if (stricmp((char*)wfile, (char*)SYSMNT_CAR_INFO_FILENAME) == 0){
			ftpdata.file = LD_CAR_INFO;
			AppServ_CnvCSVtoFLAPDT_Init((char*)SysMnt_Work, sizeof(SysMnt_Work));
		}else{
			return FRESP_WP;
		}
		break;
	case	_DIR_SYS_STBY:
		if (stricmp((char*)wfile, (char*)SYSMNT_PARAM_FILENAME) != 0)
			return FRESP_WP;
		ftpdata.file = LD_PARA_S;
		break;
	case	_DIR_SYS_STBY_MAIN:
		ftpdata.file = LD_MAIN_S;
		ftpdata.write.top_addr = (ulong)GET_MAPP;
		ftpdata.write.header_addr = (ulong)GET_MAIN_H;
		break;
	case	_DIR_SYS_STBY_VOICE:
		ftpdata.file = LD_VOICE_S;
		ftpdata.sw = ((BootInfo.sw.wave == OPERATE_AREA1)?1:0);
		ftpdata.write.top_addr = (ulong)GET_VAPP(ftpdata.sw);
		ftpdata.write.header_addr = (ulong)GET_VOICE_H(ftpdata.sw);
		break;	
	case	_DIR_ETC:
		for (i = 0; ; i++) {
			if (etc_files[i].size == 0)
				return FRESP_NOFILE;
			if (_STRICMP(wfile, etc_files[i].name) == 0) {
				break;
			}
		}
		ftpdata.file = etc_files[i].fileno;
		break;
	case	_DIR_TMP:
		if( stricmp((char*)wfile, (char*)SYSMNT_REMOTE_DL_CONNECT) == 0){
			ftpdata.file = LD_CONNECT;
		}else{
			ftpdata.file = LD_AU_SCR;
			// �t�@�C�����̃`�F�b�N
			if (au_check_filename(wfile) == 0)
		    	return FRESP_WP;
		}
		break;
	case	_DIR_REMOTEDL:
		ftpdata.file = LD_REMOTEDL;
		ftpdata.write.top_addr = (ulong)GET_MAPP;
		ftpdata.write.header_addr = (ulong)GET_MAIN_H;
		break;
	default:
		return FRESP_WP;
	}

	_STRCPY(ftpctrl.filename, wfile); // ��ʕ\���p
	ftpdata.recv.ofs = 0;
	ftpdata.recv.len = 0;
	ftpdata.recv.size = sizeof(SysMnt_Work);
	memset( SysMnt_Work, 0, sizeof( SysMnt_Work ) );
	ftpdata.recv.buff = SysMnt_Work;
	ftpdata.tra_start = 0;
	ftp_disp_file_title(1);

	return FRESP_OK;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_read_param(read parameter)
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer
///	@return			send length
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ulong	ftp_read_param(char *buff)
{
	uchar	finish;
	ushort	datalen;
	t_AppServ_ParamInfoTbl	*param_tbl;

	if (ftpdata.send.step == 1) {
		if (ftpdata.file == LD_PARA|| ftpdata.file ==LD_PARAM_UP) {
			param_tbl =	(void*)FLT_GetParamTable();
		}
		else {
			param_tbl = &param_tbl_wk;
			FLT_ReadStbyParamTbl(param_tbl);
		}
		AppServ_ConvParam2CSV_Init(param_tbl, SYSMNT_DATALEN_MAX);
		ftpdata.send.step++;
	}
	if (ftpdata.send.step == 2) {
		AppServ_CnvParam2CSV(buff, &datalen, &finish);
		if (finish)
			ftpdata.send.step++;
		return (ulong)datalen;
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_read_lockparam(read lock parameter)
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer
///	@return			send length
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/12/03<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static	ulong	ftp_read_lockparam(char *buff)
{
	uchar	finish;
	ushort	datalen;

	if (ftpdata.send.step == 1) {
		AppServ_CnvLockInfo2CSV_Init();
		ftpdata.send.step++;
	}
	if (ftpdata.send.step == 2) {
		AppServ_CnvLockInfo2CSV(buff, &datalen, &finish);
		if (finish)
			ftpdata.send.step++;
		return (ulong)datalen;
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_read_log(read sale/total log)
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer
///	@return			send length
///	@attention		None
///	@author			MATSUSHITA
/// @note			���O�P���Â��M����i���Z���O�P���Ŗ�380�o�C�g�j
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ulong	ftp_read_log(char *buff)
{
	uchar	finish = 0;
	uchar	kind;
	ulong	datalen;

	kind = eLOG_PAYMENT;
	if (ftpdata.send.step == 1) {
		switch( ftpdata.file ){
			case LD_SLOG:
				kind = eLOG_PAYMENT;
				break;
			case LD_TLOG:
				kind = eLOG_TTOTAL;
				break;
		}
		AppServ_SaleTotal_LOG_Init( kind, GetFileSize(ftpdata.file) );
		ftpdata.send.step++;
	}else if( ftpdata.send.step == 3 ){
		return 0;
	}

	AppServ_SaleTotal_LOG_edit(buff, &datalen, &finish);
	
	if (finish){
		ftpdata.send.step++;
	}
	return datalen;
	
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_read_otherlog
//[]----------------------------------------------------------------------[]
///	@param[in/out]	buff	: buffer
///	@param[in]		kind	: 0:Err 1:Arm 2:Monitor 3:OpeMonitor
///	@return			send length
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2009/01/21<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
static	ulong	ftp_read_otherlog(char *buff, uchar kind )
{
	uchar	finish = 0;
	ulong	datalen;

	if (ftpdata.send.step == 1) {
		AppServ_ConvErrArmCSV_Init( kind );
		ftpdata.send.step++;
	}else if( ftpdata.send.step == 3 ){
		return 0;
	}

	AppServ_ConvErrArmCSV(buff, &datalen, &finish);
	if (finish)
		ftpdata.send.step++;

	return (ulong)datalen;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_read_chkresult
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer
///	@return			send length
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/05/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ulong	ftp_read_chkresult(char *buff )
{
	uchar	finish = 0;
	ulong	datalen = 0;

	if (ftpdata.send.step == 1) {
		AppServ_ConvChkResultCSV_Init();
		ftpdata.send.step++;
	}else if( ftpdata.send.step == 3 ){
		return 0;
	}

	AppServ_ConvChkResultCSV(buff, &datalen, &finish);
	if (finish){
		ftpdata.send.step++;
	}
	return (ulong)datalen;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_add_sum(add sum)
//[]----------------------------------------------------------------------[]
///	@param[in]		data	: data address
///	@param[in]		len		: data length
///	@return			sum
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static ushort ftp_add_sum(const uchar* data, ulong len)
{     
	ushort	sum = 0;

	while (len--) {
		WACDOG;
		sum += *data;
		data++;
	}
	return sum;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_flash_written(wait for complete flash-write)
//[]----------------------------------------------------------------------[]
///	@param[in]		ret		: data address
///	@param[in]		len		: return value from flatask function
///	@return			1/0		: OK(1)/  (0)
///	@attention		
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_flash_written_auto(ulong ret)
{
	ushort	msg;
	int		flg;

	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return 0;
	}
	/* wait flash written */
	Lagtim(REMOTEDLTCBNO, 2, 1);
	flg = 1;
	while (flg) {
		msg = StoF( GetMessageRemote(), 1 );
		switch(msg) {
		case	TIMEOUT2:
			if (FLT_ChkWriteCmp(ret, &ret)) {
				flg = 0;
				break;
			}
			Lagtim(REMOTEDLTCBNO, 2, 1);
			break;
		case	FTPMSG_CTRL_RECV:
		case	FTPMSG_CTRL_SEND:
		case	FTPMSG_FILE_SEND:
		case	TIMEOUT1:
			queset(REMOTEDLTCBNO, msg, 0, NULL);
			break;
		default:
			break;
		}
	}
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return 0;
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_flash_written(wait for complete flash-write)
//[]----------------------------------------------------------------------[]
///	@param[in]		ret		: data address
///	@param[in]		len		: return value from flatask function
///	@return			1/0		: OK(1)/  (0)
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_flash_written_manual(ulong ret)
{
	ushort	msg;
	int		flg;
	char	on_off;		// "���~"�L�[�����ł�OPECTL.on_off���]�p�ϐ�

	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return 0;
	}
	/* wait flash written */
	Lagtim(OPETCBNO, 6, 1);
	flg = 1;
	while (flg) {
		msg = StoF( GetMessage(), 1 );
		switch(msg) {
		case	TIMEOUT6:
			if (FLT_ChkWriteCmp(ret, &ret)) {
				flg = 0;
				break;
			}
			Lagtim(OPETCBNO, 6, 1);
			break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case	LCD_DISCONNECT:
			sysment2_lcd_disconnect();
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case	KEY_MODECHG:
			sysment2_change_mode();
			break;
		case	FTPMSG_CTRL_RECV:
		case	FTPMSG_CTRL_SEND:
		case	FTPMSG_FILE_SEND:
		case	TIMEOUT7:
			queset(OPETCBNO, msg, 0, NULL);
			break;
		case 	KEY_TEN_F3:			// "���~"�L�[
			if (OPECTL.on_off == 1) {
				// ON�̎��̂�queset()
				on_off = OPECTL.on_off;
				queset(OPETCBNO, msg, 1, &on_off);
			}
			break;
		default:
			break;
		}
	}
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return 0;
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_flash_written(function select)
//[]----------------------------------------------------------------------[]
///	@param[in]		ret		: data address
///	@return			1/0		: OK(1)/  (0)
///	@attention		�t���b�V�������݊����҂��֐��ւ̃o�C�p�X����
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010/10/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
static	int	ftp_flash_written(ulong ret)
{
	if( DOWNLOADING() ){
		return(ftp_flash_written_auto(ret));
	}else{
		return(ftp_flash_written_manual(ret));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_write_prog(write program into flash)
//[]----------------------------------------------------------------------[]
///	@return			1/0		: OK(1)/  (0)
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int	ftp_write_prog(void)
{
	ulong	wlen;
	ulong	fla_addr;
	ulong	ret;
	int		res = 1;			// �߂�l�̏����l�͐�����Z�b�g
	ulong	header_index = 0;
	t_prog_chg_info *dl = remotedl_info_get();
	
	if (ftpdata.write.base_addr == 0) {
	// if 1st write
		if( (ftpdata.file == LD_MAIN_S) || (ftpdata.file == LD_REMOTEDL) ){
			ret	= 0;								// area & file unmatch error
		}else{
			ret = 1;
		}
		if( memcmp (((PRG_HDR*)SysMnt_Work)->type, &BinType[ret][0], sizeof(BinType[ret] )) != 0 ) {
			ftpdata.write.error = 2;			// �t�@�C���ُ�
			remotedl_monitor_info_set(29);
			res = 0;							// NG�Ƃ���
			goto ftp_write_prog_end;			// �I��������
		}

		if (remotedl_connect_type_get() == CTRL_PROG_DL) {
			// ���ԃ`�F�b�N
// MH810100(S) K.Onodera 2020/03/24 �Ԕԃ`�P�b�g���X�i���Ń`�F�b�N�C���j
//			if( memcmp(((PRG_HDR*)SysMnt_Work)->version, VERSNO.ver_part, 6) != 0) {
//				ftpdata.write.error = 2;			// �t�@�C���ُ�
//				remotedl_monitor_info_set(30);
//				res = 0;							// NG�Ƃ���
//				goto ftp_write_prog_end;			// �I��������
//			}
// MH810100(E) K.Onodera 2020/03/24 �Ԕԃ`�P�b�g���X�i���Ń`�F�b�N�C���j
			// �v����������+�o�[�W�����ƈ�v���邩�H
			if( memcmp(((PRG_HDR*)SysMnt_Work)->version, &dl->script[PROGNO_KIND_DL], 8) != 0) {
				ftpdata.write.error = 2;			// �t�@�C���ُ�
				remotedl_monitor_info_set(48);
				res = 0;							// NG�Ƃ���
				goto ftp_write_prog_end;			// �I��������
			}
		}

		ftpdata.write.base_addr = GetWriteTopAddress();
		
		FLT_DirectEraseSector( ftpdata.write.header_addr );
															
		memcpy(&ftpdata.write.header, &SysMnt_Work[0], sizeof(PRG_HDR));		// �w�b�_�ۑ�
		
		header_index = sizeof(PRG_HDR);
		memmove( SysMnt_Work, &SysMnt_Work[header_index], ftpdata.recv.len );
		ftpdata.recv.len -= header_index;

		wlen = ftpdata.recv.len;
		
		if( wlen > FLT_PROGRAM_SECT_SIZE ){
			wlen = FLT_PROGRAM_SECT_SIZE;
		}
		// add sum(header������)
		ftpdata.write.sum = ftp_add_sum(SysMnt_Work, wlen);
	} else {
		if ((ftpdata.recv.ofs + ftpdata.recv.len) <= ftpdata.recv.size) {
			wlen = ftpdata.recv.len;
		} else {
			wlen = ftpdata.recv.size - ftpdata.recv.ofs;		// �o�b�t�@�I�[�܂ł��P�������ݒP�ʂƂ���
		}
		if( wlen > FLT_PROGRAM_SECT_SIZE ){						// �P��̏����݂͂P�Z�N�^�Ƃ���
			wlen = FLT_PROGRAM_SECT_SIZE;
		}
		// add sum
		ftpdata.write.sum += ftp_add_sum(&SysMnt_Work[ftpdata.recv.ofs], wlen);
	}
	// flash write
	fla_addr = ftpdata.write.base_addr + ftpdata.recv.ofs;
	ret = FLT_WriteImage((char*)&SysMnt_Work[ftpdata.recv.ofs], wlen,
													fla_addr, (fla_addr & 0xffff)? 0 : 1);
	res = ftp_flash_written(ret);		// �����ݑ҂�����
	if( !res ){							
		ftpdata.write.error = 1;		// �������݃G���[
		remotedl_monitor_info_set(31);
		res = 0;						// NG�Ƃ���
		goto ftp_write_prog_end;
	}else{
		res = 1;						// ����I��
	}
	ftpdata.recv.len -= wlen;
	ftpdata.recv.ofs += wlen;
	if (ftpdata.recv.ofs >= ftpdata.recv.size) {
		ftpdata.recv.ofs -= ftpdata.recv.size;
		ftpdata.write.base_addr += ftpdata.recv.size;
	}
ftp_write_prog_end:
	// �����ݐ����E���s�Ɋւ�炸�A�����Ƃ��ēo�^����B
	if( DOWNLOADING() ){				// ���u�_�E�����[�h���s�̏ꍇ
		remotedl_exec_info_set( FLASH_WRITE_END );	// NG���ʗ��R�X�V
	}
// MH810100(S) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iLCD�f�t�H���g�N�����[�h�Ή��j
//	if( read_rotsw() == 5 ){
//		if(res == 1){// ����
//			f_SoundIns_OK = 1;// ����
//		}else{
//			f_SoundIns_OK = 0;// ���s
//		}
//	}
// MH810100(E) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iLCD�f�t�H���g�N�����[�h�Ή��j
	return res;
}

//[]----------------------------------------------------------------------[]
///	@brief			�o�C�i���t�@�C���̃w�b�_�A�h���X�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			address	:�J�n�A�h���X
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong	GetWriteHeaderAddress( void )
{
	return ftpdata.write.header_addr;
}

//[]----------------------------------------------------------------------[]
///	@brief			�o�C�i���[�t�@�C���_�E�����[�h�J�n�A�h���X�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			address	:�J�n�A�h���X
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong	GetWriteTopAddress( void )
{
	return ftpdata.write.top_addr;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_on_file_send(file send)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
long	ftp_on_file_send( void *buf, long size )
{
	long	wsize = 0;
	uchar	*p,i;
	int		len;
	
	if (ftpdata.write.error) {
		if(	ftpdata.write.error & 0x80 ){
			ftp_send_cancel( 0 );
		}
		return -1;
	}
	_SET_WATCH_TIMER();
	FAIL_SAFE_TIMER_START( GetTaskID(), GetFailSafeTimerNo() );
	ftpctrl.send_cmd |= 0x80;
	ftpdata.tra_start = 1;
	if( ftpdata.file == LD_MAIN_S || 
		ftpdata.file == LD_VOICE_A ||
		ftpdata.file == LD_VOICE_S ){
		if( size ){
			if( ftpdata.send.buff == NULL ){
				ftpdata.send.buff = SysMnt_Work;
				Flash2Read( ftpdata.send.buff, ftpdata.send.header_addr, sizeof(PRG_HDR) );
				
				ftpdata.send.len = (((PRG_HDR*)ftpdata.send.buff)->proglen+sizeof(PRG_HDR));
				ftpdata.send.index = 0;
				
				ftpdata.send.base_addr = ftpdata.send.top_addr;
				wsize = sizeof(PRG_HDR);
			}else{
				wsize = size;
				if( ftpdata.send.len ){
					if( ftpdata.send.len < wsize ){
						wsize = ftpdata.send.len;
					}
					Flash2Read( ftpdata.send.buff, ftpdata.send.base_addr, wsize );					
					ftpdata.send.base_addr += wsize;
				}else{
					wsize = 0;
				}
			}
			if( wsize > size ){
				wsize = size;
			}
			if( wsize ){
				memcpy( buf, ftpdata.send.buff, wsize );
				ftpdata.send.index += wsize;
				ftpdata.send.len -= wsize;
			}
		}
	}else{
		if(	ftpdata.send.finish ){
			wsize = 0;
			_displclr_6Line();
		}else{
			if( ftpdata.send.buff == NULL || !ftpdata.send.len ){
				ftpdata.send.buff = SysMnt_Work;
				ftpdata.send.index = 0;
				
				switch( ftpdata.file ){
					case	LD_PARA:
					case	LD_PARA_S:
						ftpdata.send.len = ftp_read_param((char*)ftpdata.send.buff);
						if( !ftpdata.send.len ){
							ftpdata.send.finish = 1;
						}
						break;
					case	LD_LOCKPARA:
						ftpdata.send.len = ftp_read_lockparam((char*)ftpdata.send.buff);
						if( !ftpdata.send.len ){
							ftpdata.send.finish = 1;
						}
						break;
					case	LD_SLOG:
					case	LD_TLOG:
						ftpdata.send.len = ftp_read_log((char*)ftpdata.send.buff);
						if( !ftpdata.send.len ){
							ftpdata.send.finish = 1;
						}
						break;
					case	LD_ERR:
					case	LD_ARM:
					case	LD_MONI:
					case	LD_OPE_MONI:
						ftpdata.send.len = ftp_read_otherlog((char*)ftpdata.send.buff, (uchar)(ftpdata.file-LD_ERR) );
						if( !ftpdata.send.len ){
							ftpdata.send.finish = 1;
						}
						break;
					case	LD_CAR_INFO:
						ftpdata.send.len = ftp_read_otherlog((char*)ftpdata.send.buff, 4 );
						if( !ftpdata.send.len ){
							ftpdata.send.finish = 1;
						}
						break;
					case	LD_PROG:
						ftpdata.send.len = ftp_make_progfile((char*)ftpdata.send.buff);
						ftpdata.send.finish = 1;
						break;
					case	LD_REMOTE_RES:
						ftpdata.send.len = 0;
						ftpdata.send.finish = 1;
						break;
					case	LD_LOGO:
					case	LD_HDR:
					case	LD_FTR:
					case	LD_ACCEPTFTR:
					case	LD_SYOM:
					case	LD_KAME:
					case	LD_TCARD_FTR:
					case	LD_AZU_FTR:
					case	LD_EDYAT:
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
					case	LD_EMG:
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
						for (i = 0; ; i++) {							// �t�@�C���I�[�v�����Ɉ��m�F���Ă���̂ŁA�K���f�[�^�͂���
							if ( etc_files[i].fileno == ftpdata.file ) {
								break;
							}
						}
						p = (uchar*)&ETC_cache[etc_files[i].ofs];
						p += FLT_HEADER_SIZE;
						if( ftpdata.file == LD_LOGO ){
							ftpdata.send.buff = p;
							ftpdata.send.len = get_bmplen(p);
							ftpdata.send.base_addr = ftpdata.send.len;
						}else{
						// NOTE : �������E�����X�͕ʊ֐������A���e������
							switch( ftpdata.file ){
								case	LD_TCARD_FTR:
									ftpdata.send.len = AppServ_InsertCrlfToTextData((char*)ftpdata.send.buff, (void*)p, T_FOOTER_GYO_MAX);
									break;
								case	LD_EDYAT:
									ftpdata.send.len = AppServ_InsertCrlfToEdyAtCommand((char*)ftpdata.send.buff, (void*)p);
									break;
								default:
									ftpdata.send.len = AppServ_InsertCrlfToHeaderFooter((char*)SysMnt_Work, (void*)p);
									break;
							}
							ftpdata.send.finish = 1;
						}
						break;
					case	LD_CHKMODE_RES:
						ftpdata.send.len = ftp_read_chkresult((char*)ftpdata.send.buff);
						if( !ftpdata.send.len ){
							ftpdata.send.finish = 1;
						}
						break;
					case	LD_PARAM_UP:
						// read �` ���k �` �Í���
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//						ftpdata.send.len = ftp_read_param_All((char*)ftpdata.send.buff,sizeof(SysMnt_Work));
						ftpdata.send.len = ftp_read_param_All( (char*)g_TempUse_Buffer,sizeof(g_TempUse_Buffer) );
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
						if( ftpdata.send.len ){
							_SET_WATCH_TIMER();
							// ���k
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//							len = zlib_Compress(ftpdata.send.buff, g_TempUse_Buffer, ftpdata.send.len, sizeof(g_TempUse_Buffer));
							memset( g_TempUse_Buffer2, 0, sizeof(g_TempUse_Buffer2) );
							len = zlib_Compress( g_TempUse_Buffer, g_TempUse_Buffer2, ftpdata.send.len, sizeof(g_TempUse_Buffer2) );
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
							if (len < 0) {
								ftpdata.send.finish = 1;	//�G���[�Ȃ̂ŏI��������
								break;
							}
							// �Í�
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//							len = EncryptWithKeyAndIV(CRYPT_KEY_REMOTEDL, g_TempUse_Buffer, ftpdata.send.buff,len, sizeof(SysMnt_Work));
							len = EncryptWithKeyAndIV( CRYPT_KEY_REMOTEDL, g_TempUse_Buffer2, ftpdata.send.buff, len, sizeof(SysMnt_Work) );
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
							if (!len) {
								ftpdata.send.finish = 1;	//�G���[�Ȃ̂ŏI��������
								break;
							}
							ftpdata.send.len = len;
						}else{
							// 2��ڂ̌ďo��finish�𗧂Ă�
							ftpdata.send.finish = 1;
						}

						break;
					case	LD_CONNECT:
						ftpdata.send.len = AppServ_MakeRemoteDl_TestConnect((char*)ftpdata.send.buff,sizeof(SysMnt_Work));
						ftpdata.send.finish = 1;
						break;

// MH810100(S) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
					// LCD�ւ̃p�����[�^�A�b�v���[�h
					case LD_PARAM_UP_LCD:
						ftpdata.send.len = ftp_output_dat_param( (char*)ftpdata.send.buff,sizeof(SysMnt_Work) );
						if( ftpdata.send.len ){
							;
						}else{
							ftpdata.send.finish = 1;
						}
						break;
// MH810100(E) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
				}
			}
			wsize = ftpdata.send.len;
			if( ftpdata.send.len > size ){
				wsize = size;
			}
			if( wsize ){
				memcpy( buf, &ftpdata.send.buff[ftpdata.send.index], wsize );
				ftpdata.send.index += wsize;
				ftpdata.send.len -= wsize;
			}else{
				_displclr_6Line();
			}
			if( ftpdata.send.base_addr && !ftpdata.send.len ){		// �r�b�g�}�b�v�f�[�^�͍ēǂݍ��݂��Ȃ��̂ŁA�����ŏI������
				ftpdata.send.finish = 1;
			}
		}
	}
	ftpctrl.send_cmd &= 0x7F;

	if( !wsize ){
		ftpdata.tra_start = 0;
	}
	return wsize;
	
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_on_file_recv(file receive)
//[]----------------------------------------------------------------------[]
///	@param[in]		end		: 
///	@return					: -1=CONTINUE
///							  0=NORMAL END
///							  1=FLASH ERROR
///							  2=FILE ERROR
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int		ftp_on_file_recv(int end)
{
	char*	p_work;
	ulong	ret;
	int		len;
	
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	ulong addr, value;
	ushort sec, item;
	ushort	TgtSecTopIndex;			/* ���[�N�G���A�擪����̑�Index�� */
	ulong	*pTgtSecTop;			/* �ړI�Z�N�^�[�̐擪�A�C�e���A�h���X */
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)


	FAIL_SAFE_TIMER_START( GetTaskID(), GetFailSafeTimerNo() );
	if ( end && !ftpdata.write.error ){
		_displclr_6Line();
	}
	if (ftpdata.write.error) {
		if(	ftpdata.write.error & 0x80 ){
			ftp_send_cancel( 0 );
		}
		return(ftpdata.write.error);
	}
	
	if (ftpdata.file == LD_MAIN_S || ftpdata.file == LD_VOICE_S ) {
		if (end) {
			if (ftpdata.write.base_addr == 0 && ftpdata.recv.len < sizeof(PRG_HDR)) {
			// file size NG
				ftpdata.write.error = 2;
				return(ftpdata.write.error);
			}
			while(ftpdata.recv.len) {
			// write remain data
				if (!ftp_write_prog()) {
					return(ftpdata.write.error);
				}
			}

			if (ftpdata.write.sum != ftpdata.write.header.sum) {	// check sum
			// sum error
				ftpdata.write.error = 2;
				queset( OPETCBNO, FTP_FORCE_LOGOUT, 0, NULL );
				return(ftpdata.write.error);
			}
			
			if( ftpdata.file == LD_MAIN_S ){
				ret = FLT_write_program_version( (uchar*)&ftpdata.write.header );
			}else{
				ret = FLT_write_wave_sum_version( ftpdata.sw, (uchar*)&ftpdata.write.header );
			}
			if( ret ){							
				ftpdata.write.error = 1;		// �������݃G���[
				return(ftpdata.write.error);
			}
			// �����̓e�X�g����
			wopelg2(OPLOG_PRG_DL, 0, 0);	// �v���O�����_�E�����[�h���{
		}
		else {
			if (ftpdata.recv.len >= 4096) {
			// ���ȏ㗭�����珑��
				if (!ftp_write_prog()) {
					ftp_send_cancel( 0 );
					if( DOWNLOADING() ){
						ftpdata.write.error += 2;					// �����ݎ��s�ŁA�L�����Z���R�}���h���M
					}

					return(ftpdata.write.error);
				}
			}
		}
	}
	else if (ftpdata.file == LD_PARA_S) {
		ulong	dummy;
		ulong	ret;
		uchar	*rp, *endp, *sp;

	// �p�����[�^�t�@�C���͎�M�o�b�t�@�T�C�Y�ȉ��Ƃ���
		if (!end){
			return 0;
		}
// MH810100(S) K.Onodera 2020/04/09 �ÓI���
//		if (ftpdata.recv.len < 1 || ftpdata.recv.len >= PARAMETER_CSV_MAX){
		if (ftpdata.recv.len < 1 || ftpdata.recv.len >= PARAM_CSV_OLD_FORM_MAX){
// MH810100(E) K.Onodera 2020/04/09 �ÓI���
			return 2;
		}
		memcpy(FTP_buff, SysMnt_Work, ftpdata.recv.len);
	// convert CSV to imange
		AppServ_ConvCSV2Param_Init(FLT_GetParamTable(), (char*)SysMnt_Work, sizeof(SysMnt_Work));
		rp = (uchar*)FTP_buff;
		endp = rp + ftpdata.recv.len;
		do {
			sp = rp;
			while(1) {
				if (rp >= endp){
					return 2;		// invalid CR
				}
				if (*rp == '\r') {
					rp++;
					break;
				}
				rp++;
			}
			p_work = (char*)(rp-sp);

			if (AppServ_CnvCSV2Param((char*)sp, (ushort)(p_work), &dummy) == 0){
				return 2;			// invalid CSV format
			}
			if (*rp == '\n'){
				rp++;		// remove LF
			}
		} while(rp < endp);
		ret = FLT_WriteParam3((char*)SysMnt_Work, 
						AppServ_ConvCsvParam_ImageAreaSizeGet(FLT_GetParamTable()),
						FLT_NOT_EXCLUSIVE);
		if (!ftp_flash_written(ret)){
			return 1;
		}
		wopelg2(OPLOG_CPARAM_DL, 0, 0);	// ���ʃp�����[�^�_�E�����[�h���{
	}
#ifdef	LOCK_MAX
	else if (ftpdata.file == LD_LOCKPARA) {
		ulong	dummy;
		ulong	ret;
		uchar	*rp, *endp, *sp;

	// �p�����[�^�t�@�C���͎�M�o�b�t�@�T�C�Y�ȉ��Ƃ���
		if (!end){
			return 0;
		}
// MH810100(S) K.Onodera 2020/04/09 �ÓI���
//		if (ftpdata.recv.len < 1 || ftpdata.recv.len >= PARAMETER_CSV_MAX){
		if (ftpdata.recv.len < 1 || ftpdata.recv.len >= PARAM_CSV_OLD_FORM_MAX){
// MH810100(E) K.Onodera 2020/04/09 �ÓI���
			return 2;
		}
		memcpy(FTP_buff, SysMnt_Work, ftpdata.recv.len);
	// convert CSV to image
		AppServ_CnvCSV2LockInfo_Init((char*)SysMnt_Work, sizeof(SysMnt_Work));
		rp = (uchar*)FTP_buff;
		endp = rp + ftpdata.recv.len;
		do {
			sp = rp;
			while(1) {
				if (rp >= endp){
					return 2;		// invalid CR
				}
				if (*rp == '\r') {
					rp++;
					break;
				}
				rp++;
			}
			p_work = (char*)(rp-sp);

			if (AppServ_CnvCSV2LockInfo((char*)sp, (ushort)(p_work), &dummy) == 0){
				return 2;			// invalid CSV format
			}
			if (*rp == '\n'){
				rp++;		// remove LF
			}
		} while(rp < endp);
		ret = FLT_WriteLockParam2((char*)SysMnt_Work, FLT_NOT_EXCLUSIVE);
		if (!ftp_flash_written(ret)){
			return 1;
		}
#ifdef	OPLOG_LKPRM_DL
		wopelg2(OPLOG_LKPRM_DL, 0, 0);	// �Ԏ��p�����[�^�_�E�����[�h���{
#endif
		memcpy(LockInfo, SysMnt_Work, sizeof(LockInfo));	// RAM�ɔ��f
		DataSumUpdate(OPE_DTNUM_LOCKINFO);					// RAM���SUM�X�V
	}
#endif	// LOCK_MAX
	else if (ftpdata.file == LD_AU_SCR) {
		if (end) {
			au_set_script((char*)SysMnt_Work, ftpdata.recv.len);
			ftpctrl.script = 1;
		}
	}
#ifdef	LOCK_MAX
	else if (ftpdata.file == LD_CAR_INFO) {
		ulong	dummy;
		uchar	*rp, *endp, *sp;
	// �p�����[�^�t�@�C���͎�M�o�b�t�@�T�C�Y�ȉ��Ƃ���
		if (ftpdata.recv.len < 1 || ftpdata.recv.len >= PARAMETER_CSV_MAX){
			return 2;
		}
			
		// ������x�o�b�t�@�ɗ��܂����珈������B ���݂́A16384Byte�ȏ㗭�܂����珈������B
		if( ftpdata.recv.len < (PARAMETER_CSV_MAX/2) && !end ){
			return 0;
		}
		
		memcpy(&FTP_buff[ftpdata.recv.ofs], &SysMnt_Work[ftpdata.recv.ofs], ftpdata.recv.len);
		if( ftpdata.recv.ofs ){
			ftpdata.recv.len += ftpdata.recv.ofs;
		}
	// convert CSV to image
		rp = (uchar*)FTP_buff;
		endp = rp + ftpdata.recv.len;

		do {
			sp = rp;
			while(1) {
				if (rp >= endp){
					if( end ){
						break;
					}
					memcpy(FTP_buff, sp, (size_t)(rp-sp));
					ftpdata.recv.ofs = (ulong)(rp-sp);
					ftpdata.recv.len = 0;
					return 0;
				}
				if (*rp == '\r') {
					rp++;
					break;
				}
				rp++;
			}

			p_work = (char*)(rp-sp);

			if (AppServ_CnvCSVtoFLAPDT((char*)sp, (ushort)(p_work), &dummy) == 0){
				return 2;			// invalid CSV format
			}
			
			if (*rp == '\n'){
				rp++;		// remove LF
			}
		} while(rp < endp);
		
		if( !dummy ){
			return 2;			// invalid CSV format
		}
			
		ftpdata.recv.ofs = ftpdata.recv.len = 0;
	}
#endif
	else if (ftpdata.file == LD_REMOTEDL) {
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		ulong	dummy;
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�����e�i���X)
		uchar	*rp, *endp, *sp;
// MH810100(S) S.Nishimoto 2020/04/07 �ÓI���(20200407:113,114)�Ή�
		int     have_comma = 0;
// MH810100(E) S.Nishimoto 2020/04/07 �ÓI���(20200407:113,114)�Ή�

		if (!end) {
			return 0;
		}

		do {
// GG120600(S) // Phase9 CRC�`�F�b�N
			// �擪��2�o�C�g�����o��
			if(remotedl_connect_type_get() == CTRL_PROG_DL && ftpctrl.cur_dir_ent == 1) {
				ftp_set_write_crc(SysMnt_Work[0],SysMnt_Work[1]);
				ftpdata.recv.len = ftpdata.recv.len -2;
				//CRC�N���A
				ftpctrl.CRC16.uscrcTemp = 0;
				memmove(SysMnt_Work,&SysMnt_Work[2],ftpdata.recv.len);
			}
// GG120600(E) // Phase9 CRC�`�F�b�N
			// ����
			ret = DecryptWithKeyAndIV(CRYPT_KEY_REMOTEDL, SysMnt_Work, g_TempUse_Buffer, ftpdata.recv.len, sizeof(g_TempUse_Buffer));
			if (!ret) {
				ftpdata.write.error = 2;
				remotedl_monitor_info_set(20);
				break;
			}

			// ��
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//			len = zlib_Uncompress(g_TempUse_Buffer, SysMnt_Work, ret, sizeof(SysMnt_Work));
// MH810100(S) K.Onodera 2020/03/24 �Ԕԃ`�P�b�g���X(#4088 �v���O�����_�E�����[�h�����s���Ă��܂�)
//			len = zlib_Uncompress( g_TempUse_Buffer, g_TempUse_Buffer2, ret, sizeof(g_TempUse_Buffer2) );
			if( remotedl_connect_type_get() == CTRL_PARAM_DL || 
				remotedl_connect_type_get() == CTRL_PARAM_DIF_DL ){
				len = zlib_Uncompress( g_TempUse_Buffer, g_TempUse_Buffer2, ret, sizeof(g_TempUse_Buffer2) );
			}else{
				len = zlib_Uncompress(g_TempUse_Buffer, SysMnt_Work, ret, sizeof(SysMnt_Work));
			}
// MH810100(E) K.Onodera 2020/03/24 �Ԕԃ`�P�b�g���X(#4088 �v���O�����_�E�����[�h�����s���Ă��܂�)
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			if (len < 0) {
				ftpdata.write.error = 2;
				remotedl_monitor_info_set(21);
				break;
			}
			ftpdata.recv.len = len;
		} while (0);

		if (ftpdata.write.error) {
			return (ftpdata.write.error);
		}

		switch (remotedl_connect_type_get()) {
		case CTRL_PROG_DL:
// GG120600(S) // Phase9 CRC�`�F�b�N
			// CRC�X�V����
			crc_ccitt_update((ushort)ftpdata.recv.len, SysMnt_Work, ftpctrl.CRC16.crcTemp, R_SHIFT);
// GG120600(E) // Phase9 CRC�`�F�b�N
			// Flash������
			do {
				// 4KB����Flash�ɏ�����
				if (!ftp_write_prog()) {
					return(ftpdata.write.error);
				}
				WACDOG;
			} while (ftpdata.recv.len);

			// base_addr��i�߂�
			ftpdata.write.base_addr += len;
			if (ftpctrl.cur_dir_ent == 1) {
				// �w�b�_������
				ret = FLT_write_program_version( (uchar*)&ftpdata.write.header );
				if (ret) {
					ftpdata.write.error = 1;					// �������݃G���[
					remotedl_monitor_info_set(22);
					return(ftpdata.write.error);
				}
				// 1�ڂ̃t�@�C���̓w�b�_���܂߂��T�C�Y�̂��߁A�w�b�_�T�C�Y�������Ă���
				ftpdata.write.base_addr -= sizeof(PRG_HDR);
			}
			// Flash�ւ�write����ێ�
			ftp_set_write_info();
			// ���g���C�J�E���g�N���A
// GG120600(S) // Phase9 ���ʂ���ʂ���
//			retry_count_clr(RETRY_KIND_DL);
			retry_count_clr(RETRY_KIND_DL_PRG);
// GG120600(E) // Phase9 ���ʂ���ʂ���

			// �S�t�@�C��DL�����H
			if (ftpctrl.cur_dir_ent >= ftpctrl.dir_ent_cnt) {
// GG120600(S) // Phase9 CRC�`�F�b�N
				if(ftpctrl.CRC16.uscrcTemp != ftp_get_write_crc()){
					ftpdata.write.error = 5;					// CRC�G���[
					remotedl_monitor_info_set(28);
					// write�����N���A
					remotedl_write_info_clear();
					return(ftpdata.write.error);
				}
// GG120600(E) // Phase9 CRC�`�F�b�N

				// write�����N���A
				remotedl_write_info_clear();

				// SUM�`�F�b�N
				if (ftpdata.write.sum != ftpdata.write.header.sum) {
					ftpdata.write.error = 5;					// SUM�l�G���[
					remotedl_monitor_info_set(23);
					return(ftpdata.write.error);
				}

				// �_�E�����[�h����
				remotedl_result_set( PROG_DL_COMP );
			}
			break;
		case CTRL_PARAM_DL:
// MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case CTRL_PARAM_DIF_DL:		//�����p�����[�^�_�E�����[�h
// MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			if (ftpdata.recv.len < 1 || ftpdata.recv.len >= PARAMETER_CSV_MAX){
				remotedl_monitor_info_set(24);
				return 2;
			}
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//			memcpy(FTP_buff, SysMnt_Work, ftpdata.recv.len);
			memcpy( g_TempUse_Buffer, g_TempUse_Buffer2, ftpdata.recv.len );
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)

			AppServ_ConvCSV2Param_Init(FLT_GetParamTable(), (char*)SysMnt_Work, sizeof(SysMnt_Work));
// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//			rp = (uchar*)FTP_buff;
			rp = (uchar*)g_TempUse_Buffer;
			sp = rp;
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			endp = rp + ftpdata.recv.len;

// MH810100(S) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//			do {
//				sp = rp;
//				while(1) {
//					if (rp >= endp){
//						remotedl_monitor_info_set(25);
//						return 2;		// invalid CR
//					}
//					if (*rp == '\r') {
//						rp++;
//						break;
//					}
//					rp++;
//				}
//				p_work = (char*)(rp-sp);
//
//				if (AppServ_CnvCSV2Param((char*)sp, (ushort)(p_work), &dummy) == 0){
//					remotedl_monitor_info_set(26);
//					return 2;			// invalid CSV format
//				}
//				if (*rp == '\n'){
//					rp++;		// remove LF
//				}
//			} while(rp < endp);
			for( ; rp < endp ; ){
				WACDOG;
				if (rp >= endp){
					remotedl_monitor_info_set(25);
					return 2;		// invalid CR
				}

				// �J���}����
				// �A�h���X��ϊ�
				if (*rp == ','){
					// Ascii -> Hex�ϊ�
					if( 1 == DecAsc_to_LongBin_withCheck((uchar*)sp, (uchar)(rp-sp), &addr) ){
						sec = (ushort)(addr/10000L);
						item = (ushort)(addr%10000L);
					}

					else{										/* ���f�[�^�ُ� */
						return 2;	// invalid CSV format
					}
					rp++;
					sp = rp;
// MH810100(S) S.Nishimoto 2020/04/07 �ÓI���(20200407:113,114)�Ή�
					have_comma = 1;	// �J���}����
// MH810100(E) S.Nishimoto 2020/04/07 �ÓI���(20200407:113,114)�Ή�
					continue;
				}
				// ���s����
				// �l��ϊ�

				if (*rp == '\r'){
// MH810100(S) S.Nishimoto 2020/04/07 �ÓI���(20200407:113,114)�Ή�
 					if( have_comma != 1){
						// �J���}�Ȃ�
						return 2;	// invalid CSV format
					}
// MH810100(E) S.Nishimoto 2020/04/07 �ÓI���(20200407:113,114)�Ή�
					// Ascii -> Hex�ϊ�
					if( 1 == DecAsc_to_LongBin_withCheck((uchar*)sp, (uchar)(rp-sp), &value) ){
						// �p�����[�^�G���A�擪����ړI�Z�N�^�[�擪�A�C�e���܂ł̑��A�C�e����get
						TgtSecTopIndex = AppServ_CnvCsvParam_SkipItemCount_tillTargetSecterGet( 1, sec );
						pTgtSecTop = (ulong*)AppServ_BinCsvSave.Csv_toBin_Param.param_image;	// work area top address get
						pTgtSecTop += TgtSecTopIndex;
						pTgtSecTop[item] = value;
					}
					else{										/* ���f�[�^�ُ� */
						return 2;	// invalid CSV format
					}
					rp++;
					if( *rp == '\n' ){
						rp++;
					}
// MH810100(S) S.Nishimoto 2020/04/07 �ÓI���(20200407:113,114)�Ή�
					have_comma = 0;	// �J���}�t���O�N���A
// MH810100(E) S.Nishimoto 2020/04/07 �ÓI���(20200407:113,114)�Ή�
					sp = rp;
					continue;
				}
				rp++;
			};
// MH810100(E) Y.Yamauchi 20191120 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			ret = FLT_WriteParam3((char*)SysMnt_Work, 
							AppServ_ConvCsvParam_ImageAreaSizeGet(FLT_GetParamTable()),
							FLT_NOT_EXCLUSIVE);
			if (!ftp_flash_written(ret)){
				remotedl_monitor_info_set(27);
				return 1;
			}
			// �_�E�����[�h����
			remotedl_result_set( PROG_DL_COMP );
			break;
		default:
			break;
		}
	}
	else if (ftpdata.file == LD_CONNECT) {
		return 0;
	}
	else {
		uchar	*src, *dst;
		ulong	wlen;
		int		i;

		if (!end){
			return 0;
		}
		for (i = 0; ; i++) {
			if (etc_files[i].size == 0){
				return 2;
			}
			if (etc_files[i].fileno == ftpdata.file){
				break;
			}
		}
		if (ftpdata.file == LD_LOGO) {
			if (ftpdata.recv.len != get_bmplen(SysMnt_Work)){
				return 2;
			}
			src = (uchar*)SysMnt_Work;
		}
		else if (ftpdata.file == LD_EDYAT) {
			AppServ_FormatEdyAtCommand(SysMnt_Work3, (char*)SysMnt_Work,
											(ushort)ftpdata.recv.len);
			src = (uchar*)SysMnt_Work3;
		}
		else if( ftpdata.file == LD_TCARD_FTR ){
			AppServ_FormatTextData(SysMnt_UP_DW_Text, (char*)SysMnt_Work,
											(ushort)ftpdata.recv.len, T_FOOTER_GYO_MAX);
			src = (uchar*)SysMnt_UP_DW_Text;	
		}
		else {
		// NOTE : �������E�����X�͕ʊ֐������A���e������
			AppServ_FormatHeaderFooter(SysMnt_Work2, (char*)SysMnt_Work,
											(ushort)ftpdata.recv.len);
			src = (uchar*)SysMnt_Work2;
		}
		dst = (uchar*)&ETC_cache[etc_files[i].ofs];
		wlen = (ulong)etc_files[i].size;
		memcpy(dst+FLT_HEADER_SIZE, src, wlen);
		FLT_SetSum(dst, wlen+FLT_HEADER_SIZE);
		ftpctrl.etc_update |= (1L << i);
	}

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_file_recv_Data(file receive)
//[]----------------------------------------------------------------------[]
///	@param[in]		end		: 
///	@return					: -1=CONTINUE
///							  0=NORMAL END
///							  1=FLASH ERROR
///							  2=FILE ERROR
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int		ftp_file_recv_Data(int end)
{
	int	ret = 0;

	ftpctrl.send_cmd |= 0x80;
	ret = ftp_on_file_recv( end );
	ftpctrl.send_cmd &= 0x7F;

	if( GetFTPMode() == _FTP_CLIENT ){
		if( GetTaskID() == OPETCBNO ){
			if (ret == 1 || ret == 2) {	// WRITE ERROR or FILE ERROR
				ftpctrl.result = FLASH_WRITE_ERR;
			}else{
				if (ftpctrl.au_sts == AU_DWL_SCR) {
					Lagcan(GetTaskID(), 7);
					Lagcan(GetTaskID(), GetFailSafeTimerNo());
				}
			}
		}else{
			if (ret) {
				if( ret == 3 || ret == 4 ){						// �����݃G���[�̂��߃L�����Z���iABORT�j�R�}���h���s���A�]���𒆒f������
					// ���Z�@����ABORT���Ă��܂��ƁA���̒ʐM���ł̓t�@�C���̃A�b�v���[�h���o���Ȃ��ׁA
					// ��x���O�A�E�g���āA�ēx���O�C���㑗�M����B
					// ���R�F
					// �f�[�^�ʐM����ABORT�R�}���h�œ]�����~����ƁA�T�[�o�[�����\�P�b�g���[�h�Ɏ��s���āA426(ABORT)��
					// ������Ԃ��̂ŁA�A�b�v���[�h���o���Ȃ��B����|�[�g�ł̂����͉\�����A�f�[�^�|�[�g�ł̃f�[�^�]����
					// �o���Ȃ��Ȃ�͗l�B
				}else{
					ftpdata.write.error += 2;
				}
				ftpctrl.au_sts = AU_ABORT_END;					// ���~�I��
				switch( ret ){
					case	1:
					case	3:
						ret = FLASH_WRITE_ERR;
						break;
					case	2:
					case	4:
						ret = PROG_DL_ERR;
						break;
					case	5:
						ret = PROG_SUM_ERR;
						break;
				}
				remotedl_result_set( (uchar)ret );		// ���ʏ����i�[				
			}else if( ftpctrl.au_sts == AU_EXEC && end ){
				remotedl_result_set( PROG_DL_COMP );	// ���ʏ����i�[				
			}
		}
	}else{
		if( ftpdata.file == LD_MAIN_S	||	// ���C���v���O����
			ftpdata.file == LD_VOICE_S	|| 	// �����f�[�^
			ftpdata.file == LD_PARA_S){		// ���ʃp�����[�^
			// �_�E�����[�h�҂��Ŏ蓮�ɂĐ؊����s���ꍇ�A�v�����N���A����
// GG120600(S) // Phase9 #6185 �ݒ�A�b�v���[�h���s�������ɃV�X�e���ؑւŋ��ʃp�����[�^�ύX����ƁA�u [�ݒ�A�b�v���[�h�@�\]-[�����J�n���茋��]-[����J�n](7000100)�v����������
//			if( remotedl_status_get() == R_DL_REQ_RCV ) {
//// GG120600(S) // Phase9 ��ʖ����`�F�b�N
////				remotedl_status_set(R_DL_IDLE);
//// GG120600(E) // Phase9 ��ʖ����`�F�b�N
//				switch (remotedl_connect_type_get()) {
//				case CTRL_PROG_DL:
//					remotedl_complete_request(REQ_KIND_VER_UP);
//					break;
//				case CTRL_PARAM_DL:
//					remotedl_complete_request(REQ_KIND_PARAM_CHG);	// ���u�����e�i���X�v������
//					break;
//// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//				case CTRL_PARAM_DIF_DL:
//					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG); // ���u�����e�i���X�v������
//// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//					break;
//				default:
//					break;
//				}
//			}
			mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 #6185 �ݒ�A�b�v���[�h���s�������ɃV�X�e���ؑւŋ��ʃp�����[�^�ύX����ƁA�u [�ݒ�A�b�v���[�h�@�\]-[�����J�n���茋��]-[����J�n](7000100)�v����������
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_init(initialize ftp)
//[]----------------------------------------------------------------------[]
///	@param[in]		mode	: 0(SERVER)/1(CLIENT)
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	void	ftp_init(int mode , uchar taskID )
{
	extern	void	Get_IP_adrr( char *ip, char task );
	
	Lagcan(OPETCBNO, 5);				// �h�A�m�u�߂��Y��h�~�^�C�}�[��~
	opa_chime(OFF);						// �`���C����~

	xPause(5);							// ��ʍX�V�p
	FileSystemUpdate();
	memset(&ftpdata, 0, sizeof(ftpdata));
	memset(&ftpctrl, 0, sizeof(ftpctrl));
	memset(&SCR_buff, 0, sizeof( SCR_buff ) );
	memset(&cmd, 0, sizeof(cmd) );
	
	ftpctrl.start = FTP_STOP;
	ftpctrl.login = -1;
	ftpctrl.mode = (uchar)mode;
	SetTaskID( taskID );
	memset( ETC_cache, 0, sizeof( ETC_cache ));
	FLT_ReadEtc(ETC_cache);
	ftpctrl.ftpHandle = 0;		// FTP�Z�b�V�����n���h��������
	if( mode ){
		memset( &ftpctrl.loginbuff, 0, sizeof(ftpctrl.loginbuff));
		memset( ftpctrl.ip, 0, sizeof( ftpctrl.ip ));
		Get_IP_adrr( ftpctrl.ip, GetTaskID() );
	}
	_SET_WATCH_TIMER();
	_sysmnt2_start_FTP();
	
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_exit(exit ftp)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	void	ftp_exit(void)
{
	ulong	ret;
	short 	msg;

	SetTaskID( 0xFF );				// TaskID�̏�����

	if (ftpctrl.etc_update) {
	// write ETC cache into flash
		ret = FLT_WriteEtc(ETC_cache);
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			return;
		}
		/* wait flash written */
		while (1) {
			if( DOWNLOADING() ){
				Lagtim(REMOTEDLTCBNO, 2, 1);
				msg = GetMessageRemote();
				Lagcan(REMOTEDLTCBNO, 2);
			}else{
				Lagtim(OPETCBNO, 6, 1);
				msg = GetMessage();
				Lagcan(OPETCBNO, 6);
				if (msg == KEY_MODECHG) {
					sysment2_change_mode();
				}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				else if ( msg == LCD_DISCONNECT ) {
					sysment2_lcd_disconnect();
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}
			if (FLT_ChkWriteCmp(ret, &ret)) {
				break;
			}
		}
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			return;
		}
	// update print header, footer,...
		if (ftpctrl.etc_update & 2) {
			Header_Rsts	= FLT_ReadHeader( &Header_Data[0][0] );	// �ׯ����؂������������ر�֓Ǎ�
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
			PrnGetRegistNum();
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
		}
		if (ftpctrl.etc_update & 4) {
			Footer_Rsts	= FLT_ReadFooter( &Footer_Data[0][0] );	// �ׯ����؂������������ر�֓Ǎ�
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
			PrnGetRegistNum();
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
		}
		if (ftpctrl.etc_update & 8) {
			Syomei_Rsts	= FLT_ReadSyomei( &Syomei_Data[0][0] );	// �ׯ����؂������������ر�֓Ǎ�
		}
		if (ftpctrl.etc_update & 16) {
			Kamei_Rsts	= FLT_ReadKamei( &Kamei_Data[0][0] );	// �ׯ����؂������������ر�֓Ǎ�
		}
		if (ftpctrl.etc_update & __accept_bit) {
			AcceptFooter_Rsts = FLT_ReadAcceptFooter( &AcceptFooter_Data[0][0] );
		}
	// update print logo
		if (ftpctrl.etc_update & 1) {
			SysMntLogoDataReg();		// logo�o�^�̂�(�\���Ȃ�)
		}
		if (ftpctrl.etc_update & __azuftr_bit) {
			AzuFtr_Rsts	= FLT_ReadAzuFtr( &AzuFtr_Data[0][0] );	// �ׯ����؂������������ر�֓Ǎ�
		}
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
		if (ftpctrl.etc_update & __crekbrftr_bit) {
			CreKbrFtr_Rsts	= FLT_ReadCreKbrFtr( &CreKbrFtr_Data[0][0] );	// �ׯ����؂������������ر�֓Ǎ�
		}
		if (ftpctrl.etc_update & __epaykbrftr_bit) {
			EpayKbrFtr_Rsts	= FLT_ReadEpayKbrFtr( &EpayKbrFtr_Data[0][0] );	// �ׯ����؂������������ر�֓Ǎ�
		}
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
		if (ftpctrl.etc_update & __futureftr_bit) {
			FutureFtr_Rsts	= FLT_ReadFutureFtr( &FutureFtr_Data[0][0] );	// �ׯ����؂������������ر�֓Ǎ�
		}
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		if (ftpctrl.etc_update & __emgftr_bit) {
			EmgFooter_Rsts	= FLT_ReadEmgFooter( &EmgFooter_Data[0][0] );	// �ׯ����؂������������ر�֓Ǎ�
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	}
	// ���ۂɖ炷���ǂ����̓^�C�}�[�N����ɔ���
	Ope_StartDoorKnobTimer();
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_timer(timer supervision of response)
//[]----------------------------------------------------------------------[]
///	@return			timeout command code
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int		ftp_timer(void)
{
	if (ftpctrl.watch_timer > 0) {
		if (--ftpctrl.watch_timer <= 0) {
			return 0;
		}
	}
	return -1;
}


//[]----------------------------------------------------------------------[]
///	@brief			SysMnt_FTPServer(FTP server main)
//[]----------------------------------------------------------------------[]
///	@return			event type
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
ushort	SysMnt_FTPServer(void)
{
	ushort	msg, event;
	int		ksg_ret;
	uchar	timeoutFlag = 0;

	dispclr();
	/* display title */
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[0]);				/* title */
	grachr(2, 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[1]);				/* My IP address */
	grachr(3, 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[2]);				/* My Port No. */
	grachr(4, 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[3]);				/* login user */
	sprintf((char*)tmpwork1, "%d.%d.%d.%d",
			(int)(CPrmSS[S_MDL][2]/1000), (int)(CPrmSS[S_MDL][2]%1000),
			(int)(CPrmSS[S_MDL][3]/1000), (int)(CPrmSS[S_MDL][3]%1000));
	grachr(2, 15, _STRLEN(tmpwork1), 0, COLOR_BLACK, LCD_BLINK_OFF, tmpwork1);
	sprintf((char*)tmpwork1, "%d", (int)CPrmSS[S_MDL][21]);
	grachr(3, 15, _STRLEN(tmpwork1), 0, COLOR_BLACK, LCD_BLINK_OFF, tmpwork1);
	grachr(4, 15, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);				/* �Ȃ� */	
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[5]);				/* service start */
	Fun_Dsp(FUNMSG[8]);						/* [8]	"�@                       �I�� " */

	ftp_init(_FTP_SERVER, OPETCBNO );

// MH810100(S) K.Onodera 2020/2/20 �Ԕԃ`�P�b�g���X�iFTP�j
	remotedl_script_typeset( MANUAL_SCRIPT );		// �X�N���v�g�t�@�C����ʃZ�b�g
// MH810100(E) K.Onodera 2020/2/20 �Ԕԃ`�P�b�g���X�iFTP�j

	Lagtim(OPETCBNO, 7, FTP_TIMEOUT);	// ��ϰ�Ď�����(1s)
	/* display function */

	ksg_ret = tfFtpdUserStart(FILESYS,
							MAX_CONNECTIONS,
							MAX_BACKLOG,
							IDLE_TIMEOUT,
							BLOCKING_STATUS	);
							
	if( ksg_ret != TM_ENOERROR ){
		BUZPIPI();
		return MOD_EXT;
	}
	INTERVAL_TIMER_START( TIMEOUT25 );			// �C���^�[�o���^�C�}�[�X�^�[�g

	FAIL_SAFE_TIMER_START( GetTaskID(), GetFailSafeTimerNo() );
	for (event = 0; ftpctrl.start; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			sysment2_lcd_disconnect();
			if (SysMnt2ModeLcdDisconnect) {
				if (ftpctrl.login < 0) {
					tfFtpdUserStop();
					event = MOD_CUT;
					ftpctrl.start = FTP_IDLE;
				}
			}
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			sysment2_change_mode();
			if (SysMnt2ModeChange) {
				if (ftpctrl.login < 0) {
					tfFtpdUserStop();
					event = MOD_CHG;
					ftpctrl.start = FTP_IDLE;
				}
			}
			break;
		case KEY_TEN_F5:		// �u�I���v
			if (ftpctrl.login < 0 && event == 0) {
				BUZPI();
				tfFtpdUserStop();
				event = MOD_EXT;
				ftpctrl.start = FTP_IDLE;
			}
			break;
		case TIMEOUT7:
			if (ftp_timer() != -1) {
				if( ftpctrl.login > 0 ){
					// �^�C���A�E�g���̏������s���B
					tfFtpdUserStop();
					grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[6]);				/* "�@�@�@�����I�����܂��@�@�@�@�@" */
					ftpctrl.start = FTP_IDLE;
					break;
				}
			}
			Lagtim(OPETCBNO, 7, FTP_TIMEOUT);	// ��ϰ7(1s)�N��
			break;

		case TIMEOUT25:									// �C���^�[�o���^�C�}�[
			FTP_Execute();
			INTERVAL_TIMER_START( TIMEOUT25 );			// �C���^�[�o���^�C�}�[�X�^�[�g
			break;
		case FTP_LOGIN:
			BUZPI();
			displclr(6);
			ftpctrl.login = 1;
			GetFTPLoginPass( (char)PC_LONGIN, ftpctrl.loginbuff[0], ftpctrl.loginbuff[1] );
			grachr(4, 15, _STRLEN(ftpctrl.loginbuff), 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)ftpctrl.loginbuff);
			Fun_Dsp(FUNMSG[0]);
			timeoutFlag = 0;
			break;
		case FTP_FORCE_LOGOUT:
			tfFtpdUserStop();
		case FTP_LOGOUT:
			ftpctrl.login = -1;
			displclr(4);
			grachr(4, 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[3]);
			grachr(4, 15, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);
			if( ftpdata.write.error ){
				BUZPIPI();
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[(ftpdata.write.error == 1) ? 47 : 46]);
			}
			else if(timeoutFlag != 0) {						// �^�C���A�E�g�ɂ�郍�O�A�E�g
				;											// BUZPIPI�ς݁A�G���[�\���ς݂̂��߉������Ȃ�
			}else{
				BUZPI();
				displclr(6);
			}
			Fun_Dsp(FUNMSG[8]);
// MH810100(S) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iLCD�f�t�H���g�N�����[�h�Ή��j
//			if( read_rotsw() == 5 ){						// �����C���X�g�[�����[�h�ł́A���O�A�E�g�Ɠ����ɏI��������
//				tfFtpdUserStop();							// �I���L�[�������̓�����s�Ȃ��AFTP�������I��������
//				event = MOD_EXT;
//				ftpctrl.start = FTP_IDLE;
//			}
// MH810100(E) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X�iLCD�f�t�H���g�N�����[�h�Ή��j
			break;

		case TIMEOUT19:								// �ʐM�t�F�[���Z�[�t�^�C�}�[
			if( ftpctrl.login != 1 ){				// ���O�C�����ĂȂ�
				break;
			}
			tfFtpdUserStop();
			
		case FTP_SERVER_TIMEOUT:
			BUZPIPI();
			ftpctrl.login = -1;
			displclr(4);
			grachr(4, 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[3]);
			grachr(4, 15, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_6[9]);
			if(msg == FTP_SERVER_TIMEOUT) {
				grachr(6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[43]);		/* " �R�l�N�V�������ؒf����܂��� " */
			}
			else {
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[56]);		// "�T�[�r�X���I�����܂����@�@�@�@"
			}
			timeoutFlag = 1;
			Fun_Dsp(FUNMSG[8]);
			break;

		case FTP_SENDING:
		case FTP_RECVING:
		case FTP_NOTFILE_R:
		case FTP_NOTFILE_W:
			if( msg == FTP_NOTFILE_R || msg == FTP_NOTFILE_W ){
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[45]);			// "�������݌�������܂���@�@�@�@"
				BUZPIPI();
			}else{
				ftp_disp_file_title((uchar)(msg-FTP_SENDING));
			}
			break;

		default:
			break;
		}
	}

	Lagcan(OPETCBNO, 7);
	Lagcan(OPETCBNO, GetFailSafeTimerNo());
	Lagcan(OPETCBNO, 25);
	if (OPECTL.Mnt_mod) {										// �����e�i���X��
		if(!Ope_IsEnableDoorKnobChime()) {
			Ope_EnableDoorKnobChime();
		}
	}

	tfFtpdUserStop();
	ftp_exit();
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
	if (SysMnt2ModeLcdDisconnect){
		event = MOD_CUT;
	}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
	if (SysMnt2ModeChange){
		event = MOD_CHG;
	}
	return event;
}

//[]----------------------------------------------------------------------[]
///	@brief			SysMnt_FTPClient(FTP client main)
//[]----------------------------------------------------------------------[]
///	@return			event type
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
ushort	SysMnt_FTPClient(void)
{
	ushort	msg;
	ushort	ret;
	char	namebuff[2][20];
	uchar	key_prohibit = 0;
	uchar	wait_reboot = 0;
	char	script;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,SMSTR3[29]);						/* ���e�s�o���O�C���� */
	ret = 0;

//	grachr(1, 0, 12, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[30]);						/* user */
//	opedsp(1, 8, no, 1, 0, 1, COLOR_BLACK, LCD_BLINK_OFF);								/* No. */
	grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);				/* "�@�@���΂炭���҂��������@�@�@" */
	Fun_Dsp(FUNMSG[0]);																	/* [0]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */

	if( remotedl_script_typeget() == PARAMTER_SCRIPT ){
		script = REMOTE_LONGIN;
	}else{
		script = PC_LONGIN;
	}
	GetFTPLoginPass( script, namebuff[0], namebuff[1] );
//	grachr(1, 12, _STRLEN(namebuff[0]), 1, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)namebuff[0]);			/* name */

	ftp_init(_FTP_CLIENT, OPETCBNO );

	memcpy( ftpctrl.loginbuff, namebuff, sizeof( ftpctrl.loginbuff ));

	memset(&sw_update, 0, sizeof(sw_update));
	au_init();

	Lagtim(OPETCBNO, 7, FTP_TIMEOUT);			// ��ϰ�Ď�����(1s)
	
	ftp_new_session();
	if( ftpctrl.start != FTP_NEW_SESSION ){
		BUZPIPI();
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[34]);							/* "�@�@���O�C���Ɏ��s���܂����@�@" */
		Fun_Dsp(FUNMSG[8]);																		/* "�@                       �I�� " */
		while (1){
			msg = StoF( GetMessage(), 1 );
			switch (msg) {
				case KEY_TEN_F5:	/* �I��(F5) */
					BUZPI();
					return MOD_EXT;
			}
		}
	}
	ftp_connection();
	if( ftpctrl.start != FTP_CONNECT ){
		BUZPIPI();
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[34]);							/* "�@�@���O�C���Ɏ��s���܂����@�@" */
		Fun_Dsp(FUNMSG[8]);																		/* "�@                       �I�� " */
		while (1){
			msg = StoF( GetMessage(), 1 );
			switch (msg) {
				case KEY_TEN_F5:	/* �I��(F5) */
					BUZPI();
					return MOD_EXT;
			}
		}
	}
	FAIL_SAFE_TIMER_START( GetTaskID(), GetFailSafeTimerNo() );
	INTERVAL_TIMER_START( TIMEOUT25 );			// �C���^�[�o���^�C�}�[�X�^�[�g

	while (ftpctrl.start){
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
			case TIMEOUT7:
				if (ftp_timer() != -1) {
					if( ftpctrl.start != FTP_STOP ){
						// �^�C���A�E�g���̏������s���B
						if( ftpctrl.result == AU_ABORT_END ){
							ftp_send_cancel(0);
						}else{
							ftp_send_quit(0);
						}
					}
				}
				Lagtim(OPETCBNO, 7, FTP_TIMEOUT);	// ��ϰ�Ď�����(1s)
				break;

			case TIMEOUT25:									// �C���^�[�o���^�C�}�[
				FTP_Execute();
				INTERVAL_TIMER_START( TIMEOUT25 );			// �C���^�[�o���^�C�}�[�X�^�[�g
				break;
			
			case TIMEOUT19:
				ftp_send_quit(0);
				break;

			case KEY_TEN_F3:
				if (key_prohibit == 1) {						// "������"��[ftpctrl.au_sts]���قȂ�̂Ŏ�t�Ȃ�
					break;
				}
				if (ftpctrl.au_sts == AU_CONFIRM) {				/* �͂�(F3) */
					ftpctrl.au_sts = AU_EXEC;
					BUZPI();
					if( !ftpctrl.result || ftpctrl.result == TM_EACCES ){
						grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[41]);			/* "�@ �A�b�v�f�[�g�����s���ł� �@" */
						displclr(3);
						grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	/* "�@�@ ���΂炭���҂��������@   " */
						displclr(5);
						Fun_Dsp(FUNMSG[82]);						/* "�@�@�@�@�@�@ ���~ �@�@�@�@�@�@" */
						_SET_WATCH_TIMER();
					}else{
						_SET_TIMEOUT();
					}
					Lagtim(OPETCBNO, 7, FTP_TIMEOUT);	// ��ϰ�Ď�����(1s)
				}
				else if (ftpctrl.au_sts == AU_EXEC) {			/* ���~(F3) */
					BUZPI();
					displclr(2);
					Fun_Dsp(FUNMSG[0]);							/* "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
					key_prohibit = 1;							// "���~"��̃L�[��t�֎~
					displclr(6);								// �t�@�C�����폜
					ftpdata.write.error |= 0x80;
					_SET_WATCH_TIMER();
				}
				break;
			case KEY_TEN_F4:				/* ������(F4) */
				if (ftpctrl.au_sts == AU_CONFIRM) {
					ftp_send_quit( 0 );
					ftpctrl.result = AU_CANCEL;
					_SET_WATCH_TIMER();
				}
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
			case LCD_DISCONNECT:
				sysment2_lcd_disconnect();
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
			case KEY_MODECHG:
				sysment2_change_mode();
				break;
		}
	}

	Lagcan(OPETCBNO, 7);
	Lagcan(OPETCBNO, GetFailSafeTimerNo());
	Lagcan(OPETCBNO, 25);

	ftp_exit();
	if (OPECTL.Mnt_mod) {										// �����e�i���X��
		if(!Ope_IsEnableDoorKnobChime()) {
			Ope_EnableDoorKnobChime();
		}
	}
	Fun_Dsp(FUNMSG[8]);								/* "�@                       �I�� " */
	dispmlclr(2,6);
	
	switch (ftpctrl.result) {
	case AU_NORMAL_END:
		BUZPI();
		
		au_get_change((char*)&sw_update.main);
		if (sw_update.main == 0 && sw_update.param == 0 && sw_update.wave == 0 ) {
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[42]);		/* "�@�A�b�v�f�[�g���I�����܂����@" */
		}
		else {
			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[42]);		/* "�@�A�b�v�f�[�g���I�����܂����@" */
			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[24]);		/* "�V�X�e���ɔ��f���邽�߂ɂ́@�@" */
			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[25]);		/* "���u�ċN�����K�v�ł��@�@�@�@�@" */
			grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[26]);		/* "�ċN�����Ă���낵���ł����H�@" */
			Fun_Dsp(FUNMSG[43]);												/* "�@�@�@�@�@�@ �͂�  �@�@�@�@�@ " */
			wait_reboot = 1;
		}
		break;
	case AU_NO_UPDATE:
		BUZPIPI();
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[40]);			/* "�A�b�v�f�[�g�̕K�v�͂���܂���" */
		break;
	case AU_CANCEL:
		BUZPI();
	case AU_ABORT_END:
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[44]);			/* "�@�A�b�v�f�[�g�𒆎~���܂����@" */
		break;
	case AU_DISCONNECT:
		BUZPIPI();
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[43]);			/* " �R�l�N�V�������ؒf����܂��� " */
		break;
	case AU_LOGIN_ERR:
		BUZPIPI();
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[34]);			/* "�@�@���O�C���Ɏ��s���܂����@�@" */
		break;
	case SCRIPT_FILE_NONE:
		BUZPIPI();
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[(!au_script_info_get()?13:15)]);	/* "�X�N���v�g�t�@�C��������܂���" */
		break;																							/* "     ���M�Ɏ��s���Ă��܂�     " */
	case SCRIPT_FILE_ERR:
		BUZPIPI();
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[14]);		/* "   �X�N���v�g���e���s���ł�   " */
		break;
	case FLASH_WRITE_ERR:
		BUZPIPI();
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[18]);		/* "FLASH�̏����݂Ɏ��s���Ă��܂� " */
		break;
	default:
		break;
	}

	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
		case LCD_DISCONNECT:
			sysment2_lcd_disconnect();
			if( wait_reboot ){
				break;
			}
			if (sw_update.main == 0 && sw_update.param == 0 && sw_update.wave == 0 ) {
				if ( SysMnt2ModeLcdDisconnect )
					return MOD_CUT;
			}
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X)
		case KEY_MODECHG:
			sysment2_change_mode();
			if( wait_reboot ){
				break;
			}
			if (sw_update.main == 0 && sw_update.param == 0 && sw_update.wave == 0 ) {
				if (SysMnt2ModeChange)
					return MOD_CHG;
			}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			if (sw_update.main == 0 && sw_update.param == 0 && sw_update.wave == 0 ) {
				if (SysMnt2ModeLcdDisconnect)
					return MOD_CUT;
			}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		case KEY_TEN_F5:	// �I��
			if( wait_reboot ){
				break;
			}
			if (sw_update.main == 0 && sw_update.param == 0 && sw_update.wave == 0) {
				BUZPI();
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
				if (SysMnt2ModeLcdDisconnect)
					return MOD_CUT;
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
				if (SysMnt2ModeChange)
					return MOD_CHG;
				return MOD_EXT;
			}
			break;
		case KEY_TEN_F3:	// �͂�
			if (sw_update.main == 1 || sw_update.param == 1 || sw_update.wave == 1 ) {
				BUZPI();
				// �ʐ؂�ւ����s
				if ( sw_update_proc() != 1 ) {
					BUZPIPI();
					dispclr();
					grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[35]);		/* "���I�[�g�A�b�v�f�[�g���@�@�@�@" */
					grachr(3, 4, 22, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[33]);		/* "�������݂Ɏ��s���܂����@�@�@�@" */
				}
				System_reset();		// Main CPU reset (It will not retrun from this function)
			}
			break;
		default:
			break;
		}
	}
	
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			sw_update_proc(update process)
//[]----------------------------------------------------------------------[]
///	@return			1/0		:	OK(1)/NG(0)
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ulong	sw_update_proc(void)
{
	uchar update_flag;
	update_flag = 0;
	if (sw_update.main) {// �v���O�����̍X�V�v���L
		update_flag |= PROGRAM_UPDATE_FLAG;
	}
	if (sw_update.wave) {// �����̍X�V�v���L
		update_flag |= WAVEDATA_UPDATE_FLAG;
	}
	if (sw_update.param) {// ���ʃp�����[�^�̍X�V�v���L
		update_flag |= PARAM_UPDATE_FLAG;
	}
	update_flag_set(update_flag);
	return (ulong)1;
}
//[]----------------------------------------------------------------------[]
///	@brief			sysmnt2_update_swdata(update switch data)
//[]----------------------------------------------------------------------[]
///	@return			event type
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ushort	sysmnt2_update_swdata(void)
{
	ushort	msg;
	uchar	flag;
	const uchar	*p;
	int i;
	
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[7] );		/* "���V�X�e���ؑց��@�@�@�@�@�@�@" */
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[17] );		/* "���C���v���O�����F�@�@�@�@�@�@" */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
//	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[18] );		// "�����f�[�^�@�@�@�F�@�@�@�@�@�@" [18]
//	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[19] );		/* "���ʃp�����[�^�[�F�@�@�@�@�@�@" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[19] );		/* "���ʃp�����[�^�[�F�@�@�@�@�@�@" */
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[20] );		// "  �V�X�e���̍X�V�����{���܂�  " [20]
	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[21] );		/* "�@�@�@ ��낵���ł����H �@�@�@" */
	Fun_Dsp(FUNMSG[19]);					/* "�@�@�@�@�@�@ �͂� �������@�@�@" */

	if (sw_update.main) {// �X�V�L
		p = ver_datwk_prog;// �X�V���鎞�͊��Ɏ擾���Ă���̂Ń��[�h���Ȃ�
		i = VER_SIZE;
	} else {
		p = VERSNO.ver_part;
		i = sizeof(VERSNO.ver_part)+sizeof(VERSNO.ver_romn);
	}
	grachr(1, 18, i, 0, COLOR_BLACK, LCD_BLINK_OFF, p);	

// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
//	if (sw_update.wave) {// �X�V�L
//		p = ver_datwk_wave_sus;// �X�V���鎞�͊��Ɏ擾���Ă���̂Ń��[�h���Ȃ�
//	} else {
//		chk_wave_data_onfrom(BootInfo.sw.wave,ver_datwk_wave_ope);// �^�p�ʂ̃o�[�W�������擾����
//		p = ver_datwk_wave_ope;
//	}
//	grachr(2, 18, VER_SIZE, 0, COLOR_BLACK, LCD_BLINK_OFF, p);//�������ł�	
//
//	if (sw_update.param) {// �X�V�L
//		grachr(3, 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[21]);		/* "�ύX����@�@�@�@�@�@�@�@�@�@�@" */
//	} else {
//		grachr(3, 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[22]);		/* "�ύX�Ȃ��@�@�@�@�@�@�@�@�@�@�@" */
//	}
	if (sw_update.param) {// �X�V�L
		grachr(2, 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[21]);		/* "�ύX����@�@�@�@�@�@�@�@�@�@�@" */
	} else {
		grachr(2, 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[22]);		/* "�ύX�Ȃ��@�@�@�@�@�@�@�@�@�@�@" */
	}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
	flag = 1;
	while (flag) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
		case KEY_MODECHG:
			return MOD_CHG;
		case KEY_TEN_F3:		// �͂�
			if (flag == 1) {
				BUZPI();
				// �ʐؑ֏��̏�������
				if (sw_update_proc()) {
					// �X�V�҂��Ŏ蓮�ɂĐ؊����s���ꍇ�A�v�����N���A����
// GG120600(S) // Phase9 #6185 �ݒ�A�b�v���[�h���s�������ɃV�X�e���ؑւŋ��ʃp�����[�^�ύX����ƁA�u [�ݒ�A�b�v���[�h�@�\]-[�����J�n���茋��]-[����J�n](7000100)�v����������
//					if( remotedl_status_get() == R_SW_WAIT ) {
//// GG120600(S) // Phase9 ��ʖ����`�F�b�N
////						remotedl_status_set(R_DL_IDLE);
//// GG120600(E) // Phase9 ��ʖ����`�F�b�N
//						switch (remotedl_connect_type_get()) {
//						case CTRL_PROG_DL:
//							if (sw_update.main == 1) {
//								remotedl_complete_request(REQ_KIND_VER_UP);
//							}
//							break;
//						case CTRL_PARAM_DL:
//						
//							if (sw_update.param == 1) {
//								remotedl_complete_request(REQ_KIND_PARAM_CHG);
//							}
//							break;
//// MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X�i���u�_�E�����[�h�j
//						case CTRL_PARAM_DIF_DL:		// �����ݒ�_�E�����[�h
//							if (sw_update.param == 1) {
//								remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG); // ���u�����e�i���X�v������
//							}
//							break;
//// MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X�i���u�_�E�����[�h�j
//						case CTRL_PROG_SW:
//							if (sw_update.main == 1) {
//								remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
//							}
//							break;
//						default:
//							break;
//						}
//					}
					mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 #6185 �ݒ�A�b�v���[�h���s�������ɃV�X�e���ؑւŋ��ʃp�����[�^�ύX����ƁA�u [�ݒ�A�b�v���[�h�@�\]-[�����J�n���茋��]-[����J�n](7000100)�v����������
					memset(&sw_update, 0, sizeof(sw_update));
					flag = 0;
				} else {
					BUZPIPI();
					dispclr();
					grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[7] );		/* "���V�X�e���ؑց��@�@�@�@�@�@�@" */
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[55] );		// "�@�V�X�e���X�V�Ɏ��s���܂����@" [55]
					Fun_Dsp(FUNMSG[8]);						/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
					if (SysMnt2ModeLcdDisconnect) {
						return MOD_CUT;
					}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X�i�����e�i���X�j
					if (SysMnt2ModeChange) {
						return MOD_CHG;
					}
					flag = 2;
				}
			}
			break;
		case KEY_TEN_F4:		// ������
			if (flag == 1) {
				BUZPI();
				return MOD_EXT;
			}
			break;
		case KEY_TEN_F5:		// �I��
			if (flag == 2) {
				BUZPI();
				return MOD_EXT;
			}
			break;
		default:
			break;
		}
	}
// system reboot ------------------
	displclr(1);
	displclr(2);
	displclr(3);
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[23] );		// " �V�X�e���̍X�V�����{���܂��� " // [23]
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[24] );		/* "�V�X�e���ɔ��f���邽�߂ɂ́@�@" */
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[25] );		/* "���u�ċN�����K�v�ł��@�@�@�@�@" */
	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[26] );		/* "�ċN�����Ă���낵���ł����H�@" */
	Fun_Dsp(FUNMSG[43]);					/* "�@�@�@�@�@�@ �͂�  �@�@�@�@�@ " */

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
		case KEY_TEN_F3:		// �͂�
			BUZPI();
			for(i = 0;i<50;i++){
				// CAN�Ƀf�[�^��n�������Ȃ�܂ő҂�
				taskchg( IDLETSKNO );	/* Change task to idletask		*/
			}
			// �V�X�e���ċN��
			System_reset();			// Main CPU reset (It will not retrun from this function)
		default:
			break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			sw_switch_select(select switching (active or stand-by))
//[]----------------------------------------------------------------------[]
///	@param[in]		sw_update	: switch info(in/out)
///	@param[in]		act_flag	: active exist=1
///	@param[in]		stby_flag	: stand-by exist=1
///	@param[in]		p_mode		: common para=0, main=1, sub=2
///	@return			event type
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ushort	sw_switch_select(uchar *sw_update, uchar act_flag, uchar stby_flag, uchar p_mode)
{
	ushort	msg;

	if (stby_flag) {
		if (*sw_update) {
			if (p_mode == 1) {
				grachr( 5 , 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[52] );	//"  ��۸��т̍X�V���������܂�   " [52]
			} else if (p_mode == 2){
				grachr( 5 , 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[53] );	//" �����f�[�^�̍X�V���������܂� " [53]
			}else{// p_mode == 0
				grachr( 3 , 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[54] );	//"�p�����[�^�[�̍X�V���������܂�" [54]
			}
		}
		else {
			if (p_mode == 1) {
				grachr( 5 , 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[49] );	//"��۸��т̍X�V��\�񂵂܂�     " [49]
			} else if (p_mode == 2){
				grachr( 5 , 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[50] );	//"�����f�[�^�̍X�V��\�񂵂܂�  " [50]
			}else{// p_mode == 0
				grachr( 3 , 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[51] );	//"�p�����[�^�[�̍X�V��\�񂵂܂�" [51]
			}
		}
		if(p_mode == 0){
			grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[21] );		/* "�@�@�@ ��낵���ł����H �@�@�@" */
		} else {
			grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR1[21] );		/* "�@�@�@ ��낵���ł����H �@�@�@" */
		}
		Fun_Dsp(FUNMSG[19]);									/* "�@�@�@�@�@�@ �͂� �������@�@�@" */
	}
	else {
		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[16] );					/* "�@ �L���ȃf�[�^������܂���@ " */
		Fun_Dsp(FUNMSG[8]);										/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
	}

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CHG;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			return MOD_CHG;
		case KEY_TEN_F3:		// �͂�
			if (stby_flag) {
				BUZPI();
				(*sw_update) ^= 1;
				if (*sw_update) {
					switch (p_mode) {
					case 0:
						wopelg2(OPLOG_CPARAM_RESERVE, 0, 0);	// ���ʃp�����[�^�\��
						break;
					case 1:
						wopelg2(OPLOG_MAINPRG_RESERVE, 0, 0);	// ���C���v���O�����\��
						break;
					case 2:
						wopelg2(OPLOG_WAVEDATA_RESERVE, 0 , 0);	// �����f�[�^�\��
						break;
					default:
						break;
					}
				}
				return MOD_EXT;
			}
			break;
		case KEY_TEN_F4:		// ������
			if (stby_flag) {
				BUZPI();
				return MOD_EXT;
			}
			break;
		case KEY_TEN_F5:		// �I��
			if (stby_flag == 0) {
				BUZPI();
				return MOD_EXT;
			}
			break;
		default:
			break;
		}
	}

}
//[]----------------------------------------------------------------------[]
///	@brief			IsMainProgExist
//[]----------------------------------------------------------------------[]
///	@return			�X�V����v���O�����L=1,��=0
///	@attention		None
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/14<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
ushort	IsMainProgExist(void)
{
	memset(ver_datwk_prog,0,sizeof(ver_datwk_prog));
	if( 0 == chk_prog_data_onfrom(ver_datwk_prog)) {
		return((ushort)1);// �f�[�^�L
	}
	return((ushort)0);// �f�[�^��
}
//[]----------------------------------------------------------------------[]
///	@brief			IsVoiceDataExist
//[]----------------------------------------------------------------------[]
///	@return			�X�V���鉹���f�[�^�L=1,��=0
///	@attention		None
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/04/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ushort	IsVoiceDataExist(void)
{
	memset(ver_datwk_wave_sus,0,sizeof(ver_datwk_wave_sus));
	if(BootInfo.sw.wave == OPERATE_AREA1) {
		// ���ݖ�1�œ��삵�Ă���ꍇ��2�Ƀf�[�^�����邩
		if( 0 == chk_wave_data_onfrom(OPERATE_AREA2,ver_datwk_wave_sus)) {
			return((ushort)1);// �f�[�^�L
		}
	} else {
		// ���ݖ�2�œ��삵�Ă���ꍇ��1�Ƀf�[�^�����邩
		if( 0 == chk_wave_data_onfrom(OPERATE_AREA1,ver_datwk_wave_sus)) {
			return((ushort)1);// �f�[�^�L
		}
	}
	return((ushort)0);// �f�[�^��
}
//[]----------------------------------------------------------------------[]
///	@brief			sysmnt2_switch_main(switch main program (active or stand-by))
//[]----------------------------------------------------------------------[]
///	@return			event type
///	@attention		None
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/04/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ushort	sysmnt2_switch_main(void)
{
	const uchar	*p;
	uchar	ver_str[VER_SIZE*2];
	int		main_size;
	
	memset( ver_str, 0x20, sizeof(ver_str));

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[8] );		/* "�����C���v���O�������@�@�@�@�@" */
	if(IsMainProgExist() != 0){
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[10] );	//"�X�V�O�F�����������������@�@�@" // [10]
		grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[11] );	//"�X�V��F�����������������@�@�@" // [11]
		// ����v���O����(�X�V�O)
		p = VERSNO.ver_part;
		main_size = sizeof(VERSNO.ver_part)+sizeof(VERSNO.ver_romn);
		as1chg(p, ver_str, main_size);
		grachr(2, 10, (main_size*2), 0, COLOR_BLACK, LCD_BLINK_OFF, ver_str);

		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[48] );	//"�@�@�@�@�@�@�@���@�@�@�@�@�@�@", // [48]

		// �X�V�v���O����(�X�V��)
		p = ver_datwk_prog;
		memset( ver_str, 0x20, sizeof(ver_str));
		as1chg(p, ver_str, VER_SIZE);
		grachr(4, 10, (VER_SIZE*2), 0, COLOR_BLACK, LCD_BLINK_OFF, ver_str);
		return sw_switch_select(&sw_update.main, 1, 1, 1);
	}else{
		return sw_switch_select(&sw_update.main, 0, 0, 1);// �L���f�[�^�Ȃ�
	}

}
//[]----------------------------------------------------------------------[]
///	@brief			sysmnt2_switch_sub(switch sub program (active or stand-by))
//[]----------------------------------------------------------------------[]
///	@return			event type
///	@attention		None
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ushort	sysmnt2_switch_sub(void)
{
	const uchar	*p;
	uchar	ver_str[VER_SIZE*2];
	
	memset( ver_str, 0x20, sizeof(ver_str));

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[9] );		//"�������f�[�^���@�@�@�@�@�@�@�@", // [09]
	if(IsVoiceDataExist() != 0){
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[10] );	//"�X�V�O�F�����������������@�@�@" // [10]
		grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[11] );	//"�X�V��F�����������������@�@�@" // [11]

		// ���쉹��(�X�V�O)
		chk_wave_data_onfrom(BootInfo.sw.wave,ver_datwk_wave_ope);// �^�p�ʂ̃o�[�W�������擾����
		p = ver_datwk_wave_ope;

		as1chg(p, ver_str, VER_SIZE);
		grachr(2, 10, VER_SIZE*2, 0, COLOR_BLACK, LCD_BLINK_OFF, ver_str);

		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[48] );//"�@�@�@�@�@�@�@���@�@�@�@�@�@�@", // [48]

		// �X�V����(�X�V��)
		p = ver_datwk_wave_sus;
		memset( ver_str, 0x20, sizeof(ver_str));
		as1chg(p, ver_str, VER_SIZE);
		grachr(4, 10, VER_SIZE*2, 0, COLOR_BLACK, LCD_BLINK_OFF, ver_str);
		return sw_switch_select(&sw_update.wave, 1, 1, 2);
	}else{
		return sw_switch_select(&sw_update.wave, 0, 0, 2);// �L���f�[�^�Ȃ�
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			sysmnt2_switch_param(switch common parameter (active or stand-by))
//[]----------------------------------------------------------------------[]
///	@return			event type
///	@attention		None
///	@author			Y.FUJIWARA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	ushort	sysmnt2_switch_param(void)
{
	uchar	stby_flag = 0;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[14] );		/* "�����ʃp�����[�^�[���@�@�@�@�@" */

	stby_flag = (uchar)ftp_check_stby_param();

	if(stby_flag != 0){// �L���f�[�^�L
		return sw_switch_select(&sw_update.param, 1, stby_flag, 0);
	}else{
		return sw_switch_select(&sw_update.param, 0, stby_flag, 0);
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			SysMnt_SystemSwitch(switch system (active or stand-by))
//[]----------------------------------------------------------------------[]
///	@return			event type
///	@attention		None
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/04/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
ushort	SysMnt_SystemSwitch(void)
{
	ushort	usSysEvent;
	char	wk[2];

	DP_CP[0] = DP_CP[1] = 0;

	memset(&sw_update, 0, sizeof(sw_update));
	_sysmnt2_start_sys_switch();

	for ( ; ; ) {
		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  SMSTR3[7] );		/* "���V�X�e���ؑց��@�@�@�@�@�@�@" */
		Fun_Dsp(FUNMSG[25]);					/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */
		usSysEvent = Menu_Slt(SYSSWMENU, SYS_SW_TBL, (char)SYS_SW_MAX, (char)1);

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usSysEvent) {
		/* 1.���C���v���O���� */
		case SYS_MAIN_PRO:
			usSysEvent = sysmnt2_switch_main();
			break;
// ���g�p(S)
		/* 2.�����f�[�^ */
		case SYS_SUB_PRO:
			usSysEvent = sysmnt2_switch_sub();
			break;
// ���g�p(E)
		/* 3.���ʃp�����[�^�[ */
		case SYS_COM_PRM:
			usSysEvent = sysmnt2_switch_param();
			break;
		case MOD_EXT:
			if (sw_update.main == 0 && sw_update.wave == 0 && sw_update.param == 0) {
				return MOD_EXT;
			} else {
				usSysEvent = sysmnt2_update_swdata();
			}
			break;
		default:
			break;
		}
		if ( usSysEvent == MOD_CHG ){
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( usSysEvent == MOD_CUT ) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_remote_auto_update
//[]----------------------------------------------------------------------[]
///	@return			None
///	@attention		None
///	@author			T.Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010/10/28<br>
///					Update	:	
/// @note			���̊֐��܂ވȍ~�̏����́Aremote_dlTask����R�[������܂�<br>
///					���������Ƃ̊֌W�iStatic�j�����܂肢���肽���Ȃ������̂ŁA�����ɒǉ����܂��B
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	ftp_remote_auto_update( void )
{
	ushort	msg;
	t_prog_chg_info *dl = remotedl_info_get();
	
	ftp_init(_FTP_CLIENT, REMOTEDLTCBNO );

	memset(&sw_update, 0, sizeof(sw_update));
	au_init();

	switch( remotedl_status_get() ){
		case	R_DL_EXEC:
			memset( &sw_update, 0, sizeof(sw_update));							// �ؑ֏����N���A
			remotedl_sw_update_bk( &sw_update.main );							// �o�b�N�A�b�v�����N���A
			break;
	}
	GetFTPLoginPass( REMOTE_LONGIN, ftpctrl.loginbuff[0], ftpctrl.loginbuff[1] );

	Lagtim(REMOTEDLTCBNO, 1, FTP_TIMEOUT);	// ��ϰ�Ď�����(1s)

// MH810100(S) K.Onodera 2019/12/13 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
#if 0
	// PKI�g�p�J�n
	ftps_pki_use();
	if( ftpctrl.start != FTPS_PKI_START ){
		//remotedl_monitor_info_set(1);		// TODO_Onodera�F�����Z�b�g����H
		Before_DL_Err_Function( LOGIN_ERR );
		return;
	}

	// ���[�g�ؖ����ǉ�
	ftps_add_rootCertificate();

#else
// MH810100(E) K.Onodera 2019/12/13 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
	// FTP�Z�b�V�����쐬
	ftp_new_session();
	if( ftpctrl.start != FTP_NEW_SESSION ){
		remotedl_monitor_info_set(1);
		Before_DL_Err_Function( LOGIN_ERR );
		return;
	}
// MH810100(S) K.Onodera 2019/12/13 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
#endif
// MH810100(E) K.Onodera 2019/12/13 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
	
	// FTP�ڑ�
	ftp_connect_remote();
	if( ftpctrl.start != FTP_CONNECT_REMOTE ){
		remotedl_monitor_info_set(2);
		Before_DL_Err_Function( LOGIN_ERR );
// GG120600(S) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
		if(ftpctrl.ftpHandle){
			tfFtpFreeSession(ftpctrl.ftpHandle );
			ftpctrl.ftpHandle = 0;
		}
// GG120600(E) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
		return;
	}

	FAIL_SAFE_TIMER_START( GetTaskID(), GetFailSafeTimerNo() );
	INTERVAL_TIMER_START( TIMEOUT5 );			// �C���^�[�o���^�C�}�[�X�^�[�g

	while (ftpctrl.start){
		msg = StoF( GetMessageRemote(), 1 );
		switch (msg) {
			case TIMEOUT1:
				if (ftp_timer() != -1) {
					if( ftpctrl.start != FTP_STOP ){
						// �^�C���A�E�g���̏������s���B
						if( ftpctrl.result == AU_ABORT_END ){
							ftp_send_cancel(0);
						}else{
							ftp_send_quit(0);
						}
						if( (ftpctrl.result&0x7FFF) == 0){
							remotedl_result_set( FOMA_COMM_ERR );
						}
					}
				}
				Lagtim(REMOTEDLTCBNO, 1, 50);	// ��ϰ�Ď�����(1s)
				break;

			case TIMEOUT4:								// �ʐM�t�F�[���Z�[�t�^�C�}�[
				// FTP�ʐM�^�C���A�E�g���ԃ`�F�b�N
				if (GetTaskID() == REMOTEDLTCBNO) {
					if (!remotedl_failsafe_timer_check()) {
						break;
					}
				}
				ftpctrl.result = AU_ABORT_END;			// �ڑ����~
				ftpdata.write.error = 5;
				remotedl_result_set( FOMA_COMM_ERR );	// ���ʏ���FOMA�ʐM�s�ǂƂ��Ċi�[
				break;

			case TIMEOUT5:									// �C���^�[�o���^�C�}�[�X�^�[�g
				FTP_Execute();
				INTERVAL_TIMER_START( TIMEOUT5 );			// �C���^�[�o���^�C�}�[�X�^�[�g
				break;
			
		}
	}
	
	Lagcan(REMOTEDLTCBNO, 1);
	Lagcan(REMOTEDLTCBNO, 4);									// �t�F�[���Z�[�t�^�C�}�[�L�����Z��
	Lagcan(REMOTEDLTCBNO, 5);									// �C���^�[�o���^�C�}�[�L�����Z��
	ftp_exit();

	// ���ʃ`�F�b�N
	ftp_remote_auto_update_end_check(dl);

}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_remote_auto_update_end_check(�I���`�F�b�N�j
//[]----------------------------------------------------------------------[]
///	@param[in]		dl		: t_prog_chg_info�ւ̃|�C���^
///	@return			�Ȃ�
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void 	ftp_remote_auto_update_end_check(t_prog_chg_info* dl)
{
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//	uchar	d_ret,connect,kind,status;
	uchar	d_ret,connect,kind;
// GG120600(E) // Phase9 ��ʖ����`�F�b�N

	connect = remotedl_connect_type_get();
	// �ڑ��`�F�b�N����
	if( connect == CTRL_CONNECT_CHK){
		d_ret = remotedl_result_get(RES_COMM);
		if( d_ret != EXCLUDED){
			switch(d_ret){
			case TEST_CHECK_COMP:
//				g_bk_ProgDlReq.ReqResult = REQ_CONN_OK;
//				// �����e�i���X�v���������M
//				NTNET_Snd_Data118_DL(&g_bk_ProgDlReq);
//				memset(&g_bk_ProgDlReq,0, sizeof(g_bk_ProgDlReq));
				if ((ftpctrl.result&0x7FFF) == AU_LOGOUT_ERR) {
					rmon_regist(RMON_FTP_LOGOUT_NG);					// ���O�A�E�gNG
				}
				else {
					rmon_regist(RMON_FTP_LOGOUT_OK);					// ���O�A�E�gOK
				}
				rmon_regist(RMON_FTP_TEST_RES_OK);						// FTP�ڑ��e�X�g����
				break;
			case LOGIN_ERR:
			case CONN_TIMEOUT_ERR:
			case TEST_UP_ERR:		// �e�X�g�t�@�C���A�b�v���[�h���s
			case TEST_DOWN_ERR:		// �e�X�g�t�@�C���_�E�����[�h���s
			case TEST_DEL_ERR:		// �e�X�g�t�@�C���폜���s
			default:
//				g_bk_ProgDlReq.ReqResult = REQ_CONN_NG;
//				// �����e�i���X�v���������M
//				NTNET_Snd_Data118_DL(&g_bk_ProgDlReq);
//				memset(&g_bk_ProgDlReq,0, sizeof(g_bk_ProgDlReq));
				switch(d_ret){
				case TEST_UP_ERR:		// �e�X�g�t�@�C���A�b�v���[�h���s
				case TEST_DOWN_ERR:		// �e�X�g�t�@�C���_�E�����[�h���s
				case TEST_DEL_ERR:		// �e�X�g�t�@�C���폜���s
					if ((ftpctrl.result&0x7FFF) == AU_LOGOUT_ERR ||		// ���O�A�E�g�G���[or
						d_ret == FOMA_COMM_ERR) {						// �ʐM�ُ�
						rmon_regist(RMON_FTP_LOGOUT_NG);				// ���O�A�E�gNG
					}
					else {
						rmon_regist(RMON_FTP_LOGOUT_OK);				// ���O�A�E�gOK
					}
					break;
				case LOGIN_ERR:
					rmon_regist(RMON_FTP_LOGIN_ID_PW_NG);				// ID or PW�s��
					break;
				case CONN_TIMEOUT_ERR:
				case FOMA_COMM_ERR:
					rmon_regist(RMON_FTP_LOGIN_TIMEOUT_NG);				// �ڑ��^�C���A�E�g
					break;
				}
				rmon_regist(RMON_FTP_TEST_RES_NG);						// FTP�ڑ��e�X�g���s
				break;
			}
		}
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//		remotedl_status_set(R_DL_IDLE);									// remotedl_task��~
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
		remotedl_complete_request(REQ_KIND_TEST);
		remotedl_chg_info_restore();									// ��񃊃X�g�A
		queset( OPETCBNO, REMOTE_DL_END, 0, NULL );

		return;	//�����ŏI��

	// �v���O�����_�E�����[�h�A�ݒ�_�E�����[�h
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//	}else if( connect == CTRL_PROG_DL || connect == CTRL_PARAM_DL){
	}else if( connect == CTRL_PROG_DL || connect == CTRL_PARAM_DL  || connect == CTRL_PARAM_DIF_DL){
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		// �_�E�����[�h����
// GG120600(S) // Phase9 ���ʂ���ʂ���
//		d_ret = remotedl_result_get(RES_DL);
		switch (connect) {
		case CTRL_PROG_DL:
			d_ret = remotedl_result_get(RES_DL_PROG);
			break;
		case CTRL_PARAM_DL:
			d_ret = remotedl_result_get(RES_DL_PARAM);
			break;
		case CTRL_PARAM_DIF_DL:
			d_ret = remotedl_result_get(RES_DL_PARAM_DIF);
			break;
		}
// GG120600(E) // Phase9 ���ʂ���ʂ���
		if( d_ret != PROG_DL_COMP &&									// ����I���ł͂Ȃ�
			d_ret != EXCLUDED ){										// �ΏۊO�ł��Ȃ�
			// ���g���C���
			if ((d_ret == LOGIN_ERR) || (d_ret == CONN_TIMEOUT_ERR)) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				kind = RETRY_KIND_CONNECT;
				switch (connect) {
				case CTRL_PROG_DL:
					kind = RETRY_KIND_CONNECT_PRG;
					break;
				case CTRL_PARAM_DL:
					kind = RETRY_KIND_CONNECT_PARAM_DL;
					break;
				case CTRL_PARAM_DIF_DL:
					kind = RETRY_KIND_CONNECT_PARAM_DL_DIFF;
					break;
				}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
			}
			else {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				kind = RETRY_KIND_DL;
				switch (connect) {
				case CTRL_PROG_DL:
					kind = RETRY_KIND_DL_PRG;
					break;
				case CTRL_PARAM_DL:
					kind = RETRY_KIND_DL_PARAM;
					break;
				case CTRL_PARAM_DIF_DL:
					kind = RETRY_KIND_DL_PARAM_DIFF;
					break;
				}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
			}
			// �G���[�̊Ď��f�[�^��o�^
			switch (connect) {
			case CTRL_PROG_DL:
				switch (d_ret) {
				case LOGIN_ERR:
					rmon_regist(RMON_FTP_LOGIN_ID_PW_NG);				// ID or PW�s��
					break;
				case CONN_TIMEOUT_ERR:
					rmon_regist(RMON_FTP_LOGIN_TIMEOUT_NG);				// �ڑ��^�C���A�E�g
					break;
				case FOMA_COMM_ERR:
					rmon_regist(RMON_PRG_DL_END_COMM_NG);				// �ʐM�ُ�
					break;
				case PROG_DL_ERR:
					rmon_regist(RMON_PRG_DL_END_FILE_NG);				// �v���O�����t�@�C���G���[
					break;
				case FLASH_WRITE_ERR:
					rmon_regist(RMON_PRG_DL_END_FLASH_WRITE_NG);		// Flash�����ݎ��s
					break;
				case PROG_DL_NONE:
					rmon_regist(RMON_PRG_DL_END_PROG_FILE_NONE);		// �v���O�����t�@�C���Ȃ�
					break;
				case PROG_SUM_ERR:
					rmon_regist(RMON_PRG_DL_END_SUM_NG);				// �v���O����SUM�l�G���[
					break;
				default:
					break;
				}
				break;
			case CTRL_PARAM_DL:
				switch (d_ret) {
				case LOGIN_ERR:
					rmon_regist(RMON_FTP_LOGIN_ID_PW_NG);				// ID or PW�s��
					break;
				case CONN_TIMEOUT_ERR:
					rmon_regist(RMON_FTP_LOGIN_TIMEOUT_NG);				// �ڑ��^�C���A�E�g
					break;
				case FOMA_COMM_ERR:
					rmon_regist(RMON_PRM_DL_END_COMM_NG);				// �ʐM�ُ�
					break;
				case PROG_DL_ERR:
					rmon_regist(RMON_PRM_DL_END_FILE_NG);				// �ݒ�t�@�C���G���[
					break;
				case FLASH_WRITE_ERR:
					rmon_regist(RMON_PRM_DL_END_FLASH_WRITE_NG);		// Flash�����ݎ��s
					break;
				case PROG_DL_NONE:
					rmon_regist(RMON_PRM_DL_END_PARAM_FILE_NONE);		// �ݒ�t�@�C���Ȃ�
					break;
				default:
					break;
				}
				break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			case CTRL_PARAM_DIF_DL:		// ���u�����e�i���X�v������
				switch (d_ret) {
				case LOGIN_ERR:
					rmon_regist(RMON_FTP_LOGIN_ID_PW_NG);				// ID or PW�s��
					break;
				case CONN_TIMEOUT_ERR:
					rmon_regist(RMON_FTP_LOGIN_TIMEOUT_NG);				// �ڑ��^�C���A�E�g
					break;
				case FOMA_COMM_ERR:
					rmon_regist(RMON_PRM_DL_DIF_END_COMM_NG);				// �ʐM�ُ�
					break;
				case PROG_DL_ERR:
					rmon_regist(RMON_PRM_DL_DIF_END_FILE_NG);				// �ݒ�t�@�C���G���[
					break;
				case FLASH_WRITE_ERR:
					rmon_regist(RMON_PRM_DL_DIF_END_FLASH_WRITE_NG);		// Flash�����ݎ��s
					break;
				case PROG_DL_NONE:
					rmon_regist(RMON_PRM_DL_DIF_END_PARAM_FILE_NONE);		// �ݒ�t�@�C���Ȃ�
					break;
				default:
					break;
				}
				break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)

			default:
				break;
			}
			if ((ftpctrl.result&0x7FFF) == AU_LOGOUT_ERR ||				// ���O�A�E�g�G���[or
				d_ret == FOMA_COMM_ERR) {								// �ʐM�ُ�
				rmon_regist(RMON_FTP_LOGOUT_NG);						// ���O�A�E�gNG
			}
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			else if (kind != RETRY_KIND_CONNECT) {
			else if (kind >= RETRY_KIND_CONNECT_MAX) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				rmon_regist(RMON_FTP_LOGOUT_OK);						// ���O�A�E�gOK
			}
			// �v���O����SUM�l�G���[�̏ꍇ�A���g���C�����ɏI������
			if (d_ret == PROG_SUM_ERR) {
// GG129000(S) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
				// �e��N���A�̑O��FROM�̃w�b�_�����폜���Ă���
				memset(&ftpdata.write.header, 0xFF, sizeof(ftpdata.write.header));
				FLT_write_program_version((uchar*)&ftpdata.write.header);
// GG129000(E) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]

				// ���g���C���N���A
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_DL_PRG);
				retry_info_clr(RETRY_KIND_CONNECT_PRG);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

				// �󂯕t�����v�����N���A����
				remotedl_complete_request(REQ_KIND_VER_UP);
				remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
// GG129000(S) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
// 				// FROM�̃w�b�_�����폜���Ă���
// 				memset(&ftpdata.write.header, 0xFF, sizeof(ftpdata.write.header));
// 				FLT_write_program_version( (uchar*)&ftpdata.write.header );
// GG129000(E) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
			}
			// �ڑ��E�_�E�����[�h���g���C
			else if (retry_info_set(kind)) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				// ���g���C���N���A
//				retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				switch (connect) {
				case CTRL_PROG_DL:
// GG129000(S) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
					// �e��N���A�̑O��FROM�̃w�b�_�����폜���Ă���
					memset(&ftpdata.write.header, 0xFF, sizeof(ftpdata.write.header));
					FLT_write_program_version((uchar*)&ftpdata.write.header);
// GG129000(E) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]

// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
					// ���g���C���N���A
					retry_info_clr(RETRY_KIND_CONNECT_PRG);
					retry_info_clr(RETRY_KIND_DL_PRG);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					if (kind == RETRY_KIND_CONNECT) {
					if (kind == RETRY_KIND_CONNECT_PRG) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
						rmon_regist(RMON_FTP_LOGIN_RETRY_OVER);			// ���g���C�I�[�o�[
					}
					else {
						rmon_regist(RMON_PRG_DL_END_RETRY_OVER);		// ���g���C�I�[�o�[
					}

					// ���g���C�I�[�o�[�Ȃ�󂯕t�����v�����N���A����
					remotedl_complete_request(REQ_KIND_VER_UP);
					remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
// GG129000(S) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
// 					// FROM�̃w�b�_�����폜���Ă���
// 					memset(&ftpdata.write.header, 0xFF, sizeof(ftpdata.write.header));
// 					FLT_write_program_version( (uchar*)&ftpdata.write.header );
// GG129000(E) R.Endo 2022/12/26 �Ԕԃ`�P�b�g���X4.0 #6766 �v���O�����_�E�����[�h�������g���C��̉��u�����e�i���X�v����E0011(���b�Z�[�W�L���[�t��)���� [���ʉ��P���� No1538]
					break;
				case CTRL_PARAM_DL:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
					// ���g���C���N���A
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
					retry_info_clr(RETRY_KIND_DL_PARAM);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					if (kind == RETRY_KIND_CONNECT) {
					if (kind == RETRY_KIND_CONNECT_PARAM_DL) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
						rmon_regist(RMON_FTP_LOGIN_RETRY_OVER);			// ���g���C�I�[�o�[
					}
					else {
						rmon_regist(RMON_PRM_DL_END_RETRY_OVER);		// ���g���C�I�[�o�[
					}

					// ���g���C�I�[�o�[�Ȃ�󂯕t�����v�����N���A����
					remotedl_complete_request(REQ_KIND_PARAM_CHG);
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				case CTRL_PARAM_DIF_DL:		// ���u�����e�i���X�v������
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
					// ���g���C���N���A
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
					retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					if (kind == RETRY_KIND_CONNECT) {
					if (kind == RETRY_KIND_CONNECT_PARAM_DL_DIFF) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
						rmon_regist(RMON_FTP_LOGIN_RETRY_OVER);			// ���g���C�I�[�o�[
					}
					else {
						rmon_regist(RMON_PRM_DL_DIF_END_RETRY_OVER);		// ���g���C�I�[�o�[
					}

					// ���g���C�I�[�o�[�Ȃ�󂯕t�����v�����N���A����
					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);	// ���u�����e�i���X�v������
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);	// ���u�����e�i���X�����X�e�[�^�X�ݒ�
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				default:
					break;
				}
			}
			else {
				// �ڑ����g���C�̏ꍇ�A�_�E�����[�h���g���C�񐔂��N���A����
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				if (kind == RETRY_KIND_CONNECT) {
//					retry_time_clr(RETRY_KIND_DL);
//				}
				if (kind < RETRY_KIND_CONNECT_MAX) {
					retry_time_clr(kind);
				}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
			}
		}else{
			if( d_ret != EXCLUDED ){									// �ΏۊO�ł͂Ȃ�
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				// ���g���C���N���A
//				retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				switch (connect) {
				case CTRL_PROG_DL:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
					// ���g���C���N���A
					retry_info_clr(RETRY_KIND_CONNECT_PRG);
					retry_info_clr(RETRY_KIND_DL_PRG);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					rmon_regist(RMON_PRG_DL_END_OK);					// �_�E�����[�h����
					if ((ftpctrl.result&0x7FFF) == AU_LOGOUT_ERR) {
						rmon_regist(RMON_FTP_LOGOUT_NG);				// ���O�A�E�gNG
					}
					else {
						rmon_regist(RMON_FTP_LOGOUT_OK);				// ���O�A�E�gOK
					}

					sw_update.main = 1;
					dl->sw_info[SW_PROG].exec = 1;
					remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_COMP);
					break;
				case CTRL_PARAM_DL:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
					// ���g���C���N���A
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
					retry_info_clr(RETRY_KIND_DL_PARAM);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					rmon_regist(RMON_PRM_DL_END_OK);					// �_�E�����[�h����
					if ((ftpctrl.result&0x7FFF) == AU_LOGOUT_ERR) {
						rmon_regist(RMON_FTP_LOGOUT_NG);				// ���O�A�E�gNG
					}
					else {
						rmon_regist(RMON_FTP_LOGOUT_OK);				// ���O�A�E�gOK
					}

					sw_update.param = 1;
					dl->sw_info[SW_PARAM].exec = 1;
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_COMP);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
					// ���g���C���N���A
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
					retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					rmon_regist(RMON_PRM_DL_DIF_END_OK);					// �_�E�����[�h����
					if ((ftpctrl.result&0x7FFF) == AU_LOGOUT_ERR) {
						rmon_regist(RMON_FTP_LOGOUT_NG);				// ���O�A�E�gNG
					}
					else {
						rmon_regist(RMON_FTP_LOGOUT_OK);				// ���O�A�E�gOK
					}

					sw_update.param = 1;
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//					dl->sw_info[SW_PARAM].exec = 1;
					dl->sw_info[SW_PARAM_DIFF].exec = 1;
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_COMP);
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				default:
					break;
				}
				remotedl_sw_update_bk(&sw_update.main);
			}
		}
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//		status = R_DL_IDLE;
//		if( d_ret == PROG_DL_COMP ){
//			if (sw_update.main == 1 || sw_update.param == 1 || sw_update.wave == 1) {
//				status = R_SW_WAIT;
//			}
//		}
// GG120600(E) // Phase9 ��ʖ����`�F�b�N

// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//		remotedl_status_set( status );									// remotedl_task��~
		if( d_ret != EXCLUDED ){									// �ΏۊO�ł͂Ȃ�
			remotedl_arrange_next_request();
		}
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
		if( connect == CTRL_PROG_DL ){									// �ڑ���ʂ����u�_�E�����[�h�̏ꍇ
			moitor_regist(OPLOG_REMOTE_DL_END, d_ret );					// �_�E�����[�h�����̑��샂�j�^��o�^
		}
	// �ݒ�A�b�v���[�h
	}else if( connect == CTRL_PARAM_UPLOAD){

	// �A�b�v���[�h����
		d_ret = remotedl_result_get(RES_UP);
		if (d_ret == PARAM_UP_COMP) {									// ����I��
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			retry_info_clr(RETRY_KIND_MAX);
			retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
			retry_info_clr(RETRY_KIND_UL);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
			rmon_regist(RMON_PRM_UP_END_OK);							// �A�b�v���[�hOK
			if ((ftpctrl.result&0x7FFF) == AU_LOGOUT_ERR) {
				rmon_regist(RMON_FTP_LOGOUT_NG);						// ���O�A�E�gNG
			}
// GG120600(S) // Phase9 �ÓI��͎w�E�C��
//			else if (kind != RETRY_KIND_CONNECT) {
			else {
// GG120600(E) // Phase9 �ÓI��͎w�E�C��
				rmon_regist(RMON_FTP_LOGOUT_OK);						// ���O�A�E�gOK
			}
			remotedl_complete_request(REQ_KIND_PARAM_UL);
			remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_COMP);
		}
		else if (d_ret != EXCLUDED) {
			// ���g���C���
			if ((d_ret == LOGIN_ERR) || (d_ret == CONN_TIMEOUT_ERR)) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				kind = RETRY_KIND_CONNECT;
				kind = RETRY_KIND_CONNECT_PARAM_UP;
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
			}
			else {
				kind = RETRY_KIND_UL;
			}
			// �G���[�̊Ď��f�[�^��o�^
			switch (d_ret) {
			case LOGIN_ERR:
				rmon_regist(RMON_FTP_LOGIN_ID_PW_NG);					// ID or PW�s��
				break;
			case CONN_TIMEOUT_ERR:
				rmon_regist(RMON_FTP_LOGIN_TIMEOUT_NG);					// �ڑ��^�C���A�E�g
				break;
			case FOMA_COMM_ERR:
				rmon_regist(RMON_PRM_UP_END_COMM_NG);					// �A�b�v���[�h���s
				break;
			case PARAM_UP_ERR:
			default:
				break;
			}
			if ((ftpctrl.result&0x7FFF) == AU_LOGOUT_ERR ||				// ���O�A�E�g�G���[or
				d_ret == FOMA_COMM_ERR) {								// �ʐM�ُ�
				rmon_regist(RMON_FTP_LOGOUT_NG);						// ���O�A�E�gNG
			}
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			else if (kind != RETRY_KIND_CONNECT) {
			else if (kind != RETRY_KIND_CONNECT_PARAM_UP) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				rmon_regist(RMON_FTP_LOGOUT_OK);						// ���O�A�E�gOK
			}
			// �ڑ��E�A�b�v���[�h���g���C
			if (retry_info_set(kind)) {
				// ���g���C���N���A
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
				retry_info_clr(RETRY_KIND_UL);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				if (kind == RETRY_KIND_CONNECT) {
				if (kind == RETRY_KIND_CONNECT_PARAM_UP) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					rmon_regist(RMON_FTP_LOGIN_RETRY_OVER);				// ���g���C�I�[�o�[
				}
				else {
					rmon_regist(RMON_PRM_UP_END_RETRY_OVER);			// ���g���C�I�[�o�[
				}

				// ���g���C�I�[�o�[�Ȃ�󂯕t�����v�����N���A����
				remotedl_complete_request(REQ_KIND_PARAM_UL);
				remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
			}
			else {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				if (kind == RETRY_KIND_CONNECT) {
				if (kind == RETRY_KIND_CONNECT_PARAM_UP) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					retry_time_clr(RETRY_KIND_UL);
				}
			}
		}
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//		status = R_DL_IDLE;
//		remotedl_status_set( status );									// remotedl_task��~
		if( d_ret != EXCLUDED ){									// �ΏۊO�ł͂Ȃ�
			remotedl_arrange_next_request();
		}
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
	}

	queset( OPETCBNO, REMOTE_DL_END, 0, NULL );

	return;
}



//[]----------------------------------------------------------------------[]
///	@brief			ftp_remote_auto_switch
//[]----------------------------------------------------------------------[]
///	@return			None
///	@attention		None
///	@author			T.Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010/10/28<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	ftp_remote_auto_switch( void )
{
	PRG_HDR	*prg;
	t_prog_chg_info *dl = remotedl_info_get();
	uchar	res = 0;
	uchar	no,i;
	
	// �V�X�����̃V�X�e���ؑ։�ʂɑJ�ڂ����sw_update������������Ă��܂��̂ŁA
	// �o�b�N�A�b�v���Ă������f�[�^�ōēx�X�V����B
	remotedl_sw_update_get( &sw_update.main );
	
	if( dl->sw_info[SW_PROG].exec == 2 ||
		dl->sw_info[SW_PROG_EX].exec == 2 ){
		FLT_read_program_version( SysMnt_Work );
		prg = (PRG_HDR*)SysMnt_Work;
		if (IsProgExist(prg)) {
			res = 0x10;
		}
	}
	
	if( dl->sw_info[SW_WAVE].exec == 2 ){
		no = BootInfo.sw.wave;
		no ^= 1;
		FLT_read_wave_sum_version( no, SysMnt_Work );
		prg = (PRG_HDR*)SysMnt_Work;
		if (IsProgExist(prg)) {
			res |= 0x20;
		}
	}
		
	if( dl->sw_info[SW_PARAM].exec == 2 ){
		if( ftp_check_stby_param() ){
			res |= 0x40;
		}
	}
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
	if( dl->sw_info[SW_PARAM_DIFF].exec == 2 ){
		if( ftp_check_stby_param() ){
			res |= 0x40;
		}
	}
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
		
	WACDOG;
	if( res & 0xf0 ){
		if( remotedl_BackUP() ){						// �S�o�b�N�A�b�v
			// �ʐؑ֏��̏�������
			for( i=0; i<BACKUP_RETRY_COUNT; i++){
				if (sw_update_proc()) {
// GG120600(S) // Phase9 �ݒ�DL�̗������c��
					switch (remotedl_connect_type_get()) {
					case CTRL_PARAM_DL:
					case CTRL_PARAM_DIF_DL:
						SetSetDiff(SETDIFFLOG_SYU_REMSET);							// ���u�ݒ�ɂė���
						break;
					}
// GG120600(E) // Phase9 �ݒ�DL�̗������c��
					remotedl_update_set();				// �v���O�����X�V�����t���O�Z�b�g
					remotedl_chg_info_bkup();			// chg_info���o�b�N�A�b�v
					System_reset();						// Main CPU reset (It will not retrun from this function)
				}
				WACDOG;
			}
			res = SW_INFO_WRITE_ERR;
		}else{
			res = BACKUP_ERR;
		}
	}else{
		res = PROG_SW_NONE;
	}
	
	remotedl_result_clr();
	remotedl_result_set( res );
	moitor_regist(OPLOG_REMOTE_SW_END, (uchar)res );
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//	remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:
	case CTRL_PROG_SW:
// GG120600(S) // Phase9 ���ʂ���ʂ���
//		switch (remotedl_result_get(RES_SW)) {
		switch (remotedl_result_get(RES_SW_PROG)) {
// GG120600(E) // Phase9 ���ʂ���ʂ���
		case BACKUP_ERR:
			rmon_regist(RMON_PRG_SW_END_BACKUP_NG);
			break;
		case PROG_SW_NONE:
			rmon_regist(RMON_PRG_SW_END_PROG_FILE_NONE);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5560:�����X�VNG���Ɍ��ʂ��X�V���Ă��Ȃ�)
			remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5560:�����X�VNG���Ɍ��ʂ��X�V���Ă��Ȃ�)
			break;
		case SW_INFO_WRITE_ERR:
			rmon_regist(RMON_PRG_SW_END_FLASH_WRITE_NG);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5560:�����X�VNG���Ɍ��ʂ��X�V���Ă��Ȃ�)
			remotedl_comp_set( INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5560:�����X�VNG���Ɍ��ʂ��X�V���Ă��Ȃ�)
			break;
		default:
			break;
		}

		if (remotedl_connect_type_get() == CTRL_PROG_DL) {
			remotedl_complete_request(REQ_KIND_VER_UP);
		}
		else {
			remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
		}
		break;
	case CTRL_PARAM_DL:
// GG120600(S) // Phase9 ���ʂ���ʂ���
//		switch (remotedl_result_get(RES_SW)) {
		switch (remotedl_result_get(RES_SW_PARAM)) {
// GG120600(E) // Phase9 ���ʂ���ʂ���
		case PROG_SW_NONE:
			rmon_regist(RMON_PRM_SW_END_PARAM_FILE_NONE);
			break;
		case SW_INFO_WRITE_ERR:
			rmon_regist(RMON_PRM_SW_END_FLASH_WRITE_NG);
			break;
		default:
			break;
		}

// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5560:�����X�VNG���Ɍ��ʂ��X�V���Ă��Ȃ�)
		remotedl_comp_set( INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5560:�����X�VNG���Ɍ��ʂ��X�V���Ă��Ȃ�)
		remotedl_complete_request(REQ_KIND_PARAM_CHG);
		break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 ���ʂ���ʂ���
//			switch (remotedl_result_get(RES_SW)) {
		switch (remotedl_result_get(RES_SW_PARAM_DIF)) {
// GG120600(E) // Phase9 ���ʂ���ʂ���
		case PROG_SW_NONE:
			rmon_regist(RMON_PRM_SW_DIF_END_PARAM_FILE_NONE);
			break;
		case SW_INFO_WRITE_ERR:
			rmon_regist(RMON_PRM_SW_DIF_END_FLASH_WRITE_NG);
			break;
		default:
			break;
		}

// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5560:�����X�VNG���Ɍ��ʂ��X�V���Ă��Ȃ�)
		remotedl_comp_set( INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5560:�����X�VNG���Ɍ��ʂ��X�V���Ă��Ȃ�)
		remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
		break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	default:
		break;
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			_displclr_6Line
//[]----------------------------------------------------------------------[]
///	@return			None
///	@attention		None
///	@author			T.Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010/10/28<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	_displclr_6Line( void )
{
	if( !FTP_DISP_MASK() ){		// ���uFTP�ʐM��/���ʃp�����[�^�̃A�b�v���[�h�i�V���[�g�j�łȂ���Ή�ʂ̍X�V���s��
		displclr(6);
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			Param_Upload_ShortCut(FTP client for param upload)
//[]----------------------------------------------------------------------[]
///	@return			event type
///	@attention		None
///	@author			T.Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010/12/09<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
ushort	Param_Upload_ShortCut(void)
{
	ushort	msg;
	ushort 	ret;
	uchar	loop = 0;

	ret = 0;


	ftp_init(_FTP_CLIENT, OPETCBNO );
	memset(&sw_update, 0, sizeof(sw_update));
	au_init();

	Lagtim(OPETCBNO, 7, FTP_TIMEOUT);	// ��ϰ�Ď�����(1s)

	ftp_new_session();
	if( ftpctrl.start != FTP_NEW_SESSION ){
		BUZPIPI();
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[3]);						/* "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@" */
		grachr(6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[34]);							/* "�@�@���O�C���Ɏ��s���܂����@�@" */
		Fun_Dsp(FUNMSG[8]);																		/* "�@                       �I�� " */
		while (1){
			msg = StoF( GetMessage(), 1 );
			switch (msg) {
				case KEY_TEN_F5:	/* �I��(F5) */
					BUZPI();
					return MOD_EXT;
			}
		}
	}
	ftp_connection();
	if( ftpctrl.start != FTP_CONNECT ){
		BUZPIPI();
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[3]);						/* "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@" */
		grachr(6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[34]);							/* "�@�@���O�C���Ɏ��s���܂����@�@" */
		Fun_Dsp(FUNMSG[8]);																		/* "�@                       �I�� " */
		while (1){
			msg = StoF( GetMessage(), 1 );
			switch (msg) {
				case KEY_TEN_F5:	/* �I��(F5) */
					BUZPI();
					return MOD_EXT;
			}
		}
	}
	FAIL_SAFE_TIMER_START( GetTaskID(), GetFailSafeTimerNo() );
	INTERVAL_TIMER_START( TIMEOUT25 );			// �C���^�[�o���^�C�}�[�X�^�[�g

	while (ftpctrl.start){
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
			case TIMEOUT7:
				if( ftpctrl.start != FTP_STOP ){
					// �^�C���A�E�g���̏������s���B
					if( ftpctrl.result == AU_ABORT_END ){
						ftp_send_cancel(0);
					}else{
						ftp_send_quit(0);
					}
				}
				Lagtim(OPETCBNO, 7, FTP_TIMEOUT);	// ��ϰ�Ď�����(1s)
				break;

			case TIMEOUT25:									// �C���^�[�o���^�C�}�[
				FTP_Execute();
				INTERVAL_TIMER_START( TIMEOUT25 );			// �C���^�[�o���^�C�}�[�X�^�[�g
				break;

			case TIMEOUT19:
				ftp_send_quit(0);
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				sysment2_lcd_disconnect();
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				sysment2_change_mode();
				break;
		}
	}
	
	Lagcan(OPETCBNO, 7);
	Lagcan(OPETCBNO, GetFailSafeTimerNo());
	Lagcan(OPETCBNO, 25);
	ftp_exit();

	ret = MOD_EXT;
	// change display
	if( ftpctrl.result != AU_NORMAL_END ){
		BUZPIPI();
		dispmlclr(3,5);
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[3]);										/* "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@" */
		Fun_Dsp(FUNMSG[8]);															/* "�@                       �I�� " */
		
		switch( ftpctrl.result ){
			case 	AU_DISCONNECT:
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[43]);									/* " �R�l�N�V�������ؒf����܂��� " */
				break;																
			case 	AU_LOGIN_ERR:
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR3[34]);									/* "�@�@���O�C���Ɏ��s���܂����@�@" */
				break;																
			case	SCRIPT_FILE_NONE:
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[(!au_script_info_get()?13:15)]);	/* "�X�N���v�g�t�@�C��������܂���" */
				break;																
			case	SCRIPT_FILE_ERR:
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[14]);								/* "   �X�N���v�g���e���s���ł�   " */
				break;
			case	FLASH_WRITE_ERR:
				grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[18]);								/* "FLASH�̏����݂Ɏ��s���Ă��܂� " */
				break;
		}

		loop = 1;
		while( loop ) {
			msg = StoF( GetMessage(), 1 );
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					sysment2_lcd_disconnect();
					if ( SysMnt2ModeLcdDisconnect ){
						BUZPI();
						ret = MOD_CUT;
						loop = 0;
				}
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:
					sysment2_change_mode();
					if (SysMnt2ModeChange){
						BUZPI();
						ret = MOD_CHG;
						loop = 0;
				}
				break;
			case KEY_TEN_F5:	// �I��
				loop = 0;
				BUZPI();
				break;
			default:
				break;
			}
		}
	}
	if (SysMnt2ModeChange){
		ret = MOD_CHG;
	}
	return ret;
}


//[]----------------------------------------------------------------------[]
///	@brief			FAIL_SAFE_TIMER_START( fail safe timer start)
//[]----------------------------------------------------------------------[]
///	@param[in]		a = TaskID ("xxxTBCNO") 
///                 b = Timer number (1�` )          
///	@return			None
///	@attention		None
///	@author			T.Nakayama
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2011/06/16<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	FAIL_SAFE_TIMER_START(uchar a, uchar b){

	ushort	failsafe;
	
	if (a == OPETCBNO) {
		failsafe = 300 * 50;
	}
	else {
		failsafe = 60 * 50;					// 1�����Z�b�g
		remotedl_failsafe_timer_clear();
	}
	Lagtim(a, b, failsafe );
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP��M���b�Z�[�W��ʒm����
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-16<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	FTPmsgSend( ushort msg, void *tmp )
{
	ushort size = 0;
	uchar taskID = GetTaskID();
	
	if( taskID != 0xFF ){
		if( tmp != NULL ){
			size = 1;
		}
		queset( taskID, msg, (char)size, tmp );
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP����M���s�Ď�
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-16<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	FTP_Execute( void )
{

	if( GetTaskID() != 0xFF && !(ftpctrl.send_cmd & 0x80) ){	// Flash�����ݒ���kasagoAPI���R�[�����Ȃ�
		if( GetFTPMode() == _FTP_CLIENT ){
			FTP_Execute_Cliant();
		}else{
			FTP_Execute_Server();
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP����M���s(FTP�T�[�o�[)
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-16<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static	void	FTP_Execute_Server( void )
{
	int		ret;
	
	ret = tfFtpdUserExecute();
	if( ret == TM_ENOERROR ){
		switch( ftpctrl.start ){
			case FTP_QUITE_REQ:
				tfFtpdUserStop();
				break;
			default:
				break;
		}
		_SET_WATCH_TIMER();
	}else{
		if( ftpctrl.login > 0 ){		// ���O�C�����H
			tfFtpdUserStop();
			FTPmsgSend( FTP_LOGOUT, NULL );
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP����M���s(FTP�N���C�A���g)
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-16<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static	void	FTP_Execute_Cliant( void )
{
	int		ret;

	if((ftpctrl.result&0x7FFF) == AU_LOGIN_ERR && ftpctrl.start == FTP_LOGIN_REQ ){
		// ���O�C���G���[����x���������ꍇ�́A�^�C���A�E�g��҂�
	}else{
		if( ftpctrl.start != FTP_STOP ){
			ret = tfFtpUserExecute(ftpctrl.ftpHandle);
			if(func_analize[ftpctrl.start] != NULL ){
				func_analize[ftpctrl.start]( ret );
			}
		}else{
			ftpctrl.start = FTP_IDLE;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ETC�z���̃f�[�^�L���i�T�C�Y�擾�j�̃`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-01-16<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
long	Get_Etc_File_Existence( short no )
{
	char	*p,i;
	long	size = 0;
	
	for (i = 0; etc_files[i].size; i++) {
		if( etc_files[i].fileno ==  no ){
			p = (char*)&ETC_cache[etc_files[i].ofs];
			if (IsETCExits(p)) {
				size = etc_files[i].size;
			}
			break;
		}
	}
	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ғ����̃^�X�N�Z�b�g
//[]----------------------------------------------------------------------[]
///	@param[in]		taskID	:�ғ����̃^�X�NID
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	SetTaskID( uchar taskID )
{
	ftpctrl.taskID = taskID;
	
	if( taskID == OPETCBNO ){
		ftpctrl.fail_safe = 19;
	}else{
		ftpctrl.fail_safe = 4;
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			�ғ����̃^�X�N�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			taskID	:�^�X�NID
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-03-30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	GetTaskID( void )
{
	return ftpctrl.taskID;
}

//[]----------------------------------------------------------------------[]
///	@brief			�t�F�[���Z�[�t�^�C�}�[���ғ����̃^�C�}�[�l���擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			uchar	:�ғ����̃^�C�}�[No
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-05-09<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	GetFailSafeTimerNo( void )
{
	return ftpctrl.fail_safe;
}

//[]----------------------------------------------------------------------[]
///	@brief			����M���̃X�e�[�^�X�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			taskID	:�^�X�NID
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-05-09<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	GetSnd_RecvStatus( void )
{
	return cmd.code;
}
//[]----------------------------------------------------------------------[]
///	@brief			����M���̃X�e�[�^�X�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			sts		:����M�X�e�[�^�X
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-05-09<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	SetSnd_RecvStatus( uchar sts)
{
	if( GetFTPMode() == _FTP_CLIENT ){
		if( GetSnd_RecvStatus() == FCMD_REQ_RECV && !ftpdata.tra_start ){
			return;
		}
	}
	cmd.code = sts;
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP���[�h�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			taskID	:�^�X�NID
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-05-09<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	GetFTPMode( void )
{
	return ftpctrl.mode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_type_cmd(�]���^�C�v�w��j
//[]----------------------------------------------------------------------[]
///	@param[in]		None
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static int ftp_send_type_cmd(void)
{
	int errorCode;

	// �]���^�C�v�w��(TYPE�R�}���h���s)
	errorCode = tfFtpType(ftpctrl.ftpHandle, TM_TYPE_BINARY);
	switch (errorCode) {
	case TM_ENOERROR:
	case TM_EWOULDBLOCK:
	case TM_FTP_XFERSTART:
	case TM_FTP_FILEOKAY:
		errorCode = OK;
		break;
	default:
		errorCode = NG;
		break;
	}
	if (errorCode != OK) {
		ftpctrl.start = FTP_QUITE_REMOTE_REQ;
		if (errorCode == TM_ENOTLOGIN || ftpctrl.login == -1) {
			ftpctrl.result = AU_LOGIN_ERR;
			remotedl_monitor_info_set(13);
			remotedl_result_set(LOGIN_ERR);
		}
		else {
			ftpctrl.result = AU_DISCONNECT;
			remotedl_monitor_info_set(14);
			remotedl_result_set(FOMA_COMM_ERR);
		}
	}
	else {
		ftpctrl.start = FTP_TYPE_REQ;
		_SET_WATCH_TIMER();
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_type_wait(�]���^�C�v�ݒ艞���҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X(���g�p�j
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static int ftp_type_wait(int res)
{
	int errorCode = res;

	switch( errorCode ){
	case TM_ENOERROR:						// ����
		switch (remotedl_connect_type_get()) {
		case CTRL_PROG_DL:
		case CTRL_PARAM_DL:
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case CTRL_PARAM_DIF_DL:
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)		
			// �v���O�����t�@�C��or�ݒ�t�@�C���_�E�����[�h
			ftpctrl.start = FTP_RETR_REQ;
			_SET_WATCH_TIMER();
			// �t�@�C���ꗗ���\�[�g(����)
			qsort(ftpctrl.dir_ent_name, ftpctrl.dir_ent_cnt, _REMOTE_MAX_PATH, (_Cmpfun *)compare_filename);
			// Flash�ւ�write�����擾
			ftp_get_write_info();
			break;
		case CTRL_PARAM_UPLOAD:
			// �ݒ�t�@�C���A�b�v���[�h
			ftpctrl.start = FTP_PARAM_UP_REQ;
			break;
		case CTRL_CONNECT_CHK:
			// test�p�t�@�C���A�b�v���[�h
			SetSnd_RecvStatus( FCMD_REQ_SEND );
			MakeRemoteFileName(cmd.remote, cmd.local, MAKE_FILENAME_TEST_UP);
			ftp_send_req_cmd(&cmd);							// �R�}���h���M
			ftpctrl.start = FTP_TEST_UP_WAIT;				// ���M�����҂�
			break;
		}
		break;
	case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
	case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
	case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
		_SET_WATCH_TIMER();
		break;
	default:
		ftpctrl.start = FTP_QUITE_REMOTE_REQ;
		remotedl_monitor_info_set(15);
		remotedl_result_set(PROG_DL_NONE);			// �A�b�v���[�h���s
		break;
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_cwd_cmd(�f�B���N�g���ړ��j
//[]----------------------------------------------------------------------[]
///	@param[in]		cmd		: �����[�g�t�@�C����
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static int ftp_send_cwd_cmd(struct _ft_sr_ *cmd)
{
	int errorCode;

	// �f�B���N�g���ړ�(CWD�R�}���h���s)
	errorCode = tfFtpCwd(ftpctrl.ftpHandle, (char *)cmd->remote);
	switch (errorCode) {
	case TM_ENOERROR:
	case TM_EWOULDBLOCK:
	case TM_FTP_XFERSTART:
	case TM_FTP_FILEOKAY:
		errorCode = OK;
		break;
	default:
		errorCode = NG;
		break;
	}
	if (errorCode != OK) {
		ftpctrl.start = FTP_QUITE_REMOTE_REQ;
		if (errorCode == TM_ENOTLOGIN || ftpctrl.login == -1) {
			ftpctrl.result = AU_LOGIN_ERR;
			remotedl_monitor_info_set(6);
			remotedl_result_set(LOGIN_ERR);
		}
		else {
			ftpctrl.result = AU_DISCONNECT;
			remotedl_monitor_info_set(7);
			remotedl_result_set(FOMA_COMM_ERR);
		}
	}
	else {
		ftpctrl.start = FTP_CWD_REQ;
		_SET_WATCH_TIMER();
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_cwd_wait(�f�B���N�g���ړ������҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X(���g�p�j
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static int ftp_cwd_wait(int res)
{
	int errorCode = res;

	switch( errorCode ){
	case TM_ENOERROR:						// ����
		// NLST�R�}���h���s
		memset(&cmd, 0, sizeof(cmd));
		ftp_send_nlst_cmd(&cmd);
		break;
	case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
	case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
	case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
		_SET_WATCH_TIMER();
		break;
	default:
		ftpctrl.start = FTP_QUITE_REMOTE_REQ;
		remotedl_monitor_info_set(8);
		remotedl_result_set(PROG_DL_NONE);			// �A�b�v���[�h���s
		break;
	}
	return errorCode;

}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_make_dl_filename(DL�t�@�C�����쐬�j
//[]----------------------------------------------------------------------[]
///	@param[in]		cmd		: �����[�g�t�@�C�����A���[�J���t�@�C����
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static void ftp_make_dl_filename(struct _ft_sr_ *cmd)
{
	memset(cmd, 0, sizeof(*cmd));
	// �����[�g�t�@�C����
	strncpy((char *)cmd->remote, (const char *)&ftpctrl.dir_ent_name[ftpctrl.cur_dir_ent], sizeof(cmd->remote));

	// ���[�J���t�@�C����
	sprintf((char *)cmd->local, "/REMOTEDL/%s", ftpctrl.dir_ent_name[ftpctrl.cur_dir_ent]);
	ftpctrl.cur_dir_ent++;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_nlst_cmd(�t�@�C���ꗗ�擾�j
//[]----------------------------------------------------------------------[]
///	@param[in]		cmd		: �����[�g�t�@�C����
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static int ftp_send_nlst_cmd(struct _ft_sr_ *cmd)
{
	int errorCode;

	// ���Ԃ��擾
	remotedl_prog_no_get((char *)ftpctrl.filename, sizeof(ftpctrl.filename));
	ftpctrl.filename_size = (uchar)strlen((char *)ftpctrl.filename);
	// �t�@�C���ꗗ�擾(NLST�R�}���h���s)
	errorCode = tfFtpDirList(ftpctrl.ftpHandle, (char *)cmd->remote, TM_DIR_SHORT, NLSTcmd_Callback);
	switch (errorCode) {
	case TM_ENOERROR:
	case TM_EWOULDBLOCK:
	case TM_FTP_XFERSTART:
	case TM_FTP_FILEOKAY:
		errorCode = OK;
		break;
	default:
		errorCode = NG;
		break;
	}
	if (errorCode != OK) {
		ftpctrl.start = FTP_QUITE_REMOTE_REQ;
		if (errorCode == TM_ENOTLOGIN || ftpctrl.login == -1) {
			ftpctrl.result = AU_LOGIN_ERR;
			remotedl_monitor_info_set(9);
			remotedl_result_set(LOGIN_ERR);
		}
		else {
			ftpctrl.result = AU_DISCONNECT;
			remotedl_monitor_info_set(10);
			remotedl_result_set(FOMA_COMM_ERR);
		}
	}
	else {
		ftpctrl.start = FTP_NLST_REQ;
		_SET_WATCH_TIMER();
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			compare_filename(�t�@�C������r�j
//[]----------------------------------------------------------------------[]
///	@param[in]		str1	: �t�@�C����
///	@param[in]		str2	: �t�@�C����
///	@return			-1:str1 > str2, 0:str1 == str2, 1:str1 < str2
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static int compare_filename(const void *str1, const void *str2)
{
    return (int)strncmp((char *)str1, (char *)str2, _REMOTE_MAX_PATH);
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_nlst_wait(�t�@�C���ꗗ�擾�����҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X(���g�p�j
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static int ftp_nlst_wait(int res)
{
	switch (res) {
	case TM_ENOERROR:
		if (ftpctrl.dir_ent_cnt != 0) {
			// TYPE�R�}���h���s
			ftp_send_type_cmd();
		}
		else {
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			remotedl_monitor_info_set(11);
			remotedl_result_set(PROG_DL_NONE);
		}
		break;
	case TM_EWOULDBLOCK:
		_SET_WATCH_TIMER();
		break;
	default:
		ftpctrl.start = FTP_QUITE_REMOTE_REQ;
		ftpctrl.result |= 0x8000;
		remotedl_monitor_info_set(12);
		remotedl_result_set(PROG_DL_NONE);
		break;
	}
	return res;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_get_file(�t�@�C��DL�j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X(���g�p�j
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static int ftp_get_file(int res)
{
	int errorCode = res;
// GG120600(S) // Phase9 ���ʂ���ʂ���
	uchar ucRes = 0;
// GG120600(E) // Phase9 ���ʂ���ʂ���

	if( !ftpctrl.result || ftpctrl.result == TM_EACCES ){
		switch( errorCode ){
		case TM_ENOERROR:							// ����
			if( GetSnd_RecvStatus() != FCMD_REQ_IDLE ){
				if( ftpctrl.result == TM_EACCES ){
					ftpctrl.result = ftp_send_req_cmd( &cmd );
				}
				_SET_WATCH_TIMER();
				break;
			}
			// DL�r��
			if (ftpctrl.cur_dir_ent < ftpctrl.dir_ent_cnt) {
				// �_�E�����[�h�t�@�C�����Z�b�g
				ftp_make_dl_filename(&cmd);
				// �t�@�C���_�E�����[�h(RETR�R�}���h���s)
				SetSnd_RecvStatus( FCMD_REQ_RECV );
				ftp_send_req_cmd( &cmd );
			}
			// �S�t�@�C��DL����
			else {
				ftpctrl.result = AU_NORMAL_END | 0x8000;
				ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			}
			break;
		case TM_EWOULDBLOCK:
		case TM_FTP_XFERSTART:
		case TM_FTP_FILEOKAY:
			_SET_WATCH_TIMER();
			break;
		default:
// GG120600(S) // Phase9 ���ʂ���ʂ���
//			if (remotedl_result_get(RES_DL) != PROG_DL_ERR) {
			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
				ucRes = RES_DL_PROG;
				break;
			case CTRL_PARAM_DL:
				ucRes = RES_DL_PARAM;
				break;
			case CTRL_PARAM_DIF_DL:		// �����ݒ�_�E�����[�h
				ucRes = RES_DL_PARAM_DIF;
				break;
			}
			if (remotedl_result_get(ucRes) != PROG_DL_ERR) {
// GG120600(E) // Phase9 ���ʂ���ʂ���
				remotedl_monitor_info_set(16);
				remotedl_result_set(FOMA_COMM_ERR);
			}
			ftpctrl.result = AU_DISCONNECT | 0x8000;
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			break;
		}
	}

	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			NLSTcmd_Callback(�t�@�C���ꗗ�擾���̃R�[���o�b�N�֐��j
//[]----------------------------------------------------------------------[]
///	@param[in]		ftpSessionPtr	: FTP�n���h��
///	@param[in]		bufferPtr		: �t�@�C����
///	@param[in]		bufferSize		: �t�@�C�����̒���(CR+LF+\0�܂�)
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int NLSTcmd_Callback(ttUserFtpHandle ftpSessionPtr, char TM_FAR *bufferPtr, int bufferSize)
{
	char *p;
	int fname_size = bufferSize - 3;	// bufferSize��(CR+LF+\0)���܂�

	if (ftpctrl.dir_ent_cnt >= _MAX_DIR_ENT_CNT) {
		return -1;
	}

	// ���ԂƊg���q�̃`�F�b�N
	p = strrchr(bufferPtr, '.');
	if (p) {
		switch (remotedl_connect_type_get()) {
		case CTRL_PROG_DL:
			// �t�@�C�����̐擪�����Ԃƈ�v���邩�H
			// �g���q����v���邩�H
			if (!strncmp(bufferPtr, (char *)ftpctrl.filename, ftpctrl.filename_size) &&
				!strcmp(p, PROG_FILE_EXT)) {
				// �t�@�C�������R�s�[
				strncpy((char *)&ftpctrl.dir_ent_name[ftpctrl.dir_ent_cnt++], bufferPtr, fname_size);
			}
			break;
		case CTRL_PARAM_DL:
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case CTRL_PARAM_DIF_DL:		// �����ݒ�_�E�����[�h
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			// �ݒ�t�@�C����1�����_�E�����[�h����
			if (ftpctrl.dir_ent_cnt == 0) {
				// �t�@�C��������v���邩�H
				sprintf((char *)ftpctrl.filename, "%s\r\n", PARAM_FILE_NAME);
				if (!strcmp(bufferPtr, (char *)ftpctrl.filename)) {
					// �t�@�C�������R�s�[
					strncpy((char *)&ftpctrl.dir_ent_name[ftpctrl.dir_ent_cnt++], bufferPtr, fname_size);
				}
			}
			break;
		default:
			break;
		}
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			get_remotedl_filename(DL�t�@�C�����擾�j
//[]----------------------------------------------------------------------[]
///	@param[in]		szName	: �t�@�C�����擾�o�b�t�@
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void get_remotedl_filename(char *szName)
{
	strcat(szName, (char *)&ftpctrl.dir_ent_name[ftpctrl.cur_dir_ent-1]);
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_set_write_info(write���ݒ�j
//[]----------------------------------------------------------------------[]
///	@param[in]		None
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static void ftp_set_write_info(void)
{
	t_write_info write_info;

	memcpy(&write_info.header, &ftpdata.write.header, sizeof(ftpdata.write.header));
	write_info.base_addr = ftpdata.write.base_addr;
	write_info.sum = ftpdata.write.sum;
	write_info.cur_dir_ent = ftpctrl.cur_dir_ent;
// GG129000(S) R.Endo 2022/12/28 �Ԕԃ`�P�b�g���X4.0 #6754 �v���O�����_�E�����[�h�������g���C��ɉ��u�Ď��f�[�^���M���� [���ʉ��P���� No1540]
	write_info.uscrcTemp = ftpctrl.CRC16.uscrcTemp;
// GG129000(E) R.Endo 2022/12/28 �Ԕԃ`�P�b�g���X4.0 #6754 �v���O�����_�E�����[�h�������g���C��ɉ��u�Ď��f�[�^���M���� [���ʉ��P���� No1540]
	remotedl_write_info_set(&write_info);
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_get_write_info(write���擾�j
//[]----------------------------------------------------------------------[]
///	@param[in]		None
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static void ftp_get_write_info(void)
{
	t_write_info write_info;

	remotedl_write_info_get(&write_info);
	memcpy(&ftpdata.write.header, &write_info.header, sizeof(ftpdata.write.header));
	ftpdata.write.base_addr = write_info.base_addr;
	ftpdata.write.sum = write_info.sum;
	ftpctrl.cur_dir_ent = write_info.cur_dir_ent;
// GG129000(S) R.Endo 2022/12/28 �Ԕԃ`�P�b�g���X4.0 #6754 �v���O�����_�E�����[�h�������g���C��ɉ��u�Ď��f�[�^���M���� [���ʉ��P���� No1540]
	ftpctrl.CRC16.uscrcTemp = write_info.uscrcTemp;
// GG129000(E) R.Endo 2022/12/28 �Ԕԃ`�P�b�g���X4.0 #6754 �v���O�����_�E�����[�h�������g���C��ɉ��u�Ď��f�[�^���M���� [���ʉ��P���� No1540]
}

// GG120600(S) // Phase9 CRC�`�F�b�N
static void ftp_set_write_crc(uchar by1,uchar by2)
{
	remotedl_prog_crc_set(by1,by2);
}

static ushort ftp_get_write_crc(void)
{
	return remotedl_prog_crc_get();
	
}
// GG120600(E) // Phase9 CRC�`�F�b�N


//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_mkd_cmd(�t�H���_�쐬�j
//[]----------------------------------------------------------------------[]
///	@param[in]		cmd		: cmd�|�C���^
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static	int	ftp_send_mkd_cmd(struct _ft_sr_ *cmd)
{
	int		errorCode;
// GG120600(S) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
	int dirname_len, target_len, i;


	target_len = strlen((char *)cmd->remote);
	dirname_len = strlen((char *)cmd->mkd) + 1; // +1��'/'�̕�
	for (i = dirname_len; i < target_len; i++) {
		if (cmd->remote[i] == '/') {
			break;
		}
	}
	memcpy(cmd->mkd, cmd->remote, i);
// GG120600(E) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
	
	errorCode = tfFtpMkd(ftpctrl.ftpHandle,
// GG120600(S) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
//							(char*)cmd->remote,
							(char*)cmd->mkd,
// GG120600(E) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
							(char*)cmd->local,		// dummy
							sizeof(cmd->local));
	switch( errorCode ){
		case	TM_ENOERROR:			// ����
		case	TM_EWOULDBLOCK:			// ����FTP�Z�b�V�����̓m���u���b�L���O�ŁA�I�y���[�V�������������Ă��Ȃ�
		case	TM_FTP_XFERSTART:		// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
		case	TM_FTP_FILEOKAY:		// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
			errorCode = OK;
			ftpctrl.send_cmd = STS_SENDING;
			break;
		default:
			errorCode = NG;
			break;
	}
	
	if( errorCode != OK ){
		ftpctrl.start = FTP_QUITE_REMOTE_REQ;
		if( errorCode == TM_ENOTLOGIN || ftpctrl.login == -1 ){
			ftpctrl.result = AU_LOGIN_ERR;
			remotedl_monitor_info_set(32);
			remotedl_result_set(LOGIN_ERR);
		}else{
			ftpctrl.result = AU_DISCONNECT;
			remotedl_monitor_info_set(33);
			remotedl_result_set(FOMA_COMM_ERR);
		}
	}else{
		ftpctrl.start = FTP_MKD_REQ;
		_SET_WATCH_TIMER();
	}
	
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_mkd_wait(�t�H���_�쐬�����҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X(���g�p�j
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static	int	ftp_mkd_wait( int res )
{
	int		errorCode = res;

	if( remotedl_connect_type_get() == CTRL_PARAM_UPLOAD){
		switch( errorCode ){
		case TM_ENOERROR:						// ����
		case TM_FTP_NAVAIL:						// �v�����ꂽ���N�G�X�g�̓A�N�Z�X������t�@�C���V�X�e���̗��R�Ŏ��s�ł��Ȃ�
// GG120600(S) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
//			// TYPE�R�}���h���s
//			ftp_send_type_cmd();
			if (strlen((char *)cmd.remote) == strlen((char *)cmd.mkd)) {
				ftp_send_type_cmd();
			}
			else {
				
				ftp_send_mkd_cmd(&cmd);
			}
// GG120600(E) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
			break;
		case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
		case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
		case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
			_SET_WATCH_TIMER();
			break;
		default:
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			remotedl_monitor_info_set(34);
			remotedl_result_set(PARAM_UP_ERR);			// �A�b�v���[�h���s
			rmon_regist(RMON_FTP_FILE_TRANS_NG);		// �]�����s
			break;
		}
	}
	return errorCode;

}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_param(�p�����[�^���M�j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X(���g�p�j
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static	int	ftp_send_param(int res)
{
	
	int		errorCode = TM_ENOERROR;
	
	if( ftpctrl.start == FTP_PARAM_UP_REQ ){
		
		memset( &cmd, 0, sizeof(cmd) );					// �R�}���h�G���A������
		SetSnd_RecvStatus( FCMD_REQ_SEND );
		MakeRemoteFileName( cmd.remote, cmd.local, MAKE_FILENAME_PARAM_UP );	// ���u�_�E�����[�h�p�̃f�[�^�쐬
		if( ftp_send_req_cmd(&cmd) == OK ){				// �R�}���h���M
			ftpctrl.start = FTP_PARAM_UP_WAIT;			// �A�b�v�����҂�		
			errorCode = TM_ENOERROR;
		}else{
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
		}
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_param_wait(�p�����[�^���M�����҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static	int	ftp_send_param_wait( int res )
{
	
	int		errorCode = res;
	
	if(!( ftpctrl.result & 0x8000 )){
		switch( errorCode ){
		case TM_ENOERROR:							// ����
			remotedl_result_set(PARAM_UP_COMP);
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			break;
		case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
		case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
		case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
			_SET_WATCH_TIMER();
			break;
		case TM_FTP_NOTLOGIN:					// ���O�C������Ă��Ȃ�
		case TM_ENOTLOGIN:						// ���[�U�͌��݃��O�C������Ă��Ȃ�
			ftpctrl.result = (AU_LOGIN_ERR | 0x8000);
			break;
		case TM_FTP_FILENAVAIL:					// �v�����ꂽ�t�@�C������̓A�N�Z�X������t�@�C���V�X�e���̗��R�Ŏ��s�ł��Ȃ�
		case TM_FTP_XFERABOR:					// 
			ftpctrl.result = (SCRIPT_FILE_NONE | 0x8000);
			break;
		default:
			ftpctrl.result = (AU_DISCONNECT | 0x8000);
			break;
		}
		if(( ftpctrl.result & 0x8000 )){
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			remotedl_monitor_info_set(35);
			remotedl_result_set(PARAM_UP_ERR);	// �A�b�v���[�h���s
			rmon_regist(RMON_FTP_FILE_TRANS_NG);				// �]�����s
		}
	}
	return errorCode;
}

// MH810100(S)
// //[]----------------------------------------------------------------------[]
// ///	@brief			ftp_read_param_All(�p�����[�^��CSV�̌`���őS���ǂ�)
// //[]----------------------------------------------------------------------[]
// ///	@param[in]		buff	: buffer
// ///	@param[in]		nSize	: buffer�̃T�C�Y
// ///	@return			send length
// ///	@attention		None
// //[]----------------------------------------------------------------------[]
// //[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
// static	int	ftp_read_param_All(char *buff,int nSize)
// {
// 	int		len = 0;
// 	int		cnt = 999;
// 	int		nTotalSize = 0;
//	
// 	while(cnt){
// 		len = ftp_read_param(buff + nTotalSize);
// 		nTotalSize  += len;
// 		if(len == 0 ){
// 			// 0�����^�[�����ꂽ��I��
// 			break;
// 		}
// 		if( nTotalSize > nSize){
// 			nTotalSize = 0;
// 		}
// 		WACDOG;
// 		cnt--;	// �������[�v�������
// 	}
//
// 	return nTotalSize;
// }
//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�i�p�����[�^�j��CSV�̌`���őS���ǂ�
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer
///	@param[in]		nSize	: buffer�̃T�C�Y
///	@return			send length
/// @author			Yamauchi
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/11/12<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static	int	ftp_read_param_All(char *buff,int nSize)
{
	int		TotalLen = 0;			// �o�b�t�@�ɋl�߂��T�C�Y
	ushort	usSec, usItem;			// ���[�v�J�E���^
	ushort	usItemCnt;				// 1�Z�N�V�����̃A�C�e����
	uchar	Asc[20];
	ulong	usTemp = 0;
	ushort	usLen = 0;				// �ϊ����1�s�T�C�Y
	ulong	*pTgtSecTop;			// Top item data address in target Section 
	t_AppServ_ParamInfoTbl	*param_tbl;

	if( ftpdata.send.step == 1 ){
		param_tbl =	(void*)FLT_GetParamTable();		// �p�����[�^���e�[�u���擾
		AppServ_ConvParam2CSV_Init( param_tbl, SYSMNT_DATALEN_MAX );	// ���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��i�������j
		memset( buff, 0, nSize );
		ftpdata.send.step++;
	}

	if( ftpdata.send.step == 2 ){
		// �Z�N�V�������[�v
		for( usSec=1; usSec<C_PRM_SESCNT_MAX; usSec++ ){
			pTgtSecTop = AppServ_BinCsvSave.param_info.csection[usSec].address;
			usItemCnt = AppServ_BinCsvSave.param_info.csection[usSec].item_num;
			// �Z�N�V�����ɃA�C�e�����Ȃ���Ύ��̃Z�N�V������
			if( !usItemCnt ){
				continue;
			}
			// �A�C�e�����[�v
			for( usItem=1; usItem<usItemCnt; usItem++ ){
				WACDOG;
				usLen = 0;
				memset( Asc, 0, sizeof(Asc) );
				usTemp = (usSec * 10000L) + usItem;
				intoasl( Asc, usTemp, (unsigned short)6 );		// �A�h���X��Ascii �ɕϊ�
				usLen = 6;
				Asc[usLen++] = ',';
				usLen += intoasl_0sup( &Asc[usLen], pTgtSecTop[usItem], (unsigned short)6 );	// �l��Ascii �ɕϊ�
				Asc[usLen++] = 0x0d;
				Asc[usLen++] = 0x0a;

				// �o�b�t�@�󂫂���H
				if( nSize > usLen + TotalLen){	// 64KB
					// �o�b�t�@��1�s���R�s�[
					memcpy( &buff[TotalLen], (const char*)Asc, strlen( (const char*)Asc) );
				}
				TotalLen += strlen( (const char*)Asc);		// �S�Z�N�V�������̃T�C�Y
			}
		}
		ftpdata.send.step++;
	}

	return TotalLen;
}
// MH810100(E)

//[]----------------------------------------------------------------------[]
///	@brief			ftp_test_up_wait(�ڑ��e�X�g�t�@�C���A�b�v���[�h�҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static	int	ftp_test_up_wait( int res )
{
	
	int		errorCode = res;
	
	if(!( ftpctrl.result & 0x8000 )){
		switch( errorCode ){
		case TM_ENOERROR:							// ����
			ftpctrl.start = FTP_TEST_DOWN;
			break;
		case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
		case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
		case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
			_SET_WATCH_TIMER();
			break;
		case TM_FTP_NOTLOGIN:					// ���O�C������Ă��Ȃ�
		case TM_ENOTLOGIN:						// ���[�U�͌��݃��O�C������Ă��Ȃ�
			ftpctrl.result = (AU_LOGIN_ERR | 0x8000);
			break;
		case TM_FTP_FILENAVAIL:					// �v�����ꂽ�t�@�C������̓A�N�Z�X������t�@�C���V�X�e���̗��R�Ŏ��s�ł��Ȃ�
		case TM_FTP_XFERABOR:					// 
			ftpctrl.result = (SCRIPT_FILE_NONE | 0x8000);
			break;
		default:
			ftpctrl.result = (AU_DISCONNECT | 0x8000);
			break;
		}
		if(( ftpctrl.result & 0x8000 )){
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			remotedl_monitor_info_set(36);
			remotedl_result_set(TEST_UP_ERR);	// �A�b�v���[�h���s
			rmon_regist(RMON_FTP_FILE_TRANS_NG);				// �]�����s
		}
	}
	return errorCode;

}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_test_down(�ڑ��e�X�g�t�@�C���A�b�v���[�h�҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static	int	ftp_test_down( int res )
{
	
	int		errorCode = TM_ENOERROR;
	
	if( ftpctrl.start == FTP_TEST_DOWN ){
		
		memset( &cmd, 0, sizeof(cmd) );					// �R�}���h�G���A������
		SetSnd_RecvStatus( FCMD_REQ_RECV );
		MakeRemoteFileName( cmd.remote, cmd.local, MAKE_FILENAME_TEST_UP );	// ���u�_�E�����[�h�p�̃f�[�^�쐬
		if( ftp_send_req_cmd(&cmd) == OK ){				// �R�}���h���M
			ftpctrl.start = FTP_TEST_DOWN_WAIT;			// �_�E�����[�h�����҂�		
			errorCode = TM_ENOERROR;
		}else{
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			ftpctrl.result |= 0x8000;
			remotedl_monitor_info_set(37);
			remotedl_result_set(TEST_DOWN_ERR);	// �_�E�����[�h���s
			rmon_regist(RMON_FTP_FILE_TRANS_NG);				// �]�����s
		}
	}
	return errorCode;

}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_test_down_wait(�ڑ��e�X�g�t�@�C���_�E�����[�h�҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static	int	ftp_test_down_wait( int res )
{
	
	int		errorCode = res;
	
	if(!( ftpctrl.result & 0x8000 )){
		switch( errorCode ){
		case TM_ENOERROR:							// ����
			ftpctrl.start = FTP_TEST_DEL;
			break;
		case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
		case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
		case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
			_SET_WATCH_TIMER();
			break;
		case TM_FTP_NOTLOGIN:					// ���O�C������Ă��Ȃ�
		case TM_ENOTLOGIN:						// ���[�U�͌��݃��O�C������Ă��Ȃ�
			ftpctrl.result = (AU_LOGIN_ERR | 0x8000);
			break;
		case TM_FTP_FILENAVAIL:					// �v�����ꂽ�t�@�C������̓A�N�Z�X������t�@�C���V�X�e���̗��R�Ŏ��s�ł��Ȃ�
		case TM_FTP_XFERABOR:					// 
			ftpctrl.result = (SCRIPT_FILE_NONE | 0x8000);
			break;
		default:
			ftpctrl.result = (AU_DISCONNECT | 0x8000);
			break;
		}
		if(( ftpctrl.result & 0x8000 )){
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			remotedl_monitor_info_set(38);
			remotedl_result_set(TEST_DOWN_ERR);	// �_�E�����[�h���s
			rmon_regist(RMON_FTP_FILE_TRANS_NG);				// �]�����s
		}
	}
	return errorCode;

}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_test_del(�ڑ��e�X�g�t�@�C���폜�j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static	int	ftp_test_del( int res )
{
	
	int		errorCode = res;
	
	if( ftpctrl.start == FTP_TEST_DEL ){
		memset( &cmd, 0, sizeof(cmd) );										// �R�}���h�G���A������
		MakeRemoteFileName( cmd.remote, cmd.local, MAKE_FILENAME_TEST_UP );	// �t�@�C�����쐬
		errorCode = tfFtpDele(ftpctrl.ftpHandle,(char*)cmd.remote);
		switch( errorCode ){
			case	TM_ENOERROR:			// ����
			case	TM_EWOULDBLOCK:			// ����FTP�Z�b�V�����̓m���u���b�L���O�ŁA�I�y���[�V�������������Ă��Ȃ�
			case	TM_FTP_XFERSTART:		// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
			case	TM_FTP_FILEOKAY:		// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
				errorCode = OK;
				ftpctrl.send_cmd = STS_SENDING;
				break;
			default:
				errorCode = NG;
				break;
		}
		
		if( errorCode != OK ){
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			ftpctrl.result |= 0x8000;
			remotedl_monitor_info_set(39);
			remotedl_result_set(TEST_DEL_ERR);	// �폜���s
			rmon_regist(RMON_FTP_FILE_DEL_NG);				// �t�@�C���폜���s
		}else{
			ftpctrl.start = FTP_TEST_DEL_WAIT;
			_SET_WATCH_TIMER();
		}
	}
	
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_test_del_wait(�ڑ��e�X�g�t�@�C���폜�҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static	int	ftp_test_del_wait( int res )
{
	
	int		errorCode = res;
	
	if( remotedl_connect_type_get() == CTRL_CONNECT_CHK){	
		if(!( ftpctrl.result & 0x8000 )){
			switch( errorCode ){
			case TM_ENOERROR:							// ����
				ftpctrl.start = FTP_QUITE_REMOTE_REQ;
				break;
			case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
			case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
			case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
				_SET_WATCH_TIMER();
				break;
			default:
				ftpctrl.result = (AU_DISCONNECT | 0x8000);
				break;
			}
			if(errorCode == TM_ENOERROR){
				remotedl_result_set(TEST_CHECK_COMP);	// �ڑ��e�X�g����
			}else if( ftpctrl.result & 0x8000 ){
				remotedl_monitor_info_set(40);
				remotedl_result_set(TEST_DEL_ERR);		// �폜���s
				rmon_regist(RMON_FTP_FILE_DEL_NG);	// �t�@�C���폜���s
			}
		}
	}
	return errorCode;

}
//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_quit_remote(send quit request)
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static	int	ftp_send_quit_remote( int res )
{
	int	errorCode = res;
	// �G���[�̗L���ɂ�����炸QUIT�𓊂���
	errorCode = tfFtpQuit(ftpctrl.ftpHandle );
	ftpctrl.start = FTP_QUITE_REMOTE_WAIT;
	
	return errorCode;
}
//[]----------------------------------------------------------------------[]
///	@brief			ftp_quit_remote_wait(�ؒf�҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res		: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
static	int	ftp_quit_remote_wait( int res )
{
	int		errorCode = res;

	switch( errorCode ){
	case TM_ENOERROR:						// ����
		ftpctrl.result = (AU_NORMAL_END | 0x8000);
		ftp_free_session(res);
		break;
	case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
		_SET_WATCH_TIMER();
		break;
	default:
		ftpctrl.start = FTP_CLOSE_REQ;
		ftpctrl.result = (AU_LOGOUT_ERR | 0x8000);
		break;
	}
	return errorCode;

}


/*--------------------------------------------------------------------------*/
/*		SNTP�����␳														*/
/*--------------------------------------------------------------------------*/
static	const ulong		DAY_SEC		= 24*60*60L;
static	const ulong		DAY_MSEC	= 24*60*60L*1000L;
static	const ushort	days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define	_1980_3_1		2529705600L		// AMANO�V�X�e���̃m�[�}���C�Y���

extern char	leapyear_check( short pa_yer );
extern int	KSG_gPpp_RauStarted;

//[]----------------------------------------------------------------------[]
///	@brief			sntp_get_seconds(get NTP sec)
//[]----------------------------------------------------------------------[]
///	@param[in]		*timestamp	: timestamp
///	@param[out]		*tm			: sec & msec
///	@return			none
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2013/04/02<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
struct _ntp_time_ {
	ulong	sec;
	ushort	msec;
};

static	void	sntp_get_seconds(ulong *timestamp, struct _ntp_time_ *tm)
{
	ulong	ltime;
	ulong	u_point;

// ��1970.1.1���O�ɂ��邱�Ƃ�NTP��2036�N�����������
	ltime = timestamp[0] - TM_JAN_1970;

	// �~���b�擾
	u_point = timestamp[1];		// �b�����i�����_�ȉ��j
	u_point >>= 16;
	if (timestamp[1] & 0x00008000L) {
	// �؂�グ
		u_point++;
		if (u_point >= 0x10000L) {
			ltime++;
			u_point -= 0x10000L;
		}
	}
	// 
	u_point *= 1000L;			// �b���~���b
	tm->sec = ltime;
	tm->msec = (ushort)(u_point >> 16);
}

//[]----------------------------------------------------------------------[]
///	@brief			sntp_get_sec(get current sec)
//[]----------------------------------------------------------------------[]
///	@param[in]		*now		: current time
///	@return			none
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2013/04/02<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
static	ulong	sntp_get_nowsec(struct clk_rec *now)
{
	ulong	ltime;
	ushort	year, month;
	ulong	d;

	ltime = 0;
	ltime += (ulong)now->seco;								// �b�����Z
	ltime += (ulong)( now->minu * 60 );						// �������Z
	ltime += (ulong)( now->hour * 60 * 60 );				// �������Z
	ltime += (ulong)( now->date * 60 * 60 * 24 );			// �������Z

	for (month = 1; month < now->mont; month++) {			// �������Z
		d = ((month != 2) || leapyear_check( (short)now->year ) == 0 ) ? days[month] : 29;
		ltime += d * 60 * 60 * 24;
	}
	for (year = 1985; year < now->year; year++) {	// �N�����Z
		d = leapyear_check( (short)year ) ? 366 : 365;
		ltime += d * 60 * 60 * 24;
	}
	// 1984/12/31(UTC)�����Z
	ltime += TM_JAN_1985 - (60 * 60 * 24);

	return ltime;
}

//[]----------------------------------------------------------------------[]
///	@brief			time_subtraction
//[]----------------------------------------------------------------------[]
///	@param[out]		*tm			: sec & msec
///	@return			none
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2013/04/02<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
static	void	time_subtraction(struct _ntp_time_ *t1, struct _ntp_time_ *t2)
{
	if (t1->sec > t2->sec ||
		(t1->sec == t2->sec && t1->msec > t2->msec)) {
	// T1 > T2
		t1->sec -= t2->sec;
		t1->msec -= t2->msec;
		if (t1->msec & 0x8000) {
			t1->msec += 1000;
			t1->sec--;
		}
	}
	else {
		t1->sec = t1->msec = 0;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�����␳������
//[]----------------------------------------------------------------------[]
/// @return			none
//[]----------------------------------------------------------------------[]
///	@auther			MATSUSHITA
///	@date			Create	: 2013/04/02
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
void	TimeAdjustInit(void)
{
	t_SYSTIME_ADJ	*ctr = &SysTimeAdj;

	memset(ctr, 0, sizeof(*ctr));
	if (_is_ntnet_remote()) {
		if (prm_get(COM_PRM, S_NTN, 122, 1, 6) == 0 &&	// ����������Ԃ� �u0=��������v * ���� ��������F0�A�������Ȃ��F1 * ����
			prm_get(COM_PRM, S_NTN, 121, 1, 1) == 1 ){	// ���ԑ䐔�f�[�^�̌`�� �� �f�[�^�`��=1(ParkingWeb)
			ctr->mode = 1;
		}
	}

	ctr->method = 1;		// ��荇�����u�w�莞���v�̂�

	ctr->req_adj = 2;		// �N�����͕␳����
	ctr->state = _T_WATCH;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����␳����
//[]----------------------------------------------------------------------[]
///	@param[in]		req = 0(SNTP�L���E����)�A1(�ꎞ��~�v���j
//						  2(��~����)�A3(��~���������Z�b�g)
/// @return			1=OK
//[]----------------------------------------------------------------------[]
///	@auther			MATSUSHITA
///	@date			Create	: 2013/04/02
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
int		TimeAdjustCtrl(ushort req)
{
	t_SYSTIME_ADJ	*ctr = &SysTimeAdj;

	switch(req) {
	case	0:
	// SNTP�@�\�L���E����
		return (ctr->mode == 0)? 0 : 1;
	case	1:
	// ��~�v��
		ctr->stop = 1;
		if (ctr->state == _T_BUSY) {
			ctr->state = _T_WATCH;
			Lagcan(OPETCBNO, _TIMERNO_SNTP);
			RAU_SNTPStart( 0 , SNTPMSG_RECV_TIME, 0, 0 );	// �␳���s�t���O�𗎂Ƃ�
		}
		ctr->reqcnt = 0;		// �蓮�␳�p�ɗv���J�E���^���N���A
		break;
	case	2:
	// ��~�����v��
		if (ctr->stop != 0) {
			ctr->stop = 0;
		}
		break;
	case	3:
	// ��~���������Z�b�g�v��
		if (ctr->stop != 0) {
			ctr->stop = 0;
			ctr->req_adj = 0;
			ctr->state = _T_WATCH;
		}
		break;
	default:
		return 0;
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�o�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		exec = ���s����
/// @return			none
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Update	: 2013/04/23
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
void	RegistReviseLog(ushort exec)
{
}

//[]----------------------------------------------------------------------[]
///	@brief			SNTP�����␳�v������
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		:	receive event message
/// @return			none
//[]----------------------------------------------------------------------[]
///	@auther			MATSUSHITA
///	@date			Create	: 2013/04/02
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
void	ReqServerTime(ushort msg)
{
	t_SYSTIME_ADJ	*ctr = &SysTimeAdj;
	struct _ntp_time_	tm;
	ushort wt;

	ctr->skip_adj = 0;		// ���̎����`�F�b�N�͗L��

	// �v������
	memset(&ctr->log, 0, sizeof(ctr->log));
	if (ctr->stop != 0) {
		ctr->log.type = 0;
	}
	else {
		ctr->log.type = ctr->req_adj;
	}
	ctr->log.reqno = ++ctr->reqcnt;
	c_Now_CLK_REC_ms_Read(&ctr->tmpClk, &ctr->tmpMsec);
	ctr->log.Req_Date.Year = ctr->tmpClk.year;
	ctr->log.Req_Date.Mon  = ctr->tmpClk.mont;
	ctr->log.Req_Date.Day  = ctr->tmpClk.date;
	ctr->log.Req_Date.Hour = ctr->tmpClk.hour;
	ctr->log.Req_Date.Min  = ctr->tmpClk.minu;
	ctr->log.Req_Date.Sec  = ctr->tmpClk.seco;

	// �擾�v��
	tm.msec = ctr->tmpMsec;
	tm.sec = sntp_get_nowsec(&ctr->tmpClk);
	RAU_SNTPStart( 1 , msg, tm.sec, tm.msec );
	wt = _SNTP_RESPONSE_TOUT;

	// �^�C���A�E�g���Ԑݒ�
	wt *= 50;		// to 20msec/LSB
	Lagtim(OPETCBNO, _TIMERNO_SNTP, wt);
}

//[]----------------------------------------------------------------------[]
///	@brief			sntpGetTime(get NTP Time)
//[]----------------------------------------------------------------------[]
///	@param[in]		time	: date_time_rec2
///	@return			0:ok, -1:bad data, -2:reject
///	@attention		None
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007/08/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
int		sntpGetTime(date_time_rec2 *time, ushort *net_msec)
{
	long	offset;
	ulong	ltime;
	ushort	year, month;
	ulong	d;
	struct _ntp_time_	tm[4];
	ttNtpPacket		pkt;
	t_SYSTIME_ADJ	*ctr = &SysTimeAdj;

	// PPP���ڑ��H
	if (KSG_gPpp_RauStarted == 0) {
		return -2;
	}

	// SNTP��M�p�P�b�g�擾
	RAU_SNTPGetPacket(&pkt);

	if (pkt.sntpVersionNember == 0) {
		RegistReviseLog(SNTP_BADDATA);
		return -1;			// VN == 0
	}
	if (pkt.sntpLeapId == 3) {
		RegistReviseLog(SNTP_BADDATA);
		return -1;			// LI==3 : unsynchronized
	}
	if (pkt.sntpStatum == 0) {
		RegistReviseLog(SNTP_BADDATA);
		return -1;			// Startum == 0 : unspecified or unavailable(SNTPv3)
	}
	if (pkt.transmitTimestampSeconds == 0 &&
		pkt.transmitTimestampFractions == 0) {
		RegistReviseLog(SNTP_BADDATA);
		return -1;			// unsynchronized
	}

	c_Now_CLK_REC_ms_Read(&ctr->tmpClk, &ctr->tmpMsec);
	offset = RAU_TIMEZONE;
	sntp_get_seconds(&pkt.transmitTimestampSeconds, &tm[2]);	// T3
	ltime = tm[2].sec;
	*net_msec = tm[2].msec;

	*net_msec += (ushort)prm_get(COM_PRM, S_NTN, 123, 4, 1);	// �Z���^�[SNTP �덷�␳
	if (*net_msec >= 1000) {
		*net_msec %= 1000;
		ltime++;
	}

	if (pkt.originateTimestampHigh != 0) {
	// �v�������M���Ԃ��聁�␳����
		tm[0].sec = pkt.originateTimestampHigh;					// T1
		tm[0].msec = (ushort)(pkt.originateTimestampLow / RAU_DECIMAL_32BIT);	// �~���b�P�ʂɕϊ�
		sntp_get_seconds(&pkt.receiveTimestampHigh, &tm[1]);	// T2
		tm[3].msec = ctr->tmpMsec;
		tm[3].sec = sntp_get_nowsec(&ctr->tmpClk);				// T4
		tm[3].sec += TM_JAN_1970;								// Set UTC time is reckoned from 0h 0m 0s UTC on 1 January 1900.
	// d = (T4 - T1) - (T3 - T2) �Ƃ��āAd/2���T�[�o�[���Ԃɉ��Z����
		time_subtraction(&tm[3], &tm[0]);		// T4 - T1
		time_subtraction(&tm[2], &tm[1]);		// T3 - T2
		time_subtraction(&tm[3], &tm[2]);		// (T4 - T1) - (T3 - T1) = d
		ctr->log.d = tm[3].sec;					// ���O�p�ɕۑ�
		ctr->log.d *= 1000L;
		ctr->log.d += tm[3].msec;
		tm[3].msec >>= 1;
		if (tm[3].sec & 1) {
			tm[3].msec += 500;
		}
		tm[3].sec >>= 1;						// = d / 2
		ltime += tm[3].sec;						// ���[�g�x���������Z
		*net_msec += tm[3].msec;
		if (*net_msec >= 1000) {
			*net_msec -= 1000;
			ltime++;
		}
	}
	ltime += offset;		// to local time

	time->Sec = (ushort)(ltime % 60);
	ltime /= 60;
	time->Min = (uchar)(ltime % 60);
	ltime /= 60;
	time->Hour = (uchar)(ltime % 24);
	ltime /= 24;

	for (year = 1970; ; year++) {
		d = (year & 3) ? 365 : 366;
		if (ltime < d){
			break;
		}
		ltime -= d;
	}
	for (month = 1; month < 12; month++) {
		d = ((month != 2) || (year & 3)) ? days[month] : 29;
		if (ltime  < d){
			break;
		}
		ltime -= d;
	}
	time->Year = year;
	time->Mon  = (uchar)month;
	time->Day  = (uchar)(1+ltime);

	if (time->Year < 2000 || time->Year > 2050) {
		RegistReviseLog(SNTP_BADDATA);
		return -1;		// �N���L���͈́i2000�`2050�j�O
	}
	if (ctr->log.d > _SNTP_d_LIMIT) {
		return (ctr->reqcnt < _SNTP_REQ_MAX)? 1 : 2;
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			���Ԑ��K��
//[]----------------------------------------------------------------------[]
///	@param[in]		hh		: ��
///	@param[in]		mm		: ��
///	@param[in]		ss		: �b
///	@return			�ʎZ�b
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/07/31
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
static
ulong	tnomlz_ex(uchar hh, uchar mm, uchar ss)
{
	ulong	seconds;
	seconds = (ulong)hh;
	seconds *= 60;
	seconds += mm;
	seconds *=60;
	seconds += ss;
	return seconds;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����덷�v�Z
//[]----------------------------------------------------------------------[]
///	@param[in]		*dt			: �T�[�o�[����
///	@param[in]		net_msec	: �T�[�o�[����(msec)
///	@param[out]		*margin		: �덷
///	@return			1
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/07/31
///	@date			Update	: 2013/04/02
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
int		CheckTimeMargin(date_time_rec2 *dt, ushort net_msec, ulong margin[])
{
	t_SYSTIME_ADJ	*ctr = &SysTimeAdj;
	ulong	now[2], net[2];

	now[0] = (ulong)dnrmlzm((short)ctr->tmpClk.year, (short)ctr->tmpClk.mont, (short)ctr->tmpClk.date);
	now[1] = tnomlz_ex(ctr->tmpClk.hour, ctr->tmpClk.minu, ctr->tmpClk.seco);
	now[1] *= 1000;
	now[1] += ctr->tmpMsec;
	net[0] = (ulong)dnrmlzm((short)dt->Year, (short)dt->Mon, (short)dt->Day);
	net[1] = tnomlz_ex(dt->Hour, dt->Min, (uchar)dt->Sec);
	net[1] *= 1000;
	net[1] += net_msec;

	margin[2] = 0;
	if (net[0] > now[0]) {
		margin[0] = net[0] - now[0];
		margin[1] = net[1] - now[1];
		if ((long)margin[1] < 0) {
			margin[1] += DAY_MSEC;
			margin[0]--;
		}
	}
	else if (net[0] < now[0]) {
		margin[0] = now[0] - net[0];
		margin[1] = now[1] - net[1];
		if ((long)margin[1] < 0) {
			margin[1] += DAY_MSEC;
			margin[0]--;
		}
		margin[2] = 1;
	}
	// if now[0] == net[0]
	else if (net[1] >= now[1]) {
		margin[0] = 0;
		margin[1] = net[1] - now[1];
	}
	else {
		margin[0] = 0;
		margin[1] = now[1] - net[1];
		margin[2] = 1;
	}
// ���O�p�ɕۑ�	
	if (margin[0] != 0) {
	// 1���ȏ�̌덷����
		ctr->log.t = DAY_MSEC;
	}
	else {
		ctr->log.t = margin[1];
	}
	if (margin[2] != 0) {
		ctr->log.t = -ctr->log.t;
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����덷���Z
//[]----------------------------------------------------------------------[]
///	@param[in]		*margin		: �덷
///	@param[out]		*in_d		: �␳�㎞��
///	@return			msec remained
//[]----------------------------------------------------------------------[]
///	@author			MATSUSHITA
///	@date			Create	: 2008/07/31
///	@date			Update	: 2013/04/02
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
ushort	AddTimeMargin(ulong margin[], short in_d[])
{
	t_SYSTIME_ADJ	*ctr = &SysTimeAdj;
	ulong	add[3];

	c_Now_CLK_REC_ms_Read(&ctr->tmpClk, &ctr->tmpMsec);
	add[0] = (ulong)dnrmlzm((short)ctr->tmpClk.year, (short)ctr->tmpClk.mont, (short)ctr->tmpClk.date);
	add[1] = tnomlz_ex(ctr->tmpClk.hour, ctr->tmpClk.minu, ctr->tmpClk.seco);
	add[1] *= 1000;
	add[1] += ctr->tmpMsec;

	if (margin[2] == 0) {
	// server time >= client time
		add[0] += margin[0];
		add[1] += margin[1];
		if (add[1] >= DAY_MSEC) {
			add[1] -= DAY_MSEC;
			add[0]++;
		}
	}
	else {
	// server time < client time
		add[0] -= margin[0];
		add[1] -= margin[1];
		if ((long)add[1] < 0) {
			add[1] += DAY_MSEC;
			add[0]--;
		}
	}

	add[2] = add[1] % 1000;			// msec
	if (add[2] != 0) {
		add[2] = 1000 - add[2];		// msec�͐؂�グ��̂Ŏc���Ԃ�Ԃ�
		add[1] += 999;				// msec�؂�グ
	}
	add[1] /= 1000;
	if (add[1] >= DAY_SEC) {
		add[1] -= DAY_SEC;
		add[0]++;
	}
	in_d[5] = (short)(add[1] % 60);		// sec
	add[1] /= 60;
	in_d[1] = (short)(add[1] % 60);		// min
	in_d[0] = (short)(add[1] / 60);		// hour
	idnrmlzm((ushort)add[0], &in_d[2], &in_d[3], &in_d[4]);		// yy-mm-dd

	// ���X�|���X����
	ctr->log.Rsp_Date.Year = (ushort)in_d[2];
	ctr->log.Rsp_Date.Mon  = (uchar)in_d[3];
	ctr->log.Rsp_Date.Day  = (uchar)in_d[4];
	ctr->log.Rsp_Date.Hour = (uchar)in_d[0];
	ctr->log.Rsp_Date.Min  = (uchar)in_d[1];
	ctr->log.Rsp_Date.Sec  = (ushort)in_d[5];

	return (ushort)add[2];
}

//[]----------------------------------------------------------------------[]
///	@brief			���������␳
//[]----------------------------------------------------------------------[]
/// @return			0=�ėv��, 1=�␳����, 2=�␳�Ȃ�, <0=�G���[
//[]----------------------------------------------------------------------[]
///	@auther			MATSUSHITA
///	@date			Create	: 2013/04/02
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
static
int		calc_revise_time(void)
{
	t_SYSTIME_ADJ	*ctr = &SysTimeAdj;
	short			date[6];
	ulong			margin[3];	// [0]=day, [1]=msec, [2]=sign
	ushort			net_msec;
	ulong			gap;	// �덷�̐�Βl(�b)
	date_time_rec2	tm;
	int				ret;
	ushort			exec;

// �T�[�o�[�����擾
	ret = sntpGetTime(&tm, &net_msec);
	if (ret < 0) {
		return ret;		// NG
	}

// �X�V�ł��Ȃ�����(23:59:50�`59)�H
	if (CLK_REC.nmin == (23*60+59) && CLK_REC.seco >= 50) {
		RegistReviseLog(SNTP_GUARD_TIME);		// ���O�o�^
		return -2;		// ��蒼��
	}

	err_chk2(ERRMDL_MAIN, ERR_MAIN_SNTP_FAIL, 0, 0, 0, 0);
	wmonlg(OPMON_TIME_AUTO_REVISE, 0 , 0);		// ���j�^�o�^

	CheckTimeMargin(&tm, net_msec, margin);		// �����덷�Z�o
	if (ret == 1) {
	// �����ɒx������
		RegistReviseLog(SNTP_DELAYED_RETRY);	// ���O�o�^
		return 0;
	}
	net_msec = AddTimeMargin(margin, date);		// �����덷�����Z

	if (margin[0] != 0) {
	// 1���ȏ�̌덷����
		gap = DAY_MSEC;
	} else {
		gap = margin[1];						// �b�덷�Z�o
	}

	// �덷����
	if (gap < _SNTP_LOWER_LIMIT) {
		exec = SNTP_NORMAL_SKIP;
	}
	else {
		exec = SNTP_NORMAL_REVISE;

		// �덷���
		gap /= 1000L;
		if (gap > _SNTP_UPPER_LIMIT) {
			exec = SNTP_LIMIT_OVER;
		}
	}
	RegistReviseLog(exec);						// ���O�o�^

	if (exec != SNTP_NORMAL_REVISE) {
		return 2;
	}

	// msec�����҂�
	ctr->state = _T_LAG;

	// �����␳�����^�C�}
	if (net_msec > 0) {
		ctr->lag_count = net_msec;				// �~���b�^�C�}�փZ�b�g
	}
	else {
		TimeAdjustMain(SNTPMSG_AUTO_SET);		// �����ύX
	}

	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����ύX
//[]----------------------------------------------------------------------[]
/// @return			none
//[]----------------------------------------------------------------------[]
///	@auther			T.Nagai
///	@date			Create	: 2015/02/25
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void	clk_auto_set(void)
{
	t_SYSTIME_ADJ	*ctr = &SysTimeAdj;

	ctr->tmpClk.year = ctr->log.Rsp_Date.Year;
	ctr->tmpClk.mont = (char)ctr->log.Rsp_Date.Mon;
	ctr->tmpClk.date = (char)ctr->log.Rsp_Date.Day;
	ctr->tmpClk.hour = (char)ctr->log.Rsp_Date.Hour;
	ctr->tmpClk.minu = (char)ctr->log.Rsp_Date.Min;
	ctr->tmpClk.seco = (char)ctr->log.Rsp_Date.Sec;
	ctr->tmpClk.ndat = dnrmlzm((short)ctr->tmpClk.year, (short)ctr->tmpClk.mont, (short)ctr->tmpClk.date);
	ctr->tmpClk.nmin = tnrmlz ((short)0, (short)0, (short)ctr->tmpClk.hour, (short)ctr->tmpClk.minu);
	ctr->tmpClk.week = (unsigned char)((ctr->tmpClk.ndat + 6) % 7);
	Ope_clk_set( &ctr->tmpClk, OPLOG_SET_TIME2_SNTP );		// ���v�X�V

	if (ctr->log.t < 0 && ctr->log.t > -60000) {
	// �␳��-59�b�܂ł̊ԂȂ玟�̕����i�𖳎�����
		ctr->skip_adj = 1;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�����␳���C��
//[]----------------------------------------------------------------------[]
///	@param[in]		msg:	CLOCK_CHG=�␳��������
///							SNTPMSG_RECV_AUTORES=SNTP����
///							TIMEOUT_SNTP=SNTP�^�C���A�E�g
/// @return			none
//[]----------------------------------------------------------------------[]
///	@auther			MATSUSHITA
///	@date			Create	: 2013/04/02
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
void	TimeAdjustMain(ushort msg)
{
	t_SYSTIME_ADJ	*ctr = &SysTimeAdj;
	int		ret;
	int		match;
	ushort	time;

// �����i�ɏ]���Ď����̎����␳���Ǘ�����
	if (ctr->mode == 0) {
		return;
	}
	switch (msg) {
	case CLOCK_CHG:
	// �����i
		if (ctr->state == _T_DELAY) {
		// �x����
			if (--ctr->delay == 0) {
				ctr->state = _T_WATCH;
			}
		}
		if (ctr->method == 1) {
		// �w�莞��
			match = 0;
			time = (ushort)prm_tim(COM_PRM, S_NTN, 122);
			if (CLK_REC.nmin == time) {
				match = 1;
			}
			if (ctr->skip_adj != 0) {
				match = 0;
				ctr->skip_adj = 0;
			}
			if (match != 0) {
			// �␳���J�n
				if (ctr->state == _T_DELAY) {
					ctr->state = _T_WATCH;		// ���g���C���ł��V�K�ɊJ�n����
				}
				ctr->waicnt = 0;				// ���g���C����␳���ł��h�P��ځh�ɖ߂�
				ctr->trycnt = 0;
				ctr->req_adj = 1;
			}
		}

		if (ctr->state == _T_WATCH) {
		// �Ď���
			if (ctr->req_adj != 0 &&
				ctr->stop == 0) {
			// �␳�v������ && �J�n��
				ctr->state = _T_BUSY;		// �����␳�J�n
				ctr->reqcnt = 0;
				ReqServerTime(SNTPMSG_RECV_AUTORES);
			}
		}
		break;

	case SNTPMSG_RECV_AUTORES:
	// SNTP����
		if (ctr->state == _T_BUSY) {
			Lagcan(OPETCBNO, _TIMERNO_SNTP);
			ret = calc_revise_time();			// �덷�v�Z
			if (ret < 0) {
			// ���s
				if (ret == -2) {
				// SNTP�v�����s
					if (++ctr->waicnt <= _SNTP_WAI_MAX) {
						ctr->state = _T_WATCH;			// ���̕����i�ł�蒼��
						break;
					}
				}
			// ��M�f�[�^�G���[
				ctr->reqcnt = 0xff;
				ctr->waicnt = 0;
				Lagtim(OPETCBNO, _TIMERNO_SNTP, 1);		// ���̉����𖳎�
			}
			else if (ret == 0) {
			// �����ɒx��������
				ReqServerTime(SNTPMSG_RECV_AUTORES);	// ���̏�ōėv��
			}
			else if (ret == 2) {
			// �␳�Ȃ�
				ctr->state = _T_WATCH;
				ctr->req_adj = 0;
				ctr->waicnt = 0;
				ctr->trycnt = 0;
			}
		}
		break;

	case TIMEOUT_SNTP:
	// SNTP�^�C���A�E�g
		if (ctr->state == _T_BUSY) {
			RegistReviseLog(SNTP_TIMEOUT);		// ���O�o�^
			RAU_SNTPStart( 0 , SNTPMSG_RECV_AUTORES, 0, 0 );	// ���s �␳���s�t���O�𗎂Ƃ�
			if (ctr->reqcnt < _SNTP_REQ_MAX) {
			// �����̏ꍇ�̓^�C���A�E�g�ł��ėv������
				ReqServerTime(SNTPMSG_RECV_AUTORES);
				break;
			}
		// ���s�m��
			err_chk2(ERRMDL_MAIN, ERR_MAIN_SNTP_FAIL, 1, 0, 0, 0);
			ctr->trycnt++;
			if (ctr->trycnt <= _SNTP_RETRY_MAX) {
				ctr->delay = _SNTP_RETRY_INT;
				ctr->state = _T_DELAY;
			}
			else {
			// �␳���s
				ctr->state = _T_WATCH;
				ctr->req_adj = 0;
				ctr->waicnt = 0;
				ctr->trycnt = 0;
			}
		}
		break;

	case SNTPMSG_AUTO_SET:
		if (ctr->state == _T_LAG) {
			ctr->lag_count = -1;
			clk_auto_set();

			// �X�e�[�^�X������
			ctr->state = _T_WATCH;
			ctr->req_adj = 0;
			ctr->waicnt = 0;
			ctr->trycnt = 0;
		}
		break;

	default:
		break;
	}
}

// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^�j
//[]----------------------------------------------------------------------[]
///	@brief			FTP�Z�b�V�����I�[�v��
//[]----------------------------------------------------------------------[]
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/11/15<br>
///					Update	:	
/// @note
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void ftp_new_session_for_LCD( void )
{
	ftpctrl.ftpHandle = tfFtpNewSession(0,
									TM_BLOCKING_OFF,
									ftpctrl.loginbuff[0],
									ftpctrl.loginbuff[1]);
	if( !ftpctrl.ftpHandle ){
	}else{
		ftpctrl.start = FTP_NEW_SESSION;
		Lagtim(OPETCBNO, 7, FTP_TIMEOUT);	// ��ϰ�Ď�����(1s)
		_SET_WATCH_TIMER();
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP�ڑ��v��
//[]----------------------------------------------------------------------[]
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/11/15<br>
///					Update	:	
/// @note
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void ftp_connect_for_LCD( void )
{
	int		errorCode;
	int		i;
	struct sockaddr_storage ss;

	typedef union {
		ulong			uladdr;
		uchar			ucaddr[4];
	} u_ipaddr;
	u_ipaddr tempIP;

	// �ڑ����ݒ�
	memset( &ss, 0, sizeof(ss) );
	ss.ss_len = sizeof(struct sockaddr_in);
	ss.ss_family = AF_INET;
	ss.ss_port = prm_get( COM_PRM, S_PKT, 30, 5, 1 );
	for( i=0; i<4; i++ ){
		tempIP.ucaddr[i] = prm_get( COM_PRM, S_PKT, 1 + i, 3, 1 );
	}
	ss.addr.ipv4.sin_addr.s_addr = tempIP.uladdr;

	errorCode = tfNgFtpConnect( ftpctrl.ftpHandle, &ss );	// �ڑ��v��
	switch( errorCode ){
		case TM_ENOERROR:						// ����
		case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
			ftpctrl.start = FTP_CONNECT_LCD;
			_SET_WATCH_TIMER();
			break;
		default:
			ftpctrl.start = FTP_STOP;
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_login_lcd(���O�C�����M�j
//[]----------------------------------------------------------------------[]
///	@param[in]		res	: ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int ftp_send_login_for_LCD( int res )
{
	int	errorCode = res;
	
	if(( ftpctrl.result & 0x8000 )){
		return errorCode;
	}
	if( ftpctrl.result != AU_LOGIN_ERR ){
		switch( errorCode ){
			// FTP�ڑ������H
			case TM_ENOERROR:
				// FTP���O�C�����s
				errorCode = tfFtpLogin( ftpctrl.ftpHandle,
										ftpctrl.loginbuff[0],
										ftpctrl.loginbuff[1],
										NULL );
				switch( errorCode ){
					case TM_ENOERROR:						// FTP���O�C������
						ftpctrl.login = 1;
						// no break
					case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
						ftpctrl.start = FTP_LOGIN_LCD;
						_SET_WATCH_TIMER();
						break;
					default:
						ftpctrl.result = AU_LOGIN_ERR | 0x8000;
						break;
				}
				break;

			// FTP�ڑ����������Ă��Ȃ�
			case TM_EWOULDBLOCK:
				//_SET_WATCH_TIMER();
				break;

			default:
				ftpctrl.result = AU_LOGIN_ERR | 0x8000;
				break;
		}
	}

	// ���O�C�����s�H
	if( ftpctrl.result == (AU_LOGIN_ERR| 0x8000) ){
		if( errorCode == TM_ESHUTDOWN ){
			remotedl_result_set(CONN_TIMEOUT_ERR);	// �ڑ��^�C���A�E�g
		}
		else {
			remotedl_result_set( LOGIN_ERR );		// ���O�C�����s
		}
		ftpctrl.start = FTP_CLOSE_REQ;
	}
	// ���O�C�������H
	else if( ftpctrl.result == TM_ENOERROR && ftpctrl.login == 1 ){
// GG120600(S) // Phase9 LCD�p��FTP�ł͉��u�Ď����j�^�͎c���Ȃ�
//		rmon_regist( RMON_FTP_LOGIN_OK );	// ���O�C������
// GG120600(E) // Phase9 LCD�p��FTP�ł͉��u�Ď����j�^�͎c���Ȃ�
	}

	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_send_type_cmd(�]���^�C�v�w��j
//[]----------------------------------------------------------------------[]
///	@param[in]		None
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int ftp_send_type_cmd_for_LCD( void )
{
	int errorCode;

	// �]���^�C�v�w��(TYPE�R�}���h���s)
	errorCode = tfFtpType( ftpctrl.ftpHandle, TM_TYPE_BINARY );
	switch( errorCode ){
		case TM_ENOERROR:
		case TM_EWOULDBLOCK:
		case TM_FTP_XFERSTART:
		case TM_FTP_FILEOKAY:
			errorCode = OK;
			break;
		default:
			errorCode = NG;
			break;
	}

	// TYPE�R�}���h���s�H
	if( errorCode != OK ){
		ftpctrl.start = FTP_QUITE_REMOTE_REQ;
		if( errorCode == TM_ENOTLOGIN || ftpctrl.login == -1 ){
			ftpctrl.result = AU_LOGIN_ERR;
		}
		else {
			ftpctrl.result = AU_DISCONNECT;
		}
	}
	// TYPE�R�}���h�����H
	else {
		ftpctrl.start = FTP_TYPE_REQ_LCD;
		_SET_WATCH_TIMER();
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_login_wait_lcd(���O�C���҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res : ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int ftp_login_wait_for_LCD( int res )
{
	
	int		errorCode = res;
	uchar	status = remotedl_status_get();
	
	switch( errorCode ){

		// ���O�C�������I
		case TM_ENOERROR:
			if( ftpctrl.login != 1){
				ftpctrl.login = 1;
// GG120600(S) // Phase9 LCD�p��FTP�ł͉��u�Ď����j�^�͎c���Ȃ�
//				rmon_regist( RMON_FTP_LOGIN_OK );	// ���O�C������
// GG120600(E) // Phase9 LCD�p��FTP�ł͉��u�Ď����j�^�͎c���Ȃ�
			}
// GG120600(S) // Phase9 LCD�p��FTP�ł͉��u�Ď����j�^�͎c���Ȃ�
//			retry_info_clr( RETRY_KIND_CONNECT );	// �ڑ����g���C�񐔃N���A
// GG120600(E) // Phase9 LCD�p��FTP�ł͉��u�Ď����j�^�͎c���Ȃ�
			// TYPE�R�}���h���s
			ftp_send_type_cmd_for_LCD();
			break;

		// FTP���O�C�����������Ă��Ȃ�
		case TM_EWOULDBLOCK:
			_SET_WATCH_TIMER();
			break;

		default:
			ftpctrl.start = FTP_QUITE_WAIT;
			ftpctrl.result = AU_LOGIN_ERR;
			remotedl_monitor_info_set(5);
			remotedl_result_set( LOGIN_ERR );		// ���O�C�����s
			break;
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_type_wait(�]���^�C�v�ݒ艞���҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res : ���X�|���X(���g�p�j
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int ftp_type_wait_for_LCD( int res )
{
	int errorCode = res;

	switch( errorCode ){
		// TYPE�R�}���h����
		case TM_ENOERROR:
			memset( &cmd, 0, sizeof(cmd) );									// �R�}���h�G���A������
			MakeRemoteFileName( cmd.remote, cmd.local, MAKE_FILENAME_PARAM_UP_FOR_LCD );	// �t�@�C�����쐬
			SetSnd_RecvStatus( FCMD_REQ_SEND );
			// �R�}���h���M(�A�b�v���[�h�v��)������
			if( ftp_send_req_cmd(&cmd) == OK ){
				ftpctrl.start = FTP_PARAM_WAIT_LCD;			// �A�b�v�����҂�
				errorCode = TM_ENOERROR;
			}
			// �R�}���h���s
			else{
				ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			}
			break;

		case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
		case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
		case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
			_SET_WATCH_TIMER();
			break;

		default:
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			remotedl_monitor_info_set(15);
			remotedl_result_set(PROG_DL_NONE);			// �A�b�v���[�h���s
			break;
	}
	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_param_wait_for_LCD(�p�����[�^���M�����҂��j
//[]----------------------------------------------------------------------[]
///	@param[in]		res : ���X�|���X
///	@return			�G���[�R�[�h
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int ftp_param_wait_for_LCD( int res )
{
	int errorCode = res;

	if( !( ftpctrl.result & 0x8000 ) ){
		switch( errorCode ){
		case TM_ENOERROR:						// ����
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
			break;
		case TM_EWOULDBLOCK:					// �I�y���[�V�������������Ă��Ȃ�
		case TM_FTP_XFERSTART:					// �f�[�^�R�l�N�V�����͂��łɊm������Ă���
		case TM_FTP_FILEOKAY:					// �t�@�C���X�e�[�^�X�͐���B�f�[�^�R�l�N�V�������m������
			_SET_WATCH_TIMER();
			break;
		case TM_FTP_NOTLOGIN:					// ���O�C������Ă��Ȃ�
		case TM_ENOTLOGIN:						// ���[�U�͌��݃��O�C������Ă��Ȃ�
			ftpctrl.result = (AU_LOGIN_ERR | 0x8000);
			break;
		case TM_FTP_FILENAVAIL:					// �v�����ꂽ�t�@�C������̓A�N�Z�X������t�@�C���V�X�e���̗��R�Ŏ��s�ł��Ȃ�
		case TM_FTP_XFERABOR:					// 
			ftpctrl.result = (SCRIPT_FILE_NONE | 0x8000);
			break;
		default:
			ftpctrl.result = (AU_DISCONNECT | 0x8000);
			break;
		}
		if( ( ftpctrl.result & 0x8000 ) ){
			ftpctrl.start = FTP_QUITE_REMOTE_REQ;
// GG120600(S) // Phase9 LCD�p��FTP�ł͉��u�Ď����j�^�͎c���Ȃ�
//			rmon_regist(RMON_FTP_FILE_TRANS_NG);				// �]�����s
// GG120600(E) // Phase9 LCD�p��FTP�ł͉��u�Ď����j�^�͎c���Ȃ�
		}
	}

	return errorCode;
}

//[]----------------------------------------------------------------------[]
///	@brief			ftp_output_dat_param(bin�`���Ńp�����[�^�o��)
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer
///	@param[in]		nSize	: buffer�̃T�C�Y
///	@return			send length
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ulong ftp_output_dat_param( char *buff, int nSize )
{
	ushort sec, secMax, itemMax;
	ulong	len, total = 0;
	ulong	*pSecTop;
	t_AppServ_ParamInfoTbl	*param_tbl;

	if( ftpdata.send.step == 1 ){
		param_tbl = (void*)FLT_GetParamTable();
		AppServ_ConvParam2CSV_Init( param_tbl, SYSMNT_DATALEN_MAX );

		secMax = AppServ_BinCsvSave.param_info.csection_num;
		for( sec=0; sec<secMax; sec++ ){
			WACDOG;
			itemMax = AppServ_BinCsvSave.param_info.csection[sec].item_num;
			if( !itemMax ){
				continue;
			}
			len = (sizeof(long) * itemMax);
			// �o�b�t�@�󂫂���H
			if( total + len < nSize ){
				pSecTop = (ulong*)AppServ_BinCsvSave.param_info.csection[sec].address;
				memcpy( &buff[total], &pSecTop[0], len );
				total += len;
			}
			// �o�b�t�@�󂫂Ȃ��H
			else{
				break;	// �I��
			}
		}
		ftpdata.send.step = 2;
	}

	return total;
}

//[]----------------------------------------------------------------------[]
///	@brief			LCD���W���[���ɑ΂���p�����[�^�A�b�v���[�h
//[]----------------------------------------------------------------------[]
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/11/15<br>
///					Update	:	
/// @note
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
int ftp_auto_update_for_LCD( void )
{
	ushort	msg;
// GG120600(S) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
	uchar bkType = CTRL_NONE;
// GG120600(E) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
// GG129000(S) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
	uchar bkScriptType = remotedl_script_typeget();	// �X�N���v�g�t�@�C����ʃo�b�N�A�b�v
// GG129000(E) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]

	remotedl_script_typeset( LCD_NO_SCRIPT );	// �X�N���v�g�t�@�C����ʃZ�b�g

	ftp_init( _FTP_CLIENT, OPETCBNO );

	// ���O�C�����[�U�[�A�p�X���[�h�Z�b�g�iLCD�̃��[�U�[/�p�X���[�h��RXA�Ɠ����j
	GetFTPLoginPass( LCD_LOGIN, ftpctrl.loginbuff[0], ftpctrl.loginbuff[1] );

	Lagtim( OPETCBNO, 7, FTP_TIMEOUT );			// ��ϰ�Ď�����(1s)

	// FTP�Z�b�V�����쐬
	ftp_new_session_for_LCD();
	if( ftpctrl.start != FTP_NEW_SESSION ){
// GG129000(S) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
		remotedl_script_typeset(bkScriptType);	// �X�N���v�g�t�@�C����ʃ��X�g�A
// GG129000(E) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
		// ���O�C�����s
		 return AU_LOGIN_ERR;
	}

	// FTP�ڑ�
	ftp_connect_for_LCD();
	if( ftpctrl.start != FTP_CONNECT_LCD ){
// GG120600(S) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
		if(ftpctrl.ftpHandle){
			tfFtpFreeSession(ftpctrl.ftpHandle );
			ftpctrl.ftpHandle = 0;
		}
// GG120600(E) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
// GG129000(S) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
		remotedl_script_typeset(bkScriptType);	// �X�N���v�g�t�@�C����ʃ��X�g�A
// GG129000(E) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
		// ���O�C�����s
		return AU_LOGIN_ERR;
	}
// GG120600(S) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
	bkType = remotedl_connect_type_get();
// GG120600(E) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
// GG120600(S) // Phase9 LCD�p
	remotedl_connect_type_set( CTRL_PARAM_UPLOAD_LCD );
// GG120600(E) // Phase9 LCD�p

	FAIL_SAFE_TIMER_START( GetTaskID(), GetFailSafeTimerNo() );	// OPE�^�X�N�ł�FailSafeTimerNo = 19
	INTERVAL_TIMER_START( TIMEOUT25 );			// �C���^�[�o���^�C�}�[�X�^�[�g

	while( ftpctrl.start ){
		msg = StoF( GetMessage(), 1 );
		switch( msg ){
			case TIMEOUT7:
				if( ftp_timer() != -1 ){
					if( ftpctrl.start != FTP_STOP ){
						// �^�C���A�E�g���̏������s���B
						if( ftpctrl.result == AU_ABORT_END ){
							ftp_send_cancel(0);
						}else{
							ftp_send_quit(0);
						}
					}
				}
				Lagtim( OPETCBNO, 7, FTP_TIMEOUT );	// ��ϰ�Ď�����(1s)
				break;

			// �C���^�[�o���^�C�}�[
			case TIMEOUT25:
				FTP_Execute();
				INTERVAL_TIMER_START( TIMEOUT25 );			// �C���^�[�o���^�C�}�[�X�^�[�g
				break;

			case TIMEOUT19:
				ftp_send_quit(0);
				break;
		}
	}

	Lagcan(OPETCBNO, 7);
	Lagcan(OPETCBNO, GetFailSafeTimerNo());
	Lagcan(OPETCBNO, 25);

	ftp_exit();
// GG120600(S) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
//// GG120600(S) // Phase9 LCD�p
//	remotedl_connect_type_set(CTRL_NONE);
//// GG120600(E) // Phase9 LCD�p
	remotedl_connect_type_set(bkType);
// GG120600(E) // Phase9 #6183 �ݒ�A�b�v���[�h��FTP�ڑ����ɓd�f����ƁA�ċN����Ƀ����e�i���X���g���Ȃ��Ȃ�
// GG129000(S) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
	remotedl_script_typeset(bkScriptType);	// �X�N���v�g�t�@�C����ʃ��X�g�A
// GG129000(E) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]

	return ftpctrl.result;
}

// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
	
// MH810100(S)
//[]----------------------------------------------------------------------[]
///	@brief			FTP�t���O
//[]----------------------------------------------------------------------[]
///	@param			req�FFTP�v���t���O
///	@return			None
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void mnt_SetFtpFlag( eFTP_REQ_TYPE req )
{
// GG120600(S) // Phase9 LCD�p
//	g_FtpRequestFlg = req;
	switch(req){
	case FTP_REQ_NONE:			// 0:�v���Ȃ�
	case FTP_REQ_NORMAL:		// 1:FTP�v������
	case FTP_REQ_WITH_POWEROFF:	// 2:FTP�v������(�d�f�v)
		g_FtpRequestFlg = req;
		break;
	case PRM_CHG_REQ_NONE:		// 0:�ύX�Ȃ�
	case PRM_CHG_REQ_NORMAL:	// 1:�ύX����
		g_PrmChgFlg = req;
		break;
	default:
		break;
	}
// GG120600(E) // Phase9 LCD�p
}

//[]----------------------------------------------------------------------[]
///	@brief		 	FTP�t���O
//[]----------------------------------------------------------------------[]
///	@param			None
///	@return			g_FtpRequestFlg�FFTP�v���t���O
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/18<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
eFTP_REQ_TYPE mnt_GetFtpFlag( void )
{
	return g_FtpRequestFlg;
}
// MH810100(E)

// GG120600(S) // Phase9 ���u�����e�i���X�p
eFTP_REQ_TYPE mnt_GetRemoteFtpFlag( void )
{
	return g_PrmChgFlg;
}
// GG120600(E) // Phase9 ���u�����e�i���X�p

// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
#if 0
//[]----------------------------------------------------------------------[]
///	@brief			PKI�g�p�J�n
//[]----------------------------------------------------------------------[]
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/12/16<br>
///					Update	:	
/// @note
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void ftps_pki_use( void )
{
	int errorCode;

	errorCode = tfUsePki();

	// �����H
	if( errorCode == TM_ENOERROR ){
		ftpctrl.start = FTPS_PKI_START;
	}
}
#endif
//[]----------------------------------------------------------------------[]
///	@brief			���[�g�ؖ����ǉ�
//[]----------------------------------------------------------------------[]
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/12/16<br>
///					Update	:	
/// @note
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int ftps_add_rootCertificate( int res )
{
	int		errorCode = res;

	switch( errorCode ){
		default:
			break;
	}

	return errorCode;
}
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X�iFTPS�Ή��j
