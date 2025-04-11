// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
//[]----------------------------------------------------------------------[]
///	@file	cal_cloud.c
///	@brief	�N���E�h�����v�Z����
///	@date	2022/06/02
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]

#include <string.h>

#include "system.h"
#include "strdef.h"
#include "flp_def.h"
#include "mem_def.h"
#include "ntnet_def.h"
#include "ope_def.h"
#include "rkn_def.h"
#include "prm_tbl.h"
#include "tbl_rkn.h"

#include "cal_cloud.h"


extern void lcdbm_notice_dsp(ePOP_DISP_KIND kind, uchar DispStatus);

static void pay_result_error_alarm_check(ushort shPayResult);
static void fee_init();
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
static void season_init();
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
static void discount_init();
static void ryo_buf_n_set();
static void ryo_buf_n_calc();
static void ryo_buf_n_get(uchar seasonFlg);
static void ryo_buf_calc();

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ���Z�J�n�����`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		ushort	: �w����t(�m�[�}���C�Y)
///	@return			ret		: ���Z�J�n�����`�F�b�N����<br>
///							  -1 = �w����t���O<br>
///							   0 = �w����t�Ɠ���<br>
///							   1 = �w����t����<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_pay_start_time_check(ushort ndatTarget)
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����
	struct clk_rec *pTime;		// ���Z�J�n����
	ushort	nminSwitch;			// ���t�ؑ֎���(�m�[�}���C�Y)

	// ���Z�J�n�������擾
	pTime = GetPayStartTime();

	// ���t�ؑ֎������擾
	nminSwitch = tnrmlz((short)0, (short)0, (short)pFeeCalc->DateSwitchHours, (short)pFeeCalc->DateSwitchMinute);

	// ���Z�J�n���������t�ؑ֎����ɒB���Ă��Ȃ��ꍇ�͎w����t���{�P��
	if ( pTime->nmin < nminSwitch ) {
		ndatTarget++;
	}

	// ���Z�J�n���t�Ǝw����t�̔�r
	if ( pTime->ndat < ndatTarget ) {
		return -1;	// �w����t���O
	}
	if ( pTime->ndat > ndatTarget ) {
		return 1;	// �w����t����
	}

	return 0;	// �w����t�Ɠ���
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ����������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void cal_cloud_init()
{
	// �����v�Z���[�h
	g_calcMode = (uchar)prm_get(COM_PRM, S_SYS, 79, 1, 1);
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ���ɏ��`�F�b�N(����)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ���ɏ��`�F�b�N����<br>
///							  0 = �����ݒ��<br>
///							  1 = ���Z�s��<br>
///							  2 = ���Z�ς�<br>
// GG124100(S) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
///							  3 = ���Z�ς�(���~)<br>
// GG124100(E) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_fee_check()
{
	stZaishaInfo_t *pZaisha = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo;	// �ݎԏ��
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����

	// �����v�Z���ʃG���[�A���[���`�F�b�N
	pay_result_error_alarm_check(pFeeCalc->shPayResult);

	// ���O�^�C�������Z�̃`�F�b�N
	FLAPDT.flp_data[(OPECTL.Pr_LokNo - 1)].lag_to_in.BIT.LAGIN = 0x00;
	switch( pFeeCalc->PaymentTimeType ) {
	case 0:		// �ʏ퐸�Z
	case 1:		// �T�[�r�X�^�C�������Z
		if ( (pZaisha->PaymentType == 1) || (pZaisha->PaymentType == 2) ) {	// ���Z/�Đ��Z
			if ( (pZaisha->dtPaymentDateTime.dtTimeYtoSec.byMonth > 0) &&
			   (pZaisha->dtPaymentDateTime.dtTimeYtoSec.byDay > 0) ) {	// ���Z��������
				FLAPDT.flp_data[(OPECTL.Pr_LokNo - 1)].lag_to_in.BIT.LAGIN = 0x01;
			}
		}
		break;
	case 2:		// ���O�^�C�������Z
		return 2;	// ���Z�ς�
	default:	// ���̑�
		return 1;	// ���Z�s��
	}

	// �������擾���ʂ̃`�F�b�N
	switch( pFeeCalc->CalcResult ) {
	case 0:		// ����
	case 2:		// �����Ȃ�
		break;
	case 1:		// �������s(�Z���^�[�����G���W���G���[)
	default:	// ���̑�
		return 1;	// ���Z�s��
	}

	// �����v�Z���ʂ̃`�F�b�N
	switch( pFeeCalc->shPayResult ) {
	case 0:		// ����
	case 1001:	// �����Ȃ�
		break;
// GG124100(S) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
	case 43:	// �����v�Z�s��(���y�C�^�C�����̍Đ��Z�͕s�ł�)
		return 3;	// ���Z�ς�(���~)
// GG124100(E) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6614 �����v�Z���ʂ��u43�v�̎��̃|�b�v�A�b�v�̓��e���s��
	default:	// ���̑�
		return 1;	// ���Z�s��
	}

	return 0;	// �����ݒ��
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z �����ݒ�
//[]----------------------------------------------------------------------[]
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// ///	@param[in]		void
///	@param[in]		uchar	: ����t���O<br>
///							  0 = ���ڈȍ~<br>
///							  1 = ����<br>
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
///	@return			ret		: �����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// short cal_cloud_fee_set()
short cal_cloud_fee_set(uchar firstFlg)
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
	uchar	f_in_coin = 0;
	ushort	in_coin[5];
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j

	// ����VL�J�[�h�f�[�^
	vl_now = V_CHM;	// ���Ԍ� ���Z�O

// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
	// �������
	syashu = pFeeCalc->FeeType;
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// 	// ����������
// 	fee_init();
	if ( firstFlg ) {	// ����
		// ����������
		fee_init();
	} else {			// ���ڈȍ~
// GG129000(S) H.Fujinaga 2023/01/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
		if ( PayInfoChange_StateCheck() == 1 ){
			// ���Z���ύX�f�[�^��M
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
			if (ryo_buf.nyukin) {
				// ���������o�b�N�A�b�v
				memcpy(in_coin, ryo_buf.in_coin, sizeof(ryo_buf.in_coin));
				f_in_coin = 1;
			}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
			// ����������
			fee_init();
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
			if (f_in_coin) {
				// �����������X�g�A
				memcpy(ryo_buf.in_coin, in_coin, sizeof(ryo_buf.in_coin));
			}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
		}else{
// GG129000(E) H.Fujinaga 2023/01/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
		// ���������
		season_init();

		// ����������
		discount_init();
// GG129000(S) H.Fujinaga 2023/01/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
		}
// GG129000(E) H.Fujinaga 2023/01/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
	}
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�

// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
	// �Ԕԏ��
	PayData.CarSearchFlg = vl_car_no.CarSearchFlg;			// �Ԕ�/�����������
	memcpy(PayData.CarSearchData, vl_car_no.CarSearchData,
		sizeof(PayData.CarSearchData));						// �Ԕ�/���������f�[�^
	memcpy(PayData.CarNumber, vl_car_no.CarNumber,
		sizeof(PayData.CarNumber));							// �Z���^�[�₢���킹���̎Ԕ�
	memcpy(PayData.CarDataID, vl_car_no.CarDataID,
		sizeof(PayData.CarDataID));							// �Ԕԃf�[�^ID(�⍇���Ɏg�p����ID)
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

#if 1	// �Q�l�Fec09(V_CHM)
	// �O�񗿋������t���O
	if ( ryo_buf.zenkai > 0 ) {
		PayData.PRTwari.BIT.RWARI = 1;	// �O�񗿋���������
	}
#endif	// �Q�l�Fec09(V_CHM)

#if 1	// �Q�l�Fet02(et40)
	// �����f�[�^�o�b�t�@�m�N���A
	memset(&ryo_buf_n, 0, sizeof(ryo_buf_n));

	// �T�[�r�X�^�C�������Z
	if ( pFeeCalc->PaymentTimeType == 1 ) {
		// �T�[�r�X�^�C�������Z�t���O
		ryo_buf.svs_tim = 1;	// �T�[�r�X�^�C�������Z����

		// �����z
		ryo_buf_n.ryo = 0;
	} else {
		// �����z
		ryo_buf_n.ryo = pFeeCalc->ParkingFee;	// ���ԗ���
	}

	// ����Ŋz
	ryo_buf_n.tax = 0;	// 0���Œ�

	// �����z
	ryo_buf_n.dis = ryo_buf.zenkai;	// �O��x���z(����������)
#endif	// �Q�l�Fet02(et40)

	// �����f�[�^�o�b�t�@�m�v�Z
	ryo_buf_n_calc();

// GG124100(S) R.Endo 2022/09/07 �Ԕԃ`�P�b�g���X3.0 #6565 �O��̎��z�������ԂōĐ��Z����ƁA�̎��؂̍��v���z���s���ɂȂ�
// #if 1	// �Q�l�Fet02(et43)
// 	// �O�񗿋������z���v
// 	ryo_buf.dis_fee += ryo_buf_n.dis;		// ���������l�������O��x���z
// 
// 	// ���������̍��v���z(�|�C���g�J�[�h�͏���)
// 	ryo_buf.fee_amount += ryo_buf.zenkai;	// ���������l�����Ȃ��O��x���z
// #endif	// �Q�l�Fet02(et43)
	// �O��x���z�͊����z�Ɋ܂߂邪�A����p�őO�񗿋������z���v���ɂ����Z����Ă����B
	// �ʏ헿���v�Z�ł͂����OnlineDiscount(ope/opesub.c)�ŃN���A���Ă������A
	// �N���E�h�����v�Z�ł͉��Z���̂��s��Ȃ��悤�ɂ���B
// GG124100(E) R.Endo 2022/09/07 �Ԕԃ`�P�b�g���X3.0 #6565 �O��̎��z�������ԂōĐ��Z����ƁA�̎��؂̍��v���z���s���ɂȂ�

	// �����f�[�^�o�b�t�@�m�擾
	ryo_buf_n_get(0);	// �ʏ�

	// �����f�[�^�o�b�t�@�v�Z
	ryo_buf_calc();

#if 1	// �Q�l�Fop_mod02
	// ���������T�u�F�Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
	in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);

	// ���Z���@
	if ( lcdbm_rsp_in_car_info_main.shubetsu == 1 ) {	// �����Z�o�ɐ��Z
		OpeNtnetAddedInfo.PayMethod = 13;	// ������Z
// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
	} else if ( lcdbm_rsp_in_car_info_main.shubetsu == 2 ) {	// ���u�������Z(���Z���ύX�f�[�^)
		OpeNtnetAddedInfo.PayMethod = 0;	// ���Ȃ����Z
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
	} else {											// ���O���Z
// GG129000(S) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
//		OpeNtnetAddedInfo.PayMethod = 0;	// ���Ȃ����Z
		if ( lcdbm_rsp_in_car_info_main.kind == 2 ) {
			// QR����
			OpeNtnetAddedInfo.PayMethod = 1;	// ���Ԍ����Z
		} else {
			// �ԔԌ���,��������
			OpeNtnetAddedInfo.PayMethod = 0;	// ���Ȃ����Z
		}
// GG129000(E) H.Fujinaga 2023/02/14 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ��j
	}

	// ���Z���[�h
	OpeNtnetAddedInfo.PayMode = 0;	// �������Z

	// �����敪
	switch ( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType ) {
	case 1:	// ���Z
	case 2:	// �Đ��Z
	case 4:	// �Đ��Z���~
		OpeNtnetAddedInfo.PayClass = 1;	// �Đ��Z
		break;
	default:
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
		OpeNtnetAddedInfo.PayClass = 0;	// ���Z
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
		break;
	}
#endif	// �Q�l�Fop_mod02

	return 0;	// ��������
}

// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ���u���Z(���Ɏ����w��) �����ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: �����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_fee_set_remote_time()
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����

	// ����VL�J�[�h�f�[�^
	vl_now = V_CHM;	// ���Ԍ� ���Z�O

	// �������
	syashu = pFeeCalc->FeeType;

	// ����������
	fee_init();

#if 1	// �Q�l�Fet02(et40)
	// �����f�[�^�o�b�t�@�m�N���A
	memset(&ryo_buf_n, 0, sizeof(ryo_buf_n));

	// �����z
	ryo_buf_n.ryo = pFeeCalc->ParkingFee;	// ���ԗ���

	// ����Ŋz
	ryo_buf_n.tax = 0;	// 0���Œ�
#endif	// �Q�l�Fet02(et40)

	// �����f�[�^�o�b�t�@�m�v�Z
	ryo_buf_n_calc();

	// �����f�[�^�o�b�t�@�m�擾
	ryo_buf_n_get(0);	// �ʏ�

	// �����f�[�^�o�b�t�@�v�Z
	ryo_buf_calc();

#if 1	// �Q�l�Fop_mod02
	// ���Z���@
	OpeNtnetAddedInfo.PayMethod = 0;	// ���Ȃ����Z

	// ���Z���[�h
	OpeNtnetAddedInfo.PayMode = 4;	// ���u���Z

	// ���u���Z���Ɏ����w��w�艞���ւ̐ݒ�p
	g_PipCtrl.stRemoteTime.RyoSyu = ryo_buf.syubet + 1;
	g_PipCtrl.stRemoteTime.Price = ryo_buf.tyu_ryo;
