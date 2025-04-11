/*[]----------------------------------------------------------------------------------------------[]*/
/*| �W�v																						   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"prm_tbl.h"
#include	"mem_def.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"flp_def.h"
#include	"ope_def.h"
#include	"cnm_def.h"
#include	"pri_def.h"
#include	"mnt_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"LKmain.h"
#include	"suica_def.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"AppServ.h"
#include	"raudef.h"
#include	"oiban.h"
// MH322914(S) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
#include	"ifm_ctrl.h"
// MH322914(E) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
extern void lcdbm_notice_dsp3( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info, uchar *str, ulong str_size );
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
static short lto_tim( void );
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
static uchar date_uriage_cmp_paymentdate( void );
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
static void disc_wari_conv_main( DISCOUNT_DATA *p_disc, wari_tiket *p_wari, uchar opeFlag );
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���Z�����W�v																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kan_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT45EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	kan_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	LOKTOTAL_DAT	*lkts;											/*								*/
	ulong	dat1, dat2;												/*								*/
	ushort	iti;													/*								*/
	short	i;														/*								*/
	ulong	wari_ryo = 0;											/* ��������						*/
	ulong	ppc_ryo	 = 0;											/* ����߲�޶��ގg�p����			*/
	uchar	uc_update = PayData.teiki.update_mon;					/* �X�V���i�[					*/
	ulong	wk_Electron_ryo;										/* �d�q�Ȱ���Z���z�Z�[�u		*/
	char	f_BunruiSyu;											/* 1=���ޏW�v����				*/
																	/*								*/
	uchar	wk_ot_car;
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	uchar	wk_sss_flg = 0;
//	uchar	wk_sss_syu = 0;											// �C�����������
//	ushort	wdt1,wdt2,wdt3;
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	uchar	wk_rissu = 0;
	wari_tiket	wari_dt;
	short	j;
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	DATE_SYOUKEI	*date_ts;
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// MH322914(S) K.Onodera 2016/10/11 AI-V�Ή��F���u���Z
	char	tmp_syu = 0;
	ushort	val = 0;
// MH322914(E) K.Onodera 2016/10/11 AI-V�Ή��F���u���Z
// �s��C��(S) K.Onodera 2016/11/28 #1578 �W�v�����ۂ̋��z�ƈ�v���Ȃ�
	ulong	ulFurikae = 0, ulFuriWari = 0, pos = 0;
// �s��C��(E) K.Onodera 2016/11/28 #1578 �W�v�����ۂ̋��z�ƈ�v���Ȃ�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
	uchar	taxableAdd_set;											/*	�ېőΏ۔�����Z�ݒ�		*/
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	struct clk_rec wrk_clk_rec;		/* �W�v�����m�[�}���C�Y�p */
	date_time_rec wrk_date;			/* �W�v�����m�[�}���C�Y�p */
	int		ec_kind;
	int		syu_idx;				/* �����W�v�̃C���f�b�N�X */
	ulong	sei_date;				/* ���Z���� */
	ulong	sy1_date;				/* �W�v���� */
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
// // GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// 	char	qrcode[QRCODE_RECEIPT_SIZE + 4];
// // GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
	memset(&wrk_clk_rec, 0x0, sizeof(wrk_clk_rec));
// MH321800(E) hosoda IC�N���W�b�g�Ή�

	if( OPECTL.f_KanSyuu_Cmp )										/* �����W�v���{�ς�				*/
		return;														/*								*/
	else															/*								*/
		OPECTL.f_KanSyuu_Cmp = 1;									/* �����W�v���{�ς�				*/
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	date_ts = &Date_Syoukei;
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	iti = ryo_buf.pkiti - 1;										/* ���Ԉʒu						*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* �s�W�v��ܰ��ر��(��d�΍�)	*/
	if(( ryo_buf.pkiti != 0xffff )&&								/* ������Z�ȊO					*/
	   ( uc_update == 0 )){											/* ������X�V���Z�ȊO			*/
		lkts = &loktl.tloktl.loktldat[iti];							/*								*/
		memcpy( &wkloktotal, lkts, sizeof( LOKTOTAL_DAT ) );		/* �s�W�v��ܰ��ر��(��d�΍�)	*/
	}																/*								*/
	DailyAggregateDataBKorRES( 0 );									/* �����W�v�G���A�̃o�b�N�A�b�v	*/
																	/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	date_uriage_syoukei_judge();									// ���t�ؑ֊ ������A���������㏬�v�̎����X�V����i���A���^�C�����p�j
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	ac_flg.cycl_fg = 14;											/* 14:�s�W�v��ܰ��ر�֓]������	*/
																	/*								*/
// �d�l�ύX(S) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
//	dat1 = ryo_buf.nyukin - ryo_buf.turisen;						/* �����z�|�ނ�K�z				*/
	dat1 = ryo_buf.nyukin - ryo_buf.turisen - ryo_buf.kabarai;		/* �����z�|�ނ�K�z�|�U�։ߕ��� */
// �d�l�ύX(E) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
																	/*								*/
																	/*								*/
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//	wdt1 = OPECTL.MPr_LokNo - 1;
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
	ts->Uri_Tryo += dat1;											/* ������グ					*/
	ts->Genuri_Tryo += dat1;										/* ����������グ				*/
	ts->Tax_Tryo += ryo_buf.tax;									/* �����(���ŁE�O��)			*/
																	/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
		if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
			date_ts->Uri_Tryo += dat1;
			date_ts->Genuri_Tryo += dat1;
		}
	}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	if(	(ryo_buf.pkiti != 0xffff)&&(uc_update == 0) ){				/* ������Z�A����X�V���Z�ȊO	*/

		wk_ot_car = syusei[iti].ot_car;								/*								*/
		wk_rissu = syusei[iti].infofg;
		memset( &syusei[iti], 0, sizeof(struct SYUSEI));			/* �C����C�����Z�p�ް��ر		*/
		syusei[iti].ot_car = wk_ot_car;								/*								*/
		if( wk_rissu & SSS_RYOUSYU ){								// �C�����ŗ̎��ؔ��s����
			syusei[iti].infofg |= SSS_RYOUSYU;						// 
		}
																	/*								*/
		if( (!FLAPDT.flp_data[iti].passwd) &&						/* �Ïؔԍ����͂Ȃ��Ő��Z		*/
			(OPECTL.Pay_mod != 2) ){								/* �C�����Z�łȂ�				*/
																	/*								*/
			syusei[iti].sei = 1;									/* �ߋ����Z�L��(�C���p)			*/
																	/*								*/
			syusei[iti].tryo = ryo_buf.tyu_ryo;						/* ���ԗ���(�C���p)				*/
			syusei[iti].gen = dat1;									/* �����̎��z(�C���p)			*/
			syusei[iti].tax = ryo_buf.tax;							/* �C���p����Ŋz				*/
																	/*								*/
			syusei[iti].iyear = car_in_f.year;						/* ���� �N(�C���p)				*/
			syusei[iti].imont = car_in_f.mon;						/*      ��						*/
			syusei[iti].idate = car_in_f.day;						/*      ��						*/
			syusei[iti].ihour = car_in_f.hour;						/*      ��						*/
			syusei[iti].iminu = car_in_f.min;						/*      ��						*/
			syusei[iti].oyear = car_ot_f.year;						/* �o�� �N(�C���p)				*/
			syusei[iti].omont = car_ot_f.mon;						/*      ��						*/
			syusei[iti].odate = car_ot_f.day;						/*      ��						*/
			syusei[iti].ohour = car_ot_f.hour;						/*      ��						*/
			syusei[iti].ominu = car_ot_f.min;						/*      ��						*/
																	/*								*/
			syusei[iti].sy_wmai = card_use[USE_SVC];				/* �g�p����(�C���p)				*/
			syusei[iti].syubetu = (uchar)(ryo_buf.syubet+1);		/* �������						*/
			CountGet( PAYMENT_COUNT, &syusei[iti].oiban );			/* ���Z�ǔ�						*/
		}															/*								*/
		FLAPDT.flp_data[iti].bk_syu = 0;							/* ���(���~)�ر				*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ ){								/* �����������v�v�Z				*/
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );
		disc_wari_conv_all( &PayData.DiscountData[i], &wari_dt );
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
		switch( wari_dt.tik_syu ){									/*								*/
			case	SERVICE:										/* ���޽��						*/
			case	KAKEURI:										/* �|����						*/
// MH810100(S) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�����ςݑΉ�/�Đ��Z_�̎��؈�)
			case	C_SERVICE:										/* ���Z���~���޽��				*/
			case	C_KAKEURI:										/* ���Z���~�X����				*/
// MH810100(E) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�����ςݑΉ�/�Đ��Z_�̎��؈�)
			case	KAISUU:											/* �񐔌�						*/
			case	MISHUU:											/* ������						*/
			case	FURIKAE:										/* �U�֊z						*/
			case	SYUUSEI:										/* �C���z						*/
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
			case SHOPPING:											/* ��������						*/
			case C_SHOPPING:
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
				wari_ryo += wari_dt.ryokin;							/* �����������Z					*/
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//				// �C�����Z�̏ꍇ�ɂ͏C�����̏C���p�f�[�^���N���A����
//				if( wari_dt.tik_syu == SYUUSEI ){
//					wk_sss_syu = syusei[wdt1].syubetu;
//					if( syusei[wdt1].sei == 1 ){
//						syusei[wdt1].sei = 0;						// �C���� �C���ޯ̧�ر
//					}
//					wk_ot_car = syusei[iti].ot_car;
//					memset( &syusei[iti], 0, sizeof(struct SYUSEI));/* �C�����ް��ر				*/
//					syusei[iti].ot_car = wk_ot_car;
//					wk_sss_flg = 1;
//				}
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
				break;												/*								*/
			case	PREPAID:										/* �v���y�C�h�J�[�h				*/
				ppc_ryo += wari_dt.ryokin;							/* ����߲�޶��ގg�p�������Z		*/
				break;												/*								*/
		}															/*								*/
	}																/*								*/

	if( PayData.teiki.Apass_off_seisan ){							/* ��������߽OFF���Z����H		*/
		ts->Apass_off_seisan += 1;									/* ��������߽OFF���Z��+1		*/
		syusei[iti].infofg |= SSS_ANTIOFF;							// �C������������߽OFF���o�^
	}

	if( PayData.BeforeTwari )										/* �O�񎞊Ԋ������z����?		*/
		wari_ryo += PayData.BeforeTwari;							/* �O�񎞊Ԋ������z���Z			*/
	if( PayData.BeforeRwari )										/* �O�񗿋��������z����?		*/
		wari_ryo += PayData.BeforeRwari;							/* �O�񗿋��������z���Z			*/
// MH810100(S) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
	if( PayData.zenkai ){											/* �O��̎��z����?				*/
		wari_ryo += PayData.zenkai;									/* �O��̎��z���Z				*/
	}
// MH810100(E) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
																	/* ����߲�޶��ގg�p�����v�Z		*/
	if( CPrmSS[S_PRP][1] != 2 ){									/* �񐔌��g�p����ݒ肶��Ȃ�������	*/
		ppc_ryo += PayData.ppc_chusi_ryo;							/* �O������߲�޶��ގg�p�������Z	*/
	}																/*								*/
	dat2 = 0L;														/*								*/
																	/* ��ʖ��W�v�̐��Z�����v�Z		*/
	switch( prm_get(COM_PRM, S_TOT, 12, 1, 1) ){					/* ��ʖ��W�v���@�H				*/
																	/*								*/
		case	0:													/* ���ԗ���						*/
			dat2 = PayData.WPrice;									/*								*/
			break;													/*								*/
																	/*								*/
		case	1:													/* �����㗿���iPPC���Z�܂ށj	*/
			if( PayData.WPrice >= (wari_ryo + ryo_buf.credit.pay_ryo) )
																	/* ���ԗ����|��������			*/
				dat2 = (PayData.WPrice) - wari_ryo - ryo_buf.credit.pay_ryo;
			break;													/*								*/
																	/*								*/
		case	2:													/* �����㗿���iPPC�A�d�q�Ȱ���Z�܂܂Ȃ��j*/
// MH321800(S) hosoda IC�N���W�b�g�Ή�
//			if( PayData.WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo + PayData.Electron_data.Suica.pay_ryo) )
//				dat2 = (PayData.WPrice) - wari_ryo - ppc_ryo - ryo_buf.credit.pay_ryo - PayData.Electron_data.Suica.pay_ryo;
			if(PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) {		// ���σ��[�_�̃N���W�b�g���ρH
				if( PayData.WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo) )
					dat2 = (PayData.WPrice) - wari_ryo - ppc_ryo - ryo_buf.credit.pay_ryo;
			} else {
				if( PayData.WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo + PayData.Electron_data.Suica.pay_ryo) )
					dat2 = (PayData.WPrice) - wari_ryo - ppc_ryo - ryo_buf.credit.pay_ryo - PayData.Electron_data.Suica.pay_ryo;
			}
// MH321800(E) hosoda IC�N���W�b�g�Ή�
			break;													/*								*/
																	/*								*/
		default:													/* ���̑��i�ݒ�װ�j			*/
			dat2 = PayData.WPrice;									/*								*/
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
			break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}

	if( uc_update == 0 )											/* ������X�V���Z�ȊO			*/
	{																/*								*/
// MH322914(S) K.Onodera 2016/10/11 AI-V�Ή��F���u���Z
		// ���u���Z�H(ParkingWeb��)
		if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_TIME ||
			OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE ){
			val = prm_get( COM_PRM, S_CEN, 40, 2, 1 );
			if( val >= 1 && val <= 12 ){
				tmp_syu = ryo_buf.syubet;	// �ޔ�
				ryo_buf.syubet = (val - 1);
			}
			ts->Remote_seisan_cnt += 1L;
			ts->Remote_seisan_ryo += ryo_buf.tyu_ryo;
		}
// MH322914(E) K.Onodera 2016/10/11 AI-V�Ή��F���u���Z
		if(( CPrmSS[S_CAL][19] != 0 )&&								/* ����ŗL��?					*/
		   ( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2 ))				/* �O��?						*/
		{															/*								*/
			if( dat2 >= ryo_buf.tax )								/* �ŋ��Ώۊz�����ŋ�?			*/
				ts->Rsei_ryo[ryo_buf.syubet] += (dat2 - ryo_buf.tax);/* ��ʖ����Z����				*/
			else													/*								*/
				ts->Rsei_ryo[ryo_buf.syubet] += dat2;				/* ��ʖ����Z����				*/
																	/*								*/
			if( ryo_buf.pkiti != 0xffff ){							/* ������Z�ȊO					*/
				if( dat1 >= ryo_buf.tax )							/* �ŋ��Ώۊz�����ŋ�?			*/
					lkts->Genuri_ryo += (dat1 - ryo_buf.tax);		/* ���Ԉʒu�ԍ��ʏW�v(��������グ)	*/
				else												/*								*/
					lkts->Genuri_ryo += dat1;						/* ���Ԉʒu�ԍ��ʏW�v(��������グ)	*/
			}
		} else {													/*								*/
			ts->Rsei_ryo[ryo_buf.syubet] += dat2;					/* ��ʖ����Z����				*/
			if( ryo_buf.pkiti != 0xffff )							/* ������Z�ȊO					*/
				lkts->Genuri_ryo += dat1;							/* ���Ԉʒu�ԍ��ʏW�v(��������グ)	*/
		}															/*								*/
// MH322914(S) K.Onodera 2017/01/05 AI-V�Ή�
		// �U�֌�����U�֊z������ + �U�֐�ɉ��Z
		for( i = 0; i < DETAIL_SYU_MAX; i++ )
		{
			switch( PayData.DetailData[i].DiscSyu ){
				case NTNET_FURIKAE_2:													// �U�֐��Z�iParkingWeb�Łj
					pos = PayData.DetailData[i].uDetail.Furikae.Pos;
					break;
				case NTNET_FURIKAE_DETAIL:
					ulFurikae = PayData.DetailData[i].uDetail.FurikaeDetail.FrkMoney;	// �����������Z
					loktl.tloktl.loktldat[pos-1].Genuri_ryo -= ulFurikae;
					lkts->Genuri_ryo += ulFurikae;
					break;
			}
		}
// MH322914(E) K.Onodera 2017/01/05 AI-V�Ή�
																	/*								*/
		if( ryo_buf.pkiti != 0xffff )								/* ������Z�ȊO					*/
			lkts->Seisan_cnt += 1;									/* ���Ԉʒu�ԍ��ʏW�v(���Z�䐔)	*/
																	/*								*/
		ts->Rsei_cnt[ryo_buf.syubet] += 1L;							/* ��ʖ����Z��				*/
// MH322914(S) K.Onodera 2016/10/11 AI-V�Ή��F���u���Z
		if( tmp_syu ){
			// �ޔ������f�[�^�����ɖ߂�
			ryo_buf.syubet = tmp_syu;
		}
// MH322914(E) K.Onodera 2016/10/11 AI-V�Ή��F���u���Z
	}
// �s��C��(S) K.Onodera 2016/11/28 #1578 �W�v�����ۂ̋��z�ƈ�v���Ȃ�
	// �U�֊z�l��(�U�֊z��U�֌��������)
	val = (ushort)prm_get(COM_PRM, S_TOT, 12, 1, 1);
	if( val == 1 || val == 2 ){
		for( i = 0; i < DETAIL_SYU_MAX; i++ )
		{
			switch( PayData.DetailData[i].DiscSyu ){
				case NTNET_FURIKAE_2:											// �U�֐��Z�iParkingWeb�Łj
					tmp_syu = (PayData.DetailData[i].uDetail.Furikae.Oiban / 100000L);
					break;
				case NTNET_FURIKAE_DETAIL:
					// �����͐U�֌�������� ------------------
					ulFurikae = PayData.DetailData[i].uDetail.FurikaeDetail.FrkMoney;	// �����������Z
					ts->Rsei_ryo[tmp_syu-1] -= ulFurikae;		// �U�֌�����U�֊z������

					// ���̑��͐U�֐悩����� ------------------
					//  ���U�֌��ɂ͌��X���Z����Ȃ�
					// ����
					ulFuriWari = PayData.DetailData[i].uDetail.FurikaeDetail.FrkDiscount;
					// �N���W�b�g
					if( PayData.DetailData[i].uDetail.FurikaeDetail.Mod == 1 ){
						ulFuriWari += PayData.DetailData[i].uDetail.FurikaeDetail.FrkCard;
					}
					// �d�q�}�l�[
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//					else if( PayData.DetailData[i].uDetail.FurikaeDetail.Mod == 2 ){
					else if( PayData.DetailData[i].uDetail.FurikaeDetail.Mod != 0 ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
						if( val == 2 ){
							ulFuriWari += PayData.DetailData[i].uDetail.FurikaeDetail.FrkCard;
						}
					}
					if( PayData.WPrice < ulFuriWari ){
						ulFuriWari = PayData.WPrice;
					}
					ts->Rsei_ryo[ryo_buf.syubet] -= ulFuriWari;
					break;
			}
		}
	}
// �s��C��(E) K.Onodera 2016/11/28 #1578 �W�v�����ۂ̋��z�ƈ�v���Ȃ�

	ts->Seisan_Tcnt += 1L;											/* �����Z��					*/

// GG129000(S) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]
// 	/* ��ʊ������� */
// 	if( PayData.SyuWariRyo ){										/* ��ʊ�������					*/
// 		ts->Rtwari_ryo[ryo_buf.syubet] += PayData.SyuWariRyo;		/* ��ʊ������v�z���Z			*/
// 		ts->Rtwari_cnt[ryo_buf.syubet] += 1;						/* ��ʊ����񐔉��Z				*/
// 	}
// GG129000(E) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]
																	/*								*/
// �d�l�ύX(S) K.Onodera 2016/11/07 �U�֐��Z
//	ts->Turi_modosi_ryo += ryo_buf.turisen;							/* �ޑK���ߊz					*/
	ts->Turi_modosi_ryo += ryo_buf.turisen + ryo_buf.kabarai;		/* �ޑK���ߊz					*/
// �d�l�ύX(E) K.Onodera 2016/11/07 �U�֐��Z
																	/*								*/
	if( ryo_buf.fusoku != 0 )										/* �x�����s���z�L��H			*/
	{																/*								*/
		ts->Harai_husoku_cnt += 1L;									/* �x�����s����				*/
		ts->Harai_husoku_ryo += ryo_buf.fusoku;						/* �x�����s���z					*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.svs_tim )											/* ���޽��ѓ��o�ɗL��			*/
	{																/*								*/
		ts->In_svst_seisan += 1L;									/* ���޽��ѓ����Z��+1			*/
		syusei[iti].infofg |= SSS_SVTIME;							/* �C�������޽��ѓ��o�ɏ��o�^	*/
	}																/*								*/

																	/*								*/
// MH810105 GG119202(S) ������������W�v�d�l���P
//	if( PayData.credit.pay_ryo )									/* �ڼޯĶ��ގg�p				*/
	if( PayData.credit.pay_ryo &&									/* �ڼޯĶ��ގg�p				*/
		PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 0)	/* �݂Ȃ����ςł͂Ȃ�			*/
// MH810105 GG119202(E) ������������W�v�d�l���P
	{																/*								*/
		if( PayData.credit.cre_type == CREDIT_CARD ){
			ts->Ccrd_sei_cnt += 1;									/* �ڼޯĶ��ސ��Z���񐔁�		*/
			ts->Ccrd_sei_ryo += PayData.credit.pay_ryo;				/* �ڼޯĶ��ސ��Z�����z��		*/
																	/*								*/
			dat1 = prm_get( COM_PRM,S_TOT,6,1,1 );					/* �ڼޯĶ��ޑ�����z/���|���z���Z�ݒ�	*/
		}
		if( dat1 == 1L )											/* ������z�ɉ��Z����			*/
		{															/*								*/
			ts->Uri_Tryo += PayData.credit.pay_ryo;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
			if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
				if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
					date_ts->Uri_Tryo += PayData.credit.pay_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
		}															/*								*/
		else if( dat1 == 2L )										/* ���|���z�ɉ��Z����			*/
		{															/*								*/
			ts->Kakeuri_Tryo += PayData.credit.pay_ryo;				/* ���|���z						*/
		}															/*								*/
		else if( dat1 == 3L )										/* ������z/���|���z�ɉ��Z����	*/
		{															/*								*/
			ts->Uri_Tryo += PayData.credit.pay_ryo;					/* ������z						*/
			ts->Kakeuri_Tryo += PayData.credit.pay_ryo;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
			if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
				if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
					date_ts->Uri_Tryo += PayData.credit.pay_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
		}															/*								*/
	}																/*								*/
	wk_Electron_ryo = 0L;

	wk_media_Type = Ope_Disp_Media_Getsub(1);						/* �d�q�Ȱ�}�̎�� �擾			*/
// MH321800(S) hosoda IC�N���W�b�g�Ή�
	if (EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) != 0 ||
		PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED ) {
		// ���σ��[�_���g�p����Ă�����
		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );							/* �d�q�}�l�[������z/���|���z���Z�ݒ�	*/
		wk_Electron_ryo = PayData.Electron_data.Ec.pay_ryo;					/* �d�q�}�l�[���Z���z�Z�[�u				*/
		ec_kind = PayData.Electron_data.Ec.e_pay_kind;
// MH810105 GG119202(S) ������������W�v�d�l���P
		if (ec_kind == EC_CREDIT_USED) {
			wk_Electron_ryo = PayData.credit.pay_ryo;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			dat1 = 0;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		}
		if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
			// �݂Ȃ����ςł���΂݂Ȃ����ς̐��Z�񐔁A���z�ɉ��Z����
			ts->Ec_minashi_cnt += 1L;										// �݂Ȃ����ω�
			ts->Ec_minashi_ryo += wk_Electron_ryo;							// �݂Ȃ����ϋ��z
		}
		else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			// �����c���Ɖ���ł���Ώ�����������̎x���ς݉񐔁A���z�ɉ��Z����
			ts->miryo_pay_ok_cnt += 1L;										// �x���ς݉�
			ts->miryo_pay_ok_ryo += wk_Electron_ryo;						// �x���ς݋��z
		}
		else {
// MH810105 GG119202(E) ������������W�v�d�l���P
		// �W�v�͊����G���A������΂�����g�p����B
		switch(ec_kind) {
		case EC_EDY_USED:
			ts->Electron_edy_cnt += 1L;										// ���Z��������
			ts->Electron_edy_ryo += wk_Electron_ryo;						//     �����z��
			break;
		case EC_NANACO_USED:
			ts->nanaco_sei_cnt += 1L;										// ���Z��������
			ts->nanaco_sei_ryo += wk_Electron_ryo;							//     �����z��
			break;
		case EC_WAON_USED:
			ts->waon_sei_cnt += 1L;											// ���Z��������
			ts->waon_sei_ryo += wk_Electron_ryo;							//     �����z��
			break;
		case EC_SAPICA_USED:
			ts->sapica_sei_cnt += 1L;										// ���Z��������
			ts->sapica_sei_ryo += wk_Electron_ryo;							//     �����z��
			break;
		case EC_KOUTSUU_USED:
			ts->koutsuu_sei_cnt += 1L;										// ���Z��������
			ts->koutsuu_sei_ryo += wk_Electron_ryo;							//     �����z��
			break;
		case EC_ID_USED:
			ts->id_sei_cnt += 1L;											// ���Z��������
			ts->id_sei_ryo += wk_Electron_ryo;								//     �����z��
			break;
		case EC_QUIC_PAY_USED:
			ts->quicpay_sei_cnt += 1L;										// ���Z��������
			ts->quicpay_sei_ryo += wk_Electron_ryo;							//     �����z��
			break;
// MH810105(S) MH364301 PiTaPa�Ή�
		case EC_PITAPA_USED:
			ts->pitapa_sei_cnt += 1L;										// ���Z��������
			ts->pitapa_sei_ryo += wk_Electron_ryo;							//     �����z��
			break;
// MH810105(E) MH364301 PiTaPa�Ή�
		case EC_CREDIT_USED:
			// �N���W�b�g��T�W�v�͊��������ɂď���
// MH810105 GG119202(S) ������������W�v�d�l���P
//			wk_Electron_ryo = PayData.credit.pay_ryo;
// MH810105 GG119202(E) ������������W�v�d�l���P
			break;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case EC_QR_USED:
			ts->qr_sei_cnt += 1L;											// ���Z��������
			ts->qr_sei_ryo += wk_Electron_ryo;								//     �����z��
			break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		default:
			break;		// ���肦�Ȃ�
		}
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//		if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//			// �݂Ȃ����ςł���΂݂Ȃ����ς̐��Z�񐔁A���z�ɉ��Z����
//			ts->Ec_minashi_cnt += 1L;										// �݂Ȃ����ϐ��Z���񐔁�
//			ts->Ec_minashi_ryo += wk_Electron_ryo;							// �@�@�@�@ �����z��
//		}
//// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
		}
// MH810105 GG119202(E) ������������W�v�d�l���P
		// �ŐV�̒ʒm��������W�v�������Z�o
		memcpy(&wrk_date, &Syuukei_sp.ec_inf.NowTime, sizeof(date_time_rec));
		wrk_date.Hour = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 3);			/* ���͋��ʃp�����[�^����擾 */
		wrk_date.Min = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 1);				/* ���͋��ʃp�����[�^����擾 */
		sy1_date = Nrm_YMDHM(&wrk_date);									/* �W�v�������m�[�}���C�Y */

		// ���Z�f�[�^����i���ό��ʃf�[�^�Ŏ�M�����j���Z�������Z�o
		c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &wrk_clk_rec);
		memcpy(&wrk_date, &wrk_clk_rec, sizeof(date_time_rec));
		sei_date = Nrm_YMDHM(&wrk_date);									/* ���Z�������m�[�}���C�Y */

		ec_kind -= EC_EDY_USED;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		if (ec_kind >= EC_BRAND_TOTAL_MAX) {
			// �͈͊O�͓����W�v���s��Ȃ�
		}
		else
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		if(sei_date >= sy1_date && sei_date < (sy1_date + 0x10000)) {
			// ���Z�����������̏W�v�����͈�
// MH810105 GG119202(S) ������������W�v�d�l���P
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// �݂Ȃ����ςł���΂݂Ȃ����ς̐��Z�񐔁A���z�ɉ��Z����
				Syuukei_sp.ec_inf.now.sp_minashi_cnt += 1L;						// �݂Ȃ����ω�
				Syuukei_sp.ec_inf.now.sp_minashi_ryo += wk_Electron_ryo;		// �݂Ȃ����ϋ��z
			}
			else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
				// �����c���Ɖ���ł���Ώ�����������̎x���ς݉񐔁A���z�ɉ��Z����
				Syuukei_sp.ec_inf.now.sp_miryo_pay_ok_cnt += 1L;				// �x���ς݉�
				Syuukei_sp.ec_inf.now.sp_miryo_pay_ok_ryo += wk_Electron_ryo;	// �x���ς݋��z
			}
			else {
// MH810105 GG119202(E) ������������W�v�d�l���P
			Syuukei_sp.ec_inf.now.cnt[ec_kind] += 1L;							/* �g�p��				*/
			Syuukei_sp.ec_inf.now.ryo[ec_kind] += wk_Electron_ryo;				/* �g�p���z				*/
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//			// �݂Ȃ����ςł���Γ����W�v�i�[�f�[�^�̈�Ԍ��ɉ��Z����
//			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//				Syuukei_sp.ec_inf.now.cnt[EC_BRAND_TOTAL_MAX - 1] += 1L;		/* �g�p��				*/
//				Syuukei_sp.ec_inf.now.ryo[EC_BRAND_TOTAL_MAX - 1] += wk_Electron_ryo; /* �g�p���z				*/
//			}
//// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
			}
// MH810105 GG119202(E) ������������W�v�d�l���P
		}
		else if(sei_date >= (sy1_date + 0x10000) && sei_date < (sy1_date + 0x20000)) {
			// ���Z�����������̏W�v�����͈�
// MH810105 GG119202(S) ������������W�v�d�l���P
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// �݂Ȃ����ςł���΂݂Ȃ����ς̐��Z�񐔁A���z�ɉ��Z����
				Syuukei_sp.ec_inf.next.sp_minashi_cnt += 1L;					// �݂Ȃ����ω�
				Syuukei_sp.ec_inf.next.sp_minashi_ryo += wk_Electron_ryo;		// �݂Ȃ����ϋ��z
			}
			else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
				// �����c���Ɖ���ł���Ώ�����������̎x���ς݉񐔁A���z�ɉ��Z����
				Syuukei_sp.ec_inf.next.sp_miryo_pay_ok_cnt += 1L;				// �x���ς݉�
				Syuukei_sp.ec_inf.next.sp_miryo_pay_ok_ryo += wk_Electron_ryo;	// �x���ς݋��z
			}
			else {
// MH810105 GG119202(E) ������������W�v�d�l���P
			Syuukei_sp.ec_inf.next.cnt[ec_kind] += 1L;							/* �g�p��				*/
			Syuukei_sp.ec_inf.next.ryo[ec_kind] += wk_Electron_ryo;				/* �g�p���z				*/
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//			// �݂Ȃ����ςł���Γ����W�v�i�[�f�[�^�̈�Ԍ��ɉ��Z����
//			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//				Syuukei_sp.ec_inf.next.cnt[EC_BRAND_TOTAL_MAX - 1] += 1L;		/* �g�p��				*/
//				Syuukei_sp.ec_inf.next.ryo[EC_BRAND_TOTAL_MAX - 1] += wk_Electron_ryo; /* �g�p���z				*/
//			}
//// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
			}
// MH810105 GG119202(E) ������������W�v�d�l���P
		}
		else if(sei_date >= (sy1_date - 0x10000) && sei_date < sy1_date) {
			// ���Z�������O���̏W�v�����͈�
			if(Syuukei_sp.ec_inf.cnt == 1) {
				// �O���̏W�v���Ȃ����ߍ쐬����
				UnNrm_YMDHM(&wrk_date, (sy1_date - 0x10000));
				memcpy(&Syuukei_sp.ec_inf.bun[Syuukei_sp.ec_inf.ptr].SyuTime, &wrk_date, sizeof(date_time_rec));
				Syuukei_sp.ec_inf.ptr++;
				Syuukei_sp.ec_inf.cnt++;
			}
			syu_idx = (Syuukei_sp.ec_inf.ptr == 0) ? (SYUUKEI_DAY_EC-1) : (Syuukei_sp.ec_inf.ptr-1);
// MH810105 GG119202(S) ������������W�v�d�l���P
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// �݂Ȃ����ςł���΂݂Ȃ����ς̐��Z�񐔁A���z�ɉ��Z����
				Syuukei_sp.ec_inf.bun[syu_idx].sp_minashi_cnt += 1L;					// �݂Ȃ����ω�
				Syuukei_sp.ec_inf.bun[syu_idx].sp_minashi_ryo += wk_Electron_ryo;		// �݂Ȃ����ϋ��z
			}
			else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
				// �����c���Ɖ���ł���Ώ�����������̎x���ς݉񐔁A���z�ɉ��Z����
				Syuukei_sp.ec_inf.bun[syu_idx].sp_miryo_pay_ok_cnt += 1L;				// �x���ς݉�
				Syuukei_sp.ec_inf.bun[syu_idx].sp_miryo_pay_ok_ryo += wk_Electron_ryo;	// �x���ς݋��z
			}
			else {
// MH810105 GG119202(E) ������������W�v�d�l���P
			Syuukei_sp.ec_inf.bun[syu_idx].cnt[ec_kind] += 1L;					/* �g�p��				*/
			Syuukei_sp.ec_inf.bun[syu_idx].ryo[ec_kind] += wk_Electron_ryo;		/* �g�p���z				*/
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//			// �݂Ȃ����ςł���Γ����W�v�i�[�f�[�^�̈�Ԍ��ɉ��Z����
//			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//				Syuukei_sp.ec_inf.bun[syu_idx].cnt[EC_BRAND_TOTAL_MAX - 1] += 1L;	/* �g�p��				*/
//				Syuukei_sp.ec_inf.bun[syu_idx].ryo[EC_BRAND_TOTAL_MAX - 1] += wk_Electron_ryo; /* �g�p���z				*/
//			}
//// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
			}
