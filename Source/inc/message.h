/*[]----------------------------------------------------------------------[]*/
/*| ү���ޒ�`��                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005-01-28                                               |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

/*--------------------------------------------------------------------------*/
/*	�����Ͻ���`															*/
/*--------------------------------------------------------------------------*/
#define		MSGGETLOW(msg)			msg&0xff/* ү���ނ̉��ʂ��擾			*/
#define		MSGGETHIGH(msg)			msg>>8	/* ү���ނ̏�ʂ��擾			*/

/*--------------------------------------------------------------------------*/
/*	����Ĕԍ���`															*/
/*--------------------------------------------------------------------------*/

	/*----------------------------------------------------------------------*/
	/*	Tenkey �ݷ�															*/
	/*----------------------------------------------------------------------*/
#define		KEY_TEN					0x0130	/* Ten Key event				*/
#define		KEY_TEN0				0x0130	/* Ten Key[0] event				*/
#define		KEY_TEN1				0x0131	/* Ten Key[1] event				*/
#define		KEY_TEN2				0x0132	/* Ten Key[2] event				*/
#define		KEY_TEN3				0x0133	/* Ten Key[3] event				*/
#define		KEY_TEN4				0x0134	/* Ten Key[4] event				*/
#define		KEY_TEN5				0x0135	/* Ten Key[5] event				*/
#define		KEY_TEN6				0x0136	/* Ten Key[6] event				*/
#define		KEY_TEN7				0x0137	/* Ten Key[7] event				*/
#define		KEY_TEN8				0x0138	/* Ten Key[8] event				*/
#define		KEY_TEN9				0x0139	/* Ten Key[9] event				*/
#define		KEY_TEN_F1				0x013a	/* Ten Key[F1](���Z) event		*/
#define		KEY_TEN_F2				0x013b	/* Ten Key[F2](�̎���) event	*/
#define		KEY_TEN_F3				0x013c	/* Ten Key[F3](���ԏؖ���) event*/
#define		KEY_TEN_F4				0x013d	/* Ten Key[F4](���) event		*/
#define		KEY_TEN_F5				0x013e	/* Ten Key[F5](�o�^) event		*/
#define		KEY_TEN_CL				0x013f	/* Ten Key[CL](C) event			*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Switch																*/
	/*----------------------------------------------------------------------*/
#define		KEY_RECIEPT				0x0111	/* �̎������� event				*/
#define		KEY_CANCEL				0x0112	/* ���Z���~���� event			*/
#define		KEY_MODECHG				0x0113	/* Mode change key(�ݒ�Ӱ�޽���)*/
#define		KEY_DOOR1				0x0114	/* Door1 key event				*/
#define		KEY_DOOR2				0x0115	/* Door2 key event				*/
#define		KEY_COINSF				0x0116	/* Coin safe event				*/
#define		KEY_NOTESF				0x0117	/* Note safe event				*/
#define		KEY_MENTSW				0x0118	/* Mentenance SW event			*/
#define		KEY_DOORARM				0x0119	/* Door alarm event				*/
#define		KEY_YOBI1				0x011A	/* �\��(��)					*/
#define		KEY_YOBI2				0x011B	/* �\��(��)					*/
#define		KEY_YOBI3				0x011C	/* �\��(��)					*/
#define		KEY_YOBI4				0x011D	/* �\��(��)					*/
#define		KEY_VIBSNS				0x011E	/* �U���ݻ� event				*/
#define		KEY_PAYSTR				0x011F	/* ���Z�J�n���� event			*/
#define		KEY_DIPSW1				0x0120	/* DIPSW1 event					*/
#define		KEY_DIPSW2				0x0121	/* DIPSW2 event					*/
#define		KEY_DIPSW3				0x0122	/* DIPSW3 event					*/
#define		KEY_DIPSW4				0x0123	/* DIPSW4 event					*/
#define		KEY_MANDET				0x0124	/* �l�̌��m�ݻ�����				*/
#define		KEY_CNDRSW				0x0125	/* ���ү��ޱ����				*/
#define		KEY_RESSW1				0x0126	/* �\������1					*/
#define		KEY_RESSW2				0x0127	/* �\������2					*/
#define		KEY_NTFALL				0x0128	/* ����ذ�ް�E�����m����		*/
#define		KEY_DOORNOBU			0x0129	/* �ޱ��޲����					*/
#define		KEY_RTSW1				0x0140	/* RTSW1 event					*/
#define		KEY_RTSW2				0x0141	/* RTSW2 event					*/
#define		KEY_RTSW3				0x0142	/* RTSW3 event					*/
#define		KEY_RTSW4				0x0143	/* RTSW4 event					*/
#define		KEY_FANET				0x0144	/* FAN��~���x���o event		*/
#define		KEY_FANST				0x0145	/* FAN�쓮�J�n���x���o event	*/
#define		KEY_MIN					0x0146	/* RXM-1��\������ event		*/
#define		KEY_RXIIN				0x0147	/* RXI-1��ėp���� event		*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	RXF DIPSW		 													*/
	/*----------------------------------------------------------------------*/
#define		FNT_DIPSW1				0x0150	/* DIPSW1 event					*/
#define		FNT_DIPSW2				0x0151	/* DIPSW2 event					*/
#define		FNT_DIPSW3				0x0152	/* DIPSW3 event					*/
#define		FNT_DIPSW4				0x0153	/* DIPSW4 event					*/
#define		FNT_YOBI1				0x0154	/* �\��(��)					*/
#define		FNT_YOBI2				0x0155	/* �\��(��)					*/
#define		FNT_YOBI3				0x0156	/* �\��(��)					*/
#define		FNT_YOBI4				0x0157	/* �\��(��)					*/

	/*----------------------------------------------------------------------*/
	/*	���Ķ�ķ�															*/
	/*----------------------------------------------------------------------*/
// MH810100(S) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j
//#define		CANCEL_EVENT			0x01f1	// �U�֌��̎Ԃ��o��
// MH810100(E) K.Onodera 2020/03/09 �Ԕԃ`�P�b�g���X�i���Z���~�����ύX�j

	/*----------------------------------------------------------------------*/
	/*	Clock																*/
	/*----------------------------------------------------------------------*/
#define		CLOCK_CHG				0x0201	/* Clock change evet			*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Time Out															*/
	/*----------------------------------------------------------------------*/
