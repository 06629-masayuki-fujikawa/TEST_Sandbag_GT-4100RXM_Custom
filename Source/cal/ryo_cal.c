/*[]----------------------------------------------------------------------------------------------[]*/
/*| �����v�Z�������C��																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2001-11-02																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/
																	/********************************/
#include	<string.h>												/*								*/
#include	"system.h"												/*								*/
#include	"mem_def.h"												/*								*/
#include	"pri_def.h"												/*								*/
#include	"rkn_def.h"												/* �S�f�t�@�C������				*/
#include	"rkn_cal.h"												/* �����֘A�f�[�^				*/
#include	"rkn_fun.h"												/* �S�T�u���[�`���錾			*/
#include	"tbl_rkn.h"												/*								*/
#include	"ope_def.h"												/*								*/
#include	"LKmain.h"												/*								*/
#include	"prm_tbl.h"												/*								*/
#include	"common.h"												/*								*/
#include	"ntnet.h"												/*								*/
#include	"ntnet_def.h"											/*								*/
#include	"flp_def.h"												/*								*/
#include	"Suica_def.h"											/*								*/
#include	"cre_ctrl.h"
#include	"raudef.h"

// GG120600(S) // Phase9 Version�𕪂���
/*[]----------------------------------------------------------------------[]*/
/*| �����v�Z���W���[�� ����                                                |*/
/*[]----------------------------------------------------------------------[]*/
// �Z���^�[�p�[�����f�[�^�ɗ����v�Zdll�o�[�W�������Z�b�g����d�l���V�K�ɒ�`����
// �����v�Zdll�o�[�W�����͊�ƂȂ��Ă��郁�C���v���O�����̃o�[�W�������Z�b�g����
// ���C���v���O�����̃o�[�W�����A�b�v�ŗ����v�Z�G���W���̕ύX���������ꍇ�̂ݗ����v�Zdll�o�[�W�������A�b�v�����d�l�Ƃ���
// �ȉ��ύX����ۂ̗���L��
// �����v�Z�G���W���̕ύX �v���O�����o�[�W���� �����v�Zdll�o�[�W����
// �Ȃ�                   GG120600             MH322918
const	ver_rec		VERSNO_RYOCAL = {
	'0',' ','M','H','8','1','7','8','0','0'				// MH817800
};
// GG120600(E) // Phase9 Version�𕪂���
	
																	/*								*/
const uchar	CHG_PKN[4][4] = {										/*								*/
	{ RY_SKC, RY_SKC_K, RY_SKC_K2, RY_SKC_K3 },						/* ���޽��						*/
	{ RY_KAK, RY_KAK_K, RY_KAK_K2, RY_KAK_K3 },						/* �|����						*/
	{ RY_WBC, RY_WBC_K, RY_WBC_K2, RY_WBC_K3 },						/* ������						*/
	{ RY_FRE, RY_FRE_K, RY_FRE_K2, RY_FRE_K3 },						/* �񐔌�						*/
};																	/*								*/
																	/*								*/
	DISCOUNT_DATA2	wk_DicData;
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �����v�Z�������C��																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ryo_cal( r_knd, num )															   |*/
/*| PARAMETER	: r_knd	; �����v�Z���															   |*/
/*|						;   0:���Ԍ�															   |*/
/*|						;   1:�����															   |*/
/*|						;   2:����߲�޶���														   |*/
/*|						;   3:���޽��															   |*/
/*|						;   4:���~��(���g�p)													   |*/
/*|						;   6:�|�����Ԍ�(���g�p)												   |*/
/*|						;   7:���u���Z(���z�w��)												   |*/
// MH810105(S) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
//// MH321800(S) hosoda IC�N���W�b�g�Ή� (�A���[�����)
///*|						;   9:���E����															   |*/
//// MH321800(E) hosoda IC�N���W�b�g�Ή� (�A���[�����)
// MH810105(E) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
/*|						;  50:�s���o��															   |*/
/*|						; 100:�C�����Z															   |*/
/*|						; 110:�����v�Z�V�~�����[�^												   |*/
/*|						; 111:�����v�Z�V�~�����[�^(���u���Z���Ɏ����w�萸�Z)					   |*/
/*|				  num	; ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-11-02																	   |*/
/*| Update		: 4500EX T.Nakayama																   |*/
/*|       		: ���޽��,�g�����޽��,������Ή�												   |*/
/*|       		: ���Z���~��̃T�[�r�X���Ή��̈ڐA(FCR-P30033 TF4800N DH917004)					   |*/
/*[]------------------------------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/
void	ryo_cal( char r_knd, ushort num )							/*								*/
{																	/*								*/
	ulong	sot_tax = 0L;											/*								*/
	ulong	dat = 0L;												/*								*/
	struct	REQ_RKN		*rhs_p;										/*								*/
	short	calcreqnum	;											/* ���������v�Z�v����			*/
	char	cnt;													/*								*/
	char	i;														/*								*/
	uchar	wrcnt = 0;												/*								*/
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
	uchar	DetailCnt = 0;
// �d�l�ύX(S) K.Onodera 2016/12/05 �U�֊z�ύX
//	ulong	OrgDiscount = 0;
// �d�l�ύX(E) K.Onodera 2016/12/05 �U�֊z�ύX
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
	uchar	pricnt = 0;												/*								*/
	ushort	wk_dat;													/*								*/
	ushort	wk_dat2;												/*								*/
	ulong	wk_dat3;												/*								*/
// MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
	ushort	wk_dat4;												/*								*/
// MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
// MH810100(S) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
	ushort	wk_dat5;												/*								*/
// MH810100(E) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
	ushort	wk_dat6;												/*								*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	ushort	wk_dat7;
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	short	wk_adr;													/* �A�h���X						*/
	short	wk_ses;													/* �p�����[�^�Z�b�V������		*/
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
// MH321800(S) �������8000�̃Z�b�g���@���C��
//	ushort	wk_DiscSyu1;											/* ������ʁFSuica���ޔԍ�		*/
//	ushort	wk_DiscSyu2;											/* ������ʁFSuica�x���z�A�c�z	*/
// MH321800(E) �������8000�̃Z�b�g���@���C��
	uchar	chg_flg = 0;
// MH810100(S) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
	uchar	DiscStatus;
// MH810100(S) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
	struct clk_rec *pTime;


// MH810100(S) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
//	long	svtm,uptm;
// MH810100(E) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
	ulong	wk_pay_ryo;				//�Đ��Z�p�v���y�C�h���z�i�[�G���A

	flp_com	*pflp;
// MH810100(S) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
//	uchar	fusei_cal=0;
// MH810100(E) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)

	struct	CAR_TIM	Adjustment_Beginning;

// MH810105(S) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
//// MH321800(S) hosoda IC�N���W�b�g�Ή� (�A���[�����)
////	�A���[�������Ȃǂ̎c�z�𑊎E���ďo�ɂ�����
////	�v�Z��̓T�[�r�X���܂��͊|�����̊����ɕt���Đ��Z�f�[�^�����W�v����
//	f_sousai = 0;
//	if (r_knd == 9) {
//		f_sousai = 1;		// ���E����
//		r_knd = 3;			// �T�[�r�X���܂��͊|����
//	}
//// MH321800(E) hosoda IC�N���W�b�g�Ή� (�A���[�����)
// MH810105(E) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
	memcpy( &Adjustment_Beginning, &car_ot, sizeof( Adjustment_Beginning ) );

	WACDOG;// RX630��WDG�X�g�b�v���o���Ȃ��d�l�̂���WDG���Z�b�g�ɂ���

	if( r_knd == 50 ){												/* �s���o�ɂɂ�闿���v�Z�H		*/
		r_knd = 0;													/* �׸ނ𐸎Z�J�n���ɍ��킹��	*/
// MH810100(S) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
//		fusei_cal = 1;												/* �s�������v�Z�p���׸ނ��Z�b�g	*/
// MH810100(E) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
	}
	if( r_knd == 110 ){												/* �����v�Z�V�~�����[�^�ɂ�闿���v�Z�H */
		r_knd = 0;													/* �׸ނ𐸎Z�J�n���ɍ��킹��	*/
		chg_flg = 1;												/* �����v�Z�V�~�����[�^�p�ɕϊ�	*/
	}
// MH322914(S) K.Onodera 2016/08/09 AI-V�Ή��F���u���Z(���Ɏ����w��)
	if( r_knd == 111 ){												/* �����v�Z�V�~�����[�^�ɂ�闿���v�Z�H */
		r_knd = 0;													/* �׸ނ𐸎Z�J�n���ɍ��킹��	*/
	}
// MH322914(E) K.Onodera 2016/08/09 AI-V�Ή��F���u���Z(���Ɏ����w��)

// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#if 1	// ��������F�N���E�h�����v�Z��fee_init(cal_cloud/cal_cloud.c)�ɑ����B
		// ���C������ۂ�fee_init�����m�F���邱�ƁB
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// �s��C��(S) K.Onodera 2016/12/08 #1645 �̎��؂̃v���y�C�h�J�[�h�c�z���������󎚂���Ȃ�
//	if((( r_knd == 0 )||( r_knd == 100 ))&&							/* ���Ԍ�or�C�����Z				*/
// MH810100(S) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
//	if((( r_knd == 0 )||( r_knd == 100 )||( r_knd == 7 ))&&			/* ���Ԍ�or�C�����Z				*/
	if((( r_knd == 0 )||( r_knd == 100 )||( r_knd == 7 )||( r_knd == 8 ))&&
// MH810100(E) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
// �s��C��(E) K.Onodera 2016/12/08 #1645 �̎��؂̃v���y�C�h�J�[�h�c�z���������󎚂���Ȃ�
		(ryo_buf.credit.pay_ryo == 0))
	{																/*								*/
		memset( req_rhs, 0, sizeof( struct REQ_RKN )*150 );			/* ���݌v�Z��i�[�ر�ر			*/
		memset( &ryo_buf, 0, sizeof( struct RYO_BUF ) );			/* �����ޯ̧�ر�				*/
		ryo_buf.ryo_flg = 0;										/* �ʏ헿�ྯ�					*/
		ryo_buf.pkiti = num;										/* ���Ԉʒu�ԍ����				*/
		memset( &PayData, 0, sizeof( Receipt_data ) );				/* ���Z���,�̎��؈��ް�		*/
// MH810102(S) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
		memset( &ntNet_56_SaveData, 0, sizeof(ntNet_56_SaveData) );	/* ���Z�f�[�^�ێ�				*/
// MH810102(E) R.Endo 2021/03/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5359 NT-NET���Z�f�[�^�ɃZ�b�g���錔��̕ύX
		NTNET_Data152_SaveDataClear();
		memset( &PPC_Data_Detail, 0, sizeof( PPC_Data_Detail ));	/* ����ߏڍ׈󎚴ر�ر			*/	
		ntnet_nmax_flg = 0;											/*�@�ő嗿���׸޸ر	*/
		if( FLAPDT.flp_data[num-1].lag_to_in.BIT.LAGIN == ON ){
			ryo_buf.lag_tim_over = 1;								/* ���O�^�C���I�[�o�[�t���O�Z�b�g */
		}
// MH810100(S) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
		if(ryo_buf.lag_tim_over == 1){
			ryo_buf.zenkai = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.GenkinFee;
			// ����ɁA���ϋ��z�𑫂�
			for( cnt=0; cnt<ONL_MAX_SETTLEMENT; cnt++ ){
				// ���ϋ敪����H
				if( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeType ){
					ryo_buf.zenkai += lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeFee;
				}
			}
// MH810100(S) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
			ryo_buf.zenkaiWari = 0;
			for( cnt=0; cnt<ONL_MAX_DISC_NUM; cnt++ ){
				// ���ԏꇂ�������Ď�ʂ�999�ȉ�
				if( lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscParkNo && 
// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
// 50/150���́A��΂��B�i�K���v�Z����Ă��܂����߁j
					lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu != NTNET_SYUBET_TIME &&
					lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu != NTNET_SYUBET &&
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
					lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu <= NTNET_SECTION_WARI_MAX){
					// �O�񊄈��z(�x���z������)�̍��v���o��
					ryo_buf.zenkaiWari = ryo_buf.zenkaiWari + lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].UsedDisc;
				}
			}
// MH810100(E) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
		}
// MH810100(E) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)

		memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );	/* NT-NET���Z�ް��t�����ر	*/
		memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* ���Z�ް��������쐬�ر�ر	*/
		NTNET_Data152_DiscDataClear();
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		NTNET_Data152_DetailDataClear();
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�

		if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			PayInfo_Class = 9;										// ���Z���f�[�^�p�����敪��Ҕ�
// MH810100(S) K.Onodera 2020/02/20 �Ԕԃ`�P�b�g���X(�����Z�o�ɑΉ�)
			if( lcdbm_rsp_in_car_info_main.shubetsu == 1 ){	// ������Z
				NTNetTime_152_wk.Year = car_ot_f.year;				// �o�ɔN
				NTNetTime_152_wk.Mon  = car_ot_f.mon;				// �o�Ɍ�
				NTNetTime_152_wk.Day  = car_ot_f.day;				// �o�ɓ�
				NTNetTime_152_wk.Hour = car_ot_f.hour;				// �o�Ɏ�
				NTNetTime_152_wk.Min  = car_ot_f.min;				// �o�ɕ�
				NTNetTime_152_wk.Sec  = 0;							// �o�ɕb
			}else{
// MH810100(E) K.Onodera 2020/02/20 �Ԕԃ`�P�b�g���X(�����Z�o�ɑΉ�)
// MH810100(S)
//			// ���Z�J�n���̎������Z�b�g
//			NTNetTime_152_wk.Year = CLK_REC.year;					// �N
//			NTNetTime_152_wk.Mon  = CLK_REC.mont;					// ��
//			NTNetTime_152_wk.Day  = CLK_REC.date;					// ��
//			NTNetTime_152_wk.Hour = CLK_REC.hour;					// ��
//			NTNetTime_152_wk.Min  = CLK_REC.minu;					// ��
//			NTNetTime_152_wk.Sec  = (ushort)CLK_REC.seco;			// �b
			// ���Z�J�n�������擾
			pTime = GetPayStartTime();
			// ���Z�J�n���̎������Z�b�g
			NTNetTime_152_wk.Year = pTime->year;					// �N
			NTNetTime_152_wk.Mon  = pTime->mont;					// ��
			NTNetTime_152_wk.Day  = pTime->date;					// ��
			NTNetTime_152_wk.Hour = pTime->hour;					// ��
			NTNetTime_152_wk.Min  = pTime->minu;					// ��
			NTNetTime_152_wk.Sec  = (ushort)pTime->seco;			// �b
// MH810100(E)
// MH810100(S) K.Onodera 2020/02/20 �Ԕԃ`�P�b�g���X(�����Z�o�ɑΉ�)
			}
// MH810100(E) K.Onodera 2020/02/20 �Ԕԃ`�P�b�g���X(�����Z�o�ɑΉ�)
		}

// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//		if( RAU_Credit_Enabale != 0 ) {
//			cre_ctl.PayStartTime.Year = car_ot_f.year;				// ���Z�J�n�N
//			cre_ctl.PayStartTime.Mon = car_ot_f.mon;				// ���Z�J�n��
//			cre_ctl.PayStartTime.Day = car_ot_f.day;				// ���Z�J�n��
//			cre_ctl.PayStartTime.Hour = car_ot_f.hour;				// ���Z�J�n��
//			cre_ctl.PayStartTime.Min = car_ot_f.min;				// ���Z�J�n��
//			cre_ctl.PayStartTime.Sec = 0;							// ���Z�J�n�b
//		}
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)

		memset( card_use, 0, sizeof( card_use ) );					/* 1���Z���ޖ��g�p����ð���		*/
		memset( card_use2, 0, sizeof( card_use2 ) );				/* 1���Z���޽�����g�p����ð���	*/
		CardUseSyu = 0;												/* 1���Z�̊�����ނ̌����ر		*/
		discount_tyushi = 0;										/*�@�����z�@�@�@�@�@�@�@�@�@�@�@*/
		memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));		/*	�ڍג��~�ر�ر				*/
		Flap_Sub_Num = 0;											/*	�ڍג��~�ر					*/
		if( r_knd == 0 ){											/* ���Ԍ�						*/
			vl_carchg( num, chg_flg );								/*								*/
// �s��C��(S) K.Onodera 2016/12/08 #1645 �̎��؂̃v���y�C�h�J�[�h�c�z���������󎚂���Ȃ�
		}else if( r_knd == 7 ){
			;
// �s��C��(E) K.Onodera 2016/12/08 #1645 �̎��؂̃v���y�C�h�J�[�h�c�z���������󎚂���Ȃ�
// MH810100(S) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
		}else if( r_knd == 8 ){
			// �̎��ؗp�ɎԔԃf�[�^���Z�b�g
			PayData.CarSearchFlg = vl_car_no.CarSearchFlg;			// �Ԕ�/�����������
			memcpy( PayData.CarSearchData, vl_car_no.CarSearchData, sizeof(PayData.CarSearchData) );
			memcpy( PayData.CarNumber, vl_car_no.CarNumber, sizeof(PayData.CarNumber) );
			memcpy( PayData.CarDataID, vl_car_no.CarDataID, sizeof(PayData.CarDataID) );
// MH810100(E) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
		}else{														/*								*/
#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
			vl_scarchg( num );										/* �C�����Z						*/
			if( syusei[num-1].infofg & SSS_ZENWARI ){				/* �S�z�����g�p���Ă���?		*/
				ryo_buf.mis_zenwari = 1;							/* �S�z�����׸�ON				*/
			}														/*								*/
#endif		// SYUSEI_PAYMENT
		}															/*								*/
	}																/*								*/