// MH810105 GG119202(E) ������������W�v�d�l���P
		} else {
			// ���Z��������������O���͈̔͊O�̏ꍇ�̓A���[����o�^���ē����ɉ��Z
// MH810105 GG119202(S) ������������W�v�d�l���P
			if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
				// �݂Ȃ����ςł���΂݂Ȃ����ς̐��Z�񐔁A���z�ɉ��Z����
				Syuukei_sp.ec_inf.now.sp_minashi_cnt += 1L;						// �݂Ȃ����ω�
				Syuukei_sp.ec_inf.now.sp_minashi_ryo += wk_Electron_ryo;		// �݂Ȃ����ϋ��z
			}
			else if (PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
				// �����c���Ɖ���ł���Ώ�����������̎x���ς݉񐔁A���z�ɉ��Z����
				Syuukei_sp.ec_inf.now.sp_miryo_pay_ok_cnt += 1L;				// �x���ς݉�
				Syuukei_sp.ec_inf.now.sp_miryo_pay_ok_ryo += wk_Electron_ryo;	// �x���ς݋��z
			}
			else {
// MH810105 GG119202(E) ������������W�v�d�l���P
			Syuukei_sp.ec_inf.now.cnt[ec_kind] += 1L;							/* �g�p��				*/
			Syuukei_sp.ec_inf.now.ryo[ec_kind] += wk_Electron_ryo;				/* �g�p���z				*/
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//			// �݂Ȃ����ςł���Γ����W�v�i�[�f�[�^�̈�Ԍ��ɉ��Z����
//			if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
//				Syuukei_sp.ec_inf.now.cnt[EC_BRAND_TOTAL_MAX - 1] += 1L;		/* �g�p��				*/
//				Syuukei_sp.ec_inf.now.ryo[EC_BRAND_TOTAL_MAX - 1] += wk_Electron_ryo; /* �g�p���z				*/
//			}
//// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
			}
// MH810105 GG119202(E) ������������W�v�d�l���P
			Syuukei_sp.ec_inf.now.unknownTimeCnt++;

			// �A���[���o�^(���σ��[�_�̌��ώ����Ɛ��Z�@�̎�����1���ȏ�̂��ꂪ����)
			alm_chk( ALMMDL_SUB, ALARM_EC_SETTLEMENT_TIME_GAP, 2 );
		}
	}
	else
// MH321800(E) hosoda IC�N���W�b�g�Ή�
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ){

		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );					/* Suica������z/���|���z���Z�ݒ�	*/
		wk_Electron_ryo = PayData.Electron_data.Suica.pay_ryo;		/* �d�q�Ȱ���Z���z�Z�[�u		*/
		Syuukei_sp.sca_inf.now.cnt += 1L;							/* Suica�g�p��				*/
		Syuukei_sp.sca_inf.now.ryo += wk_Electron_ryo;				/* Suica�g�p���z				*/
		if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {			/* �r��������					*/
			ts->Electron_sei_cnt += 1L;								/* Suica�g�p���񐔁�			*/
			ts->Electron_sei_ryo += wk_Electron_ryo;				/* Suica�g�p�����z��			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		/* �o�`�r�l�n					*/
			ts->Electron_psm_cnt += 1L;								/* PASMO�g�p���񐔁�			*/
			ts->Electron_psm_ryo += wk_Electron_ryo;				/* PASMO�g�p�����z��			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICOCA) {		/* �h�b�n�b�`					*/
			ts->Electron_ico_cnt += 1L;								/* ICOCA�g�p���񐔁�			*/
			ts->Electron_ico_ryo += wk_Electron_ryo;				/* ICOCA�g�p�����z��			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICCARD) {	/* �h�b-�b�`�q�c				*/
			ts->Electron_icd_cnt += 1L;								/* IC-CARD�g�p���񐔁�			*/
			ts->Electron_icd_ryo += wk_Electron_ryo;				/* IC-CARD�g�p�����z��			*/
		}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//	} else if( PayData.Electron_data.Suica.e_pay_kind == EDY_USED ){/* Edy��ް�̎��ް�����			*/
