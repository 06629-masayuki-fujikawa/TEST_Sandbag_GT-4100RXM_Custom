/**********************************************************************************************************************/
/*                                                                                                                    */
/*  �֐�����          �F�����v�Z�v���쐬�i�u�k�j    �L�q                                                              */
/*                                                �F--------------------------------------------------------------�F  */
/*  �֐��V���{��      �Fec09()                    �F  ���͂��ꂽ�u�k�f�[�^�ɑΉ����闿���v�Z���s���Ă��炤�ׂ̗v���F  */
/*                                                �F�v���e�[�u�����쐬����B                                      �F  */
/*                                                �F--------------------------------------------------------------�F  */
/*                                                �F���̓p�����[�^              �o�̓p�����[�^                    �F  */
/*                                                �F  	����			            ����                          �F  */
/*                                                �F--------------------------------------------------------------�F  */
/*  �ύX����          �F���R (04/07/25)     	  �F�E���Z���~��̃T�[�r�X���Ή��̈ڐA 	  						  �F  */
/*                    �F				          �F  (FCR-P30033 TF4800N DH917004)								  �F  */
/*                                                �F--------------------------------------------------------------�F  */
/**********************************************************************************************************************/
																		/**********************************************/
#include	<string.h>													/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
#include	"system.h"													/*                                            */
#include	"mem_def.h"													/*                                            */
#include	"rkn_def.h"													/*  �S�f�t�@�C������                          */
#include	"rkn_cal.h"													/*  �����֘A�f�[�^                            */
#include	"rkn_fun.h"													/*  �S�T�u���[�`���錾                        */
#include	"tbl_rkn.h"													/*                                            */
#include	"prm_tbl.h"													/*                                            */
#include	"ope_def.h"													/*                                            */
#include	"flp_def.h"
#include	"LKmain.h"													/*											  */
																		/*                                            */
