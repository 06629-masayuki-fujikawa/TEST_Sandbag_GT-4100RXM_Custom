//[]----------------------------------------------------------------------[]
///	@file		logctrl2.c
///	@brief		LOG�֘A�֐�2
/// @date		2012/05/08
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// NOTE:���O�f�[�^�o�^�֐��͒╜�d�����ɂăR���p�C���̍œK��������������Ɛ����
//      ���삵�Ȃ��̂ł��̃t�@�C���̃R���p�C���œK����}�~����
#include	<string.h>
#include	"system.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"cnm_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"LKmain.h"
#include	"ntnet.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"flp_def.h"
#include	"LKcom.h"
#include	"ntnet_def.h"
#include	"fla_def.h"
#include	"rauconstant.h"
extern ushort	Ope_Log_GetLogMaxCount( short id );
//	���L�ް��͔񏉊����ر�ɂ͂���Ȃ� ---> 36_0048���ύX����Ēʏ�N���������̈�
ushort		nearfullMaxReadSTS = 0;		// 0=init, 1=readREQ, 2=readDone

const ushort LogDatMax[][2] = {
	{sizeof(Receipt_data), RECODE_SIZE},	// eLOG_PAYMENT
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	{sizeof(enter_log), RECODE_SIZE},		// eLOG_ENTER
	{sizeof(RTPay_log), RECODE_SIZE},		// eLOG_RTPAY
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
	{sizeof(SYUKEI), RECODE_SIZE2},			// eLOG_TTOTAL
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	{sizeof(LCKTTL_LOG), RECODE_SIZE2},		// eLOG_LCKTTL
	{sizeof(RTReceipt_log), RECODE_SIZE2},	// eLOG_RTRECEIPT
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	{sizeof(Err_log), RECODE_SIZE},			// eLOG_ERROR
	{sizeof(Arm_log), RECODE_SIZE},			// eLOG_ALARM
	{sizeof(Ope_log), RECODE_SIZE},			// eLOG_OPERATE
	{sizeof(Mon_log), RECODE_SIZE},			// eLOG_MONITOR
	{sizeof(flp_log), RECODE_SIZE},			// eLOG_ABNORMAL
	{sizeof(TURI_KAN), RECODE_SIZE},		// eLOG_MONEYMANAGE
	{sizeof(ParkCar_log), RECODE_SIZE},		// eLOG_PARKING
	{sizeof(NGLOG_DATA), RECODE_SIZE},		// eLOG_NGLOG
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//	{sizeof(IoLog_Data), RECODE_SIZE},		// eLOG_IOLOG
	{sizeof(DC_QR_log), RECODE_SIZE},		// eLOG_DC_QR
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
	{sizeof(meisai_log), RECODE_SIZE},		// eLOG_CREUSE
	{sizeof(meisai_log), RECODE_SIZE},		// eLOG_iDUSE
	{sizeof(meisai_log), RECODE_SIZE},		// eLOG_HOJIN_USE
	{sizeof(t_Change_data), RECODE_SIZE},	// eLOG_REMOTE_SET
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//	{sizeof(LongPark_log), RECODE_SIZE},	// eLOG_LONGPARK
	{sizeof(LongPark_log_Pweb), RECODE_SIZE},// eLOG_LONGPARK_PWEB
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//	{sizeof(RismEvent_log), RECODE_SIZE},	// eLOG_RISMEVENT
	{sizeof(DC_LANE_log), RECODE_SIZE},		// eLOG_DC_LANE
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	{sizeof(SYUKEI), RECODE_SIZE2},			// eLOG_GTTOTAL
	{sizeof(Rmon_log), RECODE_SIZE},		// eLOG_REMOTE_MONITOR
	{sizeof(COIN_SYU), RECODE_SIZE2},		// eLOG_COINBOX(ram�̂�)
	{sizeof(NOTE_SYU), RECODE_SIZE2},		// eLOG_NOTEBOX(ram�̂�)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	{sizeof(edy_arm_log), RECODE_SIZE2},	// eLOG_EDYARM(ram�̂�)
//	{sizeof(edy_shime_log), RECODE_SIZE2},	// eLOG_EDYSHIME(ram�̂�)
	{0, RECODE_SIZE2},						// eLOG_EDYARM(ram�̂�)
	{0, RECODE_SIZE2},						// eLOG_EDYSHIME(ram�̂�)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	{sizeof(Pon_log), RECODE_SIZE2},		// eLOG_POWERON(ram�̂�)
	{sizeof(TURI_KAN), RECODE_SIZE2},		// eLOG_MNYMNG_SRAM(ram�̂�)
	{0, 0}									// terminator
};

