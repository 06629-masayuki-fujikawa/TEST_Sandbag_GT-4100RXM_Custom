/*[]--------------------------------------------------------------------------------------------------[]*/
/*|		�ݒ�ð��ْ�`																				   |*/
/*[]--------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	prm_tbl.h																		   |*/
/*[]--------------------------------------------------------------------------------------------------[]*/
/*| Author      :																					   |*/
/*| Date        :	2005-03-10																		   |*/
/*| Update      :																					   |*/
/*|----------------------------------------------------------------------------------------------------|*/
/*| �E�ݒ�ð��ِ錾�y�ѐݒ�l�擾�Ɋ֘A�����������ߐ錾												   |*/
/*|																									   |*/
/*[]------------------------------------------------------- Copyright(C) 2005 AMANO Corp.-------------[]*/
#ifndef _PRM_TBL_H_
#define _PRM_TBL_H_

/*[]--------------------------------------------------------------------------------------------------[]*/
/*|	�ݒ�֘A��̧��																					   |*/
/*| �E�ݒ�֘A�̌Œ�l��o�^����B																	   |*/
/*[]--------------------------------------------------------------------------------------------------[]*/
																		/*								*/
#define		COM_PRM			0											/* �������Ұ�					*/
#define		PEC_PRM			1											/* �����Ұ�					*/
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/* ���Ұ������MAX																						*/
/*------------------------------------------------------------------------------------------------------*/
#define		C_PRM_SESCNT_MAX	61										/* ���Ұ�����ݐ�				*/

#define		P_PRM_SESCNT_MAX	4										/* ���Ұ�����ݐ�				*/
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/* �������Ұ�����ݔԍ�(���Ұ��̾���ݔԍ���`)															*/
/*------------------------------------------------------------------------------------------------------*/
#define		S_SYS		1												/* ����							*/
#define		S_PAY		2												/* ���Z�@�ŗL					*/
#define		S_TYP		3												/* ���Z�@���ߕ�					*/
#define		S_TOK		4												/* ���ʓ�						*/
#define		S_TIK		5												/* ���Ԍ�/�����				*/
#define		S_SHA		6												/* �Ԏ�							*/
#define		S_PAS		7												/* �����						*/
#define		S_DIS		8												/* ����							*/
#define		S_SER		9												/* ���޽��						*/
#define		S_STO		10												/* �X����						*/
#define		S_TAT		11												/* ���X��						*/
#define		S_WAR		12												/* ������						*/
#define		S_PRP		13												/* ����߲��/�񐔌�				*/
#define		S_TOT		14												/* �W�v							*/
#define		S_TOP		15												/* �W�v�����					*/
#define		S_BUN		16												/* ���ޏW�v						*/
#define		S_RTP		17												/* �̎���						*/
#define		S_PRN		18												/* �󎚊֘A						*/
#define		S_MON		19												/* ����							*/
#define		S_KAN		20												/* ���K�Ǘ�						*/
#define		S_OTP		21												/* �o�͐M��						*/
#define		S_INP		22												/* ���͐M��						*/
#define		S_DSP		24												/* �ē��\��						*/
#define		S_ANA		25												/* �ē�����						*/
#define		S_REC		26												/* ���L�^						*/
#define		S_PSW		27												/* �߽ܰ��						*/
#define		S_CAL		28												/* �����v�Z��{					*/
#define		S_STM		29												/* ���޽���						*/
#define		S_PTM		30												/* ����J�n/�I��				*/
#define		S_RAT		31												/* �����/������				*/
#define		S_KOU		32												/* ����X�V						*/
#define		S_MIF		33												/* Mifare ����݇�				*/
#define		S_NTN		34												/* NT-NET						*/
#define		S_SCA		35												/* Suica						*/
#define		S_CEN		36												/* �������ް�ڑ����Ұ��ݒ�	*/
#define		S_CRE		38												/* �N���W�b�g�ݒ� */
// GM849100(S) �ʐݒ�
#define		S_SSS		40												// �ʐݒ�
// GM849100(E) �ʐݒ�
#define		S_MDL		41												/* �W���ʐMӼޭ��				*/
#define		S_LTM		46												/* ������ʖ��ݒ� */
// MH810100(S) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
#define		S_PKT		48												// LCD�ʐM�ݒ�
// MH810100(E) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
#define		S_RDL		49												/* ���u�_�E�����[�h				*/
// MH321800(S) G.So IC�N���W�b�g�Ή�
#define		S_ECR		50												/* ���σ��[�_�ݒ� */
// MH321800(E) G.So IC�N���W�b�g�Ή�
#define		S_CLX		56												/* �����v�Z�g�� */
#define		S_CLC		57												/* ��������̌n */
#define		S_CLA		58												/* ��4�`��6�����̌n */
#define		S_CLB		59												/* ��7�`��9�����̌n */
// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//																		/*								*/
//#define		C_PRM_SESCNT_END	S_CLB
#define		S_LCD		60												// LCD���W���[����p�ݒ�

#define		C_PRM_SESCNT_END	S_LCD
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/* �����Ұ�����ݔԍ�(���Ұ��̾���ݔԍ���`)															*/
/*------------------------------------------------------------------------------------------------------*/
#define		S_P01		1												/*								*/
#define		S_P02		2												/*								*/
#define		S_P03		3												/*								*/
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/* �������Ұ������(�e���Ұ�����݂̐ݒ���ڽ�����`����)												*/
/*------------------------------------------------------------------------------------------------------*/
#define		C_PRM_SES01		100											/* ���Ұ������01�ݒ萔			*/
#define		C_PRM_SES02		100											/* ���Ұ������02�ݒ萔			*/
#define		C_PRM_SES03		150											/* ���Ұ������03�ݒ萔			*/
// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//#define		C_PRM_SES04		70											/* ���Ұ������04�ݒ萔			*/
#define		C_PRM_SES04		80											/* ���Ұ������04�ݒ萔			*/
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
#define		C_PRM_SES05		30											/* ���Ұ������05�ݒ萔			*/
#define		C_PRM_SES06		75											/* ���Ұ������06�ݒ萔			*/
#define		C_PRM_SES07		150											/* ���Ұ������07�ݒ萔			*/
#define		C_PRM_SES08		27											/* ���Ұ������08�ݒ萔			*/
#define		C_PRM_SES09		140											/* ���Ұ������09�ݒ萔			*/
#define		C_PRM_SES10		300											/* ���Ұ������10�ݒ萔			*/
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
// #define		C_PRM_SES11		186											/* ���Ұ������11�ݒ萔			*/
#define		C_PRM_SES11		930											/* ���Ұ������11�ݒ萔			*/
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5793 ���X�܊����X��999�Ή�
#define		C_PRM_SES12		301											/* ���Ұ������12�ݒ萔			*/
// MH322914 (s) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)�񐔌������؂�\��
//#define		C_PRM_SES13		15											/* ���Ұ������13�ݒ萔			*/
#define		C_PRM_SES13		24											/* ���Ұ������13�ݒ萔			*/
// MH322914 (e) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)�񐔌������؂�\��
#define		C_PRM_SES14		25											/* ���Ұ������14�ݒ萔			*/
#define		C_PRM_SES15		50											/* ���Ұ������15�ݒ萔			*/
#define		C_PRM_SES16		60											/* ���Ұ������16�ݒ萔			*/
#define		C_PRM_SES17		60											/* ���Ұ������17�ݒ萔			*/
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
//#define		C_PRM_SES18		50											/* ���Ұ������18�ݒ萔			*/
#define		C_PRM_SES18		100											/* ���Ұ������18�ݒ萔			*/
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
#define		C_PRM_SES19		30											/* ���Ұ������19�ݒ萔			*/
#define		C_PRM_SES20		65											/* ���Ұ������20�ݒ萔			*/
#define		C_PRM_SES21		32											/* ���Ұ������21�ݒ萔			*/
#define		C_PRM_SES22		13											/* ���Ұ������22�ݒ萔			*/
#define		C_PRM_SES23		30											/* ���Ұ������23�ݒ萔			*/
#define		C_PRM_SES24		40											/* ���Ұ������24�ݒ萔			*/
// MH810100(S) 2020/06/02 ���Z�@�ē������ݒ�g��
//#define		C_PRM_SES25		200											/* ���Ұ������25�ݒ萔			*/
#define		C_PRM_SES25		400											/* ���Ұ������25�ݒ萔			*/
// MH810100(E) 2020/06/02 ���Z�@�ē������ݒ�g��
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//#define		C_PRM_SES26		5											/* ���Ұ������26�ݒ萔			*/
#define		C_PRM_SES26		20											/* ���Ұ������26�ݒ萔			*/
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810100(S) Y.Yoshida 2020/06/05 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//#define		C_PRM_SES27		20											/* ���Ұ������27�ݒ萔			*/
#define		C_PRM_SES27		100											/* ���Ұ������27�ݒ萔			*/
// MH810100(E) Y.Yoshida 2020/06/05 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
#define		C_PRM_SES28		55											/* ���Ұ������28�ݒ萔			*/
#define		C_PRM_SES29		40											/* ���Ұ������29�ݒ萔			*/
#define		C_PRM_SES30		150											/* ���Ұ������30�ݒ萔			*/
#define		C_PRM_SES31		900											/* ���Ұ������31�ݒ萔			*/
#define		C_PRM_SES32		240											/* ���Ұ������32�ݒ萔			*/
#define		C_PRM_SES33		100											/* ���Ұ������33�ݒ萔			*/
#define		C_PRM_SES34		150											/* ���Ұ������34�ݒ萔			*/
#define		C_PRM_SES35		100											/* ���Ұ������35�ݒ萔			*/
#define		C_PRM_SES36		100											/* ���Ұ������36�ݒ萔			*/
#define		C_PRM_SES37		100											/* ���Ұ������37�ݒ萔			*/
#define		C_PRM_SES38		100											/* ���Ұ������38�ݒ萔			*/
#define		C_PRM_SES39		200											/* ���Ұ������39�ݒ萔			*/
#define		C_PRM_SES40		100											/* ���Ұ������40�ݒ萔			*/
#define		C_PRM_SES41		100											/* ���Ұ������41�ݒ萔			*/
#define		C_PRM_SES42		100											/* ���Ұ������42�ݒ萔			*/
#define		C_PRM_SES43		10											/* ���Ұ������43�ݒ萔			*/
#define		C_PRM_SES44		10											/* ���Ұ������44�ݒ萔			*/
#define		C_PRM_SES45		50											/* ���Ұ������45�ݒ萔			*/
#define		C_PRM_SES46		200											/* ���Ұ������46�ݒ萔			*/
#define		C_PRM_SES47		100											/* ���Ұ������47�ݒ萔			*/
#define		C_PRM_SES48		100											/* ���Ұ������48�ݒ萔			*/
#define		C_PRM_SES49		100											/* ���Ұ������49�ݒ萔			*/
#define		C_PRM_SES50		100											/* ���Ұ������50�ݒ萔			*/
#define		C_PRM_SES51		20											/* ���Ұ������51�ݒ萔			*/
#define		C_PRM_SES52		50											/* ���Ұ������52�ݒ萔			*/
#define		C_PRM_SES53		100											/* ���Ұ������53�ݒ萔			*/
#define		C_PRM_SES54		100											/* ���Ұ������54�ݒ萔			*/
#define		C_PRM_SES55		100											/* ���Ұ������55�ݒ萔			*/
#define		C_PRM_SES56		100											/* ���Ұ������56�ݒ萔			*/
#define		C_PRM_SES57		300											/* ���Ұ������57�ݒ萔			*/
#define		C_PRM_SES58		900											/* ���Ұ������58�ݒ萔			*/
#define		C_PRM_SES59		900											/* ���Ұ������59�ݒ萔			*/
// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//#define		C_PRM_SES60		100											/* ���Ұ������60�ݒ萔			*/
// MH810101(S) R.Endo 2021/02/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2 �o�[�R�[�h�Ή�
//#define		C_PRM_SES60		200											/* ���Ұ������60�ݒ萔			*/
#define		C_PRM_SES60		800											/* ���Ұ������60�ݒ萔			*/
// MH810101(E) R.Endo 2021/02/17 �Ԕԃ`�P�b�g���X �t�F�[�Y2 �o�[�R�[�h�Ή�
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/* �����Ұ������(�e���Ұ�����݂̐ݒ���ڽ�����`����)												*/
/*------------------------------------------------------------------------------------------------------*/
#define		P_PRM_SES01		100											/* ���Ұ������01�ݒ萔			*/
#define		P_PRM_SES02		30											/* ���Ұ������02�ݒ萔			*/
#define		P_PRM_SES03		20											/* ���Ұ������03�ݒ萔			*/
																		/*								*/
																		/*------------------------------*/
																		/*								*/
																		/*------------------------------*/
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/*	��������Ұ�ð��ِ錾																				*/
/*------------------------------------------------------------------------------------------------------*/
extern	const ushort	CPrmCnt[];										/* ���Ұ�����ݐ��i�[�z��		*/
																		/*								*/
typedef	struct {														/*								*/
	long	CParam00[1];												/* �����00�ݒ�i�[�ޯ̧(�ђl)	*/
																		/* �������Ұ��ђl�i�[�ر 		*/
																		/* (ushort[2] �Ƃ��Ďg�p)		*/
																		/* +0,1=�T���l(CParam00�`�����܂ł�ushort���a�Z�l) */
																		/* +2,3=��L�T���l��bit���]�l	*/
	long	CParam01[C_PRM_SES01+1];									/* �����01�ݒ�i�[�ޯ̧			*/
	long	CParam02[C_PRM_SES02+1];									/* �����02�ݒ�i�[�ޯ̧			*/
	long	CParam03[C_PRM_SES03+1];									/* �����03�ݒ�i�[�ޯ̧			*/
	long	CParam04[C_PRM_SES04+1];									/* �����04�ݒ�i�[�ޯ̧			*/
	long	CParam05[C_PRM_SES05+1];									/* �����05�ݒ�i�[�ޯ̧			*/
	long	CParam06[C_PRM_SES06+1];									/* �����06�ݒ�i�[�ޯ̧			*/
	long	CParam07[C_PRM_SES07+1];									/* �����07�ݒ�i�[�ޯ̧			*/
	long	CParam08[C_PRM_SES08+1];									/* �����08�ݒ�i�[�ޯ̧			*/
	long	CParam09[C_PRM_SES09+1];									/* �����09�ݒ�i�[�ޯ̧			*/
	long	CParam10[C_PRM_SES10+1];									/* �����10�ݒ�i�[�ޯ̧			*/
	long	CParam11[C_PRM_SES11+1];									/* �����11�ݒ�i�[�ޯ̧			*/
	long	CParam12[C_PRM_SES12+1];									/* �����12�ݒ�i�[�ޯ̧			*/
	long	CParam13[C_PRM_SES13+1];									/* �����13�ݒ�i�[�ޯ̧			*/
	long	CParam14[C_PRM_SES14+1];									/* �����14�ݒ�i�[�ޯ̧			*/
	long	CParam15[C_PRM_SES15+1];									/* �����15�ݒ�i�[�ޯ̧			*/
	long	CParam16[C_PRM_SES16+1];									/* �����16�ݒ�i�[�ޯ̧			*/
	long	CParam17[C_PRM_SES17+1];									/* �����17�ݒ�i�[�ޯ̧			*/
	long	CParam18[C_PRM_SES18+1];									/* �����18�ݒ�i�[�ޯ̧			*/
	long	CParam19[C_PRM_SES19+1];									/* �����19�ݒ�i�[�ޯ̧			*/
	long	CParam20[C_PRM_SES20+1];									/* �����20�ݒ�i�[�ޯ̧			*/
	long	CParam21[C_PRM_SES21+1];									/* �����21�ݒ�i�[�ޯ̧			*/
	long	CParam22[C_PRM_SES22+1];									/* �����22�ݒ�i�[�ޯ̧			*/
	long	CParam23[C_PRM_SES23+1];									/* �����23�ݒ�i�[�ޯ̧			*/
	long	CParam24[C_PRM_SES24+1];									/* �����24�ݒ�i�[�ޯ̧			*/
	long	CParam25[C_PRM_SES25+1];									/* �����25�ݒ�i�[�ޯ̧			*/
	long	CParam26[C_PRM_SES26+1];									/* �����26�ݒ�i�[�ޯ̧			*/
	long	CParam27[C_PRM_SES27+1];									/* �����27�ݒ�i�[�ޯ̧			*/
	long	CParam28[C_PRM_SES28+1];									/* �����28�ݒ�i�[�ޯ̧			*/
	long	CParam29[C_PRM_SES29+1];									/* �����29�ݒ�i�[�ޯ̧			*/
	long	CParam30[C_PRM_SES30+1];									/* �����30�ݒ�i�[�ޯ̧			*/
	long	CParam31[C_PRM_SES31+1];									/* �����31�ݒ�i�[�ޯ̧			*/
	long	CParam32[C_PRM_SES32+1];									/* �����32�ݒ�i�[�ޯ̧			*/
	long	CParam33[C_PRM_SES33+1];									/* �����33�ݒ�i�[�ޯ̧			*/
	long	CParam34[C_PRM_SES34+1];									/* �����34�ݒ�i�[�ޯ̧			*/
	long	CParam35[C_PRM_SES35+1];									/* �����35�ݒ�i�[�ޯ̧			*/
	long	CParam36[C_PRM_SES36+1];									/* �����36�ݒ�i�[�ޯ̧			*/
	long	CParam37[C_PRM_SES37+1];									/* �����37�ݒ�i�[�ޯ̧			*/
	long	CParam38[C_PRM_SES38+1];									/* �����38�ݒ�i�[�ޯ̧			*/
	long	CParam39[C_PRM_SES39+1];									/* �����39�ݒ�i�[�ޯ̧			*/
	long	CParam40[C_PRM_SES40+1];									/* �����40�ݒ�i�[�ޯ̧			*/
	long	CParam41[C_PRM_SES41+1];									/* �����41�ݒ�i�[�ޯ̧			*/
	long	CParam42[C_PRM_SES42+1];									/* �����42�ݒ�i�[�ޯ̧			*/
	long	CParam43[C_PRM_SES43+1];									/* �����43�ݒ�i�[�ޯ̧			*/
	long	CParam44[C_PRM_SES44+1];									/* �����44�ݒ�i�[�ޯ̧			*/
	long	CParam45[C_PRM_SES45+1];									/* �����45�ݒ�i�[�ޯ̧			*/
	long	CParam46[C_PRM_SES46+1];									/* �����46�ݒ�i�[�ޯ̧			*/
	long	CParam47[C_PRM_SES47+1];									/* �����47�ݒ�i�[�ޯ̧			*/
	long	CParam48[C_PRM_SES48+1];									/* �����48�ݒ�i�[�ޯ̧			*/
	long	CParam49[C_PRM_SES49+1];									/* �����49�ݒ�i�[�ޯ̧			*/
	long	CParam50[C_PRM_SES50+1];									/* �����50�ݒ�i�[�ޯ̧			*/
	long	CParam51[C_PRM_SES51+1];									/* �����51�ݒ�i�[�ޯ̧			*/
	long	CParam52[C_PRM_SES52+1];									/* �����52�ݒ�i�[�ޯ̧			*/
	long	CParam53[C_PRM_SES53+1];									/* �����53�ݒ�i�[�ޯ̧			*/
	long	CParam54[C_PRM_SES54+1];									/* �����54�ݒ�i�[�ޯ̧			*/
	long	CParam55[C_PRM_SES55+1];									/* �����55�ݒ�i�[�ޯ̧			*/
	long	CParam56[C_PRM_SES56+1];									/* �����56�ݒ�i�[�ޯ̧			*/
	long	CParam57[C_PRM_SES57+1];									/* �����57�ݒ�i�[�ޯ̧			*/
	long	CParam58[C_PRM_SES58+1];									/* �����58�ݒ�i�[�ޯ̧			*/
	long	CParam59[C_PRM_SES59+1];									/* �����59�ݒ�i�[�ޯ̧			*/
	long	CParam60[C_PRM_SES60+1];									/* �����60�ݒ�i�[�ޯ̧			*/
} t_CParam;																/*								*/
extern	t_CParam	CParam;												/* ���ʃp�����[�^�G���A			*/
																		/* 								*/
																		/* 								*/
																		/* 								*/
extern	long	*CPrmSS[];												/* ���Ұ������ð���				*/
																		/* 								*/
extern	const short	PPrmCnt[];											/* ���Ұ�����ݐ��i�[�z��		*/
																		/* 								*/
typedef	struct {														/*								*/
	long	PParam00[1];												/* �����00�ݒ�i�[�ޯ̧(��а)	*/
	long	PParam01[P_PRM_SES01+1];									/* �����01�ݒ�i�[�ޯ̧			*/
	long	PParam02[P_PRM_SES02+1];									/* �����02�ݒ�i�[�ޯ̧			*/
	long	PParam03[P_PRM_SES03+1];									/* �����03�ݒ�i�[�ޯ̧			*/
} t_PParam;																/*								*/
extern	t_PParam	PParam;												/* �ʃp�����[�^�G���A			*/
																		/* 								*/
extern	long		*PPrmSS[];											/* ���Ұ������ð���				*/
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/*	���Ұ���̫��																						*/
/*------------------------------------------------------------------------------------------------------*/
																		/*								*/
struct	PRM_REC{														/* ���Ұ���̫�Ēl�p�\����		*/
	uchar	ses;														/*								*/
	short	adr;														/*								*/
	long	dat;														/*								*/
};																		/*								*/
																		/*								*/
extern	const struct	PRM_REC	cprm_rec[];								/* �������Ұ���̫��				*/
extern	const struct	PRM_REC	pprm_rec[];								/* �ŗL���Ұ���̫��				*/
																		/*								*/
/*------------------------------------------------------------------------------------------------------*/
/*	��������Ұ�ð��َQ�Ɗ֐��錾																		*/
/*------------------------------------------------------------------------------------------------------*/
extern	long	prm_get( char, short, short, char, char );				/* ���Ұ��Q��					*/
extern	long	prm_tim( char, short, short );							/* ���Ԏ擾						*/
extern	void	prm_init( char );										/* ���Ұ��i�[���ڽ�擾			*/
extern	void	prm_clr( char, char, char );							/* ����/�ŗL���Ұ��ر/��̫��	*/
extern	void	lockinfo_clr( char );									/* �Ԏ����Ұ�����̫��			*/
extern	void	lockmaker_clr( char );									/* ۯ����u���Ұ�����̫��		*/
extern	ulong	prm_svtim( ushort, uchar );								/* ���޽��ю擾					*/
extern	long	default_prm_get( uchar ses, ushort addr, ushort *pos);	/* ��̫�����Ұ��擾				*/
extern	void	prm_invalid_change( short, short, long* );				/* �Œ����Ұ��ϊ�				*/
extern	uchar	prm_invalid_check( void );								/* �Œ����Ұ��ϊ�����			*/
extern	void	prm_oiban_chk( void );									/* �ǔԃJ�E���^�`�F�b�N */
																		/*								*/
																		/*------------------------------*/
extern	long	nomask_next( long add );
extern	long	nomask_prev( long add );
extern	long	nomask_all_next( long add );
extern	long	nomask_all_prev( long add );
extern	long	nomask_ses_next( long add );
extern	long	nomask_ses_prev( long add );
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
extern	long	is_nomask( long current_add );
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�ݒ��\��)
#endif	// _PRM_TBL_H_
