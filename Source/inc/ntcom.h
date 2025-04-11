// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
	// UT4000�� "ntcom.h"���x�[�X��FT4000�̍������ڐA���đΉ�
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
#ifndef	___NTCOMH___
#define	___NTCOMH___
/*[]----------------------------------------------------------------------[]
 *|	filename: ntcom.h
 *[]----------------------------------------------------------------------[]
 *| summary	: NT���W���[�� RAM���L���p���b�p�[��`
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
#include	"ntcomdef.h"

extern	unsigned char		NTCom_memory_pass[10];				// NTCom pass word

extern	T_NTCOM_NORMAL_DATABUF	NTComOpeRecvNormalDataBuf;		// NTCom�^�X�N�ʏ��M�o�b�t�@�i��OPE�w�j
extern	T_NTCOM_PRIOR_DATABUF	NTComOpeRecvPriorDataBuf;		// NTCom�^�X�N�D���M�o�b�t�@�i��OPE�w�j
extern	T_NTCOM_NORMAL_DATABUF	NTComOpeSendNormalDataBuf;		// NTCom�^�X�N�ʏ푗�M�o�b�t�@�i��OPE�w�j
extern	T_NTCOM_PRIOR_DATABUF	NTComOpeSendPriorDataBuf;		// NTCom�^�X�N�D�摗�M�o�b�t�@�i��OPE�w�j

extern	T_NTCom_SndWork		NTCom_SndWork;
extern	T_NT_SND_TELEGRAM	NTCom_Tele_Broadcast;
extern	T_NT_SND_TELEGRAM	NTCom_Tele_Prior;
extern	T_NT_SND_TELEGRAM	NTCom_Tele_Normal;


/* ntcomcomm.c */
extern	T_NT_BLKDATA		send_blk;							// ���M�o�b�t�@		���������Ɉ����œn��
extern	T_NT_BLKDATA		receive_blk;						// ��M�o�b�t�@

extern	ushort NT_MakeWord(unsigned char *data);
extern	void NT_Word2Byte(unsigned char *data, ushort us);
extern	void NTComComm_create_eot_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail);


/* ntcomcomdr.c */
extern	T_NT_COMDR_CTRL		NTComComdr_Ctrl;					/* COMDR����f�[�^ */


/* ntcomdata.c */
//#define			_NTComData_ResetTelegram(tele)	((tele)->curblk = -1)
#define			_NTComData_GetBlkNum(tele)		((tele)->blknum)
#define			_NTComData_GetRemainBlkNum(tele)	((tele)->blknum - (tele)->curblk)

extern	BOOL	NTComData_IsValidTelegram(T_NT_TELEGRAM *telegram);
extern	BOOL	NTComData_PeekRcvTele_Broadcast(T_NT_TELEGRAM *telegram);
extern	BOOL	NTComData_PeekRcvTele_Prior(T_NT_TELEGRAM *telegram);
extern	BOOL	NTComData_PeekRcvTele_Normal(T_NT_TELEGRAM *telegram);
extern	void	NTComData_InitBuffer(T_NT_BUFFER *buffer, T_NT_BLKDATA *mempool, int mempool_size);
extern	void	NTComData_ResetTelegram(T_NT_TELEGRAM *telegram);


/* �f�[�^�Ǘ��@�\ ����f�[�^ */
extern	T_NTComData_Ctrl		NTComData_Ctrl;
/* �ʏ�f�[�^�o�b�t�@ */
extern	T_NT_BUFFER			NTCom_SndBuf_Normal;				/* NT�l�b�g�ւ̑��M�p */
extern	T_NT_BUFFER			NTCom_RcvBuf_Normal;				/* NT�l�b�g����̎�M�p */
/* _z_NTCom_RcvBuf_Normal���̓d����� */
/* �ő�8�̒[�������M�����f�[�^��FIFO��IBW�ɑ��M���邽�߁A���L�f�[�^���g�p���ăX�P�W���[�����O���� */
extern	T_NT_TELEGRAM_LIST	NTCom_RcvTeleSchedule;
/* �D��f�[�^�o�b�t�@ */
extern	T_NT_BUFFER			NTCom_SndBuf_Prior;					/* #005 */
extern	T_NT_BUFFER			NTCom_RcvBuf_Prior;					/* NT�l�b�g����̎�M�p */
/* ����f�[�^�o�b�t�@ */
extern	T_NT_BUFFER			NTCom_SndBuf_Broadcast;				/* NT�l�b�g�ւ̑��M�p */
extern	T_NT_BUFFER			NTCom_RcvBuf_Broadcast;				/* NT�l�b�g����̎�M�p */
/* �f�[�^�o�b�t�@�������v�[�� */
extern	T_NT_BUFMEM_POOL	NTCom_BufMemPool;
/* �G���[���o�b�t�@ */
extern	T_NT_ERR_INFO		NTCom_Err;
/* ��d�ۏؐ���f�[�^ */
extern	T_NT_FUKUDEN		NTCom_Fukuden;

