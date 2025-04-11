/*[]----------------------------------------------------------------------[]*/
/*| headder file for system                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005.01.31                                               |*/
/*| PORTING     : 2008-10-17 MATSUSHITA for CRW                            |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include	"iodefine.h"
#define	SYSTEM_TYPE_IS_NTNET	1

/*[]----------------------------------------------------------------------[]*/
/*| shorten style symbol                                                   |*/
/*[]----------------------------------------------------------------------[]*/
typedef		unsigned char	uchar;							/*								*/
typedef		unsigned short	ushort;							/*								*/
typedef		unsigned long	ulong;							/*								*/
typedef		unsigned long long	ulonglong;					/*								*/

#define	UPDATE_A_PASS		0

#define	SYUSEI_PAYMENT		0

// �����A�����Z
/*[]----------------------------------------------------------------------[]*/
/*| �����A�����Z��۸���                                                    |*/
/*[]----------------------------------------------------------------------[]*/
#define	AUTO_PAYMENT_PROGRAM		0		// �Ȃ��i�o�׎��ݒ�j

												// հ�ް��� �� �g���@�\ �� Edy���������ƭ���ǉ����A��É�ʂ��瑗�M�ް���I���B
												// Debug���Z���[�h�𐸎Z��������� ON���Ȃ��Ă��ҋ@�ɖ߂邽�тɗL���ɂ���B�i24���j
												// �ʏ�́A��M�f�[�^�𕪊�������Ƀ��O�o�^���s�����߁A�f�[�^���������Ď�M���Ă���̂�
												// �Z�܂��Ď�M���Ă���̂��𔻒f���邱�Ƃ��o���Ȃ������B
												// �{�f�o�b�O���[�h��L���ɂ��邱�ƂŁA��M�f�[�^�i���f�[�^�j�����̂܂܃��O�ɋL�^���邱�ƂŁA
												// �ǂ̒P�ʂŃf�[�^����M�����̂���c�����邱�Ƃ��o����B

/*================  �f�o�b�O���[�h  ================*/
#define	DEBUG_MODE			1
#define	NORMAL_MODE			0

// MH810100(S)
#define	GT4100_NO_LCD		0
// MH810100(E)
/*------- DEBUG_MODE_01�F�G���[�E�A���[���}�X�N	--------*/
/*														*/
/* �Â�CRM��ł́A�ʏ��Ԃł��u�������[�_�[�E���v����*/
/* �G���[����������̂Ń}�X�N����B	�@�@�@�@�@�@�@�@�@�@*/

  #define	DEBUG_MODE_01		NORMAL_MODE		// default
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
// ���]����Edy�@�\�𖢎g�p�Ƃ���
//#define	FUNCTION_MASK_EDY	DEBUG_MODE
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�

/*														*/
/*------------------------------------------------------*/

// GG124100(S) R.Endo 2022/08/29 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]
#define DEBUG_MESSAGEFULL 0		// 1�̏ꍇ�Ƀ��b�Z�[�W�o�b�t�@�t�������p�̃f�o�b�O�@�\�����삷��
// �ڍׁF���b�Z�[�W�o�b�t�@���c��100�����ɂȂ��A9999�𔭍s���A150�ȏ�ɂȂ�Ɖ�������B
//       A9999��4���A���[�����ɂ́A���1���Ɏg�p�ʂ̑����^�X�NID�A����3���Ɏ擾�\�ȃo�b�t�@�̕ω��ʂ�����B
//       �����g�p�̃A���[�����g�p���邱�ƁB
#if DEBUG_MESSAGEFULL
#define DEBUG_ALARM_ACT 0x0045	// �f�o�b�O�p�A���[���FAlarm Action
#define DEBUG_ALARM_MD 99		// �f�o�b�O�p�A���[���FAlarm Module Code
#define DEBUG_ALARM_NO 99		// �f�o�b�O�p�A���[���FAlarm No.
#endif
// GG124100(E) R.Endo 2022/08/29 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]

// MH810105(S) R.Endo 2021/12/21 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
#define DEBUG_JOURNAL_PRINT	0	// 1�̏ꍇ�Ƀf�o�b�O�����W���[�i���Ɉ󎚂���
// MH810105(E) R.Endo 2021/12/21 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��

#define ARCSENDCLASSIC		//�]�������Ƃ���

// MH810103 GG119202(S) DT10��SPI�h���C�o�Ή�
/*--------------------------------------------------------------------------------------*/
/*	DT10��SPI�h���C�o																	*/
/*--------------------------------------------------------------------------------------*/
// DT10���g�p����ꍇ�͉��Ldefine��L���ɂ���dt_spi_drv.c��CubeSuite+�̃v���W�F�N�g�ɒǉ����Ă�������
//#define	DT10_SPI					1		// SPI�h���C�o����

#ifdef	DT10_SPI
extern void Rspi_DT10_Init(void);
#endif	// DT10_SPI
// MH810103 GG119202(E) DT10��SPI�h���C�o�Ή�

/*[]----------------------------------------------------------------------[]*/
/*| �ׯ�߼��ѐݒ�l                                                        |*/
/*[]----------------------------------------------------------------------[]*/
#define		LOCK_IF_MAX		31								/* I/F�Սő�ڑ��䐔			*/
#define		LOCK_IFS_MAX	12								/* I/F�Վq�@�ő�ڑ��䐔(1�e�@�ɑ΂���)	*/
#define		LOCK_IF_REN_MAX	54								/* �eI/F�Ղɑ΂���ۯ����u�A�Ԃ̍ő吔	*/

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�pSRAM�팸�j
//#define		LOCK_MAX		324								/* �Vۯ����u�ő�ڑ���			*/
//#define		OLD_LOCK_MAX	324								/* ��ۯ����u�ő�ڑ���			*/
#define		LOCK_MAX		150								/* �Vۯ����u�ő�ڑ���			*/
#define		OLD_LOCK_MAX	150								/* ��ۯ����u�ő�ڑ���			*/
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�pSRAM�팸�j
#define		SYU_LOCK_MAX	30								/* �W�v�G���A�i�[�Ԏ��ő吔		*/
#define		FLAP_IF_MAX		20								/* �ׯ��I/F�Սő�ڑ��䐔�i����ِ��j	*/
#define		FLAP_IFS_MAX	10								/* �ׯ�ߐڑ��䐔�i�P����قɑ΂��āj		*/
#define		INT_FLAP_MAX	20								/* �ׯ�ߍő�ڑ��䐔					*/

#define		CAR_START_INDEX		0
#define		INT_CAR_START_INDEX		50
#ifndef	BIKE_START_INDEX
#define		BIKE_START_INDEX	100
#endif
#ifndef	CAR_LOCK_MAX
#define 	CAR_LOCK_MAX		50
#endif
#ifndef	BIKE_LOCK_MAX
#define 	BIKE_LOCK_MAX		50
#endif
#ifndef	INT_CAR_LOCK_MAX
#define 	INT_CAR_LOCK_MAX	20
#endif
#ifndef	TOTAL_CAR_LOCK_MAX
#define 	TOTAL_CAR_LOCK_MAX	(CAR_LOCK_MAX+INT_CAR_LOCK_MAX)
#endif

/*[]----------------------------------------------------------------------[]*/
/*| �����ޯ����ϰؾ��                                                      |*/
/*[]----------------------------------------------------------------------[]*/

/*[]----------------------------------------------------------------------[]*/
/*| �N�������������l                                                       |*/
/*[]----------------------------------------------------------------------[]*/
#define		Clock_YEAR		2005							/* �N�����l						*/
#define		Clock_MONT		1								/* �������l						*/
#define		Clock_DAY		1								/* �������l						*/
#define		Clock_HOUR		0								/* �������l						*/
#define		Clock_MINU		0								/* �������l						*/

