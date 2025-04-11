/*[]----------------------------------------------------------------------[]*/
/*|		�VI/F�Ձi�e�@�A�q�@�j���ʃw�b�_									   |*/
/*|		�E�����ɂ́A�VI/F�Ձi�e�@�Ǝq�@�j�����ʂɎg�p����萔���`���܂��B|*/
/*|		�E�܂��A�VI/F�Ձi�e�@�Ǝq�@�j�����ʂɎg�p����u�d���t�H�[�}�b�g�v��|*/
/*|		  �錾���܂��B													   |*/
/*|																		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef IF_H
#define IF_H


/*----------------------------------*/
/*			�����R���p�C��			*/
/*----------------------------------*/
/*
*	2005.05.11 �H�t������˗�
*	�����Z�@�V�~�����[�^�g�p�ɂ��]������
*	�@���Z�@�ւ̑��M�d���wCRC�x���uffH, ffH�v�Œ�l�ɂ���B		PAYnt_net.c
*	�A���Z�@�ւ̑��M�d���w�f�[�^�ǂ��ԁx���u00H�v�Œ�l�ɂ���B	PAYnt_net.c, PAYcom.c
*/
#define	QA_FOR_SIM	0				/* 1=�V�~�����[�^�g�p����E�E�E�E�]�����{�� */
									/* 0=�V�~�����[�^�g�p���Ȃ��E�E�E�{�� */

/* �q�@�u�w�b�_������蓦�����v���ؗp */
#define	DBG_NO29	0				/* 1=�w�b�_�����͌ŗL�i�q������̃w�b�_�������g�p����j�E�E�E��L���؎� */
									/* 0=�w�b�_�����͐��K�E�E�E�{�� */

/*
*	�f�o�b�O�E���O�o�͂�CRC�`�F�b�N�Ȃ��i�f�o�b�O�p�j
*	�E�\��DIP-SW���g���A�f�o�b�O�E���O�o�͂���/���Ȃ���I���\
*		(�e)DIP-SW2�̂T	ON�Ń��O�o�͂���AOFF�Ń��O�o�͂��Ȃ��B
*		(�q)DIP-SW�̂T	ON�Ń��O�o�͂���AOFF�Ń��O�o�͂��Ȃ��B
*
*	�E�\��DIP-SW���g���ACRC�`�F�b�N����/���Ȃ���I���\
*		(�e)DIP-SW2�̂U	ON��CRC�`�F�b�N�Ȃ��AOFF��CRC�`�F�b�N����B
*		(�q)DIP-SW�̂U	ON��CRC�`�F�b�N�Ȃ��AOFF��CRC�`�F�b�N����B
*/
#define	IF_DEBUG	1				/* 1=��L�f�o�b�O�̂��߁u�\��DIP-SW�v���g�p����B�E�E�E�f�o�b�O�� */
									/* 0=�u�\��DIP-SW�v���g�p���Ȃ��E�E�E�{�� */

/*
*	2005.04.27(��)
*	���x���`�}�[�N�e�X�g
*	�@10msec�C���^�[�o���^�C�}�[�����؂���B
*	�A�q�@�������Ԃ����؂���B
*	�E�ȉ���3�t�@�C���Ɋւ��܂��B
*		1)LCC_10msInt(LCCtime.c)				�E�E�E�@
*		2)toMcomdr_CharSetToRcvArea(toMcomdr.c)	�E�E�E�A
*		3)toMcom_WaitSciSendCmp(toMcom.c)		�E�E�E�B
*	�E�@�ɂ́A�Ԏ����U�̎{���i���b�N����j�M���o�̓|�[�g�iPF4�j���g���̂ŁA���ӂ̂��ƁB
*	�E�A�ɂ́A�Ԏ����U�̊J���i���b�N�����j�M���o�̓|�[�g�iPF5�j���g���̂ŁA���ӂ̂��ƁB
*	�E�{�Ԃ́A=0 �Ƃ��邩�A�V���{�����̂��R�����g�A�E�g���邱�ƁB
*/
#define	DBG_BENCH_MARK_TEST		0	/* 1=�x���`�}�[�N����E�E�E�E�e�X�g���{ */
									/* 0=�x���`�}�[�N���Ȃ��E�E�E�{�� */


/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/
/*
*	�e�@�E�ڑ��䐔�F
*	�E���Z�@�ɂȂ���u�i�e�@�j�ڑ��䐔�v�́A���g�p�B
*	 �iI/F�Ղ�������ׂ����ł͂Ȃ����߁j
*
*	�q�@�E�ڑ��䐔�F
*	�����ڑ��q�@12��{�\���R�䁁15�䕪
*	�E�̈�m�ۂ̂Ƃ��uIF_SLAVE_MAX�v���g�p����B
*	�E��������q�@�́uIF_SLAVE_MAX_USED�v���g�p����B
*
*	���b�N���u�E�ڑ��䐔�F�i�P��̎q�@�ɂȂ���Ԏ����j
*	�����U��{�\���Q�䁁�W�䕪
*	�E�̈�m�ۂ̂Ƃ��uIFS_ROOM_MAX�v���g�p����B
*	�E�e�@�����b�N���uNo,����A���̃��b�N���u�͂ǂ̎q�@���L�Ȃ̂��H����肷��Ƃ��uIFS_ROOM_MAX_USED�v���g�p����B
*	�E�����ݒ�f�[�^�ɂĂ��炤�ׂ��H���l�������A�W��ɑ����Ƃ��ɂ́A�ăR���p�C���{ROM�����������K�v�B
*	�E�q�@��ɂ͂U�䕪�̃|�[�g�����������Ȃ��B�W��ɑ����Ƃ��ɂ́A�n�[�h�������K���B
*
*	���b�N���u�E�ڑ��䐔�F�i�P��̐e�@�ɂȂ���Ԏ����j
*	�������b�N���u�U��~�����ڑ��q�@12�䁁72�䕪
*	�E�̈�m�ۂ̂Ƃ��uIFM_ROOM_MAX�v���g�p����B
*	�E�������郍�b�N���u�́uIFM_ROOM_MAX_USED�v���g�p����B
*/
#define	IF_SLAVE_MAX			31		/* �t���b�v�ECRB�i�q�@�j�ڑ��䐔 */
#define	IF_SLAVE_MAX_USED		9		// �VI/F�Ձi�q�@�j�ڑ��䐔�E�E�E���ۂɎg�p������ UT8500 I/F�Ր���@�\�d�l���ɂ�9��ƋK�肳��Ă���̂�9��Ƃ���
#define	IFS_ROOM_MAX			 8		/* ���b�N���u�ڑ��䐔�i�P��̎q�@�ɂȂ���Ԏ����j*/
#define	IFS_ROOM_MAX_USED		 6		/* ���b�N���u�ڑ��䐔�i�P��̎q�@�ɂȂ���Ԏ����E�E�E���ۂɎg�p�������j*/
#define	IFM_ROOM_MAX		(IFS_ROOM_MAX_USED * IF_SLAVE_MAX)		/*<90>���b�N���u�ڑ��䐔�i�P��̐e�@�ɂȂ���Ԏ����j*/
#define	IFM_ROOM_MAX_USED	(IFS_ROOM_MAX_USED * IF_SLAVE_MAX_USED)	/*<72>���b�N���u�ڑ��䐔�i�P��̐e�@�ɂȂ���Ԏ����j�E�E�E���ۂɎg�p�������j*/
#define	IFS_CRR_MAX			   3	/* CRR��̐ڑ��ő吔�iCRR��Ղ����Z�@�ɂȂ��閇���j*/
#define	IFS_CRRFLAP_MAX		  15	/* CRR��ւ̐ړ_���i�P���CRR�ɂȂ���Ԏ����j*/
#define	IFS_CRR_OFFSET		 100	/* CRR��ւ̃A�N�Z�X�p�I�t�Z�b�g�l */
#define	TOSCOM_INIT_TIME_FIRST		 3 * 100	// �����ݒ著�M�^�C�}�[ �N����1��ڂ�3�b (10ms�P��)
#define	TOSCOM_INIT_TIME			60 * 100	// �����ݒ著�M�^�C�}�[ 60�b���ɑ��M (10ms�P��)

/*�i�e�@�ɂƂ��āj���Z�@�ʐM�����F����M�o�b�t�@�� */
#define	PAY_SCI_SNDBUF_SIZE		(20+5+960+2+ 16)	//20=�w�b�_���A5=�f�[�^���̃w�b�_�A960=(�{�f�B)�f�[�^MAX�A2=CRC�A16=�\��
#define	PAY_SCI_RCVBUF_SIZE		(20+5+204+2+ 16)	//����B204=�����ݒ�f�[�^���i�Œ��j

/*�i�e�@�ɂƂ��āj�q�@�ʐM�����F����M�o�b�t�@�� */
#define	TOS_SCI_SNDBUF_SIZE		(10+250+2+ 16)		//10=�w�b�_���A250=(�{�f�B)�f�[�^MAX�A2=CRC�A16=�\��
#define	TOS_SCI_RCVBUF_SIZE		(10+250+2+ 16)		//����

/*�i�q�@�ɂƂ��āj�e�@�ʐM�����F����M�o�b�t�@�� */
#define	TOM_SCI_SNDBUF_SIZE		(10+250+2+ 16)		//10=�w�b�_���A250=(�{�f�B)�f�[�^MAX�A2=CRC�A16=�\��
#define	TOM_SCI_RCVBUF_SIZE		(10+250+2+ 16)		//����

/* �e�q�ԃV���A���ʐM�|�[�g�ݒ� */
//#define	IF_SCI_SET_BPS		1		/* DIP-SW�F�`�����x(bps)	 0=2400,  1=9600, 2=19200, *3=38400 */
#define	IF_SCI_SET_DBITS		0		/* �Œ�F  �f�[�^��			*0=8bits, 1=7bits 			*/
#define	IF_SCI_SET_SBITS		0		/* �Œ�F  �X�g�b�v�r�b�g	*0=1bit,  1=2bits			*/
#define	IF_SCI_SET_PBIT			0		/* �Œ�F  �p���e�B			*0=none,  1=odd,  2=even 	*/

/************************
*	���b�N���u�ڑ����	*
*************************/
enum LOCK_MAKER {
	LOCK_Nothing = 0,					/*  0�F�ڑ�����															*/
	LOCK_AIDA_bicycle,					/*  1�F�ڑ��L��i������Љp�c�G���W�j�A�����O�����b�N���u�i���]�ԁj�j	*/
	LOCK_AIDA_bike,						/*  2�F�ڑ��L��i������Љp�c�G���W�j�A�����O�����b�N���u�i�o�C�N�j�j	*/
	LOCK_YOSHIMASU_both,				/*  3�F�ڑ��L��i������Ћg�����쏊�����b�N���u�i���]�ԁA�o�C�N���p�j�j	*/
	LOCK_KOMUZU_bicycle,				/*  4�F�ڑ��L��i�R���Y�W���p�������b�N���u�i���]�ԁj�j					*/
	LOCK_HID_both,						/*  5�F�ڑ��L��iHID�����b�N���u�i���]�ԁA�o�C�N���p�j�j				*/

	LOCK_MAKER_END						/*  6�F�G���h�T�C�� */
};

/************************
*	�t���b�v���u�ڑ����	*
*************************/
enum FLAP_MAKER {
	FLAP_Nothing = 0,					// 0�F�ڑ�����
	FLAP_SANICA,						// 1: �T�j�J
	
	FLAP_MAKER_END,
};
/********************
*	�f�t�H���g�l	*
*********************/
//���b�N���u�ɂ���
#define	IF_DEFAULT_LOCK_MAKER	LOCK_Nothing	/*  0�F�ڑ�����	���b�N���u�̐ڑ��i��ԁj*/
#define	IF_DEFAULT_ENTRY_TIME			 4		/*  4�F 4�b		���Ɏԗ����m�^�C�}�[�l */
#define	IF_DEFAULT_EXIT_TIME			 4		/*  4�F 4�b		�o�Ɏԗ����m�^�C�}�[�l */
#define	IF_DEFAULT_RETRY_TIMES			 1		/*  1�F 1��		���b�N���u����ɂă��g���C����� */
#define	IF_DEFAULT_RETRY_INTER			15		/*  1�F15�b		���b�N���u����ɂă��g���C����Ԋu */

