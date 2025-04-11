#ifndef	___PIPDEFH___
#define	___PIPDEFH___

//--------------------------------------------------------------------------------
// Park i Pro  (PIP) 
// �e��^��` / define �t�@�C��
//
//--------------------------------------------------------------------------------
#define	R_BUF_MAX_H				2060
#define	SEND_MAX_H				1448	// �g�n�r�s���M�G���A�l�`�w�T�C�Y
#define	PIP_HOSTSEND_LEN_MAX	256

#define	PIP_PACKET_SIZE_MAX	(5+PIP_HOSTSEND_LEN_MAX+3)	// �r�n�g�`�a�b�b�܂ł̍ő�T�C�Y(�p�P�b�g��{���̃T�C�Y)
#define	PIP_PACKET_DATA_MAX	(PIP_HOSTSEND_LEN_MAX+3)	// �r�s�w�`�a�b�b�܂ł̍ő�T�C�Y(�e�[�u���P�����̃T�C�Y)
#define	NH_RBUF_MAX			(6+PIP_HOSTSEND_LEN_MAX+2)
#define	NH_SBUF_MAX			(6+PIP_HOSTSEND_LEN_MAX+2)

#define		PIP_ERR_DATLEN_IDB0		24		// �G���[�ʒm�f�[�^(����)

typedef struct {
	unsigned char	COUNT;							// �p�P�b�g��(�O�`�W�O)
	unsigned char	CIND;							// ���M�p�P�b�g�ʒu(�O�`�V�X�C�W�O:���M����)
	struct {
		unsigned short	LEN;						// �p�P�b�g��(�r�s�w�`�a�b�b)
		unsigned char	BUFF[PIP_PACKET_DATA_MAX];	// �f�[�^�o�b�t�@
	} DATA[80];										// �W�O��
} t_PIP_SCISendQue;									// �V���A�����M�L���[

union PIP_B0_54_CTRL {
	struct {
		unsigned short  dmy   : 7;
		unsigned short	ON_54 : 1;	// [����]�������j���ʒm(1:�������j�����o, 0:����������)
		unsigned short	B0_01 : 1;	// [�Q�[�g��]�O���ڑ��@�햢����(1:������, 0:���A)
		unsigned short	B0_02 : 1;	// [�Q�[�g��]���Z�f�[�^���M�m�f(1:������, 0:���A)
		unsigned short	B0_03 : 1;	// [�Q�[�g��]�G���[�����f�[�^���M�m�f(1:������, 0:���A)
		unsigned short	B0_04 : 1;	// [�Q�[�g��]�G���[�����f�[�^���M�m�f(1:������, 0:���A)
		unsigned short	B0_05 : 1;	// [�Q�[�g��]���Z�۔���f�[�^���M�m�f(1:������, 0:���A)
		unsigned short	B0_06 : 1;	// [�Q�[�g��]�̎��؍Ĕ��s�۔���f�[�^���M�m�f(1:������, 0:���A)
		unsigned short	B0_10 : 1;	// [�Q�[�g��]�`�q�b�m�d�s�V�[�P���V�������ُ�(1:������, 0:���A)
		unsigned short	B0_11 : 1;	// [�Q�[�g��]��M�q�`�t�d���ُ�(1:������, 0:���A)
	} bits;
	unsigned short		WordData;
};
union	pip_bits_reg {
	struct	pip_bit_tag {
		unsigned char	bit_7	: 1 ;
		unsigned char	bit_6	: 1 ;
		unsigned char	bit_5	: 1 ;
		unsigned char	bit_4	: 1 ;
		unsigned char	bit_3	: 1 ;
		unsigned char	bit_2	: 1 ;
		unsigned char	bit_1	: 1 ;
		unsigned char	bit_0	: 1 ;
	} bits;
	unsigned char	byte;
};
typedef union pip_bits_reg	PIP_BITS;

//==============================================================================
//	�o���������o�q�n����p�ϐ�(���W���[�����[�J��)
//
//	@attention	�\���̂̐擪�A�h���X��ێ����Ă���|�C���^(PIPRAM)�ȊO�͎��̂��Ȃ��B
//
//	@note	�o���������o�q�n���W���[����(���p)�O���q�`�l��Ԃɒ�`���Ă���ϐ��Q�B
//
//======================================== Copyright(C) 2007 AMANO Corp. =======

typedef struct {
	char			PIP_RamMark[16];					// �q�`�l�j�����o�p�L�[���[�h"�o�h�o�s�`�r�j�q�`�l�Q�r�s�`�q�s" (�j�����Ă�����Q�d�m�c�܂ł`�k�k�N���A)

	unsigned short	initialize_start;					// �q�`�l�̔j�����Ȃ��Ƃ��A�������� un_initialize_start �܂ŃN���A

	unsigned char	Err_sendbuf[40];					// �p�����[�^�s�ǃG���[���M�o�b�t�@

	char			uc_r_buf_h[R_BUF_MAX_H];			// �V���A����M�o�b�t�@
	unsigned short	ui_write_h;							// �V���A����M�o�b�t�@(�P�L�����N�^)�������݈ʒu
	unsigned short	ui_read_h;							// �V���A����M�o�b�t�@�ǂݍ��݈ʒu
	unsigned short	ui_count_h;							// �V���A����M�o�b�t�@�f�[�^�擾��(�o�C�g��)
	unsigned char	uc_rcv_byte_h;						// �V���A����M�o�b�t�@�P�o�C�g��M���[�N

	unsigned short	ui_txdlength_i_h;					// �V���A�����M�o�b�t�@���M�f�[�^��(���荞�ݗp)
	unsigned short	ui_txpointer_h;						// �V���A�����M�o�b�t�@���M�f�[�^�|�C���^
	unsigned char	huc_txdata_i_h[SEND_MAX_H];			// �V���A�����M�o�b�t�@(���荞�ݗp)
	unsigned char	uc_send_end_h;						// �V���A�����M�o�b�t�@���M�����t���O

	unsigned long	NH_StartTime;						/* count of timer start */
	
	unsigned short	NH_Rct1;							/* NAK retry counter */
	unsigned short	NH_Rct2;							/* Pol error counter */
	unsigned short	NH_Rct3;							/* Sel error counter */
	
	unsigned char	NH_f_ComErr;						// �V���A�����M�G���[�t���O(0:���M��, 1:�G���[�ɂ�著�M�s��)
	
	unsigned char	NH_PrevRecvChar;					/* prevous received character */
	unsigned char	NH_f_NxAck;							/* next send/receive ACK (0x30=ACK0, 0x31=ACK1) */
	
	unsigned char	NH_Rbuf[NH_RBUF_MAX+2];				/* received character save area (temp) */
	unsigned short	NH_RbufCt;							/* received character count */
	
	unsigned char	NH_Sbuf[NH_SBUF_MAX+2];				// �V���A�����M�o�b�t�@
	unsigned short	NH_SbufLen;							// �V���A�����M�o�b�t�@���̃p�P�b�g��

	PIP_BITS		pip_uc_txerr_h;						// �V���A���ʐM�G���[�t���O
	unsigned char	pip_uc_mode_h ;						// �o���������o�q�n�Ƃ̒ʐM�����X�e�[�^�X
														// 0=IDLE
														// 1=POL(/NAK)���M��
														// 2=SOH/STX��M (ETX�҂�)
														// 3=ETX��M (BCC�҂�)
														// 4=ACK���M��  (EOT.etc..�҂�)
														// 5=SEL ���M�� (ACK.etc..�҂�)
														// 6=�d�����M�� (ACK.etc..�҂�)

	unsigned char	i_time_setting;						// �|�[�����O�Ԋu����("01"�`"99"�~100ms+300ms)

	union 	PIP_B0_54_CTRL	Ctrl_B0_54;					// �h�a�j���a�O�܂��͂T�S�łl�`�h�m�ɒʒm���邷�ׂẴG���[�̔����󋵂�ێ�����ϐ�
	unsigned char	ARCs_B0_54_SendEdit[PIP_ERR_DATLEN_IDB0];	// ��ʂa�O���T�S�̂`�q�b�m�d�s�p�P�b�g�쐬�p�o�b�t�@

	unsigned char	wkuc[8];							// �o�n�k�C�`�b�j�C�m�`�j�C�d�n�s�p�P�b�g�쐬�p�o�b�t�@

	t_PIP_SCISendQue		SCI_S_QUE;					// �V���A�����M�L���[

	unsigned short	un_initialize_start;				// �q�`�l�̔j�����Ȃ��Ƃ��͂�������Q�d�m�c�܂ŃN���A���Ȃ�

	char	PIP_RamEnd[16];								// �q�`�l�j�����o�p�L�[���[�h"�o�h�o�s�`�r�j�q�`�l�Q�d�m�c�O�P"
} t_PIPRAM;

#define	pip_f_txerr_h_dr	PIPRAM.pip_uc_txerr_h.bits.bit_6	// �f�[�^�l�͈͊O�G���[
#define	pip_f_txerr_h_dl	PIPRAM.pip_uc_txerr_h.bits.bit_5	// �f�[�^���G���[(�f�[�^���I�[�o�[�A�W�̔{���łȂ��Ȃ�)
#define	pip_f_txerr_h_BCC	PIPRAM.pip_uc_txerr_h.bits.bit_4	// BCC�G���[
#define	pip_f_txerr_h_txt	PIPRAM.pip_uc_hxerr_h.bits.bit_3	// �e�L�X�g�G���[(��M���ɐV����STX������)
#define	pip_f_txerr_h_fr	PIPRAM.pip_uc_txerr_h.bits.bit_2	// �t���[�~���O�G���[
#define	pip_f_txerr_h_p		PIPRAM.pip_uc_txerr_h.bits.bit_1	// �p���e�B�G���[
#define	pip_f_txerr_h_ovrun	PIPRAM.pip_uc_txerr_h.bits.bit_0	// �I�[�o�[�����G���[

extern	t_PIPRAM	PIPRAM;								// 

extern	void			PIP_ChangeData(unsigned short Length, unsigned char *pData);		// �V���A����M�f�[�^��IFM�̃o�b�t�@�֕ϊ�
extern	void			NH_Main( void );													// IFmodule�ʐM�������C���i��ԊǗ����j
extern	unsigned char	Mt_BccCal(unsigned char *pSet, unsigned short Length);
extern	unsigned short	Mh_get_receive(void);
extern	void			NH_TimerSet(unsigned char f_Start);
extern	void			sci_init_h(void);
extern	void			sci_tx_start_h(void);
extern	void			sci_stop_h(void);
extern	void			PIP_DTRsigOut(unsigned char f_Lebel);
extern	unsigned char	PIP_BcdAscToBin_2(unsigned char *asc_dat);
extern	void			PIP_ARCsB0Regist(unsigned char err_code, unsigned char on_off);
extern	unsigned short	PIP_SciSendQue_insert(unsigned char *pData, unsigned short length);	// �V���A�����M�L���[�ւ̃f�[�^�ǉ�
extern	void IFM_RcdBufClrAll();

extern	void			NH_Init( void );
#endif //___PIPDEFH__
