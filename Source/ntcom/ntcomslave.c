// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/*[]----------------------------------------------------------------------[]*
 *|		NT-NET task communications part
 *[]----------------------------------------------------------------------[]*
 *| Author      :  J.Mitani
 *| Date        :  2005-06-09
 *| Update      :	2005-06-18	#001	J.Mitani	�d�l�ύX�̂���ENQ�d���Ƀp�P�b�g�D�惂�[�h��ǉ�
 *| Update      :	2005-06-18	#002	J.Mitani	�d�l�ύX�̂���ACK02���g���C�A�E�g�œd���폜
 *|	Update		:	2005-09-13	#003	machida.k	���M�p�D��f�[�^�o�b�t�@�͒[�������m�ۂ���
 *| Update		:	2005-12-20			machida.k	RAM���L��
 *| Update		:	2006-01-11	#004	J.Mitani	�s��C��
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
#define			_NTComData_GetRemainBlkNum(tele)	((tele)->blknum - (tele)->curblk)

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
static void slave_goto_idle( void );

static uchar slave_check_receive_data( void );
static uchar slave_get_terminal_status( uchar tele_kind, uchar send_req_flag, uchar packet_mode );

static void slave_data_received( void );				// �f�[�^��M����
static void slave_idol_prosess( void );
static void slave_stx_send( void );
static void slave_ack_send( void );
static void slave_broad_cast( void );
static uchar slave_check_send_data( void );
static void slave_start_timer(uchar timer_kind);
static void slave_check_response_detail( void );
static void slave_check_crc_ctrl_tele(BOOL bCrc);

static void int_com_slave_timer_10m(void);
static void int_com_slave_timer_100m(void);



/*[]----------------------------------------------------------------------[]*
 *|             NTComSlave_Start()                                         |*
 *[]----------------------------------------------------------------------[]*
 *|		NTCom READER communication parts initialize routine	               |*
 *[]----------------------------------------------------------------------[]*/
void NTComSlave_Start(T_NT_BLKDATA* send, T_NT_BLKDATA* receive)
{

	NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_IDOL;				// �]�ǃX�e�[�^�X
	NTComComm_Ctrl_Slave.master_status = NT_NORMAL;				// ��ǃX�e�[�^�X
	NTComComm_Ctrl_Slave.broad_cast_received = 0;					// ����f�[�^��M�ς�
	NTComComm_Ctrl_Slave.block_send_count = 0;						// �u���b�N���M��
	NTComComm_Ctrl_Slave.block_receive_count = 0;					// �u���b�N��M��
	NTComComm_Ctrl_Slave.terminal_num = NTCom_InitData.TerminalNum;	// ��ǎ��F�ڑ������^�[�~�i����	�]�ǎ��F���ǔԍ�
	NTComComm_Ctrl_Slave.receive_packet_crc_status = TRUE;			// ��M�����p�P�b�g��CRC�G���[���
	NTComComm_Ctrl_Slave.receive_crc_err_count = 0;				// CRC�G���[�̎�M�񐔡
	NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl = 0;			// CRC�G���[�̎�M�񐔡
	NTComComm_Ctrl_Slave.send_retry_count = 0;						// ���M���g���C��
	NTComComm_Ctrl_Slave.ack02_receive_count = 0;					// ACK02���g���C��	#002
	NTComComm_Ctrl_Slave.timeout_kind = 0;							// �^�C���A�E�g���	#004

	NTComSlave_send_blk = send;											// ���M�o�b�t�@		���������Ɉ����œn��
	NTComSlave_receive_blk = receive;										// ��M�o�b�t�@

	NTComComm_Ctrl_Slave.timer_10m.timerid = NTComTimer_Create(10, 10, int_com_slave_timer_10m, TRUE);
	NTComComm_Ctrl_Slave.timer_100m.timerid = NTComTimer_Create(100, 100, int_com_slave_timer_100m, TRUE);

	NTComComm_Ctrl_Slave.timer_100m.count = NTCom_InitData.Time_LineMonitoring * 10;
	NTComTimer_Start(NTComComm_Ctrl_Slave.timer_100m.timerid);

}

/*[]----------------------------------------------------------------------[]*
 *|             NTComSlave_Clear()                                         |*
 *[]----------------------------------------------------------------------[]*
 *|		NTCom READER communication parts initialize routine	               |*
 *[]----------------------------------------------------------------------[]*/
void NTComSlave_Clear( void )
{

	// ���ݎg�p���Ă���A�d���������Z�b�g����
	NTComData_ResetTelegram(&normal_telegram);
	NTComData_ResetTelegram(&prior_telegram);
	last_telegram = NULL;

}


/*[]----------------------------------------------------------------------[]*
 *|             NTComSlave_Main()                                          |*
 *[]----------------------------------------------------------------------[]*
 *|		NTCom READER function communication parts Main routine			   |*
 *[]----------------------------------------------------------------------[]*/