char	ec09( void )													/*                                            */
{																		/*                                            */
	char	sdat;														/*  �ݒ�f�[�^                                */
	char	no;															/*                                            */
	char	ans;														/*                                            */
	char	i;															/*                                            */
	char	mai;														/*                                            */
	short	sv_tim;														/*  ��������                                  */
	short	tim_t[5];													/**                                           */
	short	pno;														/** �ݒ蒓�ԏ�m�n�D                          */
	short	pnk;														/** �ݒ�g�����ԏ�m�n�D                      */
	short	pnk2;														/** �ݒ�g��2���ԏ�m�n�D                     */
	short	pnk3;														/** �ݒ�g��3���ԏ�m�n�D                     */
	long	l_dat;														/*                                            */
	struct	REQ_RKN		*rkn_p;											/*                                            */
	struct	REQ_CRD		*rkn_c;											/*                                            */
	struct	VL_KAS		*kas;											/** �񐔌��f�[�^                              */
	struct	VL_SVS		*svs;											/** �T�[�r�X���f�[�^                          */
	struct	REQ_TKC		*rkn_t;											/*                                            */
// MH810100(S) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
	ushort	mise_num;													/*                                            */
	ushort	mise_use;													/*                                            */
// MH810100(E) K.Onodera 2020/01/27 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
																		/*                                            */
/**********************************************************************************************************************/
																		/*                                            */
	ans = OK;															/*                                            */
	memset( &req_rkn.param,0x00,21 );									/*  �����v���e�[�u���N���A                    */
	rkn_p = &req_rkn;												 	/*                                            */
// MH810100(S) 2020/07/27 �R�[�h���r���[�w�E�����Ή�
	memset( &req_crd,0x00,sizeof(req_crd) );							/*  �e�[�u���N���A                    */
// MH810100(E) 2020/07/27 �R�[�h���r���[�w�E�����Ή�
	req_crd_cnt = 0;													/*  ���Ԍ��ǎ�莞  ���ް��v�������ر         */
	req_tkc_cnt = 0;													/*  ��������~���Z�v�������ر                 */
	rkn_p->syubt = syashu;												/*  �Ԏ�Z�b�g                                */
																		/*                                            */
	pno = (short)0;														/** ��{���ԏ�                                */
	pnk = (short)1;														/** �g��1���ԏ�                               */
	pnk2 = (short)2;													/** �ݒ�g��2���ԏ�                           */
	pnk3 = (short)3;													/** �ݒ�g��3���ԏ�                           */
																		/*                                            */
	switch( vl_now )													/*  �p�����[�^�u�k�f�[�^�ɂ�蕪��            */
	{																	/*                                            */
		case V_CHM:														/*  ���Ԍ��i���Z�O�j                          */
		case V_CHG:														/*  ���Ԍ��i���Z��j                          */
		case V_CHS:														/*  ���Ԍ��i���~���j                          */
		case V_CHU:														/*  �|����                                    */
																		/*                                            */
			rkn_p->param = RY_TSA;										/*  �p�����[�^�Z�b�g                          */
																		/*                                            */
			rysyasu = syashu;											/*  �Ԏ�Z�b�g                                */
			sv_tim = Prm_RateDiscTime[syashu-1];						/*                                            */
			rkn_p->data[0] = sv_tim;									/*  ��ʎ��Ԋ����Z�b�g                        */
			ans = OK;													/*                                            */
																		/*                                            */
			mai = (uchar)prm_get(COM_PRM, S_DIS, 1, 2, 1);
			if( vl_tik.wari ){											/*  �������z(���~,�C���p)                     */
				rkn_c = &req_crd[req_crd_cnt];							/*  ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@  */
				rkn_c->param = RY_RWR;									/*  �p�����[�^�Z�b�g                          */
				rkn_c->data[0] = vl_tik.wari;							/*  �������z�Z�b�g                            */
				req_crd_cnt++;											/*  ���Ԍ��ǂݎ�莞���f�[�^�v�������t�o      */
				PayData.PRTwari.BIT.RWARI = 1;							/* �O�񗿋������t���O                         */
			}															/*                                            */
			if( vl_tik.time ){											/*  �������Ԑ�(���~,�C���p)                   */
				rkn_c = &req_crd[req_crd_cnt];							/*  ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@  */
				rkn_c->param = RY_TWR;									/*  �p�����[�^�Z�b�g                          */
				rkn_c->data[0] = vl_tik.time;							/*  �������Ԑ��Z�b�g                          */
				req_crd_cnt++;											/*  ���Ԍ��ǂݎ�莞���f�[�^�v�������t�o      */
				PayData.PRTwari.BIT.TWARI = 1;							/* �O�񎞊Ԋ����t���O                         */
			}															/*                                            */
																		/*                                            */
			if( vl_tik.kry != 0 )										/*  ����������������L��					  */
			{															/*                                            */
				rkn_c = &req_crd[req_crd_cnt];							/*  ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@  */
				rkn_c->param = RY_KGK;									/*  �p�����[�^�Z�b�g�i����������������j      */
				rkn_c->data[0] = vl_tik.kry;							/*  ���z�Z�b�g                                */
				rkn_c->data[1]= 0L;										/*  �Ƃ肠�����񐔌��g�p��=0�Ƃ���            */
					if( CPrmSS[S_PRP][1] != 2 )							/*  �ݒ肪P���ގg�p�� =1 or �����Ȃ�          */
					rkn_c->data[1]= 1L;									/*  P���ގg�p�ɕύX����                     */
				req_crd_cnt++;											/*  ���Ԍ��ǂݎ�莞���f�[�^�v�������t�o      */
			}															/*                                            */
																		/*											  */
			if(Flap_Sub_Flg == 1){										/* �Đ��Z��(���Z���~�@�\����)                 */
				for(i = 0 ; i < 15 ; i++){								/* ���޽��                                    */
					if( FLAPDT_SUB[Flap_Sub_Num].sev_tik[i] != 0){
						rkn_c 		 	= &req_crd[req_crd_cnt];		/*  ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@  */
						rkn_c->param 	= RY_CSK;						/*  �p�����[�^�Z�b�g�i���Z���~�T�[�r�X���j    */
						l_dat 		 	= i+1;							/*  ��ރZ�b�g                                */
						rkn_c->data[1] 	= FLAPDT_SUB[Flap_Sub_Num].sev_tik[i];	/*	�����Z�b�g                        */
						rkn_c->data[0] 	= l_dat << 16;					/*	��ނƊ|������m���D�Z�b�g                */
						req_crd_cnt		++;								/*	���Ԍ��ǂݎ�莞���f�[�^�v�������t�o      */

						if( prm_get( COM_PRM, S_DIS, 5, 1, 2 ) == 0 ){
							card_use2[i] = FLAPDT_SUB[Flap_Sub_Num].sev_tik[i];	/*���޽�����g�p����ð���                  */
						}
					}
				}
				for(i = 0 ; i < 5 ; i++){								/* �|����                                     */
					if(FLAPDT_SUB[Flap_Sub_Num].kake_data[i].mise_no != 0){
						rkn_c 		 	= &req_crd[req_crd_cnt];		/*  ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@  */
						rkn_c->param 	= RY_CMI;						/*  �p�����[�^�Z�b�g�i���Z���~�|�����j        */
						rkn_c->data[1] 	= FLAPDT_SUB[Flap_Sub_Num].kake_data[i].maisuu;	/*	��ނƊ|������m���D�Z�b�g*/
						rkn_c->data[0] 	= FLAPDT_SUB[Flap_Sub_Num].kake_data[i].mise_no;	/*	�XNo�Z�b�g            */
						req_crd_cnt		++;								/*	���Ԍ��ǂݎ�莞���f�[�^�v�������t�o      */
					}
				}
				PayData_Sub.pay_ryo = FLAPDT_SUB[Flap_Sub_Num].ppc_chusi_ryo;/* �񐔌�or�v���y�C�h�J�[�h              */
				PayData.ppc_chusi = PayData_Sub.pay_ryo;				/* �񐔌�or�v���y�C�h�J�[�h���~�o�b�t�@       */
				if(PayData_Sub.pay_ryo){
					Pay_Flg = 1;		//�v���y�C�h�A�񐔌��g�p�׸�
				}
			}
// MH810100(S) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
			l_dat = ryo_buf.zenkai;
			if (l_dat > 999990){
				l_dat = 999990;
			}

																	/*                                            */
			if (l_dat > 0){
				rkn_c = &req_crd[req_crd_cnt];						/*  ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@  */
				rkn_c->param = RY_RWR;								/*  �p�����[�^�Z�b�g�i�j      */
				rkn_c->data[0] = l_dat;								/*  ���z�Z�b�g                                */
				req_crd_cnt++;										/*  ���Ԍ��ǂݎ�莞���f�[�^�v�������t�o      */
				PayData.PRTwari.BIT.RWARI = 1;							/* �O�񗿋������t���O                         */
			}
// MH810100(E) 2020/06/02 �y�A���]���w�E�����z�Đ��Z���ɁA�c�z�ȏォ���ԗ���������QR�������s���Ɛ����z���s���Ȓl�ƂȂ�(No.02-0014)
			prk_kind = 1;												/*  ��{�Z�b�g                                */
		break;															/*                                            */
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�

		// ������
		case V_FUN:
			rkn_p->param = RY_FKN;			// �p�����[�^�Z�b�g

			if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE )
			{
// �d�l�ύX(S) K.Onodera 2016/11/07 ���u���Z�t�H�[�}�b�g�ύX�Ή�
//				rysyasu = LockInfo[ryo_buf.pkiti-1].ryo_syu;
//				syashu = rysyasu;
// MH810100(S) K.Onodera 2020/03/04 �Ԕԃ`�P�b�g���X(#3985 ���u����̌�����Z���̗̎��؂ɗ�����ʂ��󎚂���Ȃ��s��C��)
//				if( !g_PipCtrl.stRemoteFee.Type ){
//					rysyasu = LockInfo[ryo_buf.pkiti-1].ryo_syu;
//					syashu = rysyasu;		// �t���b�v�ɐݒ肳�ꂽ���
//				}else{
// MH810100(E) K.Onodera 2020/03/04 �Ԕԃ`�P�b�g���X(#3985 ���u����̌�����Z���̗̎��؂ɗ�����ʂ��󎚂���Ȃ��s��C��)
					// �v���Ɏ�ʂ��Z�b�g����Ă���H
					if( g_PipCtrl.stRemoteFee.RyoSyu >= 1 && g_PipCtrl.stRemoteFee.RyoSyu <= 12 ){
						rysyasu = g_PipCtrl.stRemoteFee.RyoSyu;
					}else{
						rysyasu = prm_get( COM_PRM, S_CEN, 40, 2, 1 );
						if( !rysyasu ){
							rysyasu = syashu = 1;	// A��Œ�
						}
						g_PipCtrl.stRemoteFee.RyoSyu = rysyasu;
					}
// MH810100(S) K.Onodera 2020/03/04 �Ԕԃ`�P�b�g���X(#3985 ���u����̌�����Z���̗̎��؂ɗ�����ʂ��󎚂���Ȃ��s��C��)
//				}
// MH810100(E) K.Onodera 2020/03/04 �Ԕԃ`�P�b�g���X(#3985 ���u����̌�����Z���̗̎��؂ɗ�����ʂ��󎚂���Ȃ��s��C��)
// �d�l�ύX(E) K.Onodera 2016/11/07 ���u���Z�t�H�[�}�b�g�ύX�Ή�
				cr_tkpk_flg = 2;
			}
			prk_kind = 1;					// ��{�Z�b�g
			break;
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
																		/*                                            */
		case V_SAK:														/*  �T�[�r�X��                                */
			svs = &vl_svs;												/**                                           */
			if( svs->pkno == pno )			prk_kind = 1	;			/** ���ԏ�m���D����v�Ȃ�                    */
			else if( svs->pkno == pnk )		prk_kind = 2    ;			/** �g�����ԏ�m���D����v�Ȃ�                */
			else if( svs->pkno == pnk2 )	prk_kind = 3    ;			/** �g��2���ԏ�m���D����v�Ȃ�               */
			else if( svs->pkno == pnk3 )	prk_kind = 4    ;			/** �g��3���ԏ�m���D����v�Ȃ�               */
			else							prk_kind = 0 	;			/**                                           */
																		/*                                            */
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
			mise_num = (ushort)( vl_svs.mno & 0x7fff );					/* �X(�|����)���̂ݎ�o��                     */
			mise_use = (ushort)vl_svs.mno;								/* long�^�ւ̃L���X�g���l����ushort�^��       */
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// MH810100(S) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
			l_dat = 0;
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�v���̕ύX)
//			if(vl_svs.sno > 50){
//				// +50���ēX�����̊�����ނ��i�[���Ă���̂ŕ�������i�T�[�r�X���̏ꍇ�́A1�`26�j
//				l_dat = vl_svs.sno -50;
			if ( vl_svs.sno > 30 ) {
				// ���X�܊�����ʂƓX������ނ�30�ȏ���Z���Ċi�[���Ă��邽�ߕ�������
				l_dat = vl_svs.sno - 30;
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�v���̕ύX)
				vl_svs.sno = 0;
			}
