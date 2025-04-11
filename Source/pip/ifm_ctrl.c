/*[]----------------------------------------------------------------------[]*/
/*| PARKiPRO�Ή�                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Art System                                               |*/
/*| Date        : 2007-02-28                                               |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"common.h"
#include	"ifm_ctrl.h"
#include	"prm_tbl.h"
#include	"message.h"
#include	"mem_def.h"
#include	"tbl_rkn.h"
#include	"flp_def.h"
#include	"ntnet_def.h"
#include	"mnt_def.h"
#include	"ope_ifm.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"lkmain.h"
#include	"pip_def.h"


//**************************************************************************
//	COMMON FLAG
//**************************************************************************
ushort	pipcomm;			// == 02-0025�B:�r���ŕύX�����Ɠ���ł��Ȃ��̂Ńt���O��

//**************************************************************************
//	DATA PART
//**************************************************************************

// ���M�f�[�^�ڍ�
typedef struct {
	char	ID1[2];			// �f�[�^�u���b�NNo.
	char	ID2[2];			// �ŏI�u���b�N����
	char	ID3[2];			// �f�[�^���
	uchar	dat[IFM_DATA_MAX];
} t_DataDetail;
t_DataDetail	blk_data;	// ���M�p�f�[�^(1�u���b�N��)
Receipt_data	Receipt_buf;
DISCOUNT_DATA	DiscountData_Buff[NTNET_DIC_MAX];				// �����֘A�ϊ��p�̈�
#define LOGTBL_MAX	20		// ���ZLOG�A�G���[LOG�̍ő�ۑ�����

//---------------------------------------------------------------------------
#define	_TOTAL_SIZE(m)	(ushort)(m->mlen+(sizeof(tIFMMSG)-1))
#define	_MSG_LEN(size)	(ushort)(size-(sizeof(tIFMMSG)-1))

// module start flag

static	tIFM_RCV_BUF	ifm_rcv_buf;

//#pragma section		_UNINIT2
static	tIFM_SND_BUF	ifm_snd_buf;
static	tIFM_SND_CTR	ifm_snd_ctr;

//---------------------------------------------------------------------------
// serial no. for each messages
static ushort PayDataSerialNo;
static ushort PayReceiptSerialNo;

// model code
// MH810100(S)
//static	const	char	MODEL_CODE[] = MODEL_CODE_FT4000;
static	const	char	MODEL_CODE[] = MODEL_CODE_GT4100;
// MH810100(E)

static	ushort	ValidRoomNum;
static	int		ValidRoomNo[LOCK_MAX];

//---------------------------------------------------------------------------
// payment data work for search
static	Receipt_data	wkReceipt;

struct _receiptkey {
	ushort			PayMethod;			// ���Z���@
	ushort			PayClass;			// �����敪
	ushort			PayMode;			// ���ZӰ��
	ushort			OutKind;			// ���Z�o��
	ulong			PayCount;			// ���Z�ǂ���
	date_time_rec	OutTime;			// ��������
	ulong			WPlace;				// ���Ԉʒu�ް�
	ulong			ParkingNo;			// ��������ԏꇂ
	ushort			PassKind;			// ��������
	ulong			PassID;				// �����ID
};

// request message cache for operation
static	struct {
	tIFMMSG44		msg44;
	tIFMMSG76		msg76;
} cache;

//**************************************************************************
//	internal functions
//**************************************************************************

static	void	div_data(void);
static	void	send_oneblk(void);

static	void	ifm_send_msg(void);
static	tIFMMSG	*get_msgarea_and_init(int id, ushort size);

static	uchar	chk_BasicData(tIFMMSG_BASIC* dat, int wild);
static	uchar	chk_CarInfo(t_ShedInfo* dat, ushort* pos);
static	void	make_BasicData(tIFMMSG_BASIC* dat);
static	void	set_MMDDhhmm(uchar *date);
static	void	set_StatData(tM_STATUS* dat);
static	void	set_ShedStat(t_ShedStat* dat, int index);
// MH322914(S) K.Onodera 2016/09/12 AI-V�Ή��F�U�֐��Z
//static	uchar	get_crm_state(void);
// MH322914(E) K.Onodera 2016/09/12 AI-V�Ή��F�U�֐��Z
static	void	get_valid_roomnum(void);
static	void	select_discount_data(DISCOUNT_DATA* dst, const DISCOUNT_DATA* src);	// ��������I�ʂ���
static	int		set_discount_data(t_DiscountInfo* dst, DISCOUNT_DATA* src);
// MH810103 GG119202(S) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�
static	int		set_emoney_data(t_DiscountInfo* dst, Receipt_data* pay);
// MH810103 GG119202(E) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�

static	uchar	ifm_send_data62(void);
static	uchar	ifm_send_data63(void);
static	uchar	ifm_send_data64(void);
static	uchar	ifm_send_data65(ushort pos);
static	uchar	ifm_send_data66(void);
static	void	ifm_send_rsp(int id, tIFMMSG *msg, uchar rslt);

static	void	ifm_recv_data30(tIFMMSG30 *msg);
static	void	ifm_recv_data40(tIFMMSG40 *msg);
static	void	ifm_recv_data41(tIFMMSG41 *msg);
static	void	ifm_recv_data42(tIFMMSG42 *msg);
static	uchar	get_receipt_key(t_PayInfo *msg, struct _receiptkey *key);
static	uchar	search_receipt_data(struct _receiptkey *key, Receipt_data *wk);
static	void	ifm_recv_data43(tIFMMSG43 *msg);
static	void	ifm_recv_data44(tIFMMSG44 *msg);
static	uchar	ifm_check_furikae(tIFMMSG46 *msg, tIFMMSG76 *rsp, struct VL_FRS *frs);
static	void	set_sw_info(tSWP_INFO *info, uchar rslt, struct VL_FRS *frs);
static	void	ifm_recv_data45(tIFMMSG45 *msg);
static	void	ifm_recv_data46(tIFMMSG46 *msg);
static	void	ifm_recv_ibkerr(tIFMMSGB0 *msg);

static	void	ifm_recv_msg(void);

extern	short	LKopeGetLockNum( uchar, ushort, ushort * );
// MH322914(S) K.Onodera 2016/09/12 AI-V�Ή��F�U�֐��Z
extern	ushort	PiP_FurikaeSts;		// �U�֐��Z�X�e�[�^�X�t���O
// MH322914(E) K.Onodera 2016/09/12 AI-V�Ή��F�U�֐��Z

/*[]----------------------------------------------------------------------[]
 *|	name	: divide message into RAU blocks
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^�������쐬
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static void div_data(void)
{	
	ushort wrk;

	ifm_snd_ctr.blk_num = ifm_snd_buf.buff.mlen / IFM_DATA_MAX;
	wrk = ifm_snd_buf.buff.mlen % IFM_DATA_MAX;
	if (wrk) {
		ifm_snd_ctr.blk_num += 1;
		ifm_snd_ctr.last_blk_size = wrk;
	}
	else {
		ifm_snd_ctr.last_blk_size = IFM_DATA_MAX;
	}
	
	// �ŏI�u���b�N�̃o�C�g�␳�T�C�Y��ݒ�
	ifm_snd_ctr.lb_revised_size = ifm_snd_ctr.last_blk_size + 6;
	wrk = (ifm_snd_ctr.lb_revised_size) % 8;
	if (wrk) {
		ifm_snd_ctr.lb_revised_size += (8 - wrk);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: send one block
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�f�[�^���u���b�N�ɂ킯�A�V���A�����M�L���[�֏�������
 *[]----------------------------------------------------------------------[]*/
static	void	send_oneblk()
{
	ushort			size;
	uchar			*wrk;
	uchar 			i;

	div_data();	// �f�[�^�������쐬
	
	// ���M�f�[�^�쐬
	for( i = 1 ; i <= ifm_snd_ctr.blk_num ; i++ ){
		memset(&blk_data, '0', sizeof(blk_data));
		blk_data.ID1[0] += (char)(i / 10);
		blk_data.ID1[1] += (char)(i % 10);
		memcpy(blk_data.ID3, ifm_snd_buf.buff.ID, 2);

		wrk = ifm_snd_buf.buff.data;
		wrk += IFM_DATA_MAX*(i-1);	// ���u���b�N�̃A�h���X�Ɉړ�

		if (i < ifm_snd_ctr.blk_num) {	// �ŏI�u���b�N�ȊO�̏ꍇ
			memcpy(blk_data.dat, wrk, IFM_DATA_MAX);
			size = IFM_DATA_MAX + 6;
		} else {											// �ŏI�u���b�N�̏ꍇ
			memcpy(blk_data.ID2, "01", 2);	// �ŏI�u���b�N
			memcpy(blk_data.dat, wrk, (size_t)ifm_snd_ctr.last_blk_size);
			size = ifm_snd_ctr.lb_revised_size;
		}
		PIP_SciSendQue_insert((unsigned char *)&blk_data, size);	// �V���A�����M�L���[�ւ̃f�[�^�ǉ�
	}
}

//**************************************************************************
//	IFM MESSAGING PART
//**************************************************************************
/*[]----------------------------------------------------------------------[]
 *|	name	: send IFM message
 *[]----------------------------------------------------------------------[]
 *| summary	: IFM�d�����M
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	ifm_send_msg(void)
{
	tIFMMSG			*msg;
	ushort	mlen;
	uchar	*src;

	msg  = (tIFMMSG*)&ifm_snd_buf.buff;
	mlen = msg->mlen;
	src  = msg->data + mlen - 1;
	
	while( *src == '0' && mlen) {	// skip following '0'
		src--;
		mlen--;
	}
	ifm_snd_buf.buff.mlen = mlen;	// copy into send buffer
	send_oneblk();					// send start
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get send message area
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�d���G���A�m��
 *| param	: id - �f�[�^���
 *| 		: size - �d���̈�T�C�Y�i�d�����A�f�[�^��ʗ̈捞�݁j
 *| return	: tIFMMSG area
 *[]----------------------------------------------------------------------[]*/
