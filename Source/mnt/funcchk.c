/*[]----------------------------------------------------------------------[]*/
/*| ��������ݽ������������                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Y.Tanaka                                                 |*/
/*| Date        : 2005.04.12                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include	<string.h>
#include	<stdio.h>
#include	<trsocket.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"Message.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"mdl_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"tbl_rkn.h"
#include	"Lkcom.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnetauto.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"ifm_ctrl.h"
#include	"appserv.h"
#include	"fla_def.h"
#include	"ksg_def.h"
#include	"rauconstant.h"
#include	"raudef.h"
#include	"pip_def.h"
#include	"ntnet_def.h"
#include	"cre_ctrl.h"
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#include	"ntcom.h"
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#include	"FlashSerial.h"
#include	"updateope.h"

/*	CRW�ŃT�uCPU���T�|�[�g�����ňȉ��̑���_������܂��B
	�@�������`�F�b�N
	�@�T�uCPU�������`�F�b�N�ɑΉ�
	�ANT-NET�`�F�b�N
	�@IBK�^�T�uCPU�̂Q�o�H�ɑΉ�
	�B�o�[�W�����`�F�b�N
	�@���C���A�T�u�o�[�W�����̕\��
	�C�T�uCPU�ėp�ʐM���C���`�F�b�N
	CRW�����͌���GT-7000����(�ꕔ�s�v�ȉߋ������͍폜����)���̂܂܈ڐA���܂����B
*/
#include	"lkmain.h"
#include	"remote_dl.h"
#include	"IFM.h"
#include	"flp_def.h"
#include	"toSdata.h"

/*----------------------------------------------------------------------*/
/*		�֐��������ߐ錾												*/
/*----------------------------------------------------------------------*/
ushort	FunChk_Sig( void );
static void	inSigStsDsp( uchar req, uchar ionsts );				// ���͐M����ԕ\��
static void	outSigStsDsp( uchar *ionsts );						// �o�͐M����ԕ\��
ushort	FunChk_Swt( void );
ushort	FunChk_Kbd( void );
ushort	FunChk_Sht( void );
ushort	FunChk_Led( void );
ushort	FunChk_Prt( void );
ushort	FunChk_JRPrt( ushort event );
ushort	FPrnChk_tst( ushort event );
ushort	FPrnChk_ryo_tst( void );						//�̎��؈�ý�
ushort	FPrnChk_tst_rct( void );						//ڼ��ýĈ��(�ƭ-)
void FPrnChk_Testdata_mk( char sw,Receipt_data *dat );	//ýĈ󎚗p�[���ް��쐬
ushort	FPrnChk_sts( ushort event );
ushort	FPrnChk_logo( ushort event );
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
ushort	FPrnChk_RW(void);
ushort	FPrnChk_SD(void);
ushort	FPrnChk_Ver(void);
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
ushort	FunChk_Rdr( void );
ushort	FRdrChk_RW( void );
ushort	FRdrChk_Pr( void );
ushort	RdrChk_ErrDsp( uchar ercd, ulong errbit );
void	Rdr_err_dsp( uchar pos, uchar *data );
ushort	FRdrChk_Sts( void );
void	Rdr_sts_dsply( uchar pos1, uchar pos2, uchar *sta );
ushort	FRdrChk_Ver( void );
ushort	FunChk_Cmc( void );
ushort	CmcInchk( void );
void	InchkDsp( void );
ushort	CmcInchk2( uchar* );
uchar	bcdadd( uchar data1, uchar data2 );
ushort	CmcOutchk( void );
ushort	CmcStschk( void );
void	Cmc_err_dsp( uchar pos, uchar *data );
ushort	CmcVerchk( void );
ushort	FunChk_Ann( void );
ushort	Ann_chk2(void);
ushort	FunChk_BNA( void );
ushort	BnaInchk( void );
ushort	BnaStschk( void );
ushort	BnaVerchk( void );
ushort	FunChk_Mck( void );
ushort	FunChk_Ifb( uchar );
ushort	FunChk_CRRVer( void );
ushort	FunChk_CrrComChk( void );
ushort	FunChk_Version( void );
ushort FncChk_Suica( void );
ushort Com_Chk_Suica( void );
ushort Log_Print_Suica( uchar print_kind );
ushort Log_Print_Suica_menu( void );
ushort Log_Print_Suica2( void );
ushort	FncChk_CCom( void );
ushort	FncChk_bufctrl( int type );
ushort	dsp_unflushed_count(uchar type, uchar page);
void	clr_dsp_count(uchar page);
uchar	dsp_ccomchk_cursor(int index, ushort mode, uchar page);
ushort FncChk_ccomtst( void );
ushort FncChk_CcomApnDisp( void );
ushort FncChk_CcomAntDisp( void );
static void antsetup_dsp( short * dat, short pos );
ushort FncChk_cOibanClr( void );
void cOibanClrDsp( void );
void cOibanCursorDsp( uchar index, ushort mode );
ushort FncChk_CcomApnSelect( void );
ushort FncChk_CcomApnSet( ushort );
ushort FncChk_CComRemoteDLChk(void);
static unsigned short	FRdrChk_DipSwitch( void );
void	FRdrChk_Wait( void );
ushort Ryokin_Setlog_Print( void );
unsigned short	FucChk_DebugMenu(void);
unsigned short	Fchk_TimeCpuDnmDisp( void );
void	FunChk_TimeCpuEepromDumpDisp( void );
unsigned short	Fchk_Stack_StsPrint( void );
void	Fchk_Stak_chek( void );
ushort	FncChk_PiP(void);
ushort	FncChk_pipclr(void);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
ushort	FunChk_Ntnet( void );
ushort	check_nt_dtclr( void );
void	DataCntDsp( ushort dataCnt, ushort type, ushort	line );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//ushort FncChk_Cre( void );
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH810100(S) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)
ushort  FncChk_QRchk( void );
// MH810100(E) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH810100(S) Y.Yamauchi 20191010 �Ԕԃ`�P�b�g���X(�����e�i���X)	
ushort	FncChk_QRread( void );
ushort	FncChk_QRinit(void);
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
// static ushort	FncChk_QRread_result(uchar result);
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
// static ushort FncChk_QRread_result(ushort updown_page);
static ushort FncChk_QRread_result(ushort updown_page, ushort updown_page_max);
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
ushort	FncChk_QRverChk(void);
static ushort	FncChk_QRverchk_result(uchar result);
ushort	FncChk_QRinitChk(void);
ushort	FncChk_RealChk(void);
ushort	RealT_FncChk_Unsend(int type);
ushort FncChk_CenterChk(void);
static ushort FncChk_realt_OibanClear(void);
ushort real_dsp_unflushed_count(uchar type, uchar page);
void   realt_clr_dsp_count(uchar page);
uchar realt_dsp_ccomchk_cursor(int index, ushort mode, uchar page);
static ushort FncChk_DC_Chk(void);
static ushort FncChk_DC_UnsendClear(void);
static ushort FncChk_DC_ConnectCheck( void );
static ushort FncChk_DC_OibanClear( void );
// MH810100(E) Y.Yamauchi 20191010 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//static ushort	FncChk_CreUnSend( void );
//static void cre_unsend_data_dsp( struct DATA_BK * );
//static short cre_unsend_del_dsp( char );
//static ushort	FncChk_CreSaleNG( void );
//void	FncChk_CreSaleNG_fnc( char );
//static void cre_saleng_show_data( short );
//static void cre_saleng_num_dsp( uchar, char );
//static void	time_dsp2( ushort, ushort, ushort, ushort, ushort );
//static void cre_saleng_data_dsp( short );
//static void	date_dsp3( ushort, ushort, ushort, ushort, ushort, ushort );
//static short cre_saleng_del_dsp( short, char );
//static void	CRE_SaleNG_Delete( void );
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
static ushort	FncChk_CComLongParkAllRel( void );
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
static ushort FncChk_EcReader( void );
static unsigned short EcReaderBrandCondition( void );
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//static void 	dsp_brand_condition( int index );
static ushort	dsp_brand_condition(int index);
static ushort	EcReaderSubBrandCondition(uchar sub_idx);
static void		dsp_sub_brand_condition(uchar sub_idx, uchar current_page, uchar max_page);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
static ushort	EcReader_Mnt(void);
static uchar	EcReader_Mnt_CommResult( uchar mode );
static void 	EcReader_Mnt_ResetDisp(void);
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

enum {
// MH810100(S) Y.Yamauchi 2019/10/29 �Ԕԃ`�P�b�g���X(�����e�i���X)	
//	CCOM_DATA_INCAR,			// ����
// MH810100(E) Y.Yamauchi 2019/10/29 �Ԕԃ`�P�b�g���X(�����e�i���X)
	CCOM_DATA_TOTAL,			// �W�v
	CCOM_DATA_SALE,				// ���Z
	CCOM_DATA_COIN,				// �R���Ɍv
	CCOM_DATA_ERR,				// �װ
	CCOM_DATA_NOTE,				// �����Ɍv
	CCOM_DATA_ALARM,			// �װ�
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	CCOM_DATA_CAR_CNT,			// ���ԑ䐔
//	CCOM_DATA_MONITOR,			// ����
//	CCOM_DATA_MONEY, 			// ���K�Ǘ�
//	CCOM_DATA_OPE,				// ����
//	CCOM_DATA_TURI, 			// �ޑK�Ǘ�
//	CCOM_DATA_RMON,				// ���u�Ď�
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	CCOM_DATA_LPARK,			// ��������
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	CCOM_DATA_MONEY, 			// ���K�Ǘ�
	CCOM_DATA_MONITOR,			// ����
	CCOM_DATA_TURI, 			// �ޑK�Ǘ�
	CCOM_DATA_OPE,				// ����
	CCOM_DATA_RMON,				// ���u�Ď�
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	CCOM_DATA_MAX
};
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//#define CCOM_EXIST_DATA_MAX		(CCOM_DATA_OPE+1)
#define CCOM_EXIST_DATA_MAX		CCOM_DATA_OPE	// Phase1�ɂ͒ޑK�Ǘ����Ȃ����߁u+1�v���s�v
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH810100(S) Y.Yamauchi 2019/10/29 �Ԕԃ`�P�b�g���X(�����e�i���X)	
enum{
	RTCOM_DATA_PAY,				// RT���Z�f�[�^
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	RTCOM_DATA_RECEIPT,			// RT�̎��؃f�[�^
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	RT_DATA_MAX
};
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// #define RT_EXIST_DATA_MAX		(RTCOM_DATA_PAY+1)
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
enum{
	DC_DATA_QR,					// QR�m��E����f�[�^
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	DC_DATA_LANE,				// ���[�����j�^�f�[�^
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	DC_DATA_MAX
};
// MH810100(E) Y.Yamauchi 2019/10/29 �Ԕԃ`�P�b�g���X(�����e�i���X)	
CRE_SALENG	cre_saleng_work;				// ���㋑���ް��P�O�����ҏW�p���[�N

t_NtBufCount IBK_BufCount;					// IBK�e�[�u���f�[�^����
uchar	IBK_ComChkResult;					// �Z���^�[�ʐM�e�X�g���ʃR�[�h

static ulong PreCount[CCOM_DATA_MAX];		// IBK�e�[�u���f�[�^�����X�V�p
// MH810100(S) Y.Yamauchi 2019/10/29 �Ԕԃ`�P�b�g���X(�����e�i���X)	
static ulong RT_PreCount[RT_DATA_MAX];		// ���A���^�C���ʐM�e�[�u���f�[�^�����X�V�p
static ulong DC_PreCount[DC_DATA_MAX];		// ���A���^�C���ʐM�e�[�u���f�[�^�����X�V�p
// MH810100(E) Y.Yamauchi 2019/10/29 �Ԕԃ`�P�b�g���X(�����e�i���X)	

static ushort	MemoryCheck(void);
static void		FunChk_Draw_SubCPU_Version(void);

static uchar	wlcd_buf[31];
static Receipt_data	TestData;				// �[�����Z�����f�[�^�쐬�ر
static uchar	APN_str[33];
static uchar	IP_str[20];
const unsigned char APN_TBL[][33]={
	"parking.ams-amano.co.jp         ",		//�`�l�r�p�`�o�m 
	"sf.amano.co.jp                  ",		//�A�}�m�p�`�o�m
	"parkingweb.jp                   ",		//���ԏ�Z���^�[�p�`�o�m
								};
ushort FncChk_Cappi( void );
ushort	check_cappi_seqclr( void );


static ushort FunChk_CrrBoardCHK( void );
static ushort FunChk_FlpCHK( void );

static ushort	CarFuncSelect( void );
ushort	Ftp_ConnectChk( void );
ushort	ParameterUpload( uchar );
static ushort	Lan_Connect_Chk( void );
static int		pingStart (short * dat);
void pingOpenCB (int socketDescriptor);
static void		Lan_Connect_dsp( void );
static void		ping_dsp( short * dat, short pos );
static ushort	FunChk_FlapSensorControl( void );
static ushort	FunChk_FlapLoopData( void );
static ushort	FunChk_FUNOPECHK( void );
unsigned short avm_test_no[2];
char	avm_test_ch;
short	avm_test_cnt;
// MH810100(S) S.Fujii 2020/07/15 �Ԕԃ`�P�b�g���X(�`�F�b�N�T���\��)
static ushort	GetFromCheckSum( void );
#define	FROM_IN_SECTOR_SIZE		4096			// CPU�v���O�����t���b�V���Z�N�^�T�C�Y(byte)
#define FROM_IN_ADDRESS_START	0xFFE00000L		// ����FROM �v���O�����̈�擪�A�h���X
#define FROM_IN_ADDRESS_END		0xFFFFFFFFL		// ����FROM �v���O�����̈�ŏI�A�h���X
#define	FROM_PROG_SIZE			FROM_IN_ADDRESS_END - FROM_IN_ADDRESS_START + 1	// �v���O�����̈�T�C�Y
static uchar ProgramRomBuffer[FROM_IN_SECTOR_SIZE];
// MH810100(E) S.Fujii 2020/07/15 �Ԕԃ`�P�b�g���X(�`�F�b�N�T���\��)

/*[]----------------------------------------------------------------------[]*/
/*| ��������ݽ��������                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChkInit( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void FncChkInit( void )
{
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ��������ݽҲݏ���                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChkMain( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short FncChkMain( void )
{
	unsigned short	usFncEvent;
	char	wk[2];
	char	org[2];
	uchar	i;
	uchar	crr_check;

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	FncChkInit();
	// �L����CRR�������̂��`�F�b�N
	crr_check = 0;
	for (i=0; i < IFS_CRR_MAX; i++) {
		if ( 1 == IFM_LockTable.sSlave_CRR[i].bComm ){
			crr_check = 1;	// �L����CRR�����
			break;
		}
	}

	for( ; ; )
	{

		dispclr();
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[00] );			// [00]	"������`�F�b�N���@�@�@�@�@�@�@"

// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�`�F�b�N�ǉ�)
//		usFncEvent = Is_MenuStrMake( 0 );
		if( isEC_USE() ){
			// ���σ��[�_�ڑ�����
			usFncEvent = Is_MenuStrMake( 2 );
		} else {
			// �]��
			usFncEvent = Is_MenuStrMake( 0 );
		}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�`�F�b�N�ǉ�)
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		OpelogNo = 0;
		SetChange = 0;

		switch( usFncEvent ){

			case SWT_CHK:						// �@�P�D�X�C�b�`�`�F�b�N
				wopelg( OPLOG_SWCHK, 0, 0 );			// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 6;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Swt();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
//���g�p�ł��B(S)
			case KBD_CHK:						// �@�L�[���̓`�F�b�N
				wopelg( OPLOG_KEYCHK, 0, 0 );			// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 1;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Kbd();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
			case LCD_CHK:						// �@�k�b�c�`�F�b�N
				wopelg( OPLOG_LCDCHK, 0, 0 );			// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 2;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Lcd();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
//���g�p�ł��B(E)
			case LED_CHK:						// �@�k�d�c�`�F�b�N
				wopelg( OPLOG_LEDCHK, 0, 0 );			// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 3;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Led();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
//���g�p�ł��B(S)
			case SHT_CHK:						// �@�V���b�^�[�`�F�b�N
				wopelg( OPLOG_SHUTTERCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 7;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Sht();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
//���g�p�ł��B(E)
			case SIG_CHK:						// �@���o�͐M���`�F�b�N
				wopelg( OPLOG_SIGNALCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 8;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Sig();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;

			case CMC_CHK:						// �@�R�C�����b�N�`�F�b�N
				wopelg( OPLOG_COINMECHCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 5;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Cmc();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;

			case BNA_CHK:						// �@�������[�_�[�`�F�b�N
				wopelg( OPLOG_NOTEREADERCHK, 0, 0 );	// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 4;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_BNA();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;

			case PRT_CHK:						// �@�v�����^�`�F�b�N
				wopelg( OPLOG_PRINTERCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 9;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Prt();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;

// MH810100(S) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case QR_CHK:						// �@�p�q���[�_�[�`�F�b�N
				wopelg( OPLOG_QRCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 16;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FncChk_QRchk();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
// MH810100(E) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)


//���g�p�ł��B(S)
			case RDR_CHK:						//	���C���[�_�[�`�F�b�N
				wopelg( OPLOG_READERCHK, 0, 0 );		// ���엚��o�^
				if( prm_get( COM_PRM,S_PAY, 21, 1, 3 ) != 0 ){		// ���Cذ�ް�ڑ��L�H

					// �ڑ�����
					OPECTL.Ope_Mnt_flg	= 10;	// ���ڰ�������ݽ�׸޾��
					usFncEvent = FunChk_Rdr();
					OPECTL.Ope_Mnt_flg	= 0;	// ���ڰ�������ݽ�׸�ؾ��
				}
				else{
					// �ڑ��Ȃ�
					BUZPIPI();					// �ڑ��Ȃ��̏ꍇ�A���Cذ�ް�����s�Ƃ���
				}
				break;
//���g�p�ł��B(E)
			case ANN_CHK:						// 	�A�i�E���X�`�F�b�N
				wopelg( OPLOG_ANNAUNCECHK, 0, 0 );	// ���엚��o�^

					// �ڑ�����
					OPECTL.Ope_Mnt_flg	= 11;	// ���ڰ�������ݽ�׸޾��
					usFncEvent = FunChk_Ann();
					OPECTL.Ope_Mnt_flg	= 0;	// ���ڰ�������ݽ�׸�ؾ��
				break;

			case MEM_CHK:						// �������[�`�F�b�N
				wopelg( OPLOG_MEMORYCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 12;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Mck();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
//���g�p�ł��B(S)
			case LCK_CHK:						// ���b�N���u�`�F�b�N
				wopelg( OPLOG_IFBOARDCHK, 0, 0 );		// ���엚��o�^
				if( GetCarInfoParam() & 0x01 ){		// ���֏ꕹ�݂���
					OPECTL.Ope_Mnt_flg	= 13;	// ���ڰ�������ݽ�׸޾��
					usFncEvent = FunChk_Ifb( 0 );
					OPECTL.Ope_Mnt_flg	= 0;	// ���ڰ�������ݽ�׸�ؾ��
				}else{
					// �ڑ��Ȃ�
					BUZPIPI();					// �ڑ��Ȃ��̏ꍇ�Aۯ����u�����s�Ƃ���
				}
				break;
//���g�p�ł��B(E)
			case CCM_CHK:							// �Z���^�[�ʐM�`�F�b�N
				wopelg( OPLOG_CCOMCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 21;			// ���ڰ�������ݽ�׸޾��
				usFncEvent = FncChk_CCom();
				OPECTL.Ope_Mnt_flg	= 0;			// ���ڰ�������ݽ�׸�ؾ��
				break;
// MH810100(S) Y.Yamauchi 2019/10/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case REAL_CHK:							// ���A���^�C���ʐM�`�F�b�N
				wopelg( OPLOG_REALCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 27;			// ���ڰ�������ݽ�׸޾��
				usFncEvent = FncChk_RealChk();
				OPECTL.Ope_Mnt_flg	= 0;			// ���ڰ�������ݽ�׸�ؾ��
				break;

			case DC_CHK:							// DC-NET�ʐM�`�F�b�N
				wopelg( OPLOG_DCLCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 29;			// ���ڰ�������ݽ�׸޾��
				usFncEvent = FncChk_DC_Chk();
				OPECTL.Ope_Mnt_flg	= 0;			// ���ڰ�������ݽ�׸�ؾ��
				break;
// MH810100(E) Y.Yamauchi 2019/10/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case VER_CHK:						// �o�[�W�����`�F�b�N
				wopelg( OPLOG_VERSIONCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 20;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Version();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
				break;

//���g�p�ł��B(S)
			case FLP_CHK:							//�t���b�v���u�`�F�b�N
				if ( GetCarInfoParam() & 0x04 ){	// �t���b�v�ڑ��ݒ�L��H
					wopelg( OPLOG_IFBOARDCHK, 0, 0 );		// ���엚��o�^
					OPECTL.Ope_Mnt_flg	= 14;		// ���ڰ�������ݽ�׸޾��
					usFncEvent = FunChk_FlpCHK();
					OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				}else{
					BUZPIPI();					
				}
				break;
//			case CRR_CHK:						//�b�q�q�`�F�b�N
//				if ( GetCarInfoParam() & 0x04 ){// CRR�ڑ��L��H */
//					OPECTL.Ope_Mnt_flg	= 25;	// ���ڰ�������ݽ�׸޾��
//					usFncEvent = FunChk_CrrBoardCHK();
//					OPECTL.Ope_Mnt_flg	= 0;	// ���ڰ�������ݽ�׸�ؾ��
//				}else{
//					BUZPIPI();					
//				}
//				break;
			case CRR_CHK:						// �t���b�v�����`�F�b�N
				if ( 1 == crr_check ){			// CRR�ڑ��L��H */
					OPECTL.Ope_Mnt_flg	= 25;	// ���ڰ�������ݽ�׸޾��
					usFncEvent = FunChk_CrrBoardCHK();
					OPECTL.Ope_Mnt_flg	= 0;	// ���ڰ�������ݽ�׸�ؾ��
				}else{
					BUZPIPI();					
				}
				break;
//���g�p�ł��B(E)

			case CAR_FUNC_SELECT:
				OPECTL.Ope_Mnt_flg	= 26;	// ���ڰ�������ݽ�׸޾��
				usFncEvent = CarFuncSelect();
				OPECTL.Ope_Mnt_flg	= 0;	// ���ڰ�������ݽ�׸�ؾ��
				break;
			case LAN_CONNECT_CHK:
				OPECTL.Ope_Mnt_flg	= 28;	// ���ڰ�������ݽ�׸޾��
				usFncEvent = Lan_Connect_Chk();
				OPECTL.Ope_Mnt_flg	= 0;	// ���ڰ�������ݽ�׸�ؾ��
				break;
			case FUNOPE_CHK:					// �e�`�m����`�F�b�N
				wopelg( OPLOG_FANOPE_CHK, 0, 0 );	// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 50;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_FUNOPECHK();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
//���g�p�ł��B(S)				
			case SUICA_CHK:						//��ʌn�h�b���[�_�[�`�F�b�N
				wopelg( OPLOG_SUICACHK, 0, 0 );	// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 30;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FncChk_Suica();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
			case PIP_CHK:							// Park i Pro�`�F�b�N
				wopelg( OPLOG_PIPCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 19;			// ���ڰ�������ݽ�׸޾��
				usFncEvent = FncChk_PiP();
				OPECTL.Ope_Mnt_flg	= 0;			// ���ڰ�������ݽ�׸�ؾ��
				break;
			case CRE_CHK:							// �N���W�b�g�`�F�b�N
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//				OPECTL.Ope_Mnt_flg	= 22;			// ���ڰ�������ݽ�׸޾��
//				usFncEvent = FncChk_Cre();
//				OPECTL.Ope_Mnt_flg	= 0;			// ���ڰ�������ݽ�׸�ؾ��
				BUZPIPI();
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
				break;
// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�`�F�b�N�ǉ�)
			case ECR_CHK:							// ���σ��[�_�`�F�b�N
				wopelg( OPLOG_CHK_EC, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 31;			// ���ڰ�������ݽ�׸޾��
				usFncEvent = FncChk_EcReader();
				OPECTL.Ope_Mnt_flg	= 0;			// ���ڰ�������ݽ�׸�ؾ��
				break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�`�F�b�N�ǉ�)				
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
			case NTNET_CHK:						// �P�U�D�m�s�|�m�d�s�`�F�b�N
				wopelg( OPLOG_NTNETCHK, 0, 0 );		// ���엚��o�^
				OPECTL.Ope_Mnt_flg	= 15;		// ���ڰ�������ݽ�׸޾��
				usFncEvent = FunChk_Ntnet();
				OPECTL.Ope_Mnt_flg	= 0;		// ���ڰ�������ݽ�׸�ؾ��
				break;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFncEvent;
				break;
//���g�p�ł��B(E)

			default:
				break;
		}
		if(( OpelogNo )||( SetChange )){
			if( OpelogNo ) wopelg( OpelogNo, 0, 0 );		// ���엚��o�^
			if( UserMnt_SysParaUpdateCheck( OpelogNo ) ){
				usFncEvent = parameter_upload_chk();
			}
		}

		if( usFncEvent == MOD_CHG || usFncEvent == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//�h�A�m�u�J�`���C����������Ԃ̂܂܃����e�i���X���[�h�I���܂��͉�ʂ��I�������ꍇ�͗L���ɂ���
				Ope_EnableDoorKnobChime();
			}
		}

		//Return Status of Funcchk Operation
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usFncEvent == MOD_CHG ){
		if(  usFncEvent == MOD_CHG || usFncEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFncEvent;
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| IN/OUT Sygnal Check                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Sig( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const	unsigned char	SIG_IStsCol[] = { 14 };						// ���͐M���\�����
// MH810100(S) S.Takahashi 2019/12/18
//const	unsigned char	SIG_OStsCol[] = { 5,8,11,14,17,20,23 };		// �o�͐M���\�����
const	unsigned char	SIG_OStsCol[] = { 2,6,10,14,18,22,26 };		// �o�͐M���\�����
// MH810100(E) S.Takahashi 2019/12/18

ushort	FunChk_Sig( void ){

	unsigned short	ret;			// �߂�l
	short			msg = -1;		// ��Mү����
	char			i;				// ٰ�߶����
	unsigned char	page;			// �\����ʁi�O�F���͐M����ʁA�P�F�o�͐M����ʁj
	unsigned char	pos;			// �o�͐M����ʃJ�[�\���ʒu�i0�`6�F�o�͐M��1�`7�j
	unsigned char	outsig_req;		// �n�m�^�n�e�e�v���p�����[�^
	unsigned char	insts;			// ���͐M�����(RXI_IN)
	unsigned char	outsts[7];		// �o�͐M�����(�o�͐M��1�`7)
	unsigned short  sd_exioport;	// �o���߰Ă̏����l��ێ�
	unsigned char	col;
	
	sd_exioport	=	SD_EXIOPORT;

	insts = ExIOSignalrd(15);						// RXI-1���͐M����ԃ��[�h
	for( i = 0; i < 7 ; i++ )
	{
		outsts[i] = 0x01 & (SD_EXIOPORT>>(14-i));	// �o�͐M��1�`7��ԃ��[�h
	}

	page = 0;										// ��ʂP�i���́j
	pos  = 0;										// �J�[�\���ʒu������
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[11] );				// [11]	"�����o�͐M���`�F�b�N���@�@�@�@"
	inSigStsDsp( 0, insts );													// �M����ԕ\���i���́j
	Fun_Dsp( FUNMSG[36] );														// [36]	"���^�o                   �I�� "
	Lagtim ( OPETCBNO, 21, 25 );												// ���͐M��������ϰ�N��

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );
		ret = 0;

		switch( msg){			// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;														// ���o�͐M���`�F�b�N�����I��
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

			case KEY_MODECHG:

				BUZPI();
				ret = MOD_CHG;														// ���o�͐M���`�F�b�N�����I��

				break;


			case KEY_TEN_F1:	// �e�P:"���^�o"�i�\����ʐؑցj
				BUZPI();
				pos  = 0;															// �J�[�\���ʒu������
				if( page == 0 ){
					// �M����ԕ\���i�o�́j
					page = 1;
					outSigStsDsp( &outsts[0] );
// MH810100(S) S.Takahashi 2020/02/16 #3839 �y�o�́z�ɑJ�ڎ��A�u1�v���Ԏ��ɂȂ�Ȃ�
//					grachr( 3,  5, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][5] );
					grachr( 3,  SIG_OStsCol[0], 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][SIG_OStsCol[0]] );
// MH810100(E) S.Takahashi 2020/02/06 #3839 �y�o�́z�ɑJ�ڎ��A�u1�v���Ԏ��ɂȂ�Ȃ�
					Fun_Dsp( FUNMSG[37] );											// [37]	"���^�o  ��    ��  ON/OFF �I�� "
				}
				else{
					// �M����ԕ\���i���́j
					page = 0;
					inSigStsDsp( 0, insts );
					Fun_Dsp( FUNMSG[36] );											// [36]	"���^�o                   �I�� "
					Lagtim ( OPETCBNO, 21, 25 );								// ���͐M��������ϰ�N��
				}
				break;

			case KEY_TEN_F2:	// �e�Q:"��"�i���وړ��F���j
				// �M����ԕ\���i�o��)�̏ꍇ
				if(page == 1)
				{
					BUZPI();
					col = (ushort)SIG_OStsCol[pos];
					grachr( 3,  col, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][col] );
					if( pos == 0 )												// �J�[�\���ʒu�X�V
					{
						pos = 6;
					}
					else{
						pos--;
					}
					col = (ushort)SIG_OStsCol[pos];
					grachr( 3,  col, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][col] );
				}
				break;

			case KEY_TEN_F3:	// �e�R:"��"�i���وړ��F�E�j
				// �M����ԕ\���i�o��)�̏ꍇ
				if(page == 1)
				{
					BUZPI();
					col = (ushort)SIG_OStsCol[pos];
					grachr( 3,  col, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][col] );
					if( pos == 6 ){													// �J�[�\���ʒu�X�V
						pos = 0;
					}
					else{
						pos++;
					}
					col = (ushort)SIG_OStsCol[pos];
					grachr( 3,  col, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, &SIGSTR1[3][col] );
				}
				break;

			case KEY_TEN_F4:	// �e�S:"ON/OFF"
				// [�o��]��ʕ\����
				if( page == 1 )
				{
					BUZPI();
					if( outsts[pos] == 0 ){
						// ���ݏ�ԁ��n�e�e
						outsig_req = outsts[pos] = 1;								// �n�m�v��
					}
					else{
						// ���ݏ�ԁ��n�m
						outsig_req = outsts[pos] = 0;								// �n�e�e�v��
					}
// MH810100(S) S.Takahashi 2019/12/18
//					grachr( 4, (ushort)SIG_OStsCol[pos], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[outsig_req] );	// ON/OFF�\���ؑ�
					grachr( 4, (ushort)SIG_OStsCol[pos], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[outsig_req] );	// ON/OFF�\���ؑ�
// MH810100(E) S.Takahashi 2019/12/18
					ExIOSignalwt( 14-pos , outsig_req);									// �g��IO�߰ĐM���o��
				}
				else{
					// [����]��ʕ\����
					;																	//�Ȃɂ����Ȃ�
				}
				break;

			case KEY_TEN_F5:	// �e�T:�I��
				BUZPI();
				ret = MOD_EXT;															// ���o�͐M���`�F�b�N�����I��

				break;
			case TIMEOUT21:																/* ���͐M���`�F�b�N */
				insts = ExIOSignalrd(15);												// RXI-1���͐M����ԃ��[�h
				inSigStsDsp( 1, insts );												// �M����ԕ\���i���́j
				Lagtim ( OPETCBNO, 21, 25 );											// ���͐M��������ϰ�N��
				break;
			default:

				break;
		}
		if( ret != 0 ){
			break;
		}
	}
	EXIO_CS4_EXIO = SD_EXIOPORT = sd_exioport;										// �o���߰Ēl�����ɖ߂�

	return( ret );
}

static	uchar	insts_now;		// ���͐M����ԃ��[�h�o�b�t�@

static void	inSigStsDsp( uchar req, uchar ionsts )				// ���͐M����ԕ\��
{
	uchar	i;					// ٰ�߶����

	if( req == 0 ){
		// �\���v����ʁ�����
		for( i = 1 ; i < 6 ; i++ ){							// ���C���P�`�T�N���A
			displclr( (ushort)i );
		}

		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR1[0] );
		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR1[2] );

		if( ionsts == 0 ){
// MH810100(S) S.Takahashi 2019/12/18
//			grachr( 4, (ushort)SIG_IStsCol[0], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
			grachr( 4, (ushort)SIG_IStsCol[0], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
// MH810100(E) S.Takahashi 2019/12/18
		}
		else{
// MH810100(S) S.Takahashi 2019/12/18
//			grachr( 4, (ushort)SIG_IStsCol[0], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
			grachr( 4, (ushort)SIG_IStsCol[0], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
// MH810100(E) S.Takahashi 2019/12/18
		}
	}
	else{
		// �\���v����ʁ��ω��������o�͐M����Ԃ̂�
		if( insts_now != ionsts ){				// ���ݏ�ԁ��\����ԁH
			insts_now = ionsts;

			if( ionsts == 0 ){
// MH810100(S) S.Takahashi 2019/12/18
//				grachr( 4, (ushort)SIG_IStsCol[0], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
				grachr( 4, (ushort)SIG_IStsCol[0], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
// MH810100(E) S.Takahashi 2019/12/18
			}
			else{
// MH810100(S) S.Takahashi 2019/12/18
//				grachr( 4, (ushort)SIG_IStsCol[0], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
				grachr( 4, (ushort)SIG_IStsCol[0], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
// MH810100(E) S.Takahashi 2019/12/18
			}
		}
	}
}
static void	outSigStsDsp( uchar *ionsts )						// �o�͐M����ԕ\��
{
	uchar	i;					// ٰ�߶����

	// �\���v����ʁ��o��
	for( i = 1 ; i < 6 ; i++ ){							// ���C���P�`�T�N���A
		displclr( (ushort)i );
	}

	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR1[1] );
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR1[3] );

	for( i = 0 ; i < 7 ; i++ ){							// �o�͐M����ԕ\��

		if( (*ionsts) == 0 ){
// MH810100(S) S.Takahashi 2019/12/18
//			grachr( 4, (ushort)SIG_OStsCol[i], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
			grachr( 4, (ushort)SIG_OStsCol[i], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );
// MH810100(E) S.Takahashi 2019/12/18
		}
		else{
// MH810100(S) S.Takahashi 2019/12/18
//			grachr( 4, (ushort)SIG_OStsCol[i], 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
			grachr( 4, (ushort)SIG_OStsCol[i], 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[1] );
// MH810100(E) S.Takahashi 2019/12/18
		}
		ionsts++;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Switch Check                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Swt( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#define	FUNCHK_SWMAX	10

unsigned short	FunChk_Swt( void ){

	unsigned char	Sw_rd( char idx );

	short			msg = -1;
	char			i;
	int				page;
	unsigned char	sts[FUNCHK_SWMAX];
	unsigned short	lin,col,cnt;
	unsigned char	page_chg;
	const unsigned char Stsknd[] = { 12,2,2,2,8,6,4,10 };				//DAT4_1[n]
	char			strbuf[10];
	unsigned long	dipswHex;

	page = 0;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[3] );			// [03]	"���X�C�b�`�`�F�b�N���@�@�@�@�@" */
	Fun_Dsp( FUNMSG[06] );						// [06]	"�@���@�@���@�@�@�@�@�@�@ �I�� "

	for( i = 0; i < 6; i++ ){

		// �\����ʂP�i�{������ف`�h�A�J�j
		sts[i] = Sw_rd( i );
		grachr( (unsigned short)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)FSWSTR[i] );
		grachr( (unsigned short)(i+1), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
	}

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );
		page_chg = OFF;

		switch( msg){							// FunctionKey Enter

// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				break;

			case KEY_TEN_F5:					// F5:�I��

				BUZPI();
				if( CP_MODECHG == 0 ){
					// ��Ì���ԁF�n�m
					return( MOD_EXT );
				}
				else{
					// ��Ì���ԁF�n�e�e
					return( MOD_CHG );
				}

				break;

			case KEY_TEN_F1:					// F1:"��"

				BUZPI();

				page_chg = ON;					// �\����ʐؑ�
				page--;
				if( page < 0 ){
					page = 1;
				}

				break;

			case KEY_TEN_F2:					// F2:"��"

				BUZPI();

				page_chg = ON;					// �\����ʐؑ�
				page++;
				if( page > 1 ){
					page = 0;
				}

				break;

			default:
				break;
		}
		if( page == 0 ){

			// �\����ʂP�i�{������ف`�h�A�J�j

			if( page_chg == ON ){

				// �\����ʐؑ�
				for( i = 0; i < 6; i++ ){		// �{������ف`�h�A�J�܂ł̏�ԕ\�����s��
					sts[i] = Sw_rd( i );
					grachr( (unsigned short)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)FSWSTR[i] );
					grachr( (unsigned short)(i+1), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
				}
			}
			else{
				// �r�v��ԕω�
				for( i = 0; i < 6 ; i++ ){		// �{������ف`�h�A�J�܂ł̏�ԕ\�����s���i�ω�����SW�̂ݏ�ԕ\�����X�V�j
					if( sts[i] != Sw_rd( i ) ){
						sts[i] = Sw_rd( i );
						grachr( (unsigned short)(i+1), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
					}
				}
			}
		}
		else{
			// �\����ʂQ�i�U���ݻ��`DIPSW4�j

			if( page_chg == ON ){

				// �\����ʐؑ�
				for( i = 6; i < FUNCHK_SWMAX; i++ ){		// �U���ݻ��`DIPSW4�܂ł̏�ԕ\�����s��

					if( i <= 9 ){
						// �U���ݻ�,����ذ�ް�E��,�l�̌��m,���[�^���[�X�C�b�`,�f�B�b�v�X�C�b�`
						lin = (i-5);
						col = 22;
						cnt	= 8;
						displclr(lin);
						grachr( lin, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)FSWSTR[i] );
					}
					sts[i] = Sw_rd( i );
					if( i <= 7 ) {
						grachr( lin, col, cnt, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
					}
					else if(i == 8){	// ���[�^���X�C�b�`
						memset(strbuf, 0, sizeof(strbuf));
						intoas( (unsigned char *)&strbuf, sts[i], 1 );
						grachr( 3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)strbuf );
					}
					else if(i == 9){	// �f�B�b�v�X�C�b�`
						memset(strbuf, 0, sizeof(strbuf));
						dipswHex = (unsigned long)sts[i];
						hextoas( (unsigned char *)&strbuf, dipswHex, 2 , 0);
						grachr( 4, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)strbuf );
					}
				}
				displclr(5);
				displclr(6);
			}
			else{
				// �r�v��ԕω�
				for( i = 6; i < FUNCHK_SWMAX ; i++ ){		// �U���ݻ��`DIPSW4,�l�̌��m�܂ł̏�ԕ\�����s���i�ω�����SW�̂ݏ�ԕ\�����X�V�j
					if( sts[i] != Sw_rd( i ) ){
						sts[i] = Sw_rd( i );

						if( i <= 7 ){
							// �U���ݻ��`����3
							lin = (i-5);
							col = 22;
							cnt	= 8;
							grachr( lin, col, cnt, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[ sts[i] + Stsknd[i] ] );
						}
						else if(i == 8){	// ���[�^���X�C�b�`
							memset(strbuf, 0, sizeof(strbuf));
							intoas( (unsigned char *)&strbuf, sts[i], 1 );
							grachr( 3, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)strbuf );
						}
						else if(i == 9){	// �f�B�b�v�X�C�b�`
							memset(strbuf, 0, sizeof(strbuf));
							dipswHex = (unsigned long)sts[i];
							hextoas( (unsigned char *)&strbuf, dipswHex, 2 , 0);
							grachr( 4, 22, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)strbuf );
						}
					}
				}
			}
		}
	}
}

// �X�C�b�`���[�h����
unsigned char	Sw_rd(  char idx  )
{
	unsigned char ret;

	switch( idx ){		// �X�C�b�`��ʁH

		case	0:		// �ݒ�r�v
			ret = ( CP_MODECHG ) ? 0 : 1;
			break;

		case	1:		// �R�C�����b�N�Z�b�g�r�v
			ret = ( FNT_CN_DRSW ) ? 0 : 1;
			break;

		case	2:		// �R�C�����ɂr�v
			ret = ( FNT_CN_BOX_SW ) ? 0 : 1;
			break;

		case	3:		// �������ɂr�v
			ret = ( FNT_NT_BOX_SW ) ? 0 : 1;
			break;

		case	4:		// �h�A�J
			ret = ( OPE_SIG_DOOR_Is_OPEN ) ? 0 : 1;
			break;

		case	5:		// �U���Z���T�[
			ret = FNT_VIB_SNS;
			break;

		case	6:		// �������[�_�[�E���Z���T�[
			ret = ( FNT_NT_FALL_SW ) ? 0: 1;
			break;
		case	7:		// �l�̌��m
			ret = ( FNT_MAN_DET ) ? 1: 0;
			break;
		case	8:		// ���[�^���[�X�C�b�`
			ret = read_rotsw();
			break;
		case	9:		// �f�B�b�v�X�C�b�`
			ret = 0;
			ret = (key_dat[0].BYTE & 0x80) ? 0x10 : 0;	// RXF�f�B�b�v�X�C�b�`1
			ret += (key_dat[0].BYTE & 0x40) ? 0x20 : 0;	// RXF�f�B�b�v�X�C�b�`2
			ret += (key_dat[0].BYTE & 0x20) ? 0x40 : 0;	// RXF�f�B�b�v�X�C�b�`3
			ret += (key_dat[0].BYTE & 0x10) ? 0x80 : 0;	// RXF�f�B�b�v�X�C�b�`4
			ret += ExIOSignalrd(INSIG_DPSW0);			// RXM�f�B�b�v�X�C�b�`1
			ret += ExIOSignalrd(INSIG_DPSW1) << 1;		// RXM�f�B�b�v�X�C�b�`2
			ret += ExIOSignalrd(INSIG_DPSW2) << 2;		// RXM�f�B�b�v�X�C�b�`3
			ret += ExIOSignalrd(INSIG_DPSW3) << 3;		// RXM�f�B�b�v�X�C�b�`4
			break;
		default:		// ���̑�
			ret = 0;
			break;
	}
	return(ret);
}


/*[]----------------------------------------------------------------------[]*/
/*| Key Board Check                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Kbd( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

unsigned short	FunChk_Kbd( void ){

	short			msg = -1;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[4] );		/* // [04]	"���L�[���̓`�F�b�N���@�@�@�@�@" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[5] );		/* // [05]	"�@���̓L�[�F�@�@�@�@�@�@�@�@�@" */
	grachr( 2, 12, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_0[21] );		/* // [00]	"�@�@�@�@�@�@�h  �h�@�@�@�@�@�@" */
	Fun_Dsp( FUNMSG[8] );					/* // [08]	"�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */

	for( ; ; ){
		msg = GetMessage();					/*								*/
		switch( msg){						/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;
			case KEY_TEN0:
			case KEY_TEN1:
			case KEY_TEN2:
			case KEY_TEN3:
			case KEY_TEN4:
			case KEY_TEN5:
			case KEY_TEN6:
			case KEY_TEN7:
			case KEY_TEN8:
			case KEY_TEN9:
			case KEY_TEN_CL:
			case KEY_TEN_F1:					/* F1: */
			case KEY_TEN_F2:					/* F2: */
			case KEY_TEN_F3:					/* F3: */
			case KEY_TEN_F4:					/* F4: */
				BUZPI();
				grachr( 2, 12, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_0[msg - 0x0130] );
				break;
			case KEY_TEN_F5:					/* F5:Exit */
				BUZPI();
				grachr( 2, 12, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT5_0[msg - 0x0130] );
				return( MOD_EXT );
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Sht                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : �������������                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	SHTCHK_CNT	1	// �����Ώۼ�������ڐ�
#define	SHTSTS_CLM	27	// �������ԁi�J/�j�\�����

unsigned short	FunChk_Sht( void ){

	unsigned short	ret = 0;				// �߂�l
	short			msg = -1;				// ��Mү����
	unsigned char	sht_ope = OFF;			// ������J����K���׸�
	unsigned char	sw = 0;					// ����ýĒ��׸�
	unsigned char	pos = 0;				// ���ٕ\���ʒu
	unsigned char	sts[SHTCHK_CNT];		// �������ԕۑ�ܰ��iýĊJ�n�O�̏�ԁj	���P���u�v�A�Q���u�J�v
	unsigned char	sts_now[SHTCHK_CNT];	// ��������ܰ��iýĒ��̏�ԁj			���O���u�v�A�P���u�J�v
	char			counter = 0;
	#define AUTO_SHTCHK_MAX	5				// �����`�F�b�N�̍ő���s��

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHTSTR[0] );							// "���V���b�^�[�`�F�b�N���@�@�@�@"
	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );							// "�@�e�X�g�F����ýĂ��J�n���܂� "
	Fun_Dsp( FUNMSG[46] );																		// "�@���@�@���@�e�X�g �J/�� �I�� "

																								// ���Cذ�ް��������ݏ�Ԃ��ތ�A�u�v�Ƃ���
	sts[0]		= READ_SHT_flg;																	// ���ݏ�Ԃ���
	sts_now[0]	= 0;																			// �������Ծ�āu�v
	read_sht_cls();																				// ������u�v
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SHTSTR[1] );							// "�@���Cذ�ް����� �@�@�@���@�@ "
	grachr( 2, SHTSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[0] );					// �������ԕ\���u�v�i���]�\���j

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );							// ү���ގ�M

		switch( msg){											// ��Mү���ށH

// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:									// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:									// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;


			case KEY_TEN_F3:									// �e�R

				BUZPI();
				if( sw != 0 ){

					// ����ýĒ��̏ꍇ�F�e�R����~

					sw = 0;
					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );			// "�@�e�X�g�F����ýĂ��J�n���܂� "
					Fun_Dsp( FUNMSG[46] );														// "�@���@�@���@�e�X�g �J/�� �I�� "
					Lagcan( OPETCBNO, 6 );														// ����ý���ϰؾ��
					Ope_EnableDoorKnobChime();
				}
				else{
					// ����ýĒ��łȂ��ꍇ�F�e�R��ý�

					sw = 1;

																								// ���Cذ�ް��������u�v�Ƃ���
					sts_now[0]	= 0;															// �������Ծ�āu�v
					read_sht_cls();																// ������u�v
					grachr( 2, SHTSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[0] );	// �������ԕ\���u�v

																								// ��ݓ�������������u�v�Ƃ���

					pos = SHTCHK_CNT-1;															// ���وʒu�������i����ýĂŶ��ق��擪�ɂȂ�悤�ɍŌ�ʒu��Ă���j

					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[12] );			// "�@��~�F����ýĂ��~���܂� �@"
					Fun_Dsp( FUNMSG[48] );														// "�@�@�@�@�@�@ ��~ �@�@�@ �I�� "
					Lagtim( OPETCBNO, 6, 1*50 );												// ����ý���ϰ6(1s)�N��
					counter = 0;								// ����J�E���g�N���A
					Ope_DisableDoorKnobChime();
				}

				break;

			case KEY_TEN_F4:									// �e�S�iON/OFF�j

				if( sw == 0 && sht_ope == OFF ){				// ����ýĒ��A�܂��́@������J����K�����̏ꍇ�A����

					BUZPI();

					if( pos == 0 ){
						// ���وʒu�����Cذ�ް�����
						if( sts_now[pos] == 0 ){				// ���ݏ�ԁH
							sts_now[pos] = 1;					// ��ԍX�V
							read_sht_opn();						// ������u�J�v
						}
						else{
							sts_now[pos] = 0;					// ��ԍX�V
							read_sht_cls();						// ������u�v
						}
					}
					grachr( (ushort)(pos + 2), SHTSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[sts_now[pos] ? 1 : 0] );	// �������ԕ\���i���]�\���j

					Lagtim( OPETCBNO, 7, 1*50 );				// ������J����K����ϰ7(1s)�N��
					sht_ope = ON;								// ������J����K���׸޾�āi�K���J�n�j
				}
				break;

			case TIMEOUT6:										// ����ý���ϰ��ѱ��

				if( sw != 0 ){									// ����ýĒ��H

					grachr( (ushort)(pos + 2), SHTSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[sts_now[pos] ? 1 : 0] );	// ���̶݂��وʒu�F���]�\��OFF

					if( pos == 0 ){
						// ���وʒu�����Cذ�ް�����
						if( sts_now[pos] == 0 ){				// ���ݏ�ԁH
							sts_now[pos] = 1;					// ��ԍX�V
							read_sht_opn();						// ������u�J�v
						}
						else{
							sts_now[pos] = 0;					// ��ԍX�V
							read_sht_cls();						// ������u�v
						}
					}
					grachr( (ushort)(pos + 2), SHTSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT1_0[sts_now[pos] ? 1 : 0] );	// �������ԕ\���i���]�\���j

					if( pos && !sts_now[pos] ){
						if( ++counter >= AUTO_SHTCHK_MAX ){
							sw = 0;
							grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );	// "�@�e�X�g�F����ýĂ��J�n���܂� "
							Fun_Dsp( FUNMSG[46] );				// "�@���@�@���@�e�X�g �J/�� �I�� "
							Lagcan( OPETCBNO, 6 );				// ����ý���ϰؾ��
							break;
						}
					}
					Lagtim( OPETCBNO, 6, 1*50 );				// ����ý���ϰ6(1s)�N��
				}

				break;

			case TIMEOUT7:										// ������J����K����ϰ��ѱ��

				sht_ope = OFF;									// ������J����K���׸�ؾ�āi�K�������j
				break;

			default:
				break;
		}

		if( ret != 0 ){											// ����������I���H

			if( sw != 0 ){										// ����ýĒ��H
				Lagcan( OPETCBNO, 6 );							// ����ý���ϰ����
			}

			if( sht_ope == ON ){								// ������J����K�����H
				Lagcan( OPETCBNO, 7 );							// ������J����K����ϰ����
			}

			// ���Cذ�ް�������ýĊJ�n�O�̏�Ԃɖ߂�
			if( sts[0] == 1 ){
				read_sht_cls();									// ������u�v
			}
			else{
				read_sht_opn();									// ������u�J�v
			}
			// ��ݓ������������ýĊJ�n�O�̏�Ԃɖ߂�
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Led                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : LED��������                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/01                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	LEDSTS_CLM	27	// LED��ԁiON/OFF�j�\�����

unsigned short	FunChk_Led( void ){

	unsigned short	ret = 0;				// �߂�l
	short			msg = -1;				// ��Mү����
	unsigned char	i;						// ٰ�߶����
	unsigned char	sw = 0;					// ����ýĒ��׸�
	unsigned char	pos = 0;				// ���ٕ\���ʒu
	unsigned char	sts[LEDCT_MAX];			// LED��ԕۑ�ܰ��iýĊJ�n�O�̏�ԁj
	unsigned char	sts_now[LEDCT_MAX];		// LED���ܰ��iýĒ��̏�ԁj


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[6] );			// "���k�d�c�`�F�b�N���@�@�@�@�@�@"

// MH810100(S) Y.Yamauchi 2019/10/15 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );			// "�@�e�X�g�F����ýĂ��J�n���܂� "
//	Fun_Dsp( FUNMSG[47] );														// "�@���@�@���@�e�X�gON/OFF �I�� "
	if(LEDCT_MAX > 1){
		grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );			// "�@�e�X�g�F����ýĂ��J�n���܂� "
		Fun_Dsp( FUNMSG[120] );														// 	"�@�@�@�@�@�@�e�X�gON/OFF �I�� ",
		}
		else{
			Fun_Dsp( FUNMSG[50] );														// 	"�@�@�@�@�@�@�@�@�@ON/OFF �I�� ",
		}												
// MH810100(E) Y.Yamauchi 2019/10/15 �Ԕԃ`�P�b�g���X(�����e�i���X)

	for( i = 0; i < LEDCT_MAX ; i++ ){			// �SLED�ɑ΂����݂̏�Ԃ��ތ�AOFF�Ƃ���
		sts[i] = IsLedReq( i );										// ���ݏ�Ծ���
		sts_now[i] = 0;												// LED��Ծ�āiOFF�j
		LedReq( i, 0 );												// LED����iOFF�j
		grachr( (ushort)(i + 1),  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LEDSTR[i] );			// LED���̕\��
// MH810100(S) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		grachr( (ushort)(i + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );	// LED��ԕ\���iOFF�j
		grachr( (ushort)(i + 1), LEDSTS_CLM, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );	// LED��ԕ\���iOFF�j
// MH810100(E) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
	}

// MH810100(S) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	grachr( 1, LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );						// �������وʒu�𔽓]�\��
	grachr( 1, LEDSTS_CLM, 3, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );						// �������وʒu�𔽓]�\��
// MH810100(E) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );				// ү���ގ�M

		switch( msg){								// ��Mү���ށH

// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

			case KEY_MODECHG:						// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:						// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F1:						// �e�P�i���j
// MH810100(S) Y.Yamauchi 2019/10/04 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if( sw == 0 ){						// ����ýĒ��̏ꍇ�A����
//
//					BUZPI();
//
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// ���̶݂��وʒu�F���]�\��OFF
//
//					if( pos == 0 ){					// ���وʒu�X�V�i��ړ��j
//						pos = LEDCT_MAX-1;
//					}
//					else{
//						pos--;
//					}
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// �ړ����وʒu�F���]�\��
//				}
//
// MH810100(S) Y.Yamauchi 2019/10/04 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;

			case KEY_TEN_F2:						// �e�Q�i���j
// MH810100(S) Y.Yamauchi 2019/10/04 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if( sw == 0 ){						// ����ýĒ��̏ꍇ�A����
//
//					BUZPI();
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// ���̶݂��وʒu�F���]�\��OFF
//
//					if( pos >= (LEDCT_MAX-1) ){		// ���وʒu�X�V�i���ړ��j
//						pos = 0;
//					}
//					else{
//						pos++;
//					}
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// �ړ����وʒu�F���]�\��
//				}
// MH810100(E) Y.Yamauchi 2019/10/04 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;

			case KEY_TEN_F3:						// �e�R
// MH810100(S) Y.Yamauchi 2019/10/15 �Ԕԃ`�P�b�g���X(�����e�i���X)
				if(LEDCT_MAX > 1){
// MH810100(E) Y.Yamauchi 2019/10/15 �Ԕԃ`�P�b�g���X(�����e�i���X)
					BUZPI();
					if( sw != 0 ){

						// ����ýĒ��̏ꍇ�F�e�R����~

						sw = 0;
						grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[7] );			// "�@�e�X�g�F����ýĂ��J�n���܂� "
						Fun_Dsp( FUNMSG[47] );														// "�@���@�@���@�e�X�gON/OFF �I�� "
						Lagcan( OPETCBNO, 6 );														// ����ý���ϰؾ��
						Ope_EnableDoorKnobChime();
					}
					else{
						// ����ýĒ��łȂ��ꍇ�F�e�R��ý�

						sw = 1;

						for( i = 0; i < LEDCT_MAX ; i++ ){																// �SLED��OFF�Ƃ���
							sts_now[i] = 0;																				// LED��Ծ�āiOFF�j
							LedReq( i, 0 );																				// LED����iOFF�j
// MH810100(S) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							grachr( (ushort)(i + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );	// LED��ԕ\��
							grachr( (ushort)(i + 1), LEDSTS_CLM, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[0] );	// LED��ԕ\��
// MH810100(E) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
						}
						pos = LEDCT_MAX-1;							// ���وʒu�������i����ýĂŶ��ق��擪�ɂȂ�悤�ɍŌ�ʒu��Ă���j

						grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[12] );			// "�@��~�F����ýĂ��~���܂� �@"
						Fun_Dsp( FUNMSG[48] );														// "�@�@�@�@�@�@ ��~ �@�@�@ �I�� "
						Lagtim( OPETCBNO, 6, 1*50 );												// ����ý���ϰ6(1s)�N��
						Ope_DisableDoorKnobChime();
					}
// MH810100(S) Y.Yamauchi 2019/10/15 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
// MH810100(E) Y.Yamauchi 2019/10/15 �Ԕԃ`�P�b�g���X(�����e�i���X)
				break;

			case KEY_TEN_F4:						// �e�S�iON/OFF�j

				if( sw == 0 ){						// ����ýĒ��̏ꍇ�A����

					BUZPI();
					if( sts_now[pos] == 0 ){		// ��ԍX�V
						sts_now[pos] = 1;
					}
					else{
						sts_now[pos] = 0;
					}
					LedReq( pos, sts_now[pos] );	// LED����
// MH810100(S) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// LED��ԕ\���i���]�\���j
					grachr( (ushort)(pos + 1), LEDSTS_CLM, 3, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// LED��ԕ\���i���]�\���j
// MH810100(E) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				break;

			case TIMEOUT6:							// ����ý���ϰ��ѱ��

				if( sw != 0 ){						// ����ýĒ��H

// MH810100(S) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// ���̶݂��وʒu�F���]�\��OFF
					grachr( (ushort)(pos + 1), LEDSTS_CLM, 3, 0, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// ���̶݂��وʒu�F���]�\��OFF
// MH810100(E) S.Takahashi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH810100(S) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					if( pos >= (LEDCT_MAX-1) ){		// ���وʒu�X�V�i���ړ��j
//						pos = 0;
//					}
//					else{
//						pos++;
//					}
#if (LEDCT_MAX > 1)
					if( pos >= (LEDCT_MAX-1) ){		// ���وʒu�X�V�i���ړ��j
						pos = 0;
					}
					else{
						pos++;
					}
#else
					pos = 0;
#endif
// MH810100(E) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)
					if( sts_now[pos] == 0 ){		// ��ԍX�V
						sts_now[pos] = 1;
					}
					else{
						sts_now[pos] = 0;
					}
					LedReq( pos, sts_now[pos] );	// LED����
// MH810100(S) S.Takahashi 2019/12/18
//					grachr( (ushort)(pos + 1), LEDSTS_CLM, 2, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// LED��ԕ\���i���]�\���j
					grachr( (ushort)(pos + 1), LEDSTS_CLM, 3, 1, COLOR_BLACK, LCD_BLINK_OFF, SIGSTR2[sts_now[pos] ? 1 : 0] );	// LED��ԕ\���i���]�\���j
// MH810100(E) S.Takahashi 2019/12/18

					Lagtim( OPETCBNO, 6, 1*50 );	// ����ý���ϰ6(1s)�N��
				}

				break;

			default:
				break;
		}

		if( ret != 0 ){								// LED�����I���H

			if( sw != 0 ){							// ����ýĒ��H
				Lagcan( OPETCBNO, 6 );				// ����ý���ϰ����
			}

			for( i = 0; i < LEDCT_MAX ; i++ ){		// �SLED��ýĊJ�n�O�̏�Ԃɖ߂�
				LedReq( i, sts[i] );				// LED����
			}

			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Journal & Receipt printer check                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Prt( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Prt( void )
{
	unsigned short	usFprnEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{

		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[0] );		/* // [00]	"���v�����^�`�F�b�N���@�@�@�@�@" */

		usFprnEvent = Menu_Slt( FPRNMENU1, FPRN_CHK_TBL1, (char)FPRN_CHK_MAX1, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFprnEvent ){

			case FPRNJ_CHK:	// �ެ��������
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			case FPRNEJ_CHK:// �d�q�W���[�i��
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
				if( OFF == PrnJnlCheck() ){		// �ެ���������ڑ��H
					BUZPIPI();					// �ڑ��Ȃ��̏ꍇ�A�ެ�������������s�Ƃ���
					break;
				}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
				if (usFprnEvent == FPRNJ_CHK && isEJA_USE()) {
					// �d�q�W���[�i���ڑ����̓W���[�i���v�����^�`�F�b�N�s��
					BUZPIPI();
					break;
				}
				else if (usFprnEvent == FPRNEJ_CHK && !isEJA_USE()) {
					// �d�q�W���[�i���ڑ��Ȃ��͓d�q�W���[�i���`�F�b�N�s��
					BUZPIPI();
					break;
				}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
				// FunChk_JRPrt�ŏ�������ibreak�s�v�j

			case FPRNR_CHK:	// ڼ�������
				usFprnEvent = FunChk_JRPrt( usFprnEvent );
				break;

			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFprnEvent;
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:						/* F4:Exit */
				BUZPI();
				return( MOD_EXT );
				break;

			default:
				break;
		}
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X�i�����e�i���X)
//		if( usFprnEvent == MOD_CHG){
		if( usFprnEvent == MOD_CHG || usFprnEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2020/1/9 �Ԕԃ`�P�b�g���X�i�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFprnEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

unsigned short	FunChk_JRPrt( unsigned short event ){

	unsigned short	usFprnEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{

		dispclr();

		if( FPRNR_CHK == event ){
			// ڼ�����������
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[1] );		/* // "�����V�[�g�v�����^�`�F�b�N���@" */
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		}else if (FPRNEJ_CHK == event) {
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18] );		// "���d�q�W���[�i���`�F�b�N���@�@"
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		}else{
			// �ެ������������
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[2] );		/* // "���W���[�i���v�����^�`�F�b�N��" */
		}

		if( FPRNR_CHK == event ){
			// ڼ�����������
			usFprnEvent = Menu_Slt( FPRNMENU2, FPRN_CHK_TBL2, (char)FPRN_CHK_MAX2, (char)1 );
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		}else if (FPRNEJ_CHK == event) {
			usFprnEvent = Menu_Slt( FPRNMENU5, FPRN_CHK_TBL5, (char)FPRN_CHK_MAX5, (char)1 );
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		}else{
			// �ެ������������
			usFprnEvent = Menu_Slt( FPRNMENU2, FPRN_CHK_TBL3, (char)(FPRN_CHK_MAX2-1), (char)1 );
		}

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFprnEvent ){

			case FPRN7_CHK:	// ڼ��ýĲݻ�
				usFprnEvent = FPrnChk_tst_rct( );
				break;
			case FPRN3_CHK:	// �ެ��و�ý�
				usFprnEvent = FPrnChk_tst( usFprnEvent );
				break;
			case FPRN2_CHK:	// ڼ�ď�Ԋm�F
			case FPRN4_CHK:	// �ެ��ُ�Ԋm�F
				usFprnEvent = FPrnChk_sts( usFprnEvent );
				break;
			case FPRN5_CHK:	// ���S�󎚃f�[�^�o�^�iڼ�āj
				usFprnEvent = FPrnChk_logo( usFprnEvent );
				break;
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
			case FPRN_RW_CHK:	// ���[�h�E���C�g�e�X�g
				usFprnEvent = FPrnChk_RW();
				break;
			case FPRN_SD_CHK:	// �r�c�J�[�h���
				usFprnEvent = FPrnChk_SD();
				break;
			case FPRN_VER_CHK:	// �o�[�W�����m�F
				usFprnEvent = FPrnChk_Ver();
				break;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFprnEvent;
				break;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usFprnEvent == MOD_CHG ){
		if( usFprnEvent == MOD_CHG || usFprnEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFprnEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_tst                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : �����ýĈ󎚏���                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short event                                    |*/
/*|                  FPRN1_CHK : ڼ�Ĉ�ý�                               |*/
/*|                  FPRN3_CHK : �ެ��و�ý�                             |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/06/30                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		TSTPRI_END	(PREQ_PRINT_TEST | INNJI_ENDMASK)

unsigned short	FPrnChk_tst( unsigned short event )
{
	short			msg = -1;			// ��Mү����
	T_FrmPrintTest	FrmPrintTest;		// �󎚗v��ү���ލ쐬�ر
	unsigned short	ret = 0;			// �߂�l
	unsigned char	priend;				// �󎚏I���׸�				��OFF:�󎚒��AON:�󎚏I��
	unsigned char	dsptimer;			// �󎚒���ʕ\����ϰ�׸�	��OFF:��ѱ�čς�	ON:�N����

	dispclr();

	if( event == FPRN1_CHK ){
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[15] );			// "�����V�[�g�e�X�g������@�@�@�@" 
	}
	else{
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[2] );			// "���W���[�i���v�����^�`�F�b�N��" 
	}
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[3] );				// "�y�e�X�g�p�^������z�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[4] );				// "�@������ł��@�@�@�@�@�@�@�@�@" 
// MH810100(S) 2020/08/03 #4566�y�y���؉ێw�E�����z�V�X�e�������e�i���X�@�h�A�m�u�ɂ��󎚒��~���s���Ȃ�
// // MH810100(S) Y.Yamauchi 2020/03/19 �Ԕԃ`�P�b�g���X(#3963 �e�X�g����J�n��������r���Œ�~����ƃe�X�g�����ʂɗ��܂��Ă��܂��B)
// 	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18] );				// "������~�������ꍇ�́@�@�@�@ "
// 	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[19] );				// "�����e�i���X�I�����Ă�������"
// // MH810100(E) Y.Yamauchi 2020/03/19 �Ԕԃ`�P�b�g���X(#3963 �e�X�g����J�n��������r���Œ�~����ƃe�X�g�����ʂɗ��܂��Ă��܂��B)
// MH810100(E) 2020/08/03 #4566�y�y���؉ێw�E�����z�V�X�e�������e�i���X�@�h�A�m�u�ɂ��󎚒��~���s���Ȃ�
	Fun_Dsp(FUNMSG[0]);															/* [0]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
	
	if( event == FPRN1_CHK ){
		FrmPrintTest.prn_kind = R_PRI;				// �������ʁFڼ��
	}
	else{
		FrmPrintTest.prn_kind = J_PRI;				// �������ʁF�ެ���
	}

	queset( PRNTCBNO, PREQ_PRINT_TEST, sizeof( T_FrmPrintTest ), &FrmPrintTest ); 			// �e�X�g�󎚗v��
	Ope_DisableDoorKnobChime();

	Lagtim( OPETCBNO, 6, 3*50 );					// �󎚒���ʕ\����ϰ6(3s)�N���iýĈ󎚒��̉�ʕ\�����R�b�ԕێ�����j
	dsptimer	= ON;								// �󎚒���ʕ\����ϰ�׸޾��
	priend		= OFF;								// �󎚏I���׸�ؾ��

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );				// ү���ގ�M

		switch( msg ){			// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	// ����Ӱ�ސؑ�
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
				// �h�A�m�u���ǂ����̃`�F�b�N�����{
				if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				BUZPI();
				ret = MOD_CHG;
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				}
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				break;
			case TSTPRI_END:	// �e�X�g�󎚏I��
				if( dsptimer == OFF ){					// �󎚒���ʕ\����ϰ��ѱ�čς݁H
					ret = MOD_EXT;						// YES���O��ʂɖ߂�
				}
				else{
					priend = ON;						// NO�@���󎚏I���׸޾��
				}
				break;
			case TIMEOUT6:		// �󎚒���ʏI����ϰ��ѱ��
				dsptimer = OFF;							// �󎚒���ʕ\����ϰ��ѱ��
				if( priend == ON ){						// �󎚏I���ς݁H
					ret = MOD_EXT;						// YES���O��ʂɖ߂�
				}
				break;
			default:			// ���̑�
				break;
		}

		if( ret != 0 ){									// �󎚒���ʏI���H
			if( dsptimer == ON ){						// �󎚒���ʕ\����ϰ�N�����H
				Lagcan( OPETCBNO, 6 );					// �󎚒���ʕ\����ϰ����
			}
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_sts                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : �������Ԋm�F����                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short event                                    |*/
/*|                  FPRN2_CHK : ڼ�ď�Ԋm�F                              |*/
/*|                  FPRN4_CHK : �ެ��ُ�Ԋm�F                            |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/06/30                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const	unsigned char	PriErrSts[4] =	// �������������׸�
	{
		0x01,	// b0:�߰�߰Ʊ����
		0x02,	// b1:����ݵ����
		0x04,	// b2:�߰�߰����
		0x08	// b3:ͯ�މ��x�ُ�
	};

unsigned short	FPrnChk_sts( unsigned short event )	// ��Ԋm�F
{
	short			msg = -1;
	unsigned short	ret = 0;
	unsigned char	sts;
	unsigned char	com = OFF;
	unsigned short	line;
	unsigned char	i;

	dispclr();

	if( event == FPRN2_CHK ){
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[1] );				// "�����V�[�g�v�����^�`�F�b�N���@"
	}
	else{
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		if (isEJA_USE()) {
			grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18] );			// "���d�q�W���[�i���`�F�b�N���@�@"
		}
		else {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[2] );				// "���W���[�i���v�����^�`�F�b�N��"
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
	}
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[6] );					// "�y��Ԋm�F�z�@�@�@�@�@�@�@�@�@"
	Fun_Dsp( FUNMSG[8] );																// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	//��Ԋm�F
	if( event == FPRN2_CHK ){
		//	���V�[�g�v�����^
		if(ERR_CHK[ERRMDL_PRINTER][01] != 0){
			com = ON;									// �ʐM�s��
		}
		sts = (uchar)(rct_prn_buff.PrnStateMnt & 0x0f);	// ���ذ��
	}
	else{
		//	�W���[�i���v�����^
		if(ERR_CHK[ERRMDL_PRINTER][20] != 0){
			com = ON;									// �ʐM�s��
		}
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
		if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL)) {
			com = ON;									// �ʐM�s��
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
		sts = (uchar)(jnl_prn_buff.PrnStateMnt & 0x0f);	// ���ذ��
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�g�p�������߃X�e�[�^�X���\������Ȃ��j
		if (isEJA_USE()) {
			sts = (uchar)(jnl_prn_buff.PrnStateMnt & 0x1f);	// ���ذ��
		}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�g�p�������߃X�e�[�^�X���\������Ȃ��j
	}
	if( com == OFF ){

		// �ʐM��ԁ�����

		if( sts == 0 ){
			// ��ԁ��ُ�Ȃ�
			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[7] );						// "�@����ł��@�@�@�@�@�@�@�@�@�@"
		}
		else{
			// ��ԁ��ُ킠��
			line	= 2;
			for( i = 0 ; i < 4 ; i++ ){						// ��Ԃ��������������Ă���װ����S�ĕ\������
				if( sts & PriErrSts[i] ){
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���V�[�g�̃j�A�G���h��SD�j�A�G���h���\�������j
//					if (isEJA_USE()) {
					if (isEJA_USE() && event == FPRN4_CHK) {
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���V�[�g�̃j�A�G���h��SD�j�A�G���h���\�������j
						grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[i+19] );	// �װ���\��
					}
					else {
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
					grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[i+8] );	// �װ���\��
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
					}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
					line++;
				}
			}
// MH810104 GG119201(S) �d�q�W���[�i���Ή��iSD�J�[�h�g�p�������߃X�e�[�^�X���\������Ȃ��j
			if (isEJA_USE()) {
				if (sts & 0x10) {
					grachr( line, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[31] );	// �װ���\��
					line++;
				}
			}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��iSD�J�[�h�g�p�������߃X�e�[�^�X���\������Ȃ��j
		}
	}
	else{
		// �ʐM��ԁ��ُ�
		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[12] );					// "�@�ʐM�s�ǁ@�@�@�@�@�@�@�@�@�@"
	}

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );					// ү���ގ�M

		switch( msg ){									// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:							// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			default:
				break;
		}
		if( ret != 0 ){
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_logo                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���S�󎚃f�[�^�o�^����                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned short event                                    |*/
/*|                  FPRN5_CHK : ���S�󎚃f�[�^�o�^�iڼ�āj                |*/
/*|                  FPRN6_CHK : ���S�󎚃f�[�^�o�^�i�ެ��فj              |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/08/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		LOGOREG_END	(PREQ_LOGO_REGIST | INNJI_ENDMASK)

unsigned short	FPrnChk_logo( unsigned short event )
{
	short			msg = -1;			// ��Mү����
	T_FrmLogoRegist	FrmLogoRegist;		// ���S�󎚃f�[�^�o�^�v��ү���ލ쐬�ر
	unsigned short	ret = 0;			// �߂�l
	unsigned char	regend;				// �o�^�I���׸�				��OFF:�o�^���AON:�o�^�I��
	unsigned char	dsptimer;			// �o�^����ʕ\����ϰ�׸�	��OFF:��ѱ�čς�	ON:�N����

	dispclr();

	if( event == FPRN5_CHK ){
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[1] );			// "�����V�[�g�v�����^�`�F�b�N���@" 
	}
	else{
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[2] );			// "���W���[�i���v�����^�`�F�b�N��" 
	}
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[13] );				// "�y���S�󎚃f�[�^�o�^�z�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, PRTSTR[14] );		// "�@�o�^��(���΂炭���҂�������)" 
	Fun_Dsp( FUNMSG[0] );														// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" 

	if( event == FPRN5_CHK ){
		FrmLogoRegist.prn_kind = R_PRI;				// �������ʁFڼ��
	}
	else{
		FrmLogoRegist.prn_kind = J_PRI;				// �������ʁF�ެ���
	}

	queset( PRNTCBNO, PREQ_LOGO_REGIST, sizeof( T_FrmLogoRegist ), &FrmLogoRegist ); 		// ���S�󎚃f�[�^�o�^�v��
	Ope_DisableDoorKnobChime();

	Lagtim( OPETCBNO, 6, 5*50 );					// �o�^����ʕ\����ϰ6(5s)�N���i�o�^���̉�ʕ\�����T�b�ԕێ�����j
	dsptimer	= ON;								// �o�^����ʕ\����ϰ�׸޾��
	regend		= OFF;								// �o�^�I���׸�ؾ��

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );				// ү���ގ�M

		switch( msg){			// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case LOGOREG_END:	// �o�^�I��

				if( dsptimer == OFF ){					// �o�^����ʕ\����ϰ��ѱ�čς݁H
					ret = MOD_EXT;						// YES���O��ʂɖ߂�
				}
				else{
					regend = ON;						// NO�@���@�o�^�I���׸޾��
				}

				break;

			case TIMEOUT6:		// �o�^����ʏI����ϰ��ѱ��

				dsptimer = OFF;							// �o�^����ʕ\����ϰ��ѱ��

				if( regend == ON ){						// �o�^�I���ς݁H
					ret = MOD_EXT;						// YES���O��ʂɖ߂�
				}

				break;

			default:			// ���̑�
				break;
		}

		if( ret != 0 ){									// �o�^����ʏI���H

			if( dsptimer == ON ){						// �o�^����ʕ\����ϰ�N�����H
				Lagcan( OPETCBNO, 6 );					// �o�^����ʕ\����ϰ����
			}

			break;
		}
	}
	return( ret );
}

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//[]----------------------------------------------------------------------[]
///	@brief			���[�h�E���C�g�e�X�g�i�d�q�W���[�i���j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			MOD_CHG=���[�h�ؑ�, MOD_EXT=�I��
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort FPrnChk_RW(void)
{
	ushort	ret = 0;	// �߂�l
	short	msg = -1;	// ��Mү����
	ulong	time;

	if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE) ||
		IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR) ||
		ALM_CHK[ALMMDL_SUB][ALARM_SD_END] != 0) {
		BUZPIPI();
		return MOD_EXT;
	}

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18]);		// "���d�q�W���[�i���`�F�b�N���@�@"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[1]);			// "�y���[�h�E���C�g�e�X�g�z�@�@�@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[24]);		// "�@�@���΂炭���҂����������@�@"

	MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_SD_TEST_RW);				// SD�J�[�h�e�X�g�i���[�h���C�g�j

	while (ret == 0) {

		msg = StoF( GetMessage(), 1 );			// ү���ގ�M
		switch( msg){							// ��Mү���ށH
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
		case KEY_MODECHG:						// ����Ӱ�ސؑ�
			BUZPI();
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:						// �e�T�i�I���j
			BUZPI();
			ret = MOD_EXT;
			break;
		case PREQ_SD_TEST_RW:					// SD�J�[�h�e�X�g�i���[�h���C�g�j���ʒʒm
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�e�X�g�������b�Z�[�W�\���j
//			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[27]);// "�@���[�h���ԁF�@�@�@�@�@�����@"
//			memcpy(&time, &eja_work_buff[0], 4);
//			opedpl3(3, 15, time, 9, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// ���[�h����
//
//			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[28]);// "�@���C�g���ԁF�@�@�@�@�@�����@"
//			memcpy(&time, &eja_work_buff[4], 4);
//			opedpl3(4, 15, time, 9, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// ���C�g����
			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[32]);// "�e�X�g�ɐ������܂����@�@�@�@�@"
			displclr(3);
			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[27]);// "�@���[�h���ԁF�@�@�@�@�@�����@"
			memcpy(&time, &eja_work_buff[0], 4);
			opedpl3(4, 15, time, 9, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// ���[�h����

			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[28]);// "�@���C�g���ԁF�@�@�@�@�@�����@"
			memcpy(&time, &eja_work_buff[4], 4);
			opedpl3(5, 15, time, 9, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// ���C�g����
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�e�X�g�������b�Z�[�W�\���j

			Fun_Dsp( FUNMSG[8] );										// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
			break;
		case PREQ_ERR_END:
			displclr(3);
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[29]);// "�@�@ �e�X�g�Ɏ��s���܂��� �@�@"
			Fun_Dsp( FUNMSG[8] );										// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
			break;
		default:
			break;
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			SD�J�[�h���i�d�q�W���[�i���j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			MOD_CHG=���[�h�ؑ�, MOD_EXT=�I��
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort FPrnChk_SD(void)
{
	ushort	ret = 0;	// �߂�l
	short	msg = -1;	// ��Mү����
	ulong	upper, lower, work;

	if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_SD_UNAVAILABLE) ||
		IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_WRITE_ERR)) {
		BUZPIPI();
		return MOD_EXT;
	}

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18]);		// "���d�q�W���[�i���`�F�b�N���@�@"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[23]);		// "�y�r�c�J�[�h���z�@�@�@�@�@�@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[24]);		// "�@�S�̃T�C�Y�F�@�@�@�@�@�l�a�@"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[25]);		// "�@�󂫃T�C�Y�F�@�@�@�@�@�l�a�@"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[26]);		// "�@�g�p���ԁ@�F�@�@�@�@�@���ԁ@"

	Fun_Dsp( FUNMSG[8] );												// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_SD_INFO);					// SD�J�[�h���v��

	while (ret == 0) {

		msg = StoF( GetMessage(), 1 );			// ү���ގ�M
		switch( msg){							// ��Mү���ށH
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
		case KEY_MODECHG:						// ����Ӱ�ސؑ�
			BUZPI();
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:						// �e�T�i�I���j
			BUZPI();
			ret = MOD_EXT;
			break;
		case PREQ_SD_INFO:						// SD�J�[�h���ʒm
			memcpy(&upper, &eja_work_buff[0], 4);
			memcpy(&lower, &eja_work_buff[4], 4);
			work = upper * (1 << 12);			// ��ʌ���MB�P�ʂɕϊ�
			work += (lower / (1024*1024));		// ���ʌ���MB�P�ʂɕϊ�
			opedpl3(3, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// �S�̃T�C�Y

			memcpy(&upper, &eja_work_buff[8], 4);
			memcpy(&lower, &eja_work_buff[12], 4);
			work = upper * (1 << 12);			// ��ʌ���MB�P�ʂɕϊ�
			work += (lower / (1024*1024));		// ���ʌ���MB�P�ʂɕϊ�
			opedpl3(4, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// �󂫃T�C�Y

			memcpy(&work, &eja_work_buff[16], 4);
			opedpl3(5, 16, work, 8, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	// �g�p����
			break;
		default:
			break;
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�o�[�W�����m�F�i�d�q�W���[�i���j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			MOD_CHG=���[�h�ؑ�, MOD_EXT=�I��
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort FPrnChk_Ver(void)
{
	ushort	ret = 0;	// �߂�l
	short	msg = -1;	// ��Mү����
	uchar	version[8];
	uchar	sd_version[8];

	dispclr();

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[18]);		// "���d�q�W���[�i���`�F�b�N���@�@"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[4]);			// "�y�o�[�W�����m�F�z�@�@�@�@�@�@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[6]);			// "�@�o�[�W���� �� �@�@�@�@�@�@�@"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[30]);		// "�@SD�����ް� �� �@�@�@�@�@�@�@"

	Fun_Dsp( FUNMSG[8] );												// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_VER_INFO);					// �\�t�g�o�[�W�����v��

	while (ret == 0) {

		msg = StoF( GetMessage(), 1 );			// ү���ގ�M
		switch( msg){							// ��Mү���ށH
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
		case KEY_MODECHG:						// ����Ӱ�ސؑ�
			BUZPI();
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:						// �e�T�i�I���j
			BUZPI();
			ret = MOD_EXT;
			break;
		case PREQ_SW_VER_INFO:					// �\�t�g�o�[�W�����ʒm
			memcpy(version, eja_work_buff, 8);
			grachr(3, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, version);	// �o�[�W�����\��
			// �\�t�g�o�[�W�����ʒm�̎�M���SD�J�[�h�o�[�W�����v���𑗐M����
			MsgSndFrmPrn(PREQ_PRN_INFO, J_PRI, PINFO_SD_VER_INFO);				// SD�J�[�h�o�[�W�����v��
			break;
		case PREQ_SD_VER_INFO:					// SD�J�[�h�o�[�W�����ʒm
			memcpy(sd_version, eja_work_buff, 8);
			grachr(4, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, sd_version);	// SD�J�[�h�o�[�W�����\��
			break;
		default:
			break;
		}
	}
	return ret;
}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Rdr                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���Cذ�ް��������                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Rdr( void )
{
	unsigned short	usFrdrEvent;
	char			wk[2];
	char			org[2];
	short			msg;
	int				mode = -1;
	unsigned char set;

	org[0]		= DP_CP[0];
	org[1]		= DP_CP[1];
	DP_CP[0]	= DP_CP[1] = 0;

	if( opr_snd( 95 ) == 0 ){ 		// ��ԗv��
		Lagtim( OPETCBNO, 6, 2*50 );// ��ԗv����ϰ(2s)�N��
	}
	while (mode < 0) {
		msg = StoF(GetMessage(), 1);		// ү���ގ�M
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:					// ����Ӱ�ސؑ�
			BUZPI();
			return(MOD_CHG);
			break;
		case ARC_CR_E_EVT:					// ��Ԏ�M
			mode = (RED_REC.ercd == 0) ? 1: 0;
			break;
		case TIMEOUT6:						// ��ԗv����ϰ��ѱ��
			mode = 0;						// �f�B�b�v�X�C�b�`�`�F�b�N�ɑJ�ڂ����Ȃ�
			break;
		default :
			break;
		}
	}

	for( ; ; )
	{
		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );			// "�����C���[�_�[�`�F�b�N���@�@�@"
		Fun_Dsp( FUNMSG[25] );						// "�@���@�@���@�@�@�@ �Ǐo  �I�� "

		usFrdrEvent = Menu_Slt( FRDMENU, FRDR_CHK_TBL, (char)FRDR_CHK_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFrdrEvent ){

			case FRD1_CHK:							// ���[�h�^���C�g�e�X�g
				usFrdrEvent = FRdrChk_RW();
				break;

			case FRD2_CHK:							// �󎚃e�X�g
				set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
				if(set == 1 || set == 2) {
					usFrdrEvent = FRdrChk_Pr();
				}
				else {
					BUZPIPI();
				}
				break;

			case FRD3_CHK:							// ��Ԋm�F
				usFrdrEvent = FRdrChk_Sts();
				break;

			case FRD4_CHK:							// �o�[�W�����m�F
				usFrdrEvent = FRdrChk_Ver();
				break;
			case FRD5_CHK:							// �f�B�b�v�X�C�b�`�m�F
				if (mode > 0) {
					usFrdrEvent = FRdrChk_DipSwitch();
				}
				else {
					BUZPIPI();
				}
				break;

			case MOD_EXT:							// �I��
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFrdrEvent;
				break;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usFrdrEvent == MOD_CHG ){
		if( usFrdrEvent == MOD_CHG || usFrdrEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFrdrEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_RW                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���Cذ�ްذ�ށEײ�ýď���                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define		RDRSTSCHK1	0xffff0083l		// ذ�ް�ð�������ް��i������p�j
#define		RDRSTSCHK2	0xffff0383l		// ذ�ް�ð�������ް��i�l�����p�j

unsigned short	FRdrChk_RW( void )
{
	ushort		ret = 0;		// �߂�l
	short		msg = -1;		// ��Mү����
	uchar		req = 0;		// ýėv�����
	uchar		prm;			// ���Cذ�ް���
	ulong		m;				// ذ�ް�ð�������ް�
	ulong		l;				// ذ�ް�ð��


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );				// "�����C���[�_�[�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[1] );				// "�y���[�h�E���C�g�e�X�g�z�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[5] );				// "�@���C�J�[�h�����Ă��������@"

	Fun_Dsp( FUNMSG[8] );							// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "


	prm = (uchar)prm_get( COM_PRM, S_PAY, 21, 1, 3 );	// ���Cذ�ް��ʎ擾

	if( prm == 1 || prm == 3 ){						// ���Cذ�ް��ʁH
		// �����
		m = RDRSTSCHK1;								// ذ�ް�ð�������ް���āi������p�j
	}
	else{
		// �l����
		m = RDRSTSCHK2;								// ذ�ް�ð�������ް���āi�l�����p�j
	}

	for( ; ret == 0 ; ){

		if( req == 0 ){
			if( opr_snd( 92 ) == 0 ){				// ذ��ײ�ýėv��
				// ����ޑ��M�n�j
				read_sht_opn();						// ���Cذ�ް������u�J�v
				grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[5] );	// "�@���C�J�[�h�����Ă��������@"
				req = 1;
			}else{
				// ����ޑ��M�m�f
				return( MOD_EXT );
			}
		}

		msg = StoF( GetMessage(), 1 );				// ү���ގ�M

		switch( msg){								// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:						// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:						// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;
				break;

			case ARC_CR_E_EVT:						// �I������ގ�M

				if( req != 2 ){

					memcpy( &l, RED_stat, 4 );							// ذ�ް�ð���擾

					if(( RED_REC.ercd == 0 ) && (( l & m ) == 0l )){	// ýČ��ʁH
						// ����I��
						grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[10] );				// "�@����ł��@�@�@�@�@�@�@�@�@�@"
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						inc_dct( READ_WR, 1 );
//						inc_dct( READ_YO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
					}
					else{
						// �ُ�I��
					ret = RdrChk_ErrDsp( RED_REC.ercd, ( l & m ));		// �װ�\����ʏ���
					if(RED_REC.ercd == E_VERIFY || RED_REC.ercd == E_VERIFY_30){
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						inc_dct( READ_VN, 1 );
//						inc_dct( READ_WR, 1 );
//						inc_dct( READ_YO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
					}else{
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						inc_dct( READ_ER, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)

					}
					}
					req = 2;
				}
				else
				{
					req = 0;
				}

				break;

			default:
				break;
		}
	}
	read_sht_cls();														// ���Cذ�ް������u�v
	opr_snd( 90 );														// ��ԗv��
	FRdrChk_Wait();

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_Pr                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���Cذ�ް��ýď���                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FRdrChk_Pr( void )
{
	ushort		ret = 0;		// �߂�l
	short		msg = -1;		// ��Mү����
	uchar		req = 0;		// ýėv�����
	uchar		prm;			// ���Cذ�ް���
	ulong		m;				// ذ�ް�ð�������ް�
	ulong		l;				// ذ�ް�ð��


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );				// "�����C���[�_�[�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[2] );				// "�y�󎚃e�X�g�z�@�@�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[5] );				// "�@���C�J�[�h�����Ă��������@"

	Fun_Dsp( FUNMSG[8] );							// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "


	prm = (uchar)prm_get( COM_PRM, S_PAY, 21, 1, 3 );	// ���Cذ�ް��ʎ擾

	if( prm == 1 || prm == 3 ){						// ���Cذ�ް��ʁH
		// �����
		m = RDRSTSCHK1;								// ذ�ް�ð�������ް���āi������p�j
	}
	else{
		// �l����
		m = RDRSTSCHK2;								// ذ�ް�ð�������ް���āi�l�����p�j
	}

	for( ; ret == 0 ; ){

		if( req == 0 ){
			if( opr_snd( 93 ) == 0 ){				// ��ýėv��
				// ����ޑ��M�n�j
				read_sht_opn();						// ���Cذ�ް������u�J�v
				grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[5] );	// "�@���C�J�[�h�����Ă��������@"
				req = 1;
			}else{
				// ����ޑ��M�m�f
				return( MOD_EXT );
			}
		}

		msg = StoF( GetMessage(), 1 );				// ү���ގ�M

		switch( msg){								// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:						// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:						// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;
				break;

			case ARC_CR_E_EVT:						// �I������ގ�M

				if( req != 2 ){

					memcpy( &l, RED_stat, 4 );							// ذ�ް�ð���擾

					if(( RED_REC.ercd == 0 ) && (( l & m ) == 0l )){	// ýČ��ʁH
						// ����I��
						grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[10] );				// "�@����ł��@�@�@�@�@�@�@�@�@�@"
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						inc_dct( READ_YO, 1 );
//						inc_dct( VPRT_DO, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)			
					}
					else{
						// �ُ�I��
					ret = RdrChk_ErrDsp( RED_REC.ercd, ( l & m ));		// �װ�\����ʏ���
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					inc_dct( READ_ER, 1 );
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
					}
					req = 2;
				}

				break;

			default:
				break;
		}
	}
	read_sht_cls();														// ���Cذ�ް������u�v
	opr_snd( 90 );														// ��ԗv��
	FRdrChk_Wait();

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RdrChk_ErrDsp                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���Cذ�ްýĴװ��ʏ���                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char ercd                                      |*/
/*|                unsigned long errbit                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	RDR_ERR_MAX		10
// ���Cذ�ް�ð�������p�ް��i����bit���װү����NO.�j
const	unsigned long	rdr_sts_chk[32][2] =
	{
// �ð�����P
		0x01000000l,	1,	//	Bit24:S1�װсiذ�ް�ʒu�ݻ�1�j
		0x02000000l,	2,	//	Bit25:S2�װсiذ�ް�ʒu�ݻ�2�j
		0x04000000l,	3,	//	Bit26:S3�װсiذ�ް�ʒu�ݻ�3�j
		0x08000000l,	4,	//	Bit27:S4�װсiذ�ް�ʒu�ݻ�4�j
		0x10000000l,	5,	//	Bit28:S5�װсiذ�ް�ʒu�ݻ�5�j
		0x20000000l,	6,	//	Bit29:S6�װсiذ�ް�ʒu�ݻ�6�j
		0x40000000l,	7,	//	Bit30:S7�װсiذ�ް�ʒu�ݻ�7�j
		0x80000000l,	8,	//	Bit31:SP�װсi������ʒu�ݻ��j

// �ð�����Q
		0x00010000l,	9,	//	Bit16:S1�s�ǁiذ�ް�ʒu�ݻ�1�j
		0x00020000l,	10,	//	Bit17:S2�s�ǁiذ�ް�ʒu�ݻ�2�j
		0x00040000l,	11,	//	Bit18:S3�s�ǁiذ�ް�ʒu�ݻ�3�j
		0x00080000l,	12,	//	Bit19:S4�s�ǁiذ�ް�ʒu�ݻ�4�j
		0x00100000l,	13,	//	Bit20:S5�s�ǁiذ�ް�ʒu�ݻ�5�j
		0x00200000l,	14,	//	Bit21:S6�s�ǁiذ�ް�ʒu�ݻ�6�j
		0x00400000l,	15,	//	Bit22:S7�s�ǁiذ�ް�ʒu�ݻ�7�j
		0x00800000l,	16,	//	Bit23:SP�s�ǁi������ʒu�ݻ��j

// �ð�����R
		0x00000100l,	17,	//	Bit08:���Cͯ��HP�ݻ�1�s��
		0x00000200l,	18,	//	Bit09:���Cͯ��HP�ݻ�2�s��
		0,				0,	//	Bit10:�\��					���װ�\���Ȃ�
		0,				0,	//	Bit11:�\��					���װ�\���Ȃ�
		0,				0,	//	Bit12:�}������				���װ�\���Ȃ�
		0,				0,	//	Bit13:�\��					���װ�\���Ȃ�
		0,				0,	//	Bit14:�\��					���װ�\���Ȃ�
		0,				0,	//	Bit15:�O���o�^				���װ�\���Ȃ�

// �ð�����S
		0x00000001l,	19,	//	Bit00:���ދl�܂�iذ�ް�j
		0x00000002l,	20,	//	Bit01:���ދl�܂�i������j
		0x00000004l,	21,	//	Bit02:Ӱ���̏�iذ�ް�j		�����g�p�i�ð�����ɔ��f����Ȃ��j
		0x00000008l,	22,	//	Bit03:Ӱ���̏�i������j		�����g�p�i�ð�����ɔ��f����Ȃ��j
		0,				0,	//	Bit04:�\��					���װ�\���Ȃ�
		0,				0,	//	Bit05:���ޑ}���҂�			���װ�\���Ȃ�
		0,				0,	//	Bit06:����ގ�M�҂�			���װ�\���Ȃ�
		0x00000080l,	23	//	Bit07:RAM�s��

	};

// ���Cذ�ް�װ���������p�ް��i�װ���ށ��װү����NO.�j
const	unsigned char	rdr_erc_chk[RDR_ERR_MAX][2] =
	{
		0x11,	24,			// �Z���װ
		0x12,	25,			// �����װ
		0x21,	26,			// ���ĕ����װ
		0x22,	27,			// �ް��װ
		0x23,	28,			// ���è��װ
		0x26,	29,			// ��ײذ�޴װ
		0x27,	30,			// ���̧��װ
		0x31,	31,			// �s�𗝺���ގ�M
		0x24,	32,			// �����݂b�q�b�G���[
		0x25,	33			// �ǎ��b�q�b�G���[
	};

unsigned short	RdrChk_ErrDsp( uchar ercd, ulong errbit )
{
	unsigned short	ret			= 0;	// �߂�l
	unsigned char	e_page_cnt	= 1;	// �ُ�ð���\���߰�ސ�
	unsigned char	e_page_pos	= 1;	// �ُ�ð���\���߰�ވʒu
	unsigned char	e_cnt		= 0;	// �ُ�ð����
	unsigned char	i;					// ٰ�ߏ��������
	unsigned char	err_no[33];			// �ُ�ð��NO.�i�[�ر
	unsigned char	set_pos		= 0;	// �ُ�ð��NO.�i�[�߲��
	short			msg = -1;			// ��Mү����


	memset( err_no, 0, 33 );							// �ُ�ð��NO.�i�[�ر�ر

	for( i = 0 ; i < 32 ; i++ ){						// �ُ�ð���ް�����
		if( errbit & rdr_sts_chk[i][0] ){
			// �ُ킠��
			e_cnt++;									// �ُ�ð�����X�V�i+1�j
			err_no[set_pos] = (uchar)rdr_sts_chk[i][1];	// �ُ�ð��NO.�i�[�ر�ֈُ�ð��NO.���i�[
			set_pos++;									// �i�[�߲���X�V�i+1�j
		}
	}

	if( ercd != 0 ){									// �װ�����H

		for( i = 0 ; i < RDR_ERR_MAX ; i++ ){						// �װ���ތ���
			if( ercd == rdr_erc_chk[i][0] ){
				// �װ���ވ�v
				e_cnt++;								// �ُ�ð�����X�V�i+1�j
				err_no[set_pos] = rdr_erc_chk[i][1];	// �ُ�ð��NO.�i�[�ر�ֈُ�ð��NO.���i�[
				set_pos++;								// �i�[�߲���X�V�i+1�j
				break;
			}
		}
	}

	if( e_cnt ){										// �ُ킠��H

		// �ُ킪����ꍇ

		e_page_cnt = (uchar)(e_cnt / 5);				// �ُ�ð��������\���߰�ސ����(�P�߰�ނɍő�T�ð���܂ŕ\��)
		if( e_cnt % 5 ){
			e_page_cnt++;
		}
	}
	Rdr_err_dsp( e_page_pos, &err_no[0] );				// �ُ�ð���\��

	if( e_page_cnt == 1 ){								// ̧ݸ��ݷ��\��
		// ���ɕ\���y�[�W���Ȃ��ꍇ
		Fun_Dsp( FUNMSG[8] );							// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
	}
	else{
		// ���ɕ\���y�[�W������ꍇ
		Fun_Dsp( FUNMSG[6] );							// "�@���@�@���@�@�@�@�@�@�@ �I�� "
	}

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );					// ү���ގ�M

		switch( msg){									// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:							// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:							// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F1:							// �e�P�i���F�O�y�[�W�ؑցj

				if( e_page_cnt != 1 ){

					BUZPI();

					if( e_page_pos == 1 ){
						e_page_pos = e_page_cnt;
					}
					else{
						e_page_pos--;
					}
					Rdr_err_dsp( e_page_pos, &err_no[0] );
				}

				break;

			case KEY_TEN_F2:							// �e�Q�i���F���y�[�W�ؑցj

				if( e_page_cnt != 1 ){

					BUZPI();

					e_page_pos++;
					if( e_page_pos > e_page_cnt ){
						e_page_pos = 1;
					}
					Rdr_err_dsp( e_page_pos, &err_no[0] );
				}

				break;

			default:									// ���̑�
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Rdr_err_dsp                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���Cذ�ް�����ُ�ð���\������                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char pos�i�\���߰�ވʒu�j                      |*/
/*|              : unsigned char *data�i�ُ�ð��ް���߲���j               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/15                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Rdr_err_dsp( unsigned char pos, unsigned char *data )
{
	unsigned char	max;
	unsigned char	ofs;
	unsigned char	no;
	unsigned char	i;

	switch( pos ){		// �߰�ވʒu�H

		case	1:		// �P�y�[�W��
			max = 5;
			ofs = 0;
			break;

		case	2:		// �Q�y�[�W��
			max = 5;
			ofs = 5;
			break;

		case	3:		// �R�y�[�W��
			max = 5;
			ofs = 10;
			break;

		case	4:		// �S�y�[�W��
			max = 5;
			ofs = 15;
			break;

		case	5:		// �T�y�[�W��
			max = 5;
			ofs = 20;
			break;

		case	6:		// �U�y�[�W��
			max = 5;
			ofs = 25;
			break;

		case	7:		// �V�y�[�W��
			max = 3;
			ofs = 30;
			break;

		default:		// ���̑��i�y�[�W�G���[�j
			return;
	}
	for( i = 2 ; i <= 6 ; i++ ){	// ײ݂Q�`�U�ر
		displclr( (unsigned short)i );
	}

	for( i = 0 ; i < max ; i++ ){	// �ُ�ð���\���i�w��y�[�W�̍ő�s���A�܂��͕\���ް��I���܂Łj

		no = data[ofs+i];											// �\������ُ�ð��NO.�擾
		if( no == 0 ){
			break;													// �\���ް��I��
		}
		grachr( (unsigned short)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDESTR[no] );	// �ُ�ð���\��
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_Sts                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���Cذ�ް��Ԋm�F����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FRdrChk_Sts( void )
{
	ushort		ret = 0;		// �߂�l
	ushort		req = OFF;		// ��ԗv�����
	ushort		reqtimer = OFF;	// ��ԗv����ϰ���
	short		msg = -1;		// ��Mү����


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );			// "�����C���[�_�[�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[3] );			// "�y��Ԋm�F�z�@�@�@�@�@�@�@�@�@"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[7] );			// "�L���@S1S2S3S4S5S6S7�@SP�@�@HP"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[8] );			// "�ǔہ@�@�@�@�@�@�@�@�@�@�@�@�@"
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[9] );			// "���m�@�@�@�@�@�@�@�@�@�@�@�@�@"

	Fun_Dsp( FUNMSG[8] );						// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	for( ; ; ){

		if( req == OFF ) {
			if( opr_snd( 90 ) == 0 ){			// ��ԗv��
				req = ON;
			}
		} else {
			if( RD_mod < 6 ) {
				req = OFF;
			}
		}

		msg = StoF( GetMessage(), 1 );			// ү���ގ�M

		switch( msg){							// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:					// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:					// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;
				break;

			case ARC_CR_E_EVT:					// ��Ԏ�M

				Rdr_sts_dsply( RED_REC.posi[0], RED_REC.posi[1], RED_stat );	// ��ԕ\��
				Lagtim( OPETCBNO, 6, 2*50 );									// ��ԗv����ϰ(2s)�N���i��Ԋm�F��ʒ��A�Q�b���ɏ�ԗv�����s���j
				reqtimer = ON;

				break;

			case TIMEOUT6:						// ��ԗv����ϰ��ѱ��

				req = OFF;
				reqtimer = OFF;
				break;

			default:
				break;
		}
		if( ret != 0 ){		// ��Ԋm�F�I���H

			if( reqtimer == ON ){				// ��ԗv����ϰ�N�����H
				Lagcan( OPETCBNO, 6 );			// ��ԗv����ϰ����
			}

			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Rdr_sts_dsply                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���Cذ�ް��Ԋm�F����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : uchar pos1:�ݻ����                                     |*/
/*|              : uchar pos2:���Cͯ�ވʒu                                 |*/
/*|              : uchar *sta:ذ�ް���                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Rdr_sts_dsply( uchar pos1, uchar pos2, uchar *sta )
{
	uchar	i,d;
	uchar	printerUse ;

	printerUse = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
	for( i = 0; i < 8; i++ ){
		d = 0;
		if((( sta[1] >> i ) & 0x01 ) == 1 ) {
			d = 1;
		} else {
			if((( sta[0] >> i ) & 0x01 ) == 1 ) {
				d = 2;
			}
		}
		if(i < 7) {
		grachr( 3, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_5[d] );						// �ǔہiS1�`SP�j

		grachr( 4, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_5[(( pos1 >> i ) & 0x01 )] );	// ���m�iS1�`SP�j
		}
		else {
			// �v�����^���g�p�̐ݒ�ł�SP�̗ǔہA���m��'�|'��\������
			if(printerUse == 1 || printerUse == 2) {	// �v�����^����
				grachr( 3, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_5[d] );						// �ǔہiSP�j
				grachr( 4, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_5[(( pos1 >> i ) & 0x01 )] );	// ���m�iSP�j
			}
			else {										// �v�����^�Ȃ�
				grachr( 3, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3] );						// �ǔہiSP�j
				grachr( 4, (ushort)(( i < 7 ) ? 6 + i * 2 : 22 ), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_2[3] );						// ���m�iSP�j
			}
		}
	}
	grachr( 4, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_6[ (pos2 & 0x03) ] );													// HP�iͯ�ވʒu�j
}

//[]----------------------------------------------------------------------[]
///	@brief			�f�B�b�v�X�C�b�`�m�F���� 
//[]----------------------------------------------------------------------[]
///	@return			ret		: MOD_CHG or MOD_EXT
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	: 2008/02/29<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
#define		DIPSWITCH_MAX		4
static unsigned short	FRdrChk_DipSwitch( void )
{
	char		i;					// ���[�v
	int			pos;				// SW1�`4
	uchar		display;			// �\���̗L��
	uchar		sts;				// �f�B�b�v�X�C�b�`�̏��(0:OFF, 1:ON)
	uchar		sts_bit;			// �f�B�b�v�X�C�b�`�`�F�b�N�r�b�g
	int			req;				// ��ԗv�����
	ushort		ret;				// �߂�l
	short		msg;				// ��Mү����

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0]);				// "�����C���[�_�[�`�F�b�N���@�@�@"
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[11]);			// "�y�f�B�b�v�X�C�b�`�m�F�z�@�@�@"
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[12]);			// "�@�@�@�@�@�P�Q�R�S�@�@�@�@�@�@"
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[13]);			// "�@�@�@ON�@�@�@�@�@�@�@�@�@�@�@"
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[14]);			// "�@�@�@OFF �@�@�@�@�@�@�@�@�@�@"
	Fun_Dsp(FUNMSG2[42]);						// "�r�v�P �r�v�Q�@�@ �r�v�S �I�� "

	display = OFF;
	pos = 0;
	req = 0;
	msg = -1;
	ret = 0;
	do {
		switch ( msg ) {						// ��Mү����
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			ret = MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:					// ����Ӱ�ސؑ�
			BUZPI();
			ret = MOD_CHG;
			break;
		case KEY_TEN_F5:					// �e�T�i�I���j
			BUZPI();
			ret = MOD_EXT;
			break;
		case KEY_TEN_F1:					// �e�P
		case KEY_TEN_F2:					// �e�Q
		case KEY_TEN_F4:					// �e�S
			BUZPI();
			pos = (int)(msg - KEY_TEN_F1);
		case -1:
			grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[(pos == 3)? 17 : 15+pos] );
							// "�r�v�P�F�P�̃e�X�g�E�����@�@�@"
							// "�r�v�Q�F�v���O�����@�\��ʁ@�@"
							// "�r�v�S�F�ʐM�d�l�E�ʐM�A�h���X"
			req = 0;
			break;
		case ARC_CR_E_EVT:					// ��Ԏ�M
			if (req == 1) {
				if (RED_REC.ercd == 0) {
				// no error
					display  = ON;
					req = 2;
				}
			}
			break;
		case TIMEOUT6:						// ��ԗv����ϰ��ѱ��
			if (req == 1) {
				req = 0;
				BUZPIPI();					// ��񂪎擾�o���Ȃ��Ƃ��͏I��
				ret = MOD_EXT;
			}
			break;
		default:							// ���̑�
			break;
		}

		if( req == 0 ) {
			if( opr_snd( 95 ) == 0 ){ 		// ��ԗv��
				Lagtim( OPETCBNO, 6, 2*50 );// ��ԗv����ϰ(2s)�N��
				req = 1;
			}
		}
		if (display == ON) {
			displclr(3);
			displclr(4);
			sts = RDS_REC.state[pos];			// ����4bit�擾
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[13]);			// "�@�@�@ON�@�@�@�@�@�@�@�@�@�@�@"
			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[14]);			// "�@�@�@OFF �@�@�@�@�@�@�@�@�@�@"
			sts_bit = (0x01 << (DIPSWITCH_MAX-1));
			for (i = 0; i < DIPSWITCH_MAX; i++, sts_bit >>= 1) {
				if (sts & sts_bit) {
					grachr(3, (ushort)(10+(i*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[9]);	// ON��"��"
				} else {
					grachr(4, (ushort)(10+(i*2)), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[9]);	// OFF��"��"
				}
			}
			display = OFF;
		}
		msg = StoF(GetMessage(), 1);		// ү���ގ�M
	} while(ret == 0);
	Lagcan( OPETCBNO, 6 );		// ��ԗv����ϰ����
	return(ret);
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_Ver                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���Cذ�ް�ް�ޮ݊m�F����                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/13                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FRdrChk_Ver( void )
{
	ushort		ret = 0;	// �߂�l
	short		msg = -1;	// ��Mү����
	uchar		version[10];

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[0] );			// "�����C���[�_�[�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[4] );			// "�y�o�[�W�����m�F�z�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, RDRSTR[6] );			// "�@�o�[�W���� �� �@�@�@�@�@�@�@"

	Fun_Dsp( FUNMSG[8] );						// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	// 8���̃o�[�W������\������
	opr_snd( 94 );								// �ް�ޮݗv��

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );			// ү���ގ�M

		switch( msg ){							// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:					// ����Ӱ�ސؑ�
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F5:					// �e�T�i�I���j
				BUZPI();
				ret = MOD_EXT;
				break;
			case ARC_CR_VER_EVT:				// �ް�ޮ��ް���M
				memset(version, 0, sizeof(version));
				memcpy(version, RVD_REC.vers, sizeof(RVD_REC.vers));
				grachr( 3, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, version );		// �o�[�W�����m���D�\��
				break;
			default:							// ���̑�
				break;
		}
	}
	return( ret );
}


/*[]----------------------------------------------------------------------[]*/
/*| �R�C�����b�N�`�F�b�N                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Cmc( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#define	COINCHK_CNT	4	//�\�~�A�ۗ��͕ʉ��

unsigned short	FunChk_Cmc( void ){

	unsigned short	usFcmcEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		/* // [00]	"���R�C�����b�N�`�F�b�N���@�@�@" */

		usFcmcEvent = Menu_Slt( FCMCMENU, FCMC_CHK_TBL, (char)FCMC_CHK_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFcmcEvent ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;
			case KEY_TEN_F5:						/* F4:Exit */
				BUZPI();
				return( MOD_EXT );
				break;

			case FCM1_CHK:
				wopelg( OPLOG_NYUKINCHK, 0, 0 );	// ���엚��o�^
				usFcmcEvent = CmcInchk();
				break;
			case FCM2_CHK:
				wopelg( OPLOG_HARAIDASHICHK, 0, 0 );	// ���엚��o�^
				usFcmcEvent = CmcOutchk();
				break;
			case FCM3_CHK:
				usFcmcEvent = CmcStschk();
				break;
			case FCM4_CHK:
				usFcmcEvent = CmcVerchk();
				break;

			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFcmcEvent;
				break;
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usFcmcEvent == MOD_CHG ){
		if( usFcmcEvent == MOD_CHG || usFcmcEvent == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFcmcEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcInchk                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���ү�����ýď���                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CmcInchk( void )
{
	unsigned short	ret=0;					// �߂�l
	short			msg = -1;				// ��Mү����
	unsigned char	i;						// ٰ�ߏ��������
	unsigned char	cincnt[COINCHK_CNT];	// ��ݓ��������ۑ�ܰ�
	unsigned char	cnt_total;				// ��ݓ����������v
	memset( cincnt, 0, COINCHK_CNT );		// ��ݓ��������ۑ�ܰ��ر
	cn_stat( 33, 0 );						// ��������(CREM:ON)

	InchkDsp();								// �����e�X�g��ʏ�����ԕ\��
	for( i = 0; i < COINCHK_CNT ; i++ ){			// ��ݓ��������\��
		opedsp( (unsigned short)(i + 2), 14,  0, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );			// �O���\��
	}
	xPause( 50L );							// 500ms Pause

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );		// ү���ގ�M

		switch( msg ){						// ��Mү���ށH

// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:				// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F1:				// �e�P�i��ʐؑցj

				BUZPI();

				ret = CmcInchk2( cincnt );	// �ޑK�����\����ʏ���

				if( ret == 0 ){				// �����e�X�g�p���H
					InchkDsp();										// �����e�X�g��ʏ�����ԕ\��
					for( i = 0; i < COINCHK_CNT ; i++ ){			// ��ݓ��������\��
						cnt_total = bcdadd( cincnt[i], CN_RDAT.r_dat08[i] );
						opedsp( (unsigned short)(i + 2), 14, (unsigned short)bcdbin(cnt_total), 3, 0, 0,
																		COLOR_BLACK, LCD_BLINK_OFF );
					}
				}

				break;

			case KEY_TEN_F3:				// �e�R�i�N���A�j

				BUZPI();

				memset( cincnt, 0, COINCHK_CNT );					// ��ݓ��������ۑ�ܰ��ر

				for( i = 0; i < COINCHK_CNT ; i++ ){				// ��ݓ��������\���i�S�ĂO���j
					opedsp( (unsigned short)(i + 2), 14, 0, 3, 0, 0,
									COLOR_BLACK, LCD_BLINK_OFF  );
				}
				cn_stat( 8, 0 );									// �����s��(CREM:OFF,������ر)
				Lagtim( OPETCBNO, 6, 5*50 );						// ��ϰ6(5s)�N��(����ݽ����p)
				for( ; ; ){
					msg = GetMessage();								// ү���ގ�M
					if( msg == COIN_EVT )							// �R�C�����b�N�C�x���g
						break;
					// �R�C�����b�N���C���A�E�g�Ď�
					if( msg == TIMEOUT6 ){		// �^�C�}�[�U�^�C���A�E�g�Ď�
						break;
					}
				}
				Lagcan( OPETCBNO, 6 );
				cn_stat( 5, 0 );									// ��������(CREM:ON,������ر)

				break;

			case COIN_EVT:					// �R�C�����b�N�C�x���g

				switch( OPECTL.CN_QSIG ){

					case	1:				// ���ү�����u��������v��M
					case	5:				// ���ү�����u��������imax�B���j�v��M

						for( i = 0; i < COINCHK_CNT ; i++ ){		// ��ݓ��������\��
							cnt_total = bcdadd( cincnt[i], CN_RDAT.r_dat08[i] );
							opedsp( (unsigned short)(i + 2), 14, (unsigned short)bcdbin(cnt_total), 3, 0, 0,
																			COLOR_BLACK, LCD_BLINK_OFF );
						}
						if( OPECTL.CN_QSIG == 5 ){

							for( i = 0; i < COINCHK_CNT ; i++ ){	// ��ݓ��������ۑ�ܰ��Ɏ�M�ް��i�����j�����Z
								cincnt[i] = bcdadd( cincnt[i], CN_RDAT.r_dat08[i] );
							}
							cn_stat( 5, 0 );						// ��������(CREM:ON,������ر)
						}

						break;

					default:
						break;
				}

				break;

			default:						// ���̑�
				break;
		}
		if( ret != 0 ){		// �����e�X�g�I���H

			cn_stat( 8, 0 );				// �����s��(CREM:OFF,������ر)

			Lagtim( OPETCBNO, 6, 5*50 );	// ��ϰ6(5s)�N��(����ݽ����p)
			for( ; ; ){
				msg = GetMessage();			// ү���ގ�M
				if( msg == COIN_EVT )		// �R�C�����b�N�C�x���g
					break;
				// �R�C�����b�N���C���A�E�g�Ď�
				if( msg == TIMEOUT6 ){		// �^�C�}�[�U�^�C���A�E�g�Ď�
					break;
				}

			}
			Lagcan( OPETCBNO, 6 );
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : InchkDsp                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���ү�����ýĉ�ʏ�����ԕ\��                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	InchkDsp( void )
{
	unsigned char	i;						// ٰ�ߏ��������

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		// "���R�C�����b�N�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[1] );		// "�y�����e�X�g�z�@�@�@�@�@�@�@�@"
	Fun_Dsp( FUNMSG[53] );					// "�@���@�@�@�@�N���A�@�@�@ �I�� "

	for( i = 0; i < COINCHK_CNT ; i++ ){	// ��ݓ��������\���i�S�ĂO���j
		grachr( (unsigned short)(i + 2),  0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i] );	// "����"
		grachr( (unsigned short)(i + 2), 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );	// "�F"
		grachr( (unsigned short)(i + 2), 20,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	// "��"
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcInchk2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���ү�����ýď����Q�i�ޑK�����\����ʏ����j             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CmcInchk2( uchar *cincnt )
{
	unsigned short	ret		= 0;			// �߂�l
	unsigned short	dspend	= OFF;			// �ޑK�\����ʏI���׸�
	short			msg = -1;				// ��Mү����
	unsigned char	i;						// ٰ�ߏ��������
	unsigned short	yosub;					// �\/SUB����


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		// "���R�C�����b�N�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[8] );		// "�@�@�@�@�@�@�@�ۗL�����@�\/SUB"
																			// "�@�@�@�P�O�~�F�@�@�@���@�@�@��"
																			// "�@�@�@�T�O�~�F�@�@�@���@�@�@��"
																			// "�@�@�P�O�O�~�F�@�@�@���@�@�@��"
																			// "�@�@�T�O�O�~�F�@�@�@���@�@�@��"
	Fun_Dsp( FUNMSG[12] );													// "�@���@�@�@�@�@�@�@�@�@�@ �I�� "

	for( i = 0; i < COINCHK_CNT ; i++ ){									// �ޑK�����A�\/SUB�����@�\��

		grachr( (unsigned short)(i + 2),  0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT6_0[i] );						// "����"
		grachr( (unsigned short)(i + 2), 12,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );						// "�F"
		opedsp( (unsigned short)(i + 2), 14, (unsigned short)bcdbin(CN_RDAT.r_dat07[i]), 3, 0, 0,
																	COLOR_BLACK, LCD_BLINK_OFF );					// �ۗL����
		grachr( (unsigned short)(i + 2), 20, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );							// "��"

		if( i == 0 ){												// 10�~�H
			yosub = (												// 10�~�����޲�؁[�ϊ�
						( (CN_RDAT.r_dat07[5] & 0x0f) * 100 )	+
						( (CN_RDAT.r_dat07[4] >> 4) * 10 )		+
						  (CN_RDAT.r_dat07[4] & 0x0f)
					);
			opedsp( (unsigned short)(i + 2), 22, yosub, 3, 0, 0 , COLOR_BLACK, LCD_BLINK_OFF );									// �\/SUB 10�~�@�����\��
			grachr( (unsigned short)(i + 2), 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	// "��"
		}
		if( i == 1 ){												// 50�~�H
			yosub = (												// 50�~�����޲�؁[�ϊ�
						( (CN_RDAT.r_dat07[7] & 0x0f) * 100 )	+
						( (CN_RDAT.r_dat07[6] >> 4) * 10 )		+
						  (CN_RDAT.r_dat07[6] & 0x0f)
					);
			opedsp( (unsigned short)(i + 2), 22, yosub, 3, 0, 0 , COLOR_BLACK, LCD_BLINK_OFF );									// �\/SUB 50�~�@�����\��
			grachr( (unsigned short)(i + 2), 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	// "��"
		}
		if( i == 2 ){												// 100�~�H
			yosub = (												// 100�~�����޲�؁[�ϊ�
						( (CN_RDAT.r_dat07[9] & 0x0f) * 100 )	+
						( (CN_RDAT.r_dat07[8] >> 4) * 10 )		+
						  (CN_RDAT.r_dat07[8] & 0x0f)
					);
			opedsp( (unsigned short)(i + 2), 22, yosub, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );									// �\/SUB 100�~�@�����\��
			grachr( (unsigned short)(i + 2), 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );	// "��"
		}
	}

	for( ; dspend == OFF ; ){

		msg = StoF( GetMessage(), 1 );		// ү���ގ�M

		switch( msg){						// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				dspend = ON;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:				// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;
				dspend = ON;

				break;

			case KEY_TEN_F5:				// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;
				dspend = ON;

				break;

			case KEY_TEN_F1:				// �e�P�i��ʐؑցj

				BUZPI();
				dspend = ON;

				break;

			case COIN_EVT:					// �R�C�����b�N�C�x���g

				switch( OPECTL.CN_QSIG ){

					case	1:				// ���ү�����u��������v��M
					case	5:				// ���ү�����u��������imax�B���j�v��M

						for( i = 0; i < COINCHK_CNT ; i++ ){		// �ۗL�����\��
							opedsp( (unsigned short)(i + 2), 14, (unsigned short)bcdbin(CN_RDAT.r_dat07[i]), 3, 0, 0,
																			COLOR_BLACK, LCD_BLINK_OFF );
						}

						if( OPECTL.CN_QSIG == 5 ){
							for( i = 0; i < COINCHK_CNT ; i++ ){	// ��ݓ��������ۑ�ܰ��Ɏ�M�ް��i�����j�����Z
								cincnt[i] = bcdadd( cincnt[i], CN_RDAT.r_dat08[i] );
							}
							cn_stat( 5, 0 );						// ��������(CREM:ON,������ر)
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
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : bcdadd                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : BCD�ް����Z�����i1�޲�BCD+1�޲�BCD��1�޲�BCD�j          |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char data1                                     |*/
/*|              : unsigned char data2                                     |*/
/*| RETURN VALUE : unsigned char                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	bcdadd( uchar data1, uchar data2 )
{
	ushort	wrk1,wrk2;
	uchar	ret;

	wrk1 = (data1 >> 4)*10;		// data1�޲�ؕϊ�
	wrk1 += (data1 & 0x0f);

	wrk2 = (data2 >> 4)*10;		// data2�޲�ؕϊ�
	wrk2 += (data2 & 0x0f);

	wrk1 += wrk2;				// data1+data2
	if( wrk1 > 99 ){
		wrk1 = 99;
	}
	ret = (uchar)( ( (wrk1/10) << 4 ) + ( wrk1%10 ) );	// �v�Z���ʂ�BCD�ɕϊ�

	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcOutchk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���ү����oýď���                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/04                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CmcOutchk( void )
{
	unsigned short	ret		= 0;			// �߂�l
	unsigned short	re_vl	= 0;			// ���ߋ��z
	unsigned short	cn_st	= OFF;			// ���ү����
	short			msg 	= -1;			// ��Mү����

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		// "���R�C�����b�N�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[2] );		// "�y���o���e�X�g�z�@�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[3] );		// "�@���o���z�F�@�@�@�O�~�@�@�@�@"
	grachr( 3, 12,  6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_SPC[0] );	// ���z���͕����R���𔽓]�\���i��߰��j
	Fun_Dsp( FUNMSG[52] );					// "�@�@�@�@�@�@�N���A ���o  �I�� "


	for( ; ; ){

		msg = StoF( GetMessage(), 1 );		// ү���ގ�M

		switch( KEY_TEN0to9( msg ) ){		// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:				// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F3:				// �e�R�i�N���A�j

				if( cn_st != ON ){			// ���o���H�i���o���͖����Ƃ���j

					BUZPI();
					re_vl = 0;								// ���o���z�ر
					grachr( 3, 12, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_SPC[0] );		// ���o���z�𔽓]�\���i��߰��j
				}

				break;


			case KEY_TEN_F4:				// �e�S�i���o�j

				if( cn_st != ON ){			// ���o���H�i���o���͖����Ƃ���j


					if( re_vl != 0 ) {														// ���o���z���O�H
						BUZPI();
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[9] );	// "�@�d�݂𕥏o���ł��@�@�@�@�@�@"
						Fun_Dsp( FUNMSG[8] );												// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
						cn_stat( 2, 0 );													// �����s��
						cn_st = ON;															// ���o���ð�����
					}
					else{
						BUZPIPI();
					}
				}

				break;

			case KEY_TEN:					// �s�d�m�L�[�i�O�`�X�j

				if( cn_st != ON ){			// ���o���H�i���o���͖����Ƃ���j

					BUZPI();

					re_vl = ( (re_vl % 100) * 10 ) + ( msg - KEY_TEN0 );					// ���o���z���

					if( re_vl == 0 ) {														// ���o���z���O�H
						grachr( 3, 12, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_SPC[0] );	// ���o���z�𔽓]�\���i��߰��j
					}
					else{
						opedsp( 3,12, re_vl, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );		// ���o���z�\��
					}
				}

				break;

			case COIN_EVT:					// �R�C�����b�N�C�x���g

				switch( OPECTL.CN_QSIG ){

					case	2:				// ���ү�����u���o�������v��M

						if( cn_st == ON ){														// ���o���H

							cn_st = OFF;														// ���o���ð��ؾ��
							re_vl = 0;															// ���o���z�ر
							grachr( 3, 12, 6, 1, COLOR_BLACK, LCD_BLINK_OFF, DAT3_SPC[0] );	// ���o���z�𔽓]�\���i��߰��j
							displclr( 5 );						// �h���o���h�\���n�e�e
							Fun_Dsp( FUNMSG[52] );				// "�@�@�@�@�@�@�N���A ���o  �I�� "
							LedReq( CN_TRAYLED, LED_OFF );		// ��ݎ�o�����޲��LED����
						}

						break;

					case	7:				// ���ү�����u���o����ԁv��M

						if( cn_st == ON ){						// ���o���H
							LedReq( CN_TRAYLED, LED_ON );		// ��ݎ�o�����޲��LED�_��

							safecl( 0 ); 						/* ���ɓ����O���� */
							refund( (long)(re_vl*10) );			// ���o�J�n
						}

						break;

					default:				// ���̑�
						break;
				}

				break;

			default:						// ���̑�
				break;
		}
		if( ret != 0 ){			// ���o�e�X�g�I���H
			LedReq( CN_TRAYLED, LED_OFF );		// ��ݎ�o�����޲��LED����
			cn_stat( 8, 0 );					// �����s��(�ۗL�������M)
			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcStschk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���ү���Ԋm�F����                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

// ���ү��ُ�ð�������p�ް��ir_dat0c�̵̾�ā�����bit�j
const	unsigned char	cmc_err_chk[10][2] =
	{
		0,	0,			// ��а
		0,	0x08,		// �������ُ�
		0,	0x10,		//  10�~����è����
		0,	0x20,		//  50�~����è����
		0,	0x40,		// 100�~����è����
		0,	0x80,		// 500�~����è����
		1,	0x10,		// �ԋི��
		1,	0x20,		// �d�ݕ��o��
		1,	0x40,		// ���è�����i�ޑK���āj
		1,	0x80,		// ��ٽ����
	};

unsigned short	CmcStschk( void )
{
	unsigned short	ret			= 0;	// �߂�l
	unsigned char	e_page_cnt	= 1;	// �ُ�ð���\���߰�ސ�
	unsigned char	e_page_pos	= 1;	// �ُ�ð���\���߰�ވʒu
	unsigned char	e_cnt		= 0;	// �ُ�ð����
	unsigned char	i;					// ٰ�ߏ��������
	unsigned char	err_no[12];			// �ُ�ð��NO.�i�[�ر
	unsigned char	set_pos;			// �ُ�ð��NO.�i�[�߲��
	short			msg = -1;			// ��Mү����


	memset( err_no, 0, 12 );					// �ُ�ð��NO.�i�[�ر�ر

	dispclr();									// ��ʸر

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );			// "���R�C�����b�N�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[4] );			// "�y��Ԋm�F�z�@�@�@�@�@�@�@�@�@"

	if( cn_errst[0] & 0xc0 ){					// �ʐM�G���[�������H

		// �ʐM�G���[�������̏ꍇ

		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[10] );		// "�@�R�C�����b�N�ʐM�s�ǁ@�@�@�@"
	}
	else{
		// �ʐM�G���[�������łȂ��ꍇ

		set_pos = 0;
		for( i = 1 ; i < 10 ; i++ ){			// �ُ�ð���ް�����
			if( CN_RDAT.r_dat0c[cmc_err_chk[i][0]] & cmc_err_chk[i][1] ){
				// �ُ킠��
				e_cnt++;						// �ُ�ð�����X�V�i+1�j
				err_no[set_pos] = i;			// �ُ�ð��NO.�i�[�ر�ֈُ�ð��NO.���i�[
				set_pos++;						// �i�[�߲���X�V�i+1�j
			}
		}

		if( e_cnt ){							// �ُ킠��H

			// ���ү��ُ킪����ꍇ

			if( e_cnt <= 5 ){					// �ُ�ð��������\���߰�ސ����
				e_page_cnt = 1;
			}
			else if( e_cnt <= 10 ){
				e_page_cnt = 2;
			}
			else{
				e_page_cnt = 3;
			}

			Cmc_err_dsp( e_page_pos, &err_no[0] );	// �ُ�ð���\��

		}else{
			// ���ү��ُ킪�Ȃ��ꍇ

			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[5] );	// "�@����ł��@�@�@�@�@�@�@�@�@�@"
		}
	}

	if( e_page_cnt == 1 ){						// ̧ݸ��ݷ��\��
		// ���ɕ\���y�[�W���Ȃ��ꍇ
		Fun_Dsp( FUNMSG[8] );					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
	}
	else{
		// ���ɕ\���y�[�W������ꍇ
		Fun_Dsp( FUNMSG[6] );					// "�@���@�@���@�@�@�@�@�@�@ �I�� "
	}

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );			// ү���ގ�M

		switch( msg){							// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:					// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:					// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F1:					// �e�P�i���F�O�y�[�W�ؑցj

				if( e_page_cnt != 1 ){

					BUZPI();

					if( e_page_pos == 1 ){
						e_page_pos = e_page_cnt;
					}
					else{
						e_page_pos--;
					}
					Cmc_err_dsp( e_page_pos, &err_no[0] );
				}

				break;

			case KEY_TEN_F2:					// �e�Q�i���F���y�[�W�ؑցj

				if( e_page_cnt != 1 ){

					BUZPI();

					e_page_pos++;
					if( e_page_pos > e_page_cnt ){
						e_page_pos = 1;
					}
					Cmc_err_dsp( e_page_pos, &err_no[0] );
				}

				break;

			default:							// ���̑�
				break;
		}
	}
	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Cmc_err_dsp                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���ү������ُ�ð���\������                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : unsigned char pos�i�\���߰�ވʒu�j                      |*/
/*|              : unsigned char *data�i�ُ�ð��ް���߲���j               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Cmc_err_dsp( unsigned char pos, unsigned char *data )
{
	unsigned char	max;
	unsigned char	ofs;
	unsigned char	no;
	unsigned char	i;

	switch( pos ){		// �߰�ވʒu�H

		case	1:		// �P�y�[�W��
			max = 5;
			ofs = 0;
			break;

		case	2:		// �Q�y�[�W��
			max = 5;
			ofs = 5;
			break;

		case	3:		// �R�y�[�W��
			max = 2;
			ofs = 10;
			break;

		default:		// ���̑��i�y�[�W�G���[�j
			return;
	}
	for( i = 2 ; i <= 6 ; i++ ){	// ײ݂Q�`�U�ر
		displclr( (unsigned short)i );
	}

	for( i = 0 ; i < max ; i++ ){	// �ُ�ð���\���i�w��y�[�W�̍ő�s���A�܂��͕\���ް��I���܂Łj

		no = data[ofs+i];																			// �\������ُ�ð��NO.�擾
		if( no == 0 ){
			break;																					// �\���ް��I��
		}
		grachr( (unsigned short)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMESTR[no-1] );	// �ُ�ð���\��
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CmcVerchk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���ү��ް�ޮ݊m�F����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/07                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	CmcVerchk( void )
{
	unsigned short	ret=0;			// �߂�l
	short			msg = -1;		// ��Mү����

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[0] );		// "���R�C�����b�N�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[6] );		// "�y�o�[�W�����m�F�z�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CMCSTR[7] );		// "�@�o�[�W���� �� �@�@�|�@�@�@�@"

	opedsp( 3, 16, (unsigned short)( bcdbin( CN_RDAT.r_dat0d[0] ) ), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 3, 22, (unsigned short)( bcdbin( CN_RDAT.r_dat0d[1] ) ), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );

	Fun_Dsp( FUNMSG[8] );					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );		// ү���ގ�M

		switch( msg){						// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:				// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			default:						// ���̑�
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Ann                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : �ųݽ��������                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/11                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
// MH810100(S)
//unsigned short	FunChk_Ann( void )
//{
//
//	unsigned short	ret		= 0;			// �߂�l
//	short			msg		= -1;			// ��Mү����
//	unsigned short	req_msg	= 0;			// �m�F�v��ү���ޔԍ�
//	unsigned char	msg_snd	= 0;			// ү���ޑ��M�ς��׸�
//	char	ch	= 0;			// �`�����l���w��
//	unsigned char	pos		= 0;			// ����No(4) or �o�^No(5)
//	unsigned short	entry	= 0;			// �o�^No(1-50)
//	char soundVersion[20];
//	unsigned short cnt;
//	unsigned short cnt2 = 0;
//	ulong	data_cnt = 0;
//	ulong	buf_cnt;
//	ulong	data_len= 0;
//	ulong	read_Addr = 0;
//	ulong	sect_cnt;
//	ushort	chk_sum = 0;
//	ushort	cmp_sum = 0;
//	
//	dispclr();
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "���A�i�E���X�`�F�b�N���@�@�@�@"
//	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[10] );							// "�@�@�@�@�@�`�F�b�N���@�@�@�@�@"
//
//	if(BootInfo.sw.wave == OPERATE_AREA1) {
//		FLT_read_wave_sum_version(0,swdata_write_buf);// �����f�[�^��1�̏������[�h
//		read_Addr = FLT_SOUND0_SECTOR;
//	}
//	else {
//		FLT_read_wave_sum_version(1,swdata_write_buf);// �����f�[�^��2�̏������[�h
//		read_Addr = FLT_SOUND1_SECTOR;
//	}
//	// �����O�X�̎擾
//	data_len = (ulong)(swdata_write_buf[LENGTH_OFFSET]<<24);
//	data_len += (ulong)(swdata_write_buf[LENGTH_OFFSET+1]<<16);
//	data_len += (ulong)(swdata_write_buf[LENGTH_OFFSET+2]<<8);
//	data_len += (ulong)swdata_write_buf[LENGTH_OFFSET+3];
//	// �`�F�b�N�T���̎擾
//	cmp_sum = (ushort)(swdata_write_buf[SUM_OFFSET]<<8);
//	cmp_sum += (ushort)swdata_write_buf[SUM_OFFSET+1];
//	// FROM���特���f�[�^��ǂݏo���T�����Z�o����
//	for(sect_cnt = 0; sect_cnt < FLT_SOUND0_SECTORMAX; sect_cnt++,read_Addr += FLT_SOUND0_SECT_SIZE) {
//		taskchg( IDLETSKNO );
//		FlashReadData_direct(read_Addr, &FLASH_WRITE_BUFFER[0], FLT_SOUND0_SECT_SIZE);// 1�Z�N�^�������[�h
//		for (buf_cnt = 0; buf_cnt < FLT_SOUND0_SECT_SIZE; buf_cnt++) {
//			chk_sum += FLASH_WRITE_BUFFER[buf_cnt];
//			data_cnt++;
//			if(data_len <= data_cnt){
//				goto smchk_loop_end;
//			}
//		}
//	}
//smchk_loop_end:
//	dispclr();
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "���A�i�E���X�`�F�b�N���@�@�@�@"
//	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[0] );							// "�o�[�W����                    "
//	memset(&soundVersion, 0, sizeof(soundVersion));
//	for(cnt=0; cnt<sizeof(SOUND_VERSION); cnt++){
//		if(((SOUND_VERSION[cnt] >= 0x30) && (SOUND_VERSION[cnt] <= 0x39)) || 
//			((SOUND_VERSION[cnt] >= 0x41) && (SOUND_VERSION[cnt] <= 0x5a))){				// 0�`9 �܂��� A�`Z
//			soundVersion[cnt2] = 0x82;
//			soundVersion[cnt2+1] = 0x1f + SOUND_VERSION[cnt];
//			cnt2 += 2;
//		}
//		else{
//			if(SOUND_VERSION[cnt] == 0x20){
//				soundVersion[cnt2] = 0x81;
//				soundVersion[cnt2+1] = 0x40;
//				cnt2 += 2;
//			}
//			else{
//				break;
//			}
//		}
//	}
//	grachr( 2, 11, (unsigned short)strlen(soundVersion), 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char *)&soundVersion);		// ����ROM�o�[�W����
//	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[3] );							// "�@�@�@ �i�@�@�@�@ �@�@�j�@�@�@"
//	opedsp5(3, 9, (ulong)chk_sum, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	if(cmp_sum == chk_sum) {				// SUM�lOK
//		grachr(3, 18, 4, 0 ,COLOR_BLACK, LCD_BLINK_OFF, DAT2_3[0]);
//	}
//	else {									// SUM�lNG
//		grachr(3, 18, 4, 0 ,COLOR_RED, LCD_BLINK_OFF, DAT2_3[1]);
//		BUZPIPI();
//	}
//	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[1] );							// "�@���b�Z�[�WNo.�F�@�@�@�@�@�@ "
//	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[2] );							// "�@�o�^No.      �F�@�@�@�@�@�@ "
//	opedsp( 4, 17, req_msg, 3, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );							// �m�F�v��ү���ޔԍ��\���i���]�\���j
//	opedsp( 5, 19, entry, 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );							// �m�F�v��ү���ޔԍ��\���i���]�\���j
//	Fun_Dsp( FUNMSG[115] );																	// "�@���@�@���@ �ؑ�  ���s  �I�� "
//	
//	memset(avm_test_no, 0, sizeof(avm_test_no));
//	for( ; ; ){
//
//		msg = StoF( GetMessage(), 1 );
//
//		switch( KEY_TEN0to9( msg ) ){		// ��Mү���ށH
//
//			case KEY_MODECHG:				// ����Ӱ�ސؑ�
//
//				BUZPI();
//				ret = MOD_CHG;
//
//				break;
//			case KEY_TEN_F5:				// �e�T�i�I���j
//
//				BUZPI();
//				ret = MOD_EXT;
//
//				break;
//
//			case KEY_TEN_F4:				// �e�S�i���s�j
//				ope_anm( AVM_STOP );			// �ē����b�Z�[�W��~�v��
//				if (pos == 0) {
//					if(req_msg > ANN_MSG_MAX){
//						BUZPIPI();
//					} else {
//						switch(req_msg){
//// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////						case AVM_Edy_OK:
////						case AVM_Edy_NG:
//// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//
//						case AVM_BOO:
//						case AVM_BGM_MSG:
//							ch = 1;
//							break;
//						default:
//							ch = 0;
//							break;
//						}
//						avm_test_no[0] = req_msg;
//						avm_test_ch = ch;
//						avm_test_cnt = 1;
//						ope_anm(AVM_AN_TEST);
//					}
//				} else {
//					if((entry == 0) || (entry > 50)){
//						BUZPIPI();
//					}
//					else{
//						ope_anm(entry+1);
//					}
//				}
//				msg_snd = 1;				// ү���ޑ��M�ς�
//				break;
//			case KEY_TEN_F3:				// �e�R�i�ؑցj
//				BUZPI();
//				if (pos == 0) {		// ����No�N���A
//					req_msg = (req_msg >= 999) ? 0 : (req_msg + 1);
//					opedsp(4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
//				} else {			// �o�^No�N���A
//					entry = (entry >= 50) ? 0 : (entry + 1);
//					opedsp(5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
//				}
//				break;
//			case KEY_TEN_F2:				// �e�Q�i���j
//			case KEY_TEN_F1:				// �e�P�i���j
//				BUZPI();
//				pos ^= 1;
//				if (pos == 0) {
//					opedsp(4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	// ���߇��ԍ��\���i���]�\���j
//					opedsp(5, 19, entry, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �o�^���ԍ��\���i���]�\���j
//				} else {
//					opedsp(5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// �o�^���ԍ��\���i���]�\���j
//					opedsp(4, 17, req_msg, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ���߇��ԍ��\���i���]�\���j
//				}
//				break;
//			case KEY_TEN:					// �s�d�m�L�[�i�O�`�X�j
//
//				BUZPI();
//
//				if( msg_snd == 1 ){			// ү���ޑ��M�ς�?
//					msg_snd = 0;			// ү���ޖ����M
//					if (pos == 0) {
//						req_msg = 0;		// ү���ޔԍ��ر
//					} else {
//						entry = 0;			// �o�^�ԍ��ر
//					}
//				}
//				if (pos == 0) {
//					req_msg = ( (req_msg % 100) * 10 ) + ( msg - KEY_TEN0 );			// ү���ޔԍ����
//					opedsp( 4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// ү���ޔԍ��\���i���]�\���j
//				} else {
//					entry = ( (entry % 10) * 10 ) + ( msg - KEY_TEN0 );				// �o�^�ԍ����
//					opedsp( 5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// �o�^�ԍ��\���i���]�\���j
//				}
//				break;
//			default:						// ���̑�
//				break;
//		}
//		if( ret != 0 ){		// �A�i�E���X�`�F�b�N�I���H
//
//			ope_anm( AVM_STOP );			// �ē����b�Z�[�W��~�v��
//			break;
//		}
//	}
//	return( ret );
//}
//
unsigned short	FunChk_Ann( void )
{

	unsigned short	ret		= 0;			// �߂�l
	short			msg		= -1;			// ��Mү����
// MH810100(S) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
//	unsigned short	req_msg	= 0;			// �m�F�v��ү���ޔԍ�
	unsigned short	req_msg	= 1;			// �m�F�v��ү���ޔԍ�
// MH810100(E) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
	unsigned char	msg_snd	= 0;			// ү���ޑ��M�ς��׸�
	unsigned char	pos		= 0;			// ����No(4) or �o�^No(5)
// MH810100(S) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
//	unsigned short	entry	= 0;			// �o�^No(1-50)
	unsigned short	entry	= 1;			// �o�^No(1-50)
// MH810100(E) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
	char soundVersion[16];
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:547)�Ή�
	uchar soundVersionLen;
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:547)�Ή�
	uchar	startType = 0;					// �����J�n���� ( 0 = ���݂̕����𒆒f���ĕ������J�n����)	
											// ( 1 = �\��)
											// ( 2 = �҂���Ԃ̕������܂߂��ׂĂ̕����I����ɊJ�n����)	

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "���A�i�E���X�`�F�b�N���@�@�@�@"

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "���A�i�E���X�`�F�b�N���@�@�@�@"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[0] );							// "�o�[�W����                    "
	memset(&soundVersion, 0, sizeof(soundVersion));
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:547)�Ή�
//	lcdbm_get_config_audio_ver( soundVersion, sizeof(soundVersion) );
//	grachr( 2, 16, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char *)&soundVersion);		// ����ROM�o�[�W����
	soundVersionLen = lcdbm_get_config_audio_ver( soundVersion, sizeof(soundVersion) );
	grachr( 2, 16, (unsigned short)soundVersionLen, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char *)&soundVersion);		// ����ROM�o�[�W����
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:547)�Ή�
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[1] );							// "�@���b�Z�[�WNo.�F�@�@�@�@�@�@ "
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, ANNSTR[2] );							// "�@�o�^No.      �F�@�@�@�@�@�@ "
	opedsp( 4, 17, req_msg, 3, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );							// �m�F�v��ү���ޔԍ��\���i���]�\���j
	opedsp( 5, 19, entry, 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );							// �m�F�v��ү���ޔԍ��\���i���]�\���j
	Fun_Dsp( FUNMSG[115] );																	// "�@���@�@���@ �ؑ�  ���s  �I�� "
	
	memset(avm_test_no, 0, sizeof(avm_test_no));
	for( ; ret == 0; ){
		msg = StoF( GetMessage(), 1 );
		switch( msg ){		// ��Mү����
			case LCD_DISCONNECT:
				PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
				ret = MOD_CUT;
				break;
			case KEY_MODECHG:				// ����Ӱ�ސؑ�
				PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F5:				// �e�T�i�I���j
				PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
				BUZPI();
				ret = MOD_EXT;
				break;
			case KEY_TEN_F4:				// �e�S�i���s�j
				if (pos == 0 ){
// MH810100(S) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
//					 if(req_msg > ANN_MSG_MAX){
					if (req_msg == 0 || req_msg > ANN_MSG_MAX) {	// 0�̃��b�Z�[�WNo�͑��݂��Ȃ��̂ŃG���[����
// MH810100(E) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
						BUZPIPI();
					 } else {
						PKTcmd_audio_start(startType, ( uchar )pos, ( ushort )req_msg);	// �A�i�E���X�J�n�v��
					}
				}else{
// MH810100(S) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
//					if((entry == 0) || (entry > 50)){
					if ((entry == 0) || (entry > 99)) {	// �o�^No��99�܂�
// MH810100(E) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
						BUZPIPI();
					}
					else{
				 		PKTcmd_audio_start(startType, ( uchar )pos, ( ushort )entry);	// �A�i�E���X�J�n�v��
					}
				}
				msg_snd = 1;				// ү���ޑ��M�ς�
				break;
			case KEY_TEN_F3:				// �e�R�i�ؑցj
				BUZPI();
				if (pos == 0) {		// ����No�N���A
// MH810100(S) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
//					req_msg = (req_msg >= ANN_MSG_MAX) ? 0 : (req_msg + 1);
					req_msg = (req_msg >= ANN_MSG_MAX) ? 1 : (req_msg + 1);
// MH810100(E) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
					opedsp(4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
				} else {			// �o�^No�N���A
// MH810100(S) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
//					entry = (entry >= 50) ? 0 : (entry + 1);
					entry = (entry >= 99) ? 1 : (entry + 1);
// MH810100(E) S.Nishimoto 2020/09/11 �Ԕԃ`�P�b�g���X (#4832 �����e�i���X�̃A�i�E���X�`�F�b�N�Łu���b�Z�[�WNo�v�Ɓu�o�^No�v�ōĐ�����鉹�����t�ɂȂ��Ă���)
					opedsp(5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF);
				}
				break;
			case KEY_TEN_F2:				// �e�Q�i���j
			case KEY_TEN_F1:				// �e�P�i���j
				BUZPI();
				pos ^= 1;
				if (pos == 0) {
					opedsp(4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );	// ���߇��ԍ��\���i���]�\���j
					opedsp(5, 19, entry, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �o�^���ԍ��\���i���]�\���j
				} else {
					opedsp(5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// �o�^���ԍ��\���i���]�\���j
					opedsp(4, 17, req_msg, 3, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ���߇��ԍ��\���i���]�\���j
				}
				break;
			case KEY_TEN0:					// �s�d�m�L�[�i�O�`�X�j
			case KEY_TEN1:					// �s�d�m�L�[�i�O�`�X�j
			case KEY_TEN2:					// �s�d�m�L�[�i�O�`�X�j
			case KEY_TEN3:					// �s�d�m�L�[�i�O�`�X�j
			case KEY_TEN4:					// �s�d�m�L�[�i�O�`�X�j
			case KEY_TEN5:					// �s�d�m�L�[�i�O�`�X�j
			case KEY_TEN6:					// �s�d�m�L�[�i�O�`�X�j
			case KEY_TEN7:					// �s�d�m�L�[�i�O�`�X�j
			case KEY_TEN8:					// �s�d�m�L�[�i�O�`�X�j
			case KEY_TEN9:					// �s�d�m�L�[�i�O�`�X�j
				BUZPI();

				if( msg_snd == 1 ){			// ү���ޑ��M�ς�?
					msg_snd = 0;			// ү���ޖ����M
					if (pos == 0) {
						req_msg = 0;		// ү���ޔԍ��ر
					} else {
						entry = 0;			// �o�^�ԍ��ر
					}
				}
				if (pos == 0) {
					req_msg = ( (req_msg % 100) * 10 ) + ( msg - KEY_TEN0 );			// ү���ޔԍ����
					opedsp( 4, 17, req_msg, 3, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// ү���ޔԍ��\���i���]�\���j
				} else {
					entry = ( (entry % 10) * 10 ) + ( msg - KEY_TEN0 );				// �o�^�ԍ����
					opedsp( 5, 19, entry, 2, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// �o�^�ԍ��\���i���]�\���j
				}
				break;
			default:						// ���̑�
				break;
		}
		if( ret != 0 ){		// �A�i�E���X�`�F�b�N�I���H
			PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
		}
	}
	return( ret );
}
// MH810100(E)

unsigned short	Ann_chk2( void )
{

	unsigned short	ret		= 0;			// �߂�l
	short			msg		= -1;			// ��Mү����
	unsigned long	req_ryo	= 0;			// �m�F�v��ү���ޔԍ�
	unsigned short	req_sya = 0;
	unsigned char	msg_snd	= 0;			// ү���ޑ��M�ς��׸�
	unsigned char	mod = 0;;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[8] );							// "���A�i�E���X�`�F�b�N���@�@�@�@"
	grachr( 2,  0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, "�����ǂݏグ�`�F�b�N" );				// "�@�o�[�W���� �� �@�@�@�@�@�@�@"
	opedpl( 5, 10, req_ryo, 6, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );							// �m�F�v��ү���ޔԍ��\���i���]�\���j
	Fun_Dsp( FUNMSG[81] );																	// "�@�@�@�@�@�@�@�@�@ ���s  �I�� "
	
	memset(avm_test_no, 0, sizeof(avm_test_no));
	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){		// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:				// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;
			case KEY_TEN_F1:				// �e�T�i�I���j
				if(mod){
					mod = 0;
					grachr( 2,  0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, "�����ǂݏグ�`�F�b�N" );			
					displclr(5);
					opedpl( 5, 10, req_ryo, 6, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );							// �m�F�v��ү���ޔԍ��\���i���]�\���j
					req_ryo = 0;
				}
				else{
					mod = 1;
					grachr( 2,  0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, "�Ԏ��ǂݏグ�`�F�b�N" );			
					displclr(5);
					opedsp( 5, 10, req_sya, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// �m�F�v��ү���ޔԍ��\���i���]�\���j
					req_sya = 0;
				}
				break;
			case KEY_TEN_F5:				// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F4:				// �e�S�i���s�j
				BUZPI();
				if(mod == 0){
					announceFee = req_ryo;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					ope_anm(AVM_RYOUKIN );
					PKTcmd_audio_start( 0, ( uchar ) 0, ( ushort )1/*TODO�F�������e*/);	// �A�i�E���X�J�n�v��
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				else{
					key_num = req_sya;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					ope_anm(AVM_SHASHITU);
					PKTcmd_audio_start( 0, ( uchar ) 0, ( ushort )1/*TODO�F�������e*/);	// �A�i�E���X�J�n�v��
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				msg_snd = 1;
				break;


			case KEY_TEN:					// �s�d�m�L�[�i�O�`�X�j

				BUZPI();
				if(msg_snd){
					req_ryo = 0;
					req_sya = 0;
					msg_snd = 0;
				}
				if(mod == 0){
					req_ryo = ( (req_ryo % 1000000) * 10 ) + ( msg - KEY_TEN0 );					// �m�F�v��ү���ޔԍ����
					opedpl( 5, 10, req_ryo, 6, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// �m�F�v��ү���ޔԍ��\���i���]�\���j
				}
				else{
					req_sya = ( (req_sya % 1000) * 10 ) + ( msg - KEY_TEN0 );					// �m�F�v��ү���ޔԍ����
					opedsp( 5, 10, req_sya, 4, 1, 1, COLOR_BLACK, LCD_BLINK_OFF );		// �m�F�v��ү���ޔԍ��\���i���]�\���j
					
				}
				break;

			default:						// ���̑�

				break;
		}
		if( ret != 0 ){		// �A�i�E���X�`�F�b�N�I���H
// MH810100(S) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				ope_anm( AVM_STOP );		// �ē����b�Z�[�W��~�v��
				PKTcmd_audio_end( 0, 0 );	// �A�i�E���X�I���v��
// MH810100(E) Y.Yamauchi 20191209 �Ԕԃ`�P�b�g���X(�����e�i���X)
			announceFee = 0;
			key_num = 0;
			break;
		}
	}
	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| BNA Check                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_BNA( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_BNA( void )
{
	unsigned short	usFbnaEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[0] );		/* // [00]	"���������[�_�[�`�F�b�N���@�@�@" */

		usFbnaEvent = Menu_Slt( FBNAMENU, FBNA_CHK_TBL, (char)FBNA_CHK_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFbnaEvent ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				return( MOD_CHG );
				break;
			case KEY_TEN_F5:
				BUZPI();
				return( MOD_EXT );
				break;

			case FBN1_CHK:
				wopelg( OPLOG_NOTENYUCHK, 0, 0 );		// ���엚��o�^
				usFbnaEvent = BnaInchk();
				break;
			case FBN2_CHK:
				usFbnaEvent = BnaStschk();
				break;
			case FBN3_CHK:
				usFbnaEvent = BnaVerchk();
				break;

			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFbnaEvent;
				break;
			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usFbnaEvent == MOD_CUT ){
		if( usFbnaEvent == MOD_CUT || usFbnaEvent == MOD_CHG ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFbnaEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BnaInchk                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ����ذ�ް����ýď���                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/08                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	BnaInchk( void )
{

	unsigned short	ret	= 0;			// �߂�l
	short			msg	= -1;			// ��Mү����
	unsigned short	rd_cnt	= 0;		// �����Ǎ�����
	unsigned char	rd_sts	= 0;		// ����ذ�ް���
										//  0:��
										//  1:�����ۗ���
										//  2:�����߂������҂�
										//  3:�����捞�����҂�


	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[0] );		// "���������[�_�[�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[1] );		// "�y�����e�X�g�z�@�@�@�@�@�@�@�@"
	Fun_Dsp( FUNMSG[8] );													// "�@�@�@�@�@�@ �@�@�@�@�@�@�I�� "
												
	grachr( 3,  0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT7_2[0] );		// "�P�O�O�O�~�D"
	grachr( 3, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1] );		// "�F"
	opedsp( 3, 16,  0, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );											// "�O"
	grachr( 3, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[2] );		// "��"

	cn_stat( 1, 1 );						// ��������

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );		// ү���ގ�M

		switch( msg ){						// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:				// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			case KEY_TEN_F3:				// �e�R�i�߂��j

				if( rd_sts == 1 ){			// �����ۗ����H

					BUZPI();
					cn_stat( 2, 1 ); 		// �����s��
					rd_sts = 2;				// ��ԁF�����߂������҂�
					Fun_Dsp( FUNMSG[8] );	// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				}

				break;

			case KEY_TEN_F4:				// �e�S�i�捞�j

				if( rd_sts == 1 ){			// �����ۗ����H

					BUZPI();
					cn_stat( 1, 1 ); 		// ��������
					rd_sts = 3;				// ��ԁF�����捞�����҂�
					Fun_Dsp( FUNMSG[8] );	// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				}

				break;

			case NOTE_EVT:					// �������[�_�[�C�x���g

				switch( OPECTL.NT_QSIG ){

					case	1:				// ����ذ�ް����u��������v��M

						rd_cnt++;																// �Ǎ������X�V�i�{�P�j
						opedsp( 3, 16, rd_cnt, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �Ǎ������\��
						grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[6] );		// "�@������ۗ����ł��@�@�@�@�@�@"
						rd_sts = 1;								// ��ԁF�����ۗ���
						Fun_Dsp( FUNMSG[55] );					// "�@�@�@�@�@�@ �߂�  �捞  �I�� "

						break;

					case	2:				// ����ذ�ް����u���o�������v��M

						if( rd_sts == 2 ){		// �����߂������҂��H

							rd_cnt--;															// �Ǎ������X�V�i�|�P�j
							opedsp( 3, 16, rd_cnt, 3, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );	// �Ǎ������\��
							displclr( 5 );														// "�@������ۗ����ł��@�@�@�@�@�@"�\���I�t
							rd_sts = 0;															// ��ԁF��
							cn_stat( 1, 1);														// ��������
						}

						break;

					case	3:				// ����ذ�ް����u���������v��M

						if( rd_sts == 3 ){		// �����捞�����҂��H

							displclr( 5 );						// "�@������ۗ����ł��@�@�@�@�@�@"�\���I�t
							rd_sts = 0;							// ��ԁF��
						}

						break;

					case	7:				// ����ذ�ް����u���o����ԁv��M

						if( rd_sts == 2 ){		// �����߂������҂��H

							cn_stat( 10, 1);
						}

						break;

					default:				// ���̑�
						break;
				}

				break;

			default:						// ���̑�
				break;
		}
		if( ret != 0 ){			// �����e�X�g�I���H

			cn_stat( 2, 1 );	// �����s��

			break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BnaStschk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ����ذ�ް��Ԋm�F����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/08                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

// ����ذ�ް�ُ�ð�������p�ް��ir_dat1c�̵̾�ā�����bit�j
const	unsigned char	bna_err_chk[6][2] =
	{
		0,	0,			// ��а
		0,	0x04,		// ���ʕ��ُ�
		0,	0x08,		// �X�^�b�J�[�ُ�
		0,	0x10,		// �����l�܂�
		0,	0x20,		// �������o���ُ�
		0,	0x80		// �������ɖ��t
	};


unsigned short	BnaStschk( void )
{
	unsigned short	ret			= 0;	// �߂�l
	unsigned char	e_cnt		= 0;	// �ُ�ð����
	unsigned char	i;					// ٰ�ߏ��������
	unsigned char	err_no[5];			// �ُ�ð��NO.�i�[�ر
	unsigned char	set_pos;			// �ُ�ð��NO.�i�[�߲��
	unsigned char	no;					// �ُ�ð��NO.
	short			msg = -1;			// ��Mү����


	memset( err_no, 0, 5 );						// �ُ�ð��NO.�i�[�ر�ر

	dispclr();									// ��ʸر

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[0] );			// "���������[�_�[�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[2] );			// "�y��Ԋm�F�z�@�@�@�@�@�@�@�@�@"

	if( cn_errst[1] & 0xc0 ){					// �ʐM�G���[�������H

		// �ʐM�G���[�������̏ꍇ

		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[7] );		// "�@�������[�_�[�ʐM�s�ǁ@�@�@�@"
	}
	else{
		// �ʐM�G���[�������łȂ��ꍇ

		set_pos = 0;
		for( i = 1 ; i <= 5 ; i++ ){			// �ُ�ð���ް�����
			if( NT_RDAT.r_dat1c[bna_err_chk[i][0]] & bna_err_chk[i][1] ){
				// �ُ킠��
				e_cnt++;						// �ُ�ð�����X�V�i+1�j
				err_no[set_pos] = i;			// �ُ�ð��NO.�i�[�ر�ֈُ�ð��NO.���i�[
				set_pos++;						// �i�[�߲���X�V�i+1�j
			}
		}

		if( e_cnt ){							// �ُ킠��H

			// ����ذ�ް�ُ킪����ꍇ

			for( i = 0 ; i < 5 ; i++ ){			// �ُ�ð���i�ő�T�j��S�ĕ\������

				no = err_no[i];												// �\������ُ�ð��NO.�擾
				if( no == 0 ){
					break;													// �\���ް��I��
				}
				grachr( (unsigned short)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNESTR[no-1] );	// �ُ�ð���\��
			}

		}else{
			// ����ذ�ް�ُ킪�Ȃ��ꍇ

			grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[3] );	// "�@����ł��@�@�@�@�@�@�@�@�@�@"
		}
	}

	Fun_Dsp( FUNMSG[8] );						// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );			// ү���ގ�M

		switch( msg){							// ��Mү����

// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:					// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:					// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			default:							// ���̑�
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BnaVerchk                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ����ذ�ް�ް�ޮ݊m�F����                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/08                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	BnaVerchk( void )
{
	unsigned short	ret=0;			// �߂�l
	short			msg = -1;		// ��Mү����

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[0] );		// "���������[�_�[�`�F�b�N���@�@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[4] );		// "�y�o�[�W�����m�F�z�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, BNASTR[5] );		// "�@�o�[�W���� �� �@�@�|�@�@�@�@"

	opedsp( 3, 16, (unsigned short)( bcdbin( NT_RDAT.r_dat1d[0] ) ), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 3, 22, (unsigned short)( bcdbin( NT_RDAT.r_dat1d[1] ) ), 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );

	Fun_Dsp( FUNMSG[8] );					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );		// ү���ގ�M

		switch( msg){						// ��Mү����
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:				// ����Ӱ�ސؑ�

				BUZPI();
				ret = MOD_CHG;

				break;

			case KEY_TEN_F5:				// �e�T�i�I���j

				BUZPI();
				ret = MOD_EXT;

				break;

			default:						// ���̑�
				break;
		}
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Mck                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ��ذ��������                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/11                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const	unsigned long	mem_chk_adr[3] =
	{
		// �q�`�l�P�͕����I�ɂ͈�ł��邪�A�h���X�f�R�[�h��2�̋�Ԃɕ�������
		0x7000000,		// �q�`�l�P�|�P�FCS1:�A�h���X���0x07000000-0x070FFFFF(1MB)
		0x7400000,		// �q�`�l�P�|�Q�FCS1:�A�h���X���0x07400000-0x074FFFFF(1MB)
		0x6000000,		// �q�`�l�Q    �FCS2:�A�h���X���0x06000000-0x060FFFFF(1MB)
	};
#define		FCK_INSRAM_ADR	0x00000000	// �����r�q�`�l�J�n�A�h���X�F0x00000000�`0x00020000�i0x20000Byte�j
#define		FCK_INSRAM_SIZ	0x20000		// �����r�q�`�l�T�C�Y
#define		FCK_EXSRAM_SIZ	0x100000	// �O���r�q�`�l�T�C�Y
#define		FCK_EXSRAM_CHKSIZ	1024*10	// �O���r�q�`�l�P�ʓ�����̃`�F�b�N�����O�X

// ��ذ�q�vý��ް�
#define		FMCK_DATA1	0xA5
#define		FMCK_DATA2	0x5A

#define 	SEC			50
unsigned short	FunChk_Mck( void )
{
	short	msg;			// ��Mү����
	ushort	ret = 0;		// �߂�l
	ushort	i;				// ٰ�ߏ��������
	uchar	f_flag = 0;		// 0:first/1:not first

	while (1) {
		if (f_flag == 0) {
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[0]);						// line:0
			for (i = 2; i < 6; i++) {															// line:2�`5
				grachr((ushort)i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[i+4]);
			}
			Fun_Dsp(FUNMSG2[47]);					 			// "�@�@�@�@�@�@ ���s  �@�@�@�I�� "
			f_flag = 1;
		}

		msg = StoF(GetMessage(), 1);					// ү���ގ�M
		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			ret = MOD_CUT;
			return(ret);
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:								// ����Ӱ�ސؑ�
			BUZPI();
			ret = MOD_CHG;
			return(ret);
		case KEY_TEN_F5:								// �e�T�i�I���j
			BUZPI();
			ret = MOD_EXT;
			return(ret);
		case KEY_TEN_F3:								// �e�R�i���s�j
			BUZPI();
			Ope_DisableDoorKnobChime();
			ret = MemoryCheck();
			f_flag = 0;
			break;
		default:										// ���̑�
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if (ret == MOD_CHG) {
		if (ret == MOD_CHG || ret == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			return(ret);
		}
	}
	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			MemoryCheck(memory check main function)
//[]----------------------------------------------------------------------[]
///	@param[in]		none	: 
///	@return			ret		: MOD_CHG or MOD_EXT
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/12<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static ushort	MemoryCheck(void)
{
	short	msg;			// ��Mү����
	ushort	ret	= 0;		// �߂�l
	ulong	i;				// ٰ�ߏ��������
	ushort	chk_cnt;		// �����o��
	ushort	ram_no;			// �����Ώ�RAM No.
	uchar	*chk_adr;		// ������ذ���ڽ
	uchar	chk_err;		// ��������
	uchar	sav_data;		// �ް��ޔ�ر
	uchar	chk_data1;		// �����ް��P
	uchar	chk_data2;		// �����ް��Q

	dispclr();
	for (i = 0; i < 4; i++) {																// line:0�`3
		grachr((ushort)i, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[i]);
	}
	grachr(6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, MCKSTR[10]);						// "�`�F�b�N��"���]�\��(line:6)
	Fun_Dsp(FUNMSG[0]);
	xPause( 10 );

	//�����q�`�l����
	fck_chk_err = 0;
	fck_chk_adr = (uchar *)FCK_INSRAM_ADR;
	for (i = 0 ; i < FCK_INSRAM_SIZ ; i++) {						// �����ݸ޽������������s��
		WACDOG;											// WATCHDOG��ϰؾ��
		_di();											// �����݋֎~
		fck_sav_data 	= *fck_chk_adr;					// ������ذ�ް�����
		*fck_chk_adr	= FMCK_DATA1;					// �����ް��Pײ�
		fck_chk_data1	= *fck_chk_adr;					// �����ް��Pذ��
		*fck_chk_adr	= FMCK_DATA2;					// �����ް��Qײ�
		fck_chk_data2	= *fck_chk_adr;					// �����ް��Qذ��
		if ((fck_chk_data1 != FMCK_DATA1) || (fck_chk_data2 != FMCK_DATA2)) {
			// �q�v�����m�f
			fck_chk_err = 1;
		}
		*fck_chk_adr = fck_sav_data;					// ������ذ�ް���ض�ް
		fck_chk_adr++;									// �������ڽ�X�V
		_ei();											// �����݋֎~����
		if (fck_chk_err != 0) {
			// �q�v�����m�f
			break;
		}
	}
	grachr(1, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_3[fck_chk_err]);			// �����q�`�l�������ʕ\��(line:1)
	xPause( 10 );

	//�q�`�l�P�������[�`�F�b�N
	chk_err = 0;
	for (ram_no = 0 ; ram_no <= 1 ; ram_no++) {			// RAM������ذ��������ٰ��
		chk_err = 0;
		chk_adr = (uchar *)mem_chk_adr[ram_no];
		for (chk_cnt = 0 ; chk_cnt < 100 ; chk_cnt++) {	// RAM�i1Mbyte�j��10K�Â�����
			WACDOG;										// WATCHDOG��ϰؾ��
			for (i = 0 ; i < FCK_EXSRAM_CHKSIZ; i++) {	// �����ݸ޽������������s��
				_di();									// �����݋֎~
				sav_data 	= *chk_adr;					// ������ذ�ް�����
				*chk_adr	= FMCK_DATA1;				// �����ް��Pײ�
				chk_data1	= *chk_adr;					// �����ް��Pذ��
				*chk_adr	= FMCK_DATA2;				// �����ް��Qײ�
				chk_data2	= *chk_adr;					// �����ް��Qذ��
				if ((chk_data1 != FMCK_DATA1) || (chk_data2 != FMCK_DATA2)) {
					// �q�v�����m�f
					chk_err = 1;
				}
				*chk_adr = sav_data;					// ������ذ�ް���ض�ް
				chk_adr++;								// �������ڽ�X�V
				_ei();									// �����݋֎~����
				if (chk_err != 0) {
					// �q�v�����m�f
					break;
				}
			}
			if (chk_err != 0) {
				break;
			}
		}
	}
	grachr(2, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_3[chk_err]);				// �q�`�l�������ʕ\��(line:2)
	xPause( 10 );

	//�q�`�l�Q�������[�`�F�b�N
	chk_err = 0;
	chk_adr = (uchar *)mem_chk_adr[2];
	for (chk_cnt = 0 ; chk_cnt < 100 ; chk_cnt++) {	// RAM�i1Mbyte�j��10K�Â�����
		WACDOG;										// WATCHDOG��ϰؾ��
		for (i = 0 ; i < FCK_EXSRAM_CHKSIZ; i++) {	// �����ݸ޽������������s��
			_di();									// �����݋֎~
			sav_data 	= *chk_adr;					// ������ذ�ް�����
			*chk_adr	= FMCK_DATA1;				// �����ް��Pײ�
			chk_data1	= *chk_adr;					// �����ް��Pذ��
			*chk_adr	= FMCK_DATA2;				// �����ް��Qײ�
			chk_data2	= *chk_adr;					// �����ް��Qذ��
			if ((chk_data1 != FMCK_DATA1) || (chk_data2 != FMCK_DATA2)) {
				// �q�v�����m�f
				chk_err = 1;
			}
			*chk_adr = sav_data;					// ������ذ�ް���ض�ް
			chk_adr++;								// �������ڽ�X�V
			_ei();									// �����݋֎~����
			if (chk_err != 0) {
				// �q�v�����m�f
				break;
			}
		}
		if (chk_err != 0) {
			break;
		}
	}
	grachr(3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT2_3[chk_err]);				// �q�`�l�������ʕ\��(line:3)
	displclr(6);											// "�`�F�b�N��"�\���N���A
	xPause( 10 );

	Ope_EnableDoorKnobChime();
	Fun_Dsp(FUNMSG[8]);										// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
	while (ret == 0) {
		msg = StoF(GetMessage(), 1);						// ү���ގ�M
		switch (msg) {										// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:								// ����Ӱ�ސؑ�
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F5:								// �e�T�i�I���j
				BUZPI();
				ret = MOD_EXT;
				break;
			default:										// ���̑�
				break;
		}
	}
	return(ret);
}
/*[]----------------------------------------------------------------------[]*/
/*| IF������                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Ifb( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Ifb( uchar	Car_type )
{
	short	msg = -1;
	short	tno[2], btno[2];
	ushort	top, btop;
	ushort	CNTMAX;
	char	pgchg = 0;
	short	i, j;
	char	mtype;
	char	mod = 0;			// 0:�e�@�y�������No�\�� 1:�q�@�\��
	char	timeout = 1;
	short	tnoIndex[LOCK_IF_MAX];
	char	work;
	ushort	B_CNTMAX;
	const uchar	*title;
	short	terminalCount;

	dispclr();

	Lagtim( OPETCBNO, 6, 12*50 );				// ��ϰ6(12s)�N��(����ݽ����p)

	if( !Car_type ){
		mtype = _MTYPE_LOCK;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[13] );			// [12]	"���h�e�Ճ`�F�b�N���@�@�@�@�@�@"
		title = IFCSTR[1];
	}else{
		mtype = _MTYPE_INT_FLAP;
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[52] );			// [52]	"���b�q�q��`�F�b�N���@�@�@�@"	
		title = IFCSTR[4];
	}
	queset( FLPTCBNO, LK_SND_VER, 1, &mtype );	// ۯ����u�ް�ޮݗv�����M
	Ope_DisableDoorKnobChime();
	grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);		// [10]	"�@�@ ���΂炭���҂������� �@�@"
	Fun_Dsp(FUNMSG[0]);														/* [77]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */

	top =0;
	tno[0] = tno[1] = 0;						// ����ه��ر
	memset( tnoIndex, 0, sizeof( tnoIndex ));
	
	terminalCount = (short)LKcom_GetAccessTarminalCount();
	for (i = 1,CNTMAX=0; i <= terminalCount; i++) {
		work = LKcom_Search_Ifno( (uchar)i );
		if( !Car_type ){
			if( !work ){
				tnoIndex[CNTMAX] = i;
				CNTMAX++;
			}
		}else{
			if( work ){
				tnoIndex[CNTMAX] = i;
				CNTMAX++;
			}
		}
	}
	B_CNTMAX = CNTMAX;

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){			// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );					// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				Lagcan( OPETCBNO, 6 );					// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:	// F5:"�I��"
				BUZPI();
				if( mod == 1 ){							// �q�@���ް�ޮݕ\��?
					BUZPI();
					mod = 0;							// �e�@���ް�ޮݕ\��
					CNTMAX = B_CNTMAX;					// �e�@�̍ő�ڑ���
					top = btop;							// ��ʂ̐擪�̇���߂�
					tno[0] = btno[0];					// ����ه���߂�
					tno[1] = btno[1];					// ����ه���߂�
					grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );		// [00]	"�@XXXX���@�@�@�@�o�[�W�����@�@"
					Fun_Dsp( FUNMSG[6] );				// [06]	"�@���@�@���@�@�@�@       �I�� "
					pgchg = 1;
				}else{
					Lagcan( OPETCBNO, 6 );				// ��ϰ6ؾ��(����ݽ����p)
					return( MOD_EXT );
				}
				break;

			case KEY_TEN_F1:	// F1:"��"
				if( timeout )	break;
				BUZPI();
				tno[1] = tno[0];
				if( tno[0] <= 0 ){
					tno[0] = CNTMAX - 1;
				}else{
					tno[0]--;
				}
				pgchg = pag_ctl( CNTMAX, (ushort)tno[0], &top );
				if( !pgchg ){
					if( top + tno[1] % 5 < CNTMAX ){
						opedsp( (ushort)(tno[1] % 5 + 2), 4, (ushort)(!mod?(tnoIndex[tno[1]]):(tno[1] + 1)), 2, 1, 0,
															COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��
					}
					if( top + tno[0] % 5 < CNTMAX ){
						opedsp( (ushort)(tno[0] % 5 + 2), 4, (ushort)(!mod?(tnoIndex[tno[0]]):(tno[0] + 1)), 2, 1, 1,
															COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��(���])
					}
				}
				break;

			case KEY_TEN_F2:	// F2:"��"
				if( timeout )	break;
				BUZPI();
				tno[1] = tno[0];
				if( tno[0] >= CNTMAX - 1 ){
					tno[0] = 0;
				}else{
					tno[0]++;
				}
				pgchg = pag_ctl( CNTMAX, (ushort)tno[0], &top );
				if( !pgchg ){
					if( top + tno[1] % 5 < CNTMAX ){
						opedsp( (ushort)(tno[1] % 5 + 2), 4, (ushort)(!mod?(tnoIndex[tno[1]]):(tno[1] + 1)), 2, 1, 0,
															COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��
					}
					if( top + tno[0] % 5 < CNTMAX ){
						opedsp( (ushort)(tno[0] % 5 + 2), 4, (ushort)(!mod?(tnoIndex[tno[0]]):(tno[0] + 1)), 2, 1, 1,
															COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��(���])
					}
				}
				break;

			case KEY_TEN_F4:	// F4:"�Ǐo"
				break;
			case TIMEOUT6:		// ��ϰ6��ѱ��(����ݽ����p)
				grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );		// [00]	"�@�e�@���@�@�@�@�o�[�W�����@�@"
					Fun_Dsp( FUNMSG[6] );				// [25]	"�@���@�@���@�@�@�@       �I�� "
				pgchg = 1;
				timeout = 0;
				break;
			default:
				break;
		}
		if( pgchg ){
			dispmlclr( 2, 6 );											// ���ꂩ��ҏW����s���N���A
			for( i = 0, j = 0; j < 5 && (top + i) < CNTMAX; i++ ){
				work = LKcom_Search_Ifno( (uchar)(top + i + 1) );
				if( top + j < CNTMAX ){
					grachr( (ushort)(j + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[2] );		// [02]	"�@���@�@�@�|�@�@�@�@�@�@�@�@�@"
					if( j == tno[0] % 5 ){
						opedsp( (ushort)(j + 2), 4, (ushort)(!mod?(tnoIndex[top + j]):(top + i + 1)), 2, 1, 1,
																	COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��(���])
					}else{
						opedsp( (ushort)(j + 2), 4, (ushort)(!mod?(tnoIndex[top + j]):(top + i + 1)), 2, 1, 0,
																	COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��
					}
					memcpy( wlcd_buf, &OPE_CHR[0], sizeof( wlcd_buf ) );	// ��߰�(0x20)�ر
					if( IFM_LockTable.sSlave[tnoIndex[top + j] - 1].cVersion[0] == 0x00 ){
						if( !Car_type )
							memcpy( wlcd_buf, &IFCSTR[3][12], 18 );		// [����M]�\���p
						else
							memcpy( wlcd_buf, &IFCSTR[5][12], 18 );		// [����M]�\���p							
					}else{
						as1chg((uchar*)IFM_LockTable.sSlave[tnoIndex[top + j] - 1].cVersion, wlcd_buf, (uchar)8 );	// ��M�����ް�ޮ�(����)��\���������ނ֕ϊ�����
					}
					grachr( (ushort)(j + 2), 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, wlcd_buf );		// �ް�ޮݕ\��
					j++;
				}
				pgchg = 0;
			}
			
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| CRR�o�[�W�����`�F�b�N                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_CRRVer( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_CRRVer( void )
{
	short	msg = -1;
	ushort	top;
	ushort	CNTMAX;
	char	pgchg = 0;
	short	i, j;
	char	mtype;
	short	tnoIndex[LOCK_IF_MAX];
	const uchar	*title;

	dispclr();

	Lagtim( OPETCBNO, 6, 12*50 );				// ��ϰ6(12s)�N��(����ݽ����p)

	mtype = _MTYPE_INT_FLAP;
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[55] );			// [55]	"���b�q�q�o�[�W�����`�F�b�N��  "	
	title = IFCSTR[6];

	queset( FLPTCBNO, LK_SND_VER, 1, &mtype );	// ۯ����u�ް�ޮݗv�����M
	Ope_DisableDoorKnobChime();
	grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"�@�@ ���΂炭���҂������� �@�@"
	Fun_Dsp(FUNMSG[0]);														/* [77]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */

	top =0;
	memset( tnoIndex, 0, sizeof( tnoIndex ));
	
	for (i = 0,CNTMAX=0; i < IFS_CRR_MAX; i++) {
		if ( 1 == IFM_LockTable.sSlave_CRR[i].bComm ){	// CRR��̐ݒ肪����Ă��邩���m�F
			tnoIndex[CNTMAX] = i+1;
			CNTMAX++;
		}
	}

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){			// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );					// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				Lagcan( OPETCBNO, 6 );					// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:	// F5:"�I��"
				if( pgchg == 0 ){	// ���s���͏I�������Ȃ�
					break;
				}
				BUZPI();
				Lagcan( OPETCBNO, 6 );				// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:	// F1:"��"
				break;

			case KEY_TEN_F2:	// F2:"��"
				break;

			case KEY_TEN_F4:	// F4:"�Ǐo"
				break;
			case TIMEOUT6:		// ��ϰ6��ѱ��(����ݽ����p)
				grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, title );		// [06]	"�@����@�@�@�@�o�[�W�����@�@"
//					Fun_Dsp( FUNMSG[6] );				// [06]	"�@���@�@���@�@�@�@       �I�� "
					Fun_Dsp( FUNMSG[8] );				// [08]	"�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				pgchg = 1;
				break;
			default:
				break;
		}
		if( pgchg ){
			dispmlclr( 2, 6 );											// ���ꂩ��ҏW����s���N���A
			for( i = 0, j = 0; j < 5 && (top + i) < CNTMAX; i++ ){
				if( top + j < CNTMAX ){
					grachr( (ushort)(j + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[2] );		// [02]	"�@���@�@�@�|�@�@�@�@�@�@�@�@�@"
					opedsp( (ushort)(j + 2), 4, (ushort)tnoIndex[top + j], 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��
					memcpy( wlcd_buf, &OPE_CHR[0], sizeof( wlcd_buf ) );	// ��߰�(0x20)�ر
					if( IFM_LockTable.sSlave_CRR[tnoIndex[top + j] - 1].cVersion[0] == 0x00 ){
						memcpy( wlcd_buf, &IFCSTR[5][12], 18 );		// [����M]�\���p							
					}else{
						as1chg((uchar*)IFM_LockTable.sSlave_CRR[tnoIndex[top + j] - 1].cVersion, wlcd_buf, (uchar)8 );	// ��M�����ް�ޮ�(����)��\���������ނ֕ϊ�����
					}
					grachr( (ushort)(j + 2), 12, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, wlcd_buf );		// �ް�ޮݕ\��
					j++;
				}
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			CRR��`�F�b�N���j���[�\��
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort FunChk_CrrComChk( void )
{
	
	uchar	i,dsp=1;
	ushort	msg;
	short	tnoIndex[IFS_CRR_MAX]={0};
	char	index=0;
	ushort	CNTMAX;
	char	tmp[6]={0};
	
	for ( ; ; ) {

		if( dsp == 1 ){
			/* ������ʕ\�� */
			dispclr();
			grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[8] );		/* "���b�q�q�܂�Ԃ��H��e�X�g���@" */
			grachr( 1, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[6]);			/* "�@CRR����@�@ �o�[�W�����@�@" */
			Fun_Dsp( FUNMSG[68] );						// "�@���@�@���@�@�@�@���s�@ �I�� "

			for (i = 0,CNTMAX=0; i < IFS_CRR_MAX; i++) {
				if ( 1 == IFM_LockTable.sSlave_CRR[i].bComm ){	// CRR��̐ݒ肪����Ă��邩���m�F
					grachr( (ushort)(CNTMAX + 2), 0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[2] );		// [02]	"�@��"
					opedsp( (ushort)(CNTMAX + 2), 4, (ushort)i+1, 2, 1, (index==CNTMAX?1:0) ,COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��(���])
					tnoIndex[CNTMAX] = i+1;
					CNTMAX++;
				}
			}
			if( !CNTMAX ){								// �\���Ώۂ��P����������Ζ߂�
				BUZPIPI();
				return MOD_EXT;
			}
			dsp++;
		}
		
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);	// �|�[�����O�^�C�}�[�ĊJ
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);	// �|�[�����O�^�C�}�[�ĊJ
			return MOD_CHG;

		case KEY_TEN_F5:	/* "�I��" */
			if( dsp == 3 ){	// ���s���͏I�������Ȃ�
				break;
			}
			BUZPI();
			Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);	// �|�[�����O�^�C�}�[�ĊJ
			if( dsp == 4 ){
				dsp = 1;
				break;
			}
			return MOD_EXT;

		case KEY_TEN_F1:	// F1:"��"
			if( dsp > 2 ){
				break;
			}
			BUZPI();
			if( CNTMAX == 1 ){
				break;
			}
			if( index <= 0 ){
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ����ه��\���i���]�����j
				index = (char)(CNTMAX - 1);
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );		// ����ه��\���i���]�j
			}else{
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ����ه��\���i���]�����j
				index--;
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );		// ����ه��\���i���]�j
			}
			break;

		case KEY_TEN_F2:	// F2:"��"
			if( dsp > 2 ){
				break;
			}
			BUZPI();
			if( CNTMAX == 1 ){
				break;
			}
			if( index >= CNTMAX - 1 ){
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ����ه��\���i���]�����j
				index = 0;
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );		// ����ه��\���i���]�j
			}else{
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ����ه��\���i���]�����j
				index++;
				opedsp( (ushort)(index + 2), 4, (ushort)tnoIndex[index], 2, 1, 1 ,COLOR_BLACK, LCD_BLINK_OFF );		// ����ه��\���i���]�j
			}
			break;

		case KEY_TEN_F4:	// F4:"�Ǐo"
			if( dsp > 2 ){
				break;
			}
			BUZPI();
			for( i = 1; i < 8; i++ ){
				displclr( (ushort)i );		// �s���N���A
			}
			grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"�@�@ ���΂炭���҂������� �@�@"
			Fun_Dsp(FUNMSG[0]);														/* [77]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
			
			queset( FLPTCBNO, LK_SND_P_CHK, sizeof(tnoIndex[0]), &tnoIndex[index] );
			// �Ԏ��ݒ���f�t�H���g����P�O�Ԏ��ݒ肵���ꍇ�A���������Ă���ƁA�t���b�v�̌������n�߂�܂łɎ��Ԃ�������i��P�O�O�b�j
			// �ꍇ������̂Ń^�C���A�E�g�܂ł̎��Ԃ��P�Q�O�b�i�Q���j�Ƃ���B
			Lagtim( OPETCBNO, 6, 120*50 );							// ��ϰ6(120s)�N��(����ݽ����p)
			dsp++;
			break;

		case TIMEOUT6:
		case CTRL_PORT_CHK_RECV:	/* ����|�[�g����������M */
			if( dsp < 3 ){												// ��ϰ6��ݾ�
				break;
			}
			Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[9] );						/* ���uNo ����    ���uNo ���� */
			displclr( 3 );												// ���΂炭���҂�������������
			Fun_Dsp( FUNMSG[8] );										// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
			
			for( i=0; i<10; i++ ){
				grachr( (ushort)((i<5?i:i-5)+2), (ushort)(i<5?0:15), 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &IFCSTR[2][2] );			// [02]	"��"
				opedsp( (ushort)((i<5?i:i-5)+2), (ushort)(i<5?2:17), (ushort)(i+1), 2, 1, 0 ,COLOR_BLACK, LCD_BLINK_OFF );		// ����ه��\���i���]�����j
				switch( MntLockTest[i] ){
					case 0x00:
						sprintf( tmp, "%s    ", "�|");
						break;
					case 0x01:
						sprintf( tmp, "%s    ", "��");
						break;
					default:
						sprintf( tmp, "%s(%2X)","�~",MntLockTest[i]);
						break;
					
				}
				grachr( (ushort)((i<5?i:i-5)+2), (ushort)(i<5?7:22), 6, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar*)tmp );		// ���ʕ\��
			}
			toScom_LineTimerStart(IFM_LockTable.toSlave.usLineWatch);	// �|�[�����O�^�C�}�[�ĊJ
			dsp++;
			break;

		default:
			break;
		}
	}
}

// MH810100(S) S.Fujii 2020/07/15 �Ԕԃ`�P�b�g���X(�`�F�b�N�T���\��)
//[]----------------------------------------------------------------------[]
///	@brief		FROM�̃`�F�b�N�T�����v�Z
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static ushort GetFromCheckSum(void)
{
	ushort nReturn = 0;
	ulong	offset = 0;
	ulong	index = 0;
	
	// 4KB���Ƀo�b�t�@�ɏ�������
	for(offset = 0; offset < FROM_PROG_SIZE; offset += FROM_IN_SECTOR_SIZE) {
		memcpy(ProgramRomBuffer, (void*)(FROM_IN_ADDRESS_START + offset), FROM_IN_SECTOR_SIZE);
		
		// sum�l�v�Z
		for(index = 0; index < FROM_IN_SECTOR_SIZE; ++index) {
			nReturn += ProgramRomBuffer[index];
		}

		taskchg( IDLETSKNO );
	}

	return nReturn;
}
// MH810100(E) S.Fujii 2020/07/15 �Ԕԃ`�P�b�g���X(�`�F�b�N�T���\��)

/*[]----------------------------------------------------------------------[]*/
/*| �ް�ޮ�����                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Version( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ushort                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	FunChk_Version( void )
{
	ushort	msg;
	uchar	f_DspFace;		// 0:�o�[�W������ʕ\�����C1:���v�b�o�t�������_���v�\�����C2:�o�[�W�����\����
	unsigned short	usFncEvent;
// MH810100(S) Y.Yamauchi 2019/12/12 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)	
	char	Lcd_ver[16];			// Lcd�ް�ޮ݁i���ع���݁j
	char	sound_ver[16];			// �����ް�ޮ݁i���ع���݁j
// MH810100(S) S.Takahashi 2020/02/20 #3904 �o�[�W�����`�F�b�N�̃f�o�b�O���j���[��\���ł��Ȃ�
	uchar keyIndex = 0;
// MH810100(E) S.Takahashi 2020/02/20 #3904 �o�[�W�����`�F�b�N�̃f�o�b�O���j���[��\���ł��Ȃ�
// MH810100(S) S.Fujii 2020/07/15 �Ԕԃ`�P�b�g���X(�`�F�b�N�T���\��)
	char checksum[16];
	ushort nCheckSum;
// MH810100(E) S.Fujii 2020/07/15 �Ԕԃ`�P�b�g���X(�`�F�b�N�T���\��)

	memset(Lcd_ver,0,sizeof(Lcd_ver));
	memset(sound_ver,0,sizeof(sound_ver));
// MH810100(E) Y.Yamauchi 2019/12/12 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)	
	for ( ; ; ) {
		f_DspFace = 0;

		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[15]);		/* "���o�[�W�����`�F�b�N���@�@�@�@" */
		Fun_Dsp(FUNMSG[0]);						/* "�@�@�@�@�@�@�@ �@�@�@�@  �@�@ " */
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[16]);		/* "���C���v���O�����F    �@�@�@�@" */
		grachr(2, 20,  8, 0, COLOR_BLACK, LCD_BLINK_OFF, VERSNO.ver_part);	// ����ް�ޮݕ\��

		Fun_Dsp(FUNMSG[8]);						/* "�@�@�@�@�@�@�@ �@�@�@�@  �I�� " */

// MH810100(S) S.Fujii 2020/07/15 �Ԕԃ`�P�b�g���X(�`�F�b�N�T���\��)
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[51]);		// "�t�����g�v���O�����F    �@�@�@�@"
//
//		grachr(3, 20,10, 0, COLOR_BLACK, LCD_BLINK_OFF, RXF_VERSION );		// �\�t�g�o�[�W�����\��
//		
//// MH810100(S) Y.Yamauchi 20191212 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)	
////		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[53]);		// "�����f�[�^�@�@�@�F    �@�@�@�@"
////		grachr(4, 20, 9, 0, COLOR_BLACK, LCD_BLINK_OFF, SOUND_VERSION);		// �\�t�g�o�[�W�����\��
//		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[59]);		// 	"LCD���W���[��    :�@�@�@�@�@ "
//		lcdbm_get_config_prgm_ver( Lcd_ver, sizeof(Lcd_ver) );
//		grachr(4, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar *)Lcd_ver);		// 	"LCD���W���[��    :�@�@�@�@�@ "
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[60]);		// "�R���e���c�f�[�^  �F    �@�@�@",
//		lcdbm_get_config_audio_ver( sound_ver, sizeof(sound_ver) );
//		grachr(5, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,(const uchar *)sound_ver);		// �\�t�g�o�[�W�����\��
//// MH810100(E) Y.Yamauchi 20191212 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)	
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[51]);		// "�t�����g�v���O�����F    �@�@�@�@"
		grachr(4, 20,10, 0, COLOR_BLACK, LCD_BLINK_OFF, RXF_VERSION );		// �\�t�g�o�[�W�����\��
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[59]);		// 	"LCD���W���[��    :�@�@�@�@�@ "
		lcdbm_get_config_prgm_ver( Lcd_ver, sizeof(Lcd_ver) );
		grachr(5, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, (const uchar *)Lcd_ver);		// 	"LCD���W���[��    :�@�@�@�@�@ "
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[60]);		// "�R���e���c�f�[�^  �F    �@�@�@",
		lcdbm_get_config_audio_ver( sound_ver, sizeof(sound_ver) );
		grachr(6, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,(const uchar *)sound_ver);		// �\�t�g�o�[�W�����\��
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[62]);		// "�`�F�b�N�T��      �F          ",
		memset(checksum, 0, sizeof(checksum));
		nCheckSum = GetFromCheckSum();
		sprintf(checksum, "0x%04x",nCheckSum);
		grachr(3, 20, 10, 0, COLOR_BLACK, LCD_BLINK_OFF,(const uchar *)checksum);		// �`�F�b�N�T���\��
// MH810100(E) S.Fujii 2020/07/15 �Ԕԃ`�P�b�g���X(�`�F�b�N�T���\��)
		do {
			msg = StoF( GetMessage(), 1 );

			switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				return MOD_CHG;

			case KEY_TEN_F5:	/* "�I��" */
				BUZPI();
				return MOD_EXT;
// MH810100(S) S.Takahashi 2020/02/20 #3904 �o�[�W�����`�F�b�N�̃f�o�b�O���j���[��\���ł��Ȃ�
//			// �B���@�\�Ƃ��āA���vCPU��EEPROM���eDump�@�\������
//			// LCD����F4�L�[�������Ȃ���ALCD����F1�L�[���������ꍇ�̂݋N������
//			case KEY_TEN_F1:
//				if( (TENKEY_F1 == 1) && (TENKEY_F4 == 1) && (f_DspFace == 0) ){	// �ʏ�Ver�\������F4+F1����
			// �B���@�\�Ƃ��ăX�^�b�N�g�p�ʂ̈󎚋@�\������
			// F4->F1->F1->F4�̘A��������s�����ꍇ�̂݋N������
			case KEY_TEN_F1:
				if(keyIndex == 1 || keyIndex == 2) {
					++keyIndex;
				}
				else {
					keyIndex = 0;
				}
				break;
			case KEY_TEN_F4:
				if(keyIndex == 3) {
					keyIndex = 0;
// MH810100(E) S.Takahashi 2020/02/20 #3904 �o�[�W�����`�F�b�N�̃f�o�b�O���j���[��\���ł��Ȃ�
					BUZPI();
					f_DspFace = 1;
					usFncEvent = FucChk_DebugMenu();
					switch(usFncEvent){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					case LCD_DISCONNECT:
						return( MOD_CUT );
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X�i�����e�i���X)
					case MOD_CHG:
						return( MOD_CHG );
					default:
						f_DspFace = 2;
						break;
					}
				}
// MH810100(S) S.Takahashi 2020/02/20 #3904 �o�[�W�����`�F�b�N�̃f�o�b�O���j���[��\���ł��Ȃ�
				else if(keyIndex == 0) {
					++keyIndex;
				}
				else {
					keyIndex = 0;
				}
// MH810100(E) S.Takahashi 2020/02/20 #3904 �o�[�W�����`�F�b�N�̃f�o�b�O���j���[��\���ł��Ȃ�
					break;
// MH810100(S) S.Takahashi 2020/02/20 #3904 �o�[�W�����`�F�b�N�̃f�o�b�O���j���[��\���ł��Ȃ�
			case KEY_TEN0:
			case KEY_TEN1:
			case KEY_TEN2:
			case KEY_TEN3:
			case KEY_TEN4:
			case KEY_TEN5:
			case KEY_TEN6:
			case KEY_TEN7:
			case KEY_TEN8:
			case KEY_TEN9:
			case KEY_TEN_CL:
			case KEY_TEN_F2:
			case KEY_TEN_F3:
				keyIndex = 0;
				break;
// MH810100(E) S.Takahashi 2020/02/20 #3904 �o�[�W�����`�F�b�N�̃f�o�b�O���j���[��\���ł��Ȃ�
			case HIF_RECV_VER:
				/*
				 *	�T�u�b�o�t�̃o�[�W�����擾�C�x���g����Ƀ^�C�}�U�̃^�C���A�E�g�C�x���g�������Ă����ꍇ
				 *	�o�[�W������񂪋󔒂̂܂܂ƂȂ邽�߁A�T�u�b�o�t�̃o�[�W�����擾�C�x���g���L���b�`�����ꍇ��
				 *	�����Ńo�[�W�������́i�āj�\�����s���B
				 */
				FunChk_Draw_SubCPU_Version();
				break;
			default:
				break;
			}
		} while( f_DspFace != 2 );
	 }
}
/**
 *	�T�u�b�o�t�̃o�[�W������`�悷��
 *
 *	@param[in]	version_string	�o�[�W����������ւ̃|�C���^
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2008/09/20(�y)
 */
static void FunChk_Draw_SubCPU_Version(void)
{
	unsigned short	length;
	char	sub_ver[16];

	memset(sub_ver, 0, sizeof(sub_ver));

	displclr(4);
	length = (ushort)strlen(sub_ver);
	if ( 0 < length ) {
		// �����񒷂��P�ȏ�Ȃ�o�[�W������\������
		grachr(3, 18, length, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)sub_ver);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica����                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_Suica( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	FncChk_Suica( void )
{
	ushort	msg;
	ushort	ret;
	char	wk[2];
	char	org[2];

	/* �ݒ����Ұ�-NTNET�ڑ��Q�� */
// MH810103 GG119202(S) �ݒ�Q�ƕύX
//	if( prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1) {
	if (! isSX10_USE()) {
// MH810103 GG119202(E) �ݒ�Q�ƕύX
		BUZPIPI();
		return MOD_EXT;
	}

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	for ( ; ; ) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[3]);			/* "����ʌnIC���[�_�[�`�F�b�N���@" */
		msg = Menu_Slt(SUICA_MENU, FSUICA_CHK_TBL, (char)FSUICA_CHK_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case MOD_CHG:
			return(MOD_CHG);
			break;
		case MOD_EXT:		/* "�I��" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return(MOD_EXT);
			break;
		case FISMF1_CHK:	/* �ʐM�e�X�g 				*/
			ret = Com_Chk_Suica();
			break;
		case FISMF2_CHK:	/* �ʐM���O�v�����g 		*/
			ret = Log_Print_Suica_menu();
			break;
		case FISMF3_CHK:	/* �ʐM���O�v�����g�i�ُ�j */
			ret = Log_Print_Suica(1);
			break;
		case MNT_SCA_MEISAI:					// �r�����������p����
			ret = UsMnt_DiditalCasheUseLog(MNT_SCA_MEISAI);
			break;
		case MNT_SCA_SHUUKEI:					// �r���������W�v
			ret = UsMnt_DiditalCasheSyuukei(MNT_SCA_SHUUKEI);
			break;
		default:
			break;
		}
		if (ret == MOD_CHG) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if ( ret == LCD_DISCONNECT ) {
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(MOD_EXT);
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica�ʐM�`�F�b�N����  		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Com_Chk_Suica( void )         �@                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Date         : 2007-02-26                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort Com_Chk_Suica( void )
{
	ushort	msg;

// MH321801(S) �ʐM�e�X�g�Łu����v�ƕ\�������
	// �N���V�[�P���X�����O�͒ʐM�e�X�g�s�Ƃ���
	if (isEC_USE() &&
		Suica_Rec.Data.BIT.INITIALIZE == 0) {
		BUZPIPI();
		return MOD_EXT;
	}
// MH321801(E) �ʐM�e�X�g�Łu����v�ƕ\�������

	dispclr();
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�\�������C���j
	if( isEC_USE() ){
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[17]);		/* "���ʐM�e�X�g��                " */
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[4]);		/* "�@�ʐM��ԁ@�@   �F           " */
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[18]);		/* "�@�[����ԁ@�@   �F           " */
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[6]);		/* "�@�o�[�W����     �F           " */
	}
	else {
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�\�������C���j
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[3]);			/* "���r�t�h�b�`�`�F�b�N���@�@�@�@" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[4]);			/* "�@�ʐM��ԁ@�@   �F           " */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[5]);			/* "�@�[����ԁ@�@   �F           " */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[6]);			/* "�@�o�[�W����     �F           " */
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i�\�������C���j
	}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i�\�������C���j


	if ( Suica_Rec.suica_err_event.BIT.COMFAIL ) {
		grachr(2, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[2]);		/* "�ُ�" */
		grachr(3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3]);		/* "�|�|" */

	}else{
		grachr(2, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[1]);		/* "����" */
		
		if ( Suica_Rec.suica_err_event.BIT.ERR_RECEIVE )
			grachr(3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[2]);	/* "�ُ�" */
		else
			grachr(3, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[1]);	/* "����" */
		
	}

	if( !Suica_Rec.suica_err_event.BIT.COMFAIL && !Suica_Rec.suica_err_event.BIT.ERR_RECEIVE ){
		opedsp5(4, 20, (ushort)fix_data[0], 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);	/* "�@�@�@�@�@�@�@�@�@�@�@�����@�@" */
		Fun_Dsp(FUNMSG[8]);															/* "�@�@�@�@�@�@�@�@�@ �@�@  �I�� " */
	}

	else {
		grachr(4, 20, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICA_STATUS_STR[3]);		/* "�|�|" */
		Fun_Dsp(FUNMSG[8]);								/* "�@�@�@�@�@�@�@�@�@ �@�@  �I�� " */
	}

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)			
			case KEY_MODECHG:	/* ���[�h�`�F���W */
				return MOD_CHG;
				break;

			case KEY_TEN_F5:	/* ��M�^�C���A�E�g(�ʏ킠�肦�Ȃ�) */
				BUZPI();
				return MOD_EXT;
				break;

			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica���O�v�����g����  		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_Print_Suica( void )       �@                        |*/
/*| PARAMETER    : uchar print_kind 0:�ʏ�LOG 1:�ُ�LOG                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Date         : 2007-02-26                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort Log_Print_Suica( uchar print_kind )
{
	ushort	msg;
	T_FrmLogPriReq1	SuicaLogPriReq;		// �󎚗v��ү����ܰ�
	T_FrmPrnStop	SuicaPrnStop;		// �󎚒��~�v��ү����ܰ�
	ushort			pri_cmd = 0;		// �󎚗v������ފi�[ܰ�
	ushort			log_count = 0;
	uchar			status=0;


	dispclr();
	if( print_kind )
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[14]);		/* "���ʐM���O�v�����g�i�ُ�j���@" */
	else		
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[8]);			/* "���ʐM���O�v�����g���@�@�@�@�@" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[9]);			/* " �ʐM���O��        ������܂� " */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[10]);			/* "�@�@�@�v�����g���܂����H�@�@�@" */

	log_count = Log_Count_search( print_kind );	// ۸ނ�����ۂ̓o�^�������������ľ��

	opedsp(3, 11, log_count, 4, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);			// �����\��

	Fun_Dsp(FUNMSG2[38]);										// ̧ݸ��ݷ��\��

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		if( pri_cmd == 0 ){
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					return MOD_CUT;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)				
				case KEY_MODECHG:								/* ���[�h�`�F���W */
					return MOD_CHG;
					break;

				case KEY_TEN_F1:								

					if( !status ){
						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[11]);		/* "�@�@�@�N���A���܂����H�@�@�@" */
						Fun_Dsp(FUNMSG[19]);					// ̧ݸ��ݷ��\��
						status = 1;								// ��ʽð���̕ύX
						BUZPI();
					}
					break;

				case KEY_TEN_F3:
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i���V�[�g�󎚕s�Ń��O�N���A���ł��Ȃ��C���j
					if (status != 0) {
					// ��ʂ��ر��ʂ̏ꍇ
						BUZPI();
						if (print_kind != 0) {													// �ُ�LOG�󎚂̏ꍇ
							memset(&SUICA_LOG_REC_FOR_ERR, 0, sizeof(struct suica_log_rec));	// �ُ�LOG�ް��̏�����
						} else {
							memset(&SUICA_LOG_REC, 0, sizeof(struct suica_log_rec));			// �ް��̏�����
						}
						return MOD_EXT;
					}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i���V�[�g�󎚕s�Ń��O�N���A���ł��Ȃ��C���j
					if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
						BUZPIPI();
						break;
					}
					BUZPI();
					if( !status ){																// ��ʂ�����ĉ�ʂ̏ꍇ
						SuicaLogPriReq.prn_kind = R_PRI;										// �Ώ�������Fڼ��
						SuicaLogPriReq.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// �@�B��	�F�ݒ��ް�
						SuicaLogPriReq.Kakari_no = OPECTL.Kakari_Num;							// �W��No.
						memcpy( &SuicaLogPriReq.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// �󎚎���	�F���ݎ���
						if( print_kind ){														// �ُ�󎚏����̏ꍇ
							// �ُ�󎚗v�����ǂ����̔��f���v�����^�^�X�N�ōs���ׂɁA�@�BNo�̐擪Bit���g�p���Ĕ��f����B
							// ����A�@�BNo��No.1�`20�܂ł����ݒ肵�Ȃ��ׁA�擪Bit�͎g�p���Ă����Ȃ��Ɣ��f����
							SuicaLogPriReq.Kikai_no |= 0x8000;									// �@�BNo�i�[�ر�̐擪bit���g�p�����׸ނ𗧂Ă�
						}
						queset( PRNTCBNO, PREQ_SUICA_LOG, sizeof(T_FrmLogPriReq1), &SuicaLogPriReq );
						Ope_DisableDoorKnobChime();
						pri_cmd = PREQ_SUICA_LOG;												// ���M����޾���

						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[12]);										/* "�@�@�@�v�����g�� �@�@�@" */
						Fun_Dsp( FUNMSG[82] );													// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
// MH321800(S) D.Inaba IC�N���W�b�g�Ή��i���V�[�g�󎚕s�Ń��O�N���A���ł��Ȃ��C���j					
// 					}else{																		// ��ʂ��ر��ʂ̏ꍇ
// 						if( print_kind ){														// �ُ�LOG�󎚂̏ꍇ
// 							memset( &SUICA_LOG_REC_FOR_ERR,0,sizeof( struct	suica_log_rec ));	// �ُ�LOG�ް��̏�����
// 						}else
// 						memset( &SUICA_LOG_REC,0,sizeof( struct	suica_log_rec ));				// �ް��̏�����
// 						return MOD_EXT;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή��i���V�[�g�󎚕s�Ń��O�N���A���ł��Ȃ��C���j					
					}
					break;

				case KEY_TEN_F4:	
					BUZPI();
					if( status ){
						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[10]);										/* "�@�@�@�v�����g���܂����H�@�@�@" */
						Fun_Dsp(FUNMSG2[38]);													// ̧ݸ��ݷ��\���F"
						status = 0;																// ��ʽð���̕ύX
					}else return MOD_EXT;

					break;

				default:
					break;
			}
		}
		else{
			// �󎚗v����i�󎚏I���҂���ʁj

			if( msg == ( pri_cmd | INNJI_ENDMASK ) ){		// �󎚏I��ү���ގ�M�H
				msg = MOD_EXT;								// YES�F�O��ʂɖ߂�
			}

			switch( msg ){									// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					return MOD_CUT;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
					// �h�A�m�u���ǂ����̃`�F�b�N�����{
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					break;

				case KEY_TEN_F3:							// �e�R�i���~�j�L�[����

					BUZPI();

					/*------	�󎚒��~ү���ޑ��M	-----*/
					SuicaPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &SuicaPrnStop );

					return MOD_EXT;

					break;

				default:
					break;
			}

		}
		if( (msg == MOD_EXT) || (msg == MOD_CHG) ){
			break;											// �O��ʂɖ߂�
		}		
	}
	return MOD_EXT;
}
//[]----------------------------------------------------------------------[]
///	@brief			Suica���O�v�����g���j���[���� 
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/09/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort Log_Print_Suica_menu( void )
{
	ushort	msg;
	ushort	ret;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	for ( ; ; ) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[8]);			/* ���ʐM���O�v�����g�� */
		msg = Menu_Slt(SUICA_MENU2, FSUICA_CHK_TBL2, (char)FSUICA_CHK_MAX2, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return(MOD_CUT);
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)		
		case MOD_CHG:
			return(MOD_CHG);
			break;
		case MOD_EXT:		/* "�I��" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return(MOD_EXT);
			break;
		case MNT_SUICALOG_INJI:		// �ʐM���O�v�����g�i�S���O�j
			ret = Log_Print_Suica(0);
			break;
		case MNT_SUICALOG_INJI2:	// �ʐM���O�v�����g�i���߃��O�j
			ret = Log_Print_Suica2();
			break;
		default:
			break;
		}
		if (ret == MOD_CHG) {
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		if (ret == MOD_CUT ) {
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(MOD_EXT);
}
//[]----------------------------------------------------------------------[]
///	@brief			Suica���O�v�����g����(���߃��O)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/09/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort Log_Print_Suica2( void )
{


	ushort	msg;
	T_FrmLogPriReq1	SuicaLogPriReq;		// �󎚗v��ү����ܰ�
	T_FrmPrnStop	SuicaPrnStop;		// �󎚒��~�v��ү����ܰ�
	ushort			pri_cmd = 0;		// �󎚗v������ފi�[ܰ�

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[8]);			/* ���ʐM���O�v�����g�� */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[15]);			/* " ���߃��O���v�����g���܂����H " */

	Fun_Dsp(FUNMSG[19]);										// ̧ݸ��ݷ��\�� "�@�@�@�@�@�@ �͂� �������@�@�@"

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		if( pri_cmd == 0 ){
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					return MOD_CUT;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:								/* ���[�h�`�F���W */
					return MOD_CHG;
					break;

				case KEY_TEN_F3:	
					if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
						BUZPIPI();
						break;
					}
					BUZPI();
					SuicaLogPriReq.prn_kind = R_PRI;										// �Ώ�������Fڼ��
					SuicaLogPriReq.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// �@�B��	�F�ݒ��ް�
					SuicaLogPriReq.Kakari_no = OPECTL.Kakari_Num;							// �W��No.
					memcpy( &SuicaLogPriReq.PriTime, &CLK_REC, sizeof( date_time_rec ) );	// �󎚎���	�F���ݎ���
					queset( PRNTCBNO, PREQ_SUICA_LOG2, sizeof(T_FrmLogPriReq1), &SuicaLogPriReq );
					Ope_DisableDoorKnobChime();
					pri_cmd = PREQ_SUICA_LOG2;												// ���M����޾���
						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[12]);										/* "�@�@�@�v�����g�� �@�@�@" */
					Fun_Dsp( FUNMSG[82] );													// ̧ݸ��ݷ��\���F"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
					break;

				case KEY_TEN_F4:	
					BUZPI();
					return MOD_EXT;
					break;

				default:
					break;
			}
		}
		else{
			// �󎚗v����i�󎚏I���҂���ʁj

			if( msg == ( pri_cmd | INNJI_ENDMASK ) ){		// �󎚏I��ү���ގ�M�H
				msg = MOD_EXT;								// YES�F�O��ʂɖ߂�
			}

			switch( msg ){									// �C�x���g�H
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:						//�ؒf�ʒm
					return MOD_CUT;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:							// �ݒ�L�[�ؑ�
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
					// �h�A�m�u���ǂ����̃`�F�b�N�����{
					if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					}
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
					break;

				case KEY_TEN_F3:							// �e�R�i���~�j�L�[����

					BUZPI();

					/*------	�󎚒��~ү���ޑ��M	-----*/
					SuicaPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &SuicaPrnStop );

					return MOD_EXT;

					break;

				default:
					break;
			}

		}
		if( (msg == MOD_EXT) || (msg == MOD_CHG) ){
			break;											// �O��ʂɖ߂�
		}		
	}
	return MOD_EXT;


}
/*[]----------------------------------------------------------------------[]*/
/*| �ׯ�ߑ��u����                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Flp( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Flp( void )
{
	short	msg = -1;
	short	tno[2];
	ushort	top;
	char	pgchg = 0;
	char	i;
	char	timeout = 1;
	char	data[8];
	char	rcvSts;
	memset(data,0,sizeof(data));
	rcvSts = 0;

	if( !(GetCarInfoParam() & 0x02) ){
		BUZPIPI();
		return MOD_EXT;
	}

	dispclr();

	Lagtim( OPETCBNO, 6, 12*50 );				// ��ϰ6(12s)�N��(����ݽ����p)

	// �܂��ێ����Ă��邷�ׂĂ��ް�ޮݏ���ر����
	i = _MTYPE_FLAP;
	queset( FLPTCBNO, LK_SND_VER, 1, &i );	// ۯ����u�ް�ޮݗv�����M

	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[44]);			// [44]	"���t���b�vIF�Ճ`�F�b�N���@�@�@"
	grachr(4, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"�@�@ ���΂炭���҂������� �@�@"
	
	top = 0;
	tno[0] = tno[1] = 0;						// ����ه��ر

	for( ; ; ){

		msg = StoF( GetMessage(), 1 );

		switch( msg ){			// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:						// �ؒf�ʒm
				Lagcan( OPETCBNO, 6 );					// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_CUT );
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				Lagcan( OPETCBNO, 6 );					// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_CHG );
				break;

			case KEY_TEN_F5:	// F5:"�I��"
				BUZPI();
				Lagcan( OPETCBNO, 6 );				// ��ϰ6ؾ��(����ݽ����p)
				return( MOD_EXT );
				break;

			case KEY_TEN_F1:	// F1:"��"
				if( timeout )	break;
				BUZPI();
				tno[1] = tno[0];
				if( tno[0] <= 0 ){
					tno[0] = TERM_NO_MAX - 1;
				}else{
					tno[0]--;
				}
				pgchg = pag_ctl( TERM_NO_MAX, (ushort)tno[0], &top );
				if( !pgchg ){
					if( top + tno[1] % 5 < TERM_NO_MAX ){
						opedsp( (ushort)(tno[1] % 5 + 2), 4, (ushort)(tno[1] + 1), 2, 1, 0, 
															COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��
					}
					if( top + tno[0] % 5 < TERM_NO_MAX ){
						opedsp( (ushort)(tno[0] % 5 + 2), 4, (ushort)(tno[0] + 1), 2, 1, 1, 
															COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��(���])
					}
				}
				break;

			case KEY_TEN_F2:	// F2:"��"
				if( timeout )	break;
				BUZPI();
				tno[1] = tno[0];
				if( tno[0] >= TERM_NO_MAX - 1 ){
					tno[0] = 0;
				}else{
					tno[0]++;
				}
				pgchg = pag_ctl( TERM_NO_MAX, (ushort)tno[0], &top );
				if( !pgchg ){
					if( top + tno[1] % 5 < TERM_NO_MAX ){
						opedsp( (ushort)(tno[1] % 5 + 2), 4, (ushort)(tno[1] + 1), 2, 1, 0,
															COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��
					}
					if( top + tno[0] % 5 < TERM_NO_MAX ){
						opedsp( (ushort)(tno[0] % 5 + 2), 4, (ushort)(tno[0] + 1), 2, 1, 1,
															COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��(���])
					}
				}
				break;

			case TIMEOUT6:		// ��ϰ6��ѱ��(����ݽ����p)
				grachr( 1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[4] );		// [04]	"�@����ه� �@�@�@�o�[�W�����@�@"
				Fun_Dsp( FUNMSG[06] );					// [25]	"�@���@�@���@�@�@�@ �Ǐo  �I�� "
				pgchg = 1;
				timeout = 0;
				break;

			default:
				break;
		}
		if( pgchg ){
			for( i = 0; i < 5; i++ ){
				if( top + i < TERM_NO_MAX ){
					grachr( (ushort)(i + 2), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, IFCSTR[2] );		// [02]	"�@���@�@�@�|�@�@�@�@�@�@�@�@�@"
					if( i == tno[0] % 5 ){
						opedsp( (ushort)(i + 2), 4, (ushort)(top + i + 1), 2, 1, 1, 
																	COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��(���])
					}else{
						opedsp( (ushort)(i + 2), 4, (ushort)(top + i + 1), 2, 1, 0,
																	COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��
					}
					if (rcvSts == 0) {
						// �ڑ�����قȂ�
						memcpy( wlcd_buf, &IFCSTR[5][12], 32 );		// [���� ����������]�\���p
					} else if (rcvSts == 2) {
						// ROM�ް�ޮݖ���M
						memcpy( wlcd_buf, &IFCSTR[3][14], 32 );		// [����M]�\���p
					} else {
						as1chg( (uchar*)data, wlcd_buf, 8 );	// ��M�����ް�ޮ�(����)��\���������ނ֕ϊ�����
					}
					grachr( (ushort)(i + 2), 12, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, wlcd_buf );		// �ް�ޮݕ\��
				}else{
					displclr( (ushort)(i + 2) );						// ���g�p�̍s�͸ر
				}
				pgchg = 0;
			}
		}
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_tst_rct                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���V�[�g�v�����^�󎚃e�X�g                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006/06/29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
unsigned short	FPrnChk_tst_rct( void ){

	unsigned short	usFprnRctEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for( ; ; )
	{
		dispclr();

		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[15] );		/* // "�����V�[�g�e�X�g������@�@�@�@" */

		usFprnRctEvent = Menu_Slt( FPRNMENU3, FPRN_CHK_TBL4, (char)FPRN_CHK_MAX3, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( usFprnRctEvent ){
			case FPRN1_CHK:	// ڼ�Ĉ�ý�
				usFprnRctEvent = FPrnChk_tst( usFprnRctEvent );
				break;
			case FPRN8_CHK:	// ڼ��:�̎��؈�ý�
				usFprnRctEvent = FPrnChk_ryo_tst( );
				break;
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return usFprnRctEvent;
				break;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( usFprnRctEvent == MOD_CHG ){
		if( usFprnRctEvent == MOD_CHG || usFprnRctEvent == MOD_CUT ){		// ���[�h�`�F���W�������͐ؒf�ʒm
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return usFprnRctEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_ryo_tst                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ������̎���ýĈ󎚏���                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006/06/29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
#define		TSTRYOPRI_END	(PREQ_RYOUSYUU | INNJI_ENDMASK)

unsigned short	FPrnChk_ryo_tst( void )
{
	short			msg = -1;			// ��Mү����
	T_FrmReceipt	FrmPrintRctTest;	// �󎚗v��ү���ލ쐬�ر
	unsigned short	ret = 0;			// �߂�l
	unsigned char	priend;				// �󎚏I���׸�				��OFF:�󎚒��AON:�󎚏I��
	unsigned char	dsptimer;			// �󎚒���ʕ\����ϰ�׸�	��OFF:��ѱ�čς�	ON:�N����

	dispclr();

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[15] );				// "�����V�[�g�e�X�g������@�@�@�@" 
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[16] );				// "�y�̎��؈���z�@�@�@�@�@�@�@�@"
	grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, PRTSTR[4] );				// "�@������ł��@�@�@�@�@�@�@�@�@" 
	Fun_Dsp(FUNMSG[77]);														/* [77]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */

	memset(&TestData,0,sizeof(TestData));
	FPrnChk_Testdata_mk( 0,&TestData );												// �[�����Z�����f�[�^�Z�b�g
// GG129002(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�V�v�����^�󎚏����s��C���j
	memset(&FrmPrintRctTest, 0, sizeof(FrmPrintRctTest));
// GG129002(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�V�v�����^�󎚏����s��C���j
	FrmPrintRctTest.prn_kind = R_PRI;												// �������ʁFڼ��
	FrmPrintRctTest.reprint = OFF;													// �Ĕ��s�׸�
	FrmPrintRctTest.prn_data = &TestData;
	memcpy( &FrmPrintRctTest.PriTime, &CLK_REC, sizeof(date_time_rec) );			// �Ĕ��s�������e�X�g�󎚓��t�Ƃ��Ďg��

	queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof( T_FrmReceipt ), &FrmPrintRctTest ); 	// �̎��؈󎚗v��
	Ope_DisableDoorKnobChime();

	Lagtim( OPETCBNO, 6, 3*50 );					// �󎚒���ʕ\����ϰ6(3s)�N���iýĈ󎚒��̉�ʕ\�����R�b�ԕێ�����j
	dsptimer	= ON;								// �󎚒���ʕ\����ϰ�׸޾��
	priend		= OFF;								// �󎚏I���׸�ؾ��

	for( ; ; ){
		msg = StoF( GetMessage(), 1 );				// ү���ގ�M
		switch( msg){			// ��Mү���ށH
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:	// �ؒf�ʒm
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	// ����Ӱ�ސؑ�
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
				// �h�A�m�u���ǂ����̃`�F�b�N�����{
				if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				BUZPI();
				ret = MOD_CHG;
// MH810100(S) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				}
// MH810100(E) S.Nishimoto 2020/09/01 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
				break;
			case TSTRYOPRI_END:	// �̎��؈󎚏I��
				if( dsptimer == OFF ){					// �󎚒���ʕ\����ϰ��ѱ�čς݁H
					ret = MOD_EXT;						// YES���O��ʂɖ߂�
				}
				else{
					priend = ON;						// NO�@���󎚏I���׸޾��
				}
				break;
			case TIMEOUT6:		// �󎚒���ʏI����ϰ��ѱ��
				dsptimer = OFF;							// �󎚒���ʕ\����ϰ��ѱ��
				if( priend == ON ){						// �󎚏I���ς݁H
					ret = MOD_EXT;						// YES���O��ʂɖ߂�
				}
				break;
			default:			// ���̑�
				break;
		}
		if( ret != 0 ){									// �󎚒���ʏI���H
			if( dsptimer == ON ){						// �󎚒���ʕ\����ϰ�N�����H
				Lagcan( OPETCBNO, 6 );					// �󎚒���ʕ\����ϰ����
			}
			break;
		}
	}
	return( ret );
}
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FPrnChk_Testdata_mk                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : �������ýėp�[�����Z�����f�[�^�쐬                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : char sw  0=�̎���                                       |*/
/*|                Receipt_data *                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Yanase(COSMO)                                         |*/
/*| Date         : 2006/06/29                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void FPrnChk_Testdata_mk( char sw, Receipt_data *dat )
{
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	int i, cnt;
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)

	//�[�����Z�����f�[�^�쐬
	dat->Kikai_no		= 1;					// �@�B�m���i���ԍ�0,1-20�j
	dat->Oiban.i		= 1L;					// �ǔ�
	dat->Oiban.w		= 1L;					// �ǔ�
	dat->WPlace			= 1L;					// �Ԏ��ԍ�
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	dat->CarSearchFlg	= 0;					// 0=�ԔԌ���
	memset( dat->CarNumber, 0, sizeof(dat->CarNumber) );
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:552)�Ή�
//	strcpy( (char*)dat->CarNumber, "1234" );
	strncpy( (char*)dat->CarNumber, "1234", sizeof(dat->CarNumber) );
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:552)�Ή�
	memset( dat->CarDataID, 0, sizeof(dat->CarDataID) );
	for( i=0, cnt=0; i<sizeof(dat->CarDataID); i++ ){
		dat->CarDataID[i] = (0x30 + cnt);
		if( ++cnt > 9 ){
			cnt = 0;
		}
	}
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	dat->TInTime.Year	= 2000;					// ���ɓ���	�i�N�j
	dat->TInTime.Mon	= 1;					// 			�i���j
	dat->TInTime.Day	= 1;					// 			�i���j
	dat->TInTime.Hour	= 0;					// 			�i���j
	dat->TInTime.Min	= 0;					// 			�i���j
	dat->TOutTime.Year	= 2000;					// �o�ɓ���	�i�N�j
	dat->TOutTime.Mon	= 1;					// 			�i���j
	dat->TOutTime.Day	= 1;					// 			�i���j
	dat->TOutTime.Hour	= 1;					// 			�i���j
	dat->TOutTime.Min	= 0;					// 			�i���j
	dat->WPrice			= 500L;					// ���ԗ���
	dat->syu			= 1;					// ������ʁi�P�`�P�Q�F�`�`�k�j
	dat->testflag		= ON;					// �e�X�g�󎚃t���O
	dat->WInPrice		= 1000L;				// �����z
	dat->Seisan_kind	= 0;					// ���Z��ʁi0=�����A1=�������A2=�蓮�A3=����)
	dat->WTotalPrice	= 500L;					// �̎����z
	dat->Wtax			= 0L;					// �ŋ�
	dat->WChgPrice		= 500L;					// �̎��؂̏ꍇ		�F�ޑK���z
// GG129001(S) �C���{�C�X�Ή��i�ǉ��C���j
	dat->WTaxRate		= Disp_Tax_Value( (date_time_rec*)&CLK_REC );	// �K�p�ŗ��i���ݎ����j
// GG129001(E) �C���{�C�X�Ή��i�ǉ��C���j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
	dat->WBillAmount = Billingcalculation(dat);		// �����z
	dat->WTaxPrice = TaxAmountcalculation(dat, 0);	// �ېőΏۊz
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j

}

enum {
	CCOMCHK_TYPE_CLR = 0,
	CCOMCHK_TYPE_SND,
};

#define	_getid(t)	((t == CCOMCHK_TYPE_CLR)? 62 : 61)

void IBKCtrl_SetRcvData_manu(uchar id, uchar rslt)
{
	switch (id) {

	case 61:
	case 62:
		if (rslt) 	
			queset( OPETCBNO, IBK_CTRL_NG, 0, NULL );
		else
			queset( OPETCBNO, IBK_CTRL_OK, 0, NULL );
		break;

	default:
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N����                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CCom( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_CCom( void )
{
	ushort	fncChkCcomEvent;
	ushort	ret;
	char	wk[2];

	// �Z���^�[�ʐM�L���H
	if (!_is_ntnet_remote() && prm_get(COM_PRM, S_PAY, 24, 1, 2) != 2) {		//	Ntnet_Remote_Comm
		BUZPIPI();
		return MOD_EXT;
	}

	if(_is_ntnet_remote()) {
	if( (prm_get( COM_PRM, S_CEN, 65, 3, 4 ) == 0) &&
		(prm_get( COM_PRM, S_CEN, 65, 3, 1 ) == 0) &&
		(prm_get( COM_PRM, S_CEN, 66, 3, 4 ) == 0) &&
		(prm_get( COM_PRM, S_CEN, 66, 3, 1 ) == 0) &&
		(prm_get( COM_PRM, S_CEN, 51, 1, 3 ) == 0) ) {
		BUZPIPI();
		return MOD_EXT;
	}
	}

	// �������M�ꎞ��~
	ntautoStopReq();
	
	DP_CP[0] = DP_CP[1] = 0;
	fncChkCcomEvent = 0;
	ret             = 0;

	while ((fncChkCcomEvent != MOD_EXT) && (ret != MOD_CHG)) {
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[45] );		/* "���Z���^�[�ʐM�`�F�b�N��" */
		fncChkCcomEvent = Menu_Slt( CCOMMENU, CCOM_CHK_TBL, CCOM_CHK_MAX, 1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( fncChkCcomEvent ){

		case CCOM_DTCLR:	// �����M�f�[�^�N���A
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			ret = FncChk_bufctrl(CCOMCHK_TYPE_CLR);
			break;

		case CCOM_FLSH:		// �����M�f�[�^���M
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			ret = FncChk_bufctrl(CCOMCHK_TYPE_SND);
			break;

//���g�p�ł��B�iS�j			
		case CCOM_TEST:		// �e�n�l�`�ʐM�e�X�g
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1 &&				// LAN�ȊO
				prm_get( COM_PRM,S_NTN,121,1,1) != 0) {				// ���ԏ�Z���^�[�`��
				ret = FncChk_ccomtst();								// �ʐM�e�X�g
			}
			else {													// LAN�̏ꍇ�͒ʐM�e�X�g�s��
				BUZPIPI();
			}
			break;
		case CCOM_APN:		// �ڑ���`�o�m�m�F
			if(prm_get( COM_PRM,S_CEN,51,1,3 ) == 1){
				BUZPIPI();
				break;
			}
			if( (prm_get( COM_PRM,S_CEN,78,1,1 ) == 1) ||
				(prm_get( COM_PRM,S_CEN,78,1,1 ) == 2) ) {			/* FOMA�Ή�?(Y)				*/
				ret = FncChk_CcomApnDisp();
			}else{
				BUZPIPI();
			}
			break;
		case CCOM_ATN:		// �d�g��M��Ԋm�F
			if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1) {				// LAN�ȊO
				ret = FncChk_CcomAntDisp();
			}else{
				BUZPIPI();
			}
			break;
//���g�p�ł��B�iE�j			

		case CCOM_OIBANCLR:	// �Z���^�[�ǂ��ԃN���A
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
				ret = FncChk_cOibanClr();
			}else{
				BUZPIPI();
			}
			break;
		case CCOM_REMOTE_DL:
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			ret = FncChk_CComRemoteDLChk();
			break;
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
		case CCOM_LONG_PARK_ALLREL:
			if(!_is_ntnet_remote()) {
				BUZPIPI();
				break;
			}
			if(prm_get(COM_PRM, S_TYP, 135, 1, 5) != 2){	// �������Ԍ��o(�������ԃf�[�^�ɂ��ʒm)�ȊO
				BUZPIPI();
				break;
			}
			ret = FncChk_CComLongParkAllRel();
			break;
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)

		case MOD_EXT:
			break;

		default:
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( fncChkCcomEvent == MOD_CHG ){
		if( fncChkCcomEvent == MOD_CHG || fncChkCcomEvent == MOD_CUT ){		//  ���[�h�`�F���W�������͐ؒf�ʒm
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			OPECTL.Mnt_lev = (char)-1;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			if ( fncChkCcomEvent == MOD_CHG ) {
				ret = MOD_CHG;
			} else {
				ret = MOD_CUT;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)			
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	
	// �������M�ĊJ
	ntautoStartReq();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@�o�b�t�@����                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_bufctrl( int type )                              |*/
/*| PARAMETER    : type : 0: �N���A                                        |*/
/*|                       1: ���M                                          |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
static const ulong	req_flg[CCOM_DATA_MAX] = {
// MH810100(S) Y.Yamauchi 20191028 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	NTNET_BUFCTRL_REQ_INCAR,			// ����
// MH810100(E) Y.Yamauchi 20191028 �Ԕԃ`�P�b�g���X(�����e�i���X)
	NTNET_BUFCTRL_REQ_TTOTAL,			// �W�v
	NTNET_BUFCTRL_REQ_SALE,				// ���Z
	NTNET_BUFCTRL_REQ_COIN,				// �R���Ɍv
	NTNET_BUFCTRL_REQ_ERROR,			// �װ
	NTNET_BUFCTRL_REQ_NOTE,				// �����Ɍv
	NTNET_BUFCTRL_REQ_ALARM,			// �װ�
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	NTNET_BUFCTRL_REQ_CAR_COUNT,		// ���ԑ䐔
//	NTNET_BUFCTRL_REQ_MONITOR,			// ����
//	NTNET_BUFCTRL_REQ_MONEY, 			// ���K�Ǘ�
//	NTNET_BUFCTRL_REQ_OPE_MONITOR,		// ����
//	NTNET_BUFCTRL_REQ_TURI, 			// �ޑK�Ǘ�
//	NTNET_BUFCTRL_REQ_RMON,				// ���u�Ď�
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	NTNET_BUFCTRL_REQ_LONG_PARK,		// ��������
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	NTNET_BUFCTRL_REQ_MONEY, 			// ���K�Ǘ�
	NTNET_BUFCTRL_REQ_MONITOR,			// ����
	NTNET_BUFCTRL_REQ_TURI, 			// �ޑK�Ǘ�
	NTNET_BUFCTRL_REQ_OPE_MONITOR,		// ����
	NTNET_BUFCTRL_REQ_RMON,				// ���u�Ď�
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
};
static const ulong	MntTransData[CCOM_DATA_MAX][2] = {
// MH810100(S) Y.Yamauchi 20191028 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	{61, 6},		// ����
// MH810100(E) Y.Yamauchi 20191028 �Ԕԃ`�P�b�g���X(�����e�i���X)
	{61, 4},		// �W�v
	{61, 5},		// ���Z
	{62, 5},		// ��݋��ɏW�v	(ram�̂�)
	{61, 3},		// �װ
	{62, 4},		// �������ɏW�v	(ram�̂�)
	{61, 2},		// �װ�
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	{62, 3},		// ���ԑ䐔�ް�
//	{61, 1},		// ���
//	{62, 1},		// ���K�Ǘ�
//	{62, 6},		// �������
//	{62, 2},		// �ޑK�Ǘ�
//	{61, 3},		// ���u�Ď�(�Q�Ƃ���p�����[�^���Ȃ��̂ŃG���[�f�[�^�̃p�����[�^�Ɠ�������Ƃ���)
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	{61, 3},		// ��������(�Q�Ƃ���p�����[�^���Ȃ��̂ŃG���[�f�[�^�̃p�����[�^�����̎Q�ƈʒu�Ƃ���)
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	{62, 1},		// ���K�Ǘ�
	{61, 1},		// ���
	{62, 2},		// �ޑK�Ǘ�
	{62, 6},		// �������
	{61, 3},		// ���u�Ď�(�Q�Ƃ���p�����[�^���Ȃ��̂ŃG���[�f�[�^�̃p�����[�^�Ɠ�������Ƃ���)
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
};

ushort FncChk_bufctrl( int type )
{
	ushort	msg;
	uchar	mode;		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
	uchar	dsp;		// 0:�`��Ȃ�/ 1:�����X�V/ 2:��ʍX�V/ 3:�����\��
	int		item_pos;	// �J�[�\���ʒu
	ushort	pos;
	uchar	countreq = 0;
	uchar	ibkdown = 0;
	uchar	fkey_enb = 0;
	uchar	page = 0;
	uchar	save_page;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[type] );	// "���Z���^�[�����M�f�[�^�N���A��"
																		// "���Z���^�[�����M�f�[�^���M���@"
	dsp      = 2;
	mode     = 0;
	item_pos = 0;

	for ( ; ; ) {
		if (dsp) {
		// �����\���X�V�v������
			if (mode == 0) {
				if (dsp == 3) {
				// �����\��
					dsp_unflushed_count(ibkdown, page);		// ������\��
					dsp_ccomchk_cursor(item_pos, 1, page);	// ���]�\��
					Fun_Dsp(FUNMSG[(92+type)]);			// " �ر�@�S�ر �@���@       �I�� "
														// " ���M �S���M�@���@ �@�@  �I�� "
					fkey_enb = 1;						// �Ȍ�F1&F2����
					Lagtim( OPETCBNO, 6, 2*50 );		// ��ʍX�V��ϰ6(2s)�N��
				}
				else {
				// �����v��
					if (dsp == 2) {
						clr_dsp_count(page);			// �\���N���A
						fkey_enb = 0;
					}
					Lagtim( OPETCBNO, 2, 200/20 );		// �����\����ϰ2(200ms)�N�� = TIMEOUT2
					countreq = 1;						// �����v����
				}
			}
			dsp = 0;
		}
		
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			Lagcan(OPETCBNO, 6);
			Lagcan(OPETCBNO, 7);
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)	
		case KEY_MODECHG:
			Lagcan(OPETCBNO, 6);
			Lagcan(OPETCBNO, 7);
			return MOD_CHG;

		case KEY_TEN_F5:			// �u�I���v
			if (mode == 0) {		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				BUZPI();
				Lagcan(OPETCBNO, 6);
				Lagcan(OPETCBNO, 7);
				return MOD_EXT;
			}
			else if (mode == 3) {	// �G���[���
				BUZPI();
				dsp  = 2;
				mode = 0;
			}
			break;

		case KEY_TEN_F1:		// �N���A/���M
			if (mode == 0) {
				if (fkey_enb == 0){
					break;
				}
				BUZPI();
				
				dispmlclr(1, 6);
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//				pos = (!(item_pos % 2) || (item_pos == CCOM_DATA_TOTAL)) ? 4 : 8;
				pos = ((item_pos % 2) || (item_pos == CCOM_DATA_TOTAL) || (item_pos == CCOM_DATA_RMON)) ? 4 : 8;
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
				grachr( 2,    2,  pos, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[item_pos] );				// "�I������"
				grachr( 2, (ushort)(pos+2), 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[type] );		// "�f�[�^���������܂�"
																										//          ���M
				grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
				Fun_Dsp(FUNMSG[19]);																	// "�@�@�@�@�@�@ �͂� �������@�@�@"
				mode = 1;		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
			}
			break;
		case KEY_TEN_F2:		// �S�N���A/�S���M
			if (mode == 0) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				if (fkey_enb == 0){
					break;
				}
				BUZPI();
				
				dispmlclr(1, 6);
				grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX] );		// "�S"
				grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[type] );				// "�f�[�^���������܂�"
																								//          ���M
				grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );					// "�@�@�@�@��낵���ł����H�@�@�@"
				Fun_Dsp(FUNMSG[19]);								// "�@�@�@�@�@�@ �͂� �������@�@�@"
				mode = 2;		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
			}
			break;

		case KEY_TEN_F3:
			if (ibkdown) {
			// IBK�_�E�����Ȃ��IBK�ɉ��u�w�����o���Ȃ�
				if (mode == 1 || mode == 2) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				// �����M�f�[�^�N���A�v����IBK_CTRL_OK�����ցi�{�̃f�[�^���N���A�j
				// �����M�f�[�^���M�v���@��IBK_CTRL_NG�����ցi�G���[�Ƃ���j
					BUZPI();
					queset(OPETCBNO, (type == CCOMCHK_TYPE_CLR)? IBK_CTRL_OK : IBK_CTRL_NG, 0, NULL);
					break;
				}
			}
			switch (mode) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
			case 0:			// �J�[�\���ړ�
				BUZPI();
				dsp_ccomchk_cursor(item_pos, 0, page);	// ���]�\��
				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//					if (--item_pos < CCOM_DATA_INCAR) {
					if (--item_pos < CCOM_DATA_TOTAL) {
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
						item_pos = CCOM_DATA_MAX - 1;
					}
				}
				else {
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//					if (--item_pos < CCOM_DATA_INCAR) {
					if (--item_pos < CCOM_DATA_TOTAL) {
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
						item_pos = CCOM_EXIST_DATA_MAX - 1;
					}
				}
				save_page = page;
				page = dsp_ccomchk_cursor(item_pos, 1, save_page);	// ���]�\��
				if (page != save_page) {
					fkey_enb = 0;
					Lagtim( OPETCBNO, 2, 200/20 );		// �����\����ϰ2(200ms)�N�� = TIMEOUT2
					countreq = 1;						// �����v����
				}
				break;
			case 1:			// �N���A�E���M���s
				BUZPI();
				NTNET_Snd_DataTblCtrl(_getid(type), req_flg[item_pos], 0);
				Lagtim( OPETCBNO, 7, 5*50 );			// ��ѱ�ėp��ϰ7(5s)�N��
				break;
			case 2:			// �S�N���A�E�S���M���s
				BUZPI();
 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
					NTNET_Snd_DataTblCtrl(_getid(type), NTNET_BUFCTRL_REQ_ALL, 0);
				}
				else {
					NTNET_Snd_DataTblCtrl(_getid(type), NTNET_BUFCTRL_REQ_ALL_PHASE1, 0);
				}
				Lagtim( OPETCBNO, 7, 5*50 );			// ��ѱ�ėp��ϰ7(5s)�N��
				break;
			case 3:
			default:
				break;
			}
			break;

		case KEY_TEN_F4:						// �u�������v
			if ((mode == 1) || (mode == 2)) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				BUZPI();
				dsp  = 2;	// ��ʍĕ`��
				mode = 0;
			}
			else if (mode == 0) {
				BUZPI();
				dsp_ccomchk_cursor(item_pos, 0, page);	// ���]�\��
				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
					if (++item_pos >= CCOM_DATA_MAX) {
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//						item_pos = CCOM_DATA_INCAR;
						item_pos = CCOM_DATA_TOTAL;
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
					}
				}
				else {
					if (++item_pos >= CCOM_EXIST_DATA_MAX) {
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//						item_pos = CCOM_DATA_INCAR;
						item_pos = CCOM_DATA_TOTAL;
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
					}
				}
				save_page = page;
				page = dsp_ccomchk_cursor(item_pos, 1, save_page);	// ���]�\��
				if (page != save_page) {
					fkey_enb = 0;
					Lagtim( OPETCBNO, 2, 200/20 );		// �����\����ϰ2(200ms)�N�� = TIMEOUT2
					countreq = 1;						// �����v����
				}
			}
			break;

		case IBK_CTRL_NG:	// �w�����MNG
			Lagcan(OPETCBNO, 7);	// ��ѱ�ĊĎ���ݾ�
		case TIMEOUT7:		// ������ѱ��
			if (mode) {
				BUZPIPI();
				displclr(4);
				grachr( 6,  0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[3] );		// "�@�@�@���ݎ��s�ł��܂���@�@�@"
				Fun_Dsp(FUNMSG[8]);													// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				mode = 3;
			}
			break;
		case IBK_CTRL_OK:	// �w�����MOK
			if (mode) {		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				Lagcan(OPETCBNO, 7);	// ��ѱ�ĊĎ���ݾ�
				dsp  = 2;				// ��ʍĕ`��
				mode = 0;
			}
			break;

		case TIMEOUT6:							// ��ʍX�V��ϰ��ѱ��
			if (countreq == 0) {
				dsp = 1;						// �����Ď擾
			}
			else {
				countreq = 0;
				ibkdown = 1;
				dsp = 3;						// �����ĕ`��
			}
			break;

		case TIMEOUT2:							// �����\����ϰ2(200ms)�N��
		case IBK_COUNT_UPDATE:
			if (countreq) {
				countreq = 0;
				Lagcan(OPETCBNO, 6);			// ��ѱ�ĊĎ���ݾ�
			}
			ibkdown = 0;
			dsp = 3;							// �����ĕ`��
			break;
			
		default:
			break;
		}
	}
}

#define DATA_LINE_MAX	6
#define PAGE_ITEM_MAX	(DATA_LINE_MAX * 2)
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//static const uchar st_offset[CCOM_DATA_MAX] = { 14,2,1,3,5,4,6,16,8,7,9,18,19 };
//NOTE:st_offset���̐�����t_NtBufCount�̃C���f�b�N�X�ł�
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//static const uchar st_offset[CCOM_DATA_MAX] = { 14,2,1,3,5,4,6,16,8,7,9,18,19,20 };
static const uchar st_offset[CCOM_DATA_MAX] = {  2, 1, 3, 5, 4, 6, 7, 8,18, 9,19 };
static const uchar st_num_dg[CCOM_DATA_MAX] = {  4, 4, 8, 3, 8, 4, 8, 4, 8, 4, 4 };	// st_offset[]�ɑΉ�����\����������(���p)���`����
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)

// ������������
void	clr_dsp_count(uchar page)
{
	ushort i, j;
	ushort offset = page * PAGE_ITEM_MAX;

	memset(PreCount, 0xFF, sizeof(ulong)*CCOM_DATA_MAX);
	dispmlclr(1, 6);
	for (i=1, j=offset; (i<=DATA_LINE_MAX) && (j<CCOM_DATA_MAX); i++) {
// MH810100(S) Y.Yamauchi 2019/10/31 �Ԕԃ`�P�b�g���X(�����e�i���X)	
//		grachr( i,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[j++] );
//		grachr( i, 11, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX+1] );	// "��"
		grachr( i,  0, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[j++] );
		grachr( i, 12, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX+1] );	// "��"
// MH810100(E) Y.Yamauchi 2019/10/31 �Ԕԃ`�P�b�g���X(�����e�i���X)	
		
		if ((i <= DATA_LINE_MAX) && (j < CCOM_DATA_MAX)) {
			if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
				if( (i == DATA_LINE_MAX) || (page > 0) ) {
					break;
				}
			}
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//			grachr( i, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[j++] );
			grachr( i, 16, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[j++] );
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			grachr( i, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX+1] );	// "��"
		}
	}
	Fun_Dsp(FUNMSG[8]);		// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
}

// �����M�f�[�^�����擾���ĕ\������
ushort dsp_unflushed_count(uchar type, uchar page)
{
	ushort i, j;
	ushort size;
	ulong  num;
	ulong  *pt1, *pt2;
	t_NtBufCount buf;
	char  numStr[10];
	ushort offset = page * PAGE_ITEM_MAX;
	ushort k;
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	ushort col, cnt;
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	NTBUF_GetBufCount(&buf);
	NTBUF_GetBufCount(&buf, TRUE);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
	pt1 = (ulong*)&buf;
	if (type == 0) {
	// ���v�\��
		pt2 = (ulong*)&IBK_BufCount;
		size = sizeof(t_NtBufCount)/sizeof(ulong);
		for (i = 0; i < size; i++) {
			*pt1++ += *pt2++;
		}
		pt1 -= size;
	}

	// �����\��
	for (i = offset, k = 0; (i < CCOM_DATA_MAX) && (k < PAGE_ITEM_MAX); i++, k++) {
		if( prm_get(COM_PRM,S_NTN,121,1,1) == 0 ) {
			if( i == CCOM_EXIST_DATA_MAX ) {
				break;
			}
		}
		if(0 == prm_get(COM_PRM, S_NTN, MntTransData[i][0], 1, MntTransData[i][1])) {
// MH322917(S) A.Iiizumi 2018/09/21 ���u����`�F�b�N���P
//			if(i == 1 && 0 == prm_get(COM_PRM, S_NTN, 26, 1, 3)) {	// �W�v��34-0026�C=0(���M���Ȃ��j�Ȃ�O���Ƃ���
			if((i == NTNET_BUFCTRL_REQ_TTOTAL) && (0 == prm_get(COM_PRM, S_NTN, 26, 1, 3))) {	// �W�v��34-0026�C=0(���M���Ȃ��j�Ȃ�O���Ƃ���
// MH322917(E) A.Iiizumi 2018/09/21 ���u����`�F�b�N���P
				num = 0;
			}
			else {
				num = *(pt1+st_offset[i]);
			}
		}
		else {
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)+���u����`�F�b�N���P
//			num = 0;
			// ���u�Ď��ƒ������Ԃ͑��M�}�X�N�͂��Ȃ��̂ŏ�ɗL��
			if((i == NTNET_BUFCTRL_REQ_RMON)||(i == NTNET_BUFCTRL_REQ_LONG_PARK)){
				num = *(pt1+st_offset[i]);
			}else{
				num = 0;
			}
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)+���u����`�F�b�N���P
		}
		
		if (num != PreCount[i]) {
			memset(numStr, 0x00, sizeof(numStr));
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//			sprintf(numStr, "%4d", num);
//			j = (ushort)((i/2)%DATA_LINE_MAX+1);
//			if (!(i%2)) {
//				// �P���(�R��)
//// MH810100(S) Y.Yamauchi 2019/10/31 �Ԕԃ`�P�b�g���X(�����e�i���X)	
////				grachr( j, 7, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
//				grachr( j, 8, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
//// MH810100(E) Y.Yamauchi 2019/10/31 �Ԕԃ`�P�b�g���X(�����e�i���X)	
//			}
//			else {
//				// �Q���(�Q��)
//				grachr( j, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
//			}
			j = (ushort)((i/2)%DATA_LINE_MAX+1);
			if( st_num_dg[i] <= 6 ){	// ���p6�����ȉ�
				col = 8;	// 8�J������
				cnt = 4;	// 4��
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:570)�Ή�
//				sprintf(numStr, "%4d", num);
				sprintf(numStr, "%4ld", num);
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:570)�Ή�
			}else{
				col = 10;	// 10�J������
				cnt = 2;	// 2��
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:571)�Ή�
//				sprintf(numStr, "%2d", num);
				sprintf(numStr, "%2ld", num);
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:571)�Ή�
			}
			grachr( j, (ushort)((i%2)*16+col), cnt, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			PreCount[i] = num;
		}
	}

	return 0;
}

// �J�[�\���\��
uchar dsp_ccomchk_cursor(int index, ushort mode, uchar page)
{
	ushort row  = (ushort)((index/2)%DATA_LINE_MAX+1);
	uchar new_page;
	ushort col  = (ushort)(index%2);
// MH322917(S) A.Iiizumi 2018/09/21 ���u����`�F�b�N���P
//	ushort size = (col && (row>1)) ? 8 : 4;
	ushort size = 8;
// MH322917(E) A.Iiizumi 2018/09/21 ���u����`�F�b�N���P
	new_page = index / PAGE_ITEM_MAX;
	if (new_page != page) {
		clr_dsp_count(new_page);
	}
// MH810100(S) K.Onodera 2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	grachr( row, ((col)?17:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[index] );
	grachr( row, ((col)?16:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[index] );
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	return new_page;
}

static uchar ChkNo;		// �ʐM�`�F�b�N�v���f�[�^�d���쐬�p�̒ʐM����No.
/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@�Z���^�[�ʐM�e�X�g                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_ccomtst( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_ccomtst( void )
{
	ushort	msg;
	char	mode;	// 0:����/ 1:�ʐM��/ 2:����

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[2] );		// "���Z���^�[�ʐM�e�X�g���@�@�@�@"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[4] );		// " �����ڑ����e�X�g���s���܂� "
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );		// "�@�@�@�@��낵���ł����H�@�@�@"
	Fun_Dsp(FUNMSG[19]);					// "�@�@�@�@�@�@ �͂� �������@�@�@"
	
	mode = 0;
	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			return MOD_CHG;
		case KEY_TEN_F5:
			if (mode == 2) {		// �u�I���v
				BUZPI();
				return MOD_EXT; 
			}
			break;
		case KEY_TEN_F4:
			if (mode == 0) {		// �u�������v
				BUZPI();
				return MOD_EXT; 
			}
			break;
		case KEY_TEN_F3:
			if (mode == 0) {		// �u�͂��v
				BUZPI();

				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[10] );			// "�@�@ ���΂炭���҂������� �@�@"
				displclr(4);
				Fun_Dsp(FUNMSG[00]);													// "�@�@�@�@�@�@ �@�@ �@�@�@�@�@�@"
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, FCSTR1[46] );			// "�@�@�@�@ �ʐM�e�X�g�� �@�@�@�@"
				if (++ChkNo > 99){
					ChkNo = 1;
				}
				NTNET_Snd_Data100_R(0, ChkNo);				// �ʐM�`�F�b�N�v��
				mode = 1;
			}
			else if (mode == 1) {	// �u���~�v
			}
			break;
		case IBK_COMCHK_END:		// �ʐM�e�X�g�I��
			if (mode == 1) {
				BUZPIPI();
				
				blink_end();										// �_�ŏI��
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );					// "�@�@�@�@�ʐM�e�X�g�I���@�@�@�@"
				displclr(3);
				
				if (IBK_ComChkResult == 0) {
					grachr ( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[6] );				// "�@�Z���^�[�Ƃ̒ʐM�͐���ł��@"
				}
				else {
					grachr ( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[7] );				// "�Z���^�[�Ƃ̒ʐM�Ɏ��s���܂���"
					
					switch (IBK_ComChkResult) {
					case 2:											// "�@�@�i02�F������g�p���j�@�@"
					case 3:											// "�@�@ �i03�F����ڑ����s�j �@�@"
					case 5:											// "�i05�F�s�b�o�R�l�N�V�������s�j"
					case 4:											// "�i04�F�s�b�o�R�l�N�V�������s�j"
					case 6:											// "�@�@�i06�F�T�[�o�[�������j�@�@"
					case 7:											// "�@�@�i07�F�������g�p���j�@�@"
					case 11:										// "�i11�FFOMA���W���[���ڑ��s�ǁj"
					case 12:										// "�@ �i12�F�_�C�A�����M���s�j �@"
					case 13:										// "�@�@�@�i13�FPPP�ڑ����s�j �@�@"
						grachr ( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMERR[IBK_ComChkResult-1] );
						break;
					case 99:
						grachr ( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMERR[14] );		// "�@ �i99�F��M�f�[�^�s��v�j �@"
						break;
					default:
						grachr ( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMERR[15] );		// "�@�@�i���s���R�R�[�h�F�@�j�@�@"
						opedpl2( 5, 22, (ulong)IBK_ComChkResult, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF  );
						break;
					}
				}
				Fun_Dsp(FUNMSG[8]);									// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

				mode = 2;
			}
			break;
			
		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@�ڑ���`�o�m�m�F		                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CcomApnDisp( void )                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-25                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
ushort FncChk_CcomApnDisp( void )
{
	ushort	fncChkCcomApnEvent;

	ushort	msg;
	ushort	F3Cnt;									// F3 3�񉟉��ɂ��CAPN�ύX�i�B���j���[�h�֑J��

	for ( ; ; ) {
		fncChkCcomApnEvent=0;
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[0] );		// "���ڑ���`�o�m�m�F���@�@�@�@�@"
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[1] );		// "APN �@�@�@�@�@�@�@�@�@�@�@�@�@"
		if(prm_get(COM_PRM, S_PAY, 24, 1, 2) != 2) {	// �N���W�b�g�Ȃ�
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[2] );	// "������IP�@�@�@�@�@�@�@�@�@�@�@"
			grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[3] );	// "����IP�@�@�@�@�@�@�@�@�@�@�@�@"
		}
		else if(_is_ntnet_remote() && prm_get(COM_PRM, S_PAY, 24, 1, 2) == 2) {	// �Z���^�[&�N���W�b�g
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[2] );	// "������IP�@�@�@�@�@�@�@�@�@�@�@"
			grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[8] );	// "�ڼޯ�IP�@�@�@�@�@�@�@�@�@�@�@"
			grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[3] );	// "����IP�@�@�@�@�@�@�@�@�@�@�@�@"
		}
		else {											// �N���W�b�g�̂�
			grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[8] );	// "�ڼޯ�IP�@�@�@�@�@�@�@�@�@�@�@"
			grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[3] );	// "����IP�@�@�@�@�@�@�@�@�@�@�@�@"
		}

		memset( APN_str, 0x20, sizeof(APN_str));	// Clear

		CcomApnGet( APN_str );						/*	APN Get						*/
		grachr( 2,  10, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, APN_str);				// APN�\��
		grachr( 3,  10, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, &APN_str[20]);		// APN�\��

		if(prm_get(COM_PRM, S_PAY, 24, 1, 2) != 2) {	// �N���W�b�g�Ȃ�
			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 62);				// ������IP
			grachr( 4,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// ������IP�\��

			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 65);				// ����IP
			grachr( 5,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// ����IP�\��
		}
		else if(_is_ntnet_remote() && prm_get(COM_PRM, S_PAY, 24, 1, 2) == 2) {	// �Z���^�[&�N���W�b�g
			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 62);				// ������IP
			grachr( 4,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// ������IP�\��

			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CRE, 21);				// ����IP
			grachr( 5,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// �ڼޯ�P�\��

			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 65);				// ����IP
			grachr( 6,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// ����IP�\��
		}
		else {										// �N���W�b�g�̂�
			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CRE, 21);				// ����IP
			grachr( 4,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// �ڼޯ�IP�\��

			memset( IP_str, 0x20, sizeof(IP_str));		// Clear
			CcomIpGet( IP_str, S_CEN, 65);				// ����IP
			grachr( 5,  10, 15, 0, COLOR_BLACK, LCD_BLINK_OFF, IP_str);				// ����IP�\��
		}

		Fun_Dsp(FUNMSG[8]);							/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */

		F3Cnt=0;
		for ( ; ; ) {
	
			msg = StoF( GetMessage(), 1 );

			switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				return MOD_CHG;

			case KEY_TEN_F5:			// �u�I���v
				BUZPI();
				return MOD_EXT;

			case KEY_TEN_F3:		//�ύX
				if( F3Cnt < 2 ){
					F3Cnt++;
				}else{
					F3Cnt=0;
					BUZPI();
					fncChkCcomApnEvent = FncChk_CcomApnSelect();
				}
				break;
			case KEY_TEN_F1:		//
			case KEY_TEN_F2:		//
			case KEY_TEN_F4:		//
					F3Cnt=0;
				break;
			default:
				break;
			}
			if (fncChkCcomApnEvent == MOD_CHG){
				OPECTL.Mnt_mod = 0;
				OPECTL.Mnt_lev = (char)-1;
				return MOD_CHG;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			else if( fncChkCcomApnEvent == MOD_CUT ) {
				OPECTL.Mnt_mod = 0;
				OPECTL.Mnt_lev = (char)-1;
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			}else if (fncChkCcomApnEvent == MOD_EXT) {
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@�d�g��M��Ԋm�F		                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CcomAntDisp( void )                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Author      :  k.totsuka                                      		   |*/
/*| Date        :  2012-09-10                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
ushort FncChk_CcomAntDisp( void )
{
	ushort	msg;
	ushort	maxCount = 0;
	ushort	nowCount = 1;
	ushort	antAve = 0,antAve2 = 0,antMax = 0,antMin = 0;
	ushort	recAve = 0,recMax = 0,recMin = 0;
	long	antAve_work = 0,recAve_work = 0;
	char	prog[16];
	ushort	modemOff = 0;
	ushort	pos = 0;
	short	in_dat = -1;
	short	dat[2];
	ushort	measure = 0;
	ushort	tmRetryCnt;
	uchar	mnt_chg = 0;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	uchar	discon = 0;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

	// HOST�Ƒ���M���Ȃ�A���e�i���x���擾�J�n�s��
	if( RAUhost_GetSndSeqFlag() != RAU_SND_SEQ_FLG_NORMAL ||
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//		Credit_GetSeqFlag() != RAU_RCV_SEQ_FLG_NORMAL ||
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
		RAUhost_GetRcvSeqFlag() != RAU_RCV_SEQ_FLG_NORMAL ){
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[0] );				// "���d�g��M��Ԋm�F���@�@�@�@�@"
		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[11] );				// "�@�@�ʐM���͎��s�ł��܂���@�@"

		Fun_Dsp(FUNMSG[8]);																// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

		for ( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				return MOD_CHG;
			case KEY_TEN_F5:						// �u�I���v
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F1:
			case KEY_TEN_F2:
			case KEY_TEN_F3:
			case KEY_TEN_F4:						// �u�Ǐo�v
				BUZPI();
				break;
			default:
				break;
			}
		}
	}

	// �ȉ��A��������i���M�Ԋu�A���M�񐔁j����
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[0] );				// "���d�g��M��Ԋm�F���@�@�@�@�@"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[10] );				// "�@ �����������͂��ĉ����� �@"
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[8] );				// "�@���M�Ԋu�F�@�@�@�@�@�@�@�@�@"
	grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[9] );				// "�@���M�񐔁F�@�@�@�@�@�@�@�@�@"
	dat[0] = ANT_INT_DEF;						// ���M�Ԋu
	dat[1] = ANT_CNT_DEF;						// ���M��
	antsetup_dsp( dat, pos );

	Fun_Dsp(FUNMSG[68]);						// "�@���@�@���@�@�@�@ ���s  �I�� "

	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			return MOD_CHG;

		case KEY_TEN_F5:						// �u�I���v
			BUZPI();
			return MOD_EXT;

		case KEY_TEN_F1:						// �u���v
		case KEY_TEN_F2:						// �u���v
//			if(( in_dat < 1 ) || ( in_dat > 999 )){ 					// ���͒l�͈�����
//				BUZPIPI();
//				in_dat = -1;
//				break;
//			}
			if( in_dat != -1 ) {
				dat[pos] = in_dat;
				in_dat = -1;
			}

			BUZPI();
			if(pos == 0){
				pos = 1;
			}
			else{
				pos = 0;
			}
			antsetup_dsp( dat, pos );
			break;
		case KEY_TEN_F3:
			break;
		case KEY_TEN_F4:						// �u�Ǐo�v
			if( in_dat != -1 ){
				dat[pos] = in_dat;
				in_dat = -1;
			}
			if((dat[0] < 1) || (dat[0] > 999) || (dat[1] < 1) || (dat[1] > 999)){
				BUZPIPI();
				in_dat = -1;
				break;
			}
			BUZPI();
			measure = 1;
			break;
		case KEY_TEN:
			BUZPI();
			if( in_dat == -1 )
			{	// in_dat <= ���͒l
				in_dat = (short)(msg - KEY_TEN0);
			}else{
				in_dat = (short)( in_dat % 100 ) * 10 + (short)(msg - KEY_TEN0);
			}
			// ���͒l��\������i���]�\���j
			opedsp( 4+pos, 14, (unsigned short)in_dat, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );
			break;
		case KEY_TEN_CL:
			BUZPI();
			in_dat = 0;
			// ���͒l��\������i���]�\���j
			opedsp( 4+pos, 14, (unsigned short)in_dat, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );
			break;
		default:
			break;
		}
		if(measure){								// ���菈����
			break;
		}
	}

	// �ȉ��A���菈��
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[0] );				// "���d�g��M��Ԋm�F���@�@�@�@�@"
	grachr( 2,  0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_ON, CCOMSTR4[3] );			// "�@�@ ���΂炭���҂������� �@�@"
	Fun_Dsp(FUNMSG[117]);															// "             ���~        �I�� "

	if( KSG_RauGetAntLevel( 0, dat[0] ) < 0 ){
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[0] );				// "���d�g��M��Ԋm�F���@�@�@�@�@"
		grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[11] );				// "�@�@�ʐM���͎��s�ł��܂���@�@"

		Fun_Dsp(FUNMSG[8]);																// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "

		for ( ; ; ) {
			msg = StoF( GetMessage(), 1 );
			switch( KEY_TEN0to9( msg ) ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				return MOD_CHG;
			case KEY_TEN_F5:						// �u�I���v
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F1:
			case KEY_TEN_F2:
			case KEY_TEN_F3:
			case KEY_TEN_F4:						// �u�Ǐo�v
				BUZPI();
				break;
			default:
				break;
			}
		}
	}
	
	Lagtim( OPETCBNO, 6, 60*50 );					// ������M��ϰ6(60s)�N��
	tmRetryCnt = 0;
	maxCount = dat[1];

	// �v������
	for ( ; ; ) {

		for ( ; ; ) {

			msg = StoF( GetMessage(), 1 );

			switch( msg ){							// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				if(nowCount <= maxCount){
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:576)�Ή�
//					discon == 1;
					discon = 1;
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:576)�Ή�
				}
				else{
					Lagcan(OPETCBNO, 6);
					if(modemOff == 0){
						KSG_RauGetAntLevel( 1, 0 );		// �A���e�i���x���A��M�d�͎w���̎擾�I��
					}
					return MOD_CUT;
				}
				break;				
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				if(nowCount <= maxCount){
					mnt_chg ^= 1;
				}
				else{
					Lagcan(OPETCBNO, 6);
					if(modemOff == 0){
						KSG_RauGetAntLevel( 1, 0 );		// �A���e�i���x���A��M�d�͎w���̎擾�I��
					}
					return MOD_CHG;
				}
				break;

			case KEY_TEN_F5:						// �u�I���v
				BUZPI();
				Lagcan(OPETCBNO, 6);
				if(modemOff == 0){
					KSG_RauGetAntLevel( 1, 0 );		// �A���e�i���x���A��M�d�͎w���̎擾�I��
				}

				if(mnt_chg){
					return MOD_CHG;
				}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				else{
//					return MOD_EXT;
//				}
				else if(discon){
					return MOD_CUT;
				}else{
					return MOD_EXT;
				}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_TEN_F1:
			case KEY_TEN_F2:
			case KEY_TEN_F4:
				break;
			case KEY_TEN_F3:						// �u���~�v
				if(modemOff == 0){
					if(nowCount <= maxCount){
						BUZPI();
						Lagcan(OPETCBNO, 6);
						dispmlclr(1, 3);
						KSG_RauGetAntLevel( 1, 0 );		// �A���e�i���x���A��M�d�͎w���̎擾�I��
						modemOff = 1;
						grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[7] );			// "�@�@�@�����𒆎~���܂����@�@�@"
						Fun_Dsp(FUNMSG[8]);				// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
					}
				}
				break;
			case KSG_ANT_EVT:						// �A���e�i���x����M�C�x���g
				Lagtim( OPETCBNO, 6, 60*50 );			// ������M��ϰ6(60s)�N��
				if(nowCount <= maxCount){
					sprintf(prog, "(%03d/%03d)", nowCount, maxCount);
					grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[5] );		// "�@�@���蒆�@�@�@�@�@�@�@�@�@�@"
					grachr( 3, 12, 9, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar *)prog );
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[1] );		// "�A���e�i���x���F�@�@�@�@�@�@�@"
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[2] );		// "��M�d�͎w���@�F�@�@�@�@�@�@�@"
					if(KSG_gAntLevel != -1){
						opedpl3( 4, 18, KSG_gAntLevel, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					}
					else{
						grachr( 4, 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, "�|" );
						break;
					}
					if(KSG_gReceptionLevel != -1){
						opedpl3( 5, 17, KSG_gReceptionLevel, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
					}
					else{
						grachr( 5, 18, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, "�|" );
						break;
					}
					antAve_work += KSG_gAntLevel;
					recAve_work += KSG_gReceptionLevel;
					if(antMax < KSG_gAntLevel){
						antMax = KSG_gAntLevel;
					}
					if((antMin == 0) || (antMin > KSG_gAntLevel)){
						antMin = KSG_gAntLevel;
					}
					if(recMax < KSG_gReceptionLevel){
						recMax = KSG_gReceptionLevel;
					}
					if((recMin == 0) || (recMin > KSG_gReceptionLevel)){
						recMin = KSG_gReceptionLevel;
					}
					nowCount++;
				}
				else{
					Lagcan(OPETCBNO, 6);
					BUZPI();
					antAve = (antAve_work*10) / (nowCount - 1);
					antAve2 = ((antAve_work*100) / (nowCount - 1)) % 10;
					recAve = recAve_work / (nowCount - 1);
					dispmlclr(1, 5);
					grachr( 3,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[6] );		// "�@�@�@���茋�ʁ@���� �ő� �ŏ�"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[1] );		// "�A���e�i���x���F�@�@�@�@�@�@�@"
					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[2] );		// "��M�d�͎w���@�F�@�@�@�@�@�@�@"
					if(antAve2 < 5){					// �����_2�ʂ��l�̌ܓ��������_1�ʂɔ��f
						opedpl3( 4, 16, antAve/10, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
						opedpl3( 4, 18, antAve%10, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					}
					else{
						if((antAve%10) == 9){
							opedpl3( 4, 16, (antAve/10)+1, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
						}
						else{
							opedpl3( 4, 16, antAve/10, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
						}
						opedpl3( 4, 18, (antAve%10)+1, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					}
					grachr( 4,  17, 1, 0, COLOR_BLACK, LCD_BLINK_OFF, "." );
					opedpl3( 4, 23, antMax, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					opedpl3( 4, 28, antMin, 1, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );
					opedpl3( 5, 17, recAve, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
					opedpl3( 5, 22, recMax, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
					opedpl3( 5, 27, recMin, 2, 0, 0, COLOR_BLACK, LCD_BLINK_OFF );
					KSG_RauGetAntLevel( 1, 0 );			// �A���e�i���x���A��M�d�͎w���̎擾�I��
					modemOff = 1;
					Fun_Dsp(FUNMSG[8]);					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				}
				break;
			case TIMEOUT6:								// ������M��ϰ��ѱ��
				if(tmRetryCnt == 0){
					KSG_RauGetAntLevel( 0, dat[0] );	// �A���e�i���x���A��M�d�͎w���̎擾�J�n
					Lagtim( OPETCBNO, 6, 60*50 );		// ������M��ϰ6(60s)�N��
					tmRetryCnt++;
				}
				else{
					dispmlclr(1, 5);
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR4[4] );
					KSG_RauGetAntLevel( 1, 0 );				// �A���e�i���x���A��M�d�͎w���̎擾�I��
					modemOff = 1;
					Fun_Dsp(FUNMSG[8]);					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				}
				break;
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �A���e�i���x���擾�̑��M�Ԋu�A���M�񐔕\��		                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ping_dsp( short * dat, short pos )                      |*/
/*| PARAMETER    : short * dat                                             |*/
/*|              : short pos                                               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
static void antsetup_dsp( short * dat, short pos )
{
	ushort mod[2];												// 0(normal) / 1(reverse)

	memset(mod, 0x00, sizeof(mod));
	mod[pos] = 1;
	opedsp( 4, 14, (unsigned short)dat[0], 3, 0, mod[0], COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 5, 14, (unsigned short)dat[1], 3, 0, mod[1], COLOR_BLACK, LCD_BLINK_OFF );

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@�Z���^�[�ǔԃN���A                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_cOibanClr( void )                                |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_cOibanClr( void )
{
	ushort	msg;
	uchar	mode;		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
	uchar	dsp;		// 0:�`��Ȃ�/ 1:��ʕ\��
	uchar	item_pos;	// �J�[�\���ʒu
	ushort	pos;
	uchar	i;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[8] );		// "���Z���^�[�ǔԃN���A���@�@�@�@"

	dsp      = 1;
	mode     = 0;
	item_pos = 0;

	for ( ; ; ) {
		if (dsp) {
		// �����\���X�V�v������
			if (mode == 0) {
				if (dsp == 1) {
					cOibanClrDsp();						// �N���A��ʕ\��
					cOibanCursorDsp(item_pos, 1);		// ���]�\��
					Fun_Dsp(FUNMSG[119]);				// " �ر�@�S�ر �@���@       �I�� "
				}
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			return MOD_CHG;

		case KEY_TEN_F5:			// �u�I���v
			if (mode == 0) {		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				BUZPI();
				return MOD_EXT;
			}
			else if (mode == 3) {	// �G���[���
				BUZPI();
				dsp  = 1;
				mode = 0;
			}
			break;

		case KEY_TEN_F1:		// �N���A
			if (mode == 0) {
				BUZPI();
				dispmlclr(1, 6);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				pos = (!(item_pos % 2) || (item_pos == CCOM_DATA_TOTAL)) ? 4 : 8;
				pos = 4;
				// �ޑK�Ǘ�
				if( item_pos == 2 ){
					pos = 8;	// 4�����Ȃ̂�8Byte
				}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				grachr( 2,    2,  pos, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[item_pos] );				// "�I������"
				grachr( 2, (ushort)(pos+2), 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );			// "�ǔԂ��N���A���܂�"
				grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
				Fun_Dsp(FUNMSG[19]);																	// "�@�@�@�@�@�@ �͂� �������@�@�@"
				mode = 1;		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
			}
			break;
		case KEY_TEN_F2:		// �S�N���A
			if (mode == 0) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				BUZPI();
				
				dispmlclr(1, 6);
				grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX] );				// "�S"
				grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );							// "�ǔԂ��N���A���܂�"
				grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
				Fun_Dsp(FUNMSG[19]);								// "�@�@�@�@�@�@ �͂� �������@�@�@"
				mode = 2;		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
			}
			break;

		case KEY_TEN_F3:
			switch (mode) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
			case 0:			// �J�[�\���ړ�
				BUZPI();
				cOibanCursorDsp(item_pos, 0);			// ���]�\��
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				if (++item_pos >= 4) {
				if (++item_pos >= 3) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					item_pos = 0;
				}
				cOibanCursorDsp(item_pos, 1);			// ���]�\��
				break;
			case 1:			// �ʃN���A���s
				BUZPI();
				switch(item_pos) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//				case 0:									// �Z���^�[�ǔԁi���Ɂj
//					Rau_SeqNo[RAU_SEQNO_ENTER] = 1;
//					break;
//				case 1:									// �Z���^�[�ǔԁi�W�v�j
//					Rau_SeqNo[RAU_SEQNO_TOTAL] = 1;
//					break;
//				case 2:									// �Z���^�[�ǔԁi���Z�j
//					Rau_SeqNo[RAU_SEQNO_PAY] = 1;
//					break;
//				case 3:									// �Z���^�[�ǔԁi�ޑK�Ǘ��W�v�j
//					Rau_SeqNo[RAU_SEQNO_CHANGE] = 1;
//					break;
				case 0:									// �Z���^�[�ǔԁi�W�v�j
					Rau_SeqNo[RAU_SEQNO_TOTAL] = 1;
					break;
				case 1:									// �Z���^�[�ǔԁi���Z�j
					Rau_SeqNo[RAU_SEQNO_PAY] = 1;
					break;
				case 2:									// �Z���^�[�ǔԁi�ޑK�Ǘ��W�v�j
					Rau_SeqNo[RAU_SEQNO_CHANGE] = 1;
					break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				dsp  = 1;	// ��ʍĕ`��
				mode = 0;
				break;
			case 2:			// �S�N���A���s
				BUZPI();
				for(i = 0; i < RAU_SEQNO_TYPE_COUNT; ++i) {
					Rau_SeqNo[i] = 1;
				}
				dsp  = 1;	// ��ʍĕ`��
				mode = 0;
				break;
			case 3:
			default:
				break;
			}
			break;

		case KEY_TEN_F4:						// �u�������v
			if ((mode == 1) || (mode == 2)) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				BUZPI();
				dsp  = 1;	// ��ʍĕ`��
				mode = 0;
			}
			break;

		default:
			break;
		}
	}
}

// �Z���^�[�ǔԉ�ʕ\��
void cOibanClrDsp(void)
{
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	ushort i, j;
	ushort i = 1, j = 0;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

	memset(PreCount, 0xFF, sizeof(ulong)*CCOM_DATA_MAX);
	dispmlclr(1, 6);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	for (i=1, j=0; i<=2; i++) {
//		grachr( i,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );
//		grachr( i, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );
//	}
	// �P�s��
	grachr( i,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );		// "�W�v�@�@"
	grachr( i, 15, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );		// "���Z�@�@"

	// �Q�s��
	i++;
	grachr( i,  0, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[j++] );		// "�ޑK�Ǘ�"
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	Fun_Dsp(FUNMSG[8]);		// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
}

// �J�[�\���\��
void cOibanCursorDsp(uchar index, ushort mode)
{
	ushort row  = (ushort)(index/2+1);
	ushort col  = (ushort)(index%2);
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//	ushort size = (col && (row>1)) ? 8 : 4;
	ushort size = 4;

	// �ޑK�Ǘ�
	if(index == 2 ){
		size = 8;
	}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)

	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR5[index] );
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@�ڑ���`�o�m�I���i�H�ꌟ���j	               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CcomApnSelect( void ) 	                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-25                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
ushort FncChk_CcomApnSelect( void )
{
	unsigned short	fncChkCcomApnEvent;
//	ushort	msg;
	char	wk[2];
//	uchar	mode;		// 0:APN�ύX�҂����/ 1:APN�ύX�I�����
//	ushort	i;

	for ( ; ; ) {
		DP_CP[0] = DP_CP[1] = 0;
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[4] );		// "���ڑ���I���i�H�ꌟ���j���@�@"

		fncChkCcomApnEvent = Menu_Slt( CCOMMENU2, CCOM_CHK2_TBL, CCOM_CHK2_MAX, 1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (fncChkCcomApnEvent) {

			case CCOM_APN_AMS:		// �`�l�r�p�`�o�m
			case CCOM_APN_AMN:		// �A�}�m�p�`�o�m
			case CCOM_APN_PWEB:		// ���ԏ�Z���^�[�p�`�o�m
				fncChkCcomApnEvent = FncChk_CcomApnSet(fncChkCcomApnEvent);
			break;

			case MOD_EXT:
				return MOD_EXT;

			case KEY_TEN_F5:			// �u�I���v
				BUZPI();
				return MOD_EXT;

			default:
				break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];

		if (fncChkCcomApnEvent == MOD_CHG) {
			OPECTL.Mnt_mod = 0;
			OPECTL.Mnt_lev = (char)-1;
			return MOD_CHG;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		else if( fncChkCcomApnEvent == MOD_CUT) {
			OPECTL.Mnt_mod = 0;
			OPECTL.Mnt_lev = (char)-1;
			return MOD_CUT;
		}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@�ڑ���ݒ�i�H�ꌟ���j		               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CcomApnSet( void ) 	                           |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-25                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
ushort FncChk_CcomApnSet( ushort	fncChkCcomApnEvent )
{
	ushort	msg;
//	char	wk[2];
	uchar	mode;		// 0:APN�ύX�҂����/ 1:APN�ύX�I�����
	ushort	i;

	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[4] );		// "���ڑ���I���i�H�ꌟ���j���@�@"
	grachr( 1,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[5] );		// "�ڑ���́A�@�@�@�@�@�@�@�@�@�@"

	if( fncChkCcomApnEvent == CCOM_APN_AMS){
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMMENU2[0] );		// "�P�D�`�l�r�p�`�o�m�@�@�@�@�@�@"
	}else if( fncChkCcomApnEvent == CCOM_APN_AMN){
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMMENU2[1] );		// "�Q�D�A�}�m�p�`�o�m�@�@�@�@�@�@"
	}else if( fncChkCcomApnEvent == CCOM_APN_PWEB){
		grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMMENU2[2] );		// "�R�D���ԏ�Z���^�[�p�`�o�m�@�@"
	}else{
															// ""
	}
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, APN_TBL[fncChkCcomApnEvent - CCOM_APN_AMS]);				// APN�\��
	grachr( 4, 0, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, &APN_TBL[fncChkCcomApnEvent - CCOM_APN_AMS][30]);			// APN�\��
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );			// "�@�@�@�@��낵���ł����H�@�@�@"
	Fun_Dsp(FUNMSG[19]);						// "�@�@�@�@�@�@ �͂� �������@�@�@"

	mode = 0;
	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				return MOD_CHG;

			case KEY_TEN_F5:			// �u�I���v
				if( mode == 1){ 
					BUZPI();
					return MOD_EXT;
				}
				break;
			case KEY_TEN_F3:		// �u�͂��v
				if( mode == 0){ 
					BUZPI();
					for(i=0;i<32;i++){
						if(APN_TBL[fncChkCcomApnEvent - CCOM_APN_AMS][i] == 0x20){
							break;
						}
					}
					CcomApnSet( APN_TBL[fncChkCcomApnEvent - CCOM_APN_AMS],i );

					DataSumUpdate(OPE_DTNUM_COMPARA);			/* update parameter sum on ram */
					(void)FLT_WriteParam1(FLT_NOT_EXCLUSIVE);	// FlashROM update
					wopelg( OPLOG_CCOMCHK_APN, 0, 0 );			// ���엚��o�^  �����ʐM���� APN�ύX����i�V�X�e�������e�i���X�j

					grachr( 5,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[6] );		// "�`�o�m��ύX���܂����B�@�@�@�@"
					grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[7] );		// "�ċN����ɍX�V����܂��B�@�@�@"
					Fun_Dsp(FUNMSG[8]);							// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
					mode = 1;	
				}else if( mode == 1){ 
				}
				break;
			case KEY_TEN_F4:		// �u�������v
				if( mode == 0){ 
					BUZPI();
					return MOD_EXT;
				}
				break;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@APN�����ʃp���[���[�^����z��Ɏ擾����      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CcomApnGet( void ) 			                           |*/
/*| PARAMETER    : uchar *apn_str:apn������                                |*/
/*| RETURN VALUE : void				                                       |*/
/*| 			 : APN�������ݒ肩��ǂݏo���AAPN���������i�ݒ�j��	   |*/
/*| 			 : �z��ɃZ�b�g����@									   |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-26                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
void CcomApnGet( uchar *apn_str )
{
	unsigned short	apn_cnt;
	unsigned char	prm;
	short			i;
	char 			pos;

	apn_cnt=(unsigned short)(prm_get(COM_PRM,S_CEN,79,2,1));	/* APN������				*/
	prm=(unsigned char)(prm_get(COM_PRM,S_CEN,80,3,4));			/* APN 1������				*/
																/*							*/
	if((apn_cnt != 0)&&(apn_cnt <= 32)&&(prm != 0)){			/* APN�������w�肠��C32�����ȉ��C�ŏ��̃f�[�^���O�ȊO	?(Y)*/
		for( i=0; i<apn_cnt; i++){								/*							*/
			pos = (char)(4-(i%2)*3);							/* 654�܂���321				*/
			prm = (unsigned char)prm_get(COM_PRM,S_CEN,(short)(80+i/2), (char)3, (char)pos) ;/* APN	*/
			*apn_str = (uchar)(prm);							/* APN Get					*/
			apn_str++;											/*							*/
		}														/*							*/
	}															/*							*/
																/*							*/
}																/*							*/

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@IP�����ʃp���[���[�^����z��(����)�Ɏ擾���� |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CcomIpGet( void ) 			                           |*/
/*| PARAMETER    : uchar *ipstr:������i�[��                               |*/
/*| 			 : ses �����No.(���ʃp�����[�^)					           |*/
/*| 			 : adr ���ڽNo.(���ʃp�����[�^)					           |*/
/*| RETURN VALUE : void				                                       |*/
/*| 			 : IP��ݒ肩��ǂݏo���A�z��ɃZ�b�g����    		       |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-26                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
void CcomIpGet( uchar *ipstr, short	ses, short	adr)
{

	unsigned char	prm;
	char			i;
	char			j;
	short			k;
	short			spc_set;

	k=0;																			/*							*/
	for( j=0; j<4;j++){																/*	4��ï�					*/
		if( j > 0 ){																/* ����̓s���I�h���Ȃ�	*/
			ipstr[k++]= '.';														/*							*/
		}																			/*							*/
		if( j == 2 ){																/* ���̃A�h���X				*/
			adr++;																	/*							*/
		}																			/*							*/
																					/*							*/
		spc_set = 0;																/*							*/
		for( i=0 ;i<3; i++){ 														/* 3�������擾				*/
			prm =(unsigned char)(prm_get(COM_PRM,ses,adr,1,(char)(6-i-(j % 2)*3)));	/* IP 1-3������	/4-6������	*/
			prm +='0';	/* IP 1-3������	/4-6������	*/								/*							*/
			if(( prm == '0')&&(spc_set == 0)){										/*							*/
				prm = 0x20;															/* �擪��0�̓X�y�[�X�ɒu������	*/
			}else{																	/* 0�ȊO�̕������o?(Y)		*/
				spc_set = 1;														/*							*/
			}																		/*							*/
			ipstr[k++]= prm;														/*							*/
		}																			/*							*/
																					/*							*/
	}																				/*							*/
																					/*							*/
}																					/*							*/

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@APN�����ʃp���[���[�^�ɃZ�b�g����			   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CcomApnSet( void ) 			                           |*/
/*| PARAMETER    : uchar *apn_str:apn������                                |*/
/*| RETURN VALUE : void				                                       |*/
/*| 			 : APN�������ݒ�ɏ�������						       |*/
/*| Author      :  T.Nakayama                                      		   |*/
/*| Date        :  2008-08-26                                      		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
void CcomApnSet( const uchar *apn_str,unsigned short apn_cnt )
{
	short			i;											/*							*/
																/*							*/
	if((apn_cnt != 0 )&&(apn_cnt <= 32 )){						/*							*/
		CPrmSS[S_CEN][79] = apn_cnt;							/*							*/
		for( i=0;i<32;i++){										/*							*/
			if( i < apn_cnt	){									/*	APN��������?(Y)			*/
																/*							*/
				if( (i % 2) == 0 ){								/*							*/
					CPrmSS[S_CEN][80+i/2] = (long)apn_str[i]*1000L;	/*	�ݒ�㏑��			*/
				}else{											/*							*/
					CPrmSS[S_CEN][80+i/2] += (long)apn_str[i];	/*	�ݒ���Z				*/
				}												/*							*/
			}else{												/*	APN���������I��			*/
				if( (i % 2) == 0 ){								/*							*/
					CPrmSS[S_CEN][80+i/2] = 0L;					/*	�ݒ�㏑��				*/
				}else{											/*							*/
					CPrmSS[S_CEN][80+i/2] += 0L;				/*	�ݒ���Z				*/
				}												/*							*/
			}													/*							*/
		}														/*							*/
	}															/*							*/
																/*							*/
}																/*							*/

// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)
////[]----------------------------------------------------------------------[]
/////	@brief			�����ē����ԉ�ʂł̃A�i�E���X�e�X�g
////[]----------------------------------------------------------------------[]
/////	@param[in]		volume	: volume
/////	@attention		None
/////	@author			Yamada
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2008/02/05<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
//void VoiceGuideTest(char volume)
//{
//	pre_volume[0] = volume;
//	memset(avm_test_no, 0, sizeof(avm_test_no));
//	avm_test_no[0] = 145;			// ���p�[�N���ԏ�������p�����������肪�Ƃ��������܂���
//	avm_test_no[1] = 136;			// �t���b�v�ł������������Ƃ��m�F���ďo�ɂ��ĉ�����
//	avm_test_cnt = 2;
//	avm_test_ch = 0;;
//
//	ope_anm( AVM_AN_TEST );		// ���肪�Ƃ��������܂����B
//}
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(�����e�i���X)

//[]----------------------------------------------------------------------[]
//	@brief			�t���b�v���u�`�F�b�N���j���[�\��
//[]----------------------------------------------------------------------[]
//	@param[in]		volume	: volume
//	@attention		None
//	@author			Yamada
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2008/02/05<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort		FunChk_FlpCHK( void )
{
	ushort	ret;
	ushort	msg;
	char	wk[2];
	char	org[2];
	uchar	flp_check;
	uchar	i;

	/* CRR�̐ڑ��ݒ肪����Ă��Ȃ����͉�ʑJ�ڂ��Ȃ��i�x�����j*/
	if(!( GetCarInfoParam() & 0x04 )) {
		BUZPIPI();
		return MOD_EXT;
	}
	flp_check = 0;
	for( i=INT_CAR_START_INDEX ; i<TOTAL_CAR_LOCK_MAX; i++ ){
		if( (LockInfo[i].lok_syu == LK_TYPE_SANICA_FLAP) ){
			flp_check = 1;	// �T�j�J�ݒ�̃t���b�v������
			break;
		}
	}

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	while (1) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[54]);									/* "���t���b�v���u�`�F�b�N���@�@�@" */
		msg = Menu_Slt(FLPBOARDMENU, FUNC_FLP_TBL, FUNC_FLP_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return (MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)		
		case MOD_CHG:
			return(MOD_CHG);
		case MOD_EXT:		/* "�I��" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return(MOD_EXT);
		case MNT_FLP_LOOP_DATA_CHK:					// ���[�v�f�[�^�m�F
			if( 1 == flp_check ){
				wopelg( OPLOG_FLA_LOOP_DATA, 0, 0 );	// ���엚��o�^
				ret = FunChk_FlapLoopData();
			} else {				// �ʐM�^�C�v�̃t���b�v���ݒ肳��Ă��Ȃ��ꍇ�A�J�ڂ����Ȃ�
				BUZPIPI();
			}
			break;
		case MNT_FLP_SENSOR_CTRL:					// �ԗ����m�Z���T�[����
			wopelg( OPLOG_FLA_SENSOR_CTRL, 0, 0 );	// ���엚��o�^
			ret = FunChk_FlapSensorControl();
			break;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if (ret == MOD_CHG) {
		if (ret == MOD_CHG ||  ret == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(ret);
}

//[]----------------------------------------------------------------------[]
///	@brief			�t���b�v�����`�F�b�N���j���[�\��
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	FunChk_CrrBoardCHK( void )
{
	ushort	ret;
	ushort	msg;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	while (1) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[52]);		/* "���t���b�v�����`�F�b�N���@�@" */
		msg = Menu_Slt(CRRBOARDMENU, FUNC_CRR_TBL, FUNC_CRR_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return (MOD_CUT);
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)		
		case MOD_CHG:
			return(MOD_CHG);
		case MOD_EXT:		/* "�I��" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return(MOD_EXT);
		case MNT_CRR_VER:	/* �o�[�W�����`�F�b�N	*/
			wopelg( OPLOG_CCRVERCHK, 0, 0 );		// ���엚��o�^
			ret = FunChk_CRRVer();
			break;
		case MNT_CRR_TST:	/* �܂�Ԃ��e�X�g 		*/
			wopelg( OPLOG_CCRCOMCHK, 0, 0 );		// ���엚��o�^
			ret = FunChk_CrrComChk();
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if (ret == MOD_CHG) {
		if ( ret == MOD_CHG || ret == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)			
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(ret);
}

const	ulong	CarParamDef[3][10]={
	{2,		0,	0,	0,	0,20,	0,110000,	0},		// [0]���ԁiCRR�j�̂�
	{0,		0,	0,	0,	0,10,	1,	   0,	0},		// [1]���ԁiIF�j�̂�
	{41,42,43,44,45,100,101,123,124},				// [2]�ݒ�A�h���X�p
};

/*[]----------------------------------------------------------------------[]*/
/*|  �Ԏ��@�\�ݒ�F						                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : CarFuncSelect( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	CarFuncSelect( void )
{
	ushort	msg;
	char	pos=0;		/* 0:���� 1:���ԁiIF�j*/
	char	changing=0;
	char	i;
	struct	LOCKINFO_REC	CarInfo;
	
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[0]);					/* "���Ԏ��@�\�I�����@�@�@�@�@�@�@" */
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[1]);					/* "�@�@�\�I���F�@�@�@�@�@�@�@�@�@" */
	grachr(2, 12, 4, 1, COLOR_BLACK, LCD_BLINK_OFF, &KIND_3_MENU[0][2]);			/* "����/���ԁiIF�j" */
	Fun_Dsp(FUNMSG[98]);																/* "  ��    ��         �ݒ�  �I�� " */
	
	for ( ; ; ) {
		msg = StoF(GetMessage(), 1);
		if (changing != 2) {
			switch (msg) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	/* �����e�i���X�L�[ */
				BUZPI();
				return MOD_CHG;
			case KEY_TEN_F5:	/* �I��(F5) */
				BUZPI();
				return MOD_EXT;
			case KEY_TEN_F1:	/* �� */
			case KEY_TEN_F2:	/* �� */
				pos ^= 1;
				BUZPI();
				//��ʕ\���ύX
				grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[1]);								/* "�@�@�\�I���F�@�@�@�@�@�@�@�@�@" */
				grachr(2, 12, (ushort)(pos==1?10:4), 1, COLOR_BLACK, LCD_BLINK_OFF, &KIND_3_MENU[pos][2]);	/* "����/���ԁiIF�j" */
				break;
			case KEY_TEN_F4:	/* �ݒ�(F4) */
				BUZPI();
				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[2]);				/* "�@�@�@�ݒ��ύX���܂����@�@�@" */
				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFUNC[3]);				/* "�@ ���Z�@�̍ċN�����K�v�ł� �@" */
				Fun_Dsp(FUNMSG2[43]);															/* "�@�@�@�@�@�@ �m�F �@�@�@�@�@�@" */
				changing = 2;
				for( i=0;i<TBL_CNT(CarParamDef[0]);i++ ){
					CPrmSS[S_TYP][CarParamDef[2][i]] = CarParamDef[pos][i];
				}

				memset(LockInfo, 0, sizeof(LockInfo));	/* �[���N���A */
				memset(&CarInfo, 0, sizeof(CarInfo));	/* �[���N���A */
				
				if( !pos ){
					CarInfo.adr = 50;
				}else{
					CarInfo.adr = 0;
				}
				CarInfo.dat.lok_syu = 11;
				CarInfo.dat.ryo_syu = 1;
				CarInfo.dat.area = 0;
				CarInfo.dat.posi = 1;
				CarInfo.dat.if_oya = 1;
				CarInfo.dat.lok_no = 1;
						
				LockInfo[CarInfo.adr] = CarInfo.dat;

				OpelogNo = OPLOG_CAR_FUNC_SELECT;
				SetChange = 1;
				f_ParaUpdate.BIT.other = 1;				// ���d����RAM��p�����[�^�f�[�^��SUM�X�V����
				memcpy( &bk_LockInfo, &LockInfo, sizeof( LockInfo ));
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
				mnt_SetFtpFlag(PRM_CHG_REQ_NORMAL);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�				
				// �Ԏ��p�����[�^�͂����ŏ㏑�����s���B ���ʃp�����[�^�͏�ʂ̋��ʉӏ��ŏ�������
				DataSumUpdate(OPE_DTNUM_LOCKINFO);				// RAM���SUM�X�V
				FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);			// FLASH������
				if( prm_get(COM_PRM,S_NTN,39,1,2) == 1 ){
					NTNET_Snd_Data225(0);						// NTNET�փf�[�^���M
				}
				break;
			default:
				break;
			}
		}
		else {
			switch (msg) {
			case KEY_TEN_F3:
				BUZPI();
				if( CP_MODECHG == 0 ){
					return MOD_CHG;
				}else{
					return MOD_EXT;
				}
			default:
				break;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FRdrChk_Wait                   	                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| COMMENT      : ���Cذ�ް��������̖߂蔻�菈��                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : K.MOTOHASHI                                             |*/
/*| Date         : 2005/07/12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	FRdrChk_Wait( void )
{
	
	short		msg;		// ��Mү����
	
	Lagtim( OPETCBNO, 6, 10*50 );					// �����҂���ϰ(10s)�N��
	for( ; ; ){
		msg = StoF( GetMessage(), 1 );				// ү���ގ�M

		switch( msg){								// ��Mү���ށH
			case ARC_CR_E_EVT:						// �I������ގ�M
			case TIMEOUT6:							// �����҂���ϰ��ѱ��
				Lagcan( OPETCBNO, 6 );
				return;
			default:
				break;
		}
	}
	
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ݒ�ʐM���O��                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ryokin_Setlog_Print( void )                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ushort                                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
#define	RYOSET_INJI_END	(PREQ_RYOSETTEI_LOG | INNJI_ENDMASK)
extern	void	LogDateGet( date_time_rec *Old, date_time_rec *New, ushort *Date );
ushort Ryokin_Setlog_Print( void )
{
	ushort	NewOldDate[6];		// �ŌÁ��ŐV۸��ް����t�ް��i�[�ر
	ushort	LogCount;			// LOG�o�^����
	date_time_rec	NewestDateTime, OldestDateTime;
	if (Ope_isPrinterReady() == 0) {	// ���V�[�g�o�͕s��
		BUZPIPI();
		return MOD_EXT;
	}
	BUZPI();
	Ope2_Log_NewestOldestDateGet( eLOG_REMOTE_SET, &NewestDateTime, &OldestDateTime, &LogCount );
	// �ŌÁ��ŐV���t�ް��擾
	LogDateGet( &OldestDateTime, &NewestDateTime, &NewOldDate[0] );

	return SysMnt_Log_CntDsp2	(	//	���O�����\����ʏ����Q�i�Ώۊ��Ԏw�肠��j
										LOG_REMOTE_SET,
										LogCount,
										LOGSTR1[13],
										PREQ_RYOSETTEI_LOG,
										&NewOldDate[0]
									);
}	

//[]----------------------------------------------------------------------[]
///	@brief			FTP�ڑ��m�F�`�F�b�N���j���[
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	Ftp_Connect_Chk( void )
{
	ushort	ret;
	ushort	msg;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	while (1) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[0]);									/* "���e�s�o�ڑ��`�F�b�N���@�@�@�@" */
		msg = Menu_Slt(FTPCONNECTMENU, FUNC_FTPCONNECTCHK_TBL, FUNC_FTPCONNECT_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {			/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case MOD_CHG:
				ret = MOD_CHG;
				break;
			
		case MOD_EXT:			/* "�I��" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return MOD_EXT;
			
		case FTP_CONNECT_FUNC:	/* �ڑ��m�F */
			ret = Ftp_ConnectChk();
			break;

		case FTP_PARAM_UPL:		/* �p�����[�^�A�b�v���[�h	*/
			ret = ParameterUpload(0);
			break;
		}
		if( ret == MOD_CHG || ret == MOD_EXT ){
			if(!Ope_IsEnableDoorKnobChime()) {
				//�h�A�m�u�J�`���C����������Ԃ̂܂܃����e�i���X���[�h�I���܂��͉�ʂ��I�������ꍇ�͗L���ɂ���
				Ope_EnableDoorKnobChime();
			}
		}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if ( ret == MOD_CHG ) {
		if ( ret == MOD_CHG || ret == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(ret);
}

/*[]----------------------------------------------------------------------[]*/
/*| �p�����[�^�[�A�b�v���[�h                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ParameterUpload( void )                                 |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : �ݒ�Ӱ�޽���OFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	ParameterUpload( uchar type )
{
	
	ushort	msg, usSysEvent;
	uchar	loop = 0;
	
	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[12] );						// "���p�����[�^�A�b�v���[�h���@�@"
	grachr( 3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10] );						// "�@�@ ���΂炭���҂������� �@�@"
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[16] );						// "�ʐM�I����A�ēx������s���ۂ�"
	grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[17] );						// "���Ԃ��󂯂Ď��s���ĉ������B�@"
	Ope_DisableDoorKnobChime();

	wopelg( OPLOG_FTP_PARAM_UPLD, 0, 0 );							// ���엚��o�^
	remotedl_chg_info_bkup();
	remotedl_connect_req( OPLOG_REMOTE_DISCONNECT, CTRL_PARAM_UPLOAD );
	
	while ( !loop ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
		case REMOTE_CONNECT_EVT:									// FTP�ʐM�ɂ��Rism�ڑ��ؒf�ʒm
			loop = 1;
			break;
		default:
			break;
		}
	}
	
	remotedl_script_typeset( PARAMTER_SCRIPT );						// �X�N���v�g�t�@�C����ʃZ�b�g
	Param_Upload_type_set( type );
	if( type ){														// ���[�U�[�����e�y�ы��ʃp�����[�^����̃R�[��
		usSysEvent = Param_Upload_ShortCut();						// �V���[�g�J�b�g�����ֈڍs
	}else{
		usSysEvent = SysMnt_FTPClient();							// ���ʃp�����[�^�A�b�v���[�h���{
	}
	
	remotedl_connect_req( OPLOG_REMOTE_CONNECT, CTRL_PARAM_UPLOAD );
	remotedl_chg_info_restore();
	
	return usSysEvent;
}

/*[]----------------------------------------------------------------------[]*/
/*| FTP�ڑ��m�F				                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ftp_ConnectChk( void )       	                       |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : �ݒ�Ӱ�޽���OFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	Ftp_ConnectChk( void )
{
	
	ushort	msg, ret, end_flag;
	ushort	result = 0;
	uchar	mnt_chg = 0;
	ret		 = 0;
	end_flag = 0;
	
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[1]);		// "���ڑ��m�F���@�@�@�@�@�@�@�@�@"
	grachr(3, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// "�@�@ ���΂炭���҂������� �@�@"
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[16]);		// "�ʐM�I����A�ēx������s���ۂ�"
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[17]);		// "���Ԃ��󂯂Ď��s���ĉ������B�@"
	Fun_Dsp(FUNMSG[0]);														/* [0]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */

	
	wopelg( OPLOG_FTP_CONNECT_CHK, 0, 0 );							// ���엚��o�^

	remotedl_chg_info_bkup();										// �����_�̃X�e�[�^�X���o�b�N�A�b�v����
	remotedl_status_set( R_DL_IDLE );
	remotedl_connect_req( OPLOG_REMOTE_DISCONNECT, CTRL_CONNECT_CHK );
	Ope_DisableDoorKnobChime();
	
	while ( 1 ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				if( end_flag == 1 ){
					return MOD_CUT;
				}else{
					mnt_chg ^= 2;
					continue;
				}
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	/* �����e�i���X�L�[ */
				if( end_flag == 1 ){
					return MOD_CHG;
				}else{
					mnt_chg ^= 1;
					continue;
				}
				break;
								
			case KEY_TEN_F5:	/* �I��(F5) */
				if( end_flag == 1 ){
					BUZPI();
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					if( mnt_chg ){
//						return MOD_CHG;
//					}else{
//						return MOD_EXT;
//					}
					if( mnt_chg == 1){
						return MOD_CHG;
					}else if( mnt_chg == 2 ){
						return MOD_CUT;
					}else{
						return MOD_EXT;
					}
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
				}
				break;
			case REMOTE_CONNECTCHK_END:								// FTP�ʐM�ɂ��Rism�ڑ��ؒf�ʒm
				dispmlclr(3,6);										// ��ʕ\���ر
				end_flag = 1;
				if(!Ope_IsEnableDoorKnobChime()) {
					Ope_EnableDoorKnobChime();
				}
// GG120600(S) // Phase9 ���ʂ���ʂ���
//				result = remotedl_result_get(2);
				result = remotedl_result_get(RES_COMM);
// GG120600(E) // Phase9 ���ʂ���ʂ���
				if( result == PROG_DL_COMP ){
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[2] );		// "�@�@�@�@�@ ����I�� �@�@�@�@�@"
				}else{
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[3] );		// "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@"
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[4] );		// "�@�@�i�G���[�R�[�h�F�@�@�j�@�@"
					opedsp( 4, 20, result, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );				// �G���[�R�[�h�\��
					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[5] );		// "�ڑ��Ɏ��s���Ă��܂��B�ݒ�y��"
					grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[6] );		// "�ڑ����̊m�F���K�v�ł��B�@�@"
				}
#ifdef	CRW_DEBUG
				Lcd_WmsgDisp_OFF();	
#endif
				Fun_Dsp(FUNMSG[8]);									/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
				remotedl_connect_req( OPLOG_REMOTE_CONNECT, CTRL_CONNECT_CHK );
				remotedl_chg_info_restore();						// �ڑ��m�F��́A�X�e�[�^�X�����ɖ߂�
				break;
				
			default:
				break;
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ʃp�����[�^�A�b�v���[�h�m�F���                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : parameter_upload_chk( void )                            |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : �ݒ�Ӱ�޽���OFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
#define	PARAM_UPLOAD_INVALID	0

ushort	parameter_upload_chk( void )
{
	
	ushort	msg,ret;

	if(!PARAM_UPLOAD_INVALID) {
		ret = MOD_EXT;
		if( CP_MODECHG ){
			ret = MOD_CHG;
		}
		return ret;
	}

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[7] );								// "���A�b�v���[�h�m�F���@�@�@�@�@"
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[8] );								// "�p�����[�^���ύX����܂����B�@"
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[9] );								// "���ʃp�����[�^�̃A�b�v���[�h��"
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[10] );								// "�s���܂��B�@�@�@�@�@�@�@�@�@�@"
	grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FTPCONNECTSTR[11] );								// "��낵���ł����H�@�@�@�@�@�@�@"
	Fun_Dsp(FUNMSG[19]);													/* "�@�@�@�@�@�@ �͂� �������@�@�@" */

	while ( 1 ) {
		msg = StoF( GetMessage(), 1 );
		switch(msg){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:	/* �����e�i���X�L�[ */
				BUZPI();
				return MOD_CHG;
								
			case KEY_TEN_F3:	/* �͂�(F3) */
				BUZPI();
				ret = ParameterUpload(1);
				if( CP_MODECHG == 0 ){
					ret = MOD_CHG;
				}
				return ret;

			case KEY_TEN_F4:	/* ������(F4) */
				BUZPI();
				ret = MOD_EXT;
				if( CP_MODECHG == 0 ){
					ret = MOD_CHG;
				}
				return ret;
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| LAN�ڑ��m�F				                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lan_Connect_Chk( void )       	                       |*/
/*| PARAMETER    : void										               |*/
/*| RETURN VALUE : MOD_EXT : F5 Key ( End Key )                            |*/
/*|              : MOD_CHG : �ݒ�Ӱ�޽���OFF                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	Lan_Connect_Chk( void )
{
	ushort	msg, ret;
	int		ret2;
	short	in_dat = -1;
	short	in_ip[4] = {192, 168, 1, 1};
	short	pos = 0;
	short	retryCnt = 0;
	int		socketDescriptor = -1;

	wopelg( OPLOG_LAN_CONNECT_CHK, 0, 0 );									// ���엚��o�^

	Lan_Connect_dsp();
	ping_dsp( in_ip, pos );

	ret2 = 0;

	for( ret = 0; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );
		if( ret2 ){
			if( msg == TIMEOUT6 ){											// 5�b�^�C���A�E�g�ōő�4�񃊃g���C
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				if(retryCnt >= 4){
					dispmlclr(4,4);											// ��ʕ\���ر
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[4] );		// "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@"
					Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
				}
				else{
					retryCnt++;
					// ping�J�n
					socketDescriptor = pingStart(in_ip);
					Ope_DisableDoorKnobChime();
				}
				continue;
			}
			if( msg == KEY_TEN_F5 ){
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				BUZPI();
				ret2 = 0;
				Lan_Connect_dsp();
				ping_dsp( in_ip, pos );
				in_dat = -1;
				continue;
			}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//			else if( msg == KEY_MODECHG ){
			else if( msg == KEY_MODECHG || msg == LCD_DISCONNECT ){
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					;
			}
			else{
				continue;
			}
		}

		switch( KEY_TEN0to9( msg ) )
		{
			case KEY_TEN_F5:
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				BUZPI();
				ret = MOD_EXT;
				break;
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				BUZPI();
				ret = MOD_CUT;
				break;			
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				if(socketDescriptor != -1){
					tfPingClose(socketDescriptor);
					socketDescriptor = -1;
				}
				BUZPI();
				ret = MOD_CHG;
				break;
			case KEY_TEN_F1:
			case KEY_TEN_F2:
				if(( in_dat != -1 ) && ( in_dat > 255 )){ 					// ���͒l�͈�����
					BUZPIPI();
					in_dat = -1;
					break;
				}
				if( in_dat != -1 ) {
					in_ip[pos] = in_dat;
					in_dat = -1;
				}
				BUZPI();
				// ���وʒu�����̈ʒu�ɾ��
				if( msg == KEY_TEN_F2 ){
					pos = ( 3 < pos + 1 ) ? 0 : pos + 1;
				}
				else{
					pos = ( 0 > pos - 1 ) ? 3 : pos - 1;
				}
				break;
			case KEY_TEN:
				BUZPI();
				if( in_dat == -1 )
				{	// in_dat <= ���͒l
					in_dat = (short)(msg - KEY_TEN0);
				}else{
					in_dat = (short)( in_dat % 100 ) * 10 + (short)(msg - KEY_TEN0);
				}
				// ���͒l��\������i���]�\���j
				opedsp( 2, (pos*7)+1, (unsigned short)in_dat, 3, 0, 1, COLOR_BLACK, LCD_BLINK_OFF );
				break;
			case KEY_TEN_CL:
			case KEY_TEN_F3:
				BUZPI();
				in_dat = -1;
				break;
			case KEY_TEN_F4:
				if(( in_dat != -1 ) && ( in_dat > 255 )){ 					// ���͒l�͈�����
					BUZPIPI();
					in_dat = -1;
					break;
				}
				if( in_dat != -1 ){
					in_ip[pos] = in_dat;
				}
				// �I�[���u���[�h�L���X�g�͋֎~�Ƃ���
				if((in_ip[0] == 255) && (in_ip[1] == 255) && (in_ip[2] == 255) && (in_ip[3] == 255)){
					BUZPIPI();
					in_dat = -1;
					break;
				}
				BUZPI();

				// ping�J�n
				dispmlclr(4,4);												// ��ʕ\���ر
				grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[2] );		// "�@�@�@�@ �ʐM�e�X�g�� �@�@�@�@"
				Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
				retryCnt = 0;
				socketDescriptor = pingStart(in_ip);
				Ope_DisableDoorKnobChime();
				ret2 = -1;
				break;
		}
		// ���وʒu�̐��l�𔽓]�\��������
		if(( msg == KEY_TEN_F1 ) || ( msg == KEY_TEN_F2 ) || ( msg == KEY_TEN_F3 ) || ( msg == KEY_TEN_CL )){
			ping_dsp( in_ip, pos );
		}
	}
	Lagcan( OPETCBNO, 6 );

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| ping���s				                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pingStart( short *dat )                                 |*/
/*| PARAMETER    : short *dat                                              |*/
/*| RETURN VALUE : socketDescriptor                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
int pingStart(short *dat)
{
	char	myIP[16];
	char	IPAddress[16];
	int		socketDescriptor;


	memset(myIP, 0x00, sizeof(myIP));
	sprintf(myIP, "%0d.%0d.%0d.%0d",
				prm_get(COM_PRM,S_MDL,2,3,4),
				prm_get(COM_PRM,S_MDL,2,3,1),
				prm_get(COM_PRM,S_MDL,3,3,4),
				prm_get(COM_PRM,S_MDL,3,3,1));

	memset(IPAddress, 0x00, sizeof(IPAddress));
	sprintf(IPAddress, "%0d.%0d.%0d.%0d",
			dat[0],
			dat[1],
			dat[2],
			dat[3]);
	if( (strcmp(IPAddress, "127.0.0.1") == 0) ||
		(strcmp(myIP, IPAddress) == 0) ){
		// ���[�v�o�b�N
		socketDescriptor = tfPingOpenStart (IPAddress,				// IP�A�h���X
											0,						// PING�G�R�[�v���̊Ԋu�i�b�P�ʁj
											32,						// PING�G�R�[�v���̃��[�U�f�[�^��
											(ttPingCBFuncPtr)0);	// PING�G�R�[����
		if(socketDescriptor != -1){
			Lagcan( OPETCBNO, 6 );
			dispmlclr(4,4);											// ��ʕ\���ر
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[3] );		// "�@�@�@�@�@ ����I�� �@�@�@�@�@"
			Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
			tfPingClose(socketDescriptor);
		}
		else{
			Lagtim( OPETCBNO, 6, 50*5 );							// 5�b�^�C�}�[�Z�b�g
		}
	}
	else{
		socketDescriptor = tfPingOpenStart (IPAddress,				// IP�A�h���X
											5,						// PING�G�R�[�v���̊Ԋu�i�b�P�ʁj
											32,						// PING�G�R�[�v���̃��[�U�f�[�^��
											pingOpenCB);			// PING�G�R�[����
		Lagtim( OPETCBNO, 6, 50*5 );								// 5�b�^�C�}�[�Z�b�g
	}
	return socketDescriptor;
}

/*[]----------------------------------------------------------------------[]*/
/*| ping�G�R�[�R�[���o�b�N				                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pingOpenCB(int socketDescriptor)                        |*/
/*| PARAMETER    : int socketDescriptor                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void pingOpenCB(int socketDescriptor)
{
	int ret;
	ttPingInfo pingInfo;

	ret = tfPingGetStatistics(socketDescriptor, &pingInfo);
	if(ret == 0){
		Lagcan( OPETCBNO, 6 );
		dispmlclr(4,4);										// ��ʕ\���ر
		if( pingInfo.pgiTransmitted == pingInfo.pgiReceived ) {
			// �v���p�P�b�g���Ɖ����p�P�b�g���������Ȃ�ΐ���I��
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[3] );	// "�@�@�@�@�@ ����I�� �@�@�@�@�@"
		}
		else {
			grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[4] );	// "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@"
		}
		Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
	}
	if(socketDescriptor != -1){
		tfPingClose(socketDescriptor);
	}
	if(!Ope_IsEnableDoorKnobChime()) {
		Ope_EnableDoorKnobChime();
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ping�e�X�g��ʕ\��					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Lan_Connect_dsp( void )                                 |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
static void Lan_Connect_dsp( void )
{
	unsigned long	prm_41_0001_3;
	unsigned long	prm_41_0001_456;

	dispclr();

/* Display */
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[0] );		/* "���k�`�m�ڑ��`�F�b�N���@�@�@�@" */
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[1] );		/* "�@�@�@�D�@�@�@�D�@�@�@�D�@�@�@" */
	grachr( 6,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, LANCHKSTR[5] );		/* "�ʐM��ʁF�@�@�@�@�@�@�@�@�@�@" */
	prm_41_0001_3 	= (unsigned long)prm_get(COM_PRM, S_MDL, 1, 1, 4);		//	41-0001-�B
	prm_41_0001_456 = (unsigned long)prm_get(COM_PRM, S_MDL, 1, 3, 1);		//	41-0001-�C�D�E
	if ((prm_41_0001_456 == 0) || (prm_41_0001_3 == 0)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[0] );		//	�`�t�s�n
	} else if ((prm_41_0001_456 == 10) && (prm_41_0001_3 == 1)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[1] );		//	10BASE-T(�S��d)
	} else if ((prm_41_0001_456 == 10) && (prm_41_0001_3 == 2)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[2] );		//	10BASE-T(����d)
	} else if ((prm_41_0001_456 == 100) && (prm_41_0001_3 == 1)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[3] );		//	100BASE-T(�S��d)
	} else if ((prm_41_0001_456 == 100) && (prm_41_0001_3 == 2)) {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[4] );		//	100BASE-T(����d)
	} else {
		grachr( 6,  10, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, LANTYPESTR[2] );		//	10BASE-T(����d)
	}

	Fun_Dsp(FUNMSG[9]);														/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ping�e�X�gIP�\��					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ping_dsp( short * dat, short pos )                      |*/
/*| PARAMETER    : short * dat                                             |*/
/*|              : short pos                                               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
static void ping_dsp( short * dat, short pos )
{
	ushort mod[4];												// 0(normal) / 1(reverse)

	memset(mod, 0x00, sizeof(mod));
	mod[pos] = 1;
	opedsp( 2, 1, (unsigned short)dat[0], 3, 0, mod[0], COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 2, 8, (unsigned short)dat[1], 3, 0, mod[1], COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 2, 15, (unsigned short)dat[2], 3, 0, mod[2], COLOR_BLACK, LCD_BLINK_OFF );
	opedsp( 2, 22, (unsigned short)dat[3], 3, 0, mod[3], COLOR_BLACK, LCD_BLINK_OFF );

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�t���b�v�Z���T�[����\��
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort FunChk_FlapSensorControl( void )
{
	ulong	ulwork;
	ushort	i,dsp=1;
	ushort	msg, command;
	short	tnoIndex[LOCK_IF_MAX]={0};
	short	roomNo;
	ushort	index=0;
	ushort	pageIndex;
	ushort	page, oldPage;
	ushort	drawCount;
	ushort	CNTMAX;
	
	for ( ; ; ) {

		if( dsp == 1 ){
			/* ������ʕ\�� */
			dispclr();
			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[1]);		/* "���ԗ����m�Z���T�[���쁄�@�@�@" */
			grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[2]);		/* "�@�Ԏ���              ��ԁ@�@" */
			Fun_Dsp( FUNMSG[103] );						// "  �{  �|�^��  OFF   ON   �I�� "

			CNTMAX = 0;
			for( i=INT_CAR_START_INDEX; i<TOTAL_CAR_LOCK_MAX; i++ ){
				WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
				if( (LockInfo[i].if_oya != 0) && (LockInfo[i].lok_syu != 0) ){	// �^�[�~�i��No��v���ڑ�����H
					if(1 == LKcom_Search_Ifno(LockInfo[i].if_oya)) {
						tnoIndex[CNTMAX] = i;
						CNTMAX++;
					}
				}
			}
			
			if( !CNTMAX ){								// �\���Ώۂ��P����������Ζ߂�
				BUZPIPI();
				return MOD_EXT;
			}
			dsp++;
			page = 0;
			oldPage = 1;								// �`�悳���邽��1��ݒ�

		}
		else {
		
			msg = StoF( GetMessage(), 1 );

			switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				BUZPI();
				Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
				return MOD_CHG;

			case KEY_TEN_F5:	/* "�I��" */
				BUZPI();
				Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
				if( dsp == 4 ){
					dsp = 1;
					break;
				}
				return MOD_EXT;

			case KEY_TEN_F1:	// F1:"��"
				if( dsp > 2 ) {
					break;
				}
				BUZPI();
				if( CNTMAX == 1 ) {
					break;
				}
				if( index <= 0 ){
					opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
														COLOR_BLACK, LCD_BLINK_OFF );		// �Ԏ��ԍ��\���i���]�����j
					index = (char)(CNTMAX - 1);
					page = index / 5;
					if(page == oldPage) {
						opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 1,
														COLOR_BLACK, LCD_BLINK_OFF );		// �Ԏ��ԍ��\���i���]�j
					}
				}else{
					opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
														COLOR_BLACK, LCD_BLINK_OFF );		// �Ԏ��ԍ��\���i���]�����j
					--index;
					page = index / 5;
					if(page == oldPage) {
						opedsp( (ushort)((index % 5)+ 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 1,
														COLOR_BLACK, LCD_BLINK_OFF );		// �Ԏ��ԍ��\���i���]�j
					}
				}
				break;

			case KEY_TEN_F2:	// F2:"��"
				if( dsp > 2 ) {
					break;
				}
				BUZPI();
				if( CNTMAX == 1 ) {
					break;
				}
				if( index >= CNTMAX - 1 ){
					opedsp( (ushort)((index % 5)+ 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
														COLOR_BLACK, LCD_BLINK_OFF );		// �Ԏ��ԍ��\���i���]�����j
					index = 0;
					page = 0;
					if(page == oldPage) {
						opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 1,
														COLOR_BLACK, LCD_BLINK_OFF );		// �Ԏ��ԍ��\���i���]�j
					}
				}else{
					opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
														COLOR_BLACK, LCD_BLINK_OFF );		// �Ԏ��ԍ��\���i���]�����j
					++index;
					page = index / 5;
					if(page == oldPage) {
						opedsp( (ushort)((index % 5) + 2), 2, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 1,
														COLOR_BLACK, LCD_BLINK_OFF );		// �Ԏ��ԍ��\���i���]�j
					}
				}
				break;

			case KEY_TEN_F3:	// F3:"OFF"
			case KEY_TEN_F4:	// F4:"ON"
				if( dsp > 2 ) {
					break;
				}

				// �������[�v�Z���T�����ڑ��͋���OFF/ON�s���Ȃ�
				if(IFM_FlapSensor[LockInfo[tnoIndex[index]].if_oya - 1].c_LoopSensor == 0) {
					BUZPIPI();
					break;
				}
				i = (ushort)LockInfo[tnoIndex[index]].if_oya - 1;
				if( IsErrorOccuerd( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_COMFAIL + i ) ){	// �Ώے[���ʐM����
					BUZPIPI();
					break;
				}
				if( LK_TYPE_CONTACT_FLAP == LockInfo[tnoIndex[index]].lok_syu ){	// �ړ_�t���b�v�̏ꍇ�A����ON/OFF�͂����Ȃ�
					BUZPIPI();
					break;
				}
				BUZPI();
			
				ulwork = (ulong)(( LockInfo[tnoIndex[index]].area * 10000L ) + LockInfo[tnoIndex[index]].posi );
				// �\���ύX
				if(msg == KEY_TEN_F3) {
					command = LK_SND_FORCE_LOOP_OFF;
					grachr( (ushort)(index%5+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[48] );			// [48] ����OFF
					wopelg( OPLOG_FLAP_SENSOR_OFF, 0, ulwork );	// ���엚��o�^
				}
				else {
					command = LK_SND_FORCE_LOOP_ON;
					grachr( (ushort)(index%5+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[49] );			// [49] ����ON
					wopelg( OPLOG_FLAP_SENSOR_ON, 0, ulwork );	// ���엚��o�^
				}

				// �R�}���h�𑗐M
				roomNo = tnoIndex[index] + 1;
				queset( FLPTCBNO, command, sizeof(tnoIndex[0]), &roomNo );
			
				break;

			default:
				break;
			}
		}
		
		if(page != oldPage) {
			oldPage = page;
			pageIndex = page * 5; 
			for(drawCount = 0; drawCount < 5; ++drawCount) {
				if(pageIndex + drawCount < CNTMAX) {
					if(index != (pageIndex + drawCount)) {
						opedsp( (ushort)(drawCount + 2), 2,
								(ushort)LockInfo[tnoIndex[pageIndex + drawCount]].posi, 4, 1, 0,
								COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��(���]����)
					}
					else {
						opedsp( (ushort)(drawCount + 2), 2,
								(ushort)LockInfo[tnoIndex[pageIndex + drawCount]].posi, 4, 1, 1,
								COLOR_BLACK, LCD_BLINK_OFF );	// ����ه��\��(���])
					}
					i = (ushort)LockInfo[tnoIndex[pageIndex + drawCount]].if_oya - 1;
					if(	!IsErrorOccuerd( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_COMFAIL + i ) ){	// �Ώے[���ʐM����
						switch(IFM_FlapSensor[LockInfo[tnoIndex[pageIndex + drawCount]].if_oya - 1].c_LoopSensor) {
						case '1':			// OFF���
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[45] );		// [45] "�n�e�e�@"
							break;
						case '2':			// ON���
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[46] );		// [46] "�n�m�@�@"
							break;
						case '3':			// �ُ�
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[47] );		// [47] "�ُ�@�@"
							break;
						case '4':			// ����OFF���
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[48] );		// [48] "����OFF "
							break;
						case '5':			// ����ON���
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[49] );		// [49] "����ON�@"
							break;
						case '6':			// �s�����
							grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[50] );		// [50] "�s���@�@"
							break;
						default:
							switch(IFM_FlapSensor[LockInfo[tnoIndex[pageIndex + drawCount]].if_oya - 1].c_ElectroSensor) {
							case '1':			// OFF���
							case '2':			// ON���
								grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[51] );		// [51] "�@�|�@�@"
								break;
							default:
								grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[52] );		// [52] "���ڑ��@"
								break;
							}
						}
					}
					else {
						// �ʐM�ُ풆�́u���ڑ��v��\������
						grachr( (ushort)(drawCount+2), 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[52] );		// [52] "���ڑ��@"
					}
				}
				else {
					displclr((ushort)(drawCount+2));				// ���u�������ꍇ�͍s���N���A����
				}
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�t���b�v���[�v�f�[�^�\��
//[]----------------------------------------------------------------------[]
///	@param[in]		volume	: volume
///	@attention		None
///	@author			Yamada
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/02/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
ushort	FunChk_FlapLoopData( void )
{
	T_FrmAnyData	FrmChkData;
	ulong	ulData;
	ushort	i;
	ushort	msg;
	short	tnoIndex[LOCK_IF_MAX]={0};
	uchar	type;
	ushort	index = 0;
	ushort	count = 0;
	uchar	timeout = 1;
	uchar	priend = 0;
	
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[0]);		// [00] "�����[�v�f�[�^���@�@�@�@�@�@�@"
	grachr(3, 0, 30, 0 ,COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, FCSTR1[10]);	// [10]	"�@�@ ���΂炭���҂������� �@�@"
	Fun_Dsp(FUNMSG[0]);														/* [0]	"�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
	
	for( i=INT_CAR_START_INDEX; i<TOTAL_CAR_LOCK_MAX; i++ ){
		WACDOG;											// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		if( (LockInfo[i].if_oya != 0) && (LockInfo[i].lok_syu != 0) ){	// �^�[�~�i��No��v���ڑ�����H
			tnoIndex[count] = i;
			++count;
		}
	}

	if( !(GetCarInfoParam() & 0x04) || count == 0 ){	// �t���b�v�ݒ�Ȃ�
		BUZPIPI();
		return MOD_EXT;
	}
	
	type = _MTYPE_INT_FLAP;
	queset( FLPTCBNO, LK_SND_A_LOOP_DATA, 1, &type );	// ���[�v�f�[�^�v�����M
	Ope_DisableDoorKnobChime();
	
	Lagtim( OPETCBNO, 6, 6*50 );						// ��ϰ6(6s)�N��(����ݽ����p)
	
	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
			return MOD_CHG;

		case KEY_TEN_F5:	/* "�I��" */
			if(timeout == 1){
				break;
			}
			BUZPI();
			Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
			return MOD_EXT;


		case KEY_TEN_F3:	// F3:"�����"
			if(timeout == 1){
				break;
			}
			if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
				BUZPIPI();
				break;
			}
			BUZPI();
			FrmChkData.Kikai_no = (uchar)CPrmSS[S_PAY][2];
			FrmChkData.prn_kind = R_PRI;
			queset( PRNTCBNO, PREQ_LOOP_DATA, sizeof(FrmChkData), &FrmChkData);
			Ope_DisableDoorKnobChime();
			Fun_Dsp(FUNMSG[82]);		// F���\���@"�@�@�@�@�@�@ ���~ �@�@�@�@�@�@"
			
			for ( priend = 0 ; priend == 0 ; ) {
				msg = StoF( GetMessage(), 1 );
				if( (msg&0xff00) == (INNJI_ENDMASK|0x0600) ){
					msg &= (~INNJI_ENDMASK);
				}
				switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case LCD_DISCONNECT:
					Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
					return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case KEY_MODECHG:		// Ӱ����ݼ�
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					Lagcan( OPETCBNO, 6 );										// ��ϰ6��ݾ�
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
					BUZPI();
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//					return 1;
					return MOD_CHG;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
				case PREQ_LOOP_DATA:	// �󎚏I��
					priend = 1;
					break;
				case KEY_TEN_F3:		// F3���i���~�j
					BUZPI();
					FrmChkData.prn_kind = R_PRI;
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(FrmChkData), &FrmChkData );	// �󎚒��~�v��
					priend = 1;
					break;
				default:
					break;
				}
			}
			Fun_Dsp( FUNMSG[104] );						// [104]"  �{  �@�|�@ ����� �@�@  �I�� "
			break;
			
		case KEY_TEN_F4:	// F4:
			break;

		case KEY_TEN_F1:	// F1:"�{"
		case KEY_TEN_F2:	// F2:"�|"
			if(timeout == 1){
				break;
			}
		case TIMEOUT6:		// ��ϰ6��ѱ��(����ݽ����p)
			if(msg == KEY_TEN_F2) {
				BUZPI();
				if(index < count - 1) {
					++index;
				}
				else {
					index = 0;			// �擪�ɖ߂�
				}
			}
			else if(msg == KEY_TEN_F1) {
				BUZPI();
				if(index != 0) {
					--index;
				}
				else {
					index = count - 1;	// �����ɖ߂�
				}
			}
			else if(msg == TIMEOUT6) { 
				timeout = 0;
				index = 0;
				Fun_Dsp( FUNMSG[104] );						// [104]"  �{  �@�|�@ ����� �@�@  �I�� "
			}

			grachr( 1, 0, 10, 0, COLOR_BLACK, LCD_BLINK_OFF, CARFAIL[1] );			// [01]	"�@�Ԏ��ԍ��F�@�@�@�@�@�@�@�@�@"
			opedsp( 1, 10, (ushort)LockInfo[tnoIndex[index]].posi, 4, 1, 0,
												COLOR_BLACK, LCD_BLINK_OFF );		// �Ԏ��ԍ��\��

			// �e�f�[�^�̃^�C�g��������\��
			grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[3]);
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[4]);
			grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[5]);
			grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[6]);
			grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CRRBOARDSTR[7]);

			// �e�f�[�^�̒l��\��
			// '0'�`'9' 'A'�`'F'�ȊO�������"��������"��\������
			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucLoopCount, 4, &ulData)) {
				opedsp5(2, 22, (ushort)ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// ���[�v�J�E���^�\��
			}
			else {
				grachr(2, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);				// "��������"
			}

			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucOffBaseCount, 4, &ulData)) {
				opedsp5(3, 22, (ushort)ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �I�t�x�[�X�\��
			}
			else {
				grachr(3, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);				// "��������"
			}
				
			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucOffLevelCount, 4, &ulData)) {
				opedsp5(4, 22, ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �I�t���x���\��
			}
			else {
				grachr(4, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);				// "��������"
			}
			
			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucOnBaseCount, 4, &ulData)) {
				opedsp5(5, 22, ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �I���x�[�X�\��
			}
			else {
				grachr(5, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);				// "��������"
			}
			
			if(1 == HexAsc_to_LongBin_withCheck(IFM_LockTable.sSlave[LockInfo[tnoIndex[index]].if_oya - 1].tLoopCount.ucOnLevelCount, 4, &ulData)) {
				opedsp5(6, 22, ulData, 4, 1, 0, COLOR_BLACK, LCD_BLINK_OFF );		// �I�����x���\��
			}
			else {
				grachr(6, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[21]);		// "��������"
			}

			break;

		default:
			break;
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			�f�o�b�O�p���j���[
//[]----------------------------------------------------------------------[]
///	@return			ret
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/02/08<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	FucChk_DebugMenu(void)
{
	unsigned short	usFbruEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;

	for ( ; ; ) {
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, MNT_STRCHK_STR[1] );		/* 	"���f�o�b�O���j���[���@�@�@�@�@" */

		usFbruEvent = Menu_Slt( MNT_STRCHK_STR, MNT_STRCHK_TBL, (char)MNT_STRCHK_MAX, (char)1 );

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (usFbruEvent) {
		case MNTLOG1_CHK:						/* �ݒ聕�X�e�[�^�X�v�����g */
			if (Ope_isPrinterReady() == 0) {
				BUZPIPI();
				break;
			}
			usFbruEvent = Fchk_Stack_StsPrint();
			break;
		case MOD_EXT:
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return usFbruEvent;
			break;
		default:
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if (usFbruEvent == MOD_CHG) {
		if (usFbruEvent == MOD_CHG || usFbruEvent == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			return usFbruEvent;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			�X�^�b�N�g�p�ʃv�����g
//[]----------------------------------------------------------------------[]
///	@return			ret
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
unsigned short	Fchk_Stack_StsPrint( void )
{
	T_FrmLogPriReq1	MntStak_PriReq;		// �󎚗v��ү����ܰ�
	T_FrmPrnStop	MntStak_PrnStop;	// �󎚒��~�v��ү����ܰ�
	ushort			pri_cmd = 0;		// �󎚗v������ފi�[ܰ�
	ushort	msg;

	dispclr();									// ������ʁi�󎚒��j�\��
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,MNT_STRCHK_STR[2]);	/* "���ݒ聕�X�e�[�^�X�v�����g��  " */
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)�̂��߂̒�`�ړ�
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,EDY_STSPRINT_STR[2]);	/* "�@�@�@�@ �󎚒��ł�         �@" */
//	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,EDY_STSPRINT_STR[3]);	/* "�@�@���~�F�󎚂𒆎~���܂��@�@" */
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,MNT_STRCHK_STR[3]);	/* "�@�@�@�@ �󎚒��ł�         �@" */
	grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,MNT_STRCHK_STR[4]);	/* "�@�@���~�F�󎚂𒆎~���܂��@�@" */
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)�̂��߂̒�`�ړ�
	Fun_Dsp(FUNMSG[82]);						/* "�@�@�@�@�@�@ ���~ �@�@�@�@�@�@" */
	
	Fchk_Stak_chek();
	
	MntStak_PriReq.prn_kind = R_PRI;						// �Ώ�������Fڼ��
	MntStak_PriReq.Kikai_no = (ushort)CPrmSS[S_PAY][2];		// �@�B��	�F�ݒ��ް�
	MntStak_PriReq.Kakari_no = OPECTL.Kakari_Num;			// �W��No.
	memcpy( &MntStak_PriReq.PriTime, &CLK_REC, sizeof( date_time_rec ) );// �󎚎���	�F���ݎ���
	
	/* �󎚗v��*/
	queset( PRNTCBNO, PREQ_MNT_STACK, sizeof(T_FrmLogPriReq1), &MntStak_PriReq );
	Ope_DisableDoorKnobChime();

	pri_cmd = PREQ_MNT_STACK;							// ���M����޾���
	
	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		if( pri_cmd == 0 ){		// �󎚊J�n�O
			switch( msg ){		/* FunctionKey Enter */

				case KEY_TEN_F3:	/* "���~" */
					BUZPI();
					MntStak_PrnStop.prn_kind = R_PRI;			// �󎚒��~ү���ޑ��M
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &MntStak_PrnStop );
					return MOD_EXT;
					
				default:
					break;
			}
		}
		else{		// �󎚊J�n��i�󎚏I���҂���ʁj
			switch( msg ){		/* FunctionKey Enter */
				case KEY_TEN_F3:	/* "���~" */
					BUZPI();
					MntStak_PrnStop.prn_kind = R_PRI;				// �󎚒��~ү���ޑ��M
					queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &MntStak_PrnStop );
					break;
			}

			if( msg == ( pri_cmd | INNJI_ENDMASK ) ){			// �󎚏I��ү���ގ�M�H
				return MOD_EXT;									// YES�F�O��ʂɖ߂�
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�X�^�b�N�g�p�ʎZ�o
//[]----------------------------------------------------------------------[]
///	@return			ret
//[]----------------------------------------------------------------------[]
///	@author			sekiguchi
///	@date			Create	: 2010/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	Fchk_Stak_chek( void )
{
	
	unsigned short	i,j;				// ٰ�ߏ��������
	unsigned char	*chk_adr;			// ������ذ���ڽ
	unsigned char	*chk_adr_s;			// ������ذ���ı��ڽ
	unsigned char	chk_stop = 0;		// ��������
	unsigned char	chk_data1;			// �����ް��P

	uchar			stak_not = 0xED;	// ��r�p�f�[�^

	chk_adr_s = (unsigned char *)xSTK_RANGE_BTM;
	
	// STACK�`�F�b�N
	for(j = 0;j<TSKMAX;j++){						// �^�X�N�T�C�Y�����[�v
		chk_adr_s -= xSTACK_TBL[j];					// �X�^�[�g�A�h���X�Z�b�g
		chk_adr = chk_adr_s;
		for(i = 0 ; i < xSTACK_TBL[j] ; i++){		// �X�^�b�N�T�C�Y�����[�v
			WACDOG;									// WATCHDOG��ϰؾ��
			
			_di();									// �����݋֎~
			
			chk_data1 = *chk_adr;					// �f�[�^�Z�b�g
			
			if(chk_data1 != stak_not){				// !0xED
				Pri_Tasck[j][0] = (ushort)(xSTACK_TBL[j]-i);				// �X�^�b�N�g�p�ʂ̌v�Z
				Pri_Tasck[j][1] = (ushort)(Pri_Tasck[j][0]/(xSTACK_TBL[j]/100));			// �X�^�b�N�g�p�ʂ̌v�Z�i���j
				chk_stop = 1;
			}
			chk_adr++;
			
			_ei();									// �����݋֎~����
			
			if(chk_stop == 1){						// �X�^�b�N�g�p�ʂ̌v�Z�I���ׁ̈A���̃X�^�b�N�v�Z��
				chk_stop = 0;
				break;
			}
		}
	}
	// ���Z�A�W�v�f�[�^�G���A�T�C�Y
	Pri_Pay_Syu[0][0] = sizeof(Receipt_data);		// ���Z�f�[�^�T�C�Y�i1���j
	Pri_Pay_Syu[1][0] = sizeof(Syu_log);			// �W�v�f�[�^�T�C�Y�i1���j
	Pri_Pay_Syu[0][1] = ( (LogDatMax[eLOG_PAYMENT][0]*LOG_SECORNUM(eLOG_PAYMENT)) +					// ���Z���O�T�C�Y(RAM + FLASH(2MByte))
						  (((2000*1024)/LogDatMax[eLOG_PAYMENT][0])*LogDatMax[eLOG_PAYMENT][0]) ); 
	Pri_Pay_Syu[1][1] = ( (LogDatMax[eLOG_TTOTAL][0]*LOG_SECORNUM(eLOG_TTOTAL)) +					// �W�v���O�T�C�Y(RAM + FLASH(384KByte)) 
						  (((384*1024)/LogDatMax[eLOG_TTOTAL][0])*LogDatMax[eLOG_TTOTAL][0]) );		
	Pri_program = AUTO_PAYMENT_PROGRAM;				// �f�o�b�O�p�v���O�������[�h
	
}
//[]----------------------------------------------------------------------[]
///	@brief			FAN����`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/05/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	FunChk_FUNOPECHK( void )
{
	ushort	msg;
	ushort	ret			= 0;											// �߂�l
	uchar state = 0; //0:OFF 1:ON
	dispclr();															// ��ʸر
	
	CP_FAN_CTRL_SW = state;

	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNOPECHKSTR[0] );	// "���e�`�m����`�F�b�N���@�@�@�@"
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FUNOPECHKSTR[1] );	// "  ���݂̏�� ��               "
	grachr( 2, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[0] );// OFF

	Fun_Dsp( FUNMSG[50] );												// "�@�@�@�@�@�@�@�@�@ON/OFF �I�� ", // [50]

	for( ; ret == 0 ; ){

		msg = StoF( GetMessage(), 1 );									// ү���ގ�M

		switch( msg ){													// ��Mү����
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				ret = MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:											// ����Ӱ�ސؑ�
				BUZPI();
				ret = MOD_CHG;
				break;

			case KEY_TEN_F5:											// �e�T�i�I���j
				BUZPI();
				ret = MOD_EXT;
				break;

			case KEY_TEN_F4:											// �e�S
				BUZPI();
				if(state == 0){											// OFF
					state = 1;// ON��
				}else{													// ON
					state = 0;// OFFF��
				}
				grachr( 2, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, (unsigned char *)DAT4_1[state] );// OFF
				CP_FAN_CTRL_SW = state;
				break;

			default:													// ���̑�
				break;
		}
	}
	if(f_fan_timectrl == 1){											// FAN������~��
		CP_FAN_CTRL_SW = 0;												// ���̏�Ԃ͒�~
	}else{
		CP_FAN_CTRL_SW = fan_exe_state;									// �ʏ퐧�䎞�̏�Ԃɖ߂�
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Park i PRO�`�F�b�N����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CCom( void )                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_PiP( void )
{
	unsigned short	fncChkPiPEvent;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];

	// Park i PRO�L���`�F�b�N�H
	if (_is_not_pip()) {
		BUZPIPI();
		return MOD_EXT;
	}

	DP_CP[0] = DP_CP[1] = 0;
	for ( ; ; ) {

		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF,  FCSTR1[47] );		/* "��Park i PRO�`�F�b�N���@�@�@�@" */
		fncChkPiPEvent = Menu_Slt( PIPMENU, PIP_CHK_TBL, PIP_CHK_MAX, 1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (fncChkPiPEvent) {

		case PIP_DTCLR:		// �f�[�^�N���A
			fncChkPiPEvent = FncChk_pipclr();
			break;

		case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return fncChkPiPEvent;

		default:
			break;
		}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if (fncChkPiPEvent == MOD_CHG) {
		if ( fncChkPiPEvent == MOD_CHG || fncChkPiPEvent == MOD_CUT ) {
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return fncChkPiPEvent;
		}

		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Park i PRO�`�F�b�N�����@�f�[�^�N���A		                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_pipclr( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort	FncChk_pipclr(void)
{
	ushort msg;
	dispmlclr(1, 2);
	grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX] );	// "�S"
	grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[0] );			// "�f�[�^���������܂�"
	grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );			// "�@�@�@�@��낵���ł����H�@�@�@"
	Fun_Dsp(FUNMSG[19]);						// "�@�@�@�@�@�@ �͂� �������@�@�@"

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			return MOD_CHG;

		case KEY_TEN_F3:		// �u�͂��v
			IFM_RcdBufClrAll();		// �f�[�^�e�[�u���̏�����
			/* not break; */
		case KEY_TEN_F4:		// �u�������v
			BUZPI();
			return MOD_EXT;
			
		default:
			break;
		}
	}
}
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
///*[]----------------------------------------------------------------------[]*/
///*| �N���W�b�g�`�F�b�N����                                                 |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : FncChk_Cre( void )                                      |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : unsigned short                                          |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//ushort FncChk_Cre( void )
//{
//	unsigned short	fncChkCreEvent;
//	char	wk[2];
//	char	org[2];
//
//	org[0] = DP_CP[0];
//	org[1] = DP_CP[1];
//
//	// �N���W�b�g�L���H
//	if( ! CREDIT_ENABLED() ){
//		BUZPIPI();
//		return MOD_EXT;
//	}
//
//	DP_CP[0] = DP_CP[1] = 0;
//	for ( ; ; ) {
//
//		dispclr();
//		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[56] );		// "���N���W�b�g�`�F�b�N���@�@�@�@"
//
//		fncChkCreEvent = Menu_Slt( CREMENU, CRE_CHK_TBL, (char)CRE_CHK_MAX, (char)1 );
//		wk[0] = DP_CP[0];
//		wk[1] = DP_CP[1];
//
//		switch (fncChkCreEvent) {
//		case CREDIT_UNSEND:
//			fncChkCreEvent = FncChk_CreUnSend();	// �����M����f�[�^
//			break;
//
//		case CREDIT_SALENG:
//			fncChkCreEvent = FncChk_CreSaleNG();	// ���㋑�ۃf�[�^
//			break;
//
//		case MOD_EXT:								// �I���i�e�T�j
//			DP_CP[0] = org[0];
//			DP_CP[1] = org[1];
//			return fncChkCreEvent;
//
//		default:
//			break;
//		}
//
//		if (fncChkCreEvent == MOD_CHG){				// ���[�h�`�F���W
//			OPECTL.Mnt_mod = 0;
//			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//			OPECTL.Mnt_lev = (char)-1;
//			OPECTL.PasswordLevel = (char)-1;
//			return fncChkCreEvent;
//		}
//		DP_CP[0] = wk[0];
//		DP_CP[1] = wk[1];
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F�g���@�\-�N���W�b�g�����M����˗��f�[�^         |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : FncChk_CreUnSend(void)                                  |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static ushort	FncChk_CreUnSend( void )
//{
//	ushort	msg;
//	uchar	mode;
//	ushort	rev = 0;	//���]�\���p
//	short	ret;
//	char	repaint;	// ��ʍĕ`��t���O
//	T_FrmUnSendPriReq	unsend_pri;
//	struct	DATA_BK	cre_uriage_inj_iwork;
//	char	print_flg = 0;
//	uchar	key_flg;	// �e�L�[�F 0=�����A1=�L��
//
//	repaint = 1;
//	mode = 0;
//	if( cre_uriage.UmuFlag == ON ){						// �����M�f�[�^����
//		key_flg = 1;
//	}else{
//		key_flg = 0;
//	}
//
//	for ( ; ; ) {
//
//		if (repaint) {
//			dispclr();
//			grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[160]);	/* "�������M����f�[�^���@�@�@�@�@" */
//			if( cre_uriage.UmuFlag == ON ){						// �����M�f�[�^����
//				Fun_Dsp(FUNMSG2[59]);							/* " �폜 �@�@�@ ����� �đ�  �I�� " */
//				if( mode == 0 ){
//					cre_unsend_data_dsp( &cre_uriage.back );	//�f�[�^�\��
//				}
//			}
//			else {												// �����M�f�[�^�Ȃ�
//				Fun_Dsp(FUNMSG[8]);								// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
//				grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	// "�@�@�@�f�[�^�͂���܂���@�@�@"
//			}
//			repaint = 0;
//		}
//		msg = StoF(GetMessage(), 1);
//
//		/* ���M�O�`��M���� */
//		if( mode < 2 ){
//			switch (msg) {
//			case KEY_MODECHG:			// Ӱ����ݼ�
//				BUZPI();
//				Lagcan(OPETCBNO, 7);
//				return MOD_CHG;
//
//			case KEY_TEN_F1:			/* F1:�폜 */
//				if( key_flg == 0 ){
//					break;
//				}
//				if( mode == 1 ){		// ��ʂ���̑��M��
//					break;
//				}
//				
//				if(cre_ctl.Status != CRE_STS_IDLE) {	// �đ��^�C�}�ɂ�著�M��
//					BUZPIPI();
//					displclr(3);
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "�@�@�@�@�@�ُ�I���@�@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[86]);		/* "�@�@���݃N���W�b�g�������B�@�@" */
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[87]);		/* "�@�r�W�[�̂��ߎ��s�ł��܂���B" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;
//					break;
//				}
//
//				// �폜
//				BUZPI();
//				ret = cre_unsend_del_dsp(print_flg);
//				switch( ret ){
//				case  1:	// ���[�h�`�F���W
//					Lagcan(OPETCBNO, 7);
//					return( MOD_CHG );
//				case -1:	// �͂�
//					Lagcan(OPETCBNO, 7);
//					return MOD_EXT;
//				case  0:	// ������
//				default:
//					break;
//				}
//				repaint = 1;
//				break;
//
//			case KEY_TEN_F3:			// �v�����g
//				if( mode == 1 ){
//					break;
//				}
//				if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
//					BUZPIPI();
//					break;
//				}
//				if( cre_uriage.UmuFlag == OFF ) {		// �����M�f�[�^�Ȃ�
//					if( key_flg ){
//						BUZPIPI();
//						repaint = 1;
//						key_flg = 0;
//					}
//					break;
//				}
//				BUZPI();
//				unsend_pri.prn_kind = R_PRI;
//				unsend_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &unsend_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );				// �󎚎���	�F���ݎ���
//				memcpy( &cre_uriage_inj_iwork, &cre_uriage.back, sizeof( cre_uriage.back ) );		// ����˗��ް�
//				unsend_pri.back = &cre_uriage_inj_iwork;
//				queset( PRNTCBNO, PREQ_CREDIT_UNSEND, sizeof(T_FrmUnSendPriReq), &unsend_pri );	// �󎚗v��
//
//				print_flg = 1;							// �󎚃t���OON(�󎚂ł��Ȃ������ꍇ�ł��󎚂������Ƃɂ���)
//				wopelg( OPLOG_CRE_CAN_PRN,0,0 );			// ���엚��o�^
//
//				/* �v�����g�I����҂����킹�� */
//				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
//				for ( ; ; ) {
//					msg = StoF( GetMessage(), 1 );
//					/* �v�����g�I�� */
//					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
//						Lagcan(OPETCBNO, 6);
//						break;
//					}
//					/* �^�C���A�E�g(10�b)���o */
//					if (msg == TIMEOUT6) {
//						BUZPIPI();
//						break;
//					}
//					/* ���[�h�`�F���W */
//					if (msg == KEY_MODECHG) {
//						BUZPI();
//						Lagcan(OPETCBNO, 6);
//						return MOD_CHG;
//					}
//				}
//				repaint = 1;
//				break;
//
//			case KEY_TEN_F4:			// ���M
//				if( key_flg == 0 ){
//					break;
//				}
//				if( cre_uriage.UmuFlag == OFF ) {		// �����M�f�[�^�Ȃ�
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);		/* "�@�@�@�@�@ ����I�� �@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[162]);		/* "�@�@�@ ���M�������܂��� �@�@�@" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;	// ���M�ς�
//					key_flg = 0;	// ���񂩂�̓L�[����
//					break;
//				}
//				if( mode == 1 ){
//					break;
//				}
//				BUZPI();
//				rev = 0;
//				grachr(6, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "�@�@���΂炭���҂��������@�@�@" */
//
//				cre_ctl.SalesKind = CRE_KIND_MANUAL;		// ���M�v���Ɏ蓮��ݒ�
//				if(cre_ctl.Initial_Connect_Done == 0) {		// ���J��?
//					cre_ctl.OpenKind = CRE_KIND_AUTO;		// �J�ǔ����v���Ɏ�������ݒ�
//					ret = creCtrl( CRE_EVT_SEND_OPEN );		// �J�ǃR�}���h���M
//				}
//				else {
//					ret = creSendData_SALES();				//����˗��f�[�^���M
//				}
//
//				if( ret == 0 ){								//�����t�Ȃ�
//					Lagtim(OPETCBNO, 7, 25);				//�����_�ŗp�^�C�}�[�Z�b�g(500msec)
//					wopelg( OPLOG_CRE_UNSEND_SEND, 0, 0 );	//�����M����đ�
//					mode = 1;	// ���M��
//					Fun_Dsp(FUNMSG[0]);								// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@ "
//				}else if( ret == 1 ){						//��t�s�i�����ɂ͗��Ȃ��j
//					BUZPIPI();
//					dispmlclr( 2, 6 );
//					grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[163]);		/* "�@�@�@ ���M���s���܂��� �@�@�@" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;	// ���M�ς�
//				}else if( ret == -2 ){						//�ʐM������
//					BUZPIPI();
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[79]);		/* "�@�@�@�@�@�ُ�I���@�@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[86]);		/* "�@�@���݃N���W�b�g�������B�@�@" */
//					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[87]);		/* "�@�r�W�[�̂��ߎ��s�ł��܂���B" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;	// ���M�ς�
//				}
//				break;
//
//			case KEY_TEN_F5:			// �I��
//				if( mode == 1 ){
//					break;
//				}
//				BUZPI();
//				Lagcan(OPETCBNO, 7);
//				return MOD_EXT;
//
//			case CRE_EVT_02_NG:			//�J�Ǐ����G���[
//				// no break
//			case CRE_EVT_06_OK:			//����
//				// no break
//			case CRE_EVT_06_NG:			//�T�[�o����G���[��M
//				if( mode != 1 ){
//					break;				// ���M���łȂ��ꍇ�͖�������
//				}
//				Lagcan(OPETCBNO, 7);
//				BUZPIPI();
//				dispmlclr( 2, 6 );
//				if(msg == CRE_EVT_02_NG || 
//					(msg == CRE_EVT_06_NG && ((DATA_KIND_137_06 *)cre_ctl.RcvData)->Result1 == 99)) {
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);		/* "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[163]);		/* "�@�@�@ ���M���s���܂��� �@�@�@" */
//				}
//				else {
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[27]);		/* "�@�@�@�@�@ ����I�� �@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[162]);		/* "�@�@�@ ���M�������܂��� �@�@�@" */
//				}
//				Fun_Dsp(FUNMSG[8]);														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//				mode = 2;	// ���M�ς�
//				break;
//
//			case TIMEOUT7:				// �����_��
//				rev ^= 1;
//				grachr(6, 0, 30, rev, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, UMSTR3[89]);	/* "�@�@���΂炭���҂��������@�@�@" */
//				if( rev ){
//					Lagtim(OPETCBNO, 7, 50);	// �����_�ŗp�^�C�}�[���X�^�[�g(���]=1sec)
//				}
//				else{
//					Lagtim(OPETCBNO, 7, 25);	// �����_�ŗp�^�C�}�[���X�^�[�g(�ʏ�=500msec)
//				}
//				break;
//			case TIMEOUT9:				// ��M�^�C���A�E�g
//				if( mode != 1 ){
//					break;				// ���M���łȂ��ꍇ�͖�������
//				}
//				if( cre_ctl.Status == CRE_STS_IDLE ){		// �����҂���ѱ��
//					Lagcan(OPETCBNO, 7);
//					BUZPIPI();
//					dispmlclr( 2, 6 );
//					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[28]);		/* "�@�@�@�@�@ �ُ�I�� �@�@�@�@�@" */
//					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[163]);		/* "�@�@�@ ���M���s���܂��� �@�@�@" */
//					Fun_Dsp(FUNMSG[8]);													/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
//					mode = 2;	// ���M�ς�
//				}
//				break;
//			default:
//				break;
//			}
//		}
//		// ���ʕ\����
//		else {	// mode >= 2
//			switch (msg) {
//			case KEY_MODECHG:		// Ӱ����ݼ�
//				BUZPI();
//				return MOD_CHG;
//			case KEY_TEN_F5:		// �I��
//				BUZPI();
//				return MOD_EXT;
//			default:
//				break;
//			}
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g�����M����˗��f�[�^ �f�[�^�\��                             |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_unsend_data_dsp                                     |*/
///*| PARAMETER    : index �\���Ώ�                                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static void cre_unsend_data_dsp(struct DATA_BK *data)
//{
//	date_time_rec	*time;
//
//	time = &data->time;
//	displclr(2);
//
//	//���Z��
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[90]);		// "���Z���@�@�F�@�@�@�@�@�@�@�@�@"
//	date_dsp3(2, 12, (ushort)(time->Year), (ushort)(time->Mon), (ushort)(time->Day), 0);
//
//	//���Z����
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[91]);		// "���Z�����@�F�@�@�@�@�@�@�@�@�@"
//	time_dsp2(3, 12, (unsigned short)time->Hour, (unsigned short)time->Min, 0);
//
//	//���Z���z
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[92]);		// "���Z���z�@�F�@�@�@�@�@�@�~�@�@"
//
//	if( data->ryo > 999999L ){							// 999999��������"******"
//		grachr(4, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[94]);	// "�@�@�@�@�@�@������������"
//	}else{
//		opedpl(4, 12, data->ryo, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	}
//
//	//�����
//	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[93]);		// "������@�@�@�@�@�@�@�@�@�@�@�@"
//	grachr(5, 8, CRE_SHOP_ACCOUNTBAN_MAX, 0, COLOR_BLACK, LCD_BLINK_OFF,
//							(const uchar *)&data->shop_account_no[0]);	// "12345678901234567890�@�@�@�@�@"
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g�����M����˗��f�[�^ �폜��ʕ\��                           |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_unsend_del_dsp                                      |*/
///*| RETURN VALUE : short  0:normal                                         |*/
///*| RETURN VALUE : short  1:mode change                                    |*/
///*| RETURN VALUE : short -1:delete ok                                      |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static short cre_unsend_del_dsp( char pri )
//{
//	ushort	msg;
//	T_FrmUnSendPriReq	unsend_pri;
//	struct	DATA_BK	cre_uriage_inj_iwork;
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[160]);	/* "�������M����f�[�^���@�@�@�@�@" */
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[60]);	// "�@�@�@�f�[�^���폜���܂��@�@�@"
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);	// "�@�@�@�@��낵���ł����H�@�@�@"
//	Fun_Dsp(FUNMSG[19]);											// "�@�@�@�@�@�@ �͂� �������@�@�@"
//
//	for ( ; ; ) {
//
//		msg = StoF(GetMessage(), 1);
//
//		switch (msg) {
//		case KEY_TEN_F3:			// F3:�͂�
//			BUZPI();
//
//			if( cre_uriage.UmuFlag == OFF ) {		// �����M�f�[�^�Ȃ�
//				return -1;
//			}
//			
//			// �폜�O�Ɉ󎚂���
//			if(pri == 0 && Ope_isPrinterReady()) {				// ���V�[�g�Ɉ󎚉\
//				memset(&unsend_pri, 0, sizeof(unsend_pri));
//				unsend_pri.prn_kind = R_PRI;
//				unsend_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &unsend_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );				// �󎚎���	�F���ݎ���
//				memcpy( &cre_uriage_inj_iwork, &cre_uriage.back, sizeof( cre_uriage.back ) );		// ����˗��ް�
//				unsend_pri.back = &cre_uriage_inj_iwork;
//				queset( PRNTCBNO, PREQ_CREDIT_UNSEND, sizeof(T_FrmUnSendPriReq), &unsend_pri );	// �󎚗v��
//
//				/* �v�����g�I����҂����킹�� */
//				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
//				for ( ; ; ) {
//					msg = StoF( GetMessage(), 1 );
//					/* �v�����g�I�� */
//					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
//						Lagcan(OPETCBNO, 6);
//						break;
//					}
//					/* �^�C���A�E�g(10�b)���o */
//					if (msg == TIMEOUT6) {
//						BUZPIPI();
//						break;
//					}
//					/* ���[�h�`�F���W */
//					if (msg == KEY_MODECHG) {
//						BUZPI();
//						Lagcan(OPETCBNO, 6);
//						return MOD_CHG;
//					}
//				}
//			}
//			
//			// �폜
//			cre_uriage.UmuFlag = OFF;						// ����˗��f�[�^(05)���M���׸�OFF
//			memset( &cre_uriage, 0, sizeof(cre_uriage));	// ����˗��f�[�^�G���A���N���A
//			creSales_Reset();								// �^�C�}�J�E���g�A���g���C�����폜����
//
//			if( creErrorCheck() == 0 ){
//				cre_ctl.Credit_Stop = 0;	// �N���W�b�g��~�v�����Ȃ��Ȃ������������
//			}
//
//			wopelg( OPLOG_CRE_UNSEND_DEL, 0, 0 );			// �����M����폜
//			return( -1 );
//		case KEY_TEN_F4:			// F4:������
//			BUZPI();
//			if( cre_uriage.UmuFlag == OFF ) {		// �����M�f�[�^�Ȃ�
//				return( -1 );
//			}
//			return( 0 );
//		case KEY_MODECHG:			// ���[�h�`�F���W
//			BUZPI();
//			return( 1 );
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F�g���@�\-�N���W�b�g���㋑�ۃf�[�^               |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : FncChk_CreSalesNG( void )                               |*/
///*| PARAMETER    : void                                                    |*/
///*| RETURN VALUE : MOD_CHG : mode change                                   |*/
///*|              : MOD_EXT : F5 key                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static ushort	FncChk_CreSaleNG( void )
//{
//	ushort	msg;
//	char	repaint;	// ��ʍĕ`��t���O
//	short	show_index;	// �\�����̃C���f�b�N�X
//	char	data_count;	// �X�V��̃f�[�^����
//	short	ret;
//	T_FrmSaleNGPriReq	saleng_pri;
//	char	print_flg = 0;
//
//	dispclr();									// ��ʃN���A
//
//	repaint = 1;
//	show_index = 0;
//	data_count = 0;
//
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[161]);	/* "�����㋑�ۃf�[�^���@" */
//
//	for ( ; ; ) {
//	
//		if (repaint) {
//			cre_saleng_show_data( show_index );	//�f�[�^�\��
//			data_count = cre_saleng.ken;
//			repaint = 0;
//		}
//		msg = StoF(GetMessage(), 1);
//
//		switch (msg) {
//		case KEY_TEN_F1:			/* F1:�� */
//			if (data_count > 1) {	// �f�[�^����2�ȏ�̎��L��
//				BUZPI();
//				if (--show_index < 0) {
//					show_index = cre_saleng.ken - 1;
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F2:			/* F2:�� */
//			if (data_count > 1) {	// �f�[�^����2�ȏ�̎��L��
//				BUZPI();
//				if (++show_index > cre_saleng.ken - 1) {
//					show_index = 0;
//				}
//				repaint = 1;
//			}
//			break;
//		case KEY_TEN_F3:			/* F3:�v�����g */
//			if (data_count != 0) {	// �f�[�^����1�ȏ�̎��L��
//				if(Ope_isPrinterReady() == 0){			// ���V�[�g�v�����^���󎚕s�\�ȏ��
//					BUZPIPI();
//					break;
//				}
//				BUZPI();
//				saleng_pri.prn_kind = R_PRI;
//				saleng_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &saleng_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );						// �󎚎���	�F���ݎ���
//				saleng_pri.ng_data = &cre_saleng;														// �󎚎���	�F���㋑���ް�
//				queset( PRNTCBNO, PREQ_CREDIT_SALENG, (char)sizeof(T_FrmSaleNGPriReq), &saleng_pri );	// �󎚗v��
//				print_flg = 1;							// �󎚃t���OON(�󎚂ł��Ȃ������ꍇ�ł��󎚂������Ƃɂ���)
//
//				/* �v�����g�I����҂����킹�� */
//				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
//				for ( ; ; ) {
//					msg = StoF( GetMessage(), 1 );
//					/* �v�����g�I�� */
//					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
//						Lagcan(OPETCBNO, 6);
//						break;
//					}
//					/* �^�C���A�E�g(10�b)���o */
//					if (msg == TIMEOUT6) {
//						BUZPIPI();
//						break;
//					}
//					/* ���[�h�`�F���W */
//					if (msg == KEY_MODECHG) {
//						BUZPI();
//						Lagcan(OPETCBNO, 6);
//						return MOD_CHG;
//					}
//				}
//			}
//			break;
//		case KEY_TEN_F4:			/* F4:�폜 */
//			if (data_count != 0) {	// �f�[�^����1�ȏ�̎��L��
//				// �폜
//				BUZPI();
//				ret = cre_saleng_del_dsp( show_index , print_flg);
//				if (ret == 1) {
//					return( MOD_CHG );
//				} else if (ret == -1) {
//					if (show_index >= cre_saleng.ken) {
//						--show_index;
//					}
//				}
//				repaint = 1;
//			}
//			break;
//
//		case KEY_TEN_F5:			/* F5:�I�� */
//			BUZPI();
//			return( MOD_EXT );
//		case KEY_MODECHG:	// ���[�h�`�F���W
//			BUZPI();
//			return( MOD_CHG );
//		default:
//			break;
//		}
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���[�U�[�����e�i���X�F�g���@�\-�N���W�b�g���㋑��-�t�@���N�V�����\��  |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : FncChk_CreSaleNG_fnc(char ken)                          |*/
///*| PARAMETER    : char ken : ����                                         |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//void	FncChk_CreSaleNG_fnc( char ken )
//{
//	if( ken > 1 ){	// 2���ȏ�
//		Fun_Dsp(FUNMSG2[30]);						// "�@���@�@���@ ����� �폜  �I�� "
//	} else {		// 2������
//		Fun_Dsp(FUNMSG2[31]);						// "�@�@�@�@�@�@ ����� �폜  �I�� "
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g���㋑�ۃf�[�^ �\��                                         |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_show_data                                    |*/
///*| PARAMETER    : index     : �\������e�[�u���ԍ�                        |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static void cre_saleng_show_data( short index )
//{
//	if (cre_saleng.ken != 0) {		// �f�[�^�L��
//		cre_saleng_num_dsp( (uchar)(index + 1), cre_saleng.ken );	// [ / ]
//		cre_saleng_data_dsp( index );
//		FncChk_CreSaleNG_fnc( cre_saleng.ken );
//	} else {						// �f�[�^����
//		cre_saleng_num_dsp( 0, 0 );
//		displclr(1);
//		displclr(2);
//		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[57]);	// "�@�@�@�f�[�^�͂���܂���@�@�@"
//		displclr(4);
//		displclr(5);
//		displclr(6);
//		Fun_Dsp(FUNMSG[8]);					// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
//	}
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g���㋑�ۃf�[�^ �����\��                                     |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_num_dsp( uchar numerator, char denominator ) |*/
///*| PARAMETER    : numerator   ���q                                        |*/
///*| PARAMETER    : denominator ����                                        |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static void cre_saleng_num_dsp( uchar numerator, char denominator )
//{
//	// �g��\��
//	grachr(0, 22, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DAT4_2[31]);	// [  /  ]
//
//	// ���q��\��
//	opedsp3(0, 24, (unsigned short)numerator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//
//	// �����\��
//	opedsp3(0, 27, (unsigned short)denominator, 2, 1, 0, COLOR_BLACK, LCD_BLINK_OFF);
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���ԕ\��  �S�p                                                        |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : time_dsp2                                               |*/
///*| PARAMETER    : ushort low  : �s                                        |*/
///*| PARAMETER    : ushort col  : ��                                        |*/
///*| PARAMETER    : ushort hour : ��                                        |*/
///*| PARAMETER    : ushort min  : ��                                        |*/
///*|              : ushort rev  : 0:���] 1:���]                             |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
//static void	time_dsp2(ushort low, ushort col, ushort hour, ushort min, ushort rev)
//{
//	opedsp(low, col, hour, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* �� */
//	grachr(low, (ushort)(col+4), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[1]);		/* ":" */
//	opedsp(low, (ushort)(col+6), min , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);			/* �� */
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g���㋑�ۃf�[�^ �f�[�^�\��                                   |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_data_dsp                                     |*/
///*| PARAMETER    : index �\���Ώ�                                          |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//static void cre_saleng_data_dsp( short index )
//{
//	struct DATA_BK	*data;
//	date_time_rec	*time;
//
//	data = &cre_saleng.back[ index ];
//	time = &data->time;
//	displclr(2);
//
//	//���Z��
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[90]);		// "���Z���@�@�F�@�@�@�@�@�@�@�@�@"
//	date_dsp3(2, 12, (ushort)(time->Year), (ushort)(time->Mon), (ushort)(time->Day), 0);
//
//	//���Z����
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[91]);		// "���Z�����@�F�@�@�@�@�@�@�@�@�@"
//	time_dsp2(3, 12, (unsigned short)time->Hour, (unsigned short)time->Min, 0);
//
//	//���Z���z
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[92]);		// "���Z���z�@�F�@�@�@�@�@�@�~�@�@"
//
//	if( data->ryo > 999999L ){							// 999999��������"******"
//		grachr(4, 12, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[94]);	// "�@�@�@�@�@�@������������"
//	}else{
//		opedpl(4, 12, data->ryo, 6, 0, 0, COLOR_BLACK, LCD_BLINK_OFF);
//	}
//
//	//�����
//	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[93]);		// "������@�@�@�@�@�@�@�@�@�@�@�@"
//	grachr(5, 8, CRE_SHOP_ACCOUNTBAN_MAX, 0, COLOR_BLACK, LCD_BLINK_OFF,
//							(const uchar *)&data->shop_account_no[0]);	// "12345678901234567890�@�@�@�@�@"
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  ���t�\��  �S�p (�����N�^�������^������)                               |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : date_dsp3                                               |*/
///*| PARAMETER    : ushort low  : �s                                        |*/
///*| PARAMETER    : ushort col  : ��                                        |*/
///*| PARAMETER    : ushort hour : ��                                        |*/
///*| PARAMETER    : ushort min  : ��                                        |*/
///*|              : ushort rev  : 0:���] 1:���]                             |*/
///*| RETURN VALUE : void                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static void	date_dsp3(ushort low, ushort col, ushort year, ushort month, ushort day, ushort rev)
//{
//
//	opedsp(low, col  , year, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);						/* �N */
//	grachr(low, (unsigned short)(col+4), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[7]);	/* "�^" */
//	opedsp(low, (unsigned short)(col+6)  , month, 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);	/* �� */
//	grachr(low, (unsigned short)(col+10), 2, rev, COLOR_BLACK, LCD_BLINK_OFF, DAT1_1[7]);	/* "�^" */
//	opedsp(low, (unsigned short)(col+12), day , 2, 1, rev, COLOR_BLACK, LCD_BLINK_OFF);		/* �� */
//}
//
///*[]----------------------------------------------------------------------[]*/
///*|  �N���W�b�g���㋑�ۃf�[�^ �폜��ʕ\��                                 |*/
///*[]----------------------------------------------------------------------[]*/
///*| MODULE NAME  : cre_saleng_del_dsp                                      |*/
///*| PARAMETER    : index �폜�Ώ�                                          |*/
///*| RETURN VALUE : short  0:normal                                         |*/
///*| RETURN VALUE : short  1:mode change                                    |*/
///*| RETURN VALUE : short -1:delete ok                                      |*/
///*[]----------------------------------------------------------------------[]*/
///*| Date         : 2006-07-03 M.Yanase(COSMO)                              |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
//static short cre_saleng_del_dsp( short index, char pri)
//{
//	ushort	msg;
//	T_FrmSaleNGPriReq	saleng_pri;
//
//	dispclr();
//	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[161]);	// "���N���W�b�g���㋑�ۃf�[�^���@"
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[60]);	// "�@�@�@�f�[�^���폜���܂��@�@�@"
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21]);	// "�@�@�@�@��낵���ł����H�@�@�@"
//	Fun_Dsp(FUNMSG[19]);											// "�@�@�@�@�@�@ �͂� �������@�@�@"
//
//	for ( ; ; ) {
//
//		msg = StoF(GetMessage(), 1);
//
//		switch (msg) {
//		case KEY_TEN_F3:			// F3:�͂�
//			BUZPI();
//
//			// �폜���Ɉ󎚂���
//			if(pri == 0 && Ope_isPrinterReady()) {	// ���V�[�g�Ɉ󎚉\
//				saleng_pri.prn_kind = R_PRI;
//				saleng_pri.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//				memcpy( &saleng_pri.PriTime, &CLK_REC, sizeof( date_time_rec ) );						// �󎚎���	�F���ݎ���
//				saleng_pri.ng_data = &cre_saleng;														// �󎚎���	�F���㋑���ް�
//				queset( PRNTCBNO, PREQ_CREDIT_SALENG, (char)sizeof(T_FrmSaleNGPriReq), &saleng_pri );	// �󎚗v��
//
//				/* �v�����g�I����҂����킹�� */
//				Lagtim(OPETCBNO, 6, 1000);		/* 20sec timer start */
//				for ( ; ; ) {
//					msg = StoF( GetMessage(), 1 );
//					/* �v�����g�I�� */
//					if((msg&0xff00) == (INNJI_ENDMASK|0x0600)) {
//						Lagcan(OPETCBNO, 6);
//						break;
//					}
//					/* �^�C���A�E�g(10�b)���o */
//					if (msg == TIMEOUT6) {
//						BUZPIPI();
//						break;
//					}
//					/* ���[�h�`�F���W */
//					if (msg == KEY_MODECHG) {
//						BUZPI();
//						Lagcan(OPETCBNO, 6);
//						return MOD_CHG;
//					}
//				}
//			}
//
//			// �S���폜
//			CRE_SaleNG_Delete();
//			if( cre_saleng.ken == 0 ){	// �폜�� 0�� �Ȃ�G���[����
//				err_chk2( ERRMDL_CREDIT, ERR_CREDIT_NO_ACCEPT, 0x00, 0, 0, NULL );	// �G���[(����)
//			}
//			cre_ctl.Credit_Stop = 0;	// �N���W�b�g��~�v�����Ȃ��Ȃ������������
//			wopelg( OPLOG_CRE_SALE_NG_DEL, 0, 0 );		// ���엚��o�^
//			displclr(2);
//			displclr(3);
//			return( -1 );
//		case KEY_TEN_F4:			// F4:������
//			BUZPI();
//			displclr(2);
//			displclr(3);
//			return( 0 );
//		case KEY_MODECHG:			// ���[�h�`�F���W
//			BUZPI();
//			return( 1 );
//		default:
//			break;
//		}
//	}
//}
//
///*[]-----------------------------------------------------------------------[]*/
///*|  �N���W�b�g���㋑�ۃf�[�^  �f�[�^�폜                                   |*/
///*[]-----------------------------------------------------------------------[]*/
///*| MODULE NAME  : CRE_SaleNG_Delete                                        |*/
///*| PARAMETER    : none                                                     |*/
///*| RETURN VALUE : none                                                     |*/
///*[]-----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                               |*/
///*| Update       :                                                          |*/
///*[]-------------------------------------  Copyright(C) 2013 AMANO Corp.---[]*/
//static void	CRE_SaleNG_Delete( void )
//{
//	memset(&cre_saleng_work, 0, sizeof(CRE_SALENG));
//
//	nmisave(&cre_saleng, &cre_saleng_work, sizeof(CRE_SALENG));
//}
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)

// GG120600(S)
/*[]----------------------------------------------------------------------[]*/
/*|  ���[�U�[�����e�i���X�F���u�Ď��f�[�^��ʕ\��                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  :dsp_time_info( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_CHG : mode change                                   |*/
/*|              : MOD_EXT : F5 key                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
// GG120600(E)
static void dsp_time_info(uchar type)
{
	ushort col = 2;
	t_remote_time_info *pTimeInfo;
	uchar  tmpStr[12];

	if (type >= TIME_INFO_MAX) {
		return;
	}
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[9+type] );	// �^�C�g��

	// ���������擾
	pTimeInfo = remotedl_time_info_get(type);

	switch (type) {
	case PROG_DL_TIME:
	case PARAM_DL_TIME:
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case PARAM_DL_DIF_TIME:		// �����p�����[�^�_�E�����[�h
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)	
// MH322915(S) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
//		if (!remote_date_chk(&pTimeInfo->start_time) || !remote_date_chk(&pTimeInfo->sw_time)) {
		if (!Check_date_time_rec(&pTimeInfo->start_time) || !Check_date_time_rec(&pTimeInfo->sw_time)) {
// MH322915(E) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
			grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[3] );	// "�\��͂���܂���@�@�@�@�@�@�@"
			return;
		}

		// �J�n����
		sprintf((char*)tmpStr, "%02d%02d%02d%02d%02d",
			pTimeInfo->start_time.Year%100, pTimeInfo->start_time.Mon, pTimeInfo->start_time.Day,
			pTimeInfo->start_time.Hour, pTimeInfo->start_time.Min);
		grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[0] );		// "�J�n�����F�@�N�@���@���@�F�@�@"
		grachr( col, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[0] );		// �N
		grachr( col, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[2] );		// ��
		grachr( col, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[4] );		// ��
		grachr( col, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[6] );		// ��
		grachr( col, 26,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[8] );		// ��
		grachr( col, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR7[pTimeInfo->status[INFO_KIND_START]] );	// �����X�e�[�^�X
		col++;

		// �X�V����
		sprintf((char*)tmpStr, "%02d%02d%02d%02d%02d",
			pTimeInfo->sw_time.Year%100, pTimeInfo->sw_time.Mon, pTimeInfo->sw_time.Day,
			pTimeInfo->sw_time.Hour, pTimeInfo->sw_time.Min);
		grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[1] );		// "�X�V�����F�@�N�@���@���@�F�@�@"
		grachr( col, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[0] );		// �N
		grachr( col, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[2] );		// ��
		grachr( col, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[4] );		// ��
		grachr( col, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[6] );		// ��
		grachr( col, 26,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[8] );		// ��
		grachr( col, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR7[pTimeInfo->status[INFO_KIND_SW]] );	// �����X�e�[�^�X
		col++;
		break;
	case PARAM_UP_TIME:
	case RESET_TIME:
// MH322915(S) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
//		if (!remote_date_chk(&pTimeInfo->start_time)) {
		if (!Check_date_time_rec(&pTimeInfo->start_time)) {
// MH322915(E) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
			grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[3] );	// "�\��͂���܂���@�@�@�@�@�@�@"
			return;
		}

		// �J�n����
		sprintf((char*)tmpStr, "%02d%02d%02d%02d%02d",
			pTimeInfo->start_time.Year%100, pTimeInfo->start_time.Mon, pTimeInfo->start_time.Day,
			pTimeInfo->start_time.Hour, pTimeInfo->start_time.Min);
		grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[0] );		// "�J�n�����F�@�N�@���@���@�F�@�@"
		grachr( col, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[0] );		// �N
		grachr( col, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[2] );		// ��
		grachr( col, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[4] );		// ��
		grachr( col, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[6] );		// ��
		grachr( col, 26,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[8] );		// ��
		grachr( col, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR7[pTimeInfo->status[INFO_KIND_START]] );	// �����X�e�[�^�X
		col++;

		break;
	case PROG_ONLY_TIME:
// MH322915(S) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
//		if (!remote_date_chk(&pTimeInfo->sw_time)) {
		if (!Check_date_time_rec(&pTimeInfo->sw_time)) {
// MH322915(E) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
			grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[3] );	// "�\��͂���܂���@�@�@�@�@�@�@"
			return;
		}

		// �X�V����
		sprintf((char*)tmpStr, "%02d%02d%02d%02d%02d",
			pTimeInfo->sw_time.Year%100, pTimeInfo->sw_time.Mon, pTimeInfo->sw_time.Day,
			pTimeInfo->sw_time.Hour, pTimeInfo->sw_time.Min);
		grachr( col,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[1] );		// "�X�V�����F�@�N�@���@���@�F�@�@"
		grachr( col, 10,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[0] );		// �N
		grachr( col, 14,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[2] );		// ��
		grachr( col, 18,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[4] );		// ��
		grachr( col, 22,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[6] );		// ��
		grachr( col, 26,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, &tmpStr[8] );		// ��
		grachr( col, 28,  2, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR7[pTimeInfo->status[INFO_KIND_SW]] );	// �����X�e�[�^�X
		col++;

		break;
	default:
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�ʐM�`�F�b�N�����@���u�_�E�����[�h�m�F                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_CComRemoteDLChk(void)                            |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ushort FncChk_CComRemoteDLChk(void)
{
	ushort	msg;
	uchar	mode;		// 0:�����m�F���/ 1:�N���A�m�F���
	uchar	dsp;		// 0:�`��Ȃ�/ 1:��ʕ\��
	int		page;		// �y�[�W�ԍ�
// GG120600(S) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)
	u_inst_no*		pinstTemp = NULL;
	u_inst_no		instTemp;
// GG120600(E) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)

	dispclr();

	dsp		= 1;
	mode	= 0;
	page	= PROG_DL_TIME;

	for ( ; ; ) {
		if (dsp) {
			if (mode == 0) {
				if (dsp == 1) {
					dsp_time_info(page);
					Fun_Dsp(FUNMSG[118]);				// "�@���@�@���@�N���A       �I�� "
				}
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)		
		case KEY_MODECHG:
			return MOD_CHG;

		case KEY_TEN_F5:		// �u�I���v
			if (mode == 0) { 
				BUZPI();
				return MOD_EXT;
			}
			break;

		case KEY_TEN_F1:		// �y�[�W�ړ��i���j
			if (mode == 0) {	// 0:�����m�F���/ 1:�N���A�m�F���
				BUZPI();

				if (--page < 0) {
					page = TIME_INFO_MAX - 1;
				}
				dispclr();
				dsp = 1;
			}
			break;
		case KEY_TEN_F2:		// �y�[�W�ړ��i���j
			if (mode == 0) {	// 0:�����m�F���/ 1:�N���A�m�F���
				BUZPI();

				if (++page >= TIME_INFO_MAX) {
					page = PROG_DL_TIME;
				}
				dispclr();
				dsp = 1;
			}
			break;

		case KEY_TEN_F3:		// �N���A
			switch (mode) {
			case 0:				// �����m�F���
				BUZPI();
				dispmlclr(1, 6);
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR6[2] );							// "�@�@���������N���A���܂��@�@"
				grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
				Fun_Dsp(FUNMSG[19]);																	// "�@�@�@�@�@�@ �͂� �������@�@�@"
				mode = 1;		// 0:�����m�F���/ 1:�N���A�m�F���
				break;
			case 1:				// �N���A�m�F���
				BUZPI();

// GG120600(S) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)
//// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
////				remotedl_time_info_clear(page);
//				remotedl_time_info_clear(page,remotedl_instNo_get(),remotedl_instFrom_get());
//// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
				// �r���ŃN���A����邽�߁A���̎��_�ł̂�ێ����ēn��
				pinstTemp = remotedl_instNo_get_with_Time_Type(page);
				memcpy(&instTemp,pinstTemp,sizeof(u_inst_no));
				remotedl_time_info_clear(page,&instTemp,remotedl_instFrom_get_with_Time_Type(page));
// GG120600(E) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)
				dispmlclr(1, 6);
				Fun_Dsp(FUNMSG[118]);																	// "�@���@�@���@�N���A       �I�� "
				dsp  = 1;		// ��ʍĕ`��
				mode = 0;		// 0:�����m�F���/ 1:�N���A�m�F���
				break;
			default:
				break;
			}
			break;

		case KEY_TEN_F4:
			switch (mode) {		// 0:�����m�F���/ 1:�N���A�m�F���
			case 1:				// �N���A�m�F���
				BUZPI();
				dispmlclr(1, 6);
				dsp  = 1;		// ��ʍĕ`��
				mode = 0;		// 0:�����m�F���/ 1:�N���A�m�F���
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
}
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//[]----------------------------------------------------------------------[]
///	@brief		�������ԃ`�F�b�N(�������ԃf�[�^�ɂ�錟�o)
//[]----------------------------------------------------------------------[]
///	@return     ret    MOD_CHG/MOD_EXT
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date		Create	:	2018/09/06<br>
///				Update	:	
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
ushort	FncChk_CComLongParkAllRel( void )
{
	ushort	msg,i;

	dispclr();
// MH321800(S) S.Takahashi 2020/05/12 ���u�_�E�����[�h
//	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[14] );	// "���������ԑS�������M���@�@�@�@"
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[15] );	// "���������ԑS�������M���@�@�@�@"
// MH321800(E) S.Takahashi 2020/05/12 ���u�_�E�����[�h
	grachr( 2,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR3[9] );	// "�@�������ԑS�����𑗐M���܂��@"
	grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );		// "�@�@�@�@��낵���ł����H�@�@�@"
	Fun_Dsp(FUNMSG[19]);												// "�@�@�@�@�@�@ �͂� �������@�@�@"

	for ( ; ; ) {

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
			break;
		case KEY_TEN_F3:
			BUZPI();
			for(i = 0; i < LOCK_MAX ; i++){
				// �������Ԃ̑S�Ԏ��̏�Ԃ�����
				FLAPDT.flp_data[i].flp_state.BIT.b00 = 0;
				FLAPDT.flp_data[i].flp_state.BIT.b01 = 0;
			}
			// �S�����̃��O�o�^
			Make_Log_LongParking_Pweb( 0, 0, LONGPARK_LOG_ALL_RESET, LONGPARK_LOG_NON);// �S���� �������ԃf�[�^���O����
			Log_regist( LOG_LONGPARK );// �������ԃf�[�^���O�o�^
			return MOD_EXT;
			break;
		case KEY_TEN_F4:						// �u�������v
			BUZPI();
			return MOD_EXT;
			break;
		default:
			break;
		}
	}

}
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�(���σ��[�_�`�F�b�N�ǉ�)
/*[]----------------------------------------------------------------------[]*/
/*| ���σ��[�_�`�F�b�N                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FncChk_EcReader( void )                �@               |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
static ushort	FncChk_EcReader( void )
{
	ushort	msg;
	ushort	ret;
	char	wk[2];
	char	org[2];

// MH810103 GG119202(S) �ݒ�Q�ƕύX
	if (! isEC_USE()) {
		BUZPIPI();
		return MOD_EXT;
	}
// MH810103 GG119202(E) �ݒ�Q�ƕύX
	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	for ( ; ; ) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SUICASTR[16]);			/* "�����σ��[�_�`�F�b�N���@" */
		msg = Menu_Slt(ECR_MENU, FECR_CHK_TBL, (char)FECR_CHK_MAX, (char)1);
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];
		ret = 0;

		switch(msg) {		/* FunctionKey Enter */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return( MOD_CUT );
			break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case MOD_CHG:
			return( MOD_CHG );
			break;
		case MOD_EXT:		/* "�I��" */
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			return( MOD_EXT );
			break;
		case FISMF1_CHK:	/* �ʐM�e�X�g 				*/
			ret = Com_Chk_Suica();
			break;
		case FISMF2_CHK:	/* �ʐM���O�v�����g 		*/
			ret = Log_Print_Suica_menu();
			break;
		case FISMF3_CHK:	/* �ʐM���O�v�����g�i�ُ�j*/
			ret = Log_Print_Suica( 1 );
			break;
		case ECR_BRAND_COND:	/*   �u�����h��Ԋm�F   */
			ret = EcReaderBrandCondition();
			if (ret == MOD_CHG){
				return(MOD_CHG);
			}
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			if (ret == LCD_DISCONNECT){
				return(MOD_CUT);
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			break;
		case ECR_MNT:		/* ���σ��[�_���� -  ���[�_�����e�i���X	*/
			// ���σ��[�_������OK�H(���σ��[�_�N���V�[�P���X�N���ςłȂ���Γ���Ȃ�)
			if( Suica_Rec.Data.BIT.INITIALIZE == 0 ){
				BUZPIPI();
				break;
			}
			ret = EcReader_Mnt();
			if (ret == MOD_CHG){
				return(MOD_CHG);
			}
			break;
		default:
			break;
		}
		if (ret == MOD_CHG) {
			return MOD_CHG;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(MOD_EXT);
}

//[]----------------------------------------------------------------------[]
//	@brief			���σ��[�_�`�F�b�N/�u�����h��Ԋm�F
//[]----------------------------------------------------------------------[]
//	@param[in]		None
//	@return			usSysEvent		MOD_CHG : mode change<br>
// 									MOD_EXT : F5 key
//	@author			Inaba
//	@note			UsMnt_MultiEMoneyBrandCondition�ڐA
//	@attention		None
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/01/29
//					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static	ushort EcReaderBrandCondition(void)
{
	ushort				msg;
	int					mode = 0;			// 0:���s�O�A1:�v�����g��
	int					disp;
	int					index;
	ushort				pri_cmd	= 0;		// �󎚗v������ފi�[ܰ�
	uchar				pri;				// �Ώۃv�����^(0/R_PRI/J_PRI/RJ_PRI)
	T_FrmEcBrandReq		FrmPrnBrand;		// �u�����h��Ԉ󎚗v�����b�Z�[�W
	T_FrmPrnStop		FrmPrnStop;			// �󎚒��~�v��ү����ܰ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH810103(s) �d�q�}�l�[�Ή� #5582 �y���؉ێw�E�����z1�y�[�W�����Ȃ���ʂɃy�[�W�؂�ւ��{�^�����\������Ă���
//	uchar				single = 0;			// 1�u�����h�̂�
//// MH810103(e) �d�q�}�l�[�Ή� #5582 �y���؉ێw�E�����z1�y�[�W�����Ȃ���ʂɃy�[�W�؂�ւ��{�^�����\������Ă���
	int					idx;
	ushort				brand_no;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6438 �u�����h��Ԋm�F��QR�̃T�u�u�����h�m�F���Ƀh�A�m�u��߂�ƁA�ڋq��ʂɑJ�ڂ��Ȃ��B
	ushort	ret;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6438 �u�����h��Ԋm�F��QR�̃T�u�u�����h�m�F���Ƀh�A�m�u��߂�ƁA�ڋq��ʂɑJ�ڂ��Ȃ��B


	// �L���ȃe�[�u�����Ȃ��ꍇ�͏I��
	if (RecvBrandTbl[0].num == 0) {
		BUZPIPI();
		return MOD_EXT;
	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH810103(s) �d�q�}�l�[�Ή� #5582 �y���؉ێw�E�����z1�y�[�W�����Ȃ���ʂɃy�[�W�؂�ւ��{�^�����\������Ă���
//	else if( RecvBrandTbl[0].num == 1){
//		// 1�u�����h�̂�
//		single = 1;
//	}			
//// MH810103(e) �d�q�}�l�[�Ή� #5582 �y���؉ێw�E�����z1�y�[�W�����Ȃ���ʂɃy�[�W�؂�ւ��{�^�����\������Ă���
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	// �����\��
	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[10]);					/* "���u�����h��Ԋm�F���@�@�@�@�@" */
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH810103(s) �d�q�}�l�[�Ή� #5582 �y���؉ێw�E�����z1�y�[�W�����Ȃ���ʂɃy�[�W�؂�ւ��{�^�����\������Ă���
////	Fun_Dsp( FUNMSG2[17] );															/* "�@���@�@���@       ����� �I�� " */
//	if( single ){
//		Fun_Dsp( FUNMSG2[66] );															/* "�@ �@�@ �@       ����� �I�� " */
//		
//	}else{
//		Fun_Dsp( FUNMSG2[17] );															/* "�@���@�@���@       ����� �I�� " */
//	}
//// MH810103(e) �d�q�}�l�[�Ή� #5582 �y���؉ێw�E�����z1�y�[�W�����Ȃ���ʂɃy�[�W�؂�ւ��{�^�����\������Ă���
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	index = 0;
	disp = 1;
	for ( ; ; ) {
		// ��ʍX�V
		if (disp) {
			disp = 0;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//			dsp_brand_condition(index);
			brand_no = dsp_brand_condition(index);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		}
		
		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			// �h�A�m�u�̏�Ԃɂ�����炸�g�O�����삵�Ă��܂��̂ŁA
			// �h�A�m�u���ǂ����̃`�F�b�N�����{
			if (CP_MODECHG) {
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			return MOD_CHG;
// MH810100(S) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)
			}
			break;
// MH810100(E) S.Nishimoto 2020/08/31 �Ԕԃ`�P�b�g���X(#4758 �e�X�g�p�^���󎚒��Ƀh�A�m�u���J����Ə�����ʂɑJ�ڂ��A���Z�J�n���s���Ȃ���ԂƂȂ�)

		case KEY_TEN_F5:	/* �I��(F5) */
			if (mode != 0) {
			// �v�����g���͏������Ȃ�
				break;
			}
			BUZPI();
			return MOD_EXT;

		case KEY_TEN_F3:	/* ���~(F3) */
			if (mode != 1) {
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				idx = EcSubBrandCheck(brand_no);
				if (idx >= 0) {						// �T�u�u�����h�����u�����h�ԍ�
					BUZPI();
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6438 �u�����h��Ԋm�F��QR�̃T�u�u�����h�m�F���Ƀh�A�m�u��߂�ƁA�ڋq��ʂɑJ�ڂ��Ȃ��B
//					EcReaderSubBrandCondition((uchar)idx);	// �T�u�u�����h�̈ꗗ��\��
					ret = EcReaderSubBrandCondition((uchar)idx);	// �T�u�u�����h�̈ꗗ��\��
					if (ret == MOD_CHG){
						return(MOD_CHG);
					}
					if (ret == LCD_DISCONNECT){
						return(MOD_CUT);
					}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6438 �u�����h��Ԋm�F��QR�̃T�u�u�����h�m�F���Ƀh�A�m�u��߂�ƁA�ڋq��ʂɑJ�ڂ��Ȃ��B
					disp = 1;
				}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			// �v�����g���ȊO�A�������Ȃ�
				break;
			}
			/*------	�󎚒��~ү���ޑ��M	-----*/
			FrmPrnStop.prn_kind = R_PRI;			// �Ώ�������Fڼ��
			pri_cmd = PREQ_INNJI_TYUUSHI;
			queset( PRNTCBNO, PREQ_INNJI_TYUUSHI, sizeof(T_FrmPrnStop), &FrmPrnStop );
			BUZPI();
			return MOD_EXT;

		case KEY_TEN_F4:	/* �v�����g(F4) */
			if (mode == 0) {
			// ���s�O�̂ݗL��
				pri = R_PRI;
				if (check_printer(&pri) != 0) {
				// ���V�[�g�v�����^���؂� or �G���[������
					BUZPIPI();
					break;
				}
				BUZPI();
				/*------	�u�����h��Ԉ󎚗v��ү���ޑ��M	-----*/
				FrmPrnBrand.prn_kind = R_PRI;							// ��������
				FrmPrnBrand.Kikai_no = (ushort)CPrmSS[S_PAY][2];		// �@�B��
				FrmPrnBrand.pridata = &RecvBrandTbl[0];
				FrmPrnBrand.pridata2 = &RecvBrandTbl[1];
				pri_cmd = PREQ_EC_BRAND_COND;
				queset( PRNTCBNO, PREQ_EC_BRAND_COND, sizeof(T_FrmEcBrandReq), &FrmPrnBrand );
				mode = 1;
				Fun_Dsp( FUNMSG[82] );												/* "�@�@�@�@�@�@ ���~ �@�@�@�@�@�@" */
			}
			break;

		case KEY_TEN_F1:	/* ��(F1) */
			if (mode != 0) {
			// ���s�O�ȊO�A�������Ȃ�
				break;
			}
			if (RecvBrandTbl[0].num == 1) {
			// �L���ȃe�[�u����������Ȃ��ꍇ�͏������Ȃ�
				break;
			}
			BUZPI();
			if (index == 0) {
				index = RecvBrandTbl[0].num + RecvBrandTbl[1].num - 1;
			} else {
				index--;
			}
			// ��ʍX�V
			disp = 1;
			break;

		case KEY_TEN_F2:	/* ��(F2) */
			if (mode != 0) {
			// ���s�O�ȊO�A�������Ȃ�
				break;
			}
			if (RecvBrandTbl[0].num == 1) {
			// �L���ȃe�[�u����������Ȃ��ꍇ�͏������Ȃ�
				break;
			}
			BUZPI();
			if (index >= (RecvBrandTbl[0].num + RecvBrandTbl[1].num - 1)) {
				index = 0;
			} else {
				index++;
			}
			// ��ʍX�V
			disp = 1;
			break;

		case EC_BRAND_UPDATE:
			disp = 1;
			break;

		default:
			if (mode != 0) {
			// ���s�O�ȊO
				// �󎚃��b�Z�[�W����
				if (pri_cmd != 0) {
					if (msg == ( pri_cmd | INNJI_ENDMASK )) { 				// �󎚏I��ү���ގ�M�H
						// ���s����
						return MOD_EXT;
					}
				}
			}
			break;
		}
		
	}
}

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define	SUB_BRAND_VIEW_LIMIT	8	// 1�y�[�W�̃T�u�u�����h�ő�\����
//[]----------------------------------------------------------------------[]
///	@brief			�T�u�u�����h�ꗗ�\��
//[]----------------------------------------------------------------------[]
///	@param[in]		sub_idx�i0:�T�u�u�����h�Ȃ��j
///	@return			MOD_XX
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022-03-22
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static ushort EcReaderSubBrandCondition(uchar sub_idx)
{
	ushort	msg;
	uchar	disp;
	uchar	sub_num;
	uchar	max_page;
	uchar	current_page;

	if (sub_idx > 0) {
		// �T�u�u�����h����

		// �T�u�u�����h�̐�����y�[�W�����Z�o(1�y�[�W8����)
		sub_num = RecvSubBrandTbl.sub_brand_num[sub_idx-1];
		if (sub_num > 0) {
			max_page = (uchar)((sub_num-1) / SUB_BRAND_VIEW_LIMIT);
		}
	}
	else {
		max_page = 0;
	}
	current_page = 0;							// 1�y�[�W��
	disp = 1;

	while (1) {
		if (disp) {
			disp = 0;
			if (max_page == 0) {
				Fun_Dsp( FUNMSG[8] );			// "�@�@�@�@�@�@�@�@�@�@�@ �I�� "
			} else {
				Fun_Dsp( FUNMSG[6] );			// "�@���@�@���@�@�@�@�@�@�@ �I�� "
			}
			dsp_sub_brand_condition(sub_idx, current_page, max_page);
		}

		msg = StoF(GetMessage(), 1);
		switch (KEY_TEN0to9(msg)) {
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6438 �u�����h��Ԋm�F��QR�̃T�u�u�����h�m�F���Ƀh�A�m�u��߂�ƁA�ڋq��ʂɑJ�ڂ��Ȃ��B
//		case KEY_MODECHG:
//			return MOD_CHG;
		case LCD_DISCONNECT:
			return MOD_CUT;
		case KEY_MODECHG:
			// �h�A�m�u���ǂ����̃`�F�b�N�����{
			if (CP_MODECHG) {
				return MOD_CHG;
			}
			break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6438 �u�����h��Ԋm�F��QR�̃T�u�u�����h�m�F���Ƀh�A�m�u��߂�ƁA�ڋq��ʂɑJ�ڂ��Ȃ��B

		case KEY_TEN_F1:	/* ��(F1) */
			if (max_page == 0) {
				break;
			}

			BUZPI();
			if (current_page == 0) {
				current_page = max_page;
			}
			else {
				current_page--;
			}
			disp = 1;
			break;

		case KEY_TEN_F2:	/* ��(F2) */
			if (max_page == 0) {
				break;
			}

			BUZPI();
			if (current_page >= max_page) {
				current_page = 0;
			}
			else {
				current_page++;
			}
			disp = 1;
			break;

		case KEY_TEN_F5:	/* �I��(F5) */
			BUZPI();
			return MOD_EXT;
		}
	}
}

uchar	initial_message[2][19] = {
	{"���σu�����h����M"},
	{"���σu�����h�Ȃ�"},
};
//[]----------------------------------------------------------------------[]
///	@brief			�T�u�u�����h�ꗗ�\��(�ڍ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		sub_idx			: �T�u�u�����h�C���f�b�N�X�i0:�T�u�u�����h�Ȃ��j
///	@param[in]		current_page	: ���݃y�[�W
///	@param[in]		max_page		: �ő�y�[�W
///	@return			None
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022-03-22
///					Update	:
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void dsp_sub_brand_condition(uchar sub_idx, uchar current_page, uchar max_page)
{
	uchar	i, max;
	uchar	cnt;
	uchar	line_max = SUB_BRAND_VIEW_LIMIT/2;
	ushort	len;
	char	str_page[8];
	char	brand_name[15];

	// ��ʃN���A
	for(i = 1; i <= 6; i++) {
		displclr((ushort)i);
	}

	// �y�[�W��\��
	memset(str_page, 0, sizeof(str_page));
	sprintf(str_page, "[%02d/%02d]", current_page+1, max_page+1);
	grachr(0, 22, 7, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)str_page);

	if (RecvSubBrandTbl.tbl_sts.BIT.RECV == 0) {
		// "���σu�����h����M"
		grachr(3, 2, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, initial_message[0]);
		return;
	}
	else if (sub_idx == 0) {
		// "���σu�����h�Ȃ�"
		grachr(3, 2, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, initial_message[1]);
		return;
	}

	i = (uchar)(current_page * SUB_BRAND_VIEW_LIMIT);
	max = (uchar)((current_page+1) * SUB_BRAND_VIEW_LIMIT);
	for (; i < max; i++) {
		if (i >= RecvSubBrandTbl.sub_brand_num[sub_idx-1]) {
			// �T�u�u�����h���ɓ��B
			break;
		}

		// �T�u�u�����h���擾
		len = (ushort)EcGetSubBrandName(RecvSubBrandTbl.brand_no[sub_idx-1], 0, i, (uchar*)brand_name);
		if (len == 0) {
			continue;
		}

		cnt = (uchar)(i % SUB_BRAND_VIEW_LIMIT);
		if (cnt < line_max) {		// ���ɕ\�����邩�A�E�ɕ\�����邩
			// �T�u�u�����h�������ɕ\��
			grachr((ushort)(2+cnt), 2, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)brand_name);
		}
		else {
			// �T�u�u�����h���E���ɕ\��
			grachr((ushort)(2+cnt-line_max), 16, len, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)brand_name);
		}
	}
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h��ԕ\��
//[]----------------------------------------------------------------------[]
///	@param[in]		index	:�u�����h�e�[�u���C���f�b�N�X
///	@return			None
///	@author			Inaba
///	@note			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/01/29
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//static	void dsp_brand_condition(int index)
static	ushort dsp_brand_condition(int index)
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
{
	char		str_page[8];
// MH810103 GG119202(S) �u�����h���̓u�����h��񂩂�擾����
//	int			brand;
// MH810103 GG119202(E) �u�����h���̓u�����h��񂩂�擾����
	int			brand_index;
	int			check;
	int			Array_Num  = 0;
	ushort		i;
	int			page_index = index;
// MH810103 GG119202(S) �u�����h���̓u�����h��񂩂�擾����
	uchar		wname[21];
// MH810103 GG119202(E) �u�����h���̓u�����h��񂩂�擾����
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	ushort		brand_no;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	// �u�����h����10���傫�����ARecvBrandTbl[1]���Q�Ƃ���B
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//	if(index >= EC_BRAND_MAX){
//		Array_Num = 1;
//		index = index - EC_BRAND_MAX;
//	}
	if(index >= RecvBrandTbl[0].num){
		Array_Num = 1;
		index = index - RecvBrandTbl[0].num;
	}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	brand_no = RecvBrandTbl[Array_Num].ctrl[index].no;
	if (RecvBrandTbl[0].num == 1) {
		if (EcSubBrandCheck(brand_no) >= 0) {
			Fun_Dsp( FUNMSG2[68] );							// "�@�@�@�@�@�@ ����� ����� �I�� "
		}
		else {
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6438 �u�����h��Ԋm�F��QR�̃T�u�u�����h�m�F���Ƀh�A�m�u��߂�ƁA�ڋq��ʂɑJ�ڂ��Ȃ��B
//			Fun_Dsp( FUNMSG2[64] );							// "�@ �@�@ �@       ����� �I�� "
			Fun_Dsp( FUNMSG2[66] );							// "�@ �@�@ �@       ����� �I�� "
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6438 �u�����h��Ԋm�F��QR�̃T�u�u�����h�m�F���Ƀh�A�m�u��߂�ƁA�ڋq��ʂɑJ�ڂ��Ȃ��B
		}
	}
	else{
		if (EcSubBrandCheck(brand_no) >= 0) {
			Fun_Dsp( FUNMSG2[67] );							// "�@���@�@���@ ����� ����� �I�� "
		}
		else {
			Fun_Dsp( FUNMSG2[17] );							// "�@���@�@���@       ����� �I�� "
		}
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	// ��ʃN���A
	for(i = 1; i <= 5; i++) {
		displclr((ushort)i);
	}

	// �y�[�W
	memset(str_page, 0x0, sizeof(str_page));
	sprintf(str_page, "[%02d/%02d]", page_index + 1, (int)(RecvBrandTbl[0].num + RecvBrandTbl[1].num));
	grachr(0, 22, 7, 0, COLOR_BLACK, LCD_BLINK_OFF, (const unsigned char*)str_page);

	// �u�����h��
// MH810103 GG119202(S) �u�����h���̓u�����h��񂩂�擾����
//	brand = get_brand_index(RecvBrandTbl[Array_Num].ctrl[index].no);
//	if( brand == ( EC_UNKNOWN_USED - EC_USED ) ){
//		brand_index = 3;	// �s��
//	} else {
//		brand_index = brand + 2;
//	}
//	grachr(1, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_BRAND_STR[brand_index]);
//	grachr(1, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, "�F");
	brand_index = getBrandName(&RecvBrandTbl[Array_Num].ctrl[index], wname);
	grachr(1, 2, (ushort)brand_index, 0, COLOR_BLACK, LCD_BLINK_OFF, wname);
// MH810103 GG119202(E) �u�����h���̓u�����h��񂩂�擾����

	// ���x��
	for (i = 0; i < 3; i++) {
		grachr(i+2, 0, 16, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_BRAND_STR[i]);				// "    �J�ǁ^��  "
		grachr(i+2, 16, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, "�F");						// "    �L���^����  "
	}																					// "    �L���^����  "
																						// "    �T�[�r�X���"
	// ���
// MH810103 GG119202(S) �u�����h��Ԕ��菈���ύX
//	check = ((RecvBrandTbl[Array_Num].ctrl[index].status & 0x0f) == 0x01) ? 1 : 0;		// �J�� & �L�� & �� & ���쒆
// MH810103 GG119202(S) �T�[�r�X��~���u�����h�̓u�����h�ݒ�f�[�^�ɃZ�b�g���Ȃ�
//	check = (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[Array_Num].ctrl[index].status)) ? 1 : 0;	// �J�� & �L��
	check = (isEC_BRAND_STS_ENABLE(RecvBrandTbl[Array_Num].ctrl[index].status)) ? 1 : 0;	// �J�� & �L�� & �T�[�r�X��~��
// MH810103 GG119202(E) �T�[�r�X��~���u�����h�̓u�����h�ݒ�f�[�^�ɃZ�b�g���Ȃ�
// MH810103 GG119202(E) �u�����h��Ԕ��菈���ύX
// MH810103 GG119202(S) �u�����h���̓u�����h��񂩂�擾����
//	grachr(1, 18, (check ? 6 : 8), 0, COLOR_BLACK, LCD_BLINK_OFF, EC_STR[check]);		// "���p�s��/���p��"
	grachr(5, 11, 8, 0, (check ? COLOR_GREEN : COLOR_RED), LCD_BLINK_OFF, EC_STR[check]);	// "���p�s��/���p��"
// MH810103 GG119202(E) �u�����h���̓u�����h��񂩂�擾����
	check = ((RecvBrandTbl[Array_Num].ctrl[index].status & 0x01) == 0x01) ? 1 : 0;
	grachr(2, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_STR[2 + check]);					// "��/�J��"
	check = ((RecvBrandTbl[Array_Num].ctrl[index].status & 0x02) == 0x02) ? 1 : 0;
	grachr(3, 18, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_STR[4 + check]);					// "�L��/����"
	check = ((RecvBrandTbl[Array_Num].ctrl[index].status & 0x08) == 0x08) ? 1 : 0;
	grachr(4, 18, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, EC_STR[6 + check]);					// "���쒆/��~��"

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	return;
	return brand_no;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
}

// MH810103 GG119202(S) �u�����h���̓u�����h��񂩂�擾����
////[]----------------------------------------------------------------------[]
////	@brief		�u�����h�C���f�b�N�X�擾
////[]----------------------------------------------------------------------[]
////	@param[in]	no	:	�u�����h�ԍ�
////	@return		�u�����h�C���f�b�N�X
////[]----------------------------------------------------------------------[]
////	@author		emura
////	@date		Create	: 14/03/28<br>
////				Update	: 19/01/29  ���σ��[�_�Ή�
////	@file		usermnt2.c
////[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
//uchar	get_brand_index(ushort no)
//{
//// GG119200(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
////	uchar	index = EC_UNKNOWN_USED - EC_USED;
////
////	switch (no) {
////	case	BRANDNO_KOUTSUU:
////		index = EC_KOUTSUU_USED - EC_USED;
////		break;
////	case	BRANDNO_EDY:
////		index = EC_EDY_USED - EC_USED;
////		break;
////	case	BRANDNO_NANACO:
////		index = EC_NANACO_USED - EC_USED;
////		break;
////	case	BRANDNO_WAON:
////		index = EC_WAON_USED - EC_USED;
////		break;
////	case	BRANDNO_SAPICA:
////		index = EC_SAPICA_USED - EC_USED;
////		break;
////	case	BRANDNO_ID:
////		index = EC_ID_USED - EC_USED;
////		break;
////	case	BRANDNO_QUIC_PAY:
////		index = EC_QUIC_PAY_USED - EC_USED;
////		break;
////	case	BRANDNO_CREDIT:
////		index = EC_CREDIT_USED - EC_USED;
////		break;
////	case	BRANDNO_HOUJIN:
////		index = EC_HOUJIN_USED - EC_USED;
////		break;
////	default	:
////		break;
////	}
////	return index;
//	return (uchar)(convert_brandno(no) - EC_USED);
//// GG119200(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//}
// MH810103 GG119202(E) �u�����h���̓u�����h��񂩂�擾����

/*[]----------------------------------------------------------------------[]*/
/*| �@�@�@�@�@�@�@�@���[�_�����e�i���X���� /�@���M��ʁi��M��ʁj�@�@�@   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SysMnt_EcReaderMnt(void)       �@                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : MOD_EXT(F5)   MOD_CHG(�����e�i���X�I��) 	   			   |*/
/*| Date         : 2019-01-28                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static ushort EcReader_Mnt( void )
{
	ushort	msg;
	uchar	mode_now = 0; 	// ���݂̃��[�_���[�h
							// 0 = �ʏ탂�[�h
							// 1 = �����e�i���X���[�h
							// 2 = ���ʎ�M��ʂɂă����e�i���X�X�C�b�`OFF
							// 3 = ���ʎ�M��ʂɂĒʐM�s�ǔ���
// MH810100(S) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
							// 4 = ���ʎ�M��ʂɂ�LCD�ʐM�s�ǔ���
// MH810100(E) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
	uchar	disp = 1;		// 1 = ��ʍX�V
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//	uchar	mnt_exit = 0;	// �����e�i���X���[�h�I���m�F��ʃt���O�F 0 = �u�����e�i���X���v�\�� , 1 = �u�ʏ탂�[�h�Ɂ`�v�\��
	uchar	ret = 0;		// 0=����, 1=���s, 2=�����eOFF, 3=�ʐM�s��
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX

	// ���M�f�[�^������
	memset(&MntTbl, 0x0, sizeof(EC_MNT_TBL));

	for( ; ; ) {
		if( disp ){
			if( mode_now == 0 ){
				if(!Ope_IsEnableDoorKnobChime()){											// �h�A�m�u�߂��Y��`���C�������H
					Ope_EnableDoorKnobChime();												// �h�A�m�u�߂��Y��`���C���L���ɂ���(��ϰ����)
				}
				dispclr();
				grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "�����[�_�����e�i���X���@  �@�@" */
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[1] );				/* " ���[�_�������e�i���X���[�h�� " */
//				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[2] );				/* " �ڍs���܂��B��낵���ł����H "*/
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[1] );				/* "�@�@�@�@ ���σ��[�_�� �@�@�@�@" */
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[2] );				/* "�@�����e�i���X���[�h�ɂ��܂��@" */
				grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );				/* "�@�@�@ ��낵���ł����H �@�@�@" */
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX
				Fun_Dsp( FUNMSG[19] );														/* "�@�@�@�@�@�͂��@�������@�@�@" */
				MntTbl.mode = 1;		// ����̃��[�h���ʏ�Ȃ瑗�郂�[�h�̓����e�i���X���[�h
				disp = 0;
			} else {
				dispclr();
				grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "�����[�_�����e�i���X���@�@  �@" */
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//				if( mnt_exit == 0 ){
//					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[5] );			/* "  ���[�_�����e�i���X���ł��B  " */
//					Fun_Dsp( FUNMSG[8] );													/* "�@�@�@�@�@  �@�@�@  �I�� " */
//				} else {
//					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[14] );			/* " �@�@���[�_��ʏ탂�[�h�� �@�@" */
//					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[2] );			/* " �ڍs���܂��B��낵���ł����H "*/
//					Fun_Dsp( FUNMSG[19] );													/* "�@�@�@�@�@�͂��@�������@�@�@" */
//				}
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[5] );				/* " �@���[�_�����e�i���X���ł� �@" */
				Fun_Dsp( FUNMSG[8] );														/* "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� " */
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX
				MntTbl.mode = 0;		// ����̃��[�h�������e�i���X�Ȃ瑗�郂�[�h�͒ʏ탂�[�h
				disp = 0;
			}
		}

		if( mode_now == 1 ){
			Ope_DisableDoorKnobChime();														// ���σ��[�_�����e�i���X���̓h�A�m�u�߂��Y��`���C������
		}

		msg = StoF( GetMessage(), 1 );
		switch( msg ){						/* FunctionKey Enter */
// MH810100(S) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
		case LCD_DISCONNECT:
			// ���[�_��ʏ탂�[�h�Ɉڍs���ďI������
			if( mode_now == 1 ){
				Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
// MH810103 GG119202(S) �����e�i���X�𔲂���Ƃ��������҂�����
				EcReader_Mnt_CommResult( 0 );
// MH810103 GG119202(E) �����e�i���X�𔲂���Ƃ��������҂�����
			}
			return MOD_CUT;
			break;
// MH810100(E) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)

		case KEY_MODECHG:
			// ���[�_��ʏ탂�[�h�Ɉڍs���ďI������
			if( mode_now == 1 ){
				Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
// MH810103 GG119202(S) �����e�i���X�𔲂���Ƃ��������҂�����
				EcReader_Mnt_CommResult( 0 );
// MH810103 GG119202(E) �����e�i���X�𔲂���Ƃ��������҂�����
			}
			return MOD_CHG;
			break;

		case KEY_TEN_F3:					/* F3: �͂�*/
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//			if( mode_now == 0 ) {	// ���[�_�̃��[�h���ʏ�
//				BUZPI();
//				// ���[�_�������e�i���X���[�h�Ɉڍs
//				Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
//				mode_now = EcReader_Mnt_CommResult( MntTbl.mode );
//				disp = 1;
//			} else {				// ���[�_�̃��[�h�������e�i���X
//				if( mnt_exit ){		// �u�ʏ탂�[�h�Ɂ`�v�\�����ȊO�͎󂯕t���Ȃ�
//					BUZPI();
//					// ���[�_��ʏ탂�[�h�Ɉڍs
//					Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
//					mode_now = EcReader_Mnt_CommResult( MntTbl.mode );
//					mnt_exit = 0;
//					disp = 1;
//				}
//			}
//			// ���ʎ�M��ʂɂĕ����I�Ƀ����e�i���X�X�C�b�`��OFF�ɂ���
//			if( mode_now == 2 ){
//				return MOD_CHG;
//			}
//			// ���ʎ�M��ʂɂĒʐM�s�ǂ���������
//			else if( mode_now == 3 ){
//				return MOD_EXT;
//			}
//			// mode_now = 1 �������� mode_now = 0
//			else{
//				// �����e�i���XOFF(����߂Ă�����)
//				if( !OPE_SIG_DOORNOBU_Is_OPEN ){
//					if( mode_now == 1 ){
//						// �����e�i���X���[�h���ł���Βʏ탂�[�h�Ɉڍs���ďI������
//						MntTbl.mode = 0;
//						Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
//// GG119200(S) �����e�i���X�𔲂���Ƃ��������҂�����
//						EcReader_Mnt_CommResult( 0 );
//// GG119200(E) �����e�i���X�𔲂���Ƃ��������҂�����
//					}
//					return MOD_CHG;
//				}
//			}
			if (mode_now != 0) {
				break;
			}

			BUZPI();
			// ���[�_�������e�i���X���[�h�Ɉڍs
			Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
			ret = EcReader_Mnt_CommResult( MntTbl.mode );
			switch (ret) {
			case 0:				// ����
				// �����e�i���XOFF(����߂Ă�����)
				if (!OPE_SIG_DOORNOBU_Is_OPEN) {
					// ���[�_��ʏ탂�[�h�Ɉڍs
					MntTbl.mode = 0;
					Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
					EcReader_Mnt_CommResult( MntTbl.mode );
					return MOD_CHG;
				}
				// �����e�i���X���[�h
				mode_now = 1;
				disp = 1;
				break;
			case 1:				// ���s
			case 3:				// �ʐM�s��
			default:
				return MOD_EXT;
			case 2:				// �����eOFF
				return MOD_CHG;
// MH810100(S) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
			case 4:
				return MOD_CUT;
// MH810100(E) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
			}
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX
			break;
		
		case KEY_TEN_F4:					/* F4: ������*/
			if( mode_now == 0 ) {	// ���[�_�̃��[�h���ʏ�	
				BUZPI();
				return MOD_EXT;
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//			} else {				// ���[�_�̃��[�h�������e�i���X
//				if( mnt_exit ){		// �u�ʏ탂�[�h�Ɂ`�v�\�����ȊO�͎󂯕t���Ȃ�
//					BUZPI();
//					mnt_exit = 0;
//					disp = 1;
//				}
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX
			}
			break;
		
		case KEY_TEN_F5:					/* F5:�I�� */
			// �u�����e�i���X���v�\�����ȊO�͎󂯕t���Ȃ�
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//			if( mode_now == 1 && mnt_exit == 0 ) {
//				BUZPI();
//				mnt_exit = 1;
//				disp = 1;
//			}
			if (mode_now != 1) {
				break;
			}
			BUZPI();

			// ���[�_��ʏ탂�[�h�Ɉڍs
			Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
			ret = EcReader_Mnt_CommResult( MntTbl.mode );
			switch (ret) {
			case 0:				// ����
				// �ʏ탂�[�h
				mode_now = 1;
				// no break
			case 1:				// ���s
			case 3:				// �ʐM�s��
			default:
				return MOD_EXT;
			case 2:				// �����eOFF
				return MOD_CHG;
			}
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX
			break;

		case EC_MNT_UPDATE:			// ���σ��[�_�����烂�[�h�ڍs�ʒm�f�[�^���M���Ă���
			// �����Đ��������ꍇ�͓ǂݎ̂�
			if( RecvMntTbl.cmd == 1){
				break;
			} else if(RecvMntTbl.cmd == 2) {
				// ���σ��[�_���ċN���������ߐ��Z�@�̍ċN���𑣂�
				EcReader_Mnt_ResetDisp();
			}
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//			mode_now = RecvMntTbl.mode;
//			mnt_exit = 0;
//			disp = 1;				// ��ʍX�V
			if (RecvMntTbl.mode == 0) {
				// �ʏ탂�[�h
				return MOD_EXT;
			}
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX
			break;

		case EC_EVT_COMFAIL:		// �ʐM�s�ǔ���
				dispclr();
				grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );			/* "�����[�_�����e�i���X���@�@�@  " */
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[15] );			/* "�@�@�ʐM�s�ǂ��������܂����@�@" */
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[16] );			/* "���[�_�����e�i���X���I�����܂�" */
				Fun_Dsp( FUNMSG[8] );													/* "�@�@�@�@�@  �@�@�@  �I�� " */
				for( ; ; ){
					msg = StoF( GetMessage(), 1 );
					switch( msg ){								/* FunctionKey Enter */
						case KEY_TEN_F5:						/* F5:�I�� */
							BUZPI();
							return MOD_EXT;
// MH810100(S) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
						case LCD_DISCONNECT:
							return MOD_CUT;
// MH810100(E) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
						case KEY_MODECHG:
							BUZPI();
							return MOD_CHG;
						// �������Ȃ�
						default:
							break;
					}
				}
			break;

		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���[�_�����e�i���X���� /��M��ʁi���s��ʁj                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcReader_Mnt_CommResult                                 |*/
/*| PARAMETER    : uchar ���σ��[�_�֑��M�������[�h                        |*/
/*| RETURN VALUE : uchar 0 : ����                                          |*/
/*|                      1 : ���s                                          |*/
/*|                      2 : �����e�i���X�X�C�b�`OFF                       |*/
/*|                      3 : �ʐM�s�ǔ���                                  |*/
/*| Date         : 2019-01-28                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static uchar EcReader_Mnt_CommResult( uchar mode )
{
	ushort	msg;
	uchar	result = 0; 	// 0 = �����A�@1 = ���s
	uchar	permission = 0;	// 0 = �����Ȃ��A1 = ������
// MH810103 GG119202(S) �h�A���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�
//	uchar	modechg = 1;	// 0 = �����e�i���X�X�C�b�`OFF�A1 = �����e�i���X�X�C�b�`ON
	uchar	mnt_sw = 0;		// �����e�i���XOFF�ɂȂ������ǂ���
// MH810100(S) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
	uchar	disconnect = 1;	// 0 = LCD�ʐM�s�ǔ����A1 = LCD�ʐM����
// MH810100(E) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)

	if (!OPE_SIG_DOORNOBU_Is_OPEN) {
		mnt_sw = 1;			// �����eOFF�ێ�
	}
// MH810103 GG119202(E) �h�A���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�

	// ���M���������e�i���X���[�h�ɉ����ĕ\����ς��� mode�F 1 = �����e�i���X���[�h���M�@0 = �ʏ탂�[�h���M
	dispclr();
	if( mode ){
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "�����[�_�����e�i���X���@  �@�@" */
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[3] );				/* "�����e�i���X���[�h�Ɉڍs���ł�" */
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[4] );				/* "�@���΂炭���҂����������B�@" */
	} else {
		grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "�����[�_�����e�i���X���@  �@�@" */
		grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[6] );				/* "�@�ʏ탂�[�h�Ɉڍs���ł��B  �@" */
		grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[4] );				/* "�@���΂炭���҂����������B  �@" */
	}
// MH810105(S) MH364301 �t�@���N�V�����L�[��\��������
	Fun_Dsp( FUNMSG[0] );															/* "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */
// MH810105(E) MH364301 �t�@���N�V�����L�[��\��������

	// �^�C���A�E�g�ݒ�
	Lagtim(OPETCBNO, TIMERNO_EC_CMDWAIT, EC_CMDWAIT_TIME);		/* 20sec timer start */

	for( ; ; ) {

		// ���s�̎��\������
		if( result ){
			dispclr();
			grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );			/* "�����[�_�����e�i���X���@�@�@  " */
				if( mode ){
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[7] );	/* " �����e�i���X���[�h�ւ̈ڍs�� " */
				} else {
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[8] );	/* " 	  �ʏ탂�[�h�ւ̈ڍs��	  "*/
				}
			grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[9] );			/* "  �@�@�@���s���܂����B�@  �@�@" */
			Fun_Dsp( FUNMSG[8] );													/* "�@�@�@�@�@  �@�@�@      �I��  " */
			result = 0;
			permission = 1;
// MH810103 GG119202(S) �h�A���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�
			// �����e�i���XOFF(����߂Ă�����)
			if (mnt_sw) {
				return 2;
			}
// MH810103 GG119202(E) �h�A���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�
		}

		msg = StoF( GetMessage(), 1 );
		switch( msg ){						/* FunctionKey Enter */
		case EC_EVT_COMFAIL:		// �ʐM�s�ǔ���
				dispclr();
				grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );		/* "�����[�_�����e�i���X���@�@�@  " */
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[15] );		/* "�@�@�ʐM�s�ǂ��������܂����@�@" */
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[16] );		/* "���[�_�����e�i���X���I�����܂�" */
				Fun_Dsp( FUNMSG[8] );												/* "�@�@�@�@�@  �@�@�@  �I�� " */
				for( ; ; ){
					msg = StoF( GetMessage(), 1 );
					switch( msg ){													/* FunctionKey Enter */
						case KEY_TEN_F5:											/* F5:�I�� */
							BUZPI();
							result = 3;
// MH810103 GG119202(S) �₢���킹�^�C�}�L�����Z�������ǉ�
							Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) �₢���킹�^�C�}�L�����Z�������ǉ�
							return result;
						case KEY_MODECHG:
							BUZPI();
							result = 2;
// MH810103 GG119202(S) �₢���킹�^�C�}�L�����Z�������ǉ�
							Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) �₢���킹�^�C�}�L�����Z�������ǉ�
							return result;
// MH810100(S) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
						case LCD_DISCONNECT:
							result = 4;
							return result;
// MH810100(E) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
						// �������Ȃ�
						default:
							break;
					}
				}
			break;

// MH810100(S) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
		case LCD_DISCONNECT:
			// LCD�ʐM�s�ǔ���
			disconnect = 0;
			break;
// MH810100(E) S.Fujii 2020/06/30 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)

		case KEY_MODECHG:
// MH810103 GG119202(S) �h�A���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�
//				// �����e�i���X�X�C�b�`��OFF�ɂ���
//				modechg = 0;
			if (permission) {
				Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
				return 2;
			}
			else {
				mnt_sw = 1;
			}
// MH810103 GG119202(E) �h�A���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�
			break;

		case KEY_TEN_F5:					/* F5:�I�� */
			// ���s��ʈȊO�͏��������Ȃ�
			if( permission ) {
				BUZPI();
// MH810103 GG119202(S) �h�A���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�
//				// �����e�i���X�X�C�b�`��OFF�ɂ���
//				if( !modechg ){
//					// ���[�_��ʏ탂�[�h�Ɉڍs���ďI������
//					if( mode == 0 ){
//						Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
//					}
//					mode = 2;
//				} else {
				{
// MH810103 GG119202(E) �h�A���Ă��ҋ@��ʂɑJ�ڂ��Ȃ�
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//					// ���s���͕K���t��Ԃ�
//					mode = mode? 0 : 1;
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX
				}
// MH810103(s) �d�q�}�l�[�Ή�
				if( !disconnect ){
					// ���[�_��ʏ탂�[�h�Ɉڍs���ďI������
					Ec_Pri_Data_Snd( S_READER_MNT_DATA, 0 );
				}
// MH810103(e) �d�q�}�l�[�Ή�
// MH810103 GG119202(S) �₢���킹�^�C�}�L�����Z�������ǉ�
				Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) �₢���킹�^�C�}�L�����Z�������ǉ�
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//				return mode;
				return 1;	// ���s
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX
			}
			break;

		case EC_MNT_UPDATE:			// ���[�h�ڍs�ʒm�f�[�^��M
			// �����Đ��������ꍇ�͓ǂݎ̂�
			if( RecvMntTbl.cmd == 1){
				break;
			} else if(RecvMntTbl.cmd == 2) {
				// ���σ��[�_���ċN���������ߐ��Z�@�̍ċN���𑣂�
				EcReader_Mnt_ResetDisp();
			}
			// ��M�������[�h�Ɨv���������[�h���s��v�̏ꍇ��
			// ���[�h�ڍs���s�Ƃ���
			if( RecvMntTbl.mode != mode){
				result = 1;
				break;
			}
// MH810103 GG119202(S) �₢���킹�^�C�}�L�����Z�������ǉ�
			Lagcan(OPETCBNO, TIMERNO_EC_CMDWAIT);
// MH810103 GG119202(E) �₢���킹�^�C�}�L�����Z�������ǉ�
// MH810103 GG119202(S) ���[�_�[�����e�i���X��ʎd�l�ύX
//			return mode;
//			break;
			return 0;	// ����
// MH810103 GG119202(E) ���[�_�[�����e�i���X��ʎd�l�ύX

		case EC_CMDWAIT_TIMEOUT: // �^�C���A�E�g
			result = 1;
			break;

		default:
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �@�@�@�@�@�@���[�_�����e�i���X���� /�@�ċN����ʁ@�@�@�@�@�@�@�@�@�@ �@|*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : EcReader_Mnt_ResetDisp        �@                        |*/
/*| PARAMETER    : uchar                                                   |*/
/*| RETURN VALUE : MOD_EXT    			                                   |*/
/*| Date         : 2019-09-11                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
static void 	EcReader_Mnt_ResetDisp(void)
{
	ushort	msg = 0;

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[0] );				/* "�����[�_�����e�i���X���@  �@�@" */
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[11] );				/* "�@�@���σ��[�_���X�V���ł��@�@" */
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[4] );				/* "�@�@���΂炭���҂����������@�@" */
	Fun_Dsp( FUNMSG[0] );														/* "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@" */

	// �^�C���A�E�g�ݒ�
	Lagtim(OPETCBNO, TIMERNO_EC_CMDWAIT, EC_CMDWAIT_TIME);		/* 20sec timer start */

	// �^�C���A�E�g��҂�
	for( ; ; ) {
		msg = StoF( GetMessage(), 1 );
		switch( msg ){
		case EC_CMDWAIT_TIMEOUT: // �^�C���A�E�g
			grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[12] );		/* "���σ��[�_�̍X�V���������܂���" */
			grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR5[13] );		/* " �@�d����OFF/ON���Ă��������@ " */
			break;
		// �������Ȃ�
		default:
			break;
		}
	}
}
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�(���σ��[�_�`�F�b�N�ǉ�)

// MH810100(S) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)
//[]----------------------------------------------------------------------[]
/// @brief �p�q���[�_�[�`�F�b�N���j���[
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort  FncChk_QRchk( void ) 
{
	ushort msg;
	char	wk[2];
	char	org[2];

	// QR�L���H
	if( !QR_READER_USE ){
		BUZPIPI();
		return MOD_EXT;
	}
	
	org[0] = DP_CP[0];
	org[1] = DP_CP[1];

	DP_CP[0] = DP_CP[1] = 0;


	for ( ; ; ) {

		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[57] );		// "���p�q���[�_�[�`�F�b�N���@�@"

		msg = Menu_Slt( QRMENU, QR_CHK_TBL, (char)QR_CHK_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (msg) {
			case QR_READ:
				msg = FncChk_QRread();		// �ǎ�e�X�g
				break;

			case QR_VER:
				msg = FncChk_QRverChk();	// �o�[�W�����m�F
				break;

			case MOD_EXT:								// �I���i�e�T�j
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return msg;

			default:
				break;
		}
		if (msg == MOD_CHG || msg == MOD_CUT){				// ���[�h�`�F���W
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return msg;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}

//[]----------------------------------------------------------------------[]
/// @brief		QR�ǂݎ��e�X�g
//[]----------------------------------------------------------------------[]
///	@param[in]	����
/// @return 	ret		: Exit ID<br>
///					MOD_CUT=�ؒf�ʒm<br>
///					MOD_CHG=���[�h�`�F���W<br>
///					MOD_EXT=�I��
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2019/10/08<br>
///				Update	: 2021/03/26
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
// ushort	 FncChk_QRread( void )
// {
// 	short		msg;				// ��Mү����
// 	uchar		req = 0;		// ýėv�����
// 	ushort		ret = 0;		// �߂�l
// 
// 	dispclr();
// 	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[00] );	//"���p�q���[�_�[�`�F�b�N���@�@�@"
// 	grachr( 2, 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[00] );		// �p�q�R�[�h��
// 	grachr( 3, 0, 24, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[01] );		// "���[�h�e�X�g���s���܂��@�@�@�@
// 	grachr( 5, 0, 28, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[02] );		// �p�q�R�[�h���������ĉ������@�@
// 	Fun_Dsp(FUNMSG[82]);												// "�@�@�@�@�@�@ ���~ �@�@�@�@�@�@", 
// 
// 	if( PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STA) ){		// �ǎ�J�n
// 		req = 1;										// QR���[�_���䉞���҂�
// 	}else{
// 		// ����ޑ��M�m�f
// 		BUZPIPI();
// 		return( MOD_EXT );
// 	}
// 
//	while( req == 1 || ret == 0 ){
//		msg = StoF( GetMessage(), 1 );			// ү���ގ�M
//
//		switch (msg) {							// ��Mү����
//		case LCD_DISCONNECT:					// �ؒf�ʒm
//			BUZPI();
//			PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QR���[�_��~�v��
//			req = 0 ;
//			return MOD_CUT;
//			break;
//		case KEY_MODECHG:						// ����Ӱ�ސؑ�
//			BUZPI();
//			PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QR���[�_��~�v��
//			req = 0 ;
//			return MOD_CHG;
//			break;
//
//		case KEY_TEN_F3:						// ���~ (�I��)
//			BUZPI();
//			PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QR���[�_��~�v��
//			req = 0 ;
//			return MOD_EXT;
//			break;
//
//		case LCD_MNT_QR_READ_RESULT:			// �ǎ挋��
//			PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_RD_STP );		// QR���[�_��~�v��
//			ret = FncChk_QRread_result( LcdRecv.lcdbm_rsp_QR_rd_rslt.result );	//���ʕ\��
//			req = 0 ;
//			break;
//		default:
//			break;
//		}
//	}
//
//	return( 1 );
// }
ushort FncChk_QRread(void)
{
	lcdbm_rsp_QR_rd_rslt_t*	pQR		= &LcdRecv.lcdbm_rsp_QR_rd_rslt;	// QR�ǎ挋��
	QR_AmountInfo*			pAmntQR	= &pQR->QR_data.AmountType;			// QR�f�[�^��� QR���㌔
	short					msg;				// ��M���b�Z�[�W
	ushort					state	= 1;		// �ē����
	ushort					ret		= MOD_EXT;	// �I��
	ushort					updown_page;		// �㉺�ړ��̃y�[�W
	ushort					updown_page_max;	// �㉺�ړ��̃y�[�W�̍ő吔

	dispclr();
	grachr(0, 0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0]);	// "�p�q���[�_�[�`�F�b�N�@�@�@�@�@"
	grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[0]);		// "�p�q�R�[�h�́@�@�@�@�@�@�@�@�@"
	grachr(3, 0, 22, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[1]);		// "���[�h�e�X�g���s���܂��@�@�@�@"
	grachr(5, 0, 26, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[2]);		// "�p�q�R�[�h���������ĉ������@�@"
	Fun_Dsp(FUNMSG[82]);											// "             ���~             "

	Lagtim(OPETCBNO, 6, 1);	// �ǎ�J�n�^�C�}�[�X�^�[�g(����)

	while( state ) {
		msg = StoF( GetMessage(), 1 );	// ���b�Z�[�W��M

		switch ( msg ) {
		case LCD_DISCONNECT:			// �ؒf�ʒm
			BUZPI();
			ret = MOD_CUT;
			state = 0;	// �I��
			break;
		case KEY_MODECHG:				// ����Ӱ�ސؑ�
			BUZPI();
			ret = MOD_CHG;
			state = 0;	// �I��
			break;
		case KEY_TEN_F1:				// ��
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//			if ( state == 2 ) {						// ���ʉ�� �̂�
//				if ( updown_page > 0 ) {
//					updown_page--;
//				} else {
//					updown_page = updown_page_max;
//				}
//				state = FncChk_QRread_result(updown_page);
//			}
			if ( (state == 2) || (state == 5) ) {	// ���ʉ�� or ���ʉ��(��) �̂�
				updown_page--;
				state = FncChk_QRread_result(updown_page, updown_page_max);
			}
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
			break;
		case KEY_TEN_F2:				// ��
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//			if ( state == 2 ) {						// ���ʉ�� �̂�
//				if ( updown_page < updown_page_max ) {
//					updown_page++;
//				} else {
//					updown_page = 0;
//				}
//				state = FncChk_QRread_result(updown_page);
//			}
			if ( (state == 2) || (state == 4) ) {	// ���ʉ�� or ���ʉ��(��) �̂�
				updown_page++;
				state = FncChk_QRread_result(updown_page, updown_page_max);
			}
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
			break;
		case KEY_TEN_F3:				// ���~
			if ( state == 1 ) {						// ����ǎ� �̂�
				BUZPI();
				state = 0;	// �I��
			}
			break;
		case KEY_TEN_F5:				// �I��
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//			if ( (state == 2) || (state == 3) ) {	// ���ʉ�� or ���s��� �̂�
			if ( state != 1 ) {						// �ē���� �ȊO
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
				BUZPI();
				state = 0;	// �I��
			}
			break;
		case LCD_MNT_QR_READ_RESULT:	// QR�ǎ挋��(����ݽ)
			updown_page = 0;
			updown_page_max = UsMnt_QR_GetUpdownPageMax(pQR->id, pAmntQR);
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//			state = FncChk_QRread_result(updown_page);
			state = FncChk_QRread_result(updown_page, updown_page_max);
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
			if ( pQR->result != 0x01 ) {			// �ُ� �ȊO
				Lagtim(OPETCBNO, 6, 100);	// �ǎ�J�n�^�C�}�[�X�^�[�g 2�b(20ms x 100)
			} else {								// �ُ�
				Lagtim(OPETCBNO, 6, 500);	// �ǎ�J�n�^�C�}�[�X�^�[�g 10�b(20ms x 500)
			}
			break;
		case TIMEOUT6:					// �ǎ�J�n�^�C�}�[
			if ( !(PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STA)) ) {	// �ǎ�J�n
				BUZPIPI();
				return MOD_EXT;
			}
			break;
		default:
			break;
		}
	}

	Lagcan(OPETCBNO, 6);	// �ǎ�J�n�^�C�}�[�X�g�b�v
	PKTcmd_mnt_qr_ctrl_req(LCDBM_QR_CTRL_RD_STP);	// �ǎ��~

	return ret;
}
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P

//[]----------------------------------------------------------------------[]
/// @brief		QR�ǂݎ��e�X�g(�ǎ挋��)
//[]----------------------------------------------------------------------[]
///	@param		updown_page		: �㉺�ړ��̃y�[�W
///	@param		updown_page_max	: �㉺�ړ��̃y�[�W�̍ő吔
/// @return 	state			: ���<br>
///					0=�I��<br>
///					1=�ē����<br>
///					2=���ʉ��<br>
///					3=���s���<br>
///					4=���ʉ��(��)<br>
///					5=���ʉ��(��)
/// @attention	
/// @note		
//[]----------------------------------------------------------------------[]
/// @date 		Create	: 2019/10/08<br>
///				Update	: 2021/03/26
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810102(S) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P
// static ushort	FncChk_QRread_result(uchar result)
// {
// 	ushort		msg;				// ��Mү����
//
// 	dispclr();
// 	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0] );	// ���p�q���[�_�[�`�F�b�N���@�@�@",
// 	grachr( 2, 0, 14, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[00] );	//�p�q�R�[�h��
//
// 	if( result == LCDBM_RESUTL_OK ){
// 		grachr( 3, 0, 24, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[03] );	//���[�h����I���@�@�@�@�@�@�@�@", 
// 		BUZPIPI();
// 	}else{
// 		grachr( 3, 0, 24, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[04] );	//"�ǎ�Ɏ��s���܂����@�@�@�@�@�@",  
// 		grachr( 5, 0, 26, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[5] );	//		"���[�_�[���́@�@�@�@�@�@�@�@�@",  
// 		grachr( 6, 0, 26, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6] );	//	"�p�q�R�[�h���m�F���ĉ������@�@", 
// 		BUZPIPI();
// 	}
// 	Fun_Dsp(FUNMSG[8]);						//	"�@�@�@�@�@�@�@�@�@�@�@�@ �I�� ", 
// 	while(1){
// 		msg = StoF( GetMessage(), 1 );			// ү���ގ�M
// 		switch( msg ){		// FunctionKey Enter
// 		case LCD_DISCONNECT:
// 			return MOD_CUT;
// 		case KEY_MODECHG:
// 			BUZPI();
// 			return MOD_CHG;
// 			break;
// 		case KEY_TEN_F5:		// �u�I���v
// 			BUZPI();
// 			return MOD_EXT;
// 			break;				
// 		default:
// 			break;
// 		}
// 	}
// }
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
// static ushort FncChk_QRread_result(ushort updown_page)
static ushort FncChk_QRread_result(ushort updown_page, ushort updown_page_max)
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
{
	lcdbm_rsp_QR_rd_rslt_t*	pQR		= &LcdRecv.lcdbm_rsp_QR_rd_rslt;	// QR�ǎ挋��
	QR_DiscountInfo*		pDisQR	= &pQR->QR_data.DiscountType;		// QR�f�[�^��� QR������
	QR_AmountInfo*			pAmntQR	= &pQR->QR_data.AmountType;			// QR�f�[�^��� QR���㌔
// GG129000(S) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
	QR_TicketInfo*			pTicQR  = &pQR->QR_data.TicketType;			// QR�f�[�^��� QR���Ԍ�
// GG129000(E) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//	ushort					state	= 3;								// ���s���
	ushort					state	= 2;								// ���ʉ��
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�

// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
	grachr(0, 0, 20, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0]);	// "�p�q���[�_�[�`�F�b�N�@�@�@�@�@"
	dispmlclr(1, 6);
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P

// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
	if ( updown_page == 0 ) {
		state = 4;	// ���ʉ��(��)
	} else if ( updown_page >= updown_page_max ) {
		state = 5;	// ���ʉ��(��)
	}
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�

	switch( pQR->result ) {
	case 0x00:	// ����
		switch( pQR->id ) {
		case BAR_ID_DISCOUNT:	// ������
			BUZPI();
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//			dispmlclr(2, 6);
			grachr(0, 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[1]);	// "�F������"
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
//			UsMnt_QR_DispDisQR(pDisQR, updown_page);
			UsMnt_QR_DispDisQR(pDisQR, updown_page, pQR->qr_type);
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//			Fun_Dsp(FUNMSG[6]);										// "  ��    ��               �I�� "
//			state = 2;	// ���ʉ��
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
			break;
		case BAR_ID_AMOUNT:		// ���㌔
			BUZPI();
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//			dispmlclr(2, 6);
			grachr(0, 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[0]);	// "�F���㌔"
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// GG129000(S) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
//			UsMnt_QR_DispAmntQR(pAmntQR, updown_page);
			UsMnt_QR_DispAmntQR(pAmntQR, updown_page, pQR->qr_type);
// GG129000(E) H.Fujinaga 2023/01/10 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ʋ��ʃt�H�[�}�b�gQR�������Ή��j
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//			Fun_Dsp(FUNMSG[6]);										// "  ��    ��               �I�� "
//			state = 2;	// ���ʉ��
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
			break;
// GG129000(S) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
		case BAR_ID_TICKET:		// ���Ԍ�
			BUZPI();
			grachr(0, 20, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_ID_STR[2]);	// "�F���Ԍ�"
			UsMnt_QR_DispTicQR(pTicQR, updown_page);
			state = 3;	// QR���Ԍ���1�y�[�W�̂�
			break;
// GG129000(E) H.Fujinaga 2023/01/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/QR�f�[�^�\���j
		default:
			BUZPIPI();
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
			state = 3;	// ���s���
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
			break;
		}

		break;
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//	case 0x01:	// �ُ�
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
	case 0x04:	// �t�H�[�}�b�g�s��
	case 0x05:	// �ΏۊO
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
		BUZPIPI();
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
		state = 3;	// ���s���
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
		break;
	case 0x01:	// �ُ�
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
	default:
		BUZPIPI();
// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
		grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[0]);	// "�p�q�R�[�h�́@�@�@�@�@�@�@�@�@"
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[4]);	// "���[�h�e�X�g�Ɏ��s���܂����@�@"
		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[5]);	// "�p�q�R�[�h���[�_�[�Ɂ@�@�@�@�@"
		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6]);	// "��肪�������m�F���ĉ������@�@"
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
		state = 3;	// ���s���
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
		break;
	}

// MH810102(S) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�
//	if ( state == 3 ) {	// ���s���
//// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
////		dispmlclr(2, 6);
////		grachr(2, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[0]);	// "�p�q�R�[�h�́@�@�@�@�@�@�@�@�@"
////		grachr(3, 0, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[4]);	// "�ǎ�Ɏ��s���܂����@�@�@�@�@�@"
////		grachr(5, 0, 12, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[5]);	// "���[�_�[���́@�@�@�@�@�@�@�@�@"
////		grachr(6, 0, 26, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[6]);	// "�p�q�R�[�h���m�F���ĉ������@�@"
//// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
//		Fun_Dsp(FUNMSG[8]);											// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
//	}
	switch ( state ) {
	case 3:	// ���s���
		Fun_Dsp(FUNMSG[8]);											// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
		break;
	case 4:	// ���ʉ��(��)
		Fun_Dsp(FUNMSG[11]);										// "        ��               �I�� "
		break;
	case 5:	// ���ʉ��(��)
		Fun_Dsp(FUNMSG[12]);										// "  ��                     �I�� "
		break;
	case 2:	// ���ʉ��
	default:
		Fun_Dsp(FUNMSG[6]);											// "  ��    ��               �I�� "
		break;
	}
// MH810102(E) R.Endo 2021/04/09 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5473 �yCR���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\���Ńf�[�^�\���ؑւ���ɂ��Ȃ�

// MH810102(S) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P
	UsMnt_QR_ErrDisp(Lcd_QR_ErrDisp(pQR->result));
// MH810102(E) R.Endo 2021/04/01 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5454 �yPK���ƕ��v���z�����e�i���X��QR�ǎ挋�ʕ\�����e���P

	return state;
}
// MH810102(E) R.Endo 2021/03/26 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5360 �yPK���ƕ��v���z�V�X�e�������e�i���X��QR���[�_�[�`�F�b�N�̉��P

//[]----------------------------------------------------------------------[]
/// @brief QR�o�[�W�����m�F
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort	FncChk_QRverChk(void)
{
	ushort	msg;
	ushort	req	= 0;		// ýėv�����
	uchar	ret = 0;

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0]);		// "���p�q���[�_�[�`�F�b�N���@�@�@"	
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[1]);		// "�y�o�[�W�����m�F�z�@�@�@�@�@�@", 
	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[9]);			// "�@�@���΂炭���҂��������@�@�@",
	Fun_Dsp(FUNMSG[00]);												// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@",
	
	if(PKTcmd_mnt_qr_ctrl_req( LCDBM_QR_CTRL_VER )){					// �ް�ޮݗv��
		Lagtim( OPETCBNO, 6, 10*50 );									// QR�ް�ޮ݊m�F���- 6(10s)�N��		
		req = 1;													// QR���[�_���䉞���҂�
	}else{
		BUZPIPI();
		return( MOD_EXT ); // ����ޑ��M�m�f
	}
	
	for ( ;ret == 0; ) {
		msg = StoF( GetMessage(), 1 );

		switch (msg) {
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );													// QR�ް�ޮ݊m�F���-ؾ��
				return MOD_CUT;
				break;
			case KEY_MODECHG:
				BUZPI();
				Lagcan( OPETCBNO, 6 );													// QR�ް�ޮ݊m�F���-ؾ��
				return MOD_CHG;
				break;
			case LCD_MNT_QR_CTRL_RESP:													// QRذ�ސ��䉞��(����ݽ)
				if(LcdRecv.lcdbm_rsp_QR_ctrl_res.ctrl_cd == LCDBM_QR_CTRL_VER ){			// QRذ�ސ��䉞���i�o�[�W�����m�F�j
					if(req){
						Lagcan( OPETCBNO, 6 );
					 	ret = FncChk_QRverchk_result( LcdRecv.lcdbm_rsp_QR_ctrl_res.result );	// ���ʕ\��
						BUZPI();
						req = 0;
					}
				}
				break;	
			case TIMEOUT6:
				if(req){
					ret = FncChk_QRverchk_result( LCDBM_RESUTL_NG );
				}
				req = 0;
				break;
			case KEY_TEN_F5:															// �I��
				if(!req){
					BUZPI();
					Lagcan( OPETCBNO, 6 );												// QR�ް�ޮ݊m�F��ϰؾ��
					return MOD_EXT;
				}
				break;
			default:
				break;
		}
	}
	return( 1 );
}

//[]----------------------------------------------------------------------[]
/// @brief QR�o�[�W�����m�F���ʕ\��
//[]----------------------------------------------------------------------[]
/// @param		: Nones
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/11/29<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort	FncChk_QRverchk_result_info(uchar page)
{
	#define MAX_ONE_COL	28
	ushort	cnt = 0;
	uchar	tmpBuff[35];
	
	memset( tmpBuff, 0, sizeof(tmpBuff) );
// MH810100(S) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)
	dispmlclr( 3, 5 );
// MH810100(E) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)	

	switch (page) {
		case QR_HINBAN:
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[16]);	// �i��
			memcpy( tmpBuff, LcdRecv.lcdbm_rsp_QR_ctrl_res.part_no, sizeof(LcdRecv.lcdbm_rsp_QR_ctrl_res.part_no) );
			break;

		case QR_SERIAL:
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[17]);	// �V���A���ԍ�
			memcpy( tmpBuff, LcdRecv.lcdbm_rsp_QR_ctrl_res.serial_no, sizeof(LcdRecv.lcdbm_rsp_QR_ctrl_res.serial_no) );
			break;

		case QR_VERSION:
			grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[13]);	// �o�[�W����
			memcpy( tmpBuff, LcdRecv.lcdbm_rsp_QR_ctrl_res.version, sizeof(LcdRecv.lcdbm_rsp_QR_ctrl_res.version) );
			break;
		default:
			break;
	}
	cnt = strlen( ( const char* )tmpBuff );
	if( cnt > MAX_ONE_COL ){
		cnt -= MAX_ONE_COL;
		grachr( 4, 2, MAX_ONE_COL,	0, COLOR_BLACK, LCD_BLINK_OFF, &tmpBuff[0] );
		grachr( 5, 2, cnt, 			0, COLOR_BLACK, LCD_BLINK_OFF, &tmpBuff[MAX_ONE_COL] );
	}else{
		grachr( 4, 2, cnt,			0, COLOR_BLACK, LCD_BLINK_OFF, &tmpBuff[0] );
	}
	return(1);
}
//[]----------------------------------------------------------------------[]
/// @brief QR�o�[�W�����m�F����
//[]----------------------------------------------------------------------[]
/// @param		: result ��������
/// @return 	: MOD_CHG / MOD_EXT / MOD_CUT
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/11/29<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort	FncChk_QRverchk_result(uchar result){
	ushort		msg;		// ��Mү����
// MH810100(S) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)
	uchar		dsp = 0;	// 0:�`��Ȃ�/ 1:��ʕ\��
// MH810100(E) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:579,580,581)�Ή�
//	int			page;		// �y�[�W�ԍ�
	int			page = QR_HINBAN;		// �y�[�W�ԍ�
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:579,580,581)�Ή�
	
	dispclr();
// MH810100(S) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[0]);		// "���p�q���[�_�[�`�F�b�N���@�@�@"	
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_TITLE[1]);		// "�y�o�[�W�����m�F�z�@�@�@�@�@�@",   

	// ����
	if( result == LCDBM_RESUTL_OK ){
		Fun_Dsp(FUNMSG[123]);											//"�@���@�@���@�@�@�@�@�@�@ �I�� ",
		dsp = 1;
	}
	// ���s
	else{
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[14]);	// "�o�[�W�����̎擾��",  
		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[18]);	// "�@���s���܂����@�@�@�@�@�@�@�@",
		Fun_Dsp(FUNMSG[8]);												// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� ", 
		BUZPIPI();
	}
// MH810100(E) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)
	while(1){
// MH810100(S) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)
		if (dsp == 1) {
// MH810100(E) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)			
			dsp = 0;
			// �o�[�W�����ڍ׉�ʕ\��
// MH810100(S) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)
			FncChk_QRverchk_result_info( page );
			BUZPI();
// MH810100(E) Y.Yamauchi 2020/03/12 �Ԕԃ`�P�b�g���X (#4003 QR�o�[�W�����`�F�b�N��[�i��]��[�V���A���ԍ�]��[�o�[�W����]�̏�񂪎擾�ł��Ȃ��B)
		}
		// ���b�Z�[�W��M
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
			case LCD_DISCONNECT:
				return MOD_CUT;
				break;

			case KEY_MODECHG:
				BUZPI();
				return MOD_CHG;
				break;

			case KEY_TEN_F1:	// �y�[�W�ړ��i���j
				if( result == LCDBM_RESUTL_OK ){
					if (--page < 0) {
						page = QR_VER_RESULT_MAX - 1;
					}
					dsp = 1;
				}
				break;

			case KEY_TEN_F2:	// �y�[�W�ړ��i���j
				if( result == LCDBM_RESUTL_OK ){
					if (++page >= QR_VER_RESULT_MAX) {
						page = QR_HINBAN ;
					}
					dsp = 1;
				}
				break;

			case KEY_TEN_F5:		// �u�I���v
				BUZPI();
				return MOD_EXT;
				break;

			default:
				break;
		}
	}
}
//[]----------------------------------------------------------------------[]
/// @brief ���A���^�C���ʐM�`�F�b�N���j���[	
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
enum {
	UNSENDDATA_REAL_CLEAR = 0,
	// UNSENDDATA_REAL_SENT,
	CENTER_REAL_COM_TEST,
};
ushort 	FncChk_RealChk( void ) 
{
	ushort	msg;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];

	DP_CP[0] = DP_CP[1] = 0;
	while (1) {
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[58] );		// "�����A���^�C���ʐM�`�F�b�N���@", 
		msg = Menu_Slt( REALTI_MENU, REAL_CHK_TBL, (char)REAL_CHK_MAX, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch (msg) {

			// �����M�f�[�^�N���A
			case UNSEND_REAL_CLEAR:
				msg = RealT_FncChk_Unsend(UNSENDDATA_REAL_CLEAR);
				break;

			// �����M�f�[�^���M�͕s�v�ƂȂ������A�O�ׁ̈A�����̓R�����g�A�E�g�Ŏc���Ă���
			// case UNSEND_REAL_SENT:
			// 	msg = RealT_FncChk_Unsend(UNSENDDATA_REAL_SENT);	// �����M�f�[�^���M
			// 	break;

			// �Z���^�[�ʐM�e�X�g
			case CENTER_REAL_TEST:
				msg = FncChk_CenterChk();
				break;

			// �Z���^�[�ǔԃN���A
			case REAL_OIBANCLR:
				msg = FncChk_realt_OibanClear();
				break;

			case MOD_EXT:								// �I���i�e�T�j
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return msg;

			default:
				break;
		}
		if (msg == MOD_CHG || msg == MOD_CUT ){				// ���[�h�`�F���W�������͐ؒf�ʒm
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return msg;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}
//[]----------------------------------------------------------------------[]
/// @brief ���A���^�C���ʐM�����M�f�[�^�N���A	
//[]----------------------------------------------------------------------[]
/// @param		: type	0:�����M�f�[�^�N���A/1:�����M�f�[�^���M(������)
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort RealT_FncChk_Unsend( int type )
{
	ushort	msg;
	uchar	mode;		// 0:�����\�����
						// 1:�m�F���

	uchar	dsp;		// 0:�`��Ȃ�
						// 1:�����X�V
						// 2:��ʍX�V
						// 3:�����\��

	int		item_pos;	// �J�[�\���ʒu
	ushort	pos;
	uchar	fkey_enb = 0;
	uchar	page = 0;

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	uchar RT_POS_TBL[RT_DATA_MAX] =
	{
		4,	// ���Z
		6,	// �̎���
	};
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	dispclr();
	// �^�C�g���\��
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_TITLE[type] );	// "���Z���^�[�����M�f�[�^�N���A��"
	dsp      = 2;	// ��ʍX�V(�����܂�)
	mode     = 0;	// �����\�����
	item_pos = 0;	// �ʒu

	while(1){
		// �����\���X�V�v������
		if( dsp ){
			// �����\����ʁH
			if( mode == 0 ){
				// �����\��
				if( dsp == 2 || dsp == 3 ){
					realt_clr_dsp_count(page);						// �\���N���A("����   ��"�����ɂ���)
					real_dsp_unflushed_count(0, page);				// ������\��
					realt_dsp_ccomchk_cursor(item_pos, 1, page);	// ���]�\��
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//					Fun_Dsp(FUNMSG[(121+type)]);					// " �ر�@�S�ر �@���@       �I�� "
					Fun_Dsp(FUNMSG[(92)]);							// " �ر  �S�ر   ��    ��   �I�� "
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// MH810103(s) �d�q�}�l�[�Ή� ����`�F�b�N�i�����M�N���A�{�^���C���j
//// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:577)�Ή�
////					fkey_enb == 1;
//					fkey_enb = 1;
//// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:577)�Ή�
// MH810103(e) �d�q�}�l�[�Ή� ����`�F�b�N�i�����M�N���A�{�^���C���j
				}
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				Lagcan(OPETCBNO, 7);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
				Lagcan(OPETCBNO, 7);
				return MOD_CHG;

			case KEY_TEN_F5:			// �u�I���v
				if( mode == 0 ){		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
					BUZPI();
					Lagcan( OPETCBNO, 7 );
					return MOD_EXT;
				}
				else if( mode == 3 ){	// �G���[���
					BUZPI();
					dsp  = 2;			// 2:��ʍX�V
					mode = 0;			// 0:�����\�����
				}
				break;

			case KEY_TEN_F1:		// �N���A
				// �����\����ʁH
				if( mode == 0 ){
					// �t�@���N�V�����L�[�����H
					if (fkey_enb != 0){
						break;
					}
					BUZPI();
					dispmlclr(1, 6);						// �P�`�U�s�ڂ܂ŃN���A
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//					pos = (!(item_pos % 2) || (item_pos == CCOM_DATA_TOTAL)) ? 4 : 8;
//					grachr( 2,  2, pos, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[0] );				// "�I������"
					pos = RT_POS_TBL[item_pos];
					grachr( 2,  2, pos, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[item_pos] );		// "�I������"
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:582)�Ή�
//					grachr( 2, (ushort)(pos+2), 31, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_STR[type] );	// "�f�[�^���������܂�"
					grachr( 2, (ushort)(pos+2), 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_STR[type] );	// "�f�[�^���������܂�"
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:582)�Ή�
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
					Fun_Dsp(FUNMSG[19]);																	// "�@�@�@�@�@�@ �͂� �������@�@�@"
					mode = 1;																				// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
					}
				break;

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//			case KEY_TEN_F3:					// �u�͂��v
//				if( mode == 1 ){	// �m�F���
//					BUZPI();
//					switch( type ){
//						case UNSENDDATA_REAL_CLEAR:	// �����M�f�[�^�N���A
//							Ope_Log_UnreadToRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// �����MRT���Z�f�[�^�𑗐M�ς݂Ƃ���
//							break;
//						// �����M�f�[�^���M�͕s�v�ƂȂ������A�O�ׁ̈A�����̓R�����g�A�E�g�Ŏc���Ă���
//						// case UNSENDDATA_REAL_SENT:	// �����M�f�[�^���M
//						// 	break;
//						default:
//							break;
//					}
//					dsp  = 2;			// ��ʍĕ`��
//					mode = 0;			// 0:�����\�����
//				}
//				break;
			case KEY_TEN_F2:		// �S�N���A
				// �����\����ʁH
				if ( mode == 0 ) {
					// �t�@���N�V�����L�[�����H
					if ( fkey_enb != 0 ) {
						break;
					}
					BUZPI();
					dispmlclr(1, 6);						// �P�`�U�s�ڂ܂ŃN���A

					grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[RT_DATA_MAX] );	// "�S"
					grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[0] );					// "�f�[�^���������܂�"
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );					// "�@�@�@�@��낵���ł����H�@�@�@"
					Fun_Dsp(FUNMSG[19]);															// "�@�@�@�@�@�@ �͂� �������@�@�@"
					mode = 2;		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				}
				break;

			case KEY_TEN_F3:
				switch ( mode ) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
					case 0:			// �J�[�\���ړ�
						BUZPI();
						realt_dsp_ccomchk_cursor( item_pos, 0, page );		// ���]�\��
						if ( ++item_pos >= RT_DATA_MAX ) {
							item_pos = 0;
						}
						realt_dsp_ccomchk_cursor( item_pos, 1, page );		// ���]�\��
						break;
					case 1:			// �N���A
						BUZPI();
						switch ( item_pos ) {
							case 0:		// QR�m��
								Ope_Log_UnreadToRead( eLOG_RTPAY, eLOG_TARGET_LCD );
								break;
							case 1:		// ڰ����
								Ope_Log_UnreadToRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );
								break;
							default:
								break;
						}
						dsp  = 2;		// ��ʍĕ`��
						mode = 0;
						break;
					case 2:			// �S�N���A
						BUZPI();
						Ope_Log_UnreadToRead( eLOG_RTPAY, eLOG_TARGET_LCD );
						Ope_Log_UnreadToRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );
						dsp  = 2;		// ��ʍĕ`��
						mode = 0;
						break;
					case 3:
					default:
						break;
				}
				break;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

			case KEY_TEN_F4:						// �u�������v
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//				if( mode == 1){	// �m�F���
				if ( (mode == 1) || (mode == 2) ) {
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					BUZPI();
					dsp  = 2;	// ��ʍĕ`��
					mode = 0;	// 0:�����\�����
				}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				else if ( mode == 0 ) {
					// �J�[�\���ړ�
					BUZPI();
					realt_dsp_ccomchk_cursor( item_pos, 0, page );		// ���]�\��
					if ( --item_pos < 0 ) {
						item_pos = RT_DATA_MAX - 1;
					}
					realt_dsp_ccomchk_cursor( item_pos, 1, page );		// ���]�\��
				}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
				break;

			default:
				break;
		}
	}
}

#define DATA_LINE_MAX	6
#define PAGE_ITEM_MAX	(DATA_LINE_MAX * 2)
// ������������
void	realt_clr_dsp_count(uchar page)
{
	ushort i, j;
	ushort offset = page * PAGE_ITEM_MAX;

	memset(RT_PreCount, 0xFF, sizeof(ulong)*RT_DATA_MAX);
	dispmlclr(1, 6);
	for (i=1, j=offset; (i<=DATA_LINE_MAX) && (j<RT_DATA_MAX); i++) {
		grachr( i,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[j++] );
		grachr( i, 11, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[RT_DATA_MAX] );	// "��"
		
		if ((i <= DATA_LINE_MAX) && (j < RT_DATA_MAX)) {
				if( (i == DATA_LINE_MAX) || (page > 0) ) {
					break;
				}
		}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//		if( RT_DATA_MAX > 6 ){																	//�����̍��ڂ�6�ȏ�ł���ΉE���̍��ڂ�\��
		if((i*2) <= RT_DATA_MAX){
			// �E���\��
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			grachr( i, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[j++] );			//����
			grachr( i, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[RT_DATA_MAX]  );	// "��"
		}
	}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	Fun_Dsp(FUNMSG[8]);		// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
	Fun_Dsp(FUNMSG[128]);	// "            ��    ��   �I�� "
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
}

// �����M�f�[�^�����擾���ĕ\������
ushort real_dsp_unflushed_count( uchar type, uchar page )
{
	ushort count = 0;
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	ushort count_receipt = 0;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	char  numStr[10];

	// RT���Z�f�[�^���擾
	count = Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD );
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	count_receipt = Ope_Log_UnreadCountGet(eLOG_RTRECEIPT, eLOG_TARGET_LCD);
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	// �����\��
	if( count != RT_PreCount[RTCOM_DATA_PAY] ){
		memset( numStr, 0x00, sizeof(numStr) );
		sprintf( numStr, "%4d", count );
		// �R��
		grachr( 1, 7, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
		RT_PreCount[RTCOM_DATA_PAY] = count;
	}

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	// �����\��(�̎��؃f�[�^)
	if ( count_receipt != RT_PreCount[RTCOM_DATA_RECEIPT] ) {
		memset(numStr, 0x00, sizeof(numStr));
		sprintf(numStr, "%4d", count_receipt);
		// �R��
		grachr(1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr);
		RT_PreCount[RTCOM_DATA_RECEIPT] = count_receipt;
	}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	return 0;
}
// �J�[�\���\��
uchar realt_dsp_ccomchk_cursor(int index, ushort mode, uchar page)
{
	ushort row  = (ushort)((index/2)%DATA_LINE_MAX+1);
	uchar new_page;
	ushort col  = (ushort)(index%2);
	ushort size = 8;
	new_page = index / PAGE_ITEM_MAX;
	if (new_page != page) {
		realt_clr_dsp_count(new_page);
	}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF,REAL_UNSEND_TITLE[new_page]);
	grachr(row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, REAL_UNSEND_TITLE[index]);
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	return new_page;
}

static uchar ChkNo;		// �ʐM�`�F�b�N�v���f�[�^�d���쐬�p�̒ʐM����No.

//[]----------------------------------------------------------------------[]
/// @brief ���A���^�C���ʐM�Z���^�[�ʐM�e�X�g
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
/// @attention	: 
/// @note		: 
//[]----------------------------------------------------------------------[]
/// @author 	: Yamauchi
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort FncChk_CenterChk( void )
{
	ushort	msg;
	ushort	req	= 0;		// 1:���ʗv����ԁA2:���ʕ\�����

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_TITLE[1] );	// "���Z���^�[�ʐM�e�X�g���@�@�@�@"
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[4] );		// "�Z���^�[�ʐM�e�X�g���s���܂��@"
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[5] );		// "��낵���ł����H�@�@�@�@�@�@�@"
	Fun_Dsp( FUNMSG[19] );												// "�@�@�@�@�@�@ �͂� �������@�@�@"


	while(1) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case LCD_DISCONNECT:
			Lagcan( OPETCBNO, 6 );												// ر���ђʐM�m�F���-ؾ��
			return MOD_CUT;
			break;
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
		case KEY_MODECHG:
			BUZPI();
			Lagcan( OPETCBNO, 6 );												// ر���ђʐM�m�F���-ؾ��
			return MOD_CHG;
			break;

		case KEY_TEN_F4:														//������
			if( !req ){	
				BUZPI();
				return MOD_EXT;
			}
			break;
		case KEY_TEN_F3:														//�͂�
			if( !req ){
				BUZPI();
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, FCSTR1[46] );		// "�@�@�@�@ �ʐM�e�X�g�� �@�@�@�@"
				grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[9] );		// "�@�@���΂炭���҂��������@�@�@"
				Fun_Dsp( FUNMSG[00]);												// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
				// ���A���^�C���ʐM�R�}���h�Z�b�g
				if( PKTcmd_mnt_rt_con_req() ){
					Lagtim( OPETCBNO, 6, 10*50 );									// ر���ђʐM�m�F���- 6(10s)�N��
					req = 1;														// ر���ђʐM���䉞���҂�
				}
				else{
					return( MOD_EXT ); // ����ޑ��M�m�f
				}
			}
			break;

		// ر���ђʐM�a�ʌ���
		case LCD_MNT_REALTIME_RESULT:
			if( req == 1 ){
				Lagcan( OPETCBNO, 6 );												// ر���ђʐM�m�F���- 6(10s)��~
				BUZPI();
				Fun_Dsp(FUNMSG[8]);													// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� ",
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );		// "�@�@�@�@�ʐM�e�X�g�I���@�@�@�@"

				displclr(3);
				if( LcdRecv.lcdbm_rsp_rt_con_rslt.result == LCDBM_RESUTL_OK ){	// (00H:����,01H:�ʐM�ُ�(TCP�ڑ��s��),02H:�^�C���A�E�g)
					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[00]);	// "�Z���^�[�Ƃ̒ʐM�͐���ł��@�@",
				}
				else if( LcdRecv.lcdbm_rsp_rt_con_rslt.result == LCDBM_RESUTL_TIMEOUT ){
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1] );	// "�Z���^�[�Ƃ̒ʐM�Ɏ��s���܂���",
					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[3]);	// "(02�F�^�C���A�E�g)�@�@�@�@�@�@",
				}
				else{
					grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1] );	// "�Z���^�[�Ƃ̒ʐM�Ɏ��s���܂���",
					grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[2] );	// "(01�F�ʐM�ُ�(TCP�ڑ��s��))�@",
				}
			}
			req = 2;
			break;	

		case TIMEOUT6:
			if(req == 1){
				BUZPI();
				displclr(3);
				Lagcan( OPETCBNO, 6 );												// ر���ђʐM�m�F���- 6(10s)��~
				grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );		// "�@�@�@�@�ʐM�e�X�g�I���@�@�@�@"
				grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1]);		// "�Z���^�[�Ƃ̒ʐM�Ɏ��s���܂���"
				grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[3]);		// "(02�F�^�C���A�E�g)�@�@�@�@�@�@"
				Fun_Dsp(FUNMSG[8]);													// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
			}
			req = 2;
			break;

		case KEY_TEN_F5:	// �I��
			if(req == 2){
				BUZPI();
				Lagcan( OPETCBNO, 6 );												// ر���ђʐM�m�F���- 6(10s)��~
				return MOD_EXT;
			}
			break;
		default:
			break;
		}
	}
}

//[]----------------------------------------------------------------------[]
/// @brief ���A���^�C���ʐM�Z���^�[�ǔԃN���A�Q��ʑI�����
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void realt_OibanClrDsp( void )
{
	dispmlclr(1, 6);

	// �P�s��
	grachr( 1,  0, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[REAL_SEQNO_PAY] );		// "���Z�@�@"
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
	// 1�s��
	grachr( 1, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[REAL_SEQNO_PAY_AFTER] );		// "������Z"
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	grachr(2, 0, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[REAL_SEQNO_RECEIPT]);	// "�̎��؁@"
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
}

//[]----------------------------------------------------------------------[]
/// @brief���A���^�C���ʐM�Z���^�[�ǔԃN���A�Q���]����
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void realt_OibanCursorDsp( uchar index, ushort mode )
{
	ushort row;
	ushort col;
	ushort size = 4;

	if( index >= REAL_SEQNO_TYPE_COUNT ){
		return;
	}

	row = (ushort)(index/2+1);
	col = (ushort)(index%2);
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
	if(index == 1 ){
		size = 8;
	}
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	if ( index == 2 ) {
		size = 6;
	}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[index] );
}

//[]----------------------------------------------------------------------[]
/// @brief ���A���^�C���ʐM�Z���^�[�ǔԃN���A
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static ushort FncChk_realt_OibanClear( void )
{
	ushort	msg;
	uchar	mode = 0;		// 0:��ʑI�����/ 1:�N���A�m�F���
	uchar	dsp = 1;		// 0:�`��Ȃ�/ 1:��ʕ\��
	uchar	item_pos = 0;	// �J�[�\���ʒu
	ushort	pos;
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
	ushort  i = 0;
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�

	// �^�C�g���\��
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[8] );		// "���Z���^�[�ǔԃN���A���@�@�@�@"

	for ( ; ; ) {
		// �����\���X�V�v������
		if( dsp ){
			if( mode == 0 ){
				realt_OibanClrDsp();					// ��ʑI����ʕ\��
				realt_OibanCursorDsp( item_pos, 1 );	// ���]�\��
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
//				Fun_Dsp(FUNMSG[121]);				// " �ر                     �I�� "
				Fun_Dsp(FUNMSG[119]);				// " �ر�@�S�ر �@���@       �I�� "
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter

			case LCD_DISCONNECT:
				return MOD_CUT;

			case KEY_MODECHG:
				return MOD_CHG;

			// �u�I���v
			case KEY_TEN_F5:
				// ��ʑI����ʁH
				if( mode == 0 ){
					BUZPI();
					return MOD_EXT;
				}
				break;

			case KEY_TEN_F1:		// �N���A
				// ��ʑI����ʁH
				if( mode == 0 ){
					BUZPI();
					dispmlclr( 1, 6 );
					pos = 4;
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
					if( item_pos == 1 ){
						pos = 8;	// 4�����Ȃ̂�8Byte
					}
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					if ( item_pos == 2 ) {
						pos = 6;
					}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					grachr( 2,    2,  pos, 0, COLOR_BLACK, LCD_BLINK_OFF, REALSTR5[item_pos] );				// "�I������"
					grachr( 2, (ushort)(pos+2), 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );			// "�ǔԂ��N���A���܂�"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
					Fun_Dsp(FUNMSG[19]);																	// "�@�@�@�@�@�@ �͂� �������@�@�@"
					mode = 1;		// 0:��ʑI�����/ 1:�N���A�m�F���
				}
				break;
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
			case KEY_TEN_F2:		// �S�N���A
				if (mode == 0) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
					BUZPI();
					
					dispmlclr(1, 6);
					grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR1[CCOM_DATA_MAX] );				// "�S"
					grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );							// "�ǔԂ��N���A���܂�"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
					Fun_Dsp(FUNMSG[19]);								// "�@�@�@�@�@�@ �͂� �������@�@�@"
					mode = 2;		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				}
				break;
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�

			case KEY_TEN_F3:
				switch( mode ){
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
					case 0:			// �J�[�\���ړ�
						BUZPI();
						realt_OibanCursorDsp(item_pos, 0);			// ���]�\��
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//						if (++item_pos >= 2) {
						if ( ++item_pos >= REAL_SEQNO_TYPE_COUNT ) {
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
							item_pos = 0;
						}
						realt_OibanCursorDsp(item_pos, 1);			// ���]�\��
						break;
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
					// �N���A�m�F��ʁH
					case 1:
						BUZPI();
						switch( item_pos ){
							case 0:									// �Z���^�[�ǔԁi���Z�j
// MH810100(S) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4021 �Z���^�[�ǔԃN���A��A�s���t���O�������Ȃ�)
//								REAL_SeqNo[REAL_SEQNO_PAY] = 1;
								REAL_SeqNo[REAL_SEQNO_PAY] = 0;
// MH810100(E) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4021 �Z���^�[�ǔԃN���A��A�s���t���O�������Ȃ�)
								break;
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
							case 1:
								REAL_SeqNo[REAL_SEQNO_PAY_AFTER] = 0;
								break;
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
							case 2:
								REAL_SeqNo[REAL_SEQNO_RECEIPT] = 0;
								break;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
						}
						dsp  = 1;	// ��ʍĕ`��
						mode = 0;
						break;
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
					case 2:			// �S�N���A���s
						BUZPI();
						for(i = 0; i < REAL_SEQNO_TYPE_COUNT; ++i) {
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
//							REAL_SeqNo[i] = 1;
							REAL_SeqNo[i] = 0;
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
						}
						dsp  = 1;	// ��ʍĕ`��
						mode = 0;
						break;
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
					default:
						break;
				}
				break;

			// �u�������v
			case KEY_TEN_F4:
				// �N���A�m�F��ʁH
// MH810100(S) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
//				if( mode == 1 ){
				if( mode == 1 || mode == 2){
// MH810100(E) 2020/11/02 #4971 �����Z�o�ɂ̃Z���^�[�ǔԂ𕪂���Ή�
					BUZPI();
					dsp  = 1;	// ��ʍĕ`��
					mode = 0;
				}
				break;

			default:
				break;
		}
	}
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET�ʐM�`�F�b�N���j���[
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/27<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort FncChk_DC_Chk( void )
{
	ushort	msg;
	char	wk[2];
	char	org[2];

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];

	DP_CP[0] = DP_CP[1] = 0;

	while (1) {

		// �^�C�g���\��
		dispclr();
		grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, FCSTR1[61] );		// DC-NET�ʐM�`�F�b�N

		msg = Menu_Slt( DC_NET_MENU, DCNET_CHK_TBL, (char)DC_NET_CHK_TBL, (char)1 );
		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch( msg ){

			// �����M�f�[�^�N���A
			case DC_UNSEND_CLEAR:
				msg = FncChk_DC_UnsendClear();
				break;

			// �Z���^�[�ʐM�e�X�g
			case DC_CONNECT_TEST:
				msg = FncChk_DC_ConnectCheck();
				break;

			// �Z���^�[�ǔԃN���A
			case DC_OIBANCLR:
				msg = FncChk_DC_OibanClear();
				break;

			// �I��(F5)
			case MOD_EXT:
				DP_CP[0] = org[0];
				DP_CP[1] = org[1];
				return msg;

			default:
				break;
		}
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
//		if( msg == MOD_CHG ){
		if( msg == MOD_CHG || msg == MOD_CUT ){
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			OPECTL.Mnt_mod = 0;
			Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
			OPECTL.Mnt_lev = (char)-1;
			OPECTL.PasswordLevel = (char)-1;
			return msg;
		}
		
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
}


//[]----------------------------------------------------------------------[]
/// @brief DC-NET�ʐM�����M�f�[�^�N���A�i�������N���A���鏈���j
//[]----------------------------------------------------------------------[]
/// @param		: none
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/25<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// �C���[�W������
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//// ------------------------------- //
//// QR�o�^       ��
////
////
////
////
////
//// �@�@�@�@�@�@�@�@�@�@�@�@ �I�� 
//// ------------------------------- //
// ------------------------------- //
// QR�o�^       ���@ڰ����       ��
//
//
//
//
//
// �ر�@�@�S�ر�@�@���@�@�@���@�@�@�I�� 
// ------------------------------- //
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
void dc_clr_dsp_count( uchar page )
{
	ushort i, j;
	ushort offset = page * PAGE_ITEM_MAX;

	memset( DC_PreCount, 0xFF, sizeof(ulong)*DC_DATA_MAX );

	dispmlclr( 1, 6 );	// 1�`6�s�ڃN���A

	for( i=1, j=offset; (i<=DATA_LINE_MAX) && (j<DC_DATA_MAX); i++ ){
		grachr( i,  0, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[j++] );
		grachr( i, 11, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[DC_DATA_MAX] );	// "��"

		if( (i <= DATA_LINE_MAX) && (j < DC_DATA_MAX) ){
				if( (i == DATA_LINE_MAX) || (page > 0) ) {
					break;
				}
		}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//		if( DC_DATA_MAX > 6 ){																	//�����̍��ڂ�6�ȏ�ł���ΉE���̍��ڂ�\��
		if((i*2) <= DC_DATA_MAX){
			// �E���\��
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			grachr( i, 15, 8, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[j++] );			//����
			grachr( i, 28, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[DC_DATA_MAX]  );	// "��"
		}
	}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//	Fun_Dsp(FUNMSG[8]);		// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
	Fun_Dsp(FUNMSG[128]);	// "            ��    ��   �I�� "
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET�ʐM�����M�f�[�^�N���A�i�����M�f�[�^�����擾���ĕ\������j
//[]----------------------------------------------------------------------[]
/// @param		: none
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/25<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort dc_dsp_unflushed_count( void )
{
	ushort count = 0;
	char  numStr[10];
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	ushort count_lane = 0;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

	// QR�m��E����f�[�^���擾
	count = Ope_Log_UnreadCountGet( eLOG_DC_QR, eLOG_TARGET_LCD );
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	count_lane = Ope_Log_UnreadCountGet( eLOG_DC_LANE, eLOG_TARGET_LCD );
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

	// �����\��
	if( count != DC_PreCount[DC_DATA_QR] ){
		memset( numStr, 0x00, sizeof(numStr) );
		sprintf( numStr, "%4d", count );
		// �R��
		grachr( 1, 7, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
		DC_PreCount[DC_DATA_QR] = count;
	}

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	// �����\��(���[�����j�^�f�[�^)
	if( count_lane != DC_PreCount[DC_DATA_LANE] ){
		memset( numStr, 0x00, sizeof(numStr) );
		sprintf( numStr, "%4d", count_lane );
		// �R��
		grachr( 1, 24, 4, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)numStr );
		DC_PreCount[DC_DATA_QR] = count_lane;
	}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

	return 0;
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET�ʐM�����M�f�[�^�N���A�i�J�[�\���\���j
//[]----------------------------------------------------------------------[]
/// @param		: none
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/25<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar dc_dsp_ccomchk_cursor( int index, ushort mode, uchar page )
{
	ushort row  = (ushort)((index/2)%DATA_LINE_MAX+1);
	uchar new_page;
	ushort col  = (ushort)(index%2);
	ushort size = 8;
	new_page = index / PAGE_ITEM_MAX;
	if (new_page != page) {
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//		realt_clr_dsp_count(new_page);
		dc_clr_dsp_count(new_page);
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[new_page]);
	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[index]);
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	return new_page;
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET�ʐM�����M�f�[�^�N���A�i���C�������j
//[]----------------------------------------------------------------------[]
/// @param		: none
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/12/17<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort FncChk_DC_UnsendClear( void )
{
	ushort	msg;
	uchar	mode;		// 0:�����\�����
						// 1:�m�F���

	uchar	dsp;		// 0:�`��Ȃ�
						// 1:�����X�V
						// 2:��ʍX�V
						// 3:�����\��

	int		item_pos;	// �J�[�\���ʒu
	ushort	pos;
	uchar	fkey_enb = 0;
	uchar	page = 0;

	uchar DC_POS_TBL[DC_DATA_MAX] =
	{
		6,		// QR�m��
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		6,		// ���[�����j�^
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	};

	// �^�C�g���\��
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_TITLE[0] );	// "���Z���^�[�����M�f�[�^�N���A��"

	dsp      = 2;	// ��ʍX�V(�����܂�)
	mode     = 0;	// �����\�����
	item_pos = 0;	// �ʒu

	while(1){
		// �����\���X�V�v������
		if( dsp ){
			// �����\����ʁH
			if( mode == 0 ){
				// �����\��
				if( dsp == 2 || dsp == 3 ){
					dc_clr_dsp_count(page);							// �\���N���A("����   ��"�����ɂ���)
					dc_dsp_unflushed_count();						// ������\��
					dc_dsp_ccomchk_cursor( item_pos, 1, page );		// ���]�\��
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//					Fun_Dsp( FUNMSG[(121)] );						// " �ر�@�S�ر �@���@       �I�� "
					Fun_Dsp( FUNMSG[(92)] );						// " �ر  �S�ر   ��    ��   �I�� "
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// MH810103(s) �d�q�}�l�[�Ή� ����`�F�b�N�i�����M�N���A�{�^���C���j
//// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:577)�Ή�
////					fkey_enb == 1;
//					fkey_enb = 1;
//// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:577)�Ή�
// MH810103(e) �d�q�}�l�[�Ή� ����`�F�b�N�i�����M�N���A�{�^���C���j
				}
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){

			// �ؒf���m
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				Lagcan(OPETCBNO, 7);
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)

			// �h�A��
			case KEY_MODECHG:
				Lagcan(OPETCBNO, 7);
				return MOD_CHG;

			// �u�I���v
			case KEY_TEN_F5:
				if( mode == 0 ){		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
					BUZPI();
					Lagcan( OPETCBNO, 7 );
					return MOD_EXT;
				}
				else if( mode == 3 ){	// �G���[���
					BUZPI();
					dsp  = 2;
					mode = 0;
				}
				break;

			// �u�N���A�v
			case KEY_TEN_F1:
				// �����\����ʁH
				if( mode == 0 ){
					// �t�@���N�V�����L�[�����H
					if (fkey_enb != 0){
						break;
					}
					BUZPI();
					dispmlclr(1, 6);						// �P�`�U�s�ڂ܂ŃN���A

					pos = DC_POS_TBL[item_pos];
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//					grachr( 2,  2, pos, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[0] );				// "�I������"
					grachr( 2,  2, pos, 0, COLOR_BLACK, LCD_BLINK_OFF, DC_UNSEND_TITLE[item_pos] );			// "�I������"
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:583)�Ή�
//					grachr( 2, (ushort)(pos+2), 31, 0, COLOR_BLACK, LCD_BLINK_OFF, DCNET_UNSEND_STR[0] );	// "�f�[�^���������܂�"
					grachr( 2, (ushort)(pos+2), 30, 0, COLOR_BLACK, LCD_BLINK_OFF, DCNET_UNSEND_STR[0] );	// "�f�[�^���������܂�"
// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:583)�Ή�
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
					Fun_Dsp( FUNMSG[19] );																	// "�@�@�@�@�@�@ �͂� �������@�@�@"
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή�/�R�����g�j
//					mode = 1;	// �m�F��ʒ����[�h��
					mode = 1;	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή�/�R�����g�j
				}
				break;

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			case KEY_TEN_F2:		// �S�N���A
				// �����\����ʁH
				if( mode == 0 ){
					// �t�@���N�V�����L�[�����H
					if (fkey_enb != 0){
						break;
					}
					BUZPI();
					dispmlclr(1, 6);						// �P�`�U�s�ڂ܂ŃN���A

					grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[DC_SEQNO_TYPE_COUNT] );	// "�S"
					grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[0] );					// "�f�[�^���������܂�"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );					// "�@�@�@�@��낵���ł����H�@�@�@"
					Fun_Dsp(FUNMSG[19]);															// "�@�@�@�@�@�@ �͂� �������@�@�@"
					mode = 2;		// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
				}
				break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//			// �u�͂��v
//			case KEY_TEN_F3:
//				// �m�F��ʒ��H
//				if( mode == 1 ){
//					BUZPI();
//					Ope_Log_UnreadToRead( eLOG_DC_QR, eLOG_TARGET_LCD );
//					dsp  = 2;		// ��ʍĕ`��
//					mode = 0;
//				}
//				break;
			case KEY_TEN_F3:
				switch (mode) {	// 0:�����\�����/ 1:�m�F��ʁi�w���ް��j/ 2:�m�F��ʁi�S�ް��j/ 3:�G���[���
					case 0:			// �J�[�\���ړ�
						BUZPI();
						dc_dsp_ccomchk_cursor( item_pos, 0, page );		// ���]�\��
						if (++item_pos >= 2) {
							item_pos = 0;
						}
						dc_dsp_ccomchk_cursor( item_pos, 1, page );		// ���]�\��
						break;
					case 1:			// �N���A
						BUZPI();
						switch (item_pos) {
							case 0:		// QR�m��
								Ope_Log_UnreadToRead( eLOG_DC_QR, eLOG_TARGET_LCD );
								break;
							case 1:		// ڰ����
								Ope_Log_UnreadToRead( eLOG_DC_LANE, eLOG_TARGET_LCD );
								break;
							default:
								break;
						}
						dsp  = 2;		// ��ʍĕ`��
						mode = 0;
						break;
					case 2:			// �S�N���A
						BUZPI();
						Ope_Log_UnreadToRead( eLOG_DC_QR, eLOG_TARGET_LCD );
						Ope_Log_UnreadToRead( eLOG_DC_LANE, eLOG_TARGET_LCD );
						dsp  = 2;		// ��ʍĕ`��
						mode = 0;
						break;
					case 3:
					default:
						break;
				}
				break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

			// �u�������v
			case KEY_TEN_F4:
				// �m�F��ʒ��H
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//				if( mode == 1){
				if ((mode == 1) || (mode == 2)) {
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					BUZPI();
					dsp  = 2;	// ��ʍĕ`��
					mode = 0;
				}
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				else if (mode == 0) {
					// �J�[�\���ړ�
					BUZPI();
					dc_dsp_ccomchk_cursor( item_pos, 0, page );		// ���]�\��
					if (--item_pos < 0) {
						item_pos = DC_SEQNO_TYPE_COUNT - 1;
					}
					dc_dsp_ccomchk_cursor( item_pos, 1, page );		// ���]�\��
				}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
				break;

			default:
				break;
		}
	}
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET�ʐM�Z���^�[�ʐM�e�X�g
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2019/10/08<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort FncChk_DC_ConnectCheck( void )
{
	ushort	msg;
	ushort	req	= 0;		// 0:���ʕ\����ԁA1:���ʗv�����

	dispclr();
	grachr( 0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_TITLE[1] );	// "���Z���^�[�ʐM�e�X�g���@�@�@�@"
	grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[4] );		// "�Z���^�[�ʐM�e�X�g���s���܂��@"
	grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[5] );		// "��낵���ł����H�@�@�@�@�@�@�@"
	Fun_Dsp( FUNMSG[19] );												// "�@�@�@�@�@�@ �͂� �������@�@�@"


	while(1) {
		msg = StoF( GetMessage(), 1 );
		switch (msg) {
// MH810100(S) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			// LCD�ؒf���m
			case LCD_DISCONNECT:
				Lagcan( OPETCBNO, 6 );
				return MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2020/1/6 �Ԕԃ`�P�b�g���X(�����e�i���X)
			// �h�A�J
			case KEY_MODECHG:
				BUZPI();
				Lagcan( OPETCBNO, 6 );
				return MOD_CHG;
				break;

			// �u�������v
			case KEY_TEN_F4:
				if( !req ){	
					BUZPI();
					return MOD_EXT;
				}
				break;

			// �u�͂��v
			case KEY_TEN_F3:
				if( !req ){
					BUZPI();
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_ON, FCSTR1[46] );		// "�@�@�@�@ �ʐM�e�X�g�� �@�@�@�@"
					grachr( 3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, QR_STR[9] );		// "�@�@���΂炭���҂��������@�@�@"
					Fun_Dsp( FUNMSG[00]);												// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
					// ���A���^�C���ʐM�R�}���h�Z�b�g
					if( PKTcmd_mnt_dc_con_req() ){
						Lagtim( OPETCBNO, 6, 10*50 );
						req = 1;														// DC-NET�ʐM���䉞���҂�
					}
					else{
						return( MOD_EXT ); // ����ޑ��M�m�f
					}
				}
				break;

			// DC-NET�ʐM�a�ʌ���
			case LCD_MNT_DCNET_RESULT:
				if( req == 1 ){
					Lagcan( OPETCBNO, 6 );
					BUZPI();
					Fun_Dsp(FUNMSG[8]);													// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� ",
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );		// "�@�@�@�@�ʐM�e�X�g�I���@�@�@�@"

					displclr(3);
// MH810100(S) 2020/10/08 #4914  �y���؉ێw�E�����zLAN�P�[�u�����h�����Ă��Ȃ���Ԃł�����`�F�b�N�̃Z���^�[�ʐM�e�X�g������ɂȂ�(No.95)
//					if( LcdRecv.lcdbm_rsp_rt_con_rslt.result == LCDBM_RESUTL_OK ){	// (00H:����,01H:�ʐM�ُ�(TCP�ڑ��s��),02H:�^�C���A�E�g)
					if( LcdRecv.lcdbm_rsp_dc_con_rslt.result == LCDBM_RESUTL_OK ){	// (00H:����,01H:�ʐM�ُ�(TCP�ڑ��s��),02H:�^�C���A�E�g)
// MH810100(E) 2020/10/08 #4914  �y���؉ێw�E�����zLAN�P�[�u�����h�����Ă��Ȃ���Ԃł�����`�F�b�N�̃Z���^�[�ʐM�e�X�g������ɂȂ�(No.95)
						grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[00]);	// "�Z���^�[�Ƃ̒ʐM�͐���ł��@�@",
					}
// MH810100(S) 2020/10/08 #4914  �y���؉ێw�E�����zLAN�P�[�u�����h�����Ă��Ȃ���Ԃł�����`�F�b�N�̃Z���^�[�ʐM�e�X�g������ɂȂ�(No.95)
//					else if( LcdRecv.lcdbm_rsp_rt_con_rslt.result == LCDBM_RESUTL_TIMEOUT ){
					else if( LcdRecv.lcdbm_rsp_dc_con_rslt.result == LCDBM_RESUTL_TIMEOUT ){
// MH810100(E) 2020/10/08 #4914  �y���؉ێw�E�����zLAN�P�[�u�����h�����Ă��Ȃ���Ԃł�����`�F�b�N�̃Z���^�[�ʐM�e�X�g������ɂȂ�(No.95)
						grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1] );	// "�Z���^�[�Ƃ̒ʐM�Ɏ��s���܂���",
						grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[3]);	// "(02�F�^�C���A�E�g)�@�@�@�@�@�@",
					}
					else{
						grachr( 4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1] );	// "�Z���^�[�Ƃ̒ʐM�Ɏ��s���܂���",
						grachr( 5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[2] );	// "(01�F�ʐM�ُ�(TCP�ڑ��s��))�@",
					}
				}
				req = 2;
				break;	

			case TIMEOUT6:
				if(req == 1){
					BUZPI();
					displclr(3);
					Lagcan( OPETCBNO, 6 );
					grachr( 2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[5] );		// "�@�@�@�@�ʐM�e�X�g�I���@�@�@�@"
					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[1]);		// "�Z���^�[�Ƃ̒ʐM�Ɏ��s���܂���"
					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, REAL_STR[3]);		// "(02�F�^�C���A�E�g)�@�@�@�@�@�@"
					Fun_Dsp(FUNMSG[8]);													// "�@�@�@�@�@�@�@�@�@�@�@�@ �I�� "
				}
				req = 2;
				break;

			case KEY_TEN_F5:
				if( req == 2 ){
					BUZPI();
					Lagcan( OPETCBNO, 6 );
					return MOD_EXT;
				}
				break;
			default:
				break;
		}
	}
}
// MH810100(E) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)

// MH810100(S) Y.Yamauchi 2020/01/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//[]----------------------------------------------------------------------[]
/// @brief DC-NET�ʐM�Z���^�[�ǔԃN���A�Q��ʑI�����
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_OibanClrDsp( void )
{
	dispmlclr(1, 6);

	// �P�s��
	grachr( 1,  0, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[DC_SEQNO_QR] );		// "QR�o�^�@"
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	grachr( 1, 15, 6, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[DC_SEQNO_LANE] );	// "ڰ�����@ "
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET�ʐM�Z���^�[�ǔԃN���A�Q���]����
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void DC_OibanCursorDsp( uchar index, ushort mode )
{
	ushort row;
	ushort col;
	ushort size = 6;

	if( index >= DC_SEQNO_TYPE_COUNT ){
		return;
	}

	row = (ushort)(index/2+1);
	col = (ushort)(index%2);

	grachr( row, ((col)?15:0), size, mode, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[index] );
}

//[]----------------------------------------------------------------------[]
/// @brief DC-NET�ʐM�Z���^�[�ǔԃN���A
//[]----------------------------------------------------------------------[]
/// @param		: None
/// @return 	: MOD_CHG  mode change / MOD_EXT : F5 key
//[]----------------------------------------------------------------------[]
/// @date 		: Create 2020/01/07<br>
/// Update 		:
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static ushort FncChk_DC_OibanClear( void )
{
	ushort	msg;
	uchar	mode = 0;		// 0:��ʑI�����/ 1:�N���A�m�F���
	uchar	dsp = 1;		// 0:�`��Ȃ�/ 1:��ʕ\��
	uchar	item_pos = 0;	// �J�[�\���ʒu
	ushort	pos;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	uchar	i;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

	// �^�C�g���\��
	dispclr();
	grachr( 0,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR[8] );		// "���Z���^�[�ǔԃN���A���@�@�@�@"

	for ( ; ; ) {
		// �����\���X�V�v������
		if( dsp ){
			if( mode == 0 ){
				DC_OibanClrDsp();					// ��ʑI����ʕ\��
				DC_OibanCursorDsp( item_pos, 1 );	// ���]�\��
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//				Fun_Dsp(FUNMSG[121]);				// " �ر                     �I�� "
				Fun_Dsp(FUNMSG[119]);				// " �ر�@�S�ر �@���@       �I�� "
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			}
			dsp = 0;
		}

		msg = StoF( GetMessage(), 1 );

		switch( msg ){		// FunctionKey Enter

			case LCD_DISCONNECT:
				return MOD_CUT;

			case KEY_MODECHG:
				return MOD_CHG;

			// �u�I���v
			case KEY_TEN_F5:
				// ��ʑI����ʁH
				if( mode == 0 ){
					BUZPI();
					return MOD_EXT;
				}
				break;

			case KEY_TEN_F1:		// �N���A
				// ��ʑI����ʁH
				if( mode == 0 ){
					BUZPI();
					dispmlclr( 1, 6 );
					pos = 6;
					grachr( 2,    2,  pos, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[item_pos] );				// "�I������"
					grachr( 2, (ushort)(pos+2), 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );			// "�ǔԂ��N���A���܂�"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
					Fun_Dsp(FUNMSG[19]);																	// "�@�@�@�@�@�@ �͂� �������@�@�@"
					mode = 1;		// 0:��ʑI�����/ 1:�N���A�m�F���
				}
				break;

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
			case KEY_TEN_F2:		// �S�N���A
				if (mode == 0) {
					BUZPI();
					dispmlclr(1, 6);
					grachr( 2, 2,  4, 0, COLOR_BLACK, LCD_BLINK_OFF, DCSTR5[DC_DATA_MAX] );				// "�S"
					grachr( 2, 6, 18, 0, COLOR_BLACK, LCD_BLINK_OFF, CCOMSTR2[2] );							// "�ǔԂ��N���A���܂�"
					grachr( 4,  0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, SMSTR1[21] );							// "�@�@�@�@��낵���ł����H�@�@�@"
					Fun_Dsp(FUNMSG[19]);																	// "�@�@�@�@�@�@ �͂� �������@�@�@"
					mode = 2;		// 0:�����\�����/ 1:�N���A�m�F���/ 2:�S�N���A�m�F���
				}
				break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

			case KEY_TEN_F3:
				switch( mode ){
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					// �J�[�\���ړ��H
					case 0:
						BUZPI();
						DC_OibanCursorDsp(item_pos, 0);			// ���]�\��
						if (++item_pos >= 2) {
							item_pos = 0;
						}
						DC_OibanCursorDsp(item_pos, 1);			// ���]�\��
						break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					// �N���A�m�F��ʁH
					case 1:
						BUZPI();
						switch( item_pos ){
							case 0:									// �Z���^�[�ǔԁiQR�m��E����f�[�^�j
// MH810100(S) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4021 �Z���^�[�ǔԃN���A��A�s���t���O�������Ȃ�)
//								DC_SeqNo[DC_SEQNO_QR] = 1;
								DC_SeqNo[DC_SEQNO_QR] = 0;
// MH810100(E) K.Onodera 2020/03/18 �Ԕԃ`�P�b�g���X(#4021 �Z���^�[�ǔԃN���A��A�s���t���O�������Ȃ�)
								break;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
							case 1:									// �Z���^�[�ǔԁi���[�����j�^�f�[�^�j
								DC_SeqNo[DC_SEQNO_LANE] = 0;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
						}
						dsp  = 1;	// ��ʍĕ`��
						mode = 0;
						break;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					// �S�N���A���s�H
					case 2:
						BUZPI();
						for(i = 0; i < DC_SEQNO_TYPE_COUNT; ++i) {
							DC_SeqNo[i] = 0;
						}
						dsp  = 1;	// ��ʍĕ`��
						mode = 0;
						break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					default:
						break;
			}
			break;

			// �u�������v
			case KEY_TEN_F4:
				// �N���A�m�F��ʁH
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//				if( mode == 1 ){
				if ((mode == 1) || (mode == 2)) {	// 0:�����\�����/ 1:�N���A�m�F���/ 2:�S�N���A�m�F���
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
					BUZPI();
					dsp  = 1;	// ��ʍĕ`��
					mode = 0;
				}
				break;

			default:
				break;
		}
	}
}
// MH810100(E) Y.Yamauchi 2020/01/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
/*[]----------------------------------------------------------------------[]*/
/*| NTNET����                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FunChk_Ntnet( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned short	FunChk_Ntnet( void )
{
	ushort	usFntnetEvent;
	ushort	ret = 0;
	char	wk[2];
	char	org[2];

	/* �ݒ����Ұ�-NTNET�ڑ��Q�� */
	if (! (_is_ntnet()) ) {			// LOCAL NTNET�ł͖���
		BUZPIPI();
		return MOD_EXT;
	}

	org[0] = DP_CP[0];
	org[1] = DP_CP[1];
	DP_CP[0] = DP_CP[1] = 0;
	while (1) {
		dispclr();
		grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR[0]);								/* "���m�s�|�m�d�s���@�@�@�@�@�@�@" */
		usFntnetEvent = Menu_Slt(FNTNETMENU, FNTNET_CHK_TBL, (char)FNTNET_CHK_MAX, (char)1);

		wk[0] = DP_CP[0];
		wk[1] = DP_CP[1];

		switch(usFntnetEvent){		/* FunctionKey Enter */
		case FNTNET1_CHK:	// �P�D�f�[�^�N���A
			usFntnetEvent = check_nt_dtclr();
			break;
// MH341107(S) K.Onodera 2016/11/11 AI-V�Ή�(�[����)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���Ԃ̒ǔԃN���A���j���[���폜�j
//		case CCOM_SEQCLR:	// �Q�D�Z���^�[�ǔԃN���A
//			if( prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0 ){
//				usFntnetEvent = FncChk_cOibanClr();
//			}else{
//				BUZPIPI();
//			}
//			break;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���Ԃ̒ǔԃN���A���j���[���폜�j
// MH341107(E) K.Onodera 2016/11/11 AI-V�Ή�(�[����)
		case MOD_EXT:
			DP_CP[0] = org[0];
			DP_CP[1] = org[1];
			ret = MOD_EXT;
			break;
		default:
			break;
		}
		if (ret == MOD_EXT) {				// KEY_TEN_F5
			break;
		}
		if (usFntnetEvent == MOD_CHG) {		// KEY_MODECHG
			ret = MOD_CHG;
			break;
		}
		DP_CP[0] = wk[0];
		DP_CP[1] = wk[1];
	}
	return(ret);
}

/*[]----------------------------------------------------------------------[]*/
/*| NTNET���� - �ް��ر                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : check_nt_dtclr( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : unsigned short                                          |*/
/*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
ushort	check_nt_dtclr( void )
{
	ushort	msg;
	ushort	w_Count;
	t_NtBufCount	buf;
// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
	uchar	mode = 0;		// ��ʃ��[�h(0�F������ԁA1�FF2�L�[������)
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�

	dispclr();
	grachr(0, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR[0]);	// "���m�s�|�m�d�s�`�F�b�N���@�@�@"
	memset(&buf,0x00,sizeof(buf));
	NTBUF_GetBufCount(&buf, FALSE);									// �f�[�^�������擾����

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[0]);	// "���Z�@�@9999���@���j�^  9999��" ,
	grachr(1, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[5]);	// "���Z�@�@9999���@�W�v�@  9999��" ,
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
	if( buf.sndbuf_sale > 9999 ) {
		w_Count = 9999;
	} else {
		w_Count = (ushort)buf.sndbuf_sale;
	}
	DataCntDsp( w_Count, 0, 1 );
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//	if( buf.sndbuf_monitor > 9999 ) {
//		w_Count = 9999;
//	} else {
//		w_Count = (ushort)buf.sndbuf_monitor;
//	}
//	DataCntDsp( w_Count, 1, 1 );
//
//	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[1]);	// "�W�v�@�@9999���@����@�@9999��" ,
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
	if( buf.sndbuf_ttotal > 9999 ) {
		w_Count = 9999;
	} else {
		w_Count = (ushort)buf.sndbuf_ttotal;
	}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//	DataCntDsp( w_Count, 0, 2 );
//	if( buf.sndbuf_ope_monitor > 9999 ) {
//		w_Count = 9999;
//	} else {
//		w_Count = (ushort)buf.sndbuf_ope_monitor;
//	}
//	DataCntDsp( w_Count, 1, 2 );
	w_Count = 0;													// �t�F�[�Y1�ł͑��M���Ȃ����ߏ��0���Ƃ���
	DataCntDsp( w_Count, 1, 1 );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//	grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[2]);	// "�G���[�@9999���@���K�Ǘ�9999��" ,
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[6]);	// "�G���[�@9999���@���K�Ǘ�9999��" ,
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
	if( buf.sndbuf_error > 9999 ) {
		w_Count = 9999;
	} else {
		w_Count = (ushort) buf.sndbuf_error;
	}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//	DataCntDsp( w_Count, 0, 3 );
	DataCntDsp( w_Count, 0, 2 );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
	if( buf.sndbuf_money > 9999 ) {
		w_Count = 9999;
	} else {
		w_Count = (ushort)buf.sndbuf_money;
	}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//	DataCntDsp( w_Count, 1, 3 );
	w_Count = 0;													// �t�F�[�Y1�ł͑��M���Ȃ����ߏ��0���Ƃ���
	DataCntDsp( w_Count, 1, 2 );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//// MH341107(S) K.Onodera 2016/11/11 AI-V�Ή�(�[����)
//	if( prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0 ){			// �����`���̏ꍇ
//// MH341107(E) K.Onodera 2016/11/11 AI-V�Ή�(�[����)
//	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[3]);	// "�A���[��9999���@���̑��@9999��" ,
//	if( buf.sndbuf_alarm > 9999 ) {
//		w_Count = 9999;
//	} else {
//		w_Count = (ushort)buf.sndbuf_alarm;
//	}
//	DataCntDsp( w_Count, 0, 4 );
//	w_Count = (ushort)(	buf.sndbuf_coin + 			// ��݋��ɏW�v�i���ݖ��g�p�j
//					   	buf.sndbuf_note +			// �������ɏW�v�i���ݖ��g�p�j
//					   	buf.sndbuf_prior +			// �D���ޯ̧
//						buf.sndbuf_normal );		// �ʏ��ޯ̧
//    if( w_Count > 9999 ) {
//		w_Count = 9999;
//    }
//	DataCntDsp( w_Count, 1, 4 );
//// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100���g�p���O�i���Ɂj�j
////	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[4]);	// "���Ɂ@�@9999���@�@�@�@�@�@�@�@" ,
////	if( buf.sndbuf_incar > 9999 ) {
////		w_Count = 9999;
////	} else {
////		w_Count = (ushort) buf.sndbuf_incar;
////	}
////	DataCntDsp( w_Count, 0, 5 );
//// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100���g�p���O�i���Ɂj�j
//// MH341107(S) K.Onodera 2016/11/11 AI-V�Ή�(�[����)
//	} else {
//		grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[5]);	// "�A���[��9999���@�ޑK�Ǘ�9999��" ,
//		if( buf.sndbuf_alarm > 9999 ) {
//			w_Count = 9999;
//		} else {
//			w_Count = (ushort)buf.sndbuf_alarm;
//		}
//		DataCntDsp( w_Count, 0, 4 );
//		w_Count = (ushort)buf.sndbuf_turi;	 			// �ޑK�Ǘ�
//		if( w_Count > 9999 ){
//			w_Count = 9999;
//		}
//		DataCntDsp( w_Count, 1, 4 );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100���g�p���O�i���Ɂj�j
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[6]);	// "���Ɂ@�@9999���@���̑��@9999��" ,
//		if( buf.sndbuf_incar > 9999 ) {
//			w_Count = 9999;
//		} else {
//			w_Count = (ushort) buf.sndbuf_incar;
//		}
//		DataCntDsp( w_Count, 0, 5 );
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//		grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[7]);	// "���̑��@9999���@�@�@�@�@�@�@�@" ,
		grachr(3, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[7]);	// "���̑��@9999���@�@�@�@�@�@�@�@" ,
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100���g�p���O�i���Ɂj�j
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//		w_Count = (ushort)(	buf.sndbuf_coin + 			// ��݋��ɏW�v�i���ݖ��g�p�j
//						   	buf.sndbuf_note +			// �������ɏW�v�i���ݖ��g�p�j
		w_Count = (ushort)(	
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
						   	buf.sndbuf_prior +			// �D���ޯ̧
							buf.sndbuf_normal );		// �ʏ��ޯ̧
		if( w_Count > 9999 ){
			w_Count = 9999;
		}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100���g�p���O�i���Ɂj�j
//		DataCntDsp( w_Count, 1, 5 );
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//		DataCntDsp( w_Count, 0, 5 );
		DataCntDsp( w_Count, 0, 3 );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100���g�p���O�i���Ɂj�j
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
//	}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iNT-NET�`�F�b�N��ʂɑ��M�Ώۂ̂ݕ\������j
// MH341107(E) K.Onodera 2016/11/11 AI-V�Ή�(�[����)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100���g�p���O�i�o�Ɂj�j
//// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
//	if (IS_SEND_OUTCAR_DATA) {
//		grachr(6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR1[7]);	// "�o�Ɂ@�@9999���@�@�@�@�@�@�@�@" ,
//		if( buf.sndbuf_outcar > 9999 ) {
//			w_Count = 9999;
//		} else {
//			w_Count = (ushort)buf.sndbuf_outcar;
//		}
//		DataCntDsp( w_Count, 0, 6 );
//	}
//// MH364300 GG119A34(E) ���P�A���\No.83�Ή�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iGT-4100���g�p���O�i�o�Ɂj�j

// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
//	grachr(6, 1, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR[1]);	// �f�[�^���N���A���܂����H 
//	Fun_Dsp(FUNMSG[19]);				/* "�@�@�@�@�@�@ �͂� �������@�@�@" */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	Fun_Dsp(FUNMSG[121]);				/* "    �@�S�ر �@�@�@       �I�� " */
	Fun_Dsp(FUNMSG[129]);				/* "    �@�S�ر �@�@�@       �I�� " */
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�

	for ( ; ; ) {
		msg = StoF( GetMessage(), 1 );

		switch( msg ){		/* FunctionKey Enter */
		case KEY_MODECHG:
			return MOD_CHG;

// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
		case KEY_TEN_F2:	/* "�S�ر" */
			if (mode == 0) {
				BUZPI();
				displclr(6);
				grachr(6, 1, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, NTNETSTR[1]);	// �f�[�^���N���A���܂����H
				Fun_Dsp(FUNMSG[19]);											/* "�@�@�@�@�@�@ �͂� �������@�@�@" */
				mode = 1;
			}
			break;
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�

		case KEY_TEN_F3:	/* "�͂�" */
// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
			if (mode == 1) {
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�
			BUZPI();
			NTBUF_AllClr();	/* NTNET�f�[�^�S�N���A */

			// NTCOM�^�X�N���ċN��
			NTCom_ClearData(1);
			return MOD_EXT;
// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
			}
			break;
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�

		case KEY_TEN_F4:	/* "������" */
// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
			if (mode == 1) {
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�
			BUZPI();
			return MOD_EXT;
// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
			}
			break;
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�

// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
		case KEY_TEN_F5:	/* "�I��" */
			if (mode == 0) {
				BUZPI();
				return MOD_EXT;
			}
			break;
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�

		default:
			break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		�m�s�|�m�d�s�f�[�^�����\�������Q									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	LogCntDsp2( LogCnt )								|*/
/*|	PARAMETER		:	ushort	dataCnt	:	�f�[�^����						|*/
/*|					:	ushort	type:		0:���^1:�E						|*/
/*|					:	ushort	line:		�\���s�ʒu						|*/
/*|	RETURN VALUE	:	void												|*/
/*[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]*/
void	DataCntDsp( ushort dataCnt, ushort type, ushort	line )
{
	ushort	w_len, w_pos, w_pos2;

	// ���������ő匏��(9999)�ȓ��ɕ␳����
	if( dataCnt > 9999 ) {
		dataCnt = 9999;
	}
	
	// �����̌��������Ƃ߂�
	if( dataCnt >= 1000 ) {			
		w_len = 4;
	} else if (dataCnt >= 100 ) {
		w_len = 3;
	} else if (dataCnt >= 10 ) {
		w_len = 2;
	} else {
		w_len = 1;
	}

	// �\���ʒu�����肷��
	if( type == 0 ) {
		w_pos = 8 - (w_len - 4);			
		w_pos2 = 12;			
	} else {
		w_pos = 24 - (w_len - 4);			
		w_pos2 = 28;			
	}
	
	// ������\������i�O�T�v���X�j
	opedsp3( line, w_pos, dataCnt, w_len, 0, 0 ,COLOR_BLACK, LCD_BLINK_OFF );
	grachr( line, w_pos2, 2, 0, COLOR_BLACK, LCD_BLINK_OFF, (uchar*)"��" );
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
