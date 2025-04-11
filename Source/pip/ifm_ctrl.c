/*[]----------------------------------------------------------------------[]*/
/*| PARKiPRO対応                                                           |*/
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
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"lkmain.h"
#include	"pip_def.h"


//**************************************************************************
//	COMMON FLAG
//**************************************************************************
ushort	pipcomm;			// == 02-0025③:途中で変更されると動作できないのでフラグ化

//**************************************************************************
//	DATA PART
//**************************************************************************

// 送信データ詳細
typedef struct {
	char	ID1[2];			// データブロックNo.
	char	ID2[2];			// 最終ブロック判定
	char	ID3[2];			// データ種別
	uchar	dat[IFM_DATA_MAX];
} t_DataDetail;
t_DataDetail	blk_data;	// 送信用データ(1ブロック分)
Receipt_data	Receipt_buf;
DISCOUNT_DATA	DiscountData_Buff[NTNET_DIC_MAX];				// 割引関連変換用領域
#define LOGTBL_MAX	20		// 精算LOG、エラーLOGの最大保存件数

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
	ushort			PayMethod;			// 精算方法
	ushort			PayClass;			// 処理区分
	ushort			PayMode;			// 精算ﾓｰﾄﾞ
	ushort			OutKind;			// 精算出庫
	ulong			PayCount;			// 精算追い番
	date_time_rec	OutTime;			// 月日時分
	ulong			WPlace;				// 駐車位置ﾃﾞｰﾀ
	ulong			ParkingNo;			// 定期券駐車場№
	ushort			PassKind;			// 定期券種別
	ulong			PassID;				// 定期券ID
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
// MH322914(S) K.Onodera 2016/09/12 AI-V対応：振替精算
//static	uchar	get_crm_state(void);
// MH322914(E) K.Onodera 2016/09/12 AI-V対応：振替精算
static	void	get_valid_roomnum(void);
static	void	select_discount_data(DISCOUNT_DATA* dst, const DISCOUNT_DATA* src);	// 割引情報を選別する
static	int		set_discount_data(t_DiscountInfo* dst, DISCOUNT_DATA* src);
// MH810103 GG119202(S) PIP精算データの電子マネー対応
static	int		set_emoney_data(t_DiscountInfo* dst, Receipt_data* pay);
// MH810103 GG119202(E) PIP精算データの電子マネー対応

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
// MH322914(S) K.Onodera 2016/09/12 AI-V対応：振替精算
extern	ushort	PiP_FurikaeSts;		// 振替精算ステータスフラグ
// MH322914(E) K.Onodera 2016/09/12 AI-V対応：振替精算

/*[]----------------------------------------------------------------------[]
 *|	name	: divide message into RAU blocks
 *[]----------------------------------------------------------------------[]
 *| summary	: データ分割情報作成
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
	
	// 最終ブロックのバイト補正サイズを設定
	ifm_snd_ctr.lb_revised_size = ifm_snd_ctr.last_blk_size + 6;
	wrk = (ifm_snd_ctr.lb_revised_size) % 8;
	if (wrk) {
		ifm_snd_ctr.lb_revised_size += (8 - wrk);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: send one block
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信データをブロックにわけ、シリアル送信キューへ書き込む
 *[]----------------------------------------------------------------------[]*/
static	void	send_oneblk()
{
	ushort			size;
	uchar			*wrk;
	uchar 			i;

	div_data();	// データ分割情報作成
	
	// 送信データ作成
	for( i = 1 ; i <= ifm_snd_ctr.blk_num ; i++ ){
		memset(&blk_data, '0', sizeof(blk_data));
		blk_data.ID1[0] += (char)(i / 10);
		blk_data.ID1[1] += (char)(i % 10);
		memcpy(blk_data.ID3, ifm_snd_buf.buff.ID, 2);

		wrk = ifm_snd_buf.buff.data;
		wrk += IFM_DATA_MAX*(i-1);	// 次ブロックのアドレスに移動

		if (i < ifm_snd_ctr.blk_num) {	// 最終ブロック以外の場合
			memcpy(blk_data.dat, wrk, IFM_DATA_MAX);
			size = IFM_DATA_MAX + 6;
		} else {											// 最終ブロックの場合
			memcpy(blk_data.ID2, "01", 2);	// 最終ブロック
			memcpy(blk_data.dat, wrk, (size_t)ifm_snd_ctr.last_blk_size);
			size = ifm_snd_ctr.lb_revised_size;
		}
		PIP_SciSendQue_insert((unsigned char *)&blk_data, size);	// シリアル送信キューへのデータ追加
	}
}

//**************************************************************************
//	IFM MESSAGING PART
//**************************************************************************
/*[]----------------------------------------------------------------------[]
 *|	name	: send IFM message
 *[]----------------------------------------------------------------------[]
 *| summary	: IFM電文送信
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
 *| summary	: 送信電文エリア確保
 *| param	: id - データ種別
 *| 		: size - 電文領域サイズ（電文長、データ種別領域込み）
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
 *| summary	: 基本データのチェック
 *| param	: *dat - 基本データ
 *| 		: wild - 1なら各項目で"0"を許可
 *| return	: IFMエラーコード
 *[]----------------------------------------------------------------------[]*/