static	tIFMMSG	*get_msgarea_and_init(int id, ushort size)
{
	tIFMMSG			*msg;
	ushort			mlen;

	msg = (tIFMMSG*)&ifm_snd_buf;

// init message area
	mlen = _MSG_LEN(size);		/* calc. maximun length of the message */
	msg->mlen = mlen;
	msg->ID[0] = (uchar)((id / 10) | '0');
	msg->ID[1] = (uchar)((id % 10) | '0');

	memset(msg->data, '0', (size_t)mlen);

	return msg;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: check Basic Data
 *[]----------------------------------------------------------------------[]
 *| summary	: ��{�f�[�^�̃`�F�b�N
 *| param	: *dat - ��{�f�[�^
 *| 		: wild - 1�Ȃ�e���ڂ�"0"������
 *| return	: IFM�G���[�R�[�h
 *[]----------------------------------------------------------------------[]*/
uchar chk_BasicData(tIFMMSG_BASIC *dat, int wild)
{
	int wrk;

	// ���ԏꇂ
	if(!wild){									// �`�F�b�N����
		wrk = (int)astoinl(dat->ParkingNo, sizeof(dat->ParkingNo));
		if( prm_get(COM_PRM,S_SYS,1,6,1) >= GTF_PKNO_LOWER ){	// GT�t�H�[�}�b�g
			if (wrk != ((int)prm_get(COM_PRM,S_SYS,1,3,1))) {
				return _IFM_ANS_INVALID_PARAM;
			}
		}else{													// APS�t�H�[�}�b�g
			if( (wrk == 0)||(wrk != CPrmSS[S_SYS][1]) ){
				return _IFM_ANS_INVALID_PARAM;
			}
		}
	}

	// �@�B��
	if(!wild){									// �`�F�b�N����
		wrk = (int)astoinl(dat->MachineNo, sizeof(dat->MachineNo));
		if (wrk != CPrmSS[S_PAY][2]) {			// ��M���ݒ�
			return _IFM_ANS_INVALID_PARAM;
		}
	}

	// �@����
	if(!wild){
		if (memcmp(dat->ModelCode, MODEL_CODE, sizeof(dat->ModelCode)) != 0) {
			return _IFM_ANS_INVALID_PARAM;
		}
	}

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: check car info
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ����`�F�b�N
 *| param	: *dat - �Ԏ����
 *| param	: *pos - LockInfo�ݒ�No.��Ԃ�
 *| return	: IFM�G���[�R�[�h
 *[]----------------------------------------------------------------------[]*/
uchar	chk_CarInfo(t_ShedInfo* dat, ushort* pos)
{
	uchar	area;
	ushort	no;
	ushort	index;

	area = (uchar)astoinl(dat->Area, sizeof(dat->Area));
	no   = (ushort)astoinl(dat->No, sizeof(dat->No));
	if ((area != 0) ||						// ��FT-4800�ł�0�Œ�
		((no <= 0)  || (no > 9900)))
		return _IFM_ANS_INVALID_PARAM;

	if (LKopeGetLockNum(area, no, pos) == 0)
		return _IFM_ANS_NOT_FOUND;

	index = *pos - 1;
	if (LockInfo[index].lok_syu == 0 || LockInfo[index].ryo_syu == 0)
		return _IFM_ANS_NOT_FOUND;

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: make basic data
 *[]----------------------------------------------------------------------[]
 *| summary	: ��{�f�[�^�̍쐬
 *| param	: *dat - ��{�f�[�^
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	make_BasicData(tIFMMSG_BASIC *dat)
{
	intoasl(dat->ParkingNo, (ulong)(CPrmSS[S_SYS][1]%1000), sizeof(dat->ParkingNo));		// ���ԏꇂ
	intoasl(dat->MachineNo, (ulong)CPrmSS[S_PAY][2], sizeof(dat->MachineNo));		// �@�B��
	memcpy(dat->ModelCode, MODEL_CODE, sizeof(dat->ModelCode));				// �@��R�[�h
}

/*[]----------------------------------------------------------------------[]
 *|	name	: make date & time
 *[]----------------------------------------------------------------------[]
 *| summary	: �����f�[�^�쐬
 *| param	: *dat - date area
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	set_MMDDhhmm(uchar *date)
{
	intoas(&date[0], (ushort)CLK_REC.mont, 2);
	intoas(&date[2], (ushort)CLK_REC.date, 2);
	intoas(&date[4], (ushort)CLK_REC.hour, 2);
	intoas(&date[6], (ushort)CLK_REC.minu, 2);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: set machine status data
 *[]----------------------------------------------------------------------[]
 *| summary	: ���Z�@���쐬
 *| param	: *dat - data area
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	set_StatData(tM_STATUS* dat)
{
	intoas(dat->CurTime, (ushort)CLK_REC.year, 4);
	set_MMDDhhmm(&dat->CurTime[4]);
	
	dat->Stat  += (uchar)(opncls()-1);				// �c�x�Ə�
	dat->Mode  += (OPECTL.Mnt_mod)?1:0; 			// ���Z�@���샂�[�h��
	dat->Err   += Err_onf;							// �G���[������
	dat->Alarm += Alm_onf;							// �A���[��������

	dat->Full1 = (getFullFactor(0)&0x10)?'1':'0';	// ���ԂP��
	dat->Full2 = (getFullFactor(1)&0x10)?'1':'0'; 	// ���ԂQ��
	dat->Full3 = (getFullFactor(2)&0x10)?'1':'0'; 	// ���ԂR��
	
	get_valid_roomnum();
	intoas(dat->ShedNum, (ushort)ValidRoomNum, 3);	// �L���Ԏ���
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get and set valid room number
 *[]----------------------------------------------------------------------[]
 *| summary	: �L���Ԏ��ݒ�
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	get_valid_roomnum(void)
{
	int i, cnt;
	for (i = 0, cnt = 0; i < LOCK_MAX; i++) {
		if (LockInfo[i].lok_syu != 0 && LockInfo[i].ryo_syu != 0) {
			ValidRoomNo[cnt++] = i;
		}
	}
	ValidRoomNum = cnt;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: set machine status data
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��󋵏��쐬
 *| param	: *dat  - data area
 *|           index - flap data index
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	set_ShedStat(t_ShedStat* dat, int index)
{
	intoas(dat->Area, (ushort)LockInfo[index].area, 2);			// ���
	intoasl(dat->No, LockInfo[index].posi, 4);					// �Ԏ��ԍ�

	dat->Mode  += (uchar)FLAPDT.flp_data[index].mode;			// �������[�h
	dat->Stat1 += FLAPDT.flp_data[index].nstat.bits.b00;		// �ð��1:�ԗ��L��
	dat->Stat2 += FLAPDT.flp_data[index].nstat.bits.b01;		// �ð��2:�ׯ�ߏ��
	dat->Stat3 += FLAPDT.flp_data[index].nstat.bits.b02;		// �ð��3:���쒆�H
	
	if (FLAPDT.flp_data[index].mode != FLAP_CTRL_MODE1) {
		// ���Ɏ���
		intoas(&dat->ParkTime[0], (ushort)FLAPDT.flp_data[index].mont, 2);
		intoas(&dat->ParkTime[2], (ushort)FLAPDT.flp_data[index].date, 2);
		intoas(&dat->ParkTime[4], (ushort)FLAPDT.flp_data[index].hour, 2);
		intoas(&dat->ParkTime[6], (ushort)FLAPDT.flp_data[index].minu, 2);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get crm current state
 *[]----------------------------------------------------------------------[]
 *| summary	: ���Z�@�������
 *| return	: state code
 *[]----------------------------------------------------------------------[]*/
uchar	get_crm_state(void)
{
	if (OPECTL.Mnt_mod == 0) {
		switch (OPECTL.Ope_mod) {
		case 2:						// ���ԗ������Z��
			switch (ope_imf_GetStatus()) {
			case 0:
// MH322914(S) K.Onodera 2016/09/12 AI-V�Ή��F�U�֐��Z
//				return '2';
				// �U�֐��Z��
				if( PiP_FurikaeSts ){
					return '5';
				}else{
					return '2';
				}
// MH322914(E) K.Onodera 2016/09/12 AI-V�Ή��F�U�֐��Z
			case 1:					// ���z���Z��
				return '4';
			case 2:					// �U�֐��Z��
				return '5';
			default:
				break;
			}
			break;

		case 3:						// ���Z����
			return '3';

		case 0:						// �ҋ@
			return '0';
		case 100:					// �x��
			return '1';
		default:					// ���̑�
			break;
		}
	}
	return '9';
}
/*[]----------------------------------------------------------------------[]
 *|	name	: select discount data
 *[]----------------------------------------------------------------------[]
 *| summary	: ��������I�ʂ���
 *| return	: �ݒ肵���f�[�^��
 *[]----------------------------------------------------------------------[]*/
void	select_discount_data(DISCOUNT_DATA* dst, const DISCOUNT_DATA* src)
{
	uchar	i,j;
// NOTE�F��1�����͓d���̊����G���ANTNET_DIC_MAX(25)�ő�2�����͓����ێ��G���A��WTIK_USEMAX(10)���I�[�o�[�t���[���Ȃ��悤�ɂ���
	for( i = j = 0; i < WTIK_USEMAX; i++ ){			// ���Z���~�ȊO�̊������R�s�[
		if(( src[i].DiscSyu != 0 ) &&				// ������ʂ���
		   (( src[i].DiscSyu < NTNET_CSVS_M ) ||	// ���Z���~�������łȂ�
		    ( NTNET_CFRE < src[i].DiscSyu ))){
			memcpy( &dst[j], &src[i], sizeof( DISCOUNT_DATA ) );	// �������
			if( dst[j].DiscSyu == NTNET_PRI_W ){
				memset( &dst[j].uDiscData, 0, 8 );	// �s�v�f�[�^�N���A
			}
			j++;
		}
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: set discount data
 *[]----------------------------------------------------------------------[]
 *| summary	: �������ݒ�
 *| return	: �ݒ肵���f�[�^��
 *[]----------------------------------------------------------------------[]*/
int	set_discount_data(t_DiscountInfo* dst, DISCOUNT_DATA* src)
{
	int i, cnt;
	
// ��������ParkiPro�̓d����̊����G���A��25�������APayData�ł�10�����������Ă��Ȃ��̂ŁA10�񂵂��܂킳�Ȃ�.
	for (i = 0, cnt = 0; i < WTIK_USEMAX; i++) {
		if (src[i].ParkingNo != 0) {
// MH810103 GG119202(S) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�
			// PayData.DiscountData�ɂ͖₢���킹�ԍ����Z�b�g���Ȃ��̂ŉ��L�����͍폜
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//			// �⍇���ԍ��͊������ɃZ�b�g�����Ȃ�
//			if( src[i].DiscSyu == NTNET_INQUIRY_NUM ){
//				continue;
//			}
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810103 GG119202(E) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�
			//��3���̒��ԏ�No�i�[
			intoas(dst[cnt].ParkingNo, (ushort)(src[i].ParkingNo%1000), 3);		// ���ԏꇂ
			intoas(dst[cnt].kind, src[i].DiscSyu, 3);						// �������
			
			switch (src[i].DiscSyu) {
			case 11:		// ������ض
			case 12:		// �̔���ض
				intoasl(dst[cnt].div, *(ulong*)&src[i].DiscNo, 6);			// ����No.
				intoasl(dst[cnt].sum,  src[i].Discount , 6);				// �g�p�z
				intoasl(dst[cnt].inf1, src[i].DiscInfo1, 6);				// �c�z
				break;
// MH810103 GG119202(S) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�
			// PayData.DiscountData�ɂ͓d�q�}�l�[�����Z�b�g���Ȃ��̂ŉ��L�����͍폜
//			case 31:		// SUICA
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�(�A���[������d�l�ǉ�/��������������O�Ή�)
////			case 33:		// EDY
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�(�A���[������d�l�ǉ�/��������������O�Ή�)
//			case 35:		// PASMO
//			case 61:		// ICOCA
//			case 63:		// IC����
//			case 70:		// ��Ԋ���
//				if( src[i].DiscSyu != 33 ){									// Edy�ł͂Ȃ��iSuica�n���ތ��ρj�̏ꍇ
//					if( Is_SUICA_STYLE_OLD ){								// ���œ���ݒ�
//						if( src[i].DiscSyu != 35 ){							// PASMO�ȊO�̏ꍇ��
//							intoas(dst[cnt].kind, 31, 3);					// ������ʂ�Suica�Œ�Ƃ���
//						}													// PASMO�̏ꍇ��PASMO�̎�ʂƂ��đ��M
//					}else{													// �V�d�l�łł́ASuica�n�J�[�h�͑S��Suica�Ƃ��đ��M
//						intoas(dst[cnt].kind, 31, 3);						// ������ʂ�Suica�Œ�Ƃ���
//					}
//				}
//				memset(dst[cnt].div, 0x20, 24);								// ��߰�(' ')���
//				memcpy(dst[cnt].div, (uchar*)&src[i].DiscNo, 20);			// ���ޔԍ�
//				break;
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�(�A���[������d�l�ǉ�/��������������O�Ή�)
//			case 33:		// EDY
//			case 37:		// iD
//			case 65:		// WAON
//			case 81:		// QUICPay
//			case 85:		// nanaco
//			case 87:		// PiTaPa
//				memset(dst[cnt].div, 0x20, 24);								// ��߰�(' ')���
//				memcpy(dst[cnt].div, (uchar*)&src[i].DiscNo, 20);			// ���ޔԍ�
//				break;
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�(�A���[������d�l�ǉ�/��������������O�Ή�)
// MH810103 GG119202(E) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�
			case 91:		// �U�֐��Z
				intoasl(dst[cnt].div, *(ulong*)&src[i].DiscNo, 6);			// ���E�Ԏ��ԍ�
				intoasl(dst[cnt].sum, src[i].Discount , 6);					// �����z
				memset(dst[cnt].inf2, 0x20, 6);								// ��߰�(' ')���
				BCDtoASCII((uchar*)&src[i].DiscInfo1, dst[cnt].inf1, 4);	// �U�֌����Z��������
				break;
			default:
				intoas(dst[cnt].div,   src[i].DiscNo,    3);				// �����敪
				intoas(dst[cnt].used,  src[i].DiscCount, 3);				// ����
				intoasl(dst[cnt].sum,  src[i].Discount , 6);				// �����z
				intoasl(dst[cnt].inf1, src[i].DiscInfo1, 6);				// �������P
				intoasl(dst[cnt].inf2, src[i].uDiscData.common.DiscInfo2, 6);// �������Q
				break;
			}
			cnt++;
		}
	}
	return cnt;
}

// MH810103 GG119202(S) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�
static int set_emoney_data(t_DiscountInfo* dst, Receipt_data* pay)
{
	int		i, cnt = 0;
	ushort	wk_kind1, wk_kind2;
	ushort	syu;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	int		j;


	if (pay->chuusi == 1) {
		// ���Z���~���͓d�q�}�l�[�֘A�̊�����ʂ��Z�b�g���Ȃ�
		return cnt;
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	if (isEC_USE()) {
		// �d�q�}�l�[�x��������H
		if (pay->Electron_data.Ec.pay_ryo == 0) {
			return 0;
		}
		for (i = 0; i < NTNET_DIC_MAX; i++) {
			// �󂫂�����
			syu = astoin(dst[i].kind, 3);
			if (syu == 0) {
				break;
			}
		}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		if (i < (NTNET_DIC_MAX-1)) {
		if (pay->Electron_data.Ec.e_pay_kind == EC_QR_USED) {
			j = 1;
		}
		else {
			j = 2;
		}
		if (i <= (NTNET_DIC_MAX-j)) {
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			switch (pay->Electron_data.Ec.e_pay_kind) {					// ���ώ�ʂ���U�蕪��
			case	EC_EDY_USED:
				wk_kind1 = NTNET_EDY_1;									// ������ʁFEdy���ޔԍ�
				wk_kind2 = NTNET_EDY_2;									// ������ʁFEdy�x���z�A�c�z
				break;
			case	EC_NANACO_USED:
				wk_kind1 = NTNET_NANACO_1;								// ������ʁFnanaco���ޔԍ�
				wk_kind2 = NTNET_NANACO_2;								// ������ʁFnanaco�x���z�A�c�z
				break;
			case	EC_WAON_USED:
				wk_kind1 = NTNET_WAON_1;								// ������ʁFWAON���ޔԍ�
				wk_kind2 = NTNET_WAON_2;								// ������ʁFWAON�x���z�A�c�z
				break;
			case	EC_SAPICA_USED:
				wk_kind1 = NTNET_SAPICA_1;								// ������ʁFSAPICA���ޔԍ�
				wk_kind2 = NTNET_SAPICA_2;								// ������ʁFSAPICA�x���z�A�c�z
				break;
			case	EC_KOUTSUU_USED:
				wk_kind1 = NTNET_SUICA_1;								// ������ʁFSuica���ޔԍ�
				wk_kind2 = NTNET_SUICA_2;								// ������ʁFSuica�x���z�A�c�z
				break;
			case	EC_ID_USED:
				wk_kind1 = NTNET_ID_1;									// ������ʁFiD���ޔԍ�
				wk_kind2 = NTNET_ID_2;									// ������ʁFiD�x���z�A�c�z
				break;
			case	EC_QUIC_PAY_USED:
				wk_kind1 = NTNET_QUICPAY_1;								// ������ʁFQUICPay���ޔԍ�
				wk_kind2 = NTNET_QUICPAY_2;								// ������ʁFQUICPay�x���z�A�c�z
				break;
// MH810105(S) MH364301 PiTaPa�Ή�
			case	EC_PITAPA_USED:
				wk_kind1 = NTNET_PITAPA_1;								// ������ʁFPiTaPa���ޔԍ�
				wk_kind2 = NTNET_PITAPA_2;								// ������ʁFPiTaPa�x���z
				break;
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			case	EC_QR_USED:
				wk_kind1 = NTNET_QR;									// ������� QR����
				wk_kind2 = 0;
				break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			default:
				wk_kind1 = 0;
				wk_kind2 = 0;
				break;
			}

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			if (pay->Electron_data.Ec.e_pay_kind == EC_QR_USED) {
				// QR���̃R�[�h����
				// ��3���̒��ԏ�No�i�[
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// ���ԏꇂ
				intoas(dst[i].kind, wk_kind1, 3);								// �������
				intoas(dst[i].div, 0, 3);										// �����敪
				intoas(dst[i].used, 1, 3);										// �g�p����
				intoasl(dst[i].sum, pay->Electron_data.Ec.pay_ryo, 6);			// �x�����z
				intoas(dst[i].inf1, (ushort)pay->Electron_data.Ec.Brand.Qr.PayKind, 6);	// �������1
				intoas(dst[i].inf2, 0, 6);
				if(pay->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
					// �݂Ȃ����ς�0x30(���σu�����h�s��)����
					memset(dst[i].inf1, '0', 6);								// �������1
				}
				else{
					intoas(dst[i].inf1, (ushort)pay->Electron_data.Ec.Brand.Qr.PayKind, 6);	// �������1
				}
				intoas(dst[i].inf2, 0, 6);										// �������2
			}
			else
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			// �J�[�h�ԍ��Ǝx���z�^�c�z���Z�b�g����i�₢���킹�ԍ��̓Z�b�g���Ȃ��j
			if (wk_kind1 != 0 && wk_kind2 != 0) {
				// ��3���̒��ԏ�No�i�[
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// ���ԏꇂ
				intoas(dst[i].kind, wk_kind1, 3);								// �������
				// �J�[�h�ԍ�
				memset(dst[i].div, 0x20, 24);									// ��߰�(' ')���
// MH810103 MHUT40XX(S) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
//				memcpy(dst[i].div, pay->Electron_data.Ec.Card_ID, 20);			// ���ޔԍ�
				if( pay->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
					pay->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
					// �݂Ȃ����ρ{���ό���OK��M�ɂ��݂Ȃ����ς̐��Z�f�[�^�͉��No.���J�[�h������0x30���߂����l�߂�ZZ����B
					// [*]�󎚂݂Ȃ����ςƓ��l�̉��No���Z�b�g����B
					// ���ꂼ��̃J�[�h������0x30����
					memset( dst[i].div, 0x30, (size_t)(EcBrandEmoney_Digit[pay->Electron_data.Ec.e_pay_kind - EC_USED]) );
					// ���l�߂�ZZ
					memset( dst[i].div, 'Z', 2 );
				}
				else {
					memcpy(dst[i].div, pay->Electron_data.Ec.Card_ID, 20);			// ���ޔԍ�
				}
// MH810103 MHUT40XX(E) �݂Ȃ��{����OK��M���ɐ��Z�f�[�^�̃J�[�h�ԍ�����M�����J�[�h�ԍ��̂܂ܑ��M����Ă��܂�
				i++;
				cnt++;

				// ��3���̒��ԏ�No�i�[
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// ���ԏꇂ
				intoas(dst[i].kind, wk_kind2, 3);								// �������
				intoas(dst[i].div, 0, 3);										// �����敪
				intoas(dst[i].used, 1, 3);										// �g�p����
				intoasl(dst[i].sum, pay->Electron_data.Ec.pay_ryo, 6);			// �x�����z
				intoasl(dst[i].inf1, pay->Electron_data.Ec.pay_after, 6);		// �c�z
				switch (pay->Electron_data.Ec.e_pay_kind) {						// ���ώ�ʂ���U�蕪��
				case	EC_EDY_USED:
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//					intoasl(dst[i].inf2, (ulong)pay->Electron_data.Ec.Brand.Edy.DealNo, 6);			// Edy����ʔ�
					memset(dst[i].inf2, '0', 6);
					memcpy(&dst[i].inf2[1], pay->Electron_data.Ec.Brand.Edy.CardDealNo, 5);			// �J�[�h����ʔ�
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
					break;
				case	EC_NANACO_USED:
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//					intoasl(dst[i].inf2, (ulong)pay->Electron_data.Ec.Brand.Nanaco.DealNo, 6);		// nanaco����ʔ�
					memcpy(dst[i].inf2, pay->Electron_data.Ec.Brand.Nanaco.DealNo, 6);				// �[������ʔ�
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
					break;
				case	EC_WAON_USED:
					intoasl(dst[i].inf2, pay->Electron_data.Ec.Brand.Waon.point, 6);				// WAON����t�^�|�C���g
					break;
				case	EC_SAPICA_USED:
					intoasl(dst[i].inf2, (ulong)pay->Electron_data.Ec.Brand.Sapica.Details_ID, 6);	// SAPICA�ꌏ����ID
					break;
// MH810105(S) MH364301 PiTaPa�Ή�
				case	EC_PITAPA_USED:
					memset(dst[i].inf1, '0', 6);
					memcpy(&dst[i].inf1[1], pay->Electron_data.Ec.Brand.Pitapa.Slip_No, 5);			// �`�[�ԍ�
					intoas(dst[i].inf2, 0, 6);
					break;
// MH810105(E) MH364301 PiTaPa�Ή�
				default:
					intoas(dst[i].inf2, 0, 6);
					break;
				}
				cnt++;
			}
		}
	}
	else if (isSX10_USE()) {
		// �d�q�}�l�[�x��������H
		if (pay->Electron_data.Suica.pay_ryo == 0) {
			return 0;
		}
		for (i = 0; i < NTNET_DIC_MAX; i++) {
			// �󂫂�����
			syu = astoin(dst[i].kind, 3);
			if (syu == 0) {
				break;
			}
		}
		if (i < (NTNET_DIC_MAX-1)) {
			switch (pay->Electron_data.Ec.e_pay_kind) {					// ���ώ�ʂ���U�蕪��
			case	OPE_DISP_MEDIA_TYPE_SUICA:
				wk_kind1 = NTNET_SUICA_1;								// ������ʁFSuica���ޔԍ�
				wk_kind2 = NTNET_SUICA_2;								// ������ʁFSuica�x���z�A�c�z
				break;
			case	OPE_DISP_MEDIA_TYPE_PASMO:
				wk_kind1 = NTNET_PASMO_1;								// ������ʁFPASMO���ޔԍ�
				wk_kind2 = NTNET_PASMO_2;								// ������ʁFPASMO�x���z�A�c�z
				break;
			case	OPE_DISP_MEDIA_TYPE_ICOCA:
				wk_kind1 = NTNET_ICOCA_1;								// ������ʁFICOCA���ޔԍ�
				wk_kind2 = NTNET_ICOCA_2;								// ������ʁFICOCA�x���z�A�c�z
				break;
			case	OPE_DISP_MEDIA_TYPE_ICCARD:
				wk_kind1 = NTNET_ICCARD_1;								// ������ʁFIC-Card���ޔԍ�
				wk_kind2 = NTNET_ICCARD_2;								// ������ʁFIC-Card�x���z�A�c�z
				break;
			default:
				wk_kind1 = 0;
				wk_kind2 = 0;
				break;
			}

			if (wk_kind1 != 0 && wk_kind2 != 0) {
				// ��3���̒��ԏ�No�i�[
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// ���ԏꇂ
				intoas(dst[i].kind, wk_kind1, 3);								// �������
				// �J�[�h�ԍ�
				memset(dst[i].div, 0x20, 24);									// ��߰�(' ')���
				memcpy(dst[i].div, pay->Electron_data.Suica.Card_ID, 16);		// ���ޔԍ�
				i++;
				cnt++;

				// ��3���̒��ԏ�No�i�[
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// ���ԏꇂ
				intoas(dst[i].kind, wk_kind2, 3);								// �������
				intoas(dst[i].div, 0, 3);										// �����敪
				intoas(dst[i].used, 1, 3);										// �g�p����
				intoasl(dst[i].sum, pay->Electron_data.Ec.pay_ryo, 6);			// �x�����z
				intoasl(dst[i].inf1, pay->Electron_data.Ec.pay_after, 6);		// �c�z
				intoas(dst[i].inf2, 0, 6);
				cnt++;
			}
		}
	}
	return cnt;
}
// MH810103 GG119202(E) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�

/*[]----------------------------------------------------------------------[]
 *|	name	: ID62
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ԏ��񑗐M
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_send_data62(void)
{
	tIFMMSG62	*msg;
	int			i;

	msg = (tIFMMSG62*)get_msgarea_and_init(62, sizeof(tIFMMSG62));
	
	make_BasicData(&msg->Basic);
	set_StatData(&msg->Stat);
	
	// �L���Ԏ��ԍ��Z�b�g
	for (i = 0; i < ValidRoomNum; i++) {
		intoas(msg->CarInfo[i].Area, (ushort)LockInfo[ValidRoomNo[i]].area, 2);
		intoasl(msg->CarInfo[i].No,  LockInfo[ValidRoomNo[i]].posi, 4);
	}
	
	// �f�[�^���Čv�Z
	msg->mlen = sizeof(tIFMMSG_BASIC) + sizeof(tM_STATUS);
	msg->mlen += (ushort)(sizeof(t_ShedInfo)*ValidRoomNum) + 6;

	ifm_send_msg();	// IFM�d�����M
	
	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID63
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ԏ���(�Ԏ����Ȃ�)���M
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_send_data63(void)
{
	tIFMMSG63	*msg;

	msg = (tIFMMSG63*)get_msgarea_and_init(63, sizeof(tIFMMSG63));

	make_BasicData(&msg->Basic);
	set_StatData(&msg->Stat);

	ifm_send_msg();	// IFM�d�����M

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID64
 *[]----------------------------------------------------------------------[]
 *| summary	: �S�Ԏ��󋵑��M
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_send_data64(void)
{
	tIFMMSG64	*msg;
	int			i;

	msg = (tIFMMSG64*)get_msgarea_and_init(64, sizeof(tIFMMSG64));

	make_BasicData(&msg->Basic);
	set_StatData(&msg->Stat);

	// �L���Ԏ��󋵃Z�b�g
	for (i = 0; i < ValidRoomNum; i++) {
		set_ShedStat(&msg->ShedStat[i], ValidRoomNo[i]);
	}
	
	// �f�[�^���Čv�Z
	msg->mlen = sizeof(tIFMMSG_BASIC) + sizeof(tM_STATUS);
	msg->mlen += (ushort)(sizeof(t_ShedStat)*ValidRoomNum) + 6;

	ifm_send_msg();	// IFM�d�����M

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID65
 *[]----------------------------------------------------------------------[]
 *| summary	: �P�Ԏ��󋵑��M
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_send_data65(ushort pos)
{
	tIFMMSG65	*msg;

	msg = (tIFMMSG65*)get_msgarea_and_init(65, sizeof(tIFMMSG65));

	make_BasicData(&msg->Basic);
	set_StatData(&msg->Stat);
	set_ShedStat(&msg->ShedStat, (int)(pos-1));

	ifm_send_msg();	// IFM�d�����M

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID66
 *[]----------------------------------------------------------------------[]
 *| summary	: ���Z�󋵑��M
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_send_data66(void)
{
	tIFMMSG66	*msg;
	ushort		flg;
	ulong		tmp;
	int			cnt;

	msg = (tIFMMSG66*)get_msgarea_and_init(66, sizeof(tIFMMSG66));
	make_BasicData(&msg->Basic);
	
	// �f�[�^�����犄����������
	msg->mlen -= sizeof(msg->DscntInfo);

	msg->Serial[0] = 'A';
	msg->Serial[1] = get_crm_state();

	flg = OPECTL.Ope_mod;
	if (OPECTL.Mnt_mod != 0 ||
		(flg != 2 && flg != 3)) {
		flg = 0;
	}

	if (flg) {
		if( ryo_buf.pkiti != 0xffff ){
			intoas(msg->PayInfo.CarInfo.Area, (ushort)LockInfo[ryo_buf.pkiti-1].area, 2);	// ���
			intoasl(msg->PayInfo.CarInfo.No,  LockInfo[ryo_buf.pkiti-1].posi, 4);			// �Ԏ��ԍ�
		}else{
			intoas(msg->PayInfo.CarInfo.Area, (ushort)(LOCKMULTI.LockNo/10000), 2);			// ���
			intoasl(msg->PayInfo.CarInfo.No,  LOCKMULTI.LockNo%10000, 4);					// �Ԏ��ԍ�
		}
		
		if (ryo_buf.ryo_flg >= 2) {
			intoas(msg->PayInfo.ParkingNo, (ushort)(CPrmSS[S_SYS][PayData.teiki.pkno_syu+1]%1000), 4);		// ��������ԏ�ԍ�
			intoas(msg->PayInfo.PassKind,  (ushort)PayData.teiki.syu, 2);			// ��������
			intoasl(msg->PayInfo.PassID,   PayData.teiki.id, 6);					// �����ID�i�_��ԍ��j
		}

		intoas(msg->PayInfo.FareKind, (ushort)(ryo_buf.syubet+1), 2);				// �������

		// ���Ɏ���
		intoas(&msg->PayInfo.tmEnter[0], (ushort)car_in_f.mon,  2);
		intoas(&msg->PayInfo.tmEnter[2], (ushort)car_in_f.day,  2);
		intoas(&msg->PayInfo.tmEnter[4], (ushort)car_in_f.hour, 2);
		intoas(&msg->PayInfo.tmEnter[6], (ushort)car_in_f.min,  2);
		
		if (ryo_buf.ryo_flg < 2)
			tmp = ryo_buf.tyu_ryo;									// �ʏ풓�ԗ���
		else
			tmp = ryo_buf.tei_ryo;									// ����g�p���ԗ���
		intoasl(msg->PayInfo.Fare, tmp, 6);							// ���ԗ���
		intoasl(msg->SumInfo.Cash,    ryo_buf.zankin, 6);			// �x�����c�z
		intoasl(msg->SumInfo.Entered, ryo_buf.nyukin, 6);			// �������z

		// �����ڍ�
		cnt = set_discount_data(msg->DscntInfo, PayData.DiscountData);
// MH810103 GG119202(S) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�
		cnt += set_emoney_data(msg->DscntInfo, &PayData);
// MH810103 GG119202(E) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�

		
		// �ݒ肳�ꂽ���������f�[�^���ɒǉ�
		msg->mlen += (ushort)(sizeof(t_DiscountInfo)*cnt);
	}

	ifm_send_msg();	// IFM�d�����M

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: make and send response of request
 *[]----------------------------------------------------------------------[]
 *| summary	: �����f�[�^�̍쐬�E���M
 *| param	: id   - �����pid
 *| 		: msg  - ��M�f�[�^
 *|           rslt - ���ʃR�[�h
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_send_rsp(int id, tIFMMSG *msg, uchar rslt)
{
	tIFMMSG			*rsp;
	tIFMMSG_BASIC	*basic;

	rsp = get_msgarea_and_init(id, _TOTAL_SIZE(msg));		// �̈�m��
	memcpy(&rsp->data, &msg->data, (size_t)msg->mlen);		// ��M�ް���߰
	basic = (tIFMMSG_BASIC*)rsp->data;
	basic->Rslt[0] = (uchar)((rslt/10) | '0');				// ���ʺ��޾��
	basic->Rslt[1] = (uchar)((rslt%10) | '0');
	ifm_send_msg();	// IFM�d�����M
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID30
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^�v����M����
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data30(tIFMMSG30 *msg)
{
	uchar			err = _IFM_ANS_SUCCESS;
	ushort 			req_id;
	ushort			pos;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG30));		// �d��������
	err = chk_BasicData(&msg->Basic, 1);			// ��{�ް�����
	if (err == _IFM_ANS_SUCCESS) {
		req_id = (ushort)astoinl(msg->req.r30.ReqID, 2);
		switch (req_id) {
		case 62:					// ���ԏ���
			err = ifm_send_data62();
			break;
		case 63:					// ���ԏ���i�Ԏ����Ȃ��j
			err = ifm_send_data63();
			break;
		case 64:					// �S�Ԏ����
			err = ifm_send_data64();
			break;
		case 65:					// �P�Ԏ����
			err = chk_CarInfo(&msg->req.r30.CarInfo, &pos);		// �Ԏ��������
			if (err == _IFM_ANS_SUCCESS)
				err = ifm_send_data65(pos);
			break;
		case 66:					// ���Z������
			err = ifm_send_data66();
			break;
		default:
			err = _IFM_ANS_INVALID_PARAM;
			break;
		}
	}

	if (err) {
		// NG�������M
		ifm_send_rsp(60, (tIFMMSG*)msg, err);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID40
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ݎ����ύX�v����M����
 *|           NTNET_RevData119(���v�ް���M����)�Ɠ���
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data40(tIFMMSG40 *msg)
{
	tREQ40			*tm;
	struct clk_rec	clk_data;
	uchar			clkstr[11];
	union {
		unsigned long	ul;
		unsigned char	uc[4];
	} u_LifeTime;
	date_time_rec2	date;
	uchar			rslt;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG40));		// �d��������
	rslt = chk_BasicData(&msg->Basic, 0);			// ��{�ް�����
	if (rslt == _IFM_ANS_SUCCESS) {
		// ascii -> binary
		tm = &msg->req.r40;
		date.Year = (ushort)astoinl(tm->Year, 4);
		date.Mon  = (uchar)astoinl(tm->Mon,  2);
		date.Day  = (uchar)astoinl(tm->Day,  2);
		date.Hour = (uchar)astoinl(tm->Hour, 2);
		date.Min  = (uchar)astoinl(tm->Min,  2);
		date.Sec  = (uchar)astoinl(tm->Sec,  2);
		if ((date.Mon >= 1 && 12 >= date.Mon) &&
			(date.Day >= 1 && 31 >= date.Day) &&
			(date.Hour < 24 && date.Min < 60 ) &&	// ���Ԃƕ������� ��Sec(�b)�͌��X�ݒ�͈͊O�Ȃ̂��������Ȃ�	
			(chkdate((short)date.Year, (short)date.Mon, (short)date.Day) == 0)) {
			if (OPECTL.Mnt_mod == 0 &&
				(OPECTL.Ope_mod == 0 ||				// �ҋ@���?
				 OPECTL.Ope_mod == 100 ||			// �x�Ə��?
				 OPECTL.Ope_mod == 110)) {			// �ޱ���װѕ\�����?
				clk_data.year = date.Year;
				clk_data.mont = date.Mon;
				clk_data.date = date.Day;
				clk_data.hour = date.Hour;
				clk_data.minu = date.Min;
				clk_data.seco = (uchar)date.Sec;
				clk_data.ndat = dnrmlzm( (short)clk_data.year, (short)clk_data.mont, (short)clk_data.date );
				clk_data.nmin = tnrmlz ( (short)0, (short)0, (short)clk_data.hour, (short)clk_data.minu );
				clk_data.week = (unsigned char)((clk_data.ndat + 6) % 7);

				timset( &clk_data );

				memset(clkstr,0,sizeof(clkstr));
				clkstr[0] = binbcd( (unsigned char)( clk_data.year / 100 ));	/* �N(��Q��) */
				clkstr[1] = binbcd( (unsigned char)( clk_data.year % 100 ));	/* �N(���Q��) */
				clkstr[2] = binbcd( clk_data.mont );							/* �� */
				clkstr[3] = binbcd( clk_data.date );							/* �� */
				clkstr[4] = binbcd( clk_data.hour );							/* ���� */
				clkstr[5] = binbcd( clk_data.minu );							/* �� */
				u_LifeTime.ul = LifeTim2msGet();
				clkstr[7] = u_LifeTime.uc[0];
				clkstr[8] = u_LifeTime.uc[1];
				clkstr[9] = u_LifeTime.uc[2];
				clkstr[10] = u_LifeTime.uc[3];
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				Edy_Rec.edy_status.BIT.TIME_SYNC_REQ = 1;						// ���������f�[�^���M�v��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				queset( OPETCBNO, CLOCK_CHG, 11, clkstr );						/* Send message to opration task */
				wopelg(OPLOG_SET_TIME, 0, 0);
			}
			else {
				rslt = _IFM_ANS_REFUSED;
			}
		}
		else {			
			rslt = _IFM_ANS_INVALID_PARAM;			// time data NG
		}
	}

	// �����f�[�^���M
	ifm_send_rsp(70, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID41
 *[]----------------------------------------------------------------------[]
 *| summary	: �t���b�v�E���b�N����v����M����
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data41(tIFMMSG41 *msg)
{
	tREQ41		*pt;
	ushort		pos, no;
	uchar		rslt;
	ulong		ulwork;
	
	msg->mlen = _MSG_LEN(sizeof(tIFMMSG41));		// �d��������
	rslt = chk_BasicData(&msg->Basic, 0);			// ��{�ް�����
	if (rslt == _IFM_ANS_SUCCESS) {
		pt   = &msg->req.r41;
		rslt = chk_CarInfo(&pt->CarInfo, &pos);		// �Ԏ��������

		if (rslt == _IFM_ANS_SUCCESS) {
			/* ���FNTNET�ƈȉ��̓_���قȂ�
					NTNET		���b�N�J������ɉ�������
					ParkiPRO	���b�N�J�w���シ���ɉ�������
				NTNET�Ɠ����悤�Ɋ�����ɉ�������悤�ɂ���ꍇ��
					1)FLPTCBNO�ɑ��郁�b�Z�[�W��FLAP_UP_SND_NTNET/FLAP_DOWN_SND_NTNET�ɕύX
					2)RecvBackUp.ReceiveFlg��2���Z�b�g
						��NTNET_Snd_Data05()�̐擪��RecvBackUp.ReceiveFlg==2�Ȃ��IFM�ɉ���
			*/
			no = pos - 1;
			ulwork = (ulong)(( LockInfo[no].area * 10000L ) + LockInfo[no].posi );
			switch (astoinl(pt->ProcMode, 2)) {
			case 1:							// ۯ����u�w��
				queset( FLPTCBNO, FLAP_UP_SND, sizeof(pos), &pos );
#ifdef	CAR_LOCK_MAX
				if( no < TOTAL_CAR_LOCK_MAX ){
					wopelg(OPLOG_SET_FLAP_UP, 0, ulwork);
				}
				else{
					wopelg(OPLOG_SET_LOCK_CLOSE, 0, ulwork);
				}
#else
				wopelg(OPLOG_SET_LOCK_CLOSE, 0, ulwork);
#endif
				break;

			case 2:						// ۯ����u�J�w��
				queset( FLPTCBNO, FLAP_DOWN_SND, sizeof(pos), &pos );
#ifdef	CAR_LOCK_MAX
				if( no < TOTAL_CAR_LOCK_MAX ){
					wopelg(OPLOG_SET_FLAP_DOWN, 0, ulwork);
				}
				else{
					wopelg(OPLOG_SET_LOCK_OPEN, 0, ulwork);
				}
#else
				wopelg(OPLOG_SET_LOCK_OPEN, 0, ulwork);
#endif
				Kakari_Numu[pos-1] = 0;								// �����o�ɗp�W�����ر�ر
				break;

			case 31:		// �Ïؔԍ������w��
				FLAPDT.flp_data[pos-1].passwd = 0;					// �Ïؔԍ�(Atype)����
				wopelg(OPLOG_ANSHOU_B_CLR, 0, ulwork);
				break;

			default:
				rslt = _IFM_ANS_INVALID_PARAM;
				break;
			}
		}
	}
	
	// �����f�[�^���M
	ifm_send_rsp(71, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID42
 *[]----------------------------------------------------------------------[]
 *| summary	: ��t�����s�v����M����
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data42(tIFMMSG42 *msg)
{
	ushort	pos, index;
	uchar	rslt, type;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG42));		// �d��������
	if ( prm_get( COM_PRM,S_TYP,62,1,1 ) == 0 ) {
		rslt = _IFM_ANS_REFUSED;					// ��t���@�\�Ȃ�
	}
	else {
		rslt = chk_BasicData(&msg->Basic, 0);			// ��{�ް�����
		if (rslt == _IFM_ANS_SUCCESS) {
			rslt = chk_CarInfo(&msg->req.r42.CarInfo, &pos);	// �Ԏ��������
			if (rslt == _IFM_ANS_SUCCESS) {
				index = pos - 1;
				if (OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 0 &&			// �ҋ@���?
					(FLAPDT.flp_data[index].mode >= FLAP_CTRL_MODE2 && FLAPDT.flp_data[index].mode <= FLAP_CTRL_MODE4)) {	// ���Ԓ�?
					// ��t�����s����
					type = (FLAPDT.flp_data[index].uketuke == 0)? 0x12 : 0x11;
					FLAPDT.flp_data[index].uketuke = 0;					// ���s�ς݃t���OOFF
					uke_isu((ulong)(LockInfo[index].area*10000L + LockInfo[index].posi), 
								(ushort)(pos), type);
					wopelg( OPLOG_PARKI_UKETUKEHAKKO, 0, 0 );			// ���엚��o�^
				}
				else {
					rslt = _IFM_ANS_REFUSED;
				}
			}
		}
	}
	
	// �����f�[�^���M
	ifm_send_rsp(72, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get receipt parameter
 *[]----------------------------------------------------------------------[]
 *| summary	: �̎��؍Ĕ��s�v���擾
 *| return	: OK/NG
 *[]----------------------------------------------------------------------[]*/
uchar	get_receipt_key(t_PayInfo *pay, struct _receiptkey *key)
{
// �������Ɉ�v�����̂ŁA���ɒl�̃`�F�b�N�͕s�v
// �i�����Ɏg�p���Ȃ����̂��`�F�b�N�j
	key->PayMethod    = (ushort)astoinl(pay->Method, 2);
	switch(key->PayMethod) {
	case	0:
	case	2:
	case	5:
	case	11:
		break;
	default:
		return _IFM_ANS_INVALID_PARAM;
	}
	key->PayClass     = (ushort)astoinl(pay->PayClass, 2);
	switch(key->PayClass) {
	case	0:
	case	1:
	case	4:
	case	5:
	case	10:
		break;
	default:
		return _IFM_ANS_INVALID_PARAM;
	}
	key->PayMode      = (ushort)astoinl(pay->Mode, 2);
	switch(key->PayMode) {
	case	0:
	case	4:
		break;
	default:
		return _IFM_ANS_INVALID_PARAM;
	}
	key->OutKind      = (ushort)astoinl(pay->OutKind, 2);
	if (key->OutKind != 0)
		return _IFM_ANS_INVALID_PARAM;

	key->PayCount     = (ulong)astoinl(pay->PayCount, 6);
	key->OutTime.Mon  = (uchar)astoinl(&pay->tmPay[0], 2);
	key->OutTime.Day  = (uchar)astoinl(&pay->tmPay[2], 2);
	key->OutTime.Hour = (uchar)astoinl(&pay->tmPay[4], 2);
	key->OutTime.Min  = (uchar)astoinl(&pay->tmPay[6], 2);
	key->WPlace       = (ulong)astoinl((uchar*)&pay->CarInfo.Area, 6);		// area & no.
	key->ParkingNo    = (ulong)astoinl(pay->ParkingNo, 4);
	key->PassKind     = (ushort)astoinl(pay->PassKind, 2);
	key->PassID       = (ulong)astoinl(pay->PassID, 6);

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: search receipt data
 *[]----------------------------------------------------------------------[]
 *| summary	: ���Z���O����
 *| return	: OK/NG
 *[]----------------------------------------------------------------------[]*/
uchar	search_receipt_data(struct _receiptkey *key, Receipt_data *wk)
{
	ushort		index;
// ��������
// ������X�V�F���Z�����A��������ԏ�ԍ��A�������ʁA�����ID����v
// ���̑��@�@�F���Z�����A���Z�ǂ��ԁi!=0�̂Ƃ��j�A�Ԏ��ԍ�����v
	short		adr;
	ushort		index_ttl;

	index_ttl = Ope_SaleLog_TotalCountGet( PAY_LOG_CMP );
	index = index_ttl;
	while (index != 0) {
		index--;
		Ope_SaleLog_1DataGet(index, PAY_LOG_CMP, index_ttl, wk);
		if (wk->TOutTime.Mon == key->OutTime.Mon
			&& wk->TOutTime.Day == key->OutTime.Day
			&& wk->TOutTime.Hour == key->OutTime.Hour
			&& wk->TOutTime.Min == key->OutTime.Min) {
			if (key->PayMethod == 5) {
				adr = wk->teiki.pkno_syu+1;
				if (prm_get(COM_PRM,S_SYS,adr,3,1) == key->ParkingNo &&
					wk->teiki.syu == key->PassKind &&
					wk->teiki.id == key->PassID)
					return _IFM_ANS_SUCCESS;
			}
			else {
				if (wk->WPlace == key->WPlace) {
					if (key->PayCount == 0 ||
						CountSel( &wk->Oiban ) == key->PayCount)
						return _IFM_ANS_SUCCESS;
				}
			}
		}
	}
	return _IFM_ANS_INVALID_PARAM;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID43
 *[]----------------------------------------------------------------------[]
 *| summary	: �̎��؍Ĕ��s�v����M����
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data43(tIFMMSG43 *msg)
{
	T_FrmReceipt	rec_data;			// �̎��؈󎚗v����ү�����ް�
	ushort			serial_no;
	struct _receiptkey	key;
	uchar			rslt;
	
	msg->mlen = _MSG_LEN(sizeof(tIFMMSG43));		// �d��������
	rslt = chk_BasicData(&msg->Basic, 0);			// ��{�ް�����
	if (rslt == _IFM_ANS_SUCCESS) {
		rslt = get_receipt_key(&msg->PayInfo, &key);			// ���������擾
		if (rslt == _IFM_ANS_SUCCESS) {
			serial_no = (ushort)astoinl(msg->Serial, 2);
			if (serial_no == 0 || serial_no != PayReceiptSerialNo) {
				PayReceiptSerialNo = serial_no;
				rslt = search_receipt_data(&key, &wkReceipt);		// �Y�����Z�f�[�^������
				if (rslt == _IFM_ANS_SUCCESS) {
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
//					if (OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 0) {	// �ҋ@��Ԃ̂�
					if (OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 0 &&	// �ҋ@��Ԃ̂�
						Ope_isPrinterReady() != 0) {					// ���V�[�g�󎚉�
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
					// �̎��؍Ĕ��s
						rec_data.prn_kind = R_PRI;					// �������ʁFڼ��
						rec_data.prn_data = &wkReceipt;				// �̎��؈��ް����߲�����
						rec_data.kakari_no = 99;					// �W��No.99�i�Œ�j
						rec_data.reprint = ON;						// �Ĕ��s�׸޾�āi�Ĕ��s�j
						memcpy( &rec_data.PriTime, &CLK_REC, sizeof(date_time_rec) );
																	// �Ĕ��s�����i���ݓ����j���
// MH810105(S) MH364301 �C���{�C�X�Ή�
//						queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
//						wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );			// ���엚��o�^
//						LedReq( CN_TRAYLED, LED_ON );								// ��ݎ�o�����޲��LED ON
//						LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// �d�q���ϒ��~�̎��ؔ��s���̎��o����LED������ϰ
						if (IS_INVOICE) {
							rec_data.prn_kind = J_PRI;				// �W���[�i���󎚂���
							memcpy(&Cancel_pri_work, &wkReceipt, sizeof(Cancel_pri_work));
							queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
							OPECTL.f_ReIsuType = 2;
						}
						else {
							queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
							wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );	// ���엚��o�^
							LedReq( CN_TRAYLED, LED_ON );			// ��ݎ�o�����޲��LED ON
							LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// �d�q���ϒ��~�̎��ؔ��s���̎��o����LED������ϰ
						}
// MH810105(E) MH364301 �C���{�C�X�Ή�
					}
					else {
						rslt = _IFM_ANS_REFUSED;
					}
					msg->PayInfo.Receipt = (uchar)('0' + wkReceipt.ReceiptIssue);
				}
			}
		}
	}
	
	// �����f�[�^���M
	ifm_send_rsp(73, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID44
 *[]----------------------------------------------------------------------[]
 *| summary	: ���z���Z�v����M����
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data44(tIFMMSG44 *msg)
{
	ushort		pos;
	uchar		rslt;
	struct VL_GGS		ggs;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG44));		// �d��������
	rslt = chk_BasicData(&msg->Basic, 0);			// ��{�ް�����
	if (rslt == _IFM_ANS_SUCCESS) {
		rslt = chk_CarInfo(&msg->PayInfo.CarInfo, &pos);	// �Ԏ��������
		if (rslt == _IFM_ANS_SUCCESS) {
			if (OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 2 &&		// �ʏ퐸�Z���?
				ryo_buf.pkiti == pos) {								// ���Z���̎Ԏ��ƈ�v?
				ggs.ryokin = (ulong)astoinl(msg->PayInfo.Fare, 6);
				ggs.zangaku = (ulong)astoinl(msg->SumInfo.Rest, 6);
				ggs.tonyugaku = (ulong)astoinl(msg->SumInfo.Entered, 6);
				ggs.waribikigaku = (ulong)astoinl(msg->SumInfo.Discount, 6);

				if (ggs.ryokin >= ggs.waribikigaku) {
					if (ope_imf_Pay(OPEIMF_RCVKIND_GENGAKU, (void*)&ggs)) {
						memcpy(&cache.msg44, msg, (size_t)_TOTAL_SIZE(msg));
						return;		// ������Ope����IFM_Snd_GengakuAns()���Ă�
					}
					else {
						rslt = _IFM_ANS_IMPOSSIBLE;
					}
				} else {
					// ���ԗ����������z������������d���G���[
					rslt = _IFM_ANS_INVALID_PARAM;
				}
			}
			else {
				rslt = _IFM_ANS_REFUSED;
			}
		}
	}
	
	// �����f�[�^���M
	ifm_send_rsp(74, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID45 & ID46
 *[]----------------------------------------------------------------------[]
 *| summary	: �U�֐��Z�`�F�b�N
 *| return	: result code
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_check_furikae(tIFMMSG46 *msg, tIFMMSG76 *rsp, struct VL_FRS *frs)
{
	ushort			pos1, pos2;
	uchar			rslt;
	flp_com			*src;
	struct _receiptkey	key;
	static
	Receipt_data	receipt;
	int				i;

	memset(frs, 0, sizeof(*frs));
	// �����d������
	memset(rsp, '0', sizeof(tIFMMSG76));
	rsp->mlen = _MSG_LEN(sizeof(tIFMMSG76));
	rsp->ID[0] = '7';
	rsp->ID[1] = '6';
	rsp->Basic = msg->Basic;
	rsp->PayMode[0] = 'A';
	rsp->PayMode[1] = get_crm_state();
	rsp->CarInfo1 = msg->req.r46.CarInfo1;
	rsp->CarInfo2 = msg->req.r46.CarInfo2;
	rsp->PassUse  = msg->req.r46.PassUse;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG46));		// �d��������
	rslt = chk_BasicData(&msg->Basic, 0);			// ��{�ް�����
	if (rslt != _IFM_ANS_SUCCESS)
		return rslt;
	rslt = chk_CarInfo(&msg->req.r46.CarInfo1, &pos1);		// �U�֌��Ԏ��������
	if (rslt != _IFM_ANS_SUCCESS)
		return rslt;
	rslt = chk_CarInfo(&msg->req.r46.CarInfo2, &pos2);		// �U�֐�Ԏ��������
	if (rslt != _IFM_ANS_SUCCESS)
		return rslt;
	src = &FLAPDT.flp_data[pos1-1];
	if (src->mode < FLAP_CTRL_MODE5 || src->mode > FLAP_CTRL_MODE6)	// �U�֌����b�N����?
		return _IFM_ANS_FROM_NG;
	if (OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod != 2)			// �ʏ퐸�Z���?
		return _IFM_ANS_REFUSED;
	if (ryo_buf.pkiti != pos2)								// �U�֐悪���Z�Ԏ��ƈ�v?
		return _IFM_ANS_TO_NG;
	if ( OPECTL.InquiryFlg )								// �ڼޯĂ�HOST�ڑ������d�q�}�̂̈����莸�s��ײ���̏ꍇ
		return _IFM_ANS_NOW_BUSY;							// Busy�Ƃ��ĕԂ��i�l�F�V�j
	if ( OPECTL.Pay_mod == 2 )								// �C�����Z��
		return _IFM_ANS_TO_NG;
	key.PayCount     = 0;
	key.OutTime.Mon  = src->s_mont;
	key.OutTime.Day  = src->s_date;
	key.OutTime.Hour = src->s_hour;
	key.OutTime.Min  = src->s_minu;
	key.WPlace       = (ulong)(( LockInfo[pos1-1].area * 10000L ) + LockInfo[pos1-1].posi );
	rslt = search_receipt_data(&key, &receipt);				// �Y�����Z�f�[�^������
	if (rslt != _IFM_ANS_SUCCESS)
		return _IFM_ANS_FROM_NG;
	if (msg->ID[1] == '5') {
	// ���O�`�F�b�N
		frs->antipassoff_req = (receipt.teiki.id)? 1 : 0;	// �U�֌��Œ���g�p
	}
	else {
	// ���s
		if (msg->req.r46.PassUse != '0' && msg->req.r46.PassUse != '1')
			return _IFM_ANS_INVALID_PARAM;
		frs->antipassoff_req = (uchar)(msg->req.r46.PassUse - '0');		// �v����ۑ�
	}
	frs->lockno = pos1;
	frs->price = receipt.WPrice /* + receipt.Wtax */;		// ���łȂ��Wtax==0�F�O�őΉ����Ȃ�
#if	0
--	i = is_paid_remote(&receipt);
--	if (i >= 0 && receipt.wari_data[i].tik_syu == MISHUU) {
--		frs->price -= receipt.wari_data[i].ryokin;			// �����������z
--	}
#endif
	// �ȉ��̍��ڂ͂O�ŏ���������Ă���̂ŁA�������ɐώZ��
	frs->in_price = receipt.WInPrice - receipt.WChgPrice;	// ����
	for (i = 0; i < WTIK_USEMAX; i++) {
		if ((receipt.DiscountData[i].DiscSyu == NTNET_FRE ) ||	// �񐔌�
		    (receipt.DiscountData[i].DiscSyu == NTNET_PRI_W )) {// �v���y�C�h�J�[�h
			frs->in_price += receipt.DiscountData[i].Discount;	// �����z���Z
		}
	}
	frs->in_price += receipt.ppc_chusi;						// �v���y�C�h�^�񐔌����~�f�[�^
	frs->in_price += receipt.credit.pay_ryo;				// �N���W�b�g
	frs->in_price += receipt.Electron_data.Suica.pay_ryo;	// �d�q�}�l�[
		//�@Suica(PASMO)��Edy��union��pay_ryo�̈ʒu�͕ω����Ȃ�
	frs->syubetu = receipt.syu;								// �������
	frs->seisan_oiban = receipt.Oiban;						// ���Z�ǂ���
	frs->seisan_time = receipt.TOutTime;					// ���Z����
	return rslt;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID45 & ID46
 *[]----------------------------------------------------------------------[]
 *| summary	: �U�֐��Z���Z�b�g
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	set_sw_info(tSWP_INFO *info, uchar rslt, struct VL_FRS *frs)
{
	ulong	ryokin;

	memset(info, 'N', sizeof(*info));
	if (rslt == _IFM_ANS_SUCCESS || rslt == _IFM_ANS_DONE) {
		intoasl(info->FailedPaid, frs->price, 6);				// �U�֌��x���z
		intoasl(info->Switchable, frs->furikaegaku, 6);			// �U�։\�z
		if (ryo_buf.ryo_flg < 2)
			ryokin = ryo_buf.tyu_ryo;
		else
			ryokin = ryo_buf.tei_ryo;

		intoasl(info->OrgFare, ryokin, 6);						// �U�֐撓�ԗ���
		ryokin = frs->zangaku;
	}
	if (rslt == _IFM_ANS_SUCCESS) {
		intoasl(info->OrgArrear, ryokin, 6);					// �U�֐�x���c�z
		if (ryokin < frs->furikaegaku)
			ryokin = 0;
		else
			ryokin -= frs->furikaegaku;
		intoasl(info->ActArrear,  ryokin, 6);					// �U�֌�x���c�z
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID45
 *[]----------------------------------------------------------------------[]
 *| summary	: �U�֐��Z���s�\�`�F�b�N��M����
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data45(tIFMMSG45 *msg)
{
	tIFMMSG75		rsp;
	uchar			rslt;
	struct VL_FRS	frs;

	// �U�փ`�F�b�N
	rslt = ifm_check_furikae((tIFMMSG46*)msg, (tIFMMSG76*)&rsp, &frs);
	if (rslt == _IFM_ANS_SUCCESS) {
	// ����ł̐U�։\�z�����߂�
		ope_ifm_GetFurikaeGaku(&frs);
		rsp.PassUse = (uchar)(frs.antipassoff_req | '0');
	}
	// �����f�[�^���M
	rsp.ID[1] = '5';
	set_sw_info(&rsp.SwInfo, rslt, &frs);
	ifm_send_rsp(75, (tIFMMSG*)&rsp, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID46
 *[]----------------------------------------------------------------------[]
 *| summary	: �U�֐��Z�v����M����
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data46(tIFMMSG46 *msg)
{
	tIFMMSG76		rsp;
	uchar			rslt;
	struct VL_FRS	frs;

	// �U�փ`�F�b�N
	rslt = ifm_check_furikae(msg, &rsp, &frs);
	if (rslt == _IFM_ANS_SUCCESS) {
		if (ope_imf_Pay(OPEIMF_RCVKIND_FURIKAE, (void*)&frs)) {
			memcpy(&cache.msg76, &rsp, (size_t)_TOTAL_SIZE((&rsp)));
			return;		// ������Ope����IFM_Snd_FurikaeAns()���Ă�
		}
		rslt = _IFM_ANS_FROM_NG;
	}
	// �����f�[�^���M
	set_sw_info(&rsp.SwInfo, rslt, &frs);
	ifm_send_rsp(76, (tIFMMSG*)&rsp, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: received IBK error message
 *[]----------------------------------------------------------------------[]
 *| summary	: IBK�G���[�ʒm��M
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_ibkerr(tIFMMSGB0 *msg)
{
	char	mdl_code, err_code, err_occur;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSGB0));		// �d��������
	mdl_code	= (char)astoin ( msg->ModuleCode, 2 );
	err_code	= (char)astoin ( msg->ErrorCode, 2 );
	err_occur	= (char)astoin ( msg->ErrorOccur, 2 );

	// ���W���[���R�[�h�P�Q�E�T�T���L��
	if (mdl_code == 12)
		mdl_code = 55;		// 12��55�ɕύX���ēo�^
	else if (mdl_code != 55 )
		return;

	// �擾�ް��̌`�������킹��B
	switch( err_occur ){
	case	1:
		if (err_code == 1)
			err_occur = 1;		// ����
		else
			err_occur = 2;		// �����E��������
		break;
	case	2:
		err_occur = 0;			// ����
		break;
	default:
		err_occur = 2;			// �����E��������
		break;
	}

	// �o�^���s��
	err_chk( ERRMDL_REMOTE, err_code, err_occur, 0, 0 );
}

/*[]----------------------------------------------------------------------[]
 *|	name	: IFM receive message
 *[]----------------------------------------------------------------------[]
 *| summary	: IFM�d����M
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	ifm_recv_msg(void)
{
	int		id = 0;

	if (ifm_rcv_buf.buff.ID[0] > '9') {
		if (ifm_rcv_buf.buff.ID[0] == 'B' &&
			ifm_rcv_buf.buff.ID[1] == '0')
			id = 0xB0;
	}
	else {
		id = (ifm_rcv_buf.buff.ID[0] - '0') * 10 +
			 (ifm_rcv_buf.buff.ID[1] - '0');
	}

	switch(id) {
	case	30:					//�f�[�^�v��
		ifm_recv_data30(&ifm_rcv_buf.msg30);
		break;
	case	40:					// ���ݎ����ύX�v��
		ifm_recv_data40(&ifm_rcv_buf.msg40);
		break;
	case	41:					// �t���b�v�E���b�N����v��
		ifm_recv_data41(&ifm_rcv_buf.msg41);
		break;
	case	42:					// ��t�����s�v��
		ifm_recv_data42(&ifm_rcv_buf.msg42);
		break;
	case	43:					// �̎��؍Ĕ��s�v��
		ifm_recv_data43(&ifm_rcv_buf.msg43);
		break;
	case	44:					// ���z���Z�v��
		ifm_recv_data44(&ifm_rcv_buf.msg44);
		break;
	case	45:					// �U�֐��Z���s�\�`�F�b�N
		ifm_recv_data45(&ifm_rcv_buf.msg45);
		break;
	case	46:					// �U�֐��Z�v��
		ifm_recv_data46(&ifm_rcv_buf.msg46);
		break;
	case	0xB0:				// IBK�G���[�ʒm
		ifm_recv_ibkerr(&ifm_rcv_buf.msgB0);
		break;
	default:
		break;
	}
	ifm_rcv_buf.buff.mlen = 0;		// receive buffer empty
}

//==============================================================================
//	�V���A����M�f�[�^��IFM�̃o�b�t�@�֕ϊ�
//	@argument	Length		��M�f�[�^��
//	@argument	Data		��M�f�[�^
//	@return		�Ȃ�
//	@note		�V���A����M�f�[�^��IFM�̃o�b�t�@�֕ϊ�
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void PIP_ChangeData( unsigned short Length, unsigned char *pData)
{
	t_DataDetail	*raublk;
	
	raublk = (t_DataDetail*)pData;
	if (raublk->ID1[0] == '0' && raublk->ID1[1] == '1' &&				// ID1�AID2�͌Œ�l�Ȃ̂ŁA�ԈႢ���������Ƃ��m�F
		raublk->ID2[0] == '0' && raublk->ID2[1] == '1') {
		ifm_rcv_buf.buff.ID[0] = raublk->ID3[0];						// ���
		ifm_rcv_buf.buff.ID[1] = raublk->ID3[1];
		ifm_rcv_buf.buff.mlen  = Length-6;								// ����
		memset(ifm_rcv_buf.buff.data, '0', IFM_DATA_MAX);
		memcpy(ifm_rcv_buf.buff.data, raublk->dat, (size_t)(Length-6));	// �f�[�^�̃A�h���X��n��
		ifm_recv_msg();													// IFM�d����M
	}
}
//==============================================================================
//	�f�[�^�e�[�u���̏�����
//	@argument	�Ȃ�
//	@return		�Ȃ�
//	@note		���Z�f�[�^�e�[�u���ƃG���[�f�[�^�e�[�u��������������
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void IFM_RcdBufClrAll()
{
	uchar	i;
	
	if (_is_not_pip()){		// PARKiPRO�H
		return;
	}

	PAYTBL.COUNT = 0;
	PAYTBL.RIND = 0;
	PAYTBL.CIND = 0;
	ERRTBL.COUNT = 0;
	ERRTBL.RIND = 0;
	ERRTBL.CIND = 0;
	for( i = 0; i < LOGTBL_MAX; i++ ){
		PAYTBL.DATA[i].LEN = 0;
		memset( PAYTBL.DATA[i].BUFF,0,sizeof(PAYTBL.DATA[0].BUFF));
		ERRTBL.ERROR[i].LEN = 0;
		memset( ERRTBL.ERROR[i].BUFF,0,sizeof(ERRTBL.ERROR[0].BUFF));
	}
}
//==============================================================================
//	���Z�f�[�^�e�[�u���ւ̃f�[�^�~��
//	@argument	�Ȃ�
//	@return		�Ȃ�
//	@note		���Z�f�[�^��ParkiPRO�p�̐��Z�e�[�u���ɒ~�ς���
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void PIP_PayTable_insert(tIFMMSG *msg)
{
	uchar	COUNT = PAYTBL.COUNT;
	uchar	RIND = PAYTBL.RIND;
	uchar	CIND = PAYTBL.CIND;
	ushort	LEN = msg->mlen - 4;

	if (_is_not_pip()){		// PARKiPRO�H
		return;
	}

	if( COUNT < LOGTBL_MAX ){			// �ۑ��d�����Q�O���ȉ�
		COUNT++;
	} else {							// ���ɂQ�O�����܂��Ă���Ƃ�
		RIND++;							// �ŌÓd���ʒu��ύX����B
		if( RIND >= LOGTBL_MAX )
			RIND = 0;
	}
	CIND++;								// ���񏑂����ݓd���ʒu��ύX����B
	if( CIND >= LOGTBL_MAX )
		CIND = 0;


	memcpy( &PAYTBL.DATA[PAYTBL.CIND].BUFF[0], msg->data, LEN );	// ���Z�e�[�u���֒~��
	PAYTBL.DATA[PAYTBL.CIND].LEN = LEN;

	PAYTBL.COUNT = COUNT;		// �X�V
	PAYTBL.RIND = RIND;			// �X�V
	PAYTBL.CIND = CIND;			// �X�V
}
//==============================================================================
//	���Z�f�[�^�ϊ�����
//	@argument	dat	���Z�f�[�^
//	@return		�Ȃ�
//	@note		���Z�f�[�^��ϊ����A�~�Ϗ������Ă�
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Snd_Payment(const Receipt_data *dat)
{
	tIFMMSG67		*msg;
	ushort			local_PayClass;	// �����敪
	uchar			i;

	if (_is_not_pip()){		// PARKiPRO�H
		return;
	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ��iParkiPro�f�[�^�C���j
	if( dat->PayClass == 2 || dat->PayClass == 3 ){				// ���~�H
		if( !dat->WInPrice &&									// �����Ȃ�
			!carduse() &&										// �J�[�h�x�����Ȃ�
			isEC_PAY_CANCEL() ){								// �d�q���ϒ[���g�p����
			// ���~���Z�f�[�^���M���A
			// chu_isu()���Ă΂ꂽ�������d�q���ϒ[���g�p�݂̂ł���ꍇ
			return;												// ���M���Ȃ�
		}
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��iParkiPro�f�[�^�C���j


	msg = (tIFMMSG67*)get_msgarea_and_init(67, sizeof(tIFMMSG67));
	make_BasicData(&msg->Basic);

	// �f�[�^�ǔ�
	intoas(msg->Serial, PayDataSerialNo, 2);
	if (PayDataSerialNo++ > 99){
		PayDataSerialNo = 1;
	}
	
	// ���Z���
	intoas(msg->PayInfo.Method,   dat->PayMethod, 2);				// ���Z���@
	if( (PayData.Electron_data.Suica.pay_ryo != 0) && (dat->PayClass == 0 || dat->PayClass == 1) ){
		local_PayClass = dat->PayClass + 4;
	} else {
		local_PayClass = dat->PayClass;
	}
	intoas(msg->PayInfo.PayClass, local_PayClass,  2);				// �����敪
	intoas(msg->PayInfo.Mode,     dat->PayMode,   2);				// ���Z���[�h
	intoas(msg->PayInfo.OutKind,  dat->OutKind,   2);				// ���Z�o��
	msg->PayInfo.Receipt += (uchar)dat->ReceiptIssue;				// �̎���

	intoasl(msg->PayInfo.PayCount, CountSel((ST_OIBAN *)&dat->Oiban ), 6);// ���Zor���Z���~�ǂ���

	intoas(msg->PayInfo.tmPay,     (ushort)dat->TOutTime.Mon,  2);	// �o�Ɏ����F��
	intoas(&msg->PayInfo.tmPay[2], (ushort)dat->TOutTime.Day,  2);	// �@�@�@�@�@��
	intoas(&msg->PayInfo.tmPay[4], (ushort)dat->TOutTime.Hour, 2);	// �@�@�@�@�@��
	intoas(&msg->PayInfo.tmPay[6], (ushort)dat->TOutTime.Min,  2);	// �@�@�@�@�@��

	intoasl(msg->PayInfo.Fare, dat->WPrice, 6);						// ���ԗ���

	intoasl(msg->PayInfo.CarInfo.Area, (dat->WPlace/10000), 2);		// �Ԏ����F���
	intoasl(msg->PayInfo.CarInfo.No,   (dat->WPlace%10000), 4);		// �@�@�@�@�@�ԍ�
	
	intoasl(msg->PayInfo.ParkingNo, (dat->teiki.ParkingNo%1000), 4);	// ��������ԏ�ԍ�
	intoas(msg->PayInfo.PassKind,   dat->teiki.syu, 2);					// ��������
	intoasl(msg->PayInfo.PassID,    dat->teiki.id, 6);					// �����ID
	if (dat->PassCheck) {
		/* �G���A������������ۂɃA�X�L�[�l'0'�ŏ��������Ă��邽�߁A+=�Œl�������� */
		msg->PayInfo.AntiPassChk += 1;				// ����߽����
	}
	msg->PayInfo.StockCnt    += (uchar)dat->CountSet;				// �ݎԶ���
	intoas(msg->PayInfo.FareKind, dat->syu, 2);						// �������

	intoas(msg->PayInfo.tmEnter,     (ushort)dat->TInTime.Mon,  2);	// ���Ɏ����F��
	intoas(&msg->PayInfo.tmEnter[2], (ushort)dat->TInTime.Day,  2);	// �@�@�@�@�@��
	intoas(&msg->PayInfo.tmEnter[4], (ushort)dat->TInTime.Hour, 2);	// �@�@�@�@�@��
	intoas(&msg->PayInfo.tmEnter[6], (ushort)dat->TInTime.Min,  2);	// �@�@�@�@�@��

	// ���z���
	intoasl(msg->SumInfo.Cash,       dat->WInPrice - dat->WChgPrice, 6);	// ��������
	intoasl(msg->SumInfo.Entered,    dat->WInPrice, 6);						// �������z
	intoasl(msg->SumInfo.Change,     dat->WChgPrice, 6);					// �ޑK���z
	intoasl(msg->SumInfo.Shortage,   dat->WFusoku, 6);						// �ޑK���o�s�����z
	intoasl(msg->SumInfo.CreditCard, dat->credit.pay_ryo, 6);				// �ڼޯĶ��ޗ��p���z

	// �������
	for( i = 0; i < NTNET_DIC_MAX; i++ ){
		memset( &DiscountData_Buff[i],0,sizeof(DiscountData_Buff[0]));
	}
	select_discount_data( DiscountData_Buff, dat->DiscountData);	// ��������I�ʂ���
	set_discount_data(msg->DscntInfo, DiscountData_Buff);			// �������ݒ�
// MH810103 GG119202(S) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�
	set_emoney_data(msg->DscntInfo, &PayData);
// MH810103 GG119202(E) PIP���Z�f�[�^�̓d�q�}�l�[�Ή�

	PIP_PayTable_insert((tIFMMSG*)msg);
}

//==============================================================================
//	���Z�f�[�^���M����
//	@argument	count	�v������Ă��鐸�Z�f�[�^�̌���
//	@return		�Ȃ�
//	@note		�v�����ꂽ���̐��Z�f�[�^��~�σf�[�^���擾���A
//	@note		ParkiPRO�ւ̑��M�o�b�t�@�֑}������
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Snd_AccumulatePayment(uchar count)
{
	tIFMMSG		*msg;
	ushort		index;
	
	if (_is_not_pip()){		// PARKiPRO�H
		return;
	}

	if( count > PAYTBL.COUNT )	count = PAYTBL.COUNT;			// �v�������e�[�u���ɂȂ��ꍇ�͕␳
	if( count == 0 )	return;									// ���M�ł���f�[�^�͂Ȃ��B
	index = (PAYTBL.RIND + (PAYTBL.COUNT - 1) - ( count - 1 ));	// �ŏ��̑��M�p�P�b�g�ʒu���w��(�v�Z�㕉�̒l�ɂȂ邱�Ƃ͂Ȃ�)
	if( index >= LOGTBL_MAX )	index -= LOGTBL_MAX;			// �˂���������㑤��(�Z�o����)
	if( index >= LOGTBL_MAX ){
		return;
	}
	
	do{
		msg = (tIFMMSG*)get_msgarea_and_init(67, sizeof(tIFMMSG67));
		memcpy( &msg->data[0] , &PAYTBL.DATA[index].BUFF[0] , PAYTBL.DATA[index].LEN);
		ifm_send_msg();	// IFM�d�����M
		index++;
		if( index >= LOGTBL_MAX ){
			index = 0;
		}
	}while( index !=  PAYTBL.CIND );
}
//==============================================================================
//	���Z�f�[�^�����쐬
//	@argument	�Ȃ�
//	@return		�Ȃ�
//	@note		���Z�@�N�����A�{��Log��萸�Z���O���擾���A
//	@note		ParkiPRO�p�̐��Z�f�[�^�e�[�u���֕ۑ�����(20��)
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Accumulate_Payment_Init()
{
	ushort			LogCount;		// ���O�̓o�^����
	ushort			DatagetCount;	// ���O�擾�J�E���g
	uchar			flg_tgt;		// ����/�s���o�ɂ����f����t���O
	if (_is_not_pip()){		// PARKiPRO�H
		return;
	}

	LogCount = Ope_Log_TotalCountGet(eLOG_PAYMENT);	// ���O�o�^�����擾
	DatagetCount = LOGTBL_MAX;						// ���O�擾�J�E���g
	
	if( LogCount <= DatagetCount ){
		DatagetCount = 0;						// ���O�o�^������20�������̏ꍇ�A���O�o�^�����������Ԃ�
	} else {
		DatagetCount = LogCount - DatagetCount;	// �擾���O�̐擪(�ŐV����20�Ԗڂ̃��O) �o�^���� - 20�� 
	}

	while( LogCount > DatagetCount ){											// �ŐV���O LogCount == count�܂Ŏ擾����
		if( 1 == Ope_Log_1DataGet( eLOG_PAYMENT, DatagetCount, &Receipt_buf ) )	// �擪(�Ō�)����count�Ԗڂ̃��O���擾
		{
			flg_tgt = 0;
			if(( Receipt_buf.PayClass == 2 ) ||					// �����敪�����Z���~?
				 ( Receipt_buf.PayClass == 3 )){				// �����敪���Đ��Z���~?
					flg_tgt = 1;								// �w�肳�ꂽ�ʐ��Z���O��o��OK
			}else if(( Receipt_buf.OutKind !=  3 ) &&			// ���Z�o�Ɂ��s���o��?
					 ( Receipt_buf.OutKind !=  1 ) &&			// ���Z�o�Ɂ������o��?
					 ( Receipt_buf.OutKind != 11 ) &&			// ���Z�o�Ɂ������o��(���u)?
					 ( Receipt_buf.OutKind != 97 )){			// �t���b�v�㏸�A���b�N�^�C�}���o��
					flg_tgt = 1;								// �w�肳�ꂽ�ʐ��Z���O��o��OK
			}
			if( flg_tgt == 1 ){	// ����/�s���o�ɂł͂Ȃ��ꍇ
				IFM_Snd_Payment(&Receipt_buf);
			}
		}
		DatagetCount++;
	}
}

//==============================================================================
//	�G���[�f�[�^�e�[�u���ւ̃f�[�^�~��
//	@argument	msg	�G���[�f�[�^(�ϊ��ς�)
//	@return		�Ȃ�
//	@note		�G���[�f�[�^��ParkiPRO�p�̃G���[�e�[�u���ɒ~�ς���
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void PIP_ErrTable_insert(tIFMMSG *msg)
{
	uchar	COUNT = ERRTBL.COUNT;
	uchar	RIND = ERRTBL.RIND;
	uchar	CIND = ERRTBL.CIND;
	ushort	LEN = msg->mlen;

	if (_is_not_pip()){		// PARKiPRO�H
		return;
	}
	if( COUNT < LOGTBL_MAX ){		// �ۑ��d�����Q�O���ȉ�
		COUNT++;
	} else {						// ���ɂQ�O�����܂��Ă���Ƃ�
		RIND++;						// �ŌÓd���ʒu��ύX����B
		if( RIND >= LOGTBL_MAX )
			RIND = 0;
	}
	CIND++;							// ���񏑂����ݓd���ʒu��ύX����B
	if( CIND >= LOGTBL_MAX )
		CIND = 0;

	memcpy( &ERRTBL.ERROR[ERRTBL.CIND].BUFF[0], msg->data, LEN );
	ERRTBL.ERROR[ERRTBL.CIND].LEN = LEN;

	ERRTBL.COUNT = COUNT;		// �X�V
	ERRTBL.RIND = RIND;			// �X�V
	ERRTBL.CIND = CIND;			// �X�V
}
//==============================================================================
//	�G���[�f�[�^�ϊ�����(������)
//	@argument	�Ȃ�
//	@return		�Ȃ�
//	@note		�G���[�f�[�^��ϊ����A�~�Ϗ������Ă�
//	@note		�G���[�̏��́uErr_work�v���擾
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Snd_Error(void)
{
	tIFMMSG68	*msg;
	
	if (_is_not_pip()){
		return;
	}

	msg = (tIFMMSG68*)get_msgarea_and_init(68, sizeof(tIFMMSG68));
	make_BasicData(&msg->Basic);
	intoas(msg->ModuleCode, (ushort)Err_work.Errsyu, sizeof(msg->ModuleCode));
	intoas(msg->ErrCode, (ushort)Err_work.Errcod, sizeof(msg->ErrCode));
	switch( Err_work.Errdtc ){
	case 1:				// ����
		msg->Stat[1] = '1';						// �װ����
		break;
	case 2:				// �����E����
		msg->Stat[1] = '3';						// �װ�����E����
		break;
	default:			// ����
		msg->Stat[1] = '2';						// �װ����
		break;
	}
	set_MMDDhhmm(msg->Time);
	PIP_ErrTable_insert((tIFMMSG*)msg);
}
//==============================================================================
//	�G���[�f�[�^�ϊ�����(������)
//	@argument	�Ȃ�
//	@return		�Ȃ�
//	@note		�G���[�f�[�^��ϊ����A�~�Ϗ������Ă�
//	@note		�������̓R�[�h�����܂��Ă���B
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Snd_ErrorClear(void)
{
	tIFMMSG68	*msg;

	if (_is_not_pip()){
		return;
	}
	msg = (tIFMMSG68*)get_msgarea_and_init(68, sizeof(tIFMMSG68));
	make_BasicData(&msg->Basic);
	memcpy(msg->ModuleCode, "0255", sizeof(msg->ModuleCode));
	memcpy(msg->ErrCode, "0255", sizeof(msg->ErrCode));
	msg->Stat[1] = '3';						// �װ�����E����
	set_MMDDhhmm(msg->Time);
	PIP_ErrTable_insert((tIFMMSG*)msg);
}

//==============================================================================
//	�G���[�f�[�^���M����
//	@argument	count	�v������Ă���G���[�f�[�^�̌���
//	@return		�Ȃ�
//	@note		�v�����ꂽ���̃G���[�f�[�^��~�σf�[�^���擾���A
//	@note		ParkiPRO�ւ̑��M�o�b�t�@�֑}������
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Snd_AccumulateError(uchar count)
{
	tIFMMSG		*msg;
	ushort		index;
	
	if (_is_not_pip()){		// PARKiPRO�H
		return;
	}
	if( count > ERRTBL.COUNT )	count = ERRTBL.COUNT;			// �v�������e�[�u���ɂȂ��ꍇ�͕␳
	if( count == 0 )	return;									// ���M�ł���f�[�^�͂Ȃ��B
	index = (ERRTBL.RIND + (ERRTBL.COUNT - 1) - ( count - 1 ));	// �ŏ��̑��M�p�P�b�g�ʒu���w��(�v�Z�㕉�̒l�ɂȂ邱�Ƃ͂Ȃ�)
	if( index >= LOGTBL_MAX )	index -= LOGTBL_MAX;			// �˂���������㑤��(�Z�o����)
	if( index >= LOGTBL_MAX ){
		return;
	}
	
	do{
		msg = (tIFMMSG*)get_msgarea_and_init(68, sizeof(tIFMMSG68));
		memcpy( &msg->data[0] , &ERRTBL.ERROR[index].BUFF[0] , ERRTBL.ERROR[index].LEN);
		ifm_send_msg();	// IFM�d�����M
		index++;
		if( index >= LOGTBL_MAX ){
			index = 0;
		}
	}while( index !=  ERRTBL.CIND );
}

//==============================================================================
//	�G���[�f�[�^�����쐬
//	@argument	�Ȃ�
//	@return		�Ȃ�
//	@note		���Z�@�N�����A�{��Log���G���[���O���擾���A
//	@note		ParkiPRO�p�̃G���[�f�[�^�e�[�u���֕ۑ�����(20��)
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Accumulate_Error_Init()
{
	tIFMMSG68	*msg;
	Err_log		pSetBuf;
	ushort		LogCount;		// ���O�o�^�����擾
	ushort		DatagetCount;	// ���O�擾�J�E���g
	
	if (_is_not_pip()){		// PARKiPRO�H
		return;
	}
	LogCount = Ope_Log_TotalCountGet(eLOG_ERROR);	// ���O�o�^�����擾
	DatagetCount = LOGTBL_MAX;						// ���O�擾�J�E���g

	if( LogCount <= DatagetCount ){
		DatagetCount = 0;						// ���O�o�^������20�������̏ꍇ�A���O�o�^�����������Ԃ�
	} else {
		DatagetCount = LogCount - DatagetCount;	// �擾���O�̐擪(�ŐV����20�Ԗڂ̃��O) �o�^���� - 20�� 
	}

	while( LogCount > DatagetCount ){										// �ŐV���O LogCount == DatagetCount�ɂȂ�܂Ŏ擾����
		if( 1 == Ope_Log_1DataGet( eLOG_ERROR, DatagetCount, &pSetBuf ) )	// �擪(�Ō�)����count�Ԗڂ̃��O���擾
		{
			msg = (tIFMMSG68*)get_msgarea_and_init(68, sizeof(tIFMMSG68));
			make_BasicData(&msg->Basic);
			intoas(msg->ModuleCode, (ushort)pSetBuf.Errsyu, sizeof(msg->ModuleCode));
			intoas(msg->ErrCode, (ushort)pSetBuf.Errcod, sizeof(msg->ErrCode));
			switch( pSetBuf.Errdtc ){
			case 1:				// ����
				msg->Stat[1] = '1';						// �װ����
				break;
			case 2:				// �����E����
				msg->Stat[1] = '3';						// �װ�����E����
				break;
			default:			// ����
				msg->Stat[1] = '2';						// �װ����
				break;
			}
			intoas(msg->Time,     (ushort)pSetBuf.Date_Time.Mon,  2);	// �o�Ɏ����F��
			intoas(&msg->Time[2], (ushort)pSetBuf.Date_Time.Day,  2);	// �@�@�@�@�@��
			intoas(&msg->Time[4], (ushort)pSetBuf.Date_Time.Hour, 2);	// �@�@�@�@�@��
			intoas(&msg->Time[6], (ushort)pSetBuf.Date_Time.Min,  2);	// �@�@�@�@�@��
			PIP_ErrTable_insert((tIFMMSG*)msg);
		}
		DatagetCount++;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID74
 *[]----------------------------------------------------------------------[]
 *| summary	: ���z���Z����
 *| param	: sts(��t���̉����F1/-1/-7�A�������̉����F2/-2/-3)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	IFM_Snd_GengakuAns(int sts)
{
	uchar	rslt;

	switch(sts) {
	case	1:
		rslt = _IFM_ANS_SUCCESS;
		break;
	case	-1:
		rslt = _IFM_ANS_IMPOSSIBLE;
		break;
	case	2:
		rslt = _IFM_ANS_DONE;
		break;
	case	-2:
	case	-3:
		rslt = _IFM_ANS_CANCEL;
		break;
	case	-7:
		rslt = _IFM_ANS_NOW_BUSY;
		break;
	default:
		rslt = _IFM_ANS_OTHER_ERR;
		break;
	}
	ifm_send_rsp(74, (tIFMMSG*)&cache.msg44, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID76
 *[]----------------------------------------------------------------------[]
 *| summary	: �U�֐��Z����
 *| param	: sts(��t���̉����F1/-1/-7�A�������̉����F2/-2/-3)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	IFM_Snd_FurikaeAns(int sts, struct VL_FRS *frs)
{
	uchar	rslt;

	switch(sts) {
	case	1:
		rslt = _IFM_ANS_SUCCESS;
		break;
	case	-1:
		rslt = _IFM_ANS_FROM_NG;
		break;
	case	2:
		rslt = _IFM_ANS_DONE;
		break;
	case	-2:
		rslt = _IFM_ANS_ABNORMAL;
		break;
	case	-3:
		rslt = _IFM_ANS_CANCEL;
		break;
	case	-7:
		rslt = _IFM_ANS_NOW_BUSY;
		break;
	default:
		rslt = _IFM_ANS_OTHER_ERR;
		break;
	}
	if (rslt >= _IFM_ANS_DONE)					// ���s���ʂɂ���A3�Œ�Ƃ���
		cache.msg76.PayMode[1] = '3';
	set_sw_info(&cache.msg76.SwInfo, rslt, frs);
	ifm_send_rsp(76, (tIFMMSG*)&cache.msg76, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: Initialize IFM communication
 *[]----------------------------------------------------------------------[]
 *| summary	: IFM�ʐM������
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	IFM_Init(int clr)
{
//	ushort	i;

	pipcomm = (ushort)prm_get( COM_PRM,S_PAY,25,1,3 );

	ifm_rcv_buf.buff.mlen = 0;

	PayDataSerialNo = 0;
	PayReceiptSerialNo = 0;

	cache.msg44.mlen = 0;
	cache.msg76.mlen = 0;

}
