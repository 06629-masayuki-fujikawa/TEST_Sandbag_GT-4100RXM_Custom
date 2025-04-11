/*[]----------------------------------------------------------------------------------------------[]*/
/*| fukuden.c																					   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2001-12-27																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"cnm_def.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"mnt_def.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"LKmain.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"suica_def.h"
#include	"flp_def.h"
#include	"ntnetauto.h"
#include	"AppServ.h"
#include	"LKmain.h"
#include	"oiban.h"
#include	"raudef.h"
#include	"cre_ctrl.h"

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �W�v�E�󎚒��̒�d�̕��d����																   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: fukuden( void )																   |*/
/*| PARAMETER	: void																			   |*/
/*| RETURN VALUE: void																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-12-27																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void fukuden( void )
{
	char	cy, sy;													/*								*/
	SYUKEI	*ts, *gs, *ms, *ws, *ds;								/* ms(MT���v),ds(T���v)��ǉ�	*/
	unsigned long	*ss, *as;										/*								*/
	LOKTOTAL_DAT	*lkts;											/*								*/
	int		i;														/*								*/
	ushort	syasitu;												/*								*/
	uchar	f_SameData;												/* 1=������~ð��ٓ��ɖړI�ް����� */
	uchar	uc_update = PayData.teiki.update_mon;					/* �X�V���i�[					*/
	T_FrmReceipt	ReceiptPreqData;								// ���̎��؈󎚗v����				ү�����ް�
	T_FrmFusKyo		FusKyoPreqData;									// ���s���E�����o�ɏ��󎚗v����	ү�����ް�
	uchar			jnl_pri = OFF;									// �ެ��و󎚗L��
	ushort	num;
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
	long			jnl_pri_jadge;
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	char	qrcode[QRCODE_RECEIPT_SIZE + 4];
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
																	/*								*/
	cy = ac_flg.cycl_fg;											/* ���Z�����׸�					*/
	sy = ac_flg.syusyu;												/* �W�v�����׸�					*/
	ts = &skyprn;													/*								*/
	gs = &sky.gsyuk;												/* �f�s�W�v�ر					*/
	ms = &sky.msyuk;												/* �l�s�W�v�ر					*/
	ws = &wksky;													/* �W�vܰ��ر					*/
																	/*								*/
																	/*								*/
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//	if ( EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ) {
//		Ec_Settlement_Res.settlement_data = PayData.Electron_data.Ec.pay_ryo;
//		Ec_Settlement_Res.settlement_data_before = PayData.Electron_data.Ec.pay_befor;
//		Ec_Settlement_Res.settlement_data_after = PayData.Electron_data.Ec.pay_after;
//		memcpy( &Ec_Settlement_Res.Card_ID,PayData.Electron_data.Ec.Card_ID,sizeof(Ec_Settlement_Res.Card_ID));
//// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
//		c_UnNormalize_sec(PayData.Electron_data.Ec.pay_datetime, &Ec_Settlement_Res.settlement_time);					// ���Z����
//		memcpy(&Ec_Settlement_Res.inquiry_num,PayData.Electron_data.Ec.inquiry_num,sizeof(Ec_Settlement_Res.inquiry_num));// �⍇���ԍ�
//		memcpy(&Ec_Settlement_Res.Termserial_No,PayData.Electron_data.Ec.Termserial_No,sizeof(Ec_Settlement_Res.Termserial_No));// �[�����ʔԍ�
//// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
//	}
//	else
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	if( SuicaUseKindCheck( PayData.Electron_data.Suica.e_pay_kind )){
		Settlement_Res.settlement_data = PayData.Electron_data.Suica.pay_ryo;
		Settlement_Res.settlement_data_before = PayData.Electron_data.Suica.pay_befor;
		Settlement_Res.settlement_data_after = PayData.Electron_data.Suica.pay_after;
		memcpy( &Settlement_Res.Suica_ID,PayData.Electron_data.Suica.Card_ID,sizeof(Settlement_Res.Suica_ID));
	}

// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//	switch (ac_flg.ec_alarm) {
//	case 2:														/* ܰ��ر����� */
//		memcpy( &sky.tsyuk, ws, sizeof( sky.tsyuk ) );			/* ܰ��ر���s�W�v�ցi��d�΍�j	*/
//		DailyAggregateDataBKorRES( 1 );							/* �ޯ������ް���ؽı			*/
//		// no break
//	case 1:														/* ���σ��[�_ �A���[������ް��o�^ */
//		ec_alarm_syuu(EcAlarm.Brand, EcAlarm.Ryo);				/* �A���[������o�^				*/
//		break;
//// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
////	case 3:
////		// �݂Ȃ����ϔ�����
////		ReqEcDeemedJnlPrint();
////		ac_flg.ec_alarm = 0;
////		break;
//// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��
//	default :
//		break;
//	}
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
	// �܂����O����f�[�^���������Ă��Ȃ���Ώ�������
	if( ac_flg.ec_recv_deemed_fg != 0 ){
		EcRecvDeemed_RegistPri();
	}
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
	if (ac_flg.ec_deemed_fg != 0) {
		// �݂Ȃ����ϔ�����
		ReqEcDeemedJnlPrint();
		ac_flg.ec_deemed_fg = 0;
		// �������ɕύX���Đ��Z���~���O���c��
		cy = 10;
	}
// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��

	if( cy != 0 ){													/* �����`�W�v���Z���̒�d�L��H	*/
		switch( cy ){												/*								*/
																	/* ***** ���Z *****				*/
			case 10:												/* ������						*/
				safecl( 1 );										/* ������������(���ɖ����Z�o�p)	*/
				safecl( 7 );										/* ���ɖ����Z�o					*/
				ac_flg.cycl_fg = 50;								/* ��������d���A�W�v�J�n		*/
				toty_syu();											/* ���Z�r����d���A�W�v			*/
				break;												/*								*/
																	/*								*/
			case 11:												/* �������i�\���O�~�j			*/
				ryo_buf.fusoku = ryo_buf.turisen;					/* �x�����s���z���				*/
			case 12:												/* �ނ�K�����o���N��			*/
			case 13:												/* �ނ�K�����o������			*/
				if( ac_flg.cycl_fg == 12 ){							/*								*/
					refalt();										/* �s�����Z�o					*/
					ryo_buf.fusoku += SFV_DAT.reffal;				/* �x�����s���z���				*/
				}													/*								*/
				if(( ryo_buf.pkiti != 0xffff )&&					/* ������Z�ȊO?					*/
				   ( uc_update == 0 )){								/* ������X�V���Z�ȊO			*/
					syasitu = ryo_buf.pkiti;						/*								*/
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
//					queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof(syasitu), &syasitu );	/* ���Z����������(�ׯ�߰���~)	*/
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
					FLAPDT.flp_data[syasitu - 1].s_year = car_ot_f.year;// ���Z����	�i�N�j���
					FLAPDT.flp_data[syasitu - 1].s_mont = car_ot_f.mon;	// 			�i���j���
					FLAPDT.flp_data[syasitu - 1].s_date = car_ot_f.day;	// 			�i���j���
					FLAPDT.flp_data[syasitu - 1].s_hour = car_ot_f.hour;// 			�i���j���
					FLAPDT.flp_data[syasitu - 1].s_minu = car_ot_f.min;	// 			�i���j���
					IoLog_write(IOLOG_EVNT_AJAST_FIN, (ushort)LockInfo[syasitu - 1].posi, 0, 1);
				}													/*								*/
				ac_flg.cycl_fg = 13;								/*								*/
				PayData_set( 1, 0 );								/* 1���Z����					*/
				kan_syuu();											/*								*/
				safecl( 7 );										/* ���ɖ����Z�o�A�ޑK�Ǘ��W�v	*/
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//				Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );		/* ���K�Ǘ����O�f�[�^�쐬		*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* ���K�Ǘ����O�o�^				*/
				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass ) ){	/* ���K�Ǘ����O�f�[�^�쐬		*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* ���K�Ǘ����O�o�^				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
																	/*								*/
																	/*								*/
				break;												/*								*/
																	/*								*/
			case 14:												/* �s�W�v��ܰ��ر�֓]������		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/
				if(( ryo_buf.pkiti != 0xffff )&&					/* ������Z�ȊO?					*/
				   ( uc_update == 0 )){								/* ������X�V���Z�ȊO			*/
					syasitu = ryo_buf.pkiti - 1;					/*								*/
					lkts = &loktl.tloktl.loktldat[syasitu];			/*								*/
					memcpy( lkts, &wkloktotal, sizeof( LOKTOTAL_DAT ) );/* ܰ��ر���s�W�v��(��d�΍�)*/
				}													/*								*/
				DailyAggregateDataBKorRES( 1 );						/* �ޯ������ް���ؽı			*/
				ac_flg.cycl_fg = 13;								/*								*/
				PayData_set( 1, 0 );								/* 1���Z����					*/
				kan_syuu();											/*								*/
				safecl( 7 );										/* ���ɖ����Z�o�A�ޑK�Ǘ��W�v	*/
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//				Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );		/* ���K�Ǘ����O�f�[�^�쐬		*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* ���K�Ǘ����O�o�^				*/
				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass ) ) {	/* ���K�Ǘ����O�f�[�^�쐬		*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* ���K�Ǘ����O�o�^				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
				break;												/*								*/
																	/*								*/
			case 16:												/* �ʐ��Z�ް�ܰ��o�^����		*/
				if( PrnJnlCheck() == ON ){
					//	�ެ���������ڑ�����̏ꍇ
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
//					switch(CPrmSS[S_RTP][1]){	// �ެ��و󎚏����H
					jnl_pri_jadge = prm_get( COM_PRM, S_RTP, 1, 2, 1 );			// 17-0001�̒l��ϐ��ɕۑ�

					jnl_pri_jadge = invoice_prt_param_check( jnl_pri_jadge );	// �C���{�C�X���p�����[�^�ϊ�

					switch(jnl_pri_jadge){	// �ެ��و󎚏����H
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
						case	0:
						case	1:
						case	2:
							// �ެ��ق͂O�~�����L�^
							jnl_pri = ON;
							break;
						case	3:
						case	4:
						case	5:
							// �ެ��ق͒��ԗ����O�~���ȊO�L�^
							if( PayData.WPrice != 0 ){
								//	���ԗ������O
								jnl_pri = ON;
							}
							break;
						case	6:
						case	7:
						case	8:
							// �ެ��ق͌����̎��O�~���ȊO�L�^
							if( PayData.WTotalPrice != 0 ){
								//	�����̎��z���O
								jnl_pri = ON;
							}
							break;
						case	9:
						case   10:
						case   11:
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//							if( PayData.WTotalPrice != 0 || PayData.Electron_data.Suica.e_pay_kind ){
							if( PayData.WTotalPrice != 0 ||
								SuicaUseKindCheck(PayData.Electron_data.Suica.e_pay_kind) ||
								EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
								//	�����̎��z���O OR �d�q�}�̂ɂ�鐸�Z���s��ꂽ�ꍇ
								jnl_pri = ON;
							}
							break;
						default:
							jnl_pri = OFF;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
							break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
					}

					if( jnl_pri == ON ){
						//	�ެ��و󎚏����𖞂����ꍇ
						ReceiptPreqData.prn_kind = J_PRI;											// �o��������i�ެ��فj
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//						Pri_Work[1].Receipt_data = PayData;
//						ReceiptPreqData.prn_data = &Pri_Work[1].Receipt_data;	// �ް��߲�����
						memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
						ReceiptPreqData.prn_data = &Cancel_pri_work;			// �ް��߲�����
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
						ReceiptPreqData.reprint = OFF;												// �Ĕ��s�׸�ؾ�āi�ʏ�j
						queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// �󎚗v��ү���ޓo�^
					}
				}
// MH810100(S) K.Onodera  2019/12/18 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//				NTNET_Snd_Data22_FusDel( ryo_buf.pkiti, (ushort)PayData.PayMethod, (ushort)PayData.PayClass, (ushort)PayData.OutKind );	// �s���o�ɏ�񂠂�Ώ���
// MH810100(E) K.Onodera  2019/12/18 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
				goto	_after_pay_log_regist;
																	/*								*/
// MH810100(S) K.Onodera  2019/12/18 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//			case 122:
//				memcpy( &fusei, &wkfus, sizeof( struct FUSEI_SD ) );	/* �s���ް��߂�			*/
//				NTNET_Snd_Data22_FusDel( ryo_buf.pkiti, (ushort)PayData.PayMethod, (ushort)PayData.PayClass, (ushort)PayData.OutKind );	// �s���o�ɏ�񂠂�Ώ���
//				goto	_after_pay_log_regist;
// MH810100(E) K.Onodera  2019/12/18 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			case 15:												/* �s�ر�W�v���Z����			*/
				PayData_set( 1, 0 );								/* 1���Z����					*/
				Log_regist( LOG_PAYMENT );							/* �ʐ��Z���o�^				*/
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
					// RT�̎��؃f�[�^�o�^
					RTReceipt_LogRegist_AddOiban();
					// QR�̎��؃f�[�^�쐬
					MakeQRCodeReceipt(qrcode, sizeof(qrcode));
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
					// QR�m��E����f�[�^�o�^
					ope_SendCertifCommit();
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
				}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