//		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 3 );					/* Edy������z/���|���z���Z�ݒ�	*/
//		if( EDY_TEST_MODE ){										// ý�Ӱ�ނ̏ꍇ
//			dat1 = 0L;												// ������z/���|���z�ɉ��Z���Ȃ�
//		}
//		else{
//			wk_Electron_ryo = PayData.Electron_data.Edy.pay_ryo;	/* �d�q�Ȱ���Z���z�Z�[�u		*/
//			if(wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY) {			/* �d����						*/
//				ts->Electron_edy_cnt += 1L;							/* �d�����g�p���񐔁�			*/
//				ts->Electron_edy_ryo += wk_Electron_ryo;			/* �d�����g�p�����z��			*/
//				Syuukei_sp.edy_inf.now.cnt += 1L;					/* PASMO�g�p��				*/
//				Syuukei_sp.edy_inf.now.ryo += wk_Electron_ryo;		/* PASMO�g�p���z				*/
//			}
//		}
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//	if( PayData.Electron_data.Suica.e_pay_kind != 0 ) {				/* �d�q�Ȱ(Suica/Edy)�̎��ް�����*/
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ||	// SX-20�ɂ��d�q�}�l�[����
// MH810105 GG119202(S) ������������W�v�d�l���P
//		EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {			// ���σ��[�_�ɂ��d�q�}�l�[����
		(EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) &&			// ���σ��[�_�ɂ��d�q�}�l�[����
		 PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 0)) {	// �݂Ȃ����ςł͂Ȃ�
// MH810105 GG119202(E) ������������W�v�d�l���P
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
		if( dat1 == 1L ) {											/* ������z�ɉ��Z����			*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
			if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
				if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
					date_ts->Uri_Tryo += wk_Electron_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
		} else if( dat1 == 2L ) {									/* ���|���z�ɉ��Z����			*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* ���|���z						*/
		} else if( dat1 == 3L ) {									/* ������z/���|���z�ɉ��Z����	*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* ������z						*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
			if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
				if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
					date_ts->Uri_Tryo += wk_Electron_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
		}															/*								*/
	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i�݂Ȃ����ϊz�𑍔���^���|���։��Z����ݒ�j
	else{
		if( PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 ){
			dat1 = prm_get( COM_PRM, S_TOT, 7, 1, 1 );				// �݂Ȃ����ϑ�����z/���|���z���Z�ݒ�
			if( dat1 == 1L )										/* ���Z����ꍇ�e�u�����h�̉��Z�ݒ���Q�� */
			{														/*								*/
				if( PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED ){
					dat2 = prm_get( COM_PRM,S_TOT,6,1,1 );			/* �ڼޯĶ��ޑ�����z/���|���z���Z�ݒ�	*/
					if( dat2 == 1L )								/* ������z�ɉ��Z����			*/
					{												/*								*/
						ts->Uri_Tryo += PayData.credit.pay_ryo;		/* ������z						*/
						if(date_uriage_use_chk() == 0){				// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
							if(date_uriage_cmp_paymentdate() == 0){	// ���Z�\
								date_ts->Uri_Tryo += PayData.credit.pay_ryo;
							}
						}
					}												/*								*/
					else if( dat2 == 2L )							/* ���|���z�ɉ��Z����			*/
					{												/*								*/
						ts->Kakeuri_Tryo += PayData.credit.pay_ryo;	/* ���|���z						*/
					}												/*								*/
					else if( dat2 == 3L )							/* ������z/���|���z�ɉ��Z����	*/
					{												/*								*/
						ts->Uri_Tryo += PayData.credit.pay_ryo;		/* ������z						*/
						ts->Kakeuri_Tryo += PayData.credit.pay_ryo;	/* ���|���z						*/
						if(date_uriage_use_chk() == 0){				// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
							if(date_uriage_cmp_paymentdate() == 0){	// ���Z�\
								date_ts->Uri_Tryo += PayData.credit.pay_ryo;
							}
						}
					}												/*								*/
				}
				else if( EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ){
					dat2 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );		/* �d�q�}�l�[������z/���|���z���Z�ݒ�	*/
					if( dat2 == 1L )								/* ������z�ɉ��Z����			*/
					{												/*								*/
						ts->Uri_Tryo += wk_Electron_ryo;			/* ������z						*/
						if(date_uriage_use_chk() == 0){				// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
							if(date_uriage_cmp_paymentdate() == 0){	// ���Z�\
								date_ts->Uri_Tryo += wk_Electron_ryo;
							}
						}
					}
					else if( dat2 == 2L )							/* ���|���z�ɉ��Z����			*/
					{												/*								*/
						ts->Kakeuri_Tryo += wk_Electron_ryo;		/* ���|���z						*/
					}
					else if( dat2 == 3L )							/* ������z/���|���z�ɉ��Z����	*/
					{												/*								*/
						ts->Uri_Tryo += wk_Electron_ryo;			/* ������z						*/
						ts->Kakeuri_Tryo += wk_Electron_ryo;		/* ���|���z						*/
						if(date_uriage_use_chk() == 0){				// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
							if(date_uriage_cmp_paymentdate() == 0){	// ���Z�\
								date_ts->Uri_Tryo += wk_Electron_ryo;
							}
						}
					}
				}
			}
		}															/*								*/
	}																/*								*/														/*								*/
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i�݂Ȃ����ϊz�𑍔���^���|���։��Z����ݒ�j
																	/*								*/
	if( CPrmSS[S_PRP][1] == 1 )										/* ����߲�޶��ނ��g�p����ݒ�	*/
	{																/*								*/
		dat1 = prm_get( COM_PRM,S_TOT,5,1,3 );						/* ����߲�޶��ޑ�����z/���|���z���Z�ݒ�	*/
		j = 0;														/*								*/
		for( i = 0; i < WTIK_USEMAX; i++ ){							/*								*/
			disc_wari_conv( &PayData.DiscountData[i], &wari_dt );	/*								*/
			if( wari_dt.tik_syu == PREPAID ){						/* �ް�����						*/
				dat2 = wari_dt.pkno_syu;							/*								*/
				ts->Pcrd_use_cnt[dat2] += ryo_buf.pri_mai[j];		/* ����߲�޶��ގg�p���񐔁�(��{,�g��1,2,3)	*/
																	/* 7���ȏ�g�p���͍Ō�Ɏg��ꂽ��ʂɍ��v���Z����	*/
				ts->Pcrd_use_ryo[dat2] += wari_dt.ryokin;			/* ����߲�޶��ގg�p�����z��(��{,�g��1,2,3)	*/
																	/*								*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				j++;
			}														/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ )								/*								*/
	{																/*								*/
		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );		/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
		taxableAdd_set = 0xff;
		if( cancelReceipt_Waridata_chk( &wari_dt ) ){				/*			�ېőΏ�			*/
			taxableAdd_set = (uchar)prm_get( COM_PRM, S_TOT, 7, 1, 2 );	/*�ېőΏۗ��p�z�̔�����Z�ݒ�*/
		}															/*								*/
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
		switch( wari_dt.tik_syu )									/*								*/
		{															/*								*/
			case SERVICE:											/* ���޽��						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* ���޽��A,B,C					*/
																	/*								*/
				ts->Stik_use_cnt[dat1][dat2] += wari_dt.maisuu;		/* ���޽����ʖ��g�p��������(��{,�g��1,2,3)(���޽��A,B,C)	*/
				ts->Stik_use_ryo[dat1][dat2] += wari_dt.ryokin;		/* ���޽����ʖ��g�p�����z��(��{,�g��1,2,3)(���޽��A,B,C)	*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//				if( CPrmSS[S_SER][1+3*dat2] == 4 &&					/* ���޽���S�z�����ݒ�?			*/
				if( prm_get( COM_PRM, S_SER, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* ���޽���S�z�����ݒ�? */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
					ryo_buf.pkiti != 0xffff &&						/* ������Z�ȊO?					*/
					uc_update == 0 )								/* ������X�V���Z�ȊO			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* �C���p�S�z�����׸޾��		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,1 );				/* ���޽��������z/���|���z���Z�ݒ�	*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				setting_taxableAdd( &dat1, taxableAdd_set );		/* �ېőΏۗ��p�z�̔�����Z�ݒ� */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				break;												/*								*/
																	/*								*/
			case KAKEURI:											/* �|����						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* �X��1�`999(0�`998)			*/
																	/*								*/

				ts->Mno_use_Tcnt += wari_dt.maisuu;					/* �X������İ�ف��񐔁�			*/
				ts->Mno_use_Tryo += wari_dt.ryokin;					/* �X������İ�ف����z��			*/

// MH322914(S) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
//				ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;			/* �X������İ�ف��񐔁�(��{,�g��1,2,3)	*/
//				ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;			/* �X������İ�ف����z��(��{,�g��1,2,3)	*/
				// ParkingWeb�o�R��ParkiPro�Ή�����H ���AParkingWeb����̓X�������ݒ�ƈ�v�H
				if( _is_ParkingWeb_pip() && ((prm_get( COM_PRM, S_CEN, 45, 4, 1 )) == (dat2+1)) ){
					ts->Mno_use_cnt5 += wari_dt.maisuu;
					ts->Mno_use_ryo5 += wari_dt.ryokin;
				}
				else{
					ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;		/* �X������İ�ف��񐔁�(��{,�g��1,2,3)	*/
					ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;		/* �X������İ�ف����z��(��{,�g��1,2,3)	*/
// MH322914(E) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����

				if( dat1 == KIHON_PKNO || dat1 == KAKUCHOU_1 )		/* ��{ or �g��1				*/
				{													/*								*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
//					if( dat2 < 100 )								/* �X��1�`100					*/
					if( dat2 < MISE_NO_CNT )						/* �X��1�`100					*/
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* �X��1�`100İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* �X��1�`100İ�ي��������z��(��{,�g��1,2,3)	*/
						ts->Mno_use_cnt4[dat1][dat2] +=				/* �X�����������񐔁�(��{,�g��1)(�X��1�`100)	*/
												wari_dt.maisuu;		/*								*/
						ts->Mno_use_ryo4[dat1][dat2] +=				/* �X�������������z��(��{,�g��1)(�X��1�`100)	*/
												wari_dt.ryokin;		/*								*/
					}												/*								*/
					else											/* �X��101�`999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* �X��101�`999İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* �X��101�`999İ�ي��������z��(��{,�g��1,2,3)	*/
					}												/*								*/
				}													/*								*/
				else												/*								*/
				{													/*								*/
					if(( dat1 == KAKUCHOU_2 )&&						/* �g��2						*/
					   ( prm_get( COM_PRM,S_TOT,16,1,3 ) == 1 ))	/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
					else if(( dat1 == KAKUCHOU_3 )&&				/* �g��3						*/
							( prm_get( COM_PRM,S_TOT,16,1,4 ) == 1 ))/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
//					if( dat2 < 100 )								/* �X��1�`100					*/
					if( dat2 < MISE_NO_CNT )						/* �X��1�`100					*/
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* �X��1�`100İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* �X��1�`100İ�ي��������z��(��{,�g��1,2,3)	*/
						if( dat1 == KAKUCHOU_1 )					/*								*/
						{											/*								*/
							ts->Mno_use_cnt4[dat1][dat2] +=			/* �X�����������񐔁�(��{,�g��1)(�X��1�`100)	*/
												wari_dt.maisuu;		/*								*/
							ts->Mno_use_ryo4[dat1][dat2] +=			/* �X�������������z��(��{,�g��1)(�X��1�`100)	*/
												wari_dt.ryokin;		/*								*/
						}											/*								*/
					}												/*								*/
					else											/* �X��101�`999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* �X��101�`999İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* �X��101�`999İ�ي��������z��(��{,�g��1,2,3)	*/
					}												/*								*/
				}													/*								*/
// MH322914(S) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
				}
// MH322914(E) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//				if( CPrmSS[S_STO][1+3*dat2] == 4 &&					/* �X�����S�z�����ݒ�?			*/
				if( prm_get( COM_PRM, S_STO, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* �X�����S�z�����ݒ�? */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
					ryo_buf.pkiti != 0xffff &&						/* ������Z�ȊO?					*/
					uc_update == 0 )								/* ������X�V���Z�ȊO			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* �C���p�S�z�����׸޾��		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* �X����������z/���|���z���Z�ݒ�	*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				setting_taxableAdd( &dat1, taxableAdd_set );		/* �ېőΏۗ��p�z�̔�����Z�ݒ� */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				break;												/*								*/
			case KAISUU:											/* �񐔌�						*/
				if( CPrmSS[S_PRP][1] != 2 )							/* �񐔌����g�p����ݒ�ł͂Ȃ�	*/
					break;

				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
																	/*								*/
				ts->Ktik_use_cnt[dat1] += wari_dt.maisuu;			/* �g�p��������(��{,�g��1,2,3) */
				ts->Ktik_use_ryo[dat1] += wari_dt.ryokin;			/* �g�p�����z��(��{,�g��1,2,3)	*/

				dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );				/* �񐔌�������z/���|���z���Z�ݒ�	*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				break;												/*								*/
			case MISHUU:											/* ���z���Z						*/
				ts->Gengaku_seisan_cnt += 1L;						// T�W�v ���z��+1
				ts->Gengaku_seisan_ryo += wari_dt.ryokin;			// T�W�v ���z����
				break;
			case FURIKAE:											/* �U�֐��Z						*/
				ts->Furikae_seisan_cnt += 1L;						// T�W�v �U�։�+1
				ts->Furikae_seisan_ryo += wari_dt.ryokin;			// T�W�v �U�֗���
				break;
			case SYUUSEI:											/* �C�����Z						*/
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//				ts->Syuusei_seisan_Tcnt += 1L;						// T�W�v �C�����Z��+1
//				lkts->Syuusei_seisan_cnt += 1L;						// ���Ԉʒu�ԍ��ʏW�v(�C�����Z��)
//				if( prm_get(COM_PRM, S_TOT, 20, 1, 1) == 1 ){		// �U�֋��z�ŏW�v����
//					ts->Syuusei_seisan_Tryo += wari_dt.ryokin;		// T�W�v �C�����Z(�U�֊z)
//					lkts->Syuusei_seisan_ryo += wari_dt.ryokin;		// ���Ԉʒu�ԍ��ʏW�v(�C�����Z�z)
//				}else{												// ���ߊz�ŏC������(TF4800�݊�)
//					if( !(PayData.Zengakufg&0x02) ){				// �������Ȃ�
//						ts->Syuusei_seisan_Tryo += PayData.MMTwari;	// T�W�v �C�����Z(���ߊz)
//						lkts->Syuusei_seisan_ryo += PayData.MMTwari;// ���Ԉʒu�ԍ��ʏW�v(�C�����Z�z)
//					}
//				}
//				if( (PayData.Zengakufg&0x02) ){						// ����������
//					ts->Syuusei_seisan_Mcnt += 1L;					// T�W�v �C�����Z(��������)
//					ts->Syuusei_seisan_Mryo += PayData.MMTwari;		// T�W�v �C�����Z(���������z)
//				}
//
//#if 1 // AKIBA DEBUG TEST 08.06.27(S)
//				if( prm_get(COM_PRM, S_TOT, 20, 1, 6) == 1 ){				// �W�v���Z�̂�蒼������
//
//					// �����Z�񐔂̌��Z
//					if( ts->Seisan_Tcnt > 0 ){
//						ts->Seisan_Tcnt -= 1L;							// �C���� �����Z�񐔌��Z
//					}
//					// ���޽��ѓ����Z�񐔂̌��Z
//					if( syusei[wdt1].infofg & SSS_SVTIME ){				// �C���������޽��ѓ����Z�̎�
//						if( ts->In_svst_seisan > 0 ){
//							ts->In_svst_seisan -= 1L;					// �C���� ���޽��ѓ����Z��-1
//						}
//					}
//					// ����p�XOFF���Z�񐔂̌��Z
//					if( syusei[wdt1].infofg & SSS_ANTIOFF ){			// �C����������߽OFF���Z�̎�
//						if( ts->Apass_off_seisan > 0 ){
//							ts->Apass_off_seisan -= 1L;					// ��������߽OFF���Z��-1
//						}
//					}
//					// �̎��ؔ��s�񐔂̌��Z
//					if( syusei[wdt1].infofg & SSS_RYOUSYU ){			// �C���� �̎��ؔ��s����
//						if( ts->Ryosyuu_pri_cnt > 0 ){
//							ts->Ryosyuu_pri_cnt -= 1L;					// �̎��ؔ��s��-1
//						}
//					}
//					// �����ʖ����Z�񐔂̌��Z
//					if( syusei[wdt1].tei_syu ){							// �C�����Œ�����g�p�̎�
//						wdt2 = (syusei[wdt1].tei_syu & 0xc0) >> 6;		// ���ԏꇂ
//						wdt3 = syusei[wdt1].tei_syu & 0x3f;				// ������
//						if( ts->Teiki_use_cnt[wdt2][wdt3-1] > 0 ){
//							ts->Teiki_use_cnt[wdt2][wdt3-1] -= 1L;		// �C���� ����g�p��-1
//						}
//					}
//
//					// ������z�̌��Z(������z-�C�����̕��ߊz)
//					if( !(PayData.Zengakufg&0x02) ){							// �������Ȃ�
//						if( ts->Uri_Tryo >= PayData.MMTwari ){
//							ts->Uri_Tryo	-= PayData.MMTwari;					/* ������z						*/
//// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
//							if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
//								if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
//									date_ts->Uri_Tryo -= PayData.MMTwari;
//								}
//							}
//// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
//						}
//						// ����������z�̌��Z(����������z-�C�����̕��ߊz)
//						if( ts->Genuri_Tryo >= PayData.MMTwari ){
//							ts->Genuri_Tryo	-= PayData.MMTwari;					/* ����������グ				*/
//// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
//							if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
//								if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
//									date_ts->Genuri_Tryo -= PayData.MMTwari;
//								}
//							}
//// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
//						}
//					}
//					// ���Ԉʒu�ʐ��Z�񐔂̌��Z
//					if( loktl.tloktl.loktldat[wdt1].Seisan_cnt > 0 ){
//						loktl.tloktl.loktldat[wdt1].Seisan_cnt -= 1;	// �C���� ���Ԉʒu�ʐ��Z��-1
//					}
//					// ���Ԉʒu�ʌ����̎��z�̂�蒼��
//					// �@�C�����̎Ԏ�����C�����ł̌�����������
//					// �A�C�����̌��������C����։��Z����
//					if( loktl.tloktl.loktldat[wdt1].Genuri_ryo >= syusei[wdt1].gen ){
//						loktl.tloktl.loktldat[wdt1].Genuri_ryo -= syusei[wdt1].gen;
//					}														// �C���� ���Ԉʒu�ʌ������㌸�Z
//					if( !(PayData.Zengakufg&0x02) ){							// �������Ȃ�
//							lkts->Genuri_ryo += (syusei[wdt1].gen - PayData.MMTwari);
//					}else{
//							lkts->Genuri_ryo += (syusei[wdt1].gen);
//					}
//
//					// ������ʖ����Z�񐔂̌��Z
//					if( ts->Rsei_cnt[wk_sss_syu-1] > 0 ){
//						ts->Rsei_cnt[wk_sss_syu-1] -= 1L;				// �C���� ��ʖ����Z�񐔌��Z
//					}
//					// ������ʖ������̌��Z
//					if( !(PayData.Zengakufg&0x02) ){							// �������Ȃ�
//						if( prm_get(COM_PRM, S_TOT, 12, 1, 1) != 0 ){			/* ��ʖ��W�v���@�H�F�����㗿��	*/
//							ts->Rsei_ryo[ryo_buf.syubet] += 
//									(syusei[wdt1].gen - PayData.MMTwari);		// �C����ɏC�����̌������㕪�����Z
//							if( ts->Rsei_ryo[wk_sss_syu-1] >= syusei[wdt1].gen ){
//								ts->Rsei_ryo[wk_sss_syu-1] -= syusei[wdt1].gen;	/* �C������ʖ����Z�����������	*/
//							}
//						}else{													/* ��ʖ��W�v���@�H�F���ԗ���	*/
//							if( ts->Rsei_ryo[wk_sss_syu-1] >= syusei[wdt1].ryo ){
//								ts->Rsei_ryo[wk_sss_syu-1] -= syusei[wdt1].ryo;
//							}
//						}
//					}else{
//						if( prm_get(COM_PRM, S_TOT, 12, 1, 1) != 0 ){			/* ��ʖ��W�v���@�H�F�����㗿��	*/
//							ts->Rsei_ryo[ryo_buf.syubet] += syusei[wdt1].gen;	// �C����ɏC�����̌������㕪�����Z
//							if( ts->Rsei_ryo[ryo_buf.syubet] >= PayData.MMTwari ){
//								ts->Rsei_ryo[ryo_buf.syubet] -= PayData.MMTwari;
//							}
//							if( ts->Rsei_ryo[wk_sss_syu-1] >= syusei[wdt1].gen ){
//								ts->Rsei_ryo[wk_sss_syu-1] -= syusei[wdt1].gen;	/* �C������ʖ����Z�����������	*/
//							}
//						}else{													/* ��ʖ��W�v���@�H�F���ԗ���	*/
//							if( ts->Rsei_ryo[wk_sss_syu-1] >= syusei[wdt1].ryo ){
//								ts->Rsei_ryo[wk_sss_syu-1] -= syusei[wdt1].ryo;
//							}
//						}
//					}
//					// ���ޏW�v�̌��Z
//					if( prm_get(COM_PRM, S_TOT, 12, 1, 2) != 0 ){			/* ��ʖ��W�v���@�H�F�����㗿��	*/
//						bunrui_syusei( (ushort)(wdt1+1), syusei[wdt1].gen );
//					}else{
//						bunrui_syusei( (ushort)(wdt1+1), syusei[wdt1].ryo );
//					}
//				}
//#endif	// AKIBA DEBUG TEST 08.06.27(S)
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
				break;
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
			case SHOPPING:											/* ��������						*/
// GG124100(S) R.Endo 2022/10/03 �Ԕԃ`�P�b�g���X3.0 #6635 �����]�͂̂���g�p�ς݂̔������������Đ��Z�œK�p���ꂽ�Ƃ��A�W�v��1���g�p�����Ƃ��Ĉ󎚂���� [���ʉ��P���� No1534]
// 				ts->Kaimono_use_cnt[wari_dt.minute] += 1;			/* ���������@�`�I	���񐔁�	*/
				ts->Kaimono_use_cnt[wari_dt.minute] += wari_dt.maisuu;	/* ���������@�`�I	���񐔁�	*/
// GG124100(E) R.Endo 2022/10/03 �Ԕԃ`�P�b�g���X3.0 #6635 �����]�͂̂���g�p�ς݂̔������������Đ��Z�œK�p���ꂽ�Ƃ��A�W�v��1���g�p�����Ƃ��Ĉ󎚂���� [���ʉ��P���� No1534]
				ts->Kaimono_use_ryo[wari_dt.minute] += wari_dt.ryokin;	/* 				 	�����z��	*/
				
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* �X����������z/���|���z���Z�ݒ�	*/
//				dat1 = prm_get(COM_PRM, S_ONL, ONLINE_MEMBER_TOTAL_COUNT, 1, 3);
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				break;
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
// GG129000(S) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]
			case SYUBETU:											// ��ʊ���
				dat1 = (ulong)(wari_dt.syubetu - 1);
				ts->Rtwari_ryo[dat1] += wari_dt.ryokin;		// ��ʊ������v�z���Z
				ts->Rtwari_cnt[dat1] += wari_dt.maisuu;		// ��ʊ����񐔉��Z
				break;
// GG129000(E) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]
		}															/*								*/
	}																/*								*/
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
	for( i = 0; i < DETAIL_SYU_MAX; i++ )
	{
		disc_Detail_conv( &PayData.DetailData[i], &wari_dt );
		switch( wari_dt.tik_syu )
		{
			case FURIKAE2:											// �U�֐��Z�iParkingWeb�Łj
				ts->Furikae_seisan_cnt2 += 1L;						// T�W�v �U�։�+1
				ts->Furikae_seisan_ryo2 += wari_dt.ryokin;			// T�W�v �U�֗���
				// �����ȊO�̉ߕ�������H
				if( PayData.FRK_RetMod && PayData.FRK_Return ){
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//					ts->Furikae_Card_cnt[PayData.FRK_RetMod]++;
//					ts->Furikae_CardKabarai[PayData.FRK_RetMod] += PayData.FRK_Return;
					if (PayData.FRK_RetMod == 1) {
						ts->Furikae_Card_cnt[1]++;
						ts->Furikae_CardKabarai[1] += PayData.FRK_Return;
					} else {
						ts->Furikae_Card_cnt[2]++;
						ts->Furikae_CardKabarai[2] += PayData.FRK_Return;
					}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
				}
				break;
		}
	}
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�

	if(( ryo_buf.pkiti != 0xffff )&&											/* ������Z�ȊO					*/
	   ( uc_update == 0 )){														/* ������X�V���Z�ȊO			*/

	if( syusei[iti].infofg & SSS_ZENWARI ){										// �S�z��������
		syusei[iti].sy_wari = ryo_buf.waribik + c_pay;
	}else{																		// �S�z�����Ȃ�
		if( ryo_buf.tim_amount != 0 ){											// ���Ԋ�������
			syusei[iti].sy_wari = ryo_buf.fee_amount 
								+ PayData.credit.pay_ryo + wk_Electron_ryo;		// �ڼޯ�+E�Ȱ
			dat2 = 0L;
			if( ryo_buf.fee_amount >= c_pay ){
				dat2 = ryo_buf.fee_amount - c_pay;
			}
			if( ryo_buf.waribik >= dat2 ){
				syusei[iti].sy_time = ryo_buf.waribik - dat2;					//
			}
		}else{																	// ���Ԋ����Ȃ�
			syusei[iti].sy_wari = ryo_buf.waribik + c_pay						// time_amount�Ȃ��ł����waribik�͑S�ė�������
								+ PayData.credit.pay_ryo + wk_Electron_ryo;		// �ڼޯ�+E�Ȱ
		}
	}
	}
																	/*								*/
	if( uc_update ){												/* ������X�V���Z				*/
		dat1 = (ulong)PayData.teiki.pkno_syu;						/* ���ԏꇂ�̎��				*/
		dat2 = (ulong)(PayData.teiki.syu - 1);						/* ��������-1					*/
		ts->Teiki_kou_cnt[dat1][dat2] += 1L;						/* ������X�V��+1				*/
		ts->Teiki_kou_ryo[dat1][dat2] +=							/*								*/
// �d�l�ύX(S) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
//					(ulong)(ryo_buf.nyukin - ryo_buf.turisen);		/* ������X�V���z���Z			*/
					(ulong)(ryo_buf.nyukin - ryo_buf.turisen - ryo_buf.kabarai);		// ������X�V���z���Z
// �d�l�ύX(E) K.Onodera 2016/11/28 #1589 ��������f�[�^�́A�����z����ޑK�Ɖߕ��������������l�Ƃ���
	}																/*								*/

	f_BunruiSyu = 0;												/* ���ޏW�v����t���O�N���A		*/

	if( ryo_buf.ryo_flg >= 2 )										/* ����g�p?					*/
	{																/*								*/
		dat1 = (ulong)PayData.teiki.pkno_syu;						/* ���ԏꇂ�̎��				*/
		ts->Teiki_use_cnt[dat1][PayData.teiki.syu-1] += 1L;			/* ����g�p��+1				*/
		if(( ryo_buf.pkiti != 0xffff )&&							/* ������Z�ȊO					*/
		   ( uc_update == 0 )){										/* ������X�V���Z�ȊO			*/

			syusei[iti].ryo = ryo_buf.tei_ryo;						/* �C���p���ԗ��ྯ�			*/

			syusei[iti].tei_syu = PayData.teiki.syu;				/* ������						*/
			syusei[iti].tei_syu |= (uchar)(PayData.teiki.pkno_syu<<6);
			syusei[iti].tei_id = (ushort)PayData.teiki.id;			/* �����id(1�`12000)			*/
			syusei[iti].tei_sd = dnrmlzm( (short)PayData.teiki.s_year,/* �L���J�n�N����ɰ�ײ��		*/
									  (short)PayData.teiki.s_mon,	/*								*/
									  (short)PayData.teiki.s_day );	/*								*/
			syusei[iti].tei_ed = dnrmlzm( (short)PayData.teiki.e_year,/* �L���I���N����ɰ�ײ��		*/
									  (short)PayData.teiki.e_mon,	/*								*/
									  (short)PayData.teiki.e_day );	/*								*/
		}															/*								*/
		if(	( 1 == (uchar)prm_get(COM_PRM, S_BUN, 54, 1, 3) )		/* ��������Z���ɕ��ޏW�v�ւ̉��Z���� */
				&&													/*   ����						*/
			( DoBunruiSyu(ryo_buf.syubet) ) ){						/* ���ޏW�v����i��ʖ������j	*/
				f_BunruiSyu = 1;									/* ���ޏW�v������Z�b�g		*/
		}
	}																/*								*/
	else															/*								*/
	{																/*								*/
		if(( ryo_buf.pkiti != 0xffff )&&							/* ������Z�ȊO					*/
		   ( uc_update == 0 ))										/* ������X�V���Z�ȊO			*/
			syusei[iti].ryo = ryo_buf.tyu_ryo;						/* �C���p���ԗ��ྯ�			*/

		if( DoBunruiSyu(ryo_buf.syubet) ){							/* ���ޏW�v����i��ʖ������j	*/
			f_BunruiSyu = 1;										/* ���ޏW�v������Z�b�g		*/
		}
	}																/*								*/
																	/*								*/
	if( f_BunruiSyu ){												/* ���ޏW�v����					*/

		dat2 = 0L;													/*								*/
		// ���ԗ��������߂�i�������܂ށ^�܂܂Ȃ��ݒ�ɏ]���j
		switch( prm_get(COM_PRM, S_TOT, 12, 1, 2) ){				// ���ޏW�v�̒��ԗ��������i�ݒ�j�ɂ�蕪��
		case	0:													// �����O�̗����𒓎ԗ����Ƃ���
		default:
			dat2 = PayData.WPrice;
			break;
		case	1:													// ������̗����𒓎ԗ����Ƃ���i�v���y�͌����̎��z�Ƃ���j
			if( PayData.WPrice >= (wari_ryo + ryo_buf.credit.pay_ryo) )
				dat2 = (PayData.WPrice) - wari_ryo - ryo_buf.credit.pay_ryo;
																	// ���ԗ����|��������
			break;
		case	2:													// ������̗����𒓎ԗ����Ƃ���i�v���y�͊����z�Ƃ���j
			if( PayData.WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo + PayData.Electron_data.Suica.pay_ryo) )
				dat2 = (PayData.WPrice) - wari_ryo - ppc_ryo - ryo_buf.credit.pay_ryo - PayData.Electron_data.Suica.pay_ryo;
																	// ���ԗ����|���������|PPC�����|�d�q�Ȱ����
			break;
		}

#if 1 // AKIBA DEBUG TEST 08.06.27(S)
//		bunrui( ryo_buf.pkiti, dat2, ryo_buf.syubet );				/* ���ޏW�v���Z		*/
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
//		if( wk_sss_flg ){											// �C�����Z��
//			if( prm_get(COM_PRM, S_TOT, 20, 1, 6) == 1 ){			// �W�v���Z�̂�蒼������
//				if( prm_get(COM_PRM, S_TOT, 12, 1, 2) != 0 ){		// ������̗���
//					if( !(PayData.Zengakufg&0x02) ){				// �������Ȃ�
//						dat2 += syusei[wdt1].gen;
//						dat2 -= PayData.MMTwari;
//					}else{											// ����������
//						dat2 += syusei[wdt1].gen;
//						dat2 -= PayData.MMTwari;
//					}
//				}
//			}
//		}
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X(�̎��؈�)
#endif // AKIBA DEBUG TEST 08.06.27(S)
// �d�l�ύX(S) K.Onodera 2016/12/13 #1674 �W�v�̐��Z�����ɑ΂���U�֕��̉��Z���@��ύX����
		if( ulFuriWari ){
			dat2 -= ulFuriWari;
		}
// �d�l�ύX(E) K.Onodera 2016/12/13 #1674 �W�v�̐��Z�����ɑ΂���U�֕��̉��Z���@��ύX����
		if( ryo_buf.pkiti != 0xffff ){
			bunrui( ryo_buf.pkiti, dat2, ryo_buf.syubet );			/* ���ޏW�v���Z		*/
// �d�l�ύX(S) K.Onodera 2016/12/13 #1674 �W�v�̐��Z�����ɑ΂���U�֕��̉��Z���@��ύX����
			// �U�֐��Z�H
			if( ulFurikae ){
				bunrui_Erace( GetFurikaeSrcReciptData(), ulFurikae, ryo_buf.syubet );
			}
// �d�l�ύX(E) K.Onodera 2016/12/13 #1674 �W�v�̐��Z�����ɑ΂���U�֕��̉��Z���@��ύX����
		}else{
			bunrui( 0, dat2, ryo_buf.syubet );						/* ���ޏW�v���Z		*/
		}
	}

	ts->tou[0] += (ulong)ryo_buf.in_coin[0];						/* ���Z�������z  10�~�g�p����	*/
	ts->tou[1] += (ulong)ryo_buf.in_coin[1];						/* ���Z�������z  50�~�g�p����	*/
	ts->tou[2] += (ulong)ryo_buf.in_coin[2];						/* ���Z�������z 100�~�g�p����	*/
	ts->tou[3] += (ulong)ryo_buf.in_coin[3];						/* ���Z�������z 500�~�g�p����	*/
	ts->tou[4] += (ulong)ryo_buf.in_coin[4];						/* ���Z�������z1000�~�g�p����	*/
	ts->tounyu = ts->tou[0]*10 + ts->tou[1]*50 + ts->tou[2]*100 +	/*								*/
				 ts->tou[3]*500 + ts->tou[4]*1000;					/* ���Z�������z	���z			*/
																	/*								*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[0];						/* ���Z���o���z  10�~�g�p����	*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[1];						/* ���Z���o���z  50�~�g�p����	*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[2];						/* ���Z���o���z 100�~�g�p����	*/
	ts->sei[3] += (ulong)ryo_buf.out_coin[3];						/* ���Z���o���z 500�~�g�p����	*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[4];						/* ���Z���o���z  10�~�g�p����(�\�~)*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[5];						/* ���Z���o���z 50�~�g�p����(�\�~)*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[6];						/* ���Z���o���z 100�~�g�p����(�\�~)*/
	ts->seisan = ts->sei[0]*10 + ts->sei[1]*50 + ts->sei[2]*100 +	/*								*/
				 ts->sei[3]*500;									/* ���Z���o���z	���z			*/
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 15;											/* 15:�s�ر�W�v���Z����			*/
	Log_regist( LOG_PAYMENT );										/* �ʐ��Z���o�^				*/
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
	if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		Set_Pay_RTPay_Data();	// ���Z����log�ް��̍쐬
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		Set_Pay_RTReceipt_Data();
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		// ر���ѐ��Z�ް��̾����ǔԂ��X�V����log�ɏ������ޏ���
		RTPay_LogRegist_AddOiban();
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		if ( OPECTL.f_eReceiptReserve ) {	// �d�q�̎��ؗ\��
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
// 			// RT�̎��؃f�[�^�o�^
// 			RTReceipt_LogRegist_AddOiban();
// 
// 			// QR�̎��؃f�[�^�쐬
// 			MakeQRCodeReceipt(qrcode, sizeof(qrcode));
// 
// 			// QR�̎��؃|�b�v�A�b�v�\��
// 			lcdbm_notice_dsp3(POP_QR_RECIPT, 0, 0, (uchar *)qrcode, sizeof(qrcode));	// 12:QR�̎���
			// QR�̎��ؕ\��
			QrReciptDisp();
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
		}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		// QR�m��E����f�[�^�o�^
		ope_SendCertifCommit();
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
	}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
	Suica_Rec.Data.BIT.LOG_DATA_SET = 0;							/* Suica���Z�׸޸ر				*/
	ac_flg.cycl_fg = 19;		// ���O�o�^����
																	/*								*/
	if( ryo_buf.ryo_flg >= 2 )										/* ����g�p?					*/
	{																/*								*/
		memcpy( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* ���~����ް��ޔ�				*/
																	/*								*/
		ac_flg.cycl_fg = 20;		// ��������~�e�[�u���ޔ������`���X�V��
																	/*								*/
		for( i=0; i<TKI_CYUSI_MAX; i++ )							/*								*/
		{															/*								*/
			if( tki_cyusi.dt[i].no == PayData.teiki.id &&			/* ���~�ޯ̧�Ɏg�p����ް��L��?	*/
				tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*					*/
			{														/*								*/
				NTNET_Snd_Data219(1, &tki_cyusi.dt[i]);				/* 1���폜�ʒm���M				*/
				TKI_Delete(i);										/* ���~���1���ر				*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* �����ð��ٍX�V(�o��)	*/
					  (ushort)PayData.teiki.id,						/*								*/
					  1,											/*								*/
					  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*							*/
					  0xffff );										/*								*/
	}																/*								*/
																	/*								*/
	CountUp( PAYMENT_COUNT );										/* ���Z�ǔ�+1					*/
	if( ryo_buf.fusoku != 0 ){										/* �a��ؔ��s?					*/
		CountUp(DEPOSIT_COUNT);										/* �a��ؒǔ�+1					*/
	}
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
	}
// MH810100(S) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
	if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		DC_PopCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔԂ�Pop
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
	}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
																	/*								*/
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	if( PayData.credit.pay_ryo ){									/* �ڼޯĶ��ގg�p				*/
//		ac_flg.cycl_fg = 80;										// �N���W�b�g���p���דo�^�J�n
//		Log_regist( LOG_CREUSE );									/* �N���W�b�g���p���דo�^		*/
//	}																/*								*/
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	if (EcUseEMoneyKindCheck(PayData.Electron_data.Ec.e_pay_kind) &&
		PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
		// ��������ς݂��m�F�����ꍇ��������������L�^��o�^����
		ac_flg.cycl_fg = 90;										// ������������L�^�o�^�J�n
		EcAlarmLog_Regist(&EcAlarm.Ec_Res);
		ac_flg.cycl_fg = 91;										// ������������L�^�o�^����
		// ������������L�^�̍Đ��Z����o�^����
		EcAlarmLog_RepayLogRegist(&PayData);
	}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	ac_flg.cycl_fg = 17;											/* 17:�ʐ��Z�ް��o�^����		*/
																	/*								*/
	return;															/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ��ʖ� ���ޏW�v����^���Ȃ��`�F�b�N															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: chu_syuu( void )																   |*/
/*| PARAMETER	: ���  : 0=A��A...11=L��														   |*/
/*| RETURN VALUE: 1=���ޏW�v����A0=���Ȃ�														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Date		: 2005-10-25																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	DoBunruiSyu( char Syubetu )									/*								*/
{																	/*								*/
	short	Num;													/* ���ʐݒ�ԍ�	(52 or 53)		*/
	char	Ichi;													/* ���ʐݒ�� �f�[�^�ʒu(1-6)	*/

	if( 6 > Syubetu )
		Num = 52;
	else
		Num = 53;

	Ichi = (char)((Syubetu % 6) + 1);

	if( 0 != (uchar)prm_get(COM_PRM, S_BUN, Num, 1, Ichi) )			/* ��ʖ� ���ޏW�v�ւ̉��Z���� 	*/
		return	(char)1;											/* ���ޏW�v����					*/

	return	(char)0;												/* ���ޏW�v���Ȃ�				*/
}
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���Z���~�W�v																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: chu_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT45EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	cyu_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	ulong	dat1, dat2;												/*								*/
	ushort	iti;													/*								*/
	short	i;														/*								*/
	uchar	f_SameData;												/* 1=������~ð��ٓ��ɖړI�ް����� */
	uchar	uc_update = PayData.teiki.update_mon;					/* �X�V���i�[					*/
	ushort	Flp_sub_no;
	uchar	Flp_sub_ok;
	uchar	wk_cnt=0;
	uchar	uc_buf_size;
	wari_tiket	wari_dt;
	short	j;
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	DATE_SYOUKEI	*date_ts;
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
	uchar	MiryoaddFlag = 0;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
	uchar	taxableAdd_set;											/*	�ېőΏ۔�����Z�ݒ�		*/
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	date_ts = &Date_Syoukei;
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	iti = ryo_buf.pkiti - 1;										/* ���Ԉʒu						*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* �s�W�v��ܰ��ر�ցi��d�΍�j	*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	date_uriage_syoukei_judge();									// ���t�ؑ֊ ������A���������㏬�v�̎����X�V����i���A���^�C�����p�j
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	DailyAggregateDataBKorRES( 0 );									/* �����W�v�G���A�̃o�b�N�A�b�v	*/
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
																	/*								*/
	ac_flg.cycl_fg = 24;											/* 24:�s�W�v��ܰ��ر�֓]������	*/
																	/*								*/
																	/*								*/
	ts->Seisan_chusi_cnt += 1L;										/* ���Z���~��					*/
	ts->Seisan_chusi_ryo += ryo_buf.nyukin;							/* ���~�����������z				*/
																	/*								*/
	if( ryo_buf.fusoku != 0 )										/* �x�����s���z�L��H			*/
	{																/*								*/
		ts->Harai_husoku_cnt += 1L;									/* �x�����s����				*/
		ts->Harai_husoku_ryo += ryo_buf.fusoku;						/* �x�����s���z					*/
	}																/*								*/

// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	if (EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
		PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) {

		if (PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0) {
			// ���ϒ��ɏ�Q�����A�܂��́A�����c���Ɖ�^�C���A�E�g����
			// ������������i�x���s���j�W�v
			ts->miryo_unknown_cnt += 1L;
			ts->miryo_unknown_ryo += EcAlarm.Ec_Res.settlement_data;

			// �����W�v�͓����Ƃ��ăJ�E���g����
			Syuukei_sp.ec_inf.now.sp_miryo_unknown_cnt += 1L;
			Syuukei_sp.ec_inf.now.sp_miryo_unknown_ryo += EcAlarm.Ec_Res.settlement_data;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
			MiryoaddFlag = 1;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
		}
	}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
																	/*								*/
	if(Flap_Sub_Flg == 1){									// �Ő��Z�{B�G���A�L
		Flp_sub_ok = 1;
		Flp_sub_no = Flap_Sub_Num;							// ��ƒ��ڍ״ر�i�[
	}else if((Flap_Sub_Flg == 2) || (Flap_Sub_Num == 10)){	// �Đ��Z�{B�G���A���A�}���`���Z(B�ر��)�̎�
		Flp_sub_ok = 0;
	}else{														// �Đ��Z���A�����Z���~
		for(i = 0 ; i < 10 ; i++){
			if(FLAPDT_SUB[i].WPlace == 0){
				break;
			}
		}
		if(i < 10){
			Flp_sub_ok = 1;
			Flp_sub_no = i;
		}else{
			memmove(&FLAPDT_SUB[0],&FLAPDT_SUB[1],sizeof(flp_com_sub)*9);			//�O�ɂP�V�t�g
			memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));	//�ر
			Flp_sub_ok = 1;
			Flp_sub_no = 9;
		}
	}
	if( CPrmSS[S_PRP][1] == 1 )										/* ����߲�޶��ނ��g�p����ݒ�	*/
	{																/*								*/
		dat1 = prm_get( COM_PRM,S_TOT,5,1,3 );						/* ����߲�޶��ޑ�����z/���|���z���Z�ݒ�	*/
		j = 0;														/*								*/
		for( i = 0; i < WTIK_USEMAX; i++ ){							/*								*/
			disc_wari_conv( &PayData.DiscountData[i], &wari_dt );	/*								*/
			if( wari_dt.tik_syu == PREPAID ){						/* �ް�����						*/
				dat2 = wari_dt.pkno_syu;							/*								*/
				ts->Pcrd_use_cnt[dat2] += ryo_buf.pri_mai[j];		/* ����߲�޶��ގg�p���񐔁�(��{,�g��1,2,3)	*/
																	/* 7���ȏ�g�p���͍Ō�Ɏg��ꂽ��ʂɍ��v���Z����	*/
				ts->Pcrd_use_ryo[dat2] += wari_dt.ryokin;			/* ����߲�޶��ގg�p�����z��(��{,�g��1,2,3)	*/
																	/*								*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				if(Flp_sub_ok){										/* �ڍג��~�G���A�g�p			*/
					FLAPDT_SUB[Flp_sub_no].ppc_chusi_ryo += wari_dt.ryokin;	/* �ڍג��~�G���A�ɉ��Z���� */
				}													/*								*/
				j++;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ )								/*								*/
	{																/*								*/
		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );		/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
		taxableAdd_set = 0xff;
		if( cancelReceipt_Waridata_chk( &wari_dt ) ){				/*			�ېőΏ�			*/
			taxableAdd_set = (uchar)prm_get( COM_PRM, S_TOT, 7, 1, 2 );	/*�ېőΏۗ��p�z�̔�����Z�ݒ�*/
		}															/*								*/
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
		switch( wari_dt.tik_syu )									/*								*/
		{															/*								*/
			case SERVICE:											/* ���޽��						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* ���޽��A,B,C					*/
																	/*								*/
				ts->Stik_use_cnt[dat1][dat2] += wari_dt.maisuu;		/* ���޽����ʖ��g�p��������(��{,�g��1,2,3)(���޽��A,B,C)	*/
				ts->Stik_use_ryo[dat1][dat2] += wari_dt.ryokin;		/* ���޽����ʖ��g�p�����z��(��{,�g��1,2,3)(���޽��A,B,C)	*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//				if( CPrmSS[S_SER][1+3*dat2] == 4 &&					/* ���޽���S�z�����ݒ�?			*/
				if( prm_get( COM_PRM, S_SER, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* ���޽���S�z�����ݒ�? */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
					ryo_buf.pkiti != 0xffff &&						/* ������Z�ȊO?					*/
					uc_update == 0 )								/* ������X�V���Z�ȊO			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* �C���p�S�z�����׸޾��		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,1 );				/* ���޽��������z/���|���z���Z�ݒ�	*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				setting_taxableAdd( &dat1, taxableAdd_set );		/* �ېőΏۗ��p�z�̔�����Z�ݒ� */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				if(Flp_sub_ok){
					FLAPDT_SUB[Flp_sub_no].sev_tik[dat2] += wari_dt.maisuu;	// �ڍג��~�G���A�ɉ��Z����
				}
				break;												/*								*/
																	/*								*/
			case KAKEURI:											/* �|����						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* �X��1�`999(0�`998)			*/
																	/*								*/
				ts->Mno_use_Tcnt += wari_dt.maisuu;					/* �X������İ�ف��񐔁�			*/
				ts->Mno_use_Tryo += wari_dt.ryokin;					/* �X������İ�ف����z��			*/
																	/*								*/
				ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;			/* �X������İ�ف��񐔁�(��{,�g��1,2,3)	*/
				ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;			/* �X������İ�ف����z��(��{,�g��1,2,3)	*/
																	/*								*/
				if( dat1 == KIHON_PKNO || dat1 == KAKUCHOU_1 )		/* ��{ or �g��1				*/
				{													/*								*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
//					if( dat2 < 100 )								/* �X��1�`100					*/
					if( dat2 < MISE_NO_CNT )						/* �X��1�`100					*/
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* �X��1�`100İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* �X��1�`100İ�ي��������z��(��{,�g��1,2,3)	*/
						ts->Mno_use_cnt4[dat1][dat2] +=				/* �X�����������񐔁�(��{,�g��1)(�X��1�`100)	*/
												wari_dt.maisuu;		/*								*/
						ts->Mno_use_ryo4[dat1][dat2] +=				/* �X�������������z��(��{,�g��1)(�X��1�`100)	*/
												wari_dt.ryokin;		/*								*/
					}												/*								*/
					else											/* �X��101�`999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* �X��101�`999İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* �X��101�`999İ�ي��������z��(��{,�g��1,2,3)	*/
					}												/*								*/
				}													/*								*/
				else												/*								*/
				{													/*								*/
					if(( dat1 == KAKUCHOU_2 )&&						/* �g��2						*/
					   ( prm_get( COM_PRM,S_TOT,16,1,3 ) == 1 ))	/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
					else if(( dat1 == KAKUCHOU_3 )&&				/* �g��3						*/
							( prm_get( COM_PRM,S_TOT,16,1,4 ) == 1 ))/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
//					if( dat2 < 100 )								/* �X��1�`100					*/
					if( dat2 < MISE_NO_CNT )						/* �X��1�`100					*/
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* �X��1�`100İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* �X��1�`100İ�ي��������z��(��{,�g��1,2,3)	*/
						if( dat1 == KAKUCHOU_1 )					/*								*/
						{											/*								*/
							ts->Mno_use_cnt4[dat1][dat2] +=			/* �X�����������񐔁�(��{,�g��1)(�X��1�`100)	*/
												wari_dt.maisuu;		/*								*/
							ts->Mno_use_ryo4[dat1][dat2] +=			/* �X�������������z��(��{,�g��1)(�X��1�`100)	*/
												wari_dt.ryokin;		/*								*/
						}											/*								*/
					}												/*								*/
					else											/* �X��101�`999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* �X��101�`999İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* �X��101�`999İ�ي��������z��(��{,�g��1,2,3)	*/
					}												/*								*/
				}													/*								*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//				if( CPrmSS[S_STO][1+3*dat2] == 4 &&					/* �X�����S�z�����ݒ�?			*/
				if( prm_get( COM_PRM, S_STO, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* �X�����S�z�����ݒ�? */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
					ryo_buf.pkiti != 0xffff &&						/* ������Z�ȊO?					*/
					uc_update == 0 )								/* ������X�V���Z�ȊO			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* �C���p�S�z�����׸޾��		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* �X����������z/���|���z���Z�ݒ�	*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				setting_taxableAdd( &dat1, taxableAdd_set );		/* �ېőΏۗ��p�z�̔�����Z�ݒ� */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				if(Flp_sub_ok){
					for(wk_cnt=0;wk_cnt<5;wk_cnt++){				//�����XNo����
						if(FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no == wari_dt.syubetu){
							FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].maisuu += wari_dt.maisuu;//�����XNo�̎��A�������i�[����
							break;
						}
					}
					if(wk_cnt == 5){								//�����XNo��
						for(wk_cnt=0;wk_cnt<5;wk_cnt++){
							if(FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no == 0){
								FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no = wari_dt.syubetu;//�X��No
								FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].maisuu += wari_dt.maisuu;//����
								break;
							}
						}
					}
					if(wk_cnt == 5){								//�����XNo���{�ڍג��~�󂫴ر��

						if(Flp_sub_no < 9){
							uc_buf_size = (uchar)(9 - Flp_sub_no);
							memset(&FLAPDT_SUB[Flp_sub_no],0,sizeof(flp_com_sub));			// �ڍג��~�ر(���Z����)�ر
							memmove(&FLAPDT_SUB[Flp_sub_no],&FLAPDT_SUB[Flp_sub_no+1],sizeof(flp_com_sub)*uc_buf_size);
						}
						if(Flap_Sub_Num == 10){
							memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));	// �I�[�ڍג��~�ر�ر
						}else{
							memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));	// �I�[�ڍג��~�ر�ر
						}
						Flp_sub_ok = 0;								//�i�[��׸޸ر
					}
				}
				break;												/*								*/

			case KAISUU:											/* �񐔌�						*/
				if( CPrmSS[S_PRP][1] != 2 )							/* �񐔌����g�p����ݒ�ł͂Ȃ�	*/
					break;

				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
																	/*								*/
				ts->Ktik_use_cnt[dat1] += wari_dt.maisuu;			/* �g�p��������(��{,�g��1,2,3) */
				ts->Ktik_use_ryo[dat1] += wari_dt.ryokin;			/* �g�p�����z��(��{,�g��1,2,3)	*/

				dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );				/* �񐔌�������z/���|���z���Z�ݒ�	*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				if(Flp_sub_ok){										//�ڍג��~�G���A�g�p
					FLAPDT_SUB[Flp_sub_no].ppc_chusi_ryo += wari_dt.ryokin;	// �ڍג��~�G���A�ɉ��Z����B
				}
				break;												/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
																	/*								*/
	ts->tou[0] += (ulong)ryo_buf.in_coin[0];						/* ���Z�������z  10�~�g�p����	*/
	ts->tou[1] += (ulong)ryo_buf.in_coin[1];						/* ���Z�������z  50�~�g�p����	*/
	ts->tou[2] += (ulong)ryo_buf.in_coin[2];						/* ���Z�������z 100�~�g�p����	*/
	ts->tou[3] += (ulong)ryo_buf.in_coin[3];						/* ���Z�������z 500�~�g�p����	*/
	ts->tou[4] += (ulong)ryo_buf.in_coin[4];						/* ���Z�������z1000�~�g�p����	*/
	ts->tounyu = ts->tou[0]*10 + ts->tou[1]*50 + ts->tou[2]*100 +	/*								*/
				 ts->tou[3]*500 + ts->tou[4]*1000;					/* ���Z�������z	���z			*/
																	/*								*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[0];						/* ���Z���o���z  10�~�g�p����	*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[1];						/* ���Z���o���z  50�~�g�p����	*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[2];						/* ���Z���o���z 100�~�g�p����	*/
	ts->sei[3] += (ulong)ryo_buf.out_coin[3];						/* ���Z���o���z 500�~�g�p����	*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[4];						/* ���Z���o���z  10�~�g�p����(�\�~)*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[5];						/* ���Z���o���z 50�~�g�p����(�\�~)*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[6];						/* ���Z���o���z 100�~�g�p����(�\�~)*/
	ts->seisan = ts->sei[0]*10 + ts->sei[1]*50 + ts->sei[2]*100 +	/*								*/
				 ts->sei[3]*500;									/* ���Z���o���z	���z			*/
																	/*								*/
	if(( card_use[USE_SVC] || card_use[USE_PPC] || card_use[USE_NUM] )&&	/* �����L��?					*/
	   ryo_buf.pkiti != 0xffff &&									/* ������Z�ȊO?					*/
	   uc_update == 0 )												/* ������X�V���Z�ȊO			*/
	{																/*								*/
		FLAPDT.flp_data[iti].bk_syu = (ushort)(ryo_buf.syubet + 1);	/* ���(���~,�C���p)			*/
		FLAPDT.flp_data[iti].bk_wmai = card_use[USE_SVC];			/* �g�p����(���~,�C���p)		*/
		FLAPDT.flp_data[iti].bk_wari = ryo_buf.fee_amount;			/* �������z(���~,�C���p)		*/
		FLAPDT.flp_data[iti].bk_time = ryo_buf.tim_amount;			/* �������Ԑ�(���~,�C���p)		*/
		if(Flp_sub_ok){
			FLAPDT_SUB[Flp_sub_no].syu = (uchar)(ryo_buf.syubet + 1);		// ���
			FLAPDT_SUB[Flp_sub_no].TInTime = PayData.TInTime;				// ���Ɏ���
			FLAPDT_SUB[Flp_sub_no].WPlace = PayData.WPlace;			// �Ԏ��ԍ����i�[����
		}
	}																/*								*/
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 25;											/* 25:�s�ر�W�v���Z����			*/
																	/*								*/
	if( ryo_buf.ryo_flg >= 2 )										/* ����g�p?					*/
	{																/*								*/
		memcpy( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* ���~����ް��ޔ�				*/
																	/*								*/
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
//		Log_regist( LOG_PAYSTOP );									/* ���Z���~���o�^				*/
		if(	MiryoaddFlag == 1){
			Log_regist( LOG_PAYSTOP_FU );									/* ���Z���~���o�^				*/
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime���Z���~�f�[�^���M����
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
				Set_Cancel_RTPay_Data();
				// ر���ѐ��Z�ް��̾����ǔԂ��X�V����log�ɏ������ޏ���
				RTPay_LogRegist_AddOiban();
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			}
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			// QR�m��E����f�[�^�o�^
			ope_SendCertifCancel();
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
		Suica_Rec.Data.BIT.LOG_DATA_SET = 0;						/* Suica���Z�׸޸ر				*/
																	/*								*/
		f_SameData = 0;												/*								*/
																	/*								*/
		if( TKI_CYUSI_MAX < tki_cyusi.count )						/* fail safe					*/
			tki_cyusi.count = TKI_CYUSI_MAX;						/*								*/
																	/*								*/
		if( TKI_CYUSI_MAX <= tki_cyusi.wtp )						/* fail safe					*/
			tki_cyusi.wtp = tki_cyusi.count - 1;					/*								*/
																	/*								*/
		for( i=0; i<tki_cyusi.count; i++ )							/*								*/
		{															/*								*/
			if( 0L == tki_cyusi.dt[i].pk ){							/* ð��ق��ް��Ȃ�				*/
				break;												/* i=�o�^����					*/
			}														/*								*/
			if( tki_cyusi.dt[i].no == PayData.teiki.id &&			/* �Ē��~?						*/
				tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*					*/
			{														/*								*/
				f_SameData = 1;										/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		if( 0 == f_SameData )										/* �����ް��Ȃ�(�V�K�o�^�K�v)	*/
		{															/*								*/
			if( i == TKI_CYUSI_MAX ){								/* �o�^����Full					*/
				NTNET_Snd_Data219(1, &tki_cyusi.dt[0]);				/* 1���폜�ʒm���M				*/
				TKI_Delete(0);										/* �Ō��ް��폜					*/
				nmisave( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* ���~����ް��ޔ�		*/
			}														/*								*/
			tki_cyusi.dt[tki_cyusi.wtp].syubetu = PayData.syu;		/* �������(A�`L:1�`12)			*/
			tki_cyusi.dt[tki_cyusi.wtp].pk =						/*								*/
						CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];	/* ���ԏꇂ						*/
			tki_cyusi.dt[tki_cyusi.wtp].no = PayData.teiki.id;		/* �l����(1�`12000)			*/
			tki_cyusi.dt[tki_cyusi.wtp].tksy = PayData.teiki.syu;	/* ������(1�`15)				*/
			tki_cyusi.dt[tki_cyusi.wtp].year = PayData.TInTime.Year;/* �����N						*/
			memcpy( &tki_cyusi.dt[tki_cyusi.wtp].mon, PayData.teiki.t_tim, 4 );	/* ������������		*/
			tki_cyusi.dt[tki_cyusi.wtp].sec = 0;					/* �����b						*/
																	/*								*/
			NTNET_Snd_Data219(0, &tki_cyusi.dt[tki_cyusi.wtp]);		/* 1���X�V�ʒm���M				*/
																	/*								*/
			if( tki_cyusi.count < TKI_CYUSI_MAX )					/*								*/
			{														/*								*/
				tki_cyusi.count++;									/* �o�^������+1					*/
			}														/*								*/
																	/*								*/
			tki_cyusi.wtp++;										/* ײ��߲��+1					*/
			if( tki_cyusi.wtp >= TKI_CYUSI_MAX )					/*								*/
			{														/*								*/
				tki_cyusi.wtp = 0;									/*								*/
			}														/*								*/
		}															/*								*/
		WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* �����ð��ٍX�V(�o��)	*/
					  (ushort)PayData.teiki.id,						/*								*/
					  1,											/*								*/
					  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*							*/
					  0xffff );										/*								*/
	}																/*								*/
	else{															/*								*/
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
//		Log_regist( LOG_PAYSTOP );									/* ���Z���~���o�^				*/
		if(	MiryoaddFlag == 1){
			Log_regist( LOG_PAYSTOP_FU );									/* ���Z���~���o�^				*/
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime���Z���~�f�[�^���M����
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
				Set_Cancel_RTPay_Data();
				// ر���ѐ��Z�ް��̾����ǔԂ��X�V����log�ɏ������ޏ���
				RTPay_LogRegist_AddOiban();
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			}
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			// QR�m��E����f�[�^�o�^
			ope_SendCertifCancel();
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
		Suica_Rec.Data.BIT.LOG_DATA_SET = 0;						/* Suica���Z�׸޸ر				*/
	}																/*								*/
																	/*								*/
	CountUp( CANCEL_COUNT );										/* ���Z���~�ǔ�+1				*/
	if( ryo_buf.fusoku != 0 ){										/* �a��ؔ��s?					*/
		CountUp(DEPOSIT_COUNT);										/* �a��ؒǔ�+1					*/
	}
// MH810100(S) 2020/06/22  #4507 ID56�̾���ǔԂ��������ݸ���Ă���Ă��Ȃ��ꍇ������
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
//	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
//		RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
//	}
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5379�F���Z���~�̐��Z�f�[�^���M��ɐ��Z�f�[�^�̃Z���^�[�ǔԂ��d������s��̑΍�j
//	if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime���Z���~�f�[�^���M����
	if( PAY_CAN_DATA_SEND || MiryoaddFlag == 1 ){
		// ParkingWeb/RealTime���Z���~�f�[�^���M����܂��͌��σ��[�_�ɂ�鐸�Z���~���o�^��
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5379�F���Z���~�̐��Z�f�[�^���M��ɐ��Z�f�[�^�̃Z���^�[�ǔԂ��d������s��̑΍�j
		if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
		}
	}
// MH810100(E) 2020/06/22  #4507 ID56�̾���ǔԂ��������ݸ���Ă���Ă��Ȃ��ꍇ������
// MH810100(S) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
	if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		DC_PopCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔԂ�Pop
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
	}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
																	/*								*/
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	if ((EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
		 PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) &&
		(PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0)) {
		// ���ϒ��ɏ�Q�����A�܂��́A�����c���Ɖ�^�C���A�E�g�������߁A
		// ������������L�^��o�^����
		ac_flg.cycl_fg = 100;										// ������������L�^�o�^�J�n
		EcAlarmLog_Regist(&EcAlarm.Ec_Res);
	}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	ac_flg.cycl_fg = 27;											/* 27:���Z���~�ް��o�^����		*/
																	/*								*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| ������Z�����W�v(���Z�ް���M�W�v)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: mulkan_syu( pr_lokno )														   |*/
/*| PARAMETER	: pr_lokno : ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-10-31																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	mulkan_syu( ushort pr_lokno )								/*								*/
{																	/*								*/
	LOKTOTAL_DAT	*lkts;											/*								*/
	ulong	dat1;													/*								*/
	ushort	iti;													/*								*/
	struct CAR_TIM wk_time1,wk_time2;								/*								*/
	uchar	i;														/*								*/
	uchar	buf_size;												/*								*/
																	/*								*/
	iti = pr_lokno - 1;												/* ���Ԉʒu						*/
	lkts = &loktl.tloktl.loktldat[iti];								/*								*/
	memcpy( &wkloktotal, lkts, sizeof( LOKTOTAL_DAT ) );			/* �s�W�v��ܰ��ر��(��d�΍�)	*/
																	/*								*/
																	/*								*/
																	/*								*/
//	dat1 = RecvNtnetDt.RData22.InPrice - RecvNtnetDt.RData22.ChgPrice;/* �����z�|�ނ�K�z			*/
	dat1 = RecvNtnetDt.RData22.CashPrice;							/* ��������						*/
																	/*								*/
																	/*								*/
	memset( &syusei[iti], 0, sizeof(struct SYUSEI));				/* �C�����Z�p�ް��ر			*/
																	/*								*/
																	/*								*/
	syusei[iti].gen = dat1;											/* �����̎��z(�C���p)			*/
	syusei[iti].tax = RecvNtnetDt.RData22.Tax;						/* �C���p����Ŋz				*/
																	/*								*/
	syusei[iti].iyear = RecvNtnetDt.RData22.InTime.Year;			/* ���� �N(�C���p)				*/
	syusei[iti].imont = RecvNtnetDt.RData22.InTime.Mon;				/*      ��						*/
	syusei[iti].idate = RecvNtnetDt.RData22.InTime.Day;				/*      ��						*/
	syusei[iti].ihour = RecvNtnetDt.RData22.InTime.Hour;			/*      ��						*/
	syusei[iti].iminu = RecvNtnetDt.RData22.InTime.Min;				/*      ��						*/
	syusei[iti].oyear = RecvNtnetDt.RData22.OutTime.Year;			/* �o�� �N(�C���p)				*/
	syusei[iti].omont = RecvNtnetDt.RData22.OutTime.Mon;			/*      ��						*/
	syusei[iti].odate = RecvNtnetDt.RData22.OutTime.Day;			/*      ��						*/
	syusei[iti].ohour = RecvNtnetDt.RData22.OutTime.Hour;			/*      ��						*/
	syusei[iti].ominu = RecvNtnetDt.RData22.OutTime.Min;			/*      ��						*/
//	syusei[iti].ot_car = 0;											/* ���o��						*/
//	syusei[iti].warigk = ryo_buf.waribik + c_pay;					/* �C���p�����z���				*/
																	/*								*/
//	syusei[iti].sy_wmai = card_use[USE_SVC];						/* �g�p����(�C���p)				*/
//	syusei[iti].sy_wari = ryo_buf.fee_amount;						/* �������z(�C���p)				*/
//	syusei[iti].sy_time = ryo_buf.tim_amount;						/* �������Ԑ�(�C���p)			*/
																	/*								*/
	if(( CPrmSS[S_CAL][19] != 0 )&&									/* ����ŗL��?					*/
	   ( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2 )&&					/* �O��?						*/
	   ( dat1 >= RecvNtnetDt.RData22.Tax ))							/* �ŋ��Ώۊz�����ŋ�?			*/
	{																/*								*/
		lkts->Genuri_ryo += (dat1 - RecvNtnetDt.RData22.Tax);		/* ���Ԉʒu�ԍ��ʏW�v(��������グ)	*/
	} else {														/*								*/
		lkts->Genuri_ryo += dat1;									/* ���Ԉʒu�ԍ��ʏW�v(��������グ)	*/
	}																/*								*/
																	/*								*/
	lkts->Seisan_cnt += 1;											/* ���Ԉʒu�ԍ��ʏW�v(���Z�䐔)	*/
																	/*								*/
	if( RecvNtnetDt.RData22.PassData.PassID )						/* ����g�p?					*/
	{																/*								*/
		syusei[iti].tei_syu =										/*								*/
				(uchar)RecvNtnetDt.RData22.PassData.Syubet;			/* ������						*/
		syusei[iti].tei_id =										/*								*/
				(ushort)RecvNtnetDt.RData22.PassData.PassID;		/* �����id(1�`12000)			*/
		syusei[iti].tei_sd =										/*								*/
			dnrmlzm( (short)RecvNtnetDt.RData22.PassData.SYear,		/* �L���J�n�N����ɰ�ײ��		*/
				(short)RecvNtnetDt.RData22.PassData.SMon,			/*								*/
				(short)RecvNtnetDt.RData22.PassData.SDate );		/*								*/
		syusei[iti].tei_ed =										/*								*/
			dnrmlzm( (short)RecvNtnetDt.RData22.PassData.EYear,		/* �L���I���N����ɰ�ײ��		*/
				(short)RecvNtnetDt.RData22.PassData.EMon,			/*								*/
				(short)RecvNtnetDt.RData22.PassData.EDate );		/*								*/
	}																/*								*/
	syusei[iti].ryo = RecvNtnetDt.RData22.Price;					/* �C���p���ԗ��ྯ�			*/
																	/*								*/
	for(i = 0 ; i < 10 ; i++){										/*	���Z���~�ް�����			*/
		if(FLAPDT_SUB[i].WPlace ==  LockInfo[iti].posi){			/*	���ԏ�No����				*/
			wk_time1.year = RecvNtnetDt.RData22.InTime.Year;		/*								*/
			wk_time1.mon = RecvNtnetDt.RData22.InTime.Mon;			/*								*/
			wk_time1.day = RecvNtnetDt.RData22.InTime.Day;			/*								*/
			wk_time1.hour = RecvNtnetDt.RData22.InTime.Hour;		/*								*/
			wk_time1.min = RecvNtnetDt.RData22.InTime.Min;			/*								*/
			/*�ڍג��~�����ް�*/									/*								*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;				/*								*/
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;				/*								*/
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;				/*								*/
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;				/*								*/
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;				/*								*/
																	/*								*/
			if(0 == ec64(&wk_time1,&wk_time2)){						/*	���Ɏ��ԓ���				*/
				buf_size = (uchar)(9 - i);							/*								*/
				memset(&FLAPDT_SUB[i],0,sizeof(flp_com_sub));		/*	�ڍג��~�ر(���Z����)�ر	*/
				memmove(&FLAPDT_SUB[i],&FLAPDT_SUB[i+1],sizeof(flp_com_sub)*buf_size);
				memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));		/*	�I�[�ڍג��~�ر�ر			*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| ������Z���~�W�v(���Z�ް���M�W�v)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: mulchu_syu( pr_lokno )														   |*/
/*| PARAMETER	: pr_lokno : ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-10-31																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	mulchu_syu( ushort pr_lokno )								/*								*/
{																	/*								*/
	ushort	iti, us_wsyu;											/*								*/
	short	i;														/*								*/
	struct CAR_TIM wk_time1,wk_time2;								/*	ܰ����Դر					*/
	uchar	cyuushi_flg1=0;											/*	���~�ް��ر�׸�				*/
	uchar	cyuushi_flg2=0;											/*	�ڍג��~�ް��ر�׸�			*/
	ushort	flap_no;												/*	���Ԉʒu					*/
	uchar	wk_cnt;													/*	ܰ�����						*/
	uchar	uc_buf_size;
																	/*								*/
	iti = pr_lokno - 1;												/* ���Ԉʒu						*/
																	/*								*/
	if(FLAPDT.flp_data[iti].bk_syu) {								/*								*/
		cyuushi_flg1 = 1;											/*	���~�ް��ر�׸�ON			*/
	}else{															/*								*/
		FLAPDT.flp_data[iti].bk_wari = 0;							/*								*/
		FLAPDT.flp_data[iti].bk_wmai = 0;							/*								*/
		FLAPDT.flp_data[iti].bk_time = 0;							/*								*/
	}																/*								*/
	us_wsyu = RecvNtnetDt.RData22.Syubet;							/* ���~���get					*/
	if( prm_get( COM_PRM,S_SHA,(short)(1+6*(us_wsyu-1)),2,5 ) ){	/* ���~��ʂ͐ݒ�ς�?			*/
		FLAPDT.flp_data[iti].bk_syu = us_wsyu;						/* ���(���~,�C���p)			*/
	}																/*								*/
	for(i = 0 ; i < 10 ; i++){										/*	���Z���~�ް�����			*/
		if(FLAPDT_SUB[i].WPlace ==  LockInfo[iti].posi){			/*	���ԏ�No����				*/
			/*�����ް�*/											/*								*/
			wk_time1.year = RecvNtnetDt.RData22.InTime.Year;		/*								*/
			wk_time1.mon = RecvNtnetDt.RData22.InTime.Mon;			/*								*/
			wk_time1.day = RecvNtnetDt.RData22.InTime.Day;			/*								*/
			wk_time1.hour = RecvNtnetDt.RData22.InTime.Hour;		/*								*/
			wk_time1.min = RecvNtnetDt.RData22.InTime.Min;			/*								*/
			/*�ڍג��~�����ް�*/									/*								*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;				/*								*/
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;				/*								*/
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;				/*								*/
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;				/*								*/
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;				/*								*/
																	/*								*/
			if(0 == ec64(&wk_time1,&wk_time2)){						/*	���Ɏ��ԓ���				*/
				if(cyuushi_flg1 == 0){								/*	���~�ر��					*/
					memset(&FLAPDT_SUB[i],0,sizeof(flp_com_sub));	/*	�ڍג��~�ް��ر�ر			*/
				}													/*								*/
				flap_no = i;										/*	���Ԉʒu���					*/
				cyuushi_flg2 = 1;									/*	�ڍג��~�ް��ر�׸�ON		*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
	if((i == 10) && (cyuushi_flg1 == 0) && (cyuushi_flg2 == 0)){	/*	���~�A�ڍג��~�׸�OFF		*/
		for(i = 0 ; i < 10 ; i++){									/*	�ڍג��~�ر�󂫌���			*/
			if(FLAPDT_SUB[i].WPlace == 0){							/*	�󂫗L						*/
				flap_no = i;										/*								*/
				cyuushi_flg2 = 1;									/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		if(i > 10){													/*	�󂫖�						*/
			memmove(&FLAPDT_SUB[0],&FLAPDT_SUB[1],sizeof(flp_com_sub)*9);/*	��ԌÂ��ް��폜		*/
			memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));			/*	�ŏI�ر�ر					*/
			cyuushi_flg2 = 1;										/*								*/
			flap_no = 9;											/*								*/
		}															/*								*/
		if(cyuushi_flg2){											/*								*/
			FLAPDT_SUB[flap_no].WPlace = LockInfo[iti].posi;		/*								*/
			FLAPDT_SUB[flap_no].TInTime.Year = RecvNtnetDt.RData22.InTime.Year;	/*					*/
			FLAPDT_SUB[flap_no].TInTime.Mon = RecvNtnetDt.RData22.InTime.Mon;	/*					*/
			FLAPDT_SUB[flap_no].TInTime.Day = RecvNtnetDt.RData22.InTime.Day;	/*					*/
			FLAPDT_SUB[flap_no].TInTime.Hour = RecvNtnetDt.RData22.InTime.Hour;	/*					*/
			FLAPDT_SUB[flap_no].TInTime.Min = RecvNtnetDt.RData22.InTime.Min;	/*					*/
			FLAPDT_SUB[flap_no].syu = (uchar)RecvNtnetDt.RData22.Syubet;	/*						*/
		}															/*								*/
	}																/*								*/
	for( i=0; i<NTNET_DIC_MAX; i++ ){								/*								*/
		switch( RecvNtnetDt.RData22.DiscountData[i].DiscSyu ){		/* �������						*/
		case NTNET_SVS_M:											/* ������� ���޽��(����)		*/
		case NTNET_KAK_M:											/* ������� �X����(����)		*/
		case NTNET_TKAK_M:											/* ������� ���X��(����)		*/
		case NTNET_WRI_M:											/* ������� ������(����)		*/
		case NTNET_FRE:												/* ������� �񐔌�				*/
		case NTNET_PRI_W:											/* ������� ��������߲�޶���	*/
			if( RecvNtnetDt.RData22.DiscountData[i].uDiscData.common.DiscFlg == 0 ){/* �V�K���Z(�����ς݂ł͂Ȃ�) */
				FLAPDT.flp_data[iti].bk_wari +=						/*								*/
					RecvNtnetDt.RData22.DiscountData[i].Discount;	/* �������z(���~,�C���p)		*/
				FLAPDT.flp_data[iti].bk_wmai +=						/*								*/
					RecvNtnetDt.RData22.DiscountData[i].DiscCount;	/* �g�p����(���~,�C���p)		*/
			}														/*								*/
			break;													/*								*/
		case NTNET_SVS_T:											/* ������� ���޽��(����)		*/
		case NTNET_KAK_T:											/* ������� �X����(����)		*/
		case NTNET_TKAK_T:											/* ������� ���X��(����)		*/
		case NTNET_WRI_T:											/* ������� ������(����)		*/
			if( RecvNtnetDt.RData22.DiscountData[i].uDiscData.common.DiscFlg == 0 ){/* �V�K���Z(�����ς݂ł͂Ȃ�) */
				FLAPDT.flp_data[iti].bk_time +=						/*								*/
					RecvNtnetDt.RData22.DiscountData[i].uDiscData.common.DiscInfo2;/* �������Ԑ�(���~,�C���p) */
				FLAPDT.flp_data[iti].bk_wmai +=						/*								*/
					RecvNtnetDt.RData22.DiscountData[i].DiscCount;	/* �g�p����(���~,�C���p)		*/
			}														/*								*/
			break;													/*								*/
		}															/*								*/
		if(cyuushi_flg2){											/* ���~�����ް��i�[				*/
			switch( RecvNtnetDt.RData22.DiscountData[i].DiscSyu ){	/* �������						*/
			case NTNET_FRE:											/* ������� �񐔌�				*/
			case NTNET_PRI_W:										/* ������� ��������߲�޶���	*/
				FLAPDT_SUB[flap_no].ppc_chusi_ryo +=
					RecvNtnetDt.RData22.DiscountData[i].Discount;	/* �������z(���~,�C���p)		*/
				break;												/*								*/
			case NTNET_SVS_M:										/* ������� ���޽��(����)		*/
			case NTNET_SVS_T:										/* ������� ���޽��(����)		*/
				FLAPDT_SUB[flap_no].sev_tik[RecvNtnetDt.RData22.DiscountData[i].DiscNo-1] +=
						(uchar)RecvNtnetDt.RData22.DiscountData[i].DiscCount;	/* �g�p����(���~,�C���p)	*/
				break;												/*								*/
			case NTNET_KAK_M:										/* ������� �X����(����)		*/
			case NTNET_TKAK_M:										/* ������� ���X��(����)		*/
			case NTNET_WRI_M:										/* ������� ������(����)		*/
			case NTNET_KAK_T:										/* ������� �X����(����)		*/
			case NTNET_TKAK_T:										/* ������� ���X��(����)		*/
			case NTNET_WRI_T:										/* ������� ������(����)		*/
				for(wk_cnt=0;wk_cnt<5;wk_cnt++){					/* �����XNo����					*/
					if(FLAPDT_SUB[flap_no].kake_data[wk_cnt].mise_no == RecvNtnetDt.RData22.DiscountData[i].DiscNo){
						FLAPDT_SUB[flap_no].kake_data[wk_cnt].maisuu += (uchar)RecvNtnetDt.RData22.DiscountData[i].DiscCount;/*�����XNo�̎��A�������i�[����*/
						break;										/*								*/
					}												/*								*/
				}													/*								*/
				if(wk_cnt == 5){									/* �����XNo��					*/
					for(wk_cnt=0;wk_cnt<5;wk_cnt++){				/*								*/
						if(FLAPDT_SUB[flap_no].kake_data[wk_cnt].mise_no == 0){
							FLAPDT_SUB[flap_no].kake_data[wk_cnt].mise_no = RecvNtnetDt.RData22.DiscountData[i].DiscNo;/*�X��No*/
							FLAPDT_SUB[flap_no].kake_data[wk_cnt].maisuu += (uchar)RecvNtnetDt.RData22.DiscountData[i].DiscCount;/*����*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
				}													/*								*/
				if(wk_cnt == 5){									/* �����XNo���{�ڍג��~�󂫴ر��*/
					if(flap_no < 9){
						uc_buf_size = (uchar)(9 - flap_no);
						memset(&FLAPDT_SUB[flap_no],0,sizeof(flp_com_sub));			// �ڍג��~�ر(���Z����)�ر
						memmove(&FLAPDT_SUB[flap_no],&FLAPDT_SUB[flap_no+1],sizeof(flp_com_sub)*uc_buf_size);
					}
					if(flap_no == 10){
						memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));							// �I�[�ڍג��~�ر�ر
					}else{
						memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));							// �I�[�ڍג��~�ر�ر
					}
					cyuushi_flg2 = 0;								/*	�i�[��׸޸ر				*/
				}													/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �s���o�ɁE�����o�Ɏ��W�v																	   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fus_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: short	: ret	1=�����o��,	2=�s���o��											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	fus_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	ushort	iti;													/*								*/
	struct CAR_TIM wk_time1,wk_time2;
	uchar	i;
	uchar	buf_size;
	short	ret = 1;
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
																	/*								*/
	iti = fusei.fus_d[0].t_iti - 1;									/* ���Ԉʒu(1�`324)-1			*/
																	/*								*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* �s�W�v��ܰ��ر��(��d�΍�)	*/
	memcpy( &wkloktotal,											/* �s�W�v��ܰ��ر��(��d�΍�)	*/
			&loktl.tloktl.loktldat[iti],							/*								*/
			sizeof( LOKTOTAL_DAT ) );								/*								*/
																	/*								*/
	ac_flg.cycl_fg = 32;											/* 32:�s�W�v��ܰ��ر�֓]������	*/
																	/*								*/
																	/*								*/
	if( fusei.fus_d[0].kyousei == 1 || fusei.fus_d[0].kyousei == 11 )/* ��������ŏo�ɁH				*/
	{																/*								*/
		ts->Kyousei_out_Tcnt += 1;									/* �����o�ɉ�					*/
		loktl.tloktl.loktldat[iti].Kyousei_out_cnt += 1;			/* �����o�ɉ�					*/
																	/*								*/
		/*** ���~���Ɋ���������ꍇ�͒��ԗ����|�����������W�v���� ***/
		ts->Kyousei_out_Tryo += ryo_buf.dsp_ryo;					/* �����o�ɉ�					*/
		loktl.tloktl.loktldat[iti].Kyousei_out_ryo +=				/* �����o�ɋ��z					*/
												ryo_buf.dsp_ryo;	/*								*/
	}else{															/* �s��(�ʏ�F0�A�C���F2�A3)	*/
		ts->Husei_out_Tcnt += 1;									/* �s���o�ɉ�					*/
		loktl.tloktl.loktldat[iti].Husei_out_cnt += 1;				/* �s���o�ɉ�					*/
																	/*								*/
		/*** ���~���Ɋ���������ꍇ�͒��ԗ����|�����������W�v���� ***/
																	/*								*/
		// kyousei = 3�͒��ԗ���0�~
		if( fusei.fus_d[0].kyousei != 3 ){							/* �ʏ�A�C���ŗ�������			*/
			ts->Husei_out_Tryo += ryo_buf.dsp_ryo;					/* �s���o�ɋ��z					*/
			loktl.tloktl.loktldat[iti].Husei_out_ryo +=				/* �s���o�ɋ��z					*/
												ryo_buf.dsp_ryo;	/*								*/
		}
		ret = 2;
	}																/*								*/
																	/*								*/
	if( DoBunruiSyu(ryo_buf.syubet) ){								/* ���ޏW�v����i��ʖ������j	*/

		// kyousei = 3�͒��ԗ���0�~
		if( fusei.fus_d[0].kyousei == 3 ){							/* �C�����Z�ł̕s��(����0�~)	*/
			bunrui( 0, 0, ryo_buf.syubet );							/* ���ޏW�v���Z					*/
		}else{														/* �ʏ�s���A�C���ŗ�������		*/
			if( fusei.fus_d[0].kyousei == 1 ){						// �����o��
				if( prm_get(COM_PRM, S_BUN, 59, 1, 1) == 0 ){		// ���ޏW�v���Z����
					bunrui( 0, ryo_buf.dsp_ryo , ryo_buf.syubet );	// ���ޏW�v���Z
				}													// 
			}else{													// �s���o��
				if( prm_get(COM_PRM, S_BUN, 59, 1, 2) == 0 ){		// ���ޏW�v���Z����
					bunrui( 0, ryo_buf.dsp_ryo , ryo_buf.syubet );	// ���ޏW�v���Z
				}													// 
			}														// 
		}
	}
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 33;											/* 33:�s�ر�W�v���Z����			*/
																	/*								*/
	Log_regist( LOG_ABNORMAL );										/* �s���E�����o�ɏ��o�^		*/
																	/*								*/
	ac_flg.cycl_fg = 35;											/* 35:�s���E�����o�ɏ��o�^����*/
																	/*								*/
	memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );			/* �s���ް���ܰ��ر��			*/
																	/*								*/
	ac_flg.cycl_fg = 36;											/* 36:�s���ް���ܰ��ر�֓]������*/
																	/*								*/
	memcpy( &fusei, &wkfus.fus_d[1],								/* �ް����						*/
			sizeof( struct FUSEI_D )*(LOCK_MAX-1) );				/*								*/
	fusei.kensuu -= 1;												/* �����|�P						*/
																	/*								*/
	ac_flg.cycl_fg = 37;											/* 37:�ް���āE����-1����		*/
																	/*								*/
	memset( &fusei.fus_d[LOCK_MAX-1], 0, sizeof( struct FUSEI_D ) );/*								*/
																	/*								*/
	ac_flg.cycl_fg = 38;											/* 38:6���ڸر����				*/
																	/*								*/
	memset( &syusei[iti], 0, sizeof(struct SYUSEI));				/* �C�����Z�p�ް��ر			*/
	for(i = 0 ; i < 10 ; i++){
		if(FLAPDT_SUB[i].WPlace == (ulong)( LockInfo[iti].area*10000L +
					  LockInfo[iti].posi )){		//���Ԉʒu������
			/*���Ɏ���*/
			wk_time1.year = wkfus.fus_d[0].iyear ;
			wk_time1.mon = wkfus.fus_d[0].imont ;
			wk_time1.day = wkfus.fus_d[0].idate ;
			wk_time1.hour = wkfus.fus_d[0].ihour ;
			wk_time1.min = wkfus.fus_d[0].iminu ;
			/*�ڍג��~�����ް�*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;
			
			if(0 == ec64(&wk_time1,&wk_time2)){		//���Ɏ���������
				if(i < 9){
					buf_size = (uchar)(9 - i);
					memset(&FLAPDT_SUB[i],0,sizeof(flp_com_sub));			// �ڍג��~�ر(���Z����)�ر
					memmove(&FLAPDT_SUB[i],&FLAPDT_SUB[i+1],sizeof(flp_com_sub)*buf_size);
				}
				memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));							// �I�[�ڍג��~�ر�ر
				break;
			}
		}
	}
	return ret;														/*								*/
}																	/*								*/
																	/*								*/
#if SYUSEI_PAYMENT
//---------------------------------------------------------------------------------------------------
// �V�C�����Z�@�\�ǉ�(�V�C�����Z�ł͂��̊֐��͎g��Ȃ�)
//---------------------------------------------------------------------------------------------------
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �C�����Z���W�v																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: syu_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	syu_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	LOKTOTAL_DAT	*lkts;											/*								*/
	ushort	itis, itiw;												/*								*/
	short	i;														/*								*/
	ulong	dat1, dat2;												/*								*/
	ulong	wk_Electron_ryo;										/* �d�q�Ȱ���Z���z�Z�[�u		*/
	wari_tiket	wari_dt;
																	/*								*/
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
																	/*								*/
	itis = OPECTL.Pr_LokNo - 1;										/* ���������Ԉʒu�ԍ�			*/
	itiw = OPECTL.MPr_LokNo - 1;									/* �ԈႢ���Ԉʒu�ԍ�			*/
	lkts = &loktl.tloktl.loktldat[itis];							/*								*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* �s�W�v��ܰ��ر�ցi��d�΍�j	*/
	memcpy( &wkloktotal, lkts, sizeof( LOKTOTAL_DAT ) );			/* �s�W�v��ܰ��ر��(��d�΍�)	*/
	dat1 = 0L;														/*								*/
																	/*								*/
	ac_flg.cycl_fg = 74;											/* 74							*/
																	/*								*/
																	/*								*/
	if( ryo_buf.nyukin > ryo_buf.turisen )							/*								*/
	{																/*								*/
		dat1 = ryo_buf.nyukin - ryo_buf.turisen;					/* �����z�|�ނ�K�z				*/
		ts->Genuri_Tryo += dat1;									/* ����������グ				*/
		ts->Uri_Tryo += dat1;										/* ������グ					*/
	} else {														/*								*/
		if( ryo_buf.nyukin == 0 )									/*								*/
		{															/*								*/
			ts->Syuusei_seisan_Tryo += ryo_buf.turisen;				/* �C�����Z���ߋ��z				*/
		}															/*								*/
	}																/*								*/
	if( ryo_buf.ryo_flg < 2 )										/* ����g�p�������Z?			*/
	{																/*								*/
		syusei[itis].ryo = ryo_buf.tyu_ryo;							/* �C���p���ԗ��ྯ�			*/
		if( DoBunruiSyu(ryo_buf.syubet) ){							/* ���ޏW�v����i��ʖ������j	*/
		if( ryo_buf.tyu_ryo > ryo_buf.mis_tyu )						/* ���ԗ���(��)�����ԗ���(��)?	*/
		{															/*								*/
			bunrui( 0, ryo_buf.tyu_ryo - ryo_buf.mis_tyu , ryo_buf.syubet );/* ���ޏW�v���Z			*/
		} else {													/*								*/
			bunrui( 0, 0 ,ryo_buf.syubet );							/*								*/
		}															/*								*/
		}
	} else {														/*								*/
		syusei[itis].ryo = ryo_buf.tei_ryo;							/* �C���p���ԗ��ྯ�			*/
		if(	( 1 == (uchar)prm_get(COM_PRM, S_BUN, 54, 1, 3) )		/* ��������Z���ɕ��ޏW�v�ւ̉��Z���� */
				&&													/*   ����						*/
			( DoBunruiSyu(ryo_buf.syubet) ) ){						/* ���ޏW�v����i��ʖ������j	*/
		if( ryo_buf.tei_ryo > ryo_buf.mis_tyu )						/* ���ԗ���(��)�����ԗ���(��)?	*/
		{															/*								*/
			bunrui( 0, ryo_buf.tei_ryo - ryo_buf.mis_tyu ,ryo_buf.syubet );	/* ���ޏW�v���Z			*/
		} else {													/*								*/
			bunrui( 0, 0 , ryo_buf.syubet );						/*								*/
		}															/*								*/
		}
	}																/*								*/
	lkts->Seisan_cnt += 1;											/* ���Ԉʒu�ԍ��ʏW�v(���Z�䐔)	*/
	syusei[itis].sei = 0;											/* �ߋ����Z�L��𖳂��ɂ���		*/
	syusei[itis].gen = dat1;										/* �����̎��z					*/
	ts->Rsei_cnt[ryo_buf.syubet] += 1L;								/* ��ʖ����Z��				*/
	ts->Seisan_Tcnt += 1L;											/* �����Z��					*/
	ts->Syuusei_seisan_Tcnt += 1L;									/* �C�����Z��					*/
																	/*								*/
	ts->Turi_modosi_ryo += ryo_buf.turisen;							/* �ޑK���ߊz					*/
																	/*								*/
	if( ryo_buf.fusoku != 0 )										/* �x�����s���z�L��?			*/
	{																/*								*/
		ts->Harai_husoku_cnt += 1L;									/* �x�����s����				*/
		ts->Harai_husoku_ryo += ryo_buf.fusoku;						/* �x�����s���z					*/
	}																/*								*/
																	/*								*/
	if(( CPrmSS[S_CAL][19] != 0 )&&									/* ����ŗL��?					*/
	   ( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2 )&&					/* �O��?						*/
	   ( dat1 >= ryo_buf.tax ))										/* �ŋ��Ώۊz�����ŋ�?			*/
	{																/*								*/
		dat2 = 0;													/*								*/
		if( ryo_buf.tax > syusei[itiw].tax )						/* ���������Z�Ŋz���ԈႢ���Z�Ŋz*/
		{															/*								*/
			dat2 = ( ryo_buf.tax - syusei[itiw].tax );				/* ����ł̍��z�����Z			*/
			ts->Tax_Tryo += dat2;									/* �����(���ŁE�O��)			*/
		}															/*								*/
		ts->Rsei_ryo[ryo_buf.syubet] += (dat1 - dat2);				/* ��ʖ���������グ			*/
		lkts->Genuri_ryo += (dat1 - dat2);							/* ���Ԉʒu�ԍ��ʏW�v(��������グ)	*/
	} else {														/*								*/
		ts->Rsei_ryo[ryo_buf.syubet] += dat1;						/* ��ʖ���������グ			*/
		lkts->Genuri_ryo += dat1;									/* ���Ԉʒu�ԍ��ʏW�v(��������グ)	*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.svs_tim )											/* ���޽��ѓ����Z�L��			*/
	{																/*								*/
		ts->In_svst_seisan += 1L;									/* ���޽��ѓ����Z��+1			*/
	}																/*								*/
																	/*								*/
	if( PayData.credit.pay_ryo )									/* �ڼޯĶ��ގg�p				*/
	{																/*								*/
		ts->Ccrd_sei_cnt += 1;										/* �ڼޯĶ��ސ��Z���񐔁�		*/
		ts->Ccrd_sei_ryo += PayData.credit.pay_ryo;					/* �ڼޯĶ��ސ��Z�����z��		*/
																	/*								*/
		dat1 = prm_get( COM_PRM,S_TOT,6,1,6 );						/* �ڼޯĶ��ޑ�����z/���|���z���Z�ݒ�	*/
		if( dat1 == 1L )											/* ������z�ɉ��Z����			*/
		{															/*								*/
			ts->Uri_Tryo += PayData.credit.pay_ryo;					/* ������z						*/
		}															/*								*/
		else if( dat1 == 2L )										/* ���|���z�ɉ��Z����			*/
		{															/*								*/
			ts->Kakeuri_Tryo += PayData.credit.pay_ryo;				/* ���|���z						*/
		}															/*								*/
		else if( dat1 == 3L )										/* ������z/���|���z�ɉ��Z����	*/
		{															/*								*/
			ts->Uri_Tryo += PayData.credit.pay_ryo;					/* ������z						*/
			ts->Kakeuri_Tryo += PayData.credit.pay_ryo;				/* ���|���z						*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	wk_media_Type = Ope_Disp_Media_Getsub(1);						/* �d�q�Ȱ�}�̎�� �擾			*/
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind )){
		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );					/* Suica������z/���|���z���Z�ݒ�	*/
		wk_Electron_ryo = PayData.Electron_data.Suica.pay_ryo;		/* �d�q�Ȱ���Z���z�Z�[�u		*/
		if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {			/* �r��������					*/
			ts->Electron_sei_cnt += 1L;								/* Suica�g�p���񐔁�			*/
			ts->Electron_sei_ryo += wk_Electron_ryo;				/* Suica�g�p�����z��			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		/* �o�`�r�l�n					*/
			ts->Electron_psm_cnt += 1L;								/* PASMO�g�p���񐔁�			*/
			ts->Electron_psm_ryo += wk_Electron_ryo;				/* PASMO�g�p�����z��			*/
		}
	} else if( PayData.Electron_data.Suica.e_pay_kind == EDY_USED ){/* Edy��ް�̎��ް�����			*/
		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 3 );					/* Edy������z/���|���z���Z�ݒ�	*/
		if( EDY_TEST_MODE ){										// ý�Ӱ�ނ̏ꍇ
			dat1 = 0L;												// ������z/���|���z�ɉ��Z���Ȃ�
		}
		else{
			wk_Electron_ryo = PayData.Electron_data.Edy.pay_ryo;	/* �d�q�Ȱ���Z���z�Z�[�u		*/
			if(wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY) {			/* �d����						*/
				ts->Electron_edy_cnt += 1L;							/* �d�����g�p���񐔁�			*/
				ts->Electron_edy_ryo += wk_Electron_ryo;			/* �d�����g�p�����z��			*/
			}
		}
	}
	if( PayData.Electron_data.Suica.e_pay_kind != 0 ) {				/* �d�q�Ȱ(Suica/Edy)�̎��ް�����*/
		if( dat1 == 1L ) {											/* ������z�ɉ��Z����			*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* ������z						*/
		} else if( dat1 == 2L ) {									/* ���|���z�ɉ��Z����			*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* ���|���z						*/
		} else if( dat1 == 3L ) {									/* ������z/���|���z�ɉ��Z����	*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* ������z						*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* ���|���z						*/
		}															/*								*/
	}

	if( CPrmSS[S_PRP][1] )											/* ����߲�޶���,�񐔌��g�p����ݒ�	*/
	{																/*								*/
		if( CPrmSS[S_PRP][1] == 1 )									/* ����߲�޶��ގg�p����ݒ�		*/
		{															/*								*/
			dat1 = prm_get( COM_PRM,S_TOT,5,1,3 );					/* ����߲�޶��ޑ�����z/���|���z���Z�ݒ�	*/
		}															/*								*/
		else														/* �񐔌��g�p����ݒ�			*/
		{															/*								*/
			dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );					/* �񐔌�������z/���|���z���Z�ݒ�	*/
		}															/*								*/
		j = 0;														/*								*/
		for( i = 0; i < WTIK_USEMAX; i++ ){							/*								*/
			disc_wari_conv( &PayData.DiscountData[i], &wari_dt );	/*								*/
			if( wari_dt.tik_syu == PREPAID ){						/* �ް�����						*/
				dat2 = wari_dt.pkno_syu;							/*								*/
				if( CPrmSS[S_PRP][1] == 1 ){						/* ����߲�޶��ގg�p����ݒ�		*/
					ts->Pcrd_use_cnt[dat2] += ryo_buf.pri_mai[j];	/* ����߲�޶��ގg�p���񐔁�(��{,�g��1,2,3)	*/
																	/* 2���ȏ�g�p���͍Ō�Ɏg��ꂽ��ʂɍ��v���Z����	*/
					ts->Pcrd_use_ryo[dat2] += wari_dt.ryokin;		/* ����߲�޶��ގg�p�����z��(��{,�g��1,2,3)	*/
				}													/*								*/
				else												/* �񐔌��g�p����ݒ�			*/
				{													/*								*/
					ts->Ktik_use_cnt[dat2] += 1L;					/* �񐔌��g�p���񐔁�(��{,�g��1,2,3)	*/
					ts->Ktik_use_ryo[dat2] += wari_dt.ryokin;		/* �񐔌��g�p�����z��(��{,�g��1,2,3)	*/
				}													/*								*/
																	/*								*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				j++;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ )								/*								*/
	{																/*								*/
		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );		/*								*/
		switch( wari_dt.tik_syu )									/*								*/
		{															/*								*/
			case SERVICE:											/* ���޽��						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* ���޽��A,B,C					*/
																	/*								*/
				ts->Stik_use_cnt[dat1][dat2] += wari_dt.maisuu;		/* ���޽����ʖ��g�p��������(��{,�g��1,2,3)(���޽��A,B,C)	*/
				ts->Stik_use_ryo[dat1][dat2] += wari_dt.ryokin;		/* ���޽����ʖ��g�p�����z��(��{,�g��1,2,3)(���޽��A,B,C)	*/
																	/*								*/
				if( CPrmSS[S_SER][1+3*dat2] == 4 )					/* ���޽���S�z�����ݒ�?			*/
				{													/*								*/
					syusei[itis].infofg |= SSS_ZENWARI;				/* �C���p�S�z�����׸޾��		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,1 );				/* ���޽��������z/���|���z���Z�ݒ�	*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			case KAKEURI:											/* �|����						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* �X��1�`999(0�`998)			*/
																	/*								*/
				ts->Mno_use_Tcnt += wari_dt.maisuu;					/* �X������İ�ف��񐔁�			*/
				ts->Mno_use_Tryo += wari_dt.ryokin;					/* �X������İ�ف����z��			*/
																	/*								*/
				ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;			/* �X������İ�ف��񐔁�(��{,�g��1,2,3)	*/
				ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;			/* �X������İ�ف����z��(��{,�g��1,2,3)	*/
																	/*								*/
				if( dat1 == KIHON_PKNO || dat1 == KAKUCHOU_1 )		/* ��{ or �g��1				*/
				{													/*								*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
//					if( dat2 < 100 )								/* �X��1�`100					*/
					if( dat2 < MISE_NO_CNT )						/* �X��1�`100					*/
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* �X��1�`100İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* �X��1�`100İ�ي��������z��(��{,�g��1,2,3)	*/
						ts->Mno_use_cnt4[dat1][dat2] +=				/* �X�����������񐔁�(��{,�g��1)(�X��1�`100)	*/
												wari_dt.maisuu;		/*								*/
						ts->Mno_use_ryo4[dat1][dat2] +=				/* �X�������������z��(��{,�g��1)(�X��1�`100)	*/
												wari_dt.ryokin;		/*								*/
					}												/*								*/
					else											/* �X��101�`999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* �X��101�`999İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* �X��101�`999İ�ي��������z��(��{,�g��1,2,3)	*/
					}												/*								*/
				}													/*								*/
				else												/*								*/
				{													/*								*/
					if(( dat1 == KAKUCHOU_2 )&&						/* �g��2						*/
					   ( prm_get( COM_PRM,S_TOT,16,1,3 ) == 1 ))	/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
					else if(( dat1 == KAKUCHOU_3 )&&				/* �g��3						*/
							( prm_get( COM_PRM,S_TOT,16,1,4 ) == 1 ))/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
//					if( dat2 < 100 )								/* �X��1�`100					*/
					if( dat2 < MISE_NO_CNT )						/* �X��1�`100					*/
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* �X��1�`100İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* �X��1�`100İ�ي��������z��(��{,�g��1,2,3)	*/
						if( dat1 == KAKUCHOU_1 )					/*								*/
						{											/*								*/
							ts->Mno_use_cnt4[dat1][dat2] +=			/* �X�����������񐔁�(��{,�g��1)(�X��1�`100)	*/
												wari_dt.maisuu;		/*								*/
							ts->Mno_use_ryo4[dat1][dat2] +=			/* �X�������������z��(��{,�g��1)(�X��1�`100)	*/
												wari_dt.ryokin;		/*								*/
						}											/*								*/
					}												/*								*/
					else											/* �X��101�`999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* �X��101�`999İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* �X��101�`999İ�ي��������z��(��{,�g��1,2,3)	*/
					}												/*								*/
				}													/*								*/
																	/*								*/
				if( CPrmSS[S_STO][1+3*dat2] == 4 &&					/* �X�����S�z�����ݒ�?			*/
					ryo_buf.pkiti != 0xffff )						/* ������Z�ȊO?					*/
				{													/*								*/
					syusei[itis].infofg |= SSS_ZENWARI;				/* �C���p�S�z�����׸޾��		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* �X����������z/���|���z���Z�ݒ�	*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				break;												/*								*/
			case KAISUU:											/* �񐔌�						*/
				if( CPrmSS[S_PRP][1] != 2 )							/* �񐔌����g�p����ݒ�ł͂Ȃ�	*/
					break;

				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
																	/*								*/
				ts->Ktik_use_cnt[dat1] += wari_dt.maisuu;			/* �g�p��������(��{,�g��1,2,3) */
				ts->Ktik_use_ryo[dat1] += wari_dt.ryokin;			/* �g�p�����z��(��{,�g��1,2,3)	*/

				dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );				/* �񐔌�������z/���|���z���Z�ݒ�	*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				break;												/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
																	/*								*/
	if( ryo_buf.ryo_flg >= 2 )										/* ����g�p?					*/
	{																/*								*/
		dat1 = (ulong)PayData.teiki.pkno_syu;						/* ���ԏꇂ�̎��				*/
		ts->Teiki_use_cnt[dat1][PayData.teiki.syu-1] += 1L;			/* ����g�p��+1				*/
		syusei[itis].tei_syu = PayData.teiki.syu;					/* ������						*/
		syusei[itis].tei_id = PayData.teiki.id;						/* �����id(1�`12000)			*/
		syusei[itis].tei_sd = dnrmlzm( (short)PayData.teiki.s_year,	/* �L���J�n�N����ɰ�ײ��		*/
									   (short)PayData.teiki.s_mon,	/*								*/
									   (short)PayData.teiki.s_day );/*								*/
		syusei[itis].tei_ed = dnrmlzm( (short)PayData.teiki.e_year,	/* �L���I���N����ɰ�ײ��		*/
									   (short)PayData.teiki.e_mon,	/*								*/
									   (short)PayData.teiki.e_day );/*								*/

		if(	( 1 == (uchar)prm_get(COM_PRM, S_BUN, 54, 1, 3) )		/* ��������Z���ɕ��ޏW�v�ւ̉��Z���� */
				&&													/*   ����						*/
			( DoBunruiSyu(ryo_buf.syubet) ) ){						/* ���ޏW�v����i��ʖ������j	*/

			bunrui( 0, ryo_buf.tei_ryo , ryo_buf.syubet );			/* ���ޏW�v���Z					*/
		}
	}																/*								*/
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 75;											/* 75							*/
																	/*								*/
}																	/*								*/
#endif
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���Z�r��(���Z�����O)��d���A�W�v															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: toty_syu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-11-15																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	toty_syu( void )											/* ���Z�r��(���Z�����O)��d���A	*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	ulong	dat1, dat2;												/*								*/
	ushort	iti;													/*								*/
	short	i;														/*								*/
	uchar	f_SameData;												/* 1=������~ð��ٓ��ɖړI�ް����� */
	uchar	uc_update = PayData.teiki.update_mon;					/* �X�V���i�[					*/
	ushort	Flp_sub_no;
	uchar	Flp_sub_ok;
	uchar	wk_cnt=0;
	uchar	uc_buf_size;
	ulong	wk_Electron_ryo=0;										/* �d�q�Ȱ���Z���z�Z�[�u		*/
	wari_tiket	wari_dt;
	short	j;
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	DATE_SYOUKEI	*date_ts;
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
	uchar	taxableAdd_set;											/*	�ېőΏ۔�����Z�ݒ�		*/
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH321800(S) hosoda IC�N���W�b�g�Ή�
//	struct clk_rec wrk_clk_rec;		/* �W�v�����m�[�}���C�Y�p */
//	date_time_rec wrk_date;			/* �W�v�����m�[�}���C�Y�p */
//	int		ec_kind;
//	int		syu_idx;				/* �����W�v�̃C���f�b�N�X */
//	ulong	sei_date;				/* ���Z���� */
//	ulong	sy1_date;				/* �W�v���� */
//	memset(&wrk_clk_rec, 0x0, sizeof(wrk_clk_rec));
//// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810105 GG119202(E) ������������W�v�d�l���P

	dat1 = 0;
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	date_ts = &Date_Syoukei;
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	iti = ryo_buf.pkiti - 1;										/* ���Ԉʒu						*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* �s�W�v��ܰ��ر�ցi��d�΍�j	*/
	DailyAggregateDataBKorRES( 0 );									/* �����W�v�G���A�̃o�b�N�A�b�v	*/
// MH322914 (s) kasiyama 2016/07/13 ���d�̎��؂̕��ߊz�C��[���ʃo�ONo.1232](MH341106)
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	date_uriage_syoukei_judge();									// ���t�ؑ֊ ������A���������㏬�v�̎����X�V����i���A���^�C�����p�j
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	if( ryo_buf.nyukin != 0L )										/* �����L��H					*/
	{																/*								*/
		ryo_buf.fusoku = ryo_buf.nyukin;							/* �����z��s���z				*/
		ryo_buf.turisen = ryo_buf.nyukin;							/* �����z��ނ�K				*/
	}																/*								*/
// MH322914 (e) kasiyama 2016/07/13 ���d�̎��؂̕��ߊz�C��[���ʃo�ONo.1232](MH341106)
																	/*								*/
	PayData_set( 1, 1 );											/* 1���Z����					*/
																	/*								*/
	ac_flg.cycl_fg = 51;											/* 51:�s�W�v��ܰ��ر�֓]������	*/
																	/*								*/
																	/*								*/
	if( ryo_buf.nyukin != 0L )										/* �����L��H					*/
	{																/*								*/
		ts->Harai_husoku_cnt += 1L;									/* �x�����s����				*/
		ts->Harai_husoku_ryo += ryo_buf.nyukin;						/* �x�����s���z�ɓ����z���Z		*/
																	/*								*/
// MH322914 (s) kasiyama 2016/07/13 ���d�̎��؂̕��ߊz�C��[���ʃo�ONo.1232](MH341106)
//		ryo_buf.fusoku = ryo_buf.nyukin;							/* �����z��s���z				*/
//		ryo_buf.turisen = ryo_buf.nyukin;							/* �����z��ނ�K				*/
// MH322914 (e) kasiyama 2016/07/13 ���d�̎��؂̕��ߊz�C��[���ʃo�ONo.1232](MH341106)
	}																/*								*/
																	/*								*/
	ts->Seisan_chusi_cnt += 1L;										/* ���Z���~��					*/
	ts->Seisan_chusi_ryo += ryo_buf.nyukin;							/* ���~�����������z				*/
																	/*								*/
	if(Flap_Sub_Flg == 1){									// �Ő��Z�{B�G���A�L
		Flp_sub_ok = 1;
		Flp_sub_no = Flap_Sub_Num;							// ��ƒ��ڍ״ر�i�[
	}else if((Flap_Sub_Flg == 2) || (Flap_Sub_Num == 10)){	// �Đ��Z�{B�G���A���A�}���`���Z(B�ر��)�̎�
		Flp_sub_ok = 0;
	}else{														// �Đ��Z���A�����Z���~
		for(i = 0 ; i < 10 ; i++){
			if(FLAPDT_SUB[i].WPlace == 0){
				break;
			}
		}
		if(i < 10){
			Flp_sub_ok = 1;
			Flp_sub_no = i;
		}else{
			memmove(&FLAPDT_SUB[0],&FLAPDT_SUB[1],sizeof(flp_com_sub)*9);			//�O�ɂP�V�t�g
			memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));	//�ر
			Flp_sub_ok = 1;
			Flp_sub_no = 9;
		}
	}

	wk_media_Type = Ope_Disp_Media_Getsub(1);						/* �d�q�Ȱ�}�̎�� �擾			*/
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH321800(S) hosoda IC�N���W�b�g�Ή�
//// ���d�q�}�l�[�g�p���͒��~���Ȃ��̂ŁA���̏������������Ƃ͂Ȃ��͂��ł��B�i���d�ł������ɂȂ�͂��B�j
//// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
////	if (EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) != 0 ||
////		PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED ) {
//	if ((EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) != 0 ||
//		 PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) &&
//		(PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0)) {	// �݂Ȃ����ς̕��d���͏W�v���Ȃ�(���Z���~�����̂���)
//// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��
//		// ���σ��[�_���g�p����Ă�����
//		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );							/* �d�q�}�l�[������z/���|���z���Z�ݒ�	*/
//		wk_Electron_ryo = PayData.Electron_data.Ec.pay_ryo;					/* �d�q�}�l�[���Z���z�Z�[�u				*/
//		ec_kind = PayData.Electron_data.Ec.e_pay_kind;
//		// �W�v�͊����G���A������΂�����g�p����B
//		switch(ec_kind) {
//		case EC_EDY_USED:
//			ts->Electron_edy_cnt += 1L;										// ���Z��������
//			ts->Electron_edy_ryo += wk_Electron_ryo;						//     �����z��
//			break;
//		case EC_NANACO_USED:
//			ts->nanaco_sei_cnt += 1L;										// ���Z��������
//			ts->nanaco_sei_ryo += wk_Electron_ryo;							//     �����z��
//			break;
//		case EC_WAON_USED:
//			ts->waon_sei_cnt += 1L;											// ���Z��������
//			ts->waon_sei_ryo += wk_Electron_ryo;							//     �����z��
//			break;
//		case EC_SAPICA_USED:
//			ts->sapica_sei_cnt += 1L;										// ���Z��������
//			ts->sapica_sei_ryo += wk_Electron_ryo;							//     �����z��
//			break;
//		case EC_KOUTSUU_USED:
//			ts->koutsuu_sei_cnt += 1L;										// ���Z��������
//			ts->koutsuu_sei_ryo += wk_Electron_ryo;							//     �����z��
//			break;
//		case EC_ID_USED:
//			ts->id_sei_cnt += 1L;											// ���Z��������
//			ts->id_sei_ryo += wk_Electron_ryo;								//     �����z��
//			break;
//		case EC_QUIC_PAY_USED:
//			ts->quicpay_sei_cnt += 1L;										// ���Z��������
//			ts->quicpay_sei_ryo += wk_Electron_ryo;							//     �����z��
//			break;
//		case EC_CREDIT_USED:
//			// �N���W�b�g��T�W�v�͊��������ɂď���
//			wk_Electron_ryo = PayData.credit.pay_ryo;
//			break;
//		default:
//			break;		// ���肦�Ȃ�
//		}
//		// �ŐV�̒ʒm��������W�v�������Z�o
//		memcpy(&wrk_date, &Syuukei_sp.ec_inf.NowTime, sizeof(date_time_rec));
//		wrk_date.Hour = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 3);			/* ���͋��ʃp�����[�^����擾 */
//		wrk_date.Min = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 1);				/* ���͋��ʃp�����[�^����擾 */
//		sy1_date = Nrm_YMDHM(&wrk_date);									/* �W�v�������m�[�}���C�Y */
//
//		// ���Z�f�[�^����i���ό��ʃf�[�^�Ŏ�M�����j���Z�������Z�o
//		c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &wrk_clk_rec);
//		memcpy(&wrk_date, &wrk_clk_rec, sizeof(date_time_rec));
//		sei_date = Nrm_YMDHM(&wrk_date);									/* ���Z�������m�[�}���C�Y */
//
//		ec_kind -= EC_EDY_USED;
//		if(sei_date >= sy1_date && sei_date < (sy1_date + 0x10000)) {
//			// ���Z�����������̏W�v�����͈�
//			Syuukei_sp.ec_inf.now.cnt[ec_kind] += 1L;							/* �g�p��				*/
//			Syuukei_sp.ec_inf.now.ryo[ec_kind] += wk_Electron_ryo;				/* �g�p���z				*/
//		}
//		else if(sei_date >= (sy1_date + 0x10000) && sei_date < (sy1_date + 0x20000)) {
//			// ���Z�����������̏W�v�����͈�
//			Syuukei_sp.ec_inf.next.cnt[ec_kind] += 1L;							/* �g�p��				*/
//			Syuukei_sp.ec_inf.next.ryo[ec_kind] += wk_Electron_ryo;				/* �g�p���z				*/
//		}
//		else if(sei_date >= (sy1_date - 0x10000) && sei_date < sy1_date) {
//			// ���Z�������O���̏W�v�����͈�
//			if(Syuukei_sp.ec_inf.cnt == 1) {
//				// �O���̏W�v���Ȃ����ߍ쐬����
//				UnNrm_YMDHM(&wrk_date, (sy1_date - 0x10000));
//				memcpy(&Syuukei_sp.ec_inf.bun[Syuukei_sp.ec_inf.ptr].SyuTime, &wrk_date, sizeof(date_time_rec));
//				Syuukei_sp.ec_inf.ptr++;
//				Syuukei_sp.ec_inf.cnt++;
//			}
//			syu_idx = (Syuukei_sp.ec_inf.ptr == 0) ? (SYUUKEI_DAY_EC-1) : (Syuukei_sp.ec_inf.ptr-1);
//			Syuukei_sp.ec_inf.bun[syu_idx].cnt[ec_kind] += 1L;					/* �g�p��				*/
//			Syuukei_sp.ec_inf.bun[syu_idx].ryo[ec_kind] += wk_Electron_ryo;		/* �g�p���z				*/
//		} else {
//			// ���Z��������������O���͈̔͊O�̏ꍇ�̓A���[����o�^���ē����ɉ��Z
//			Syuukei_sp.ec_inf.now.cnt[ec_kind] += 1L;							/* �g�p��				*/
//			Syuukei_sp.ec_inf.now.ryo[ec_kind] += wk_Electron_ryo;				/* �g�p���z				*/
//			Syuukei_sp.ec_inf.now.unknownTimeCnt++;
//
//			// �A���[���o�^(���σ��[�_�̌��ώ����Ɛ��Z�@�̎�����1���ȏ�̂��ꂪ����)
//			alm_chk( ALMMDL_SUB, ALARM_EC_SETTLEMENT_TIME_GAP, 2 );
//		}
//	}
//	else
//// MH321800(E) hosoda IC�N���W�b�g�Ή�
// MH810105 GG119202(E) ������������W�v�d�l���P
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ){
		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 2 );					/* Suica������z/���|���z���Z�ݒ�	*/
		wk_Electron_ryo = PayData.Electron_data.Suica.pay_ryo;		/* �d�q�Ȱ���Z���z�Z�[�u		*/
		Syuukei_sp.sca_inf.now.cnt += 1L;							/* Suica�g�p��				*/
		Syuukei_sp.sca_inf.now.ryo += wk_Electron_ryo;				/* Suica�g�p���z				*/
		if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {			/* �r��������					*/
			ts->Electron_sei_cnt += 1L;								/* Suica�g�p���񐔁�			*/
			ts->Electron_sei_ryo += wk_Electron_ryo;				/* Suica�g�p�����z��			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		/* �o�`�r�l�n					*/
			ts->Electron_psm_cnt += 1L;								/* PASMO�g�p���񐔁�			*/
			ts->Electron_psm_ryo += wk_Electron_ryo;				/* PASMO�g�p�����z��			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICOCA) {		/* �h�b�n�b�`					*/
			ts->Electron_ico_cnt += 1L;								/* ICOCA�g�p���񐔁�			*/
			ts->Electron_ico_ryo += wk_Electron_ryo;				/* ICOCA�g�p�����z��			*/
		} else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICCARD) {	/* �h�b-�b�`�q�c				*/
			ts->Electron_icd_cnt += 1L;								/* IC-CARD�g�p���񐔁�			*/
			ts->Electron_icd_ryo += wk_Electron_ryo;				/* IC-CARD�g�p�����z��			*/
		}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//	} else if( PayData.Electron_data.Suica.e_pay_kind == EDY_USED ){/* Edy��ް�̎��ް�����			*/
//		dat1 = prm_get( COM_PRM, S_TOT, 6, 1, 3 );					/* Edy������z/���|���z���Z�ݒ�	*/
//		if( EDY_TEST_MODE ){										// ý�Ӱ�ނ̏ꍇ
//			dat1 = 0L;												// ������z/���|���z�ɉ��Z���Ȃ�
//		}
//		else{
//			wk_Electron_ryo = PayData.Electron_data.Edy.pay_ryo;	/* �d�q�Ȱ���Z���z�Z�[�u		*/
//			if(wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY) {			/* �d����						*/
//				ts->Electron_edy_cnt += 1L;							/* �d�����g�p���񐔁�			*/
//				ts->Electron_edy_ryo += wk_Electron_ryo;			/* �d�����g�p�����z��			*/
//				Syuukei_sp.edy_inf.now.cnt += 1L;					/* PASMO�g�p��				*/
//				Syuukei_sp.edy_inf.now.ryo += wk_Electron_ryo;		/* PASMO�g�p���z				*/
//			}
//		}
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	}
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	else if (EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
			PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) {
		// ����z�ɉ��Z���Ȃ�
		wk_Electron_ryo = 0;

		if (PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0) {
			// ���ϒ��ɏ�Q�����A�܂��́A�����c���Ɖ�^�C���A�E�g����
			// ������������i�x���s���j�W�v
			ts->miryo_unknown_cnt += 1L;
			ts->miryo_unknown_ryo += EcAlarm.Ec_Res.settlement_data;

			// �����W�v�͓����Ƃ��ăJ�E���g����
			Syuukei_sp.ec_inf.now.sp_miryo_unknown_cnt += 1L;
			Syuukei_sp.ec_inf.now.sp_miryo_unknown_ryo += EcAlarm.Ec_Res.settlement_data;
		}
	}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
////	if( PayData.Electron_data.Suica.e_pay_kind != 0 ) {				/* �d�q�Ȱ(Suica/Edy)�̎��ް�����*/
//	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ||	// SX-20�ɂ��d�q�}�l�[����
//// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
////		EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {			// ���σ��[�_�ɂ��d�q�}�l�[����
//		EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) &&			// ���σ��[�_�ɂ��d�q�}�l�[����
//		PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ){	// �݂Ȃ����ς̕��d���͏W�v���Ȃ�(���Z���~�����̂���)
//// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��
//// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind ) ){	// SX-20�ɂ��d�q�}�l�[����
// MH810105 GG119202(E) ������������W�v�d�l���P
		if( dat1 == 1L ) {											/* ������z�ɉ��Z����			*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
			if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
				if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
					date_ts->Uri_Tryo += wk_Electron_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
		} else if( dat1 == 2L ) {									/* ���|���z�ɉ��Z����			*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* ���|���z						*/
		} else if( dat1 == 3L ) {									/* ������z/���|���z�ɉ��Z����	*/
			ts->Uri_Tryo += wk_Electron_ryo;						/* ������z						*/
			ts->Kakeuri_Tryo += wk_Electron_ryo;					/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
			if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
				if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
					date_ts->Uri_Tryo += wk_Electron_ryo;
				}
			}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
		}															/*								*/
	}

	if( CPrmSS[S_PRP][1] == 1 )										/* ����߲�޶��ނ��g�p����ݒ�	*/
	{																/*								*/
		dat1 = prm_get( COM_PRM,S_TOT,5,1,3 );						/* ����߲�޶��ޑ�����z/���|���z���Z�ݒ�	*/
		j = 0;														/*								*/
		for( i = 0; i < WTIK_USEMAX; i++ ){							/*								*/
			disc_wari_conv( &PayData.DiscountData[i], &wari_dt );	/*								*/
			if( wari_dt.tik_syu == PREPAID ){						/* �ް�����						*/
				dat2 = wari_dt.pkno_syu;							/*								*/
				ts->Pcrd_use_cnt[dat2] += ryo_buf.pri_mai[j];		/* ����߲�޶��ގg�p���񐔁�(��{,�g��1,2,3)	*/
																	/* 7���ȏ�g�p���͍Ō�Ɏg��ꂽ��ʂɍ��v���Z����	*/
				ts->Pcrd_use_ryo[dat2] += wari_dt.ryokin;			/* ����߲�޶��ގg�p�����z��(��{,�g��1,2,3)	*/
																	/*								*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				if(Flp_sub_ok){										/* �ڍג��~�G���A�g�p			*/
					FLAPDT_SUB[Flp_sub_no].ppc_chusi_ryo += wari_dt.ryokin;/* �ڍג��~�G���A�ɉ��Z���� */
				}													/*								*/
				j++;												/*								*/
			}														/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	for( i = 0; i < WTIK_USEMAX; i++ )								/*								*/
	{																/*								*/
		disc_wari_conv( &PayData.DiscountData[i], &wari_dt );		/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
		taxableAdd_set = 0xff;
		if( cancelReceipt_Waridata_chk( &wari_dt ) ){				/*			�ېőΏ�			*/
			taxableAdd_set = (uchar)prm_get( COM_PRM, S_TOT, 7, 1, 2 );	/*�ېőΏۗ��p�z�̔�����Z�ݒ�*/
		}	
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
		switch( wari_dt.tik_syu )									/*								*/
		{															/*								*/
			case SERVICE:											/* ���޽��						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* ���޽��A,B,C					*/
																	/*								*/
				ts->Stik_use_cnt[dat1][dat2] += wari_dt.maisuu;		/* ���޽����ʖ��g�p��������(��{,�g��1,2,3)(���޽��A,B,C)	*/
				ts->Stik_use_ryo[dat1][dat2] += wari_dt.ryokin;		/* ���޽����ʖ��g�p�����z��(��{,�g��1,2,3)(���޽��A,B,C)	*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//				if( CPrmSS[S_SER][1+3*dat2] == 4 &&					/* ���޽���S�z�����ݒ�?			*/
				if( prm_get( COM_PRM, S_SER, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* ���޽���S�z�����ݒ�? */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
					ryo_buf.pkiti != 0xffff &&						/* ������Z�ȊO?					*/
					uc_update == 0 )								/* ������X�V���Z�ȊO			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* �C���p�S�z�����׸޾��		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,1 );				/* ���޽��������z/���|���z���Z�ݒ�	*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				setting_taxableAdd( &dat1, taxableAdd_set );		/* �ېőΏۗ��p�z�̔�����Z�ݒ� */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				if(Flp_sub_ok){
					FLAPDT_SUB[Flp_sub_no].sev_tik[dat2] += wari_dt.maisuu;	// �ڍג��~�G���A�ɉ��Z����
				}
				break;												/*								*/
																	/*								*/
			case KAKEURI:											/* �|����						*/
				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
				dat2 = (ulong)( wari_dt.syubetu - 1 );				/* �X��1�`999(0�`998)			*/
																	/*								*/
				ts->Mno_use_Tcnt += wari_dt.maisuu;					/* �X������İ�ف��񐔁�			*/
				ts->Mno_use_Tryo += wari_dt.ryokin;					/* �X������İ�ف����z��			*/
																	/*								*/
				ts->Mno_use_cnt1[dat1] += wari_dt.maisuu;			/* �X������İ�ف��񐔁�(��{,�g��1,2,3)	*/
				ts->Mno_use_ryo1[dat1] += wari_dt.ryokin;			/* �X������İ�ف����z��(��{,�g��1,2,3)	*/
																	/*								*/
				if( dat1 == KIHON_PKNO || dat1 == KAKUCHOU_1 )		/* ��{ or �g��1				*/
				{													/*								*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
//					if( dat2 < 100 )								/* �X��1�`100					*/
					if( dat2 < MISE_NO_CNT )						/* �X��1�`100					*/
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* �X��1�`100İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* �X��1�`100İ�ي��������z��(��{,�g��1,2,3)	*/
						ts->Mno_use_cnt4[dat1][dat2] +=				/* �X�����������񐔁�(��{,�g��1)(�X��1�`100)	*/
												wari_dt.maisuu;		/*								*/
						ts->Mno_use_ryo4[dat1][dat2] +=				/* �X�������������z��(��{,�g��1)(�X��1�`100)	*/
												wari_dt.ryokin;		/*								*/
					}												/*								*/
					else											/* �X��101�`999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* �X��101�`999İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* �X��101�`999İ�ي��������z��(��{,�g��1,2,3)	*/
					}												/*								*/
				}													/*								*/
				else												/*								*/
				{													/*								*/
					if(( dat1 == KAKUCHOU_2 )&&						/* �g��2						*/
					   ( prm_get( COM_PRM,S_TOT,16,1,3 ) == 1 ))	/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
					else if(( dat1 == KAKUCHOU_3 )&&				/* �g��3						*/
							( prm_get( COM_PRM,S_TOT,16,1,4 ) == 1 ))/*								*/
					{												/*								*/
						dat1 = KAKUCHOU_1;							/*								*/
					}												/*								*/
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
//					if( dat2 < 100 )								/* �X��1�`100					*/
					if( dat2 < MISE_NO_CNT )						/* �X��1�`100					*/
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
					{												/*								*/
						ts->Mno_use_cnt2[dat1] += wari_dt.maisuu;	/* �X��1�`100İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo2[dat1] += wari_dt.ryokin;	/* �X��1�`100İ�ي��������z��(��{,�g��1,2,3)	*/
						if( dat1 == KAKUCHOU_1 )					/*								*/
						{											/*								*/
							ts->Mno_use_cnt4[dat1][dat2] +=			/* �X�����������񐔁�(��{,�g��1)(�X��1�`100)	*/
												wari_dt.maisuu;		/*								*/
							ts->Mno_use_ryo4[dat1][dat2] +=			/* �X�������������z��(��{,�g��1)(�X��1�`100)	*/
												wari_dt.ryokin;		/*								*/
						}											/*								*/
					}												/*								*/
					else											/* �X��101�`999					*/
					{												/*								*/
						ts->Mno_use_cnt3[dat1] += wari_dt.maisuu;	/* �X��101�`999İ�ي������񐔁�(��{,�g��1,2,3)	*/
						ts->Mno_use_ryo3[dat1] += wari_dt.ryokin;	/* �X��101�`999İ�ي��������z��(��{,�g��1,2,3)	*/
					}												/*								*/
				}													/*								*/
																	/*								*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//				if( CPrmSS[S_STO][1+3*dat2] == 4 &&					/* �X�����S�z�����ݒ�?			*/
				if( prm_get( COM_PRM, S_STO, (short)(1+3*dat2), 1, 1 ) == 4 &&	/* �X�����S�z�����ݒ�? */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
					ryo_buf.pkiti != 0xffff &&						/* ������Z�ȊO?					*/
					uc_update == 0 )								/* ������X�V���Z�ȊO			*/
				{													/*								*/
					syusei[iti].infofg |= SSS_ZENWARI;				/* �C���p�S�z�����׸޾��		*/
				}													/*								*/
																	/*								*/
				dat1 = prm_get( COM_PRM,S_TOT,5,1,2 );				/* �X����������z/���|���z���Z�ݒ�	*/
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				setting_taxableAdd( &dat1, taxableAdd_set );		/* �ېőΏۗ��p�z�̔�����Z�ݒ� */
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/�ېőΏۃT�[�r�X���^�X�������p�z�̔�����Z�ݒ�Ƀp�^�[����ǉ�����j
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				if(Flp_sub_ok){
					for(wk_cnt=0;wk_cnt<5;wk_cnt++){				//�����XNo����
						if(FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no == wari_dt.syubetu){
							FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].maisuu += wari_dt.maisuu;//�����XNo�̎��A�������i�[����
							break;
						}
					}
					if(wk_cnt == 5){								//�����XNo��
						for(wk_cnt=0;wk_cnt<5;wk_cnt++){
							if(FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no == 0){
								FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].mise_no = wari_dt.syubetu;//�X��No
								FLAPDT_SUB[Flp_sub_no].kake_data[wk_cnt].maisuu += wari_dt.maisuu;//����
								break;
							}
						}
					}
					if(wk_cnt == 5){								//�����XNo���{�ڍג��~�󂫴ر��

						if(Flp_sub_no < 9){
							uc_buf_size = (uchar)(9 - Flp_sub_no);
							memset(&FLAPDT_SUB[Flp_sub_no],0,sizeof(flp_com_sub));	// �ڍג��~�ر(���Z����)�ر
							memmove(&FLAPDT_SUB[Flp_sub_no],&FLAPDT_SUB[Flp_sub_no+1],sizeof(flp_com_sub)*uc_buf_size);
						}
						if(Flp_sub_no == 10){
							memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));	// �I�[�ڍג��~�ر�ر
						}else{
							memset(&FLAPDT_SUB[9],0,sizeof(flp_com_sub));// �I�[�ڍג��~�ر�ر
						}
						Flp_sub_ok = 0;								//�i�[��׸޸ر
					}
				}
				break;												/*								*/
			case KAISUU:											/* �񐔌�						*/
				if( CPrmSS[S_PRP][1] != 2 )							/* �񐔌����g�p����ݒ�ł͂Ȃ�	*/
					break;

				dat1 = (ulong)wari_dt.pkno_syu;						/* ���ԏꇂ�̎��				*/
																	/*								*/
				ts->Ktik_use_cnt[dat1] += wari_dt.maisuu;			/* �g�p��������(��{,�g��1,2,3) */
				ts->Ktik_use_ryo[dat1] += wari_dt.ryokin;			/* �g�p�����z��(��{,�g��1,2,3)	*/

				dat1 = prm_get( COM_PRM,S_TOT,5,1,4 );				/* �񐔌�������z/���|���z���Z�ݒ�	*/
				if( dat1 == 1L )									/* ������z�ɉ��Z����			*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				else if( dat1 == 2L )								/* ���|���z�ɉ��Z����			*/
				{													/*								*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
				}													/*								*/
				else if( dat1 == 3L )								/* ������z/���|���z�ɉ��Z����	*/
				{													/*								*/
					ts->Uri_Tryo += wari_dt.ryokin;					/* ������z						*/
					ts->Kakeuri_Tryo += wari_dt.ryokin;				/* ���|���z						*/
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
					if(date_uriage_use_chk() == 0){// ���t�ؑ֊�̑�����̏��v�擾����i���A���^�C�����p�j
						if(date_uriage_cmp_paymentdate() == 0){// ���Z�\
							date_ts->Uri_Tryo += wari_dt.ryokin;
						}
					}
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
				}													/*								*/
				if(Flp_sub_ok){										//�ڍג��~�G���A�g�p
					FLAPDT_SUB[Flp_sub_no].ppc_chusi_ryo += wari_dt.ryokin;	// �ڍג��~�G���A�ɉ��Z����B
				}
				break;												/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
																	/*								*/
	ts->tou[0] += (ulong)ryo_buf.in_coin[0];						/* ���Z�������z  10�~�g�p����	*/
	ts->tou[1] += (ulong)ryo_buf.in_coin[1];						/* ���Z�������z  50�~�g�p����	*/
	ts->tou[2] += (ulong)ryo_buf.in_coin[2];						/* ���Z�������z 100�~�g�p����	*/
	ts->tou[3] += (ulong)ryo_buf.in_coin[3];						/* ���Z�������z 500�~�g�p����	*/
	ts->tou[4] += (ulong)ryo_buf.in_coin[4];						/* ���Z�������z1000�~�g�p����	*/
	ts->tounyu = ts->tou[0]*10 + ts->tou[1]*50 + ts->tou[2]*100 +	/*								*/
				 ts->tou[3]*500 + ts->tou[4]*1000;					/* ���Z�������z	���z			*/
																	/*								*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[0];						/* ���Z���o���z  10�~�g�p����	*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[1];						/* ���Z���o���z  50�~�g�p����	*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[2];						/* ���Z���o���z 100�~�g�p����	*/
	ts->sei[3] += (ulong)ryo_buf.out_coin[3];						/* ���Z���o���z 500�~�g�p����	*/
	ts->sei[0] += (ulong)ryo_buf.out_coin[4];						/* ���Z���o���z  10�~�g�p����(�\�~)*/
	ts->sei[1] += (ulong)ryo_buf.out_coin[5];						/* ���Z���o���z 50�~�g�p����(�\�~)*/
	ts->sei[2] += (ulong)ryo_buf.out_coin[6];						/* ���Z���o���z 100�~�g�p����(�\�~)*/
	ts->seisan = ts->sei[0]*10 + ts->sei[1]*50 + ts->sei[2]*100 +	/*								*/
				 ts->sei[3]*500;									/* ���Z���o���z	���z			*/
																	/*								*/
	if(( card_use[USE_SVC] || card_use[USE_PPC] || card_use[USE_NUM] )&&	/* �����L��?					*/
	   ryo_buf.pkiti != 0xffff &&									/* ������Z�ȊO?					*/
	   uc_update == 0 )												/* ������X�V���Z�ȊO			*/
	{																/*								*/
		FLAPDT.flp_data[iti].bk_syu = (ushort)(ryo_buf.syubet + 1);	/* ���(���~,�C���p)			*/
		FLAPDT.flp_data[iti].bk_wmai = card_use[USE_SVC];			/* �g�p����(���~,�C���p)		*/
		FLAPDT.flp_data[iti].bk_wari = ryo_buf.fee_amount;			/* �������z(���~,�C���p)		*/
		FLAPDT.flp_data[iti].bk_time = ryo_buf.tim_amount;			/* �������Ԑ�(���~,�C���p)		*/
		if(Flp_sub_ok){
			FLAPDT_SUB[Flp_sub_no].syu = (uchar)(ryo_buf.syubet + 1);		// ���
			FLAPDT_SUB[Flp_sub_no].TInTime = PayData.TInTime;				// ���Ɏ���
			FLAPDT_SUB[Flp_sub_no].WPlace = PayData.WPlace;			// �Ԏ��ԍ����i�[����
		}
	}																/*								*/
																	/*								*/
																	/*								*/
	ac_flg.cycl_fg = 52;											/* 52:���Z�r����d���A�W�v����	*/
																	/*								*/
	if( ryo_buf.ryo_flg >= 2 )										/* ����g�p?					*/
	{																/*								*/
		memcpy( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* ���~����ް��ޔ�				*/
																	/*								*/
// MH810105(S) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
//		Log_regist( LOG_PAYSTOP );									/* ���Z���~���o�^				*/
		Log_regist( LOG_PAYSTOP_FU );								/* ���Z���~���o�^				*/
// MH810105(E) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime���Z���~�f�[�^���M����
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
				Set_Cancel_RTPay_Data();
				// ر���ѐ��Z�ް��̾����ǔԂ��X�V����log�ɏ������ޏ���
				RTPay_LogRegist_AddOiban();
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			}
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			// QR�m��E����f�[�^�o�^
			ope_SendCertifCancel();
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
		Suica_Rec.Data.BIT.LOG_DATA_SET = 0;						/* Suica���Z�׸޸ر				*/
																	/*								*/
		f_SameData = 0;												/*								*/
																	/*								*/
		if( TKI_CYUSI_MAX < tki_cyusi.count )						/* fail safe					*/
			tki_cyusi.count = TKI_CYUSI_MAX;						/*								*/
																	/*								*/
		if( TKI_CYUSI_MAX <= tki_cyusi.wtp )						/* fail safe					*/
			tki_cyusi.wtp = tki_cyusi.count - 1;					/*								*/
																	/*								*/
		for( i=0; i<tki_cyusi.count; i++ )							/*								*/
		{															/*								*/
			if( 0L == tki_cyusi.dt[i].pk ){							/* ð��ق��ް��Ȃ�				*/
				break;												/* i=�o�^����					*/
			}														/*								*/
			if( tki_cyusi.dt[i].no == PayData.teiki.id &&			/* �Ē��~?						*/
				tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*					*/
			{														/*								*/
				f_SameData = 1;										/*								*/
				break;												/*								*/
			}														/*								*/
		}															/*								*/
		if( 0 == f_SameData )										/* �����ް��Ȃ�(�V�K�o�^�K�v)	*/
		{															/*								*/
			if( i == TKI_CYUSI_MAX ){								/* �o�^����Full					*/
				NTNET_Snd_Data219(1, &tki_cyusi.dt[0]);				/* 1���폜�ʒm���M				*/
				TKI_Delete(0);										/* �Ō��ް��폜					*/
				nmisave( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* ���~����ް��ޔ�		*/
			}														/*								*/
			tki_cyusi.dt[tki_cyusi.wtp].syubetu = PayData.syu;		/* �������(A�`L:1�`12)			*/
			tki_cyusi.dt[tki_cyusi.wtp].pk =						/*								*/
						CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];	/* ���ԏꇂ						*/
			tki_cyusi.dt[tki_cyusi.wtp].no = PayData.teiki.id;		/* �l����(1�`12000)			*/
			tki_cyusi.dt[tki_cyusi.wtp].tksy = PayData.teiki.syu;	/* ������(1�`15)				*/
			tki_cyusi.dt[tki_cyusi.wtp].year = PayData.TInTime.Year;/* �����N						*/
			memcpy( &tki_cyusi.dt[tki_cyusi.wtp].mon, PayData.teiki.t_tim, 4 );	/* ������������		*/
			tki_cyusi.dt[tki_cyusi.wtp].sec = 0;					/* �����b						*/
																	/*								*/
			NTNET_Snd_Data219(0, &tki_cyusi.dt[tki_cyusi.wtp]);		/* 1���X�V�ʒm���M				*/
																	/*								*/
			if( tki_cyusi.count < TKI_CYUSI_MAX )					/*								*/
			{														/*								*/
				tki_cyusi.count++;									/* �o�^������+1					*/
			}														/*								*/
																	/*								*/
			tki_cyusi.wtp++;										/* ײ��߲��+1					*/
			if( tki_cyusi.wtp >= TKI_CYUSI_MAX )					/*								*/
			{														/*								*/
				tki_cyusi.wtp = 0;									/*								*/
			}														/*								*/
		}															/*								*/
		WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* �����ð��ٍX�V(�o��)	*/
					  (ushort)PayData.teiki.id,						/*								*/
					  1,											/*								*/
					  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*							*/
					  0xffff );										/*								*/
	}																/*								*/
	else{															/*								*/
// MH810105(S) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
//		Log_regist( LOG_PAYSTOP );									/* ���Z���~���o�^				*/
		Log_regist( LOG_PAYSTOP_FU );								/* ���Z���~���o�^				*/
// MH810105(E) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime���Z���~�f�[�^���M����
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
				Set_Cancel_RTPay_Data();
				// ر���ѐ��Z�ް��̾����ǔԂ��X�V����log�ɏ������ޏ���
				RTPay_LogRegist_AddOiban();
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			}
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			// QR�m��E����f�[�^�o�^
			ope_SendCertifCancel();
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
		Suica_Rec.Data.BIT.LOG_DATA_SET = 0;					/* Suica���Z�׸޸ر				*/
	}																/*								*/
																	/*								*/
	CountUp( CANCEL_COUNT );										/* ���Z���~�ǔ�+1				*/
	if( ryo_buf.fusoku != 0 ){										/* �a��ؔ��s?					*/
		CountUp(DEPOSIT_COUNT);										/* �a��ؒǔ�+1					*/
	}
// MH810105(S) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
	}
// MH810105(E) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
// MH810100(S) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
	if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
		DC_PopCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔԂ�Pop
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
	}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) K.Onodera  2020/01/22 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	if ((EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
		 PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) &&
		(PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0)) {
		// ���ϒ��ɏ�Q�����A�܂��́A�����c���Ɖ�^�C���A�E�g�������߁A
		// ������������L�^��o�^����
		ac_flg.cycl_fg = 110;										// ������������L�^�o�^�J�n
		EcAlarmLog_Regist(&EcAlarm.Ec_Res);
	}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
																	/*								*/
	ac_flg.cycl_fg = 54;											/* 54:���Z���~�ް��o�^����		*/
																	/*								*/
	safecl( 7 );													/* ���ɖ����Z�o�A�ޑK�Ǘ��W�v	*/
																	/*								*/
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//	Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );		/* ���~���̋��K�Ǘ����O�f�[�^�쐬*/
//	Log_regist( LOG_MONEYMANAGE_NT );								/* ���K�Ǘ����O�o�^				*/
	if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {	/* ���~���̋��K�Ǘ����O�f�[�^�쐬*/
		Log_regist( LOG_MONEYMANAGE_NT );							/* ���K�Ǘ����O�o�^				*/
	}
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
																	/* �K�����ɖ����Z�o��ɍs�� 	*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �t���b�v�㏸�E���b�N�^�C�����o�ɏW�v														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: lto_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lto_syuu( void )											/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	ushort	iti;													/*								*/

																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
																	/*								*/
	iti = locktimeout.fus_d[0].t_iti - 1;							/* ���Ԉʒu(1�`324)-1			*/
																	/*								*/
	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* �s�W�v��ܰ��ر��(��d�΍�)	*/
	memcpy( &wkloktotal,											/* �s�W�v��ܰ��ر��(��d�΍�)	*/
			&loktl.tloktl.loktldat[iti],							/*								*/
			sizeof( LOKTOTAL_DAT ) );								/*								*/
																	/*								*/
	ac_flg.cycl_fg = 42;											/* 42:�s�W�v��ܰ��ر�֓]������	*/
																	/*								*/
																	/*								*/
	if( ryo_buf.svs_tim )											/* ���޽��ѓ��o�ɗL��			*/
	{																/*								*/
		ts->In_svst_seisan += 1L;									/* ���޽��ѓ����Z��+1			*/
	}																/*								*/
																	/*								*/
	if( DoBunruiSyu(ryo_buf.syubet) ){								/* ���ޏW�v����i��ʖ������j	*/
		bunrui( 0, ryo_buf.dsp_ryo , ryo_buf.syubet );				/* ���ޏW�v���Z					*/
	}
																	/*								*/

	ac_flg.cycl_fg = 43;											/* 45:ۯ��E�ׯ�ߏ㏸�o�ɏ��o�^����*/
	PayData_set_LO( locktimeout.fus_d[0].t_iti , 0 , 0 , 97);		/*								*/
	Log_Write(eLOG_PAYMENT, &PayData, TRUE);						/*								*/
																	/*								*/
	ac_flg.cycl_fg = 45;											/* 45:ۯ��E�ׯ�ߏ㏸�o�ɏ��o�^����*/
																	/*								*/
	memcpy( &wklocktimeout, &locktimeout, sizeof( struct FUSEI_SD ) );	/* ۯ��E�ׯ�ߏ㏸�o���ް���ܰ��ر��	*/
																	/*								*/
	ac_flg.cycl_fg = 46;											/* 46:ۯ��E�ׯ�ߏ㏸�o���ް���ܰ��ر�֓]������*/
																	/*								*/
	memcpy( &locktimeout, &wklocktimeout.fus_d[1],					/* �ް����						*/
			sizeof( struct FUSEI_D )*(LOCK_MAX-1) );				/*								*/
	locktimeout.kensuu -= 1;										/* �����|�P						*/
																	/*								*/
	ac_flg.cycl_fg = 47;											/* 47:�ް���āE����-1����		*/
																	/*								*/
	memset( &locktimeout.fus_d[LOCK_MAX-1], 0, sizeof( struct FUSEI_D ) );/*						*/
																	/*								*/
	ac_flg.cycl_fg = 48;											/* 48:6���ڸر����				*/
																	/*								*/
	CountUp( PAYMENT_COUNT );										/* ���Z�ǔ�+1					*/
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
	}
	return;															/*								*/
}																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �t���b�v�㏸�E���b�N�^�C�����o�ɏW�v����													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fus_kyo( void )																   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: void																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	lto_syuko( void )											/*								*/
{																	/*								*/
	ushort	iti = 0;												/*								*/
	struct	CAR_TIM		in_bak;										/*								*/
	struct	CAR_TIM		ot_bak;										/*								*/
	struct	CAR_TIM		in_f_bak;									/*								*/
	struct	CAR_TIM		ot_f_bak;									/*								*/
	ulong	tyu_ryo, tax, nyukin, turisen;							/*								*/
	ushort	pkiti;													/*								*/
																	/*								*/
	if (AppServ_IsLogFlashWriting(eLOG_PAYMENT) != 0)				// ���Z���O��FlashRom�����ݒ��H
		return;														// 
																	// 
	if(( locktimeout.kensuu != 0 )&&								/* ۯ��E�ׯ�ߏ㏸�o�ɗL��A����*/
	   ( ac_flg.cycl_fg == 0 ))										/* �󎚊���?					*/
	{																/*								*/
		iti = locktimeout.fus_d[0].t_iti;							/*								*/
		pkiti = ryo_buf.pkiti;										/* ���Ԉʒu						*/
		memcpy( &in_bak, &car_in, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &ot_bak, &car_ot, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &in_f_bak, &car_in_f, sizeof( struct CAR_TIM ));	/*								*/
		memcpy( &ot_f_bak, &car_ot_f, sizeof( struct CAR_TIM ));	/*								*/
		memcpy( &PayDataBack, &PayData, sizeof( Receipt_data ));	/* 1���Z���,�̎��؈��ް�		*/
		tyu_ryo = ryo_buf.tyu_ryo;									/* ���ԗ���						*/
		tax = ryo_buf.tax;											/* �����						*/
		nyukin = ryo_buf.nyukin;									/* �����z						*/
		turisen = ryo_buf.turisen;									/* �ޑK							*/
		if(( iti >= 1 )&&( iti <= LOCK_MAX ))						/* ���Ԉʒu�ԍ�1�ȏ�324�ȉ�?	*/
		{															/*								*/
			ac_flg.cycl_fg = 40;									/*								*/
			cm27();													/*								*/
			if( lto_tim() == 1 ){									/* ���o�Ɏ������				*/
																	/* ����NG?(Y)					*/
																	/*								*/
				ac_flg.cycl_fg = 45;								/* 45:ۯ��E�ׯ�ߏ㏸�o�ɏ��o�^����*/
																	/*								*/
				memcpy( &wklocktimeout, &locktimeout, sizeof( struct FUSEI_SD ) );/*				*/
																	/*								*/
				ac_flg.cycl_fg = 46;								/* 46:ۯ��E�ׯ�ߏ㏸�o���ް���ܰ��ر�֓]������*/
																	/*								*/
				memcpy( &locktimeout, &wklocktimeout.fus_d[1],		/* �ް����						*/
						sizeof(struct FUSEI_D)*(LOCK_MAX-1) );		/*								*/
				locktimeout.kensuu -= 1;							/* ����-1						*/
																	/*								*/
				ac_flg.cycl_fg = 47;								/* 47:�ް���āE����-1����		*/
																	/*								*/
				memset( &locktimeout.fus_d[(LOCK_MAX-1)],			/*								*/
						0, sizeof(struct FUSEI_D));					/*								*/
																	/*								*/
				ac_flg.cycl_fg = 0;									/* 0:6���ڸر����(�ҋ@)			*/
				return;												/*								*/
			}
			ryo_buf.credit.pay_ryo = 0;
			ryo_cal( 0, iti );										/* �����v�Z						*/
//			PayData_set(0,0);
			ryo_buf.svs_tim = 1;
			ryo_buf.dsp_ryo = 0;
			ac_flg.cycl_fg = 41;									/*								*/
			lto_syuu();												/* �W�v���Z						*/
																	/* 								*/
		} else {													/* �Ԏ��ԍ����z��O?(Y)			*/
																	/* 								*/
			ac_flg.cycl_fg = 45;									/* 45:ۯ��E�ׯ�ߏ㏸�o�ɏ��o�^����*/
																	/*								*/
			memcpy( &wklocktimeout, &locktimeout, sizeof( struct FUSEI_SD ) );	/*					*/
																	/*								*/
			ac_flg.cycl_fg = 46;									/* 46:ۯ��E�ׯ�ߏ㏸�o���ް���ܰ��ر�֓]������*/
																	/*								*/
			memcpy( &locktimeout, &wklocktimeout.fus_d[1],			/* �ް����						*/
					sizeof(struct FUSEI_D)*(LOCK_MAX-1) );			/*								*/
			locktimeout.kensuu -= 1;								/* ����-1						*/
																	/*								*/
			ac_flg.cycl_fg = 47;									/* 47:�ް���āE����-1����		*/
																	/*								*/
			memset( &locktimeout.fus_d[(LOCK_MAX-1)],				/*								*/
					0, sizeof(struct FUSEI_D));						/*								*/
																	/*								*/
			ac_flg.cycl_fg = 48;									/* 48:6���ڸر����				*/
																	/*								*/
			ex_errlg( ERRMDL_MAIN, ERR_MAIN_LOCKNUMNG, 2, 0 );		/* ۯ��E�ׯ�ߏ㏸�o�Ɉʒu1�`324�ȊO	*/
		}															/*								*/
		ac_flg.cycl_fg = 0;											/*								*/
		ryo_buf.pkiti = pkiti;										/* ���Ԉʒu						*/
		memcpy( &car_in, &in_bak, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &car_ot, &ot_bak, sizeof( struct CAR_TIM ));		/*								*/
		memcpy( &car_in_f, &in_f_bak, sizeof( struct CAR_TIM ));	/*								*/
		NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
		memcpy( &car_ot_f, &ot_f_bak, sizeof( struct CAR_TIM ));	/*								*/
		memcpy( &PayData, &PayDataBack, sizeof( Receipt_data ));	/* 1���Z���,�̎��؈��ް�		*/
		NTNET_Data152_SaveDataUpdate();
		ryo_buf.tyu_ryo = tyu_ryo;									/* ���ԗ���						*/
		ryo_buf.tax = tax;											/* �����						*/
		ryo_buf.nyukin = nyukin;									/* �����z						*/
		ryo_buf.turisen = turisen;									/* �ޑK							*/
																	/*								*/
	}																/*								*/
	return;															/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���o�Ɏ����Z�b�g																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: lto_tim( void )																   |*/
/*| RETURN VALUE: ret	: ���o�Ɏ�������(1980�`2079�N�ȓ����H)  0:OK  1:NG						   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*|				car_in,car_ot,car_in_f,car_ot_f�ɓ��o�Ɏ������Z�b�g����							   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short	lto_tim( void )										/*								*/
{																	/*								*/
	ushort	in_tim, out_tim;										/*								*/
																	/*								*/
	memset( &car_in, 0, sizeof( struct CAR_TIM ) );					/* ���Ԏ����ر					*/
	memset( &car_ot, 0, sizeof( struct CAR_TIM ) );					/* �o�Ԏ����ر					*/
	memset( &car_in_f, 0, sizeof( struct CAR_TIM ) );				/* ���Ԏ����ر					*/
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memset( &car_ot_f, 0, sizeof( struct CAR_TIM ) );				/* �o�Ԏ����ر					*/
																	/*								*/
	car_in.year = locktimeout.fus_d[0].iyear;						/* ����	�N						*/
	car_in.mon  = locktimeout.fus_d[0].imont;						/*		��						*/
	car_in.day  = locktimeout.fus_d[0].idate;						/*		��						*/
	car_in.hour = locktimeout.fus_d[0].ihour;						/*		��						*/
	car_in.min  = locktimeout.fus_d[0].iminu;						/*		��						*/
	car_in.week = (char)youbiget( car_in.year,						/*		�j��					*/
								(short)car_in.mon,					/*								*/
								(short)car_in.day );				/*								*/
						 											/*								*/
	car_ot.year = locktimeout.fus_d[0].oyear;						/* �o��	�N						*/
	car_ot.mon  = locktimeout.fus_d[0].omont;						/*		��						*/
	car_ot.day  = locktimeout.fus_d[0].odate;						/*		��						*/
	car_ot.hour = locktimeout.fus_d[0].ohour;						/*		��						*/
	car_ot.min  = locktimeout.fus_d[0].ominu;						/*		��						*/
	car_ot.week = (char)youbiget( car_ot.year,						/*		�j��					*/
								(short)car_ot.mon,					/*								*/
								(short)car_ot.day );				/*								*/
	memcpy( &car_in_f, &car_in, sizeof( struct CAR_TIM ) );			/* ���Ԏ���Fix					*/
	NTNET_Data152Save((void *)&car_in_f, NTNET_152_CARINTIME);
	memcpy( &car_ot_f, &car_ot, sizeof( struct CAR_TIM ) );			/* �o�Ԏ���Fix					*/
																	/*								*/
	if( ( !car_in.year || !car_in.mon || !car_in.day ) ||
	    ( !car_ot.year || !car_ot.mon || !car_ot.day ) ||
	    chkdate( car_in.year, (short)car_in.mon, (short)car_in.day )||	/* ���Ɏ����K��O?			*/
	    chkdate( car_ot.year, (short)car_ot.mon, (short)car_ot.day ) ){	/* �o�Ɏ����K��O?			*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_FKINOUTTIMENG, 2, 0 );		/* ���o�Ɏ����K��O				*/
		return( 1 );												/*								*/
	}																/*								*/
	in_tim = dnrmlzm((short)( car_in.year + 1 ),					/* (���ɓ�+1�N)normlize			*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	out_tim = dnrmlzm( car_ot.year,									/* �o�ɓ�normlize				*/
					(short)car_ot.mon,								/*								*/
					(short)car_ot.day );							/*								*/
	if( in_tim <= out_tim ){										/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_FKOVER1YEARCAL, 2, 1 );		/* 1�N�ȏ�̗����v�Z���s�����Ƃ��� */
		return( 1 );												/*								*/
	}																/*								*/
	in_tim = dnrmlzm((short)( car_in.year ),						/* (���ɓ�)normlize				*/
					(short)car_in.mon,								/*								*/
					(short)car_in.day );							/*								*/
	if( in_tim > out_tim ){											/*								*/
		ex_errlg( ERRMDL_MAIN, ERR_MAIN_FKINOUTGYAKU, 2, 1 );		/* ���o�ɋt�]					*/
		return( 1 );												/*								*/
	}																/*								*/
	return( 0 );													/*								*/
}																	/*								*/

//[]----------------------------------------------------------------------[]
///	@brief			�s�E�f�s�E�l�s�v�����g
//[]----------------------------------------------------------------------[]
///	@param[in]		preq	: �󎚃^�C�v
///	@param[in]		*src	: �W�v�f�[�^
///	@return			*�󎚃f�[�^
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/09/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
SYUKEI	*syuukei_prn( int preq, SYUKEI *src )
{
	uchar	set1_39_6;

	src->Kikai_no = (uchar)CPrmSS[S_PAY][2];					// �@�B��
	if (preq == PREQ_AT_SYUUKEI){
		src->Kakari_no = 0;										// �����W�v���͌W���ԍ����󎚂��Ȃ�
	}
	else{
		src->Kakari_no = OPECTL.Kakari_Num;						// �W���No.
		loktl.tloktl.Kakari_no = OPECTL.Kakari_Num;
	}
	memcpy( &src->NowTime, &CLK_REC, sizeof( date_time_rec ) );	// ���ݎ���
	if (preq == PREQ_AT_SYUUKEI ||
		preq == PREQ_TGOUKEI) {
		memcpy( &src->In_car_Tcnt, &CarCount, sizeof( CAR_COUNT ) );	// ���o�ɑ䐔
		CountGet( T_TOTAL_COUNT, &src->Oiban );					// T���v�ǔ�
		CountGet( T_TOTAL_COUNT, &loktl.tloktl.Oiban );			// T���v�ǔ�(�Ԏ��W�v)
	}
	else if (preq == PREQ_GTGOUKEI) {
		CountGet( GT_TOTAL_COUNT, &src->Oiban );				// GT���v�ǔ�
		CountGet( GT_TOTAL_COUNT, &loktl.tloktl.Oiban );		// GT���v�ǔ�(�Ԏ��W�v)
	}
	set1_39_6 = (uchar)prm_get(COM_PRM,S_SYS,39,1,1);
	if( set1_39_6 == 3 ){
		src->Now_car_Tcnt = PPrmSS[S_P02][6]+PPrmSS[S_P02][10];	// �����ݒ��ԑ䐔
	}else if( set1_39_6 == 4 ){
		src->Now_car_Tcnt = PPrmSS[S_P02][6]+PPrmSS[S_P02][10]	// �����ݒ��ԑ䐔
							   + PPrmSS[S_P02][14];
	}else{
		src->Now_car_Tcnt = PPrmSS[S_P02][2];					// �����ݒ��ԑ䐔
	}
	src->Now_car_cnt[0] = PPrmSS[S_P02][6];						// ���ݒ��ԑ䐔1
	src->Now_car_cnt[1] = PPrmSS[S_P02][10];					// ���ݒ��ԑ䐔2
	src->Now_car_cnt[2] = PPrmSS[S_P02][14];					// ���ݒ��ԑ䐔3
	memcpy(&skyprn, src, sizeof(SYUKEI));			// �󎚃f�[�^�m��
	return &skyprn;
}

// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
const	date_time_rec	nulldate = { 0 };
SYUKEI	*tsyTbl[] = { &skyprn, &sky.gsyuk, &sky.msyuk};				/* ����s�W�v�i�󎚁j			*/
SYUKEI	*dsyTbl[] = { &sky.tsyuk, &sky.gsyuk, &sky.msyuk};			/* ���݂s�W�v					*/
SYUKEI	*zsyTbl[] = { &skybk.tsyuk, &skybk.gsyuk, &skybk.msyuk};	/* �O��s�W�v					*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �W�v�ر�N���A���菈��																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: Check_syuukei_clr( char no )														   |*/
/*| PARAMETER	: no	: 0 = �s���v																		   |*/
/*| 					: 1 = �f�s���v															   |*/
/*| 					: 2 = �l�s���v															   |*/
/*| RETURN VALUE: char	: 0 = �N���A�s��														   |*/
/*|						  1 = �N���A��															   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
char	Check_syuukei_clr( char no )
{
	unsigned long	*ts, *gs;										/*								*/
	SYUKEI	*tsy, *dsy, *zsy;
	int		i;														/*								*/

	tsy = tsyTbl[no];												/* ����W�v�i�󎚁j				*/
	dsy = dsyTbl[no];												/* ���ݏW�v						*/
	zsy = zsyTbl[no];												/* �O��W�v						*/
	

	//�@�󎚏W�v�̃w�b�_�[�ƑO��W�v�̃w�b�_�[�̔�r
	// syuukei_clr()���ꂽ�ꍇ�A�󎚂����f�[�^�ƑO��W�v����v����
	if (memcmp(zsy, tsy, _SYU_HDR_SIZ) == 0) {
	// �󎚓��e���o�^�ς݂̑O�񍇌v�ƈ�v
		wmonlg(OPMON_SYUUKEI_IRGCLR1, NULL, (ulong)(no+1));					// ���j�^�o�^
		return 0;
	}

	//�A���ݏW�v�̒ǔԁA����W�v�����A�O��W�v������ �󎚏W�v�̒ǔԁA����W�v�����A�O��W�v�����̔�r
	if( ( 0 != memcmp( &dsy->NowTime, &tsy->NowTime, sizeof( date_time_rec ) ))	||	// ����W�v�������s��v
		( 0 != memcmp( &dsy->OldTime, &tsy->OldTime, sizeof( date_time_rec ) )) ||		// �O��W�v�������s��v
		( CountSel( &dsy->Oiban ) != CountSel( &tsy->Oiban ) )){						// �󎚊J�n���ɃZ�b�g�����ǔԂƕs��v
		// �s��v�̏ꍇ�A�N���A�s��
		wmonlg(OPMON_SYUUKEI_IRGCLR2, NULL, (ulong)(no+1));					// ���j�^�o�^
		return 0;
	}

	//�C���ݏW�v�̍���W�v������0�N���A����Ă��邩�m�F
	if (memcmp(&dsy->NowTime, &nulldate, sizeof(dsy->NowTime)) == 0) {
		// ���ݏW�v�������Ȃ����N���A����Ă���
		wmonlg(OPMON_SYUUKEI_IRGCLR4, NULL, (ulong)(no+1));					// ���j�^�o�^
		return 0;
	}

	// T���v�ȊO�͂����܂ł�OK�Ƃ���
	if( no != 0 ){
		return 1;
	}
	
	// �����܂ł�1���T���v��2��N���A���������s����Ă��͂������Ǝv���邪�A�l�̔�r���s��
	// �K���uT���v�󎚃G���A��T���v�{�̃G���A�v�̂͂��Ȃ̂ō��L�����𖞂����Ȃ����ڂ�����΃N���A���������s���Ȃ�
	gs = &dsy->Uri_Tryo;										/*								*/
	ts = &tsy->Uri_Tryo;										/* �s�|�󎚂s					*/ 
	WACDOG;														/* �����ޯ�ؾ�Ď��s				*/
	//�B����T���v���ڂ̒l�ƈ�T���v���ڂ̒l�̔�r
	for( i = 0; i < _SYU_DAT_CNT; i++, ts++, gs++ )				/*								*/
	{															/*								*/
		if( *gs < *ts ){										// T�W�v�󎚂̕����傫���l�͂��蓾�Ȃ�
			wmonlg(OPMON_SYUUKEI_IRGCLR3, NULL, (ulong)(no+1));			// ���j�^�o�^
			return 0;											/*								*/
		}														/*								*/
	}															/*								*/
	WACDOG;														/* �����ޯ�ؾ�Ď��s				*/
	
	return 1;
}
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �s�E�f�s�W�v�ر�X�V																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: syuukei_clr( no )																   |*/
/*| PARAMETER	: no	: 0 = �s���v															   |*/
/*| 					: 1 = �f�s���v															   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-12-28																	   |*/
/*| Update		: 2002-03-26 Y.Takahashi                                                           |*/
/*| Update		: 2004-07-13 T.Nakayama 4500EX:FLP_MAX�ɒu������                                   |*/
/*|             : �T�[�r�X���A����������Z                                                       |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	syuukei_clr( char no )										/*								*/
{																	/*								*/
	unsigned long	*ts, *gs;										/*								*/
	SYUKEI	*tsy, *gsy, *msy, *wsy, *tsybk, *dsy;					/* msy(MT���v),dsy(T���v)��ǉ�	*/
	int		i;														/*								*/
																	/*								*/
	tsy = &skyprn;													/*								*/
	gsy = &sky.gsyuk;												/*								*/
	msy = &sky.msyuk;												/* msy(MT���v)��ǉ�			*/
	tsybk = &skybk.tsyuk;											/*								*/
	wsy = &wksky;													/*								*/
																	/*								*/
	if( no == 0 ){													/*								*/
		ac_flg.syusyu = 12;											/* 12:�s���v�󎚊���			*/
																	/*								*/
		memcpy( wsy, gsy, sizeof( SYUKEI ) );						/* �f�s�W�v��ܰ��ر��(��d�΍�)	*/
		memcpy( &wkloktotal, loktl.gloktl.loktldat, sizeof( LOKTOTAL_DAT ) ); /* �Ԏ���GT�W�v��ܰ��ر��(��d�΍�)	*/
		memcpy( &CarCount_W, &CarCount, sizeof( CAR_COUNT ) );		/* ���o�ɶ����ޯ�����			*/
		ac_flg.syusyu = 13;											/* 13:�f�s�W�v��ܰ��ر�֓]������*/
																	/*								*/
		if( tsy->In_car_Tcnt <= CarCount.In_car_Tcnt ){				/*								*/
			CarCount.In_car_Tcnt -= tsy->In_car_Tcnt;				/* �����ɑ䐔�X�V				*/
		}else{														/*								*/
			CarCount.In_car_Tcnt = 0L;								/* �����ɑ䐔�X�V				*/
		}															/*								*/
		if( tsy->In_car_cnt[0] <= CarCount.In_car_cnt[0] ){			/*								*/
			CarCount.In_car_cnt[0] -= tsy->In_car_cnt[0];			/* ���ɑ䐔1�X�V				*/
		}else{														/*								*/
			CarCount.In_car_cnt[0] = 0L;							/* ���ɑ䐔1�X�V				*/
		}															/*								*/
		if( tsy->In_car_cnt[1] <= CarCount.In_car_cnt[1] ){			/*								*/
			CarCount.In_car_cnt[1] -= tsy->In_car_cnt[1];			/* ���ɑ䐔2�X�V				*/
		}else{														/*								*/
			CarCount.In_car_cnt[1] = 0L;							/* ���ɑ䐔2�X�V				*/
		}															/*								*/
		if( tsy->In_car_cnt[2] <= CarCount.In_car_cnt[2] ){			/*								*/
			CarCount.In_car_cnt[2] -= tsy->In_car_cnt[2];			/* ���ɑ䐔3�X�V				*/
		}else{														/*								*/
			CarCount.In_car_cnt[2] = 0L;							/* ���ɑ䐔3�X�V				*/
		}															/*								*/
		if( tsy->Out_car_Tcnt <= CarCount.Out_car_Tcnt ){			/*								*/
			CarCount.Out_car_Tcnt -= tsy->Out_car_Tcnt;				/* ���o�ɑ䐔�X�V				*/
		}else{														/*								*/
			CarCount.Out_car_Tcnt = 0L;								/* ���o�ɑ䐔�X�V				*/
		}															/*								*/
		if( tsy->Out_car_cnt[0] <= CarCount.Out_car_cnt[0] ){		/*								*/
			CarCount.Out_car_cnt[0] -= tsy->Out_car_cnt[0];			/* �o�ɑ䐔1�X�V				*/
		}else{														/*								*/
			CarCount.Out_car_cnt[0] = 0L;							/* �o�ɑ䐔1�X�V				*/
		}															/*								*/
		if( tsy->Out_car_cnt[1] <= CarCount.Out_car_cnt[1] ){		/*								*/
			CarCount.Out_car_cnt[1] -= tsy->Out_car_cnt[1];			/* �o�ɑ䐔2�X�V				*/
		}else{														/*								*/
			CarCount.Out_car_cnt[1] = 0L;							/* �o�ɑ䐔2�X�V				*/
		}															/*								*/
		if( tsy->Out_car_cnt[2] <= CarCount.Out_car_cnt[2] ){		/*								*/
			CarCount.Out_car_cnt[2] -= tsy->Out_car_cnt[2];			/* �o�ɑ䐔3�X�V				*/
		}else{														/*								*/
			CarCount.Out_car_cnt[2] = 0L;							/* �o�ɑ䐔3�X�V				*/
		}															/*								*/
																	/*								*/
		ts = &tsy->Uri_Tryo;										/*								*/
		gs = &sky.gsyuk.Uri_Tryo;									/*								*/
																	/*								*/
		for( i = 0; i < _SYU_DAT_CNT; i++, ts++, gs++ )				/*								*/
		{															/*								*/
			*gs += *ts;												/*								*/
		}															/*								*/
		CountAdd(gsy, tsy);											/* �ǔԉ��Z						*/
																	/*								*/
		for( i = 0; i < LOCK_MAX; i++ )
		{
			WACDOG;													// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			loktl.gloktl.loktldat[i].Genuri_ryo += loktl.tloktl.loktldat[i].Genuri_ryo;
			loktl.gloktl.loktldat[i].Seisan_cnt += loktl.tloktl.loktldat[i].Seisan_cnt;
			loktl.gloktl.loktldat[i].Husei_out_cnt += loktl.tloktl.loktldat[i].Husei_out_cnt;
			loktl.gloktl.loktldat[i].Husei_out_ryo += loktl.tloktl.loktldat[i].Husei_out_ryo;
			loktl.gloktl.loktldat[i].Kyousei_out_cnt += loktl.tloktl.loktldat[i].Kyousei_out_cnt;
			loktl.gloktl.loktldat[i].Kyousei_out_ryo += loktl.tloktl.loktldat[i].Kyousei_out_ryo;
			loktl.gloktl.loktldat[i].Uketuke_pri_cnt += loktl.tloktl.loktldat[i].Uketuke_pri_cnt;
			loktl.gloktl.loktldat[i].Syuusei_seisan_cnt += loktl.tloktl.loktldat[i].Syuusei_seisan_cnt;
			loktl.gloktl.loktldat[i].Syuusei_seisan_ryo += loktl.tloktl.loktldat[i].Syuusei_seisan_ryo;
		}

		memcpy( tsybk, tsy, sizeof( SYUKEI ) );						/* �s�W�v��O��s�W�v�־��		*/
		memcpy( &loktlbk.tloktl, &loktl.tloktl, sizeof( LOKTOTAL ) ); /* �Ԏ����W�v��O��W�v�־��	*/
																	/*								*/
		ac_flg.syusyu = 14;											/* 14:�s�W�v���f�s�W�v�։��Z����*/
																	/*								*/
		Log_regist( LOG_TTOTAL );									/* �s���v���o�^				*/
																	/*								*/
		ac_flg.syusyu = 16;											/* 16:�s���v���o�^����		*/
																	/*								*/
																	/*								*/
		if( prm_get(COM_PRM, S_SCA, 11, 1, 1) == 1 )				/*								*/
			Suica_Data_Snd( S_PAY_DATA,&CLK_REC);					/* SX-10���ߏ������s			*/
																	/*								*/
// note:�󎚒��ɐ��Z�Ȃ��o�ɂȂǂ�T�W�v���J�E���g�A�b�v���Ă��܂����ꍇ�̑΍�
//		T�W�v���s���A�usky.tsyuk�v����uskyprn�v���쐬���A�����T�W�v�̃f�[�^�Ƃ��Ċm�肳����B
//		�uskyprn�v�̃f�[�^�ň���ALog�o�^�Ȃǂ̏��������{�B�usky.tsyuk�v�͏W�v�̈�Ƃ��āA�ʏ�ʂ�J�E���g�A�b�v������B
//		�usky.tsyuk�v - �uskyprn�v ������ƈ󎚒��ɃJ�E���g�A�b�v�������̂̂ݎc��A������ȍ~��T�W�v�usky.tsyuk�v�Ƃ���B
		dsy = &sky.tsyuk;		// dst = T							/* �s�W�v						*/
		memcpy( wsy, dsy, sizeof( SYUKEI ) );						/* �s�W�v��ܰ��ر��(��d�΍�)	*/
		ac_flg.syusyu = 17;											/* 17:�s�W�v��ܰ��ر�֓]������	*/
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j(�R�����g�̂ݒǉ�)
// ������T�W�v�̃w�b�_�[�����N���A���Ă���̂ŒǔԃG���A��0�ɂȂ�
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j(�R�����g�̂ݒǉ�)
		memset( dsy, 0, _SYU_HDR_SIZ );								/* �w�b�_�[���N���A				*/
		gs = &dsy->Uri_Tryo;										/*								*/
		ts = &tsy->Uri_Tryo;										/* �s�|�󎚂s					*/ 
		WACDOG;														/* �����ޯ�ؾ�Ď��s				*/
		for( i = 0; i < _SYU_DAT_CNT; i++, ts++, gs++ )				/*								*/
		{															/*								*/
			*gs -= *ts;												/*								*/
		}															/*								*/
		memcpy( &dsy->OldTime, &tsybk->NowTime, sizeof( date_time_rec ) ); /* �O��W�v�������			*/
		memset( &loktl.tloktl, 0, sizeof( LOKTOTAL ) );				/* �Ԏ����W�v��ر				*/
																	/*								*/
		ac_flg.syusyu = 18;											/* 18:�s�W�v�ر�ر����			*/
																	/*								*/
		CountUp( T_TOTAL_COUNT );									/* T���v�ǔ�+1					*/
		CountClear( CLEAR_COUNT_T );								/* �ǔԸر(T���v������)			*/
		if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			RAU_UpdateCenterSeqNo(RAU_SEQNO_TOTAL);
		}
																	/*								*/
	}																/*								*/
	else if( no == 1 ) {											/*								*/
		int		mtflg;
		ac_flg.syusyu = 22;											/* 22:�f�s���v�󎚊���			*/
		mtflg = (prm_get(COM_PRM, S_TOT, 1, 1, 1) != 0)? 1 : 0;		/* MT�W�v����^���Ȃ�			*/
																	/*								*/
		memcpy( wsy, msy, sizeof( SYUKEI ) );						/* �l�s�W�v��ܰ��ر��(��d�΍�)	*/
																	/*								*/
		ac_flg.syusyu = 23;											/* 23:�l�s�W�v��ܰ��ر�֓]������*/
																	/*								*/
		memcpy( &skybk.gsyuk, gsy, sizeof( SYUKEI ) );				/* �f�s�W�v��O��f�s�W�v�־��	*/
		memcpy( &loktlbk.gloktl, &loktl.gloktl, sizeof( LOKTOTAL ) ); /* �Ԏ����W�v��O��W�v�־��	*/
		if (mtflg) {
			ts = &gsy->Uri_Tryo;									/*								*/
			gs = &msy->Uri_Tryo;									/*								*/
																	/*								*/
			WACDOG;													/* �����ޯ�ؾ�Ď��s				*/
			for( i = 0; i < _SYU_DAT_CNT; i++, ts++, gs++ )			/*								*/
			{														/*								*/
				*gs += *ts;											/*								*/
			}														/*								*/
			CountAdd(msy, gsy);										/* �ǔԉ��Z						*/
		}
		if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			tsy->CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_TOTAL);
		}

		ac_flg.syusyu = 24;											/* 24:�f�s�W�v���l�s�W�v�։��Z����*/
		Log_regist( LOG_GTTOTAL );									/* �f�s���v���o�^				*/

		ac_flg.syusyu = 26;											/* 26:�f�s���v���o�^����		*/

		memset( gsy, 0, sizeof( sky.gsyuk ) );						/* �f�s�W�v�ر�ر				*/
		memcpy( &gsy->OldTime, &skybk.gsyuk.NowTime, sizeof( date_time_rec ) );	/* �O��W�v�������		*/
		memset( &loktl.gloktl, 0, sizeof( LOKTOTAL ) );				/* �Ԏ����W�v��ر				*/
																	/*								*/
		ac_flg.syusyu = 27;											/* 27:�f�s�W�v�ر�ر����		*/
																	/*								*/
		if (mtflg) {
			CountUp( GT_TOTAL_COUNT );								/* GT���v�ǔ�+1					*/
		}
		CountClear( CLEAR_COUNT_GT );								/* �ǔԸر(GT���v������)		*/
		if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			RAU_UpdateCenterSeqNo(RAU_SEQNO_TOTAL);
		}
																	/*								*/
	}																/*								*/
	else{
	// �l�s���v�󎚊�����
		ac_flg.syusyu = 102;										/* 102:�l�s���v�󎚊���			*/
		memcpy( &skybk.msyuk, msy, sizeof( SYUKEI ) );				/* �@�l�s�W�v��O��l�s�W�v��	*/
																	/*								*/
		ac_flg.syusyu = 107;										/*								*/
		memset( msy, 0, sizeof( SYUKEI ) );							/* �l�s�W�v�ر�ر				*/
		memcpy( &msy->OldTime, &skybk.msyuk.NowTime, sizeof( date_time_rec ) ); /* �O��W�v�������			*/
		CountClear( CLEAR_COUNT_MT );								/* �ǔԸر(MT���v������)		*/ 
																	/*								*/
		ac_flg.syusyu = 0;											/*								*/
	}																/*								*/
																	/*								*/
	return;															/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ��݁E�������ɏW�v����																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kinko_syu( no, pri )															   |*/