// �s��C��(S) K.Onodera 2016/12/08 #1645 �̎��؂̃v���y�C�h�J�[�h�c�z���������󎚂���Ȃ�
//// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
//	// ���u���Z�H
//	else if( r_knd == 7 ){
//		memset( req_rhs, 0, sizeof( struct REQ_RKN )*150 );			/* ���݌v�Z��i�[�ر�ر			*/
//		memset( &ryo_buf, 0, sizeof( struct RYO_BUF ) );			/* �����ޯ̧�ر�				*/
//		ryo_buf.ryo_flg = 0;										/* �ʏ헿�ྯ�					*/
//		ryo_buf.pkiti = num;										/* ���Ԉʒu�ԍ����				*/
//		memset( &PayData, 0, sizeof( Receipt_data ) );				/* ���Z���,�̎��؈��ް�		*/
//	}
//// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
// �s��C��(E) K.Onodera 2016/12/08 #1645 �̎��؂̃v���y�C�h�J�[�h�c�z���������󎚂���Ȃ�
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#endif	// �����܂ŁF�N���E�h�����v�Z��fee_init(cal_cloud/cal_cloud.c)�ɑ����B
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
																	/*								*/
																	/*								*/
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#if 1	// ��������F�N���E�h�����v�Z��ryo_buf_n_set(cal_cloud/cal_cloud.c)�ɑ����B
		// ���C������ۂ�ryo_buf_n_set�����m�F���邱�ƁB
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	ryo_buf_n.require = ryo_buf.zankin;								/*								*/
	ryo_buf_n.dis     = ryo_buf.waribik;							/*								*/
	ryo_buf_n.nyu     = ryo_buf.nyukin;								/*								*/
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#endif	// �����܂ŁF�N���E�h�����v�Z��ryo_buf_n_set(cal_cloud/cal_cloud.c)�ɑ����B
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
																	/*								*/
	tol_dis = ryo_buf.waribik;										/*								*/
																	/*								*/
	ec09();															/* VL�������Ұ����				*/
	if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){						/* �S���ʻ��޽���,��ڰ����		*/
		se_svt.stim = (short)CPrmSS[S_STM][2];						/* ���޽���get					*/
		se_svt.gtim = (short)CPrmSS[S_STM][3];						/* ��ڰ����get					*/
	}else{															/* ��ʖ����޽���				*/
		se_svt.stim = (short)CPrmSS[S_STM][5+(3*(rysyasu-1))];		/* ��ʖ����޽���get			*/
		se_svt.gtim = (short)CPrmSS[S_STM][6+(3*(rysyasu-1))];		/* ��ʖ���ڰ����get			*/
	}																/*								*/

	if( num != 0xffff ){
		SvsTime_Syu[num-1] = rysyasu;								/* ���Z���̗�����ʾ�āi׸���я����p�j*/
	}

	if( num != 0xffff ){
		pflp = &FLAPDT.flp_data[num-1];
	}else{
		pflp = &LOCKMULTI.lock_mlt;
	}

	if( pflp->lag_to_in.BIT.LAGIN == ON ){
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�������ԓ���׸���ѵ��ނ͖����Ƃ��鎖_�폜
//		se_svt.stim = 0;											/* Yes�����޽��т𖳌��Ƃ���	*/
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�������ԓ���׸���ѵ��ނ͖����Ƃ��鎖_�폜
	}
	else
	{
// MH322914 (s) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)
//		if(	( pflp->mode == FLAP_CTRL_MODE3 ) ||							// �㏸���쒆�܂���
//			( pflp->mode == FLAP_CTRL_MODE4 ) ||							// �㏸�ς�
//			( fusei_cal == 1 ))												// �s���o�ɂɂ�闿���v�Z���{�H
//		{
//			svtm = (long)se_svt.stim;
//			svtm *= 60;
//
//			if( num >= LOCK_START_NO)
//			{
//				uptm = Carkind_Param(ROCK_CLOSE_TIMER, (char)(LockInfo[num - 1].ryo_syu), 5,1);
//			}
//			else
//			{
//				uptm = Carkind_Param(FLAP_UP_TIMER, (char)(LockInfo[num - 1].ryo_syu), 5,1);
//			}
//			uptm = (long)(((uptm/100)*60) + (uptm%100));
//
//
//			if( svtm == uptm )											
//			{
//				if( prm_get( COM_PRM,S_TYP,62,1,2 ) ) { 				// �C�����Z�@�\����ݒ�
//					if(OPECTL.Pay_mod == 2){							// �C�����Z��
//						se_svt.stim = 0;								// ���޽��і���
//					}else{												// �ʏ퐸�Z��
//						if( num != 0xffff ){							// ���ݐ��Z�łȂ�
//							if( syusei[num-1].sei != 2 ){				// �����Ԏ��łȂ�
//								se_svt.stim = 0;						// ���޽��і���
//							}											//
//						}else{											// ���ݐ��Z�̂Ƃ�
//							se_svt.stim = 0;							// ���޽��і���
//						}
//					}													//
//				}else{													// �C�����Z�@�\�Ȃ��ݒ�ł���Ε����Ԏ��Ȃ�
//					se_svt.stim = 0;									// ���޽��і���
//				}
//			}
//		}
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//		if ( prm_get( COM_PRM, S_TYP, 68, 1, 5 ) == 0 ) {
//			// ���b�N�^�C�}�[�ƃT�[�r�X�^�C�����������Ԃ̎��ɃT�[�r�X�^�C�������b�N�܂łƂ���
//			if(	( pflp->mode == FLAP_CTRL_MODE3 ) ||							// �㏸���쒆�܂���
//				( pflp->mode == FLAP_CTRL_MODE4 ) ||							// �㏸�ς�
//				( fusei_cal == 1 ))												// �s���o�ɂɂ�闿���v�Z���{�H
//			{
//				svtm = (long)se_svt.stim;
//				svtm *= 60;
//	
//				if( num >= LOCK_START_NO)
//				{
//					uptm = Carkind_Param(ROCK_CLOSE_TIMER, (char)(LockInfo[num - 1].ryo_syu), 5,1);
//				}
//				else
//				{
//					uptm = Carkind_Param(FLAP_UP_TIMER, (char)(LockInfo[num - 1].ryo_syu), 5,1);
//				}
//				uptm = (long)(((uptm/100)*60) + (uptm%100));
//	
//	
//				if( svtm == uptm )											
//				{
//					if( prm_get( COM_PRM,S_TYP,62,1,2 ) ) { 				// �C�����Z�@�\����ݒ�
//						if(OPECTL.Pay_mod == 2){							// �C�����Z��
//							se_svt.stim = 0;								// ���޽��і���
//						}else{												// �ʏ퐸�Z��
//							if( num != 0xffff ){							// ���ݐ��Z�łȂ�
//								if( syusei[num-1].sei != 2 ){				// �����Ԏ��łȂ�
//									se_svt.stim = 0;						// ���޽��і���
//								}											//
//							}else{											// ���ݐ��Z�̂Ƃ�
//								se_svt.stim = 0;							// ���޽��і���
//							}
//						}													//
//					}else{													// �C�����Z�@�\�Ȃ��ݒ�ł���Ε����Ԏ��Ȃ�
//						se_svt.stim = 0;									// ���޽��і���
//					}
//				}
//			}
//		}
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// MH322914 (e) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)
	}

	et02();															/* �����v�Z						*/
	for( calcreqnum =  0 ;											/* �����v�Z�v����				*/
		  calcreqnum < (short)req_crd_cnt	;						/* ex.�e�����ɗ����v�Z			*/
		  calcreqnum ++					)							/* ���������قȂ�				*/
	{																/*								*/
		memcpy( &req_rkn, req_crd + calcreqnum,						/* �����v�Z�v������				*/
				sizeof(req_rkn)				) ;						/* ex.2 ���ڈȍ~���v����		*/
		et02()	;													/* �����v�Z						*/
	}																/*								*/
																	/*								*/
	if( tki_flg != OFF )											/* ���ԑђ���㎞�Ԋ��� 		*/
	{																/* �������Ȃ�					*/
		if( tki_ken ){												/*								*/
			ryo_buf.dis_fee = 0L;									/* �O�񗿋������z�ر			*/
			ryo_buf.dis_tim = 0L;									/* �O�񎞊Ԋ����z�ر			*/
			ryo_buf.fee_amount = 0L;								/* ���������z�ر				*/
			ryo_buf.tim_amount = 0L;								/* �������Ԑ��ر				*/
			wk_pay_ryo = PayData_Sub.pay_ryo;
			memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));
			PayData_Sub.pay_ryo = wk_pay_ryo;
			wrcnt_sub = 0;
			discount_tyushi = 0;									/*�@�����z�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
		}															/*								*/
		for( i = 0; i < (short)tki_ken; i++ )						/* �ۑ����ėL�闿���v�Z 		*/
		{															/* ��ٰ��						*/
			memset( &req_rkn.param,0x00,21);						/* �����v��ð��ٸر				*/
			memcpy( (char *)&req_rkn,								/* �ۑ����Ă������v�Z�v 		*/
					(char *)&req_tki[i],10 );						/* �����Ăїv��					*/
			if((req_rkn.param == RY_FRE_K)							/* �g��1�񐔌������v��			*/
			 ||(req_rkn.param == RY_FRE)							/* �񐔌������v��				*/
			 ||(req_rkn.param == RY_FRE_K2)							/* �g��2�񐔌������v��			*/
			 ||(req_rkn.param == RY_FRE_K3)							/* �g��3�񐔌������v��			*/

			 ||(req_rkn.param == RY_EMY)							/* �d�q���σJ�[�h				*/
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			 ||(req_rkn.param == RY_EQR)							/* QR�R�[�h����     			*/
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			 ||(req_rkn.param == RY_GNG)							/* ���z			*/
// �s��C��(S) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
			 ||(req_rkn.param == RY_GNG_FEE)						/* ���z			*/
			 ||(req_rkn.param == RY_GNG_TIM)						/* ���z			*/
// �s��C��(E) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
			 ||(req_rkn.param == RY_FRK)							/* �U��			*/
			 ||(req_rkn.param == RY_PCO_K2)							/* �g��2����߲�޶���			*/
			 ||(req_rkn.param == RY_PCO_K3)							/* �g��3����߲�޶���			*/
			 ||(req_rkn.param == RY_PCO_K)							/* �g��1����߲�޶���			*/
			 ||(req_rkn.param == RY_PCO))							/* ����߲�޶���					*/
			{														/*								*/
			   req_rkn.param = 0xff;								/*								*/
			}														/*								*/
			et02();													/* �����v�Z����					*/
		}															/*								*/
		tki_ken = 0;												/* �����v�Z�ۑ������ر			*/
		re_req_flg = OFF;											/* ���ԑђ���㎞�Ԋ��� 		*/
	}																/* �׸�ؾ��						*/
																	/*								*/
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#if 1	// ��������F�N���E�h�����v�Z��ryo_buf_n_get(cal_cloud/cal_cloud.c)�ɑ����B
		// ���C������ۂ�ryo_buf_n_get�����m�F���邱�ƁB
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	ryo_buf.zankin  = ryo_buf_n.require ;							/* ���ԗ��ྯ�					*/
	ryo_buf.tax     = ryo_buf_n.tax;								/* �ŋྯ�						*/
	ryo_buf.waribik = ryo_buf_n.dis;								/* �����z						*/
	if( OPECTL.Ope_mod != 22 )										/* ����X�V���łȂ��Ȃ�			*/
	ryo_buf.syubet  = (char)(rysyasu - 1);							/* ������ʾ��(��ʐ؊���)		*/
																	/*								*/
	if( r_knd == 1 )												/* ���������?					*/
	{																/*								*/
		ryo_buf.ryo_flg += 2;										/*								*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.ryo_flg < 2 )										/* ���Ԍ����Z����				*/
	{																/*								*/
		ryo_buf.tik_syu = rysyasu;									/* �������						*/
		ryo_buf.tyu_ryo = ryo_buf_n.ryo;							/* ���ԗ��ྯ�					*/
	}																/*								*/
	else															/*								*/
	{																/*								*/
		ryo_buf.pas_syu = rysyasu;									/* ������̗������				*/
		ryo_buf.tei_ryo = ryo_buf_n.ryo;							/* ��������ྯ�				*/
	}																/*								*/
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#endif	// �����܂ŁF�N���E�h�����v�Z��ryo_buf_n_get(cal_cloud/cal_cloud.c)�ɑ����B
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
																	/*								*/
	/*====================================================*/		/*								*/
	// ����������O�̊e�����������폜����							/*								*/
	/*====================================================*/		/*								*/
	for( cnt = 0; cnt < rhspnt; cnt++ )								/* �����v�Z���ް����ɂȂ�܂�	*/
	{																/*								*/
		rhs_p = &req_rhs[cnt];										/*								*/
		if(( rhs_p->param == RY_PKC		||							/* �����v�Z�v��������������̏ꍇ	*/
			 rhs_p->param == RY_PKC_K	||							/*								*/
			 rhs_p->param == RY_PKC_K2	||							/*								*/
			 rhs_p->param == RY_PKC_K3	||				/*								*/
			 ((rhs_p->param == RY_KCH) && (( rhs_p->data[1] >> 16 )== 2))) &&	// ������Ԏ�ؑ�
			 cnt != 0 )
		{															/*								*/
			for( i = (char)(cnt-1); i > 0; i-- )					/*								*/
			{														/*								*/
				rhs_p = &req_rhs[i];								/*								*/
				if( rhs_p->param == RY_TWR || 						/* ������������					*/
					rhs_p->param == RY_RWR || 						/* ���Ԋ�������					*/
					rhs_p->param == RY_SKC || 						/* ���޽������					*/
					rhs_p->param == RY_SKC_K ||						/* �g�����޽������				*/
					rhs_p->param == RY_SKC_K2 ||					/* �g��2���޽������				*/
					rhs_p->param == RY_SKC_K3 ||					/* �g��3���޽������				*/
					rhs_p->param == RY_KAK ||						/* �|��������					*/
					rhs_p->param == RY_KAK_K ||						/* �g��1�|����					*/
					rhs_p->param == RY_KAK_K2 ||					/* �g��2�|����					*/
					rhs_p->param == RY_KAK_K3 ||					/* �g��3�|����					*/
					rhs_p->param == RY_SSS ||						/* �C��							*/
					rhs_p->param == RY_CSK ||						/* ���Z���~���޽��				*/
					rhs_p->param == RY_SNC			)				/* �X��������					*/
				{													/*								*/
					rhs_p->param = 0xff;							/* �v���ް�FF�ر				*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
		}															/*								*/
	}																/*								*/
	/*==============================================================================================*/
	/* �������Ұ����̎���(�ʐ��Z���܂�)�A���Z�ް��̊���������S�Ă����ō쐬����				*/
	/*==============================================================================================*/
	wk_dat = (ushort)(1+6*(rysyasu-1));
	wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = ��ʎ��Ԋ����l		*/

// MH321800(S) �������8000�̃Z�b�g���@���C��
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
////	if( (r_knd != 0) || (wk_dat2) )									/* ���Ԍ��ȊO or ��ʎ��Ԋ����ݒ肠��	*/
//	// r_kind = 0�ł��邪�A�N���W�b�g���Z���͊������ɖ⍇���ԍ�(8000)���Z�b�g����
//	if( (r_knd != 0) || (wk_dat2) || (r_knd == 0 && vl_now == V_CRE) )	/* ���Ԍ��ȊO or ��ʎ��Ԋ����ݒ肠��	*/
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
	if( (r_knd != 0) || (wk_dat2) )									/* ���Ԍ��ȊO or ��ʎ��Ԋ����ݒ肠��	*/
// MH321800(E) �������8000�̃Z�b�g���@���C��
	{																/*								*/
		memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* ���Z�ް��������쐬�ر�ر	*/
		NTNET_Data152_DiscDataClear();
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		NTNET_Data152_DetailDataClear();
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
		/* ��ʎ��Ԋ���������� WDicData �̐擪�ɃZ�b�g���� */
		/* �i�����z0�~�ł�NT-NET�d���ő��M����j			*/
		if( wk_dat2 ){												/* ��ʎ��Ԋ����ݒ肠��			*/
			PayData.DiscountData[0].ParkingNo = CPrmSS[S_SYS][1];	/* ���ԏꇂ�i��{���ԏꇂ�j		*/
			PayData.DiscountData[0].DiscSyu = NTNET_SYUBET;			/* ������ʁi��ʎ��Ԋ����j		*/
			PayData.DiscountData[0].DiscNo = rysyasu;				/* �����敪�i�������A�`L�F�ύX��j	*/
			PayData.DiscountData[0].DiscCount =	1;					/* �g�p����						*/
// GG129000(S) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// 			PayData.DiscountData[0].Discount = PayData.SyuWariRyo;	/* �����z						*/
// GG129000(E) R.Endo 2023/05/31 �Ԕԃ`�P�b�g���X4.1 #7029 ����9��g�p��QR���㌔��ǂ񂾏ꍇ�ɐ��Z���O���甃�����z���v����������
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 			PayData.DiscountData[0].uDiscData.common.MoveMode = 1;	/* �g�p							*/
// 			PayData.DiscountData[0].uDiscData.common.DiscFlg = 0;	/* �����ς�(�V�K���Z)			*/
			PayData.DiscountData[0].uDiscData.common.PrevDiscount = 0;			// ����g�p�����O�񐸎Z�܂ł̊������z
			PayData.DiscountData[0].uDiscData.common.PrevUsageDiscount  = 0;	// �O�񐸎Z�܂ł̎g�p�ς݊������z
			PayData.DiscountData[0].uDiscData.common.PrevUsageDiscCount = 0;	// �O�񐸎Z�܂ł̎g�p�ςݖ���
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DPARKINGNO, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DSYU, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DNO, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DCOUNT, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DISCOUNT, 0);
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_PREVDISCOUNT, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_PREVUSAGEDISCOUNT, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_PREVUSAGEDCOUNT, 0);
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			++wrcnt;												/*								*/
			/* �ȉ��͒��Ԍ��i����񖳂��j�ȊO���̏����Ȃ̂ŁA���̏����ʂ�Skip���� */
// MH321800(S) �������8000�̃Z�b�g���@���C��
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
////			if( r_knd == 0 ){										/* ������񖳂����Ԍ�			*/
//			// r_kind = 0�ł��邪�A�N���W�b�g���Z���͊������ɖ⍇���ԍ�(8000)���Z�b�g���邽�ߏ����I�������Ȃ�
//			if( r_knd == 0 && vl_now != V_CRE ){						/* ������񖳂����Ԍ�			*/
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810105 GG119202(S) ��ʊ�����̃N���W�b�g���Z�Ő��Z�f�[�^�Ɋ�����񂪃Z�b�g����Ȃ�
//			if( r_knd == 0 ){										/* ������񖳂����Ԍ�			*/
			if( r_knd == 0 && vl_now != V_CRE ){					/* ������񖳂����Ԍ��i�N���W�b�g�ȊO�j*/
// MH810105 GG119202(E) ��ʊ�����̃N���W�b�g���Z�Ő��Z�f�[�^�Ɋ�����񂪃Z�b�g����Ȃ�
// MH321800(E) �������8000�̃Z�b�g���@���C��
				goto	ryo_cal_10;									/* if�����I��					*/
			}
		}
																	/*								*/
		for( cnt = 0; cnt < rhspnt; cnt++ )							/*								*/
		{															/*								*/
			rhs_p = &req_rhs[cnt];									/*								*/
			switch( rhs_p->param )									/* �����v�Z�v�����ނɂ�蕪��	*/
			{														/*								*/
			  case RY_SKC:											/* ��{���޽��					*/
			  case RY_SKC_K:										/* �g��1���޽��					*/
			  case RY_SKC_K2:										/* �g��2���޽��					*/
			  case RY_SKC_K3:										/* �g��3���޽��					*/
				wk_dat = (ushort)((rhs_p->data[0] &					/* ��޾��						*/
									0xffff0000) >> 16);				/*								*/
				if( rhs_p->data[1] == 0 ){							/*								*/
					rhs_p->data[1] = 1;								/* ������0�Ȃ�1���				*/
				}													/*								*/
// MH810100(S) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
				DiscStatus = (uchar)(( rhs_p->data[0] >> 15 ) & 0x00000001 );	/* �����X�e�[�^�X	*/
// MH810100(E) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
																	/*								*/
				if(( wk_dat >= 1 )&&( wk_dat <= SVS_MAX )){			/* ���޽��A�`O					*/
					// ���Z�ް��p									/*								*/
					wk_dat2 = (ushort)CPrmSS[S_SER][1+3*(wk_dat-1)];/*								*/
					if( wk_dat2 == 1 ){								/*								*/
						wk_dat2 = NTNET_SVS_T;						/* �������=���޽��(����)		*/
					}else{											/*								*/
						wk_dat2 = NTNET_SVS_M;						/* �������=���޽��(����)		*/
					}												/*								*/
																	/*								*/
					if( rhs_p->param == RY_SKC ){					/* ��{���ԏꇂ					*/
						wk_dat3 = CPrmSS[S_SYS][1];					/* ���ԏꇂ���					*/
					}else if( rhs_p->param == RY_SKC_K ){			/* �g��1���ԏꇂ				*/
						wk_dat3 = CPrmSS[S_SYS][2];					/* ���ԏꇂ���					*/
					}else if( rhs_p->param == RY_SKC_K2 ){			/* �g��2���ԏꇂ				*/
						wk_dat3 = CPrmSS[S_SYS][3];					/* ���ԏꇂ���					*/
					}else{											/* �g��3���ԏꇂ				*/
						wk_dat3 = CPrmSS[S_SYS][4];					/* ���ԏꇂ���					*/
					}												/*								*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
					if ( rhs_p->data[1] >= 2000 ) {			// �O�񐸎Z�܂ł̊���
						//  �O�񐸎Z�܂ł̊����̖���
						wk_dat7 = (ushort)(rhs_p->data[1] - 2000);
					} else if ( rhs_p->data[1] >= 1000 ) {	// ����g�p�����O�񐸎Z�܂ł̊���
						// ����g�p�����O�񐸎Z�܂ł̊����̖���
						wk_dat7 = (ushort)(rhs_p->data[1] - 1000);
					} else {								// ����g�p��������
						// ����g�p���������̖���
						wk_dat7 = (ushort)(rhs_p->data[1]);
					}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
																	/*								*/
					for( i=0; i < WTIK_USEMAX; i++ ){				/*								*/
						if(( PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* ���ԏꇂ��������?*/
						   ( PayData.DiscountData[i].DiscSyu == wk_dat2 )&&		/* ������ʂ�������?*/
						   ( PayData.DiscountData[i].DiscNo == wk_dat )&&		/* �����敪��������?*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// // MH810100(S) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// 						   (rhs_p->data[1] < 1000 ) &&		// �t���O�������Ă��Ȃ�
// 						   (PayData.DiscountData[i].uDiscData.common.MoveMode < 1000) &&
// // MH810100(E) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// // MH810100(S) K.Onodera 2020/02/10 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// //						   ( PayData.DiscountData[i].DiscInfo1 == (rhs_p->data[0] & 0x0000ffff) ))	/* �������1(�|�����)��������? */
// 						   ( PayData.DiscountData[i].DiscInfo1 == (rhs_p->data[0] & 0x00007fff) )&&	// �������1(�|�����)��������?
// 						   ( PayData.DiscountData[i].uDiscData.common.DiscFlg == DiscStatus ))		// �����ςݏ�Ԃ�������?
// // MH810100(S) K.Onodera 2020/02/10 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
						   ( PayData.DiscountData[i].DiscInfo1 == (rhs_p->data[0] & 0x00007fff) ))	// �������1(�|�����)��������?
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
						{											/*								*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 							PayData.DiscountData[i].DiscCount +=	/* �g�p����						*/
// 										(ushort)rhs_p->data[1];		/*								*/
// 							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
// 							PayData.DiscountData[i].Discount += rhs_p->data[2];	/* �����z			*/
// 							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
							if ( DiscStatus ) {	// �����ς�
								// �O�񐸎Z�܂ł̎g�p�ςݖ���
								PayData.DiscountData[i].uDiscData.common.PrevUsageDiscCount += (uchar)wk_dat7;
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVUSAGEDCOUNT, i);

								// �O�񐸎Z�܂ł̎g�p�ς݊������z
								PayData.DiscountData[i].uDiscData.common.PrevUsageDiscount += (ulong)rhs_p->data[2];
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVUSAGEDISCOUNT, i);
							} else {			// ������
								if ( (rhs_p->data[1] < 2000) && (rhs_p->data[1] >= 1000) ) {	// ����g�p�����O�񐸎Z�܂ł̊���
									// ����g�p�����O�񐸎Z�܂ł̊������z
									PayData.DiscountData[i].uDiscData.common.PrevDiscount += (ulong)rhs_p->data[2];
									NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVDISCOUNT, i);
								}

								// ����g�p��������
								PayData.DiscountData[i].DiscCount += (uchar)wk_dat7;
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);

								// ����g�p�����������z
								PayData.DiscountData[i].Discount += (ulong)rhs_p->data[2];
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
							}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
							if( wk_dat2 == NTNET_SVS_T ){			/* ���Ԋ���?					*/
								PayData.DiscountData[i].uDiscData.common.DiscInfo2 +=	/*			*/
									prm_tim( COM_PRM,S_SER,(short)(2+3*(wk_dat-1)));	/* �������2(�������Ԑ��F��) */
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
							}										/*								*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){	/* �����Y������					*/
						PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* ���ԏꇂ					*/
						PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* �������					*/
						PayData.DiscountData[wrcnt].DiscNo = wk_dat;	/* �����敪(���޽��A�`C)	*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// // MH810100(S) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// //						PayData.DiscountData[wrcnt].DiscCount =		/* �g�p����						*/
// //										(ushort)rhs_p->data[1];		/*								*/
// 						if( rhs_p->data[1] >= 2000){
// 							PayData.DiscountData[wrcnt].DiscCount =		/* �g�p����						*/
// 											(ushort)rhs_p->data[1]-2000;		/*								*/
// 						}else if(rhs_p->data[1] < 1000){
// 							PayData.DiscountData[wrcnt].DiscCount =		/* �g�p����						*/
// 											(ushort)rhs_p->data[1];		/*								*/
// 						}else{
// 							PayData.DiscountData[wrcnt].DiscCount =	0;	/* �g�p����						*/
// 						}
// // MH810100(E) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// 						PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* �����z			*/
						if ( DiscStatus ) {	// �����ς�
							// �O�񐸎Z�܂ł̎g�p�ςݖ���
							PayData.DiscountData[wrcnt].uDiscData.common.PrevUsageDiscCount = (uchar)wk_dat7;

							// �O�񐸎Z�܂ł̎g�p�ς݊������z
							PayData.DiscountData[wrcnt].uDiscData.common.PrevUsageDiscount = (ulong)rhs_p->data[2];
						} else {			// ������
							if ( (rhs_p->data[1] < 2000) && (rhs_p->data[1] >= 1000) ) {	// ����g�p�����O�񐸎Z�܂ł̊���
								// ����g�p�����O�񐸎Z�܂ł̊������z
								PayData.DiscountData[wrcnt].uDiscData.common.PrevDiscount = (ulong)rhs_p->data[2];
							}

							// ����g�p��������
							PayData.DiscountData[wrcnt].DiscCount = (uchar)wk_dat7;

							// ����g�p�����������z
							PayData.DiscountData[wrcnt].Discount = (ulong)rhs_p->data[2];
						}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
						PayData.DiscountData[wrcnt].DiscInfo1 =		/*								*/
// MH810100(S) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
//									rhs_p->data[0] & 0x0000ffff;	/* �������1(�|�����)			*/
									rhs_p->data[0] & 0x00007fff;	/* �������1(�|�����)			*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 						PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg =	/* �����ς�			*/
// 												(ushort)DiscStatus;	/* 								*/
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(E) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
						if( wk_dat2 == NTNET_SVS_T ){				/* ���Ԋ���?					*/
							PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 =	/*			*/
								prm_tim( COM_PRM,S_SER,(short)(2+3*(wk_dat-1)));/* �������2(�������Ԑ�) */
						}else{										/*								*/
							PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;	/* �������2(���g�p) */
						}											/*								*/
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// // MH810100(S) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// //						PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* �g�p			*/
// 						if(rhs_p->data[1] == 1000){
// 							// �̎��؂ł܂Ƃ߂�׃t���O�����Ă���
// 							PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1001;	/* �g�p			*/
// 						}else if( rhs_p->data[1] >= 2000){
// 							PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 2001;	/* �g�p			*/
// 						}else{
// 							PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* �g�p			*/
// 						}
// // MH810100(E) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// // MH810100(S) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// //						PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* �����ς�(�V�K���Z) */
// // MH810100(E) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVDISCOUNT, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVUSAGEDISCOUNT, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVUSAGEDCOUNT, wrcnt);
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
						wrcnt++;									/*								*/
					}												/*								*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			  case RY_KAK:											/* ��{�|����(�X������)			*/
			  case RY_KAK_K:										/* �g��1�|����(�X������)		*/
			  case RY_KAK_K2:										/* �g��2�|����(�X������)		*/
			  case RY_KAK_K3:										/* �g��3�|����(�X������)		*/
				if( rhs_p->data[1] == 0 ){							/*								*/
					rhs_p->data[1] = 1;								/* ������0�Ȃ�1���				*/
				}													/*								*/
// MH810100(S) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
//				wk_dat5 = (ushort)((rhs_p->data[0] &					/* ��޾��						*/
//									0xffff0000) >> 16);				/*								*/
				wk_dat5 = (ushort)((rhs_p->data[0] >> 16) & 0x000000ff);	/* �X������ރZ�b�g			*/
				wk_dat6 = (ushort)((rhs_p->data[0] >> 24) & 0x000000ff);	/* ���X�܊�����ʃZ�b�g		*/
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
// MH810100(E) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
				// ���Z�ް��p										/*								*/
// MH810100(S) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
//				wk_dat = (ushort)rhs_p->data[0];					/* �X��							*/
				wk_dat = (ushort)( rhs_p->data[0] & 0x00007fff );	/* �X��							*/
				DiscStatus = (uchar)(( rhs_p->data[0] >> 15 ) & 0x00000001 );	/* �����X�e�[�^�X	*/
// MH810100(E) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
//				wk_dat2 = (ushort)CPrmSS[S_STO][1+3*(wk_dat-1)];	/*								*/
//				if( wk_dat2 == 1 ){									/*								*/
//					wk_dat2 = NTNET_KAK_T;							/* �������=�X����(����)		*/
//				}else{												/*								*/
//					wk_dat2 = NTNET_KAK_M;							/* �������=�X����(����)		*/
//				}													/*								*/
				if ( wk_dat6 ) {									/* ���X�܊���					*/
					wk_adr = (short)(wk_dat6 * 2);
					wk_ses = (short)S_TAT;
					if ( prm_get(COM_PRM, wk_ses, wk_adr, 1, 1) == 1 ) {
						wk_dat2 = NTNET_TKAK_T;
					} else {
						wk_dat2 = NTNET_TKAK_M;
					}
				} else {											/* �X����/��������				*/
					wk_adr = (short)((wk_dat * 3) - 2);
					wk_ses = (short)S_STO;
					if ( prm_get(COM_PRM, wk_ses, wk_adr, 1, 1) == 1 ) {
						wk_dat2 = NTNET_KAK_T;
					} else {
						wk_dat2 = NTNET_KAK_M;
					}
				}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
																	/*								*/
				if( rhs_p->param == RY_KAK ){						/* ��{���ԏꇂ					*/
					wk_dat3 = CPrmSS[S_SYS][1];						/* ���ԏꇂ���					*/
				}else if( rhs_p->param == RY_KAK_K ){				/* �g��1���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][2];						/* ���ԏꇂ���					*/
				}else if( rhs_p->param == RY_KAK_K2 ){				/* �g��2���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][3];						/* ���ԏꇂ���					*/
				}else{												/* �g��3���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][4];						/* ���ԏꇂ���					*/
				}													/*								*/
// MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
				wk_dat4 = 0;										/* �������1�N���A				*/
				CheckShoppingDisc( wk_dat3, &wk_dat2, wk_dat, &wk_dat4 );	/* ���������`�F�b�N		*/
// MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
				if ( rhs_p->data[1] >= 2000 ) {			// �O�񐸎Z�܂ł̊���
					//  �O�񐸎Z�܂ł̊����̖���
					wk_dat7 = (ushort)(rhs_p->data[1] - 2000);
				} else if ( rhs_p->data[1] >= 1000 ) {	// ����g�p�����O�񐸎Z�܂ł̊���
					// ����g�p�����O�񐸎Z�܂ł̊����̖���
					wk_dat7 = (ushort)(rhs_p->data[1] - 1000);
				} else {								// ����g�p��������
					// ����g�p���������̖���
					wk_dat7 = (ushort)(rhs_p->data[1]);
				}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
																	/*								*/
				for( i=0; i < WTIK_USEMAX; i++ ){					/*								*/
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 					if(( PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* ���ԏꇂ��������?	*/
// 					   ( PayData.DiscountData[i].DiscSyu == wk_dat2 )&&	/* ������ʂ�������?		*/
// // MH810100(S) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// 					   (rhs_p->data[1] < 1000 ) &&		// �t���O�������Ă��Ȃ�
// 					   (PayData.DiscountData[i].uDiscData.common.MoveMode < 1000) &&
// // MH810100(E) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// // MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// //					   ( PayData.DiscountData[i].DiscNo == wk_dat ))	/* �����敪��������?		*/
// 					   ( PayData.DiscountData[i].DiscNo == wk_dat )&&	/* �����敪��������?		*/
// 					   ( PayData.DiscountData[i].uDiscData.common.DiscFlg == DiscStatus ))	/* �����ςݏ�Ԃ�������?*/
// // MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// 					{												/*								*/
// 						PayData.DiscountData[i].DiscCount +=		/* �g�p����						*/
// 									(ushort)rhs_p->data[1];			/*								*/
// 						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
// 						PayData.DiscountData[i].Discount += rhs_p->data[2];	/* �����z				*/
// 						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
					if (   (PayData.DiscountData[i].ParkingNo  == wk_dat3)					// ���ԏꇂ
						&& (PayData.DiscountData[i].DiscSyu    == wk_dat2)					// �������
						&& (PayData.DiscountData[i].DiscNo     == wk_dat )					// �����敪
						&& ((((wk_dat2 == NTNET_KAK_M) || (wk_dat2 == NTNET_KAK_T)) &&		// �X����
						     (PayData.DiscountData[i].DiscInfo1 == wk_dat5))				// �������1(�X�������)
						||  (((wk_dat2 == NTNET_TKAK_M) || (wk_dat2 == NTNET_TKAK_T)) &&	// ���X�܊���
						     (PayData.DiscountData[i].DiscInfo1 == wk_dat6)))				// �������1(���X�܊������)
						&& (wk_dat2 != NTNET_SHOP_DISC_AMT)									// ��������(���z)�ł͂Ȃ�
						&& (wk_dat2 != NTNET_SHOP_DISC_TIME)								// ��������(����)�ł͂Ȃ�
					) {
						if ( DiscStatus ) {	// �����ς�
							// �O�񐸎Z�܂ł̎g�p�ςݖ���
							PayData.DiscountData[i].uDiscData.common.PrevUsageDiscCount += (uchar)wk_dat7;
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVUSAGEDCOUNT, i);

							// �O�񐸎Z�܂ł̎g�p�ς݊������z
							PayData.DiscountData[i].uDiscData.common.PrevUsageDiscount += (ulong)rhs_p->data[2];
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVUSAGEDISCOUNT, i);
						} else {			// ������
							if ( (rhs_p->data[1] < 2000) && (rhs_p->data[1] >= 1000) ) {	// ����g�p�����O�񐸎Z�܂ł̊���
								// ����g�p�����O�񐸎Z�܂ł̊������z
								PayData.DiscountData[i].uDiscData.common.PrevDiscount += (ulong)rhs_p->data[2];
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVDISCOUNT, i);
							}

							// ����g�p��������
							PayData.DiscountData[i].DiscCount += (uchar)wk_dat7;
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);

							// ����g�p�����������z
							PayData.DiscountData[i].Discount += (ulong)rhs_p->data[2];
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
						}
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(S) K.Onodera 2020/02/28 �Ԕԃ`�P�b�g���X(���������Ή�)
//						if( wk_dat2 == NTNET_KAK_T ){				/* ���Ԋ���?					*/
						if(( wk_dat2 == NTNET_KAK_T ) ||			/* ���Ԋ���?					*/
// GG124100(S) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// // MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
// 						   ( wk_dat2 == NTNET_TKAK_T ) ||		 	/*								*/
// // MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
// 						   ( wk_dat2 == NTNET_SHOP_DISC_TIME )){	/*								*/
						   ( wk_dat2 == NTNET_TKAK_T )) {
// GG124100(E) R.Endo 2022/08/09 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(E) K.Onodera 2020/02/28 �Ԕԃ`�P�b�g���X(���������Ή�)
							PayData.DiscountData[i].uDiscData.common.DiscInfo2 +=	/*				*/
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
//								prm_tim( COM_PRM,S_STO,(short)(2+3*(wk_dat-1)));	/* �������2(�������Ԑ��F��) */
								prm_tim(COM_PRM, wk_ses, (wk_adr + 1));	/* �������2(�������Ԑ��F��) */
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
						}											/*								*/
						break;										/*								*/
					}												/*								*/
				}													/*								*/
				if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){		/* �����Y������					*/
					PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* ���ԏꇂ						*/
					PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* �������						*/
					PayData.DiscountData[wrcnt].DiscNo = wk_dat;	/* �����敪(�X��)				*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// // MH810100(S) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// //					PayData.DiscountData[wrcnt].DiscCount =			/* �g�p����						*/
// //										(ushort)rhs_p->data[1];		/*								*/
// 					if( rhs_p->data[1] >= 2000){
// 						PayData.DiscountData[wrcnt].DiscCount =		/* �g�p����						*/
// 										(ushort)rhs_p->data[1]-2000;		/*								*/
// 					}else if(rhs_p->data[1] < 1000){
// 						PayData.DiscountData[wrcnt].DiscCount =		/* �g�p����						*/
// 										(ushort)rhs_p->data[1];		/*								*/
// 					}else{
// 						PayData.DiscountData[wrcnt].DiscCount =	0;	/* �g�p����						*/
// 					}
// // MH810100(E) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// 					PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* �����z				*/
					if ( DiscStatus ) {	// �����ς�
						// �O�񐸎Z�܂ł̎g�p�ςݖ���
						PayData.DiscountData[wrcnt].uDiscData.common.PrevUsageDiscCount = (uchar)wk_dat7;

						// �O�񐸎Z�܂ł̎g�p�ς݊������z
						PayData.DiscountData[wrcnt].uDiscData.common.PrevUsageDiscount = (ulong)rhs_p->data[2];
					} else {			// ������
						if ( (rhs_p->data[1] < 2000) && (rhs_p->data[1] >= 1000) ) {	// ����g�p�����O�񐸎Z�܂ł̊���
							// ����g�p�����O�񐸎Z�܂ł̊������z
							PayData.DiscountData[wrcnt].uDiscData.common.PrevDiscount = (ulong)rhs_p->data[2];
						}

						// ����g�p��������
						PayData.DiscountData[wrcnt].DiscCount = (uchar)wk_dat7;

						// ����g�p�����������z
						PayData.DiscountData[wrcnt].Discount = (ulong)rhs_p->data[2];
					}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(S) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
//// MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
////					PayData.DiscountData[wrcnt].DiscInfo1 = 0L;		/* �������1(���g�p)			*/
//					PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)wk_dat4;	/* �������1			*/
//// MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
					if( wk_dat2 == NTNET_KAK_M || wk_dat2 == NTNET_KAK_T){
						PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)wk_dat5;	/* �������1			*/
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
					} else if ( (wk_dat2 == NTNET_TKAK_M) || (wk_dat2 == NTNET_TKAK_T) ) {
						PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)wk_dat6;	/* �������1			*/
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
					}else{
						PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)wk_dat4;	/* �������1			*/
					}
