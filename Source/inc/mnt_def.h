/*[]----------------------------------------------------------------------[]*/
/*| Maintenance control header file                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2XXX-XX-XX                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
#ifndef _MNT_DEF_H_
#define _MNT_DEF_H_

#include	"mem_def.h"

/*--------------------------------------------------------------------------*/
/*	Externs																	*/
/*--------------------------------------------------------------------------*/

/* Const table */
extern	const	unsigned short	USER_TBL[][4];
extern	const	unsigned short	SYS_TBL[][4];
extern	const	unsigned short	PARA_SET_TBL[][4];
extern	const	unsigned short	BPARA_SET_TBL[][4];
extern	const	unsigned short	CPARA_SET_TBL[][4];
extern	const	unsigned short	RPARA_SET_TBL[][4];
extern	const	unsigned short	FUN_CHK_TBL[][4];
extern	const	unsigned short	FLCD_CHK_TBL[][4];
extern	const	unsigned short	FSIG_CHK_TBL[][4];
extern	const	unsigned short	FPRN_CHK_TBL1[][4];
extern	const	unsigned short	FPRN_CHK_TBL2[][4];
extern	const	unsigned short	FPRN_CHK_TBL3[][4];
extern	const	unsigned short	FPRN_CHK_TBL4[][4];
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
extern	const	unsigned short	FPRN_CHK_TBL5[][4];
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
extern	const	unsigned short	FRDR_CHK_TBL[][4];
extern	const	unsigned short	FCMC_CHK_TBL[][4];
extern	const	unsigned short	FBNA_CHK_TBL[][4];
extern	const	unsigned short	LOG_FILE_TBL[][4];
extern	const	unsigned short	USM_MNC_TBL1[][4];
extern	const	unsigned short	USM_MNC_TBL2[][4];
extern	const	unsigned short	USM_MNS_TBL[][4];
extern	const	unsigned short	MNT_SEL_TBL[][4];
extern	const	unsigned short	CARD_ISU_TBL[][4];
extern	const	unsigned short	FUNC_CNT_TBL1[][4];
extern	const	unsigned short	FUNC_CNT_TBL2[][4];
extern	const	unsigned short	UPDN_TBL[][4];
extern	const	unsigned short	UPLORD_TBL[][4];
extern	const	unsigned short	DWLORD_TBL[][4];
extern	const	unsigned short	BKRS_TBL[][4];
extern	const	unsigned short	USM_EXT_TBL[][4];
extern	const	unsigned short	USM_EXTCRE_TBL[][4];
extern	const	unsigned short	MIFARE_CHK_TBL[][4];
extern	const	unsigned short	USM_LBSET_TBL[][4];
extern	const	unsigned short	USM_LOCK_TBL[][4];
extern	const	unsigned short	USM_KIND3_TBL[][4];
extern	const	unsigned short	USM_KIND_TBL[][4];

extern	const	unsigned short	CCOM_CHK_TBL[][4];
extern	const	unsigned short	CCOM_CHK2_TBL[][4];


// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern	const	unsigned short	USM_EXTEDY_TBL[][4];
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
extern	const	unsigned short	PIP_CHK_TBL[][4];
extern	const	unsigned short	USM_EXTSCA_TBL[][4];
extern	const	unsigned short	FSUICA_CHK_TBL[][4];
extern	const	unsigned short	FSUICA_CHK_TBL2[][4];
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern	const	unsigned short	FEDY_CHK_TBL[][4];
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
extern	const	unsigned short	FILE_TRF_TBL[][4];
extern	const	unsigned short	SYS_SW_TBL[][4];
extern	const 	unsigned short	UPLOAD_LOG_TBL[][4];
extern	const	unsigned short	USM_RT_TBL[][4];
extern	const	unsigned short	CRE_CHK_TBL[][4];
// MH810100(S) Y.Yamauchi 20191008 �Ԕԃ`�P�b�g���X(�����e�i���X)
extern	const	unsigned short	QR_CHK_TBL[][4];
extern	const	unsigned short	REAL_CHK_TBL[][4];
extern	const	unsigned short	DCNET_CHK_TBL[][4];
// MH810100(E) Y.Yamauchi 20191008 �Ԕԃ`�P�b�g���X(�����e�i���X)
// NTNET�`�F�b�N���j���[
extern	const	unsigned short	FNTNET_CHK_TBL[][4];

extern	const	unsigned short	USM_LOG_TBL[][4];
extern	const	unsigned short	USM_EXTCRE_CAPPI_TBL[][4];

extern	const	unsigned short	FUNC_FLP_TBL[][4];
extern	const	unsigned short	FUNC_CRR_TBL[][4];

extern	const	unsigned short	FUNC_MAFCHK_TBL[][4];
extern	const	unsigned short	FUNC_CAPPICHK_TBL[][4];

extern	const	unsigned short	FUNC_FTPCONNECTCHK_TBL[][4];
extern	const	unsigned short	MNT_STRCHK_TBL[][4];

extern	const	unsigned short	USM_CERTI_TBL[][4];
extern	const	char			USM_CERTI_TBL_MAX;

// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�`�F�b�N�ǉ�)
extern	const	unsigned short	FUN_CHK_TBL2[][4];
extern	const	unsigned short	FECR_CHK_TBL[][4];
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�`�F�b�N�ǉ�)

/* Groval variables */

extern	char	DP_CP[2];				/* 0:Cursor Possion ( 0�`2 )				*/
										/* 1:Item �� displayed in the first line	*/