void	NTComSlave_Main( void )
{
	if (NTComComm_Ctrl_Slave.timer_10m.count == 0) {	// �^�C���A�E�g���Ă������M���s�Ƃ��ăA�C�h����
		slave_goto_idle();
	}


	// NE-NET�p�P�b�g���o
	switch(NTComComdr_GetRcvPacket(NTComSlave_receive_blk)){
	case	NT_COMDR_NO_PKT:						// �A�C�h��
		break;
	case	NT_COMDR_PKT_RCVING:					// �p�P�b�g��M��
		if ((NTComComm_Ctrl_Slave.timeout_kind == 3) && (NTComComm_Ctrl_Slave.timer_10m.count > 0)) {		// #004
			// �N�����Ă���^�C�}�[��T3��������
			// T2 �^�C�}�[�N���J�n
			NTComComm_Ctrl_Slave.timeout_kind = 2;
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		}
		break;
	case	NT_COMDR_PKT_RCVD:						// �p�P�b�g��M����
		// �p�P�b�g�`�F�b�N
		if (NTComComm_packet_check(NTComSlave_receive_blk, NTComComm_Ctrl_Slave.cur_seq_no, NTCom_InitData.TerminalNum)) {
			// TURE
			NTComComm_Ctrl_Slave.receive_packet_crc_status = NTComComm_crc_check(NTComSlave_receive_blk);

			// T2�^�C�}�[�X�g�b�v
			NTComComm_Ctrl_Slave.timer_10m.count = -1;
			slave_data_received();		// ��{�I�ȓd���`�F�b�N��ʉ߂����̂ŁA�v���g�R���Ɉˑ������������J�n�B
		}
		break;
	case	NT_COMDR_ERR_SCI:						// �ʐM�G���[(�I�[�o�[�����G���[/�t���[�~���O�G���[/�p���e�B�G���[)
		// �ʐM�G���[�����@���@IBW�ɃG���[������ʒm

		break;
	case	NT_COMDR_ERR_INVALID_LEN:				// �p�P�b�g�f�[�^���s��
	case	NT_COMDR_ERR_TIMEOUT:					// �p�P�b�g��M�����O�ɃL�����N�^�ԃ^�C���A�E�g
	default:
		break;
	}

	if (NTComComm_Ctrl_Slave.status == NT_COM_SLAVE_IDOL
	 && NTComComm_Ctrl_Slave.timer_100m.count == 0
	 && NTComComm_Ctrl_Slave.master_status == NT_NORMAL) {	// �ʐM����Ď��^�C�}�[�`�F�b�N

		NTCom_err_chk(NT_ERR_NO_RESPONSE, 1, 0, 0, NULL);
		NTComData_SetTerminalStatus(NTComComm_Ctrl_Slave.terminal_num, NT_ABNORMAL, &normal_telegram, &prior_telegram);
		NTComComm_Ctrl_Slave.master_status = NT_ABNORMAL;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|			NTComSlave_isIdle()
 *[]-----------------------------------------------------------------------[]*
 *|			�^�X�N�̃A�C�h����Ԃ��擾����
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
BOOL NTComSlave_isIdle( void )
{
	if (NTComComm_Ctrl_Slave.status == NT_COM_SLAVE_IDOL) {
		return TRUE;
	}
	return FALSE;
}

/*[]----------------------------------------------------------------------[]*
 *|             slave_goto_idle() 		    	                           |*
 *[]----------------------------------------------------------------------[]*
 *|		phase change to IDLE                     						   |*
 *[]----------------------------------------------------------------------[]*/
void	slave_goto_idle( void )
{

	// �A�C�h����Ԃֈڍs
	NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_IDOL;

	// �ʐM����Ď��^�C�}�[�ĊJ
	NTComComm_Ctrl_Slave.timer_100m.count = NTCom_InitData.Time_LineMonitoring * 10;

	// 10m�^�C�}�[��~
	NTComComm_Ctrl_Slave.timer_10m.count = -1;

//	NTCom_Com_Status(NT_LED_IDLE);

}


/*[]----------------------------------------------------------------------[]*
 *|             data_recived()												|*
 *[]----------------------------------------------------------------------[]*
 *|			                       		   |*
 *[]----------------------------------------------------------------------[]*/
void slave_data_received( void ) {

	switch (NTComComm_Ctrl_Slave.status) {
	case NT_COM_SLAVE_IDOL:				/* �A�C�h�� (ENQ�҂�)*/
		slave_idol_prosess();
		break;
	case NT_COM_SLAVE_STX_SEND:			/* STX���M�� */
		slave_stx_send();
		break;
	case NT_COM_SLAVE_ACK_SEND:			/* ACK���M�� */
		slave_ack_send();
		break;
	case NT_COM_SLAVE_BROAD_CAST_WAIT:	/* �����M�҂� */
		slave_broad_cast();
		break;
	default:
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|				slave_idol_prosess()												|*
 *[]-----------------------------------------------------------------------[]*
 *|		�]�Ǐ�ԁ@�A�C�h�����̏���											|*
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture											|*
 *[]-----------------------------------------------------------------------[]*/
void slave_idol_prosess( void ) {

	uchar send_enable;
	uchar terminal_status;

	switch (NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]) {
	case ENQ:

		// ����d��CRC�`�F�b�N
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		// ENQ����M�ł����̂ŒʐM����Ď��^�C�}�[���Ƃ߂�
		NTComComm_Ctrl_Slave.timer_100m.count = -1;

		if (NTComComm_Ctrl_Slave.receive_packet_crc_status == FALSE) {
			// CRC�G���[��ENQ����M�����̂Ŗ����B
			return;
		}

		if (NTComComm_Ctrl_Slave.master_status == NT_ABNORMAL) {
			//�[����ԃA�u�m�[�}���̂Ƃ��@ENQ����������m�[�}���ɖ߂�
			// �ʐM�s�ǉ���
			NTCom_err_chk(NT_ERR_NO_RESPONSE, 0, 0, 0, NULL);
			// �[���X�e�[�^�X�ʏ�
			NTComData_SetTerminalStatus(NTComComm_Ctrl_Slave.terminal_num, NT_NORMAL, &normal_telegram, &prior_telegram);
			// �[����ԕύX
			NTComComm_Ctrl_Slave.master_status = NT_NORMAL;
		}

		NTComComm_Ctrl_Slave.cur_seq_no = NTComSlave_receive_blk->data[NT_OFS_LINK_NO];	// �V�[�P���X�i���o�[�擾

		if (NTComSlave_receive_blk->data[NT_OFS_TERMINAL_NO] == 0xFF) {
			// ����Z���N�e�B���O
			NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_BROAD_CAST_WAIT;
			// t6�Ń^�C�}�[�N��
			NTComComm_Ctrl_Slave.timeout_kind = 6;
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		} else {
			// ����ȊO

			if (NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG] != 0x00) {
				// ���M�v���t���O��0�łȂ��̂ŃZ���N�e�B���O
				NTComComm_Ctrl_Slave.block_send_count = NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG];	// ���M�u���b�N�����擾
				NTComComm_Ctrl_Slave.block_receive_count = 0;
			}

			send_enable = slave_check_send_data();
			if (send_enable) {	// ���M�\�ȃf�[�^�����邩�H�@��ǂ���M�\���H

				NTComComm_Ctrl_Slave.send_retry_count = 0;			// �đ��M�񐔃N���A

				// ���M�\
				if (send_enable == SEND_ENABLE_PRIOR) {
					// �D��f�[�^���M
					NTComData_GetSendBlock(&prior_telegram, NTComSlave_send_blk);
					// �đ��p�ɍŌ�ɑ������d���̏���ۑ����Ă���
					last_telegram = &prior_telegram;
				} else if (send_enable == SEND_ENABLE_NORMAL) {
					// �ʏ�f�[�^���M
					NTComData_GetSendBlock(&normal_telegram, NTComSlave_send_blk);
					// �đ��p�ɍŌ�ɑ������d���̏���ۑ����Ă���
					last_telegram = &normal_telegram;
				}
//	#001	��
				NTComComm_Ctrl_Slave.last_terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
																				, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
																				, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	��	#001

				if ((NTComComm_Ctrl_Slave.last_terminal_status & 0xFF) == 0) {	// ���ǂ���M�\���H	#004
					NTComComm_create_stx_telegram(NTComSlave_send_blk
									  , NT_DIRECTION_TO_MASTER
									  , NTComComm_Ctrl_Slave.terminal_num
									  , NTComComm_Ctrl_Slave.cur_seq_no
									  , NTComComm_Ctrl_Slave.last_terminal_status
									  , NTComComm_Ctrl_Slave.broad_cast_received
									  , NT_RES_NORMAL);
				} else {
					NTComComm_create_stx_telegram(NTComSlave_send_blk
									  , NT_DIRECTION_TO_MASTER
									  , NTComComm_Ctrl_Slave.terminal_num
									  , NTComComm_Ctrl_Slave.cur_seq_no
									  , NTComComm_Ctrl_Slave.last_terminal_status
									  , NTComComm_Ctrl_Slave.broad_cast_received
									  , NT_RES_BUFFER_FULL);
				}
				NTComComm_Ctrl_Slave.broad_cast_received = NT_NOT_BROADCASTED;

				NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_STX_SEND;
				NTComComm_Ctrl_Slave.timeout_kind = 3;
			} else {
				// ���M�s��
				if (NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG] == 0) {
					// �|�[�����O
					// EOT�d������
					NTComComm_create_eot_telegram(NTComSlave_send_blk
									  , NT_DIRECTION_TO_MASTER
									  , NTComComm_Ctrl_Slave.terminal_num
									  , NTComComm_Ctrl_Slave.cur_seq_no
//	#001	��
									  , slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
																  , NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
																  , NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE])
//	��	#001
									  , NTComComm_Ctrl_Slave.broad_cast_received
									  , NT_RES_NORMAL);

					NTComComm_Ctrl_Slave.broad_cast_received = NT_NOT_BROADCASTED;

				} else {
					// �Z���N�e�B���O
//	#001	��
					terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
																, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
																, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	��	#001
					if ((terminal_status & 0xFF) == 0) {	// ���ǂ���M�\���H	#004

						// ��M�\
						// ACK�d������
						NTComComm_create_ack_telegram(NTComSlave_send_blk
										  , NT_DIRECTION_TO_MASTER
										  , NTComComm_Ctrl_Slave.terminal_num
										  , NTComComm_Ctrl_Slave.cur_seq_no
										  , terminal_status
										  , NTComComm_Ctrl_Slave.broad_cast_received
										  , NT_RES_NORMAL);

						NTComComm_Ctrl_Slave.broad_cast_received = NT_NOT_BROADCASTED;

						// ACK���M��ֈڍs
						NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_ACK_SEND;
						NTComComm_Ctrl_Slave.timeout_kind = 3;
					} else {
						// ��M�s�\
						// EOT�d������
						NTComComm_create_eot_telegram(NTComSlave_send_blk
										  , NT_DIRECTION_TO_MASTER
										  , NTComComm_Ctrl_Slave.terminal_num
										  , NTComComm_Ctrl_Slave.cur_seq_no
										  , terminal_status
										  , NTComComm_Ctrl_Slave.broad_cast_received
										  , NT_RES_BUFFER_FULL);

						NTComComm_Ctrl_Slave.broad_cast_received = NT_NOT_BROADCASTED;

					}
				}
			}
			// �d�����M
			NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
			if (NTComComm_Ctrl_Slave.timeout_kind == 3) {
				slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
			} else {
				// �A�C�h����Ԃֈڍs
				slave_goto_idle();
			}
		}
		break;
	case ACK:
	case STX:
	case NAK:
	case EOT:
	default:
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|				slave_wait_ack()												|*
 *[]-----------------------------------------------------------------------[]*
 *|		�]�Ǐ�ԁ@STX���M��̏���											|*
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture											|*
 *[]-----------------------------------------------------------------------[]*/