// MH810100(E) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg =	/* �����ς�				*/
// 												(ushort)DiscStatus;	/* 								*/
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
// MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
//					if( wk_dat2 == NTNET_KAK_T ){					/* ���Ԋ���?					*/
					if(( wk_dat2 == NTNET_KAK_T ) ||				/* ���Ԋ���?					*/
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
					   ( wk_dat2 == NTNET_TKAK_T ) ||			 	/*								*/
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
					   ( wk_dat2 == NTNET_SHOP_DISC_TIME )){		/*								*/
// MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(���������Ή�)
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 =	/*				*/
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
//							prm_tim( COM_PRM,S_STO,(short)(2+3*(wk_dat-1)));/* �������2(�������Ԑ�)*/
							prm_tim(COM_PRM, wk_ses, (wk_adr + 1));	/* �������2(�������Ԑ��F��)	*/
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�����̕ύX)
					}else{											/*								*/
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;/* �������2(���g�p) */
					}												/*								*/
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// // MH810100(S) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// //					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* �g�p				*/
// 					if(rhs_p->data[1] == 1000){
// 						// �̎��؂ł܂Ƃ߂�׃t���O�����Ă���
// 						PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1001;	/* �g�p			*/
// 					}else if( rhs_p->data[1] >= 2000){
// 						PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 2001;	/* �g�p			*/
// 					}else{
// 						PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* �g�p			*/
// 					}
// // MH810100(E) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
// // MH810100(S) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// //					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* �����ς�(�V�K���Z)*/
// // MH810100(E) K.Onodera 2020/02/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVDISCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVUSAGEDISCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVUSAGEDCOUNT, wrcnt);
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
					wrcnt++;										/*								*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			  case RY_PCO:											/* ��{����߲�޶���				*/
			  case RY_PCO_K:										/* �g��1����߲�޶���			*/
			  case RY_PCO_K2:										/* �g��2����߲�޶���			*/
			  case RY_PCO_K3:										/* �g��3����߲�޶���			*/
				/* �󂫴ر���� */
				for( i=0; (i < WTIK_USEMAX) &&								/* �f�[�^�������				*/
						  (0 != PayData.DiscountData[i].ParkingNo); i++ ){
					;
				}

				wk_dat2 = NTNET_PRI_W;										/* �������=��������߲�޶���	*/
				if( rhs_p->param == RY_PCO ){								/* ��{���ԏꇂ					*/
					wk_dat3 = CPrmSS[S_SYS][1];								/* ���ԏꇂ���					*/
				}else if( rhs_p->param == RY_PCO_K ){						/* �g��1���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][2];								/* ���ԏꇂ���					*/
				}else if( rhs_p->param == RY_PCO_K2 ){						/* �g��2���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][3];								/* ���ԏꇂ���					*/
				}else{														/* �g��3���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][4];								/* ���ԏꇂ���					*/
				}															/*								*/

				/* �V�K�o�^ */
				if( i < WTIK_USEMAX ){										/* ��Ĵر�ɋ󂫂�����ꍇ 		*/
					PayData.DiscountData[i].ParkingNo = wk_dat3;			/* ���ԏꇂ						*/
					PayData.DiscountData[i].DiscSyu = wk_dat2;				/* �������						*/
					PayData.DiscountData[i].DiscNo =						/* ����No.						*/
						(ushort)(PPC_Data_Detail.ppc_data_detail[pricnt].ppc_id >> 16);	/*					*/
					PayData.DiscountData[i].DiscCount =						/*								*/
						(ushort)(PPC_Data_Detail.ppc_data_detail[pricnt].ppc_id & 0x0000ffff);	/*			*/
					PayData.DiscountData[i].Discount = rhs_p->data[2];		/* �����i���p�j���� 			*/
					PayData.DiscountData[i].DiscInfo1 =						/* ���p�㶰�ގc�z				*/
						PPC_Data_Detail.ppc_data_detail[pricnt].pay_after;	/*								*/
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
					PayData.DiscountData[i].uDiscData.ppc_data.pay_befor =	/* �x���O�c�z					*/
						PPC_Data_Detail.ppc_data_detail[pricnt].pay_befor;	/*								*/
					PayData.DiscountData[i].uDiscData.ppc_data.kigen_year =	/* �L�������N					*/
						PPC_Data_Detail.ppc_data_detail[pricnt].kigen_year;	/*								*/
					PayData.DiscountData[i].uDiscData.ppc_data.kigen_mon =	/* �L��������					*/
						PPC_Data_Detail.ppc_data_detail[pricnt].kigen_mon;	/*								*/
					PayData.DiscountData[i].uDiscData.ppc_data.kigen_day =	/* �L��������					*/
						PPC_Data_Detail.ppc_data_detail[pricnt].kigen_day;	/*								*/
					pricnt++;												/*								*/
					wrcnt++;												/*								*/
				}															/*								*/
				break;												/*								*/

