#ifndef	___CRE_CTRL_H___
#define	___CRE_CTRL_H___
/*[]----------------------------------------------------------------------[]
 *| System      : FT4000
 *| Module      : CRE�ʐM���䋤�ʒ�`
 *[]----------------------------------------------------------------------[]
 *| Date        : 2013.07.01
 *| Update      : 
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/


// �ڼޯċ@�\�������ʃG���A
typedef struct st_cre_ctl {

	uchar	*RcvData;							// ��M�o�b�t�@�|�C���^

	uchar	Initial_Connect_Done;				// ����ڑ�����
	uchar	Credit_Stop;						// �N���W�b�g���Z��~��

	short	Status;								// ��Ԕԍ�
	short	Event;								// �N���v��
	short	OpenKind;							// �J�ǃR�}���h�v��
	short	SalesKind;							// ����f�[�^���M�v��
	
	ushort	SeqNo;								// �ڼޯēd�������ǂ���
	ulong	SlipNo;								// �[�������ʔԁi�`�[���j

	uchar	Timeout_Announce;					// ��M�^�C���A�E�g���̃A�i�E���X�ۃt���O
	short	MessagePtnNum;						// LCD�\�� cre �ŗL������ݔԍ�

	ushort	Result_Wait;						// �����d���p�҂�����

	struct st_open {							// �J�ǃR�}���h�p���g���C����
		char	Cnt;							// ���g���C�񐔁i�R��ڂ܂Ő�����j
		char	Stage;							// 0:�P�`�Q��ځA1:�R��ڈȍ~
		char	Min;							// �o�ߎ��ԁi���j
		char	Wait[2];						// [0]:�T���A[1]:�T�O��
	} Open;

	struct st_sales {							// ����˗��p���g���C����
		char	Cnt;							// ���g���C��
		char	Stage;							// 0:�P�`�Q��ځA1:�R��ڈȍ~
		char	Min;							// �o�ߎ��ԁi���j
		char	Wait[2];						// [0]:�T���A[1]:�T�O��
	} Sales;

	struct st_onlinetest {						// �����m�F�p���g���C����
		char	Min;							// �o�ߎ��ԁi���j
		char	Wait;							// ���s�Ԋu(��)
	} OnlineTest;

	// �ޔ��G���A�i�����f�[�^�`�F�b�N�p�j
	struct st_save {
		ushort			SeqNo;					// �����ǂ���
		date_time_rec2	Date;					// �����N���������b
		ulong			SlipNo;					// �[�������ʔԁi�`�[���j
		ulong			Amount;					// ������z
		uchar			AppNo[6];				// ���F�ԍ�
		uchar			ShopAccountNo[20];		// �����X����ԍ�
		uchar			TestKind;				// ýĎ��
	} Save;

	date_time_rec2	PayStartTime;				// ���Z�J�n����

} td_cre_ctl;

extern td_cre_ctl	cre_ctl;


// �ǂ��Ԕ͈�
#define		CRE_SEQ_MIN		1					// �����ǂ���
#define		CRE_SEQ_MAX		0xFFFF
#define		CRE_SLIP_MIN	1					// �[�������ʔԁi�`�[���j
#define		CRE_SLIP_MAX	99999

// ��Ԕԍ�: Status
#define		CRE_STS_IDLE				0		// �ҋ@
#define		CRE_STS_WAIT_OPEN			1		// 02�҂�
#define		CRE_STS_WAIT_CONFIRM		2		// 04�҂�
#define		CRE_STS_WAIT_SALES			3		// 06�҂�
#define		CRE_STS_WAIT_ONLINETEST		4		// 08�҂�
#define		CRE_STS_WAIT_RETURN			5		// 10�҂�

// �N���v��: Event
#define		CRE_EVT_SEND_OPEN			1		// �J�ǃR�}���h(01)
#define		CRE_EVT_SEND_CONFIRM		2		// �^�M�⍇��(03)
#define		CRE_EVT_SEND_SALES			3		// ����˗�(05)
#define		CRE_EVT_SEND_ONLINETEST		4		// ��ײ�ý�(07)
#define		CRE_EVT_SEND_RETURN			5		// �ԕi�⍇��(09)
#define		CRE_EVT_RECV_ANSWER			11		// �����d����M
#define		CRE_EVT_TIME_UP				21		// ������ѱ��

// ���M�R�}���h���
#define		CRE_SNDCMD_OPEN				1		// �J�ǃR�}���h(01)
#define		CRE_SNDCMD_CONFIRM			3		// �^�M�⍇��(03)
#define		CRE_SNDCMD_SALES			5		// ����˗�(05)
#define		CRE_SNDCMD_ONLINETEST		7		// ��ײ�ý�(07)
#define		CRE_SNDCMD_RETURN			9		// �ԕi�⍇��(09)

// ��M�R�}���h���
#define		CRE_RCVCMD_OPEN				2		// �J�ǃR�}���h����(02)
#define		CRE_RCVCMD_CONFIRM			4		// �^�M�⍇������(04)
#define		CRE_RCVCMD_SALES			6		// ����˗�����(06)
#define		CRE_RCVCMD_ONLINETEST		8		// ��ײ�ýĉ���(08)
#define		CRE_RCVCMD_RETURN			10		// �ԕi�⍇������(10)

// �R�}���h�^�C���A�E�g
#define		CRE_CMD_TIMEOUT				0xff	// �R�}���h�^�C���A�E�g

// �R�}���h���M�v��
#define		CRE_KIND_STARTUP			0		// �N��
#define		CRE_KIND_MANUAL				1		// �蓮(�����e�i���X����)
#define		CRE_KIND_RETRY				2		// ���g���C
#define		CRE_KIND_PAYMENT			3		// ���Z����
#define		CRE_KIND_REOPEN				4		// �ĊJ��(�g���p:���ݖ��g�p)
#define		CRE_KIND_AUTO				5		// ����

// �J�[�h�h�c
#define		CREDIT_CARDID_JIS1			0x7F	// �i�h�r�P�i�h�r�n�J�[�h�j7F=�����ԍ�
#define		CREDIT_CARDID_JIS2			0x61	// �i�h�r�Q�̃��[�h�h�c(JIS2,JIS1�ȊO�Ńv���y�C�h,�W���J�[�h�A�ȊO�̃J�[�h���AJIS2�����Ƃ��ľ����ɖ₢���킹����j

// �[�����ʔԍ��T�C�Y
#define		CREDIT_TERM_ID_NO_SIZE		13		// ���u�N���W�b�g�̒[�����ʔԍ��T�C�Y

// �J�ǃR�}���h�E���g���C
#define		CRE_RETRY_CNT_0				2		// Stage1:�J�Ԃ���
#define		CRE_RETRY_CNT_1				3		// Stage2:�ڍs��
#define		CRE_RETRY_WAIT_0			5		// Stage1:�Ԋu
#define		CRE_RETRY_WAIT_1			50		// Stage2:�Ԋu

#define		CRE_STATUS_OK				0x00	// �N���W�b�g����
#define		CRE_STATUS_DISABLED			0x01	// �N���W�b�g�ݒ�Ȃ�
#define		CRE_STATUS_NOTINIT			0x02	// ����ڑ�������
#define		CRE_STATUS_STOP				0x04	// �N���W�b�g��~��
#define		CRE_STATUS_UNSEND			0x08	// ����˗������M
#define		CRE_STATUS_SENDING			0x10	// �ʐM��
#define		CRE_STATUS_PPP_DISCONNECT	0x20	// PPP���ڑ�

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�i���g�p��`�폜�j�j�iFT-4000N�FMH364304���p�j
//// �N���W�b�g�T�[�o�[�ڑ��L��
//#define		CREDIT_ENABLED()		( (prm_get(COM_PRM, S_PAY, 24, 1, 2)) == 2 || (prm_get(COM_PRM, S_PAY, 24, 1, 2)) == 3)	// 1=CCT(���Ή�)
//																															// 2=CRE(FOMA)
//																															// 3=CRE(LAN)
//
//
//// ���Z���
//typedef struct st_creSeisanInfo {	// opetask => creCtrl
//	long			amount;				// ������z
//	unsigned char	jis_1[37];			// JIS1�����ް�
//	unsigned char	jis_2[69];			// JIS2�����ް�
//} td_creSeisanInfo;
//
//extern td_creSeisanInfo		creSeisanInfo;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�i���g�p��`�폜�j�j�iFT-4000N�FMH364304���p�j


// �v���g�^�C�v

//cre_ctrl.c
extern	short	creCtrl( short event );
extern	void	creCtrlInit( uchar flg );
extern	uchar	creStatusCheck( void );
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�i���g�p��`�폜�j�j�iFT-4000N�FMH364304���p�j
//extern	short	creLimitCheck( ulong Amount );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�i���g�p��`�폜�j�j�iFT-4000N�FMH364304���p�j
extern	void	creOneMinutesCheckProc( void );
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�i���g�p��`�폜�j�j�iFT-4000N�FMH364304���p�j
//extern	short	creOnlineTestCheck( void );
//extern	short	creErrorCheck( void );
//
//short			creSendData_OPEN( void );		// �J�ǃR�}���h(01)
//short			creSendData_CONFIRM( void );	// �^�M�⍇���f�[�^(03)
//short			creSendData_SALES( void );		// ����˗��f�[�^(05)
//short			creSendData_ONLINETEST( void );	// ��ײ�ý�(07)
//short			creSendData_RETURN( void );		// �ԕi�⍇���f�[�^(09)
//
//short			creRecvData( void );
//
//void			creSaleNG_Add( void );
//void			creUpdatePayData( DATA_KIND_137_04 *RcvData );
//short			creTimeOut( void );
//short			creResultCheck( void );
//void			creMemCpyRight(uchar *dist, uchar *src, ushort dist_len, ushort src_len);
//extern void		creMessageAnaOnOff( short OnOff, short num );
//
//extern void		creSeqNo_Init( void );		// �ڼޯēd���p�ǂ��ԏ�����
//extern ushort	creSeqNo_Count( void );		// �ڼޯēd���p�ǂ��ԃJ�E���g
//extern ushort	creSeqNo_Get( void );		// �ڼޯēd���p�ǂ��ԁF���ݒl��Ԃ�
//extern ulong	creSlipNo_Count( void );	// �[�������ʔԁi�`�[�ԍ��j�J�E���g
//extern ulong	creSlipNo_Get( void );		// �[�������ʔԁi�`�[�ԍ��j�F���ݒl��Ԃ�
//extern void		creSales_Init( void );		// ����˗��p���g���C����f�[�^������
//extern void		creSales_Reset( void );		// ����˗��̌o�ߎ��ԃJ�E���g�����Z�b�g
//extern uchar	creSales_Check( void );		// ����˗��̌o�ߎ��Ԃ��`�F�b�N
//extern void		creSales_Count( void );		// ����˗��f�[�^�đ��񐔃J�E���g
//extern void		creSales_Send( short kind );	// ����˗��̍đ�����
//extern void		creOpen_Init( void );		// �J�ǃR�}���h�p���g���C����f�[�^������
//extern void		creOpen_Enable( void );		// �J�ǃR�}���h�̃��g���C���M����
//extern void		creOpen_Disable( void );	// �J�ǃR�}���h�̃��g���C���M�֎~
//extern uchar	creOpen_Check( void );		// �J�ǃR�}���h�̌o�ߎ��Ԃ��`�F�b�N
//extern void		creOpen_Send( void );		// �J�ǃR�}���h���M����
//extern void		creOnlineTest_Init( void );	// ��ײ�ýĂ̐���f�[�^������
//extern void		creOnlineTest_Reset( void );// ��ײ�ýĂ̌o�ߎ��ԃJ�E���g�����Z�b�g
//extern void		creOnlineTest_Count( void );// ��ײ�ýĂ̌o�ߎ��Ԃ��J�E���g
//extern uchar	creOnlineTest_Check( void );// ��ײ�ýĂ̌o�ߎ��Ԃ��`�F�b�N
//extern void		creOnlineTest_Send( void );	// ��ײ�ýĂ̑��M����
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�i���g�p��`�폜�j�j�iFT-4000N�FMH364304���p�j
extern void		creInfoInit( void );		// �N���W�b�g��񏉊���
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�i���g�p��`�폜�j�j�iFT-4000N�FMH364304���p�j
//extern void		creRegMonitor( ushort code, uchar type, ulong info );	// �N���W�b�g�֘A���j�^�o�^
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�i���g�p��`�폜�j�j�iFT-4000N�FMH364304���p�j
extern	void	creCheckRejectSaleData( void );

#endif	/* ___CRE_CTRL_H___ */
