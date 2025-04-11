//[]----------------------------------------------------------------------[]
///	@file		can_api.c
///	@brief		CAN�h���C�oAPI
/// @date		2012/01/19
///	@author		m.onouchi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

/*--- Include -------------------*/
#include <string.h>
#include <machine.h>
#include "config_can_api.h"
#include "iodefine.h"
#include "can_api.h"
#include	"can_def.h"

/*--- Pragma --------------------*/


/*--- Prototype -----------------*/
static void CanClearSentData(const unsigned long ch_nr, const unsigned long mbox_nr);
static unsigned long R_CAN_WaitTxRx(const unsigned long ch_nr, const unsigned long mbox_nr);
static void R_CAN_ConfigCANinterrupts(const unsigned long ch_nr);

/*--- Define --------------------*/
// �n�[�h�E�F�A�ɋN�������Q�ł̃��b�N��������邽�߂̃^�C�}
#define DEC_CHK_CAN_SW_TMR			(--can_tmo_cnt != 0)
#define RESET_CAN_SW_TMR			(can_tmo_cnt = MAX_CAN_SW_DELAY);
#define MAX_CAN_SW_DELAY			(0x8000)
#define CHECK_MBX_NR				{if (mbox_nr > 31) return R_CAN_SW_BAD_MBX;}

// Board specific port defines.
#define CAN_TRX_PDR(x, y)			CAN_TRX_PDR_PREPROC(x, y)
#define CAN_TRX_PDR_PREPROC(x, y)	(PORT ## x .PDR.BIT.B ## y)
#define CAN_TRX_PODR(x, y)			CAN_TRX_PODR_PREPROC(x, y)
#define CAN_TRX_PODR_PREPROC(x, y)	(PORT ## x .PODR.BIT.B ## y)

/*--- Gloval Value --------------*/


/*--- Extern --------------------*/


//[]----------------------------------------------------------------------[]
///	@brief			CAN�y���t�F����������
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@return			R_CAN_OK				: ����I��<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_BAD_ACTION_TYPE:	: ����`�̐ݒ�w��<br>
///					R_CAN_SW_RST_ERR		: CAN�y���t�F���������Z�b�g���[�h�ɂȂ�Ȃ�<br>
///					R_CAN_SW_WAKEUP_ERR		: CAN�y���t�F�������E�F�C�N�A�b�v���Ȃ�<br>
///					R_CAN_SW_SLEEP_ERR		: CAN�y���t�F�������X���[�v���[�h�ɂȂ���<br>
///					R_CAN_SW_HALT_ERR		: CAN�y���t�F�������z���g���[�h�ɂȂ�Ȃ�
///	@author			m.onouchi
///	@note			�r�b�g���[�g�C�}�X�N�C���[���{�b�N�X�̃f�t�H���g�CCAN ���荞�݂�ݒ肷��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_Create(const unsigned long ch_nr)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long i, j;
	unsigned long can_tmo_cnt = MAX_CAN_SW_DELAY;

	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Exit Sleep mode. */
	api_status |= R_CAN_Control(ch_nr, EXITSLEEP_CANMODE);	// �X���[�v���[�h���畜�A

	/* Sleep -> RESET mode. */
	api_status |= R_CAN_Control(ch_nr, RESET_CANMODE);		// ���Z�b�g���[�h�ֈڍs

	/*** Setting of CAN0 Control register.***/
	can_block_p->CTLR.BIT.BOM = 0;							// �o�X�I�t���A���[�h:�m�[�}��(ISO11898-1�d�l����)
	can_block_p->CTLR.BIT.MBM = 1;							// FIFO���[���{�b�N�X���[�h(FIFO���g�p����)
	can_block_p->CTLR.BIT.IDFM = 0;							// �W��ID���[�h(�g��ID���g�p���Ȃ�)
	can_block_p->CTLR.BIT.MLM = 1;							// ���ǎ��͎�M���Ȃ��i�I�[�o�[�������[�h�j
	can_block_p->CTLR.BIT.TPM = 0;							// ID�D�摗�M���[�h
	can_block_p->CTLR.BIT.TSRC = 0;							// �^�C���X�^���v�J�E���^�̃��Z�b�g�����Ȃ�
	can_block_p->CTLR.BIT.TSPS = 3;							// �^�C���X�^���v�v���X�P�[��:8�r�b�g�^�C������

	/* Set BAUDRATE */
	R_CAN_SetBitrate(ch_nr);								// �ʐM���x�ݒ�

	can_block_p->MKIVLR.LONG = 0xFFFFFFFF;					// �S���[���{�b�N�X:�}�X�N����
	can_block_p->TFCR.BIT.TFE = 0;							// ���MFIFO����bit 0:���MFIFO�֎~
	can_block_p->RFCR.BIT.RFE = 0;							// ��MFIFO����bit 0:��MFIFO�֎~

	/* Configure CAN interrupts. */ 
	R_CAN_ConfigCANinterrupts(ch_nr);						// CAN���荞�ݏ�����

	/* Reset -> HALT mode */
	api_status |= R_CAN_Control(ch_nr, HALT_CANMODE);		// �z���g���[�h�ֈڍs

	/* Configure mailboxes in Halt mode. */
	for (i = 0; i < 32; i++) {
		can_block_p->MB[i].ID.LONG = 0x00;
		can_block_p->MB[i].DLC = 0x00;
		for (j = 0; j < 8; j++) {
			can_block_p->MB[i].DATA[j] = 0x00;
		}
		for (j = 0; j < 2; j++) {
			can_block_p->MB[i].TS = 0x00;
		}
	}

	/* Halt -> OPERATION mode */
	/* Note: EST and BLIF flag go high here when stepping code in debugger. */
	api_status |= R_CAN_Control(ch_nr, OPERATE_CANMODE);	// �ʏ퓮�샂�[�h�ֈڍs

	/* Time Stamp Counter reset. Set the TSRC bit to 1 in CAN Operation mode. */
	can_block_p->CTLR.BIT.TSRC = 1;									// �^�C���X�^���v�J�E���^�̃��Z�b�g������B
	while ((can_block_p->CTLR.BIT.TSRC) && DEC_CHK_CAN_SW_TMR) {	// ���W�X�^�̕ω����m�F
		;
	}
	if (can_tmo_cnt == 0) {		// �^�C���A�E�g
		api_status |= R_CAN_SW_TSRC_ERR;
	}

	/* Check for errors so far, report, and clear. */
	if (can_block_p->STR.BIT.EST) {		// �G���[����
		api_status |= R_CAN_SW_RST_ERR;
	}

	/* Clear Error Interrupt Factor Judge Register. */
	if (can_block_p->EIFR.BYTE) {		// �o�X�G���[���o
										// �G���[���[�j���O���o
										// �G���[�p�b�V�u���o
										// �o�X�I�t�J�n���o
										// �o�X�I�t���A���o
										// ��M�I�[�o�������o
										// �I�[�o���[�h�t���[�����M���o
										// �o�X���b�N���o
		api_status |= R_CAN_SW_RST_ERR;
	}
	can_block_p->EIFR.BYTE = 0x00;		// �G���[���荞�ݗv���N���A

	/* Clear Error Code Store Register. */
	if (can_block_p->ECSR.BYTE) {		// �X�^�b�t�G���[���o
										// �t�H�[���G���[���o
										// ACK�G���[���o
										// CRC�G���[���o
										// �r�b�g�G���[(���Z�V�u)���o
										// �r�b�g�G���[(�h�~�i���g)���o
										// ACK�f���~�^�G���[���o
										// �~�ς����G���[�R�[�h���o��
		api_status |= R_CAN_SW_RST_ERR;
	}
	can_block_p->ECSR.BYTE = 0x00;		// �G���[�R�[�h�i�[���W�X�^�N���A

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN�g�����V�[�o�|�[�g�̒[�q�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		action_type				: ENABLE=�[�q�̗L����<br>
///											: DISABLE=�[�q�̖�����<br>
///											: CANPORT_TEST_LISTEN_ONLY=���b�X���I�������[�h<br>
///											: CANPORT_TEST_0_EXT_LOOPBACK=�Z���t�e�X�g���[�h0(�O�����[�v�o�b�N)<br>
///											: CANPORT_TEST_1_INT_LOOPBACK=�Z���t�e�X�g���[�h1(�������[�v�o�b�N)<br>
///											: CANPORT_RETURN_TO_NORMAL=�ʏ퓮�샂�[�h�ւ̕��A
///	@return			R_CAN_OK				: ����I��<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_BAD_ACTION_TYPE:	: ����`�̐ݒ�w��<br>
///					R_CAN_SW_RST_ERR		: CAN�y���t�F���������Z�b�g���[�h�ɂȂ�Ȃ�<br>
///					R_CAN_SW_HALT_ERR		: CAN�y���t�F�������z���g���[�h�ɂȂ�Ȃ�
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_PortSet(const unsigned long ch_nr, const unsigned long action_type)
{  
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	switch (action_type) {
	case ENABLE:
		/* P3_2 as CTX0 and P3_3 as CRX0. */
		PORT5.PDR.BIT.B4 = 1;				// P54�o�͐ݒ�(CTX1)
		PORT5.PDR.BIT.B5 = 0;				// P55���͐ݒ�(CRX1)
		break;

	case DISABLE:
		break;

	/* Run in Listen Only test mode. */
	case CANPORT_TEST_LISTEN_ONLY:
		api_status = R_CAN_Control(0, HALT_CANMODE);		// �z���g���[�h�ֈڍs
		can_block_p->TCR.BYTE = 0x03;						// CAN�e�X�g���[�h����
															// ���b�X���I�������[�h
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);	// �ʏ퓮�샂�[�h�ֈڍs
		api_status |= R_CAN_PortSet(0, ENABLE);				// �[�q�̗L����
		break;

	/* Run in External Loopback test mode. */
	case CANPORT_TEST_0_EXT_LOOPBACK:
		api_status = R_CAN_Control(0, HALT_CANMODE);		// �z���g���[�h�ֈڍs
		can_block_p->TCR.BYTE = 0x05;						// CAN�e�X�g���[�h����
															// �Z���t�e�X�g���[�h0(�O�����[�v�o�b�N)
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);	// �ʏ퓮�샂�[�h�ֈڍs
		api_status |= R_CAN_PortSet(0, ENABLE);				// �[�q�̗L����
		break;

	/* Run in Internal Loopback test mode. */
	case CANPORT_TEST_1_INT_LOOPBACK:
		api_status = R_CAN_Control(0, HALT_CANMODE);		// �z���g���[�h�ֈڍs
		can_block_p->TCR.BYTE = 0x07;						// CAN�e�X�g���[�h����
															// �Z���t�e�X�g���[�h1(�������[�v�o�b�N)
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);	// �ʏ퓮�샂�[�h�ֈڍs
		break;

	/* Return to default CAN bus mode. */
	case CANPORT_RETURN_TO_NORMAL:
		api_status = R_CAN_Control(0, HALT_CANMODE);		// �z���g���[�h�ֈڍs
		can_block_p->TCR.BYTE = 0x00;						// CAN�e�X�g���[�h�֎~
															// CAN�e�X�g���[�h�ł͂Ȃ�
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);	// �ʏ퓮�샂�[�h�ֈڍs
		api_status |= R_CAN_PortSet(0, ENABLE);				// �[�q�̗L����
		break;

	default:	// ����`�̐ݒ�w��
		api_status = R_CAN_BAD_ACTION_TYPE;
		break;
	}
	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN���샂�[�h�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		action_type				: EXITSLEEP_CANMODE=�X���[�v���[�h���畜�A<br>
///											: ENTERSLEEP_CANMODE=�X���[�v���[�h�ֈڍs<br>
///											: RESET_CANMODE=���Z�b�g���[�h�ֈڍs<br>
///											: HALT_CANMODE=�z���g���[�h�ֈڍs<br>
///											: OPERATE_CANMODE=�ʏ퓮�샂�[�h�ֈڍs
///	@return			R_CAN_OK				: ����I��<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_BAD_ACTION_TYPE:	: ����`�̐ݒ�w��<br>
///					R_CAN_SW_RST_ERR		: CAN�y���t�F���������Z�b�g���[�h�ɂȂ�Ȃ�<br>
///					R_CAN_SW_WAKEUP_ERR		: CAN�y���t�F�������E�F�C�N�A�b�v���Ȃ�<br>
///					R_CAN_SW_SLEEP_ERR		: CAN�y���t�F�������X���[�v���[�h�ɂȂ�Ȃ�<br>
///					R_CAN_SW_HALT_ERR		: CAN�y���t�F�������z���g���[�h�ɂȂ�Ȃ�
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_Control(const unsigned long ch_nr, const unsigned long action_type)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long can_tmo_cnt = MAX_CAN_SW_DELAY;
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	switch (action_type) {
	case EXITSLEEP_CANMODE:		// �X���[�v���[�h���畜�A
		can_block_p->CTLR.BIT.SLPM = CAN_NOT_SLEEP;						// CAN�X���[�v���[�h������
		while ((can_block_p->STR.BIT.SLPST) && DEC_CHK_CAN_SW_TMR) {	// �X�e�[�^�X���W�X�^�Ń��[�h�̕ω����m�F
			nop();
		}
		if (can_tmo_cnt == 0) {	// �^�C���A�E�g
			api_status = R_CAN_SW_WAKEUP_ERR;
		}
		break;

	case ENTERSLEEP_CANMODE:	// �X���[�v���[�h�ֈڍs
		can_block_p->CTLR.BIT.SLPM = CAN_SLEEP;							// CAN�X���[�v���[�h�ɕύX
		while ((!can_block_p->STR.BIT.SLPST) && DEC_CHK_CAN_SW_TMR) {	// �X�e�[�^�X���W�X�^�Ń��[�h�̕ω����m�F
			nop();
		}
		if (can_tmo_cnt == 0) {	// �^�C���A�E�g
			api_status = R_CAN_SW_SLEEP_ERR;
		}
		break;

	case RESET_CANMODE:			// ���Z�b�g���[�h�ֈڍs
		can_block_p->CTLR.BIT.CANM = CAN_RESET;							// CAN���Z�b�g���[�h�ɕύX
		while ((!can_block_p->STR.BIT.RSTST) && DEC_CHK_CAN_SW_TMR) {	// �X�e�[�^�X���W�X�^�Ń��[�h�̕ω����m�F
			;
		}
		if (can_tmo_cnt == 0) {	// �^�C���A�E�g
			api_status = R_CAN_SW_RST_ERR;
		}
		break;

	case HALT_CANMODE:			// �z���g���[�h�ֈڍs
		can_block_p->CTLR.BIT.CANM = CAN_HALT;							// CAN�z���g���[�h�ɕύX
		while ((!can_block_p->STR.BIT.HLTST) && DEC_CHK_CAN_SW_TMR) {	// �X�e�[�^�X���W�X�^�Ń��[�h�̕ω����m�F
			;
		}
		if (can_tmo_cnt == 0) {	// �^�C���A�E�g
			api_status = R_CAN_SW_HALT_ERR;
		}
		break;

	case OPERATE_CANMODE:		// �ʏ퓮�샂�[�h�ֈڍs
		MSTP_CAN1  =  0;												// CAN1�̃��W���[���X�g�b�v��Ԃ�����
		while ((MSTP_CAN1) && DEC_CHK_CAN_SW_TMR) {						// ���W�X�^�̕ω����m�F
			nop();
		}
		can_block_p->CTLR.BIT.CANM = CAN_OPERATION;						// CAN�I�y���[�V�������[�h�ɕύX

		while ((can_block_p->STR.BIT.RSTST) && DEC_CHK_CAN_SW_TMR) {	// �X�e�[�^�X���W�X�^�Ń��[�h�̕ω����m�F
			;
		}
		if (can_tmo_cnt == 0) {	// �^�C���A�E�g
			api_status = R_CAN_SW_RST_ERR;
		}
		break;

	default:	// ����`�̐ݒ�w��
		api_status = R_CAN_BAD_ACTION_TYPE;
		break;
	}

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			���M���[���{�b�N�X�̐ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		mbox_nr					: ���[���{�b�N�X�ԍ�(0-31)<br>
///					*frame_p				: �f�[�^�t���[��<br>
///					frame_type				: DATA_FRAME=�ʏ�̃f�[�^�t���[��<br>
///											: REMOTE_FRAME=�����[�g�f�[�^�t���[��
///	@return			R_CAN_OK				: ����I��<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_BAD_ACTION_TYPE:	: ����`�̐ݒ�w��<br>
///	@author			m.onouchi
///	@note			�w�肳�ꂽ�t���[�������[���{�b�N�X�ɏ�������R_CAN_Tx���R�[�����ăo�X�ɑ��M����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_TxSet(const unsigned long ch_nr, const unsigned long mbox_nr, const can_std_frame_t *frame_p, const unsigned long frame_type)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long i;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// ����mbox_nr�͈̔̓`�F�b�N
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait for any previous transmission to complete. */
	api_status = R_CAN_WaitTxRx(0, mbox_nr);				// �O�̑��M����������܂ő҂�

	/* Interrupt disable the mailbox.in case it was a receive mailbox */
	can_block_p->MIER.LONG &= ~(bit_set[mbox_nr]);			// ���[���{�b�N�X���荞�݋֎~

	/* Clear message mailbox control register (trmreq to 0). */
	can_block_p->MCTL[mbox_nr].BYTE = 0;					// ���b�Z�[�W���䃌�W�X�^�N���A

	/*** Set Mailbox. ***/
	/* Set CAN message mailbox buffer Standard ID */
	can_block_p->MB[mbox_nr].ID.BIT.SID = frame_p->id;		// ID�Z�b�g

	/* Set the Data Length Code */
	can_block_p->MB[mbox_nr].DLC = frame_p->dlc;			// �f�[�^���Z�b�g

 	/* Frame select: Data frame = 0, Remote = 1 */
	if (frame_type == REMOTE_FRAME) {
		can_block_p->MB[mbox_nr].ID.BIT.RTR = 1;			// �����[�g�t���[����I��
	} else {
		can_block_p->MB[mbox_nr].ID.BIT.RTR = 0;			// �f�[�^�t���[����I��
	}

	/* Frame select: Standard = 0, Extended = 1 */
	can_block_p->MB[mbox_nr].ID.BIT.IDE = 0;				// �W��ID

	/* Copy frame data into mailbox */
	for (i = 0; ((i < frame_p->dlc) && (i<8)); i++) {		// �f�[�^�R�s�[
		can_block_p->MB[mbox_nr].DATA[i] = frame_p->data[i];
	}

	/* Interrupt enable the mailbox */
	can_block_p->MIER.LONG |= (bit_set[mbox_nr]);			// ���[���{�b�N�X���荞�݋���

	R_CAN_Tx(ch_nr, mbox_nr);	// ���b�Z�[�W���M�w��

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			���b�Z�[�W���M�w��
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		mbox_nr					: ���[���{�b�N�X�ԍ�(0-31)<br>
///	@return			R_CAN_OK				: ����I��<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_SW_SET_TX_TMO:	: �O�̑��M�̏I���҂����^�C���A�E�g�����B<br>
///					R_CAN_SW_SET_RX_TMO:	: �O�̎�M�̏I���҂����^�C���A�E�g�����B
///	@author			m.onouchi
///	@note			�w�肳�ꂽ���[���{�b�N�X�Ƀt���[���̑��M���w������B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_Tx(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// ����mbox_nr�͈̔̓`�F�b�N
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait for any previous transmission to complete. */
	api_status = R_CAN_WaitTxRx(0, mbox_nr);		// �O�̑��M����������܂ő҂�

	/* Clear SentData flag since we are about to send anew. */
	CanClearSentData(ch_nr, mbox_nr);				// �O�f�[�^�̑��M��ԉ���
	
	/* Set TrmReq bit to "1" */
	can_block_p->MCTL[mbox_nr].BIT.TX.TRMREQ = 1;	// ���M�v��
	
 	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			���b�Z�[�W���M�����`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		mbox_nr					: ���[���{�b�N�X�ԍ�(0-31)<br>
///	@return			R_CAN_OK				: ���M����<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_MSGLOST			: ���b�Z�[�W�͏㏑�����ꂽ������ꂽ�B<br>
///					R_CAN_NO_SENTDATA		: ���b�Z�[�W�����M����Ȃ������B
///	@author			m.onouchi
///	@note			�w�肳�ꂽ���[���{�b�N�X����f�[�^�t���[�������M���ꂽ���Ƃ��m�F����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_TxCheck(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// ����mbox_nr�͈̔̓`�F�b�N
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	if (can_block_p->MCTL[mbox_nr].BIT.TX.SENTDATA == 0) {
		api_status = R_CAN_NO_SENTDATA;
	} else {								// ���M����
		/* Clear SentData flag. */
		CanClearSentData(ch_nr, mbox_nr);	// �O�f�[�^�̑��M��ԉ���
	}

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			���b�Z�[�W���M�̒��~(abort)
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		mbox_nr					: ���[���{�b�N�X�ԍ�(0-31)<br>
///	@return			R_CAN_OK				: ����I��<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_SW_ABORT_ERR		: �A�{�[�g�҂��^�C���A�E�g
///	@author			m.onouchi
///	@attention		�f�[�^�����ɑ��M�ς݂̏ꍇ��R_CAN_SW_ABORT_ERR���Ԃ�܂��B
///	@note			�w�肳�ꂽ���[���{�b�N�X�̃f�[�^���M�𒆎~����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_TxStopMsg(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long can_tmo_cnt = MAX_CAN_SW_DELAY;   
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// ����mbox_nr�͈̔̓`�F�b�N
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Clear message mailbox control register. Setting TRMREQ to 0 should abort. */
	can_block_p->MCTL[mbox_nr].BYTE = 0;			// ���b�Z�[�W���䃌�W�X�^�N���A

	/* Wait for abort. */
	while ((can_block_p->MCTL[mbox_nr].BIT.TX.TRMABT) && DEC_CHK_CAN_SW_TMR) {	// ���M�A�{�[�g�̊����҂�
		;
	}
	if (can_tmo_cnt == 0) {		// �^�C���A�E�g
		api_status = R_CAN_SW_ABORT_ERR;
	}

	/* Clear abort flag. */
	can_block_p->MCTL[mbox_nr].BIT.TX.TRMABT = 0;	// ���M�A�{�[�g�����t���O�N���A

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			�O�f�[�^�̑��M��ԉ���
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr		: �`���l���ԍ�(1)
///	@param[in]		mbox_nr		: ���[���{�b�N�X�ԍ�(0-31)
///	@return			void
///	@author			m.onouchi
///	@note			�w�肳�ꂽ���[���{�b�N�X�ɑ΂��Ď��̃t���[�����M���J�n�ł����Ԃɂ���B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#pragma inline(CanClearSentData)
static void CanClearSentData(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return;
	}

	/* Clear SentData to 0 *after* setting TrmReq to 0. */
	can_block_p->MCTL[mbox_nr].BIT.TX.TRMREQ = 0;	// ���̑��M��v�����邽�߂̎葱���@
	nop();
	can_block_p->MCTL[mbox_nr].BIT.TX.SENTDATA = 0;	// ���̑��M��v�����邽�߂̎葱���A
}


//[]----------------------------------------------------------------------[]
///	@brief			���b�Z�[�W��M�w��
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		mbox_nr					: ���[���{�b�N�X�ԍ�(0-31)<br>
///	@param[in]		sid						: ��M����CAN ID(0x0000-0x07FF)
///					frame_type				: DATA_FRAME=�ʏ�̃f�[�^�t���[��<br>
///											: REMOTE_FRAME=�����[�g�f�[�^�t���[��
///	@return			R_CAN_OK				: ����I��<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_SW_SET_TX_TMO:	: �O�̑��M�̏I���҂����^�C���A�E�g�����B<br>
///					R_CAN_SW_SET_RX_TMO:	: �O�̎�M�̏I���҂����^�C���A�E�g�����B
///	@author			m.onouchi
///	@note			�w�肳�ꂽID�����f�[�^�t���[������M����悤���[���{�b�N�X�Ɏw������B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_RxSet(const unsigned long ch_nr, const unsigned long mbox_nr, const unsigned long sid, const unsigned long frame_type)
{
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// ����mbox_nr�͈̔̓`�F�b�N
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait for any previous transmission/reception to complete. */
	api_status = R_CAN_WaitTxRx(0, mbox_nr);		// �O�̑��M����������܂ő҂�

	/* Interrupt disable the mailbox. */
	// NOTE:FIFO��M���[�h�̏ꍇ��MBX_28���g�p����
	can_block_p->MIER.LONG &= ~(bit_set[mbox_nr]);	// ���[���{�b�N�X���荞�݋֎~
	//NOTE:MB29�ɊY�������MFIFO���������^�C�~���O����r�b�g�́u0�F�����M�����Ŕ����v�Ƃ���B(��������0�Ƃ��Ă���̂ł����ł͕ύX���Ȃ�)

	/* Clear message mailbox control register. */
	can_block_p->RFCR.BYTE = 0;						// ��MFIFO���䃌�W�X�^�N���A

	// NOTE:FIFO��M���[�h�̏ꍇ��FIDCR1���g�p����
	/*** Set Mailbox. ***/
	can_block_p->FIDCR1.BIT.SID = sid;		// ID�Z�b�g
	can_block_p->FIDCR1.BIT.EID = 0;
	/* Dataframe = 0, Remote frame = 1	*/
	if (frame_type == REMOTE_FRAME) {
		can_block_p->FIDCR1.BIT.RTR = 1;	// �����[�g�t���[����I��
	} else {
		can_block_p->FIDCR1.BIT.RTR = 0;	// �f�[�^�t���[����I��
	}
	/* Frame select: Standard = 0, Extended = 1 */
	can_block_p->FIDCR1.BIT.IDE = 0;		// �W��ID

	/* Interrupt enable the mailbox */
	can_block_p->MIER.LONG |= (bit_set[mbox_nr]);	// ���[���{�b�N�X���荞�݋���

	/* Request to receive the frame with RecReq bit. */
	can_block_p->RFCR.BIT.RFE = 1;							// ��MFIFO����bit 1:��MFIFO����

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			���b�Z�[�W��M�����`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		mbox_nr					: ���[���{�b�N�X�ԍ�(0-31)<br>
///	@return			R_CAN_OK				: ��M���b�Z�[�W����<br>
///					R_CAN_NOT_OK			: ��M���b�Z�[�W�Ȃ�<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_RXPOLL_TMO		: ��M���̃��b�Z�[�W���^�C���A�E�g�����B
///	@author			m.onouchi
///	@note			�w�肳�ꂽ���[���{�b�N�X���f�[�^�t���[������M�������Ƃ��m�F����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_RxPoll(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_NOT_OK;
	unsigned long poll_delay = MAX_CAN_REG_POLLTIME;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// ����mbox_nr�͈̔̓`�F�b�N
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait if new data is currently being received. */
	while ((can_block_p->MCTL[mbox_nr].BIT.RX.INVALDATA) && poll_delay) {	// ���b�Z�[�W��M��(�҂�)
		poll_delay--;
	}
	if (poll_delay == 0) {	// �^�C���A�E�g
		api_status = R_CAN_RXPOLL_TMO;
	} else {
		if (can_block_p->MCTL[mbox_nr].BIT.RX.NEWDATA == 1) {	// �V���b�Z�[�W��M����
			api_status = R_CAN_OK;
		}
	}
	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			��M���b�Z�[�W�̎��o��
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		mbox_nr					: ���[���{�b�N�X�ԍ�(0-31)<br>
///	@param[out]		*frame_p				: �f�[�^�t���[��
///	@return			R_CAN_OK				: ��M���b�Z�[�W����<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_MSGLOST			: ���b�Z�[�W�͏㏑�����ꂽ������ꂽ�B
///	@author			m.onouchi
///	@note			�w�肵�����[���{�b�N�X����f�[�^�t���[�����擾����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_RxRead(const unsigned long ch_nr, const unsigned long mbox_nr, can_std_frame_t *const frame_p)
{
	unsigned long i;
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// ����mbox_nr�͈̔̓`�F�b�N
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	if(can_block_p->RFCR.BIT.RFEST == 0){	// ��MFIFO���ǃ��b�c�Z�[�W�L��
		api_status = R_CAN_OK;
		if(can_block_p->RFCR.BIT.RFMLF){// ���b�Z�[�W���X�g����
			can_block_p->RFCR.BIT.RFMLF = 0;
			// �I�[�o�[���[�h�i���b�Z�[�W���X�g�j�G���[�̓o�^
			g_ucCAN1LostErr = 1;
		}
		//	/* Copy received data from message mailbox to memory */
		frame_p->id = can_block_p->MB[mbox_nr].ID.BIT.SID;				// ID�擾
		frame_p->dlc = can_block_p->MB[mbox_nr].DLC;					// �f�[�^���擾
		for (i = 0; i < can_block_p->MB[mbox_nr].DLC; i++) {			// �f�[�^�R�s�[
			frame_p->data[i] = can_block_p->MB[mbox_nr].DATA[i];
		}
		can_block_p->RFPCR = 0xFF;//FIFO�̃|�C���^�X�V
	}else{
		api_status = R_CAN_NOT_OK;
	}

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			���b�Z�[�W�̎󂯓���}�X�N�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		mbox_nr					: ���[���{�b�N�X�ԍ�(0-31)<br>
///	@param[in]		sid_mask_value			: �}�X�N�l
///	@return			void
///	@author			m.onouchi
///	@attention		�}�X�N�̐ݒ�͘A������4�̃��[���{�b�N�X�ŋ��L����܂��B
///	@note			�w�肵�����[���{�b�N�X�Ƀt�B���^�������Ď�M���郁�b�Z�[�W�𐧌�����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void R_CAN_RxSetMask(const unsigned long ch_nr, const unsigned long mbox_nr, const unsigned long sid_mask_value)
{
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return;
	}

	/* Write to MKR0 to MKR7 in CAN reset mode or CAN halt mode. */
	R_CAN_Control(0, HALT_CANMODE);							// �z���g���[�h�ֈڍs

	/* Set mask for the goup of mailboxes. */
	can_block_p->MKR[mbox_nr/4].BIT.SID = sid_mask_value;	// �}�X�N�l��ݒ�

	/* Set mailbox mask to be used. (0 = mask VALID.) */
//NOTE:FIFO��M���[�h�̏ꍇ��MBX28�`MBX31���u�}�X�N�L���v�Ƃ���
	can_block_p->MKIVLR.LONG &= ~(bit_set[CANBOX_RX_28]);		// �}�X�N�L����
	can_block_p->MKIVLR.LONG &= ~(bit_set[CANBOX_RX_29]);		// �}�X�N�L����
	can_block_p->MKIVLR.LONG &= ~(bit_set[CANBOX_RX_30]);		// �}�X�N�L����
	can_block_p->MKIVLR.LONG &= ~(bit_set[CANBOX_RX_31]);		// �}�X�N�L����

	R_CAN_Control(0, OPERATE_CANMODE);						// �ʏ퓮�샂�[�h�ֈڍs
}


//[]----------------------------------------------------------------------[]
///	@brief			�O�̑��M����������܂ő҂�
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: �`���l���ԍ�(1)
///	@param[in]		mbox_nr					: ���[���{�b�N�X�ԍ�(0-31)
///	@return			R_CAN_OK				: ��M���b�Z�[�W����<br>
///					R_CAN_SW_BAD_MBX		: ���݂��Ȃ����[���{�b�N�X�ԍ�<br>
///					R_CAN_BAD_CH_NR			: ���݂��Ȃ��`���l���ԍ�<br>
///					R_CAN_SW_SET_TX_TMO:	: �O�̑��M�̏I���҂����^�C���A�E�g�����B<br>
///					R_CAN_SW_SET_RX_TMO:	: �O�̎�M�̏I���҂����^�C���A�E�g�����B
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#pragma inline(R_CAN_WaitTxRx)
static unsigned long R_CAN_WaitTxRx(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long can_tmo_cnt = MAX_CAN_SW_DELAY;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// ����mbox_nr�͈̔̓`�F�b�N
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait for any previous transmission to complete. */
	if (can_block_p->MCTL[mbox_nr].BIT.TX.TRMREQ) {		// ���M�v������
		while ((can_block_p->MCTL[mbox_nr].BIT.TX.SENTDATA == 1) && DEC_CHK_CAN_SW_TMR) {	// ���M�����҂�
			;
		}
		if (can_tmo_cnt == 0) {	// �^�C���A�E�g
			api_status = R_CAN_SW_SET_TX_TMO;
		}
	} else if (can_block_p->MCTL[mbox_nr].BIT.TX.RECREQ) {	// ��M�v������
		while ((can_block_p->MCTL[mbox_nr].BIT.RX.INVALDATA == 1) && DEC_CHK_CAN_SW_TMR) {	// ��M�����҂�
			;
		}
		if (can_tmo_cnt == 0) {	// �^�C���A�E�g
			api_status = R_CAN_SW_SET_RX_TMO;
		}
	}
	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN���j�b�g�̏�ԃ`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr						: �`���l���ԍ�(1)
///	@return			R_CAN_STATUS_ERROR_ACTIVE	: �G���[�A�N�e�B�u<br>
///					R_CAN_STATUS_ERROR_PASSIVE	: �G���[�p�b�V�u<br>
///					R_CAN_STATUS_BUSOFF			: �o�X�I�t
///	@author			m.onouchi
///	@note			CAN�y���t�F�����̃X�e�[�^�X�t���O���`�F�b�N���Č��݂̏�Ԃ�Ԃ��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_CheckErr(const unsigned long ch_nr)
{
	unsigned long api_status = R_CAN_STATUS_ERROR_ACTIVE;
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	if (can_block_p->STR.BIT.EST) {
		if (can_block_p->STR.BIT.EPST) {
			api_status = R_CAN_STATUS_ERROR_PASSIVE;
		} else if (can_block_p->STR.BIT.BOST) {
			api_status = R_CAN_STATUS_BUSOFF;
		}
	}

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN�ʐM���x�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr		: �`���l���ԍ�(1)
///	@return			void
///	@author			m.onouchi
///	@note			config_can_api.h�Œ�`���ꂽ�l��CAN�̒ʐM���x��ݒ肷��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void R_CAN_SetBitrate(const unsigned long ch_nr)
{
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return;
	}

	/* Set TSEG1, TSEG2 and SJW. */
	can_block_p->BCR.BIT.BRP = CAN_BRP - 1;
	can_block_p->BCR.BIT.TSEG1 = CAN_TSEG1 - 1;
	can_block_p->BCR.BIT.TSEG2 = CAN_TSEG2 - 1;
	can_block_p->BCR.BIT.SJW = CAN_SJW - 1;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN���荞�ݏ�����
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr	: �`���l���ԍ�(1)
///	@return			void
///	@author			m.onouchi
///	@attention		���[���{�b�N�X�̊��荞�݂͎g�p���Ɍʂɋ�����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void R_CAN_ConfigCANinterrupts(const unsigned long ch_nr)
{
	if (ch_nr == 1) {
		/* Configure CAN Tx interrupt. */
		IEN(CAN1, TXM1) = 1;				// vector 55(CAN1 TXM1):���荞�ݗv������
		IPR(CAN1, TXM1) = CAN1_INT_LVL;		// ���荞�ݗD�惌�x��

		/* Configure CAN Rx interrupt. */
		IEN(CAN1, RXF1) = 1;				// vector 52(CAN1 RXF1):���荞�ݗv������
		IPR(CAN1, RXF1) = CAN1_INT_LVL_RCV;		// ���荞�ݗD�惌�x��

		/* Configure CAN Error interrupt. */
		ICU.IER[IER_ICU_GROUPE0].BIT.IEN2 = 1;	// vector 106:���荞�ݗv������
		ICU.GEN[GEN_CAN1_ERS1].BIT.EN1 = 1;		// CAN1 ERS1:���荞�ݗv������
		ICU.IPR[IPR_ICU_GROUPE0].BIT.IPR = CAN1_INT_LVL;	// ���荞�ݗD�惌�x��

		CAN1.EIER.BYTE = 0x18;				// �o�X�G���[���荞�݋֎~
											// �G���[���[�j���O���荞�݋֎~
											// �G���[�p�b�V�u���荞�݋֎~
											// �o�X�I�t�J�n���荞�݋���
											// �o�X�I�t���A���荞�݋���
											// ��M�I�[�o�������荞�݋֎~
											// �I�[�o���[�h�t���[�����M���荞�݋֎~
											// �o�X���b�N���荞�݋֎~

		CAN1.MIER.LONG = 0x00000000;		// �S���[���{�b�N�X���荞�݋֎~
	}
}