// MH321800(S) �������8000�̃Z�b�g���@���C��
//			case RY_EMY:											/* �d�q�}�l�[����				*/
//				/* �V�K�o�^�iSuica�EEdy��1���Z��1���̂ݎg�p�j */
//				/* �V�K�o�^�iSuica��1���Z��1���̂ݎg�p�j */
//				wk_media_Type = Ope_Disp_Media_Getsub(1);					// �d�q�}�l�[�}�̎�� �擾
//				if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {			// Suica
//					wk_DiscSyu1 = NTNET_SUICA_1;							/* ������ʁFSuica���ޔԍ�		*/
//					wk_DiscSyu2 = NTNET_SUICA_2;							/* ������ʁFSuica�x���z�A�c�z	*/
//				}
//				else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		// PASMO
//					wk_DiscSyu1 = NTNET_PASMO_1;							/* ������ʁFPASMO���ޔԍ�		*/
//					wk_DiscSyu2 = NTNET_PASMO_2;							/* ������ʁFPASMO�x���z�A�c�z	*/
//				}
//				else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICOCA) {		// ICOCA
//					wk_DiscSyu1 = NTNET_ICOCA_1;							/* ������ʁFICOCA���ޔԍ�		*/
//					wk_DiscSyu2 = NTNET_ICOCA_2;							/* ������ʁFICOCA�x���z�A�c�z	*/
//				}
//				else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICCARD) {		// IC-Card
//					wk_DiscSyu1 = NTNET_ICCARD_1;							/* ������ʁFICOCA���ޔԍ�		*/
//					wk_DiscSyu2 = NTNET_ICCARD_2;							/* ������ʁFICOCA�x���z�A�c�z	*/
//				}
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////				else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY) {			// Edy
////					wk_DiscSyu1 = NTNET_EDY_1;								/* ������ʁFEdy���ޔԍ�		*/
////					wk_DiscSyu2 = NTNET_EDY_2;								/* ������ʁFEdy�x���z�A�c�z	*/
////				}
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//// MH321800(S) hosoda IC�N���W�b�g�Ή�
//				else if (wk_media_Type == OPE_DISP_MEDIA_TYPE_EC) {			// ���σ��[�_
//					switch (PayData.Electron_data.Ec.e_pay_kind) {			// ���ώ�ʂ���U�蕪��
//					case	EC_EDY_USED:
//						wk_DiscSyu1 = NTNET_EDY_1;							/* ������ʁFEdy���ޔԍ�		*/
//						wk_DiscSyu2 = NTNET_EDY_2;							/* ������ʁFEdy�x���z�A�c�z	*/
//						break;
//					case	EC_NANACO_USED:
//						wk_DiscSyu1 = NTNET_NANACO_1;						/* ������ʁFnanaco���ޔԍ�		*/
//						wk_DiscSyu2 = NTNET_NANACO_2;						/* ������ʁFnanaco�x���z�A�c�z	*/
//						break;
//					case	EC_WAON_USED:
//						wk_DiscSyu1 = NTNET_WAON_1;							/* ������ʁFWAON���ޔԍ�		*/
//						wk_DiscSyu2 = NTNET_WAON_2;							/* ������ʁFWAON�x���z�A�c�z	*/
//						break;
//					case	EC_SAPICA_USED:
//						wk_DiscSyu1 = NTNET_SAPICA_1;						/* ������ʁFSAPICA���ޔԍ�		*/
//						wk_DiscSyu2 = NTNET_SAPICA_2;						/* ������ʁFSAPICA�x���z�A�c�z	*/
//						break;
//					case	EC_KOUTSUU_USED:
//						// ��ʌnIC�J�[�h��Suica��ID���g�p����
//						wk_DiscSyu1 = NTNET_SUICA_1;						/* ������ʁFSuica���ޔԍ�		*/
//						wk_DiscSyu2 = NTNET_SUICA_2;						/* ������ʁFSuica�x���z�A�c�z	*/
//						break;
//					case	EC_ID_USED:
//						wk_DiscSyu1 = NTNET_ID_1;							/* ������ʁFiD���ޔԍ�		*/
//						wk_DiscSyu2 = NTNET_ID_2;							/* ������ʁFiD�x���z�A�c�z	*/
//						break;
//					case	EC_QUIC_PAY_USED:
//						wk_DiscSyu1 = NTNET_QUICPAY_1;						/* ������ʁFQUICPay���ޔԍ�		*/
//						wk_DiscSyu2 = NTNET_QUICPAY_2;						/* ������ʁFQUICPay�x���z�A�c�z	*/
//						break;
//					default	:
//					// ���肦�Ȃ�
//						break;
//					}
//				}
//// MH321800(E) hosoda IC�N���W�b�g�Ή�
//				for( i=0; (i < WTIK_USEMAX) && (0 != PayData.DiscountData[i].ParkingNo); i++ ){
//					;														/* �i�[�ꏊ[�ʒu]��T��			*/
//				}
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				if( wk_media_Type == OPE_DISP_MEDIA_TYPE_EC ){
//					if( i < WTIK_USEMAX-2 ){									/* ��Ĵر�ɂR���̋󂫂�����ꍇ */
//						memset(&wk_DicData, 0x20, sizeof(wk_DicData));
//						wk_DicData.ParkingNo = CPrmSS[S_SYS][1];				/* ���ԏꇂ�F���g�p				*/
//						wk_DicData.DiscSyu = wk_DiscSyu1;						/* ������ʁFSuica���ޔԍ�		*/
//						if( PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement != 1 ){ // �݂Ȃ����ςł͂Ȃ��H
//							memcpy(wk_DicData.CardNo,
//									PayData.Electron_data.Ec.Card_ID, sizeof(wk_DicData.CardNo));
//						}
//
//						memcpy(&PayData.DiscountData[i], &wk_DicData, sizeof(wk_DicData));
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
//
//						i++;
//						PayData.DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	/* ���ԏꇂ�F���g�p				*/
//						PayData.DiscountData[i].DiscSyu = wk_DiscSyu2;			/* ������ʁFSuica�x���z�A�c�z	*/
//						PayData.DiscountData[i].DiscNo = 0;						/* �����敪�F�x�����i�Œ�j		*/
//						PayData.DiscountData[i].DiscCount = 1;					/* �g�p�����F1���i�Œ�j		*/
//						PayData.DiscountData[i].Discount = PayData.Electron_data.Ec.pay_ryo;		/* �����z�F�x�����z */
//						PayData.DiscountData[i].DiscInfo1 = PayData.Electron_data.Ec.pay_after;		/* �������P�F�c�z */
//						/* �������Q */
//						switch (PayData.Electron_data.Ec.e_pay_kind) {		// ���ώ�ʂ���U�蕪��
//							case	EC_EDY_USED:
//								PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Ec.Brand.Edy.DealNo;			/* Edy����ʔ� */
//								break;
//							case	EC_NANACO_USED:
//								PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Ec.Brand.Nanaco.DealNo;		/* nanaco����ʔ� */
//								break;
//							case	EC_WAON_USED:
//								PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Ec.Brand.Waon.point;			/* WAON����t�^�|�C���g */
//								break;
//							case	EC_SAPICA_USED:
//								PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Ec.Brand.Sapica.Details_ID;	/* SAPICA�ꌏ����ID */
//								break;
//							case	EC_KOUTSUU_USED:
//								// �Z�b�g���鍀�ڂȂ�
//								break;
//							case	EC_ID_USED:
//								// TODO:�ڍ׌��܂莟��
//								break;
//							case	EC_QUIC_PAY_USED:
//								// TODO:�ڍ׌��܂莟��
//								break;
//							default	:
//							// ���肦�Ȃ�
//								break;
//						}
//						PayData.DiscountData[i].uDiscData.common.MoveMode = 0;	/* �g�p�^�ԋp�F�ԋp	�i�Œ�j	*/
//						PayData.DiscountData[i].uDiscData.common.DiscFlg = 0;	/* �����ς݁F�V�K���Z�i�Œ�)	*/
//
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//
//						i++;
//						memset(&wk_DicData, 0, sizeof(wk_DicData));
//						wk_DicData.ParkingNo = CPrmSS[S_SYS][1];				/* ���ԏꇂ�F���g�p				*/
//						wk_DicData.DiscSyu = NTNET_INQUIRY_NUM;					/* ������ʁF�⍇���ԍ�	       	*/
//						memcpy( wk_DicData.CardNo, PayData.Electron_data.Ec.inquiry_num, sizeof(wk_DicData.CardNo) );// �⍇���ԍ�
//
//						memcpy(&PayData.DiscountData[i], &wk_DicData, sizeof(wk_DicData));
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
//					}
//				}
//				else {
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//				if( i < WTIK_USEMAX-1 ){									/* ��Ĵر�ɂQ���̋󂫂�����ꍇ */
//					memset(&wk_DicData, 0, sizeof(wk_DicData));
//					wk_DicData.ParkingNo = CPrmSS[S_SYS][1];				/* ���ԏꇂ�F���g�p				*/
//					wk_DicData.DiscSyu = wk_DiscSyu1;						/* ������ʁFSuica���ޔԍ�		*/
//
//					memset(wk_DicData.CardNo, 0x20, sizeof(wk_DicData.CardNo));
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////					if( wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY ){			/* ������ʁFEdy���ޔԍ�		*/
////						BCDtoASCII( PayData.Electron_data.Edy.Card_ID, wk_DicData.CardNo, 8 );
////					}else{
//					{
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//						memcpy( wk_DicData.CardNo,
//								PayData.Electron_data.Suica.Card_ID,
//								sizeof(PayData.Electron_data.Suica.Card_ID));
//					}
//					memcpy(&PayData.DiscountData[i], &wk_DicData, sizeof(wk_DicData));
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
//					i++;
//					PayData.DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	/* ���ԏꇂ�F���g�p				*/
//					PayData.DiscountData[i].DiscSyu = wk_DiscSyu2;			/* ������ʁFSuica�x���z�A�c�z	*/
//					PayData.DiscountData[i].DiscNo = 0;						/* �����敪�F�x�����i�Œ�j		*/
//					PayData.DiscountData[i].DiscCount = 1;					/* �g�p�����F1���i�Œ�j		*/
//					PayData.DiscountData[i].Discount = PayData.Electron_data.Suica.pay_ryo;		/* �����z�F�x�����z */
//					PayData.DiscountData[i].DiscInfo1 = PayData.Electron_data.Suica.pay_after;	/* �������P�F�c�z */
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
////					if( wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY ){
////						PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Edy.deal_no;	/* �������Q�FEdy����ʔ� */
////						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
////					}
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//					PayData.DiscountData[i].uDiscData.common.MoveMode = 0;	/* �g�p�^�ԋp�F�ԋp	�i�Œ�j	*/
//					PayData.DiscountData[i].uDiscData.common.DiscFlg = 0;	/* �����ς݁F�V�K���Z�i�Œ�)	*/
//				}
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//				}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
//				break;
// MH321800(E) �������8000�̃Z�b�g���@���C��

			  case RY_FRE:											/* ��{�񐔌�					*/
			  case RY_FRE_K:										/* �g��1�񐔌�					*/
			  case RY_FRE_K2:										/* �g��2�񐔌�					*/
			  case RY_FRE_K3:										/* �g��3�񐔌�					*/


				/** �����ް��ւ̓o�^ **/

				if( 0xff == (ushort)rhs_p->data[1] )				/* �S�z�����񐔌� 				*/
					wk_dat = 99;
				else
					wk_dat = 0;


				/** �ʐM�d���p�ް��ւ̓o�^ **/

				wk_dat2 = NTNET_FRE;								/* �������=�񐔌�				*/
																	/*								*/
				/* ���ԏ�No get */
				if( rhs_p->param == RY_FRE_K )						/* �g��1���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][2];						/* ���ԏꇂ���					*/
				else if( rhs_p->param == RY_FRE_K2 )				/* �g��2���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][3];						/* ���ԏꇂ���					*/
				else if( rhs_p->param == RY_FRE_K3 )				/* �g��3���ԏꇂ				*/
					wk_dat3 = CPrmSS[S_SYS][4];						/* ���ԏꇂ���					*/
				else												/* ��{���ԏꇂ					*/
					wk_dat3 = CPrmSS[S_SYS][1];						/* ���ԏꇂ���					*/
																	/*								*/
				/* �����ر�ւ̉��Z���� */
				for( i=0; i < WTIK_USEMAX; i++ ){					/*								*/

					if(( PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* ���ԏꇂ��������?	*/
					   ( PayData.DiscountData[i].DiscSyu == wk_dat2 ))		/* ������ʂ�������?	*/
					{												/*								*/
						if( (99 != PayData.DiscountData[i].DiscNo) &&
							(99 != wk_dat) )						/* ���������񐔌����m			*/
						{
							PayData.DiscountData[i].DiscNo +=
							(uchar)(rhs_p->data[0] & 0x0000ffff);	/* �g�p�x�� ���Z				*/

							PayData.DiscountData[i].DiscCount += (uchar)1;		/* �g�p����			*/
							PayData.DiscountData[i].Discount += rhs_p->data[2];	/* �����z			*/
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
							break;									/*								*/
						}
					}												/*								*/
				}													/*								*/

				/* �V�K�o�^ */
				if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){		/* �����Y������					*/
					PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* ���ԏꇂ						*/
					PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* �������						*/
					if( 99 == wk_dat )								/*								*/
						PayData.DiscountData[wrcnt].DiscNo = (ushort)wk_dat;	/* �����敪 (�g�p�x���A99=�S�z����) */
					else											/*								*/
						PayData.DiscountData[wrcnt].DiscNo =		/*								*/
							(uchar)(rhs_p->data[0] & 0x0000ffff);	/* �g�p�x�� ���Z				*/
																	/*								*/
					PayData.DiscountData[wrcnt].DiscCount = (uchar)1;		/* �g�p����				*/
					PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* �����z				*/
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 0;	/* �ԋp				*/
// 					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* �����ς�(�V�K���Z)*/
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
					wrcnt++;										/*								*/
				}													/*								*/
				break;												/*								*/

			case	RY_GNG:											/* ���z���Z						*/
// �s��C��(S) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
			case	RY_GNG_FEE:
			case	RY_GNG_TIM:
// �s��C��(E) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)


				// ���Z�ް��p
				if( wrcnt < WTIK_USEMAX ){										// �����ް����ȉ��̂�
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
					// ���u���Z������(���z�w��)
// �s��C��(S) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
//					if( vl_now == V_DIS_FEE ){
					if( rhs_p->param == RY_GNG_FEE ){
// �s��C��(E) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
						PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏꇂ�F��{���ԏꇂ
						PayData.DiscountData[wrcnt].DiscSyu = g_PipCtrl.stRemoteFee.DiscountKind;		// �������
						PayData.DiscountData[wrcnt].DiscNo = g_PipCtrl.stRemoteFee.DiscountType;		// �����敪
						PayData.DiscountData[wrcnt].DiscCount = g_PipCtrl.stRemoteFee.DiscountCnt;		// �����g�p����
						PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];							// �����z
						PayData.DiscountData[wrcnt].DiscInfo1 = g_PipCtrl.stRemoteFee.DiscountInfo1;	// �������P
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = g_PipCtrl.stRemoteFee.DiscountInfo2;	// �������Q
					}
					// ���u���Z������(���Ɏ����w��)
// �s��C��(S) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
//					else if( vl_now == V_DIS_TIM ){
					else if( rhs_p->param == RY_GNG_TIM ){
// �s��C��(E) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
						PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];						// ���ԏꇂ�F��{���ԏꇂ
						PayData.DiscountData[wrcnt].DiscSyu = g_PipCtrl.stRemoteTime.DiscountKind;		// �������
						PayData.DiscountData[wrcnt].DiscNo = g_PipCtrl.stRemoteTime.DiscountType;		// �����敪
						PayData.DiscountData[wrcnt].DiscCount = g_PipCtrl.stRemoteTime.DiscountCnt;		// �����g�p����
						PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];							// �����z
						PayData.DiscountData[wrcnt].DiscInfo1 = g_PipCtrl.stRemoteTime.DiscountInfo1;	// �������P
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = g_PipCtrl.stRemoteTime.DiscountInfo2;	// �������Q
					}
					// ���z���Z
					else{
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
						PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];	// ���ԏꇂ�F��{���ԏꇂ
						PayData.DiscountData[wrcnt].DiscSyu = NTNET_GENGAKU;		// ������ʁF���z���Z
						PayData.DiscountData[wrcnt].DiscNo = 0;						// �����敪�F�x�����i�Œ�j
						PayData.DiscountData[wrcnt].DiscCount = 1;					// �g�p�����F1���i�Œ�j
						PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];		// �����z
						PayData.DiscountData[wrcnt].DiscInfo1 = 0L;					// �������1(���g�p)
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;// �������2(���g�p)
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
					}
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	// �g�p
// 					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	// �����ς�(�V�K���Z)
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
					wrcnt++;
				}

				break;

			case	RY_FRK:											/* �U�֐��Z						*/

				// �̎���,�ʐ��Z���p
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
				if( PiP_GetFurikaeSts() ){
// �d�l�ύX(S) K.Onodera 2016/12/05 �U�֊z�ύX
//					// �U�֌��ƐU�֐�̗�����ʂ����� or ������ʂ��قȂ��Ă��T�[�r�X��/�|���茔�U�ւ���H
//					if( prm_get(COM_PRM, S_CEN, 32, 1, 1) || ((ryo_buf.syubet+1) == vl_frs.syubetu) ){
//						OrgDiscount = ( vl_frs.price - (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku) );	// �U�֊���
//					}else{
//						OrgDiscount = ( vl_frs.in_price - (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku) );	// �U�֊���
//					}
//					// �U�֐�̒��ԗ����𒴂���H
//					if( OrgDiscount > vl_frs.furikaegaku ){
//						OrgDiscount = vl_frs.furikaegaku;
//					}
// �d�l�ύX(E) K.Onodera 2016/12/05 �U�֊z�ύX
					if( g_PipCtrl.stFurikaeInfo.Remain <= 0 ){
// �d�l�ύX(S) K.Onodera 2016/12/02 �U�֊z�ύX
//						ulong	kabarai = 0;
//						kabarai = (g_PipCtrl.stFurikaeInfo.DestFeeDiscount - vl_frs.furikaegaku);
						// �ߕ��� = �U�֑Ώۊz - �U�։\�z
// �d�l�ύX(E) K.Onodera 2016/12/02 �U�֊z�ύX
						// �����g�p����H
						if( vl_frs.genkin_furikaegaku ){
// �d�l�ύX(S) K.Onodera 2016/12/02 �U�֊z�ύX
//							if( kabarai > vl_frs.genkin_furikaegaku ){
//								kabarai = vl_frs.genkin_furikaegaku;
//							}
// �d�l�ύX(E) K.Onodera 2016/12/02 �U�֊z�ύX
							PayData.FRK_Return = 					// �U�։ߕ�����(�N���W�b�g���̌���ԋ����܂�)
// �d�l�ύX(S) K.Onodera 2016/12/02 �U�֊z�ύX
//							ryo_buf.kabarai = kabarai;				// �U�։ߕ�����(�ԋ����삷��z)
							ryo_buf.kabarai = vl_frs.kabarai;
// �d�l�ύX(E) K.Onodera 2016/12/02 �U�֊z�ύX
// MH810100(S) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// �s��C��(S) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
//							NTNET_Data152Save((void *)(&PayData.FRK_Return), NTNET_152_KABARAI);
//// �s��C��(E) K.Onodera 2016/12/08 #1642 �U�։ߕ�����(�����ɂ�镥���߂�)���A ���Z���f�[�^�̒ޑK�z�ɉ��Z����Ȃ�
// MH810100(E) K.Onodera 2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
						}
						// �N���W�b�g���͓d�q�}�l�[�g�p����H
						else if( vl_frs.card_type && vl_frs.card_furikaegaku ){
// �d�l�ύX(S) K.Onodera 2016/12/02 �U�֊z�ύX
//							if( kabarai > vl_frs.card_furikaegaku ){
//								kabarai = vl_frs.card_furikaegaku;
//							}
// �d�l�ύX(E) K.Onodera 2016/12/02 �U�֊z�ύX
// �d�l�ύX(S) K.Onodera 2016/12/02 �U�֊z�ύX
//							PayData.FRK_Return = kabarai;			// �U�։ߕ�����(�N���W�b�g���̌���ԋ����܂�)
							PayData.FRK_Return = vl_frs.kabarai;
// �d�l�ύX(E) K.Onodera 2016/12/02 �U�֊z�ύX
							ryo_buf.kabarai = 0;					// �U�։ߕ�����(�ԋ����삷��z)=0
							PayData.FRK_RetMod = vl_frs.card_type;	// �}��
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
							NTNET_Data152Save((void *)&PayData.FRK_RetMod, NTNET_152_CARD_FUSOKU_TYPE );
							NTNET_Data152Save((void *)&PayData.FRK_Return, NTNET_152_CARD_FUSOKU );
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
						}
					}
					for( DetailCnt=0; DetailCnt<DETAIL_SYU_MAX; DetailCnt++ ){
						switch( DetailCnt ){
							case 0:	// �U�֐��Z
								PayData.DetailData[DetailCnt].ParkingNo = CPrmSS[S_SYS][1];												// ���ԏꇂ�F��{���ԏꇂ
								PayData.DetailData[DetailCnt].DiscSyu = NTNET_FURIKAE_2;
								PayData.DetailData[DetailCnt].uDetail.Furikae.Pos = (ulong)(LockInfo[vl_frs.lockno - 1].area*10000L
															 									+ LockInfo[vl_frs.lockno - 1].posi);	// �U�֌������
								PayData.DetailData[DetailCnt].uDetail.Furikae.OrgFee = vl_frs.price;									// �U�֌����ԗ���
								PayData.DetailData[DetailCnt].uDetail.Furikae.Total  = vl_frs.furikaegaku;								// �U�֊z
								PayData.DetailData[DetailCnt].uDetail.Furikae.Oiban  = vl_frs.seisan_oiban.i;							// �U�֌����Z�ǔԁ���ʂQ���𗿋���ʂƂ���
																																		// �g�p���Ă��邪�A����ǔԂ�0�`99999�Ȃ̂Ŗ��Ȃ�
								PayData.DetailData[DetailCnt].uDetail.Furikae.Oiban  += (vl_frs.syubetu * 100000L);						// �U�֌��������
								break;
							case 1:	// �U�֐��Z����
								PayData.DetailData[DetailCnt].ParkingNo = CPrmSS[S_SYS][1];							// ���ԏꇂ�F��{���ԏꇂ
								PayData.DetailData[DetailCnt].DiscSyu = NTNET_FURIKAE_DETAIL;
								PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.Mod = vl_frs.card_type;			// �}�̎��
								if( vl_frs.card_type ){
									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkMoney = 0;
// �d�l�ύX(S) K.Onodera 2016/12/05 �U�֊z�ύX
//									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkCard = ( vl_frs.furikaegaku - OrgDiscount );
									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkCard = vl_frs.card_furikaegaku;
// �d�l�ύX(E) K.Onodera 2016/12/05 �U�֊z�ύX
								}else{
// �d�l�ύX(S) K.Onodera 2016/12/05 �U�֊z�ύX
//									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkMoney = ( vl_frs.furikaegaku - OrgDiscount );
									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkMoney = vl_frs.genkin_furikaegaku;
// �d�l�ύX(E) K.Onodera 2016/12/05 �U�֊z�ύX
									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkCard = 0;
								}
// �d�l�ύX(S) K.Onodera 2016/12/05 �U�֊z�ύX
//								PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkDiscount = OrgDiscount;		// �U�֊z(����)
								PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkDiscount = vl_frs.wari_furikaegaku;	// �U�֊z(����)
// �d�l�ύX(E) K.Onodera 2016/12/05 �U�֊z�ύX
								break;
							default:
								continue;
								break;
						}
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DPARKINGNO, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DSYU, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DNO, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DCOUNT, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DISCOUNT, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DINFO1, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DINFO2, DetailCnt);
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
					}
				}else{
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//					PayData.MMPlace = (ulong)(LockInfo[vl_frs.lockno - 1].area*10000L
//												 + LockInfo[vl_frs.lockno - 1].posi);
//					PayData.MMPrice = vl_frs.price;
//					PayData.MMInPrice = vl_frs.in_price;
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
				}
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�

				// ���Z�ް��p
				if( wrcnt < WTIK_USEMAX ){											// �����ް����ȉ��̂�
// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
//					if( PiP_GetFurikaeSts() ){
//						PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// ���ԏꇂ�F��{���ԏꇂ
//						PayData.DiscountData[wrcnt].DiscSyu = NTNET_FURIKAE_2;			// 2002
//						wk_dat3 = (LockInfo[vl_frs.lockno-1].area*10000L) + LockInfo[vl_frs.lockno-1].posi;
//						*(ulong*)&PayData.DiscountData[wrcnt].DiscNo = wk_dat3;								// �����������
//						PayData.DiscountData[wrcnt].Discount = vl_frs.furikaegaku;							// �x���ϋ��z(�U�։\�z)
//						if( g_PipCtrl.stFurikaeInfo.Remain > 0 ){
//							PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = g_PipCtrl.stFurikaeInfo.Remain;	// �s�����z
//						}else{
//// �s��C��(S) K.Onodera 2016/10/13 #1514 �U�֐�Œ���𗘗p����ƒʒm������Œ�����g�p�����ɐ��Z�������̒ޑK���������Ȃ�
////// �s��C��(S) K.Onodera 2016/10/07 #1516 ���Ԋ���������g�p�����U�֐��Z�ŕ����߂������z�����Ȃ�
//////							ryo_buf.turisen = (g_PipCtrl.stFurikaeInfo.DestFeeDiscount - ryo_buf.tyu_ryo);
////							ryo_buf.turisen = (g_PipCtrl.stFurikaeInfo.DestFeeDiscount - vl_frs.furikaegaku);			// �U�֌��U�։\�z - �U�֊z
////// �s��C��(E) K.Onodera 2016/10/07 #1516 ���Ԋ���������g�p�����U�֐��Z�ŕ����߂������z�����Ȃ�
////							if( ryo_buf.turisen > vl_frs.genkin_furikaegaku ){
////								ryo_buf.turisen = vl_frs.genkin_furikaegaku;
////							}
////							PayData.DiscountData[wrcnt].DiscInfo1 = ryo_buf.turisen;									// �ߕ����z(�����̂�)
//							PayData.DiscountData[wrcnt].DiscInfo1 = (g_PipCtrl.stFurikaeInfo.DestFeeDiscount - vl_frs.furikaegaku);
//							// �����g�p����H
//							if( vl_frs.genkin_furikaegaku ){
//								if( PayData.DiscountData[wrcnt].DiscInfo1 > vl_frs.genkin_furikaegaku ){
//									PayData.DiscountData[wrcnt].DiscInfo1 = vl_frs.genkin_furikaegaku;
//								}
//								PayData.FRK_Return = 										// �U�։ߕ�����(�N���W�b�g���̌���ԋ����܂�)
//								ryo_buf.kabarai = PayData.DiscountData[wrcnt].DiscInfo1;	// �U�։ߕ�����(�ԋ����삷��z)
//							}
//							// �N���W�b�g���͓d�q�}�l�[�g�p����H
//							else if( vl_frs.card_type && vl_frs.card_furikaegaku ){
//								if( PayData.DiscountData[wrcnt].DiscInfo1 > vl_frs.card_furikaegaku ){
//									PayData.DiscountData[wrcnt].DiscInfo1 = vl_frs.card_furikaegaku;
//								}
//								PayData.FRK_Return = PayData.DiscountData[wrcnt].DiscInfo1;	// �U�։ߕ�����(�N���W�b�g���̌���ԋ����܂�)
//								ryo_buf.kabarai = 0;										// �U�։ߕ�����(�ԋ����삷��z)=0
//								PayData.FRK_RetMod = vl_frs.card_type;						// �}��
//							}
//// �d�l�ύX(S) K.Onodera 2016/10/28 �U�։ߕ������ƒޑK�؂蕪��
//
//// �d�l�ύX(E) K.Onodera 2016/10/28 �U�։ߕ������ƒޑK�؂蕪��
//// �s��C��(E) K.Onodera 2016/10/13 #1514 �U�֐�Œ���𗘗p����ƒʒm������Œ�����g�p�����ɐ��Z�������̒ޑK���������Ȃ�
//						}
//					}
//					else{
//// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
					if( PiP_GetFurikaeSts() == 0 ){
// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
					PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// ���ԏꇂ�F��{���ԏꇂ
					PayData.DiscountData[wrcnt].DiscSyu = NTNET_FURIKAE;			// ������ʁF���z���Z

					wk_dat3 = (LockInfo[vl_frs.lockno-1].area*10000L) + LockInfo[vl_frs.lockno-1].posi;
					*(ulong*)&PayData.DiscountData[wrcnt].DiscNo = wk_dat3;

					PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];			// �����z
					PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)rhs_p->data[4];	// �������1�F�U�֌� ���Z��������
					PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = CountSel(&vl_frs.seisan_oiban);// �������2�F�U�֌� ���Z�ǔ�
// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
					}
// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;		// �g�p
// 					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;		// �����ς�(�V�K���Z)
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
					wrcnt++;
				}
				break;

			case	RY_SSS:
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//				// �̎���,�ʐ��Z���p
//				PayData.MMSyubetu = vl_frs.syubetu;									// �U�֌��E�������
//				PayData.MMPlace = (ulong)(LockInfo[vl_frs.lockno - 1].area*10000L
//											 + LockInfo[vl_frs.lockno - 1].posi);
//				PayData.MMPrice = vl_frs.price;										// �ԈႢ���Z�F���ԗ���
//				PayData.MMInPrice = syusei[vl_frs.lockno-1].gen;					// �ԈႢ���Z�F���������z
//				PayData.MMRwari = syusei[vl_frs.lockno-1].sy_wari;					// �ԈႢ���Z�F�����������z
//				PayData.MMPwari = vl_frs.seisan_oiban;								// �C���� ���Z�ǔ�
//
//				if(prm_get(COM_PRM, S_TYP, 62, 1, 2) == 1){							// �����ʂ̂ݎ��Ԋ������܂�
//					if(vl_frs.syubetu == (char)(ryo_buf.syubet+1)){					// �C�����ƏC����Ŏ�ʂ������H
//						PayData.MMRwari += syusei[vl_frs.lockno-1].sy_time;			// �ԈႢ���Z�F���Ԋ������z
//					}
//				}else{																// ���Ԋ������܂�
//					// �ݒ��0�`2�͈̔͂�0�̎��͂����ɂ͗��Ȃ����߁A1�ȊO�̔��f�Ƃ���
//					PayData.MMRwari += syusei[vl_frs.lockno-1].sy_time;				// �ԈႢ���Z�F���Ԋ������z
//				}
//				if( (syusei[vl_frs.lockno-1].infofg & SSS_ZENWARI)&&				// �S�z��������
//					(prm_get(COM_PRM, S_TYP, 98, 1, 2)) ){							// �S�z��������
//					PayData.Zengakufg |= 0x01;										// �S�z����bit0��1�Ƃ���
//				}
//
//				// ���Z�ް��p
//				if( wrcnt < (WTIK_USEMAX-2) ){						// �����ް����ȉ��̂�
//					// �����G���A1
//					PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// ���ԏꇂ�F��{���ԏꇂ
//					PayData.DiscountData[wrcnt].DiscSyu = NTNET_SYUSEI_1;			// ������ʁF�C�����Z(92)
//					wk_dat3 = (LockInfo[vl_frs.lockno-1].area*10000L) + LockInfo[vl_frs.lockno-1].posi;
//					*(ulong*)&PayData.DiscountData[wrcnt].DiscNo = wk_dat3;
//
//					PayData.DiscountData[wrcnt].Discount = 0;						// �����z�F�C���� 0�˕��ߊz(ntnetctrl�Ŋi�[)
//					PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)rhs_p->data[4];	// �������1�F�U�֌� ���Z��������
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = CountSel(&vl_frs.seisan_oiban);// �������2�F�U�֌� ���Z�ǔ�
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
//					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;		// �g�p
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;		// �����ς�(�V�K���Z)
//					wrcnt++;
//
//					// �����G���A2
//					PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// ���ԏꇂ�F��{���ԏꇂ
//					PayData.DiscountData[wrcnt].DiscSyu = NTNET_SYUSEI_2;			// ������ʁF�C�����Z(93)
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
//					if( PayData.Zengakufg&0x01 ){									// �S�z�����L��
//						PayData.DiscountData[wrcnt].DiscNo = 1;						// �����敪�F
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
//					}
//					PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];			// �����z�F�C���� �U�֊z
//					PayData.DiscountData[wrcnt].DiscInfo1 = PayData.MMInPrice;		// �������1�F�U�֌� �����z
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = PayData.MMRwari;// �������2�F�U�֌� �����z
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
//					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;		// �g�p
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;		// �����ς�(�V�K���Z)
//					wrcnt++;
//
//					// �����G���A3
//					PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// ���ԏꇂ�F��{���ԏꇂ
//					PayData.DiscountData[wrcnt].DiscSyu = NTNET_SYUSEI_3;			// ������ʁF�C�����Z(94)
//					PayData.DiscountData[wrcnt].Discount = 0;						// �����z�F�C���� 0�˖������z(ntnetctrl�Ŋi�[)
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
//					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;		// �g�p
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;		// �����ς�(�V�K���Z)
//					wrcnt++;
//				}
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
				break;