/*[]----------------------------------------------------------------------[]*/
/*| ү���ޑ��M�^LAG��ϰ�i�e�@�\������ϰ���j                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| ��ϰ��MAX�l�⑼������ɑ��₵�Ă�OK�B10�ʂȂ��肠��܂���B         |*/
/*| 20ms�̊����݂��g�p���Ă���̂ł��͈͓̔��ő��₵�Ă��������B           |*/
/*[]----------------------------------------------------------------------[]*/
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j
//#define		PRN_TIM_MAX		8								// �������� ��ϰ��
#define		PRN_TIM_MAX		10								// �������� ��ϰ��
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j
															// 1:�����BUSY�Ď���ϰ�iڼ�āj
															// 2:�����BUSY�Ď���ϰ�i�ެ��فj
															// 3:�߰�߰Ʊ���ފĎ���ϰ�iڼ�āj
															// 4:�߰�߰Ʊ���ފĎ���ϰ�i�ެ��فj
															// 5:���ް��ҏW�x����ϰ�iڼ�āj
															// 6:���ް��ҏW�x����ϰ�i�ެ��فj
															// 7:ڼ�Ĉ󎚊�������ϰ
															// 8:�ެ��و󎚊�������ϰ
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j
															// 9:�����^�����ݒ�҂��^�C���A�E�g
															// 10:�f�[�^�����݊����҂��^�C�}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�f�[�^�����݊����҂��^�C�}�ύX�j

#define		NTNET_TIM_MAX	2								/* NTNET��� ��ϰ��				*/
#define		NTNETDOPA_TIM_MAX	2							/* NTNET-DOPA��� ��ϰ��			*/
#define		REMOTEDL_TIM_MAX	5							/* ���u�޳�۰����� ��ϰ��		*/
#define		CAN_TIM_MAX	2									/* CAN��� ��ϰ��				*/
// MH810100(S) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
// PKT��� ��ϰ
enum{
	PKT_TIM_CONNECT = 1,									// �ڑ��R�}���h�J�n�^�C�}
	PKT_TIM_CON_RES_WT,										// �R�l�N�V���������҂�����
	PKT_TIM_CON_INTVL,										// �R�l�N�V�������M�Ԋu
	PKT_TIM_CON_DIS_WT,										// �R�l�N�V�����ؒf�҂�����
	PKT_TIM_RES_WT,											// �p�P�b�g�����҂�����
	PKT_TIM_KEEPALIVE,										// �L�[�v�A���C�u���M�Ԋu
	PKT_TIM_RTPAY_RES,										// ���Z�����f�[�^�҂��^�C�}
	PKT_TIM_DC_QR_RES,										// QR�m��E��������f�[�^�҂��^�C�}
	PKT_TIM_RTPAY_RESND,									// �đ��E�F�C�g�^�C�}
	PKT_TIM_DC_QR_RESND,									// �đ��E�F�C�g�^�C�}
	PKT_TIM_DISCON_WAIT,									// �R�l�N�V�����ؒf���o�҂��^�C�}
// MH810100(S) K.Onodera 2020/03/12 �Ԕԃ`�P�b�g���X(�ڑ��G���[�K�[�h�^�C�}�ǉ�)
	PKT_TIM_ERR_GUARD,										// �ڑ��G���[�K�[�h�^�C�}
// MH810100(E) K.Onodera 2020/03/12 �Ԕԃ`�P�b�g���X(�ڑ��G���[�K�[�h�^�C�}�ǉ�)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	PKT_TIM_DC_LANE_RES,									// ���[�����j�^�����f�[�^�҂��^�C�}
	PKT_TIM_DC_LANE_RESND,									// �đ��E�F�C�g�^�C�}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	PKT_TIM_RTRECEIPT_RES,									// �̎��؃f�[�^�����҂��^�C�}
	PKT_TIM_RTRECEIPT_RESND,								// �đ��E�F�C�g�^�C�}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	// ���ǉ��͂��̏��
	PKT_TIM_END
};
#define PKT_TIM_MAX	 (PKT_TIM_END - 1)	// PKT��� ��ϰ��
// MH810100(E) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j

/*[]----------------------------------------------------------------------[]*/
/*| �֐�Call�^LAG��ϰ��`(�e���������ϰ�Ŏg�p�\����ϰ����)               |*/
/*[]----------------------------------------------------------------------[]*/
/*** x10ms Lag timer ***/
#define		LAG10_MAX		1								/* 10ms���������ϰ��(base 2ms)	*/

/*** x20ms Lag timer ***/

enum{
	LAG20_RD_SHTCTL = 0,									/* ���Cذ�ް���������			*/
	LAG20_CN_SHTCTL,										/* ��ݓ��������������			*/
	LAG20_OUTCNT1,											/* ���Z�����M��1(�o�ɐM��1)����	*/
	LAG20_OUTCNT2,                                          /* ���Z�����M��2(�o�ɐM��2)����	*/
	LAG20_GATEOPN,                                          /* �ްĊJ�M������				*/
	LAG20_GATECLS,                                          /* �ްĕM������				*/
	LAG20_RD_SHTCLS,										/* ���Cذ�ް������҂���ϰ		*/
	LAG20_KEY_REPEAT,
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	LAG20_JNL_PRI_WAIT_TIMER,								// �W���[�i���󎚊����҂�
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	
	// �^�C�}�[�ǉ�����ꍇ�ɂ͏�L����ɋL�ڂ��邱��
	LAG20_MAX
};


/*** x500ms Lag timer ***/
#define		LAG500_PRINTER_ERR_RECIEVE	0					/* ������װ��M���荞�݂̘A�������h�~�̈�				*/
#define		LAG500_MIF_WRITING_FAIL_BUZ	1					/* MIFARE������NG�p�޻ް */
#define		LAG500_MIF_LED_ONOFF		2					/* ICC LED �_�ŗp		 */
#define		LAG500_SUICA_STATUS_TIMER			3			/* SX-10��ԊĎ���� */
#define		LAG500_SUICA_NONE_TIMER				4			/* ����f�[�^�̑��M���̖�����(��t�s��) */
#define		LAG500_SUICA_MIRYO_RESET_TIMER		5			/* ������ԉ�����ϰ */
#define		LAG500_SUICA_ZANDAKA_RESET_TIMER	6			/* Suika�c���s���\��������ϰ */
#define		LAG500_RECEIPT_LEDOFF_DELAY			7			/* ���Z������ �̎������s���̎�o����LED�_������ */
#define		LAG500_EDY_LED_RESET_TIMER			8			/* EDYLED������ϰ */
#define		LAG500_VIB_SNS_TIMER				9			/* �U�����m��ϰ */