//�ΐ��Z�@�ʐM�p�����[�^
#define	IF_DEFAULT_toPAY_RECV_LIMIT		100		/*100�F1000msec	��M�f�[�^�ő厞��				2005.07.25 ����(��500msec) */
#define	IF_DEFAULT_toPAY_ANSWER_LIMIT	30		/* 30�F 300msec	�e�L�X�g���M��̉����҂�����	2005.06.27 ����(��50msec) */
												/*
												*	2005.07.25
												*	��I/F�Ղ́u����Ď��v�́A�e�L�X�g���M�ォ��
												*	  ������M���n�߂ł͖����A������M�����܂ł̎��ԂƂȂ�B
												*	�EIBK���p�̎�M�A���S���Y���̂��߁A
												*	  ��M���荞�݃n���h�����ŁA��M���Ȃ���d����͂���̂ł͂Ȃ�
												*	  ��M������A�d����͂���̂ŁA���̂悤�ȉ��߂�������𓾂Ȃ��B
												*	�E�e�L�X�g���M��̉������uACK/NAK/EOT�v�̏ꍇ�A�d�������Z���̂ŁA���قǖ��ɂȂ�Ȃ���
												*	  �e�L�X�g���M��̉������u�e�L�X�g�ŁA�������ݒ�f�[�^�v�̂悤�Ȓ����d���̏ꍇ�A
												*	  �d����M�����O�Ɂw�����҂����ԁx�^�C���A�E�g���N�����P�[�X������B
												*	 �i�C�j�V��������DIP-SW�N���ŁA���̕s��ƂȂ����E�E�E���̂Ƃ���100msec<19200bps>�������j
												*	�ENT-NET�v���g�R���ł́A��ǂ���̃Z���N�e�B���O�ɑ΂��A
												*	  �]�ǂ��u�f�[�^����v�̂Ƃ��AACK�̑����STX�d���ŉ������A
												*	  ��ǂ͎��Ɂu���肽���f�[�^�v��STX�d���ɂđ����Ă���B
												*	�E�]�ǂ��炷��ƁA�e�L�X�g���M��̉����́AACK�̑����STX�d���ŉ�������鎖�ɂȂ�
												*	  ����STX�d�����u�����ݒ�f�[�^�v�������̂ŁA�w�����҂����ԁx�^�C���A�E�g���N�������B
												*	�E���̂悤�ȕϑ��v���g�R�����̗p�������߁A�\�����ʎ����N���肤��B
												*
												*	�����̂悤�Ȏ���́AI/F�ՌŗL�̖��Ȃ̂ŁA
												*	  �u��M�f�[�^�ő厞�ԁv�{�u�e�L�X�g���M��̉����҂����ԁv���^�C�}�l�Ƃ��č̗p����B
												*	  ���ƂŁA��L�s����������B
												*
												*	���`�����x�ɂ��d����M�����܂ł̎��Ԃ��A�Q�l�Ƃ��Ĉȉ��ɋL���B
												*	  �u�����ݒ�f�[�^�i20+5+204+2=231�j�v�̏ꍇ�A
												*		 2400bps�F4.167(�P��׸��]������)msec �~ 231�޲� �� 962.577 �� 963msec
												*		 9600bps�F1.042(�P��׸��]������)msec �~ 231�޲� �� 240.702 �� 241msec
												*		19200bps�F0.512(�P��׸��]������)msec �~ 231�޲� �� 118.271 �� 119msec
												*		38400bps�F0.260(�P��׸��]������)msec �~ 231�޲� ��  60.060 ��  61msec
												*/
#define	IF_DEFAULT_toPAY_SEND_WAIT		 1		/*  1�F 10msec	���M�f�[�^Wait�^�C�}�[ */
#define	IF_DEFAULT_toPAY_DATA_RETRY		10		/* 10�F 10��	�f�[�^���M�đ��� */
#define	IF_DEFAULT_toPAY_NAK_RETRY		 3		/*  3�F  3��	NAK���M�đ��� */

//�Ύq�@�ʐM�p�����[�^
#define	IF_DEFAULT_38400_LINE_WATCH			 6	/*  6�F 60msec	�ʐM��M(���)�Ď��^�C�}�i38400bps�j*/
#define	IF_DEFAULT_19200_LINE_WATCH			10	/* 10�F 100msec	�ʐM��M(���)�Ď��^�C�}�i19200bps�j*/
#define	IF_DEFAULT_9600_LINE_WATCH			20	/* 20�F 200msec	�ʐM��M(���)�Ď��^�C�}�i 9600bps�j*/
#define	IF_DEFAULT_2400_LINE_WATCH			30	/* 30�F 300msec	�ʐM��M(���)�Ď��^�C�}�i 2400bps�j*/
#define	IF_DEFAULT_toSLAVE_POLLING			 0	/*  0�F  0msec	POL/SEL�Ԋu */
#define	IF_DEFAULT_toSLAVE_RETRY			 3	/*  3�F  3��	�f�[�^�đ��� */
#define	IF_DEFAULT_toSLAVE_SKIP_NO_ANSWER	10	/* 10�F 10��	�������G���[����� */

#define	IF_DEFAULT_toSLAVE_POLLING_ALONE	10	/* 10�F100msec	�ЂƂ������̗�O POL/SEL�Ԋu */

#define	IF_DEFAULT_SLAVE_ANSWER_TIME		10	/* 10�F10msec	�q�@���A�e�@�����ԗv�����󂯁A��������܂ł̎����l */
												/*              �E�q�@�x���`�}�[�N�e�X�g�ɂāA�e�@����̓d����M�����`�����d�����M�����܂ł̎��� */
												/*              �E�Ώۓd���́A�e�@����́u��ԃf�[�^�v���v�ƁA�q�@���́u�q�@��ԃf�[�^�v�Ƃ��� */
//I/F�ՑS�ʂ̃V���A���ʐM
#define	IF_DEFAULT_SEND_WAIT_TIME		2		//  2�F 4msec	���M�E�F�C�g�^�C�} �^�C�}�̍ŏ��P�ʂ�2ms���
												/*	2005.06.14 �����E�ǉ� */
												/*	�e�q�ԃw�b�_������蓦�����Ώ��Ƃ��āu3msec�̑��M�҂��v�Ɍ��� */
												/*	���̍����͈ȉ��B*/
												/*	�E�����ԃ^�C���A�E�g������������A�d���̖����܂Ŏ�M����(��M����)�ƔF������ */
												/*	�uIBK�𓥏P�v�����A���S���Y�����̗p���Ă���֌W�ŁA*/
												/*	�E�����ԃ^�C�}�[�́A1msec�C���^�[�o���^�C�}���g���Ă���A*/
												/*	�E�e�Ƒ��̎q�@�A����Ɏ����̑��M�҂������ꂼ��1msec�̌덷����Ƃ��āA���v��3msec */

/********************************
*	���Z�@�E�VI/F��(�e�@)��		*
*********************************/
/*	�`������R�[�h�iNT-NET�j*/
#define	PAY_SOH				0x01
#define	PAY_STX				0x02
#define	PAY_ETX				0x03
#define	PAY_EOT				0x04
#define	PAY_ENQ				0x05
#define	PAY_NAK				0x15
#define	PAY_ACK				0x06