// MH810100(E) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
			if( vl_svs.sno == 0 )										/*  �X�m���D�����v��                          */
			{															/*                                            */
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
//				sdat = (char)CPrmSS[S_STO][3+((vl_svs.mno-1)*3)];		/*  �؊���̎Ԏ���擾                        */
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�v���̕ύX)
//				sdat = (char)CPrmSS[S_STO][3+((mise_num-1)*3)];			/*  �؊���̎Ԏ���擾                        */
				if ( mise_num > MISE_NO_CNT ) {
					// ���X�܊����ݒ肩��擾
					sdat = (char)prm_get(COM_PRM, S_TAT, (mise_num - 69), 2, 1);
				} else {
					// �X�����ݒ肩��擾
					sdat = (char)prm_get(COM_PRM, S_STO, (mise_num * 3), 2, 1);
				}
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�v���̕ύX)
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
				if( sdat != 0 ) 										/*  �Ԏ�؂�ւ��L��H                        */
				{														/*                                            */
					rysyasu = sdat;										/*  �Ԏ�Z�b�g                                */
					syashu = rysyasu;									/*  �Ԏ�Z�b�g                                */
																		/*                                            */
					rkn_p->syubt = syashu;								/*  �Ԏ�Z�b�g                                */
					rkn_p->param = RY_KCH;								/*  �p�����[�^�Z�b�g�i�Ԏ�؊��j              */
					sv_tim = Prm_RateDiscTime[syashu-1];				/*  ��ʎ��Ԋ���                              */
					rkn_p->data[0] = sv_tim;							/*  ��ʎ��Ԋ����Z�b�g                        */
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
//					rkn_p->data[1] = ( (long)3 << 16 | (long)vl_svs.mno);/*  ���f�[�^�ƓX�m���D�Z�b�g                 */
					rkn_p->data[1] = ( (long)3 << 16 | (long)mise_num);	/*  ���f�[�^�ƓX�m���D�Z�b�g                 */
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
																		/*                                            */
					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(sdat-1)),2,5 ) )/*  �Ԏ�؊��旿����ʂ̐ݒ肠��          */
					{													/*                                            */
						rkn_c = &req_crd[req_crd_cnt];					/*  ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@  */
						rkn_c->param = RY_KAK;							/*  �|�����茔�Z�b�g                          */
						if( prk_kind == 2 )								/*  �g�����Ԃm���D�̏ꍇ �p�����[�^�Z�b�g     */
							rkn_c->param = RY_KAK_K;					/*                                            */
						else if( prk_kind == 3 )						/*  �g������2�m���D�̏ꍇ �p�����[�^�Z�b�g    */
							rkn_c->param = RY_KAK_K2;					/*                                            */
						else if( prk_kind == 4 )						/*  �g������3�m���D�̏ꍇ �p�����[�^�Z�b�g    */
							rkn_c->param = RY_KAK_K3;					/*                                            */
// MH810100(S) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
//// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
////						rkn_c->data[0] = vl_svs.mno;					/*  �X�m���D�Z�b�g                            */
//						rkn_c->data[0] = (long)mise_use;				/*  �g�p�ς݃t���O�{�X���Z�b�g                */
//// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�v���̕ύX)
//						rkn_c->data[0] = l_dat << 16;					/*  ������ރZ�b�g                           */
						if ( l_dat > 20 ) {
							// �X�����̊�����ނ�50���Z���Ċi�[���Ă��邽�ߍX��20���Z���ĕ�������
							rkn_c->data[0] = ((l_dat - 20) << 16) & 0x00ff0000;		/*  �X������ރZ�b�g              */
						} else if ( l_dat > 0 ) {
							rkn_c->data[0] = (l_dat << 24) & 0xff000000;			/*  ���X�܊�����ʃZ�b�g          */
						}
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�v���̕ύX)
						rkn_c->data[0] |= (long)mise_use;				/*  �g�p�ς݃t���O�{�X���Z�b�g           		*/
// MH810100(E) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
						rkn_c->data[1] = 1;								/*  �����Z�b�g                                */
						req_crd_cnt++;									/*  ���Ԍ��ǂݎ�莞���f�[�^�v�������t�o      */
					}													/*                                            */
				}														/*                                            */
				else													/*                                            */
				{														/*                                            */
					rkn_p->param = RY_KAK;								/*  �|�����茔�Z�b�g                          */
					if( prk_kind == 2 )									/*  �g�����Ԃm���D�̏ꍇ �p�����[�^�Z�b�g     */
						rkn_p->param = RY_KAK_K;						/*                                            */
					else if( prk_kind == 3 )							/*  �g������2�m���D�̏ꍇ �p�����[�^�Z�b�g    */
						rkn_p->param = RY_KAK_K2;						/*                                            */
					else if( prk_kind == 4 )							/*  �g������3�m���D�̏ꍇ �p�����[�^�Z�b�g    */
						rkn_p->param = RY_KAK_K3;						/*                                            */
// MH810100(S) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
//// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
////					rkn_p->data[0] = vl_svs.mno;						/*  �X�m���D�Z�b�g                            */
//					rkn_p->data[0] = (long)mise_use;					/*  �g�p�ς݃t���O�{�X���Z�b�g                */
//// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�v���̕ύX)
//					rkn_p->data[0] = l_dat << 16;						/*  ������ރZ�b�g                          */
					if ( l_dat > 20 ) {
						// �X�����̊�����ނ�50���Z���Ċi�[���Ă��邽�ߍX��20���Z���ĕ�������
						rkn_p->data[0] = ((l_dat - 20) << 16) & 0x00ff0000;		/*  �X������ރZ�b�g                  */
					} else if ( l_dat > 0 ) {
						rkn_p->data[0] = (l_dat << 24) & 0xff000000;			/*  ���X�܊�����ʃZ�b�g              */
					}
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�Z�v���̕ύX)
					rkn_p->data[0] |= (long)mise_use;					/*  �g�p�ς݃t���O�{�X���Z�b�g              */