void slave_stx_send( void ) {

	uchar terminal_status;
	uchar response_detail;

	switch (NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]) {
	case ENQ:

		// ����d��CRC�`�F�b�N
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		if (NTComSlave_receive_blk->data[NT_OFS_TERMINAL_NO] == 0xFF) {		// ����ENQ�H
			// ����ENQ
			// ����҂��֏�Ԃ��ڍs
			NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_BROAD_CAST_WAIT;
			NTComComm_Ctrl_Slave.cur_seq_no = NTComSlave_receive_blk->data[NT_OFS_LINK_NO];	// �V�[�P���X�i���o�[�擾
			// t6�Ń^�C�}�[�N��
			NTComComm_Ctrl_Slave.timeout_kind = 6;
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		} else {
			// �ʏ�ENQ
			// EOT�d������
//	#001	��
			terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
														, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
														, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	��	#001
			NTComComm_create_eot_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComSlave_receive_blk->data[NT_OFS_LINK_NO]
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , NT_RES_SEQUENCE_ERR);

			// �A�C�h����Ԃֈڍs
			slave_goto_idle();
			NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);

		}
		break;
	case ACK:

		// ����d��CRC�`�F�b�N
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		// �����ڍׂ��m�F����B
		slave_check_response_detail();

		// �A�C�h����Ԃֈڍs
		slave_goto_idle();
		break;
	case STX:

		// ���O�ɑ��M�����[���X�e�[�^�X�Ɣ�r����B
		if (NTComComm_Ctrl_Slave.last_terminal_status & NT_ALL_BUFFER_FULL) {
			// ���O�ɒ[���X�e�[�^�X�t���ŕԂ����̂�STX������ꂽ�B
//	#001	��
			terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
														, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
														, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	��	#001
			// EOT81�ŕԂ��B
			NTComComm_create_eot_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComSlave_receive_blk->data[NT_OFS_LINK_NO]
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , NT_RES_BUFFER_FULL);

			// �A�C�h����Ԃֈڍs
			slave_goto_idle();
		} else {
			// ���O�ɒ[���X�e�[�^�X�t���ȊO
			// �����ڍׂ��m�F����B
			slave_check_response_detail();

			response_detail = slave_check_receive_data();
//	#001	��
			terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
														, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
														, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);

