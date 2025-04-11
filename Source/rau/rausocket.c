/****************************************************************************/
/*																			*/
/*		�V�X�e�����@:�@RAU�V�X�e��											*/
/*		�t�@�C����	:  rausocket.c											*/
/*		�@�\		:  �\�P�b�g�ʐM����										*/
/*																			*/
/****************************************************************************/

#include	"system.h"
#include	"ope_def.h"
#include	"rau.h"
#include	"raudef.h"
#include	"trsocket.h"
#include	"trsntp.h"
#include	"prm_tbl.h"
#include	"message.h"
/*------------------------------------------------------------------------------*/
#pragma	section	_UNINIT3		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// CS2:�A�h���X���0x06039000-0x060FFFFF(1MB)
RAU_SOCK_CONDITION	RAU_upSockCondition;			// ������\�P�b�g�ʐM���
RAU_SOCK_CONDITION	RAU_downSockCondition;			// �������\�P�b�g�ʐM���
RAU_SOCK_CONDITION	RAU_creditSockCondition;			// �N���W�b�g����\�P�b�g�ʐM���
uchar	RAU_SendBuff[RAU_DATA_MAX + 1];			// ���������M�o�b�t�@
uchar	RAU_RecvBuff[RAU_DATA_MAX + 1];			// ��������M�o�b�t�@

/*------------------------------------------------------------------------------*/
#pragma	section	
/*------------------------------------------------------------------------------*/
int		RAU_upListenSocket = -1;					// ����Listen�\�P�b�g
int		RAU_upSocket = -1;							// ������\�P�b�g
int		RAU_downListenSocket = -1;					// �����Listen�\�P�b�g
int		RAU_downSocket = -1;						// �������\�P�b�g
uchar	Credit_TcpConnect = 0;							// 1:connect��
int		RAU_creditSocket = -1;						// �N���W�b�g����\�P�b�g
int		RAU_sntpListenSocket = -1;					// SNTPListen�\�P�b�g
int		socketDesc = TM_SOCKET_ERROR;				// SNTP �\�P�b�g�̏��
int		StateFlag = TM_SNTP_STATUS_SEND;			// SNTP ��ԃt���O
uchar	sntp_start = 0;								// SNTP �����␳�X�^�[�g�t���O
ushort	sntp_rmsg = 0;								// SNTP ���b�Z�[�W���M��(����/�蓮)
static	ttNtpPacket sntpdata;						// SNTP��M�p�P�b�g
static	ulong transmitTimestamp[2];					// SNTP�v������ 0:sec�ώZ�l 1:msec
uchar	RAU_AntennaLevelState = 0;					// �A���e�i���x���`�F�b�N�J�n(1)�E�I��(0)���
uchar	RAU_TcpConnect = 0;							// 1:connect��
const	char	RAU_dumy_str[RAU_DUMMY_STR_LEN + 1]={0x00,0x00,0x00,0x10,'A','M','A','N','O','_','_','D','U','M','M','Y',0};
uchar	RAU_SendDummyContinueCount = 0;				// �_�~�[�f�[�^�A�����M��

extern	int		KSG_gPpp_RauStarted;				// PPP�ڑ��t���O
extern	unsigned char	KSG_AntAfterPppStarted;