_after_pay_log_regist:
				ac_flg.cycl_fg = 19;								/*								*/
			case 19:
				if( ryo_buf.ryo_flg >= 2 )							/* ����g�p?					*/
				{													/*								*/
					memcpy( &tki_cyusi_wk,							/*								*/
							&tki_cyusi,								/*								*/
							sizeof(t_TKI_CYUSI) );					/* ���~����ް��ޔ�				*/
																	/*								*/
					ac_flg.cycl_fg = 20;							/*								*/
					for( i=0; i<TKI_CYUSI_MAX; i++ )				/*								*/
					{												/*								*/
						if( tki_cyusi.dt[i].no == PayData.teiki.id &&/* ���~�ޯ̧�Ɏg�p����ް��L��?*/
							tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*		*/
						{											/*								*/
							NTNET_Snd_Data219(1, &tki_cyusi.dt[i]);	/* 1���폜�ʒm���M				*/
							TKI_Delete((short)i);					/* ���~���1���ر				*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* �����ð��ٍX�V(�o��)	*/
								  (ushort)PayData.teiki.id,			/*								*/
								  1,								/*								*/
								  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*				*/
								  0xffff );							/*								*/
				}													/*								*/
																	/*								*/
				CountUp( PAYMENT_COUNT );							/* ���Z�ǔ�+1					*/
				if( ryo_buf.fusoku != 0 ){							/* �a��ؔ��s?					*/
					CountUp(DEPOSIT_COUNT);							/* �a��ؒǔ�+1					*/
				}
				if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
					RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
				}
// MH810100(S) K.Onodera  2020/01/23 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
				if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
					DC_PopCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔԂ�Pop
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
				}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) K.Onodera  2020/01/23 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
																	/*								*/
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//				if( PayData.credit.pay_ryo ){						/* �ڼޯĶ��ގg�p				*/
//					ac_flg.cycl_fg = 80;							// �N���W�b�g���p���דo�^�J�n
//					Log_regist( LOG_CREUSE );						/* �N���W�b�g���p���דo�^		*/
//				}													/*								*/
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
				if (EcUseEMoneyKindCheck(PayData.Electron_data.Ec.e_pay_kind) &&
					PayData.EcResult == EPAY_RESULT_MIRYO_ZANDAKA_END) {
					// ��������ς݂��m�F�����ꍇ��������������L�^��o�^����
					ac_flg.cycl_fg = 90;							// ������������L�^�o�^�J�n
					EcAlarmLog_Regist(&EcAlarm.Ec_Res);
					ac_flg.cycl_fg = 91;							// ������������L�^�o�^����
					// ������������L�^�̍Đ��Z����o�^����
					EcAlarmLog_RepayLogRegist(&PayData);
				}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
				ac_flg.cycl_fg = 17;								/* 17:�ʐ��Z�ް��o�^����		*/
																	/*								*/
				// no break											/*								*/
																	/*								*/
			case 17:												/* �ʐ��Z�ް��o�^����			*/
				safecl( 7 );										/* ���ɖ����Z�o					*/
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//				Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );		/* ���K�Ǘ����O�f�[�^�쐬		*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* ���K�Ǘ����O�o�^				*/
				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {	/* ���K�Ǘ����O�f�[�^�쐬		*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* ���K�Ǘ����O�o�^				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
				break;												/*								*/
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
			case 90:												// ������������L�^�o�^�J�n
				// ��������ς݂��m�F�����ꍇ��������������L�^��o�^����
				EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				ac_flg.cycl_fg = 91;								// ������������L�^�o�^����
				// no break
			case 91:
				// ������������L�^�̍Đ��Z����o�^����
				EcAlarmLog_RepayLogRegist(&PayData);
				ac_flg.cycl_fg = 17;								/* 17:�ʐ��Z�ް��o�^����		*/

				safecl( 7 );										/* ���ɖ����Z�o					*/
				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {	/* ���K�Ǘ����O�f�[�^�쐬		*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* ���K�Ǘ����O�o�^				*/
				}													/*								*/
				break;
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
// MH321800(S) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//			case 80:												// �N���W�b�g���p���דo�^�J�n
//				Log_regist( LOG_CREUSE );							/* �N���W�b�g���p���דo�^		*/
//				ac_flg.cycl_fg = 17;								/* 17:�ʐ��Z�ް��o�^����		*/
//				safecl( 7 );										/* ���ɖ����Z�o					*/
//// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
////				Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass );		/* ���K�Ǘ����O�f�[�^�쐬		*/
////				Log_regist( LOG_MONEYMANAGE_NT );					/* ���K�Ǘ����O�o�^				*/
//				if( Make_Log_MnyMng( OpeNtnetAddedInfo.PayClass )) {	/* ���K�Ǘ����O�f�[�^�쐬		*/
//					Log_regist( LOG_MONEYMANAGE_NT );				/* ���K�Ǘ����O�o�^				*/
//				}													/*								*/
//// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//				break;												/*								*/
// MH321800(E) T.Nagai IC�N���W�b�g�Ή� �s�v�@�\�폜(�Z���^�[�N���W�b�g)
			case 20:												/* ������~�f�[�^�X�V��			*/
				memcpy( &tki_cyusi, &tki_cyusi_wk,					/*								*/
						sizeof(t_TKI_CYUSI) );						/* �ޔ𒆎~����ް������ɖ߂�	*/
				goto	_after_pay_log_regist;
																	/*								*/
																	/* ***** ���~ *****				*/
			case 21:												/* �������i�\���O�~�j			*/
				ryo_buf.fusoku = ryo_buf.turisen = ryo_buf.nyukin;	/* �x�����s���z���				*/
			case 22:												/* �ނ�K�����o���N��			*/
			case 23:												/* �ނ�K�����o������			*/
				if( ac_flg.cycl_fg == 22 ){							/*								*/
					refalt();										/* �s�����Z�o					*/
					ryo_buf.fusoku += SFV_DAT.reffal;				/* �x�����s���z���				*/
				}													/*								*/
				ac_flg.cycl_fg = 23;								/*  							*/
				PayData_set( 1, 1 );								/* 1���Z����					*/
				cyu_syuu();											/* ���Z���~�W�v					*/
				safecl( 7 );										/* ���ɖ����Z�o�A�ޑK�Ǘ��W�v	*/
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//				Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );/* ���~���̋��K�Ǘ����O�f�[�^�쐬	*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* ���K�Ǘ����O�o�^				*/
				if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {/* ���~���̋��K�Ǘ����O�f�[�^�쐬	*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* ���K�Ǘ����O�o�^				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
				break;												/*								*/
																	/*								*/
			case 24:												/* �s�W�v��ܰ��ر�֓]������		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/
				DailyAggregateDataBKorRES( 1 );						/* �ޯ������ް���ؽı			*/
				ac_flg.cycl_fg = 23;								/*								*/
				PayData_set( 1, 1 );								/* 1���Z����					*/
				cyu_syuu();											/*								*/
				safecl( 7 );										/* ���ɖ����Z�o�A�ޑK�Ǘ��W�v	*/
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//				Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );/* ���~���̋��K�Ǘ����O�f�[�^�쐬	*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* ���K�Ǘ����O�o�^				*/
				if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {/* ���~���̋��K�Ǘ����O�f�[�^�쐬	*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* ���K�Ǘ����O�o�^				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
				break;												/*								*/
																	/*								*/
			case 26:												/* ���Z���~�ް�ܰ��o�^����		*/
				memcpy( &tki_cyusi, &tki_cyusi_wk,					/*								*/
						sizeof(t_TKI_CYUSI) );						/* �ޔ𒆎~����ް������ɖ߂�	*/
				if( PrnJnlCheck() == ON ){
					//	�ެ���������ڑ�����̏ꍇ
					ReceiptPreqData.prn_kind = J_PRI;				// �o��������i�ެ��فj
					ReceiptPreqData.prn_data = &Cancel_pri_work;	// �ް��߲�����
					ReceiptPreqData.reprint = OFF;					// �Ĕ��s�׸�ؾ�āi�ʏ�j
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// �󎚗v��ү���ޓo�^
				}
				IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
				goto	_after_can_log1_regist;
			case 28:												/* ���Z���~�ް�ܰ��o�^����		*/
				if( PrnJnlCheck() == ON ){
					//	�ެ���������ڑ�����̏ꍇ
					ReceiptPreqData.prn_kind = J_PRI;				// �o��������i�ެ��فj
					ReceiptPreqData.prn_data = &Cancel_pri_work;	// �ް��߲�����
					ReceiptPreqData.reprint = OFF;					// �Ĕ��s�׸�ؾ�āi�ʏ�j
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// �󎚗v��ү���ޓo�^
				}
				IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
				goto	_after_can_log2_regist;
																	/*								*/
			case 25:												/* �s�ر�W�v���Z����			*/
				PayData_set( 1, 1 );								/* 1���Z����					*/
				if( ryo_buf.ryo_flg >= 2 )							/* ����g�p?					*/
				{													/*								*/
					memcpy( &tki_cyusi_wk,							/*								*/
							&tki_cyusi,								/*								*/
							sizeof(t_TKI_CYUSI) );					/* ���~����ް��ޔ�				*/
																	/*								*/
					Log_regist( LOG_PAYSTOP );						/* ���Z���~���o�^				*/
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
_after_can_log1_regist:
																	/*								*/
					f_SameData = 0;									/*								*/
																	/*								*/
					if( TKI_CYUSI_MAX < tki_cyusi.count )			/* fail safe					*/
						tki_cyusi.count = TKI_CYUSI_MAX;			/*								*/
																	/*								*/
					if( TKI_CYUSI_MAX <= tki_cyusi.wtp )			/* fail safe					*/
						tki_cyusi.wtp = tki_cyusi.count - 1;		/*								*/
																	/*								*/
					for( i=0; i<tki_cyusi.count; i++ )				/*								*/
					{												/*								*/
						if( 0L == tki_cyusi.dt[i].pk ){				/* ð��ق��ް��Ȃ�				*/
							break;									/* i=�o�^����					*/
						}											/*								*/
						if( tki_cyusi.dt[i].no == PayData.teiki.id &&/* �Ē��~?						*/
							tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*		*/
						{											/*								*/
							f_SameData = 1;							/*								*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					if( 0 == f_SameData )							/* �����ް��Ȃ�(�V�K�o�^�K�v)	*/
					{												/*								*/
						if( i == TKI_CYUSI_MAX ){					/* �o�^����Full					*/
							NTNET_Snd_Data219(1, &tki_cyusi.dt[0]);	/* 1���폜�ʒm���M				*/
							TKI_Delete(0);							/* �Ō��ް��폜					*/
							nmisave( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* ���~����ް��ޔ�		*/
						}											/*								*/
						tki_cyusi.dt[tki_cyusi.wtp].syubetu =		/*								*/
													PayData.syu;	/* �������(A�`L:1�`12)			*/
						tki_cyusi.dt[tki_cyusi.wtp].pk =			/*								*/
							CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];/* ���ԏꇂ						*/
						tki_cyusi.dt[tki_cyusi.wtp].no =			/*								*/
												PayData.teiki.id;	/* �l����(1�`12000)			*/
						tki_cyusi.dt[tki_cyusi.wtp].tksy =			/*								*/
												PayData.teiki.syu;	/* ������(1�`15)				*/
						tki_cyusi.dt[tki_cyusi.wtp].year =			/* �����N						*/
											PayData.TInTime.Year;	/*								*/
						memcpy( &tki_cyusi.dt[tki_cyusi.wtp].mon,	/*								*/
								PayData.teiki.t_tim, 4 );			/* ������������					*/
						tki_cyusi.dt[tki_cyusi.wtp].sec = 0;		/* �����b						*/
																	/*								*/
						NTNET_Snd_Data219(0, &tki_cyusi.dt[tki_cyusi.wtp]);	/* 1���X�V�ʒm���M		*/
																	/*								*/
						if( tki_cyusi.count < TKI_CYUSI_MAX )		/*								*/
						{											/*								*/
							tki_cyusi.count++;						/* �o�^������+1					*/
						}											/*								*/
																	/*								*/
						tki_cyusi.wtp++;							/* ײ��߲��+1					*/
						if( tki_cyusi.wtp >= TKI_CYUSI_MAX )		/*								*/
						{											/*								*/
							tki_cyusi.wtp = 0;						/*								*/
						}											/*								*/
					}												/*								*/
					WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* �����ð��ٍX�V(�o��)	*/
								  (ushort)PayData.teiki.id,			/*								*/
								  1,								/*								*/
								  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*				*/
								  0xffff );							/*								*/
				}													/*								*/
				else{												/*								*/
					Log_regist( LOG_PAYSTOP );						/* ���Z���~���o�^				*/
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
				}													/*								*/
_after_can_log2_regist:
																	/*								*/
				CountUp( CANCEL_COUNT );							/* ���Z���~�ǔ�+1				*/
				if( ryo_buf.fusoku != 0 ){							/* �a��ؔ��s?					*/
					CountUp(DEPOSIT_COUNT);							/* �a��ؒǔ�+1					*/
				}
				if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
					RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
				}
// MH810100(S) K.Onodera  2020/01/23 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
				if( PayData.PayMode !=4 ){	// ���u���Z�łȂ��H
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
					DC_PopCenterSeqNo( DC_SEQNO_QR );	// �Z���^�[�ǔԂ�Pop
// MH810100(S) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
				}
// MH810100(E) K.Onodera  2020/03/03 #3944 �Ԕԃ`�P�b�g���X(���u���Z�Ő��Z�I�����Ƀ��A���^�C���T�[�o�[�֐��Z�f�[�^�𑗐M����s��C��)
// MH810100(E) K.Onodera  2020/01/23 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
				if ((EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
					PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) &&
					(PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0)) {
					// ���ϒ��ɏ�Q�����A�܂��́A�����c���Ɖ�^�C���A�E�g�������߁A
					// ������������L�^��o�^����
					ac_flg.cycl_fg = 100;							// ������������L�^�o�^�J�n
					EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
																	/*								*/
				ac_flg.cycl_fg = 27;								/* 27:���Z���~�ް��o�^����		*/
			case 27:												/* ���Z���~�ް��o�^����			*/
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
_after_can_log3_regist:
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
				safecl( 7 );										/* ���ɖ����Z�o�A�ޑK�Ǘ��W�v	*/
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//				Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );/* ���~���̋��K�Ǘ����O�f�[�^�쐬	*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* ���K�Ǘ����O�o�^				*/
				if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {/* ���~���̋��K�Ǘ����O�f�[�^�쐬	*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* ���K�Ǘ����O�o�^				*/
				}													/*								*/
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
				break;												/*								*/
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
			case 100:												// ������������L�^�o�^�J�n
				// ���ϒ��ɏ�Q�����A�܂��́A�����c���Ɖ�^�C���A�E�g�������߁A
				// ������������L�^��o�^����
				EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				ac_flg.cycl_fg = 27;								/* 27:���Z���~�ް��o�^����		*/
				goto	_after_can_log3_regist;
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
																	/* ***** �s���E���� *****		*/
			case 32:												/* �s�W�v��ܰ��ر�֓]������		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/
				syasitu = fusei.fus_d[0].t_iti - 1;					/*								*/
				lkts = &loktl.tloktl.loktldat[syasitu];				/*								*/
				memcpy( lkts, &wkloktotal, sizeof( LOKTOTAL_DAT ) );/* ܰ��ر���s�W�v��(��d�΍�)	*/
																	/*								*/
				ac_flg.cycl_fg = 31;								/*								*/
			case 31:
				if( fus_syuu() == 2 ){								/*								*/
					if( prm_get(COM_PRM,S_NTN,63,1,5) == 0 ){		/* �������M���Ȃ��ݒ�̏ꍇ		*/
						ntautoReqToSend( NTNET_BUFCTRL_REQ_SALE );	/* ���Z�f�[�^���M�v��			*/
					}												/*								*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			case 81:
				logwork.flp.Kikai_no = (uchar)CPrmSS[S_PAY][2];		// �@�BNo.

				logwork.flp.In_Time.Year = car_in_f.year;				/* ���ɔN�������� 	*/
				logwork.flp.In_Time.Mon  = car_in_f.mon;
				logwork.flp.In_Time.Day  = car_in_f.day;
				logwork.flp.In_Time.Hour = car_in_f.hour;
				logwork.flp.In_Time.Min  = car_in_f.min;
				logwork.flp.Date_Time.Year = car_ot_f.year;				/* ���Z�N�������� 	*/
				logwork.flp.Date_Time.Mon  = car_ot_f.mon;
				logwork.flp.Date_Time.Day  = car_ot_f.day;
				logwork.flp.Date_Time.Hour = car_ot_f.hour;
				logwork.flp.Date_Time.Min  = car_ot_f.min;

				num = ryo_buf.pkiti - 1;
				logwork.flp.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	/* ���Ԉʒu 		*/

				logwork.flp.Lok_No = num +1;

				if( fusei.fus_d[0].kyousei == 1 || fusei.fus_d[0].kyousei == 11 ){
					CountGet( KIYOUSEI_COUNT, &logwork.flp.count );
					logwork.flp.Lok_inf = 0;							/* ���� 			*/
				}
				else{					// 0�F�ʏ�s���A2�F�C���s��(��������)�A3�F�C���s��(�����Ȃ�)
					CountGet( FUSEI_COUNT, &logwork.flp.count );
					logwork.flp.Lok_inf = 1;							/* �s�� 			*/
					if( fusei.fus_d[0].kyousei != 0 ){
						logwork.flp.Lok_inf = 2;						/* �C�����Z�ɂ��s�� */
					}
				}
				if( fusei.fus_d[0].kyousei == 3 ){
					logwork.flp.fusei_fee = 0;						/* ���ԗ��� 		*/
				}else{
					logwork.flp.fusei_fee = ryo_buf.dsp_ryo;		/* ���ԗ��� 		*/
				}
				logwork.flp.ryo_syu = ryo_buf.tik_syu;				/* ������� 		*/

				memcpy( &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp], &logwork.flp, sizeof( flp_log ) );
				Log_Write(eLOG_ABNORMAL, &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp], TRUE);// �s���E�����o�Ƀ��O�o�^
			case 34:												/* �s���E�����o�ɏ��ܰ��o�^����*/
				if( PrnJnlCheck() == ON ){
					//	�ެ���������ڑ�����̏ꍇ
					FusKyoPreqData.prn_kind = J_PRI;											// �o��������i�ެ��فj
					FusKyoPreqData.prn_data = &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp];	// �ް��߲�����
					queset( PRNTCBNO, PREQ_FUSKYO_JOU, sizeof(T_FrmFusKyo), &FusKyoPreqData );	// �󎚗v��ү���ޓo�^
				}
				if(logwork.flp.Lok_inf == 0){
					IoLog_write(IOLOG_EVNT_FORCE_FIN, (ushort)FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].WPlace, 0, 0);
					CountUp( KIYOUSEI_COUNT );													//�����o�ɒǂ���
				}else{
					IoLog_write(IOLOG_EVNT_OUT_ILLEGAL, (ushort)FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].WPlace, 0, 0);
					CountUp( FUSEI_COUNT );														//�s���o�ɒǂ���
					if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
						RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
					}
				}
				FLP_LOG_DAT.Flp_wtp++;															// ײ��߲�� +1
				if( FLP_LOG_DAT.Flp_wtp >= FLP_LOG_CNT ){
					FLP_LOG_DAT.Flp_wtp = 0;
				}
				ac_flg.cycl_fg = 35;
				goto	_after_abnormal_log_regist;
																	/*								*/
			case 33:												/* �s�ر�W�v���Z����			*/
				Log_regist( LOG_ABNORMAL );							/* �s���E�����o�ɏ��			*/
				ac_flg.cycl_fg += 1;								/* 35							*/
																	/*								*/
			case 35:												/* �s���E�����o�ɏ��o�^����	*/