//	��	#001
			if (response_detail == NT_RES_CRC_ERR) {
				NTComComm_create_nak_telegram(NTComSlave_send_blk
								  , NT_DIRECTION_TO_MASTER
								  , NTComComm_Ctrl_Slave.terminal_num
								  , NTComComm_Ctrl_Slave.cur_seq_no
								  , terminal_status
								  , NT_NOT_BROADCASTED
								  , response_detail);
				// NAK��Ԃ����Ƃ��͏�ԑJ�ڂ��Ȃ��B
			} else {
				NTComComm_create_ack_telegram(NTComSlave_send_blk
								  , NT_DIRECTION_TO_MASTER
								  , NTComComm_Ctrl_Slave.terminal_num
								  , NTComComm_Ctrl_Slave.cur_seq_no
								  , terminal_status
								  , NT_NOT_BROADCASTED
								  , response_detail);
				if ((NTComComm_Ctrl_Slave.block_receive_count < NTComComm_Ctrl_Slave.block_send_count)		// ���M�v���t���O���H
				 && (response_detail != NT_RES_BLOCKNO_ERR1)) {											// �u���b�N�����łȂ���Ύ�M�����`�F�b�N�@#004
					// ���M�v���t���O������
					// STX��Ƀf�[�^����M�����̂�ACK���M��ɑJ��
					NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_ACK_SEND;
					NTComComm_Ctrl_Slave.timeout_kind = 3;
				} else {
					// ���M�v���t���O���ɓ��B
					// �A�C�h����Ԃֈڍs
					slave_goto_idle();
				}
			}
		}
		NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
		if (NTComComm_Ctrl_Slave.timeout_kind == 3) {
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		}

		break;
	case NAK:

		// ����d��CRC�`�F�b�N
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		if (NTComSlave_receive_blk->data[NT_OFS_RESPONSE_DETAIL] == NT_RES_CRC_ERR) {
			// NAK�@���@0x80
			NTComComm_Ctrl_Slave.send_retry_count++;			// �đ��M�񐔃C���N�������g
			if (NTComComm_Ctrl_Slave.send_retry_count > (NTCom_InitData.Retry + 3)) {
				// ACK01�������Ȃ��̂ŁA�G���h���X�h�~�̂��ߋ����I���
				NTCom_err_chk(NT_ERR_RETRY_OVER, 2, 0, 0, NULL);
				NTComData_DeleteTelegram(last_telegram);
				NTComComm_Ctrl_Slave.send_retry_count = 0;			// �đ��M�񐔃N���A
				NTComComm_Ctrl_Slave.ack02_receive_count = 0;		// �d�����폜�����̂�ACK02�̐����N���A	#002
				slave_goto_idle();		//#004
			} else {
				// �u���b�N�Ď擾
				NTComData_GetSendBlock(last_telegram, NTComSlave_send_blk);
				// �d���Đ���
				NTComComm_create_stx_telegram(NTComSlave_send_blk
								  , NT_DIRECTION_TO_MASTER
								  , NTComComm_Ctrl_Slave.terminal_num
								  , NTComComm_Ctrl_Slave.cur_seq_no
//	#001	��
								  , slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
															  , NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
															  , NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE])
//	��	#001
								  , NTComComm_Ctrl_Slave.broad_cast_received
								  , NT_RES_NORMAL);
				// �đ��M
				NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
				NTComComm_Ctrl_Slave.timeout_kind = 3;
				slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
			}
		}
		break;
	case EOT:
		// ����d��CRC�`�F�b�N
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);
		// �A�C�h����
		slave_goto_idle();

		break;
	default:
		// �A�C�h����
		slave_goto_idle();

		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|				slave_wait_stx()											|*
 *[]-----------------------------------------------------------------------[]*
 *|		�]�Ǐ�ԁ@ACK���M��̏���											|*
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture											|*
 *[]-----------------------------------------------------------------------[]*/