/*| PARAMETER	: no	: 0 = ��݋��ɍ��v														   |*/
/*| 					: 1 = �������ɍ��v														   |*/
/*| 					: 2 = ��݋��ɏ��v														   |*/
/*| 					: 3 = �������ɏ��v														   |*/
/*| 																							   |*/
/*| 			  pri	: 0 = �󎚂Ȃ�															   |*/
/*| 			  		: 1 = �󎚂���															   |*/
/*| 																							   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*| 																							   |*/
/*| ���l�F��Q���Ұ��i�󎚂Ȃ��^����j�́A���ɍ��v���̂ݗL���Ƃ���								   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Motohashi																	   |*/
/*| Date		: 2005-09-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	kinko_syu( char no, char pri )
{
	int		i;
	T_FrmCoSyuukei FrmCoSyuukei;
	T_FrmSiSyuukei FrmSiSyuukei;

	switch( no ){

		case	0:	// ��݋��ɍ��v

			if( pri == 1 ){
				ac_flg.syusyu = 30;														// 30:���ް��쐬�J�n
			}
			memset( &coin_syu.Tryo, 0, sizeof(ulong) * (1+(COIN_SYU_CNT*2)) );
			coin_syu.cnt[0] = SFV_DAT.safe_dt[0];
			coin_syu.cnt[1] = SFV_DAT.safe_dt[1];
			coin_syu.cnt[2] = SFV_DAT.safe_dt[2];
			coin_syu.cnt[3] = SFV_DAT.safe_dt[3];
			coin_syu.ryo[0] = ( (unsigned long)coin_syu.cnt[0] * 10L );
			coin_syu.ryo[1] = ( (unsigned long)coin_syu.cnt[1] * 50L );
			coin_syu.ryo[2] = ( (unsigned long)coin_syu.cnt[2] * 100L );
			coin_syu.ryo[3] = ( (unsigned long)coin_syu.cnt[3] * 500L );
			for( i = 0; i < 4; i++ ){
				coin_syu.Tryo += coin_syu.ryo[i];										// ���ɓ������z���Z
			}
			if( pri == 1 ){
				ac_flg.syusyu = 31;														// 31:���ް��쐬����
			}
			coin_syu.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// �@�B��
			CountGet( COIN_SAFE_COUNT, &coin_syu.Oiban );								// �ǔ�
			coin_syu.Kakari_no = OPECTL.Kakari_Num;										// �W���ԍ�set
			memcpy( &coin_syu.NowTime, &CLK_REC, sizeof( date_time_rec ) );				// ���ݎ���
			if( pri == 1 ){
				// �󎚂���ꍇ
				FrmCoSyuukei.prn_kind = R_PRI;
				FrmCoSyuukei.prn_data = &coin_syu;
				queset( PRNTCBNO, PREQ_COKINKO_G, sizeof(T_FrmCoSyuukei), &FrmCoSyuukei );	// ��݋��ɍ��v��
			}
			else{
				// �󎚂��Ȃ��ꍇ
				kinko_clr( 0 );															// ��݋��ɍX�V����
			}

			break;

		case	1:	// �������ɍ��v

			if( pri == 1 ){
				ac_flg.syusyu = 40;														// 40:���ް��쐬�J�n
			}
			memset( &note_syu.Tryo, 0, sizeof(ulong) * (1+(SIHEI_SYU_CNT*2)) );
			note_syu.cnt[0] = ( (unsigned long)SFV_DAT.nt_safe_dt );					// ��������
			note_syu.ryo[0] = ( (unsigned long)SFV_DAT.nt_safe_dt*1000L );				// ���ɓ����z
			for( i = 0; i < 4; i++ ){
				note_syu.Tryo += note_syu.ryo[i];										// ���ɓ������z���Z
			}
			if( pri == 1 ){
				ac_flg.syusyu = 41;														// 41:���ް��쐬����
			}
			note_syu.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// �@�B��
			CountGet( NOTE_SAFE_COUNT, &note_syu.Oiban );								// �ǔ�
			note_syu.Kakari_no = OPECTL.Kakari_Num;										// �W���ԍ�set
			memcpy( &note_syu.NowTime, &CLK_REC, sizeof( date_time_rec ) );				// ���ݎ���
			if( pri == 1 ){
				// �󎚂���ꍇ
				FrmSiSyuukei.prn_kind = R_PRI;
				FrmSiSyuukei.prn_data = &note_syu;
				queset( PRNTCBNO, PREQ_SIKINKO_G, sizeof(T_FrmSiSyuukei), &FrmSiSyuukei );	// �������ɍ��v��
			}
			else{
				// �󎚂��Ȃ��ꍇ
				kinko_clr( 1 );															// �������ɍX�V����
			}

			break;

		case	2:	// ��݋��ɏ��v

			memset( &coin_syu.Tryo, 0, sizeof(ulong) * (1+(COIN_SYU_CNT*2)) );
			coin_syu.cnt[0] = SFV_DAT.safe_dt[0];
			coin_syu.cnt[1] = SFV_DAT.safe_dt[1];
			coin_syu.cnt[2] = SFV_DAT.safe_dt[2];
			coin_syu.cnt[3] = SFV_DAT.safe_dt[3];
			coin_syu.ryo[0] = ( (unsigned long)coin_syu.cnt[0] * 10L );
			coin_syu.ryo[1] = ( (unsigned long)coin_syu.cnt[1] * 50L );
			coin_syu.ryo[2] = ( (unsigned long)coin_syu.cnt[2] * 100L );
			coin_syu.ryo[3] = ( (unsigned long)coin_syu.cnt[3] * 500L );
			for( i = 0; i < 4; i++ ){
				coin_syu.Tryo += coin_syu.ryo[i];										// ���ɓ������z���Z
			}

			coin_syu.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// �@�B��
			coin_syu.Kakari_no = OPECTL.Kakari_Num;										// �W���ԍ�set
			memcpy( &coin_syu.NowTime, &CLK_REC, sizeof( date_time_rec ) );				// ���ݎ���
			FrmCoSyuukei.prn_kind = R_PRI;
			FrmCoSyuukei.prn_data = &coin_syu;
			queset( PRNTCBNO, PREQ_COKINKO_S, sizeof(T_FrmCoSyuukei), &FrmCoSyuukei );	// ��݋��ɏ��v��

			break;

		case	3:	// �������ɏ��v

			memset( &note_syu.Tryo, 0, sizeof(ulong) * (1+(SIHEI_SYU_CNT*2)) );
			note_syu.cnt[0] = ( (unsigned long)SFV_DAT.nt_safe_dt );					// ��������
			note_syu.ryo[0] = ( (unsigned long)SFV_DAT.nt_safe_dt*1000L );				// ���ɓ����z
			for( i = 0; i < 4; i++ ){
				note_syu.Tryo += note_syu.ryo[i];										// ���ɓ������z���Z
			}

			note_syu.Kikai_no = (uchar)CPrmSS[S_PAY][2];								// �@�B��
			note_syu.Kakari_no = OPECTL.Kakari_Num;										// �W���ԍ�set
			memcpy( &note_syu.NowTime, &CLK_REC, sizeof( date_time_rec ) );				// ���ݎ���
			FrmSiSyuukei.prn_kind = R_PRI;
			FrmSiSyuukei.prn_data = &note_syu;
			queset( PRNTCBNO, PREQ_SIKINKO_S, sizeof(T_FrmSiSyuukei), &FrmSiSyuukei );	// �������ɏW�v��

			break;

	}

	return;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ��݁E�������ɏW�v�X�V																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kinko_clr( no )																   |*/