enum {
	LAG500_iDC_TIMER = 10,
	LAG500_ERROR_DISP_DELAY,
	LAG500_CREDIT_ANA_TIMER,
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	LAG500_EDY_STOP_RECV_TIMER,								// Edy��~������M�҂��^�C�}�[�i�ݒ�{10�b�j
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	LAG500_REMOTEDL_RETRY_CONNECT_TIMER,					// ���u�_�E�����[�h�p�ڑ����g���C�^�C�}
	LAG500_DISCONNECT_WAIT_TIMER,							// Rism�ؒf�҂��^�C�}�[
	LAG500_CAN_HEARTBEAT_TIMER,								// �n�[�g�r�[�g���M�����^�C�}�[
	LAG500_CAPPI_CONNECT_TIMER,								// Cappi TCP�R�l�N�g�҂��^�C�}
	LAG500_CAPPI_CLOSE_TIMER,								// Cappi TCP�ؒf�҂��^�C�}
	LAG500_SUICA_NO_RESPONSE_TIMER,							/* ����f�[�^�̑��M���̖������p(��t��) */
// MH321800(S) G.So IC�N���W�b�g�Ή�
	LAG500_EC_NOT_READY_TIMER,								// ����f�[�^�̑��M���̖������p(��t��)
	LAG500_EC_AUTO_CANCEL_TIMER,							// �A���[�������̐��Z�����L�����Z���^�C�}
	LAG500_EC_WAIT_BOOT_TIMER,								// ���σ��[�_�N�������҂��^�C�}�[
	LAG500_EC_START_REMOVAL_TIMER,							// ���σ��[�_�J�[�h�������҂��N���^�C�}�[
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	LAG500_EJA_PINFO_RES_WAIT_TIMER,						// �d�q�W���[�i���v�����^��񉞓��҂��^�C�}
	LAG500_EJA_RESET_WAIT_TIMER,							// �d�q�W���[�i�����Z�b�g�����҂��^�C�}
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810105(S) MH364301 �C���{�C�X�Ή�
	LAG500_JNL_PRI_WAIT_TIMER,								// �W���[�i���󎚊����҂�
	LAG500_RECEIPT_MISS_DISP_TIMER,							// �̎��؎��s�\���^�C�}
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	LAG500_RCT_PINFO_RES_WAIT_TIMER,						// ���V�[�g�v�����^��񉞓��҂��^�C�}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	// ����ǉ��^�C�}�[�͂����ɒǉ�
	LAG500_MAX
};


/*[]----------------------------------------------------------------------[]*/
/*| TCB,MCB����                                                            |*/
/*[]----------------------------------------------------------------------[]*/
#define		MSGBUF_CNT		512								/* number of MCB max			*/
#define		MSGBUFSZ		64								/* message buffer size			*/

enum{
	OPETCBNO = 0,
	PRNTCBNO,
	FLPTCBNO,
	NTNETTCBNO,
	NTNETDOPATCBNO,
	REMOTEDLTCBNO,												/* TCB For Remote Download task	*/
	CANTCBNO,
	CAPPITCBNO,
// MH810100(S) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
	PKTTCBNO,
// MH810100(E) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
	// ������͂��̏�ɒǉ����Ă������Ɓ�
	TCB_CNT
};

#define		MSG_SET			0x01							/*								*/
#define		MSG_EMPTY		0x00							/*								*/

/*[]----------------------------------------------------------------------[]*/
/*| ����ԍ���`                                                            |*/
/*[]----------------------------------------------------------------------[]*/
enum {
			IDLETSKNO = 0,									/* idle task					*/
			OPETSKNO,										/* operation task				*/
			CNMTSKNO,										/* coinmech and notereader task */
			PRNTSKNO,										/* printer task					*/
// MH810100(S) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
//			LKCOMTSKNO,										/* lkcom task					*/
			PKTTSKNO,										// PKT task
// MH810100(E) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
			FLATSKNO,										/* flashrom task				*/
			NTNETTSKNO,										/* nt-net task					*/
			REMOTEDLTSKNO,									/* Remote Download task			*/
			CANTSKNO,										/* can task						*/
			MRDTSKNO,										/* Mag Reader task				*/
			KSGTSKNO,
			IDLESUBTSKNO,									/* idle sub task				*/
			PIPTSKNO,										/* ParkiPro task				*/
			RAUTSKNO,										/* rau task						*/
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
			NTCOMTSKNO,										/* nt task(NT-NET�ʐM������)	*/
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
			TSKMAX											/* count of task */
};
/*[]----------------------------------------------------------------------[]*/
/*| �߰Ċ���t����`                                                       |*/
/*[]----------------------------------------------------------------------[]*/

#define		FNT_DIPSW_1		key_dat[0].BIT.B7				/* �f�B�b�v�X�C�b�`1			*/
#define		FNT_DIPSW_2		key_dat[0].BIT.B6				/* �f�B�b�v�X�C�b�`2			*/
#define		FNT_DIPSW_3		key_dat[0].BIT.B5				/* �f�B�b�v�X�C�b�`3			*/
#define		FNT_DIPSW_4		key_dat[0].BIT.B4				/* �f�B�b�v�X�C�b�`4			*/

#define		TENKEY_0		key_dat[1].BIT.B7				/* �e���L�[0					*/
#define		TENKEY_1		key_dat[1].BIT.B6				/* �e���L�[1					*/
#define		TENKEY_2		key_dat[1].BIT.B5				/* �e���L�[2					*/
#define		TENKEY_3		key_dat[1].BIT.B4				/* �e���L�[3					*/
#define		TENKEY_4		key_dat[1].BIT.B3				/* �e���L�[4					*/
#define		TENKEY_5		key_dat[1].BIT.B2				/* �e���L�[5					*/
#define		TENKEY_6		key_dat[1].BIT.B1				/* �e���L�[6					*/
#define		TENKEY_7		key_dat[1].BIT.B0				/* �e���L�[7					*/

#define		TENKEY_8		key_dat[2].BIT.B7				/* �e���L�[8					*/
#define		TENKEY_9		key_dat[2].BIT.B6				/* �e���L�[9					*/
#define		TENKEY_F1		key_dat[2].BIT.B5				/* �e���L�[F1(���Z)				*/
#define		TENKEY_F2		key_dat[2].BIT.B4				/* �e���L�[F2(�̎���)			*/
#define		TENKEY_F3		key_dat[2].BIT.B3				/* �e���L�[F3(���ԏؖ���)		*/
#define		TENKEY_F4		key_dat[2].BIT.B2				/* �e���L�[F4(���)				*/
#define		TENKEY_F5		key_dat[2].BIT.B1				/* �e���L�[F5(�o�^)				*/
#define		TENKEY_C		key_dat[2].BIT.B0				/* �e���L�[C					*/

#define		FNT_CN_DRSW		key_dat[3].BIT.B7				/* �R�C�����b�N��				*/
#define		FNT_NT_BOX_SW	key_dat[3].BIT.B6				/* �������[�_�[��				*/
#define		FNT_CN_BOX_SW	key_dat[3].BIT.B5				/* �R�C�����ɒE��				*/
#define		FNT_NT_FALL_SW	key_dat[3].BIT.B4				/* �������[�_�[�E��				*/
#define		FNT_VIB_SNS		key_dat[3].BIT.B3				/* �U���Z���T�[					*/
#define		FNT_MAN_DET		key_dat[3].BIT.B2				/* �l�̌��m�Z���T�[				*/

#define		RT_SW0			port_in_dat.BIT.B0				/* (RTSW0)					*/
#define		RT_SW1			port_in_dat.BIT.B1				/* (RTSW1)					*/
#define		RT_SW2			port_in_dat.BIT.B2				/* (RTSW2)					*/
#define		RT_SW3			port_in_dat.BIT.B3				/* (RTSW3)					*/
#define		DIP_SW0			port_in_dat.BIT.B4				/* (DIPSW0)					*/
#define		DIP_SW1			port_in_dat.BIT.B5				/* (DIPSW1)					*/
#define		DIP_SW2			port_in_dat.BIT.B6				/* (DIPSW2)					*/
#define		DIP_SW3			port_in_dat.BIT.B7				/* (DIPSW3)					*/

#define		FNTH_OFF		port_in_dat.BIT.B10				/* (TH_OFF)					*/
#define		FNTH_ON			port_in_dat.BIT.B11				/* (TH_ON)					*/
#define		SC_KEY			port_in_dat.BIT.B12				/* (C_KEY)					*/
#define		SM_KEY			port_in_dat.BIT.B13				/* (M_KEY)					*/
#define		RXM1_IN			port_in_dat.BIT.B14				/* (M_IN)					*/
#define		RXI1_IN			port_in_dat.BIT.B15				/* (RXI_IN)					*/

extern	unsigned char	SD_LCD;								/* �ޯ�ײē_����� 0=���� 1=�_��*/
extern	unsigned short	SD_EXIOPORT;						/* �g��I/O�|�[�g�C���[�W�o�b�t�@*/

/*[]----------------------------------------------------------------------[]*/
/*| �߰Ċ���t����`                                                       |*/
/*[]----------------------------------------------------------------------[]*/
#define		CP_CN_SYN		PORT8.PODR.BIT.B6				/* COINMECH SYN					*/

#define		CP_FB_RES		PORT3.PODR.BIT.B2				/* FB-7000�̃��Z�b�g�o��		*/

#define		CP_RES_DET1		PORT1.PODR.BIT.B1				/* �\������1					*/

#define		CP_IF_RTS		PORT7.PODR.BIT.B0				/* APS�ʐM IF�ՒʐM RTS����M��		*/
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#define		CP_NT_RTS		PORT6.PODR.BIT.B0				/* NT-NET�ʐM RTS����M��		*/
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#define		CP_CAN_RES		PORT6.PODR.BIT.B6				/* CAN��ؾ�ďo��				*/

#define		CP_RED_SHUT_OPEN	PORT0.PODR.BIT.B3			// ���C���[�_�[�V���b�^�I�[�v��
#define		CP_RED_SHUT_CLOSE	PORT0.PODR.BIT.B5			// ���C���[�_�[�V���b�^�N���[�Y
#define		CP_FAN_CTRL_SW		PORT6.PODR.BIT.B1			// FAN����SW
#define		CP_DOOR_OPEN_SEN	PORT6.PIDR.BIT.B2			// �h�A�J�Z���T
#define		CP_RD_SHUTLED		PORT8.PODR.BIT.B7			// ���C���[�_�[�V���b�^LED

#define		CP_MODECHG		(0 == OPE_SIG_DOORNOBU_Is_OPEN)	// �h�A�m�u��Ԃ�ݒ�Ӱ�޽����ɑ�p 0:ON 1:OFF

#define		MISC0_CP_SUB_CPU	0x80						/* �T�uCPU���Z�b�g���� H=���Z�b�g         �������݂�OutMISC0wt���g�p�C�ǂݏo����SD_MISC0���Q�Ƃ��邱��*/

#define		MISC2_CP_CPU_HWRST2	0x08						/* CPU�n�[�h�E�F�A���Z�b�g���� H=���Z�b�g �������݂�OutMISC2wt���g�p�C�ǂݏo����SD_MISC2���Q�Ƃ��邱��*/
#define		MISC2_CP_PRN2_RES	0x10						/* ���V�[�g�v�����^2���Z�b�g H=���Z�b�g   �������݂�OutMISC2wt���g�p�C�ǂݏo����SD_MISC2���Q�Ƃ��邱��*/
#define		MISC2_CP_RAM_WPT	0x20						/* RAM WRITE PROTECT                      �������݂�OutMISC2wt���g�p�C�ǂݏo����SD_MISC1���Q�Ƃ��邱��*/
#define		MISC2_CP_LD1		0x40						/* LD1�_������ H=�_��                     �������݂�OutMISC2wt���g�p�C�ǂݏo����SD_MISC2���Q�Ƃ��邱��*/
#define		MISC2_CP_LD2		0x80						/* LD2�_������ H=�_��                     �������݂�OutMISC2wt���g�p�C�ǂݏo����SD_MISC2���Q�Ƃ��邱��*/
/*[]----------------------------------------------------------------------[]*/
/*| Sodiac��`   		                                                   |*/
/*[]----------------------------------------------------------------------[]*/
#define		SODIAC_MUTE_SW		PORT0.PODR.BIT.B2			/*	SodiacMute switch	*/

/*----------------------------------*/
/* CRC-CCITT :  x^{16}+x^{12}+x^5+1 */
/*----------------------------------*/
#define		CRCPOLY1			0x1021						/* �����						*/
#define		CRCPOLY2			0x8408  					/* �E���						*/
#define		CHAR_BIT			8							/* number of bits in a char		*/
#define		L_SHIFT				0							/* �����						*/
#define		R_SHIFT				1							/* �E���						*/

/*** structure ***/
/*[]----------------------------------------------------------------------[]*/
/*| ��ϰ�ϐ�                                                               |*/
/*[]----------------------------------------------------------------------[]*/
extern	unsigned short OPE_Timer[];							/* ���ڰ�������p��ϰ			*/
extern	unsigned short PRN_Timer[PRN_TIM_MAX];				/* ���������p��ϰ				*/
extern	unsigned short NTNET_Timer[NTNET_TIM_MAX];			/* NTNET����p��ϰ				*/
extern	unsigned short NTNETDOPA_Timer[NTNETDOPA_TIM_MAX];	/* NTNET-DOPA����p��ϰ			*/
extern	unsigned short REMOTEDL_Timer[REMOTEDL_TIM_MAX];	/* NTNET-DOPA����p��ϰ			*/
extern	unsigned short CAN_Timer[CAN_TIM_MAX];				/* CAN����p��ϰ					*/
// MH810100(S) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
extern	unsigned short PKT_Timer[PKT_TIM_MAX];				// PKT����p��ϰ
// MH810100(E) K.Onodera 2019/10/31 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j

/*[]----------------------------------------------------------------------[]*/
/*| �����݊֘A��`                                                         |*/
/*[]----------------------------------------------------------------------[]*/
#define		KeyDatMax		4								/* ����ײݐ�					*/
// MH810100(S) K.Onodera 2019/11/06 �Ԕԃ`�P�b�g���X(���Z�@�|LCD�ԒʐM�@�\�ǉ�)
#define		KeyDatMaxLCD	2								/* LCD����̃{�^�����Byte��	*/
// MH810100(E) K.Onodera 2019/11/06 �Ԕԃ`�P�b�g���X(���Z�@�|LCD�ԒʐM�@�\�ǉ�)
#define		PortInTblMax	16								/* �����߰Đ�					*/

union	key_rec {
	unsigned char BYTE;
	struct {
		unsigned char B7:1;									/* Bit 7						*/
		unsigned char B6:1;									/* Bit 6						*/
		unsigned char B5:1;									/* Bit 5						*/
		unsigned char B4:1;									/* Bit 4						*/
		unsigned char B3:1;									/* Bit 3						*/
		unsigned char B2:1;									/* Bit 2						*/
		unsigned char B1:1;									/* Bit 1						*/
		unsigned char B0:1;									/* Bit 0						*/
	} BIT;
};

extern	union	key_rec	key_dat[KeyDatMax];					/* �������ް��Ұ��				*/
// MH810100(S) K.Onodera 2019/11/06 �Ԕԃ`�P�b�g���X(���Z�@�|LCD�ԒʐM�@�\�ǉ�)
extern	union	key_rec	key_dat_LCD[KeyDatMaxLCD];			/* LCD�����M�����O�{�^�����	*/
// MH810100(E) K.Onodera 2019/11/06 �Ԕԃ`�P�b�g���X(���Z�@�|LCD�ԒʐM�@�\�ǉ�)
union	port_in_rec {
	unsigned short WORD;
	struct {
		unsigned char B15:1;								/* Bit 15						*/
		unsigned char B14:1;								/* Bit 14						*/
		unsigned char B13:1;								/* Bit 13						*/
		unsigned char B12:1;								/* Bit 12						*/
		unsigned char B11:1;								/* Bit 11						*/
		unsigned char B10:1;								/* Bit 10						*/
		unsigned char B9:1;									/* Bit 9						*/
		unsigned char B8:1;									/* Bit 8						*/
		unsigned char B7:1;									/* Bit 7						*/
		unsigned char B6:1;									/* Bit 6						*/
		unsigned char B5:1;									/* Bit 5						*/
		unsigned char B4:1;									/* Bit 4						*/
		unsigned char B3:1;									/* Bit 3						*/
		unsigned char B2:1;									/* Bit 2						*/
		unsigned char B1:1;									/* Bit 1						*/
		unsigned char B0:1;									/* Bit 0						*/
	} BIT;
};
extern	union	port_in_rec	port_in_dat;					/* �����߰��ް��Ұ��			*/
extern	const	unsigned short scn_evt_tbl[16];				/* ����ć��o�^					*/
extern	unsigned short	port_data[3];						/* �g��I/O�߰���������p�ޯ̧	*/
extern	unsigned char	door_data[3];						/* �h�A�J�Z���T�����������p	*/
extern	unsigned char	doorDat;							/* �O��h�A�J�Z���T���			*/
extern	unsigned char	doornobu_data[3];					/* �h�A�m�u�Z���T�����������p	*/
extern	unsigned char	doornobuDat;						/* �O��h�A�m�u�Z���T���		*/
extern	char		port_indat_onff[PortInTblMax];			// �g��I/O�|�[�g��on/off�t���O
extern	char		door_indat_onff;						// �h�A�J�Z���T��on/off�t���O
extern	uchar		f_port_scan_OK;							// ���荞�݃X�L�������t���O 0:�֎~ 1:����

extern	volatile unsigned char	IN_TIM_Cnt;					/* ���̷݂����݂̎��			*/

/*[]----------------------------------------------------------------------[]*/
/*| �޻ް�֘A��`                                                          |*/
/*[]----------------------------------------------------------------------[]*/

/*[]----------------------------------------------------------------------[]*/
/*| ���v��`                                                               |*/
/*[]----------------------------------------------------------------------[]*/
															/*------------------------------*/
struct	clk_rec	{											/* Clock Data					*/
															/* ---------------------------- */
	unsigned short	year;									/* Year							*/
	unsigned char	mont;									/* Month (1-12)					*/
	unsigned char	date;									/* Date  (1-31)					*/
	unsigned char	hour;									/* Hour  (0-23)					*/
	unsigned char	minu;									/* Minute(0-59)					*/
	unsigned char	seco;									/* Second(0-59)					*/
	unsigned char	week;									/* Day   (0:SUN-6:SAT)			*/
	unsigned short	ndat;									/* Normalize Date				*/
	unsigned short	nmin;									/* Normalize Minute				*/
															/*------------------------------*/
};
															/*------------------------------*/
extern	struct	clk_rec	CLK_REC;							/* Clock Data Recive Buffer		*/
															/*------------------------------*/
extern	struct	clk_rec	clk_save;							// ��d����

#define	_1DAY_TIME_MILLI_SEC		86400000L				// 24���Ԃ�ms�P�ʂŕ\�������l
/*[]----------------------------------------------------------------------[]*/
/*| �N���������m�[�}���C�Y�\����                                           |*/
/*[]----------------------------------------------------------------------[]*/
typedef	union {
															/*------------------------------*/
	ulong	ul;												/* b31-16=�N����, b15-0=����	*/
	ushort	us[2];											/* [0]=�N����, [1]=����			*/
															/*------------------------------*/
} t_NrmYMDHM;
#pragma pack
/*[]----------------------------------------------------------------------[]*/
/*| ���t�\����                                                             |*/
/*[]----------------------------------------------------------------------[]*/
// MH810100(S)
typedef	struct {
															/*------------------------------*/
	unsigned short	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
	unsigned char	Hour;									/* Hour							*/
	unsigned char	Min;									/* Minute						*/
	unsigned char	Sec;									/* Second						*/
															/*------------------------------*/
} DATE_YMDHMS;
// MH810100(E)

// MH810100(S) K.Onodera 2020/01/12 �Ԕԃ`�P�b�g���X�iQR�m��E����f�[�^�j
typedef	struct {
															/*------------------------------*/
	unsigned short	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
															/*------------------------------*/
} DATE_YMD;
// MH810100(E) K.Onodera 2020/01/12 �Ԕԃ`�P�b�g���X�iQR�m��E����f�[�^�j

typedef	struct {
															/*------------------------------*/
	unsigned short	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
	unsigned char	Hour;									/* Hour							*/
	unsigned char	Min;									/* Minute						*/
															/*------------------------------*/
}	date_time_rec;

/*[]----------------------------------------------------------------------[]*/
/*| ���t�\����2                                                            |*/
/*[]----------------------------------------------------------------------[]*/
typedef	struct {
															/*------------------------------*/
	unsigned short	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
	unsigned char	Hour;									/* Hour							*/
	unsigned char	Min;									/* Minute						*/
	unsigned short	Sec;									/* Second						*/
															/*------------------------------*/
}	date_time_rec2;

/*[]----------------------------------------------------------------------[]*/
/*| ���t�\����3                                                            |*/
/*[]----------------------------------------------------------------------[]*/
typedef	struct {
															/*------------------------------*/
	unsigned char	Year;									/* Year							*/
	unsigned char	Mon;									/* Month						*/
	unsigned char	Day;									/* Day							*/
	unsigned char	Hour;									/* Hour							*/
	unsigned char	Min;									/* Minute						*/
	unsigned char	Sec;									/* Second						*/
															/*------------------------------*/
}	date_time_rec3;
#pragma unpack

/*[]----------------------------------------------------------------------[]*/
/*| ү���ޒ�`                                                             |*/
/*[]----------------------------------------------------------------------[]*/
typedef	struct {
															/*------------------------------*/
	unsigned short	command;								/* command						*/
															/*------------------------------*/
	unsigned char	data[MSGBUFSZ];							/* infomation					*/
															/*------------------------------*/
}	MSG;

typedef	struct {
															/*------------------------------*/
	char	*msg_next;										/* ���ޯ̧���ڽ					*/
															/*------------------------------*/
	MSG		msg;											/* 								*/
															/*------------------------------*/
} MsgBuf;

extern	MsgBuf	msgbuf[MSGBUF_CNT];

typedef struct {
															/*------------------------------*/
	MsgBuf		*msg_top;									/* ү���ސ擪���ڽ				*/
															/*------------------------------*/
	MsgBuf		*msg_end;									/* ү���ލŏI���ڽ				*/
															/*------------------------------*/
} MCB;

extern	MCB		mcb;

typedef	struct {
															/*------------------------------*/
	unsigned char 	event;									/* �N�� 1(ON)/0(OFF)            */
															/*------------------------------*/
	unsigned char	level;									/* �D�揇��7�`0                 */
															/*------------------------------*/
	unsigned short	status;									/* ����ð��						*/
															/*------------------------------*/
	MsgBuf			*msg_top;								/* �擪������ޯ̧���ڽ			*/
															/*------------------------------*/
	MsgBuf			*msg_end;								/* �ŏI������ޯ̧���ڽ			*/
															/*------------------------------*/
} TCB;
#define	TARGET_MSGGET_PRM_MAX	8
typedef struct {
	unsigned short	Count;									/* �o�^����						*/
	unsigned short	Command[TARGET_MSGGET_PRM_MAX];			/* ү����ID						*/
} t_TARGET_MSGGET_PRM;

extern	TCB		tcb[];

extern	char	CNMTSK_START;								/* ���ү�����N���׸�			*/
extern	char	RMTCOM_START;								/* ���u�ʐM�N���׸�				*/
extern	char	PCCOM_START;								/* PC�ʐM&���u�ʐM�N���׸�		*/
extern	unsigned short	SUBCPU_MONIT;						/* ���CPU�Ď���ϰ				*/
extern	unsigned char	WPTCOUNT;							/* write protect off count		*/
extern	unsigned long	LifeTimer10ms;
extern	unsigned char	Tim10msCount;						/* 10ms�ώZ�J�E���^(every 2ms + 1) */
extern	unsigned char	xPauseStarting;						/* xPause()�ŵ��������N�������� */
extern	unsigned char	xPauseStarting_PRNTSK;				/* xPause()�������������N�������� */
extern	unsigned char	Tim500msCount;						/* 500ms�ώZ�J�E���^(every 20ms + 1) */
extern	unsigned char	TIM500_START;						/* 500ms interval�o�߃t���O (0=�o�߂��ĂȂ��A�ȊO��N��o��) */
extern	unsigned char	SIGCHK_TRG;							/* �o�͐M�������A���������ضޗp	*/
typedef union{
	unsigned short	USHORT;
	struct{
		unsigned char	RESERVE_BIT15		:1;						// Bit 15 = �\��
		unsigned char	RESERVE_BIT14		:1;						// Bit 14 = �\��
		unsigned char	RESERVE_BIT13		:1;						// Bit 13 = �\��
		unsigned char	RESERVE_BIT12		:1;						// Bit 12 = �\��
		unsigned char	RESERVE_BIT11		:1;						// Bit 11 = �\��
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
//		unsigned char	RESERVE_BIT10		:1;						// Bit 10 = �\��
		unsigned char	R_PRINTER_RCV_INFO	:1;						// Bit 10 = ���V�[�g�v�����^��񉞓���M
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//		unsigned char	RESERVE_BIT9		:1;						// Bit 9 = �\��
//		unsigned char	RESERVE_BIT8		:1;						// Bit 8 = �\��
		unsigned char	EJA_RCV_STS			:1;						// Bit 9 = �d�q�W���[�i���v�����^�X�e�[�^�X��M
		unsigned char	J_PRINTER_RCV_INFO	:1;						// Bit 8 = �W���[�i���v�����^��񉞓���M
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// Sc16RecvInSending_IRQ5_Level_L
//		unsigned char	RESERVE_BIT7		:1;						// Bit 7 = �\��
		unsigned char	Print1ByteSendReq	:1;						// Bit 7 = �\��
		unsigned char	J_PRINTER_NEAR_END	:1;						// Bit 6 = �v�����^�̃j�A�G���h�X�e�[�^�X���
		unsigned char	J_PRINTER_RCV_CMP	:1;						// Bit 5 = �W���[�i���v�����^��M����
		unsigned char	J_PRINTER_SND_CMP	:1;						// Bit 4 = �W���[�i���v�����^���M����
		unsigned char	R_PRINTER_NEAR_END	:1;						// Bit 3 = �v�����^�̃j�A�G���h�X�e�[�^�X���
		unsigned char	R_PRINTER_RCV_CMP	:1;						// Bit 2 = ���V�[�g�v�����^��M����
		unsigned char	R_PRINTER_SND_CMP	:1;						// Bit 1 = ���V�[�g�v�����^���M����
		unsigned char	RTC_1_MIN_IRQ		:1;						// Bit 0 = RTC�����i���荞�ݔ���
	} BIT;
} t_I2C_EVENT;

extern	t_I2C_EVENT	I2C_Event_Info;									// I2C�C�x���g����
extern	unsigned char	RP_OV_PA_FR;								/* ڼ�Ă̵��ް�݁A���è�A�ڰ�ݸ޴װ����	*/
extern	unsigned char	JP_OV_PA_FR;								/* �ެ��ق̵��ް�݁A���è�A�ڰ�ݸ޴װ����	*/
typedef union{
	unsigned short	USHORT;
	struct{
		unsigned char	I2C_BFULL_RECV_PIP		:1;		// �\��(PIP��M�o�b�t�@�t��)	����/������
		unsigned char	I2C_BFULL_SEND_PIP		:1;		// �\��(PIP���M�o�b�t�@�t��)	����/������
		unsigned char	I2C_BFULL_RECV_CAPPI	:1;		// �\��(CAPPI��M�o�b�t�@�t��)	����/������
		unsigned char	I2C_BFULL_SEND_CAPPI	:1;		// �\��(CAPPI���M�o�b�t�@�t��)	����/������
		unsigned char	I2C_BFULL_RECV_JP		:1;		// BIT11 = �W���[�i���v�����^��M�o�b�t�@�t��(�wI2C�ެ��َ�M�ޯ̧�فx)	����/������
		unsigned char	I2C_BFULL_SEND_JP		:1;		// BIT10 = �W���[�i���v�����^���M�o�b�t�@�t��(�wI2C�ެ��ّ��M�ޯ̧�فx)	����/������
		unsigned char	I2C_BFULL_RECV_RP		:1;		// BIT 9 = ���V�[�g�v�����^��M�o�b�t�@�t��(�wI2Cڼ�Ď�M�ޯ̧�فx)		����/������
		unsigned char	I2C_BFULL_SEND_RP		:1;		// BIT 8 = ���V�[�g�v�����^���M�o�b�t�@�t��(�wI2Cڼ�đ��M�ޯ̧�فx)		����/������
		unsigned char	BIT7					:1;		// BIT 7 = Reserved
		unsigned char	BIT6					:1;		// Bit 6 = Reserved
		unsigned char	BIT5					:1;		// Bit 5 = Reserved
		unsigned char	I2C_PREV_TOUT			:1;		// Bit 4 = �O��Status�ω��҂�Timeout	������������
		unsigned char	I2C_BUS_BUSY			:1;		// Bit 3 = �o�XBUSY						������������
		unsigned char	I2C_SEND_DATA_LEVEL		:1;		// Bit 2 = ���M�f�[�^���x���G���[		������������
		unsigned char	I2C_CLOCK_LEVEL			:1;		// Bit 1 = �N���b�N���x���G���[			������������ 
		unsigned char	I2C_R_W_MODE			:1;		// Bit 0 = Read/Write���[�h�G���[		������������
	} BIT;
} t_I2C_BUSERROR;

typedef struct {
	unsigned char	SendRP;
	unsigned char	RecvRP;
	unsigned char	SendJP;
	unsigned char	RecvJP;
} t_I2C_ERR_STS_EVENT;

extern	t_I2C_BUSERROR			I2C_BusError_Info;
extern	t_I2C_ERR_STS_EVENT		I2cErrEvent;
extern	t_I2C_ERR_STS_EVENT		I2cErrStatus;


// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
extern	unsigned long	LifeTimer1ms;						/* 1ms�ώZ����					*/
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
extern	unsigned long	LifeTimer2ms;						/* 2ms�ώZ����(Use IFcom)		*/
extern	uchar		Tim1sCount;									// 1s�ώZ�J�E���^(every 20ms + 1) 
extern	unsigned short	FBcom_2msT1;
extern	unsigned short	FBcom_2msT2;
extern	unsigned short	FBcom_20msT1;
extern	unsigned short	FBcom_20msT2;
extern	unsigned short	Capcom_2msT1;
extern	unsigned short	Capcom_2msT2;
extern	unsigned short	Capcom_20msT1;
extern	unsigned short	Capcom_20msT2;
extern	unsigned short	Capcom_20msT3;
extern	ushort			Btcom_1msT1;								// �����ԊĎ��^�C�}�[
extern	ushort			Btcom_20msT1;								// ��M�҂��^�C���A�E�g�p
extern	uchar			f_bluetooth_init;							// bluetooth�C�j�V�����V�[�P���X����t���O
#define FAN_EXE_BUF 10
typedef struct {
	ushort	year;									// Year
	uchar	mont;									// Month (1-12)
	uchar	date;									// Date  (1-31)
	uchar	hour;									// Hour  (0-23)
	uchar	minu;									// Minute(0-59)
	uchar	f_exe;									// 0:�f�[�^�Ȃ� 1:FAN ON 2:FAN OFF
} t_fan_time;
typedef struct {
	t_fan_time	fan_exe[FAN_EXE_BUF];
	uchar		index;
} t_fan_time_ctrl;
extern	t_fan_time_ctrl	fan_exe_time;								// FAN���쓮�J�n�������Ԃ̊i�[�o�b�t�@
#define	FLT_SWDATA_WRITE_SIZE			256 // FROM�̏������ݍŏ��T�C�Y��256BYTE�̂���
extern unsigned char swdata_write_buf[FLT_SWDATA_WRITE_SIZE];
extern	unsigned short	FLP_TIM_CTRL;
#define		rangechk(c,d,e)	((c)<=(e)&&(e)<=(d))			/* �͈�����						*/
extern	volatile unsigned short	SHTTER_CTRL;						/* ���������Ăяo��			*/

extern	ulong	system_ticks;
extern	uchar	MAC_address[6];

// �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
// ���ɗp�\����
typedef struct {
	unsigned short count;	// 5sec����
	unsigned short index;	// �Ԏ����ޯ�� 1�`100
}t_AutoPayment_In;

// ���Z�p�\����
typedef struct {
	unsigned char  mode;	// ����Ӱ�ށi0�F���Z�J�n�^1�F�Ԏ����͒��^2�F�������j
	unsigned short proc_no;	// ���Z���̎Ԏ����ޯ��
	unsigned short next_no;	// ���񌟍��J�n���ޯ��
}t_AutoPayment_Out;

// �������Z�p�\����
typedef struct {
	t_AutoPayment_In	In;
	t_AutoPayment_Out	Out;
	unsigned short AUTOPAY_TRG;
}t_AutoPayment;

extern	t_AutoPayment  AutoPayment;

#endif

#define	SKIP_APASS_CHK	(ryo_buf.apass_off == 1) || (PPrmSS[S_P01][3] == 1)		// �ǂ�����OFF
#define	DO_APASS_CHK	(ryo_buf.apass_off == 0) && (PPrmSS[S_P01][3] == 0)		// �ǂ�����OFF

extern	volatile	uchar	dummy_Read;
extern uchar	x10ms_count;								/* ���� 20ms����			*/
extern uchar	x100ms_count;								/* ���� 100ms����			*/
extern ulong	Now_t;										/* ���ݎ���(ms�P��)			*/
extern uchar fan_exe_state;	// �T�[�~�X�^�̉��x�ɂ��FAN�̃|�[�g����̒l,���ԑтŋ�����~���邽�ߕ��A�����邽�߂̑ޔ��G���A
							// 0:FAN����OFF 1:FAN����ON
extern uchar f_fan_timectrl;// �����ɂ�鋭����~���������t���O 0:�ʏ� 1:������~��
extern uchar f_SoundIns_OK;	// �����C���X�g�[�����[�hRSW=5�̌��ʃt���O 1:���� 0:���s

extern const unsigned char prog_update_flag[4];
extern const unsigned char wave_update_flag[4];
extern const unsigned char parm_update_flag[4];
typedef struct{
	unsigned short wave;// �����f�[�^
	unsigned short parm;// ���ʃp�����[�^
	unsigned short reserve1;
	unsigned short reserve2;
}SW;

// �ʐ؂�ւ��Z�N�V�������
typedef struct{
	unsigned char f_prog_update[4];			// �v���O�����X�V�t���O
	unsigned char f_wave_update[4];			// �����f�[�^�X�V�t���O
	unsigned char f_parm_update[4];			// ���ʃp�����[�^�X�V�t���O
	SW sw;									// �^�p�ʏ��
	SW sw_bakup;							// �^�p�ʏ��o�b�N�A�b�v(�X�V���̃��J�o���p)
	SW sw_flash;							// �t���b�V���ɏ����ꂽ�^�p�ʏ��
	unsigned short err_count;				// �u�[�g�v���O�����Ŕ��������G���[�̉�(�g�[�^��10���������畜���s�\�Ƃ���)
}SWITCH_DATA;

extern SWITCH_DATA	BootInfo;						// �N�����ʑI�����
extern unsigned char f_wave_datchk;					// �����f�[�^�����邩�Ȃ����������t���Owave_data_swchk()�̖߂�l���i�[����
extern const unsigned char LOG_VERSION;				// ���O�o�[�W����1 ����ȑO�̓o�[�W��������
extern const unsigned char LOG_PASS_ROM[8];			// ���O�G���A�̃p�X���[�hROM�l
extern unsigned char LOG_PASS_RAM[8];				// ���O�G���A�̃p�X���[�hRAM�l
extern uchar bk_log_ver;							// �o�b�N�A�b�v���̃��O�o�[�W����
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
extern uchar LONG_PARK_COUNT;						// �������Ԍ��o�p�^�C�}�J�E���^(�A���[���p)
extern uchar LONG_PARK_COUNT2;						// �������Ԍ��o�p�^�C�}�J�E���^(�h�A�A���[���p)
extern uchar LONG_PARK_COUNT3;						// �������Ԍ��o�p�^�C�}�J�E���^(�������ԃ��O�p)
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH810100(S) K.Onodera 2020/01/17 �Ԕԃ`�P�b�g���X(LCD���Z�b�g�ʒm�Ή�)
extern uchar SystemResetFlg;						// �V�X�e�����Z�b�g���s�t���O
// MH810100(E) K.Onodera 2020/01/17 �Ԕԃ`�P�b�g���X(LCD���Z�b�g�ʒm�Ή�)

// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
// ParkingWeb/RealTime���Z���~�f�[�^���M����
#define	PAY_CAN_DATA_SEND	0	// 0:���Z���~�f�[�^���M���Ȃ�/1:����
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
// MH810102(S) �d�q�}�l�[�Ή� #5329 ���Z�@�̓d����OFF/ON�������AE2615-265����������
extern uchar		WAKEUP_COUNT;								// �N���^�C�}�J�E���^
extern uchar		WAKEUP_OVER;								// �N���ς݃t���O
// MH810102(E) �d�q�}�l�[�Ή� #5329 ���Z�@�̓d����OFF/ON�������AE2615-265����������

/*[]----------------------------------------------------------------------[]*/
/*| function prototype                                                     |*/
/*[]----------------------------------------------------------------------[]*/

/* idletask.c */
extern	void	idletask( void );
extern	void	tim500_mon( void );
extern	void	Tcb_Init( void );
extern	void	Mcb_Init( void );
extern	MsgBuf	*GetBuf( void );
extern	void	PutMsg( unsigned char, MsgBuf* );
extern	MsgBuf	*GetMsg( unsigned char );
extern	void	FreeBuf( MsgBuf* );
extern	void	GetMsgWait( unsigned char , MsgBuf * );
MsgBuf	*Target_MsgGet( uchar id, t_TARGET_MSGGET_PRM *pReq );
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(���ʉ��PNo.1362/���Z���~�Ɠ����ɓd�q�}�l�[�^�b�`�ŁA�J�[�h�����肷�邪���Z���~���Ă��܂��s��΍�)
extern const MsgBuf *Target_MsgGet_delete1( const unsigned char task_id, const unsigned short message_id );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(���ʉ��PNo.1362/���Z���~�Ɠ����ɓd�q�}�l�[�^�b�`�ŁA�J�[�h�����肷�邪���Z���~���Ă��܂��s��΍�)
extern	void	xPause( unsigned long );
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
extern	void	xPause1ms( unsigned long );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
extern	void	xPause2ms( unsigned long );
extern	void	xPause_PRNTSK( unsigned long );
extern	MsgBuf	*Target_MsgGet_Range( uchar, t_TARGET_MSGGET_PRM * );

/* taskchg.src */
extern	void	taskchg( unsigned short );

/* comctl.src */
extern	void	_di( void );
extern	void	_ei( void );
extern	unsigned char _di2( void );
extern	void	_ei2( unsigned long );
extern	void	wait2us( unsigned long );
extern	void	wait1us( void );

/* comfnc.c */
extern	short	queset( unsigned char, unsigned short, char, void * );
extern	void	inc_dct( short, short );
extern	unsigned char	bcdbin( unsigned char );
extern	short	bcdbin2( unsigned char * );
extern	short	bcdbin3( unsigned char * );
extern	unsigned char	binbcd( unsigned char );
extern	unsigned char	bcccal	( char *, short );
extern	unsigned char	sumcal( char *, short );
extern	void	cnvdec2	( char *, short );
extern	void	cnvdec4	( char *, long );
extern	unsigned short	astoin	( unsigned char *, short );
extern	unsigned short	astotm	( unsigned char * );
extern	unsigned long	astoinl ( unsigned char *, short );
extern	unsigned char	astohx	( unsigned char * );
extern	unsigned short	astohx2 ( unsigned char * );
extern	void	intoas	( unsigned char *, unsigned short, unsigned short );
extern	void	intoasl ( unsigned char *, unsigned long, unsigned short );
extern	unsigned char	intoasl_0sup( unsigned char *buf, unsigned long data, unsigned short size );
extern	char	hexcnv1 ( char );
extern	void	hxtoasc ( unsigned char *, unsigned char );
extern	void	hxtoas	( unsigned char *, unsigned char );
extern	void	hxtoas2	( unsigned char *, unsigned short );
extern	ushort	binlen( ulong );
extern	void	tmtoas	( unsigned char *, short );
extern	void	nmisave	( void *, void *, short );
extern	void	nmicler	( void *, short );
extern	void	nmimset	( void *, unsigned char, short );
extern	void	nmitrap	( void );
extern	unsigned short	dnrmlzm ( short, short, short );
extern	void	idnrmlzm( unsigned short, short *, short *, short * );
extern	unsigned long	enc_nmlz_mdhm (unsigned short, unsigned char, unsigned char, unsigned char, unsigned char);
extern	void	dec_nmlz_mdhm (unsigned long, unsigned short*, unsigned char*, unsigned char*, unsigned char*, unsigned char*);
extern	short	medget ( short, short );
extern	short	tnrmlz ( short, short, short, short );
extern	void	itnrmlz( short, short, short, short *, short * );
extern	unsigned char	majorty( unsigned char *, unsigned char );
extern	unsigned char	majomajo( unsigned char *, unsigned char, unsigned char );
extern	void	ClkrecUpdate( unsigned char * );
extern	short	chkdate( short, short, short );
extern	short	youbiget( short, short, short );
extern	void	crc_ccitt( ushort, uchar *, uchar *, uchar );				/* CRC16�Z�o:Lock,NT-NET,Mifare	*/
// GG120600(S) // Phase9 CRC�`�F�b�N
extern	void	crc_ccitt_update( ushort, uchar *, uchar *, uchar );				/* CRC16�Z�o:Lock,NT-NET,Mifare	*/
// GG120600(E) // Phase9 CRC�`�F�b�N
extern	uchar	HexAsc_to_LongBin_withCheck( uchar *pAsc, uchar Length, ulong *pBin );
extern	uchar	DeciAsc_to_LongBin_withCheck( uchar *pAsc, uchar Length, ulong *pBin );
extern	unsigned short	sumcal_ushort( unsigned short *stad, unsigned short nu );
extern	unsigned char	memcmp_w( const unsigned short *pSrc1, const unsigned short *pSrc2, unsigned long Length );
extern	uchar	DecAsc_to_LongBin_withCheck( uchar *pAsc, uchar Length, ulong *pBin );
extern	ulong	get_bmplen(uchar *bmp);
extern	uchar	DecAsc_to_LongBin_Minus( uchar *pAsc, uchar Length, long *pBin );
extern	unsigned long	c_arraytoint32(unsigned char array[], short size);
extern	void	c_int32toarray(unsigned char array[], unsigned long data, short size);
extern	uchar	CheckCRC(uchar *pFrom, ushort usSize, uchar *pHere);
extern	unsigned long	astohex ( char *, unsigned int, unsigned char );
extern	void	hextoas ( unsigned char *, unsigned long, unsigned int, unsigned long );
extern	uchar	SJIS_Size( uchar *data );																		// �������菈��
extern	void	c_Now_CLK_REC_ms_Read( struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec );
extern	uchar	c_ClkCheck( struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec );
extern	void	c_Normalize_ms( struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec, ushort *pNormDay, ulong *pNormTime );
extern	void	c_UnNormalize_ms( ushort *pNormDay, ulong *pNormTime, struct clk_rec *pCLK_REC, ushort *pCLK_REC_msec );
extern	ulong	c_Normalize_sec( struct clk_rec *pCLK_REC );
extern	void	c_UnNormalize_sec( ulong NormSec, struct clk_rec *pCLK_REC );
extern	void	memcpyFlushLeft(uchar *dist, uchar *src, ushort dist_len, ushort src_len);
extern	void	memcpyFlushRight(uchar *dest, uchar *src, ushort dest_len, ushort src_len);
// MH322915(S) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
extern	uchar	Check_date_time_rec( date_time_rec *pTime );
// MH322915(E) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
extern	void	memcpyFlushLeft2(uchar *dist, uchar *src, ushort dist_len, ushort src_len);
extern	void	change_CharInArray(uchar *src, ushort src_len, ushort start, ushort end, uchar before, uchar after);
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
extern	int		datetimecheck(struct clk_rec *clk_data);
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��

/* cpu_init.c */
extern	void	cpu_init( void );
extern	void	port_init( void );
extern	void	port_nmi( void );
extern	void	dog_init( void );
extern	void	dog_reset( void );
extern	void	dog_stop( void );
extern	void	scpu_reset( void );
extern	void	System_reset( void );
extern	void	TPU0_init( void );
extern	void	TPU1_init( void );
extern	void	TPU2_init( void );
extern	void	TPU3_init( void );
extern	void CAN1_init( void );
extern	void I2C1_init( void );
extern	void Sci2Init( void );
extern	void Sci3Init( void );
extern	void Sci4Init( void );
extern	void Sci6Init( void );
extern	void Sci7Init( void );
extern	void Sci9Init( void );
extern	void Sci10Init( void );
extern	void Sci11Init( void );
extern	void CMTU0_CMT0_init( void );
#define		WACDOG			dog_reset()	/* wacdog counter reset			*/

extern	int		ExSRam_clear( void );
/* hard_init.c */


/* Int_RAM.c */


/* Lagtim.c */
extern	void	Lagtim( unsigned char ucId , unsigned char ucNo , unsigned short usTime );
extern	void	Lagcan( unsigned char ucId , unsigned char ucNo );
extern	void	LagChk( void );
extern	unsigned char	LagChkExe( unsigned char ucId , unsigned char ucNo );
extern	void	Lag10msInit( void );
extern	void	Lag20msInit( void );
extern	void	Lag500msInit( void );
extern	void	LagTim10ms( char no, short time, void ( *func )( void ) );
extern	void	LagTim20ms( char no, short time, void ( *func )( void ) );
extern	void	LagTim500ms( char no, short time, void ( *func )( void ) );
extern	void	LagCan10ms( char no );
extern	void	LagCan20ms( char no );
extern	void	LagCan500ms( char no );
extern	void	LagChk10ms( void );
extern	void	LagChk20ms( void );
extern	void	LagChk500ms( void );
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
extern	uchar	LagTim20ms_Is_Counting( char no );
extern	uchar	LagTim500ms_Is_Counting( char no );
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
extern	unsigned long	LifeTimGet( void );
extern	unsigned long	LifePastTimGet( unsigned long StartTime );
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
extern	unsigned long	LifeTim1msGet( void );
extern	unsigned long	LifePastTim1msGet( unsigned long StartTime );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
extern	unsigned long	LifeTim2msGet( void );
extern	unsigned char	LifePastTim2msGet( unsigned long, unsigned long );
extern	unsigned long c_2msPastTimeGet( unsigned long StartTime );
extern	unsigned long c_PastTime_Subtraction( unsigned long Time, unsigned long SubValue );

/* keyctrl.c */
extern	void	(* const KEY_SCAN_Exec[])( void );
extern	void	key_read( void );
extern	void	door_int_read( void );
extern	unsigned short	ex_port_red( void );
extern	void	SCAN_INP1_Exec( void );
extern	void	SCAN_INP2_Exec( void );
extern	void	BUZINT( void );
extern	void	BUZERR( void );
extern	void	BUZPI( void );
extern	void	BUZPIPI( void );
extern	void	BUZPIPIPI( void );
extern	void	PION( void );
extern	void	Ptout_Init( void );
extern	void	PortAlm_Init( void );

extern	void	BCDtoASCII( uchar *bcd, uchar *ascii, ushort len );

extern	uchar	RXF_VERSION[10];
extern	ulong	SodiacSoundAddress;
extern	uchar	SOUND_VERSION[10];
#define		OPE_TIM1MS_MAX		4	/* ���ڰ�������p��ϰ(Base = 1ms)		*/
#define		REMOTEDL_TIM1MS_MAX	4	/* ���u�޳�۰������p��ϰ(Base = 1ms)	*/
extern	void	Lagtim_1ms( unsigned char ucId , unsigned char ucNo , unsigned short usTime );
extern	void	Lagcan_1ms( unsigned char ucId , unsigned char ucNo );
extern	void	LagChk_1ms( void );
extern	unsigned short OPE_Tim_Base1ms[OPE_TIM1MS_MAX];				/* ���ڰ�������p��ϰ(Base = 1ms)		*/
extern	unsigned short REMOTEDL_Tim_Base1ms[REMOTEDL_TIM1MS_MAX];	/* ���u�޳�۰������p��ϰ(Base = 1ms)	*/

/* nonslib.c */
extern	int stricmp(const char *d, const char *s);
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
/* longpark.c */
extern	void LongTermParkingCheck( void );
extern	void LongTermParkingCheck_r10( void );
extern	void LongTermParkingCheck_Resend( void );
extern	void LongTermParkingCheck_Resend_flagset( void );
extern	void LongTermParkingCheck_r10_prmcng( void );
extern	void LongTermParkingCheck_r10_defset( void );
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)


#endif	// _SYSTEM_H_