/* �d���^�C�v */
#define	PAY_POL_BLCK		('P')	/* 'P'olling:  <0x50>�܂Ƃ߁j�|�[�����O�R�[�h/ENQ�d�� */
#define	PAY_POL_QFULL_BLCK	('Q')	/* 'Q'ueue:    <0x51>�܂Ƃ߁j�|�[�����O�R�[�h/ENQ�d���i�L���[FULL�j*/
#define	PAY_SEL_BLCK		('S')	/* 'S'electing:<0x53>�܂Ƃ߁j�Z���N�e�B���O�R�[�h/ENQ�d�� */
#define	PAY_SEL_QFULL_BLCK	('q')	/* 'q'ueue:    <0x71>�܂Ƃ߁j�Z���N�e�B���O�R�[�h/ENQ�d���i�L���[FULL�j*/
#define	PAY_BC_BLCK			('B')	/* 'B'roadcast:<0x42>�܂Ƃ߁j����ENQ�d�� */
#define	PAY_EOT_BLCK		('E')	/* 'E'ot:      <0x45>�܂Ƃ߁jACK�d�� */
#define	PAY_ACK_BLCK		('A')	/* 'A'ck:      <0x41>�܂Ƃ߁jACK�d�� */
#define	PAY_NAK_BLCK		('N')	/* 'N'ak:      <0x4E>�܂Ƃ߁jNAK�d�� */
#define	PAY_TEXT_OK_BLCK	('T')	/* 'T'ure:     <0x54>�܂Ƃ߁j����e�L�X�g */
#define	PAY_TEXT_NG_BLCK	('F')	/* 'F'lase:    <0x46>�܂Ƃ߁j�ُ�e�L�X�g */
#define	PAY_CONTINUE		('C')	/* 'C'ontinue: <0x43>�܂Ƃ߁j�d����M�r���i���m��̂��߉�͕s�\�j*/
#define	PAY_OTHER_BLCK		('O')	/* 'O'ther:    <0x4F>�܂Ƃ߁j���g���Ăł͖��� */
#define	PAY_ILLEAGL_BLCK	('I')	/* 'I'llegal:  <0x49>�܂Ƃ߁j���̑��ُ̈�d�� */

/* �w�b�_������ */
#define	NT_HEADER_STR		"NTCOM"
#define	NT_HEADER_LEN		5

/********************************
*	�VI/F��(�e�q)��				*
*********************************/
/* �`������R�[�h */
#define	IF_ACK				0x06
#define	IF_NAK				0x15

/* �w�b�_������ */
#define	IF_HEADER_STR		"IFCOM"
#define	IF_HEADER_LEN		5

/************************************
*	CRC16-CCITT
*	�E�Z�o�֐��Ăяo�����ɂ́A�u�E�V�t�g�v���w�肷��
*************************************/
#define	CRC16_R_SHIFT		1

/************************************
*	�L���[�̐�
*	�E�|�[�����O�����҂����X�g
*	�E�Z���N�e�B���O�҂����X�g
*	�E�q�@��ԃ��X�g
*	�E���b�N���슮���҂����X�g
*************************************/
#define		PAY_Q_WAIT_ANSWER		(IF_SLAVE_MAX + 1)	/* �|�[�����O�����҂����X�g */
#define		TOS_Q_WAIT_SELECT		(IF_SLAVE_MAX + 1)	/* �Z���N�e�B���O�҂����X�g */
#define		LCM_Q_SLAVE_STATUS		(IF_SLAVE_MAX + 1)	/* �q�@��ԃ��X�g */
#define		LCM_Q_WAIT_ACTION		(IFM_ROOM_MAX + 1)	/* ���b�N���슮���҂����X�g */

/*
*	��̐e���A���b�N���u�̓��슮����҂����Ɏ��̓���v�����q�Ɏw���ł���ő吔
*	�E�ŏ��͂P�ł����쒆�Ȃ�A�҂�����B
*	�E�����͑�����\������B
*/
#define		IF_ACTION_MAX			2

/*	�E���b�N���u����ɔ����A���̓��슮����҂܂ł̐������Ԃ́A*/
/*	  �J/�M���o�͎��ԁ{���ł��邪�A���́u�{���v���ԁi�~���b�j*/
#define		IFS_ANSWER_WAIT_TIME	(10 * IF_SLAVE_MAX) + 100	/* 10msec�P�� */

/*	�E���Z�@���o�[�W�����v������ĉ�������܂ł̐������ԁi�b�j*/
#define		IFS_VERSION_WAIT_TIME	10	// 10�b

/*
*	2005.06.17 �����E�폜
*	���ȉ��̐������Ԃ́A���b�N���u�̓���M���o�͎��ԂɈˑ����邽�߁A
*	  �ꗥ�ɂ͓���ł��Ȃ��B
*	  ����āA�M���o�͎��Ԃ���Z�o���鎖�ɂ��A�萔��p����B
*/
/*	�E���Z�@���S���b�N�J/�v������ĉ�������܂ł̐������ԁi�b�j*/
/*	�E���Z�@���ʃ��b�N�J/�e�X�g�v������ĉ�������܂ł̐������ԁi�b�j*/
/*	�E���Z�@���S���b�N�J/�e�X�g�v������ĉ�������܂ł̐������ԁi�b�j*/

/*
*	�G���[�ڍ׃R�[�h
*
*/
/* �e�̓����܂��́A���Z�@�ʐM�ɑ΂��� */
enum {
	E_IFM_BEGIN,				//�����Ǘ��ԍ��́u1�v����n�߂�
//��M�G���[�i���荞�݌��m�j
	E_PAY_Parity,				//[ 1�F����]�p���e�B
	E_PAY_Frame,				//[ 2�F����]�t���[�~���O
	E_PAY_Overrun,				//[ 3�F����]�I�[�o�[����

//�d���ُ�
	E_PAY_PACKET_Timeover,		//[ 4�F����]��M�������ԉz��
	E_PAY_PACKET_Headerless,	//[ 5�F����]�w�b�_���܂Ŏ�M���Ȃ������i�����Ԃ��󂢂��j
	E_PAY_PACKET_HeadStr,		//[ 6�F����]�w�b�_�����ُ�
	E_PAY_PACKET_DataSize,		//[ 7�F����]�f�[�^�T�C�Y�Ǝ�M�����s��v
	E_PAY_PACKET_WrongCode,		//[ 8�F����]����R�[�h�s��
	E_PAY_PACKET_CRC,			//[ 9�F����]CRC�ُ�
	E_PAY_PACKET_WrongKind,		//[10�F����]�s��(�f�[�^���)�d��
	E_PAY_PACKET_DUPLICATE,		//[11�F����]�u�ǂ��ԁv���O��d���Əd�����Ă���

//�菇�ُ�
	E_NT_NET_Protocol,			//[12�F����]�v���g�R���ُ�
	E_PAY_NAK_Retry,			//[13�F����]NAK(���M)���g���C�I�[�o�[
	E_PAY_NO_Answer,			//[14�F����]���M���g���C�I�[�o�[(������)

//�������s��
	E_PAY_QFULL_PollingWait,	//<15�F��>�L���[�t���i���Z�@�|�[�����O�����҂��j
	E_IFM_QFULL_SlaveStatus,	//<16�F��>�L���[�t���i�q�@��ԃ��X�g�j
	E_IFM_QFULL_LockAction,		//<17�F��>�L���[�t���i���b�N����҂����X�g�j
	E_IFS_QFULL_SelectingWait,	//<18�F��>�L���[�t���i�q�@�Z���N�e�B���O�҂��j

//�d���f�[�^�s��
	E_IFM_INIT_Data,			//[19�F����]�����ݒ�f�[�^�ُ�
	E_IFM_CTRL_Command,			//[20�F����]����f�[�^�ُ�

//�|�j��
	E_IFM_TEST_Next,			//[21�F����]����w���ُ�

//�H
	E_IFM_MODE_Change,			//[22�F����]���[�h�ؑֈُ�
	E_IFM_Password,				//[23�F����]�p�X���[�h�j��

//�v���O�����s��
	E_IFM_BUG,					//[24�F����]�o�O�����H

//�G���h�T�C��
	E_IFM_END
};

