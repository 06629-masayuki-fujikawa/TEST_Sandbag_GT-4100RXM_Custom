/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		Rau�ʐM		 																						   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	ksgRauTable.c																			   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �ERau�ʐM �f�[�^�e�[�u������																			   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2011 AMANO Corp.----------[]*/
																		/*							*/
#include	<machine.h>													/*							*/
#include	<string.h>													/*							*/
#include	"system.h"	
#include	"prm_tbl.h"	
#include	"remote_dl.h"
#include	"ksgRauTable.h"												/* �ް�ð��ٍ\����			*/
#include	"ksg_def.h"
#include	"ksgRauModem.h"
#include	"ksgRauModemData.h"
#include	"raudef.h"
																		/*							*/
struct		KSG_RAUCONF		KSG_RauConf;
//struct		KSG_DPACONF		KSG_DpaConf;

/*[]------------------------------------------------------------------------------------------[]*/
/*|	�ް�ð��ُ�����																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : KSG_RauInitialTbl()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauInitialTbl( void )
{																		/*							*/
	memset( &KSG_RauConf, 0x00, sizeof(KSG_RauConf));					/* ���ʐݒ�̏����l			*/
//	memset( &KSG_DpaConf, 0x00, sizeof(KSG_DpaConf));					/* ���ʐݒ�̏����l			*/

	// DOPA/FOMA�ؑ�
	KSG_uc_FomaFlag = 1;												/* FOMA�Œ�					*/
	KSG_uc_AdapterType = 0;			// UM02-KO
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
//	if (prm_get(COM_PRM, S_CEN, 52, 1, 2) != 0) {
//		KSG_uc_AdapterType = 1;		// UM03-KO
//	}
	if (prm_get(COM_PRM, S_CEN, 52, 1, 2) == 1) {
		KSG_uc_AdapterType = 1;		// UM03-KO
	}else if (prm_get(COM_PRM, S_CEN, 52, 1, 2) == 2) {
		KSG_uc_AdapterType = 2;		// AD-04S(UM04-KO)
	}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	Rau�ݒ�̔��f																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauSetRauConf()														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauSetRauConf(void)
{																		/*							*/
	unsigned char	i;													/*							*/

	// �ʐM���x
	KSG_RauConf.speed = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 4);

	// �f�[�^��
// GG129000(S) W.Hoshino 2023/03/23 #6973 rau�̏����ݒ莞�A�Q�Ɛ�̋��ʃp�����[�^�A�h���X���ԈႦ�Ă���[���ʉ��P���� No 1549]
//	KSG_RauConf.data_len = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 4);
	KSG_RauConf.data_len = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 3);
// GG129000(E) W.Hoshino 2023/03/23 #6973 rau�̏����ݒ莞�A�Q�Ɛ�̋��ʃp�����[�^�A�h���X���ԈႦ�Ă���[���ʉ��P���� No 1549]

	// �X�g�b�v�r�b�g
	KSG_RauConf.stop_bit = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 2);

	// �p���e�B�r�b�g
	KSG_RauConf.parity_bit = (unsigned char)prm_get(COM_PRM, S_CEN, 74, 1, 1);

	// �f�[�^�������@
	KSG_RauConf.deta_full_proc = (unsigned char)prm_get(COM_PRM, S_CEN, 51, 1, 1);

	// ���g���C��(0�Œ�)
	KSG_RauConf.retry = 0;

	// �^�C���A�E�g(�P��100ms)(1�Œ�)
	KSG_RauConf.time_out = 1;

	// ���f���ڑ����g���C(��)(0�Œ�)
	KSG_RauConf.modem_retry = 0;

	// ���f���̗L�� 0:����(FOMA) 1:�Ȃ�(LAN)
	KSG_RauConf.modem_exist = (unsigned char)prm_get(COM_PRM, S_CEN, 51, 1, 3);
	if(prm_get(COM_PRM, S_PAY, 24, 1, 1) != 2){							// Rau�g�p�����擾
		if(RAU_Credit_Enabale == 2){
			KSG_RauConf.modem_exist = 0;
		}
		else{
			KSG_RauConf.modem_exist = 1;
		}
	}
	// Dopa/FOMA�؊���
	KSG_RauConf.foma_dopa = (unsigned char)CPrmSS[S_CEN][78];

	// Dopa �����敪 [0]=�펞�ʐM�Ȃ�, [1]=�펞�ʐM����
	KSG_RauConf.Dpa_proc_knd = (unsigned char)prm_get(COM_PRM, S_CEN, 51, 1, 2);

	// Dopa HOST��IP�A�h���X
	KSG_RauConf.Dpa_IP_h.SEG[0] = (unsigned char)prm_get( COM_PRM, S_CEN, 62, 3, 4 );
	KSG_RauConf.Dpa_IP_h.SEG[1] = (unsigned char)prm_get( COM_PRM, S_CEN, 62, 3, 1 );
	KSG_RauConf.Dpa_IP_h.SEG[2] = (unsigned char)prm_get( COM_PRM, S_CEN, 63, 3, 4 );
	KSG_RauConf.Dpa_IP_h.SEG[3] = (unsigned char)prm_get( COM_PRM, S_CEN, 63, 3, 1 );

	// Dopa HOST�ǃ|�[�g�ԍ�
	KSG_RauConf.Dpa_port_h = CPrmSS[S_CEN][64];

	// Dopa ����IP�A�h���X
	KSG_RauConf.Dpa_IP_m.SEG[0] = (unsigned char)prm_get( COM_PRM, S_CEN, 65, 3, 4 );
	KSG_RauConf.Dpa_IP_m.SEG[1] = (unsigned char)prm_get( COM_PRM, S_CEN, 65, 3, 1 );
	KSG_RauConf.Dpa_IP_m.SEG[2] = (unsigned char)prm_get( COM_PRM, S_CEN, 66, 3, 4 );
	KSG_RauConf.Dpa_IP_m.SEG[3] = (unsigned char)prm_get( COM_PRM, S_CEN, 66, 3, 1 );

	// Dopa ���ǃ|�[�g�ԍ�
	KSG_RauConf.Dpa_port_m = CPrmSS[S_CEN][67];

	// Dopa ���M���F�؎菇 0:MS-CHAP 1:PAP 2:CHAP 3:�F�؂Ȃ�
	KSG_RauConf.Dpa_ppp_ninsho = (unsigned char)prm_get(COM_PRM, S_CEN, 52, 1, 1);

	// Dopa ���ʐM�^�C�}�[(�b)
	KSG_RauConf.Dpa_nosnd_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 54, 4, 1);

	// Dopa �Ĕ��đ҂��^�C�}�[(�b)
	KSG_RauConf.Dpa_cnct_rty_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 53, 3, 4);

	// Dopa �_�C�A�������҂�����(�b)
