/**********************************************************************************************************************/
/*                                                                                                                    */
/*  �֐�����          �F�T�[�r�X������              �L�q                                                              */
/*                                                �F--------------------------------------------------------------�F  */
/*  �֐��V���{��      �Fet45()                    �F  �T�[�r�X���̎�ށA�����ɂ�莞�Ԋ����E���������E�Ԏ�ؑւɂ�F  */
/*                                                �F��e�����������s���B                                          �F  */
/*                                                �F--------------------------------------------------------------�F  */
/*                                                �F���̓p�����[�^              �o�̓p�����[�^                    �F  */
/*                                                �F  ����                        ����                            �F  */
/*                                                �F                                                              �F  */
/*                                                �F--------------------------------------------------------------�F  */
/**********************************************************************************************************************/
																		/**********************************************/
#include	<string.h>													/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
#include	"system.h"													/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
#include	"mem_def.h"													/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
#include	"rkn_def.h"													/*�@�S�f�t�@�C�������@�@�@�@�@�@�@�@�@�@�@�@�@*/
#include	"rkn_cal.h"													/*�@�����֘A�f�[�^�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
#include	"rkn_fun.h"													/*�@�S�T�u���[�`���錾�@�@�@�@�@�@�@�@�@�@�@�@*/
#include	"Tbl_rkn.h"													/*                                            */
#include	"prm_tbl.h"													/*                                            */
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
#include	"ope_def.h"
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
#include	"flp_def.h"													/*                                            */
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
#include	"ntnet_def.h"
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)
void	et45()															/*  �T�[�r�X������                            */
{																		/*                                            */
	struct	REQ_RKN	*req_p;												/*  ���݌v�Z��i�[�G���A                      */
	struct	RYO_BUF_N	*r;												/*  ���݌v�Z��i�[�G���A                      */
	long			turi = 0;											/*                                            */
	long			wk_dis = 0;											/*                                            */
	long			wk_ryo = 0;											/*                                            */
	long			sav_dat = 0;										/*  �f�[�^�Z�[�u                              */
	long			wok_pdis = 0;										/*  ���������������z                          */
	short			mai;												/*                                            */
	short			tckt_type;											/*  �T�[�r�X�����                            */
	long			cash_dis;											/*  ��������                                  */
	short			cnt;												/*  ٰ�߶���                                  */
	long			disc = 0;											/*  �������p�����z�i���Ԋ����p�j  			  */
																		/*                                            */
	long			st_jik = 0;											/*�������ԕۑ����[�N                          */
	short			wk_adr;												/* ���ڽ                                      */
	char			role = 0;											/* ����                                       */
	long			wk_ryo2 = 0;										/* ���Z���~�@�\����                           */
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
	struct	REQ_RKN	*n_req_p;
	uchar			disc_sts;
// MH810103(S) R.Endo 2021/07/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5841 �y���؉ێw�E�����z�uQR�������Ŏ�ʐ؊����Đ��Z�v �� �u���O�^�C���� ���� �ǉ����������O�ɍĐ��Z�v ���s���Ɗ������z�ɕs���Ȓl���Z�b�g�����
//	ulong			last_disc;
	ulong			last_disc 	= 0;
// MH810103(E) R.Endo 2021/07/29 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5841 �y���؉ێw�E�����z�uQR�������Ŏ�ʐ؊����Đ��Z�v �� �u���O�^�C���� ���� �ǉ����������O�ɍĐ��Z�v ���s���Ɗ������z�ɕs���Ȓl���Z�b�g�����
	ulong			new_disc;
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// MH810100(S) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
	char			disFlag = 0;
// MH810100(E) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
																		/*                                            */
/**********************************************************************************************************************/
																		/*                                            */
	req_p		= &req_rhs[tb_number];									/*  ���ݗ����v�Z�v���|�C���^�[                */
	r			= &ryo_buf_n;											/*  ���ݗ����v�Z�v���|�C���^�[                */
	tckt_type 	= (short)((req_rkn.data[0] >> 16) & 0x0000ffff);		/*  �T�[�r�X����ގQ��                        */
																		/*                                            */
	wk_adr		= (short)( 1+3*(tckt_type-1) );							/*                                            */
	role		= (char)CPrmSS[S_SER][wk_adr];							/*                                            */
// MH810105(S) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
//// MH321800(S) hosoda IC�N���W�b�g�Ή� (�A���[�����)
//	if (f_sousai != 0) {
//		role = 4;
//	}
//// MH321800(E) hosoda IC�N���W�b�g�Ή� (�A���[�����)
// MH810105(E) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
																		/*                                            */
	discount	= 0l;													/*  �����z�O�N���A                            */
	memmove( req_p, &req_rkn, sizeof(req_rkn) );						/*  ���ݗ����v�Z�v��TBL���v�Z����TBL�փZ�b�g  */
																		/*                                            */
	if ( ! req_rkn.data[1] )	mai = 1;								/*  �����P�Ƃ���                              */
	else						mai = (short)req_rkn.data[1];			/*  �T�[�r�X�������Q��                        */
																		/*                                            */
	if ( role == 1 || role == 2  || role == 4 )							/*  �������P���Q�̏ꍇ�̂ݏ���������          */
	{																	/*                                            */
		if (ctyp_flg == 4)												/*  �����Ώۊz�������{�ŋ��̏ꍇ              */
		{																/*                                            */
			wk_ryo 	 = r->ryo   - r->dis + r->tax;						/*  �����z������                              */
			cons_tax = 0;												/*  �ŋ����O�i�g�p���Ȃ��j                    */
		}																/*                                            */
		else															/*                                            */
		{																/*                                            */
			wk_ryo 	 = r->ryo - r->dis;									/*  �����z������                              */
		}																/*                                            */
																		/*                                            */
		switch( role )													/*  �����ɂ�菈���𕪊�                      */
		{																/*                                            */
			case 1:														/*  �������P�i���Ԋ����j�̏ꍇ      	      */
				memcpy ( &car_ot, &carot_mt, 7 );						// �o�Ɏ��������ȏo�Ɏ����ɃR�s�[����
																		// 
				jik_dtm		= prm_tim( 0,S_SER,(short)(wk_adr+1));		/*  �������Ԃ��Z�o                            */
				st_jik = jik_dtm;										/*                                            */
				for( cnt = 0 ; cnt < mai ; cnt ++ )						/*                                            */
				{														/*                                            */
					if(req_rkn.param != RY_CSK)							/*                                            */
						ryo_buf.tim_amount += (ulong)jik_dtm;			/*  �������Ԑ����Z                            */
					if( tki_flg == PAS_TIME )							// ���ԑђ�������̏ꍇ
					{													/*                                            */
						jik_dtm = st_jik;								/* 0���Z�b�g����邽�ߒl���ăZ�b�g            */
						sav_dat 		= req_rkn.data[1];				/*  �f�[�^�Z�[�u                              */
						req_rkn.data[1] = tsn_tki.kind;					/*  �����ʃZ�b�g                            */
						re_req_flg 		= ON;							/*  ���ԑђ���㎞�Ԋ����t���O�Z�b�g          */
						if( (su_jik_dtm != 0)&&(CPrmSS[S_CAL][30] == 1)	// ��ʊ������Ԃ�����ݒ肪�P��
									&&( su_jik_plus	== 0 ))				// �P��ڂȂ��
						{												/*                                            */
							jik_dtm = jik_dtm + su_jik_dtm;				/*  �������ԂɎ�ʊ������Ԃ��v���X            */
							su_jik_plus = 1;							/*  �����ʊ������ԃv���X�������n�m          */
						}												/*                                            */
						discount 		= et422();						/*  ������Ԋ����z�Z�o                        */
						req_rkn.data[1] = sav_dat;						/*  �f�[�^�Z�[�u                              */
					}													/*                                            */
					else		discount = et421();						/*  ���������flg��ON�ȊO  �ʏ펞�Ԋ����z�Z�o */
/* �����z���O���菭�Ȃ��ƍ��񊄈��z��ϲŽ�ް��ƂȂ��Ă��܂��A�W�v�ް���ł�ϲŽ�ŏW�v����Ă��܂���                 */
/* �O�񊄈��z��荡�񊄈��z�����Ȃ��ꍇ�ͤ���񊄈��z��O�񊄈��z�Ƃ�����񕪂��O�~�Ƃ���.                              */
/* ���Z���~���͊����z���������z�ɑ΂����z���������ׁAmae_dis�Ɋ����ް����Z�b�g���Ȃ����Ƃ��� */
					if(req_rkn.param != RY_CSK){						/*  ���~���H                                  */
						if( mae_dis > discount ){						/*                                            */
							discount = mae_dis;							/*                                            */
						}												/*                                            */
						wk_dis		= discount;							/*  �������������z                            */
						discount	= discount - mae_dis;				/*  ���񎞊Ԋ����z�Z�o                        */
						mae_dis		= wk_dis;							/*  �����Ԋ����z�i�g�[�^�����Ԋ����z�j        */
					}
					else{ 
						if( tyushi_mae_dis > discount ){				/*                                            */
							discount = tyushi_mae_dis;					/*                                            */
						}												/*                                            */
						wk_dis		= discount;							/*  �������������z                            */
						discount	= discount - tyushi_mae_dis;		/*  ���񎞊Ԋ����z�Z�o                        */
						tyushi_mae_dis		= wk_dis;					/*  �����Ԋ����z�i�g�[�^�����Ԋ����z�j        */
					}
					disc		+= discount ;							/*  �������p�����z�ɍ��񊄈��z���Z            */
																		/*                                            */
					if( percent != 0 )									/*  ���������������ꍇ                        */
					{													/*                                            */
						if( CPrmSS[S_CAL][21] == 1 )					/*  �������̖�������������ꍇ                */
							wok_pdis = ec68( discount,(short)percent);	/*  ��������������                            */
						else											/*  �������������Ȃ��ꍇ                      */
							wok_pdis = discount * percent / 10000 ;		/*  �P�O�O�O�O�Ŋ���                          */
																		/*                                            */
						discount = discount - wok_pdis;					/*  �������̑Ή�����                          */
					}													/*                                            */
				}														/*                                            */
																		/*                                            */
			break;														/*  �����I��                                  */
																		/*                                            */
			case 2:														/*  �������Q�i���������j�̏ꍇ                */
				cash_dis = CPrmSS[S_SER][wk_adr+1];						/*  �ݒ芄���������Z�o                        */
																		/*                                            */
				discount 		= (long)cash_dis;						/*  �����z���P������̊����z                  */
			break;														/*  �����I��                                  */
																		/*                                            */
			case 4:														/*  �������S�i�S�z�����j�̏ꍇ                */
				discount		= MACRO_WARIBIKI_GO_RYOUKIN;			/*  �����z���������|�v���y���p�z              */
				mai 	 		= 1;									/*  �g�p����                                  */
				ryoukin  		= 0;									/*  �������O	                              */
				syu_tax  		= 0;									/*  �ŋ����O                                  */
				base_dis_flg 	= ON;									/*  ��{�����S�z�����t���O�n�m                */
			break;														/*  �����I��                                  */
		}																/*                                            */
																		/*                                            */
		if( (role != 1)&&(role != 4) )	discount = discount * mai;		/*  ���Ԋ����ł͂Ȃ�          �������̊����z  */
		else if( mai 	 > 1 )			discount = disc ;				/*  ���Ԋ����Ŗ����͂Q���ȏ�  �������p�����z  */
																		/*                                            */
		if(req_rkn.param == RY_CSK){									/*                                            */
			wk_ryo2 = wk_ryo;											/*                                            */
			wk_ryo = r->ryo - discount_tyushi;							/*                                            */
		}																/*                                            */
// MH810100(S) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
		disc_sts = (uchar)(( req_p->data[0] >> 15 ) & 0x00000001 );			// �����X�e�[�^�X���o��
		if( disc_sts ){														// ������
// MH810100(S) 2020/09/02 �Đ��Z���̕������Ή�
//			last_disc = Ope_GetLastDisc();									// �����ϊz��o��
			last_disc = Ope_GetLastDisc(discount);									// �����ϊz��o��
// MH810100(E) 2020/09/02 �Đ��Z���̕������Ή�
			
// MH810100(S) 2020/09/28 �y�A���]���w�E�����z���񐸎Z�����z200�~��S�z����������ԂōĐ��Z���s���ƁA�Đ��Z�����z���S�z��������Ȃ�(No.02-0001)
//			// �O�񕪂����̎��_�ň����i�������傫���Ȃ��Ă���ꍇ�́A�ēx�K�p���ꂽ�B�j
//			if( discount >= last_disc){
//				discount = discount - last_disc;
//				disFlag = 1;
//			// �S�z���̏ꍇ��MAX�ɃZ�b�g
//			}else if(base_dis_flg == ON){
//				discount = NMAX_OVER;
//				discount = discount - last_disc;
//				disFlag = 1;
			// �S�z���̏ꍇ��MAX�ɃZ�b�g
			if(base_dis_flg == ON){
				discount = NMAX_OVER;
				discount = discount - last_disc;
				disFlag = 1;
			// �O�񕪂����̎��_�ň����i�������傫���Ȃ��Ă���ꍇ�́A�ēx�K�p���ꂽ�B�j
			}else if( discount >= last_disc){
				discount = discount - last_disc;
				disFlag = 1;
// MH810100(E) 2020/09/28 �y�A���]���w�E�����z���񐸎Z�����z200�~��S�z����������ԂōĐ��Z���s���ƁA�Đ��Z�����z���S�z��������Ȃ�(No.02-0001)
			}else{
				// �����̏ꍇ�́A�����Ɉ������������ꍇ
				discount = 0; 
				disFlag = 1;
			}
		}
// MH810100(E) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
																		/*                                            */
		if (MACRO_WARIBIKI_GO_RYOUKIN <= discount)						/*  �����z����{�������傫���ꍇ            */
		{																/*                                            */
			if( jitu_wari )												/*                                            */
			{															/*                                            */
				discount =  MACRO_WARIBIKI_GO_RYOUKIN;					/*    ������̗��������߂�      			  */
																		/*                                            */
				if(req_rkn.param != RY_CSK){							/*                                            */
					tol_dis  += discount;								/*  �����z���v�������z���v�{�����z            */
				}														/*                                            */
			}															/*                                            */
			else														/*  �����������Ȃ��ꍇ                        */
			{															/*                                            */
				if( turi_wari )											/*  �ނ�K�ΏۂƂ���ꍇ                      */
				{														/*                                            */
					turi 	 = discount - wk_ryo;						/*                                            */
					svcd_dat = turi;									/*                                            */
				}														/*                                            */
				if(req_rkn.param != RY_CSK){							/*                                            */
					tol_dis  += wk_ryo;									/*  �����z���v�������z���v�{�����z            */
				}														/*                                            */
			}															/*                                            */
			ryoukin  	 = 0;											/*  ������̗��������߂�                      */
			syu_tax  	 = 0;											/*  �ŋ����O                                  */
			base_dis_flg = ON;											/*  ��{�����S�z�����t���O�n�m                */
		}																/*                                            */
		else															/*											  */
		{																/*                                            */
			if(req_rkn.param != RY_CSK){								/*                                            */
				ryoukin  = wk_ryo  - discount;							/*  �����z������   ������̗��������߂�       */
				tol_dis += discount;									/*  �����z���v�������z���v�{�����z            */
			}															/*                                            */
		}																/*                                            */
																		/*                                            */
		if( ctyp_flg != 4 && cons_tax_ratio )							/*  �����Ώۂ��ېőO������ŗ�������ꍇ    */
			cons_tax = ec68( ryoukin , cons_tax_ratio );				/*  ����Ŋz�������㗿���~����ŗ�            */
	}																	/*                                            */
																		/*                                            */
	if(req_rkn.param == RY_CSK){										/*                                            */
		PayData_Sub.wari_data[wrcnt_sub].tik_syu = 0x01;				/*                                            */
		PayData_Sub.wari_data[wrcnt_sub].syubetu = (uchar)tckt_type;	/*                                            */
		PayData_Sub.wari_data[wrcnt_sub].maisuu = (uchar)mai;			/*                                            */
		PayData_Sub.wari_data[wrcnt_sub].ryokin = discount;				/*                                            */
		wrcnt_sub++;													/*                                            */
		wk_ryo = wk_ryo2;												/*                                            */
		discount_tyushi += discount;									/*                                            */
	}																	/*                                            */
	if(req_rkn.param != RY_CSK){										/*                                            */
																		/*                                            */
		if( ctyp_flg != 4 )												/*                                            */
		{																/*                                            */
			r->tax = cons_tax;											/*  ���ԗ�������ł̃Z�[�u                    */
																		/*                                            */
			if( ctyp_flg == 1 )											/*  ���ł̏ꍇ                                */
			{															/*                                            */
				syu_tax  = cons_tax;									/*  ����ł��W�v�p�G���A��                    */
				cons_tax = 0;											/*  ����ł��O                                */
			}															/*                                            */
		}																/*                                            */
																		/*                                            */
		r->dis			= tol_dis;										/*  ���ԗ�������ł̃Z�[�u                    */
		req_p->data[2]	= discount;										/*  �����z���Z�b�g                            */
		req_p->data[3]	= ryoukin;										/*  ������z���Z�b�g                          */
		req_p->data[4]	= cons_tax;										/*  ����Ŋz���Z�b�g                          */
																		/*                                            */
		if( role == 2 ){												/*  ��������?                                 */
			ryo_buf.fee_amount += discount;								/*  �����z���Z                                */
		}																/*                                            */
	}																	/*                                            */
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_GT3890���ǉ�)
//	if (role != 1 && role != 2 && role != 4) {
//		return;
//	}
	// �I�����C���f�[�^�̊������Ȃ�
	// ���Z�@�ݒ�ɍ��킹�Ċ�����ʂ��㏑��
	if( role == 1 ){	// ���Ԋ���
			m_stDisc.DiscSyu = NTNET_SVS_T;
	}else{				// ���������E�S�z����
			m_stDisc.DiscSyu = NTNET_SVS_M;
	}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_GT3890���ǉ�)
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
	disc_sts = (uchar)(( req_p->data[0] >> 15 ) & 0x00000001 );			// �����X�e�[�^�X���o��
	if( disc_sts ){														// ������
// MH810100(S) 2020/09/02 �Đ��Z���̕������Ή�
//		last_disc = Ope_GetLastDisc();									// �����ϊz��o��
// MH810100(E) 2020/09/02 �Đ��Z���̕������Ή�
// MH810100(S) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
		if(disFlag){
			// �߂��i�f�[�^�쐬�ׂ̈Ɂj
			discount = discount + last_disc;
		}
// MH810100(E) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
// MH810100(S) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j
		wk_ryo2 = last_disc;
		last_disc =	Ope_GetLastDiscOrg();
// MH810100(E) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j
// MH810100(S) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
//		if( m_stDisc.Discount >= last_disc ){							// ���Ɋ����ϊz�𒴂��Ă���H
		if( m_stDisc.Discount != 0 && m_stDisc.Discount >= last_disc ){							// ���Ɋ����ϊz�𒴂��Ă���H
// MH810100(E) 2020/08/06 #4563�y�A���]���w�E�����z��ʊ����̏��ԑΉ�
			// ���񊄈��z��S�ĐV�K�����Ƃ���
			req_p->data[0] &= 0xffff7fff;								// �����σr�b�g�𗎂Ƃ�
		}else{
			if( m_stDisc.Discount + discount > last_disc ){				// ���񊄈��Ŋ����ϊz�𒴂���H
				new_disc = ( m_stDisc.Discount + discount ) - last_disc;// ����V�K�����z�Z�o
				tb_number++;											// ����ǉ��������͗����v�Z�v���e�[�u���𕪂���
				n_req_p =  &req_rhs[tb_number];
				memcpy( n_req_p, req_p, sizeof(req_rkn) );				// ���ݗ����v�Z�v���e�[�u���R�s�[
				// �����ϕ��̗����v�Z�v���e�[�u���C��
				req_p->data[2] = discount - new_disc;					// �����ϊz = ���񊄈��z - ����V�K�����z
				req_p->data[3] = ryoukin + new_disc;					// ������z = �����㗿�� + ����V�K�����z
// MH810100(S) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
				req_p->data[1] = req_p->data[1] + 2000;					// �t���O�Ƃ���+2000
// MH810100(E) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
				// ����V�K���������̗����v�Z�v���e�[�u���C��
				n_req_p->data[0] &= 0xffff7fff;							// �����σr�b�g�𗎂Ƃ�
				n_req_p->data[2] = new_disc;							// ����ǉ��������z�Z�b�g
// MH810100(S) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
				n_req_p->data[1] = 1000;								// 0���t���O�Ƃ���+1000
// MH810100(E) 2020/06/15 �Ԕԃ`�P�b�g���X(#4229 �y���ƕ����r���[�w�E�z�O�񔼒[�Ȋ����K����A�Đ��Z�Ŏc��̊������K�����ꂽ�ꍇ�ANT-NET�ō��z���M���邪�g�p������0�Ƃ��Ăق���)
			}
// MH810100(S) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
// MH810100(S) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j
			last_disc = wk_ryo2;
// MH810100(E) 2020/09/10 #4821 �y�A���]���w�E�����z1��̐��Z�œ���XNo�̎{�݊����𕡐��g�p����ƁA�Đ��Z���ɑO�񗘗p���Ƃ��ăJ�E���g����銄��������1���ƂȂ�NT-NET���Z�f�[�^�Ɋ�����񂪃Z�b�g����Ă��܂��iNo.02-0057�j
			if(( base_dis_flg == ON || disFlag == 1)&& last_disc != 0){
				req_p->data[2] = last_disc;
				if(disFlag == 0){
					// RT�f�[�^�p��
					discount = last_disc;
				}
			}
// MH810100(E) 2020/06/30 �y�A���]���w�E�����z�Đ��Z���̊����K�����@�̕ύX
		}
	}
	m_stDisc.Discount += (ulong)discount;
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
	return ;															/*                                            */
}																		/**********************************************/