// MH321800(S) �������8000�̃Z�b�g���@���C��
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//			case RY_CRE:
//				// ���σ��[�_�̸ڼޯČ��ς̂ݗL��
//				if( isEC_USE() ){
//					for( i=0; (i < WTIK_USEMAX) && (0 != PayData.DiscountData[i].ParkingNo); i++ ){
//						;														/* �i�[�ꏊ[�ʒu]��T��			*/
//					}
//					if( i < NTNET_DIC_MAX ){									/* ��Ĵر��1���̋󂫂�����ꍇ */
//						memset(&wk_DicData, 0, sizeof(wk_DicData));
//						wk_DicData.ParkingNo = CPrmSS[S_SYS][1];				/* ���ԏꇂ�F���g�p				*/
//						wk_DicData.DiscSyu = NTNET_INQUIRY_NUM;					/* ������ʁF�⍇���ԍ�	       	*/
//						// �ڼޯČ��ς͖⍇���ԍ����Ȃ���0x20����
//						memset( &wk_DicData.CardNo[0], 0x20, sizeof(wk_DicData.CardNo) );
//						memcpy(&PayData.DiscountData[i], &wk_DicData, sizeof(wk_DicData));
//
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
//					}
//				}
//				break;
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH321800(E) �������8000�̃Z�b�g���@���C��
			}														/*								*/
		}															/*								*/
		CardUseSyu = wrcnt;											/* 1���Z�̊�����ނ̌����ر		*/
	}																/*								*/
ryo_cal_10:
																	/*								*/
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#if 1	// ��������F�N���E�h�����v�Z��ryo_buf_calc(cal_cloud/cal_cloud.c)�ɑ����B
		// ���C������ۂ�ryo_buf_calc�����m�F���邱�ƁB
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
	if( ryo_buf.ryo_flg <= 1 )										/* ����g�p�������Z�H			*/
	{																/*								*/
		if(	ryo_buf.tyu_ryo >= ryo_buf.waribik )					/*								*/
		{															/*								*/
			ryo_buf.kazei = ryo_buf.tyu_ryo -	ryo_buf.waribik;	/* �ېőΏۊz�����ԗ��� �| �����z�v	*/
		} else {													/*								*/
			ryo_buf.kazei = 0l;										/*								*/
		}															/*								*/
	} else {														/* ����g�p						*/
		if(	ryo_buf.tei_ryo >= ryo_buf.waribik )					/*								*/
		{															/*								*/
			ryo_buf.kazei = ryo_buf.tei_ryo -	ryo_buf.waribik;	/* �ېőΏۊz����������ԗ��� �| �����z�v	*/
		} else {													/*								*/
			ryo_buf.kazei = 0l;										/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	if( CPrmSS[S_CAL][19] )											/* �ŗ��ݒ肠��					*/
	{																/*								*/
		if( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2L )					/* �O��?						*/
		{															/*								*/
			sot_tax = ryo_buf.tax;									/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.credit.pay_ryo != 0 )
	{
		if( ryo_buf.zankin == 0 ) {
			if ((ryo_buf.credit.cre_type == CREDIT_CARD) || (ryo_buf.credit.cre_type == CREDIT_HOJIN)){
				// tyu_ryo �͎��ԑђ���ł̏o�Ɏ��ȂǁA�������ݸނ� 0 �̏ꍇ�����肦��A dsp_ryo ���g��.
				if( ryo_buf.dsp_ryo < ( ryo_buf.nyukin + ryo_buf.credit.pay_ryo ) ){
					//�����ƁA�N���W�b�g�F���̂���Ⴂ���ɋN���肤��
					ryo_buf.turisen =  ( ryo_buf.nyukin + ryo_buf.credit.pay_ryo ) - ryo_buf.dsp_ryo;
				}
				ryo_buf.dsp_ryo = 0L;
			}
			else {
				// �T�[�r�X���A�v���y�C�h�J�[�h�Ȃǂ̏ꍇ�Ɠ������A��������dsp_ryo�Ɏc��
				// ope��invcrd()�ŒޑK���v�Z������B
				// �i�N���W�b�g�J�[�h����iD�Ɠ����ŗǂ��͂��������@��Ƃ̌݊����l�����ĕς��Ȃ��j
				if (ryo_buf.dsp_ryo >= ryo_buf.credit.pay_ryo) {
					ryo_buf.dsp_ryo -= ryo_buf.credit.pay_ryo;
				}
				else {
					ryo_buf.dsp_ryo = 0;
				}
			}
		}
		goto L_RyoCalCrePay;
	}

	switch( ryo_buf.ryo_flg )										/*								*/
	{																/*								*/
		case 0:														/* �ʏ�(����g�p����)���Z		*/
			if(( ryo_buf.tyu_ryo + sot_tax )						/* ���ԗ����{�O�� �� �����z�{�����z�v?	*/
					< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay ))	/*								*/
			{														/*								*/
				dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay )	/*								*/
						- ( ryo_buf.tyu_ryo + sot_tax );			/*								*/
// MH322914(S) K.Onodera 2016/12/12 AI-V�Ή��F�U�֐��Z
				if( OPECTL.op_faz == 2 && PiP_GetFurikaeSts() ){
					// �������U�ւŒ�����g�p�����ɒޑK����̐��Z���s�����ꍇ
					// �ޑK�͎Z�o�ς݂̂��ߔ�����
					break;
				}
// MH322914(E) K.Onodera 2016/12/12 AI-V�Ή��F�U�֐��Z
				if( dat <= ryo_buf.nyukin )							/*								*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin - dat;			/* ������������߂�				*/
				}													/*								*/
			}														/*								*/

			if( e_inflg == 1 ) {									// �d�q�}�l�[�ɂ�������
				if(( ryo_buf.tyu_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin ))
				{													/*								*/
					ryo_buf.dsp_ryo = ( ryo_buf.tyu_ryo + sot_tax  )/* �\��������(���ԗ����{�O��)	*/
									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin );	/*	�|�����z�v	*/
				} else {											/*								*/
					ryo_buf.dsp_ryo = 0l;							/* �\���������O					*/
					if( ryo_buf.nyukin != 0 )						/* �����L��?					*/
					{												/*								*/
						ryo_buf.turisen = ryo_buf.nyukin;			/* �S�z�߂�						*/
					}												/*								*/
				}													/*								*/
			}else{									 				// �����z = ���ԗ���
				if(( ryo_buf.tyu_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin ))
				{													/*								*/
					ryo_buf.dsp_ryo = ( ryo_buf.tyu_ryo + sot_tax  )/* �\��������(���ԗ����{�O��)	*/
// MH810100(S) m.saito 2020/05/26 �Ԕԃ`�P�b�g���X(#4184 �Đ��Z���̕s��Ή�)
//// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
////									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin );	/*	�|�����z�v	*/
//									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin + ryo_buf.zenkai );	/*	�|�����z�v	*/
//// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin );	/*	�|�����z�v	*/
// MH810100(E) m.saito 2020/05/26 �Ԕԃ`�P�b�g���X(#4184 �Đ��Z���̕s��Ή�)
				} else {											/*								*/
					ryo_buf.dsp_ryo = 0l;							/* �\���������O					*/
					if( ryo_buf.nyukin != 0 )						/* �����L��?					*/
					{												/*								*/
						ryo_buf.turisen = ryo_buf.nyukin;			/* �S�z�߂�						*/
					}												/*								*/
				}													/*								*/
			}
			break;													/*								*/
		case 1:														/* �C��(����g�p����)���Z		*/
			if(( ryo_buf.tyu_ryo + sot_tax ) >= ( ryo_buf.waribik + c_pay ))	/*					*/
			{														/*								*/
				ryo_buf.dsp_ryo = ( ryo_buf.tyu_ryo + sot_tax )		/* �\��������(���ԗ����{�O��)	*/
								 	- ( ryo_buf.waribik + c_pay );	/*					�|�����z�v	*/
			} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;								/* �\���������O					*/
				if( ryo_buf.nyukin != 0 )							/* �����L��?					*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin;				/* �S�z�߂�						*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
		case 2:														/* �ʏ�(����g�p)���Z			*/

			if( e_inflg == 1 ) {									// �d�q�}�l�[�ɂ�������
				if(( ryo_buf.tei_ryo + sot_tax )						/* ������ԗ����{�O�� �� �����z�{�����z�v�H*/
						< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin))
				{														/*								*/
					dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin)
						- ( ryo_buf.tei_ryo + sot_tax );				/*								*/
					if( dat <= ryo_buf.nyukin )							/*								*/
					{													/*								*/
						ryo_buf.turisen = ryo_buf.nyukin - dat;			/* ������������߂�				*/
					}													/*								*/
				}														/*								*/
				if(( ryo_buf.tei_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin))
				{														/*								*/
					ryo_buf.dsp_ryo = ( ryo_buf.tei_ryo + sot_tax  )	/* �\��������(������ԗ����{�O��)	*/
									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin);	/*		�|�����z�v	*/
				} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;									/* �\���������O					*/
					if( ryo_buf.nyukin != 0 )							/* �����L��?					*/
					{													/*								*/
						ryo_buf.turisen = ryo_buf.nyukin;				/* �S�z�߂�						*/
					}													/*								*/
				}														/*								*/
			}else{									 				// �����z = ���ԗ���
			if(( ryo_buf.tei_ryo + sot_tax )						/* ������ԗ����{�O�� �� �����z�{�����z�v�H*/
						< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin))	/*			*/
				{														/*								*/
					dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin)	/*				*/
						- ( ryo_buf.tei_ryo + sot_tax );				/*								*/
					if( dat <= ryo_buf.nyukin )							/*								*/
					{													/*								*/
						ryo_buf.turisen = ryo_buf.nyukin - dat;			/* ������������߂�				*/
					}													/*								*/
				}														/*								*/
				if(( ryo_buf.tei_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin))	/*	*/
				{														/*								*/
					ryo_buf.dsp_ryo = ( ryo_buf.tei_ryo + sot_tax  )	/* �\��������(������ԗ����{�O��)	*/
									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin);	/*		�|�����z�v	*/
				} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;									/* �\���������O					*/
					if( ryo_buf.nyukin != 0 )							/* �����L��?					*/
					{													/*								*/
						ryo_buf.turisen = ryo_buf.nyukin;				/* �S�z�߂�						*/
					}													/*								*/
				}														/*								*/
			}
			break;													/*								*/
		case 3:														/* �C��(����g�p)���Z			*/
			if(( ryo_buf.tei_ryo + sot_tax ) >= (ryo_buf.waribik + c_pay ))	/*						*/
			{														/*								*/
				ryo_buf.dsp_ryo = ( ryo_buf.tei_ryo + sot_tax )		/* �\��������(������ԗ����{�O��)	*/
								  - ( ryo_buf.waribik + c_pay );	/*					�|�����z�v	*/
			} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;								/* �\���������O					*/
				if( ryo_buf.nyukin != 0 )							/* �����L��?					*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin;				/* �S�z�߂�						*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
	}																/*								*/
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#endif	// �����܂ŁF�N���E�h�����v�Z��ryo_buf_calc(cal_cloud/cal_cloud.c)�ɑ����B
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

L_RyoCalCrePay:
	WACDOG;// RX630��WDG�X�g�b�v���o���Ȃ��d�l�̂���WDG���Z�b�g�ɂ���
	memcpy( &car_ot, &Adjustment_Beginning, sizeof( car_ot ) );		/*								*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
																	/*								*/
