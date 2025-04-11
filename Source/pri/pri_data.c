/*[]---------------------------------------------------------------------------[]*/
/*|		���������䕔�F�����ް��ر												|*/
/*|																				|*/
/*|	̧�ٖ���	:	Pri_Data.c													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author		: K.Motohashi													|*/
/*|	Date		: 2005-07-19													|*/
/*|	UpDate		:																|*/
/*|																				|*/
/*[]--------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"Pri_def.h"



/*------------------------------------------*/
/*	�񏉊����ر								*/
/*------------------------------------------*/


/*------------------------------------------*/
/*	�������ر								*/
/*------------------------------------------*/

PRN_DATA_BUFF	rct_prn_buff;			// ڼ��(SCI0)	����M�ް��Ǘ��ޯ̧
PRN_DATA_BUFF	jnl_prn_buff;			// �ެ���(SCI3)	����M�ް��Ǘ��ޯ̧
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
uchar			eja_work_buff[PRN_EJA_BUF_SIZE];	// �d�q�W���[�i���p���[�N�o�b�t�@
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

PRN_PROC_DATA	rct_proc_data;			// ڼ��		�󎚏��������ް�
PRN_PROC_DATA	jnl_proc_data;			// �ެ���	�󎚏��������ް�

uchar			rct_init_sts;			// ڼ��		���������(�O�F�����������҂��^�P�F�����������^�Q�F���������s�^�R�F���ڑ�)
uchar			jnl_init_sts;			// �ެ���	���������(�O�F�����������҂��^�P�F�����������^�Q�F���������s�^�R�F���ڑ�)
uchar			rct_init_rty;			// ڼ��		����������ޑ��M��ײ�����
uchar			jnl_init_rty;			// �ެ���	����������ޑ��M��ײ�����

uchar			GyouCnt_r;				// ڼ��		���M�ς݈��ް��s��
uchar			GyouCnt_j;				// �ެ���	���M�ς݈��ް��s��

// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
uchar			GyouCnt_All_r;			// ڼ��		���M�ς݈��ް��S�s��
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
uchar			GyouCnt_All_j;			// �ެ���	���M�ς݈��ް��S�s��
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

MSG 			NextMsg_r;				// ڼ��		�󎚗v��ү���ޕۑ��ޯ̧
MSG 			NextMsg_j;				// �ެ���	�󎚗v��ү���ޕۑ��ޯ̧

uchar			PriDummyMsg;			// ��аү���ޓo�^��ԁiON�F�o�^����^OFF�F�o�^�Ȃ��j
MsgBuf			*TgtMsg;				// �����Ώ�ү�����ޯ̧�߲��

uchar			terget_pri;				// �����Ώ���������
uchar			next_prn_msg;			// �󎚏������̎��ҏW�v��ү�����׸�	�iON�F���ҏW�v���^OFF�F�V�K�ҏW�v���j
uchar			syuukei_kind;			// �󎚏������̏W�v���

uchar			rct_goukei_pri;			// ���v�L�^�󎚏�ԁiڼ�ėp�j	��ON�F�󎚏�����
uchar			jnl_goukei_pri;			// ���v�L�^�󎚏�ԁi�ެ��ٗp�j	��ON�F�󎚏�����
uchar			rct_atsyuk_pri;			// �����W�v�󎚏�ԁiڼ�ėp�j	��ON�F�󎚏�����
uchar			jnl_atsyuk_pri;			// �����W�v�󎚏�ԁi�ެ��ٗp�j	��ON�F�󎚏�����
uchar			turikan_pri_status;		// �ޑK�Ǘ�����Ă��󎚏I����Ă���

uchar			prn_zero;				// �W�v�ް��O���󎚗L���i�ݒ��ް��j
date_time_rec	Repri_Time;				// ������ē����i�O��W�v�󎚗p�j
uchar			Repri_kakari_no;		// ������ČW��No.

uchar			header_hight;			// ͯ�ް���ް��̏c���ށi�ޯĐ��j
uchar			gyoukan_data;			// ���s���i�ޯĐ��j�w��i���O:�ݒ��ް��ɏ]���^���O:�{�w���ް��ɏ]���j

char			prn_work[64];			// �󎚏���ܰ�
char			cTmp1[256];				// �󎚏���ܰ�
char			cTmp2[64];				// �󎚏���ܰ�
char			cMnydata[64];			// �󎚏���ܰ�
char			cEditwk1[64];			// ���ް��ҏWܰ��P
char			cEditwk2[64];			// ���ް��ҏWܰ��Q
char			cEditwk3[64];			// ���ް��ҏWܰ��R

char			cErrlog[160];			// �װ۸ޗp�i�װ���o�^�ޯ̧�j

ulong			Header_Rsts;			// ͯ�ް���ް��Ǎ����
ulong			Footer_Rsts;			// ̯�����ް��Ǎ����
char			Header_Data[4][36];		// ͯ�ް���ް�
char			Footer_Data[4][36];		// ̯�����ް�
ulong			AcceptFooter_Rsts;			// ��t��̯�����ް��Ǎ����
char			AcceptFooter_Data[4][36];	// ��t��̯�����ް�

char			Logo_Fdata[24][56];		// ۺވ��ް��Ǎ�ܰ�
char			Logo_Fla_data[24];		// ۺވ��ް��ϊ�ܰ�1�i�ׯ����؂��ް��j
char			Logo_Reg_data[24];		// ۺވ��ް��ϊ�ܰ�2�i�ׯ����؂�ϊ������ް��j
uchar			Logo_Reg_sts_rct;		// ۺޓo�^������ԁiڼ�āj
uchar			Logo_Reg_sts_jnl;		// ۺޓo�^������ԁi�ެ��فj

t_Pri_Work		Pri_Work[2];			// �v�����^�^�X�N���[�N�G���A
										// ���ZLOG�E�W�vLOG�󎚂Ɏg�p��
										//   [0]=���V�[�g�v�����^�p�A[1]=�W���[�i���v�����^�p

ulong			Syomei_Rsts;			// �x�����ށE���������ް��Ǎ����
ulong			Kamei_Rsts;				// �����X�����ް��Ǎ����
char			Syomei_Data[4][36];		// �x�����ށE���������ް�
char			Kamei_Data[4][36];		// �����X�����ް�

uchar			JR_Print_Wait_tim[2]={0xff, 0xff};
										// �ȉ��ƯĂ̓����n�߂���̌o�ߎ���(�P��:20ms)
										// [0] = ڼ�������
										// [1] = �ެ��������

uchar			f_Prn_R_SendTopChar;	// 1=ڼ��������󎚗v���̐擪�s�������M�O�i�����M�j
										// 0=�擪1��׸����M�v����
uchar			f_Prn_J_SendTopChar;	// 1=�ެ���������󎚗v���̐擪�s�������M�O�i�����M�j
										// 0=�擪1��׸����M�v����
uchar 			Cal_Parameter_Flg;		// �ݒ�l�ُ�t���O 0:����/1:�ُ�

ulong			TCardFtr_Rsts;			// �s�J�[�h̯�����ް��Ǎ����
char			TCardFtr_Data[T_FOOTER_GYO_MAX][36];	// �s�J�[�h̯�����ް�
ulong			AzuFtr_Rsts;			// �s�J�[�h̯�����ް��Ǎ����
char			AzuFtr_Data[AZU_FTR_GYO_MAX][36];	// �s�J�[�h̯�����ް�
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
ulong			CreKbrFtr_Rsts;			// �ߕ���̯�����ް��Ǎ����
char			CreKbrFtr_Data[KBR_FTR_GYO_MAX][36];	// �ߕ���̯�����ް�
ulong			EpayKbrFtr_Rsts;			// �ߕ���̯�����ް��Ǎ����
char			EpayKbrFtr_Data[KBR_FTR_GYO_MAX][36];	// �ߕ���̯�����ް�
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
ulong			FutureFtr_Rsts;				// ����x���z̯�����ް��Ǎ����
char			FutureFtr_Data[FUTURE_FTR_GYO_MAX][36];	// ����x���z̯�����ް�
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
ulong			EmgFooter_Rsts;			// ��Q�A���[̯�����ް��Ǎ����
char			EmgFooter_Data[EMGFOOT_GYO_MAX][36];	// ��Q�A���[̯�����ް�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
ushort			Pri_Tasck[TSKMAX][2];		// �^�X�N�g�p�ʁA��
ulong			Pri_Pay_Syu[2][2];			// ���Z�A�W�v�f�[�^�T�C�Y
uchar			Pri_program;				// �f�o�b�ONo
uchar			f_partial_cut;				// �p�[�V�����J�b�g����t���O(�p�[�V�����J�b�g�����s���ꂽ���Ƃ������t���O 0:�����s 1:���s)

uchar			rct_timer_end;			// ���V�[�g�v�����^�󎚊�����^�C�}�[
uchar			PriBothPrint;			// �v�����^�{�W���[�i���󎚃t���O
uchar			BothPrnStatus;			// �v�����^�{�W���[�i���󎚂̃G���[�X�e�[�^�X
ushort			BothPrintCommand;		// �v�����^�{�W���[�i���󎚂̃G���[�R�}���h
// MH322914 (s) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
uchar			ryo_inji;				// ���V�[�g�v�����^�̎��؈󎚏�� 0=�󎚒��ł͂Ȃ�/1=�󎚒�
// MH322914 (e) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
EC_SETTLEMENT_RES	Deemedalmrct;
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
PRN_RCV_DATA_BUFF_R	pri_rcv_buff_r;
uchar			ryo_stock;				// ���V�[�g�f�[�^�X�g�b�N�t���O 0:�X�g�b�N�Ȃ� 1:�X�g�b�N���� 2:�X�g�b�N�j�� 0xff:�X�g�b�N�󎚊���
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
uchar			RegistNum[14];			// �C���{�C�X�̓o�^�ԍ��iT+13���̐����j
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j