void slave_ack_send( void ) {

	uchar terminal_status;
	uchar response_detail;

	switch (NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]) {
	case ENQ:

		// ����d��CRC�`�F�b�N
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		if (NTComSlave_receive_blk->data[NT_OFS_TERMINAL_NO] == 0xFF) {		// ����ENQ�H
			// ����ENQ
			// ����҂��֏�Ԃ��ڍs
			NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_BROAD_CAST_WAIT;
			NTComComm_Ctrl_Slave.cur_seq_no = NTComSlave_receive_blk->data[NT_OFS_LINK_NO];	// �V�[�P���X�i���o�[�擾
			// t6�Ń^�C�}�[�N��
			NTComComm_Ctrl_Slave.timeout_kind = 6;
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		} else {
			// �ʏ�ENQ
			// EOT�d������
//	#001	��
			terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
														, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
														, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	��	#001
			NTComComm_create_eot_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComSlave_receive_blk->data[NT_OFS_LINK_NO]
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , NT_RES_SEQUENCE_ERR);
			NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);

			// �A�C�h����Ԃֈڍs
			slave_goto_idle();
		}
		break;
	case STX:
	// �O��ACK�@STX���M���̃o�b�t�@��Ԃ��K�v�@�O��o�b�t�@�t���������疳������EOT81
		response_detail = slave_check_receive_data();
//	#001	��
		terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
													, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
													, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	��	#001

		if (response_detail == NT_RES_CRC_ERR) {
			NTComComm_create_nak_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComComm_Ctrl_Slave.cur_seq_no
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , response_detail);
		}
		 else {
			NTComComm_create_ack_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComComm_Ctrl_Slave.cur_seq_no
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , response_detail);
		}

		if ((NTComComm_Ctrl_Slave.block_receive_count < NTComComm_Ctrl_Slave.block_send_count)
		 && ((response_detail == NT_RES_NORMAL) || (response_detail == NT_RES_BLOCKNO_ERR2) || response_detail == NT_RES_CRC_ERR)) {		// ���M�v���t���O���H	#004

			// ���M�v���t���O������
			// STX��Ƀf�[�^����M�����̂�ACK���M��ɑJ��
			NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_ACK_SEND;
			NTComComm_Ctrl_Slave.timeout_kind = 3;
		} else {
			// ���M�v���t���O���ɓ��B
			// �A�C�h����Ԃֈڍs
			slave_goto_idle();
		}
		NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
		if (NTComComm_Ctrl_Slave.timeout_kind == 3) {
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		}

		break;
	case ACK:
	case NAK:

		// ����d��CRC�`�F�b�N
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		// EOT�𑗐M��
//	#001	��
		terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
												, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
												, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	��	#001
		NTComComm_create_eot_telegram(NTComSlave_send_blk
						  , NT_DIRECTION_TO_MASTER
						  , NTComComm_Ctrl_Slave.terminal_num
						  , NTComComm_Ctrl_Slave.cur_seq_no
						  , terminal_status
						  , NT_NOT_BROADCASTED
						  , NT_RES_SEQUENCE_ERR);

		NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
		// �A�C�h����
		slave_goto_idle();
		break;
	case EOT:

		// ����d��CRC�`�F�b�N
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

	default:
		// �A�C�h����
		slave_goto_idle();
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|				slave_broad_cast()											|*
 *[]-----------------------------------------------------------------------[]*
 *|		�]�Ǐ�ԁ@����҂��̏���											|*
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture											|*
 *[]-----------------------------------------------------------------------[]*/