_after_abnormal_log_regist:
				memcpy( &wkfus, &fusei, sizeof( struct FUSEI_SD ) );/* �s���ް���ܰ��ر��			*/
				ac_flg.cycl_fg += 1;								/* 36							*/
																	/*								*/
			case 36:												/* �s���ް���ܰ��ر�֓]������	*/
				memcpy( &fusei, &wkfus, sizeof( struct FUSEI_SD ) );/*								*/
																	/*								*/
				memcpy( &fusei, &wkfus.fus_d[1],					/* �ް����						*/
						sizeof(struct FUSEI_D)*(LOCK_MAX-1) );		/*								*/
				fusei.kensuu -= 1;									/*								*/
				ac_flg.cycl_fg += 1;								/* 37							*/
																	/*								*/
			case 37:												/* �ް���āE����-1����			*/
				memset( &fusei.fus_d[(LOCK_MAX-1)],					/*								*/
						0, sizeof(struct FUSEI_D));					/*								*/
				ac_flg.cycl_fg += 1;								/* 38							*/
				break;												/*								*/
																	/*								*/
																	/*  �ׯ�ߏ㏸�Eۯ�����ѓ��o��	*/
			case 42:
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/
				syasitu = locktimeout.fus_d[0].t_iti - 1;			/*								*/
				lkts = &loktl.tloktl.loktldat[syasitu];				/*								*/
				memcpy( lkts, &wkloktotal, sizeof( LOKTOTAL_DAT ) );/* ܰ��ر���s�W�v��(��d�΍�)	*/
																	/*								*/
			case 41:
				lto_syuu();											/*								*/
				break;												/*								*/
			case 44:
			
			case 43:
				PayData_set_LO(locktimeout.fus_d[0].t_iti,0,0,97);	/*								*/
				Log_Write(eLOG_PAYMENT, &PayData, TRUE);			/*								*/
			case 45:
				memcpy( &wklocktimeout, &locktimeout, sizeof( struct FUSEI_SD ) );	/*								*/
			case 46:
				memcpy( &locktimeout, &wklocktimeout.fus_d[1],		/* �ް����						*/
						sizeof(struct FUSEI_D)*(LOCK_MAX-1) );		/*								*/
				locktimeout.kensuu -= 1;							/* ����-1						*/
			case 47:
				memset( &locktimeout.fus_d[(LOCK_MAX-1)],			/*								*/
						0, sizeof(struct FUSEI_D));					/*								*/
				ac_flg.cycl_fg = 48;								/* 48							*/
				break;
																	/* ***** ���Z�r���̒�d *****	*/
			case 51:												/* �s�W�v��ܰ��ر�֓]������		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/
				DailyAggregateDataBKorRES( 1 );						/* �ޯ������ް���ؽı			*/
																	/*								*/
			case 50:												/* ��������d���A�W�v�J�n		*/
				toty_syu();											/* ���Z�r����d���A�W�v			*/
				break;												/*								*/
																	/*								*/
			case 53:												/* ���Z���~�ް�ܰ��o�^����		*/
				memcpy( &tki_cyusi, &tki_cyusi_wk,					/*								*/
						sizeof(t_TKI_CYUSI) );						/* �ޔ𒆎~����ް������ɖ߂�	*/
				if( PrnJnlCheck() == ON ){
					//	�ެ���������ڑ�����̏ꍇ
					ReceiptPreqData.prn_kind = J_PRI;				// �o��������i�ެ��فj
					ReceiptPreqData.prn_data = &Cancel_pri_work;	// �ް��߲�����
					ReceiptPreqData.reprint = OFF;					// �Ĕ��s�׸�ؾ�āi�ʏ�j
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// �󎚗v��ү���ޓo�^
				}
				IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
				goto	_after_can_log21_regist;
			case 55:												/* ���Z���~�ް�ܰ��o�^����		*/
				if( PrnJnlCheck() == ON ){
					//	�ެ���������ڑ�����̏ꍇ
					ReceiptPreqData.prn_kind = J_PRI;				// �o��������i�ެ��فj
					ReceiptPreqData.prn_data = &Cancel_pri_work;	// �ް��߲�����
					ReceiptPreqData.reprint = OFF;					// �Ĕ��s�׸�ؾ�āi�ʏ�j
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// �󎚗v��ү���ޓo�^
				}
				IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
				goto	_after_can_log22_regist;
																	/*								*/
			case 52:												/* �s�ر�W�v���Z����			*/
				if( ryo_buf.ryo_flg >= 2 )							/* ����g�p?					*/
				{													/*								*/
					memcpy( &tki_cyusi_wk,							/*								*/
							&tki_cyusi,								/*								*/
							sizeof(t_TKI_CYUSI) );					/* ���~����ް��ޔ�				*/
																	/*								*/
					Log_regist( LOG_PAYSTOP );						/* ���Z���~���o�^				*/
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
_after_can_log21_regist:
																	/*								*/
					f_SameData = 0;									/*								*/
																	/*								*/
					if( TKI_CYUSI_MAX < tki_cyusi.count )			/* fail safe					*/
						tki_cyusi.count = TKI_CYUSI_MAX;			/*								*/
																	/*								*/
					if( TKI_CYUSI_MAX <= tki_cyusi.wtp )			/* fail safe					*/
						tki_cyusi.wtp = tki_cyusi.count - 1;		/*								*/
																	/*								*/
					for( i=0; i<tki_cyusi.count; i++ )				/*								*/
					{												/*								*/
						if( 0L == tki_cyusi.dt[i].pk ){				/* ð��ق��ް��Ȃ�				*/
							break;									/* i=�o�^����					*/
						}											/*								*/
						if( tki_cyusi.dt[i].no == PayData.teiki.id &&/* �Ē��~?						*/
							tki_cyusi.dt[i].pk == CPrmSS[S_SYS][PayData.teiki.pkno_syu+1] )	/*		*/
						{											/*								*/
							f_SameData = 1;							/*								*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					if( 0 == f_SameData )							/* �����ް��Ȃ�(�V�K�o�^�K�v)	*/
					{												/*								*/
						if( i == TKI_CYUSI_MAX ){					/* �o�^����Full					*/
							NTNET_Snd_Data219(1, &tki_cyusi.dt[0]);	/* 1���폜�ʒm���M				*/
							TKI_Delete(0);							/* �Ō��ް��폜					*/
							nmisave( &tki_cyusi_wk, &tki_cyusi, sizeof(t_TKI_CYUSI) );	/* ���~����ް��ޔ�	*/
						}											/*								*/
						tki_cyusi.dt[tki_cyusi.wtp].syubetu =		/*								*/
													PayData.syu;	/* �������(A�`L:1�`12)			*/
						tki_cyusi.dt[tki_cyusi.wtp].pk =			/*								*/
							CPrmSS[S_SYS][PayData.teiki.pkno_syu+1];/* ���ԏꇂ						*/
						tki_cyusi.dt[tki_cyusi.wtp].no =			/*								*/
												PayData.teiki.id;	/* �l����(1�`12000)			*/
						tki_cyusi.dt[tki_cyusi.wtp].tksy =			/*								*/
										PayData.teiki.syu;			/* ������(1�`15)				*/
						tki_cyusi.dt[tki_cyusi.wtp].year =			/* �����N						*/
											PayData.TInTime.Year;	/*								*/
						memcpy( &tki_cyusi.dt[tki_cyusi.wtp].mon,	/*								*/
								PayData.teiki.t_tim, 4 );			/* ������������					*/
						tki_cyusi.dt[tki_cyusi.wtp].sec = 0;		/* �����b						*/
																	/*								*/
						NTNET_Snd_Data219(0, &tki_cyusi.dt[tki_cyusi.wtp]);	/* 1���X�V�ʒm���M		*/
																	/*								*/
						if( tki_cyusi.count < TKI_CYUSI_MAX )		/*								*/
						{											/*								*/
							tki_cyusi.count++;						/* �o�^������+1					*/
						}											/*								*/
																	/*								*/
						tki_cyusi.wtp++;							/* ײ��߲��+1					*/
						if( tki_cyusi.wtp >= TKI_CYUSI_MAX )		/*								*/
						{											/*								*/
							tki_cyusi.wtp = 0;						/*								*/
						}											/*								*/
					}												/*								*/
					WritePassTbl( (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/* �����ð��ٍX�V(�o��)	*/
								  (ushort)PayData.teiki.id,			/*								*/
								  1,								/*								*/
								  (ulong)CPrmSS[S_SYS][PayData.teiki.pkno_syu+1],	/*				*/
								  0xffff );							/*								*/
				}													/*								*/
				else{												/*								*/
					Log_regist( LOG_PAYSTOP );						/* ���Z���~���o�^				*/
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
				}													/*								*/
_after_can_log22_regist:
																	/*								*/
				CountUp( CANCEL_COUNT );							/* ���Z���~�ǔ�+1				*/
				if( ryo_buf.fusoku != 0 ){							/* �a��ؔ��s?					*/
					CountUp(DEPOSIT_COUNT);							/* �a��ؒǔ�+1					*/
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
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
				if ((EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ||
					PayData.Electron_data.Ec.e_pay_kind == EC_CREDIT_USED) &&
					(PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm != 0)) {
					// ���ϒ��ɏ�Q�����A�܂��́A�����c���Ɖ�^�C���A�E�g�������߁A
					// ������������L�^��o�^����
					ac_flg.cycl_fg = 110;							// ������������L�^�o�^�J�n
					EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
																	/*								*/
				ac_flg.cycl_fg = 54;								/* 54:���Z���~�ް��o�^����		*/
				break;												/*								*/
																	/*								*/
			case 54:												/* ���Z���~�ް��o�^����			*/
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
_after_can_log23_regist:
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
				safecl( 7 );										/* ���ɖ����Z�o�A�ޑK�Ǘ��W�v	*/
// MH322914 (s) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
//				Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) );/* ���~���̋��K�Ǘ����O�f�[�^�쐬	*/
//				Log_regist( LOG_MONEYMANAGE_NT );					/* ���K�Ǘ����O�o�^				*/
				if( Make_Log_MnyMng( (ushort)(OpeNtnetAddedInfo.PayClass+2) )) {/* ���~���̋��K�Ǘ����O�f�[�^�쐬	*/
					Log_regist( LOG_MONEYMANAGE_NT );				/* ���K�Ǘ����O�o�^				*/
				}
// MH322914 (e) kasiyama 2016/07/13 �e�[�u���f�[�^�̌������s���ɉ��Z�����[���ʃo�ONo.1221](MH341106)
				break;												/*								*/
																	/*								*/
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
			case 110:												// ������������L�^�o�^�J�n
				// ���ϒ��ɏ�Q�����A�܂��́A�����c���Ɖ�^�C���A�E�g�������߁A
				// ������������L�^��o�^����
				EcAlarmLog_Regist(&EcAlarm.Ec_Res);
				ac_flg.cycl_fg = 54;								/* 54:���Z���~�ް��o�^����		*/
				goto	_after_can_log23_regist;
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
			case 57:												/* 57: ���ϐ��Z���~��			*/
				Log_Write(eLOG_PAYMENT, &EcRecvDeemedData, TRUE);
				// no break
			case 58:												/* 58: ���ϐ��Z���~�f�[�^��t	*/
				if( PrnJnlCheck() == ON ){
					ReceiptPreqData.prn_kind = J_PRI;				/* �������ʁF�ެ���			*/
					ReceiptPreqData.prn_data = &EcRecvDeemedData;	/* �̎��؈��ް����߲�����		*/

					// pritask�֒ʒm
					queset(PRNTCBNO, PREQ_RECV_DEEMED, sizeof(T_FrmReceipt), &ReceiptPreqData);
				}
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
				// �ڍ׃G���[�R�[�h��o�^����
				EcErrCodeChk( EcRecvDeemedData.EcErrCode, EcRecvDeemedData.Electron_data.Ec.e_pay_kind );
				EcRecvDeemedData.WFlag = 1;							/* ���O����ް����d�󎚊����׸�ON  */
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
				break;
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�

			default:												/*								*/
				ac_flg.cycl_fg = 0;									/*								*/
				break;												/*								*/
		}															/*								*/
		ac_flg.cycl_fg = 0;											/*								*/
	}																/*								*/

	if( sy != 0 )													// �W�v�󎚂���W�v���Z��(���v)�̒�d�L��H
	{																/*								*/
		switch( sy )												/*								*/
		{															/* ***** �s���v *****			*/
			case 11:
				Make_Log_TGOUKEI();									/* �Ԏ����W�v���܂߂�T���v۸ލ쐬*/
				if( prm_get(COM_PRM, S_NTN, 26, 1, 3) ){			// �s���v���M����
					NTNET_Snd_TGOUKEI();
				}
				syuukei_clr( 0 );									/*								*/
				break;												/*								*/

			case 13:												/* ܰ��ر�֓]���ς�				*/
																	/* remote.�ޯ����߁i��d�΍�j*/
				memcpy( gs, ws, sizeof( sky.gsyuk ) );				/* ܰ��ر���f�s�W�v�ցi��d�΍�j*/
				memcpy( loktl.gloktl.loktldat, &wkloktotal, sizeof( LOKTOTAL_DAT ) ); /* ܰ��ر���Ԏ���GT�W�v��(��d�΍�)	*/
				memcpy( &CarCount, &CarCount_W, sizeof( CAR_COUNT ) );	/* ���o�ɶ��Ă����ɖ߂�		*/
				ac_flg.syusyu = 12;									/* 12:�s���v�󎚊���			*/
																	/*								*/
			case 12:												/* �s���v�󎚊���				*/
				syuukei_clr( 0 );									/*								*/
				if( prm_get(COM_PRM, S_NTN, 65, 1, 6) ){	// �o�b�`���M�ݒ�L
	 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
						ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL);
	 				}
					else {
						ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL_PHASE1);
					}
				}
				break;												/*								*/
																	/*								*/
			case 15:												/* �s���v���ܰ��o�^����		*/
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// Log_Write()/Log_Write_Pon()����eLOG_LCKTTL�̏ꍇ��ac_flg.syusyu=19�Ƃ��Ă��邪�A�����ł�ac_flg.syusyu=19
// �Ƃ��Ă�case 19�ł͌���_after_syu_log_regist�ɔ�Ԃ����Ȃ̂ŁA�P���ɎԎ����W�v���O�o�^���X�L�b�v����
//-				// case 15�̃��J�o����Log_Write_Pon()�ŏ�������
//-				Log_Write(eLOG_LCKTTL, &LCKT_LOG_WK, TRUE);			// �Ԏ����W�v���O(31�Ԏ��ȍ~)�o�^
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
				goto	_after_syu_log_regist;
																	/*								*/
			case 14:												/* �s�W�v���f�s�W�v�։��Z����	*/
				Log_regist( LOG_TTOTAL );							/* �s���v���o�^				*/

_after_syu_log_regist:
				ac_flg.syusyu = 16;									/* 16:�s���v���o�^����		*/

				if( prm_get(COM_PRM, S_SCA, 11, 1, 1) == 1 )		/* �������Ұ��̐ݒ�l���L���͈͓��̏ꍇ	*/
					Suica_Rec.Data.BIT.FUKUDEN_SET = 1;				/* ���Z�f�[�^���M�v������		*/

				ds = &sky.tsyuk;		// dst = T					/* �s�W�v						*/
				memcpy( ws, ds, sizeof( SYUKEI ) );					/* �s�W�v��ܰ��ر��(��d�΍�)	*/
				ac_flg.syusyu = 17;									/* 17:���񎞍���ܰ��o�^����		*/
				memset( ds, 0, _SYU_HDR_SIZ );						/* �w�b�_�[���N���A				*/
				ss = &ds->Uri_Tryo;									/*								*/
				as = &ts->Uri_Tryo;									/* �s�|�󎚂s					*/ 
				for( i = 0; i < _SYU_DAT_CNT; i++, ss++, as++ )		/*								*/
				{													/*								*/
					*ss -= *as;										/*								*/
				}													/*								*/
				memcpy( &ds->OldTime, &skybk.tsyuk.NowTime, sizeof( date_time_rec ) ); /* �O��W�v�������			*/
				ac_flg.syusyu = 18;									/* 18:�s�W�v�ر�ر����			*/
				CountUp( T_TOTAL_COUNT );							/* T���v�ǔ�+1					*/
				CountClear( CLEAR_COUNT_T );						/* �ǔԸر(T���v������)			*/
				break;												/*								*/
																	/*								*/
			case 19:												/* �W�v���O�o�^����				*/
			// �t���b�V������ɕ��d���s�����߁A�����ς݂Ȃ��Syu_count��0�ł���
				goto	_after_syu_log_regist;
				
																	/*								*/
			case 16:												/* 16:�s���v���o�^����		*/
				if( prm_get(COM_PRM, S_SCA, 11, 1, 1) == 1 )		/* �������Ұ��̐ݒ�l���L���͈͓��̏ꍇ	*/
					Suica_Rec.Data.BIT.FUKUDEN_SET = 1;				/* ���Z�f�[�^���M�v������		*/
				ds = &sky.tsyuk;		// dst = T					/* �s�W�v						*/
				memcpy( ws, ds, sizeof( SYUKEI ) );					/* �s�W�v��ܰ��ر��(��d�΍�)	*/
				ac_flg.syusyu = 17;									/* 17:���񎞍���ܰ��o�^����		*/
			case 17:												/* ���񎞍���ܰ��o�^����		*/
				ds = &sky.tsyuk;		// dst = T					/* �s�W�v						*/
				memset( ds, 0, _SYU_HDR_SIZ );						/* �w�b�_�[���N���A				*/
				ss = &ds->Uri_Tryo;									/*								*/
				as = &ts->Uri_Tryo;									/* �s�|�󎚂s					*/ 
				for( i = 0; i < _SYU_DAT_CNT; i++, ss++, as++ )		/*								*/
				{													/*								*/
					*ss -= *as;										/*								*/
				}													/*								*/
				memcpy( &ds->OldTime, &skybk.tsyuk.NowTime, sizeof( date_time_rec ) ); /* �O��W�v�������			*/
				memset( &loktl.tloktl, 0, sizeof( LOKTOTAL ) );		/* �Ԏ����W�v��ر				*/
				ac_flg.syusyu = 18;									/* 18:�s�W�v�ر�ر����			*/
				CountUp( T_TOTAL_COUNT );							/* T���v�ǔ�+1					*/
				CountClear( CLEAR_COUNT_T );						/* �ǔԸر(T���v������)			*/
				if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
					RAU_UpdateCenterSeqNo(RAU_SEQNO_TOTAL);
				}
				break;												/*								*/
																	/*								*/
																	/* ***** �f�s���v *****			*/
			case 25:												/* �f�s���v���O�o�^��t			*/
				// case 25�̃��J�o����Log_Write_Pon()�ŏ�������
				goto	_after_gsyu_log_regist;
			case 23:												/* ���񎞍���ܰ��o�^����		*/
				memcpy( ws, ms, sizeof( SYUKEI ) );					/* �l�s�W�v��ܰ��ر��(��d�΍�)	*/
				ac_flg.syusyu = 22;									/* 22:�f�s���v�󎚊���			*/
																	/*								*/
			case 22:												/* �f�s���v�󎚊���				*/
				memcpy( ws, ms, sizeof( SYUKEI ) );					/* �l�s�W�v��ܰ��ر��(��d�΍�)	*/
				ac_flg.syusyu = 23;									/* 23:�l�s�W�v��ܰ��ر�֓]������*/
				memcpy( &skybk.gsyuk, gs, sizeof( SYUKEI ) );		/* �f�s�W�v��O��f�s�W�v�־��	*/
				memcpy( &loktlbk.gloktl, &loktl.gloktl, sizeof( LOKTOTAL ) ); /* �Ԏ����W�v��O��W�v�־��	*/
				if ( prm_get(COM_PRM, S_TOT, 1, 1, 1) != 0) {
					as = &gs->Uri_Tryo;								/*								*/
					ss = &ms->Uri_Tryo;								/*								*/
																	/*								*/
					for( i = 0; i < _SYU_DAT_CNT; i++, as++, ss++ )	/*								*/
					{												/*								*/
						*ss += *as;									/*								*/
					}												/*								*/
					CountAdd(ms, gs);								/* �ǔԉ��Z						*/
					if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
						gs->CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_TOTAL);
					}
				}

				ac_flg.syusyu = 24;									/* 24:�f�s�W�v���l�s�W�v�։��Z����*/
			case 24:												/* �f�s�W�v���l�s�W�v�։��Z����	*/
				Log_regist( LOG_GTTOTAL );							/* �f�s���v���o�^				*/