#endif	// �Q�l�Fop_mod02

	return 0;	// ��������
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ���u���Z(���z�w��) �����ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: �����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_fee_set_remote_fee()
{
	// ����VL�J�[�h�f�[�^
	vl_now = V_FUN;	// ����������

	// ����������
	fee_init();

#if 1	// �Q�l�Fec09(V_FUN)
	if ( (g_PipCtrl.stRemoteFee.RyoSyu >= 1) &&
		 (g_PipCtrl.stRemoteFee.RyoSyu <= 12) ) {	// �v���Ɏ�ʂ���
		syashu = (char)g_PipCtrl.stRemoteFee.RyoSyu;
	} else {
		syashu = (char)prm_get(COM_PRM, S_CEN, 40, 2, 1);
		if ( !syashu ) {
			syashu = 1;	// A��Œ�
		}
		g_PipCtrl.stRemoteFee.RyoSyu = syashu;
	}
#endif	// �Q�l�Fec09(V_FUN)

#if 1	// �Q�l�Fet02(et48)
	// �����f�[�^�o�b�t�@�m�N���A
	memset(&ryo_buf_n, 0, sizeof(ryo_buf_n));

	// �����z
	ryo_buf_n.ryo = g_PipCtrl.stRemoteFee.Price;	// ���ԗ���

	// ����Ŋz
	ryo_buf_n.tax = 0;	// 0���Œ�

	// ���������͌Œ藿���t���O
	fun_kti = ON;

	// �o�ɁE���Ɏ����s���t���O
	ryo_buf.fumei_fg = 2;	// ���Ɏ����s������

	// �s���t���O�ڍח��R
	ryo_buf.fumei_reason = fumei_LOST;	// �������Z����
#endif	// �Q�l�Fet02(et48)

	// �����f�[�^�o�b�t�@�m�v�Z
	ryo_buf_n_calc();

	// �����f�[�^�o�b�t�@�m�擾
	ryo_buf_n_get(0);	// �ʏ�

	// �����f�[�^�o�b�t�@�v�Z
	ryo_buf_calc();

#if 1	// �Q�l�Fop_mod02
	// ���Z���@
	if ( g_PipCtrl.stRemoteFee.Type == 1 ) {	// ������Z
		OpeNtnetAddedInfo.PayMethod = 13;	// ������Z
	} else {									// �o�ɐ��Z
		OpeNtnetAddedInfo.PayMethod = 0;	// ���Ȃ����Z
	}

	// ���Z���[�h
	OpeNtnetAddedInfo.PayMode = 4;	// ���u���Z
#endif	// �Q�l�Fop_mod02

	return 0;	// ��������
}
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ����`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ����`�F�b�N����<br>
///							  SEASON_CHK_OK = �G���[�Ȃ�<br>
///							  SEASON_CHK_INVALID_SEASONDATA = ��������G���[<br>
///							  SEASON_CHK_PRE_SALE_NG = �̔��O����G���[<br>
///							  SEASON_CHK_BEFORE_VALID = �����O�G���[<br>
///							  SEASON_CHK_AFTER_VALID = �����؂�G���[<br>
///							  SEASON_CHK_INVALID_PARKINGLOT = ���ԏꇂ�G���[<br>
///	@note	������A���[����񓙂��`�F�b�N���鏈���B<br>
///			�ʏ헿���v�Z��CheckSeasonCardData(ope/opemain.c)�ɑ����B<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
eSEASON_CHK_RESULT cal_cloud_season_check()
{
	stMasterInfo_t *pMaster = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo;	// �}�X�^�[���
	stZaishaInfo_t *pZaisha = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo;	// �ݎԏ��
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����
	stParkKindNum_t *pCard = NULL;
	int i;
	stDatetTimeYtoSec_t processTime;
	ushort pass_id;
	uchar ascDat[ALM_LOG_ASC_DAT_SIZE];
	uchar typ;

	// �J�[�h����
	for ( i = 0; i < ONL_MAX_CARDNUM; i++ ) {
		if( pMaster->stCardDataInfo[i].CardType == CARD_TYPE_PASS ) {	// ���
			pCard = &pMaster->stCardDataInfo[i];
			break;
		}
	}
	if ( pCard == NULL ) {	// �����������Ȃ�
		return SEASON_CHK_OK;	// �G���[�Ȃ�
	}

	// ������
	memset(&CRD_DAT.PAS, 0, sizeof(pas_rcc));

	// �A���[���p���ݒ�
	memset(ascDat, 0x00, ALM_LOG_ASC_DAT_SIZE);
	memcpy(&ascDat[0], &pCard->ParkingLotNo, 4);
	pass_id = astoin(pCard->byCardNo, 5);
	memcpy(&ascDat[4], &pass_id, 2);

	// ������A���[�����̃`�F�b�N
	switch( pFeeCalc->PassAlarm ) {
	case 1:		// �������
		alm_chk2(ALMMDL_SUB2, ALARM_GT_MUKOU_PASS_USE, 2, 1, 1, (void *)&ascDat);
		return SEASON_CHK_INVALID_SEASONDATA;	// ��������G���[
	case 2:		// �̔��O���
		return SEASON_CHK_PRE_SALE_NG;			// �̔��O����G���[
	case 3:		// �����O���
// GG129000(S) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j
// 		alm_chk2(ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&ascDat);
// 		return SEASON_CHK_BEFORE_VALID;			// �����O�G���[
		// ���1�`15 �����؂��t�ݒ�(07-0005�E,07-0015�E,,,07-0145�E)��
		// �u����(=1)�v�����̓���Ƃ���ׁA�����O�G���[�𖳎����Ď󂯕t����B
// GG129000(E) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j
	case 4:		// �����؂���
// GG129000(S) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j
// 		alm_chk2(ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&ascDat);
// 		return SEASON_CHK_AFTER_VALID;			// �����؂�G���[
		// ���1�`15 �����؂��t�ݒ�(07-0005�E,07-0015�E,,,07-0145�E)��
		// �u����(=1)�v�����̓���Ƃ���ׁA�����؂�G���[�𖳎����Ď󂯕t����B
// GG129000(E) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j
	case 0:		// ����
	default:	// ���̑�
		break;
	}

	// ���ԏꇂ�̃`�F�b�N
	for ( typ = 0; typ < 4; typ++ ) {
		if ( pCard->ParkingLotNo == prm_get(COM_PRM, S_SYS, (1 + typ), 6, 1) ) {
			break;
		}
	}
	if ( (pCard->ParkingLotNo == 0) ||	// ���ԏꇂ��0
		 (typ >= 4) ) {					// ���ԏꇂ����{�E�g���P�`�R�̐ݒ�ƈ�v���Ȃ�
		alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&ascDat);
		return SEASON_CHK_INVALID_PARKINGLOT;	// ���ԏꇂ�G���[
	}

	// �������������̐ݒ�
	memset(&processTime, 0, sizeof(processTime));
	switch ( pMaster->InOutStatus ) {
	case 1:	// �o�ɒ�
		memcpy(&processTime, &pZaisha->dtPaymentDateTime.dtTimeYtoSec, sizeof(processTime));
		break;
	case 2:	// ���ɒ�
		memcpy(&processTime, &pZaisha->dtEntryDateTime.dtTimeYtoSec, sizeof(processTime));
		break;
	case 0:	// �������
	default:
		break;
	}

	// Pass Kind
	CRD_DAT.PAS.knd = pMaster->SeasonKind;			// ������

	// Pass Start Day (BIN)
	CRD_DAT.PAS.std = dnrmlzm(
		(short)pMaster->StartDate.shYear,			// �L���J�n�N
		(short)pMaster->StartDate.byMonth,			// �L���J�n��
		(short)pMaster->StartDate.byDay);			// �L���J�n��

	// Pass End Day (BIN)
	CRD_DAT.PAS.end = dnrmlzm(
		(short)pMaster->EndDate.shYear,				// �L���I���N
		(short)pMaster->EndDate.byMonth,			// �L���I����
		(short)pMaster->EndDate.byDay);				// �L���I����

	// [0]-[2] = Start Day, [3]-[5] = End Day
	CRD_DAT.PAS.std_end[0] = (char)(pMaster->StartDate.shYear % 100);	// �L���J�n�N
	CRD_DAT.PAS.std_end[1] = (char)pMaster->StartDate.byMonth;			// �L���J�n��
	CRD_DAT.PAS.std_end[2] = (char)pMaster->StartDate.byDay;			// �L���J�n��
	CRD_DAT.PAS.std_end[3] = (char)(pMaster->EndDate.shYear % 100);		// �L���I���N
	CRD_DAT.PAS.std_end[4] = (char)pMaster->EndDate.byMonth;			// �L���I����
	CRD_DAT.PAS.std_end[5] = (char)pMaster->EndDate.byDay;				// �L���I����

	// Parking No.
	CRD_DAT.PAS.pno = (long)pCard->ParkingLotNo;	// ���ԏꇂ

	// GT Format Flag
	CRD_DAT.PAS.GT_flg = 1;

	// Personal Code
	CRD_DAT.PAS.cod = astoin(pCard->byCardNo, 5);	// �J�[�h�ԍ�

	// Status
	CRD_DAT.PAS.sts = (short)pMaster->InOutStatus;	// ���o�ɃX�e�[�^�X

	// Tranzaction Day-Time
	CRD_DAT.PAS.trz[0] = processTime.byMonth;		// ������
	CRD_DAT.PAS.trz[1] = processTime.byDay;			// ������
	CRD_DAT.PAS.trz[2] = processTime.byHours;		// ������
	CRD_DAT.PAS.trz[3] = processTime.byMinute;		// ������

	// Pass Type
	CRD_DAT.PAS.typ = (char)typ;					// ����敪

	return SEASON_CHK_OK;	// �G���[�Ȃ�
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ����ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
///	@note			�O������F�����ݒ�(cal_cloud_fee_set)�����{�ς�
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_season_set()
{
	stMasterInfo_t *pMaster = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo;	// �}�X�^�[���
// GG129000(S) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����
// GG129000(E) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j
	ushort ndatTemp;

	// ����VL�J�[�h�f�[�^
	vl_now = V_TSC;										// �����(���Ԍ����p�L��)

	// ���Z���@
	OpeNtnetAddedInfo.PayMethod = 2;					// ��������Z

#if 1	// �Q�l�Fal_pasck_set
	// ��������
	PayData.teiki.syu = (uchar)CRD_DAT.PAS.knd;			// ������
	NTNET_Data152Save((void *)(&PayData.teiki.syu), NTNET_152_TEIKISYU);

	// ������X�e�[�^�X(�ǎ掞)
	PayData.teiki.status = (uchar)CRD_DAT.PAS.sts;		// ���o�ɃX�e�[�^�X

	// �����ID
	PayData.teiki.id = CRD_DAT.PAS.cod;					// �J�[�h�ԍ�
	NTNET_Data152Save((void *)(&PayData.teiki.id), NTNET_152_TEIKIID);

	// ��������ԏ�No.���
	PayData.teiki.pkno_syu = (uchar)CRD_DAT.PAS.typ;	// ����敪
	NTNET_Data152Save((void *)(&PayData.teiki.pkno_syu), NTNET_152_PKNOSYU);

	// �X�V����
	PayData.teiki.update_mon = 0;

	// �L������(�J�n:�N)
	PayData.teiki.s_year = pMaster->StartDate.shYear;	// �L���J�n�N

	// �L������(�J�n:��)
	PayData.teiki.s_mon = pMaster->StartDate.byMonth;	// �L���J�n��

	// �L������(�J�n:��)
	PayData.teiki.s_day = pMaster->StartDate.byDay;		// �L���J�n��

	// �L������(�I��:�N)
	PayData.teiki.e_year = pMaster->EndDate.shYear;		// �L���I���N

	// �L������(�I��:��)
	PayData.teiki.e_mon = pMaster->EndDate.byMonth;		// �L���I����

	// �L������(�I��:��)
	PayData.teiki.e_day = pMaster->EndDate.byDay;		// �L���I����

	// ������������
	memcpy(PayData.teiki.t_tim, CRD_DAT.PAS.trz, 4);	// ������������

	// ����X�V���Z���̍X�V����
	PayData.teiki.update_rslt1 = 0;

	// ����X�V���Z���̃��x�����s����
	PayData.teiki.update_rslt2 = 0;

	// ���ԏ�No.
	PayData.teiki.ParkingNo = (ulong)CRD_DAT.PAS.pno;
	NTNET_Data152Save((void *)(&PayData.teiki.ParkingNo), NTNET_152_PARKNOINPASS);

	// �yGT-4100���g�p�z���������J�[�hn���`�F�b�N
	PassPkNoBackup = 0;
	PassIdBackup = 0;
	PassIdBackupTim = 0;

	// �J�[�h���g�p����(�����)
	card_use[USE_PAS] += 1;
#endif	// �Q�l�Fal_pasck_set

#if 1	// �Q�l�Fop_mod02
// GG129000(S) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j
	// �����O�E�����؂������󂯕t�����ꍇ�͊����؂�ԋ߃`�F�b�N�����Ȃ�
	if ( (pFeeCalc->PassAlarm != 3) &&	// �����O���
	     (pFeeCalc->PassAlarm != 4) ) {	// �����؂���
// GG129000(E) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j

	// ��������؂�ԋ߃`�F�b�N
	ndatTemp = (ushort)prm_get(COM_PRM, S_TIK, 13, 2, 1);	// ����������؂�\��
	if ( ndatTemp <= 15 ) {		// �����؂�n���O
		if ( ndatTemp == 0 ) {	// �����؂�3���O
			ndatTemp = 3;
		}
		ndatTemp = CRD_DAT.PAS.end - ndatTemp;
		if ( cal_cloud_pay_start_time_check(ndatTemp) >= 0 ) {
			OPECTL.PassNearEnd = 1;	// �����؂�ԋ�
		}
	}

// GG129000(S) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j
	}
// GG129000(E) R.Endo 2023/01/16 �Ԕԃ`�P�b�g���X4.0 #6807 �����؂�̒���̈����������؂��t���鑊���̓���ɕύX����i�ݒ�07-0005�E�j
#endif	// �Q�l�Fop_mod02

	// �����f�[�^�o�b�t�@�m�ݒ�
	ryo_buf_n_set();

#if 1	// �Q�l�Fet02(et47)
	// �N���E�h�����v�Z�ł͍ŏ��������𔽉f�������ԗ����Ȃ̂ŉ��߂Đݒ肵�Ȃ�
#endif	// �Q�l�Fet02(et47)

	// �����f�[�^�o�b�t�@�m�v�Z
	ryo_buf_n_calc();

	// �����f�[�^�o�b�t�@�m�擾
	ryo_buf_n_get(1);	// ����ݒ�

	// �����f�[�^�o�b�t�@�v�Z
	ryo_buf_calc();

#if 1	// �Q�l�Fop_mod02
	// �I�y���[�V�����t�F�[�Y
	if ( OPECTL.op_faz == 0 ) {	// �������M
		OPECTL.op_faz = 1;	// ������
	}

	// ���Z�T�C�N���t���O
	ac_flg.cycl_fg = 10;	// ����

	// ���������T�u�F�Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
	in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);
#endif	// �Q�l�Fop_mod02

	return 0;	// ��������
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z QR�f�[�^�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		tMediaDetail	: QR�f�[�^
///	@param[in]		QR_YMDData		: QR���t�\����
///	@return			ret		: QR�f�[�^�`�F�b�N����<br>
///							  RESULT_NO_ERROR = �g�p�\<br>
///							  RESULT_DISCOUNT_TIME_MAX = �������<br>
///							  RESULT_BAR_EXPIRED = �L�������O<br>
///							  RESULT_BAR_ID_ERR = �ΏۊO<br>
///	@note			QR���t�\���̂�ope_CanUseBarcode_sub�Ŋ����ύX�e�[�u��<br>
///					(08-0008�`)���l���������t�����Ă���B
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
OPE_RESULT cal_cloud_qrdata_check(tMediaDetail *pMedia, QR_YMDData* pYmdData)
{
	QR_DiscountInfo *pDisc = &pMedia->Barcode.QR_data.DiscountType;	// QR�f�[�^��� QR������
	int i;
	ushort ndatStart = 0;		// �L���J�n��(������)
	ushort ndatEnd = 0xffff;	// �L���I����(������)
	ushort dayTemp;
	uchar foundFlag = 0;		// ������Ȃ�
// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
	uchar kaimonoCount = 0;		// ���������Ȃ�

	// ��������������
	for ( i = 0; i < CardUseSyu; i++ ) {
		if ( (PayData.DiscountData[i].DiscSyu == NTNET_SHOP_DISC_AMT) ||	// ��������(���z)
			 (PayData.DiscountData[i].DiscSyu == NTNET_SHOP_DISC_TIME) ) {	// ��������(����)
			kaimonoCount += 1;	// 1���Z��1�����Z�b�g����Ȃ����ߌ���������Z���Ĕ�����
			break;
		}
	}
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������

	switch ( pMedia->Barcode.id ) {
	case BAR_ID_DISCOUNT:	// QR������
		// QR�������F�������(�[���̐���)
		switch ( pDisc->DiscKind ) {
		case NTNET_SVS_M:			// �T�[�r�X������(���z)
		case NTNET_SVS_T:			// �T�[�r�X������(����)
			// �T�[�r�X���FQR�����������敪�̒l��(�[���̐���)
			if ( !rangechk(1, SVS_MAX, pDisc->DiscClass) ) {
				return RESULT_BAR_ID_ERR;	// �ΏۊO
			}

			// ����̊���������
			for ( i = 0; i < CardUseSyu; i++ ) {
				if (   (PayData.DiscountData[i].ParkingNo == pDisc->ParkingNo)	// ���ԏꇂ
					&& (PayData.DiscountData[i].DiscSyu   == pDisc->DiscKind)	// �������
					&& (PayData.DiscountData[i].DiscNo    == pDisc->DiscClass)	// �T�[�r�X�����
					&& (PayData.DiscountData[i].DiscInfo1 == pDisc->ShopNp)		// �|�����
				) {
					foundFlag = 1;	// ��������
					break;
				}
			}

			break;
		case NTNET_KAK_M:			// �X����(���z)
		case NTNET_KAK_T:			// �X����(����)
		case NTNET_TKAK_M:			// ���X�܊���(���z)
		case NTNET_TKAK_T:			// ���X�܊���(����)
			// ����̊���������
			for ( i = 0; i < CardUseSyu; i++ ) {
				if (   (PayData.DiscountData[i].ParkingNo == pDisc->ParkingNo)	// ���ԏꇂ
					&& (PayData.DiscountData[i].DiscSyu   == pDisc->DiscKind)	// �������
					&& (PayData.DiscountData[i].DiscNo    == pDisc->ShopNp)		// �X��
					&& (PayData.DiscountData[i].DiscInfo1 == pDisc->DiscClass)	// �������/�������
				) {
					foundFlag = 1;	// ��������
					break;
				}
			}
			break;
		default:
			return RESULT_BAR_ID_ERR;	// �ΏۊO
		}

		// QR�������F���ꊄ�����퐔(�[���̐���)
// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// 		if ( CardUseSyu >= WTIK_USEMAX ) {	// ���ꊄ�����퐔���ő�l�ȏ�
// 			// ����̊�����������Ȃ��ꍇ�͊�������Ƃ���
// 			if ( !foundFlag ) {
// 				return RESULT_DISCOUNT_TIME_MAX;	// �������
// 			}
		if ( (CardUseSyu - kaimonoCount - foundFlag) >= (WTIK_USEMAX - 1) ) {	// ����Ɣ������������������퐔������10��ȏ�
			return RESULT_DISCOUNT_TIME_MAX;	// �������
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
		}

		// �L��������ݒ�
		if ( (pYmdData->StartDate.Year != 0) ||
			 (pYmdData->StartDate.Mon  != 0) ||
			 (pYmdData->StartDate.Day  != 0) ||
			 (pYmdData->EndDate.Year != 99) ||
			 (pYmdData->EndDate.Mon  != 99) ||
			 (pYmdData->EndDate.Day  != 99) ) {		// �L����������
			ndatStart = dnrmlzm(
				(short)pYmdData->StartDate.Year,
				(short)pYmdData->StartDate.Mon,
				(short)pYmdData->StartDate.Day);	// �L���J�n��
			ndatEnd = dnrmlzm(
				(short)pYmdData->EndDate.Year,
				(short)pYmdData->EndDate.Mon,
				(short)pYmdData->EndDate.Day);		// �L���I����
		}

		break;
	case BAR_ID_AMOUNT:	// QR���㌔
// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
		// QR���㌔�F�������z���v������
		for ( i = 0; i < CardUseSyu; i++ ) {
			if ( PayData.DiscountData[i].DiscSyu == NTNET_KAIMONO_GOUKEI ) {	// ������� �������z���v
				foundFlag = 1;	// ��������
				break;
			}
		}

		// QR���㌔�F���ꊄ�����퐔(�[���̐���)
		if ( (CardUseSyu - kaimonoCount - foundFlag) >= (WTIK_USEMAX - 1) ) {	// ����Ɣ������������������퐔������10��ȏ�
			return RESULT_DISCOUNT_TIME_MAX;	// �������
		}
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������

		// �L��������ݒ�
		ndatStart = dnrmlzm(
			(short)pYmdData->IssueDate.Year,
			(short)pYmdData->IssueDate.Mon,
			(short)pYmdData->IssueDate.Day);	// �L���J�n��
		dayTemp = (ushort)prm_get(COM_PRM, S_SYS, 77, 2, 1);	// �L������
		if ( dayTemp == 0 ) {
			dayTemp = 99;
		}
		ndatEnd = ndatStart + dayTemp - 1;

		break;
	}

	// QR�L������
	if ( (cal_cloud_pay_start_time_check(ndatStart) < 0) ||	// �L���J�n���O
		 (cal_cloud_pay_start_time_check(ndatEnd) > 0) ) {	// �L���I������
		return RESULT_BAR_EXPIRED;	// �L�������O
	}

	return RESULT_NO_ERROR;	// �g�p�\
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ���ɏ��`�F�b�N(����)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ���ɏ��`�F�b�N����<br>
///							  0 = �����ݒ��<br>
///							  1 = ���ɏ��NG<br>
///							  2 = ���ɏ��NG(QR����Ȃ�)<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_discount_check()
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����

	// �����v�Z���ʃG���[�A���[���`�F�b�N
	pay_result_error_alarm_check(pFeeCalc->shPayResult);

	// �����d�����p�`�F�b�N�̃`�F�b�N
	switch( lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup ) {
	case 0:		// �`�F�b�N�Ȃ�
	case 1:		// �����d���Ȃ�
		break;
	default:	// ���̑�
		return 2;	// ���ɏ��NG(QR����Ȃ�)
	}

	// �������擾���ʂ̃`�F�b�N
	switch( pFeeCalc->CalcResult ) {
	case 0:		// ����
	case 2:		// �����Ȃ�
		break;
	case 1:		// �������s(�Z���^�[�����G���W���G���[)
	default:	// ���̑�
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 6);	// 6 = �����K�p���s
		return 1;	// ���ɏ��NG
	}

	// �����v�Z���ʂ̃`�F�b�N(�|�b�v�A�b�v�ƌ���)
	switch( pFeeCalc->shPayResult ) {
	case 0:		// ����
	case 1001:	// �����Ȃ�
		break;
	case 64:	// �����֘A�G���[(���Ή��̊������܂܂�Ă��܂�)
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 1);	// 1 = �������NG
		return 1;	// ���ɏ��NG
	case 65:	// �����֘A�G���[(�����̎g�p�����̍��v���S�����g�p���x���𒴉߂��Ă��܂�)
	case 66:	// �����֘A�G���[(�T�[�r�X���̎g�p�������g�p���������𒴉߂��Ă��܂�)
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 2);	// 2 = ����NG
		return 1;	// ���ɏ��NG
	case 62:	// �����֘A�G���[(���p�s�Ԏ�ɐݒ肳��Ă���ׁA�T�[�r�X�������p�ł��܂���)
	case 63:	// �����֘A�G���[(2���ڈȍ~��%�����͗��p�ł��܂���)
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 3);	// 3 = �J�[�h�敪NG
		return 1;	// ���ɏ��NG
	case 61:	// �����֘A�G���[(���񐸎Z�ȊO�͎�ʐؑւ͗��p�s��)
	case 67:	// �����֘A�G���[(�Đ��Z��%�����͐V���ɗ��p�ł��܂���)
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 4);	// 4 = �Ԏ�ؑ֕s��
		return 1;	// ���ɏ��NG
	default:	// ���̑�
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 6);	// 6 = �����K�p���s
		return 1;	// ���ɏ��NG
	}

	return 0;	// �����ݒ��
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z �����ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: �����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
///	@note			�O������F�����ݒ�(cal_cloud_fee_set)�����{�ς�
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_discount_set()
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����
	ulong discAmountTemp;
	int i;
	uchar countTemp;
	uchar addFlag;
	uchar j;

// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// 	// ����������
// 	discount_init();
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�

	for ( i = 0; i < ONL_MAX_DISC_NUM; i++ ) {
		if ( !(pFeeCalc->stDiscountInfo[i].DiscParkNo) ) {
			break;
		}

		// �T�[�r�X���E�|�����̖���
		switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
		case NTNET_SVS_M:			// �T�[�r�X������(���z)
		case NTNET_SVS_T:			// �T�[�r�X������(����)
		case NTNET_KAK_M:			// �X����(���z)
		case NTNET_KAK_T:			// �X����(����)
		case NTNET_TKAK_M:			// ���X�܊���(���z)
		case NTNET_TKAK_T:			// ���X�܊���(����)
		case NTNET_SHOP_DISC_AMT:	// ��������(���z)
		case NTNET_SHOP_DISC_TIME:	// ��������(����)
			countTemp = pFeeCalc->stDiscountInfo[i].DiscCount;
			break;
		default:
			countTemp = 0;
			break;
		}

		// �J�[�h���g�p�����e�[�u��(�T�[�r�X���E�|����)
		card_use[USE_SVC] += countTemp;

		// �J�[�h���g�p�����e�[�u��(�V�K�T�[�r�X���E�|����)
		if ( pFeeCalc->stDiscountInfo[i].DiscStatus < 2 ) {
			card_use[USE_N_SVC] += countTemp;
		}

		// ����VL�J�[�h�f�[�^
		vl_now = V_SAK;	// �T�[�r�X��

		// �����f�[�^�o�b�t�@�m�ݒ�
		ryo_buf_n_set();

#if 1	// �Q�l�Fet02(et45/et51)
		// �������z
		discAmountTemp  = pFeeCalc->stDiscountInfo[i].DiscAmount;
		discAmountTemp += pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

		switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
		case NTNET_SYUBET_TIME:		// ��ʊ���(����)
		case NTNET_SYUBET:			// ��ʊ���(���z)
			// �����z�������
			if ( ryo_buf_n.ryo < discAmountTemp ) {
				ryo_buf_n.ryo = 0;
			} else {
				ryo_buf_n.ryo -= discAmountTemp;
			}
			break;
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6549 ���������z���v(2001)�́u���v���z�v�̏��𒓎ԗ����̊����ɓK�p���Ă���
// 		default:
		case NTNET_SVS_M:			// �T�[�r�X������(���z)
		case NTNET_KAK_M:			// �X����(���z)
		case NTNET_TKAK_M:			// ���X�܊���(���z)
		case NTNET_SHOP_DISC_AMT:	// ��������(���z)
		case NTNET_SVS_T:			// �T�[�r�X������(����)
		case NTNET_KAK_T:			// �X����(����)
		case NTNET_TKAK_T:			// ���X�܊���(����)
		case NTNET_SHOP_DISC_TIME:	// ��������(����)
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6549 ���������z���v(2001)�́u���v���z�v�̏��𒓎ԗ����̊����ɓK�p���Ă���
			// �����z�ɑ���
			ryo_buf_n.dis += discAmountTemp;
			break;
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6549 ���������z���v(2001)�́u���v���z�v�̏��𒓎ԗ����̊����ɓK�p���Ă���
		default:
			break;
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6549 ���������z���v(2001)�́u���v���z�v�̏��𒓎ԗ����̊����ɓK�p���Ă���
		}

		switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
		case NTNET_SVS_M:			// �T�[�r�X������(���z)
		case NTNET_KAK_M:			// �X����(���z)
		case NTNET_TKAK_M:			// ���X�܊���(���z)
		case NTNET_SHOP_DISC_AMT:	// ��������(���z)
		case NTNET_SYUBET:			// ��ʊ���(���z)
			// ���������̍��v���z(�|�C���g�J�[�h�͏���)
			ryo_buf.fee_amount += discAmountTemp;
			break;
		case NTNET_SVS_T:			// �T�[�r�X������(����)
		case NTNET_KAK_T:			// �X����(����)
		case NTNET_TKAK_T:			// ���X�܊���(����)
		case NTNET_SHOP_DISC_TIME:	// ��������(����)
		case NTNET_SYUBET_TIME:		// ��ʊ���(����)
			// ���Ԋ����̍��v���z(�|�C���g�J�[�h�͏���)
			ryo_buf.tim_amount += pFeeCalc->stDiscountInfo[i].DiscTime;
			ryo_buf.tim_amount += pFeeCalc->stDiscountInfo[i].PrevUsageDiscTime;
			break;
// GG129000(S) R.Endo 2024/01/12 #7217 ���������v�\���Ή�
		case NTNET_KAIMONO_GOUKEI:	// �������z���v
			// �����̍��v���z
			ryo_buf.shopping_total += pFeeCalc->stDiscountInfo[i].DiscAmountSetting;
			break;
// GG129000(E) R.Endo 2024/01/12 #7217 ���������v�\���Ή�
// GG129004(S) M.Fujikawa 2024/12/11 �������z�������Ή�
		case NTNET_KAIMONO_INFO:	// �������
			if(pFeeCalc->stDiscountInfo[i].DiscInfo == 0){
				ryo_buf.shopping_info = 0xFFFFFFFF;
			}else{ 
				ryo_buf.shopping_info = pFeeCalc->stDiscountInfo[i].DiscInfo;
			}
			break;
// GG129004(E) M.Fujikawa 2024/12/11 �������z�������Ή�
		default:
			break;
		}
#endif	// �Q�l�Fet02(et45/et51)

		// �����f�[�^�o�b�t�@�m�v�Z
		ryo_buf_n_calc();

		// �����f�[�^�o�b�t�@�m�擾
		ryo_buf_n_get(0);	// �ʏ�

#if 1	// �Q�l�Fryo_cal
		// �����ǉ��t���O���N���A
		addFlag = 0;

		switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
		case NTNET_SVS_M:			// �T�[�r�X������(���z)
		case NTNET_SVS_T:			// �T�[�r�X������(����)
			// ����̊����͂܂Ƃ߂�
			for ( j = 0; j < CardUseSyu; j++ ) {
				if (   (PayData.DiscountData[j].ParkingNo == pFeeCalc->stDiscountInfo[i].DiscParkNo)	// ���ԏꇂ
					&& (PayData.DiscountData[j].DiscSyu   == pFeeCalc->stDiscountInfo[i].DiscSyu)		// �������
					&& (PayData.DiscountData[j].DiscNo    == pFeeCalc->stDiscountInfo[i].DiscCardNo)	// �T�[�r�X�����
					&& (PayData.DiscountData[j].DiscInfo1 == pFeeCalc->stDiscountInfo[i].DiscInfo)		// �|�����
				) {
					// �������X�V
					addFlag = 1;

					break;
				}
			}

			// ����̊�����������Ȃ��ꍇ�͒ǉ�����
			if ( (j >= CardUseSyu) && (CardUseSyu < WTIK_USEMAX) ) {
				// ������ǉ�
				addFlag = 2;
			}

			break;
		case NTNET_KAK_M:			// �X����(���z)
		case NTNET_KAK_T:			// �X����(����)
		case NTNET_TKAK_M:			// ���X�܊���(���z)
		case NTNET_TKAK_T:			// ���X�܊���(����)
			// ����̊����͂܂Ƃ߂�
			for ( j = 0; j < CardUseSyu; j++ ) {
				if (   (PayData.DiscountData[j].ParkingNo  == pFeeCalc->stDiscountInfo[i].DiscParkNo)	// ���ԏꇂ
					&& (PayData.DiscountData[j].DiscSyu    == pFeeCalc->stDiscountInfo[i].DiscSyu)		// �������
					&& (PayData.DiscountData[j].DiscNo     == pFeeCalc->stDiscountInfo[i].DiscCardNo)	// �X��
					&& (PayData.DiscountData[j].DiscInfo1  == pFeeCalc->stDiscountInfo[i].DiscInfo)		// �������/�������
				) {
					// �������X�V
					addFlag = 1;

					break;
				}
			}

			// ����̊�����������Ȃ��ꍇ�͒ǉ�����
			if ( (j >= CardUseSyu) && (CardUseSyu < WTIK_USEMAX) ) {
				// ������ǉ�
				addFlag = 2;
			}

			break;
		case NTNET_SHOP_DISC_AMT:	// ��������(���z)
		case NTNET_SHOP_DISC_TIME:	// ��������(����)
			// ������ǉ�(����������1�����Z�b�g���Ȃ�)
			addFlag = 2;

			break;
		case NTNET_SYUBET:			// ��ʊ���(���z)
		case NTNET_SYUBET_TIME:		// ��ʊ���(����)
			// ������ǉ�(��ʊ�����1�����Z�b�g���Ȃ�)
			addFlag = 2;

// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// 			// ��ʊ�������
// 			PayData.SyuWariRyo += pFeeCalc->stDiscountInfo[i].DiscAmount;	// ����̊������z(����)
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������

			break;
// GG124100(S) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6125 �yPK���ƕ��v���zNT-NET���Z�f�[�^�̊�������2001(�������z���v)���Z�b�g
		case NTNET_KAIMONO_GOUKEI:	// �������z���v
			// �������z���v��ǉ�(�������z���v��1�����Z�b�g���Ȃ�)
			addFlag = 3;

			break;
// GG124100(E) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6125 �yPK���ƕ��v���zNT-NET���Z�f�[�^�̊�������2001(�������z���v)���Z�b�g
		default:
			break;
		}

		// NT-NET���Z�f�[�^�p�̃f�[�^��ݒ�
		switch ( addFlag ) {
		case 1:	// �������X�V
			// �������Ԑ�
			switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
			case NTNET_SVS_T:			// �T�[�r�X������(����)
			case NTNET_KAK_T:			// �X����(����)
			case NTNET_TKAK_T:			// ���X�܊���(����)
			case NTNET_SHOP_DISC_TIME:	// ��������(����)
			case NTNET_SYUBET_TIME:		// ��ʊ���(����)
// GG129000(S) R.Endo 2022/12/14 �Ԕԃ`�P�b�g���X4.0 #6758 �����ʂ̎��Ԋ������Đ��Z�������p�����ꍇ�ANT-NET���Z�f�[�^�̊������2�ɃZ�b�g���銄�����Ԃ̒l���s��
// 				PayData.DiscountData[j].uDiscData.common.DiscInfo2 +=
// 					pFeeCalc->stDiscountInfo[i].DiscTimeSetting;
				if ( pFeeCalc->stDiscountInfo[i].DiscStatus != 2 ) {	// ������
					PayData.DiscountData[j].uDiscData.common.DiscInfo2 +=
						pFeeCalc->stDiscountInfo[i].DiscTimeSetting;
				}
// GG129000(E) R.Endo 2022/12/14 �Ԕԃ`�P�b�g���X4.0 #6758 �����ʂ̎��Ԋ������Đ��Z�������p�����ꍇ�ANT-NET���Z�f�[�^�̊������2�ɃZ�b�g���銄�����Ԃ̒l���s��
				break;
			default:
				break;
			}

			// ����g�p�����������z
			PayData.DiscountData[j].Discount +=
				pFeeCalc->stDiscountInfo[i].DiscAmount;

			// �O�񐸎Z�܂ł̎g�p�ς݊������z
// GG129000(S) R.Endo 2022/12/08 �Ԕԃ`�P�b�g���X4.0 #6755 PayData�̎g�p�ς݊����̔z��C���f�b�N�X�̕ϐ��ԈႢ
// 			PayData.DiscountData[jik_dtm].uDiscData.common.PrevUsageDiscount +=
			PayData.DiscountData[j].uDiscData.common.PrevUsageDiscount +=
// GG129000(E) R.Endo 2022/12/08 �Ԕԃ`�P�b�g���X4.0 #6755 PayData�̎g�p�ς݊����̔z��C���f�b�N�X�̕ϐ��ԈႢ
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 2 ) {	// �����ς�
				// ����g�p�����O�񐸎Z�܂ł̊������z
				PayData.DiscountData[j].uDiscData.common.PrevDiscount +=
					pFeeCalc->stDiscountInfo[i].DiscAmount;

				// �O�񐸎Z�܂ł̎g�p�ςݖ���
				PayData.DiscountData[j].uDiscData.common.PrevUsageDiscCount +=
					pFeeCalc->stDiscountInfo[i].DiscCount;
			} else {												// ������
				// ����g�p��������
				PayData.DiscountData[j].DiscCount +=
					pFeeCalc->stDiscountInfo[i].DiscCount;
			}

			// NT-NET���Z���f�[�^�p�ۑ��̈�ɕۑ�
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_DCOUNT, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_DISCOUNT, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_DINFO2, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_PREVDISCOUNT, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_PREVUSAGEDISCOUNT, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_PREVUSAGEDCOUNT, j);

			break;
		case 2:	// ������ǉ�
			// �i�[�ł��錏���𒴂����ꍇ�͊i�[���Ȃ�
			if ( CardUseSyu >= WTIK_USEMAX ) {
				break;
			}

			// ���ԏꇂ
			PayData.DiscountData[CardUseSyu].ParkingNo =
				pFeeCalc->stDiscountInfo[i].DiscParkNo;

			// �������
			PayData.DiscountData[CardUseSyu].DiscSyu =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// �����敪
			PayData.DiscountData[CardUseSyu].DiscNo =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			// ����g�p�����������z
			PayData.DiscountData[CardUseSyu].Discount =
				pFeeCalc->stDiscountInfo[i].DiscAmount;

			// �������1
			PayData.DiscountData[CardUseSyu].DiscInfo1 =
				(ulong)pFeeCalc->stDiscountInfo[i].DiscInfo;

			// �������2
			switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
			case NTNET_SVS_T:			// �T�[�r�X������(����)
			case NTNET_KAK_T:			// �X����(����)
			case NTNET_TKAK_T:			// ���X�܊���(����)
			case NTNET_SHOP_DISC_TIME:	// ��������(����)
			case NTNET_SYUBET_TIME:		// ��ʊ���(����)