uchar chk_BasicData(tIFMMSG_BASIC *dat, int wild)
{
	int wrk;

	// 駐車場№
	if(!wild){									// チェックする
		wrk = (int)astoinl(dat->ParkingNo, sizeof(dat->ParkingNo));
		if( prm_get(COM_PRM,S_SYS,1,6,1) >= GTF_PKNO_LOWER ){	// GTフォーマット
			if (wrk != ((int)prm_get(COM_PRM,S_SYS,1,3,1))) {
				return _IFM_ANS_INVALID_PARAM;
			}
		}else{													// APSフォーマット
			if( (wrk == 0)||(wrk != CPrmSS[S_SYS][1]) ){
				return _IFM_ANS_INVALID_PARAM;
			}
		}
	}

	// 機械№
	if(!wild){									// チェックする
		wrk = (int)astoinl(dat->MachineNo, sizeof(dat->MachineNo));
		if (wrk != CPrmSS[S_PAY][2]) {			// 受信＝設定
			return _IFM_ANS_INVALID_PARAM;
		}
	}

	// 機種ｺｰﾄﾞ
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
 *| summary	: 車室情報チェック
 *| param	: *dat - 車室情報
 *| param	: *pos - LockInfo設定No.を返す
 *| return	: IFMエラーコード
 *[]----------------------------------------------------------------------[]*/
uchar	chk_CarInfo(t_ShedInfo* dat, ushort* pos)
{
	uchar	area;
	ushort	no;
	ushort	index;

	area = (uchar)astoinl(dat->Area, sizeof(dat->Area));
	no   = (ushort)astoinl(dat->No, sizeof(dat->No));
	if ((area != 0) ||						// ※FT-4800では0固定
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
 *| summary	: 基本データの作成
 *| param	: *dat - 基本データ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	make_BasicData(tIFMMSG_BASIC *dat)
{
	intoasl(dat->ParkingNo, (ulong)(CPrmSS[S_SYS][1]%1000), sizeof(dat->ParkingNo));		// 駐車場№
	intoasl(dat->MachineNo, (ulong)CPrmSS[S_PAY][2], sizeof(dat->MachineNo));		// 機械№
	memcpy(dat->ModelCode, MODEL_CODE, sizeof(dat->ModelCode));				// 機種コード
}

/*[]----------------------------------------------------------------------[]
 *|	name	: make date & time
 *[]----------------------------------------------------------------------[]
 *| summary	: 日時データ作成
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
 *| summary	: 精算機情報作成
 *| param	: *dat - data area
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	set_StatData(tM_STATUS* dat)
{
	intoas(dat->CurTime, (ushort)CLK_REC.year, 4);
	set_MMDDhhmm(&dat->CurTime[4]);
	
	dat->Stat  += (uchar)(opncls()-1);				// 営休業状況
	dat->Mode  += (OPECTL.Mnt_mod)?1:0; 			// 精算機動作モード状況
	dat->Err   += Err_onf;							// エラー発生状況
	dat->Alarm += Alm_onf;							// アラーム発生状況

	dat->Full1 = (getFullFactor(0)&0x10)?'1':'0';	// 満車１状況
	dat->Full2 = (getFullFactor(1)&0x10)?'1':'0'; 	// 満車２状況
	dat->Full3 = (getFullFactor(2)&0x10)?'1':'0'; 	// 満車３状況
	
	get_valid_roomnum();
	intoas(dat->ShedNum, (ushort)ValidRoomNum, 3);	// 有効車室数
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get and set valid room number
 *[]----------------------------------------------------------------------[]
 *| summary	: 有効車室設定
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
 *| summary	: 車室状況情報作成
 *| param	: *dat  - data area
 *|           index - flap data index
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	set_ShedStat(t_ShedStat* dat, int index)
{
	intoas(dat->Area, (ushort)LockInfo[index].area, 2);			// 区画
	intoasl(dat->No, LockInfo[index].posi, 4);					// 車室番号

	dat->Mode  += (uchar)FLAPDT.flp_data[index].mode;			// 処理モード
	dat->Stat1 += FLAPDT.flp_data[index].nstat.bits.b00;		// ｽﾃｰﾀｽ1:車両有無
	dat->Stat2 += FLAPDT.flp_data[index].nstat.bits.b01;		// ｽﾃｰﾀｽ2:ﾌﾗｯﾌﾟ状態
	dat->Stat3 += FLAPDT.flp_data[index].nstat.bits.b02;		// ｽﾃｰﾀｽ3:動作中？
	
	if (FLAPDT.flp_data[index].mode != FLAP_CTRL_MODE1) {
		// 入庫時刻
		intoas(&dat->ParkTime[0], (ushort)FLAPDT.flp_data[index].mont, 2);
		intoas(&dat->ParkTime[2], (ushort)FLAPDT.flp_data[index].date, 2);
		intoas(&dat->ParkTime[4], (ushort)FLAPDT.flp_data[index].hour, 2);
		intoas(&dat->ParkTime[6], (ushort)FLAPDT.flp_data[index].minu, 2);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get crm current state
 *[]----------------------------------------------------------------------[]
 *| summary	: 精算機処理状態
 *| return	: state code
 *[]----------------------------------------------------------------------[]*/
uchar	get_crm_state(void)
{
	if (OPECTL.Mnt_mod == 0) {
		switch (OPECTL.Ope_mod) {
		case 2:						// 駐車料金精算中
			switch (ope_imf_GetStatus()) {
			case 0:
// MH322914(S) K.Onodera 2016/09/12 AI-V対応：振替精算
//				return '2';
				// 振替精算中
				if( PiP_FurikaeSts ){
					return '5';
				}else{
					return '2';
				}
// MH322914(E) K.Onodera 2016/09/12 AI-V対応：振替精算
			case 1:					// 減額精算中
				return '4';
			case 2:					// 振替精算中
				return '5';
			default:
				break;
			}
			break;

		case 3:						// 精算完了
			return '3';

		case 0:						// 待機
			return '0';
		case 100:					// 休業
			return '1';
		default:					// その他
			break;
		}
	}
	return '9';
}
/*[]----------------------------------------------------------------------[]
 *|	name	: select discount data
 *[]----------------------------------------------------------------------[]
 *| summary	: 割引情報を選別する
 *| return	: 設定したデータ数
 *[]----------------------------------------------------------------------[]*/
void	select_discount_data(DISCOUNT_DATA* dst, const DISCOUNT_DATA* src)
{
	uchar	i,j;
// NOTE：第1引数は電文の割引エリアNTNET_DIC_MAX(25)で第2引数は内部保持エリアはWTIK_USEMAX(10)よりオーバーフローしないようにする
	for( i = j = 0; i < WTIK_USEMAX; i++ ){			// 精算中止以外の割引情報コピー
		if(( src[i].DiscSyu != 0 ) &&				// 割引種別あり
		   (( src[i].DiscSyu < NTNET_CSVS_M ) ||	// 精算中止割引情報でない
		    ( NTNET_CFRE < src[i].DiscSyu ))){
			memcpy( &dst[j], &src[i], sizeof( DISCOUNT_DATA ) );	// 割引情報
			if( dst[j].DiscSyu == NTNET_PRI_W ){
				memset( &dst[j].uDiscData, 0, 8 );	// 不要データクリア
			}
			j++;
		}
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: set discount data
 *[]----------------------------------------------------------------------[]
 *| summary	: 割引情報設定
 *| return	: 設定したデータ数
 *[]----------------------------------------------------------------------[]*/
int	set_discount_data(t_DiscountInfo* dst, DISCOUNT_DATA* src)
{
	int i, cnt;
	
// 割引情報はParkiProの電文上の割引エリアは25件だが、PayDataでは10件しか持っていないので、10回しかまわさない.
	for (i = 0, cnt = 0; i < WTIK_USEMAX; i++) {
		if (src[i].ParkingNo != 0) {
// MH810103 GG119202(S) PIP精算データの電子マネー対応
			// PayData.DiscountDataには問い合わせ番号をセットしないので下記処理は削除
//// MH321800(S) D.Inaba ICクレジット対応
//			// 問合せ番号は割引情報にセットさせない
//			if( src[i].DiscSyu == NTNET_INQUIRY_NUM ){
//				continue;
//			}
//// MH321800(E) D.Inaba ICクレジット対応
// MH810103 GG119202(E) PIP精算データの電子マネー対応
			//下3桁の駐車場No格納
			intoas(dst[cnt].ParkingNo, (ushort)(src[i].ParkingNo%1000), 3);		// 駐車場№
			intoas(dst[cnt].kind, src[i].DiscSyu, 3);						// 割引種別
			
			switch (src[i].DiscSyu) {
			case 11:		// 割引ﾌﾟﾘｶ
			case 12:		// 販売ﾌﾟﾘｶ
				intoasl(dst[cnt].div, *(ulong*)&src[i].DiscNo, 6);			// ｶｰﾄﾞNo.
				intoasl(dst[cnt].sum,  src[i].Discount , 6);				// 使用額
				intoasl(dst[cnt].inf1, src[i].DiscInfo1, 6);				// 残額
				break;
// MH810103 GG119202(S) PIP精算データの電子マネー対応
			// PayData.DiscountDataには電子マネー情報をセットしないので下記処理は削除
//			case 31:		// SUICA
//// MH321800(S) D.Inaba ICクレジット対応(アラーム取引仕様追加/処理未了取引ログ対応)
////			case 33:		// EDY
//// MH321800(E) D.Inaba ICクレジット対応(アラーム取引仕様追加/処理未了取引ログ対応)
//			case 35:		// PASMO
//			case 61:		// ICOCA
//			case 63:		// ICｶｰﾄﾞ
//			case 70:		// 乗車割引
//				if( src[i].DiscSyu != 33 ){									// Edyではない（Suica系ｶｰﾄﾞ決済）の場合
//					if( Is_SUICA_STYLE_OLD ){								// 旧版動作設定
//						if( src[i].DiscSyu != 35 ){							// PASMO以外の場合は
//							intoas(dst[cnt].kind, 31, 3);					// 割引種別をSuica固定とする
//						}													// PASMOの場合はPASMOの種別として送信
//					}else{													// 新仕様版では、Suica系カードは全てSuicaとして送信
//						intoas(dst[cnt].kind, 31, 3);						// 割引種別をSuica固定とする
//					}
//				}
//				memset(dst[cnt].div, 0x20, 24);								// ｽﾍﾟｰｽ(' ')ｾｯﾄ
//				memcpy(dst[cnt].div, (uchar*)&src[i].DiscNo, 20);			// ｶｰﾄﾞ番号
//				break;
//// MH321800(S) D.Inaba ICクレジット対応(アラーム取引仕様追加/処理未了取引ログ対応)
//			case 33:		// EDY
//			case 37:		// iD
//			case 65:		// WAON
//			case 81:		// QUICPay
//			case 85:		// nanaco
//			case 87:		// PiTaPa
//				memset(dst[cnt].div, 0x20, 24);								// ｽﾍﾟｰｽ(' ')ｾｯﾄ
//				memcpy(dst[cnt].div, (uchar*)&src[i].DiscNo, 20);			// ｶｰﾄﾞ番号
//				break;
//// MH321800(E) D.Inaba ICクレジット対応(アラーム取引仕様追加/処理未了取引ログ対応)
// MH810103 GG119202(E) PIP精算データの電子マネー対応
			case 91:		// 振替精算
				intoasl(dst[cnt].div, *(ulong*)&src[i].DiscNo, 6);			// 区画・車室番号
				intoasl(dst[cnt].sum, src[i].Discount , 6);					// 割引額
				memset(dst[cnt].inf2, 0x20, 6);								// ｽﾍﾟｰｽ(' ')ｾｯﾄ
				BCDtoASCII((uchar*)&src[i].DiscInfo1, dst[cnt].inf1, 4);	// 振替元精算月日時分
				break;
			default:
				intoas(dst[cnt].div,   src[i].DiscNo,    3);				// 割引区分
				intoas(dst[cnt].used,  src[i].DiscCount, 3);				// 枚数
				intoasl(dst[cnt].sum,  src[i].Discount , 6);				// 割引額
				intoasl(dst[cnt].inf1, src[i].DiscInfo1, 6);				// 割引情報１
				intoasl(dst[cnt].inf2, src[i].uDiscData.common.DiscInfo2, 6);// 割引情報２
				break;
			}
			cnt++;
		}
	}
	return cnt;
}

// MH810103 GG119202(S) PIP精算データの電子マネー対応
static int set_emoney_data(t_DiscountInfo* dst, Receipt_data* pay)
{
	int		i, cnt = 0;
	ushort	wk_kind1, wk_kind2;
	ushort	syu;
// MH810105(S) MH364301 QRコード決済対応
	int		j;


	if (pay->chuusi == 1) {
		// 精算中止時は電子マネー関連の割引種別をセットしない
		return cnt;
	}
// MH810105(E) MH364301 QRコード決済対応

	if (isEC_USE()) {
		// 電子マネー支払いあり？
		if (pay->Electron_data.Ec.pay_ryo == 0) {
			return 0;
		}
		for (i = 0; i < NTNET_DIC_MAX; i++) {
			// 空きを検索
			syu = astoin(dst[i].kind, 3);
			if (syu == 0) {
				break;
			}
		}
// MH810105(S) MH364301 QRコード決済対応
//		if (i < (NTNET_DIC_MAX-1)) {
		if (pay->Electron_data.Ec.e_pay_kind == EC_QR_USED) {
			j = 1;
		}
		else {
			j = 2;
		}
		if (i <= (NTNET_DIC_MAX-j)) {
// MH810105(E) MH364301 QRコード決済対応
			switch (pay->Electron_data.Ec.e_pay_kind) {					// 決済種別から振り分け
			case	EC_EDY_USED:
				wk_kind1 = NTNET_EDY_1;									// 割引種別：Edyｶｰﾄﾞ番号
				wk_kind2 = NTNET_EDY_2;									// 割引種別：Edy支払額、残額
				break;
			case	EC_NANACO_USED:
				wk_kind1 = NTNET_NANACO_1;								// 割引種別：nanacoｶｰﾄﾞ番号
				wk_kind2 = NTNET_NANACO_2;								// 割引種別：nanaco支払額、残額
				break;
			case	EC_WAON_USED:
				wk_kind1 = NTNET_WAON_1;								// 割引種別：WAONｶｰﾄﾞ番号
				wk_kind2 = NTNET_WAON_2;								// 割引種別：WAON支払額、残額
				break;
			case	EC_SAPICA_USED:
				wk_kind1 = NTNET_SAPICA_1;								// 割引種別：SAPICAｶｰﾄﾞ番号
				wk_kind2 = NTNET_SAPICA_2;								// 割引種別：SAPICA支払額、残額
				break;
			case	EC_KOUTSUU_USED:
				wk_kind1 = NTNET_SUICA_1;								// 割引種別：Suicaｶｰﾄﾞ番号
				wk_kind2 = NTNET_SUICA_2;								// 割引種別：Suica支払額、残額
				break;
			case	EC_ID_USED:
				wk_kind1 = NTNET_ID_1;									// 割引種別：iDｶｰﾄﾞ番号
				wk_kind2 = NTNET_ID_2;									// 割引種別：iD支払額、残額
				break;
			case	EC_QUIC_PAY_USED:
				wk_kind1 = NTNET_QUICPAY_1;								// 割引種別：QUICPayｶｰﾄﾞ番号
				wk_kind2 = NTNET_QUICPAY_2;								// 割引種別：QUICPay支払額、残額
				break;
// MH810105(S) MH364301 PiTaPa対応
			case	EC_PITAPA_USED:
				wk_kind1 = NTNET_PITAPA_1;								// 割引種別：PiTaPaｶｰﾄﾞ番号
				wk_kind2 = NTNET_PITAPA_2;								// 割引種別：PiTaPa支払額
				break;
// MH810105(E) MH364301 PiTaPa対応
// MH810105(S) MH364301 QRコード決済対応
			case	EC_QR_USED:
				wk_kind1 = NTNET_QR;									// 割引種別 QR決済
				wk_kind2 = 0;
				break;
// MH810105(E) MH364301 QRコード決済対応
			default:
				wk_kind1 = 0;
				wk_kind2 = 0;
				break;
			}

// MH810105(S) MH364301 QRコード決済対応
			if (pay->Electron_data.Ec.e_pay_kind == EC_QR_USED) {
				// QRこのコード決済
				// 下3桁の駐車場No格納
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// 駐車場№
				intoas(dst[i].kind, wk_kind1, 3);								// 割引種別
				intoas(dst[i].div, 0, 3);										// 割引区分
				intoas(dst[i].used, 1, 3);										// 使用枚数
				intoasl(dst[i].sum, pay->Electron_data.Ec.pay_ryo, 6);			// 支払金額
				intoas(dst[i].inf1, (ushort)pay->Electron_data.Ec.Brand.Qr.PayKind, 6);	// 割引情報1
				intoas(dst[i].inf2, 0, 6);
				if(pay->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1) {
					// みなし決済は0x30(決済ブランド不明)埋め
					memset(dst[i].inf1, '0', 6);								// 割引情報1
				}
				else{
					intoas(dst[i].inf1, (ushort)pay->Electron_data.Ec.Brand.Qr.PayKind, 6);	// 割引情報1
				}
				intoas(dst[i].inf2, 0, 6);										// 割引情報2
			}
			else
// MH810105(E) MH364301 QRコード決済対応
			// カード番号と支払額／残額をセットする（問い合わせ番号はセットしない）
			if (wk_kind1 != 0 && wk_kind2 != 0) {
				// 下3桁の駐車場No格納
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// 駐車場№
				intoas(dst[i].kind, wk_kind1, 3);								// 割引種別
				// カード番号
				memset(dst[i].div, 0x20, 24);									// ｽﾍﾟｰｽ(' ')ｾｯﾄ
// MH810103 MHUT40XX(S) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
//				memcpy(dst[i].div, pay->Electron_data.Ec.Card_ID, 20);			// ｶｰﾄﾞ番号
				if( pay->Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 1 &&
					pay->Electron_data.Ec.E_Status.BIT.deemed_settlement == 0 ) {
					// みなし決済＋決済結果OK受信によるみなし決済の精算データは会員No.をカード桁数分0x30埋めし左詰めでZZする。
					// [*]印字みなし決済と同様の会員Noをセットする。
					// それぞれのカード桁数分0x30埋め
					memset( dst[i].div, 0x30, (size_t)(EcBrandEmoney_Digit[pay->Electron_data.Ec.e_pay_kind - EC_USED]) );
					// 左詰めでZZ
					memset( dst[i].div, 'Z', 2 );
				}
				else {
					memcpy(dst[i].div, pay->Electron_data.Ec.Card_ID, 20);			// ｶｰﾄﾞ番号
				}
// MH810103 MHUT40XX(E) みなし＋決済OK受信時に精算データのカード番号が受信したカード番号のまま送信されてしまう
				i++;
				cnt++;

				// 下3桁の駐車場No格納
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// 駐車場№
				intoas(dst[i].kind, wk_kind2, 3);								// 割引種別
				intoas(dst[i].div, 0, 3);										// 割引区分
				intoas(dst[i].used, 1, 3);										// 使用枚数
				intoasl(dst[i].sum, pay->Electron_data.Ec.pay_ryo, 6);			// 支払金額
				intoasl(dst[i].inf1, pay->Electron_data.Ec.pay_after, 6);		// 残額
				switch (pay->Electron_data.Ec.e_pay_kind) {						// 決済種別から振り分け
				case	EC_EDY_USED:
// MH810103 MHUT40XX(S) Edy・WAON対応
//					intoasl(dst[i].inf2, (ulong)pay->Electron_data.Ec.Brand.Edy.DealNo, 6);			// Edy取引通番
					memset(dst[i].inf2, '0', 6);
					memcpy(&dst[i].inf2[1], pay->Electron_data.Ec.Brand.Edy.CardDealNo, 5);			// カード取引通番
// MH810103 MHUT40XX(E) Edy・WAON対応
					break;
				case	EC_NANACO_USED:
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//					intoasl(dst[i].inf2, (ulong)pay->Electron_data.Ec.Brand.Nanaco.DealNo, 6);		// nanaco取引通番
					memcpy(dst[i].inf2, pay->Electron_data.Ec.Brand.Nanaco.DealNo, 6);				// 端末取引通番
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
					break;
				case	EC_WAON_USED:
					intoasl(dst[i].inf2, pay->Electron_data.Ec.Brand.Waon.point, 6);				// WAON今回付与ポイント
					break;
				case	EC_SAPICA_USED:
					intoasl(dst[i].inf2, (ulong)pay->Electron_data.Ec.Brand.Sapica.Details_ID, 6);	// SAPICA一件明細ID
					break;
// MH810105(S) MH364301 PiTaPa対応
				case	EC_PITAPA_USED:
					memset(dst[i].inf1, '0', 6);
					memcpy(&dst[i].inf1[1], pay->Electron_data.Ec.Brand.Pitapa.Slip_No, 5);			// 伝票番号
					intoas(dst[i].inf2, 0, 6);
					break;
// MH810105(E) MH364301 PiTaPa対応
				default:
					intoas(dst[i].inf2, 0, 6);
					break;
				}
				cnt++;
			}
		}
	}
	else if (isSX10_USE()) {
		// 電子マネー支払いあり？
		if (pay->Electron_data.Suica.pay_ryo == 0) {
			return 0;
		}
		for (i = 0; i < NTNET_DIC_MAX; i++) {
			// 空きを検索
			syu = astoin(dst[i].kind, 3);
			if (syu == 0) {
				break;
			}
		}
		if (i < (NTNET_DIC_MAX-1)) {
			switch (pay->Electron_data.Ec.e_pay_kind) {					// 決済種別から振り分け
			case	OPE_DISP_MEDIA_TYPE_SUICA:
				wk_kind1 = NTNET_SUICA_1;								// 割引種別：Suicaｶｰﾄﾞ番号
				wk_kind2 = NTNET_SUICA_2;								// 割引種別：Suica支払額、残額
				break;
			case	OPE_DISP_MEDIA_TYPE_PASMO:
				wk_kind1 = NTNET_PASMO_1;								// 割引種別：PASMOｶｰﾄﾞ番号
				wk_kind2 = NTNET_PASMO_2;								// 割引種別：PASMO支払額、残額
				break;
			case	OPE_DISP_MEDIA_TYPE_ICOCA:
				wk_kind1 = NTNET_ICOCA_1;								// 割引種別：ICOCAｶｰﾄﾞ番号
				wk_kind2 = NTNET_ICOCA_2;								// 割引種別：ICOCA支払額、残額
				break;
			case	OPE_DISP_MEDIA_TYPE_ICCARD:
				wk_kind1 = NTNET_ICCARD_1;								// 割引種別：IC-Cardｶｰﾄﾞ番号
				wk_kind2 = NTNET_ICCARD_2;								// 割引種別：IC-Card支払額、残額
				break;
			default:
				wk_kind1 = 0;
				wk_kind2 = 0;
				break;
			}

			if (wk_kind1 != 0 && wk_kind2 != 0) {
				// 下3桁の駐車場No格納
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// 駐車場№
				intoas(dst[i].kind, wk_kind1, 3);								// 割引種別
				// カード番号
				memset(dst[i].div, 0x20, 24);									// ｽﾍﾟｰｽ(' ')ｾｯﾄ
				memcpy(dst[i].div, pay->Electron_data.Suica.Card_ID, 16);		// ｶｰﾄﾞ番号
				i++;
				cnt++;

				// 下3桁の駐車場No格納
				intoas(dst[i].ParkingNo, (ushort)(CPrmSS[S_SYS][1]%1000), 3);	// 駐車場№
				intoas(dst[i].kind, wk_kind2, 3);								// 割引種別
				intoas(dst[i].div, 0, 3);										// 割引区分
				intoas(dst[i].used, 1, 3);										// 使用枚数
				intoasl(dst[i].sum, pay->Electron_data.Ec.pay_ryo, 6);			// 支払金額
				intoasl(dst[i].inf1, pay->Electron_data.Ec.pay_after, 6);		// 残額
				intoas(dst[i].inf2, 0, 6);
				cnt++;
			}
		}
	}
	return cnt;
}
// MH810103 GG119202(E) PIP精算データの電子マネー対応

/*[]----------------------------------------------------------------------[]
 *|	name	: ID62
 *[]----------------------------------------------------------------------[]
 *| summary	: 駐車場情報送信
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_send_data62(void)
{
	tIFMMSG62	*msg;
	int			i;

	msg = (tIFMMSG62*)get_msgarea_and_init(62, sizeof(tIFMMSG62));
	
	make_BasicData(&msg->Basic);
	set_StatData(&msg->Stat);
	
	// 有効車室番号セット
	for (i = 0; i < ValidRoomNum; i++) {
		intoas(msg->CarInfo[i].Area, (ushort)LockInfo[ValidRoomNo[i]].area, 2);
		intoasl(msg->CarInfo[i].No,  LockInfo[ValidRoomNo[i]].posi, 4);
	}
	
	// データ長再計算
	msg->mlen = sizeof(tIFMMSG_BASIC) + sizeof(tM_STATUS);
	msg->mlen += (ushort)(sizeof(t_ShedInfo)*ValidRoomNum) + 6;

	ifm_send_msg();	// IFM電文送信
	
	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID63
 *[]----------------------------------------------------------------------[]
 *| summary	: 駐車場情報(車室情報なし)送信
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_send_data63(void)
{
	tIFMMSG63	*msg;

	msg = (tIFMMSG63*)get_msgarea_and_init(63, sizeof(tIFMMSG63));

	make_BasicData(&msg->Basic);
	set_StatData(&msg->Stat);

	ifm_send_msg();	// IFM電文送信

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID64
 *[]----------------------------------------------------------------------[]
 *| summary	: 全車室状況送信
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_send_data64(void)
{
	tIFMMSG64	*msg;
	int			i;

	msg = (tIFMMSG64*)get_msgarea_and_init(64, sizeof(tIFMMSG64));

	make_BasicData(&msg->Basic);
	set_StatData(&msg->Stat);

	// 有効車室状況セット
	for (i = 0; i < ValidRoomNum; i++) {
		set_ShedStat(&msg->ShedStat[i], ValidRoomNo[i]);
	}
	
	// データ長再計算
	msg->mlen = sizeof(tIFMMSG_BASIC) + sizeof(tM_STATUS);
	msg->mlen += (ushort)(sizeof(t_ShedStat)*ValidRoomNum) + 6;

	ifm_send_msg();	// IFM電文送信

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID65
 *[]----------------------------------------------------------------------[]
 *| summary	: １車室状況送信
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
uchar	ifm_send_data65(ushort pos)
{
	tIFMMSG65	*msg;

	msg = (tIFMMSG65*)get_msgarea_and_init(65, sizeof(tIFMMSG65));

	make_BasicData(&msg->Basic);
	set_StatData(&msg->Stat);
	set_ShedStat(&msg->ShedStat, (int)(pos-1));

	ifm_send_msg();	// IFM電文送信

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID66
 *[]----------------------------------------------------------------------[]
 *| summary	: 精算状況送信
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
	
	// データ長から割引分を除く
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
			intoas(msg->PayInfo.CarInfo.Area, (ushort)LockInfo[ryo_buf.pkiti-1].area, 2);	// 区画
			intoasl(msg->PayInfo.CarInfo.No,  LockInfo[ryo_buf.pkiti-1].posi, 4);			// 車室番号
		}else{
			intoas(msg->PayInfo.CarInfo.Area, (ushort)(LOCKMULTI.LockNo/10000), 2);			// 区画
			intoasl(msg->PayInfo.CarInfo.No,  LOCKMULTI.LockNo%10000, 4);					// 車室番号
		}
		
		if (ryo_buf.ryo_flg >= 2) {
			intoas(msg->PayInfo.ParkingNo, (ushort)(CPrmSS[S_SYS][PayData.teiki.pkno_syu+1]%1000), 4);		// 定期券駐車場番号
			intoas(msg->PayInfo.PassKind,  (ushort)PayData.teiki.syu, 2);			// 定期券種別
			intoasl(msg->PayInfo.PassID,   PayData.teiki.id, 6);					// 定期券ID（契約番号）
		}

		intoas(msg->PayInfo.FareKind, (ushort)(ryo_buf.syubet+1), 2);				// 料金種別

		// 入庫時刻
		intoas(&msg->PayInfo.tmEnter[0], (ushort)car_in_f.mon,  2);
		intoas(&msg->PayInfo.tmEnter[2], (ushort)car_in_f.day,  2);
		intoas(&msg->PayInfo.tmEnter[4], (ushort)car_in_f.hour, 2);
		intoas(&msg->PayInfo.tmEnter[6], (ushort)car_in_f.min,  2);
		
		if (ryo_buf.ryo_flg < 2)
			tmp = ryo_buf.tyu_ryo;									// 通常駐車料金
		else
			tmp = ryo_buf.tei_ryo;									// 定期使用駐車料金
		intoasl(msg->PayInfo.Fare, tmp, 6);							// 駐車料金
		intoasl(msg->SumInfo.Cash,    ryo_buf.zankin, 6);			// 支払い残額
		intoasl(msg->SumInfo.Entered, ryo_buf.nyukin, 6);			// 投入金額

		// 割引詳細
		cnt = set_discount_data(msg->DscntInfo, PayData.DiscountData);
// MH810103 GG119202(S) PIP精算データの電子マネー対応
		cnt += set_emoney_data(msg->DscntInfo, &PayData);
// MH810103 GG119202(E) PIP精算データの電子マネー対応

		
		// 設定された割引分をデータ長に追加
		msg->mlen += (ushort)(sizeof(t_DiscountInfo)*cnt);
	}

	ifm_send_msg();	// IFM電文送信

	return _IFM_ANS_SUCCESS;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: make and send response of request
 *[]----------------------------------------------------------------------[]
 *| summary	: 応答データの作成・送信
 *| param	: id   - 応答用id
 *| 		: msg  - 受信データ
 *|           rslt - 結果コード
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_send_rsp(int id, tIFMMSG *msg, uchar rslt)
{
	tIFMMSG			*rsp;
	tIFMMSG_BASIC	*basic;

	rsp = get_msgarea_and_init(id, _TOTAL_SIZE(msg));		// 領域確保
	memcpy(&rsp->data, &msg->data, (size_t)msg->mlen);		// 受信ﾃﾞｰﾀｺﾋﾟｰ
	basic = (tIFMMSG_BASIC*)rsp->data;
	basic->Rslt[0] = (uchar)((rslt/10) | '0');				// 結果ｺｰﾄﾞｾｯﾄ
	basic->Rslt[1] = (uchar)((rslt%10) | '0');
	ifm_send_msg();	// IFM電文送信
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID30
 *[]----------------------------------------------------------------------[]
 *| summary	: データ要求受信処理
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data30(tIFMMSG30 *msg)
{
	uchar			err = _IFM_ANS_SUCCESS;
	ushort 			req_id;
	ushort			pos;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG30));		// 電文長復元
	err = chk_BasicData(&msg->Basic, 1);			// 基本ﾃﾞｰﾀﾁｪｯｸ
	if (err == _IFM_ANS_SUCCESS) {
		req_id = (ushort)astoinl(msg->req.r30.ReqID, 2);
		switch (req_id) {
		case 62:					// 駐車場情報
			err = ifm_send_data62();
			break;
		case 63:					// 駐車場情報（車室情報なし）
			err = ifm_send_data63();
			break;
		case 64:					// 全車室情報
			err = ifm_send_data64();
			break;
		case 65:					// １車室情報
			err = chk_CarInfo(&msg->req.r30.CarInfo, &pos);		// 車室情報ﾁｪｯｸ
			if (err == _IFM_ANS_SUCCESS)
				err = ifm_send_data65(pos);
			break;
		case 66:					// 精算処理状況
			err = ifm_send_data66();
			break;
		default:
			err = _IFM_ANS_INVALID_PARAM;
			break;
		}
	}

	if (err) {
		// NG応答送信
		ifm_send_rsp(60, (tIFMMSG*)msg, err);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID40
 *[]----------------------------------------------------------------------[]
 *| summary	: 現在時刻変更要求受信処理
 *|           NTNET_RevData119(時計ﾃﾞｰﾀ受信処理)と等価
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

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG40));		// 電文長復元
	rslt = chk_BasicData(&msg->Basic, 0);			// 基本ﾃﾞｰﾀﾁｪｯｸ
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
			(date.Hour < 24 && date.Min < 60 ) &&	// 時間と分をﾁｪｯｸ ※Sec(秒)は元々設定範囲外なのでﾁｪｯｸしない	
			(chkdate((short)date.Year, (short)date.Mon, (short)date.Day) == 0)) {
			if (OPECTL.Mnt_mod == 0 &&
				(OPECTL.Ope_mod == 0 ||				// 待機状態?
				 OPECTL.Ope_mod == 100 ||			// 休業状態?
				 OPECTL.Ope_mod == 110)) {			// ﾄﾞｱ閉時ｱﾗｰﾑ表示状態?
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
				clkstr[0] = binbcd( (unsigned char)( clk_data.year / 100 ));	/* 年(上２桁) */
				clkstr[1] = binbcd( (unsigned char)( clk_data.year % 100 ));	/* 年(下２桁) */
				clkstr[2] = binbcd( clk_data.mont );							/* 月 */
				clkstr[3] = binbcd( clk_data.date );							/* 日 */
				clkstr[4] = binbcd( clk_data.hour );							/* 時間 */
				clkstr[5] = binbcd( clk_data.minu );							/* 分 */
				u_LifeTime.ul = LifeTim2msGet();
				clkstr[7] = u_LifeTime.uc[0];
				clkstr[8] = u_LifeTime.uc[1];
				clkstr[9] = u_LifeTime.uc[2];
				clkstr[10] = u_LifeTime.uc[3];
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//				Edy_Rec.edy_status.BIT.TIME_SYNC_REQ = 1;						// 日時同期データ送信要求
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
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

	// 応答データ送信
	ifm_send_rsp(70, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID41
 *[]----------------------------------------------------------------------[]
 *| summary	: フラップ・ロック制御要求受信処理
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data41(tIFMMSG41 *msg)
{
	tREQ41		*pt;
	ushort		pos, no;
	uchar		rslt;
	ulong		ulwork;
	
	msg->mlen = _MSG_LEN(sizeof(tIFMMSG41));		// 電文長復元
	rslt = chk_BasicData(&msg->Basic, 0);			// 基本ﾃﾞｰﾀﾁｪｯｸ
	if (rslt == _IFM_ANS_SUCCESS) {
		pt   = &msg->req.r41;
		rslt = chk_CarInfo(&pt->CarInfo, &pos);		// 車室情報ﾁｪｯｸ

		if (rslt == _IFM_ANS_SUCCESS) {
			/* 注：NTNETと以下の点が異なる
					NTNET		ロック開閉完了後に応答する
					ParkiPRO	ロック開閉指示後すぐに応答する
				NTNETと同じように完了後に応答するようにする場合は
					1)FLPTCBNOに送るメッセージをFLAP_UP_SND_NTNET/FLAP_DOWN_SND_NTNETに変更
					2)RecvBackUp.ReceiveFlgに2をセット
						→NTNET_Snd_Data05()の先頭でRecvBackUp.ReceiveFlg==2ならばIFMに応答
			*/
			no = pos - 1;
			ulwork = (ulong)(( LockInfo[no].area * 10000L ) + LockInfo[no].posi );
			switch (astoinl(pt->ProcMode, 2)) {
			case 1:							// ﾛｯｸ装置閉指示
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

			case 2:						// ﾛｯｸ装置開指示
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
				Kakari_Numu[pos-1] = 0;								// 強制出庫用係員№ｴﾘｱｸﾘｱ
				break;

			case 31:		// 暗証番号消去指示
				FLAPDT.flp_data[pos-1].passwd = 0;					// 暗証番号(Atype)消去
				wopelg(OPLOG_ANSHOU_B_CLR, 0, ulwork);
				break;

			default:
				rslt = _IFM_ANS_INVALID_PARAM;
				break;
			}
		}
	}
	
	// 応答データ送信
	ifm_send_rsp(71, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID42
 *[]----------------------------------------------------------------------[]
 *| summary	: 受付券発行要求受信処理
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data42(tIFMMSG42 *msg)
{
	ushort	pos, index;
	uchar	rslt, type;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG42));		// 電文長復元
	if ( prm_get( COM_PRM,S_TYP,62,1,1 ) == 0 ) {
		rslt = _IFM_ANS_REFUSED;					// 受付券機能なし
	}
	else {
		rslt = chk_BasicData(&msg->Basic, 0);			// 基本ﾃﾞｰﾀﾁｪｯｸ
		if (rslt == _IFM_ANS_SUCCESS) {
			rslt = chk_CarInfo(&msg->req.r42.CarInfo, &pos);	// 車室情報ﾁｪｯｸ
			if (rslt == _IFM_ANS_SUCCESS) {
				index = pos - 1;
				if (OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 0 &&			// 待機状態?
					(FLAPDT.flp_data[index].mode >= FLAP_CTRL_MODE2 && FLAPDT.flp_data[index].mode <= FLAP_CTRL_MODE4)) {	// 駐車中?
					// 受付券発行処理
					type = (FLAPDT.flp_data[index].uketuke == 0)? 0x12 : 0x11;
					FLAPDT.flp_data[index].uketuke = 0;					// 発行済みフラグOFF
					uke_isu((ulong)(LockInfo[index].area*10000L + LockInfo[index].posi), 
								(ushort)(pos), type);
					wopelg( OPLOG_PARKI_UKETUKEHAKKO, 0, 0 );			// 操作履歴登録
				}
				else {
					rslt = _IFM_ANS_REFUSED;
				}
			}
		}
	}
	
	// 応答データ送信
	ifm_send_rsp(72, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get receipt parameter
 *[]----------------------------------------------------------------------[]
 *| summary	: 領収証再発行要件取得
 *| return	: OK/NG
 *[]----------------------------------------------------------------------[]*/
uchar	get_receipt_key(t_PayInfo *pay, struct _receiptkey *key)
{
// 検索時に一致を取るので、特に値のチェックは不要
// （検索に使用しないものをチェック）
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
 *| summary	: 精算ログ検索
 *| return	: OK/NG
 *[]----------------------------------------------------------------------[]*/
uchar	search_receipt_data(struct _receiptkey *key, Receipt_data *wk)
{
	ushort		index;
// 検索条件
// 定期券更新：精算時刻、定期券駐車場番号、定期券種別、定期券IDが一致
// その他　　：精算時刻、精算追い番（!=0のとき）、車室番号が一致
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
 *| summary	: 領収証再発行要求受信処理
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data43(tIFMMSG43 *msg)
{
	T_FrmReceipt	rec_data;			// 領収証印字要求＞ﾒｯｾｰｼﾞﾃﾞｰﾀ
	ushort			serial_no;
	struct _receiptkey	key;
	uchar			rslt;
	
	msg->mlen = _MSG_LEN(sizeof(tIFMMSG43));		// 電文長復元
	rslt = chk_BasicData(&msg->Basic, 0);			// 基本ﾃﾞｰﾀﾁｪｯｸ
	if (rslt == _IFM_ANS_SUCCESS) {
		rslt = get_receipt_key(&msg->PayInfo, &key);			// 検索条件取得
		if (rslt == _IFM_ANS_SUCCESS) {
			serial_no = (ushort)astoinl(msg->Serial, 2);
			if (serial_no == 0 || serial_no != PayReceiptSerialNo) {
				PayReceiptSerialNo = serial_no;
				rslt = search_receipt_data(&key, &wkReceipt);		// 該当精算データを検索
				if (rslt == _IFM_ANS_SUCCESS) {
// GG129001(S) データ保管サービス対応（ジャーナル接続なし対応）
//					if (OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 0) {	// 待機状態のみ
					if (OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 0 &&	// 待機状態のみ
						Ope_isPrinterReady() != 0) {					// レシート印字可
// GG129001(E) データ保管サービス対応（ジャーナル接続なし対応）
					// 領収証再発行
						rec_data.prn_kind = R_PRI;					// ﾌﾟﾘﾝﾀ種別：ﾚｼｰﾄ
						rec_data.prn_data = &wkReceipt;				// 領収証印字ﾃﾞｰﾀのﾎﾟｲﾝﾀｾｯﾄ
						rec_data.kakari_no = 99;					// 係員No.99（固定）
						rec_data.reprint = ON;						// 再発行ﾌﾗｸﾞｾｯﾄ（再発行）
						memcpy( &rec_data.PriTime, &CLK_REC, sizeof(date_time_rec) );
																	// 再発行日時（現在日時）ｾｯﾄ
// MH810105(S) MH364301 インボイス対応
//						queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
//						wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );			// 操作履歴登録
//						LedReq( CN_TRAYLED, LED_ON );								// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED ON
//						LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// 電子決済中止領収証発行時の取り出し口LED消灯ﾀｲﾏｰ
						if (IS_INVOICE) {
							rec_data.prn_kind = J_PRI;				// ジャーナル印字が先
							memcpy(&Cancel_pri_work, &wkReceipt, sizeof(Cancel_pri_work));
							queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
							OPECTL.f_ReIsuType = 2;
						}
						else {
							queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &rec_data);
							wopelg( OPLOG_PARKI_RYOSHUSAIHAKKO, 0, 0 );	// 操作履歴登録
							LedReq( CN_TRAYLED, LED_ON );			// ｺｲﾝ取出し口ｶﾞｲﾄﾞLED ON
							LagTim500ms( LAG500_RECEIPT_LEDOFF_DELAY, OPE_RECIPT_LED_DELAY, op_ReciptLedOff );	// 電子決済中止領収証発行時の取り出し口LED消灯ﾀｲﾏｰ
						}
// MH810105(E) MH364301 インボイス対応
					}
					else {
						rslt = _IFM_ANS_REFUSED;
					}
					msg->PayInfo.Receipt = (uchar)('0' + wkReceipt.ReceiptIssue);
				}
			}
		}
	}
	
	// 応答データ送信
	ifm_send_rsp(73, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID44
 *[]----------------------------------------------------------------------[]
 *| summary	: 減額精算要求受信処理
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data44(tIFMMSG44 *msg)
{
	ushort		pos;
	uchar		rslt;
	struct VL_GGS		ggs;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG44));		// 電文長復元
	rslt = chk_BasicData(&msg->Basic, 0);			// 基本ﾃﾞｰﾀﾁｪｯｸ
	if (rslt == _IFM_ANS_SUCCESS) {
		rslt = chk_CarInfo(&msg->PayInfo.CarInfo, &pos);	// 車室情報ﾁｪｯｸ
		if (rslt == _IFM_ANS_SUCCESS) {
			if (OPECTL.Mnt_mod == 0 && OPECTL.Ope_mod == 2 &&		// 通常精算状態?
				ryo_buf.pkiti == pos) {								// 精算中の車室と一致?
				ggs.ryokin = (ulong)astoinl(msg->PayInfo.Fare, 6);
				ggs.zangaku = (ulong)astoinl(msg->SumInfo.Rest, 6);
				ggs.tonyugaku = (ulong)astoinl(msg->SumInfo.Entered, 6);
				ggs.waribikigaku = (ulong)astoinl(msg->SumInfo.Discount, 6);

				if (ggs.ryokin >= ggs.waribikigaku) {
					if (ope_imf_Pay(OPEIMF_RCVKIND_GENGAKU, (void*)&ggs)) {
						memcpy(&cache.msg44, msg, (size_t)_TOTAL_SIZE(msg));
						return;		// 応答はOpeからIFM_Snd_GengakuAns()を呼ぶ
					}
					else {
						rslt = _IFM_ANS_IMPOSSIBLE;
					}
				} else {
					// 駐車料金が割引額未満だったら電文エラー
					rslt = _IFM_ANS_INVALID_PARAM;
				}
			}
			else {
				rslt = _IFM_ANS_REFUSED;
			}
		}
	}
	
	// 応答データ送信
	ifm_send_rsp(74, (tIFMMSG*)msg, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID45 & ID46
 *[]----------------------------------------------------------------------[]
 *| summary	: 振替精算チェック
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
	// 応答電文生成
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

	msg->mlen = _MSG_LEN(sizeof(tIFMMSG46));		// 電文長復元
	rslt = chk_BasicData(&msg->Basic, 0);			// 基本ﾃﾞｰﾀﾁｪｯｸ
	if (rslt != _IFM_ANS_SUCCESS)
		return rslt;
	rslt = chk_CarInfo(&msg->req.r46.CarInfo1, &pos1);		// 振替元車室情報ﾁｪｯｸ
	if (rslt != _IFM_ANS_SUCCESS)
		return rslt;
	rslt = chk_CarInfo(&msg->req.r46.CarInfo2, &pos2);		// 振替先車室情報ﾁｪｯｸ
	if (rslt != _IFM_ANS_SUCCESS)
		return rslt;
	src = &FLAPDT.flp_data[pos1-1];
	if (src->mode < FLAP_CTRL_MODE5 || src->mode > FLAP_CTRL_MODE6)	// 振替元ロック解除?
		return _IFM_ANS_FROM_NG;
	if (OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod != 2)			// 通常精算状態?
		return _IFM_ANS_REFUSED;
	if (ryo_buf.pkiti != pos2)								// 振替先が精算車室と一致?
		return _IFM_ANS_TO_NG;
	if ( OPECTL.InquiryFlg )								// ｸﾚｼﾞｯﾄのHOST接続中か電子媒体の引去り失敗ﾘﾄﾗｲ中の場合
		return _IFM_ANS_NOW_BUSY;							// Busyとして返す（値：７）
	if ( OPECTL.Pay_mod == 2 )								// 修正精算中
		return _IFM_ANS_TO_NG;
	key.PayCount     = 0;
	key.OutTime.Mon  = src->s_mont;
	key.OutTime.Day  = src->s_date;
	key.OutTime.Hour = src->s_hour;
	key.OutTime.Min  = src->s_minu;
	key.WPlace       = (ulong)(( LockInfo[pos1-1].area * 10000L ) + LockInfo[pos1-1].posi );
	rslt = search_receipt_data(&key, &receipt);				// 該当精算データを検索
	if (rslt != _IFM_ANS_SUCCESS)
		return _IFM_ANS_FROM_NG;
	if (msg->ID[1] == '5') {
	// 事前チェック
		frs->antipassoff_req = (receipt.teiki.id)? 1 : 0;	// 振替元で定期使用
	}
	else {
	// 実行
		if (msg->req.r46.PassUse != '0' && msg->req.r46.PassUse != '1')
			return _IFM_ANS_INVALID_PARAM;
		frs->antipassoff_req = (uchar)(msg->req.r46.PassUse - '0');		// 要求を保存
	}
	frs->lockno = pos1;
	frs->price = receipt.WPrice /* + receipt.Wtax */;		// 内税ならばWtax==0：外税対応しない
#if	0
--	i = is_paid_remote(&receipt);
--	if (i >= 0 && receipt.wari_data[i].tik_syu == MISHUU) {
--		frs->price -= receipt.wari_data[i].ryokin;			// 未収金分減額
--	}
#endif
	// 以下の項目は０で初期化されているので、無条件に積算可
	frs->in_price = receipt.WInPrice - receipt.WChgPrice;	// 現金
	for (i = 0; i < WTIK_USEMAX; i++) {
		if ((receipt.DiscountData[i].DiscSyu == NTNET_FRE ) ||	// 回数券
		    (receipt.DiscountData[i].DiscSyu == NTNET_PRI_W )) {// プリペイドカード
			frs->in_price += receipt.DiscountData[i].Discount;	// 割引額加算
		}
	}
	frs->in_price += receipt.ppc_chusi;						// プリペイド／回数券中止データ
	frs->in_price += receipt.credit.pay_ryo;				// クレジット
	frs->in_price += receipt.Electron_data.Suica.pay_ryo;	// 電子マネー
		//　Suica(PASMO)とEdyはunionでpay_ryoの位置は変化しない
	frs->syubetu = receipt.syu;								// 料金種別
	frs->seisan_oiban = receipt.Oiban;						// 精算追い番
	frs->seisan_time = receipt.TOutTime;					// 精算時刻
	return rslt;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID45 & ID46
 *[]----------------------------------------------------------------------[]
 *| summary	: 振替精算情報セット
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	set_sw_info(tSWP_INFO *info, uchar rslt, struct VL_FRS *frs)
{
	ulong	ryokin;

	memset(info, 'N', sizeof(*info));
	if (rslt == _IFM_ANS_SUCCESS || rslt == _IFM_ANS_DONE) {
		intoasl(info->FailedPaid, frs->price, 6);				// 振替元支払額
		intoasl(info->Switchable, frs->furikaegaku, 6);			// 振替可能額
		if (ryo_buf.ryo_flg < 2)
			ryokin = ryo_buf.tyu_ryo;
		else
			ryokin = ryo_buf.tei_ryo;

		intoasl(info->OrgFare, ryokin, 6);						// 振替先駐車料金
		ryokin = frs->zangaku;
	}
	if (rslt == _IFM_ANS_SUCCESS) {
		intoasl(info->OrgArrear, ryokin, 6);					// 振替先支払残額
		if (ryokin < frs->furikaegaku)
			ryokin = 0;
		else
			ryokin -= frs->furikaegaku;
		intoasl(info->ActArrear,  ryokin, 6);					// 振替後支払残額
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID45
 *[]----------------------------------------------------------------------[]
 *| summary	: 振替精算実行可能チェック受信処理
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data45(tIFMMSG45 *msg)
{
	tIFMMSG75		rsp;
	uchar			rslt;
	struct VL_FRS	frs;

	// 振替チェック
	rslt = ifm_check_furikae((tIFMMSG46*)msg, (tIFMMSG76*)&rsp, &frs);
	if (rslt == _IFM_ANS_SUCCESS) {
	// 現状での振替可能額を求める
		ope_ifm_GetFurikaeGaku(&frs);
		rsp.PassUse = (uchar)(frs.antipassoff_req | '0');
	}
	// 応答データ送信
	rsp.ID[1] = '5';
	set_sw_info(&rsp.SwInfo, rslt, &frs);
	ifm_send_rsp(75, (tIFMMSG*)&rsp, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID46
 *[]----------------------------------------------------------------------[]
 *| summary	: 振替精算要求受信処理
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_data46(tIFMMSG46 *msg)
{
	tIFMMSG76		rsp;
	uchar			rslt;
	struct VL_FRS	frs;

	// 振替チェック
	rslt = ifm_check_furikae(msg, &rsp, &frs);
	if (rslt == _IFM_ANS_SUCCESS) {
		if (ope_imf_Pay(OPEIMF_RCVKIND_FURIKAE, (void*)&frs)) {
			memcpy(&cache.msg76, &rsp, (size_t)_TOTAL_SIZE((&rsp)));
			return;		// 応答はOpeからIFM_Snd_FurikaeAns()を呼ぶ
		}
		rslt = _IFM_ANS_FROM_NG;
	}
	// 応答データ送信
	set_sw_info(&rsp.SwInfo, rslt, &frs);
	ifm_send_rsp(76, (tIFMMSG*)&rsp, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: received IBK error message
 *[]----------------------------------------------------------------------[]
 *| summary	: IBKエラー通知受信
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ifm_recv_ibkerr(tIFMMSGB0 *msg)
{
	char	mdl_code, err_code, err_occur;

	msg->mlen = _MSG_LEN(sizeof(tIFMMSGB0));		// 電文長復元
	mdl_code	= (char)astoin ( msg->ModuleCode, 2 );
	err_code	= (char)astoin ( msg->ErrorCode, 2 );
	err_occur	= (char)astoin ( msg->ErrorOccur, 2 );

	// モジュールコード１２・５５が有効
	if (mdl_code == 12)
		mdl_code = 55;		// 12は55に変更して登録
	else if (mdl_code != 55 )
		return;

	// 取得ﾃﾞｰﾀの形式をあわせる。
	switch( err_occur ){
	case	1:
		if (err_code == 1)
			err_occur = 1;		// 発生
		else
			err_occur = 2;		// 発生・解除同時
		break;
	case	2:
		err_occur = 0;			// 解除
		break;
	default:
		err_occur = 2;			// 発生・解除同時
		break;
	}

	// 登録を行う
	err_chk( ERRMDL_REMOTE, err_code, err_occur, 0, 0 );
}

/*[]----------------------------------------------------------------------[]
 *|	name	: IFM receive message
 *[]----------------------------------------------------------------------[]
 *| summary	: IFM電文受信
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
	case	30:					//データ要求
		ifm_recv_data30(&ifm_rcv_buf.msg30);
		break;
	case	40:					// 現在時刻変更要求
		ifm_recv_data40(&ifm_rcv_buf.msg40);
		break;
	case	41:					// フラップ・ロック制御要求
		ifm_recv_data41(&ifm_rcv_buf.msg41);
		break;
	case	42:					// 受付券発行要求
		ifm_recv_data42(&ifm_rcv_buf.msg42);
		break;
	case	43:					// 領収証再発行要求
		ifm_recv_data43(&ifm_rcv_buf.msg43);
		break;
	case	44:					// 減額精算要求
		ifm_recv_data44(&ifm_rcv_buf.msg44);
		break;
	case	45:					// 振替精算実行可能チェック
		ifm_recv_data45(&ifm_rcv_buf.msg45);
		break;
	case	46:					// 振替精算要求
		ifm_recv_data46(&ifm_rcv_buf.msg46);
		break;
	case	0xB0:				// IBKエラー通知
		ifm_recv_ibkerr(&ifm_rcv_buf.msgB0);
		break;
	default:
		break;
	}
	ifm_rcv_buf.buff.mlen = 0;		// receive buffer empty
}

//==============================================================================
//	シリアル受信データをIFMのバッファへ変換
//	@argument	Length		受信データ長
//	@argument	Data		受信データ
//	@return		なし
//	@note		シリアル受信データをIFMのバッファへ変換
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void PIP_ChangeData( unsigned short Length, unsigned char *pData)
{
	t_DataDetail	*raublk;
	
	raublk = (t_DataDetail*)pData;
	if (raublk->ID1[0] == '0' && raublk->ID1[1] == '1' &&				// ID1、ID2は固定値なので、間違いが無いことを確認
		raublk->ID2[0] == '0' && raublk->ID2[1] == '1') {
		ifm_rcv_buf.buff.ID[0] = raublk->ID3[0];						// 種別
		ifm_rcv_buf.buff.ID[1] = raublk->ID3[1];
		ifm_rcv_buf.buff.mlen  = Length-6;								// 長さ
		memset(ifm_rcv_buf.buff.data, '0', IFM_DATA_MAX);
		memcpy(ifm_rcv_buf.buff.data, raublk->dat, (size_t)(Length-6));	// データのアドレスを渡す
		ifm_recv_msg();													// IFM電文受信
	}
}
//==============================================================================
//	データテーブルの初期化
//	@argument	なし
//	@return		なし
//	@note		精算データテーブルとエラーデータテーブルを初期化する
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void IFM_RcdBufClrAll()
{
	uchar	i;
	
	if (_is_not_pip()){		// PARKiPRO？
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
//	精算データテーブルへのデータ蓄積
//	@argument	なし
//	@return		なし
//	@note		精算データをParkiPRO用の精算テーブルに蓄積する
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void PIP_PayTable_insert(tIFMMSG *msg)
{
	uchar	COUNT = PAYTBL.COUNT;
	uchar	RIND = PAYTBL.RIND;
	uchar	CIND = PAYTBL.CIND;
	ushort	LEN = msg->mlen - 4;

	if (_is_not_pip()){		// PARKiPRO？
		return;
	}

	if( COUNT < LOGTBL_MAX ){			// 保存電文数２０件以下
		COUNT++;
	} else {							// 既に２０件溜まっているとき
		RIND++;							// 最古電文位置を変更する。
		if( RIND >= LOGTBL_MAX )
			RIND = 0;
	}
	CIND++;								// 次回書き込み電文位置を変更する。
	if( CIND >= LOGTBL_MAX )
		CIND = 0;


	memcpy( &PAYTBL.DATA[PAYTBL.CIND].BUFF[0], msg->data, LEN );	// 精算テーブルへ蓄積
	PAYTBL.DATA[PAYTBL.CIND].LEN = LEN;

	PAYTBL.COUNT = COUNT;		// 更新
	PAYTBL.RIND = RIND;			// 更新
	PAYTBL.CIND = CIND;			// 更新
}
//==============================================================================
//	精算データ変換処理
//	@argument	dat	精算データ
//	@return		なし
//	@note		精算データを変換し、蓄積処理を呼ぶ
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Snd_Payment(const Receipt_data *dat)
{
	tIFMMSG67		*msg;
	ushort			local_PayClass;	// 処理区分
	uchar			i;

	if (_is_not_pip()){		// PARKiPRO？
		return;
	}
// MH810105(S) MH364301 QRコード決済対応（ParkiProデータ修正）
	if( dat->PayClass == 2 || dat->PayClass == 3 ){				// 中止？
		if( !dat->WInPrice &&									// 入金なし
			!carduse() &&										// カード支払いなし
			isEC_PAY_CANCEL() ){								// 電子決済端末使用あり
			// 中止精算データ送信時、
			// chu_isu()が呼ばれた原因が電子決済端末使用のみである場合
			return;												// 送信しない
		}
	}
// MH810105(E) MH364301 QRコード決済対応（ParkiProデータ修正）


	msg = (tIFMMSG67*)get_msgarea_and_init(67, sizeof(tIFMMSG67));
	make_BasicData(&msg->Basic);

	// データ追番
	intoas(msg->Serial, PayDataSerialNo, 2);
	if (PayDataSerialNo++ > 99){
		PayDataSerialNo = 1;
	}
	
	// 精算情報
	intoas(msg->PayInfo.Method,   dat->PayMethod, 2);				// 精算方法
	if( (PayData.Electron_data.Suica.pay_ryo != 0) && (dat->PayClass == 0 || dat->PayClass == 1) ){
		local_PayClass = dat->PayClass + 4;
	} else {
		local_PayClass = dat->PayClass;
	}
	intoas(msg->PayInfo.PayClass, local_PayClass,  2);				// 処理区分
	intoas(msg->PayInfo.Mode,     dat->PayMode,   2);				// 精算モード
	intoas(msg->PayInfo.OutKind,  dat->OutKind,   2);				// 精算出庫
	msg->PayInfo.Receipt += (uchar)dat->ReceiptIssue;				// 領収証

	intoasl(msg->PayInfo.PayCount, CountSel((ST_OIBAN *)&dat->Oiban ), 6);// 精算or精算中止追い番

	intoas(msg->PayInfo.tmPay,     (ushort)dat->TOutTime.Mon,  2);	// 出庫時刻：月
	intoas(&msg->PayInfo.tmPay[2], (ushort)dat->TOutTime.Day,  2);	// 　　　　　日
	intoas(&msg->PayInfo.tmPay[4], (ushort)dat->TOutTime.Hour, 2);	// 　　　　　時
	intoas(&msg->PayInfo.tmPay[6], (ushort)dat->TOutTime.Min,  2);	// 　　　　　分

	intoasl(msg->PayInfo.Fare, dat->WPrice, 6);						// 駐車料金

	intoasl(msg->PayInfo.CarInfo.Area, (dat->WPlace/10000), 2);		// 車室情報：区画
	intoasl(msg->PayInfo.CarInfo.No,   (dat->WPlace%10000), 4);		// 　　　　　番号
	
	intoasl(msg->PayInfo.ParkingNo, (dat->teiki.ParkingNo%1000), 4);	// 定期券駐車場番号
	intoas(msg->PayInfo.PassKind,   dat->teiki.syu, 2);					// 定期券種別
	intoasl(msg->PayInfo.PassID,    dat->teiki.id, 6);					// 定期券ID
	if (dat->PassCheck) {
		/* エリアを初期化する際にアスキー値'0'で初期化しているため、+=で値を代入する */
		msg->PayInfo.AntiPassChk += 1;				// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	}
	msg->PayInfo.StockCnt    += (uchar)dat->CountSet;				// 在車ｶｳﾝﾄ
	intoas(msg->PayInfo.FareKind, dat->syu, 2);						// 料金種別

	intoas(msg->PayInfo.tmEnter,     (ushort)dat->TInTime.Mon,  2);	// 入庫時刻：月
	intoas(&msg->PayInfo.tmEnter[2], (ushort)dat->TInTime.Day,  2);	// 　　　　　日
	intoas(&msg->PayInfo.tmEnter[4], (ushort)dat->TInTime.Hour, 2);	// 　　　　　時
	intoas(&msg->PayInfo.tmEnter[6], (ushort)dat->TInTime.Min,  2);	// 　　　　　分

	// 金額情報
	intoasl(msg->SumInfo.Cash,       dat->WInPrice - dat->WChgPrice, 6);	// 現金売上
	intoasl(msg->SumInfo.Entered,    dat->WInPrice, 6);						// 投入金額
	intoasl(msg->SumInfo.Change,     dat->WChgPrice, 6);					// 釣銭金額
	intoasl(msg->SumInfo.Shortage,   dat->WFusoku, 6);						// 釣銭払出不足金額
	intoasl(msg->SumInfo.CreditCard, dat->credit.pay_ryo, 6);				// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額

	// 割引情報
	for( i = 0; i < NTNET_DIC_MAX; i++ ){
		memset( &DiscountData_Buff[i],0,sizeof(DiscountData_Buff[0]));
	}
	select_discount_data( DiscountData_Buff, dat->DiscountData);	// 割引情報を選別する
	set_discount_data(msg->DscntInfo, DiscountData_Buff);			// 割引情報設定
// MH810103 GG119202(S) PIP精算データの電子マネー対応
	set_emoney_data(msg->DscntInfo, &PayData);
// MH810103 GG119202(E) PIP精算データの電子マネー対応

	PIP_PayTable_insert((tIFMMSG*)msg);
}

//==============================================================================
//	精算データ送信処理
//	@argument	count	要求されている精算データの件数
//	@return		なし
//	@note		要求された数の精算データを蓄積データより取得し、
//	@note		ParkiPROへの送信バッファへ挿入する
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Snd_AccumulatePayment(uchar count)
{
	tIFMMSG		*msg;
	ushort		index;
	
	if (_is_not_pip()){		// PARKiPRO？
		return;
	}

	if( count > PAYTBL.COUNT )	count = PAYTBL.COUNT;			// 要求件数テーブルにない場合は補正
	if( count == 0 )	return;									// 送信できるデータはない。
	index = (PAYTBL.RIND + (PAYTBL.COUNT - 1) - ( count - 1 ));	// 最初の送信パケット位置を指定(計算上負の値になることはない)
	if( index >= LOGTBL_MAX )	index -= LOGTBL_MAX;			// 突き抜けたら上側へ(算出完了)
	if( index >= LOGTBL_MAX ){
		return;
	}
	
	do{
		msg = (tIFMMSG*)get_msgarea_and_init(67, sizeof(tIFMMSG67));
		memcpy( &msg->data[0] , &PAYTBL.DATA[index].BUFF[0] , PAYTBL.DATA[index].LEN);
		ifm_send_msg();	// IFM電文送信
		index++;
		if( index >= LOGTBL_MAX ){
			index = 0;
		}
	}while( index !=  PAYTBL.CIND );
}
//==============================================================================
//	精算データ初期作成
//	@argument	なし
//	@return		なし
//	@note		精算機起動時、本体Logより精算ログを取得し、
//	@note		ParkiPRO用の精算データテーブルへ保存する(20件)
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Accumulate_Payment_Init()
{
	ushort			LogCount;		// ログの登録件数
	ushort			DatagetCount;	// ログ取得カウント
	uchar			flg_tgt;		// 強制/不正出庫か判断するフラグ
	if (_is_not_pip()){		// PARKiPRO？
		return;
	}

	LogCount = Ope_Log_TotalCountGet(eLOG_PAYMENT);	// ログ登録件数取得
	DatagetCount = LOGTBL_MAX;						// ログ取得カウント
	
	if( LogCount <= DatagetCount ){
		DatagetCount = 0;						// ログ登録件数が20件未満の場合、ログ登録件数分だけ返す
	} else {
		DatagetCount = LogCount - DatagetCount;	// 取得ログの先頭(最新から20番目のログ) 登録件数 - 20件 
	}

	while( LogCount > DatagetCount ){											// 最新ログ LogCount == countまで取得する
		if( 1 == Ope_Log_1DataGet( eLOG_PAYMENT, DatagetCount, &Receipt_buf ) )	// 先頭(最古)からcount番目のログを取得
		{
			flg_tgt = 0;
			if(( Receipt_buf.PayClass == 2 ) ||					// 処理区分＝精算中止?
				 ( Receipt_buf.PayClass == 3 )){				// 処理区分＝再精算中止?
					flg_tgt = 1;								// 指定された個別精算ログ取出しOK
			}else if(( Receipt_buf.OutKind !=  3 ) &&			// 精算出庫≠不正出庫?
					 ( Receipt_buf.OutKind !=  1 ) &&			// 精算出庫≠強制出庫?
					 ( Receipt_buf.OutKind != 11 ) &&			// 精算出庫≠強制出庫(遠隔)?
					 ( Receipt_buf.OutKind != 97 )){			// フラップ上昇、ロック閉タイマ内出庫
					flg_tgt = 1;								// 指定された個別精算ログ取出しOK
			}
			if( flg_tgt == 1 ){	// 強制/不正出庫ではない場合
				IFM_Snd_Payment(&Receipt_buf);
			}
		}
		DatagetCount++;
	}
}

//==============================================================================
//	エラーデータテーブルへのデータ蓄積
//	@argument	msg	エラーデータ(変換済み)
//	@return		なし
//	@note		エラーデータをParkiPRO用のエラーテーブルに蓄積する
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void PIP_ErrTable_insert(tIFMMSG *msg)
{
	uchar	COUNT = ERRTBL.COUNT;
	uchar	RIND = ERRTBL.RIND;
	uchar	CIND = ERRTBL.CIND;
	ushort	LEN = msg->mlen;

	if (_is_not_pip()){		// PARKiPRO？
		return;
	}
	if( COUNT < LOGTBL_MAX ){		// 保存電文数２０件以下
		COUNT++;
	} else {						// 既に２０件溜まっているとき
		RIND++;						// 最古電文位置を変更する。
		if( RIND >= LOGTBL_MAX )
			RIND = 0;
	}
	CIND++;							// 次回書き込み電文位置を変更する。
	if( CIND >= LOGTBL_MAX )
		CIND = 0;

	memcpy( &ERRTBL.ERROR[ERRTBL.CIND].BUFF[0], msg->data, LEN );
	ERRTBL.ERROR[ERRTBL.CIND].LEN = LEN;

	ERRTBL.COUNT = COUNT;		// 更新
	ERRTBL.RIND = RIND;			// 更新
	ERRTBL.CIND = CIND;			// 更新
}
//==============================================================================
//	エラーデータ変換処理(発生時)
//	@argument	なし
//	@return		なし
//	@note		エラーデータを変換し、蓄積処理を呼ぶ
//	@note		エラーの情報は「Err_work」より取得
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
	case 1:				// 発生
		msg->Stat[1] = '1';						// ｴﾗｰ発生
		break;
	case 2:				// 発生・解除
		msg->Stat[1] = '3';						// ｴﾗｰ発生・解除
		break;
	default:			// 解除
		msg->Stat[1] = '2';						// ｴﾗｰ解除
		break;
	}
	set_MMDDhhmm(msg->Time);
	PIP_ErrTable_insert((tIFMMSG*)msg);
}
//==============================================================================
//	エラーデータ変換処理(解除時)
//	@argument	なし
//	@return		なし
//	@note		エラーデータを変換し、蓄積処理を呼ぶ
//	@note		解除時はコードが決まっている。
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
	msg->Stat[1] = '3';						// ｴﾗｰ発生・解除
	set_MMDDhhmm(msg->Time);
	PIP_ErrTable_insert((tIFMMSG*)msg);
}

//==============================================================================
//	エラーデータ送信処理
//	@argument	count	要求されているエラーデータの件数
//	@return		なし
//	@note		要求された数のエラーデータを蓄積データより取得し、
//	@note		ParkiPROへの送信バッファへ挿入する
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Snd_AccumulateError(uchar count)
{
	tIFMMSG		*msg;
	ushort		index;
	
	if (_is_not_pip()){		// PARKiPRO？
		return;
	}
	if( count > ERRTBL.COUNT )	count = ERRTBL.COUNT;			// 要求件数テーブルにない場合は補正
	if( count == 0 )	return;									// 送信できるデータはない。
	index = (ERRTBL.RIND + (ERRTBL.COUNT - 1) - ( count - 1 ));	// 最初の送信パケット位置を指定(計算上負の値になることはない)
	if( index >= LOGTBL_MAX )	index -= LOGTBL_MAX;			// 突き抜けたら上側へ(算出完了)
	if( index >= LOGTBL_MAX ){
		return;
	}
	
	do{
		msg = (tIFMMSG*)get_msgarea_and_init(68, sizeof(tIFMMSG68));
		memcpy( &msg->data[0] , &ERRTBL.ERROR[index].BUFF[0] , ERRTBL.ERROR[index].LEN);
		ifm_send_msg();	// IFM電文送信
		index++;
		if( index >= LOGTBL_MAX ){
			index = 0;
		}
	}while( index !=  ERRTBL.CIND );
}

//==============================================================================
//	エラーデータ初期作成
//	@argument	なし
//	@return		なし
//	@note		精算機起動時、本体Logよりエラーログを取得し、
//	@note		ParkiPRO用のエラーデータテーブルへ保存する(20件)
//	@author		2012/10/24 yanagawa
//======================================== Copyright(C) 2006 AMANO Corp. =======
void	IFM_Accumulate_Error_Init()
{
	tIFMMSG68	*msg;
	Err_log		pSetBuf;
	ushort		LogCount;		// ログ登録件数取得
	ushort		DatagetCount;	// ログ取得カウント
	
	if (_is_not_pip()){		// PARKiPRO？
		return;
	}
	LogCount = Ope_Log_TotalCountGet(eLOG_ERROR);	// ログ登録件数取得
	DatagetCount = LOGTBL_MAX;						// ログ取得カウント

	if( LogCount <= DatagetCount ){
		DatagetCount = 0;						// ログ登録件数が20件未満の場合、ログ登録件数分だけ返す
	} else {
		DatagetCount = LogCount - DatagetCount;	// 取得ログの先頭(最新から20番目のログ) 登録件数 - 20件 
	}

	while( LogCount > DatagetCount ){										// 最新ログ LogCount == DatagetCountになるまで取得する
		if( 1 == Ope_Log_1DataGet( eLOG_ERROR, DatagetCount, &pSetBuf ) )	// 先頭(最古)からcount番目のログを取得
		{
			msg = (tIFMMSG68*)get_msgarea_and_init(68, sizeof(tIFMMSG68));
			make_BasicData(&msg->Basic);
			intoas(msg->ModuleCode, (ushort)pSetBuf.Errsyu, sizeof(msg->ModuleCode));
			intoas(msg->ErrCode, (ushort)pSetBuf.Errcod, sizeof(msg->ErrCode));
			switch( pSetBuf.Errdtc ){
			case 1:				// 発生
				msg->Stat[1] = '1';						// ｴﾗｰ発生
				break;
			case 2:				// 発生・解除
				msg->Stat[1] = '3';						// ｴﾗｰ発生・解除
				break;
			default:			// 解除
				msg->Stat[1] = '2';						// ｴﾗｰ解除
				break;
			}
			intoas(msg->Time,     (ushort)pSetBuf.Date_Time.Mon,  2);	// 出庫時刻：月
			intoas(&msg->Time[2], (ushort)pSetBuf.Date_Time.Day,  2);	// 　　　　　日
			intoas(&msg->Time[4], (ushort)pSetBuf.Date_Time.Hour, 2);	// 　　　　　時
			intoas(&msg->Time[6], (ushort)pSetBuf.Date_Time.Min,  2);	// 　　　　　分
			PIP_ErrTable_insert((tIFMMSG*)msg);
		}
		DatagetCount++;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ID74
 *[]----------------------------------------------------------------------[]
 *| summary	: 減額精算応答
 *| param	: sts(受付時の応答：1/-1/-7、完了時の応答：2/-2/-3)
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
 *| summary	: 振替精算応答
 *| param	: sts(受付時の応答：1/-1/-7、完了時の応答：2/-2/-3)
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
	if (rslt >= _IFM_ANS_DONE)					// 実行結果についてA3固定とする
		cache.msg76.PayMode[1] = '3';
	set_sw_info(&cache.msg76.SwInfo, rslt, frs);
	ifm_send_rsp(76, (tIFMMSG*)&cache.msg76, rslt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: Initialize IFM communication
 *[]----------------------------------------------------------------------[]
 *| summary	: IFM通信初期化
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