extern	unsigned char	*fck_chk_adr;		// ������ذ���ڽ�i�����q�`�l��ذ�����p�j
extern	unsigned char	fck_chk_err;		// ��������		�i�����q�`�l��ذ�����p�j
extern	unsigned char	fck_sav_data;		// �ް��ޔ�ر	�i�����q�`�l��ذ�����p�j
extern	unsigned char	fck_chk_data1;		// �����ް��P	�i�����q�`�l��ذ�����p�j
extern	unsigned char	fck_chk_data2;		// �����ް��Q	�i�����q�`�l��ذ�����p�j
enum{
// MH810100(S) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	Multi_Total = 1,
//	Credit,
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////	Edy,
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	Suica,
//	iD,
//	Hojin,
//	DLock,
	DLock = 1,
// MH810100(E) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	MTotal,		// �l�s�W�v
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i���σ��[�_�����ǉ��j
	EcReader,	// ���σ��[�_
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i���σ��[�_�����ǉ��j
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	EJournal,	// �d�q�W���[�i��
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	USM_EXT_MAX
};
extern	unsigned char	Ext_Menu_Max;										/* ���j���[�\�����ڐ��i�ݒ�ɂ��ρj*/
extern	unsigned char	USM_ExtendMENU[USM_EXT_MAX][31];					/* �g���@�\ Menu �i�\�z�p�j*/
extern	unsigned short	USM_ExtendMENU_TBL[USM_EXT_MAX][4];					/* �g���@�\ Menu TBL�i�\�z�p�j*/

extern short	gCurSrtPara;	// �Ԏ����ށi����/���ցj
extern short	DispAdjuster;	// �Ԏ��p�����[�^�\���p�����l

extern uchar	MovCntClr;		// ���춳�ĸر���s�L���iOFF=���s���^ON=�L�j

/* Arcnet NID & Version No. */
struct	st_arc_com	{
	char			id;
	char			nid;
	unsigned char	ver[8];
	char			result;
};

//---------------------------------
// �̎��؍Ĕ��s�\���p
//---------------------------------
typedef struct {
	ushort			nextIndex;			// ���̃��O�C���f�b�N�X
	ushort			nextCount;			// ���̌����Ώۃ��O����
} t_OpeLog_FindNextLog;

// Function (mntdata.c)
extern	unsigned short	StoF( unsigned short keyid, char mode );
extern	unsigned short	Menu_Slt( const unsigned char (*d_str)[31], const unsigned short (*d_tbl)[4], char cnt, char mod );
extern	char			pag_ctl( unsigned short ind, unsigned short pos, unsigned short *top );

// Function (parametersec.c)
extern	unsigned short	ParSetMain( void );
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern	unsigned short	SysParWrite( char * f_DataChange, char * edy_DataChange );
extern	unsigned short	SysParWrite( char * f_DataChange );
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
extern	unsigned short	CarParWrite( char * f_DataChange );
extern	unsigned short	RckParWrite( char * f_DataChange );
// MH810100(S) Y.Yamauchi 2019/10/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
extern  unsigned short	BPara_Set( void );
// MH810100(E) Y.Yamauchi 2019/10/10 �Ԕԃ`�P�b�g���X(�����e�i���X)

// Function (fncchk.c)
extern	unsigned short	FncChkMain( void );
extern	unsigned short	FunChk_Lcd( void );
extern	void			IBKCtrl_SetRcvData_manu(uchar id, uchar rslt);
extern	uchar	bcdadd( uchar data1, uchar data2 );

extern	void CcomApnGet( uchar * );
extern	void CcomApnSet( const uchar *,unsigned short );
extern	void CcomIpGet( uchar *, short, short);
// MH810100(S) Y.Yamauchi 2019/12/05 �Ԕԃ`�P�b�g���X(�����e�i���X)
//extern	void VoiceGuideTest( char );
extern	BOOL PKTcmd_mnt_qr_ctrl_req( unsigned char ctrl_cd );
extern	BOOL PKTcmd_mnt_rt_con_req( void );
extern	BOOL PKTcmd_mnt_dc_con_req( void );
// MH810100(E) Y.Yamauchi 2019/12/05 �Ԕԃ`�P�b�g���X(�����e�i���X)
// Function (lcdchk.c)
extern	unsigned short	FColorLCDChk_PatternDSP( void );
extern	unsigned short	FColorLCDChk_Bk( void );
extern	unsigned short	FColorLCDChk_Lumine_change( void );

extern	int				UsMnt_mnyctl_chk( void );
extern	uchar			UserMnt_SysParaUpdateCheck( ushort );
extern	unsigned char	Is_ExtendMenuMake( void );
extern	void			Is_ExtendMenuMakeTable(uchar, uchar, uchar);
extern	short			Is_CarMenuMake( uchar );
extern	ushort 			Is_MenuStrMake( uchar );
extern	unsigned char	GetCarInfoParam( void );
// ���j���[����ݒ�ɂĕω������郁�j���[�Ǘ��p��enum�ł�
enum{
	EXTEND_MENU=0,
	CAR_3_MENU,
	CAR_2_MENU,
	MV_CNT_MENU,
	LOCK_CTRL_MENU,
};

extern	const	ushort	Car_Start_Index[3];
extern	void	Ope_clk_set( struct clk_rec *p_clk_data, ushort OpeMonCode );

// Function (usermnt2.c)
extern	unsigned short	UsMnt_AreaSelect(short kind);
extern	unsigned short	UsMnt_StaySts(void);
extern	unsigned short	UsMnt_ParkCnt(void);
extern	unsigned short	UsMnt_FullCtrl(void);
extern	unsigned short	UsMnt_Receipt(void);
									// �ʐ��Z���O�����E1�����o���pdefine
#define	PAY_LOG_ALL				0	// �S�ʐ��Z���O��ΏۂƂ���
#define	PAY_LOG_CMP				1	// ���Z����(�s���E�����o�ɂ�����)�̌ʐ��Z���O��ΏۂƂ���
#define	PAY_LOG_STP				2	// ���Z���~�E�Đ��Z���~�̌ʐ��Z���O��ΏۂƂ���
#define	PAY_LOG_CMP_STP			3	// ���Z����(�s���E�����o�ɂ�����)�E���Z���~�E�Đ��Z���~�̌ʐ��Z���O��ΏۂƂ���

extern	ushort			Ope_SaleLog_TotalCountGet( uchar );
extern	uchar			Ope_SaleLog_1DataGet( ushort, uchar, ushort, Receipt_data * );
extern	uchar			Ope_SaleLog_First1DataGet( ushort Index, uchar Kind, ushort Index_Kind, Receipt_data *pSetBuf, t_OpeLog_FindNextLog* pNextLog );
extern	uchar			Ope_SaleLog_Next1DataGet( ushort Index, uchar Kind, Receipt_data *pSetBuf, t_OpeLog_FindNextLog* pNextLog );
extern	unsigned short	UsMnt_PassInv(void);
extern	unsigned short	UsMnt_PassEnter(void);
extern	unsigned short	UsMnt_PassCheck(void);
extern	unsigned short	UsMnt_SrvTime(void);
extern	unsigned short	UsMnt_LockTimer(uchar);
extern	unsigned short	UsMnt_LockTimerEx(void);
extern	unsigned short	UsMnt_SplDay(void);
extern	unsigned short	UsMnt_PassWord(void);
extern	unsigned short	UsMnt_BackLignt(void);
extern	unsigned short	UsMnt_KeyVolume(void);
extern	unsigned short	UsMnt_TickValid(void);
extern	unsigned short	UsMnt_BusyTime(void);
extern	unsigned short	UsMnt_Mnysetmenu(void);
extern	void			TKI_Delete(short);
extern	unsigned short	UsMnt_Extendmenu(void);
extern	unsigned short	UsMnt_FTotal(void);
extern	unsigned short	UsMnt_PassStop(void);
extern	const struct	TKI_CYUSI	*TKI_Get(struct	TKI_CYUSI *data, short ofs);
extern	void			TKI_DeleteAll(void);
extern	unsigned short	UsMnt_PassExTime(void);
extern	unsigned short	UsMnt_LabelPrint(void);
extern	unsigned short	UsMnt_LabelSet(void);
extern	unsigned short	UsMnt_PreAreaSelect(short);
extern	unsigned short	UsMnt_PwdKyo(void);
extern	unsigned short	UsMnt_VolSwTime(void);
extern	char clk_test( short data, char pos );
extern	unsigned short	UsMnt_StatusView(void);
extern	unsigned short	UsMnt_CreditMnu(void);
extern	unsigned short	UsMnt_CreUseLog(void);
extern	unsigned short	UsMnt_CreUseLog_GetLogCnt(ushort Sdate, ushort Edate);
extern	unsigned short	UsMnt_VoiceGuideTime(void);

#define	_TKI_Idx2Ofs(idx)	_offset(tki_cyusi.wtp, TKI_CYUSI_MAX-tki_cyusi.count+idx, TKI_CYUSI_MAX)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//extern	unsigned short	UsMnt_EdyMnu(void);
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

extern	unsigned short	UsMnt_SuicaMnu(void);
extern	unsigned short	UsMnt_DiditalCasheSyuukei(ushort);
extern	unsigned short	UsMnt_DiditalCasheUseLog(ushort);
extern	unsigned short	SysMnt_FTPClient(void);
extern	unsigned short	SysMnt_FTPServer(void);
extern  unsigned short	SysMnt_SystemSwitch(void);
extern	unsigned short	UsMnt_AttendantValidData(void);
extern	unsigned short	UsMnt_Logprintmenu(void);
extern	unsigned short	UsMnt_NgLog(void);
extern	unsigned short	UsMnt_IoLog(ushort);
// MH810100(S)  K.Onodera   2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//extern	ushort UsMnt_SearchIOLog(ushort, ushort, ulong, ulong, IoLog_Data*);
// MH810100(E)  K.Onodera   2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
extern	void	LockInfoDataCheck(void);
extern	ushort	UsColorLCDLumine(void);
extern unsigned short	UsMnt_ParkingCertificate(void);
extern  unsigned short	SysMnt_RyoTest(void);

// �����␳����
extern int		sntpGetTime(date_time_rec2 *time, ushort * net_msec);
extern void		TimeAdjustInit(void);
extern int		TimeAdjustCtrl(ushort req);
extern void		RegistReviseLog(ushort exec);
extern void		ReqServerTime(ushort msg);
extern int		CheckTimeMargin(date_time_rec2 *dt, ushort net_msec, ulong margin[]);
extern ushort	AddTimeMargin(ulong margin[], short in_d[]);
extern void		TimeAdjustMain(ushort msg);
extern void		clk_auto_set(void);
extern	void	UsMnt_datecheck( uchar );

// Function (usermnt.c)
extern	unsigned short	UsMnt_Total( short );
extern	void	LogDateDsp4( ushort *Date );
extern	void	LogDateDsp5( ushort *Date, uchar pos );
extern	void	LogCntDsp( ushort LogCnt );
extern	uchar	LogDatePosUp( uchar *pos, uchar req );

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
// �g���@�\/���σ��[�_����
extern	const	unsigned short	USM_EXTEC_TBL[][4];
extern unsigned short GetEcShuukeiCnt(ushort s_date, ushort e_date);
// �V�X�e�������e�i���X/���σ��[�_�`�F�b�N/�u�����h�m�F���
// MH810103 GG119202(S) �u�����h���̓u�����h��񂩂�擾����
//extern unsigned char  get_brand_index( ushort no );
// MH810103 GG119202(E) �u�����h���̓u�����h��񂩂�擾����
extern uchar          check_printer( uchar *pri_kind );
extern unsigned short UsMnt_ECVoiceGuide( void );
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

// MH810100(S) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)
extern int lcdbm_setting_upload_FTP( void );
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X(�����e�i���X)

// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
extern uchar Lcd_QR_ErrDisp( ushort result );
extern void UsMnt_QR_ErrDisp( int err );
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
//extern void UsMnt_QR_DispDisQR(QR_DiscountInfo *pDisQR, ushort updown_page);
//extern void UsMnt_QR_DispAmntQR(QR_AmountInfo *pAmntQR, ushort updown_page);
extern void UsMnt_QR_DispDisQR(QR_DiscountInfo *pDisQR, ushort updown_page, ushort qrtype);
extern void UsMnt_QR_DispAmntQR(QR_AmountInfo *pAmntQR, ushort updown_page, ushort qrtype);
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
extern ushort UsMnt_QR_GetUpdownPageMax(ushort id, QR_AmountInfo *pAmntQR);
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
// GG129000(S) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
extern void UsMnt_QR_DispTicQR(QR_TicketInfo *pTicQR, ushort updown_page);
// GG129000(E) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j

/*--------------------------------------------------------------------------*/
/*	Difines																	*/
/*--------------------------------------------------------------------------*/

/* Macro */
#define	KEY_TEN0to9(msg)	(KEY_TEN0<=msg)&&(msg<=KEY_TEN9) ? KEY_TEN:msg

#define		PGCNT(cnt)		( cnt % 5 ? cnt / 5 : (cnt / 5) - 1 )

/* �^�C�}�[ */
#define	OPE_LCD_BACK_CNTL	1					/* LCD Back Timer					*/	//��

/* ���j���[�e�[�u���̍��ڐ� */
/* Menu items max */
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
// #define		SYS_MENU_MAX		9				/* Engeneer						*/
#define		SYS_MENU_MAX		8				/* Engeneer						*/
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define		FL_TRF_MAX			2				/* �t�@�C���]�� Menu�@�@�@�@�@�@*/
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�����e�i���X)
//#define		SYS_SW_MAX			3				/* �V�X�e���ؑ� Menu�@�@�@�@�@�@*/
#define		SYS_SW_MAX			2				/* �V�X�e���ؑ� Menu�@�@�@�@�@�@*/
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//#define		FNTNET_CHK_MAX		3				/* NTNET�`�F�b�N���j���[		*/
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���Ԃ̒ǔԃN���A���j���[���폜�j
//#define		FNTNET_CHK_MAX		2				/* NTNET�`�F�b�N���j���[		*/
#define		FNTNET_CHK_MAX		1				/* NTNET�`�F�b�N���j���[		*/
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���Ԃ̒ǔԃN���A���j���[���폜�j
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

#define		PARA_SET_MAX		3				/* 								*/
#define		BPARA_SET_MAX		3				/* 								*/
#define		CPARA_SET_MAX		3				/* 								*/
#define		RPARA_SET_MAX		3				/* 								*/
enum {
	__swt_check = 0,	// �X�C�b�`�`�F�b�N
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	 __kbd_check,		// �L�[���̓`�F�b�N
//	__lcd_check,		// �k�b�c�`�F�b�N
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
	__led_check,		// �k�d�c�`�F�b�N
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
// __sht_check,		// �V���b�^�[�`�F�b�N
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
	__sig_check,		// ���o�͐M���`�F�b�N
	__cmc_check,		// �R�C�����b�N�`�F�b�N
	__bna_check,		// �������[�_�[�`�F�b�N
	__prt_check,		// �v�����^�`�F�b�N
// MH810100(S) Y.Yamauchi 20191008 �Ԕԃ`�P�b�g���X(�����e�i���X)
	__qr_check,		//�p�q���[�_�[�`�F�b�N�@
// MH810100(E) Y.Yamauchi 20191008 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
// __rdr_check,		// ���C���[�_�[�`�F�b�N
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
	__ann_check,		// �A�i�E���X�`�F�b�N
	__mem_check,		// �������[�`�F�b�N
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
// __lck_check,		// ���b�NIF�Ճ`�F�b�N
// __flp_check,		// �t���b�v���u�`�F�b�N
// __crr_check,		// �t���b�v�����`�F�b�N
//	__sca_check,		// ��ʌnIC�J�[�h�`�F�b�N
//	__pip_check,		// Park i Pro�`�F�b�N
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
	__lan_connect_chk,	// �k�`�m�ڑ��`�F�b�N
	__funope_check,		// �e�`�m����`�F�b�N
	__centercom_check,	//	�Z���^�[�ʐM�`�F�b�N
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
// __cre_check,			// �N���W�b�g�`�F�b�N
	__realtime_check,	// ���A���^�C���ʐM�`�F�b�N
	__dc_net_check,		// DC-NET�ʐM�`�F�b�N
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	__ntn_check,		// �m�s�|�m�d�s�`�F�b�N
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	__ver_check,	//�o�[�W�����`�F�b�N
	FUN_CHK_MAX
};
// MH810100(S) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
enum {
	__swt_check2 = 0,	// �X�C�b�`�`�F�b�N
	__led_check2,		// �k�d�c�`�F�b�N
	__sig_check2,		// ���o�͐M���`�F�b�N
	__cmc_check2,		// �R�C�����b�N�`�F�b�N
	__bna_check2,		// �������[�_�[�`�F�b�N
	__prt_check2,		// �v�����^�`�F�b�N
	__qr_check2,		//�p�q���[�_�[�`�F�b�N�@
	__ann_check2,		// �A�i�E���X�`�F�b�N
	__mem_check2,		// �������[�`�F�b�N
	__sca_check2,		// : ���σ��[�_�`�F�b�N
	__lan_connect_chk2,	// �k�`�m�ڑ��`�F�b�N
	__funope_check2,		// �e�`�m����`�F�b�N
	__centercom_check2,	//	�Z���^�[�ʐM�`�F�b�N
	__realtime_check2,	// ���A���^�C���ʐM�`�F�b�N
	__dc_net_check2,		// DC-NET�ʐM�`�F�b�N
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	__ntn_check2,		// �m�s�|�m�d�s�`�F�b�N
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	__ver_check2,	//�o�[�W�����`�F�b�N
	FUN_CHK_MAX2
};
// MH810100(E) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

enum{
	__MNT_ERARM = 0,	// .�G���[�E�A���[���m�F
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)
// __MNT_FLCTL,		// .���b�N���u�J��
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	__MNT_MNCNT,		// .�ޑK�Ǘ�
	__MNT_TTOTL,		// .T�W�v
	__MNT_GTOTL,		// .GT�W�v
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
// __MNT_STAYD,		// .�◯�ԏ��
// __MNT_LOGPRN,		// .�������v�����g
// __MNT_FLCNT,		// .���ԑ䐔
// __MNT_FLCTRL,		// .���ԃR���g���[��
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	__MNT_STSVIEW,		// .��Ԋm�F
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
// __MNT_FLSTS,		// .�Ԏ����
// __MNT_CARFAIL,		// .�Ԏ��̏�
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	__MNT_REPRT,		// .�̎����Ĕ��s
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
// __MNT_UKERP,		// .���ԏؖ����Ĕ��s
// __MNT_INVLD,		// .����L��/����
// __MNT_ENTRD,		// .�������/�o��
// __MNT_PSTOP,		// .��������Z���~�f�[�^
// __MNT_PASCK,		// .������`�F�b�N
// __MNT_TKTDT,		// .���f�[�^�m�F
	__MNT_QRCHECK,		// .QR�f�[�^�m�F
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	__MNT_CLOCK,		// .���v����
	__MNT_SERVS,		// .�T�[�r�X�^�C��
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
// __MNT_FLTIM,		// .���b�N���u�^�C�}�[
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	__MNT_SPCAL,		// .���ʓ�/���ʊ���
	__MNT_OPCLS,		// .�c�x�Ɛؑ�
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
// __MNT_SHTER,		// .�V���b�^�[�J��
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	__MNT_PWMOD,		// .�W���p�X���[�h
	__MNT_KEYVL,		// .�L�[���ʒ���
	__MNT_CNTRS,		// .�P�x����
	__MNT_VLSW,			// .�����ē�����
	__MNT_OPNHR,		// .�c�ƊJ�n����
// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
// __MNT_TKEXP,		// .������
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	__MNT_MNYSET,		// .�����ݒ�

// MH810100(S) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
// __MNT_ATTENDDATA,	// .�W���L���f�[�^
// __MNT_PWDKY,		// .�����ԍ������o��
// MH810100(E) Y.Yamauchi 20191003 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	__MNT_EXTEND,		// .�g���@�\
	USER_MENU_MAX,
};

#define		MNT_SEL_MAX			2				/*								*/
#define		FLCD_CHK_MAX1		2				/*								*/
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
// #define		FLCD_CHK_MAX2		3				/*								*/
#define		FLCD_CHK_MAX2		1				/*								*/
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//#define		FPRN_CHK_MAX1		2				/*								*/
#define		FPRN_CHK_MAX1		3				/*								*/
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
#define		FPRN_CHK_MAX2		3				/*								*/
#define		FPRN_CHK_MAX3		2				/*								*/
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define		FPRN_CHK_MAX5		4				/*								*/
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
#define		FRDR_CHK_MAX		5				/*								*/
#define		FCMC_CHK_MAX		4				/*								*/
#define		USM_MNC_MAX1		3				/*								*/
#define		USM_MNC_MAX2		5				/*								*/
#define		FBNA_CHK_MAX		3				/*								*/
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//�@MH810101 Takei(S) �t�F�[�Y2�@2021/02/16 ����Ή��@��������j�����o��
#define		USM_MNS_MAX			8				/* �����ݒ���̍��ڐ�			*/
// #define		USM_MNS_MAX			7				/* �����ݒ���̍��ڐ�			*/
//�@MH810101 Takei(E) �t�F�[�Y2�@2021/02/16 ����Ή��@��������j�����o��
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define		CARD_ISU_MAX		2				/* 								*/
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
// #define		LOG_MENU_MAX		16				/*								*/
#define		LOG_MENU_MAX		15				/*								*/
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define		FCNT_MENU1_MAX		2				/* 								*/
#define		FCNT_MENU2_MAX		3				/* 								*/
#define		USM_EXTCRE_MAX		4				/* 								*/
#define		MIFARE_CHK_MAX		4				/* 								*/
#define		LBSET_MENU_MAX		3				/* 								*/
#define		USM_EXTCRE_CAPPI_MAX	2			/* �N���W�b�g����(Cappi)���ڐ��@*/
#define		USM_LOG_MAX			3				/* 	�������v�����g�ƭ����ڐ�	*/
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//#define		CCOM_CHK_MAX		7
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
// #define		CCOM_CHK_MAX		8
// #define		CCOM_CHK_MAX		5
#define		CCOM_CHK_MAX		4
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
#define		CCOM_CHK2_MAX		3
#define		PIP_CHK_MAX			1
#define		CRE_CHK_MAX			2
// MH810100(S) Y.Yamauchi 20191008 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define		QR_CHK_MAX			2
#define		REAL_CHK_MAX		3
#define		DC_NET_CHK_TBL		3
// MH810100(E) Y.Yamauchi 20191008 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define		LOCK_MENU_MAX1		2		/* ���b�N���u�J�� Menu1 */
#define		LOCK_MENU_MAX2		4		/* ���b�N���u�J�� Menu2 */
#define		KIND_MENU_MAX		3		/* �Ԏ�I�� Menu */
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
// #define		BKRS_MAX			3
#define		BKRS_MAX			2
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define		UPDN_MAX			2
extern	const	char		UPLD_MAX;
extern	const	char		DNLD_MAX;

#define		UPLD_LOG_MAX		4				/* ���O�f�[�^ Menu�@�@�@�@�@�@�@*/

#define		FSIG_CHK_MAX		2
#define		FSUICA_CHK_MAX		5				/*								*/
#define		FSUICA_CHK_MAX2		2				/*								*/
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		FEDY_CHK_MAX		7				/*								*/
//
//#define		USM_EXTEDY_MAX		5				/* �d�������j���[���ڐ�			*/
//#define		USM_BUN_MAX			2				/* �d�������j���[���ڐ�(���g�p)	*/
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		USM_EXTSCA_KND		5				/* �d�q�}�l�[��ސ�(���g�p)		*/
#define		USM_EXTSCA_MAX		2				/* �d�q�}�l�[���j���[���ڐ�		*/

#define		FUNC_FLP_MAX		2				/* �t���b�v���u�`�F�b�N���j���[���ڐ�*/
#define		FUNC_CRR_MAX		2				/* �t���b�v�����`�F�b�N���j���[���ڐ�*/
#define		FUNC_MAF_MAX		3				/* ���u�ʐM���j���[���ڐ�		*/
#define		FUNC_CAPPI_MAX		2				/* CAPPI�`�F�b�N���j���[���ڐ�		*/

#define		FUNC_FTPCONNECT_MAX		2			/* FTP�ڑ��m�F���j���[���ڐ�		*/
#define		MNT_STRCHK_MAX		1				/* �f�o�b�O�p���j���[���ڐ�*/
#define		MNT_FEETEST_MAX		2				/* �����v�Z�e�X�g���ڐ�*/
#define		ANT_CNT_DEF			100				/* �d�g��M��Ԏ擾�񐔃f�t�H���g(100��)	*/
#define		ANT_INT_DEF			5				/* �d�g��M��Ԏ擾�Ԋu�f�t�H���g(500ms)	*/

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
#define		FECR_CHK_MAX		5				/* ���σ��[�_�`�F�b�N ���j���[���ڐ� */
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//#define		USM_EXTEC_MAX		3				/* ���σ��[�_���� ���j���[���ڐ�*/
#define		USM_EXTEC_MAX		4				/* ���σ��[�_���� ���j���[���ڐ�*/
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define		USM_EXTEJ_MAX		1				/* �d�q�W���[�i�� ���j���[���ڐ�*/
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

/* Maintenance operation level */
#define		ATTEND			0					/* 								*/
#define		MANAGE			1					/* 								*/
#define		ENGENE			2					/* 								*/

/* Operation ID */
/* Phase 0 */
#define	USERMNTMAIN	1	/* User Maintenance...Attendant or Manager */
#define	SYSMNTMAIN	2	/* System Maintenance...Engeneer */

/* Phase 1 */
/* User Maintenance 0-69 */
enum{
	// �ȉ��̔ԍ��́A���ꃁ�j���[���ň�ӂ̐��l�ł���΂����̂ŁA���j���[�ԍ��ƃ����N����K�v�͂Ȃ�
	// ����̒ǉ��͈�ӂ̔ԍ��Ɣ��Ȃ��悤�ɐݒ肷�邱�ƁB
	// �����_�ł�MNT_IVTRY=50�Ȃ̂ŁA���������ɔ��Ȃ��悤�ɂ��邱��
	MNT_NONE	= 0,				// �����Ȃ�
	MNT_ERARM	= 1,                // .�G���[�E�A���[���m�F
	MNT_FLCTL	,                   // .���b�N���u�J��
	MNT_MNCNT	,                   // .�ޑK�Ǘ�
	MNT_TTOTL	,					// .T�W�v
	MNT_GTOTL	,                   // .GT�W�v
	MNT_STAYD	,                   // .�◯�ԏ��
	MNT_LOGPRN	,                   // .�������v�����g	
	MNT_FLCNT	,                   // .���ԑ䐔
	MNT_FLCTRL	,                   // .���ԃR���g���[��
	MNT_STSVIEW	,                   // .��Ԋm�F
	MNT_FLSTS	,                   // .�Ԏ����
	MNT_CARFAIL	,                   // .�Ԏ��̏�
	MNT_REPRT	,                   // .�̎����Ĕ��s
	MNT_UKERP	,                   // .���ԏؖ����Ĕ��s
	MNT_INVLD	,                   // .����L��/����
	MNT_ENTRD	,                   // .�������/�o��
	MNT_PSTOP	,                   // .��������Z���~�f�[�^
	MNT_PASCK	,                   // .������`�F�b�N
	MNT_TKTDT	,                   // .���f�[�^�m�F
// MH810100(S) Y.Yamauchi 2019/11/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
	MNT_QRCHECK,					// .QR�f�[�^�m�F
// MH810100(E) Y.Yamauchi 2019/11/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
	MNT_CLOCK	,                   // .���v����
	MNT_SERVS	,                   // .�T�[�r�X�^�C��
	MNT_FLTIM	,                   // .���b�N���u�^�C�}�[
	MNT_SPCAL	,                   // .���ʓ�/���ʊ���
	MNT_OPCLS	,                   // .�c�x�Ɛؑ�
	MNT_SHTER	,                   // .�V���b�^�[�J��
	MNT_PWMOD	,                   // .�W���p�X���[�h
	MNT_KEYVL	,                   // .�L�[���ʒ���
	MNT_CNTRS	,                   // .�P�x����
	MNT_VLSW	,                   // .���ʈē�����
	MNT_OPNHR	,                   // .�c�ƊJ�n����
	MNT_TKEXP	,                   // .������
	MNT_MNYSET	,                   // .�����ݒ�
	MNT_ATTENDDATA	,               // .�W���L���f�[�^
	MNT_EXTEND	,                   // .�g���@�\
};

#define	MNT_IVTRY	50			// �C���x���g��
#define	MNT_BKLIT	51			// �o�b�N���C�g�_�����@
#define	MNT_MNYBOX	52			// ���Ɋm�F
#define	MNT_MNYCHG	53			// �ޑK����
#define	MNT_FTOTL	54			// �����䐔�W�v
#define	MNT_LBSET	55			// ���x���v�����^�ݒ�
#define	MNT_LBPAPER	56			// �p���c�ʐݒ�
#define	MNT_LBPAY	57			// ���x�����s�s���̐ݒ�
#define	MNT_LBTESTP	58			// ���x���e�X�g��
#define	MNT_PWDKY	59			// �Ïؔԍ������o��
#define MNT_MNYLIM 	60			// Limit money setting
#define MNT_MNYTIC	61			// Service ticket setting
#define MNT_MNYSHP	62			// Shop setting
#define MNT_MNYTAX	63			// Tax setting
#define MNT_MNYKID	64			// A-L kind setting
#define MNT_MNYCHA	65			// Charge setting
#define MNT_MNYTWK	66			// Ticket disable week setting

/* Phase 2 */       
/* System Maintenance 70-89 */
#define	MNT_PRSET	70
#define	MNT_FNCHK	71
#define	MNT_FNCNT	72
#define	MNT_LKCNT	73
#define	MNT_LOGFL	74
#define	MNT_BAKRS	75
#define	MNT_INITI	76
#define	MNT_CDISS	77
#define	MNT_OPLOG	78
#define	MNT_SYSMN	79
#define	MNT_TBSET	80
#define	MNT_CRSET	81
#define	MNT_FORMT	82
#define	MNT_UPDWN	83
#define	MNT_MNYPRI	84

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define	MNT_EDY_MEISAI	85	// �d�������p����
//#define	MNT_EDY_SHUUKEI	86	// �d�����W�v
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define	MNT_SCA			87	// �r������������
#define	MNT_SCA_MEISAI	88	// �r�����������p����
#define	MNT_SCA_SHUUKEI	89	// �r���������W�v

/* Function Check 90-110 */
#define	SWT_CHK		90
#define	LCD_CHK		91
#define	PRT_CHK		92
#define	CMC_CHK		93
#define	BNA_CHK		94
#define	SIG_CHK		95
#define	MEM_CHK		96
#define	ANN_CHK		97
#define	ARC_CHK		99
#define	LED_CHK		100
#define	KBD_CHK		101
#define	SHT_CHK		102
#define	RDR_CHK		103
#define	NTNET_CHK	104
#define	VER_CHK		105
#define	MIF_CHK		106
#define	LCK_CHK		107
#define	FLP_CHK		108
#define	SUICA_CHK	109
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define	EDY_CHK		110
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define	CCM_CHK		111
#define	PIP_CHK		112
#define	FUNOPE_CHK	113
#define MNT_FLTRF	114
#define MNT_SYSSW	115
#define CRE_CHK		116
#define	CRR_CHK		119

/* Function Check 121-125 */
#define	FLCD1_CHK	121
#define	FLCD2_CHK	122
#define	FLCD3_CHK	123
#define	FLCD4_CHK	124

/* Function Check 127-128 */
#define	FSIG1_CHK	127
#define	FSIG2_CHK	128

/* Function Check 131-136 */
#define	FPRNR_CHK	131		// ڼ�����������
#define	FPRNJ_CHK	132		// �ެ������������

#define	FPRN1_CHK	133		// ڼ�������	�FýĈ�
#define	FPRN2_CHK	134		// ڼ�������	�F��Ԋm�F
#define	FPRN3_CHK	135		// �ެ��������	�FýĈ�
#define	FPRN4_CHK	136		// �ެ��������	�F��Ԋm�F
#define	FPRN5_CHK	137		// ڼ�������	�F���S�󎚃f�[�^�o�^
#define	FPRN7_CHK	139		// ڼ��ýĲݻ�
#define	FPRN8_CHK	140		// ڼ�������	�F�̎������

/* Function Check 141-145 */
#define	FRD1_CHK	141
#define	FRD2_CHK	142
#define	FRD3_CHK	143
#define	FRD4_CHK	144
#define	FRD5_CHK	145

/* Function Check 151-154 */
#define	FCM1_CHK	151
#define	FCM2_CHK	152
#define	FCM3_CHK	153
#define	FCM4_CHK	154

/* Function Check 161- 163*/
#define	FBN1_CHK	161
#define	FBN2_CHK	162
#define	FBN3_CHK	163

/* Function Check 167-169 */
#define	FNTNET1_CHK	167
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//#define	FNTNET2_CHK	168
//#define	FNTNET3_CHK	169
#define	CCOM_SEQCLR	168
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/* Log Files 171-181 */
#define	ERR_LOG		171		// �G���[���
#define	ARM_LOG		172		// �A���[�����
#define	TGOU_LOG	173		// �s���v���
#define	COIN_LOG	174		// �R�C�����ɏ��
#define	NOTE_LOG	175		// �������ɏ��
#define	TURI_LOG	176		// �ޑK�Ǘ����
#define	SEISAN_LOG	177		// �ʐ��Z���
#define	FUSKYO_LOG	178		// �s���E�����o�ɏ��
#define	OPE_LOG		180		// ������
#define	TEIFUK_LOG	181		// �╜�d���
#define	CLEAR_LOG	182		// ���O�t�@�C���N���A
#define	MONI_LOG	183		// ���j�^���
#define	ADDPRM_LOG	184		// �A���[�����
#define	RTPAY_LOG	185		// �����e�X�g���
#define	GTGOU_LOG	186		// �f�s���v���
#define	RMON_LOG	187		// ���u�Ď����

/* Backup & Restore XX-XX(25?) */

/* Card Issue 190-192 */
#define	CARD_PASS	190		/* ��� */
#define	CARD_KKRI	191		/* �W���J�[�h */
#define	CARD_MIFA	192		/* Mifare��� */

/* Exit ID */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define MOD_CUT		0xFFFD
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
#define	MOD_EXT		0xFFFE
#define	MOD_CHG		0xFFFF

/* Maintenance Select 300-301 */
#define	USR_MNT		300
#define	SYS_MNT		301

/* Parameter Setting1 200-202(3) */
#define	BASC_PARA	200		/* Basic Parameter */
#define	CELL_PARA	201		/* Cell Parameter */
#define	ROCK_PARA	202		/* Rock Parameter */

/* Basic Parameter Setting 210-213 */
#define	CMON_PARA	210		/* Common Parameter */
#define	INDV_PARA	211		/* Individual Parameter */
#define	PRNT_PARA	212		/* Print All Parameter */
#define	DEFA_PARA	213		/* Default All Parameter */

/* Cell Parameter Setting 220-222 */
#define	CARP_PARA	220		/* Car Parameter */
#define	CARP_PRNT	221		/* Print All Parameter */
#define	CARP_DEFA	222		/* Default All Parameter */

/* Rock Parameter Setting 230-232 */
#define	RCKP_PARA	230		/* Car Parameter */
#define	RCKP_PRNT	231		/* Print All Parameter */
#define	RCKP_DEFA	232		/* Default All Parameter */

/* Function count 240-241 */
#define	FCNT_FUNC	240		/* ����J�E���g */
#define	FCNT_ROCK	241		/* ���b�N���u����J�E���g */
#define	FCNT_FLAP	242		/* �t���b�v���u����J�E���g */

// Proc number of money management
#define	MNY_CHG_BEFORE			100
#define	MNY_CHG_AFTER			101
#define	MNY_CTL_BEFORE			102
#define	MNY_CTL_AFTER			103
#define MNY_INCOIN				104
#define MNY_CTL_AUTO			105
#define MNY_CHG_10YEN			106
#define MNY_CHG_50YEN			107
#define MNY_CHG_100YEN			108
#define MNY_CHG_500YEN			109
#define MNY_CHG_10SUBYEN		110
#define MNY_CHG_100SUBYEN		111
#define MNY_CTL_AUTOSTART		112
#define MNY_CTL_AUTOCOMPLETE	113
#define MNY_COIN_CASETTE		114
#define MNY_COIN_INVSTART		115
#define MNY_COIN_INVCOMPLETE	116
#define MNY_COIN_INVBUTTON		117
#define MNY_CHG_50SUBYEN		118

/* backup or restore 250-252 */
#define	BKRS_BK					250
#define	BKRS_RS					251
#define	BKRS_FLSTS				252

/* upload or download 260-261 */
#define	UPDN_UP					260
#define	UPDN_DN					261

/* upload(download) data 270-275 */
#define	LD_LOGO					270
#define	LD_HDR					271
#define	LD_FTR					272
#define	LD_PARA					273
#define	LD_SLOG					274
#define	LD_TLOG					275
#define	LD_LOCKPARA				276
#define	LD_ACCEPTFTR			277
#define	LD_SYOM					278
#define	LD_KAME					279

/* Mifare checkt 280-283 */
#define	CHK_COMTST				280		/* �ʐM�e�X�g */
#define	CHK_REDLITTST			281		/* ���[�h�E���C�g�e�X�g  */
#define	CHK_CADPECRNUMRED		282		/* �J�[�h�ŗL�ԍ����[�h  */
#define	CHK_VERCHK				283		/* �o�[�W�����`�F�b�N  */

/* Lock-machine 290-291 */
#define	MNT_FLAPUD				290		/* �t���b�v�㏸���~ */
#define	MNT_BIKLCK				291		/* ���փ��b�N�J�� */
#define	MNT_FLAPUD_ALL			292		/* �t���b�v�㏸���~�i�S�āj		*/
#define	MNT_BIKLCK_ALL			293		/* ���փ��b�N�J�i�S�āj	 	*/
#define	MNT_INT_CAR				319		/* ����(����) */
#define	MNT_CAR					320		/* ���� */
#define	MNT_BIK					321		/* ���� */

/* User Maintenance 200-299 */
#define	MNT_CREDIT				310	// �N���W�b�g�����i�g���@�\�j
#define	MNT_CREUSE				311	// �N���W�b�g���p���ׁi�g���@�\�j
#define	MNT_CRECONECT			312	// �N���W�b�g�ڑ��m�F�i�g���@�\�j
#define	MNT_CREUNSEND			313	// �N���W�b�g�����M����˗��f�[�^�i�g���@�\�j
#define	MNT_CRESALENG			314	// �N���W�b�g���㋑�ۃf�[�^�i�g���@�\�j


/* upload(download) data (2) : 320-329 */
enum {
	LD_EDYAT = 320,
	LD_USERDEF,
	LD_LOG_DATA,
	LD_ERR,
	LD_ARM,
	LD_MONI,
	LD_OPE_MONI,
	LD_PAY_CAL,				// �����v�Z�p�̃f�o�b�O����
	LD_CAR_INFO,			// �Ԏ����
	LD_TCARD_FTR,			// T�J�[�h�t�b�^�[
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
	LD_CREKBR_FTR,			// �N���W�b�g�ߕ����t�b�^�[
	LD_EPAYKBR_FTR,			// �d�q�}�l�[�ߕ����t�b�^�[
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
	LD_FUTURE_FTR,			// ����x���z�t�b�^�[
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	LD_EMG,					// ��Q�A���[�t�b�^�[
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	LD_AZU_FTR = 500,		// �a��؃t�b�^�[
	LD_REMOTE_RES,			// ���u�_�E�����[�h����
	LD_PROG,
	LD_PARA_S,
	LD_VOICE_A,
	LD_MAIN_S,
	LD_VOICE_S,
	LD_AU_SCR,
	LD_CHKMODE_RES,
	LD_REMOTEDL,
	LD_PARAM_UP,
	LD_CONNECT,
// MH810100(S) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
	LD_PARAM_UP_LCD,
// MH810100(E) K.Onodera 2019/11/20 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)

	__ldfile_max
};

#define	CCOM_DTCLR				330		// �����M�f�[�^�N���A
#define	CCOM_FLSH				331		// �����M�f�[�^���M
#define	CCOM_TEST				332		// �Z���^�[�ʐM�e�X�g
#define	CCOM_APN				333		// �ڑ���`�o�m�m�F
#define	CCOM_APN_AMS			334		// �ڑ���I���i�H�ꌟ���j�`�l�r
#define	CCOM_APN_AMN			335		// �ڑ���I���i�H�ꌟ���j�`�l�`�m�n
#define	CCOM_APN_PWEB			336		// �ڑ���I���i�H�ꌟ���j���ԏ�Z���^�[�i�����p�j
#define	CCOM_ATN				337		// �d�g��M��Ԋm�F
#define	CCOM_OIBANCLR			338		// �Z���^�[�ǔԃN���A
#define	CCOM_REMOTE_DL			339		// ���u�_�E�����[�h�m�F

#define	PIP_TEST				340		// ��ڑ��ʐM�e�X�g
#define	PIP_DTCLR				341		// �f�[�^�N���A

#define	CREDIT_UNSEND			343		// �����M����f�[�^
#define	CREDIT_SALENG			344		// ���㋑�ۃf�[�^

/* new add No.380�` */
// Menu_Slt()��menu_tbl�ւ̑Ή�
enum {
	MNT_INV_MONEY = 380,	// No.380
	MNT_INV_KIND,			// No.381
	MNT_INV_ALL,			// No.382
	FISMF1_CHK,				// No.383
	FISMF2_CHK,				// No.384
	FISMF3_CHK,				// No.385
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	FEDY1_CHK,				// No.386
//	FEDY2_CHK,				// No.387
//	FEDY3_CHK,				// No.388
//	FEDY4_CHK,				// No.389
//	FEDY5_CHK,				// No.390
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	FPIP1_CHK,				// No.391
	FPIP2_CHK,				// No.392
	FAPS1_CHK,				// No.393
	FAPS2_CHK,				// No.394
	// ���[�U�[�����e�i���X�̃��j���[�������������Đ݌v�����̔ԍ��Ǘ��ł�
	// �Ή��ł��Ȃ��Ȃ��Ă��Ă���B�{���Ȃ�΃��[�U�[�����e��0�`69�̉ӏ���
	// �錾���������A���łɈ�t�ɂȂ��Ă���̂ŁA�����ɒ�`����B
	// ����A�������K�v������B
	MNT_CREUPDATE,			// �J�[�h���f�[�^�v���i�g���@�\�j
	MNT_NGLOG,				// �s����
	MNT_IOLOG,				// ���o��
	MNT_PRISET,				// �����ݒ胁�j���[ �v�����g
	MNT_CRR_VER,			// CRR�o�[�W�����`�F�b�N
	MNT_CRR_TST,			// CRR�܂�Ԃ��e�X�g
	MNT_MAFCOMCHK,			// ��ڑ��ʐM�e�X�g
	MNT_MAFDATACLR,			// �f�[�^�N���A
	MNT_MAF_RISM_COMCHK,	// Rism�T�[�o�[�ڑ��m�F
	MNT_CAPPI_LOOPBACK,		// �ʐM�܂�Ԃ��e�X�g
	MNT_CAPPI_RESET,		// �ǔԃ��Z�b�g
	MNT_CAPPI_COMCHK,	    // Cappi�T�[�o�[�ڑ��m�F
	MNT_WCARD_INF,
	MNT_HOJIN,
	CAR_FUNC_SELECT,		// Car Function Select
	MNT_PARAM_LOG,
	FTP_CONNECT_CHK,		// FTP�ڑ��`�F�b�N
	FTP_PARAM_UPL,			// ���ʃp�����[�^�A�b�v���[�h
	FTP_CONNECT_FUNC,		// �ڑ��m�F
	LAN_CONNECT_CHK,		// LAN�ڑ��`�F�b�N
	CHK_PRINT,
	MNT_FLP_LOOP_DATA_CHK,	// ���[�v�f�[�^�m�F
	MNT_FLP_SENSOR_CTRL,	// �ԗ����m�Z���T�[����
	MNTLOG1_CHK,			// 
	MNT_DLOCK	,			// �d�����b�N�Ή�
	MNT_SUICALOG_INJI,		// Suica�ʐM���O��(�S�̃��O)
	MNT_SUICALOG_INJI2,		// Suica�ʐM���O��(���߃��O)
	MNT_CERTI_SET,
	MNT_CERTI_ISSU,
	MNT_RYOTS,				// �����e�X�g
	MNT_MTOTL,				// �l�s�W�v
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
	CCOM_LONG_PARK_ALLREL,	// �������ԑS����
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	MNT_ECR,				// ���σ��[�_����
	MNT_ECR_SHUUKEI,		// �����W�v�v�����g
	MNT_ECR_MEISAI,			// ���p���׃v�����g
	ECR_BRAND_COND,			// �u�����h��Ԋm�F
	MNT_ECR_ALARM_LOG,		// ������������L�^
	ECR_CHK,				// ���σ��[�_�`�F�b�N
	ECR_MNT,				// ���[�_�����e�i���X
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810100(S) Y.Yamauchi 2019/10/15 �Ԕԃ`�P�b�g���X(�����e�i���X)
	QR_CHK,					// �p�q���[�_�[�`�F�b�N
	QR_READ,				// �ǎ�e�X�g�@�@�@�@�@�@�@�@
	QR_VER,					// �o�[�W�����m�F�@�@�@�@�@�@
	REAL_CHK,				// ���A���^�C���ʐM�`�F�b�N
	UNSEND_REAL_CLEAR,			// �����M�f�[�^�N���A 
	CENTER_REAL_TEST,			// �Z���^�[�ʐM�e�X�g
	REAL_OIBANCLR,				// �Z���^�[�ǔԃN���A
	DC_CHK,					// DC-NET�ʐM�`�F�b�N
	DC_UNSEND_CLEAR,			// �����M�f�[�^�N���A
	DC_CONNECT_TEST,			// �Z���^�[�ʐM�e�X�g
	DC_OIBANCLR,				// �Z���^�[�ǔԃN���A
// MH810100(E) Y.Yamauchi 2019/10/15 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	MNT_ECR_MINASHI,		// �݂Ȃ����σv�����g
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	FPRNEJ_CHK,				// �d�q�W���[�i���`�F�b�N
	FPRN_RW_CHK,			// ���[�h���C�g�e�X�g
	FPRN_SD_CHK,			// �r�c�J�[�h���
	FPRN_VER_CHK,			// �o�[�W�����m�F
	MNT_EJ,					// �d�q�W���[�i��
	MNT_EJ_SD_INF,			// �r�c�J�[�h���
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
};

/* User Maintenance Edy 350-360 */
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define	MNT_EDY					350	// �d��������
//#define	MNT_EDY_ARM				351	// �A���[��������
//#define	MNT_EDY_CENT			352	// �Z���^�[�ʐM�J�n
//#define	MNT_EDY_SHIME			353	// �d�������ߋL�^���
//#define	MNT_EDY_TEST			354	// TEST�p
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

#define	FILE_SERI				360		// �V���A���ڑ�
#define	FILE_FTP_CLI			361		// FTP�ڑ�(�N���C�A���g)
#define	FILE_FTP_SEV			362		// FTP�ڑ�(�T�[�o�[)

#define	SYS_MAIN_PRO			365		// ���C���v���O����
#define	SYS_SUB_PRO				366		// �T�u�v���O����
#define	SYS_COM_PRM				367		// ���ʃp�����[�^�[

// �W���L���f�[�^�o�^��
#define	MNT_ATTEND_REGSTER_COUNT	10	// ��ʂœo�^�\�ȃf�[�^��
#define MNT_MIN(a,b)	((a) < (b) ? (a) : (b))

#define	PRM_LOKNO_MIN	100	// ���u���A�Ԃ̗L���͈͂̉���
#define	PRM_LOKNO_MAX	315	// ���u���A�Ԃ̗L���͈͂̏��

//---------------------------------
// ���ް��\���p
//---------------------------------
struct CrServiceRec {
	ushort	ParkNo;				// ���ԏ�No.
	ushort	Kind;
	ushort	ShopNo;
	uchar	StartDate[3];
	uchar	EndDate[3];
	ushort	Status;
};

struct CrPrepaidRec {
	ushort	ParkNo;				// ���ԏ�No.
	ushort	Mno;
	uchar	IssueDate[3];
	ulong	SaleAmount;
	ulong	RemainAmount;
	ushort	LimitAmount;
	ulong	CardNo;
};

struct CrPassRec {
	ushort	ParkNo;
	ushort	Code;
	ushort	Kind;
	uchar	StartDate[3];
	uchar	EndDate[3];
	ushort	Status;
	uchar	WriteDate[4];
};
struct CrKaitikRec {
	ushort	ParkNo;				// ���ԏ�No.
	ushort	TanRyo;				// �P�ʋ��z
	ushort	LimDosu;			// �����x��
	uchar	StartDate[3];		// �L���J�n��
	uchar	EndDate[3];			// �L���I����
	ushort	Kaisu;				// �񐔁i�c��j
	uchar	WriteDate[4];		// ��������
};

extern union CrDspRec {

	struct	CrServiceRec service;
	struct	CrPrepaidRec prepaid;
	struct	CrPassRec pass;
	struct	CrKaitikRec	kaiticket;

} cr_dsp;

typedef struct{
	short	Section;							// ���ʃp�����[�^�̃Z�N�V�����ԍ�
	short	Address;							// ���ʃp�����[�^�̃A�h���X�ԍ�
	char	Assign;								// ��r����ʒu
	long	Value;								// ��r����l
}CeMenuPara;

union ioLogNumberInfo{
	ulong		n;
	struct{
		ulong	sdat:13;		// �ŌÓ������ۑ��̈�
		ulong	edat:13;		// �ŐV�������ۑ��̈�
		ulong	evnt: 6;		// �ŐV���۔��������ۑ��ʒu
	}log;
};

//---------------------------------
// ���샍�O�o�^�p
//---------------------------------
extern ushort	OpelogNo;

extern ushort	OpelogNo2;


/* error information */
typedef struct {
	long	errmsg;			/* error message to display */
	ulong	address;		/* detail of error */
}t_SysMnt_ErrInfo;

/* error message number */
#define	_SYSMNT_ERR_NONE			27
#define	_SYSMNT_ERR_COM				29
#define	_SYSMNT_ERR_NO_DATA			30
#define	_SYSMNT_ERR_INVALID_DATA	31
#define	_SYSMNT_ERR_DATA_LOCKED		32
#define	_SYSMNT_ERR_WRITE			33
#define	_SYSMNT_ERR_ERASE			34
#define	_SYSMNT_ERR_VERIFY			36

extern	void	sysmnt_Backup(t_SysMnt_ErrInfo *errinfo);
extern	void	sysmnt_Restore(t_SysMnt_ErrInfo *errinfo);
extern	ushort	parameter_upload_chk(void);

//---------------------------------
// �ݒ�FLASH���������p
//---------------------------------
extern ushort	SetChange;
#define		SysMnt_Work_Size		65536

typedef struct {
	long	from;	// �}�X�N�J�n�A�h���X
	long	to;		// �}�X�N�I���A�h���X
} PRM_MASK;

extern	const	PRM_MASK param_mask[];		// �ݒ�S��
extern	const	PRM_MASK param_mask31_0[];	// �����
extern	const	PRM_MASK param_mask31_1[];	// ������

extern	const	short	prm_mask_max;	// �}�X�N�f�[�^����
extern	const	short	prm_mask31_0_max;	// �}�X�N�f�[�^�����F�����
extern	const	short	prm_mask31_1_max;	// �}�X�N�f�[�^�����F������

// MH810100(S) Y.Yamauchi 2019/10/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
//---------------------------------
// QR�o�[�W�����\���p
//---------------------------------
enum{
	QR_HINBAN,
	QR_SERIAL,
	QR_VERSION,
	QR_VER_MAX,
};
#define QR_VER_RESULT_MAX				QR_VER_MAX

// GG120600(S) // Phase9 ���u�����e�i���X�p
//typedef enum{
//	FTP_REQ_NONE,			// 0:�v���Ȃ�
//	FTP_REQ_NORMAL,			// 1:FTP�v������
//	FTP_REQ_WITH_POWEROFF,	// 2:FTP�v������(�d�f�v)
//} eFTP_REQ_TYPE;
// GG120600(E) // Phase9 ���u�����e�i���X�p

//---------------------------------
// QR�f�[�^�m�F�p
//---------------------------------
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
////������
// enum{
//	QR_DISCOUNT_MAKE_DATE,	// QR���s�N/��/��
//	QR_DISCOUNT_MAKE_TIME,	// QR���s��/��/�b
//	PARKING_LOT,	// ���ԏ�Lot�ԍ�
//	DISCOUNT_KIND,	// �������
//	DISCOUNT_CLASS,	// �����敪
//	SHOP_NUMBER,	// �X�ԍ�
//	CARD_START_DATE,		// �L���J�n�N/��/��
//	CARD_START_HOUR,		// �L���J�n��/��/�b
//// MH810100(S) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
//	FACILITY_CODE,	// �{�݃R�[�h
//	SHOP_CODE,		// �X�܃R�[�h
//	POS_NUMBER,		// �X�ܓ��[���ԍ�
//	ISSUE_NUMBER,	// ���V�[�g���s�ǂ���
//// MH810100(E) Y.Yoshida 2020/06/11 �������ɍ��ڒǉ�(#4206 QR�t�H�[�}�b�g�ύX�Ή�)
//
//	QR_DISCOUNT_DATA_MAX
// };
////���㌔
// enum{
//	QR_AMOUNT_MAKE_DATE,	// QR���s�N/��/��
//	QR_AMOUNT_MAKE_TIME,	// QR���s��/��/�b
//	FACILITY_NUMBER,// �{�ݔԍ�
//	QR_AMOUNT_SHOP_NUMBER,	// �X�ԍ�
//	MACHINE_NUMBER,	// �X�ܓ��[���ԍ�
//	RECIETE_NUMBER,	// ���V�[�g���s�ǂ���
//	QR_AMOUNT_HEAD = RECIETE_NUMBER,
//	BARCODE_TEYPE_1, // �o�[�R�[�h�^�C�v�P
//	DATA_1,			// �f�[�^�P
//	BARCODE_TEYPE_2, // �o�[�R�[�h�^�C�v�Q
//	DATA_2,			// �f�[�^�Q
//	BARCODE_TEYPE_3, // �o�[�R�[�h�^�C�v�R
//	DATA_3,			// �f�[�^�R
//	QR_AMOUNT_DATA_MAX
// };
// #define	BAR_DATA_OFFSET		(DATA_1 - BARCODE_TEYPE_1)
#define	BAR_DATA_OFFSET	(2)
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P

// GG120600(S) // Phase9 ���u�����e�i���X�p
//extern void mnt_SetFtpFlag( eFTP_REQ_TYPE req );
// GG120600(E) // Phase9 ���u�����e�i���X�p
extern eFTP_REQ_TYPE mnt_GetFtpFlag( void );

typedef struct {
	ushort				err;		// �G���[
	ushort				id;			// QR�R�[�hID
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	ushort				type;		// QR�R�[�h�t�H�[�}�b�g�^�C�v
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
	lcdbm_rsp_QR_com_u	QR_Data;	// QR���(�p�[�X�ς�)
} _backdata;

typedef struct {
	int			cnt;
	_backdata	backdata[5];	// �ߋ��f�[�^
} _ticketdata;
extern _ticketdata	ticketdata;

// MH810100(E) Y.Yamauchi 2019/10/10 �Ԕԃ`�P�b�g���X(�����e�i���X)
#endif	// _MNT_DEF_H_