extern	eNT_DATASET	NTComData_SetSndBlock(T_NT_BLKDATA *blk);


/* ntcommain.c */
extern	uchar				NTCom_Condit;							/* task condition */
extern	uchar				NTCom_SciPort;							/* use SCI channel number (0 or 2) */
/** received command from IBW **/
//extern	t_NTCom_RcvQueCtrl	NTCom_RcvQueCtrl;						/* queue control data */
															/* If you want to get this data, you call "NTCom_ArcRcvQue_Read()" */
/** initial setting command received flag **/
extern	T_NT_INITIAL_DATA	NTCom_InitData;							/* initial setting data */
extern	t_NT_ComErr			NTCom_ComErr;

extern	void	NTCom_err_chk(char code, char kind, char f_data, char err_ctl, void *pData);


/* ntcomsci.c */
/* SCI����f�[�^ */
extern	T_NT_SCI_CTRL		NTComSci_Ctrl;

extern	void	NTComSci_Stop(void);


/* ntcomslave.c */
extern	T_NT_BLKDATA		*NTComSlave_send_blk;					// ���M�o�b�t�@		���������Ɉ����œn��
extern	T_NT_BLKDATA		*NTComSlave_receive_blk;					// ��M�o�b�t�@
extern	T_NTCOM_SLAVE_CTRL	NTComComm_Ctrl_Slave;						// �]�ǊǗ��p�\����
extern	T_NT_TELEGRAM		normal_telegram;						// �ʏ�f�[�^�d�����
extern	T_NT_TELEGRAM		prior_telegram;						// �D��f�[�^�d�����
extern	T_NT_TELEGRAM		*last_telegram;						// �Ō�ɑ������d��


/* ntcomtimer.c */
/* 1ms���x�O���[�v */
extern	T_NT_TIM_CTRL		NTComTimer_1ms[NT_TIM_1MS_CH_MAX];
/* 10ms���x�O���[�v */
extern	T_NT_TIM_CTRL		NTComTimer_10ms[NT_TIM_10MS_CH_MAX];
/* 100ms���x�O���[�v */
extern	T_NT_TIM_CTRL		NTComTimer_100ms[NT_TIM_100MS_CH_MAX];
/* �^�C�}ONOFF���� */
extern	BOOL				NTComTimer_Enable;	/* TRUE=�L�� */

extern	void	NTCom_TimoutChk();


/* ntcombuf.c */
extern	uchar	NTCom_GetSendPriorDataID(void);
extern	uchar	NTCom_GetSendNormalDataID(void);
extern	void	NTCom_ClearData(uchar mode);

extern	eNTNET_RESULT	NTCom_SetSendData(const uchar* pData, ushort size, uchar type);
extern	ushort	NTCom_GetSendData(uchar type, T_NT_BLKDATA* pRecvData);
extern	BOOL	NTCom_CanSetSendData(uchar type);

extern	uchar	read_rotsw(void);			// ���[�^���[�X�C�b�`�̏�Ԍ��o
extern	void	NTCom_Init( void );			// NT task initial routine
extern	void	NTComData_Clear(void);		// �f�[�^�Ǘ����W���[�����̃f�[�^�S�ăN���A
extern	void	NTComComm_Clear( void );	// �p�X���[�h�j�󎞏���
extern	void	NTCom_FuncStart( void );	// NTCom function start
extern	void	NTCom_FuncMain( void );		// credit function Main routine
											// �^�C�}�[�@�\
extern	ushort	NTComTimer_Create(ushort unit, ushort time, void (*func)(void), BOOL cyclic);
extern	BOOL	NTComTimer_Start(ushort id);// �^�C�}�[�X�^�[�g
											// ��M�p�P�b�g�擾
extern	eNT_COMDR_STS	NTComComdr_GetRcvPacket(T_NT_BLKDATA *buf);
											// ��M�����p�P�b�g�̏�Ԃ��`�F�b�N����B
											// �p�P�b�g���M(���M�������^�C���A�E�g�܂Ń��^�[�����Ȃ�))
extern	eNT_COMDR_SND	NTComComdr_SendPacket(T_NT_BLKDATA *buf, int wait, int timeout);
extern	BOOL	NTComComm_packet_check(T_NT_BLKDATA* check_blk, uchar seq_no, uchar terminal_no);
extern	void	NTComComm_create_stx_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail);
extern	void	NTComComm_create_ack_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail);
											// CRC���`�F�b�N����B
