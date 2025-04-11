/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�f�[�^�e�[�u����`�t�@�C��																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      : S.Takahashi																				   |*/
/*| Date        : 2012-09-10																				   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/

#include	"ksgmac.h"													/* MAC ���ڋ@�\ͯ��			*/


/****************************************************************/
/*		�q�`�t�ݒ�\����										*/
/****************************************************************/
struct KSG_RAUCONF {
	unsigned char	speed;					// �ʐM���x
	unsigned char	data_len;				// �f�[�^��
	unsigned char	stop_bit;				// �X�g�b�v�r�b�g
	unsigned char	parity_bit;				// �p���e�B�r�b�g
	unsigned char	deta_full_proc;			// �f�[�^�������@
	unsigned short	retry;					// ���g���C��
	unsigned short	time_out;				// �^�C���A�E�g
	unsigned char	modem_retry;			// ���f���ڑ����g���C
	unsigned char	modem_exist;			// ���f���̗L�� 0:����(FOMA) 1:�Ȃ�(LAN)
	unsigned char	foma_dopa;				// 20H/'0' = DOPA   / '1' = FOMA, FOMA�̏ꍇ init_data �� APN�w�� ex>' 1sf.amano.co.jp'
	unsigned char	Dpa_proc_knd;			// Dopa �����敪 [0]=�펞�ʐM�Ȃ�, [1]=�펞�ʐM����
	IPv4			Dpa_IP_h;				// Dopa HOST��IP�A�h���X
	unsigned int	Dpa_port_h;				// Dopa HOST�ǃ|�[�g�ԍ�
	IPv4			Dpa_IP_m;				// Dopa ����IP�A�h���X
	unsigned int	Dpa_port_m;				// Dopa ���ǃ|�[�g�ԍ�
	unsigned char	Dpa_ppp_ninsho;			// Dopa ���M���F�؎菇 0:MS-CHAP 1:PAP 2:CHAP 3:�F�؂Ȃ�
	unsigned short	Dpa_nosnd_tm;			// Dopa ���ʐM�^�C�}�[(�b)
	unsigned short	Dpa_cnct_rty_tm;		// Dopa �Ĕ��đ҂��^�C�}�[(�b)
	unsigned short	Dpa_dial_wait_tm;		// Dopa �_�C�A�������҂�����(�b)
	unsigned short	Dpa_com_wait_tm;		// Dopa �R�}���h(ACK)�҂��^�C�}�[(�b)
	unsigned short	Dpa_data_rty_tm;		// Dopa �f�[�^�đ��҂��^�C�}�[(��)
	unsigned short	Dpa_discnct_tm;			// Dopa TCP�R�l�N�V�����ؒf�҂��^�C�}�[(�b)
	unsigned short	Dpa_cnct_rty_cn;		// Dopa �Ĕ��ĉ�
	unsigned char	Dpa_data_rty_cn;		// Dopa �f�[�^�đ���(NAK,������)
	unsigned char	Dpa_data_code;			// Dopa HOST���d���R�[�h [0]=�����R�[�h, [1]=�o�C�i���R�[�h
	unsigned char	Dpa_data_snd_rty_cnt;	// �c���o���������p�P�b�g���M���g���C��(������)
	unsigned short	Dpa_ack_wait_tm;		// �c���o���������p�P�b�g�����҂�����(�`�b�j/�m�`�j)
	unsigned char	Dpa_data_rcv_rty_cnt;	// �c���o���������p�P�b�g��M���g���C��(�m�`�j)
	unsigned short	Dpa_port_watchdog_tm;	// �c���o���������ʐM�Ď��^�C�}(�b)
	unsigned short	Dpa_port_m2;			// �c���o�������� ���ǃ|�[�g�ԍ�
	IPv4			netmask;				// �T�u�l�b�g�}�X�N
	unsigned short	APNLen;					// �`�o�m������
	unsigned char	APNName[32];			// �`�o�m
	IPv4			Dpa_IP_Cre_h;			// �Z���^�[�N���W�b�g HOST��IP�A�h���X
	unsigned int	Dpa_port_Cre_h;			// �Z���^�[�N���W�b�g HOST�ǃ|�[�g�ԍ�
};

extern	struct	KSG_RAUCONF	KSG_RauConf;