// GG129000(S) R.Endo 2022/12/14 �Ԕԃ`�P�b�g���X4.0 #6758 �����ʂ̎��Ԋ������Đ��Z�������p�����ꍇ�ANT-NET���Z�f�[�^�̊������2�ɃZ�b�g���銄�����Ԃ̒l���s��
// 				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscInfo2 =
// 					pFeeCalc->stDiscountInfo[i].DiscTimeSetting;
				if ( pFeeCalc->stDiscountInfo[i].DiscStatus != 2 ) {	// ������
					PayData.DiscountData[CardUseSyu].uDiscData.common.DiscInfo2 =
						pFeeCalc->stDiscountInfo[i].DiscTimeSetting;
				}
// GG129000(E) R.Endo 2022/12/14 �Ԕԃ`�P�b�g���X4.0 #6758 �����ʂ̎��Ԋ������Đ��Z�������p�����ꍇ�ANT-NET���Z�f�[�^�̊������2�ɃZ�b�g���銄�����Ԃ̒l���s��
				break;
			default:
				break;
			}

			// �O�񐸎Z�܂ł̎g�p�ς݊������z
			PayData.DiscountData[CardUseSyu].uDiscData.common.PrevUsageDiscount =
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 2 ) {	// �����ς�
				// ����g�p�����O�񐸎Z�܂ł̊������z
				PayData.DiscountData[CardUseSyu].uDiscData.common.PrevDiscount =
					pFeeCalc->stDiscountInfo[i].DiscAmount;

				// �O�񐸎Z�܂ł̎g�p�ςݖ���
				PayData.DiscountData[CardUseSyu].uDiscData.common.PrevUsageDiscCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;
			} else {												// ������
				// ����g�p��������
				PayData.DiscountData[CardUseSyu].DiscCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;
			}

// GG124100(S) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
			// ��ʐ؊���Ԏ�
			PayData.DiscountData[CardUseSyu].uDiscData.common.FeeKindSwitchSetting =
				pFeeCalc->stDiscountInfo[i].FeeKindSwitchSetting;

			// ��������
			if ( pFeeCalc->stDiscountInfo[i].DiscTimeSetting ) {			// ���Ԋ���
				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 1;
			} else if ( pFeeCalc->stDiscountInfo[i].DiscAmountSetting ) {	// ��������
				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 2;
			} else if ( pFeeCalc->stDiscountInfo[i].DiscPercentSetting ) {	// ������
				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 3;
			} else {														// �����Ȃ�
// GG124100(S) R.Endo 2022/09/13 �Ԕԃ`�P�b�g���X3.0 #6589 QR�������̑S�z�������g�p����Ɨ̎��؂Ɋ�����񂪈󎚂���Ȃ�
// 				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 0;
				// �S�z�����Ή�
				if ( (pFeeCalc->stDiscountInfo[i].DiscAmount > 0) ||				// ����̊������z(����)����
					 (pFeeCalc->stDiscountInfo[i].FeeKindSwitchSetting == 0) ) {	// ��ʐ؊���Ԏ�Ȃ�
					PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 4;
				} else {
					PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 0;
				}
// GG124100(E) R.Endo 2022/09/13 �Ԕԃ`�P�b�g���X3.0 #6589 QR�������̑S�z�������g�p����Ɨ̎��؂Ɋ�����񂪈󎚂���Ȃ�
			}
// GG124100(E) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�

			// NT-NET���Z���f�[�^�p�ۑ��̈�ɕۑ�
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DPARKINGNO, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DSYU, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DNO, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DISCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO1, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO2, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_PREVDISCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_PREVUSAGEDISCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_PREVUSAGEDCOUNT, CardUseSyu);
// GG124100(S) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_FEEKINDSWITCHSETTING, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DROLE, CardUseSyu);
// GG124100(E) R.Endo 2022/08/19 �Ԕԃ`�P�b�g���X3.0 #6520 �g�p�����T�[�r�X���̏�񂪗̎��؂Ɉ󎚂���Ȃ�

			// �����������Z
			CardUseSyu++;

			break;
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6125 �yPK���ƕ��v���zNT-NET���Z�f�[�^�̊�������2001(�������z���v)���Z�b�g
// 		case 0:	// �������X�V���ǉ�������Ă��Ȃ��ꍇ
		case 3:	// �������z���v��ǉ�
			// �i�[�ł��錏���𒴂����ꍇ�͊i�[���Ȃ�
			if ( CardUseSyu >= WTIK_USEMAX ) {
				break;
			}

			// ���ԏꇂ
			PayData.DiscountData[CardUseSyu].ParkingNo =
				pFeeCalc->stDiscountInfo[i].DiscParkNo;

			// �������
			PayData.DiscountData[CardUseSyu].DiscSyu =
				pFeeCalc->stDiscountInfo[i].DiscSyu;


			// ����g�p�����������z(���v���z)
// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6568 ���A���^�C�����Z�f�[�^/NT-NET���Z�f�[�^�̔������z���v�́u�����z�v�ɒl���Z�b�g����Ȃ�
// 			PayData.DiscountData[CardUseSyu].Discount =
// 				pFeeCalc->stDiscountInfo[i].DiscAmount;
			PayData.DiscountData[CardUseSyu].Discount =
				pFeeCalc->stDiscountInfo[i].DiscAmountSetting;
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6568 ���A���^�C�����Z�f�[�^/NT-NET���Z�f�[�^�̔������z���v�́u�����z�v�ɒl���Z�b�g����Ȃ�

			// ����g�p��������
			PayData.DiscountData[CardUseSyu].DiscCount =
				pFeeCalc->stDiscountInfo[i].DiscCount;

			// NT-NET���Z���f�[�^�p�ۑ��̈�ɕۑ�
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DPARKINGNO, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DSYU, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DISCOUNT, CardUseSyu);

			// �����������Z
			CardUseSyu++;

			break;
		case 0:	// �ǉ����X�V���Ȃ��ꍇ
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6125 �yPK���ƕ��v���zNT-NET���Z�f�[�^�̊�������2001(�������z���v)���Z�b�g
		default:
			break;
		}
#endif	// �Q�l�Fryo_cal

		// �����f�[�^�o�b�t�@�v�Z
		ryo_buf_calc();

#if 1	// �Q�l�FDiscountForOnline
		// �I�y���[�V�����t�F�[�Y
		if ( OPECTL.op_faz == 0 ) {	// �������M
			OPECTL.op_faz = 1;	// ������
		}

		// ���Z�T�C�N���t���O
		ac_flg.cycl_fg = 10;	// ����

		// ���������T�u�F�Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)
		in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);
#endif	// �Q�l�FDiscountForOnline

#if 1	// �Q�l�FOnlineDiscount
// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6568 ���A���^�C�����Z�f�[�^/NT-NET���Z�f�[�^�̔������z���v�́u�����z�v�ɒl���Z�b�g����Ȃ�
		// ���A���^�C�����Z�f�[�^�p�̃f�[�^��ݒ�
		switch ( addFlag ) {
		case 1:	// �������X�V
		case 2:	// ������ǉ�
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6568 ���A���^�C�����Z�f�[�^/NT-NET���Z�f�[�^�̔������z���v�́u�����z�v�ɒl���Z�b�g����Ȃ�
			// ���ԏꇂ
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscParkNo =
				pFeeCalc->stDiscountInfo[i].DiscParkNo;

			// ���
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscSyu =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// �J�[�h�敪
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCardNo =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			// �敪
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscNo =
				pFeeCalc->stDiscountInfo[i].DiscNo;

			// ����
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCount =
				pFeeCalc->stDiscountInfo[i].DiscCount;

			// �������
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscInfo =
				pFeeCalc->stDiscountInfo[i].DiscInfo;

			// �Ή��J�[�h���
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCorrType =
				pFeeCalc->stDiscountInfo[i].DiscCorrType;

			// �X�e�[�^�X
			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 0 ) {	// ������
				RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscStatus = 1;	// ���񊄈�(���Z�����Ŋ����ςƂ���)
			} else {												// �������ȊO
				RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscStatus =
					pFeeCalc->stDiscountInfo[i].DiscStatus;
			}

			// ������
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscFlg =
				pFeeCalc->stDiscountInfo[i].DiscFlg;

			// �����z
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].Discount =
				pFeeCalc->stDiscountInfo[i].DiscAmount +
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)
			// ����
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscTime =
				pFeeCalc->stDiscountInfo[i].DiscTime +
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscTime;
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�J�[�h���Rev.No.11)

// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6568 ���A���^�C�����Z�f�[�^/NT-NET���Z�f�[�^�̔������z���v�́u�����z�v�ɒl���Z�b�g����Ȃ�
			break;
		case 3:	// �������z���v��ǉ�
			// ���ԏꇂ
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscParkNo =
				pFeeCalc->stDiscountInfo[i].DiscParkNo;

			// ���
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscSyu =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// �J�[�h�敪
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCardNo =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			// �敪
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscNo =
				pFeeCalc->stDiscountInfo[i].DiscNo;

			// ����
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCount =
				pFeeCalc->stDiscountInfo[i].DiscCount;

			// �������
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscInfo =
				pFeeCalc->stDiscountInfo[i].DiscInfo;

			// �Ή��J�[�h���
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCorrType =
				pFeeCalc->stDiscountInfo[i].DiscCorrType;

			// �X�e�[�^�X
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscStatus =
				pFeeCalc->stDiscountInfo[i].DiscStatus;

			// ������
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscFlg =
				pFeeCalc->stDiscountInfo[i].DiscFlg;

			// �����z
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].Discount =
				pFeeCalc->stDiscountInfo[i].DiscAmountSetting;

			break;
		case 0:	// �ǉ����X�V���Ȃ��ꍇ
		default:
			break;
		}
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6568 ���A���^�C�����Z�f�[�^/NT-NET���Z�f�[�^�̔������z���v�́u�����z�v�ɒl���Z�b�g����Ȃ�
// GG129004(S) R.Endo 2024/12/10 �d�q�̎��ؑΉ�
		// RT�̎��؃f�[�^�p�̃f�[�^��ݒ�
		switch ( addFlag ) {
		case 1:	// �������X�V
		case 2:	// ������ǉ�
			// ���� ���ԏꇂ
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[0] =
				((pFeeCalc->stDiscountInfo[i].DiscParkNo >> 16) & 0x000000FF);
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[1] =
				((pFeeCalc->stDiscountInfo[i].DiscParkNo >> 8) & 0x000000FF);
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[2] =
				 (pFeeCalc->stDiscountInfo[i].DiscParkNo & 0x000000FF);

			// ���� ���
			RTReceipt_Data.receipt_info.arrstDiscount[i].Kind =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// ���� �敪
			RTReceipt_Data.receipt_info.arrstDiscount[i].CardType =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 2 ) {	// �����ς�
				// ���� �g�p��
				RTReceipt_Data.receipt_info.arrstDiscount[i].UsageCount = 0;

				// ���� �O��g�p��
				RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;
			} else {
				// ���� �g�p��
				RTReceipt_Data.receipt_info.arrstDiscount[i].UsageCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;

				// ���� �O��g�p��
				RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageCount = 0;
			}

			// ���� �����z
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisAmount =
				pFeeCalc->stDiscountInfo[i].DiscAmount;

			// ���� �O�񊄈��z
			RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageDisAmount =
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

			// ���� ��ʐ؊����
			RTReceipt_Data.receipt_info.arrstDiscount[i].FeeKindSwitchSetting =
				pFeeCalc->stDiscountInfo[i].FeeKindSwitchSetting;

			// ���� �������1
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisType =
				pFeeCalc->stDiscountInfo[i].DiscInfo;

			// ���� �������2
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisType2 =
				pFeeCalc->stDiscountInfo[i].DiscTime +
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscTime;

			// ���� �K�i������ �ېőΏ�
			RTReceipt_Data.receipt_info.arrstDiscount[i].EligibleInvoiceTaxable = 0;

			break;
		case 3:	// �������z���v��ǉ�
			// ���� ���ԏꇂ
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[0] =
				((pFeeCalc->stDiscountInfo[i].DiscParkNo >> 16) & 0x000000FF);
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[1] =
				((pFeeCalc->stDiscountInfo[i].DiscParkNo >> 8) & 0x000000FF);
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[2] =
				 (pFeeCalc->stDiscountInfo[i].DiscParkNo & 0x000000FF);

			// ���� ���
			RTReceipt_Data.receipt_info.arrstDiscount[i].Kind =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// ���� �敪
			RTReceipt_Data.receipt_info.arrstDiscount[i].CardType =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 2 ) {	// �����ς�
				// ���� �g�p��
				RTReceipt_Data.receipt_info.arrstDiscount[i].UsageCount = 0;

				// ���� �O��g�p��
				RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;
			} else {
				// ���� �g�p��
				RTReceipt_Data.receipt_info.arrstDiscount[i].UsageCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;

				// ���� �O��g�p��
				RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageCount = 0;
			}

			// ���� �����z
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisAmount =
				pFeeCalc->stDiscountInfo[i].DiscAmount;

			// ���� �O�񊄈��z
			RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageDisAmount =
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

			// ���� ��ʐ؊����
			RTReceipt_Data.receipt_info.arrstDiscount[i].FeeKindSwitchSetting =
				pFeeCalc->stDiscountInfo[i].FeeKindSwitchSetting;

			// ���� �������1
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisType =
				pFeeCalc->stDiscountInfo[i].DiscInfo;

			// ���� �������2
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisType2 = 0;

			// ���� �K�i������ �ېőΏ�
			RTReceipt_Data.receipt_info.arrstDiscount[i].EligibleInvoiceTaxable = 0;

			break;
		case 0:	// �ǉ����X�V���Ȃ��ꍇ
		default:
			break;
		}