#define		TIMEOUT					0x0300	/* Time out 					*/
#define		TIMEOUT1				0x0301	/* Time out No.1				*/
#define		TIMEOUT2				0x0302	/* Time out No.2				*/
#define		TIMEOUT3				0x0303	/* Time out No.3				*/
#define		TIMEOUT4				0x0304	/* Time out No.4				*/
#define		TIMEOUT5				0x0305	/* Time out No.5				*/
#define		TIMEOUT6				0x0306	/* Time out No.6				*/
#define		TIMEOUT7				0x0307	/* Time out No.7				*/
#define		TIMEOUT8				0x0308	/* Time out No.8				*/
#define		TIMEOUT9				0x0309	/* Time out No.9				*/
#define		TIMEOUT10				0x030a	/* Time out No.10				*/
#define		TIMEOUT11				0x030b	/* Time out No.11				*/
#define		TIMEOUT12				0x030c	/* Time out No.12				*/

/* 1:���ڰ��݊֘A				*/
/* 2:ү�,�̎��؎�t,�װ�\���Ď�	*/
/* 3:���Cذ�ް�Ƽ�يĎ�			*/
/* 4:�s��������Ď�,�װ�\���Ď�	*/
/* 5:�h�A�m�u�߂��Y��h�~�^�C�}�[	*/
/* 6:����ݽ,������,������⍇��	*/
/* 7:����ݽ��,������J��Ԍp���Ď�,�ǎ�װ�\���p	*/
/* 8:������߂蔻����ϰ			*/
/* 9:�N���W�b�g(CCT)�֘A�^�C�}�[ (����TimeOut / ��������TimeOut / 'r'TimeOut) */
/* 10:�T�C�N���b�N�\���^�C�}�[ & Suica����ޗv�����wait��ϰ */
/* 11:Suica�c���\����ϰ			*/
/* 12:Mifare�������ݗp			*/
/* 13:ParkiPRO�đ��^�C�}�[		*/
enum {
		TIMEOUT13 =	0x030d,			/* 13:ParkiPRO�đ��^�C�}�[				*/
		TIMEOUT14,					/* Time out No.14				*/
		TIMEOUT15,					/* Time out No.15(���g�p)		*/
		TIMEOUT16,
		TIMEOUT17,
		TIMEOUT18,					// ����������׸ފĎ��p��ϰ�i���̋@�\�Ƃ̕��p�s�j
		TIMEOUT19,				/* 						*/
		TIMEOUT20,				/*	��ү������o�������҂���ϰ	*/
		TIMEOUT21,				/*	���o�͐M��������ϰ			*/
		TIMEOUT22,					/* SODIAC�Đ��v���C���^�[�o���^�C�}�[(CH1)	*/
		TIMEOUT23,					/* SODIAC�Đ��v���C���^�[�o���^�C�}�[(CH2)	*/
		TIMEOUT24,					/* SODIAC FROM�A�N�Z�X���J����ϰ			*/
		TIMEOUT25,					/* FTP�v���O�����_�E�����[�h�p	*/
		TIMEOUT26,					/* �ޑK�s���A�p���s���̃T�C�N���b�N�\���p�^�C�}�[	*/
		TIMEOUT27,					/* �l�̌��m�Z���T�[�}�X�N���ԃ^�C�}		*/
		TIMEOUT28,					/* LCD�Ă��t���h�~�^�C�}		*/
		TIMEOUT29,					/* �h�A�m�u�߂��Y��h�~�A���[���x�����ԃ^�C�}		*/
		TIMEOUT30,					/* �h�A�m�u�߂���G���[�E�A���[�����M�^�C�}�[		*/
		TIMEOUT31,					// �v���O�����؊��O�̖����M�f�[�^���M�Ď��^�C�}
		TIMEOUT32,					// SNTP�����Ď�
		TIMEOUT33,					/*	�x��u�U�[����f�B���C�^�C�}(�h�A�x��)	*/
		TIMEOUT34,					/*	�x��u�U�[����f�B���C�^�C�}(�U���Z���T�[)	*/
		TIMEOUT35,					/*	�x��u�U�[����f�B���C�^�C�}(�����E��)	*/
// MH321800(S) G.So IC�N���W�b�g�Ή�
		EC_CMDWAIT_TIMEOUT,			/* ���[�_����R�}���h���M���̌��ʎ�M�҂��^�C�}�[	*/
		EC_CYCLIC_DISP_TIMEOUT,		/* �T�C�N���b�N�\���^�C�}�[ & ec����ޗv�����wait��ϰ	*/
		EC_RECEPT_SEND_TIMEOUT,		/* ec������M�^�C�}�[	*/
		EC_INQUIRY_WAIT_TIMEOUT,	/* �⍇��(������)���ʑ҂��^�C�}�[*/
		EC_REMOVAL_WAIT_TIMEOUT,	/* �J�[�h�������҂��^�C�}�[*/
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
		EC_BRAND_NEGO_WAIT_TIMEOUT,	// �u�����h�l�S�V�G�[�V�������̉����҂��^�C�}
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
#if (6 == AUTO_PAYMENT_PROGRAM)
		AUTO_PAYMENT_TIMEOUT,	
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
		TIMEOUT_ICCREDIT_STOP,		// IC�ڼޯĒ�~�҂����
		TIMEOUT_MNT_RESTART,		// ����ݽ�J�n�đ����
		TIMEOUT_MNT_RESTOP,			// ����ݽ�I���đ����
		TIMEOUT_REMOTE_PAY_RESP,	// ���u���Z�J�n����OK�҂����
		TIMEOUT_QR_START_RESP,		// QRذ�ފJ�n����OK�҂����
		TIMEOUT_QR_STOP_RESP,		// QRذ�ޒ�~����OK�҂����
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) K.Onodera 2020/02/21 �Ԕԃ`�P�b�g���X( #3858 �N���ҋ@��Ԗ������[�v�h�~)
		TIMEOUT_LCD_STA_WAIT,		// LCD�N���ʒm/�N�������ʒm�҂����
// MH810100(E) K.Onodera 2020/02/21 �Ԕԃ`�P�b�g���X( #3858 �N���ҋ@��Ԗ������[�v�h�~)
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
		EJA_INIT_WAIT_TIMEOUT,		// �d�q�W���[�i���������ݒ�҂��^�C�}
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
		RPT_PRINT_WAIT_TIMEOUT,		// ���V�[�g�󎚊����҂��^�C�}
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
		RPT_INFORES_WAIT_TIMEOUT,	// ���V�[�g���v���^�C�}
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
		TIMEOUT_QR_RECEIPT_DISP,	// QR�R�[�h���s�ē��\���^�C�}�[
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j

//�@<--�Ȍ�^�C�}�[�ǉ��͂�����
		TIMEOUT_MAX
};
#define	OPE_TIM_MAX		(TIMEOUT_MAX-TIMEOUT1)		
#define	TIMEOUT_SNTP	TIMEOUT32
#define	_TIMERNO_SNTP	(TIMEOUT_SNTP-TIMEOUT)
// MH321800(S) G.So IC�N���W�b�g�Ή�
#define TIMERNO_EC_CMDWAIT		(EC_CMDWAIT_TIMEOUT-TIMEOUT)
#define	TIMERNO_EC_CYCLIC_DISP	(EC_CYCLIC_DISP_TIMEOUT-TIMEOUT)
#define	TIMERNO_EC_RECEPT_SEND	(EC_RECEPT_SEND_TIMEOUT-TIMEOUT)
#define	TIMERNO_EC_INQUIRY_WAIT (EC_INQUIRY_WAIT_TIMEOUT-TIMEOUT)
#define TIMERNO_EC_REMOVAL_WAIT (EC_REMOVAL_WAIT_TIMEOUT-TIMEOUT)
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
#define	TIMERNO_EC_BRAND_NEGO_WAIT	(EC_BRAND_NEGO_WAIT_TIMEOUT-TIMEOUT)
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#if (6 == AUTO_PAYMENT_PROGRAM)
#define TIMERNO_AUTO_PAYMENT_TIMEOUT	(AUTO_PAYMENT_TIMEOUT-TIMEOUT)
#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
#define TIMERNO_ICCREDIT_STOP		(TIMEOUT_ICCREDIT_STOP 		- TIMEOUT)	// IC�ڼޯĒ�~�҂����
#define TIMERNO_MNT_RESTART			(TIMEOUT_MNT_RESTART 		- TIMEOUT)	// ����ݽ�J�n�đ����
#define TIMERNO_MNT_RESTOP			(TIMEOUT_MNT_RESTOP 		- TIMEOUT)	// ����ݽ��~�đ����
#define TIMERNO_REMOTE_PAY_RESP		(TIMEOUT_REMOTE_PAY_RESP 	- TIMEOUT)	// ����ʒm(���u���Z�J�n����OK)�҂����
#define TIMERNO_QR_START_RESP		(TIMEOUT_QR_START_RESP 		- TIMEOUT)	// QRذ�ފJ�n����OK�҂����
#define TIMERNO_QR_STOP_RESP		(TIMEOUT_QR_STOP_RESP 		- TIMEOUT)	// QRذ�ޒ�~����OK�҂����
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
// MH810100(S) K.Onodera 2020/02/21 �Ԕԃ`�P�b�g���X(#3858�N���ҋ@��Ԗ������[�v�h�~)
#define TIMERNO_LCD_STA_WAIT		(TIMEOUT_LCD_STA_WAIT 		- TIMEOUT)	// LCD�N���ʒm/�N�������ʒm�҂����
// MH810100(E) K.Onodera 2020/02/21 �Ԕԃ`�P�b�g���X(#3858�N���ҋ@��Ԗ������[�v�h�~)
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
#define	TIMERNO_EJA_INIT_WAIT	(EJA_INIT_WAIT_TIMEOUT-TIMEOUT)
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ芮���҂��^�C�}�ύX�j
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
#define	TIMERNO_RPT_PRINT_WAIT	(RPT_PRINT_WAIT_TIMEOUT-TIMEOUT)			// ڼ�Ĉ󎚊����҂����
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
#define	TIMERNO_RPT_INFORES_WAIT	(RPT_INFORES_WAIT_TIMEOUT-TIMEOUT)		// ڼ�ď��v�����
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
#define	TIMERNO_QR_RECEIPT_DISP		(TIMEOUT_QR_RECEIPT_DISP-TIMEOUT)		// QR�R�[�h���s�ē��\���^�C�}�[
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Flapper Define														*/
	/*----------------------------------------------------------------------*/
