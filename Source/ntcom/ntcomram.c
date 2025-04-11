// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"

uchar				NTCom_Condit = 0;						/* task condition */

/* ntcomtimer.c */
/* 1ms���x�O���[�v */
T_NT_TIM_CTRL		NTComTimer_1ms[NT_TIM_1MS_CH_MAX];
/* 10ms���x�O���[�v */
T_NT_TIM_CTRL		NTComTimer_10ms[NT_TIM_10MS_CH_MAX];
/* 100ms���x�O���[�v */
T_NT_TIM_CTRL		NTComTimer_100ms[NT_TIM_100MS_CH_MAX];
/* �^�C�}ONOFF���� */
BOOL				NTComTimer_Enable = FALSE;				/* TRUE=�L�� */

/*------------------------------------------------------------------------------*/
#pragma	section	_UNINIT4		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// CS2:�A�h���X���0x06039000-0x060FFFFF(1MB)
T_NTCom_SndWork		NTCom_SndWork;
T_NT_SND_TELEGRAM	NTCom_Tele_Broadcast;
T_NT_SND_TELEGRAM	NTCom_Tele_Prior;
T_NT_SND_TELEGRAM	NTCom_Tele_Normal;


/* ntcomcomm.c */
T_NT_BLKDATA		send_blk;								// ���M�o�b�t�@		���������Ɉ����œn��
T_NT_BLKDATA		receive_blk;							// ��M�o�b�t�@


/* ntcomcomdr.c */
T_NT_COMDR_CTRL		NTComComdr_Ctrl;						/* COMDR����f�[�^ */


/* ntcomdata.c */
/* �f�[�^�Ǘ��@�\ ����f�[�^ */
T_NTComData_Ctrl		NTComData_Ctrl;
/* �G���[���o�b�t�@ */
T_NT_ERR_INFO		NTCom_Err;

/* ntcommain.c */
uchar				NTCom_SciPort;							/* use SCI channel number (0 or 2) */
/** received command from IBW **/
//t_NTCom_RcvQueCtrl	NTCom_RcvQueCtrl;						/* queue control data */
															/* If you want to get this data, you call "NTCom_ArcRcvQue_Read()" */
/** initial setting command received flag **/
T_NT_INITIAL_DATA	NTCom_InitData;							/* initial setting data */
t_NT_ComErr			NTCom_ComErr;


/* ntcomsci.c */
/* SCI����f�[�^ */
T_NT_SCI_CTRL		NTComSci_Ctrl;


/* ntcomslave.c */
T_NT_BLKDATA		*NTComSlave_send_blk;					// ���M�o�b�t�@		���������Ɉ����œn��
T_NT_BLKDATA		*NTComSlave_receive_blk;				// ��M�o�b�t�@
T_NTCOM_SLAVE_CTRL	NTComComm_Ctrl_Slave;					// �]�ǊǗ��p�\����

// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