void slave_broad_cast( void ) {
	switch (NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]) {
	case STX:
		if (NTComComm_Ctrl_Slave.receive_packet_crc_status == FALSE) {
			// CRC�G���[�̃f�[�^����M�����̂Ŗ����B
			NTComComm_Ctrl_Slave.broad_cast_received = NT_BROADCASTED_CRC_ERR;		// ����f�[�^CRC�G���[
			// �A�C�h����
			slave_goto_idle();
		} else {
			// ��M�o�b�t�@�Ƀf�[�^���Z�b�g����B
			switch (NTComData_SetRcvBlock(NTComSlave_receive_blk)) {
			case NT_DATA_NORMAL:						/* ���� */
				// ����f�[�^�����M
				NTComComm_Ctrl_Slave.broad_cast_received = NT_BROADCASTED;		// ����f�[�^��M�ς�
				break;
			case NT_DATA_BLOCK_INVALID1:				/* ACK02 or NT_NORMAL_BLKNUM_MAX�҂��Ă��ŏI�u���b�N�����Ȃ��ꍇ */
			case NT_DATA_BLOCK_INVALID2:				/* ACK03 */
			case NT_DATA_BUFFERFULL:				/* �D��o�b�t�@����t�ŃZ�b�g�ł��Ȃ� */
			default:
				break;
			}
			// �A�C�h����Ԃֈڍs
			slave_goto_idle();
		}
		break;
	case ENQ:
	case ACK:
	case NAK:
	case EOT:
		// ����d��CRC�`�F�b�N
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);
	default:
		// �A�C�h����
		slave_goto_idle();
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|			slave_check_send_data()
 *[]-----------------------------------------------------------------------[]*
 *|				��ǂɃf�[�^���M�\���`�F�b�N����
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
uchar slave_check_send_data( void ) {

	// ��ǗD��f�[�^��M�\�H
	if ((NTComSlave_receive_blk->data[NT_OFS_TERMINAL_STS] & NT_PRIOR_BUFFER_FULL) == 0) {
		// �ŏ��ɗD��f�[�^�̃e���O�������`�F�b�N����
		if (NTComData_IsValidTelegram(&prior_telegram)) {
			// �e���O�������L����������A�D��f�[�^����ŕԂ��
			return SEND_ENABLE_PRIOR;
		} else {
			// �e���O������������������Peek���Ă݂�
			if (NTComData_PeekSndTele_Prior(NTComComm_Ctrl_Slave.terminal_num, &prior_telegram)) {	/* #003 */
				// �߂�l���L����������D��f�[�^����ŕԂ��
				return SEND_ENABLE_PRIOR;
			}
		}

	}
	// ��ǒʏ�f�[�^��M�\�H
	if ((NTComSlave_receive_blk->data[NT_OFS_TERMINAL_STS] & NT_NORMAL_BUFFER_FULL) == 0) {
		// �D��f�[�^���Ȃ�������A�ʏ�f�[�^���`�F�b�N����
		if (NTComData_IsValidTelegram(&normal_telegram)) {
			// �e���O�������L����������A�ʏ�f�[�^����ŕԂ��
			return SEND_ENABLE_NORMAL;
		} else {
			// �e���O������������������APeek���Ă݂�
			if (NTComData_PeekSndTele_Normal(NTComComm_Ctrl_Slave.terminal_num, &normal_telegram)) {
				// �߂�l���L����������ʏ�f�[�^����ŕԂ��
				return SEND_ENABLE_NORMAL;
			}
		}
	}

	// �ǂ��ɂ�����������Ȃ�������A�f�[�^�Ȃ��ŕԂ��
	return SEND_DISABLE;

}

/*[]-----------------------------------------------------------------------[]*
 *|			slave_get_terminal_status()
 *[]-----------------------------------------------------------------------[]*
 *|				�]�ǃ^�[�~�i���X�e�[�^�X���擾
 *|	param		tele_kind		�d�����
 *|	param		send_req_flag	���M�v���t���O
 *|	return
 *[]-----------------------------------------------------------------------[]*/
uchar slave_get_terminal_status( uchar tele_kind, uchar send_req_flag, uchar packet_mode ) {

	uchar ret = 0;
	T_NT_BUF_STATUS buf_status;
	T_NT_BUF_COUNT bufcnt;

	NTComData_GetBufferStatus(&buf_status, NTComComm_Ctrl_Slave.terminal_num);
	NTComData_GetRcvBufferCount(&bufcnt);

	if (tele_kind == ENQ) {
		if (packet_mode == NT_PRIOR_DATA) {
			// �D��f�[�^ENQ
			if (buf_status.prior_r == TRUE		// �d�������݂���B
			 || bufcnt.prior != 0) {			// �o�b�t�@�g�p�ʂ��O�łȂ��B
				ret |= NT_PRIOR_BUFFER_FULL;
			}
		} else {
			// �ʏ�f�[�^ENQ
			if (buf_status.normal_r == TRUE		// �d�������݂���B
			 || send_req_flag + bufcnt.normal > NT_NORMAL_BLKNUM_MAX) {	// �g�p�ʁ@�{�@���M�ʁ@���@Max
				ret |= NT_NORMAL_BUFFER_FULL;
			}
		}
		// ENQ�̎��͒ʏ�f�[�^�̑��M�v���t���O�ƁA��M�o�b�t�@�g�p�ʂ��r����
	} else {
		// ENQ�ȊO�̎��̓o�b�t�@�̋󂫂��Ď��
		if (buf_status.prior_r == TRUE) {	// �d�������݂���B
			ret |= NT_PRIOR_BUFFER_FULL;
		}
		if (buf_status.normal_r == TRUE) {	// �d�������݂���B
			ret |= NT_NORMAL_BUFFER_FULL;
		}
	}

	return ret;
}
/*[]-----------------------------------------------------------------------[]*
 *|			int_com_slave_timer_10m()
 *[]-----------------------------------------------------------------------[]*
 *|				1m sec �^�C�}�[���荞�ݏ���
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void int_com_slave_timer_10m(void) {

	if (NTComComm_Ctrl_Slave.timer_10m.count > 0) {
		NTComComm_Ctrl_Slave.timer_10m.count--;
	}

}
/*[]-----------------------------------------------------------------------[]*
 *|			int_com_slave_timer_100m()
 *[]-----------------------------------------------------------------------[]*
 *|				100m sec �^�C�}�[���荞�ݏ���
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void int_com_slave_timer_100m(void) {

	if (NTComComm_Ctrl_Slave.timer_100m.count > 0) {
		NTComComm_Ctrl_Slave.timer_100m.count--;
	}

}
/*[]-----------------------------------------------------------------------[]*
 *|			int_com_slave_timer_100m()
 *[]-----------------------------------------------------------------------[]*
 *|				100m sec �^�C�}�[���荞�ݏ���
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void slave_start_timer(uchar timer_kind) {

	switch (timer_kind) {
	case 1:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t1);
		break;
	case 2:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t2);
		break;
	case 3:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t3);
		break;
	case 4:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t4);
		break;
	case 5:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t5);
		break;
	case 6:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal((NTCom_InitData.Time_t6 + NTCom_InitData.Time_t7));
		break;
	case 7:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t7);
		break;
	case 8:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t8);
		break;
	case 9:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t9);
		break;
	default:
		break;
	}
	NTComTimer_Start(NTComComm_Ctrl_Slave.timer_10m.timerid);
}


/*[]-----------------------------------------------------------------------[]*
 *|			slave_check_response_detail()
 *[]-----------------------------------------------------------------------[]*
 *|				STX���M��̓d���̉����ڍׂ��`�F�b�N���āA�d�������������B�������`�F�b�N����
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void slave_check_response_detail () {

	switch (NTComSlave_receive_blk->data[NT_OFS_RESPONSE_DETAIL]) {
	case NT_RES_NORMAL:
		if (last_telegram->kind == NT_DATA_KIND_PRIOR_SND) {
			// �D��f�[�^�͂P�u���b�N�̂͂��Ȃ̂ŁA���ꂽ��폜����B
			NTComData_DeleteTelegram(last_telegram);
			NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
		} else {
			// ACK����M�����̂Ŏ��̃u���b�N�ɐi�߂�B
			NTComData_SeekBlock(last_telegram, 1, NT_SEEK_CUR);

			// 1�u���b�N����ɑ��M����
			if (_NTComData_GetRemainBlkNum(last_telegram) < 1) {		// �d���ɖ����M�u���b�N�͂��邩
				// �����M�u���b�N�Ȃ�
				// 1�d�����M�����A���M�o�b�t�@����d���폜
				NTComData_DeleteTelegram(last_telegram);
				NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
			}
		}
		break;
	case NT_RES_CRC_RETRY_OVER:
		// CRC�G���[�Ń��g���C�I�[�o�[�A���M�d�����폜�B
		NTComData_DeleteTelegram(last_telegram);
		NTCom_err_chk(NT_ERR_ACK01, 2, 0, 0, NULL);
		NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
		break;
	case NT_RES_BLOCKNO_ERR1:
		// �u���b�N�A�ԃG���[�Ŏ�Ǒ��ő��M�f�[�^�S�u���b�N�폜
		NTComComm_Ctrl_Slave.ack02_receive_count++;
		if (NTComComm_Ctrl_Slave.ack02_receive_count >= NTCom_InitData.Retry) {
			// ���g���C�񐔕���M�����̂ŁA���M�d���j��
			NTComData_DeleteTelegram(last_telegram);
			NTCom_err_chk(NT_ERR_ACK02, 2, 0, 0, NULL);
			NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
		} else {
			// ����d���擪����đ��M
			NTComData_SeekBlock(last_telegram, 0, NT_SEEK_SET);
		}
		break;
	case NT_RES_BLOCKNO_ERR2:
		if (last_telegram->kind == NT_DATA_KIND_PRIOR_SND) {
			// �D��f�[�^�͂P�u���b�N�̂͂��Ȃ̂ŁA���ꂽ��폜����B
			NTComData_DeleteTelegram(last_telegram);
			NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
		} else {
			// ���񑗂����d���͗L���Ȃ̂Ŏ��̃u���b�N�ɐi�߂�B
			NTComData_SeekBlock(last_telegram, 1, NT_SEEK_CUR);

			// 1�u���b�N�ɑ��M����
			if (_NTComData_GetRemainBlkNum(last_telegram) < 1) {		// �d���ɖ����M�u���b�N�͂��邩
				// �����M�u���b�N�Ȃ�
				// 1�d�����M�����A���M�o�b�t�@����d���폜
				NTComData_DeleteTelegram(last_telegram);
				NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
			}
		}
	default:
		break;
	}

}


/*[]-----------------------------------------------------------------------[]*
 *|			slave_check_receive_data()
 *[]-----------------------------------------------------------------------[]*
 *|				��M�����d���̐��������`�F�b�N����
 *|	param
 *|	return		�����ڍׂ�Ԃ�
 *[]-----------------------------------------------------------------------[]*/