/* �q�ɑ΂��� */
enum {
	E_IFS_BEGIN,				//�����Ǘ��ԍ��́u1�v����n�߂�
//��M�G���[�i���荞�݌��m�j
	E_IFS_Parity,				//[ 1�F����]�p���e�B
	E_IFS_Frame,				//[ 2�F����]�t���[�~���O
	E_IFS_Overrun,				//[ 3�F����]�I�[�o�[����

//�d���ُ�
	E_IFS_PACKET_Headerless,	//[ 4�F����]�w�b�_���܂Ŏ�M���Ȃ������i�����Ԃ��󂢂��j
	E_IFS_PACKET_HeadStr,		//[ 5�F����]�w�b�_�����ُ�
	E_IFS_PACKET_WrongSlave,	//[ 6�F����]����Ⴂ�i���K�̉�������ł͖����j
	E_IFS_PACKET_DataSize,		//[ 7�F����]�f�[�^�T�C�Y�Ǝ�M�����s��v
	E_IFS_PACKET_CRC,			//[ 8�F����]CRC�s��v
	E_IFS_PACKET_WrongKind,		//[ 9�F����]�s��(�f�[�^���)�d��

//�菇�ُ�
	E_IF_Protocol,				//[10�F����]�v���g�R���ُ�
	E_IFS_NAK_Retry,			//[11�F����]���M���g���C�I�[�o�[(NAK)
	E_IFS_NO_Answer,			//<12�F��>���M���g���C�I�[�o�[(������)

//�v���O�����s��
	E_IFS_BUG,					//[13�F����]�o�O�����H

//�G���h�T�C��
	E_IFS_END
};

/* �G���[��� */
enum {
	E_IF_RECOVER,	//=0�F�G���[����
	E_IF_OCCUR,		//=1�F�G���[����
	E_IF_BOTH		//=2�F���������E����
};

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/
#pragma pack
/*
*	�ʐM�d���Ȃ̂ŁA�\���̃����o�̋��E���������u1�v�Ƃ���B
*	�E�S�ăo�C�g��łȂ��Ă͂Ȃ�Ȃ��B
*	�E2�o�C�g�����A4�o�C�g�����A�����Ȃǂ��܂܂Ȃ��l�ɒ��ӂ��邱�ƁB
*/

/* (65H)�G���[��ԃf�[�^�́u�e�@�v�G���[��ԃr�b�g�}�b�v */
typedef struct {
	ushort	bfReserved            :12;	/* bit4�`15�F�\�� */
	ushort	bfQFull_SelectingWait :1;	/* bit3�F�L���[�t���i�q�@�Z���N�e�B���O�҂��j*/
	ushort	bfQFull_LockAction    :1;	/* bit2�F�L���[�t���i���b�N����҂����X�g�j*/
	ushort	bfQFull_SlaveStatus   :1;	/* bit1�F�L���[�t���i�q�@��ԃ��X�g�j*/
	ushort	bfQFull_PollingWait   :1;	/* bit0�F�L���[�t���i���Z�@�|�[�����O�����҂��j*/
} t_IFM_bfError;

/* (65H)�G���[��ԃf�[�^�́u�q�@�v�G���[��ԃr�b�g�}�b�v */
typedef struct {
	ushort	bfReserved            :15;	/* bit1�`15�F�\�� */
	ushort	bfNO_Answer           :1;	/* bit0�F���M���g���C�I�[�o�[(������) */
} t_IFS_bfError;

/************************************
*	���Z�@�E�e�@��					*
*	�ʐM�d���t�H�[�}�b�g			*
*************************************/






/**************************************************************
*	NT-NET�ʐM����
*	��{�e�L�X�g�d���t�H�[�}�b�g
*	�E�e�L�X�g�{�f�B���Ƃ́H
*		���j�f�[�^���́w�f�[�^�{�́x�̂��ƁB
*			�f�[�^���́u�e�L�X�g�w�b�_���v��
*					  �u�e�L�X�g�{�f�B���v�ƂȂ琬��B�e�L�X�g�{�f�B����CRC(2�o�C�g)���܂ށB
*
*	�E�f�[�^�T�C�Y���w�b�_��(20)�{�f�[�^��(9�`967)�|CRC��(2)�E�E�E�iCRC�͊܂܂��j
*		  �w�b�_����20�o�C�g
*		  �f�[�^�����e�L�X�g�w�b�_��(5)�{�e�L�X�g�{�f�B���iMAX=960�j
*			 CRC���� 2�o�C�g
*
*	�e�L�X�g�{�f�B���i�f�[�^��ʂɂ��قȂ�j
*	�E�R�}���h�i���Z�@���e�@�j
*		(23H)�����ݒ�f�[�^									��206�iCRC�܂ށj
*		(21H)���b�N���u����f�[�^							��7  �iCRC�܂ށj
*
*	�E�����i�e�@�ː��Z�@�j
*		(22H)I/F�՗v���f�[�^								��4  �iCRC�܂ށj
*	  ��(61H)�ʏ��ԃf�[�^									��454�iCRC�܂ށj
			�������u���b�N�ɂ��邱�Ƃ͂��Ȃ��B�i���̃|�[�����O���Ɏc�����������j
*			�E�w�胍�b�N���u�E�Z���T�[��ԉ���
*			�E�ڑ��S���b�N���u�E�Z���T�[��ԉ���
*			�E�ڑ��S���b�N���u�E�J/����
*		(62H)���b�N���u�E�J/�e�X�g����(�S�āE�w�苤��)	��93 �iCRC�܂ށj
*		(63H)�����e�i���X���v������						��867�iCRC�܂ށj
*	  ��(64H)�o�[�W�����v������								��131�iCRC�܂ށj
*	  ��(65H)�G���[��ԃf�[�^								��35 �iCRC�܂ށj
*	  ��(66H)�G���[�f�[�^									��6  �iCRC�܂ށj
***************************************************************/