#define		LK_DATAREC				0x0400	/* IF�Ղ���ް���M				*/
#define		LK_SND_CLS				0x0411	/* IF�Ղ֐����ް����M�v��(��)	*/
#define		LK_SND_OPN				0x0412	/* IF�Ղ֐����ް����M�v��(�J)	*/
#define		LK_SND_A_STS			0x0413	/* IF�Ղ֐����ް����M�v��(�S��ԗv��)	*/
#define		LK_SND_STS				0x0414	/* IF�Ղ֐����ް����M�v��(��ԗv��)		*/
#define		LK_SND_A_CLS			0x0415	/* IF�Ղ֐����ް����M�v��(�S�ĕ�)		*/
#define		LK_SND_A_OPN			0x0416	/* IF�Ղ֐����ް����M�v��(�S�ĊJ)		*/
#define		LK_SND_MNT				0x0417	/* IF�Ղ֐����ް����M�v��(��Ï��v��)	*/
#define		LK_SND_A_TEST			0x0418	/* IF�Ղ֐����ް����M�v��(�S�J��ý�)	*/
#define		LK_SND_TEST				0x0419	/* IF�Ղ֐����ް����M�v��(�J��ý�)		*/
#define		LK_SND_A_CNT			0x041a	/* IF�Ղ֐����ް����M�v��(�S���춳�ĸر)*/
#define		LK_SND_CNT				0x041b	/* IF�Ղ֐����ް����M�v��(���춳�ĸر)	*/
#define		LK_SND_VER				0x041c	/* IF�Ղ֐����ް����M�v��(�ް�ޮݗv��)	*/
#define		LK_SND_ERR				0x041d	/* IF�Ղ֐����ް����M�v��(�װ��ԗv��)	*/
#define		LK_SND_P_CHK			0x041e	/* IF�Ղ֐����ް����M�v��(����|�[�g����)	*/
#define		LK_SND_A_LOOP_DATA		0x041f	/* ���[�v�f�[�^�v��						*/
#define		FLAP_A_UP_SND_MNT		0x0420	/* �S�ׯ�ߏ㏸�v��(����ݽ)		*/
#define		FLAP_A_DOWN_SND_MNT		0x0421	/* �S�ׯ�߉��~�v��(����ݽ)		*/
#define		LOCK_A_CLS_SND_MNT		0x0422	/* �Sۯ����u�v��(����ݽ)		*/
#define		LOCK_A_OPN_SND_MNT		0x0423	/* �Sۯ����u�J�v��(����ݽ)		*/
#define		FLAP_A_UP_SND_NTNET		0x0424	/* �S�ׯ�ߏ㏸�v��(NTNET)		*/
#define		FLAP_A_DOWN_SND_NTNET	0x0425	/* �S�ׯ�߉��~�v��(NTNET)		*/
#define		LOCK_A_CLS_SND_NTNET	0x0426	/* �Sۯ����u�v��(NTNET)		*/
#define		LOCK_A_OPN_SND_NTNET	0x0427	/* �Sۯ����u�J�v��(NTNET)		*/
#define		INT_FLAP_A_UP_SND_MNT		0x042a	/* �S�����ׯ�ߏ㏸�v��(����ݽ)		*/
#define		INT_FLAP_A_DOWN_SND_MNT		0x042b	/* �S�����ׯ�߉��~�v��(����ݽ)		*/
#define		INT_FLAP_A_UP_SND_NTNET		0x042c	/* �S�����ׯ�ߏ㏸�v��(NTNET)		*/
#define		INT_FLAP_A_DOWN_SND_NTNET	0x042d	/* �S�����ׯ�߉��~�v��(NTNET)		*/
#define		FLAP_UP_SND				0x0430	/* �ׯ�ߏ㏸�w��(����ݽ)		*/
#define		FLAP_DOWN_SND			0x0440	/* �ׯ�߉��~�w��(����ݽ)		*/
#define		FLAP_UP_SND_SS			0x0431	/* �ׯ�ߏ㏸�w��(�C�����Z��)	*/
#define		FLAP_DOWN_SND_SS		0x0441	/* �ׯ�߉��~�w��(�C�����Z��)	*/
#define		FLAP_UP_SND_NTNET		0x0432	/* NT-NET �ׯ�ߏ㏸�w��			*/
#define		FLAP_DOWN_SND_NTNET		0x0442	/* NT-NET �ׯ�߉��~�w��			*/
#define		FLAP_UP_SND_RAU			0x0433	/* RAU �ׯ�ߏ㏸�w��			*/
#define		FLAP_DOWN_SND_RAU		0x0443	/* RAU �ׯ�߉��~�w��			*/
#define		CAR_FLPMOV_SND			0x0450	/* �ׯ�ߔ���Ď���ϰ��ѱ���	*/
#define		CAR_SVSLUG_SND			0x0460	/* ���޽���׸���ѱ���			*/
#define		CAR_INCHK_SND			0x0461	/* ���ɔ�����ѱ���				 */
#define		CAR_PAYCOM_SND			0x0491	/* ���Z����						*/
											/*								*/
