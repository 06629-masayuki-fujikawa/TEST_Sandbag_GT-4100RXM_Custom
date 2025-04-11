/*[]----------------------------------------------------------------------[]*/
/*| NT-NET�֘A����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
#include	<string.h>
#include	<stddef.h>
#include	"system.h"
#include	"iodefine.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"LKmain.h"
#include	"flp_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"prm_tbl.h"
#include	"irq1.h"
#include	"cnm_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"mnt_def.h"
#include	"ntnetauto.h"
#include	"Suica_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"ifm_ctrl.h"
#include	"remote_dl.h"
#include	"mdl_def.h"
#include	"aes_sub.h"
#include	"raudef.h"
#include	"oiban.h"
#include	"raudef.h"
#include	"cre_ctrl.h"
#include	"remote_dl.h"
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6554 �Z���^�p�[�����f�[�^ �ύX�Ή�
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6554 �Z���^�p�[�����f�[�^ �ύX�Ή�

RECEIVE_BACKUP	RecvBackUp;								/* NE-NET�v���ް���M���ޯ�����	*/

union {
	t_TKI_CYUSI	TeikiCyusi;
} z_Work;

static uchar s_Is234StateValid = 0;
static uchar s_IsRevData80_R = 0;
static uchar basicSeqNo = 0;								// �V�[�P���XNo(1�`99)�������N�����̏����l�́u0�v

// MH322914(S) K.Onodera 2016/08/30 AI-V�Ή��F�U�֐��Z
ushort	PiP_FurikaeSts;		// �U�֐��Z�X�e�[�^�X�t���O
// MH322914(E) K.Onodera 2016/08/30 AI-V�Ή��F�U�֐��Z
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
static ulong	VehicleCountDateTime = 0;
static ushort	VehicleCountSeqNo = 0;

SETTLEMENT_INFO	t_Settlement;
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�

static const ushort c_coin[COIN_SYU_CNT] = {10, 50, 100, 500};
static	char	ntdata65_UnknownVer[] = "--------";

// T���v/T���v ���M�֐�
static	void	NTNET_Snd_SyukeiKihon(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiRyokinMai(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiBunrui(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiWaribiki(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiTeiki(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//static	void	NTNET_Snd_SyukeiShashitsuMai(LOKTOTAL *syukei, uchar ID, uchar Save, ushort Type);
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
static	void	NTNET_Snd_SyukeiKinsen(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);
static	void	NTNET_Snd_SyukeiSyuryo(SYUKEI *syukei, uchar ID, uchar Save, ushort Type);

#define	_NTNET_Snd_Data30(syu)		NTNET_Snd_SyukeiKihon(syu, 30, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data31(syu)		NTNET_Snd_SyukeiRyokinMai(syu, 31, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data32(syu)		NTNET_Snd_SyukeiBunrui(syu, 32, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data33(syu)		NTNET_Snd_SyukeiWaribiki(syu, 33, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data34(syu)		NTNET_Snd_SyukeiTeiki(syu, 34, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data35(syu)		NTNET_Snd_SyukeiShashitsuMai(syu, 35, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data36(syu)		NTNET_Snd_SyukeiKinsen(syu, 36, NTNET_DO_BUFFERING, 1)
#define	_NTNET_Snd_Data41(syu)		NTNET_Snd_SyukeiSyuryo(syu, 41, NTNET_DO_BUFFERING, 1)

#define	_NTNET_Snd_Data158(syu)		NTNET_Snd_SyukeiKihon(syu, 158, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data159(syu)		NTNET_Snd_SyukeiRyokinMai(syu, 159, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data160(syu)		NTNET_Snd_SyukeiBunrui(syu, 160, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data161(syu)		NTNET_Snd_SyukeiWaribiki(syu, 161, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data162(syu)		NTNET_Snd_SyukeiTeiki(syu, 162, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data163(syu)		NTNET_Snd_SyukeiShashitsuMai(syu, 163, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data164(syu)		NTNET_Snd_SyukeiKinsen(syu, 164, NTNET_NOT_BUFFERING, 11)
#define	_NTNET_Snd_Data169(syu)		NTNET_Snd_SyukeiSyuryo(syu, 169, NTNET_NOT_BUFFERING, 11)

// �������v/�������v ��M�֐�
static	void	NTNET_Rev_SyukeiKihon(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiRyokinMai(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiBunrui(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiWaribiki(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiTeiki(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiKinsen(SYUKEI *syukei);
static	void	NTNET_Rev_SyukeiSyuryo(void);

#define	_NTNET_RevData66(syu)		NTNET_Rev_SyukeiKihon(syu)
#define	_NTNET_RevData67(syu)		NTNET_Rev_SyukeiRyokinMai(syu)
#define	_NTNET_RevData68(syu)		NTNET_Rev_SyukeiBunrui(syu)
#define	_NTNET_RevData69(syu)		NTNET_Rev_SyukeiWaribiki(syu)
#define	_NTNET_RevData70(syu)		NTNET_Rev_SyukeiTeiki(syu)
#define	_NTNET_RevData72(syu)		NTNET_Rev_SyukeiKinsen(syu)
#define	_NTNET_RevData77(syu)		NTNET_Rev_SyukeiSyuryo()

#define	_NTNET_RevData194(syu)		NTNET_Rev_SyukeiKihon(syu)
#define	_NTNET_RevData195(syu)		NTNET_Rev_SyukeiRyokinMai(syu)
#define	_NTNET_RevData196(syu)		NTNET_Rev_SyukeiBunrui(syu)
#define	_NTNET_RevData197(syu)		NTNET_Rev_SyukeiWaribiki(syu)
#define	_NTNET_RevData198(syu)		NTNET_Rev_SyukeiTeiki(syu)
#define	_NTNET_RevData200(syu)		NTNET_Rev_SyukeiKinsen(syu)
#define	_NTNET_RevData205(syu)		NTNET_Rev_SyukeiSyuryo()

static void		ntnet_DateTimeCnv(date_time_rec2 *dst, const date_time_rec *src);
static void		ntnet_TeikiCyusiCnvCrmToNt(TEIKI_CHUSI *ntnet, const struct TKI_CYUSI *crm, BOOL all);
static BOOL		ntnet_TeikiCyusiCnvNtToCrm(struct TKI_CYUSI *crm, const TEIKI_CHUSI *ntnet);
static short	ntnet_SearchTeikiCyusiData(ulong ParkingNo, ulong PassID);
static ushort	ntnet_CoinIdx(ushort kind);
static BOOL	chk_hpmonday(t_splday_yobi date);
uchar	cyushi_chk( ushort no );

void	ntnet_Snd_CoinData( uchar kind, ushort type );
void	ntnet_Snd_NoteData( uchar kind, ushort type );
void	ntnet_set_req(ulong req, uchar *dat, int max);
int		car_park_condition(int no, uchar new_cond);
ulong	Convert_PassInfo( ulong ParkingNo, ulong PassID );
void	NTNET_Snd_Data22_FusDel( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind );
static	int		get_param_addr(ushort kind, ushort addr, ushort *seg, ushort *inaddr);
int		check_acceptable(void);
void NTNET_AfterRcv_CallBack();

// �ݒ�ꎞ�ۑ��o�b�t�@�i���V�X�e���N�����ɂO�N���A�j
struct _setdata_ {
	ushort	addr;
	ushort	count;
	long	data[1];
};
NTNET_TMP_BUFFER tmp_buffer;
uchar	SetCarInfoSelect( short index );
#define	SP_RANGE_DEF( a )	((a*1000L)+(a*100L)+(a*10L))
static	uchar	InCarTimeChk( ushort num, struct clk_rec *clk_para );
static void	NTNET_Edit_BasicData( uchar, uchar, uchar, DATA_BASIC *);
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//static	void	NTNET_Snd_Data152_SK(void);
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100
//// MH810102(S) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
//static void NTNET_Edit_Data56_ParkingTkt_Pass( Receipt_data *p_RcptDat, DATA_KIND_56_r17 *p_NtDat );
//// MH810102(E) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
static void NTNET_Edit_Data56_ParkingTkt_Pass( Receipt_data *p_RcptDat, DATA_KIND_56_rXX *p_NtDat );
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100

// MH321800(S) hosoda IC�N���W�b�g�Ή�
#define	CARD_EC_KOUTSUU_USED_TEMP		99				// �����ƌ��σ��[�_�ł̌�ʌn�J�[�h�Ƃ���ʂ��邽�߂̈ꎞ�I�Ȓl
static const	ushort	ec_discount_kind_tbl[EC_BRAND_TOTAL_MAX] = {
// �u�����h�ԍ��Ɗ����W�v�̊�����ʂƂ̑Ή�
// ���u�����h�ԍ��̏��ɒ�`���Ă�������
	NTNET_EDY_0, NTNET_NANACO_0, NTNET_WAON_0, NTNET_SAPICA_0, NTNET_SUICA_1,
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	NTNET_ID_0, NTNET_QUICPAY_0, 0, 0, 0,
// MH810105(S) MH364301 PiTaPa�Ή�
//	NTNET_ID_0, NTNET_QUICPAY_0, NTNET_QR, 0, 0,
	NTNET_ID_0, NTNET_QUICPAY_0, NTNET_QR, NTNET_PITAPA_0, 0,
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
};
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
static void	NTNET_RevDataPiP(void);
static ushort PIP_CheckCarInfo(uchar area,ulong no, ushort* pos);
static void NTNET_RevData16_01_ReceiptAgain(void);
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//static void NTNET_RevData16_03_Furikae( void );
//static ushort PiP_CheckFurikae( void );
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
static void NTNET_RevData16_03_Furikae_Info( void );
static void NTNET_RevData16_03_Furikae_Check( void );
static void NTNET_RevData16_03_Furikae_Go( void );
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//static void NTNET_RevData16_05_ReceiveTkt(void);
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
static void NTNET_RevData16_08_RemoteCalc( void );
static void NTNET_RevData16_08_RemoteCalc_Time( void );
static void NTNET_RevData182_RemoteCalcFee(void);
static void NTNET_RevData16_10_ErrorAlarm( void );
static ushort PIP_GetOccurErr( PIP_ErrAlarm *tbl );
static ushort PIP_GetOccurAlarm( PIP_ErrAlarm *tbl );
static void PiP_GetFurikaeInfo( Receipt_data* receipt );
static uchar PiP_GetNewestPayReceiptData( Receipt_data *rcp, ushort area, ushort no );

extern	short	LKopeGetLockNum( uchar, ushort, ushort * );

extern	const	uchar	ErrAct_index[];
extern	uchar	get_crm_state(void);
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�

/*[]----------------------------------------------------------------------[]*/
/*| �ް���M����                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_GetRevData                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_GetRevData( ushort msg, uchar *data )
{
	t_NtNet_ClrHandle	h;
	eNTNET_BUF_KIND		BufKind = NTNET_BUF_INIT;
	const t_NtBufState	*ntbufst;
	ushort				usdata;

	switch( msg ){
	case IBK_NTNET_DAT_REC:											// NTNET�ް���M
		if( NTBUF_GetRcvNtData( &RecvNtnetDt, NTNET_BUF_PRIOR ) > 0 ){
			// �D���ް���M
			BufKind = NTNET_BUF_PRIOR;
		}
		else if( NTBUF_GetRcvNtData( &RecvNtnetDt, NTNET_BUF_NORMAL ) > 0 ){
			// �ޯ̧�ݸނłȂ��ʏ��ް���M
			BufKind = NTNET_BUF_NORMAL;
		}
		else if( NTBUF_GetRcvNtData( &RecvNtnetDt, NTNET_BUF_BROADCAST ) > 0 ){
			// ����f�[�^
			BufKind = NTNET_BUF_BROADCAST;
		}

		if( BufKind == NTNET_BUF_PRIOR || BufKind == NTNET_BUF_NORMAL ||	// �D��or�ʏ�
			BufKind == NTNET_BUF_BROADCAST ){								// or�����ް���M?
			NTNET_CtrlRecvData();
			//phase = 1;											// ��M�ް��ر
			NTBUF_ClrRcvNtData_Prepare( BufKind, &h );
			//phase = 2;
			NTBUF_ClrRcvNtData_Exec( &h );
			//phase = 3;
		}
		break;
	case IBK_NTNET_FREE_REC:										// NTNET FREE�߹���ް���M
		break;
	case IBK_NTNET_ERR_REC:											// NTNET �װ�ް���M
		err_chk( (char)data[0], (char)data[1], (char)data[2], 0, 0 );	// NTNET IBK�װ
		break;
	case IBK_NTNET_BUFSTATE_CHG:									// NTNET �ޯ̧��ԕω�
		memcpy( &usdata, data, 2 );
		ntbufst = NTBUF_GetBufState();
		switch( usdata ){
		case 20:													// �����ް�
			if( !(ntbufst->car_in & 0x02) ){						// �ޯ̧FULL����?
				err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_BUFFULL, NTERR_RELEASE, 0, 0 );
			}
			queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
			break;
		case 22:													// ���Z�ް�
			if( !(ntbufst->sale & 0x02) ){							// �ޯ̧FULL����?
				err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_BUFFULL, NTERR_RELEASE, 0, 0 );
			}
			if( !(ntbufst->sale & 0x01) ){							// �ޯ̧NEAR FULL����?
				err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_BUFNFULL, NTERR_RELEASE, 0, 0 );
			}
			queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
			break;
		}
		
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �ް���M����                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_CtrlRecvData                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTNET_CtrlRecvData( void )
{
	// �V�X�e��ID�`�F�b�N
	switch (RecvNtnetDt.DataBasic.SystemID) {
	case REMOTE_DL_SYSTEM:
		NTNET_CtrlRecvData_DL();
		return;
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή�
	case PIP_SYSTEM:
		NTNET_RevDataPiP();
		return;
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή�
	default:
		break;
	}

	switch( RecvNtnetDt.DataBasic.DataKind ){
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	case 1:													// �Ԏ��⍇���ް�
//		NTNET_RevData01();
//		break;
//	case 2:													// �Ԏ��⍇�������ް�
//		NTNET_RevData02();
//		break;
//	case 3:													// �Ԏ��⍇������NG�ް�
//		NTNET_RevData03();
//		break;
//	case 4:													// ۯ������ް�
//		NTNET_RevData04();
//		break;
//	case 22:												// ���Z�ް�(CPS)
//	case 23:												// ���Z�ް�(EPS)
//		NTNET_RevData22();
//		break;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	case 100:												// �����ް�
		NTNET_RevData100();
		break;
	case 103:												// �Ǘ��ް��v��
		NTNET_RevData103();
		break;
	case 109:												// �ް��v��2
		NTNET_RevData109();
		break;
	case 154:
		NTNET_RevData154();									// �Z���^�p�f�[�^�v��
		break;
	case 156:
		NTNET_RevData156();									// ���������ݒ�v��
		break;
	case 119:												// ���v�ް�
		NTNET_AfterRcv_CallBack();
		NTNET_RevData119();
		break;
	case 143:												// ������⍇�������ް�
		NTNET_RevData143();
		break;
	case 80:												// ���ʐݒ��ް�
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_RevData80_R();							// ���uIBK����f�[�^
		} else {
			NTNET_RevData80();								// �ݒ�f�[�^���M�v��
		}
		break;
	case 91:
		NTNET_RevData91();									// ��������Z���~�e�[�u���f�[�^
		break;
	case 93:												// ������ð��ð���
		NTNET_RevData93();
		break;
	case 94:												// ������X�Vð���
		NTNET_RevData94();
		break;
	case 95:												// ������o�Ɏ���ð���
		NTNET_RevData95();
		break;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	case 97:												// �Ԏ��p�����[�^��M
//		NTNET_RevData97();
//		break;
//	case 98:
//		NTNET_RevData98();									// ۯ����u���Ұ�
//		break;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	case 110:
		NTNET_RevData110();									// �ް��v��2����NG
		break;
	case 116:	// no break
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_RevData116_R();							// �ʐM�`�F�b�N�v���f�[�^
			break;
		}
	case 117:
		NTNET_RevData116();									// ������f�[�^�X�V�E������f�[�^�`�F�b�N
		break;
	/* �������v�W�v�f�[�^ */
	case 66:
		_NTNET_RevData66(&sky.fsyuk);						// �W�v��{�f�[�^
		break;
	case 67:
		_NTNET_RevData67(&sky.fsyuk);						// ������ʖ��W�v�f�[�^
		break;
		case 68:
		_NTNET_RevData68(&sky.fsyuk);						// ���ޏW�v�f�[�^
		break;
	case 69:
		_NTNET_RevData69(&sky.fsyuk);						// �����W�v�f�[�^
		break;
	case 70:
		_NTNET_RevData70(&sky.fsyuk);						// ����W�v�f�[�^
		break;
	case 71:
	//	_NTNET_RevData71									// �Ԏ����W�v�f�[�^
		break;
	case 72:
		_NTNET_RevData72(&sky.fsyuk);						// ���K�W�v�f�[�^
		break;
	case 73:
	//	_NTNET_RevData73									// �^�C�����W�W�v�f�[�^
		break;
	case 74:
	//	_NTNET_RevData74									// �ʏW�v�f�[�^
		break;
	case 77:
		_NTNET_RevData77(&sky.fsyuk);						// �W�v�I���ʒm�f�[�^
		break;
	/* �������v�W�v�f�[�^ */
	case 194:
		_NTNET_RevData194(&sky.fsyuk);						// �W�v��{�f�[�^
		break;
	case 195:
		_NTNET_RevData195(&sky.fsyuk);						// ������ʖ��W�v�f�[�^
		break;
	case 196:
		_NTNET_RevData196(&sky.fsyuk);						// ���ޏW�v�f�[�^
		break;
	case 197:
		_NTNET_RevData197(&sky.fsyuk);						// �����W�v�f�[�^
		break;
	case 198:
		_NTNET_RevData198(&sky.fsyuk);						// ����W�v�f�[�^
		break;
	case 199:
	//	_NTNET_RevData199									// �Ԏ����W�v�f�[�^
		break;
	case 200:
		_NTNET_RevData200(&sky.fsyuk);						// ���K�W�v�f�[�^
		break;
	case 201:
	//	_NTNET_RevData201									// �^�C�����W�W�v�f�[�^
		break;
	case 202:
	//	_NTNET_RevData202									// �ʏW�v�f�[�^
		break;
	case 205:
		_NTNET_RevData205(&sky.fsyuk);						// �W�v�I���ʒm�f�[�^
		break;
	case 83:
	//	NTNET_RevData83();									// �ݒ�f�[�^���M�v��
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_RevData83_R();							// �e�[�u�������f�[�^
		}
		else {
			NTNET_RevData83();									// �ݒ�f�[�^���M�v��
		}
		break;
	case 78:
		NTNET_RevData78();									// �ݒ�f�[�^�v��
		break;
	case 114:
		NTNET_RevData114();									// �ݒ�f�[�^���M�v��
		break;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	case 234:
//		NTNET_RevData234();									// ���ݑ䐔�Ǘ��f�[�^
//		break;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	case 92:
		NTNET_RevData92();									// ���u������X�e�[�^�X�e�[�u��
		break;
	case 240:
		NTNET_RevData240();									// ���u����f�[�^
		break;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	case 243:
//		NTNET_RevData243();									// ���u�f�[�^�v��
//		break;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	case 90:
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_RevData90_R();									// �j�A�t���ʒm�f�[�^
		}
		break;
	case 101:
		if (RecvNtnetDt.DataBasic.SystemID == REMOTE_SYSTEM) {
			NTNET_Rev_Data101_R();								// �ʐM�`�F�b�N���ʃf�[�^
		}
		break;
	case 142:												// ������⍇���f�[�^
		NTNET_RevData142();								// ������⍇���f�[�^
		break;
	case 150:
		NTNET_RevData150();									// �����W��ID�e�[�u���f�[�^
		break;
	case 254:
		NTNET_RevData254();									// �W��ID��ԃf�[�^
		break;
	case 188:												// ���u�_�E�����[�h�v��
		NTNET_RevData188(&RecvNtnetDt);
		break;
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ��⍇���ް�(�ް����01)��M����                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData01                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData01( void )
{
	ulong	ulwork;
	ushort	i;
	ushort	uswork;

	uswork = 1;														// ���ڑ�
	if( RecvNtnetDt.RData01.LockNo != 0 ){							// ����񂠂�H
		for( i = 0; i < LOCK_MAX; i++ ){
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if( !SetCarInfoSelect((short)i) ){
				continue;
			}
			ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// �����Get
			if( RecvNtnetDt.RData01.LockNo == ulwork ){				// �v�����ꂽ�����ƈ�v?
				uswork = 0;											// ����
				if( m_mode_chk_psl( (ushort)(i+1) ) == ON ){		// �蓮Ӱ������
					uswork = 2;										// �蓮Ӱ�ޒ�
				}
				break;
			}
		}
	}
	// ���ڑ��̏ꍇ
	if (uswork == 1) {
		ulwork = RecvNtnetDt.RData01.LockNo;
		i = 0;
	}

	NTNET_Snd_Data02( ulwork, (ushort)(i+1), uswork );				// NT-NET�Ԏ��⍇�������ް��쐬
}


/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ��⍇�������ް�(�ް����02)��M����                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData02                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData02( void )
{
	uchar	i = 0;
	uchar	j = 0;
	if( RecvNtnetDt.RData02.LockNo == OPECTL.multi_lk ){			// �⍇�������Ԏ����Ɠ��l?
		LOCKMULTI.LockNo = RecvNtnetDt.RData02.LockNo;
		if( RecvNtnetDt.RData02.Answer == 2 ){
			LOCKMULTI.Answer = 4;
		}
		else{
			LOCKMULTI.Answer = RecvNtnetDt.RData02.Answer;
		}
		/****************************/
		/*	LOCKMULTI.Answer		*/
		/*		0:����				*/
		/*		1:���ڑ��i�Y���Ȃ��j*/
		/*		4:�蓮Ӱ�ޒ�		*/
		/****************************/
		memcpy( &LOCKMULTI.lock_mlt, RecvNtnetDt.RData02.LkInfo, sizeof( RecvNtnetDt.RData02.LkInfo ) );
/*�ǉ��ް����i�[*/
		LOCKMULTI.ppc_chusi_ryo_mlt = RecvNtnetDt.RData02.PpcData;
		for(i=0 ; i<15 ; i++)
			LOCKMULTI.sev_tik_mlt[i] = (uchar)RecvNtnetDt.RData02.SvsTiket[i];
		for(i=0,j=0 ; i<5 ; i++,j+=2){
			LOCKMULTI.kake_data_mlt[i].mise_no = (ushort)RecvNtnetDt.RData02.KakeTiket[j];
			LOCKMULTI.kake_data_mlt[i].maisuu = (uchar)RecvNtnetDt.RData02.KakeTiket[j+1];
		}
		queset( OPETCBNO, IBK_NTNET_LOCKMULTI, 0, NULL );			// NTNET ������Z�p�ް�(�Ԏ��⍇�������ް�)��M�ʒm
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ��⍇������NG�ް�(�ް����03)��M����                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData03                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData03( void )
{
	if( RecvNtnetDt.RData03.LockNo == OPECTL.multi_lk ){			// �⍇�������Ԏ����Ɠ��l?
		LOCKMULTI.LockNo = RecvNtnetDt.RData03.LockNo;

		if(  RecvNtnetDt.RData03.Answer == 0 ){
			LOCKMULTI.Answer = 2;									// ���ڑ��i�Y���Ȃ��j
		}
		else{
			LOCKMULTI.Answer = 3;									// �ʐM��Q��
		}
		queset( OPETCBNO, IBK_NTNET_LOCKMULTI, 0, NULL );			// NTNET ������Z�p�ް�(�Ԏ��⍇�������ް�)��M�ʒm
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ۯ������ް�(�ް����04)��M����                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData04                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData04( void )
{
	ulong	ulwork;
	ushort	i;
	uchar		CarCnd;
	flp_com*	fl_wrk;
	void	(*_ntnet_snd_data05)(ulong, uchar, uchar);		/* �֐��|�C���^ */
	ushort	recv_MAF = 0;
	ushort	move;
	
	_ntnet_snd_data05 = NTNET_Snd_Data05;		// �ʏ��NT-NET�ւ̑��M�|�C���^���Z�b�g
	move = RecvNtnetDt.RData04.MoveMode;
	i = 0;

	if( RecvBackUp.ReceiveFlg == 0 ){								// �v����M�ς݂ł͂Ȃ�?

		RecvBackUp.ReceiveFlg = 1;									// �v����M�ς�
		memcpy( &RecvBackUp.ParkingNo, &RecvNtnetDt.DataBasic.ParkingNo, 22 );	// ��M�ް��ޯ�����

		switch( move ){	// �����敪�H

			case	5:	// �Sۯ����u��
				if( recv_MAF )	i = 1;
				queset( FLPTCBNO, LOCK_A_CLS_SND_NTNET, sizeof(i), &i );	// ۯ����u�Ǘ������ifcmain�j��ү���ޑ��M
				wopelg( OPLOG_SET_A_LOCK_CLOSE, 0, 0 );				// ���엚��o�^�F�Sۯ����u��(NT-NET)
				_ntnet_snd_data05( 0, 0, 34 );						// �ԐM�i�����0�A�ԗ����m��ԁ��ڑ������A���u��ԁ��Sۯ����u�j
				break;

			case	6:	// �Sۯ����u�J
				if( recv_MAF )	i = 1;
				queset( FLPTCBNO, LOCK_A_OPN_SND_NTNET, sizeof(i), &i );	// ۯ����u�Ǘ������ifcmain�j��ү���ޑ��M
				wopelg( OPLOG_SET_A_LOCK_OPEN, 0, 0 );				// ���엚��o�^�F�Sۯ����u�J(NT-NET)
				_ntnet_snd_data05( 0, 0, 35 );						// �ԐM�i�����0�A�ԗ����m��ԁ��ڑ������A���u��ԁ��Sۯ����u�J�j
				memset( &Kakari_Numu[0], 0, LOCK_MAX );				// �����o�ɗp�W�����ر�ر
				break;

			case	32:	// �S�ׯ�ߏ㏸
				if( recv_MAF )	i = 1;
				queset( FLPTCBNO, FLAP_A_UP_SND_NTNET, sizeof(i), &i );	// ۯ����u�Ǘ������ifcmain�j��ү���ޑ��M
				queset( FLPTCBNO, INT_FLAP_A_UP_SND_NTNET, sizeof(i), &i );	// ۯ����u�Ǘ������ifcmain�j��ү���ޑ��M
				wopelg( OPLOG_SET_A_FLAP_UP, 0, 0 );				// ���엚��o�^�F�S�ׯ�ߏ㏸(NT-NET)
				_ntnet_snd_data05( 0, 0, 32 );						// �ԐM�i�����0�A�ԗ����m��ԁ��ڑ������A���u��ԁ��S�ׯ�ߏ㏸�j
				break;

			case	33:	// �S�ׯ�߉��~
				if( recv_MAF )	i = 1;
				queset( FLPTCBNO, FLAP_A_DOWN_SND_NTNET, sizeof(i), &i );	// ۯ����u�Ǘ������ifcmain�j��ү���ޑ��M
				queset( FLPTCBNO, INT_FLAP_A_DOWN_SND_NTNET, sizeof(i), &i );	// ۯ����u�Ǘ������ifcmain�j��ү���ޑ��M
				wopelg( OPLOG_SET_A_FLAP_DOWN, 0, 0 );				// ���엚��o�^�F�S�ׯ�߉��~(NT-NET)
				_ntnet_snd_data05( 0, 0, 33 );						// �ԐM�i�����0�A�ԗ����m��ԁ��ڑ������A���u��ԁ��S�ׯ�߉��~�j
				memset( &Kakari_Numu[0], 0, LOCK_MAX );				// �����o�ɗp�W�����ر�ر
				break;

			default:	// ���̑�
				if( RecvNtnetDt.RData04.LockNo != 0 ){				// ����񂠂�H
				for( i = 0; i < LOCK_MAX; i++ ){
					WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
					if( !SetCarInfoSelect((short)i) ){
						continue;
					}
					ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// �����Get
					if( RecvNtnetDt.RData04.LockNo == ulwork ){								// �v�����ꂽ�����ƈ�v?
						i++;
						if( RecvNtnetDt.RData04.MoveMode == 1 ){							// ۯ����u�w��
							if( recv_MAF ){
								recv_MAF = FLAP_UP_SND_MAF;
							}else{
								recv_MAF = FLAP_UP_SND_NTNET;
							}
							queset( FLPTCBNO, recv_MAF, sizeof(i), &i );
							if( i < TOTAL_CAR_LOCK_MAX ){
								wopelg(OPLOG_SET_FLAP_UP, 0, ulwork);
							}
							else{
								wopelg(OPLOG_SET_LOCK_CLOSE, 0, ulwork);
							}
							return;
						}
						else if( RecvNtnetDt.RData04.MoveMode == 2 ){						// ۯ����u�J�w��
							if( recv_MAF ){
								recv_MAF = FLAP_DOWN_SND_MAF;
							}else{
								recv_MAF = FLAP_DOWN_SND_NTNET;
							}
							queset( FLPTCBNO, recv_MAF, sizeof(i), &i );
							if( i < TOTAL_CAR_LOCK_MAX ){
								wopelg(OPLOG_SET_FLAP_DOWN, 0, ulwork);
							}
							else{
								wopelg(OPLOG_SET_LOCK_OPEN, 0, ulwork);
							}
							Kakari_Numu[i-1] = 0;								// �����o�ɗp�W�����ر�ر
							return;
						}
						else if (RecvNtnetDt.RData04.MoveMode == 31) {		// �Ïؔԍ������w��
							fl_wrk = &FLAPDT.flp_data[i-1];
							if(fl_wrk->passwd != 0){						// �Ïؔԍ����o�^����Ă���ꍇ
								fl_wrk->passwd = 0;							// �Ïؔԍ�(Atype)����
							}
							wopelg(OPLOG_ANSHOU_B_CLR, 0, ulwork);
			
							// �ԗ����m��ԁEۯ����u��Ԏ擾�iNTNET_Snd_Data05_Sub���p�j
							CarCnd = 0;
							if( fl_wrk->nstat.bits.b08 == 0 ){				// �ڑ��L��?
								if( fl_wrk->nstat.bits.b00 ){				// �ԗ��L��?
									CarCnd = 1;								// �ԗ��L��
								}else{
									CarCnd = 2;								// �ԗ�����
								}
							}else{
								CarCnd = 0;									// �ڑ�����
							}
							// �ԐM(���b�N���u���31�F�Ïؔԍ�����)
							_ntnet_snd_data05( RecvNtnetDt.RData04.LockNo, CarCnd, 31 );
							return;
						}
						break;
					}
				}
			}
			// �v�����ꂽ�����(�Ԏ�)���ݒ肳��Ă��Ȃ��A�������͋����E�����敪�ُ�̏ꍇ
			_ntnet_snd_data05( RecvNtnetDt.RData04.LockNo, 0, 6 );		// �ڑ������ŋ����ԐM
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�ް�(�ް����22)��M����                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData22                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData22( void )
{
	ulong	ulwork;
	ushort	i;

	if(	(RecvNtnetDt.RData22.PayMethod == 5)||						// ������X�V
		(RecvNtnetDt.RData22.LockNo == 0 )){						// �����
		return;
	}
	if( (RecvNtnetDt.RData22.DataBasic.MachineNo == RcvDt22.MachineNo )&&	// �[���@�B��
		(RecvNtnetDt.RData22.PayCount == RcvDt22.PayCount)&&				// ���Z�ǔ�
		(RecvNtnetDt.RData22.LockNo	== RcvDt22.LockNo)&&					// �����
		(RecvNtnetDt.RData22.PayClass == RcvDt22.PayClass)&&				// �����敪
		(memcmp( &RecvNtnetDt.RData22.OutTime, &RcvDt22.OutTime, 
					sizeof(date_time_rec2) ) == 0 ) )						// �o�ɔN����(���Z�N����)
	{
		return;
	}
	RcvDt22.MachineNo = RecvNtnetDt.RData22.DataBasic.MachineNo;
	RcvDt22.PayCount = RecvNtnetDt.RData22.PayCount;
	RcvDt22.LockNo = RecvNtnetDt.RData22.LockNo;
	RcvDt22.PayClass = RecvNtnetDt.RData22.PayClass;
	memcpy( &RcvDt22.OutTime,&RecvNtnetDt.RData22.OutTime,sizeof(date_time_rec2) );

	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;														// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( !SetCarInfoSelect((short)i) ){
			continue;
		}
		ulwork = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// �����Get
		if( RecvNtnetDt.RData22.LockNo == ulwork ){					// �v�����ꂽ�����ƈ�v?
			i++;
			if( RecvNtnetDt.RData22.PayClass == 2 ||
				RecvNtnetDt.RData22.PayClass == 3 ){
				// ���Z���~�A�Đ��Z���~
				mulchu_syu( i );									// ������Z���~�W�v(���Z�ް���M�W�v)
			}else if( RecvNtnetDt.RData22.PayClass == 0 ||			// ���Z����or�Đ��Z����
					  RecvNtnetDt.RData22.PayClass == 1 ||
					  RecvNtnetDt.RData22.PayClass == 4 ||			// �N���W�b�g���Z�E�Đ��Z
					  RecvNtnetDt.RData22.PayClass == 5 ||			// �N���W�b�g�Đ��Z
					  RecvNtnetDt.RData22.PayClass == 14 ||			// �@�l�J�[�h���Z
					  RecvNtnetDt.RData22.PayClass == 15 ){			// �@�l�J�[�h�Đ��Z
				// ���Z����
				queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof(i), &i );	// ���Z����������(�ׯ�߰���~)
				mulkan_syu( i );									// ������Z�����W�v(���Z�ް���M�W�v)
				SvsTime_Syu[i-1] = (uchar)RecvNtnetDt.RData22.Syubet;	// �Ԏ����̐��Z���̗������set
				memcpy( &FLAPDT.flp_data[i-1].s_year, &RecvNtnetDt.RData22.OutTime.Year, 6 ); // �o��(���Z)�������Z�b�g����
				IoLog_write(IOLOG_EVNT_AJAST_FIN, (ushort)LockInfo[i - 1].posi, 0, 0);
			}
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Ԑ�����ύX			                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : car_park_condition                                      |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int		car_park_condition(int no, uchar new_cond)
{
	uchar	old_cond;

//	�ENTNET�ő����Ă��閞�󐧌�P�`�R��car_full[1�`3]�ɕۑ�����
//	�Ecar_full[n]���ω������瑀�샂�j�^�ɓo�^����
//	 ���{�̐ݒ�PPrmSS�ɂ͉e�����Ȃ�

	if (new_cond) {
	// �ύX����
		old_cond = FLAGPT.car_full[no];
		switch( new_cond ){
		case	1:								// ��������
			FLAGPT.car_full[no] = 1;
			break;
		case	2:								// �������
			FLAGPT.car_full[no] = 2;
			break;
		case	3:								// �قږ���
		case	4:								// ����
			FLAGPT.car_full[no] = 0;
			break;
		default:								// ����`�Ȃ珑�������Ȃ�
			break;
		}
		if (FLAGPT.car_full[no] != old_cond)
		// car_full[no]���ω�����
			return 1;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ް�(�ް����100)��M����                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData100                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData100( void )
{
	ulong work;
	ulong req;
	RECEIVE_REC		tmp;
	req = 0;

	// �c�x�Ɛ؊�
	memcpy(&tmp,&FLAGPT.receive_rec,sizeof(tmp));
	work = (ulong)(tmp.flg_ock);
	if( RecvNtnetDt.RData100.ControlData[0] == 1 ){						// �x��?
		tmp.flg_ock = 2;												// �����x�Ɛ؊�
		tmp.flg_ocd = CLK_REC.ndat;
		tmp.flg_oct = CLK_REC.nmin;
	}
	else if( RecvNtnetDt.RData100.ControlData[0] == 2 ){				// �c��?
		tmp.flg_ock = 1;												// �����c�Ɛ؊�
		tmp.flg_ocd = CLK_REC.ndat;
		tmp.flg_oct = CLK_REC.nmin;
	}
	else if( RecvNtnetDt.RData100.ControlData[0] == 3 ){				// ����?
		tmp.flg_ock = 0;												// �c�x�Ǝ���
		tmp.flg_ocd = 0;
		tmp.flg_oct = 0;
	}
	if( RecvNtnetDt.RData100.ControlData[0] ){							// �c�x�Ɛؑ֎w������?
		nmisave(&FLAGPT.receive_rec, &tmp, sizeof(RECEIVE_REC) );
		queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
	}
	if (work != (ulong)(tmp.flg_ock)) {
		wopelg(OPLOG_SET_EIKYUGYO, 0, 0);
	}
	
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	work = 0;
//	// ���Ԑ���P
//	work |= (ulong)car_park_condition(1, RecvNtnetDt.RData100.ControlData[1]);
//	// ���Ԑ���Q
//	work |= (ulong)car_park_condition(2, RecvNtnetDt.RData100.ControlData[2]);
//	// ���Ԑ���R
//	work |= (ulong)car_park_condition(3, RecvNtnetDt.RData100.ControlData[10]);
//	if (work) {
//	// �ǂꂩ���ω�������
//		wopelg(OPLOG_SET_MANSYACTRL, 0, 0);
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

	// ����߽�؊�
	if( RecvNtnetDt.RData100.ControlData[3] == 1 ){						// ����?
		PPrmSS[S_P01][3] = 0L;											// ����߽��������
		wopelg( OPLOG_SET_NYUSYUTUCHK_ON, 0, 0 );		// ���엚��o�^
	}
	else if( RecvNtnetDt.RData100.ControlData[3] == 2 ){				// ����?
		PPrmSS[S_P01][3] = 1L;											// ����߽������������
		wopelg( OPLOG_SET_NYUSYUTUCHK_OFF, 0, 0 );		// ���엚��o�^
	}

	if( RecvNtnetDt.RData100.ControlData[7] == 1 ){						// ���Z�b�g�w��(�Ǘ��f�[�^�v��)�L��H
		req = 0L;
		if (prm_get(0, S_NTN, 43, 1, 1) == 1) {
			// ���v�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_CLOCK;
		}
		if (prm_get(0, S_NTN, 43, 1, 2) == 1) {
			// ����f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_CTRL;
		}
		if (prm_get(0, S_NTN, 43, 1, 4) == 1) {
			// ������X�e�[�^�X�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_PASSSTS;
		}
		if (prm_get(0, S_NTN, 43, 1, 5) == 1) {
			// ��������~�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_PASSSTOP;
		}
		if (prm_get(0, S_NTN, 43, 1, 6) == 1) {
			// ������o�Ɏ����f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_PASSEXIT;
		}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		if (prm_get(0, S_NTN, 44, 1, 1) == 1) {
//			// �Ԏ��p�����[�^�f�[�^���M�`�F�b�N
//			req |= NTNET_MANDATA_LOCKINFO;
//		}
//		if (prm_get(0, S_NTN, 44, 1, 2) == 1) {
//			// ���b�N���u�f�[�^���M�`�F�b�N
//			req |= NTNET_MANDATA_LOCKMARKER;
//		}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		if (prm_get(0, S_NTN, 44, 1, 4) == 1) {
			// ������X�V�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_PASSCHANGE;
		}
		if (prm_get(0, S_NTN, 44, 1, 6) == 1) {
			// ���ʓ��ݒ�f�[�^���M�`�F�b�N
			req |= NTNET_MANDATA_SPECIALDAY;
		}
	}

	NTNET_Snd_Data101( RecvNtnetDt.RData100.DataBasic.MachineNo );		// NT-NET���䉞���ް��쐬
	if( req ){															// �Ǘ��ް��v������?
		NTNET_Snd_Data104(req);											// NT-NET�Ǘ��ް��v���쐬
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �Ǘ��ް��v��(�ް����103)��M����                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData103                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData103( void )
{
	ushort	i;

	char req = 0;
	ushort code;
	
	if (_is_ntnet_remote()) {
		for (i = 0; i < 70; i++) {
			if (RecvNtnetDt.RData103.ControlData[i]) {
				req++;
				code = i;
			}
		}
		switch(code) {
		case	0:	// ���v�ް��v��
		case	18:	// ���ʓ��ݒ�f�[�^�v��
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		case	25:	// ���ԑ䐔�Ǘ��f�[�^�v��
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			break;
		default:
			req = 0;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}
		if (req != 1) {
			// �Ǘ��f�[�^�v��NG���M
			code = 0;	// ���ڑ��Ƃ��ĕԑ�
			NTNET_Snd_Data105(RecvNtnetDt.RData103.DataBasic.MachineNo, code);
			return;
		}
	}

	if( RecvNtnetDt.RData103.ControlData[0] ){						// ���v�ް��v������
		NTNET_Snd_Data229_Exec();										// NT-NET���v�ް��쐬
	}
	if (_is_ntnet_remote() == 0) {
		if( RecvNtnetDt.RData103.ControlData[2] ){						// ���ʐݒ��ް��v���v������
			for (i = S_SYS; i < C_PRM_SESCNT_END; i++) {
				NTNET_Snd_Data208(RecvNtnetDt.RData103.DataBasic.MachineNo, 0, i);	// NT-NET���ʐݒ��ް��쐬
			}
			NTNET_Snd_Data208(RecvNtnetDt.RData103.DataBasic.MachineNo, 1, i);		// NT-NET���ʐݒ��ް��쐬
		}
		if( RecvNtnetDt.RData103.ControlData[13] ){						// ��������Z���~ð����ް��v������
			NTNET_Snd_Data219(2, NULL);									// NT-NET��������Z���~ð����ް��쐬
		}
		if( RecvNtnetDt.RData103.ControlData[15] ){						// ������ð��ð����ް��v������
			NTNET_Snd_Data221();										// NT-NET������ð��ð����ް��쐬
		}
		if( RecvNtnetDt.RData103.ControlData[16] ){						// ������X�Vð����ް��v������
			NTNET_Snd_Data222();										// NT-NET������X�Vð����ް��쐬
		}
		if( RecvNtnetDt.RData103.ControlData[17] ){						// ����o�Ɏ���ð����ް��v������
			NTNET_Snd_Data223();										// NT-NET������o�Ɏ���ð����ް��쐬
		}
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		if( RecvNtnetDt.RData103.ControlData[19] ){						// �Ԏ����Ұ��v������
//			NTNET_Snd_Data225(RecvNtnetDt.RData103.DataBasic.MachineNo);	// NT-NET�Ԏ����Ұ��쐬
//		}
//		if( RecvNtnetDt.RData103.ControlData[20] ){						// ۯ����u���Ұ��v������
//			NTNET_Snd_Data226(RecvNtnetDt.RData103.DataBasic.MachineNo);	// NT-NETۯ����u���Ұ��쐬
//		}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	}

	if (RecvNtnetDt.RData103.ControlData[18]) {		// ���ʓ��ݒ��ް�
		NTNET_Snd_Data211_Exec();
	}
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	if (RecvNtnetDt.RData103.ControlData[25]) {		// ���ԑ䐔�Ǘ��ް�
//		NTNET_Snd_Data235_Exec(1);
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
}


/*[]----------------------------------------------------------------------[]*/
/*| �ް��v��2(�ް����109)��M����                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData109                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData109( void )
{
	char req = 0;
	ushort code, i;

	if (_is_ntnet_remote()) {
		for (i = 0; i < 20; i++) {
			if (RecvNtnetDt.RData109.ControlData[i]) {
				req++;
				code = i;
			}
		}
		switch(code) {
		case	5:	// �R�C�����ɏ��v�f�[�^
		case	6:	// �������ɏ��v�f�[�^
		case	7:	// ����J�E���g�f�[�^
		case	8:	// �[������ް��v��
		case	9:	// �[������ް��v��
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		case	11:	// �ȈՎԎ����ð��ٗv��
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			break;
		default:
			req = 0;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		}
		if (req != 1) {
			// �Ǘ��f�[�^�v��NG���M
			code = 0;	// ���ڑ��Ƃ��ĕԑ�
			NTNET_Snd_Data110(RecvNtnetDt.RData109.DataBasic.MachineNo, code);
			return;
		}
	}

	if( RecvNtnetDt.RData109.ControlData[0] ){							// T���v�f�[�^�v������
		NTNET_Snd_TSYOUKEI();	// T���v�f�[�^�쐬
	}
	if( RecvNtnetDt.RData109.ControlData[8] ){							// �[������ް��v������
		NTNET_Snd_Data230( RecvNtnetDt.RData109.DataBasic.MachineNo );	// NT-NET�[������ް��쐬
	}
	if( RecvNtnetDt.RData109.ControlData[9] ){							// �[������ް��v������
		NTNET_Snd_Data231( RecvNtnetDt.RData109.DataBasic.MachineNo );	// NT-NET�[������ް��쐬
	}
	if( RecvNtnetDt.RData109.ControlData[10] ){							// ���K�Ǘ��f�[�^�v������
		NTNET_Snd_Data126( RecvNtnetDt.RData109.DataBasic.MachineNo, 40 );	// NT-NET���K�Ǘ��f�[�^�쐬
	}
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	if( RecvNtnetDt.RData109.ControlData[11] ){							// �ȈՎԎ����ð��ٗv������
//		NTNET_Snd_Data12( RecvNtnetDt.RData109.DataBasic.MachineNo );	// NT-NET�ȈՎԎ����ð��ٍ쐬
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	if (RecvNtnetDt.RData109.ControlData[5]) {		// �R�C�����ɏ��v�f�[�^���M
		NTNET_Snd_Data130();
	}
	if (RecvNtnetDt.RData109.ControlData[6]) {		// �������ɏ��v�f�[�^���M
		NTNET_Snd_Data132();
	}
	if (RecvNtnetDt.RData109.ControlData[7]) {		// ����J�E���g�f�[�^���M
		NTNET_Snd_Data228(RecvNtnetDt.RData109.DataBasic.MachineNo);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�p�f�[�^�v��(�ް����154)��M����                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData154                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData154( void )
{
	char req = 0;
	ushort code;

	if (_is_ntnet_remote()) {
		if( RecvNtnetDt.RData154.SeisanJyoutai_Req ){	// ���Z��ԃf�[�^�v��
			req++;
		}
		if( RecvNtnetDt.RData154.RealTime_Req ){		// ���A���^�C�����v��
			req++;
		}
		if( RecvNtnetDt.RData154.TermInfo_Req ) {		// �[�����f�[�^�v��
			req++;
		}
		if( RecvNtnetDt.RData154.SynchroTime_Req ) {	// ���������f�[�^�v��
			req++;
		}
		if (req != 1) {
			// �Z���^�p�f�[�^�v��NG���M
			code = 9;								// 9:�p�����[�^NG
			NTNET_Snd_Data155(RecvNtnetDt.RData154.DataBasic.MachineNo, code);
			return;
		}
		if( RecvNtnetDt.RData154.SeisanJyoutai_Req ){	// ���Z��ԃf�[�^�v��
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
//			NTNET_Snd_Data152();						// ���Z��ԃf�[�^�v��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//			NTNET_Snd_Data152_r12();					// ���Z��ԃf�[�^�v��
			NTNET_Snd_Data152_rXX();					// ���Z��ԃf�[�^�v��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		}
		if( RecvNtnetDt.RData154.RealTime_Req ){		// ���A���^�C�����v��
			NTNET_Snd_Data153();						// ���A���^�C�����v��
		}
		if( RecvNtnetDt.RData154.TermInfo_Req ) {		// �[�����f�[�^�v��
			// �Z���^�[�p�f�[�^�v�����ʂ𑗐M
			code = 100;									// 100:�����t�i�[�����v���ɑ΂��Ă̂݁j
			NTNET_Snd_Data155(RecvNtnetDt.RData109.DataBasic.MachineNo, code);
			NTNET_Snd_Data65(RecvNtnetDt.RData109.DataBasic.MachineNo);
		}
		if( RecvNtnetDt.RData154.SynchroTime_Req ) {	// ���������f�[�^�v��
			code = 0;								// 0:����
			NTNET_Snd_Data157(RecvNtnetDt.RData154.DataBasic.MachineNo, code ,NTNET_SYNCHRO_GET );
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ���������ݒ�v��(�ް����156)��M����                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData156                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData156( void )
{
	char err;
	ushort code;
	ulong 	date1;

	err = 0;
	if (_is_ntnet_remote()) {
		if( 0 == RecvNtnetDt.RData156.Time_synchro_Req ){	// ���������ݒ�v���Ȃ��̏ꍇ
			err = 1;
		}
		if( 23 < RecvNtnetDt.RData156.synchro_hour ){		// �������� 23���傫���ꍇ��NG
			err = 1;
		}
		if( 59 < RecvNtnetDt.RData156.synchro_minu ){		// �������� 59���傫���ꍇ��NG
			err = 1;
		}
		if ( 1 == err ) {
			// �Z���^�p�f�[�^�v��NG���M
			code = 9;	// 9:�p�����[�^NG
		} else {
			// �������Ԃ����ʃp�����[�^�փZ�b�g
			date1 = CPrmSS[S_NTN][122];
			date1 /= 10000;												// ��2���͕ύX���Ȃ�
			date1 *= 100;
			date1 += (ulong)RecvNtnetDt.RData156.synchro_hour;
			date1 *= 100;
			date1 += (ulong)RecvNtnetDt.RData156.synchro_minu;
			CPrmSS[S_NTN][122] = date1;									// �����������Z�b�g

			DataSumUpdate(OPE_DTNUM_COMPARA);							// ���ʐݒ��ް��т��X�V
			FLT_WriteParam1(FLT_NOT_EXCLUSIVE);							// FlashROM update
			SetSetDiff(SETDIFFLOG_SYU_REMSET);							// �f�t�H���g�Z�b�g���O�o�^����
			wopelg(OPLOG_SET_SNTP_SYNCHROTIME, 0, 0);					// ���엚��o�^
			code = 0;	// 0:����
		}
		NTNET_Snd_Data157(RecvNtnetDt.RData156.DataBasic.MachineNo, code, NTNET_SYNCHRO_SET);
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ���v�ް�(�ް����119)��M����                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData119                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData119( void )
{
	struct	clk_rec wk_CLK_REC;				// ��M�d�����e(�o�C�i��)
	ushort	wk_CLK_REC_msec;				//							�~���b
	ushort	Setting_Supplement_msec;		// �ݒ�̒x�����Ԓl
	ushort	NormDay;						// �N�����m�[�}���C�Y�l
	ulong	NormTime;						// �����b�~���b�m�[�}���C�Y�l
	ushort	WaitTime_ForTimeSet;			// ���v�Z�b�g�܂ł�WAIT����
	ulong	wkul;
	ushort	wkus;

	if( prm_get(COM_PRM,S_NTN,29,1,1) != 0 ){
	// 34-0029���O�ȊO�̎��͎�M�������Ȃ��B
		return;
	}
	// NT-NET ���v�f�[�^�V�d���ɑΉ��i�␳�l���j
	// ���v�X�V�v���͏�Ɏ󂯕t���邱�Ƃ���

	wk_CLK_REC.year = RecvNtnetDt.RData119.Year;
	wk_CLK_REC.mont = RecvNtnetDt.RData119.Mon;
	wk_CLK_REC.date = RecvNtnetDt.RData119.Day;
	wk_CLK_REC.hour = RecvNtnetDt.RData119.Hour;
	wk_CLK_REC.minu = RecvNtnetDt.RData119.Min;
	wk_CLK_REC.seco = (uchar)RecvNtnetDt.RData119.Sec;
	wk_CLK_REC_msec = RecvNtnetDt.RData119.MSec;

	// ��M�d���̗L�����`�F�b�N
	if( !c_ClkCheck(&wk_CLK_REC, &wk_CLK_REC_msec) ){		// ���v�lNG
		return;
	}

	// �␳�lLimit�`�F�b�N
	if(_is_ntnet_remote() == 0){							// NT-NET
		wkul = prm_get(COM_PRM,S_NTN,34,2,5);				// �␳�l�L�����Ԑݒ�Get(�b)get(34-0034�@�A)
	}else{													// ���uNT-NET
		wkul = prm_get(COM_PRM,S_CEN,76,2,5);				// �␳�l�L�����Ԑݒ�Get(�b)get(36-0076�@�A)
	}
	wkul *= 1000L;											// �P�ʂ�ms�ɕϊ�
	if( wkul < RecvNtnetDt.RData119.HOSEI_MSec ){			// �L�����ԊO
		return;
	}

	// ���v�ݒ�l���m�[�}���C�Y���ĕ␳�������Z
	c_Normalize_ms( &wk_CLK_REC, &wk_CLK_REC_msec, &NormDay, &NormTime );	// �ݒ莞���m�[�}���C�Y�l(ms)get

	if(_is_ntnet_remote() == 0){											// NT-NET
		Setting_Supplement_msec = (ushort)prm_get(COM_PRM,S_NTN,34,4,1);	// �x�����Ԑݒ�l(ms)get(34-0034�B�C�D�E)
	}else{																	// ���uNT-NET
		Setting_Supplement_msec = (ushort)prm_get(COM_PRM,S_CEN,76,4,1);	// �x�����Ԑݒ�l(ms)get(36-0076�B�C�D�E)
	}

	wkul = c_2msPastTimeGet( RecvNtnetDt.RData119.HOSEI_wk );				// Arcnet��M�`���݂܂ł̌o�ߎ���(ms)get

	wkul = NormTime + RecvNtnetDt.RData119.HOSEI_MSec + Setting_Supplement_msec + wkul;	
																			// �␳�l�����Z���������m�[�}���C�Y�l �Z�o

	// �K�[�h�����i�Ȃ��ƍl���邪�K�[�h�j
	if( NormTime > wkul ){									// �␳�����玞�v�l���}�C�i�X�ɂȂ�ꍇ�͕␳���Ȃ�
		;
	}else{
		NormTime = wkul;
	}

	// ���v�̍X�V(set)�́A�{�d�������ɁA����00�~���b�̃^�C�~���O�ōs�Ȃ�
	// �����ł́A����00�b�܂ł̎���(�~���b)�ƁA���̎��̎����l�����߂�
	wkus = (ushort)(NormTime % 1000);						// �~���b����get
	WaitTime_ForTimeSet = 1000 - wkus;						// ����00�b�܂ł̎���get
	NormTime += (ulong)WaitTime_ForTimeSet;					// ���Ԃ�����00�b�ɂ���

	// �␳�l���Z�������v�ݒ�l���A���m�[�}���C�Y
	c_UnNormalize_ms( &NormDay, &NormTime, &wk_CLK_REC, &wk_CLK_REC_msec );

	xPause2ms( (ulong)(WaitTime_ForTimeSet/2) );			// 00�~���b�ƂȂ�܂�wait

	// ���v�X�V
	Ope_clk_set( &wk_CLK_REC, OPLOG_SET_TIME2_COM );
}

/*[]----------------------------------------------------------------------[]*/
/*| ���v�ް�(�ް����119)sub����                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| NT-NET lower���W���[�������v�f�[�^��M�Ɣ��f���������Call�����֐�   |*/
/*|	0�J�b�g�f�[�^�L���������Call�����B								   |*/
/*|	LifeTime�l���Z�b�g����												   |*/
/*|	���d���ł��i�d�������Z���Ă��j�������ĐV�d���i�f�[�^�̓I�[��0�j�Ƃ���  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : *dat = NT-NET�d�� ID3����̃f�[�^�ւ̃|�C���^		   |*/
/*|				   siz = dat�̃f�[�^�T�C�Y(ID3����)						   |*/
/*| RETURN VALUE : �ǉ������f�[�^��                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2009/05/12                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void NTNET_AfterRcv_CallBack()
{
	ulong	LifeTime;

	LifeTime = LifeTim2msGet();
	RecvNtnetDt.RData119.HOSEI_wk = LifeTime;				// +37(�\��)�Ɏ�M���_��LifeTime�l��set
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ԏ�ԍ��{�����ID�ϊ����� 		                          		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Convert_PassInfo                                        |*/
/*| PARAMETER    : ParkingNo:���ԏ�ԍ�                                    |*/
/*|		 : PassID   :�����ID					   						   |*/
/*| RETURN VALUE : �ϊ������l                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.namioka                                               |*/
/*| Date         : 2007-12-04                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
ulong	Convert_PassInfo( ulong ParkingNo, ulong PassID ){

	ulong ul_Prm;

	ul_Prm = ParkingNo;
	ul_Prm *= 100000L;				// ��������ԏ�set
	ul_Prm += PassID;				// �����IDset(1�`12000)

	return( ul_Prm );
}

/*[]----------------------------------------------------------------------[]*/
/*| ������⍇�������ް�(�ް����143)��M����                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData143                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-11-04                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData143( void )
{
	ushort status;
	ulong use_parking;
	ushort valid;
	uchar	parking_no_check=0;

	status = 0xFFFF;
	use_parking = 0xFFFFFFFF;
	valid = 0xFFFF;

	if (RecvNtnetDt.RData116.UsingParkingNo != 0) {
		use_parking = (ulong)RecvNtnetDt.RData143.UseParkingNo;
	}else{
		return;
	}

	parking_no_check = NTNET_GetParkingKind( RecvNtnetDt.RData143.ParkingNo, PKOFS_SEARCH_LOCAL );

	if (RecvNtnetDt.RData143.PassState == NTNET_PASSUPDATE_INVALID) {
		if( parking_no_check != 0xFF ){
			if(RecvNtnetDt.RData143.ParkingNo < 1000 ){	//APS
				wopelg(OPLOG_SET_TEIKI_YUKOMUKO, 0, Convert_PassInfo(RecvNtnetDt.RData143.ParkingNo,RecvNtnetDt.RData143.PassID));		// ���엚��o�^
			}else{											//GT
				wopelg(OPLOG_GT_SET_TEIKI_YUKOMUKO, RecvNtnetDt.RData143.ParkingNo, RecvNtnetDt.RData143.PassID);		// ���엚��o�^
			}
		}
		valid = 1;
	} else {
		if( parking_no_check != 0xFF ){
			if(RecvNtnetDt.RData143.ParkingNo < 1000 ){				//APS
				wopelg(OPLOG_SET_STATUS_CHENGE, 0, Convert_PassInfo(RecvNtnetDt.RData143.ParkingNo,RecvNtnetDt.RData143.PassID));		// ���엚��o�^
			}else{														//GT
				wopelg(OPLOG_GT_SET_STATUS_CHENGE, RecvNtnetDt.RData143.ParkingNo, RecvNtnetDt.RData143.PassID);		// ���엚��o�^
			}
		}
		valid = 0;
		status = RecvNtnetDt.RData143.PassState;
	}
	WritePassTbl( (ulong)RecvNtnetDt.RData143.ParkingNo,
				  (ushort)RecvNtnetDt.RData143.PassID,
				  (ushort)(status),
				  (ulong)use_parking,
				  (ushort)valid );

	if( RecvNtnetDt.RData143.ParkingNo == OPECTL.ChkPassPkno &&		// �⍇���������ԏꇂ�Ɠ��l?
		RecvNtnetDt.RData143.PassID == OPECTL.ChkPassID ){			// �⍇�����������ID�Ɠ��l?

		queset( OPETCBNO, IBK_NTNET_CHKPASS, 22, &RecvNtnetDt.RData143.Reserve );	// NTNET ����⍇�������ް���M�ʒm
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �����W��ID�e�[�u���f�[�^(�ް����150)��M����                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData150                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-06-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_RevData150( void )
{
}

/* �W��ID��ԃf�[�^�d���p�X�e�[�^�X */
#define	NTNET_KAKARI_YUKOU		0x00	/* �W��ID�L�� */
#define	NTNET_KAKARI_MUKOU		0x03	/* �W��ID���� */

/*[]----------------------------------------------------------------------[]*/
/*| �W��ID��ԃf�[�^(ID254)��M����                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData254		                                   |*/
/*| PARAMETER    : data  : �W��ID��ԃf�[�^�d��                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-06-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_RevData254( void )
{
	return ;
}

/* �W���ێ�f�[�^���M�p�X�e�[�^�X */
#define	NTNET_KAKARI_NYUKO		0x00	/* ����							*/
#define	NTNET_KAKARI_MNTSTART	0x01	/* �W�����ނɂ������ݽ�J�n		*/
#define	NTNET_KAKARI_MNTEND		0x02	/* ����ݽ�I��					*/
#define	NTNET_KAKARI_SYUKKO		0x03	/* �o��							*/
#define	NTNET_KAKARI_KAISYUU	0x04	/* �W�����ޖ����̂��߶��މ��	*/

/*[]----------------------------------------------------------------------[]*/
/*| �W���ێ��ް�(ID108)�쐬���M����                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNETDOPA_Snd_Data108                                   |*/
/*| PARAMETER    : Status  : �W���ێ�f�[�^�X�e�[�^�X                      |*/
/*|             0:���� (NTDOPA_KAKARI_NYUKO)                               |*/
/*|             1:�W���J�[�h�ɂ�胁���e�i���X�J�n (NTDOPA_KAKARI_MNTSTAR) |*/
/*|             2:�����e�i���X�I�� (NTDOPA_KAKARI_MNTEND)                  |*/
/*|             3:�o�� (NTDOPA_KAKARI_SYUKKO)                              |*/
/*|             4:�W���J�[�h�����̈׃J�[�h��� (NTDOPA_KAKARI_KAISYUU)     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2006-06-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_Snd_Data108(ushort KakariNo, uchar Status)
{
	/* �X�e�[�^�X�l�`�F�b�N */
	if(Status != NTNET_KAKARI_NYUKO	&&
	   Status != NTNET_KAKARI_MNTSTART	&&
	   Status != NTNET_KAKARI_MNTEND	&&
	   Status != NTNET_KAKARI_SYUKKO	&&
	   Status != NTNET_KAKARI_KAISYUU	){
		return ;	
	}

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_108 ) );

	BasicDataMake( 108, 0x01 );									// ��{�ް��쐬

	SendNtnetDt.SData108.ParkingNo	= (ulong)CPrmSS[S_SYS][1];	// ���ԏꇂ
	SendNtnetDt.SData108.KakariNo	= KakariNo;					// �W��No.
	SendNtnetDt.SData108.Status		= Status;					// �X�e�[�^�X
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_108 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_108 ), NTNET_BUF_NORMAL );	// �ް����M�o�^
	}
	return ;
}

/*[]----------------------------------------------------------------------[]*/
/*| ��{�ް��쐬����                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BasicDataMake                                           |*/
/*| PARAMETER    : knd  : �ް����                                         |*/
/*|                keep : �ް��ێ��׸�                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	BasicDataMake( uchar knd, uchar keep )
{
	// ����ID
// MH322914(S) K.Onodera 2016/08/10 AI-V�Ή�
//	SendNtnetDt.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];
	switch( knd ){
		case PIP_RES_KIND_REMOTE_CALC_ENTRY:
		case PIP_RES_KIND_REMOTE_CALC_FEE:
		case PIP_RES_KIND_OCCUR_ERRALM:
		case PIP_RES_KIND_RECEIPT_AGAIN:
		case PIP_RES_KIND_FURIKAE:
		case PIP_RES_KIND_RECEIVE_TKT:
			// �V�X�e��ID16�̉����H
			if( RecvNtnetDt.DataBasic.SystemID == PIP_SYSTEM ){
				SendNtnetDt.DataBasic.SystemID = PIP_SYSTEM;
				break;
			}
			// no break;
		default:
			SendNtnetDt.DataBasic.SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];
																	// 0:�ׯ�߼���
																	// 1:���u�ׯ�߼���
																	// 2:�ׯ�ߥ���ּ���
																	// 3:���u�ׯ�ߥ���ּ���
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
																	// 4:�ްĎ�����
																	// 5:���u�ްĎ�����
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		break;
	}
// MH322914(E) K.Onodera 2016/08/10 AI-V�Ή�

	SendNtnetDt.DataBasic.DataKind = knd;							// �ް����
	SendNtnetDt.DataBasic.DataKeep = keep;							// �ް��ێ��׸�
	SendNtnetDt.DataBasic.ParkingNo = (ulong)CPrmSS[S_SYS][1];		// ���ԏꇂ
	SendNtnetDt.DataBasic.ModelCode = NTNET_MODEL_CODE;				// �@����

	SendNtnetDt.DataBasic.MachineNo = (ulong)CPrmSS[S_PAY][2];		// �@�B��
	if(( knd == 100 )||( knd == 109 ))
		SendNtnetDt.DataBasic.MachineNo = (ulong)CPrmSS[S_NTN][3];	// ���M���^�[�~�i����
	if( ((knd >= 30) && (knd <= 36)) || (knd == 41)||	//T���v�f�[�^�̏ꍇ�͏W�v�������Ԃ��Z�b�g����
	    ((knd >=158) && (knd <=164)) || (knd ==169)){	//T���v�f�[�^�̏ꍇ�͏W�v�������Ԃ��Z�b�g����
		SendNtnetDt.DataBasic.Year = (uchar)( NTNetTTotalTime.Year % 100 );	// �N
		SendNtnetDt.DataBasic.Mon =  NTNetTTotalTime.Mon;					// ��
		SendNtnetDt.DataBasic.Day =  NTNetTTotalTime.Day;					// ��
		SendNtnetDt.DataBasic.Hour = NTNetTTotalTime.Hour;					// ��
		SendNtnetDt.DataBasic.Min =  NTNetTTotalTime.Min;					// ��
		SendNtnetDt.DataBasic.Sec =  (uchar)NTNetTTotalTime.Sec;			// �b
	}else if(knd == 152){
		switch(PayInfo_Class){										// ���Z���f�[�^�p�����敪���Q�Ƃ���
		case 8:														// ���Z�O�͗v�����ꂽ�������Z�b�g
			SendNtnetDt.DataBasic.Year = (uchar)( CLK_REC.year % 100 );		// �N
			SendNtnetDt.DataBasic.Mon = (uchar)CLK_REC.mont;				// ��
			SendNtnetDt.DataBasic.Day = (uchar)CLK_REC.date;				// ��
			SendNtnetDt.DataBasic.Hour = (uchar)CLK_REC.hour;				// ��
			SendNtnetDt.DataBasic.Min = (uchar)CLK_REC.minu;				// ��
			SendNtnetDt.DataBasic.Sec = (uchar)CLK_REC.seco;				// �b
			break;
		case 0:														// ���Z��͑O�񐸎Z�������Z�b�g
		case 2:														// ���Z���~�͑O�񐸎Z���~�������Z�b�g
// MH322914 (s) kasiyama 2016/07/11 ���Z��񉞓��f�[�^�̏����N�����ɐ��Z����������Ȃ�(�N���W�b�g)[���ʃo�ONo.1179]
		// �\�����ŏ����敪��0�ɂȂ�
		case 4:
// MH322914 (e) kasiyama 2016/07/11 ���Z��񉞓��f�[�^�̏����N�����ɐ��Z����������Ȃ�(�N���W�b�g)[���ʃo�ONo.1179]
			SendNtnetDt.DataBasic.Year = (uchar)( NTNetTime_152.Year % 100 );	// �N
			SendNtnetDt.DataBasic.Mon =  NTNetTime_152.Mon;						// ��
			SendNtnetDt.DataBasic.Day =  NTNetTime_152.Day;						// ��
			SendNtnetDt.DataBasic.Hour = NTNetTime_152.Hour;					// ��
			SendNtnetDt.DataBasic.Min =  NTNetTime_152.Min;						// ��
			SendNtnetDt.DataBasic.Sec =  (uchar)NTNetTime_152.Sec;				// �b
			break;
		case 9:															// ���Z���͐��Z�J�n�������Z�b�g
			SendNtnetDt.DataBasic.Year = (uchar)( NTNetTime_152_wk.Year % 100 );	// �N
			SendNtnetDt.DataBasic.Mon =  NTNetTime_152_wk.Mon;						// ��
			SendNtnetDt.DataBasic.Day =  NTNetTime_152_wk.Day;						// ��
			SendNtnetDt.DataBasic.Hour = NTNetTime_152_wk.Hour;						// ��
			SendNtnetDt.DataBasic.Min =  NTNetTime_152_wk.Min;						// ��
			SendNtnetDt.DataBasic.Sec =  (uchar)NTNetTime_152_wk.Sec;				// �b
			break;
		}
	}else{
		SendNtnetDt.DataBasic.Year = (uchar)( CLK_REC.year % 100 );		// �N
		SendNtnetDt.DataBasic.Mon = (uchar)CLK_REC.mont;				// ��
		SendNtnetDt.DataBasic.Day = (uchar)CLK_REC.date;				// ��
		SendNtnetDt.DataBasic.Hour = (uchar)CLK_REC.hour;				// ��
		SendNtnetDt.DataBasic.Min = (uchar)CLK_REC.minu;				// ��
		SendNtnetDt.DataBasic.Sec = (uchar)CLK_REC.seco;				// �b
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ��⍇���ް�(�ް����01)�쐬����                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data01                                        |*/
/*| PARAMETER    : op_lokno : �ڋq�p���Ԉʒu�ԍ�(��2��A�`Z,��4��1�`9999)   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-10-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data01( ulong op_lokno )
{
	if (_is_ntnet_remote()) {
		return;
	}

	memset( &LOCKMULTI, 0, sizeof( lock_multi ) );
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_01 ) );

	BasicDataMake( 1, 1 );											// ��{�ް��쐬

	SendNtnetDt.SData01.LockNo = op_lokno;							// �����

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_01 ));

	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_01 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ��⍇�������ް�(�ް����02)�쐬����                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data02                                        |*/
/*| PARAMETER    : op_lokno : �ڋq�p���Ԉʒu�ԍ�(��2��A�`Z,��4��1�`9999)   |*/
/*|                pr_lokno : ���������p���Ԉʒu�ԍ�(1�`324)               |*/
/*|                ans      : ���� 0=����, 1=���ڑ�(�Y������)              |*/
/*|                                2=�蓮Ӱ�ޒ�                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data02( ulong op_lokno, ushort pr_lokno, ushort ans )
{
	ushort	lkno;
	uchar	cnt;
	uchar	i,j;
	ulong	cal;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_02 ) );

	BasicDataMake( 2, 1 );											// ��{�ް��쐬
	// ���ԏꇂ�`�����b�܂ł͎Ԏ��⍇���ް��ɂĎ�M�����ް������̂܂ܾ�Ă���
	memcpy( &SendNtnetDt.DataBasic.ParkingNo, &RecvNtnetDt.DataBasic.ParkingNo, 22 );

	SendNtnetDt.SData02.LockNo = op_lokno;							// �����
	SendNtnetDt.SData02.Answer = ans;								// ����
	if( ans == 0 || ans == 2 ){		// ���ʁ��u����v�܂��́u�蓮Ӱ�ޒ��v�H
		lkno = pr_lokno - 1;

		FLAPDT.flp_data[lkno].ryo_syu = (ushort)LockInfo[lkno].ryo_syu;

		memcpy( SendNtnetDt.SData02.LkInfo,
				&FLAPDT.flp_data[lkno].mode,
				sizeof( SendNtnetDt.SData02.LkInfo ) );				// �Ԏ����
		SendNtnetDt.SData02.LkInfo[2] &= 0x03;
		SendNtnetDt.SData02.LkInfo[4] &= 0x03;
		memset( &SendNtnetDt.SData02.LkInfo[26],0,sizeof(char)*8);	//���~�����A�̎��z�ɐ��Z�����ް�����Ă����׸ر
		cal = OrgCal( (ushort)lkno );
		memcpy( &SendNtnetDt.SData02.LkInfo[26], &cal, 4);
		memset( &SendNtnetDt.SData02.LkInfo[46], 0, 2 );	// ���ɔ������/�ׯ�ߏ㏸��ς͕s�v�Ȃ̂ŃZ�b�g���Ȃ�
		
		cnt = cyushi_chk( lkno );			//�ڍג��~����
		if( cnt != 99 ){
			SendNtnetDt.SData02.PpcData = FLAPDT_SUB[cnt].ppc_chusi_ryo;
			for(i=0 ; i<15 ; i++)
				SendNtnetDt.SData02.SvsTiket[i] = FLAPDT_SUB[cnt].sev_tik[i];
			for(i=0,j=0 ; i<5 ; i++,j+=2){
				SendNtnetDt.SData02.KakeTiket[j] = FLAPDT_SUB[cnt].kake_data[i].mise_no;
				SendNtnetDt.SData02.KakeTiket[j+1] = FLAPDT_SUB[cnt].kake_data[i].maisuu;
			}
		}
	}

	if( ans == 0 ){
		IoLog_write(IOLOG_EVNT_AJAST_STA, (ushort)op_lokno, 0, 0);
	}
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_02 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_02 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ۯ����䌋���ް�(�ް����05)�쐬����                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data05                                        |*/
/*| PARAMETER    : op_lokno : �ڋq�p���Ԉʒu�ԍ�(��2��A�`Z,��4��1�`9999)   |*/
/*|                loksns   : �ԗ����m���                                 |*/
/*|                lokst    : ۯ����u���                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data05( ulong op_lokno, uchar loksns, uchar lokst )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_05 ) );

	BasicDataMake( 5, 1 );											// ��{�ް��쐬

	// ���ԏꇂ�`�����b�܂ł͎Ԏ��⍇���ް��ɂĎ�M�����ް������̂܂ܾ�Ă���
	memcpy( &SendNtnetDt.DataBasic.ParkingNo, &RecvBackUp.ParkingNo, 22 );

	SendNtnetDt.SData05.LockNo = op_lokno;							// �����
	SendNtnetDt.SData05.LockSense = loksns;							// �ԗ����m���
	SendNtnetDt.SData05.LockState = lokst;							// ۯ����u���

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_05 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_05 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}

	RecvBackUp.ReceiveFlg = 0;										// �v������M
}

/*[]----------------------------------------------------------------------[]*/
/*| �ȈՎԎ����ð���(�ް����12)�쐬����                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data12                                        |*/
/*| PARAMETER    : MachineNo : ���M��[���@�B��                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data12( ulong MachineNo )
{
	short	i;
	short	j = 0;
	ushort	len = 0;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_12 ) );

	BasicDataMake( 12, 1 );											// ��{�ް��쐬

	SendNtnetDt.SData12.SMachineNo = MachineNo;						// ���M��[���@�B��
	len = sizeof(DATA_BASIC) + 4;	// ��{�f�[�^�{���M��[���@�B��
	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;														// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s

		if (_is_ntnet_normal() || FlpSetChk((ushort)i)) {
			// �L���ȎԎ����̂݃Z�b�g
			if( !SetCarInfoSelect(i) ){
				continue;
			}
			if( j >= OLD_LOCK_MAX ){
				break;
			}
			SendNtnetDt.SData12.LockState[j].LockNo =
				(ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// �����

			SendNtnetDt.SData12.LockState[j].NowState =
									FLAPDT.flp_data[i].nstat.word & 0x01FF;		// ���ݽð��

			SendNtnetDt.SData12.LockState[j].Year =
									FLAPDT.flp_data[i].year;			// ���ɔN

			SendNtnetDt.SData12.LockState[j].Mont =
									FLAPDT.flp_data[i].mont;			// ���Ɍ�

			SendNtnetDt.SData12.LockState[j].Date =
									FLAPDT.flp_data[i].date;			// ���ɓ�

			SendNtnetDt.SData12.LockState[j].Hour =
									FLAPDT.flp_data[i].hour;			// ���Ɏ�

			SendNtnetDt.SData12.LockState[j].Minu =
									FLAPDT.flp_data[i].minu;			// ���ɕ�

			SendNtnetDt.SData12.LockState[j].Syubet =
									(ushort)LockInfo[i].ryo_syu;		// �������

			SendNtnetDt.SData12.LockState[j].TyuRyo = 0L;				// ���ԗ���(���g�p)

			j++;
			len += sizeof(LOCK_STATE);
		}
	}
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, len, NTNET_BUF_NORMAL );	// �ް����M�o�^
	}
}
void	NTNET_Snd_ParkCarNumDataMk(void *p, uchar knd)
{
	int i;
	PARKCAR_DATA11	*p1;
	PARKCAR_DATA21	*p2;

	// ���A���^�C���Ȗ���Ԕ��ʂ��擾����ׂɁA���ݑ䐔�������s��
	//�i5�b���Ƃɍs�Ȃ���M���o������sig_chk()���ŃR�[������Ă��邪�A���A���^�C���ȏ��ɂ���ׁj
	fulchk();																// ���ݑ䐔����

	if( knd == 0 ) {
		p1 = (PARKCAR_DATA11 *)p;

		memset( p1, 0, sizeof( PARKCAR_DATA11 ) );

		// ����ԗ��J�E���g 0�Œ�

		if (_is_ntnet_remote()) {											// ���uNT-NET�ݒ�̂Ƃ��̂݃f�[�^�Z�b�g
			if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {					// 01-0039�D���ԑ䐔��{�̂ŊǗ�����
				switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
				case 0:														// 01-0039�E�p�r�ʒ��ԑ䐔�ؑւȂ�
					p1->FullNo1 = (ushort)PPrmSS[S_P02][3];					// ���ԑ䐔
					p1->CarCnt1 = (ushort)PPrmSS[S_P02][2];					// ���ݑ䐔
					switch (PPrmSS[S_P02][1]) {								// ���������Ӱ��
					case	1:												// ��������
						p1->Full[0] = 11;									// �u�������ԁv��ԃZ�b�g
						break;
					case	2:												// �������
						p1->Full[0] = 10;									// �u������ԁv��ԃZ�b�g
						break;
					default:												// ����
						p1->Full[0] = CARFULL_stat[0];						// �u���or���ԁv��ԃZ�b�g
						break;
					}
					break;
				case 3:														// 01-0039�E�p�r�ʒ��ԑ䐔�Q�n��
				case 4:														// 01-0039�E�p�r�ʒ��ԑ䐔�R�n��
					p1->FullNo1 = (ushort)PPrmSS[S_P02][7];					// ���ԑ䐔
					p1->CarCnt1 = (ushort)PPrmSS[S_P02][6];					// ���ݑ䐔�P
					switch (PPrmSS[S_P02][5]) {								// ��������ԃ��[�h1
					case	1:												// ��������
						p1->Full[0] = 11;									// �u�������ԁv��ԃZ�b�g
						break;
					case	2:												// �������
						p1->Full[0] = 10;									// �u������ԁv��ԃZ�b�g
						break;
					default:												// ����
						p1->Full[0] = CARFULL_stat[0];						// �u���or���ԁv��ԃZ�b�g
						break;
					}

					p1->FullNo2 = (ushort)PPrmSS[S_P02][11];				// ���ԑ䐔�Q
					p1->CarCnt2 = (ushort)PPrmSS[S_P02][10];				// ���ݑ䐔�Q
					switch (PPrmSS[S_P02][9]) {								// ��������ԃ��[�h2
					case	1:												// ��������
						p1->Full[1] = 11;									// �u�������ԁv��ԃZ�b�g
						break;
					case	2:												// �������
						p1->Full[1] = 10;									// �u������ԁv��ԃZ�b�g
						break;
					default:												// ����
						p1->Full[1] = CARFULL_stat[1];						// �u���or���ԁv��ԃZ�b�g
						break;
					}

					if (i == 4) {
						p1->FullNo3 = (ushort)PPrmSS[S_P02][15];			// ���ԑ䐔�R
						p1->CarCnt3 = (ushort)PPrmSS[S_P02][14];			// ���ݑ䐔�R
						switch (PPrmSS[S_P02][13]) {						// ��������ԃ��[�h3
						case	1:											// ��������
							p1->Full[2] = 11;								// �u�������ԁv��ԃZ�b�g
							break;
						case	2:											// �������
							p1->Full[2] = 10;								// �u������ԁv��ԃZ�b�g
							break;
						default:											// ����
							p1->Full[2] = CARFULL_stat[2];					// �u���or���ԁv��ԃZ�b�g
							break;
						}
					}
					break;
				default:
					break;
				}
			}
		}
	} else {
		p2 = (PARKCAR_DATA21 *)p;

		memset( p2, 0, sizeof( PARKCAR_DATA21 ) );

		// ��ٰ�ߔԍ�

		// �p�r�ʒ��ԑ䐔�ݒ�

		if (_is_ntnet_remote()) {											// ���uNT-NET�ݒ�̂Ƃ��̂݃f�[�^�Z�b�g
			if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {					// 01-0039�D���ԑ䐔��{�̂ŊǗ�����
				switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
				case 0:														// 01-0039�E�p�r�ʒ��ԑ䐔�ؑւȂ�
					p2->FullNo1 = (ushort)PPrmSS[S_P02][3];					// ���ԑ䐔
					if (PPrmSS[S_P02][2] > PPrmSS[S_P02][3]) {
						p2->EmptyNo1 = 0;									// ��ԑ䐔
					} else {
						p2->EmptyNo1 = (ushort)(PPrmSS[S_P02][3] - PPrmSS[S_P02][2]);	// ��ԑ䐔(���ԑ䐔 - ���ݒ��ԑ䐔)
					}
					switch (PPrmSS[S_P02][1]) {								// ���������Ӱ��
					case	1:												// ��������
						p2->FullSts1 = 11;									// �u�������ԁv��ԃZ�b�g
						break;
					case	2:												// �������
						p2->FullSts1 = 10;									// �u������ԁv��ԃZ�b�g
						break;
					default:												// ����
						p2->FullSts1 = CARFULL_stat[0];						// �u���or���ԁv��ԃZ�b�g
						break;
					}
					break;
				case 3:														// 01-0039�E�p�r�ʒ��ԑ䐔�Q�n��
				case 4:														// 01-0039�E�p�r�ʒ��ԑ䐔�R�n��
					p2->FullNo1 = (ushort)PPrmSS[S_P02][7];					// ���ԑ䐔�P
					if (PPrmSS[S_P02][6] > PPrmSS[S_P02][7]) {
						p2->EmptyNo1 = 0;									// ��ԑ䐔�P
					} else {
						p2->EmptyNo1 = (ushort)(PPrmSS[S_P02][7] - PPrmSS[S_P02][6]);	// ��ԑ䐔�P(���ԑ䐔 - ���ݒ��ԑ䐔)
					}
					switch (PPrmSS[S_P02][5]) {								// ��������ԃ��[�h1
					case	1:												// ��������
						p2->FullSts1 = 11;									// �u�������ԁv��ԃZ�b�g
						break;
					case	2:												// �������
						p2->FullSts1 = 10;									// �u������ԁv��ԃZ�b�g
						break;
					default:												// ����
						p2->FullSts1 = CARFULL_stat[0];						// �u���or���ԁv��ԃZ�b�g
						break;
					}

					p2->FullNo2 = (ushort)PPrmSS[S_P02][11];				// ���ԑ䐔�Q
					if (PPrmSS[S_P02][10] > PPrmSS[S_P02][11]) {
						p2->EmptyNo2 = 0;									// ��ԑ䐔�Q
					} else {
						p2->EmptyNo2 = (ushort)(PPrmSS[S_P02][11] - PPrmSS[S_P02][10]);	// ��ԑ䐔�Q(���ԑ䐔 - ���ݒ��ԑ䐔)
					}
					switch (PPrmSS[S_P02][9]) {								// ��������ԃ��[�h2
					case	1:												// ��������
						p2->FullSts2 = 11;									// �u�������ԁv��ԃZ�b�g
						break;
					case	2:												// �������
						p2->FullSts2 = 10;									// �u������ԁv��ԃZ�b�g
						break;
					default:												// ����
						p2->FullSts2 = CARFULL_stat[1];						// �u���or���ԁv��ԃZ�b�g
						break;
					}

					if (i == 4) {
						p2->FullNo3 = (ushort)PPrmSS[S_P02][15];			// ���ԑ䐔�R
						if (PPrmSS[S_P02][14] > PPrmSS[S_P02][15]) {
							p2->EmptyNo3 = 0;								// ��ԑ䐔�R
						} else {
							p2->EmptyNo3 = (ushort)(PPrmSS[S_P02][15] - PPrmSS[S_P02][14]);	// ��ԑ䐔�R(���ԑ䐔 - ���ݒ��ԑ䐔)
						}
						switch (PPrmSS[S_P02][13]) {						// ��������ԃ��[�h3
						case	1:											// ��������
							p2->FullSts3 = 11;								// �u�������ԁv��ԃZ�b�g
							break;
						case	2:											// �������
							p2->FullSts3 = 10;								// �u������ԁv��ԃZ�b�g
							break;
						default:											// ����
							p2->FullSts3 = CARFULL_stat[2];					// �u���or���ԁv��ԃZ�b�g
							break;
						}
					}
					break;

				default:
					break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ް�(�ް����20)�쐬����                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data20                                        |*/
/*| PARAMETER    : pr_lokno : ���������p���Ԉʒu�ԍ�(1�`324)               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data20( ushort pr_lokno )
{
	ushort	lkno;
	const t_NtBufState	*ntbufst;
	uchar	ans;
	ushort	len;

	lkno = pr_lokno - 1;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_20 ) );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		BasicDataMake( 54, 0 );										// ��{�ް��쐬
	} else {
		BasicDataMake( 20, 0 );										// ��{�ް��쐬
	}

	SendNtnetDt.SData20.InCount = NTNetDataCont[0];					// ���ɒǂ���
	NTNetDataCont[0]++;												// ���ɒǂ���+1

	SendNtnetDt.SData20.Syubet = LockInfo[lkno].ryo_syu;			// �����敪

	if( FLAPDT.flp_data[lkno].lag_to_in.BIT.SYUUS == 1 ){	// �C�����Z�̎�
		SendNtnetDt.SData20.InMode = 6;								// ����Ӱ�ށF�C�����Z����
	}else{
		SendNtnetDt.SData20.InMode = (FLAPDT.flp_data[lkno].lag_to_in.BIT.FUKUG)? 5 : 0;	// ����Ӱ��
	}

	SendNtnetDt.SData20.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
								+ LockInfo[lkno].posi );			// �����
	SendNtnetDt.SData20.CardType = 0;								// ���Ԍ�����(���g�p)
	SendNtnetDt.SData20.CMachineNo = 0;								// ���Ԍ��@�B��
	SendNtnetDt.SData20.CardNo = 0L;								// ���Ԍ��ԍ�(�����ǂ���)
	SendNtnetDt.SData20.InTime.Year = FLAPDT.flp_data[lkno].year;	// ���ɔN
	SendNtnetDt.SData20.InTime.Mon = FLAPDT.flp_data[lkno].mont;	// ���Ɍ�
	SendNtnetDt.SData20.InTime.Day = FLAPDT.flp_data[lkno].date;	// ���ɓ�
	SendNtnetDt.SData20.InTime.Hour = FLAPDT.flp_data[lkno].hour;	// ���Ɏ�
	SendNtnetDt.SData20.InTime.Min = FLAPDT.flp_data[lkno].minu;	// ���ɕ�
	SendNtnetDt.SData20.InTime.Sec = 0;								// ���ɕb
	SendNtnetDt.SData20.PassCheck = 1;								// ����߽�������Ȃ�
	if( prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[lkno].ryo_syu-1)*6)),1,1 ) ){	// ��ʖ����Ă���ݒ�?
		SendNtnetDt.SData20.CountSet = 0;							// �ݎԶ���(����)
	}else{
		SendNtnetDt.SData20.CountSet = 1;							// �ݎԶ���(���Ȃ�)
	}
	// ������ް���0�Œ�

	len = sizeof( DATA_KIND_20 );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		memmove(&SendNtnetDt.SData54.InCount, &SendNtnetDt.SData20.InCount, sizeof(DATA_KIND_20)-sizeof(DATA_BASIC));

		SendNtnetDt.SData54.FmtRev = 0;								// �t�H�[�}�b�gRev.��
		NTNET_Snd_ParkCarNumDataMk(&SendNtnetDt.SData54.ParkData, 0);

		len = sizeof( DATA_KIND_54 );
	}
	if(_is_ntnet_remote()) {
		ans = RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		ans = NTBUF_SetSendNtData( &SendNtnetDt, len, NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
	switch( ans ){
	case NTNET_BUFSET_NORMAL:										// ����I��
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_SENDBUF, NTERR_RELEASE, 0, 0 );
		break;
	case NTNET_BUFSET_STATE_CHG:									// �ޯ̧��ԕω�(�����݂͊���)
		ntbufst = NTBUF_GetBufState();
		if( ntbufst->car_in & 0x02 ){								// �ޯ̧FULL����?
			err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_BUFFULL, NTERR_EMERGE, 0, 0 );
		}
		queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
		break;
	case NTNET_BUFSET_DEL_OLD:										// �Ō��ް�������
	case NTNET_BUFSET_DEL_NEW:										// �ŐV�ް�������
	case NTNET_BUFSET_CANT_DEL:										// �ޯ̧FULL�����ݒ肪"�x��"�̂��ߏ����s��
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_SENDBUF, NTERR_EMERGE, 0, 0 );
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ް�(�ް����20)�쐬���� �U�֐��Z���̉����ɗp���M�֐�              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data20_frs                                    |*/
/*| PARAMETER    : pr_lokno : ���������p���Ԉʒu�ԍ�(1�`324)               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
void	NTNET_Snd_Data20_frs( ushort pr_lokno, void *data )
{
	ushort	lkno;
	const t_NtBufState	*ntbufst;
	uchar	ans;
	flp_com *flp_data = (flp_com*)data;
	ushort	len;

	lkno = pr_lokno - 1;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_20 ) );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		BasicDataMake( 54, 0 );										// ��{�ް��쐬
	} else {
		BasicDataMake( 20, 0 );										// ��{�ް��쐬
	}

	SendNtnetDt.SData20.InCount = NTNetDataCont[0];					// ���ɒǂ���
	NTNetDataCont[0]++;												// ���ɒǂ���+1

	SendNtnetDt.SData20.Syubet = LockInfo[lkno].ryo_syu;			// �����敪
	SendNtnetDt.SData20.InMode = (flp_data->lag_to_in.BIT.FUKUG)? 5 : 0;		// ����Ӱ��
	SendNtnetDt.SData20.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
								+ LockInfo[lkno].posi );			// �����
	SendNtnetDt.SData20.CardType = 0;								// ���Ԍ�����(���g�p)
	SendNtnetDt.SData20.CMachineNo = 0;								// ���Ԍ��@�B��
	SendNtnetDt.SData20.CardNo = 0L;								// ���Ԍ��ԍ�(�����ǂ���)
	SendNtnetDt.SData20.InTime.Year = flp_data->year;	// ���ɔN
	SendNtnetDt.SData20.InTime.Mon = flp_data->mont;	// ���Ɍ�
	SendNtnetDt.SData20.InTime.Day = flp_data->date;	// ���ɓ�
	SendNtnetDt.SData20.InTime.Hour = flp_data->hour;	// ���Ɏ�
	SendNtnetDt.SData20.InTime.Min = flp_data->minu;	// ���ɕ�
	SendNtnetDt.SData20.InTime.Sec = 0;								// ���ɕb
	SendNtnetDt.SData20.PassCheck = 1;						// ����߽�������Ȃ�

	if( prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[lkno].ryo_syu-1)*6)),1,1 ) ){	// ��ʖ����Ă���ݒ�?
		SendNtnetDt.SData20.CountSet = 0;							// �ݎԶ���(����)
	}else{
		SendNtnetDt.SData20.CountSet = 1;							// �ݎԶ���(���Ȃ�)
	}
	// ������ް���0�Œ�

	len = sizeof( DATA_KIND_20 );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		memmove(&SendNtnetDt.SData54.InCount, &SendNtnetDt.SData20.InCount, sizeof(DATA_KIND_20)-sizeof(DATA_BASIC));

		SendNtnetDt.SData54.FmtRev = 0;								// �t�H�[�}�b�gRev.��
		NTNET_Snd_ParkCarNumDataMk(&SendNtnetDt.SData54.ParkData, 0);

		len = sizeof( DATA_KIND_54 );
	}
	if(_is_ntnet_remote()) {
		ans = RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		ans = NTBUF_SetSendNtData( &SendNtnetDt, len, NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
	switch( ans ){
	case NTNET_BUFSET_NORMAL:										// ����I��
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_SENDBUF, NTERR_RELEASE, 0, 0 );
		break;
	case NTNET_BUFSET_STATE_CHG:									// �ޯ̧��ԕω�(�����݂͊���)
		ntbufst = NTBUF_GetBufState();
		if( ntbufst->car_in & 0x02 ){								// �ޯ̧FULL����?
			err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_BUFFULL, NTERR_EMERGE, 0, 0 );
		}
		queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
		break;
	case NTNET_BUFSET_DEL_OLD:										// �Ō��ް�������
	case NTNET_BUFSET_DEL_NEW:										// �ŐV�ް�������
	case NTNET_BUFSET_CANT_DEL:										// �ޯ̧FULL�����ݒ肪"�x��"�̂��ߏ����s��
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID20_SENDBUF, NTERR_EMERGE, 0, 0 );
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �N���W�b�g���Z����                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_decision_credit                                   |*/
/*| RETURN VALUE : 1 as credit                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.9.20                                               |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int	ntnet_decision_credit(credit_use *p)
{
	if (p->pay_ryo) {
		switch(p->cre_type) {
		default:
			break;
		}
		return 1;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z���̕s���o�ɑ΍�												   |*/
/*| ���Z���������ꍇ�A���̎Ԏ����s���o�ɓo�^����Ă���Ώ�������B		   |*/
/*| �����ԗ������Z�������̂�Call���邱�ƁB								   |*/
/*| �@�i����X�V���A���Z���~����Call���Ă͂Ȃ�Ȃ��j					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data22_FusDel                                 |*/
/*| PARAMETER    : pr_lokno  : ���������p���Ԉʒu�ԍ�(1�`324)              |*/
/*|              : paymethod : ���Z����                                    |*/
/*|              : payclass  : �����敪                                    |*/
/*|              : outkind   : 0=�ʏ퐸�Z, 1=�����o��                      |*/
/*|              :             2=���Z���Ȃ��o��, 3=�s���o��                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                   |*/
/*| Date         : 2007-12-12                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data22_FusDel( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind )
{
	ushort	i,j;
	ushort	FuseiDataCount;										// �s���E�����ް��� �o�^����
	uchar	f_FuseiDataFound = 0;								// 1=found
	char	save_cycl_fg;										// ���Z�����׸ޕۑ��ر


	if( pr_lokno == 0xffff ){									// ���ݐ��Z�̂Ƃ�
		return;													// ���̐��Z�@�̎Ԏ��Ȃ̂Ńf�[�^�Ȃ�
	}

	if( ( paymethod != 5 ) &&									// ����X�V���Z
		( payclass != 2 && payclass != 3 ) &&					// ���Z���~
		( fusei.kensuu != 0) )									// �s���o�ɏ�񂠂�
	{
		FuseiDataCount = fusei.kensuu;							// �s���E�����ް��� �o�^����get
		if( FuseiDataCount > LOCK_MAX ){						// �ް��j�����̶ް��
			FuseiDataCount = LOCK_MAX;
		}

		for( j=0; j<FuseiDataCount; j++ ){
			if( fusei.fus_d[j].t_iti == pr_lokno ){				// �����Ώۃf�[�^����
				f_FuseiDataFound = 1;							// �������̕s���o�ɋL�^�ɍ����Z���������Ԏ�������
				break;
			}
		}

		if( !f_FuseiDataFound )									// �����Ώۃf�[�^�Ȃ�
			return;

		// �s���f�[�^�G���A����Y���Ԏ��̃f�[�^����������
		// TempFus�ɏ�����f�[�^���쐬

		memset( &TempFus, 0, sizeof( struct FUSEI_SD ) );		// �s���ް�ܰ��ر�ر

		for( i=j=0; j<FuseiDataCount; j++ ){					// �s���E�����ް������������
			if( fusei.fus_d[j].t_iti == pr_lokno ){				// �����Ώۃf�[�^
				;	// �������Ȃ�
			}
			else{												// �������Ȃ��f�[�^
				memcpy( &TempFus.fus_d[i], &fusei.fus_d[j], sizeof(struct FUSEI_D) );	// 1����copy
				++i;											// ���i�[�ʒuset�i�� �o�^�����j
			}
		}
		TempFus.kensuu = i;										// �o�^����set
																// TempFus�ɐV�ް�set����
		// fusei�ر�̍X�V
		
		memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );	// �s���ް���ܰ��ر�i�ޯ�ذ�ޯ����ߗ̈�j��

		save_cycl_fg = ac_flg.cycl_fg;							// �����Z�����׸ޒl�ۑ�
		ac_flg.cycl_fg = 122;									// 122:�s���ް���ܰ��ر�֓]�������i���@�j

		memcpy( &fusei, &TempFus, sizeof( struct FUSEI_SD ) );	// �V�ް��ɍX�V

		ac_flg.cycl_fg = save_cycl_fg;							// ���Z�����׸ޒl���A�i���A�j
																// �s���ް��ر�X�V�����̂���
		// �y��d����z���@�`���A�Ԃɒ�d���������ꍇ�Awkfus �� fusei�ɖ߂��āAac_flg.cycl_fg = 15 �����蒼���B
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�ް�(�ް����22,23)�쐬����(�t���b�v�㏸�A���b�N����ϓ��o�ɗp)     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data22_LO                                     |*/
/*| PARAMETER    : pr_lokno  : ���������p���Ԉʒu�ԍ�(1�`324)              |*/
/*|              : paymethod : ���Z����                                    |*/
/*|              : payclass  : �����敪                                    |*/
/*|              : outkind   : 0=�ʏ퐸�Z, 1=�����o��                      |*/
/*|              :             2=���Z���Ȃ��o��, 3=�s���o��                |*/
/*|              :             97=�t���b�v�㏸�A���b�N����ϓ��o��          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2007-09-03                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data22_LO( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind )
{
	ushort	lkno;
	const t_NtBufState	*ntbufst;
	uchar	work, ans;
	ushort	len;

	ntbufst = NTBUF_GetBufState();
	if( ntbufst->sale & 0x01 ){										// �ޯ̧NEAR FULL������?
		return;														// Ʊ�ٔ������͕s�������̐��Z�ް��͍��Ȃ�
	}

	lkno = pr_lokno - 1;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_22 ) );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		if( CPrmSS[S_NTN][27] == 0 ){
			work = 57;												// �o���ް�
		}else{
			work = 56;												// ���O�ް�
		}
	} else {
	if( CPrmSS[S_NTN][27] == 0 ){
		work = 23;													// �o���ް�
	}else{
		work = 22;													// ���O�ް�
	}
	}
	BasicDataMake( work, 0 );										// ��{�ް��쐬

	CountGet( PAYMENT_COUNT, (ST_OIBAN *)&SendNtnetDt.SData22.PayCount );			// ���Z�ǂ���
	SendNtnetDt.SData22.PayMethod = paymethod;						// ���Z���@
	SendNtnetDt.SData22.PayClass = payclass;						// �����敪
	SendNtnetDt.SData22.PayMode = 0;								// ���ZӰ��
	SendNtnetDt.SData22.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
								+ LockInfo[lkno].posi );			// �����
	SendNtnetDt.SData22.CardType = 0;								// ���Ԍ�����
	SendNtnetDt.SData22.CMachineNo = 0;								// ���Ԍ��@�B��
	SendNtnetDt.SData22.CardNo = 0L;								// ���Ԍ��ԍ�(�����ǂ���)
	SendNtnetDt.SData22.OutTime.Year = car_ot_f.year;				// �o�ɔN
	SendNtnetDt.SData22.OutTime.Mon = car_ot_f.mon;					// �o�Ɍ�
	SendNtnetDt.SData22.OutTime.Day = car_ot_f.day;					// �o�ɓ�
	SendNtnetDt.SData22.OutTime.Hour = car_ot_f.hour;				// �o�Ɏ�
	SendNtnetDt.SData22.OutTime.Min = car_ot_f.min;					// �o�ɕ�
	SendNtnetDt.SData22.OutTime.Sec = 0;							// �o�ɕb

	SendNtnetDt.SData22.KakariNo = 0;								// �W����
	SendNtnetDt.SData22.OutKind = outkind;							// ���Z�o��
	SendNtnetDt.SData22.InTime.Year = car_in_f.year;				// ���ɔN
	SendNtnetDt.SData22.InTime.Mon = car_in_f.mon;					// ���Ɍ�
	SendNtnetDt.SData22.InTime.Day = car_in_f.day;					// ���ɓ�
	SendNtnetDt.SData22.InTime.Hour = car_in_f.hour;				// ���Ɏ�
	SendNtnetDt.SData22.InTime.Min = car_in_f.min;					// ���ɕ�
	SendNtnetDt.SData22.InTime.Sec = 0;								// ���ɕb
	// ���Z�N���������b��0�Ƃ���
	// �O�񐸎Z�N���������b��0�Ƃ���
	SendNtnetDt.SData22.TaxPrice = 0;								// �ېőΏۊz
	SendNtnetDt.SData22.TotalPrice = 0;								// ���v���z(HOST���g�p�̂���)
	SendNtnetDt.SData22.Tax = 0;									// ����Ŋz
	SendNtnetDt.SData22.Syubet = (ushort)LockInfo[lkno].ryo_syu;		// �������
	SendNtnetDt.SData22.Price = 0;									// ���ԗ���
	SendNtnetDt.SData22.PassCheck = 1;								// ����߽����
	if(( CPrmSS[S_NTN][27] == 0 )&&									// �o���ް�?
	   ( prm_get( COM_PRM,S_SHA,(short)(2+((ryo_buf.tik_syu-1)*6)),1,1 ) )){	// ��ʖ����Ă���ݒ�?
		SendNtnetDt.SData22.CountSet = 2;							// �ݎԶ���-1����
	}else{
		SendNtnetDt.SData22.CountSet = 1;							// �ݎԶ��Ă��Ȃ�
	}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//	memset( SendNtnetDt.SData22.Reserve1, 0x20, 2 );				// �ő嗿���K�p�񐔁i�����Ή��j
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

	len = sizeof( DATA_KIND_22 );

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		memmove(&SendNtnetDt.SData56.PayCount, &SendNtnetDt.SData22.PayCount, sizeof(DATA_KIND_22)-sizeof(DATA_BASIC));

		SendNtnetDt.SData56.FmtRev = 0;								// �t�H�[�}�b�gRev.��
		NTNET_Snd_ParkCarNumDataMk(&SendNtnetDt.SData56.ParkData, 0);

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 		len = sizeof( DATA_KIND_56 ) - sizeof( DISCOUNT_DATA );
		len = sizeof( DATA_KIND_56 ) - sizeof( t_SeisanDiscountOld );
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	}
	if(_is_ntnet_remote()) {
		ans = RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		ans = NTBUF_SetSendNtData( &SendNtnetDt, len, NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
	switch( ans ){
	case NTNET_BUFSET_NORMAL:										// ����I��
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_SENDBUF, NTERR_RELEASE, 0, 0 );
		break;
	case NTNET_BUFSET_STATE_CHG:									// �ޯ̧��ԕω�(�����݂͊���)
		ntbufst = NTBUF_GetBufState();
		if( ntbufst->sale & 0x02 ){									// �ޯ̧FULL����?
			err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_BUFFULL, NTERR_EMERGE, 0, 0 );
		}
		if( ntbufst->sale & 0x01 ){									// �ޯ̧NEAR FULL����?
			err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_BUFNFULL, NTERR_EMERGE, 0, 0 );
		}
		queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );
		break;
	case NTNET_BUFSET_DEL_OLD:										// �Ō��ް�������
	case NTNET_BUFSET_DEL_NEW:										// �ŐV�ް�������
	case NTNET_BUFSET_CANT_DEL:										// �ޯ̧FULL�����ݒ肪"�x��"�̂��ߏ����s��
		err_chk( ERRMDL_NTNET, ERR_NTNET_ID22_SENDBUF, NTERR_EMERGE, 0, 0 );
		break;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���ԑ䐔�f�[�^/���ԑ䐔�����f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_DataParkCarNum
 *| PARAMETER    : uchar 58: / 59:
 *|              : mod : ���M���[�h
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2012-03-06
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTNET_Snd_DataParkCarNumWeb(uchar kind)
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_58 ) );

	// ��{�ް��쐬
	BasicDataMake( kind, 1 );
	// �t�H�[�}�b�gRev.��
	SendNtnetDt.SData58.FmtRev = 0;

	// ���ԑ䐔�f�[�^�쐬
	NTNET_Snd_ParkCarNumDataMk((void*)&SendNtnetDt.SData58.ParkData, 1);

	// �ް����M�o�^
	if(memcmp(&SData58_bk.ParkData, &SendNtnetDt.SData58.ParkData, sizeof(PARKCAR_DATA2)) != 0) {
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_58 ));
		}
		else {
			NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_58), NTNET_BUF_NORMAL);
		}
		memcpy(&SData58_bk, &SendNtnetDt.SData58, sizeof(DATA_KIND_58));
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���ԑ䐔�f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data58
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2012-03-06
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTNET_Snd_Data58()
{
	NTNET_Snd_DataParkCarNumWeb((uchar)58);
}


/*[]----------------------------------------------------------------------[]*/
/*| ���䉞���ް�(�ް����101)�쐬����                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data101                                       |*/
/*| PARAMETER    : MachineNo : ���M��[���@�B��                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data101( ulong MachineNo )
{
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//uchar	wk_ful;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_100 ) );

	BasicDataMake( 101, 1 );										// ��{�ް��쐬

	SendNtnetDt.SData101.SMachineNo = MachineNo;					// ���M��[���@�B��
	if( RecvNtnetDt.RData100.ControlData[0] ){						// �c�x�Ɛ؊�?
		if( FLAGPT.receive_rec.flg_ock == 0 ){
			SendNtnetDt.SData101.ControlData[0] = 3;				// ����
		}
		else if( FLAGPT.receive_rec.flg_ock == 1 ){
			SendNtnetDt.SData101.ControlData[0] = 2;				// �c��
		}
		else if( FLAGPT.receive_rec.flg_ock == 2 ){
			SendNtnetDt.SData101.ControlData[0] = 1;				// �x��
		}
	}

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	if (RecvNtnetDt.RData100.ControlData[1]) {
//		wk_ful = FLAGPT.car_full[1]; 
//
//		switch (wk_ful) {
//		case 0:
//			SendNtnetDt.SData101.ControlData[1] = 4;				// ����
//			break;
//		case 1:
//			SendNtnetDt.SData101.ControlData[1] = 1;				// ��������
//			break;
//		case 2:
//			SendNtnetDt.SData101.ControlData[1] = 2;				// �������
//			break;
//		case 3:
//			SendNtnetDt.SData101.ControlData[1] = 0;				// �قږ���
//			break;
//		default:
//			break;
//		}
//	}
//	if (RecvNtnetDt.RData100.ControlData[2]) {
//		switch (FLAGPT.car_full[2]) {
//		case 0:
//			SendNtnetDt.SData101.ControlData[2] = 4;				// ����
//			break;                                  
//		case 1:                                     
//			SendNtnetDt.SData101.ControlData[2] = 1;				// ��������
//			break;                                  
//		case 2:                                     
//			SendNtnetDt.SData101.ControlData[2] = 2;				// �������
//			break;                                  
//		case 3:                                     
//			SendNtnetDt.SData101.ControlData[2] = 0;				// �قږ���
//			break;
//		default:
//			break;
//		}
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	if( RecvNtnetDt.RData100.ControlData[3] ){						// ����߽�؊�?
		if( PPrmSS[S_P01][3] == 0 ){
			SendNtnetDt.SData101.ControlData[3] = 1;				// ����
		}
		else if( PPrmSS[S_P01][3] == 1 ){
			SendNtnetDt.SData101.ControlData[3] = 2;				// ����
		}
	}
	// ؾ�ėv�����󂯕t����
	SendNtnetDt.SData101.ControlData[7] = RecvNtnetDt.RData100.ControlData[7];	// ؾ�Ďw��
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	if (RecvNtnetDt.RData100.ControlData[10]) {
//		switch (FLAGPT.car_full[3]) {
//		case 0:
//			SendNtnetDt.SData101.ControlData[10] = 4;				// ����
//			break;                                  
//		case 1:                                     
//			SendNtnetDt.SData101.ControlData[10] = 1;				// ��������
//			break;                                  
//		case 2:                                     
//			SendNtnetDt.SData101.ControlData[10] = 2;				// �������
//			break;                                  
//		case 3:                                     
//			SendNtnetDt.SData101.ControlData[10] = 0;				// �قږ���
//			break;
//		default:
//			break;
//		}
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_100 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �Ǘ��ް��v��(�ް����104)�쐬����                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data104                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data104( ulong req )
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_103 ) );

	BasicDataMake( 104, 1 );										// ��{�ް��쐬

	SendNtnetDt.SData104.SMachineNo = 0L;							// ���M��[���@�B��
	if (req & NTNET_MANDATA_CLOCK) {
		// ���v�f�[�^���M�`�F�b�N
		SendNtnetDt.SData104.ControlData[0] = 1;
	}
	if (req & NTNET_MANDATA_CTRL) {
		// ����f�[�^���M�`�F�b�N
		SendNtnetDt.SData104.ControlData[1] = 1;
	}
	if (req & NTNET_MANDATA_COMMON) {
		// ���ʐݒ�f�[�^���M�`�F�b�N
		SendNtnetDt.SData104.ControlData[2] = 1;
	}
	if (req & NTNET_MANDATA_PASSSTS) {
		// ������X�e�[�^�X�f�[�^���M�`�F�b�N
		SendNtnetDt.SData104.ControlData[15] = 1;
	}
	if (req & NTNET_MANDATA_PASSCHANGE) {
		// ������X�V�f�[�^���M�`�F�b�N
		SendNtnetDt.SData104.ControlData[16] = 1;
	}
	if (req & NTNET_MANDATA_PASSEXIT) {
		// ������o�Ɏ����f�[�^���M�`�F�b�N
		SendNtnetDt.SData104.ControlData[17] = 1;
	}
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	if (req & NTNET_MANDATA_LOCKINFO) {
//		// �Ԏ��p�����[�^�f�[�^���M�`�F�b�N
//		SendNtnetDt.SData104.ControlData[19] = 1;
//	}
//	if (req & NTNET_MANDATA_LOCKMARKER) {
//		// ���b�N���u�f�[�^���M�`�F�b�N
//		SendNtnetDt.SData104.ControlData[20] = 1;
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
	if (req & NTNET_MANDATA_INVTKT) {
		// �������Ԍ��f�[�^���M�`�F�b�N
		SendNtnetDt.SData104.ControlData[11] = 1;
	}
	if (req & NTNET_MANDATA_PASSSTOP) {
		// ��������~�f�[�^���M�`�F�b�N
		SendNtnetDt.SData104.ControlData[13] = 1;
	}
	if (req & NTNET_MANDATA_SPECIALDAY) {
		// ���ʓ��ݒ�f�[�^���M�`�F�b�N
		SendNtnetDt.SData104.ControlData[18] = 1;
	}
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	if (req & NTNET_MANDATA_PARKNUMCTL) {
//		// ���ԑ䐔�Ǘ��f�[�^���M�`�F�b�N
//		SendNtnetDt.SData104.ControlData[25] = 1;
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_103 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_103 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �װ�ް�(�ް����120)�쐬����                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data120                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data120( void )
{
}


/*[]----------------------------------------------------------------------[]*/
/*| �װ�ް�(�ް����120)�쐬����(�װ�ر�v���p)                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data120_CL                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data120_CL( void )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_120 ) );

	BasicDataMake( 120, 0 );										// ��{�ް��쐬

	SendNtnetDt.SData120.Errsyu = 255;								// �װ���
	SendNtnetDt.SData120.Errcod = 255;								// �װ����
	SendNtnetDt.SData120.Errdtc = 2;								// �װ����/����
	SendNtnetDt.SData120.Errlev = 5;								// �װ����
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_120 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_120 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �װ��ް�(�ް����121)�쐬����                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data121                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data121( void )
{
}


/*[]----------------------------------------------------------------------[]*/
/*| �װ��ް�(�ް����121)�쐬����(�װѸر�v���p)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data121_CL                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data121_CL( void )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_121 ) );

	BasicDataMake( 121, 0 );										// ��{�ް��쐬

	SendNtnetDt.SData121.Armsyu = 255;		// �װ���
	SendNtnetDt.SData121.Armcod = 255;		// �װ����
	SendNtnetDt.SData121.Armdtc = 2;		// �װ����/����
	SendNtnetDt.SData121.Armlev = 5;		// �װ����
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_121 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_121 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ���K�Ǘ��ް�(�ް����126)�쐬����                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data126                                       |*/
/*| PARAMETER    : payclass : �����敪                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data126( ulong MachineNo, ushort payclass )
{
	char	i;
	ushort	wk_mai;
	uchar	f_Send;
	uchar	pos = 0;


	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_126 ) );

	BasicDataMake( 126, 0 );										// ��{�ް��쐬

	SendNtnetDt.SData126.SMachineNo = MachineNo;					// ���M��[���@�B��

	if( ( ( 0 == payclass ) || ( 1 == payclass ) ) && (OpeNtnetAddedInfo.PayMethod !=5 )){	// ���Z�����i���Zor�Đ��Z�jand ����X�V�ł͂Ȃ�
		if (ntnet_decision_credit(&PayData.credit)) {
			payclass += 4;											// �����敪��ڼޯĐ��Z�i�ڼޯčĐ��Z�j�ɂ���
		}
	}

	if( payclass <= 5 ){											// ���Z����
		SendNtnetDt.SData126.PayCount = CountSel( &PayData.Oiban);	// ���Z or ���Z���~�ǔ�set
// MH810105(S) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�
//		SendNtnetDt.SData126.PayTime.Year = PayData.TOutTime.Year;	// ���Z�N
//		SendNtnetDt.SData126.PayTime.Mon = PayData.TOutTime.Mon;	// ���Z��
//		SendNtnetDt.SData126.PayTime.Day = PayData.TOutTime.Day;	// ���Z��
//		SendNtnetDt.SData126.PayTime.Hour = PayData.TOutTime.Hour;	// ���Z��
//		SendNtnetDt.SData126.PayTime.Min = PayData.TOutTime.Min;	// ���Z��
//		SendNtnetDt.SData126.PayTime.Sec = 0;						// ���Z�b

// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//		// ���	(0=���O���Z�^1�������Z�o�ɐ��Z)
//		if(PayData.shubetsu == 0){
		// ���	(0=���O���Z�^1�������Z�o�ɐ��Z�^2=���u���Z(���Z���ύX))
		if(PayData.shubetsu == 0 || PayData.shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
			SendNtnetDt.SData126.PayTime.Year = PayData.TOutTime.Year;	// ���Z�N
			SendNtnetDt.SData126.PayTime.Mon = PayData.TOutTime.Mon;	// ���Z��
			SendNtnetDt.SData126.PayTime.Day = PayData.TOutTime.Day;	// ���Z��
			SendNtnetDt.SData126.PayTime.Hour = PayData.TOutTime.Hour;	// ���Z��
			SendNtnetDt.SData126.PayTime.Min = PayData.TOutTime.Min;	// ���Z��
			SendNtnetDt.SData126.PayTime.Sec = 0;						// ���Z�b
		}else{
			SendNtnetDt.SData126.PayTime.Year = PayData.TUnpaidPayTime.Year;	// ���Z�N
			SendNtnetDt.SData126.PayTime.Mon = PayData.TUnpaidPayTime.Mon;		// ���Z��
			SendNtnetDt.SData126.PayTime.Day = PayData.TUnpaidPayTime.Day;		// ���Z��
			SendNtnetDt.SData126.PayTime.Hour = PayData.TUnpaidPayTime.Hour;	// ���Z��
			SendNtnetDt.SData126.PayTime.Min = PayData.TUnpaidPayTime.Min;		// ���Z��
			SendNtnetDt.SData126.PayTime.Sec = 0;								// ���Z�b

		}
// MH810105(E) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�
	}
	SendNtnetDt.SData126.PayClass = payclass;						// �����敪

	SendNtnetDt.SData126.KakariNo = OPECTL.Kakari_Num;				// �W����

	SendNtnetDt.SData126.CoinSf[0].Money = 10;						// ��݋��ɋ���(10�~)
	SendNtnetDt.SData126.CoinSf[1].Money = 50;						// ��݋��ɋ���(50�~)
	SendNtnetDt.SData126.CoinSf[2].Money = 100;						// ��݋��ɋ���(100�~)
	SendNtnetDt.SData126.CoinSf[3].Money = 500;						// ��݋��ɋ���(500�~)
	if( payclass != 10 ){											// ��݋��ɍ��v�łȂ�
		SendNtnetDt.SData126.CoinSf[0].Mai = SFV_DAT.safe_dt[0];	// ��݋��ɖ���(10�~)
		SendNtnetDt.SData126.CoinSf[1].Mai = SFV_DAT.safe_dt[1];	// ��݋��ɖ���(50�~)
		SendNtnetDt.SData126.CoinSf[2].Mai = SFV_DAT.safe_dt[2];	// ��݋��ɖ���(100�~)
		SendNtnetDt.SData126.CoinSf[3].Mai = SFV_DAT.safe_dt[3];	// ��݋��ɖ���(500�~)
	}

	for( i=0; i<4; i++ ){
		SendNtnetDt.SData126.CoinSfTotal +=							// ��݋��ɑ��z
// MH322914 (s) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
//			SendNtnetDt.SData126.CoinSf[i].Mai * SendNtnetDt.SData126.CoinSf[i].Money;
			(ulong)SendNtnetDt.SData126.CoinSf[i].Mai * (ulong)SendNtnetDt.SData126.CoinSf[i].Money;
// MH322914 (e) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
	}

	SendNtnetDt.SData126.NoteSf[0].Money = 1000;					// �������ɋ���(1000�~)
	if( payclass != 11 ){											// �������ɍ��v�łȂ�
		SendNtnetDt.SData126.NoteSf[0].Mai = SFV_DAT.nt_safe_dt;	// �������ɖ���(1000�~)
	}
	SendNtnetDt.SData126.NoteSfTotal =								// �������ɑ��z
// MH322914 (s) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
//		SendNtnetDt.SData126.NoteSf[0].Mai * SendNtnetDt.SData126.NoteSf[0].Money;
		(ulong)SendNtnetDt.SData126.NoteSf[0].Mai * (ulong)SendNtnetDt.SData126.NoteSf[0].Money;
// MH322914 (e) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)

	// ���K�Ǘ��Ȃ����͋��ɏ��̂ݑ��M
	if( CPrmSS[S_KAN][1] ){											// ���K�Ǘ�����ݒ�
		switch( (uchar)payclass ){

		case 20:
		case 30:
		case 32:
			SendNtnetDt.SData126.Coin[0].Mai = turi_kan.turi_dat[0].sin_mai;	// ��ݏz����(10�~)
			SendNtnetDt.SData126.Coin[1].Mai = turi_kan.turi_dat[1].sin_mai;	// ��ݏz����(50�~)
			SendNtnetDt.SData126.Coin[2].Mai = turi_kan.turi_dat[2].sin_mai;	// ��ݏz����(100�~)
			SendNtnetDt.SData126.Coin[3].Mai = turi_kan.turi_dat[3].sin_mai;	// ��ݏz����(500�~)
			// NTNET_Snd_Data126()�̕���FT4000�ł͖��g�p�ׁ̈i�����Ή��j�C�����Ă����B
			turikan_subtube_set();
			for( i=0;i<2;i++ ){
				switch((( turi_kan.sub_tube >> (i*8)) & 0x000F )){
					case 0x01:
						pos = 0;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 10;								// ������z(10�~)
						break;
					case 0x02:
						pos = 1;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 50;								// ������z(50�~)
						break;
					case 0x04:
						pos = 2;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 100;								// ������z(100�~)
						break;
					case 0:
					default:																		// �ڑ��Ȃ�
						continue;
				}
				SendNtnetDt.SData126.CoinYotiku[i].Mai = turi_kan.turi_dat[pos].ysin_mai;			// ��ݗ\�~����(10�~/50�~/100�~)
			}
			break;

		default:
			// 10�~
			SendNtnetDt.SData126.Coin[0].Mai = turi_kan.turi_dat[0].zen_mai		// ��ݏz����(10�~)
									+ (ushort)turi_kan.turi_dat[0].sei_nyu;
			wk_mai = (ushort)(turi_kan.turi_dat[0].sei_syu + turi_kan.turi_dat[0].kyosei);
			if( SendNtnetDt.SData126.Coin[0].Mai < wk_mai ){
				SendNtnetDt.SData126.Coin[0].Mai = 0;
			}else{
				SendNtnetDt.SData126.Coin[0].Mai -= wk_mai;
			}
			// 50�~
			SendNtnetDt.SData126.Coin[1].Mai = turi_kan.turi_dat[1].zen_mai		// ��ݏz����(50�~)
									+ (ushort)turi_kan.turi_dat[1].sei_nyu;
			wk_mai = (ushort)(turi_kan.turi_dat[1].sei_syu + turi_kan.turi_dat[1].kyosei);
			if( SendNtnetDt.SData126.Coin[1].Mai < wk_mai ){
				SendNtnetDt.SData126.Coin[1].Mai = 0;
			}else{
				SendNtnetDt.SData126.Coin[1].Mai -= wk_mai;
			}
			// 100�~
			SendNtnetDt.SData126.Coin[2].Mai = turi_kan.turi_dat[2].zen_mai		// ��ݏz����(100�~)
									+ (ushort)turi_kan.turi_dat[2].sei_nyu;
			wk_mai = (ushort)(turi_kan.turi_dat[2].sei_syu + turi_kan.turi_dat[2].kyosei);
			if( SendNtnetDt.SData126.Coin[2].Mai < wk_mai ){
				SendNtnetDt.SData126.Coin[2].Mai = 0;
			}else{
				SendNtnetDt.SData126.Coin[2].Mai -= wk_mai;
			}
			// 500�~
			SendNtnetDt.SData126.Coin[3].Mai = turi_kan.turi_dat[3].zen_mai		// ��ݏz����(500�~)
									+ (ushort)turi_kan.turi_dat[3].sei_nyu;
			wk_mai = (ushort)(turi_kan.turi_dat[3].sei_syu + turi_kan.turi_dat[3].kyosei);
			if( SendNtnetDt.SData126.Coin[3].Mai < wk_mai ){
				SendNtnetDt.SData126.Coin[3].Mai = 0;
			}else{
				SendNtnetDt.SData126.Coin[3].Mai -= wk_mai;
			}

			// NTNET_Snd_Data126()�̕���FT4000�ł͖��g�p�ׁ̈i�����Ή��j�C�����Ă����B
			turikan_subtube_set();
			for( i=0;i<2;i++ ){
				switch((( turi_kan.sub_tube >> (i*8)) & 0x000F )){
					case 0x01:
						pos = 0;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 10;								// ������z(10�~)
						break;
					case 0x02:
						pos = 1;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 50;								// ������z(50�~)
						break;
					case 0x04:
						pos = 2;
						SendNtnetDt.SData126.CoinYotiku[i].Money = 100;								// ������z(100�~)
						break;
					case 0:
					default:																		// �ڑ��Ȃ�
						continue;
				}
				SendNtnetDt.SData126.CoinYotiku[i].Mai = turi_kan.turi_dat[pos].yzen_mai;			// ��ݗ\�~����(10�~/50�~/100�~)
				wk_mai = (ushort)(turi_kan.turi_dat[pos].ysei_syu + turi_kan.turi_dat[pos].ykyosei);
				if( SendNtnetDt.SData126.CoinYotiku[i].Mai < wk_mai ){
					SendNtnetDt.SData126.CoinYotiku[i].Mai = 0;
				}else{
					SendNtnetDt.SData126.CoinYotiku[i].Mai -= wk_mai;
				}
			}
			break;
		}
		SendNtnetDt.SData126.Coin[0].Money = 10;						// ��ݏz����(10�~)
		SendNtnetDt.SData126.Coin[1].Money = 50;						// ��ݏz����(50�~)
		SendNtnetDt.SData126.Coin[2].Money = 100;						// ��ݏz����(100�~)
		SendNtnetDt.SData126.Coin[3].Money = 500;						// ��ݏz����(500�~)
	}
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
	// ���O�쐬���ɔ�����s���Ă���̂ŁA�����ł͕s�v�ł��邪�c���Ă���
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
	/* ���Z���쎞�͋��K���ɕω��������ꍇ�͑��M���Ȃ� */
	f_Send = 1;
	if( payclass <= 5 ){											// ���Z����i����or���~�B�j
		if( 0 == memcmp( &SendNtnetDt.SData126.CoinSf[0], 			// ������񂪑O�񑗐M���e�Ɠ����i�d���� ��݋��Ɉȉ��j
						 &Ntnet_Prev_SData126.CoinSf[0],
						 (sizeof(DATA_KIND_126) - sizeof(DATA_BASIC) - 20) ) )
		{
			f_Send = 0;												// ���K�Ǘ��f�[�^�𑗐M���Ȃ�
		}
		//�N������0�̎��͑��M���Ȃ�(�ŏ��P�ʂƔ�r�Ƃ��Ă���)
		if( (SendNtnetDt.SData126.PayTime.Year < 1980) ||
			(SendNtnetDt.SData126.PayTime.Mon < 1) ||
			(SendNtnetDt.SData126.PayTime.Day < 1) ){
			f_Send = 0;												// ���K�Ǘ��f�[�^�𑗐M���Ȃ�
		}
	}

	if( f_Send ){													// ���K�Ǘ��f�[�^�𑗐M����
		memcpy( &Ntnet_Prev_SData126, &SendNtnetDt.SData126, sizeof(DATA_KIND_126) );
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_126 ));
		}
		else {
			NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_126 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
		}
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ������⍇���ް�(�ް����142)�쐬����                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data142                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-11-04                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data142( ulong pkno, ulong passid )
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_142 ) );

	BasicDataMake( 142, 1 );										// ��{�ް��쐬

	SendNtnetDt.SData142.ParkingNo = pkno;							// ���ԏꇂ
	SendNtnetDt.SData142.PassID = passid;							// �����ID

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_142 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_142 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| ���v�ް�(�ް����229)�쐬����                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data229                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data229( void )
{
	if (_is_ntnet_remote()) {
		return;
	}

	NTNET_Snd_Data229_Exec();
}

void	NTNET_Snd_Data229_Exec( void )
{
	struct  clk_rec wk_CLK_REC;
	ushort	wk_CLK_REC_msec;
	ulong	NowLifeTime;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_119 ) );

	BasicDataMake( 229, 1 );										// ��{�ް��쐬

	c_Now_CLK_REC_ms_Read( &wk_CLK_REC, &wk_CLK_REC_msec );			// ���ݎ���get
	NowLifeTime = LifeTim2msGet();									// ����LifeTime get

	SendNtnetDt.SData229.Year = wk_CLK_REC.year;					// Year
	SendNtnetDt.SData229.Mon  = wk_CLK_REC.mont;					// Month
	SendNtnetDt.SData229.Day  = wk_CLK_REC.date;					// Day
	SendNtnetDt.SData229.Hour = wk_CLK_REC.hour;					// Hour
	SendNtnetDt.SData229.Min  = wk_CLK_REC.minu;					// Minute
	SendNtnetDt.SData229.Sec  = wk_CLK_REC.seco;					// Second
	SendNtnetDt.SData229.MSec = wk_CLK_REC_msec;					// Millisecond
	//SendNtnetDt.SData229.HOSEI_MSec								// ���M���ɃZ�b�g����
	if( NowLifeTime == 0 ){											// �����O�X���Œ肷��
		NowLifeTime = 1;											// �i0�J�b�g����Ȃ��悤�Ɂj
	}
	SendNtnetDt.SData229.HOSEI_wk = NowLifeTime;					// �␳�l ��ƈ�

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_119 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_119 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �[������ް�(�ް����230)�쐬����                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data230                                       |*/
/*| PARAMETER    : MachineNo : ���M��[���@�B��                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data230( ulong MachineNo )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_230 ) );

	BasicDataMake( 230, 1 );										// ��{�ް��쐬

	SendNtnetDt.SData230.SMachineNo = MachineNo;					// ���M��[���@�B��
	memset( SendNtnetDt.SData230.ProgramVer, 0x20, 12 );			// CRM����ް�ޮ݊i�[�ر�ر
	memcpy( &SendNtnetDt.SData230.ProgramVer[4], VERSNO.ver_part, 8 );	// CRM����ް�ޮ݊i�[(�E�l��)

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_230 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_230 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �[������ް�(�ް����231)�쐬����                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data231                                       |*/
/*| PARAMETER    : MachineNo : ���M��[���@�B��                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data231( ulong MachineNo )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_231 ) );

	BasicDataMake( 231, 1 );										// ��{�ް��쐬

	SendNtnetDt.SData231.SMachineNo = MachineNo;					// ���M��[���@�B��
	if( OPECTL.Mnt_mod ){											// ����ݽӰ��
		SendNtnetDt.SData231.TerminalSt = 9;						// �[�����
	}else{
		switch( OPECTL.Ope_mod ){
		case 0:														// �ҋ@
		case 100:													// �x�Ə���
		case 110:													// �ޱ���װѕ\������
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		case 230:													// ���Z�ς݈ē�����
		case 255:													// �`�P�b�g���X�p�N���ҋ@
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		default:
			SendNtnetDt.SData231.TerminalSt = 0;					// �[�����
			break;
		case 1:														// ���Ԉʒu�ԍ����͏���
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//		case 11:													// �C�����Z���Ԉʒu�ԍ����͏���
//		case 70:													// �߽ܰ�ޓ��͏���(���Z��)
//		case 80:													// �߽ܰ�ޓo�^����(���Ɏ�)
//		case 90:													// ��t�����s����
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
			SendNtnetDt.SData231.TerminalSt = 1;					// �[�����
			break;
		case 2:														// �����\��,��������
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//		case 12:													// �C�����Z�����\��
//		case 13:													// �C�����Z��������
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
			SendNtnetDt.SData231.TerminalSt = 2;					// �[�����
			break;
		case 3:														// ���Z��������
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//		case 14:													// �C�����Z��������
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
			SendNtnetDt.SData231.TerminalSt = 3;					// �[�����
			break;
		}
	}
	if( opncls() == 2 ){											// �x��?
		SendNtnetDt.SData231.OpenClose = 1;							// �x��
	}
	SendNtnetDt.SData231.CardNGType = 0;							// NG���ޓǎ���e

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_231 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_231 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| leading zero suppress			                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : regist monitor with informations                        |*/
/*| PARAMETER    : code = monitor code                                     |*/
/*|              : error = error code                                      |*/
/*|              : param = optional data                                   |*/
/*|              : num = number of param digits                            |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.10.10                                              |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	regist_mon(ushort code, ushort error, ushort param, ushort num)
{
	uchar	info[10];
	memset(info, 0, sizeof(info));
	if (error == 0) {
		info[0] =
		info[1] =
		info[2] = ' ';
	}
	else {
		intoas(info, error, 3);
	}
	if (num) {
		intoas(&info[3], param, num);
	}
	wmonlg(code, info, 0);
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ�A�h���X�ϊ�				                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : get_param_addr                                          |*/
/*| PARAMETER    : kind = 0 as all, 1 as r/w                               |*/
/*|              : addr = specified address                                |*/
/*|              : *seg = internal segment address                         |*/
/*|              : *inaddr = internal address                              |*/
/*| RETURN VALUE : 1 as OK                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.9.29                                               |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
	struct _paraaddr_ {
		ushort	topaddr;
		ushort	endaddr;
		ushort	seg;
		ushort	inaddr;
	} addr_tbl[] = {
		{412, 413, 6, 3},	// A�Ԏ�j�����̗������
		{414, 415, 6, 9},	// B�Ԏ�j�����̗������
		{416, 417, 6,15},	// C�Ԏ�j�����̗������
		{418, 419, 6,21},	// D�Ԏ�j�����̗������
		{420, 421, 6,27},	// E�Ԏ�j�����̗������
		{422, 423, 6,33},	// F�Ԏ�j�����̗������
		{424, 425, 6,39},	// G�Ԏ�j�����̗������
		{426, 427, 6,45},	// H�Ԏ�j�����̗������
		{428, 429, 6,51},	// I�Ԏ�j�����̗������
		{430, 431, 6,57},	// J�Ԏ�j�����̗������
		{432, 433, 6,63},	// K�Ԏ�j�����̗������
		{434, 435, 6,69},	// L�Ԏ�j�����̗������
		{436, 438, 6, 4},
		{472, 472, 39, 105},	// �@�l�J�[�h�g�p��/�s�ݒ�
		{490, 495, 4, 1},	// ���ʊ��ԂP�`�R
		{551, 551, 28, 2},

		{558, 559, 28, 13},
		{602, 618, 31, 3},			{660, 899, 31, 61},
		{902, 918, 31, 303},		{960, 1199, 31, 361},
		{1202, 1218, 31, 603},		{1260, 1499, 31, 661},
		{1502,1518, 58, 3},			{1560, 1799, 58, 61},		// ��S�����̌n�����ݒ�
		{1802,1818, 58, 303},		{1860, 2099, 58, 361},		// ��T�����̌n�����ݒ�
		{2102,2118, 58, 603},		{2160, 2399, 58, 661},		// ��U�����̌n�����ݒ�
		{2402,2418, 59, 3},			{2460, 2699, 59, 61},		// ��V�����̌n�����ݒ�
		{2702,2718, 59, 303},		{2760, 2999, 59, 361},		// ��W�����̌n�����ݒ�
		{3002,3018, 59, 603},		{3060, 3299, 59, 661},		// ��X�����̌n�����ݒ�
		{5001,5005, 56, 1},			// ��ʖ��ő嗿���ݒ�
		{5020, 5031, 56, 20},		// �����ԍő�P�ݒ�(�����v�Z�g��)
		{5032, 5043, 56, 32},		// �����ԍő�Q�ݒ�
		{5101, 5101, 28, 41},		// �����ԍő�ݒ�(�����v�Z�g������)
		{5201, 5201, 39, 14},		// �O�F�O�O�������ݒ�
		{5301, 5340, 29,  1},		// �T�[�r�X�^�C���ݒ�i�O���[�X�^�C���E���O�^�C���j
		{5351, 5352,  1, 40},		// �����w��̉c�x�Ɛ؊��̉c�ƊJ�n�E�I������
		{5361, 5363,  2, 29},		// �o�b�N���C�g�ݒ�i�������@�E�_�����ԁj
		{0, 0, 0, 0}
	};

static	int		get_param_addr(ushort kind, ushort addr, ushort *seg, ushort *inaddr)
{
	static	const

	struct _paraaddr_	*p;
	p = addr_tbl;
	while(p->topaddr) {
		if (addr >= p->topaddr && addr <= p->endaddr) {
			*seg = p->seg;
			*inaddr = (addr - p->topaddr) + p->inaddr;
			return 1;
		}
		p++;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ�A�h���X�`�F�b�N			                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : check_remote_addr                                       |*/
/*| PARAMETER    : kind = 0 as all, 1 as r/w                               |*/
/*|              : addr = specified address                                |*/
/*|              : count = data count                                      |*/
/*| RETURN VALUE : 1 as OK                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.9.29                                               |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int		check_remote_addr(ushort kind, ushort addr, ushort count)
{
	ushort	seg, inaddr;
	if (count == 0)
		return 0;
	if (count >= 19)		//�ő�ݒ�f�[�^���F18
		return 0;
	while(count) {
		if (!get_param_addr(kind, addr, &seg, &inaddr))
			return 0;
		addr++;
		count--;
	}
	return 1;
}

/*[]----------------------------------------------------------------------[]*/
/*| 10�i�޲�ذ�̎w�茅���O�ɂ���                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : param6_mask_zero         							   |*/
/*| PARAMETER    : indat(in) ���f�[�^                                      |*/
/*| PARAMETER    : maskptn(in) �}�X�N�p�^�[��(16�i.ex> 0x00000F=1�̈ʂ�0   |*/
/*| RETURN VALUE : MASK���ް�											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : COSMO_OKAMOTO                                           |*/
/*| Date         : 2008.6.30                                               |*/
/*| UpDate       :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
long param6_mask_zero ( long indat, long maskptn )
{
	unsigned char	cnt;
	unsigned char	lkt[6];
	long			ldat,lmsk;

	ldat = indat;
	lmsk = 0x0f;
	for( cnt = 0; cnt < 6; cnt ++){
		if( maskptn & lmsk ){
			lkt[cnt] = 0;
		}else{
			lkt[cnt] = (unsigned char)(ldat % 10L);
		}
		ldat /= 10L;
		lmsk <<= 4;
	}
	return (long)( 	(long)lkt[5]*100000L + 
					(long)lkt[4]*10000L + 
					(long)lkt[3]*1000L + 
					(long)lkt[2]*100L + 
					(long)lkt[1]*10L + 
					(long)lkt[0] );
}
//10�i�̂P���̐��l�����o���}�N�� EX.> MC_UPPARA_P10( 123456 ) ==> 5 //
#define		MC_UDPARA_P1000(val) ((val % 10000L) / 1000L)
#define		MC_UDPARA_P100(val) ((val % 1000L) / 100L)
#define		MC_UDPARA_P10(val) ((val % 100L) / 10L)
#define		MC_UDPARA_P1(val) (val % 10L)
#define		MAX_TYPE_SET_S	5002		// �ő嗿���^�C�v�ݒ�A�h���X(�J�n)
#define		MAX_TYPE_SET_E	5005		// �ő嗿���^�C�v�ݒ�A�h���X(�I��)
#define		FT4500_MX_BAND		8		// FT4500���ԑэő�ݒ�l
#define		FT4500_MX_N_HOUR	7		// FT4500�����Ԃ���ő�ݒ�l
#define		GETA_10_4	10000L			// �p�����[�^�Z�o�p
#define		GETA_10_2	100L			// �p�����[�^�Z�o�p
/*[]----------------------------------------------------------------------[]*/
/*| �����ݒ胍�O(���u�����ݒ�ύX�p)                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ryokin_Settei_Log                                       |*/
/*| PARAMETER    : addr   :		�A�h���X(FT4500�݊�)                       |*/
/*|              : data	  :		�ݒ�l                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2010.05.20                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	Ryokin_Settei_Log(ushort addr, long data)
{
	memcpy( &remote_Change_data.rcv_Time, &CLK_REC, sizeof( date_time_rec ) );		// ���ݎ���
	remote_Change_data.addr = addr;													// �ݒ�A�h���X
	remote_Change_data.rcv_data = data;												// �ݒ�f�[�^

	Log_Write(eLOG_REMOTE_SET, &remote_Change_data, TRUE);							// ���u�����ݒ胍�O
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ�f�[�^�X�V					                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : update_param                                            |*/
/*| PARAMETER    : none                                                    |*/
/*| RETURN VALUE : 1 as OK                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.9.29                                               |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int		update_param(void)
{
	int		blks;
	struct _setdata_	*p;
	ushort	addr, i;
	ushort	seg, inaddr;
	long	lpwk;
	long	lpwk2;

	blks = tmp_buffer.bufcnt;
	p = (struct _setdata_*)tmp_buffer.buffer;
	while(blks) {
		addr = p->addr;
		for (i = 0; i < p->count; i++) {
			get_param_addr(1, addr, &seg, &inaddr);
			Ryokin_Settei_Log(addr, p->data[i]);									// ���u�ݒ胍�O

			switch( addr ){
			case 551:
				if ( p->data[i] == 7L ){
					CPrmSS[seg][inaddr] = 10L;
				}
				else if ( p->data[i] == 8L ){
					CPrmSS[seg][inaddr] = 5L;
				}else{
					CPrmSS[seg][inaddr] = p->data[i];
				}
				break;
			case 558:
					lpwk = ((prm_get( COM_PRM, S_CAL, 38, 2, 5 )  * 10000L) + 
							( p->data[i] ) +
							( prm_get( COM_PRM, S_CAL, 38, 2, 1 ) ));
					CPrmSS[S_CAL][38] = lpwk;
				break;

			case 559:
					lpwk = ((prm_get( COM_PRM, S_CAL, 38, 2, 5 )  * 10000L) + 
							( prm_get( COM_PRM, S_CAL, 38, 2, 3 ) * 100L) +
							( p->data[i] % 100L ));
					CPrmSS[S_CAL][38] = lpwk;
				break;
			case 412:
			case 414:	// B��
			case 416:	// C��
			case 418:	// D��
			case 420:	// E��
			case 422:	// F��
			case 424:	// G��
			case 426:	// H��
			case 428:	// I��
			case 430:	// J��
			case 432:	// K��
			case 434:	// L��
				// [�Q���ΐ��؁Q]
				CPrmSS[S_SHA][(3+((addr%412)*3))] = param6_mask_zero ( CPrmSS[S_SHA][(3+((addr%412)*3))], 0x0ffff0 ) + ((p->data[i] % 10000L) * 10L);
				break;
			case 413:
			case 415:	// B��
			case 417:	// C��
			case 419:	// D��
			case 421:	// E��
			case 423:	// F��
			case 425:	// G��
			case 427:	// H��
			case 429:	// I��
			case 431:	// J��
			case 433:	// K��
			case 435:	// L��
				lpwk = param6_mask_zero ( CPrmSS[S_SHA][(3+((addr%413)*3))], 0xf0000f );
				lpwk += MC_UDPARA_P1000(p->data[i]);	 		// =>[�|�|�|�|�|��]
				lpwk += MC_UDPARA_P10  (p->data[i]) * 100000L;	// =>[���|�|�|�|�|]
				CPrmSS[S_SHA][(3+((addr%413)*3))] = lpwk;

				lpwk = param6_mask_zero ( CPrmSS[S_SHA][(4+((addr%413)*3))], 0xff0000 );
				lpwk += MC_UDPARA_P100(p->data[i]) * 100000L;	// =>[�y�|�|�|�|�|]
				lpwk += MC_UDPARA_P1  (p->data[i]) * 10000L;	// =>[�|���|�|�|�|]
				CPrmSS[S_SHA][(4+((addr%413)*3))] = lpwk;
				break;
			case 436:
				//���ʊ��� A =>[..O...]
				CPrmSS[S_SHA][4] = param6_mask_zero ( CPrmSS[S_SHA][4],  0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1000 (p->data[i])));
				//���ʊ��� B =>[..O...]
				CPrmSS[S_SHA][10] = param6_mask_zero( CPrmSS[S_SHA][10], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P100 (p->data[i])));
				//���ʊ��� C =>[..O...]
				CPrmSS[S_SHA][16] = param6_mask_zero( CPrmSS[S_SHA][16], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P10 (p->data[i])));
				//���ʊ��� D =>[..O...]
				CPrmSS[S_SHA][22] = param6_mask_zero( CPrmSS[S_SHA][22], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1 (p->data[i])));
				break;
			case 437:
				//���ʊ��� E =>[..O...]
				CPrmSS[S_SHA][28] = param6_mask_zero( CPrmSS[S_SHA][28], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1000 (p->data[i])));
				//���ʊ��� F =>[..O...]
				CPrmSS[S_SHA][34] = param6_mask_zero( CPrmSS[S_SHA][34], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P100 (p->data[i])));
				//���ʊ��� G =>[..O...]
				CPrmSS[S_SHA][40] = param6_mask_zero( CPrmSS[S_SHA][40], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P10 (p->data[i])));
				//���ʊ��� H =>[..O...]                                               
				CPrmSS[S_SHA][46] = param6_mask_zero( CPrmSS[S_SHA][46], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1 (p->data[i])));
				break;
			case 438:
				//���ʊ��� I =>[..O...]
				CPrmSS[S_SHA][52] = param6_mask_zero( CPrmSS[S_SHA][52], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1000 (p->data[i])));
				//���ʊ��� J =>[..O...]
				CPrmSS[S_SHA][58] = param6_mask_zero( CPrmSS[S_SHA][58], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P100 (p->data[i])));
				//���ʊ��� K =>[..O...]
				CPrmSS[S_SHA][64] = param6_mask_zero( CPrmSS[S_SHA][64], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P10 (p->data[i])));
				//���ʊ��� L =>[..O...]
				CPrmSS[S_SHA][70] = param6_mask_zero( CPrmSS[S_SHA][70], 0x00ffff ) + (SP_RANGE_DEF(MC_UDPARA_P1 (p->data[i])));
				break;
			case 472:
				CPrmSS[seg][inaddr] = ((p->data[i]/1000L)%10L);						// 1000�̌������̂܂܃Z�b�g����
				break;				
			case 5361:
				lpwk = ((prm_get(COM_PRM, (short)seg, (short)inaddr, 1, 2) * 10L) + (p->data[i]%10L));	// �e���L�[�ݒ�{1�̌��̂݃Z�b�g����
				CPrmSS[seg][inaddr] = lpwk;											
				break;				
			default:
				if( addr >= MAX_TYPE_SET_S && addr <= MAX_TYPE_SET_E){				// ��ʖ��̍ő嗿���^�C�v�̐ݒ� 
				/* �ő嗿���^�C�v��FT4500�p�̐ݒ�l�ő����Ă���̂ŕϊ�����(7 �� 10 8 �� 5) */
				/* �ЂƂ̃A�h���X�ɂR�̐ݒ肪�����Ă��邽�߂Q�������o���� */
					lpwk = p->data[i]/GETA_10_4;					// �@�A�𒊏o
					if(lpwk == FT4500_MX_N_HOUR){					// �����Ԃ���ő�
						lpwk2 = SP_MX_N_MH_NEW*GETA_10_4;			// FT4800�ł͂P�O
					}
					else if(lpwk == FT4500_MX_BAND){				// ���ԑэő�
						lpwk2 = SP_MX_BAND*GETA_10_4;				// FT4800�ł͂T
					}
					else {
						lpwk2 = lpwk*GETA_10_4;
					}
					lpwk = (p->data[i]%GETA_10_4)/GETA_10_2;		// �B�C�𒊏o
					if(lpwk == FT4500_MX_N_HOUR){
						lpwk2 += SP_MX_N_MH_NEW*GETA_10_2;
					}
					else if(lpwk == FT4500_MX_BAND){
						lpwk2 += SP_MX_BAND*GETA_10_2;
					}
					else {
						lpwk2 += lpwk*GETA_10_2;
					}
					lpwk = p->data[i]%GETA_10_2;						// �D�E�𒊏o
					if(lpwk == FT4500_MX_N_HOUR){
						lpwk2 += SP_MX_N_MH_NEW;
					}
					else if(lpwk == FT4500_MX_BAND){
						lpwk2 += SP_MX_BAND;
					}
					else {
						lpwk2 += lpwk;
					}
					CPrmSS[seg][inaddr] = lpwk2;
					break;
				}
				CPrmSS[seg][inaddr] = p->data[i];
				break;
			}
			addr++;
		}
		p = (struct _setdata_*)&p->data[p->count];
		blks--;
	}
	return 1;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�@��ԃ`�F�b�N				                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : check_acceptable                                        |*/
/*| PARAMETER    : none                                                    |*/
/*| RETURN VALUE : 1 as OK                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.11.06                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
int		check_acceptable(void)
{
	if (OPECTL.Mnt_mod == 0 &&
		(OPECTL.Ope_mod == 0				// �ҋ@
		 || OPECTL.Ope_mod == 1				// ���Ԍ��҂�
		 || OPECTL.Ope_mod == 100))			// �x��?
		return 1;
	return 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ菀���N���A					                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_ClrSetup                                          |*/
/*| PARAMETER    : none                                                    |*/
/*| RETURN VALUE : none                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.11.06                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_ClrSetup(void)
{
	tmp_buffer.prepare = 0;					// cancel prepare command
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ʐݒ��ް�(�ް����80)��M����                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData80                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData80( void )
{
	RP_DATA_KIND_80	*msg = &RecvNtnetDt.RP_RData80;
	ushort	moncode;
	
	uchar	okng;
	int		size;

	size = (2+2) + msg->DataCount * (int)(sizeof(long));
	okng = 1;
	if (msg->SMachineNo != (ulong)CPrmSS[S_PAY][2]) {
		okng = _RPKERR_INVALID_MACHINENO;
	}
// prepare command is received?
	else if (tmp_buffer.prepare == 0) {
		okng = _RPKERR_NO_PREPARE_COMMAND;
	}
// specified address is valid?
	else if (msg->Segment != 0 ||
			check_remote_addr(1, msg->TopAddr, msg->DataCount) == 0) {
		okng = _RPKERR_INVALID_PARA_ADDR;
	}
// data is bufferable?
	else if ((tmp_buffer.bufofs + size) >= sizeof(tmp_buffer.buffer)) {
		okng = _RPKERR_NO_PREPARE_COMMAND;		// overflow
	}
	else if (! check_acceptable()) {
		okng = _RPKERR_COMMAND_REJECT;
	}
	else {
// all OK! now save the data

		memcpy(&tmp_buffer.buffer[tmp_buffer.bufofs], &msg->TopAddr, (size_t)size);
		tmp_buffer.bufofs += size;
		tmp_buffer.bufcnt++;
	}
// answer
	if (msg->Status == 1) {
		NTNET_Ans_Data99(msg->DataBasic.SystemID, msg->DataBasic.MachineNo,
								NTNET_COMPLETE_CPRM, okng);
	}
// regist monitor
	moncode = OPMON_RSETUP_RCVOK;
	if (okng == 1) {
		okng = 0;
	}
	else {
		moncode++;
		NTNET_ClrSetup();					// cancel prepare command if error
	}
	regist_mon(moncode, (ushort)okng, msg->TopAddr, 6);
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ʓ��ݒ�f�[�^(�ް����83)��M����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData83                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2006-09-27                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData83( void )
{
	ushort 	i;
	ulong 	date1, date2, date3, shift;

	f_NTNET_RCV_SPLDATE = 1;					// ���ʓ��ݒ�f�[�^��M�ɂ�鋤�����Ұ��X�V��

	// ���ʓ��ݒ��M�ް����������Ұ�TBL�ɔ��f����B
	for( i=0; i<3; i++ ){												// ���ʊ��ԂP�`�R
		date1 = (ulong)RecvNtnetDt.RData83.kikan[i].sta_Mont;
		date1 *= 100;
		date1 += (ulong)RecvNtnetDt.RData83.kikan[i].sta_Date;
		CPrmSS[S_TOK][i*2+1] = date1;									// �J�n����

		date1 = (ulong)RecvNtnetDt.RData83.kikan[i].end_Mont;
		date1 *= 100;
		date1 += (ulong)RecvNtnetDt.RData83.kikan[i].end_Date;
		CPrmSS[S_TOK][i*2+2] = date1;									// �I������
	}

	for( i=0; i<31; i++ ){												// ���ʓ����V�t�g�P�`�R�P
		date1  = (ulong)RecvNtnetDt.RData83.date[i].mont;
		date1 *= 100;
		date1 += (ulong)RecvNtnetDt.RData83.date[i].day;
		shift = (ulong)RecvNtnetDt.RData83.Shift[i];

		CPrmSS[S_TOK][NTNET_SPLDAY_START+i] = shift*100000 + date1;		// �V�t�g�{����
	}

	if (CPrmSS[S_TOK][41]) {
		for( i=0; i<12; i++ ){											// ���ʗj���P�`�P�Q
			date1 = (ulong)RecvNtnetDt.RData83.yobi[i].mont;			// ��
			date2 = (ulong)RecvNtnetDt.RData83.yobi[i].week;			// �T
			date3 = (ulong)RecvNtnetDt.RData83.yobi[i].yobi;			// �j��

			CPrmSS[S_TOK][NTNET_SPYOBI_START+i] = date1*100 + date2*10 +date3;
		}
	}
	else {
		for( i=0; i<6; i++ ){											// �n�b�s�[�}���f�[�P�`�P�Q
			date1 = 0;
			date2 = 0;
			if(chk_hpmonday(RecvNtnetDt.RData83.yobi[i*2])) {			// �n�b�s�[�}���f�[�Ƀ}�b�`���邩�i�O�j
				date1 = (ulong)RecvNtnetDt.RData83.yobi[i*2].mont;		// ��
				date1 *= 10;
				date1 += (ulong)RecvNtnetDt.RData83.yobi[i*2].week;		// �T
			}
			if(chk_hpmonday(RecvNtnetDt.RData83.yobi[i*2+1])) {			// �n�b�s�[�}���f�[�Ƀ}�b�`���邩�i��j
				date2 = (ulong)RecvNtnetDt.RData83.yobi[i*2+1].mont;	// ��
				date2 *= 10;
				date2 += RecvNtnetDt.RData83.yobi[i*2+1].week;			// �T
			}

			CPrmSS[S_TOK][NTNET_HMON_START+i] = date1*1000 + date2;		// �n�b�s�[�}���f�[�i�O�{��j
		}
	}
	
	CPrmSS[S_TOK][NTNET_5_6] = ((RecvNtnetDt.RData83.spl56 & 0x01 ) ^ 0x01);		// 5�^6����ʓ��Ƃ���
																		// spl56��0�̏ꍇ��1�Ƃ��Aspl56��1�̏ꍇ��0�Ƃ���
	for( i=0 ; i<6; i++ ){												// ���ʔN�����P�`�U
		date1 = (ulong)RecvNtnetDt.RData83.year[i].year;				// �N
		if( date1 >= 2000 ){
			date1 -= 2000;
		}
		date2 = (ulong)RecvNtnetDt.RData83.year[i].mont;				// ��
		date3 = (ulong)RecvNtnetDt.RData83.year[i].day;					// ��

		CPrmSS[S_TOK][NTNET_SPYEAR_START+i] = date1*10000 + date2*100 + date3;
	}

	DataSumUpdate(OPE_DTNUM_COMPARA);									// ���ʐݒ��ް��т��X�V
	FLT_WriteParam1(FLT_NOT_EXCLUSIVE);									// FlashROM update
	SetSetDiff(SETDIFFLOG_SYU_REMSET);									// �f�t�H���g�Z�b�g���O�o�^����

	f_NTNET_RCV_MC10_EXEC = 1;											// mc10()�X�V���K�v�Ȑݒ肪�ʐM�ōX�V���ꂽ���set
	wopelg(OPLOG_SET_TOKUBETU_UPDATE, 0, 0);							// ���엚��o�^

	NTNET_Snd_Data99(RecvNtnetDt.RData83.DataBasic.MachineNo, NTNET_COMPLETE_SPDAY);	// NT-NET�ݒ��M�����f�[�^�쐬
}

/*[]----------------------------------------------------------------------[]*/
/*| ��������Z���~�f�[�^�e�[�u��(�ް����91)��M����                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData91                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData91( void )
{
	t_TKI_CYUSI *work = &z_Work.TeikiCyusi;	// ��������~�f�[�^�ҏW�p���[�N
	TEIKI_CHUSI *ntdata;
	short idx,i;
	
	f_NTNET_RCV_TEIKITHUSI = 1;

	switch (RecvNtnetDt.RData91.ProcMode) {
	case 0:		// 1���X�V
		ntdata = &RecvNtnetDt.RData91.TeikiChusi[0];
		// �����ΏۂƂ���f�[�^������
		idx = ntnet_SearchTeikiCyusiData(ntdata->ParkingNo, ntdata->PassID);
		// ��d�ۏؗp���[�N�G���A�Ƀf�[�^�쐬
		memcpy(work, &tki_cyusi, sizeof(t_TKI_CYUSI));
		if (idx >= 0) {
			// �Y���f�[�^���� �� ���̃f�[�^���㏑��
			if (ntnet_TeikiCyusiCnvNtToCrm(&work->dt[_TKI_Idx2Ofs(idx)], &RecvNtnetDt.RData91.TeikiChusi[0])) {
				// ���[�N�G���A������̈�֏�������
				nmisave(&tki_cyusi, work, sizeof(t_TKI_CYUSI));
				wopelg(OPLOG_SET_TEIKI_TYUSI, 0, 0);
			}
		}
		else {
			// �Y���f�[�^�Ȃ� �� �Ō���ɒǉ�
			if (ntnet_TeikiCyusiCnvNtToCrm(&work->dt[work->wtp], &RecvNtnetDt.RData91.TeikiChusi[0])) {
				if (work->count < TKI_CYUSI_MAX) {
					work->count++;
				}
				if (++work->wtp >= TKI_CYUSI_MAX) {
					work->wtp = 0;
				}
				// ���[�N�G���A������̈�֏�������
				nmisave(&tki_cyusi, work, sizeof(t_TKI_CYUSI));
				wopelg(OPLOG_SET_TEIKI_TYUSI, 0, 0);
			}
		}
		break;
	case 1:		// 1���폜
		ntdata = &RecvNtnetDt.RData91.TeikiChusi[0];
		idx = ntnet_SearchTeikiCyusiData(ntdata->ParkingNo, ntdata->PassID);
		if (idx >= 0) {
			TKI_Delete(idx);
			wopelg(OPLOG_SET_TEIKI_TYUSI, 0, 0);
		}
		break;
	case 2:		// �S�f�[�^�X�V
		// ��d�ۏؗp���[�N�G���A�Ƀf�[�^�쐬
		memset(work, 0, sizeof(t_TKI_CYUSI));
		i = 0;
		while (work->count < TKI_CYUSI_MAX) {
			if (RecvNtnetDt.RData91.TeikiChusi[i].ParkingNo == 0) {
				// �ȍ~�A�L���f�[�^�Ȃ�
				break;
			}
			if (ntnet_TeikiCyusiCnvNtToCrm(&work->dt[work->count], &RecvNtnetDt.RData91.TeikiChusi[i]) == FALSE) {
				i++;
				continue;
			}
			i++;
			work->count++;
		}
		if (work->count < TKI_CYUSI_MAX) {
			work->wtp = work->count;
		} else {
			work->wtp = 0;
		}
		// ���[�N�G���A������̈�֏�������
		nmisave(&tki_cyusi, work, sizeof(t_TKI_CYUSI));
		wopelg(OPLOG_SET_TEIKI_TYUSI, 0, 0);
		break;
	default:
		break;
	}
	NTNET_Snd_Data99(RecvNtnetDt.RData91.DataBasic.MachineNo, NTNET_COMPLETE_TEIKI_TYUSHI);		/* �ݒ��M�����ʒm�ް����M */
}

/*[]----------------------------------------------------------------------[]*/
/*| ������ð��ð���(�ް����93)��M����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData93                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData93( void )
{
	/* ������ð��ð��ٍX�V */
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	memcpy(pas_tbl, RecvNtnetDt.RData93.PassTable, sizeof(pas_tbl));
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	wopelg(OPLOG_SET_STATUS_TBL_UPDATE, 0, 0);		// ���엚��o�^
	
	NTNET_Snd_Data99(RecvNtnetDt.RData93.DataBasic.MachineNo, NTNET_COMPLETE_TEIKI_STS);		/* �ݒ��M�����ʒm�ް����M */
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�Vð���(�ް����94)��M����                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData94                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData94( void )
{
	/* ������X�Vð��ٍX�V */
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	memcpy(pas_renewal, RecvNtnetDt.RData94.PassRenewal, sizeof(pas_renewal));
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	NTNET_Snd_Data99(RecvNtnetDt.RData94.DataBasic.MachineNo, NTNET_COMPLETE_TEIKI_UPDATE);		/* �ݒ��M�����ʒm�ް����M */
}

/*[]----------------------------------------------------------------------[]*/
/*| ������o�Ɏ���ð���(�ް����95)��M����                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData95                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData95( void )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	short	i;
//
//	f_NTNET_RCV_TEIKIEXTIM = 1;
//
//	/* ������o�Ɏ���ð��ٍX�V */
//	for (i = 0; i < PASS_EXTIMTBL_MAX; i++) {
//		if (RecvNtnetDt.RData95.PassExTable[i].PassId == 0)
//			break;
//		pas_extimtbl.PassExTbl[i] = RecvNtnetDt.RData95.PassExTable[i];
//	}
//	pas_extimtbl.Count = i;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	
	NTNET_Snd_Data99(RecvNtnetDt.RData95.DataBasic.MachineNo, NTNET_COMPLETE_TEIKI_CAROUT);		/* �ݒ��M�����ʒm�ް����M */
}

/*[]----------------------------------------------------------------------[]*
 *| �Ԏ��p�����[�^��M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData97
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2005-11-30
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData97( void )
{

	if (RecvNtnetDt.RData97.ModelCode != NTNET_MODEL_CODE) {
		return;
	}
	
	if (OPESETUP_SetupDataChk(97, 0)) {
		NtNet_FukudenParam.DataKind	= 97;
		NtNet_FukudenParam.Src		= &RecvNtnetDt.RData97;
		NTNET_UpdateParam(&NtNet_FukudenParam);
		wopelg(OPLOG_SET_SYASITUPARAM, 0, 0);
	}
	
	NTNET_Snd_Data99(RecvNtnetDt.RData97.DataBasic.MachineNo, NTNET_COMPLETE_SHASHITSU_PRM);	// NT-NET�ݒ��M�����f�[�^�쐬
}

/*[]----------------------------------------------------------------------[]*/
/*| ۯ����u���Ұ��ݒ�(�ް����98)��M����                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData98                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART machida                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData98( void )
{

	if (RecvNtnetDt.RData98.ModelCode != NTNET_MODEL_CODE) {
		return;
	}
	if (OPESETUP_SetupDataChk(98, 0)) {
		NtNet_FukudenParam.DataKind	= 98;
		NtNet_FukudenParam.Src		= &RecvNtnetDt.RData98;
		NTNET_UpdateParam(&NtNet_FukudenParam);
		wopelg(OPLOG_SET_LOCKPARAM, 0, 0);
	}
	
	NTNET_Snd_Data99(RecvNtnetDt.RData98.DataBasic.MachineNo, NTNET_COMPLETE_LOCK_PRM);	// NT-NET�ݒ��M�����f�[�^�쐬
}

/*[]----------------------------------------------------------------------[]*/
/*| �ް��v��2����NG(�ް����110)��M����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData110                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData110( void )
{
	if( (RecvNtnetDt.RData110.ControlData[3] == 1) || 
		(RecvNtnetDt.RData110.ControlData[4] == 1) ||
		(RecvNtnetDt.RData110.ControlData[4] == 2) ){
		NTNetTotalEndError = RecvNtnetDt.RData110.Result;
		if (NTNetTotalEndFlag == 0) {
			NTNetTotalEndFlag = -1;		// �����W�v�f�[�^��M���s
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ����f�[�^�X�V(�ް����116)��M����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData116                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData116( void )
{
	ushort status;
//	ushort use_parking;
	ulong use_parking;
	ushort valid;
	date_time_rec exit_time;
	uchar parking_no_check=0;

	status = 0xFFFF;
	use_parking = 0xFFFFFFFF;
	valid = 0xFFFF;

	parking_no_check = NTNET_GetParkingKind( RecvNtnetDt.RData116.ParkingNo, PKOFS_SEARCH_LOCAL );

	if (RecvNtnetDt.RData116.Status <= NTNET_PASSUPDATE_INVALID) {
		// �P���ڂ��̐ݒ�
		if ((RecvNtnetDt.RData116.ProcMode & NTNET_PASSUPDATE_STATUS) == NTNET_PASSUPDATE_STATUS) {
			// ����X�e�[�^�X�e�[�u���X�V
			if (RecvNtnetDt.RData116.ParkingNo != 0 && RecvNtnetDt.RData116.PassID != 0) {
				if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_INVALID) {
					if( parking_no_check != 0xFF ){
						if(RecvNtnetDt.RData116.ParkingNo < 1000 ){			//APS
							wopelg(OPLOG_SET_TEIKI_YUKOMUKO, 0, Convert_PassInfo(RecvNtnetDt.RData116.ParkingNo,RecvNtnetDt.RData116.PassID));		// ���엚��o�^
						}else{												//GT
							wopelg(OPLOG_GT_SET_TEIKI_YUKOMUKO, RecvNtnetDt.RData116.ParkingNo,RecvNtnetDt.RData116.PassID);		// ���엚��o�^
						}
					}
					valid = 1;
				} else {
					if( parking_no_check != 0xFF ){
						if(RecvNtnetDt.RData116.ParkingNo < 1000 ){			//APS
							wopelg(OPLOG_SET_STATUS_CHENGE, 0, Convert_PassInfo(RecvNtnetDt.RData116.ParkingNo,RecvNtnetDt.RData116.PassID));		// ���엚��o�^
						}else{												//GT
							wopelg(OPLOG_GT_SET_STATUS_CHENGE, RecvNtnetDt.RData116.ParkingNo,RecvNtnetDt.RData116.PassID);		// ���엚��o�^
						}
					}
					valid = 0;
					status = RecvNtnetDt.RData116.Status;
				}
				if (RecvNtnetDt.RData116.UsingParkingNo != 0) {
					use_parking = (ulong)RecvNtnetDt.RData116.UsingParkingNo;
				}

				WritePassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, (ushort)RecvNtnetDt.RData116.PassID, status, use_parking, valid );
			}
		}

		if ((RecvNtnetDt.RData116.ProcMode & NTNET_PASSUPDATE_UPDATESTATUS) == NTNET_PASSUPDATE_UPDATESTATUS) {
			// ����X�V�X�e�[�^�X�e�[�u���X�V
			if (RecvNtnetDt.RData116.UpdateStatus == 0 || RecvNtnetDt.RData116.UpdateStatus == 1) {
				if (RecvNtnetDt.RData116.ParkingNo != 0 && RecvNtnetDt.RData116.PassID != 0) {
				}
			} else if (RecvNtnetDt.RData116.UpdateStatus == 2 || RecvNtnetDt.RData116.UpdateStatus == 3) {
				if (RecvNtnetDt.RData116.ParkingNo != 0) {
				}
			}
		}

		if ((RecvNtnetDt.RData116.ProcMode & NTNET_PASSUPDATE_OUTTIME) == NTNET_PASSUPDATE_OUTTIME) {
			// �o�Ɏ����e�[�u���X�V
			if ((ushort)RecvNtnetDt.RData116.ParkingNo  == 0 && (ushort)RecvNtnetDt.RData116.PassID == 0){
				if (RecvNtnetDt.RData116.OutYear == 0
				&& RecvNtnetDt.RData116.OutMonth == 0
				&& RecvNtnetDt.RData116.OutDay == 0
				&& RecvNtnetDt.RData116.OutHour == 0
				&& RecvNtnetDt.RData116.OutMin == 0) {
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//						// ���ԏ�ԍ��@����ԍ��@�o�Ɏ����@�I�[���@�O�@�̂��ߏo�Ɏ����e�[�u���S�폜�B
//						memset(&pas_extimtbl, 0, sizeof(PASS_EXTIMTBL));
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
				}
			} else if ((ushort)RecvNtnetDt.RData116.ParkingNo != 0 && (ushort)RecvNtnetDt.RData116.PassID != 0) {
				if (RecvNtnetDt.RData116.OutYear == 0
				&& RecvNtnetDt.RData116.OutMonth == 0
				&& RecvNtnetDt.RData116.OutDay == 0
				&& RecvNtnetDt.RData116.OutHour == 0
				&& RecvNtnetDt.RData116.OutMin == 0) {
					// �o�Ɏ����@�I�[���@�O�@�̂��ߏo�Ɏ����e�[�u���폜�B
					PassExitTimeTblDelete( (ulong)RecvNtnetDt.RData116.ParkingNo, (ushort)RecvNtnetDt.RData116.PassID );
				} else {
					exit_time.Year	= RecvNtnetDt.RData116.OutYear;
					exit_time.Mon	= RecvNtnetDt.RData116.OutMonth;
					exit_time.Day	= RecvNtnetDt.RData116.OutDay;
					exit_time.Hour	= RecvNtnetDt.RData116.OutHour;
					exit_time.Min	= RecvNtnetDt.RData116.OutMin;
					PassExitTimeTblWrite( (ulong)RecvNtnetDt.RData116.ParkingNo, (ushort)RecvNtnetDt.RData116.PassID, &exit_time );
				}
			}
		}

	} else {
		// �P����e�[�u���ݒ�
		if (RecvNtnetDt.RData116.ParkingNo != 0) {

			if (RecvNtnetDt.RData116.UsingParkingNo != 0) {
				use_parking = (ulong)RecvNtnetDt.RData116.UsingParkingNo;
			}
			if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_ALL_INITIAL) {
				// �S���ڏ����� 4
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 0, use_parking, 0, 0 );
				wopelg(OPLOG_SET_TEIKI_ALLYUKO, 0, 0);		// ���엚��o�^

			} else if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_ALL_INITIAL2) {
				// ����e�[�u�����̗L�����ڂ̂ݏ����� 11
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 0, use_parking, 0, 1 );

			} else if (NTNET_PASSUPDATE_ALL_IN == RecvNtnetDt.RData116.Status) {
				// �S���ړ��ɒ� 5
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 2, use_parking, 0, 0 );

			} else if (NTNET_PASSUPDATE_ALL_OUT == RecvNtnetDt.RData116.Status) {
				// �S���ڏo�ɒ� 6
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 1, use_parking, 0, 0 );

			} else if (NTNET_PASSUPDATE_ALL_OUT_FREE == RecvNtnetDt.RData116.Status) {
				// �S���ړ��ɉ� 7
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 2, use_parking, 0, 2 );

			} else if (NTNET_PASSUPDATE_ALL_IN_FREE == RecvNtnetDt.RData116.Status) {
				// �S���ڏo�ɉ� 8
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 1, use_parking, 0, 3 );

			} else if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_ALL_INVALID) {
				// �P����e�[�u�����̗L���E�����r�b�g�̂ݖ����� 9
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 0xFFFF, use_parking, 1, 0 );
				wopelg(OPLOG_SET_TEIKI_ALLMUKO, 0, 0);		// ���엚��o�^

			} else if (RecvNtnetDt.RData116.Status == NTNET_PASSUPDATE_ALL_VALID) {
				// �P����e�[�u�����̗L���E�����r�b�g�̂ݗL���� 10
				FillPassTbl((ulong)RecvNtnetDt.RData116.ParkingNo, 0xFFFF, use_parking, 0, 0 );
				wopelg(OPLOG_SET_TEIKI_ALLYUKO, 0, 0);		// ���엚��o�^
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ�f�[�^���M����(ID84)�쐬����                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data84                                        |*/
/*| PARAMETER    : MachineNo : ���M��[���@�B��                            |*/
/*|                ng  : 1 = ���MNG, 0 = ���MOK                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data84(ulong MachineNo, uchar ng)
{
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ��M�����ʒm�f�[�^(ID99)�쐬����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data99                                        |*/
/*| PARAMETER    : MachineNo : ���M��[���@�B��                            |*/
/*|                CompleteKind  : �ݒ芮�������f�[�^�̎��                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data99(ulong MachineNo, int CompleteKind)
{
	if (CompleteKind < NTNET_COMPLETE_MAX) {
		memset(&SendNtnetDt, 0, sizeof(DATA_KIND_99));
		BasicDataMake(99, 1);										// ��{�ް��쐬
		SendNtnetDt.SData99.SMachineNo = MachineNo;					// ���M��[���@�B��
		SendNtnetDt.SData99.CompleteInfo[CompleteKind] = 1;			// �ݒ芮�����
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_99 ));
		}
		else {
			NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_99), NTNET_BUF_NORMAL);	// �ް����M�o�^
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���䉞���ް�(�ް����101)�쐬���� �i���Z�b�g������p�j                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data101_2                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data101_2( void )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_100 ) );

	BasicDataMake( 101, 1 );										// ��{�ް��쐬

	SendNtnetDt.SData101.SMachineNo = 0;							// ���M��[���@�B��

	SendNtnetDt.SData101.ControlData[7] = 1;						// ����

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_100 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}

	NTNetID100ResetFlag = 0;										// ���Z�b�g�t���O����
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ��M�����ʒm�f�[�^(ID99)�쐬����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Ans_Data99                                        |*/
/*| PARAMETER    : SysID : ���M��V�X�e��ID                                |*/
/*|              : MachineNo : ���M��[���@�B��                            |*/
/*|                CompleteKind  : �ݒ芮�������f�[�^�̎��                |*/
/*|                CompleteInfo  : �������e                                |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.10.1                                               |*/
/*| NOTE         : ���p�[�N�ݒ艞���p                                      |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Ans_Data99(uchar SysID, ulong MachineNo, int CompleteKind, uchar CompleteInfo)
{
	DATA_KIND_99	*msg = &SendNtnetDt.SData99;

	if (CompleteKind < NTNET_COMPLETE_MAX) {
		memset(msg, 0, sizeof(*msg));
		BasicDataMake(99, 1);								// ��{�ް��쐬
		msg->SMachineNo = MachineNo;						// ���M��[���@�B��
		msg->CompleteInfo[CompleteKind] = CompleteInfo;		// �ݒ芮�����
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(*msg));
		}
		else {
			NTBUF_SetSendNtData(msg, sizeof(*msg), NTNET_BUF_PRIOR);	// �ް����M�o�^(�D��f�[�^�j
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �ް��v��2(�ް����109)�쐬����                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data109                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data109( ulong req, char value )
{
	int i;

	if (_is_ntnet_remote()) {
		return;
	}

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_109 ) );
	
	BasicDataMake( 109, 1 );										// ��{�ް��쐬
	
	SendNtnetDt.SData109.SMachineNo = 0L;							// ���M��[���@�B��

	NTNetTotalEndError = 0;

	if (req & (NTNET_DATAREQ2_MSYOUKEI | NTNET_DATAREQ2_MGOUKEI | NTNET_DATAREQ2_NMSYOUKEI)) {
		memset( &sky.fsyuk, 0, sizeof( SYUKEI ) );
	}
	
	// �v�����e���Z�b�g(�������v,�������v,���ݕ������v�͑��v���ƕ��p�ł��Ȃ��d�l�̂��ߌʂɐݒ肷��)
	if ((req & NTNET_DATAREQ2_MSYOUKEI) == NTNET_DATAREQ2_MSYOUKEI) {
		SendNtnetDt.SData109.ControlData[3] = 1;				// �������v
		NTNetTotalEndFlag = 0;	// �����W�v�f�[�^��M�ҋ@��
	} else if ((req & NTNET_DATAREQ2_MGOUKEI) == NTNET_DATAREQ2_MGOUKEI) {
		SendNtnetDt.SData109.ControlData[4] = value;			// �������v
		NTNetTotalEndFlag = 0;	// �����W�v�f�[�^��M�ҋ@��
	} else if ((req & NTNET_DATAREQ2_NMSYOUKEI) == NTNET_DATAREQ2_NMSYOUKEI) {
		SendNtnetDt.SData109.ControlData[12] = value;			// ���ݕ������v
		NTNetTotalEndFlag = 0;	// �����W�v�f�[�^��M�ҋ@��
	} else {
		for (i = 0; i < _countof(SendNtnetDt.SData109.ControlData); i++) {
			if (req & (0x00000001L << i)) {
				SendNtnetDt.SData109.ControlData[i] = 1;				// �v��ON
			}
		}
		
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_109 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_109 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ������f�[�^�X�V(�ް����116)�쐬����                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data116                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data116( uchar ProcMode, ulong PassId, ulong ParkingId, uchar UseParkingKind, uchar Status, uchar UpdateStatus, date_time_rec *ExitTime )
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_116 ) );

	BasicDataMake( 116, 1 );										// ��{�ް��쐬

	SendNtnetDt.SData116.ProcMode = ProcMode;
	SendNtnetDt.SData116.PassID = PassId;
	SendNtnetDt.SData116.ParkingNo = ParkingId;

	if ((ProcMode & NTNET_PASSUPDATE_STATUS) == NTNET_PASSUPDATE_STATUS) {
		SendNtnetDt.SData116.Status = Status;
		if (Status <= NTNET_PASSUPDATE_INVALID) {
			SendNtnetDt.SData116.UsingParkingNo = (ulong)CPrmSS[S_SYS][1];		// ���Z�@�̊�{���ԏꇂ set
		}
	}

	if ((ProcMode & NTNET_PASSUPDATE_OUTTIME) == NTNET_PASSUPDATE_OUTTIME) {
		if (ExitTime == NULL) {
			SendNtnetDt.SData116.OutYear	= 0;
			SendNtnetDt.SData116.OutMonth	= 0;
			SendNtnetDt.SData116.OutDay		= 0;
			SendNtnetDt.SData116.OutHour	= 0;
			SendNtnetDt.SData116.OutMin		= 0;
		} else {
			SendNtnetDt.SData116.OutYear	= ExitTime->Year;
			SendNtnetDt.SData116.OutMonth	= ExitTime->Mon;
			SendNtnetDt.SData116.OutDay		= ExitTime->Day;
			SendNtnetDt.SData116.OutHour	= ExitTime->Hour;
			SendNtnetDt.SData116.OutMin		= ExitTime->Min;
		}
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_116 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_116 ), NTNET_BUF_NORMAL );	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���j�^�f�[�^(�ް����122)�쐬����                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data122                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data122( uchar kind, uchar code, uchar level, uchar *info, uchar *message )
{
	uchar	wks;
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_122 ) );

	BasicDataMake( 122, 0 );										// ��{�ް��쐬

	SendNtnetDt.SData122.Monsyu = kind;		// ���j�^���
	SendNtnetDt.SData122.Moncod = code;		// ���j�^�R�[�h
	SendNtnetDt.SData122.Monlev = level;	// ���j�^���x��
	if (info != NULL) {
		memcpy( &SendNtnetDt.SData122.Mondat1, info, sizeof(SendNtnetDt.SData122.Mondat1));
	}
	if (message != NULL) {
		memcpy( &SendNtnetDt.SData122.Mondat2, message, sizeof(SendNtnetDt.SData122.Mondat2));
	}
	
	wks = (uchar)prm_get(COM_PRM, S_NTN, 37, 1, 1);
	
	if ( wks != 9 && SendNtnetDt.SData122.Monlev >= wks ) {			// �e�@���M����
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_122 ));
		}
		else {
			NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_122 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
		}
	}

	/* ���ʃp�����[�^39-0022��1�̈ʂɂđ��M��/�s�`�F�b�N */
}

/*[]----------------------------------------------------------------------[]*/
/*| ���샂�j�^�f�[�^(�ް����123)�쐬����                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data123                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data123( uchar kind, uchar code, uchar level, uchar *before, uchar *after, uchar *message )
{
	uchar	wks;
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_123 ) );

	BasicDataMake( 123, 0 );										// ��{�ް��쐬

	SendNtnetDt.SData123.OpeMonsyu = kind;	// ���샂�j�^���
	SendNtnetDt.SData123.OpeMoncod = code;	// ���샂�j�^�R�[�h
	SendNtnetDt.SData123.OpeMonlev = level;	// ���샂�j�^���x��
	memcpy( &SendNtnetDt.SData123.OpeMondat1, before, sizeof(SendNtnetDt.SData123.OpeMondat1));
	memcpy( &SendNtnetDt.SData123.OpeMondat2, after, sizeof(SendNtnetDt.SData123.OpeMondat2));
	if (message != NULL) {
		memcpy( &SendNtnetDt.SData123.OpeMondat3, message, sizeof(SendNtnetDt.SData123.OpeMondat3));
	}
	
	if( SendNtnetDt.SData123.OpeMonsyu == 80 ){
		if( SendNtnetDt.SData123.OpeMoncod == 86 ){								// ���u�_�E�����[�h�����̑��샂�j�^
			if( *((ulong*)SendNtnetDt.SData123.OpeMondat1) > PROG_DL_RESET ){	// ���Z�b�g�����֘A�̃��j�^
				*((ulong*)SendNtnetDt.SData123.OpeMondat1) = PROG_DL_RESET;		// ���Z�b�g�i9�j�Ƃ��ēd���͑��M����
			}
		}
	}
	
	wks = (uchar)prm_get(COM_PRM, S_NTN, 37, 1, 2);
	
	if ( wks != 9 && SendNtnetDt.SData123.OpeMonlev >= wks ) {		// �e�@���M����
		if(_is_ntnet_remote()) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_123 ));
		}
		else {
			NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_123 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
		}
	}

	/* ���ʃp�����[�^39-0022��10�̈ʂɂđ��M��/�s�`�F�b�N */
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ʐݒ��ް�(�ް����208)�쐬���� - 1����ݑ��M                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data208                                       |*/
/*| PARAMETER    : mac   : ���M��[���@�B��                                |*/
/*|              : sts   : �ް��ð��(0:���� 1:�ŏI)                        |*/
/*|              : seg   : �ݒ辸����(�����)                               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data208(ulong mac, ushort sts, ushort seg)
{
	_NTNET_Snd_Data208(mac, sts, seg, 1, CPrmCnt[seg]);
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ʐݒ��ް�(�ް����208)�쐬���� - ���ڽ�w��                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : _NTNET_Snd_Data208                                      |*/
/*| PARAMETER    : mac   : ���M��[���@�B��                                |*/
/*|              : sts   : �ް��ð��(0:���� 1:�ŏI)                        |*/
/*|              : seg   : �ݒ辸����(�����)                               |*/
/*|              : addr  : �J�n���ڽ                                       |*/
/*|              : count : �ݒ��ް���                                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	_NTNET_Snd_Data208(ulong mac, ushort sts, ushort seg, ushort addr, ushort count)
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_80));

	BasicDataMake(208, 1);														// ��{�ް��쐬

	SendNtnetDt.SData208.SMachineNo = mac;										// ���M��[���@�B��
	SendNtnetDt.SData208.uMode = 0;												// �X�VӰ��
	SendNtnetDt.SData208.ModelCode = NTNET_MODEL_CODE;							// �[���@��R�[�h
	SendNtnetDt.SData208.Status = sts;											// �ް��ð��
	SendNtnetDt.SData208.Segment = seg;											// �ݒ辸����
	SendNtnetDt.SData208.TopAddr = addr;										// �J�n���ڽ
	SendNtnetDt.SData208.DataCount = count;										// �ݒ��ް���
	memcpy(SendNtnetDt.SData208.Data, &CPrmSS[seg][addr], count*sizeof(long));	// �ݒ��ް�

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_80 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_80), NTNET_BUF_NORMAL);	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ʐݒ��ް�(�ް����208)�쐬���� - ���ڽ�w��                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Ans_Data208                                       |*/
/*| PARAMETER    : SysID : ���M��V�X�e��ID                                |*/
/*|              : MachineNo : ���M��[���@�B��                            |*/
/*|              : addr  : �J�n���ڽ                                       |*/
/*|              : count : �ݒ��ް���                                      |*/
/*|              : okng  : ��������                                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.10.1                                               |*/
/*| NOTE         : ���p�[�N�ݒ艞���p                                      |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Ans_Data208(uchar SysID, ulong MachineNo, ushort addr, ushort count, uchar okng)
{
	RP_DATA_KIND_80	*msg = &SendNtnetDt.RP_SData208;
	ushort	seg, inaddr;
	ushort	i;
	long	tmp;
	long	lpwk,lpwk2;

	memset(msg, 0, sizeof(*msg));

	BasicDataMake( 208, 1 );										// ��{�ް��쐬
	msg->SMachineNo	= MachineNo;
	msg->ProcMode = okng;
	msg->Status = 1;
	msg->Segment = 0;
	msg->TopAddr = addr;
	msg->DataCount = count;		// <= RP_PARAMDATA_MAX�@�ł��邱��
	if (okng == 0) {
		for (i = 0; i < count; i++) {
			switch(addr) {
			case	412:	// A��
			case	414:	// B��
			case	416:	// C��
			case	418:	// D��
			case	420:	// E��
			case	422:	// F��
			case	424:	// G��
			case	426:	// H��
			case	428:	// I��
			case	430:	// J��
			case	432:	// K��
			case	434:	// L��
				msg->Data[i] = prm_get(COM_PRM, S_SHA, (short)(3+((addr%412)*3)), 4, 2);	// 2345:���|��
				break;
			case	413:	// A��
			case	415:	// B��
			case	417:	// C��
			case	419:	// D��
			case	421:	// E��
			case	423:	// F��
			case	425:	// G��
			case	427:	// H��
			case	429:	// I��
			case	431:	// J��
			case	433:	// K��
			case	435:	// L��
				tmp = prm_get(COM_PRM, S_SHA, (short)(3+((addr%413)*3)), 1, 1);		// 6:��
				tmp *= 10;
				tmp += prm_get(COM_PRM, S_SHA, (short)(4+((addr%413)*3)), 1, 6);		// 1:�y
				tmp *= 10;
				tmp += prm_get(COM_PRM, S_SHA, (short)(3+((addr%413)*3)), 1, 6);		// 1:��
				tmp *= 10;
				tmp += prm_get(COM_PRM, S_SHA, (short)(4+((addr%413)*3)), 1, 5);		// 2:��
				msg->Data[i] = tmp;
				break;
			case	436:
			case	437:
			case	438:
				tmp = 0;
				inaddr = 4 + (addr-436)*24;
				for (seg = 0; seg < 4; seg++) {
					tmp *= 10;
					tmp += prm_get(COM_PRM, S_SHA, (short)inaddr, 1, 4);
					inaddr += 6;
				}
				msg->Data[i] = tmp;
				break;
// 4 -> 7 : �����Ԃ���ő�
// 5 -> 8 : ���ԑэő�
			case	551:
				get_param_addr(0, addr, &seg, &inaddr);
				msg->Data[i] = CPrmSS[seg][inaddr];
				if( msg->Data[i] == 4 || msg->Data[i] == 10 ){
					msg->Data[i] = 7;
				}else if( msg->Data[i] == 5 ){
					msg->Data[i] = 8;
				}
				break;
// n���ԍő� 24 -> 2400 = 24:00
			case	558:
				get_param_addr(0, addr, &seg, &inaddr);
					msg->Data[i] = prm_get(COM_PRM, S_CAL, 38, 2, 3);
				msg->Data[i] *=100;
				break;
			case	559:
					msg->Data[i] = prm_get(COM_PRM, S_CAL, 38, 2, 1);
				break;
			case	472:
				break;
			case	5361:
				msg->Data[i] = prm_get(COM_PRM, S_PAY, 29, 1, 1);
				break;
			default:
				get_param_addr(0, addr, &seg, &inaddr);
				if( addr >= MAX_TYPE_SET_S && addr <= MAX_TYPE_SET_E){
				/* ��ʖ��̍ő嗿���^�C�v�̐ݒ� */
				/* �ő嗿���^�C�v�͐ݒ�l��FT4500�p�ɕϊ�(10��7  5��8)���đ��M���� */
					lpwk = CPrmSS[seg][inaddr]/GETA_10_4;					// �@�A�𒊏o
					if(lpwk == SP_MX_N_MH_NEW){								// �����Ԃ���ő�(�P�O)
						lpwk2 = FT4500_MX_N_HOUR*GETA_10_4;					// FT4500�ł͂V
					}
					else if(lpwk == SP_MX_BAND){							// ���ԑэő�(�T)
						lpwk2 = FT4500_MX_BAND*GETA_10_4;					// FT4500�ł͂W
					}
					else {
						lpwk2 = lpwk*GETA_10_4;
					}
					lpwk = (CPrmSS[seg][inaddr]%GETA_10_4)/GETA_10_2;		// �B�C�𒊏o
					if(lpwk == SP_MX_N_MH_NEW){								// �����Ԃ���ő�(�P�O)
						lpwk2 += FT4500_MX_N_HOUR*GETA_10_2;				// FT4500�ł͂V
					}
					else if(lpwk == SP_MX_BAND){							// ���ԑэő�(�T)
						lpwk2 += FT4500_MX_BAND*GETA_10_2;					// FT4500�ł͂W
					}
					else {
						lpwk2 += lpwk*GETA_10_2;
					}
					lpwk = CPrmSS[seg][inaddr]%GETA_10_2;					// �D�E�𒊏o
					if(lpwk == SP_MX_N_MH_NEW){
						lpwk2 += FT4500_MX_N_HOUR;
					}
					else if(lpwk == SP_MX_BAND){
						lpwk2 += FT4500_MX_BAND;
					}
					else {
						lpwk2 += lpwk;
					}
					msg->Data[i] = lpwk2;
					break;
				}
				msg->Data[i] = CPrmSS[seg][inaddr];
				break;
			}
			addr++;
		}
	}
	NTBUF_SetSendNtData(msg, (ushort)RP_DATA80_LEN, NTNET_BUF_NORMAL);	// �ް����M�o�^
}

/*[]----------------------------------------------------------------------[]*/
/*| ��������Z���~�f�[�^�e�[�u��(�ް����219)���M����                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData219                                        |*/
/*| PARAMETER    : Mode �� 0=1���X�V, 1=1���폜, 2=�S�f�[�^�X�V            |*/
/*|                Data �� ���M�f�[�^(Mode��0or1�̂Ƃ��̂ݎg�p)            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data219(uchar Mode, struct TKI_CYUSI *Data)
{
	short i;
	
	if (_is_ntnet_remote()) {
		return;
	}
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_219));
	BasicDataMake(219, 1);														// ��{�ް��쐬
	SendNtnetDt.SData219.ProcMode = Mode;
	
	switch (Mode) {
	case 1:		// 1���폜
		ntnet_TeikiCyusiCnvCrmToNt(&SendNtnetDt.SData219.TeikiChusi[0], Data, TRUE);
		break;
	case 0:		// 1���X�V
		ntnet_TeikiCyusiCnvCrmToNt(&SendNtnetDt.SData219.TeikiChusi[0], Data, TRUE);
		break;
	case 2:		// �S�f�[�^�X�V
		for (i = 0; i < tki_cyusi.count; i++) {
			ntnet_TeikiCyusiCnvCrmToNt(&SendNtnetDt.SData219.TeikiChusi[i], TKI_Get(NULL, i), TRUE);
		}
		break;
	default:
		return;
	}
	
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_219 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_219), NTNET_BUF_NORMAL);	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ������ð��ð���(�ް����221)�쐬����                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data221                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-21                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data221(void)
{
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_93));
	BasicDataMake(221, 1);														// ��{�ް��쐬
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	memcpy(SendNtnetDt.SData221.PassTable, pas_tbl, sizeof(pas_tbl));			// ������ð��ð���
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_93 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_93), NTNET_BUF_NORMAL);	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ������X�Vð���(�ް����222)�쐬����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data222                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data222(void)
{
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_94));
	BasicDataMake(222, 1);														// ��{�ް��쐬
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	memcpy(SendNtnetDt.SData222.PassRenewal, pas_renewal, sizeof(pas_renewal));	// ������ð��ð���
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_94 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_94), NTNET_BUF_NORMAL);	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ������o�Ɏ���ð���(�ް����223)�쐬����                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data223                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data223(void)
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	short	i;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)

	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_95));
	BasicDataMake(223, 1);																				// ��{�ް��쐬
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	/* ������o�Ɏ���ð��� */
//	for (i = 0; i < pas_extimtbl.Count; i++) {
//		SendNtnetDt.SData223.PassExTable[i] = pas_extimtbl.PassExTbl[i];
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_95 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_95), NTNET_BUF_NORMAL);							// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*
 *| �Ԏ��p�����[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data225
 *| PARAMETER    : MachineNo = ���M��[���@�BNo.
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2005-11-30
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data225( ulong MachineNo )
{

	short i;
	short j=0;
	if (_is_ntnet_remote()) {
		return;
	}

	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_225));
	BasicDataMake(225, 1);										// ��{�ް��쐬
	SendNtnetDt.SData225.ModelCode = NTNET_MODEL_CODE;			// �[���@��R�[�h
	SendNtnetDt.SData225.SMachineNo	= MachineNo;				// ���M��[���@�B��

	for (i = 0; i < LOCK_MAX; i++) {

		WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( !SetCarInfoSelect(i) ){
			continue;
		}
		if( j >= OLD_LOCK_MAX ){
			break;
		}
		SendNtnetDt.SData225.LockInfo[j].lok_syu	=	LockInfo[i].lok_syu;
		SendNtnetDt.SData225.LockInfo[j].ryo_syu	=	LockInfo[i].ryo_syu;
		SendNtnetDt.SData225.LockInfo[j].area		=	LockInfo[i].area;
		SendNtnetDt.SData225.LockInfo[j].posi		=	LockInfo[i].posi;
		SendNtnetDt.SData225.LockInfo[j].if_oya		=	LockInfo[i].if_oya;
		if( i < BIKE_START_INDEX ) {	// ���ԏ��̏ꍇ�A�ϊ����K�v
			SendNtnetDt.SData225.LockInfo[j].lok_no		=	LockInfo[i].lok_no + PRM_LOKNO_MIN ;	// �\���p��+100�����l���킽��
		} else {													// ���ւ͏]���ʂ�
			SendNtnetDt.SData225.LockInfo[j].lok_no		=	LockInfo[i].lok_no;
		}
		j++;
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_225 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_225), NTNET_BUF_NORMAL);	// �ް����M�o�^
	}

}

/*[]----------------------------------------------------------------------[]*/
/*| ۯ����u���Ұ�(�ް����226)�쐬����                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data226                                       |*/
/*| PARAMETER    : MachineNo = ���M��[���@�BNo.                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data226( ulong MachineNo )
{
	int i;
	if (_is_ntnet_remote()) {
		return;
	}
	
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_226));
	BasicDataMake(226, 1);										// ��{�ް��쐬
	SendNtnetDt.SData226.ModelCode = NTNET_MODEL_CODE;			// �[���@��R�[�h

	SendNtnetDt.SData226.SMachineNo	= MachineNo;				// ���M��[���@�B��
	
	for (i = 0; i < 6; i++) {
		SendNtnetDt.SData226.LockMaker[i].in_tm		= LockMaker[i].in_tm;
		SendNtnetDt.SData226.LockMaker[i].ot_tm		= LockMaker[i].ot_tm;
		SendNtnetDt.SData226.LockMaker[i].r_cnt		= LockMaker[i].r_cnt;
		SendNtnetDt.SData226.LockMaker[i].r_tim		= LockMaker[i].r_tim;
		SendNtnetDt.SData226.LockMaker[i].open_tm	= LockMaker[i].open_tm;
		SendNtnetDt.SData226.LockMaker[i].clse_tm	= LockMaker[i].clse_tm;
	}
	
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_226 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_226), NTNET_BUF_NORMAL);	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*
 *| �ݒ�f�[�^�v����M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData78
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-28
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData78( void )
{
	RP_DATA_KIND_78	*msg = &RecvNtnetDt.RP_RData78;
	ushort	moncode;
	uchar	okng;

	NTNET_ClrSetup();

	okng = 0;
// match machine no.?
	if (msg->SMachineNo != (ulong)CPrmSS[S_PAY][2]) {
		okng = _RPKERR_INVALID_MACHINENO;
	}
// specified address is valid?
	else if (msg->Segment != 0 ||
			check_remote_addr(0, msg->TopAddr, msg->DataCount) == 0) {
		okng = _RPKERR_INVALID_PARA_ADDR;
	}
	else if (! check_acceptable()) {
		okng = _RPKERR_COMMAND_REJECT;
	}
// answer
	NTNET_Ans_Data208(msg->DataBasic.SystemID, msg->DataBasic.MachineNo,
							msg->TopAddr, msg->DataCount, okng);
// regist monitor
	moncode = OPMON_RSETUP_REFOK;
	if (okng != 0) {
		moncode++;
	}
	regist_mon(moncode, (ushort)okng, msg->TopAddr, 6);
}

/*[]----------------------------------------------------------------------[]*
 *| �ݒ�f�[�^���M�v������
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData114
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-28
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData114( void )
{
	RP_DATA_KIND_114	*msg = &RecvNtnetDt.RData114;
	ushort	moncode;
	uchar	okng;

// check each request
	okng = 0;
	if (msg->reqSend == 1) {
		moncode = OPMON_RSETUP_PREOK;
	}
	else if (msg->reqSend == 2) {
		moncode = OPMON_RSETUP_CANOK;
	}
	else if (msg->reqExec == 1) {
		moncode = OPMON_RSETUP_EXEOK;
	}
	else {
		moncode = OPMON_RSETUP_NOREQ;
	}

// check parameter & condition
	if (moncode == OPMON_RSETUP_NOREQ) {
		okng = _RPKERR_COMMAND_REJECT;
	}
	else {
	// match machine no.?
		if (msg->SMachineNo != (ulong)CPrmSS[S_PAY][2]) {
			okng = _RPKERR_INVALID_MACHINENO;
			moncode++;		// NG code
		}
		else {
			switch(moncode) {
			case	OPMON_RSETUP_EXEOK:
				if (tmp_buffer.prepare == 0) {
					okng = _RPKERR_NO_PREPARE_COMMAND;
					moncode++;		// NG code
					break;
				}
			/*	not-break */
			case	OPMON_RSETUP_PREOK:
				if (! check_acceptable()) {
					okng = _RPKERR_COMMAND_REJECT;
					moncode++;		// NG code
					break;
				}
				break;
			default:
				break;
			}
		}
	}
// answer for request
	NTNET_Ans_Data115(msg->DataBasic.SystemID, msg->DataBasic.MachineNo,
								okng);

// exec that request & regist monitor
	if (okng == 0) {
		wmonlg(moncode, 0, 0);
		switch(moncode) {
		case	OPMON_RSETUP_PREOK:
			tmp_buffer.prepare = 1;
			tmp_buffer.bufcnt = 0;
			tmp_buffer.bufofs = 0;
			break;
		case	OPMON_RSETUP_CANOK:
			NTNET_ClrSetup();
			break;
		case	OPMON_RSETUP_EXEOK:
			update_param();
			NTNET_ClrSetup();
			DataSumUpdate(OPE_DTNUM_COMPARA);		/* update parameter sum on ram */
													// �����Ұ��ͺ�߰���Ȃ��iAppServ_PParam_Copy = OFF�j
			FLT_WriteParam1(FLT_NOT_EXCLUSIVE);		/* FlashROM update */
			f_NTNET_RCV_MC10_EXEC = 1;				/* mc10()�X�V���K�v�Ȑݒ肪�ʐM�ōX�V���ꂽ���set */
			wmonlg(OPMON_RSETUP_CHGOK, 0, 0);		// �ύX�I��OK
			SetSetDiff(SETDIFFLOG_SYU_REMSET);		// �f�t�H���g�Z�b�g���O�o�^����
			break;
		}
	}
	else {
		NTNET_ClrSetup();					// cancel prepare command if error
		regist_mon(moncode, (ushort)okng, 0, 0);
	}
}

const ushort SwDspNum[5] = { 1, 0, 0, 2, 3 };	// �ؑ։�ʐ�
												// 0: ��ʐؑւȂ�
												// 1: ���Ή�
												// 2: ���Ή�
												// 3: �Q��ʐؑ�
												// 4: �R��ʐؑ�
/*[]----------------------------------------------------------------------[]*
 *| ���ݑ䐔�Ǘ��f�[�^��M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData234
 *| PARAMETER    : 
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_RevData234(void)
{
	ushort i;
	PARK_NUM_CTRL_Sub*	pt;
	uchar addr;
	uchar stat = (uchar)prm_get(COM_PRM,S_SYS,39,1,2);

	if ((stat == 0) ||													// ���ԏ󋵕\���@�\�Ȃ�
		(prm_get(COM_PRM,S_SYS,39,1,1) != RecvNtnetDt.RData234.State))	// �p�r�ʒ��ԑ䐔�ݒ聂���ʐݒ�
	{
		s_Is234StateValid = 0;
		return;
	}
	s_Is234StateValid = 1;

	if (SwDspNum[RecvNtnetDt.RData234.State]) {
		if (RecvNtnetDt.RData234.State == 0) {
			addr = 1;
		} else {
			addr = 5;
		}
		pt = (PARK_NUM_CTRL_Sub*)&PPrmSS[S_P02][addr];

		for (i = 0; i < SwDspNum[RecvNtnetDt.RData234.State]; i++) {
			switch (RecvNtnetDt.RData234.Data[i].CurNum) {			// ���ݑ䐔
			case 10000:				// ���ݑ䐔���{�P
				if (stat == 2) {
					pt->car_cnt++;
				}
				break;
			case 10001:				// �ύX�Ȃ�
				break;
			case 10002:				// ���ݑ䐔���[�P
				if (pt->car_cnt && (stat == 2)) {
					pt->car_cnt--;
				}
				break;
			default:
				if (RecvNtnetDt.RData234.Data[i].CurNum <= 9999)
				{
					pt->car_cnt = RecvNtnetDt.RData234.Data[i].CurNum;
				}
				break;
			}

			switch (RecvNtnetDt.RData234.Data[i].NoFullNum) {			// ���ԉ����䐔
			case 10001:				// �ύX�Ȃ�
				break;
			default:
				pt->kai_cnt = RecvNtnetDt.RData234.Data[i].NoFullNum;	// ���ԉ����䐔
				break;
			}

			switch (RecvNtnetDt.RData234.Data[i].FullNum) {				// ���ԑ䐔
			case 10001:				// �ύX�Ȃ�
				break;
			default:
				pt->ful_cnt = RecvNtnetDt.RData234.Data[i].FullNum;		// ���ԑ䐔
				break;
			}
			pt++;
		}

		NTNET_Snd_Data99(RecvNtnetDt.RData234.DataBasic.MachineNo, NTNET_COMPLETE_PRKCTRL);	// NT-NET�ݒ��M�����f�[�^�쐬
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���ݑ䐔�Ǘ��f�[�^��M���菈��
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_is234StateValid
 *[]----------------------------------------------------------------------[]*
 *| Date         : 2006-09-27
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
char	NTNET_is234StateValid(void)
{
	if (s_Is234StateValid) {
		// �p�r�ʒ��ԑ䐔�ݒ聁���ʐݒ�
		return 1;
	} else {
		// �p�r�ʒ��ԑ䐔�ݒ聂���ʐݒ�
		return 0;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���u������X�e�[�^�X�e�[�u����M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData92
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData92(void)
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	ushort i;
//
//	/* ������ð��ð��ٍX�V */
//	switch (RecvNtnetDt.RData92.ProcMode) {
//	case 0:		// �S�X�V
//		memcpy(pas_tbl, RecvNtnetDt.RData92.PassTable, sizeof(pas_tbl));
//		break;
//	case 1:		// �L�������r�b�g(b07)�̂ݍX�V
//		for (i = 0; i < PAS_MAX; i++) {
//			pas_tbl[i].BIT.INV = RecvNtnetDt.RData92.PassTable[i].BIT.INV;
//		}
//		break;
//	case 2:		// ���o�ɃX�e�[�^�X(b0�`b3)�̂ݍX�V
//		for (i = 0; i < PAS_MAX; i++) {
//			pas_tbl[i].BIT.STS = RecvNtnetDt.RData92.PassTable[i].BIT.STS;
//		}
//		break;
//	default:
//		break;
//	}
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	wopelg(OPLOG_SET_STATUS_TBL_UPDATE, 0, 0);		// ���엚��o�^
}

/*[]----------------------------------------------------------------------[]*
 *| ���u����f�[�^��M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData240
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData240(void)
{
	ulong work;

	// �O���[�vNo  �Ή��Ȃ�
	// ����߽�؊�
	if( RecvNtnetDt.RData240.ControlData[0] == 1 ){						// ����?
		PPrmSS[S_P01][3] = 0L;											// ����߽��������
		wopelg( OPLOG_SET_NYUSYUTUCHK_ON, 0, 0 );		// ���엚��o�^
	}
	else if( RecvNtnetDt.RData240.ControlData[0] == 2 ){				// ����?
		PPrmSS[S_P01][3] = 1L;											// ����߽������������
		wopelg( OPLOG_SET_NYUSYUTUCHK_OFF, 0, 0 );		// ���엚��o�^
	}

	// ����Ԏw��Ӱ�� �Ή��Ȃ�
	// ����Ԑ���ԍ� �Ή��Ȃ�
	work = 0;
	// ���Ԑ���P
	work |= (ulong)car_park_condition(1, RecvNtnetDt.RData240.ControlData[3]);
	// ���Ԑ���Q
	work |= (ulong)car_park_condition(2, RecvNtnetDt.RData240.ControlData[4]);
	// ���Ԑ���R
	work |= (ulong)car_park_condition(3, RecvNtnetDt.RData240.ControlData[5]);
	if (work) {
	// �ǂꂩ���ω�������
		wopelg(OPLOG_SET_MANSYACTRL, 0, 0);
	}
}

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
///*[]----------------------------------------------------------------------[]*
// *| ���u�f�[�^�v��
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData243
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| Author       : 
// *| Date         : 2007-02-02
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//void	NTNET_RevData243(void)
//{
//	char i;
//	char req = 0;
//	ushort code;
//	
//	if (_is_ntnet_remote()) {
//		req = 0;
//		for (i = 0; i < 20; i++) {
//			if (RecvNtnetDt.RData243.ControlData[i]) {
//				req++;
//				code = i;
//			}
//		}
//		switch(code) {
//		case	0:	// �S�Ԏ����e�[�u���v��
//		case	1:	// ���ԑ䐔�f�[�^�v��
//			break;
//		default:
//			req = 0;
//// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
//			break;
//// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
//		}
//		if (req != 1) {
//			// �Ǘ��f�[�^�v��NG���M
//			code = 0;	// ���ڑ��Ƃ��ĕԑ�
//			NTNET_Snd_Data244(code);
//			return;
//		}
//	}
//
//	// �S�Ԏ����e�[�u���v��
//	if (RecvNtnetDt.RData243.ControlData[0] != 0) {
//		memset(&RyoCalSim, 0, sizeof(RyoCalSim));
//		RyoCalSim.GroupNo = RecvNtnetDt.RData243.GroupNo;
//		queset(OPETCBNO, OPE_REQ_CALC_FEE, 0, NULL);
//	}
//	// ���ԑ䐔�f�[�^�v��
//	if (RecvNtnetDt.RData243.ControlData[1]) {
//		NTNET_Snd_Data238();
//	}
//}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

/*[]----------------------------------------------------------------------[]*
 *| �ݒ�f�[�^���M��������
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_SndData115
 *| PARAMETER    : ���M��[���@�B��
 *|                �����敪(0: OK/ 1: NG)
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-28
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data115( ulong MachineNo, uchar ProcMode )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_115 ) );

	// ��{�ް��쐬
	BasicDataMake( 115, 1 );
	
	SendNtnetDt.SData115.SMachineNo = MachineNo;
	SendNtnetDt.SData115.ProcMode   = ProcMode;
									
	// �ް����M�o�^
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_115 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_115 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ�f�[�^���M����(ID115)�쐬����                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data115                                       |*/
/*| PARAMETER    : SysID : ���M��V�X�e��ID                                |*/
/*|              : MachineNo : ���M��[���@�B��                            |*/
/*|                ng  : 0 = ���MOK/���̑� = �G���[�R�[�h                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : MATSUSHITA                                              |*/
/*| Date         : 2007.10.1                                               |*/
/*| NOTE         : ���p�[�N�ݒ艞���p                                      |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Ans_Data115(uchar SysID, ulong MachineNo, uchar ng)
{
	RP_DATA_KIND_115	*msg = &SendNtnetDt.SData115;

	memset(msg, 0, sizeof(*msg));

	BasicDataMake( 115, 1 );										// ��{�ް��쐬
	msg->SMachineNo	= MachineNo;
	msg->ProcMode = ng;

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(*msg));
	}
	else {
		NTBUF_SetSendNtData(msg, sizeof(*msg), NTNET_BUF_PRIOR);	// �ް����M�o�^(�D��f�[�^�j
	}
}

/*[]----------------------------------------------------------------------[]*
 *| �R�C�����ɏ��v�f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data130
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data130( void )
{
	ntnet_Snd_CoinData(130, 41);
}

/*[]----------------------------------------------------------------------[]*
 *| �R�C�����ɍ��v�f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data131
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-10-18
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data131( void )
{
	ntnet_Snd_CoinData(131, 51);
}

/*[]----------------------------------------------------------------------[]*
 *| �R�C�����ɏW�v�f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : ntnet_Snd_CoinData
 *| PARAMETER    : kind : �f�[�^���
 *|                type : �W�v�^�C�v�i���v�F41/���v�F51�j
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-10-18
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ntnet_Snd_CoinData( uchar kind, ushort type )
{
	ushort i;
	uchar  keep;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_130 ) );

	// ��{�ް��쐬
	keep = 0;				// �ێ�����
	if( type == 41 ){		// ���v�̂Ƃ�
		keep = 1;			// �ێ����Ȃ�
	}
	BasicDataMake( kind, keep );

	if( type == 51 )		// ���v�̂Ƃ�
	{
		SendNtnetDt.SData130.KakariNo = OPECTL.Kakari_Num;		// �W����
		SendNtnetDt.SData130.SeqNo = CountSel( &coin_syu.Oiban );			// �ǔ�
	}

	// �^�C�v�ݒ�
	SendNtnetDt.SData130.Type = type;		// ���v(41)/���v(51)
	
	SendNtnetDt.SData130.Sf.Coin[0].Money = 10;						// ��݋��ɋ���(10�~)
	SendNtnetDt.SData130.Sf.Coin[1].Money = 50;						// ��݋��ɋ���(50�~)
	SendNtnetDt.SData130.Sf.Coin[2].Money = 100;					// ��݋��ɋ���(100�~)
	SendNtnetDt.SData130.Sf.Coin[3].Money = 500;					// ��݋��ɋ���(500�~)

	SendNtnetDt.SData130.Sf.Coin[0].Mai = SFV_DAT.safe_dt[0];		// ��݋��ɖ���(10�~)
	SendNtnetDt.SData130.Sf.Coin[1].Mai = SFV_DAT.safe_dt[1];		// ��݋��ɖ���(50�~)
	SendNtnetDt.SData130.Sf.Coin[2].Mai = SFV_DAT.safe_dt[2];		// ��݋��ɖ���(100�~)
	SendNtnetDt.SData130.Sf.Coin[3].Mai = SFV_DAT.safe_dt[3];		// ��݋��ɖ���(500�~)

	for( i=0; i<4; i++ ){
		SendNtnetDt.SData130.SfTotal +=								// ��݋��ɑ��z
// MH322914 (s) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
//			SendNtnetDt.SData130.Sf.Coin[i].Mai * SendNtnetDt.SData130.Sf.Coin[i].Money;
			(ulong)SendNtnetDt.SData130.Sf.Coin[i].Mai * (ulong)SendNtnetDt.SData130.Sf.Coin[i].Money;
// MH322914 (e) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
	}
									
	// �ް����M�o�^
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_130 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_130 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*
 *| �������ɏW�v�f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data132
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data132( void )
{
	ntnet_Snd_NoteData(132, 42);
}

/*[]----------------------------------------------------------------------[]*
 *| �������ɏW�v�f�[�^���M����(���v)
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data133
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-10-18
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data133( void )
{
	ntnet_Snd_NoteData(133, 52);
}

/*[]----------------------------------------------------------------------[]*
 *| �������ɏW�v�f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : ntnet_Snd_NoteData
 *| PARAMETER    : kind : �f�[�^���
 *|                type : �W�v�^�C�v�i���v�F42/���v�F52�j
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-10-18
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ntnet_Snd_NoteData( uchar kind, ushort type )
{
	uchar  keep;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_132 ) );

	// ��{�ް��쐬
	keep = 0;				// �ێ�����
	if( type == 42 ){		// ���v�̂Ƃ�
		keep = 1;			// �ێ����Ȃ�
	}
	BasicDataMake( kind, keep );
	if( type == 52 )		// ���v�̂Ƃ�
	{
		SendNtnetDt.SData132.KakariNo = OPECTL.Kakari_Num;			// �W����
		SendNtnetDt.SData132.SeqNo = CountSel ( &note_syu.Oiban );	// �ǔ�
	}

	// �^�C�v�ݒ�
	SendNtnetDt.SData132.Type = type;		// �������ɏ��v(42)/���v(52)

	SendNtnetDt.SData132.Sf.Note[0].Money = 1000;					// �������ɋ���(1000�~)
	SendNtnetDt.SData132.Sf.Note[0].Mai = SFV_DAT.nt_safe_dt;		// �������ɖ���(1000�~)
	SendNtnetDt.SData132.SfTotal =									// �������ɑ��z
// MH322914 (s) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
//		SendNtnetDt.SData132.Sf.Note[0].Mai * SendNtnetDt.SData132.Sf.Note[0].Money;
		(ulong)SendNtnetDt.SData132.Sf.Note[0].Mai * (ulong)SendNtnetDt.SData132.Sf.Note[0].Money;
// MH322914 (e) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
									
	// �ް����M�o�^
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_132 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_132 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���ʓ��ݒ�f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data211
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : GT4700���痬�p
 *| Date         : 2006-09-28
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data211( void )
{
	if (_is_ntnet_remote()) {
		return;
	}
	NTNET_Snd_Data211_Exec();
}

void	NTNET_Snd_Data211_Exec( void )
{
	ushort	i, pos;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_211 ) );

	// ��{�ް��쐬
	BasicDataMake( 211, 1 );										
	
	// ���ʓ��ݒ��ް��쐬
	for( i=0; i<3; i++ ){														// ���ʊ��ԂP�`�R
		SendNtnetDt.SData211.kikan[i].sta_Mont = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)(i*2+1), 2, 3 );			// �J�n��
		SendNtnetDt.SData211.kikan[i].sta_Date = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)(i*2+1), 2, 1 );			// �J�n��
		SendNtnetDt.SData211.kikan[i].end_Mont = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)(i*2+2), 2, 3 );			// �I����
		SendNtnetDt.SData211.kikan[i].end_Date = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)(i*2+2), 2, 1 );			// �I����
	}
    
	for( i=0, pos=NTNET_SPLDAY_START; pos<= NTNET_SPLDAY_END; i++, pos++ ){		// ���ʓ����V�t�g�P�`�R�P
		SendNtnetDt.SData211.date[i].mont =
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 3 );				// ��
		SendNtnetDt.SData211.date[i].day = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 1 );				// ��
		SendNtnetDt.SData211.Shift[i] =
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 6 );				// �V�t�g
	} 
	
	if (CPrmSS[S_TOK][41]) {
		for( i=0, pos=NTNET_SPYOBI_START; pos<= NTNET_SPYOBI_END; i++, pos++ ){	// ���ʗj���P�`�P�Q
			SendNtnetDt.SData211.yobi[i].mont = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 3 );			// ��
			SendNtnetDt.SData211.yobi[i].week = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 2 );			// �T
			SendNtnetDt.SData211.yobi[i].yobi = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 1 );			// �j��
		}
	}
	else {
		for( i=0, pos=NTNET_HMON_START; pos<= NTNET_HMON_END; i+=2, pos++ ){	// �n�b�s�[�}���f�[�P�`�P�Q
			SendNtnetDt.SData211.yobi[i].mont = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 5 );			// ��
			SendNtnetDt.SData211.yobi[i].week = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 4 );			// �T
			if( 0 != SendNtnetDt.SData211.yobi[i].mont )						// �f�[�^����
				SendNtnetDt.SData211.yobi[i].yobi = 1;							// �j���i���j���Œ�j

			SendNtnetDt.SData211.yobi[i+1].mont = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 2 );			// ��
			SendNtnetDt.SData211.yobi[i+1].week = 
				(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 1, 1 );			// �T
			if( 0 != SendNtnetDt.SData211.yobi[i+1].mont )						// �f�[�^����
				SendNtnetDt.SData211.yobi[i+1].yobi = 1;						// �j���i���j���Œ�j
		}
	}

	SendNtnetDt.SData211.spl56 = (ushort)((CPrmSS[S_TOK][NTNET_5_6] & 0x01) ^ 0x01 );	// 5�^6����ʓ��Ƃ���
	
	for( i=0, pos=NTNET_SPYEAR_START; pos<= NTNET_SPYEAR_END; i++, pos++ ){		// ���ʔN�����P�`�U
		SendNtnetDt.SData211.year[i].year = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 5 );				// �N
		if( SendNtnetDt.SData211.year[i].year != 0 ){
			SendNtnetDt.SData211.year[i].year += 2000;
		}
		SendNtnetDt.SData211.year[i].mont = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 3 );				// ��
		SendNtnetDt.SData211.year[i].day = 
			(ushort)prm_get( COM_PRM, S_TOK, (short)pos, 2, 1 );				// ��
	}

	// �ް����M�o�^
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_211 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_211 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ����J�E���g�f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data228
 *| PARAMETER    : MachineNo : ���M��[���@�B��
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data228( ulong MachineNo )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_228 ) );

	// ��{�ް��쐬
	BasicDataMake( 228, 1 );
	
	SendNtnetDt.SData228.MachineNo = MachineNo;
	memcpy(SendNtnetDt.SData228.Count, Mov_cnt_dat, (sizeof(ulong) * MOV_CNT_MAX));

	// �ް����M�o�^
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_228 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_228 ), NTNET_BUF_NORMAL );
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���ԑ䐔�Ǘ��f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data235
 *| PARAMETER    : 
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-09-27
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data235(char bReq)
{
	if (_is_ntnet_remote()) {
		return;
	}
	NTNET_Snd_Data235_Exec(bReq);
}

void	NTNET_Snd_Data235_Exec( char bReq )
{
	ushort i = 0;
	PARK_NUM_CTRL_Sub*	pt;
	uchar addr;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_235 ) );

	// ��{�ް��쐬
	BasicDataMake( 235, 1 );

	if ((prm_get(COM_PRM,S_SYS,39,1,2) == 0) ||
		((prm_get(COM_PRM,S_SYS,39,1,2) == 1) && (bReq == 1))) {
		// �O���[�v�ȉ��A���ׂ�0�ő��M
		SendNtnetDt.SData235.GroupNo = 0;
		SendNtnetDt.SData235.State   = 0;
		SendNtnetDt.SData235.Data[i].CurNum    = 0;
		SendNtnetDt.SData235.Data[i].NoFullNum = 0;
		SendNtnetDt.SData235.Data[i].FullNum   = 0;
	} else {
		SendNtnetDt.SData235.GroupNo = 0;
		SendNtnetDt.SData235.State   = prm_get(COM_PRM,S_SYS,39,1,1);		// �p�r�ʒ��ԑ䐔�ݒ�
		
		if (SwDspNum[SendNtnetDt.SData235.State]) {
			if (SendNtnetDt.SData235.State == 0) {
				addr = 1;
			} else {
				addr = 5;
			}
			pt = (PARK_NUM_CTRL_Sub*)&PPrmSS[S_P02][addr];
		
			for (i = 0; i < SwDspNum[SendNtnetDt.SData235.State]; i++) {
				SendNtnetDt.SData235.Data[i].CurNum    = pt->car_cnt;		// ���ݑ䐔
				SendNtnetDt.SData235.Data[i].NoFullNum = pt->kai_cnt;		// ���ԉ����䐔
				SendNtnetDt.SData235.Data[i].FullNum   = pt->ful_cnt;		// ���ԑ䐔
				pt++;
			}
		}
	}

	// �ް����M�o�^
	switch( bReq ){
		case	0:
		case	1:	// �e�@�݂̂ɑ��M
			if(_is_ntnet_remote()) {
				RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_235 ));
			}
			else {
				NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_235 ), NTNET_BUF_NORMAL );  // FT4800N
			}
			break;
		case	2:	// �e�@�y��MAF�ɑ��M
			if(_is_ntnet_remote()) {
				RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_235 ));
			}
			else {
			NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_235 ), NTNET_BUF_NORMAL );
			}
			// no break;
			break;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���ԑ䐔�f�[�^/���ԑ䐔�����f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_DataParkCarNum
 *| PARAMETER    : uchar 236: / 238:
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-01-31
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_DataParkCarNum(uchar kind)
{
	int i, j;
	ulong curnum = 0;
	uchar uc_prm;
	ulong curnum_1 = 0;
	ulong curnum_2 = 0;
	ulong curnum_3 = 0;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_236 ) );

	// ��{�ް��쐬
	BasicDataMake( kind, 1 );
	// ��ٰ�ߔԍ�
	SendNtnetDt.SData236.GroupNo = 0;
	for (i = 0; i < LOCK_MAX; i++) {
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		// ������ʖ��Ɍ��ݍݎԶ���
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {

				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );	// ��ʖ����Ă���ݒ�get

				if( uc_prm == 1 ){			// ���ݑ䐔�P���J�E���g
					curnum++;
					curnum_1++;
					SendNtnetDt.SData236.RyCurNum[j]++;
				}
				else if( uc_prm == 2 ){		// ���ݑ䐔�Q���J�E���g
					curnum++;
					curnum_2++;
					SendNtnetDt.SData236.RyCurNum[j]++;
				}
				else if( uc_prm == 3 ){		// ���ݑ䐔�R���J�E���g
					curnum++;
					curnum_3++;
					SendNtnetDt.SData236.RyCurNum[j]++;
				}
			}
		}
	}
	// ���ݒ��ԑ䐔
	SendNtnetDt.SData236.CurNum = curnum;

	// �p�r�ʒ��ԑ䐔�ݒ�
	SendNtnetDt.SData236.State = prm_get(COM_PRM,S_SYS,39,1,1);
	if (_is_ntnet_remote()) {											// ���uNT-NET�ݒ�̂Ƃ��̂݃f�[�^�Z�b�g
		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {					// 01-0039�D���ԑ䐔��{�̂ŊǗ�����
			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
			case 0:														// 01-0039�E�p�r�ʒ��ԑ䐔�ؑւȂ�
				SendNtnetDt.SData236.FullNo1 = PPrmSS[S_P02][3];		// ���ԑ䐔
				if (curnum > PPrmSS[S_P02][3]) {
					SendNtnetDt.SData236.EmptyNo1 = 0;					// ��ԑ䐔
				} else {
					SendNtnetDt.SData236.EmptyNo1 = PPrmSS[S_P02][3] - curnum;					// ��ԑ䐔(���ԑ䐔 - ���ݒ��ԑ䐔)
				}
				if( SendNtnetDt.SData236.EmptyNo1 == 0 ){		// ��Ԃ��u�O�v�̏ꍇ
					SendNtnetDt.SData236.Full[0] = 1;			// �u���ԁv��ԃZ�b�g
				}
				switch (PPrmSS[S_P02][1]) {								// ���������Ӱ��
				case	1:												// ��������
					SendNtnetDt.SData236.Full[0] = 1;					// �u�������ԁv��ԃZ�b�g
					break;
				case	2:												// �������
					SendNtnetDt.SData236.Full[0] = 0;					// �u������ԁv��ԃZ�b�g
					break;
				default:												// ����
					break;
				}
				break;

			case 3:														// 01-0039�E�p�r�ʒ��ԑ䐔�Q�n��
			case 4:														// 01-0039�E�p�r�ʒ��ԑ䐔�R�n��
				SendNtnetDt.SData236.FullNo1 = PPrmSS[S_P02][7];		// ���ԑ䐔�P
				if (curnum_1 > PPrmSS[S_P02][7]) {
					SendNtnetDt.SData236.EmptyNo1 = 0;					// ��ԑ䐔�P
				} else {
					SendNtnetDt.SData236.EmptyNo1 = PPrmSS[S_P02][7] - curnum_1;				// ��ԑ䐔�P(���ԑ䐔 - ���ݒ��ԑ䐔)
				}
				if( SendNtnetDt.SData236.EmptyNo1 == 0 ){		// ��Ԃ��u�O�v�̏ꍇ
					SendNtnetDt.SData236.Full[0] = 1;			// �u���ԁv��ԃZ�b�g
				}
				switch (PPrmSS[S_P02][5]) {								// ��������ԃ��[�h1
				case	1:												// ��������
					SendNtnetDt.SData236.Full[0] = 1;					// �u�������ԁv��ԃZ�b�g
					break;
				case	2:												// �������
					SendNtnetDt.SData236.Full[0] = 0;					// �u������ԁv��ԃZ�b�g
					break;
				default:												// ����
					break;
				}

				SendNtnetDt.SData236.FullNo2 = PPrmSS[S_P02][11];		// ���ԑ䐔�Q
				if (curnum_2 > PPrmSS[S_P02][11]) {
					SendNtnetDt.SData236.EmptyNo2 = 0;					// ��ԑ䐔�Q
				} else {
					SendNtnetDt.SData236.EmptyNo2 = PPrmSS[S_P02][11] - curnum_2;				// ��ԑ䐔�Q(���ԑ䐔 - ���ݒ��ԑ䐔)
				}
				if( SendNtnetDt.SData236.EmptyNo2 == 0 ){		// ��Ԃ��u�O�v�̏ꍇ
					SendNtnetDt.SData236.Full[1] = 1;			// �u���ԁv��ԃZ�b�g
				}
				switch (PPrmSS[S_P02][9]) {								// ��������ԃ��[�h1
				case	1:												// ��������
					SendNtnetDt.SData236.Full[1] = 1;					// �u�������ԁv��ԃZ�b�g
					break;
				case	2:												// �������
					SendNtnetDt.SData236.Full[1] = 0;					// �u������ԁv��ԃZ�b�g
					break;
				default:												// ����
					break;
				}

				if (i == 4) {
					SendNtnetDt.SData236.FullNo3 = PPrmSS[S_P02][15];	// ���ԑ䐔�R
					if (curnum_3 > PPrmSS[S_P02][15]) {
						SendNtnetDt.SData236.EmptyNo3 = 0;				// ��ԑ䐔�R
					} else {
						SendNtnetDt.SData236.EmptyNo3 = PPrmSS[S_P02][15] - curnum_3;			// ��ԑ䐔�R(���ԑ䐔 - ���ݒ��ԑ䐔)
					}
					if( SendNtnetDt.SData236.EmptyNo3 == 0 ){		// ��Ԃ��u�O�v�̏ꍇ
						SendNtnetDt.SData236.Full[2] = 1;			// �u���ԁv��ԃZ�b�g
					}
					switch (PPrmSS[S_P02][13]) {							// ��������ԃ��[�h1
					case	1:												// ��������
						SendNtnetDt.SData236.Full[2] = 1;					// �u�������ԁv��ԃZ�b�g
						break;
					case	2:												// �������
						SendNtnetDt.SData236.Full[2] = 0;					// �u������ԁv��ԃZ�b�g
						break;
					default:												// ����
						break;
					}
				}
				break;

			default:
				break;
			}
		}
		// ������"2"(�قږ���)�͖��g�p
	}
	if(_is_ntnet_remote()) {
		if (kind != 236) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_236 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_236), NTNET_BUF_NORMAL);
	}
}
/*[]----------------------------------------------------------------------[]*
 *| ���ԑ䐔�f�[�^���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data236
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-01-31
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_Data236(void)
{
	if( prm_get(COM_PRM,S_NTN,121,1,1)!=0 ) {
		memset(&SData58_bk, 0, sizeof(DATA_KIND_58));
		NTNET_Snd_DataParkCarNumWeb((uchar)58);
		return;
	}
	NTNET_Snd_DataParkCarNum((uchar)236);
}

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
///*[]----------------------------------------------------------------------[]*
// *| ���ԑ䐔�����f�[�^ ���M����
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_Snd_Data238
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| Author       : 
// *| Date         : 2006-02-02
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//void	NTNET_Snd_Data238(void)
//{
//	NTNET_Snd_DataParkCarNum((uchar)238);
//}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

/*[]----------------------------------------------------------------------[]*
 *| �Ǘ��f�[�^�v��NG ���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data105
 *| PARAMETER    : MachineNo:���M��[���@�B�� / code:�G���[�R�[�h
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data105(ulong MachineNo, ushort code)
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_105 ) );

	// ��{�ް��쐬
	BasicDataMake( 105, 1 );

	SendNtnetDt.SData105.TermNo = MachineNo;
	SendNtnetDt.SData105.Result = code;

	// �ް����M�o�^
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_105 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_105), NTNET_BUF_NORMAL);
	}
}

/*[]----------------------------------------------------------------------[]*
 *| �f�[�^�v���Q����NG ���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data110
 *| PARAMETER    : MachineNo:���M��[���@�B�� / code:�G���[�R�[�h
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data110(ulong MachineNo, ushort code)
{
	// ��{�ް��쐬
	BasicDataMake( 110, 1 );

	SendNtnetDt.SData110.SMachineNo = MachineNo;
	memcpy(SendNtnetDt.SData110.ControlData, RecvNtnetDt.RData109.ControlData, 
			sizeof(RecvNtnetDt.RData109.ControlData));
	SendNtnetDt.SData110.Result = code;

	// ��Q�[���̃^�[�~�i��No01�`32(all 0)
	memset(SendNtnetDt.SData110.ErrTerminal, 0, sizeof(SendNtnetDt.SData110.ErrTerminal));

	// �ް����M�o�^
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_110 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_110), NTNET_BUF_NORMAL);
	}
}
/*[]----------------------------------------------------------------------[]*
 *| �Z���^�p�f�[�^�v�����ʑ��M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data155
 *| PARAMETER    : MachineNo:���M��[���@�B�� / code:�G���[�R�[�h
 *| RETURN VALUE : void
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data155(ulong MachineNo, ushort code)
{
	// ��{�ް��쐬
	BasicDataMake( 155, 1 );

	SendNtnetDt.SData155.FmtRev = 10;							// �t�H�[�}�b�gRev.No.(10�Œ�)
	SendNtnetDt.SData155.SMachineNo = MachineNo;
	SendNtnetDt.SData155.SeisanJyoutai_Req = RecvNtnetDt.RData154.SeisanJyoutai_Req;
	SendNtnetDt.SData155.RealTime_Req = RecvNtnetDt.RData154.RealTime_Req;
	SendNtnetDt.SData155.Reserve1 = RecvNtnetDt.RData154.Reserve1;
	SendNtnetDt.SData155.Reserve2 = RecvNtnetDt.RData154.Reserve2;
	SendNtnetDt.SData155.Reserve3 = RecvNtnetDt.RData154.Reserve3;
	SendNtnetDt.SData155.Reserve4 = RecvNtnetDt.RData154.Reserve4;
	SendNtnetDt.SData155.Reserve5 = RecvNtnetDt.RData154.Reserve5;
	SendNtnetDt.SData155.Reserve6 = RecvNtnetDt.RData154.Reserve6;
	SendNtnetDt.SData155.SynchroTime_Req = RecvNtnetDt.RData154.SynchroTime_Req;
	SendNtnetDt.SData155.TermInfo_Req = RecvNtnetDt.RData154.TermInfo_Req;
	SendNtnetDt.SData155.Result = code;

	// ��Q�[���̃^�[�~�i��No01�`32(all 0)
	memset(SendNtnetDt.SData155.ErrTerminal, 0, sizeof(SendNtnetDt.SData155.ErrTerminal));

	// �ް����M�o�^
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_155));
	} else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_155), NTNET_BUF_NORMAL);
	}
}
/*[]----------------------------------------------------------------------[]*
 *| ���������ݒ�v�� ����(�ް����157)���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data157
 *| PARAMETER    : MachineNo:���M��[���@�B�� / code:�G���[�R�[�h
 *| RETURN VALUE : void
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data157(ulong MachineNo, ushort code , uchar setget )
{
	// ��{�ް��쐬
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_157 ) );
	BasicDataMake( 157, 1 );
	SendNtnetDt.SData157.FmtRev = 10;							// �t�H�[�}�b�gRev.No.(10�Œ�)
	SendNtnetDt.SData157.SMachineNo = MachineNo;
	SendNtnetDt.SData157.Reserve = 0;
	SendNtnetDt.SData157.Result = code;
	
	if( NTNET_SYNCHRO_GET == setget ){	// ���������f�[�^�̎Q��( NTNET_RevData154 )
		SendNtnetDt.SData157.Time_synchro_Req = 0;				// 0 = �ύX�Ȃ�
		SendNtnetDt.SData157.synchro_hour = (uchar)prm_get(COM_PRM, S_NTN, 122, 2, 3);
		SendNtnetDt.SData157.synchro_minu = (uchar)prm_get(COM_PRM, S_NTN, 122, 2, 1);
	} else {							// ���������f�[�^�̐ݒ�( NTNET_RevData156 )
		SendNtnetDt.SData157.Time_synchro_Req = ( ( code == 9 ) ? 0:1 );	// code = 9(���s)�̏ꍇ�A0�F�ύX�Ȃ��Acode = 0(����)�̏ꍇ�A1�F�ύX����
		SendNtnetDt.SData157.synchro_hour = RecvNtnetDt.RData156.synchro_hour;
		SendNtnetDt.SData157.synchro_minu = RecvNtnetDt.RData156.synchro_minu;
	}

	// �ް����M�o�^
	RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_157));
}
/************************************/
/*									*/
/************************************/
uchar	NTNET_GetMostErrCode(const uchar ErrMdl)
{
	uchar	ErrCode = 0;
	ushort	i;
	ushort	GetLevel;
	uchar	ErrLevel= 0;

	for (i=0; i<ERR_NUM_MAX; i++) {
		GetLevel = GetErrorOccuerdLevel(ErrMdl, i);	// see -> IsErrorOccuerd()
		//	ErrLevel������
		if ((uchar)GetLevel > ErrLevel) {
			ErrLevel = (uchar)GetLevel;
			ErrCode = i;
		}
	}
	return(ErrCode);
}
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
///*[]----------------------------------------------------------------------[]*
// *| ���Z���f�[�^���M����
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//void	NTNET_Snd_Data152(void)
//{
//	ushort	i;
//	ushort	j;
//	ushort	num;
//	uchar	ErrCode;		// ERRMDL_COIM, ERRMDL_NOTE, ERRMDL_READER��ʂ̺���
//	EMONEY	t_EMoney;
//	CREINFO t_Credit;
//
//	ST_OIBAN w_oiban;
//	
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_152 ) );
//
//	/************************/
//	/*	��{�ް��쐬		*/
//	/************************/
//	BasicDataMake( 152, 1 );					// �ް��ێ��׸� = 1�Œ�
//
//	SendNtnetDt.SData152.FmtRev = 10;			// ̫�ϯ�Rev.��
//	/************************************/
//	/*	�@���� = t_KikiStatus			*/		// NTNET_Edit_Data56_r10
//	/************************************/
//	if( opncls() == 1 ){	// �c�ƒ�
//		SendNtnetDt.SData152.Kiki.KyugyoStatus 	= 0;	// �@����_�c�x��	0:�c��
//	} else {				// �x�ƒ�
//		SendNtnetDt.SData152.Kiki.KyugyoStatus 	= 1;	// �@����_�c�x��	1:�x��
//	}
//	SendNtnetDt.SData152.Kiki.Lc1Status 		= 0;	// �@����_LC1���	0:OFF/1:ON
//	SendNtnetDt.SData152.Kiki.Lc2Status 		= 0;	// �@����_LC2���	0:OFF/1:ON
//	SendNtnetDt.SData152.Kiki.Lc3Status 		= 0;	// �@����_LC3���	0:OFF/1:ON
//	SendNtnetDt.SData152.Kiki.GateStatus 		= 0;	// �@����_�Q�[�g���	0:��/1:�J
//	/********************************************************************/
//	/*	AlmAct[ALM_MOD_MAX][ALM_NUM_MAX]��[2][0]��ð���(02:���֘A)���	*/
//	/*	00-99��ΏۂɍŌ�ɔ��������װ�									*/
//	/*	ALMMDL_SUB2		alm_chk2()	t_EAM_Act	AlmAct					*/
//	/********************************************************************/
//	SendNtnetDt.SData152.Kiki.NgCardRead 	= NgCard;	// NG�J�[�h�ǎ���e	0���Ȃ�, 1�`255��NT-NET�̃A���[�����02�̃R�[�h���e�Ɠ���
//	/****************************************************************************************/
//	/*	�������G���[ = t_Erroring															*/
//	/*	ErrAct[ERR_MOD_MAX][ERR_NUM_MAX]													*/
//	/*	IsErrorOccuerd()�֐��ɂē��Y�̴װ��ʂ��00-99��Ώۂɍŏ�ʂ̎�Ԃ̴װ���ނ�ݒ�	*/
//	/****************************************************************************************/
//	// ERRMDL_COIM		3.Coin
//	ErrCode = NTNET_GetMostErrCode(ERRMDL_COIM);
//	SendNtnetDt.SData152.Err.ErrCoinmech	= ErrCode;	// �R�C�����b�N	�G���[���03�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
//	// ERRMDL_NOTE		4.Note
//	ErrCode = NTNET_GetMostErrCode(ERRMDL_NOTE);
//	SendNtnetDt.SData152.Err.ErrNoteReader	= ErrCode;	// �������[�_�[	�G���[���04�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
//	// ERRMDL_READER	1.Reader
//	ErrCode = NTNET_GetMostErrCode(ERRMDL_READER);
//	SendNtnetDt.SData152.Err.ErrGikiReader	= ErrCode;	// ���C���[�_�[	�G���[���01�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
//	// �ȉ��AFT-4000 �ł͖��Ή�
//	SendNtnetDt.SData152.Err.ErrNoteHarai	= 0;		// �������o�@	�G���[���05�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
//	SendNtnetDt.SData152.Err.ErrCoinJyunkan	= 0;		// �R�C���z��	�G���[���06�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
//	SendNtnetDt.SData152.Err.ErrNoteJyunkan	= 0;		// �����z��		�G���[���14�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
//	/************************************/
//	/*									*/
//	/************************************/
//	if( PayInfo_Class == 4 ){
//		// �N���W�b�g���Z
//		SendNtnetDt.SData152.PayClass	= (uchar)0;					// �����敪
//	}else{
//		SendNtnetDt.SData152.PayClass	= (uchar)PayInfo_Class;		// �����敪
//	}
//	if(SendNtnetDt.SData152.PayClass == 8) {						// ���Z�O��
//		SendNtnetDt.SData152.MoneyInOut.MoneyKind_In = 0x1f;		// ����L��	���ڋ���̗L��
//		SendNtnetDt.SData152.MoneyInOut.MoneyKind_Out = 0x1f;		// ����L��	���ڋ���̗L��
//		SendNtnetDt.SData152.AntiPassCheck = 1;						// �A���`�p�X�`�F�b�N	1���`�F�b�NOFF
//		goto LSendPayInfo;
//	}
//	else if(SendNtnetDt.SData152.PayClass == 100 ||					// �����o��
//			SendNtnetDt.SData152.PayClass == 101 ||					// �s���o��
//			SendNtnetDt.SData152.PayClass == 102) {					// �t���b�v�㏸�O�o��
//		NTNET_Snd_Data152_SK();
//		goto LSendPayInfo;
//	}
//
//	w_oiban.w = ntNet_152_SaveData.Oiban.w;
//	w_oiban.i = ntNet_152_SaveData.Oiban.i;
//	SendNtnetDt.SData152.PayCount 	= CountSel( &w_oiban);	// ���Z�ǔ�(0�`99999)	syusei[LOCK_MAX]
//	SendNtnetDt.SData152.PayMethod 	= ntNet_152_SaveData.PayMethod;	// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
//	SendNtnetDt.SData152.PayMode 	= ntNet_152_SaveData.PayMode;	// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z/20��Mifare�v���y�C�h���Z)
//	SendNtnetDt.SData152.CMachineNo = 0;							// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
//
//	if(SendNtnetDt.SData152.PayClass == 9 ){								// ���Z��
//		num = ryo_buf.pkiti - 1;
//		SendNtnetDt.SData152.FlapArea 	= (ushort)LockInfo[num].area;		// �t���b�v�V�X�e��	���		0�`99
//		SendNtnetDt.SData152.FlapParkNo = (ushort)LockInfo[num].posi;		// 					�Ԏ��ԍ�	0�`9999
//	}
//	else if(SendNtnetDt.SData152.PayMethod != 5 ){							// ������X�V�ȊO
//		SendNtnetDt.SData152.FlapArea 	= (ushort)(ntNet_152_SaveData.WPlace/10000);	// �t���b�v�V�X�e��	���		0�`99
//		SendNtnetDt.SData152.FlapParkNo = (ushort)(ntNet_152_SaveData.WPlace%10000);	// 					�Ԏ��ԍ�	0�`9999
//		SendNtnetDt.SData152.KakariNo 	= (ushort)ntNet_152_SaveData.KakariNo;			// �W����	0�`9999
//		SendNtnetDt.SData152.OutKind 	= ntNet_152_SaveData.OutKind;					// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
//																			//  	   10���Q�[�g�J��/20��Mifare�v���y�C�h���Z�o��
//																			// 		   97�����b�N�J�E�t���b�v�㏸�O�����Z�o��/98�����O�^�C�����o��
//																			// 		   99���T�[�r�X�^�C�����o��
//	}
//	if(SendNtnetDt.SData152.PayClass == 9 ){								// ���Z��
//		SendNtnetDt.SData152.CountSet 	= 1;								// �ݎԃJ�E���g 1�����Ȃ�
//	}
//	else {
//		SendNtnetDt.SData152.CountSet 	= ntNet_152_SaveData.CountSet;		// �ݎԃJ�E���g	0������(+1)/1�����Ȃ�/2������(-1)
//	}
//	/********************************************/
//	/*	����/�O�񐸎Z_YMDHMS = t_InPrevYMDHMS	*/
//	/********************************************/
//	if( SendNtnetDt.SData152.PayMethod != 5 ){					// ������X�V�ȊO
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Year		= (uchar)(ntNet_152_SaveData.carInTime.year%100);	// ����	�N		00�`99 2000�`2099
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Mon		= ntNet_152_SaveData.carInTime.mon;					// 		��		01�`12
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Day		= ntNet_152_SaveData.carInTime.day;					// 		��		01�`31
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Hour		= ntNet_152_SaveData.carInTime.hour;				// 		��		00�`23
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Min		= ntNet_152_SaveData.carInTime.min;					// 		��		00�`59
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Sec		= 0;												// 		�b		00�`59(���C�J�[�h��0�Œ�)
//		// �O�񐸎Z
//	}
//	/************************************/
//	/*									*/
//	/************************************/
//	SendNtnetDt.SData152.ReceiptIssue 	= ntNet_152_SaveData.ReceiptIssue;	// �̎��ؔ��s�L��	0���̎��؂Ȃ�/1���̎��؂���
//	if(SendNtnetDt.SData152.PayMethod != 5 ){					// ������X�V�ȊO
//		SendNtnetDt.SData152.Syubet 	= ntNet_152_SaveData.Syubet;	// �������			1�`
//		SendNtnetDt.SData152.Price 		= ntNet_152_SaveData.Price;		// ���ԗ���			0�`
//	}
//	SendNtnetDt.SData152.CashPrice 		= ntNet_152_SaveData.CashPrice;	// ��������			0�`
//	SendNtnetDt.SData152.InPrice 		= ntNet_152_SaveData.InPrice;	// �������z			0�`
//	SendNtnetDt.SData152.ChgPrice 		= (ushort)ntNet_152_SaveData.ChgPrice;	// �ޑK���z			0�`9999
//
//	if(SendNtnetDt.SData152.PayClass == 9 ){									// ���Z��
//		SendNtnetDt.SData152.Syubet 	= (char)(ryo_buf.syubet + 1);			// �������			1�`
//		if( ryo_buf.ryo_flg < 2 ) {												// ���Ԍ����Z����
//			SendNtnetDt.SData152.Price = ryo_buf.tyu_ryo;						// ���ԗ���
//		}
//		else {
//			SendNtnetDt.SData152.Price = ryo_buf.tei_ryo;						// �������
//		}
//		SendNtnetDt.SData152.InPrice 	= ryo_buf.nyukin;						// �������z			0�`
//		SendNtnetDt.SData152.ChgPrice 	= (ushort)ryo_buf.turisen;				// �ޑK���z			0�`9999
//	}
//	/****************************************************/
//	/*	���K���(����������/���o������) = t_MoneyInOut	*/
//	/****************************************************/
//	SendNtnetDt.SData152.MoneyInOut.MoneyKind_In = 0x1f;						// ����L��	���ڋ���̗L��
//																				//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
//																				//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
//	if(SendNtnetDt.SData152.PayClass == 0 || 									// �����敪 ���Z��
//		SendNtnetDt.SData152.PayClass == 2 || 									// �����敪 ���Z���~
//		SendNtnetDt.SData152.PayClass == 3) { 									// �����敪 �Đ��Z���~
//		SendNtnetDt.SData152.MoneyInOut.In_10_cnt    = (uchar)ntNet_152_SaveData.in_coin[0];	// ����������(10�~)		0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_50_cnt    = (uchar)ntNet_152_SaveData.in_coin[1];	// ����������(50�~)		0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_100_cnt   = (uchar)ntNet_152_SaveData.in_coin[2];	// ����������(100�~)	0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_500_cnt   = (uchar)ntNet_152_SaveData.in_coin[3];	// ����������(500�~)	0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_1000_cnt  = (uchar)ntNet_152_SaveData.in_coin[4]	// ����������(1000�~)	0�`255
//														+ ntNet_152_SaveData.f_escrow;		// �G�X�N�������ԋp����+1����
//		SendNtnetDt.SData152.MoneyInOut.In_2000_cnt  = 0;							// ����������(2000�~)	0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_5000_cnt  = 0;							// ����������(5000�~)	0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_10000_cnt = 0;							// ����������(10000�~)	0�`255
//	}
//	else if(SendNtnetDt.SData152.PayClass == 9) {								// �����敪 ���Z��
//		SendNtnetDt.SData152.MoneyInOut.In_10_cnt    = (uchar)ryo_buf.in_coin[0];	// ����������(10�~)		0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_50_cnt    = (uchar)ryo_buf.in_coin[1];	// ����������(50�~)		0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_100_cnt   = (uchar)ryo_buf.in_coin[2];	// ����������(100�~)	0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_500_cnt   = (uchar)ryo_buf.in_coin[3];	// ����������(500�~)	0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_1000_cnt  = (uchar)ryo_buf.in_coin[4];	// ����������(1000�~)	0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_2000_cnt  = 0;							// ����������(2000�~)	0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_5000_cnt  = 0;							// ����������(5000�~)	0�`255
//		SendNtnetDt.SData152.MoneyInOut.In_10000_cnt = 0;							// ����������(10000�~)	0�`255
//	}
//	SendNtnetDt.SData152.MoneyInOut.MoneyKind_Out = 0x1f;						// ����L��	���ڋ���̗L��
//																				//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
//																				//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
//	if(SendNtnetDt.SData152.PayClass == 0 || 									// �����敪 ���Z��
//		SendNtnetDt.SData152.PayClass == 2 || 									// �����敪 ���Z���~
//		SendNtnetDt.SData152.PayClass == 3) { 									// �����敪 �Đ��Z���~
//		SendNtnetDt.SData152.MoneyInOut.Out_10_cnt    = (uchar)ntNet_152_SaveData.out_coin[0];	// ���o������(10�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_50_cnt    = (uchar)ntNet_152_SaveData.out_coin[1];	// ���o������(50�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_100_cnt   = (uchar)ntNet_152_SaveData.out_coin[2];	// ���o������(100�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_500_cnt   = (uchar)ntNet_152_SaveData.out_coin[3];	// ���o������(500�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_1000_cnt  = ntNet_152_SaveData.f_escrow;			// ���o������(1000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_2000cnt   = 0;							// ���o������(2000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_5000_cnt  = 0;							// ���o������(5000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_10000_cnt = 0;							// ���o������(10000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//	}
//	else if(SendNtnetDt.SData152.PayClass == 9) {								// �����敪 ���Z��
//		SendNtnetDt.SData152.MoneyInOut.Out_10_cnt    = (uchar)ryo_buf.out_coin[0]	// ���o������(10�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
//															+ (uchar)ryo_buf.out_coin[4];
//		SendNtnetDt.SData152.MoneyInOut.Out_50_cnt    = (uchar)ryo_buf.out_coin[1]	// ���o������(50�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
//															+ (uchar)ryo_buf.out_coin[5];
//		SendNtnetDt.SData152.MoneyInOut.Out_100_cnt   = (uchar)ryo_buf.out_coin[2]	// ���o������(100�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//															+ (uchar)ryo_buf.out_coin[6];
//		SendNtnetDt.SData152.MoneyInOut.Out_500_cnt   = (uchar)ryo_buf.out_coin[3];	// ���o������(500�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_1000_cnt  = 0;							// ���o������(1000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_2000cnt   = 0;							// ���o������(2000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_5000_cnt  = 0;							// ���o������(5000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//		SendNtnetDt.SData152.MoneyInOut.Out_10000_cnt = 0;							// ���o������(10000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
//	}
//	/************************************/
//	/*									*/
//	/************************************/
//	SendNtnetDt.SData152.HaraiModoshiFusoku = (ushort)ntNet_152_SaveData.HaraiModoshiFusoku;			// ���ߕs���z	0�`9999
//	SendNtnetDt.SData152.Reserve1 = 0;											// �\��(�T�C�Y�����p)	0
//	if( SendNtnetDt.SData152.PayClass != 8 &&									// ���Z�O�ȊO
//		SendNtnetDt.SData152.PayClass != 9) {									// ���Z���ȊO
//		SendNtnetDt.SData152.AntiPassCheck = ntNet_152_SaveData.AntiPassCheck;	// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�NOFF/2������OFF
//	}
//	else {																		// ���Z�Oor���Z��
//		SendNtnetDt.SData152.AntiPassCheck = 1;									// �A���`�p�X�`�F�b�N	1���`�F�b�NOFF
//	}
//																				//						��������g�p���Ȃ��ꍇ�́u�P���`�F�b�NOFF�v�Œ�
//	if( (SendNtnetDt.SData152.PayClass != 9 && ntNet_152_SaveData.ParkNoInPass) || 		// ���Z���ȊO ��������Z����
//		(SendNtnetDt.SData152.PayClass == 9 && ryo_buf.ryo_flg >= 2) ) {		// ���Z�� ��������Z����
//		if(SendNtnetDt.SData152.PayClass != 9) {
//			SendNtnetDt.SData152.ParkNoInPass = ntNet_152_SaveData.ParkNoInPass;		// ������@���ԏꇂ	0�`999999
//		}
//		else {
//			SendNtnetDt.SData152.ParkNoInPass = CPrmSS[S_SYS][ntNet_152_SaveData.pkno_syu+1];	// ������@���ԏꇂ	0�`999999
//		}
//		/****************************************/
//		/*	���Z�}�̏��1, 2 = t_MediaInfo		*/
//		/****************************************/
//		SendNtnetDt.SData152.Media[0].MediaKind = (ushort)(ntNet_152_SaveData.pkno_syu + 2);	// ���Z�}�̏��P�@���(���C���}��)	0�`99
//		// �J�[�h�ԍ�[30]
//		intoasl(SendNtnetDt.SData152.Media[0].MediaCardNo, ntNet_152_SaveData.teiki_id, 5);				// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		// �J�[�h���[16]
//		intoasl(SendNtnetDt.SData152.Media[0].MediaCardInfo, ntNet_152_SaveData.teiki_syu, 2);	// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		// ���Z�}�̏��2�i�T�u�}�́j�� �I�[���O�Ƃ���
//	}
//	/************************************/
//	/*									*/
//	/************************************/
//	if( ntNet_152_SaveData.e_pay_kind != 0 ){									// �d�q���ώ�� Suica:1, Edy:2
//		SendNtnetDt.SData152.CardKind = (ushort)2;								// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//	}else if( PayInfo_Class == 4 ){												// �N���W�b�g���Z
//		SendNtnetDt.SData152.CardKind = (ushort)1;								// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//	}
//
//	switch( SendNtnetDt.SData152.CardKind ){									// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//	case 1:
//		SendNtnetDt.SData152.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// �Í�������
//		SendNtnetDt.SData152.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// �Í����ԍ�
//		memset( &t_Credit, 0, sizeof( t_Credit ) );
//		t_Credit.amount = (ulong)ntNet_152_SaveData.c_pay_ryo;					// ���ϊz
//		memcpyFlushLeft( &t_Credit.card_no[0], &ntNet_152_SaveData.c_Card_No[0], sizeof(t_Credit.card_no), sizeof(ntNet_152_SaveData.c_Card_No) );	// ����ԍ�
//		memcpyFlushLeft( &t_Credit.cct_num[0], &ntNet_152_SaveData.c_cct_num[0],
//							sizeof(t_Credit.cct_num), CREDIT_TERM_ID_NO_SIZE );	// �[�����ʔԍ�
//		memcpyFlushLeft( &t_Credit.kid_code[0], &ntNet_152_SaveData.c_kid_code[0], sizeof(t_Credit.kid_code), sizeof(ntNet_152_SaveData.c_kid_code) );	// KID �R�[�h
//		t_Credit.app_no = ntNet_152_SaveData.c_app_no;							// ���F�ԍ�
//		t_Credit.center_oiban = 0;												// ���������ǔ�
//		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );				// AES �Í���
//		memcpy( (uchar *)&SendNtnetDt.SData152.settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// ���Ϗ��
//		break;
//	case 2:
//		SendNtnetDt.SData152.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// �Í�������
//		SendNtnetDt.SData152.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// �Í����ԍ�
//		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//		t_EMoney.amount = ntNet_152_SaveData.e_pay_ryo;							// �d�q���ϐ��Z���@���ϊz
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//		memcpy( &t_EMoney.card_id[0], &ntNet_152_SaveData.e_Card_ID[0],
//				sizeof(ntNet_152_SaveData.e_Card_ID) );						// �d�q���ϐ��Z���@����ID (Ascii 16��)
//		t_EMoney.card_zangaku = ntNet_152_SaveData.e_pay_after;					// �d�q���ϐ��Z���@���ό�c��
//		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );				// AES �Í���
//		memcpy( (uchar *)&SendNtnetDt.SData152.settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// ���Ϗ��
//		break;
//	}
//	/************************************/
//	/*	���� = t_SeisanDiscount			*/
//	/************************************/
//	for( i = j = 0; i < WTIK_USEMAX; i++ ){								// ���Z���~�ȊO�̊������R�s�[
//		if(( ntNet_152_SaveData.DiscountData[i].DiscSyu != 0 ) &&					// ������ʂ���
//		   (( ntNet_152_SaveData.DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// ���Z���~�������łȂ�
//		    ( NTNET_CFRE < ntNet_152_SaveData.DiscountData[i].DiscSyu ))){
//		    if( ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA�ȊO(�ԍ�)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(�x���z�E�c�z)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO�ȊO(�ԍ�)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(�x���z�E�c�z)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY�ȊO	(�ԍ�)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(�x���z�E�c�z)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA�ȊO(�ԍ�)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(�x���z�E�c�z)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&	// IC-Card�ȊO(�ԍ�)
//		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICCARD_2 ) 	//			(�x���z�E�c�z)
//		    {
//				SendNtnetDt.SData152.SDiscount[j].ParkingNo	= ntNet_152_SaveData.DiscountData[i].ParkingNo;		// ���ԏ�No.
//				SendNtnetDt.SData152.SDiscount[j].Kind		= ntNet_152_SaveData.DiscountData[i].DiscSyu;		// �������
//				SendNtnetDt.SData152.SDiscount[j].Group		= ntNet_152_SaveData.DiscountData[i].DiscNo;		// �����敪
//				SendNtnetDt.SData152.SDiscount[j].Callback	= ntNet_152_SaveData.DiscountData[i].DiscCount;		// �������
//				SendNtnetDt.SData152.SDiscount[j].Amount	= ntNet_152_SaveData.DiscountData[i].Discount;		// �����z
//				SendNtnetDt.SData152.SDiscount[j].Info1		= ntNet_152_SaveData.DiscountData[i].DiscInfo1;		// �������1
//				if( SendNtnetDt.SData152.SDiscount[j].Kind != NTNET_PRI_W ){
//					SendNtnetDt.SData152.SDiscount[j].Info2 = ntNet_152_SaveData.DiscountData[i].uDiscData.common.DiscInfo2;	// �������2
//				}
//				j++;
//		    }
//		}
//	}
//	/********************/
//	/*	�ް����M�o�^	*/
//	/********************/
//LSendPayInfo:
//	if(_is_ntnet_remote()) {
//		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_152));
//	} else {
//		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_152), NTNET_BUF_NORMAL);
//	}
//}
//
///*[]----------------------------------------------------------------------[]*
// *| ���Z���f�[�^�s���o�ɁE�����o�ɁE�t���b�v�㏸�O���M����
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//static	void	NTNET_Snd_Data152_SK(void)
//{
//	SendNtnetDt.SData152.PayClass 	= 0;							// �����敪(0�����Z/1���Đ��Z/2�����Z���~/3���Đ��Z���~/(4���N���W�b�g���Z��))
//	SendNtnetDt.SData152.PayCount 	= CountSel(&PayInfoData_SK.Oiban);	// �����o��or�s���o�ɒǔ�or�t���b�v�㏸�O�i���Z�ǔԁj
//	SendNtnetDt.SData152.PayMethod 	= 0;							// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
//	SendNtnetDt.SData152.PayMode 	= 0;							// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z/20��Mifare�v���y�C�h���Z)
//	SendNtnetDt.SData152.CMachineNo = 0;							// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
//
//	SendNtnetDt.SData152.FlapArea 	= (ushort)(PayInfoData_SK.WPlace/10000);	// �t���b�v�V�X�e��	���		0�`99
//	SendNtnetDt.SData152.FlapParkNo = (ushort)(PayInfoData_SK.WPlace%10000);	// 					�Ԏ��ԍ�	0�`9999
//	SendNtnetDt.SData152.KakariNo 	= PayInfoData_SK.KakariNo;		// �W����	0�`9999
//	SendNtnetDt.SData152.OutKind 	= PayInfoData_SK.OutKind;		// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
//	SendNtnetDt.SData152.CountSet	= PayInfoData_SK.CountSet;		// �ݎԶ���-1����
//
//	/********************************************/
//	/*	����/�O�񐸎Z_YMDHMS = t_InPrevYMDHMS	*/
//	/********************************************/
//	if( SendNtnetDt.SData152.PayMethod != 5 ){					// ������X�V�ȊO
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Year	= 
//										(uchar)(PayInfoData_SK.TInTime.Year % 100);	// ����	�N		00�`99 2000�`2099
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Mon	= PayInfoData_SK.TInTime.Mon;	// 		��		01�`12
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Day	= PayInfoData_SK.TInTime.Day;	// 		��		01�`31
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Hour	= PayInfoData_SK.TInTime.Hour;	// 		��		00�`23
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Min	= PayInfoData_SK.TInTime.Min;	// 		��		00�`59
//		SendNtnetDt.SData152.InPrev_ymdhms.In_Sec	= 0;							// 		�b		00�`59(���C�J�[�h��0�Œ�)
//	}
//
//	SendNtnetDt.SData152.Syubet 	= PayInfoData_SK.syu;					// �������			1�`
//	SendNtnetDt.SData152.Price 		= PayInfoData_SK.WPrice;				// ���ԗ���			0�`
//
//	SendNtnetDt.SData152.MoneyInOut.MoneyKind_In = 0x1f;					// ����L��	���ڋ���̗L��
//																			//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
//																			//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
//	SendNtnetDt.SData152.MoneyInOut.MoneyKind_Out = 0x1f;					// ����L��	���ڋ���̗L��
//																			//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
//																			//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
//	SendNtnetDt.SData152.AntiPassCheck = PayInfoData_SK.PassCheck;			// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�NOFF/2������OFF
//																			//						��������g�p���Ȃ��ꍇ�́u�P���`�F�b�NOFF�v�Œ�
//
//}
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�

/*[]----------------------------------------------------------------------[]*
 *| ���A���^�C����񑗐M����
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data153(void)
{
	uchar	Kan0001_6;
	PARKCAR_DATA11	NowParkcarData;
	ushort	coinNowPoint;
	ushort	coinFullPoint;
	ushort	coinPerPoint;
	uchar	i;
	ushort	shu, mai;
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	uchar	f_update = 0;
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�

	memset( &NowParkcarData, 0, sizeof(PARKCAR_DATA11) );
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_153 ) );

	/************************/
	/*	��{�ް��쐬		*/
	/************************/
	BasicDataMake( 153, 1 );					// �ް��ێ��׸� = 1�Œ�

// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
//	SendNtnetDt.SData153.FmtRev = 10;			// ̫�ϯ�Rev.��	10�Œ� ---> Phase1�̑����ް��̒l��0�Œ�ł���
	SendNtnetDt.SData153.FmtRev = 11;			// ̫�ϯ�Rev.��	11�Œ�
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	/************************************/
	/*	���ԑ䐔 = PARKCAR_DATA11		*/
	/************************************/
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	NTNET_Snd_ParkCarNumDataMk(&SendNtnetDt.SData153.ParkData ,0);
	// ���ԑ䐔1�`����3�����Ԃ�0�Œ�Ƃ���
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	/************************************/
	/*	���Z�@���K��� = t_MacMoneyInfo	*/
	/************************************/
	SendNtnetDt.SData153.MoneyInfo[0].ModelCode = NTNET_MODEL_CODE;				// �@��R�[�h(001�`999) 0=���Ȃ�
	SendNtnetDt.SData153.MoneyInfo[0].MachineNo = prm_get(COM_PRM, S_PAY, 2, 2, 1);		// �@�B��
	SendNtnetDt.SData153.MoneyInfo[0].Result 	= 0;							// �v������(0������A1�������Ȃ��A2���^�C���A�E�g)
	SendNtnetDt.SData153.MoneyInfo[0].Uriage 	= sky.tsyuk.Uri_Tryo;			// ������z				0�`999999
	SendNtnetDt.SData153.MoneyInfo[0].Cash 		= sky.tsyuk.Genuri_Tryo;		// ����������z			0�`999999
	
// MH322917(S) A.Iiizumi 2018/12/04 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����
		f_update = date_uriage_update_chk();
		if((f_update == DATE_URIAGE_CLR_TODAY)||(f_update == DATE_URIAGE_CLR_BEFORE)){
			// ���t�͕ς�������܂������W�v����Ă��Ȃ���ԁi�O���̏��̂܂܁j�W�v���Z���ɍX�V����邽�ߔ��肪�K�v
			SendNtnetDt.SData153.MoneyInfo[0].Date_Uriage_use = 1;									// ���t�ؑ֎���� �g�p�L�� 1���g�p
			// ���t�ؑ֎���� ������z      �����̃f�[�^�͂Ȃ�����0���Z�b�g
			SendNtnetDt.SData153.MoneyInfo[0].Date_Uriage = 0;
			// ���t�ؑ֎���� ����������z  �����̃f�[�^�͂Ȃ�����0���Z�b�g
			SendNtnetDt.SData153.MoneyInfo[0].Date_Cash = 0;
			SendNtnetDt.SData153.MoneyInfo[0].Date_hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// ���t�ؑ֎���� �ؑ֎� 0�`23
			SendNtnetDt.SData153.MoneyInfo[0].Date_min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// ���t�ؑ֎���� �ؑ֕� 0�`12
		}else{
			SendNtnetDt.SData153.MoneyInfo[0].Date_Uriage_use = 1;									// ���t�ؑ֎���� �g�p�L�� 1���g�p
			SendNtnetDt.SData153.MoneyInfo[0].Date_Uriage = Date_Syoukei.Uri_Tryo;					// ���t�ؑ֎���� ������z      0�`999999
			SendNtnetDt.SData153.MoneyInfo[0].Date_Cash = Date_Syoukei.Genuri_Tryo;					// ���t�ؑ֎���� ����������z  0�`999999
			SendNtnetDt.SData153.MoneyInfo[0].Date_hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// ���t�ؑ֎���� �ؑ֎� 0�`23
			SendNtnetDt.SData153.MoneyInfo[0].Date_min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// ���t�ؑ֎���� �ؑ֕� 0�`12
		}
	}
// MH322917(E) A.Iiizumi 2018/12/04 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	// ���ɓ������z���Z
	SendNtnetDt.SData153.MoneyInfo[0].CoinTotal = (ulong)SFV_DAT.safe_dt[0] * 10L;
	SendNtnetDt.SData153.MoneyInfo[0].CoinTotal += (ulong)SFV_DAT.safe_dt[1] * 50L;
	SendNtnetDt.SData153.MoneyInfo[0].CoinTotal += (ulong)SFV_DAT.safe_dt[2] * 100L;
	SendNtnetDt.SData153.MoneyInfo[0].CoinTotal += (ulong)SFV_DAT.safe_dt[3] * 500L;
	
	SendNtnetDt.SData153.MoneyInfo[0].NoteTotal = (ulong)SFV_DAT.nt_safe_dt * 1000L;	// �������ɑ������z		0�`999999

	// �R�C�����Ɏ��[��(%)�̃Z�b�g
	coinNowPoint = (SFV_DAT.safe_dt[0] * prm_get(COM_PRM, S_MON, 20, 2, 1)) +
					(SFV_DAT.safe_dt[1] * prm_get(COM_PRM, S_MON, 21, 2, 1)) +
					(SFV_DAT.safe_dt[2] * prm_get(COM_PRM, S_MON, 22, 2, 1)) +
					(SFV_DAT.safe_dt[3] * prm_get(COM_PRM, S_MON, 23, 2, 1));
	coinFullPoint = prm_get(COM_PRM, S_MON, 27, 5, 1);
	if(coinNowPoint < coinFullPoint) {
		coinPerPoint = (coinNowPoint * 100) / coinFullPoint;
		SendNtnetDt.SData153.MoneyInfo[0].CoinReceive = coinPerPoint;
	}
	else {
		SendNtnetDt.SData153.MoneyInfo[0].CoinReceive = 100;
	}
	// �������Ɏ��[�����̃Z�b�g
	SendNtnetDt.SData153.MoneyInfo[0].NoteReceive = SFV_DAT.nt_safe_dt;

	Kan0001_6 = (ushort)prm_get(COM_PRM, S_KAN, 1, 1, 1);		//	20_0001�E
	if ((Kan0001_6 == 1) || (Kan0001_6 == 2)) {
		SendNtnetDt.SData153.MoneyInfo[0].KinsenKanriFlag = 0;	// ���K�Ǘ�����
	} else {
		SendNtnetDt.SData153.MoneyInfo[0].KinsenKanriFlag = 1;	// ���K�Ǘ��Ȃ�
	}
	//	CNM_REC_REC   CN_RDAT	--->	�ޑK�؂��Ԃ�1000�~�͂Ȃ�
	if( CN_RDAT.r_dat09[0] == 0 ){								// �ޑK�؂��� 10�~�Ȃ�
		SendNtnetDt.SData153.MoneyInfo[0].TuriStatus |= 0x01;
	}
	if( CN_RDAT.r_dat09[1] == 0 ){								// �ޑK�؂��� 50�~�Ȃ�
		SendNtnetDt.SData153.MoneyInfo[0].TuriStatus |= 0x02;
	}
	if( CN_RDAT.r_dat09[2] == 0 ){								// �ޑK�؂��� 100�~�Ȃ�
		SendNtnetDt.SData153.MoneyInfo[0].TuriStatus |= 0x04;
	}
	if( CN_RDAT.r_dat09[3] == 0 ){								// �ޑK�؂��� 500�~�Ȃ�
		SendNtnetDt.SData153.MoneyInfo[0].TuriStatus |= 0x08;
	}

	SendNtnetDt.SData153.MoneyInfo[0].TuriMai_10 	= turi_kan.turi_dat[0].gen_mai;	// 10�~�ۗL����
	SendNtnetDt.SData153.MoneyInfo[0].TuriMai_50 	= turi_kan.turi_dat[1].gen_mai;	// 50�~�ۗL����
	SendNtnetDt.SData153.MoneyInfo[0].TuriMai_100	= turi_kan.turi_dat[2].gen_mai;	// 100�~�ۗL����
	SendNtnetDt.SData153.MoneyInfo[0].TuriMai_500 	= turi_kan.turi_dat[3].gen_mai;	// 500�~�ۗL����
	// �\�~1, 2
	turikan_subtube_set();
	for( i=0; i<2; i++ ){
		switch( ((turi_kan.sub_tube >> (i*8)) & 0x000F) ){
		case 0x01:
			shu = 10;
			mai = turi_kan.turi_dat[0].ygen_mai;
			break;
		case 0x02:
			shu = 50;
			mai = turi_kan.turi_dat[1].ygen_mai;
			break;
		case 0x04:
			shu = 100;
			mai = turi_kan.turi_dat[2].ygen_mai;
			break;
		case 0:
		default:
			shu = 0;
			mai = 0;
			break;
		}
		if( i == 0 ){
			SendNtnetDt.SData153.MoneyInfo[0].Yochiku1_shu = shu;	// ������z	0���񓋍ځ@10/50/100/500/1000/2000/5000/10000
			SendNtnetDt.SData153.MoneyInfo[0].Yochiku1_mai = mai;	// �ۗL����	0�`9999
		} else {
			SendNtnetDt.SData153.MoneyInfo[0].Yochiku2_shu = shu;	// ������z	0���񓋍ځ@10/50/100/500/1000/2000/5000/10000
			SendNtnetDt.SData153.MoneyInfo[0].Yochiku2_mai = mai;	// �ۗL����	0�`9999
		}
	}
	// �\�~3, 4, 5
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku3_shu = 0;		// ������z	0���񓋍ځ@10/50/100/500/1000/2000/5000/10000
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku3_mai = 0;		// �ۗL����	0�`9999
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku4_shu = 0;		// ������z	0���񓋍ځ@10/50/100/500/1000/2000/5000/10000
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku4_mai = 0;		// �ۗL����	0�`9999
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku5_shu = 0;		// ������z	0���񓋍ځ@10/50/100/500/1000/2000/5000/10000
	SendNtnetDt.SData153.MoneyInfo[0].Yochiku5_mai = 0;		// �ۗL����	0�`9999
	//	���Z�@�Q��� - ���Z�@�R�Q����0�ő��M����
	/********************/
	/*	�ް����M�o�^	*/
	/********************/
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_153));
	} else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_153), NTNET_BUF_NORMAL);
	}
}

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
///*[]----------------------------------------------------------------------[]*
// *| ���u�f�[�^�v������NG ���M����
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_Snd_Data244
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| Author       : 
// *| Date         : 2006-02-02
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
//void	NTNET_Snd_Data244(ushort code)
//{
//	// ��{�ް��쐬
//	BasicDataMake( 244, 1 );
//
//	SendNtnetDt.SData244.GroupNo = RecvNtnetDt.RData243.GroupNo;
//	memcpy(SendNtnetDt.SData244.ControlData, RecvNtnetDt.RData243.ControlData, 
//			sizeof(RecvNtnetDt.RData243.ControlData));
//	SendNtnetDt.SData244.Result = code;
//
//	// ��Q�[���̃^�[�~�i��No01�`32(all 0)
//	memset(SendNtnetDt.SData244.MachineNo, 0, sizeof(SendNtnetDt.SData244.MachineNo));
//
//	// �ް����M�o�^
//	if(_is_ntnet_remote()) {
//		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_244 ));
//	}
//	else {
//		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_244), NTNET_BUF_NORMAL);
//	}
//}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

/*[]----------------------------------------------------------------------[]*
 *| �S�Ԏ����f�[�^ ���M����
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data245
 *| PARAMETER    : GroupNo : �O���[�vNo
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2006-02-02
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data245(ushort GroupNo)
{
	short	i;
	RYO_INFO	*ryo_info;
	ushort	len = 0;

	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_245));

	BasicDataMake(245, 1);											// ��{�ް��쐬

	SendNtnetDt.SData245.GroupNo = GroupNo;							// �O���[�vNo
	len = sizeof(DATA_BASIC) + 2;	// ��{�f�[�^�{�O���[�vNo
	// �L���ȎԎ������[�v����
	i = 0;
	ryo_info = &RyoCalSim.RyoInfo[0];
	while (ryo_info[i].pr_lokno) {
		WACDOG;																	// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s

		SendNtnetDt.SData245.LockState[i].LockNo = ryo_info[i].op_lokno;		// �����
		SendNtnetDt.SData245.LockState[i].NowState = ryo_info[i].nstat;			// ���ݽð��
		SendNtnetDt.SData245.LockState[i].Year = ryo_info[i].indate.year;		// ���ɔN
		SendNtnetDt.SData245.LockState[i].Mont = ryo_info[i].indate.mont;		// ���Ɍ�
		SendNtnetDt.SData245.LockState[i].Date = ryo_info[i].indate.date;		// ���ɓ�
		SendNtnetDt.SData245.LockState[i].Hour = ryo_info[i].indate.hour;		// ���Ɏ�
		SendNtnetDt.SData245.LockState[i].Minu = ryo_info[i].indate.minu;		// ���ɕ�
		SendNtnetDt.SData245.LockState[i].Syubet = ryo_info[i].kind;			// �������
		SendNtnetDt.SData245.LockState[i].TyuRyo = ryo_info[i].fee;				// ���ԗ���

		i++;
		len += sizeof(LOCK_STATE);
		if (i >= OLD_LOCK_MAX) {
			break;
		}
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, len);
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, len, NTNET_BUF_NORMAL);	// �ް����M�o�^
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| �n�b�s�[�}���f�[�`�F�b�N                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : chk_hpmonday                                            |*/
/*| PARAMETER    : date                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2006-09-28                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	chk_hpmonday(t_splday_yobi date)
{
	if ((date.mont < 1) || (date.mont > 12)) return FALSE;
	if ((date.week < 1) || (date.week > 5))  return FALSE;
	if (date.yobi != 1)                      return FALSE;
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*| T���v�W�v�f�[�^�쐬����                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_TGOUKEI                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_TGOUKEI(void)
{
}

/*[]----------------------------------------------------------------------[]*/
/*| T���v�W�v�f�[�^�쐬����                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_TSYOUKEI                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_TSYOUKEI(void)
{
	memset(&NTNetTTotalTime,0,sizeof(NTNetTTotalTime));
	//�W�v������ۑ��B�ЂƂ̏W�v�Q�Ŏ������قȂ�Ȃ��悤�ɏC���BBasicDataMake()�݂̂Ŏg�p�����B
	NTNetTTotalTime.Year = CLK_REC.year;	// �N
	NTNetTTotalTime.Mon  = CLK_REC.mont;	// ��
	NTNetTTotalTime.Day  = CLK_REC.date;	// ��
	NTNetTTotalTime.Hour = CLK_REC.hour;	// ��
	NTNetTTotalTime.Min  = CLK_REC.minu;	// ��
	NTNetTTotalTime.Sec  = CLK_REC.seco;	// �b

	_NTNET_Snd_Data158(&sky.tsyuk);
	_NTNET_Snd_Data159(&sky.tsyuk);
	_NTNET_Snd_Data160(&sky.tsyuk);
	_NTNET_Snd_Data161(&sky.tsyuk);
	_NTNET_Snd_Data162(&sky.tsyuk);
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	_NTNET_Snd_Data163(&loktl.tloktl);
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
	_NTNET_Snd_Data164(&sky.tsyuk);
	_NTNET_Snd_Data169(&sky.tsyuk);
}

/*[]----------------------------------------------------------------------[]*/
/*| �W�v��{�f�[�^�쐬����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiKihon                                   |*/
/*| PARAMETER    : syukei    : �W�v�f�[�^(���f�[�^)                        |*/
/*|                ID        : �f�[�^ID                                    |*/
/*|                Save      : �f�[�^�ێ��t���O                            |*/
/*|                Type      : �W�v�^�C�v                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiKihon(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_30 *data = &SendNtnetDt.SData30;
	
	memset(data, 0, sizeof(DATA_KIND_30));
	
	BasicDataMake(ID, Save);										// ��{�ް��쐬
	data->Type					= Type;								// �W�v�^�C�v
	data->KakariNo				= syukei->Kakari_no;				// �W��No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// �W�v�ǔ�
	ntnet_DateTimeCnv(&data->NowTime, &syukei->NowTime);			// ����W�v
	ntnet_DateTimeCnv(&data->LastTime, &syukei->OldTime);			// �O��W�v
	data->SettleNum				= syukei->Seisan_Tcnt;				// �����Z��
	data->Kakeuri				= syukei->Kakeuri_Tryo;				// ���|���z
	data->Cash					= syukei->Genuri_Tryo;				// ����������z
	data->Uriage				= syukei->Uri_Tryo;					// ������z
	data->Tax					= syukei->Tax_Tryo;					// ������Ŋz
	data->Charge				= syukei->Turi_modosi_ryo;			// �ޑK���ߊz
	data->CoinTotalNum			= syukei->Ckinko_goukei_cnt;		// �R�C�����ɍ��v��
	data->NoteTotalNum			= syukei->Skinko_goukei_cnt;		// �������ɍ��v��
	data->CyclicCoinTotalNum	= syukei->Junkan_goukei_cnt;		// �z�R�C�����v��
	data->NoteOutTotalNum		= syukei->Siheih_goukei_cnt;		// �������o�@���v��
	data->SettleNumServiceTime	= syukei->In_svst_seisan;			// �T�[�r�X�^�C�������Z��
	data->Shortage.Num			= syukei->Harai_husoku_cnt;			// ���o�s����
	data->Shortage.Amount		= syukei->Harai_husoku_ryo;			// ���o�s�����z
	data->Cancel.Num			= syukei->Seisan_chusi_cnt;			// ���Z���~��
	data->Cancel.Amount			= syukei->Seisan_chusi_ryo;			// ���Z���~���z
	data->AntiPassOffSettle		= syukei->Apass_off_seisan;			// �A���`�p�XOFF���Z��
	data->ReceiptIssue			= syukei->Ryosyuu_pri_cnt;			// �̎��ؔ��s����
	data->WarrantIssue			= syukei->Azukari_pri_cnt;			// �a��ؔ��s����
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	data->AllSystem.CarOutIllegal.Num	= syukei->Husei_out_Tcnt;	// �S���u  �s���o�ɉ�
//	data->AllSystem.CarOutIllegal.Amount= syukei->Husei_out_Tryo;	//                 ���z
//	data->AllSystem.CarOutForce.Num		= syukei->Kyousei_out_Tcnt;	//         �����o�ɉ�
//	data->AllSystem.CarOutForce.Amount	= syukei->Kyousei_out_Tryo;	//                 ���z
//	data->AllSystem.AcceptTicket		= syukei->Uketuke_pri_Tcnt;	//         ��t�����s��
//	data->AllSystem.ModifySettle.Num	= syukei->Syuusei_seisan_Tcnt;	//     �C�����Z��
//	data->AllSystem.ModifySettle.Amount	= syukei->Syuusei_seisan_Tryo;	//             ���z
//	data->CarInTotal			= syukei->In_car_Tcnt;				// �����ɑ䐔
//	data->CarOutTotal			= syukei->Out_car_Tcnt;				// ���o�ɑ䐔
//	data->CarIn1				= syukei->In_car_cnt[0];			// ����1���ɑ䐔
//	data->CarOut1				= syukei->Out_car_cnt[0];			// �o��1�o�ɑ䐔
//	data->CarIn2				= syukei->In_car_cnt[1];			// ����2���ɑ䐔
//	data->CarOut2				= syukei->Out_car_cnt[1];			// �o��2�o�ɑ䐔
//	data->CarIn3				= syukei->In_car_cnt[2];			// ����3���ɑ䐔
//	data->CarOut3				= syukei->Out_car_cnt[2];			// �o��3�o�ɑ䐔
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
	data->MiyoCount	= syukei->Syuusei_seisan_Mcnt;					// ��������
	data->MiroMoney	= syukei->Syuusei_seisan_Mryo;					// �������z
	data->LagExtensionCnt		= syukei->Lag_extension_cnt;		// ���O�^�C��������

	if(_is_ntnet_remote()) {
		if (ID != 30) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_30 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_30), NTNET_BUF_NORMAL);
	}
	/* ��{�W�v�f�[�^(ID30)��MAF�ɑ��M */
}

/*[]----------------------------------------------------------------------[]*/
/*| ������ʖ��W�v�f�[�^�쐬����                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiRyokinMai                               |*/
/*| PARAMETER    : syukei    : �W�v�f�[�^(���f�[�^)                        |*/
/*|                ID        : �f�[�^ID                                    |*/
/*|                Save      : �f�[�^�ێ��t���O                            |*/
/*|                Type      : �W�v�^�C�v                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiRyokinMai(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_31 *data = &SendNtnetDt.SData31;
	int i;
	int j;
	
	memset(data, 0, sizeof(DATA_KIND_31));
	
	BasicDataMake(ID, Save);										// ��{�ް��쐬
	data->Type					= Type;								// �W�v�^�C�v
	data->KakariNo				= syukei->Kakari_no;				// �W��No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// �W�v�ǔ�
	for (i = 0, j = 0; i < RYOUKIN_SYU_CNT; i++) {							// ���01�`50
		if (syukei->Rsei_cnt[i] == 0 && syukei->Rsei_ryo[i] == 0
		 && syukei->Rtwari_cnt[i] == 0 && syukei->Rtwari_ryo[i] == 0) {
		 	// �f�[�^�����ׂĂO�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			data->Kind[j].Kind	= i + 1;								//            �������
			data->Kind[j].Settle.Num		= syukei->Rsei_cnt[i];		//            ���Z��
			data->Kind[j].Settle.Amount		= syukei->Rsei_ryo[i];		//            ����z
			data->Kind[j].Discount.Num		= syukei->Rtwari_cnt[i];	//            ������
			data->Kind[j].Discount.Amount	= syukei->Rtwari_ryo[i];	//            �����z
			j++;
		}
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 31) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_31 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_31), NTNET_BUF_NORMAL);
	}
	/* ������ʖ��W�v�f�[�^(ID31)��MAF�ɑ��M */
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ޏW�v�f�[�^�쐬����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiBunrui                                  |*/
/*| PARAMETER    : syukei    : �W�v�f�[�^(���f�[�^)                        |*/
/*|                ID        : �f�[�^ID                                    |*/
/*|                Save      : �f�[�^�ێ��t���O                            |*/
/*|                Type      : �W�v�^�C�v                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiBunrui(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	int i;
	DATA_KIND_32 *data = &SendNtnetDt.SData32;
	char	flg[3] = {0,0,0};
	char	j,cnt = 0;
	char	pram_set[] = {0,0,50,100};

	memset(data, 0, sizeof(DATA_KIND_32));
	
	BasicDataMake(ID, Save);										// ��{�ް��쐬
	data->Type					= Type;								// �W�v�^�C�v
	data->KakariNo				= syukei->Kakari_no;				// �W��No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// �W�v�ǔ�

/*���ޏW�v�P*/
	for(j = 1 ; j <= 3 ; j++){
		for(i = 1 ; i <= 6 ; i++){
			if(j == (uchar)prm_get(COM_PRM, S_BUN, 52, 1, (char)i)){
				flg[cnt] = j;
				cnt++;
				break;
			}
			if(j == (uchar)prm_get(COM_PRM, S_BUN, 53, 1, (char)i)){
				flg[cnt] = j;
				cnt++;
				break;
			}
		}
	}
	if(flg[0]){
		data->Kind					= CPrmSS[S_BUN][1]+pram_set[flg[0]];	// ���ԕ��ޏW�v�̎��
		for (i = 0; i < BUNRUI_CNT; i++) {									// ����01�`48
			data->Group[i].Num		= syukei->Bunrui1_cnt[flg[0]-1][i];		//            �䐔1
			data->Group[i].Amount	= syukei->Bunrui1_ryo[flg[0]-1][i];		//            �䐔2�^���z
		}
		data->GroupTotal.Num		= syukei->Bunrui1_cnt1[flg[0]-1];		// ���ވȏ�   �䐔1
		data->GroupTotal.Amount		= syukei->Bunrui1_ryo1[flg[0]-1];		//            �䐔2�^���z
		data->Unknown.Num			= syukei->Bunrui1_cnt2[flg[0]-1];		// ���ޕs��   �䐔1
		data->Unknown.Amount		= syukei->Bunrui1_ryo2[flg[0]-1];		//            �䐔2�^���z
		flg[0] = 0;
	}
/*���ޏW�v�Q*/
	if(flg[1]){
		data->Kind2					= CPrmSS[S_BUN][1]+pram_set[flg[1]];	// ���ԕ��ޏW�v�̎��
		for (i = 0; i < BUNRUI_CNT; i++) {									// ����01�`48
			data->Group2[i].Num		= syukei->Bunrui1_cnt[flg[1]-1][i];		//            �䐔1
			data->Group2[i].Amount	= syukei->Bunrui1_ryo[flg[1]-1][i];		//            �䐔2�^���z
		}
		data->GroupTotal2.Num		= syukei->Bunrui1_cnt1[flg[1]-1];		// ���ވȏ�   �䐔1
		data->GroupTotal2.Amount		= syukei->Bunrui1_ryo1[flg[1]-1];	//            �䐔2�^���z
		data->Unknown2.Num			= syukei->Bunrui1_cnt2[flg[1]-1];		// ���ޕs��   �䐔1
		data->Unknown2.Amount		= syukei->Bunrui1_ryo2[flg[1]-1];		//            �䐔2�^���z
		flg[1] = 0;
	}
/*���ޏW�v�R*/
	if(flg[2]){
		data->Kind3					= CPrmSS[S_BUN][1]+pram_set[flg[2]];	// ���ԕ��ޏW�v�̎��
		for (i = 0; i < BUNRUI_CNT; i++) {									// ����01�`48
			data->Group3[i].Num		= syukei->Bunrui1_cnt[flg[2]-1][i];		//            �䐔1
			data->Group3[i].Amount	= syukei->Bunrui1_ryo[flg[2]-1][i];		//            �䐔2�^���z
		}
		data->GroupTotal3.Num		= syukei->Bunrui1_cnt1[flg[2]-1];		// ���ވȏ�   �䐔1
		data->GroupTotal3.Amount		= syukei->Bunrui1_ryo1[flg[2]-1];	//            �䐔2�^���z
		data->Unknown3.Num			= syukei->Bunrui1_cnt2[flg[2]-1];		// ���ޕs��   �䐔1
		data->Unknown3.Amount		= syukei->Bunrui1_ryo2[flg[2]-1];		//            �䐔2�^���z
		flg[2] = 0;
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 32) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_32 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_32), NTNET_BUF_NORMAL);
	}
	/* ���ޏW�v�f�[�^(ID32)��MAF�ɑ��M */
}

/*[]----------------------------------------------------------------------[]*/
/*| �����W�v�f�[�^�쐬����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiWaribiki                                |*/
/*| PARAMETER    : syukei    : �W�v�f�[�^(���f�[�^)                        |*/
/*|                ID        : �f�[�^ID                                    |*/
/*|                Save      : �f�[�^�ێ��t���O                            |*/
/*|                Type      : �W�v�^�C�v                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiWaribiki(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_33 *data = &SendNtnetDt.SData33;
	int i, parking, group;

	wk_media_Type = 0;
	memset(data, 0, sizeof(DATA_KIND_33));
	
	BasicDataMake(ID, Save);										// ��{�ް��쐬
	data->Type					= Type;								// �W�v�^�C�v
	data->KakariNo				= syukei->Kakari_no;				// �W��No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// �W�v�ǔ�
	i = 0;
	
// �N���W�b�g
	if (syukei->Ccrd_sei_cnt != 0 || syukei->Ccrd_sei_ryo != 0) {

	    data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];				// ��{���ԏ�ԍ����

		data->Discount[i].Kind		= 30;
		data->Discount[i].Num		= syukei->Ccrd_sei_cnt;
		data->Discount[i].Amount	= syukei->Ccrd_sei_ryo;
		i++;
	}

// �T�[�r�X��
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		for (group = 0; group < SERVICE_SYU_CNT; group++) {
			if (syukei->Stik_use_cnt[parking][group] == 0
			 && syukei->Stik_use_ryo[parking][group] == 0) {
				// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
				continue;
			} else {
				data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
				data->Discount[i].Kind		= 1;
				data->Discount[i].Group		= group + 1;
				data->Discount[i].Num		= syukei->Stik_use_cnt[parking][group];
				data->Discount[i].Amount	= syukei->Stik_use_ryo[parking][group];
				i++;
			}
		}
	}

// �v���y�C�h
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Pcrd_use_cnt[parking] == 0
		 && syukei->Pcrd_use_ryo[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 11;
			data->Discount[i].Num		= syukei->Pcrd_use_cnt[parking];
			data->Discount[i].Amount	= syukei->Pcrd_use_ryo[parking];
			i++;
		}
	}

// �񐔌�
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Ktik_use_cnt[parking] == 0
		 && syukei->Ktik_use_ryo[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 5;
			data->Discount[i].Num		= syukei->Ktik_use_cnt[parking];
			data->Discount[i].Amount	= syukei->Ktik_use_ryo[parking];
			i++;
		}
	}

// �X���� �XNo.��
	for (parking = 0; parking < PKNO_WARI_CNT; parking++) {
		for (group = 0; group < MISE_NO_CNT; group++) {
			if (syukei->Mno_use_cnt4[parking][group] == 0
			 && syukei->Mno_use_ryo4[parking][group] == 0) {
				// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
				continue;
			} else {
				data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
				data->Discount[i].Kind		= 2;
				if( CPrmSS[S_TAT][1] == 1L ){
					data->Discount[i].Kind = 3;
					data->Discount[i].Info = ( (group + 1) > 100 ?
												CPrmSS[S_TAT][32+((group + 1)-101)]:CPrmSS[S_STO][3+3*group]);
				}
				data->Discount[i].Group		= group + 1;
				data->Discount[i].Num		= syukei->Mno_use_cnt4[parking][group];
				data->Discount[i].Amount	= syukei->Mno_use_ryo4[parking][group];
				i++;
			}
		}
	}

// �X���� ���ԏ�No.��
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt1[parking] == 0
		 && syukei->Mno_use_ryo1[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 60;
			data->Discount[i].Num		= syukei->Mno_use_cnt1[parking];
			data->Discount[i].Amount	= syukei->Mno_use_ryo1[parking];
			i++;
		}
	}

// �X���� 1�`100���v
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt2[parking] == 0
		 && syukei->Mno_use_ryo2[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 61;
			data->Discount[i].Num		= syukei->Mno_use_cnt2[parking];
			data->Discount[i].Amount	= syukei->Mno_use_ryo2[parking];
			i++;
		}
	}

// �X���� 101�`999���v
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt3[parking] == 0
		 && syukei->Mno_use_ryo3[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			data->Discount[i].Kind		= 62;
			data->Discount[i].Num		= syukei->Mno_use_cnt3[parking];
			data->Discount[i].Amount	= syukei->Mno_use_ryo3[parking];
			i++;
		}
	}

// �X���� �S�������v
	if (syukei->Mno_use_Tcnt != 0 || syukei->Mno_use_Tryo != 0) {

	    data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];				// ��{���ԏ�ԍ����
		data->Discount[i].Kind		= 63;
		data->Discount[i].Num		= syukei->Mno_use_Tcnt;
		data->Discount[i].Amount	= syukei->Mno_use_Tryo;
		i++;
	}

// ��ʊ���
	for (group = 0; group < RYOUKIN_SYU_CNT; group++) {
		if( syukei->Rtwari_cnt[group] != 0 ){
		data->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];
		data->Discount[i].Kind		= 50;
		data->Discount[i].Group		= group + 1;
		data->Discount[i].Num		= syukei->Rtwari_cnt[group];
	  //data->Discount[i].Callback  = 0;
		data->Discount[i].Amount	= syukei->Rtwari_ryo[group];
	  //data->Discount[i].Info		= 0;
	  //data->Discount[i].Rsv		= 0;
		i++;
		}
	}

// Suica
	if( syukei->Electron_sei_cnt ){
		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
		data->Discount[i].Kind = NTNET_SUICA_1;							// ������ʁF31�iSuica���ρj�Œ�
		data->Discount[i].Group = 0;									// �����敪�F���g�p(0)
		data->Discount[i].Num = syukei->Electron_sei_cnt;				// �����񐔁F
		data->Discount[i].Callback = 0;									// ��������F���g�p(0)
		data->Discount[i].Amount = syukei->Electron_sei_ryo;			// �����z  �F
		data->Discount[i].Info = 0;										// �������F���g�p(0)
		data->Discount[i].Rsv = 0;										// �\���@�@�F���g�p(0)
		i++;
	}

	if( syukei->Electron_psm_cnt ){
// PASMO
		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
		data->Discount[i].Kind = NTNET_PASMO_0;							// ������ʁF33�iPASMO���ρj�Œ�
		data->Discount[i].Group = 0;									// �����敪�F���g�p(0)
		data->Discount[i].Num = syukei->Electron_psm_cnt;				// �����񐔁F
		data->Discount[i].Callback = 0;									// ��������F���g�p(0)
		data->Discount[i].Amount = syukei->Electron_psm_ryo;			// �����z  �F
		data->Discount[i].Info = 0;										// �������F���g�p(0)
		data->Discount[i].Rsv = 0;										// �\���@�@�F���g�p(0)
		i++;
	}

// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	if( syukei->Electron_edy_cnt ){
//// Edy
//		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
//		data->Discount[i].Kind = NTNET_EDY_0;							// ������ʁF32�iEdy���ρj�Œ�
//		data->Discount[i].Group = 0;									// �����敪�F(0)���������
//		data->Discount[i].Num = syukei->Electron_edy_cnt;				// �����񐔁F
//		data->Discount[i].Callback = 0;									// ��������F���g�p(0)
//		data->Discount[i].Amount = syukei->Electron_edy_ryo;			// �����z  �F
//		data->Discount[i].Info = 0;										// �������F���g�p(0)
//		data->Discount[i].Rsv = 0;										// �\���@�@�F���g�p(0)
//
//		i++;
//	}
//
//	if( syukei->Electron_Arm_cnt ){
//		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
//		data->Discount[i].Kind = NTNET_EDY_0;							// ������ʁF32�iEdy���ρj�Œ�
//		data->Discount[i].Group = 1;									// �����敪�F(1)�װю����
//		data->Discount[i].Num = syukei->Electron_Arm_cnt;				// �����񐔁F
//		data->Discount[i].Callback = 0;									// ��������F���g�p(0)
//		data->Discount[i].Amount = syukei->Electron_Arm_ryo;			// �����z  �F
//		data->Discount[i].Info = 0;										// �������F���g�p(0)
//		data->Discount[i].Rsv = 0;										// �\���@�@�F���g�p(0)
//		i++;
//	}
//
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	if( syukei->Electron_ico_cnt ){
// ICOCA
		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
		data->Discount[i].Kind = NTNET_ICOCA_0;							// ������ʁF35�iICOCA���ρj�Œ�
		data->Discount[i].Group = 0;									// �����敪�F(0)���������
		data->Discount[i].Num = syukei->Electron_ico_cnt;				// �����񐔁F
		data->Discount[i].Callback = 0;									// ��������F���g�p(0)
		data->Discount[i].Amount = syukei->Electron_ico_ryo;			// �����z  �F
		data->Discount[i].Info = 0;										// �������F���g�p(0)
		data->Discount[i].Rsv = 0;										// �\���@�@�F���g�p(0)

		i++;
	}
	
	if( syukei->Electron_icd_cnt ){

// IC-Card
		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
		data->Discount[i].Kind = NTNET_ICCARD_0;						// ������ʁF36�iIC-Card���ρj�Œ�
		data->Discount[i].Group = 0;									// �����敪�F(0)���������
		data->Discount[i].Num = syukei->Electron_icd_cnt;				// �����񐔁F
		data->Discount[i].Callback = 0;									// ��������F���g�p(0)
		data->Discount[i].Amount = syukei->Electron_icd_ryo;			// �����z  �F
		data->Discount[i].Info = 0;										// �������F���g�p(0)
		data->Discount[i].Rsv = 0;										// �\���@�@�F���g�p(0)

		i++;
	}

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	if (syukei->Gengaku_seisan_cnt) {
//		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
//		data->Discount[i].Kind = NTNET_GENGAKU;							// ������ʁF���z���Z
//		data->Discount[i].Group = 0;									// �����敪�F���g�p(0)
//		data->Discount[i].Num = syukei->Gengaku_seisan_cnt;				// �����񐔁F
//		data->Discount[i].Callback = 0;									// ��������F���g�p(0)
//		data->Discount[i].Amount = syukei->Gengaku_seisan_ryo;			// �����z  �F
//		data->Discount[i].Info = 0;										// �������F���g�p(0)
//		data->Discount[i].Rsv = 0;										// �\���@�@�F���g�p(0)
//		i++;
//	}
//
//	if (syukei->Furikae_seisan_cnt) {
//		data->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
//		data->Discount[i].Kind = NTNET_FURIKAE;							// ������ʁF�U�֐��Z
//		data->Discount[i].Group = 0;									// �����敪�F���g�p(0)
//		data->Discount[i].Num = syukei->Furikae_seisan_cnt;				// �����񐔁F
//		data->Discount[i].Callback = 0;									// ��������F���g�p(0)
//		data->Discount[i].Amount = syukei->Furikae_seisan_ryo;			// �����z  �F
//		data->Discount[i].Info = 0;										// �������F���g�p(0)
//		data->Discount[i].Rsv = 0;										// �\���@�@�F���g�p(0)
//		i++;
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	if(_is_ntnet_remote()) {
		if (ID != 33) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_33 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_33), NTNET_BUF_NORMAL);
	}
	/* �����W�v�f�[�^(ID33)��MAF�ɑ��M */
}

/*[]----------------------------------------------------------------------[]*/
/*| ����W�v�f�[�^�쐬����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiTeiki                                   |*/
/*| PARAMETER    : syukei    : �W�v�f�[�^(���f�[�^)                        |*/
/*|                ID        : �f�[�^ID                                    |*/
/*|                Save      : �f�[�^�ێ��t���O                            |*/
/*|                Type      : �W�v�^�C�v                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiTeiki(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_34 *data = &SendNtnetDt.SData34;
	int i, parking, kind;
	
	memset(data, 0, sizeof(DATA_KIND_34));
	
	BasicDataMake(ID, Save);										// ��{�ް��쐬
	data->Type					= Type;								// �W�v�^�C�v
	data->KakariNo				= syukei->Kakari_no;				// �W��No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// �W�v�ǔ�
	i = 0;
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		for (kind = 0; kind < TEIKI_SYU_CNT; kind++) {
			if ( ( syukei->Teiki_use_cnt[parking][kind] == 0 ) &&		// ����g�p�񐔂��O�̏ꍇ
				 ( syukei->Teiki_kou_cnt[parking][kind] == 0 ) ) {		// ����X�V�񐔂��O�̏ꍇ
				// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
				continue;
			} else {
				data->Pass[i].ParkingNo		= CPrmSS[S_SYS][1+parking];				// ���ԏ�No.
				data->Pass[i].Kind			= kind + 1;								// ���
				data->Pass[i].Num			= syukei->Teiki_use_cnt[parking][kind];	// ��
				data->Pass[i].Update.Num	= syukei->Teiki_kou_cnt[parking][kind];	// �X�V��
				data->Pass[i].Update.Amount	= syukei->Teiki_kou_ryo[parking][kind];	// �X�V������z
				i++;
			}
		}
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 34) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_34 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_34), NTNET_BUF_NORMAL);
	}
	/* ����W�v�f�[�^(ID34)��MAF�ɑ��M */
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ����W�v�f�[�^�쐬����                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiShashitsuMai                            |*/
/*| PARAMETER    : syukei    : �W�v�f�[�^(���f�[�^)                        |*/
/*|                ID        : �f�[�^ID                                    |*/
/*|                Save      : �f�[�^�ێ��t���O                            |*/
/*|                Type      : �W�v�^�C�v                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiShashitsuMai(LOKTOTAL *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_35 *data = &SendNtnetDt.SData35;
	t_SyuSub_Lock	*dst;
	LOKTOTAL_DAT	*src;
	int i;
	int	cnt;
	ulong	posi;
	
	memset(data, 0, sizeof(DATA_KIND_35));
	
	BasicDataMake(ID, Save);										// ��{�ް��쐬
	data->Type					= Type;								// �W�v�^�C�v
	data->KakariNo				= syukei->Kakari_no;				// �W��No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// �W�v�ǔ�
	for (i = 0 , cnt = 0; i < LOCK_MAX; i++) {
		WACDOG;														// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if(LockInfo[i].lok_no != 0){								//�ڑ��L�H
			if( !SetCarInfoSelect((short)i) ){
				continue;
			}
			if( cnt >= OLD_LOCK_MAX ){
				break;
			}
			posi = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	/* ���Ԉʒu 		*/
			dst = &data->Lock[cnt];
			src = &syukei->loktldat[i];
			dst->LockNo					= posi;						// �����
			dst->CashAmount				= src->Genuri_ryo;				// ��������
			dst->Settle					= src->Seisan_cnt;				// ���Z��
			dst->CarOutIllegal.Num		= src->Husei_out_cnt;			// �s���o�ɉ�
			dst->CarOutIllegal.Amount	= src->Husei_out_ryo;			//         ���z
			dst->CarOutForce.Num		= src->Kyousei_out_cnt;			// �����o�ɉ�
			dst->CarOutForce.Amount		= src->Kyousei_out_ryo;			//         ���z
			dst->AcceptTicket			= src->Uketuke_pri_cnt;			// ��t�����s��
			dst->ModifySettle.Num		= src->Syuusei_seisan_cnt;		// �C�����Z��
			dst->ModifySettle.Amount	= src->Syuusei_seisan_ryo;		//         ���z
			cnt++;
		}
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 35) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_35 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_35), NTNET_BUF_NORMAL);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���K�W�v�f�[�^�쐬����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiKinsen                                  |*/
/*| PARAMETER    : syukei    : �W�v�f�[�^(���f�[�^)                        |*/
/*|                ID        : �f�[�^ID                                    |*/
/*|                Save      : �f�[�^�ێ��t���O                            |*/
/*|                Type      : �W�v�^�C�v                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiKinsen(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{

	DATA_KIND_36 *data = &SendNtnetDt.SData36;
	ulong	w;
	int		i;
	
	memset(data, 0, sizeof(DATA_KIND_36));
	
	BasicDataMake(ID, Save);										// ��{�ް��쐬
	data->Type					= Type;								// �W�v�^�C�v
	data->KakariNo				= syukei->Kakari_no;				// �W��No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// �W�v�ǔ�
	data->Total					= syukei->Kinko_Tryo;				// ���ɑ������z
	data->NoteTotal				= syukei->Note_Tryo;				// �������ɑ������z
	data->CoinTotal				= syukei->Coin_Tryo;				// �R�C�����ɑ������z
	for (i = 0; i < COIN_SYU_CNT; i++) {							// �R�C��1�`4
		data->Coin[i].Kind		= c_coin[i];						//        ����
		data->Coin[i].Num		= syukei->Coin_cnt[i];				//        ����
	}
	data->Note[0].Kind			= 1000;								// ����1  ����
	data->Note[0].Num2			= syukei->Note_cnt[0];				//        ��������
	w = 0;
	for (i = 0; i < COIN_SYU_CNT; i++) {
		w += ((syukei->tou[i] + syukei->hoj[i]) * c_coin[i]);
	}
	data->CycleAccept			= w;								// �z���������z
	w = 0;
	for (i = 0; i < COIN_SYU_CNT; i++) {
		w += ((syukei->sei[i] + syukei->kyo[i]) * c_coin[i]);
	}
	data->CyclePay				= w;								// �z�����o���z
	data->NoteAcceptTotal		= syukei->tou[4] * 1000;			// �����������z
	for (i = 0; i < _countof(data->Cycle); i++) {
		data->Cycle[i].CoinKind			= c_coin[i];				// �z��1�`4 �R�C������
		data->Cycle[i].Accept			= syukei->tou[i];			//          ��������
		data->Cycle[i].Pay				= syukei->sei[i];			//          �o������
		data->Cycle[i].ChargeSupply		= syukei->hoj[i];			//          �ޑK��[����
		data->Cycle[i].SlotInventory	= syukei->kyo[i];			//          �C���x���g������(��o��)
	}
	
	if(_is_ntnet_remote()) {
		if (ID != 36) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_36 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_36), NTNET_BUF_NORMAL);
	}
	/* ���K�W�v�f�[�^(ID36)��MAF�ɑ��M */
}

/*[]----------------------------------------------------------------------[]*/
/*| �W�v�I���ʒm�f�[�^�쐬����                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_SyukeiSyuryo                                  |*/
/*| PARAMETER    : syukei    : �W�v�f�[�^(���f�[�^)                        |*/
/*|                ID        : �f�[�^ID                                    |*/
/*|                Save      : �f�[�^�ێ��t���O                            |*/
/*|                Type      : �W�v�^�C�v                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_SyukeiSyuryo(SYUKEI *syukei, uchar ID, uchar Save, ushort Type)
{
	DATA_KIND_41 *data = &SendNtnetDt.SData41;
	
	memset(data, 0, sizeof(DATA_KIND_41));
	
	BasicDataMake(ID, Save);										// ��{�ް��쐬
	data->Type					= Type;								// �W�v�^�C�v
	data->KakariNo				= syukei->Kakari_no;				// �W��No.
	data->SeqNo					= CountSel( &syukei->Oiban );		// �W�v�ǔ�
	
	if(_is_ntnet_remote()) {
		if (ID != 41) {
			RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_41 ));
		}
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_41), NTNET_BUF_NORMAL);
	}
	/* �W�v�I���ʒm�f�[�^(ID41)��MAF�ɑ��M */
}


/*[]----------------------------------------------------------------------[]*/
/*| �W�v��{�f�[�^��M����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiKihon                                   |*/
/*| PARAMETER    : syukei : �W�v�f�[�^					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ogura                                                   |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiKihon(SYUKEI *syukei)
{
	/* �����W�v�f�[�^��M�ҋ@���ȊO */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no			= RecvNtnetDt.RData66.KakariNo;							// �W��No.
	syukei->Oiban.w				= 														// �W�v�ǔ�
	syukei->Oiban.i				= RecvNtnetDt.RData66.SeqNo;							// �W�v�ǔ�
	syukei->NowTime.Year		= RecvNtnetDt.RData66.NowTime.Year;						// ����W�v
	syukei->NowTime.Mon			= RecvNtnetDt.RData66.NowTime.Mon;						// 
	syukei->NowTime.Day			= RecvNtnetDt.RData66.NowTime.Day;						// 
	syukei->NowTime.Hour		= RecvNtnetDt.RData66.NowTime.Hour;						// 
	syukei->NowTime.Min			= RecvNtnetDt.RData66.NowTime.Min;						// 
	syukei->OldTime.Year		= RecvNtnetDt.RData66.LastTime.Year;					// �O��W�v;
	syukei->OldTime.Mon			= RecvNtnetDt.RData66.LastTime.Mon;						// 
	syukei->OldTime.Day			= RecvNtnetDt.RData66.LastTime.Day;						// 
	syukei->OldTime.Hour		= RecvNtnetDt.RData66.LastTime.Hour;					// 
	syukei->OldTime.Min			= RecvNtnetDt.RData66.LastTime.Min;						// 
	syukei->Seisan_Tcnt			= RecvNtnetDt.RData66.SettleNum;						// �����Z��m;
	syukei->Kakeuri_Tryo		= RecvNtnetDt.RData66.Kakeuri;							// ���|���z
	syukei->Genuri_Tryo			= RecvNtnetDt.RData66.Cash;   							// ����������z
	syukei->Uri_Tryo			= RecvNtnetDt.RData66.Uriage; 							// ������z
	syukei->Tax_Tryo			= RecvNtnetDt.RData66.Tax;    							// ������Ŋz
	syukei->Turi_modosi_ryo		= RecvNtnetDt.RData66.Charge; 							// �ޑK���ߊz
	syukei->Ckinko_goukei_cnt	= RecvNtnetDt.RData66.CoinTotalNum;						// �R�C�����ɍ��v��
	syukei->Skinko_goukei_cnt	= RecvNtnetDt.RData66.NoteTotalNum;						// �������ɍ��v��
	syukei->Junkan_goukei_cnt	= RecvNtnetDt.RData66.CyclicCoinTotalNum;				// �z�R�C�����v��
	syukei->Siheih_goukei_cnt	= RecvNtnetDt.RData66.NoteOutTotalNum;					// �������o�@���v��
	syukei->In_svst_seisan		= RecvNtnetDt.RData66.SettleNumServiceTime;				// �T�[�r�X�^�C�������Z��
	syukei->Harai_husoku_cnt	= RecvNtnetDt.RData66.Shortage.Num;						// ���o�s����
	syukei->Harai_husoku_ryo	= RecvNtnetDt.RData66.Shortage.Amount;					// ���o�s�����z
	syukei->Seisan_chusi_cnt	= RecvNtnetDt.RData66.Cancel.Num;						// ���Z���~��
	syukei->Seisan_chusi_ryo	= RecvNtnetDt.RData66.Cancel.Amount;					// ���Z���~���z
	syukei->Apass_off_seisan	= RecvNtnetDt.RData66.AntiPassOffSettle;				// �A���`�p�XOFF���Z��
	syukei->Ryosyuu_pri_cnt		= RecvNtnetDt.RData66.ReceiptIssue;						// �̎��ؔ��s����
	syukei->Azukari_pri_cnt		= RecvNtnetDt.RData66.WarrantIssue;						// �a��ؔ��s����
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	syukei->Husei_out_Tcnt		= RecvNtnetDt.RData66.AllSystem.CarOutIllegal.Num;		// �S���u �s���o�� ��
//	syukei->Husei_out_Tryo		= RecvNtnetDt.RData66.AllSystem.CarOutIllegal.Amount;	//                 ���z
//	syukei->Kyousei_out_Tcnt	= RecvNtnetDt.RData66.AllSystem.CarOutForce.Num;		//        �����o�� ��
//	syukei->Kyousei_out_Tryo	= RecvNtnetDt.RData66.AllSystem.CarOutForce.Amount;		//                 ���z
//	syukei->Uketuke_pri_Tcnt	= RecvNtnetDt.RData66.AllSystem.AcceptTicket;			//        ��t�����s��
//	syukei->Syuusei_seisan_Tcnt	= RecvNtnetDt.RData66.AllSystem.ModifySettle.Num;		//        �C�����Z ��
//	syukei->Syuusei_seisan_Tryo	= RecvNtnetDt.RData66.AllSystem.ModifySettle.Amount;	//                 ���z
//	syukei->In_car_Tcnt			= RecvNtnetDt.RData66.CarInTotal;						// �����ɑ䐔
//	syukei->Out_car_Tcnt		= RecvNtnetDt.RData66.CarOutTotal;						// ���o�ɑ䐔
//	syukei->In_car_cnt[0]		= RecvNtnetDt.RData66.CarIn1;							// ����1���ɑ䐔
//	syukei->Out_car_cnt[0]		= RecvNtnetDt.RData66.CarOut1;							// �o��1�o�ɑ䐔1;
//	syukei->In_car_cnt[1]		= RecvNtnetDt.RData66.CarIn2;							// ����2���ɑ䐔
//	syukei->Out_car_cnt[1]		= RecvNtnetDt.RData66.CarOut2;							// �o��2�o�ɑ䐔2;
//	syukei->In_car_cnt[2]		= RecvNtnetDt.RData66.CarIn3;							// ����3���ɑ䐔
//	syukei->Out_car_cnt[2]		= RecvNtnetDt.RData66.CarOut3;							// �o��3�o�ɑ䐔3;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
	syukei->Lag_extension_cnt	= RecvNtnetDt.RData66.LagExtensionCnt;					// ���O�^�C��������
	
}

/*[]----------------------------------------------------------------------[]*/
/*| ������ʖ��W�v�f�[�^��M����                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiRyokinMai                               |*/
/*| PARAMETER    : syukei : �W�v�f�[�^					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiRyokinMai(SYUKEI *syukei)
{
	ushort	i, kind;
	
	/* �����W�v�f�[�^��M�ҋ@���ȊO */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no	= RecvNtnetDt.RData67.KakariNo;								// �W��No.
	syukei->Oiban.w		= 															// �W�v�ǔ�
	syukei->Oiban.i		= RecvNtnetDt.RData67.SeqNo;								// �W�v�ǔ�
	
	for ( i = 0; i < _countof( RecvNtnetDt.RData67.Kind ); i++) {					// ��� 01�`50
		kind = RecvNtnetDt.RData67.Kind[i].Kind;
		if ( kind >= 1 && kind <= RYOUKIN_SYU_CNT ){								// ������ʐ� 1�`12
			kind--;
			syukei->Rsei_cnt[kind]		= RecvNtnetDt.RData67.Kind[i].Settle.Num;		// ���Z��
			syukei->Rsei_ryo[kind]		= RecvNtnetDt.RData67.Kind[i].Settle.Amount;	// ����z
			syukei->Rtwari_cnt[kind]	= RecvNtnetDt.RData67.Kind[i].Discount.Num;		// ������
			syukei->Rtwari_ryo[kind]	= RecvNtnetDt.RData67.Kind[i].Discount.Amount;	// �����z
		}
	}
	
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ޏW�v�f�[�^��M����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiBunrui                                  |*/
/*| PARAMETER    : syukei : �W�v�f�[�^					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiBunrui(SYUKEI *syukei)
{
	uchar i;
	uchar	bn_syu;
	uchar	tyu_syu;
	/* �����W�v�f�[�^��M�ҋ@���ȊO */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no	= RecvNtnetDt.RData68.KakariNo;						// �W��No.
	syukei->Oiban.w		= 													// �W�v�ǔ�
	syukei->Oiban.i		= RecvNtnetDt.RData68.SeqNo;						// �W�v�ǔ�

	/*���ޏW�v�P*/
	bn_syu = 0xff;
	tyu_syu= 0xff;
	if(( RecvNtnetDt.RData68.Kind >= 1 ) && ( RecvNtnetDt.RData68.Kind <= 7 )){
		bn_syu = 0;
		tyu_syu = (uchar)RecvNtnetDt.RData68.Kind;//�W�v���
	}else if(( RecvNtnetDt.RData68.Kind >= 51 ) && ( RecvNtnetDt.RData68.Kind <= 57 )){
		bn_syu = 1;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind - 50);//�W�v���
	}else if(( RecvNtnetDt.RData68.Kind >= 101 ) && ( RecvNtnetDt.RData68.Kind <= 107 )){
		bn_syu = 2;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind - 100);//�W�v���
	}
	if(( tyu_syu == CPrmSS[S_BUN][1] )&&( bn_syu != 0xff )){
		for (i = 0; i < BUNRUI_CNT; i++) {										// ����01�`48
			syukei->Bunrui1_cnt[bn_syu][i] = RecvNtnetDt.RData68.Group[i].Num;		//           �䐔1
			syukei->Bunrui1_ryo[bn_syu][i] = RecvNtnetDt.RData68.Group[i].Amount;	//           �䐔2�^���z
		}
		syukei->Bunrui1_cnt1[bn_syu] = RecvNtnetDt.RData68.GroupTotal.Num;			// ���ވȏ�@�䐔
		syukei->Bunrui1_ryo1[bn_syu] = RecvNtnetDt.RData68.GroupTotal.Amount;		//           �䐔2�^���z
		syukei->Bunrui1_cnt2[bn_syu] = RecvNtnetDt.RData68.Unknown.Num;				// ���ޕs���@�䐔
		syukei->Bunrui1_ryo2[bn_syu] = RecvNtnetDt.RData68.Unknown.Amount;			//           �䐔2�^���z
	}
	/*���ޏW�v�Q*/
	bn_syu = 0xff;
	tyu_syu= 0xff;
	if(( RecvNtnetDt.RData68.Kind2 >= 1 ) && ( RecvNtnetDt.RData68.Kind2 <= 7 )){
		bn_syu = 0;
		tyu_syu = (uchar)RecvNtnetDt.RData68.Kind2;//�W�v���
	}else if(( RecvNtnetDt.RData68.Kind2 >= 51 ) && ( RecvNtnetDt.RData68.Kind2 <= 57 )){
		bn_syu = 1;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind2 - 50);//�W�v���
	}else if(( RecvNtnetDt.RData68.Kind2 >= 101 ) && ( RecvNtnetDt.RData68.Kind2 <= 107 )){
		bn_syu = 2;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind2 - 100);//�W�v���
	}
	if(( tyu_syu == CPrmSS[S_BUN][1] )&&( bn_syu != 0xff )){
		for (i = 0; i < BUNRUI_CNT; i++) {										// ����01�`48
			syukei->Bunrui1_cnt[bn_syu][i] = RecvNtnetDt.RData68.Group2[i].Num;		//           �䐔1
			syukei->Bunrui1_ryo[bn_syu][i] = RecvNtnetDt.RData68.Group2[i].Amount;	//           �䐔2�^���z
		}
		syukei->Bunrui1_cnt1[bn_syu] = RecvNtnetDt.RData68.GroupTotal2.Num;			// ���ވȏ�@�䐔
		syukei->Bunrui1_ryo1[bn_syu] = RecvNtnetDt.RData68.GroupTotal2.Amount;		//           �䐔2�^���z
		syukei->Bunrui1_cnt2[bn_syu] = RecvNtnetDt.RData68.Unknown2.Num;				// ���ޕs���@�䐔
		syukei->Bunrui1_ryo2[bn_syu] = RecvNtnetDt.RData68.Unknown2.Amount;			//           �䐔2�^���z
	}
	/*���ޏW�v�R*/
	bn_syu = 0xff;
	tyu_syu= 0xff;
	if(( RecvNtnetDt.RData68.Kind3 >= 1 ) && ( RecvNtnetDt.RData68.Kind3 <= 7 )){
		bn_syu = 0;
		tyu_syu = (uchar)RecvNtnetDt.RData68.Kind3;//�W�v���
	}else if(( RecvNtnetDt.RData68.Kind3 >= 51 ) && ( RecvNtnetDt.RData68.Kind3 <= 57 )){
		bn_syu = 1;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind3 - 50);//�W�v���
	}else if(( RecvNtnetDt.RData68.Kind3 >= 101 ) && ( RecvNtnetDt.RData68.Kind3 <= 107 )){
		bn_syu = 2;
		tyu_syu = (uchar)(RecvNtnetDt.RData68.Kind3 - 100);//�W�v���
	}
	if(( tyu_syu == CPrmSS[S_BUN][1] )&&( bn_syu != 0xff )){
		for (i = 0; i < BUNRUI_CNT; i++) {										// ����01�`48
			syukei->Bunrui1_cnt[bn_syu][i] = RecvNtnetDt.RData68.Group3[i].Num;		//           �䐔1
			syukei->Bunrui1_ryo[bn_syu][i] = RecvNtnetDt.RData68.Group3[i].Amount;	//           �䐔2�^���z
		}
		syukei->Bunrui1_cnt1[bn_syu] = RecvNtnetDt.RData68.GroupTotal3.Num;			// ���ވȏ�@�䐔
		syukei->Bunrui1_ryo1[bn_syu] = RecvNtnetDt.RData68.GroupTotal3.Amount;		//           �䐔2�^���z
		syukei->Bunrui1_cnt2[bn_syu] = RecvNtnetDt.RData68.Unknown3.Num;				// ���ޕs���@�䐔
		syukei->Bunrui1_ryo2[bn_syu] = RecvNtnetDt.RData68.Unknown3.Amount;			//           �䐔2�^���z
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �����W�v�f�[�^��M����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiWaribiki                                |*/
/*| PARAMETER    : syukei : �W�v�f�[�^					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiWaribiki(SYUKEI *syukei)
{
	uchar	parking;
	ushort	i, group, kind;
	
	/* �����W�v�f�[�^��M�ҋ@���ȊO */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no	= RecvNtnetDt.RData69.KakariNo;									// �W��No.
	syukei->Oiban.w		= 																// �W�v�ǔ�
	syukei->Oiban.i		= RecvNtnetDt.RData69.SeqNo;									// �W�v�ǔ�
	
	for ( i = 0; i < _countof( RecvNtnetDt.RData69.Discount ); i++ ){					//����01�`500
		kind = RecvNtnetDt.RData69.Discount[i].Kind;									//������ʊl��
		group = RecvNtnetDt.RData69.Discount[i].Group;									//�����敪�l��
		parking = NTNET_GetParkingKind( RecvNtnetDt.RData69.Discount[i].ParkingNo, PKOFS_SEARCH_LOCAL );	//���ԏ�ԍ��l��

		// �N���W�b�g
		if ( kind == 30 ){
			syukei->Ccrd_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Ccrd_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// �X���� �S�������v
		} else if ( kind == 63 ){
			syukei->Mno_use_Tcnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Mno_use_Tryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// ��ʊ���
		// Suica
		} else if ( kind == 31 ){
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//			syukei->Electron_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
//			syukei->Electron_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			if(isEC_USE()) {
				syukei->koutsuu_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
				syukei->koutsuu_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			} else {
				syukei->Electron_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
				syukei->Electron_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
		// PASMO
		} else if ( kind == 33 ){
			syukei->Electron_psm_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Electron_psm_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// ICOCA
		} else if ( kind == NTNET_ICOCA_0 ){
			syukei->Electron_ico_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Electron_ico_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// IC-Card
		} else if ( kind == NTNET_ICCARD_0 ){
			syukei->Electron_icd_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->Electron_icd_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		// Edy
		} else if ( kind == 32 ){
			if( 0 == group ){		// �������
				syukei->Electron_edy_cnt = RecvNtnetDt.RData69.Discount[i].Num;
				syukei->Electron_edy_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			}else{					// ���s����i�A���[������j
				syukei->Electron_Arm_cnt = RecvNtnetDt.RData69.Discount[i].Num;
				syukei->Electron_Arm_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
			}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
		} else if(kind == NTNET_NANACO_0) {
			syukei->nanaco_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->nanaco_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		} else if(kind == NTNET_WAON_0) {
			syukei->waon_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->waon_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		} else if(kind == NTNET_SAPICA_0) {
			syukei->sapica_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->sapica_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		} else if(kind == NTNET_ID_0) {
			syukei->id_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->id_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
		} else if(kind == NTNET_QUICPAY_0) {
			syukei->quicpay_sei_cnt = RecvNtnetDt.RData69.Discount[i].Num;
			syukei->quicpay_sei_ryo = RecvNtnetDt.RData69.Discount[i].Amount;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
		} else {
			if ( parking != 0xFF ){														//��v���钓�ԏ�ԍ�������ꍇ
				switch ( kind ){
				// �T�[�r�X��
				case	01:
					if ( group >= 1 && group <= SERVICE_SYU_CNT ) {						// �T�[�r�X�������敪1�`3
						group--;
						syukei->Stik_use_cnt[parking][group] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Stik_use_ryo[parking][group] = RecvNtnetDt.RData69.Discount[i].Amount;
					}
					break;
				// �v���y�C�h
				case	11:
						syukei->Pcrd_use_cnt[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Pcrd_use_ryo[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				// �񐔌�
				case	05:
						syukei->Ktik_use_cnt[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Ktik_use_ryo[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				// �X���� �XNo.��
				case	02:
						if (((group >= 1) && (group <= MISE_NO_CNT)) && ((parking == 0) || (parking == 1))) {	// �X�����敪1�`100�@���@��{�E�g��
							group--;
							syukei->Mno_use_cnt4[parking][group] = RecvNtnetDt.RData69.Discount[i].Num;
							syukei->Mno_use_ryo4[parking][group] = RecvNtnetDt.RData69.Discount[i].Amount;
						}
					break;
				// �X���� ���ԏ�No.��
				case	60:
						syukei->Mno_use_cnt1[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Mno_use_ryo1[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				// �X���� 1�`100���v
				case	61:
						syukei->Mno_use_cnt2[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Mno_use_ryo2[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				// �X���� 101�`255���v
				case	62:
						syukei->Mno_use_cnt3[parking] = RecvNtnetDt.RData69.Discount[i].Num;
						syukei->Mno_use_ryo3[parking] = RecvNtnetDt.RData69.Discount[i].Amount;
					break;
				default:
					break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ����W�v�f�[�^��M����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiTeiki                                   |*/
/*| PARAMETER    : syukei : �W�v�f�[�^					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiTeiki(SYUKEI *syukei)
{
	uchar	parking;
	ushort	i;

	/* �����W�v�f�[�^��M�ҋ@���ȊO */
	if (NTNetTotalEndFlag)
		return;
	
	syukei->Kakari_no	= RecvNtnetDt.RData70.KakariNo;									// �W��No.
	syukei->Oiban.w		= 																// �W�v�ǔ�
	syukei->Oiban.i		= RecvNtnetDt.RData70.SeqNo;									// �W�v�ǔ�
	for ( i = 0; i < _countof( RecvNtnetDt.RData70.Pass ); i++) {						// �����001�`100
		parking = NTNET_GetParkingKind( RecvNtnetDt.RData70.Pass[i].ParkingNo, PKOFS_SEARCH_LOCAL );
		if ( parking != 0xFF ){
			// �������ʐ�1�`15
			if ( RecvNtnetDt.RData70.Pass[i].Kind >= 1 && RecvNtnetDt.RData70.Pass[i].Kind <= TEIKI_SYU_CNT ){ 
				syukei->Teiki_use_cnt[parking][RecvNtnetDt.RData70.Pass[i].Kind-1] = RecvNtnetDt.RData70.Pass[i].Num;
			}
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ���K�W�v�f�[�^��M����                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiKinsen                                  |*/
/*| PARAMETER    : syukei : �W�v�f�[�^					<OUT>              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ri                                                      |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiKinsen(SYUKEI *syukei)
{
	static const ushort c_coin[] = { 10, 50, 100, 500 };
	ushort	i, j;

	/* �����W�v�f�[�^��M�ҋ@���ȊO */
	if (NTNetTotalEndFlag)
		return;

	
	syukei->Kakari_no	= RecvNtnetDt.RData72.KakariNo;									// �W��No.
	syukei->Oiban.w		= 																// �W�v�ǔ�
	syukei->Oiban.i		= RecvNtnetDt.RData72.SeqNo;									// �W�v�ǔ�
	syukei->Kinko_Tryo	= RecvNtnetDt.RData72.Total;									// ���ɑ������z
	syukei->Note_Tryo	= RecvNtnetDt.RData72.NoteTotal;								// �������ɑ������z
	syukei->Coin_Tryo	= RecvNtnetDt.RData72.CoinTotal;								// �R�C�����ɑ������z

	for ( i = 0; i < COIN_SYU_CNT; i++ ) {												// �R�C��1�`4
		j = ntnet_CoinIdx(RecvNtnetDt.RData72.Coin[i].Kind);							// �R�C������
		if( j != 0xFFFF ){
			syukei->Coin_ryo[j] = RecvNtnetDt.RData72.Coin[i].Kind * RecvNtnetDt.RData72.Coin[i].Num;	// �R�C�����ɓ����z�@�e���했
			syukei->Coin_cnt[j] = RecvNtnetDt.RData72.Coin[i].Num;						// �R�C�����ɓ������@�e���했
		}
	}
	
	for ( i = 0; i < SIHEI_SYU_CNT; i++ ) {												// ����1�`4
		if ( RecvNtnetDt.RData72.Note[i].Kind == 1000 ){								// ��������
			syukei->Note_ryo[0] = RecvNtnetDt.RData72.Note[i].Kind * RecvNtnetDt.RData72.Note[i].Num2;	// �������ɓ����z�@�e���했
			syukei->Note_cnt[0] = RecvNtnetDt.RData72.Note[i].Num2;						// �������ɓ������@�e���했
		}
	}
	
	syukei->tou[4] = RecvNtnetDt.RData72.NoteAcceptTotal / 1000;						// ������������
	
	for ( i = 0; i < COIN_SYU_CNT; i++ ) {												// �z��1�`4
		j = ntnet_CoinIdx(RecvNtnetDt.RData72.Cycle[i].CoinKind);						// �z��1�`4�R�C������
		if( j != 0xFFFF ){
			syukei->tou[j] = RecvNtnetDt.RData72.Cycle[i].Accept;						// ���Z���������@�e���했
			syukei->hoj[j] = RecvNtnetDt.RData72.Cycle[i].ChargeSupply;					// �ޑK��[�����@�e���했
			syukei->sei[j] = RecvNtnetDt.RData72.Cycle[i].Pay;							// ���Z���o�����@�e���했
			syukei->kyo[j] = RecvNtnetDt.RData72.Cycle[i].SlotInventory;				// �������o�����@�e���했
		}
	}
	syukei->tounyu = 1000 * syukei->tou[4];
	syukei->hojyu  = 
	syukei->seisan = 
	syukei->kyosei = 0;
	for (i = 0; i < _countof(c_coin); i++) {
		syukei->tounyu += ( c_coin[i] * syukei->tou[i] );								// ���Z�������z�@���z
		syukei->hojyu  += ( c_coin[i] * syukei->hoj[i] );								// �ޑK��[���z�@���z
		syukei->seisan += ( c_coin[i] * syukei->sei[i] );								// ���Z���o���z�@���z
		syukei->kyosei += ( c_coin[i] * syukei->kyo[i] );								// �������o���z�@���z
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �W�v�I���ʒm�f�[�^��M����                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_SyukeiSyuryo                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ogura                                                   |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Rev_SyukeiSyuryo( void )
{
	/* �����W�v�f�[�^��M���� */
	if (NTNetTotalEndFlag == 0) {
		NTNetTotalEndFlag = 1;		// �����W�v�f�[�^��M����
	}
}


/*[]----------------------------------------------------------------------[]*/
/*| Free�ް��쐬����                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_DataFree                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_Snd_DataFree( void )
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_Free ) );
	memcpy( SendNtnetDt.SDataFree.FreeData, "FREE", 4 );
	SendNtnetDt.SDataFree.FreeData[5] = 5;	/* �f�[�^�N���A */
	if( NTBUF_SetSendFreeData( &SendNtnetDt ) == 0 ){
		// �o�b�t�@�I�[�o�[���C�g����
		
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �����ް�(�ް����100)�쐬����  �i����߽�ݒ�j                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data100                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : H.Sekiguchi                                             |*/
/*| Date         : 2006-08-23                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_Snd_Data100( void )
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_100 ) );			//�f�[�^�N���A
	BasicDataMake( 100 , 1 );									//��{�f�[�^�쐬
	SendNtnetDt.SData101.SMachineNo = 0;						// ���M��[���@�B���i�e�@���j
//����߽�ݒ�
	if( PPrmSS[S_P01][3] == 0 ){
		SendNtnetDt.SData101.ControlData[3] = 1;				// ����
	}
	else if( PPrmSS[S_P01][3] == 1 ){
		SendNtnetDt.SData101.ControlData[3] = 2;				// ����
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_100 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
	
}

/*[]----------------------------------------------------------------------[]*/
/*| �ݒ��ް��v��(�ް����90)�쐬���� - ���ڽ�w��                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data90                                        |*/
/*| PARAMETER    : mac   : ���M��[���@�B��                                |*/
/*|              : sts   : �ް��ð��(0:���� 1:�ŏI)                        |*/
/*|              : seg   : �ݒ辸����(�����)                               |*/
/*|              : addr  : �J�n���ڽ                                       |*/
/*|              : count : �ݒ��ް���                                      |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2006-11-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static const ushort NtDataPrm90[1][4]= {

		{  0,   S_DIS,   8,  18 },			// ������
};

void	NTNET_Snd_Data90(uchar type)
{
	if (_is_ntnet_remote()) {
		return;
	}
	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_78));

	BasicDataMake(90, 1);														// ��{�ް��쐬

	SendNtnetDt.SData90.SMachineNo = 0L;										// ���M������ه����e�@
	SendNtnetDt.SData90.ModelCode = (ulong)NTNET_MODEL_CODE;					// �[���@��R�[�h
	SendNtnetDt.SData90.PrmDiv = NtDataPrm90[type][0];							// ���Ұ��敪
	SendNtnetDt.SData90.Segment = NtDataPrm90[type][1];							// �v��������
	SendNtnetDt.SData90.TopAddr = NtDataPrm90[type][2];							// �J�n���ڽ
	SendNtnetDt.SData90.DataCount = NtDataPrm90[type][3];						// �ݒ��ް���

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_78 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_78), NTNET_BUF_NORMAL);	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ��MNTNET�f�[�^�ŎԎ��p�����[�^or���b�N���u�p�����[�^�X�V(��d�ۏؑΉ�) |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_UpdateParam                                       |*/
/*| PARAMETER    : fukuden - �X�V���f�[�^���                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTNET_UpdateParam(NTNETCTRL_FUKUDEN_PARAM *fukuden)
{
	union {
		DATA_KIND_97 *p97;
		DATA_KIND_98 *p98;
	}u;
	int i;
	int j=0;
	
	switch (fukuden->DataKind) {
	case 97:	// �Ԏ��p�����[�^
		u.p97 = (DATA_KIND_97*)fukuden->Src;
		for (i = 0; i < LOCK_MAX; i++) {
			WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if( !SetCarInfoSelect((short)i) ){
				continue;
			}
			if( j >= OLD_LOCK_MAX ){
				break;
			}
			LockInfo[i].lok_syu	=	(uchar)u.p97->LockInfo[j].lok_syu;
			LockInfo[i].ryo_syu	=	(uchar)u.p97->LockInfo[j].ryo_syu;
			LockInfo[i].area	=	(uchar)u.p97->LockInfo[j].area;
			LockInfo[i].posi	=	(ulong)u.p97->LockInfo[j].posi;
			LockInfo[i].if_oya	=	(uchar)u.p97->LockInfo[j].if_oya;
			if( i < BIKE_START_INDEX ) {	// ���ԏ��̏ꍇ�A�ϊ����K�v
				if( u.p97->LockInfo[j].lok_no < PRM_LOKNO_MIN || u.p97->LockInfo[j].lok_no > PRM_LOKNO_MAX ){	// �͈̓`�F�b�N( 100�`315 �L�� )
					LockInfo[i].lok_no	=	0;														// �͈͊O�̏ꍇ�A0(�ڑ������)��ۑ�
				} else {
					LockInfo[i].lok_no	=	(uchar)( u.p97->LockInfo[j].lok_no - PRM_LOKNO_MIN );	// �͈͓��̏ꍇ�A100�������ĕۑ�����(�ۑ��̈悪1byte�̂���)
				}
			} else {						// ���ւ͏]���ʂ�
				LockInfo[i].lok_no	=	(uchar)u.p97->LockInfo[j].lok_no;
			}
			j++;
		}
		DataSumUpdate(OPE_DTNUM_LOCKINFO);				// RAM���SUM�X�V
		FLT_WriteLockParam1(FLT_NOT_EXCLUSIVE);			// FLASH������
		break;
	case 98:	// ���b�N���u�p�����[�^
		u.p98 = (DATA_KIND_98*)fukuden->Src;
		for (i = 0; i < _countof(LockMaker); i++) {
			LockMaker[i].in_tm		= (uchar)u.p98->LockMaker[i].in_tm;
			LockMaker[i].ot_tm		= (uchar)u.p98->LockMaker[i].ot_tm;
			LockMaker[i].r_cnt		= (uchar)u.p98->LockMaker[i].r_cnt;
			LockMaker[i].r_tim		= u.p98->LockMaker[i].r_tim;
			LockMaker[i].open_tm	= (uchar)u.p98->LockMaker[i].open_tm;
			LockMaker[i].clse_tm	= (uchar)u.p98->LockMaker[i].clse_tm;
		}
		break;
	default:	// ���d�����Ȃ�
		break;
	}
	
	fukuden->DataKind = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ԏ�ԍ��擾                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_GetParkinngNo                                     |*/
/*| PARAMETER    : uchar ParkingKind                                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong NTNET_GetParkingNo(uchar ParkingKind)
{
	ulong parking_no;

	parking_no = CPrmSS[S_SYS][65 + ParkingKind];
	return parking_no;

}

/*[]----------------------------------------------------------------------[]*/
/*| ����X�V�X�e�[�^�X�擾                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_ReadUpDateStatus                                  |*/
/*| PARAMETER    : ulong PassId                                            |*/
/*| RETURN VALUE : uchar                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar ntnet_ReadUpDateStatus(ulong PassId)
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	PAS_RENEWAL work;
//	uchar status;
//	work = pas_renewal[PassId / 4];
//
//	switch (PassId % 4) {
//	case 1:
//		status = work.BIT.Bt01;
//		break;
//	case 2:
//		status = work.BIT.Bt23;
//		break;
//	case 3:
//		status = work.BIT.Bt45;
//		break;
//	case 0:
//	default:
//		status = work.BIT.Bt67;
//		break;
//	
//	}
//
//	return status;
	return 0;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

/*[]----------------------------------------------------------------------[]*/
/*| �W�v�I���ʒm�f�[�^                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_isTotalEndReceived                                |*/
/*| PARAMETER    : ushort result                                           |*/
/*| RETURN VALUE : int 0:����M                                            |*/
/*| RETURN VALUE : int 1:��M����                                          |*/
/*| RETURN VALUE : int-1:NG��M                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	NTNET_isTotalEndReceived( ushort *result )
{
	*result = NTNetTotalEndError;
	return NTNetTotalEndFlag;
}

/*[]----------------------------------------------------------------------[]*/
/*| �����f�[�^�ϊ�                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_DateTimeCnv                                       |*/
/*| PARAMETER    : dst = �ϊ���, src = �ϊ���                              |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ntnet_DateTimeCnv(date_time_rec2 *dst, const date_time_rec *src)
{
	dst->Year	= src->Year;
	dst->Mon	= src->Mon;
	dst->Day	= src->Day;
	dst->Hour	= src->Hour;
	dst->Min	= src->Min;
	dst->Sec	= 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| ��������Z���~�f�[�^����                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_SearchTeikiCyusiData                              |*/
/*| PARAMETER    : ParkingNo = �����L�[1(���ԏ�No.)                        |*/
/*|                PassID    = �����L�[2(�����ID)                         |*/
/*| RETURN VALUE : �Y���f�[�^�̃e�[�u���擪����̃C���f�b�N�X              |*/
/*|                �f�[�^��������Ȃ��Ƃ��́A-1                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	ntnet_SearchTeikiCyusiData(ulong ParkingNo, ulong PassID)
{
	short idx, ofs;
	
	idx = 0;
	for (idx = 0; idx < tki_cyusi.count; idx++) {
		ofs = _TKI_Idx2Ofs(idx);
		if (tki_cyusi.dt[ofs].pk == ParkingNo && tki_cyusi.dt[ofs].no == PassID) {
			return idx;
		}
	}
	
	return -1;
}

/*[]----------------------------------------------------------------------[]*/
/*| ��������Z���~�f�[�^�ϊ�(CRM�t�H�[�}�b�g��NTNET�t�H�[�}�b�g)           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_TeikiCyusiCnvCrmToNt                              |*/
/*| PARAMETER    : ntnet = NTNET�t�H�[�}�b�g�f�[�^           <OUT>         |*/
/*|                crm   = CRM�t�H�[�}�b�g�f�[�^             <IN>          |*/
/*|                all   = TRUE���S�Ă̍��ڂ��Z�b�g                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	ntnet_TeikiCyusiCnvCrmToNt(TEIKI_CHUSI *ntnet, const struct TKI_CYUSI *crm, BOOL all)
{
	
	ntnet->ParkingNo		=	crm->pk;
	ntnet->PassID			=	crm->no;
	if (all) {
		ntnet->PassKind			=	crm->tksy;
		ntnet->Year				=	crm->year;
		ntnet->Month			=	crm->mon;
		ntnet->Day				=	crm->day;
		ntnet->Hour				=	crm->hour;
		ntnet->Min				=	crm->min;
		ntnet->Sec				=	0;
		ntnet->ChargeType		=	crm->syubetu;
		ntnet->UseCount			=	crm->use_count;
		ntnet->CouponRyo		=	crm->kry;
		ntnet->DiscountMoney	=	crm->wryo;
		ntnet->DiscountTime		=	crm->wminute;
		ntnet->DiscountRate		=	crm->wpercent;
		ntnet->ShopNo			=	crm->mno;
		ntnet->ServiceTicketA	=	crm->sa[0];
		ntnet->ServiceTicketB	=	crm->sa[1];
		ntnet->ServiceTicketC	=	crm->sa[2];
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ��������Z���~�f�[�^�ϊ�(NTNET�t�H�[�}�b�g��CRM�t�H�[�}�b�g)           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_TeikiCyusiCnvNtToCrm                              |*/
/*| PARAMETER    : crm   = CRM�t�H�[�}�b�g�f�[�^           <OUT>           |*/
/*|                ntnet = NTNET�t�H�[�}�b�g�f�[�^         <IN>            |*/
/*| RETURN VALUE : BOOL                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	ntnet_TeikiCyusiCnvNtToCrm(struct TKI_CYUSI *crm, const TEIKI_CHUSI *ntnet)
{
	crm->pk			=	ntnet->ParkingNo;
	crm->no			=	(short)ntnet->PassID;
	crm->tksy		=	(uchar)ntnet->PassKind;
	crm->year		=	ntnet->Year;
	crm->mon		=	(uchar)ntnet->Month;
	crm->day		=	(uchar)ntnet->Day;
	crm->hour		=	(uchar)ntnet->Hour;
	crm->min		=	(uchar)ntnet->Min;
	crm->syubetu	=	(uchar)ntnet->ChargeType;
	crm->use_count	=	(uchar)ntnet->UseCount;
	crm->kry		=	ntnet->CouponRyo;
	crm->wryo		=	ntnet->DiscountMoney;
	crm->wminute	=	ntnet->DiscountTime;
	crm->wpercent	=	(uchar)ntnet->DiscountRate;
	crm->mno		=	(char)ntnet->ShopNo;
	crm->sa[0]		=	(char)ntnet->ServiceTicketA;
	crm->sa[1]		=	(char)ntnet->ServiceTicketB;
	crm->sa[2]		=	(char)ntnet->ServiceTicketC;
	return TRUE;
}

/*[]----------------------------------------------------------------------[]*/
/*| ����敪�擾�i�}�X�^�[�̋敪�j                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_GetParkingKind                                    |*/
/*| PARAMETER    : ulong PassId                                            |*/
/*|                int search = PKOFS_SEARCH_LOCAL or PKOFS_SEARCH_MASTER  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : mitani                                                  |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar NTNET_GetParkingKind(ulong ParkingId, int search)
{
	int i, base;
	uchar parking_kind;

	parking_kind = 0xFF;

	if (ParkingId == 0) {
		return parking_kind;
	}

	if (search == PKOFS_SEARCH_MASTER) {
		base = 65;
	}
	else {	// search == PKOFS_SEARCH_LOCAL
		base = 1;
	}
	for (i = 0; i < 4; i++) {
		if (ParkingId == CPrmSS[S_SYS][base + i]) {
			parking_kind = (uchar)i;
			break;
		}
	}

	return parking_kind;
}

/*[]----------------------------------------------------------------------[]*/
/*| �R�C�����킩��CRM���f�[�^�C���f�b�N�X���擾                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : ntnet_CoinIdx                                           |*/
/*| PARAMETER    : kind = �R�C������                                       |*/
/*| RETURN VALUE : CRM���f�[�^�C���f�b�N�X(0�`3)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	ntnet_CoinIdx(ushort kind)
{
	switch (kind){
	case	10:
		return 0;
	case	50:
		return 1;
	case	100:
		return 2;
	case	500:
		return 3;
	default:
		return 0xFFFF;
	}
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �ڍג��~�ر����																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: cyushi_chk( no )																   |*/
/*| PARAMETER	: no	; ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: H.Sekiguchi																	   |*/
/*| Date		: 2006-10-15																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar cyushi_chk( ushort no )
{
	struct CAR_TIM wk_time1,wk_time2;
	uchar	i;

	for(i = 0 ; i < 10 ; i++){
		if(FLAPDT_SUB[i].WPlace == LockInfo[no].posi){
			/*���Ɏ���*/
			wk_time1.year = FLAPDT.flp_data[no].year ;
			wk_time1.mon = FLAPDT.flp_data[no].mont ;
			wk_time1.day = FLAPDT.flp_data[no].date ;
			wk_time1.hour = FLAPDT.flp_data[no].hour ;
			wk_time1.min = FLAPDT.flp_data[no].minu ;
			/*�ڍג��~�����ް�*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;
			
			if(0 == ec64(&wk_time1,&wk_time2)){
					break;
			}
		}
	}
	if(i >= 10){
		return 99;									//�Đ��Z&&�ڍג��~�ر��
	}else{
		return i;									//�Đ��Z&&�ڍג��~�ر�L
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ��{�ް�2�쐬����                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : BasicDataMake_R                                         |*/
/*| PARAMETER    : knd  : �ް����                                         |*/
/*|                keep : �ް��ێ��׸�                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-08-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	BasicDataMake_R( uchar knd, uchar keep )
{
	SendNtnetDt.DataBasicR.SystemID = REMOTE_SYSTEM;	// ����ID
	SendNtnetDt.DataBasicR.DataKind = knd;				// �ް����
	SendNtnetDt.DataBasicR.DataKeep = keep;				// �ް��ێ��׸�+ ����/�蓮�׸�
}

/*[]----------------------------------------------------------------------[]*/
/*| ���uIBK����f�[�^(�ް����60)���M����                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data60_R                                      |*/
/*| PARAMETER    : req = �v���f�[�^�t���O                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data60_R(ulong req)
{
	memset(&SendNtnetDt.SData60_R, 0, sizeof(DATA_KIND_60_R));
	BasicDataMake_R(60, 1);	// �ŏ�ʃr�b�g�F�����Œ�
	ntnet_set_req(req, SendNtnetDt.SData60_R.Data, REMOTE_IBKCTRL_MAX);

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_60_R ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_60_R), NTNET_BUF_PRIOR);	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���M�E�e�[�u���N���A�v���f�[�^(�ް����61/62)���M����                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_DataTblCtrl                                   |*/
/*| PARAMETER    : id  = 61: ���M�v��/ 62: �N���A�v��                      |*/
/*|                req = �v���f�[�^�t���O                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_DataTblCtrl(uchar id, ulong req, uchar bAuto)
{
	uchar keep;

	if (bAuto) {
		keep = 0x01;	// ����
	} else {
		keep = 0x81;	// �蓮
	}
	memset(&SendNtnetDt.SData61_R, 0, sizeof(DATA_KIND_61_R));
	BasicDataMake_R(id, keep);	// �ŏ�ʃr�b�g�F����/�蓮
	ntnet_set_req(req, SendNtnetDt.SData61_R.Data, NTNET_BUFCTRL_REQ_MAX);

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_61_R ));
	}
	else {
		if( keep == 0x81 )		// �蓮���M
			NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_61_R), NTNET_BUF_PRIOR);		// �D���ޯ̧�ő��M�o�^
		else					// �������M
			NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_61_R), NTNET_BUF_NORMAL);	// �ʏ��ޯ̧�ő��M�o�^
	}
}

// �v���f�[�^��ݒ�
void	ntnet_set_req(ulong req, uchar *dat, int max)
{
	ushort i;
	ulong  flg = 0x00000001;

	for (i = 0; i < max; i++) {
		if (req & (flg << i)) dat[i] = 1;
	}
}

static DATA_KIND_100R_sub SndSata100R_BU;
/*[]----------------------------------------------------------------------[]*/
/*| �ʐM�`�F�b�N�v���f�[�^(�ް����100)���M����                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data100_R                                     |*/
/*| PARAMETER    : mode = 0: ����/ 1: ���~                                 |*/
/*|                chk_num = �ʐM����No.                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data100_R(uchar mode, uchar chk_num)
{
	memset(&SendNtnetDt.SData100_R, 0, sizeof(DATA_KIND_100_R));
	memset(&SndSata100R_BU, 0, sizeof(DATA_KIND_100R_sub));

	BasicDataMake_R(100, 0x01);

	SendNtnetDt.SData100_R.SeqNo = 0;									// ���ݼ�ه�(0:�������Ȃ�)

	SendNtnetDt.SData100_R.sub.ParkingNo  = (ulong)CPrmSS[S_SYS][1];	// ���ԏꇂ
	SendNtnetDt.SData100_R.sub.ModelCode  = NTNET_MODEL_CODE;			// �@����
	SendNtnetDt.SData100_R.sub.SMachineNo = 0;	// ���M���� 0:main

	SendNtnetDt.SData100_R.sub.Year = (uchar)( CLK_REC.year % 100 );	// �N
	SendNtnetDt.SData100_R.sub.Mon  = (uchar)CLK_REC.mont;				// ��
	SendNtnetDt.SData100_R.sub.Day  = (uchar)CLK_REC.date;				// ��
	SendNtnetDt.SData100_R.sub.Hour = (uchar)CLK_REC.hour;				// ��
	SendNtnetDt.SData100_R.sub.Min  = (uchar)CLK_REC.minu;				// ��
	SendNtnetDt.SData100_R.sub.Sec  = (uchar)CLK_REC.seco;				// �b

	SendNtnetDt.SData100_R.sub.ChkNo = chk_num;							// �ʐM����No.

	SendNtnetDt.SData100_R.Data[0] = mode;								// �v�����
	SendNtnetDt.SData100_R.Data[1] = 10;								// ��ѱ��

	memcpy(&SndSata100R_BU, &SendNtnetDt.SData100_R.sub, sizeof(DATA_KIND_100_R));	// �ް��ޯ�����

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100_R ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_100_R), NTNET_BUF_PRIOR);	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �ʐM�`�F�b�N���ʃf�[�^(�ް����101)��M����                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_Data101_R                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Rev_Data101_R(void)
{
	int ret;
	
	// ���M�f�[�^�Ɣ�r
	ret = memcmp(&SndSata100R_BU, &RecvNtnetDt.RData101_R.sub, sizeof(DATA_KIND_100R_sub));
	if (ret == 0) {
		IBK_ComChkResult = RecvNtnetDt.RData101_R.Data[0];	// ���ʃR�[�h�Z�b�g
	}
	else {
		IBK_ComChkResult = 99;								// ��M�f�[�^�s��v
	}

	queset( OPETCBNO, IBK_COMCHK_END, 0, NULL );			// NTNET �ʐM�e�X�g�I���ʒm
}

#define RelaySize	38
static union {
	uchar  cRelayBuf[RelaySize];
	ushort sRelayBuf[RelaySize/2];
} rbuf;

/*[]----------------------------------------------------------------------[]*/
/*| �e�[�u�������f�[�^(�ް����83)��M����                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData83_R                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       :                                                         |*/
/*| Date         : 2007-01-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData83_R(void)
{

	//kind:80 ��M����̗���Ȃ�AHOST�ɍ��Z�l�Œʒm(IBK_BufCount��IBK�̎����Ă�l)
	if (s_IsRevData80_R) {
		NTNET_Snd_Data63_R();
		s_IsRevData80_R = 0;
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���uIBK����f�[�^
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData80_R
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData80_R(void)
{
	if (RecvNtnetDt.RData60_R.Data[0] == 1) {
		// �����t���OON  NTNET_RevData83_R���ɎQ�Ƃ���
		s_IsRevData80_R = 1;
		NTNET_Snd_Data60_R(1);
	}
}

/*[]----------------------------------------------------------------------[]*
 *| �e�[�u�������f�[�^���M
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data63_R
 *| PARAMETER    : 
 *| RETURN VALUE : 
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data63_R(void)
{
	t_NtBufCount buf;
	ushort *pt = rbuf.sRelayBuf;

	// CRM�ێ��̌����擾
	NTBUF_GetBufCount(&buf);

	memset(&SendNtnetDt.SData63_R, 0, sizeof(DATA_KIND_83_R));
	BasicDataMake_R( 63, 1 );										// ��{�ް��쐬

	// IBK + CRM�i��U���p���[�N�ɃR�s�[�j
	memset(&rbuf.sRelayBuf, 0, sizeof(rbuf.sRelayBuf));
	// ����

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 6)) {
//		*pt = (ushort)buf.sndbuf_incar;
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	// �o��
	*pt++;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	*pt = (ushort)buf.sndbuf_outcar;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ������������� 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	// ���Z
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 5)) {
		*pt = (ushort)buf.sndbuf_sale;
	}
	// �W�v
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 4)) {
		if(0 != prm_get(COM_PRM, S_NTN, 26, 1, 3)) {	// �W�v��34-0026�C=0(���M���Ȃ��j�Ȃ�O���Ƃ���
			*pt = (ushort)buf.sndbuf_ttotal;
		}
	}
	// �G���[
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 3)) {
		*pt = (ushort)buf.sndbuf_error;
	}
	// �A���[��
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 2)) {
		*pt = (ushort)buf.sndbuf_alarm;
	}
	// ���j�^�[
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 61, 1, 1)) {
		*pt = (ushort)buf.sndbuf_monitor;
	}
	// ���샂�j�^�[
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 6)) {
		*pt = (ushort)buf.sndbuf_ope_monitor;
	}
	// �R�C�����ɏW�v
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 5)) {
		*pt = (ushort)buf.sndbuf_coin;
	}
	// �������ɏW�v
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 4)) {
		*pt = (ushort)buf.sndbuf_note;
	}
	// ���ԑ䐔
	*pt++;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 3)) {
//		*pt = (ushort)buf.sndbuf_car_count;
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	// ���䐔�E���ԃf�[�^
	*pt++;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	*pt = (ushort)buf.sndbuf_area_count;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	// ���K�Ǘ�
	*pt++;
	if(0 == prm_get(COM_PRM, S_NTN, 62, 1, 1)) {
		*pt = (ushort)buf.sndbuf_money;
	}

	memcpy(SendNtnetDt.SData63_R.Data, rbuf.cRelayBuf, RelaySize);

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_83_R ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_83_R ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}


/*[]----------------------------------------------------------------------[]*
 *| �j�A�t���ʒm�f�[�^
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData90_R
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData90_R(void)
{
	int i;
	ulong	occur = 0;	// �ω��̂���������
	ulong	mask = 0;	// �j�A�t�������̂���
	ulong	bit;

	// ���M�v��/�f�[�^�N���A�v���̕��тɍ��킹�� -->
	// ���Z�f�[�^(ID22��ID23)�̃}�[�W
	if (RecvNtnetDt.RData90_R.Data[2] == 0) {
		// ID22�ω��Ȃ�
		RecvNtnetDt.RData90_R.Data[2] = RecvNtnetDt.RData90_R.Data[3];	// ID23
	}
	else if (RecvNtnetDt.RData90_R.Data[2] == 2) {
		// ID22�j�A�t������
		if (RecvNtnetDt.RData90_R.Data[3] == 1)
			RecvNtnetDt.RData90_R.Data[2] = 1;		// ������D��
	}
	// �ЂƂO�ɂ߂�
	memcpy(&RecvNtnetDt.RData90_R.Data[3], &RecvNtnetDt.RData90_R.Data[4], NTNET_BUFCTRL_REQ_MAX-3);
	RecvNtnetDt.RData90_R.Data[NTNET_BUFCTRL_REQ_MAX-1] = 0;

	// �ϊ���f�[�^����r�b�g�f�[�^�擾
	bit = 1L;
	for (i = 0; i < NTNET_BUFCTRL_REQ_MAX; i++) {
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		if(( i == 0 ) || ( i == 1 ) || ( i == 10 ) || ( i == 11 )){
			// ����/�o��/���ԑ䐔/���䐔�E���ԃf�[�^�̓`�F�b�N����
			// bit�����炵�ăX�L�b�v����
			bit <<= 1;
			continue;
		}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		// �ω�����
		if (RecvNtnetDt.RData90_R.Data[i] != 0) {
			mask |= bit;
		}
		// �j�A�t������
		if (RecvNtnetDt.RData90_R.Data[i] == 1) {
			if(i == 12) {
				bit <<= 1;
			}
			occur |= bit;
		}
		bit <<= 1;
	}

	// �ʒm
	if (occur) {
		ntautoPostNearFull(occur);	// �������M�֔����ʒm
	}
	if (mask) {
		NTBUF_SetIBKNearFull(mask, occur);	// Buffer�w�֒ʒm
	}
}

/*[]----------------------------------------------------------------------[]*
 *| �ʐM�`�F�b�N�v���f�[�^
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData116_R
 *| PARAMETER    : void
 *| RETURN VALUE : void
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData116_R(void)
{
	IBK_ComChkResult = RecvNtnetDt.RData101_R.Data[0];	// �G���[�R�[�h�Z�b�g

	if (RecvNtnetDt.RData101_R.Data[0] != 0) {	// �`�F�b�N�J�n�ȊO�Ȃ�ǂݎ̂�
		return;
	}

	NTNET_Snd_Data117_R(0, (char*)&RecvNtnetDt.RData101_R);	// ���ʃR�[�h"OK"�ŕԓ�
}

/*[]----------------------------------------------------------------------[]*
 *| �ʐM�`�F�b�N���ʃf�[�^���M
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data117_R
 *| PARAMETER    : 
 *| RETURN VALUE : 
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-02-13
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data117_R(uchar code, char *data)
{
	DATA_KIND_100_R *blk;

	blk = (DATA_KIND_100_R*)data;
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_100_R ) );

	BasicDataMake_R( 117, 1 );										// ��{�ް��쐬

	SendNtnetDt.SData100_R.SeqNo = blk->SeqNo;
	memcpy((char*)&SendNtnetDt.SData100_R.sub, (char*)&blk->sub, sizeof(DATA_KIND_100R_sub));

	SendNtnetDt.SData100_R.Data[0] = code;

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_100_R ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_100_R ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ������⍇���f�[�^��M
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_RevData142
 *| PARAMETER    : 
 *| RETURN VALUE : 
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-03-20
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_RevData142(void)
{
	if (!_is_ntnet_remote()) {
		return;		// HOST�ʐM���̂ݑΉ�
	}

	if (RecvNtnetDt.RData142.ParkingNo == 0 ||
		RecvNtnetDt.RData142.PassID == 0) {
		return;		// ���ԏ�No�A�����ID��0�̏ꍇ�͖���
	}

	NTNET_Snd_Data143(RecvNtnetDt.RData142.ParkingNo, RecvNtnetDt.RData142.PassID);
}

/*[]----------------------------------------------------------------------[]*
 *| ������⍇�����ʃf�[�^���M
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : NTNET_Snd_Data143
 *| PARAMETER    : 
 *| RETURN VALUE : 
 *[]----------------------------------------------------------------------[]*
 *| Author       : 
 *| Date         : 2007-03-20
 *| UpDate       : 
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void	NTNET_Snd_Data143(ulong ParkingNo, ulong PassID)
{
	PAS_TBL		*pass_tbl;
	date_time_rec	time;
	ushort		status;

	memset(&SendNtnetDt, 0, sizeof(DATA_KIND_143));

	// ������ð���ް��擾
	pass_tbl = GetPassData((ulong)ParkingNo, (ushort)PassID);
	if (pass_tbl == NULL) {
		return;
	}
	//������X�V�ð���擾
	if (ReadPassRenewalTbl((ulong)ParkingNo, (ushort)PassID, &status) < 0) {
		return;
	}
	// ������o�Ɏ���ð��ٓǏo��
	memset(&time, 0, sizeof(date_time_rec));
	PassExitTimeTblRead((ulong)ParkingNo, (ushort)PassID, &time);

	// �ް����
	BasicDataMake(143, 1);
	SendNtnetDt.SData143.ParkingNo = ParkingNo;
	SendNtnetDt.SData143.PassID = PassID;
	if(pass_tbl->BIT.INV == 0){									// �L��
		SendNtnetDt.SData143.PassState = pass_tbl->BIT.STS;		// 0:�����@1:�o�ɒ�  2:���ɒ�
	}else{														// ����
		SendNtnetDt.SData143.PassState = 3;						// 3:����
	}
	SendNtnetDt.SData143.RenewalState = (uchar)status;
	memcpy(&SendNtnetDt.SData143.OutTime, &time, sizeof(date_time_rec));
	SendNtnetDt.SData143.UseParkingNo = (ulong)prm_get(COM_PRM, S_SYS, 1, 6, 1);

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_143 ));
	}
	else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_143), NTNET_BUF_PRIOR);	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �p�X���[�h�f�[�^(�ް����84)��M����                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Rev_Data84                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Suzuki                                                |*/
/*| Date         : 2006-06-26                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	NTNET_RevData84( void )
{
	uchar	w_uc;
	ulong 	w_ul;

	// �߽ܰ���ް���M�ް����������Ұ�TBL�ɔ��f����B
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord[0] & 0x0f);
	w_ul = (ulong)(w_uc * 1000); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord[1] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc * 100); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord[2] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc * 10); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord[3] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc); 

	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord2[0] & 0x0f);
	w_ul = (ulong)(w_uc * 1000); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord2[1] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc * 100); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord2[2] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc * 10); 
	w_uc = (uchar)(RecvNtnetDt.RData84.Data.PassWord2[3] & 0x0f);
	w_ul = w_ul + (ulong)(w_uc); 

	DataSumUpdate(OPE_DTNUM_COMPARA);					/* update parameter sum on ram */
	FLT_WriteParam1(FLT_NOT_EXCLUSIVE);					// FlashROM update
	SetSetDiff(SETDIFFLOG_SYU_REMSET);					// �f�t�H���g�Z�b�g���O�o�^����

	// ���샍�O��o�^����
	wopelg(OPLOG_SET_PASSWORD, 0, 0);
}


typedef	struct {
	uchar			id;			// ���
	uchar			code;		// ����
} t_MltE_CPS;

const t_MltE_CPS	MltE_CPS_SndTbl[4][50] = {	// �����䐸�Z�@���̐e�@���M�v�ۃ`�F�b�N�p�e�[�u��
	{	// type=00�i�G���[�j
		{0x03, 0x01}, {0x03, 0x05}, {0x03, 0x06}, {0x03, 0x07}, {0x03, 0x0c},
		{0x03, 0x0d}, {0x03, 0x0e}, {0x03, 0x0f}, {0x03, 0x12}, {0x03, 0x13},
		{0x03, 0x14}, {0x03, 0x05}, {0x04, 0x01}, {0x04, 0x05}, {0x04, 0x06},
		{0x04, 0x07}, {0x04, 0x08}, {0x02, 0x01}, {0x02, 0x04}, {0x00, 0x11},
		{0x01, 0x01}, {0x01, 0x0a}, {0x01, 0x0b}, {0x01, 0x0d}, {0x01, 0x0e},
		{0x01, 0x0f}, {0x01, 0x10}, {0x01, 0x11}, {0x01, 0x12}, {0x01, 0x13},
		{0x01, 0x14}, {0x01, 0x18}, {0x01, 0x19}, {0x01, 0x20}, {0x00, 0x1f},
		{0x00, 0x12}, {0x09, 0x0a}, {0x09, 0x0b}, {0x00, 0x19}, {0x3f, 0x01},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}
	},
	{	// type=01�i�A���[���j
		{0x01, 0x1d}, {0x01, 0x02}, {0x00, 0x01}, {0x00, 0x05}, {0x00, 0x06},
		{0x01, 0x06}, {0x01, 0x20}, {0x01, 0x07}, {0x01, 0x1f}, {0x01, 0x0f},
		{0x01, 0x10}, {0x01, 0x11}, {0x01, 0x12}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}
	},
	{	// type=02�i���j�^�j
		{0x01, 0x0d}, {0x01, 0x0e}, {0x01, 0x0b}, {0x01, 0x0c}, {0x01, 0x07},
		{0x01, 0x08}, {   0,   88}, {   0,   89}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}
	},
	{	// type=03�i���샂�j�^�j
		{0x01, 0x01}, {0x01, 0x02}, {0x01, 0x0b}, {0x01, 0x0c}, {0x12, 0x01},
		{0x12, 0x02}, {0x12, 0x03}, {0x12, 0x04}, {0x12, 0x05}, {0x12, 0x06},
		{0x12, 0x07}, {0x11, 0x01}, {0x11, 0x02}, {0x11, 0x0c}, {0x01, 0x0f},
		{0x50, 0x5a}, {0x50, 0x5b}, {0x01, 0x14}, {0x01, 0x15}, {0x01, 0x16},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff},
		{0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}, {0xff, 0xff}
	},
};

/*[]----------------------------------------------------------------------[]
 *|	name	: Remote exclusive use Charge calculation
 *[]----------------------------------------------------------------------[]
 *| summary	: ���u�p�����v�Z����
 *| return	: ���ԗ���
 *[]----------------------------------------------------------------------[]*/
ulong	OrgCal( ushort Index )
{

	char	tim[6];
	ulong	ret=0;
	
	if ( LockInfo[Index].lok_syu != 0 ) {				// �ڑ�������Ԏ��̏ꍇ
		if( FLAPDT.flp_data[Index].nstat.bits.b00 ){	// �ݎԂ��Ă���ꍇ
			if( FLAPDT.flp_data[Index].timer < 0){
				if( InCarTimeChk( (ushort)(Index+1), &CLK_REC ) == 0 ){	// ���o�Ɏ���OK?(Y)
					ret = 1;
				}
			}
		}
	}

	if( !ret )
		return ret;
	
	memcpy( &CAL_WORK.car_in, &car_in, sizeof( struct CAR_TIM ));  	/*�@���Ԏ����@*/
	memcpy( &CAL_WORK.car_ot, &car_ot, sizeof( struct CAR_TIM ));  	/*�@�o�Ԏ����@*/
	memcpy( &CAL_WORK.org_in, &org_in, sizeof( struct CAR_TIM )); 	/*�@���Ԏ����@*/
	memcpy( &CAL_WORK.org_ot, &org_ot, sizeof( struct CAR_TIM )); 	/*�@�o�Ԏ����@*/
	memcpy( &CAL_WORK.carin_mt, &carin_mt, sizeof( struct CAR_TIM )); /*�@���Ԏ���(��)�@*/
	memcpy( &CAL_WORK.carot_mt, &carot_mt, sizeof( struct CAR_TIM )); /*�@�o�Ԏ���(��)�@*/
	memcpy( &CAL_WORK.nmax_in, &nmax_in, sizeof( struct CAR_TIM ));  /*�@�����ԍő��_�����@*/
	
	CAL_WORK.wk_strt = wk_strt;								  		/*�w�莞���ő�check�����ɉ��Z���鎞�ԁ@�@�@�@ */
	CAL_WORK.iti_flg = iti_flg;		  								/*�@��藿���уt���O�@�@�@�@�@�@�@�@�@�@�@�@�@*/
	CAL_WORK.amartim = amartim;		  								/*����藿���ї]�莞�ԁ@�@�@�@�@�@�@�@�@�@�@�@*/

	memcpy( tim,&FLAPDT.flp_data[Index].year,6 ); 					/* ���Ԏ������ 			*/
	car_in.year  = *(short*)tim ;									/* ���ԔN				*/
	car_in.mon   = tim[2]	;										/*	   ��				*/
	car_in.day   = tim[3]	;										/*�@�@ �� �@�@�@�@�@	*/
	car_in.hour  = tim[4]	;										/*�@�@ �� �@�@�@�@�@	*/
	car_in.min   = tim[5]	;										/*�@�@ �� �@�@�@�@�@	*/
	car_in.week  = (char)youbiget( (short)car_in.year ,				/*�@�@ �j�� �@�@�@�@	*/
						 (short)car_in.mon  ,						/*�@�@	�@�@�@�@�@�@	*/
						 (short)car_in.day  )	;					/*�@�@	�@�@�@�@�@�@	*/

	memcpy( tim, &CLK_REC, 6 );

	car_ot.year = *(short*)tim   ;									/* �o�ɔN �@�@�@�@�@	*/
	car_ot.mon  = tim[2]       ;									/*�@�@ �� �@�@�@�@�@	*/
	car_ot.day  = tim[3]       ;									/*     �� �@�@�@�@�@	*/
	car_ot.week  = (char)youbiget( (short)car_in.year ,				//     �j��
								(short)car_in.mon  ,
								(short)car_in.day  );				/*�@�@	�@�@�@�@�@�@	*/
	car_ot.hour = tim[4]       ;									/* 	   �� �@�@�@�@�@	*/
	car_ot.min  = tim[5]       ;									/*�@�@ �� �@�@�@�@�@	*/

	memcpy( &nmax_in,	&org_in, 7 );								// ���ۂ̓��Ɏ������m���Ԋ�_�����o�b�t�@�֊i�[
	CAL_WORK.ryoukin = ryoukin;
	CAL_WORK.total = total;
	CAL_WORK.syubt = (int)req_rkn.syubt;
	CAL_WORK.base_ryo = base_ryo;
	CAL_WORK.discount = discount;
	CAL_WORK.ntnet_nmax_bak = ntnet_nmax_flg;						/*�@�ő嗿���׸��ޯ����߁@*/
	memcpy( &CAL_WORK.stim, &se_svt.stim, sizeof( struct	SVT	));
	CAL_WORK.mod = 1;												/* �ޯ����ߒ����		*/
	ryoukin = 0;
	if( LockInfo[Index].ryo_syu <= 0 || LockInfo[Index].ryo_syu > 12){
		req_rkn.syubt = 1;
	}else{
		req_rkn.syubt = LockInfo[Index].ryo_syu;
	}
	if( CPrmSS[S_STM][1] == 0 ){									/* �S���ʻ��޽���,��ڰ����		*/
		se_svt.stim = (short)CPrmSS[S_STM][2];						/* ���޽���get					*/
		se_svt.gtim = (short)CPrmSS[S_STM][3];						/* ��ڰ����get					*/
		se_svt.rtim = (short)CPrmSS[S_STM][4];						/* ׸����get					*/
	}else{															/* ��ʖ����޽���				*/
		se_svt.stim = (short)CPrmSS[S_STM][5+(3*(req_rkn.syubt-1))];/* ��ʖ����޽���get			*/
		se_svt.gtim = (short)CPrmSS[S_STM][6+(3*(req_rkn.syubt-1))];/* ��ʖ���ڰ����get			*/
		se_svt.rtim = (short)CPrmSS[S_STM][7+(3*(req_rkn.syubt-1))];/* ��ʖ�׸����get				*/
	}																/*								*/

	if( FLAPDT.flp_data[Index].lag_to_in.BIT.LAGIN == ON ){
		se_svt.stim = 0;											/* Yes�����޽��т𖳌��Ƃ���	*/
	}
	else
	{
// MH322914 (s) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)
//		if(	( FLAPDT.flp_data[Index].mode == FLAP_CTRL_MODE3 ) ||
//			( FLAPDT.flp_data[Index].mode == FLAP_CTRL_MODE4 ) )
//		{
//			long	svtm,uptm;
//
//			svtm = (long)se_svt.stim;
//			svtm *= 60;
//
//			if( (Index+1) >= LOCK_START_NO)
//			{
//				uptm = Carkind_Param(ROCK_CLOSE_TIMER, req_rkn.syubt, 5,1);
//			}
//			else
//			{
//				uptm = Carkind_Param(FLAP_UP_TIMER, req_rkn.syubt, 5,1);
//			}
//			uptm = (long)(((uptm/100)*60) + (uptm%100));
//
//			if( svtm == uptm )											
//			{
//				se_svt.stim = 0;											// ���޽��і���
//			}
//		}
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//		if ( prm_get( COM_PRM, S_TYP, 68, 1, 5 ) == 0 ) {
//			// ���b�N�^�C�}�[�ƃT�[�r�X�^�C�����������Ԃ̎��ɃT�[�r�X�^�C�������b�N�܂łƂ���
//			if(	( FLAPDT.flp_data[Index].mode == FLAP_CTRL_MODE3 ) ||
//				( FLAPDT.flp_data[Index].mode == FLAP_CTRL_MODE4 ) )
//			{
//				long	svtm,uptm;
//
//				svtm = (long)se_svt.stim;
//				svtm *= 60;
//
//				if( (Index+1) >= LOCK_START_NO)
//				{
//					uptm = Carkind_Param(ROCK_CLOSE_TIMER, req_rkn.syubt, 5,1);
//				}
//				else
//				{
//					uptm = Carkind_Param(FLAP_UP_TIMER, req_rkn.syubt, 5,1);
//				}
//				uptm = (long)(((uptm/100)*60) + (uptm%100));
//
//				if( svtm == uptm )											
//				{
//					se_svt.stim = 0;											// ���޽��і���
//				}
//			}
//		}
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// MH322914 (e) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)
	}
	WACDOG;// RX630��WDG�X�g�b�v���o���Ȃ��d�l�̂���WDG���Z�b�g�ɂ���
	et40_remote();													/* �����Z�o				*/
	WACDOG;// RX630��WDG�X�g�b�v���o���Ȃ��d�l�̂���WDG���Z�b�g�ɂ���
	ret = ryoukin;
	Remote_Cal_Data_Restor();
	return( ret );

}

/*[]----------------------------------------------------------------------[]*/
/*| ���o�Ɏ�������                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : InCarTimeChk( num, clk )				                   |*/
/*| PARAMETER    : num      : �Ԏ���                                       |*/
/*|              : clk      : ���ݎ���                                     |*/
/*| RETURN VALUE : ret      : ���o�Ɏ������� 0:OK 1:NG 					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2010-10-28                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
uchar	InCarTimeChk( ushort num, struct clk_rec *clk_para )
{
	flp_com	*frp;
	ushort	in_tim, out_tim;

	frp = &FLAPDT.flp_data[num-1];

	if( ( !frp->year || !frp->mont || !frp->date ) ||
	    chkdate( (short)frp->year, (short)frp->mont, (short)frp->date )){	// ���Ɏ����K��O?
		return( 1 );
	}

	in_tim = dnrmlzm((short)frp->year,								// (���ɓ�)normlize
					(short)frp->mont,
					(short)frp->date );
	out_tim = dnrmlzm((short)clk_para->year,						// (�o�ɓ�)���ݔN������normlize
					(short)clk_para->mont,
					(short)clk_para->date );
					
	if( in_tim > out_tim ){											// ���ɓ��̕����傫���ꍇ
		return( 1 );												// �G���[�Ƃ���
	}																
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| �o�b�N�A�b�v�f�[�^���X�g�A����                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Remote_Cal_Data_Restor( void )		                   |*/
/*| PARAMETER    : void					                                   |*/
/*| RETURN VALUE : void								 					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2009-06-02                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void Remote_Cal_Data_Restor( void ){

	if( CAL_WORK.mod ){			/* �ޯ����ߒ�?		*/
		ryoukin = CAL_WORK.ryoukin;
		total = CAL_WORK.total;
		req_rkn.syubt = (uchar)CAL_WORK.syubt;
		base_ryo = CAL_WORK.base_ryo;
		discount = CAL_WORK.discount;
		memcpy( &car_in, &CAL_WORK.car_in, sizeof( struct CAR_TIM )); /*�@���Ԏ����@*/
		memcpy( &car_ot, &CAL_WORK.car_ot, sizeof( struct CAR_TIM )); /*�@�o�Ԏ����@*/
		memcpy( &org_in, &CAL_WORK.org_in, sizeof( struct CAR_TIM )); /*�@���Ԏ����@*/
		memcpy( &org_ot, &CAL_WORK.org_ot, sizeof( struct CAR_TIM )); /*�@�o�Ԏ����@*/
		ntnet_nmax_flg = CAL_WORK.ntnet_nmax_bak;					/*�@�ő嗿���׸�ؽı	*/
		memcpy( &se_svt.stim, &CAL_WORK.stim, sizeof( struct	SVT	));
		memcpy( &carin_mt, &CAL_WORK.carin_mt, sizeof( struct CAR_TIM )); /*�@���Ԏ���(��)�@*/
		memcpy( &carot_mt, &CAL_WORK.carot_mt, sizeof( struct CAR_TIM )); /*�@�o�Ԏ���(��)�@*/
		memcpy( &nmax_in, &CAL_WORK.nmax_in, sizeof( struct CAR_TIM ));  /*�@�����ԍő��_�����@*/
		wk_strt = CAL_WORK.wk_strt;								  		/*�w�莞���ő�check�����ɉ��Z���鎞�ԁ@�@�@�@ */
		iti_flg = CAL_WORK.iti_flg;		  								/*�@��藿���уt���O�@�@�@�@�@�@�@�@�@�@�@�@�@*/
		amartim = CAL_WORK.amartim;		  								/*����藿���ї]�莞�ԁ@�@�@�@�@�@�@�@�@�@�@�@*/
		CAL_WORK.mod = 0;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ����i�[�͈̓`�F�b�N����			                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : SetCarInfoSelect			                               |*/
/*| PARAMETER    : index	�Ԏ������i�[���邽�߂�Index				   |*/
/*| RETURN VALUE : ret	0:�����͈�	1:�L���͈�						       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka	                                               |*/
/*| Date         : 2009-09-08                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	SetCarInfoSelect( short index )
{
	uchar ret=1;
	uchar param=1;
		
	param = GetCarInfoParam();
	
	switch( param ){
		case	0x01:			// CRA�ڑ��̂�
			if( index < BIKE_START_INDEX ){
				ret = 0;		// ����
			}
			break;
		case	0x02:			// IBC�ڑ��̂�
			if( index >= INT_CAR_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x03:			// IBC+CRA�ڑ����p
			if( index >= INT_CAR_START_INDEX && index < BIKE_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x04:			// CRR�ڑ��̂�
			if( index < INT_CAR_START_INDEX || index >= BIKE_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x05:			// CRA+CRR�ڑ����p
			if( index < INT_CAR_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x06:			// IBC+CRR�ڑ����p
			if( index >= BIKE_START_INDEX ){
				ret = 0;
			}
			break;
		case	0x07:			// IBC+CRA+CRR�ڑ����p
			if( index >= OLD_LOCK_MAX ){
				ret = 0;
			}
			break;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| �����v�Z�p�G���A�X�V                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : mc10exec( void )		         	                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN		 : void					                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : namioka                                                 |*/
/*| Date         : 2010-05-12                                              |*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	mc10exec( void )
{
	if( f_NTNET_RCV_MC10_EXEC ){				// mc10()�X�V���K�v�Ȑݒ肪�ʐM�ōX�V���ꂽ
		if( check_acceptable() ){				// mc10()���s�\����OK�H
			mc10();
			f_NTNET_RCV_MC10_EXEC = 0;
		}
	}
}

const char	Series_Name[12] = {"FT-4000FX   "};
/*[]----------------------------------------------------------------------[]*/
/*| ���Z�@�@����f�[�^(�ް����190)�쐬����                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data190                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2010-10-05                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	NTNET_Snd_Data190( void )
{

}
/*[]----------------------------------------------------------------------[]*/
/*| ���u�_�E�����[�h�v��(�ް����188)��M����                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData188                                        |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2010-10-12                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	NTNET_RevData188( void *data )
{

}
/*[]----------------------------------------------------------------------[]*/
/*| ���u�_�E�����[�h�����f�[�^(�ް����188)���M����		                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data189	                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka                                                 |*/
/*| Date         : 2010-04-28                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	NTNET_Snd_Data189( uchar err_code, uchar data )
{

}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�ް�(�ް����22,23)�ҏW����                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data22                                        |*/
/*| PARAMETER    : p_RcptDat(IN) ���Z���O�f�[�^(Receipt_data�^)�ւ̃|�C���^|*/
/*|              : p_NtDat  (IN) ���Z�f�[�^(DATA_KIND_22�^)�ւ̃|�C���^    |*/
/*| RETURN VALUE : ret        ���Z�f�[�^�T�C�Y(�V�X�e��ID�`)               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                             |*/
/*| Date         : 2012-02-13                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	NTNET_Edit_Data22( Receipt_data *p_RcptDat, DATA_KIND_22 *p_NtDat )
{
	uchar	i;
	uchar	j;
	ushort	ret;
// MH321800(S) �������8000�̃Z�b�g���@���C��
	ushort		wk_kind1, wk_kind2;
// MH321800(E) �������8000�̃Z�b�g���@���C��

	memset( p_NtDat, 0, sizeof( DATA_KIND_22 ) );

	p_NtDat->DataBasic.SystemID = p_RcptDat->DataBasic.SystemID;		// ����ID
	p_NtDat->DataBasic.DataKind = p_RcptDat->DataBasic.DataKind;		// �ް����
	p_NtDat->DataBasic.DataKeep = p_RcptDat->DataBasic.DataKeep;		// �ް��ێ��׸�
	p_NtDat->DataBasic.ParkingNo = p_RcptDat->DataBasic.ParkingNo;		// ���ԏꇂ
	p_NtDat->DataBasic.ModelCode = p_RcptDat->DataBasic.ModelCode;		// �@����
	p_NtDat->DataBasic.MachineNo = p_RcptDat->DataBasic.MachineNo;		// �@�B��
	memcpy( &p_NtDat->DataBasic.Year, &p_RcptDat->DataBasic.Year, 6 );	// �����N���������b

	p_NtDat->PayCount = CountSel( &p_RcptDat->Oiban);					// ���Zor���Z���~�ǂ���
	p_NtDat->PayMethod = p_RcptDat->PayMethod;							// ���Z���@
	p_NtDat->PayClass = p_RcptDat->PayClass;							// �����敪
	p_NtDat->PayMode = p_RcptDat->PayMode;								// ���ZӰ��(�������Z)

// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->LockNo = p_RcptDat->WPlace;							// �����
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ������������� 2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	p_NtDat->CardType = 0;												// ���Ԍ�����
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	p_NtDat->CMachineNo = 0;											// ���Ԍ��@�B��
	p_NtDat->CMachineNo	= p_RcptDat->CMachineNo;						// ���Ԍ��@�B��
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	p_NtDat->CardNo = 0L;												// ���Ԍ��ԍ�(�����ǂ���)
	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->OutTime.Year = p_RcptDat->TOutTime.Year;				// �o�ɔN
		p_NtDat->OutTime.Mon = p_RcptDat->TOutTime.Mon;					// �o�Ɍ�
		p_NtDat->OutTime.Day = p_RcptDat->TOutTime.Day;					// �o�ɓ�
		p_NtDat->OutTime.Hour = p_RcptDat->TOutTime.Hour;				// �o�Ɏ�
		p_NtDat->OutTime.Min = p_RcptDat->TOutTime.Min;					// �o�ɕ�
		p_NtDat->OutTime.Sec = 0;										// �o�ɕb
		p_NtDat->KakariNo = p_RcptDat->KakariNo;						// �W����
		p_NtDat->OutKind = p_RcptDat->OutKind;							// ���Z�o��
	}
	p_NtDat->ReceiptIssue = p_RcptDat->ReceiptIssue;					// �̎��ؔ��s�L��
	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->InTime.Year = p_RcptDat->TInTime.Year;					// ���ɔN
		p_NtDat->InTime.Mon = p_RcptDat->TInTime.Mon;					// ���Ɍ�
		p_NtDat->InTime.Day = p_RcptDat->TInTime.Day;					// ���ɓ�
		p_NtDat->InTime.Hour = p_RcptDat->TInTime.Hour;					// ���Ɏ�
		p_NtDat->InTime.Min = p_RcptDat->TInTime.Min;					// ���ɕ�
		p_NtDat->InTime.Sec = 0;										// ���ɕb
	}
	// ���Z�N���������b��0�Ƃ���
	// �O�񐸎Z�N���������b��0�Ƃ���
	p_NtDat->TaxPrice = 0;												// �ېőΏۊz
	p_NtDat->TotalPrice = 0;											// ���v���z(HOST���g�p�̂���)
	p_NtDat->Tax = p_RcptDat->Wtax;										// ����Ŋz
	if( p_NtDat->PayMethod != 5 ){										// �X�V���Z�ȊO
		p_NtDat->Syubet = p_RcptDat->syu;								// �������
		p_NtDat->Price = p_RcptDat->WPrice;								// ���ԗ���
	}
	p_NtDat->CashPrice =
						p_RcptDat->WInPrice - p_RcptDat->WChgPrice;		// ��������
	p_NtDat->InPrice = p_RcptDat->WInPrice;								// �������z
	p_NtDat->ChgPrice = p_RcptDat->WChgPrice;							// �ޑK���z
	p_NtDat->Fusoku = p_RcptDat->WFusoku;								// �ޑK���o�s�����z
	p_NtDat->FusokuFlg = 0;												// �ޑK���o�s�������׸�
	p_NtDat->PayObsFlg = 0;												// ���Z����Q�����׸�
	p_NtDat->ChgOverFlg = 0;											// ���ߏ���z���ް�����׸�

	p_NtDat->PassCheck = p_RcptDat->PassCheck;							// ����߽����
	p_NtDat->CountSet = p_RcptDat->CountSet;							// �ݎԶ��Ă��Ȃ�
	if( p_RcptDat->teiki.ParkingNo ){									// ��������Z����or������X�V��
		p_NtDat->PassData.ParkingNo = p_RcptDat->teiki.ParkingNo;		// ��������ԏꇂ
		p_NtDat->PassData.PassID = p_RcptDat->teiki.id;					// �����ID
		p_NtDat->PassData.Syubet = p_RcptDat->teiki.syu;				// ��������
		p_NtDat->PassData.State = p_RcptDat->teiki.status;				// ������ð��
		p_NtDat->PassData.MoveMode = 0;									// ������ԋp
		p_NtDat->PassData.ReadMode = 0;									// �����ذ��ײ�
		p_NtDat->PassData.SYear =
								(uchar)(p_RcptDat->teiki.s_year%100);	// ������J�n�N
		p_NtDat->PassData.SMon = p_RcptDat->teiki.s_mon;				// ������J�n��
		p_NtDat->PassData.SDate = p_RcptDat->teiki.s_day;				// ������J�n��
		p_NtDat->PassData.EYear =
								(uchar)(p_RcptDat->teiki.e_year%100);	// ������I���N
		p_NtDat->PassData.EMon = p_RcptDat->teiki.e_mon;				// ������I����
		p_NtDat->PassData.EDate = p_RcptDat->teiki.e_day;				// ������I����
	}
	if( p_NtDat->PayMethod == 5 ){
		p_NtDat->PassRenewalPric = p_RcptDat->WPrice;					// ������X�V����

		p_NtDat->PassRenewalCondition = 1;								// ������X�V����
		if( rangechk( 1, 3, p_RcptDat->teiki.status ) ){				// �ð��1�`3�͈͓�
			p_NtDat->PassRenewalCondition = 11;							// ������X�V����
		}
		p_NtDat->PassRenewalPeriod = p_RcptDat->teiki.update_mon;		// ������X�V����(�X�V����)
	}

// MH810103 GG119202(S) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
//	if (ntnet_decision_credit(&p_RcptDat->credit)) {
	if (ntnet_decision_credit(&p_RcptDat->credit) &&
		p_NtDat->PayClass != 2 && p_NtDat->PayClass != 3 ){					// ���Z���~�A�Đ��Z���~�ł͂Ȃ�
// MH810103 GG119202(E) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
//	memcpy( p_NtDat->CreditCardNo,
//			p_RcptDat->credit.card_no,
//			sizeof( p_NtDat->CreditCardNo ) );							// �ڼޯĶ��މ����
// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
	if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
	    p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
		// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.��0x20h���߂���B
		// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
		memset( &p_NtDat->CreditCardNo[0], 0x20, sizeof(p_NtDat->CreditCardNo) );
	}
	else {
// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
	memcpyFlushLeft( &p_NtDat->CreditCardNo[0],
					 (uchar *)&p_RcptDat->credit.card_no[0],
					 sizeof( p_NtDat->CreditCardNo ),
					 sizeof( p_RcptDat->credit.card_no ) );				// �ڼޯĶ��މ����
// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
	}
// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	// ���No�̏�7���ځ`��5���ڈȊO��'*'���i�[����Ă�����'0'�ɒu��
	change_CharInArray( &p_NtDat->CreditCardNo[0], sizeof(p_NtDat->CreditCardNo), 7, 5, '*', '0' );
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�	
	p_NtDat->Credit_ryo = p_RcptDat->credit.pay_ryo;					// �ڼޯĶ��ޗ��p���z
	p_NtDat->CreditSlipNo = p_RcptDat->credit.slip_no;					// �ڼޯĶ��ޓ`�[�ԍ�
	p_NtDat->CreditAppNo = p_RcptDat->credit.app_no;					// �ڼޯĶ��ޏ��F�ԍ�
// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
//	memcpy( p_NtDat->CreditName,
//			p_RcptDat->credit.card_name,
//			sizeof( p_NtDat->CreditName ) );							// �ڼޯĶ��މ�Ж�
	memcpyFlushLeft( &p_NtDat->CreditName[0],
					 (uchar *)&p_RcptDat->credit.card_name[0],
					 sizeof( p_NtDat->CreditName ),
					 sizeof( p_RcptDat->credit.card_name ) );			// �ڼޯĶ��މ�Ж�
// MH321800(E) NT-NET���Z�f�[�^�d�l�ύX
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	p_NtDat->CreditDate[0] = p_RcptDat->credit.CreditDate[0];			// �ڼޯĶ��ޗL������(�N)
//	p_NtDat->CreditDate[1] = p_RcptDat->credit.CreditDate[1];			// �ڼޯĶ��ޗL������(��)
//	p_NtDat->CreditProcessNo = p_RcptDat->credit.CenterProcOiBan;		// �ڼޯĶ��޾�������ǂ���
	// �����I��0���Z�b�g
	p_NtDat->CreditDate[0] = 0;											// �ڼޯĶ��ޗL������(�N)
	p_NtDat->CreditDate[1] = 0;											// �ڼޯĶ��ޗL������(��)
	p_NtDat->CreditProcessNo = 0;										// �ڼޯĶ��޾�������ǂ���
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
	}

	for( i = j = 0; i < WTIK_USEMAX; i++ ){								// ���Z���~�ȊO�̊������R�s�[
		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&				// ������ʂ���
		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||	// ���Z���~�������łȂ�
		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 			memcpy( &p_NtDat->DiscountData[j], &p_RcptDat->DiscountData[i], sizeof( DISCOUNT_DATA ) );	// �������
// 			if( p_NtDat->DiscountData[j].DiscSyu == NTNET_PRI_W ){
// 				memset( &p_NtDat->DiscountData[j].uDiscData, 0, 8 );	// �s�v�f�[�^�N���A
// 			}
// 			j++;
			if ( p_RcptDat->DiscountData[i].Discount || p_RcptDat->DiscountData[i].DiscCount ) {
				p_NtDat->DiscountData[j].ParkingNo = p_RcptDat->DiscountData[i].ParkingNo;
				p_NtDat->DiscountData[j].DiscSyu   = p_RcptDat->DiscountData[i].DiscSyu;
				p_NtDat->DiscountData[j].DiscNo    = p_RcptDat->DiscountData[i].DiscNo;
				p_NtDat->DiscountData[j].DiscCount = p_RcptDat->DiscountData[i].DiscCount;
				p_NtDat->DiscountData[j].Discount  = p_RcptDat->DiscountData[i].Discount;
				p_NtDat->DiscountData[j].DiscInfo1 = p_RcptDat->DiscountData[i].DiscInfo1;
				p_NtDat->DiscountData[j].uDiscData.common.DiscInfo2 =
					p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;
				p_NtDat->DiscountData[j].uDiscData.common.DiscFlg = 0;
				switch ( p_RcptDat->DiscountData[i].DiscSyu ) {
				case NTNET_SVS_M:			// �T�[�r�X������(���z)
				case NTNET_SVS_T:			// �T�[�r�X������(����)
				case NTNET_KAK_M:			// �X����(���z)
				case NTNET_KAK_T:			// �X����(����)
				case NTNET_TKAK_M:			// ���X�܊���(���z)
				case NTNET_TKAK_T:			// ���X�܊���(����)
				case NTNET_SYUBET:			// ��ʊ���(���z)
				case NTNET_SYUBET_TIME:		// ��ʊ���(����)
				case NTNET_SHOP_DISC_AMT:	// ��������(���z)
				case NTNET_SHOP_DISC_TIME:	// ��������(����)
				case NTNET_GENGAKU:			// ���z���Z
				case NTNET_FURIKAE:			// �U�֐��Z
				case NTNET_SYUSEI_1:		// �C�����Z
				case NTNET_SYUSEI_2:		// �C�����Z
				case NTNET_SYUSEI_3:		// �C�����Z
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 1;
					break;
				case NTNET_PRI_W:			// �����v���y�C�h�J�[�h
					memset(&p_NtDat->DiscountData[j].uDiscData, 0, 8);	// �s�v�f�[�^�N���A
					break;
				default:
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 0;
					break;
				}
				j++;
			}

			if ( p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscount ||
				 p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscCount ) {
				p_NtDat->DiscountData[j].ParkingNo = p_RcptDat->DiscountData[i].ParkingNo;
				p_NtDat->DiscountData[j].DiscSyu   = p_RcptDat->DiscountData[i].DiscSyu;
				p_NtDat->DiscountData[j].DiscNo    = p_RcptDat->DiscountData[i].DiscNo;
				p_NtDat->DiscountData[j].DiscCount = p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscCount;
				p_NtDat->DiscountData[j].Discount  = p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscount;
				p_NtDat->DiscountData[j].DiscInfo1 = p_RcptDat->DiscountData[i].DiscInfo1;
				p_NtDat->DiscountData[j].uDiscData.common.DiscInfo2 =
					p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;
				p_NtDat->DiscountData[j].uDiscData.common.DiscFlg = 1;
				switch ( p_RcptDat->DiscountData[i].DiscSyu ) {
				case NTNET_SVS_M:			// �T�[�r�X������(���z)
				case NTNET_SVS_T:			// �T�[�r�X������(����)
				case NTNET_KAK_M:			// �X����(���z)
				case NTNET_KAK_T:			// �X����(����)
				case NTNET_TKAK_M:			// ���X�܊���(���z)
				case NTNET_TKAK_T:			// ���X�܊���(����)
				case NTNET_SYUBET:			// ��ʊ���(���z)
				case NTNET_SYUBET_TIME:		// ��ʊ���(����)
				case NTNET_SHOP_DISC_AMT:	// ��������(���z)
				case NTNET_SHOP_DISC_TIME:	// ��������(����)
				case NTNET_GENGAKU:			// ���z���Z
				case NTNET_FURIKAE:			// �U�֐��Z
				case NTNET_SYUSEI_1:		// �C�����Z
				case NTNET_SYUSEI_2:		// �C�����Z
				case NTNET_SYUSEI_3:		// �C�����Z
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 1;
					break;
				case NTNET_PRI_W:			// �����v���y�C�h�J�[�h
					memset(&p_NtDat->DiscountData[j].uDiscData, 0, 8);	// �s�v�f�[�^�N���A
					break;
				default:
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 0;
					break;
				}
				j++;
			}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
		}
	}
// MH321800(S) �������8000�̃Z�b�g���@���C��
	// ���σ��[�_�ڑ�����
	if (isEC_USE()) {
// MH810103 GG119202(S) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
		// ���Z���~�A�Đ��Z���~���͊������ɃZ�b�g���Ȃ�
		if (p_NtDat->PayClass != 2 && p_NtDat->PayClass != 3) {
// MH810103 GG119202(E) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
		if (p_RcptDat->credit.pay_ryo != 0) {
			// �N���W�b�g����
			/* �󂫴ر���� */
			for (i = 0; (i < NTNET_DIC_MAX) &&
					(0 != p_NtDat->DiscountData[i].ParkingNo); i++) {
				;
			}		/* �f�[�^�������				*/
			if (i < NTNET_DIC_MAX) {
				p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];		// ���ԏ�No.
				p_NtDat->DiscountData[i].DiscSyu = NTNET_INQUIRY_NUM;		// �������
				// �N���W�b�g���ώ��͖₢���킹�ԍ�����M���Ȃ��̂�0x20���߂Ƃ���
				memset(&p_NtDat->DiscountData[i].DiscNo, 0x20, 16);
				p_NtDat->DiscountData[i].uDiscData.common.MoveMode = 1;
				p_NtDat->DiscountData[i].uDiscData.common.DiscFlg = 0;
// MH810103 (s) �d�q�}�l�[�Ή�
				i++;
				j++;
// MH810103 (e) �d�q�}�l�[�Ή�
			}
		}
		else if (p_RcptDat->Electron_data.Ec.pay_ryo != 0) {
			// �d�q�}�l�[����
			/* �󂫴ر���� */
			for (i = 0; (i < NTNET_DIC_MAX) &&
					(0 != p_NtDat->DiscountData[i].ParkingNo); i++) {
				;
			}		/* �f�[�^�������				*/
			if (i < (NTNET_DIC_MAX-2)) {
				switch (p_RcptDat->Electron_data.Ec.e_pay_kind) {			// ���ώ�ʂ���U�蕪��
				case	EC_EDY_USED:
					wk_kind1 = NTNET_EDY_1;									// ������ʁFEdy���ޔԍ�
					wk_kind2 = NTNET_EDY_2;									// ������ʁFEdy�x���z�A�c�z
					break;
				case	EC_NANACO_USED:
					wk_kind1 = NTNET_NANACO_1;								// ������ʁFnanaco���ޔԍ�
					wk_kind2 = NTNET_NANACO_2;								// ������ʁFnanaco�x���z�A�c�z
					break;
				case	EC_WAON_USED:
					wk_kind1 = NTNET_WAON_1;								// ������ʁFWAON���ޔԍ�
					wk_kind2 = NTNET_WAON_2;								// ������ʁFWAON�x���z�A�c�z
					break;
				case	EC_SAPICA_USED:
					wk_kind1 = NTNET_SAPICA_1;								// ������ʁFSAPICA���ޔԍ�
					wk_kind2 = NTNET_SAPICA_2;								// ������ʁFSAPICA�x���z�A�c�z
					break;
				case	EC_KOUTSUU_USED:
					// ��ʌnIC�J�[�h��Suica��ID���g�p����
					wk_kind1 = NTNET_SUICA_1;								// ������ʁFSuica���ޔԍ�
					wk_kind2 = NTNET_SUICA_2;								// ������ʁFSuica�x���z�A�c�z
					break;
				case	EC_ID_USED:
					wk_kind1 = NTNET_ID_1;									// ������ʁFiD���ޔԍ�
					wk_kind2 = NTNET_ID_2;									// ������ʁFiD�x���z�A�c�z
					break;
				case	EC_QUIC_PAY_USED:
					wk_kind1 = NTNET_QUICPAY_1;								// ������ʁFQUICPay���ޔԍ�
					wk_kind2 = NTNET_QUICPAY_2;								// ������ʁFQUICPay�x���z�A�c�z
					break;
				default:
					wk_kind1 = 0;
					wk_kind2 = 0;
					break;
				}

				if (wk_kind1 != 0 && wk_kind2 != 0) {
					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// ���ԏ�No.
					p_NtDat->DiscountData[i].DiscSyu = wk_kind1;			// �������
					// �J�[�h�ԍ�
// MH810103 GG119202(S) �d�q�}�l�[�Ή�
//					memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Ec.Card_ID, 16);
// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
///					memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Ec.Card_ID, sizeof(p_RcptDat->Electron_data.Ec.Card_ID));
					if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
	    				p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
						// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.���J�[�h������0x30���߂����l�߂�ZZ����B
						// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
						memset( &p_NtDat->DiscountData[i].DiscNo, 0x20, sizeof(p_RcptDat->Electron_data.Ec.Card_ID) );
						// ���ꂼ��̃J�[�h������0x30����
						memset( &p_NtDat->DiscountData[i].DiscNo, 0x30, (size_t)(EcBrandEmoney_Digit[p_RcptDat->Electron_data.Ec.e_pay_kind - EC_USED]) );
						// ���l�߂�ZZ
						memset( &p_NtDat->DiscountData[i].DiscNo, 'Z', 2 );
					}
					else {
						memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Ec.Card_ID, sizeof(p_RcptDat->Electron_data.Ec.Card_ID));
					}
// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
// MH810103 GG119202(S) �d�q�}�l�[�J�[�h�ԍ��ϊ�����
					switch (p_RcptDat->Electron_data.Ec.e_pay_kind) {
					case EC_KOUTSUU_USED:
						// �t�F�[�Y1�t�H�[�}�b�g�ł͓d�q�}�l�[�̃J�[�h�ԍ���'*'->'0'�ϊ�����
						change_CharInArray((uchar*)&p_NtDat->DiscountData[i].DiscNo, 20, 3, 5, '*', '0');
						break;
					case EC_EDY_USED:
					case EC_NANACO_USED:
					case EC_WAON_USED:
					case EC_ID_USED:
					case EC_QUIC_PAY_USED:
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
						// �t�F�[�Y1�t�H�[�}�b�g�ł͓d�q�}�l�[�̃J�[�h�ԍ���'*'->'0'�ϊ�����
						change_CharInArray((uchar*)&p_NtDat->DiscountData[i].DiscNo, 20, 1, 5, '*', '0');
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
						break;
					default:
						break;
					}
// MH810103 GG119202(E) �d�q�}�l�[�J�[�h�ԍ��ϊ�����
					i++;
// MH810103 (s) �d�q�}�l�[�Ή�
					j++;
// MH810103 (e) �d�q�}�l�[�Ή�

					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// ���ԏ�No.
					p_NtDat->DiscountData[i].DiscSyu = wk_kind2;			// �������
					p_NtDat->DiscountData[i].DiscNo = 0;					// �����敪
					p_NtDat->DiscountData[i].DiscCount = 1;					// �g�p����
					p_NtDat->DiscountData[i].Discount = p_RcptDat->Electron_data.Ec.pay_ryo;	// �x�����z
					p_NtDat->DiscountData[i].DiscInfo1 = p_RcptDat->Electron_data.Ec.pay_after;	// �c�z
					switch (p_RcptDat->Electron_data.Ec.e_pay_kind) {		// ���ώ�ʂ���U�蕪��
					case	EC_EDY_USED:
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = p_RcptDat->Electron_data.Ec.Brand.Edy.DealNo;			// Edy����ʔ�
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = astoinl(p_RcptDat->Electron_data.Ec.Brand.Edy.CardDealNo, 5);	// �J�[�h����ʔ�
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
						break;
					case	EC_NANACO_USED:
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = p_RcptDat->Electron_data.Ec.Brand.Nanaco.DealNo;		// nanaco����ʔ�
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = astoinl(p_RcptDat->Electron_data.Ec.Brand.Nanaco.DealNo, 6);	// �[������ʔ�
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
						break;
					case	EC_WAON_USED:
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = p_RcptDat->Electron_data.Ec.Brand.Waon.point;			// WAON����t�^�|�C���g
						break;
					case	EC_SAPICA_USED:
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = p_RcptDat->Electron_data.Ec.Brand.Sapica.Details_ID;	// SAPICA�ꌏ����ID
						break;
// MH810104 GG119202(S) nanaco�EiD�EQUICPay�Ή�
					case	EC_ID_USED:
						// �c�z���ʒm����邱�Ƃ͂Ȃ����A�O�̂��ߊ������1���N���A����
						p_NtDat->DiscountData[i].DiscInfo1 = 0;
						// �Z�b�g���鍀�ڂȂ�
						break;
					case	EC_QUIC_PAY_USED:
						// �`�[�ԍ���0���Z�b�g
						p_NtDat->DiscountData[i].DiscInfo1 = 0;
						// ����ʔԂ�0���Z�b�g
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = 0;
						break;
// MH810104 GG119202(E) nanaco�EiD�EQUICPay�Ή�
					default:
						p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = 0;
						break;
					}
					p_NtDat->DiscountData[i].uDiscData.common.MoveMode = 0;
					p_NtDat->DiscountData[i].uDiscData.common.DiscFlg = 0;
					i++;
// MH810103 (s) �d�q�}�l�[�Ή�
					j++;
// MH810103 (e) �d�q�}�l�[�Ή�

					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// ���ԏ�No.
					p_NtDat->DiscountData[i].DiscSyu = NTNET_INQUIRY_NUM;	// �������
// MH810103 GG119202(S) �d�q�}�l�[�Ή�
//					memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Ec.inquiry_num, 16);
					memcpyFlushLeft( (uchar *)&p_NtDat->DiscountData[i].DiscNo, (uchar *)&p_RcptDat->Electron_data.Ec.inquiry_num[0],
										16, sizeof( p_RcptDat->Electron_data.Ec.inquiry_num ) );	// �₢���킹�ԍ�
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
					p_NtDat->DiscountData[i].uDiscData.common.MoveMode = 1;
					p_NtDat->DiscountData[i].uDiscData.common.DiscFlg = 0;
// MH810103 (s) �d�q�}�l�[�Ή�
					i++;
					j++;
// MH810103 (e) �d�q�}�l�[�Ή�
				}
			}
		}
// MH810103 GG119202(S) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
		}
// MH810103 GG119202(E) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
	}
	else if (isSX10_USE()) {
		if (p_RcptDat->Electron_data.Suica.pay_ryo != 0) {
			/* �󂫴ر���� */
			for (i = 0; (i < NTNET_DIC_MAX) &&
					(0 != p_NtDat->DiscountData[i].ParkingNo); i++) {
				;
			}		/* �f�[�^�������				*/
			if (i < (NTNET_DIC_MAX-1)) {
				switch(Ope_Disp_Media_Getsub(1)) {
				case	OPE_DISP_MEDIA_TYPE_SUICA:
					wk_kind1 = NTNET_SUICA_1;								// ������ʁFSuica���ޔԍ�
					wk_kind2 = NTNET_SUICA_2;								// ������ʁFSuica�x���z�A�c�z
					break;
				case	OPE_DISP_MEDIA_TYPE_PASMO:
					wk_kind1 = NTNET_PASMO_1;								// ������ʁFPASMO���ޔԍ�
					wk_kind2 = NTNET_PASMO_2;								// ������ʁFPASMO�x���z�A�c�z
					break;
				case	OPE_DISP_MEDIA_TYPE_ICOCA:
					wk_kind1 = NTNET_ICOCA_1;								// ������ʁFICOCA���ޔԍ�
					wk_kind2 = NTNET_ICOCA_2;								// ������ʁFICOCA�x���z�A�c�z
					break;
				case	OPE_DISP_MEDIA_TYPE_ICCARD:
					wk_kind1 = NTNET_ICCARD_1;								// ������ʁFIC-Card���ޔԍ�
					wk_kind2 = NTNET_ICCARD_2;								// ������ʁFIC-Card�x���z�A�c�z
					break;
				default:
					wk_kind1 = 0;
					wk_kind2 = 0;
					break;
				}

				if (wk_kind1 != 0 && wk_kind2 != 0) {
					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// ���ԏ�No.
					p_NtDat->DiscountData[i].DiscSyu = wk_kind1;			// �������
					// �J�[�h�ԍ�
					memcpy(&p_NtDat->DiscountData[i].DiscNo, p_RcptDat->Electron_data.Suica.Card_ID, 16);
					i++;
// MH810103 (s) �d�q�}�l�[�Ή�
					j++;
// MH810103 (e) �d�q�}�l�[�Ή�

					p_NtDat->DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	// ���ԏ�No.
					p_NtDat->DiscountData[i].DiscSyu = wk_kind2;			// �������
					p_NtDat->DiscountData[i].DiscNo = 0;					// �����敪
					p_NtDat->DiscountData[i].DiscCount = 1;					// �g�p����
					p_NtDat->DiscountData[i].Discount = p_RcptDat->Electron_data.Suica.pay_ryo;		// �x�����z
					p_NtDat->DiscountData[i].DiscInfo1 = p_RcptDat->Electron_data.Suica.pay_after;	// �c�z
					p_NtDat->DiscountData[i].uDiscData.common.DiscInfo2 = 0;
					p_NtDat->DiscountData[i].uDiscData.common.MoveMode = 0;
					p_NtDat->DiscountData[i].uDiscData.common.DiscFlg = 0;
// MH810103 (s) �d�q�}�l�[�Ή�
					i++;
					j++;
// MH810103 (e) �d�q�}�l�[�Ή�
				}
			}
		}
	}
// MH321800(E) �������8000�̃Z�b�g���@���C��

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 	ret = sizeof( DATA_KIND_22 ) - sizeof( DISCOUNT_DATA )*NTNET_DIC_MAX;// �����f�[�^�܂ł̃T�C�Y�Z�o
	ret = sizeof(DATA_KIND_22) - sizeof(t_SeisanDiscountOld) * NTNET_DIC_MAX;	// �����f�[�^�܂ł̃T�C�Y�Z�o
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	ret += (ushort)sizeof( DISCOUNT_DATA )*j;							// �Z�b�g���������f�[�^�T�C�Y�����Z

	return ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�ް�(�ް����22,23)�ҏW����(�s�������p)                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Edit_Data22_SK                                    |*/
/*| PARAMETER    : p_RcptDat(IN) ���Z���O�f�[�^(Receipt_data�^)�ւ̃|�C���^|*/
/*|              : p_NtDat  (IN) ���Z�f�[�^(DATA_KIND_22�^)�ւ̃|�C���^    |*/
/*| RETURN VALUE : ret        ���Z�f�[�^�T�C�Y(�V�X�e��ID�`)               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                             |*/
/*| Date         : 2012-02-16                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	NTNET_Edit_Data22_SK( Receipt_data *p_RcptDat, DATA_KIND_22 *p_NtDat )
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_22 ) );

	p_NtDat->DataBasic.SystemID = p_RcptDat->DataBasic.SystemID;		// ����ID
	p_NtDat->DataBasic.DataKind = p_RcptDat->DataBasic.DataKind;		// �ް����
	p_NtDat->DataBasic.DataKeep = p_RcptDat->DataBasic.DataKeep;		// �ް��ێ��׸�
	p_NtDat->DataBasic.ParkingNo = p_RcptDat->DataBasic.ParkingNo;		// ���ԏꇂ
	p_NtDat->DataBasic.ModelCode = p_RcptDat->DataBasic.ModelCode;		// �@����
	p_NtDat->DataBasic.MachineNo = p_RcptDat->DataBasic.MachineNo;		// �@�B��
	memcpy( &p_NtDat->DataBasic.Year, &p_RcptDat->DataBasic.Year, 6 );	// �����N���������b

	p_NtDat->PayCount = CountSel( &p_RcptDat->Oiban );					// ���Zor���Z���~�ǂ���
	p_NtDat->PayMethod = p_RcptDat->PayMethod;							// ���Z���@
	p_NtDat->PayClass = p_RcptDat->PayClass;							// �����敪
	p_NtDat->PayMode = 0;												// ���ZӰ��(�������Z)
	p_NtDat->LockNo = p_RcptDat->WPlace;								// �����
	p_NtDat->CardType = 0;												// ���Ԍ�����
	p_NtDat->CMachineNo = 0;											// ���Ԍ��@�B��
	p_NtDat->CardNo = 0L;												// ���Ԍ��ԍ�(�����ǂ���)
	p_NtDat->OutTime.Year = p_RcptDat->TOutTime.Year;					// �o�ɔN
	p_NtDat->OutTime.Mon = p_RcptDat->TOutTime.Mon;						// �o�Ɍ�
	p_NtDat->OutTime.Day = p_RcptDat->TOutTime.Day;						// �o�ɓ�
	p_NtDat->OutTime.Hour = p_RcptDat->TOutTime.Hour;					// �o�Ɏ�
	p_NtDat->OutTime.Min = p_RcptDat->TOutTime.Min;						// �o�ɕ�
	p_NtDat->OutTime.Sec = 0;											// �o�ɕb
	p_NtDat->KakariNo = p_RcptDat->KakariNo;							// �W����
	p_NtDat->OutKind = p_RcptDat->OutKind;								// ���Z�o��
	p_NtDat->InTime.Year = p_RcptDat->TInTime.Year;						// ���ɔN
	p_NtDat->InTime.Mon = p_RcptDat->TInTime.Mon;						// ���Ɍ�
	p_NtDat->InTime.Day = p_RcptDat->TInTime.Day;						// ���ɓ�
	p_NtDat->InTime.Hour = p_RcptDat->TInTime.Hour;						// ���Ɏ�
	p_NtDat->InTime.Min = p_RcptDat->TInTime.Min;						// ���ɕ�
	p_NtDat->InTime.Sec = 0;											// ���ɕb
	// ���Z�N���������b��0�Ƃ���
	// �O�񐸎Z�N���������b��0�Ƃ���
	p_NtDat->TaxPrice = 0;												// �ېőΏۊz
	p_NtDat->TotalPrice = 0;											// ���v���z(HOST���g�p�̂���)
	p_NtDat->Tax = p_RcptDat->Wtax;										// ����Ŋz
	p_NtDat->Syubet = p_RcptDat->syu;									// �������
	p_NtDat->Price = p_RcptDat->WPrice;									// ���ԗ���
	p_NtDat->PassCheck = 1;												// ����߽����
	p_NtDat->CountSet = p_RcptDat->CountSet;							// �ݎԶ��Ă��Ȃ�

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 	ret = sizeof( DATA_KIND_22 ) - sizeof( DISCOUNT_DATA )*NTNET_DIC_MAX;// �����f�[�^�܂ł̃T�C�Y�Z�o
	ret = sizeof(DATA_KIND_22) - sizeof(t_SeisanDiscountOld) * NTNET_DIC_MAX;	// �����f�[�^�܂ł̃T�C�Y�Z�o
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]

	return ret;
}
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
////[]----------------------------------------------------------------------[]
/////	@brief			���Ƀ��O�f�[�^�쐬(LOG�ɕۑ�����`��)
////[]----------------------------------------------------------------------[]
/////	@param[in]		pr_lokno : ���������p���Ԉʒu�ԍ�
/////	@return			woid	: 
/////	@author			A.iiizumi
/////	@attention		
/////	@note			
////[]----------------------------------------------------------------------[]
/////	@date			Create	: 2012/02/15<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
//void	Make_Log_Enter( unsigned short pr_lokno)
//{
//	ushort	lkno;
//	int i, j;
//	ushort curnum = 0;
//	uchar uc_prm;
//	ushort curnum_1 = 0;
//	ushort curnum_2 = 0;
//	ushort curnum_3 = 0;
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//	date_time_rec	wk_CLK_REC;
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//
//	lkno = pr_lokno - 1;
//
//	memset( &Enter_data, 0, sizeof( Enter_data ) );
//
//	Enter_data.CMN_DT.DT_54.ID = 20;
//	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
//		Enter_data.CMN_DT.DT_54.ID = 54;
//	}
//
//	Enter_data.InCount = NTNetDataCont[0];											// ���ɒǂ���
//	NTNetDataCont[0]++;																// ���ɒǂ���+1
//
//	Enter_data.Syubet = LockInfo[lkno].ryo_syu;										// �����敪
//
//	if( FLAPDT.flp_data[lkno].lag_to_in.BIT.SYUUS == 1 ){							// �C�����Z�̎�
//		Enter_data.InMode = 6;														// ����Ӱ�ށF�C�����Z����
//	}else{
//		if(FLAPDT.flp_data[lkno].lag_to_in.BIT.FUKUG != 0){							// ����Ӱ��
//			Enter_data.InMode = 5;
//		} else {
//			Enter_data.InMode = 0;
//		}
//	}
//
//	Enter_data.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
//								+ LockInfo[lkno].posi );							// �����
//	Enter_data.CardType = 0;														// ���Ԍ�����(���g�p)
//	Enter_data.CMachineNo = 0;														// ���Ԍ��@�B��
//	Enter_data.CardNo = 0L;															// ���Ԍ��ԍ�(�����ǂ���)
//
//	Enter_data.InTime.Year = FLAPDT.flp_data[lkno].year;							// ���ɔN
//	Enter_data.InTime.Mon = FLAPDT.flp_data[lkno].mont;								// ���Ɍ�
//	Enter_data.InTime.Day = FLAPDT.flp_data[lkno].date;								// ���ɓ�
//	Enter_data.InTime.Hour = FLAPDT.flp_data[lkno].hour;							// ���Ɏ�
//	Enter_data.InTime.Min = FLAPDT.flp_data[lkno].minu;								// ���ɕ�
//	Enter_data.InTime.Sec = 0;														// ���ɕb
//	// �����N�����������œ����
//	Enter_data.CMN_DT.DT_54.ProcDate.Year	= (uchar)( CLK_REC.year % 100 );		// �N
//	Enter_data.CMN_DT.DT_54.ProcDate.Mon	= (uchar)CLK_REC.mont;					// ��
//	Enter_data.CMN_DT.DT_54.ProcDate.Day	= (uchar)CLK_REC.date;					// ��
//	Enter_data.CMN_DT.DT_54.ProcDate.Hour	= (uchar)CLK_REC.hour;					// ��
//	Enter_data.CMN_DT.DT_54.ProcDate.Min	= (uchar)CLK_REC.minu;					// ��
//	Enter_data.CMN_DT.DT_54.ProcDate.Sec	= (uchar)CLK_REC.seco;					// �b
//	Enter_data.PassCheck = 1;														// ����߽�������Ȃ�
//	if( prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[lkno].ryo_syu-1)*6)),1,1 ) ){	// ��ʖ����Ă���ݒ�?
//		Enter_data.CountSet = 0;													// �ݎԶ���(����)
//	}else{
//		Enter_data.CountSet = 1;													// �ݎԶ���(���Ȃ�)
//	}
//	// ������ް���0�Œ�
//	Enter_data.CMN_DT.DT_54.PascarCnt = 0;		// ����ԗ��J�E���g
//	if (Enter_data.CMN_DT.DT_54.ID != 54) {		// 20/54
//		return;
//	}
//	for (i = 0; i < LOCK_MAX; i++) {
//		// ������ʖ��Ɍ��ݍݎԶ���
//		if (FLAPDT.flp_data[i].nstat.bits.b00) {
//			j = LockInfo[i].ryo_syu - 1;
//			if (j >= 0 && j < 12) {
//				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );// ��ʖ����Ă���ݒ�get
//				if( uc_prm == 1 ){			// ���ݑ䐔�P���J�E���g
//					curnum++;
//					curnum_1++;
//				}
//				else if( uc_prm == 2 ){		// ���ݑ䐔�Q���J�E���g
//					curnum++;
//					curnum_2++;
//				}
//				else if( uc_prm == 3 ){		// ���ݑ䐔�R���J�E���g
//					curnum++;
//					curnum_3++;
//				}
//			}
//		}
//	}
//	// ����ԗ��J�E���g 0�Œ�
//	if (_is_ntnet_remote()) {												// ���uNT-NET�ݒ�̂Ƃ��̂݃f�[�^�Z�b�g
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039�D���ԑ䐔��{�̂ŊǗ�����
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039�E�p�r�ʒ��ԑ䐔�ؑւȂ�
//				Enter_data.CMN_DT.DT_54.FullNo1 = (ushort)PPrmSS[S_P02][3];	// ���ԑ䐔
//				Enter_data.CMN_DT.DT_54.CarCnt1 = curnum;					// ���ݑ䐔
//				switch (PPrmSS[S_P02][1]) {									// ���������Ӱ��
//				case	1:													// ��������
//					Enter_data.CMN_DT.DT_54.Full[0] = 11;					// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					Enter_data.CMN_DT.DT_54.Full[0] = 10;					// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum >= PPrmSS[S_P02][3]) {
//						Enter_data.CMN_DT.DT_54.Full[0] = 1;				// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039�E�p�r�ʒ��ԑ䐔�Q�n��
//			case 4:															// 01-0039�E�p�r�ʒ��ԑ䐔�R�n��
//				Enter_data.CMN_DT.DT_54.FullNo1 = (ushort)PPrmSS[S_P02][7];	// ���ԑ䐔
//				Enter_data.CMN_DT.DT_54.CarCnt1 = curnum_1;					// ���ݑ䐔�P
//				switch (PPrmSS[S_P02][5]) {									// ��������ԃ��[�h1
//				case	1:													// ��������
//					Enter_data.CMN_DT.DT_54.Full[0] = 11;					// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					Enter_data.CMN_DT.DT_54.Full[0] = 10;					// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						Enter_data.CMN_DT.DT_54.Full[0] = 1;				// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				Enter_data.CMN_DT.DT_54.FullNo2 = (ushort)PPrmSS[S_P02][11];// ���ԑ䐔�Q
//				Enter_data.CMN_DT.DT_54.CarCnt2 = curnum_2;					// ���ݑ䐔�Q
//				switch (PPrmSS[S_P02][9]) {									// ��������ԃ��[�h2
//				case	1:													// ��������
//					Enter_data.CMN_DT.DT_54.Full[1] = 11;					// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					Enter_data.CMN_DT.DT_54.Full[1] = 10;					// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						Enter_data.CMN_DT.DT_54.Full[1] = 1;				// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				if (i == 4) {
//					Enter_data.CMN_DT.DT_54.FullNo3 = (ushort)PPrmSS[S_P02][15];// ���ԑ䐔�R
//					Enter_data.CMN_DT.DT_54.CarCnt3 = curnum_3;					// ���ݑ䐔�R
//					switch (PPrmSS[S_P02][13]) {							// ��������ԃ��[�h3
//					case	1:												// ��������
//						Enter_data.CMN_DT.DT_54.Full[2] = 11;				// �u�������ԁv��ԃZ�b�g
//						break;
//					case	2:												// �������
//						Enter_data.CMN_DT.DT_54.Full[2] = 10;				// �u������ԁv��ԃZ�b�g
//						break;
//					default:												// ����
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							Enter_data.CMN_DT.DT_54.Full[2] = 1;			// �u���ԁv��ԃZ�b�g
//						}
//						break;
//					}
//				}
//				break;
//			default:
//				break;
//			}
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//			// �䐔�Ǘ��ǔ� ------------------------
//			memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );
//			// ���ꎞ���H
//			if( GetVehicleCountDate() == Nrm_YMDHM( &wk_CLK_REC ) ){
//				// ���ꎞ���̏ꍇ�͒ǂ��Ԃ��{�P
//				AddVehicleCountSeqNo();
//			}else{
//				// �قȂ鎞���̏ꍇ�͒ǂ��Ԃ��O�Ƃ���
//				SetVehicleCountDate( Nrm_YMDHM( &wk_CLK_REC ) );
//				ClrVehicleCountSeqNo();
//			}
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntYear	= (wk_CLK_REC.Year % 100);	// �N
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntMon	= wk_CLK_REC.Mon;			// ��
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntDay	= wk_CLK_REC.Day;			// ��
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntHour	= wk_CLK_REC.Hour;			// ��
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntMin	= wk_CLK_REC.Min;			// ��
//			Enter_data.CMN_DT.DT_54.CarCntInfo.CarCntSeq	= GetVehicleCountSeqNo();	// �ǔ�
//			Enter_data.CMN_DT.DT_54.CarCntInfo.Reserve1		= 0;						// �\��
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//		}
//	}
//	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//		Enter_data.CMN_DT.DT_54.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_ENTER);	// �Z���^�[�ǔԁi���Ɂj
//		RAU_UpdateCenterSeqNo(RAU_SEQNO_ENTER);										// �Z���^�[�ǔԍX�V�i���Ɂj
//	}
//}
////[]----------------------------------------------------------------------[]
/////	@brief			���Ƀ��O�f�[�^�쐬(LOG�ɕۑ�����`��)�U�֐��Z���̉����ɗp
////[]----------------------------------------------------------------------[]
/////	@param[in]		pr_lokno : ���������p���Ԉʒu�ԍ�
/////	@param[in]		*data : flp_com�^�̃|�C���^
/////	@return			woid	: 
/////	@author			A.iiizumi
/////	@attention		
/////	@note			
////[]----------------------------------------------------------------------[]
/////	@date			Create	: 2012/02/15<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
//void	Make_Log_Enter_frs( unsigned short pr_lokno, void *data )
//{
//	ushort	lkno;
//	flp_com *flp_data = (flp_com*)data;
//	int i, j;
//	ushort curnum = 0;
//	uchar uc_prm;
//	ushort curnum_1 = 0;
//	ushort curnum_2 = 0;
//	ushort curnum_3 = 0;
//
//	lkno = pr_lokno - 1;
//
//	memset( &Enter_data, 0, sizeof( Enter_data ) );
//
//	Enter_data.CMN_DT.DT_54.ID = 20;
//	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
//		Enter_data.CMN_DT.DT_54.ID = 54;
//	}
//
//	Enter_data.InCount = NTNetDataCont[0];											// ���ɒǂ���
//	NTNetDataCont[0]++;																// ���ɒǂ���+1
//
//	Enter_data.Syubet = LockInfo[lkno].ryo_syu;										// �����敪
//
//	if(flp_data->lag_to_in.BIT.FUKUG != 0){											// ����Ӱ��
//		Enter_data.InMode = 5;
//	} else {
//		Enter_data.InMode = 0;
//	}
//
//	Enter_data.LockNo = (ulong)(( LockInfo[lkno].area * 10000L )
//								+ LockInfo[lkno].posi );							// �����
//	Enter_data.CardType = 0;														// ���Ԍ�����(���g�p)
//	Enter_data.CMachineNo = 0;														// ���Ԍ��@�B��
//	Enter_data.CardNo = 0L;															// ���Ԍ��ԍ�(�����ǂ���)
//	Enter_data.InTime.Year = flp_data->year;										// ���ɔN
//	Enter_data.InTime.Mon = flp_data->mont;											// ���Ɍ�
//	Enter_data.InTime.Day = flp_data->date;											// ���ɓ�
//	Enter_data.InTime.Hour = flp_data->hour;										// ���Ɏ�
//	Enter_data.InTime.Min = flp_data->minu;											// ���ɕ�
//	Enter_data.InTime.Sec = 0;														// ���ɕb
//	// �����N�����������œ����
//	Enter_data.CMN_DT.DT_54.ProcDate.Year	= (uchar)( CLK_REC.year % 100 );		// �N
//	Enter_data.CMN_DT.DT_54.ProcDate.Mon	= (uchar)CLK_REC.mont;					// ��
//	Enter_data.CMN_DT.DT_54.ProcDate.Day	= (uchar)CLK_REC.date;					// ��
//	Enter_data.CMN_DT.DT_54.ProcDate.Hour	= (uchar)CLK_REC.hour;					// ��
//	Enter_data.CMN_DT.DT_54.ProcDate.Min	= (uchar)CLK_REC.minu;					// ��
//	Enter_data.CMN_DT.DT_54.ProcDate.Sec	= (uchar)CLK_REC.seco;					// �b
//	Enter_data.PassCheck = 1;														// ����߽�������Ȃ�
//	if( prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[lkno].ryo_syu-1)*6)),1,1 ) ){	// ��ʖ����Ă���ݒ�?
//		Enter_data.CountSet = 0;													// �ݎԶ���(����)
//	}else{
//		Enter_data.CountSet = 1;													// �ݎԶ���(���Ȃ�)
//	}
//	// ������ް���0�Œ�
//	Enter_data.CMN_DT.DT_54.PascarCnt = 0;		// ����ԗ��J�E���g
//	if (Enter_data.CMN_DT.DT_54.ID != 54) {		// 20/54
//		return;
//	}
//	for (i = 0; i < LOCK_MAX; i++) {
//		// ������ʖ��Ɍ��ݍݎԶ���
//		if (FLAPDT.flp_data[i].nstat.bits.b00) {
//			j = LockInfo[i].ryo_syu - 1;
//			if (j >= 0 && j < 12) {
//				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );// ��ʖ����Ă���ݒ�get
//				if( uc_prm == 1 ){			// ���ݑ䐔�P���J�E���g
//					curnum++;
//					curnum_1++;
//				}
//				else if( uc_prm == 2 ){		// ���ݑ䐔�Q���J�E���g
//					curnum++;
//					curnum_2++;
//				}
//				else if( uc_prm == 3 ){		// ���ݑ䐔�R���J�E���g
//					curnum++;
//					curnum_3++;
//				}
//			}
//		}
//	}
//	// ����ԗ��J�E���g 0�Œ�
//	if (_is_ntnet_remote()) {												// ���uNT-NET�ݒ�̂Ƃ��̂݃f�[�^�Z�b�g
//		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {						// 01-0039�D���ԑ䐔��{�̂ŊǗ�����
//			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
//			case 0:															// 01-0039�E�p�r�ʒ��ԑ䐔�ؑւȂ�
//				Enter_data.CMN_DT.DT_54.FullNo1 = (ushort)PPrmSS[S_P02][3];	// ���ԑ䐔
//				Enter_data.CMN_DT.DT_54.CarCnt1 = curnum;					// ���ݑ䐔
//				switch (PPrmSS[S_P02][1]) {									// ���������Ӱ��
//				case	1:													// ��������
//					Enter_data.CMN_DT.DT_54.Full[0] = 11;					// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					Enter_data.CMN_DT.DT_54.Full[0] = 10;					// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum >= PPrmSS[S_P02][3]) {
//						Enter_data.CMN_DT.DT_54.Full[0] = 1;				// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				break;
//			case 3:															// 01-0039�E�p�r�ʒ��ԑ䐔�Q�n��
//			case 4:															// 01-0039�E�p�r�ʒ��ԑ䐔�R�n��
//				Enter_data.CMN_DT.DT_54.FullNo1 = (ushort)PPrmSS[S_P02][7];	// ���ԑ䐔
//				Enter_data.CMN_DT.DT_54.CarCnt1 = curnum_1;					// ���ݑ䐔�P
//				switch (PPrmSS[S_P02][5]) {									// ��������ԃ��[�h1
//				case	1:													// ��������
//					Enter_data.CMN_DT.DT_54.Full[0] = 11;					// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					Enter_data.CMN_DT.DT_54.Full[0] = 10;					// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_1 >= PPrmSS[S_P02][7]) {
//						Enter_data.CMN_DT.DT_54.Full[0] = 1;				// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				Enter_data.CMN_DT.DT_54.FullNo2 = (ushort)PPrmSS[S_P02][11];// ���ԑ䐔�Q
//				Enter_data.CMN_DT.DT_54.CarCnt2 = curnum_2;					// ���ݑ䐔�Q
//				switch (PPrmSS[S_P02][9]) {									// ��������ԃ��[�h2
//				case	1:													// ��������
//					Enter_data.CMN_DT.DT_54.Full[1] = 11;					// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:													// �������
//					Enter_data.CMN_DT.DT_54.Full[1] = 10;					// �u������ԁv��ԃZ�b�g
//					break;
//				default:													// ����
//					if (curnum_2 >= PPrmSS[S_P02][11]) {
//						Enter_data.CMN_DT.DT_54.Full[1] = 1;				// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//				if (i == 4) {
//					Enter_data.CMN_DT.DT_54.FullNo3 = (ushort)PPrmSS[S_P02][15];// ���ԑ䐔�R
//					Enter_data.CMN_DT.DT_54.CarCnt3 = curnum_3;					// ���ݑ䐔�R
//					switch (PPrmSS[S_P02][13]) {							// ��������ԃ��[�h3
//					case	1:												// ��������
//						Enter_data.CMN_DT.DT_54.Full[2] = 11;				// �u�������ԁv��ԃZ�b�g
//						break;
//					case	2:												// �������
//						Enter_data.CMN_DT.DT_54.Full[2] = 10;				// �u������ԁv��ԃZ�b�g
//						break;
//					default:												// ����
//						if (curnum_3 >= PPrmSS[S_P02][15]) {
//							Enter_data.CMN_DT.DT_54.Full[2] = 1;			// �u���ԁv��ԃZ�b�g
//						}
//						break;
//					}
//				}
//				break;
//			default:
//				break;
//			}
//		}
//	}
//	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//		Enter_data.CMN_DT.DT_54.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_ENTER);	// �Z���^�[�ǔԁi���Ɂj
//		RAU_UpdateCenterSeqNo(RAU_SEQNO_ENTER);										// �Z���^�[�ǔԍX�V�i���Ɂj
//	}
//}
////[]----------------------------------------------------------------------[]
/////	@brief			���Ƀf�[�^(�f�[�^���20)�ҏW����
////[]----------------------------------------------------------------------[]
/////	@param[in]		p_RcptDat : ���O������o�������Ƀf�[�^�̃|�C���^
/////	@param[out]		p_NtDat   : ���Ƀf�[�^(DATA_KIND_20�^)�ւ̃|�C���^  
/////	@return			ret       : ���Ƀf�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
/////	@author			A.iiizumi
/////	@attention		
/////	@note			
////[]----------------------------------------------------------------------[]
/////	@date			Create	: 2012/02/15<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
//unsigned short	NTNET_Edit_Data20( enter_log *p_RcptDat, DATA_KIND_20 *p_NtDat )
//{
//	unsigned short ret;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_20 ) );
//	NTNET_Edit_BasicData( 20, 0, p_RcptDat->CMN_DT.DT_54.SeqNo, &p_NtDat->DataBasic);	// ��{�f�[�^�쐬
//
//	memcpy(&p_NtDat->InCount, p_RcptDat, sizeof( enter_log ) - sizeof(p_RcptDat->CMN_DT));
//
//	// �����N�������Z�b�g
//	p_NtDat->DataBasic.Year	= p_RcptDat->CMN_DT.DT_54.ProcDate.Year;				// �����N
//	p_NtDat->DataBasic.Mon	= p_RcptDat->CMN_DT.DT_54.ProcDate.Mon;					// ������
//	p_NtDat->DataBasic.Day	= p_RcptDat->CMN_DT.DT_54.ProcDate.Day;					// ������
//	p_NtDat->DataBasic.Hour	= p_RcptDat->CMN_DT.DT_54.ProcDate.Hour;				// ������
//	p_NtDat->DataBasic.Min	= p_RcptDat->CMN_DT.DT_54.ProcDate.Min;					// ������
//	p_NtDat->DataBasic.Sec	= p_RcptDat->CMN_DT.DT_54.ProcDate.Sec;					// �����b
//
//	ret = sizeof( DATA_KIND_20 ); 
//	return ret;
//}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ������������� 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//[]----------------------------------------------------------------------[]
///	@brief			�G���[�f�[�^(�f�[�^���120)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�����G���[�f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : �G���[�f�[�^(DATA_KIND_120�^)�ւ̃|�C���^  
///	@return			ret       : �G���[�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data120( Err_log *p_RcptDat, DATA_KIND_120 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_120 ) );
	NTNET_Edit_BasicData( 120, 0, p_RcptDat->ErrSeqNo, &p_NtDat->DataBasic);		// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;														// �����b

	p_NtDat->Errsyu = p_RcptDat->Errsyu;											// �װ���
	p_NtDat->Errcod = p_RcptDat->Errcod;											// �װ����

	switch( p_RcptDat->Errdtc ){
	case 1:																			// ����
		p_NtDat->Errdtc = 1;														// �װ����
		break;
	case 2:																			// �����E����
		p_NtDat->Errdtc = 3;														// �װ�����E����
		break;
	default:																		// ����
		p_NtDat->Errdtc = 2;														// �װ����
		break;
	}

	p_NtDat->Errlev = p_RcptDat->Errlev;											// �װ����

	if( p_RcptDat->Errinf == 2 ){													// �װ���L��(bin)
		memcpy( &p_NtDat->Errdat1[6],
				&p_RcptDat->ErrBinDat,
				4 );																// �װ���
	}else if( p_RcptDat->Errinf ==1 ){												//�װ��񂪁u�L��v�ŃA�X�L�[�Ȃ�
		strncpy( (char*)&p_NtDat->Errdat2, (char*)&p_RcptDat->Errdat,sizeof( p_NtDat->Errdat2 )  );
	}

	ret = sizeof( DATA_KIND_120 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�A���[���f�[�^(�f�[�^���121)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�����A���[���f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : �A���[���f�[�^(DATA_KIND_121�^)�ւ̃|�C���^  
///	@return			ret       : �A���[���f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data121( Arm_log *p_RcptDat, DATA_KIND_121 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_121 ) );
	NTNET_Edit_BasicData( 121, 0, p_RcptDat->ArmSeqNo, &p_NtDat->DataBasic);		// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;														// �����b

	p_NtDat->Armsyu = p_RcptDat->Armsyu;											// �װю��
	p_NtDat->Armcod = p_RcptDat->Armcod;											// �װѺ���
	switch( p_RcptDat->Armdtc ){
	case 1:																			// ����
		p_NtDat->Armdtc = 1;														// �װє���
		break;
	case 2:																			// �����E����
		p_NtDat->Armdtc = 3;														// �װє����E����
		break;
	default:																		// ����
		p_NtDat->Armdtc = 2;														// �װщ���
		break;
	}
	p_NtDat->Armlev = p_RcptDat->Armlev;											// �װ�����


	// �װя��t���d���쐬 
	if( 2 == p_RcptDat->Arminf ){													// �޲���ް�����
		memcpy(	&p_NtDat->Armdat1[6], 
				&p_RcptDat->ArmBinDat , 4 );
	}else if( 1 == p_RcptDat->Arminf ){												// ascii�ް�����
		if(p_RcptDat->Armsyu == 2){
			if( ( (p_RcptDat->Armcod >= 40) && (p_RcptDat->Armcod <= 49) )
			 || (p_RcptDat->Armcod == 54)){
				memcpy(	&p_NtDat->Armdat1[4], 
					&p_RcptDat->Armdat[0] , 6 );
			}
		}
	}

	ret = sizeof( DATA_KIND_121 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			�G���[�f�[�^(�f�[�^���63)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�����G���[�f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : �G���[�f�[�^(DATA_KIND_63�^)�ւ̃|�C���^  
///	@return			ret       : �G���[�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			t.hashimoto
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2013/02/27<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data63( Err_log *p_RcptDat, DATA_KIND_63 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_63 ) );
	NTNET_Edit_BasicData( 63, 0, p_RcptDat->ErrSeqNo, &p_NtDat->DataBasic);			// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;														// �����b

	p_NtDat->FmtRev = 10;															// ̫�ϯ�Rev.��
	p_NtDat->Errsyu = p_RcptDat->Errsyu;											// �װ���
	p_NtDat->Errcod = p_RcptDat->Errcod;											// �װ����

	switch( p_RcptDat->Errdtc ){
	case 1:																			// ����
		p_NtDat->Errdtc = 1;														// �װ����
		break;
	case 2:																			// �����E����
		p_NtDat->Errdtc = 3;														// �װ�����E����
		break;
	default:																		// ����
		p_NtDat->Errdtc = 2;														// �װ����
		break;
	}

	p_NtDat->Errlev = p_RcptDat->Errlev;											// �װ����
	if( p_RcptDat->ErrDoor & ERR_LOG_DOOR_STS_F ){									// �h�A��,1=�J
		p_NtDat->ErrDoor = 1;
	}
	if( p_RcptDat->Errinf == 2 ){													// �װ���L��(bin)
		memcpy( &p_NtDat->Errdat[6],
				&p_RcptDat->ErrBinDat,
				4 );																// �װ���
	}

	ret = sizeof( DATA_KIND_63 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			�A���[���f�[�^(�f�[�^���64)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�����A���[���f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : �A���[���f�[�^(DATA_KIND_64�^)�ւ̃|�C���^  
///	@return			ret       : �A���[���f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			t.hashimoto
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data64( Arm_log *p_RcptDat, DATA_KIND_64 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_64 ) );
	NTNET_Edit_BasicData( 64, 0, p_RcptDat->ArmSeqNo, &p_NtDat->DataBasic);			// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;														// �����b

	p_NtDat->FmtRev = 10;															// ̫�ϯ�Rev.��
	p_NtDat->Armsyu = p_RcptDat->Armsyu;											// �װю��
	p_NtDat->Armcod = p_RcptDat->Armcod;											// �װѺ���
	switch( p_RcptDat->Armdtc ){
	case 1:																			// ����
		p_NtDat->Armdtc = 1;														// �װє���
		break;
	case 2:																			// �����E����
		p_NtDat->Armdtc = 3;														// �װє����E����
		break;
	default:																		// ����
		p_NtDat->Armdtc = 2;														// �װщ���
		break;
	}
	p_NtDat->Armlev = p_RcptDat->Armlev;											// �װ�����
	if( p_RcptDat->ArmDoor & ERR_LOG_DOOR_STS_F ){									// �h�A��,1=�J
		p_NtDat->ArmDoor = 1;
	}

	// �װя��t���d���쐬 
	if( 2 == p_RcptDat->Arminf ){													// �޲���ް�����
		memcpy(	&p_NtDat->Armdat[6], 
				&p_RcptDat->ArmBinDat , 4 );
	}

	ret = sizeof( DATA_KIND_64 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���j�^�f�[�^(�f�[�^���122)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�������j�^�f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : ���j�^�f�[�^(DATA_KIND_122�^)�ւ̃|�C���^  
///	@return			ret       : ���j�^�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data122( Mon_log *p_RcptDat, DATA_KIND_122 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_122 ) );
	NTNET_Edit_BasicData( 122, 0, p_RcptDat->MonSeqNo, &p_NtDat->DataBasic);		// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;														// �����b

	p_NtDat->Monsyu = p_RcptDat->MonKind;											// ���j�^���
	p_NtDat->Moncod = p_RcptDat->MonCode;											// ���j�^�R�[�h
	p_NtDat->Monlev = p_RcptDat->MonLevel;											// ���j�^���x��
	// ���j�^���
	memcpy( &p_NtDat->Mondat1, &p_RcptDat->MonInfo[0], sizeof(p_NtDat->Mondat1));

	ret = sizeof( DATA_KIND_122 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���샂�j�^�f�[�^(�f�[�^���123)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�������샂�j�^�f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : ���샂�j�^�f�[�^(DATA_KIND_123�^)�ւ̃|�C���^  
///	@return			ret       : ���샂�j�^�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data123( Ope_log *p_RcptDat, DATA_KIND_123 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_123 ) );
	NTNET_Edit_BasicData( 123, 0, p_RcptDat->OpeSeqNo, &p_NtDat->DataBasic);		// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;														// �����b

	p_NtDat->OpeMonsyu = p_RcptDat->OpeKind;										// ���샂�j�^���
	p_NtDat->OpeMoncod = p_RcptDat->OpeCode;										// ���샂�j�^�R�[�h
	p_NtDat->OpeMonlev = p_RcptDat->OpeLevel;										// ���샂�j�^���x��

	memcpy( &p_NtDat->OpeMondat1, &p_RcptDat->OpeBefore, sizeof(p_NtDat->OpeMondat1));
	memcpy( &p_NtDat->OpeMondat2, &p_RcptDat->OpeAfter, sizeof(p_NtDat->OpeMondat2));

	ret = sizeof( DATA_KIND_123 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�R�C�����ɏW�v�f�[�^(�f�[�^���131)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�����R�C�����ɏW�v�f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : �R�C�����Ƀf�[�^(DATA_KIND_130�^)�ւ̃|�C���^  
///	@return			ret       : �R�C�����Ƀf�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data131( COIN_SYU *p_RcptDat, DATA_KIND_130 *p_NtDat )
{
	unsigned short ret,i;

	memset( p_NtDat, 0, sizeof( DATA_KIND_130 ) );
	NTNET_Edit_BasicData( 131, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);			// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->NowTime.Mon;							// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->NowTime.Day;							// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->NowTime.Min;							// ������
	p_NtDat->DataBasic.Sec = 0;														// �����b

	p_NtDat->KakariNo = p_RcptDat->Kakari_no;										// �W����
	p_NtDat->SeqNo = CountSel( &p_RcptDat->Oiban );									// �ǔ�

	// �^�C�v�ݒ�
	p_NtDat->Type = 51;																// ���v(51)
	
	p_NtDat->Sf.Coin[0].Money = 10;													// ��݋��ɋ���(10�~)
	p_NtDat->Sf.Coin[1].Money = 50;													// ��݋��ɋ���(50�~)
	p_NtDat->Sf.Coin[2].Money = 100;												// ��݋��ɋ���(100�~)
	p_NtDat->Sf.Coin[3].Money = 500;												// ��݋��ɋ���(500�~)

	p_NtDat->Sf.Coin[0].Mai = p_RcptDat->cnt[0];									// ��݋��ɖ���(10�~)
	p_NtDat->Sf.Coin[1].Mai = p_RcptDat->cnt[1];									// ��݋��ɖ���(50�~)
	p_NtDat->Sf.Coin[2].Mai = p_RcptDat->cnt[2];									// ��݋��ɖ���(100�~)
	p_NtDat->Sf.Coin[3].Mai = p_RcptDat->cnt[3];									// ��݋��ɖ���(500�~)

	for( i=0; i<4; i++ ){
// MH322914 (s) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
//		p_NtDat->SfTotal += p_NtDat->Sf.Coin[i].Mai * p_NtDat->Sf.Coin[i].Money;	// ��݋��ɑ��z
		p_NtDat->SfTotal += (ulong)p_NtDat->Sf.Coin[i].Mai * (ulong)p_NtDat->Sf.Coin[i].Money;	// ��݋��ɑ��z
// MH322914 (e) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
	}

	ret = sizeof( DATA_KIND_130 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�������ɏW�v�f�[�^(�f�[�^���133)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�����������ɍ��v�f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : �������Ƀf�[�^(DATA_KIND_132�^)�ւ̃|�C���^  
///	@return			ret       : �������Ƀf�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data133( NOTE_SYU *p_RcptDat, DATA_KIND_132 *p_NtDat )
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_132 ) );
	NTNET_Edit_BasicData( 133, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);			// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->NowTime.Mon;							// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->NowTime.Day;							// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->NowTime.Min;							// ������
	p_NtDat->DataBasic.Sec = 0;														// �����b
	p_NtDat->KakariNo = p_RcptDat->Kakari_no;										// �W����
	p_NtDat->SeqNo = CountSel( &p_RcptDat->Oiban );									// �ǔ�

	// �^�C�v�ݒ�
	p_NtDat->Type = 52;																// ���v(52)

	p_NtDat->Sf.Note[0].Money = 1000;												// �������ɋ���(1000�~)
	p_NtDat->Sf.Note[0].Mai = p_RcptDat->cnt[0];									// �������ɖ���(1000�~)
// MH322914 (s) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
//	p_NtDat->SfTotal = p_NtDat->Sf.Note[0].Mai * p_NtDat->Sf.Note[0].Money;			// �������ɑ��z
	p_NtDat->SfTotal = (ulong)p_NtDat->Sf.Note[0].Mai * (ulong)p_NtDat->Sf.Note[0].Money;			// �������ɑ��z
// MH322914 (e) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)

	ret = sizeof( DATA_KIND_132 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���ԑ䐔�f�[�^���O�쐬(LOG�ɕۑ�����`��)
//[]----------------------------------------------------------------------[]
///	@param[in]		void    : 
///	@return			woid    : 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Make_Log_ParkCarNum( void )
{

	int i, j;
	char full;
	ulong curnum = 0;
	uchar uc_prm;
	ushort curnum_1 = 0;
	ushort curnum_2 = 0;
	ushort curnum_3 = 0;

	memset( &ParkCar_data, 0, sizeof( ParkCar_data ) );

	ParkCar_data.CMN_DT.DT_58.ID = 236;
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		ParkCar_data.CMN_DT.DT_58.ID = 58;
	}

	ParkCar_data.Time.Year = (uchar)( CLK_REC.year % 100 );		// �N
	ParkCar_data.Time.Mon = (uchar)CLK_REC.mont;				// ��
	ParkCar_data.Time.Day = (uchar)CLK_REC.date;				// ��
	ParkCar_data.Time.Hour = (uchar)CLK_REC.hour;				// ��
	ParkCar_data.Time.Min = (uchar)CLK_REC.minu;				// ��
	ParkCar_data.Time.Sec = (uchar)CLK_REC.seco;				// �b

	for (i = 0; i < LOCK_MAX; i++) {
		// ������ʖ��Ɍ��ݍݎԶ���
		if (FLAPDT.flp_data[i].nstat.bits.b00) {
			j = LockInfo[i].ryo_syu - 1;
			if (j >= 0 && j < 12) {
				ParkCar_data.RyCurNum[j]++;
				uc_prm = (uchar)prm_get( COM_PRM,S_SHA,(short)(2+((LockInfo[i].ryo_syu-1)*6)),1,1 );// ��ʖ����Ă���ݒ�get
				if( uc_prm == 1 ){			// ���ݑ䐔�P���J�E���g
					curnum++;
					curnum_1++;
				}
				else if( uc_prm == 2 ){		// ���ݑ䐔�Q���J�E���g
					curnum++;
					curnum_2++;
				}
				else if( uc_prm == 3 ){		// ���ݑ䐔�R���J�E���g
					curnum++;
					curnum_3++;
				}
			}
		}
	}

	// ���ݒ��ԑ䐔
	ParkCar_data.CurNum = curnum;

	// �p�r�ʒ��ԑ䐔�ݒ�
	ParkCar_data.State = prm_get(COM_PRM,S_SYS,39,1,1);

	// ����1�`3������
	for (i = 0; i < 3; i++) {
		// ��Ԏ擾
		full = getFullFactor((uchar)i);
		if(ParkCar_data.CMN_DT.DT_58.ID == 236) {
			if (full & 0x10) {
				// ����
				ParkCar_data.Full[i] = 1;
			} else {
				// ���
				ParkCar_data.Full[i] = 0;
			}
		}
		else {										// �V�t�H�[�}�b�g(ID:58)
			if (full == FORCE_FULL) {				// ��������
				ParkCar_data.Full[i] = 11;
			}
			else if(full == FORCE_VACANCY) {		// �������
				ParkCar_data.Full[i] = 10;
			}
			else if (full & 0x10) {					// ����
				ParkCar_data.Full[i] = 1;
			}
			else {									// ���
				ParkCar_data.Full[i] = 0;
			}
		}
		// ������"2"(�قږ���)�͖��g�p
	}
	ParkCar_data.CMN_DT.DT_58.FmtRev = 0;	// �t�H�[�}�b�gRev.��
	ParkCar_data.GroupNo = 0;				// ��ٰ�ߔԍ�

	if (ParkCar_data.CMN_DT.DT_58.ID != 58) {		// 236/58
		return;
	}
	// �p�r�ʒ��ԑ䐔�ݒ�
	if (_is_ntnet_remote()) {											// ���uNT-NET�ݒ�̂Ƃ��̂݃f�[�^�Z�b�g
		if (prm_get(COM_PRM, S_SYS, 39, 1, 2) == 2) {					// 01-0039�D���ԑ䐔��{�̂ŊǗ�����
			switch (i = (int)prm_get(COM_PRM, S_SYS, 39, 1, 1)) {
			case 0:														// 01-0039�E�p�r�ʒ��ԑ䐔�ؑւȂ�
				ParkCar_data.CMN_DT.DT_58.FullNo1 = (ushort)PPrmSS[S_P02][3];	// ���ԑ䐔
				if (curnum > PPrmSS[S_P02][3]) {
					ParkCar_data.CMN_DT.DT_58.EmptyNo1 = 0;						// ��ԑ䐔
				} else {
					ParkCar_data.CMN_DT.DT_58.EmptyNo1 = (ushort)PPrmSS[S_P02][3] - curnum;	// ��ԑ䐔(���ԑ䐔 - ���ݒ��ԑ䐔)
				}
//////////////////////////////////////////////////////////////////////////
//				switch (PPrmSS[S_P02][1]) {								// ���������Ӱ��
//				case	1:												// ��������
//					ParkCar_data.Full[0] = 11;							// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:												// �������
//					ParkCar_data.Full[0] = 10;							// �u������ԁv��ԃZ�b�g
//					break;
//				default:												// ����
//					if( ParkCar_data.CMN_DT.DT_58.EmptyNo1 == 0 ){		// ��Ԃ��u�O�v�̏ꍇ
//						ParkCar_data.Full[0] = 1;						// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//////////////////////////////////////////////////////////////////////////
				break;
			case 3:														// 01-0039�E�p�r�ʒ��ԑ䐔�Q�n��
			case 4:														// 01-0039�E�p�r�ʒ��ԑ䐔�R�n��
				ParkCar_data.CMN_DT.DT_58.FullNo1 = (ushort)PPrmSS[S_P02][7];	// ���ԑ䐔�P
				if (curnum_1 > PPrmSS[S_P02][7]) {
					ParkCar_data.CMN_DT.DT_58.EmptyNo1 = 0;						// ��ԑ䐔�P
				} else {
					ParkCar_data.CMN_DT.DT_58.EmptyNo1 = (ushort)(PPrmSS[S_P02][7] - curnum_1);	// ��ԑ䐔�P(���ԑ䐔 - ���ݒ��ԑ䐔)
				}
//////////////////////////////////////////////////////////////////////////
//				switch (PPrmSS[S_P02][5]) {								// ��������ԃ��[�h1
//				case	1:												// ��������
//					ParkCar_data.Full[0] = 11;							// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:												// �������
//					ParkCar_data.Full[0] = 10;							// �u������ԁv��ԃZ�b�g
//					break;
//				default:												// ����
//					if( ParkCar_data.CMN_DT.DT_58.EmptyNo1 == 0 ){		// ��Ԃ��u�O�v�̏ꍇ
//						ParkCar_data.Full[0] = 1;						// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//////////////////////////////////////////////////////////////////////////
				ParkCar_data.CMN_DT.DT_58.FullNo2 = (ushort)PPrmSS[S_P02][11];	// ���ԑ䐔�Q
				if (curnum_2 > PPrmSS[S_P02][11]) {
					ParkCar_data.CMN_DT.DT_58.EmptyNo2 = 0;						// ��ԑ䐔�Q
				} else {
					ParkCar_data.CMN_DT.DT_58.EmptyNo2 = (ushort)(PPrmSS[S_P02][11] - curnum_2);	// ��ԑ䐔�Q(���ԑ䐔 - ���ݒ��ԑ䐔)
				}
//////////////////////////////////////////////////////////////////////////
//				switch (PPrmSS[S_P02][9]) {								// ��������ԃ��[�h2
//				case	1:												// ��������
//					ParkCar_data.Full[1] = 11;							// �u�������ԁv��ԃZ�b�g
//					break;
//				case	2:												// �������
//					ParkCar_data.Full[1] = 10;							// �u������ԁv��ԃZ�b�g
//					break;
//				default:												// ����
//					if( ParkCar_data.CMN_DT.DT_58.EmptyNo2 == 0 ){		// ��Ԃ��u�O�v�̏ꍇ
//						ParkCar_data.Full[1] = 1;						// �u���ԁv��ԃZ�b�g
//					}
//					break;
//				}
//////////////////////////////////////////////////////////////////////////
				if (i == 4) {
					ParkCar_data.CMN_DT.DT_58.FullNo3 = (ushort)PPrmSS[S_P02][15];	// ���ԑ䐔�R
					if (curnum_3 > PPrmSS[S_P02][15]) {
						ParkCar_data.CMN_DT.DT_58.EmptyNo3 = 0;						// ��ԑ䐔�R
					} else {
						ParkCar_data.CMN_DT.DT_58.EmptyNo3 = (ushort)(PPrmSS[S_P02][15] - curnum_3);// ��ԑ䐔�R(���ԑ䐔 - ���ݒ��ԑ䐔)
					}
//////////////////////////////////////////////////////////////////////////
//					switch (PPrmSS[S_P02][13]) {						// ��������ԃ��[�h3
//					case	1:											// ��������
//						ParkCar_data.Full[2] = 11;						// �u�������ԁv��ԃZ�b�g
//						break;
//					case	2:											// �������
//						ParkCar_data.Full[2] = 10;						// �u������ԁv��ԃZ�b�g
//						break;
//					default:											// ����
//						if( ParkCar_data.CMN_DT.DT_58.EmptyNo3 == 0 ){	// ��Ԃ��u�O�v�̏ꍇ
//							ParkCar_data.Full[2] = 1;					// �u���ԁv��ԃZ�b�g
//						}
//						break;
//					}
//////////////////////////////////////////////////////////////////////////
				}
				break;
			default:
				break;
			}
		}
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			���ԑ䐔�f�[�^(�f�[�^���236)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�������ԑ䐔�f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : ���ԑ䐔�f�[�^(DATA_KIND_236�^)�ւ̃|�C���^  
///	@return			ret       : ���ԑ䐔�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data236( ParkCar_log *p_RcptDat, DATA_KIND_236 *p_NtDat )
{
	unsigned short ret,i;

	memset( p_NtDat, 0, sizeof( DATA_KIND_236 ) );
	NTNET_Edit_BasicData( 236, 0, p_RcptDat->CMN_DT.DT_58.SeqNo, &p_NtDat->DataBasic);	// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Time.Year % 100);					// �����N(���O�쐬���ɉ�2���ɂ��Ă���)
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Time.Mon;							// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Time.Day;							// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Time.Hour;							// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Time.Min;							// ������
	p_NtDat->DataBasic.Sec = (uchar)p_RcptDat->Time.Sec;							// �����b

	p_NtDat->CurNum = p_RcptDat->CurNum;											// ���ݒ��ԑ䐔
	p_NtDat->State = p_RcptDat->State;												// �p�r�ʒ��ԑ䐔�ݒ�

	for(i = 0; i<20; i++){															// �������1�`20���ݒ��ԑ䐔
		p_NtDat->RyCurNum[i] =  p_RcptDat->RyCurNum[i];
	}

	for(i = 0; i<3; i++){															// ����1�`3������
		p_NtDat->Full[i] = p_RcptDat->Full[i];
	}

	ret = sizeof( DATA_KIND_236 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			NT-NET��{�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		knd : �f�[�^���
///	@param[in]		knd : �f�[�^�ێ��t���O
///	@param[in]		seqNo : �V�[�P���V����No.
///	@param[in]		basic : DATA_BASIC�^�̃|�C���^
///	@return			void  : 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void	NTNET_Edit_BasicData( uchar knd, uchar keep, uchar SeqNo, DATA_BASIC *basic)
{
	// ����ID
	basic->SystemID = NTNET_SYSTEM_ID[prm_get( COM_PRM, S_NTN, 25, 1, 1 )];
																					// 0:�ׯ�߼���
																					// 1:���u�ׯ�߼���
																					// 2:�ׯ�ߥ���ּ���
																					// 3:���u�ׯ�ߥ���ּ���
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
																					// 4:�ްĎ�����
																					// 5:���u�ްĎ�����
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	basic->DataKind = knd;															// �ް����
	basic->DataKeep = keep;															// �ް��ێ��׸�
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		basic->SeqNo = SeqNo;														// �V�[�P���XNo(1�`99:�d�����������0)
	}
	basic->ParkingNo = (ulong)CPrmSS[S_SYS][1];										// ���ԏꇂ
	basic->ModelCode = NTNET_MODEL_CODE;											// �@����

	basic->MachineNo = (ulong)CPrmSS[S_PAY][2];										// �@�B��

}

//[]----------------------------------------------------------------------[]
///	@brief			���K�Ǘ����O�f�[�^�쐬(LOG�ɕۑ�����`��)
//[]----------------------------------------------------------------------[]
///	@param[in]		payclass : �����敪
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
/////	@return			woid	: 
///	@return			TRUE	: ���K�Ǘ��f�[�^���M�v
///					FALSE	: ���K�Ǘ��f�[�^���M�s�v
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//void	Make_Log_MnyMng( ushort payclass )
BOOL	Make_Log_MnyMng( ushort payclass )
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
{
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
	BOOL ret = TRUE;
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)

	if( ( ( 0 == payclass ) || ( 1 == payclass ) ) && (OpeNtnetAddedInfo.PayMethod !=5 )){	// ���Z�����i���Zor�Đ��Z�jand ����X�V�ł͂Ȃ�
		if (ntnet_decision_credit(&PayData.credit)) {
			payclass += 4;													// �����敪��ڼޯĐ��Z�i�ڼޯčĐ��Z�j�ɂ���
		}
	}

	if( payclass <= 5 ){													// ���Z����
		// ���Z�ǔԁE���Z����set(���Z���쎞�ȊO�͂O)
		turi_kan.PayCount = CountSel( &PayData.Oiban );						// ���Z or ���Z���~�ǔ�set
// MH810105(S) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�
//		turi_kan.PayDate.Year = PayData.TOutTime.Year;
//		turi_kan.PayDate.Mon = PayData.TOutTime.Mon;
//		turi_kan.PayDate.Day = PayData.TOutTime.Day;
//		turi_kan.PayDate.Hour = PayData.TOutTime.Hour;
//		turi_kan.PayDate.Min = PayData.TOutTime.Min;
//		// �����������o�Ɏ����Ƃ���
//		turi_kan.ProcDate.Year = (uchar)( PayData.TOutTime.Year % 100 );
//		turi_kan.ProcDate.Mon = PayData.TOutTime.Mon;
//		turi_kan.ProcDate.Day = PayData.TOutTime.Day;
//		turi_kan.ProcDate.Hour = PayData.TOutTime.Hour;
//		turi_kan.ProcDate.Min = PayData.TOutTime.Min;
//		turi_kan.ProcDate.Sec = 0;
// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//		// ���	(0=���O���Z�^1�������Z�o�ɐ��Z)
//		if(PayData.shubetsu == 0){
		// ���	(0=���O���Z�^1�������Z�o�ɐ��Z�^2=���u���Z(���Z���ύX))
		if(PayData.shubetsu == 0 || PayData.shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
			turi_kan.PayDate.Year = PayData.TOutTime.Year;
			turi_kan.PayDate.Mon = PayData.TOutTime.Mon;
			turi_kan.PayDate.Day = PayData.TOutTime.Day;
			turi_kan.PayDate.Hour = PayData.TOutTime.Hour;
			turi_kan.PayDate.Min = PayData.TOutTime.Min;
			// �����������o�Ɏ����Ƃ���
			turi_kan.ProcDate.Year = (uchar)( PayData.TOutTime.Year % 100 );
			turi_kan.ProcDate.Mon = PayData.TOutTime.Mon;
			turi_kan.ProcDate.Day = PayData.TOutTime.Day;
			turi_kan.ProcDate.Hour = PayData.TOutTime.Hour;
			turi_kan.ProcDate.Min = PayData.TOutTime.Min;
			turi_kan.ProcDate.Sec = 0;

		}else{
			turi_kan.PayDate.Year = PayData.TUnpaidPayTime.Year;
			turi_kan.PayDate.Mon = PayData.TUnpaidPayTime.Mon;
			turi_kan.PayDate.Day = PayData.TUnpaidPayTime.Day;
			turi_kan.PayDate.Hour = PayData.TUnpaidPayTime.Hour;
			turi_kan.PayDate.Min = PayData.TUnpaidPayTime.Min;
			// �������������Z�����i�����Z�o�ɗp�j�Ƃ���
			turi_kan.ProcDate.Year	= (uchar)(PayData.TUnpaidPayTime.Year % 100);	// ���Z�N
			turi_kan.ProcDate.Mon	= PayData.TUnpaidPayTime.Mon;				// ���Z��
			turi_kan.ProcDate.Day	= PayData.TUnpaidPayTime.Day;				// ���Z��
			turi_kan.ProcDate.Hour	= PayData.TUnpaidPayTime.Hour;			// ���Z��
			turi_kan.ProcDate.Min	= PayData.TUnpaidPayTime.Min;				// ���Z��

		}
// MH810105(E) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�
	}else if( payclass == 10 ){												// �R�C�����ɏW�v
		// �����������R�C�����ɏW�v�̍���W�v�����Ƃ���
		turi_kan.ProcDate.Year = (uchar)( coin_syu.NowTime.Year % 100 );
		turi_kan.ProcDate.Mon = coin_syu.NowTime.Mon;
		turi_kan.ProcDate.Day = coin_syu.NowTime.Day;
		turi_kan.ProcDate.Hour = coin_syu.NowTime.Hour;
		turi_kan.ProcDate.Min = coin_syu.NowTime.Min;
		turi_kan.ProcDate.Sec = 0;
	}else if( payclass == 11 ){												// �������ɏW�v
		// �����������������ɏW�v�̍���W�v�����Ƃ���
		turi_kan.ProcDate.Year = (uchar)( note_syu.NowTime.Year % 100 );
		turi_kan.ProcDate.Mon = note_syu.NowTime.Mon;
		turi_kan.ProcDate.Day = note_syu.NowTime.Day;
		turi_kan.ProcDate.Hour = note_syu.NowTime.Hour;
		turi_kan.ProcDate.Min = note_syu.NowTime.Min;
		turi_kan.ProcDate.Sec = 0;
	}else if(( payclass == 20 ) ||											// �W���J�[�h�ɂ��R�C������������̃R�C����[
			 ( payclass == 30 ) ||											// �ݒ�@����̒ޑK��[
			 ( payclass == 32 )){											// ���K�Ǘ�
		// �����������ޑK�Ǘ��̍���W�v�����Ƃ���
		turi_kan.ProcDate.Year = (uchar)( turi_kan.NowTime.Year % 100 );
		turi_kan.ProcDate.Mon = turi_kan.NowTime.Mon;
		turi_kan.ProcDate.Day = turi_kan.NowTime.Day;
		turi_kan.ProcDate.Hour = turi_kan.NowTime.Hour;
		turi_kan.ProcDate.Min = turi_kan.NowTime.Min;
		turi_kan.ProcDate.Sec = 0;
	}else{
		// �C���x���g�����A�����������擾�ł��Ȃ��ꍇ�͏������������ݎ����Ƃ���
		turi_kan.ProcDate.Year = (uchar)( CLK_REC.year % 100 );
		turi_kan.ProcDate.Mon = (uchar)CLK_REC.mont;
		turi_kan.ProcDate.Day = (uchar)CLK_REC.date;
		turi_kan.ProcDate.Hour = (uchar)CLK_REC.hour;
		turi_kan.ProcDate.Min = (uchar)CLK_REC.minu;
		turi_kan.ProcDate.Sec = (uchar)CLK_REC.seco;
	}

	turi_kan.PayClass = (uchar)payclass;									// �����敪set

	turi_kan.Kakari_no = OPECTL.Kakari_Num;									// �W����
	turikan_subtube_set();

	if( payclass != 10 ){													// ��݋��ɍ��v�łȂ�
		turi_kan.safe_dt[0] = SFV_DAT.safe_dt[0];							// ��݋��ɖ���(10�~)set
		turi_kan.safe_dt[1] = SFV_DAT.safe_dt[1];							// ��݋��ɖ���(50�~)set
		turi_kan.safe_dt[2] = SFV_DAT.safe_dt[2];							// ��݋��ɖ���(100�~)set
		turi_kan.safe_dt[3] = SFV_DAT.safe_dt[3];							// ��݋��ɖ���(500�~)set
	}

	if( payclass != 11 ){													// �������ɍ��v�łȂ�
		turi_kan.nt_safe_dt = SFV_DAT.nt_safe_dt;							// �������ɖ���(1000�~)
	}

// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
	if( payclass <= 5 ) {
		ret = FALSE;

		if( memcmp( turi_kan_bk.safe_dt, turi_kan.safe_dt, sizeof( turi_kan.safe_dt )) != 0 ) {	// �R�C�����ɖ���
			ret = TRUE;
		}
		if( turi_kan_bk.nt_safe_dt != turi_kan.nt_safe_dt ) {	// �������ɖ���
			ret = TRUE;
		}
		if( CPrmSS[S_KAN][1] ){													// ���K�Ǘ�����ݒ�
			if( memcmp( turi_kan_bk.turi_dat, turi_kan.turi_dat, sizeof( turi_kan.turi_dat )) != 0 ) {	// ���K�f�[�^�i4���핪�j
				ret = TRUE;
			}
		}
		
		//�N������0�̎��̓��O�o�^���Ȃ�(�ŏ��P�ʂƔ�r�Ƃ��Ă���)
		if( (turi_kan.PayDate.Year < 1980) ||
			(turi_kan.PayDate.Mon < 1) ||
			(turi_kan.PayDate.Day < 1) ){
			ret = FALSE;
		}
	}
	
	if(ret == TRUE) {
		turi_kan_bk = turi_kan;
	}

	return ret;
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
}

//[]----------------------------------------------------------------------[]
///	@brief			���K�Ǘ��f�[�^(�f�[�^���126)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�������K�Ǘ����̃|�C���^
///	@param[out]		p_NtDat   : ���K�Ǘ��f�[�^(DATA_KIND_126�^)�ւ̃|�C���^
///	@return			ret       : ���K�Ǘ��f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`)
///	@author			m.nagashima
///	@attention		
///	@note			�O��ҏW���e�Ɠ����ŋ��K�Ǘ��f�[�^���M�s�v�̏ꍇ�Aret=0��Ԃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/21<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data126( TURI_KAN *p_RcptDat, DATA_KIND_126 *p_NtDat )
{
	char	i;
	ushort	wk_mai;
	uchar	f_Send;
	ushort	ret = 0;
	uchar	pos = 0;

	// �N������̌����A�����Ȃ��̐��Z���~�E�Đ��Z���~���͋��K�Ǘ��f�[�^�𑗐M���Ȃ�

	memset( p_NtDat, 0, sizeof( DATA_KIND_126 ) );

	NTNET_Edit_BasicData( 126, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// ��{�ް��쐬
	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->ProcDate.Year % 100 );		// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->ProcDate.Mon;				// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->ProcDate.Day;				// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->ProcDate.Hour;				// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->ProcDate.Min;				// ������
	p_NtDat->DataBasic.Sec = 0;												// �����b

	/* ���Z�ǔԂƐ��Z����set(���Z���쎞�ȊO�͂O) */
	if( p_RcptDat->PayClass <= 5 ){											// ���Z����
		p_NtDat->PayCount = p_RcptDat->PayCount;							// ���Z or ���Z���~�ǔ�set
		p_NtDat->PayTime.Year = p_RcptDat->PayDate.Year;					// ���Z�N
		p_NtDat->PayTime.Mon = p_RcptDat->PayDate.Mon;						// ���Z��
		p_NtDat->PayTime.Day = p_RcptDat->PayDate.Day;						// ���Z��
		p_NtDat->PayTime.Hour = p_RcptDat->PayDate.Hour;					// ���Z��
		p_NtDat->PayTime.Min = p_RcptDat->PayDate.Min;						// ���Z��
		p_NtDat->PayTime.Sec = 0;											// ���Z�b
	}
	p_NtDat->PayClass = p_RcptDat->PayClass;								// �����敪

	p_NtDat->KakariNo = p_RcptDat->Kakari_no;								// �W����

	p_NtDat->CoinSf[0].Money = 10;											// ��݋��ɋ���(10�~)
	p_NtDat->CoinSf[1].Money = 50;											// ��݋��ɋ���(50�~)
	p_NtDat->CoinSf[2].Money = 100;											// ��݋��ɋ���(100�~)
	p_NtDat->CoinSf[3].Money = 500;											// ��݋��ɋ���(500�~)
	if( p_RcptDat->PayClass != 10 ){										// ��݋��ɍ��v�łȂ�
		p_NtDat->CoinSf[0].Mai = p_RcptDat->safe_dt[0];						// ��݋��ɖ���(10�~)
		p_NtDat->CoinSf[1].Mai = p_RcptDat->safe_dt[1];						// ��݋��ɖ���(50�~)
		p_NtDat->CoinSf[2].Mai = p_RcptDat->safe_dt[2];						// ��݋��ɖ���(100�~)
		p_NtDat->CoinSf[3].Mai = p_RcptDat->safe_dt[3];						// ��݋��ɖ���(500�~)
	}

	for( i=0; i<4; i++ ){
		p_NtDat->CoinSfTotal +=												// ��݋��ɑ��z
// MH322914 (s) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
//			p_NtDat->CoinSf[i].Mai * p_NtDat->CoinSf[i].Money;
			(ulong)p_NtDat->CoinSf[i].Mai * (ulong)p_NtDat->CoinSf[i].Money;
// MH322914 (e) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
	}

	p_NtDat->NoteSf[0].Money = 1000;										// �������ɋ���(1000�~)
	if( p_RcptDat->PayClass != 11 ){										// �������ɍ��v�łȂ�
		p_NtDat->NoteSf[0].Mai = p_RcptDat->nt_safe_dt;						// �������ɖ���(1000�~)
	}
	p_NtDat->NoteSfTotal =													// �������ɑ��z
// MH322914 (s) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)
//		p_NtDat->NoteSf[0].Mai * p_NtDat->NoteSf[0].Money;
		(ulong)p_NtDat->NoteSf[0].Mai * (ulong)p_NtDat->NoteSf[0].Money;
// MH322914 (e) kasiyama 2016/07/07 Long�^�ɕϊ����ĂȂ����߁A�I�[�o�[�t���[�����l�ő��M�����(���ʉ��PNo.1188)(MH341106)

	// ���K�Ǘ��Ȃ����͋��ɏ��̂ݑ��M
	if( CPrmSS[S_KAN][1] ){													// ���K�Ǘ�����ݒ�
		switch( (uchar)p_RcptDat->PayClass ){

		case 20:
		case 30:
		case 32:
			p_NtDat->Coin[0].Mai = p_RcptDat->turi_dat[0].sin_mai;			// ��ݏz����(10�~)
			p_NtDat->Coin[1].Mai = p_RcptDat->turi_dat[1].sin_mai;			// ��ݏz����(50�~)
			p_NtDat->Coin[2].Mai = p_RcptDat->turi_dat[2].sin_mai;			// ��ݏz����(100�~)
			p_NtDat->Coin[3].Mai = p_RcptDat->turi_dat[3].sin_mai;			// ��ݏz����(500�~)
			for( i=0;i<2;i++ ){
				switch((( p_RcptDat->sub_tube >> (i*8)) & 0x000F )){
					case 0x01:
						pos = 0;
						p_NtDat->CoinYotiku[i].Money = 10;								// ������z(10�~)
						break;
					case 0x02:
						pos = 1;
						p_NtDat->CoinYotiku[i].Money = 50;								// ������z(50�~)
						break;
					case 0x04:
						pos = 2;
						p_NtDat->CoinYotiku[i].Money = 100;								// ������z(100�~)
						break;
					case 0:
					default:															// �ڑ��Ȃ�
						continue;
				}
				p_NtDat->CoinYotiku[i].Mai = p_RcptDat->turi_dat[pos].ysin_mai;			// ��ݗ\�~����(10�~/50�~/100�~)
			}
			break;

		default:
			// 10�~
			p_NtDat->Coin[0].Mai = p_RcptDat->turi_dat[0].zen_mai			// ��ݏz����(10�~)
									+ (ushort)p_RcptDat->turi_dat[0].sei_nyu;
			wk_mai = (ushort)(p_RcptDat->turi_dat[0].sei_syu + p_RcptDat->turi_dat[0].kyosei);
			if( p_NtDat->Coin[0].Mai < wk_mai ){
				p_NtDat->Coin[0].Mai = 0;
			}else{
				p_NtDat->Coin[0].Mai -= wk_mai;
			}
			// 50�~
			p_NtDat->Coin[1].Mai = p_RcptDat->turi_dat[1].zen_mai			// ��ݏz����(50�~)
									+ (ushort)p_RcptDat->turi_dat[1].sei_nyu;
			wk_mai = (ushort)(p_RcptDat->turi_dat[1].sei_syu + p_RcptDat->turi_dat[1].kyosei);
			if( p_NtDat->Coin[1].Mai < wk_mai ){
				p_NtDat->Coin[1].Mai = 0;
			}else{
				p_NtDat->Coin[1].Mai -= wk_mai;
			}
			// 100�~
			p_NtDat->Coin[2].Mai = p_RcptDat->turi_dat[2].zen_mai			// ��ݏz����(100�~)
									+ (ushort)p_RcptDat->turi_dat[2].sei_nyu;
			wk_mai = (ushort)(p_RcptDat->turi_dat[2].sei_syu + p_RcptDat->turi_dat[2].kyosei);
			if( p_NtDat->Coin[2].Mai < wk_mai ){
				p_NtDat->Coin[2].Mai = 0;
			}else{
				p_NtDat->Coin[2].Mai -= wk_mai;
			}
			// 500�~
			p_NtDat->Coin[3].Mai = p_RcptDat->turi_dat[3].zen_mai			// ��ݏz����(500�~)
									+ (ushort)p_RcptDat->turi_dat[3].sei_nyu;
			wk_mai = (ushort)(p_RcptDat->turi_dat[3].sei_syu + p_RcptDat->turi_dat[3].kyosei);
			if( p_NtDat->Coin[3].Mai < wk_mai ){
				p_NtDat->Coin[3].Mai = 0;
			}else{
				p_NtDat->Coin[3].Mai -= wk_mai;
			}

			for( i=0;i<2;i++ ){
				switch((( p_RcptDat->sub_tube >> (i*8)) & 0x000F )){
					case 0x01:
						pos = 0;
						p_NtDat->CoinYotiku[i].Money = 10;								// ������z(10�~)
						break;
					case 0x02:
						pos = 1;
						p_NtDat->CoinYotiku[i].Money = 50;								// ������z(50�~)
						break;
					case 0x04:
						pos = 2;
						p_NtDat->CoinYotiku[i].Money = 100;								// ������z(100�~)
						break;
					case 0:
					default:															// �ڑ��Ȃ�
						continue;
				}
				p_NtDat->CoinYotiku[i].Mai = p_RcptDat->turi_dat[pos].yzen_mai;	// ��ݗ\�~����(10�~)
				wk_mai = (ushort)(p_RcptDat->turi_dat[pos].ysei_syu + p_RcptDat->turi_dat[pos].ykyosei);
				if( p_NtDat->CoinYotiku[i].Mai < wk_mai ){
					p_NtDat->CoinYotiku[i].Mai = 0;
				}else{
					p_NtDat->CoinYotiku[i].Mai -= wk_mai;
				}
			}
			break;
		}
		p_NtDat->Coin[0].Money = 10;										// ��ݏz����(10�~)
		p_NtDat->Coin[1].Money = 50;										// ��ݏz����(50�~)
		p_NtDat->Coin[2].Money = 100;										// ��ݏz����(100�~)
		p_NtDat->Coin[3].Money = 500;										// ��ݏz����(500�~)
	}
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
	// ���O�쐬���ɔ�����s���Ă���̂ŁA�����ł͕s�v�ł��邪�c���Ă���
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
	/* ���Z���쎞�͋��K���ɕω��������ꍇ�͑��M���Ȃ� */
	f_Send = 1;
	if( p_RcptDat->PayClass <= 5 ){											// ���Z����i����or���~�B�j
		if( 0 == memcmp( &p_NtDat->CoinSf[0], 								// ������񂪑O�񑗐M���e�Ɠ����i�d���� ��݋��Ɉȉ��j
						 &Ntnet_Prev_SData126.CoinSf[0],
						 (sizeof(DATA_KIND_126) - sizeof(DATA_BASIC) - 20) ) ){
			f_Send = 0;														// ���K�Ǘ��f�[�^�𑗐M���Ȃ�
		}
	//�� �d��ON����̒ޑK��[�ɂċ��K�Ǘ��f�[�^�����M����Ȃ��s��C�� 
		//�N������0�̎��͑��M���Ȃ�(�ŏ��P�ʂƔ�r�Ƃ��Ă���)
		if( (p_NtDat->PayTime.Year < 1980) ||
			(p_NtDat->PayTime.Mon < 1) ||
			(p_NtDat->PayTime.Day < 1) ){
			f_Send = 0;														// ���K�Ǘ��f�[�^�𑗐M���Ȃ�
		}
	}

	if( f_Send ){															// ���K�Ǘ��f�[�^�𑗐M����
		memcpy( &Ntnet_Prev_SData126, p_NtDat, sizeof(DATA_KIND_126) );
																			// �O�񑗐M ���K�Ǘ��f�[�^�ۑ�
		ret = sizeof( DATA_KIND_126 );
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ޑK�Ǘ��W�v�f�[�^(�f�[�^���135)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�����ޑK�Ǘ����̃|�C���^
///	@param[out]		p_NtDat   : �ޑK�Ǘ��f�[�^(DATA_KIND_135�^)�ւ̃|�C���^
///	@return			ret       : �ޑK�Ǘ��f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`)
///	@author			
///	@attention		
///	@note			�O��ҏW���e�Ɠ����ŋ��K�Ǘ��f�[�^���M�s�v�̏ꍇ�Aret=0��Ԃ�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/12/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data135( TURI_KAN *p_TuriDat, DATA_KIND_135 *p_NtDat )
{
	char	i;
	ushort	ret = 0;
	uchar	pos = 0;

	memset( p_NtDat, 0, sizeof( DATA_KIND_135 ) );

	NTNET_Edit_BasicData( 135, 0, p_TuriDat->SeqNo, &p_NtDat->DataBasic);	// ��{�ް��쐬
	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_TuriDat->ProcDate.Year % 100 );		// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_TuriDat->ProcDate.Mon;				// ������
	p_NtDat->DataBasic.Day = (uchar)p_TuriDat->ProcDate.Day;				// ������
	p_NtDat->DataBasic.Hour = (uchar)p_TuriDat->ProcDate.Hour;				// ������
	p_NtDat->DataBasic.Min = (uchar)p_TuriDat->ProcDate.Min;				// ������
	p_NtDat->DataBasic.Sec = 0;												// �����b

	p_NtDat->FmtRev = 10;													// �t�H�[�}�b�gRev.No

	p_NtDat->CenterSeqNo = p_TuriDat->CenterSeqNo;							// �Z���^�[�ǔԁi�ޑK�Ǘ��j
	
	p_NtDat->Oiban = CountSel( &p_TuriDat->Oiban );							// ���K�Ǘ����v�ǔ�

	p_NtDat->PayClass = p_TuriDat->PayClass;								// �����敪


	p_NtDat->KakariNo = p_TuriDat->Kakari_no;								// �W����

	// �R�C������1�`4
	for( i=0; i<4; i++ ){
		p_NtDat->turi_dat[i].Kind = c_coin[i];								// ������z(10/50/100/500)
		p_NtDat->turi_dat[i].gen_mai = p_TuriDat->turi_dat[i].gen_mai;		// ���݁i�ύX�O�j�ۗL����
		p_NtDat->turi_dat[i].zen_mai = p_TuriDat->turi_dat[i].zen_mai;		// �O��ۗL����
		p_NtDat->turi_dat[i].sei_nyu = p_TuriDat->turi_dat[i].sei_nyu;		// ���Z����������
		if(p_TuriDat->turi_dat[i].sei_syu <= 0xFFFF){
			p_NtDat->turi_dat[i].sei_syu = p_TuriDat->turi_dat[i].sei_syu;	// ���Z���o������
		}
		else{
			p_NtDat->turi_dat[i].sei_syu = 0xFFFF;							// ���Z���o������
		}
		p_NtDat->turi_dat[i].jyun_syu = 0;									// �z�o������
		if(p_TuriDat->turi_dat[i].hojyu <= 0xFFFF){
			p_NtDat->turi_dat[i].hojyu = p_TuriDat->turi_dat[i].hojyu;		// �ޑK��[����
		}
		else{
			p_NtDat->turi_dat[i].hojyu = 0xFFFF;							// �ޑK��[����
		}
		if(p_TuriDat->turi_dat[i].hojyu_safe <= 0xFFFF){
			p_NtDat->turi_dat[i].hojyu_safe = p_TuriDat->turi_dat[i].hojyu_safe;// �ޑK��[�����ɔ�������
		}
		else{
			p_NtDat->turi_dat[i].hojyu_safe = 0xFFFF;						// �ޑK��[�����ɔ�������
		}
		if(p_TuriDat->turi_dat[i].kyosei <= 0xFFFF){
			p_NtDat->turi_dat[i].turi_kyosei = p_TuriDat->turi_dat[i].kyosei;	// �������o����(�ޑK��)
		}
		else{
			p_NtDat->turi_dat[i].turi_kyosei = 0xFFFF;						// �������o����(�ޑK��)
		}
		p_NtDat->turi_dat[i].kin_kyosei = 0;								// �������o����(����)
		p_NtDat->turi_dat[i].sin_mai = p_TuriDat->turi_dat[i].sin_mai;		// �V�K�ݒ薇��
	}

		
	for( i=0;i<2;i++ ){
		switch((( p_TuriDat->sub_tube >> (i*8)) & 0x000F )){
			case 0x01:
				pos = 0;
				p_NtDat->yturi_dat[i].Kind = 10;								// ������z(10�~)
				break;
			case 0x02:
				pos = 1;
				p_NtDat->yturi_dat[i].Kind = 50;								// ������z(50�~)
				break;
			case 0x04:
				pos = 2;
				p_NtDat->yturi_dat[i].Kind = 100;								// ������z(100�~)
				break;
			case 0:
			default:															// �ڑ��Ȃ�
				continue;
		}
		p_NtDat->yturi_dat[i].gen_mai = p_TuriDat->turi_dat[pos].ygen_mai;		// ���݁i�ύX�O�j�ۗL����
		p_NtDat->yturi_dat[i].zen_mai = p_TuriDat->turi_dat[pos].yzen_mai;		// �O��ۗL����
		p_NtDat->yturi_dat[i].sei_nyu = 0;										// ���Z����������
		if(p_TuriDat->turi_dat[pos].ysei_syu <= 0xFFFF){
			p_NtDat->yturi_dat[i].sei_syu = p_TuriDat->turi_dat[pos].ysei_syu;	// ���Z���o������
		}
		else{
			p_NtDat->yturi_dat[i].sei_syu = 0xFFFF;								// ���Z���o������
		}
		p_NtDat->yturi_dat[i].jyun_syu = 0;										// �z�o������
		p_NtDat->yturi_dat[i].hojyu = 0;										// �ޑK��[����
		p_NtDat->yturi_dat[i].hojyu_safe = 0;									// �ޑK��[�����ɔ�������
		if(p_TuriDat->turi_dat[pos].ykyosei <= 0xFFFF){
			p_NtDat->yturi_dat[i].turi_kyosei = p_TuriDat->turi_dat[pos].ykyosei;		// �������o����(�ޑK��)
		}
		else{
			p_NtDat->yturi_dat[i].turi_kyosei = 0xFFFF;							// �������o����(�ޑK��)
		}
		p_NtDat->yturi_dat[i].kin_kyosei = 0;									// �������o����(����)
		p_NtDat->yturi_dat[i].sin_mai = p_TuriDat->turi_dat[pos].ysin_mai;		// �V�K�ݒ薇��
	}
	ret = sizeof( DATA_KIND_135 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�W�v���O�f�[�^�쐬(LOG�ɕۑ�����`��)
//[]----------------------------------------------------------------------[]
///	@param[in]		void 
///	@return			void
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Make_Log_TGOUKEI( void )
{
	// ���O���[�N�G���A�N���A
	memset( &TSYU_LOG_WK, 0, sizeof( SYUKEI ) );
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	memset( &LCKT_LOG_WK, 0, sizeof( LCKTTL_LOG ) );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	// �s�W�v�G���A�R�s�[
	memcpy( &TSYU_LOG_WK, &skyprn, sizeof( SYUKEI ) );
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	// �s�W�v�G���A��30�Ԏ����̃f�[�^�R�s�[
//	memcpy( &TSYU_LOG_WK.loktldat[0], &loktl.tloktl.loktldat[0], sizeof( LOKTOTAL_DAT )*SYU_LOCK_MAX );
//	// �Ԏ����Ԉʒu�ʏW�v(���Ԉʒu��)�̊�{�f�[�^���s�W�v�G���A����Z�b�g
//	LCKT_LOG_WK.Kikai_no = TSYU_LOG_WK.Kikai_no;
//	LCKT_LOG_WK.Kakari_no = TSYU_LOG_WK.Kakari_no;
//	LCKT_LOG_WK.Oiban = TSYU_LOG_WK.Oiban;
//	memcpy( &LCKT_LOG_WK.NowTime, &TSYU_LOG_WK.NowTime, sizeof( date_time_rec ) );
//	memcpy( &LCKT_LOG_WK.OldTime, &TSYU_LOG_WK.OldTime, sizeof( date_time_rec ) );
//	// 31�Ԏ��ȍ~�̒��Ԉʒu�ʏW�v(���Ԉʒu��)�R�s�[
//	memcpy( &LCKT_LOG_WK.loktldat[0], &loktl.tloktl.loktldat[SYU_LOCK_MAX], sizeof( LOKTOTAL_DAT )*(LOCK_MAX-SYU_LOCK_MAX) );
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ������������� 2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		TSYU_LOG_WK.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_TOTAL);
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�W�v��{�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: �W�v��{�f�[�^(DATA_KIND_30�^)�ւ̃|�C���^  
///	@return			ret		: �W�v��{�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiKihon( SYUKEI *syukei, ushort Type, DATA_KIND_30 *p_NtDat )
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_30 ) );
	if( Type >= 1 && Type <= 6) {												// �s���v
		NTNET_Edit_BasicData( 30, 0, syukei->SeqNo[0], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
	}else{
		NTNET_Edit_BasicData( 30, 1, syukei->SeqNo[0], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b

	p_NtDat->Type								= Type;							// �W�v�^�C�v
	p_NtDat->KakariNo							= syukei->Kakari_no;			// �W��No.
	p_NtDat->SeqNo								= CountSel( &syukei->Oiban );	// �W�v�ǔ�
	ntnet_DateTimeCnv(&p_NtDat->NowTime, &syukei->NowTime);						// ����W�v
	ntnet_DateTimeCnv(&p_NtDat->LastTime, &syukei->OldTime);					// �O��W�v
	p_NtDat->SettleNum							= syukei->Seisan_Tcnt;			// �����Z��
	p_NtDat->Kakeuri							= syukei->Kakeuri_Tryo;			// ���|���z
	p_NtDat->Cash								= syukei->Genuri_Tryo;			// ����������z
	p_NtDat->Uriage								= syukei->Uri_Tryo;				// ������z
	p_NtDat->Tax								= syukei->Tax_Tryo;				// ������Ŋz
	p_NtDat->Charge								= syukei->Turi_modosi_ryo;		// �ޑK���ߊz
	p_NtDat->CoinTotalNum						= syukei->Ckinko_goukei_cnt;	// �R�C�����ɍ��v��
	p_NtDat->NoteTotalNum						= syukei->Skinko_goukei_cnt;	// �������ɍ��v��
	p_NtDat->CyclicCoinTotalNum					= syukei->Junkan_goukei_cnt;	// �z�R�C�����v��
	p_NtDat->NoteOutTotalNum					= syukei->Siheih_goukei_cnt;	// �������o�@���v��
	p_NtDat->SettleNumServiceTime				= syukei->In_svst_seisan;		// �T�[�r�X�^�C�������Z��
	p_NtDat->Shortage.Num						= syukei->Harai_husoku_cnt;		// ���o�s����
	p_NtDat->Shortage.Amount					= syukei->Harai_husoku_ryo;		// ���o�s�����z
	p_NtDat->Cancel.Num							= syukei->Seisan_chusi_cnt;		// ���Z���~��
	p_NtDat->Cancel.Amount						= syukei->Seisan_chusi_ryo;		// ���Z���~���z
	p_NtDat->AntiPassOffSettle					= syukei->Apass_off_seisan;		// �A���`�p�XOFF���Z��
	p_NtDat->ReceiptIssue						= syukei->Ryosyuu_pri_cnt;		// �̎��ؔ��s����
	p_NtDat->WarrantIssue						= syukei->Azukari_pri_cnt;		// �a��ؔ��s����
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	p_NtDat->AllSystem.CarOutIllegal.Num		= syukei->Husei_out_Tcnt;		// �S���u  �s���o�ɉ�
//	p_NtDat->AllSystem.CarOutIllegal.Amount		= syukei->Husei_out_Tryo;		//                 ���z
//	p_NtDat->AllSystem.CarOutForce.Num			= syukei->Kyousei_out_Tcnt;		//         �����o�ɉ�
//	p_NtDat->AllSystem.CarOutForce.Amount		= syukei->Kyousei_out_Tryo;		//                 ���z
//	p_NtDat->AllSystem.AcceptTicket				= syukei->Uketuke_pri_Tcnt;		//         ��t�����s��
//	p_NtDat->AllSystem.ModifySettle.Num			= syukei->Syuusei_seisan_Tcnt;	// �C�����Z��
//	p_NtDat->AllSystem.ModifySettle.Amount		= syukei->Syuusei_seisan_Tryo;	//         ���z
//	p_NtDat->CarInTotal							= syukei->In_car_Tcnt;			// �����ɑ䐔
//	p_NtDat->CarOutTotal						= syukei->Out_car_Tcnt;			// ���o�ɑ䐔
//	p_NtDat->CarIn1								= syukei->In_car_cnt[0];		// ����1���ɑ䐔
//	p_NtDat->CarOut1							= syukei->Out_car_cnt[0];		// �o��1�o�ɑ䐔
//	p_NtDat->CarIn2								= syukei->In_car_cnt[1];		// ����2���ɑ䐔
//	p_NtDat->CarOut2							= syukei->Out_car_cnt[1];		// �o��2�o�ɑ䐔
//	p_NtDat->CarIn3								= syukei->In_car_cnt[2];		// ����3���ɑ䐔
//	p_NtDat->CarOut3							= syukei->Out_car_cnt[2];		// �o��3�o�ɑ䐔
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ������������� 2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// �����������ł̖������͊�{�ް��ɍ��ڂ�V�݂��đ��M����
	p_NtDat->MiyoCount							= syukei->Syuusei_seisan_Mcnt;	// ��������
	p_NtDat->MiroMoney							= syukei->Syuusei_seisan_Mryo;	// �������z
	p_NtDat->LagExtensionCnt					= syukei->Lag_extension_cnt;	// ���O�^�C��������
//	�\���ر�̗]���Rev2[11]

	ret = sizeof( DATA_KIND_30 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			������ʖ��W�v�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: ������ʖ��W�v�f�[�^(DATA_KIND_31�^)�ւ̃|�C���^  
///	@return			ret		: ������ʖ��W�v�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiRyokinMai( SYUKEI *syukei, ushort Type, DATA_KIND_31 *p_NtDat )
{
	int		i;
	int		j;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_31 ) );
	if( Type >= 1 && Type <= 6) {												// �s���v
		NTNET_Edit_BasicData( 31, 0, syukei->SeqNo[1], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
	}else{
		NTNET_Edit_BasicData( 31, 1, syukei->SeqNo[1], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b

	p_NtDat->Type					= Type;										// �W�v�^�C�v
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// �W�v�ǔ�
	for (i = 0, j = 0; i < RYOUKIN_SYU_CNT; i++) {								// ���01�`50
		if (syukei->Rsei_cnt[i] == 0 && syukei->Rsei_ryo[i] == 0
		 && syukei->Rtwari_cnt[i] == 0 && syukei->Rtwari_ryo[i] == 0) {
		 	// �f�[�^�����ׂĂO�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			p_NtDat->Kind[j].Kind	= i + 1;									//            �������
			p_NtDat->Kind[j].Settle.Num		= syukei->Rsei_cnt[i];				//            ���Z��
			p_NtDat->Kind[j].Settle.Amount		= syukei->Rsei_ryo[i];			//            ����z
			p_NtDat->Kind[j].Discount.Num		= syukei->Rtwari_cnt[i];		//            ������
			p_NtDat->Kind[j].Discount.Amount	= syukei->Rtwari_ryo[i];		//            �����z
			j++;
		}
	}

	ret = sizeof( DATA_KIND_31 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���ޏW�v�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: ���ޏW�v�f�[�^(DATA_KIND_32�^)�ւ̃|�C���^  
///	@return			ret		: ���ޏW�v�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiBunrui( SYUKEI *syukei, ushort Type, DATA_KIND_32 *p_NtDat )
{
	int		i;
	char	j = 0;
	char	cnt = 0;
	char	flg[3] = {0,0,0};
	char	pram_set[] = {0,0,50,100};
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_32 ) );
	if( Type >= 1 && Type <= 6) {												// �s���v
		NTNET_Edit_BasicData( 32, 0, syukei->SeqNo[2], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
	}else{
		NTNET_Edit_BasicData( 32, 1, syukei->SeqNo[2], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b

	p_NtDat->Type					= Type;										// �W�v�^�C�v
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// �W�v�ǔ�

/*���ޏW�v�P*/
	for(j = 1 ; j <= 3 ; j++){
		for(i = 1 ; i <= 6 ; i++){
			if(j == (uchar)prm_get(COM_PRM, S_BUN, 52, 1, (char)i)){
				flg[cnt] = j;
				cnt++;
				break;
			}
			if(j == (uchar)prm_get(COM_PRM, S_BUN, 53, 1, (char)i)){
				flg[cnt] = j;
				cnt++;
				break;
			}
		}
	}
	if(flg[0]){
		p_NtDat->Kind					= CPrmSS[S_BUN][1]+pram_set[flg[0]];	// ���ԕ��ޏW�v�̎��
		for (i = 0; i < BUNRUI_CNT; i++) {										// ����01�`48
			p_NtDat->Group[i].Num		= syukei->Bunrui1_cnt[flg[0]-1][i];		//            �䐔1
			p_NtDat->Group[i].Amount	= syukei->Bunrui1_ryo[flg[0]-1][i];		//            �䐔2�^���z
		}
		p_NtDat->GroupTotal.Num		= syukei->Bunrui1_cnt1[flg[0]-1];			// ���ވȏ�   �䐔1
		p_NtDat->GroupTotal.Amount		= syukei->Bunrui1_ryo1[flg[0]-1];		//            �䐔2�^���z
		p_NtDat->Unknown.Num			= syukei->Bunrui1_cnt2[flg[0]-1];		// ���ޕs��   �䐔1
		p_NtDat->Unknown.Amount		= syukei->Bunrui1_ryo2[flg[0]-1];			//            �䐔2�^���z
		flg[0] = 0;
	}
/*���ޏW�v�Q*/
	if(flg[1]){
		p_NtDat->Kind2					= CPrmSS[S_BUN][1]+pram_set[flg[1]];	// ���ԕ��ޏW�v�̎��
		for (i = 0; i < BUNRUI_CNT; i++) {										// ����01�`48
			p_NtDat->Group2[i].Num		= syukei->Bunrui1_cnt[flg[1]-1][i];		//            �䐔1
			p_NtDat->Group2[i].Amount	= syukei->Bunrui1_ryo[flg[1]-1][i];		//            �䐔2�^���z
		}
		p_NtDat->GroupTotal2.Num		= syukei->Bunrui1_cnt1[flg[1]-1];		// ���ވȏ�   �䐔1
		p_NtDat->GroupTotal2.Amount		= syukei->Bunrui1_ryo1[flg[1]-1];		//            �䐔2�^���z
		p_NtDat->Unknown2.Num			= syukei->Bunrui1_cnt2[flg[1]-1];		// ���ޕs��   �䐔1
		p_NtDat->Unknown2.Amount		= syukei->Bunrui1_ryo2[flg[1]-1];		//            �䐔2�^���z
		flg[1] = 0;
	}
/*���ޏW�v�R*/
	if(flg[2]){
		p_NtDat->Kind3					= CPrmSS[S_BUN][1]+pram_set[flg[2]];	// ���ԕ��ޏW�v�̎��
		for (i = 0; i < BUNRUI_CNT; i++) {										// ����01�`48
			p_NtDat->Group3[i].Num		= syukei->Bunrui1_cnt[flg[2]-1][i];		//            �䐔1
			p_NtDat->Group3[i].Amount	= syukei->Bunrui1_ryo[flg[2]-1][i];		//            �䐔2�^���z
		}
		p_NtDat->GroupTotal3.Num		= syukei->Bunrui1_cnt1[flg[2]-1];		// ���ވȏ�   �䐔1
		p_NtDat->GroupTotal3.Amount		= syukei->Bunrui1_ryo1[flg[2]-1];		//            �䐔2�^���z
		p_NtDat->Unknown3.Num			= syukei->Bunrui1_cnt2[flg[2]-1];		// ���ޕs��   �䐔1
		p_NtDat->Unknown3.Amount		= syukei->Bunrui1_ryo2[flg[2]-1];		//            �䐔2�^���z
		flg[2] = 0;
	}

	ret = sizeof( DATA_KIND_32 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����W�v�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: �����W�v�f�[�^(DATA_KIND_33�^)�ւ̃|�C���^  
///	@return			ret		: �����W�v�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiWaribiki( SYUKEI *syukei, ushort Type, DATA_KIND_33 *p_NtDat )
{
	int		i;
	int		parking;
	int		group;
	ushort	ret;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//// MH341107(S) K.Onodera 2016/11/08 AI-V�Ή�
//	ushort	mod;
//// MH341107(E) K.Onodera 2016/11/08 AI-V�Ή�
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	ulong	sei_cnt, sei_ryo;		// �W�v�p
	int	j;
// MH321800(E) hosoda IC�N���W�b�g�Ή�

	wk_media_Type = 0;
	
	memset( p_NtDat, 0, sizeof( DATA_KIND_33 ) );
	if( Type >= 1 && Type <= 6) {												// �s���v
		NTNET_Edit_BasicData( 33, 0, syukei->SeqNo[3], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
	}else{
		NTNET_Edit_BasicData( 33, 1, syukei->SeqNo[3], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b

	p_NtDat->Type					= Type;										// �W�v�^�C�v
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// �W�v�ǔ�
	i = 0;
	

// �N���W�b�g
	if (syukei->Ccrd_sei_cnt != 0 || syukei->Ccrd_sei_ryo != 0) {

		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ��{���ԏ�ԍ����

		p_NtDat->Discount[i].Kind		= 30;
		p_NtDat->Discount[i].Num	= syukei->Ccrd_sei_cnt;
		p_NtDat->Discount[i].Amount	= syukei->Ccrd_sei_ryo;
		i++;
	}

// �T�[�r�X��
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		for (group = 0; group < SERVICE_SYU_CNT; group++) {
			if (syukei->Stik_use_cnt[parking][group] == 0
			 && syukei->Stik_use_ryo[parking][group] == 0) {
				// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
				continue;
			} else {
				p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
				p_NtDat->Discount[i].Kind		= 1;
				p_NtDat->Discount[i].Group		= group + 1;
				p_NtDat->Discount[i].Num		= syukei->Stik_use_cnt[parking][group];
				p_NtDat->Discount[i].Amount	= syukei->Stik_use_ryo[parking][group];
				i++;
			}
		}
	}

// �v���y�C�h
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Pcrd_use_cnt[parking] == 0
		 && syukei->Pcrd_use_ryo[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 11;
			p_NtDat->Discount[i].Num		= syukei->Pcrd_use_cnt[parking];
			p_NtDat->Discount[i].Amount	= syukei->Pcrd_use_ryo[parking];
			i++;
		}
	}

// �񐔌�
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Ktik_use_cnt[parking] == 0
		 && syukei->Ktik_use_ryo[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 5;
			p_NtDat->Discount[i].Num		= syukei->Ktik_use_cnt[parking];
			p_NtDat->Discount[i].Amount	= syukei->Ktik_use_ryo[parking];
			i++;
		}
	}

// �X���� �XNo.��
	for (parking = 0; parking < PKNO_WARI_CNT; parking++) {
		for (group = 0; group < MISE_NO_CNT; group++) {
			if (syukei->Mno_use_cnt4[parking][group] == 0
			 && syukei->Mno_use_ryo4[parking][group] == 0) {
				// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
				continue;
			} else {
				p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
				p_NtDat->Discount[i].Kind		= 2;
				p_NtDat->Discount[i].Group		= group + 1;
				p_NtDat->Discount[i].Num		= syukei->Mno_use_cnt4[parking][group];
				p_NtDat->Discount[i].Amount	= syukei->Mno_use_ryo4[parking][group];
				i++;
			}
		}
	}

// �X���� ���ԏ�No.��
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt1[parking] == 0
		 && syukei->Mno_use_ryo1[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 60;
			p_NtDat->Discount[i].Num		= syukei->Mno_use_cnt1[parking];
			p_NtDat->Discount[i].Amount	= syukei->Mno_use_ryo1[parking];
			i++;
		}
	}

// �X���� 1�`100���v
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt2[parking] == 0
		 && syukei->Mno_use_ryo2[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 61;
			p_NtDat->Discount[i].Num		= syukei->Mno_use_cnt2[parking];
			p_NtDat->Discount[i].Amount	= syukei->Mno_use_ryo2[parking];
			i++;
		}
	}

// �X���� 101�`255���v
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if (syukei->Mno_use_cnt3[parking] == 0
		 && syukei->Mno_use_ryo3[parking] == 0) {
			// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
			continue;
		} else {
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
			p_NtDat->Discount[i].Kind		= 62;
			p_NtDat->Discount[i].Num		= syukei->Mno_use_cnt3[parking];
			p_NtDat->Discount[i].Amount	= syukei->Mno_use_ryo3[parking];
			i++;
		}
	}

// �X���� �S�������v
	if (syukei->Mno_use_Tcnt != 0 || syukei->Mno_use_Tryo != 0) {

		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ��{���ԏ�ԍ����
		p_NtDat->Discount[i].Kind		= 63;
		p_NtDat->Discount[i].Num		= syukei->Mno_use_Tcnt;
		p_NtDat->Discount[i].Amount	= syukei->Mno_use_Tryo;
		i++;
	}

// ��ʊ���
	for (group = 0; group < RYOUKIN_SYU_CNT; group++) {
		if( syukei->Rtwari_cnt[group] != 0 ){
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];
			p_NtDat->Discount[i].Kind		= 50;
			p_NtDat->Discount[i].Group		= group + 1;
			p_NtDat->Discount[i].Num		= syukei->Rtwari_cnt[group];
			p_NtDat->Discount[i].Amount	= syukei->Rtwari_ryo[group];
			i++;
		}
	}

// ��ʊ���
	if( syukei->Electron_sei_cnt ){
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
		p_NtDat->Discount[i].Kind = NTNET_SUICA_1;								// ������ʁF31�iSuica���ρj�Œ�
		p_NtDat->Discount[i].Group = 0;											// �����敪�F���g�p(0)
		p_NtDat->Discount[i].Num = syukei->Electron_sei_cnt;					// �����񐔁F
		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_sei_ryo;					// �����z  �F
		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
		p_NtDat->Discount[i].Rsv = 0;											// �\���@�@�F���g�p(0)
		i++;
	}

	if( syukei->Electron_psm_cnt ){
// PASMO
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
		p_NtDat->Discount[i].Kind = NTNET_PASMO_0;								// ������ʁF33�iPASMO���ρj�Œ�
		p_NtDat->Discount[i].Group = 0;											// �����敪�F���g�p(0)
		p_NtDat->Discount[i].Num = syukei->Electron_psm_cnt;					// �����񐔁F
		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_psm_ryo;					// �����z  �F
		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
		p_NtDat->Discount[i].Rsv = 0;											// �\���@�@�F���g�p(0)
		i++;
	}
// MH321800(S) hosoda IC�N���W�b�g�Ή�, �]����Edy�������폜
//-	if( syukei->Electron_edy_cnt ){
//-// Edy
//-		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
//-		p_NtDat->Discount[i].Kind = NTNET_EDY_0;								// ������ʁF32�iEdy���ρj�Œ�
//-		p_NtDat->Discount[i].Group = 0;											// �����敪�F(0)���������
//-		p_NtDat->Discount[i].Num = syukei->Electron_edy_cnt;					// �����񐔁F
//-		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
//-		p_NtDat->Discount[i].Amount = syukei->Electron_edy_ryo;					// �����z  �F
//-		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
//-		p_NtDat->Discount[i].Rsv = 0;											// �\���@�@�F���g�p(0)
//-		i++;
//-	}
//-
//-	if( syukei->Electron_Arm_cnt ){
//-		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
//-		p_NtDat->Discount[i].Kind = NTNET_EDY_0;								// ������ʁF32�iEdy���ρj�Œ�
//-		p_NtDat->Discount[i].Group = 1;											// �����敪�F(1)�װю����
//-		p_NtDat->Discount[i].Num = syukei->Electron_Arm_cnt;					// �����񐔁F
//-		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
//-		p_NtDat->Discount[i].Amount = syukei->Electron_Arm_ryo;					// �����z  �F
//-		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
//-		p_NtDat->Discount[i].Rsv = 0;											// �\���@�@�F���g�p(0)
//-		i++;
//-	}
// MH321800(E) hosoda IC�N���W�b�g�Ή�, �]����Edy�������폜

	if( syukei->Electron_ico_cnt ){
// ICOCA
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
		p_NtDat->Discount[i].Kind = NTNET_ICOCA_0;								// ������ʁF35�iICOCA���ρj�Œ�
		p_NtDat->Discount[i].Group = 0;											// �����敪�F(0)���������
		p_NtDat->Discount[i].Num = syukei->Electron_ico_cnt;					// �����񐔁F
		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_ico_ryo;					// �����z  �F
		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
		p_NtDat->Discount[i].Rsv = 0;											// �\���@�@�F���g�p(0)
		i++;
	}
	
	if( syukei->Electron_icd_cnt ){

// IC-Card
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
		p_NtDat->Discount[i].Kind = NTNET_ICCARD_0;								// ������ʁF36�iIC-Card���ρj�Œ�
		p_NtDat->Discount[i].Group = 0;											// �����敪�F(0)���������
		p_NtDat->Discount[i].Num = syukei->Electron_icd_cnt;					// �����񐔁F
		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_icd_ryo;					// �����z  �F
		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
		p_NtDat->Discount[i].Rsv = 0;											// �\���@�@�F���g�p(0)
		i++;
	}

// MH321800(S) hosoda IC�N���W�b�g�Ή�
	// ���W�v�f�[�^�ɃA���[��������̓Z�b�g���Ȃ�
	for (j = 0; j < TBL_CNT(ec_discount_kind_tbl); j++) {
		switch (ec_discount_kind_tbl[j]) {
		case	NTNET_EDY_0:
			sei_cnt = syukei->Electron_edy_cnt;
			sei_ryo = syukei->Electron_edy_ryo;
			break;
		case	NTNET_NANACO_0:
			sei_cnt = syukei->nanaco_sei_cnt;
			sei_ryo = syukei->nanaco_sei_ryo;
			break;
		case	NTNET_WAON_0:
			sei_cnt = syukei->waon_sei_cnt;
			sei_ryo = syukei->waon_sei_ryo;
			break;
		case	NTNET_SAPICA_0:
			sei_cnt = syukei->sapica_sei_cnt;
			sei_ryo = syukei->sapica_sei_ryo;
			break;
		case	NTNET_SUICA_1:
			sei_cnt = syukei->koutsuu_sei_cnt;
			sei_ryo = syukei->koutsuu_sei_ryo;
			break;
		case	NTNET_ID_0:
			sei_cnt = syukei->id_sei_cnt;
			sei_ryo = syukei->id_sei_ryo;
			break;
		case	NTNET_QUICPAY_0:
			sei_cnt = syukei->quicpay_sei_cnt;
			sei_ryo = syukei->quicpay_sei_ryo;
			break;
		default	:
			sei_cnt = 0L;
			sei_ryo = 0L;
			break;
		}
		if (sei_cnt != 0L) {
		// ���ϏW�v
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
			p_NtDat->Discount[i].Kind = ec_discount_kind_tbl[j];				// ������ʁFEdy/nanaco/WAON/SAPICA�Ȃ�
			p_NtDat->Discount[i].Group = 0;										// �����敪�F(0)���������
			p_NtDat->Discount[i].Num = sei_cnt;									// �����񐔁F
			p_NtDat->Discount[i].Callback = 0;									// ��������F���g�p(0)
			p_NtDat->Discount[i].Amount = sei_ryo;								// �����z  �F
			p_NtDat->Discount[i].Info = 0;										// �������F���g�p(0)
			p_NtDat->Discount[i].Rsv = 0;										// �\���@�@�F���g�p(0)

			i++;
		}
	}
// MH321800(E) hosoda IC�N���W�b�g�Ή�

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	if (syukei->Gengaku_seisan_cnt) {
//		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
//		p_NtDat->Discount[i].Kind = NTNET_GENGAKU;								// ������ʁF���z���Z
//		p_NtDat->Discount[i].Group = 0;											// �����敪�F���g�p(0)
//		p_NtDat->Discount[i].Num = syukei->Gengaku_seisan_cnt;					// �����񐔁F
//		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
//		p_NtDat->Discount[i].Amount = syukei->Gengaku_seisan_ryo;				// �����z  �F
//		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
//		p_NtDat->Discount[i].Rsv = 0;											// �\���@�@�F���g�p(0)
//		i++;
//	}
//
//	if (syukei->Furikae_seisan_cnt) {
//		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
//		p_NtDat->Discount[i].Kind = NTNET_FURIKAE;								// ������ʁF�U�֐��Z
//		p_NtDat->Discount[i].Group = 0;											// �����敪�F���g�p(0)
//		p_NtDat->Discount[i].Num = syukei->Furikae_seisan_cnt;					// �����񐔁F
//		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
//		p_NtDat->Discount[i].Amount = syukei->Furikae_seisan_ryo;				// �����z  �F
//		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
//		p_NtDat->Discount[i].Rsv = 0;											// �\���@�@�F���g�p(0)
//		i++;
//	}
//// MH341107(S) K.Onodera 2016/11/08 AI-V�Ή�
//	for( mod=1; mod<MOD_TYPE_MAX; mod++ ){
//		if( syukei->Furikae_CardKabarai[mod] ){
//			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
//			p_NtDat->Discount[i].Kind = NTNET_KABARAI;							// ������ʁF�U�֐��Z(ParkingWeb��)
//			p_NtDat->Discount[i].Group = mod;									// �����敪�F�}�̎��
//			p_NtDat->Discount[i].Num = syukei->Furikae_Card_cnt[mod];			// �����񐔁F���g�p(0)
//			p_NtDat->Discount[i].Callback = 0;									// ��������F���g�p(0)
//			p_NtDat->Discount[i].Amount = syukei->Furikae_CardKabarai[mod];		// �����z  �F�ߕ������z
//			p_NtDat->Discount[i].Info = 0;										// �������F���g�p(0)
//			i++;
//		}
//	}
//// MH341107(E) K.Onodera 2016/11/08 AI-V�Ή�
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	ret = sizeof( DATA_KIND_33 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			����W�v�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: ����W�v�f�[�^(DATA_KIND_34�^)�ւ̃|�C���^  
///	@return			ret		: ����W�v�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiTeiki( SYUKEI *syukei, ushort Type, DATA_KIND_34 *p_NtDat )
{
	int		i;
	int		parking;
	int		kind;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_34 ) );
	if( Type >= 1 && Type <= 6) {												// �s���v
		NTNET_Edit_BasicData( 34, 0, syukei->SeqNo[4], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
	}else{
		NTNET_Edit_BasicData( 34, 1, syukei->SeqNo[4], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b

	p_NtDat->Type					= Type;										// �W�v�^�C�v
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// �W�v�ǔ�
	i = 0;
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		for (kind = 0; kind < TEIKI_SYU_CNT; kind++) {
			if ( ( syukei->Teiki_use_cnt[parking][kind] == 0 ) &&				// ����g�p�񐔂��O�̏ꍇ
				 ( syukei->Teiki_kou_cnt[parking][kind] == 0 ) ) {				// ����X�V�񐔂��O�̏ꍇ
				// �f�[�^���O�̏ꍇ�͓d���Ɋi�[�����X�L�b�v����B
				continue;
			} else {
				p_NtDat->Pass[i].ParkingNo		= CPrmSS[S_SYS][1+parking];		// ���ԏ�No.
				p_NtDat->Pass[i].Kind			= kind + 1;						// ���
				p_NtDat->Pass[i].Num			= syukei->Teiki_use_cnt[parking][kind];	// ��
				p_NtDat->Pass[i].Update.Num		= syukei->Teiki_kou_cnt[parking][kind];	// �X�V��
				p_NtDat->Pass[i].Update.Amount	= syukei->Teiki_kou_ryo[parking][kind];	// �X�V������z
				i++;
			}
		}
	}

	ret = sizeof( DATA_KIND_34 ); 
	return ret;
}

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// //[]----------------------------------------------------------------------[]
// ///	@brief			�Ԏ����W�v�f�[�^�ҏW����
// //[]----------------------------------------------------------------------[]
// ///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
// ///	@param[in]		Type	: �W�v�^�C�v
// ///	@param[out]		p_NtDat	: �Ԏ����W�v�f�[�^(DATA_KIND_35�^)�ւ̃|�C���^  
// ///	@return			ret		: �Ԏ����W�v�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
// ///	@author			m.nagashima
// ///	@attention		
// ///	@note			
// //[]----------------------------------------------------------------------[]
// ///	@date			Create	: 2012/02/24<br>
// ///					Update	: 
// //[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// unsigned short	NTNET_Edit_SyukeiShashitsuMai( SYUKEI *syukei, ushort Type, DATA_KIND_35 *p_NtDat )
// {
// 	t_SyuSub_Lock	*dst;
// 	LOKTOTAL_DAT	*src;
// 	int		i;
// 	int		cnt;
// 	ulong	posi;
// 	ushort	ret;
// 
// 	// �W�v�f�[�^�̒ǔԂƈ�v����Ԏ����W�v�f�[�^����������
// 	ret = Ope_Log_ShashitsuLogGet( syukei, &lckttl_wk );
// 	memset( p_NtDat, 0, sizeof( DATA_KIND_35 ) );
// 	if( Type >= 1 && Type <= 6) {
// 		NTNET_Edit_BasicData( 35, 0, syukei->SeqNo[5], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
// 	}else{
// 		NTNET_Edit_BasicData( 35, 1, syukei->SeqNo[5], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 1
// 	}
// 
// 	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
// 	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
// 	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
// 	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
// 	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
// 	p_NtDat->DataBasic.Sec = 0;													// �����b
// 
// 	p_NtDat->Type					= Type;										// �W�v�^�C�v
// 	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
// 	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// �W�v�ǔ�
// 	for (i = 0 , cnt = 0; i < LOCK_MAX; i++) {
// 		WACDOG;																	// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
// 		if(LockInfo[i].lok_no != 0){											// �ڑ��L�H
// 			if( !SetCarInfoSelect((short)i) ){
// 				continue;
// 			}
// 			if( cnt >= OLD_LOCK_MAX ){
// 				break;
// 			}
// 			posi = (ulong)(( LockInfo[i].area * 10000L ) + LockInfo[i].posi );	// ���Ԉʒu/
// 			dst = &p_NtDat->Lock[cnt];
// 			if( Type >= 1 && Type <= 6) {										// �s���v
// 				// ���O�����o�����f�[�^�̎Ԏ����G���A�̃|�C���^�Z�o
// 				if( i < SYU_LOCK_MAX ){				// 1�`30�Ԏ�
// 					src = &syukei->loktldat[i];
// 				}else{								// 31�Ԏ��`
// 					if( ret ){						// �Ԏ����W�v�f�[�^����OK
// 						src = &lckttl_wk.loktldat[i - SYU_LOCK_MAX];
// 					}else{
// 						break;
// 					}
// 				}
// 			}else{
// 				// �s���v�p�Ԏ����G���A�̃|�C���^�Z�o
// 				src = &loktl.tloktl.loktldat[i];
// 			}
// 			dst->LockNo					= posi;									// �����
// 			dst->CashAmount				= src->Genuri_ryo;						// ��������
// 			dst->Settle					= src->Seisan_cnt;						// ���Z��
// 			dst->CarOutIllegal.Num		= src->Husei_out_cnt;					// �s���o�ɉ�
// 			dst->CarOutIllegal.Amount	= src->Husei_out_ryo;					//         ���z
// 			dst->CarOutForce.Num		= src->Kyousei_out_cnt;					// �����o�ɉ�
// 			dst->CarOutForce.Amount		= src->Kyousei_out_ryo;					//         ���z
// 			dst->AcceptTicket			= src->Uketuke_pri_cnt;					// ��t�����s��
// 			dst->ModifySettle.Num		= src->Syuusei_seisan_cnt;				// �C�����Z��
// 			dst->ModifySettle.Amount	= src->Syuusei_seisan_ryo;				//         ���z
// 			cnt++;
// 		}
// 	}
// 
// 	ret = sizeof( DATA_KIND_35 ); 
// 	return ret;
// }
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

//[]----------------------------------------------------------------------[]
///	@brief			���K�W�v�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: ���K�W�v�f�[�^(DATA_KIND_36�^)�ւ̃|�C���^  
///	@return			ret		: ���K�W�v�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiKinsen( SYUKEI *syukei, ushort Type, DATA_KIND_36 *p_NtDat )
{
	static const ushort c_coin[COIN_SYU_CNT] = {10, 50, 100, 500};
	ulong	w;
	int		i;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_36 ) );
	if( Type >= 1 && Type <= 6) {												// �s���v
		NTNET_Edit_BasicData( 36, 0, syukei->SeqNo[6], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
	}else{
		NTNET_Edit_BasicData( 36, 1, syukei->SeqNo[6], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b

	p_NtDat->Type					= Type;										// �W�v�^�C�v
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// �W�v�ǔ�
	p_NtDat->Total					= syukei->Kinko_Tryo;						// ���ɑ������z
	p_NtDat->NoteTotal				= syukei->Note_Tryo;						// �������ɑ������z
	p_NtDat->CoinTotal				= syukei->Coin_Tryo;						// �R�C�����ɑ������z
	for (i = 0; i < COIN_SYU_CNT; i++) {										// �R�C��1�`4
		p_NtDat->Coin[i].Kind		= c_coin[i];								//        ����
		p_NtDat->Coin[i].Num		= syukei->Coin_cnt[i];						//        ����
	}
	p_NtDat->Note[0].Kind			= 1000;										// ����1  ����
	p_NtDat->Note[0].Num2			= syukei->Note_cnt[0];						//        ��������
	w = 0;
	for (i = 0; i < COIN_SYU_CNT; i++) {
		w += ((syukei->tou[i] + syukei->hoj[i]) * c_coin[i]);
	}
	p_NtDat->CycleAccept			= w;										// �z���������z
	w = 0;
	for (i = 0; i < COIN_SYU_CNT; i++) {
		w += ((syukei->sei[i] + syukei->kyo[i]) * c_coin[i]);
	}
	p_NtDat->CyclePay				= w;										// �z�����o���z
	p_NtDat->NoteAcceptTotal		= syukei->tou[4] * 1000;					// �����������z
	for (i = 0; i < _countof(p_NtDat->Cycle); i++) {
		p_NtDat->Cycle[i].CoinKind			= c_coin[i];						// �z��1�`4 �R�C������
		p_NtDat->Cycle[i].Accept			= syukei->tou[i];					//          ��������
		p_NtDat->Cycle[i].Pay				= syukei->sei[i];					//          �o������
		p_NtDat->Cycle[i].ChargeSupply		= syukei->hoj[i];					//          �ޑK��[����
		p_NtDat->Cycle[i].SlotInventory		= syukei->kyo[i];					//          �C���x���g������(��o��)
	}

	ret = sizeof( DATA_KIND_36 ); 
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�W�v�I���ʒm�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: �W�v�I���ʒm�f�[�^(DATA_KIND_41�^)�ւ̃|�C���^  
///	@return			ret		: �W�v�I���ʒm�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short	NTNET_Edit_SyukeiSyuryo( SYUKEI *syukei, ushort Type, DATA_KIND_41 *p_NtDat )
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_41 ) );
	if( Type >= 1 && Type <= 6) {												// �s���v
		NTNET_Edit_BasicData( 41, 0, syukei->SeqNo[7], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
	}else{
		NTNET_Edit_BasicData( 41, 1, syukei->SeqNo[7], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 1
	}

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b

	p_NtDat->Type					= Type;										// �W�v�^�C�v
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel( &syukei->Oiban );				// �W�v�ǔ�

	ret = sizeof( DATA_KIND_41 ); 
	return ret;
}
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ������������� 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
////--------------------------------------------------------------
////	���Ƀt�H�[�}�b�g����(20/54)
//unsigned char	NTNET_Edit_isData20_54(enter_log *p_RcptDat)		// ����log�ް�(IN)
//{
//	unsigned char ret = 0;
//
//	if (p_RcptDat->CMN_DT.DT_54.ID == 20) {
//		ret = 20;
//	} else if (p_RcptDat->CMN_DT.DT_54.ID == 54) {
//		ret = 54;
//	}
//	return ret;
//}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ������������� 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//--------------------------------------------------------------
//	���Z�t�H�[�}�b�g����(22/56)
unsigned char	NTNET_Edit_isData22_56(Receipt_data *p_RcptDat)		// ���Zlog�ް�(IN)
{
	unsigned char ret = 0;

	if (p_RcptDat->ID == 22) {
		ret = 22;
	} else if (p_RcptDat->ID == 56) {
		ret = 56;
	}
	return ret;
}
//--------------------------------------------------------------
//	���ԑ䐔�t�H�[�}�b�g����(236/58)
unsigned char	NTNET_Edit_isData236_58(ParkCar_log *p_RcptDat)		// ���ԑ䐔�plog�ް�(IN)
{
	unsigned char ret = 0;

	if (p_RcptDat->CMN_DT.DT_58.ID == 236) {
		ret = 236;
	} else if (p_RcptDat->CMN_DT.DT_58.ID == 58) {
		ret = 58;
	}
	return ret;
}
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
////--------------------------------------------------------------
////	�V����NT-NET�t�H�[�}�b�g�ݒ�(20̫�ϯĂ�FmtRev, PARKCAR_DATA1��t��)	// �Q�� = NTNET_Edit_Data20
//unsigned short	NTNET_Edit_Data54(	enter_log 		*p_RcptDat,	// ����log�ް�(IN)
//									DATA_KIND_54 	*p_NtDat )	// ����NT-NET(OUT)
//{
//	unsigned short ret;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_54 ) );
//	NTNET_Edit_BasicData( 54, 0, p_RcptDat->CMN_DT.DT_54.SeqNo, &p_NtDat->DataBasic);	// ��{�f�[�^�쐬
//	memcpy(&p_NtDat->InCount, p_RcptDat, sizeof( enter_log ) - sizeof(p_RcptDat->CMN_DT));
//
//	// �����N�������Z�b�g
//	p_NtDat->DataBasic.Year	= p_RcptDat->CMN_DT.DT_54.ProcDate.Year;	// �����N
//	p_NtDat->DataBasic.Mon	= p_RcptDat->CMN_DT.DT_54.ProcDate.Mon;		// ������
//	p_NtDat->DataBasic.Day	= p_RcptDat->CMN_DT.DT_54.ProcDate.Day;		// ������
//	p_NtDat->DataBasic.Hour	= p_RcptDat->CMN_DT.DT_54.ProcDate.Hour;	// ������
//	p_NtDat->DataBasic.Min	= p_RcptDat->CMN_DT.DT_54.ProcDate.Min;		// ������
//	p_NtDat->DataBasic.Sec	= p_RcptDat->CMN_DT.DT_54.ProcDate.Sec;		// �����b
//
//	// 20 -> 54�ɒǉ����ꂽ�ް��̐ݒ�(FmtRev, PARKCAR_DATA1)
//	p_NtDat->FmtRev = 0;					// �t�H�[�}�b�gRev.��
//	p_NtDat->ParkData.FullNo1 	= p_RcptDat->CMN_DT.DT_54.FullNo1;		// ���ԑ䐔�P
//	p_NtDat->ParkData.CarCnt1 	= p_RcptDat->CMN_DT.DT_54.CarCnt1;		// ���ݑ䐔�P
//	p_NtDat->ParkData.FullNo2 	= p_RcptDat->CMN_DT.DT_54.FullNo2;		// ���ԑ䐔�Q
//	p_NtDat->ParkData.CarCnt2 	= p_RcptDat->CMN_DT.DT_54.CarCnt2;		// ���ݑ䐔�Q
//	p_NtDat->ParkData.FullNo3 	= p_RcptDat->CMN_DT.DT_54.FullNo3;		// ���ԑ䐔�R
//	p_NtDat->ParkData.CarCnt3 	= p_RcptDat->CMN_DT.DT_54.CarCnt3;		// ���ݑ䐔�R
//	p_NtDat->ParkData.PascarCnt = p_RcptDat->CMN_DT.DT_54.PascarCnt;		// ����ԗ��J�E���g
//	p_NtDat->ParkData.Full[0] 	= p_RcptDat->CMN_DT.DT_54.Full[0];		// ����1������
//	p_NtDat->ParkData.Full[1] 	= p_RcptDat->CMN_DT.DT_54.Full[1];		// ����2������
//	p_NtDat->ParkData.Full[2] 	= p_RcptDat->CMN_DT.DT_54.Full[2];		// ����3������
//
//	ret = sizeof( DATA_KIND_54 );
//	return ret;
//}
////	�V����NT-NET�t�H�[�}�b�g�ݒ�(20̫�ϯĂ�FmtRev, PARKCAR_DATA11��t��)	// �Q�� = NTNET_Edit_Data20
//unsigned short	NTNET_Edit_Data54_r10(	enter_log 			*p_RcptDat,		// ����log�ް�(IN)
//										DATA_KIND_54_r10 	*p_NtDat )		// ����NT-NET(OUT)
//{
//	unsigned short ret;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_54_r10 ) );
//	NTNET_Edit_BasicData( 54, 0,  p_RcptDat->CMN_DT.DT_54.SeqNo, &p_NtDat->DataBasic );	// ��{�f�[�^�쐬
//
//	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����Ɏ������Z�b�g����̂ōăZ�b�g
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->InTime.Year % 100);		// ���ɔN
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->InTime.Mon;					// ���Ɍ�
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->InTime.Day;					// ���ɓ�
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->InTime.Hour;				// ���Ɏ�
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->InTime.Min;					// ���ɕ�
//	p_NtDat->DataBasic.Sec	= (uchar)p_RcptDat->InTime.Sec;					// ���ɕb
//
//	p_NtDat->FmtRev = 10;													// �t�H�[�}�b�gRev.��
//
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->CMN_DT.DT_54.FullNo1;			// ���ԑ䐔�P
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CMN_DT.DT_54.CarCnt1;			// ���ݑ䐔�P
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->CMN_DT.DT_54.FullNo2;			// ���ԑ䐔�Q
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CMN_DT.DT_54.CarCnt2;			// ���ݑ䐔�Q
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->CMN_DT.DT_54.FullNo3;			// ���ԑ䐔�R
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CMN_DT.DT_54.CarCnt3;			// ���ݑ䐔�R
//	p_NtDat->ParkData.CarFullFlag	= 0;									// �䐔�E�����ԃt���O
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->CMN_DT.DT_54.PascarCnt;	// ����ԗ��J�E���g
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->CMN_DT.DT_54.Full[0];			// ����1������
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->CMN_DT.DT_54.Full[1];			// ����2������
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->CMN_DT.DT_54.Full[2];			// ����3������
//
//	p_NtDat->CenterSeqNo	= p_RcptDat->CMN_DT.DT_54.CenterSeqNo;			// �Z���^�[�ǔԁi���Ɂj
//
//	p_NtDat->InCount	= p_RcptDat->InCount;								// ���ɒǂ���
//	p_NtDat->Syubet		= (uchar)p_RcptDat->Syubet;							// �����敪
//	p_NtDat->InMode		= (uchar)p_RcptDat->InMode;							// ����Ӱ��
//	p_NtDat->CMachineNo	= (uchar)p_RcptDat->CMachineNo;						// ���Ԍ��@�B��
//	p_NtDat->Reserve	= 0;												// �\���i�T�C�Y�����p�j
//	p_NtDat->FlapArea	= (ushort)(p_RcptDat->LockNo/10000);				// �t���b�v�V�X�e��	���		0�`99
//	p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->LockNo%10000);				// 					�Ԏ��ԍ�	0�`9999
//	p_NtDat->PassCheck	= (uchar)p_RcptDat->PassCheck;						// ����߽����
//	p_NtDat->CountSet	= (uchar)p_RcptDat->CountSet;						// �ݎԶ���
//
//	// ���ɔ}�̏��� 0 ���Z�b�g����
//	memset( &p_NtDat->Media[0], 0, sizeof( p_NtDat->Media ) );				// ���ɔ}�̏��1�`4
//
//	ret = sizeof( DATA_KIND_54_r10 );
//	return ret;
//}
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//--------------------------------------------------------------
//	�V���ZNT-NET�t�H�[�}�b�g�ݒ�(22̫�ϯĂ�FmtRev, PARKCAR_DATA1��t��)	// �Q�� = NTNET_Edit_Data22
unsigned short	NTNET_Edit_Data56(	Receipt_data 	*p_RcptDat,	// ���Zlog�ް�(IN)
									DATA_KIND_56 	*p_NtDat )	// ���ZNT-NET(OUT)
{
	uchar	i;
	uchar	j;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_56 ) );

	p_NtDat->DataBasic.SystemID = p_RcptDat->DataBasic.SystemID;		// ����ID
	p_NtDat->DataBasic.DataKind = p_RcptDat->DataBasic.DataKind;		// �ް����
	p_NtDat->DataBasic.DataKeep = p_RcptDat->DataBasic.DataKeep;		// �ް��ێ��׸�
	p_NtDat->DataBasic.ParkingNo = p_RcptDat->DataBasic.ParkingNo;		// ���ԏꇂ
	p_NtDat->DataBasic.ModelCode = p_RcptDat->DataBasic.ModelCode;		// �@����
	p_NtDat->DataBasic.MachineNo = p_RcptDat->DataBasic.MachineNo;		// �@�B��
	memcpy( &p_NtDat->DataBasic.Year, &p_RcptDat->DataBasic.Year, 6 );	// �����N���������b

	p_NtDat->PayCount = CountSel( &p_RcptDat->Oiban);					// ���Zor���Z���~�ǂ���
	p_NtDat->PayMethod = p_RcptDat->PayMethod;							// ���Z���@
	p_NtDat->PayClass = p_RcptDat->PayClass;							// �����敪
	p_NtDat->PayMode = p_RcptDat->PayMode;								// ���ZӰ��(�������Z)

// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->LockNo = p_RcptDat->WPlace;							// �����
//	}
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	p_NtDat->CardType = 0;												// ���Ԍ�����
// MH810100(S) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	p_NtDat->CMachineNo = 0;											// ���Ԍ��@�B��
	p_NtDat->CMachineNo	= p_RcptDat->CMachineNo;						// ���Ԍ��@�B��
// MH810100(E) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
	p_NtDat->CardNo = 0L;												// ���Ԍ��ԍ�(�����ǂ���)
	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->OutTime.Year = p_RcptDat->TOutTime.Year;				// �o�ɔN
		p_NtDat->OutTime.Mon = p_RcptDat->TOutTime.Mon;					// �o�Ɍ�
		p_NtDat->OutTime.Day = p_RcptDat->TOutTime.Day;					// �o�ɓ�
		p_NtDat->OutTime.Hour = p_RcptDat->TOutTime.Hour;				// �o�Ɏ�
		p_NtDat->OutTime.Min = p_RcptDat->TOutTime.Min;					// �o�ɕ�
		p_NtDat->OutTime.Sec = 0;										// �o�ɕb
		p_NtDat->KakariNo = p_RcptDat->KakariNo;						// �W����
		p_NtDat->OutKind = p_RcptDat->OutKind;							// ���Z�o��
	}
	p_NtDat->ReceiptIssue = p_RcptDat->ReceiptIssue;					// �̎��ؔ��s�L��
	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->InTime.Year = p_RcptDat->TInTime.Year;					// ���ɔN
		p_NtDat->InTime.Mon = p_RcptDat->TInTime.Mon;					// ���Ɍ�
		p_NtDat->InTime.Day = p_RcptDat->TInTime.Day;					// ���ɓ�
		p_NtDat->InTime.Hour = p_RcptDat->TInTime.Hour;					// ���Ɏ�
		p_NtDat->InTime.Min = p_RcptDat->TInTime.Min;					// ���ɕ�
		p_NtDat->InTime.Sec = 0;										// ���ɕb
	}
	// ���Z�N���������b��0�Ƃ���
	// �O�񐸎Z�N���������b��0�Ƃ���
	p_NtDat->TaxPrice = 0;												// �ېőΏۊz
	p_NtDat->TotalPrice = 0;											// ���v���z(HOST���g�p�̂���)
	p_NtDat->Tax = p_RcptDat->Wtax;										// ����Ŋz
	if( p_NtDat->PayMethod != 5 ){										// �X�V���Z�ȊO
		p_NtDat->Syubet = p_RcptDat->syu;								// �������
		p_NtDat->Price = p_RcptDat->WPrice;								// ���ԗ���
	}
	p_NtDat->CashPrice =
						p_RcptDat->WInPrice - p_RcptDat->WChgPrice;		// ��������
	p_NtDat->InPrice = p_RcptDat->WInPrice;								// �������z
	p_NtDat->ChgPrice = p_RcptDat->WChgPrice;							// �ޑK���z
	p_NtDat->Fusoku = p_RcptDat->WFusoku;								// �ޑK���o�s�����z
	p_NtDat->FusokuFlg = 0;												// �ޑK���o�s�������׸�
	p_NtDat->PayObsFlg = 0;												// ���Z����Q�����׸�
	p_NtDat->ChgOverFlg = 0;											// ���ߏ���z���ް�����׸�

	p_NtDat->PassCheck = p_RcptDat->PassCheck;							// ����߽����
	p_NtDat->CountSet = p_RcptDat->CountSet;							// �ݎԶ��Ă��Ȃ�
	if( p_RcptDat->teiki.ParkingNo ){									// ��������Z����or������X�V��
		p_NtDat->PassData.ParkingNo = p_RcptDat->teiki.ParkingNo;		// ��������ԏꇂ
		p_NtDat->PassData.PassID = p_RcptDat->teiki.id;					// �����ID
		p_NtDat->PassData.Syubet = p_RcptDat->teiki.syu;				// ��������
		p_NtDat->PassData.State = p_RcptDat->teiki.status;				// ������ð��
		p_NtDat->PassData.MoveMode = 0;									// ������ԋp
		p_NtDat->PassData.ReadMode = 0;									// �����ذ��ײ�
		p_NtDat->PassData.SYear =
								(uchar)(p_RcptDat->teiki.s_year%100);	// ������J�n�N
		p_NtDat->PassData.SMon = p_RcptDat->teiki.s_mon;				// ������J�n��
		p_NtDat->PassData.SDate = p_RcptDat->teiki.s_day;				// ������J�n��
		p_NtDat->PassData.EYear =
								(uchar)(p_RcptDat->teiki.e_year%100);	// ������I���N
		p_NtDat->PassData.EMon = p_RcptDat->teiki.e_mon;				// ������I����
		p_NtDat->PassData.EDate = p_RcptDat->teiki.e_day;				// ������I����
	}
	if( p_NtDat->PayMethod == 5 ){
		p_NtDat->PassRenewalPric = p_RcptDat->WPrice;					// ������X�V����

		p_NtDat->PassRenewalCondition = 1;								// ������X�V����
		if( rangechk( 1, 3, p_RcptDat->teiki.status ) ){				// �ð��1�`3�͈͓�
			p_NtDat->PassRenewalCondition = 11;							// ������X�V����
		}
		p_NtDat->PassRenewalPeriod = p_RcptDat->teiki.update_mon;		// ������X�V����(�X�V����)
	}

	if (ntnet_decision_credit(&p_RcptDat->credit)) {

	memcpy( p_NtDat->CreditCardNo,
			p_RcptDat->credit.card_no,
			sizeof( p_NtDat->CreditCardNo ) );							// �ڼޯĶ��މ����
	p_NtDat->Credit_ryo = p_RcptDat->credit.pay_ryo;					// �ڼޯĶ��ޗ��p���z
	p_NtDat->CreditSlipNo = p_RcptDat->credit.slip_no;					// �ڼޯĶ��ޓ`�[�ԍ�
	p_NtDat->CreditAppNo = p_RcptDat->credit.app_no;					// �ڼޯĶ��ޏ��F�ԍ�
	memcpy( p_NtDat->CreditName,
			p_RcptDat->credit.card_name,
			sizeof( p_NtDat->CreditName ) );							// �ڼޯĶ��މ�Ж�
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	p_NtDat->CreditDate[0] = p_RcptDat->credit.CreditDate[0];			// �ڼޯĶ��ޗL������(�N)
//	p_NtDat->CreditDate[1] = p_RcptDat->credit.CreditDate[1];			// �ڼޯĶ��ޗL������(��)
//	p_NtDat->CreditProcessNo = p_RcptDat->credit.CenterProcOiBan;		// �ڼޯĶ��޾�������ǂ���
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
	}
//	p_NtDat->PayCalMax = p_RcptDat->PayCalMax;							// �ő嗿���z������

	for( i = j = 0; i < WTIK_USEMAX; i++ ){								// ���Z���~�ȊO�̊������R�s�[
		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&				// ������ʂ���
		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||	// ���Z���~�������łȂ�
		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 			memcpy( &p_NtDat->DiscountData[j], &p_RcptDat->DiscountData[i], sizeof( DISCOUNT_DATA ) );	// �������
// 			if( p_NtDat->DiscountData[j].DiscSyu == NTNET_PRI_W ){
// 				memset( &p_NtDat->DiscountData[j].uDiscData, 0, 8 );	// �s�v�f�[�^�N���A
// 			}
// 			j++;
			if ( p_RcptDat->DiscountData[i].Discount || p_RcptDat->DiscountData[i].DiscCount ) {
				p_NtDat->DiscountData[j].ParkingNo = p_RcptDat->DiscountData[i].ParkingNo;
				p_NtDat->DiscountData[j].DiscSyu   = p_RcptDat->DiscountData[i].DiscSyu;
				p_NtDat->DiscountData[j].DiscNo    = p_RcptDat->DiscountData[i].DiscNo;
				p_NtDat->DiscountData[j].DiscCount = p_RcptDat->DiscountData[i].DiscCount;
				p_NtDat->DiscountData[j].Discount  = p_RcptDat->DiscountData[i].Discount;
				p_NtDat->DiscountData[j].DiscInfo1 = p_RcptDat->DiscountData[i].DiscInfo1;
				p_NtDat->DiscountData[j].uDiscData.common.DiscInfo2 =
					p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;
				p_NtDat->DiscountData[j].uDiscData.common.DiscFlg = 0;
				switch ( p_RcptDat->DiscountData[i].DiscSyu ) {
				case NTNET_SVS_M:			// �T�[�r�X������(���z)
				case NTNET_SVS_T:			// �T�[�r�X������(����)
				case NTNET_KAK_M:			// �X����(���z)
				case NTNET_KAK_T:			// �X����(����)
				case NTNET_TKAK_M:			// ���X�܊���(���z)
				case NTNET_TKAK_T:			// ���X�܊���(����)
				case NTNET_SYUBET:			// ��ʊ���(���z)
				case NTNET_SYUBET_TIME:		// ��ʊ���(����)
				case NTNET_SHOP_DISC_AMT:	// ��������(���z)
				case NTNET_SHOP_DISC_TIME:	// ��������(����)
				case NTNET_GENGAKU:			// ���z���Z
				case NTNET_FURIKAE:			// �U�֐��Z
				case NTNET_SYUSEI_1:		// �C�����Z
				case NTNET_SYUSEI_2:		// �C�����Z
				case NTNET_SYUSEI_3:		// �C�����Z
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 1;
					break;
				case NTNET_PRI_W:			// �����v���y�C�h�J�[�h
					memset(&p_NtDat->DiscountData[j].uDiscData, 0, 8);	// �s�v�f�[�^�N���A
					break;
				default:
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 0;
					break;
				}
				j++;
			}

			if ( p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscount ||
				 p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscCount ) {
				p_NtDat->DiscountData[j].ParkingNo = p_RcptDat->DiscountData[i].ParkingNo;
				p_NtDat->DiscountData[j].DiscSyu   = p_RcptDat->DiscountData[i].DiscSyu;
				p_NtDat->DiscountData[j].DiscNo    = p_RcptDat->DiscountData[i].DiscNo;
				p_NtDat->DiscountData[j].DiscCount = p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscCount;
				p_NtDat->DiscountData[j].Discount  = p_RcptDat->DiscountData[i].uDiscData.common.PrevUsageDiscount;
				p_NtDat->DiscountData[j].DiscInfo1 = p_RcptDat->DiscountData[i].DiscInfo1;
				p_NtDat->DiscountData[j].uDiscData.common.DiscInfo2 =
					p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;
				p_NtDat->DiscountData[j].uDiscData.common.DiscFlg = 1;
				switch ( p_RcptDat->DiscountData[i].DiscSyu ) {
				case NTNET_SVS_M:			// �T�[�r�X������(���z)
				case NTNET_SVS_T:			// �T�[�r�X������(����)
				case NTNET_KAK_M:			// �X����(���z)
				case NTNET_KAK_T:			// �X����(����)
				case NTNET_TKAK_M:			// ���X�܊���(���z)
				case NTNET_TKAK_T:			// ���X�܊���(����)
				case NTNET_SYUBET:			// ��ʊ���(���z)
				case NTNET_SYUBET_TIME:		// ��ʊ���(����)
				case NTNET_SHOP_DISC_AMT:	// ��������(���z)
				case NTNET_SHOP_DISC_TIME:	// ��������(����)
				case NTNET_GENGAKU:			// ���z���Z
				case NTNET_FURIKAE:			// �U�֐��Z
				case NTNET_SYUSEI_1:		// �C�����Z
				case NTNET_SYUSEI_2:		// �C�����Z
				case NTNET_SYUSEI_3:		// �C�����Z
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 1;
					break;
				case NTNET_PRI_W:			// �����v���y�C�h�J�[�h
					memset(&p_NtDat->DiscountData[j].uDiscData, 0, 8);	// �s�v�f�[�^�N���A
					break;
				default:
					p_NtDat->DiscountData[j].uDiscData.common.MoveMode = 0;
					break;
				}
				j++;
			}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
		}
	}
	// 22 -> 56�ɒǉ����ꂽ�ް��̐ݒ�(FmtRev, PARKCAR_DATA1)
	p_NtDat->FmtRev = 0;					// �t�H�[�}�b�gRev.��
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	p_NtDat->ParkData.FullNo1 	= p_RcptDat->FullNo1;					// ���ԑ䐔�P
//	p_NtDat->ParkData.CarCnt1 	= p_RcptDat->CarCnt1;					// ���ݑ䐔�P
//	p_NtDat->ParkData.FullNo2 	= p_RcptDat->FullNo2;					// ���ԑ䐔�Q
//	p_NtDat->ParkData.CarCnt2 	= p_RcptDat->CarCnt2;					// ���ݑ䐔�Q
//	p_NtDat->ParkData.FullNo3 	= p_RcptDat->FullNo3;					// ���ԑ䐔�R
//	p_NtDat->ParkData.CarCnt3 	= p_RcptDat->CarCnt3;					// ���ݑ䐔�R
//	p_NtDat->ParkData.PascarCnt = p_RcptDat->PascarCnt;					// ����ԗ��J�E���g
//	p_NtDat->ParkData.Full[0] 	= p_RcptDat->Full[0];					// ����1������
//	p_NtDat->ParkData.Full[1] 	= p_RcptDat->Full[1];					// ����2������
//	p_NtDat->ParkData.Full[2] 	= p_RcptDat->Full[2];					// ����3������
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 	ret = sizeof( DATA_KIND_56 ) - sizeof( DISCOUNT_DATA );				// ��������24�܂�
	ret = sizeof( DATA_KIND_56 ) - sizeof( t_SeisanDiscountOld );		// ��������24�܂�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	return ret;
}
//--------------------------------------------------------------
//	�V���ZNT-NET�t�H�[�}�b�g�ݒ�(22̫�ϯĂ�FmtRev, PARKCAR_DATA1��t��)	// �Q�� = NTNET_Edit_Data22_SK
unsigned short	NTNET_Edit_Data56_SK(	Receipt_data 	*p_RcptDat,	// ���Zlog�ް�(IN)
										DATA_KIND_56 	*p_NtDat )	// ���ZNT-NET(OUT)
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_56 ) );

	p_NtDat->DataBasic.SystemID = p_RcptDat->DataBasic.SystemID;		// ����ID
	p_NtDat->DataBasic.DataKind = p_RcptDat->DataBasic.DataKind;		// �ް����
	p_NtDat->DataBasic.DataKeep = p_RcptDat->DataBasic.DataKeep;		// �ް��ێ��׸�
	p_NtDat->DataBasic.ParkingNo = p_RcptDat->DataBasic.ParkingNo;		// ���ԏꇂ
	p_NtDat->DataBasic.ModelCode = p_RcptDat->DataBasic.ModelCode;		// �@����
	p_NtDat->DataBasic.MachineNo = p_RcptDat->DataBasic.MachineNo;		// �@�B��
	memcpy( &p_NtDat->DataBasic.Year, &p_RcptDat->DataBasic.Year, 6 );	// �����N���������b
	p_NtDat->PayCount = CountSel( &p_RcptDat->Oiban );					// ���Zor���Z���~�ǂ���
	p_NtDat->PayMethod = p_RcptDat->PayMethod;							// ���Z���@
	p_NtDat->PayClass = p_RcptDat->PayClass;							// �����敪
	p_NtDat->PayMode = 0;												// ���ZӰ��(�������Z)
	p_NtDat->LockNo = p_RcptDat->WPlace;								// �����
	p_NtDat->CardType = 0;												// ���Ԍ�����
	p_NtDat->CMachineNo = 0;											// ���Ԍ��@�B��
	p_NtDat->CardNo = 0L;												// ���Ԍ��ԍ�(�����ǂ���)
	p_NtDat->OutTime.Year = p_RcptDat->TOutTime.Year;					// �o�ɔN
	p_NtDat->OutTime.Mon = p_RcptDat->TOutTime.Mon;						// �o�Ɍ�
	p_NtDat->OutTime.Day = p_RcptDat->TOutTime.Day;						// �o�ɓ�
	p_NtDat->OutTime.Hour = p_RcptDat->TOutTime.Hour;					// �o�Ɏ�
	p_NtDat->OutTime.Min = p_RcptDat->TOutTime.Min;						// �o�ɕ�
	p_NtDat->OutTime.Sec = 0;											// �o�ɕb
	p_NtDat->KakariNo = p_RcptDat->KakariNo;							// �W����
	p_NtDat->OutKind = p_RcptDat->OutKind;								// ���Z�o��
	p_NtDat->InTime.Year = p_RcptDat->TInTime.Year;						// ���ɔN
	p_NtDat->InTime.Mon = p_RcptDat->TInTime.Mon;						// ���Ɍ�
	p_NtDat->InTime.Day = p_RcptDat->TInTime.Day;						// ���ɓ�
	p_NtDat->InTime.Hour = p_RcptDat->TInTime.Hour;						// ���Ɏ�
	p_NtDat->InTime.Min = p_RcptDat->TInTime.Min;						// ���ɕ�
	p_NtDat->InTime.Sec = 0;											// ���ɕb
	// ���Z�N���������b��0�Ƃ���
	// �O�񐸎Z�N���������b��0�Ƃ���
	p_NtDat->TaxPrice = 0;												// �ېőΏۊz
	p_NtDat->TotalPrice = 0;											// ���v���z(HOST���g�p�̂���)
	p_NtDat->Tax = p_RcptDat->Wtax;										// ����Ŋz
	p_NtDat->Syubet = p_RcptDat->syu;									// �������
	p_NtDat->Price = p_RcptDat->WPrice;									// ���ԗ���
	p_NtDat->PassCheck = 1;												// ����߽����
	p_NtDat->CountSet = p_RcptDat->CountSet;							// �ݎԶ��Ă��Ȃ�
//	p_NtDat->PayCalMax = p_RcptDat->PayCalMax;							// �ő嗿���z������

	// 22 -> 56�ɒǉ����ꂽ�ް��̐ݒ�(FmtRev, PARKCAR_DATA1)
	p_NtDat->FmtRev = 0;					// �t�H�[�}�b�gRev.��
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	p_NtDat->ParkData.FullNo1 	= p_RcptDat->FullNo1;					// ���ԑ䐔�P
//	p_NtDat->ParkData.CarCnt1 	= p_RcptDat->CarCnt1;					// ���ݑ䐔�P
//	p_NtDat->ParkData.FullNo2 	= p_RcptDat->FullNo2;					// ���ԑ䐔�Q
//	p_NtDat->ParkData.CarCnt2 	= p_RcptDat->CarCnt2;					// ���ݑ䐔�Q
//	p_NtDat->ParkData.FullNo3 	= p_RcptDat->FullNo3;					// ���ԑ䐔�R
//	p_NtDat->ParkData.CarCnt3 	= p_RcptDat->CarCnt3;					// ���ݑ䐔�R
//	p_NtDat->ParkData.PascarCnt = p_RcptDat->PascarCnt;					// ����ԗ��J�E���g
//	p_NtDat->ParkData.Full[0] 	= p_RcptDat->Full[0];					// ����1������
//	p_NtDat->ParkData.Full[1] 	= p_RcptDat->Full[1];					// ����2������
//	p_NtDat->ParkData.Full[2] 	= p_RcptDat->Full[2];					// ����3������
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 	ret = sizeof( DATA_KIND_56 ) - sizeof( DISCOUNT_DATA );				// ��������24�܂�
	ret = sizeof( DATA_KIND_56 ) - sizeof( t_SeisanDiscountOld );		// ��������24�܂�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	return ret;
}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
////	�V���ZNT-NET�t�H�[�}�b�g�ݒ�(22̫�ϯĂ�FmtRev, PARKCAR_DATA1��t��)	// �Q�� = NTNET_Edit_Data22
//unsigned short	NTNET_Edit_Data56_r10(	Receipt_data 		*p_RcptDat,		// ���Zlog�ް�(IN)
//										DATA_KIND_56_r10 	*p_NtDat )	// ���ZNT-NET(OUT)
//{
//	uchar	i;
//	uchar	j;
//	ushort	ret;
//	EMONEY	t_EMoney;
//	CREINFO	t_Credit;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_56_r10 ) );
//
//	NTNET_Edit_BasicData( 57, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// ��{�ް��쐬
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TOutTime.Year % 100);		// ���Z�N
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TOutTime.Mon;				// ���Z��
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TOutTime.Day;				// ���Z��
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TOutTime.Hour;				// ���Z��
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TOutTime.Min;				// ���Z��
//	p_NtDat->DataBasic.Sec	= 0;
//	p_NtDat->FmtRev = 10;													// �t�H�[�}�b�gRev.��
//	p_NtDat->CenterSeqNo = p_RcptDat->CenterSeqNo;							// �Z���^�[�ǔԁi���Z�j
//
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->FullNo1;						// ���ԑ䐔�P
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CarCnt1;						// ���ݑ䐔�P
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->FullNo2;						// ���ԑ䐔�Q
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CarCnt2;						// ���ݑ䐔�Q
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->FullNo3;						// ���ԑ䐔�R
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CarCnt3;						// ���ݑ䐔�R
//	p_NtDat->ParkData.CarFullFlag	= (uchar)0;								// �䐔�E�����ԃt���O
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->PascarCnt;				// ����ԗ��J�E���g
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->Full[0];						// ����1������
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->Full[1];						// ����2������
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->Full[2];						// ����3������
//
//	p_NtDat->PayCount	= CountSel( &p_RcptDat->Oiban);						// ���Zor���Z���~�ǂ���
//	p_NtDat->PayMethod	= (uchar)p_RcptDat->PayMethod;						// ���Z���@
//	if( p_RcptDat->PayClass == 4 ){
//		// �N���W�b�g���Z
//		p_NtDat->PayClass	= (uchar)0;										// �����敪
//	}else{
//		p_NtDat->PayClass	= (uchar)p_RcptDat->PayClass;					// �����敪
//	}
//	p_NtDat->PayMode	= (uchar)p_RcptDat->PayMode;						// ���ZӰ��(�������Z)
//	p_NtDat->CMachineNo	= 0;												// ���Ԍ��@�B��
//
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->FlapArea	= (ushort)(p_RcptDat->WPlace/10000);			// �t���b�v�V�X�e��	���		0�`99
//		p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->WPlace%10000);			// 					�Ԏ��ԍ�	0�`9999
//		p_NtDat->KakariNo	= p_RcptDat->KakariNo;							// �W����
//		p_NtDat->OutKind	= p_RcptDat->OutKind;							// ���Z�o��
//	}
//
//	p_NtDat->CountSet	= p_RcptDat->CountSet;								// �ݎԃJ�E���g���Ȃ�
//
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->InPrev_ymdhms.In_Year	= (uchar)(p_RcptDat->TInTime.Year%100);	// ���ɔN
//		p_NtDat->InPrev_ymdhms.In_Mon	= p_RcptDat->TInTime.Mon;				// ���Ɍ�
//		p_NtDat->InPrev_ymdhms.In_Day	= p_RcptDat->TInTime.Day;				// ���ɓ�
//		p_NtDat->InPrev_ymdhms.In_Hour	= p_RcptDat->TInTime.Hour;				// ���Ɏ�
//		p_NtDat->InPrev_ymdhms.In_Min	= p_RcptDat->TInTime.Min;				// ���ɕ�
//		p_NtDat->InPrev_ymdhms.In_Sec	= 0;									// ���ɕb
//
//		// �O�񐸎Z�N���������b��0�Ƃ���
//		p_NtDat->InPrev_ymdhms.Prev_Year	= 0;							// �O�񐸎Z�N
//		p_NtDat->InPrev_ymdhms.Prev_Mon		= 0;							// �O�񐸎Z��
//		p_NtDat->InPrev_ymdhms.Prev_Day		= 0;							// �O�񐸎Z��
//		p_NtDat->InPrev_ymdhms.Prev_Hour	= 0;							// �O�񐸎Z��
//		p_NtDat->InPrev_ymdhms.Prev_Min		= 0;							// �O�񐸎Z��
//		p_NtDat->InPrev_ymdhms.Prev_Sec		= 0;							// �O�񐸎Z�b
//	}
//
//	p_NtDat->ReceiptIssue	= p_RcptDat->ReceiptIssue;						// �̎��ؔ��s�L��
//
//	if( p_NtDat->PayMethod != 5 ){											// �X�V���Z�ȊO
//		p_NtDat->Syubet	= p_RcptDat->syu;									// �������
//		p_NtDat->Price	= p_RcptDat->WPrice;								// ���ԗ���
//	}
//
//	p_NtDat->CashPrice	= (ulong)p_RcptDat->WTotalPrice;					// ��������
//	p_NtDat->InPrice	= p_RcptDat->WInPrice;								// �������z
//	p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// �ޑK���z
//
//	p_NtDat->MoneyInOut.MoneyKind_In	= 0x1f;								// ����L��	���ڋ���̗L���@�������i10�~,50�~,100�~,500�~,1000�~�j
//	p_NtDat->MoneyInOut.In_10_cnt		= (uchar)p_RcptDat->in_coin[0];		// ����������(10�~)		0�`255
//	p_NtDat->MoneyInOut.In_50_cnt		= (uchar)p_RcptDat->in_coin[1];		// ����������(50�~)		0�`255
//	p_NtDat->MoneyInOut.In_100_cnt		= (uchar)p_RcptDat->in_coin[2];		// ����������(100�~)	0�`255
//	p_NtDat->MoneyInOut.In_500_cnt		= (uchar)p_RcptDat->in_coin[3];		// ����������(500�~)	0�`255
//	p_NtDat->MoneyInOut.In_1000_cnt		= (uchar)p_RcptDat->in_coin[4];		// ����������(1000�~)	0�`255
//	if(p_RcptDat->f_escrow) {												// �G�X�N�������߂�����
//		++p_NtDat->MoneyInOut.In_1000_cnt;									// �G�X�N�������߂����𓊓������ɉ��Z
//	}
//
//	p_NtDat->MoneyInOut.MoneyKind_Out	= 0x1f;								// ����L��	���ڋ���̗L���@���o���i10�~,50�~,100�~,500�~,1000�~�j
//	p_NtDat->MoneyInOut.Out_10_cnt		= (uchar)p_RcptDat->out_coin[0];	// ���o������(10�~)		0�`255
//	p_NtDat->MoneyInOut.Out_50_cnt		= (uchar)p_RcptDat->out_coin[1];	// ���o������(50�~)		0�`255
//	p_NtDat->MoneyInOut.Out_100_cnt		= (uchar)p_RcptDat->out_coin[2];	// ���o������(100�~)	0�`255
//	p_NtDat->MoneyInOut.Out_500_cnt		= (uchar)p_RcptDat->out_coin[3];	// ���o������(500�~)	0�`255
//	p_NtDat->MoneyInOut.Out_1000_cnt	= (uchar)p_RcptDat->f_escrow;		// ���o������(1000�~)	0�`255
//
//	p_NtDat->HaraiModoshiFusoku	= (ushort)p_RcptDat->WFusoku;				// ���ߕs���z	0�`9999
//
//	p_NtDat->PassCheck = p_RcptDat->PassCheck;								// �A���`�p�X�`�F�b�N
//
//	if( p_RcptDat->teiki.ParkingNo ) {
//		p_NtDat->ParkingNo	= p_RcptDat->teiki.ParkingNo;								// ��������ԏꇂ
//		p_NtDat->Media[0].MediaKind	= (ushort)(p_RcptDat->teiki.pkno_syu + 2);			// ���Z�}�̏��P�@���(���C���}��)	0�`99
//		intoasl(p_NtDat->Media[0].MediaCardNo, p_RcptDat->teiki.id, 5);					// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		intoasl(p_NtDat->Media[0].MediaCardInfo, p_RcptDat->teiki.syu, 2);				// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		// ���Z�}�̏��2�i�T�u�}�́j�� �I�[���O�Ƃ���
//	}
//
//	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 ){	// �d�q���ώ�� Suica:1, Edy:2
//		p_NtDat->CardKind = (ushort)2;											// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//	}else if( p_RcptDat->PayClass == 4 ){					// �N���W�b�g���Z
//		p_NtDat->CardKind = (ushort)1;											// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//	}
//
//	switch( p_NtDat->CardKind ){							// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//	case 1:
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// �Í�������
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// �Í����ԍ�
//		memset( &t_Credit, 0, sizeof( t_Credit ) );
//		t_Credit.amount = (ulong)p_RcptDat->credit.pay_ryo;									// ���ϊz
//		memcpyFlushLeft( &t_Credit.card_no[0], (uchar *)&p_RcptDat->credit.card_no[0], sizeof(t_Credit.card_no), sizeof(p_RcptDat->credit.card_no) );	// ����ԍ�
//		memcpyFlushLeft( &t_Credit.cct_num[0], (uchar *)&p_RcptDat->credit.CCT_Num[0],
//							sizeof(t_Credit.cct_num), CREDIT_TERM_ID_NO_SIZE );				// �[�����ʔԍ�
//		memcpyFlushLeft( &t_Credit.kid_code[0], &p_RcptDat->credit.kid_code[0], sizeof(t_Credit.kid_code), sizeof(p_RcptDat->credit.kid_code) );		// KID �R�[�h
//		t_Credit.app_no = p_RcptDat->credit.app_no;											// ���F�ԍ�
//		t_Credit.center_oiban = 0;															// ���������ǔ�(�Z���^�[��0�Œ�)
//		
//		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );							// AES �Í���
//		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// ���Ϗ��
//		break;
//	case 2:
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// �Í�������
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// �Í����ԍ�
//		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//		t_EMoney.amount = p_RcptDat->Electron_data.Suica.pay_ryo;							// �d�q���ϐ��Z���@���ϊz
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//		memcpy( &t_EMoney.card_id[0], &p_RcptDat->Electron_data.Suica.Card_ID[0],
//				sizeof(p_RcptDat->Electron_data.Suica.Card_ID) );							// �d�q���ϐ��Z���@����ID (Ascii 16��)
//		t_EMoney.card_zangaku = p_RcptDat->Electron_data.Suica.pay_after;					// �d�q���ϐ��Z���@���ό�c��
//		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );							// AES �Í���
//		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// ���Ϗ��
//		break;
//	}
//
//	p_NtDat->MoneyIn		= (ulong)0;										// �^�C�����W���F�����������z
//	p_NtDat->MoneyOut		= (ulong)0;										// �^�C�����W���F�o���������z
//	p_NtDat->MoneyBack		= (ulong)0;										// �^�C�����W���F���ߋ��z
//	p_NtDat->MoneyFusoku	= (ulong)0;										// �^�C�����W���F����s�����z
//
//	for( i = j = 0; i < WTIK_USEMAX; i++ ){									// ���Z���~�ȊO�̊������R�s�[
//		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&					// ������ʂ���
//		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// ���Z���~�������łȂ�
//		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
//		    if( p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA�ȊO(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(�x���z�E�c�z)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO�ȊO(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(�x���z�E�c�z)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY�ȊO	(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(�x���z�E�c�z)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA�ȊO(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(�x���z�E�c�z)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card�ȊO(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_2 ) 		//			(�x���z�E�c�z)
//		    {
//				p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DiscountData[i].ParkingNo;						// ���ԏ�No.
//				p_NtDat->SDiscount[j].Kind		= p_RcptDat->DiscountData[i].DiscSyu;						// �������
//				p_NtDat->SDiscount[j].Group		= p_RcptDat->DiscountData[i].DiscNo;						// �����敪
//				p_NtDat->SDiscount[j].Callback	= p_RcptDat->DiscountData[i].DiscCount;						// �������
//				p_NtDat->SDiscount[j].Amount	= p_RcptDat->DiscountData[i].Discount;						// �����z
//				p_NtDat->SDiscount[j].Info1		= p_RcptDat->DiscountData[i].DiscInfo1;						// �������1
//				if( p_NtDat->SDiscount[j].Kind != NTNET_PRI_W ){
//					p_NtDat->SDiscount[j].Info2	= p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;	// �������2
//				}
//				j++;
//		    }
//		}
//	}
//
//	ret = sizeof( DATA_KIND_56_r10 );
//
//	return ret;
//}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

//--------------------------------------------------------------
//	WEB�p���ԑ䐔�pNT-NET�t�H�[�}�b�g�ݒ�(236̫�ϯĂƂ͕ʂ�58̫�ϯĂ��쐬)	// �Q�� = NTNET_Edit_Data236
unsigned short	NTNET_Edit_Data58(	ParkCar_log		*p_RcptDat,	// ���ԑ䐔�plog�ް�(IN)
									DATA_KIND_58 	*p_NtDat )	// ���ԑ䐔�pNT-NET(OUT)
{
	unsigned short ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_58 ) );
	NTNET_Edit_BasicData( 58, 1, p_RcptDat->CMN_DT.DT_58.SeqNo, &p_NtDat->DataBasic);	// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Time.Year % 100);					// �����N(���O�쐬���ɉ�2���ɂ��Ă���)
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Time.Mon;							// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Time.Day;							// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Time.Hour;							// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Time.Min;							// ������
	p_NtDat->DataBasic.Sec = (uchar)p_RcptDat->Time.Sec;							// �����b

	// 236 -> 58(��̫�ϯ�)���ް��̐ݒ�(FmtRev, PARKCAR_DATA2)
	p_NtDat->FmtRev = 0;					// �t�H�[�}�b�gRev.��
	p_NtDat->ParkData.GroupNo	= p_RcptDat->GroupNo;	// ��ٰ�ߔԍ�
	p_NtDat->ParkData.State		= p_RcptDat->State;	// �p�r�ʒ��ԑ䐔�ݒ�
	p_NtDat->ParkData.Full[0]	= p_RcptDat->Full[0];	// ����1������
	p_NtDat->ParkData.Full[1]	= p_RcptDat->Full[1];	// ����2������
	p_NtDat->ParkData.Full[2]	= p_RcptDat->Full[2];	// ����3������
	p_NtDat->ParkData.EmptyNo1	= p_RcptDat->CMN_DT.DT_58.EmptyNo1;	// ��ԑ䐔�P
	p_NtDat->ParkData.FullNo1	= p_RcptDat->CMN_DT.DT_58.FullNo1;	// ���ԑ䐔�P
	p_NtDat->ParkData.EmptyNo2	= p_RcptDat->CMN_DT.DT_58.EmptyNo2;	// ��ԑ䐔�Q
	p_NtDat->ParkData.FullNo2	= p_RcptDat->CMN_DT.DT_58.FullNo2;	// ���ԑ䐔�Q
	p_NtDat->ParkData.EmptyNo3	= p_RcptDat->CMN_DT.DT_58.EmptyNo3;	// ��ԑ䐔�R
	p_NtDat->ParkData.FullNo3	= p_RcptDat->CMN_DT.DT_58.FullNo3;	// ���ԑ䐔�R

	ret = sizeof( DATA_KIND_58 ); 
	return ret;
}
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
////[]----------------------------------------------------------------------[]
/////	@brief			�W�v��{�f�[�^�ҏW����
////[]----------------------------------------------------------------------[]
/////	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
/////	@param[in]		Type	: �W�v�^�C�v
/////	@param[out]		p_NtDat	: �W�v�I���ʒm�f�[�^(DATA_KIND_42�^)�ւ̃|�C���^  
/////	@return			ret		: �W�v�I���ʒm�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
////[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
//unsigned short	NTNET_Edit_SyukeiKihon_r10( SYUKEI *syukei, ushort Type, DATA_KIND_42 *p_NtDat )
//{
//	static const ushort c_coin[COIN_SYU_CNT] = {10, 50, 100, 500};
//	ushort	i;
//	ushort	ret;
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_42 ) );
//	NTNET_Edit_BasicData( 42, 0, syukei->SeqNo[0], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0
//
//	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
//	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
//	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
//	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
//	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
//	p_NtDat->DataBasic.Sec = 0;													// �����b
//
//	p_NtDat->FmtRev					= 10;										// �t�H�[�}�b�gRev.No.
//	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// �W�v�^�C�v 1:T, 2:GT
//	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
//	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// �W�v�ǔ�
//	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
//	case	0:		// �ʒǔ�
//		if (Type == 11) {														// GT�W�v
//			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// �J�n�ǔ�
//			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// �I���ǔ�
//		}
//		break;
//	case	1:		// �ʂ��ǂ���
//		if (Type == 1 || Type == 11) {
//			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// �J�n�ǔ�
//			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// �I���ǔ�
//		}
//		break;
//	default:
//		break;
//	}
//	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// �Z���^�[�ǔԁi�W�v�j
//	p_NtDat->LastTime.Year = (uchar)(syukei->OldTime.Year % 100 );				// �����N
//	p_NtDat->LastTime.Mon = (uchar)syukei->OldTime.Mon;							// ������
//	p_NtDat->LastTime.Day = (uchar)syukei->OldTime.Day;							// ������
//	p_NtDat->LastTime.Hour = (uchar)syukei->OldTime.Hour;						// ������
//	p_NtDat->LastTime.Min = (uchar)syukei->OldTime.Min;							// ������
//	p_NtDat->LastTime.Sec = 0;													// �����b
//	p_NtDat->SettleNum							= syukei->Seisan_Tcnt;			// �����Z��
//	p_NtDat->Kakeuri							= syukei->Kakeuri_Tryo;			// ���|���z
//	p_NtDat->Cash								= syukei->Genuri_Tryo;			// ����������z
//	p_NtDat->Uriage								= syukei->Uri_Tryo;				// ������z
//	p_NtDat->Tax								= syukei->Tax_Tryo;				// ������Ŋz
//	p_NtDat->Charge								= syukei->Turi_modosi_ryo;		// �ޑK���ߊz
//	p_NtDat->CoinTotalNum						= (ushort)syukei->Ckinko_goukei_cnt;	// �R�C�����ɍ��v��
//	p_NtDat->NoteTotalNum						= (ushort)syukei->Skinko_goukei_cnt;	// �������ɍ��v��
//	p_NtDat->CyclicCoinTotalNum					= (ushort)syukei->Junkan_goukei_cnt;	// �z�R�C�����v��
//	p_NtDat->NoteOutTotalNum					= (ushort)syukei->Siheih_goukei_cnt;	// �������o�@���v��
//	p_NtDat->SettleNumServiceTime				= syukei->In_svst_seisan;		// �T�[�r�X�^�C�������Z��
//	p_NtDat->Shortage.Num						= syukei->Harai_husoku_cnt;		// ���o�s����
//	p_NtDat->Shortage.Amount					= syukei->Harai_husoku_ryo;		// ���o�s�����z
//	p_NtDat->Cancel.Num							= syukei->Seisan_chusi_cnt;		// ���Z���~��
//	p_NtDat->Cancel.Amount						= syukei->Seisan_chusi_ryo;		// ���Z���~���z
//	p_NtDat->AntiPassOffSettle					= syukei->Apass_off_seisan;		// �A���`�p�XOFF���Z��
//	p_NtDat->ReceiptIssue						= syukei->Ryosyuu_pri_cnt;		// �̎��ؔ��s����
//	p_NtDat->WarrantIssue						= syukei->Azukari_pri_cnt;		// �a��ؔ��s����
//// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
////	p_NtDat->AllSystem.CarOutIllegal.Num		= syukei->Husei_out_Tcnt;		// �S���u  �s���o�ɉ�
////	p_NtDat->AllSystem.CarOutIllegal.Amount		= syukei->Husei_out_Tryo;		//                 ���z
////	p_NtDat->AllSystem.CarOutForce.Num			= syukei->Kyousei_out_Tcnt;		//         �����o�ɉ�
////	p_NtDat->AllSystem.CarOutForce.Amount		= syukei->Kyousei_out_Tryo;		//                 ���z
////	p_NtDat->AllSystem.AcceptTicket				= syukei->Uketuke_pri_Tcnt;		//         ��t�����s��
////	p_NtDat->AllSystem.ModifySettle.Num			= syukei->Syuusei_seisan_Tcnt;	// �C�����Z��
////	p_NtDat->AllSystem.ModifySettle.Amount		= syukei->Syuusei_seisan_Tryo;	//         ���z
////	p_NtDat->CarInTotal							= syukei->In_car_Tcnt;			// �����ɑ䐔
////	p_NtDat->CarOutTotal						= syukei->Out_car_Tcnt;			// ���o�ɑ䐔
////	p_NtDat->CarIn1								= syukei->In_car_cnt[0];		// ����1���ɑ䐔
////	p_NtDat->CarOut1							= syukei->Out_car_cnt[0];		// �o��1�o�ɑ䐔
////	p_NtDat->CarIn2								= syukei->In_car_cnt[1];		// ����2���ɑ䐔
////	p_NtDat->CarOut2							= syukei->Out_car_cnt[1];		// �o��2�o�ɑ䐔
////	p_NtDat->CarIn3								= syukei->In_car_cnt[2];		// ����3���ɑ䐔
////	p_NtDat->CarOut3							= syukei->Out_car_cnt[2];		// �o��3�o�ɑ䐔
//// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// �����������ł̖������͊�{�ް��ɍ��ڂ�V�݂��đ��M����
//	p_NtDat->MiyoCount							= syukei->Syuusei_seisan_Mcnt;	// ��������
//	p_NtDat->MiroMoney							= syukei->Syuusei_seisan_Mryo;	// �������z
//	p_NtDat->LagExtensionCnt					= syukei->Lag_extension_cnt;	// ���O�^�C��������
//	// ���K�֘A���
//	p_NtDat->Total					= syukei->Kinko_Tryo;						// ���ɑ������z
//	p_NtDat->NoteTotal				= syukei->Note_Tryo;						// �������ɑ������z
//	p_NtDat->CoinTotal				= syukei->Coin_Tryo;						// �R�C�����ɑ������z
//	for (i = 0; i < COIN_SYU_CNT; i++) {										// �R�C��1�`4
//		p_NtDat->Coin[i].Kind		= c_coin[i];								//        ����
//		p_NtDat->Coin[i].Num		= (ushort)syukei->Coin_cnt[i];				//        ����
//	}
//	p_NtDat->Note[0].Kind			= 1000;										// ����1  ����
//	p_NtDat->Note[0].Num2			= (ushort)syukei->Note_cnt[0];				//        ��������
//
//	ret = sizeof( DATA_KIND_42 ); 
//	return ret;
//}
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//[]----------------------------------------------------------------------[]
///	@brief			������ʖ��W�v�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: ������ʖ��W�v�f�[�^(DATA_KIND_43�^)�ւ̃|�C���^  
///	@return			ret		: ������ʖ��W�v�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//unsigned short	NTNET_Edit_SyukeiRyokinMai_r10( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat )
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//unsigned short	NTNET_Edit_SyukeiRyokinMai_r13( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiRyokinMai_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
{
	ushort	i;
	ushort	j;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_43 ) );
	NTNET_Edit_BasicData( 43, 0, syukei->SeqNo[1], &p_NtDat->DataBasic);		// �s���v�F�f�[�^�ێ��t���O = 0
	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//	p_NtDat->FmtRev					= 10;										// �t�H�[�}�b�gRev.No.
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//	p_NtDat->FmtRev					= 13;										// �t�H�[�}�b�gRev.No.
	p_NtDat->FmtRev					= 15;										// �t�H�[�}�b�gRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// �W�v�^�C�v 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// �W�v�ǔ�
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// �ʒǔ�
		if (Type == 11) {														// GT�W�v
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// �I���ǔ�
		}
		break;
	case	1:		// �ʂ��ǂ���
		if (Type == 1 || Type == 11) {
		// T���v�^���v
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// �I���ǔ�
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// �Z���^�[�ǔԁi�W�v�j
	for (i = 0, j = 0; i < RYOUKIN_SYU_CNT; i++) {								// ���01�`50
// GG124100(S) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
// 		if( prm_get( COM_PRM, S_SHA, (short)(i*6+1), 2 , 5 ) == 1 ) {			// ������ʂ̐ݒ肠��
		if ( (CLOUD_CALC_MODE &&												// �N���E�h�����v�Z���[�h
			  (syukei->Rsei_cnt[i] ||											//  ���Z�񐔃f�[�^����
			   syukei->Rsei_ryo[i] ||											//  ����z�f�[�^����
			   syukei->Rtwari_cnt[i] ||											//  �����񐔃f�[�^����
			   syukei->Rtwari_ryo[i])) ||										//  �����z�f�[�^����
			 (!CLOUD_CALC_MODE &&												// �ʏ헿���v�Z���[�h
			  (prm_get(COM_PRM, S_SHA, (short)(i * 6 + 1), 2 , 5) == 1)) ) {	//  �ݒ肠��
// GG124100(E) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
			// �ݒ肳��Ă��闿����ʂ́A�g�p����Ă��Ȃ��ꍇ�ł��W�v���ڂ��Z�b�g����
			p_NtDat->Kind[j].Kind	= i + 1;									// �������
			p_NtDat->Kind[j].Settle.Num			= syukei->Rsei_cnt[i];			// ���Z��
			p_NtDat->Kind[j].Settle.Amount		= syukei->Rsei_ryo[i];			// ����z
			p_NtDat->Kind[j].Discount.Num		= syukei->Rtwari_cnt[i];		// ������
			p_NtDat->Kind[j].Discount.Amount	= syukei->Rtwari_ryo[i];		// �����z
			j++;
		}
	}

	ret = sizeof( DATA_KIND_43 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			�����W�v�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: �����W�v�f�[�^(DATA_KIND_45�^)�ւ̃|�C���^  
///	@return			ret		: �����W�v�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			m.nagashima
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//unsigned short	NTNET_Edit_SyukeiWaribiki_r10( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat )
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//unsigned short	NTNET_Edit_SyukeiWaribiki_r13( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiWaribiki_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
{
	int		i;
	int		parking;
	int		group;
// GG124100(S) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
// 	uchar	set;
// GG124100(E) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
	ushort	pos;
	ushort	ret;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//// �d�l�ύX(S) K.Onodera 2016/11/02 �����W�v�f�[�^�t�H�[�}�b�g�Ή�
//	ushort	mod;
//// �d�l�ύX(E) K.Onodera 2016/11/02 �����W�v�f�[�^�t�H�[�}�b�g�Ή�
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	ulong	sei_cnt, sei_ryo;		// �W�v�p
	int	j;
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
	ushort	brand_no;
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����W�v�f�[�^�ҏW����)
	BOOL	wk_flag = FALSE;
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����W�v�f�[�^�ҏW����)

	wk_media_Type = 0;
	
	memset( p_NtDat, 0, sizeof( DATA_KIND_45 ) );
	NTNET_Edit_BasicData( 45, 0, syukei->SeqNo[2], &p_NtDat->DataBasic);		// �s���v�F�f�[�^�ێ��t���O = 0

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//	p_NtDat->FmtRev					= 10;										// �t�H�[�}�b�gRev.No.
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//	p_NtDat->FmtRev					= 13;										// �t�H�[�}�b�gRev.No.
	p_NtDat->FmtRev					= 15;										// �t�H�[�}�b�gRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// �W�v�^�C�v 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// �W�v�ǔ�
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// �ʒǔ�
		if (Type == 11) {														// GT�W�v
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// �I���ǔ�
		}
		break;
	case	1:		// �ʂ��ǂ���
		if (Type == 1 || Type == 11) {
		// T���v�^���v
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// �I���ǔ�
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// �Z���^�[�ǔԁi�W�v�j
	i = 0;

// �N���W�b�g
// MH321801(S) D.Inaba IC�N���W�b�g�Ή�
//// MH321800(S) hosoda IC�N���W�b�g�Ή�
////	if( CREDIT_ENABLED() ){
//	if( isEcBrandNoEnabledForSetting(EC_CHECK_CREDIT) ){
//// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//	if( isEcBrandNoEnabledForSetting(BRANDNO_CREDIT) ){
	if( isEcBrandNoEnabledForRecvTbl(BRANDNO_CREDIT) ||
		syukei->Ccrd_sei_cnt != 0 || syukei->Ccrd_sei_ryo != 0){
		// �u�����h�e�[�u�����������Č�����Ȃ��Ă��W�v����Ă����
		// �����W�v�f�[�^�ɃZ�b�g����
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
// MH321801(E) D.Inaba IC�N���W�b�g�Ή�

		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ��{���ԏ�ԍ����

		p_NtDat->Discount[i].Kind		= 30;
		p_NtDat->Discount[i].Num	= syukei->Ccrd_sei_cnt;
		p_NtDat->Discount[i].Amount	= syukei->Ccrd_sei_ryo;
		i++;
	}

// �T�[�r�X��
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if(CPrmSS[S_SYS][1+parking]) {											// ���ԏ�ݒ肠��
			if(parking == 0) {													// ��{���ԏ�
				pos = 6;
			}
			else if(parking == 1) {												// �g��1���ԏ�
				pos = 1;
			}
			else if(parking == 2) {												// �g��3���ԏ�
				pos = 2;
			}
			else if(parking == 3) {												// �g��3���ԏ�
				pos = 3;
			}
			if(prm_get(COM_PRM, S_SYS, 71, 1, pos)) {							// �Ώۂ̒��ԏ�Ŏg�p��
				for (group = 0; group < SERVICE_SYU_CNT; group++) {
// GG124100(S) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
// 					set = (uchar)prm_get(COM_PRM, S_SER, 1 + 3 * group, 1, 1);
// 					if (set >= 1 && set <= 4) {										// ��ʎg�p�ݒ肠��
					if ( (CLOUD_CALC_MODE &&												// �N���E�h�����v�Z���[�h
						  (syukei->Stik_use_cnt[parking][group] ||							//  �����f�[�^����
						   syukei->Stik_use_ryo[parking][group])) ||						//  ���z�f�[�^����
						 (!CLOUD_CALC_MODE &&												// �ʏ헿���v�Z���[�h
						  (prm_get(COM_PRM, S_SER, (short)(1 + 3 * group), 1, 1) >= 1) &&	//  �ݒ肠��
						  (prm_get(COM_PRM, S_SER, (short)(1 + 3 * group), 1, 1) <= 4)) ) {		
// GG124100(E) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
						// �ݒ肪����Ύg�p�̗L���ɍS�炸�ݒ肷��
						p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
						p_NtDat->Discount[i].Kind		= 1;
						p_NtDat->Discount[i].Group		= group + 1;
						p_NtDat->Discount[i].Num		= syukei->Stik_use_cnt[parking][group];
						p_NtDat->Discount[i].Amount		= syukei->Stik_use_ryo[parking][group];
						i++;
					}
				}
			}
		}
	}

// �v���y�C�h
	if(1 == prm_get(COM_PRM, S_PRP, 1, 1, 1)) {									// �v���y�C�h�ݒ肠��
		for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
			if (CPrmSS[S_SYS][1+parking]) {
				if(parking == 0) {												// ��{���ԏ�
					pos = 6;
				}
				else if(parking == 1) {											// �g��1���ԏ�
					pos = 1;
				}
				else if(parking == 2) {											// �g��3���ԏ�
					pos = 2;
				}
				else if(parking == 3) {											// �g��3���ԏ�
					pos = 3;
				}
				if(prm_get(COM_PRM, S_SYS, 72, 1, pos)) {						// �Ώۂ̒��ԏ�Ŏg�p��
					// �ݒ肪����Ύg�p�̗L���ɍS�炸�ݒ肷��
					p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
					p_NtDat->Discount[i].Kind		= 11;
					p_NtDat->Discount[i].Num		= syukei->Pcrd_use_cnt[parking];
					p_NtDat->Discount[i].Amount		= syukei->Pcrd_use_ryo[parking];
					i++;
				}
			}
		}
	}

// �񐔌�
	if(2 == prm_get(COM_PRM, S_PRP, 1, 1, 1)) {									// �񐔌��ݒ肠��
		for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
			if (CPrmSS[S_SYS][1+parking]) {
				if(parking == 0) {												// ��{���ԏ�
					pos = 6;
				}
				else if(parking == 1) {											// �g��1���ԏ�
					pos = 1;
				}
				else if(parking == 2) {											// �g��3���ԏ�
					pos = 2;
				}
				else if(parking == 3) {											// �g��3���ԏ�
					pos = 3;
				}
				if(prm_get(COM_PRM, S_SYS, 72, 1, pos)) {						// �Ώۂ̒��ԏ�Ŏg�p��
					// �ݒ肪����Ύg�p�̗L���ɍS�炸�ݒ肷��
					p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1+parking];
					p_NtDat->Discount[i].Kind		= 5;
					p_NtDat->Discount[i].Num		= syukei->Ktik_use_cnt[parking];
					p_NtDat->Discount[i].Amount		= syukei->Ktik_use_ryo[parking];
					i++;
				}
			}
		}
	}

	// �X�����͑S�����̂ݐݒ肷��
// GG124100(S) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
	if ( CLOUD_CALC_MODE &&				// �N���E�h�����v�Z���[�h
		 (syukei->Mno_use_Tcnt ||		//  �񐔃f�[�^����
		  syukei->Mno_use_Tryo) ) {		//  ���z�f�[�^����
		wk_flag = TRUE;
	} else if ( !CLOUD_CALC_MODE ) {	// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
	for(group = 0; group < MISE_NO_CNT; ++group) {								// �X�����ݒ肪���邩�`�F�b�N
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//		if((CPrmSS[S_STO][1 + 3 * group]) || (CPrmSS[S_STO][3 + 3 * group])){	// �X�����ݒ�(����or��ʐ؊�)����
		if((prm_get( COM_PRM, S_STO, (short)(1 + 3 * group), 1, 1 )) ||
		   (prm_get( COM_PRM, S_STO, (short)(3 + 3 * group), 2, 1 ))){			// �X�����ݒ�(����or��ʐ؊�)����
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
			break;
		}
	}
// MH322914(S) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
//	if(group != MISE_NO_CNT) {													// �X��������
	// ParkingWeb�o�R��ParkiPro�Ή�����H
	if( _is_ParkingWeb_pip() || (group != MISE_NO_CNT) ){
// MH322914(E) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����W�v�f�[�^�ҏW����)
		wk_flag = TRUE;
	}
	if ( prm_get(COM_PRM, S_TAT, 1, 1, 1) == 1 ) {								// ���X�܊�������
		for ( group = 0; group < T_MISE_SYU_CNT; ++group ) {
			if ( prm_get(COM_PRM, S_TAT, 2 + (group * 2), 1, 1) != 0 ) {
				wk_flag = TRUE;
				break;
			}
		}
		for ( group = 0; group < (T_MISE_NO_CNT - MISE_NO_CNT); ++group ) {
			if ( prm_get(COM_PRM, S_TAT, 32 + group, 2, 1) != 0 ) {
				wk_flag = TRUE;
				break;
			}
		}
	}
// GG124100(S) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
	}
// GG124100(E) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
	if ( wk_flag ) {
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����W�v�f�[�^�ҏW����)
		p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];						// ��{���ԏ�ԍ����
		p_NtDat->Discount[i].Kind		= 63;
		p_NtDat->Discount[i].Num		= syukei->Mno_use_Tcnt;
		p_NtDat->Discount[i].Amount		= syukei->Mno_use_Tryo;
		i++;
	}

// GG124100(S) R.Endo 2022/10/07 �Ԕԃ`�P�b�g���X3.0 #6641 NT-NET�����W�v�f�[�^�Ɏg�p���������������̓��e���Z�b�g����Ȃ� [���ʉ��P���� No1535]
// ��������
	for ( group = 0; group < (KAIMONO_WARISYU_MAX + 1); group++ ) {
		if ( (CLOUD_CALC_MODE &&												// �N���E�h�����v�Z���[�h
			  (syukei->Kaimono_use_cnt[group] ||								//  �񐔃f�[�^����
			   syukei->Kaimono_use_ryo[group])) ||								//  ���z�f�[�^����
			 (!CLOUD_CALC_MODE) ) {												// �ʏ헿���v�Z���[�h
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];
			p_NtDat->Discount[i].Kind		= 6;
			p_NtDat->Discount[i].Group		= 0;
			p_NtDat->Discount[i].Num		= syukei->Kaimono_use_cnt[group];
			p_NtDat->Discount[i].Amount		= syukei->Kaimono_use_ryo[group];
			p_NtDat->Discount[i].Info		= group;
			i++;
		}
	}
// GG124100(E) R.Endo 2022/10/07 �Ԕԃ`�P�b�g���X3.0 #6641 NT-NET�����W�v�f�[�^�Ɏg�p���������������̓��e���Z�b�g����Ȃ� [���ʉ��P���� No1535]

// ��ʊ���
	for (group = 0; group < RYOUKIN_SYU_CNT; group++) {
// GG124100(S) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
// 		if (1 == prm_get(COM_PRM, S_SHA, 1 + 6 * group, 2, 5) && 				// ��ʐݒ肠��
// 			0 != prm_get(COM_PRM, S_SHA, 1 + 6 * group, 4, 1)) { 				// �Ԏ튄������
			if ( (CLOUD_CALC_MODE &&												// �N���E�h�����v�Z���[�h
				  (syukei->Rtwari_cnt[group] ||										//  �񐔃f�[�^����
				   syukei->Rtwari_ryo[group])) ||									//  ���z�f�[�^����
				 (!CLOUD_CALC_MODE &&												// �ʏ헿���v�Z���[�h
				  (prm_get(COM_PRM, S_SHA, (short)(1 + 6 * group), 2, 5) == 1) &&	//  �ݒ肠��
				  (prm_get(COM_PRM, S_SHA, (short)(1 + 6 * group), 4, 1) != 0)) ) {
// GG124100(E) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
			p_NtDat->Discount[i].ParkingNo	= CPrmSS[S_SYS][1];
			p_NtDat->Discount[i].Kind		= 50;
			p_NtDat->Discount[i].Group		= group + 1;
			p_NtDat->Discount[i].Num		= syukei->Rtwari_cnt[group];
			p_NtDat->Discount[i].Amount		= syukei->Rtwari_ryo[group];
			i++;
		}
	}

	if(prm_get(COM_PRM, S_PAY,24, 1, 3) == 1){
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
		p_NtDat->Discount[i].Kind = NTNET_SUICA_1;								// ������ʁF31�iSuica���ρj�Œ�
		p_NtDat->Discount[i].Group = 0;											// �����敪�F���g�p(0)
		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
// Suica
		if( syukei->Electron_sei_cnt ){
			p_NtDat->Discount[i].Num = syukei->Electron_sei_cnt;				// �����񐔁F
			p_NtDat->Discount[i].Amount = syukei->Electron_sei_ryo;				// �����z  �F
		}

		if( syukei->Electron_psm_cnt ){
// PASMO
			p_NtDat->Discount[i].Num += syukei->Electron_psm_cnt;				// �����񐔁F
			p_NtDat->Discount[i].Amount += syukei->Electron_psm_ryo;			// �����z  �F
		}

		if( syukei->Electron_ico_cnt ){
// ICOCA
			p_NtDat->Discount[i].Num += syukei->Electron_ico_cnt;				// �����񐔁F
			p_NtDat->Discount[i].Amount += syukei->Electron_ico_ryo;			// �����z  �F
		}

		i++;
	}
	
// MH321800(S) hosoda IC�N���W�b�g�Ή�, �]����Edy�������폜
//-	if( syukei->Electron_edy_cnt ){
//-// Edy
//-		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
//-		p_NtDat->Discount[i].Kind = NTNET_EDY_0;								// ������ʁF32�iEdy���ρj�Œ�
//-		p_NtDat->Discount[i].Group = 0;											// �����敪�F(0)���������
//-		p_NtDat->Discount[i].Num = syukei->Electron_edy_cnt;					// �����񐔁F
//-		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
//-		p_NtDat->Discount[i].Amount = syukei->Electron_edy_ryo;					// �����z  �F
//-		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
//-		i++;
//-	}
//-
//-	if( syukei->Electron_Arm_cnt ){
//-		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
//-		p_NtDat->Discount[i].Kind = NTNET_EDY_0;								// ������ʁF32�iEdy���ρj�Œ�
//-		p_NtDat->Discount[i].Group = 1;											// �����敪�F(1)�װю����
//-		p_NtDat->Discount[i].Num = syukei->Electron_Arm_cnt;					// �����񐔁F
//-		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
//-		p_NtDat->Discount[i].Amount = syukei->Electron_Arm_ryo;					// �����z  �F
//-		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
//-		i++;
//-	}
// MH321800(E) hosoda IC�N���W�b�g�Ή�, �]����Edy�������폜

	if( syukei->Electron_icd_cnt ){

// IC-Card
		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
		p_NtDat->Discount[i].Kind = NTNET_ICCARD_0;								// ������ʁF36�iIC-Card���ρj�Œ�
		p_NtDat->Discount[i].Group = 0;											// �����敪�F(0)���������
		p_NtDat->Discount[i].Num = syukei->Electron_icd_cnt;					// �����񐔁F
		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
		p_NtDat->Discount[i].Amount = syukei->Electron_icd_ryo;					// �����z  �F
		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
		i++;
	}

// MH321800(S) hosoda IC�N���W�b�g�Ή�
	// ���W�v�f�[�^�ɃA���[��������̓Z�b�g���Ȃ�
	for (j = 0; j < TBL_CNT(ec_discount_kind_tbl); j++) {
		switch (ec_discount_kind_tbl[j]) {
		case	NTNET_EDY_0:
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			brand_no = BRANDNO_EDY;
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			sei_cnt = syukei->Electron_edy_cnt;
			sei_ryo = syukei->Electron_edy_ryo;
			break;
		case	NTNET_NANACO_0:
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			brand_no = BRANDNO_NANACO;
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			sei_cnt = syukei->nanaco_sei_cnt;
			sei_ryo = syukei->nanaco_sei_ryo;
			break;
		case	NTNET_WAON_0:
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			brand_no = BRANDNO_WAON;
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			sei_cnt = syukei->waon_sei_cnt;
			sei_ryo = syukei->waon_sei_ryo;
			break;
		case	NTNET_SAPICA_0:
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			brand_no = BRANDNO_SAPICA;
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			sei_cnt = syukei->sapica_sei_cnt;
			sei_ryo = syukei->sapica_sei_ryo;
			break;
		case	NTNET_SUICA_1:
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			brand_no = BRANDNO_KOUTSUU;
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			sei_cnt = syukei->koutsuu_sei_cnt;
			sei_ryo = syukei->koutsuu_sei_ryo;
			break;
		case	NTNET_ID_0:
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			brand_no = BRANDNO_ID;
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			sei_cnt = syukei->id_sei_cnt;
			sei_ryo = syukei->id_sei_ryo;
			break;
		case	NTNET_QUICPAY_0:
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			brand_no = BRANDNO_QUIC_PAY;
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			sei_cnt = syukei->quicpay_sei_cnt;
			sei_ryo = syukei->quicpay_sei_ryo;
			break;
// MH810105(S) MH364301 PiTaPa�Ή�
		case	NTNET_PITAPA_0:
			brand_no = BRANDNO_PITAPA;
			sei_cnt = syukei->pitapa_sei_cnt;
			sei_ryo = syukei->pitapa_sei_ryo;
			break;
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case	NTNET_QR:
			brand_no = BRANDNO_QR;
			sei_cnt = syukei->qr_sei_cnt;
			sei_ryo = syukei->qr_sei_ryo;
			break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		default	:
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			brand_no = 0;
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
			sei_cnt = 0L;
			sei_ryo = 0L;
			break;
		}
// MH810103 GG119202(S) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
//		if (sei_cnt != 0L) {
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//		if( isEcBrandNoEnabledForSetting(brand_no) ){
		if( isEcBrandNoEnabledForRecvTbl(brand_no) ||
			sei_cnt != 0 || sei_ryo != 0) {
		// �u�����h�e�[�u�����������Č�����Ȃ��Ă��W�v����Ă����
		// �����W�v�f�[�^�ɃZ�b�g����
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
// MH810103 GG119202(E) �g�p����Ă��Ȃ��ꍇ�ł������W�v�f�[�^�ɃZ�b�g����
		// ���ϏW�v
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
			p_NtDat->Discount[i].Kind = ec_discount_kind_tbl[j];				// ������ʁFEdy/nanaco/WAON/SAPICA�Ȃ�
			p_NtDat->Discount[i].Group = 0;										// �����敪�F(0)���������
			p_NtDat->Discount[i].Num = sei_cnt;									// �����񐔁F
			p_NtDat->Discount[i].Callback = 0;									// ��������F���g�p(0)
			p_NtDat->Discount[i].Amount = sei_ryo;								// �����z  �F
			p_NtDat->Discount[i].Info = 0;										// �������F���g�p(0)

			i++;
		}
	}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	if (isEMoneyReader()) {
		if( syukei->Ec_minashi_cnt ){
			// �݂Ȃ�����
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
			p_NtDat->Discount[i].Kind = NTNET_DEEMED_PAY;						// ������ʁF1021�i�݂Ȃ����ρj�Œ�
			p_NtDat->Discount[i].Group = 0;										// �����敪�F(0)���������
			p_NtDat->Discount[i].Num = syukei->Ec_minashi_cnt;					// �����񐔁F
			p_NtDat->Discount[i].Callback = 0;									// ��������F���g�p(0)
			p_NtDat->Discount[i].Amount = syukei->Ec_minashi_ryo;				// �����z  �F
			p_NtDat->Discount[i].Info = 0;										// �������F���g�p(0)

			i++;
		}
		if( syukei->miryo_pay_ok_cnt ){
			// �����x���ς�
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
			p_NtDat->Discount[i].Kind = NTNET_MIRYO_PAY;						// ������ʁF1022�i�����x���ς݁j�Œ�
			p_NtDat->Discount[i].Group = 0;										// �����敪�F(0)���������
			p_NtDat->Discount[i].Num = syukei->miryo_pay_ok_cnt;				// �����񐔁F
			p_NtDat->Discount[i].Callback = 0;									// ��������F���g�p(0)
			p_NtDat->Discount[i].Amount = syukei->miryo_pay_ok_ryo;				// �����z  �F
			p_NtDat->Discount[i].Info = 0;										// �������F���g�p(0)

			i++;
		}
		if( syukei->miryo_unknown_cnt ){
			// �����x���s��
			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
			p_NtDat->Discount[i].Kind = NTNET_MIRYO_UNKNOWN;					// ������ʁF1023�i�����x���s���j�Œ�
			p_NtDat->Discount[i].Group = 0;										// �����敪�F(0)���������
			p_NtDat->Discount[i].Num = syukei->miryo_unknown_cnt;				// �����񐔁F
			p_NtDat->Discount[i].Callback = 0;									// ��������F���g�p(0)
			p_NtDat->Discount[i].Amount = syukei->miryo_unknown_ryo;			// �����z  �F
			p_NtDat->Discount[i].Info = 0;										// �������F���g�p(0)

			i++;
		}
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//// MH322914(S) K.Onodera 2016/10/12 AI-V�Ή�
////	if(prm_get(COM_PRM, S_PAY,25, 1, 3) == 1){
//	if( _is_Normal_pip() ){
//// MH322914(E) K.Onodera 2016/10/12 AI-V�Ή�
//		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
//		p_NtDat->Discount[i].Kind = NTNET_GENGAKU;								// ������ʁF���z���Z
//		p_NtDat->Discount[i].Group = 0;											// �����敪�F���g�p(0)
//		p_NtDat->Discount[i].Num = syukei->Gengaku_seisan_cnt;					// �����񐔁F
//		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
//		p_NtDat->Discount[i].Amount = syukei->Gengaku_seisan_ryo;				// �����z  �F
//		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
//		i++;
//	}
//
//// MH322914(S) K.Onodera 2016/10/12 AI-V�Ή�
////	if(prm_get(COM_PRM, S_PAY,25, 1, 3) == 1){
//	if( _is_Normal_pip() ){
//// MH322914(E) K.Onodera 2016/10/12 AI-V�Ή�
//		p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏ�No�F�����Ԕԍ�
//		p_NtDat->Discount[i].Kind = NTNET_FURIKAE;								// ������ʁF�U�֐��Z
//		p_NtDat->Discount[i].Group = 0;											// �����敪�F���g�p(0)
//		p_NtDat->Discount[i].Num = syukei->Furikae_seisan_cnt;					// �����񐔁F
//		p_NtDat->Discount[i].Callback = 0;										// ��������F���g�p(0)
//		p_NtDat->Discount[i].Amount = syukei->Furikae_seisan_ryo;				// �����z  �F
//		p_NtDat->Discount[i].Info = 0;											// �������F���g�p(0)
//		i++;
//	}
//// MH322914(S) K.Onodera 2016/10/11 AI-V�Ή��F�U�֐��Z
//	if( _is_ParkingWeb_pip() ){
//		for( mod=0; mod<MOD_TYPE_MAX; mod++ ){
//			switch( mod ){
//				case MOD_TYPE_CREDIT:
//					// �N���W�b�g�g�p�ݒ肠��H
//					if( !prm_get( COM_PRM, S_PAY, 24, 1, 2 ) ){
//						continue;
//					}
//					break;
//				case MOD_TYPE_EMONEY:
//					if( !prm_get( COM_PRM, S_PAY, 24, 1, 3 )  ){
//						continue;
//					}
//					break;
//				default:
//					continue;
//					break;
//			}
//			p_NtDat->Discount[i].ParkingNo = CPrmSS[S_SYS][1];					// ���ԏ�No�F�����Ԕԍ�
//			p_NtDat->Discount[i].Kind = NTNET_KABARAI;							// ������ʁF�U�֐��Z(ParkingWeb��)
//			p_NtDat->Discount[i].Group = mod;									// �����敪�F�}�̎��
//// �s��C��(S) K.Onodera 2016/11/25 #1588 �����W�v�f�[�^�ɐU�։ߕ������̏�񂪃Z�b�g����Ȃ�
////			if( syukei->Furikae_CardKabarai[i] ){
//			if( syukei->Furikae_CardKabarai[mod] ){
//// �s��C��(E) K.Onodera 2016/11/25 #1588 �����W�v�f�[�^�ɐU�։ߕ������̏�񂪃Z�b�g����Ȃ�
//				p_NtDat->Discount[i].Num = syukei->Furikae_Card_cnt[mod];		// �����񐔁F���g�p(0)
//				p_NtDat->Discount[i].Callback = 0;								// ��������F���g�p(0)
//// �s��C��(S) K.Onodera 2016/11/25 #1588 �����W�v�f�[�^�ɐU�։ߕ������̏�񂪃Z�b�g����Ȃ�
////				p_NtDat->Discount[i].Amount = syukei->Furikae_CardKabarai[i];	// �����z  �F�ߕ������z
//				p_NtDat->Discount[i].Amount = syukei->Furikae_CardKabarai[mod];	// �����z  �F�ߕ������z
//// �s��C��(E) K.Onodera 2016/11/25 #1588 �����W�v�f�[�^�ɐU�։ߕ������̏�񂪃Z�b�g����Ȃ�
//				p_NtDat->Discount[i].Info = 0;									// �������F���g�p(0)
//			}
//			i++;
//		}
//	}
//// MH322914(E) K.Onodera 2016/10/11 AI-V�Ή��F�U�֐��Z
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	ret = sizeof( DATA_KIND_45 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			����W�v�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: ����W�v�f�[�^(DATA_KIND_46�^)�ւ̃|�C���^  
///	@return			ret		: ����W�v�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//unsigned short	NTNET_Edit_SyukeiTeiki_r10( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat )
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//unsigned short	NTNET_Edit_SyukeiTeiki_r13( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiTeiki_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
{
	int		i;
	int		parking;
	int		kind;
	ushort	pos;
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_46 ) );
	NTNET_Edit_BasicData( 46, 0, syukei->SeqNo[3], &p_NtDat->DataBasic);		// �s���v�F�f�[�^�ێ��t���O = 0

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//	p_NtDat->FmtRev					= 10;										// �t�H�[�}�b�gRev.No.
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//	p_NtDat->FmtRev					= 13;										// �t�H�[�}�b�gRev.No.
	p_NtDat->FmtRev					= 15;										// �t�H�[�}�b�gRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// �W�v�^�C�v 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// �W�v�ǔ�
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// �ʒǔ�
		if (Type == 11) {														// GT�W�v
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// �I���ǔ�
		}
		break;
	case	1:		// �ʂ��ǂ���
		if (Type == 1 || Type == 11) {
		// T���v�^���v
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// �I���ǔ�
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// �Z���^�[�ǔԁi�W�v�j
	i = 0;
	for (parking = 0; parking < PKNO_SYU_CNT; parking++) {
		if(CPrmSS[S_SYS][1+parking]) {											// ���ԏ�ݒ肠��
			if(parking == 0) {													// ��{���ԏ�
				pos = 6;
			}
			else if(parking == 1) {												// �g��1���ԏ�
				pos = 1;
			}
			else if(parking == 2) {												// �g��3���ԏ�
				pos = 2;
			}
			else if(parking == 3) {												// �g��3���ԏ�
				pos = 3;
			}
			if(prm_get(COM_PRM, S_SYS, 70, 1, pos)) {							// �Ώۂ̒��ԏ�Ŏg�p��
				for (kind = 0; kind < TEIKI_SYU_CNT; kind++) {
// GG124100(S) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
// 					if(0 != prm_get(COM_PRM, S_PAS, 1 + (10 * kind), 2, 1)) {		// ����g�p�H
					if ( (CLOUD_CALC_MODE &&											// �N���E�h�����v�Z���[�h
						  syukei->Teiki_use_cnt[parking][kind]) ||						//  �񐔃f�[�^����
						 (!CLOUD_CALC_MODE &&											// �ʏ헿���v�Z���[�h
						  prm_get(COM_PRM, S_PAS, (short)(1 + (10 * kind)), 2, 1)) ) {	//  �ݒ肠��
// GG124100(E) R.Endo 2022/10/05 �Ԕԃ`�P�b�g���X3.0 #6636 NT-NET�����W�v�f�[�^�Ɏg�p���������̓��e�����f����Ȃ�
						p_NtDat->Pass[i].ParkingNo		= CPrmSS[S_SYS][1+parking];	// ���ԏ�No.
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//						p_NtDat->Pass[i].Kind			= kind + 1;					// ���
						p_NtDat->Pass[i].Kind			= (uchar)(kind + 1);		// ���
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
						p_NtDat->Pass[i].Num			= syukei->Teiki_use_cnt[parking][kind];	// ��
						p_NtDat->Pass[i].Callback		= 0;						// ��������i���g�p�j
						i++;
					}
				}
			}
		}
	}

	ret = sizeof( DATA_KIND_46 ); 
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			�W�v�I���ʒm�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: �W�v�I���ʒm�f�[�^(DATA_KIND_53�^)�ւ̃|�C���^  
///	@return			ret		: �W�v�I���ʒm�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//unsigned short	NTNET_Edit_SyukeiSyuryo_r10( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat )
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//unsigned short	NTNET_Edit_SyukeiSyuryo_r13( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiSyuryo_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
{
	ushort	ret;

	memset( p_NtDat, 0, sizeof( DATA_KIND_53 ) );
	NTNET_Edit_BasicData( 53, 0, syukei->SeqNo[4], &p_NtDat->DataBasic);		// ��{�ް��쐬

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b
// �d�l�ύX(S) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//	p_NtDat->FmtRev					= 10;										// �t�H�[�}�b�gRev.No.
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//	p_NtDat->FmtRev					= 13;										// �t�H�[�}�b�gRev.No.
	p_NtDat->FmtRev					= 15;										// �t�H�[�}�b�gRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
// �d�l�ύX(E) K.Onodera 2016/12/14 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// �W�v�^�C�v 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// �W�v�ǔ�
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// �ʒǔ�
		if (Type == 11) {														// GT�W�v
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// �I���ǔ�
		}
		break;
	case	1:		// �ʂ��ǂ���
		if (Type == 1 || Type == 11) {
		// T���v�^���v
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// �I���ǔ�
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// �Z���^�[�ǔԁi�W�v�j

	ret = sizeof( DATA_KIND_53 ); 
	return ret;
}

//	WEB�p���ԑ䐔�pNT-NET�t�H�[�}�b�g�ݒ�(236̫�ϯĂƂ͕ʂ�58_r10̫�ϯĂ��쐬)	// �Q�� = NTNET_Edit_Data236
unsigned short	NTNET_Edit_Data58_r10(	ParkCar_log			*p_RcptDat,		// ���ԑ䐔�plog�ް�(IN)
										DATA_KIND_58_r10 	*p_NtDat_r10 )	// ���ԑ䐔�pNT-NET(OUT)
{
	unsigned short ret;

	memset( p_NtDat_r10, 0, sizeof( DATA_KIND_58_r10 ) );
	NTNET_Edit_BasicData( 58, 0, p_RcptDat->CMN_DT.DT_58.SeqNo, &p_NtDat_r10->DataBasic);	// ��{�ް��쐬

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat_r10->DataBasic.Year = (uchar)(p_RcptDat->Time.Year % 100);	// �����N(���O�쐬���ɉ�2���ɂ��Ă���)
	p_NtDat_r10->DataBasic.Mon = (uchar)p_RcptDat->Time.Mon;			// ������
	p_NtDat_r10->DataBasic.Day = (uchar)p_RcptDat->Time.Day;			// ������
	p_NtDat_r10->DataBasic.Hour = (uchar)p_RcptDat->Time.Hour;			// ������
	p_NtDat_r10->DataBasic.Min = (uchar)p_RcptDat->Time.Min;			// ������
	p_NtDat_r10->DataBasic.Sec = (uchar)p_RcptDat->Time.Sec;			// �����b

	// 236 -> 58_r10(��̫�ϯ�)���ް��̐ݒ�(FmtRev, PARKCAR_DATA2)
	p_NtDat_r10->FmtRev = 10;											// �t�H�[�}�b�gRev.��
	p_NtDat_r10->ParkData.FullSts1	= p_RcptDat->Full[0];				// ����1������
	p_NtDat_r10->ParkData.EmptyNo1	= p_RcptDat->CMN_DT.DT_58.EmptyNo1;	// ��ԑ䐔�P
	p_NtDat_r10->ParkData.FullNo1	= p_RcptDat->CMN_DT.DT_58.FullNo1;	// ���ԑ䐔�P
	p_NtDat_r10->ParkData.FullSts2	= p_RcptDat->Full[1];				// ����2������
	p_NtDat_r10->ParkData.EmptyNo2	= p_RcptDat->CMN_DT.DT_58.EmptyNo2;	// ��ԑ䐔�Q
	p_NtDat_r10->ParkData.FullNo2	= p_RcptDat->CMN_DT.DT_58.FullNo2;	// ���ԑ䐔�Q
	p_NtDat_r10->ParkData.FullSts3	= p_RcptDat->Full[2];				// ����3������
	p_NtDat_r10->ParkData.EmptyNo3	= p_RcptDat->CMN_DT.DT_58.EmptyNo3;	// ��ԑ䐔�R
	p_NtDat_r10->ParkData.FullNo3	= p_RcptDat->CMN_DT.DT_58.FullNo3;	// ���ԑ䐔�R

	ret = sizeof( DATA_KIND_58_r10 ); 
	return ret;
}
/*[]----------------------------------------------------------------------[]*/
/*| �Z���^�[�p�[������ް�(�ް����65)�쐬����                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data65                                        |*/
/*| PARAMETER    : MachineNo : ���M��[���@�B��                            |*/
/*| RETURN VALUE : void                                                    |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTNET_Snd_Data65( ulong MachineNo )
{
	ushort	len;
	char*	pStr;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_65 ) );

	BasicDataMake( 65, 0 );										// ��{�ް��쐬

	SendNtnetDt.SData65.DataBasic.SeqNo = GetNtDataSeqNo();		// �V�[�P���V����No.
// GG120600(S) // Phase9 �o�[�W�����ύX
//// GG120600(S) // MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb�t�F�[�Y9�Ή�)
////	SendNtnetDt.SData65.FmtRev = 11;							// �t�H�[�}�b�gRev.No.(11�Œ�)
//	SendNtnetDt.SData65.FmtRev = 12;							// �t�H�[�}�b�gRev.No.(12�Œ�)
//// GG120600(E) // MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb�t�F�[�Y9�Ή�)
	SendNtnetDt.SData65.FmtRev = 13;							// �t�H�[�}�b�gRev.No.(13�Œ�)
// GG120600(E) // Phase9 �o�[�W�����ύX
	SendNtnetDt.SData65.SMachineNo = MachineNo;					// ���M��[���@�B��

	SendNtnetDt.SData65.Result				= 0;				// �v������
	SendNtnetDt.SData65.AcceptReq.bits.b00	= ReqAcceptTbl[VER_UP_REQ];			// �o�[�W�����A�b�v�v����t
	SendNtnetDt.SData65.AcceptReq.bits.b01	= ReqAcceptTbl[PARAM_CHG_REQ];		// �ݒ�ύX�v����t
	SendNtnetDt.SData65.AcceptReq.bits.b02	= ReqAcceptTbl[PARAM_UPLOAD_REQ];	// �ݒ�v����t
	SendNtnetDt.SData65.AcceptReq.bits.b03	= ReqAcceptTbl[RESET_REQ];			// ���Z�b�g�v����t
	SendNtnetDt.SData65.AcceptReq.bits.b04	= ReqAcceptTbl[PROG_ONLY_CHG_REQ];	// �v���O�����؊��v����t
	SendNtnetDt.SData65.AcceptReq.bits.b05	= ReqAcceptTbl[FTP_CHG_REQ];		// FTP�ݒ�ύX�v����t
	SendNtnetDt.SData65.AcceptReq.bits.b06	= ReqAcceptTbl[REMOTE_FEE_REQ];		// ���u�����ݒ��t
// MH810100(S) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	SendNtnetDt.SData65.AcceptReq.bits.b07	= ReqAcceptTbl[PARAM_DIFF_CHG_REQ];	// �ݒ�ύX�v����t
// MH810100(E) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)

	// RXM�\�t�g�o�[�W�����i�E�l�߁j
	memcpy( SendNtnetDt.SData65.ProgramVer, "RXM ", 4 );
	if(strlen((char*)VERSNO.ver_part)) {
		memcpy( &SendNtnetDt.SData65.ProgramVer[4], VERSNO.ver_part, 8 );
	}
	else {
		memcpy( &SendNtnetDt.SData65.ProgramVer[4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
	}
	
// GG120600(S) // Phase9 Version�𕪂���
//// GG120600(S) // MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb�t�F�[�Y9�Ή�)
//	// �x�[�X�v���O�����o�[�W����(���v���O�����o�[�W����)
//	memcpy( SendNtnetDt.SData65.BaseProgramVer, &SendNtnetDt.SData65.ProgramVer, 12 );
//	
//	// �����v�Zdll�o�[�W����(���v���O�����o�[�W����)
//	memcpy( SendNtnetDt.SData65.RyoCalDllVer, &SendNtnetDt.SData65.ProgramVer, 12 );
//// GG120600(E) // MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb�t�F�[�Y9�Ή�)

	// �x�[�X�v���O�����o�[�W����(���v���O�����o�[�W����)
	memcpy( SendNtnetDt.SData65.BaseProgramVer, "RXM ", 4 );
	if(strlen((char*)VERSNO_BASE.ver_part)) {
		memcpy( &SendNtnetDt.SData65.BaseProgramVer[4], VERSNO_BASE.ver_part, 8 );
	}
	else {
		memcpy( &SendNtnetDt.SData65.BaseProgramVer[4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
	}

// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6554 �Z���^�p�[�����f�[�^ �ύX�Ή�
	if ( !CLOUD_CALC_MODE ) {	// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6554 �Z���^�p�[�����f�[�^ �ύX�Ή�
	
	// �����v�Zdll�o�[�W����(���v���O�����o�[�W����)
	memcpy( SendNtnetDt.SData65.RyoCalDllVer, "RXM ", 4 );
	if(strlen((char*)VERSNO_RYOCAL.ver_part)) {
		memcpy( &SendNtnetDt.SData65.RyoCalDllVer[4], VERSNO_RYOCAL.ver_part, 8 );
	}
	else {
		memcpy( &SendNtnetDt.SData65.RyoCalDllVer[4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
	}

// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6554 �Z���^�p�[�����f�[�^ �ύX�Ή�
	}
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6554 �Z���^�p�[�����f�[�^ �ύX�Ή�

// GG120600(E) // Phase9 Version�𕪂���
	
	// RXF�\�t�g�o�[�W�����i�E�l�߁j
	memcpy( &SendNtnetDt.SData65.ModuleVer[0][0], "RXF ", 4 );
	pStr = strchr((char*)RXF_VERSION, ' ');
	if(pStr) {
		len = (ushort)((uchar*)pStr - (uchar*)RXF_VERSION);
	}
	else {
		len = strlen((char*)RXF_VERSION);
	}
	if(len) {
		if(len > 9) {
			len = 9;
		}
		memcpy( &SendNtnetDt.SData65.ModuleVer[0][12 - len], RXF_VERSION, len);
	}
	else {
		memcpy( &SendNtnetDt.SData65.ModuleVer[0][4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
	}
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb�t�F�[�Y9�Ή�)
//	// �����f�[�^�o�[�W�����i�E�l�߁j
//	memcpy( &SendNtnetDt.SData65.ModuleVer[1][0], "RXM ", 4 );
//	pStr = strchr((char*)SOUND_VERSION, ' ');
//	if(pStr) {
//		len = (ushort)((uchar*)pStr - (uchar*)SOUND_VERSION);
//	}
//	else {
//		len = strlen((char*)SOUND_VERSION);
//	}
//	if(len) {
//		if(len > 9) {
//			len = 9;
//		}
//		memcpy( &SendNtnetDt.SData65.ModuleVer[1][12 - len], SOUND_VERSION, len);
//	}
//	else {
//		memcpy( &SendNtnetDt.SData65.ModuleVer[1][4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
//	}
//	
//	// ���C���[�_�[�o�[�W�����i�E�l�߁j
//	if( prm_get( COM_PRM,S_PAY,21,1,3 ) ){						// ���Cذ�ް�g�p
//		memcpy( &SendNtnetDt.SData65.ModuleVer[2][0], "FBL ", 4 );
//		pStr = strchr((char*)MRD_VERSION, ' ');
//		if(pStr) {
//			len = (ushort)((uchar*)pStr - (uchar*)MRD_VERSION);
//		}
//		else {
//			len = strlen((char*)MRD_VERSION);
//		}
//		if(len) {
//			if(len > 9) {
//				len = 9;
//			}
//			memcpy( &SendNtnetDt.SData65.ModuleVer[2][12 - len], MRD_VERSION, len);
//		}
//		else {
//			memcpy( &SendNtnetDt.SData65.ModuleVer[2][4], ntdata65_UnknownVer, strlen(ntdata65_UnknownVer) );
//		}
//	}
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb�t�F�[�Y9�Ή�)
// GG120600(S) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�
	// ��M�d�����r�W����
	SendNtnetDt.SData65.DataRevs[0].DataProto	= 1;				// 1=���uNT-NET
	SendNtnetDt.SData65.DataRevs[0].DataSysID	= REMOTE_DL_SYSTEM;	// �V�X�e��ID
	SendNtnetDt.SData65.DataRevs[0].DataID		= REMOTE_MNT_REQ;	// ���u�����e�i���X�v��
	SendNtnetDt.SData65.DataRevs[0].DataRev		= 11;
// GG120600(E) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_65 ));
	}
	else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_65 ), NTNET_BUF_PRIOR );	// �ް����M�o�^
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���Z���f�[�^�p�ۑ��i�[����
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152Save(void *saveData, uchar saveDataCategory)
{

	NTNET_152_U_SAVEINF *uData152;
	
	// ���uNT-NET�̐ڑ��ݒ肪�L���ł������e�X�g���s���łȂ���ΐ��Z���f�[�^���X�V
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		uData152 = (NTNET_152_U_SAVEINF *)saveData;
		switch(saveDataCategory){
			case NTNET_152_OIBAN:
				memcpy(&ntNet_152_SaveData.Oiban, (uchar *)&uData152->Oiban, sizeof(ntNet_152_SaveData.Oiban));
				break;
			case NTNET_152_PAYMETHOD:
				ntNet_152_SaveData.PayMethod = uData152->PayMethod;
				break;
			case NTNET_152_PAYMODE:
				ntNet_152_SaveData.PayMode = uData152->PayMode;
				break;
// MH810100(S) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
			case NTNET_152_CMACHINENO:
				ntNet_152_SaveData.CMachineNo = uData152->CMachineNo;
				break;
// MH810100(E) K.Onodera  2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//			case NTNET_152_WPLACE:
//				ntNet_152_SaveData.WPlace = uData152->WPlace;
//				break;
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			case NTNET_152_KAKARINO:
				ntNet_152_SaveData.KakariNo = uData152->KakariNo;
				break;
			case NTNET_152_OUTKIND:
				ntNet_152_SaveData.OutKind = uData152->OutKind;
				break;
			case NTNET_152_COUNTSET:
				ntNet_152_SaveData.CountSet = uData152->CountSet;
				break;
			case NTNET_152_CARINTIME:
				memcpy(&ntNet_152_SaveData.carInTime, (uchar *)&uData152->carInTime, sizeof(ntNet_152_SaveData.carInTime));
				break;
			case NTNET_152_RECEIPTISSUE:
				ntNet_152_SaveData.ReceiptIssue = uData152->ReceiptIssue;
				break;
			case NTNET_152_SYUBET:
				ntNet_152_SaveData.Syubet = uData152->Syubet;
				break;
			case NTNET_152_PRICE:
				ntNet_152_SaveData.Price = uData152->Price;
				break;
			case NTNET_152_CASHPRICE:
				ntNet_152_SaveData.CashPrice = uData152->CashPrice;
				break;
			case NTNET_152_INPRICE:
				ntNet_152_SaveData.InPrice = uData152->InPrice;
				break;
			case NTNET_152_CHGPRICE:
				ntNet_152_SaveData.ChgPrice = uData152->ChgPrice;
				break;
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// �s��C��(S) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
//			case NTNET_152_KABARAI:
//				ntNet_152_SaveData.FrkReturn = uData152->FrkReturn;
//				break;
//// �s��C��(E) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			case NTNET_152_INCOIN:
				memcpy(&ntNet_152_SaveData.in_coin[0], (uchar *)uData152->in_coin, sizeof(ntNet_152_SaveData.in_coin));
				break;
			case NTNET_152_OUTCOIN:
				memcpy(&ntNet_152_SaveData.out_coin[0], (uchar *)uData152->out_coin, sizeof(ntNet_152_SaveData.out_coin));
				break;
			case NTNET_152_FESCROW:
				ntNet_152_SaveData.f_escrow = uData152->f_escrow;
				break;
			case NTNET_152_HARAIMODOSHIFUSOKU:
				ntNet_152_SaveData.HaraiModoshiFusoku = uData152->HaraiModoshiFusoku;
				break;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
			case NTNET_152_CARD_FUSOKU_TYPE:
				ntNet_152_SaveData.CardFusokuType = uData152->CardFusokuType;
				break;
			case NTNET_152_CARD_FUSOKU:
				ntNet_152_SaveData.CardFusokuTotal = uData152->CardFusokuTotal;
				break;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
			case NTNET_152_ANTIPASSCHECK:
				ntNet_152_SaveData.AntiPassCheck = uData152->AntiPassCheck;
				break;
			case NTNET_152_PARKNOINPASS:
				ntNet_152_SaveData.ParkNoInPass = uData152->ParkNoInPass;
				break;
			case NTNET_152_PKNOSYU:
				ntNet_152_SaveData.pkno_syu = uData152->pkno_syu;
				break;
			case NTNET_152_TEIKIID:
				ntNet_152_SaveData.teiki_id = uData152->teiki_id;
				break;
			case NTNET_152_TEIKISYU:
				ntNet_152_SaveData.teiki_syu = uData152->teiki_syu;
				break;
// MH810100(S) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
			case NTNET_152_MEDIAKIND1:
				ntNet_152_SaveData.MediaKind1 = uData152->MediaKind1;
				break;
			case NTNET_152_MEDIACARDNO1:
				memcpy( ntNet_152_SaveData.MediaCardNo1, (uchar *)uData152->MediaCardNo1, sizeof(ntNet_152_SaveData.MediaCardNo1));
				break;
			case NTNET_152_MEDIACARDINFO1:
				memcpy( ntNet_152_SaveData.MediaCardInfo1, (uchar *)uData152->MediaCardInfo1, sizeof(ntNet_152_SaveData.MediaCardInfo1));
				break;
			case NTNET_152_MEDIAKIND2:
				ntNet_152_SaveData.MediaKind2 = uData152->MediaKind2;
				break;
			case NTNET_152_MEDIACARDNO2:
				memcpy( ntNet_152_SaveData.MediaCardNo2, (uchar *)uData152->MediaCardNo2, sizeof(ntNet_152_SaveData.MediaCardNo2));
				break;
			case NTNET_152_MEDIACARDINFO2:
				memcpy( ntNet_152_SaveData.MediaCardInfo2, (uchar *)uData152->MediaCardInfo2, sizeof(ntNet_152_SaveData.MediaCardInfo2));
				break;
// MH810100(E) K.Onodera 2019/12/16 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
			case NTNET_152_ECARDKIND:
				ntNet_152_SaveData.e_pay_kind = uData152->e_pay_kind;
				break;
			case NTNET_152_EPAYRYO:
				ntNet_152_SaveData.e_pay_ryo = uData152->e_pay_ryo;
				break;
			case NTNET_152_ECARDID:
// MH321800(S) hosoda IC�N���W�b�g�Ή�
//-				memcpy(&ntNet_152_SaveData.e_Card_ID[0], (uchar *)uData152->e_Card_ID, sizeof(ntNet_152_SaveData.e_Card_ID));
				if (EcUseKindCheck(ntNet_152_SaveData.e_pay_kind)) {	// �K��NTNET_152_ECARDKIND����ɌĂ΂�Ă���̂ŃJ�[�h��ʂ̔���Ɏg��
				// ���σ��[�_�ł̃J�[�h�d����
					memcpy(&ntNet_152_SaveData.e_Card_ID[0], (uchar *)uData152->e_Card_ID, sizeof(ntNet_152_SaveData.e_Card_ID));
				}
				else {
				// SX10�ł̃J�[�h�d����
					memset(&ntNet_152_SaveData.e_Card_ID[0], 0x20, sizeof(ntNet_152_SaveData.e_Card_ID));
					memcpy(&ntNet_152_SaveData.e_Card_ID[0], (uchar *)uData152->e_Card_ID, sizeof(PayData.Electron_data.Suica.Card_ID));
				}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
				break;
			case NTNET_152_EPAYAFTER:
				ntNet_152_SaveData.e_pay_after = uData152->e_pay_after;
				break;
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
			case NTNET_152_ECINQUIRYNUM:
// MH810103 GG119202(S) �d�q�}�l�[�Ή�
//				memcpy(&ntNet_152_SaveData.e_inquiry_num[0], (uchar *)uData152->e_inquiry_num, sizeof(ntNet_152_SaveData.e_inquiry_num));
				// 15���̖₢���킹�ԍ������l�߂�16���ɃZ�b�g
				memcpyFlushLeft( (uchar *)&ntNet_152_SaveData.e_inquiry_num[0], (uchar *)&uData152->e_inquiry_num[0],
								sizeof(ntNet_152_SaveData.e_inquiry_num), sizeof(PayData.Electron_data.Ec.inquiry_num) );	// �₢���킹�ԍ�
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
				break;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
			case NTNET_152_CPAYRYO:
				ntNet_152_SaveData.c_pay_ryo = uData152->c_pay_ryo;
				break;
			case NTNET_152_CCARDNO:
				memcpy(&ntNet_152_SaveData.c_Card_No[0], (uchar *)uData152->c_Card_No, sizeof(ntNet_152_SaveData.c_Card_No));
				break;
			case NTNET_152_CCCTNUM:
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				memcpy(&ntNet_152_SaveData.c_cct_num[0], (uchar *)uData152->c_cct_num, sizeof(ntNet_152_SaveData.c_cct_num));
				memset(&ntNet_152_SaveData.c_cct_num[0], 0x20, sizeof(ntNet_152_SaveData.c_cct_num));
				memcpy(&ntNet_152_SaveData.c_cct_num[0], (uchar *)uData152->c_cct_num, sizeof(ryo_buf.credit.CCT_Num));
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
				break;
			case NTNET_152_CKID:
				memcpy(&ntNet_152_SaveData.c_kid_code[0], (uchar *)uData152->c_kid_code, sizeof(ntNet_152_SaveData.c_kid_code));
				break;
			case NTNET_152_CAPPNO:
				ntNet_152_SaveData.c_app_no = uData152->c_app_no;
				break;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
			case NTNET_152_CTRADENO:
				memcpy(&ntNet_152_SaveData.c_trade_no[0], (uchar *)uData152->c_trade_no, sizeof(ntNet_152_SaveData.c_trade_no));
				break;
			case NTNET_152_SLIPNO:
				ntNet_152_SaveData.c_slipNo = uData152->c_slipNo;
				break;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			case NTNET_152_TRANS_STS:
				ntNet_152_SaveData.e_Transactiontatus = uData152->e_Transactiontatus;
				break;
			case NTNET_152_QR_MCH_TRADE_NO:
				memcpy(ntNet_152_SaveData.qr_MchTradeNo, uData152->qr_MchTradeNo, sizeof(ntNet_152_SaveData.qr_MchTradeNo));
				break;
			case NTNET_152_QR_PAY_TERM_ID:
				memcpy(ntNet_152_SaveData.qr_PayTermID, uData152->qr_PayTermID, sizeof(ntNet_152_SaveData.qr_PayTermID));
				break;
			case NTNET_152_QR_PAYKIND:
				ntNet_152_SaveData.qr_PayKind = uData152->qr_PayKind;
				break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���Z���f�[�^�p �����ڍ׃f�[�^�ۑ�����
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_DiscDataSave(void *saveData, uchar saveDataCategory, uchar saveIndex)
{

	NTNET_152_U_SAVEINF *uData152;
	
	// ���uNT-NET�̐ڑ��ݒ肪�L���ł������e�X�g���s���łȂ���ΐ��Z���f�[�^���X�V
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		uData152 = (NTNET_152_U_SAVEINF *)saveData;
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// // MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// 		if( uData152->DiscountData.uDiscData.common.DiscFlg ){	// �����ς�
// 			return;
// 		}
// // MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
		switch(saveDataCategory){
			case NTNET_152_DPARKINGNO:
				ntNet_152_SaveData.DiscountData[saveIndex].ParkingNo = uData152->DiscountData.ParkingNo;
				break;
			case NTNET_152_DSYU:
				ntNet_152_SaveData.DiscountData[saveIndex].DiscSyu = uData152->DiscountData.DiscSyu;
				break;
			case NTNET_152_DNO:
				ntNet_152_SaveData.DiscountData[saveIndex].DiscNo = uData152->DiscountData.DiscNo;
				break;
			case NTNET_152_DCOUNT:
				ntNet_152_SaveData.DiscountData[saveIndex].DiscCount = uData152->DiscountData.DiscCount;
				break;
			case NTNET_152_DISCOUNT:
				ntNet_152_SaveData.DiscountData[saveIndex].Discount = uData152->DiscountData.Discount;
				break;
			case NTNET_152_DINFO1:
				ntNet_152_SaveData.DiscountData[saveIndex].DiscInfo1 = uData152->DiscountData.DiscInfo1;
				break;
			case NTNET_152_DINFO2:
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.DiscInfo2 = uData152->DiscountData.uDiscData.common.DiscInfo2;
				break;
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			case NTNET_152_PREVDISCOUNT:		// ���� ����g�p�����O�񐸎Z�܂ł̊������z
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.PrevDiscount =
					uData152->DiscountData.uDiscData.common.PrevDiscount;
				break;
			case NTNET_152_PREVUSAGEDISCOUNT:	// ���� �O�񐸎Z�܂ł̎g�p�ς݊������z
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.PrevUsageDiscount =
					uData152->DiscountData.uDiscData.common.PrevUsageDiscount;
				break;
			case NTNET_152_PREVUSAGEDCOUNT:		// ���� �O�񐸎Z�܂ł̎g�p�ςݖ���
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.PrevUsageDiscCount =
					uData152->DiscountData.uDiscData.common.PrevUsageDiscCount;
				break;
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// GG124100(S) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
			case NTNET_152_FEEKINDSWITCHSETTING:	// ���� ��ʐ؊���Ԏ�
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.FeeKindSwitchSetting =
					uData152->DiscountData.uDiscData.common.FeeKindSwitchSetting;
				break;
			case NTNET_152_DROLE:					// ���� ��������
				ntNet_152_SaveData.DiscountData[saveIndex].uDiscData.common.DiscRole =
					uData152->DiscountData.uDiscData.common.DiscRole;
				break;
// GG124100(E) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
			default:
				break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���Z���f�[�^�p�����ڍ׃G���A�N���A
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_DiscDataClear(void)
{

	// ���uNT-NET�̐ڑ��ݒ肪�L���ł������e�X�g���s���łȂ���ΐ��Z���f�[�^���X�V
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		memset((uchar *)&ntNet_152_SaveData.DiscountData[0], 0, sizeof(ntNet_152_SaveData.DiscountData));
	}
}

// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
/*[]----------------------------------------------------------------------[]*
 *| ���Z���f�[�^�p ���׃f�[�^�ۑ�����
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_DetailDataSave( void *saveData, uchar saveDataCategory, uchar saveIndex )
{

	NTNET_152_U_SAVEINF *uData152;
	
	// ���uNT-NET�̐ڑ��ݒ肪�L���ł������e�X�g���s���łȂ���ΐ��Z���f�[�^���X�V
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		uData152 = (NTNET_152_U_SAVEINF *)saveData;
		switch(saveDataCategory){
			case NTNET_152_DPARKINGNO:
				ntNet_152_SaveData.DetailData[saveIndex].ParkingNo = uData152->DetailData.ParkingNo;
				break;
			case NTNET_152_DSYU:
				ntNet_152_SaveData.DetailData[saveIndex].DiscSyu = uData152->DetailData.DiscSyu;
				break;
			case NTNET_152_DNO:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.DiscNo = uData152->DetailData.uDetail.Common.DiscNo;
				break;
			case NTNET_152_DCOUNT:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.DiscCount = uData152->DetailData.uDetail.Common.DiscCount;
				break;
			case NTNET_152_DISCOUNT:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.Discount = uData152->DetailData.uDetail.Common.Discount;
				break;
			case NTNET_152_DINFO1:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.DiscInfo1 = uData152->DetailData.uDetail.Common.DiscInfo1;
				break;
			case NTNET_152_DINFO2:
				ntNet_152_SaveData.DetailData[saveIndex].uDetail.Common.DiscInfo2 = uData152->DetailData.uDetail.Common.DiscInfo2;
				break;
			default:
				break;
		}
	}
}
/*[]----------------------------------------------------------------------[]*
 *| ���Z���f�[�^�p���׃G���A�N���A
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_DetailDataClear(void)
{

	// ���uNT-NET�̐ڑ��ݒ肪�L���ł������e�X�g���s���łȂ���ΐ��Z���f�[�^���X�V
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		memset((uchar *)&ntNet_152_SaveData.DetailData[0], 0, sizeof(ntNet_152_SaveData.DetailData));
	}
}
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�

/*[]----------------------------------------------------------------------[]*
 *| ���Z���f�[�^�p�ۑ��̈�S�N���A
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_SaveDataClear(void)
{

	
	// ���uNT-NET�̐ڑ��ݒ肪�L���ł������e�X�g���s���łȂ���ΐ��Z���f�[�^���X�V
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		memset(&ntNet_152_SaveData.Oiban, 0, sizeof(ntNet_152_SaveData.Oiban));
		ntNet_152_SaveData.PayMethod = 0;
		ntNet_152_SaveData.PayMode = 0;
// MH810100(S) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
		ntNet_152_SaveData.CMachineNo = 0;
// MH810100(E) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		ntNet_152_SaveData.WPlace = 0;
// MH810100(E) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		ntNet_152_SaveData.KakariNo = 0;
		ntNet_152_SaveData.OutKind = 0;
		ntNet_152_SaveData.CountSet = 0;
		ntNet_152_SaveData.ReceiptIssue = 0;
		ntNet_152_SaveData.Syubet = 0;
		ntNet_152_SaveData.Price = 0;
		ntNet_152_SaveData.CashPrice = 0;
		ntNet_152_SaveData.InPrice = 0;
		ntNet_152_SaveData.ChgPrice = 0;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		ntNet_152_SaveData.FrkReturn = 0;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		memset(&ntNet_152_SaveData.in_coin[0], 0, sizeof(ntNet_152_SaveData.in_coin));
		memset(&ntNet_152_SaveData.out_coin[0], 0, sizeof(ntNet_152_SaveData.out_coin));
		ntNet_152_SaveData.f_escrow = 0;
		ntNet_152_SaveData.HaraiModoshiFusoku = 0;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		ntNet_152_SaveData.CardFusokuType = 0;
		ntNet_152_SaveData.CardFusokuTotal = 0;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		ntNet_152_SaveData.AntiPassCheck = 0;
		ntNet_152_SaveData.ParkNoInPass = 0;
		ntNet_152_SaveData.pkno_syu = 0;
		ntNet_152_SaveData.teiki_id = 0;
		ntNet_152_SaveData.teiki_syu = 0;
// MH810100(S) K.Onodera  2020/02/17  �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
		ntNet_152_SaveData.MediaKind1 = 0;
		memset(&ntNet_152_SaveData.MediaCardNo1[0], 0, sizeof(ntNet_152_SaveData.MediaCardNo1));
		memset(&ntNet_152_SaveData.MediaCardInfo1[0], 0, sizeof(ntNet_152_SaveData.MediaCardInfo1));
		ntNet_152_SaveData.MediaKind2 = 0;
		memset(&ntNet_152_SaveData.MediaCardNo2[0], 0, sizeof(ntNet_152_SaveData.MediaCardNo2));
		memset(&ntNet_152_SaveData.MediaCardInfo2[0], 0, sizeof(ntNet_152_SaveData.MediaCardInfo2));
// MH810100(E) K.Onodera  2020/02/17  �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
		ntNet_152_SaveData.e_pay_kind = 0;
		ntNet_152_SaveData.e_pay_ryo = 0;
		memset(&ntNet_152_SaveData.e_Card_ID[0], 0, sizeof(ntNet_152_SaveData.e_Card_ID));
		ntNet_152_SaveData.e_pay_after = 0;
		ntNet_152_SaveData.c_pay_ryo = 0;
		memset(&ntNet_152_SaveData.c_Card_No[0], 0, sizeof(ntNet_152_SaveData.c_Card_No));
		memset(&ntNet_152_SaveData.c_cct_num[0], 0, sizeof(ntNet_152_SaveData.c_cct_num));
		memset(&ntNet_152_SaveData.c_kid_code[0], 0, sizeof(ntNet_152_SaveData.c_kid_code));
		ntNet_152_SaveData.c_app_no = 0;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		memset(&ntNet_152_SaveData.c_trade_no[0], 0, sizeof(ntNet_152_SaveData.c_trade_no));
		ntNet_152_SaveData.c_slipNo = 0;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	}
}

/*[]----------------------------------------------------------------------[]*
 *| ���Z���f�[�^�p�ۑ��̈�X�V
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
void NTNET_Data152_SaveDataUpdate(void)
{

	uchar i;
// MH810100(S) K.Onodera  2020/02/17  �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)	
	uchar	card_info[16];
// MH810100(E) K.Onodera  2020/02/17  �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)	
	
	// ���uNT-NET�̐ڑ��ݒ肪�L���ł������e�X�g���s���łȂ���ΐ��Z���f�[�^���X�V
	if((prm_get(COM_PRM, S_PAY,24, 1, 1) == 2) && (ryo_test_flag == 0)){
		memcpy(&ntNet_152_SaveData.Oiban, &PayData.Oiban, sizeof(ntNet_152_SaveData.Oiban));
		ntNet_152_SaveData.PayMethod = PayData.PayMethod;
		ntNet_152_SaveData.PayMode = PayData.PayMode;
// MH810100(S) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
		ntNet_152_SaveData.CMachineNo = PayData.CMachineNo;
// MH810100(E) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
// MH810100(S) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		ntNet_152_SaveData.WPlace = PayData.WPlace;
// MH810100(E) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		ntNet_152_SaveData.KakariNo = PayData.KakariNo;
		ntNet_152_SaveData.OutKind = PayData.OutKind;
		ntNet_152_SaveData.CountSet = PayData.CountSet;
		ntNet_152_SaveData.ReceiptIssue = PayData.ReceiptIssue;
		ntNet_152_SaveData.Syubet = PayData.syu;
		ntNet_152_SaveData.Price = PayData.WPrice;
		ntNet_152_SaveData.CashPrice = PayData.WTotalPrice;
		ntNet_152_SaveData.InPrice = PayData.WInPrice;
		ntNet_152_SaveData.ChgPrice = PayData.WChgPrice;
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// �s��C��(S) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
//		if( PayData.FRK_RetMod == 0 ){
//			ntNet_152_SaveData.FrkReturn = PayData.FRK_Return;
//		}else{
//			ntNet_152_SaveData.FrkReturn = 0;
//		}
//// �s��C��(E) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
		ntNet_152_SaveData.FrkReturn = 0;
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		memcpy(&ntNet_152_SaveData.in_coin[0], &PayData.in_coin[0], sizeof(ntNet_152_SaveData.in_coin));
		memcpy(&ntNet_152_SaveData.out_coin[0], &PayData.out_coin[0], sizeof(ntNet_152_SaveData.out_coin));
		ntNet_152_SaveData.f_escrow = PayData.f_escrow;
		ntNet_152_SaveData.HaraiModoshiFusoku = PayData.WFusoku;
		ntNet_152_SaveData.AntiPassCheck = PayData.PassCheck;
		ntNet_152_SaveData.ParkNoInPass = PayData.teiki.ParkingNo;
		ntNet_152_SaveData.pkno_syu = PayData.teiki.pkno_syu;
		ntNet_152_SaveData.teiki_id = PayData.teiki.id;
		ntNet_152_SaveData.teiki_syu = PayData.teiki.syu;
// MH810100(S) K.Onodera  2020/02/17  �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
		ntNet_152_SaveData.MediaKind1 = PayData.MediaKind1;
		memcpy( ntNet_152_SaveData.MediaCardNo1, PayData.MediaCardNo1, sizeof( ntNet_152_SaveData.MediaCardNo1 ) );
		if( PayData.MediaKind1 == CARD_TYPE_PASS ){
			memset( card_info, 0, sizeof(card_info) );
			intoas( card_info, PayData.teiki.syu, 2 );
			memcpy( ntNet_152_SaveData.MediaCardInfo1, card_info, sizeof( ntNet_152_SaveData.MediaCardInfo1 ) );
		}
		ntNet_152_SaveData.MediaKind2 = PayData.MediaKind2;
		memcpy( ntNet_152_SaveData.MediaCardNo2, PayData.MediaCardNo2, sizeof( ntNet_152_SaveData.MediaCardNo2 ) );
// MH810100(E) K.Onodera  2020/02/17  �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
// MH321800(S) hosoda IC�N���W�b�g�Ή�
		if (isEC_USE()) {
		// ���σ��[�_
			ntNet_152_SaveData.e_pay_kind = PayData.Electron_data.Ec.e_pay_kind;
			ntNet_152_SaveData.e_pay_ryo = PayData.Electron_data.Ec.pay_ryo;
			memcpy(&ntNet_152_SaveData.e_Card_ID[0], &PayData.Electron_data.Ec.Card_ID[0], sizeof(ntNet_152_SaveData.e_Card_ID));
			ntNet_152_SaveData.e_pay_after = PayData.Electron_data.Ec.pay_after;
		}
		else {
// MH321800(E) hosoda IC�N���W�b�g�Ή�
		ntNet_152_SaveData.e_pay_kind = PayData.Electron_data.Suica.e_pay_kind;
		ntNet_152_SaveData.e_pay_ryo = PayData.Electron_data.Suica.pay_ryo;
// MH321800(S) hosoda IC�N���W�b�g�Ή�
//-		memcpy(&ntNet_152_SaveData.e_Card_ID[0], &PayData.Electron_data.Suica.Card_ID[0], sizeof(ntNet_152_SaveData.e_Card_ID));
		// e_Card_ID[20], Suica.Card_ID[16]�̂��߁A�󔒂Ŗ��߂Ă��獶�l�߃R�s�[
		memset(&ntNet_152_SaveData.e_Card_ID[0], 0x20, sizeof(ntNet_152_SaveData.e_Card_ID));
		memcpy(&ntNet_152_SaveData.e_Card_ID[0], &PayData.Electron_data.Suica.Card_ID[0], sizeof(PayData.Electron_data.Suica.Card_ID));
// MH321800(E) hosoda IC�N���W�b�g�Ή�
		ntNet_152_SaveData.e_pay_after = PayData.Electron_data.Suica.pay_after;
// MH321800(S) hosoda IC�N���W�b�g�Ή�
		}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
		ntNet_152_SaveData.c_pay_ryo = PayData.credit.pay_ryo;
		memcpy(&ntNet_152_SaveData.c_Card_No[0], &PayData.credit.card_no[0], sizeof(ntNet_152_SaveData.c_Card_No));
// GG120600(S) // Phase9 �ÓI��͎w�E�C��
//		memcpy(&ntNet_152_SaveData.c_cct_num[0], &PayData.credit.CCT_Num[0], sizeof(ntNet_152_SaveData.c_cct_num));
		memset(&ntNet_152_SaveData.c_cct_num[0], 0x20, sizeof(ntNet_152_SaveData.c_cct_num));
		memcpy(&ntNet_152_SaveData.c_cct_num[0], &PayData.credit.CCT_Num[0], sizeof(ntNet_152_SaveData.c_cct_num) < sizeof(PayData.credit.CCT_Num) ? sizeof(ntNet_152_SaveData.c_cct_num) : sizeof(PayData.credit.CCT_Num));
// GG120600(E) // Phase9 �ÓI��͎w�E�C��
		memcpy(&ntNet_152_SaveData.c_kid_code[0], &PayData.credit.kid_code[0], sizeof(ntNet_152_SaveData.c_kid_code));
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		ntNet_152_SaveData.c_app_no = PayData.credit.app_no;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		memcpy(&ntNet_152_SaveData.c_trade_no[0], &PayData.credit.ShopAccountNo, sizeof(ntNet_152_SaveData.c_trade_no));
		ntNet_152_SaveData.c_slipNo = PayData.credit.slip_no;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		// ����X�e�[�^�X
		if (PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
			ntNet_152_SaveData.e_Transactiontatus = 3;	// �����x���s��
		}
		else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			ntNet_152_SaveData.e_Transactiontatus = 2;	// �����x���ς�
		}
		else if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			ntNet_152_SaveData.e_Transactiontatus = 1;	// �݂Ȃ�����
		}
		else {
			ntNet_152_SaveData.e_Transactiontatus = 0;	// �x����
		}

		if (PayData.credit.pay_ryo != 0) {
			// ���F�ԍ�
			ntNet_152_SaveData.c_app_no = PayData.credit.app_no;
		}
		else {
			// �₢���킹�ԍ��^����ԍ��iQR�j
			memcpy(ntNet_152_SaveData.e_inquiry_num,
					PayData.Electron_data.Ec.inquiry_num,
					sizeof(PayData.Electron_data.Ec.inquiry_num));
			switch (PayData.Electron_data.Ec.e_pay_kind) {
			case EC_ID_USED:
				// ���F�ԍ�
				ntNet_152_SaveData.c_app_no =
						astoinl(PayData.Electron_data.Ec.Brand.Id.Approval_No, 7);
				break;
			case EC_QUIC_PAY_USED:
				// ���F�ԍ�
				ntNet_152_SaveData.c_app_no =
						astoinl(PayData.Electron_data.Ec.Brand.Quickpay.Approval_No, 7);
				break;
			case EC_PITAPA_USED:
				// ���F�ԍ�
				// �I�t���C�����F���ώ��̓I�[��'*'�̂��߁A'0'�֕ϊ�����
				if (PayData.Electron_data.Ec.Brand.Pitapa.Approval_No[0] == '*') {
					ntNet_152_SaveData.c_app_no = 0;
				}
				else {
					ntNet_152_SaveData.c_app_no =
							astoinl(PayData.Electron_data.Ec.Brand.Pitapa.Approval_No, 8);
				}
				break;
			case EC_QR_USED:
				// Mch����ԍ�
				memcpy(ntNet_152_SaveData.qr_MchTradeNo,
						PayData.Electron_data.Ec.Brand.Qr.MchTradeNo,
						sizeof(ntNet_152_SaveData.qr_MchTradeNo));
				// �x���[��ID
				memcpy(ntNet_152_SaveData.qr_PayTermID,
						PayData.Electron_data.Ec.Brand.Qr.PayTerminalNo,
						sizeof(ntNet_152_SaveData.qr_PayTermID));
				// ���σu�����h
				ntNet_152_SaveData.qr_PayKind =
						PayData.Electron_data.Ec.Brand.Qr.PayKind;
				break;
			default:
				break;
			}
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		for(i=0; i<WTIK_USEMAX; i++){
			ntNet_152_SaveData.DiscountData[i].ParkingNo = PayData.DiscountData[i].ParkingNo;
			ntNet_152_SaveData.DiscountData[i].DiscSyu = PayData.DiscountData[i].DiscSyu;
			ntNet_152_SaveData.DiscountData[i].DiscNo = PayData.DiscountData[i].DiscNo;
			ntNet_152_SaveData.DiscountData[i].DiscCount = PayData.DiscountData[i].DiscCount;
			ntNet_152_SaveData.DiscountData[i].Discount = PayData.DiscountData[i].Discount;
			ntNet_152_SaveData.DiscountData[i].DiscInfo1 = PayData.DiscountData[i].DiscInfo1;
			ntNet_152_SaveData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.DiscountData[i].uDiscData.common.DiscInfo2;
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			ntNet_152_SaveData.DiscountData[i].uDiscData.common.PrevDiscount =
				PayData.DiscountData[i].uDiscData.common.PrevDiscount;
			ntNet_152_SaveData.DiscountData[i].uDiscData.common.PrevUsageDiscount =
				PayData.DiscountData[i].uDiscData.common.PrevUsageDiscount;
			ntNet_152_SaveData.DiscountData[i].uDiscData.common.PrevUsageDiscCount =
				PayData.DiscountData[i].uDiscData.common.PrevUsageDiscCount;
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
		}
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		for(i=0; i<DETAIL_SYU_MAX; i++){
			ntNet_152_SaveData.DetailData[i].ParkingNo = PayData.DetailData[i].ParkingNo;
			ntNet_152_SaveData.DetailData[i].DiscSyu = PayData.DetailData[i].DiscSyu;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscNo = PayData.DetailData[i].uDetail.Common.DiscNo;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscCount = PayData.DetailData[i].uDetail.Common.DiscCount;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.Discount = PayData.DetailData[i].uDetail.Common.Discount;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscInfo1 = PayData.DetailData[i].uDetail.Common.DiscInfo1;
			ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscInfo2 = PayData.DetailData[i].uDetail.Common.DiscInfo2;
		}
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
	}
}

// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
///*[]----------------------------------------------------------------------[]*
// *| �ڼޯ�:���M�d��(�J�Ǻ����:��ײ�ý�)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_01( uchar ReSend )
//{
//	uchar	ret = 0;
//	struct	clk_rec	clk_tmp;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_01 ) );
//
//	/****************************************************/
//	/*	��{�ް��쐬									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// �ް��ێ��׸� = 1�Œ�
//
//	/****************************************************/
//	/*	�ڼޯċ����ް��쐬								*/
//	/****************************************************/
//	SendNtnetDt.SData136_01.Common.FmtRev		= 10;					// ̫�ϯ�Rev.��
//	SendNtnetDt.SData136_01.Common.DestInfo		= 99;					// ���M����
//	SendNtnetDt.SData136_01.Common.DataIdCode1	= 1;					// �d�����ʃR�[�h�@
//	SendNtnetDt.SData136_01.Common.DataIdCode2	= 0;					// �d�����ʃR�[�h�A
//
//	/****************************************************/
//	/*	�����ǂ��ԕҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_01.SeqNo	= creSeqNo_Get();					// ���݂̏����ǂ��Ԏ擾
//	cre_ctl.Save.SeqNo				= creSeqNo_Get();					// �đ��p�ɑޔ�
//	creSeqNo_Count();													// ���̃f�[�^�p�ɏ����ǂ��ԁ{�P
//
//	/****************************************************/
//	/*	�����N�����ҏW									*/
//	/****************************************************/
//	clk_tmp = CLK_REC;
//	SendNtnetDt.SData136_01.Proc.Year	= clk_tmp.year;					// �����N
//	SendNtnetDt.SData136_01.Proc.Mon	= clk_tmp.mont;					// ������
//	SendNtnetDt.SData136_01.Proc.Day	= clk_tmp.date;					// ������
//	SendNtnetDt.SData136_01.Proc.Hour	= clk_tmp.hour;					// ������
//	SendNtnetDt.SData136_01.Proc.Min	= clk_tmp.minu;					// ������
//	SendNtnetDt.SData136_01.Proc.Sec	= (ushort)clk_tmp.seco;			// �����b
//
//	/****************************************************/
//	/*	�ޔ��G���A�i�����f�[�^�`�F�b�N�p�j				*/
//	/****************************************************/
//	cre_ctl.Save.Date.Year	= clk_tmp.year;								// �����N
//	cre_ctl.Save.Date.Mon	= clk_tmp.mont;								// ������
//	cre_ctl.Save.Date.Day	= clk_tmp.date;								// ������
//	cre_ctl.Save.Date.Hour	= clk_tmp.hour;								// ������
//	cre_ctl.Save.Date.Min	= clk_tmp.minu;								// ������
//	cre_ctl.Save.Date.Sec	= (ushort)clk_tmp.seco;						// �����b
//
//	/****************************************************/
//	/*	�ް����M�o�^									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_01) );
//		ret = 1;
//	}
//
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]*
// *| �ڼޯ�:���M�d��(�ڼޯė^�M�⍇���ް�)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_03( void )
//{
//	uchar	ret = 0;
//	CARDDAT	t_Card;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_03 ) );
//
//	/****************************************************/
//	/*	��{�ް��쐬									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// �ް��ێ��׸� = 1�Œ�
//
//	/****************************************************/
//	/*	�ڼޯċ����ް��쐬								*/
//	/****************************************************/
//	SendNtnetDt.SData136_03.Common.FmtRev		= 10;					// ̫�ϯ�Rev.��
//	SendNtnetDt.SData136_03.Common.DestInfo		= 99;					// ���M����
//	SendNtnetDt.SData136_03.Common.DataIdCode1	= 3;					// �d�����ʃR�[�h�@
//	SendNtnetDt.SData136_03.Common.DataIdCode2	= 0;					// �d�����ʃR�[�h�A
//
//	/****************************************************/
//	/*	�����ǂ��ԕҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_03.SeqNo		= creSeqNo_Get();				// ���݂̏����ǂ��Ԏ擾
//	cre_ctl.Save.SeqNo					= creSeqNo_Get();				// �ڼޯĔ���˗��ް��p�ɑޔ��i03�͍đ����Ȃ��j
//	creSeqNo_Count();													// ���̃f�[�^�p�ɏ����ǂ��ԁ{�P
//
//	/****************************************************/
//	/*	���Z�N�����ҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_03.Pay.Year	= cre_ctl.PayStartTime.Year;	// ���Z�J�n�N
//	SendNtnetDt.SData136_03.Pay.Mon		= cre_ctl.PayStartTime.Mon;		// ���Z�J�n��
//	SendNtnetDt.SData136_03.Pay.Day		= cre_ctl.PayStartTime.Day; 	// ���Z�J�n��
//	SendNtnetDt.SData136_03.Pay.Hour	= cre_ctl.PayStartTime.Hour;	// ���Z�J�n��
//	SendNtnetDt.SData136_03.Pay.Min		= cre_ctl.PayStartTime.Min;		// ���Z�J�n��
//	SendNtnetDt.SData136_03.Pay.Sec		= cre_ctl.PayStartTime.Sec; 	// ���Z�J�n�b
//
//	/****************************************************/
//	/*	�ް��ڍוҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_03.SlipNo		= creSlipNo_Count();			// �[�������ʔԁi�`�[���j
//	SendNtnetDt.SData136_03.Amount		= (ulong)creSeisanInfo.amount;	// ������z
//	memset( &SendNtnetDt.SData136_03.AppNo[0],
//		0x30, sizeof(SendNtnetDt.SData136_03.AppNo) );					// ���F�ԍ�
//	SendNtnetDt.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;		// �Í�������
//	SendNtnetDt.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;		// �Í����ԍ�
//	memset( &t_Card, 0, sizeof( t_Card ) );
//	memset( &t_Card.JIS2Data[0], 0x20, sizeof( t_Card.JIS2Data ) );		// JIS2�����ް�
//	memcpy( &t_Card.JIS2Data[0], &creSeisanInfo.jis_2[0], sizeof( t_Card.JIS2Data ) );
//	memset( &t_Card.JIS1Data[0], 0x20, sizeof( t_Card.JIS1Data ) );		// JIS1�����ް�
//	memcpy( &t_Card.JIS1Data[0], &creSeisanInfo.jis_1[0], sizeof( t_Card.JIS1Data ) );
//	t_Card.PayMethod	= 0;											// �x�����@�i���Ή��j
//	t_Card.DivCount		= 0;											// �����񐔁i���Ή��j
//	AesCBCEncrypt( (uchar *)&t_Card, sizeof( t_Card ) );				// AES �Í���
//	memcpy( (uchar *)&SendNtnetDt.SData136_03.Crypt,
//		(uchar *)&t_Card, sizeof( SendNtnetDt.SData136_03.Crypt ) );	// �ڼޯď��
//
//	/****************************************************/
//	/*	�ޔ��G���A�i�����f�[�^�`�F�b�N�p�j				*/
//	/****************************************************/
//	cre_ctl.Save.Date.Year	= cre_ctl.PayStartTime.Year;				// ���Z�J�n�N
//	cre_ctl.Save.Date.Mon	= cre_ctl.PayStartTime.Mon;					// ���Z�J�n��
//	cre_ctl.Save.Date.Day	= cre_ctl.PayStartTime.Day; 				// ���Z�J�n��
//	cre_ctl.Save.Date.Hour	= cre_ctl.PayStartTime.Hour;				// ���Z�J�n��
//	cre_ctl.Save.Date.Min	= cre_ctl.PayStartTime.Min;					// ���Z�J�n��
//	cre_ctl.Save.Date.Sec	= cre_ctl.PayStartTime.Sec; 				// ���Z�J�n�b
//	cre_ctl.Save.SlipNo		= creSlipNo_Get();;							// �[�������ʔԁi�`�[���j
//	cre_ctl.Save.Amount		= (ulong)creSeisanInfo.amount;				// ������z
//
//	/****************************************************/
//	/*	�ް����M�o�^									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_03) );
//		ret = 1;
//	}
//
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]*
// *| �ڼޯ�:���M�d��(�ڼޯĔ���˗��ް�)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_05( uchar ReSend )
//{
//	uchar	ret = 0;
//	t_MediaInfoCre	t_Card;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_05 ) );
//
//	/****************************************************/
//	/*	��{�ް��쐬									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// �ް��ێ��׸� = 1�Œ�
//
//	/****************************************************/
//	/*	�ڼޯċ����ް��쐬								*/
//	/****************************************************/
//	SendNtnetDt.SData136_05.Common.FmtRev		= 11;					// ̫�ϯ�Rev.��
//	SendNtnetDt.SData136_05.Common.DestInfo		= 99;					// ���M����
//	SendNtnetDt.SData136_05.Common.DataIdCode1	= 5;					// �d�����ʃR�[�h�@
//	SendNtnetDt.SData136_05.Common.DataIdCode2	= 0;					// �d�����ʃR�[�h�A
//
//	/****************************************************/
//	/*	�����ǂ��ԕҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_05.SeqNo	= creSeqNo_Get();					// ���݂̏����ǂ��Ԏ擾
//	cre_ctl.Save.SeqNo				= creSeqNo_Get();					// �đ��p�ɑޔ�
//	creSeqNo_Count();													// ���̃f�[�^�p�ɏ����ǂ��ԁ{�P
//
//	/****************************************************/
//	/*	���Z�N�����ҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_05.Pay.Year	= cre_uriage.back.time.Year;	// ���Z�N
//	SendNtnetDt.SData136_05.Pay.Mon		= cre_uriage.back.time.Mon;		// ���Z��
//	SendNtnetDt.SData136_05.Pay.Day		= cre_uriage.back.time.Day;		// ���Z��
//	SendNtnetDt.SData136_05.Pay.Hour	= cre_uriage.back.time.Hour;	// ���Z��
//	SendNtnetDt.SData136_05.Pay.Min		= cre_uriage.back.time.Min;		// ���Z��
//	SendNtnetDt.SData136_05.Pay.Sec		= 0;							// ���Z�b
//
//	/****************************************************/
//	/*	�ް��ڍוҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_05.SlipNo		= cre_uriage.back.slip_no;		// �[�������ʔԁi�`�[���j
//	SendNtnetDt.SData136_05.Amount		= cre_uriage.back.ryo;			// ������z
//	memcpy( &SendNtnetDt.SData136_05.AppNo[0],							// ���F�ԍ�
//		&cre_uriage.back.AppNoChar[0], sizeof(SendNtnetDt.SData136_05.AppNo) );
//	memcpy( &SendNtnetDt.SData136_05.ShopAccountNo[0],					// �����X����ԍ�
//		&cre_uriage.back.shop_account_no[0], sizeof(SendNtnetDt.SData136_05.ShopAccountNo) );
//	SendNtnetDt.SData136_05.PayMethod	= 0;							// �x�����@�i���Ή��j
//	SendNtnetDt.SData136_05.DivCount	= 0;							// �����񐔁i���Ή��j
//	SendNtnetDt.SData136_05.CMachineNo  = cre_uriage.back.CMachineNo;	// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
//	SendNtnetDt.SData136_05.PayMethod2  = cre_uriage.back.PayMethod2;	// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
//	SendNtnetDt.SData136_05.PayClass    = cre_uriage.back.PayClass;		// �����敪(0�����Z/1���Đ��Z/2�����Z���~
//	SendNtnetDt.SData136_05.PayMode     = cre_uriage.back.PayMode;		// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z)
//	SendNtnetDt.SData136_05.FlapArea    = cre_uriage.back.FlapArea;		// �t���b�v�V�X�e��	���		0�`99
//	SendNtnetDt.SData136_05.FlapParkNo  = cre_uriage.back.FlapParkNo;	// 					�Ԏ��ԍ�	0�`9999
//	SendNtnetDt.SData136_05.Price       = cre_uriage.back.Price;		// ���ԗ���		0�`
//	SendNtnetDt.SData136_05.OptionPrice = cre_uriage.back.OptionPrice;	// ���̑�����	0�`
//	SendNtnetDt.SData136_05.Discount    = cre_uriage.back.Discount;		// �����z(���Z)
//	SendNtnetDt.SData136_05.CashPrice   = cre_uriage.back.CashPrice;	// ��������		0�`
//	SendNtnetDt.SData136_05.PayCount    = cre_uriage.back.PayCount;		// ���Z�ǔ�(0�`99999)
//	// ���Z�}�̏��1
//	SendNtnetDt.SData136_05.MediaKind = cre_uriage.back.MediaKind;					// ���(�}��)	0�`99
//	if(cre_uriage.back.MediaKind != 0){// �Í����͎�ʂ��L���Ȏ��̂ݍs�Ȃ�
//		SendNtnetDt.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;				// �Í�������
//		SendNtnetDt.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;				// �Í����ԍ�
//		memset( &t_Card, 0, sizeof( t_Card ) );
//		memcpy( &t_Card.MediaCardNo[0], &cre_uriage.back.Media.MediaCardNo[0], sizeof( t_Card.MediaCardNo ) );// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		memcpy( &t_Card.MediaCardInfo[0], &cre_uriage.back.Media.MediaCardInfo[0], sizeof( t_Card.MediaCardInfo ) );// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		AesCBCEncrypt( (uchar *)&t_Card, sizeof( t_Card ) );						// AES �Í���
//		memcpy( (uchar *)&SendNtnetDt.SData136_05.Media,
//			(uchar *)&t_Card, sizeof( SendNtnetDt.SData136_05.Media ) );
//	}
//
//	/****************************************************/
//	/*	�ޔ��G���A�i�����f�[�^�`�F�b�N�p�j				*/
//	/****************************************************/
//	cre_ctl.Save.Date.Year	= cre_uriage.back.time.Year;				// ���Z�N
//	cre_ctl.Save.Date.Mon	= cre_uriage.back.time.Mon;					// ���Z��
//	cre_ctl.Save.Date.Day	= cre_uriage.back.time.Day;					// ���Z��
//	cre_ctl.Save.Date.Hour	= cre_uriage.back.time.Hour;				// ���Z��
//	cre_ctl.Save.Date.Min	= cre_uriage.back.time.Min;					// ���Z��
//	cre_ctl.Save.Date.Sec	= 0;										// ���Z�b
//	cre_ctl.Save.SlipNo		= cre_uriage.back.slip_no;					// �[�������ʔԁi�`�[���j
//	cre_ctl.Save.Amount		= cre_uriage.back.ryo;						// ������z
//	memcpy( &cre_ctl.Save.AppNo[0], &cre_uriage.back.AppNoChar[0],
//								sizeof(cre_ctl.Save.AppNo) );			// ���F�ԍ�
//	memcpy( &cre_ctl.Save.ShopAccountNo[0], &cre_uriage.back.shop_account_no[0],
//								sizeof(cre_ctl.Save.ShopAccountNo) );	// �����X����ԍ�
//
//	/****************************************************/
//	/*	�ް����M�o�^									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_05) );
//		ret = 1;
//	}
//
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]*
// *| �ڼޯ�:���M�d��(ýĺ����)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_07( uchar ReSend )
//{
//	uchar	ret = 0;
//	uchar	test_kind = 0;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_07 ) );
//
//	/****************************************************/
//	/*	��{�ް��쐬									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// �ް��ێ��׸� = 1�Œ�
//
//	/****************************************************/
//	/*	�ڼޯċ����ް��쐬								*/
//	/****************************************************/
//	SendNtnetDt.SData136_07.Common.FmtRev		= 10;					// ̫�ϯ�Rev.��
//	SendNtnetDt.SData136_07.Common.DestInfo		= 99;					// ���M����
//	SendNtnetDt.SData136_07.Common.DataIdCode1	= 7;					// �d�����ʃR�[�h�@
//	SendNtnetDt.SData136_07.Common.DataIdCode2	= 0;					// �d�����ʃR�[�h�A
//
//	/****************************************************/
//	/*	�����ǂ��ԕҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_07.SeqNo	= creSeqNo_Get();					// ���݂̏����ǂ��Ԏ擾
//	cre_ctl.Save.SeqNo				= creSeqNo_Get();					// �đ��p�ɑޔ�
//	creSeqNo_Count();													// ���̃f�[�^�p�ɏ����ǂ��ԁ{�P
//
//	/****************************************************/
//	/*	�ް��ڍוҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_07.TestKind	= test_kind;					// ýĎ�� 0=�����m�F
//
//	/****************************************************/
//	/*	�ޔ��G���A�i�����f�[�^�`�F�b�N�p�j				*/
//	/****************************************************/
//	cre_ctl.Save.TestKind	= test_kind;								// ýĎ��
//
//	/****************************************************/
//	/*	�ް����M�o�^									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_07) );
//		ret = 1;
//	}
//
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]*
// *| �ڼޯ�:���M�d��(�ڼޯĕԕi�⍇���ް�)
// *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//uchar	NTNET_Snd_Data136_09( uchar ReSend )
//{
//	uchar	ret = 0;
//	struct	clk_rec	clk_tmp;
//
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_136_09 ) );
//
//	/****************************************************/
//	/*	��{�ް��쐬									*/
//	/****************************************************/
//	BasicDataMake( 136, 1 );											// �ް��ێ��׸� = 1�Œ�
//
//	/****************************************************/
//	/*	�ڼޯċ����ް��쐬								*/
//	/****************************************************/
//	SendNtnetDt.SData136_09.Common.FmtRev		= 10;					// ̫�ϯ�Rev.��
//	SendNtnetDt.SData136_09.Common.DestInfo		= 99;					// ���M����
//	SendNtnetDt.SData136_09.Common.DataIdCode1	= 9;					// �d�����ʃR�[�h�@
//	SendNtnetDt.SData136_09.Common.DataIdCode2	= 0;					// �d�����ʃR�[�h�A
//
//	/****************************************************/
//	/*	�����ǂ��ԕҏW									*/
//	/****************************************************/
//	SendNtnetDt.SData136_09.SeqNo	= creSeqNo_Get();					// ���݂̏����ǂ��Ԏ擾
//	cre_ctl.Save.SeqNo				= creSeqNo_Get();					// �đ��p�ɑޔ�
//	creSeqNo_Count();													// ���̃f�[�^�p�ɏ����ǂ��ԁ{�P
//
//	/****************************************************/
//	/*	�����N�����ҏW									*/
//	/****************************************************/
//	clk_tmp = CLK_REC;
//	SendNtnetDt.SData136_09.Proc.Year	= clk_tmp.year;					// �����N
//	SendNtnetDt.SData136_09.Proc.Mon	= clk_tmp.mont;					// ������
//	SendNtnetDt.SData136_09.Proc.Day	= clk_tmp.date;					// ������
//	SendNtnetDt.SData136_09.Proc.Hour	= clk_tmp.hour;					// ������
//	SendNtnetDt.SData136_09.Proc.Min	= clk_tmp.minu;					// ������
//	SendNtnetDt.SData136_09.Proc.Sec	= (ushort)clk_tmp.seco;			// �����b
//
//	/****************************************************/
//	/*	�ް��ڍוҏW									*/
//	/****************************************************/
//// TODO: SlipNo ���J�E���g����^���Ȃ��͗v�m�F
////	SendNtnetDt.SData136_09.SlipNo		= creSlipNo_Get();				// �[�������ʔԁi�`�[���j
//	SendNtnetDt.SData136_09.SlipNo		= creSlipNo_Count();			// �[�������ʔԁi�`�[���j
//	memset( &SendNtnetDt.SData136_09.AppNo[0],
//		0x30, sizeof(SendNtnetDt.SData136_09.AppNo) );					// �ԕi�Ώ� ���F�ԍ��i���Ή��j
//	memset( &SendNtnetDt.SData136_09.ShopAccountNo[0],
//		0x30, sizeof(SendNtnetDt.SData136_09.ShopAccountNo) );			// �ԕi�Ώ� �����X����ԍ��i���Ή��j
//
//	/****************************************************/
//	/*	�ޔ��G���A�i�����f�[�^�`�F�b�N�p�j				*/
//	/****************************************************/
//	cre_ctl.Save.Date.Year	= clk_tmp.year;								// �����N
//	cre_ctl.Save.Date.Mon	= clk_tmp.mont;								// ������
//	cre_ctl.Save.Date.Day	= clk_tmp.date;								// ������
//	cre_ctl.Save.Date.Hour	= clk_tmp.hour;								// ������
//	cre_ctl.Save.Date.Min	= clk_tmp.minu;								// ������
//	cre_ctl.Save.Date.Sec	= (ushort)clk_tmp.seco;						// �����b
//	cre_ctl.Save.SlipNo		= creSlipNo_Get();;							// �[�������ʔԁi�`�[���j
//	memset( &cre_ctl.Save.AppNo[0], 0x30, 6 );							// ���F�ԍ��i���Ή��j
//	memset( &cre_ctl.Save.ShopAccountNo[0], 0x30, 20 );					// �����X����ԍ��i���Ή��j
//
//	/****************************************************/
//	/*	�ް����M�o�^									*/
//	/****************************************************/
//	if( RAU_Credit_Enabale != 0 ) {
//		Cre_SetSendNtData( (const uchar*)&SendNtnetDt, sizeof(DATA_KIND_136_09) );
//		ret = 1;
//	}
//
//	return ret;
//}
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)

//[]----------------------------------------------------------------------[]
///	@brief		NT-NET�f�[�^�ɐݒ肷��V�[�P���V����No.���擾����
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		�V�[�P���V����No.
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
uchar	GetNtDataSeqNo(void)
{
	uchar	seqNo = basicSeqNo;
	
	++basicSeqNo;
	if(basicSeqNo > 99) {
		basicSeqNo = 1;
	}
	
	return seqNo;
}	

void NTNET_CtrlRecvData_DL(void)
{
	switch (RecvNtnetDt.DataBasic.DataKind) {
// GG120600(S) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�
//	case 111:												// ���u�����e�i���X�v��
	case REMOTE_MNT_REQ:									// ���u�����e�i���X�v��
// GG120600(E) // Phase9 �Z���^�[�p�[�����f�[�^�Ɏ�M�d�����r�W������ǉ�
		NTNET_RevData111_DL();
		break;
	default:
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���u�����e�i���X�v��(�ް����1)��M����                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_RevData111_DL                                     |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai                                                 |*/
/*| Date         : 2014-12-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
void NTNET_RevData111_DL(void)
{
	DATA_KIND_111_DL *p = &RecvNtnetDt.RData111_DL;
	ushort len1 = sizeof(*p) - offsetof(DATA_KIND_111_DL, ProgDlReq);	// ���ߔԍ�1�`�\���̒���
	ushort len2 = sizeof(*p) - offsetof(DATA_KIND_111_DL, FtpInfo);		// FTP �T�[�oIP�`�\���̒���

	// ���M��^�[�~�i�����`�F�b�N
// GG120600(S) // Phase9 #6219 �y�A���]���w�E�z�Z���^�[�ɓo�^����Ă���GT-4100�̃^�[�~�i��No.�ƒ[���̋@�BNo.���قȂ�ꍇ�ɐݒ�A�b�v���[�h�Ɏ��s����
//	if ((p->SMachineNo == CPrmSS[S_PAY][2])) {
	if ((p->SMachineNo == CPrmSS[S_NTN][3])) {
// GG120600(E) // Phase9 #6219 �y�A���]���w�E�z�Z���^�[�ɓo�^����Ă���GT-4100�̃^�[�~�i��No.�ƒ[���̋@�BNo.���قȂ�ꍇ�ɐݒ�A�b�v���[�h�Ɏ��s����
		// �Í�������Ă��邩�H
		if (p->DataBasic.CMN_DT.DT_BASIC.encryptMode) {
			AesCBCDecrypt((uchar *)&p->FtpInfo, sizeof(p->FtpInfo));
		}
		remotedl_ftp_info_set(&p->FtpInfo);
		queset( OPETCBNO, REMOTE_DL_REQ, len1 - len2, &p->ProgDlReq );
	}
//	else {
//		// �p�����[�^NG
//		p->ProgDlReq.ReqResult = REQ_NOT_ACCEPT;
//		NTNET_Snd_Data118_DL(&p->ProgDlReq);
//	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���u�����e�i���X�v������(�ް����2)���M����                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Snd_Data118_DL                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai                                                 |*/
/*| Date         : 2014-12-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
void NTNET_Snd_Data118_DL(t_ProgDlReq *pDlReq)
{
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_118_DL ) );

	BasicDataMake( 118, 1 );								// ��{�ް��쐬
	SendNtnetDt.DataBasic.SystemID = REMOTE_DL_SYSTEM;		// �V�X�e��ID
	SendNtnetDt.SData118_DL.FmtRev = 10;					// �t�H�[�}�b�gRev.��
// GG120600(S) // Phase9 #6219 �y�A���]���w�E�z�Z���^�[�ɓo�^����Ă���GT-4100�̃^�[�~�i��No.�ƒ[���̋@�BNo.���قȂ�ꍇ�ɐݒ�A�b�v���[�h�Ɏ��s����
//	SendNtnetDt.SData118_DL.SMachineNo = CPrmSS[S_PAY][2];	// ���M���^�[�~�i����
	SendNtnetDt.SData118_DL.SMachineNo = CPrmSS[S_NTN][3];	// ���M���^�[�~�i����
// GG120600(E) // Phase9 #6219 �y�A���]���w�E�z�Z���^�[�ɓo�^����Ă���GT-4100�̃^�[�~�i��No.�ƒ[���̋@�BNo.���قȂ�ꍇ�ɐݒ�A�b�v���[�h�Ɏ��s����
	memcpy(&SendNtnetDt.SData118_DL.ProgDlReq, pDlReq, sizeof(*pDlReq));

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_118_DL ));
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ���u�Ď��f�[�^(�ް����61)�ҏW����                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : NTNET_Edit_Data125_DL                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai                                                 |*/
/*| Date         : 2014-12-22                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
unsigned short NTNET_Edit_Data125_DL(Rmon_log *p_RcptDat, DATA_KIND_125_DL *p_NtDat)
{
	int len;

	memset( p_NtDat, 0, sizeof( DATA_KIND_125_DL ) );
	NTNET_Edit_BasicData( 125, 0, p_RcptDat->RmonSeqNo, &p_NtDat->DataBasic);		// ��{�ް��쐬
	p_NtDat->DataBasic.SystemID = REMOTE_DL_SYSTEM;									// �V�X�e��ID

	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����O�̎������Z�b�g����̂ōăZ�b�g
	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->Date_Time.Year % 100 );			// �����N
	p_NtDat->DataBasic.Mon = (uchar)p_RcptDat->Date_Time.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)p_RcptDat->Date_Time.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)p_RcptDat->Date_Time.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)p_RcptDat->Date_Time.Min;						// ������
	p_NtDat->DataBasic.Sec = (uchar)p_RcptDat->Date_Time.Sec;						// �����b

// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//	p_NtDat->FmtRev = 10;															// ̫�ϯ�Rev.��
	p_NtDat->FmtRev = 11;															// ̫�ϯ�Rev.��
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
	p_NtDat->SMachineNo = REMOTE_PC_TERMINAL_NO;									// ���M���^�[�~�i����
	p_NtDat->InstNo1 = p_RcptDat->RmonInstNo.ulinstNo[0];							// ���ߔԍ�1
	p_NtDat->InstNo2 = p_RcptDat->RmonInstNo.ulinstNo[1];							// ���ߔԍ�2
	p_NtDat->FuncNo = p_RcptDat->RmonFuncNo;										// �[���@�\�ԍ�
	p_NtDat->ProcNo = p_RcptDat->RmonProcNo;										// �����ԍ�
	p_NtDat->ProcInfoNo = p_RcptDat->RmonProcInfoNo;								// �������ԍ�
	p_NtDat->StatusNo = p_RcptDat->RmonStatusNo;									// �󋵔ԍ�
	p_NtDat->StatusDetailNo = p_RcptDat->RmonStatusDetailNo;						// �󋵏ڍהԍ�
	p_NtDat->MonitorInfo = p_RcptDat->RmonInfo;										// �[���Ď����
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
	p_NtDat->RmonFrom = p_RcptDat->RmonFrom;										// ���ߗv����0=�Z���^�[/1=�[��
	p_NtDat->RmonUploadReq = p_RcptDat->RmonUploadReq;								// �ݒ�A�b�v���[�h�v��
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

	if (p_RcptDat->RmonStartTime1.Year) {
		p_NtDat->Start_Year1 = p_RcptDat->RmonStartTime1.Year % 100;				// �\��J�n����1
		memcpy(&p_NtDat->Start_Mon1, &p_RcptDat->RmonStartTime1.Mon, 4);
	}
	if (p_RcptDat->RmonStartTime2.Year) {
		p_NtDat->Start_Year2 = p_RcptDat->RmonStartTime2.Year % 100;				// �\��J�n����2
		memcpy(&p_NtDat->Start_Mon2, &p_RcptDat->RmonStartTime2.Mon, 4);
	}
	len = strlen((char *)p_RcptDat->RmonProgNo);
	if (len > 0) {
		memcpy(p_NtDat->Prog_No, p_RcptDat->RmonProgNo, len-sizeof(p_NtDat->Prog_Ver));		// �v���O��������
		memcpy(p_NtDat->Prog_Ver, &p_RcptDat->RmonProgNo[len-sizeof(p_NtDat->Prog_Ver)], sizeof(p_NtDat->Prog_Ver));	// �v���O�����o�[�W����
	}

	return sizeof( DATA_KIND_125_DL );
}
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
///*[]----------------------------------------------------------------------[]*
// *| Park I Pro�d����M����
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevDataPiP
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static void	NTNET_RevDataPiP(void)
{
	// ParkingWeb�o�RParkiPro�ݒ�Ȃ��H
	if( !_is_ParkingWeb_pip() ){
		return;
	}

	// �f�[�^���
	switch( RecvNtnetDt.DataBasic.DataKind )
	{
		// �̎��؍Ĕ��s�v��
		case PIP_REQ_KIND_RECEIPT_AGAIN:
			NTNET_RevData16_01_ReceiptAgain();
			break;

// MH810100(S) K.Onodera 2019/10/25 �Ԕԃ`�P�b�g���X�iPark i PRO�Ή��j
//		// �U�֐��Z�v��
//		case PIP_REQ_KIND_FURIKAE:
//			NTNET_RevData16_03_Furikae();
//			break;
//
//		// ��t�����s�v��
//		case PIP_REQ_KIND_RECEIVE_TKT:
//			NTNET_RevData16_05_ReceiveTkt();
//			break;
// MH810100(E) K.Onodera 2019/10/25 �Ԕԃ`�P�b�g���X�iPark i PRO�Ή��j

		// ���u���Z�v��
		case PIP_REQ_KIND_REMOTE_CALC:
			NTNET_RevData16_08_RemoteCalc();
			break;

		// �������G���[�A���[�����v��
		case PIP_REQ_KIND_OCCUR_ERRALM:
			NTNET_RevData16_10_ErrorAlarm();
			break;

		default:
			break;
	}
}

static ushort PIP_CheckCarInfo(uchar area,ulong no, ushort* pos)
{
	ushort	index;

	if ((area != 0) ||						// ��FT-4800�ł�0�Œ�
		((no <= 0)  || (no > 9900)))
		return PIP_RES_RESULT_NG_PARAM;

	if (LKopeGetLockNum(area, no, pos) == 0)
		return PIP_RES_RESULT_NG_NO_CAR;

	index = *pos - 1;
	if (LockInfo[index].lok_syu == 0 || LockInfo[index].ryo_syu == 0)
		return PIP_RES_RESULT_NG_NO_CAR;

	return PIP_RES_RESULT_OK;
}

// ���r���[�w�E�Ή�(S) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
static	Receipt_data	wkReceipt;
// ���r���[�w�E�Ή�(E) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����

///*[]----------------------------------------------------------------------[]*
// *| ��t�����s����
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData16_05_ReceiveTkt
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:���[�j���O�΍�)
//static void NTNET_RevData16_05_ReceiveTkt(void)
void NTNET_RevData16_05_ReceiveTkt(void)
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:���[�j���O�΍�)
{
	ushort rslt = PIP_RES_RESULT_NG_DENY;
	ushort pos = 0;
	ushort index = 0;
	ushort type = 0;
// ���r���[�w�E�Ή�(S) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
	uchar  permit = 0;
// ���r���[�w�E�Ή�(E) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����

	// ��t���@�\�Ȃ��H
	if ( prm_get( COM_PRM,S_TYP,62,1,1 ) == 0 ) {
		rslt = PIP_RES_RESULT_NG_DENY;					// ��t����
	}
	// �����W�v���H
	else if( auto_syu_prn == 2 ){
		rslt = PIP_RES_RESULT_NG_DENY;					// ��t����
	}
	else {
		rslt = PIP_CheckCarInfo(RecvNtnetDt.RData16_05.Area,RecvNtnetDt.RData16_05.ulNo, &pos);	// �Ԏ��������
		if (rslt == PIP_RES_RESULT_OK) {
			index = pos - 1;
// ���r���[�w�E�Ή�(S) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
			memset( &wkReceipt, 0, sizeof( Receipt_data) );
			if( PiP_GetNewestPayReceiptData( &wkReceipt, RecvNtnetDt.RData16_05.Area, RecvNtnetDt.RData16_05.ulNo ) ){
				// �T�[�r�X�^�C�����o�ɁH���A���Z��(�ݎ�)
				if( wkReceipt.OutKind == 99 && (FLAPDT.flp_data[index].mode == FLAP_CTRL_MODE5 || FLAPDT.flp_data[index].mode == FLAP_CTRL_MODE6) ){
					permit = 1;
				}
			}
// ���r���[�w�E�Ή�(E) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
// ���r���[�w�E�Ή�(S) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
//			if (OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod <= 1) &&			// �ҋ@���?
//				(FLAPDT.flp_data[index].mode >= FLAP_CTRL_MODE2 && FLAPDT.flp_data[index].mode <= FLAP_CTRL_MODE4)) {	// ���Ԓ�?
			if( OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod <= 1 ){
				if( (FLAPDT.flp_data[index].mode >= FLAP_CTRL_MODE2 && FLAPDT.flp_data[index].mode <= FLAP_CTRL_MODE4) || permit ){
// ���r���[�w�E�Ή�(E) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
					// ��t�����s����
// ���r���[�w�E�Ή�(S) K.Onodera 2016/10/04 ��t���͏�������Ɋ֌W�Ȃ����s���A�Ĕ��s�Ƃ���
//					type = (FLAPDT.flp_data[index].uketuke == 0)? 0x12 : 0x11;
					type = (FLAPDT.flp_data[index].uketuke == 0)? 0x22 : 0x21;
// ���r���[�w�E�Ή�(E) K.Onodera 2016/10/04 ��t���͏�������Ɋ֌W�Ȃ����s���A�Ĕ��s�Ƃ���
					FLAPDT.flp_data[index].uketuke = 0;					// ���s�ς݃t���OOFF
					if( uke_isu((ulong)(LockInfo[index].area*10000L + LockInfo[index].posi), (ushort)(pos), type) ){
						// ���sNG
						rslt = PIP_RES_RESULT_NG_DENY;
					}else{
						wopelg( OPLOG_PARKI_UKETUKEHAKKO, 0, 0 );			// ���엚��o�^
					}
// ���r���[�w�E�Ή�(S) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
				}else{
					rslt = PIP_RES_RESULT_NG_DENY;
				}
// ���r���[�w�E�Ή�(E) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
			}else {
				rslt = PIP_RES_RESULT_NG_DENY;
			}
		}
	}
	
	// �����f�[�^���M
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_06 ) );

	BasicDataMake( PIP_RES_KIND_RECEIVE_TKT, 1 );									// ��{�ް��쐬 �ް��ێ��׸� = 1�Œ�

	SendNtnetDt.SData16_06.Common.FmtRev 		= 10;								// �t�H�[�}�b�gRev.No.(10�Œ�)
	SendNtnetDt.SData16_06.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// ���M��̒[���^�[�~�i�������Z�b�g����i99�����uPC�j
	SendNtnetDt.SData16_06.Common.CenterSeqNo	= RecvNtnetDt.RData16_05.Common.CenterSeqNo;	// �Z���^�[�ǔ�
	SendNtnetDt.SData16_06.Result				= rslt;								// ����
	SendNtnetDt.SData16_06.Area 				= RecvNtnetDt.RData16_05.Area;		// ���
	SendNtnetDt.SData16_06.ulNo 				= RecvNtnetDt.RData16_05.ulNo;		// �Ԏ��ԍ�
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_06 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_06 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
}
// ���r���[�w�E�Ή�(S) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
//static	Receipt_data	wkReceipt;
// ���r���[�w�E�Ή�(E) K.Onodera 2016/10/05 �T�[�r�X�^�C�������Z��͍ݎԂł���Ύ�t�����s����
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
static	CREINFO_r14	t_Credit;
static	EMONEY_r14	t_EMoney;
static	QRCODE_rXX	t_QR;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
///*[]----------------------------------------------------------------------[]*
// *| �̎��؍Ĕ��s����
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData16_01_ReceiptAgain
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static void NTNET_RevData16_01_ReceiptAgain( void )
{
	ushort			rslt = PIP_RES_RESULT_NG_DENY;
// MH810100(S) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
//	ushort			i, j, d;
// MH810100(E) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
	T_FrmReceipt	rec_data;			// �̎��؈󎚗v����ү�����ް�
// MH810100(S) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
//	CREINFO_r14		t_Credit;
//	EMONEY_r14		t_EMoney;
// MH810100(E) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
// �s��C��(S) K.Onodera 2016/09/27 #1504 �̎��؍Ĕ��s�����f�[�^�Łu���Z�ǔԁv�ȍ~�̓��e���Q�o�C�g����Ă���
	ushort			size = 0;
// �s��C��(E) K.Onodera 2016/09/27 #1504 �̎��؍Ĕ��s�����f�[�^�Łu���Z�ǔԁv�ȍ~�̓��e���Q�o�C�g����Ă���
// MH810100(S) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
	ushort	cnt_ttl;
	ushort	id;
	ushort	date[3];
	ushort	seisan_date, target_date;
// MH810100(E) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
// MH810100(S) 2020/08/18 �Ԕԃ`�P�b�g���X(#4599 ParkiPro�̗̎��؍Ĕ��s�ŃN���W�b�g���Z�̗̎��؂��Ĕ��s���ł��Ȃ�)
	uchar			PayClassTmp;										// �����敪
// MH810100(E) 2020/08/18 �Ԕԃ`�P�b�g���X(#4599 ParkiPro�̗̎��؍Ĕ��s�ŃN���W�b�g���Z�̗̎��؂��Ĕ��s���ł��Ȃ�)

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_02 ) );
	memset( &wkReceipt, 0, sizeof( Receipt_data) );

// MH810100(S) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
//	// ����
//	wkReceipt.WFlag			= 0;
//	wkReceipt.chuusi		= 0;
//	wkReceipt.Kikai_no		= (uchar)CPrmSS[S_PAY][2];															// ���Z�@No.
//	wkReceipt.Oiban.i		= RecvNtnetDt.RData16_01.PayCount;													// ���Z�ǔ�
//	wkReceipt.WPrice		= RecvNtnetDt.RData16_01.TotalFee;													// ���ԗ���or����X�V����
//	wkReceipt.syu			= RecvNtnetDt.RData16_01.Syubet;													// �������
//	wkReceipt.KakariNo		= RecvNtnetDt.RData16_01.KakariNo;													// �W����
//	wkReceipt.PayClass		= RecvNtnetDt.RData16_01.PayClass;													// �����敪
//	wkReceipt.ReceiptIssue	= RecvNtnetDt.RData16_01.ReceiptIssue;												// �̎������s�L��
//	wkReceipt.WInPrice		= RecvNtnetDt.RData16_01.InMoney;													// �����z
//	wkReceipt.WChgPrice		= RecvNtnetDt.RData16_01.Change;													// �ޑK�z
//	wkReceipt.WFusoku		= RecvNtnetDt.RData16_01.Fusoku;													// ���߂��s��
//	// ��������ԏꇂ�Z�b�g�H
//	if( RecvNtnetDt.RData16_01.PassInfo.ParkNo ){
//		// teiki.pkno_syu�H
//		wkReceipt.teiki.ParkingNo = RecvNtnetDt.RData16_01.PassInfo.ParkNo;
//		if( RecvNtnetDt.RData16_01.MainMedia.Kind >= 2 ){
//			wkReceipt.teiki.pkno_syu = (RecvNtnetDt.RData16_01.MainMedia.Kind - 2);
//		}else{
//			wkReceipt.teiki.pkno_syu = 0;
//		}
//		wkReceipt.teiki.id = astoinl( RecvNtnetDt.RData16_01.MainMedia.CardNo, 5 );
//		wkReceipt.teiki.syu = astoinl( RecvNtnetDt.RData16_01.MainMedia.CardInfo, 2 );
//	}
//	switch( RecvNtnetDt.RData16_01.CardPaymentKind ){			// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//		case 1:
//			memset( &t_Credit, 0, sizeof( t_Credit ) );
//			memcpy( (uchar *)&t_Credit, RecvNtnetDt.RData16_01.CardPaymentInfo, sizeof(t_Credit) );
//			// �Í�������=AES�H
//			if( RecvNtnetDt.RData16_01.DataBasic.CMN_DT.DT_BASIC.encryptMode == 1 ){
//				AesCBCDecrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );
//			}
//			wkReceipt.credit.pay_ryo = t_Credit.amount;		// ���ϊz
//			wkReceipt.credit.app_no = t_Credit.app_no;		// ���F�ԍ�
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
////// �s��C��(S) K.Onodera 2016/12/12 #1666 �N���W�b�g�J�[�h�ɂ�鐸�Z�̗̎��؍Ĕ��s�����{����ƃN���W�b�g�J�[�h���F����000000�ň󎚂����
////			intoasl( (uchar*)&wkReceipt.credit.AppNoChar[0], wkReceipt.credit.app_no, 6 );		// ���F�ԍ� ������ێ� ���l��
////// �s��C��(E) K.Onodera 2016/12/12 #1666 �N���W�b�g�J�[�h�ɂ�鐸�Z�̗̎��؍Ĕ��s�����{����ƃN���W�b�g�J�[�h���F����000000�ň󎚂����
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//			wkReceipt.credit.slip_no = t_Credit.slip_no;	// �`�[�ԍ�
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
////			memcpyFlushLeft( (uchar *)&wkReceipt.credit.card_no[0], &t_Credit.card_no[0], sizeof(wkReceipt.credit.card_no), sizeof(t_Credit.card_no) );
////			memcpyFlushLeft( (uchar *)&wkReceipt.credit.CCT_Num[0], &t_Credit.cct_num[0], sizeof(wkReceipt.credit.CCT_Num), sizeof(t_Credit.cct_num) );
////			memcpyFlushLeft( (uchar *)&wkReceipt.credit.kid_code[0], &t_Credit.kid_code[0], sizeof(wkReceipt.credit.kid_code), sizeof(t_Credit.kid_code) );
//			// ���No�̏�7���ځ`��5���ڈȊO��'0'���i�[����Ă�����'*'�ɒu��
//			change_CharInArray( (uchar *)&t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '0', '*' );
//// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
////			//�ڼޯĶ��މ���� �݂Ȃ����ώ���NULL(0x0)�Ŗ��߂�
////			memcpyFlushLeft2( (uchar *)&wkReceipt.credit.card_no[0], &t_Credit.card_no[0], sizeof(wkReceipt.credit.card_no), sizeof(t_Credit.card_no) );
////			//�ڼޯĶ��ޒ[�����ʔԍ� �݂Ȃ����ώ���NULL(0x0)�Ŗ��߂�
////			memcpyFlushLeft2( (uchar *)&wkReceipt.credit.CCT_Num[0], &t_Credit.cct_num[0], sizeof(wkReceipt.credit.CCT_Num), sizeof(t_Credit.cct_num) );
////			//KID���� �݂Ȃ����ώ���NULL(0x0)�Ŗ��߂�
////			memcpyFlushLeft2( (uchar *)&wkReceipt.credit.kid_code[0], &t_Credit.kid_code[0], sizeof(wkReceipt.credit.kid_code), sizeof(t_Credit.kid_code) );
//			//�ڼޯĶ��މ���� �݂Ȃ����ώ��̓X�y�[�X���߂���
//			memcpyFlushLeft( (uchar *)&wkReceipt.credit.card_no[0], &t_Credit.card_no[0], sizeof(wkReceipt.credit.card_no), sizeof(t_Credit.card_no) );
//			//�ڼޯĶ��ޒ[�����ʔԍ� �݂Ȃ����ώ��̓X�y�[�X���߂���
//			memcpyFlushLeft( (uchar *)&wkReceipt.credit.CCT_Num[0], &t_Credit.cct_num[0], sizeof(wkReceipt.credit.CCT_Num), sizeof(t_Credit.cct_num) );
//			//KID���� �݂Ȃ����ώ��̓X�y�[�X���߂���
//			memcpyFlushLeft( (uchar *)&wkReceipt.credit.kid_code[0], &t_Credit.kid_code[0], sizeof(wkReceipt.credit.kid_code), sizeof(t_Credit.kid_code) );
//// MH321800(E) NT-NET���Z�f�[�^�d�l�ύX
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//// �d�l�ύX(S) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
//			memcpyFlushLeft( (uchar *)&wkReceipt.credit.ShopAccountNo[0], &t_Credit.ShopAccountNo[0], sizeof(wkReceipt.credit.ShopAccountNo), sizeof(t_Credit.ShopAccountNo) );
//			memcpy( (uchar *)&wkReceipt.credit.card_name[0], RecvNtnetDt.RData16_01.card_name, sizeof(wkReceipt.credit.card_name) );
//// �d�l�ύX(E) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//			wkReceipt.Electron_data.Ec.e_pay_kind = EC_CREDIT_USED;
//// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
////			if (wkReceipt.credit.app_no == 0 &&
////				wkReceipt.credit.slip_no == 0 &&
////				wkReceipt.credit.card_no[0] == 0 &&
////				wkReceipt.credit.CCT_Num[0] == 0 &&
////				wkReceipt.credit.kid_code[0] == 0 ) {
////				// ���F�ԍ��A�`�[�ԍ��A����ԍ��A�[�����ʔԍ��AKID�R�[�h�� 0 �ł��鎞
//			if (wkReceipt.credit.app_no == 0 &&
//				wkReceipt.credit.slip_no == 0 &&
//				wkReceipt.credit.card_no[0] == 0x20 &&
//				wkReceipt.credit.CCT_Num[0] == 0x20 &&
//				wkReceipt.credit.kid_code[0] == 0x20 ) {
//				// ���F�ԍ��A�`�[�ԍ���0�A����ԍ��A�[�����ʔԍ��AKID�R�[�h�̐擪��0x20�̂Ƃ�
//// MH321800(E) NT-NET���Z�f�[�^�d�l�ύX
//				// �݂Ȃ����ς̃t���O�𗧂Ă�
//				wkReceipt.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;
//			}
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
//			break;
//		case 2:
//			memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//			memcpy( (uchar *)&t_EMoney, RecvNtnetDt.RData16_01.CardPaymentInfo, sizeof(t_EMoney) );
//			// �Í�������=AES�H
//			if( RecvNtnetDt.RData16_01.DataBasic.CMN_DT.DT_BASIC.encryptMode == 1 ){
//				AesCBCDecrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );
//			}
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
////			wkReceipt.Electron_data.Suica.pay_ryo = t_EMoney.amount;			// �d�q���ϐ��Z��� ���ϊz
////			wkReceipt.Electron_data.Suica.pay_after = t_EMoney.card_zangaku;	// �d�q���ϐ��Z��� ���ό�c��
////			wkReceipt.Electron_data.Suica.pay_befor = (t_EMoney.card_zangaku + t_EMoney.amount);
////			memcpy( &wkReceipt.Electron_data.Suica.Card_ID[0], &t_EMoney.card_id[0], sizeof(wkReceipt.Electron_data.Suica.Card_ID) );	// �d�q���ϐ��Z�����ID (Ascii 16��)
////// �s��C��(S) K.Onodera 2016/12/13 #1668 �d�q�}�l�[���Z�̗̎����Ĕ��s�����{�����ۂɗ̎����ɓd�q�}�l�[�̏�񂪈󎚂���Ȃ�
////			wkReceipt.Electron_data.Suica.e_pay_kind = SUICA_USED;				// ��ʌn�͂܂Ƃ߂�Suica���Z�b�g���Ă���
////// �s��C��(E) K.Onodera 2016/12/13 #1668 �d�q�}�l�[���Z�̗̎����Ĕ��s�����{�����ۂɗ̎����ɓd�q�}�l�[�̏�񂪈󎚂���Ȃ�
////			break;
//			if(isEC_USE()) {
//				wkReceipt.Electron_data.Ec.pay_ryo = t_EMoney.amount;			// �d�q���ϐ��Z��� ���ϊz
//				wkReceipt.Electron_data.Ec.pay_after = t_EMoney.card_zangaku;	// �d�q���ϐ��Z��� ���ό�c��
//				wkReceipt.Electron_data.Ec.pay_befor = (t_EMoney.card_zangaku + t_EMoney.amount);
//				memcpy( &wkReceipt.Electron_data.Ec.Card_ID[0], &t_EMoney.card_id[0], sizeof(wkReceipt.Electron_data.Ec.Card_ID) );
//				wkReceipt.Electron_data.Ec.e_pay_kind = EC_KOUTSUU_USED;
//				if (wkReceipt.Electron_data.Ec.pay_after == 0 &&
//					wkReceipt.Electron_data.Ec.Card_ID[0] == 0) {
//					// ���ό�c���A�J�[�hID�� 0 �ł��鎞
//					// �݂Ȃ����ς̃t���O�𗧂Ă�
//					wkReceipt.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;
//				}
//			} else {
//				wkReceipt.Electron_data.Suica.pay_ryo = t_EMoney.amount;			// �d�q���ϐ��Z��� ���ϊz
//				wkReceipt.Electron_data.Suica.pay_after = t_EMoney.card_zangaku;	// �d�q���ϐ��Z��� ���ό�c��
//				wkReceipt.Electron_data.Suica.pay_befor = (t_EMoney.card_zangaku + t_EMoney.amount);
//				memcpy( &wkReceipt.Electron_data.Suica.Card_ID[0], &t_EMoney.card_id[0], sizeof(wkReceipt.Electron_data.Suica.Card_ID) );	// �d�q���ϐ��Z�����ID (Ascii 16��)
//				wkReceipt.Electron_data.Suica.e_pay_kind = SUICA_USED;				// ��ʌn�͂܂Ƃ߂�Suica���Z�b�g���Ă���
//			}
//			break;
//		case 3:				// Edy
//		case 5:				// WAON
//		case 6:				// nanaco
//		case 7:				// SAPICA
//		case 12:			// iD
//		case 13:			// QUICPay
//			memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//			memcpy( (uchar *)&t_EMoney, RecvNtnetDt.RData16_01.CardPaymentInfo, sizeof(t_EMoney) );
//			// �Í�������=AES�H
//			if( RecvNtnetDt.RData16_01.DataBasic.CMN_DT.DT_BASIC.encryptMode == 1 ){
//				AesCBCDecrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );
//			}
//			wkReceipt.Electron_data.Ec.pay_ryo = t_EMoney.amount;			// �d�q���ϐ��Z��� ���ϊz
//			wkReceipt.Electron_data.Ec.pay_after = t_EMoney.card_zangaku;	// �d�q���ϐ��Z��� ���ό�c��
//			wkReceipt.Electron_data.Ec.pay_befor = (t_EMoney.card_zangaku + t_EMoney.amount);
//			memcpy( &wkReceipt.Electron_data.Ec.Card_ID[0], &t_EMoney.card_id[0], sizeof(wkReceipt.Electron_data.Ec.Card_ID) );	// �d�q���ϐ��Z�����ID
//			switch(RecvNtnetDt.RData16_01.CardPaymentKind) {
//				case 3:				// Edy
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_EDY_USED;
//					break;
//				case 5:				// WAON
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_WAON_USED;
//					break;
//				case 6:				// nanaco
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_NANACO_USED;
//					break;
//				case 7:				// SAPICA
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_SAPICA_USED;
//					break;
//				case 12:			// iD
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_ID_USED;
//					break;
//				case 13:		// QUICPay
//					wkReceipt.Electron_data.Ec.e_pay_kind = EC_QUIC_PAY_USED;
//					break;
//			}
//			if (wkReceipt.Electron_data.Ec.pay_after == 0 &&
//				wkReceipt.Electron_data.Ec.Card_ID[0] == 0) {
//				// �݂Ȃ����ς̃t���O�𗧂Ă�
//				wkReceipt.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;
//			}
//			break;
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
//	}
//	for( i = j = d = 0; i < (WTIK_USEMAX + DETAIL_SYU_MAX); i++ ){
//		if(( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != 0 ) &&					// ������ʂ���
//		   (( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet < NTNET_CSVS_M ) ||		// ���Z���~�������łȂ�
//		    ( NTNET_CFRE < RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet ))){
//// �d�l�ύX(S) K.Onodera 2016/11/07 �̎��؍Ĕ��s�v���t�H�[�}�b�g�ύX�Ή�
////		    if( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_SUICA_1 &&		// SUICA�ȊO(�ԍ�)
//			// �U�֐��Z
//			if( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_FURIKAE_2 ||
//				RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_FURIKAE_DETAIL ||
//				RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_FURIKAE_DETAIL ||
//				RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_FUTURE || 
//				RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet == NTNET_AFTER_PAY ){
//				wkReceipt.DetailData[d].ParkingNo				 = RecvNtnetDt.RData16_01.stDiscount[i].ParkNo;
//				wkReceipt.DetailData[d].DiscSyu					 = RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet;
//				wkReceipt.DetailData[d].uDetail.Common.DiscNo	 = RecvNtnetDt.RData16_01.stDiscount[i].DiscType;
//				wkReceipt.DetailData[d].uDetail.Common.DiscCount = RecvNtnetDt.RData16_01.stDiscount[i].DiscCount;
//				wkReceipt.DetailData[d].uDetail.Common.Discount	 = RecvNtnetDt.RData16_01.stDiscount[i].DiscPrice;
//				wkReceipt.DetailData[d].uDetail.Common.DiscInfo1 = RecvNtnetDt.RData16_01.stDiscount[i].DiscInfo1;
//				wkReceipt.DetailData[d].uDetail.Common.DiscInfo2 = RecvNtnetDt.RData16_01.stDiscount[i].DiscInfo2;
//				d++;
//			}
//			else if( RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_SUICA_1 &&		// SUICA�ȊO(�ԍ�)
//// �d�l�ύX(E) K.Onodera 2016/11/07 �̎��؍Ĕ��s�v���t�H�[�}�b�g�ύX�Ή�
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_SUICA_2 &&		//			(�x���z�E�c�z)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_PASMO_1 &&		// PASMO�ȊO(�ԍ�)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_PASMO_2 &&		//			(�x���z�E�c�z)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_EDY_1 &&		// EDY�ȊO	(�ԍ�)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_EDY_2 &&		//			(�x���z�E�c�z)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_ICOCA_1 &&		// ICOCA�ȊO(�ԍ�)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_ICOCA_2 &&		//			(�x���z�E�c�z)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_ICCARD_1 &&	// IC-Card�ȊO(�ԍ�)
//		    	RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet != NTNET_ICCARD_2 ) 	//			(�x���z�E�c�z)
//		    {
//				wkReceipt.DiscountData[j].ParkingNo	= RecvNtnetDt.RData16_01.stDiscount[i].ParkNo;			// ���ԏ�No.
//				wkReceipt.DiscountData[j].DiscSyu	= RecvNtnetDt.RData16_01.stDiscount[i].DiscSyubet;		// �������
//				wkReceipt.DiscountData[j].DiscNo	= RecvNtnetDt.RData16_01.stDiscount[i].DiscType;		// �����敪
//				wkReceipt.DiscountData[j].DiscCount	= RecvNtnetDt.RData16_01.stDiscount[i].DiscCount;		// �������
//				wkReceipt.DiscountData[j].Discount	= RecvNtnetDt.RData16_01.stDiscount[i].DiscPrice;		// �����z
//				wkReceipt.DiscountData[j].DiscInfo1	= RecvNtnetDt.RData16_01.stDiscount[i].DiscInfo1;		// �������1
//				if( wkReceipt.DiscountData[j].DiscSyu != NTNET_PRI_W ){
//					wkReceipt.DiscountData[j].uDiscData.common.DiscInfo2 = RecvNtnetDt.RData16_01.stDiscount[i].DiscInfo2;	// �������2
//				}
//// �s��C��(S) K.Onodera 2016/11/30 #1604 �̎��؂��Ĕ��s�Ŏx�����O�c�z���u0�~�v�ň󎚂���Ă��܂�
//		    	else{
//		    		// �g�p�O�c�z = �g�p���z + �c�z
//		    		wkReceipt.DiscountData[j].uDiscData.common.DiscInfo2 = (wkReceipt.DiscountData[j].Discount + wkReceipt.DiscountData[j].DiscInfo1);
//		    	}
//// �s��C��(E) K.Onodera 2016/11/30 #1604 �̎��؂��Ĕ��s�Ŏx�����O�c�z���u0�~�v�ň󎚂���Ă��܂�
//				j++;
//		    }
//		}
//	}
//	// ���Z�H
//	if( !RecvNtnetDt.RData16_01.PayInfoKind ){
//// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
////		wkReceipt.WPlace	= (ulong)((RecvNtnetDt.RData16_01.Area * 10000L) + RecvNtnetDt.RData16_01.No);
//// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		wkReceipt.PayMode	= RecvNtnetDt.RData16_01.PayMode;													// ���Z���[�h
//// MH810100(S) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//		wkReceipt.CMachineNo	= RecvNtnetDt.RData16_01.CMachineNo;											// ���Ԍ��@�B��(���ɋ@�B��)
//// MH810100(E) K.Onodera  2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//		for( i=0; i<6; i++ ){
//			if( RecvNtnetDt.RData16_01.InTime[i] ){
//				break;
//			}
//		}
//		// �I�[��0�łȂ�����
//		if( i != 6 ){
//			wkReceipt.TInTime.Year	= RecvNtnetDt.RData16_01.InTime[0] + 2000;	// ���ɔN
//			wkReceipt.TInTime.Mon	= RecvNtnetDt.RData16_01.InTime[1];			// ���Ɍ�
//			wkReceipt.TInTime.Day	= RecvNtnetDt.RData16_01.InTime[2];			// ���ɓ�
//			wkReceipt.TInTime.Hour	= RecvNtnetDt.RData16_01.InTime[3];			// ���Ɏ�
//			wkReceipt.TInTime.Min	= RecvNtnetDt.RData16_01.InTime[4];			// ���ɕ�
//		}
//// MH810100(S) K.Onodera  2020/03/04 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		for( i=0; i<6; i++ ){
//			if( RecvNtnetDt.RData16_01.PreOutTime[i] ){
//				break;
//			}
//		}
//		// �I�[��0�łȂ�����
//		if( i != 6 ){
//			wkReceipt.BeforeTPayTime.Year	= RecvNtnetDt.RData16_01.PreOutTime[0] + 2000;	// �O�񐸎Z�N
//			wkReceipt.BeforeTPayTime.Mon	= RecvNtnetDt.RData16_01.PreOutTime[1];			// �O�񐸎Z��
//			wkReceipt.BeforeTPayTime.Day	= RecvNtnetDt.RData16_01.PreOutTime[2];			// �O�񐸎Z��
//			wkReceipt.BeforeTPayTime.Hour	= RecvNtnetDt.RData16_01.PreOutTime[3];			// �O�񐸎Z��
//			wkReceipt.BeforeTPayTime.Min	= RecvNtnetDt.RData16_01.PreOutTime[4];			// �O�񐸎Z��
//		}
//// MH810100(E) K.Onodera  2020/03/04 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		for( i=0; i<6; i++ ){
//			if( RecvNtnetDt.RData16_01.OutTime[i] ){
//				break;
//			}
//		}
//		// �I�[��0�łȂ�����
//		if( i != 6 ){
//// MH810100(S) K.Onodera  2020/03/04 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:�R�����g�C��)
////			wkReceipt.TOutTime.Year	= RecvNtnetDt.RData16_01.OutTime[0] + 2000;	// �O�񐸎Z�N
////			wkReceipt.TOutTime.Mon	= RecvNtnetDt.RData16_01.OutTime[1];		// �O�񐸎Z��
////			wkReceipt.TOutTime.Day	= RecvNtnetDt.RData16_01.OutTime[2];		// �O�񐸎Z��
////			wkReceipt.TOutTime.Hour	= RecvNtnetDt.RData16_01.OutTime[3];		// �O�񐸎Z��
////			wkReceipt.TOutTime.Min	= RecvNtnetDt.RData16_01.OutTime[4];		// �O�񐸎Z��
//			wkReceipt.TOutTime.Year	= RecvNtnetDt.RData16_01.OutTime[0] + 2000;	// ���Z�N
//			wkReceipt.TOutTime.Mon	= RecvNtnetDt.RData16_01.OutTime[1];		// ���Z��
//			wkReceipt.TOutTime.Day	= RecvNtnetDt.RData16_01.OutTime[2];		// ���Z��
//			wkReceipt.TOutTime.Hour	= RecvNtnetDt.RData16_01.OutTime[3];		// ���Z��
//			wkReceipt.TOutTime.Min	= RecvNtnetDt.RData16_01.OutTime[4];		// ���Z��
//// MH810100(E) K.Onodera  2020/03/04 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:�R�����g�C��)
//		}
//		wkReceipt.OutKind	= RecvNtnetDt.RData16_01.OutKind;				// ���Z�o��
//		wkReceipt.CountSet	= RecvNtnetDt.RData16_01.CountSet;				// �ݎԃJ�E���g
//		wkReceipt.PayMethod	= RecvNtnetDt.RData16_01.PayMethod;				// ���Z���@
//		wkReceipt.PassCheck	= RecvNtnetDt.RData16_01.Antipas;				// �A���`�p�X�`�F�b�N
//// �d�l�ύX(S) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
//		// �U�֐搸�Z�H
//		if( RecvNtnetDt.RData16_01.PayMethod == 12 ){
//			wkReceipt.FRK_RetMod = RecvNtnetDt.RData16_01.CardFusokuType;
//			// �����ȊO�̕s���z����H
//			if( wkReceipt.FRK_RetMod ){
//				wkReceipt.FRK_Return = RecvNtnetDt.RData16_01.CardFusoku;
//			}else{
//				wkReceipt.FRK_Return = RecvNtnetDt.RData16_01.Change;
//				wkReceipt.WChgPrice  = 0;
//			}
//		}
//// �d�l�ύX(E) K.Onodera 2016/11/02 �̎��؍Ĕ��s�v���t�H�[�}�b�g�Ή�
//	}
//	// ����X�V�H
//	else{
//// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
////		wkReceipt.WPlace = 9999;
//// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		if( RecvNtnetDt.RData16_01.PassInfo.ParkNo ){
//			wkReceipt.teiki.id		= RecvNtnetDt.RData16_01.PassInfo.Id;					// �����ID
//			wkReceipt.teiki.syu		= RecvNtnetDt.RData16_01.PassInfo.Kind;					// ��������
//			wkReceipt.teiki.status	= RecvNtnetDt.RData16_01.PassInfo.Status;				// ������ð��
//			wkReceipt.teiki.s_year	= RecvNtnetDt.RData16_01.PassInfo.StartDate[0]+2000;	// ������J�n�N
//			wkReceipt.teiki.s_mon	= RecvNtnetDt.RData16_01.PassInfo.StartDate[1];			// ������J�n��
//			wkReceipt.teiki.s_day	= RecvNtnetDt.RData16_01.PassInfo.StartDate[2];			// ������J�n��
//			wkReceipt.teiki.e_year	= RecvNtnetDt.RData16_01.PassInfo.EndDate[0]+2000;		// ������I���N
//			wkReceipt.teiki.e_mon	= RecvNtnetDt.RData16_01.PassInfo.EndDate[1];			// ������I����
//			wkReceipt.teiki.e_day	= RecvNtnetDt.RData16_01.PassInfo.EndDate[2];			// ������I����
//			wkReceipt.WPrice		= RecvNtnetDt.RData16_01.PassInfo.UpdateFee;			// ������X�V����
//			wkReceipt.teiki.update_mon = RecvNtnetDt.RData16_01.PassInfo.UpdateTerm;		// ������X�V����(�X�V����)
//		}
//	}
	// ���Z�H
	if( !RecvNtnetDt.RData16_01.PayInfoKind ){
		// �ʐ��Z��񃍃O���猟������
		date[0] = RecvNtnetDt.RData16_01.OutTime[0] + 2000;
		date[1] = RecvNtnetDt.RData16_01.OutTime[1];
		date[2] = RecvNtnetDt.RData16_01.OutTime[2];
		target_date = dnrmlzm( date[0], date[1], date[2] );	// �Ώۂ̓��t(�m�[�}���C�Y)
		cnt_ttl = Ope2_Log_CountGet_inDate( eLOG_PAYMENT, &date[0], &id);				// �ʐ��Z��񃍃O����Ώۓ��̃��O�������擾
		for( ; cnt_ttl > 0 ; ) {
			if( 0 == Ope_Log_1DataGet(eLOG_PAYMENT, id, &wkReceipt )) {
				// ���O�擾�ł��Ȃ�������I��
				cnt_ttl = 0;
				break;
			}
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4748 �y�����Z�o�ɂ̗̎��؂�̎��؍Ĕ��s�v���f�[�^�iID:16�j�����{�����Ƃ��A�̎����Ĕ��s�����f�[�^���u���ʁ�12�v�ƂȂ�̎��؂̔��s�����s����Ȃ�)
//			seisan_date = dnrmlzm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day );
// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//			// ���	(0=���O���Z�^1�������Z�o�ɐ��Z)
//			if(wkReceipt.shubetsu == 0){
			// ���	(0=���O���Z�^1�������Z�o�ɐ��Z�^2=���u���Z(���Z���ύX))
			if(wkReceipt.shubetsu == 0 || wkReceipt.shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
				seisan_date = dnrmlzm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day );

			}else{
				seisan_date = dnrmlzm( wkReceipt.TUnpaidPayTime.Year, wkReceipt.TUnpaidPayTime.Mon, wkReceipt.TUnpaidPayTime.Day );

			}
// MH810100(e) 2020/08/20 �Ԕԃ`�P�b�g���X(#4748 �y�����Z�o�ɂ̗̎��؂�̎��؍Ĕ��s�v���f�[�^�iID:16�j�����{�����Ƃ��A�̎����Ĕ��s�����f�[�^���u���ʁ�12�v�ƂȂ�̎��؂̔��s�����s����Ȃ�)
			if( target_date != seisan_date ) {
				// �擾�������O�̓��t������Ă�����I��
				cnt_ttl = 0;
				break;
			}
// MH810100(S) 2020/08/18 �Ԕԃ`�P�b�g���X(#4599 ParkiPro�̗̎��؍Ĕ��s�ŃN���W�b�g���Z�̗̎��؂��Ĕ��s���ł��Ȃ�)
//			if(wkReceipt.TOutTime.Hour == RecvNtnetDt.RData16_01.OutTime[3]
//				&& wkReceipt.TOutTime.Min == RecvNtnetDt.RData16_01.OutTime[4] ) {
//				if( wkReceipt.PayClass == RecvNtnetDt.RData16_01.PayClass
			PayClassTmp = wkReceipt.PayClass;										// �����敪
			if( PayClassTmp == 4 || PayClassTmp == 5){
				// �N���W�b�g���Z�E�N���W�b�g�Đ��Z�̏ꍇ�́A0�E1�ɕϊ�����iNT-NET�ł�4�C5�͗��Ȃ��j
				PayClassTmp = PayClassTmp - 4;
			}
// MH810100(S) 2020/08/20 �Ԕԃ`�P�b�g���X(#4748 �y�����Z�o�ɂ̗̎��؂�̎��؍Ĕ��s�v���f�[�^�iID:16�j�����{�����Ƃ��A�̎����Ĕ��s�����f�[�^���u���ʁ�12�v�ƂȂ�̎��؂̔��s�����s����Ȃ�)
//			if(wkReceipt.TOutTime.Hour == RecvNtnetDt.RData16_01.OutTime[3]
//				&& wkReceipt.TOutTime.Min == RecvNtnetDt.RData16_01.OutTime[4] ) {
//				if( PayClassTmp == RecvNtnetDt.RData16_01.PayClass
//// MH810100(E) 2020/08/18 �Ԕԃ`�P�b�g���X(#4599 ParkiPro�̗̎��؍Ĕ��s�ŃN���W�b�g���Z�̗̎��؂��Ĕ��s���ł��Ȃ�)
//					&& CountSel( &wkReceipt.Oiban ) == RecvNtnetDt.RData16_01.PayCount ) {
//					// ���Z�����A�����敪�A���Z�ǔԂ���v���Ă����猩���������Ƃɂ���
//					break;
//				}
//			}
// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//			// ���	(0=���O���Z�^1�������Z�o�ɐ��Z)
//			if(wkReceipt.shubetsu == 0){
			// ���	(0=���O���Z�^1�������Z�o�ɐ��Z�^2=���u���Z(���Z���ύX))
			if(wkReceipt.shubetsu == 0 || wkReceipt.shubetsu == 0){
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
				if(wkReceipt.TOutTime.Hour == RecvNtnetDt.RData16_01.OutTime[3]
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
//					&& wkReceipt.TOutTime.Min == RecvNtnetDt.RData16_01.OutTime[4] ) {
					&& wkReceipt.TOutTime.Min == RecvNtnetDt.RData16_01.OutTime[4]
					&& wkReceipt.TOutTime_Sec == RecvNtnetDt.RData16_01.OutTime[5] ) {
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
					if( PayClassTmp == RecvNtnetDt.RData16_01.PayClass
						&& CountSel( &wkReceipt.Oiban ) == RecvNtnetDt.RData16_01.PayCount ) {
						// ���Z�����A�����敪�A���Z�ǔԂ���v���Ă����猩���������Ƃɂ���
						break;
					}
				}
			}else{
				if(wkReceipt.TUnpaidPayTime.Hour == RecvNtnetDt.RData16_01.OutTime[3]
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
//					&& wkReceipt.TUnpaidPayTime.Min == RecvNtnetDt.RData16_01.OutTime[4] ) {
					&& wkReceipt.TUnpaidPayTime.Min == RecvNtnetDt.RData16_01.OutTime[4]
					&& wkReceipt.TUnpaidPayTime_Sec == RecvNtnetDt.RData16_01.OutTime[5] ) {
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
					if( PayClassTmp == RecvNtnetDt.RData16_01.PayClass
						&& CountSel( &wkReceipt.Oiban ) == RecvNtnetDt.RData16_01.PayCount ) {
						// ���Z�����A�����敪�A���Z�ǔԂ���v���Ă����猩���������Ƃɂ���
						break;
					}
				}

			}
// MH810100(E) 2020/08/20 �Ԕԃ`�P�b�g���X(#4748 �y�����Z�o�ɂ̗̎��؂�̎��؍Ĕ��s�v���f�[�^�iID:16�j�����{�����Ƃ��A�̎����Ĕ��s�����f�[�^���u���ʁ�12�v�ƂȂ�̎��؂̔��s�����s����Ȃ�)
			id++;
			cnt_ttl--;
		}
	}
	// ����X�V�H
	else{
		// ����X�V�͖��Ή�
		cnt_ttl = 0;
	}
// MH810100(E) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j

// MH810105(S) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�
//// MH810105(S) MH364301 �C���{�C�X�Ή�
//	wkReceipt.WTaxRate = Disp_Tax_Value( &wkReceipt.TOutTime );								// �K�p�ŗ�
//// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//	// ���	(0=���O���Z�^1�������Z�o�ɐ��Z)
//	if(wkReceipt.shubetsu == 0){
	// ���	(0=���O���Z�^1�������Z�o�ɐ��Z�^2=���u���Z(���Z���ύX))
	if(wkReceipt.shubetsu == 0 || wkReceipt.shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
		wkReceipt.WTaxRate = Disp_Tax_Value( &wkReceipt.TOutTime );								// �K�p�ŗ�
	}else{
		wkReceipt.WTaxRate = Disp_Tax_Value( &wkReceipt.TUnpaidPayTime );						// �K�p�ŗ�
	}
// MH810105(E) MH364301 �C���{�C�X�Ή� #6406 �ŗ��ύX����Ɂu�o��ς̎ԗ��𐸎Z�v�Ő��Z�������ɗ̎��ؔ��s����ƁA�̎��؈󎚂̏���ŗ��ɕύX�O�ŗ����󎚂���Ă��܂�
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
	wkReceipt.WBillAmount = Billingcalculation(&wkReceipt);
	if (wkReceipt.chuusi == 0) {
		wkReceipt.WTaxPrice = TaxAmountcalculation(&wkReceipt, 0);
	}
	else {
		wkReceipt.WTaxPrice = TaxAmountcalculation(&wkReceipt, 1);
	}
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j

	// ���OK�H(�ҋ@or�Ԏ��ԍ����͒����A�����W�v���łȂ�����)
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//// MH810100(S) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
////	if( OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod <= 1) && auto_syu_prn != 2 ){
//  if( OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod <= 1) && auto_syu_prn != 2 && cnt_ttl > 0){
//// MH810100(E) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
	if( OPECTL.Mnt_mod == 0 && (OPECTL.Ope_mod <= 1) && auto_syu_prn != 2 && cnt_ttl > 0 &&
		Ope_isPrinterReady() != 0 ){
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
	// �̎��؍Ĕ��s
// GG129002(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�V�v�����^�󎚏����s��C���j
		memset(&rec_data, 0, sizeof(rec_data));
// GG129002(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�V�v�����^�󎚏����s��C���j
		rec_data.prn_kind = R_PRI;						// �������ʁFڼ��
		rec_data.prn_data = &wkReceipt;					// �̎��؈��ް����߲�����
		rec_data.kakari_no = 99;						// �W��No.99�i�Œ�j
		rec_data.reprint = ON;							// �Ĕ��s�׸޾�āi�Ĕ��s�j
		memcpy( &rec_data.PriTime, &CLK_REC, sizeof(date_time_rec) );
														// �Ĕ��s�����i���ݓ����j���
// MH810105(S) MH364301 �C���{�C�X�Ή�
//		queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
//		wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );		// ���엚��o�^
//		LedReq( CN_TRAYLED, LED_ON );					// ��ݎ�o�����޲��LED ON
//		LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// �d�q���ϒ��~�̎��ؔ��s���̎��o����LED������ϰ
		if (IS_INVOICE) {
			rec_data.prn_kind = J_PRI;					// �W���[�i���󎚂���
			memcpy(&Cancel_pri_work, &wkReceipt, sizeof(Cancel_pri_work));
			queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
			OPECTL.f_ReIsuType = 1;
		}
		else {
			queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
			wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );	// ���엚��o�^
			LedReq( CN_TRAYLED, LED_ON );				// ��ݎ�o�����޲��LED ON
			LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// �d�q���ϒ��~�̎��ؔ��s���̎��o����LED������ϰ
		}
// MH810105(E) MH364301 �C���{�C�X�Ή�
		rslt = PIP_RES_RESULT_OK;
	}
// MH810100(S) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
	else if ( cnt_ttl == 0 ){
		// �ʐ��Z��񃍃O���猩����Ȃ������ꍇ�̓p�����[�^�s�ǂƂ���
		rslt = PIP_RES_RESULT_NG_PARAM;					// �p�����[�^�s��
	}
// MH810100(E) S.Fujii 2020/07/30 #4542 �y�A���]���w�E�����zAI-V����̗̎��؍Ĕ��s��ύX����i���؉ێw�E No17�j
	else {
		rslt = PIP_RES_RESULT_NG_DENY;					// ��t�s���
	}

	// 1�x�㏑��
// �s��C��(S) K.Onodera 2016/09/27 #1504 �̎��؍Ĕ��s�����f�[�^�Łu���Z�ǔԁv�ȍ~�̓��e���Q�o�C�g����Ă���
//		memcpy( &SendNtnetDt.SData16_02, &RecvNtnetDt.RData16_01, sizeof(DATA_KIND_16_02) );
	size =  (ushort)_offsetof(DATA_KIND_16_02, PayCount);
	size = ( sizeof(DATA_KIND_16_02) - size );
	memcpy( &SendNtnetDt.SData16_02.PayCount, &RecvNtnetDt.RData16_01.PayCount, size );
// �s��C��(E) K.Onodera 2016/09/27 #1504 �̎��؍Ĕ��s�����f�[�^�Łu���Z�ǔԁv�ȍ~�̓��e���Q�o�C�g����Ă���

	// �����f�[�^���M
	BasicDataMake( PIP_RES_KIND_RECEIPT_AGAIN, 1 );									// ��{�ް��쐬 �ް��ێ��׸� = 1�Œ�
// MH310100(S) S.Fujii 2020/09/07 �Ԕԃ`�P�b�g���X(#4596 NT-NET���Z���f�[�^�̃V�[�P���V����No���ω����Ȃ�)
	SendNtnetDt.SData16_02.DataBasic.SeqNo = GetNtDataSeqNo();
// MH310100(E) S.Fujii 2020/09/07 �Ԕԃ`�P�b�g���X(#4596 NT-NET���Z���f�[�^�̃V�[�P���V����No���ω����Ȃ�)
	
// MH810105(S) MH364301 �̎��؍Ĕ��s�v���f�[�^Rev11��M
//	SendNtnetDt.SData16_02.Common.FmtRev 		= 10;											// �t�H�[�}�b�gRev.No.(10�Œ�)
	SendNtnetDt.SData16_02.Common.FmtRev 		= RecvNtnetDt.RData16_01.Common.FmtRev;			// �t�H�[�}�b�gRev.No.(��M����Rev��Ԃ�)
// MH810105(E) MH364301 �̎��؍Ĕ��s�v���f�[�^Rev11��M
	SendNtnetDt.SData16_02.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// ���M��̒[���^�[�~�i�������Z�b�g����i99�����uPC�j
	SendNtnetDt.SData16_02.Common.CenterSeqNo	= RecvNtnetDt.RData16_01.Common.CenterSeqNo;	// �Z���^�[�ǔ�
	SendNtnetDt.SData16_02.Result				= rslt;								// ����
	SendNtnetDt.SData16_02.Type					= RecvNtnetDt.RData16_01.PayInfoKind;// ���Z�����
	// �J�[�h���ϋ敪��0�łȂ��ꍇ�͈Í�������
	if( RecvNtnetDt.RData16_01.CardPaymentKind ){
		SendNtnetDt.SData16_02.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// �Í�������
		SendNtnetDt.SData16_02.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// �Í����ԍ�
	}
	if( _is_ntnet_remote() ){
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_02 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_02 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		�������G���[�A���[���v����M
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/09/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_10_ErrorAlarm( void )
{
	ushort	rslt = PIP_RES_RESULT_OK;
	ushort	cnt = 0;

	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_11 ) );
	BasicDataMake( PIP_RES_KIND_OCCUR_ERRALM, 1 );									// ��{�ް��쐬 �ް��ێ��׸� = 1�Œ�

	// �G���[�v���H
	if( RecvNtnetDt.RData16_10.ReqFlg == 0 ){
		cnt = PIP_GetOccurErr( &SendNtnetDt.SData16_11.stErrAlm[0] );
	}
	// �A���[���v���H
	else if( RecvNtnetDt.RData16_10.ReqFlg == 1 ){
		cnt = PIP_GetOccurAlarm( &SendNtnetDt.SData16_11.stErrAlm[0] );
	}
	// ���̑�
	else{
		rslt = PIP_RES_RESULT_NG_PARAM;
	}

	// ���ʕ����Z�b�g�����M
	SendNtnetDt.SData16_11.Common.FmtRev 		= 10;											// �t�H�[�}�b�gRev.No.(10�Œ�)
	SendNtnetDt.SData16_11.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;						// ���M��̒[���^�[�~�i�������Z�b�g����i99�����uPC�j
	SendNtnetDt.SData16_11.Common.CenterSeqNo	= RecvNtnetDt.RData16_10.Common.CenterSeqNo;	// �Z���^�[�ǔ�
	SendNtnetDt.SData16_11.Result				= rslt;											// ����
	SendNtnetDt.SData16_11.Kind					= RecvNtnetDt.RData16_10.ReqFlg;				// �����G���[�A���[���t���O�i0=�G���[�^1=�A���[���j
	SendNtnetDt.SData16_11.Count				= cnt;											// �������G���[�A���[������
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_11 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_11 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
}

///*[]----------------------------------------------------------------------[]*
// *| �������G���[��񉞓�
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData182_OccurErr
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static ushort PIP_GetOccurErr( PIP_ErrAlarm *tbl )
{
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	extern 	ushort	LKcom_RoomNoToType( ulong roomNo );
//	static const uchar	ucBit[] = { 0x01, 0x02, 0x04, 0x08 };
//	ushort		Index = 0, i = 0, j = 0, errcod, sno, smax;
//	ulong		errinfo;
//	uchar		ucErrWk;
	ushort		Index = 0, i = 0, j = 0;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	// �������̃G���[���l�߂�i�t���b�v�ƃ��b�N�͏����j
	for( i = 0; i < ERR_MOD_MAX; i++ ){
		WACDOG;
		for( j=1; j<ERR_NUM_MAX; j++ ){
// �s��C��(S) K.Onodera 2016/10/13 #1505 �A���[��01-61(5000�~�D�ނ�؂�)�������܂ރf�[�^����������Ă��܂�
//			if( ERR_CHK[i][j] ){
			if( ERR_CHK[i][j] && isDefToErrAlmTbl(0, i, j) ){
// �s��C��(E) K.Onodera 2016/10/13 #1505 �A���[��01-61(5000�~�D�ނ�؂�)�������܂ރf�[�^����������Ă��܂�
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//				if( ( i == mod_flapcrb ) &&
//					( j == ERR_FLAPLOCK_LOCKCLOSEFAIL || j == ERR_FLAPLOCK_LOCKOPENFAIL ||	// �J/�� or �㏸/���~�G���[
//		 			  j == ERR_FLAPLOCK_DOWNRETRYOVER || j == ERR_FLAPLOCK_DOWNRETRY ) ) {	// �ی쏈�����g���C�I�[�o�[/�ی쏈���J�n
//					// �G���[��ʁF�P�U�i�ʐM�^�C�v�F�t���b�v�^���b�N���u�j
//					continue;
//				}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
				// �Z�b�g�\�ȍő吔�𒴂�����G���[���̂݃J�E���g����
				if( Index < PIP_OCCUR_ERROR_ALARM_MAX ){
					tbl[Index].ucKind	= ErrAct_index[i];				// ���
					tbl[Index].ucCoce	= j;							// �R�[�h
					tbl[Index].ucLevel	= (uchar)getErrLevel(i,j);		// ���x��
					if( OPE_SIG_DOOR_Is_OPEN == 0 ){					// �h�A���
						tbl[Index].ucDoor = 0;
					}else{
						tbl[Index].ucDoor = 1;
					}
					memcpy( &tbl[Index].ucInfo[6], &ERR_INFO[i][j], 4 );	// �G���[���
				}
				Index++;
			}
		}
	}
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	// �G���[��ʁF�P�U�i�ʐM�^�C�v�F�t���b�v�^���b�N���u�j�㏸�^���~���b�N�̌���
//	switch( GetCarInfoParam() ){	// �ݒ�03-0100�Q�Ɓ@�ԁA�o�C�N
//		case 0x01: // ���b�N�̂�
//			sno = BIKE_START_INDEX;						// 100�`
//			smax = BIKE_START_INDEX + BIKE_LOCK_MAX;	// 100+50
//			break;
//		case 0x04: // �Ԃ̂�
//			sno = INT_CAR_START_INDEX;						// 50�`
//			smax = INT_CAR_START_INDEX + INT_CAR_LOCK_MAX;	// 50+20
//			break;
//		case 0x05: // ����
//		default:
//			sno = INT_CAR_START_INDEX;					// 50�`
//			smax = BIKE_START_INDEX + BIKE_LOCK_MAX;	// 100+50
//			break;
//	}
//
//	for( ; sno < smax; sno++ ){
//
//		ucErrWk = flp_err_search( sno, &errinfo );
//		// �G���[�Ԏ�����H
//		if( ucErrWk ){
//			// �G���[�̎Ԏ��̓��b�N���u�H
//			if( 0 == LKcom_RoomNoToType(errinfo) ){
//				// 18-0019�E=1�@�㏸/���~�y�ъJ/���b�N�󎚂��Ȃ��H
//				if( prm_get( COM_PRM, S_PRN, 19, 1, 1 ) == 0 ){
//					continue;
//				}
//			}
//			// �G���[�̎Ԏ�̓t���b�v�H
//			else{
//				// 18-0019�C=1�@�㏸/���~�y�ъJ/���b�N�󎚂��Ȃ��H
//				if( prm_get( COM_PRM, S_PRN, 19, 1, 3 ) == 0 ){
//					continue;
//				}
//			}
//			for( i=0; i<4; i++ ){
//				if( ucErrWk & ucBit[i] ){
//					switch( ucBit[i] ){
//						case 0x01:
//							errcod = ERR_FLAPLOCK_LOCKCLOSEFAIL;			// E1638:�㏸���b�N������
//							break;
//						case 0x02:
//							errcod = ERR_FLAPLOCK_LOCKOPENFAIL;				// E1639:�������b�N������
//							break;
//						case 0x04:
//							errcod = ERR_FLAPLOCK_DOWNRETRYOVER;			// E1640:���g���C�I�[�o�[�G���[
//							break;
//						case 0x08:
//							errcod = ERR_FLAPLOCK_DOWNRETRY;				// E1641:���g���C����J�n�G���[
//							break;
//						default:
//							continue;
//							break;
//					}
//					if( isDefToErrAlmTbl(0, (uchar)mod_flapcrb, (uchar)errcod) ){
//						if( Index < PIP_OCCUR_ERROR_ALARM_MAX ){
//							tbl[Index].ucKind	= ErrAct_index[mod_flapcrb];				// ���
//							tbl[Index].ucCoce	= errcod;									// �R�[�h
//							tbl[Index].ucLevel	= (uchar)getErrLevel(mod_flapcrb,errcod);	// ���x��
//							if( OPE_SIG_DOOR_Is_OPEN == 0 ){								// �h�A���
//								tbl[Index].ucDoor = 0;
//							}else{
//								tbl[Index].ucDoor = 1;
//							}
//							memcpy( &tbl[Index].ucInfo[6], &ERR_LOCK_INFO[sno][i], 4 );	// �G���[���
//						}
//						Index++;
//					}
//				}
//			}
//		}
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	return Index;
}

///*[]----------------------------------------------------------------------[]*
// *| �������A���[����񉞓�
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData182_OccurAlarm
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static ushort PIP_GetOccurAlarm( PIP_ErrAlarm *tbl )
{
	ushort		Index = 0;
	ushort		i = 0, j = 0;
	ushort	count = 0, Total = 0;

	// �������װт̐��𐔂���
	for( i = 0; i < ALM_MOD_MAX; i++ ){
		WACDOG;
		for( j = 1; j < ALM_NUM_MAX; j++ ){
// �s��C��(S) K.Onodera 2016/10/13 #1505 �A���[��01-61(5000�~�D�ނ�؂�)�������܂ރf�[�^����������Ă��܂�
//			if( ALM_CHK[i][j] ){
			if( ALM_CHK[i][j] && isDefToErrAlmTbl(1, i, j) ){
// �s��C��(E) K.Onodera 2016/10/13 #1505 �A���[��01-61(5000�~�D�ނ�؂�)�������܂ރf�[�^����������Ă��܂�
				count++;
			}
		}
	}
	Total = count;		// �������擾
	if( count > PIP_OCCUR_ERROR_ALARM_MAX ){
		count = PIP_OCCUR_ERROR_ALARM_MAX;
	}
	if( count != 0 ){
		for( i = 0; i < ALM_MOD_MAX; i++ ){
			WACDOG;
			for( j = 1; j < ALM_NUM_MAX; j++ ){
// �s��C��(S) K.Onodera 2016/10/13 #1505 �A���[��01-61(5000�~�D�ނ�؂�)�������܂ރf�[�^����������Ă��܂�
//				if( ALM_CHK[i][j] ){
				if( ALM_CHK[i][j] && isDefToErrAlmTbl(1, i, j) ){
// �s��C��(E) K.Onodera 2016/10/13 #1505 �A���[��01-61(5000�~�D�ނ�؂�)�������܂ރf�[�^����������Ă��܂�
					tbl[Index].ucKind = i;							// ���
					tbl[Index].ucCoce = j;							// �R�[�h
					tbl[Index].ucLevel = (uchar)getAlmLevel(i,j);	// ���x��
					if( OPE_SIG_DOOR_Is_OPEN == 0 ){				// �h�A���
						tbl[Index].ucDoor = 0;
					}else{
						tbl[Index].ucDoor = 1;
					}
					memcpy( &tbl[Index].ucInfo[6], &ALM_INFO[i][j], 4 );	// �G���[���
					count--;
					Index++;
				}
			}
		}
	}

	return Total;
}

//[]----------------------------------------------------------------------[]
///	@brief		�U�֐��Z�v����M
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/18<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:���[�j���O�΍�)
//static void NTNET_RevData16_03_Furikae( void )
void NTNET_RevData16_03_Furikae( void )
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:���[�j���O�΍�)
{
	switch( RecvNtnetDt.RData16_03.ReqKind ){
		// �U�֑Ώۏ��
		case PIP_FURIKAE_TYPE_INFO:
			NTNET_RevData16_03_Furikae_Info();
			break;
		// �U�֐��Z���s�\�`�F�b�N
		case PIP_FURIKAE_TYPE_CHK:
			NTNET_RevData16_03_Furikae_Check();
			break;
		// �U�֐��Z
		case PIP_FURIKAE_TYPE_GO:
			NTNET_RevData16_03_Furikae_Go();
			break;
		default:
			// NG����(�p�����[�^�s��)
			NTNET_Snd_Data16_04( PIP_RES_RESULT_NG_PARAM );
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		�U�֐��Z����������
//[]----------------------------------------------------------------------[]
void PiP_FurikaeInit( void )
{
	PiP_FurikaeSts = PIP_FRK_STS_IDLE;
}
//[]----------------------------------------------------------------------[]
///	@brief		�U�֐��Z�J�n����
//[]----------------------------------------------------------------------[]
void PiP_FurikaeStart( void )
{
	PiP_FurikaeSts = PIP_FRK_STS_ACP;
}
//[]----------------------------------------------------------------------[]
///	@brief		�U�֐��Z�J�n����
//[]----------------------------------------------------------------------[]
ushort PiP_GetFurikaeSts( void )
{
	return PiP_FurikaeSts;
}
//[]----------------------------------------------------------------------[]
///	@brief		�U�֐��Z�I������
//[]----------------------------------------------------------------------[]
void PiP_FurikaeEnd( void )
{
	PiP_FurikaeSts = PIP_FRK_STS_IDLE;
}

//[]----------------------------------------------------------------------[]
///	@brief		�U�֐��Z�I������
//[]----------------------------------------------------------------------[]
Receipt_data* GetFurikaeSrcReciptData( void )
{
	return &wkReceipt;
}
//[]----------------------------------------------------------------------[]
///	@brief		�U�փ`�F�b�N����
//[]----------------------------------------------------------------------[]
static ushort PiP_CheckFurikae( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ushort			index, pos1, pos2;
	uchar			sts = 0;

	// �e��`�F�b�N
	do{
		// �P�D��ԃ`�F�b�N(�U�֒��łȂ�����)
		if( PiP_FurikaeSts == PIP_FRK_STS_IDLE ){
			memset( &g_PipCtrl.stFurikaeInfo, 0, sizeof(g_PipCtrl.stFurikaeInfo) );
		}else{
			rslt = PIP_RES_RESULT_NG_DENY;
			break;
		}

		// �Q�D��ԃ`�F�b�N(�ҋ@��ʂł��邱��)
		sts = (get_crm_state() - 0x30);			// ���Z�������
		// �Ԏ��ԍ����͒��H
		if( OPECTL.Ope_mod == 1 && OPECTL.Mnt_mod == 0 ){
			sts = 0;
		}
		// �U�֑Ώۏ��v���̏ꍇ�A�����p�ɕێ�
		if( RecvNtnetDt.RData16_03.ReqKind == PIP_FURIKAE_TYPE_INFO ){
			g_PipCtrl.stFurikaeInfo.PayState = (ushort)sts;
		}
		// �ҋ@�łȂ��H
		if( sts ){
			rslt = PIP_RES_RESULT_NG_DENY;
			break;
		}

		// �R.�����W�v���`�F�b�N
		if( auto_syu_prn == 2 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// ��t�s���
			break;
		}

		// �S�D�N���W�b�gHOST�ڑ����H
		if( OPECTL.InquiryFlg ){
			rslt = PIP_RES_RESULT_NG_BUSY;
			break;
		}

		// �T�D�U�֌��Ԏ����`�F�b�N
		rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_03.SrcArea,RecvNtnetDt.RData16_03.SrcNo, &pos1 );	// �Ԏ����擾
		if( rslt == PIP_RES_RESULT_OK ){
			index = pos1 - 1;
			// ���s�\�`�F�b�N�łȂ���Ή����p�ɕێ�
			if( RecvNtnetDt.RData16_03.ReqKind != PIP_FURIKAE_TYPE_CHK ){
				g_PipCtrl.stFurikaeInfo.SrcStatus = FLAPDT.flp_data[index].nstat.word;
			}
		}else{
			break;
		}

		// �U�D�t���b�v���
		if( FLAPDT.flp_data[index].mode < FLAP_CTRL_MODE5 || FLAPDT.flp_data[index].mode > FLAP_CTRL_MODE6 ){
			rslt = PIP_RES_RESULT_NG_FURIKAE_SRC;	// �U�֌������G���[
			break;
		}

		// �V�D�U�֐�Ԏ����`�F�b�N
		rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_03.DestArea,RecvNtnetDt.RData16_03.DestNo, &pos2 );	// �Ԏ����擾
		if( rslt == PIP_RES_RESULT_OK ){
			index = pos2 - 1;
			// ���s�\�`�F�b�N�łȂ���Ή����p�ɕێ�
			if( RecvNtnetDt.RData16_03.ReqKind != PIP_FURIKAE_TYPE_CHK ){
				g_PipCtrl.stFurikaeInfo.DestStatus = FLAPDT.flp_data[index].nstat.word;
			}
		}else{
			break;
		}

		// �W�D�t���b�v���
		if( FLAPDT.flp_data[index].mode < FLAP_CTRL_MODE2 || FLAPDT.flp_data[index].mode > FLAP_CTRL_MODE4 ){
			rslt = PIP_RES_RESULT_NG_FURIKAE_DST;	// �U�֐�����G���[
			break;
		}

// ���r���[�w�E�Ή�(S) K.Onodera 2016/10/04 �s�v�Ȕ�����폜
//		// �X�D�U�֌��ƐU�֐悪��v�H
//		if( pos1 == pos2 ){
//			rslt = PIP_RES_RESULT_NG_PARAM;
//			break;
//		}
// ���r���[�w�E�Ή�(E) K.Onodera 2016/10/04 �s�v�Ȕ�����폜

	}while(0);

	return rslt;
}

//[]----------------------------------------------------------------------[]
///	@brief		�Ԏ��������ɐ��Z�f�[�^���� ���ŐV����
//[]----------------------------------------------------------------------[]
static uchar PiP_GetNewestPayReceiptData( Receipt_data *rcp, ushort area, ushort no )
{
	uchar			chk = 0;	// ��v�Ԏ��Ȃ�
	ushort			index = 0, index_ttl = 0;

	// ���Z�f�[�^����
	index_ttl = Ope_SaleLog_TotalCountGet( PAY_LOG_CMP );
	index = index_ttl;
	while( index != 0 ){
		index--;
		Ope_SaleLog_1DataGet( index, PAY_LOG_CMP, index_ttl, rcp );
		// �Ԏ���v�H
		if( rcp->WPlace == ( area * 10000L ) + no ){
			chk = 1;
			break;
		}
	}

	return chk;
}

//[]----------------------------------------------------------------------[]
///	@brief		�U�֑Ώۏ��v��
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/18<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_03_Furikae_Info( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ushort			i;
	ulong			CompTime1 = 0, CompTime2 = 0;

	// �U�֑O�`�F�b�N����
	rslt = PiP_CheckFurikae();

	// �`�F�b�NOK�H
	if( rslt == PIP_RES_RESULT_OK ){

		// ���Z�f�[�^�擾
		memset( &wkReceipt, 0, sizeof( Receipt_data) );
		if( 0 == PiP_GetNewestPayReceiptData( &wkReceipt, RecvNtnetDt.RData16_03.SrcArea, RecvNtnetDt.RData16_03.SrcNo ) ){
			rslt = PIP_RES_RESULT_NG_NO_CAR;
			// NG����
			NTNET_Snd_Data16_04( rslt );
			return;
		}
		// �U�֌����Z�����m�[�}���C�Y
		CompTime1 = enc_nmlz_mdhm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day,
																wkReceipt.TOutTime.Hour, wkReceipt.TOutTime.Min );
		// �O��W�v�����m�[�}���C�Y
		CompTime2 = enc_nmlz_mdhm( sky.tsyuk.OldTime.Year, sky.tsyuk.OldTime.Mon, sky.tsyuk.OldTime.Day,
																sky.tsyuk.OldTime.Hour, sky.tsyuk.OldTime.Min );
		// �W�v���ׂ��ł���H
		if( CompTime1 <= CompTime2 ){
			// NG����
			rslt = PIP_RES_RESULT_NG_DENY;
			NTNET_Snd_Data16_04( rslt );
			return;
		}

		// �����p�f�[�^�Z�b�g =============
		// �@����
		g_PipCtrl.stFurikaeInfo.OpenClose	= (uchar)(opncls()-1);				// �c�x�Ə�
		g_PipCtrl.stFurikaeInfo.ErrOccur	= Err_onf;							// �G���[������
		g_PipCtrl.stFurikaeInfo.AlmOccur	= Alm_onf;							// �A���[��������
		if( OPECTL.Mnt_mod ){
			g_PipCtrl.stFurikaeInfo.MntMode	= 1; 								// ���Z�@���샂�[�h��
		}
		// �U�֌�
		PiP_GetFurikaeInfo( &wkReceipt );										// �U�֌������擾
		g_PipCtrl.stFurikaeInfo.PassUse		= vl_frs.antipassoff_req;			// �U�֌�������p�L��
		g_PipCtrl.stFurikaeInfo.SrcArea		= RecvNtnetDt.RData16_03.SrcArea;	// ���
		g_PipCtrl.stFurikaeInfo.SrcNo		= RecvNtnetDt.RData16_03.SrcNo;		// �Ԏ�
		g_PipCtrl.stFurikaeInfo.SrcFeeKind	= wkReceipt.syu;					// �������
		g_PipCtrl.stFurikaeInfo.SrcFee		= wkReceipt.WPrice;					// ���ԗ���
		for( i=0; i<WTIK_USEMAX; i++ ){
// �s��C��(S) K.Onodera 2016/11/24 #1585 �U�֌��������z�ɖ��֌W�̒l���Z�b�g�����
//			g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DiscountData[i].Discount;	// �����z���Z
			if( ( wkReceipt.DiscountData[i].DiscSyu != 0 ) &&					// ������ʂ���
			    (( wkReceipt.DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// ���Z���~�������łȂ�
			    ( NTNET_CFRE < wkReceipt.DiscountData[i].DiscSyu )) ){
			    if( wkReceipt.DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA�ȊO(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(�x���z�E�c�z)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO�ȊO(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(�x���z�E�c�z)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_EDY_1 &&			// EDY�ȊO	(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_EDY_2 &&			//			(�x���z�E�c�z)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA�ȊO(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(�x���z�E�c�z)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card�ȊO(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICCARD_2 &&		//			(�x���z�E�c�z)
			    	wkReceipt.DiscountData[i].DiscSyu < NTNET_SECTION_WARI_MAX) // �������Ƃ��Ďg�p����͎̂��001�`999�܂�
			    {
			    	g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DiscountData[i].Discount;	// �����z���Z
			    }
			}
// �s��C��(E) K.Onodera 2016/11/24 #1585 �U�֌��������z�ɖ��֌W�̒l���Z�b�g�����
		}
// �d�l�ύX(S) K.Onodera 2016/12/02 �U�֊z�ύX
		for( i=0; i<DETAIL_SYU_MAX; i++ ){
			if( wkReceipt.DiscountData[i].DiscSyu == NTNET_FURIKAE_2 ){
				g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DetailData[i].uDetail.Furikae.Total;
			}
		}
// �d�l�ύX(E) K.Onodera 2016/12/02 �U�֊z�ύX
		memcpy( &g_PipCtrl.stFurikaeInfo.SrcPayTime, &wkReceipt.TOutTime, sizeof(date_time_rec) );	// ���Z���� �����܂�
		memcpy( &g_PipCtrl.stFurikaeInfo.SrcInTime,  &wkReceipt.TInTime, sizeof(date_time_rec) );		// ���Ɏ��� �����܂�
		// �U�֐�
		g_PipCtrl.stFurikaeInfo.DestArea	= RecvNtnetDt.RData16_03.DestArea;	// ���
		g_PipCtrl.stFurikaeInfo.DestNo		= RecvNtnetDt.RData16_03.DestNo;	// �Ԏ�
		// ��L�ȊO�̐U�֐���͗����v�Z���K�v�Ȃ��߁AOPE���ŃZ�b�g

		// �����v�Z�v��
		queset( OPETCBNO, OPE_REQ_FURIKAE_TARGET_INFO, 0, NULL );

	}
	else{
		// NG����
		NTNET_Snd_Data16_04( rslt );
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		�U�֌����擾
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
static void PiP_GetFurikaeInfo( Receipt_data* receipt )
{
	ushort	i;

	memset(&vl_frs, 0, sizeof(vl_frs));

	PIP_CheckCarInfo( RecvNtnetDt.RData16_03.SrcArea,RecvNtnetDt.RData16_03.SrcNo, &vl_frs.lockno );

	// ����g�p����H
	if( RecvNtnetDt.RData16_03.SrcPassUse ){
		vl_frs.antipassoff_req = (receipt->teiki.id)? 1 : 0;	// �U�֌��Œ���g�p
	}

	vl_frs.price = receipt->WPrice /* + receipt.Wtax */;		// ���łȂ��Wtax==0�F�O�őΉ����Ȃ�

	// �ȉ��̍��ڂ͂O�ŏ���������Ă���̂ŁA�������ɐώZ��
	vl_frs.in_price = vl_frs.genkin_furikaegaku = receipt->WInPrice - receipt->WChgPrice;	// ����
	for (i = 0; i < WTIK_USEMAX; i++) {
		if ((receipt->DiscountData[i].DiscSyu == NTNET_FRE ) ||	// �񐔌�
		    (receipt->DiscountData[i].DiscSyu == NTNET_PRI_W )) {// �v���y�C�h�J�[�h
			vl_frs.in_price += receipt->DiscountData[i].Discount;	// �����z���Z
		}
	}
	vl_frs.in_price += receipt->ppc_chusi;						// �v���y�C�h�^�񐔌����~�f�[�^
	vl_frs.in_price += receipt->credit.pay_ryo;					// �N���W�b�g
	vl_frs.in_price += receipt->Electron_data.Suica.pay_ryo;		// �d�q�}�l�[
// �d�l�ύX(S) K.Onodera 2016/10/28 �̎��؃t�H�[�}�b�g
	// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//	if( receipt->Electron_data.Suica.e_pay_kind != 0 ){	// �d�q���ώ�� Suica:1, Edy:2
	if( EcUseKindCheck(receipt->Electron_data.Ec.e_pay_kind) ){
		switch(receipt->Electron_data.Ec.e_pay_kind) {
		case EC_EDY_USED:
			vl_frs.card_type = (ushort)3;
			break;
		case EC_NANACO_USED:
			vl_frs.card_type = (ushort)6;
			break;
		case EC_WAON_USED:
			vl_frs.card_type = (ushort)5;
			break;
		case EC_SAPICA_USED:
			vl_frs.card_type = (ushort)7;
			break;
		case EC_KOUTSUU_USED:
			vl_frs.card_type = (ushort)2;
			break;
		case EC_ID_USED:
			vl_frs.card_type = (ushort)8;
			break;
		case EC_QUIC_PAY_USED:
			vl_frs.card_type = (ushort)9;
			break;
		default:
			break;		// ���肦�Ȃ�
		}
		vl_frs.card_furikaegaku = receipt->Electron_data.Ec.pay_ryo;
	}else if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind )){
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
		vl_frs.card_type = (ushort)2;
		vl_frs.card_furikaegaku = receipt->Electron_data.Suica.pay_ryo;
	}else if( receipt->PayClass == 4 ){					// �N���W�b�g���Z
		vl_frs.card_type = (ushort)1;
		vl_frs.card_furikaegaku = receipt->credit.pay_ryo;
	}
// �d�l�ύX(E) K.Onodera 2016/10/28 �̎��؃t�H�[�}�b�g
// �d�l�ύX(S) K.Onodera 2016/12/02 �U�֊z�ύX
	for( i=0; i<DETAIL_SYU_MAX; i++ ){
		if( receipt->DetailData[i].DiscSyu == NTNET_FURIKAE_2 ){
			vl_frs.in_price += (receipt->DetailData[i].uDetail.Furikae.Total - receipt->FRK_Return );
		}
		if( receipt->DetailData[i].DiscSyu == NTNET_FURIKAE_DETAIL ){
			// �����H
			if( !receipt->DetailData[i].uDetail.FurikaeDetail.Mod ){
				vl_frs.genkin_furikaegaku += (receipt->DetailData[i].uDetail.FurikaeDetail.FrkMoney - receipt->FRK_Return);
			}
			// �N���W�b�gor�d�q�}�l�[�H
			else{
				vl_frs.card_type		   = receipt->DetailData[i].uDetail.FurikaeDetail.Mod;
				vl_frs.card_furikaegaku   += (receipt->DetailData[i].uDetail.FurikaeDetail.FrkCard - receipt->FRK_Return);
			}
		}
	}
// �d�l�ύX(E) K.Onodera 2016/12/02 �U�֊z�ύX
	vl_frs.syubetu = receipt->syu;									// �������
	vl_frs.seisan_oiban = receipt->Oiban;							// ���Z�ǂ���
	vl_frs.seisan_time = receipt->TOutTime;						// ���Z����
}

//[]----------------------------------------------------------------------[]
///	@brief		�U�֐��Z���s�\�`�F�b�N�v��
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/18<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_03_Furikae_Check( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ulong			CompTime1 = 0, CompTime2 = 0;

	// ���s�\�`�F�b�N
	rslt = PiP_CheckFurikae();

	// �`�F�b�NOK�H
	if( rslt == PIP_RES_RESULT_OK ){

		// ���Z�f�[�^�擾
		memset( &wkReceipt, 0, sizeof( Receipt_data) );
		if( 0 == PiP_GetNewestPayReceiptData( &wkReceipt, RecvNtnetDt.RData16_03.SrcArea, RecvNtnetDt.RData16_03.SrcNo ) ){
			rslt = PIP_RES_RESULT_NG_NO_CAR;
			// NG����
			NTNET_Snd_Data16_04( rslt );
			return;
		}
		// �U�֌����Z�����m�[�}���C�Y
		CompTime1 = enc_nmlz_mdhm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day,
																wkReceipt.TOutTime.Hour, wkReceipt.TOutTime.Min );
		// �O��W�v�����m�[�}���C�Y
		CompTime2 = enc_nmlz_mdhm( sky.tsyuk.OldTime.Year, sky.tsyuk.OldTime.Mon, sky.tsyuk.OldTime.Day,
																sky.tsyuk.OldTime.Hour, sky.tsyuk.OldTime.Min );
		// �W�v���ׂ��ł���H
		if( CompTime1 <= CompTime2 ){
			// NG����
			rslt = PIP_RES_RESULT_NG_DENY;
			NTNET_Snd_Data16_04( rslt );
			return;
		}

		// �U�֌��Ԏ�
		g_PipCtrl.stFurikaeInfo.SrcArea		= RecvNtnetDt.RData16_03.SrcArea;	// ���
		g_PipCtrl.stFurikaeInfo.SrcNo		= RecvNtnetDt.RData16_03.SrcNo;		// �Ԏ�
		// �U�֐�Ԏ�
		g_PipCtrl.stFurikaeInfo.DestArea	= RecvNtnetDt.RData16_03.DestArea;	// ���
		g_PipCtrl.stFurikaeInfo.DestNo		= RecvNtnetDt.RData16_03.DestNo;	// �Ԏ�
	}

	// ���ʉ���(OK/NG)
	NTNET_Snd_Data16_04( rslt );
}

//[]----------------------------------------------------------------------[]
///	@brief		�U�֐��Z�v��
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/18<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_03_Furikae_Go( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ushort			i;
	ulong			CompTime1 = 0, CompTime2 = 0;

	// ���s�\�`�F�b�N
	rslt = PiP_CheckFurikae();

	// �`�F�b�NOK�H
	if( rslt == PIP_RES_RESULT_OK ){

		// ���Z�f�[�^�擾
		memset( &wkReceipt, 0, sizeof( Receipt_data) );
		if( 0 == PiP_GetNewestPayReceiptData( &wkReceipt, RecvNtnetDt.RData16_03.SrcArea, RecvNtnetDt.RData16_03.SrcNo ) ){
			rslt = PIP_RES_RESULT_NG_NO_CAR;
			// NG����
			NTNET_Snd_Data16_04( rslt );
			return;
		}
		// �U�֌����Z�����m�[�}���C�Y
		CompTime1 = enc_nmlz_mdhm( wkReceipt.TOutTime.Year, wkReceipt.TOutTime.Mon, wkReceipt.TOutTime.Day,
																wkReceipt.TOutTime.Hour, wkReceipt.TOutTime.Min );
		// �O��W�v�����m�[�}���C�Y
		CompTime2 = enc_nmlz_mdhm( sky.tsyuk.OldTime.Year, sky.tsyuk.OldTime.Mon, sky.tsyuk.OldTime.Day,
																sky.tsyuk.OldTime.Hour, sky.tsyuk.OldTime.Min );
		// �W�v���ׂ��ł���H
		if( CompTime1 <= CompTime2 ){
			// NG����
			rslt = PIP_RES_RESULT_NG_DENY;
			NTNET_Snd_Data16_04( rslt );
			return;
		}

		g_PipCtrl.stFurikaeInfo.ReqKind = UNI_KIND_FURIKAE;
		// �U�֌��Ԏ�
		PiP_GetFurikaeInfo( &wkReceipt );										// �U�֌������擾
		g_PipCtrl.stFurikaeInfo.PassUse		= vl_frs.antipassoff_req;			// �U�֌�������p�L��
		g_PipCtrl.stFurikaeInfo.SrcArea		= RecvNtnetDt.RData16_03.SrcArea;	// ���
		g_PipCtrl.stFurikaeInfo.SrcNo		= RecvNtnetDt.RData16_03.SrcNo;		// �Ԏ�
		g_PipCtrl.stFurikaeInfo.SrcFeeKind	= wkReceipt.syu;					// �������
		g_PipCtrl.stFurikaeInfo.SrcFee		= wkReceipt.WPrice;					// ���ԗ���
		for( i=0; i<WTIK_USEMAX; i++ ){
// �s��C��(S) K.Onodera 2016/11/24 #1585 �U�֌��������z�ɖ��֌W�̒l���Z�b�g�����
//			g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DiscountData[i].Discount;	// �����z���Z
			if( ( wkReceipt.DiscountData[i].DiscSyu != 0 ) &&					// ������ʂ���
			    (( wkReceipt.DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// ���Z���~�������łȂ�
			    ( NTNET_CFRE < wkReceipt.DiscountData[i].DiscSyu )) ){
			    if( wkReceipt.DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA�ȊO(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(�x���z�E�c�z)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO�ȊO(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(�x���z�E�c�z)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_EDY_1 &&			// EDY�ȊO	(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_EDY_2 &&			//			(�x���z�E�c�z)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA�ȊO(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(�x���z�E�c�z)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card�ȊO(�ԍ�)
			    	wkReceipt.DiscountData[i].DiscSyu != NTNET_ICCARD_2 &&		// 			(�x���z�E�c�z)
			   		wkReceipt.DiscountData[i].DiscSyu < NTNET_SECTION_WARI_MAX ) // �������Ƃ��Ďg�p����͎̂��001�`999�܂�
			    {
			    	g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DiscountData[i].Discount;	// �����z���Z
			    }
			}
// �s��C��(E) K.Onodera 2016/11/24 #1585 �U�֌��������z�ɖ��֌W�̒l���Z�b�g�����
		}
// �d�l�ύX(S) K.Onodera 2016/12/02 �U�֊z�ύX
		for( i=0; i<DETAIL_SYU_MAX; i++ ){
			if( wkReceipt.DiscountData[i].DiscSyu == NTNET_FURIKAE_2 ){
				// �U�֊z�����Z(���ԗ�����荂���Ȃ�ꍇ�����邪�A�}�C�i�X�Ƃ��Ĉ󎚂��Ă��邽��)
				g_PipCtrl.stFurikaeInfo.SrcFeeDiscount += wkReceipt.DetailData[i].uDetail.Furikae.Total;
			}
		}
// �d�l�ύX(E) K.Onodera 2016/12/02 �U�֊z�ύX
		memcpy( &g_PipCtrl.stFurikaeInfo.SrcPayTime, &wkReceipt.TOutTime, sizeof(date_time_rec) );	// ���Z���� �����܂�
		memcpy( &g_PipCtrl.stFurikaeInfo.SrcInTime,  &wkReceipt.TInTime, sizeof(date_time_rec) );		// ���Ɏ��� �����܂�
		// �U�֐�Ԏ�
		g_PipCtrl.stFurikaeInfo.DestArea	= RecvNtnetDt.RData16_03.DestArea;	// ���
		g_PipCtrl.stFurikaeInfo.DestNo		= RecvNtnetDt.RData16_03.DestNo;	// �Ԏ�

		// OPE�ɐU�֎��s�v��
		queset( OPETCBNO, OPE_REQ_FURIKAE_GO, 0, NULL );
	}
	// �`�F�b�NNG�H
	else{
		// NG����
		NTNET_Snd_Data16_04( rslt );
	}
}

///*[]----------------------------------------------------------------------[]*
// *| �U�֐��Z����
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
void NTNET_Snd_Data16_04( ushort rslt )
{
// MH322914(S) K.Onodera 2016/12/22 [�ÓI���-570]�z��C���f�b�N�X�I�[�o�[
	ushort size = 0;
// MH322914(E) K.Onodera 2016/12/22 [�ÓI���-570]�z��C���f�b�N�X�I�[�o�[

	// �����f�[�^���M
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_04 ) );

	BasicDataMake( PIP_RES_KIND_FURIKAE, 1 );									// ��{�ް��쐬 �ް��ێ��׸� = 1�Œ�

	SendNtnetDt.SData16_04.Common.FmtRev 		= 10;								// �t�H�[�}�b�gRev.No.(10�Œ�)
	SendNtnetDt.SData16_04.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// ���M��̒[���^�[�~�i�������Z�b�g����i99�����uPC�j
	SendNtnetDt.SData16_04.Common.CenterSeqNo	= RecvNtnetDt.RData16_03.Common.CenterSeqNo;	// �Z���^�[�ǔ�
	SendNtnetDt.SData16_04.RcvKind				= RecvNtnetDt.RData16_03.ReqKind;	// �������
	SendNtnetDt.SData16_04.Result				= rslt;								// ����

	// ���퉞��
	if( rslt == PIP_RES_RESULT_OK ){
// MH322914(S) K.Onodera 2016/12/22 [�ÓI���-575]�C���f�b�N�X�I�[�o�[
//		memcpy( &SendNtnetDt.SData16_04.OpenClose, &g_PipCtrl.stFurikaeInfo.OpenClose, sizeof(g_PipCtrl.stFurikaeInfo) );
		size =  (ushort)_offsetof(DATA_KIND_16_04, OpenClose);
		size = ( sizeof(DATA_KIND_16_04) - size );
		memcpy( &SendNtnetDt.SData16_04.OpenClose, &g_PipCtrl.stFurikaeInfo.OpenClose, size );
// MH322914(E) K.Onodera 2016/12/22 [�ÓI���-575]�z��C���f�b�N�X�I�[�o�[
		SendNtnetDt.SData16_04.stDestInfo.SrcFeeDiscount = vl_frs.furikaegaku;	// �U�։\�z���Z�b�g
	}
	else{
		PiP_FurikaeSts = PIP_FRK_STS_IDLE;
	}
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_04 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_04 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
}


//[]----------------------------------------------------------------------[]
///	@brief		���u���Z�v����M
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_08_RemoteCalc( void )
{
	switch( RecvNtnetDt.RData16_07.ReqKind ){
		case PIP_REMOTE_TYPE_CALC:		// �����v�Z
		case PIP_REMOTE_TYPE_CALC_TIME:	// ���Ɏ����w�艓�u���Z
			NTNET_RevData16_08_RemoteCalc_Time();
			break;
		case PIP_REMOTE_TYPE_CALC_FEE:	// ���Z���z�w�艓�u���Z
			NTNET_RevData182_RemoteCalcFee();
			break;
		default:
			// NG����
			NTNET_Snd_Data16_08( PIP_RES_RESULT_NG_PARAM );	// �p�����[�^�s��
			break;
	}
}


//[]----------------------------------------------------------------------[]
///	@brief		���u���Z�����v�Z/���Ɏ��Ԏw�萸�Z�v��
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
static void NTNET_RevData16_08_RemoteCalc_Time( void )
{
	ushort			rslt = PIP_RES_RESULT_OK;
	ushort			ndat = 0;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	ushort 			pos = 0;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	// �𗝃`�F�b�N
	do{
		// 1.�N�����`�F�b�N
		if( chkdate( (short)RecvNtnetDt.RData16_07.InTime.Year, (short)RecvNtnetDt.RData16_07.InTime.Mon, (short)RecvNtnetDt.RData16_07.InTime.Day ) ){
			// NG
			rslt = PIP_RES_RESULT_NG_PARAM;		// �p�����[�^�s��
			break;
		}
		// 2.�����b�`�F�b�N
		if( RecvNtnetDt.RData16_07.InTime.Hour > 23 ||
			RecvNtnetDt.RData16_07.InTime.Min  > 59 ||
			RecvNtnetDt.RData16_07.InTime.Sec  > 59 ){
			// NG
			rslt = PIP_RES_RESULT_NG_PARAM;		// �p�����[�^�s��
			break;
		}
		// 3.���t�`�F�b�N
		ndat = dnrmlzm( (short)RecvNtnetDt.RData16_07.InTime.Year, (short)RecvNtnetDt.RData16_07.InTime.Mon, (short)RecvNtnetDt.RData16_07.InTime.Day );
		// �����H
		if( CLK_REC.ndat < ndat ){
			// NG
			rslt = PIP_RES_RESULT_NG_PARAM;		// �p�����[�^�s��
			break;
		}
		// �����H
		else if( CLK_REC.ndat == ndat ){
			// �����H
			if( CLK_REC.nmin < tnrmlz ( (short)0, (short)0, (short)RecvNtnetDt.RData16_07.InTime.Hour, (short)RecvNtnetDt.RData16_07.InTime.Min ) ){
				// NG
				rslt = PIP_RES_RESULT_NG_PARAM;		// �p�����[�^�s��
				break;
			}
		}
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//		// 4.�Ԏ���ԃ`�F�b�N
//		rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_07.Area, RecvNtnetDt.RData16_07.ulNo, &pos );
//		if( rslt != PIP_RES_RESULT_OK ){
//			break;
//		}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		// 5.��ԃ`�F�b�N
		if( OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod > 1 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// ��t�s���
			break;
		}
		// 6.�����W�v���`�F�b�N
		if( auto_syu_prn == 2 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// ��t�s���
			break;
		}

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
		if ( !CLOUD_CALC_MODE ) {	// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		// ������ʃ`�F�b�N
		if( prm_get( COM_PRM,S_SHA,(short)(1+6*(RecvNtnetDt.RData16_07.Syubet-1)),2,5 ) == 0L ){	// �Ԏ�ݒ�Ȃ��i�g�p����ݒ�łȂ��j
			// NG
			rslt = PIP_RES_RESULT_NG_PARAM;		// �p�����[�^�s��
			break;
		}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
		}
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

		// 7.�����敪�A��ʃ`�F�b�N
		if( RecvNtnetDt.RData16_07.stDiscount.ParkNo ){
			if( 0 == RecvNtnetDt.RData16_07.stDiscount.DiscSyubet ||
				(2 == RecvNtnetDt.RData16_07.stDiscount.DiscSyubet && 0 == RecvNtnetDt.RData16_07.stDiscount.DiscType) ){
				// NG
				rslt = PIP_RES_RESULT_NG_PARAM;		// �p�����[�^�s��
				break;
			}
		}
	}while(0);

	// ���u���Z�����w��
	if( rslt == PIP_RES_RESULT_OK ){
		g_PipCtrl.stRemoteTime.ReqKind = UNI_KIND_REMOTE_TIME;
// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
		g_PipCtrl.stRemoteTime.ulPno = RecvNtnetDt.RData16_07.DataBasic.ParkingNo;				// ���ԏꇂ
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//		g_PipCtrl.stRemoteTime.Area = RecvNtnetDt.RData16_07.Area;
//		g_PipCtrl.stRemoteTime.ulNo = RecvNtnetDt.RData16_07.ulNo;
		g_PipCtrl.stRemoteTime.RyoSyu = RecvNtnetDt.RData16_07.Syubet;							// �������
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		memcpy( &g_PipCtrl.stRemoteTime.InTime, &RecvNtnetDt.RData16_07.InTime, sizeof(g_PipCtrl.stRemoteTime.InTime) );
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		g_PipCtrl.stRemoteTime.FutureFee = RecvNtnetDt.RData16_07.FutureFee;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		// �������
		g_PipCtrl.stRemoteTime.DiscountKind = RecvNtnetDt.RData16_07.stDiscount.DiscSyubet;		// �������
		g_PipCtrl.stRemoteTime.DiscountType = RecvNtnetDt.RData16_07.stDiscount.DiscType;		// �����敪
		g_PipCtrl.stRemoteTime.DiscountCnt = RecvNtnetDt.RData16_07.stDiscount.DiscCount;		// �����g�p����
		g_PipCtrl.stRemoteTime.Discount = RecvNtnetDt.RData16_07.stDiscount.DiscPrice;			// �������z
		g_PipCtrl.stRemoteTime.DiscountInfo1 = RecvNtnetDt.RData16_07.stDiscount.DiscInfo1;		// �������P
		g_PipCtrl.stRemoteTime.DiscountInfo2 = RecvNtnetDt.RData16_07.stDiscount.DiscInfo2;		// �������Q
		// OPE�֒ʒm
		if( RecvNtnetDt.RData16_07.ReqKind == PIP_REMOTE_TYPE_CALC ){
			queset( OPETCBNO, OPE_REQ_REMOTE_CALC_TIME_PRE, 0, NULL );
		}else{
			queset( OPETCBNO, OPE_REQ_REMOTE_CALC_TIME, 0, NULL );
		}
	}else{
		// NG�����f�[�^���M
		NTNET_Snd_Data16_08( rslt );
	}
}

///*[]----------------------------------------------------------------------[]*
// *| ���u���Z���Ɏ����w�艞��
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
void NTNET_Snd_Data16_08( ushort rslt )
{
	ulong	intime, outtime;
	ushort	indate, outdate;


	// �����f�[�^���M
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_08 ) );

	BasicDataMake( PIP_RES_KIND_REMOTE_CALC_ENTRY, 1 );								// ��{�ް��쐬 �ް��ێ��׸� = 1�Œ�

	SendNtnetDt.SData16_08.Common.FmtRev 		= 10;								// �t�H�[�}�b�gRev.No.(10�Œ�)
	SendNtnetDt.SData16_08.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// ���M��̒[���^�[�~�i�������Z�b�g����i99�����uPC�j
	SendNtnetDt.SData16_08.Common.CenterSeqNo	= RecvNtnetDt.RData16_07.Common.CenterSeqNo;	// �Z���^�[�ǔ�
	SendNtnetDt.SData16_08.RcvKind		= RecvNtnetDt.RData16_07.ReqKind;			// �������
	SendNtnetDt.SData16_08.Result				= rslt;								// ����

	// ���펞�ɃZ�b�g -----------------
	if( rslt == PIP_RES_RESULT_OK ){
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//		SendNtnetDt.SData16_08.Area					= RecvNtnetDt.RData16_07.Area;		// ���
//		SendNtnetDt.SData16_08.ulNo					= RecvNtnetDt.RData16_07.ulNo;		// �Ԏ�
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		memcpy( &SendNtnetDt.SData16_08.InTime, &RecvNtnetDt.RData16_07.InTime, sizeof(SendNtnetDt.SData16_08.InTime) );	// ���Ɏ���
		memcpy( &SendNtnetDt.SData16_08.stDiscount, &RecvNtnetDt.RData16_07.stDiscount, sizeof(SendNtnetDt.SData16_08.stDiscount) );	// �������
		SendNtnetDt.SData16_08.stDiscount.DiscPrice = g_PipCtrl.stRemoteTime.Discount;
		SendNtnetDt.SData16_08.Syubet			= g_PipCtrl.stRemoteTime.RyoSyu;
		SendNtnetDt.SData16_08.Price			= g_PipCtrl.stRemoteTime.Price;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		SendNtnetDt.SData16_08.FutureFee		= RecvNtnetDt.RData16_07.FutureFee;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		memcpy( &SendNtnetDt.SData16_08.PayTime, &g_PipCtrl.stRemoteTime.OutTime, sizeof(SendNtnetDt.SData16_08.PayTime) );	// �����v�Z����

		// ���ɓ��v�Z
		indate	= dnrmlzm( (short)SendNtnetDt.SData16_08.InTime.Year, (short)SendNtnetDt.SData16_08.InTime.Mon, (short)SendNtnetDt.SData16_08.InTime.Day );
		// �o�ɓ��v�Z
		outdate	= dnrmlzm( (short)SendNtnetDt.SData16_08.PayTime.Year, (short)SendNtnetDt.SData16_08.PayTime.Mon, (short)SendNtnetDt.SData16_08.PayTime.Day );

		// ���ɓ����i�b���Z�j�v�Z
		intime	=	(indate*24*60*60) +								// �N����
					(SendNtnetDt.SData16_08.InTime.Hour*60*60) +	// ��
					(SendNtnetDt.SData16_08.InTime.Min*60) +		// ��
					(SendNtnetDt.SData16_08.InTime.Sec);			// �b
		// �o�ɓ����i�b���Z�j�v�Z
		outtime	=	(outdate*24*60*60) +
					(SendNtnetDt.SData16_08.PayTime.Hour*60*60) +
					(SendNtnetDt.SData16_08.PayTime.Min*60) +
					(SendNtnetDt.SData16_08.PayTime.Sec);
		// �o�ɓ��������ɓ���
		if( outtime >= intime ){
			SendNtnetDt.SData16_08.ParkTotalSec	= outtime - intime;												// ���Ԏ��Ԏ擾�i�b���Z�j
			SendNtnetDt.SData16_08.ParkTimeDay	= SendNtnetDt.SData16_08.ParkTotalSec/(60*60*24);				// ���Ԏ��ԁi���j�v�Z
			SendNtnetDt.SData16_08.ParkTimeHour	= SendNtnetDt.SData16_08.ParkTotalSec%(60*60*24)/(60*60);		// ���Ԏ��ԁi���j�v�Z
			SendNtnetDt.SData16_08.ParkTimeMin	= SendNtnetDt.SData16_08.ParkTotalSec%(60*60)/60;				// ���Ԏ��ԁi���j�v�Z
			SendNtnetDt.SData16_08.ParkTimeSec	= SendNtnetDt.SData16_08.ParkTotalSec%60;						// ���Ԏ��ԁi�b�j�v�Z
		}
	}
	
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_08 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_08 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
}



///*[]----------------------------------------------------------------------[]*
// *| ���u���Z���z�w�萸�Z�v��
// *[]----------------------------------------------------------------------[]*
// *| MODULE NAME  : NTNET_RevData182_RemoteCalcFee
// *| PARAMETER    : void
// *| RETURN VALUE : void
// *[]----------------------------------------------------------------------[]*
// *| UpDate       : 
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
static void NTNET_RevData182_RemoteCalcFee(void)
{
	ushort			rslt = PIP_RES_RESULT_OK;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	ushort 			pos = 0;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	// �𗝃`�F�b�N
	do{
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//		// 1.�Ԏ���ԃ`�F�b�N
//// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
////		rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_07.Area, RecvNtnetDt.RData16_07.ulNo, &pos );
//		// ������Z�H
//		if( RecvNtnetDt.RData16_07.Type ){
//			//if( RecvNtnetDt.RData16_07.ulNo != 9900 ){
//			//	rslt = PIP_RES_RESULT_NG_PARAM;
//			//}
//		}
//		// �o�ɐ��Z�H
//		else{
//			rslt = PIP_CheckCarInfo( RecvNtnetDt.RData16_07.Area, RecvNtnetDt.RData16_07.ulNo, &pos );
//		}
//// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//		if( rslt != PIP_RES_RESULT_OK ){
//			break;
//		}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		// 2.��ԃ`�F�b�N
		if( OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod > 1 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// ��t�s���
			break;
		}
		// 3.�����W�v���`�F�b�N
		if( auto_syu_prn == 2 ){
			rslt = PIP_RES_RESULT_NG_DENY;		// ��t�s���
			break;
		}
		// 4.�����敪�A��ʃ`�F�b�N
		if( RecvNtnetDt.RData16_07.stDiscount.ParkNo ){
			if( 0 == RecvNtnetDt.RData16_07.stDiscount.DiscSyubet ||
				(2 == RecvNtnetDt.RData16_07.stDiscount.DiscSyubet && 0 == RecvNtnetDt.RData16_07.stDiscount.DiscType) ){
				// NG
				rslt = PIP_RES_RESULT_NG_PARAM;		// �p�����[�^�s��
				break;
			}
		}
	}while(0);

	// ���u���Z���z�w��
	if( rslt == PIP_RES_RESULT_OK ){
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		g_PipCtrl.stRemoteFee.ReqKind = UNI_KIND_REMOTE_FEE;
		g_PipCtrl.stRemoteFee.Type = RecvNtnetDt.RData16_07.Type;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
// �d�l�ύX(S) K.Onodera 2016/11/07 ������Z�Ή�
		g_PipCtrl.stRemoteFee.RyoSyu = RecvNtnetDt.RData16_07.Syubet;
// �d�l�ύX(E) K.Onodera 2016/11/07 ������Z�Ή�
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//		g_PipCtrl.stRemoteFee.Area = RecvNtnetDt.RData16_07.Area;
//		g_PipCtrl.stRemoteFee.ulNo = RecvNtnetDt.RData16_07.ulNo;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		g_PipCtrl.stRemoteFee.Price = RecvNtnetDt.RData16_07.Price;
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		g_PipCtrl.stRemoteFee.FutureFee = RecvNtnetDt.RData16_07.FutureFee;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		// �������
		g_PipCtrl.stRemoteFee.DiscountKind = RecvNtnetDt.RData16_07.stDiscount.DiscSyubet;	// �������
		g_PipCtrl.stRemoteFee.DiscountType = RecvNtnetDt.RData16_07.stDiscount.DiscType;	// �����敪
		g_PipCtrl.stRemoteFee.DiscountCnt = RecvNtnetDt.RData16_07.stDiscount.DiscCount;	// �����g�p����
		g_PipCtrl.stRemoteFee.Discount = RecvNtnetDt.RData16_07.stDiscount.DiscPrice;		// �������z
		g_PipCtrl.stRemoteFee.DiscountInfo1 = RecvNtnetDt.RData16_07.stDiscount.DiscInfo1;	// �������P
		g_PipCtrl.stRemoteFee.DiscountInfo2 = RecvNtnetDt.RData16_07.stDiscount.DiscInfo2;	// �������Q
		// OPE�֒ʒm
		queset( OPETCBNO, OPE_REQ_REMOTE_CALC_FEE, 0, NULL );
	}else{
		// NG�����f�[�^���M
		NTNET_Snd_Data16_09(rslt);
	}
}

///*[]----------------------------------------------------------------------[]*
// *| ���u���Z���z�w�艞��
// *[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]*/
void NTNET_Snd_Data16_09( ushort rslt )
{
	// �����f�[�^���M
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_16_09 ) );

	BasicDataMake( PIP_RES_KIND_REMOTE_CALC_FEE, 1 );								// ��{�ް��쐬 �ް��ێ��׸� = 1�Œ�

	SendNtnetDt.SData16_09.Common.FmtRev 		= 10;								// �t�H�[�}�b�gRev.No.(10�Œ�)
	SendNtnetDt.SData16_09.Common.SMachineNo 	= REMOTE_PC_TERMINAL_NO;			// ���M��̒[���^�[�~�i�������Z�b�g����i99�����uPC�j
	SendNtnetDt.SData16_09.Common.CenterSeqNo	= RecvNtnetDt.RData16_07.Common.CenterSeqNo;	// �Z���^�[�ǔ�
	SendNtnetDt.SData16_09.Result				= rslt;								// ����
	// ���펞�ɃZ�b�g -----------------
	if( rslt == PIP_RES_RESULT_OK ){
// �d�l�ύX(S) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		SendNtnetDt.SData16_09.Type					= RecvNtnetDt.RData16_07.Type;		// ���u���Z���
// �d�l�ύX(E) K.Onodera 2016/10/25 ���u���Z�t�H�[�}�b�g�ύX�Ή�
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//		SendNtnetDt.SData16_09.Area					= RecvNtnetDt.RData16_07.Area;		// ���
//		SendNtnetDt.SData16_09.ulNo					= RecvNtnetDt.RData16_07.ulNo;		// �Ԏ�
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		SendNtnetDt.SData16_09.Syubet				= g_PipCtrl.stRemoteFee.RyoSyu;		// �������
		SendNtnetDt.SData16_09.Price				= RecvNtnetDt.RData16_07.Price;		// ���ԗ���
// �d�l�ύX(S) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		SendNtnetDt.SData16_09.FutureFee			= RecvNtnetDt.RData16_07.FutureFee;
// �d�l�ύX(E) K.Onodera 2016/11/04 ���u���Z�t�H�[�}�b�g�ύX�Ή�
		memcpy( &SendNtnetDt.SData16_09.stDiscount, &RecvNtnetDt.RData16_07.stDiscount, sizeof(SendNtnetDt.SData16_09.stDiscount) );	// �������
		SendNtnetDt.SData16_09.stDiscount.DiscPrice = g_PipCtrl.stRemoteFee.Discount;
	}

	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof( DATA_KIND_16_09 ));
	}else {
		NTBUF_SetSendNtData( &SendNtnetDt, sizeof( DATA_KIND_16_09 ), NTNET_BUF_BUFFERING );	// �ް����M�o�^
	}
}
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
////[]----------------------------------------------------------------------[]
/////	@brief		���Z�f�[�^(Rev.14�F�t�F�[�Y7)
////[]----------------------------------------------------------------------[]
/////	@param[in]	p_RcptDat	�F���Z���O�f�[�^
/////	@param[out]	p_NtDat		�F���ZNT-NET�f�[�^
/////	@return		none
////[]----------------------------------------------------------------------[]
/////	@date		Create	: 2016/11/01<br>
/////				Update	: 
////[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
//unsigned short	NTNET_Edit_Data56_r14( Receipt_data *p_RcptDat, DATA_KIND_56_r14 *p_NtDat )
//{
//	uchar		i;
//	uchar		j;
//	ushort		ret;
//	EMONEY_r14	t_EMoney;
//	CREINFO_r14	t_Credit;
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
////	date_time_rec	wk_CLK_REC;
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_56_r14 ) );
//
//	NTNET_Edit_BasicData( 57, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// ��{�ް��쐬
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TOutTime.Year % 100);		// ���Z�N
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TOutTime.Mon;				// ���Z��
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TOutTime.Day;				// ���Z��
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TOutTime.Hour;				// ���Z��
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TOutTime.Min;				// ���Z��
//	p_NtDat->DataBasic.Sec	= 0;
//	p_NtDat->FmtRev = 14;													// �t�H�[�}�b�gRev.��
//	p_NtDat->CenterSeqNo = p_RcptDat->CenterSeqNo;							// �Z���^�[�ǔԁi���Z�j
//
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->FullNo1;						// ���ԑ䐔�P
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CarCnt1;						// ���ݑ䐔�P
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->FullNo2;						// ���ԑ䐔�Q
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CarCnt2;						// ���ݑ䐔�Q
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->FullNo3;						// ���ԑ䐔�R
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CarCnt3;						// ���ݑ䐔�R
//	p_NtDat->ParkData.CarFullFlag	= (uchar)0;								// �䐔�E�����ԃt���O
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->PascarCnt;				// ����ԗ��J�E���g
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->Full[0];						// ����1������
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->Full[1];						// ����2������
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->Full[2];						// ����3������
//
//	// �䐔�Ǘ��ǔ� ------------------------
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
////	memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );
////	// ���ꎞ���H
////	if( VehicleCountDateTime == Nrm_YMDHM( &wk_CLK_REC ) ){
////		// ���ꎞ���̏ꍇ�͒ǂ��Ԃ��{�P
////		VehicleCountSeqNo++;
////	}else{
////		// �قȂ鎞���̏ꍇ�͒ǂ��Ԃ��O�Ƃ���
////		VehicleCountDateTime = Nrm_YMDHM( &wk_CLK_REC );
////		VehicleCountSeqNo = 0;
////	}
////	p_NtDat->ParkData.CarCntInfo.CarCntYear		= (wk_CLK_REC.Year % 100);	// �N
////	p_NtDat->ParkData.CarCntInfo.CarCntMon		= wk_CLK_REC.Mon;			// ��
////	p_NtDat->ParkData.CarCntInfo.CarCntDay		= wk_CLK_REC.Day;			// ��
////	p_NtDat->ParkData.CarCntInfo.CarCntHour		= wk_CLK_REC.Hour;			// ��
////	p_NtDat->ParkData.CarCntInfo.CarCntMin		= wk_CLK_REC.Min;			// ��
////	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= VehicleCountSeqNo;		// �ǔ�
////	p_NtDat->ParkData.CarCntInfo.Reserve1		= 0;						// �\��
//	p_NtDat->ParkData.CarCntInfo.CarCntYear		= p_RcptDat->CarCntInfo.CarCntYear;	// �N
//	p_NtDat->ParkData.CarCntInfo.CarCntMon		= p_RcptDat->CarCntInfo.CarCntMon;	// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntDay		= p_RcptDat->CarCntInfo.CarCntDay;	// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntHour		= p_RcptDat->CarCntInfo.CarCntHour;	// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntMin		= p_RcptDat->CarCntInfo.CarCntMin;	// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= p_RcptDat->CarCntInfo.CarCntSeq;	// �ǔ�
//	p_NtDat->ParkData.CarCntInfo.Reserve1		= p_RcptDat->CarCntInfo.Reserve1;	// �\��
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//	// -------------------------------------
//
//	p_NtDat->PayCount	= CountSel( &p_RcptDat->Oiban);						// ���Zor���Z���~�ǂ���
//	p_NtDat->PayMethod	= (uchar)p_RcptDat->PayMethod;						// ���Z���@
//	if( p_RcptDat->PayClass == 4 ){
//		// �N���W�b�g���Z
//		p_NtDat->PayClass	= (uchar)0;										// �����敪
//	}else{
//		p_NtDat->PayClass	= (uchar)p_RcptDat->PayClass;					// �����敪
//	}
//	p_NtDat->PayMode	= (uchar)p_RcptDat->PayMode;						// ���ZӰ��(�������Z)
//	p_NtDat->CMachineNo	= 0;												// ���Ԍ��@�B��
//
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->FlapArea	= (ushort)(p_RcptDat->WPlace/10000);			// �t���b�v�V�X�e��	���		0�`99
//		p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->WPlace%10000);			// 					�Ԏ��ԍ�	0�`9999
//		p_NtDat->KakariNo	= p_RcptDat->KakariNo;							// �W����
//		p_NtDat->OutKind	= p_RcptDat->OutKind;							// ���Z�o��
//	}
//
//	p_NtDat->CountSet	= p_RcptDat->CountSet;								// �ݎԃJ�E���g���Ȃ�
//
//	if( p_NtDat->PayMethod != 5 ){
//		p_NtDat->InPrev_ymdhms.In_Year	= (uchar)(p_RcptDat->TInTime.Year%100);	// ���ɔN
//		p_NtDat->InPrev_ymdhms.In_Mon	= p_RcptDat->TInTime.Mon;				// ���Ɍ�
//		p_NtDat->InPrev_ymdhms.In_Day	= p_RcptDat->TInTime.Day;				// ���ɓ�
//		p_NtDat->InPrev_ymdhms.In_Hour	= p_RcptDat->TInTime.Hour;				// ���Ɏ�
//		p_NtDat->InPrev_ymdhms.In_Min	= p_RcptDat->TInTime.Min;				// ���ɕ�
//		p_NtDat->InPrev_ymdhms.In_Sec	= 0;									// ���ɕb
//
//		// �O�񐸎Z�N���������b��0�Ƃ���
//		p_NtDat->InPrev_ymdhms.Prev_Year	= 0;							// �O�񐸎Z�N
//		p_NtDat->InPrev_ymdhms.Prev_Mon		= 0;							// �O�񐸎Z��
//		p_NtDat->InPrev_ymdhms.Prev_Day		= 0;							// �O�񐸎Z��
//		p_NtDat->InPrev_ymdhms.Prev_Hour	= 0;							// �O�񐸎Z��
//		p_NtDat->InPrev_ymdhms.Prev_Min		= 0;							// �O�񐸎Z��
//		p_NtDat->InPrev_ymdhms.Prev_Sec		= 0;							// �O�񐸎Z�b
//	}
//
//	p_NtDat->ReceiptIssue	= p_RcptDat->ReceiptIssue;						// �̎��ؔ��s�L��
//
//	if( p_NtDat->PayMethod != 5 ){											// �X�V���Z�ȊO
//		p_NtDat->Syubet	= p_RcptDat->syu;									// �������
//		p_NtDat->Price	= p_RcptDat->WPrice;								// ���ԗ���
//	}
//
//	p_NtDat->CashPrice	= p_RcptDat->WTotalPrice;							// ��������
//	p_NtDat->InPrice	= p_RcptDat->WInPrice;								// �������z
//// �d�l�ύX(S) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
////	p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// �ޑK���z
//	if( p_RcptDat->FRK_RetMod == 0 ){
//		p_NtDat->ChgPrice	= (ushort)(p_RcptDat->WChgPrice + p_RcptDat->FRK_Return);	// �ޑK���z + �U�։ߕ�����
//	}else{
//		p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// �ޑK���z
//	}
//// �d�l�ύX(E) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
//
//	p_NtDat->MoneyInOut.MoneyKind_In	= 0x1f;								// ����L��	���ڋ���̗L���@�������i10�~,50�~,100�~,500�~,1000�~�j
//	p_NtDat->MoneyInOut.In_10_cnt		= (uchar)p_RcptDat->in_coin[0];		// ����������(10�~)		0�`255
//	p_NtDat->MoneyInOut.In_50_cnt		= (uchar)p_RcptDat->in_coin[1];		// ����������(50�~)		0�`255
//	p_NtDat->MoneyInOut.In_100_cnt		= (uchar)p_RcptDat->in_coin[2];		// ����������(100�~)	0�`255
//	p_NtDat->MoneyInOut.In_500_cnt		= (uchar)p_RcptDat->in_coin[3];		// ����������(500�~)	0�`255
//	p_NtDat->MoneyInOut.In_1000_cnt		= (uchar)p_RcptDat->in_coin[4];		// ����������(1000�~)	0�`255
//	if(p_RcptDat->f_escrow) {												// �G�X�N�������߂�����
//		++p_NtDat->MoneyInOut.In_1000_cnt;									// �G�X�N�������߂����𓊓������ɉ��Z
//	}
//
//	p_NtDat->MoneyInOut.MoneyKind_Out	= 0x1f;								// ����L��	���ڋ���̗L���@���o���i10�~,50�~,100�~,500�~,1000�~�j
//	p_NtDat->MoneyInOut.Out_10_cnt		= (uchar)p_RcptDat->out_coin[0];	// ���o������(10�~)		0�`255
//	p_NtDat->MoneyInOut.Out_50_cnt		= (uchar)p_RcptDat->out_coin[1];	// ���o������(50�~)		0�`255
//	p_NtDat->MoneyInOut.Out_100_cnt		= (uchar)p_RcptDat->out_coin[2];	// ���o������(100�~)	0�`255
//	p_NtDat->MoneyInOut.Out_500_cnt		= (uchar)p_RcptDat->out_coin[3];	// ���o������(500�~)	0�`255
//	p_NtDat->MoneyInOut.Out_1000_cnt	= (uchar)p_RcptDat->f_escrow;		// ���o������(1000�~)	0�`255
//
//	p_NtDat->HaraiModoshiFusoku	= (ushort)p_RcptDat->WFusoku;				// ���ߕs���z	0�`9999
//	if( p_RcptDat->FRK_Return && p_RcptDat->FRK_RetMod ){
//		p_NtDat->CardFusokuType = p_RcptDat->FRK_RetMod;					// ���o�s���}�̎��(�����ȊO)
//		p_NtDat->CardFusokuTotal = p_RcptDat->FRK_Return;					// ���o�s���z(�����ȊO)
//	}
//	p_NtDat->SaleParkingNo = (ulong)CPrmSS[S_SYS][1];						// ����撓�ԏꇂ(0�`999999   �������p�i�W���ł͊�{���ԏꇂ���Z�b�g�j
//
//	p_NtDat->PassCheck = p_RcptDat->PassCheck;								// �A���`�p�X�`�F�b�N
//
//	if( p_RcptDat->teiki.ParkingNo ) {
//		p_NtDat->ParkingNo	= p_RcptDat->teiki.ParkingNo;								// ��������ԏꇂ
//		p_NtDat->Media[0].MediaKind	= (ushort)(p_RcptDat->teiki.pkno_syu + 2);			// ���Z�}�̏��P�@���(���C���}��)	0�`99
//		intoasl(p_NtDat->Media[0].MediaCardNo, p_RcptDat->teiki.id, 5);					// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		intoasl(p_NtDat->Media[0].MediaCardInfo, p_RcptDat->teiki.syu, 2);				// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		// ���Z�}�̏��2�i�T�u�}�́j�� �I�[���O�Ƃ���
//	}
//
//	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 ){	// �d�q���ώ�� Suica:1, Edy:2
//		p_NtDat->CardKind = (ushort)2;											// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//	}else if( p_RcptDat->PayClass == 4 ){					// �N���W�b�g���Z
//		p_NtDat->CardKind = (ushort)1;											// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//	}
//
//	switch( p_NtDat->CardKind ){							// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
//	case 1:
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// �Í�������
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// �Í����ԍ�
//		memset( &t_Credit, 0, sizeof( t_Credit ) );
//		t_Credit.amount = (ulong)p_RcptDat->credit.pay_ryo;									// ���ϊz
//		memcpyFlushLeft( &t_Credit.card_no[0], (uchar *)&p_RcptDat->credit.card_no[0], sizeof(t_Credit.card_no), sizeof(p_RcptDat->credit.card_no) );	// ����ԍ�
//		memcpyFlushLeft( &t_Credit.cct_num[0], (uchar *)&p_RcptDat->credit.CCT_Num[0],
//							sizeof(t_Credit.cct_num), CREDIT_TERM_ID_NO_SIZE );				// �[�����ʔԍ�
//		memcpyFlushLeft( &t_Credit.kid_code[0], &p_RcptDat->credit.kid_code[0], sizeof(t_Credit.kid_code), sizeof(p_RcptDat->credit.kid_code) );		// KID �R�[�h
//		t_Credit.app_no = p_RcptDat->credit.app_no;											// ���F�ԍ�
//		t_Credit.center_oiban = 0;															// ���������ǔ�(�Z���^�[��0�Œ�)
//		memcpyFlushLeft( &t_Credit.ShopAccountNo[0], (uchar*)&p_RcptDat->credit.ShopAccountNo[0], sizeof(t_Credit.ShopAccountNo), sizeof(p_RcptDat->credit.ShopAccountNo) );	// �����X����ԍ�
//		t_Credit.slip_no = p_RcptDat->credit.slip_no;										// �`�[�ԍ�
//		
//		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );							// AES �Í���
//		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// ���Ϗ��
//		break;
//	case 2:
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// �Í�������
//		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// �Í����ԍ�
//		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
//		t_EMoney.amount = p_RcptDat->Electron_data.Suica.pay_ryo;							// �d�q���ϐ��Z���@���ϊz
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//		memcpy( &t_EMoney.card_id[0], &p_RcptDat->Electron_data.Suica.Card_ID[0],
//				sizeof(p_RcptDat->Electron_data.Suica.Card_ID) );							// �d�q���ϐ��Z���@����ID (Ascii 16��)
//		t_EMoney.card_zangaku = p_RcptDat->Electron_data.Suica.pay_after;					// �d�q���ϐ��Z���@���ό�c��
//		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );							// AES �Í���
//		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// ���Ϗ��
//		break;
//	}
//
//	p_NtDat->TotalSale		= (ulong)0;										// ���v���z(����)
//	p_NtDat->DeleteSeq		= (ulong)0;										// �폜�f�[�^�ǔ�
//
//	for( i = j = 0; i < WTIK_USEMAX; i++ ){									// ���Z���~�ȊO�̊������R�s�[
//		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&					// ������ʂ���
//		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// ���Z���~�������łȂ�
//		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
//		    if( p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA�ȊO(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(�x���z�E�c�z)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO�ȊO(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(�x���z�E�c�z)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY�ȊO	(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(�x���z�E�c�z)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA�ȊO(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(�x���z�E�c�z)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card�ȊO(�ԍ�)
//		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_2 ) 		//			(�x���z�E�c�z)
//		    {
//				p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DiscountData[i].ParkingNo;						// ���ԏ�No.
//				p_NtDat->SDiscount[j].Kind		= p_RcptDat->DiscountData[i].DiscSyu;						// �������
//				p_NtDat->SDiscount[j].Group		= p_RcptDat->DiscountData[i].DiscNo;						// �����敪
//				p_NtDat->SDiscount[j].Callback	= p_RcptDat->DiscountData[i].DiscCount;						// �������
//				p_NtDat->SDiscount[j].Amount	= p_RcptDat->DiscountData[i].Discount;						// �����z
//				p_NtDat->SDiscount[j].Info1		= p_RcptDat->DiscountData[i].DiscInfo1;						// �������1
//				if( p_NtDat->SDiscount[j].Kind != NTNET_PRI_W ){
//					p_NtDat->SDiscount[j].Info2	= p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;	// �������2
//				}
//				j++;
//		    }
//		}
//	}
//	for( i=0; i<DETAIL_SYU_MAX; i++ ){
//		if( p_RcptDat->DetailData[i].DiscSyu ){
//			p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DetailData[i].ParkingNo;					// ���ԏ�No.
//			p_NtDat->SDiscount[j].Kind		= p_RcptDat->DetailData[i].DiscSyu;						// �������
//			p_NtDat->SDiscount[j].Group		= p_RcptDat->DetailData[i].uDetail.Common.DiscNo;		// �����敪
//			p_NtDat->SDiscount[j].Callback	= p_RcptDat->DetailData[i].uDetail.Common.DiscCount;	// �������
//			p_NtDat->SDiscount[j].Amount	= p_RcptDat->DetailData[i].uDetail.Common.Discount;		// �����z
//			p_NtDat->SDiscount[j].Info1		= p_RcptDat->DetailData[i].uDetail.Common.DiscInfo1;	// �������1
//			p_NtDat->SDiscount[j].Info2		= p_RcptDat->DetailData[i].uDetail.Common.DiscInfo2;	// �������2
//			j++;
//		}
//	}
//
//// �s��C��(S) K.Onodera 2016/12/09 �A���]���w�E(0�J�b�g�O�̃T�C�Y������Ă���)
////	ret = sizeof( DATA_KIND_56_r10 );
//	ret = sizeof( DATA_KIND_56_r14 );
//// �s��C��(E) K.Onodera 2016/12/09 �A���]���w�E(0�J�b�g�O�̃T�C�Y������Ă���)
//
//	return ret;
//}
//
//[]----------------------------------------------------------------------[]
///	@brief		���Z�f�[�^(Rev.17�F�t�F�[�Y7)
//[]----------------------------------------------------------------------[]
///	@param[in]	p_RcptDat	�F���Z���O�f�[�^
///	@param[out]	p_NtDat		�F���ZNT-NET�f�[�^
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/11/01<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//unsigned short	NTNET_Edit_Data56_r17( Receipt_data *p_RcptDat, DATA_KIND_56_r17 *p_NtDat )
unsigned short	NTNET_Edit_Data56_rXX( Receipt_data *p_RcptDat, DATA_KIND_56_rXX *p_NtDat )
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
{
	uchar		i;
	uchar		j;
	ushort		ret;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	EMONEY_r14	t_EMoney;
//	CREINFO_r14 t_Credit;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810100(S) K.Onodera  2020/01/31 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	uchar		knd;
// MH810100(E) K.Onodera  2020/01/31 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH810100(S) K.Onodera  2020/02/07 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	t_MediaInfoCre2 EncMediaInfo;
// MH810100(E) K.Onodera  2020/02/07 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
// GG124100(S) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
	SHABAN_INFO	EncShabanInfo;
// GG124100(E) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	memset( p_NtDat, 0, sizeof( DATA_KIND_56_r17 ) );
	memset( p_NtDat, 0, sizeof( DATA_KIND_56_rXX ) );
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810100(S) K.Onodera  2020/01/31 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	NTNET_Edit_BasicData( 57, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// ��{�ް��쐬
	
	if( prm_get( COM_PRM, S_NTN, 27, 1, 1 ) == 0 ){	// ���Z�f�[�^ID�̎�舵�� = �o�����Z�@ID�ő��M�H
		knd = 57;	// EPS
	}else{
		knd = 56;	// CPS
	}
	NTNET_Edit_BasicData( knd, 0, p_RcptDat->SeqNo, &p_NtDat->DataBasic);	// ��{�ް��쐬
// MH810100(E) K.Onodera  2020/01/31 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH810100(S) 2020/08/31 #4787 ���A���^�C���ʐM�@�����Z�o�ɂ̐��Z�f�[�^�iID�F57�j�̐��Z�N���������b���o�ɔN���������b�Ɠ����ɂȂ��Ă���
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TOutTime.Year % 100);		// ���Z�N
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TOutTime.Mon;				// ���Z��
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TOutTime.Day;				// ���Z��
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TOutTime.Hour;				// ���Z��
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TOutTime.Min;				// ���Z��

// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//	// ���	(0=���O���Z�^1�������Z�o�ɐ��Z)
//	if(p_RcptDat->shubetsu == 0){
	// ���	(0=���O���Z�^1�������Z�o�ɐ��Z�^2=���u���Z(���Z���ύX))
	if(p_RcptDat->shubetsu == 0 || p_RcptDat->shubetsu == 2){
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
		p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TOutTime.Year % 100);		// ���Z�N
		p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TOutTime.Mon;				// ���Z��
		p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TOutTime.Day;				// ���Z��
		p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TOutTime.Hour;				// ���Z��
		p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TOutTime.Min;				// ���Z��
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
		p_NtDat->DataBasic.Sec	= (uchar)p_RcptDat->TOutTime_Sec;				// ���Z�b
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j

	}else{
		p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->TUnpaidPayTime.Year % 100);	// ���Z�N
		p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->TUnpaidPayTime.Mon;				// ���Z��
		p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->TUnpaidPayTime.Day;				// ���Z��
		p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->TUnpaidPayTime.Hour;			// ���Z��
		p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->TUnpaidPayTime.Min;				// ���Z��
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
		p_NtDat->DataBasic.Sec	= (uchar)p_RcptDat->TUnpaidPayTime_Sec;				// ���Z�b
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j

	}
// MH810100(E) 2020/08/31 #4787 ���A���^�C���ʐM�@�����Z�o�ɂ̐��Z�f�[�^�iID�F57�j�̐��Z�N���������b���o�ɔN���������b�Ɠ����ɂȂ��Ă���
// GG129000(S) R.Endo 2023/02/16 �Ԕԃ`�P�b�g���X4.0 #6937 NTNET�̐��Z�f�[�^�̏����b�Ƀf�[�^�����Ȃ�
// 	p_NtDat->DataBasic.Sec	= 0;
// GG129000(E) R.Endo 2023/02/16 �Ԕԃ`�P�b�g���X4.0 #6937 NTNET�̐��Z�f�[�^�̏����b�Ƀf�[�^�����Ȃ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	p_NtDat->FmtRev = 17;													// �t�H�[�}�b�gRev.��
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��iRev.No.�X�V�j
//	p_NtDat->FmtRev = 18;													// �t�H�[�}�b�gRev.��
	p_NtDat->FmtRev = 20;													// �t�H�[�}�b�gRev.��
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��iRev.No.�X�V�j
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	p_NtDat->CenterSeqNo = p_RcptDat->CenterSeqNo;							// �Z���^�[�ǔԁi���Z�j

// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->FullNo1;						// ���ԑ䐔�P
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CarCnt1;						// ���ݑ䐔�P
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->FullNo2;						// ���ԑ䐔�Q
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CarCnt2;						// ���ݑ䐔�Q
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->FullNo3;						// ���ԑ䐔�R
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CarCnt3;						// ���ݑ䐔�R
//	p_NtDat->ParkData.CarFullFlag	= (uchar)0;								// �䐔�E�����ԃt���O
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->PascarCnt;				// ����ԗ��J�E���g
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->Full[0];						// ����1������
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->Full[1];						// ����2������
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->Full[2];						// ����3������
//
//	// �䐔�Ǘ��ǔ� ------------------------
//	p_NtDat->ParkData.CarCntInfo.CarCntYear		= p_RcptDat->CarCntInfo.CarCntYear;	// �N
//	p_NtDat->ParkData.CarCntInfo.CarCntMon		= p_RcptDat->CarCntInfo.CarCntMon;	// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntDay		= p_RcptDat->CarCntInfo.CarCntDay;	// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntHour		= p_RcptDat->CarCntInfo.CarCntHour;	// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntMin		= p_RcptDat->CarCntInfo.CarCntMin;	// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= p_RcptDat->CarCntInfo.CarCntSeq;	// �ǔ�
//	p_NtDat->ParkData.CarCntInfo.Reserve1		= p_RcptDat->CarCntInfo.Reserve1;	// �\��
//	// -------------------------------------
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH810100(S) 2020/07/09 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX �䐔�Ǘ��ǔԂ�䐔�Ǘ������Ȃ��Ă��t�^����)
	// �䐔�Ǘ��ǔ� ------------------------
	p_NtDat->ParkData.CarCntInfo.CarCntYear		= p_RcptDat->CarCntInfo.CarCntYear;	// �N
	p_NtDat->ParkData.CarCntInfo.CarCntMon		= p_RcptDat->CarCntInfo.CarCntMon;	// ��
	p_NtDat->ParkData.CarCntInfo.CarCntDay		= p_RcptDat->CarCntInfo.CarCntDay;	// ��
	p_NtDat->ParkData.CarCntInfo.CarCntHour		= p_RcptDat->CarCntInfo.CarCntHour;	// ��
	p_NtDat->ParkData.CarCntInfo.CarCntMin		= p_RcptDat->CarCntInfo.CarCntMin;	// ��
	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= p_RcptDat->CarCntInfo.CarCntSeq;	// �ǔ�
	p_NtDat->ParkData.CarCntInfo.Reserve1		= p_RcptDat->CarCntInfo.Reserve1;	// �\��
// MH810100(E) 2020/07/09 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX �䐔�Ǘ��ǔԂ�䐔�Ǘ������Ȃ��Ă��t�^����)

	p_NtDat->PayCount	= CountSel( &p_RcptDat->Oiban);						// ���Zor���Z���~�ǂ���
	p_NtDat->PayMethod	= (uchar)p_RcptDat->PayMethod;						// ���Z���@
	if( p_RcptDat->PayClass == 4 ){
		// �N���W�b�g���Z
		p_NtDat->PayClass	= (uchar)0;										// �����敪
// MH810100(S) 2020/08/18 �Ԕԃ`�P�b�g���X(#4594 �Đ��Z�ŃN���W�b�g���Z�����NT-NET���O���Z�f�[�^(ID:56)�̏����敪��5�ɂȂ�)
	}else if( p_RcptDat->PayClass == 5){
		// �N���W�b�g�Đ��Z
		p_NtDat->PayClass	= (uchar)1;										// �����敪
// MH810100(E) 2020/08/18 �Ԕԃ`�P�b�g���X(#4594 �Đ��Z�ŃN���W�b�g���Z�����NT-NET���O���Z�f�[�^(ID:56)�̏����敪��5�ɂȂ�)
	}else{
		p_NtDat->PayClass	= (uchar)p_RcptDat->PayClass;					// �����敪
	}
	p_NtDat->PayMode	= (uchar)p_RcptDat->PayMode;						// ���ZӰ��(�������Z)
// MH810100(S) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	p_NtDat->CMachineNo	= 0;												// ���Ԍ��@�B��
	p_NtDat->CMachineNo	= p_RcptDat->CMachineNo;							// ���Ԍ��@�B��
// MH810100(E) K.Onodera  2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

	if( p_NtDat->PayMethod != 5 ){
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		p_NtDat->FlapArea	= (ushort)(p_RcptDat->WPlace/10000);			// �t���b�v�V�X�e��	���		0�`99
//		p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->WPlace%10000);			// 					�Ԏ��ԍ�	0�`9999
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		p_NtDat->KakariNo	= p_RcptDat->KakariNo;							// �W����
		p_NtDat->OutKind	= p_RcptDat->OutKind;							// ���Z�o��
	}

	p_NtDat->CountSet	= p_RcptDat->CountSet;								// �ݎԃJ�E���g���Ȃ�

	if( p_NtDat->PayMethod != 5 ){
		p_NtDat->InPrev_ymdhms.In_Year	= (uchar)(p_RcptDat->TInTime.Year%100);	// ���ɔN
		p_NtDat->InPrev_ymdhms.In_Mon	= p_RcptDat->TInTime.Mon;				// ���Ɍ�
		p_NtDat->InPrev_ymdhms.In_Day	= p_RcptDat->TInTime.Day;				// ���ɓ�
		p_NtDat->InPrev_ymdhms.In_Hour	= p_RcptDat->TInTime.Hour;				// ���Ɏ�
		p_NtDat->InPrev_ymdhms.In_Min	= p_RcptDat->TInTime.Min;				// ���ɕ�
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
//		p_NtDat->InPrev_ymdhms.In_Sec	= 0;									// ���ɕb
		p_NtDat->InPrev_ymdhms.In_Sec	= p_RcptDat->TInTime_Sec;				// ���ɕb
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j

// MH810100(S) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�Đ��Z�Ή�)
// MH810100(S) 2020/08/19 �Ԕԃ`�P�b�g���X(#4595 �Đ��Z�ŃN���W�b�g���Z�����NT-NET���O���Z�f�[�^(ID:56)�̑O�񐸎Z�N���������b��0�ɂȂ�)
//		if(( p_RcptDat->PayClass == 1 ) || ( p_RcptDat->PayClass == 3 )){	// �Đ��Zor�Đ��Z���~
		if(( p_NtDat->PayClass == 1 ) || ( p_NtDat->PayClass == 3 )){	// �Đ��Zor�Đ��Z���~
// MH810100(E) 2020/08/19 �Ԕԃ`�P�b�g���X(#4595 �Đ��Z�ŃN���W�b�g���Z�����NT-NET���O���Z�f�[�^(ID:56)�̑O�񐸎Z�N���������b��0�ɂȂ�)
			p_NtDat->InPrev_ymdhms.Prev_Year = (uchar)(p_RcptDat->BeforeTPayTime.Year%100);	// �O�񐸎Z�N
			p_NtDat->InPrev_ymdhms.Prev_Mon	 = p_RcptDat->BeforeTPayTime.Mon;				// �O�񐸎Z��
			p_NtDat->InPrev_ymdhms.Prev_Day	 = p_RcptDat->BeforeTPayTime.Day;				// �O�񐸎Z��
			p_NtDat->InPrev_ymdhms.Prev_Hour = p_RcptDat->BeforeTPayTime.Hour;				// �O�񐸎Z��
			p_NtDat->InPrev_ymdhms.Prev_Min	 = p_RcptDat->BeforeTPayTime.Min;				// �O�񐸎Z��
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
//			p_NtDat->InPrev_ymdhms.Prev_Sec	 = 0;											// �O�񐸎Z�b
			p_NtDat->InPrev_ymdhms.Prev_Sec	 = p_RcptDat->BeforeTPayTime_Sec;				// �O�񐸎Z�b
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
		}else{
// MH810100(E) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�Đ��Z�Ή�)
			// �O�񐸎Z�N���������b��0�Ƃ���
			p_NtDat->InPrev_ymdhms.Prev_Year	= 0;							// �O�񐸎Z�N
			p_NtDat->InPrev_ymdhms.Prev_Mon		= 0;							// �O�񐸎Z��
			p_NtDat->InPrev_ymdhms.Prev_Day		= 0;							// �O�񐸎Z��
			p_NtDat->InPrev_ymdhms.Prev_Hour	= 0;							// �O�񐸎Z��
			p_NtDat->InPrev_ymdhms.Prev_Min		= 0;							// �O�񐸎Z��
			p_NtDat->InPrev_ymdhms.Prev_Sec		= 0;							// �O�񐸎Z�b
// MH810100(S) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�Đ��Z�Ή�)
		}
// MH810100(E) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�Đ��Z�Ή�)
	}

	p_NtDat->ReceiptIssue	= p_RcptDat->ReceiptIssue;						// �̎��ؔ��s�L��

	if( p_NtDat->PayMethod != 5 ){											// �X�V���Z�ȊO
		p_NtDat->Syubet	= p_RcptDat->syu;									// �������
		p_NtDat->Price	= p_RcptDat->WPrice;								// ���ԗ���
	}

	p_NtDat->CashPrice	= p_RcptDat->WTotalPrice;							// ��������
	p_NtDat->InPrice	= p_RcptDat->WInPrice;								// �������z
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	if( p_RcptDat->FRK_RetMod == 0 ){
//		p_NtDat->ChgPrice	= (ushort)(p_RcptDat->WChgPrice + p_RcptDat->FRK_Return);	// �ޑK���z + �U�։ߕ�����
//	}else{
//		p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// �ޑK���z
//	}
	p_NtDat->ChgPrice	= (ushort)p_RcptDat->WChgPrice;						// �ޑK���z
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	p_NtDat->MoneyInOut.MoneyKind_In	= 0x1f;								// ����L��	���ڋ���̗L���@�������i10�~,50�~,100�~,500�~,1000�~�j
	p_NtDat->MoneyInOut.In_10_cnt		= (uchar)p_RcptDat->in_coin[0];		// ����������(10�~)		0�`255
	p_NtDat->MoneyInOut.In_50_cnt		= (uchar)p_RcptDat->in_coin[1];		// ����������(50�~)		0�`255
	p_NtDat->MoneyInOut.In_100_cnt		= (uchar)p_RcptDat->in_coin[2];		// ����������(100�~)	0�`255
	p_NtDat->MoneyInOut.In_500_cnt		= (uchar)p_RcptDat->in_coin[3];		// ����������(500�~)	0�`255
	p_NtDat->MoneyInOut.In_1000_cnt		= (uchar)p_RcptDat->in_coin[4];		// ����������(1000�~)	0�`255
	if(p_RcptDat->f_escrow) {												// �G�X�N�������߂�����
		++p_NtDat->MoneyInOut.In_1000_cnt;									// �G�X�N�������߂����𓊓������ɉ��Z
	}

	p_NtDat->MoneyInOut.MoneyKind_Out	= 0x1f;								// ����L��	���ڋ���̗L���@���o���i10�~,50�~,100�~,500�~,1000�~�j
	p_NtDat->MoneyInOut.Out_10_cnt		= (uchar)p_RcptDat->out_coin[0];	// ���o������(10�~)		0�`255
	p_NtDat->MoneyInOut.Out_50_cnt		= (uchar)p_RcptDat->out_coin[1];	// ���o������(50�~)		0�`255
	p_NtDat->MoneyInOut.Out_100_cnt		= (uchar)p_RcptDat->out_coin[2];	// ���o������(100�~)	0�`255
	p_NtDat->MoneyInOut.Out_500_cnt		= (uchar)p_RcptDat->out_coin[3];	// ���o������(500�~)	0�`255
	p_NtDat->MoneyInOut.Out_1000_cnt	= (uchar)p_RcptDat->f_escrow;		// ���o������(1000�~)	0�`255

	p_NtDat->HaraiModoshiFusoku	= (ushort)p_RcptDat->WFusoku;				// ���ߕs���z	0�`9999
	if( p_RcptDat->FRK_Return && p_RcptDat->FRK_RetMod ){
		p_NtDat->CardFusokuType = p_RcptDat->FRK_RetMod;					// ���o�s���}�̎��(�����ȊO)
		p_NtDat->CardFusokuTotal = p_RcptDat->FRK_Return;					// ���o�s���z(�����ȊO)
	}
	p_NtDat->SaleParkingNo = (ulong)CPrmSS[S_SYS][1];						// ����撓�ԏꇂ(0�`999999   �������p�i�W���ł͊�{���ԏꇂ���Z�b�g�j

// GG132000(S) ���Z�����^���~����NT-NET�ʐM�̐��Z�f�[�^�Ƀf�[�^���ڂ�ǉ�
	p_NtDat->MaxChargeApplyFlg		= ntNet_56_SaveData.MaxFeeApplyFlg;		// �ő嗿���K�p�t���O
	p_NtDat->MaxChargeApplyCnt		= ntNet_56_SaveData.MaxFeeApplyCnt;		// �ő嗿���K�p��
	p_NtDat->MaxChargeSettingFlg	= ntNet_56_SaveData.MaxFeeSettingFlg;	// �ő嗿���ݒ�L��
// GG132000(E) ���Z�����^���~����NT-NET�ʐM�̐��Z�f�[�^�Ƀf�[�^���ڂ�ǉ�

	p_NtDat->PassCheck = p_RcptDat->PassCheck;								// �A���`�p�X�`�F�b�N

	if( p_RcptDat->teiki.ParkingNo ) {
		p_NtDat->ParkingNo	= p_RcptDat->teiki.ParkingNo;								// ��������ԏꇂ
// MH810100(S) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
//		p_NtDat->Media[0].MediaKind	= (ushort)(p_RcptDat->teiki.pkno_syu + 2);			// ���Z�}�̏��P�@���(���C���}��)	0�`99
//		intoasl(p_NtDat->Media[0].MediaCardNo, p_RcptDat->teiki.id, 5);					// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		intoasl(p_NtDat->Media[0].MediaCardInfo, p_RcptDat->teiki.syu, 2);				// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		// ���Z�}�̏��2�i�T�u�}�́j�� �I�[���O�Ƃ���
	}
	if( p_RcptDat->MediaKind1 ){	// ���Z�}�̏��1��ʂ���H
		/************************************/
		/*	���Z�}�̏��1 = t_MediaInfo		*/
		/************************************/
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// �Í�������
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// �Í����ԍ�
		p_NtDat->Media[0].MediaKind	= p_RcptDat->MediaKind1;								// ���Z�}�̏��1���(���C���}��) = ���Ԍ�(101)/�����(102)
		memset( &EncMediaInfo, 0, sizeof( t_MediaInfoCre2 ) );
		memcpy( EncMediaInfo.MediaCardNo, p_RcptDat->MediaCardNo1,							// �J�[�h�ԍ�(���Ԍ�:���Ԍ��@�B��(���ɋ@�B��)(���3��) + ���Ԍ��ԍ�(����6��)
													sizeof( EncMediaInfo.MediaCardNo ) );	//            �����:�����ID(5��))
		if( p_RcptDat->MediaKind1 == CARD_TYPE_PASS ){	// ���Z�}�̏��1��� = �����(102)�H
			intoas( EncMediaInfo.MediaCardInfo,p_RcptDat->teiki.syu, 2 );					// �J�[�h���(��������(2���A1�`15))
		}
// MH810100(S) m.saito 2020/05/15 �Ԕԃ`�P�b�g���X(#4166 ���Z�}�̎��101,102,203�͈Í������Ȃ�)�Ή�
//// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:380)�Ή�
////		AesCBCEncrypt( EncMediaInfo.MediaCardNo, sizeof( t_MediaInfoCre2 ) );				// AES �Í���
//		AesCBCEncrypt( (unsigned char*)&EncMediaInfo, sizeof( t_MediaInfoCre2 ) );			// AES �Í���
//// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:380)�Ή�
// MH810100(E) m.saito 2020/05/14 �Ԕԃ`�P�b�g���X(#4166 ���Z�}�̎��101,102,203�͈Í������Ȃ�)�Ή�
// MH810100(S) m.saito 2020/05/14 �Ԕԃ`�P�b�g���X(#4166 ���Z�f�[�^�̐ݒ���e���s��)�̑Ή�
//// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:381,382)�Ή�
////		memcpy( p_NtDat->Media[0].MediaCardNo, EncMediaInfo.MediaCardNo,					// �J�[�h�ԍ�(30Byte) + �J�[�h���(16Byte) + �\��(2Byte)
//		memcpy( &p_NtDat->Media[0], &EncMediaInfo,											// �J�[�h�ԍ�(30Byte) + �J�[�h���(16Byte) + �\��(2Byte)
//// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:381,382)�Ή�
		memcpy( p_NtDat->Media[0].MediaCardNo, EncMediaInfo.MediaCardNo,					// �J�[�h�ԍ�(30Byte) + �J�[�h���(16Byte) + �\��(2Byte)
// MH810100(E) m.saito 2020/05/14 �Ԕԃ`�P�b�g���X(#4166 ���Z�f�[�^�̐ݒ���e���s��)�̑Ή�
													 sizeof( t_MediaInfoCre2 ) );
	}
	if( p_RcptDat->MediaKind2 == CARD_TYPE_CAR_NUM ){	// ���Z�}�̏���� = �Ԕ�(203)����H
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;				// �Í�������
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;				// �Í����ԍ�
		p_NtDat->Media[1].MediaKind	= CARD_TYPE_CAR_NUM;						// ���Z�}�̏����(�T�u�}��) = �Ԕ�(203)
		memset( &EncMediaInfo, 0, sizeof( t_MediaInfoCre2 ) );
		memcpy( EncMediaInfo.MediaCardNo, p_RcptDat->MediaCardNo2,				// �J�[�h�ԍ�(���^�x�ǖ�(2Byte) + ���ޔԍ�(3Byte)
										sizeof( EncMediaInfo.MediaCardNo ) );	//            + �p�r����(2Byte) + ��A�w��ԍ�(4Byte))
// MH810100(S) m.saito 2020/05/15 �Ԕԃ`�P�b�g���X(#4166 ���Z�}�̎��101,102,203�͈Í������Ȃ�)�Ή�
//// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:383)�Ή�
////		AesCBCEncrypt( EncMediaInfo.MediaCardNo, sizeof( t_MediaInfoCre2 ) );	// AES �Í���
//		AesCBCEncrypt( (unsigned char*)&EncMediaInfo, sizeof( t_MediaInfoCre2 ) );	// AES �Í���
//// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:383)�Ή�
// MH810100(E) m.saito 2020/05/15 �Ԕԃ`�P�b�g���X(#4166 ���Z�}�̎��101,102,203�͈Í������Ȃ�)�Ή�
// MH810100(S) m.saito 2020/05/14 �Ԕԃ`�P�b�g���X(#4166 ���Z�f�[�^�̐ݒ���e���s��)�̑Ή�
//// MH810100(S) S.Nishimoto 2020/04/09 �ÓI���(20200407:384,385)�Ή�
////		memcpy( p_NtDat->Media[1].MediaCardNo, EncMediaInfo.MediaCardNo,		// �J�[�h�ԍ�(30Byte) + �J�[�h���(16Byte) + �\��(2Byte)
//		memcpy( &p_NtDat->Media[1], &EncMediaInfo,									// �J�[�h�ԍ�(30Byte) + �J�[�h���(16Byte) + �\��(2Byte)
//// MH810100(E) S.Nishimoto 2020/04/09 �ÓI���(20200407:384,385)�Ή�
		memcpy( p_NtDat->Media[1].MediaCardNo, EncMediaInfo.MediaCardNo,		// �J�[�h�ԍ�(30Byte) + �J�[�h���(16Byte) + �\��(2Byte)
// MH810100(E) m.saito 2020/05/14 �Ԕԃ`�P�b�g���X(#4166 ���Z�f�[�^�̐ݒ���e���s��)�̑Ή�
												 sizeof( t_MediaInfoCre2 ) );
// MH810100(E) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	}

// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�K�i���������L�����Z�b�g����j
	// �f�[�^�ۊǃT�[�r�X�Ή��ȍ~�͕K��1���Z�b�g����
	p_NtDat->Invoice = 1;
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�K�i���������L�����Z�b�g����j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
	memcpy(p_NtDat->RegistNum, p_RcptDat->RegistNum, sizeof(p_RcptDat->RegistNum));
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
	p_NtDat->TaxPrice = p_RcptDat->WTaxPrice;
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�K�p�ŗ����Z�b�g����j
	p_NtDat->TaxRate = p_RcptDat->WTaxRate;
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�K�p�ŗ����Z�b�g����j

// MH810103 GG119202(S) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
//	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 ){	// �d�q���ώ�� Suica:1, Edy:2
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 &&	// �d�q���ώ�� Suica:1, Edy:2
//		p_NtDat->PayClass != 2 && p_NtDat->PayClass != 3 ){	// ���Z���~�A�Đ��Z���~�ł͂Ȃ�
	if( p_RcptDat->Electron_data.Suica.e_pay_kind != 0 ){	// �d�q���ώ�� Suica:1, Edy:2
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(E) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
		switch (p_RcptDat->Electron_data.Suica.e_pay_kind) {
		case EC_EDY_USED:
			p_NtDat->CardKind = (ushort)3;
			break;
		case EC_NANACO_USED:
			p_NtDat->CardKind = (ushort)6;
			break;
		case EC_WAON_USED:
			p_NtDat->CardKind = (ushort)5;
			break;
		case EC_SAPICA_USED:
			p_NtDat->CardKind = (ushort)7;
			break;
		case EC_KOUTSUU_USED:
			p_NtDat->CardKind = (ushort)CARD_EC_KOUTSUU_USED_TEMP;		// �����Ƌ�ʂ��邽�߂̒l���ꎞ�I�ɃZ�b�g
			break;
		case EC_ID_USED:
			p_NtDat->CardKind = (ushort)12;
			break;
		case EC_QUIC_PAY_USED:
			p_NtDat->CardKind = (ushort)13;
			break;
		case EC_CREDIT_USED:
			p_NtDat->CardKind = (ushort)1;
			break;
		case EC_HOUJIN_USED:
			p_NtDat->CardKind = (ushort)1;
			break;
// MH810105(S) MH364301 PiTaPa�Ή�
		case EC_PITAPA_USED:
			p_NtDat->CardKind = (ushort)4;
			break;
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case EC_QR_USED:
			p_NtDat->CardKind = (ushort)16;
			break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		default:		// 2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
			p_NtDat->CardKind = (ushort)2;
			break;
		}
	}

	switch( p_NtDat->CardKind ){							// �J�[�h���ϋ敪	0=�Ȃ��A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
	case 1:
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// �Í�������
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// �Í����ԍ�
		memset( &t_Credit, 0, sizeof( t_Credit ) );
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		t_Credit.amount = (ulong)p_RcptDat->credit.pay_ryo;									// ���ϊz
//// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
////		memcpy( &t_Credit.card_no[0], &p_RcptDat->credit.card_no[0], sizeof( t_Credit.card_no ) );	// �ڼޯĶ��މ����
//// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//		if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
//			p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
//			// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.��0x20h���߂���B
//			// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
//			memset( &t_Credit.card_no[0], 0x20, sizeof(t_Credit.card_no) );
//		}
//		else {
//// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//		// �݂Ȃ����ώ��̓X�y�[�X���߂���
//		memcpyFlushLeft( &t_Credit.card_no[0], (uchar *)&p_RcptDat->credit.card_no[0],
//						sizeof(t_Credit.card_no), sizeof(p_RcptDat->credit.card_no) );		// �ڼޯĶ��މ����
//// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//		}
//// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//// MH321800(E) NT-NET���Z�f�[�^�d�l�ύX
//		// ���No�̏�7���ځ`��5���ڈȊO��'*'���i�[����Ă�����'0'�ɒu��
//		change_CharInArray( &t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '*', '0' );
//// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
////		// �݂Ȃ����ώ���NULL(0x0)�Ŗ��߂�
////		memcpyFlushLeft2( &t_Credit.cct_num[0], (uchar *)&p_RcptDat->credit.CCT_Num[0],
////						sizeof(t_Credit.cct_num), sizeof(p_RcptDat->credit.CCT_Num));		// �[�����ʔԍ�
////		memcpy( &t_Credit.kid_code[0], &p_RcptDat->credit.kid_code[0], sizeof(t_Credit.kid_code));// KID �R�[�h
//		// �݂Ȃ����ώ��̓X�y�[�X���߂���
//		memcpyFlushLeft( &t_Credit.cct_num[0], (uchar *)&p_RcptDat->credit.CCT_Num[0],
//						sizeof(t_Credit.cct_num), sizeof(p_RcptDat->credit.CCT_Num));		// �[�����ʔԍ�
//		// �݂Ȃ����ώ��̓X�y�[�X���߂���
//		memcpyFlushLeft( &t_Credit.kid_code[0], (uchar *)&p_RcptDat->credit.kid_code[0],
//						sizeof(t_Credit.kid_code), sizeof(p_RcptDat->credit.kid_code) );	// KID�R�[�h
//// MH321800(E) NT-NET���Z�f�[�^�d�l�ύX
//		t_Credit.app_no = p_RcptDat->credit.app_no;											// ���F�ԍ�
//		t_Credit.center_oiban = 0;															// ���������ǔ�(�Z���^�[��0�Œ�)
//		memcpyFlushLeft( &t_Credit.ShopAccountNo[0], (uchar*)&p_RcptDat->credit.ShopAccountNo[0],
//						sizeof(t_Credit.ShopAccountNo), sizeof(p_RcptDat->credit.ShopAccountNo) );	// �����X����ԍ�
//		t_Credit.slip_no = p_RcptDat->credit.slip_no;										// �`�[�ԍ�
		// ���ϒ[���敪
		p_NtDat->PayTerminalClass = 1;

		// ����X�e�[�^�X
		if (p_RcptDat->Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
			// �����x���s��
			p_NtDat->Transactiontatus = 3;
		}
		else if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			// �݂Ȃ�����
			p_NtDat->Transactiontatus = 1;
		}
		else {
			// �x��
			p_NtDat->Transactiontatus = 0;
		}

		// �����敪���Q�A�R�Ō��ϐ��Z���~�̏ꍇ
		if (p_RcptDat->PayClass == 2 || p_RcptDat->PayClass == 3) {
			// ���p���z
			t_Credit.amount = 0;
			// �J�[�h�ԍ�
			memset(&t_Credit.card_no[0], 0x20, sizeof(t_Credit.card_no));
			// �[�����ʔԍ�
			memset(&t_Credit.cct_num[0], 0x20, sizeof(t_Credit.cct_num));
			// KID�R�[�h
			memset(&t_Credit.kid_code[0], 0x20, sizeof(t_Credit.kid_code));
			// ���F�ԍ�
			t_Credit.app_no = 0;
			// �`�[�ԍ�
			t_Credit.slip_no = 0;
		}
		else {
			// ���p���z
			t_Credit.amount = p_RcptDat->credit.pay_ryo;
			// �J�[�h�ԍ�
			if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
				p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
				// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.��0x20h���߂���B
				// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
				memset( &t_Credit.card_no[0], 0x20, sizeof(t_Credit.card_no) );
			}
			else {
				memcpyFlushLeft(&t_Credit.card_no[0],
								(uchar*)&p_RcptDat->credit.card_no[0],
								sizeof(t_Credit.card_no),
								sizeof(p_RcptDat->credit.card_no));
			}
			change_CharInArray( &t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '*', '0' );
			// �[�����ʔԍ�
			memcpyFlushLeft(&t_Credit.cct_num[0],
							(uchar*)&p_RcptDat->credit.CCT_Num[0],
							sizeof(t_Credit.cct_num),
							sizeof(p_RcptDat->credit.CCT_Num));
			// KID�R�[�h
			memcpyFlushLeft(&t_Credit.kid_code[0],
							(uchar*)&p_RcptDat->credit.kid_code[0],
							sizeof(t_Credit.kid_code),
							sizeof(p_RcptDat->credit.kid_code));
			// ���F�ԍ�
			t_Credit.app_no = p_RcptDat->credit.app_no;
			// �`�[�ԍ�
			t_Credit.slip_no = p_RcptDat->credit.slip_no;
		}

		// �Z���^�����ǔ�
		t_Credit.center_oiban = 0;
		// �����X����ԍ�
		memset(t_Credit.ShopAccountNo, 0x20, sizeof(t_Credit.ShopAccountNo));
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		
		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );							// AES �Í���
		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// ���Ϗ��
		break;
	case 2:				// ��ʌn�d�q�}�l�[(SX-20)
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// �Í�������
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// �Í����ԍ�
		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
		t_EMoney.amount = p_RcptDat->Electron_data.Suica.pay_ryo;							// �d�q���ϐ��Z���@���ϊz
		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
		memcpy( &t_EMoney.card_id[0], &p_RcptDat->Electron_data.Suica.Card_ID[0],
				sizeof(p_RcptDat->Electron_data.Suica.Card_ID) );							// �d�q���ϐ��Z���@����ID (Ascii 16��)
		t_EMoney.card_zangaku = p_RcptDat->Electron_data.Suica.pay_after;					// �d�q���ϐ��Z���@���ό�c��
		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );							// AES �Í���
		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// ���Ϗ��
		break;
	case 3:				// Edy
// MH810105(S) MH364301 PiTaPa�Ή�
	case 4:				// PiTaPa
// MH810105(E) MH364301 PiTaPa�Ή�
	case 5:				// WAON
	case 6:				// nanaco
	case 7:				// SAPICA
	case CARD_EC_KOUTSUU_USED_TEMP:		// ��ʌn (�����Ƌ�ʂ��邽�߈ꎞ�I�ɃZ�b�g�����l)
	case 12:			// iD
	case 13:			// QUICPay
		if (p_NtDat->CardKind == CARD_EC_KOUTSUU_USED_TEMP) {
		// ��ʌnIC�J�[�h�̃J�[�h���ϋ敪�ɐ��K�̒l���Z�b�g
			p_NtDat->CardKind = 2;
		}
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// �Í�������
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// �Í����ԍ�
		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		t_EMoney.amount = p_RcptDat->Electron_data.Ec.pay_ryo;								// �d�q���ϐ��Z���@���ϊz
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//		if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
//			p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
//			// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.���J�[�h������0x30���߂����l�߂�ZZ����B
//			// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
//			// ���ꂼ��̃J�[�h������0x30����
//			memset( &t_EMoney.card_id[0], 0x30, (size_t)(EcBrandEmoney_Digit[p_RcptDat->Electron_data.Suica.e_pay_kind - EC_USED]) );
//			// ���l�߂�ZZ
//			memset( &t_EMoney.card_id[0], 'Z', 2 );
//		}
//		else {
//// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//		memcpy( &t_EMoney.card_id[0], &p_RcptDat->Electron_data.Ec.Card_ID[0],
//				sizeof(p_RcptDat->Electron_data.Ec.Card_ID) );								// �d�q���ϐ��Z���@����ID (Ascii 20��)
//// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//		}
//// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//		t_EMoney.card_zangaku = p_RcptDat->Electron_data.Ec.pay_after;						// �d�q���ϐ��Z���@���ό�c��
//// MH810103 GG119202(S) ���Z�f�[�^�̌��Ϗ��ɖ₢���킹�ԍ��̓Z�b�g���Ȃ�
//		// ���Ϗ��ɖ⍇���ԍ����Z�b�g����d�l�̓t�H�[�}�b�gRev18�őΉ�
////// GG119200(S) �d�q�}�l�[�Ή�
//////		// �݂Ȃ����ώ���NULL(0x0)�Ŗ��߂�
//////		memcpyFlushLeft2( &t_EMoney.inquiry_num[0], &ntNet_152_SaveData.e_inquiry_num[0],
//////				sizeof(ntNet_152_SaveData.e_inquiry_num), sizeof(t_EMoney.inquiry_num) );	// �⍇���ԍ�
////		// �݂Ȃ����ώ���NULL(0x0)�Ŗ��߂�
////		memcpyFlushLeft( &t_EMoney.inquiry_num[0], &p_RcptDat->Electron_data.Ec.inquiry_num[0],
////						sizeof(t_EMoney.inquiry_num), sizeof(p_RcptDat->Electron_data.Ec.inquiry_num) );	// �⍇���ԍ�
////// GG119200(E) �d�q�}�l�[�Ή�
//// MH810103 GG119202(E) ���Z�f�[�^�̌��Ϗ��ɖ₢���킹�ԍ��̓Z�b�g���Ȃ�
		// ���ϒ[���敪
		p_NtDat->PayTerminalClass = 1;

		// ����X�e�[�^�X
		if (p_RcptDat->Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
			// �����x���s��
			p_NtDat->Transactiontatus = 3;
		}
		else if (p_RcptDat->EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			// �����x���ς�
			p_NtDat->Transactiontatus = 2;
		}
		else if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			// �݂Ȃ�����
			p_NtDat->Transactiontatus = 1;
		}
		else {
			// �x��
			p_NtDat->Transactiontatus = 0;
		}

		// �����敪���Q�A�R�Ō��ϐ��Z���~�̏ꍇ
		if (p_RcptDat->PayClass == 2 || p_RcptDat->PayClass == 3) {
			// ���p���z
			t_EMoney.amount = 0;
			// �J�[�h�ԍ�
			memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
			memset(&t_EMoney.card_id[0], 0x30, (size_t)(EcBrandEmoney_Digit[p_RcptDat->Electron_data.Ec.e_pay_kind - EC_USED]));
			memset(&t_EMoney.card_id[0], 'Z', 2);
			// �J�[�h�c�z
			t_EMoney.card_zangaku = 0;
			// �₢���킹�ԍ�
			memset(&t_EMoney.inquiry_num[0], 0x20, sizeof(t_EMoney.inquiry_num));
			// ���F�ԍ�
			t_EMoney.approbal_no = 0;
		}
		else {
			// ���p���z
			t_EMoney.amount = p_RcptDat->Electron_data.Ec.pay_ryo;

			// �J�[�h�ԍ�
			if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
				p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){
				// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.���J�[�h������0x30���߂����l�߂�ZZ����B
				// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
				memset(	&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id) );
				memset( &t_EMoney.card_id[0], 0x30, (size_t)(EcBrandEmoney_Digit[p_RcptDat->Electron_data.Suica.e_pay_kind - EC_USED]) );
				memset( &t_EMoney.card_id[0], 'Z', 2 );
			}
			else{
				memcpyFlushLeft(&t_EMoney.card_id[0],
								(uchar*)&p_RcptDat->Electron_data.Ec.Card_ID[0],
								sizeof(t_EMoney.card_id),
								sizeof(p_RcptDat->Electron_data.Ec.Card_ID));
			}
			// �J�[�h�c�z
			t_EMoney.card_zangaku = p_RcptDat->Electron_data.Ec.pay_after;
			// �₢���킹�ԍ�
			memcpyFlushLeft(&t_EMoney.inquiry_num[0],
							(uchar*)&p_RcptDat->Electron_data.Ec.inquiry_num[0],
							sizeof(t_EMoney.inquiry_num),
							sizeof(p_RcptDat->Electron_data.Ec.inquiry_num));

			// ���F�ԍ�
			switch (p_RcptDat->Electron_data.Ec.e_pay_kind) {
			case EC_ID_USED:
				t_EMoney.approbal_no =
							astoinl(p_RcptDat->Electron_data.Ec.Brand.Id.Approval_No, 7);
				break;
			case EC_QUIC_PAY_USED:
				t_EMoney.approbal_no =
							astoinl(p_RcptDat->Electron_data.Ec.Brand.Quickpay.Approval_No, 7);
				break;
			case EC_PITAPA_USED:
				// �I�t���C�����F���ώ��̓I�[��'*'�̂��߁A'0'�֕ϊ�����
				if (p_RcptDat->Electron_data.Ec.Brand.Pitapa.Approval_No[0] == '*') {
					t_EMoney.approbal_no = 0;
				}
				else {
					t_EMoney.approbal_no =
								astoinl(p_RcptDat->Electron_data.Ec.Brand.Pitapa.Approval_No, 8);
				}
				break;
			default:
				t_EMoney.approbal_no = 0;
				break;
			}
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );							// AES �Í���
		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// ���Ϗ��
		break;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	case 16:
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;							// �Í�������
		p_NtDat->DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;							// �Í����ԍ�
		memset( &t_QR, 0, sizeof( t_QR ) );
		// ���ϒ[���敪
		p_NtDat->PayTerminalClass = 1;

		// ����X�e�[�^�X
		if (p_RcptDat->Electron_data.Ec.E_Status.BIT.miryo_confirm == 1) {
			// �����x���s��
			p_NtDat->Transactiontatus = 3;
		}
		else if (p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			// �݂Ȃ�����
			p_NtDat->Transactiontatus = 1;
		}
		else {
			// �x��
			p_NtDat->Transactiontatus = 0;
		}

		// �����敪���Q�A�R�Ō��ϐ��Z���~�̏ꍇ
		if (p_RcptDat->PayClass == 2 || p_RcptDat->PayClass == 3) {
			// ���p���z
			t_QR.amount = 0;
			// Mch����ԍ�
			memset(&t_QR.MchTradeNo[0], 0x20, sizeof(t_QR.MchTradeNo));

			if (p_RcptDat->Electron_data.Ec.E_Flag.BIT.deemSettleCancal == 1) {
				// ���~�ł��邪���ό��ʂ���M�ł��Ȃ������̂�
				// ���ꂼ��̌�����0x20���߂��A�x���[��ID��13���A����ԍ���15���A0x30���߂���
				// �x���[��ID
				memset(&t_QR.PayTerminalNo, 0x20, sizeof(t_QR.PayTerminalNo));
				memset(&t_QR.PayTerminalNo, 0x30, 13);
				// ����ԍ�
				memset(&t_QR.DealNo, 0x20, sizeof(t_QR.DealNo));
				memset(&t_QR.DealNo, 0x30, 15);
			}
			else {
				// �x���[��ID
				memcpyFlushLeft(&t_QR.PayTerminalNo[0],
								&p_RcptDat->Electron_data.Ec.Brand.Qr.PayTerminalNo[0],
								sizeof(t_QR.PayTerminalNo),
								sizeof(p_RcptDat->Electron_data.Ec.Brand.Qr.PayTerminalNo));
				// ����ԍ�
				memcpyFlushLeft(&t_QR.DealNo[0],
								&p_RcptDat->Electron_data.Ec.inquiry_num[0],
								sizeof(t_QR.DealNo),
								sizeof(p_RcptDat->Electron_data.Ec.inquiry_num));
			}
		}
		else {
			// ���p���z
			t_QR.amount = p_RcptDat->Electron_data.Ec.pay_ryo;

			if( p_RcptDat->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
				// Mch����ԍ�
				memset(&t_QR.MchTradeNo[0], 0x20, sizeof(t_QR.MchTradeNo));
				// �x���[��ID
				memcpyFlushLeft(&t_QR.PayTerminalNo[0],
								&p_RcptDat->Electron_data.Ec.Brand.Qr.PayTerminalNo[0],
								sizeof(t_QR.PayTerminalNo),
								sizeof(p_RcptDat->Electron_data.Ec.Brand.Qr.PayTerminalNo));
				// ����ԍ�
				memcpyFlushLeft(&t_QR.DealNo[0],
								&p_RcptDat->Electron_data.Ec.inquiry_num[0],
								sizeof(t_QR.DealNo),
								sizeof(p_RcptDat->Electron_data.Ec.inquiry_num));
			}
			else{
				// Mch����ԍ�
				memcpyFlushLeft(&t_QR.MchTradeNo[0],
								&p_RcptDat->Electron_data.Ec.Brand.Qr.MchTradeNo[0],
								sizeof(t_QR.MchTradeNo),
								sizeof(p_RcptDat->Electron_data.Ec.Brand.Qr.MchTradeNo));
				// �x���[��ID
				memset(&t_QR.PayTerminalNo, 0x20, sizeof(t_QR.PayTerminalNo));
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6381 QR�R�[�h���ςŒʏ퐸�Z���̐��Z�f�[�^�Ō��Ϗ��̎x���[��ID�A����ԍ��� 0�ƂȂ��Ă��܂�
//				memset(&t_QR.PayTerminalNo, 0x30, 13);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6381 QR�R�[�h���ςŒʏ퐸�Z���̐��Z�f�[�^�Ō��Ϗ��̎x���[��ID�A����ԍ��� 0�ƂȂ��Ă��܂�
				// ����ԍ�
				memset(&t_QR.DealNo, 0x20, sizeof(t_QR.DealNo));
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6381 QR�R�[�h���ςŒʏ퐸�Z���̐��Z�f�[�^�Ō��Ϗ��̎x���[��ID�A����ԍ��� 0�ƂȂ��Ă��܂�
//				memset(&t_QR.DealNo, 0x30, 15);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6381 QR�R�[�h���ςŒʏ퐸�Z���̐��Z�f�[�^�Ō��Ϗ��̎x���[��ID�A����ԍ��� 0�ƂȂ��Ă��܂�
			}
		}
		// ���σu�����h
		t_QR.PayKind = p_RcptDat->Electron_data.Ec.Brand.Qr.PayKind;

		AesCBCEncrypt( (uchar *)&t_QR, sizeof( t_QR ) );									// AES �Í���
		memcpy( (uchar *)&p_NtDat->settlement[0], (uchar *)&t_QR, sizeof( t_QR ) );			// ���Ϗ��
		break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	default:
		break;
	}

	p_NtDat->TotalSale		= (ulong)0;										// ���v���z(����)
	p_NtDat->DeleteSeq		= (ulong)0;										// �폜�f�[�^�ǔ�

// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6590 ���u���Z���ANT-NET�̐��Z�f�[�^�̍��ځu�Ԕԏ��L���v���Ԕԏ�񂠂�ɂȂ� [���ʉ��P���� No1532]
// // GG124100(S) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
// 	p_NtDat->CarNumInfoFlg	= (uchar)1;										// �Ԕԏ��L��
// 
// 	// �Ԕԏ��
// 	memcpy(&EncShabanInfo, &p_RcptDat->ShabanInfo, sizeof(SHABAN_INFO));
// 	AesCBCEncrypt((unsigned char *)&EncShabanInfo, sizeof(EncShabanInfo));	// AES �Í���
// 	memcpy(&p_NtDat->ShabanInfo, &EncShabanInfo, sizeof(SHABAN_INFO));
// // GG124100(E) R.Endo 2021/12/23 �Ԕԃ`�P�b�g���X3.0 #6123 NT-NET���Z�f�[�^�̎Ԕԏ��Z�b�g
	// �Ԕԏ��L��
	if ( p_RcptDat->ShabanInfo.LandTransOfficeName[0] ||	// ���^�x�ǖ�����
		 p_RcptDat->ShabanInfo.ClassNum[0] ||				// ���ޔԍ�����
		 p_RcptDat->ShabanInfo.UsageCharacter[0] ||			// �p�r��������
		 p_RcptDat->ShabanInfo.SeqDesignNumber[0] ) {		// ��A�w��ԍ�����
		p_NtDat->CarNumInfoFlg = (uchar)1;
	}

	// �Ԕԏ��
	if ( p_NtDat->CarNumInfoFlg ) {
		memcpy(&EncShabanInfo, &p_RcptDat->ShabanInfo, sizeof(SHABAN_INFO));
		AesCBCEncrypt((unsigned char *)&EncShabanInfo, sizeof(EncShabanInfo));	// AES �Í���
		memcpy(&p_NtDat->ShabanInfo, &EncShabanInfo, sizeof(SHABAN_INFO));
	}
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6590 ���u���Z���ANT-NET�̐��Z�f�[�^�̍��ځu�Ԕԏ��L���v���Ԕԏ�񂠂�ɂȂ� [���ʉ��P���� No1532]

	for( i = j = 0; i < WTIK_USEMAX; i++ ){									// ���Z���~�ȊO�̊������R�s�[
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 		if(( p_RcptDat->DiscountData[i].DiscSyu != 0 ) &&					// ������ʂ���
// 		   (( p_RcptDat->DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// ���Z���~�������łȂ�
// 		    ( NTNET_CFRE < p_RcptDat->DiscountData[i].DiscSyu ))){
// 		    if( p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA�ȊO(�ԍ�)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO�ȊO(�ԍ�)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY�ȊO	(�ԍ�)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_WAON_1 &&		// WAON�ȊO	(�ԍ�)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_WAON_2 &&		//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SAPICA_1 &&		// SAPICA�ȊO	(�ԍ�)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_SAPICA_2 &&		//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_NANACO_1 &&		// NANACO�ȊO	(�ԍ�)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_NANACO_2 &&		//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ID_1 &&			// iD�ȊO	(�ԍ�)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ID_2 &&			//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_QUICPAY_1 &&	// QUICPay�ȊO	(�ԍ�)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_QUICPAY_2 &&	//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA�ȊO(�ԍ�)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&		// IC-Card�ȊO(�ԍ�)
// // MH810100(S) 2020/06/12 #4199 �y�A���]���w�E�����zNT-NET���Z�f�[�^�ł̊����̍����i�[
// //		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_2 )		//			(�x���z�E�c�z)
// 		    	p_RcptDat->DiscountData[i].DiscSyu != NTNET_ICCARD_2 &&		//			(�x���z�E�c�z)
// 		    	!((( p_RcptDat->DiscountData[i].DiscSyu == NTNET_SHOP_DISC_TIME ) ||	// �����ςݔ��������ȊO
// 		    	   ( p_RcptDat->DiscountData[i].DiscSyu == NTNET_SHOP_DISC_AMT )) &&
// 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) &&
// // MH810100(E) 2020/06/12 #4199 �y�A���]���w�E�����zNT-NET���Z�f�[�^�ł̊����̍����i�[
// // MH810100(S) K.Onodera  2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// 		    	!((( p_RcptDat->DiscountData[i].DiscSyu == NTNET_SVS_T ) ||	// �����ς݃T�[�r�X���ȊO
// 		    	   ( p_RcptDat->DiscountData[i].DiscSyu == NTNET_SVS_M )) &&
// 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) &&
// 		    	!((( p_RcptDat->DiscountData[i].DiscSyu == NTNET_KAK_T ) ||	// �����ςݓX�����ȊO
// 		    	   ( p_RcptDat->DiscountData[i].DiscSyu == NTNET_KAK_M )) &&
// // MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(NT-NET���Z�f�[�^�̕ύX)
// // 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) )
// 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) &&
// 		    	!((( p_RcptDat->DiscountData[i].DiscSyu == NTNET_TKAK_T ) ||	// �����ςݑ��X�܊����ȊO
// 		    	   ( p_RcptDat->DiscountData[i].DiscSyu == NTNET_TKAK_M )) &&
// 		    	  ( p_RcptDat->DiscountData[i].uDiscData.common.DiscFlg == 1 )) )
// // MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(NT-NET���Z�f�[�^�̕ύX)
// // MH810100(E) K.Onodera  2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// GG124100(S) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6522 �����z��0�~�̊�����NT-NET���Z�f�[�^�̊������ɃZ�b�g����Ȃ�
// 		// �u������ʁv�͊����v�Z���Ƀ`�F�b�N�ς݂̂��߁u����g�p�����������z�v�̗L���̂݃`�F�b�N����
// 		if ( p_RcptDat->DiscountData[i].Discount > 0 ) {
		// �u������ʁv�͊����v�Z���Ƀ`�F�b�N�ς݂̂��߂����ł̓`�F�b�N���Ȃ�
		if ( p_RcptDat->DiscountData[i].Discount ||		// ����g�p�����������z
			 p_RcptDat->DiscountData[i].DiscCount ) {	// ����g�p��������
// GG124100(E) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6522 �����z��0�~�̊�����NT-NET���Z�f�[�^�̊������ɃZ�b�g����Ȃ�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
		    {
				p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DiscountData[i].ParkingNo;						// ���ԏ�No.
				p_NtDat->SDiscount[j].Kind		= p_RcptDat->DiscountData[i].DiscSyu;						// �������
				p_NtDat->SDiscount[j].Group		= p_RcptDat->DiscountData[i].DiscNo;						// �����敪
				p_NtDat->SDiscount[j].Callback	= p_RcptDat->DiscountData[i].DiscCount;						// �������
				p_NtDat->SDiscount[j].Amount	= p_RcptDat->DiscountData[i].Discount;						// �����z
				p_NtDat->SDiscount[j].Info1		= p_RcptDat->DiscountData[i].DiscInfo1;						// �������1
				if( p_NtDat->SDiscount[j].Kind != NTNET_PRI_W ){
					p_NtDat->SDiscount[j].Info2	= p_RcptDat->DiscountData[i].uDiscData.common.DiscInfo2;	// �������2
				}
				j++;
		    }
		}
	}
	for( i=0; i<DETAIL_SYU_MAX; i++ ){
		if( p_RcptDat->DetailData[i].DiscSyu ){
			p_NtDat->SDiscount[j].ParkingNo	= p_RcptDat->DetailData[i].ParkingNo;					// ���ԏ�No.
			p_NtDat->SDiscount[j].Kind		= p_RcptDat->DetailData[i].DiscSyu;						// �������
			p_NtDat->SDiscount[j].Group		= p_RcptDat->DetailData[i].uDetail.Common.DiscNo;		// �����敪
			p_NtDat->SDiscount[j].Callback	= p_RcptDat->DetailData[i].uDetail.Common.DiscCount;	// �������
			p_NtDat->SDiscount[j].Amount	= p_RcptDat->DetailData[i].uDetail.Common.Discount;		// �����z
			p_NtDat->SDiscount[j].Info1		= p_RcptDat->DetailData[i].uDetail.Common.DiscInfo1;	// �������1
			p_NtDat->SDiscount[j].Info2		= p_RcptDat->DetailData[i].uDetail.Common.DiscInfo2;	// �������2
			j++;
		}
	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH321800(S) �������8000�̃Z�b�g���@���C��
//	// ���σ��[�_�ڑ�����
//	if (isEC_USE()) {
//// MH810103 GG119202(S) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
//		// ���Z���~�A�Đ��Z���~���͊������ɃZ�b�g���Ȃ�
//		if (p_NtDat->PayClass != 2 && p_NtDat->PayClass != 3) {
//// MH810103 GG119202(E) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
//		if (p_RcptDat->credit.pay_ryo != 0) {
//			// �N���W�b�g����
//			/* �󂫴ر���� */
//			for (i = 0; (i < NTNET_DIC_MAX) &&
//					(0 != p_NtDat->SDiscount[i].ParkingNo); i++) {
//				;
//			}		/* �f�[�^�������				*/
//			if (i < NTNET_DIC_MAX) {
//				p_NtDat->SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];			// ���ԏ�No.
//				p_NtDat->SDiscount[i].Kind = NTNET_INQUIRY_NUM;				// �������
//				// �N���W�b�g���ώ��͖₢���킹�ԍ�����M���Ȃ��̂�0x20���߂Ƃ���
//				memset(&p_NtDat->SDiscount[i].Group, 0x20, 16);
//			}
//		}
//		else if (p_RcptDat->Electron_data.Ec.pay_ryo != 0) {
//			// �d�q�}�l�[����
//			// �������ɃJ�[�hNo.�Ɨ��p�z�̓Z�b�g���Ȃ�
//			/* �󂫴ر���� */
//			for (i = 0; (i < NTNET_DIC_MAX) &&
//					(0 != p_NtDat->SDiscount[i].ParkingNo); i++) {
//				;
//			}		/* �f�[�^�������				*/
//			if (i < NTNET_DIC_MAX) {
//				p_NtDat->SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];			// ���ԏ�No.
//				p_NtDat->SDiscount[i].Kind = NTNET_INQUIRY_NUM;				// �������
//// MH810103 GG119202(S) �d�q�}�l�[�Ή�
////				memcpy(&p_NtDat->SDiscount[i].Group, p_RcptDat->Electron_data.Ec.inquiry_num, 16);
//				memcpyFlushLeft( (uchar *)&p_NtDat->SDiscount[i].Group, (uchar *)&p_RcptDat->Electron_data.Ec.inquiry_num[0],
//									16, sizeof( p_RcptDat->Electron_data.Ec.inquiry_num ) );	// �₢���킹�ԍ�
//// MH810103 GG119202(E) �d�q�}�l�[�Ή�
//			}
//		}
//// MH810103 GG119202(S) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
//		}
//// MH810103 GG119202(E) ���ϐ��Z���~���͐��Z�f�[�^�ɂ݂Ȃ����Ϗ����Z�b�g���Ȃ�
//	}
//// MH321800(E) �������8000�̃Z�b�g���@���C��
	if ((p_RcptDat->PayClass == 2 || p_RcptDat->PayClass == 3) &&
		p_NtDat->Transactiontatus == 3) {
		// ���ϐ��Z���~
		/* �󂫴ر���� */
		for (i = 0; (i < NTNET_DIC_MAX) &&
				(0 != p_NtDat->SDiscount[i].ParkingNo); i++) {
			;
		}		/* �f�[�^�������				*/
		if (i < NTNET_DIC_MAX) {
			p_NtDat->SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];	// ���ԏ�No.
			p_NtDat->SDiscount[i].Kind = NTNET_MIRYO_UNKNOWN;	// �������
			p_NtDat->SDiscount[i].Group = p_NtDat->CardKind;	// �����敪
			p_NtDat->SDiscount[i].Callback = 1;					// �g�p����
			p_NtDat->SDiscount[i].Amount = 0;					// �����z

			if (p_RcptDat->credit.pay_ryo != 0) {
				p_NtDat->SDiscount[i].Info1 = p_RcptDat->credit.pay_ryo;			// �������1
			}
			else if (p_RcptDat->Electron_data.Ec.pay_ryo != 0) {
				p_NtDat->SDiscount[i].Info1 = p_RcptDat->Electron_data.Ec.pay_ryo;	// �������1
			}

			p_NtDat->SDiscount[i].Info2 = 0;					// �������2
		}
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810102(S) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
	NTNET_Edit_Data56_ParkingTkt_Pass(p_RcptDat, p_NtDat);
// MH810102(E) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	ret = sizeof( DATA_KIND_56_r17 );
	ret = sizeof( DATA_KIND_56_rXX );
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	return ret;
}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810102(S) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
//[]----------------------------------------------------------------------[]
///	@brief		���Z�f�[�^(���Ԍ�/�����)
//[]----------------------------------------------------------------------[]
///	@param[in]	p_RcptDat	�F���Z���O�f�[�^
///	@param[out]	p_NtDat		�F���ZNT-NET�f�[�^
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2021/03/17<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//static void NTNET_Edit_Data56_ParkingTkt_Pass( Receipt_data *p_RcptDat, DATA_KIND_56_r17 *p_NtDat )
static void NTNET_Edit_Data56_ParkingTkt_Pass( Receipt_data *p_RcptDat, DATA_KIND_56_rXX *p_NtDat )
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
{
	t_MediaInfoCre2 EncMediaInfo;

	// ���Ԍ��ݒ�
	if ( (ntNet_56_SaveData.NormalFlag != 0) &&	// ������p�ȊO
		 (p_RcptDat->PayMethod == 2) ) {		// ��������Z
		// ���Z���@
		p_NtDat->PayMethod = 0;											// ���Ȃ����Z

		// �ݎԃJ�E���g
		// 34-0027(���Z�f�[�^ID�̎�舵��)��07-00X4(���X �J�E���g�M���^���ݑ䐔�J�E���g)��������
		// ���肵�����ʂ�ݒ肷�ׂ������AGT-4100�ł͍ݎԊǗ����Ȃ����߁u1�v�Œ�Ƃ���B
		// ����̏ڍׂ�opesub��SetAddPayData()���Q�ƁB
		p_NtDat->CountSet = 1;											// ���Ȃ�

		// �A���`�p�X�`�F�b�N
		p_NtDat->PassCheck = 1;											// �`�F�b�NOFF

		// ��������ԏꇂ
		p_NtDat->ParkingNo = 0;

		// ���Z�}�̂P���(���C���}��)
		p_NtDat->Media[0].MediaKind = CARD_TYPE_PARKING_TKT;			// ���Ԍ�

		// ����/���Z �}�̏��
		memset(&EncMediaInfo, 0, sizeof(t_MediaInfoCre2));
		memcpy(EncMediaInfo.MediaCardNo,								// ���Ԍ��@�B��(���ɋ@�B��)(���3���A0�`255)
			ntNet_56_SaveData.MachineTktNo,								// ���Ԍ��ԍ�(����6���A0�`999999)
			sizeof(ntNet_56_SaveData.MachineTktNo));
		memcpy(p_NtDat->Media[0].MediaCardNo, &EncMediaInfo, sizeof(t_MediaInfoCre2));
	}

	// ������ݒ�
	if ( (ntNet_56_SaveData.NormalFlag == 0) &&	// ������p
		 (p_RcptDat->PayMethod != 2) &&			// ��������Z�ȊO
		 (ntNet_56_SaveData.ParkingNo) ) {		// ������J�[�h��񂠂�
		// ���Z���@
		p_NtDat->PayMethod = 2;											// ��������Z

		// �ݎԃJ�E���g(���Ԍ��ݒ�Ɠ��l)
		p_NtDat->CountSet = 1;											// ���Ȃ�

		// �A���`�p�X�`�F�b�N
		// 07-00X2(���X ���o�`�F�b�N)�������ɔ��肵�����ʂ�ݒ肷�ׂ������A
		// GT-4100�ł̓A���`�p�X�`�F�b�N���Ȃ����߁u1�v�Œ�Ƃ���B
		// ����̏ڍׂ�opesub��SetAddPayData()���Q�ƁB
		p_NtDat->PassCheck = 1;											// �`�F�b�NOFF

		// ����� ���ԏꇂ
		p_NtDat->ParkingNo = ntNet_56_SaveData.ParkingNo;				// ���ԏꇂ

		// ���Z�}�̂P���(���C���}��)
		p_NtDat->Media[0].MediaKind = CARD_TYPE_PASS;					// �����

		// ����/���Z �}�̏��
		memset(&EncMediaInfo, 0, sizeof(t_MediaInfoCre2));
		intoasl(EncMediaInfo.MediaCardNo, ntNet_56_SaveData.id, 5);		// �����ID(5���A0�`12000)
		intoas(EncMediaInfo.MediaCardInfo, ntNet_56_SaveData.syu, 2);	// ��������(2���A1�`15)
		memcpy(p_NtDat->Media[0].MediaCardNo, &EncMediaInfo, sizeof(t_MediaInfoCre2));
	}
}
// MH810102(E) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX

// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
////[]----------------------------------------------------------------------[]
/////	@brief		���Ƀf�[�^(Rev.13�F�t�F�[�Y7)
////[]----------------------------------------------------------------------[]
/////	@param[in]	p_RcptDat	�F���Ƀ��O�f�[�^
/////	@param[out]	p_NtDat		�F����NT-NET�f�[�^
/////	@return		none
////[]----------------------------------------------------------------------[]
/////	@date		Create	: 2016/11/02<br>
/////				Update	: 
////[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
//unsigned short	NTNET_Edit_Data54_r13( enter_log *p_RcptDat, DATA_KIND_54_r13 *p_NtDat )
//{
//	unsigned short ret;
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
////	date_time_rec	wk_CLK_REC;
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//
//	memset( p_NtDat, 0, sizeof( DATA_KIND_54_r13 ) );
//	NTNET_Edit_BasicData( 54, 0,  p_RcptDat->CMN_DT.DT_54.SeqNo, &p_NtDat->DataBasic );	// ��{�f�[�^�쐬
//
//	// �����N���������b�� BasicDataMake()�ŃZ�b�g���������ł͂Ȃ����Ɏ������Z�b�g����̂ōăZ�b�g
//	p_NtDat->DataBasic.Year	= (uchar)(p_RcptDat->InTime.Year % 100);		// ���ɔN
//	p_NtDat->DataBasic.Mon	= (uchar)p_RcptDat->InTime.Mon;					// ���Ɍ�
//	p_NtDat->DataBasic.Day	= (uchar)p_RcptDat->InTime.Day;					// ���ɓ�
//	p_NtDat->DataBasic.Hour	= (uchar)p_RcptDat->InTime.Hour;				// ���Ɏ�
//	p_NtDat->DataBasic.Min	= (uchar)p_RcptDat->InTime.Min;					// ���ɕ�
//	p_NtDat->DataBasic.Sec	= (uchar)p_RcptDat->InTime.Sec;					// ���ɕb
//
//	p_NtDat->FmtRev = 13;													// �t�H�[�}�b�gRev.��
//
//	p_NtDat->ParkData.FullNo1	= p_RcptDat->CMN_DT.DT_54.FullNo1;			// ���ԑ䐔�P
//	p_NtDat->ParkData.CarCnt1	= p_RcptDat->CMN_DT.DT_54.CarCnt1;			// ���ݑ䐔�P
//	p_NtDat->ParkData.FullNo2	= p_RcptDat->CMN_DT.DT_54.FullNo2;			// ���ԑ䐔�Q
//	p_NtDat->ParkData.CarCnt2	= p_RcptDat->CMN_DT.DT_54.CarCnt2;			// ���ݑ䐔�Q
//	p_NtDat->ParkData.FullNo3	= p_RcptDat->CMN_DT.DT_54.FullNo3;			// ���ԑ䐔�R
//	p_NtDat->ParkData.CarCnt3	= p_RcptDat->CMN_DT.DT_54.CarCnt3;			// ���ݑ䐔�R
//	p_NtDat->ParkData.CarFullFlag	= 0;									// �䐔�E�����ԃt���O
//	p_NtDat->ParkData.PascarCnt	= (uchar)p_RcptDat->CMN_DT.DT_54.PascarCnt;	// ����ԗ��J�E���g
//	p_NtDat->ParkData.Full[0]	= p_RcptDat->CMN_DT.DT_54.Full[0];			// ����1������
//	p_NtDat->ParkData.Full[1]	= p_RcptDat->CMN_DT.DT_54.Full[1];			// ����2������
//	p_NtDat->ParkData.Full[2]	= p_RcptDat->CMN_DT.DT_54.Full[2];			// ����3������
//	// �䐔�Ǘ��ǔ� ------------------------
//// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
////	memcpy( &wk_CLK_REC, &CLK_REC, sizeof( date_time_rec ) );
////	// ���ꎞ���H
////	if( VehicleCountDateTime == Nrm_YMDHM( &wk_CLK_REC ) ){
////		// ���ꎞ���̏ꍇ�͒ǂ��Ԃ��{�P
////		VehicleCountSeqNo++;
////	}else{
////		// �قȂ鎞���̏ꍇ�͒ǂ��Ԃ��O�Ƃ���
////		VehicleCountDateTime = Nrm_YMDHM( &wk_CLK_REC );
////		VehicleCountSeqNo = 0;
////	}
////	p_NtDat->ParkData.CarCntInfo.CarCntYear		= (wk_CLK_REC.Year % 100);	// �N
////	p_NtDat->ParkData.CarCntInfo.CarCntMon		= wk_CLK_REC.Mon;			// ��
////	p_NtDat->ParkData.CarCntInfo.CarCntDay		= wk_CLK_REC.Day;			// ��
////	p_NtDat->ParkData.CarCntInfo.CarCntHour		= wk_CLK_REC.Hour;			// ��
////	p_NtDat->ParkData.CarCntInfo.CarCntMin		= wk_CLK_REC.Min;			// ��
////	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= VehicleCountSeqNo;		// �ǔ�
////	p_NtDat->ParkData.CarCntInfo.Reserve1		= 0;						// �\��
//	p_NtDat->ParkData.CarCntInfo.CarCntYear		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntYear;	// �N
//	p_NtDat->ParkData.CarCntInfo.CarCntMon		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntMon;		// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntDay		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntDay;		// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntHour		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntHour;	// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntMin		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntMin;		// ��
//	p_NtDat->ParkData.CarCntInfo.CarCntSeq		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.CarCntSeq;		// �ǔ�
//	p_NtDat->ParkData.CarCntInfo.Reserve1		= p_RcptDat->CMN_DT.DT_54.CarCntInfo.Reserve1;		// �\��
//// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
//	// -------------------------------------
//
//	p_NtDat->CenterSeqNo	= p_RcptDat->CMN_DT.DT_54.CenterSeqNo;			// �Z���^�[�ǔԁi���Ɂj
//
//	p_NtDat->InCount	= p_RcptDat->InCount;								// ���ɒǂ���
//	p_NtDat->Syubet		= (uchar)p_RcptDat->Syubet;							// �����敪
//	p_NtDat->InMode		= (uchar)p_RcptDat->InMode;							// ����Ӱ��
//	p_NtDat->CMachineNo	= (uchar)p_RcptDat->CMachineNo;						// ���Ԍ��@�B��
//	p_NtDat->Reserve	= 0;												// �\���i�T�C�Y�����p�j
//	p_NtDat->FlapArea	= (ushort)(p_RcptDat->LockNo/10000);				// �t���b�v�V�X�e��	���		0�`99
//	p_NtDat->FlapParkNo	= (ushort)(p_RcptDat->LockNo%10000);				// 					�Ԏ��ԍ�	0�`9999
//	p_NtDat->PassCheck	= (uchar)p_RcptDat->PassCheck;						// ����߽����
//	p_NtDat->CountSet	= (uchar)p_RcptDat->CountSet;						// �ݎԶ���
//
//	// ���ɔ}�̏��� 0 ���Z�b�g����
//	memset( &p_NtDat->Media[0], 0, sizeof( p_NtDat->Media ) );				// ���ɔ}�̏��1�`4
//
//// �s��C��(S) K.Onodera 2016/12/09 �A���]���w�E(0�J�b�g�O�̃T�C�Y������Ă���)
////	ret = sizeof( DATA_KIND_54_r10 );
//	ret = sizeof( DATA_KIND_54_r13 );
//// �s��C��(E) K.Onodera 2016/12/09 �A���]���w�E(0�J�b�g�O�̃T�C�Y������Ă���)
//	return ret;
//}
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
//[]----------------------------------------------------------------------[]
///	@brief		���Z���f�[�^(Rev.12�F�t�F�[�Y7)
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/11/04<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//void	NTNET_Snd_Data152_r12( void )
void	NTNET_Snd_Data152_rXX( void )
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
{
	ushort	i;
	ushort	j;
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	ushort	num;
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
	uchar	ErrCode;		// ERRMDL_COIM, ERRMDL_NOTE, ERRMDL_READER��ʂ̺���
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	EMONEY_r14	t_EMoney;
//	CREINFO_r14 t_Credit;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810100(S) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	t_MediaInfoCre2 EncMediaInfo;
// MH810100(E) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)

	ST_OIBAN w_oiban;
	
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_152_r12 ) );
	memset( &SendNtnetDt, 0, sizeof( DATA_KIND_152_rXX ) );
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	/************************/
	/*	��{�ް��쐬		*/
	/************************/
	BasicDataMake( 152, 1 );					// �ް��ێ��׸� = 1�Œ�
// MH310100(S) S.Nishimoto 2020/08/21 �Ԕԃ`�P�b�g���X(#4596 NT-NET���Z���f�[�^�̃V�[�P���V����No���ω����Ȃ�)
	SendNtnetDt.SData152_r12.DataBasic.SeqNo = GetNtDataSeqNo();
// MH310100(E) S.Nishimoto 2020/08/21 �Ԕԃ`�P�b�g���X(#4596 NT-NET���Z���f�[�^�̃V�[�P���V����No���ω����Ȃ�)

// MH321800(S) Y.Tanizaki ���Z���f�[�^Rev.13�Ή�
//	SendNtnetDt.SData152_r12.FmtRev = 12;			// ̫�ϯ�Rev.��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	SendNtnetDt.SData152_r12.FmtRev = 13;			// ̫�ϯ�Rev.��
	SendNtnetDt.SData152_r12.FmtRev = 14;			// ̫�ϯ�Rev.��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH321800(E) Y.Tanizaki ���Z���f�[�^Rev.13�Ή�
	/************************************/
	/*	�@���� = t_KikiStatus			*/		// NTNET_Edit_Data56_r10
	/************************************/
	if( opncls() == 1 ){	// �c�ƒ�
		SendNtnetDt.SData152_r12.Kiki.KyugyoStatus 	= 0;	// �@����_�c�x��	0:�c��
	} else {				// �x�ƒ�
		SendNtnetDt.SData152_r12.Kiki.KyugyoStatus 	= 1;	// �@����_�c�x��	1:�x��
	}
	SendNtnetDt.SData152_r12.Kiki.Lc1Status 		= 0;	// �@����_LC1���	0:OFF/1:ON
	SendNtnetDt.SData152_r12.Kiki.Lc2Status 		= 0;	// �@����_LC2���	0:OFF/1:ON
	SendNtnetDt.SData152_r12.Kiki.Lc3Status 		= 0;	// �@����_LC3���	0:OFF/1:ON
	SendNtnetDt.SData152_r12.Kiki.GateStatus 		= 0;	// �@����_�Q�[�g���	0:��/1:�J
	/********************************************************************/
	/*	AlmAct[ALM_MOD_MAX][ALM_NUM_MAX]��[2][0]��ð���(02:���֘A)���	*/
	/*	00-99��ΏۂɍŌ�ɔ��������װ�									*/
	/*	ALMMDL_SUB2		alm_chk2()	t_EAM_Act	AlmAct					*/
	/********************************************************************/
	SendNtnetDt.SData152_r12.Kiki.NgCardRead 	= NgCard;	// NG�J�[�h�ǎ���e	0���Ȃ�, 1�`255��NT-NET�̃A���[�����02�̃R�[�h���e�Ɠ���
	/****************************************************************************************/
	/*	�������G���[ = t_Erroring															*/
	/*	ErrAct[ERR_MOD_MAX][ERR_NUM_MAX]													*/
	/*	IsErrorOccuerd()�֐��ɂē��Y�̴װ��ʂ��00-99��Ώۂɍŏ�ʂ̎�Ԃ̴װ���ނ�ݒ�	*/
	/****************************************************************************************/
	// ERRMDL_COIM		3.Coin
	ErrCode = NTNET_GetMostErrCode(ERRMDL_COIM);
	SendNtnetDt.SData152_r12.Err.ErrCoinmech	= ErrCode;	// �R�C�����b�N	�G���[���03�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	// ERRMDL_NOTE		4.Note
	ErrCode = NTNET_GetMostErrCode(ERRMDL_NOTE);
	SendNtnetDt.SData152_r12.Err.ErrNoteReader	= ErrCode;	// �������[�_�[	�G���[���04�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	// ERRMDL_READER	1.Reader
	ErrCode = NTNET_GetMostErrCode(ERRMDL_READER);
	SendNtnetDt.SData152_r12.Err.ErrGikiReader	= ErrCode;	// ���C���[�_�[	�G���[���01�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	// �ȉ��AFT-4000 �ł͖��Ή�
	SendNtnetDt.SData152_r12.Err.ErrNoteHarai	= 0;		// �������o�@	�G���[���05�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	SendNtnetDt.SData152_r12.Err.ErrCoinJyunkan	= 0;		// �R�C���z��	�G���[���06�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	SendNtnetDt.SData152_r12.Err.ErrNoteJyunkan	= 0;		// �����z��		�G���[���14�̃R�[�h���e�Ɠ���(���x���̍������̂���Z�b�g����)
	/************************************/
	/*									*/
	/************************************/
	if( PayInfo_Class == 4 ){
		// �N���W�b�g���Z
		SendNtnetDt.SData152_r12.PayClass	= (uchar)0;					// �����敪
	}else{
		SendNtnetDt.SData152_r12.PayClass	= (uchar)PayInfo_Class;		// �����敪
	}
	if(SendNtnetDt.SData152_r12.PayClass == 8) {						// ���Z�O��
		SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_In = 0x1f;		// ����L��	���ڋ���̗L��
		SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_Out = 0x1f;		// ����L��	���ڋ���̗L��
		SendNtnetDt.SData152_r12.AntiPassCheck = 1;						// �A���`�p�X�`�F�b�N	1���`�F�b�NOFF
		goto LSendPayInfo;
	}
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	else if(SendNtnetDt.SData152_r12.PayClass == 100 ||					// �����o��
//			SendNtnetDt.SData152_r12.PayClass == 101 ||					// �s���o��
//			SendNtnetDt.SData152_r12.PayClass == 102) {					// �t���b�v�㏸�O�o��
//		NTNET_Snd_Data152_SK();
//		goto LSendPayInfo;
//	}
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������

	w_oiban.w = ntNet_152_SaveData.Oiban.w;
	w_oiban.i = ntNet_152_SaveData.Oiban.i;
	SendNtnetDt.SData152_r12.PayCount 	= CountSel( &w_oiban);	// ���Z�ǔ�(0�`99999)	syusei[LOCK_MAX]
	SendNtnetDt.SData152_r12.PayMethod 	= ntNet_152_SaveData.PayMethod;	// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
	SendNtnetDt.SData152_r12.PayMode 	= ntNet_152_SaveData.PayMode;	// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z/20��Mifare�v���y�C�h���Z)
// MH810100(S) K.Onodera  2020/02/05  �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
//	SendNtnetDt.SData152_r12.CMachineNo = 0;							// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
	SendNtnetDt.SData152_r12.CMachineNo = ntNet_152_SaveData.CMachineNo;// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
// MH810100(E) K.Onodera  2020/02/05 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)

// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	if(SendNtnetDt.SData152_r12.PayClass == 9 ){								// ���Z��
//		num = ryo_buf.pkiti - 1;
//		SendNtnetDt.SData152_r12.FlapArea 	= (ushort)LockInfo[num].area;		// �t���b�v�V�X�e��	���		0�`99
//		SendNtnetDt.SData152_r12.FlapParkNo = (ushort)LockInfo[num].posi;		// 					�Ԏ��ԍ�	0�`9999
//	}
//	else if(SendNtnetDt.SData152_r12.PayMethod != 5 ){							// ������X�V�ȊO
//		SendNtnetDt.SData152_r12.FlapArea 	= (ushort)(ntNet_152_SaveData.WPlace/10000);	// �t���b�v�V�X�e��	���		0�`99
//		SendNtnetDt.SData152_r12.FlapParkNo = (ushort)(ntNet_152_SaveData.WPlace%10000);	// 					�Ԏ��ԍ�	0�`9999
	if(SendNtnetDt.SData152_r12.PayMethod != 5 ){								// ������X�V�ȊO
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
		SendNtnetDt.SData152_r12.KakariNo 	= (ushort)ntNet_152_SaveData.KakariNo;			// �W����	0�`9999
		SendNtnetDt.SData152_r12.OutKind 	= ntNet_152_SaveData.OutKind;					// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
																			//  	   10���Q�[�g�J��/20��Mifare�v���y�C�h���Z�o��
																			// 		   97�����b�N�J�E�t���b�v�㏸�O�����Z�o��/98�����O�^�C�����o��
																			// 		   99���T�[�r�X�^�C�����o��
	}
	if(SendNtnetDt.SData152_r12.PayClass == 9 ){								// ���Z��
		SendNtnetDt.SData152_r12.CountSet 	= 1;								// �ݎԃJ�E���g 1�����Ȃ�
	}
	else {
		SendNtnetDt.SData152_r12.CountSet 	= ntNet_152_SaveData.CountSet;		// �ݎԃJ�E���g	0������(+1)/1�����Ȃ�/2������(-1)
	}
	/********************************************/
	/*	����/�O�񐸎Z_YMDHMS = t_InPrevYMDHMS	*/
	/********************************************/
	if( SendNtnetDt.SData152_r12.PayMethod != 5 ){					// ������X�V�ȊO
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Year		= (uchar)(ntNet_152_SaveData.carInTime.year%100);	// ����	�N		00�`99 2000�`2099
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Mon		= ntNet_152_SaveData.carInTime.mon;					// 		��		01�`12
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Day		= ntNet_152_SaveData.carInTime.day;					// 		��		01�`31
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Hour		= ntNet_152_SaveData.carInTime.hour;				// 		��		00�`23
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Min		= ntNet_152_SaveData.carInTime.min;					// 		��		00�`59
// GG129000(S) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
//		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Sec		= 0;												// 		�b		00�`59(���C�J�[�h��0�Œ�)
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Sec		= ntNet_152_SaveData.carInTime.sec;					// 		�b		00�`59
// GG129000(E) T.Nagai 2023/01/23 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���Ɏ����ɕb���Z�b�g����j
		// �O�񐸎Z
	}
	/************************************/
	/*									*/
	/************************************/
	SendNtnetDt.SData152_r12.ReceiptIssue 	= ntNet_152_SaveData.ReceiptIssue;	// �̎��ؔ��s�L��	0���̎��؂Ȃ�/1���̎��؂���
	if(SendNtnetDt.SData152_r12.PayMethod != 5 ){					// ������X�V�ȊO
		SendNtnetDt.SData152_r12.Syubet 	= ntNet_152_SaveData.Syubet;	// �������			1�`
		SendNtnetDt.SData152_r12.Price 		= ntNet_152_SaveData.Price;		// ���ԗ���			0�`
	}
	SendNtnetDt.SData152_r12.CashPrice 		= ntNet_152_SaveData.CashPrice;	// ��������			0�`
	SendNtnetDt.SData152_r12.InPrice 		= ntNet_152_SaveData.InPrice;	// �������z			0�`
// MH810100(S) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// �s��C��(S) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
////	SendNtnetDt.SData152_r12.ChgPrice 		= (ushort)ntNet_152_SaveData.ChgPrice;	// �ޑK���z			0�`9999
//	SendNtnetDt.SData152_r12.ChgPrice 		= (ushort)(ntNet_152_SaveData.ChgPrice + ntNet_152_SaveData.FrkReturn);	// �ޑK���z 0�`9999
//// �s��C��(E) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
	SendNtnetDt.SData152_r12.ChgPrice 		= (ushort)ntNet_152_SaveData.ChgPrice;	// �ޑK���z			0�`9999
// MH810100(E) K.Onodera  2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

	if(SendNtnetDt.SData152_r12.PayClass == 9 ){									// ���Z��
		SendNtnetDt.SData152_r12.Syubet 	= (char)(ryo_buf.syubet + 1);			// �������			1�`
		if( ryo_buf.ryo_flg < 2 ) {												// ���Ԍ����Z����
			SendNtnetDt.SData152_r12.Price = ryo_buf.tyu_ryo;						// ���ԗ���
		}
		else {
			SendNtnetDt.SData152_r12.Price = ryo_buf.tei_ryo;						// �������
		}
		SendNtnetDt.SData152_r12.InPrice 	= ryo_buf.nyukin;						// �������z			0�`
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//// �d�l�ύX(S) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
////		SendNtnetDt.SData152_r12.ChgPrice 	= (ushort)ryo_buf.turisen;				// �ޑK���z			0�`9999
//		SendNtnetDt.SData152_r12.ChgPrice 	= (ushort)(ryo_buf.turisen + ryo_buf.kabarai);	// �ޑK���z			0�`9999
//// �d�l�ύX(E) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
		SendNtnetDt.SData152_r12.ChgPrice 	= (ushort)ryo_buf.turisen;				// �ޑK���z			0�`9999
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
	}
	/****************************************************/
	/*	���K���(����������/���o������) = t_MoneyInOut	*/
	/****************************************************/
	SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_In = 0x1f;						// ����L��	���ڋ���̗L��
																				//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
																				//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
	if(SendNtnetDt.SData152_r12.PayClass == 0 || 									// �����敪 ���Z��
		SendNtnetDt.SData152_r12.PayClass == 2 || 									// �����敪 ���Z���~
		SendNtnetDt.SData152_r12.PayClass == 3) { 									// �����敪 �Đ��Z���~
		SendNtnetDt.SData152_r12.MoneyInOut.In_10_cnt    = (uchar)ntNet_152_SaveData.in_coin[0];	// ����������(10�~)		0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_50_cnt    = (uchar)ntNet_152_SaveData.in_coin[1];	// ����������(50�~)		0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_100_cnt   = (uchar)ntNet_152_SaveData.in_coin[2];	// ����������(100�~)	0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_500_cnt   = (uchar)ntNet_152_SaveData.in_coin[3];	// ����������(500�~)	0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_1000_cnt  = (uchar)ntNet_152_SaveData.in_coin[4]	// ����������(1000�~)	0�`255
														+ ntNet_152_SaveData.f_escrow;		// �G�X�N�������ԋp����+1����
		SendNtnetDt.SData152_r12.MoneyInOut.In_2000_cnt  = 0;							// ����������(2000�~)	0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_5000_cnt  = 0;							// ����������(5000�~)	0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_10000_cnt = 0;							// ����������(10000�~)	0�`255
	}
	else if(SendNtnetDt.SData152_r12.PayClass == 9) {								// �����敪 ���Z��
		SendNtnetDt.SData152_r12.MoneyInOut.In_10_cnt    = (uchar)ryo_buf.in_coin[0];	// ����������(10�~)		0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_50_cnt    = (uchar)ryo_buf.in_coin[1];	// ����������(50�~)		0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_100_cnt   = (uchar)ryo_buf.in_coin[2];	// ����������(100�~)	0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_500_cnt   = (uchar)ryo_buf.in_coin[3];	// ����������(500�~)	0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_1000_cnt  = (uchar)ryo_buf.in_coin[4];	// ����������(1000�~)	0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_2000_cnt  = 0;							// ����������(2000�~)	0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_5000_cnt  = 0;							// ����������(5000�~)	0�`255
		SendNtnetDt.SData152_r12.MoneyInOut.In_10000_cnt = 0;							// ����������(10000�~)	0�`255
	}
	SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_Out = 0x1f;						// ����L��	���ڋ���̗L��
																				//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
																				//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
	if(SendNtnetDt.SData152_r12.PayClass == 0 || 									// �����敪 ���Z��
		SendNtnetDt.SData152_r12.PayClass == 2 || 									// �����敪 ���Z���~
		SendNtnetDt.SData152_r12.PayClass == 3) { 									// �����敪 �Đ��Z���~
		SendNtnetDt.SData152_r12.MoneyInOut.Out_10_cnt    = (uchar)ntNet_152_SaveData.out_coin[0];	// ���o������(10�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_50_cnt    = (uchar)ntNet_152_SaveData.out_coin[1];	// ���o������(50�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_100_cnt   = (uchar)ntNet_152_SaveData.out_coin[2];	// ���o������(100�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_500_cnt   = (uchar)ntNet_152_SaveData.out_coin[3];	// ���o������(500�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_1000_cnt  = ntNet_152_SaveData.f_escrow;			// ���o������(1000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_2000cnt   = 0;							// ���o������(2000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_5000_cnt  = 0;							// ���o������(5000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_10000_cnt = 0;							// ���o������(10000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
	}
	else if(SendNtnetDt.SData152_r12.PayClass == 9) {								// �����敪 ���Z��
		SendNtnetDt.SData152_r12.MoneyInOut.Out_10_cnt    = (uchar)ryo_buf.out_coin[0]	// ���o������(10�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
															+ (uchar)ryo_buf.out_coin[4];
		SendNtnetDt.SData152_r12.MoneyInOut.Out_50_cnt    = (uchar)ryo_buf.out_coin[1]	// ���o������(50�~)		0�`255(�����o���s�����͊܂܂Ȃ�)
															+ (uchar)ryo_buf.out_coin[5];
		SendNtnetDt.SData152_r12.MoneyInOut.Out_100_cnt   = (uchar)ryo_buf.out_coin[2]	// ���o������(100�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
															+ (uchar)ryo_buf.out_coin[6];
		SendNtnetDt.SData152_r12.MoneyInOut.Out_500_cnt   = (uchar)ryo_buf.out_coin[3];	// ���o������(500�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_1000_cnt  = 0;							// ���o������(1000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_2000cnt   = 0;							// ���o������(2000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_5000_cnt  = 0;							// ���o������(5000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
		SendNtnetDt.SData152_r12.MoneyInOut.Out_10000_cnt = 0;							// ���o������(10000�~)	0�`255(�����o���s�����͊܂܂Ȃ�)
	}
	/************************************/
	/*									*/
	/************************************/
	SendNtnetDt.SData152_r12.HaraiModoshiFusoku = (ushort)ntNet_152_SaveData.HaraiModoshiFusoku;		// ���ߕs���z	0�`9999
	if( ntNet_152_SaveData.CardFusokuTotal ){
		SendNtnetDt.SData152_r12.CardFusokuType = ntNet_152_SaveData.CardFusokuType;					// ���o�s���}�̎��(�����ȊO)
		SendNtnetDt.SData152_r12.CardFusokuTotal = ntNet_152_SaveData.CardFusokuTotal;					// ���o�s���z(�����ȊO)
	}
	SendNtnetDt.SData152_r12.Reserve1 = 0;											// �\��(�T�C�Y�����p)	0
	if( SendNtnetDt.SData152_r12.PayClass != 8 &&									// ���Z�O�ȊO
		SendNtnetDt.SData152_r12.PayClass != 9) {									// ���Z���ȊO
		SendNtnetDt.SData152_r12.AntiPassCheck = ntNet_152_SaveData.AntiPassCheck;	// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�NOFF/2������OFF
	}
	else {																		// ���Z�Oor���Z��
		SendNtnetDt.SData152_r12.AntiPassCheck = 1;									// �A���`�p�X�`�F�b�N	1���`�F�b�NOFF
	}
																				//						��������g�p���Ȃ��ꍇ�́u�P���`�F�b�NOFF�v�Œ�
	if( (SendNtnetDt.SData152_r12.PayClass != 9 && ntNet_152_SaveData.ParkNoInPass) || 		// ���Z���ȊO ��������Z����
		(SendNtnetDt.SData152_r12.PayClass == 9 && ryo_buf.ryo_flg >= 2) ) {		// ���Z�� ��������Z����
		if( SendNtnetDt.SData152_r12.PayClass != 9 ){
			SendNtnetDt.SData152_r12.ParkNoInPass = ntNet_152_SaveData.ParkNoInPass;		// ������@���ԏꇂ	0�`999999
		}
		else {
			SendNtnetDt.SData152_r12.ParkNoInPass = CPrmSS[S_SYS][ntNet_152_SaveData.pkno_syu+1];	// ������@���ԏꇂ	0�`999999
		}
// MH810100(S) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
//		/****************************************/
//		/*	���Z�}�̏��1, 2 = t_MediaInfo		*/
//		/****************************************/
//		SendNtnetDt.SData152_r12.Media[0].MediaKind = (ushort)(ntNet_152_SaveData.pkno_syu + 2);	// ���Z�}�̏��P�@���(���C���}��)	0�`99
//		// �J�[�h�ԍ�[30]
//		intoasl(SendNtnetDt.SData152_r12.Media[0].MediaCardNo, ntNet_152_SaveData.teiki_id, 5);		// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		// �J�[�h���[16]
//		intoasl(SendNtnetDt.SData152_r12.Media[0].MediaCardInfo, ntNet_152_SaveData.teiki_syu, 2);	// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		// ���Z�}�̏��2�i�T�u�}�́j�� �I�[���O�Ƃ���
	}
	if( ntNet_152_SaveData.MediaKind1 ){														// ���Z�}�̏��1��ʂ���H
		/************************************/
		/*	���Z�}�̏��1 = t_MediaInfo		*/
		/************************************/
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;				// �Í�������
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;				// �Í����ԍ�
		SendNtnetDt.SData152_r12.Media[0].MediaKind	= ntNet_152_SaveData.MediaKind1;			// ���Z�}�̏��1���(���C���}��) = ���Ԍ�(101)/�����(102)
		memset( &EncMediaInfo, 0, sizeof( t_MediaInfoCre2 ) );
		memcpy( EncMediaInfo.MediaCardNo, ntNet_152_SaveData.MediaCardNo1,						// �J�[�h�ԍ�(���Ԍ�:���Ԍ��@�B��(���ɋ@�B��)(���3��) + ���Ԍ��ԍ�(����6��)
														sizeof( EncMediaInfo.MediaCardNo ) );	//            �����:�����ID(5��))
		memcpy( EncMediaInfo.MediaCardInfo, ntNet_152_SaveData.MediaCardInfo1,					// �J�[�h���(���Ԍ�:�|(�Ȃ�)
														sizeof( EncMediaInfo.MediaCardInfo ) );	//            �����:��������(2��))
		AesCBCEncrypt( EncMediaInfo.MediaCardNo, sizeof( t_MediaInfoCre2 ) );					// AES �Í���
		memcpy( SendNtnetDt.SData152_r12.Media[0].MediaCardNo, EncMediaInfo.MediaCardNo,		// �J�[�h�ԍ�(30Byte) + �J�[�h���(16Byte) + �\��(2Byte)
														 sizeof( t_MediaInfoCre2 ) );
	}
	if( ntNet_152_SaveData.MediaKind2 == CARD_TYPE_CAR_NUM ){									// ���Z�}�̏��2��� = �Ԕ�(203)����H
		/************************************/
		/*	���Z�}�̏��2 = t_MediaInfo		*/
		/************************************/
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;				// �Í�������
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;				// �Í����ԍ�
		SendNtnetDt.SData152_r12.Media[1].MediaKind	= CARD_TYPE_CAR_NUM;						// ���Z�}�̏��2���(�T�u�}��) = �Ԕ�(203)
		memset( &EncMediaInfo, 0, sizeof( t_MediaInfoCre2 ) );
		memcpy( EncMediaInfo.MediaCardNo, ntNet_152_SaveData.MediaCardNo2,						// �J�[�h�ԍ�(���^�x�ǖ�(2Byte) + ���ޔԍ�(3Byte)
														sizeof( EncMediaInfo.MediaCardNo ) );	//            + �p�r����(2Byte) + ��A�w��ԍ�(4Byte))
		AesCBCEncrypt( EncMediaInfo.MediaCardNo, sizeof( t_MediaInfoCre2 ) );					// AES �Í���
		memcpy( SendNtnetDt.SData152_r12.Media[1].MediaCardNo, EncMediaInfo.MediaCardNo,		// �J�[�h�ԍ�(30Byte) + �J�[�h���(16Byte) + �\��(2Byte)
														 sizeof( t_MediaInfoCre2 ) );
// MH810100(E) K.Onodera  2020/02/17 �Ԕԃ`�P�b�g���X(ParkingWeb���Z�f�[�^���Z�}��(�Ԕ�)�Ή�)
	}
	/************************************/
	/*									*/
	/************************************/
	if( ntNet_152_SaveData.e_pay_kind != 0 ){									// �d�q���ώ�� Suica:1, Edy:2
// MH321800(S) hosoda IC�N���W�b�g�Ή�
//-		SendNtnetDt.SData152_r12.CardKind = (ushort)2;								// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
		switch (ntNet_152_SaveData.e_pay_kind) {
		case EC_EDY_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)3;
			break;
		case EC_NANACO_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)6;
			break;
		case EC_WAON_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)5;
			break;
		case EC_SAPICA_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)7;
			break;
		case EC_ID_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)12;
			break;
		case EC_QUIC_PAY_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)13;
			break;
// MH810105(S) MH364301 PiTaPa�Ή�
		case EC_PITAPA_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)4;
			break;
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case EC_QR_USED:
			SendNtnetDt.SData152_r12.CardKind = (ushort)16;
			break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		case EC_KOUTSUU_USED:	// ��ʌnIC�J�[�h
		default:		// 2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
			SendNtnetDt.SData152_r12.CardKind = (ushort)2;
			break;
		}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	}else if( PayInfo_Class == 4 ){												// �N���W�b�g���Z
	}else if( PayInfo_Class == 4 || ntNet_152_SaveData.c_pay_ryo != 0 ){			// �N���W�b�g���Z
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		SendNtnetDt.SData152_r12.CardKind = (ushort)1;								// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
	}

	switch( SendNtnetDt.SData152_r12.CardKind ){									// �J�[�h���ϋ敪	0=�Ȃ��A1=�N���W�b�g�A2=��ʌn�d�q�}�l�[(Suica,PASMO,ICOCA��)
	case 1:
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// �Í�������
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// �Í����ԍ�
		memset( &t_Credit, 0, sizeof( t_Credit ) );
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		t_Credit.amount = (ulong)ntNet_152_SaveData.c_pay_ryo;					// ���ϊz
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
////		memcpyFlushLeft( &t_Credit.card_no[0], &ntNet_152_SaveData.c_Card_No[0], sizeof(t_Credit.card_no), sizeof(ntNet_152_SaveData.c_Card_No) );	// ����ԍ�
////		memcpyFlushLeft( &t_Credit.cct_num[0], &ntNet_152_SaveData.c_cct_num[0],
////							sizeof(t_Credit.cct_num), CREDIT_TERM_ID_NO_SIZE );	// �[�����ʔԍ�
////		memcpyFlushLeft( &t_Credit.kid_code[0], &ntNet_152_SaveData.c_kid_code[0], sizeof(t_Credit.kid_code), sizeof(ntNet_152_SaveData.c_kid_code) );	// KID �R�[�h
//// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
////		memcpy( &t_Credit.card_no[0], &ntNet_152_SaveData.c_Card_No[0], sizeof(t_Credit.card_no) );		// ����ԍ�
//		// �݂Ȃ����ώ��̓X�y�[�X���߂���
//		memcpyFlushLeft( &t_Credit.card_no[0], &ntNet_152_SaveData.c_Card_No[0],
//						sizeof(t_Credit.card_no), sizeof(ntNet_152_SaveData.c_Card_No) );	// ����ԍ�
//// MH321800(E) NT-NET���Z�f�[�^�d�l�ύX
//		// ���No�̏�7���ځ`��5���ڈȊO��'*'���i�[����Ă�����'0'�ɒu��
//		change_CharInArray( &t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '*', '0' );
//// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
////		memcpyFlushLeft2( &t_Credit.cct_num[0], &ntNet_152_SaveData.c_cct_num[0], sizeof(t_Credit.cct_num), sizeof(ntNet_152_SaveData.c_cct_num) );		// �[�����ʔԍ�
////		memcpy( &t_Credit.kid_code[0], &ntNet_152_SaveData.c_kid_code[0], sizeof(t_Credit.kid_code) );	// KID �R�[�h
//		// �݂Ȃ����ώ��̓X�y�[�X���߂���
//		memcpyFlushLeft( &t_Credit.cct_num[0], &ntNet_152_SaveData.c_cct_num[0],
//						sizeof(t_Credit.cct_num), sizeof(ntNet_152_SaveData.c_cct_num));	// �[�����ʔԍ�
//		// �݂Ȃ����ώ��̓X�y�[�X���߂���
//		memcpyFlushLeft( &t_Credit.kid_code[0], &ntNet_152_SaveData.c_kid_code[0],
//						sizeof(t_Credit.kid_code), sizeof(ntNet_152_SaveData.c_kid_code) );	// KID�R�[�h
//// MH321800(E) NT-NET���Z�f�[�^�d�l�ύX
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//		t_Credit.app_no = ntNet_152_SaveData.c_app_no;							// ���F�ԍ�
//		t_Credit.center_oiban = 0;												// ���������ǔ�
//		memcpyFlushLeft( &t_Credit.ShopAccountNo[0], &ntNet_152_SaveData.c_trade_no[0], sizeof(t_Credit.ShopAccountNo), sizeof(ntNet_152_SaveData.c_trade_no) );	// �����X����ԍ�
//		t_Credit.slip_no = ntNet_152_SaveData.c_slipNo;							// �`�[�ԍ�
		// ���ϒ[���敪
		SendNtnetDt.SData152_r12.PayTerminalClass = 1;
		// ����X�e�[�^�X
		SendNtnetDt.SData152_r12.Transactiontatus = ntNet_152_SaveData.e_Transactiontatus;

		// ���p���z
		if (ntNet_152_SaveData.e_Transactiontatus != 3) {
			t_Credit.amount = ntNet_152_SaveData.c_pay_ryo;
		}
		else {
			// �����x���s���͗��p���z0�~�Ƃ���
			t_Credit.amount = 0;
		}

		// �J�[�h�ԍ�
		memcpyFlushLeft(&t_Credit.card_no[0],
						&ntNet_152_SaveData.c_Card_No[0],
						sizeof(t_Credit.card_no),
						sizeof(ntNet_152_SaveData.c_Card_No) );
		// ���No�̏�7���ځ`��5���ڈȊO��'*'���i�[����Ă�����'0'�ɒu��
		change_CharInArray( &t_Credit.card_no[0], sizeof(t_Credit.card_no), 7, 5, '*', '0' );

		// �[�����ʔԍ�
		memcpyFlushLeft(&t_Credit.cct_num[0],
						&ntNet_152_SaveData.c_cct_num[0],
						sizeof(t_Credit.cct_num),
						sizeof(ntNet_152_SaveData.c_cct_num));
		// KID�R�[�h
		memcpyFlushLeft(&t_Credit.kid_code[0],
						&ntNet_152_SaveData.c_kid_code[0],
						sizeof(t_Credit.kid_code),
						sizeof(ntNet_152_SaveData.c_kid_code));
		// ���F�ԍ�
		t_Credit.app_no = ntNet_152_SaveData.c_app_no;
		// �`�[�ԍ�
		t_Credit.slip_no = ntNet_152_SaveData.c_slipNo;

		// ���������ǔ�
		t_Credit.center_oiban = 0;
		// �����X����ԍ�
		memset(&t_Credit.ShopAccountNo[0], 0x20, sizeof(t_Credit.ShopAccountNo));
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		AesCBCEncrypt( (uchar *)&t_Credit, sizeof( t_Credit ) );				// AES �Í���
		memcpy( (uchar *)&SendNtnetDt.SData152_r12.settlement[0], (uchar *)&t_Credit, sizeof( t_Credit ) );	// ���Ϗ��
		break;
	case 2:
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	case 3:				// Edy
	case 5:				// WAON
	case 6:				// nanaco
	case 7:				// SAPICA
	case 12:			// iD
	case 13:			// QUICPay
// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810105(S) MH364301 PiTaPa�Ή�
	case 4:				// PiTaPa
// MH810105(E) MH364301 PiTaPa�Ή�
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// �Í�������
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// �Í����ԍ�
		memset( &t_EMoney, 0, sizeof( t_EMoney ) );
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		t_EMoney.amount = ntNet_152_SaveData.e_pay_ryo;							// �d�q���ϐ��Z���@���ϊz
//		memset(&t_EMoney.card_id[0], 0x20, sizeof(t_EMoney.card_id));
//		memcpy( &t_EMoney.card_id[0], &ntNet_152_SaveData.e_Card_ID[0],
//				sizeof(ntNet_152_SaveData.e_Card_ID) );						// �d�q���ϐ��Z���@����ID (Ascii 16��)
//		t_EMoney.card_zangaku = ntNet_152_SaveData.e_pay_after;					// �d�q���ϐ��Z���@���ό�c��
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//		if( isEC_USE() ){														// ���σ��[�_�ł̐��Z�H
//// MH810103 GG119202(S) �d�q�}�l�[�Ή�
////			// �݂Ȃ����ώ���NULL(0x0)�Ŗ��߂�
////			memcpyFlushLeft2( &t_EMoney.inquiry_num[0], &ntNet_152_SaveData.e_inquiry_num[0],
////					sizeof(ntNet_152_SaveData.e_inquiry_num), sizeof(t_EMoney.inquiry_num) );	// �⍇���ԍ�
//// MH810103 GG119201(S) ���Ϗ��ɖ₢���킹�ԍ����Z�b�g���Ȃ�
//			// ���Ϗ��ɖ⍇���ԍ����Z�b�g����d�l�̓t�H�[�}�b�gRev14�ȍ~�őΉ�
////			// �݂Ȃ����ώ���NULL(0x0)�Ŗ��߂�
////			memcpyFlushLeft( &t_EMoney.inquiry_num[0], &ntNet_152_SaveData.e_inquiry_num[0],
////						sizeof(t_EMoney.inquiry_num), sizeof(ntNet_152_SaveData.e_inquiry_num) );	// �₢���킹�ԍ�
//// MH810103 GG119201(E) ���Ϗ��ɖ₢���킹�ԍ����Z�b�g���Ȃ�
//// MH810103 GG119202(E) �d�q�}�l�[�Ή�
//		}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
		// ���ϒ[���敪
		SendNtnetDt.SData152_r12.PayTerminalClass = 1;

		// ����X�e�[�^�X
		SendNtnetDt.SData152_r12.Transactiontatus = ntNet_152_SaveData.e_Transactiontatus;

		// ���p���z
		if (ntNet_152_SaveData.e_Transactiontatus != 3) {
			t_EMoney.amount = ntNet_152_SaveData.e_pay_ryo;
		}
		else {
			t_EMoney.amount = 0;
		}

		// �J�[�h�c�z
		t_EMoney.card_zangaku = ntNet_152_SaveData.e_pay_after;
		// �J�[�h�ԍ�
		memcpyFlushLeft(&t_EMoney.card_id[0],
						&ntNet_152_SaveData.e_Card_ID[0],
						sizeof(t_EMoney.card_id),
						sizeof(ntNet_152_SaveData.e_Card_ID) );
		// �₢���킹�ԍ�
		memcpyFlushLeft(&t_EMoney.inquiry_num[0],
						&ntNet_152_SaveData.e_inquiry_num[0],
						sizeof(t_EMoney.inquiry_num),
						sizeof(ntNet_152_SaveData.e_inquiry_num) );
		// ���F�ԍ�
		t_EMoney.approbal_no = ntNet_152_SaveData.c_app_no;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		AesCBCEncrypt( (uchar *)&t_EMoney, sizeof( t_EMoney ) );				// AES �Í���
		memcpy( (uchar *)&SendNtnetDt.SData152_r12.settlement[0], (uchar *)&t_EMoney, sizeof( t_EMoney ) );	// ���Ϗ��
		break;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	case 16:			// QR�R�[�h����
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptMode = (ushort)1;	// �Í�������
		SendNtnetDt.SData152_r12.DataBasic.CMN_DT.DT_BASIC.encryptKey = (ushort)0;	// �Í����ԍ�
		memset(&t_QR, 0, sizeof(t_QR));

		// ���ϒ[���敪
		SendNtnetDt.SData152_r12.PayTerminalClass = 1;
		// ����X�e�[�^�X
		SendNtnetDt.SData152_r12.Transactiontatus = ntNet_152_SaveData.e_Transactiontatus;

		// ���p���z
		if (ntNet_152_SaveData.e_Transactiontatus != 3) {
			t_QR.amount = ntNet_152_SaveData.e_pay_ryo;
		}
		else {
			t_QR.amount = 0;
		}

		// Mch����ԍ�
		memcpyFlushLeft(&t_QR.MchTradeNo[0],
						&ntNet_152_SaveData.qr_MchTradeNo[0],
						sizeof(t_QR.MchTradeNo),
						sizeof(ntNet_152_SaveData.qr_MchTradeNo));
		// �x���[��ID
		memcpyFlushLeft(&t_QR.PayTerminalNo[0],
						&ntNet_152_SaveData.qr_PayTermID[0],
						sizeof(t_QR.PayTerminalNo),
						sizeof(ntNet_152_SaveData.qr_PayTermID));
		// ����ԍ�
		memcpyFlushLeft(&t_QR.DealNo[0],
						&ntNet_152_SaveData.e_inquiry_num[0],
						sizeof(t_QR.DealNo),
						sizeof(ntNet_152_SaveData.e_inquiry_num));

		// ���σu�����h
		t_QR.PayKind = ntNet_152_SaveData.qr_PayKind;

		AesCBCEncrypt( (uchar *)&t_QR, sizeof( t_QR ) );				// AES �Í���
		memcpy( (uchar *)&SendNtnetDt.SData152_r12.settlement[0], (uchar *)&t_QR, sizeof( t_QR ) );	// ���Ϗ��
		break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	}
	/************************************/
	/*	���� = t_SeisanDiscount			*/
	/************************************/
	for( i = j = 0; i < WTIK_USEMAX; i++ ){								// ���Z���~�ȊO�̊������R�s�[
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 		if(( ntNet_152_SaveData.DiscountData[i].DiscSyu != 0 ) &&					// ������ʂ���
// 		   (( ntNet_152_SaveData.DiscountData[i].DiscSyu < NTNET_CSVS_M ) ||		// ���Z���~�������łȂ�
// 		    ( NTNET_CFRE < ntNet_152_SaveData.DiscountData[i].DiscSyu ))){
// 		    if( ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SUICA_1 &&		// SUICA�ȊO(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SUICA_2 &&		//			(�x���z�E�c�z)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_PASMO_1 &&		// PASMO�ȊO(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_PASMO_2 &&		//			(�x���z�E�c�z)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_EDY_1 &&		// EDY�ȊO	(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_EDY_2 &&		//			(�x���z�E�c�z)
// // MH321800(S) hosoda IC�N���W�b�g�Ή�
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_WAON_1 &&		// WAON�ȊO	(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_WAON_2 &&		//			(�x���z�E�c�z)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SAPICA_1 &&		// SAPICA�ȊO	(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_SAPICA_2 &&		//			(�x���z�E�c�z)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_NANACO_1 &&		// NANACO�ȊO	(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_NANACO_2 &&		//			(�x���z�E�c�z)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ID_1 &&			// iD�ȊO	(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ID_2 &&			//			(�x���z�E�c�z)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_QUICPAY_1 &&	// QUICPay�ȊO	(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_QUICPAY_2 &&	//			(�x���z�E�c�z)
// // MH321800(E) hosoda IC�N���W�b�g�Ή�
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICOCA_1 &&		// ICOCA�ȊO(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICOCA_2 &&		//			(�x���z�E�c�z)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICCARD_1 &&	// IC-Card�ȊO(�ԍ�)
// 		    	ntNet_152_SaveData.DiscountData[i].DiscSyu != NTNET_ICCARD_2 ) 	//			(�x���z�E�c�z)
// GG124100(S) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6522 �����z��0�~�̊�����NT-NET���Z�f�[�^�̊������ɃZ�b�g����Ȃ�
// 		// �u������ʁv�͊����v�Z���Ƀ`�F�b�N�ς݂̂��߁u����g�p�����������z�v�̗L���̂݃`�F�b�N����
// 		if ( ntNet_152_SaveData.DiscountData[i].Discount > 0 ) {
		// �u������ʁv�͊����v�Z���Ƀ`�F�b�N�ς݂̂��߂����ł̓`�F�b�N���Ȃ�
		if ( ntNet_152_SaveData.DiscountData[i].Discount ||		// ����g�p�����������z
			 ntNet_152_SaveData.DiscountData[i].DiscCount ) {	// ����g�p��������
// GG124100(E) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6522 �����z��0�~�̊�����NT-NET���Z�f�[�^�̊������ɃZ�b�g����Ȃ�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
		    {
				SendNtnetDt.SData152_r12.SDiscount[j].ParkingNo	= ntNet_152_SaveData.DiscountData[i].ParkingNo;		// ���ԏ�No.
				SendNtnetDt.SData152_r12.SDiscount[j].Kind		= ntNet_152_SaveData.DiscountData[i].DiscSyu;		// �������
				SendNtnetDt.SData152_r12.SDiscount[j].Group		= ntNet_152_SaveData.DiscountData[i].DiscNo;		// �����敪
				SendNtnetDt.SData152_r12.SDiscount[j].Callback	= ntNet_152_SaveData.DiscountData[i].DiscCount;		// �������
				SendNtnetDt.SData152_r12.SDiscount[j].Amount	= ntNet_152_SaveData.DiscountData[i].Discount;		// �����z
				SendNtnetDt.SData152_r12.SDiscount[j].Info1		= ntNet_152_SaveData.DiscountData[i].DiscInfo1;		// �������1
				if( SendNtnetDt.SData152_r12.SDiscount[j].Kind != NTNET_PRI_W ){
					SendNtnetDt.SData152_r12.SDiscount[j].Info2 = ntNet_152_SaveData.DiscountData[i].uDiscData.common.DiscInfo2;	// �������2
				}
				j++;
		    }
		}
	}
	for( i=0; i<DETAIL_SYU_MAX; i++ ){
		if( ntNet_152_SaveData.DetailData[i].DiscSyu != 0  ){
			SendNtnetDt.SData152_r12.SDiscount[j].ParkingNo	= ntNet_152_SaveData.DetailData[i].ParkingNo;					// ���ԏ�No.
			SendNtnetDt.SData152_r12.SDiscount[j].Kind		= ntNet_152_SaveData.DetailData[i].DiscSyu;						// �������
			SendNtnetDt.SData152_r12.SDiscount[j].Group		= ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscNo;		// �����敪
			SendNtnetDt.SData152_r12.SDiscount[j].Callback	= ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscCount;	// �������
			SendNtnetDt.SData152_r12.SDiscount[j].Amount	= ntNet_152_SaveData.DetailData[i].uDetail.Common.Discount;		// �����z
			SendNtnetDt.SData152_r12.SDiscount[j].Info1		= ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscInfo1;	// �������1
			SendNtnetDt.SData152_r12.SDiscount[j].Info2		= ntNet_152_SaveData.DetailData[i].uDetail.Common.DiscInfo2;	// �������2
			j++;
		}
	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH321800(S) �������8000�̃Z�b�g���@���C��
//	// ���σ��[�_�ڑ�����
//	if (isEC_USE()) {
//		if (ntNet_152_SaveData.c_pay_ryo != 0) {
//			// �N���W�b�g����
//			/* �󂫴ر���� */
//			for (i = 0; (i < NTNET_DIC_MAX) &&
//					(0 != SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo); i++) {
//				;
//			}		/* �f�[�^�������				*/
//			if (i < NTNET_DIC_MAX) {
//				SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];			// ���ԏ�No.
//				SendNtnetDt.SData152_r12.SDiscount[i].Kind = NTNET_INQUIRY_NUM;				// �������
//				// �N���W�b�g���ώ��͖₢���킹�ԍ�����M���Ȃ��̂�0x20���߂Ƃ���
//				memset(&SendNtnetDt.SData152_r12.SDiscount[i].Group, 0x20, 16);
//			}
//		}
//		else if (ntNet_152_SaveData.e_pay_ryo != 0) {
//			// �d�q�}�l�[����
//			// �������ɃJ�[�hNo.�Ɨ��p�z�̓Z�b�g���Ȃ�
//			/* �󂫴ر���� */
//			for (i = 0; (i < NTNET_DIC_MAX) &&
//					(0 != SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo); i++) {
//				;
//			}		/* �f�[�^�������				*/
//			if (i < NTNET_DIC_MAX) {
//				SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];			// ���ԏ�No.
//				SendNtnetDt.SData152_r12.SDiscount[i].Kind = NTNET_INQUIRY_NUM;				// �������
//// MH810103 GG119202(S) �d�q�}�l�[�Ή�
////				memcpy(&SendNtnetDt.SData152_r12.SDiscount[i].Group, ntNet_152_SaveData.e_inquiry_num, 16);
//				memcpyFlushLeft( (uchar *)&SendNtnetDt.SData152_r12.SDiscount[i].Group, (uchar *)&ntNet_152_SaveData.e_inquiry_num[0],
//									16, sizeof( ntNet_152_SaveData.e_inquiry_num ) );		// �₢���킹�ԍ�
//// MH810103 GG119202(E) �d�q�}�l�[�Ή�
//			}
//		}
//	}
//// MH321800(E) �������8000�̃Z�b�g���@���C��
	if ((PayInfo_Class == 2 || PayInfo_Class == 3) &&
		ntNet_152_SaveData.e_Transactiontatus == 3) {
		// ���ϐ��Z���~
		/* �󂫴ر���� */
		for (i = 0; (i < NTNET_DIC_MAX) &&
				(0 != SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo); i++) {
			;
		}		/* �f�[�^�������				*/
		if (i < NTNET_DIC_MAX) {
			SendNtnetDt.SData152_r12.SDiscount[i].ParkingNo = CPrmSS[S_SYS][1];	// ���ԏ�No.
			SendNtnetDt.SData152_r12.SDiscount[i].Kind = NTNET_MIRYO_UNKNOWN;	// �������
			SendNtnetDt.SData152_r12.SDiscount[i].Group =
											SendNtnetDt.SData152_r12.CardKind;	// �����敪
			SendNtnetDt.SData152_r12.SDiscount[i].Callback = 1;					// �g�p����
			SendNtnetDt.SData152_r12.SDiscount[i].Amount = 0;					// �����z

			if (ntNet_152_SaveData.c_pay_ryo != 0) {
				SendNtnetDt.SData152_r12.SDiscount[i].Info1 =
												ntNet_152_SaveData.c_pay_ryo;	// �������1
			}
			else if (ntNet_152_SaveData.e_pay_ryo != 0) {
				SendNtnetDt.SData152_r12.SDiscount[i].Info1 =
												ntNet_152_SaveData.e_pay_ryo;	// �������1
			}

			SendNtnetDt.SData152_r12.SDiscount[i].Info2 = 0;					// �������2
		}
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	/********************/
	/*	�ް����M�o�^	*/
	/********************/
LSendPayInfo:
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	if(_is_ntnet_remote()) {
//		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_152_r12));
//	} else {
//		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_152_r12), NTNET_BUF_NORMAL);
//	}
	if(_is_ntnet_remote()) {
		RAU_SetSendNtData((const uchar*)&SendNtnetDt, sizeof(DATA_KIND_152_rXX));
	} else {
		NTBUF_SetSendNtData(&SendNtnetDt, sizeof(DATA_KIND_152_rXX), NTNET_BUF_NORMAL);
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
}
/*[]----------------------------------------------------------------------[]*
 *| ���Z���f�[�^�s���o�ɁE�����o�ɁE�t���b�v�㏸�O���M����
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
// MH810100(S) K.Onodera  2019/12/25 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:���[�j���O�΍�)
//static	void	NTNET_Snd_Data152_SK(void)
void	NTNET_Snd_Data152_SK(void)
// MH810100(E) K.Onodera  2019/12/25 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������:���[�j���O�΍�)
{
	SendNtnetDt.SData152_r12.PayClass 	= 0;							// �����敪(0�����Z/1���Đ��Z/2�����Z���~/3���Đ��Z���~/(4���N���W�b�g���Z��))
	SendNtnetDt.SData152_r12.PayCount 	= CountSel(&PayInfoData_SK.Oiban);	// �����o��or�s���o�ɒǔ�or�t���b�v�㏸�O�i���Z�ǔԁj
	SendNtnetDt.SData152_r12.PayMethod 	= 0;							// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
	SendNtnetDt.SData152_r12.PayMode 	= 0;							// ���Z���[�h(0���������Z/1�����������Z/2���蓮���Z/3�����Z�Ȃ�/4�����u���Z/20��Mifare�v���y�C�h���Z)
	SendNtnetDt.SData152_r12.CMachineNo = 0;							// ���Ԍ��@�B��(���ɋ@�B��)	0�`255

	SendNtnetDt.SData152_r12.FlapArea 	= (ushort)(PayInfoData_SK.WPlace/10000);	// �t���b�v�V�X�e��	���		0�`99
	SendNtnetDt.SData152_r12.FlapParkNo = (ushort)(PayInfoData_SK.WPlace%10000);	// 					�Ԏ��ԍ�	0�`9999
	SendNtnetDt.SData152_r12.KakariNo 	= PayInfoData_SK.KakariNo;		// �W����	0�`9999
	SendNtnetDt.SData152_r12.OutKind 	= PayInfoData_SK.OutKind;		// ���Z�o��	0���ʏ퐸�Z/1�������o��/(2�����Z�Ȃ��o��)/3���s���o��/9���˔j�o��
	SendNtnetDt.SData152_r12.CountSet	= PayInfoData_SK.CountSet;		// �ݎԶ���-1����

	/********************************************/
	/*	����/�O�񐸎Z_YMDHMS = t_InPrevYMDHMS	*/
	/********************************************/
	if( SendNtnetDt.SData152_r12.PayMethod != 5 ){					// ������X�V�ȊO
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Year	= 
										(uchar)(PayInfoData_SK.TInTime.Year % 100);	// ����	�N		00�`99 2000�`2099
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Mon	= PayInfoData_SK.TInTime.Mon;	// 		��		01�`12
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Day	= PayInfoData_SK.TInTime.Day;	// 		��		01�`31
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Hour	= PayInfoData_SK.TInTime.Hour;	// 		��		00�`23
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Min	= PayInfoData_SK.TInTime.Min;	// 		��		00�`59
		SendNtnetDt.SData152_r12.InPrev_ymdhms.In_Sec	= 0;							// 		�b		00�`59(���C�J�[�h��0�Œ�)
	}

	SendNtnetDt.SData152_r12.Syubet 	= PayInfoData_SK.syu;					// �������			1�`
	SendNtnetDt.SData152_r12.Price 		= PayInfoData_SK.WPrice;				// ���ԗ���			0�`

	SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_In = 0x1f;					// ����L��	���ڋ���̗L��
																			//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
																			//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
	SendNtnetDt.SData152_r12.MoneyInOut.MoneyKind_Out = 0x1f;					// ����L��	���ڋ���̗L��
																			//			1bit��:10�~�A2bit��:50�~�A3bit��:100�~�A4bit��:500�~
																			//			5bit��:1000�~�A6bit��:2000�~�A7bit��:5000�~�A8bit��:10000�~
	SendNtnetDt.SData152_r12.AntiPassCheck = PayInfoData_SK.PassCheck;			// �A���`�p�X�`�F�b�N	0���`�F�b�NON/1���`�F�b�NOFF/2������OFF
																			//						��������g�p���Ȃ��ꍇ�́u�P���`�F�b�NOFF�v�Œ�

}
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
// �d�l�ύX(S) K.Onodera 2016/11/04 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�W�v��{�f�[�^�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		syukei	: ���O������o�����W�v�f�[�^�̃|�C���^
///	@param[in]		Type	: �W�v�^�C�v
///	@param[out]		p_NtDat	: �W�v�I���ʒm�f�[�^(DATA_KIND_42_r13�^)�ւ̃|�C���^  
///	@return			ret		: �W�v�I���ʒm�f�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//unsigned short	NTNET_Edit_SyukeiKihon_r13( SYUKEI *syukei, ushort Type, DATA_KIND_42_r13 *p_NtDat )
unsigned short	NTNET_Edit_SyukeiKihon_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_42 *p_NtDat )
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
{
	static const ushort c_coin[COIN_SYU_CNT] = {10, 50, 100, 500};
	ushort	i;
	ushort	ret;

// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//	memset( p_NtDat, 0, sizeof( DATA_KIND_42_r13 ) );
	memset( p_NtDat, 0, sizeof( DATA_KIND_42 ) );
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
	NTNET_Edit_BasicData( 42, 0, syukei->SeqNo[0], &p_NtDat->DataBasic);	// �s���v�F�f�[�^�ێ��t���O = 0

	p_NtDat->DataBasic.Year = (uchar)(syukei->NowTime.Year % 100 );				// �����N
	p_NtDat->DataBasic.Mon = (uchar)syukei->NowTime.Mon;						// ������
	p_NtDat->DataBasic.Day = (uchar)syukei->NowTime.Day;						// ������
	p_NtDat->DataBasic.Hour = (uchar)syukei->NowTime.Hour;						// ������
	p_NtDat->DataBasic.Min = (uchar)syukei->NowTime.Min;						// ������
	p_NtDat->DataBasic.Sec = 0;													// �����b

// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//	p_NtDat->FmtRev					= 13;										// �t�H�[�}�b�gRev.No.
	p_NtDat->FmtRev					= 15;										// �t�H�[�}�b�gRev.No.
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
	p_NtDat->Type					= (Type == 1) ? 1 : 2;						// �W�v�^�C�v 1:T, 2:GT
	p_NtDat->KakariNo				= syukei->Kakari_no;						// �W��No.
	p_NtDat->SeqNo					= CountSel(&syukei->Oiban);					// �W�v�ǔ�
	switch(prm_get(COM_PRM, S_SYS, 13, 1, 1)){
	case	0:		// �ʒǔ�
		if (Type == 11) {														// GT�W�v
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_T_TOTAL][1];		// �I���ǔ�
		}
		break;
	case	1:		// �ʂ��ǂ���
		if (Type == 1 || Type == 11) {
			p_NtDat->StartSeqNo		= syukei->oiban_range[_OBN_WHOLE][0];		// �J�n�ǔ�
			p_NtDat->EndSeqNo		= syukei->oiban_range[_OBN_WHOLE][1];		// �I���ǔ�
		}
		break;
	default:
		break;
	}
	p_NtDat->CenterSeqNo 			= syukei->CenterSeqNo;						// �Z���^�[�ǔԁi�W�v�j
	p_NtDat->LastTime.Year = (uchar)(syukei->OldTime.Year % 100 );				// �����N
	p_NtDat->LastTime.Mon = (uchar)syukei->OldTime.Mon;							// ������
	p_NtDat->LastTime.Day = (uchar)syukei->OldTime.Day;							// ������
	p_NtDat->LastTime.Hour = (uchar)syukei->OldTime.Hour;						// ������
	p_NtDat->LastTime.Min = (uchar)syukei->OldTime.Min;							// ������
	p_NtDat->LastTime.Sec = 0;													// �����b
	p_NtDat->SettleNum							= syukei->Seisan_Tcnt;			// �����Z��
	p_NtDat->Kakeuri							= syukei->Kakeuri_Tryo;			// ���|���z
	p_NtDat->Cash								= syukei->Genuri_Tryo;			// ����������z
	p_NtDat->Uriage								= syukei->Uri_Tryo;				// ������z
	p_NtDat->Tax								= syukei->Tax_Tryo;				// ������Ŋz
	p_NtDat->Excluded							= 0;							// ������ΏۊO���z
	p_NtDat->Charge								= syukei->Turi_modosi_ryo;		// �ޑK���ߊz
	p_NtDat->CoinTotalNum						= (ushort)syukei->Ckinko_goukei_cnt;	// �R�C�����ɍ��v��
	p_NtDat->NoteTotalNum						= (ushort)syukei->Skinko_goukei_cnt;	// �������ɍ��v��
	p_NtDat->CyclicCoinTotalNum					= (ushort)syukei->Junkan_goukei_cnt;	// �z�R�C�����v��
	p_NtDat->NoteOutTotalNum					= (ushort)syukei->Siheih_goukei_cnt;	// �������o�@���v��
	p_NtDat->SettleNumServiceTime				= syukei->In_svst_seisan;		// �T�[�r�X�^�C�������Z��
	p_NtDat->Shortage.Num						= syukei->Harai_husoku_cnt;		// ���o�s����
	p_NtDat->Shortage.Amount					= syukei->Harai_husoku_ryo;		// ���o�s�����z
	p_NtDat->Cancel.Num							= syukei->Seisan_chusi_cnt;		// ���Z���~��
	p_NtDat->Cancel.Amount						= syukei->Seisan_chusi_ryo;		// ���Z���~���z
	p_NtDat->AntiPassOffSettle					= syukei->Apass_off_seisan;		// �A���`�p�XOFF���Z��
	p_NtDat->ReceiptIssue						= syukei->Ryosyuu_pri_cnt;		// �̎��ؔ��s����
	p_NtDat->WarrantIssue						= syukei->Azukari_pri_cnt;		// �a��ؔ��s����
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	p_NtDat->AllSystem.CarOutIllegal.Num		= syukei->Husei_out_Tcnt;		// �S���u  �s���o�ɉ�
//	p_NtDat->AllSystem.CarOutIllegal.Amount		= syukei->Husei_out_Tryo;		//                 ���z
//	p_NtDat->AllSystem.CarOutForce.Num			= syukei->Kyousei_out_Tcnt;		//         �����o�ɉ�
//	p_NtDat->AllSystem.CarOutForce.Amount		= syukei->Kyousei_out_Tryo;		//                 ���z
//	p_NtDat->AllSystem.AcceptTicket				= syukei->Uketuke_pri_Tcnt;		//         ��t�����s��
//	p_NtDat->AllSystem.ModifySettle.Num			= syukei->Syuusei_seisan_Tcnt;	// �C�����Z��
//	p_NtDat->AllSystem.ModifySettle.Amount		= syukei->Syuusei_seisan_Tryo;	//         ���z
//	p_NtDat->CarInTotal							= syukei->In_car_Tcnt;			// �����ɑ䐔
//	p_NtDat->CarOutTotal						= syukei->Out_car_Tcnt;			// ���o�ɑ䐔
//	p_NtDat->CarIn1								= syukei->In_car_cnt[0];		// ����1���ɑ䐔
//	p_NtDat->CarOut1							= syukei->Out_car_cnt[0];		// �o��1�o�ɑ䐔
//	p_NtDat->CarIn2								= syukei->In_car_cnt[1];		// ����2���ɑ䐔
//	p_NtDat->CarOut2							= syukei->Out_car_cnt[1];		// �o��2�o�ɑ䐔
//	p_NtDat->CarIn3								= syukei->In_car_cnt[2];		// ����3���ɑ䐔
//	p_NtDat->CarOut3							= syukei->Out_car_cnt[2];		// �o��3�o�ɑ䐔
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
// �����������ł̖������͊�{�ް��ɍ��ڂ�V�݂��đ��M����
	p_NtDat->MiyoCount							= syukei->Syuusei_seisan_Mcnt;	// ��������
	p_NtDat->MiroMoney							= syukei->Syuusei_seisan_Mryo;	// �������z
	p_NtDat->LagExtensionCnt					= syukei->Lag_extension_cnt;	// ���O�^�C��������
	p_NtDat->SaleParkingNo						= (ulong)CPrmSS[S_SYS][1];		// ����撓�ԏꇂ(0�`999999   �������p�i�W���ł͊�{���ԏꇂ���Z�b�g�j
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
//	p_NtDat->FurikaeCnt							= syukei->Furikae_seisan_cnt2;	// �U�։�
//	p_NtDat->FurikaeTotal						= syukei->Furikae_seisan_ryo2;	// �U�֊z
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������
	p_NtDat->RemoteCnt							= syukei->Remote_seisan_cnt;	// ���u���Z��
	p_NtDat->RemoteTotal						= syukei->Remote_seisan_ryo;	// ���u���Z�z
	// ���K�֘A���
	p_NtDat->Total					= syukei->Kinko_Tryo;						// ���ɑ������z
	p_NtDat->NoteTotal				= syukei->Note_Tryo;						// �������ɑ������z
	p_NtDat->CoinTotal				= syukei->Coin_Tryo;						// �R�C�����ɑ������z
	for (i = 0; i < COIN_SYU_CNT; i++) {										// �R�C��1�`4
		p_NtDat->Coin[i].Kind		= c_coin[i];								//        ����
		p_NtDat->Coin[i].Num		= (ushort)syukei->Coin_cnt[i];				//        ����
	}
	p_NtDat->Note[0].Kind			= 1000;										// ����1  ����
	p_NtDat->Note[0].Num2			= (ushort)syukei->Note_cnt[0];				//        ��������

// GG129000(S) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
//	ret = sizeof( DATA_KIND_42_r13 ); 
	ret = sizeof( DATA_KIND_42 ); 
// GG129000(E) M.Fujikawa 2023/09/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@�W�v�f�[�^�̃t�H�[�}�b�gRev.No��Rev.15�ɂȂ��Ă��Ȃ��@�s�#7132
	return ret;
}
// �d�l�ύX(E) K.Onodera 2016/11/04 �W�v��{�f�[�^�t�H�[�}�b�g�Ή�
// �s��C��(S) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
void SetVehicleCountDate( ulong val ){
	VehicleCountDateTime = val;
}
ulong GetVehicleCountDate( void ){
	return VehicleCountDateTime;
}
void AddVehicleCountSeqNo( void ){
	VehicleCountSeqNo++;
}
void ClrVehicleCountSeqNo( void ){
	VehicleCountSeqNo = 0;
}
ushort GetVehicleCountSeqNo( void ){
	return VehicleCountSeqNo;
}
// �s��C��(E) K.Onodera 2016/11/30 #1586 �U�֌����Z�f�[�^�̎���f�[�^�ŁA�䐔�Ǘ��ǔԂ̔N�������������M���̔N���������ɂȂ��Ă���
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//[]----------------------------------------------------------------------[]
///	@brief			�������ԏ�񃍃O�f�[�^�쐬(LOG�ɕۑ�����`��)
//[]----------------------------------------------------------------------[]
///	@param[in]		pr_lokno : ���������p���Ԉʒu�ԍ� 0�̏ꍇ�͑S����(�Ԏ��Ȃ�)
///	@param[in]		time	: ���o����(�ݒ�l) �S�����̏ꍇ�u0�v���Z�b�g
///	@param[in]		knd	: 0=���� 1=���� 2=�S����
///	@param[in]		knd2: 0=�w��Ȃ�(��������) 1=���Z 2=�����^�s�� (knd��1=�����̎��̂ݗL��)
///	@return			woid	: 
///	@author			A.iiizumi
///	@attention		
///	@note			�S�����̏ꍇ�͌��o����1/���o����2�͔��肵�Ȃ�(�s�v�Ȃ���)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2018/09/03<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
void	Make_Log_LongParking_Pweb( unsigned long pr_lokno, unsigned short time, uchar knd, uchar knd2)
{
	ushort	lkno;

	memset( &LongParking_data_Pweb, 0, sizeof( LongParking_data_Pweb ) );

	LongParking_data_Pweb.Knd = knd;		// ����/����/�S����
	LongParking_data_Pweb.Ck_Time = time;	// �������Ԍ��o����
	LongParking_data_Pweb.LockNo = pr_lokno;// ���������p���Ԉʒu�ԍ�
	LongParking_data_Pweb.SeqNo = GetNtDataSeqNo();// �V�[�P���V����No.�̐ݒ�

	if(knd == LONGPARK_LOG_RESET){// ����
		if(knd2 == LONGPARK_LOG_PAY){// ���Z�ɂ�����
			LongParking_data_Pweb.OutTime.Year = car_ot_f.year;	// ���Z����	�i�N�j���
			LongParking_data_Pweb.OutTime.Mon = car_ot_f.mon;	// 			�i���j���
			LongParking_data_Pweb.OutTime.Day = car_ot_f.day;	// 			�i���j���
			LongParking_data_Pweb.OutTime.Hour = car_ot_f.hour;	// 			�i���j���
			LongParking_data_Pweb.OutTime.Min = car_ot_f.min;	// 			�i���j���
			// �b�́u0�v�Œ�
		}else if(knd2 == LONGPARK_LOG_KYOUSEI_FUSEI){// ����/�s���o�ɂɂ�����
			LongParking_data_Pweb.OutTime.Year = CLK_REC.year;	// ��������	�i�N�j���
			LongParking_data_Pweb.OutTime.Mon = CLK_REC.mont;	// 			�i���j���
			LongParking_data_Pweb.OutTime.Day = CLK_REC.date;	// 			�i���j���
			LongParking_data_Pweb.OutTime.Hour = CLK_REC.hour;	// 			�i���j���
			LongParking_data_Pweb.OutTime.Min = CLK_REC.minu;	// 			�i���j���
			// �b�́u0�v�Œ�
		}
	}

	if(knd != LONGPARK_LOG_ALL_RESET){// �S�����ȊO
		// �S�����ȊO�͓��Ɏ������Z�b�g
		lkno = pr_lokno - 1;
		LongParking_data_Pweb.InTime.Year = FLAPDT.flp_data[lkno].year;		// ���ɔN
		LongParking_data_Pweb.InTime.Mon = FLAPDT.flp_data[lkno].mont;		// ���Ɍ�
		LongParking_data_Pweb.InTime.Day = FLAPDT.flp_data[lkno].date;		// ���ɓ�
		LongParking_data_Pweb.InTime.Hour = FLAPDT.flp_data[lkno].hour;		// ���Ɏ�
		LongParking_data_Pweb.InTime.Min = FLAPDT.flp_data[lkno].minu;		// ���ɕ�
	}

	// �����N�����������œ����
	LongParking_data_Pweb.ProcDate.Year	= CLK_REC.year;				// �N
	LongParking_data_Pweb.ProcDate.Mon	= CLK_REC.mont;				// ��
	LongParking_data_Pweb.ProcDate.Day	= CLK_REC.date;				// ��
	LongParking_data_Pweb.ProcDate.Hour	= CLK_REC.hour;				// ��
	LongParking_data_Pweb.ProcDate.Min	= CLK_REC.minu;				// ��
	LongParking_data_Pweb.ProcDate.Sec	= (ushort)CLK_REC.seco;		// �b

}
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//[]----------------------------------------------------------------------[]
///	@brief			�������ԏ��f�[�^(�f�[�^���61)�ҏW����
//[]----------------------------------------------------------------------[]
///	@param[in]		p_RcptDat : ���O������o�����������ԏ��f�[�^�̃|�C���^
///	@param[out]		p_NtDat   : ���Ƀf�[�^(DATA_KIND_20�^)�ւ̃|�C���^  
///	@return			ret       : ���Ƀf�[�^�̃f�[�^�T�C�Y(�V�X�e��ID�`) 
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2018/09/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2018 AMANO Corp.---[]
unsigned short	NTNET_Edit_Data61(	LongPark_log_Pweb 	*p_RcptDat,		// �������ԏ��log�ް�(IN)
									DATA_KIND_61_r10 	*p_NtDat )		// �������ԏ��NT-NET(OUT)
{
	unsigned short ret;
	unsigned long data;
	memset( p_NtDat, 0, sizeof( DATA_KIND_61_r10 ) );
	NTNET_Edit_BasicData( 61, 0,  p_RcptDat->SeqNo, &p_NtDat->DataBasic );	// ��{�f�[�^�쐬

	p_NtDat->dtc = p_RcptDat->Knd;							// ����/����/�S����

	p_NtDat->DataBasic.Year = (uchar)(p_RcptDat->ProcDate.Year % 100);	// �����N
	p_NtDat->DataBasic.Mon  = p_RcptDat->ProcDate.Mon;					// ������
	p_NtDat->DataBasic.Day  = p_RcptDat->ProcDate.Day;					// ������
	p_NtDat->DataBasic.Hour = p_RcptDat->ProcDate.Hour;					// ������
	p_NtDat->DataBasic.Min  = p_RcptDat->ProcDate.Min;					// ������
	p_NtDat->DataBasic.Sec  = p_RcptDat->ProcDate.Sec;					// �����b

	p_NtDat->FmtRev = 10;									// �t�H�[�}�b�gRev.��

	p_NtDat->InTime.Year = (uchar)(p_RcptDat->InTime.Year % 100);		// ���ɔN
	p_NtDat->InTime.Mon  = p_RcptDat->InTime.Mon;						// ���Ɍ�
	p_NtDat->InTime.Day  = p_RcptDat->InTime.Day;						// ���ɓ�
	p_NtDat->InTime.Hour = p_RcptDat->InTime.Hour;						// ���Ɏ�
	p_NtDat->InTime.Min  = p_RcptDat->InTime.Min;						// ���ɕ�
	// �b��0

	p_NtDat->OutTime.Year = (uchar)(p_RcptDat->OutTime.Year % 100);	// ���Z(�o��)�����i�N�j���
	p_NtDat->OutTime.Mon  = p_RcptDat->OutTime.Mon;						// �i���j
	p_NtDat->OutTime.Day  = p_RcptDat->OutTime.Day;						// �i���j
	p_NtDat->OutTime.Hour = p_RcptDat->OutTime.Hour;					// �i���j
	p_NtDat->OutTime.Min  = p_RcptDat->OutTime.Min;						// �i���j
	// �b��0

	p_NtDat->Ck_Time = (ulong)(p_RcptDat->Ck_Time * 60);		// �������Ԍ��o����(��)

	if(p_RcptDat->Knd != LONGPARK_LOG_ALL_RESET){				// �S�����ȊO�̓Z�b�g����
		// ���ɔ}�̂̐ݒ�
		p_NtDat->Media[0].MediaKind = 51;						// ���(�}��) ���b�N�E�t���b�v
		// �@�B��
		data = (ulong)CPrmSS[S_PAY][2];
		intoasl(p_NtDat->Media[0].MediaCardNo, data, 3);		// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
		// ���
		data = (ulong)LockInfo[p_RcptDat->LockNo-1].area;
		intoasl(&p_NtDat->Media[0].MediaCardNo[3], data, 2);	// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
		// �Ԏ��V�[�P���V����No
		data = 0;// 0�Œ�
		intoasl(&p_NtDat->Media[0].MediaCardNo[5], data, 2);	// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
		// �Ԏ�No
		data = LockInfo[p_RcptDat->LockNo-1].posi;
		intoasl(&p_NtDat->Media[0].MediaCardNo[7], data, 4);	// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
	}

	ret = sizeof( DATA_KIND_61_r10 );
	return ret;
}
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