/*| PARAMETER	: no	: 0 = ��݋���															   |*/
/*| 					: 1 = ��������															   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-12-12																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	kinko_clr( char no )										/*								*/
{																	/*								*/
	SYUKEI	*ts, *ws;												/*								*/
	int		i;														/*								*/
	COIN_SYU	*cbk;
	NOTE_SYU	*nbk;

	cbk = &cobk_syu;
	nbk = &nobk_syu;
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
																	/*								*/
																	/*								*/
	if( no == 0 ){													/*								*/
		ac_flg.syusyu = 32;											/* 32:�󎚊���					*/
		memcpy( ws, ts, sizeof( sky.tsyuk ) );						/* �s�W�v��ܰ��ر�ցi��d�΍�j	*/
		ac_flg.syusyu = 33;											/* 33:�s�W�v��ܰ��ر�֓]������	*/
																	/*								*/
		ts->Kinko_Tryo += coin_syu.Tryo;							/* �s�W�v�։��Z					*/
		ts->Coin_Tryo += coin_syu.Tryo;								/* �s�W�v�։��Z					*/
		for( i = 0; i < 4; i++ ){									/*								*/
			ts->Coin_cnt[i] += coin_syu.cnt[i];						/*								*/
			ts->Coin_ryo[i] += coin_syu.ryo[i];						/*								*/
		}															/*								*/
		ts->Ckinko_goukei_cnt += 1;									/* ��݋��ɍ��v��				*/
																	/*								*/
		//ts->kinko += coin_syu.Tryo;								/* �s�W�v�։��Z					*/
		//for( i = 0; i < 4; i++ ){									/*								*/
		//	ts->kin[i] += coin_syu.ryo[i];							/*								*/
		//}															/*								*/
																	/*								*/
		memcpy( cbk, &coin_syu, sizeof( coin_syu ) );				/* �R�C�����ɍ��v��O��R�C�����ɍ��v�־��*/
		ac_flg.syusyu = 34;											/* 34:���ɏW�v���s�W�v�֓]������*/
																	/*								*/
		Log_regist( LOG_COINBOX );									/* �R�C�����ɏW�v�����Z		*/
																	/*								*/
																	/*								*/
		memset( &SFV_DAT.safe_dt[0], 0, sizeof( SFV_DAT.safe_dt ));	/*								*/
		memcpy( &coin_syu.OldTime,									/*								*/
				&coin_syu.NowTime, sizeof( date_time_rec ) );		/* �O��W�v�������				*/
		memset( &coin_syu.Tryo, 0, sizeof(ulong) * (1+(COIN_SYU_CNT*2)) );	/*				*/
																	/*								*/
		ac_flg.syusyu = 37;											/* 37:���ɏW�v�ر����			*/
																	/*								*/
		CountUp( COIN_SAFE_COUNT );									/* ��݋��ɒǔ�+1				*/
																	/*								*/
	}																/*								*/
	else{															/*								*/
		ac_flg.syusyu = 42;											/* 42:�󎚊���					*/
		memcpy( ws, ts, sizeof( sky.tsyuk ) );						/* �s�W�v��ܰ��ر�ցi��d�΍�j	*/
		ac_flg.syusyu = 43;											/* 43:�s�W�v��ܰ��ر�֓]������	*/
																	/*								*/
		ts->Kinko_Tryo += note_syu.Tryo;							/* �s�W�v�։��Z					*/
		ts->Note_Tryo += note_syu.Tryo;								/* �s�W�v�։��Z					*/
		for( i = 0; i < 4; i++ ){									/*								*/
			ts->Note_cnt[i] += note_syu.cnt[i];						/*								*/
			ts->Note_ryo[i] += note_syu.ryo[i];						/*								*/
		}															/*								*/
		ts->Skinko_goukei_cnt += 1;									/* �������ɍ��v��				*/
																	/*								*/
		memcpy( nbk, &note_syu, sizeof( note_syu ) );				/* �������ɍ��v��O�񎆕����ɍ��v�־��*/
		ac_flg.syusyu = 44;											/* 44:���ɏW�v���s�W�v�֓]������*/
																	/*								*/
		Log_regist( LOG_NOTEBOX );									/* �������ɏW�v�����Z			*/
																	/*								*/
																	/*								*/
		SFV_DAT.nt_safe_dt = 0;										/*								*/
		memcpy( &note_syu.OldTime,									/*								*/
				&note_syu.NowTime, sizeof( date_time_rec ) );		/* �O��W�v�������				*/
		memset( &note_syu.Tryo, 0, sizeof(ulong) * (1+(SIHEI_SYU_CNT*2)) );	/*			*/
																	/*								*/
		ac_flg.syusyu = 47;											/* 47:���ɏW�v�ر����			*/
																	/*								*/
		CountUp( NOTE_SAFE_COUNT );									/* �������ɒǔ�+1				*/
																	/*								*/
	}																/*								*/
																	/*								*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �W�v�ر������																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: syu_init( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-12-12																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	syu_init( void )											/*								*/
{																	/*								*/
																	/*								*/
	memset( &sky, 0, sizeof( struct SKY ) );						/*								*/
	memset( &skybk, 0, sizeof( struct SKY ) );						/*								*/
	memset( &turi_kan, 0, sizeof( TURI_KAN ) );						/*								*/
	memset( &turi_dat, 0, sizeof( struct turi_dat_rec ) );			/*								*/
	memset( &fusei, 0, sizeof( struct FUSEI_SD ) );					/*								*/
	memset( &coin_syu, 0, sizeof( COIN_SYU ) );						/*								*/
	memset( &note_syu, 0, sizeof( NOTE_SYU ) );						/*								*/
	memset( &loktl, 0, sizeof( struct LOKTL ) );					/*								*/
	memset( &loktlbk, 0, sizeof( struct LOKTL ) );					/*								*/
	memset( &cobk_syu, 0, sizeof( cobk_syu ) );						/*								*/
	memset( &nobk_syu, 0, sizeof( nobk_syu ) );						/*								*/
	memset( &locktimeout, 0, sizeof( struct FUSEI_SD ) );			/*								*/
																	/*								*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| DISCOUNT_DATA �� wari_tiket �����f�[�^�ϊ�													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: disc_wari_conv( void )														   |*/
/*| PARAMETER	: index	: 0 = PayData.DiscountData�C���f�b�N�X�l								   |*/
/*| 			  pdata	: 1 = wari_tiket�^�f�[�^�|�C���^										   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: m.nagashima																	   |*/
/*| Date		: 2012-02-09																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	disc_wari_conv( DISCOUNT_DATA *p_disc, wari_tiket *p_wari )
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
{
	disc_wari_conv_main(p_disc, p_wari, 0);	// ����g�p����������Ԃ�
}

void	disc_wari_conv_prev( DISCOUNT_DATA *p_disc, wari_tiket *p_wari )
{
	disc_wari_conv_main(p_disc, p_wari, 1);	// �O�񐸎Z�܂ł̎g�p�ς݊�����Ԃ�
}

void	disc_wari_conv_all( DISCOUNT_DATA *p_disc, wari_tiket *p_wari )
{
	disc_wari_conv_main(p_disc, p_wari, 2);	// �S�Ă̊�����Ԃ�
}

// GG124100(S) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�
void	disc_wari_conv_new( DISCOUNT_DATA *p_disc, wari_tiket *p_wari )
{
	disc_wari_conv_main(p_disc, p_wari, 3);	// ���g�p������Ԃ�
}
// GG124100(E) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�

//[]----------------------------------------------------------------------[]
///	@brief			DISCOUNT_DATA �� wari_tiket �����f�[�^�ϊ�(����w��)
//[]----------------------------------------------------------------------[]
///	@param[in]		DISCOUNT_DATA	: PayData.DiscountData�|�C���^
///	@param[in]		wari_tiket		: wari_tiket�^�f�[�^�|�C���^
///	@param[in]		uchar			: ����t���O<br>
///									  0 = ����g�p����������Ԃ�<br>
///									  1 = �O�񐸎Z�܂ł̎g�p�ς݊�����Ԃ�<br>
///									  2 = �S�Ă̊�����Ԃ�<br>
// GG124100(S) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�
///									  3 = ���g�p������Ԃ�<br>
// GG124100(E) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�
///	@return			void
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void disc_wari_conv_main( DISCOUNT_DATA *p_disc, wari_tiket *p_wari, uchar opeFlag )
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
{
	uchar	i;
	uchar	tik_syu;
	ulong	ParkingNo;
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	ulong	tmpDiscount = 0;
	ushort	tmpDiscCount = 0;
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]

	memset( p_wari, 0, sizeof( wari_tiket ) );

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	switch ( opeFlag ) {
	case 1:	// �O�񐸎Z�܂ł̎g�p�ς݊�����Ԃ�
		tmpDiscCount = p_disc->uDiscData.common.PrevUsageDiscCount;
		tmpDiscount  = p_disc->uDiscData.common.PrevUsageDiscount;
		tmpDiscount += p_disc->uDiscData.common.PrevDiscount;
		break;
	case 2:	// �S�Ă̊�����Ԃ�
		tmpDiscCount  = p_disc->DiscCount;
		tmpDiscCount += p_disc->uDiscData.common.PrevUsageDiscCount;
		tmpDiscount  = p_disc->Discount;
		tmpDiscount += p_disc->uDiscData.common.PrevUsageDiscount;
		break;
// GG124100(S) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�
// 	case 0:	// ����g�p����������Ԃ�
// 	default:
	case 3:	// �V�K�̊�����Ԃ�
		// �O�񐸎Z�܂ł̊����̖��g�p���͊܂߂Ȃ�
// GG124100(E) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�
		tmpDiscCount = p_disc->DiscCount;
		tmpDiscount  = p_disc->Discount;
		tmpDiscount -= p_disc->uDiscData.common.PrevDiscount;
		break;
// GG124100(S) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�
	case 0:	// ����g�p����������Ԃ�
	default:
		tmpDiscCount = p_disc->DiscCount;
		tmpDiscount  = p_disc->Discount;
		break;
// GG124100(E) R.Endo 2022/09/30 �Ԕԃ`�P�b�g���X3.0 #6631 �Đ��Z���ɁA�O�񐸎Z�Ŏg�p���������̒ǉ��̊��������W�v�ɔ��f����Ȃ�
	}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]

	switch( p_disc->DiscSyu ){
		case NTNET_SVS_T:			// ���޽��(����)
		case NTNET_SVS_M:			// ���޽��(����)
// MH810100(S) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�����ςݑΉ�/�Đ��Z_�̎��؈�)
//			tik_syu = SERVICE;
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 			if( p_disc->uDiscData.common.DiscFlg == 0 ){
// 				tik_syu = SERVICE;
// 			}else{
// 				tik_syu = C_SERVICE;
// 			}
			if ( tmpDiscount || tmpDiscCount ) {
				if ( opeFlag == 1 ) {
					tik_syu = C_SERVICE;
				} else {
					tik_syu = SERVICE;
				}
			} else {
				return;	// �f�[�^�Ȃ�
			}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(E) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�����ςݑΉ�/�Đ��Z_�̎��؈�)
			break;
		case NTNET_KAK_T:			// �X����(����)
		case NTNET_KAK_M:			// �X����(����)
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����f�[�^�ϊ��̕ύX)
		case NTNET_TKAK_M:			// ���X��(����)
		case NTNET_TKAK_T:			// ���X��(����)
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����f�[�^�ϊ��̕ύX)
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
//// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
//		case NTNET_SHOP_DISC_AMT:	// ��������(���z)
//		case NTNET_SHOP_DISC_TIME:	// ��������(����)
//// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
// MH810100(S) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�����ςݑΉ�/�Đ��Z_�̎��؈�)
//			tik_syu = KAKEURI;
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 			if( p_disc->uDiscData.common.DiscFlg == 0 ){
// 				tik_syu = KAKEURI;
// 			}else{
// 				tik_syu = C_KAKEURI;
// 			}
			if ( tmpDiscount || tmpDiscCount ) {
				if ( opeFlag == 1 ) {
					tik_syu = C_KAKEURI;
				} else {
					tik_syu = KAKEURI;
				}
			} else {
				return;	// �f�[�^�Ȃ�
			}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// MH810100(E) K.Onodera  2020/01/29 �Ԕԃ`�P�b�g���X(�����ςݑΉ�/�Đ��Z_�̎��؈�)
			break;
		case NTNET_FRE:				// �񐔌�
			tik_syu = KAISUU;
			break;
		case NTNET_WRI_M:			// ������(����)
		case NTNET_WRI_T:			// ������(����)
			tik_syu = WARIBIKI;
			break;
		case NTNET_PRI_W:			// �v���y�C�h�J�[�h(��ʁ^�����͎Q�ƕs��)
			tik_syu = PREPAID;
			break;
		case NTNET_GENGAKU:			// ���z���Z
			tik_syu = MISHUU;
			break;
		case NTNET_FURIKAE:			// �U�֐��Z
			tik_syu = FURIKAE;
			break;
		case NTNET_SYUSEI_1:		// �C�����Z
			tik_syu = SYUUSEI;
			break;
		case NTNET_CSVS_M:			// ���Z���~���޽��(����)
		case NTNET_CSVS_T:			// ���Z���~���޽��(����)
			tik_syu = C_SERVICE;
			break;
		case NTNET_CKAK_M:			// ���Z���~�X����(����)
		case NTNET_CKAK_T:			// ���Z���~�X����(����)
			tik_syu = C_KAKEURI;
			break;
		case NTNET_CWRI_M:			// ���Z���~������(����)
		case NTNET_CWRI_T:			// ���Z���~������(����)
			tik_syu = C_WARIBIKI;
			break;
		case NTNET_CFRE:			// ���Z���~�񐔌�
			tik_syu = C_KAISUU;
			break;
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
		case NTNET_SHOP_DISC_AMT:	// ��������(���z)
		case NTNET_SHOP_DISC_TIME:	// ��������(����)
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 			if( p_disc->uDiscData.common.DiscFlg == 0 ){
// 				tik_syu = SHOPPING;
// 			}else{
// 				tik_syu = C_SHOPPING;
// 			}
			if ( tmpDiscount || tmpDiscCount ) {
				if ( opeFlag == 1 ) {
					tik_syu = C_SHOPPING;
				} else {
					tik_syu = SHOPPING;
				}
			} else {
				return;	// �f�[�^�Ȃ�
			}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
			break;
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//		default :
//			return;
// GG129000(S) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]
		case NTNET_SYUBET:		// ��ʊ���(���z)
		case NTNET_SYUBET_TIME:	// ��ʊ���(����)
			if ( tmpDiscount || tmpDiscCount ) {
				if ( opeFlag == 1 ) {
					tik_syu = C_SYUBETU;
				} else {
					tik_syu = SYUBETU;
				}
			} else {
				return;	// �f�[�^�Ȃ�
			}
			break;
// GG129000(E) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]
		case	0:					// �f�[�^�Ȃ�
			return;
		default :					// �s��
			tik_syu = INVALID;
			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
	}

	ParkingNo = p_disc->ParkingNo;
	for( i = 0; i < 4; i++ ){
		if( ParkingNo == CPrmSS[S_SYS][i+1] ){
			break;
		}
	}
	if( i > 3 ){
		i = 0;
	}
	p_wari->pkno_syu = i;						// ���ԏ�m���D���
	p_wari->tik_syu = tik_syu;					// ���������
// MH810104(S) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����f�[�^�ϊ��̕ύX)
// // MH322914(S) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
// 	p_wari->syubetu = (uchar)p_disc->DiscNo;	// ���
// 	p_wari->syubetu = p_disc->DiscNo;			// ���
// // MH322914(E) K.Onodera 2017/03/08 AI-V�Ή��F���u���Z����
	p_wari->syubetu = p_disc->DiscNo;			// ���(�X�����i�[)
	if ( (p_disc->DiscSyu == NTNET_TKAK_M) || (p_disc->DiscSyu == NTNET_TKAK_T) ) {
		p_wari->mise_no = p_disc->DiscInfo1;	// �X�m���D(���X�܊�����ʂ��i�[)
	}
// MH810104(E) R.Endo 2021/08/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����f�[�^�ϊ��̕ύX)
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
// 	p_wari->maisuu = (uchar)p_disc->DiscCount;	// ����
// 	p_wari->ryokin = p_disc->Discount;			// ��������
	p_wari->maisuu = (uchar)tmpDiscCount;				// ����
	p_wari->ryokin = tmpDiscount;				// ��������
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6465 ����10��Ή� [���ʉ��P���� No1526]
	if( tik_syu == SYUUSEI ){
		// �C�����Z(�U�֊z)�͊����G���A2��Discount
		p_wari->ryokin = (p_disc+1)->Discount;
	}
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
	if(tik_syu == SHOPPING || tik_syu == C_SHOPPING) {
		// ���������̊����K�pNo.��ݒ�
		p_wari->minute = p_disc->DiscInfo1;
	}
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)

	return;
}
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
void	disc_Detail_conv( DETAIL_DATA *p_disc, wari_tiket *p_wari )
{
	uchar	i;
	uchar	tik_syu;
	ulong	ParkingNo;

	memset( p_wari, 0, sizeof( wari_tiket ) );
	switch( p_disc->DiscSyu ){
		case NTNET_FURIKAE_2:		// �U�֐��Z
			tik_syu = FURIKAE2;
			break;
		case NTNET_FURIKAE_DETAIL:
			tik_syu = FURIKAE_DETAIL;
			break;
		default:
			return;
	}

	ParkingNo = p_disc->ParkingNo;
	for( i = 0; i < 4; i++ ){
		if( ParkingNo == CPrmSS[S_SYS][i+1] ){
			break;
		}
	}
	if( i > 3 ){
		i = 0;
	}
	p_wari->pkno_syu = i;										// ���ԏ�m���D���
	p_wari->tik_syu = tik_syu;									// ���������
	p_wari->syubetu = (uchar)p_disc->uDetail.Common.DiscNo;		// ���
	p_wari->maisuu = (uchar)p_disc->uDetail.Common.DiscCount;	// ����
	p_wari->ryokin = p_disc->uDetail.Common.DiscInfo1;			// ��������

	return;
}
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
//[]----------------------------------------------------------------------[]
///	@brief		���A���^�C�����ł̓��t�ؑ֊�̑�����̏��v�擾�@�\�̎g�p��
//[]----------------------------------------------------------------------[]
///	@return		ret : 0:�g�p����  1:�g�p���Ȃ�
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/08/30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
uchar	date_uriage_use_chk( void )
{
	uchar	hour;
	uchar	min;
	uchar	ret;

	ret = 0;// ���t�ؑ֊�̑�����̏��v�擾���ŏ��ݒ�Ő݂��Ă��������ƕ��w�E�ŏ����Œ�(����)�ɕύX
	hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// ����� ��
	min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// ����� ��

	if((23 < hour) || (59 < min)){
		ret = 1;//�ݒ�l���s�����g�p���Ȃ�
	}

	return( ret );
}

//[]----------------------------------------------------------------------[]
///	@brief		���A���^�C�����ł̓��t�ؑ֊�̏��v�̎����̍X�V����
//[]----------------------------------------------------------------------[]
///	@return      ret : DATE_URIAGE_NO_CHANGE     �͈͓��i�X�V�s�v�j 
///	                   DATE_URIAGE_CLR_TODAY     �͈͊O�i�X�V�K�v�F�i�񂾏ꍇ�j�������̊�����Z�b�g+�W�v���N���A
///	                   DATE_URIAGE_CLR_BEFORE    �͈͊O�i�X�V�K�v�F�߂����ꍇ�j���O���̊�����Z�b�g+�W�v���N���A
///	                   DATE_URIAGE_PRMCNG_TODAY  �ݒ�ύX������X�V�i�����X�V�j�� �����̊�����Z�b�g(�W�v�N���A���Ȃ�)
///	                   DATE_URIAGE_PRMCNG_BEFORE �ݒ�ύX������X�V�i�O���X�V�j�� �O���̊�����Z�b�g(�W�v�N���A���Ȃ�)
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/08/30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
uchar	date_uriage_update_chk( void )
{
	uchar	update = 0;
	short	ck_year,ck_mon,ck_day,ck_hour,ck_min;
	uchar	settei_Hour,settei_Min,ck_flg;

// �{�֐����R�[������O�ɐݒ�l�̔�����s���Ă���̂ł����ł͍s��Ȃ�
	settei_Hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// ����� ��
	settei_Min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// ����� ��
	ck_flg = DATE_URIAGE_PRMCNG_NOCLR;	// ������ύX���̏W�v�N���A(���Ȃ��Œ�) �ŏ��ݒ�Ő݂��Ă��������ƕ��w�E�ŏ����Œ�ɕύX
	
// �^�p���ɐݒ�ύX���ꂽ���͊���݂̂��X�V����
	if( (Date_Syoukei.Date_Time.Hour != settei_Hour) || (Date_Syoukei.Date_Time.Min != settei_Min) ){
		if(CLK_REC.hour <  settei_Hour){
			if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//������ύX���W�v�N���A���Ȃ�
				update = DATE_URIAGE_PRMCNG_BEFORE;		// �W�v�N���A���Ȃ��ŁA������X�V�i�O���X�V�j
			}else{
				update = DATE_URIAGE_CLR_BEFORE;		// �W�v�N���A���A������X�V�i�O���X�V�j
			}
		}else if(CLK_REC.hour ==  settei_Hour){
			// ����v
			if(CLK_REC.minu < settei_Min){
				if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//������ύX���W�v�N���A���Ȃ�
					update = DATE_URIAGE_PRMCNG_BEFORE;	// �W�v�N���A���Ȃ��ŁA������X�V�i�O���X�V�j
				}else{
					update = DATE_URIAGE_CLR_BEFORE;	// �W�v�N���A���A������X�V�i�O���X�V�j
				}
			}else{
				if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//������ύX���W�v�N���A���Ȃ�
					update = DATE_URIAGE_PRMCNG_TODAY;	// �W�v�N���A���Ȃ��ŁA������X�V�i�����X�V�j
				}else{
					update = DATE_URIAGE_CLR_TODAY;		// �W�v�N���A���A������X�V�i�����X�V�j
				}
			}
		}else{
			if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//������ύX���W�v�N���A���Ȃ�
				update = DATE_URIAGE_PRMCNG_TODAY;		// �W�v�N���A���Ȃ��ŁA������X�V�i�����X�V�j
			}else{
				update = DATE_URIAGE_CLR_TODAY;			// �W�v�N���A���A������X�V�i�����X�V�j
			}
		}
		return( update );
	}

// ���ݎ������X�V������O�̔���i�����ύX�ōX�V������O�ɖ߂��ꂽ���j
	if(CLK_REC.year < Date_Syoukei.Date_Time.Year){
		update = DATE_URIAGE_CLR_TODAY;// �ߋ��̂��ߍX�V
	}else if(CLK_REC.year == Date_Syoukei.Date_Time.Year){
		// �N��v
		if(CLK_REC.mont < Date_Syoukei.Date_Time.Mon){
			update = DATE_URIAGE_CLR_TODAY;// �ߋ��̂��ߍX�V
		}else if(CLK_REC.mont == Date_Syoukei.Date_Time.Mon){
			// ����v
			if(CLK_REC.date < Date_Syoukei.Date_Time.Day){
				update = DATE_URIAGE_CLR_TODAY;// �ߋ��̂��ߍX�V
			}else if(CLK_REC.date == Date_Syoukei.Date_Time.Day){
				// ����v
				if(CLK_REC.hour < Date_Syoukei.Date_Time.Hour){
					update = DATE_URIAGE_CLR_BEFORE;// �ߋ��̂��ߍX�V�i�O���X�V�j
				}else if(CLK_REC.hour == Date_Syoukei.Date_Time.Hour){
					// ����v
					if(CLK_REC.minu < Date_Syoukei.Date_Time.Min){
						update = DATE_URIAGE_CLR_BEFORE;// �ߋ��̂��ߍX�V�i�O���X�V�j
					}
				}
			}
		}
	}
	
	if(update == DATE_URIAGE_CLR_TODAY){
		// �N�^���^���̂����ꂩ���ߋ��̏ꍇ�ł��A���^���̒l�ɂ���ē������O���̂ǂ���ɍX�V���邩����K�v
		// ���^�����ߋ��̏ꍇ�͑O���X�V�ł���ȊO�͓����X�V
		if(CLK_REC.hour < Date_Syoukei.Date_Time.Hour){
				update = DATE_URIAGE_CLR_BEFORE;// �ߋ��̂��ߍX�V�i�O���X�V�j
		}else if(CLK_REC.hour == Date_Syoukei.Date_Time.Hour){
			// ����v
			if(CLK_REC.minu < Date_Syoukei.Date_Time.Min){
					update = DATE_URIAGE_CLR_BEFORE;// �ߋ��̂��ߍX�V�i�O���X�V�j
			}
		}
	}
	if(update != DATE_URIAGE_NO_CHANGE){
		return( update );// �X�V������O�ɖ߂���Ă��邱�Ƃɂ��͈͊O
	}

// ���ݎ������X�V����ɑ΂���24���Ԉȏ�o�߂������`�F�b�N
	ck_year = Date_Syoukei.Date_Time.Year;
	ck_mon =  Date_Syoukei.Date_Time.Mon;
	ck_day =  Date_Syoukei.Date_Time.Day;
	ck_hour = Date_Syoukei.Date_Time.Hour;
	ck_min =  Date_Syoukei.Date_Time.Min;

	ck_hour += 24;//�X�V�����24���ԉ��Z
	// 24h���Z�������Ƃɂ��␳����
	if(ck_hour >= 24){
		ck_day++;//1�����Z
		ck_hour = ck_hour % 24;
		for ( ; ; ) {
			if (ck_day <= medget(ck_year, ck_mon)) {// �����ȉ�
				break;
			}
			// �Y�����̓����������Č������Z
			ck_day -= medget(ck_year, ck_mon);
			if (++ck_mon > 12) {
				ck_mon = 1;
				ck_year++;
			}
		}
	}

	if(CLK_REC.year > ck_year){
		update = DATE_URIAGE_CLR_BEFORE;// �X�V������B�i�����^�O������L��j
	}else if(CLK_REC.year == ck_year){
		// �N��v
		if(CLK_REC.mont > ck_mon){
			update = DATE_URIAGE_CLR_BEFORE;// �X�V������B�i�����^�O������L��j
		}else if(CLK_REC.mont == ck_mon){
			// ����v
			if(CLK_REC.date > ck_day){
				update = DATE_URIAGE_CLR_BEFORE;// �X�V������B�i�����^�O������L��j
			}else if(CLK_REC.date == ck_day){
				// ����v
				if(CLK_REC.hour > ck_hour){
					update = DATE_URIAGE_CLR_TODAY;// �X�V������B�i�����X�V�m��j
				}else if(CLK_REC.hour == ck_hour){
					// ����v
					if(CLK_REC.minu >= ck_min){
						update = DATE_URIAGE_CLR_TODAY;// �X�V������B�i�����X�V�m��j
					}
				}
			}
		}
	}
	
	if(update == DATE_URIAGE_CLR_BEFORE){
		// �N�^���^���̂����ꂩ�����B���Ă��A���^���̒l�ɂ���ē������O���̂ǂ���ɍX�V���邩����K�v
		// ���^�����ߋ��̏ꍇ�͑O���X�V�ł���ȊO�͓����X�V
		if(CLK_REC.hour <  ck_hour){
				update = DATE_URIAGE_CLR_BEFORE;// �ߋ��̂��ߍX�V�i�O���X�V�j
		}else if(CLK_REC.hour ==  ck_hour){
			// ����v
			if(CLK_REC.minu < ck_min){
				update = DATE_URIAGE_CLR_BEFORE;// �ߋ��̂��ߍX�V�i�O���X�V�j
			}else{
				update = DATE_URIAGE_CLR_TODAY;// �����X�V
			}
		}else{
			update = DATE_URIAGE_CLR_TODAY;// �����X�V
		}
	}
	return( update );
}
//[]----------------------------------------------------------------------[]
///	@brief		���A���^�C�����ł̓��t�ؑ֊���v�̎����̍X�V����
//[]----------------------------------------------------------------------[]
///	@return	
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/09/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	date_uriage_syoukei_judge( void )
{
	uchar	flg;
	if(date_uriage_use_chk() != 0){// ���t�ؑ֊�̑�����̏��v�擾���Ȃ�
		return;
	}
	
	flg = date_uriage_update_chk();
	
	date_uriage_syoukei_clr(flg);// �W�v���̃N���A�A�ݒ�̕ύX
}

//[]----------------------------------------------------------------------[]
///	@brief		���A���^�C�����ł̓��t�ؑ֊�̎����̍X�V����
//[]----------------------------------------------------------------------[]
///	@param[in]  uchar flg: DATE_URIAGE_NO_CHANGE �X�V�s�v
///	                       DATE_URIAGE_CLR_TODAY:�����̊�����Z�b�g+�W�v���N���A
///	                       DATE_URIAGE_CLR_BEFORE:�O���̊�����Z�b�g+�W�v���N���A
///	                       DATE_URIAGE_PRMCNG_TODAY:�����̊�����Z�b�g(�W�v�N���A���Ȃ�)
///	                       DATE_URIAGE_PRMCNG_BEFORE:�O���̊�����Z�b�g(�W�v�N���A���Ȃ�)
///	@return	
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/08/30<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	date_uriage_syoukei_clr( uchar flg)
{
	uchar	hour;
	uchar	min;
	ushort	us_wk;
	short	s_wky, s_wkm, s_wkd;

	// �{�֐����R�[������O�ɐݒ�l�̔�����s���Ă���̂ł����ł͍s��Ȃ�
	hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// ����� ��
	min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// ����� ��

	if((flg == DATE_URIAGE_CLR_TODAY) || (flg == DATE_URIAGE_PRMCNG_TODAY)){// �����̊�����Z�b�g
		if(flg == DATE_URIAGE_CLR_TODAY){
			memset( &Date_Syoukei, 0, sizeof( Date_Syoukei ) );//�W�v�����N���A
		}
		Date_Syoukei.Date_Time.Year = CLK_REC.year;
		Date_Syoukei.Date_Time.Mon = CLK_REC.mont;
		Date_Syoukei.Date_Time.Day = CLK_REC.date;
		Date_Syoukei.Date_Time.Hour = hour;
		Date_Syoukei.Date_Time.Min = min;
	}else if((flg == DATE_URIAGE_CLR_BEFORE) || (flg == DATE_URIAGE_PRMCNG_BEFORE)){// �O���̊�����Z�b�g
		if(flg == DATE_URIAGE_CLR_BEFORE){
			memset( &Date_Syoukei, 0, sizeof( Date_Syoukei ) );//�W�v�����N���A
		}
		us_wk = dnrmlzm((short)CLK_REC.year, (short)CLK_REC.mont, (short)CLK_REC.date );// �N�����m�[�}���C�Y
		us_wk--;// -1���X�V
		idnrmlzm( us_wk, &s_wky, &s_wkm, &s_wkd );// �t�m�[�}���C�Y��1���X�V�������t�����߂�
		Date_Syoukei.Date_Time.Year = (ushort)s_wky;
		Date_Syoukei.Date_Time.Mon  = (uchar)s_wkm;
		Date_Syoukei.Date_Time.Day  = (uchar)s_wkd;
		Date_Syoukei.Date_Time.Hour = hour;
		Date_Syoukei.Date_Time.Min = min;
	}else{
		//DATE_URIAGE_NO_CHANGE�̎��͉������Ȃ�
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		���A���^�C�����ł̓��t�ؑ֊�̏��v�̎����Ɛ��Z�����̔�r
//[]----------------------------------------------------------------------[]
///	@return		ret : 0:�͈͓��i���Z�j  1:�͈͊O�i���Z�s�j
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/09/13<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
uchar	date_uriage_cmp_paymentdate( void )
{
	uchar	update = 0;
// ���t�ؑ֊�����O�ɐ��Z���J�n�������A���Z�������_�œ��t�ؑ֊�������߂��Ă����ꍇ
// ���t�ؑ֊�̏��v�ɉ��Z�����Ȃ����߂̔���

// ���Z�����Ɠ��t�ؑ֊�������r����
	if(car_ot.year < Date_Syoukei.Date_Time.Year){
		update = 1;// �ߋ��̂��߉��Z�s��
	}else if(CLK_REC.year == Date_Syoukei.Date_Time.Year){
		// �N��v
		if(car_ot.mon < Date_Syoukei.Date_Time.Mon){
			update = 1;// �ߋ��̂��߉��Z�s��
		}else if(car_ot.mon == Date_Syoukei.Date_Time.Mon){
			// ����v
			if(car_ot.day < Date_Syoukei.Date_Time.Day){
				update = 1;// �ߋ��̂��߉��Z�s��
			}else if(car_ot.day == Date_Syoukei.Date_Time.Day){
				// ����v
				if(car_ot.hour < Date_Syoukei.Date_Time.Hour){
					update = 1;// �ߋ��̂��߉��Z�s��
				}else if(car_ot.hour == Date_Syoukei.Date_Time.Hour){
					// ����v
					if(car_ot.min < Date_Syoukei.Date_Time.Min){
						update = 1;// �ߋ��̂��߉��Z�s��
					}
				}
			}
		}
	}
	return( update );
}
//[]----------------------------------------------------------------------[]
///	@brief		���A���^�C�����ł̓��t�ؑ֊�����̐ݒ�ύX���s��ꂽ�Ƃ��̐ݒ�X�V����
//[]----------------------------------------------------------------------[]
///	@return	
///	@author		A.Iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2018/12/05<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	date_uriage_prmcng_judge( void )
{
	uchar	update = DATE_URIAGE_NO_CHANGE;
	uchar	settei_Hour,settei_Min,ck_flg;
	if(date_uriage_use_chk() != 0){// ���t�ؑ֊�̑�����̏��v�擾���Ȃ�
		return;
	}

	settei_Hour = (uchar)prm_get( COM_PRM,S_CEN,97,2,3 );	// ����� ��
	settei_Min = (uchar)prm_get( COM_PRM,S_CEN,97,2,1 );	// ����� ��
	ck_flg = DATE_URIAGE_PRMCNG_NOCLR;	// ������ύX���̏W�v�N���A(���Ȃ��Œ�) �ŏ��ݒ�Ő݂��Ă��������ƕ��w�E�ŏ����Œ�ɕύX

	// �^�p���ɐݒ�ύX���ꂽ���̊���X�V����
	// �u������ύX���W�v�N���A���Ȃ��v�ݒ�̏ꍇ�A�ݒ�ύX�������_�Ŋ�������X�V���Ȃ���
	// �ݒ�ύX��A�����Ԑ��Z���������Ȃ���ԂŐݒ�ύX��̊�����ɓ��B�����ꍇ�A�W�v�N���A������Ȃ����߂̑Ή�
	if(ck_flg == DATE_URIAGE_PRMCNG_NOCLR){//������ύX���W�v�N���A���Ȃ�
		// �ݒ肪�ύX����Ă����ꍇ�͂��̎��_�Ŋ�������X�V����
		if( (Date_Syoukei.Date_Time.Hour != settei_Hour) || (Date_Syoukei.Date_Time.Min != settei_Min) ){
			if(CLK_REC.hour <  settei_Hour){
				update = DATE_URIAGE_PRMCNG_BEFORE;		// �W�v�N���A���Ȃ��ŁA������X�V�i�O���X�V�j
			}else if(CLK_REC.hour ==  settei_Hour){
				// ����v
				if(CLK_REC.minu < settei_Min){
					update = DATE_URIAGE_PRMCNG_BEFORE;	// �W�v�N���A���Ȃ��ŁA������X�V�i�O���X�V�j
				}else{
					update = DATE_URIAGE_PRMCNG_TODAY;	// �W�v�N���A���Ȃ��ŁA������X�V�i�����X�V�j
				}
			}else{
				update = DATE_URIAGE_PRMCNG_TODAY;		// �W�v�N���A���Ȃ��ŁA������X�V�i�����X�V�j
			}
			date_uriage_syoukei_clr(update);// ������̍X�V
		}
	}

}
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
//// MH321800(S) G.So IC�N���W�b�g�Ή�
///*[]----------------------------------------------------------------------------------------------[]*/
///*| �A���[������o�^																			   |*/
///*[]----------------------------------------------------------------------------------------------[]*/
///*| MODULE NAME	: ec_alarm_syuu( uchar brand, ulong ryo )										   |*/
///*| PARAMETER	: brand	: �g�p�u�����h															   |*/
///*| 			  ryo	: �A���[������z														   |*/
///*| RETURN VALUE: void	: 																		   |*/
///*[]----------------------------------------------------------------------------------------------[]*/
///*| Author		: G.So																			   |*/
///*| Date		: 2019-01-29																	   |*/
///*| Update		:																				   |*/
///*[]------------------------------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
//void	ec_alarm_syuu( uchar brand, ulong ryo )
//{
//	SYUKEI	*ts, *ws;
//// MH810103 GG119202(S) �A���[������񐔂͓����ɃJ�E���g����
////	struct clk_rec wrk_clk_rec;		/* �W�v�����m�[�}���C�Y�p */
////	date_time_rec wrk_date;			/* �W�v�����m�[�}���C�Y�p */
//// MH810103 GG119202(E) �A���[������񐔂͓����ɃJ�E���g����
//// MH810105 GG119202(S) ������������W�v�d�l���P
////	int		ec_kind;
//// MH810105 GG119202(E) ������������W�v�d�l���P
//// MH810103 GG119202(S) �A���[������񐔂͓����ɃJ�E���g����
////	int		syu_idx;				/* �����W�v�̃C���f�b�N�X */
////	ulong	sei_date;				/* ���Z���� */
////	ulong	sy1_date;				/* �W�v���� */
////	memset(&wrk_clk_rec, 0x0, sizeof(wrk_clk_rec));
//// MH810103 GG119202(E) �A���[������񐔂͓����ɃJ�E���g����
//
//	EcAlarm.Ryo = ryo;											// ���d�p�A���[������z
//	EcAlarm.Brand = brand;										// ���d�p�A���[������u�����h
//
//	ac_flg.ec_alarm = 1;										/* 1:�A���[���������ۑ����� */
//
//	ts = &sky.tsyuk;												/*								*/
//	ws = &wksky;													/*								*/
//	memcpy( ws, ts, sizeof( sky.tsyuk ) );							/* �s�W�v��ܰ��ر��(��d�΍�)	*/
//
//	DailyAggregateDataBKorRES( 0 );									/* �����W�v�G���A�̃o�b�N�A�b�v	*/
//
//	ac_flg.ec_alarm = 2;											/* 2:�W�v��ܰ��ر�֓]������	*/
//
//// MH810105 GG119202(S) ������������W�v�d�l���P
////// �W�v�͊����G���A������΂�����g�p����̂ŁA���ꂼ��ɉ��Z����B
////	switch(brand) {
////	case EC_EDY_USED:
////		ts->Electron_Arm_cnt += 1L;									// ���Z��������
////		ts->Electron_Arm_ryo += ryo;								//     �����z��
////		break;
////	case EC_NANACO_USED:
////		ts->nanaco_alm_cnt += 1L;									// ���Z��������
////		ts->nanaco_alm_ryo += ryo;									//     �����z��
////		break;
////	case EC_WAON_USED:
////		ts->waon_alm_cnt += 1L;										// ���Z��������
////		ts->waon_alm_ryo += ryo;									//     �����z��
////		break;
////	case EC_SAPICA_USED:
////		ts->sapica_alm_cnt += 1L;									// ���Z��������
////		ts->sapica_alm_ryo += ryo;									//     �����z��
////		break;
////	case EC_KOUTSUU_USED:
////		ts->koutsuu_alm_cnt += 1L;									// ���Z��������
////		ts->koutsuu_alm_ryo += ryo;									//     �����z��
////		break;
////	case EC_ID_USED:
////		ts->id_alm_cnt += 1L;										// ���Z��������
////		ts->id_alm_ryo += ryo;										//     �����z��
////		break;
////	case EC_QUIC_PAY_USED:
////		ts->quicpay_alm_cnt += 1L;									// ���Z��������
////		ts->quicpay_alm_ryo += ryo;									//     �����z��
////		break;
////	case EC_CREDIT_USED:
////		// �N���W�b�g�ŃA���[������͖���
////		break;
////	default:
////		// ���肦�Ȃ�
////		ac_flg.ec_alarm = 0;
////		return;
////	}
//	// ������������i�x���s���j�W�v
//	ts->miryo_unknown_cnt += 1L;
//	ts->miryo_unknown_ryo += ryo;
//
//	// ���ό��ʃf�[�^�i���ϖ����m��j�ɂ͌��ϓ������Z�b�g����Ȃ����߁A�����W�v�͓����Ƃ��ăJ�E���g����
//	Syuukei_sp.ec_inf.now.sp_miryo_unknown_cnt += 1L;
//	Syuukei_sp.ec_inf.now.sp_miryo_unknown_ryo += ryo;
//// MH810105 GG119202(E) ������������W�v�d�l���P
//// MH810103 GG119202(S) �A���[������񐔂͓����ɃJ�E���g����
////	// �ŐV�̒ʒm��������W�v�������Z�o
////	memcpy(&wrk_date, &Syuukei_sp.ec_inf.NowTime, sizeof(date_time_rec));
////	wrk_date.Hour = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 3);			/* ���͋��ʃp�����[�^����擾 */
////	wrk_date.Min = (uchar)prm_get(COM_PRM, S_ECR, 3, 2, 1);				/* ���͋��ʃp�����[�^����擾 */
////	sy1_date = Nrm_YMDHM(&wrk_date);									/* �W�v�������m�[�}���C�Y */
////
////	// ���Z�f�[�^����i���ό��ʃf�[�^�Ŏ�M�����j���Z�������Z�o
////	c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &wrk_clk_rec);
////	memcpy(&wrk_date, &wrk_clk_rec, sizeof(date_time_rec));
////	sei_date = Nrm_YMDHM(&wrk_date);									/* ���Z�������m�[�}���C�Y */
////
////	ec_kind = brand - EC_EDY_USED;
////	if(sei_date >= sy1_date && sei_date < (sy1_date + 0x10000)) {
////		// ���Z�����������̏W�v�����͈�
////		Syuukei_sp.ec_inf.now.cnt2[ec_kind] += 1L;						/* �g�p��				*/
////		Syuukei_sp.ec_inf.now.ryo2[ec_kind] += ryo;						/* �g�p���z				*/
////	}
////	else if(sei_date >= (sy1_date + 0x10000) && sei_date < (sy1_date + 0x20000)) {
////		// ���Z�����������̏W�v�����͈�
////		Syuukei_sp.ec_inf.next.cnt2[ec_kind] += 1L;						/* �g�p��				*/
////		Syuukei_sp.ec_inf.next.ryo2[ec_kind] += ryo;					/* �g�p���z				*/
////	}
////	else if(sei_date >= (sy1_date - 0x10000) && sei_date < sy1_date) {
////		// ���Z�������O���̏W�v�����͈�
////		if(Syuukei_sp.ec_inf.cnt == 1) {
////			// �O���̏W�v���Ȃ����ߍ쐬����
////			UnNrm_YMDHM(&wrk_date, (sy1_date - 0x10000));
////			memcpy(&Syuukei_sp.ec_inf.bun[Syuukei_sp.ec_inf.ptr].SyuTime, &wrk_date, sizeof(date_time_rec));
////			Syuukei_sp.ec_inf.ptr++;
////			Syuukei_sp.ec_inf.cnt++;
////		}
////		syu_idx = (Syuukei_sp.ec_inf.ptr == 0) ? (SYUUKEI_DAY_EC-1) : (Syuukei_sp.ec_inf.ptr-1);
////		Syuukei_sp.ec_inf.bun[syu_idx].cnt2[ec_kind] += 1L;				/* �g�p��				*/
////		Syuukei_sp.ec_inf.bun[syu_idx].ryo2[ec_kind] += ryo;			/* �g�p���z				*/
////	} else {
////		// ���Z��������������O���͈̔͊O�̏ꍇ�̓A���[����o�^���ē����ɉ��Z
////		Syuukei_sp.ec_inf.now.cnt2[ec_kind] += 1L;						/* �g�p��				*/
////		Syuukei_sp.ec_inf.now.ryo2[ec_kind] += ryo;						/* �g�p���z				*/
////		Syuukei_sp.ec_inf.now.unknownTimeCnt++;
////
////		// �A���[���o�^(���σ��[�_�̌��ώ����Ɛ��Z�@�̎�����1���ȏ�̂��ꂪ����)
////		alm_chk( ALMMDL_SUB, ALARM_EC_SETTLEMENT_TIME_GAP, 2 );
////	}
//// MH810105 GG119202(S) ������������W�v�d�l���P
////	// ���ό��ʃf�[�^�i���ϖ����m��j�ɂ͌��ϓ������Z�b�g����Ȃ����߁A�����W�v�͓����Ƃ��ăJ�E���g����
////	ec_kind = brand - EC_EDY_USED;
////	Syuukei_sp.ec_inf.now.cnt2[ec_kind] += 1L;							/* �g�p��				*/
////	Syuukei_sp.ec_inf.now.ryo2[ec_kind] += ryo;							/* �g�p���z				*/
//// MH810105 GG119202(E) ������������W�v�d�l���P
//// MH810103 GG119202(E) �A���[������񐔂͓����ɃJ�E���g����
//
//	ac_flg.ec_alarm = 0;												/* �W�v���Z����			*/
//
//	return;
//}
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX

// MH810105 GG119202(S) T���v�A���󎚑Ή�
//[]----------------------------------------------------------------------[]
///	@brief			T���v�A����
//[]----------------------------------------------------------------------[]
///	@param[in]		pri_req		: �󎚎�ʁiT���v�^�O��T���v�^�����W�v�j
///	@param[in]		pFrmSyuukei	: �s���v�󎚃f�[�^�t�H�[�}�b�g�|�C���^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/08/30
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void ec_linked_total_print(ushort pri_req, T_FrmSyuukei *pFrmSyuukei)
{
	T_FrmSyuukeiEc	FrmSyuk2;
	SYUKEI			*syu = pFrmSyuukei->prn_data;
	date_time_rec	oldtime, nowtime, wk_time;
	ushort			LogCount;
	ushort			FstIdx, LstIdx;

	// T_FrmSyuukeiEc��T_FrmSyuukei+T_FrmTgoukeiEc�̍\����
	memset(&FrmSyuk2, 0, sizeof(FrmSyuk2));
	memcpy(&FrmSyuk2, pFrmSyuukei, sizeof(*pFrmSyuukei));

	if (prm_get(COM_PRM, S_ECR, 5, 1, 3) == 1) {
		// �݂Ȃ����σv�����g
		FrmSyuk2.syuk.print_flag |= TGOUKEI_EC_MINASHI;
		FrmSyuk2.Ec.Minashi.prn_kind = R_PRI;						// �Ώ�������Fڼ��
		LogCount = Ope2_Log_NewestOldestDateGet_AttachOffset(LOG_ECMINASHI,
								&nowtime, &oldtime, &FstIdx, &LstIdx);
		if (syu->OldTime.Year != 0 ||
			syu->OldTime.Mon != 0 ||
			syu->OldTime.Day != 0 ||
			syu->OldTime.Hour != 0 ||
			syu->OldTime.Min != 0) {
			// �O��W�v�����`����W�v�����������͈͂Ƃ���
			FrmSyuk2.Ec.Minashi.BType = 2;							// �������@�F����
			memcpy(&FrmSyuk2.Ec.Minashi.TSttTime, &syu->OldTime,
					sizeof(FrmSyuk2.Ec.Minashi.TSttTime));			// �J�n����
			memcpy(&FrmSyuk2.Ec.Minashi.TEndTime, &syu->NowTime,
					sizeof(FrmSyuk2.Ec.Minashi.TEndTime));			// �I������
		}
		else {
			if (pri_req == PREQ_TGOUKEI_Z) {
				// �O��W�v�������I�[��0�̏ꍇ��
				// �m�[�}���C�Y��_+1���`����W�v�����������͈͂Ƃ���i�O��T���v�j
				FrmSyuk2.Ec.Minashi.BType = 2;						// �������@�F����
				FrmSyuk2.Ec.Minashi.TSttTime.Year = 1980;			// �J�n�����i1980/3/2 00:00�j
				FrmSyuk2.Ec.Minashi.TSttTime.Mon = 3;
				FrmSyuk2.Ec.Minashi.TSttTime.Day = 2;
				FrmSyuk2.Ec.Minashi.TSttTime.Hour = 0;
				FrmSyuk2.Ec.Minashi.TSttTime.Min = 0;
				memcpy(&FrmSyuk2.Ec.Minashi.TEndTime, &syu->NowTime,
						sizeof(FrmSyuk2.Ec.Minashi.TEndTime));		// �I������
			}
			else {
				// �O��W�v�������I�[��0�̏ꍇ�͑S�����Ƃ���iT���v�j
				FrmSyuk2.Ec.Minashi.BType = 0;						// �������@�F�S��
				memcpy(&FrmSyuk2.Ec.Minashi.TSttTime, &oldtime,
						sizeof(FrmSyuk2.Ec.Minashi.TSttTime));		// �J�n����
				memcpy(&FrmSyuk2.Ec.Minashi.TEndTime, &nowtime,
						sizeof(FrmSyuk2.Ec.Minashi.TEndTime));		// �I������
			}
		}
		FrmSyuk2.Ec.Minashi.LogCount = LogCount;					// �݂Ȃ����σ��O����
		FrmSyuk2.Ec.Minashi.Ffst_no = FstIdx;						// �����������ŏ��Ɉ�v����FlashROM�̏��ԍ�
		FrmSyuk2.Ec.Minashi.Flst_no = LstIdx;						// �����������Ō�Ɉ�v����FlashROM�̏��ԍ�
	}
	if (prm_get(COM_PRM, S_ECR, 5, 1, 2) == 1) {
		// ������������L�^
		FrmSyuk2.syuk.print_flag |= TGOUKEI_EC_ALARM;
		FrmSyuk2.Ec.Alarm.prn_kind = R_PRI;							// �Ώ�������Fڼ��
		if (syu->OldTime.Year != 0 ||
			syu->OldTime.Mon != 0 ||
			syu->OldTime.Day != 0 ||
			syu->OldTime.Hour != 0 ||
			syu->OldTime.Min != 0) {
			// �O��W�v�����`����W�v�����������͈͂Ƃ���
			FrmSyuk2.Ec.Alarm.StartTime = Nrm_YMDHM((date_time_rec*)&syu->OldTime);	// �J�n����
			FrmSyuk2.Ec.Alarm.EndTime = Nrm_YMDHM((date_time_rec*)&syu->NowTime);	// �I������
		}
		else {
			if(pri_req == PREQ_TGOUKEI_Z){
				// �O��W�v�������I�[��0�̏ꍇ��
				// �m�[�}���C�Y��_+1���`����W�v�����������͈͂Ƃ���i�O��T���v�j
				wk_time.Year = 1980;								// �J�n�����i1980/3/2 00:00�j
				wk_time.Mon = 3;
				wk_time.Day = 2;
				wk_time.Hour = 0;
				wk_time.Min = 0;
				FrmSyuk2.Ec.Alarm.StartTime = Nrm_YMDHM((date_time_rec*)&wk_time);
				FrmSyuk2.Ec.Alarm.EndTime = Nrm_YMDHM((date_time_rec*)&syu->NowTime);	// �I������
			}
			else{
				// �O��W�v�������I�[��0�̏ꍇ�͑S�����Ƃ���iT���v�j
			}
		}
	}
	queset(PRNTCBNO, pri_req, sizeof(FrmSyuk2), &FrmSyuk2);
}
// MH810105 GG119202(E) T���v�A���󎚑Ή�
// GM849100(S) M.Fujikawa 2025/03/11 ���S�����R�[���Z���^�[�Ή� �R�[�h�`�F�b�N#257073
ulong set_parking_time( date_time_rec *InTime, date_time_rec *OutTime )
{

	ulong			indate;					// ���ɓ�
	ulong			outdate;				// �o�ɓ�
	ulong			m_intime;				// ���ɓ����i�����Z�j
	ulong			m_outtime;				// �o�ɓ����i�����Z�j
	ulong			parktime = 0L;			// ���Ԏ��ԁi�����Z�j


	if( InTime->Year != 0 ) {
		// ���ɓ��v�Z
		indate	= dnrmlzm( (short)InTime->Year, (short)InTime->Mon, (short)InTime->Day );
		// �o�ɓ��v�Z
		outdate	= dnrmlzm( (short)OutTime->Year, (short)OutTime->Mon, (short)OutTime->Day	);

		// ���ɓ����i�����Z�j�v�Z
		m_intime = (indate*24*60) + (InTime->Hour*60) + (InTime->Min);
		// �o�ɓ����i�����Z�j�v�Z
		m_outtime = (outdate*24*60) + (OutTime->Hour*60) + (OutTime->Min);

		// �o�ɓ��������ɓ���
		if( m_outtime >= m_intime ) {
			parktime = m_outtime - m_intime;		// ���Ԏ��Ԏ擾�i�����Z�j

			// �Z�o�������Ԏ��Ԃ͂X�X�X�X�X�X���ȏ�
			if( parktime > 999999L ) {
				parktime = 999999L;				// �X�X�X�X�X�X���ŃZ�b�g
			} 
		// �o�ɓ��������ɓ���
		} else {
			parktime = 0L;					// �O���ŃZ�b�g
		}
	// ���ɓ����Ȃ�
	} else {
		parktime = 0L;			// ���Ԏ������O���ŃZ�b�g
	}

	return( parktime );

}
// GM849100(E) M.Fujikawa 2025/03/11 ���S�����R�[���Z���^�[�Ή� �R�[�h�`�F�b�N#257073