// MH810100(E) 2020/09/09 #4755 �X�����̊�����ޑΉ��i�y�A���]���w�E�����zNT-NET���Z�f�[�^�Ɋ������2,102�̊������̒l���i�[����Ȃ�(02-0045)�j
					rkn_p->data[1] = 1;									/*  �����Z�b�g                                */
				}														/*                                            */
			}															/*                                            */
			else														/*  �T�[�r�X��                                */
			{															/*                                            */
				sdat = (char)CPrmSS[S_SER][3+((vl_svs.sno-1)*3)];		/*  �؊���̎Ԏ���擾                        */
				if( sdat != 0 ) 										/*  �Ԏ�؂�ւ��L��H                        */
				{														/*                                            */
					rysyasu = sdat;										/*  �Ԏ�Z�b�g                                */
					syashu = rysyasu;									/*  �Ԏ�Z�b�g                                */
																		/*                                            */
					rkn_p->syubt = syashu;								/*  �Ԏ�Z�b�g                                */
					rkn_p->param = RY_KCH;								/*  �p�����[�^�Z�b�g�i�Ԏ�؊��j              */
					sv_tim = Prm_RateDiscTime[syashu-1];				/*  ��ʎ��Ԋ���                              */
					rkn_p->data[0] = sv_tim;							/*  ��ʎ��Ԋ����Z�b�g                        */
					rkn_p->data[1] = ( (long)1 << 16 | (long)vl_svs.sno);/*  ���f�[�^�ƃT�[�r�X���̎�ރZ�b�g         */
																		/*                                            */
					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(sdat-1)),2,5 ) )/*  �Ԏ�؊��旿����ʂ̐ݒ肠��          */
					{													/*                                            */
						rkn_c = &req_crd[req_crd_cnt];					/*  ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@  */
						rkn_c->param = RY_SKC;							/*  �p�����[�^�Z�b�g                          */
						if( prk_kind == 2 )								/*  �g�����Ԃm���D�̏ꍇ �p�����[�^�Z�b�g     */
							rkn_c->param = RY_SKC_K;					/*                                            */
						else if( prk_kind == 3 )						/*  �g������2�m���D�̏ꍇ �p�����[�^�Z�b�g    */
							rkn_c->param = RY_SKC_K2;					/*                                            */
						else if( prk_kind == 4 )						/*  �g������3�m���D�̏ꍇ �p�����[�^�Z�b�g    */
							rkn_c->param = RY_SKC_K3;					/*                                            */
						l_dat = vl_svs.sno;								/*  ��ރZ�b�g                                */
						rkn_c->data[1] = 1;								/*  �����Z�b�g                                */
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
//						rkn_c->data[0] =								/*  ��ނƊ|������m���D�Z�b�g                */
//								( (l_dat << 16) | (long)vl_svs.mno  );	/*                                            */
						rkn_c->data[0] = l_dat << 16;					/*  ��ރZ�b�g                                */
						rkn_c->data[0] |= (long)mise_use;				/*  �g�p�ς݃t���O�{�|���懂�Z�b�g            */
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
						req_crd_cnt++;									/*  ���Ԍ��ǂݎ�莞���f�[�^�v�������t�o      */
					}													/*                                            */
				}														/*                                            */
				else													/*                                            */
				{														/*                                            */
					rkn_p->param = RY_SKC;								/*  �p�����[�^�Z�b�g                          */
					if( prk_kind == 2 )									/*  �g�����Ԃm���D�̏ꍇ �p�����[�^�Z�b�g     */
						rkn_p->param = RY_SKC_K;						/*                                            */
					else if( prk_kind == 3 )							/*  �g������2�m���D�̏ꍇ �p�����[�^�Z�b�g    */
						rkn_p->param = RY_SKC_K2;						/*                                            */
					else if( prk_kind == 4 )							/*  �g������3�m���D�̏ꍇ �p�����[�^�Z�b�g    */
						rkn_p->param = RY_SKC_K3;						/*                                            */
					l_dat = vl_svs.sno;									/*  ��ރZ�b�g                                */
					rkn_p->data[1] = 1;									/*  �����Z�b�g                                */
// MH810100(S) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
//					rkn_p->data[0] =									/*  ��ނƊ|������m���D�Z�b�g                */
//							( (l_dat << 16) | (long)vl_svs.mno  );		/*                                            */
					rkn_p->data[0] = l_dat << 16;						/*  ��ރZ�b�g                                */
					rkn_p->data[0] |= (long)mise_use;					/*  �g�p�ς݃t���O�{�|���懂�Z�b�g            */
// MH810100(E) K.Onodera 2020/02/04 �Ԕԃ`�P�b�g���X(�����ςݑΉ�)
				}														/*                                            */
			}															/*                                            */
		break;															/*                                            */
																		/*                                            */
		case V_SYU:														/*  �T�[�r�X���^������i��ʐ؊��j            */
			ans = ec191( V_SYU,0,syashu,0 );							/*  �Ԏ�؊��b�g�d�b�j                        */
		break;															/*                                            */
																		/*                                            */
		case V_KAM:														/*  �񐔌��i�����j                            */
		case V_KAG:														/*  �񐔌��i�����j                            */
			kas = &vl_kas;												/**                                           */
			if( kas->pkno == pno )			prk_kind = 1;				/** ���ԏ�m���D����v�Ȃ�      ��{�Z�b�g    */
			else if( kas->pkno == pnk )		prk_kind = 2;				/** �g�����ԏ�m���D����v�Ȃ�  �g���Z�b�g    */
			else if( kas->pkno == pnk2 )	prk_kind = 3;				/** �g��2���ԏ�m���D����v�Ȃ�               */
			else if( kas->pkno == pnk3 )	prk_kind = 4;				/** �g��3���ԏ�m���D����v�Ȃ�               */
			else							prk_kind = 0 ;				/** ���ԏ�NO.����v���Ȃ��Ȃ�                 */
																		/*                                            */
			rkn_p->param = RY_FRE;										/*  �p�����[�^�Z�b�g                          */
			if( prk_kind == 2 )				rkn_p->param = RY_FRE_K;	/*  �g�����Ԃm���D�̏ꍇ  �p�����[�^�Z�b�g    */
			else if( prk_kind == 3 )		rkn_p->param = RY_FRE_K2;	/*  �g������2�m���D�̏ꍇ �p�����[�^�Z�b�g    */
			else if( prk_kind == 4 )		rkn_p->param = RY_FRE_K3;	/*  �g������3�m���D�̏ꍇ �p�����[�^�Z�b�g    */
			rkn_p->data[0] = vl_kas.nno;								/*  �c��񐔃Z�b�g                            */
			rkn_p->data[1] = vl_kas.tnk;								/*  �P�񐔒P���Z�b�g                          */
			if( vl_now == V_KAM)			rkn_p->data[1] 	= 0xff;		/*  �񐔌��i�����j  �P�񐔒P���Z�b�g�i�e�e�j  */
		break;															/*                                            */
																		/*                                            */
		case V_TST:														/*  ������i���Ԍ����p�Ȃ��j                  */
		case V_TSC:														/*  ������i    �V    �L��j                  */
			if( vl_now == V_TST)										/** ������i���Ԍ����p�Ȃ��j                  */
			{															/**                                           */
				// ���b�N���u�^�C�v�̂��߁A���Ԍ����p�Ȃ��͋@�\���Ȃ��B�g�p���鎞�ɍ�肱�ގ��BMH544401�ǋL
				if( cr_dat_n == RID_APS )								/*                                            */
				{														/**                                           */
					for( i = 0; i < 4; i ++ ) 							/**                                           */
					{													/**                                           */
						tim_t[i] = (short)tsn_tki.sttim[i];				/**                                           */
					}													/**                                           */
					cm13( 1,tim_t );									/** �N�����͈̓`�F�b�N                        */
					if( tsn_tki.status != 0 )							/**                                           */
					{													/**                                           */
						org_in.year	= tim_t[4];							/** ���ɔN�����ݔN�Ƃ���                      */
						org_in.mon	= (char)tim_t[0];					/** �X�e�[�^�X�����i���j                      */
						org_in.day	= (char)tim_t[1];					/** �X�e�[�^�X�����i���j                      */
						org_in.hour	= (char)tim_t[2];					/** �X�e�[�^�X�����i���j                      */
						org_in.min	= (char)tim_t[3];					/** �X�e�[�^�X�����i���j                      */
						org_in.week	=									/** ���ɗj���Z�b�g                            */
							(char)youbiget( tim_t[4],tim_t[0],tim_t[1]);/**                                           */
					}													/**                                           */
					else 												/** '93.8.3 tazou start                       */
					{													/**                                           */
						org_in.year = hzuk.y;			 				/** ���ɔN�����ݔN�Ƃ���                      */
						org_in.mon 	= hzuk.m;      		 				/** ���ɔN�����ݎ����i���j                    */
						org_in.day 	= hzuk.d;      		 				/** ���ɔN�����ݎ����i���j                    */
						org_in.week	= hzuk.w;			 				/** ���ɗj���Z�b�g                            */
						org_in.hour = jikn.t;      		 				/** ���ɔN�����ݎ����i���j                    */
						org_in.min 	= jikn.m;       					/** ���ɔN�����ݎ����i���j                    */
					}													/**                                           */
				}														/**                                           */
			}															/**                                           */
			if( ryo_buf.pass_zero == 1 ){								/** ����������O�~���Z�v������                */
				memcpy( &org_in, &org_ot, sizeof( struct CAR_TIM ) );	/** ���o�ɓ������Ƃ���                        */
			}															/**                                           */
			if( cr_dat_n == RID_APS )									/** �`�o�r����̏ꍇ                          */
			{															/**                                           */
				if( tsn_tki.pkno == pno )				prk_kind = 1;	/** ��{���ԏꇂ����v�Ȃ�  ��{�Z�b�g        */
				else if( tsn_tki.pkno == pnk )			prk_kind = 2;	/** �g�����ԏꇂ����v�Ȃ�  �g���Z�b�g        */
				else if( tsn_tki.pkno == pnk2 )			prk_kind = 3;	/** �g��2���ԏꇂ����v�Ȃ�                   */
				else if( tsn_tki.pkno == pnk3 )			prk_kind = 4;	/** �g��3���ԏꇂ����v�Ȃ�                   */
				else									prk_kind = 0;	/**                                           */
			}															/**                                           */
			if( cr_dat_n == RID_JIS )					prk_kind = 1;	/** �i�h�r����̏ꍇ                          */
			rkn_p->param = RY_PKC;										/*  �p�����[�^�Z�b�g                          */
			if( prk_kind == 2 )			rkn_p->param = RY_PKC_K;		/*  �g�����Ԃm���D�̏ꍇ  �p�����[�^�Z�b�g    */
			else if( prk_kind == 3 )	rkn_p->param = RY_PKC_K2;		/*  �g������2�m���D�̏ꍇ �p�����[�^�Z�b�g    */
			else if( prk_kind == 4 )	rkn_p->param = RY_PKC_K3;		/*  �g������3�m���D�̏ꍇ �p�����[�^�Z�b�g    */
			rkn_p->data[0] = tsn_tki.kind;								/*  ������                                  */
																		/*                                            */
 			cr_tkpk_flg = 1  ;											/*  ������g�p���     						  */
			if( tki_pk_use <= 2 )		tki_pk_use ++ 	;				/*  ���/���Ԍ��g�p   	����+1		      	  */
																		/*                                            */
																		/*                                            */
																		/*                                            */
			if( tsn_tki.status == 1  	&& 								/*  �ð���͏o�ɒ�                             */
				tkcyu_ichi 			 	&& 								/*  ���~����ް��L��						  */
				vl_now 		   == V_TST 	)							/*  ���Ԍ����p�Ȃ�                            */
			{															/*                                            */
				// ���b�N���u�^�C�v�̂��߁A���Ԍ����p�Ȃ��͋@�\���Ȃ��B�g�p���鎞�ɍ�肱�ގ��BMH544401�ǋL
				if( (no = (char)tki_cyusi.dt[tkcyu_ichi-1].mno) != 0 )	/*  ������ǂݎ�莞  �X�f�[�^�L��            */
				{														/*                                            */
					sdat = (char)CPrmSS[S_STO][3+((vl_svs.mno-1)*3)];	/* �؊���̎Ԏ���擾						*/
																		/*                                            */
					if( sdat != 0 )										/*  �Ԏ�؂�ւ��L��H                        */
					{													/*                                            */
						ans = ec191( V_CHG,no,sdat,0 );					/*  �Ԏ�؊��b�g�d�b�j                        */
					}													/*                                            */
					rkn_t 		 	= &req_tkc[req_tkc_cnt];			/*  ������~�ް��v���i�[�ޯ̧  			   	  */
					rkn_t->param 	= RY_CMI;							/*  �p�����[�^�Z�b�g�i�X�m���D��������j      */
					rkn_t->data[0] 	= no;								/*  �X�m���D�Z�b�g                            */
					rkn_t->data[1] 	= 1;								/*  �����Z�b�g                                */
					req_tkc_cnt		++;									/*  ��������~���Z�v���[���� UP  	    	  */
				}														/*                                            */
																		/*                                            */
				mai = (uchar)prm_get(COM_PRM, S_DIS, 1, 2, 1);
				for( i = 0; i < 3; i++ )								/*  �T�[�r�X���핪                            */
				{														/*                                            */
					if( (no = tki_cyusi.dt[tkcyu_ichi-1].sa[i]) != 0 )	/*  �`�`�b�T�[�r�X���̖����L��H              */
					{													/*                                            */
						if( mai > card_use[USE_SVC] )					/*  �T�[�r�X���g�p�����̌��x�`�F�b�N          */
						{												/*                                            */
							if( mai <= (no + card_use[USE_SVC]) )		/*  �T�[�r�X���g�p���������x���z���Ă���      */
								no = (char)(mai - card_use[USE_SVC]);	/*  ���������Z�b�g                            */
																		/*                                            */
							switch( CPrmSS[S_SER][(1+3*i)] )			/* ���޽�������擾                            */
							{											/*                                            */
								case 1:									/*                                            */
								case 2:									/*                                            */
								case 4:									/*  ���z��������                              */
									rkn_t 		 	= &req_tkc[req_tkc_cnt];/*  ������~�ް��v���i�[�ޯ̧             */
									rkn_t->param 	= RY_CSK;			/*  �p�����[�^�Z�b�g�i���Z���~�T�[�r�X���j    */
									l_dat 		 	= i+1;				/*  ��ރZ�b�g                                */
									rkn_t->data[1] 	= no;				/*	�����Z�b�g                                */
									rkn_t->data[0] 	= l_dat << 16;		/*	��ނƊ|������m���D�Z�b�g                */
									req_tkc_cnt		++;					/*	��������~���Z�v���[�����t�o              */
									card_use[USE_SVC] += no;			/*                                            */
								break;									/*                                            */
																		/*                                            */
								case 3:									/*                                            */
									rkn_t 		 	= &req_tkc[req_tkc_cnt];/*  ������~�ް��v���i�[�ޯ̧         	  */
									rkn_t->param 	= RY_CSK;			/*  �p�����[�^�Z�b�g�i���Z���~�T�[�r�X���j    */
									l_dat 		 	= i+1;				/*  ��ރZ�b�g                                */
									rkn_t->data[1] 	= no;				/*	�����Z�b�g                                */
									rkn_t->data[0] 	= l_dat << 16;		/*	��ނƊ|������m���D�Z�b�g                */
									req_tkc_cnt		++;					/*	��������~���Z�v���[�����t�o              */
								break;									/*                                            */
							}											/*                                            */
						}												/*                                            */
					}													/*                                            */
				}														/*                                            */
				if(( tki_cyusi.dt[tkcyu_ichi-1].kry != 0 )				/*	����������������L��H                    */
						&&(CPrmSS[S_PRP][1] != 0L))						/*  �񐔌�orP���ގg�p����					*/
				{														/*                                            */
					rkn_t = &req_tkc[req_tkc_cnt];						/*	������~�ް��v���i�[�ޯ̧                 */
					rkn_t->param = RY_KGK;								/*	�p�����[�^�Z�b�g�i����������������j      */
					rkn_t->data[0] = tki_cyusi.dt[tkcyu_ichi-1].kry;	/*	���z�Z�b�g                                */
					rkn_t->data[1]= 0L;									/*  �Ƃ肠�����񐔌��g�p��=0�Ƃ���			*/
					if( CPrmSS[S_PRP][1] == 1L )						/*	�ݒ肪P���ގg�p�� =1					*/
						rkn_t->data[1]= 1L;								/*  P���ގg�p�ɕύX����					*/
					req_tkc_cnt++;										/*	��������~���Z�v���[�����t�o              */
				}														/*                                            */
			}															/*                                            */
																		/*                                            */
		break;															/*                                            */
																		/*                                            */
		case V_PRI:														/*	�v���y�C�h�J�[�h                          */
			prk_kind = 1;												/** ��{�Z�b�g        						  */
			if( tsn_prp.pakno == pnk )			prk_kind = 2;			/** �g�����ԏꇂ����v�Ȃ�  �g���Z�b�g        */
			else if( tsn_prp.pakno == pnk2 )	prk_kind = 3;			/** �g��2���ԏꇂ����v�Ȃ�                   */
			else if( tsn_prp.pakno == pnk3 )	prk_kind = 4;			/** �g��3���ԏꇂ����v�Ȃ�                   */
																		/*                                            */
			rkn_p->param = RY_PCO;										/*	�p�����[�^�Z�b�g                          */
			if( prk_kind == 2 )			rkn_p->param = RY_PCO_K;		/*	�g�����Ԃm���D�̏ꍇ  �p�����[�^�Z�b�g    */
			else if( prk_kind == 3 )	rkn_p->param = RY_PCO_K2;		/*  �g������2�m���D�̏ꍇ �p�����[�^�Z�b�g    */
			else if( prk_kind == 4 )	rkn_p->param = RY_PCO_K3;		/*  �g������3�m���D�̏ꍇ �p�����[�^�Z�b�g    */
			rkn_p->data[0] = tsn_prp.zan;								/*	�c�z�Z�b�g                                */
			rkn_p->data[1] = (unsigned long)tsn_prp.sei;				/*	���Z�P�ʃZ�b�g                            */
																		/*                                            */
			if( prp_rno_cnt < 7 )										/*	����߲�ރJ�[�h�m���D�o�b�t�@���ȉ�        */
			{															/*                                            */
				prp_rno_buf[prp_rno_cnt] = tsn_prp.rno;					/*	�o�o�J�[�h�m���D�Z�[�u                    */
				prp_rno_cnt++;											/*	�o�o�J�[�h�m���D�o�b�t�@�J�E���g�t�o      */
			}															/*                                            */
		break;															/*                                            */
		case V_EMY:														/*	�d�q�}�l�[		                          */
			prk_kind = 1;												/** ��{�Z�b�g        						  */
			rkn_p->param = RY_EMY;										/*	�p�����[�^�Z�b�g                          */
		break;															/*                                            */
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case V_EQR:														/*	QR�R�[�h		                          */
			prk_kind = 1;												/** ��{�Z�b�g        						  */
			rkn_p->param = RY_EQR;										/*	�p�����[�^�Z�b�g                          */
			break;														/*                                            */
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		case	V_CRE:									//�N���W�b�g�J�[�h
			rkn_p->param = RY_CRE;						//�v�Z���
		break;

		case	V_GNG:													/* ���z���Z									  */
// �s��C��(S) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
//// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
//		case 	V_DIS_FEE:												// ���u���Z��
//		case	V_DIS_TIM:
//// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
// �s��C��(E) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
			rkn_p->param = RY_GNG;										/* �v�Z���									  */
		break;
// �s��C��(S) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
		// ���u���Z(���z�w��)�Ŋ������z����
		case V_DIS_FEE:
			rkn_p->param = RY_GNG_FEE;
			break;
		// ���u���Z(���Ɏ����w��)�Ŋ������z����
		case V_DIS_TIM:
			rkn_p->param = RY_GNG_TIM;
			break;
// �s��C��(E) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
		case	V_FRK:													/* �U�֐��Z									  */
			rkn_p->param = RY_FRK;										/* �v�Z���									  */
		break;
		case	V_SSS:													/* �C�����Z									  */
																		/*											  */
			if( (vl_frs.syubetu != LockInfo[vl_frs.lockno-1].ryo_syu) ){/* �C�����̐��Z�������̎Ԏ�ƏC�����̐ݒ��̎Ԏ킪�قȂ� */
				// �Ԏ�؊��Z�b�g										/*											  */
				rysyasu = vl_frs.syubetu;								/* �C�����̎Ԏ�Z�b�g                         */
				
				syashu = rysyasu;										/* �Ԏ�Z�b�g                                 */
																		/*											  */
				rkn_p->syubt = syashu;									/* �Ԏ�Z�b�g                                 */
				rkn_p->param = RY_KCH;									/* �p�����[�^�Z�b�g�i�Ԏ�؊��j               */
				sv_tim = Prm_RateDiscTime[syashu-1];					/* ��ʎ��Ԋ���                               */
				rkn_p->data[0] = sv_tim;								/* ��ʎ��Ԋ����Z�b�g                         */
				rkn_p->data[1] = 0;										/* ���f�[�^�ƃT�[�r�X���̎�ރZ�b�g           */
																		/*											  */
				// �U�֊z�Z�b�g											/*											  */
				rkn_c = &req_crd[req_crd_cnt];							/* ���Ԍ��ǂݎ�莞���f�[�^�v���i�[�o�b�t�@   */
				rkn_c->param = RY_SSS;									/* �v�Z���									  */
				req_crd_cnt++;											/* ���Ԍ��ǂݎ�莞���f�[�^�v�������t�o       */
			}else{														/*											  */
				rkn_p->param = RY_SSS;									/* �v�Z���									  */
			}															/*											  */
		break;															/*											  */
	}																	/*                                            */
																		/*                                            */
	 ans = OK ;															/*	�A���T�[���n�j                            */
																		/*                                            */
	return( ans );														/*                                            */
}																		/**********************************************/