/************************************
*	�VI/F�ՁE�e�q��					*
*	�ʐM�d���t�H�[�}�b�g			*
*************************************/
/****************************************
* �q�@�Z���T�[���
*	2005.03.04 �����E�ύX
*	bit3�F(=0)�����~��p�����̂ŁA�Ӗ��������Ȃ��Ȃ������A
*	����ɓ��삪�I��������H�ۂ��𔽉f����u��ԁv�ƈӖ��t�����B
*****************************************/
typedef union {
	struct {
		uchar	      B7:1;		/*	bit7�F�����e�i���X���[�h	0=�ʏ�^�p/1=�����e�i���X			2005.03.30 �����E�ǉ� */
		uchar	reserved:2;		/*	bit6�F�\��					0=�Œ�											*/
								/*	bit5�F�\��					0=�Œ�											*/
		uchar	      B4:1;		/*	bit4�F���b�N���u�������	0=�������/1=�J�������						*/
		uchar	      B3:1;		/*	bit3�F���b�N���u������	0=���슮��/1=���쒆	(�������ݕs��)	2005.03.04 �����E�ύX */
		uchar	      B2:1;		/*	bit2�F�ԗ����m�Z���T�[���	0=OFF/1=����ON		(�������ݕs��)				*/
		uchar	      B1:1;		/*	bit1�F���b�N�Z���T�[���	0=OFF/1=�{��ON		(�������ݕs��)				*/
		uchar	      B0:1;		/*	bit0�F���b�N�J�Z���T�[���	0=OFF/1=�J��ON		(�������ݕs��)				*/
	}		Bit;				/* �r�b�g�A�N�Z�X */
	uchar	Byte;				/* �o�C�g�A�N�Z�X */
} t_IF_LockSensor;

/****************************************
* �q�@�Z���T�[���
*	2005.03.04 �����E�ύX
*	bit3�F(=0)�����~��p�����̂ŁA�Ӗ��������Ȃ��Ȃ������A
*	����ɓ��삪�I��������H�ۂ��𔽉f����u��ԁv�ƈӖ��t�����B
*****************************************/
typedef struct{
	uchar	c_LoopSensor;			// ���[�v�Z���T��� '0':���g�p
									//					'1':OFF���
									//					'2':ON���
									//					'3':�ُ�
									//					'4':����OFF���
									//					'5':����ON���
									//					'6':�s�����
	uchar	c_ElectroSensor;		// �d���Z���T��� 	'0':���g�p
									//					'1':OFF���
									//					'2':ON���
	uchar	c_LockPlate;			// ���b�N���		'0':�ҋ@��
									//					'1':���~��
									//					'2':�㏸��
									//					'3':���~�G���[�i���~���ɃG���[�������j
									//					'4':�㏸�G���[�i�㏸���ɃG���[�������j
									//					'5':�������~
									//					'6':�����㏸
									//					'8':�s�����b�N
	uchar	c_FlapSensor;			//					'0':����
									//					'1':���~
									//					'2':�㏸
									//					'3':�ُ�
} t_IF_FlapSensor;

typedef struct {					// ���[�v�J�E���^
	uchar	ucLoopCount[4];			// ���[�v�J�E���g
	uchar	ucOffBaseCount[4];		// �I�t�x�[�X�J�E���g
	uchar	ucOnLevelCount[4];		// �I�����x���J�E���g
	uchar	ucOffLevelCount[4];		// �I�t���x���J�E���g
	uchar	ucOnBaseCount[4];		// �I���x�[�X�J�E���g
} t_IF_FlatLoopCounter;

typedef struct {
	struct {																/*								*/
		struct {
			ulong	ulAction;		/* �J����̍��v�񐔁i�����E�蓮�̍��v�A���ĊJ�����2��Ɛ�����j		*/
			ulong	ulManual;		/* I/F��SW�ɂ��蓮�ł̊J�E���썇�v��									*/
			ulong	ulTrouble;		/* �̏Ⴕ�����v�񐔁i���~�b�gSW��Ԃ��ω����Ȃ����j							*/
		} sLock[IFS_ROOM_MAX];
	} sSlave[IF_SLAVE_MAX];
} t_IF_LockAction;

/****************************************
*	�w�b�_�����i���ʁj
*	�E�f�[�^����10�o�C�g
*****************************************/
typedef struct {
	uchar	c_String[5];			/* �w�b�_������						"IFCOM"�Œ�								*/
	uchar	c_Size[2];				/* �f�[�^�T�C�Y�i�w�b�_������擪����BCC�܂ł̃T�C�Y�B�o�C�i���j			*/
	uchar	c_Kind[1];				/* �f�[�^���																*/
									/*		�i49H�F'I'�j�����ݒ�f�[�^		�i�e�@���q�@�j						*/
									/*		�i52H�F'R'�j��ԗv���f�[�^		�i�e�@���q�@�j						*/
									/*		�i57H�F'W'�j��ԏ����݃f�[�^	�i�e�@���q�@�j						*/
									/*		�i06H�FACK�j�m�艞���f�[�^		�i�q�@�ːe�@�j						*/
									/*		�i15H�FNAK�j�ے艞���f�[�^		�i�q�@�ːe�@�j						*/
									/*		�i41H�F'A'�j�q�@��ԃf�[�^		�i�q�@�ːe�@�j						*/
									/*		�i56H�F'V'�j�q�@�o�[�W�����f�[�^�i�q�@�ːe�@�j						*/
	uchar	c_TerminalNo[2];		/* I/F�Ձi�q�@�j�̃^�[�~�i��No.		�i"01"�`"15"�j							*/
} t_IF_Header;

/****************************************
*	�e�@���q�@
*	�i49H�F'I'�j�����ݒ�f�[�^
*	�E�f�[�^����14�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_Connect[IFS_ROOM_MAX];/* ���b�N���u�̐ڑ��i��ԁj													*/
									/* [0]���b�N���uNo.1														*/
									/* [6]���b�N���uNo.7�i�\���j												*/
									/* [7]���b�N���uNo.8�i�\���j												*/
									/*	'0'�F�ڑ�����															*/
									/*	'1'�F�ڑ��L��i������Љp�c�G���W�j�A�����O�����b�N���u�i���]�ԁj�j		*/
									/*	'2'�F�ڑ��L��i������Љp�c�G���W�j�A�����O�����b�N���u�i�o�C�N�j�j		*/
									/*	'3'�F�ڑ��L��i������Ћg�����쏊�����b�N���u�i���]�ԁA�o�C�N���p�j�j	*/
									/*	'4'�F�ڑ��L��i�R���Y�W���p�������b�N���u�i���]�ԁj�j					*/
									/*	'5'�F�ڑ��L��iHID�����b�N���u�i���]�ԁA�o�C�N���p�j�j					*/
	uchar	c_EntryTimer[2];		/* ���Ɏԗ����m�^�C�}�[				 (�ԗ����m�m�莞��  ��"01"�`"99" �b)	*/
	uchar	c_ExitTimer[2];			/* �o�Ɏԗ����m�^�C�}�[				 (�ԗ��񌟒m�m�莞�ԁ�"01"�`"99" �b)	*/
//CRC
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_IF_Body_I49;

/****************************************
*	�e�@���q�@
*	�i52H�F'R'�j��ԗv���f�[�^
*	�E�f�[�^����3�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_Request[1];			/* �v�����e		�i'0'�`'F'�j		*/
									/*	 	 '0'�F�q�@��ԃf�[�^�v��	*/
									/*		 '1'�F�o�[�W�����v��		*/
									/*	'2'�`'F'�F�\��					*/
//CRC
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_IF_Body_R52;

/****************************************
*	�e�@���q�@
*	�i57H�F'W'�j��ԏ����݃f�[�^
*	�E�f�[�^����9�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_LockOnTime[2];		/* ���b�N���u���̐M���o�͎��ԁi�~150msec�j	�i"01"�`"99"�j				*/
	uchar	c_LockOffTime[2];		/* ���b�N���u�J���̐M���o�͎��ԁi�~150msec�j	�i"01"�`"99"�j				*/
	uchar	c_RoomNo[1];			/* ���b�N���uNo.�i�\���܂ށj					�i'1'�`'8'�j				*/
	uchar	c_MakerID[1];			/* ���b�N���u���[�J�[ID�i�����ݒ�f�[�^�́u���b�N���u�̐ڑ��v���l�j		2005.03.30 �����E�ǉ� */
t_IF_LockSensor	unCommand;			/* �������݃Z���T�[��ԁi�o�C�i���j											*/
									/*		bit0�F���b�N�J�Z���T�[���	0=OFF/1=ON�i�������ݕs�j				*/
									/*		bit1�F���b�N�Z���T�[���	0=OFF/1=ON�i�������ݕs�j				*/
									/*		bit2�F�ԗ����m�Z���T�[���	0=OFF/1=ON�i�������ݕs�j				*/
									/*		bit3�F���b�N���u������	0=���슮��/1=���쒆	(�������ݕs��)	2005.03.04 �����E�ύX */
									/*		bit4�F���b�N���u�������	0=�������/1=�J�������				*/
									/*		bit5�F�\��					0=�Œ�i�������ݕs�j					*/
									/*		bit6�F�K�[�h���Ԏw���r�b�g	1=�Œ�									*/
									/*		bit7�F�����e�i���X���[�h	0=�ʏ�^�p/1=�����e�i���X			2005.03.30 �����E�ǉ� */
//CRC
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_IF_Body_W57;

/****************************************
*	�q�@�ːe�@
*	�i06H�FACK ����R�[�h�j�m�艞���f�[�^
*	�E�f�[�^����3�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_Request[1];			/* �v�����e		�i'0'�`'F'�j		*/
									/*		 '0'�F�Œ�					*/
									/*	'1'�`'F'�F�\��					*/
//CRC
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_IF_Body_ACK;

/*
*	�q�@�ːe�@
*	�i15H�FNAK ����R�[�h�j�ے艞���f�[�^
*	�E�f�[�^����3�o�C�g�iCRC�܂ށj
*/
typedef struct {
	uchar	c_Request[1];			/* �v�����e		�i'0'�`'F'�j								*/
									/*		 '0'�F�d��(BCC/�f�[�^��)�ُ�						*/
									/*		 '1'�F�����ݒ�f�[�^�v���i�����ݒ�f�[�^����M�j	*/
									/*	'2'�`'F'�F�\��											*/
//CRC
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g							*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g							*/
} t_IF_Body_NAK;

/****************************************
*	�q�@�ːe�@
*	�i41H�F'A'�j�q�@��ԃf�[�^
*	�E�f�[�^����12�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_Request[1];			/* �v�����e		�i'0'�`'F'�j												*/
									/*		 '0'�F�e�@�ɑ΂���v������											*/
									/*		 '1'�F�����ݒ�f�[�^�v��											*/
									/*		 '2'�F�Z���T�[���m��											2005.08.26 �����E�ǉ� */
									/*	'2'�`'F'�F�\��															*/
	uchar	c_Type[1];				/* ���u���							'0'�Œ�i���b�N���u�j					*/
t_IF_LockSensor	unSensor[IFS_ROOM_MAX];	/* ���b�N���u�Z���T�[��ԁi�o�C�i���j									*/
									/*		bit0�F���b�N�J�Z���T�[���	0=OFF/1=ON								*/
									/*		bit1�F���b�N�Z���T�[���	0=OFF/1=ON								*/
									/*		bit2�F�ԗ����m�Z���T�[���	0=OFF/1=ON								*/
									/*		bit3�F���b�N���u������	0=���슮��/1=���쒆	(�������ݕs��)	2005.03.04 �����E�ύX */
									/*		bit4�F���b�N���u�������	0=�������/1=�J�������				*/
									/*		bit5�F�\��					0=�Œ�									*/
									/*		bit6�F�\��					0=�Œ�									*/
									/*		bit7�F�����e�i���X���[�h	0=�Œ�									*/
//CRC
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g							*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g							*/
} t_IF_Body_A41;

/****************************************
*	�q�@�ːe�@
*	�i56H�F'V'�j�q�@�o�[�W�����f�[�^
*	�E�f�[�^����12�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_Request[1];			/* �v�����e		�i'0'�`'F'�j												*/
									/*		 '0'�F�e�@�ɑ΂���v������											*/
									/*		 '1'�F�����ݒ�f�[�^�v��											*/
									/*	'2'�`'F'�F�\��															*/
	uchar	c_Reserved[1];			/* �\��								'0'�Œ�									*/
	uchar	c_Version[8];			/* �o�[�W�����i���ԁAASCII�j												*/
//CRC
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g							*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g							*/
} t_IF_Body_V56;