//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�o�^
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG���
///	@param[in]	dat		: LOG�f�[�^
///	@param[in]	strage	: SRAM�t���O
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 2012/05/08 A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Log_Write(short Lno, void *dat, BOOL strage)
{
	int		i;
	ushort	cnt = LOG_SECORNUM(Lno);
	struct log_record	*lp = LOG_DAT+Lno;
	struct	log_record	*bp;
	ushort	wcnt;
	short	kind = 0;
// MH322914 (s) kasiyama 2016/07/13 ���O�ۑ��ł��Ȃ���Ԃ����P[���ʃo�ONo.1225](MH341106)
	ulong	ul_Lno = Lno;
	uchar	err_log[sizeof(Err_log)];
// MH322914 (e) kasiyama 2016/07/13 ���O�ۑ��ł��Ȃ���Ԃ����P[���ʃo�ONo.1225](MH341106)

	if(strage){								// Flash�Ƀf�[�^������
		if( lp->count[lp->kind] >= cnt ){	// ���ꂩ�珑������SRAM�̈�̌���MAX
// MH322914 (s) kasiyama 2016/07/13 ���O�ۑ��ł��Ȃ���Ԃ����P[���ʃo�ONo.1225](MH341106)
//			return;
			if( FLT_Check_LogWriteReq(Lno) == TRUE){ // FROM�������ݒ�
				FLT_Force_ResetLogWriteReq(Lno);// �������ݗv����Ԃ������I�Ƀ��Z�b�g����
			}
			for(i=0; i<eLOG_TARGET_MAX; i++){
				if( lp->unread[i] > lp->count[lp->kind] ) {
					lp->unread[i] -= lp->count[lp->kind];
				} else {
					lp->unread[i] = 0;
				}
			}
			lp->count[lp->kind] = 0; // SRAM�f�[�^��ԃN���A
			lp->wtp = 0;
			if(Lno == eLOG_ERROR) {
				// �G���[���O�������ݒ��ɃG���[���O���������ނ��߃G���[���O��ޔ�����
				memcpy(err_log, dat, sizeof(Err_log));
			}
			err_chk2(ERRMDL_MAIN, ERR_MAIN_SRAM_LOG_BUFFER_FULL, 2, 2, 0, &ul_Lno);	// SRAM���O�o�b�t�@�t��
			if(Lno == eLOG_ERROR) {
				memcpy(dat, err_log, sizeof(Err_log));
			}
// MH322914 (e) kasiyama 2016/07/13 ���O�ۑ��ł��Ȃ���Ԃ����P[���ʃo�ONo.1225](MH341106)
		}
	}
	log_bakup.Lno = Lno;
	log_bakup.strage = strage;
	log_bakup.dat_p = dat;
	log_bakup.stat_kind = lp->kind;
	log_bakup.stat_wtp = lp->wtp;
	log_bakup.stat_count = lp->count[lp->kind];
	for(i=0; i<eLOG_TARGET_MAX; i++){
		log_bakup.f_unread[i] = lp->f_unread[i];
	}
	log_bakup.f_recover = 1;// �����J�n
	if(log_bakup.Lno == eLOG_TTOTAL){// T�W�v
		if(ac_flg.syusyu == 14){
			ac_flg.syusyu = 15;// �s���v���O�o�^��t
		}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	}else if(log_bakup.Lno == eLOG_LCKTTL){// �Ԏ����W�v
//		if(ac_flg.syusyu == 15){
//			ac_flg.syusyu = 19;// �Ԏ����W�v���O(31�Ԏ��ȍ~)�o�^��t
//		}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	}else if(log_bakup.Lno == eLOG_GTTOTAL){// GT�W�v
		if(ac_flg.syusyu == 24){
			ac_flg.syusyu = 25;// �f�s���v���O�o�^��t
		}
	}else if(log_bakup.Lno == eLOG_COINBOX){// �R�C�����ɏW�v���O
		if(ac_flg.syusyu == 34){
			ac_flg.syusyu = 35;// �R�C�����ɏW�v���O��t
		}
	}else if(log_bakup.Lno == eLOG_MONEYMANAGE){// ���K�Ǘ����O(NT-NET�f�[�^�p)
		if(ac_flg.syusyu == 35){// �R�C�����ɏW�v��
			ac_flg.syusyu = 36;// ���K�Ǘ����O���O��t
		}else if(ac_flg.syusyu == 45){// �������ɏW�v��
			ac_flg.syusyu = 46;// ���K�Ǘ����O���O��t
		}else if(ac_flg.syusyu == 52){// ���K�Ǘ��W�v��
			ac_flg.syusyu = 53;// ���K�Ǘ����O���O��t
		}else if(ac_flg.cycl_fg == 54){// ���Z���~������
			ac_flg.cycl_fg = 0;
		}else if(ac_flg.cycl_fg == 17){// ���Z������(17:�ʐ��Z�ް��o�^����)
			ac_flg.cycl_fg = 0;
		}else if(ac_flg.cycl_fg == 27){// ���Z���~������(27:���Z���~�ް��o�^����)
			ac_flg.cycl_fg = 0;
		}
	}else if(log_bakup.Lno == eLOG_NOTEBOX){// �������ɏW�v���O
		if(ac_flg.syusyu == 44){
			ac_flg.syusyu = 45;// �������ɏW�v���O��t
		}
	}else if(log_bakup.Lno == eLOG_MNYMNG_SRAM){// ���K�Ǘ����O(SRAM)
		if(ac_flg.syusyu == 53){
			ac_flg.syusyu = 54;// ���K�Ǘ����O(SRAM)��t
		}
	}else if(log_bakup.Lno == eLOG_ABNORMAL){// �s���E�����o�Ƀ��O
		if(ac_flg.cycl_fg == 81){
			ac_flg.cycl_fg = 34;// �s���E�����o�Ƀ��O��t
		}
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	}else if((log_bakup.Lno == eLOG_CREUSE)||(log_bakup.Lno == eLOG_iDUSE)){// �N���W�b�g���Z
//		if(ac_flg.cycl_fg == 80){// �N���W�b�g���p���דo�^�J�n
//			ac_flg.cycl_fg = 17;// ���Z������(17:�ʐ��Z�ް��o�^����)
//		}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
	}else if(log_bakup.Lno == eLOG_PAYMENT){// ���Z���O
		if(ac_flg.cycl_fg == 33){// �s���E�����o�ɏ��o�^
			ac_flg.cycl_fg = 81;// �s���E�����o�Ƀf�[�^(NT-NET���Z�f�[�^�p)
		}else if(ac_flg.cycl_fg == 43){// �t���b�v�㏸�A���b�N�^�C�}���o��
			ac_flg.cycl_fg = 45;// �t���b�v�㏸�A���b�N�^�C�}���o��(NT-NET���Z�f�[�^�p)
		} else {
			if(ac_flg.cycl_fg == 15){// ���Z������
				ac_flg.cycl_fg = 16;// ���Z���O��t
			}else if(ac_flg.cycl_fg == 25){// ���Z���~���@
				// cyu_syuu()���̔���ɂ��Z�b�g����t���O���ς��
				if( ryo_buf.ryo_flg >= 2 ){// ����g�p
					ac_flg.cycl_fg = 26;// ���Z���O��t
				}else{
					ac_flg.cycl_fg = 28;// ���Z���O��t
				}
			}else if(ac_flg.cycl_fg == 52){// ���Z���~���A
				// toty_syu()���̔���ɂ��Z�b�g����t���O���ς��
				if( ryo_buf.ryo_flg >= 2 ){// ����g�p
					ac_flg.cycl_fg = 53;// ���Z���O��t
				}else{
					ac_flg.cycl_fg = 55;// ���Z���O��t
				}
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
			}else if(ac_flg.cycl_fg == 57){		// ���ϐ��Z���~��
				ac_flg.cycl_fg = 58;			// ���Z���O��t
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
			}
		}
	}
	if( LogDatMax[Lno][0] > RECODE_SIZE ){			// SRAM��1�ʂ������ĂȂ��H
		// �W�v�f�[�^�Ȃ�
		lp->kind = kind;// �K��0�ɂ���(kind = 0���)
		log_bakup.f_recover = 10;// �W�v�f�[�^��t
		while( AppServ_IsLogFlashWriting(Lno) ){	// �����ݏI���܂őҍ��킹
			taskchg( IDLETSKNO );
		}
	}else{
		kind = lp->kind;
		log_bakup.f_recover = 20;// ���̑��f�[�^��t
	}

	memcpy(lp->dat.s1[kind]+lp->wtp, dat, LogDatMax[Lno][0]);
	if( strage == 0){// RAM�݂̂ŊǗ����郍�O�̏ꍇ
		for(i=0; i<eLOG_TARGET_MAX; i++){
			if(lp->f_unread[i] < cnt){
				lp->f_unread[i]++;// �f�[�^�ǉ���薢�ǃJ�E���^���Z
			}
			else {
				// SRAM�݂̂̏ꍇ��f_unread���ő吔�̏�Ԃŏ������܂ꂽ�ꍇ�̓o�b�t�@�t��
				lp->writeFullFlag[i] = 1;	// �f�[�^�t���t���OON
			}
		}
	}
	log_bakup.f_recover = 2;// RAM�]������
	lp->count[kind]++;
	log_bakup.f_recover = 3;// RAM�J�E���^�X�V

	for(i=0; i<eLOG_TARGET_MAX; i++){
		lp->writeLogFlag[i] = 1;			// ���O�����݃t���OON
	}

	if( lp->count[kind]<cnt ){
		lp->wtp += LogDatMax[Lno][0];
		log_bakup.f_recover = 4;// RAM���C�g�|�C���^�X�V(�f�[�^FULL�łȂ�)
	}
	else{	// full
		if( strage ){
			log_bakup.f_recover = 30;// RAM FULL�ɂ��FROM�������ݏ����J�n
			while( AppServ_IsLogFlashWriting(Lno) ){	// �U�Ȃ͂�
				taskchg( IDLETSKNO );
			}
			for(i=0; i<eLOG_TARGET_MAX; i++){
				lp->writeFullFlag[i] = 1;	// �f�[�^�t���t���OON
			}
			AppServ_LogFlashWriteReq(Lno);	// FlashROM�֏����݈˗�
			log_bakup.f_recover = 31;// FROM�������ݗv������
			if( LogDatMax[Lno][0]>RECODE_SIZE){
				while( AppServ_IsLogFlashWriting(Lno) ){	// �������ݒ�
					taskchg( IDLETSKNO );
				}
				lp->count[kind] = 0;
				lp->wtp = 0;
				log_bakup.f_recover = 311;// ���O�������ݑ҂����킹
			}
			else{
				lp->kind = (kind)? 0: 1;
				log_bakup.f_recover = 321;// SRAM�ʐ؂�ւ����{
				lp->count[lp->kind] = 0;
				lp->wtp = 0;
				log_bakup.f_recover = 323;// SRAM�ʃJ�E���^�X�V
			}
		}else{	// SRAM�̂�
			lp->count[kind] = cnt;									// RAM�ő吔�܂ŒB�����̂ŏ��MAX�ƂȂ�
			log_bakup.f_recover = 40;// RAM FULL�ɂ��SRAM�G���A�X�V(SRAM�݂̂ŊǗ����Ă��郍�O)
			lp->wtp += LogDatMax[Lno][0];							// ���C�g�|�C���^�����O�T�C�Y���i�߂�
			log_bakup.f_recover = 41;// RAM���C�g�|�C���^�X�V
			bp = (struct log_record *)( LogDatMax[Lno][0] * cnt );	// ���O�Ō���̃|�C���^�����߂�
			if( lp->wtp >= (ulong)bp ){								// ���C�g�|�C���^�����O�G���A���z����
				log_bakup.f_recover = 42;// RAM���C�g�|�C���^�I�[�o�[��茳�ɖ߂��J�n
				lp->wtp = 0;										// ���C�g�|�C���^��擪�ɖ߂�
				log_bakup.f_recover = 43;// RAM���C�g�|�C���^�I�[�o�[��茳�ɖ߂��I��
			}else{
				log_bakup.f_recover = 44;// RAM���C�g�|�C���^�I�[�o�[��茳�ɖ߂�(�ێ����O�����ő働�O��)
				wcnt = lp->wtp / LogDatMax[Lno][0];					// �擪����̏����ݐ������߂�
				for(i=0; i<eLOG_TARGET_MAX; i++){
					if( lp->unread[i] < wcnt ){						// ���ǃ��R�[�h�ԍ��𒴂���(���ǃ��R�[�h���㏑�����ꂽ)
						if(lp->f_unread[i] >= cnt){					// ���ǃf�[�^�����t�̏ꍇ�X�V
							lp->unread[i] = wcnt;					// ���ǃ��R�[�h�ԍ����ŌẪ��O�Ƃ���
						}
					}
				}
			}
		}
	}
	Log_clear_log_bakupflag();// �����I�����J�o���p�f�[�^�N���A
}
//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�o�^���d����
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/05/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Log_Write_Pon(void)
{
	int		i;
	ushort	cnt;
	struct log_record	*lp;
	struct	log_record	*bp;
	ushort	wcnt;
	short	kind = 0;
	Receipt_data	*pay_p;

	if(log_bakup.f_recover == 0){// ���J�o���s�v
		Log_clear_log_bakupflag();
		return;
	}
	lp = &LOG_DAT[log_bakup.Lno];
	cnt = LOG_SECORNUM(log_bakup.Lno);
	// �ȉ��ꕶ�̏�����Log_Write()�́u�����J�n�v�̕����ōs�Ȃ����̂ł��邪�A����ȍ~�̏���
	// �ł��g�p����̂ŕK���Z�b�g����K�v������B
	kind = log_bakup.stat_kind;
	if(log_bakup.Lno == eLOG_TTOTAL){// T�W�v
		if(ac_flg.syusyu == 14){
			ac_flg.syusyu = 15;// �s���v���O�o�^��t
		}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	}else if(log_bakup.Lno == eLOG_LCKTTL){// �Ԏ����W�v
//		if(ac_flg.syusyu == 15){
//			ac_flg.syusyu = 19;// �Ԏ����W�v���O(31�Ԏ��ȍ~)�o�^��t
//		}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	}else if(log_bakup.Lno == eLOG_GTTOTAL){// GT�W�v
		if(ac_flg.syusyu == 24){
			ac_flg.syusyu = 25;// �f�s���v���O�o�^��t
		}
	}else if(log_bakup.Lno == eLOG_COINBOX){// �R�C�����ɏW�v���O
		if(ac_flg.syusyu == 34){
			ac_flg.syusyu = 35;// �R�C�����ɏW�v���O��t
		}
	}else if(log_bakup.Lno == eLOG_MONEYMANAGE){// ���K�Ǘ����O(NT-NET�f�[�^�p)
		if(ac_flg.syusyu == 35){// �R�C�����ɏW�v��
			ac_flg.syusyu = 36;// ���K�Ǘ����O���O��t
		}else if(ac_flg.syusyu == 45){// �������ɏW�v��
			ac_flg.syusyu = 46;// ���K�Ǘ����O���O��t
		}else if(ac_flg.syusyu == 52){// ���K�Ǘ��W�v��
			ac_flg.syusyu = 53;// ���K�Ǘ����O���O��t
		}else if(ac_flg.cycl_fg == 54){// ���Z���~������
			ac_flg.cycl_fg = 0;
		}else if(ac_flg.cycl_fg == 17){// ���Z������(17:�ʐ��Z�ް��o�^����)
			ac_flg.cycl_fg = 0;
		}else if(ac_flg.cycl_fg == 27){// ���Z���~������(27:���Z���~�ް��o�^����)
			ac_flg.cycl_fg = 0;
		}
	}else if(log_bakup.Lno == eLOG_NOTEBOX){// �������ɏW�v���O
		if(ac_flg.syusyu == 44){
			ac_flg.syusyu = 45;// �������ɏW�v���O��t
		}
	}else if(log_bakup.Lno == eLOG_MNYMNG_SRAM){// ���K�Ǘ����O(SRAM)
		if(ac_flg.syusyu == 53){
			ac_flg.syusyu = 54;// ���K�Ǘ����O(SRAM)��t
		}
	}else if(log_bakup.Lno == eLOG_ABNORMAL){// �s���E�����o�Ƀ��O
		if(ac_flg.cycl_fg == 81){
			ac_flg.cycl_fg = 34;// �s���E�����o�Ƀ��O��t
		}
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	}else if((log_bakup.Lno == eLOG_CREUSE)||(log_bakup.Lno == eLOG_iDUSE)){// �N���W�b�g���Z
//		if(ac_flg.cycl_fg == 80){// �N���W�b�g���p���דo�^�J�n
//			ac_flg.cycl_fg = 17;// ���Z������(17:�ʐ��Z�ް��o�^����)
//		}
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
	}else if(log_bakup.Lno == eLOG_PAYMENT){// ���Z���O
		if(ac_flg.cycl_fg == 33){// �s���E�����o�ɏ��o�^
			ac_flg.cycl_fg = 81;// �s���E�����o�Ƀf�[�^(NT-NET���Z�f�[�^�p)
		}else if(ac_flg.cycl_fg == 43){// �t���b�v�㏸�A���b�N�^�C�}���o��
			ac_flg.cycl_fg = 45;// �t���b�v�㏸�A���b�N�^�C�}���o��(NT-NET���Z�f�[�^�p)
		} else {
			pay_p = (Receipt_data*)log_bakup.dat_p;
			// �����܂ł����PayData_set()�̂��Ȃ������ł��Ȃ��̂�
			// ���d�󎚂̂��߂̃t���O�󎚎�ʂP���i1:���d�j�Ƃ���
			pay_p->WFlag = 1;
			if(ac_flg.cycl_fg == 15){// ���Z������
				ac_flg.cycl_fg = 16;// ���Z���O��t
			}else if(ac_flg.cycl_fg == 25){// ���Z���~���@
				// cyu_syuu()���̔���ɂ��Z�b�g����t���O���ς��
				if( ryo_buf.ryo_flg >= 2 ){// ����g�p
					ac_flg.cycl_fg = 26;// ���Z���O��t
				}else{
					ac_flg.cycl_fg = 28;// ���Z���O��t
				}
			}else if(ac_flg.cycl_fg == 52){// ���Z���~���A
				// toty_syu()���̔���ɂ��Z�b�g����t���O���ς��
				if( ryo_buf.ryo_flg >= 2 ){// ����g�p
					ac_flg.cycl_fg = 53;// ���Z���O��t
				}else{
					ac_flg.cycl_fg = 55;// ���Z���O��t
				}
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
			}else if(ac_flg.cycl_fg == 57){		// ���ϐ��Z���~��
				ac_flg.cycl_fg = 58;			// ���Z���O��t
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
			}
		}
	}
	switch(log_bakup.f_recover){
		case 1:// �����J�n
			if( LogDatMax[log_bakup.Lno][0] > RECODE_SIZE ){// SRAM��1�ʂ������ĂȂ��H
				// �W�v�f�[�^�̂�
				log_bakup.stat_kind = 0;
				log_bakup.f_recover = 10;// �W�v�f�[�^��t
			}else{
				kind = log_bakup.stat_kind;
				log_bakup.f_recover = 20;// ���̑��f�[�^��t
			}
		case 10:// �W�v�f�[�^��t
		case 20:// ���̑��f�[�^��t
			memcpy(lp->dat.s1[kind]+lp->wtp, log_bakup.dat_p, LogDatMax[log_bakup.Lno][0]);
			if( log_bakup.strage == 0){// RAM�݂̂ŊǗ����郍�O�̏ꍇ
				for(i=0; i<eLOG_TARGET_MAX; i++){
					if(lp->f_unread[i] < cnt){
						if(log_bakup.f_unread[i] == lp->f_unread[i]){
							// �l�������Ȃ�J�E���g�A�b�v���O�Œ�d�����̂ōX�V����
							lp->f_unread[i]++;// �f�[�^�ǉ���薢�ǃJ�E���^���Z
						}
					}
					else {
						// SRAM�݂̂̏ꍇ��f_unread���ő吔�̏�Ԃŏ������܂ꂽ�ꍇ�̓o�b�t�@�t��
						lp->writeFullFlag[i] = 1;	// �f�[�^�t���t���OON
					}
				}
			}
			log_bakup.f_recover = 2;// RAM�]������
		case 2:// RAM�]������
			if(log_bakup.stat_count == lp->count[kind]){
				// �l�������Ȃ�J�E���g�A�b�v���O�Œ�d�����̂ōX�V����
				lp->count[kind]++;
			}
			log_bakup.f_recover = 3;// RAM�J�E���^�X�V
			break;
		case 3:// RAM�J�E���^�X�V
			goto point4;
		case 30:// RAM FULL�ɂ��FROM�������ݏ����J�n
			goto point5;
		case 31:// FROM�������ݗv������
			goto point6;
		case 321:// SRAM�ʐ؂�ւ����{
			goto point7;
		case 40:// RAM FULL�ɂ��SRAM�G���A�X�V(SRAM�݂̂ŊǗ����Ă��郍�O)
			goto point8;
		case 41:// RAM���C�g�|�C���^�X�V
			goto point9;
		case 42:// RAM���C�g�|�C���^�I�[�o�[��茳�ɖ߂��J�n
			goto point10;
		case 44:// RAM���C�g�|�C���^�I�[�o�[��茳�ɖ߂�(�ێ����O�����ő働�O��)
			goto point11;
		case 4:// RAM���C�g�|�C���^�X�V(�f�[�^FULL�łȂ�)
		case 311:// ���O�������ݑ҂����킹
		case 323:// SRAM�ʃJ�E���^�X�V
		case 43:// RAM���C�g�|�C���^�I�[�o�[��茳�ɖ߂��I��
		default:// ���J�o���s�v�Ƃ݂Ȃ�
			goto point_end;
	}
point4:
	for(i=0; i<eLOG_TARGET_MAX; i++){
		lp->writeLogFlag[i] = 1;				// ���O�����݃t���OON
	}
	if( lp->count[kind]<cnt ){
		if(log_bakup.stat_wtp == lp->wtp){
			// �l�������Ȃ�J�E���g�A�b�v���O�Œ�d�����̂ōX�V����
			lp->wtp += LogDatMax[log_bakup.Lno][0];
		}
		log_bakup.f_recover = 4;// RAM���C�g�|�C���^�X�V(�f�[�^FULL�łȂ�)
	}
	else{			// full
		if( log_bakup.strage ){
			log_bakup.f_recover = 30;// RAM FULL�ɂ��FROM�������ݏ����J�n
point5:
			if( FLT_Check_LogWriteReq(log_bakup.Lno) == FALSE){// ��d�O�̗v���͂Ȃ�
				for(i=0; i<eLOG_TARGET_MAX; i++){
					lp->writeFullFlag[i] = 1;	// �f�[�^�t���t���OON
				}
				// ��d�O��AppServ_LogFlashWriteReq()��FLT_WriteLog()���ł̗v�����������Ă��Ȃ�
				// ���ߍŏ�������Ȃ���
				FLT_Force_ResetLogWriteReq(log_bakup.Lno);// �������ݗv���O�ɒ�d�O�̏�Ԃ������I�Ƀ��Z�b�g����
				AppServ_LogFlashWriteReq(log_bakup.Lno);	// FlashROM�֏����݈˗�
				// �v�����������Ă���ꍇ�́A�ʂŎ��{����FROM�̃��J�o�[�����Ŏ��s�����
			}
			log_bakup.f_recover = 31;// FROM�������ݗv������
point6:
			if( LogDatMax[log_bakup.Lno][0]>RECODE_SIZE){
				// �ʏ폈���ł͏������݊����̑҂����킹�����Ă��邪�A���d�����ł�
				// AppServ_LogFlashWriteReq()���Ń^�X�N�؂�ւ����Ȃ��ŏ������ނ̂ŕs�v
				lp->count[kind] = 0;
				lp->wtp = 0;
				log_bakup.f_recover = 311;// ���O�������ݑ҂����킹
			}
			else{
				if(log_bakup.stat_kind == lp->kind){
					// �l�������Ȃ�X�V���O�Œ�d�����̂ōX�V����
					lp->kind = (kind)? 0: 1;
				}
				log_bakup.f_recover = 321;// SRAM�ʐ؂�ւ����{
point7:
				lp->count[lp->kind] = 0;
				lp->wtp = 0;
				log_bakup.f_recover = 323;// SRAM�ʃJ�E���^�X�V
			}
		}else{	// SRAM�̂�
			lp->count[kind] = cnt;									// RAM�ő吔�܂ŒB�����̂ŏ��MAX�ƂȂ�
			log_bakup.f_recover = 40;// RAM FULL�ɂ��SRAM�G���A�X�V(SRAM�݂̂ŊǗ����Ă��郍�O)
point8:
			if(log_bakup.stat_wtp == lp->wtp){
				// �l�������Ȃ�J�E���g�A�b�v���O�Œ�d�����̂ōX�V����
				lp->wtp += LogDatMax[log_bakup.Lno][0];// ���C�g�|�C���^�����O�T�C�Y���i�߂�
			}
			log_bakup.f_recover = 41;// RAM���C�g�|�C���^�X�V
point9:
			bp = (struct log_record *)( LogDatMax[log_bakup.Lno][0] * cnt );// ���O�Ō���̃|�C���^�����߂�
			if( lp->wtp >= (ulong)bp ){								// ���C�g�|�C���^�����O�G���A���z����
				log_bakup.f_recover = 42;// RAM���C�g�|�C���^�I�[�o�[��茳�ɖ߂��J�n
point10:
				lp->wtp = 0;										// ���C�g�|�C���^��擪�ɖ߂�
				log_bakup.f_recover = 43;// RAM���C�g�|�C���^�I�[�o�[��茳�ɖ߂��I��
			}else{
				log_bakup.f_recover = 44;// RAM���C�g�|�C���^�I�[�o�[��茳�ɖ߂�(�ێ����O�����ő働�O��)
point11:
				wcnt = lp->wtp / LogDatMax[log_bakup.Lno][0];		// �擪����̏����ݐ������߂�
				for(i=0; i<eLOG_TARGET_MAX; i++){
					if( lp->unread[i] < wcnt ){						// ���ǃ��R�[�h�ԍ��𒴂���(���ǃ��R�[�h���㏑�����ꂽ)
						if(lp->f_unread[i] >= cnt){					// ���ǃf�[�^�����t�̏ꍇ�X�V
							lp->unread[i] = wcnt;					// ���ǃ��R�[�h�ԍ����ŌẪ��O�Ƃ���
						}
					}
				}
			}
		}
	}
point_end:
	Log_clear_log_bakupflag();// �����I�����J�o���p�f�[�^�N���A
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�o�^���d����
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/05/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Log_clear_log_bakupflag(void)
{
	int i;
// LOG�f�[�^�o�^���d�����p�̃t���O�A�ޔ��f�[�^�N���A����
	log_bakup.f_recover = 0;// ��������
	log_bakup.Lno = 0;
	log_bakup.strage = 0;
	log_bakup.dat_p = 0;
	log_bakup.stat_kind = 0;
	log_bakup.stat_wtp = 0;
	log_bakup.stat_count = 0;
	for(i=0; i<eLOG_TARGET_MAX; i++){
		log_bakup.f_unread[i] = 0;
	}
}
