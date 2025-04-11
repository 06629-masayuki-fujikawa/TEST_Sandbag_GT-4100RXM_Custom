// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/*[]----------------------------------------------------------------------[]*
 *|		NT-NET task communications parts
 *[]----------------------------------------------------------------------[]*
 *| Author      :  J.Mitani
 *| Date        :  2005-06-08
 *| Update      :	2005.05.28	machida.k	[see _NT_DBG]	�f�o�b�O�R�[�h�ǉ�(���������ɋ����I��CRC�G���[�𔭐�������)
 *|				:	2005.06.15	#001	J.Mitani �d�l�ύX�ɂ��ENQ�d���ɗD�惂�[�h�t���O��ǉ�
 *|				:	2005.08.16	#002	J.Mitani �[���J�b�g����w�b�_�[�݂̂̓d���𖳎����Ă��܂��\�����������̂��C���
 *|					2005-12-20			machida.k	RAM���L��
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/
#define		NT_CRCPOLY1				0x1021										/* ���V�t�g						*/
#define		NT_CRCPOLY2				0x8408  									/* �E�V�t�g						*/
#define		NT_CHAR_BIT				8											/* number of bits in a char		*/
#define		NT_L_SHIFT				0											/* ���V�t�g						*/
#define		NT_R_SHIFT				1											/* �E�V�t�g						*/

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/

/*----------------------------------*/
/*			area define				*/
/*----------------------------------*/

/*----------------------------------*/
/*			table define			*/
/*----------------------------------*/

/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	NT_CrcCcitt(ushort length, uchar cbuf[], uchar *result, uchar type );





/*[]----------------------------------------------------------------------[]*
 *|             NTComComm_Start()                                          |*
 *[]----------------------------------------------------------------------[]*
 *|		NT�|NET�J�n����										               |*
 *[]----------------------------------------------------------------------[]*/
void	NTComComm_Start( void )
{
	NTComSlave_Start(&send_blk, &receive_blk);
}

/*[]----------------------------------------------------------------------[]*
 *|             NTComComm_Clear()                                          |*
 *[]----------------------------------------------------------------------[]*
 *|		�p�X���[�h�j�󎞏���								               |*
 *[]----------------------------------------------------------------------[]*/
void	NTComComm_Clear( void )
{
	NTComSlave_Clear();
}

/*[]----------------------------------------------------------------------[]*
 *|             NTComComm_Main()                                           |*
 *[]----------------------------------------------------------------------[]*
 *|		NT�|NET���C������												   |*
 *[]----------------------------------------------------------------------[]*/
void	NTComComm_Main( void )
{
	NTComSlave_Main();
}



/*[]-----------------------------------------------------------------------[]*
 *|             NTComComm_packet_check()
 *[]-----------------------------------------------------------------------[]*
 *|		��M�����p�P�b�g�̏�Ԃ��`�F�b�N����B
 *[]-----------------------------------------------------------------------[]*
 *|		check_blk		�`�F�b�N�Ώۂ̃u���b�N
 *|		seq_no			��r�p�V�[�P���X�i���o�[
 *|		terminal_no		��r�p�[���ԍ�
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture
 *[]-----------------------------------------------------------------------[]*/
BOOL NTComComm_packet_check(T_NT_BLKDATA* check_blk, uchar seq_no, uchar terminal_no){

	ushort len;

	// ����R�[�h�`�F�b�N
	if (check_blk->data[NT_OFS_TELEGRAM_KIND] != STX
	 && check_blk->data[NT_OFS_TELEGRAM_KIND] != EOT
	 && check_blk->data[NT_OFS_TELEGRAM_KIND] != ACK
	 && check_blk->data[NT_OFS_TELEGRAM_KIND] != ENQ
	 && check_blk->data[NT_OFS_TELEGRAM_KIND] != NAK) {
		return FALSE;
	}

	len = NT_MakeWord(&check_blk->data[NT_OFS_DATA_SIZE_HI]);

	// �f�[�^�T�C�Y�`�F�b�N
	if (check_blk->data[NT_OFS_TELEGRAM_KIND] == STX) {
		if (25 > len || len > 985) {
			return FALSE;
		}
	} else {	// ENQ ACK NAK EOT
		if (len != 20) {
			return FALSE;
		}
	}

	// �[���ԍ��`�F�b�N
	if ((check_blk->data[NT_OFS_TERMINAL_NO] != 0xFF) && (terminal_no != check_blk->data[NT_OFS_TERMINAL_NO])) {
		return FALSE;
	}

	// �ʐM�����NNo.�`�F�b�N
	//ENQ���͔�r���Ȃ�
	if (check_blk->data[NT_OFS_TELEGRAM_KIND] != ENQ) {
		// ���݂̑I������Ă���[��No.�Ɣ�r
		if (seq_no != check_blk->data[NT_OFS_LINK_NO]) {
			return FALSE;
		}
	}
	check_blk->len = len;

	return TRUE;
}

/*[]-----------------------------------------------------------------------[]*
 *|             NTComComm_crc_check()
 *[]-----------------------------------------------------------------------[]*
 *|			 CRC���`�F�b�N����B
 *[]-----------------------------------------------------------------------[]*
 *|		buff	�`�F�b�N����d���B
 *[]-----------------------------------------------------------------------[]*
 *|		TRUE = CRC�G���[�Ȃ�	FALSE = CRC�G���[����
 *[]-----------------------------------------------------------------------[]*/
BOOL NTComComm_crc_check(T_NT_BLKDATA* buff) {

	uchar crc[2];

	NT_CrcCcitt(buff->len, buff->data, crc, 1);

	if (crc[0] != buff->data[buff->len]
	 || crc[1] != buff->data[buff->len + 1]) {
		return FALSE;
	}

	return TRUE;
}

/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_enq_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		�d�����i�[����o�b�t�@
 *|		uchar send_req_flag		���M�v���t���O
 *|		uchar terminal_no		�[��No.
 *|		uchar seq_no			�V�[�P���XNo.
 *|		uchar terminal_status	�[���X�e�[�^�X
 *|		uchar packet_mode		�p�P�b�g�D�惂�[�h
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void NTComComm_create_enq_telegram(T_NT_BLKDATA* buff, uchar send_req_flag, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar packet_mode) {

	uchar	crc[2];

	memset(buff->data, 0, 0x14);								//�g���̈���O�ŏ�����
	buff->len = 0x16;											//���M�f�[�^���@�i�f�[�^�T�C�Y�@�{�@CRC16�j

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"���R�s�[
	buff->data[NT_OFS_DATA_SIZE_HI]			= 0x00;				//��ʂ͂O
	buff->data[NT_OFS_DATA_SIZE_LOW]		= 0x14;				//���ʂ͂Q�O�i14h�j
	buff->data[NT_OFS_SEND_REQ_FLAG]		= send_req_flag;	//���M�v���t���O
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//�[��No.
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//�V�[�P���XNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//�[���X�e�[�^�X
	buff->data[NT_OFS_PACKET_MODE]			= packet_mode;		//�p�P�b�g�D�惂�[�h	#001
	buff->data[NT_OFS_TELEGRAM_KIND]		= ENQ;				//ENQ

	NT_CrcCcitt(0x14, buff->data, crc, 1);

	buff->data[0x14]						= crc[0];			//CRC
	buff->data[0x14 + 1]					= crc[1];			//CRC

}
/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_stx_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		�d�����i�[����o�b�t�@
 *|		uchar direction			�f�[�^���M����
 *|		uchar terminal_no		�[��No.
 *|		uchar seq_no			�V�[�P���XNo.
 *|		uchar terminal_status	�[���X�e�[�^�X
 *|		uchar broadcast_flag	�����M�ς݃t���O
 *|		uchar response_detail	�����ڍ�
 *[]-----------------------------------------------------------------------[]*/
void NTComComm_create_stx_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail) {

	uchar	crc[2];
	ulong		Start_1msLifeTime;								// �d����M���_��1ms���C�t�^�C�}�[
	ulong		Past_1msTime;									// ��M�`���݂܂ł̌o�ߎ��ԁix1ms�j

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"���R�s�[
	buff->data[NT_OFS_DIRECTION]			= direction;		//���M����
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//�[��No.
	buff->data[NT_OFS_SLAVE_TERMINAL_NO1]	= 0;				//
	buff->data[NT_OFS_SLAVE_TERMINAL_NO2]	= 0;				//
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//�V�[�P���XNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//�[���X�e�[�^�X
	buff->data[NT_OFS_BROADCASTED_FLAG]		= broadcast_flag;	//����f�[�^��M�ς�

	buff->data[NT_OFS_TELEGRAM_KIND]		= STX;				//STX

	buff->data[NT_OFS_RESPONSE_DETAIL]		= response_detail;	//�����ڍ�

	if( TRUE == NT_IsNewTypeTimeSetPacket(buff) ) {				// �V�`���̎��v�f�[�^�d��
		memcpy( &Start_1msLifeTime, &buff->data[NT_OFS_DATA_TMPKT_FREETIMER], 4 );	// ��M���_�̃��C�t�^�C�}�[�lget
		Past_1msTime = LifePastTim1msGet( Start_1msLifeTime );		// ��M���_����̌o�ߎ���get
		memcpy( &buff->data[NT_OFS_DATA_TMPKT_HOSEI], &Past_1msTime, 4 );	// ��M���_����̌o�ߎ���set
	}

	NT_CrcCcitt(buff->len, buff->data, crc, 1);

	buff->data[buff->len]						= crc[0];		//CRC
	buff->data[buff->len + 1]					= crc[1];		//CRC
	buff->len += 2;
}
/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_ack_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		�d�����i�[����o�b�t�@
 *|		uchar direction			�f�[�^���M����
 *|		uchar terminal_no		�[��No.
 *|		uchar seq_no			�V�[�P���XNo.
 *|		uchar terminal_status	�[���X�e�[�^�X
 *|		uchar broadcast_flag	�����M�ς݃t���O
 *|		uchar response_detail	�����ڍ�
 *[]-----------------------------------------------------------------------[]*/
void NTComComm_create_ack_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail) {

	uchar	crc[2];

	memset(buff->data, 0, 0x14);								//�g���̈���O�ŏ�����
	buff->len = 0x16;											//���M�f�[�^���@�i�f�[�^�T�C�Y�@�{�@CRC16�j

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"���R�s�[
	buff->data[NT_OFS_DATA_SIZE_HI]			= 0x00;				//��ʂ͂O
	buff->data[NT_OFS_DATA_SIZE_LOW]		= 0x14;				//���ʂ͂Q�O�i14h�j
	buff->data[NT_OFS_DIRECTION]			= direction;		//���M����
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//�[��No.
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//�V�[�P���XNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//�[���X�e�[�^�X
	buff->data[NT_OFS_BROADCASTED_FLAG]		= broadcast_flag;	//�����M�ς݃t���O

	buff->data[NT_OFS_TELEGRAM_KIND]		= ACK;				//ENQ

	buff->data[NT_OFS_RESPONSE_DETAIL]		= response_detail;	//�����ڍ�

	NT_CrcCcitt(0x14, buff->data, crc, 1);

	buff->data[0x14]						= crc[0];			//CRC
	buff->data[0x14 + 1]					= crc[1];			//CRC



}
/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_nak_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		�d�����i�[����o�b�t�@
 *|		uchar direction			�f�[�^���M����
 *|		uchar terminal_no		�[��No.
 *|		uchar seq_no			�V�[�P���XNo.
 *|		uchar terminal_status	�[���X�e�[�^�X
 *|		uchar broadcast_flag	�����M�ς݃t���O
 *|		uchar response_detail	�����ڍ�
 *[]-----------------------------------------------------------------------[]*/
void NTComComm_create_nak_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail) {

	uchar	crc[2];

	memset(buff->data, 0, 0x14);								//�g���̈���O�ŏ�����
	buff->len = 0x16;											//���M�f�[�^���@�i�f�[�^�T�C�Y�@�{�@CRC16�j

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"���R�s�[
	buff->data[NT_OFS_DATA_SIZE_HI]			= 0x00;				//��ʂ͂O
	buff->data[NT_OFS_DATA_SIZE_LOW]		= 0x14;				//���ʂ͂Q�O�i14h�j
	buff->data[NT_OFS_DIRECTION]			= direction;		//���M����
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//�[��No.
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//�V�[�P���XNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//�[���X�e�[�^�X
	buff->data[NT_OFS_BROADCASTED_FLAG]		= broadcast_flag;	//�����M�ς݃t���O

	buff->data[NT_OFS_TELEGRAM_KIND]		= NAK;				//ENQ

	buff->data[NT_OFS_RESPONSE_DETAIL]		= response_detail;	//�����ڍ�

	NT_CrcCcitt(0x14, buff->data, crc, 1);

	buff->data[0x14]						= crc[0];			//CRC
	buff->data[0x14 + 1]					= crc[1];			//CRC



}
/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_create_eot_telegram()
 *[]-----------------------------------------------------------------------[]*
 *|		T_NT_BLKDATA* buff		�d�����i�[����o�b�t�@
 *|		uchar direction			�f�[�^���M����
 *|		uchar terminal_no		�[��No.
 *|		uchar seq_no			�V�[�P���XNo.
 *|		uchar terminal_status	�[���X�e�[�^�X
 *|		uchar broadcast_flag	�����M�ς݃t���O
 *|		uchar response_detail	�����ڍ�
 *[]-----------------------------------------------------------------------[]*/
void NTComComm_create_eot_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail) {

	uchar	crc[2];

	memset(buff->data, 0, 0x14);								//�g���̈���O�ŏ�����
	buff->len = 0x16;											//���M�f�[�^���@�i�f�[�^�T�C�Y�@�{�@CRC16�j

	memcpy(buff->data, NT_PKT_SIG, 5);							//"NTCOM"���R�s�[
	buff->data[NT_OFS_DATA_SIZE_HI]			= 0x00;				//��ʂ͂O
	buff->data[NT_OFS_DATA_SIZE_LOW]		= 0x14;				//���ʂ͂Q�O�i14h�j
	buff->data[NT_OFS_DIRECTION]			= direction;		//���M����
	buff->data[NT_OFS_TERMINAL_NO]			= terminal_no;		//�[��No.
	buff->data[NT_OFS_LINK_NO]				= seq_no;			//�V�[�P���XNo.
	buff->data[NT_OFS_TERMINAL_STS]			= terminal_status;	//�[���X�e�[�^�X
	buff->data[NT_OFS_BROADCASTED_FLAG]		= broadcast_flag;	//�����M�ς݃t���O

	buff->data[NT_OFS_TELEGRAM_KIND]		= EOT;				//ENQ

	buff->data[NT_OFS_RESPONSE_DETAIL]		= response_detail;	//�����ڍ�

	NT_CrcCcitt(0x14, buff->data, crc, 1);

	buff->data[0x14]						= crc[0];			//CRC
	buff->data[0x14 + 1]					= crc[1];			//CRC

}

/*[]-----------------------------------------------------------------------[]*
 *|				NTComComm_isIdle()
 *[]-----------------------------------------------------------------------[]*
 *|		�^�X�N�̃A�C�h����Ԃ��擾����
 *[]-----------------------------------------------------------------------[]*/
BOOL NTComComm_isIdle( void ) {
	return NTComSlave_isIdle();
}

/* CRC16-CCITT �Z�o�֐� (�E�V�t�g���g�p����) */									/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	Calculate CRC-CCITT																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : crc_ccitt( len, cbuf, result, type )														   |*/
/*| PARAMETER    : len   : Data length																		   |*/
/*|              : cbuf  : Char data																		   |*/
/*|				 : result: Crc																				   |*/
/*|				 : type  : 0: left shift 1:rigth shift														   |*/
/*| RETURN VALUE : void																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�ECRC�̌v�Z���s��																						   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
/* #1 */
void	NT_CrcCcitt(
ushort	length,
uchar	cbuf[],
uchar	*result,
uchar	type )
{																				/*								*/
	ushort	i, j;
	uchar	crc_wk;
	ushort	crc;
																				/*								*/
	crc		= 0x0000;															/*								*/
	crc_wk	= 0x0000;															/*								*/
																				/*								*/
	switch( type ){																/*								*/
																				/*								*/
	case 0: 	/* ���V�t�g	*/													/*								*/
		for(i = 0; i < length; i++) {											/*								*/
			crc ^= (ushort)cbuf[i] << (16 - NT_CHAR_BIT);
			for (j = 0; j < NT_CHAR_BIT; j++){									/*								*/
				if (crc & 0x8000){												/*								*/
					crc = (crc << 1) ^ NT_CRCPOLY1;								/*								*/
				}else{															/*								*/
					crc <<= 1;													/*								*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	case 1: 	/* �E�V�t�g	*/													/*								*/
		for (i = 0; i < length; i++) {											/*								*/
			crc ^= cbuf[i];														/*								*/
			for (j = 0; j < NT_CHAR_BIT; j++){									/*								*/
				if (crc & 1){													/*								*/
					crc = (crc >> 1) ^ NT_CRCPOLY2;								/*								*/
				}else{															/*								*/
					crc >>= 1;													/*								*/
				}																/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	}																			/*								*/
	crc_wk = (uchar)(crc & 0x00ff);
	*result = crc_wk;															/*								*/
																				/*								*/
	crc_wk = (uchar)((crc & 0xff00) >> NT_CHAR_BIT);
	*(result+1) = crc_wk;														/*								*/
																				/*								*/
}																				/*								*/

ushort NT_MakeWord(unsigned char *data)
{
	ushort	value;

	value =  *(data + 0);
	value <<= 8;
	value |= *(data + 1);

	return value;
}
void NT_Word2Byte(unsigned char *data, ushort us)
{
	*(data + 0) = (us & 0xff00) >> 8;
	*(data + 1) = (us & 0x00ff);
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