/****************************************
*	�t���b�v�q�@�ːe�@
*	�i57H�F'W'�j�t���b�v�q�@��ԏ������݃f�[�^
*	�E�f�[�^����9�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_Reserved1[2];			// �\�� "00"�Œ�
	uchar	c_Reserved2[2];			// �\�� "00"�Œ�
	uchar	c_Reserved3[1];			// �\�� '0'�Œ�
	uchar	c_Reserved4[1];			// �\�� '0'�Œ�
	uchar	c_Command[1];			// �������݃f�[�^	'1':���[�v����OFF
									//					'2':���[�v����ON
									//					'4':�t���b�v�㏸
									//					'5':�t���b�v���~
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_FLAP_Body_W57;

/****************************************
*	�t���b�v�q�@�ːe�@
*	�i41H�F'A'�j�t���b�v�q�@��ԃf�[�^
*	�E�f�[�^����12�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_Request[1];			// �v�����e '0':�e�@�ɑ΂��鉞������
	uchar	c_Type[1];				// ���u��� 'F'�Œ�i�t���b�v���u�j
	t_IF_FlapSensor t_FlapSensor;	// �t���b�v�Z���T��ԏ��
	uchar	c_Reserved2[4];			// �\�� "0000"�Œ�
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_FLAP_Body_A41;

/****************************************
*	�t���b�v�q�@�ːe�@
*	�i41H�F'A'�j�t���b�v�q�@��ԃf�[�^
*	�E�f�[�^����12�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_Request[1];			// �v�����e '0':�e�@�ɑ΂��鉞������
	t_IF_FlatLoopCounter t_LoopCounter;	// ���[�v�f�[�^
	uchar	ucCRC16[2];				/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
									/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_FLAP_Body_S53;
/****************************************
*	�e�@���q�@
*	�i49H�F'I'�jCRR�����ݒ�f�[�^
*	�E�f�[�^����32�o�C�g
*****************************************/
typedef struct {
	uchar	ucConnect[IFS_CRRFLAP_MAX][2];	/* CRR��No.���A�ڑ��w��													*/
											/* �ړ_1�`10���Ƀ^�[�~�i��No.������t����								*/
											/* ucConnect[3] = '05'�Ɛݒ肷��ƁA�ړ_4�̓^�[�~�i��No.5 �ƂȂ�		*/
//CRC
	uchar	ucCRC16[2];					/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
										/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_FLAP_Body_I49;
/****************************************
*	�e�@���q�@
*	�i56H�F'V'�jCRR�o�[�W�����f�[�^
*	�E�f�[�^����10�o�C�g
*****************************************/
typedef struct {
	uchar	dummy[8];					/* �\��								*/
//CRC
	uchar	ucCRC16[2];					/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
										/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_FLAP_Body_V56;
/****************************************
*	�q�@�ːe�@
*	�i76H�F'v'�jCRR�o�[�W�����f�[�^
*	�E�f�[�^����10�o�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_Version[8];				/* �o�[�W�����i���ԁAASCII�j		*/
//CRC
	uchar	ucCRC16[2];					/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
										/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_FLAP_Body_v76;
/****************************************
*	�e�@���q�@
*	�i54H�F'T'�jCRR�܂�Ԃ��e�X�g
*	�E�f�[�^����10�o�C�g
*****************************************/
typedef struct {
	uchar	dummy[8];					/* �\��								*/
//CRC
	uchar	ucCRC16[2];					/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
										/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_FLAP_Body_T54;
/****************************************
*	�q�@�ːe�@
*	�i74H�F't'�jCRR�܂�Ԃ��e�X�g����
*	�E�f�[�^����17�޲ăo�C�g�iCRC�܂ށj
*****************************************/
typedef struct {
	uchar	c_TestResult[IFS_CRRFLAP_MAX];	/* �e�X�g����						*/
//CRC
	uchar	ucCRC16[2];					/*	[0]�FCRC16�r�b�g�̉���8�r�b�g	*/
										/*	[1]�FCRC16�r�b�g�̏��8�r�b�g	*/
} t_FLAP_Body_t74;

/************************
*	�e�q�ԓd���S�̑�	*
*************************/
typedef struct {
	t_IF_Header	sHead;				/* �w�b�_����				  �i���ʁj	�f�[�^����10 */
	union {
		t_IF_Body_I49	sI49;		/* �����ݒ�f�[�^		�i�e�@���q�@�j	�f�[�^����14 */
		t_IF_Body_R52	sR52;		/* ��ԗv���f�[�^		�i�e�@���q�@�j	�f�[�^���� 3 */
		t_IF_Body_W57	sW57;		/* ��ԏ����݃f�[�^		�i�e�@���q�@�j	�f�[�^���� 9 */
		t_IF_Body_ACK	sACK;		/* �m�艞���f�[�^		�i�q�@�ːe�@�j	�f�[�^���� 3 */
		t_IF_Body_NAK	sNAK;		/* �ے艞���f�[�^		�i�q�@�ːe�@�j	�f�[�^���� 3 */
		t_IF_Body_A41	sA41;		/* �q�@��ԃf�[�^		�i�q�@�ːe�@�j	�f�[�^����12 */
		t_IF_Body_V56	sV56;		/* �q�@�o�[�W�����f�[�^	�i�q�@�ːe�@�j	�f�[�^����12 */
		t_FLAP_Body_W57	sFlapW57;	/* ��ԏ����݃f�[�^		�i�e�@���q�@�j	�f�[�^���� 9 */
		t_FLAP_Body_A41	sFlapA41;	/* �q�@��ԃf�[�^		�i�q�@�ːe�@�j	�f�[�^����12 */
		t_FLAP_Body_S53	sFlapS53;	/* ���[�v�f�[�^����		�i�q�@�ːe�@�j	�f�[�^����23 */
		t_FLAP_Body_I49	sFlapI49;	/* CRR�����ݒ�f�[�^	�i�e�@���q�@�j	�f�[�^����32 */
		t_FLAP_Body_V56	sFlapV56;	/* CRR�o�[�W�����f�[�^	�i�e�@���q�@�j	�f�[�^����10 */
		t_FLAP_Body_v76	sFlapv76;	/* CRR�o�[�W�����f�[�^	�i�q�@�ːe�@�j	�f�[�^����10 */
		t_FLAP_Body_T54	sFlapT54;	/* CRR�܂�Ԃ��e�X�g	�i�e�@���q�@�j	�f�[�^����10 */
		t_FLAP_Body_t74 sFlapt74;	/* CRR�܂�Ԃ��e�X�g���ʁi�q�@�ːe�@�j	�f�[�^����17 */
		uchar			Byte[1];	/* �o�C�g��E�E�E�K�v�H					�d���Œ���24 */
	} unBody;
} t_IF_Packet;

#pragma unpack
/*
*	�\���̃����o�̋��E���������u1�v�Ƃ���B�������B
*/
#endif	// IF_H

