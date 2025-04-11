// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/*[]----------------------------------------------------------------------[]
 *| NT-NET�ʐM������
 *[]----------------------------------------------------------------------[]
 *| Author      : S.Takahashi
 *| Date        : 2013.02.22
 *| Update      :
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"

/*----------------------------------*/
/*		function external define	*/
/*----------------------------------*/

/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	NTCom_Init( void );
void	NTCom_ConfigInit( void );



/*[]----------------------------------------------------------------------[]*/
/*|             NTCom_Init()                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|          NT task initial routine                                       |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	NTCom_Init( void )
{

	NTCom_ConfigInit();

	/* ntcomcomm.c */
	memset(&send_blk, 0, sizeof(T_NT_BLKDATA));					// ���M�o�b�t�@		���������Ɉ����œn��
	memset(&receive_blk, 0, sizeof(T_NT_BLKDATA));				// ��M�o�b�t�@

	/* ntcomcomdr.c */
	memset(&NTComComdr_Ctrl, 0, sizeof(T_NT_COMDR_CTRL));			/* COMDR����f�[�^ */

	/* ntcomdata.c */
	/* �G���[���o�b�t�@ */
	memset(&NTCom_Err, 0, sizeof(T_NT_ERR_INFO));

	/* ntcommain.c */
	NTCom_Condit = 0;												/* task condition */
	NTCom_SciPort = 0;											/* use SCI channel number (0 or 2) */

	/** initial setting command received flag **/
	memset(&NTCom_ComErr, 0, sizeof(t_NT_ComErr));

	/* ntcomsci.c */
	/* SCI����f�[�^ */
	memset(&NTComSci_Ctrl, 0, sizeof(T_NT_SCI_CTRL));

	/* ntcomtimer.c */
	/* 1ms���x�O���[�v */
	memset(&NTComTimer_1ms, 0, sizeof(T_NT_TIM_CTRL) * NT_TIM_1MS_CH_MAX);
	/* 10ms���x�O���[�v */
	memset(&NTComTimer_10ms, 0, sizeof(T_NT_TIM_CTRL) * NT_TIM_10MS_CH_MAX);
	/* 100ms���x�O���[�v */
	memset(&NTComTimer_100ms, 0, sizeof(T_NT_TIM_CTRL) * NT_TIM_100MS_CH_MAX);
	/* �^�C�}ONOFF���� */
	NTComTimer_Enable = FALSE;	/* TRUE=�L�� */

}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTCom_ConfigInit
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ʃp�����[�^�֘A����ݒ������������
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	NTCom_ConfigInit( void )
{

	/* ��ǁ^�]�� */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.ExecMode = prm_get(COM_PRM, S_NTN, 2, 1, 1);
	NTCom_InitData.ExecMode = prm_get(COM_PRM, S_SSS, 2, 1, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* �q�@�ڑ��䐔(��ǎ�)�^���[��No.(�]�ǎ�) */
// MH364300 GG119A23(S) // GG122600(S) Y.Tanizaki NT-NET�^�[�~�i��No�g��(8->16)
//	NTCom_InitData.TerminalNum = prm_get(COM_PRM, S_NTN, 3, 1, 1);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.TerminalNum = prm_get(COM_PRM, S_NTN, 3, 2, 1);
	NTCom_InitData.TerminalNum = prm_get(COM_PRM, S_SSS, 3, 2, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
// MH364300 GG119A23(E) // GG122600(E) Y.Tanizaki NT-NET�^�[�~�i��No�g��(8->16)

	/* �ʐMBPS */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Baudrate = prm_get(COM_PRM, S_NTN, 4, 1, 1);
	NTCom_InitData.Baudrate = prm_get(COM_PRM, S_SSS, 4, 1, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* ENQ���M��̉����҂�����(t1) */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_t1 = prm_get(COM_PRM, S_NTN, 5, 4, 1);
	NTCom_InitData.Time_t1 = prm_get(COM_PRM, S_SSS, 5, 4, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* ��M�f�[�^�ő厞��(t2) */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_t2 = prm_get(COM_PRM, S_NTN, 6, 4, 1);
	NTCom_InitData.Time_t2 = prm_get(COM_PRM, S_SSS, 6, 4, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* �e�L�X�g���M��̉����҂�����(t3) */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_t3 = prm_get(COM_PRM, S_NTN, 7, 4, 1);
	NTCom_InitData.Time_t3 = prm_get(COM_PRM, S_SSS, 7, 4, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* ���ǂƂ̃C���^�[�o������(t4) */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_t4 = prm_get(COM_PRM, S_NTN, 8, 4, 1);
	NTCom_InitData.Time_t4 = prm_get(COM_PRM, S_SSS, 8, 4, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* �����ԃ^�C�}�[(t5) */
	// NOTE:���̃V���A���f�o�C�X���������ɓ��삵(I2C��SPI�Ȃ�)�A�ߏ�ɕ��ׂ������������Ƀ^�X�N���x����
	// �������ǂ����Ȃ��Ȃ�\�������邽�߁A�����ԃ^�C�}�[���킴�ƐL�΂�(�}�[�W��2�{)���Ԃ�����������
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_t5 = prm_get(COM_PRM, S_NTN, 9, 4, 1) * 2;
	NTCom_InitData.Time_t5 = prm_get(COM_PRM, S_SSS, 9, 4, 1) * 2;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* ����ENQ��̃f�[�^���MWait����(t6) */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_t6 = prm_get(COM_PRM, S_NTN, 10, 4, 1);
	NTCom_InitData.Time_t6 = prm_get(COM_PRM, S_SSS, 10, 4, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* ��M�f�[�^�ő厞��(t7) */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_t7 = prm_get(COM_PRM, S_NTN, 11, 4, 1);
	NTCom_InitData.Time_t7 = prm_get(COM_PRM, S_SSS, 11, 4, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* ���񏈗��O��WAIT����(t8) */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_t8 = prm_get(COM_PRM, S_NTN, 12, 4, 1);
	NTCom_InitData.Time_t8 = prm_get(COM_PRM, S_SSS, 12, 4, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* ���M�E�F�C�g�^�C�}�[(t9) */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_t9 = prm_get(COM_PRM, S_NTN, 13, 4, 1);
	NTCom_InitData.Time_t9 = prm_get(COM_PRM, S_SSS, 13, 4, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* �f�[�^���g���C��M�� */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Retry = prm_get(COM_PRM, S_NTN, 14, 2, 1);
	NTCom_InitData.Retry = prm_get(COM_PRM, S_SSS, 14, 2, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* �u���b�N���M�P�� */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.BlkSndUnit = prm_get(COM_PRM, S_NTN, 15, 2, 1);
	NTCom_InitData.BlkSndUnit = prm_get(COM_PRM, S_SSS, 15, 2, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* �ʐM����Ď��^�C�}�[ */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_LineMonitoring = prm_get(COM_PRM, S_NTN, 16, 2, 1);
	NTCom_InitData.Time_LineMonitoring = prm_get(COM_PRM, S_SSS, 16, 2, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* �������G���[����� */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.NoResNum = prm_get(COM_PRM, S_NTN, 17, 2, 1);
	NTCom_InitData.NoResNum = prm_get(COM_PRM, S_SSS, 17, 2, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* ���������X�L�b�v���� */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.SkipCycle = prm_get(COM_PRM, S_NTN, 18, 2, 1);
	NTCom_InitData.SkipCycle = prm_get(COM_PRM, S_SSS, 18, 2, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

	/* MAIN���W���[������̎�M�����҂����� */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j
//	NTCom_InitData.Time_ResFromMain = prm_get(COM_PRM, S_NTN, 19, 2, 1);
	NTCom_InitData.Time_ResFromMain = prm_get(COM_PRM, S_SSS, 19, 2, 1);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�ݒ�A�h���X�ύX�j

}

/*[]----------------------------------------------------------------------[]*/
/*|             ntcomtask()                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|          NT task main routine                                          |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	ntcomtask( void )
{
	NTCom_Init();

	NTCom_FuncStart();			/* function start */

	for( ;; ){
		taskchg( IDLETSKNO );

		NTCom_FuncMain();			/* RAU process exec */
	}
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