_after_gsyu_log_regist:

				ac_flg.syusyu = 26;									/* 26:�f�s���v���o�^����		*/
			case 26:												/* �f�s���v���o�^����			*/
				memset( gs, 0, sizeof( sky.gsyuk ) );				/* �f�s�W�v�ر�ر				*/
				memcpy( &gs->OldTime, &skybk.gsyuk.NowTime, sizeof( date_time_rec ) );	/* �O��W�v�������		*/
				memset( &loktl.gloktl, 0, sizeof( LOKTOTAL ) );		/* �Ԏ����W�v��ر				*/
																	/*								*/
				ac_flg.syusyu = 27;									/* 27:�f�s�W�v�ر�ر����		*/
																	/*								*/
				if ( prm_get(COM_PRM, S_TOT, 1, 1, 1) != 0) {
					CountUp( GT_TOTAL_COUNT );						/* GT���v�ǔ�+1					*/
				}
				CountClear( CLEAR_COUNT_GT );						/* �ǔԸر(GT���v������)		*/
				if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
					RAU_UpdateCenterSeqNo(RAU_SEQNO_TOTAL);
				}
				break;												/*								*/
																	/*								*/
																	/* ***** �l�s���v *****			*/
			case 102:												/* �l�s���v�󎚊���				*/
				syuukei_clr( 2 );									/*								*/
				break;												/*								*/
																	/*								*/
			case 107:												/* �l�s�W�v����					*/
				memset( ms, 0, sizeof( SYUKEI ) );					/* �l�s�ر�ر					*/
				memcpy( &ms->OldTime, &skybk.msyuk.NowTime, sizeof( date_time_rec ) ); /* �O��W�v�������			*/
				CountClear( CLEAR_COUNT_MT );						/* �ǔԸر(MT���v������)		*/
				break;												/*								*/
																	/* ***** �R�C�����ɏW�v *****	*/
			case 33:												/* �s�W�v��ܰ��ر�֓]������		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/
				ac_flg.syusyu = 32;									/* 32:�󎚊���					*/
																	/*								*/
			case 32:												/* �󎚊���						*/
				kinko_clr( 0 );										/* ��݋��ɏW�v�󎚊�������		*/
				break;												/*								*/
																	/*								*/
			case 35:												/* ��݋��ɏW�v���ܰ��o�^����	*/
			Make_Log_MnyMng( 10 );									// ���K�Ǘ����O�f�[�^�쐬
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);			// ���K�Ǘ����O�o�^
			goto	_after_money_change_log_coin_regist;
																	/*								*/
			case 34:												/* ���ɏW�v���s�W�v�֓]������	*/
				Log_regist( LOG_COINBOX );							/* ��݋��ɏW�v�����Z			*/
				// Don't break										/*								*/
																	/*								*/
			case 36:												/* ���ɏW�v���o�^����			*/