#define		FLAP_UP_SND_MAF			0x0492	/* MAF �ׯ�ߏ㏸�w��			*/
#define		FLAP_DOWN_SND_MAF		0x0493	/* MAF �ׯ�߉��~�w��			*/

#define		CAR_FURIKAE_OUT			0x0495	// �U�֌��̎Ԃ��o��

#define		DOWNLOCKOVER_SND		0x0496	/* ���~ۯ���ѱ���				*/
#define		CTRL_PORT_CHK_RECV		0x0497	/* ����|�[�g����������M		*/
#define		LK_SND_FORCE_LOOP_ON	0x5010	/* �t���b�v�������[�vON			*/
#define		LK_SND_FORCE_LOOP_OFF	0x5011	/* �t���b�v�������[�vOFF		*/

enum {
	_MTYPE_ALL = 0,  // �S�w��
	_MTYPE_LOCK,     // �Sۯ����u   = 1
	_MTYPE_FLAP,     // �S�ׯ�ߑ��u = 2
	_MTYPE_INT_FLAP, // �S�����ׯ�ߑ��u = 3
};
	/*----------------------------------------------------------------------*/
	/*	Coin																*/
	/*----------------------------------------------------------------------*/
#define		COIN_EVT				0x0500	/* ���ү������					*/
#define		COIN_IN_EVT				0x0501	/* ��������						*/
#define		COIN_OT_EVT				0x0502	/* ���o������					*/
#define		COIN_IM_EVT				0x0505	/* ��������imax�B���j			*/
#define		COIN_EN_EVT				0x0507	/* ���o�����					*/
#define		COIN_ER_EVT				0x0509	/* ���o���װ����				*/
#define		COIN_CASSET				0x0510	/* ��ݶ��Ē�					*/
#define		COIN_INVENTRY			0x0511	/* ү����݂ɂ�������؏I��		*/
#define		COIN_IH_EVT				0x0521	/* ��������(�ۗL��������)		*/
#define		COIN_ES_TIM				0x0530	/* ������ϰ��ѱ��				*/
#define		COIN_CNT_SND			0x0535	/* �ۗL�������M����				*/
#define		COIN_RJ_EVT				0x0540	/* ���Z���~						*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Printer																*/
	/*----------------------------------------------------------------------*/
#define		PRINTER_EVT				0x0600	/* ������֘Aү����				*/
#define		PRNFINISH_EVT			0x1600	/* ������֘Aү����				*/
	/* ���̑���ү����0x0600�`0x06ff�܂�pri_def.h�Œ�`						*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	NT45EX��0x0701�`0x0751�܂Ŏg�p�̂���0x0701�`0x07ff�͖��g�p�Ƃ���	*/
	/*----------------------------------------------------------------------*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Note																*/
	/*----------------------------------------------------------------------*/
#define		NOTE_EVT				0x0800	/* ����ذ�ް�����				*/
#define		NOTE_IN_EVT				0x0801	/* ��������						*/
#define		NOTE_OT_EVT				0x0802	/* ���o������					*/
#define		NOTE_IM_EVT				0x0803	/* ��������						*/
#define		NOTE_I2_EVT				0x0805	/* �㑱��������					*/
#define		NOTE_EN_EVT				0x0807	/* ���o�����					*/
#define		NOTE_EM_EVT				0x0809	/* �߂��װ����					*/
#define		NOTE_ER_EVT				0x080A	/* ���[�װ����					*/
#define		NOTE_SO_EVT				0x0810	/* ������J						*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	NT45EX��0x0901�`0x0925�܂Ŏg�p�̂���0x0901�`0x09ff�͖��g�p�Ƃ���	*/
	/*----------------------------------------------------------------------*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	Operation Event Message												*/
	/*----------------------------------------------------------------------*/
#define		OPE_EVT					0x0a00	/* �I�y���[�V�����C�x���g		*/
#define		OPE_OPNCLS_EVT			0x0a01	/* �����c�x�� event				*/
#define		OPE_REQ_CALC_FEE		0x0a02	/* �����v�Z�v��					*/
											/*								*/
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
#define		OPE_REQ_REMOTE_CALC_TIME_PRE		0x0a11	// ���u���Z�����v�Z�v��
#define		OPE_REQ_REMOTE_CALC_TIME			0x0a12	// ���u���Z�����w�萸�Z�v��
#define		OPE_REQ_REMOTE_CALC_FEE				0x0a14	// ���u���Z���z�w�萸�Z�v��
#define		OPE_REQ_FURIKAE_TARGET_INFO			0x0a16	// �U�֑Ώۏ��擾�v��
#define		OPE_REQ_FURIKAE_GO					0x0a17	// �U�֎��s�v��
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�)_for_LCD_IN_CAR_INFO_op_mod00))
#define		OPE_REQ_LCD_CALC_IN_TIME			0x0a22	// �Ԕԃ`�P�b�g���X���Z(���Ɏ����w��)�v��
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�)_for_LCD_IN_CAR_INFO_op_mod00))

// MH321800(S) G.So IC�N���W�b�g�Ή�
#define		AUTO_CANCEL				0x0ae2
#define		OPE_EC_ALARM_LOG_PRINT_1_END	0x0ae3
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(S) MH364301 �C���{�C�X�Ή�
#define		OPE_DELAY_RYO_PRINT		0x0ae4
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
#define		OPE_DUMMY_NOTICE		0x0ae5
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j

// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
/* 0x0b00�`0x0bff = �\���i�����p�j											*/
	/*----------------------------------------------------------------------*/
	/*	Arcnet Event Message												*/
	/*----------------------------------------------------------------------*/
											/* Reader						*/
#define		ARC_CR_EVT				0x0c00	/* Reader event					*/
#define		ARC_CR_R_EVT			0x0c14	/* 20:Read Data Receive event	*/
#define		ARC_CR_E_EVT			0x0c15	/* 21:END  Data Receive event	*/
#define		ARC_CR_VER_EVT			0x0c16	/* 22:Vertion   Receive event	*/
#define		ARC_CR_EOT_EVT			0x0c17	/* 23:EOT Status Receive event	*/
#define		ARC_CR_EOT_RCMD			0x0c18	/* 24:EOT Status RCMD Receive event*/
#define		ARC_CR_EOT_RCMD_WT		0x0c19	/* 25:EOT Status RCMD Wait event*/
// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
#define		ARC_CR_EOT_MOVE_TKT		0x0c1e	/* 30:EOT Status Tkt move mouth */
// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
#define		ARC_CR_SND_EVT			0x0c32	/* 50:Command Send Complete event*/
#define		ARC_CR_SER_EVT			0x0c50	/* 80:Command Send Error event	*/
											/* ARC_CR_SER_EVT�͖��g�p		*/
											/*								*/
											/* Announce Machine				*/
#define		ARC_AVM_EVT				0x0d00  /* Announce event				*/
#define		ARC_AVM_EXEC_EVT		0x0d01  /* 01:������Բ����				*/
#define		ARC_AVM_VER_EVT			0x0d20	/* 32:�ް�ޮ��ް������			*/
#define		ARC_AVM_SND_EVT			0x0d3c	/* 60:���M���������				*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	NTNET Event Message													*/
	/*----------------------------------------------------------------------*/
#define		NTLWR_RCV_EVT			0x0e00	/* recv event					*/
#define		NTLWR_SNDFIN_EVT		0x0e01	/* send finish event			*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	0x0f00�`0x0fff = �\��(�����p)										*/
	/*	0x1000�`0x12ff = NT45EX�ׯ�ߊg���Ŏg�p								*/
	/*	0x1300�`0x15ff = �\��(�����p)										*/
	/*	0x1600�`0x16ff = ������Ŏg�p										*/
	/*	0x1700�`0x1fff = �\��(�����p)										*/
	/*----------------------------------------------------------------------*/
#define		NTDOPALWR_RCV_EVT		0x0f00	/* recv event					*/
#define		NTDOPALWR_SNDFIN_EVT	0x0f01	/* send finish event			*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/*	IBK�֘A(0x2000�`0x2fff)												*/
	/*----------------------------------------------------------------------*/
#define		IBK_EVT					0x2000  /* IBK event					*/
#define		IBK_MIF_EVT_ERR			0x2001	/* MIF:���M���ʴװ				*/
											/*								*/
// A1����ށi���޾ݼݸ�)						/*								*/
#define		IBK_MIF_A1_OK_EVT		0x2002	/* MIF:���޼ر�ID�擾����		*/
#define		IBK_MIF_A1_NG_EVT		0x2003	/* MIF:���޼رٓǏo��NG			*/
											/*								*/
// A2����� (���޾ݼݸ�+�Ǐo��)				/*								*/
#define		IBK_MIF_A2_OK_EVT		0x2004	/* MIF:�����ް��Ǐo������		*/
#define		IBK_MIF_A2_NG_EVT		0x2005	/* MIF:���޼رٓǏo��NG			*/
											/*								*/
// A3����� (���޾ݼݸ�+�Ǐo��)				/*								*/
#define		IBK_MIF_A3_OK_EVT		0x2006	/* MIF:�����ް��Ǐo������		*/
#define		IBK_MIF_A3_NG_EVT		0x2007	/* MIF:���޼رٓǏo��NG			*/
											/*								*/
// A4����� (���޾ݼݸ�+�Ǐo��)				/*								*/
#define		IBK_MIF_A4_OK_EVT		0x2008	/* MIF:�����ް��Ǐo������		*/
#define		IBK_MIF_A4_NG_EVT		0x2009	/* MIF:���޼رٓǏo��NG			*/
											/*								*/
// A5����� (���޾ݼݸ�+�Ǐo��)				/*								*/
#define		IBK_MIF_A5_OK_EVT		0x200a	/* MIF:�����ް��Ǐo������		*/
#define		IBK_MIF_A5_NG_EVT		0x200b	/* MIF:���޼رٓǏo��NG			*/
											/*								*/
// A6����� (���޾ݼݸ�+�Ǐo��)				/*								*/
#define		IBK_MIF_A6_OK_EVT		0x200c	/* MIF:�����ް��Ǐo������		*/
#define		IBK_MIF_A6_NG_EVT		0x200d	/* MIF:���޼رٓǏo��NG			*/
											/*								*/
// A7����� (���޾ݼݸ�+�Ǐo��)				/*								*/
#define		IBK_MIF_A7_OK_EVT		0x200e	/* MIF:�����ް��Ǐo������		*/
#define		IBK_MIF_A7_NG_EVT		0x200f	/* MIF:���޼رٓǏo��NG			*/
											/*								*/
// AE����� (���޾ݼݸ�+�Ǐo��)				/*								*/
#define		IBK_MIF_AE_OK_EVT		0x2010	/* MIF:�����ް��Ǐo������		*/
#define		IBK_MIF_AE_NG_EVT		0x2011	/* MIF:���޼رٓǏo��NG			*/
											/*								*/
// AF����� (���޾ݼݸ�+�Ǐo��)				/*								*/
#define		IBK_MIF_AF_OK_EVT		0x2012	/* MIF:�����ް��Ǐo������		*/
#define		IBK_MIF_AF_NG_EVT		0x2013	/* MIF:���޼رٓǏo��NG			*/
											/*								*/
#define		IBK_NTNET_LOCKMULTI		0x2020	/* NTNET ������Z�p�ް���M		*/
#define		IBK_NTNET_CHKPASS		0x2021	/* NTNET ����⍇�������ް���M	*/

											/*								*/
#define		IBK_NTNET_ERR_REC		0x20a1	/* NTNET �װ�ް���M			*/
#define		IBK_NTNET_FREE_REC		0x20a2	/* NTNET FREE�߹���ް���M		*/
#define		IBK_NTNET_DAT_REC		0x20a5	/* NTNET�ް���M				*/
#define		IBK_NTNET_BUFSTATE_CHG	0x20aa	/* NTNET�ޯ̧��ԕω�			*/
											/*								*/
#define		IBK_LPR_SRLT_REC		0x20b0	/* ���������:���M�����ް���M	*/
#define		IBK_LPR_ERR_REC			0x20b1	/* ���������:�װ�ް���M		*/
#define		IBK_LPR_B1_REC			0x20b2	/* ���������:�󎚏I���ʒm��M	*/
#define		IBK_LPR_B1_REC_OK		0x20b3	/* ���������:�󎚐���I��		*/
#define		IBK_LPR_B1_REC_NG		0x20b4	/* ���������:�󎚈ُ�I��		*/

#define		IBK_COUNT_UPDATE	0x20c0		/* IBK �e�[�u���������ʎ�M	*/
#define		IBK_CTRL_OK			0x20c1		/* IBK �w�����MOK			*/
#define		IBK_CTRL_NG			0x20c2		/* IBK �w�����MNG			*/
#define		IBK_COMCHK_END		0x20c3		/* IBK �ʐM�`�F�b�N�I��		*/

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		IBK_EDY_RCV				0x20d0	/* EDY ��M�����			*/
//#define		IBK_EDY_ERR_REC			0x20d1	/* EDY ���M�װ�����			*/
//#define		IBK_EDY_RECV_TIMEOUT	0x20d2	/* EDY ������M�^�C���A�E�g	*/
//#define		IBK_EDY_SEND_ERR		0x20d3	/* EDY ���M�ُ�ʒm	*/
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

#define		IFM_SEND_OK				0x20e0
#define		IFM_SEND_NG				0x20e1
#define		IFM_SEND_FAIL			0x20e2
#define		IFM_RECV				0x20e8
#define		IFM_RECV_VER			0x20e9

	/*----------------------------------------------------------------------*/
	/*	Other	(0x2100�`0x21ff)											*/
	/*----------------------------------------------------------------------*/
#define		MID_OTHER_EVT			0x2100	/*								*/
#define		MID_STSCHG_CHGEND		0x2100	/* �ޑK�L���ω� (data[0] 0=�ޖ�������,1=����		*/
#define		MID_STSCHG_R_PNEND		0x2101	/* ڼ����������؂�ω� (data[0] 0=�؂ꕜ��,1=����	*/
#define		MID_STSCHG_J_PNEND		0x2102	/* �ެ�����������؂�ω� (data[0] 0=�؂ꕜ��,1=����	*/
#define		MID_STSCHG_ANY_ERROR	0x2103	/* �װ �L���ɕω�����			*/
#define		MID_STSCHG_ANY_ALARM	0x2104	/* �װїL���ɕω�����			*/
#define		MID_RECI_SW_TIMOUT		0x2105	/* �̎��ؔ��s�\��ϰ��ѱ�Ēʒm	*/
#define		SYU_SEI_SEISAN			0x2106	// �C�����Z�v��
#define		DUMMY_KEY_EVENT			0x2107	// ���Z������ʂ���̃e���L�[���̓C�x���g
#define		KEY_DOOR1_DUMMY			0x2108	// Edy�Z���^�[�ʐM���̃h�A�C�x���g�i�_�~�[�p�j
											/*								*/
	/*----------------------------------------------------------------------*/
	/* �Z���^�[�N���W�b�g�֘A (0x2200�`0x22ff)								*/
	/*----------------------------------------------------------------------*/
#define		CRE_EVT					0x2200		/* Bit15 - - Bit8 */
#define		CRE_EVT_RECV			0x2220		/* CRE�f�[�^��M�ʒm */
#define		CRE_EVT_02_OK			0x2221		/* CRE�f�[�^��� 02 = �J�ǃR�}���h���� */
#define		CRE_EVT_04_OK			0x2222		/* CRE�f�[�^��� 04 = �^�M�⍇�����ʃf�[�^ */
#define		CRE_EVT_06_OK			0x2223		/* CRE�f�[�^��� 06 = ����˗����ʃf�[�^ */
#define		CRE_EVT_08_OK			0x2224		/* CRE�f�[�^��� 08 = ��ײݴ�ĉ��� */
#define		CRE_EVT_0A_OK			0x2225		/* CRE�f�[�^��� 0A = �ԕi�⍇�����ʃf�[�^ */
#define		CRE_EVT_02_NG			0x2226		/* CRE�f�[�^��� 02 = �J�ǃR�}���h���� */
#define		CRE_EVT_04_NG1			0x2227		/* CRE�f�[�^��� 04 = �^�M�⍇�����ʃf�[�^ */
#define		CRE_EVT_04_NG2			0x2228		/* CRE�f�[�^��� 04 = �^�M�⍇�����ʃf�[�^ */
#define		CRE_EVT_06_NG			0x2229		/* CRE�f�[�^��� 06 = ����˗����ʃf�[�^ */
#define		CRE_EVT_08_NG			0x222A		/* CRE�f�[�^��� 08 = ��ײݴ�ĉ��� */
#define		CRE_EVT_0A_NG			0x222B		/* CRE�f�[�^��� 0A = �ԕi�⍇�����ʃf�[�^ */
#define		CRE_EVT_SNDERR			0x222C		/* �N���W�b�g�T�[�o�ւ̃f�[�^���M���s�ʒm */
#define		CRE_EVT_CONN_PPP		0x222D		/* PPP�ڑ����� */
#define		CRE_EVT_06_TOUT			0x222E		/* CRE�f�[�^��� 06 = ����˗����ʃf�[�^(��ѱ��) */
#define		MAF_CAPPI_EVT_COMTEST_RCV	0x2251	/* �܂�Ԃ��e�X�g �o�[�W�����ʒm */
#define		MAF_CAPPI_EVT_SEVCOM_RCV	0x2252	/* Cappi�T�[�o�[�a�ʊm�F������M(B1��M) */
#define		I2CDEV2_1_EVT_I2CSNDCMP_RCV	0x2253		/* I2cDev2_1 I2C���M�����ʒm��M */

	/*----------------------------------------------------------------------*/
	/*	0x2300�`0xffff = �\��(�����p)										*/
	/*----------------------------------------------------------------------*/
	/*----------------------------------------------------------------------*/
	/* Suica�֘A (0x2300�`0x2310)											*/
	/*----------------------------------------------------------------------*/
#define		SUICA_EVT				0x2300	/* SUICA�p�����	*/
#define		ELE_EVENT_CANSEL		0x2301	/* �d�q���Ϸ�ݾي�������āi���Z���~�p�j	*/
#define		ELE_EVT_STOP			0x2302	/* �d�q���ϒ�~�p�����	*/
#define		SUICA_INITIAL_END		0x2303	/* Suica����������	*/
#define		SUICA_ENABLE_EVT		0x2304	/* SUICA�X�e�[�^�X�ω��p�i��t�j�����	*/
#define		SUICA_DISABLE_EVT		0x2305	/* SUICA�X�e�[�^�X�ω��p�i��t�s�j�����	*/

#define		SUICA_PAY_DATA_ERR		0x2307	/* SUICA���σf�[�^�ُ�ʒm	*/
											/*								*/
	/*----------------------------------------------------------------------*/
	/* ParkiPRO�I�y���[�V�����֘A (0x24xx)									*/
	/*----------------------------------------------------------------------*/
/* for communication test */
#define		IFMCHK_OK_EVT			0x2400
#define		IFMCHK_NG_EVT			0x2401
/* for payment */
#define		IFMPAY_GENGAKU			0x2410		/* ���z���Z��t�� */
#define		IFMPAY_FURIKAE			0x2411		/* �U�֐��Z��t�� */


	/*----------------------------------------------------------------------*/
	/*	MAF�֘A(0x2500�`0x25ff)												*/
	/*----------------------------------------------------------------------*/

	/*----------------------------------------------------------------------*/
	/*	���u�_�E�����[�h�֘A(0x2600�`)												*/
	/*----------------------------------------------------------------------*/
#define		REMOTE_DL_EVT			0x2600		/* ���u�_�E�����[�h�C�x���g	*/
#define		REMOTE_DL_REQ			0x2601		/* ���u�_�E�����[�h�v��		*/
#define		REMOTE_CONNECT_EVT		0x2602		/* Rism�ڑ��ؒf/���������ʒm*/
#define		REMOTE_DL_END			0x2603		/* ���u�_�E�����[�h����		*/
#define		REMOTE_RETRY_SND		0x2604		/* ���g���C���M�v��			*/
#define		REMOTE_CONNECTCHK_END	0x2605		/* �ڑ��m�F�I�� */
	/*----------------------------------------------------------------------*/
	/*	CAN�ʐM�֘A(0x2700�`)												*/
	/*----------------------------------------------------------------------*/
enum {
		CAN_EMG_EVT		= 0x2700,	// �G�}�[�W�F���V�[��M
};

	/*----------------------------------------------------------------------*/
	/*	I2C�ʐM�֘A(0x2800�`)												*/
	/*----------------------------------------------------------------------*/
enum {
		I2C_EVT		= 0x2800,					// I2C�C�x���g
		I2C_RP_ERR_EVT,							// I2C�ʐM�G���[�ʒm�iRP�j
		I2C_JP_ERR_EVT,							// I2C�ʐM�G���[�ʒm�iJP�j
};

enum {
		SODIAC_EVT		= 0x2900,				// SODIAC(����)�C�x���g
		SODIAC_NEXT_REQ,						// ���̉����f�[�^�Đ��v��
		SODIAC_PLAY_CMP,						// �����f�[�^�Đ�����
};
enum {
		FTP_EVENT				=	0x2A00,		/* FTP�֘A�C�x���g */
		FTP_LOGIN,								/* FTP���O�C�����ʒʒm�iFTPD��p�j */
		FTP_LOGOUT,								/* FTP���O�A�E�g���ʒʒm�iFTPD��p�j */
		FTP_RECVING,							/* FTP	��M������ */
		FTP_SENDING,							/* FTP	���M������ */
		FTP_NOTFILE_R,							/* FTP	�Ώۃf�[�^���Ȃ�(Read��) */
		FTP_NOTFILE_W,							/* FTP	�Ώۃf�[�^���Ȃ�(Write��) */
		FTP_SERVER_TIMEOUT,						/* FTP�T�[�o�[�^�C���A�E�g�ʒm�iFTPD��p�j */
		FTP_FORCE_LOGOUT,						/* FTP�T�[�o�[�������O�A�E�g�iFTPD��p�j */
		
};

	/*----------------------------------------------------------------------*/
	/*	KASAGO�ʐM�֘A(0x2B00�`)												*/
	/*----------------------------------------------------------------------*/
enum {
		KSG_ANT_EVT		= 0x2B00,				// �A���e�i���x����M�C�x���g
};

// MH321800(S) G.So IC�N���W�b�g�Ή�
	/*----------------------------------------------------------------------*/
	/* ���σ��[�_�֘A (0x2C00�`0x2C10)										*/
	/*----------------------------------------------------------------------*/
enum {
		EC_EVT = 0x2C00,					/* ���σ��[�_�p�����			*/
		EC_BRAND_UPDATE,					/* �u�����h��ԍX�V�����		*/
		EC_VOLUME_UPDATE,					/* ���ʕύX�X�V�����			*/
		EC_MNT_UPDATE,						/* ���[�_�����e�i���X�X�V�����	*/
		EC_EVT_CRE_PAID,					/* �N���W�b�g���ϊ���			*/
		EC_EVT_CRE_PAID_NG,					/* �N���W�b�g���ώ��s			*/
		EC_EVT_CARD_STS_UPDATE,				/* �J�[�h������ԍX�V�ʒm 		*/
		EC_EVT_DEEMED_SETTLEMENT,			/* �݂Ȃ����σg���K�����ʒm 	*/
		EC_EVT_CANCEL_PAY_OFF,				/* ���Z�L�����Z�������ʒm 		*/
		EC_EVT_COMFAIL,						/* �ʐM�s�ǔ����ʒm				*/
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		EC_BRAND_RESULT_CANCELED,			/* �u�����h�I������ �L�����Z��  */
		EC_BRAND_RESULT_UNSELECTED,			/* �u�����h�I������ ���s		*/
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
		EC_EVT_NOT_AVAILABLE_ERR,			// ���σ��[�_�؂藣���G���[����
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
		EC_EVT_DEEMED_SETTLE_FUKUDEN,		// �݂Ȃ����ψ����i���d�j
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
		EC_EVT_ENABLE_NYUKIN,				// �ē����C�x���g
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		EC_EVT_QR_PAID,						/* QR�R�[�h���ϊ���				*/
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
};
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810100(S) Y.Yamauchi 2019/10/10 �Ԕԃ`�P�b�g���X(�����e�i���X)	
	/*----------------------------------------------------------------------*/
	/*	LCD�ʐM�֘A(0x3000�`)												*/
	/*----------------------------------------------------------------------*/