#if SYUSEI_PAYMENT
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �C�����Z�p�����v�Z����Ҳ�																	   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: sryo_cal( num )																   |*/
/*| PARAMETER	: num	; ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2005-07-12																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sryo_cal( ushort num )
{
	cm27();																// �׸޸ر

	ryo_cal( 100, num );												// �C�����Z�����v�Z

	if( syusei[num - 1].tei_id != 0 ){									// �ԈႦ������g�p?
		vl_passet( num );
		ryo_cal( 1, num );												// ����������v�Z
	}
}
#endif		// SYUSEI_PAYMENT

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �T�[�r�X�^�C�����`�F�b�N																	   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: SvsTimChk( ushort no )														   |*/
/*|				  																				   |*/
/*| PARAMETER	: no = �ׯ�߁^ۯ����uNo.(1�`324)												   |*/
/*|				  																				   |*/
/*| RETURN VALUE: �`�F�b�N����																	   |*/
/*|				  	OK:���޽��ђ�																   |*/
/*|				  	NG:���޽��яI��																   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Motohashi(AMANO)															   |*/
/*| Date		: 2006-10-13																	   |*/
/*| Update		: 																				   |*/
/*|				  																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
char	SvsTimChk( ushort no )
{
	char			ans = NG;			// �������ʁi�߂�l�j
	flp_com			*pk_info;			// ���Ԉʒu����߲���
	long			svstime;			// ���޽��ѐݒ�l
	uchar			syu;				// �Ԏ�
	struct	CAR_TIM wk_intm;										// ���Ɏ���
	struct	CAR_TIM	now_time;			// ���ݎ���
	short			ret;				// ������r����
// MH810100(S) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)
//	long	svtm,uptm;
// MH810100(E) K.Onodera 2019/12/05 �Ԕԃ`�P�b�g���X(�����v�Z)

	if( (no > 0) && (no <= LOCK_MAX) ){						// ���uNo.����

		pk_info = &FLAPDT.flp_data[no-1];					// ���Ԉʒu����߲������

		if( pk_info->nstat.bits.b00 == 1 ){					// �Ԏ��������
			// �ԗ�����

			// ���Ɏ����ް��擾

			wk_intm.year = pk_info->year;							// ���Ɂi�N�j
			wk_intm.mon  = pk_info->mont;							// ���Ɂi���j
			wk_intm.day  = pk_info->date;							// ���Ɂi���j
																	// ���Ɂi�j���j
			wk_intm.week = (char)youbiget( wk_intm.year, (short)wk_intm.mon, (short)wk_intm.day );
			wk_intm.hour = pk_info->hour;							// ���Ɂi���j
			wk_intm.min  = pk_info->minu;							// ���Ɂi���j

			// ���ݎ����ް��擾
			now_time.year = CLK_REC.year;					// ���݁i�N�j
			now_time.mon  = CLK_REC.mont;					// ���݁i���j
			now_time.day  = CLK_REC.date;					// ���݁i���j
			now_time.week = CLK_REC.week;					// ���݁i�j���j
			now_time.hour = CLK_REC.hour;					// ���݁i���j
			now_time.min  = CLK_REC.minu;					// ���݁i���j

			// ���޽��ѐݒ�l�擾
			if( pk_info->lag_to_in.BIT.LAGIN == ON ){		// ׸���ϰ��ѱ�Ăɂ��ē��ɁH
				svstime = 0;								// Yes�����޽��т͖���
			}
			else{
				if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){	// �S���ʻ��޽��сH
					svstime = CPrmSS[S_STM][2];				// ���ʻ��޽��ѐݒ�l�擾
				}else{
					syu = LockInfo[no-1].ryo_syu;
					svstime = CPrmSS[S_STM][5+(3*(syu-1))];	// ��ʖ����޽��ѐݒ�l�擾
				}

// MH322914 (s) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)
//				if(	( pk_info->mode == FLAP_CTRL_MODE3 )||( pk_info->mode == FLAP_CTRL_MODE4 ) )
//				{
//					svtm = svstime * 60;
//
//					if( no >= LOCK_START_NO)
//					{
//						uptm = (long)(( CPrmSS[S_TYP][69] / 100 ) * 60 );		// ����b�ɕϊ����i�[
//						uptm += (long)( CPrmSS[S_TYP][69] % 100 );				// �b�̐ݒ�����Z���S�Ă�b���Z����
//					}
//					else
//					{
//						uptm = (long)(( CPrmSS[S_TYP][118] / 100 ) * 60 );		// ����b�ɕϊ����i�[
//						uptm += (long)( CPrmSS[S_TYP][118] % 100 );				// �b�̐ݒ�����Z���S�Ă�b���Z����
//					}
//					if( svtm == uptm )
//					{
//						if( prm_get( COM_PRM,S_TYP,62,1,2 ) ) { 				// �C�����Z�@�\����ݒ�
//							if( syusei[no-1].sei != 2 ){						// �����Ԏ��łȂ�
//								svstime = 0;									// ���޽��і���
//							}													//
//						}else{													// �C�����Z�@�\�Ȃ��ݒ�ł���Ε����Ԏ��Ȃ�
//							svstime = 0;										// ���޽��і���
//						}
//					}
//				}
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//				if( prm_get( COM_PRM, S_TYP, 68, 1, 5 ) == 0 ) {
//					// ���b�N�^�C�}�[�ƃT�[�r�X�^�C�����������Ԃ̎��ɃT�[�r�X�^�C�������b�N�܂łƂ���
//					if(	( pk_info->mode == FLAP_CTRL_MODE3 )||( pk_info->mode == FLAP_CTRL_MODE4 ) )
//					{
//						svtm = svstime * 60;
//
//						if( no >= LOCK_START_NO)
//						{
//							uptm = Carkind_Param(ROCK_CLOSE_TIMER, LockInfo[no-1].ryo_syu, 5,1);
//						}
//						else
//						{
//							uptm = Carkind_Param(FLAP_UP_TIMER, LockInfo[no-1].ryo_syu, 5,1);
//						}
//						uptm = (long)(((uptm/100)*60) + (uptm%100));
//
//						if( svtm == uptm )
//						{
//							if( prm_get( COM_PRM,S_TYP,62,1,2 ) ) { 				// �C�����Z�@�\����ݒ�
//								if( syusei[no-1].sei != 2 ){						// �����Ԏ��łȂ�
//									svstime = 0;									// ���޽��і���
//								}													//
//							}else{													// �C�����Z�@�\�Ȃ��ݒ�ł���Ε����Ԏ��Ȃ�
//								svstime = 0;										// ���޽��і���
//							}
//						}
//					}
//				}
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
// MH322914 (e) kasiyama 2016/07/12 ���b�N�^�C�����T�[�r�X�^�C���܂ŉ�������[���ʃo�ONo.1211](MH341106)
			}

			ec70( &wk_intm, svstime );										// ���Ɏ����ɻ��޽��т����Z
			ret = ec64( &now_time, &wk_intm );								// ������r


			if( ret != -1 ){
				// ���ݎ��� �� ���Ɏ���+���޽���
				if( svstime != 0 )
					ans = OK;								// ���޽��ђ�
			}
		}
	}
	return( ans );
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���u���Z���`�F�b�N																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: is_paid_remote(Receipt_data*)													   |*/
/*| PARAMETER	: num	; 1���Z����ް�															   |*/
/*| RETURN VALUE: wari_data�̲��ޯ��/�Ȃ��ꍇ-1													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
int	is_paid_remote(Receipt_data* dat)
{
	int i;
	
	for (i = 0; i < WTIK_USEMAX; i++) {
		if (dat->DiscountData[i].DiscSyu == NTNET_GENGAKU) {
			return i;
		}
		if (dat->DiscountData[i].DiscSyu == NTNET_FURIKAE) {
			return i;
		}
	}
	
	return -1;
}
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
int	is_ParkingWebFurikae( Receipt_data* dat )
{
	int i;

	for (i = 0; i < DETAIL_SYU_MAX; i++) {
		if (dat->DetailData[i].DiscSyu == NTNET_FURIKAE_2) {
			return i;
		}
	}

	return -1;
}
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �V�C�����Z�i�C�����Z���`�F�b�N�j															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: is_paid_syusei(Receipt_data*)													   |*/
/*| PARAMETER	: num	; 1���Z����ް�															   |*/
/*| RETURN VALUE: wari_data�̲��ޯ��/�Ȃ��ꍇ-1													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
short	is_paid_syusei(Receipt_data* dat)
{
	short i;
	
	for (i = 0; i < WTIK_USEMAX; i++) {
		if (dat->DiscountData[i].DiscSyu == NTNET_SYUSEI_1) {
			return i;
		}
	}
	
	return -1;
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| �V�C�����Z�isyusei�ޯ̧����ryo_buf�ɾ�āj													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ryo_SyuseiRyobufSet(void)	     												   |*/
/*| PARAMETER	:                            													   |*/
/*| RETURN VALUE:                            													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
void	ryo_SyuseiRyobufSet( void )
{
uchar	set;

	if( OPECTL.Pay_mod == 2 ){													// �C�����Z�̎�
		ryo_buf.mis_ryo	= syusei[vl_frs.lockno-1].gen;							// �ԈႢ�����̎��z���
		ryo_buf.mis_wari= syusei[vl_frs.lockno-1].sy_wari;						// �ԈႢ���������z���

		set = (uchar)prm_get(COM_PRM, S_TYP, 62, 1, 2);							// ���Ԋ����܂ށH
		if( set == 1){															// �����ʂ̂ݎ��Ԋ������܂�
			if(syusei[vl_frs.lockno-1].syubetu == (char)(ryo_buf.syubet+1)){	// �C�����ƏC����Ŏ�ʂ������H
				ryo_buf.mis_wari += syusei[vl_frs.lockno-1].sy_time;			// �ԈႢ���Z�F���Ԋ������z
			}
		}else if( set == 2){													// ���Ԋ������܂�
			ryo_buf.mis_wari += syusei[vl_frs.lockno-1].sy_time;				// �ԈႢ���Z�F���Ԋ������z
		}

		if( syusei[vl_frs.lockno-1].infofg & SSS_ZENWARI ){						// �S�z�����g�p���Ă���?
			if( prm_get(COM_PRM, S_TYP, 98, 1, 2) ){							// �S�z��������
				ryo_buf.mis_zenwari = 1;										// �S�z�����׸�ON
			}
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| ��ʖ��ɐݒ肷��̂����� 				       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Get_Pram_Syubet	                                       |*/
/*| PARAMETER    : no  (�ݒ荀��)                                          |*/
/*| RETURN VALUE : set_mod                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2010-09-07                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
char Get_Pram_Syubet( char no ){
	char	set_mod;
	if(no <= 5){
		set_mod = (char)prm_get(COM_PRM, S_LTM, 1, 1, (char)(6-no));		
		if( (no == 4)||(no == 5) ){
			set_mod = 0;
		}
	}
	else{
		set_mod = (char)prm_get(COM_PRM, S_LTM, 2, 1, (char)(12-no));
		if( no == 6 ){
			set_mod = 0;
		}
	}
	return set_mod;
}

/*[]----------------------------------------------------------------------[]*/
/*| ��ʖ��̐ݒ�l�擾 						       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Get_Pram_Syubet	                                       |*/
/*| PARAMETER    : no  (�ݒ荀��)                                          |*/
/*|              : syu (�Ԏ�)                                              |*/
/*|              : len (����)                                              |*/
/*|              : pos (�ݒ�ʒu)�����g�p�̏ꍇ�͂O���w�肷��              |*/
/*| RETURN VALUE : param(�ݒ�l)                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| note     ��ʖ��ƑS�Ԏ틤�ʂŐݒ肪������Ă��鍀�ڂ̐ݒ�l���擾����  |*/
/*|          �S�Ԏ틤�ʐݒ�̏ꍇ���O�^�C���A�b�v��̗����͐ݒ���Q�Ƃ��Ȃ�|*/
/*| Date         : 2010-09-07                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
long Carkind_Param( char no, char syu, char len, char pos)
{
	long	param;
	short	sec,adr;
	
	adr = 0;
	sec = 0;
	syu -= 1;							// ��ʂ͂P����w�肳��Ă���̂�-1����
	if(Get_Pram_Syubet(no)){			// ��ʖ�
		sec = S_LTM;					// �Z�N�V����
		switch(no){
		case FLAP_UP_TIMER:				// �t���b�v�㏸�^�C�}�[
			adr = 31+syu;				// �A�h���X
			break;
		case ROCK_CLOSE_TIMER:			// ���b�N�^�C�}�[
			adr = 11+syu;				// �A�h���X
			break;
		case FLP_ROCK_INTIME:			// ���b�N�E�t���b�v�㏸�E���O�^�C��������
			adr = 51+syu;				// �A�h���X
			break;
		case TYUU_DAISUU_MODE:			// ���ԑ䐔�Ǘ�����
			adr = 91+syu;				// �A�h���X
			break;
		case IN_CHK_TIME:				// ���ɔ���^�C��
			adr = 111+syu;				// �A�h���X
			break;
		case LAG_PAY_PTN:				// ���O�^�C���A�b�v��̗���
			switch(syu){				// ��ʖ��̃A�h���X�̎w��
				case 0:
				case 1:
				case 2:
					adr = 71;
					break;
				case 3:
				case 4:
				case 5:
					adr = 72;
					break;
				case 6:
				case 7:
				case 8:
					adr = 73;
					break;
				case 9:
				case 10:
				case 11:
					adr = 74;
					break;
			}
			pos = (char)(6-((syu%3)*2)-1);	// �ݒ�ʒu�̎Z�o
			break;
		case WARI_PAY_PTN:				// ���Ԋ���/�����̗���
			switch(syu){				// ��ʖ��̃A�h���X�̎w��
				case 0:
				case 1:
				case 2:
					adr = 81;
					break;
				case 3:
				case 4:
				case 5:
					adr = 82;
					break;
				case 6:
				case 7:
				case 8:
					adr = 83;
					break;
				case 9:
				case 10:
				case 11:
					adr = 84;
					break;
			}
			pos = (char)(6-((syu%3)*2)-1);	// �ݒ�ʒu�̎Z�o
			break;
		}
	}
	else{								// �S�Ԏ틤��
		switch(no){
		case FLAP_UP_TIMER:				// �t���b�v�㏸�^�C�}�[
			sec	= S_TYP;
			adr = 118;
			break;
		case ROCK_CLOSE_TIMER:			// ���b�N�^�C�}�[
			sec	= S_TYP;
			adr = 69;
			break;
		case FLP_ROCK_INTIME:			// ���b�N�E�t���b�v�㏸�E���O�^�C��������
			sec	= S_TYP;
			adr = 68;
			break;
		case TYUU_DAISUU_MODE:			// ���ԑ䐔�Ǘ�����
			sec = S_SYS;
			adr = 39;
			break;
		case IN_CHK_TIME:				// ���ɔ���^�C��
			sec = S_TYP;
			adr = 47;
			break;
		case LAG_PAY_PTN:				// ���O�^�C���A�b�v��̗���
			/* �S�Ԏ틤�ʐݒ�̏ꍇ���O�^�C���A�b�v��̗����͊�{����Ƃ�ŌŒ�Ƃ��� */
			return 0;

		case WARI_PAY_PTN:				// ���Ԋ���/�����̗���
			sec = S_CAL;
			adr = 17;
			break;
		}
	}
	param = prm_get(COM_PRM,sec,adr,len,pos);
	return param;
}

// MH810100(S) Y.Watanabe 2020/02/26 �Ԕԃ`�P�b�g���X(���������Ή�)
//[]----------------------------------------------------------------------[]
///	@brief		�����̗����v�Z�֘A�ϐ��N���A
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void ClearRyoCalDiscInfo( void )
{
	struct REQ_RKN *rhs_p = NULL;
	ushort i, cnt = 0;

	// �����v�Z�̊����֘A�ϐ��N���A
	tol_dis		 = 0;
	discount	 = 0;
	mae_dis		 = 0;
	discount_tyushi = 0;
	tyushi_mae_dis = 0;
	jik_dtm		 = 0;

	cnt = rhspnt;
	for( i=1; i<cnt; i++ ){
		rhs_p = &req_rhs[cnt - i];
// MH810100(S) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
//		if( rhs_p->param == RY_TSA || rhs_p->param == RY_PKC ){
		if( rhs_p->param == RY_TSA || rhs_p->param == RY_PKC || rhs_p->param == RY_RWR ){
// MH810100(E) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
			break;
		}
		memset( rhs_p, 0, sizeof(struct REQ_RKN) );
		rhspnt--;
	}
	tb_number = rhspnt;
}
// MH810100(E) Y.Watanabe 2020/02/26 �Ԕԃ`�P�b�g���X(���������Ή�)
// MH810100(S) 2020/06/10 #4216�y�A���]���w�E�����z�Ԏ�ݒ��ʎ��Ԋ����ݒ莞��QR����(���Ԋ���)���s���Ă��K�p����Ȃ�
void IncCatintime( struct	CAR_TIM		*wok_tm )
{
	if(su_jik_dtm){
		ec70( wok_tm,(long)su_jik_dtm );
	}
	
}
// MH810100(E) 2020/06/10 #4216�y�A���]���w�E�����z�Ԏ�ݒ��ʎ��Ԋ����ݒ莞��QR����(���Ԋ���)���s���Ă��K�p����Ȃ�
