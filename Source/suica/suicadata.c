/*[]----------------------------------------------------------------------[]*/
/*| suica�ް���`                                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Namioka                                                |*/
/*| Date        : 2006-07-07                                               |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
// MH321800(S) G.So IC�N���W�b�g�Ή�
#include	<string.h>
// MH321800(E) G.So IC�N���W�b�g�Ή�
#include	"suica_def.h"
#include	"system.h"


SUICA_SETTLEMENT_RES Settlement_Res;

SUICA_REC	Suica_Rec;
SUICA_SEND_BUFF	Suica_Snd_Buf;													

unsigned long	pay_dsp;							// �\���˗��f�[�^
unsigned char	err_data;							// �ُ�f�[�^
t_STATUS_DATA_INFO	STATUS_DATA;					// ��ԃf�[�^
struct	clk_rec	time_data;							// ���ԓ����f�[�^
unsigned char	fix_data[2];						// �Œ�f�[�^
unsigned char	suica_errst;						/* Communication  Status */
unsigned char	suica_errst_bak;					/* Communication  Status */
unsigned char 	suica_err[5];
unsigned char 	Retrycount;							/* Retry���Đ� */
unsigned char	dsp_work_buf[2];					// ��ʕ\���p�ꎞ�ޔ�̈�
unsigned char	Status_Retry_Count;					// ��t�s���̐����ް����M����
unsigned char	Status_Retry_Count_NG;					// ��t�s���̐����ް����M����
unsigned char	Status_Retry_Count_OK;					// ��t���̐����ް����M����
unsigned char	Ope_Suica_Status;					// Ope������̑��M�ð��
unsigned char	disp_media_flg;						// ��ʕ\���p�@���Z�}�̎g�p��(0)��(1)
unsigned char	DspWorkerea[7];						// ���Ԍ��}���҂���ʁ@���p�}�̕\��ү����No.
unsigned short	DspChangeTime[2];					// ��ʕ\���p����ϰ�l��ė̈�
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//unsigned char	dsp_change;							// ��ʐؑ֗p�׸�
unsigned char	dsp_change;							// �����������̉�ʕ\���׸� 0=�����֌W�\���Ȃ� 1=�����֌W�\���� 2=�����֌W�\����(����f�[�^�����c���ʒm�I�����M��)
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
unsigned char	dsp_fusoku;							// �c���s���װѕ\�������׸�
long			w_settlement;						// Suica�c���s�����̐��Z�z�i�O���͎c���s���łȂ����Ӗ�����j
unsigned char	suica_fusiku_flg;
unsigned char	mode_Lagtim10;						// Lagtimer10�̎g�p��� 0:���p�\�}�̻��د��\�� 1:Suica��~��̎�t���đ��MWait�I��
unsigned short	nyukin_delay[DELAY_MAX];			// ������������ү���ނ�ێ� [0]:���ү�	[1]:����
unsigned char	delay_count;						// ������������ү���޶��Đ�
unsigned short	wk_media_Type;						// ���Z�}�́i�d�q�}�l�[�j���
unsigned char	time_out;							// ��ʖ߂���ѱ�Ĕ�������̈�
// MH321800(S) G.So IC�N���W�b�g�Ή�
//long			Product_Select_Data;				// ���M�������i�I���f�[�^(���ώ��̔�r�p)
// MH321800(E) G.So IC�N���W�b�g�Ή�
unsigned char suica_work_buf[S_BUF_MAXSIZE];
struct	clk_rec	suica_work_time;
t_STATUS_DATA_INFO	STATUS_DATA_WAIT;					// ��ԃf�[�^(��M�҂��p)

/* Suica ����� */
const unsigned char	suica_tbl[][2] = {
	{ 0xE0, 0x1F },									// �X�^���o�C�v��
	{ 0xE1, 0x1E },									// �ꊇ�v��
	{ 0xE2, 0x1D },									// ���͗v��
	{ 0xE3, 0x1C },									// �o�͎w��
	{ 0xE4, 0x1B },									// ���͗v���̍đ��A�ꊇ�v���̍đ�
// MH321800(S) G.So IC�N���W�b�g�Ή�
//	{ 0xE5, 0x1A }};
	{ 0xE5, 0x1A },									// �o�͎w�߂̍đ�
	{ 0xE2, 0x1D },									// ���͗v����̓��͗v���đ�
};
// MH321800(E) G.So IC�N���W�b�g�Ή�

// MH321800(S) G.So IC�N���W�b�g�Ή�
void	*const	jvma_init_tbl[] = {
//	0				1				2				3				4
	NULL,			NULL,			NULL,			NULL,			ec_init,
//	5				6				7				8				9
	NULL,			NULL,			NULL,			NULL,			NULL
};

#pragma	section	_UNINIT1							/* "B":Uninitialized data area in external RAM1 */
long			Product_Select_Data;				// ���M�����I�����i�f�[�^(���ώ��̔�r�p)
char			EcEdyTerminalNo[20];				// Edy��ʒ[��ID
t_JVMA_SETUP	jvma_setup;							// ���d���ȂǂɈȑO�̐ݒ���Q�Ƃ����邽�߁A�N���������������Ȃ�
// MH321800(E) G.So IC�N���W�b�g�Ή�
