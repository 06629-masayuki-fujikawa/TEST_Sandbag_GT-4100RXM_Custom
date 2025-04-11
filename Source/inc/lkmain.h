/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �ׯ��(ۯ����u)���䕔																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      :																							   |*/
/*| Date        : 2005-02-01																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _LK_MAIN_H_
#define _LK_MAIN_H_
																				/*								*/
/*------------------*/															/*								*/
/*	ۯ����u���ݒ�	*/															/*								*/
/*------------------*/															/*								*/
typedef struct {																/*								*/
	uchar		lok_syu;														/* ۯ����u���					*/
	uchar		ryo_syu;														/* �������						*/
	uchar		area;															/* ���(1�`26)					*/
	ulong		posi;															/* ���Ԉʒu��(1�`9999)			*/
	uchar		if_oya;															// �ڑ��^�[�~�i��No
	uchar		lok_no;															// �ڑ��^�[�~�i��No�ɑ΂��郍�b�N���u�A��
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
}t_LockInfo;																	/*								*/
																				/*								*/
extern t_LockInfo	LockInfo[LOCK_MAX];											/* ۯ����u���ð���				*/
extern t_LockInfo	bk_LockInfo[LOCK_MAX];										/* �̏�Ԏ��pۯ����u���ð���	*/
																				/*								*/
																				/*								*/
/*----------------------*/														/*								*/
/*	ۯ����u��ʖ��ݒ�	*/														/*								*/
/*----------------------*/														/*								*/
typedef struct {																/*								*/
	uchar		in_tm;															/* ���Ɏԗ����m��ϰ				*/
	uchar		ot_tm;															/* �o�Ɏԗ����m��ϰ				*/
	uchar		r_cnt;															/* ۯ����u��ײ��				*/
	ushort		r_tim;															/* ۯ����u��ײ�Ԋu				*/
	uchar		open_tm;														/* �J����M���o�͎���			*/
	uchar		clse_tm;														/* ����M���o�͎���			*/
	// ���F�{�\���̂̓o�b�N�A�b�v/���X�g�A�Ώۂł��邽�߁A
	//	   �����o��ǉ�����ꍇ�͕ʓr�������L�q���Ȃ���
	//	   �o�b�N�A�b�v/���X�g�A������I�����܂���B
}t_LockMaker;																	/*								*/
																				/*								*/
extern t_LockMaker	LockMaker[6];												/* ۯ����u���					*/
																				/* [0]�F�p�c���]��				*/
																				/* [1]�F�p�c�o�C�N				*/
																				/* [2]�F�g�v���]�ԁE�o�C�N		*/
																				/* [3]�F�R���Y���]�ԁE�o�C�N	*/
																				/* [4]�FHID���]�ԁE�o�C�N		*/
																				/* [5]�F�\��					*/
																				/* 								*/
/*----------------------*/														/*								*/
/*	�Ԏ����Ұ���̫��	*/														/*								*/
/*----------------------*/														/*								*/
struct	LOCKINFO_REC{															/* �Ԏ����Ұ���̫�Ēl�p�\����	*/
	short		adr;															/*								*/
	t_LockInfo	dat;															/*								*/
};																				/*								*/
																				/*								*/
extern	const struct	LOCKINFO_REC	lockinfo_rec1[];						/* �Ԏ����Ұ���̫�āi���ԁj		*/
extern	const struct	LOCKINFO_REC	lockinfo_rec2[];						/* �Ԏ����Ұ���̫�āi���ցj		*/
extern	const struct	LOCKINFO_REC	lockinfo_rec3[];						/* �Ԏ����Ұ���̫�āi���ցi�����j�j		*/

// �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
extern	const struct	LOCKINFO_REC	lockinfo_recAP1[];						/* �����A�����Z�p�Ԏ����Ұ��i���ԁj	*/
extern	const struct	LOCKINFO_REC	lockinfo_recAP2[];						/* �����A�����Z�p�Ԏ����Ұ��i���ցj	*/
#endif
																				/* 								*/
																				/* 								*/
/*----------------------*/														/*								*/
/*	ۯ�������Ұ���̫��	*/														/*								*/
/*----------------------*/														/*								*/
struct	LOCKMAKER_REC{															/* ۯ�������Ұ���̫�Ēl�p�\����*/
	short		adr;															/*								*/
	t_LockMaker	dat;															/*								*/
};																				/*								*/
																				/*								*/
extern	const struct	LOCKMAKER_REC	lockmaker_rec[];						/* ۯ�������Ұ���̫��			*/
																				/* 								*/
																				/* 								*/
#define	LOCKINFO_REC_MAX1	1													/* �Ԏ����Ұ���̫�Đ��i���ԁj	*/
#define	LOCKINFO_REC_MAX2	1													/* �Ԏ����Ұ���̫�Đ��i���ցj	*/
#define	LOCKMAKER_REC_MAX	6													/* ۯ�������Ұ���̫�Đ�		*/
#define	LOCKINFO_REC_MAX3	20													/* �Ԏ����Ұ���̫�Đ��i�������ցj	*/
																				/*------------------------------*/
#endif	// _LK_MAIN_H_