static	uchar	RAU_Prev_ppp_State;					// ���O��PPP�ڑ��t���O
static	RAU_LINE_STATE	RAU_UpSockProc(void);
static	RAU_LINE_STATE	RAU_DownSockProc(void);
char	leapyear_check( short pa_yer );
void	RAU_SNTPProc(void);
static	int		RAU_CreateUpSocket(void);
static	int		RAU_CreateDownSocket(void);
static	int		RAU_ConnectUpline(void);
static	void	RAU_CloseUpline(BOOL resetReq);
static	void	RAU_CloseDownline(void);
static	uchar	DpaSndQue_Read(uchar *pData, ushort *len, uchar kind, int port);
static	void RAU_DpaRcvQue_Set(uchar *pData, ushort len, int port);
void 	RAU_upSocketCallback(int sock, int event);
void 	RAU_downSocketCallback(int sock, int event);
static	RAU_LINE_STATE	RAU_CreditSockProc(void);
static	int RAU_CreateCreditSocket(void);
static	int	RAU_ConnectCreditline(void);
static	void	RAU_CloseCreditline(BOOL resetReq);
void 	RAU_creditSocketCallback(int sock, int event);
static	void	RAU_ClearLineState(void);

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SockMain
 *[]----------------------------------------------------------------------[]
 *| summary	: �\�P�b�g�������C��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SockMain(void)
{
	RAU_LINE_STATE upState = RAU_LINE_NO_ERROR;		// ������������
	RAU_LINE_STATE downState = RAU_LINE_NO_ERROR;	// �������������
	RAU_LINE_STATE creState;						// �N���W�b�g�����������
	uchar	modemCutState;
	
	if(KSG_gPpp_RauStarted == 0) {					// PPP���ؒf����Ă���
		RAU_Tm_DISCONNECT.tm = 0;					// �R�l�N�V�����ؒf�҂��^�C�}(��~)
		RAU_Tm_No_Action.tm = 0;					// ���ʐM�^�C�}(��~)
		RAU_SendDummyContinueCount = 0;				// �_�~�[�f�[�^�A�����M�񐔃N���A
		RAU_Tm_Port_Watchdog.tm = 0;				// �������ʐM�Ď��^�C�}(��~)
	}

	if((RAU_AntennaLevelState == 0) && (KSG_AntAfterPppStarted == 0)){	// �A���e�i���x�����蒆�ł͂Ȃ�
		if(RAU_Prev_ppp_State == 1 && KSG_gPpp_RauStarted == 0){		// PPP���ؒf���ꂽ
			if(RAU_upSocket != -1) {
				RAU_CloseUpline(TRUE);
			}
			
			if(RAU_downSocket != -1) {
				RAU_CloseDownline();
			}
			
			if(RAU_downListenSocket != -1) {
				tfClose(RAU_downListenSocket);
				RAU_downListenSocket = -1;
			}
			if(RAU_creditSocket != -1){
				RAU_CloseCreditline(TRUE);
			}
			// �f�[�^�đ��҂��^�C�}�N��
			RAU_Tm_data_rty.bits0.bit_0_13 = RauConfig.Dpa_data_rty_tm;	
			RAU_f_data_rty_rq = 1;
			RAU_f_data_rty_ov = 0;
		}
		if(RAU_Prev_ppp_State == 0 && KSG_gPpp_RauStarted == 1) {		// PPP���m�����ꂽ
			queset(OPETCBNO, CRE_EVT_CONN_PPP, 0, NULL);				// OPE�ɒʒm
		}
		RAU_Prev_ppp_State = KSG_gPpp_RauStarted;
	}
	else {
		RAU_Prev_ppp_State = 0;
	}

	if(_is_ntnet_remote()){
	// ���������
		upState = RAU_UpSockProc();
	
	// ����������
		downState = RAU_DownSockProc();
	}
	
	creState = RAU_CreditSockProc();
	/* �N���W�b�g�ʐM�ňُ킪�������Ă����A����̃R�l�N�V�����͈ێ����� */
	/* ���l�ɏ��A����̒ʐM�ňُ킪�������Ă��N���W�b�g�̃R�l�N�V�����͈ێ����� */
	if(creState != RAU_LINE_SEND){
		if(CreditCtrl.TCPdiscnct_req){
			/* TCP�ؒf�v�� */
			CreditCtrl.TCPdiscnct_req = 0;
			RAU_CloseCreditline(TRUE);
		}
	}

	if(RauConfig.modem_exist == 0 && 								// ���f���ڑ���
		((upState != RAU_LINE_SEND || downState != RAU_LINE_SEND) && creState != RAU_LINE_SEND) ) {	// ���M�f�[�^�Ȃ�
		if(KSG_RauGetMdmCutState() != 0) {							// ���f���ؒf�v������
			KSG_RauSetMdmCutState(2);								// ���f���ؒf���ɕύX
			if((RauCtrl.tcpConnectFlag.port.upload == 0 ||
				RauCtrl.tcpConnectFlag.port.download == 0) && RauCtrl.tcpConnectFlag.port.credit == 0){		// TCP���ڑ�
				KSG_RauClosePPPSession();							// PPP�ؒf
				KSG_RauSetMdmCutState(0);
			}
			else {
				// �㉺�����TCP��ؒf����
				RAU_CloseUpline(TRUE);
				RAU_CloseDownline();
				RAU_CloseCreditline(TRUE);
			}
		}
		else {
			if(RAU_f_No_Action_ov == 1){			// ������ؒf�����̂�h�����߂ɖ��ʐM�^�C�}�̃^�C���A�E�g
				RauCtrl.TCPcnct_req = 1;			// �s�b�o�ڑ��v���t���O(�ڑ��v��)
				// �_�~�[�f�[�^"AMANO__DUMMY"�𑗐M����
				if(RauCtrl.tcpConnectFlag.port.upload == 1) {	// �����TCP�ڑ���
					send(RAU_upSocket, RAU_dumy_str, RAU_DUMMY_STR_LEN, 0);
					RauCtrl.DPA_TCP_DummySend = 1;								// �_�~�[�p�P�b�g���M�t���O
					RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;	// �p�P�b�g�𑗐M����̂Ŗ��ʐM�^�C�}����
					RAU_f_No_Action_ov = 0;
					++RAU_SendDummyContinueCount;				// �_�~�[�f�[�^�A�����M�񐔉��Z
					if(RAU_SendDummyContinueCount >= 3) {		// �A�����ă_�~�[�f�[�^�����M���ꑱ���Ă��邩
						RAU_ClearLineState();					// �㉺����̏�Ԃ��N���A����
					}
				}
			}
			else {
				if(RauConfig.Dpa_nosnd_tm) {		// ���ʐM�^�C�}�ݒ肠��
					if(RAU_f_No_Action_rq == 0) {	// ���ʐM�^�C�}��~��
						if(KSG_gPpp_RauStarted) {	// PPP�ڑ���
							// ���ʐM�^�C�}���N��
							RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;
							RAU_f_No_Action_rq = 1;	// ���ʐM�^�C�}(�N��)
							RAU_f_No_Action_ov = 0;
						}
					}
				}
			}
		}
	}
	else {
		if(upState != RAU_LINE_SEND || downState != RAU_LINE_SEND) {	// �@�㉺����Ƃ����M�f�[�^�Ȃ�
			modemCutState = KSG_RauGetMdmCutState();
			if(modemCutState == 1) {					// ���f���ؒf�v������
				KSG_RauSetMdmCutState(2);				// ���f���ؒf���ɕύX
				// �㉺�����TCP��ؒf����
				if(RauCtrl.tcpConnectFlag.port.upload == 1) {
					RAU_CloseUpline(TRUE);
				}
				if(RauCtrl.tcpConnectFlag.port.download == 1) {	// TCP���ڑ�
					RAU_CloseDownline();
				}
				if(RauCtrl.tcpConnectFlag.port.credit == 1) {		// �N���W�b�g����ڑ���
					RAU_CloseCreditline(TRUE);
				}
			}
			else if(modemCutState == 2) {
				KSG_RauSetMdmCutState(0);
			}
		}
	}
	RAU_SNTPProc();				// SNTP������������

}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SockInit
 *[]----------------------------------------------------------------------[]
 *| summary	: �\�P�b�g��������������
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SockInit(void)
{
	RAU_upSockCondition = RAU_SOCK_INIT;			// ������\�P�b�g�ʐM���
	RAU_downSockCondition = RAU_SOCK_INIT;			// �������\�P�b�g�ʐM���
	RAU_upSocket = -1;								// ������\�P�b�g
	RAU_downSocket = -1;							// �������\�P�b�g
	SetUpNetState(NET_STA_IDLE);
	SetDownNetState(NET_STA_IDLE);
	RAU_creditSockCondition = RAU_SOCK_INIT;
	RAU_creditSocket = -1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_UpSockProc
 *[]----------------------------------------------------------------------[]
 *| summary	: ������\�P�b�g����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	RAU_LINE_STATE	RAU_UpSockProc(void)
{
	int ret;
	int		sockError;
	ushort	sendLen;										// ���M�f�[�^��
	RAU_LINE_STATE state = RAU_LINE_NO_ERROR;

	switch(RAU_upSockCondition) {
	case RAU_SOCK_INIT:										// ������
		if(KSG_gPpp_RauStarted) {							// PPP�ڑ����H
			RAU_upSockCondition = RAU_SOCK_OPEN;
			RauCtrl.connRetryCount = 0;						// ���g���C�񐔃N���A
		}
		break;
	case RAU_SOCK_OPEN:										// �\�P�b�g�I�[�v��
		if(RAU_AntennaLevelState) {							// �A���e�i���x���`�F�b�N��
			RAU_upSockCondition = RAU_SOCK_CLOSED;
			RauCtrl.connRetryCount = 0;						// ���g���C�񐔃N���A
			break;
		}
		if(KSG_gPpp_RauStarted) {
			if(RAU_upSocket != -1) {
				tfClose(RAU_upSocket);
				RAU_upSocket = -1;
				RAUhost_TcpConnReq_Clear();
			}
			
			if(RauCtrl.TCPcnct_req == 1) {					// TCP�ڑ��v������
				if(RAU_f_data_rty_ov) {						// �f�[�^�đ��҂��^�C�}���^�C���A�E�g����܂ł͐ڑ����J�n���Ȃ��B
				RAU_upSocket = RAU_CreateUpSocket();		// ������\�P�b�g�쐬
				if(RAU_upSocket != -1) {
					if(-1 != RAU_ConnectUpline()) {
						// TCP�ڑ��J�n
						RAU_upSockCondition = RAU_SOCK_CONNECTING;
					}
					else {
						tfClose(RAU_upSocket);
						RAU_upSockCondition = RAU_SOCK_CLOSING;
					}
				}
				}
				else {
					RAUhost_TcpConnReq_Clear();
					RauCtrl.TCPcnct_req = 0;				// �ڑ��v�����N���A����
					RauCtrl.connRetryCount = 0;				// ���g���C�񐔃N���A
				}
			}
			else {
				RauCtrl.connRetryCount = 0;					// ���g���C�񐔃N���A
			}
		}
		else {
			RauCtrl.connRetryCount = 0;						// ���g���C�񐔃N���A
		}
		break;
	case RAU_SOCK_CONNECTING:								// TCP�m����
		// �������Ȃ�
		break;
	case RAU_SOCK_CONNECTED:								// TCP�m��
		RAU_upSockCondition = RAU_SOCK_IDLE;
		RauCtrl.tcpConnectFlag.port.upload = 1;
		break;
	case RAU_SOCK_LISTENING:								// TCP�ڑ��҂�
		break;
	case RAU_SOCK_IDLE:										// TCP�ڑ���
		if(RAU_upSocket != -1) {							// �\�P�b�g�L���iTCP�ڑ����j
			RauCtrl.TCPcnct_req = 0;						// ���ɐڑ����Ȃ̂�TCP�ڑ��v�����N���A

			// �ؒf�v��������΃Z�b�V�����N���[�Y
			if(RAU_f_TCPdiscnct_ov == 1) {
				RAU_f_TCPdiscnct_ov = 0;
				RAU_CloseUpline(TRUE);
				break;
			}
			
			// ���M����
			if(DpaSndQue_Read(RAU_SendBuff, &sendLen, 0, UPLINE)) {	// ���M�f�[�^����H
				ret = send(RAU_upSocket, (char*)RAU_SendBuff, (int)sendLen, 0);
				if(ret == -1) {
					sockError = tfGetSocketError(RAU_upSocket);
					if(sockError == TM_EWOULDBLOCK) {		// ��M�f�[�^�Ȃ�(�m���u���b�L���O�̂��߃G���[�ł͂Ȃ�)
						;
					}
					else {
						RAU_CloseUpline(TRUE);
						break;
					}
				}
				else if(ret > RAU_DATA_MAX) {
					RAUhost_SetError(ERR_RAU_DPA_SEND_LEN);	// ��������M�f�[�^���ُ�(92)
					RAU_CloseUpline(TRUE);
					break;
				}
				state = RAU_LINE_SEND;
				
				// ���ʐM�^�C�}�N�����Ȃ�A���M�������߃^�C�}����������
				if( RAU_f_No_Action_rq == 1 ){
					RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;
					RAU_f_No_Action_ov = 0;
					RAU_SendDummyContinueCount = 0;			// �_�~�[�f�[�^�A�����M�񐔃N���A
				}
			}
			
			// ��M����
// MH322914(S) S.Takahashi 2017/02/16 ���f�[�^�p�P�b�g�����΍�
//			ret = recv(RAU_upSocket, (char*)RAU_RecvBuff, RAU_DATA_MAX, 0);	// �f�[�^��M
			ret = recv(RAU_upSocket, (char*)RAU_RecvBuff, RAU_NET_RES_LENGTH_MAX, 0);	// �f�[�^��M
// MH322914(E) S.Takahashi 2017/02/16 ���f�[�^�p�P�b�g�����΍�
			if(ret > 0) {									// ��M�f�[�^����
// MH322914(S) S.Takahashi 2017/02/16 ���f�[�^�p�P�b�g�����΍�
//				if(ret > RAU_NET_RES_LENGTH_MAX) {
//					RAUhost_SetError(ERR_RAU_DPA_RECV_LEN);	// �������M�f�[�^���ُ�(93)
//					ret = RAU_DATA_MAX;
//				}
// MH322914(E) S.Takahashi 2017/02/16 ���f�[�^�p�P�b�g�����΍�
				// ��M�f�[�^���i�[����
				RAU_DpaRcvQue_Set(RAU_RecvBuff, (ushort)ret, UPLINE);
			}
			else if(ret == -1) {							// �G���[����?
				sockError = tfGetSocketError(RAU_upSocket);
				if(sockError == TM_EWOULDBLOCK) {			// ��M�f�[�^�Ȃ�(�m���u���b�L���O�̂��߃G���[�ł͂Ȃ�)
					;
				}
				else {										// �G���[����
					RAU_CloseUpline(TRUE);
				}
			}
		}
		break;
	case RAU_SOCK_CLOSING:									// TCP�ؒf��
		// �������Ȃ�
		if(RAU_upSocket == -1) {
			RAU_upSockCondition = RAU_SOCK_CLOSED;
		}
		break;
	case RAU_SOCK_CLOSED:									// TCP�ؒf
		RauCtrl.tcpConnectFlag.port.upload = 0;
		if(KSG_gPpp_RauStarted) {
			RAU_upSockCondition = RAU_SOCK_OPEN;
		}
		else if(RAU_AntennaLevelState == 1) {				// �A���e�i���x���`�F�b�N��
			;												// �������Ȃ�(RAU_SOCK_CLOSED�̂܂�)
		}
		else {
			RAU_upSockCondition = RAU_SOCK_INIT;
		}
		break;
	default:
		break;
	}
	
	return state;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_DownSockProc
 *[]----------------------------------------------------------------------[]
 *| summary	: �������\�P�b�g����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	RAU_LINE_STATE	RAU_DownSockProc(void)
{
	int ret;
	struct	sockaddr	addr;								// �ڑ������A�h���X
	int		acceptSock;										// �ڑ������\�P�b�g
	int		addeSize;
	int		sockError;
	ushort	sendLen;										// ���M�f�[�^��
	RAU_LINE_STATE state = RAU_LINE_NO_ERROR;

	switch(RAU_downSockCondition) {
	case RAU_SOCK_INIT:										// ������
		if(KSG_gPpp_RauStarted) {							// PPP�ڑ����H
			RAU_downSockCondition = RAU_SOCK_OPEN;
		}
		break;
	case RAU_SOCK_OPEN:										// �\�P�b�g�I�[�v��
		if(KSG_gPpp_RauStarted) {
			if(RAU_downListenSocket != -1) {
				tfClose(RAU_downListenSocket);				// �I�[�v���ς݂Ȃ�N���[�Y����
				RAU_downListenSocket = -1;
			}

			RAU_downListenSocket = RAU_CreateDownSocket();	// �\�P�b�g�쐬
			if(RAU_downListenSocket != -1) {
				RAU_downSockCondition = RAU_SOCK_LISTENING;
			}
			else {
				sockError = tfGetSocketError(RAU_downListenSocket);
			}
		}
		break;
	case RAU_SOCK_CONNECTING:								// TCP�m����
		break;
	case RAU_SOCK_CONNECTED:								// TCP�m��
		break;
	case RAU_SOCK_LISTENING:								// TCP�ڑ��҂�
		if(RAU_downListenSocket != -1) {
			if(RAU_AntennaLevelState) {							// �A���e�i���x���`�F�b�N��
				tfClose(RAU_downListenSocket);
				RAU_downListenSocket = -1;
				RAU_downSockCondition = RAU_SOCK_CLOSED;
				break;
			}
			ret = listen(RAU_downListenSocket, 3);
			if(ret != -1) {									// �ڑ�����
				acceptSock = accept(RAU_downListenSocket, &addr, &addeSize);
				if(acceptSock != -1) {
					if(RAU_downSocket == -1) {				// ���ݖ��ڑ�
						RAU_downSocket = acceptSock;
						// �\�P�b�g�̃R�[���o�b�N�o�^
						tfRegisterSocketCB(RAU_downSocket, RAU_downSocketCallback, 
											TM_CB_REMOTE_CLOSE | TM_CB_SOCKET_ERROR |
											TM_CB_RESET | TM_CB_CLOSE_COMPLT | TM_CB_CONNECT_COMPLT);
						
						// ��������ԕύX
						RAU_downSockCondition = RAU_SOCK_IDLE;
						RauCtrl.tcpConnectFlag.port.download = 1;
					}
					else {									// ���ɐڑ��ς�
						tfClose(acceptSock);
					}
				}
				else {
					sockError = tfGetSocketError(RAU_downListenSocket);
				}
			}
			else {
				sockError = tfGetSocketError(RAU_downListenSocket);
				if(sockError == TM_EWOULDBLOCK) {			// �ڑ��Ȃ�(�m���u���b�L���O�̂��߃G���[�ł͂Ȃ�)
					;
				}
				else {
					tfClose(RAU_downListenSocket);
					RAU_downListenSocket = -1;
					RAU_downSockCondition = RAU_SOCK_OPEN;
				}
			}
		}
		else {
			RAU_downSockCondition = RAU_SOCK_CLOSED;		// CLOSED�ɑJ�ڂ�OPEN��Ƀ��X�j���O�\�P�b�g���쐬����
		}
		break;
	case RAU_SOCK_IDLE:										// TCP�ڑ���
		if(RAU_downSocket != -1) {							// �\�P�b�g�L���iTCP�ڑ����j
			// �������ʐM�Ď����^�C���A�E�g�̏ꍇ�́AHOST�����Ȃ��Ƃ��Đؒf����
			if(RAU_f_Port_Watchdog_ov == 1) {
				RAU_CloseDownline();						// �z�X�g�̉����Ȃ��Ƃ݂Ȃ�����ؒf
				RAU_Tm_Port_Watchdog.tm = 0;				// �������Ď��^�C�}(��~)
				RAUhost_DwonLine_common_timeout();			// ��M�V�[�P���X���O�ɖ߂�
			}
			
			// ���M����
			if(DpaSndQue_Read(RAU_SendBuff, &sendLen, 0, DOWNLINE)) {	// ���M�f�[�^����H
				ret = send(RAU_downSocket, (char*)RAU_SendBuff, (int)sendLen, 0);
				if(ret == -1) {
					sockError = tfGetSocketError(RAU_upSocket);
					if(sockError == TM_EWOULDBLOCK) {		// ��M�f�[�^�Ȃ�(�m���u���b�L���O�̂��߃G���[�ł͂Ȃ�)
						;
					}
					else {
						RAU_CloseDownline();				// �Z�b�V�����N���[�Y
						break;
					}
				}
				else if(ret > RAU_DATA_MAX) {
					RAUhost_SetError(ERR_RAU_DPA_SEND_LEN_D);	// ���������M�f�[�^���ُ�(192)
					RAU_CloseDownline();					// �Z�b�V�����N���[�Y
					break;
				}
				state = RAU_LINE_SEND;

				// ���ʐM�^�C�}�N�����Ȃ�A���M�������߃^�C�}����������
				if( RAU_f_No_Action_rq == 1 ){
					RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;
					RAU_SendDummyContinueCount = 0;			// �_�~�[�f�[�^�A�����M�񐔃N���A
					RAU_f_No_Action_ov = 0;
				}
			}
			
			// ��M����
			ret = recv(RAU_downSocket, (char*)RAU_RecvBuff, RAU_DATA_MAX, 0);	// �f�[�^��M
			if(ret > 0) {									// ��M�f�[�^����
				if(ret > RAU_DATA_MAX) {
					RAUhost_SetError(ERR_RAU_DPA_RECV_LEN_D);	// ��������M�f�[�^���ُ�(193)
					ret = RAU_DATA_MAX;
				}
				// ��M�f�[�^���i�[����
				RAU_DpaRcvQue_Set(RAU_RecvBuff, (ushort)ret, DOWNLINE);
			}
			else if(ret == -1) {							// �G���[����?
				sockError = tfGetSocketError(RAU_downSocket);
				if(sockError == TM_EWOULDBLOCK) {			// ��M�f�[�^�Ȃ�(�m���u���b�L���O�̂��߃G���[�ł͂Ȃ�)
					;
				}
				else {										// �G���[����
					RAU_CloseDownline();
					RAU_downSockCondition = RAU_SOCK_CLOSING;
					RauCtrl.tcpConnectFlag.port.download = 0;
				}
			}
		}
		break;
	case RAU_SOCK_CLOSING:									// TCP�ؒf��
		if(RAU_downSocket == -1) {
			RAU_downSockCondition = RAU_SOCK_CLOSED;
		}
		break;
	case RAU_SOCK_CLOSED:									// TCP�ؒf
		RauCtrl.tcpConnectFlag.port.download = 0;
		if(KSG_gPpp_RauStarted) {
			RAU_downSockCondition = RAU_SOCK_OPEN;
		}
		else if(RAU_AntennaLevelState == 1) {				// �A���e�i���x���`�F�b�N��
			;												// �������Ȃ�(RAU_SOCK_CLOSED�̂܂�)
		}
		else {
			RAU_downSockCondition = RAU_SOCK_INIT;
			
		}
		break;
	default:
		break;
	}
	
	return state;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SNTPStart
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP�����␳�J�n����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SNTPStart( uchar onoff ,ushort msg, ulong now_sec, ushort now_msec )
{
	sntp_start = onoff;
	sntp_rmsg = msg;
	if(socketDesc != TM_SOCKET_ERROR) {
		tfClose(socketDesc);
	}
	socketDesc = TM_SOCKET_ERROR;				// SNTP �\�P�b�g�̏��
	StateFlag = TM_SNTP_STATUS_SEND;			// SNTP ��ԃt���O
	// �v���J�n
	if (sntp_start) {
		// �v������(UTC)
		transmitTimestamp[0] = now_sec;
		transmitTimestamp[1] = now_msec;
	}
	// �v����~
	else {
		memset(transmitTimestamp, 0, sizeof(transmitTimestamp));
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SNTPProc
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP������������
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SNTPProc(void)
{
	int ret;

	struct sockaddr_in ipv4_Setting;
	ttSockAddrStoragePtr ipAddrPtr;
	char	myIP[16];
	
	if( sntp_start == 1 ){	// SNTP �X�^�[�g�t���O��ON�̏ꍇ�̂ݎ��{
		if(KSG_gPpp_RauStarted == 1 ) {										// PPP�ڑ���
			ipv4_Setting.sin_len = sizeof( ipv4_Setting );					// length of struct sockaddr_in
			ipv4_Setting.sin_family = AF_INET;								// AF_INET
			ipv4_Setting.sin_port = _SNTP_PORTNO;							// SNTP �|�[�g�ԍ�
			memset(myIP, 0x00, sizeof(myIP));
			sprintf(myIP, "%03d.%03d.%03d.%03d",
						prm_get(COM_PRM, S_CEN, 62, 3, 4),
						prm_get(COM_PRM, S_CEN, 62, 3, 1),
						prm_get(COM_PRM, S_CEN, 63, 3, 4),
						prm_get(COM_PRM, S_CEN, 63, 3, 1));
			ipv4_Setting.sin_addr.s_addr = inet_addr(myIP);	
			ipAddrPtr = (ttSockAddrStoragePtr) &ipv4_Setting;				// SNTP�T�[�o�[IP�A�h���X
			ret = tfSntpGetTimeByUnicast( TM_BLOCKING_OFF, &socketDesc, &StateFlag, ipAddrPtr );	// SNTP�����␳���s
			
			if( ret == TM_EWOULDBLOCK && StateFlag == TM_SNTP_STATUS_RECV ){
				// �֐����J��Ԃ��R�[�����邽�߁A�����ł͉������Ȃ�
			} else {	// ����I��or������SNTP�n���h��
				sntp_start = 0;									// ���� SNTP �X�^�[�g�t���O�𗎂Ƃ�
			}
		}
		else {																// PPP���ڑ�
			sntp_start = 0;
			queset(OPETCBNO, sntp_rmsg, 0, NULL);
		}
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SNTPGetPacket
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP��M�p�P�b�g�擾����
 *[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void	RAU_SNTPGetPacket( void *voidPtr )
{
	ttNtpPacket *pkt;

	if (voidPtr) {
		pkt = voidPtr;
		memcpy(pkt, &sntpdata, sizeof(sntpdata));
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SNTPStart
 *[]----------------------------------------------------------------------[]
 *| summary	: �[�N�`�F�b�N
 *| return	: 0:���N�A1�F�[�N
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
char	leapyear_check( short pa_yer )
{
	char	cm_ret;
	cm_ret = 0;			// ���N�ɂ��Ă���
	if (( (pa_yer % 4) == 0 )&&( (pa_yer % 100) != 0 )){	// 4�Ŋ���؂�� and 100�Ŋ���؂�Ȃ�
		cm_ret = 1;		// �[�N
	}
	if ((pa_yer % 400) == 0 ){								// 100�Ŋ���؂�Ă��A400�Ŋ���؂��Ɖ[�N
		cm_ret = 1;		// �[�N
	}
	return( cm_ret );
}
/*[]----------------------------------------------------------------------[]
 *|	name	: tfKernelGetSystemTime
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP���ݎ����擾����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int tfKernelGetSystemTime( ttUser32Bit * daysPrt, ttUser32Bit * secondsPtr )
{
	*daysPrt = 0;
	*secondsPtr = transmitTimestamp[0];

	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: tfKernelSetSystemTime
 *[]----------------------------------------------------------------------[]
 *| summary	: SNTP�����ݒ菈��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int tfKernelSetSystemTime( ttUser32BitPtr secondsPtr, void * voidPtr , int size )
{
	ttNtpPacket *pkt;

	pkt = voidPtr;

	// ���������ł͂Ȃ�
	if (pkt->sntpMode != TM_SNTP_MODE_SERVER) {
		return -1;
	}
	// �v�������s��v
	if (transmitTimestamp[0] != 0) {
		if (transmitTimestamp[0] != pkt->originateTimestampHigh &&
			0 != pkt->originateTimestampLow) {
			return -1;
		}
	}
	// SNTP�p�P�b�g��ۑ�
	memcpy(&sntpdata, pkt, sizeof(*pkt));
	queset(OPETCBNO, sntp_rmsg, 0, NULL);
	
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetUpNetState
 *[]----------------------------------------------------------------------[]
 *| summary	: ������̏�Ԃ��擾����
 *| retrun	: RAU_NET_IDLE				���ڑ�
 *|			  RAU_NET_IDLE				PPP�m����
 *|			  RAU_NET_TCP_CONNECTIED	TCP�m����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
RAU_NET_STATE	RAU_GetUpNetState(void)
{
	if(KSG_gPpp_RauStarted) {						// PPP�m����
		if(RauCtrl.tcpConnectFlag.port.upload) {	// �����TCP�m����
			return RAU_NET_TCP_CONNECTIED;
		}
		else {
			return RAU_NET_PPP_OPENED;
		}
	}
	
	return RAU_NET_IDLE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetDownNetState
 *[]----------------------------------------------------------------------[]
 *| summary	: �������̏�Ԃ��擾����
 *| retrun	: RAU_NET_IDLE				���ڑ�
 *|			  RAU_NET_IDLE				PPP�m����
 *|			  RAU_NET_TCP_CONNECTIED	TCP�m����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
RAU_NET_STATE	RAU_GetDownNetState(void)
{
	if(KSG_gPpp_RauStarted) {						// PPP�m����
		if(RauCtrl.tcpConnectFlag.port.download) {	// ������TCP�m����
			return RAU_NET_TCP_CONNECTIED;
		}
		else {
			return RAU_NET_PPP_OPENED;
		}
	}
	
	return RAU_NET_IDLE;
}

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_DpaRcvQue_Init()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command receive queue initialize.                             |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_DpaRcvQue_Init( void )
{
	memset(&DPA_Rcv_Ctrl, 0, sizeof(DPA_Rcv_Ctrl));
	memset(&DPA2_Rcv_Ctrl, 0, sizeof(DPA2_Rcv_Ctrl));
}


/*[]----------------------------------------------------------------------[]*/
/*|             RAU_DpaRcvQue_Set()                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command set to receive queue.                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pointer for receive data.                                     |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void RAU_DpaRcvQue_Set(uchar *pData, ushort len, int port)
{
	short	length;										// �f�[�^��
	short	oldLen;										// �p�P�b�g�r���̃f�[�^��
	short	remain;										// �c��f�[�^��
	ushort	index = 0;
	
	remain = (short)len;
	if( port == DOWNLINE ){	// ����������̎�M�f�[�^���Z�b�g����B
		if( len > RAU_DATA_MAX ) {
			len = RAU_DATA_MAX;
		}
		
		while(index < len) {
			if(DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len == 0) {
				// �p�P�b�g�̐擪�����M
				if(remain < 4) {							// �f�[�^������M���Ă��Ȃ�
					// �f�[�^�����s���̂��ߎ����̃f�[�^���擾����
					// �p�P�b�g��M�r���̂��߃C���f�b�N�X�A�J�E���^�͍X�V���Ȃ�
					DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len = remain;
					memcpy( DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Dat, pData, remain );
					break;
				}
			}

			// ��M�f�[�^�i�[�i�p�P�b�g�̓r������܂��͓r���܂ł̉\������j
			memcpy( &DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Dat[DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len], 
								pData + index, remain );		// �Ƃ肠�����S�ăR�s�[
			
			// 1�p�P�b�g���̃f�[�^����ݒ肷��
			length = (short)RAU_Byte2Long(DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Dat);
			oldLen = DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len;
			DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len = length;
			
			if(length <= remain + oldLen) {						// 1�p�P�b�g�S�Ď�M�H
				// �S�f�[�^��M���Ă���΃C���f�b�N�X�A�J�E���^���X�V
				++DPA_Rcv_Ctrl.Count;							// queue regist count update
				++DPA_Rcv_Ctrl.WriteIdx;						// next write index update
				DPA_Rcv_Ctrl.WriteIdx &= 3;
			}
			index += remain < length ? remain : length;			// ���̃p�P�b�g�ƂȂ�擪�C���f�b�N�X��ݒ�
			remain -= (length - DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.WriteIdx ].Len);
		}
	}
	else if( port == CREDIT ){
		if( len > RAU_DATA_MAX ) {
			len = RAU_DATA_MAX;
		}
		
		while(index < len) {
			if(DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len == 0) {
				// �p�P�b�g�̐擪�����M
				if(remain < 4) {							// �f�[�^������M���Ă��Ȃ�
					// �f�[�^�����s���̂��ߎ��̃f�[�^���擾����
					// �p�P�b�g��M�r���̂��߃C���f�b�N�X�A�J�E���^�͍X�V���Ȃ�
					DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len = remain;
					memcpy( DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Dat, pData, remain );
					break;
				}
			}

			// ��M�f�[�^�i�[�i�p�P�b�g�̓r������܂��͓r���܂ł̉\������j
			memcpy( &DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Dat[DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len], 
								pData + index, remain );		// �Ƃ肠�����S�ăR�s�[
			
			// 1�p�P�b�g���̃f�[�^����ݒ肷��
			length = (short)RAU_Byte2Long(DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Dat);
			oldLen = DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len;
			DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len = length;
			
			if(length <= remain + oldLen) {						// 1�p�P�b�g�S�Ď�M�H
				// �S�f�[�^��M���Ă���΃C���f�b�N�X�A�J�E���^���X�V
				++DPA_Credit_RcvCtrl.Count;							// queue regist count update
				++DPA_Credit_RcvCtrl.WriteIdx;						// next write index update
				DPA_Credit_RcvCtrl.WriteIdx &= 3;
			}
			index += remain < length ? remain : length;			// ���̃p�P�b�g�ƂȂ�擪�C���f�b�N�X��ݒ�
			remain -= (length - DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.WriteIdx ].Len);
		}
	} else {				// ���������̎�M�f�[�^���Z�b�g����B
		memcpy(DPA2_Rcv_Ctrl.dpa_data[DPA2_Rcv_Ctrl.WriteIdx], pData, RAU_NET_RES_LENGTH_MAX);

		++DPA2_Rcv_Ctrl.Count;
		++DPA2_Rcv_Ctrl.WriteIdx;
		DPA2_Rcv_Ctrl.WriteIdx &= 3;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_DpacRcvQue_Read()                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command read from receive queue.                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pointer for set command data (receive mail style).            |*/
/*| return : 1=received data exist,  0=data none                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar RAU_DpaRcvQue_Read(uchar *pData, ushort *len, int port)
{
	ushort	w_len;

	if( port == DOWNLINE ){	// ����������̎�M�f�[�^�����o���B
		if( 0 == DPA_Rcv_Ctrl.Count ) {
			return	(uchar)0;
		}

		w_len = DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.ReadIdx ].Len;
		DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.ReadIdx ].Len = 0;
		
		if( w_len > RAU_DATA_MAX ) {
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memcpy( pData, DPA_Rcv_Ctrl.dpa_data[ DPA_Rcv_Ctrl.ReadIdx ].Dat, w_len );

		--DPA_Rcv_Ctrl.Count;							// queue regist count update
		++DPA_Rcv_Ctrl.ReadIdx;							// next read index update
		DPA_Rcv_Ctrl.ReadIdx &= 3;
	}
	else if( port == CREDIT ){
		if( 0 == DPA_Credit_RcvCtrl.Count ) {
			return	(uchar)0;
		}

		w_len = DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.ReadIdx ].Len;
		DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.ReadIdx ].Len = 0;
		
		if( w_len > RAU_DATA_MAX ) {
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memcpy( pData, DPA_Credit_RcvCtrl.dpa_data[ DPA_Credit_RcvCtrl.ReadIdx ].Dat, w_len );

		--DPA_Credit_RcvCtrl.Count;							// queue regist count update
		++DPA_Credit_RcvCtrl.ReadIdx;							// next read index update
		DPA_Credit_RcvCtrl.ReadIdx &= 3;
	} else {				// ���������̎�M�f�[�^�����o���B
		if( 0 == DPA2_Rcv_Ctrl.Count ) {	// �f�[�^�Ȃ�
			return	(uchar)0;
		}

		*len = RAU_NET_RES_LENGTH_MAX;	// ������̎�M�f�[�^�͂`�b�j���m�`�b�����Ȃ��̂Ńo�b�t�@�͏������B
		memcpy(pData, DPA2_Rcv_Ctrl.dpa_data[DPA2_Rcv_Ctrl.ReadIdx], RAU_NET_RES_LENGTH_MAX);

		--DPA2_Rcv_Ctrl.Count;
		++DPA2_Rcv_Ctrl.ReadIdx;
		DPA2_Rcv_Ctrl.ReadIdx &= 3;
	}

	return	(uchar)1;
}

/*[]----------------------------------------------------------------------[]*/
/*|             DpaSndQue_Init()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command send queue initialize.                                |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	DpaSndQue_Init( void )
{
	DPA_Snd_Ctrl.Count = 0;
	DPA_Snd_Ctrl.ReadIdx = 0;
	DPA_Snd_Ctrl.WriteIdx = 0;
	DPA_Snd_Ctrl.dummy = 0;

	DPA2_Snd_Ctrl.Count = 0;
	DPA2_Snd_Ctrl.ReadIdx = 0;
	DPA2_Snd_Ctrl.WriteIdx = 0;
	DPA2_Snd_Ctrl.dummy = 0;
}


/*[]----------------------------------------------------------------------[]*/
/*|             DpaSndQue_Set()                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command set to send queue.                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pointer for send data.                                        |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void DpaSndQue_Set(uchar *pData, ushort len, int port)
{
	if( len > RAU_DATA_MAX ){
		len = RAU_DATA_MAX;
	}
	if( port == DOWNLINE ){	// �������ւ̑��M�f�[�^���Z�b�g����B
		DPA2_Snd_Ctrl.dpa_data[DPA2_Snd_Ctrl.WriteIdx].Len = len;

		memcpy(DPA2_Snd_Ctrl.dpa_data[DPA2_Snd_Ctrl.WriteIdx].Dat, pData, len);

		++DPA2_Snd_Ctrl.Count;
		++DPA2_Snd_Ctrl.WriteIdx;
		DPA2_Snd_Ctrl.WriteIdx &= 3;
	}
	else if( port == CREDIT ){
		DPA_Credit_SndCtrl.dpa_data[ DPA_Credit_SndCtrl.WriteIdx ].Len = len;

		memcpy( DPA_Credit_SndCtrl.dpa_data[ DPA_Credit_SndCtrl.WriteIdx ].Dat, pData, len );

		++DPA_Credit_SndCtrl.Count;							// queue regist count update
		++DPA_Credit_SndCtrl.WriteIdx;						// next write index update
		DPA_Credit_SndCtrl.WriteIdx &= 3;
	} else {				// ������ւ̑��M�f�[�^���Z�b�g����B
		DPA_Snd_Ctrl.dpa_data[ DPA_Snd_Ctrl.WriteIdx ].Len = len;

		memcpy( DPA_Snd_Ctrl.dpa_data[ DPA_Snd_Ctrl.WriteIdx ].Dat, pData, len );

		++DPA_Snd_Ctrl.Count;							// queue regist count update
		++DPA_Snd_Ctrl.WriteIdx;						// next write index update
		DPA_Snd_Ctrl.WriteIdx &= 3;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|             DpaSndQue_Read()                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|     Dopa command read from send queue.                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : pointer for set command data.                                 |*/
/*| return : 1=received data exist,  0=data none                           |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar DpaSndQue_Read(uchar *pData, ushort *len, uchar kind, int port)
{
	ushort	w_len;

	if( port == DOWNLINE ){	// �������ւ̑��M�f�[�^�����o���B
		if( 0 == DPA2_Snd_Ctrl.Count ){	// �f�[�^�Ȃ�
			return	(uchar)0;
		}
		if( kind ){
			return	(uchar)1;
		}

		w_len = DPA2_Snd_Ctrl.dpa_data[DPA2_Snd_Ctrl.ReadIdx].Len;
		if( w_len > RAU_DATA_MAX ){
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memcpy(pData, DPA2_Snd_Ctrl.dpa_data[DPA2_Snd_Ctrl.ReadIdx].Dat, w_len);

		--DPA2_Snd_Ctrl.Count;
		++DPA2_Snd_Ctrl.ReadIdx;
		DPA2_Snd_Ctrl.ReadIdx &= 3;
	}
	else if( port == CREDIT ){
		if( 0 == DPA_Credit_SndCtrl.Count ){	// �f�[�^�Ȃ�
			return	(uchar)0;
		}
		if( kind ){
			return	(uchar)1;
		}

		w_len = DPA_Credit_SndCtrl.dpa_data[DPA_Credit_SndCtrl.ReadIdx].Len;
		if( w_len > RAU_DATA_MAX ){
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memset(pData, 0x00, RAU_RCV_MAX_H );
		memcpy(pData, DPA_Credit_SndCtrl.dpa_data[DPA_Credit_SndCtrl.ReadIdx].Dat, w_len);

		--DPA_Credit_SndCtrl.Count;
		++DPA_Credit_SndCtrl.ReadIdx;
		DPA_Credit_SndCtrl.ReadIdx &= 3;
	} else {
		if( 0 == DPA_Snd_Ctrl.Count ){
			return	(uchar)0;
		}
		if( kind ){									// ���M�ް��̗L���̂�����
			return	(uchar)1;
		}

		w_len = DPA_Snd_Ctrl.dpa_data[ DPA_Snd_Ctrl.ReadIdx ].Len;

		if( w_len > RAU_DATA_MAX ){
			w_len = RAU_DATA_MAX;
		}
		*len = w_len;

		memcpy(pData, DPA_Snd_Ctrl.dpa_data[ DPA_Snd_Ctrl.ReadIdx ].Dat, w_len);

		--DPA_Snd_Ctrl.Count;							// queue regist count update
		++DPA_Snd_Ctrl.ReadIdx;							// next read index update
		DPA_Snd_Ctrl.ReadIdx &= 3;
	}

	return	(uchar)1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CreateUpSocket
 *[]----------------------------------------------------------------------[]
 *| summary	: ������̃\�P�b�g���쐬����
 *[]----------------------------------------------------------------------[]
 *| return	: -1�ȊO	�쐬�����\�P�b�g�f�B�X�N���v�^
 *|			  -1		�쐬���s
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int RAU_CreateUpSocket(void)
{
	int		sock = -1;						// �\�P�b�g�f�B�X�N���v�^
	int		on = 1;							// �I�v�V�����L��
	int		maxrt;							// �^�C���A�E�g����
	struct sockaddr_in	myAddr;				// ���ǃA�h���X�E�|�[�g�ԍ�

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// �\�P�b�g�쐬
	if(sock == -1) {
		return sock;
	}
	
	// �m���u���b�L���O
	if(-1 == tfBlockingState(sock, TM_BLOCKING_OFF)) {
		tfClose(sock);
		return -1;
	}

	// TIME_WAIT�ł��|�[�g�̃I�[�v�����\�ɂ���
	if(-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))) {
		tfClose(sock);
		return -1;
	}

	// connect�̃^�C���A�E�g���Ԃ�ύX����
	maxrt = RAU_CONN_TIMEOUT;
	if(-1 == setsockopt(sock, IP_PROTOTCP, TCP_MAXRT, (char*)&maxrt, sizeof(maxrt))) {
		tfClose(sock);
		return -1;
	}

	// �\�P�b�g�C�x���g�R�[���o�b�N�֐��̓o�^
	tfRegisterSocketCB(sock, RAU_upSocketCallback, 
						TM_CB_REMOTE_CLOSE | TM_CB_SOCKET_ERROR |
						TM_CB_RESET | TM_CB_CLOSE_COMPLT | TM_CB_CONNECT_COMPLT);

	// ���ǃ|�[�g�����蓖�Ă�
	memset(&myAddr, 0x00, sizeof(myAddr));
	myAddr.sin_family      = AF_INET;
	myAddr.sin_port        = htons(RauConfig.Dpa_port_m);	// ��莩�ǃ|�[�g�ԍ�
	myAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_m);		// ����IP�A�h���X
	if(-1 == bind(sock, (struct sockaddr *)&myAddr, sizeof(myAddr))) {
		tfClose(sock);
		sock = -1;
	}
	
	return sock;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CreateDownSocket
 *[]----------------------------------------------------------------------[]
 *| summary	: �������̃\�P�b�g���쐬����
 *[]----------------------------------------------------------------------[]
 *| return	: -1�ȊO	�쐬�����\�P�b�g�f�B�X�N���v�^
 *|			  -1		�쐬���s
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int RAU_CreateDownSocket(void)
{
	int		sock = -1;						// �\�P�b�g�f�B�X�N���v�^
	struct sockaddr_in	myAddr;				// ���ǃA�h���X�E�|�[�g�ԍ�
	
	// Listen�p�̃\�P�b�g�쐬
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// �\�P�b�g�쐬
	if(sock == -1) {
		return sock;
	}
	
	// �m���u���b�L���O
	if(-1 == tfBlockingState(sock, TM_BLOCKING_OFF)) {
		tfClose(sock);
		return -1;
	}
	
	// ���ǃ|�[�g�����蓖�Ă�
	memset(&myAddr, 0x00, sizeof(myAddr));
	myAddr.sin_family      = AF_INET;
	myAddr.sin_port        = htons(RauConfig.Dpa_port_m2);	// ���莩�ǃ|�[�g�ԍ�
	myAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_m);		// ����IP�A�h���X
	if(-1 == bind(sock, (struct sockaddr *)&myAddr, sizeof(myAddr))) {
		tfClose(sock);
		sock = -1;
	}
	
	return sock;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_ConnectUpline
 *[]----------------------------------------------------------------------[]
 *| summary	: �������TCP�ڑ����J�n����
 *[]----------------------------------------------------------------------[]
 *| return	: -1�ȊO	�m���J�n
 *|			  -1		�m�����s
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
int	RAU_ConnectUpline(void)
{
	struct sockaddr_in	serverAddr;			// HOST�A�h���X�E�|�[�g�ԍ�
	int	sockError;							// �\�P�b�g�G���[�R�[�h
	int	ret;
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family      = AF_INET;
	serverAddr.sin_port        = htons(RauConfig.Dpa_port_h);	// ����HOST�|�[�g�ԍ�
	serverAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_h);		// HOSTIP�A�h���X
	
	ret = connect(RAU_upSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(ret == -1) {
		sockError = tfGetSocketError(RAU_upSocket);
		if(sockError == TM_EINPROGRESS ||			// �m���u���b�L���O�Őڑ����̓G���[�ł͂Ȃ�
			sockError == TM_EISCONN) {				// ���ɐڑ����̓G���[�ł͂Ȃ�(connect�`GetSockError�ԂŐڑ�����)
			ret = 0;								// �G���[�Ƃ��Ȃ�
		}
	}
	if(ret == 0) {
		RAU_TcpConnect = 1;
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CloseUpline
 *[]----------------------------------------------------------------------[]
 *| summary	: �������TCP�ڑ���ؒf����
 *| param	: resetReq	TRUE:TCP�R�l�N�V�����v���̃N���A
 *[]----------------------------------------------------------------------[]
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_CloseUpline(BOOL resetReq)
{
	tfClose(RAU_upSocket);					// �\�P�b�g�N���[�Y
	RAU_upSockCondition = RAU_SOCK_CLOSING;	// �N���[�Y�C�x���g�҂�
	RauCtrl.tcpConnectFlag.port.upload = 0;

	// TIME_WAIT�����҂��^�C�}(�N��)
	if(RauConfig.Dpa_discnct_tm != 0){
		RAU_Tm_TCP_TIME_WAIT.bits0.bit_0_13 = RauConfig.Dpa_discnct_tm;
		RAU_f_TCPtime_wait_rq = 1;
		RAU_f_TCPtime_wait_ov = 0;
	}
	else{
		RAU_f_TCPtime_wait_rq = 0;
		RAU_f_TCPtime_wait_ov = 1;
	}
	if(resetReq) {
		RAUhost_TcpConnReq_Clear();
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CloseDownline
 *[]----------------------------------------------------------------------[]
 *| summary	: ��������TCP�ڑ���ؒf����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_CloseDownline(void)
{
	tfClose(RAU_downSocket);					// �\�P�b�g�N���[�Y
	RAU_downSockCondition = RAU_SOCK_CLOSING;	// �N���[�Y�C�x���g�҂�
	RauCtrl.tcpConnectFlag.port.download = 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_downSocketCallback
 *[]----------------------------------------------------------------------[]
 *| summary	: ������̃\�P�b�g�C�x���g�R�[���o�b�N�֐�
 *[]----------------------------------------------------------------------[]
 *| param	: sock		�\�P�b�g�f�B�X�N���v�^
 *|			  event		�ʒm�C�x���g
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void 	RAU_upSocketCallback(int sock, int event)
{
	if(RAU_upSocket != sock) {									// �\�P�b�g���قȂ�܂��͊��ɃN���[�Y�ς݁H
		return;
	}
	
	if( (event & TM_CB_RESET) || (event & TM_CB_REMOTE_CLOSE) || (event & TM_CB_SOCKET_ERROR) ) {
			if(RAU_upSocket != -1) {
				if(RAU_TcpConnect == 1) {
					if(RauCtrl.connRetryCount >= RAU_CONN_RETRY_MAX) {	// connect���g���C�I�[�o�[
						RAUhost_SetError(ERR_RAU_HOST_COMMUNICATION);
						RAU_CloseUpline(TRUE);
						RauCtrl.TCPcnct_req = 0;						// TCP�ڑ��v���N���A
						RauCtrl.connRetryCount = 0;						// ���g���C�񐔃N���A
						// �f�[�^�đ��҂��^�C�}�N��
						RAU_Tm_data_rty.bits0.bit_0_13 = RauConfig.Dpa_data_rty_tm;	
						RAU_f_data_rty_rq = 1;
						RAU_f_data_rty_ov = 0;
						RAU_TcpConnect = 0;
					}
					else {
						RAUhost_SetError(ERR_RAU_TCP_CONNECT);			// �G���[�R�[�h�X�P(������s�b�o�R�l�N�V�����^�C���A�E�g)��ʒm
						tfClose(RAU_upSocket);
						RAU_upSockCondition = RAU_SOCK_CLOSING;
						++RauCtrl.connRetryCount;
					}
				}
				else {
					RAU_upSockCondition = RAU_SOCK_CLOSED;		/* �N���[�Y������Ԃֈڍs				*/
					RAU_CloseUpline(TRUE);
// MH322914 (s) kasiyama 2016/07/22 �ؒf��r���u���b�N���瑗�M���Ă��܂����̉��P[���ʃo�ONo.1311]
					// �؂ꂽ�F���̏ꍇ�́A�N���A����iRAU_CloseUpline�֐�����RAUhost_TcpConnReq_Clear�֐����ĂԂ�RauCtrl.TCPcnct_req��0�ł��邽�߁A�N���A����Ȃ��̂ŁA�����ŃN���A����j
					RAU_uc_retrycnt_h = 0;										// ������̑��M���g���C�J�E���^�N���A
					RAU_Tm_Ackwait.tm = 0;										// ������̂`�b�j��M�҂��^�C�}�N���A
					RAU_uc_retrycnt_reconnect_h = 0;							// ������̑��M���g���C�J�E���^�N���A
					RAU_Tm_Ackwait_reconnect.tm = 0;							// ������̍Đڑ��҂��^�C�}�N���A
			
					RAUque_DeleteQue();										// �e�[�u�����瑗�M���������f�[�^�̂ݏ����B
					pRAUhost_SendIdInfo = NULL;								// �e�[�u���̎Q�Ƃ���߂�B
			
					RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);			// ���M�V�[�P���X�O��(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)�ɖ߂��B
			
					RAU_ui_data_length_h = 0;									// ��M�f�[�^���N���A
					RAU_uc_mode_h = S_H_IDLE;									// �`�b�j�҂����������ăA�C�h���Ɉڍs
// MH322914 (e) kasiyama 2016/07/22 �ؒf��r���u���b�N���瑗�M���Ă��܂����̉��P[���ʃo�ONo.1311]
					RAU_TcpConnect = 0;
				}
			}
			else {
				RAU_upSockCondition = RAU_SOCK_CLOSED;			/* �N���[�Y������Ԃֈڍs				*/
				RAU_TcpConnect = 0;
			}
	}
	else if (event & TM_CB_CLOSE_COMPLT) {
			RAU_upSockCondition = RAU_SOCK_CLOSED;				/* �N���[�Y������Ԃֈڍs				*/
			RAU_upSocket = -1;
	}
	else if (event & TM_CB_CONNECT_COMPLT) {
			RauCtrl.TCPcnct_req = 0;							// TCP�ڑ��v���N���A
			RauCtrl.connRetryCount = 0;							// ���g���C�񐔃N���A
			RAU_TcpConnect = 0;
			RAU_upSockCondition = RAU_SOCK_CONNECTED;			/* �R�l�N�g������Ԃֈڍs				*/
	}
	return;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_downSocketCallback
 *[]----------------------------------------------------------------------[]
 *| summary	: �������̃\�P�b�g�C�x���g�R�[���o�b�N�֐�
 *[]----------------------------------------------------------------------[]
 *| param	: sock		�\�P�b�g�f�B�X�N���v�^
 *|			  event		�ʒm�C�x���g
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void 	RAU_downSocketCallback(int sock, int event)
{
	if(RAU_downSocket != sock) {								// �\�P�b�g���قȂ�܂��͊��ɃN���[�Y�ς݁H
		return;
	}
	
	if( (event & TM_CB_RESET) || (event & TM_CB_REMOTE_CLOSE) || (event & TM_CB_SOCKET_ERROR) ) {
		if(RAU_downSocket != -1) {
			RAU_CloseDownline();
		}
		else {
			RAU_upSockCondition = RAU_SOCK_CLOSED;				/* �N���[�Y������Ԃֈڍs				*/
		}
	}
	else if (event & TM_CB_CLOSE_COMPLT) {
		RAU_downSockCondition = RAU_SOCK_CLOSED;				/* �N���[�Y������Ԃֈڍs				*/
		RAU_downSocket = -1;
	}
	return;
}
void	SetUpNetState(uchar state)
{
	RauCtrl.upNetState = state;
}

uchar	GetUpNetState(void)
{
	return RauCtrl.upNetState;
}

void	SetDownNetState(uchar state)
{
	RauCtrl.downNetState = state;
}

uchar	GetDownNetState(void)
{
	return RauCtrl.downNetState;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_SetAntennaLevelState
 *[]----------------------------------------------------------------------[]
 *| summary	: �A���e�i���x���`�F�b�N�J�n�E�I�����w������
 *[]----------------------------------------------------------------------[]
 *| param	: state		0:�J�n 1:�I��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_SetAntennaLevelState(uchar	state)
{
	RAU_AntennaLevelState = state;

	if(RAU_AntennaLevelState == 1) {						// �A���e�i���x���`�F�b�N�J�n
		if(RauCtrl.tcpConnectFlag.port.upload == 1) {		// ������ڑ���
			RAU_CloseUpline(TRUE);
		}
		
		if(RauCtrl.tcpConnectFlag.port.download == 1) {		// �������ڑ���
			RAU_CloseDownline();
			tfClose(RAU_downListenSocket);
			RAU_downListenSocket = -1;
		}
		if(RauCtrl.tcpConnectFlag.port.credit == 1) {		// �N���W�b�g����ڑ���
			RAU_CloseCreditline(TRUE);
		}
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetAntennaLevelState
 *[]----------------------------------------------------------------------[]
 *| summary	: �A���e�i���x���`�F�b�N��Ԃ��擾����
 *[]----------------------------------------------------------------------[]
 *| param	: state		0:�J�n 1:�I��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	RAU_GetAntennaLevelState(void)
{
	return RAU_AntennaLevelState;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CreditSockProc
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g����\�P�b�g����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	RAU_LINE_STATE	RAU_CreditSockProc(void)
{
	int ret;
	int		sockError;
	ushort	sendLen;										// ���M�f�[�^��
	RAU_LINE_STATE state = RAU_LINE_NO_ERROR;

	switch(RAU_creditSockCondition) {
	case RAU_SOCK_INIT:										// ������
		if(KSG_gPpp_RauStarted) {							// PPP�ڑ����H
			RAU_creditSockCondition = RAU_SOCK_OPEN;
			CreditCtrl.connRetryCount = 0;						// ���g���C�񐔃N���A
		}
		break;
	case RAU_SOCK_OPEN:										// �\�P�b�g�I�[�v��
		if(RAU_AntennaLevelState) {							// �A���e�i���x���`�F�b�N��
			RAU_creditSockCondition = RAU_SOCK_CLOSED;
			CreditCtrl.connRetryCount = 0;						// ���g���C�񐔃N���A
			break;
		}
		if(KSG_gPpp_RauStarted) {
			if(RAU_creditSocket != -1) {
				tfClose(RAU_creditSocket);
				RAU_creditSocket = -1;
				Credit_TcpConnReq_Clear();
			}
			if(CreditCtrl.TCPcnct_req == 1) {					// TCP�ڑ��v������
//				if(Credit_data_rty_ov) {						// �f�[�^�đ��҂��^�C�}���^�C���A�E�g����܂ł͐ڑ����J�n���Ȃ��B
					RAU_creditSocket = RAU_CreateCreditSocket();		// ������\�P�b�g�쐬
					if(RAU_creditSocket != -1) {
						if(-1 != RAU_ConnectCreditline()) {
							// TCP�ڑ��J�n
							RAU_creditSockCondition = RAU_SOCK_CONNECTING;
						}
						else {
							tfClose(RAU_creditSocket);
							RAU_creditSockCondition = RAU_SOCK_CLOSING;
						}
					}
//				}
//				else {
//					Credit_TcpConnReq_Clear();
//					CreditCtrl.TCPcnct_req = 0;				// �ڑ��v�����N���A����
//					CreditCtrl.connRetryCount = 0;				// ���g���C�񐔃N���A
//				}
			}
			else {
				CreditCtrl.connRetryCount = 0;					// ���g���C�񐔃N���A
			}
		}
		else {
			CreditCtrl.connRetryCount = 0;						// ���g���C�񐔃N���A
		}
		break;
	case RAU_SOCK_CONNECTING:								// TCP�m����
		// �������Ȃ�
		break;
	case RAU_SOCK_CONNECTED:								// TCP�m��
		RAU_creditSockCondition = RAU_SOCK_IDLE;
		RauCtrl.tcpConnectFlag.port.credit = 1;
		break;
	case RAU_SOCK_LISTENING:								// TCP�ڑ��҂�
		break;
	case RAU_SOCK_IDLE:										// TCP�ڑ���
		if(RAU_creditSocket != -1){
			CreditCtrl.TCPcnct_req = 0;						// ���ɐڑ����Ȃ̂�TCP�ڑ��v�����N���A
			// ���M����
			if(DpaSndQue_Read(RAU_SendBuff, &sendLen, 0, CREDIT)) {	// ���M�f�[�^����H
				ret = send(RAU_creditSocket, (char*)RAU_SendBuff, (int)sendLen, 0);
				if(ret == -1) {
					sockError = tfGetSocketError(RAU_creditSocket);
					if(sockError == TM_EWOULDBLOCK) {		// ��M�f�[�^�Ȃ�(�m���u���b�L���O�̂��߃G���[�ł͂Ȃ�)
						;
					}
					else {
						RAU_CloseCreditline(TRUE);
						break;
					}
				}
				else if(ret > RAU_DATA_MAX) {
					Credit_SetError(ERR_CREDIT_FMA_SEND_LEN);	// ��������M�f�[�^���ُ�(92)
					RAU_CloseCreditline(TRUE);
					break;
				}
				state = RAU_LINE_SEND;
				
//				// ���ʐM�^�C�}�N�����Ȃ�A���M�������߃^�C�}����������
//				if( RAU_f_No_Action_rq == 1 ){
//					RAU_Tm_No_Action.bits0.bit_0_13 = RauConfig.Dpa_nosnd_tm;
//					RAU_f_No_Action_ov = 0;
//				}
			}
			
			// ��M����
			ret = recv(RAU_creditSocket, (char*)RAU_RecvBuff, RAU_DATA_MAX, 0);	// �f�[�^��M
			if(ret > 0) {									// ��M�f�[�^����
				if(ret > RAU_DATA_MAX) {
					Credit_SetError(ERR_CREDIT_FMA_RECV_LEN);	// �������M�f�[�^���ُ�(93)
					ret = RAU_DATA_MAX;
				}
				// ��M�f�[�^���i�[����
				RAU_DpaRcvQue_Set(RAU_RecvBuff, (ushort)ret, CREDIT);
			}
			else if(ret == -1) {							// �G���[����?
				sockError = tfGetSocketError(RAU_creditSocket);
				if(sockError == TM_EWOULDBLOCK) {			// ��M�f�[�^�Ȃ�(�m���u���b�L���O�̂��߃G���[�ł͂Ȃ�)
					;
				}
				else {										// �G���[����
					RAU_CloseCreditline(TRUE);
				}
			}
		}
		break;
	case RAU_SOCK_CLOSING:									// TCP�ؒf��
		// �������Ȃ�
		if(RAU_creditSocket == -1) {
			RAU_creditSockCondition = RAU_SOCK_CLOSED;
		}
		break;
	case RAU_SOCK_CLOSED:									// TCP�ؒf
		RauCtrl.tcpConnectFlag.port.credit = 0;
		if(KSG_gPpp_RauStarted) {
			RAU_creditSockCondition = RAU_SOCK_OPEN;
		}
		else if(RAU_AntennaLevelState == 1) {				// �A���e�i���x���`�F�b�N��
			;												// �������Ȃ�(RAU_SOCK_CLOSED�̂܂�)
		}
		else {
			RAU_creditSockCondition = RAU_SOCK_INIT;
		}
		break;
	default:
		break;
	}
	
	return state;
	
}

static	int RAU_CreateCreditSocket(void)
{
	int		sock = -1;						// �\�P�b�g�f�B�X�N���v�^
	int		on = 1;							// �I�v�V�����L��
	int		maxrt;							// �^�C���A�E�g����
	struct sockaddr_in	myAddr;				// ���ǃA�h���X�E�|�[�g�ԍ�

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// �\�P�b�g�쐬
	if(sock == -1) {
		return sock;
	}
	
	// �m���u���b�L���O
	if(-1 == tfBlockingState(sock, TM_BLOCKING_OFF)) {
		tfClose(sock);
		return -1;
	}

	// TIME_WAIT�ł��|�[�g�̃I�[�v�����\�ɂ���
	if(-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))) {
		tfClose(sock);
		return -1;
	}

	// connect�̃^�C���A�E�g���Ԃ�ύX����
	maxrt = RAU_CONN_TIMEOUT;
	if(-1 == setsockopt(sock, IP_PROTOTCP, TCP_MAXRT, (char*)&maxrt, sizeof(maxrt))) {
		tfClose(sock);
		return -1;
	}

	// �\�P�b�g�C�x���g�R�[���o�b�N�֐��̓o�^
	tfRegisterSocketCB(sock, RAU_creditSocketCallback, 
						TM_CB_REMOTE_CLOSE | TM_CB_SOCKET_ERROR |
						TM_CB_RESET | TM_CB_CLOSE_COMPLT | TM_CB_CONNECT_COMPLT);

	// ���ǃ|�[�g�����蓖�Ă�
	memset(&myAddr, 0x00, sizeof(myAddr));
	myAddr.sin_family      = AF_INET;
	myAddr.sin_port        = htons(RauConfig.Dpa_port_m3);	// ��莩�ǃ|�[�g�ԍ�
	myAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_m);		// ����IP�A�h���X
	if(-1 == bind(sock, (struct sockaddr *)&myAddr, sizeof(myAddr))) {
		tfClose(sock);
		sock = -1;
	}
	
	return sock;
	
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_ConnectUpline
 *[]----------------------------------------------------------------------[]
 *| summary	: �������TCP�ڑ����J�n����
 *[]----------------------------------------------------------------------[]
 *| return	: -1�ȊO	�m���J�n
 *|			  -1		�m�����s
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	int	RAU_ConnectCreditline(void)
{
	struct sockaddr_in	serverAddr;			// HOST�A�h���X�E�|�[�g�ԍ�
	int	sockError;							// �\�P�b�g�G���[�R�[�h
	int	ret;
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family      = AF_INET;
	serverAddr.sin_port        = htons(RauConfig.Dpa_port_Cre);	// �N���W�b�g�T�[�o�[�|�[�g�ԍ�
	serverAddr.sin_addr.s_addr = htonl(RauConfig.Dpa_IP_Cre);		// HOSTIP�A�h���X
	
	ret = connect(RAU_creditSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(ret == -1) {
		sockError = tfGetSocketError(RAU_creditSocket);
		if(sockError == TM_EINPROGRESS ||			// �m���u���b�L���O�Őڑ����̓G���[�ł͂Ȃ�
			sockError == TM_EISCONN) {				// ���ɐڑ����̓G���[�ł͂Ȃ�(connect�`GetSockError�ԂŐڑ�����)
			ret = 0;								// �G���[�Ƃ��Ȃ�
		}
	}
	if(ret == 0) {
		Credit_TcpConnect = 1;
	}
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_CloseUpline
 *[]----------------------------------------------------------------------[]
 *| summary	: �������TCP�ڑ���ؒf����
 *| param	: resetReq	TRUE:TCP�R�l�N�V�����v���̃N���A
 *[]----------------------------------------------------------------------[]
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
static	void	RAU_CloseCreditline(BOOL resetReq)
{
	tfClose(RAU_creditSocket);					// �\�P�b�g�N���[�Y
	RAU_creditSockCondition = RAU_SOCK_CLOSING;	// �N���[�Y�C�x���g�҂�
	RauCtrl.tcpConnectFlag.port.credit = 0;
	CreditCtrl.TCPdiscnct_req = 0;
	
	// TIME_WAIT�����҂��^�C�}(�N��)
	if(RauConfig.Credit_discnct_tm != 0){
		CRE_tm_TCP_TIME_WAIT.bits0.bit_0_13 = RauConfig.Credit_discnct_tm;
		Credit_TCPtime_wait_rq = 1;
		Credit_TCPtime_wait_ov = 0;
	}
	else{
		Credit_TCPtime_wait_rq = 0;
		Credit_TCPtime_wait_ov = 1;
	}
	if(resetReq) {
		Credit_TcpConnReq_Clear();
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_creditSocketCallback
 *[]----------------------------------------------------------------------[]
 *| summary	: ������̃\�P�b�g�C�x���g�R�[���o�b�N�֐�
 *[]----------------------------------------------------------------------[]
 *| param	: sock		�\�P�b�g�f�B�X�N���v�^
 *|			  event		�ʒm�C�x���g
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void 	RAU_creditSocketCallback(int sock, int event)
{
	if(RAU_creditSocket != sock) {									// �\�P�b�g���قȂ�܂��͊��ɃN���[�Y�ς݁H
		return;
	}
	
	if( (event & TM_CB_RESET) || (event & TM_CB_REMOTE_CLOSE) || (event & TM_CB_SOCKET_ERROR) ) {
			if(RAU_creditSocket != -1) {
				if(Credit_TcpConnect == 1) {
					if(CreditCtrl.connRetryCount >= RauConfig.Credit_cnct_rty_cnt) {	// connect���g���C�I�[�o�[
						Credit_SetError(ERR_CREDIT_HOST_COMMUNICATION);
						RAU_CloseCreditline(TRUE);
						CreditCtrl.TCPcnct_req = 0;						// TCP�ڑ��v���N���A
						CreditCtrl.connRetryCount = 0;						// ���g���C�񐔃N���A
						// �f�[�^�đ��҂��^�C�}�N��
						Credit_TcpConnect = 0;
						/* �ڑ����s�͑��M�v���̎��s�Ȃ̂ŃG���[�C�x���g�𓊂��� */
						queset(OPETCBNO, CRE_EVT_SNDERR, 0, NULL);
					}
					else {
						Credit_SetError(ERR_CREDIT_TCP_CONNECT);			// �G���[�R�[�h�X�P(�s�b�o�R�l�N�V�����^�C���A�E�g)��ʒm
						tfClose(RAU_creditSocket);
						RAU_creditSockCondition = RAU_SOCK_CLOSING;
						++CreditCtrl.connRetryCount;
					}
				}
				else {
					RAU_creditSockCondition = RAU_SOCK_CLOSED;		/* �N���[�Y������Ԃֈڍs				*/
					RAU_CloseCreditline(TRUE);
					Credit_TcpConnect = 0;
				}
			}
			else {
				RAU_creditSockCondition = RAU_SOCK_CLOSED;			/* �N���[�Y������Ԃֈڍs				*/
				Credit_TcpConnect = 0;
			}
	}
	else if (event & TM_CB_CLOSE_COMPLT) {
			RAU_creditSockCondition = RAU_SOCK_CLOSED;				/* �N���[�Y������Ԃֈڍs				*/
			RAU_creditSocket = -1;
	}
	else if (event & TM_CB_CONNECT_COMPLT) {
			CreditCtrl.TCPcnct_req = 0;							// TCP�ڑ��v���N���A
			CreditCtrl.connRetryCount = 0;							// ���g���C�񐔃N���A
			Credit_TcpConnect = 0;
			RAU_creditSockCondition = RAU_SOCK_CONNECTED;			/* �R�l�N�g������Ԃֈڍs				*/
	}
	return;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_GetUpNetState
 *[]----------------------------------------------------------------------[]
 *| summary	: ������̏�Ԃ��擾����
 *| retrun	: RAU_NET_IDLE				���ڑ�
 *|			  RAU_NET_IDLE				PPP�m����
 *|			  RAU_NET_TCP_CONNECTIED	TCP�m����
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
RAU_NET_STATE	RAU_GetCreditNetState(void)
{
	if(KSG_gPpp_RauStarted) {						// PPP�m����
		if(RauCtrl.tcpConnectFlag.port.credit) {	// �����TCP�m����
			return RAU_NET_TCP_CONNECTIED;
		}
		else {
			return RAU_NET_PPP_OPENED;
		}
	}
	
	return RAU_NET_IDLE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_ClearLineState
 *[]----------------------------------------------------------------------[]
 *| summary	: �����Ԃ��N���A����
 *| retrun	: none
 *[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
void	RAU_ClearLineState(void)
{

	// �_�~�[�f�[�^�A�����M�񐔃N���A
	RAU_SendDummyContinueCount = 0;
}
void RAU_StopDownline(void)
{
	// RAU_downSocket��close
	RAU_CloseDownline();
	// RAU_downListenSocket��close
	tfClose(RAU_downListenSocket);
	RAU_downListenSocket = -1;
	// ��ԑJ�ڂ��Ȃ��悤�ɂ��邽�߁ARAU_SOCK_CONNECTED���Z�b�g����
	RAU_downSockCondition = RAU_SOCK_CONNECTED;
}