enum{
		LCD_BASE			= 0x3000,		// LCD�֘A
		LCD_QR_REQ,							// 0x3001	QR���[�_����v��
		LCD_ERROR_NOTICE,					// 0x3002	�װ�ʒm
		LCD_OPERATION_NOTICE,				// 0x3003	����ʒm
		LCD_IN_CAR_INFO,					// 0x3004	���ɏ��
		LCD_QR_CONF_CAN_RES,				// 0x3005	QR�m��E����f�[�^����
		LCD_QR_DATA,						// 0x3006	QR�f�[�^
		LCD_ICC_SETTLEMENT_STS,				// 0x3007	���Ϗ���ް�
		LCD_ICC_SETTLEMENT_RSLT,			// 0x3008	���ό��ʏ��
		LCD_MNT_QR_CTRL_RESP,				// 0x3009	QRذ�ސ��䉞��(����ݽ)
		LCD_MNT_QR_READ_RESULT,				// 0x300A	QR�ǎ挋��(����ݽ)
		LCD_MNT_REALTIME_RESULT,			// 0x300B	ر���ђʐM�a�ʌ���(����ݽ)
		LCD_MNT_DCNET_RESULT,				// 0x300C	DC-NET�ʐM�a�ʌ���(����ݽ)
		LCD_COMMUNICATION_CUTTING,			// 0x300D	�ؒf���m(PKTtask��LCD�Ƃ̒ʐM���s�ʎ��ɔ��s�����)
		LCD_DISCONNECT,						// 0x300E	LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING�̋���ү����
		LCD_LCDBM_OPCD_PAY_STP,				// 0x300F	���Z���~����ү����	����ʒm(���Z���~�v��=LCDBM_OPCD_PAY_STP)��M����IC�ڼޯĔ�⍇�����̎��ɔ��s
		LCD_LCDBM_OPCD_STA_NOT,				// 0x3010	���Z���~����ү����	����ʒm(�N���ʒm=LCDBM_OPCD_STA_NOT)��M����IC�ڼޯĔ�⍇�����̎��ɔ��s
		LCD_LCD_DISCONNECT,					// 0x3011	���Z���~����ү����	LCD_DISCONNECT(�ȉ���2��message��M���ɔ��s)��M����IC�ڼޯĔ�⍇�����̎��ɔ��s
											// 								1=LCDBM_OPCD_RESET_NOT(ؾ�Ēʒm) / 2=LCD_COMMUNICATION_CUTTING(�ؒf���m)
// MH810100(S) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
		LCD_CHG_IDLE_DISP,					// 0x3012	�ҋ@��ʒʒm(op_mod02()�ő���ʒm - �ҋ@��ʒʒm��M�ς݂ł��邱�Ƃ̒ʒm�p)
// MH810100(E) K.Onodera 2020/03/26 #4065 �Ԕԃ`�P�b�g���X�i���Z�I����ʑJ�ڌ�A�����ē�����������܂Łu�g�b�v�ɖ߂�v���������Ă�������ʂɑJ�ڂł��Ȃ��s��Ή��j
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		LCD_LANE_DATA_RES,					// 0x3013	���[�����j�^�f�[�^����
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//		���V�KLCD_�n�ǉ�
		LCD_MESSAGE_MAX						// 
};
// MH810100(E) Y.Yamauchi 2019/10/10 �Ԕԃ`�P�b�g���X(�����e�i���X)	

