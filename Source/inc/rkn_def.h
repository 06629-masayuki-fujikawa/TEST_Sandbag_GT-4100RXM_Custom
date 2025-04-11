/*[]----------------------------------------------------------------------[]*/
/*| Rkn_def.h                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : R.HARA                                                   |*/
/*| Date        : 2001-11-22                                               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#ifndef _RKN_DEF_H_
#define _RKN_DEF_H_
												/*							*/
#define		TKI_MAX			15					/* ������					*/
#define		SKI_MAX			3					/* �T�[�r�X�������ύX		*/
#define		SVS_MAX			15					/* �T�[�r�X����ő吔		*/
												/*							*/
#define		V_CHM			24					/* ���Ԍ��@���Z�O			*/
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
#define		V_FUN			25					/* �������@�@�@�@�@�@�@�@�@ */
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
#define		V_CHG			27					/* ���Ԍ��@���Z��			*/
#define		V_CHS			28					/* ���~��					*/
#define		V_CHU			29					/* �|����					*/
#define		V_KAM			30					/* �񐔌��@����				*/
#define		V_KAG			31					/* �񐔌��@���z				*/
#define		V_SAK			32					/* �T�[�r�X��				*/
#define		V_SYU			33					/* ��ʐ؊��@������@�T�[�r�X��	*/
#define		V_PRI			34					/* �v���y�C�h�J�[�h			*/
#define		V_TSC			35					/* ������i���Ԍ������j		*/
#define		V_TST			42					/* ������i���Ԍ����p�j		*/
#define		V_CRE			43					/* �N���W�b�g�J�[�h */
#define		V_EMY			50					/* �d�q�}�l�[				*/
#define		V_GNG			51					/* ���z���Z 				*/
#define		V_FRK			52					/* �U�֐��Z 				*/
#define		V_SSS			53					/* �C�����Z					*/
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define		V_EQR			54					/* QR�R�[�h 				*/
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
#define		V_DIS_FEE		70					// ���u���Z������(���z�w��)
#define		V_DIS_TIM		71					// ���u���Z������(���Ɏ����w��)
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
												/*							*/
/* �J�[�h���[�_�[�R�}���h	*/
#define		RID_JIS			0x41				/* �i�h�r������i�`�s�l�W������j	*/
#define		RID_CKM			0x24				/* ���Ԍ��i���Z�O�j			*/
#define		RID_CKG			0x25				/* ���Ԍ��i���Z��j			*/
#define		RID_CKS			0x26				/* ���Ԍ��i���Z�@�ɂ����鐸�Z���~�j	*/
#define		RID_CKU			0x27				/* ���Ԍ��i�|���胉�C�^�ɂĊ|���肳�ꂽ���j	*/
#define		RID_CKT			0x28				/* ���Ԍ��i�Đ��Z�̐��Z���~���j	*/
#define		RID_CKZ			0x29				/* ���Ԍ��i�g�p�ό��@�ݒ�4=2�Ő��Z�ς݂̌��j	*/
#define		RID_SAI			0x2a				/* �Ĕ��s��					*/
#define		RID_FUN			0x2b				/* �������i�e�X�g���j		*/
#define		RID_KAI			0x2c				/* �񐔌�					*/
#define		RID_SAV			0x2d				/* �T�[�r�X��				*/
#define		RID_APS			0x1a				/* �`�o�r�����				*/
#define		RID_RCM			0x34				/* �������Ԍ��i���Z�O�j		*/
#define		RID_RCG			0x35				/* �������Ԍ��i���Z��j		*/
#define		RID_RCS			0x36				/* �������Ԍ��i���Z�@�ɂ����鐸�Z���~�j	*/
#define		RID_RCU			0x37				/* �������Ԍ��i�|���胉�C�^�ɂĊ|���茔�j	*/
#define		RID_RCT			0x38				/* �������Ԍ��i�Đ��Z�̐��Z���~���j	*/
												/*							*/
/* �����v�Z�f�t�@�C��		*/
#define		RY_TSA			0					/* ���Ԏ�������̗����v�Z	*/
#define		RY_OTR			1					/* ����v�Z				*/
#define		RY_TWR			2					/* ���Ԋ�������				*/
#define		RY_RWR			3					/* ������������				*/
#define		RY_FRE			4					/* �񐔌�����				*/
#define		RY_SKC			5					/* �T�[�r�X������			*/
#define		RY_PCO			6					/* �v���y�C�h�J�[�h����		*/
#define		RY_PKC			7					/* ���������				*/
#define		RY_FKN			8					/* ����������				*/
#define		RY_BNC			9					/* ������Z����				*/
#define		RY_TAT			10					/* �ŃL�[����				*/
#define		RY_SNC			11					/* �X�m����������			*/
#define		RY_XXX			12					/* �ώZ����					*/
#define		RY_PWR			13					/* ����������				*/
#define		RY_KRY			14					/* �Œ藿������				*/
#define		RY_QUA			15					/* �ۏ؋�����				*/
#define		RY_RAY			16					/* �����߂��L�[				*/
#define		RY_KCH			17					/* �Ԏ�؂�ւ�				*/
#define		RY_RAG			18					/* ���O�^�C������			*/
#define		RY_KGK			19					/* ����������������			*/
#define		RY_CSK			20					/* ���Z���~�T�[�r�X������	*/
#define		RY_CPP			21					/* ���Z���~�v���y�C�h�J�[�h����	*/
#define		RY_CMI			22					/* ���Z���~�X�m����������		*/
#define		RY_KAK			23					/* �|��������				*/
#define		RY_FRE_K		24					/* �񐔌�����(�g������NO.)	*/
#define		RY_SKC_K		25					/* �T�[�r�X������(�g������NO.)	*/
#define		RY_PCO_K		26					/* �v���y�C�h�J�[�h����(�g������NO.)	*/
#define		RY_PKC_K		27					/* ���������(�g������NO.)	*/
#define		RY_SKC_K2		28					/* �T�[�r�X������(�g��2����NO.)	*/
#define		RY_SKC_K3		29					/* �T�[�r�X������(�g��3����NO.)	*/
#define		RY_KAK_K		30					/* �|��������(�g������NO.)	*/
#define		RY_KAK_K2		31					/* �|��������(�g��2����NO.)	*/
#define		RY_KAK_K3		32					/* �|��������(�g��3����NO.)	*/
#define		RY_WBC			33					/* ����������				*/
#define		RY_WBC_K		34					/* ����������(�g������NO.)	*/
#define		RY_WBC_K2		35					/* ����������(�g��2����NO.)	*/
#define		RY_WBC_K3		36					/* ����������(�g��3����NO.)	*/
#define		RY_PCO_K2		37					/* �v���y�C�h�J�[�h����(�g��2����NO.)	*/
#define		RY_PCO_K3		38					/* �v���y�C�h�J�[�h����(�g��3����NO.)	*/
#define		RY_FRE_K2		39					/* �񐔌�����(�g��2����NO.)	*/
#define		RY_FRE_K3		40					/* �񐔌�����(�g��3����NO.)	*/
#define		RY_PKC_K2		41					/* ���������(�g��2����NO.)	*/
#define		RY_PKC_K3		42					/* ���������(�g��3����NO.)	*/
#define		RY_CRE			43					/* �N���W�b�g�J�[�h */
#define		RY_EMY			44					/* �d�q�}�l�[����				*/
#define		RY_GNG			45					/* ���z���Z 				*/
#define		RY_FRK			46					/* �U�֐��Z 				*/
#define		RY_SSS			47					/* �C�����Z					*/
// �s��C��(S) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
#define		RY_GNG_FEE		48					// ���u���Z(���z�w��)���z����
#define		RY_GNG_TIM		49					// ���u���Z(���Ɏ����w��)���z����
// �s��C��(E) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define		RY_EQR			51					/* QR�R�[�h����				*/
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
												/*							*/
/* �V�X�f�t�@�C��			*/
#define		LOOP			1					/* �i�v���[�v�L��			*/
#define		NG				0xff				/* �m�f						*/
#define		OK				0x00				/* �n�j						*/
#define		OFF				0					/* �n�e�e					*/
#define		ON				1					/* �n�m						*/
#define		NOF				2					/* �n�m�^�n�e�e�ȊO			*/
#define		T_TX_TIM		0x77				/* �������Z���ԗL��		*/
												/*							*/
/* ���ԃf�t�@�C��			*/
#define		T_DAY			1440				/* ������̕�				*/

/* FCMAIN					*/
#define		wbcd2i(p)		(((char *)(p))[0]*10+((char *)(p))[1])	/* ܰ��BCD�l�������l�ϊ�*/
#define		cardLOST		0					/* �������Z��				*/
#define		cardREPUB		1					/* �Ĕ��s��					*/
#define		cardTICKET		2					/* ���Ԍ�					*/
#define		cardPASS		3					/* �����					*/
#define		cardPREPAY		4					/* �o�o�b					*/
#define		cardLIMITNUM	5					/* �񐔌� 					*/
#define		cardSERVICE		6					/* ���޽��					*/
#define 	cardANOTHER		10					/* ���g�p�s��				*/
												/*							*/
#define		Cmarching		0					/* ��:�}������				*/
#define		Creadable		1					/* ��:�ǎ�s��				*/
#define		Cparknum		2					/* ��:���ԇ�				*/
#define		Caskhost		3					/* ��:CM�⍇���L			*/
#define		Cillegal		4					/* ��:�s����				*/
#define		Coverend		5					/* ��:�����؂�				*/
#define		Cnearend		6					/* ��:�����؂�ԋ�			*/
												/*							*/
#define		SP_DAY_MAX		31					/* ���ʓ�					*/
#define		SP_RANGE_MAX	3					/* ���ʊ���					*/
#define		SP_HAPPY_MAX	12					/* ʯ�߰���ް				*/
#define		SP_YAER_MAX		6					/* ���ʔN����					*/
#define		SP_WEEK_MAX		12					/* ���ʗj��					*/


#define		MACRO_WARIBIKI_GO_RYOUKIN	((wk_ryo < (c_pay+e_pay)) ? 0L:(wk_ryo - c_pay - e_pay ))	/* ������ ���ԗ����Ƀv���y�������Z���� */

#define		NMAX_OVER		999990					// �m���ԍő�l��o�ߌ�̍ő嗿��
													// 
#define		PASS_WEEK_TAIKEI	0					// �̌n�؊�(����ѓr���ŗj���؊��Ȃ�)
#define		PASS_WEEK_DAY_SP	1					// ��(0:00)�؊�(����ѓr���ŗj���؊�����)
#define		PASS_WEEK_DAY_NO	2					// ��(0:00)�؊�(����ѓr���ŗj���؊��Ȃ�)
													// 
#define		NMAX_OUT_PIT		0					// �o�Ɋ
#define		NMAX_IN_PIT			1					// ���Ɋ
#define		NMAX_TAI_PIT		2					// �̌n�

#define		PAS_TIME			1					// ���ԑђ��
#define		PAS_DISC			2					// �������Ԓ��
#define		PAS_CANG			3					// �Ԏ�؊����
#define		FEE_TAIKEI_MAX		9					// �����̌n��
#define		RYO_CAL_EXTEND		(((char)CPrmSS[S_CLX][1]) == 1)
#define		SYUBET_MAX		12			// ��ʂ̐�(�`�`�k)
#define 	RYO_TAIKEI_SETCNT	300		// �P�����̌n�̐ݒ萔
#define 	RYO_SYUBET_SETCNT	20		// �P��ʂ̐ݒ萔

/* �ő嗿���^�C�v�̐ݒ�l��define�Œ�`����B */
/* �ȑO�̒��l��define�ɒu�������邱�Ƃ͂��Ȃ��̂ŁA�������Ȃǒ��ӂ��邱�� */
#define		SP_MX_NON			0		// �����ő嗿���Ȃ�
#define		SP_MX_INTIME		1		// ���Ɏ����ő�(6/12/24)
#define		SP_MX_TIME			2		// �����w������ő�
#define		SP_MX_WTIME			3		// �Q��ނ̎����w������ő�
#define		SP_MX_N_HOUR		4		// �����Ԃ���ő�
#define		SP_MX_BAND			5		// ���ԑя���i�����я���j
#define		SP_MX_N_MH_NEW		10		// �����Ԃ���ő�
#define		SP_MX_N_MHOUR_W		20		// �Q��ނ̂����Ԃ���ő�
#define		SP_MX_612HOUR		9		// 6h,12,Max 
#endif	// _RKN_DEF_H_