uchar slave_check_receive_data () {

	uchar response_detail = NT_RES_NORMAL;
	uchar ret;

	// CRC�G���[���`�F�b�N
	if (NTComComm_Ctrl_Slave.receive_packet_crc_status == FALSE) {
		NTComComm_Ctrl_Slave.receive_crc_err_count++;
		// CRC�G���[
		if (NTComComm_Ctrl_Slave.receive_crc_err_count > NTCom_InitData.Retry) {// CRC�G���[���g���C�񐔈ȏ�H
			NTComComm_Ctrl_Slave.receive_crc_err_count = 0;
			// CRC�G���[���g���C�I�[�o�[		ACK01
			response_detail = NT_RES_CRC_RETRY_OVER;
			NTCom_err_chk(NT_ERR_CRC_DATA, 2, 0, 0, NULL);
		} else {
			// CRC���g���C�񐔖���				NAK80
			response_detail = NT_RES_CRC_ERR;
		}

	} else {
		// CRC�͐���
		ret = NTComData_SetRcvBlock(NTComSlave_receive_blk);
		switch (ret) {		// �d������M�o�b�t�@�ɃZ�b�g
		case NT_DATA_NORMAL:						/* ���� */
			// ACK�d��								ACK00
			response_detail = NT_RES_NORMAL;
			NTComComm_Ctrl_Slave.receive_crc_err_count = 0;	// ����Ɏ�M�ł����̂łO�ɂ���B
			NTComComm_Ctrl_Slave.block_receive_count++;	// ��M�u���b�N��
			break;
		case NT_DATA_BLOCK_INVALID1:				/* ACK02 or NT_NORMAL_BLKNUM_MAX�҂��Ă��ŏI�u���b�N�����Ȃ��ꍇ */
			// ACK02
			response_detail = NT_RES_BLOCKNO_ERR1;
			NTComComm_Ctrl_Slave.block_receive_count++;	// ��M�u���b�N��
			NTCom_err_chk(NT_ERR_BLK_NT, 2, 0, 0, NULL);
			break;
		case NT_DATA_BLOCK_INVALID2:				/* ACK03 */
			// ACK03
			response_detail = NT_RES_BLOCKNO_ERR2;
			NTComComm_Ctrl_Slave.block_receive_count++;	// ��M�u���b�N��
			break;
		case NT_DATA_BUFFERFULL:				/* �ʏ�o�b�t�@����t�ŃZ�b�g�ł��Ȃ� */
			// ��{�I�ɂ��肦�Ȃ�
			response_detail = NT_RES_BLOCKNO_ERR1;
			NTCom_err_chk(NT_ERR_BLK_OVER_NT, 2, 0, 0, NULL);
			break;
		case NT_DATA_NO_MORE_TELEGRAM:			/* ���łɂP�d�����݂��Ă��� */
			// ��{�I�ɂ��肦�Ȃ�
			response_detail = NT_RES_BUFFER_FULL;
			break;
		default:
			break;
		}
	}

	return response_detail;
}


/*[]-----------------------------------------------------------------------[]*
 *|			slave_check_crc_ctrl_tele()
 *[]-----------------------------------------------------------------------[]*
 *|			����d����CRC�`�F�b�N���s���
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void slave_check_crc_ctrl_tele( BOOL bCrc ) {

	if (bCrc) {
		NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl = 0;
	} else {
		if (NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl < NTCom_InitData.Retry) {
			NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl++;
		}
		if (NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl >= NTCom_InitData.Retry) {
			NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl = 0;
			NTCom_err_chk(NT_ERR_CRC_CODE, 2, 0, 0, NULL);
		}
	}
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