_after_money_change_log_coin_regist:
				memset( &SFV_DAT.safe_dt[0],						/*								*/
							0, sizeof( SFV_DAT.safe_dt ));			/*								*/
				memcpy( &coin_syu.OldTime,							/*								*/
					&coin_syu.NowTime, sizeof( date_time_rec ) );	/* �O��W�v�������				*/
				memset( &coin_syu.Tryo, 0, sizeof(ulong) * (1+(COIN_SYU_CNT*2)) );	/*				*/
				ac_flg.syusyu = 37;									/* 37:���ɏW�v�ر����			*/
				CountUp( COIN_SAFE_COUNT );							/* ��݋��ɒǔ�+1				*/
				break;												/*								*/
																	/*								*/
																	/* ***** �������ɏW�v *****		*/
			case 43:												/* �s�W�v��ܰ��ر�֓]������		*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/
				ac_flg.syusyu = 42;									/* 42:�󎚊���					*/
																	/*								*/
			case 42:												/* �󎚊���						*/
				kinko_clr( 1 );										/* �������ɏW�v�󎚊�������		*/
				break;												/*								*/
																	/*								*/
			case 45:												/* �������ɏW�v���ܰ��o�^����	*/
				Make_Log_MnyMng( 11 );								// ���K�Ǘ����O�f�[�^�쐬
				Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);		// ���K�Ǘ����O�o�^
				goto	_after_money_change_log_note_regist;
																	/*								*/
			case 44:												/* ���ɏW�v���s�W�v�֓]������	*/
				Log_regist( LOG_NOTEBOX );							/* �������ɏW�v�����Z			*/
				// Don't break										/*								*/
																	/*								*/
			case 46:												/* ���ɏW�v���o�^����			*/