// TODO:�Z�N�V�����R�U�Ƀ_�C�A�������҂����Ԃ��Ȃ��̂ŗv�m�F
// �Ƃ肠�����b��Ƃ���10�b���Z�b�g
	KSG_RauConf.Dpa_dial_wait_tm = 10;

	// Dopa �R�}���h(ACK)�҂��^�C�}�[(�b)
	KSG_RauConf.Dpa_com_wait_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 68, 3, 1);

	// Dopa �f�[�^�đ��҂��^�C�}�[(��)
	KSG_RauConf.Dpa_data_rty_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 75, 3, 1);

	// Dopa TCP�R�l�N�V�����ؒf�҂��^�C�}�[(�b)
	KSG_RauConf.Dpa_discnct_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 69, 4, 1);

	// Dopa �Ĕ��ĉ�
	KSG_RauConf.Dpa_cnct_rty_cn = (unsigned short)prm_get(COM_PRM, S_CEN, 53, 3, 1);

	// Dopa �f�[�^�đ���(NAK,������)
	KSG_RauConf.Dpa_data_rty_cn = (unsigned char)prm_get(COM_PRM, S_CEN, 70, 1, 2);

	// Dopa HOST���d���R�[�h [0]=�����R�[�h, [1]=�o�C�i���R�[�h(0�Œ�)
	KSG_RauConf.Dpa_data_code = 0;

	// �c���o���������p�P�b�g���M���g���C��(������)
	KSG_RauConf.Dpa_data_snd_rty_cnt = (unsigned char)prm_get(COM_PRM, S_CEN, 73, 1, 2);

	// �c���o���������p�P�b�g�����҂�����(�`�b�j/�m�`�j)
	KSG_RauConf.Dpa_ack_wait_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 72, 3, 1);

	// �c���o���������p�P�b�g��M���g���C��(�m�`�j)
	KSG_RauConf.Dpa_data_rcv_rty_cnt = (unsigned char)prm_get(COM_PRM, S_CEN, 73, 1, 1);

	// �c���o���������ʐM�Ď��^�C�}(�b)
	KSG_RauConf.Dpa_port_watchdog_tm = (unsigned short)prm_get(COM_PRM, S_CEN, 72, 3, 4);

	// �c���o�������� ���ǃ|�[�g�ԍ�
	KSG_RauConf.Dpa_port_m2 = CPrmSS[S_CEN][71];

	// �T�u�l�b�g�}�X�N
	KSG_RauConf.netmask.SEG[0] = (unsigned char)prm_get( COM_PRM, S_MDL, 4, 3, 4 );
	KSG_RauConf.netmask.SEG[1] = (unsigned char)prm_get( COM_PRM, S_MDL, 4, 3, 1 );
	KSG_RauConf.netmask.SEG[2] = (unsigned char)prm_get( COM_PRM, S_MDL, 5, 3, 4 );
	KSG_RauConf.netmask.SEG[3] = (unsigned char)prm_get( COM_PRM, S_MDL, 5, 3, 1 );

	// LAN Port�� APN������ //
	KSG_RauConf.APNLen = (unsigned char)prm_get(COM_PRM, S_CEN, 79, 3, 1);

	// Access Point Name
	for(i = 0; i < 32; ++i) {
		KSG_RauConf.APNName[i] = (uchar)prm_get(COM_PRM, S_CEN, (short)(80+(i / 2)), 3, (char)((i % 2 == 0) ? 4:1));
	}

	// �Z���^�[�N���W�b�g HOST��IP�A�h���X
	KSG_RauConf.Dpa_IP_Cre_h.SEG[0] = (unsigned char)prm_get( COM_PRM, S_CRE, 21, 3, 4 );
	KSG_RauConf.Dpa_IP_Cre_h.SEG[1] = (unsigned char)prm_get( COM_PRM, S_CRE, 21, 3, 1 );
	KSG_RauConf.Dpa_IP_Cre_h.SEG[2] = (unsigned char)prm_get( COM_PRM, S_CRE, 22, 3, 4 );
	KSG_RauConf.Dpa_IP_Cre_h.SEG[3] = (unsigned char)prm_get( COM_PRM, S_CRE, 22, 3, 1 );

	// �Z���^�[�N���W�b�g HOST�ǃ|�[�g�ԍ�
	KSG_RauConf.Dpa_port_Cre_h = CPrmSS[S_CRE][23];

}																		/*							*/