extern	BOOL	NTComComm_crc_check(T_NT_BLKDATA* buff);
											// �[���ڑ���ԕω��ʒm
extern	BOOL	NTComData_SetTerminalStatus(int terminal, eNT_STS sts, T_NT_TELEGRAM *telegram_normal, T_NT_TELEGRAM *telegram_prior);
											// �u���b�N�|�C���^�̈ʒu�ɂ���f�[�^���擾����
extern	int		NTComData_GetSendBlock(T_NT_TELEGRAM *telegram, T_NT_BLKDATA *blk);
											// �o�b�t�@����d���f�[�^���폜����
extern	void	NTComData_DeleteTelegram(T_NT_TELEGRAM *telegram);
											// �u���b�N�|�C���^�̈ʒu�ɂ���f�[�^���擾����
extern	int		NTComData_GetSendBlock(T_NT_TELEGRAM *telegram, T_NT_BLKDATA *blk);
extern	void	NTComComm_create_nak_telegram(T_NT_BLKDATA* buff ,uchar direction, uchar terminal_no, uchar seq_no, uchar terminal_status, uchar broadcast_flag, uchar response_detail);
											// ��M�o�b�t�@�Ƀf�[�^�Z�b�g
extern	eNT_DATASET	NTComData_SetRcvBlock(T_NT_BLKDATA *blk);
											// ���M�\�ȗD��f�[�^�d���̏����擾
extern	BOOL	NTComData_PeekSndTele_Prior(int terminal, T_NT_TELEGRAM *telegram);
											// ���M�\�Ȓʏ�f�[�^�d���̏����擾
extern	BOOL	NTComData_PeekSndTele_Normal(int terminal, T_NT_TELEGRAM *telegram);
											// �o�b�t�@��Ԏ擾
extern	void	NTComData_GetBufferStatus(T_NT_BUF_STATUS *bufsts, int terminal);
											// ��M�o�b�t�@�Ɋ܂܂��u���b�N�����擾����
extern	void	NTComData_GetRcvBufferCount(T_NT_BUF_COUNT *bufcnt);
											// �u���b�N�|�C���^���ړ�����
extern	int		NTComData_SeekBlock(T_NT_TELEGRAM *telegram, int offset, eNT_SEEK origin);
extern	void	NTComData_Start(void);		// �f�[�^�Ǘ����W���[��������
extern	void	NTComTimer_Init(void);		// �^�C�}�[�@�\������
extern	void	NTComComm_Start( void );	// NT�|NET�J�n����
extern	void	NTComComm_Main( void );		// NT�|NET���C������
											// �u���b�N�|�C���^�̈ʒu�ɂ���f�[�^���擾����
extern	int		NTComData_GetRecvBlock(T_NT_TELEGRAM *telegram, T_NTCom_SndWork *sndWork);
											// NTComComdr���W���[��������
extern	void	NTComComdr_Init(ushort timeout_c2c, uchar port, uchar speed, uchar dbit, uchar sbit, uchar pbit);
											// NTCom READER communication parts initialize routine
extern	void	NTComSlave_Start(T_NT_BLKDATA* send, T_NT_BLKDATA* receive);
											// NTCom READER communication parts initialize routine
extern	void 	NTComSlave_Clear( void );
											// NTCom READER function communication parts Main routine
extern	void	NTComSlave_Main( void );
											// �V�`���̎��v�f�[�^�d�����ۂ��̔���֐�
extern	BOOL	NT_IsNewTypeTimeSetPacket( T_NT_BLKDATA *blk );
extern	BOOL 	NTComSlave_isIdle( void );	// �^�X�N�̃A�C�h����Ԃ��擾����
											// NTComComdr���W���[��������
extern	void	NTComComdr_Init(ushort timeout_c2c, uchar port, uchar speed, uchar dbit, uchar sbit, uchar pbit);
											// �|�[�g������
extern	void	NTComSci_Init(T_NT_SCI_CALLBACK RcvFunc, T_NT_SCI_CALLBACK StsFunc,
				uchar Port, uchar Speed, uchar Dbits, uchar Sbits, uchar Pbit);
extern	void	NTComSci_Start(void);		// �|�[�g����J�n
											// �V���A�����M���J�n����
extern	BOOL	NTComSci_SndReq(uchar *buf, ushort len);
extern	BOOL	NTComSci_IsSndCmp(void);	// �V���A�����M�������擾����

#endif/* ___NTCOMH___ */
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