_after_money_change_log_note_regist:
				SFV_DAT.nt_safe_dt = 0;								/*								*/
				memcpy( &note_syu.OldTime,							/*								*/
					&note_syu.NowTime, sizeof( date_time_rec ) );	/* �O��W�v�������				*/
				memset( &note_syu.Tryo, 0, sizeof(ulong) * (1+(SIHEI_SYU_CNT*2)) );/*		*/
					ac_flg.syusyu = 47;								/* 47:���ɏW�v�ر����			*/
				CountUp( NOTE_SAFE_COUNT );							/* �������ɒǔ�+1				*/
				break;												/*								*/
																	/*								*/
																	/* ***** �ޑK�Ǘ��W�v *****		*/
			case 53:												/* ���K�Ǘ����ܰ��o�^����		*/
				Log_Write(eLOG_MNYMNG_SRAM, &turi_kan, FALSE);								// ���K�Ǘ����O�o�^(SRAM)
				goto	_after_money_change_log_regist;
																	/*								*/
			case 52:												/* �󎚊���						*/
				turikan_clr();										/* �ޑK�Ǘ��ر�̍X�V����		*/
				break;												/*								*/
																	/*								*/
			case 54:												/*								*/
_after_money_change_log_regist:
				memcpy( &turi_kwk,									/*								*/
						&turi_kan, sizeof( TURI_KAN ) );			/*								*/
				ac_flg.syusyu = 55;									/* 55:�ޑK�Ǘ��W�v��ܰ��ر�֓]������*/
																	/*								*/
			case 55:												/* �ޑK�Ǘ��W�v��ܰ��ر�֓]������*/
				turikan_clr_sub2();									/*								*/
				ac_flg.syusyu = 56;									/* 56:���K�Ǘ��W�v�ر����		*/
				turikan_clr_sub();									/* T�W�v�֒ޑK��[�A�������o�����Z */
																	/*								*/
				ac_flg.syusyu = 58;									/*								*/
				break;												/*								*/
			case 57:												/* �ޑK��[�A�������o��T�W�v�։��Z�O */
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/
				ac_flg.syusyu = 56;									/* 55:�ޑK�Ǘ��W�v��ܰ��ر�֓]������*/
																	/*								*/
			case 56:												/* �ޑK��[�A�������o��T�W�v�։��Z�O */
				turikan_clr_sub();									/* T�W�v�֒ޑK��[�A�������o�����Z */
				break;												/*								*/
																	/*								*/
			case 60:												/* �������o������T�W�v�։��Z�O	*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/

				turikan_inventry( 0 );								/* T�W�v�֋������o�����Z 		*/
				break;

			case 61:												/* �������o������T�W�v�։��Z�O	*/
				memcpy( ts, ws, sizeof( sky.tsyuk ) );				/* ܰ��ر���s�W�v�ցi��d�΍�j	*/

				turikan_inventry( 1 );								/* T�W�v�֋������o�����Z 		*/
				break;
			default:												/*								*/
				ac_flg.syusyu = 0;									/*								*/
				break;												/*								*/
		}															/*								*/
		ac_flg.syusyu = 0;
	}
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
//// �s��C��(S) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
//	if( FurikaeDestFlapNo ){
//		queset( FLPTCBNO, CAR_PAYCOM_SND, sizeof( FurikaeDestFlapNo ), &FurikaeDestFlapNo );	// ���Z����������(�ׯ�߰���~)
//	}
//// �s��C��(E) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X�i�t���b�v��Ή����j
	return;
}