// MH810100(S) K.Onodera 2019/11/01 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j
	/*----------------------------------------------------------------------*/
	/*	PKT�^�X�N�p(0x3100�`)												*/
	/*----------------------------------------------------------------------*/
enum{
		PKT_BASE			= 0x3100,		// PKT�֘A
		PKT_SOC_CON,
		PKT_SOC_CLS,
		PKT_SOC_RST,
		PKT_SOC_SND,
		PKT_SOC_RCV,
		PKT_SND_PRT,
		PKT_RST_PRT,
};
// MH810100(E) K.Onodera 2019/11/01 �Ԕԃ`�P�b�g���X�iLCD�ʐM�j


	/*----------------------------------------------------------------------*/
	/* SNTP		(0xE00x)													*/
	/*----------------------------------------------------------------------*/
enum {
	SNTPMSG_EVT = 0xE000,
	SNTPMSG_RECV_TIME,						/* SNTP response manual			*/
	SNTPMSG_INVALID_RECV_TIME,				/* �s��������M(�蓮)			*/
	SNTPMSG_RECV_AUTORES,					/* SNTP response auto			*/
	SNTPMSG_TIMEOUT,						/* SNTP request timeout			*/
	SNTPMSG_AUTO_SET,						/* clock set					*/
	SNTPMSG_INVALID_RECV_AUTO,				/* �s��������M(����)			*/
};
	/*----------------------------------------------------------------------*/
	/* TELNET	(0xE01x)													*/
	/*----------------------------------------------------------------------*/
	/*----------------------------------------------------------------------*/
	/* FTP		(0xE02x)													*/
	/*----------------------------------------------------------------------*/
enum {
	FTPMSG_CTRL_RECV = 0xE020,
	FTPMSG_CTRL_SEND,
	FTPMSG_FILE_SEND,
	FTPMSG_FILE_RECV,
	FTPMSG_FILE_RECVEND
};
	/*----------------------------------------------------------------------*/
	/* BASE		(0xEF0x)													*/
	/*----------------------------------------------------------------------*/
enum {
	HIF_RECV_VER = 0xEF00
};
enum {
			TIMEOUT1_1MS =	0x3301,
			TIMEOUT2_1MS,
			TIMEOUT3_1MS,
			TIMEOUT4_1MS,
			TIMEOUT_1MS_MAX
};
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
/**
	*	LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING��M���̋���ү����(LCD_DISCONNECT)
 */
typedef	struct {
	unsigned short		MsgId;	// 1=LCDBM_OPCD_RESET_NOT / 2=LCD_COMMUNICATION_CUTTING
} lcdbm_lcd_disconnect_t;		// LCD_DISCONNECT
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))

#endif	// _MESSAGE_H_