// GG129004(E) R.Endo 2024/12/10 �d�q�̎��ؑΉ�
#endif	// �Q�l�FOnlineDiscount
	}

	return 0;
}

// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ���u���Z(���Ɏ����w��) �����ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: �����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_discount_set_remote_time()
{
	// ����VL�J�[�h�f�[�^
	vl_now = V_DIS_TIM;	// ���u���Z������(���Ɏ����w��)

	// �����f�[�^�o�b�t�@�m�ݒ�
	ryo_buf_n_set();

#if 1	// �Q�l�Fet02(et95)
	// �����z�𒴂���ꍇ�͐����z���������z�Ƃ���
	if ( g_PipCtrl.stRemoteTime.Discount > ryo_buf_n.require ) {
		g_PipCtrl.stRemoteTime.Discount = ryo_buf_n.require;
	}

	// �����z
	ryo_buf_n.dis = g_PipCtrl.stRemoteTime.Discount;
#endif	// �Q�l�Fet02(et95)

	// �����f�[�^�o�b�t�@�m�v�Z
	ryo_buf_n_calc();

	// �����f�[�^�o�b�t�@�m�擾
	ryo_buf_n_get(0);	// �ʏ�

#if 1	// �Q�l�Fryo_cal
	// �i�[�ł��錏���𒴂����ꍇ�͊i�[���Ȃ�
	if ( CardUseSyu < WTIK_USEMAX ) {
		// ���ԏꇂ
		PayData.DiscountData[CardUseSyu].ParkingNo =
			(ulong)prm_get(COM_PRM, S_SYS, 1, 6, 1);

		// �������
		PayData.DiscountData[CardUseSyu].DiscSyu =
			g_PipCtrl.stRemoteTime.DiscountKind;

		// �����敪
		PayData.DiscountData[CardUseSyu].DiscNo =
			g_PipCtrl.stRemoteTime.DiscountType;

		// ����g�p�����������z
		PayData.DiscountData[CardUseSyu].Discount =
			ryo_buf.waribik;

		// �������1
		PayData.DiscountData[CardUseSyu].DiscInfo1 =
			g_PipCtrl.stRemoteTime.DiscountInfo1;

		// �������2
		PayData.DiscountData[CardUseSyu].uDiscData.common.DiscInfo2 =
			g_PipCtrl.stRemoteTime.DiscountInfo2;

		// ����g�p��������
		PayData.DiscountData[CardUseSyu].DiscCount =
			(uchar)g_PipCtrl.stRemoteTime.DiscountCnt;

		// ��������
		switch ( g_PipCtrl.stRemoteTime.DiscountKind ) {
		case NTNET_SVS_M:			// �T�[�r�X������(���z)
		case NTNET_KAK_M:			// �X����(���z)
		case NTNET_TKAK_M:			// ���X�܊���(���z)
		case NTNET_SHOP_DISC_AMT:	// ��������(���z)
		case NTNET_SYUBET:			// ��ʊ���(���z)
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 1;	// ���Ԋ���
			break;
		case NTNET_SVS_T:			// �T�[�r�X������(����)
		case NTNET_KAK_T:			// �X����(����)
		case NTNET_TKAK_T:			// ���X�܊���(����)
		case NTNET_SHOP_DISC_TIME:	// ��������(����)
		case NTNET_SYUBET_TIME:		// ��ʊ���(����)
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 2;	// ��������
			break;
		default:
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 0;	// �����Ȃ�
			break;
		}

		// NT-NET���Z���f�[�^�p�ۑ��̈�ɕۑ�
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DPARKINGNO, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DSYU, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DNO, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DCOUNT, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DISCOUNT, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO1, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO2, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DROLE, CardUseSyu);

		// �����������Z
		CardUseSyu++;
	}
#endif	// �Q�l�Fryo_cal

	// �����f�[�^�o�b�t�@�v�Z
	ryo_buf_calc();

#if 1	// �Q�l�Fop_mod02
	// ���������T�u�F���u���Z�����w�萸�Z�v��
	in_mony(OPE_REQ_REMOTE_CALC_TIME, 0);
#endif	// �Q�l�Fop_mod02

	return 0;	// ��������
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ���u���Z(���z�w��) �����ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: �����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_discount_set_remote_fee()
{
	// ����VL�J�[�h�f�[�^
	vl_now = V_DIS_FEE;	// ���u���Z������(���z�w��)

	// �����f�[�^�o�b�t�@�m�ݒ�
	ryo_buf_n_set();

#if 1	// �Q�l�Fet02(et95)
	// �����z�𒴂���ꍇ�͐����z���������z�Ƃ���
	if ( g_PipCtrl.stRemoteFee.Discount > ryo_buf_n.require ) {
		g_PipCtrl.stRemoteFee.Discount = ryo_buf_n.require;
	}

	// �����z
	ryo_buf_n.dis = g_PipCtrl.stRemoteFee.Discount;
#endif	// �Q�l�Fet02(et95)

	// �����f�[�^�o�b�t�@�m�v�Z
	ryo_buf_n_calc();

	// �����f�[�^�o�b�t�@�m�擾
	ryo_buf_n_get(0);	// �ʏ�

#if 1	// �Q�l�Fryo_cal
	// �i�[�ł��錏���𒴂����ꍇ�͊i�[���Ȃ�
	if ( CardUseSyu < WTIK_USEMAX ) {
		// ���ԏꇂ
		PayData.DiscountData[CardUseSyu].ParkingNo =
			(ulong)prm_get(COM_PRM, S_SYS, 1, 6, 1);

		// �������
		PayData.DiscountData[CardUseSyu].DiscSyu =
			g_PipCtrl.stRemoteFee.DiscountKind;

		// �����敪
		PayData.DiscountData[CardUseSyu].DiscNo =
			g_PipCtrl.stRemoteFee.DiscountType;

		// ����g�p�����������z
		PayData.DiscountData[CardUseSyu].Discount =
			ryo_buf.waribik;

		// �������1
		PayData.DiscountData[CardUseSyu].DiscInfo1 =
			g_PipCtrl.stRemoteFee.DiscountInfo1;

		// �������2
		PayData.DiscountData[CardUseSyu].uDiscData.common.DiscInfo2 =
			g_PipCtrl.stRemoteFee.DiscountInfo2;

		// ����g�p��������
		PayData.DiscountData[CardUseSyu].DiscCount =
			(uchar)g_PipCtrl.stRemoteFee.DiscountCnt;

		// ��������
		switch ( g_PipCtrl.stRemoteFee.DiscountKind ) {
		case NTNET_SVS_M:			// �T�[�r�X������(���z)
		case NTNET_KAK_M:			// �X����(���z)
		case NTNET_TKAK_M:			// ���X�܊���(���z)
		case NTNET_SHOP_DISC_AMT:	// ��������(���z)
		case NTNET_SYUBET:			// ��ʊ���(���z)
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 1;	// ���Ԋ���
			break;
		case NTNET_SVS_T:			// �T�[�r�X������(����)
		case NTNET_KAK_T:			// �X����(����)
		case NTNET_TKAK_T:			// ���X�܊���(����)
		case NTNET_SHOP_DISC_TIME:	// ��������(����)
		case NTNET_SYUBET_TIME:		// ��ʊ���(����)
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 2;	// ��������
			break;
		default:
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 0;	// �����Ȃ�
			break;
		}

		// NT-NET���Z���f�[�^�p�ۑ��̈�ɕۑ�
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DPARKINGNO, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DSYU, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DNO, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DCOUNT, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DISCOUNT, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO1, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO2, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DROLE, CardUseSyu);

		// �����������Z
		CardUseSyu++;
	}
#endif	// �Q�l�Fryo_cal

	// �����f�[�^�o�b�t�@�v�Z
	ryo_buf_calc();

#if 1	// �Q�l�Fop_mod02
	// ���������T�u�F���u���Z���z�w�萸�Z�v��
	in_mony(OPE_REQ_REMOTE_CALC_FEE, 0);
#endif	// �Q�l�Fop_mod02

	return 0;	// ��������
}
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z �d�q�}�l�[�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
///	@note			�O������F�����ݒ�(cal_cloud_fee_set)�����{�ς�
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_emoney_set()
{
	// ����VL�J�[�h�f�[�^
	vl_now = V_EMY;	// �d�q�}�l�[

	// �����f�[�^�o�b�t�@�m�ݒ�
	ryo_buf_n_set();

#if 1	// �Q�l�Fet02(et94)
	// �d�q�}�l�[�����z�v
	ryo_buf.emonyin += e_pay;

	// �d�q�}�l�[�g�p��
	e_incnt++;

	// �����z
	ryo_buf_n.require = 0;
#endif	// �Q�l�Fet02(et94)

// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6573 VP6800�Œ��ԗ����𐸎Z���A���Z��ʂ́u���Z�c���E��ԕ\���G���A�v���u���Ɓ@0�~�v�ɂȂ�Ȃ�
// 	// �����f�[�^�o�b�t�@�m�v�Z
// 	ryo_buf_n_calc();
	// �d�q�}�l�[��ryo_buf_n_calc�֐��ɂ����鏈�������{���Ȃ��B
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6573 VP6800�Œ��ԗ����𐸎Z���A���Z��ʂ́u���Z�c���E��ԕ\���G���A�v���u���Ɓ@0�~�v�ɂȂ�Ȃ�

	// �����f�[�^�o�b�t�@�m�擾
	ryo_buf_n_get(0);	// �ʏ�

	// �����f�[�^�o�b�t�@�v�Z
	ryo_buf_calc();

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z QR�R�[�h���ϐݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
///	@note			�O������F�����ݒ�(cal_cloud_fee_set)�����{�ς�
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_eqr_set()
{
	// ����VL�J�[�h�f�[�^
	vl_now = V_EQR;	// QR�R�[�h����

	// �����f�[�^�o�b�t�@�m�ݒ�
	ryo_buf_n_set();

#if 1	// �Q�l�Fet02(et94)
	// �d�q�}�l�[�����z�v
	ryo_buf.emonyin += e_pay;

	// �d�q�}�l�[�g�p��
	e_incnt++;

	// �����z
	ryo_buf_n.require = 0;
#endif	// �Q�l�Fet02(et94)

// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6573 VP6800�Œ��ԗ����𐸎Z���A���Z��ʂ́u���Z�c���E��ԕ\���G���A�v���u���Ɓ@0�~�v�ɂȂ�Ȃ�
// 	// �����f�[�^�o�b�t�@�m�v�Z
// 	ryo_buf_n_calc();
	// QR�R�[�h���ς�ryo_buf_n_calc�֐��ɂ����鏈�������{���Ȃ��B
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6573 VP6800�Œ��ԗ����𐸎Z���A���Z��ʂ́u���Z�c���E��ԕ\���G���A�v���u���Ɓ@0�~�v�ɂȂ�Ȃ�

	// �����f�[�^�o�b�t�@�m�擾
	ryo_buf_n_get(0);	// �ʏ�

	// �����f�[�^�o�b�t�@�v�Z
	ryo_buf_calc();

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z �N���W�b�g�J�[�h�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ����ݒ茋��<br>
///							  0 = ��������<br>
///							  1 = �������s<br>
///	@note			�O������F�����ݒ�(cal_cloud_fee_set)�����{�ς�
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_credit_set()
{
	// ����VL�J�[�h�f�[�^
	vl_now = V_CRE;	// �N���W�b�g�J�[�h

	// �����f�[�^�o�b�t�@�m�ݒ�
	ryo_buf_n_set();

#if 1	// �Q�l�Fet02(et90)
	// �����z
	ryo_buf_n.require = 0;
#endif	// �Q�l�Fet02(et90)

// GG124100(S) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6573 VP6800�Œ��ԗ����𐸎Z���A���Z��ʂ́u���Z�c���E��ԕ\���G���A�v���u���Ɓ@0�~�v�ɂȂ�Ȃ�
// 	// �����f�[�^�o�b�t�@�m�v�Z
// 	ryo_buf_n_calc();
	// �N���W�b�g��ryo_buf_n_calc�֐��ɂ����鏈�������{���Ȃ��B
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6573 VP6800�Œ��ԗ����𐸎Z���A���Z��ʂ́u���Z�c���E��ԕ\���G���A�v���u���Ɓ@0�~�v�ɂȂ�Ȃ�

	// �����f�[�^�o�b�t�@�m�擾
	ryo_buf_n_get(0);	// �ʏ�

	// �����f�[�^�o�b�t�@�v�Z
	ryo_buf_calc();

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z �����v�Z���ʃG���[�A���[���`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		uchar	: �����v�Z����<br>
///	@return			void
///	@note	�����v�Z���ʂ��`�F�b�N���A�G���[�A���[����o�^����B<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void pay_result_error_alarm_check(ushort shPayResult)
{
	switch( shPayResult ) {
	case 1:		// �����v�Z�ݒ�p�����[�^�s��(�����ݒ��񂪌�����܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_PAY_SET_ERR, 2);			// �����ݒ�ُ�
		break;
	case 2:		// �����v�Z�ݒ�p�����[�^�s��(�������񂪌�����܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_PASS_SET_ERR, 2);			// ������ݒ�ُ�
		break;
	case 3:		// �����v�Z�ݒ�p�����[�^�s��(�����p�^�[����񂪌�����܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_PAYPATTERN_SET_ERR, 2);		// �����p�^�[���ݒ�ُ�
		break;
	case 4:		// �����v�Z�ݒ�p�����[�^�s��(���Ԏ��ԍő嗿����񂪌�����܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_MAXCHARGE_SET_ERR, 2);		// ���Ԏ��ԍő嗿���ݒ�ُ�
		break;
	case 5:		// �����v�Z�ݒ�p�����[�^�s��(���ԑя�񂪌�����܂���
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_TIMEZONE_SET_ERR, 2);		// ���ԑѐݒ�ُ�
		break;
	case 6:		// �����v�Z�ݒ�p�����[�^�s��(�P�����Ԃ�0���̂��߁A�v�Z�ł��܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_UNITTIME_SET_ERR, 2);		// �P�ʎ��Ԑݒ�ُ�
		break;
	case 7:		// �����v�Z�ݒ�p�����[�^�s��(�T�[�r�X����񂪌�����܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_SVS_SET_ERR, 2);			// �T�[�r�X���ݒ�ُ�
		break;
	case 8:		// �����v�Z�ݒ�p�����[�^�s��(�X������񂪌�����܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_KAK_SET_ERR, 2);			// �X�����ݒ�ُ�
		break;
	case 9:		// �����v�Z�ݒ�p�����[�^�s��(���X�܊�����񂪌�����܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_TKAK_SET_ERR, 2);			// ���X�܊����ݒ�ُ�
		break;
	case 10:	// �����v�Z�ݒ�p�����[�^�s��(PMC�T�[�r�X����񂪌�����܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_PMCSVS_SET_ERR, 2);			// PMC�T�[�r�X���ݒ�ُ�
		break;
	case 11:	// �����v�Z�ݒ�p�����[�^�s��(�Ԏ튄����񂪌�����܂���)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_SYUBET_SET_ERR, 2);			// �Ԏ튄���ݒ�ُ�
		break;
	case 12:	// �����v�Z�ݒ�p�����[�^�s��(��������񂪌�����܂���
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_WRI_SET_ERR, 2);			// �������ݒ�ُ�
		break;
	case 21:	// �����v�Z�v���f�[�^�s��(���Z�����������f�[�^�ł�)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_PAYDATE_INVALID, 2, 0);		// ���Z��������
		break;
	case 22:	// �����v�Z�v���f�[�^�s��([���񐸎Z]���ɓ����������f�[�^�ł�)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_ENTRYDATE_INVALID, 2, 0);		// ���ɓ��������i����j
		break;
	case 23:	// �����v�Z�v���f�[�^�s��([�Đ��Z][���ɂ���Čv�Z]���ɓ����������f�[�^�ł�)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_ENTRYDATE_INVALID_OLD, 2, 0);	// ���ɓ��������i�Đ��Z�j
		break;
	case 24:	// �����v�Z�v���f�[�^�s��([�Đ��Z][�O�񐸎Z����v�Z]�O�񐸎Z�����������f�[�^�ł�)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_PAYDATE_INVALID_OLD, 2, 0);	// �O�񐸎Z���������i�Đ��Z�j
		break;
	case 41:	// �����v�Z�s��(���ɓ�������̒��Ԏ��Ԃ�{0}�N�𒴂��Ă��܂�)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_OVER1YEARCAL, 2, 0);			// 1�N�ȏ㗿���v�Z�i���Ɂj
		break;
	case 42:	// �����v�Z�s��(�O�񐸎Z��������̒��Ԏ��Ԃ�{0}�N�𒴂��Ă��܂�)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_OVER1YEARCAL_OLD, 2, 0);		// 1�N�ȏ㗿���v�Z�i�O�񐸎Z�j
		break;
	case 64:	// �����֘A�G���[(���Ή��̊������܂܂�Ă��܂�)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_UNSUPPORTED_DISC_USE, 2);	// ���Ή������g�p
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ����������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	1���Z���̏������������鏈���B<br>
///			�ʏ헿���v�Z��ryo_cal(cal/ryo_cal.c)�̈ꕔ(��)�ɑ����B<br>
///			��ryo_cal�̐擪�B�ڍה͈͂�ryo_cal���̃R�����g�Q�ƁB<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void fee_init()
{
	stZaishaInfo_t *pZaisha = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo;	// �ݎԏ��
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����
	int i;

	// ������
	memset(&ryo_buf, 0, sizeof(ryo_buf));						// �����f�[�^�o�b�t�@�FRYO_BUF
	memset(&PayData, 0, sizeof(PayData));						// ���Z���,�̎��؈󎚃f�[�^�FReceipt_data
	memset(&ntNet_56_SaveData, 0, sizeof(ntNet_56_SaveData));	// NT-NET���Z�f�[�^�p�ۑ��̈�FntNet_56_saveInf
	NTNET_Data152_SaveDataClear();								// NT-NET���Z���f�[�^�p�ۑ��̈�FntNet_152_saveInf
	memset(&OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo));	// NT-NET���Z�f�[�^�t�����Ft_OpeNtnetAddedInfo
	memset(card_use, 0, sizeof(card_use));						// �J�[�h���g�p�����e�[�u���Fuchar

	// GT-4100���g�p
	memset(&PPC_Data_Detail, 0, sizeof(PPC_Data_Detail));		// �v���y�C�h�J�[�h�f�[�^�ڍׁFt_PPC_DATA_DETAIL
	c_pay = 0;													// �v���y�C�h�J�[�h�g�p�z�Flong
	ntnet_nmax_flg = 0;											// �ő嗿���z������t���O�Fchar
	memset(&PayData_Sub, 0, sizeof(PayData_Sub));				// �󎚗p�ڍג��~�G���A�FReceipt_Data_Sub
	discount_tyushi = 0;										// �����z(���Z���~�@�\����)�Flong
	Flap_Sub_Num = 0;											// �ڍג��~�G���A�ԍ��Fuchar

	// �N���E�h�����v�Z���g�p
	memset(&req_rkn, 0, sizeof(req_rkn));						// �����v���e�[�u���FREQ_RKN
	memset(req_rhs, 0, sizeof(req_rhs));						// ���݌v�Z��i�[��FREQ_RKN
	tb_number = 0;												// �v�Z���ʊi�[�e�[�u���ʂ��ԍ��Fshort
	rhspnt = 0;													// ���݊i�[���Fchar
	memset(req_crd, 0, sizeof(req_crd));						// ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@�FREQ_CRD
	req_crd_cnt = 0;											// ���Ԍ��ǂݎ�莞���f�[�^�v�������Fchar
	memset(req_tkc, 0, sizeof(req_tkc));						// ��������~���Z�v���i�[�o�b�t�@�FREQ_TKC
	req_tkc_cnt = 0;											// ��������~���Z�v���i�[�����Fchar

// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
	// ���������
	season_init();
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�

	// ����������
	discount_init();

	// ���Z�������Ԉʒu�ԍ�
	ryo_buf.pkiti = OPECTL.Pr_LokNo;	// ���������p���Ԉʒu�ԍ�

	// ���O�^�C���I�[�o�[����
	if ( FLAPDT.flp_data[(OPECTL.Pr_LokNo - 1)].lag_to_in.BIT.LAGIN == ON ) {
		ryo_buf.lag_tim_over = 1;	// ���O�^�C���I�[�o�[
	}

// GG129000(S) H.Fujinaga 2023/01/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//	// ���O�^�C���I�[�o�[
//	if ( ryo_buf.lag_tim_over == 1 ) {
	// ���O�^�C���I�[�o�[ or ���Z���ύX�f�[�^��M��
	if ( ryo_buf.lag_tim_over == 1 || PayInfoChange_StateCheck() == 1 ) {
// GG129000(E) H.Fujinaga 2023/01/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
		// �O��x���z(����������)
		ryo_buf.zenkai = pZaisha->GenkinFee;
		for ( i = 0; i < ONL_MAX_SETTLEMENT; i++ ) {
			if ( pZaisha->stSettlement[i].CargeType ) {	// ���ϋ敪����
				ryo_buf.zenkai += pZaisha->stSettlement[i].CargeFee;	// ���ϋ��z
			}
		}

		// �O�񊄈��z(�x���z������)
		for ( i = 0; i < ONL_MAX_DISC_NUM; i++ ) {
			if ( !(pFeeCalc->stDiscountInfo[i].DiscParkNo) ) {
				break;
			}
			ryo_buf.zenkaiWari += pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;	// �O�񐸎Z�܂ł̎g�p�ς݊������z(����)
		}
// GG129000(S) ���P�A��No.69,No.73 ���Z�ς݂̍ݎԂ��ݎԑ��M�������ɐ��Z���i���O���Z�F�����ς݁i�����܂ށj�j���iGM803002���p�j
		if (ryo_buf.zenkai > 0 || ryo_buf.zenkaiWari > 0) {
			// �I�y���[�V�����t�F�[�Y
			if ( OPECTL.op_faz == 0 ) {	// �������M
				OPECTL.op_faz = 1;	// ������
			}
		}
// GG129000(E) ���P�A��No.69,No.73 ���Z�ς݂̍ݎԂ��ݎԑ��M�������ɐ��Z���i���O���Z�F�����ς݁i�����܂ށj�j���iGM803002���p�j
	}

	// NT-NET�ݒ�F�f�[�^�`���ݒ� �E�f�[�^�`�� ���ԏ�Z���^�[�`��(�Œ�)
	if ( prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0 ) {
		// ���Z���f�[�^�p�����敪
		PayInfo_Class = 9;	// ���Z�r��

		// �ݎԏ��̐��Z�����ɂ͖����Z�o�ɐ��Z���͏o�ɓ����A
		// ���O���Z���͐��Z�J�n�������ݒ肳��Ă���B
		// �Q�l�Fset_tim_only_out_card
		NTNetTime_152_wk.Year = pZaisha->dtPaymentDateTime.dtTimeYtoSec.shYear;
		NTNetTime_152_wk.Mon = pZaisha->dtPaymentDateTime.dtTimeYtoSec.byMonth;
		NTNetTime_152_wk.Day = pZaisha->dtPaymentDateTime.dtTimeYtoSec.byDay;
		NTNetTime_152_wk.Hour = pZaisha->dtPaymentDateTime.dtTimeYtoSec.byHours;
		NTNetTime_152_wk.Min = pZaisha->dtPaymentDateTime.dtTimeYtoSec.byMinute;
		NTNetTime_152_wk.Sec = pZaisha->dtPaymentDateTime.dtTimeYtoSec.bySecond;
	}

// GG124100(S) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// 	// �Ԕԏ��
// 	PayData.CarSearchFlg = vl_car_no.CarSearchFlg;			// �Ԕ�/�����������
// 	memcpy(PayData.CarSearchData, vl_car_no.CarSearchData,
// 		sizeof(PayData.CarSearchData));						// �Ԕ�/���������f�[�^
// 	memcpy(PayData.CarNumber, vl_car_no.CarNumber,
// 		sizeof(PayData.CarNumber));							// �Z���^�[�₢���킹���̎Ԕ�
// 	memcpy(PayData.CarDataID, vl_car_no.CarDataID,
// 		sizeof(PayData.CarDataID));							// �Ԕԃf�[�^ID(�⍇���Ɏg�p����ID)
// 
// // GG124100(S) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6521 ��ʐ؊���̎Ԏ킪�̎��؂␸�Z�f�[�^�ɔ��f����Ȃ�
// 	// �������
// 	syashu = pFeeCalc->FeeType;
// // GG124100(E) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6521 ��ʐ؊���̎Ԏ킪�̎��؂␸�Z�f�[�^�ɔ��f����Ȃ�
// GG124100(E) R.Endo 2022/08/26 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
}

// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ���������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	1���Z���̒���֘A�������������鏈���B<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void season_init()
{
	// ������
	memset(&PayData.teiki, 0, sizeof(PayData.teiki));				// ����f�[�^�Fteiki_use

	// �J�[�h���g�p�����e�[�u���͒������������
	card_use[USE_PAS] = 0;
}
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ����������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	1���Z���̊����֘A�������������鏈���B<br>
///			�ʏ헿���v�Z��OnlineDiscount(ope/opesub.c)�̈ꕔ(��)�ɑ����B<br>
///			��OnlineDiscount�̐擪�t�߁B�ڍה͈͂�OnlineDiscount���̃R�����g�Q�ƁB<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void discount_init()
{
	int i;

	// ������
	memset(PayData.DiscountData, 0, sizeof(PayData.DiscountData));	// ���Z���,�̎��؈󎚃f�[�^�FDISCOUNT_DATA
// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// 	PayData.SyuWariRyo = 0;											// ��ʊ��������Fulong
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
	memset(RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0,
		sizeof(RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo));	// RT���Z�f�[�^���O�FstDiscount_t
	NTNET_Data152_DiscDataClear();									// NT-NET���Z���f�[�^�p�ۑ��̈� �����ڍ׃G���A
	NTNET_Data152_DetailDataClear();								// NT-NET���Z���f�[�^�p�ۑ��̈� ���׃G���A
	memset(card_use2, 0, sizeof(card_use2));						// �T�[�r�X�����g�p�����e�[�u���Fuchar
	CardUseSyu = 0;													// 1���Z�̊�����ނ̌����Fuchar
// GG129000(S) R.Endo 2024/01/12 #7217 ���������v�\���Ή�
	ryo_buf.shopping_total = 0;										// �����������z���v�Funsigned long
// GG129000(E) R.Endo 2024/01/12 #7217 ���������v�\���Ή�
// GG129004(S) M.Fujikawa 2024/10/22 �������z�������Ή�
	ryo_buf.shopping_info = 0;										// �����������z���v�Funsigned long
// GG129004(E) M.Fujikawa 2024/10/22 �������z�������Ή�

	// �J�[�h���g�p�����e�[�u���͒��Ԍ��E������ȊO��������
	for ( i = 0; i < USE_MAX; i++ ) {
		switch ( i ) {
		case USE_TIK:	// ���Ԍ�
		case USE_PAS:	// �����
			break;
		default:
			card_use[i] = 0;
			break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z �����f�[�^�o�b�t�@�m�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	ryo_buf�̒l��ryo_buf_n�ɐݒ肷�鏈���B<br>
///			�ʏ헿���v�Z��ryo_cal(cal/ryo_cal.c)�̈ꕔ(��)�ɑ����B<br>
///			��ryo_cal��ec09(cal/ec09.c)�O�B�ڍה͈͂�ryo_cal���̃R�����g�Q�ƁB<br>
///			<br>
///			�ʏ헿���v�Z����̕ύX�_(�ׂ��ȃ��t�@�N�^�����O������)<br>
///			�E�����z�Ə���Ŋz�̐ݒ���ǉ��B<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void ryo_buf_n_set()
{
	// �����z
	switch ( ryo_buf.ryo_flg ) {
	case 2:	// �ʏ퐸�Z(�������)
	case 3:	// �C�����Z(�������)(���g�p)
		ryo_buf_n.ryo = ryo_buf.tei_ryo;	// ������ł̒��ԗ���
		break;
	case 0:	// �ʏ퐸�Z(����Ȃ�)
	case 1:	// �C�����Z(����Ȃ�)(���g�p)
	default:
		ryo_buf_n.ryo = ryo_buf.tyu_ryo;	// ���ԗ���
		break;
	}

	// ����Ŋz
	ryo_buf_n.tax = ryo_buf.tax;			// ����Ŋz

	// �����z
	ryo_buf_n.dis = ryo_buf.waribik;		// �����z�v

	// �����z
	ryo_buf_n.nyu = ryo_buf.nyukin;			// ���������z�v

	// �����z
	ryo_buf_n.require = ryo_buf.zankin;		// �\���c����z
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z �����f�[�^�o�b�t�@�m�v�Z
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	ryo_buf_n�̈ꕔ�l���v�Z�ŋ��߂鏈���B<br>
///			�ʏ헿���v�Z��et02(cal/et02.c)�̈ꕔ(��)�ɑ����B<br>
///			��et02�̌㔼�B�ڍה͈͂�et02���̃R�����g�Q�ƁB<br>
///			<br>
///			�ʏ헿���v�Z����̕ύX�_(�ׂ��ȃ��t�@�N�^�����O������)<br>
///			�E�����z���������ɂ��鏈�����e�����v�Z����(et45��)����ړ��B<br>
///			�E����ł�0%(����)�Œ�Ƃ��A0%(����)�ȊO��z�肵�������͍폜�B<br>
///			�E�T�[�r�X���t���O(svcd_dat)�̏����͖��g�p�̂��ߍ폜�B<br>
///			�E�s���z(ryo_buf_n.hus)�̏����͖��g�p�̂��ߍ폜�B<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void ryo_buf_n_calc()
{
	// �����z�Ɗ����z�̌v�Z
	if ( ryo_buf_n.ryo < ryo_buf_n.dis ) {		// �����z��芄���z���傫��
		ryo_buf_n.require = 0;

		// �����v�Z��{�ݒ�F�����z �E�����z �������Ƃ���(�Œ�)
		if ( prm_get(COM_PRM, S_CAL, 35, 1, 1) == 1 ) {
			// �����z���������ɂ���
			ryo_buf_n.dis = ryo_buf_n.ryo;
		}
	} else {
		ryo_buf_n.require = ryo_buf_n.ryo - ryo_buf_n.dis;
	}

	// �����z�ƃv���y�C�h�J�[�h�g�p�z(���g�p)�̌v�Z
	if ( ryo_buf_n.require < c_pay ) {			// �����z���v���y�C�h�J�[�h�g�p�z���傫��
		ryo_buf_n.require = 0;
	} else {
		ryo_buf_n.require -= c_pay;
	}

	// �����z�Ɠ����z�̌v�Z
	if ( ryo_buf_n.require < ryo_buf_n.nyu ) {	// �����z�������z���傫��
		ryo_buf_n.require = 0;
	} else {
		ryo_buf_n.require -= ryo_buf_n.nyu;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z �����f�[�^�o�b�t�@�m�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		uchar			: ����t���O<br>
///									  0 = �ʏ�<br>
///									  1 = ����ݒ�<br>
///	@return			void
///	@note	ryo_buf�̒l��ryo_buf_n����擾���鏈���B<br>
///			�ʏ헿���v�Z��ryo_cal(cal/ryo_cal.c)�̈ꕔ(��)�ɑ����B<br>
///			��ryo_cal��et02(cal/et02.c)��B�ڍה͈͂�ryo_cal���̃R�����g�Q�ƁB<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void ryo_buf_n_get(uchar seasonFlg)
{
	// ����g�p�������Z
	if ( seasonFlg ) {
		ryo_buf.ryo_flg += 2;				// �������
	}

	switch ( ryo_buf.ryo_flg ) {
	case 2:	// �ʏ퐸�Z(�������)
	case 3:	// �C�����Z(�������)(���g�p)
		// ������������
		ryo_buf.pas_syu = syashu;			// �Ԏ�

		// ������ł̒��ԗ���
		ryo_buf.tei_ryo = ryo_buf_n.ryo;	// �����z

		break;
	case 0:	// �ʏ퐸�Z(����Ȃ�)
	case 1:	// �C�����Z(����Ȃ�)(���g�p)
	default:
		// ���Ԍ��������
		ryo_buf.tik_syu = syashu;			// �Ԏ�

		// ���ԗ���
		ryo_buf.tyu_ryo = ryo_buf_n.ryo;	// �����z

		break;
	}

	// ����Ŋz
	ryo_buf.tax = ryo_buf_n.tax;			// ����Ŋz

	// �����z�v
	ryo_buf.waribik = ryo_buf_n.dis;		// �����z

	// �\���c����z(�d�q�}�l�[�����z�v�������Ă��Ȃ�)
	ryo_buf.zankin = ryo_buf_n.require;		// �����z

	// �������
	ryo_buf.syubet = (char)(syashu - 1);
}

//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z �����f�[�^�o�b�t�@�v�Z
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	ryo_buf�̈ꕔ�l���v�Z�ŋ��߂鏈���B<br>
///			�ʏ헿���v�Z��ryo_cal(cal/ryo_cal.c)�̈ꕔ(��)�ɑ����B<br>
///			��ryo_cal�̖����B�ڍה͈͂�ryo_cal���̃R�����g�Q�ƁB<br>
///			<br>
///			�ʏ헿���v�Z����̕ύX�_(�ׂ��ȃ��t�@�N�^�����O������)<br>
///			�E�������ƒ���Ȃ��̏����������ϐ����݂̂̂��ߋ��ʉ��B<br>
///			�E����ł�0%(����)�Œ�Ƃ��A0%(����)�ȊO��z�肵�������͍폜�B<br>
///			�E�C�����Z�͔�Ή��̂��ߏC�����Z�p�̏����͍폜�B<br>
///			�E�U�֐��Z�͔�Ή��̂��ߐU�֐��Z�p�̏����͍폜�B<br>
///			�E�N���W�b�g���Z���ʂ̏����𑼂Ƌ��ʉ��B<br>
///			�@(�⑫�FCREDIT_CARD�ȊO��cre_type�͖��g�p)<br>
///			�E�d�q�}�l�[�L���ɂ�鏈���������������ߋ��ʉ��B<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void ryo_buf_calc()
{
	ulong ryoTemp;
	ulong sottaxTemp;
	ulong wariTemp;
	ulong overTemp;

	// ���ԗ���
	switch ( ryo_buf.ryo_flg ) {
	case 2:	// �ʏ퐸�Z(�������)
	case 3:	// �C�����Z(�������)(���g�p)
		ryoTemp = ryo_buf.tei_ryo;
		break;
	case 0:	// �ʏ퐸�Z(����Ȃ�)
	case 1:	// �C�����Z(����Ȃ�)(���g�p)
	default:
		ryoTemp = ryo_buf.tyu_ryo;
		break;
	}

	// �ېőΏۊz
	if ( ryoTemp < ryo_buf.waribik ) {	// ���ԗ�����芄���z�v���傫��
		ryo_buf.kazei = 0;
	} else {
		ryo_buf.kazei = ryoTemp - ryo_buf.waribik;
	}

	// �O�Ŋz
	sottaxTemp = 0;	// ���ŌŒ�

	// �\������(�d�q�}�l�[�����z�v�������Ă���)�ƒޑK
	if ( (ryo_buf.zankin == 0) &&					// �\���c����z����
		 ntnet_decision_credit(&ryo_buf.credit) ) {	// �N���W�b�g���Z
		// �����̂���Ⴂ�ŃN���W�b�g�J�[�h�Ɠ����̍��v���\�������𒴂���ꍇ�ɓ�����߂�
		if ( ryo_buf.dsp_ryo < (ryo_buf.nyukin + ryo_buf.credit.pay_ryo) ) {
			ryo_buf.turisen = (ryo_buf.nyukin + ryo_buf.credit.pay_ryo) - ryo_buf.dsp_ryo;
		}

		// �\������
		if ( ryo_buf.dsp_ryo < ryo_buf.credit.pay_ryo ) {
			ryo_buf.dsp_ryo = 0;
		} else {
			ryo_buf.dsp_ryo = ryo_buf.dsp_ryo - ryo_buf.credit.pay_ryo;
		}
	} else {										// �\���c����z���聕�N���W�b�g���Z�ȊO
		// �����z�v
		wariTemp = ryo_buf.waribik + c_pay + ryo_buf.emonyin;

		// ���������z�v�{�����z�v�����ԗ����{�O�Ŋz�𒴂���ꍇ�ɃI�[�o�[����������߂�
		if ( (ryoTemp + sottaxTemp) < (ryo_buf.nyukin + wariTemp) ) {
			overTemp = (ryo_buf.nyukin + wariTemp) - (ryoTemp + sottaxTemp);

			// �I�[�o�[����������߂�
			if ( overTemp <= ryo_buf.nyukin ) {
				ryo_buf.turisen = ryo_buf.nyukin - overTemp;
			}
		}

		// �\������
		if ( (ryoTemp + sottaxTemp) < wariTemp ) {
			ryo_buf.dsp_ryo = 0;

			// ����������ΑS�z�߂�
			if ( ryo_buf.nyukin > 0 ) {
				ryo_buf.turisen = ryo_buf.nyukin;
			}
		} else {
			ryo_buf.dsp_ryo = (ryoTemp + sottaxTemp) - wariTemp;
		}
	}
}
// GG124100(E) R.Endo 2022/09/14 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// GG129000(S) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
//[]----------------------------------------------------------------------[]
///	@brief			�N���E�h�����v�Z ���ɏ��`�F�b�N(�m�F�̂�)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ���ɏ��`�F�b�N����<br>
///							  0 = �����ݒ��<br>
///							  1 = ���ɏ��NG<br>
///							  2 = ���ɏ��NG(QR����Ȃ�)<br>
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
short cal_cloud_discount_check_only()
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// �����v�Z����

	// �����d�����p�`�F�b�N�̃`�F�b�N
	switch( lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup ) {
	case 0:		// �`�F�b�N�Ȃ�
	case 1:		// �����d���Ȃ�
		break;
	default:	// ���̑�
		return 2;	// ���ɏ��NG(QR����Ȃ�)
	}

	// �������擾���ʂ̃`�F�b�N
	switch( pFeeCalc->CalcResult ) {
	case 0:		// ����
	case 2:		// �����Ȃ�
		break;
	case 1:		// �������s(�Z���^�[�����G���W���G���[)
	default:	// ���̑�
		return 1;	// ���ɏ��NG
	}

	return 0;	// �����ݒ��
}
// GG129000(E) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
